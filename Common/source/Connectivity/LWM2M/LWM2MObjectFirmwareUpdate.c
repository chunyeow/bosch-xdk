/*
* Licensee agrees that the example code provided to Licensee has been developed and released by Bosch solely as an example to be used as a potential reference for application development by Licensee. 
* Fitness and suitability of the example code for any use within application developed by Licensee need to be verified by Licensee on its own authority by taking appropriate state of the art actions and measures (e.g. by means of quality assurance measures).
* Licensee shall be responsible for conducting the development of its applications as well as integration of parts of the example code into such applications, taking into account the state of the art of technology and any statutory regulations and provisions applicable for such applications. Compliance with the functional system requirements and testing there of (including validation of information/data security aspects and functional safety) and release shall be solely incumbent upon Licensee. 
* For the avoidance of doubt, Licensee shall be responsible and fully liable for the applications and any distribution of such applications into the market.
* 
* 
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are 
* met:
* 
*     (1) Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer. 
* 
*     (2) Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.  
*     
*     (3)The name of the author may not be used to
*     endorse or promote products derived from this software without
*     specific prior written permission.
* 
*  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR 
*  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
*  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
*  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
*  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
*  POSSIBILITY OF SUCH DAMAGE.
*/
/*----------------------------------------------------------------------------*/

/**
 *  @brief This module will handle, FOTA related activities like FOTA resources State
 *  change, firmware storage and etc.
 *
 *  @file
 */
/* module includes ********************************************************** */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_COMMON_ID_LWM2MOBJECTFIRMWAREUPDATE

/* system header files */
//lint -e49 error in standard libraries suppressed #include <stdint.h>
//lint -e309 error in third party libraries suppressed BCDS_Assert.h
//lint -esym(956,*) /* Suppressing Non const, non volatile static or external variable */
/* own header files */
#include "BCDS_Fota.h"
#include "LWM2MObjectFirmwareUpdate.h"
#include "LWM2MObjectFirmwareUpdatePrivate.h"
#include "BCDS_FotaConfig.h"

/* additional interface header files */
#include "ff.h"
#include "BCDS_NVMConfig.h"
#include "BCDS_FWContainer.h"
#include "Serval_CoapClient.h"
#include "BCDS_CoapBlockwise.h"
#include "BCDS_Retcode.h"
#include "BCDS_SDCardPartitionAgent.h"
#include "BCDS_FotaRegistryAgent.h"
#include "BCDS_FWC1NoCryptoVerificationAgent.h"
#include "BCDS_Block512CopyAgent.h"
#include "BCDS_CoapDownloadAgent.h"
#include "BCDS_EFM32XXPartitionAgent.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "BCDS_BSP_Board.h"

#ifndef BCDS_FOTA_REQUEST_RETRY_TIMEOUT_MS
#define BCDS_FOTA_REQUEST_RETRY_TIMEOUT_MS      (COAP_ACK_TIMEOUT * 1000UL)
#endif /* BCDS_FOTA_REQUEST_RETRY_TIMEOUT_MS */

static_assert((1000UL <= BCDS_FOTA_REQUEST_RETRY_TIMEOUT_MS)&&(BCDS_FOTA_REQUEST_RETRY_TIMEOUT_MS <= UINT32_MAX), "FOTA request retry timeout must be atleast 1000 ms and lesser than UINT32_MAX")

#define FOTA_MS_TO_TICKS(X)  ((portTickType) X / portTICK_RATE_MS)
#define FOTA_TIMER_TRIGGER_TIMEOUT  FOTA_MS_TO_TICKS(2000)  /* 2 seconds */
#define FOTA_UPDATE_NOTIFY_TIME_IN_MS      UINT8_C(100) /**< Time for server to get the fota update status before rebooting */

static uint32_t RecBlockOptionValue = 0UL; /* the CoAP block offset value from the block header */

static struct NVM_Item_S NvmItemCurrentState;
static struct NVM_Item_S NvmItemNewState;
static struct NVM_Item_S NvmItemDownloadInProgress;
static struct NVM_Item_S NvmItemFirmwarePackageUrl;
static struct NVM_Item_S NvmItemCurrentResult;
static struct NVM_Item_S NvmItemDownloadFwCrc;
static const struct NVM_Item_S *NvmItems[] = {
        &NvmItemCurrentState,
        &NvmItemNewState,
        &NvmItemDownloadInProgress,
        &NvmItemFirmwarePackageUrl,
        &NvmItemCurrentResult,
        &NvmItemDownloadFwCrc
};
#define WriteFotaContext(id, parm, size) NVM_Write(&NVMUser, *(NvmItems[id]), parm, size)
#define FlushFotaContext() NVM_Flush(&NVMUser)
#define ReadFotaContext(id, parm, size) NVM_Read(&NVMUser,*(NvmItems[id]),parm, size)
#define FOTA_FIRMWARE_PACKAGE_URL_SIZE NVM_ITEM_ID_FIRMWARE_PACKAGE_URL_SIZE
#define FOTA_FIRMWARE_CRC_SIZE NVM_ITEM_ID_NEW_FW_CRC_SIZE

static FotaPartitionAgent_T * PrimaryPartitionAgent;
static FotaPartitionAgent_T * BackupPartitionAgent;
static FotaPartitionAgent_T * DownloadPartitionAgent;
static FotaRegistryAgent_T RegistryAgent;
static TimerHandle_t FotaTimerHandle = NULL; /* Handle for software timer to handle internal timeout requests and send failures */
static EventHub_T eventHub;

static bool UpdateTriggeredOnceFlag = false;
static bool HeaderValidationCheck = false;
static FWContainer_Header_T CurrentFirmwareHeader = { 0 };
static FWContainer_Header_T DownloadingFirmwareHeader = { 0 };

/**
 * @brief   This function maps the FotaStateMachine_State_T to a uint8_t value for which the
 * FotaStateTransitionTable will define the state transition possibility.
 *
 * @param[in] state
 *            FOTA state to be set or the current value.
 *
 * @retval  uint8_t
 *          UINT8_MAX if the state is not supported for state transition.
 *          Or any other valid value
 *
 * @see FotaStateTransitionTable
 */
static uint8_t FotaStateToValue(FotaStateMachine_State_T state)
{
    switch (state)
    {
    case FOTA_IDLE:
        return 0U;
    case FOTA_FAILED_NOTIFY:
        return 1U;
    case FOTA_DOWNLOADING_READY_FOR_REQUEST:
        return 2U;
    case FOTA_DOWNLOADING_NOTIFY:
        return 3U;
    case FOTA_DOWNLOADING_WAITING_FOR_RESPONSE:
        return 4U;
    case FOTA_DOWNLOADING_PROCESSING_RESPONSE:
        return 5U;
    case FOTA_DOWNLOADING_SUSPEND:
        return 6U;
    case FOTA_DOWNLOADING_RESUME:
        return 7U;
    case FOTA_DOWNLOADED:
        return 8U;
    case FOTA_UPDATING_NOTIFY:
        return 9U;
    case FOTA_UPDATING:
        return 10U;
    case FOTA_INVALID:
        case FOTA_STATE_NOT_IN_USERPAGE:
        default:
        return UINT8_MAX;
    }
}

/*
 * Below is the overview of the state transition table where:
 * 0 - state change is not permitted.
 * 1 - state change is permitted.
 *
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * | Current state        |                                                                     state to be changed                                                                                 |
 * |                      |--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                      | IDLE | FAILED_NOTIFY | READY_FOR_REQUEST | DOWNLOADING_NOTIFY | WAITING_FOR_RESPONSE | PROCESSING_RESPONSE | SUSPEND | RESUME | DOWNLOADED | UPDATING_NOTIFY | UPDATING |
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * | IDLE                 |  1   |       1       |         1         |          1         |           0          |           0         |    0    |    0   |      0     |        0        |     0    |
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * | FAILED_NOTIFY        |  1   |       1       |         1         |          1         |           0          |           0         |    0    |    0   |      0     |        0        |     0    |
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * | READY_FOR_REQUEST    |  1   |       1       |         1         |          0         |           1          |           0         |    1    |    0   |      1     |        0        |     0    |
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * | DOWNLOADING_NOTIFY   |  1   |       1       |         1         |          0         |           0          |           0         |    1    |    0   |      1     |        0        |     0    |
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * | WAITING_FOR_RESPONSE |  1   |       1       |         1         |          0         |           0          |           1         |    1    |    0   |      1     |        0        |     0    |
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * | PROCESSING_RESPONSE  |  1   |       1       |         1         |          0         |           0          |           0         |    1    |    0   |      1     |        0        |     0    |
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * | SUSPEND              |  1   |       1       |         0         |          0         |           0          |           0         |    1    |    1   |      0     |        0        |     0    |
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * | RESUME               |  1   |       1       |         1         |          0         |           0          |           0         |    1    |    0   |      0     |        0        |     0    |
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * | DOWNLOADED           |  1   |       1       |         0         |          0         |           0          |           0         |    0    |    0   |      0     |        1        |     1    |
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * | UPDATING_NOTIFY      |  1   |       1       |         0         |          0         |           0          |           0         |    0    |    0   |      1     |        0        |     1    |
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * | UPDATING             |  1   |       1       |         0         |          0         |           0          |           0         |    0    |    0   |      0     |        0        |     0    |
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 *
 * This table is defined for 11U x 11U since there are 11 valid states.
 * 1st dimension of the array represents the current state.
 * 2nd dimension of the array represents the state to be changed.
 */
static const bool FotaStateTransitionTable[11U][11U] =
        {
                { true, true, true, true, false, false, false, false, false, false, false },
                { true, true, true, true, false, false, false, false, false, false, false },
                { true, true, true, false, true, false, true, false, true, false, false },
                { true, true, true, false, false, false, true, false, true, false, false },
                { true, true, true, false, false, true, true, false, true, false, false },
                { true, true, true, false, false, false, true, false, true, false, false },
                { true, true, false, false, false, false, true, true, false, false, false },
                { true, true, true, false, false, false, true, false, false, false, false },
                { true, true, false, false, false, false, false, false, false, true, true },
                { true, true, false, false, false, false, false, false, true, false, true },
                { true, true, false, false, false, false, false, false, false, false, false }
        };

/* Local variable declaration */

static CmdProcessor_T* FotaCmdProcessorHandle = NULL;
static char PackageUrl[FOTA_MAX_PACKAGE_URL];
static volatile FotaStateMachine_State_T FOTA_State = FOTA_INVALID; /* instance created for FOTA_state and assigned default state */
static bool NotifyPendingFlag = false; /* Flag to intimate notification */

#define FOTA_URIPATH_OBJECT_INSTANCE            (UINT8_C(2))        /* Object Instance Instance is used to notify the fota object information */
#define FOTA_URIPATH_OBJECT_INDEX               (UINT8_C(2))        /* Object Index Value Represents the Index value of DeviceResource Inofrmation Structure while connecting to server */

/**
 * Instance of URI path structure for FOTA state
 */
static Lwm2m_URI_Path_T instanceUriPath =
        {
        FOTA_URIPATH_OBJECT_INDEX,
        FOTA_URIPATH_OBJECT_INSTANCE,
                -1
        }; /* instance of URI path structure for FOTA state */

#define FOTA_RESOURCE_INDEX_STATE 3 /* index in resource table below */
#define FOTA_RESOURCE_INDEX_RESULT 5 /* index in resource table below */

/**
 * LWM2M Firmware Object resources information are packed in this array, this called by Registration/Firmware updation
 */
LWM2MObjectFirmwareUpdate_Resource_T LWM2MObjectFirmwareUpdateResources =
        {
                { FOTA_RESOURCE_PACKAGE, LWM2M_DYNAMIC(FotaPackageResource) }, /* This is non-readable! */
                { FOTA_RESOURCE_PACKAGE_URI, LWM2M_DYNAMIC(FotaStateMachine_UriDownload) | LWM2M_WRITE_ONLY },
                { FOTA_RESOURCE_UPDATE, LWM2M_FUNCTION(FotaStateMachine_Update) },
                { FOTA_RESOURCE_UPDATE_STATE, LWM2M_INTEGER((int) FOTA_IDLE) | LWM2M_CONFIRM_NOTIFY },
                { FOTA_RESOURCE_UPDATE_SUPPORT_OBJ, LWM2M_BOOL(false) },
                { FOTA_RESOURCE_UPDATE_RESULT, LWM2M_INTEGER((uint32_t) LWM2M_OBJ_FW_UPDATE_DEFAULT) | LWM2M_CONFIRM_NOTIFY },
        };

/**
 * @brief Triggers notification to the server intimating the firmware update resource changes.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T StateAndUpdateResultNotify(void)
{
    Retcode_T returnCode = RETCODE_OK;

    if (true == NotifyPendingFlag)
    {
        if (RC_OK != Lwm2mReporting_multipleResourcesChanged(&instanceUriPath, 2, FOTA_RESOURCE_INDEX_STATE, FOTA_RESOURCE_INDEX_RESULT))
        {
            returnCode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) FOTA_FAILED_RESULT_AND_STATE_UPDATE);
        }
        else
        {
            NotifyPendingFlag = false;
        }
    }
    return returnCode;
}

/**
 * @brief Gets the current state of firmware update.
 *
 * @return int32_t
 *         current state of firmware update.
 */
static inline int32_t GetFirmwareUpdateState(void)
{
    return ((int32_t) FOTA_State & FOTA_LWM2M_STATUS_MASK);
}

/**
 * @brief Sets the current state of firmware update.
 *
 * @param[in] state
 *            the current state of firmware update.
 */
static void UpdateFirmwareUpdateStateResource(int32_t state)
{
    LWM2MObjectFirmwareUpdateResources.state.data.i = state;
}

/**
 * @brief Sets and triggers notification to the server intimating the firmware update state resource changes.
 *
 * @param[in] state
 *            the current state of firmware update.
 *
 * @param[in] notify
 *            boolean to trigger notification.
 *
 * @param[in] flush
 *            boolean to flush the changes to the NVM.
 *
 * @param[out] isLwm2mChange
 *             boolean to represent if the FOTA state requested to be set is a LWM2M (LWM2MObjectFirmwareUpdate_State_T) change.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T SetAndNotifyFotaState(FotaStateMachine_State_T state, bool notify, bool flush, bool * isLwm2mChange)
{
    uint32_t newStateValue;
    Retcode_T returnCode = RETCODE_OK;

    /* The FOTA_CONTEXT_STATE is written as uint32_t.
     * To avoid garbage from an enum upon code optimization,
     * we type-cast it as below. */
    uint32_t fotaState = (uint32_t) state;
    *isLwm2mChange = false;

    if (FOTA_State != (FotaStateMachine_State_T) fotaState)
    {
        newStateValue = ((uint32_t) fotaState & FOTA_LWM2M_STATUS_MASK);

        if (LWM2MObjectFirmwareUpdateResources.state.data.i != (int32_t) newStateValue)
        {
            /* Set the current FOTA State in NVM */
            returnCode = WriteFotaContext(FOTA_CONTEXT_STATE, (uint32_t * ) &newStateValue, sizeof(uint32_t));
            if (RETCODE_OK == returnCode)
            {
                *isLwm2mChange = true;
                UpdateFirmwareUpdateStateResource((int32_t) newStateValue);
                NotifyPendingFlag = true;
            }
        }
        if (RETCODE_OK == returnCode)
        {
            FOTA_State = (FotaStateMachine_State_T) fotaState;
        }
    }
    if ((RETCODE_OK == returnCode) && (true == notify) && (true == NotifyPendingFlag))
    {
        /* trigger notification to the server */
        returnCode = StateAndUpdateResultNotify();
    }
    if ((RETCODE_OK == returnCode) && (true == flush))
    {
        /* save changes to flash */
        returnCode = FlushFotaContext();
    }

    return returnCode;
}

static void FailureReason(Retcode_T rc, uint32_t * fotaControlValue)
{
    switch (rc)
    {
    case FOTA_FW_CRCINIT_FAIL:
        case FOTA_FW_CRCRUN_FAIL:
        case FOTA_FW_CRCFINAL_FAIL:
        case FOTA_FW_CRC_FAIL:
        FOTA_SET_UPDATE_RESULT(*fotaControlValue, LWM2M_OBJ_FW_UPDATE_CRC_FAILED);
        break;

    case FOTA_RETCODE_FIRMWARE_VERSION_FAIL:
        FOTA_SET_UPDATE_RESULT(*fotaControlValue, LWM2M_OBJ_FW_UPDATE_UNSUPPORTED_TYPE);
        break;

    case FOTA_RETCODE_FIRMWARE_SIZE_FAIL:
        FOTA_SET_UPDATE_RESULT(*fotaControlValue, LWM2M_OBJ_FW_UPDATE_OUT_OF_STORAGE);
        break;

    default:
        FOTA_SET_UPDATE_RESULT(*fotaControlValue, LWM2M_OBJ_FW_UPDATE_FAILED);
        break;

    }
}

/**
 * @brief Gets the current update result of firmware update.
 *
 * @return int32_t
 * current update result of firmware update.
 */
static int32_t GetFotaResultResource(void)
{
    return LWM2MObjectFirmwareUpdateResources.updateResult.data.i;
}

/**
 * @brief Sets the current update result of firmware update.
 *
 * @param[in] updateResult
 *            the current update result of firmware update.
 */
static void UpdateFirmwareUpdateResultResource(uint8_t updateResult)
{
    LWM2MObjectFirmwareUpdateResources.updateResult.data.i = (int32_t) updateResult;
}

/**
 * @brief Sets and triggers notification to the server intimating the firmware update "update result" resource changes.
 *
 * @param[in] updateResult
 * the current update result of firmware update.
 *
 * @param[in] notify
 * boolean to trigger notification.
 *
 * @param[in] flush
 * boolean to flush the changes to the NVM.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T SetAndNotifyFotaUpdateResult(uint8_t updateResult, bool notify, bool flush)
{
    Retcode_T returnCode = RETCODE_OK;

    if (GetFotaResultResource() != updateResult)
    {
        /* Set the current FOTA State in NVM */
        returnCode = WriteFotaContext(FOTA_CONTEXT_CURRENT_RESULT, (uint8_t * ) &updateResult, sizeof(uint8_t));
        if (RETCODE_OK == returnCode)
        {
            UpdateFirmwareUpdateResultResource(updateResult);
            NotifyPendingFlag = true;
        }
    }
    if ((RETCODE_OK == returnCode) && (true == notify) && (true == NotifyPendingFlag))
    {
        /* Save changes to flash */
        returnCode = StateAndUpdateResultNotify();
    }
    if ((RETCODE_OK == returnCode) && (true == flush))
    {
        /* Save changes to flash */
        returnCode = FlushFotaContext();
    }

    return returnCode;
}

/**
 * @brief Validates if a firmware update state value.
 *
 * @param[in] state
 * a firmware update state value.
 *
 * @return  boolean representing the validity.
 */
static bool IsFotaStateValid(LWM2MObjectFirmwareUpdate_State_T state)
{
    bool status = false;
    if ((LWM2M_OBJ_FW_UPDATE_STATE_IDLE == state) ||
            (LWM2M_OBJ_FW_UPDATE_STATE_DOWNLOADING == state) ||
            (LWM2M_OBJ_FW_UPDATE_STATE_DOWNLOADED == state) ||
            (LWM2M_OBJ_FW_UPDATE_STATE_UPDATING == state))
    {
        status = true;
    }
    return (status);
}

/* The function description is available in the Private header */
static bool FotaIsStateChangePermitted(FotaStateMachine_State_T state)
{
    if ((UINT8_MAX == FotaStateToValue(FOTA_State)) || (UINT8_MAX == FotaStateToValue(state)))
    {
        return false;
    }
    else
    {
        return (FotaStateTransitionTable[FotaStateToValue(FOTA_State)][FotaStateToValue(state)]);
    }
}

static void FotaPersistenceAgent(uint32_t param2)
{
    if (param2 != 0UL)
    {
        Retcode_T returnCode = RETCODE_OK;

        bool stateChange = false;
        uint32_t state = 0UL;
        bool updateResultChange = false;
        uint32_t updateResult = 0UL;
        bool newFirmwareChange = false;
        uint8_t newFirmwareValue = false;
        bool dlInProgressChange = false;
        uint8_t dlInProgressValue = false;
        bool urlTypeChange = false;
        bool urlTypeValue = false;

        FOTA_RETREIVE_STATE_INFO(param2, state, stateChange);
        FOTA_RETREIVE_NEW_FIRMWARE_INFO(param2, newFirmwareValue, newFirmwareChange);
        FOTA_RETREIVE_DL_IN_PROGRESS(param2, dlInProgressValue, dlInProgressChange);
        FOTA_RETREIVE_URL_TYPE(param2, urlTypeValue, urlTypeChange);
        FOTA_RETREIVE_UPDATE_RESULT_INFO(param2, updateResult, updateResultChange);

        if ((stateChange) && (RETCODE_OK == returnCode))
        {
            if (FotaIsStateChangePermitted((FotaStateMachine_State_T) state))
            {
                returnCode = SetAndNotifyFotaState((FotaStateMachine_State_T) state, !(uint8_t) updateResultChange, false, &stateChange);
            }
            else
            {
                stateChange = false;
                returnCode = RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_FOTA_STATE_CHANGE_DISCARDED);
            }
        }
        if ((newFirmwareChange) && (RETCODE_OK == returnCode))
        {
            returnCode = WriteFotaContext(FOTA_CONTEXT_NEW_FIRMWARE, (uint8_t * ) &newFirmwareValue, sizeof(uint8_t));
        }
        if ((dlInProgressChange) && (RETCODE_OK == returnCode))
        {
            returnCode = WriteFotaContext(FOTA_CONTEXT_DOWNLOAD_IN_PROGRESS, (uint8_t * )&dlInProgressValue, sizeof(uint8_t));
        }
        if ((urlTypeChange) && (RETCODE_OK == returnCode))
        {
            if (urlTypeValue)
            {
                returnCode = WriteFotaContext(FOTA_CONTEXT_URL, (void * ) PackageUrl, FOTA_FIRMWARE_PACKAGE_URL_SIZE);
            }
            else
            {
                uint8_t packageUrlTemp[FOTA_FIRMWARE_PACKAGE_URL_SIZE] = { 0 };
                returnCode = WriteFotaContext(FOTA_CONTEXT_URL, (void* ) packageUrlTemp, FOTA_FIRMWARE_PACKAGE_URL_SIZE);
            }
        }
        if ((updateResultChange) && (RETCODE_OK == returnCode))
        {
            returnCode = SetAndNotifyFotaUpdateResult((uint8_t) updateResult, true, false);
        }
        if ((RETCODE_OK == returnCode) &&
                ((uint8_t) stateChange | (uint8_t) updateResultChange | (uint8_t) newFirmwareChange |
                        (uint8_t) dlInProgressChange | (uint8_t) urlTypeChange))
        {
            returnCode = FlushFotaContext();
        }
        if (RETCODE_OK != returnCode)
        {
            Retcode_RaiseError(returnCode);
        }
    }
    else
    {
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INCONSITENT_STATE));
    }
}

#define FIRMWARE_FILE_NAME_MAX_SIZE     UINT8_C(16)
#define FIRMWARE_FILE_DOWNLOAD          "firmware.bin"
#define FIRMWARE_FILE_BACKUP            "firmware.bkp"
#define FILE_PAGE_SIZE                  UINT16_C(512)
#define FILE_OPERATION_READ             UINT8_C(0)
#define FILE_OPERATION_WRITE            UINT8_C(1)
static uint32_t BlockNoValue = 0UL;

static Retcode_T SDCardStorage_getFileSize(const char* path, uint32_t* size, uint8_t fileOperation)
{
    Retcode_T retval = RETCODE_OK;
    FRESULT _fileSystemResult;
    FILINFO _fileInfo;
#if _USE_LFN
    _fileInfo.lfname = NULL;
#endif
    memset(&_fileInfo, 0, sizeof(_fileInfo));
    _fileSystemResult = f_stat(path, &_fileInfo);
    if (FR_OK == _fileSystemResult)
    {
        *size = _fileInfo.fsize;
    }
    else if (FR_NO_FILE == _fileSystemResult)
    {
        if (fileOperation == FILE_OPERATION_READ)
        {
            retval = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) SDCARD_FILE_NOT_FOUND);
        }
    }
    else
    {
        retval = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) SDCARD_FILE_READ_ERROR);
    }
    return retval;
}

static Retcode_T SDCardStorage_checkCardInserted(void)
{
    Retcode_T retval = RETCODE_OK;
    SDCardDriver_Status_T status = SDCardDriver_GetDetectStatus();
    if (SDCARD_NOT_INSERTED == status)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) SDCARD_NOT_DETECTED));
    }
    return retval;
}

static Retcode_T SDCardStorage_WriteBlockNo(uint32_t blocknumber)
{
    BlockNoValue = blocknumber;
    return (RETCODE_OK);
}

static Retcode_T SDCardStorage_ReadBlockNo(uint32_t *blocknumber)
{
    if (NULL == blocknumber)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    Retcode_T retcode = RETCODE_OK;
    if (0UL != BlockNoValue)
    {
        *blocknumber = BlockNoValue;
    }
    else
    {
        uint32_t fileSize = UINT32_C(0);

        retcode = SDCardStorage_checkCardInserted();
        if (RETCODE_OK == retcode)
        {
            retcode = SDCardStorage_getFileSize((const char*) FIRMWARE_FILE_DOWNLOAD, &fileSize, FILE_OPERATION_READ);
            if ((Retcode_T) SDCARD_FILE_NOT_FOUND == Retcode_GetCode(retcode))
            {
                retcode = RETCODE_OK;
            }
        }
        if (RETCODE_OK == retcode)
        {
            *blocknumber = fileSize / FOTA_COAP_BLOCK_SIZE;
        }
        else
        {
            *blocknumber = UINT32_C(0);
        }
    }

    return (retcode);
}

/**
 * @brief Resets the FOTA related data and NVM credentials to its default state.
 *
 * @param[in] resetState
 * boolean to reset the state variable.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T FotaStateMachine_ResetCredentials(bool resetState, uint32_t * controlValue)
{
    Retcode_T RetValue = RETCODE_OK;
    uint32_t NextBlockNumber = 0;

    /* Reset the Next block to download in NVM to zero */
    RetValue = SDCardStorage_WriteBlockNo(NextBlockNumber);

    if (RETCODE_OK == RetValue)
    {
        if (true == resetState)
        {
            uint32_t fotaControlValue = 0UL;

            FOTA_SET_DL_IN_PROGRESS(fotaControlValue, FALSE);
            FOTA_SET_STATE(fotaControlValue, FOTA_IDLE);
            FOTA_SET_URL_TYPE(fotaControlValue, false);
            FotaPersistenceAgent(fotaControlValue);
            ;
        }
        else
        {
            FOTA_SET_DL_IN_PROGRESS(*controlValue, FALSE);
            FOTA_SET_URL_TYPE(*controlValue, false);
        }
    }
    return RetValue;
}

/* local function prototype declarations */

/* The function description is available in the Private header */
static retcode_t FotaPackageResource(Lwm2mSerializer_T *serializerPntr, Lwm2mParser_T *parserPntr)
{
    BCDS_UNUSED(serializerPntr);
    retcode_t rc = RC_OK;

    if (parserPntr != NULL)
    {
        rc = RC_LWM2M_METHOD_NOT_ALLOWED;
    }
    else
    {
        /* ignore this, it is write-only */
    }
    return rc;
}

/**
 * @brief   Parses a string from an incoming lwm2m message.
 *          The function takes care to handle TLV decoding if necessary
 *
 * @param[in] parserPntr
 *          Pointer to the parser structure (typically obtained in the callback)
 *
 * @param[in] packageUrl
 *          Pointer to the buffer for the storing the extracted package URL
 *
 * @retval RC_OK on success
 *         RC_LWM2M_PARSING_ERROR if the data could not be parsed properly
 *         RC_PLATFORM_ERROR if the data is not processed due to application error
 */
static retcode_t ExtractLocation(Lwm2mParser_T *parserPntr, char *Url, uint32_t * fotaControlValue)
{
    StringDescr_T descr;
    retcode_t rc = Lwm2mParser_getString(parserPntr, &descr);
    Retcode_T retcode = RETCODE_OK;

    if (RC_OK == rc)
    {
        if ((0 == descr.length) && (LWM2M_OBJ_FW_UPDATE_STATE_UPDATING != (LWM2MObjectFirmwareUpdate_State_T) GetFirmwareUpdateState())) /* Empty URI Triggered from Server For Aborting the FOTA Activity */
        {
            FOTA_SET_STATE(*fotaControlValue, FOTA_FAILED_NOTIFY);
            FOTA_SET_UPDATE_RESULT(*fotaControlValue, LWM2M_OBJ_FW_UPDATE_DEFAULT);
            FOTA_SET_URL_TYPE(*fotaControlValue, false);

            if ((int32_t) LWM2M_OBJ_FW_UPDATE_STATE_DOWNLOADING == GetFirmwareUpdateState())
            {
                retcode = CoapDownloadAgent->StopDownload();
                if (RETCODE_OK != retcode)
                {
                    Retcode_RaiseError(retcode);
                }
            }
        }
        else if (descr.length >= (int) FOTA_MAX_PACKAGE_URL)
        {
            rc = RC_APP_ERROR;
        }
        else if (LWM2M_OBJ_FW_UPDATE_STATE_IDLE == (LWM2MObjectFirmwareUpdate_State_T) GetFirmwareUpdateState())
        {
            memset(Url, '\0', FOTA_MAX_PACKAGE_URL);
            StringDescr_copySegment(&descr, Url, 0, FOTA_MAX_PACKAGE_URL);
        }
    }
    return rc;
}

/**
 * @brief   This function is used to read FOTA State, FOTA Resut and FOTA FW acceptance data from NVM.
 *
 * @param[out] state
 * Read firmware update state value from the NVM at power-on
 *
 * @param[out] updateResult
 * Read firmware update "update result" value from the NVM at power-on
 *
 * @param[out] appStatus
 * Read firmware update application status (acceptance flag) value from the NVM at power-on
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T GetFotaNVMData(uint32_t* state, uint8_t* updateResult, uint8_t* appStatus)
{
    Retcode_T returnCode = RETCODE_OK;
    /* Retrieve the FOTA State from NVM after POR */
    returnCode = ReadFotaContext(FOTA_CONTEXT_STATE, state, 4U);
    if (RETCODE_OK == returnCode)
    {
        /* Retrieve the FOTA Result from NVM after POR */
        returnCode = ReadFotaContext(FOTA_CONTEXT_CURRENT_RESULT, (uint8_t * )updateResult, sizeof(uint8_t));
        if (RETCODE_OK == returnCode)
        {
            /* Retrieve the Software quality flag from NVM after New Firmware Upgrade */
            returnCode = ReadFotaContext(FOTA_CONTEXT_NEW_FIRMWARE, (uint8_t * )appStatus, sizeof(uint8_t));
        }
    }
    else
    {
        returnCode = RETCODE(RETCODE_SEVERITY_WARNING, (Retcode_T ) FOTA_FAILED_NVM_READ_CREDENTIALS);
    }
    return returnCode;
}

static void UpdatetriggerStep2(void * param, uint32_t data)
{
    BCDS_UNUSED(param);
    BCDS_UNUSED(data);

    vTaskDelay(FOTA_UPDATE_NOTIFY_TIME_IN_MS / portTICK_PERIOD_MS);
    BSP_Board_SoftReset();

}

/*Do Reboot Here since the download is success give few milliseconds delay before Reboot*/
static void UpdatetriggerStep1(void * param, uint32_t data)
{
    BCDS_UNUSED(param);
    BCDS_UNUSED(data);

    if (false == UpdateTriggeredOnceFlag)
    {
        Retcode_T returnCode = RETCODE_OK;

        uint32_t fotaControlValue = 0UL;
        /* To proceed the FOTA update application will be doing the necessary tasks befor calling FotaStateMachine_Update() API
         while getting the FOTA_State = FOTA_UPDATING_NOTIFY */
        /* Reset all the FOTA credentials before reset */
        returnCode = FotaStateMachine_ResetCredentials(false, &fotaControlValue);
        if (RETCODE_OK == returnCode)
        {
            /* Signal to bootloader that a new firmware package has been received */
            FOTA_SET_NEW_FIRMWARE(fotaControlValue, 1U);
            FOTA_SET_STATE(fotaControlValue, FOTA_UPDATING);
            FotaPersistenceAgent(fotaControlValue);
        }

        if (RETCODE_OK == returnCode)
        {
            returnCode = CmdProcessor_Enqueue(FotaCmdProcessorHandle, UpdatetriggerStep2, NULL, 0UL);
            UpdateTriggeredOnceFlag = true;
        }
        if (RETCODE_OK != returnCode)
        {
            Retcode_RaiseError(returnCode);
        }
    }
    else
    {
        printf("Redundant Update trigger \r\n");
    }
}

/* The function description is available in the Private header */
static retcode_t FotaStateMachine_Update(Lwm2mSerializer_T *serializerPntr, Lwm2mParser_T *parserPntr)
{
    BCDS_UNUSED(parserPntr);
    BCDS_UNUSED(serializerPntr);

    retcode_t rc = RC_OK;
    Retcode_T retcode = RETCODE_OK;

    if (FOTA_DOWNLOADED != FOTA_State)
    {
        rc = RC_LWM2M_BAD_REQUEST;
    }
    else
    {
        /* Set the current FOTA State in NVM */
        uint32_t fotaControlValue = 0UL;
        FOTA_SET_STATE(fotaControlValue, FOTA_UPDATING_NOTIFY);
        FotaPersistenceAgent(fotaControlValue);

        retcode = CmdProcessor_Enqueue(FotaCmdProcessorHandle, UpdatetriggerStep1, NULL, 0UL);
        if (RETCODE_OK != retcode)
        {
            Retcode_RaiseError(retcode);
        }
    }
    return (rc);
}

static CoapDownloadAgentCfg_T CoapDownloadConfiguration;

/* The function description is available in the Private header */
static retcode_t FotaStateMachine_UriDownload(Lwm2mSerializer_T *serializerPntr, Lwm2mParser_T *parserPntr)
{
    BCDS_UNUSED(serializerPntr);

    retcode_t rc = RC_OK;

    if (NULL == parserPntr)
    {
        rc = RC_LWM2M_METHOD_NOT_ALLOWED;
    }
    else
    {
        /* Extract the firmware location details */
        uint32_t fotaControlValue = 0UL;
        rc = ExtractLocation(parserPntr, PackageUrl, &fotaControlValue);
        if (RC_OK != rc)
        {
            FOTA_SET_UPDATE_RESULT(fotaControlValue, LWM2M_OBJ_FW_UPDATE_INVALID_URI);
        }
        else if (((FOTA_IDLE == FOTA_State) || (FOTA_FAILED_NOTIFY == FOTA_State)) && (0UL == fotaControlValue))/* Initiate a firmware download only if the current state is IDLE state */
        {
            /* configuring the firmware download */

            CoapDownloadConfiguration.CmdProcessorHandle = FotaCmdProcessorHandle;
            CoapDownloadConfiguration.BlockNumber = 0UL;
            memset(CoapDownloadConfiguration.PackageUrl, 0U, FOTA_MAX_PACKAGE_URL);
            memcpy(CoapDownloadConfiguration.PackageUrl, PackageUrl, strlen(PackageUrl));

            Retcode_T returnCode = CoapDownloadAgent->InitalizeDownload(&CoapDownloadConfiguration);

            if (RETCODE_OK != returnCode)
            {
                if ((Retcode_T) SDCARD_NOT_DETECTED == Retcode_GetCode(returnCode))
                {
                    FOTA_SET_UPDATE_RESULT(fotaControlValue, LWM2M_OBJ_FW_UPDATE_OUT_OF_STORAGE);
                }
                else
                {
                    FOTA_SET_UPDATE_RESULT(fotaControlValue, LWM2M_OBJ_FW_UPDATE_INVALID_URI);
                }
                Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_WARNING, (Retcode_T ) FOTA_FAILED_TO_SETUP_NEW_DOWNLOAD));
            }
            else
            {
                returnCode = SDCardStorage_WriteBlockNo(0UL);

                if (RETCODE_OK == returnCode)
                {
                    HeaderValidationCheck = false;
                    RecBlockOptionValue = 0UL;
                    /* status updated to downloading */
                    FOTA_SET_DL_IN_PROGRESS(fotaControlValue, TRUE);
                    FOTA_SET_STATE(fotaControlValue, FOTA_DOWNLOADING_READY_FOR_REQUEST);
                    FOTA_SET_UPDATE_RESULT(fotaControlValue, LWM2M_OBJ_FW_UPDATE_DEFAULT);
                    FOTA_SET_URL_TYPE(fotaControlValue, true);
                }
            }
        }

        FotaPersistenceAgent(fotaControlValue);
    }
    return rc;
}

/**
 * @brief   This function is used resume the FOTA after the reset or connection loss.
 *          This function uses the details from the NVM and controls the state machine to
 *          continue collecting the block form last collected block number.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T FotaStateMachine_ResumeFota(uint32_t * nextBlockNumber)
{
    Retcode_T ReturnVal = RETCODE_OK;

    /* Read the next block to be download before reset */

    ReturnVal = SDCardStorage_ReadBlockNo(nextBlockNumber);

    if (RETCODE_OK == ReturnVal)
    {
        /* Read the package URI which was used for downloading before reset */
        ReturnVal = ReadFotaContext(FOTA_CONTEXT_URL, (void * ) PackageUrl, FOTA_FIRMWARE_PACKAGE_URL_SIZE);
    }
    if (RETCODE_OK == ReturnVal)
    {

        CoapDownloadConfiguration.CmdProcessorHandle = FotaCmdProcessorHandle;
        CoapDownloadConfiguration.BlockNumber = *nextBlockNumber;

        memset(CoapDownloadConfiguration.PackageUrl, 0U, FOTA_MAX_PACKAGE_URL);
        memcpy(CoapDownloadConfiguration.PackageUrl, PackageUrl, strlen(PackageUrl));

        Retcode_T returnCode = CoapDownloadAgent->InitalizeDownload(&CoapDownloadConfiguration);

        if (RETCODE_OK != returnCode)
        {
            Retcode_RaiseError(ReturnVal);

            ReturnVal = SetAndNotifyFotaUpdateResult((uint32_t) LWM2M_OBJ_FW_UPDATE_FAILED, false, true);
            if (RETCODE_OK == ReturnVal)
            {
                ReturnVal = FotaStateMachine_ResetCredentials(true, NULL);
            }
        }
        else
        {
            uint32_t fotaControlValue = 0UL;
            ReturnVal = SDCardStorage_WriteBlockNo(*nextBlockNumber);
            if (RETCODE_OK == ReturnVal)
            {
                RecBlockOptionValue = 0UL;
                /* status updated to downloading */
                FOTA_SET_DL_IN_PROGRESS(fotaControlValue, TRUE);
                FOTA_SET_STATE(fotaControlValue, FOTA_DOWNLOADING_READY_FOR_REQUEST);
                FOTA_SET_UPDATE_RESULT(fotaControlValue, LWM2M_OBJ_FW_UPDATE_DEFAULT);
                FOTA_SET_URL_TYPE(fotaControlValue, true);
                FotaPersistenceAgent(fotaControlValue);
            }
        }
    }

    return ReturnVal;
}

/**
 * @brief   This function is used to check the new firmware acceptance on power up & update the
 * FOTA result & FOTA state FOTA object resources.so that device will report it to the server
 * after registration.
 *
 * @param[in] acceptNewFW
 * Flag read from the NVM is passed. This could be set by the bootloader after the update.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T CheckFotaNewFirmwareAcceptance(uint8_t acceptNewFW)
{
    Retcode_T returnCode = RETCODE_OK;
    bool isLwm2mStateChange = false;

    if (acceptNewFW == FALSE)
    {
        /*  Updating the FOTA Updated status is success in Result resource,
         *  only if NVM_ITEM_ID_IS_NEW_FW is Set and FOTA state is in Updating
         */
        returnCode = SetAndNotifyFotaState(FOTA_IDLE, false, false, &isLwm2mStateChange);
        if (RETCODE_OK == returnCode)
        {
            returnCode = SetAndNotifyFotaUpdateResult((uint32_t) LWM2M_OBJ_FW_UPDATE_SUCCESS, true, true);
        }
    }
    else
    {
        /*  Updating the FOTA Updated status is failed in Result resource,
         *  Because firmware swapping has failed.
         */
        returnCode = SetAndNotifyFotaState(FOTA_DOWNLOADED, false, false, &isLwm2mStateChange);
        if (RETCODE_OK == returnCode)
        {
            returnCode = SetAndNotifyFotaUpdateResult((uint32_t) LWM2M_OBJ_FW_UPDATE_FAILED, true, true);
        }
    }
    return returnCode;
}

/* This function will reset the FOTA credentials directly */
static Retcode_T ForcedResetCredentials(void)
{
    uint32_t updateResult = (uint32_t) LWM2M_OBJ_FW_UPDATE_FAILED;
    uint32_t NextBlockNumber = 0UL;
    uint8_t dlInProgressValue = FALSE;
    uint8_t newFirmwareValue = false;
    uint8_t packageUrlTemp[FOTA_FIRMWARE_PACKAGE_URL_SIZE] = { 0 };
    uint32_t stateValue = (uint32_t) FOTA_IDLE;

    /* Reset the Next block to download in NVM to zero */
    Retcode_T returnCode = RETCODE_OK;

    returnCode = SDCardStorage_WriteBlockNo(NextBlockNumber);
    if (RETCODE_OK == returnCode)
    {
        returnCode = WriteFotaContext(FOTA_CONTEXT_DOWNLOAD_IN_PROGRESS, (uint8_t * )&dlInProgressValue, sizeof(uint8_t));
    }
    if (RETCODE_OK == returnCode)
    {
        returnCode = WriteFotaContext(FOTA_CONTEXT_URL, (void* ) packageUrlTemp, FOTA_FIRMWARE_PACKAGE_URL_SIZE);
    }
    if (RETCODE_OK == returnCode)
    {
        returnCode = WriteFotaContext(FOTA_CONTEXT_NEW_FIRMWARE, (uint8_t * ) &newFirmwareValue, sizeof(uint8_t));
    }
    if (RETCODE_OK == returnCode)
    {
        returnCode = WriteFotaContext(FOTA_CONTEXT_STATE, (uint32_t * ) &stateValue, sizeof(uint32_t));
    }
    if (RETCODE_OK == returnCode)
    {
        FOTA_State = (FotaStateMachine_State_T) stateValue;
        UpdateFirmwareUpdateStateResource((int32_t) stateValue);
    }
    if (RETCODE_OK == returnCode)
    {
        returnCode = WriteFotaContext(FOTA_CONTEXT_CURRENT_RESULT, (uint8_t * ) &updateResult, sizeof(uint8_t));
    }
    if (RETCODE_OK == returnCode)
    {
        UpdateFirmwareUpdateResultResource(updateResult);
    }
    if (RETCODE_OK == returnCode)
    {
        returnCode = FlushFotaContext();
    }
    if (RETCODE_OK == returnCode)
    {
        NotifyPendingFlag = true;
        /* Save changes to flash */
        returnCode = StateAndUpdateResultNotify();
    }

    return (returnCode);
}

static void VerifyFirmwareUponDownloaded(void * param, uint32_t data)
{
    BCDS_UNUSED(param);
    BCDS_UNUSED(data);

    uint32_t fotaControlValue = 0UL;

    Retcode_T returnCode = FWC1NoCryptoVerificationAgent->VerifyImage();
    if (RETCODE_OK == returnCode)
    {
        printf("Downloaded Image verification passed \r\n");

        Retcode_T returnValue = RETCODE_OK;

        memset(&DownloadingFirmwareHeader, 0, sizeof(FWContainer_Header_T));
        returnCode = FWC1NoCryptoVerificationAgent->ReadHeader(&DownloadingFirmwareHeader);
        if (RETCODE_OK == returnValue)
        {
            uint32_t downloadCrcValue = DownloadingFirmwareHeader.FirmwareCRC;
            returnValue = WriteFotaContext(FOTA_CONTEXT_DOWNLOAD_CRC, (void * )&downloadCrcValue, FOTA_FIRMWARE_CRC_SIZE);
        }
        if (RETCODE_OK == returnValue)
        {
            printf("Downloaded Image CRC check passed \r\n");
            FOTA_SET_DL_IN_PROGRESS(fotaControlValue, FALSE);
            FOTA_SET_STATE(fotaControlValue, FOTA_DOWNLOADED);
            FOTA_SET_UPDATE_RESULT(fotaControlValue, LWM2M_OBJ_FW_UPDATE_DEFAULT);
        }
        else
        {
            printf("Downloaded Image CRC check failed \r\n");
            FOTA_SET_STATE(fotaControlValue, FOTA_FAILED_NOTIFY);
            FOTA_SET_UPDATE_RESULT(fotaControlValue, LWM2M_OBJ_FW_UPDATE_FAILED);
        }
    }
    else
    {
        printf("FWC1NoCryptoVerificationAgent->VerifyImage failed \r\n");
        FailureReason(Retcode_GetCode(returnCode), &fotaControlValue);
        FOTA_SET_UPDATE_RESULT(fotaControlValue, LWM2M_OBJ_FW_UPDATE_CRC_FAILED);
        FOTA_SET_STATE(fotaControlValue, FOTA_FAILED_NOTIFY);
    }
    FotaPersistenceAgent(fotaControlValue);
}

void FotaEvent(TaskEvent_T event, void * data)
{
    Retcode_T retcode = RETCODE_OK;
    switch (event)
    {
    case FOTA_EVENT_DOWNLOADAGENT_STARTED:
        printf("FOTA_EVENT_DOWNLOADAGENT_STARTED \r\n");
        break;
    case FOTA_EVENT_DOWNLOADAGENT_STOPPED:
        printf("FOTA_EVENT_DOWNLOADAGENT_STOPPED \r\n");
        break;
    case FOTA_EVENT_DOWNLOADAGENT_CONTINUE:
        printf("FOTA_EVENT_DOWNLOADAGENT_CONTINUE \r\n");
        break;
    case FOTA_EVENT_DOWNLOADAGENT_UNEXPECTED_ERROR:
        printf("FOTA_EVENT_DOWNLOADAGENT_UNEXPECTED_ERROR \r\n");

        if (pdPASS != xTimerStart(FotaTimerHandle, FOTA_TIMER_TRIGGER_TIMEOUT))
        {
            Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_FOTA_TIMER_START_TRIGGER_FAILED));
        }
        break;
    case FOTA_EVENT_DOWNLOADAGENT_PACKAGE_LOADED:
        RecBlockOptionValue = *(uint32_t *) data;
        printf("FOTA_EVENT_DOWNLOADAGENT_PACKAGE_LOADED Received Block - %d\r\n", (int) (RecBlockOptionValue + 1));

        if (false == HeaderValidationCheck)
        {
            if ((((RecBlockOptionValue + 1) * FOTA_COAP_BLOCK_SIZE) >= FWCONTAINER_BLOCK_SIZE) &&
                    (RecBlockOptionValue >= (FOTA_BLOCK_STORAGE_UPDATE_FREQUENCY - 1)))
            {
                uint32_t fotaControlValue = 0UL;
                Retcode_T returnCode = FWC1NoCryptoVerificationAgent->VerifyHeader();
                if (RETCODE_OK == returnCode)
                {
                    printf("Firmware header verification succeeded \r\n");

                    memset(&DownloadingFirmwareHeader, 0, sizeof(FWContainer_Header_T));
                    returnCode = FWC1NoCryptoVerificationAgent->ReadHeader(&DownloadingFirmwareHeader);
                    if (RETCODE_OK == returnCode)
                    {
                        memset(&CurrentFirmwareHeader, 0, sizeof(FWContainer_Header_T));
                        returnCode = EFM32XXPartitionAgent->Read(EFM32XXPartitionAgent, (uint8_t *) &CurrentFirmwareHeader, 0UL, sizeof(FWContainer_Header_T));
                        if (RETCODE_OK == returnCode)
                        {
                            if (CurrentFirmwareHeader.FirmwareVersion > DownloadingFirmwareHeader.FirmwareVersion)
                            {
                                printf("Firmware Version check failed \r\n");
                                FOTA_SET_UPDATE_RESULT(fotaControlValue, LWM2M_OBJ_FW_UPDATE_UNSUPPORTED_TYPE);
                                returnCode = RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_FAILURE); /* Only to notify the server */
                            }
                            else
                            {
                                printf("Firmware Version check passed \r\n");
                                HeaderValidationCheck = true;
                            }
                        }
                        else
                        {
                            printf("EFM32XXPartitionAgent->Read failed \r\n");
                            FOTA_SET_UPDATE_RESULT(fotaControlValue, LWM2M_OBJ_FW_UPDATE_OUT_OF_STORAGE);
                        }

                    }
                    else
                    {
                        printf("FWC1NoCryptoVerificationAgent->ReadHeader failed \r\n");
                        FOTA_SET_UPDATE_RESULT(fotaControlValue, LWM2M_OBJ_FW_UPDATE_OUT_OF_STORAGE);
                    }

                }
                else
                {
                    printf("FWC1NoCryptoVerificationAgent->VerifyImage failed \r\n");
                    FailureReason(Retcode_GetCode(returnCode), &fotaControlValue);
                }

                if (RETCODE_OK != returnCode)
                {
                    retcode = CoapDownloadAgent->StopDownload();
                    if (RETCODE_OK == retcode)
                    {
                        FOTA_SET_STATE(fotaControlValue, FOTA_FAILED_NOTIFY);
                        FotaPersistenceAgent(fotaControlValue);
                    }
                }
            }
        }

        break;
    case FOTA_EVENT_DOWNLOADAGENT_COMPLETE:
        printf("FOTA_EVENT_DOWNLOADAGENT_COMPLETE \r\n");
        retcode = CmdProcessor_Enqueue(FotaCmdProcessorHandle, VerifyFirmwareUponDownloaded, NULL, 0UL);
        break;
    case FOTA_EVENT_DOWNLOADAGENT_FAIL:
        printf("FOTA_EVENT_DOWNLOADAGENT_FAIL \r\n");
        break;
    case FOTA_EVENT_PARTITION_WRITE_FAIL:
        printf("FOTA_EVENT_PARTITION_WRITE_FAIL \r\n");
        break;
    case FOTA_EVENT_PARTITION_ERASE_FAIL:
        printf("FOTA_EVENT_PARTITION_ERASE_FAIL \r\n");
        break;
    case FOTA_EVENT_PARTITION_OUT_OF_STORAGE:
        printf("FOTA_EVENT_PARTITION_OUT_OF_STORAGE \r\n");
        break;
    case FOTA_EVENT_PARTITION_READ_FAIL:
        printf("FOTA_EVENT_PARTITION_READ_FAIL \r\n");
        break;
    case FOTA_EVENT_VERIFICATIONAGENT_IMAGE_FAIL:
        printf("FOTA_EVENT_VERIFICATIONAGENT_IMAGE_FAIL \r\n");
        break;

    case FOTA_EVENT_PARTITION_ERASE_OK:
        printf("FOTA_EVENT_PARTITION_ERASE_OK \r\n");
        break;

    case FOTA_EVENT_VERIFICATIONAGENT_HEADER_FAIL:
        printf("FOTA_EVENT_VERIFICATIONAGENT_HEADER_FAIL \r\n");
        break;

    case FOTA_EVENT_PARTITION_AGENT_INIT_OK:
        printf("FOTA_EVENT_PARTITION_AGENT_INIT_OK \r\n");
        break;

    case FOTA_EVENT_PARTITION_WRITE_OK:
        printf("FOTA_EVENT_PARTITION_WRITE_OK \r\n");
        break;

    case FOTA_EVENT_PARTITION_READ_OK:
        printf("FOTA_EVENT_PARTITION_READ_OK \r\n");
        break;

    default:
        printf("Default event - %d \r\n", (int) event);
        break;
    }

    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseError(retcode);
    }
}

static void FotaTimerCallback(TimerHandle_t xTimer)
{
    BCDS_UNUSED(xTimer);
    Retcode_T retcode = RETCODE_OK;

    if ((int32_t) LWM2M_OBJ_FW_UPDATE_STATE_DOWNLOADING == GetFirmwareUpdateState())
    {
        CoapDownloadAgentCfg_T cfg;
        cfg.BlockNumber = RecBlockOptionValue;
        retcode = CoapDownloadAgent->ResumeDownload(&cfg);
        if (RETCODE_OK != retcode)
        {
            Retcode_RaiseError(retcode);
        }
    }
    else
    {
        printf("Timer triggered when state was %d\r\n", (int) FOTA_State);
    }
}

Retcode_T Fota_EventHub_Init(void)
{
    Retcode_T Retcode = RETCODE_OK;

    Retcode = EventHub_Initialize(&eventHub);
    if (RETCODE_OK != Retcode)
    {
        Retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_FAILURE);
    }

    Retcode = EventHub_ObserveAll(&eventHub, (EventHandler_T) FotaEvent);
    if (RETCODE_OK != Retcode)
    {
        Retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_FAILURE);
    }

    return Retcode;
}

/* The description is in interface header */
Retcode_T LWM2MObjectFirmwareUpdate_Init(CmdProcessor_T* cmdProcessorHandle)
{
    Retcode_T returnCode = RETCODE_OK;

    HeaderValidationCheck = false;
    NotifyPendingFlag = false;
    RecBlockOptionValue = 0UL;

    NvmItemCurrentState = NVM_ITEM_ID_FOTA_CURRENT_STATE;
    NvmItemNewState = NVM_ITEM_ID_IS_NEW_FW;
    NvmItemDownloadInProgress = NVM_ITEM_ID_DOWNLOAD_IN_PROGRESS;
    NvmItemFirmwarePackageUrl = NVM_ITEM_ID_FIRMWARE_PACKAGE_URL;
    NvmItemCurrentResult = NVM_ITEM_ID_FOTA_CURRENT_RESULT;
    NvmItemDownloadFwCrc = NVM_ITEM_ID_NEW_FW_CRC;

    if (NULL == cmdProcessorHandle)
    {
        return (RETCODE(RETCODE_SEVERITY_FATAL, (uint32_t ) RETCODE_NULL_POINTER));
    }
    FotaCmdProcessorHandle = cmdProcessorHandle;

    Retcode_T ReturnValue = RETOCDE_MK_CODE((Retcode_T )RETCODE_FAILURE);
    PrimaryPartitionAgent = EFM32XXPartitionAgent;
    BackupPartitionAgent = SDCardPartitionAgent;
    DownloadPartitionAgent = SDCardPartitionAgent;

    PrimaryPartitionAgent->Partition = FOTA_PRIMARY_PARTTITION_START_ADDRESS;
    BackupPartitionAgent->Partition = FOTA_PARTITION_BACKUP;
    DownloadPartitionAgent->Partition = FOTA_PARTITION_DOWNLOAD;

    PrimaryPartitionAgent->PartitionSize = FOTA_PARTITION_MAX_SIZE;
    BackupPartitionAgent->PartitionSize = FOTA_PARTITION_MAX_SIZE;
    DownloadPartitionAgent->PartitionSize = FOTA_PARTITION_MAX_SIZE;

    ReturnValue = Fota_EventHub_Init();
    if (RETCODE_OK == ReturnValue)
    {
        ReturnValue = SDCardPartitionAgent->Initialize(SDCardPartitionAgent, &RegistryAgent, NULL, (FotaPartitionAgent_Adr_T) FOTA_PARTITION_DOWNLOAD, FOTA_PARTITION_MAX_SIZE);
    }
    if (RETCODE_OK == ReturnValue)
    {
        ReturnValue = CoapDownloadAgent->Initialize(SDCardPartitionAgent, &RegistryAgent, &eventHub);
    }
    if (RETCODE_OK == ReturnValue)
    {
        ReturnValue = PrimaryPartitionAgent->Initialize(PrimaryPartitionAgent, &RegistryAgent, &eventHub, PrimaryPartitionAgent->Partition, FOTA_PARTITION_MAX_SIZE);
    }
    if (RETCODE_OK == ReturnValue)
    {
        ReturnValue = BackupPartitionAgent->Initialize(BackupPartitionAgent, &RegistryAgent, NULL, BackupPartitionAgent->Partition, FOTA_PARTITION_MAX_SIZE);
    }
    if (RETCODE_OK == ReturnValue)
    {
        ReturnValue = EFM32XXPartitionAgent->Initialize(EFM32XXPartitionAgent, &RegistryAgent, NULL, FOTA_PRIMARY_PARTTITION_START_ADDRESS, FOTA_PARTITION_MAX_SIZE);
    }
    if (RETCODE_OK == ReturnValue)
    {
        uint32_t PublicKey = 0UL;

        ReturnValue = FWC1NoCryptoVerificationAgent->Initialize(DownloadPartitionAgent, &RegistryAgent, NULL, &PublicKey, 256UL);
    }

    if (RETCODE_OK == returnCode)
    {
        if (NULL == (FotaTimerHandle = xTimerCreate("FotaTimer", FOTA_MS_TO_TICKS(BCDS_FOTA_REQUEST_RETRY_TIMEOUT_MS), pdFALSE, (void *) 0, FotaTimerCallback)))
        {
            returnCode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_OUT_OF_RESOURCES);
        }
    }

    return returnCode;
}

/* The description is in interface header */
Retcode_T LWM2MObjectFirmwareUpdate_Enable(void)
{
    Retcode_T returnCode = RETCODE_OK;
    bool resetCredentialsFlag = false;
    uint32_t fotaState = 0;
    uint8_t result = 0;
    uint8_t isFWWaitingAcceptance = FALSE;
    uint8_t DownloadInProgress = FALSE;
    uint32_t NextBlockNumber = 0;
    bool isLwm2mStateChange = false;

    /*Reading Fotaresult, acceptance & state information from NVM on power up*/
    returnCode = GetFotaNVMData(&fotaState, &result, &isFWWaitingAcceptance);
    if (RETCODE_OK != returnCode)
    {
        /* NVM read has failed. There is no point in continuing further. */
        return returnCode;
    }

    /* validating the boundary conditions */
    if (IsFotaStateValid((LWM2MObjectFirmwareUpdate_State_T) fotaState))
    {
        FOTA_State = (FotaStateMachine_State_T) fotaState;
        UpdateFirmwareUpdateStateResource(GetFirmwareUpdateState());
        if (result <= (uint32_t) LWM2M_OBJ_FW_UPDATE_FAILED)
        {
            UpdateFirmwareUpdateResultResource(result);
        }
        else
        {
            resetCredentialsFlag = true;
        }
    }
    else
    {
        resetCredentialsFlag = true;
    }

    if (false == resetCredentialsFlag)
    {
        switch (FOTA_State)
        {
        case FOTA_IDLE:
            case FOTA_FAILED_NOTIFY:
            case FOTA_DOWNLOADED:
            /* Do nothing.
             * FOTA_IDLE - Everything is fine.
             * FOTA_FAILED_NOTIFY - Intimate the server and reset in state machine.
             * FOTA_DOWNLOADED - We await update trigger. */
            break;
        case FOTA_UPDATING:
            case FOTA_UPDATING_NOTIFY:
            /* In-case of FOTA_UPDATING we will discard the update trigger
             * for the application to diagnose the last error and await next trigger. */
            returnCode = CheckFotaNewFirmwareAcceptance(isFWWaitingAcceptance);
            break;
        case FOTA_DOWNLOADING_READY_FOR_REQUEST:
            case FOTA_DOWNLOADING_WAITING_FOR_RESPONSE:

            /* The Firmware download result from NVM is success.
             * We are in one of the several application's downloading states.
             * Confirm whether FOTA download was in progress before reset */
            returnCode = ReadFotaContext(FOTA_CONTEXT_DOWNLOAD_IN_PROGRESS, (uint8_t * )&DownloadInProgress, sizeof(uint8_t));

            if (RETCODE_OK == returnCode)
            {
                if (TRUE == DownloadInProgress)
                {
                    /* Get the FOTA credentials from NVM and be in state notify */
                    returnCode = FotaStateMachine_ResumeFota(&NextBlockNumber);
                    if (RETCODE_OK != returnCode)
                    {
                        resetCredentialsFlag = true;
                    }
                }
                else
                {
                    /* We had successfully downloaded and verified previously. */
                    returnCode = SetAndNotifyFotaState(FOTA_DOWNLOADED, false, false, &isLwm2mStateChange);
                    if (RETCODE_OK == returnCode)
                    {
                        returnCode = SetAndNotifyFotaUpdateResult((uint32_t) LWM2M_OBJ_FW_UPDATE_DEFAULT, false, true);
                    }
                }
            }
            break;
        default:
            /* Code should not reach here as all the valid FOTA states are already addressed. */
            returnCode = RETCODE(RETCODE_SEVERITY_WARNING, FOTA_FAILED_DUE_TO_INCONSISTENT_STATE);
            break;
        }
    }
    else
    {
        returnCode = RETCODE(RETCODE_SEVERITY_WARNING, FOTA_FAILED_DUE_TO_INCONSISTENT_STATE);
    }

    if (true == resetCredentialsFlag)
    {
        /* Intimating the error to the application and resetting the credentials. */
        Retcode_RaiseError(returnCode);

        returnCode = ForcedResetCredentials();
    }

    return returnCode;
}

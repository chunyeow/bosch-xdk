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
 * @brief This file handles the LWM2M module features.
 *
 * @file
 */

/* module includes ********************************************************** */
/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_LWM2M
#if XDK_CONNECTIVITY_LWM2M
/* own header files */
#include "XDK_LWM2M.h"

/* system header files */
#include <stdio.h>

/* additional interface header files */
#include "BCDS_WlanNetworkConfig.h"
#include "BCDS_CmdProcessor.h"
#include "BCDS_BSP_Board.h"
#include "BCDS_BSP_LED.h"
#include "BCDS_WlanNetworkConnect.h"
#include "BSP_BoardType.h"
#include "LWM2MObjectDevice.h"
#include "LWM2MObjects.h"
#include "LWM2MDnsUtil.h"
#include "LWM2MSensorDeviceGyroscope.h"
#include "LWM2M.h"
#include "MbedTLSAdapter.h"

#include <Serval_Network.h>
#include <Serval_Log.h>
#include <Serval_Timer.h>
#include <Serval_CoapClient.h>
#include <Serval_Clock.h>
#include <Serval_Scheduler.h>
#include <Serval_Lwm2m.h>
#include <Serval_OpControl.h>

#include "FreeRTOS.h"
#include "timers.h"
#include "semphr.h"

/* constant definitions ***************************************************** */

#define LWM2M_REPORTING_SUPPORT_SUSPEND                 1

#if SERVAL_ENABLE_DTLS_SESSION_ID
#define LWM2M_MAX_DTLS_SESSION_ID_SIZE                  32
#else /* SERVAL_ENABLE_DTLS_SESSION_ID */
#define LWM2M_MAX_DTLS_SESSION_ID_SIZE                   0
#endif /* SERVAL_ENABLE_DTLS_SESSION_ID */

#define SYSTEM_TIMER_INTERVAL_IN_MS                     UINT16_C(1000)

#define REBOOT_STATE_INIT                               UINT8_C(0) /**< Initial state for reboot */
#define REBOOT_STATE_START                              UINT8_C(1) /**< reboot started, stop registration handling, wait sometime for the ACK to be sent */
#define REBOOT_STATE_EXECUTE                            UINT8_C(5) /**< execute reboot, gracefully, set serval to sleep (stores MID/Token, if possible) */
#define REBOOT_STATE_EMERGENCY                          UINT8_C(15) /**< emergency reboot, if graceful reboot could not be done in time */

#define LWM2M_REGISTRATION_SCHEDULED                    1  /**< calls to Lwm2mRegistration_update/registration from serval scheduler */
#define LWM2M_REGISTRATION_UPDATE_INTERVAL(LIFETIME)    ((LIFETIME * 4 )/ 5) /**< calculate update interval from lifetime */

#define REBOOT_MSG_DELAY_IN_MS                          UINT8_C(100) /**< last sleep on reboot to give logging a chance */

#define DNS_REPEAT_TIME_IN_S                            UINT16_C(60 * 5) /**< repeat DNS lookup after 5 min (after tests => 12 h) */
#define MAX_SERVAL_SCHEDULER_TRIES                      UINT8_C(20)  /**< maximum number of TimeChanged calls with pending registration callback before LED alert */

#define RED_LED_HOLD_TIME_IN_MS                         UINT32_C(5000) /**< time in milliseconds to stick at least on the value set by \link LWM2M_RedLedSetState \endlink */
#define ORANGE_LED_HOLD_TIME_IN_MS                      UINT32_C(5000) /**< time in milliseconds to stick at least on the value set by \link LWM2M_OrangeLedSetState \endlink */
#define YELLOW_LED_HOLD_TIME_IN_MS                      UINT32_C(2000) /**< time in milliseconds to stick at least on the value set by \link LWM2M_YellowLedSetState \endlink */

#define LED_MUTEX_TIMEOUT                               (100/(portTICK_PERIOD_MS)) /**< timeout for LED mutex functions */

#define MILLIS_TO_SEC(MS)                               ((MS)/UINT32_C(1000)) /**< convert milliseconds to seconds */

/**< Macro for the non secure serval stack expected LWM2M URL format */
#define LWM2M_URL_FORMAT_NON_SECURE          "coap://%s:%d"

/**< Macro for the secure serval stack expected LWM2M URL format */
#define LWM2M_URL_FORMAT_SECURE              "coaps://%s:%d"

/**< Macro for the number of LWM2M servers */
#define LWM2M_NUMBER_OF_SERVERS                    UINT32_C(1)

/**< Macro for the LWM2M server index which can take any value between 0 to (LWM2M_NUMBER_OF_SERVERS -1 ).
 * Here number of servers is configured as 1, hence to get memory of that server, the index value is passed as 0 */
#define LWM2M_SERVER_INDEX                         UINT32_C(0)

/**< Macro for the time interval to send notification to the server about the system time change */
#define LWM2M_TIME_CHANGE_NOTIFICATION_PERIOD      UINT32_C(5000)

/**< Macro for the software timer start or stop timeout */
#define LWM2M_TIMER_START_OR_STOP_TIMEOUT          UINT32_C(1000)

/**< Macro for the maximum retry count of LWM2M registration update */
#define LWM2M_REG_UPDATE_RETRY_MAX                 UINT8_C(10)

/* local variables ********************************************************** */

/**
 * @brief Enum to represent the server registration state
 */
enum ServerRegistrationState_E
{
    SRS_NONE,
    SRS_ENABLED,
    SRS_RETRY_REGISTRATION,
    SRS_REGISTRATION_PENDING,
    SRS_REGISTRATED,
    SRS_UPDATE_PENDING,
    SRS_DEREGISTRATION_PENDING,
    SRS_SEND_FAILED,
    SRS_REGISTRATION_TIMEOUT,
    SRS_UPDATE_TIMEOUT,
};

/**
 * @brief Typedef to represent the server registration state
 */
typedef enum ServerRegistrationState_E ServerRegistrationState_T;

/**
 * @brief Enum to represent the DNS state
 */
enum DnsState_E
{
    DNS_NONE,
    DNS_HOST,
    DNS_PENDING,
    DNS_RESOLVED,
    DNS_ERROR
};

/**
 * @brief Enum representing the registration session state.
 *
 */
enum RegistrationSessionState_E
{
    NO_SESSION,
    SAME_SESSION,
    NEW_SESSION,
    LOST_SESSION,
    CHANGED_SESSION,
};

/**
 * @brief typedef  to represent the registration session state.
 *
 */
typedef enum RegistrationSessionState_E RegistrationSessionState_T;

/**
 * @brief Typedef to represent the DNS state
 */
typedef enum DnsState_E DnsState_T;

/**
 * @brief Typedef to represent the registration information
 */
struct RegistrationInfo_S
{
    ServerRegistrationState_T state;
    DnsState_T dnsState;
    uint32_t lastRegistrationLifetime;
    uint32_t lastRegistrationSent;
    uint32_t lastRegistrationMessage;
    uint32_t lastDns;
    Callable_T refreshCallable;
    char destination[64];
    char destinationResolved[64];
#if LWM2M_MAX_DTLS_SESSION_ID_SIZE
    uint8_t session[LWM2M_MAX_DTLS_SESSION_ID_SIZE + 1];
#endif /* LWM2M_MAX_DTLS_SESSION_ID_SIZE */
    bool retry;
};

typedef struct RegistrationInfo_S RegistrationInfo_T;

/* @todo - Provide this from the BSP layer. */
typedef enum BSP_LED_Command_E BSP_LED_Command_T;

/**< LWM2M server URL. The size is fixed as 80U considering the overhead of protocol, URL string (65) and port number */
static char LWM2MServerURL[80] = { 0 };

/**< LWM2M setup information */
static LWM2M_Setup_T LWM2MSetup;

static uint32_t LWM2MRedLedHandle = (uint32_t) BSP_XDK_LED_R; /**< variable to store red led handle */
static uint32_t LWM2MOrangeLedHandle = (uint32_t) BSP_XDK_LED_O; /**< variable to store orange led handle */
static uint32_t LWM2MYellowLedHandle = (uint32_t) BSP_XDK_LED_Y; /**< variable to store yellow led handle */

static LWM2M_LedStateChangeHandler_T LWM2MRedLedStateChangeHandler = NULL; /**< led state change handler to report changes of the red LED */
static LWM2M_LedStateChangeHandler_T LWM2MOrangeLedStateChangeHandler = NULL; /**< led state change handler to report changes of the orange LED */
static LWM2M_LedStateChangeHandler_T LWM2MYellowLedStateChangeHandler = NULL; /**< led state change handler to report changes of the yellow LED */
static volatile uint64_t LWM2MLastSetRed = 0; /**< system time in milliseconds of the last set by \link LWM2M_RedLedSetState \endlink */
static volatile uint64_t LWM2MLastSetOrange = 0; /**< system time in milliseconds of the last set by \link LWM2M_OrangeLedSetState \endlink */
static volatile uint64_t LWM2MLastSetYellow = 0; /**< system time in milliseconds of the last set by \link LWM2M_YellowLedSetState \endlink */
static bool LWM2MUseLedsForRegistration = false; /**< indicate, to use LEDs for registration interface */

static RegistrationInfo_T ServerRegistrationInfo[LWM2M_MAX_NUMBER_SERVERS];

/** Reboot task for serval scheduler */
static Callable_T RebootCallable;

/** variable to store timer handle for time changed*/
static xTimerHandle TimeChangeTimer_ptr = NULL;

/** Mutex for LED timing */
static xSemaphoreHandle LedMutexHandle;

/** start indicator. Used for fast registration update to validate proper registration. */
static volatile uint8_t Started = 0;
static volatile uint8_t RebootState = REBOOT_STATE_INIT; /**< reboot state. */

static CmdProcessor_T FotaCmdProcessor; /**< FOTA Command Processor Handle */

/* local functions ********************************************************** */

LWM2M_Setup_T * LWM2MGetCredentials(void)
{
    return &LWM2MSetup;
}

static Retcode_T InitFotaStateMachine(void)
{
    /* @todo - Check if LWM2M_Setup_S CmdProcessorHandle can be reused without affecting the performance much. */
    Retcode_T ReturnValue = CmdProcessor_Initialize(&FotaCmdProcessor, (char*) "FotaCmdProcessor", TASK_PRIO_FOTA_CMD_PROCESSOR, TASK_STACK_SIZE_FOTA_CMD_PROCESSOR, TASK_Q_LEN_FOTA_CMD_PROCESSOR);

    if (RETCODE_OK == ReturnValue)
    {
        ReturnValue = LWM2MObjectFirmwareUpdate_Init(&FotaCmdProcessor);
    }

    return ReturnValue;
}

/**
 * @brief This function adjusts the last set led time
 *
 * @param[in/out] ledTime
 * time in milliseconds
 */
static void AdjustLastSetLedTime(volatile uint64_t* ledTime)
{
    uint64_t now;
    if (RC_OK == Clock_getTimeMillis(&now))
    {
        if (pdTRUE == xSemaphoreTake(LedMutexHandle, LED_MUTEX_TIMEOUT))
        {
            *ledTime = now;
            if (pdTRUE != xSemaphoreGive(LedMutexHandle))
            {
                assert(false);
            }
        }
    }
}

/**
 * @brief This function checks the last set led time
 *
 * @param[in/out] ledTime
 * time in milliseconds
 *
 * @param[in] now
 * time in milliseconds
 *
 * @param[in] timeout
 * timeout
 *
 * @return true if successful
 *         false for failure
 */
static bool CheckLastSetLedTime(volatile uint64_t* ledTime, uint64_t now, uint32_t timeout)
{
    uint64_t currentLedTime = 0;
    if (pdTRUE == xSemaphoreTake(LedMutexHandle, LED_MUTEX_TIMEOUT))
    {
        currentLedTime = (*ledTime);
        if (pdTRUE != xSemaphoreGive(LedMutexHandle))
        {
            assert(false);
        }
    }
    return (currentLedTime + timeout) < now;
}

/**
 * @brief Get registration state name. Usually used by logging.
 *
 * @param[in] state
 * registration state
 *
 * @return name of registration state
 */
static const char* GetRegistrationStateName(ServerRegistrationState_T state)
{
    switch (state)
    {
    case SRS_NONE:
        return "NONE";
    case SRS_ENABLED:
        return "ENABLED";
    case SRS_RETRY_REGISTRATION:
        return "RETRY REGISTER";
    case SRS_REGISTRATION_PENDING:
        return "REGISTER PENDING";
    case SRS_REGISTRATED:
        return "REGISTERED";
    case SRS_UPDATE_PENDING:
        return "UPDATE PENDING";
    case SRS_DEREGISTRATION_PENDING:
        return "DEREG PENDING";
    case SRS_SEND_FAILED:
        return "SEND FAILED";
    case SRS_REGISTRATION_TIMEOUT:
        return "REGISTER TIMEOUT";
    case SRS_UPDATE_TIMEOUT:
        return "UPDATE TIMEOUT";
    }
    return "?!?";
}

/**
 * @brief Get DNS state name. Usually used by logging.
 *
 * @param[in] state
 * DNS state
 *
 * @return name of DNS state
 */
static const char* GetDnsStateName(DnsState_T state)
{
    switch (state)
    {
    case DNS_NONE:
        return "";
    case DNS_HOST:
        return "DNS HOST ";
    case DNS_PENDING:
        return "DNS PENDING ";
    case DNS_RESOLVED:
        return "DNS RESOLVED ";
    case DNS_ERROR:
        return "DNS ERROR ";
    }
    return "?!?";
}

/**
 * @brief Get server index by registration info.
 *
 * @param[in] info
 * registration info
 *
 * @return server index, LWM2M_MAX_NUMBER_SERVERS
 * if no server index for provided server info was not found.
 */
static uint8_t GetServerIdxByInfo(RegistrationInfo_T * info)
{
    uint8_t ServerIdx;

    for (ServerIdx = 0; ServerIdx < LWM2M_MAX_NUMBER_SERVERS; ++ServerIdx)
    {
        if (info == &ServerRegistrationInfo[ServerIdx])
        {
            break;
        }
    }

    return ServerIdx;
}

/**
 * @brief Set led state.
 *
 * @param[in] handle
 * handle to LED
 *
 * @param[in] operation
 * LED operation such as ON , OFF etc
 *
 * @param[in] handler
 * led state change handler
 */
static void LWM2MSetLedState(uint32_t handle, BSP_LED_Command_T operation, LWM2M_LedStateChangeHandler_T handler)
{
    Retcode_T retcode = RETCODE_OK;
    /*At present the handle can either be BSP_XDK_LED_R ,BSP_XDK_LED_O , BSP_XDK_LED_Y */
    if (handle >= (uint32_t) BSP_XDK_LED_ONBOARD_MAX)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);

    }
    else
    {
        retcode = BSP_LED_Switch(handle, (uint32_t) operation);
        if (RETCODE_OK == retcode)
        {
            if (NULL != handler)
            {
                switch (operation)
                {
                case BSP_LED_COMMAND_OFF:
                    handler(false);
                    break;
                case BSP_LED_COMMAND_ON:
                    handler(true);
                    break;
                default:
                    retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
                    break;
                }
            }
        }
    }
    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseError(retcode);
    }
}

/**
 * @brief Adjust LEDs according registration state.
 *
 * @param[in] serverIdx
 * server index.
 *
 * @note Currently only index 0 is supported.
 */
static void Interface_RegisterStateToLed(uint8_t serverIdx)
{
    if (LWM2MUseLedsForRegistration && (0 == serverIdx))
    {
        switch (ServerRegistrationInfo[serverIdx].state)
        {
        case SRS_NONE:
            LWM2MSetLedState(LWM2MRedLedHandle, BSP_LED_COMMAND_OFF, LWM2MRedLedStateChangeHandler);
            LWM2MSetLedState(LWM2MOrangeLedHandle, BSP_LED_COMMAND_OFF, LWM2MOrangeLedStateChangeHandler);
            break;
        case SRS_ENABLED:
            LWM2MSetLedState(LWM2MRedLedHandle, BSP_LED_COMMAND_ON, LWM2MRedLedStateChangeHandler);
            LWM2MSetLedState(LWM2MOrangeLedHandle, BSP_LED_COMMAND_OFF, LWM2MOrangeLedStateChangeHandler);
            break;
        case SRS_REGISTRATED:
            LWM2MSetLedState(LWM2MRedLedHandle, BSP_LED_COMMAND_OFF, LWM2MRedLedStateChangeHandler);
            LWM2MSetLedState(LWM2MOrangeLedHandle, BSP_LED_COMMAND_ON, LWM2MOrangeLedStateChangeHandler);
            break;
        default:
            break;
        }
    }
}

#if LWM2M_MAX_DTLS_SESSION_ID_SIZE

static RegistrationSessionState_T RegistrationUpdateSession(uint8_t ServerIdx)
{
    uint8_t session[LWM2M_MAX_DTLS_SESSION_ID_SIZE + 1] =
    {   LWM2M_MAX_DTLS_SESSION_ID_SIZE};
    retcode_t rc = Lwm2mRegistration_getSecureConnId(ServerIdx, &session[0], &session[1]);
    RegistrationSessionState_T result = NO_SESSION;

    if (RC_OK != rc)
    {
        session[0] = 0;
    }

    if ((0 < ServerRegistrationInfo[ServerIdx].session[0]) && (0 < session[0]))
    {
        if (0 == memcmp(ServerRegistrationInfo[ServerIdx].session, session, session[0]))
        {
            result = SAME_SESSION;
        }
        else
        {
            memset(ServerRegistrationInfo[ServerIdx].session, 0, sizeof(ServerRegistrationInfo[ServerIdx].session));
            memcpy(ServerRegistrationInfo[ServerIdx].session, session, session[0] + 1);
            result = CHANGED_SESSION;
        }
    }
    else if (0 < ServerRegistrationInfo[ServerIdx].session[0])
    {
        memset(ServerRegistrationInfo[ServerIdx].session, 0, sizeof(ServerRegistrationInfo[ServerIdx].session));
        result = LOST_SESSION;
    }
    else if (0 < session[0])
    {
        memset(ServerRegistrationInfo[ServerIdx].session, 0, sizeof(ServerRegistrationInfo[ServerIdx].session));
        memcpy(ServerRegistrationInfo[ServerIdx].session, session, session[0] + 1);
        result = NEW_SESSION;
    }
    else
    {
        result = NO_SESSION;
    }
    return result;
}
#endif /* LWM2M_MAX_DTLS_SESSION_ID_SIZE */

/**
 * @brief Update registration state. Set state and adjust lastRegistrationMessage and lastRegistrationLifetime
 * according the current values. After startup only called within serval scheduler.
 *
 * @param[in] serverIdx
 * server index
 *
 * @param[in] state
 * server state do be set.
 */
static void UpdateRegistrationState(uint8_t serverIdx, enum ServerRegistrationState_E state)
{
    Lwm2mServer_T* Server = Lwm2m_getServer(serverIdx);
    RegistrationInfo_T * Info = &ServerRegistrationInfo[serverIdx];
    if (NULL != Server && NULL != Info)
    {
#if LWM2M_REPORTING_SUPPORT_SUSPEND
        bool changed = false;
#endif /* LWM2M_REPORTING_SUPPORT_SUSPEND */
        bool Failed = (SRS_SEND_FAILED == state);
        bool Pending = (SRS_UPDATE_PENDING == state) || (SRS_REGISTRATION_PENDING == state);

        if (Failed)
        {
            /* SRS_SEND_FAILED is only used to trigger time updates */
            Clock_getTime(&Info->lastRegistrationSent); //lint !e534, ignore return value
            Info->lastRegistrationMessage = Info->lastRegistrationSent;
            Info->lastRegistrationLifetime = Server->lifetime;
            state = SRS_ENABLED;
        }
#if LWM2M_REPORTING_SUPPORT_SUSPEND
        if (SRS_REGISTRATED == state)
        {
            changed = ((SRS_REGISTRATED != Info->state) && (SRS_UPDATE_PENDING != Info->state));
        }
        else if (SRS_ENABLED == state)
        {
            changed = (SRS_ENABLED != Info->state && SRS_REGISTRATION_PENDING != Info->state);
        }
#endif /* LWM2M_REPORTING_SUPPORT_SUSPEND */
        Info->state = state;
        if (Pending)
        {
            Clock_getTime(&Info->lastRegistrationSent); //lint !e534, ignore return value
            Info->lastRegistrationLifetime = Server->lifetime;
        }
        else
        {
            if (SRS_ENABLED == state)
            {
                Info->retry = false;
                if (DNS_RESOLVED == ServerRegistrationInfo[serverIdx].dnsState)
                {
                    /** reset DNS in case of changed address caused failure */
                    ServerRegistrationInfo[serverIdx].dnsState = DNS_HOST;
                }
#if LWM2M_REPORTING_SUPPORT_SUSPEND
                if (changed)
                {
                    Lwm2mReporting_suspendNotifications(serverIdx);
                }
#endif /* LWM2M_REPORTING_SUPPORT_SUSPEND */
            }
            else if (SRS_REGISTRATED == state)
            {
                uint32_t now;

                Clock_getTime(&now); //lint !e534, ignore return value
                Info->retry = false;
                Info->lastRegistrationMessage = Info->lastRegistrationSent;
                printf("Registration in %u [s]\n", (unsigned int) (now - Info->lastRegistrationSent));

#if LWM2M_REPORTING_SUPPORT_SUSPEND
                if (changed)
                {
                    Lwm2mReporting_resumeNotifications(serverIdx, LWM2M_SEND_NOTIFICATIONS);
                }
#endif /* LWM2M_REPORTING_SUPPORT_SUSPEND */
#if LWM2M_MAX_DTLS_SESSION_ID_SIZE
                RegistrationUpdateSession(serverIdx); //lint !e534, ignore return value
#endif /* LWM2M_MAX_DTLS_SESSION_ID_SIZE */
            }
            else if (SRS_RETRY_REGISTRATION == state)
            {
                Info->retry = true;
            }
            Interface_RegisterStateToLed(serverIdx);
        }
    }
}

/**
 * @brief Check and send registration interface request.
 *        Intended to be called by serval scheduler. May be called also from timer (experimental), depending on
 * \link LWM2M_REGISTRATION_SCHEDULED \endlink.
 *
 * @param[in] callable
 * serval callable
 *
 * @param[in] status
 * status
 *
 * @return RC_OK, if sent successful or if no request is required.
 *                Otherwise a error code from sending the request is returned.
 */
static retcode_t SendRegistration(Callable_T * callable, retcode_t status)
{
    (void) status;
    static int Counter = 0; //lint !e956, only called by serval scheduler

    retcode_t rc = RC_OK;
    RegistrationSessionState_T sessionState = NO_SESSION;
    /*lint -e(413) -e(530) -e(78) -e(40) suppresing since this is due to the implementation of servalstack*/
    RegistrationInfo_T* Info = CALLABLE_GET_CONTEXT(RegistrationInfo_T, refreshCallable, callable);
    uint8_t ServerIdx = GetServerIdxByInfo(Info);

#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
    /**return the remaining stack */
    uint32_t HighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    printf("scheduler remaining stack = %lu\n\r", HighWaterMark);
#endif /* (INCLUDE_uxTaskGetStackHighWaterMark == 1) */

    ++Counter;

#if LWM2M_MAX_DTLS_SESSION_ID_SIZE
    sessionState = RegistrationUpdateSession(ServerIdx);
    if ((SAME_SESSION < sessionState) && (SRS_REGISTRATED == Info->state))
    {
        printf("==> DTLS session changed, registration forced\r\n");
        Info->state = SRS_ENABLED;
    }
#endif /* LWM2M_MAX_DTLS_SESSION_ID_SIZE */

    switch (Info->state)
    {
    case SRS_REGISTRATED:
        printf("==> registration update\r\n");
        rc = Lwm2mRegistration_update(ServerIdx);
        if (RC_OK == rc)
        {
            UpdateRegistrationState(ServerIdx, SRS_UPDATE_PENDING);
        }
        break;
    case SRS_RETRY_REGISTRATION:
        case SRS_ENABLED:
        printf("==> registration retry\r\n");
        rc = Lwm2mRegistration_register(ServerIdx);
        if (RC_OK == rc)
        {
            UpdateRegistrationState(ServerIdx, SRS_REGISTRATION_PENDING);
        }
        break;
    case SRS_REGISTRATION_TIMEOUT:
        if (!Info->retry && SAME_SESSION == sessionState)
        {
#if SERVAL_ENABLE_DTLS
            /* retry with new DTLS session */
            /*lint -e(534) todo:Need to check on this time being suppressing*/
            Lwm2mRegistration_closeSecureConn(ServerIdx);
#endif /* SERVAL_ENABLE_DTLS */
            UpdateRegistrationState(ServerIdx, SRS_RETRY_REGISTRATION);
            printf("==> registration timeout, retry\r\n");
        }
        else
        {
#if !SERVAL_ENABLE_DTLS_SESSION_ID
#if SERVAL_ENABLE_DTLS
            /*lint -e(534) todo:Need to check on this time being suppressing*/
            Lwm2mRegistration_closeSecureConn(ServerIdx);
#endif /* SERVAL_ENABLE_DTLS */
#endif /* !SERVAL_ENABLE_DTLS_SESSION_ID */
            printf("==> registration timeout\r\n");
            UpdateRegistrationState(ServerIdx, SRS_ENABLED);
        }
        Callable_clear(callable);
        return RC_OK;
    case SRS_UPDATE_TIMEOUT:
        if (!Info->retry && SAME_SESSION == sessionState)
        {
#if SERVAL_ENABLE_DTLS
            /* retry with new DTLS session => trigger register instead of update to ensure new observes */
            /*lint -e(534) todo:Need to check on this time being suppressing*/
            Lwm2mRegistration_closeSecureConn(ServerIdx);
#endif /* SERVAL_ENABLE_DTLS */
            UpdateRegistrationState(ServerIdx, SRS_RETRY_REGISTRATION);
            printf("==> registration update timeout, retry register\r\n");
        }
        else
        {
            printf("==> registration update timeout\r\n");
            UpdateRegistrationState(ServerIdx, SRS_ENABLED);
        }
        Callable_clear(callable);
        return RC_OK;
    default:
        break;
    }

    if (RC_OK == rc)
    {
        printf("==> Waiting for successful registration %d!\r\n", Counter);
    }
    else
    {
        uint32_t now = 0;
        if (RC_COAP_CLIENT_SESSION_ALREADY_ACTIVE == rc)
        {
            printf("==> BUSY, send registration pending %d!\r\n", Counter);
        }
        else if (RC_COAP_SECURE_CONNECTION_ERROR == rc)
        {
            printf("==> send registration %d, secure error or handshake pending!\r\n", Counter);
        }
        else
        {
            printf("==> send registration failed %d " RC_RESOLVE_FORMAT_STR "\n", Counter, RC_RESOLVE((int)rc));
        }
        if (RC_OK == Clock_getTime(&now))
        {
            uint32_t last = Info->retry ? Info->lastRegistrationSent : Info->lastRegistrationMessage;
            if ((last + Info->lastRegistrationLifetime) < now)
            {
                printf("==> Registration expired %d!\r\n", Counter);
                /*lint -e(534) todo:Need to check on this time being suppressing*/
#if SERVAL_ENABLE_DTLS
                Lwm2mRegistration_closeSecureConn(ServerIdx);
#endif /* SERVAL_ENABLE_DTLS */
                UpdateRegistrationState(ServerIdx, SRS_SEND_FAILED);
            }
        }
    }
    Callable_clear(callable);

    return rc;
}

/**
 * @brief Callback from asynchrony DNS.
 *
 * @param[in] rc
 * result code of DNS lookup.
 *
 * @param[in] resolvedURL
 * resolved URL, if DNS lookup was successful.
 *
 * @param[in] context
 * context pointer.
 */
static void DnsCallback(retcode_t rc, const char* resolvedURL, void * context)
{
    RegistrationInfo_T * Info = context;
    uint8_t ServerIdx = GetServerIdxByInfo(Info);
    Lwm2mServer_T* Server = Lwm2m_getServer(ServerIdx);

    if (RC_OK != rc || sizeof(Server->serverAddress) <= strlen(resolvedURL))
    {
        memset(Server->serverAddress, 0, sizeof(Server->serverAddress));
        Info->dnsState = DNS_ERROR;
        printf("LWM2M server resolve error " RC_RESOLVE_FORMAT_STR "\n", RC_RESOLVE((int)rc));
    }
    else
    {
        strcpy(Server->serverAddress, resolvedURL);
        Info->dnsState = DNS_RESOLVED;
        printf("LWM2M server resolved: %s\r\n", Server->serverAddress);
    }
}

/**
 * @brief Scheduler callback for reboot task.
 * Calls OpControl_trySleep and, if not RC_RETRY_SLEEP_LATER, calls rebootNow.
 *
 * @param callable_ptr
 * Unused
 *
 * @param status
 * Unused
 *
 * @return RC_OK in case of success and error code otherwise.
 *
 */
static retcode_t CallRebootNow(Callable_T* callable_ptr, retcode_t status)
{
    (void) callable_ptr;
    (void) status;
    retcode_t rc = OpControl_trySleep();
    if (RC_RETRY_SLEEP_LATER != rc)
    {
        LWM2M_RebootNow("REBOOTING ...");
    }
    return RC_OK;
}

/**
 * @brief Timer callback to handler registration interface.
 *
 * @param[in] serverIdx
 * server index
 *
 * @param[in] timeMillis
 * system time in milliseconds
 */
static void RegistrationTimer(uint8_t serverIdx, uint64_t timeMillis)
{
    static uint8_t AlertCounter = 0; //lint !e956, only called by timer
    uint32_t Time = MILLIS_TO_SEC(timeMillis);
    Lwm2mServer_T* Server = Lwm2m_getServer(serverIdx);
    RegistrationInfo_T* Info = &ServerRegistrationInfo[serverIdx];
    uint32_t Elapsed = Time - Info->lastRegistrationMessage;
    bool Log = true;
    bool scheduleRegistration = (SRS_REGISTRATION_TIMEOUT == Info->state) || (SRS_UPDATE_TIMEOUT == Info->state) || (SRS_RETRY_REGISTRATION == Info->state);

    if (!scheduleRegistration)
    {
        bool RefreshRegistration = (Info->lastRegistrationLifetime != Server->lifetime);

        if (!RefreshRegistration)
        {
            uint32_t Interval = LWM2M_REGISTRATION_UPDATE_INTERVAL(Info->lastRegistrationLifetime);
            if (0 == Started && SRS_ENABLED == Info->state)
            {
                /* initial register */
                Interval = 0;
                Started = 1;
            }
            else if (1 == Started && SRS_REGISTRATED == Info->state && Interval > 10)
            {
                /* fast register update after first registration since restart */
                Interval = 10;
            }
            RefreshRegistration = (Interval <= Elapsed);
        }

        if (RefreshRegistration)
        {
            if (SRS_REGISTRATED == Info->state || SRS_ENABLED == Info->state)
            {
                if (DNS_NONE != Info->dnsState)
                {
                    if (DNS_HOST != Info->dnsState && Time > (Info->lastDns + DNS_REPEAT_TIME_IN_S))
                    {
                        /* DNs retry */
                        Info->dnsState = DNS_HOST;
                    }
                    if (DNS_HOST == Info->dnsState)
                    {
                        retcode_t rc = LWM2MDnsUtil_ExtendedResolveHostname(Info->destination, Info->destinationResolved, sizeof(Info->destinationResolved), DNS_TIMEOUT_IN_S, DnsCallback, Info);
                        if (RC_DNS_OK != rc)
                        {
                            if (RC_DNS_PENDING == rc)
                            {
                                Info->lastDns = Time;
                                Info->dnsState = DNS_PENDING;
                                printf("LWM2M server resolve started: %s\r\n", Info->destination);
                            }
                            else if (RC_DNS_BUSY == rc)
                            {
                                printf("LWM2M server resolve pending: %s\r\n", Info->destination);
                            }
                            else
                            {
                                Info->lastDns = Time;
                                Info->dnsState = DNS_ERROR;
                                printf("LWM2M server not resolved: %s\r\n", Info->destination);
                            }
                        }
                        else
                        {
                            printf("LWM2M server resolved: %s\r\n", Info->destinationResolved);
                            strncpy(Server->serverAddress, Info->destinationResolved, sizeof(Server->serverAddress) - 1);
                            if (sizeof(Server->serverAddress) <= strlen(Info->destinationResolved))
                            {
                                printf("Resolved LWM2M server truncated to '%s'\n", Server->serverAddress);
                            }
                            Info->lastDns = Time;
                            Info->dnsState = DNS_RESOLVED;
                        }
                        Log = false;
                    }
                    RefreshRegistration = (DNS_RESOLVED == Info->dnsState);
                }
                scheduleRegistration = RefreshRegistration;
            }
        }
    }

    if (scheduleRegistration)
    {
        if (!Callable_isAssigned(&Info->refreshCallable))
        {
            printf("Try refresh registration %s (%u sec)\r\n", GetRegistrationStateName(Info->state), (unsigned) Elapsed);
            (void) Callable_assign(&Info->refreshCallable, SendRegistration);
#if LWM2M_REGISTRATION_SCHEDULED
            if (RC_OK != Scheduler_enqueue(&Info->refreshCallable, RC_OK))
            {
                Callable_clear(&Info->refreshCallable);
            }
#else /* LWM2M_REGISTRATION_SCHEDULED */
            Callable_call(&Info->refreshCallable, RC_OK);
#endif /* LWM2M_REGISTRATION_SCHEDULED */
            Log = false;
            AlertCounter = 0;
        }
        else
        {
            if (SRS_REGISTRATED == Info->state && Elapsed > Info->lastRegistrationLifetime)
            {
                Info->state = SRS_ENABLED;
            }
            if (MAX_SERVAL_SCHEDULER_TRIES > AlertCounter)
            {
                ++AlertCounter;
            }
        }
    }
    else if (!Callable_isAssigned(&Info->refreshCallable))
    {
        AlertCounter = 0;
    }

    if (Log && (0 == (Elapsed % 20)))
    {
        printf("   %s (since %u sec, lifetime %u sec)\r\n", GetRegistrationStateName(Info->state), (unsigned) Elapsed, (unsigned) Info->lastRegistrationLifetime);
        if (DNS_NONE != Info->dnsState)
        {
            printf("   %s(since %u sec)\r\n", GetDnsStateName(Info->dnsState), (unsigned) (Time - Info->lastDns));
        }
        if (Callable_isAssigned(&Info->refreshCallable))
        {
            printf("   scheduled registration task pending!\r\n");
        }
    }
    if (LWM2MUseLedsForRegistration && (0 == serverIdx))
    {
        uint32_t Operation = (Elapsed & 1) ? (uint32_t) BSP_LED_COMMAND_OFF : (uint32_t) BSP_LED_COMMAND_ON;
        switch (Info->state)
        {
        case SRS_ENABLED:
            if (MAX_SERVAL_SCHEDULER_TRIES <= AlertCounter)
            {
                /* serval scheduler seems to be blocked */
                if (CheckLastSetLedTime(&LWM2MLastSetRed, timeMillis, RED_LED_HOLD_TIME_IN_MS)
                        && CheckLastSetLedTime(&LWM2MLastSetOrange, timeMillis, ORANGE_LED_HOLD_TIME_IN_MS))
                {
                    LWM2MSetLedState(LWM2MRedLedHandle, (BSP_LED_Command_T) Operation, LWM2MRedLedStateChangeHandler);
                    LWM2MSetLedState(LWM2MOrangeLedHandle, (BSP_LED_Command_T) Operation, LWM2MOrangeLedStateChangeHandler);
                }
            }
            break;
        case SRS_REGISTRATION_PENDING:
            if (CheckLastSetLedTime(&LWM2MLastSetRed, timeMillis, RED_LED_HOLD_TIME_IN_MS))
            {
                LWM2MSetLedState(LWM2MRedLedHandle, (BSP_LED_Command_T) Operation, LWM2MRedLedStateChangeHandler);
            }
            break;
        case SRS_UPDATE_PENDING:
            if (CheckLastSetLedTime(&LWM2MLastSetOrange, timeMillis, ORANGE_LED_HOLD_TIME_IN_MS))
            {
                LWM2MSetLedState(LWM2MOrangeLedHandle, (BSP_LED_Command_T) Operation, LWM2MOrangeLedStateChangeHandler);
            }
            break;
        default:
            break;
        }
        if (CheckLastSetLedTime(&LWM2MLastSetYellow, timeMillis, YELLOW_LED_HOLD_TIME_IN_MS))
        {
            LWM2MSetLedState(LWM2MYellowLedHandle, (BSP_LED_Command_T) Operation, LWM2MYellowLedStateChangeHandler);
        }
    }
}

/**
 * @brief Handler to check the reboot state machine and the registration interface.
 *
 * @param[in] param1
 * Unused
 *
 * @param[in] data
 * Unused
 */
static void TimeChangedHandler(void * param1, uint32_t data)
{
    BCDS_UNUSED(param1);
    BCDS_UNUSED(data);

    retcode_t rc = RC_OK;
    uint64_t TimeMillis = 0;
    static uint64_t LastTimeMillis = 0;
    if (REBOOT_STATE_INIT < RebootState)
    {
        // device reboots
        ++RebootState;
        if (REBOOT_STATE_EMERGENCY <= RebootState)
        {
            LWM2M_RebootNow("REBOOT timeout => emergency!");
        }
        else if (REBOOT_STATE_EXECUTE <= RebootState)
        {
#if (SERVAL_LOG_LEVEL >= SERVAL_LOG_LEVEL_INFO)
            printf("REBOOT execute %u", RebootState);
#else /* (SERVAL_LOG_LEVEL >= SERVAL_LOG_LEVEL_INFO) */
            printf("REBOOT execute %u\n", RebootState);
#endif /* (SERVAL_LOG_LEVEL >= SERVAL_LOG_LEVEL_INFO) */
            rc = Scheduler_enqueue(&RebootCallable, RC_OK);
            if (RC_OK != rc)
            {
                printf("REBOOT execute %u failed " RC_RESOLVE_FORMAT_STR, RebootState, RC_RESOLVE((int)rc));
            }
        }
        return;
    }
    rc = Clock_getTimeMillis(&TimeMillis);
    if (RC_OK != rc)
    {
        printf("Clock_getTime failed! " RC_RESOLVE_FORMAT_STR "\n", RC_RESOLVE((int)rc));
    }

    if (0 < LastTimeMillis)
    {
        LastTimeMillis = TimeMillis - LastTimeMillis;
        if (SYSTEM_TIMER_INTERVAL_IN_MS < LastTimeMillis)
        {
            LastTimeMillis -= SYSTEM_TIMER_INTERVAL_IN_MS;
            if ((SYSTEM_TIMER_INTERVAL_IN_MS >> 2) < LastTimeMillis)
            {
                printf("Timer delayed %lu ms\r\n", (unsigned long) LastTimeMillis);
            }
        }
    }
    LastTimeMillis = TimeMillis;
    /* though the callback doesn't provide the server info, only one server is supported! */
    RegistrationTimer(0, TimeMillis);
    /* notify system time change after registration */
    LWM2MObjectDevice_NotifyTimeChanged();

}

/**
 * @brief The function is to get the registration status of the Device while connection to LWM2M Server
 * and It will toggle the Orange LED to indicate the Registration Success State and Red LED will indicate the Registration Failure state.
 *
 * @param[in] serverIdx
 * server index
 *
 * param[in] status
 * registration status
 */
static void RegistrationCallback(uint8_t serverIdx, retcode_t status)
{
    static bool fotaStateMachineRunOnceFlag = true;
    Retcode_T ReturnValue = RETCODE_OK;
    if (status == RC_OK)
    {
        UpdateRegistrationState(serverIdx, SRS_REGISTRATED);
        printf("Registration succeeded\r\n");
        if (fotaStateMachineRunOnceFlag)
        {
            ReturnValue = LWM2MObjectFirmwareUpdate_Enable();

            if (RETCODE_OK == ReturnValue)
            {
                fotaStateMachineRunOnceFlag = false;
                printf("FOTA state machine run was successful \r\n");
            }
            else
            {
                Retcode_RaiseError(ReturnValue);
                fotaStateMachineRunOnceFlag = true;
                printf("FOTA state machine run failed \r\n");
            }
        }

    }
    else
    {
        if (RC_COAP_REQUEST_TIMEOUT == status)
        {
            UpdateRegistrationState(serverIdx, SRS_REGISTRATION_TIMEOUT);
        }
        else
        {
            UpdateRegistrationState(serverIdx, SRS_ENABLED);
        }
        printf("registration failed " RC_RESOLVE_FORMAT_STR "\n", RC_RESOLVE((int)status));
    }
}
/**
 * @brief The function is to get the Updated Registration status of the Device while re-connection to LWM2M Server.
 *
 * @param[in] serverIdx
 * server index
 *
 * @param[in] status
 * registration status
 */
static void RegistrationUpdateCallback(uint8_t serverIdx, retcode_t status)
{
    if (status == RC_OK)
    {
        UpdateRegistrationState(serverIdx, SRS_REGISTRATED);
        printf("Registration update succeeded\r\n");
        if (1 == Started)
        {
            /* first registration update after a restart */
            printf("First update register after restart");
            Started = 2;
        }
    }
    else
    {
        if (RC_COAP_REQUEST_TIMEOUT == status)
        {
            UpdateRegistrationState(serverIdx, SRS_UPDATE_TIMEOUT);
        }
        else
        {
            UpdateRegistrationState(serverIdx, SRS_ENABLED);
        }
        printf("registration update failed " RC_RESOLVE_FORMAT_STR "\n", RC_RESOLVE((int)status));
    }
}
/**
 * @brief The function is to deregister the device in LWM2M Server.
 *
 * @param[in] serverIdx
 * server index
 *
 * @param[in] status
 * registration status
 */
static void DeregistrationCallback(uint8_t serverIdx, retcode_t status)
{
    UpdateRegistrationState(serverIdx, SRS_NONE);
    if (status == RC_OK)
    {
        printf("Deregistration succeeded\r\n");
    }
    else
    {
        printf("deregistration failed " RC_RESOLVE_FORMAT_STR "\n", RC_RESOLVE((int)status));
    }
}

/**
 * @brief   This function is used to @todo
 *
 * @param[in] pxTimer
 * Unused
 */
void TimeChanged(xTimerHandle pxTimer)
{
    BCDS_UNUSED(pxTimer);

    Retcode_T retcode = CmdProcessor_Enqueue(LWM2MSetup.CmdProcessorHandle, TimeChangedHandler, NULL, UINT32_C(0));
    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseError(retcode);
    }
}

/**
 * @brief   This is the callback function triggered after an event occurred in the stack that need to be notified to the application.
 *
 * @param[in] eventType
 * To specify which event we are notifying the application about (registration success or resources changed)
 *
 * @param[in] path
 * structure containing the uri path of the target
 *
 * @param[in] status
 * will tell the application about the kind of error that occurred
 */
static void LWM2MApplicationCallback(Lwm2m_Event_Type_T eventType, Lwm2m_URI_Path_T *path, retcode_t status)
{
    BCDS_UNUSED(path);
    Retcode_T retcode = RETCODE_OK;
    LWM2M_StatusNotification_T appStatus = LWM2M_UNCLASSIFIED_EVENT;
    uint8_t ServerIdx = 0; /* though the callback doesn't provide the server info, only one server is supported! */

    if (RC_OK != status)
    {
        printf("LWM2MApplicationCallback : Serval Stack Error Code : " RC_RESOLVE_FORMAT_STR "\r\n", RC_RESOLVE((unsigned int )status));
    }

    switch (eventType)
    {
    case LWM2M_EVENT_TYPE_REGISTRATION:
        RegistrationCallback(ServerIdx, status);
        if (status == RC_OK)
        {
            appStatus = LWM2M_REGISTRATION_SUCCESS;
        }
        else
        {
            appStatus = LWM2M_REGISTRATION_FAILURE;
        }
        break;

    case LWM2M_EVENT_TYPE_REGISTRATION_UPDATE:
        RegistrationUpdateCallback(ServerIdx, status);
        if (status == RC_OK)
        {
            appStatus = LWM2M_REGISTRATION_UPDATE_SUCCESS;
        }
        else
        {
            appStatus = LWM2M_REGISTRATION_UPDATE_FAILURE;
        }
        break;

    case LWM2M_EVENT_TYPE_DEREGISTRATION:
        DeregistrationCallback(ServerIdx, status);
        if (status == RC_OK)
        {
            appStatus = LWM2M_REGISTRATION_UPDATE_SUCCESS;
        }
        else
        {
            appStatus = LWM2M_REGISTRATION_UPDATE_FAILURE;
        }
        break;

    case LWM2M_EVENT_TYPE_NOTIFICATION:

        if (status == RC_OK)
        {
            appStatus = LWM2M_NOTIFICATION_SUCCESS;
        }
        else
        {
            appStatus = LWM2M_NOTIFICATION_FAILURE;
        }
        break;

    default:
        {
#if DEBUG_LOGGING
        printf("NotificationCallback : eventType - %d \r\n"
                "\t LWM2M_EVENT_TYPE_BOOTSTRAP - %d"
                "\t LWM2M_EVENT_TYPE_WRITE - %d"
                "\t LWM2M_EVENT_TYPE_OBJECT_CREATED - %d"
                "\t LWM2M_EVENT_TYPE_OBJECT_DELETED - %d"
                "\t LWM2M_EVENT_TYPE_NEW_OBSERVER - %d"
                "\t LWM2M_EVENT_TYPE_OBSERVATION_CANCELED - %d"
                "\t LWM2M_EVENT_TYPE_NEW_SERVER_ADDED - %d",
                (int) eventType,
                (int) LWM2M_EVENT_TYPE_BOOTSTRAP,
                (int) LWM2M_EVENT_TYPE_WRITE,
                (int) LWM2M_EVENT_TYPE_OBJECT_CREATED,
                (int) LWM2M_EVENT_TYPE_OBJECT_DELETED,
                (int) LWM2M_EVENT_TYPE_NEW_OBSERVER,
                (int) LWM2M_EVENT_TYPE_OBSERVATION_CANCELED,
                (int) LWM2M_EVENT_TYPE_NEW_SERVER_ADDED);
#endif /* DEBUG_LOGGING */
    }
        return;
    }

    if (NULL != LWM2MSetup.StatusNotificationCB)
    {
        LWM2MSetup.StatusNotificationCB(appStatus);
    }
    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseError(retcode);
    }
}

#if SERVAL_ENABLE_DTLS
/**
 * @brief   This function can be used to copy the LWM2M information to the buffer.
 *
 * @param[in] buffer
 * The input buffer
 *
 * @param[in] data
 * LWM2M information
 *
 * @param[in]len
 * length of the data
 *
 * @note
 * This is a temporary function which will be exported by lwm2m_security in future version.
 *
 */
static inline retcode_t Lwm2mInterface_copyToBuffer(OutputBuffer_T* buffer, const char* data, uint16_t len)
{
    retcode_t rc = RC_OK;
    if (buffer->writableLength == 0)
    {
        buffer->length = len;
        buffer->content = (char*) data;
    }
    else if (buffer->writableLength >= len)
    {
        buffer->length = len;
        memcpy(buffer->content, data, len);
    }
    else
    {
        rc = RC_DTLS_INSUFFICIENT_MEMORY;
    }

    return rc;
}

/**
 * @brief LWM2MSecurityCallback is a Callback function registered to the stack which will be
 * used to obtain security information such as pre-shared-keys, certificates, etc.
 *
 * @param[in]      token
 * The type of token that is requested.
 * The token.type field determines the data type of tokenData.
 * The other fields in token determine the validity of fields in that data type
 * and provide additional information to the application about the type of
 * connection involved.
 *
 * The mapping of the token.type field to data types of tokenData is
 * CIPHERS              -> CiphersData_T,
 * PSK_SERVER_ID_HINT   -> ServerIdHintData_T,
 * PSK_PEER_KEY_AND_ID  -> PeerKeyAndIdData_T,
 * CRT_PEER_NAME        -> PeerNameData_T,
 * CRT_CA               -> CaData_T,
 * CRT_OWN_CERTIFICATE  -> OwnCertificateData_T,
 *
 * @param[in/out]  tokenData
 * Token data contains additional data to help identify what to provide to the
 * implementation as well as everything required to actually provide this data.
 * An application implementing the callback is expected to cast token data to
 * a type depending on the type field of the provided token.
 *
 * The data that is provided in output buffers must be in a format that the
 * underlying SSL implementation can understand. No conversion is performed.
 *
 * @return  RC_OK if the token was provided
 *          RC_DTLS_TOKEN_NOT_PROVIDED if the application does not want to provide the token (e.g. client certificate)
 *          RC_DTLS_PEER_REJECTED if the application does not want to communicate with that peer
 *          RC_DTLS_UNSUPPORTED_TOKEN if the token type cannot be provided by the application
 *          RC_DTLS_INSUFFICIENT_MEMORY if the token does not fit into the allocated space
 */
static retcode_t LWM2MSecurityCallback(SecurityToken_T token, SecurityData_T* tokenData)
{
    if (token.type != PSK_PEER_KEY_AND_ID)
    {
        return RC_DTLS_UNSUPPORTED_TOKEN;
    }

    retcode_t rc = Lwm2mSecurity_defaultCallback(token, tokenData);
    if (RC_DTLS_PEER_REJECTED == rc)
    {
        PeerKeyAndIdData_T* data = (PeerKeyAndIdData_T*) tokenData;
        Lwm2mServer_T* Server = Lwm2m_getServer(LWM2M_SERVER_INDEX);
        rc = Lwm2mInterface_copyToBuffer(&data->ourIdentity, Server->securityInfo.my_identity, strlen(Server->securityInfo.my_identity));
        if (RC_OK == rc)
        {
            rc = Lwm2mInterface_copyToBuffer(&data->key, (const char*) Server->securityInfo.secret_key, Server->securityInfo.secret_key_length);
        }
    }
    return rc;
}

#endif /* SERVAL_ENABLE_DTLS */

static inline Lwm2m_Binding_T LWM2MGetBinding(const char* Binding)
{
    Lwm2m_Binding_T Result = UDP;

    if (NULL != Binding && 0 == strncmp("UQ", Binding, 3))
    {
        Result = UDP_QUEUED;
    }

    return Result;
}

/* global functions ********************************************************* */

/** Refer private header for description */
void LWM2M_Reboot(void)
{
    if (REBOOT_STATE_INIT == RebootState)
    {
        printf("REBOOT triggered");
        RebootState = REBOOT_STATE_START;
#if LWM2M_REPORTING_SUPPORT_SUSPEND
        Lwm2mReporting_suspendNotifications(0);
#endif /* LWM2M_REPORTING_SUPPORT_SUSPEND */
    }
    else
    {
        printf("REBOOT already pending %d", RebootState);
    }
}

/** Refer private header for description */
void LWM2M_RedLedSetState(bool on)
{
    AdjustLastSetLedTime(&LWM2MLastSetRed);
    LWM2MSetLedState(LWM2MRedLedHandle, on ? BSP_LED_COMMAND_ON : BSP_LED_COMMAND_OFF, LWM2MRedLedStateChangeHandler);
}

/** Refer private header for description */
void LWM2M_OrangeLedSetState(bool on)
{
    AdjustLastSetLedTime(&LWM2MLastSetOrange);
    LWM2MSetLedState(LWM2MOrangeLedHandle, on ? BSP_LED_COMMAND_ON : BSP_LED_COMMAND_OFF, LWM2MOrangeLedStateChangeHandler);
}

/** Refer private header for description */
void LWM2M_YellowLedSetState(bool on)
{
    AdjustLastSetLedTime(&LWM2MLastSetYellow);
    LWM2MSetLedState(LWM2MYellowLedHandle, on ? BSP_LED_COMMAND_ON : BSP_LED_COMMAND_OFF, LWM2MYellowLedStateChangeHandler);
}

/** Refer private header for description */
void LWM2M_SetRedLedStateChangeHandler(LWM2M_LedStateChangeHandler_T handler)
{
    LWM2MRedLedStateChangeHandler = handler;
}

/** Refer private header for description */
void LWM2M_SetOrangeLedStateChangeHandler(LWM2M_LedStateChangeHandler_T handler)
{
    LWM2MOrangeLedStateChangeHandler = handler;
}

/** Refer private header for description */
void LWM2M_SetYellowLedStateChangeHandler(LWM2M_LedStateChangeHandler_T handler)
{
    LWM2MYellowLedStateChangeHandler = handler;
}

/** Refer private header for description */
void LWM2M_RebootNow(const char *msg)
{
#if (SERVAL_LOG_LEVEL >= SERVAL_LOG_LEVEL_ERROR)
    /* this message may be lost because of the reboot */
    printf(msg);
    /* wait to give the above logging a chance */
    vTaskDelay(REBOOT_MSG_DELAY_IN_MS / portTICK_PERIOD_MS);
#endif /* (SERVAL_LOG_LEVEL >= SERVAL_LOG_LEVEL_ERROR) */
    BSP_Board_SoftReset();
}

/** Refer interface header for description */
Retcode_T LWM2M_Setup(LWM2M_Setup_T * setup)
{
    Retcode_T retcode = RETCODE_OK;

    if (NULL == setup)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        if (setup->IsSecure)
        {
            if (LWM2M_SECURITY_MODE_PRE_SHARED_KEY != setup->SecurityMode)
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NOT_SUPPORTED);
            }
            else if ((NULL == setup->SecurityPreSharedKey.Identity) ||
                    (NULL == setup->SecurityPreSharedKey.Key))
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
            }
        }
        if (RETCODE_OK == retcode)
        {
            if ((0UL == setup->CmdProcessorHandle) ||
                    (0UL == setup->Lifetime) ||
                    (NULL == setup->ServerURL) ||
                    (NULL == setup->EndPointName))
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NOT_SUPPORTED);
            }
        }
        if (RETCODE_OK == retcode)
        {
            TimeChangeTimer_ptr = xTimerCreate((const char * const ) "SysTime", /* text name, used for debugging. */
            SYSTEM_TIMER_INTERVAL_IN_MS / portTICK_PERIOD_MS, /* The timer period in ticks. */
            pdTRUE, /* The timers will auto-reload themselves when they expire. */
            (void *) NULL, /* Assign each timer a unique id equal to its array index. */
            TimeChanged /* On expire of the timer this function will be called. */
            );
            if (TimeChangeTimer_ptr == NULL)
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }

        }
        if (RETCODE_OK == retcode)
        {
            LedMutexHandle = xSemaphoreCreateMutex();
            if (NULL == LedMutexHandle)
            {
                assert(false);
            }
        }
        if (RETCODE_OK == retcode)
        {
            ;
        }
        if (RETCODE_OK == retcode)
        {
            LWM2MSetup = *setup;
        }
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T LWM2M_Enable(void)
{
    Retcode_T retcode = RETCODE_OK;

    memset(ServerRegistrationInfo, 0, sizeof(ServerRegistrationInfo));
    for (int idx = 0; idx < LWM2M_MAX_NUMBER_SERVERS; ++idx)
    {
        ServerRegistrationInfo[idx].state = SRS_NONE;
        ServerRegistrationInfo[idx].dnsState = DNS_NONE;
        ServerRegistrationInfo[idx].retry = false;
    }
#if SERVAL_ENABLE_DTLS
    Security_setCallback(LWM2MSecurityCallback);

#if SERVAL_ENABLE_DTLS && SERVAL_TLS_MBEDTLS
    if (RC_OK != MbedTLSAdapter_Initialize())
    {
        printf("MbedTLSAdapter_Initialize : unable to initialize Mbedtls.\r\n");
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_INIT_REQUEST_FAILED);
    }
#endif /* SERVAL_ENABLE_DTLS && SERVAL_TLS_MBEDTLS */

#endif /* SERVAL_ENABLE_DTLS */
    if (RETCODE_OK == retcode)
    {
        if (LWM2MSetup.IsSecure)
        {
            sprintf(LWM2MServerURL, LWM2M_URL_FORMAT_SECURE, LWM2MSetup.ServerURL, LWM2MSetup.ServerPort);
        }
        else
        {
            sprintf(LWM2MServerURL, LWM2M_URL_FORMAT_NON_SECURE, LWM2MSetup.ServerURL, LWM2MSetup.ServerPort);
        }
        LWM2MDeviceResourceInfo.name = LWM2MSetup.EndPointName;
        LWM2MDeviceResourceInfo.secure = LWM2MSetup.IsSecure;
        LWM2MDeviceResourceInfo.binding = LWM2MGetBinding(LWM2MSetup.Binding);
        LWM2MUseLedsForRegistration = (LWM2M_TEST_MODE_NO != LWM2MSetup.TestMode);
        Callable_assign(&RebootCallable, CallRebootNow); //lint !e534, ignore return value

        /* @todo - Check why this is done prior */
        LWM2MObjectConnectivityMonitoring_Init();
        LWM2MSensorDeviceGyroscope_InitRand();

        retcode = InitFotaStateMachine();
    }

    if (RETCODE_OK == retcode)
    {
        if (RC_OK != Lwm2m_initialize(&LWM2MDeviceResourceInfo))
        {
            retcode = (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_LWM2M_INIT_FAILED));
        }
    }

    if (RETCODE_OK == retcode)
    {
        Lwm2mServer_T* server = Lwm2m_getServer(LWM2M_SERVER_INDEX);

        Lwm2m_setNumberOfServers(LWM2M_NUMBER_OF_SERVERS);
        server->permissions[0] = LWM2M_READ_ALLOWED | LWM2M_WRITE_ALLOWED | LWM2M_EXECUTE_ALLOWED;
        server->lifetime = LWM2MSetup.Lifetime;
#if SERVAL_ENABLE_DTLS
        if (LWM2MSetup.IsSecure)
        {
            printf("LWM2M_Enable : with DTLS (PSK) \r\n");

            /* The peer_identity is intended to be filled with 0s during intialization
             * and is filled in with the server's psk "server hint" during the DTLS handshake,
             * if the server sends one. Leshan for example doesn't send that server hint. */
            memset(server->securityInfo.peer_identity, 0, sizeof(server->securityInfo.peer_identity) - 1);

            strncpy(server->securityInfo.my_identity, LWM2MSetup.SecurityPreSharedKey.Identity, sizeof(server->securityInfo.my_identity) - 1);
            if (sizeof(server->securityInfo.my_identity) <= strlen(LWM2MSetup.SecurityPreSharedKey.Identity))
            {
                printf("PSK identity truncated to '%s'\n", server->securityInfo.my_identity);
            }

            if (sizeof(server->securityInfo.secret_key) <= LWM2MSetup.SecurityPreSharedKey.KeyLength)
            {
                printf("PSK secret key truncated to '%s'\n", server->securityInfo.secret_key);
                server->securityInfo.secret_key_length = sizeof(server->securityInfo.secret_key) - 1;
            }
            else
            {
                server->securityInfo.secret_key_length = LWM2MSetup.SecurityPreSharedKey.KeyLength;

            }
            strncpy((char*) server->securityInfo.secret_key, LWM2MSetup.SecurityPreSharedKey.Key, server->securityInfo.secret_key_length);
        }
        else
#endif /* SERVAL_ENABLE_DTLS */
        {
            printf("LWM2M_Enable : without DTLS \r\n");
        }
    }
    if (RETCODE_OK == retcode)
    {
        if (RC_OK != LWM2MDnsUtil_Init())
        {
            retcode = (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_LWM2M_DNSUTIL_FAILED));
        }
    }
    if (RETCODE_OK == retcode)
    {
        /* initialize LWM2M resource synchronization mutex */
        LWM2MObjects_Init(LWM2MSetup.ConfirmableNotification);

        LWM2MObjectDevice_Init();
        LWM2MObjectLightControl_Init();
        LWM2MObjectAlertNotification_Init();
        LWM2MObjectAccelerometer_Init();
        LWM2MObjectBarometer_Init();
        LWM2MObjectGyroscope_Init();
        LWM2MObjectHumidity_Init();
        LWM2MObjectIlluminance_Init();
        LWM2MObjectMagnetometer_Init();
        LWM2MObjectTemperature_Init();
        LWM2MObjectSensorDevice_Init(LWM2MSetup.MACAddr);

        /* enable LWM2M objects */
        LWM2MObjectDevice_Enable();
        LWM2MObjectAlertNotification_Enable();
        LWM2MObjectLightControl_Enable(LWM2M_TEST_MODE_YES == LWM2MSetup.TestMode);
        LWM2MObjectConnectivityMonitoring_Enable();
        LWM2MObjectSensorDevice_Enable();
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T LWM2M_Register(void)
{
    Retcode_T retcode = RETCODE_OK;

    uint32_t Time = 0;
    uint8_t ServerIdx = 0;
    Lwm2mServer_T* server = Lwm2m_getServer(LWM2M_SERVER_INDEX);

    if (RC_OK != Clock_getTime(&Time))
    {
        retcode = (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_LWM2M_CLOCK_GETTIME_FAILED));
    }
    else
    {
        ServerRegistrationInfo[ServerIdx].lastDns = Time;
    }

    if (retcode == RETCODE_OK)
    {
        /* @todo - Move this to Setup / enable as soon as URL is resolved. */
        if (RC_OK != LWM2MDnsUtil_ResolveHostname(LWM2MServerURL, ServerRegistrationInfo[ServerIdx].destinationResolved, sizeof(ServerRegistrationInfo[ServerIdx].destinationResolved), DNS_TIMEOUT_IN_S))
        {
            retcode = (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_LWM2M_DNSRESOLVE_HOSTNAME_FAILED));
        }
        else
        {
            if (0 != strcmp(LWM2MServerURL, ServerRegistrationInfo[ServerIdx].destinationResolved))
            {
                printf("LWM2M server resolved: %s\r\n", ServerRegistrationInfo[ServerIdx].destinationResolved);
                ServerRegistrationInfo[ServerIdx].dnsState = DNS_RESOLVED;
                strncpy(ServerRegistrationInfo[ServerIdx].destination, LWM2MServerURL, sizeof(ServerRegistrationInfo[ServerIdx].destination) - 1);
                if (sizeof(ServerRegistrationInfo[ServerIdx].destination) <= strlen(LWM2MServerURL))
                {
                    printf("URL LWM2M server truncated to '%s'\n", ServerRegistrationInfo[ServerIdx].destination);
                }
            }
            else
            {
                ServerRegistrationInfo[ServerIdx].dnsState = DNS_NONE;
            }
        }
        strncpy(server->serverAddress, ServerRegistrationInfo[ServerIdx].destinationResolved, sizeof(server->serverAddress) - 1);
        if (sizeof(server->serverAddress) <= strlen(ServerRegistrationInfo[ServerIdx].destinationResolved))
        {
            printf("Resolved LWM2M server truncated to '%s'\r\n", server->serverAddress);
        }
    }
    if (retcode == RETCODE_OK)
    {
        if (RC_OK != Lwm2m_start(Ip_convertIntToPort(LWM2MSetup.ClientPort), LWM2MApplicationCallback))
        {
            retcode = (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_LWM2M_STACK_START_FAILED));
        }
    }
    if (retcode == RETCODE_OK)
    {
        if (pdTRUE != xTimerStart(TimeChangeTimer_ptr, portMAX_DELAY))
        {
            retcode = (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_LWM2M_TIMER_START_FAILED));
        }
        else
        {
            ServerRegistrationInfo[0].state = SRS_ENABLED;
        }
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T LWM2M_TriggerForceRegister(void)
{
    RegistrationInfo_T* Info = &ServerRegistrationInfo[0];

    if (SRS_REGISTRATED == Info->state)
    {
        printf("==> force re-registration\r\n");
        Info->state = SRS_RETRY_REGISTRATION;
    }
    else if (SRS_ENABLED == Info->state)
    {
        printf("==> force registration\r\n");
        Info->lastRegistrationMessage = 0;
        Started = 0;
    }
    printf("====> %s\r\n", GetRegistrationStateName(Info->state));
    return RETCODE_OK;
}

/** Refer interface header for description */
Retcode_T LWM2M_AlertNotification(char * value)
{
    LWM2MObjectAlertNotification_SetValue(value);
    return RETCODE_OK;
}

/** Refer interface header for description */
Retcode_T LWM2M_ConnectivityMonitoring(LWM2M_ConnectivityMonitoring_T notify, void * value)
{
    Retcode_T retcode = RETCODE_OK;

    switch (notify)
    {
    case LWM2M_SET_IP_ADDRESS:
        LWM2MObjectConnectivityMonitoring_SetIpAddress((const char*) value);
        break;
    case LWM2M_SET_RADIO_SIGNAL_STRENGTH:
        LWM2MObjectConnectivityMonitoring_SetRadioSignalStrength((int) value);
        break;
    default:
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
        break;
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T LWM2M_LedMixModeControl(LWM2M_LedMixMode_T state)
{
    Retcode_T retcode = RETCODE_OK;

    switch (state)
    {
    case LWM2M_LED_MIX_MODE_RED_ON:
        LWM2M_RedLedSetState(true);
        break;
    case LWM2M_LED_MIX_MODE_RED_OFF:
        LWM2M_RedLedSetState(false);
        break;
    case LWM2M_LED_MIX_MODE_YELLOW_ON:
        LWM2M_YellowLedSetState(true);
        break;
    case LWM2M_LED_MIX_MODE_YELLOW_OFF:
        LWM2M_YellowLedSetState(false);
        break;
    case LWM2M_LED_MIX_MODE_ORANGE_ON:
        LWM2M_OrangeLedSetState(true);
        break;
    case LWM2M_LED_MIX_MODE_ORANGE_OFF:
        LWM2M_OrangeLedSetState(false);
        break;
    default:
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
        break;
    }
    return retcode;
}

#endif /* XDK_CONNECTIVITY_LWM2M */

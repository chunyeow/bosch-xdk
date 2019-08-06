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

#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_GRIDEYESERVICE

#include "BCDS_Retcode.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "semphr.h"
#include "task.h"
#include "GridEyeService.h"
#include "BleTypes.h"
#include "BCDS_Ble.h"
#include "BCDS_BlePeripheral.h"
/*lint -e652 -e830 */
#include "attserver.h"
/*lint +e652 +e830 */

#ifndef BLE_OS_DELAY_UPON_CONNECT
#define BLE_OS_DELAY_UPON_CONNECT       (UINT32_C(250))
#endif /* BLE_OS_DELAY_UPON_CONNECT */

static bool IsFirstSendPending = false;
static TickType_t BleLastWakeTimeUponConnect = 0UL;

/* Put the type and macro definitions here */

/**
 * @brief Typedef to represent the characteristic properties of GridEye communication  service
 */
typedef struct GridEye_Characteristic_S GridEye_Characteristic_T;

/**
 * @brief structure to hold the characteristic properties of GridEye communication  service
 */
struct GridEye_Characteristic_S
{
    uint8_t CharacteristicUUID[16];
    AttUuid UUIDType;
    Att128BitCharacteristicAttribute Characteristic;
    AttAttribute CharacteristicAttribute;
};

/*Buffer to write BLE data to the client*/
static uint8_t GridEyeCharacteristicWrite[100];

/*Attribute handle for the BLE GridEye sensor Service */
static AttServiceAttribute GridEyeServiceAttribute;

/*Callback function of type Ble_DataReceivedCallBack to read the data received */
static GridEyeService_DataReceivedCallBack DataReadCallBack = NULL;

static GriEyeService_SendEventCallback SendCallBack = NULL;

/*BLE GridEye sensor Service UUID: 0783B03E-8535-B5A0-7140-A304D2495CB7 */
const uint8_t GridEyeServiceUUID[16] = { 0xB7, 0x5C, 0x49, 0xD2, 0x04, 0xA3, 0x40, 0x71, 0xA0, 0xB5, 0x35, 0x85, 0x3E, 0xB0, 0x83, 0x07 };

/*Characteristic property for the gridEyeData transmit Characteristic < UUID - 0783B03E-8535-B5A0-7140-A304D2495CB8 >*/
static GridEye_Characteristic_T GridEyeBleTxCharProperties = {
        { 0xB8, 0x5C, 0x49, 0xD2, 0x04, 0xA3, 0x40, 0x71, 0xA0, 0xB5, 0x35, 0x85, 0x3E, 0xB0, 0x83, 0x07 },
        { 0, { 0 } },
        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 } };

/*Characteristic property for gridEyeData receive Characteristic characteristic <UUID -  0783B03E-8535-B5A0-7140-A304D2495CBA> */
static GridEye_Characteristic_T GridEyeBleRxCharProperties = {
        { 0xBA, 0x5C, 0x49, 0xD2, 0x04, 0xA3, 0x40, 0x71, 0xA0, 0xB5, 0x35, 0x85, 0x3E, 0xB0, 0x83, 0x07 },
        { 0, { 0 } },
        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 } };

/*Characteristic property for the flowControl Notify characteristic <UUID -  0783B03E-8535-B5A0-7140-A304D2495CB9> */
static GridEye_Characteristic_T GridEyeBleNotifyCharProperties = {
        { 0xB9, 0x5C, 0x49, 0xD2, 0x04, 0xA3, 0x40, 0x71, 0xA0, 0xB5, 0x35, 0x85, 0x3E, 0xB0, 0x83, 0x07 },
        { 0, { 0 } },
        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 } };

/* Register/Add custom UUID with this stack API to show service UUID details while advertising  */
extern void BleGap_Add128bitsServiceUUID(const U8 *uuid);

/* Put constant and variable definitions here */

static uint16_t ConnectionHandle; /*Connection handle to client */

/* Put private function declarations here */

/**
 * @brief This is the GridEye BLE sensor service callback, register at the time of BLE initialization.
 *        This application level callback gets triggered on various BLE event such as, Attribute events
 *
 * @param[in] attData  Contains the type of actionEvent triggered by BLE
 **/
static void GridEyeService_ServiceCallback(AttServerCallbackParms* attData)
{
    if (NULL == attData)
    {
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_WARNING, (Retcode_T ) RETCODE_NULL_POINTER));
        return;
    }
    switch (attData->event)
    {
    /* Client is willing to write value */
    case ATTEVT_SERVER_WRITE_REQ:
        if (ATTSTATUS_SUCCESS == attData->status)
        {
            if (attData->parms.writeReq.attribute == &GridEyeBleRxCharProperties.CharacteristicAttribute)
            {
                attData->parms.writeReq.response = ATTPDU_STATUS_SUCCESS;
            }
        }
        break;

        /* Client has successfully written value */
    case ATTEVT_SERVER_WRITE_COMPLETE:
        if (ATTSTATUS_SUCCESS == attData->status)
        {
            AttWriteCompleteData rxData = attData->parms.writeComplete;
            if (attData->parms.writeComplete.attribute == &GridEyeBleRxCharProperties.CharacteristicAttribute)
            {
                const uint8_t *rxValue;
                uint16_t rxLength;
                rxValue = rxData.value;
                rxLength = rxData.length;
                if (NULL != DataReadCallBack)
                {
                    DataReadCallBack((uint8_t *) rxValue, rxLength, NULL);
                }
            }
        }
        break;

        /* Handle value indication
         * Indicates the Packet could finally be sent or not
         */
    case ATTEVT_SERVER_HVI_SENT:
        {
        Retcode_T retcode = RETCODE_OK;
        if (ATTSTATUS_SUCCESS != attData->status)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_DATA_SEND_FAIL);
        }
        if (NULL != SendCallBack)
        {
            SendCallBack(retcode);
        }
    }
        break;

    case ATTEVT_SERVER_FREE:
        case ATTEVT_SERVER_READ_REQ:
        case ATTEVT_SERVER_HANDLE_VALUE_CONFIRMATION:
        break;

    default:
        break;
    }

}

/**
 * @brief This contains the implementation to register the GridEye Service to the BLE stack
 * @retval RETCODE_OK if success error otherwise
 **/
Retcode_T GridEyeService_Register(void)
{
    BleStatus registerStatus = BLESTATUS_SUCCESS;
    AttStatus attRequestStatus;
    Retcode_T retcode = RETCODE_OK;

    ATT_SERVER_SecureDatabaseAccess();

    BleGap_Add128bitsServiceUUID(&GridEyeServiceUUID[0]);

    attRequestStatus = ATT_SERVER_RegisterServiceAttribute(ATTPDU_SIZEOF_128_BIT_UUID, (uint8_t *) GridEyeServiceUUID,
            GridEyeService_ServiceCallback, &(GridEyeServiceAttribute));

    if (ATTSTATUS_SUCCESS != attRequestStatus)
    {
        /*
         * - ATTSTATUS_DATABASE_FROZEN (only in case ATT_HANDLES_ARE_FREEZABLE)
         *   if ATT_SERVER_FreezeDatabase() has already been called,
         *   in which case more change is accepted in the database.
         * - ATTSTATUS_BUSY if there is at least one link layer connection,
         *   in which case the database's structure shall not be modified.
         * - ATTSTATUS_DATABASE_FULL if there is no room in the handle's space
         *   to APPEND this attribute at the back of the database.
         * - ATTSTATUS_INVALID_PARM (only checked if ATT_PARMS_CHECK == 1 ;
         *   asserted otherwise if ATT_DEBUG == 1) if valueLen is not 2 or 16,
         *   or any pointer parameter is NULL.( always asserted) if database is not locked.
         * - ATTSSTATUS_FAILED: if the service memory is already in the database.
         */
        registerStatus = BLESTATUS_FAILED;
    }
    else
    {
        /* RX Characteristic (gridEyeDataCharacteristicId) */

        GridEyeBleRxCharProperties.UUIDType.size = ATT_UUID_SIZE_128;
        GridEyeBleRxCharProperties.UUIDType.value.uuid128 = (uint8_t *) GridEyeBleRxCharProperties.CharacteristicUUID;

        attRequestStatus = ATT_SERVER_AddCharacteristic(
        ATTPROPERTY_WRITE | ATTPROPERTY_WRITEWITHOUTRESPONSE,
                (Att16BitCharacteristicAttribute*) &GridEyeBleRxCharProperties.Characteristic,
                &GridEyeBleRxCharProperties.UUIDType,
                ATT_PERMISSIONS_ALLACCESS, 20,
                (uint8_t *) &GridEyeCharacteristicWrite[20], 0, 0,
                &GridEyeServiceAttribute,
                &GridEyeBleRxCharProperties.CharacteristicAttribute);

        if (ATTSTATUS_SUCCESS != attRequestStatus)
        {
            /* - ATTSTATUS_DATABASE_FROZEN (only in case ATT_HANDLES_ARE_FREEZABLE)
             *   if ATT_SERVER_FreezeDatabase() has already been called,
             *   in which case more change is accepted in the database.
             * - ATTSTATUS_BUSY if there is at least one link layer connection,
             *   in which case the database's structure shall not be modified.
             * - ATTSTATUS_DATABASE_FULL if there is no room in the handle's space
             *   to APPEND this attribute AFTER the last attribute currently belonging
             *   to this service (might be the service attribute itself).
             * - ATTSTATUS_INVALID_PARM (only checked if ATT_PARMS_CHECK == 1 ;
             *   asserted otherwise if ATT_DEBUG == 1) if the UUID is not a 16 or
             *   128 bit one, or any pointer parameter is NULL, or the serviceAttribute
             *   is invalid or not registered or memory is already in the database.
             */
            registerStatus = BLESTATUS_FAILED;
        }

        else
        {
            /* TX Characteristic (serialPortCharacterUUID2) */

            GridEyeBleTxCharProperties.UUIDType.size = ATT_UUID_SIZE_128;
            GridEyeBleTxCharProperties.UUIDType.value.uuid128 = (uint8_t *) GridEyeBleTxCharProperties.CharacteristicUUID;

            attRequestStatus = ATT_SERVER_AddCharacteristic(
            ATTPROPERTY_NOTIFY | ATTPROPERTY_INDICATE,
                    (Att16BitCharacteristicAttribute*) &GridEyeBleTxCharProperties.Characteristic,
                    &GridEyeBleTxCharProperties.UUIDType,
                    ATT_PERMISSIONS_ALLACCESS, 20,
                    (uint8_t *) &GridEyeCharacteristicWrite[20], 0, 0,
                    &GridEyeServiceAttribute,
                    &GridEyeBleTxCharProperties.CharacteristicAttribute);

            if (ATTSTATUS_SUCCESS != attRequestStatus)
            {
                /* - ATTSTATUS_DATABASE_FROZEN (only in case ATT_HANDLES_ARE_FREEZABLE)
                 *   if ATT_SERVER_FreezeDatabase() has already been called,
                 *   in which case more change is accepted in the database.
                 * - ATTSTATUS_BUSY if there is at least one link layer connection,
                 *   in which case the database's structure shall not be modified.
                 * - ATTSTATUS_DATABASE_FULL if there is no room in the handle's space
                 *   to APPEND this attribute AFTER the last attribute currently belonging
                 *   to this service (might be the service attribute itself).
                 * - ATTSTATUS_INVALID_PARM (only checked if ATT_PARMS_CHECK == 1 ;
                 *   asserted otherwise if ATT_DEBUG == 1) if the UUID is not a 16 or
                 *   128 bit one, or any pointer parameter is NULL, or the serviceAttribute
                 *   is invalid or not registered or memory is already in the database.
                 */
                registerStatus = BLESTATUS_FAILED;
            }

            else
            {
                /* Notify Characteristic (flowControlNotify) */
                GridEyeBleNotifyCharProperties.UUIDType.size = ATT_UUID_SIZE_128;
                GridEyeBleNotifyCharProperties.UUIDType.value.uuid128 = (uint8_t *) GridEyeBleNotifyCharProperties.CharacteristicUUID;

                attRequestStatus = ATT_SERVER_AddCharacteristic(
                ATTPROPERTY_NOTIFY,
                        (Att16BitCharacteristicAttribute*) &GridEyeBleNotifyCharProperties.Characteristic,
                        &GridEyeBleNotifyCharProperties.UUIDType,
                        ATT_PERMISSIONS_ALLACCESS, 20,
                        (uint8_t *) &GridEyeCharacteristicWrite[20], 0, 0,
                        &GridEyeServiceAttribute,
                        &GridEyeBleNotifyCharProperties.CharacteristicAttribute);

                if (ATTSTATUS_SUCCESS != attRequestStatus)
                {
                    /* - ATTSTATUS_DATABASE_FROZEN (only in case ATT_HANDLES_ARE_FREEZABLE)
                     *   if ATT_SERVER_FreezeDatabase() has already been called,
                     *   in which case more change is accepted in the database.
                     * - ATTSTATUS_BUSY if there is at least one link layer connection,
                     *   in which case the database's structure shall not be modified.
                     * - ATTSTATUS_DATABASE_FULL if there is no room in the handle's space
                     *   to APPEND this attribute AFTER the last attribute currently belonging
                     *   to this service (might be the service attribute itself).
                     * - ATTSTATUS_INVALID_PARM (only checked if ATT_PARMS_CHECK == 1 ;
                     *   asserted otherwise if ATT_DEBUG == 1) if the UUID is not a 16 or
                     *   128 bit one, or any pointer parameter is NULL, or the serviceAttribute
                     *   is invalid or not registered or memory is already in the database.
                     */
                    registerStatus = BLESTATUS_FAILED;
                }
            }
        }
    }

    ATT_SERVER_ReleaseDatabaseAccess();
    if (registerStatus != BLESTATUS_SUCCESS)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_FAILURE);
    }
    else
    {
        retcode = RETCODE_OK;
    }
    return retcode;
}

/* The description is in the private header */
Retcode_T GridEyeService_Init(GridEyeService_DataReceivedCallBack readCallback, GriEyeService_SendEventCallback sendCallback)
{
    if (NULL != readCallback)
    {
        DataReadCallBack = readCallback;
        SendCallBack = sendCallback;
        return RETCODE_OK;
    }
    else
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_NULL_POINTER));
    }
}

Retcode_T GridEyeService_SendData(U8* payload, U8 payloadLen)
{
    Retcode_T retcode = RETCODE_OK;
    AttStatus attWriteStatus = ATTSTATUS_FAILED;
    AttStatus attSendNotifyStatus = ATTSTATUS_FAILED;

    if ((NULL != payload) && (UINT8_C(0) != payloadLen))
    {
        ATT_SERVER_SecureDatabaseAccess();
        attWriteStatus = ATT_SERVER_WriteAttributeValue(&GridEyeBleTxCharProperties.CharacteristicAttribute, (const uint8_t *) payload, payloadLen);
        if (ATTSTATUS_SUCCESS != attWriteStatus)
        {
            /*
             * If:
             * - ATTSTATUS_INVALID_PARM (only checked if ATT_PARMS_CHECK == 1 ;
             *   asserted otherwise if ATT_DEBUG == 1)
             * - ATTSTATUS_FAILED if the attribute is NULL or could not be found in the database.
             */
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_REWRITE_OF_ATT_FAILED);
        }
        if (RETCODE_OK == retcode)
        {
            ConnectionHandle = BlePeripheral_GetConnectionHandle();
            if (UINT16_C(0) == ConnectionHandle)
            {
                /* if the remote device is not connected. */
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_NO_DEVICE_PAIRED);
            }
        }
        if (RETCODE_OK == retcode)
        {
            if (false == IsFirstSendPending)
            {
                vTaskDelayUntil(&BleLastWakeTimeUponConnect, pdMS_TO_TICKS(BLE_OS_DELAY_UPON_CONNECT));
                IsFirstSendPending = true;
            }
        }
        if (RETCODE_OK == retcode)
        {
            attSendNotifyStatus = ATT_SERVER_SendNotification(&GridEyeBleTxCharProperties.CharacteristicAttribute, ConnectionHandle);
            if ((ATTSTATUS_PENDING == attSendNotifyStatus) || (ATTSTATUS_SUCCESS == attSendNotifyStatus))
            {
                ;/* don't disturb retcode */
            }
            else if (ATTSTATUS_FAILED == attSendNotifyStatus)
            {
                /* if the channel is closed at the level below, or if the Packet can not be placed in the send queue. */
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_DATA_SEND_FAIL);
            }
            else if (ATTSTATUS_INVALID_PARM == attSendNotifyStatus)
            {
                /* if:
                 * - attribute cannot be found in the database, or
                 * - attribute can be found in the database but is not committed, or
                 * - channel is an invalid combination. (always asserted)
                 * - the database is not locked. */
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_ATT_INVALID_PARAM);
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_UNEXPECTED_BEHAVIOR);
            }
        }

        ATT_SERVER_ReleaseDatabaseAccess();
    }
    else
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_INVALID_PARAM);
    }
    return (retcode);
}

/* The description is in the private header */
void GridEyeService_UpdateConnectionStatus(bool connectionStatus)
{
    if (!connectionStatus)
    {
        IsFirstSendPending = connectionStatus;
    }
    else
    {
        BleLastWakeTimeUponConnect = xTaskGetTickCount();
    }
}

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
 * @file
 *
 * This module handles the BLE peripheral feature
 *
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_BLE

#if XDK_CONNECTIVITY_BLE

/* own header files */
#include "XDK_BLE.h"

/* system header files */
#include <stdio.h>

/* additional interface header files */
#include "BleDeviceInfoService.h"
#include "GridEyeService.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "XDK_Utils.h"
#include "BLECustomConfig.h"
#include "SensorServices.h"
/* constant definitions ***************************************************** */
/* plausibility checks ****************************************************** */

static_assert(BLE_RANDOM_ADDRESSING_SUPPORT == 1, "Random addressing must be enabled for XDK projects that use Bluetooth Smart as"
        "a fall-back solution in case the device cannot load a proper public MAC address.")

/**< Macro for synchronization signal timeout in terms of milli-second used for BLE peripheral events and mutex guard */
#define BLE_EVENT_SYNC_TIMEOUT                  UINT32_C(1000)
#define BLE_MAC_ADDRESS_LENGTH                  UINT8_C(0x06)

/* local variables ********************************************************** */

/**< Handle for BLE peripheral event signal synchronization */
static SemaphoreHandle_t BleEventSignal = (SemaphoreHandle_t) NULL;
/**< Handle for BLE data send complete signal synchronization */
static SemaphoreHandle_t BleSendCompleteSignal = (SemaphoreHandle_t) NULL;
/**< Handle for BLE data send Mutex guard */
static SemaphoreHandle_t BleSendGuardMutex = (SemaphoreHandle_t) NULL;
/**< BLE peripheral setup information */
static BLE_Setup_T BLESetup;
/**< BLE send status */
static Retcode_T BleSendStatus;
/**< BLE connection status flag */
static bool BleIsConnected = false;
static bool BleWakeUpStatus = false;
/**< BLE peripheral event */
static BlePeripheral_Event_T BleEvent = BLE_PERIPHERAL_EVENT_MAX;

/**
 * @brief  This is the send status notification function registered at
 * the time of BLE peripheral service registration.
 *
 * @param[in]   sendStatus
 * Status of the send operation
 */
static void BleSendCallback(Retcode_T sendStatus)
{
    BleSendStatus = sendStatus;
    if (xSemaphoreGive(BleSendCompleteSignal) != pdTRUE)
    {
        /* We would not expect this call to fail because we expect the application thread to wait for this semaphore */
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR));
    }
}

/**
 * @brief  This is the callback for BLE_BCDS_BIDIRECTIONAL_SERVICE data reception.
 *
 * @param[in]   rxBuffer
 * pointer to the received data buffer
 *
 * @param[in]   rxDataLength
 * Length of the received data
 */
void BleBcdsBidirectionalServiceDataRxCB(uint8_t *rxBuffer, uint8_t rxDataLength)
{
    if (NULL != BLESetup.DataRxCB)
    {
        BLESetup.DataRxCB(rxBuffer, rxDataLength, NULL);
    }
}

/**
 * @brief  This is the callback for BLE_XDK_SENSOR_SERVICES data reception.
 *
 * @param[in]   rxBuffer
 * pointer to the received data buffer
 *
 * @param[in]   rxDataLength
 * Length of the received data
 *
 * @param[in]   param
 * A #SensorServices_Info_T pointer.
 */
void BleXdkSensorServicesServiceDataRxCB(uint8_t *rxBuffer, uint8_t rxDataLength, SensorServices_Info_T * param)
{
    if (NULL != BLESetup.DataRxCB)
    {
        BLESetup.DataRxCB(rxBuffer, rxDataLength, param);
    }
}

/**
 * @brief  This is the callback for BLE_XDK_GRIDEYE_SERVICE data reception.
 *
 * @param[in]   rxBuffer
 * pointer to the received data buffer
 *
 * @param[in]   rxDataLength
 * Length of the received data
 *
 * @param[in]   param
 * Unused
 */
void BleXdkGridEyeServicesServiceDataRxCB(uint8_t *rxBuffer, uint8_t rxDataLength, void * param)
{
    if (NULL != BLESetup.DataRxCB)
    {
        BLESetup.DataRxCB(rxBuffer, rxDataLength, param);
    }
}

/**
 * @brief This is the function that will be called by the stack in order to register the BLE services the application wants to use.
 * This function will be called after the application has called #BlePeripheral_Start.
 * Within this function, all services that the application requires will be registered.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T BleServiceRegistry(void)
{
    Retcode_T retcode = RETCODE_OK;

    if (true == BLESetup.IsDeviceCharacteristicEnabled)
    {
        retcode = BleDeviceInformationService_Initialize(BLESetup.CharacteristicValue);
    }
    if (BLE_BCDS_BIDIRECTIONAL_SERVICE == BLESetup.Service)
    {
        if (RETCODE_OK == retcode)
        {
            retcode = BidirectionalService_Init(BleBcdsBidirectionalServiceDataRxCB, BleSendCallback);
        }
        if (RETCODE_OK == retcode)
        {
            retcode = BidirectionalService_Register();
        }
    }
    else if (BLE_XDK_SENSOR_SERVICES == BLESetup.Service)
    {
        if (RETCODE_OK == retcode)
        {
            retcode = SensorServices_Init(BleXdkSensorServicesServiceDataRxCB, BleSendCallback);
        }
        if (RETCODE_OK == retcode)
        {
            retcode = SensorServices_Register();
        }
    }
    else if (BLE_XDK_GRIDEYE_SERVICE == BLESetup.Service)
    {
        if (RETCODE_OK == retcode)
        {
            retcode = GridEyeService_Init(BleXdkGridEyeServicesServiceDataRxCB, BleSendCallback);
        }
        if (RETCODE_OK == retcode)
        {
            retcode = GridEyeService_Register();
        }
    }
    else if (BLE_USER_CUSTOM_SERVICE == BLESetup.Service)
    {
        if (RETCODE_OK == retcode)
        {
            if (NULL != BLESetup.CustomServiceRegistryCB)
            {
                retcode = BLESetup.CustomServiceRegistryCB();
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_BLE_CUSTOM_CB_IS_NULL);
            }
        }
    }
    else
    {
        if (RETCODE_OK == retcode)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_BLE_INVALID_SERVICE_IS_SETUP);
        }
    }
    if (true == BLESetup.IsDeviceCharacteristicEnabled)
    {
        if (RETCODE_OK == retcode)
        {
            retcode = BleDeviceInformationService_Register();
        }
    }

    return (retcode);
}

/**
 * @brief This is the function that will be called by the stack when it changes its state.
 *
 * @param [in]  event
 * The event that has occurred.
 *
 * @param [in]  data
 * void pointer pointing to a data type with further information based on the event. Unused now.
 *
 *
 * |             event                      |   data type  |
 * -----------------------------------------|---------------
 * |  BLE_PERIPHERAL_STARTED                |   Retcode_T  |
 * |  BLE_PERIPHERAL_SERVICES_REGISTERED    |   unused     |
 * |  BLE_PERIPHERAL_SLEEP_SUCCEEDED        |   Retcode_T  |
 * |  BLE_PERIPHERAL_WAKEUP_SUCCEEDED       |   Retcode_T  |
 * |  BLE_PERIPHERAL_CONNECTED              |   Ble_RemoteDeviceAddress_T |
 * |  BLE_PERIPHERAL_DISCONNECTED           |   Ble_RemoteDeviceAddress_T |
 * |  BLE_PERIPHERAL_ERROR                  |   Retcode_T |
 *
 */
static void BleEventCallBack(BlePeripheral_Event_T event, void * data)
{
    BCDS_UNUSED(data);
    BleEvent = event;

    switch (event)
    {
        case BLE_PERIPHERAL_STARTED:
        printf("BleEventCallBack : BLE powered ON successfully \r\n");
        if (pdTRUE != xSemaphoreGive(BleEventSignal))
        {
            /* We would not expect this call to fail because we expect the application thread to wait for this semaphore */
            Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_SEMAPHORE_ERROR));
        }
        break;
        case BLE_PERIPHERAL_SERVICES_REGISTERED:
        break;
        case BLE_PERIPHERAL_SLEEP_SUCCEEDED:
        printf("BleEventCallBack : BLE successfully entered into sleep mode \r\n");
        break;
        case BLE_PERIPHERAL_WAKEUP_SUCCEEDED:
        BleWakeUpStatus = true;
        printf("BleEventCallBack : Device Wake up succeeded \r\n");
        if (pdTRUE != xSemaphoreGive(BleEventSignal))
        {
            /* We would not expect this call to fail because we expect the application thread to wait for this semaphore */
            Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_SEMAPHORE_ERROR));
        }
        break;
        case BLE_PERIPHERAL_CONNECTED:
        printf("BleEventCallBack : Device connected \r\n");
        BleIsConnected = true;
        if (BLE_XDK_SENSOR_SERVICES == BLESetup.Service)
        {
            SensorServices_UpdateConnectionStatus(BleIsConnected);
        }
        if (BLE_XDK_GRIDEYE_SERVICE == BLESetup.Service)
        {
            GridEyeService_UpdateConnectionStatus(BleIsConnected);
        }
        break;
        case BLE_PERIPHERAL_DISCONNECTED:
        printf("BleEventCallBack : Device Disconnected \r\n");
        BleIsConnected = false;
        if (BLE_XDK_SENSOR_SERVICES == BLESetup.Service)
        {
            SensorServices_UpdateConnectionStatus(BleIsConnected);
        }
        if (BLE_XDK_GRIDEYE_SERVICE == BLESetup.Service)
        {
            GridEyeService_UpdateConnectionStatus(BleIsConnected);
        }
        break;
        case BLE_PERIPHERAL_ERROR:
        printf("BleEventCallBack : BLE Error Event \r\n");
        break;
        default:
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_BLE_INVALID_EVENT_RECEIVED));
        break;
    }
}

/** Refer interface header for description */
Retcode_T BLE_Setup(BLE_Setup_T * setup)
{
    Retcode_T retcode = RETCODE_OK;
    uint8_t bleMacAddr[BLE_MAC_ADDRESS_LENGTH] =
    {   0, 0, 0, 0, 0, 0};
    uint8_t bleMac[BLE_MAC_ADDRESS_LENGTH] =
    {   0, 0, 0, 0, 0, 0};
    uint64_t bleMacAddress = UINT64_C(0);
    if (NULL == setup)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        BleEventSignal = xSemaphoreCreateBinary();
        if (NULL == BleEventSignal)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
        }
        if (RETCODE_OK == retcode)
        {
            BleSendCompleteSignal = xSemaphoreCreateBinary();
            if (NULL == BleSendCompleteSignal)
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
                vSemaphoreDelete(BleEventSignal);
            }
        }
        if (RETCODE_OK == retcode)
        {
            BleSendGuardMutex = xSemaphoreCreateMutex();
            if (NULL == BleSendGuardMutex)
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
                vSemaphoreDelete(BleEventSignal);
                vSemaphoreDelete(BleSendCompleteSignal);
            }
        }
        if (RETCODE_OK == retcode)
        {
            retcode = BlePeripheral_Initialize(BleEventCallBack, BleServiceRegistry);
        }
        if (RETCODE_OK == retcode)
        {
            retcode = BlePeripheral_SetDeviceName((uint8_t*) setup->DeviceName);
        }
        if (RETCODE_OK == retcode)
        {
            if (setup->IsMacAddrConfigured)
            {
                retcode = BlePeripheral_SetMacAddress(setup->MacAddr);
            }
            else
            {
                if (RETCODE_OK == retcode)
                {
                    retcode = Utils_GetMacInfoFromNVM(UTILS_BLE_MAC_DATA, bleMacAddr);
                }
                if (RETCODE_OK == retcode)
                {
                    for (uint8_t i = UINT8_C(0); i < BLE_MAC_ADDRESS_LENGTH; i++)
                    {
                        bleMac[(BLE_MAC_ADDRESS_LENGTH - (i + 1))] = bleMacAddr[i];
                    }
                    bleMacAddress = (*((uint64_t *) bleMac)) & (UINT64_C(0x0000FFFFFFFFFFFF));
                    retcode = BlePeripheral_SetMacAddress(bleMacAddress);
                }
            }
        }
        if (RETCODE_OK == retcode)
        {
            BLESetup = *setup;
        }
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T BLE_Enable(void)
{
    Retcode_T retcode = RETCODE_OK;

    /* @todo - BLE in XDK is unstable for wakeup upon bootup.
     * Added this delay for the same.
     * This needs to be addressed in the HAL/BSP. */
    vTaskDelay(pdMS_TO_TICKS(1000));

    /* This is a dummy take. In case of any callback received
     * after the previous timeout will be cleared here. */
    (void) xSemaphoreTake(BleEventSignal, pdMS_TO_TICKS(0));
    retcode = BlePeripheral_Start();
    if (RETCODE_OK == retcode)
    {
        if (pdTRUE != xSemaphoreTake(BleEventSignal, pdMS_TO_TICKS(BLE_EVENT_SYNC_TIMEOUT)))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_BLE_START_FAILED);
        }
        else if (BleEvent != BLE_PERIPHERAL_STARTED)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_BLE_START_FAILED);
        }
        else
        {
            /* Do not disturb retcode */;
        }
    }

    /* This is a dummy take. In case of any callback received
     * after the previous timeout will be cleared here. */
    (void) xSemaphoreTake(BleEventSignal, pdMS_TO_TICKS(0));
    if (RETCODE_OK == retcode)
    {
        BleWakeUpStatus = false;
        retcode = BlePeripheral_Wakeup();
    }
    if (RETCODE_OK == retcode)
    {
        if (pdTRUE != xSemaphoreTake(BleEventSignal, pdMS_TO_TICKS(BLE_EVENT_SYNC_TIMEOUT)))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_BLE_WAKEUP_FAILED);
        }
        else if (false == BleWakeUpStatus)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_BLE_WAKEUP_FAILED);
        }
        else
        {
            /* Do not disturb retcode */;
        }
    }
    return retcode;
}

/** Refer interface header for description */
bool BLE_IsConnected(void)
{
    return BleIsConnected;
}

/** Refer interface header for description */
Retcode_T BLE_SendData(uint8_t* dataToSend, uint8_t dataToSendLen, void * param, uint32_t timeout)
{
    Retcode_T retcode = RETCODE_OK;

    if (pdTRUE == xSemaphoreTake(BleSendGuardMutex, pdMS_TO_TICKS(BLE_EVENT_SYNC_TIMEOUT)))
    {
        if (BLE_IsConnected())
        {
            BleSendStatus = RETCODE_OK;
            /* This is a dummy take. In case of any callback received
             * after the previous timeout will be cleared here. */
            (void) xSemaphoreTake(BleSendCompleteSignal, pdMS_TO_TICKS(0));

            switch (BLESetup.Service)
            {
                case BLE_BCDS_BIDIRECTIONAL_SERVICE:
                retcode = BidirectionalService_SendData(dataToSend, dataToSendLen);
                if (RETCODE_OK == retcode)
                {
                    if (pdTRUE != xSemaphoreTake(BleSendCompleteSignal, pdMS_TO_TICKS(timeout)))
                    {
                        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_BLE_SEND_FAILED);
                    }
                    else
                    {
                        retcode = BleSendStatus;
                    }
                }
                break;

                case BLE_XDK_SENSOR_SERVICES:
                retcode = SensorServices_SendData(dataToSend, dataToSendLen, (SensorServices_Info_T *) param);
                if (RETCODE_OK == retcode)
                {
                    if (pdTRUE != xSemaphoreTake(BleSendCompleteSignal, pdMS_TO_TICKS(timeout)))
                    {
                        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_BLE_SEND_FAILED);
                    }
                    else
                    {
                        retcode = BleSendStatus;
                    }
                }
                break;

                case BLE_XDK_GRIDEYE_SERVICE:
                retcode = GridEyeService_SendData(dataToSend, dataToSendLen);
                if (RETCODE_OK == retcode)
                {
                    if (pdTRUE != xSemaphoreTake(BleSendCompleteSignal, pdMS_TO_TICKS(timeout)))
                    {
                        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_BLE_SEND_FAILED);
                    }
                    else
                    {

                        retcode = BleSendStatus;
                    }
                }
                break;

                default:
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NOT_SUPPORTED);
                break;
            }
        }
        else
        {
            retcode = RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_BLE_NOT_CONNECTED);
        }

        if (pdTRUE != xSemaphoreGive(BleSendGuardMutex))
        {
            /* This is fatal since the BleSendGuardMutex must be given as the same thread takes this */
            retcode = RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_BLE_SEND_MUTEX_NOT_RELEASED);
        }
    }
    else
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR);
    }
    return retcode;
}
#endif

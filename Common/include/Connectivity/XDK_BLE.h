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
 * @ingroup CONNECTIVITY
 *
 * @defgroup BLE BLE
 * @{
 *
 * @brief This module handles the BLE peripheral feature
 *
 * @file
 **/


/* header definition ******************************************************** */
#ifndef XDK_BLE_H_
#define XDK_BLE_H_

/* local interface declaration ********************************************** */
#include "BCDS_Retcode.h"
#include "BCDS_Ble.h"
#include "BCDS_BlePeripheral.h"
#include "BCDS_BidirectionalService.h"
#include "XDK_SensorServices.h"
#include "BCDS_SensorServices.h"

/**
 * @brief   Enum to represent the BLE services supported.
 */
enum BLE_Service_E
{
    BLE_BCDS_BIDIRECTIONAL_SERVICE,
    BLE_XDK_SENSOR_SERVICES,
    BLE_XDK_GRIDEYE_SERVICE,
    BLE_USER_CUSTOM_SERVICE
};

/**
 * @brief   Typedef to represent the BLE service supported.
 */
typedef enum BLE_Service_E BLE_Service_T;

/**
 * @brief  Typedef to notify the application about any incoming data
 *
 * @param[in]   rxBuffer
 * pointer to the received data buffer
 *
 * @param[in]   rxDataLength
 * Length of the received data
 *
 * @param[in]   param
 * unused if setup->Service is not BLE_XDK_SENSOR_SERVICES. Otherwise, of type #SensorServices_Info_T pointer.
 *
 * @note Do not perform any heavy processing within this function and return ASAP.
 */
typedef void (*BLE_DataRxCB_T)(uint8_t *rxBuffer, uint8_t rxDataLength, void * param);

/**
 * @brief   Structure to represent the BLE device characteristics values.
 */
struct BLE_DeviceCharacteristicsValue_S
{
    uint8_t * ModelNumber; /**< Pointer to the device model number */
    uint8_t * Manufacturer; /**< Pointer to the device manufacturer */
    uint8_t * SoftwareRevision; /**< Pointer to the device software revision */
};

/**
 * @brief   Typedef to represent the BLE device characteristics value.
 */
typedef struct BLE_DeviceCharacteristicsValue_S BLE_DeviceCharacteristicsValue_T;

/**
 * @brief   struct to represent the BLE setup features.
 */
struct BLE_Setup_S
{
    const char* DeviceName; /**< Pointer to the device name */
    bool IsMacAddrConfigured; /**< Boolean representing if the MAC address is to be configured */
    uint64_t MacAddr; /**< MAC Address to be configured. Unused if IsMacAddrConfigured is false */
    BLE_Service_T Service; /**< BLE peripheral service type */
    bool IsDeviceCharacteristicEnabled; /**< Boolean if device characteristics value should be added to the service information */
    BLE_DeviceCharacteristicsValue_T CharacteristicValue; /**< BLE characteristic value. Unused if IsDeviceCharacteristicEnabled is false. */
    BLE_DataRxCB_T DataRxCB; /**< Data receive callback. Unused if Service is BLE_USER_CUSTOM_SERVICE. */
    BlePeripheral_ServiceRegistryCallback CustomServiceRegistryCB; /**< Custom service callback. Unused if Service is not BLE_USER_CUSTOM_SERVICE. */
};

/**
 * @brief   Typedef to represent the BLE setup feature.
 */
typedef struct BLE_Setup_S BLE_Setup_T;

/**
 * @brief This will setup the BLE
 *
 * @param[in] setup
 * Pointer to the BLE setup feature
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - This must be the first API to be called if BLE feature is to be used.
 * - Do not call this API more than once.
 */
Retcode_T BLE_Setup(BLE_Setup_T * setup);

/**
 * @brief This will enable the BLE
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #BLE_Setup must have been successful prior.
 * - Do not call this API more than once.
 */
Retcode_T BLE_Enable(void);

/**
 * @brief This will tell if any peer is connected via BLE
 *
 * @return  Boolean representing if a peer is connected or not.
 */
bool BLE_IsConnected(void);

/**
 * @brief This will send the data via BLE
 *
 * @param[in]   dataToSend
 * The payload to be sent
 *
 * @param[in]   dataToSendLen
 * The length of the payload to be sent
 *
 * @param[in]   param
 * unused if setup->Service is not BLE_XDK_SENSOR_SERVICES. Otherwise, type casted to #SensorServices_Info_T pointer.
 *
 * @param[in]   timeout
 * Timeout for the payload to be sent
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 * @retval #RETCODE_BLE_NOT_CONNECTED if there is no peer connected.
 *
 * @note
 * - #BLE_Setup must have been successful prior.
 * - Do not call this API more than once.
 * - This is a blocking call guarded with a mutex of wait time of 1 second.
 * - This supports only BLE_BCDS_BIDIRECTIONAL_SERVICE and BLE_XDK_SENSOR_SERVICES.
 */
Retcode_T BLE_SendData(uint8_t* dataToSend, uint8_t dataToSendLen, void * param, uint32_t timeout);

#endif /* XDK_WLAN_H_ */

/**@} */

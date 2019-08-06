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

#ifndef SENSORSERVICES_H
#define SENSORSERVICES_H

/* Include all headers which are needed by this file. */
#include "XDK_SensorServices.h"

/* Put the type and macro definitions here */

/**
 * @brief  Typedef to notify the application about any incoming data
 * for any of the registered sensor services.
 *
 * @details  Typedef to notify the application about any incoming data
 * for any of the registered sensor services.
 *
 * @param[in]   rxBuffer
 * pointer to the received data buffer
 *
 * @param[in]   rxDataLength
 * Length of the received data
 *
 * @param[in]   sensorServicesInfo
 * Consists of the necessary service information of the received characteristic.
 */
typedef void (*SensorServices_DataReceivedCallBack)(uint8_t *rxBuffer, uint8_t rxDataLength, SensorServices_Info_T * sensorServicesInfo);

/**
 * @brief  Typedef to send status notification.
 * @details  Typedef to send status notification.
 *
 * @param[in]   sendStatus
 * Status of the send operation
 *
 * @note
 * For every single SensorServices_SendData, this must be synchronized before triggering a new send.
 * If the interface user wants to have a queuing mechanism for sending data where the user need not
 * worry about this notification and shall load the send API per need, it is excepted to be implemented
 * as a wrapper at the application end on top of this.
 */
typedef void (*SensorServices_SendEventCallback)(Retcode_T sendStatus);

/**
 * @brief   Initialize application callback to intimate for events from sensor services
 * @details   Initialize application callback to intimate for events from sensor services
 *
 * @param[in]   readCallback
 * Application Callback that will be called on data reception
 * for any sensor services characteristics
 *
 * @param[in]   sendCallback
 * Application Callback that will be notified about the data send status.
 *
 * @return  RETCODE_OK on success, or an error code otherwise. Refer #Retcode_General_E and #Ble_Retcode_E for other values.
 *
 * @see SensorServices_SendData
 */
Retcode_T SensorServices_Init(SensorServices_DataReceivedCallBack readCallback, SensorServices_SendEventCallback sendCallback);

/**
 * @brief  This function registers the Sensor Services at the ATT server.
 *         This must be done during the boot phase of the stack.
 * @details  This function registers the Sensor Services at the ATT server.
 *         This must be done during the boot phase of the stack.
 *
 * @see BlePeripheral_ServiceRegistryCallback
 *
 * @return  RETCODE_OK on success, or an error code otherwise. Refer #Retcode_General_E and #Ble_Retcode_E for other values.
 *
 * @note Sensor Services must have been successfully initialized prior.
 */
Retcode_T SensorServices_Register(void);

/**
 * @brief   This function sends data to the client
 * @details   This function sends data to the client
 *
 * @param[in]   dataToSend
 * The payload to be sent
 *
 * @param[in]   dataToSendLen
 * The length of the payload to be sent
 *
 * @param[in]   param
 * Provides the service characteristics information
 *
 * @return  RETCODE_OK on success, or an error code otherwise. Refer #Retcode_General_E and #Ble_Retcode_E for other values.
 *
 * @note
 * Sensor Services must have been successfully initialized and registered prior.
 * SensorServices_SendEventCallback will be triggered to provide the status of every individual send's.
 *
 * @see SensorServices_SendEventCallback
 */
Retcode_T SensorServices_SendData(uint8_t* payload, uint8_t payloadLen, SensorServices_Info_T * sensorServiceInfo);

/**
 * @brief   This function notifies the BLE connection status
 * @details  This function notifies the BLE connection status
 *
 * @param[in]   connectionStatus
 * BLE connection status
 *
 */
void SensorServices_UpdateConnectionStatus(bool connectionStatus);

#endif /* XDK_SENSORSERVICES_H */
/**@} */

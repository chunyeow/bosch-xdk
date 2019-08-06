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
 * This module handles the GridEye Service
 *
 */


#ifndef GRIDEYESERVICE_H_
#define GRIDEYESERVICE_H_

/* Include all headers which are needed by this file. */
#include "BCDS_Retcode.h"

/* Put the type and macro definitions here */

/**
 * @brief This is function template which is needs to be implemented by the application in order to receive data from the BLE central device
 *
 * @details  Typedef to notify the application about any incoming data
 *
 * @param[in]   rxBuffer
 * pointer to the received data buffer
 *
 * @param[in]   rxDataLength
 * Length of the received data
 *
 * @param[in]   param
 * Unused
 *
 * @note This is not used as of now for this application, But this is give as an example for the user, to configure some parameters of GridEye sensor
 */
typedef void (*GridEyeService_DataReceivedCallBack)(uint8_t *rxBuffer, uint8_t rxDataLength, void * param);

/**
 * @brief This is the callback triggered after the successful sending of data over BLE,
 *        the semaphore is released here to synchronize the calling task to initiate next BLE data transfer.
 *
 * @param[in]   sendStatus
 * Status of the send operation
 *
 */
typedef void (*GriEyeService_SendEventCallback)(Retcode_T sendStatus);

/* Put the function declarations here */

/**
 * @brief   Initializes the GridEye Service
 * @brief   Initializes the GridEye Service with a readCallback and a sendCallback to be implemented by the application.
 *
 * @param[in]   readCallback
 * Application Callback that will be called on data reception
 * for any data reception
 *
 * @param[in]   sendCallback
 * Application Callback that will be notified about the data send status.
 *
 * @return  RETCODE_OK on success, or an error code otherwise. Refer #Retcode_General_E and #Ble_Retcode_E for other values.
 *
 * @see GridEyeService_SendData
 */
Retcode_T GridEyeService_Init(GridEyeService_DataReceivedCallBack readCallback, GriEyeService_SendEventCallback sendCallback);

/**
 * @brief This contains the implementation to register the GridEye Service to the BLE stack
 *
 * @return RETCODE_OK on success, or an error code otherwise. Refer #Retcode_General_E and #Ble_Retcode_E for other values.
 *
 * @note GridEye Services must have been successfully initialized prior to this call.
 **/
Retcode_T GridEyeService_Register(void);

/**
 * @brief      The function is used to transmit data to ble to the BLE GridEye Sensor Service
 *
 * @param[in]   payload: pointer holding the data to be transmitted over BLE
 *
 * @param[in]   payloadLen: length of the data to be transmitted
 *
 * @retval RETCODE_OK in the case of success or an error code otherwise.
 *
 * @note This function is non-blocking call and hence a signalling mechanism like semaphore needed for proper sending of data
 */
Retcode_T GridEyeService_SendData(uint8_t* payload, uint8_t payloadLen);

/**
 * @brief   This function notifies the BLE connection status
 * @details  This function notifies the BLE connection status
 *
 * @param[in]   connectionStatus
 * BLE connection status
 *
 */
void GridEyeService_UpdateConnectionStatus(bool connectionStatus);

#endif /* GRIDEYESERVICE_H_ */
/**@} */

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
#ifndef BLE_DEVICEINFO_SERVICE_H
#define BLE_DEVICEINFO_SERVICE_H

#include "XDK_BLE.h"

typedef BLE_DeviceCharacteristicsValue_T BleDeviceInformationService_CharacteristicValue_T;

/**
 *@brief This is to configure the user characteristics value for the BLE standard Device Information Service .

 * Prerequisite: Failing to call this API before BleDeviceInformationService_Register() would result in unknown characteristics value
 * 					for Device Information Service
 *
 * @param characteristicValue structure to hold the characteristic value.
 *
 * @retval #RETCODE_OK  if success
 *         #RETCODE_FAILURE for error case
 *
 *
 */
Retcode_T BleDeviceInformationService_Initialize(BleDeviceInformationService_CharacteristicValue_T characteristicValue);

/**
 *@brief This API registers the BLE standard Device Information Service & needs to be called in the BLE GAP event callback function
 *		 registered using the BlePeripheral_Initialize() API for proper operation.
 *
 * @retval RETCODE_OK if BLE Service Registered successfully
 *		   RETCODE_FAILURE Failed to Register the Ble Device Info Service
 */
Retcode_T BleDeviceInformationService_Register(void);

#endif /* BLE_DEVICEINFO_SERVICE_H */

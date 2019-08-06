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
 *  @ingroup CONFIGURATION
 *
 *  @defgroup BCDS_BLUETOOTHCONFIG BCDS_BluetoothConfig
 *  @{
 *
 *  @brief configurations for the BCDS Bluetooth components of XDK110 system.
 *
 *  @details
 *
 *  @file
 *
 */

#ifndef BCDS_BLUETOOTHCONFIG_H_
#define BCDS_BLUETOOTHCONFIG_H_

/* additional interface header files */
#include "BCDS_TaskConfig.h"
/* public interface declaration */

/* public type and macro definitions */

/**< Default value is set to 4 because of stack's internal send buffer size. Minimum size is 1
Note: If this is called based on callback synchronization, it shall equate to atleast the number of threads calling it */
#define BLE_SEND_QUEUE_SIZE								(UINT32_C(4))

/**< Number of paired devices the BLE stack can keep track. Minimum count is 1*/
#define BLE_PAIRED_DEVICE_COUNT 						(UINT8_C(1))

/**< Minimum size is 1. Maximum size is 256*/
#define BLE_DEFERRED_RX_BUFFER_SIZE 					(UINT32_C(256))

/* public function prototype declarations */

/* public global variable declarations */

/* inline function definitions */

#endif /* BCDS_BLUETOOTHCONFIG_H_    */

/**@}*/
/** ************************************************************************* */

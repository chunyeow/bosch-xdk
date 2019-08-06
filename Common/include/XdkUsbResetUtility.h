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
 * @ingroup COMMON
 *
 * @defgroup USB_RESET_UTILITY USB Utility
 * @{
 *
 * @brief This is used to map the USB interrupts
 *
 * @file
 * ****************************************************************************/

/* header definition ******************************************************** */
#ifndef XDKUSBRESETUTILITY_H_
#define XDKUSBRESETUTILITY_H_

/* public interface declaration ********************************************* */

/* public type and macro definitions */
#include "BCDS_Retcode.h"

/**
 * @brief Typedef to define the function prototype to notify the data received information via. the USB interface.
 */
typedef void (*UsbResetUtility_UsbAppCallback_T)(uint8_t *, uint32_t);

/* public function prototype declarations */

/* public global variable declarations */

/**
 * @brief This API will initialize this module by allocating the necessary resources.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
Retcode_T UsbResetUtility_Init(void);

/**
 * @brief This API is used to register the application callback
 *
 * @param[in]   UsbResetUtility_UsbAppCallback_T
 * User defined function to be notified of the USB data received.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #UsbResetUtility_Init must have been successful prior.
 * - usbAppCallback is called from deferred ISR and user should consider this in application handling
 */
Retcode_T UsbResetUtility_RegAppISR(UsbResetUtility_UsbAppCallback_T usbAppCallback);

/**
 * @brief This API will de-initialize this module.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #UsbResetUtility_Init must have been successful prior.
 */
Retcode_T UsbResetUtility_DeInit(void);

#endif /* XDKUSBRESETUTILITY_H_ */

/**@}*/
/** ************************************************************************* */

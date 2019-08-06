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
 * @brief Private header for LWM2M file.
 *
 * @file
 */

#ifndef LWM2M_H_
#define LWM2M_H_

#include "XDK_LWM2M.h"

/* public type and macro definitions */

/**
 * Callback handler for LED state changes.
 *
 * @param[in] state
 * new state of LED. true for LED on, false, for off
 */
typedef void (*LWM2M_LedStateChangeHandler_T)(bool state);

/**
 * @brief Reboot the device.
 *
 * When the function is called first, it reboots with a delay of 5s to have time to send a response.
 * Though reboot is sometimes used for undetermined working devices, the delay of 5 s may not work.
 * Therefore, a second call of this function reboots the device immediately (and without sending a response).
 */
void LWM2M_Reboot(void);

/**
 * @brief Set state of the red LED.

 * Set state of the red LED and reports that to the associated handler.
 * @see LWM2M_SetRedLedStateChangeHandler
 *
 * @param[in] on
 * state of LED
 */
void LWM2M_RedLedSetState(bool on);

/**
 * @brief Set state of the orange LED.
 *
 * Set state of the orange LED and reports that to the associated handler.
 * @see LWM2M_SetOrangeLedStateChangeHandler
 *
 * @param[in] on
 * state of LED
 */
void LWM2M_OrangeLedSetState(bool on);

/**
 * @brief Set state of the yellow LED.
 *
 * Set state of the yellow LED and reports that to the associated handler.
 * @see LWM2M_SetOrangeLedStateChangeHandler
 *
 * @param[in] on
 * state of LED
 */
void LWM2M_YellowLedSetState(bool on);

/**
 * @brief Set change handler for red LED.
 *
 * @param[in] handler
 * callback for LED state changes
 */
void LWM2M_SetRedLedStateChangeHandler(LWM2M_LedStateChangeHandler_T handler);

/**
 * @brief Set change handler for orange LED.
 *
 * @param[in] handler
 * callback for LED state changes
 */
void LWM2M_SetOrangeLedStateChangeHandler(LWM2M_LedStateChangeHandler_T handler);

/**
 * @brief Set change handler for yellow LED.
 *
 * @param[in] handler
 * callback for LED state changes
 */
void LWM2M_SetYellowLedStateChangeHandler(LWM2M_LedStateChangeHandler_T handler);

/**
 * @brief Reboots the device.
 * Prints last message and wait before reboot the device, if SERVAL_LOG_LEVEL is at least SERVAL_LOG_LEVEL_ERROR.
 *
 * @param[in]
 * msg message to be printed
 */
void LWM2M_RebootNow(const char *msg);

LWM2M_Setup_T * LWM2MGetCredentials(void);

#endif /* LWM2M_H_ */

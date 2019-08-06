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
 * @defgroup LED LED
 * @{
 *
 * @brief This module handles the LED features.
 * The LED control are not thread safe, yet.
 * It is expected that the interface user manages this.
 *
 * @file
 */

/* header definition ******************************************************** */
#ifndef XDK_LED_H_
#define XDK_LED_H_

/* local interface declaration ********************************************** */
#include "BCDS_Retcode.h"

/**
 * @brief   Enum to represent the inbuilt and external LED's.
 */
enum LED_E
{
    LED_INBUILT_INVALID = 0x00,
    LED_INBUILT_RED = 0x01,
    LED_INBUILT_YELLOW = 0x02,
    LED_INBUILT_ORANGE = 0x04
};

/**
 * @brief   Typedef to represent the inbuilt and external LED.
 */
typedef enum LED_E LED_T;

/**
 * @brief   Enum to represent the patterns of LED's supported.
 */
enum LED_Pattern_E
{
    LED_PATTERN_ROLLING, /**< This is roll through all the in-built LED's periodically forward and reverse */
};

/**
 * @brief   Typedef to represent the pattern of LED's supported.
 */
typedef enum LED_Pattern_E LED_Pattern_T;

/**
 * @brief This will setup the LED's available on-board
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
Retcode_T LED_Setup(void);

/**
 * @brief This will enable the LED's available on-board
 *
 * @note By default these are Powered OFF
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
Retcode_T LED_Enable(void);

/**
 * @brief This will power ON a particular LED
 *
 * @param[in]   led
 * The LED to be powered ON (More the one LED can be provided with a bit-wise OR)
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
Retcode_T LED_On(LED_T led);

/**
 * @brief This will power OFF a particular LED
 *
 * @param[in]   led
 * The LED to be powered OFF (More the one LED can be provided with a bit-wise OR)
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
Retcode_T LED_Off(LED_T led);

/**
 * @brief This will toggle a particular LED
 *
 * @param[in]   led
 * The LED to be toggled (More the one LED can be provided with a bit-wise OR)
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
Retcode_T LED_Toggle(LED_T led);

/**
 * @brief This will blink the LED based on the user requested configuration.
 *
 * @param[in]   enable
 * Boolean to enable/disable LED blink
 *
 * @param[in]   ledToBlink
 * LED to blink (More the one LED can be provided for blinking with a bit-wise OR)
 * Unused if 'enable' is 'false'
 *
 * @param[in]   onTimeInMs
 * LED ON time while blinking in ms
 * Unused if 'enable' is 'false'
 *
 * @param[in]   offTimeInMs
 * LED OFF time while blinking in ms
 * Unused if 'enable' is 'false'
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note - #LED_Blink when enabled, it is expected for the interface user to disable
 * prior to other LED related API calls to avoid misbehavior.
 * It uses software timer internally which is shared with #LED_Pattern API.
 * So user is expected to avoid using them in parallel.
 */
Retcode_T LED_Blink(bool enable, LED_T ledToBlink, uint32_t onTimeInMs, uint32_t offTimeInMs);

/**
 * @brief This will provide a LED pattern based on the user requested configuration
 *
 * @param[in]   enable
 * Boolean to enable/disable LED pattern
 *
 * @param[in]   pattern
 * LED pattern to be enabled
 * Unused if 'enable' is 'false'
 *
 * @param[in]   statusTimeInMs
 * LED status time until which an ongoing LED pattern change must be held in ms
 * Unused if 'enable' is 'false'
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note - #LED_Pattern when enabled, it is expected for the interface user to disable
 * prior to other LED related API calls to avoid misbehavior.
 * It uses software timer internally which is shared with #LED_Blink API.
 * So user is expected to avoid using them in parallel.
 */
Retcode_T LED_Pattern(bool enable, LED_Pattern_T pattern, uint32_t statusTimeInMs);

/**
 * @brief This will provide the status of a particular LED
 *
 * @param[in]   led
 * The LED to be read for status
 *
 * @param[in/out]   status
 * Status on the IO port. User needs to provide memory for the same.
 *
 * @note This will read the value on the IO port and provide the same to the user
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
Retcode_T LED_Status(LED_T led, uint8_t * status);

#endif /* XDK_LED_H_ */

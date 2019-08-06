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
 * @defgroup BATTERYMONITOR Battery Monitor
 * @{
 *
 * @brief This module handles all the battery related activities.
 *
 * @details This module is used to initialize battery charger & measure the battery voltage.
 *
 * @file
 */
#ifndef BATTERYMONITOR_H_
#define BATTERYMONITOR_H_

/* Include all headers which are needed by this file. */
#include "BCDS_Retcode.h"
/* Put the type and macro definitions here */

/* Put the function declarations here */
/**
 * @brief Measures the specified voltage signal
 *
 * @details: the analog signals from the charger routed to the MCU could be fetched using this function
 *
 * This function once called will return the value of the specified signal in millivolts.
 *
 * @param[out]  outputVoltage: measured battery voltage in millivolts
 *
 * @retval  RETCODE_OK in case of successfully getting the battery voltage.
 * @retval  RETCODE_NULL_POINTER in case of NULL pointer passed.
 * @retval  RETCODE_BSP_CHARGER_NOT_ENABLED in case of accessing before enabling the charger.
 */
Retcode_T BatteryMonitor_MeasureSignal( uint32_t* outputVoltage);
/**
 * @brief Initialize the Battery monitor module.
 *
 * @details: Enables the Battery Charger & Initialize the Battery monitor to measure the battery voltage
 *
 * @retval  RETCODE_OK in case of successfully Initializing the Battery monitor.
 * @retval  RETCODE_SEMAPHORE_ERROR in case not able to create Semaphore.
 */
Retcode_T BatteryMonitor_Init(void);
#endif /* XDK110_BATTERYMONITOR_H_ */
/**@} */


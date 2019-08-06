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
 * @ingroup SENSOR
 *
 * @defgroup LEM LEM sensor
 * @{
 *
 * @brief This Module is Configuration header for Lem Sensor module.
 *
 * @file
 */

#ifndef XDK_LEM_H_
#define XDK_LEM_H_

#include "BCDS_Retcode.h"

/* Put the type and macro definitions here */

/**
 * @brief   Initialize the required components and variables of lem sensor module
 *
 * @param[out]    currentRatedTransformationRatio
 *            Current transformation ratio for the core used in lemsensor
 *
 * @note : example- Rated transformation ratio is 44.44 for 75Ampere split core current transformer
 *
 * @return #RETCODE_OK upon successful initialization or error code otherwise
 */
Retcode_T Lem_Setup(float currentRatedTransformationRatio);

/**
 * @brief function to initialize the ADC, DMA , Board detect pin, Timer required
 * for the LEM sensor sampling & Starts the sampling timer.
 *
 * @return   Status of the LemSensor Initialization
 *            - RETCODE_OK if the initialization is successful
 *            - Error for failure
 */
Retcode_T Lem_Enable(void);

/**
 * @brief function to Deinitialize lemsensor module.
 *
 * @return   Status of the LemSensor Deinitialization
 *            - RETCODE_OK if the Deinitialization is successful
 *            - Error for failure
 */
Retcode_T Lem_Disable(void);

/**
 * @brief function to get the RMS voltage.
 *
 * @param[out]    rmsvoltage:  To get the LEM Sensor RMS voltage in volts.
 *
 * @return   Status of the Getting the LemSensor Rms Voltage
 *            - RETCODE_OK if the RMS voltage got successfully
 *            - Error for failure.
 *
 * @note     On every #SAMPLING_WINDOW_CYCLES the RMS voltage is updated.
 *
 */
Retcode_T Lem_GetRmsVoltage(float * rmsvoltage);
/**
 * @brief function to get the RMS current.
 *
 * @param[out]    rmsCurrent:  To get the LEM Sensor RMS current in ampere.
 *
 * @return   Status of the Getting the LemSensor Rms currnt
 *            - RETCODE_OK if the RMS current got successfully
 *            - Error for failure.
 *
 */
Retcode_T Lem_GetRmsCurrent(float * rmsCurrent);

#endif /* XDK_LEM_H_ */

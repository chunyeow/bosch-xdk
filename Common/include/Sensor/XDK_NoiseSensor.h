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
 * @defgroup NOISESENSOR Noise sensor
 * @{
 *
 * @brief  Noise Sensor Interface.
 *
 * @file
 */

#ifndef XDK_NOISESENSOR_H_
#define XDK_NOISESENSOR_H_

#include "BCDS_HALConfig.h"

#ifndef BCDS_FEATURE_BSP_MIC_AKU340
#define BCDS_FEATURE_BSP_MIC_AKU340         0
#warning BCDS_FEATURE_BSP_MIC_AKU340 was not defined in BCDS_HALConfig.h file. Disabling Noise Sensor feature.
#endif /* BCDS_FEATURE_BSP_MIC_AKU340 */

#if BCDS_FEATURE_BSP_MIC_AKU340
#include "BCDS_Retcode.h"

/**
 * @brief   Initialize the required components and variables of noise sensor module
 *
 * @param[in]   samplingFreqency
 * Adc Sampling frequency in hertz (Hz)
 *
 * @retval #RETCODE_OK upon successful initialization
 * @retval #RETCODE_RTOS_QUEUE_ERROR if freertos queue create fails
 */
Retcode_T NoiseSensor_Setup(uint32_t samplingFreqency);

/**
 * @brief   Enable the noise sensor, Configure the ADC parameters and start ADC scan
 *
 * @retval #RETCODE_OK on success
 * @retval #RETCODE_NULL_POINTER if the Adc resource handle is NULL
 * @return In case of any other error code refer #AdcCentral_StartScan
 *
 * @note
 * Channel-4 is used for ADC to read the noise sensor data.
 * ADC Resolution as 12 bit
 */
Retcode_T NoiseSensor_Enable(void);

/**
 * @brief   To read the last calculated RMS voltage
 *
 * @param[out]   rmsValue
 *               Calculated RMS voltage for the range of 256 samples
 *
 * @param[in]   timeout in milli seconds
 *              Timeout to read the RmsVoltage
 *
 * @retval #RETCODE_OK upon successful execution
 * @retval #RETCODE_RTOS_QUEUE_ERROR if queue create fails
 * @retval #RETCODE_NULL_POINTER if the pointer is NULL
 * @retval #RETCODE_NOISE_SENSOR_ENABLE_FAILED if noise sensor enable fails
 * @retval #RETCODE_NO_DATA_ON_QUEUE if no data available on queue
 *
 * @note
 * If the calculated value isn't available in queue then
 * it will wait until the data availability
 */
Retcode_T NoiseSensor_ReadRmsValue(float *rmsValue, uint32_t timeout);

/**
 * @brief   Disable the noise sensor module and stop adc scan
 *
 * @retval #RETCODE_OK upon successful execution
 * @return In case of any other error code refer #AdcCentral_StopScan
 */
Retcode_T NoiseSensor_Disable(void);

/**
 * @brief   To close noise sensor module
 *
 * @retval  #RETCODE_OK upon successful execution
 */
Retcode_T NoiseSensor_Close(void);

#endif  /* BCDS_FEATURE_BSP_MIC_AKU340 */

#endif /* XDK_NOISESENSOR_H_ */

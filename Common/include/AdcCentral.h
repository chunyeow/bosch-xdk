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
 * @defgroup MCU_ADC ADC
 * @{
 *
 * @brief This module handles the ADC Channel configurations, manages the buffers & callbacks
 * for the multiple ADC users centrally and independently
 * @file
 **/
#ifndef XDK_ADCCENTRAL_H_
#define XDK_ADCCENTRAL_H_

/* Include all headers which are needed by this file. */
#include "Mcu_Adc.h"

/* Put the type and macro definitions here */

/**
 * @brief Application callback for scan channel configuration
 * @param[out] adc MCU handle for ADC
 * @param[out] samplingFrequency structure holding data and its sampling frequency
 * @param[out] buffer pointer pointing to the ADC samples
 */
typedef void (*AdcCentral_ScanCallback_T)(ADC_T adc, uint32_t samplingFrequency, uint16_t* buffer);


/**
 * @brief Application callback template for single channel configuration
 * @param[in] adc MCU handle for ADC
 * @param[out] buffer pointer pointing to the ADC samples
 */
typedef void (*AdcCentral_SingleCallback_T)(ADC_T adc, uint16_t* buffer);

/**
 * @brief struct to configure ADC in single mode
 */
struct AdcCentral_ConfigSingle_S
{
    Adc_Channel_T Channel;
    uint16_t* BufferPtr;
    Adc_Reference_T Reference;
    Adc_Resolution_T Resolution;
    Adc_AcqTime_T AcqTime;
    AdcCentral_SingleCallback_T Appcallback;
};
typedef struct AdcCentral_ConfigSingle_S AdcCentral_ConfigSingle_T, *AdcCentral_ConfigSinglePtr_T;

/**
 * @brief struct to configure ADC in scan mode
 */
struct AdcCentral_ConfigScan_S
{
    uint32_t ChannelScanMask;
    uint16_t *BufferPtr;
    uint32_t SamplingRateInHz;
    AdcCentral_ScanCallback_T Appcallback;
};
typedef struct AdcCentral_ConfigScan_S AdcCentral_ConfigScan_T, *AdcCentral_ConfigScanPtr_T;
/* Put the function declarations here */

/**
 * @brief   Initialize the ADC Central Module.
 *
 * @retval  RETCODE_OK upon successful execution, error otherwise
 */
Retcode_T AdcCentral_Init(void);

/**
 * @brief   Start Single Mode on ADC Interface.
 *
 * @param [in]  adc : ADC handle.
 *
 * @param [in] configStart : Structure which holds the configurations(Resolution, ADC reference,callback) related to single mode.
 *
 * @retval  RETCODE_OK upon successful execution, error otherwise
 */
Retcode_T AdcCentral_StartSingle(ADC_T adc, AdcCentral_ConfigSinglePtr_T configStart);

/**
 * @brief   Start Scan Mode on ADC Interface & Start the PRS Timer as producer.
 *
 * @param [in]  adc : ADC handle.
 *
 * @param [in] configScan : Structure which holds the configurations related to scan mode.
 *
 * @Note Three channels(CH4, CH5 & CH6) are supported for the ADC scan & maximum 3 independent users can be possible.
 * 		 Number of samples is fixed to 256 samples per channel. So if multiple channels are enabled by single user then provide buffer accordingly.
 * 		 ADC sampling is done with the user which has the highest sampling frequency. Lower sampling frequency users are notified with ADC callback on multiples of higher sampling frequency user.
 * 		 Independent sampling frequency works better when lower sampling frequency is multiples of higher sampling frequency. So when ADC Scan complete callback is triggered for
 * 		 the users actual frequency at which the adc samples provided is notified with the sampleFrequency parameter in the #AdcCentral_SingleCallback_T callback.
 *
 * @retval  RETCODE_OK upon successful execution, error otherwise
 */
Retcode_T AdcCentral_StartScan(ADC_T adc, AdcCentral_ConfigScanPtr_T configScan);

/**
 * @brief   Stop the Scan mode on ADC Interface and stops the PRS timer used for sampling adc channels.
 *
 * @param [in]  adc : ADC handle.
 *
 * @param [in]  ChannelScanMask : Channel in which scan has to be stopped
 *
 * @retval RETCODE_OK upon successful execution, error otherwise
 */
Retcode_T AdcCentral_StopScan(ADC_T adc,uint32_t ChannelScanMask);

/**
 * @brief   De-Initialize the ADC Module.
 *
 * @retval  RETCODE_OK upon successful execution, error otherwise
 */
Retcode_T AdcCentral_DeInit(void);

#endif /* XDK_ADCCENTRAL_H_ */
/**@} */


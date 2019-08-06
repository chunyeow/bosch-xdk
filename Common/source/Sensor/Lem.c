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
 * This module handles the LEM sensor features.
 *
 * @brief Lem-Sensor signal sampling
 * @details This module samples the ADC5 & ADC6 channels and calculates the RMS voltage which is updated on the window period configured
 * by the SAMPLING_WINDOW_CYCLES
 *
 */

#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_LEMSENSOR

#include <math.h>
#include "XDK_Lem.h"
#include "BSP_BoardType.h"
#include "FreeRTOS.h"
#include "BCDS_Assert.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "BCDS_MCU_Timer.h"
#include "BCDS_MCU_Timer_Handle.h"
#include "em_adc.h"
#include "em_dma.h"
#include "em_gpio.h"
#include "BSP_Adc.h"
#include "Mcu_Adc_Handle.h"
#include "Mcu_Adc.h"
#include "AdcCentral.h"
#include "AdcCentralConfig.h"
#include "BSP_LemSensor.h"

/* Put the type and macro definitions here */
#define TIMER_SAMPLING_FREQ             5000UL  /**< ADC Signal sampling frequency in Hz */
#define NO_OF_CHANNELS_USED				UINT32_C(2)
#define ADC_BUFFER_SIZE					(ADC_CENTRAL_NO_OF_SAMPLES * NO_OF_CHANNELS_USED)

static uint16_t ScanData[ADC_BUFFER_SIZE]; /**< LEM sensor ADC user buffer for CH5 */
static volatile float RMSVoltageSumOfSquares = 0.0f; /**< RMS Voltage on Ch5 */
static float CurrentRatedTransformationRatio = 0.0f;/**< Current rated transformation ratio for lem sensor*/
/**
 * @brief ADC Central callback function which calculates the RMS voltage for the Differential signal from the  AD5 & AD6 channels.
 *
 * @param[out] adc handle not used.
 *
 * @param[out] SamplingFreq which tells the adc samples collected frequency.
 *
 * @param[out] bufferPtr which is pointer to the application provided adc buffer which holds adc samples for CH5 & Ch6.
 *
 * @Note ADC central always provides 256 fixed number of adc samples per channel so the expecting the users to provide the buffer accordingly.
 *
 */
static void CalculateRunningRMS(ADC_T adc, uint32_t SamplingFreq, uint16_t* bufferPtr)
{
    BCDS_UNUSED(adc);
    BCDS_UNUSED(SamplingFreq);

    float VoltageCh5;
    float VoltageCh6;
    float Voltage = 0.0f;
    float sumOfSquares = 0.0f;
    uint32_t sumCH1 = 0;
    uint32_t sumCH2 = 0;

    /* Activate DMA for sampling */
    sumCH1 = 0;
    sumCH2 = 0;
    for (uint32_t counter = 0; counter < (ADC_BUFFER_SIZE); counter += NO_OF_CHANNELS_USED)
    {
        sumCH1 = *(bufferPtr + counter);
        sumCH2 = *(bufferPtr + (counter + 1));
        VoltageCh5 = (((float) (sumCH1)) * ADC_CONVERSION_FACTOR) / (4095.0f); /* Converting ADC Value to Voltage */
        VoltageCh6 = (((float) (sumCH2)) * ADC_CONVERSION_FACTOR) / (4095.0f); /* Converting ADC Value to Voltage */
        Voltage = VoltageCh6 - VoltageCh5; /* Calculate the differential Voltage */
        sumOfSquares += (Voltage * Voltage); /* Calculation the Square of the voltages */
    }
    RMSVoltageSumOfSquares = sumOfSquares;
}

/* Refer Interface Header for API documentation */
Retcode_T Lem_Setup(float currentRatedTransformationRatio)
{
    Retcode_T retcode = RETCODE_OK;
    if (currentRatedTransformationRatio)
    {
        CurrentRatedTransformationRatio = currentRatedTransformationRatio;
    }
    else
    {
        retcode = RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_INVALID_PARAM);
    }
    return retcode;
}

/* Refer Interface Header for API documentation */
Retcode_T Lem_Enable(void)
{
    Retcode_T retcode = RETCODE_OK;
    AdcCentral_ConfigScan_T configScan;

    /* Initialization of LEM Sensor */
    retcode = BSP_LemSensor_Connect();
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_LemSensor_Enable();
    }
    if (RETCODE_OK == retcode)
    {
        retcode = AdcCentral_Init();
    }
    if (RETCODE_OK == retcode)
    {
        configScan.BufferPtr = (uint16_t *) ScanData;
        configScan.Appcallback = CalculateRunningRMS;
        configScan.ChannelScanMask = (uint32_t) ADC_ENABLE_CH5_SCAN | (uint32_t) ADC_ENABLE_CH6_SCAN;
        configScan.SamplingRateInHz = TIMER_SAMPLING_FREQ;

        retcode = AdcCentral_StartScan(BSP_Adc_GetHandle(), &configScan);
    }
    return retcode;
}

/* Refer Interface Header for API documentation */
Retcode_T Lem_Disable(void)
{
    Retcode_T retcode = RETCODE_OK;
    retcode = AdcCentral_StopScan(BSP_Adc_GetHandle(), ((uint32_t) ADC_ENABLE_CH5_SCAN | (uint32_t) ADC_ENABLE_CH6_SCAN));
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_LemSensor_Disable();
    }
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_LemSensor_Disconnect();
    }
    return retcode;
}

/* Refer Interface Header for API documentation */
Retcode_T Lem_GetRmsVoltage(float * rmsvoltage)
{
    Retcode_T retcode = RETCODE_OK;
    if (NULL == rmsvoltage)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        *rmsvoltage = sqrt(RMSVoltageSumOfSquares / ADC_CENTRAL_NO_OF_SAMPLES);
    }
    return retcode;
}

/* Refer Interface Header for API documentation */
Retcode_T Lem_GetRmsCurrent(float * rmsCurrent)
{
    Retcode_T retcode = RETCODE_OK;
    if (NULL == rmsCurrent)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        *rmsCurrent = (sqrt(RMSVoltageSumOfSquares / ADC_CENTRAL_NO_OF_SAMPLES)) * CurrentRatedTransformationRatio;
    }
    return retcode;
}

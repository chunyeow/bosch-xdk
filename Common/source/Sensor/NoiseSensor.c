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

#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_COMMON_ID_NOISESENSOR

#include "BCDS_HAL.h"
#include "BCDS_HALConfig.h"
#include "XDK_NoiseSensor.h"

#if BCDS_FEATURE_BSP_MIC_AKU340

#include "AdcCentral.h"
#include "Mcu_Adc.h"
#include "BSP_Adc.h"
#include "BCDS_BSP_Mic_AKU340.h"
#include "BCDS_Retcode.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "XdkCommonInfo.h"
#include "AdcCentralConfig.h"

/* system header files */
#include <stdio.h>
#include <math.h>

static uint16_t CH4UserBuffer[ADC_CENTRAL_NO_OF_SAMPLES] = { 0 };
static HWHandle_T AdcHandle = NULL;
static QueueHandle_t RmsQueue = NULL;
static AdcCentral_ConfigScan_T ConfigScan;

static void ProcessCH4Data(ADC_T adc, uint32_t samplingFreq, uint16_t* bufferPtr)
{
    BCDS_UNUSED(adc);
    BCDS_UNUSED(samplingFreq);
    BCDS_UNUSED(bufferPtr);

    float sumOfSquares = 0.0f;
    float voltageCh4 = 0.0f;
    float rmsVoltage = 0.0f;
    uint32_t sumCH1 = 0;

    for (uint32_t counter = 0; counter < ADC_CENTRAL_NO_OF_SAMPLES; counter++)
    {
        sumCH1 = *(CH4UserBuffer + counter);
        voltageCh4 = (float) ((sumCH1 * ADC_CONVERSION_FACTOR) / (4095.0f));
        sumOfSquares += (voltageCh4 * voltageCh4);
    }

    rmsVoltage = sqrt(sumOfSquares / ADC_CENTRAL_NO_OF_SAMPLES);
    if (NULL != RmsQueue)
    {
        if (pdTRUE != xQueueOverwrite(RmsQueue, &rmsVoltage))
        {
            Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_QUEUE_WRITE_FAILED));
        }
    }
}

Retcode_T NoiseSensor_Setup(uint32_t samplingFreqency)
{
    Retcode_T retcode = RETCODE_OK;

    RmsQueue = xQueueCreate(1U, sizeof(float));
    if (NULL == RmsQueue)
    {
        printf("Failed to create queue \r\n");
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_RTOS_QUEUE_ERROR);
    }
    else
    {
        if (retcode == RETCODE_OK)
        {
            retcode = AdcCentral_Init();
        }
        if (retcode == RETCODE_OK)
        {
            ConfigScan.SamplingRateInHz = samplingFreqency;
        }
    }
    return retcode;
}

Retcode_T NoiseSensor_Enable(void)
{
    Retcode_T retcode = RETCODE_OK;

    retcode = BSP_Mic_AKU340_Connect();
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_Mic_AKU340_Enable();
    }
    if (RETCODE_OK == retcode)
    {
        AdcHandle = BSP_Adc_GetHandle();
        if (NULL == AdcHandle)
        {
            printf("Adc resource handle is NULL \r\n");
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
        }
        else
        {
            ConfigScan.BufferPtr = CH4UserBuffer;
            ConfigScan.Appcallback = ProcessCH4Data;
            ConfigScan.ChannelScanMask = (uint32_t) ADC_ENABLE_CH4_SCAN;

            retcode = AdcCentral_StartScan(AdcHandle, &ConfigScan);
        }
    }
    return retcode;
}

Retcode_T NoiseSensor_ReadRmsValue(float *rmsValue, uint32_t timeout)
{
    Retcode_T retcode = RETCODE_OK;
    uint32_t queueResult;
    float dataVal = 0.0;

    if (NULL == AdcHandle)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NOISE_SENSOR_ENABLE_FAILED);
    }
    else
    {
        if (NULL != RmsQueue && NULL != rmsValue)
        {
            queueResult = xQueueReceive(RmsQueue, &dataVal, (TickType_t) timeout);
            if (pdTRUE == queueResult)
            {
                *rmsValue = dataVal;
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NO_DATA_ON_QUEUE);
                *rmsValue = 0.0f;
            }
        }
        else
        {
            if (NULL == RmsQueue)
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_RTOS_QUEUE_ERROR);
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
            }
        }
    }

    return retcode;
}

Retcode_T NoiseSensor_Disable(void)
{
    Retcode_T retcode = RETCODE_OK;

    retcode = AdcCentral_StopScan(AdcHandle, ConfigScan.ChannelScanMask);
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_Mic_AKU340_Disable();
    }
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_Mic_AKU340_Disconnect();
    }
    if (RETCODE_OK == retcode)
    {
        AdcHandle = NULL;
    }
    return retcode;
}

Retcode_T NoiseSensor_Close(void)
{
    Retcode_T retcode = RETCODE_OK;

    /**< @todo - Call AdcCentral_DeInit in a safe way */

    return retcode;
}
#endif  /* BCDS_FEATURE_BSP_MIC_AKU340 */


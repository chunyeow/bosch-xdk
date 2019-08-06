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
/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_BATTERYMONITOR

#include "BatteryMonitor.h"

/* additional interface header files */
#include "AdcCentral.h"
#include "BCDS_BSP_Charger_BQ2407X.h"
#include "BSP_Adc.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "semphr.h"
#include "XdkCommonInfo.h"
/* Put the type and macro definitions here */
#define BATTERYMONITOR_ADC_RESOLUTION 4096 /**<ADC resolution is configured to 12 bit hence 2^12=4096 */
#define BATTERYMONITOR_ADC_REFERENCE  2500 /**<ADC reference voltage is configured to 2.5 Volts */

/* Put constant and variable definitions here */

static SemaphoreHandle_t AdcSampleSemaphore = NULL;
static bool IsInitialized = false;

void AdcSampleIRQCallback(ADC_T adc, uint16_t *buffer)
{
    BCDS_UNUSED(adc);
    BCDS_UNUSED(buffer);
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    if (pdTRUE == xSemaphoreGiveFromISR(AdcSampleSemaphore, &higherPriorityTaskWoken))
    {
        portYIELD_FROM_ISR(higherPriorityTaskWoken);
    }
    else
    {
        /* ignore... semaphore has already been given */
    }
}

/* Put private function declarations here */
Retcode_T BatteryMonitor_Init(void)
{
    Retcode_T retval = RETCODE_OK;

    if (true == IsInitialized)
    {
        return RETCODE_OK;
    }
    AdcSampleSemaphore = xSemaphoreCreateBinary();
    if (NULL == AdcSampleSemaphore)
    {
        retval = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR);
    }
    if (RETCODE_OK == retval)
    {
        retval = BSP_Charger_BQ2407X_Connect();
    }
    if (RETCODE_OK == retval)
    {
        retval = BSP_Charger_BQ2407X_Enable((uint32_t) BSP_XDK_CHARGING_SPEED_1);
    }
    if (RETCODE_OK == retval)
    {
        retval = AdcCentral_Init();
    }
    if (RETCODE_OK == retval)
    {
        IsInitialized = true;
    }
    return retval;
}

/* Put function implementations here */
Retcode_T BatteryMonitor_MeasureSignal(uint32_t* outputVoltage)
{
    Retcode_T retval = RETCODE_OK;
    AdcCentral_ConfigSingle_T singleConfig;
    uint32_t batteryVoltage = 0;

    singleConfig.AcqTime = ADC_ACQ_TIME_16;
    singleConfig.Appcallback = AdcSampleIRQCallback;
    singleConfig.BufferPtr = (uint16_t *) outputVoltage;
    singleConfig.Channel = ADC_ENABLE_CH7;
    singleConfig.Reference = ADC_REF_2V5;
    singleConfig.Resolution = ADC_RESOLUTION_12BIT;

    if (false == IsInitialized)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_BATTERY_MONITOR_NOT_INIT));
    }
    retval = AdcCentral_StartSingle(BSP_Adc_GetHandle(), &singleConfig);

    if (RETCODE_OK == retval)
    {
        if (pdTRUE != xSemaphoreTake(AdcSampleSemaphore, (TickType_t) pdMS_TO_TICKS(100)))
        {
            retval = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_TIMEOUT);
        }
        else
        {
            /* The below conversation converts the ADC output to actual battery voltage */
            batteryVoltage = ((2 * (*(uint16_t *) outputVoltage) * BATTERYMONITOR_ADC_REFERENCE) / BATTERYMONITOR_ADC_RESOLUTION);
            *outputVoltage = batteryVoltage;
        }
    }
    return retval;
}

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
#define BCDS_MODULE_ID XDK_COMMON_ID_ADCCENTRAL

#include "AdcCentral.h"
#include "Mcu_Adc.h"
#include "Mcu_Adc_Handle.h"
#include "BCDS_MCU_DMA_Handle.h"
#include "BCDS_GuardedTask.h"
#include "BSP_Adc.h"
#include "XdkCommonInfo.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "em_dma.h"
#include "AdcCentralConfig.h"

/* Put the type and macro definitions here */
#define MAX_NO_OF_SCAN_CHANNELS 3

#define ADC_DMA_BUFFER_SIZE (2*ADC_CENTRAL_NO_OF_SAMPLES*MAX_NO_OF_SCAN_CHANNELS)

/**
 * Enumeration of ADC central single conversion state machine states.
 */
typedef enum AdcCentral_SingleState_E
{
    ADC_SINGLE_STATE_INIT = 0,
    ADC_SINGLE_STATE_READY,
    ADC_SINGLE_CONVERSION_STATE
} AdcCentral_SingleState_T;

/**
 * Enumeration of ADC central Initialize state machine states.
 */
typedef enum AdcCentral_State_E
{
    ADC_CENTRAL_INIT = 0,
    ADC_CENTRAL_READY
} AdcCentral_State_T;

struct AdcCentral_ScanChannelInfo_S
{
    uint32_t Channelmask;
    uint32_t SamplingCounter;
    uint32_t SamplingFreq;
    uint16_t *Buffer;
    AdcCentral_ScanCallback_T Appcallback;
};

typedef struct AdcCentral_ScanChannelInfo_S AdcCentral_ScanChannelInfo_T;

struct AdcCentral_SingleChannelInfo_S
{
    AdcCentral_SingleState_T State;
    AdcCentral_SingleCallback_T Appcallback;
};
typedef struct AdcCentral_SingleChannelInfo_S AdcCentral_SingleChannelInfo_T;

struct AdcCentral_Info_S
{
    AdcCentral_State_T State;
    GuardedTask_T AdcScanGuardedTaskHandle;
    AdcCentral_ScanChannelInfo_T ScanChannel[MAX_NO_OF_SCAN_CHANNELS];
    SemaphoreHandle_t ScanChannelInfoLock;
    SemaphoreHandle_t ScanChannelConfigLock;
    AdcCentral_SingleChannelInfo_T SingleChannel;
    uint32_t ScanSamplingFreq;
    uint32_t ScanChannelMask;
    uint8_t ScanChannelInfoIndex;
    volatile bool IsScanChannelConfigChanged;
    uint16_t* ScanBufferPtr;
};
typedef struct AdcCentral_Info_S AdcCentral_Info_T;

static uint16_t scanDataBufffer[ADC_DMA_BUFFER_SIZE];
static volatile uint16_t *bufferPointer;

AdcCentral_Info_T AdcCentalInfo;

//static volatile uint8_t ADCScanConfigChanged = false;
/* Put constant and variable definitions here */

static bool IsScanChannelActive(Adc_Channel_T channel)
{
    uint32_t CurrentMask = AdcCentalInfo.ScanChannelMask >> 12;
    if ((CurrentMask >> (uint32_t) channel) & 0x01)
    {
        return true;
    }
    return false;
}

/* Put private function declarations here */
static bool IsChannelAlreadyActivated(uint32_t channelMask)
{
    uint32_t CurrentMask = AdcCentalInfo.ScanChannelMask >> 12;
    if ((channelMask != 0) || (CurrentMask != 0))
    {
        channelMask = channelMask >> 12;

        for (uint8_t i = 0; i < MAX_NO_OF_SCAN_CHANNELS; i++)
        {
            if ((channelMask >> i) & (CurrentMask >> i))
            {
                return true;
            }
        }
    }
    return false;
}

static Retcode_T findScanChannelsInfo(uint8_t* ChannelCnt, uint8_t* ChannelNo, uint32_t ChannelMask)
{
    Retcode_T retval = RETCODE_OK;

    if ((NULL == ChannelNo) || (NULL == ChannelCnt))
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    uint8_t noOfScanChannels = 0;
    if (ChannelMask != 0)
    {
        ChannelMask = ChannelMask >> 12;
        for (uint8_t i = 0; i < MAX_NO_OF_SCAN_CHANNELS; i++)
        {
            if ((ChannelMask >> i) & (0x01))
            {
                *(ChannelNo + noOfScanChannels) = i + 4;
                noOfScanChannels++;
            }
        }
    }
    *ChannelCnt = noOfScanChannels;
    return retval;
}

static uint8_t findindexOfScanChannel(uint32_t channelMask)
{
    uint8_t scanChannelIndex = 0;
    if (channelMask != 0)
    {
        for (uint8_t i = 0; i < MAX_NO_OF_SCAN_CHANNELS; i++)
        {
            if (AdcCentalInfo.ScanChannel[i].Channelmask == channelMask)
            {
                scanChannelIndex = i;
                break;
            }
        }
    }
    return scanChannelIndex;
}

void AdcIRQCallback(ADC_T adc, Mcu_Adc_Event_T event, uint16_t *buff)
{
    struct Mcu_Adc_Handle_S* adcPtr = (struct Mcu_Adc_Handle_S*) adc;
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    Retcode_T retval = RETCODE_OK;

    struct MCU_DMA_Channel_S *localDmaHandle = (struct MCU_DMA_Channel_S *) adcPtr->Link1RegPtr;
    uint32_t noOfDmaSamples = (adcPtr->_DriverCtx.NumOfSamples) * (adcPtr->_DriverCtx.NumOfScanChannels);

    if (event.ScanComplete == 1)
    {
        if (AdcCentalInfo.IsScanChannelConfigChanged)
        {
            AdcCentalInfo.IsScanChannelConfigChanged = false;
            if (pdTRUE == xSemaphoreGiveFromISR(AdcCentalInfo.ScanChannelConfigLock, &higherPriorityTaskWoken))
            {
                portYIELD_FROM_ISR(higherPriorityTaskWoken);
            }
            else
            {
                /* ignore... semaphore has already been given */
            }
        }
        else
        {
            uint32_t channel = localDmaHandle->ChannelId;
            bool primary = *((uint32_t*) adcPtr->_DriverCtx.PointerValue);
            DMA_RefreshPingPong(channel, primary, false, NULL, NULL, noOfDmaSamples - 1, false);
        }
        bufferPointer = buff;
        retval = GuardedTask_SignalFromIsr(&AdcCentalInfo.AdcScanGuardedTaskHandle);
    }
    else if (event.SingleComplete == 1)
    {
        AdcCentalInfo.SingleChannel.Appcallback(adc, buff);
        AdcCentalInfo.SingleChannel.State = ADC_SINGLE_STATE_READY;
    }
    else
    {
        retval = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_FAILURE);
    }
    if (retval != RETCODE_OK)
    {
        Retcode_RaiseErrorFromIsr(retval);
    }
}

static void receiveProcess(void)
{
    Retcode_T retval = RETCODE_OK;
    uint8_t TotalNumOfChannelsInScan = 0;
    uint8_t ChannelListinfo[3] = { 0 };
    uint32_t BufferPointer[2] = { 0 };

    if (pdTRUE == xSemaphoreTake(AdcCentalInfo.ScanChannelInfoLock, portMAX_DELAY))
    {
        retval = findScanChannelsInfo(&TotalNumOfChannelsInScan, ChannelListinfo, AdcCentalInfo.ScanChannelMask);
        if (RETCODE_OK != retval)
        {
            goto end;
        }

        if (AdcCentalInfo.ScanChannel[0].Channelmask == AdcCentalInfo.ScanChannelMask)
        {
            memcpy(AdcCentalInfo.ScanChannel[0].Buffer, (void *) bufferPointer, (ADC_CENTRAL_NO_OF_SAMPLES * TotalNumOfChannelsInScan * 2));
            AdcCentalInfo.ScanChannel[0].Appcallback(BSP_Adc_GetHandle(), AdcCentalInfo.ScanSamplingFreq, AdcCentalInfo.ScanChannel[0].Buffer);
            goto end;
        }

        else if (TotalNumOfChannelsInScan == MAX_NO_OF_SCAN_CHANNELS)
        {
            for (int8_t i = 0; i < MAX_NO_OF_SCAN_CHANNELS; i++)
            {
                retval = findScanChannelsInfo(&TotalNumOfChannelsInScan, ChannelListinfo, AdcCentalInfo.ScanChannel[i].Channelmask);
                if (RETCODE_OK != retval)
                {
                    goto end;
                }
                if (UINT8_C(2) == TotalNumOfChannelsInScan)
                {
                    AdcCentalInfo.ScanChannel[i].SamplingCounter++;
                    if (AdcCentalInfo.ScanChannel[i].SamplingCounter == AdcCentalInfo.ScanSamplingFreq / AdcCentalInfo.ScanChannel[i].SamplingFreq)
                    {
                        for (int8_t j = 0; j < 2; j++)
                        {
                            if (5 < ChannelListinfo[j])
                            {
                                BufferPointer[j] = (uint32_t) (bufferPointer + 2);

                            }
                            else if (5 > ChannelListinfo[j])
                            {
                                BufferPointer[j] = (uint32_t) bufferPointer;
                            }
                            else
                            {
                                BufferPointer[j] = (uint32_t) (bufferPointer + 1);
                            }
                        }
                        /* Populate the buffer data */
                        for (int32_t j = 0; j < ADC_CENTRAL_NO_OF_SAMPLES; j++)
                        {
                            *(AdcCentalInfo.ScanChannel[i].Buffer + ((j * 2) + 0)) = *((uint16_t*) BufferPointer[0]);
                            *(AdcCentalInfo.ScanChannel[i].Buffer + ((j * 2) + 1)) = *((uint16_t*) BufferPointer[1]);
                            BufferPointer[0] += 6;
                            BufferPointer[1] += 6;
                        }
                        AdcCentalInfo.ScanChannel[i].Appcallback(BSP_Adc_GetHandle(), AdcCentalInfo.ScanSamplingFreq / AdcCentalInfo.ScanChannel[i].SamplingCounter, AdcCentalInfo.ScanChannel[i].Buffer);
                        AdcCentalInfo.ScanChannel[i].SamplingCounter = 0;
                    }
                }
                else if (UINT8_C(1) == TotalNumOfChannelsInScan)
                {
                    AdcCentalInfo.ScanChannel[i].SamplingCounter++;
                    if (AdcCentalInfo.ScanChannel[i].SamplingCounter == AdcCentalInfo.ScanSamplingFreq / AdcCentalInfo.ScanChannel[i].SamplingFreq)
                    {
                        if (5 < ChannelListinfo[0])
                        {
                            BufferPointer[0] = (uint32_t) (bufferPointer + 2);
                        }
                        else if (5 > ChannelListinfo[0])
                        {
                            BufferPointer[0] = (uint32_t) bufferPointer;
                        }
                        else
                        {
                            BufferPointer[0] = (uint32_t) (bufferPointer + 1);
                        }
                        /* Populate the buffer data */
                        for (int32_t j = 0; j < ADC_CENTRAL_NO_OF_SAMPLES; j++)
                        {
                            *(AdcCentalInfo.ScanChannel[i].Buffer + j) = *((uint16_t*) BufferPointer[0]);
                            BufferPointer[0] += 6;
                        }
                        AdcCentalInfo.ScanChannel[i].Appcallback(BSP_Adc_GetHandle(), AdcCentalInfo.ScanSamplingFreq / AdcCentalInfo.ScanChannel[i].SamplingCounter, AdcCentalInfo.ScanChannel[i].Buffer);
                        AdcCentalInfo.ScanChannel[i].SamplingCounter = 0;
                    }
                }
            }
        }
        else if (TotalNumOfChannelsInScan == UINT8_C(2))
        {
            uint32_t buffptr = 0;
            uint8_t RunningChInfo[3] = { 0 };
            if (ChannelListinfo[0] < ChannelListinfo[1])
            {
                BufferPointer[0] = (uint32_t) (bufferPointer);
                BufferPointer[1] = (uint32_t) (bufferPointer + 1);
            }
            else
            {
                BufferPointer[0] = (uint32_t) (bufferPointer + 1);
                BufferPointer[1] = (uint32_t) (bufferPointer);
            }

            for (int8_t i = 0; i < UINT8_C(2); i++)
            {
                retval = findScanChannelsInfo(&TotalNumOfChannelsInScan, RunningChInfo, AdcCentalInfo.ScanChannel[i].Channelmask);
                if (RETCODE_OK != retval)
                {
                    goto end;
                }
                if (UINT8_C(1) == TotalNumOfChannelsInScan)
                {
                    AdcCentalInfo.ScanChannel[i].SamplingCounter++;
                    if (AdcCentalInfo.ScanChannel[i].SamplingCounter == AdcCentalInfo.ScanSamplingFreq / AdcCentalInfo.ScanChannel[i].SamplingFreq)
                    {
                        if (i == 0)
                        {
                            buffptr = BufferPointer[0];
                        }
                        else
                        {
                            buffptr = BufferPointer[1];
                        }
                        /* Populate the buffer data */
                        for (int32_t j = 0; j < ADC_CENTRAL_NO_OF_SAMPLES; j++)
                        {
                            *(AdcCentalInfo.ScanChannel[i].Buffer + j) = *((uint16_t*) buffptr);
                            buffptr += 4;
                        }
                        AdcCentalInfo.ScanChannel[i].Appcallback(BSP_Adc_GetHandle(), AdcCentalInfo.ScanSamplingFreq / AdcCentalInfo.ScanChannel[i].SamplingCounter, AdcCentalInfo.ScanChannel[i].Buffer);
                        AdcCentalInfo.ScanChannel[i].SamplingCounter = 0;
                    }
                }
            }
        }
        end:
        if (pdTRUE != xSemaphoreGive(AdcCentalInfo.ScanChannelInfoLock))
        {
            Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR));
        }
        if (retval != RETCODE_OK)
        {
            Retcode_RaiseError(retval);
        }
    }
    else
    {
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR));
    }
}

Retcode_T AdcCentral_DeInit(void)
{
    Retcode_T retval = RETCODE_OK;
    if (ADC_CENTRAL_INIT == AdcCentalInfo.State)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_ADC_CENTRAL_ALREADY_DEINIT));
    }
    retval = Mcu_Adc_StopScan(BSP_Adc_GetHandle());
    if (RETCODE_OK == retval)
    {
        if (AdcCentalInfo.ScanChannelConfigLock != NULL)
        {
            vSemaphoreDelete(AdcCentalInfo.ScanChannelConfigLock);
            AdcCentalInfo.ScanChannelConfigLock = NULL;
        }
        if (AdcCentalInfo.ScanChannelInfoLock != NULL)
        {
            vSemaphoreDelete(AdcCentalInfo.ScanChannelInfoLock);
            AdcCentalInfo.ScanChannelInfoLock = NULL;
        }
        retval = GuardedTask_Deinitialize(&AdcCentalInfo.AdcScanGuardedTaskHandle);
    }
    if (RETCODE_OK == retval)
    {
        retval = BSP_Adc_Disable();
    }
    if (RETCODE_OK == retval)
    {
        retval = Mcu_Adc_DeInitialize(BSP_Adc_GetHandle());
    }
    if (RETCODE_OK == retval)
    {
        retval = BSP_Adc_Disconnect();
        AdcCentalInfo.SingleChannel.State = ADC_SINGLE_STATE_INIT;
        AdcCentalInfo.State = ADC_CENTRAL_INIT;
    }

    return retval;
}

/* Put function implementations here */
Retcode_T AdcCentral_Init(void)
{
    Retcode_T retval = RETCODE_OK;
    if (ADC_CENTRAL_READY == AdcCentalInfo.State)
    {
        return retval;
    }
    retval = BSP_Adc_Connect();
    if (RETCODE_OK == retval)
    {
        retval = Mcu_Adc_Initialize(BSP_Adc_GetHandle(), AdcIRQCallback);
    }
    if (RETCODE_OK == retval)
    {
        retval = BSP_Adc_Enable();
    }
    if (RETCODE_OK == retval)
    {
        retval = GuardedTask_Initialize(&AdcCentalInfo.AdcScanGuardedTaskHandle, receiveProcess, "ADC_SCAN_TASK", ADCCENTRAL_TASK_PRIORITY, ADCCENTRAL_TASK_STACK_SIZE);
    }
    if (RETCODE_OK == retval)
    {
        AdcCentalInfo.ScanChannelInfoLock = xSemaphoreCreateMutex();
        AdcCentalInfo.ScanChannelConfigLock = xSemaphoreCreateBinary();
    }
    if ((NULL == AdcCentalInfo.ScanChannelInfoLock) || (NULL == AdcCentalInfo.ScanChannelConfigLock))
    {
        retval = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR);
    }
    if (RETCODE_OK == retval)
    {
        AdcCentalInfo.State = ADC_CENTRAL_READY;
        AdcCentalInfo.SingleChannel.State = ADC_SINGLE_STATE_READY;
        AdcCentalInfo.ScanSamplingFreq = 0;
        AdcCentalInfo.ScanChannelMask = 0;
        AdcCentalInfo.ScanChannelInfoIndex = 0;
    }
    return retval;
}

Retcode_T AdcCentral_StartSingle(ADC_T adc, AdcCentral_ConfigSinglePtr_T configStart)
{
    Retcode_T retval = RETCODE_OK;
    if ((NULL == adc) || (NULL == configStart))
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM));
    }
    if (ADC_SINGLE_STATE_READY != AdcCentalInfo.SingleChannel.State)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_ADC_SINGLE_CHANNEL_NOT_READY));
    }
    if (true == IsScanChannelActive(configStart->Channel))
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_ADC_CENTRAL_USED_IN_SCAN));
    }
    Mcu_Adc_ConfigSingle_T SingleConfig;
    SingleConfig.AcqTime = configStart->AcqTime;
    SingleConfig.Channel = configStart->Channel;
    SingleConfig.Reference = configStart->Reference;
    SingleConfig.Resolution = configStart->Resolution;
    SingleConfig.BufferPtr = configStart->BufferPtr;
    AdcCentalInfo.SingleChannel.Appcallback = configStart->Appcallback;
    AdcCentalInfo.SingleChannel.State = ADC_SINGLE_CONVERSION_STATE;
    retval = Mcu_Adc_StartSingle(adc, &SingleConfig);
    if (RETCODE_OK != retval)
    {
        AdcCentalInfo.SingleChannel.State = ADC_SINGLE_STATE_READY;
    }
    return retval;
}

Retcode_T AdcCentral_StartScan(ADC_T adc, AdcCentral_ConfigScanPtr_T configScan)
{
    Retcode_T retval = RETCODE_OK;
    Mcu_Adc_ConfigScan_T ScanConfig;
    uint8_t ChannelCnt = 0;
    uint8_t ChannelNo[3] = { 0 };

    if ((NULL == adc) || (NULL == configScan))
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    if ((MAX_NO_OF_SCAN_CHANNELS <= AdcCentalInfo.ScanChannelInfoIndex))
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_ADC_REACHED_MAX_SCAN_CHANNELS));
    }
    if (pdTRUE == xSemaphoreTake(AdcCentalInfo.ScanChannelInfoLock, portMAX_DELAY))
    {
        if (IsChannelAlreadyActivated(configScan->ChannelScanMask))
        {
            retval = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_ADC_SCAN_CHANNEL_ALREADY_ENABLED);
        }
        if (RETCODE_OK == retval)
        {
            retval = findScanChannelsInfo(&ChannelCnt, ChannelNo, AdcCentalInfo.ScanChannelMask);
        }
        if (RETCODE_OK == retval)
        {
            if (MAX_NO_OF_SCAN_CHANNELS == ChannelCnt)
            {
                retval = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_ADC_REACHED_MAX_SCAN_CHANNELS);
            }
        }
        if (RETCODE_OK == retval)
        {
            if (UINT8_C(0) != AdcCentalInfo.ScanChannelInfoIndex)
            {
                AdcCentalInfo.IsScanChannelConfigChanged = true;
                if (pdTRUE == xSemaphoreTake(AdcCentalInfo.ScanChannelConfigLock, portMAX_DELAY))
                {
                    retval = Mcu_Adc_StopScan(adc);
                }
                else
                {
                    retval = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR);
                }
            }
            if (RETCODE_OK == retval)
            {
                AdcCentalInfo.ScanChannel[AdcCentalInfo.ScanChannelInfoIndex].SamplingFreq = configScan->SamplingRateInHz;

                AdcCentalInfo.ScanChannelMask |= configScan->ChannelScanMask;
                if (AdcCentalInfo.ScanSamplingFreq <= configScan->SamplingRateInHz)
                {
                    AdcCentalInfo.ScanSamplingFreq = configScan->SamplingRateInHz;
                }

                ScanConfig.AcqTime = ADC_ACQ_TIME_128;
                ScanConfig.ChannelScanMask = AdcCentalInfo.ScanChannelMask;
                ScanConfig.SamplingRateInHz = AdcCentalInfo.ScanSamplingFreq;
                ScanConfig.Reference = ADCCENTRAL_REF_VOLTAGE;
                ScanConfig.Resolution = ADC_RESOLUTION_12BIT;
                ScanConfig.BufferPtr = scanDataBufffer;
                ScanConfig.NoOfSamples = ADC_CENTRAL_NO_OF_SAMPLES;

                AdcCentalInfo.ScanChannel[AdcCentalInfo.ScanChannelInfoIndex].Appcallback = configScan->Appcallback;
                AdcCentalInfo.ScanChannel[AdcCentalInfo.ScanChannelInfoIndex].Buffer = configScan->BufferPtr;
                AdcCentalInfo.ScanChannel[AdcCentalInfo.ScanChannelInfoIndex].Channelmask = configScan->ChannelScanMask;
                AdcCentalInfo.ScanChannelInfoIndex++;
                retval = Mcu_Adc_StartScan(adc, &ScanConfig);
            }
        }
        if (pdTRUE != xSemaphoreGive(AdcCentalInfo.ScanChannelInfoLock))
        {
            retval = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR);
        }
    }
    else
    {
        retval = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR);
    }
    return retval;
}

Retcode_T AdcCentral_StopScan(ADC_T adc, uint32_t ChannelScanMask)
{
    Retcode_T retval = RETCODE_OK;
    Mcu_Adc_ConfigScan_T ScanConfig;
    uint8_t channelIndex;

    if (NULL == adc)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_FAILURE));
    }
    if (!(IsChannelAlreadyActivated(ChannelScanMask)))
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_ADC_SCAN_CHANNEL_NOT_ENABLED));
    }
    retval = Mcu_Adc_StopScan(adc);
    if (RETCODE_OK == retval)
    {
        channelIndex = findindexOfScanChannel(ChannelScanMask);
        for (int8_t indx = channelIndex; indx < MAX_NO_OF_SCAN_CHANNELS; indx++)
        {
            if (indx != MAX_NO_OF_SCAN_CHANNELS - 1)
            {
                AdcCentalInfo.ScanChannel[indx].Appcallback = AdcCentalInfo.ScanChannel[indx + 1].Appcallback;
                AdcCentalInfo.ScanChannel[indx].Buffer = AdcCentalInfo.ScanChannel[indx + 1].Buffer;
                AdcCentalInfo.ScanChannel[indx].Channelmask = AdcCentalInfo.ScanChannel[indx + 1].Channelmask;
                memset((void*) &AdcCentalInfo.ScanChannel[indx + 1], 0, sizeof(AdcCentral_ScanChannelInfo_T));
            }
            else
            {
                memset((void*) &AdcCentalInfo.ScanChannel[indx], 0, sizeof(AdcCentral_ScanChannelInfo_T));
            }
        }
        AdcCentalInfo.ScanChannelInfoIndex--;

        if (AdcCentalInfo.ScanChannelInfoIndex)
        {
            AdcCentalInfo.ScanChannelMask ^= ChannelScanMask;
            ScanConfig.AcqTime = ADC_ACQ_TIME_16;
            ScanConfig.ChannelScanMask = AdcCentalInfo.ScanChannelMask;
            ScanConfig.SamplingRateInHz = AdcCentalInfo.ScanSamplingFreq;
            ScanConfig.Reference = ADCCENTRAL_REF_VOLTAGE;
            ScanConfig.Resolution = ADC_RESOLUTION_12BIT;
            ScanConfig.BufferPtr = scanDataBufffer;
            ScanConfig.NoOfSamples = ADC_CENTRAL_NO_OF_SAMPLES;
            retval = Mcu_Adc_StartScan(adc, &ScanConfig);
        }
        else
        {
            AdcCentalInfo.ScanChannelMask = 0;
            AdcCentalInfo.ScanSamplingFreq = 0;
        }
    }
    return retval;
}

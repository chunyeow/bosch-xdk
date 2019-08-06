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
#define BCDS_MODULE_ID XDK_COMMON_ID_LORA

#if XDK_CONNECTIVITY_LORA
/* own header files */
#include "XDK_LoRa.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "BCDS_LoRaDevice.h"

/* Put the type and macro definitions here */
#define JOIN_TIMEOUT 10000
/* Put constant and variable definitions here */
/**< LORA setup information */
static LoRa_Setup_T LoraSetupInfo;
/**< Handle for LORA join operation  */
static SemaphoreHandle_t LoraJoinHandle;
/**< LoRa join status */
static bool LoRaJoinStatus = false;

/* Put private function declarations here */
/**
 * @brief   LoRa Network Callback function used by the stack to communicate events to the application
 *
 * @param[in] flags Currently not used
 *
 * @param[in] eventInfo Hold the information about the event
 *
 */
static void LoRaCallbackFunc(void *flags, LoRaMacEventInfo_T *eventInfo)
{
    BCDS_UNUSED(flags);
    Retcode_T retcode = RETCODE_OK;
    switch (eventInfo->event)
    {
        case LORA_NETWORK_JOIN_SUCCESS:
        LoRaJoinStatus = true;
        if (pdTRUE != xSemaphoreGive(LoraJoinHandle))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR);
        }
        break;
        case LORA_NETWORK_JOIN_TIMEOUT:
        case LORA_NETWORK_JOIN_FAILURE:
        LoRaJoinStatus = false;
        if (pdTRUE != xSemaphoreGive(LoraJoinHandle))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR);
        }
        break;
        case LORA_NETWORK_RECEIVED_PACKET:
        LoraSetupInfo.EventCallback(LORA_EVENT_RECEIVED_PACKET);
        break;
        case LORA_NETWORK_SEND_FAILED:
        LoraSetupInfo.EventCallback(LORA_EVENT_SEND_FAILED);
        break;
        case LORA_NETWORK_RECEIVE_FAILED:
        LoraSetupInfo.EventCallback(LORA_EVENT_RECEIVE_FAILED);
        break;
        default:
        break;
    }
    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseError(retcode);
    }
}
/* Put function implementations here */

/** Refer interface header for description */
Retcode_T LoRa_Setup(LoRa_Setup_T * setup)
{
    Retcode_T retcode = RETCODE_OK;
    if (NULL == setup)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    LoraJoinHandle = xSemaphoreCreateBinary();
    if (NULL == LoraJoinHandle)
    {
        vSemaphoreDelete(LoraJoinHandle);
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
    }
    else
    {
        LoRaJoinStatus = false;
        LoraSetupInfo = *setup;
    }
    return (retcode);
}

/** Refer interface header for description */
Retcode_T LoRa_Enable(void)
{
    Retcode_T retcode = RETCODE_OK;
    uint64_t hwDevEUI;
    retcode = LoRaDevice_Init(LoRaCallbackFunc, LoraSetupInfo.Freq);
    if (RETCODE_OK == retcode)
    {
        retcode = LoRaDevice_SetRxWindow2(0, LoraSetupInfo.RxFreq);
    }
    if (RETCODE_OK == retcode)
    {
        if (LoraSetupInfo.DevEUI == NULL)
        {
            retcode = LoRa_GetHwEUI(&hwDevEUI);
            if (retcode == RETCODE_OK)
            {
                LoraSetupInfo.DevEUI = &hwDevEUI;
                retcode = LoRaDevice_SetDevEUI(hwDevEUI);
            }
        }
        else
        {
            retcode = LoRaDevice_SetDevEUI(*(LoraSetupInfo.DevEUI));
        }
    }
    if (RETCODE_OK == retcode)
    {
        retcode = LoRaDevice_SetAppEUI(LoraSetupInfo.AppEUI);
    }

    if (RETCODE_OK == retcode)
    {
        retcode = LoRaDevice_SetAppKey(LoraSetupInfo.AppKey);
    }

    if (RETCODE_OK == retcode)
    {
        retcode = LoRaDevice_SetRadioCodingRate(LoraSetupInfo.CodingRate);
    }

    if (RETCODE_OK == retcode)
    {
        retcode = LoRaDevice_SaveConfig();
    }

    return (retcode);
}

/** Refer interface header for description */
Retcode_T LoRa_Join(void)
{
    Retcode_T retcode = RETCODE_OK;

    // This is a dummy semaphore take
    (void)xSemaphoreTake(LoraJoinHandle, 0);
    retcode = LoRaDevice_Join((LoRaDeviceJoin_T) LoraSetupInfo.JoinType);
    if (RETCODE_OK == retcode)
    {
        if ((pdTRUE != xSemaphoreTake(LoraJoinHandle, pdMS_TO_TICKS(JOIN_TIMEOUT)) || (false == LoRaJoinStatus)))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_LORA_JOIN_FAILED);
        }
    }
    return (retcode);
}

/** Refer interface header for description */
Retcode_T LoRa_SetDataRate(uint8_t dataRate)
{
    Retcode_T retcode = RETCODE_OK;
    retcode = LoRaDevice_SetDataRate(dataRate);
    return (retcode);
}

/** Refer interface header for description */
Retcode_T LoRa_SendUnconfirmed(uint8_t LoRaPort, uint8_t *dataBuffer, uint32_t dataBufferSize)
{
    Retcode_T retcode = RETCODE_OK;
    if (NULL == dataBuffer)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    retcode = LoRaDevice_SendUnconfirmed(LoRaPort, dataBuffer, (uint16_t) dataBufferSize);
    return (retcode);
}

/** Refer interface header for description */
Retcode_T LoRa_SendConfirmed(uint8_t LoRaPort, uint8_t *dataBuffer, uint32_t dataBufferSize)
{
    Retcode_T retcode = RETCODE_OK;
    if (NULL == dataBuffer)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    retcode = LoRaDevice_SendConfirmed(LoRaPort, dataBuffer, (uint16_t) dataBufferSize);
    return (retcode);
}

/** Refer interface header for description */
Retcode_T LoRa_Disable(void)
{
    Retcode_T retcode = RETCODE_OK;
    retcode = LoRaDevice_Deinit();
    return (retcode);
}

/** Refer interface header for description */
Retcode_T LoRa_Close(void)
{
    Retcode_T retcode = RETCODE_OK;
    vSemaphoreDelete(LoraJoinHandle);
    LoraJoinHandle = NULL;
    return (retcode);
}

/** Refer interface header for description */
Retcode_T LoRa_GetHwEUI(uint64_t *hwDevEUI)
{
    Retcode_T retcode = RETCODE_OK;
    retcode = LoRaDevice_GetHwEUI(hwDevEUI);
    return (retcode);
}

/** Refer interface header for description */
Retcode_T LoRa_SetADR(bool enable)
{
    return LoRaDevice_SetADR(enable);
}

/** Refer interface header for description */
Retcode_T LoRa_GetADR(bool* adr)
{
    Retcode_T retcode = RETCODE_OK;
    if (NULL == adr)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    retcode = LoRaDevice_GetADR(adr);
    return retcode;
}
#endif /* XDK_CONNECTIVITY_LORA */

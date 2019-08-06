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
 * @brief This Module contains necessary implementation in order to enable user to develop his application using the
 *        Extension port SPI communication interface. This module can be used a reference design for interfacing user device
 *        via SPI of extension port
 *
 */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_EXTENSIONPORTSPI

#include "ExtensionPortSpi.h"
#include "BCDS_Assert.h"
#include "BCDS_MCU_SPI.h"
#include "BSP_ExtensionPort.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"

/* Put the type and macro definitions here */
/** SPI handle for hardware SPI instance created for External SPI hardware */
static HWHandle_T SpiHandle;

/* semaphore unblocking completion of frame transmission*/
static SemaphoreHandle_t SpiCompleteSync = NULL;

/* Put constant and variable definitions here */

/* Put private function declarations here */

/** @brief Callback function to be triggered upon Tx & Rx interrupts through the SPI
 *
 */
static void AppCallback(SPI_T spi, struct MCU_SPI_Event_S event)
{
    if ((SpiCompleteSync != NULL) && (NULL != spi))
    {
        if ((event.TxComplete) || (event.RxComplete))
        {
            portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
            if (xSemaphoreGiveFromISR(SpiCompleteSync, &xHigherPriorityTaskWoken) == pdTRUE)
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }

    }
    else
    {
        Retcode_RaiseErrorFromIsr (RETCODE( RETCODE_SEVERITY_FATAL, RETCODE_NULL_POINTER));
    }

    if (event.RxError)
    {
        Retcode_RaiseErrorFromIsr(RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t) RETCODE_EXTENSIONPORTSPI_RX_ERROR));
    }

    if (event.TxError)
    {
        Retcode_RaiseErrorFromIsr(RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t) RETCODE_EXTENSIONPORTSPI_TX_ERROR));
    }

    if (event.DataLoss)
    {
        Retcode_RaiseErrorFromIsr(RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t) RETCODE_EXTENSIONPORTSPI_DATALOSS));
    }
}

/* Put function implementations here */

/** Refer interface header for description */
Retcode_T ExtensionPortSpi_Initialize(ExtensionPortSpi_Config_T * config)
{
    Retcode_T ret = RETCODE_OK;
    ret = BSP_ExtensionPort_ConnectSpi();
    if (RETCODE_OK == ret)
    {
        ret = BSP_ExtensionPort_SetSpiConfig(BSP_EXTENSIONPORT_SPI_BAUDRATE, config->BaudRate, NULL);
    }
    if (RETCODE_OK == ret)
    {
        ret = BSP_ExtensionPort_SetSpiConfig(BSP_EXTENSIONPORT_SPI_MODE, (uint32_t) config->ClockMode, NULL);
    }
    if (RETCODE_OK == ret)
    {
        ret = BSP_ExtensionPort_SetSpiConfig(BSP_EXTENSIONPORT_SPI_BIT_ORDER, (uint32_t) config->BitOrder, NULL);
    }
    if (RETCODE_OK == ret)
    {
        if (NULL == SpiCompleteSync)
        {
            SpiCompleteSync = xSemaphoreCreateBinary();
            if (NULL == SpiCompleteSync)
            {
                ret = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
        }
        if (NULL != SpiCompleteSync)
        {
            SpiHandle = BSP_ExtensionPort_GetSpiHandle();
            if (NULL != SpiHandle)
            {
                ret = MCU_SPI_Initialize(SpiHandle, AppCallback);
            }
            else
            {
                ret = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
            }
        }
    }
    if (RETCODE_OK == ret)
    {
        ret = BSP_ExtensionPort_EnableSpi();
    }
    return ret;

}

/** Refer interface header for description */
Retcode_T ExtensionPortSpi_Read(uint8_t *readVal, uint32_t readLength, uint32_t timeout)
{
    Retcode_T retVal = RETCODE_OK;

    if ((readLength == 0) || (NULL == readVal))
    {
        retVal = RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_INVALID_PARAM);
    }
    if (RETCODE_OK == retVal)
    {
        if (SpiCompleteSync == NULL)
        {
            retVal = RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_UNINITIALIZED);
        }
    }
    if (RETCODE_OK == retVal)
    {
        retVal = MCU_SPI_Receive(SpiHandle, readVal, readLength);

        if (RETCODE_OK == retVal)
        {
            if (pdTRUE != xSemaphoreTake(SpiCompleteSync, pdMS_TO_TICKS(timeout)))
            {
                retVal = RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_TIMEOUT);
            }
        }
    }
    return (retVal);
}

/** Refer interface header for description */
Retcode_T ExtensionPortSpi_Write(uint8_t *writeVal, uint32_t writeLength, uint32_t timeout)
{
    Retcode_T retVal = RETCODE_OK;

    if (writeLength == 0)
    {
        retVal = RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_INVALID_PARAM);
    }
    if (RETCODE_OK == retVal)
    {
        if (SpiCompleteSync == NULL)
        {
            retVal = RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_UNINITIALIZED);
        }
    }
    if (RETCODE_OK == retVal)
    {
        retVal = MCU_SPI_Send(SpiHandle, writeVal, writeLength);
        if (RETCODE_OK == retVal)
        {
            if (pdTRUE != xSemaphoreTake(SpiCompleteSync, pdMS_TO_TICKS(timeout)))
            {
                retVal = RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_TIMEOUT);
            }
        }
    }
    return (retVal);
}

/** Refer interface header for description */
Retcode_T ExtensionPortSpi_DeInitialize(void)
{
    Retcode_T retcode = RETCODE_OK;

    if (((HWHandle_T) NULL == SpiHandle))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_UNINITIALIZED);
    }

    if (RETCODE_OK == retcode)
    {
        retcode = MCU_SPI_Deinitialize(SpiHandle);
    }

    if (RETCODE_OK == retcode)
    {
        retcode = BSP_ExtensionPort_DisableSpi();
    }
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_ExtensionPort_DisonnectSpi();
    }

    if (RETCODE_OK == retcode)
    {
        retcode = BSP_ExtensionPort_Disconnect();
    }

    if (RETCODE_OK == retcode)
    {
        if (NULL != SpiCompleteSync)
        {
            vSemaphoreDelete(SpiCompleteSync);
            SpiCompleteSync = NULL;
        }
        SpiHandle = NULL;
    }

    return retcode;
}

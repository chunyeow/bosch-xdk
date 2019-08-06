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
 * @brief This module has the implementation of Reset from terminal program.
 *
 * ****************************************************************************/

//lint -esym(956,*) /* Suppressing "Non const, non volatile static or external variable" lint warning*/
/* module includes ********************************************************** */

/* system header files */
#include <stdint.h>
#include <string.h>

#if BCDS_EMLIB_INCLUDE_USB

#include "em_device.h"

#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_COMMON_ID_USBRESETUTILITY

/* own header files */
#include "XdkUsbResetUtility.h"

/* additional interface header files */
#include "BCDS_USB.h"
#include "BCDS_NVMConfig.h"
#include "BCDS_RingBuffer.h"
#include "FreeRTOS.h"
#include "timers.h"

/* constant definitions ***************************************************** */
#define URU_BOOT_ENABLE             (UINT8_C(1))                    /**< Enable the Boot flag  */
#define URU_APPL_RESET_ENABLE       (UINT8_C(2))                    /**< Enable Application Rest flag */
#define URU_FLAG_CLEAR              (UINT32_C(0))                   /**< Disable the Reset flag */
#define URU_BOOT_MAGIC_WORD         'b'                             /**< word to write in user page to reboot*/
#define URU_BUFFER_SIZE             (UINT32_C(256))                 /**< size of the buffer in which data is stored*/
#define BOOT_ENGAGEVALUE            0xAA55AA55UL
/* local variables ********************************************************** */

/**Array to hold the data coming from USB*/
static uint8_t RcvBuffer[URU_BUFFER_SIZE];
static RingBuffer_T DeferredRxBuffer;
static uint8_t UsbReceiveBuffer[URU_BUFFER_SIZE];

/** Instance for receive Application callback of USB */
static UsbResetUtility_UsbAppCallback_T UsbAppCallback = NULL;

const uint8_t Boot_Reset_Cmd[] = "#reBoot$"; /* Reset Magic word */
const uint8_t Appl_Reset_Cmd[] = "#reSet$"; /* command for application Reset */
extern unsigned int __SwitchToBoot;
/* global variables ********************************************************* */

/* inline functions ********************************************************* */

/* local functions ********************************************************** */

/**
 * @brief This API is using in USB configuration to trigger USB Receive Interrupt callback
 *        from application layer.
 * @param[in]	usbRcvBuffer
 *         Receive buffer where the data is received in the USB Interrupt callback.
 * @param[in]	usbRcvBufLength
 *       The size of the data received.
 *
 */
void UsbResetUtilityISRCallback(uint8_t *usbRcvBuffer, uint16_t usbRcvBufLength);

/**
 * @brief The function resets the board
 */
static void UsbResetUtilitySystemReset(void)
{
    /*Disconnects the USB*/
    USBD_Disconnect();

    /*This function will reset the board*/
    NVIC_SystemReset();
}

/**
 * @brief The function is been called from the timer queue when the string received from USB does not match with the magic string.
 * It is called so that the USB handling can be pended from the ISR context.
 *
 * @param [in] param1
 * Unused
 *
 * @param [in] param2
 * Unused
 *
 */
static void UsbResetUtility_InteruptHandling(void *param1, uint32_t param2)
{
    BCDS_UNUSED(param1);
    BCDS_UNUSED(param2);

    Retcode_T retcode = RETCODE_OK;
    uint32_t actualBytesRead = 0U;

    memset(UsbReceiveBuffer, 0x00, sizeof(UsbReceiveBuffer));
    actualBytesRead = RingBuffer_Read(&DeferredRxBuffer, UsbReceiveBuffer, sizeof(UsbReceiveBuffer));

    /* Validating the system  BootEngage command */
    if (actualBytesRead >= strlen((const char*)Boot_Reset_Cmd) &&
            0U == strncmp((const char*) UsbReceiveBuffer, (const char*) Boot_Reset_Cmd, sizeof(Boot_Reset_Cmd)))
    {
#if (XDK_FOTA_ENABLED_BOOTLOADER == 0) || (XDK_FOTA_ENABLED_BOOTLOADER == 1)
        uint8_t bootEngage = URU_BOOT_MAGIC_WORD;
        /* engage Bootloader flag in userpage */
        retcode = NVM_WriteUInt8(&NVMUser, NVM_ITEM_BOOTLOADER_ENGAGE, &bootEngage);
        if (RETCODE_OK == retcode)
        {
            retcode = NVM_Flush(&NVMUser);
        }
#else
        /* engage Bootloader flag in userpage */
        __SwitchToBoot = BOOT_ENGAGEVALUE;
#endif

        if (RETCODE_OK == retcode)
        {
            /*doing the system reset */
            UsbResetUtilitySystemReset();
        }
    }
    /* Validating the system  Application Reset command */
    else if (actualBytesRead >= strlen((const char*)Appl_Reset_Cmd) &&
            0U == strncmp((const char*) UsbReceiveBuffer, (const char*) Appl_Reset_Cmd, sizeof(Appl_Reset_Cmd)))
    {
        /* doing the system reset */
        UsbResetUtilitySystemReset();
    }
    else if ((UsbAppCallback != NULL) && (0U != actualBytesRead))
    {
        UsbAppCallback(UsbReceiveBuffer, actualBytesRead);
    }
    else
    {
        /* Do nothing */
    }
    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseError(retcode);
    }
}

/* global functions ********************************************************* */

/** Refer interface header for description */
Retcode_T UsbResetUtility_Init(void)
{
    Retcode_T retcode = RETCODE_OK;
    __SwitchToBoot = 0x00;
    /* Registering the URU callback function */
    if (USB_SUCCESS != USB_callBackMapping(UsbResetUtilityISRCallback))
    {
        retcode = RETCODE(RETCODE_SEVERITY_NONE, RETCODE_FAILURE);
    }
    else
    {
        RingBuffer_Initialize(&DeferredRxBuffer, RcvBuffer, sizeof(RcvBuffer));
    }

    return retcode;
}

/** Refer interface header for description */
void UsbResetUtilityISRCallback(uint8_t *usbRcvBuffer, uint16_t usbRcvBufLength)
{
    uint16_t rcvLength = 0U;
    uint32_t bytesWritten = 0UL;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if ((NULL != usbRcvBuffer) && (0UL != usbRcvBufLength))
    {
        /* Only copy so many bytes as we actually have room in our RcvBuffer */
        rcvLength = usbRcvBufLength < sizeof(RcvBuffer) ? usbRcvBufLength : sizeof(RcvBuffer);

        /* Copies the received string to a static buffer*/
        bytesWritten = RingBuffer_Write(&DeferredRxBuffer, usbRcvBuffer, rcvLength);

        if (bytesWritten != rcvLength)
        {
            Retcode_RaiseErrorFromIsr(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES));
        }

        if (pdPASS == xTimerPendFunctionCallFromISR(UsbResetUtility_InteruptHandling, NULL, 0UL, &xHigherPriorityTaskWoken))
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        else
        {
            Retcode_RaiseErrorFromIsr(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_TIMER_PEND_FAILED));
        }
    }
    else
    {
        Retcode_RaiseErrorFromIsr(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_USB_RECIEVE_FAILED));
    }
}

/** Refer interface header for description */
Retcode_T UsbResetUtility_RegAppISR(UsbResetUtility_UsbAppCallback_T usbAppCallback)
{
    Retcode_T retcode = RETCODE_OK;

    if (NULL != usbAppCallback)
    {
        UsbAppCallback = usbAppCallback;
    }
    else
    {
        retcode = RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_NULL_POINTER);
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T UsbResetUtility_DeInit(void)
{
    /* @todo : Currently we can't un-register from BSP, meaning we will continue to receive callbacks from BSP_USB. */
    UsbAppCallback = NULL;

    return RETCODE_OK;
}

#endif /* BCDS_EMLIB_INCLUDE_USB */

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

/**  @brief
 *
 * This module contains "Main" routine  and the APIs , in which user can easily integrate application
 * specific routines
 *
 * ****************************************************************************/

/* module includes ********************************************************** */
/* Public Header ************************************************************ */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_COMMON_ID_SYSTEMSTARTUP

/* system header files */
#include "BCDS_Basics.h"
#include <stdio.h>

/* own header files */
#include "XdkSystemStartup.h"

/* additional interface header files */
#include "XdkUsbResetUtility.h"
#include "BCDS_TaskConfig.h"
#include "BCDS_CmdProcessor.h"
#include "BCDS_BSP_Board.h"
#include "BCDS_BSP_LED.h"
#include "BSP_BoardType.h"
#include "BCDS_Assert.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "BCDS_NVMConfig.h"
#include "BCDS_MCU_Watchdog.h"
#include "BCDS_MCU_Sleep.h"

/* constant definitions ***************************************************** */

#define BLE_MAC_USERPAGE_FLAG_BIT    UINT8_C(0x02)
#define WIFI_MAC_USERPAGE_FLAG_BIT   UINT8_C(0x01)

/* Function Prototypes ***************************************************** */

/**
 * @brief This function is SysTick Handler.
 * This is called when ever the IRQ is hit.
 * This is a temporary implementation where
 * the SysTick_Handler() is not directly mapped
 * to xPortSysTickHandler(). Instead it is only
 * called if the scheduler has started.
 */
static void SysTickPreCallback(void);

/* local variables ********************************************************** */

static BSP_Systick_Callback_T SystickPreCallback = SysTickPreCallback;

extern void xPortSysTickHandler(void);

/**
 * @brief Data buffer of the user data NVM section.
 * @details This buffer is used by the NVM module to access and write the non
 * volatile memory (NVM) section in the user data page of the internal flash.
 */
static uint8_t NVMUserData[NVM_SECTION_UserPage_BUFFER_SIZE];

/* inline functions ********************************************************* */

/* local functions ********************************************************** */

/**
 * @brief Global instance that is used by NVM to maintain internal (secured)
 * data
 */
struct NVM_S NVMUser =
        {
        NVM_SECTION_UserPage,
                NVMUserData,
                NVM_SECTION_UserPage_BUFFER_SIZE
        };

/** Refer function prototype for description */
static void SysTickPreCallback(void)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
        SystickPreCallback = xPortSysTickHandler;
    }
}

/**
 * @brief        System Error Handling routine
 *
 * @param [in]   Error: Error Information from Basic
 *
 * @param [in]   isfromIsr :currently unused, can be used for future implementation.
 */
void DefaultErrorHandlingFunc(Retcode_T error, bool isfromIsr)
{
    if (false == isfromIsr)
    {
        /* @TODO ERROR HANDLING / DIAGNOSIS SHOULD BE DONE FOR THE ERRORS RAISED FROM PLATFORM */
        uint32_t packageID = Retcode_GetPackage(error);
        uint32_t code = Retcode_GetCode(error);
        uint32_t moduleId = Retcode_GetModuleId(error);
        Retcode_Severity_T severity = Retcode_GetSeverity(error);
        char * severityString = "Error";
        char * packageString = "Unclassified";

        switch (severity)
        {
        case RETCODE_SEVERITY_FATAL:
            severityString = "Fatal Error";
            break;
        case RETCODE_SEVERITY_ERROR:
            severityString = "Error";
            break;
        case RETCODE_SEVERITY_WARNING:
            severityString = "Warning";
            break;
        case RETCODE_SEVERITY_INFO:
            severityString = "Info";
            break;
        default:
            break;
        }

        switch (packageID)
        {
        case 2:
            packageString = "Utils";
            break;
        case 6:
            packageString = "Algo";
            break;
        case 7:
            packageString = "SensorUtils";
            break;
        case 8:
            packageString = "Sensors";
            break;
        case 9:
            packageString = "SensorToolbox";
            break;
        case 10:
            packageString = "WLAN";
            break;
        case 13:
            packageString = "BLE";
            break;
        case 26:
            packageString = "FOTA";
            break;
        case 30:
            packageString = "ServalPAL";
            break;
        case 33:
            packageString = "LoRaDrivers";
            break;
        case 35:
            packageString = "Essentials";
            break;
        case 36:
            packageString = "Drivers";
            break;
        case 101:
            packageString = "BSTLib";
            break;
        case 103:
            packageString = "FreeRTOS";
            break;
        case 105:
            packageString = "BLEStack";
            break;
        case 106:
            packageString = "EMlib";
            break;
        case 107:
            packageString = "ServalStack";
            break;
        case 108:
            packageString = "WiFi";
            break;
        case 112:
            packageString = "FATfs";
            break;
        case 115:
            packageString = "Escrypt_CycurTLS";
            break;
        case 122:
            packageString = "GridEye";
            break;
        case 123:
            packageString = "BSX";
            break;
        case 153:
            packageString = "XDK110 Application";
            break;
        case 175:
            packageString = "BSP";
            break;
        default:
            break;
        }

#if SERVAL_ENABLE_DTLS && SERVAL_TLS_MBEDTLS
        if ((30 == packageID) && (4 == moduleId) && (2 == severity) && (12 == code))
        {
            ;/* do nothing for now */ /* @todo - remove this session of code once dtls buffer handling is fixed */
          return;
        }
#endif /* #if SERVAL_ENABLE_DTLS && SERVAL_TLS_MBEDTLS */
        printf("%s in %s package.\r\n"
                "\tPackage ID: %u\r\n"
                "\tModule ID: %u\r\n"
                "\tSeverity code: %u\r\n"
                "\tError code: %u\r\n",
                severityString, packageString,
                (unsigned int) packageID,
                (unsigned int) moduleId,
                (unsigned int) severity,
                (unsigned int) code);

    }
    else
    {
        /* @todo - Find a way to log error safely in ISR context */;
    }

}

#ifndef NDEBUG /* valid only for debug builds */
/**
 * @brief This API is called when function enters an assert
 *
 * @param[in] line : line number where asserted
 * @param[in] file : file name which is asserted
 *
 */

void assertIndicationMapping(const unsigned long line, const unsigned char * const file)
{
    /* Switch on the LEDs */
    Retcode_T retcode = RETCODE_OK;

    retcode = BSP_LED_SwitchAll((uint32_t) BSP_LED_COMMAND_ON);

    printf("asserted at Filename %s , line no  %ld \n\r", file, line);

    if (retcode != RETCODE_OK)
    {
        printf("LED's ON failed during assert");
    }
    /* @todo - mta5cob : Move this to Utils shared package */
    vTaskSuspendAll();
}
#endif

Retcode_T systemStartup(void)
{
    Retcode_T retcode = RETCODE_OK;
    uint32_t param1 = 0;
    void* param2 = NULL;
    uint8_t contentFlag = UINT8_C(0);

    /* Initialize the callbacks for the system tick */
    BSP_Board_OSTickInitialize(SystickPreCallback, NULL);

#ifndef NDEBUG /* valid only for debug builds */
    if (RETCODE_OK == retcode)
    {
        retcode = Assert_Initialize(assertIndicationMapping);
    }
#endif
    /* First we initialize the board. */
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_Board_Initialize(param1, param2);
    }
#if BCDS_EMLIB_INCLUDE_USB
    if (RETCODE_OK == retcode)
    {
        retcode = UsbResetUtility_Init();
    }
#endif
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_LED_Connect();
    }
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_LED_EnableAll();
    }
    if (RETCODE_OK == retcode)
    {
        retcode = NVM_Initialize(&NVMUser);
    }
    if (RETCODE_OK == retcode)
    {
        /* Read the BLE/Wifi bit */
        retcode = NVM_ReadUInt8(&NVMUser, NVM_ITEM_CONTENT_INDEX, &contentFlag);
        if (RETCODE_OK == retcode)
        {
            /* check if BLE/Wifi bit is set */
            if ((BLE_MAC_USERPAGE_FLAG_BIT | WIFI_MAC_USERPAGE_FLAG_BIT) != contentFlag)
            {
                contentFlag = (BLE_MAC_USERPAGE_FLAG_BIT | WIFI_MAC_USERPAGE_FLAG_BIT);
                retcode = NVM_WriteUInt8(&NVMUser, NVM_ITEM_CONTENT_INDEX, &contentFlag);
                if (RETCODE_OK == retcode)
                {
                    retcode = NVM_Flush(&NVMUser);
                }
                if (RETCODE_OK == retcode)
                {
                    retcode = NVM_ReadUInt8(&NVMUser, NVM_ITEM_CONTENT_INDEX, &contentFlag);
                    if (RETCODE_OK == retcode)
                    {
                        if ((BLE_MAC_USERPAGE_FLAG_BIT | WIFI_MAC_USERPAGE_FLAG_BIT) != contentFlag)
                        {
                            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NVM_WIFI_AND_BLE_BIT_SET_FAILED);
                        }
                    }
                }
            }
        }
    }

#ifndef NXDK_COMMON_WATCHDOG
    /* initialize watchdog timer */
    if (RETCODE_OK == retcode)
    {
        MCU_Watchdog_Init_T wdgInitStruct =
                {
                        .WdgCallback = NULL,
                        .ResetMode = MCU_WATCHDOG_RESET_ON,
                        .RunOnCpuHalt = false,
                        .Timeout = 256000 //4.26 minutes (max timeout supported by emlib)
                };

        retcode = MCU_Watchdog_Init((WdgHandle_T) & wdgInitStruct);
    }
    /* enable watchdog timer */
    if (RETCODE_OK == retcode)
    {
        retcode = MCU_Watchdog_Enable();
    }
#endif /* #ifndef NXDK_COMMON_WATCHDOG */

    return retcode;
}

/**
 * @brief       This function is called when is stack overflows and the program gets into infinite loop.
 *
 * @param[in]   Task handle
 * @param[in]   Task name
 *
 */
void vApplicationStackOverflowHook(xTaskHandle *pxTask,
        signed char *pcTaskName)
{
    Retcode_T retcode = RETCODE_OK;

    printf("----- STACK OVERFLOW -----Task Name:%s-----Current Task Handle:0x%x\r\n", pcTaskName, (int) pxTask);

    retcode = BSP_LED_Switch((uint32_t) BSP_XDK_LED_Y, (uint32_t) BSP_LED_COMMAND_ON);

    if (RETCODE_OK == retcode)
    {
        retcode = BSP_LED_Switch((uint32_t) BSP_XDK_LED_O, (uint32_t) BSP_LED_COMMAND_ON);
    }
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_LED_Switch((uint32_t) BSP_XDK_LED_R, (uint32_t) BSP_LED_COMMAND_OFF);
    }
    if (RETCODE_OK != retcode)
    {
        printf("StackOverflowHook LED indication failed with error Code - %x", (unsigned int) retcode);
    }

    /* This function will be called if a task overflows its stack, if
     configCHECK_FOR_STACK_OVERFLOW != 0.  It might be that the function
     parameters have been corrupted, depending on the severity of the stack
     overflow.  When this is the case pxCurrentTCB can be inspected in the
     debugger to find the offending task. */
    for (;;)
    {
        ; /* endless loop */
    }
}

void vApplicationMallocFailedHook(void)
{
    printf("----- HEAP ISSUE ----\r\n");

    for (;;)
    {
        ; /* endless loop */
    }
}

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
 implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
     function then they must be declared static - otherwise they will be allocated on
     the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
     state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     Note that, as the array is necessarily of type StackType_t,
     configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 application must provide an implementation of vApplicationGetTimerTaskMemory()
 to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
     function then they must be declared static - otherwise they will be allocated on
     the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     Note that, as the array is necessarily of type StackType_t,
     configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vApplicationIdleHook(void)
{
    /* feed the Watch dog */
    (void) MCU_Watchdog_Feed();
    MCU_Sleep_EnterSleep(1);
}
/** ************************************************************************* */

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
 * This module handles the Button features.
 *
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_BUTTON

/* own header files */
#include "XDK_Button.h"

#if XDK_SENSOR_BUTTON

/* additional interface header files */
#include "BCDS_BSP_Button.h"
#include "BSP_BoardType.h"
#include "FreeRTOS.h"
#include "task.h"

/* constant definitions ***************************************************** */

enum Button_EventTypeClassification_E
{
    INTERNAL_BUTTON_1_PRESSED,
    INTERNAL_BUTTON_1_RELEASED,
    INTERNAL_BUTTON_2_PRESSED,
    INTERNAL_BUTTON_2_RELEASED,
};

typedef enum Button_EventTypeClassification_E Button_EventTypeClassification_T;

/* local variables ********************************************************** */

/**< Button Setup */
static Button_Setup_T ButtonSetup;

/* global variables ********************************************************* */

static void ButtonEventDequeue(void *param1, uint32_t param2)
{
    BCDS_UNUSED(param1);

    if (INTERNAL_BUTTON_1_PRESSED == (Button_EventTypeClassification_T) param2)
    {
        if (NULL != ButtonSetup.InternalButton1Callback)
        {
            ButtonSetup.InternalButton1Callback(BUTTON_EVENT_PRESSED);
        }
    }
    else if (INTERNAL_BUTTON_1_RELEASED == (Button_EventTypeClassification_T) param2)
    {
        if (NULL != ButtonSetup.InternalButton1Callback)
        {
            ButtonSetup.InternalButton1Callback(BUTTON_EVENT_RELEASED);
        }
    }
    else if (INTERNAL_BUTTON_2_PRESSED == (Button_EventTypeClassification_T) param2)
    {
        if (NULL != ButtonSetup.InternalButton2Callback)
        {
            ButtonSetup.InternalButton2Callback(BUTTON_EVENT_PRESSED);
        }
    }
    else if (INTERNAL_BUTTON_2_RELEASED == (Button_EventTypeClassification_T) param2)
    {
        if (NULL != ButtonSetup.InternalButton2Callback)
        {
            ButtonSetup.InternalButton2Callback(BUTTON_EVENT_RELEASED);
        }
    }
    else
    {
        Retcode_RaiseErrorFromIsr(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM));
    }
}

static void Button1Callback(uint32_t data)
{
    Retcode_T retcode = RETCODE_OK;
    if (BSP_XDK_BUTTON_PRESS == (BSP_ButtonPress_T) data)
    {
        retcode = CmdProcessor_EnqueueFromIsr(ButtonSetup.CmdProcessorHandle, ButtonEventDequeue, NULL, (uint32_t) INTERNAL_BUTTON_1_PRESSED);
    }
    else if (BSP_XDK_BUTTON_RELEASE == (BSP_ButtonPress_T) data)
    {
        retcode = CmdProcessor_EnqueueFromIsr(ButtonSetup.CmdProcessorHandle, ButtonEventDequeue, NULL, (uint32_t) INTERNAL_BUTTON_1_RELEASED);
    }
    else
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
    }
    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseErrorFromIsr(retcode);
    }
}

static void Button2Callback(uint32_t data)
{
    Retcode_T retcode = RETCODE_OK;
    if (BSP_XDK_BUTTON_PRESS == (BSP_ButtonPress_T) data)
    {
        retcode = CmdProcessor_EnqueueFromIsr(ButtonSetup.CmdProcessorHandle, ButtonEventDequeue, NULL, (uint32_t) INTERNAL_BUTTON_2_PRESSED);
    }
    else if (BSP_XDK_BUTTON_RELEASE == (BSP_ButtonPress_T) data)
    {
        retcode = CmdProcessor_EnqueueFromIsr(ButtonSetup.CmdProcessorHandle, ButtonEventDequeue, NULL, (uint32_t) INTERNAL_BUTTON_2_RELEASED);
    }
    else
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
    }
    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseErrorFromIsr(retcode);
    }
}

/** Refer interface header for description */
Retcode_T Button_Setup(Button_Setup_T * setup)
{
    Retcode_T retcode = RETCODE_OK;
    if (NULL == setup)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        if ((setup->InternalButton1isEnabled) || (setup->InternalButton2isEnabled))
        {
            ButtonSetup = *setup;
        }
        else
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
        }
    }
    return (retcode);
}

/** Refer interface header for description */
Retcode_T Button_Enable(void)
{
    Retcode_T retcode = RETCODE_OK;

    retcode = BSP_Button_Connect();
    if (retcode == RETCODE_OK)
    {
        if (ButtonSetup.InternalButton1isEnabled)
        {
            retcode = BSP_Button_Enable((uint32_t) BSP_XDK_BUTTON_1, Button1Callback);
        }
    }
    if (retcode == RETCODE_OK)
    {
        if (ButtonSetup.InternalButton2isEnabled)
        {
            retcode = BSP_Button_Enable((uint32_t) BSP_XDK_BUTTON_2, Button2Callback);
        }
    }

    return (retcode);
}
#endif /* XDK_SENSOR_BUTTON */

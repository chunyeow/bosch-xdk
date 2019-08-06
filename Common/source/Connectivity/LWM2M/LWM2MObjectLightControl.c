/*
* Licensee agrees that the example code provided to Licensee has been developed and released by Bosch solely as an example to be used as a potential reference for Licensee’s application development. 
* Fitness and suitability of the example code for any use within Licensee’s applications need to be verified by Licensee on its own authority by taking appropriate state of the art actions and measures (e.g. by means of quality assurance measures).
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
*
* Contributors:
* Bosch Software Innovations GmbH
*/
/*----------------------------------------------------------------------------*/

/**
 * @brief This file provides the implementation of LWM2MObjectLightControl module.
 *
 * @file
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_LWM2MOBJECTLIGHTCONTROL

/* own header files */
#include "LWM2M.h"
#include "LWM2MObjectLightControl.h"
#include "LWM2MObjects.h"
#include "LWM2MUtil.h"

/* additional interface header files */
#include <Serval_Exceptions.h>
#include <Serval_Lwm2m.h>

#define LIGHTCONTROL_RESOURCES_INDEX(res) LWM2M_RESOURCES_INDEX(LightRedResources, res)
static LWM2M_MUTEX_INSTANCE(mutex) = LWM2M_MUTEX_INIT_VALUE;

static retcode_t RW_Red(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr);

static retcode_t RW_Orange(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr);

static retcode_t RW_Yellow(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr);

static volatile bool Red = false;
static volatile bool Orange = false;
static volatile bool Yellow = false;
static volatile bool Started = false;
static volatile bool LedTestMode = false;

LWM2MObjectLightControl_Resource_T LightRedResources =
        {
                { 5706, LWM2M_STRING_RO("255,0,0") },
                { 5850, LWM2M_DYNAMIC(RW_Red) },
        };

LWM2MObjectLightControl_Resource_T LightOrangeResources =
        {
                { 5706, LWM2M_STRING_RO("255,165,0") },
                { 5850, LWM2M_DYNAMIC(RW_Orange) },
        };

LWM2MObjectLightControl_Resource_T LightYellowResources =
        {
                { 5706, LWM2M_STRING_RO("255,255,0") },
                { 5850, LWM2M_DYNAMIC(RW_Yellow) },
        };

static bool AdjustState(volatile bool* current, bool state)
{
    bool Result = false;
    if (LWM2M_MUTEX_LOCK(mutex))
    {
        if (*current != state)
        {
            *current = state;
            Result = true;
        }
        LWM2M_MUTEX_UNLOCK(mutex);
    }
    return Result;
}

static void NotifyRed(bool state)
{
    if (AdjustState(&Red, state) && Started)
    {
        Lwm2m_URI_Path_T path = { LWM2MOBJECTS_IX_LIGHTCONTROL_0, LWM2MOBJECTS_IX_LIGHTCONTROL_0, LIGHTCONTROL_RESOURCES_INDEX(onOff) };
        /*lint -e(534) purposefully suppressing the warning:Ignoring return value of function,since in CONF mode this may fail many times */
        Lwm2mReporting_resourceChanged(&path);
    }
}

static void NotifyOrange(bool state)
{
    if (AdjustState(&Orange, state) && Started)
    {
        Lwm2m_URI_Path_T path = { LWM2MOBJECTS_IX_LIGHTCONTROL_0, LWM2MOBJECTS_IX_LIGHTCONTROL_1, LIGHTCONTROL_RESOURCES_INDEX(onOff) };
        /*lint -e(534) purposefully suppressing the warning:Ignoring return value of function,since in CONF mode this may fail many times */
        Lwm2mReporting_resourceChanged(&path);
    }
}

static void NotifyYellow(bool state)
{
    if (AdjustState(&Yellow, state) && Started)
    {
        Lwm2m_URI_Path_T path = { LWM2MOBJECTS_IX_LIGHTCONTROL_0, LWM2MOBJECTS_IX_LIGHTCONTROL_2, LIGHTCONTROL_RESOURCES_INDEX(onOff) };
        /*lint -e(534) purposefully suppressing the warning:Ignoring return value of function,since in CONF mode this may fail many times */
        Lwm2mReporting_resourceChanged(&path);
    }
}

static retcode_t LightControl(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr,
        volatile bool* state, void (*handler)(bool state))
{
    retcode_t rc = RC_LWM2M_METHOD_NOT_ALLOWED;
    if (NULL != parser_ptr)
    {
        /* write value */
        bool value;
        rc = Lwm2mParser_getBool(parser_ptr, &value);
        if (RC_OK == rc && *state != value)
        {
            if (&Red == state)
            {
                printf("LWM2M server set LED red to %d\n", value ? 1 : 0);
            }
            else if (&Orange == state)
            {
                printf("LWM2M server set LED orange to %d\n", value ? 1 : 0);
            }
            else if (&Yellow == state)
            {
                printf("LWM2M server set LED yellow to %d\n", value ? 1 : 0);
            }
            if (!LedTestMode)
                handler(value);
        }
    }
    else if ( NULL != serializer_ptr)
    {
        /* read value */
        rc = Lwm2mSerializer_serializeBool(serializer_ptr, *state);
    }
    return rc;
}

static retcode_t RW_Red(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    return LightControl(serializer_ptr, parser_ptr, &Red, LWM2M_RedLedSetState);
}

static retcode_t RW_Orange(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    return LightControl(serializer_ptr, parser_ptr, &Orange, LWM2M_OrangeLedSetState);
}

static retcode_t RW_Yellow(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    return LightControl(serializer_ptr, parser_ptr, &Yellow, LWM2M_YellowLedSetState);
}

/* global functions ********************************************************* */

/** Refer interface header for description */
void LWM2MObjectLightControl_Init(void)
{
    Started = false;
    LWM2M_MUTEX_CREATE(mutex);
    LWM2M_SetRedLedStateChangeHandler(NotifyRed);
    LWM2M_SetOrangeLedStateChangeHandler(NotifyOrange);
    LWM2M_SetYellowLedStateChangeHandler(NotifyYellow);
}

/** Refer interface header for description */
void LWM2MObjectLightControl_Enable(bool testMode)
{
    LedTestMode = testMode;
    Started = true;
}

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
 * @brief This file provides the implementation of LWM2MObjectTemperature module.
 *
 * @file
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_LWM2MOBJECTTEMPERATURE

/* own header files */
#include "LWM2MObjects.h"
#include "LWM2MUtil.h"
#include "LWM2M.h"

/* additional interface header files */
#include <Serval_Clock.h>
#include <Serval_Exceptions.h>
#include <BCDS_Retcode.h>
#include "XDK_SensorHandle.h"
/* global variables ********************************************************* */

/* local variables ********************************************************** */

/*lint -e(956) const ?*/
static Lwm2m_URI_Path_T TemperatureUriPath = { LWM2MOBJECTS_IX_TEMPERATURE_0, LWM2MOBJECTS_IX_TEMPERATURE_0, -1 };

/* constant definitions ***************************************************** */
#define TEMPERATURE_RESOURCE_INDEX(res) LWM2M_RESOURCES_INDEX(LWM2MObjectTemperatureResources, res)

#define FLUSH_RESOURCES	Lwm2mReporting_multipleResourcesChanged(&TemperatureUriPath, 6, \
		TEMPERATURE_RESOURCE_INDEX(minRangeValue),\
		TEMPERATURE_RESOURCE_INDEX(maxRangeValue),\
		TEMPERATURE_RESOURCE_INDEX(units),\
		TEMPERATURE_RESOURCE_INDEX(sensorValue),\
		TEMPERATURE_RESOURCE_INDEX(minMeasuredValue),\
		TEMPERATURE_RESOURCE_INDEX(maxMeasuredValue))

/* local functions ********************************************************** */

static retcode_t ResetMinMaxValues(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr);
static void LWM2MObjectTemperatureInternalEnable(float minRangeValue_C, float maxRangeValue_C);
static void LWM2MObjectTemperatureInternalDisable(void);
static void LWM2MObjectTemperatureInternalSetValue(float sensorValue_C);

/* TODO need a way to define inactive resources. */
LWM2MObjectTemperatureResource_T LWM2MObjectTemperatureResources =
        {
                { 5601, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, /* minMeassuredValue|R|Single|O|Float |   | */
                { 5602, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, /* maxMeassuredValue|R|Single|O|Float |   | */
                { 5603, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, /* minRangeValue    |R|Single|O|Float |   | */
                { 5604, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, /* maxRangeValue    |R|Single|O|Float |   | */
                { 5605, LWM2M_FUNCTION(ResetMinMaxValues) }, /* resetMinMaxValue |E|Single|O|      |   | */
                { 5700, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, /* sensorValue      |R|Single|M|Float |   | */
                { 5701, LWM2M_STRING_RO("") }, /* units            |R|Single|O|String|   |see:http://unitsofmeasure.org/ucum.html */
        };

static Lwm2m_Pair_Resource_Update_T AsyncCall_Enabler = { .set_pair = LWM2MObjectTemperatureInternalEnable, .mutex = LWM2M_MUTEX_INIT_VALUE };
static Lwm2m_Call_T AsyncCall_Disabler = { .call = LWM2MObjectTemperatureInternalDisable };
static Lwm2m_Single_Resource_Update_T AsyncCall_Updater = { .set_single = LWM2MObjectTemperatureInternalSetValue, .mutex = LWM2M_MUTEX_INIT_VALUE };
/*lint -e(956) after initialization only accessed by serval scheduler */
static bool MinMaxInit = false;
static volatile bool Started = false;

static retcode_t ResetMinMaxValues(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    (void) parser_ptr;
    (void) serializer_ptr;

    INIT_LWM2M_DYNAMIC_CHANGES(changes);

    LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectTemperatureResources, minMeasuredValue, LWM2MObjectTemperatureResources.sensorValue.data.f);
    LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectTemperatureResources, maxMeasuredValue, LWM2MObjectTemperatureResources.sensorValue.data.f);
    if (Started)
        LWM2M_DYNAMIC_CHANGES_REPORT(changes, TemperatureUriPath);

    printf("Temperature: reset min/max Values to current\r\n");
    return (RC_OK);
}

static void LWM2MObjectTemperatureInternalEnable(float minRangeValue_C, float maxRangeValue_C)
{
    Started = true;
    LWM2MObjectTemperatureResources.units.data.s = "Cel";
    LWM2MObjectTemperatureResources.minRangeValue.data.f = minRangeValue_C;
    LWM2MObjectTemperatureResources.maxRangeValue.data.f = maxRangeValue_C;
    LWM2MObjectTemperatureResources.minMeasuredValue.data.f = minRangeValue_C;
    LWM2MObjectTemperatureResources.maxMeasuredValue.data.f = minRangeValue_C;
    LWM2MObjectTemperatureResources.sensorValue.data.f = minRangeValue_C;
    MinMaxInit = false;
    /*lint -e(534) purposefully suppressing the warning:Ignoring return value of function,since in CONF mode this may fail many times */
    FLUSH_RESOURCES;
}

static void LWM2MObjectTemperatureInternalDisable(void)
{
    Started = true;
    LWM2MObjectTemperatureResources.units.data.s = "";
    LWM2MObjectTemperatureResources.minRangeValue.data.f = 0.0F;
    LWM2MObjectTemperatureResources.maxRangeValue.data.f = 0.0F;
    LWM2MObjectTemperatureResources.minMeasuredValue.data.f = 0.0F;
    LWM2MObjectTemperatureResources.maxMeasuredValue.data.f = 0.0F;
    LWM2MObjectTemperatureResources.sensorValue.data.f = 0.0F;
    MinMaxInit = false;
    /*lint -e(534) purposefully suppressing the warning:Ignoring return value of function,since in CONF mode this may fail many times */
    FLUSH_RESOURCES;
}

static void LWM2MObjectTemperatureInternalSetValue(float sensorValue_C)
{
    INIT_LWM2M_DYNAMIC_CHANGES(changes);

    if (MinMaxInit)
    {
        if (LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectTemperatureResources, sensorValue, sensorValue_C))
        {
            LWM2M_DYNAMIC_CHANGES_SET_FLOAT_MIN_VALUE(changes, LWM2MObjectTemperatureResources, minMeasuredValue, sensorValue_C);
            LWM2M_DYNAMIC_CHANGES_SET_FLOAT_MAX_VALUE(changes, LWM2MObjectTemperatureResources, maxMeasuredValue, sensorValue_C);
            if (Started)
                LWM2M_DYNAMIC_CHANGES_REPORT(changes, TemperatureUriPath);
        }
    }
    else
    {
        MinMaxInit = true;
        LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectTemperatureResources, sensorValue, sensorValue_C);
        LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectTemperatureResources, minMeasuredValue, sensorValue_C);
        LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectTemperatureResources, maxMeasuredValue, sensorValue_C);
        if (Started)
            LWM2M_DYNAMIC_CHANGES_REPORT(changes, TemperatureUriPath);
    }
}

/* global functions ********************************************************* */

/** Refer interface header for description */
void LWM2MObjectTemperature_Init(void)
{
    Started = false;
    LWM2M_MUTEX_CREATE(AsyncCall_Updater.mutex);
    AsyncCall_Enabler.mutex = AsyncCall_Updater.mutex;
}

/** Refer interface header for description */
void LWM2MObjectTemperature_Enable(float minRangeValue_C, float maxRangeValue_C)
{
    LWM2MUtil_UpdatePairResources(minRangeValue_C, maxRangeValue_C, &AsyncCall_Enabler);
}

/** Refer interface header for description */
void LWM2MObjectTemperature_Disable(void)
{
    LWM2MUtil_Schedule(&AsyncCall_Disabler);
}

/** Refer interface header for description */
void LWM2MObjectTemperature_SetValue(float sensorValue_C)
{
    LWM2MUtil_UpdateSingleResource(sensorValue_C, &AsyncCall_Updater);
}

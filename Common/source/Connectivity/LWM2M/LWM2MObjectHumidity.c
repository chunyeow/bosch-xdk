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
 * @brief This file provides the implementation of LWM2MObjectHumidity module.
 *
 * @file
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_LWM2MOBJECTHUMIDITY
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
static Lwm2m_URI_Path_T HumidityUriPath = { LWM2MOBJECTS_IX_HUMIDITY_0, LWM2MOBJECTS_IX_HUMIDITY_0, -1 };
static volatile bool started = false;

/* constant definitions ***************************************************** */
#define HUMIDITY_RESOURCE_INDEX(res) LWM2M_RESOURCES_INDEX(LWM2MObjectHumidityResources, res)

#define FLUSH_RESOURCES	Lwm2mReporting_multipleResourcesChanged(&HumidityUriPath, 6, \
		HUMIDITY_RESOURCE_INDEX(minRangeValue),\
		HUMIDITY_RESOURCE_INDEX(maxRangeValue),\
		HUMIDITY_RESOURCE_INDEX(units),\
		HUMIDITY_RESOURCE_INDEX(sensorValue),\
		HUMIDITY_RESOURCE_INDEX(minMeasuredValue),\
		HUMIDITY_RESOURCE_INDEX(maxMeasuredValue))

/* local functions ********************************************************** */
static retcode_t ResetMinMaxValues(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr);
static void LWM2MObjectHumidityInternalEnable(float minRangeValue, float maxRangeValue);
static void LWM2MObjectHumidityInternalDisable(void);
static void LWM2MObjectHumidityInternalSetValue(float sensorValue_prh);

/* TODO need a way to define inactive resources. */
LWM2MObjectHumidityResource_T LWM2MObjectHumidityResources =
        {
                { 5601, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, // minMeassuredValue|R|Single|O|Float |   |
                { 5602, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, // maxMeassuredValue|R|Single|O|Float |   |
                { 5603, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, // minRangeValue    |R|Single|O|Float |   |
                { 5604, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, // maxRangeValue    |R|Single|O|Float |   |
                { 5605, LWM2M_FUNCTION(ResetMinMaxValues) }, // resetMinMaxValue |E|Single|O|      |   |
                { 5700, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, // sensorValue      |R|Single|M|Float |   |
                { 5701, LWM2M_STRING_RO("") }, // units            |R|Single|O|String|   |see:http://unitsofmeasure.org/ucum.html
        };

/*lint -e(956) */
static bool MinMaxInit = false;

static Lwm2m_Pair_Resource_Update_T AsyncCall_Enabler = { .set_pair = LWM2MObjectHumidityInternalEnable, .mutex = LWM2M_MUTEX_INIT_VALUE };
static Lwm2m_Call_T AsyncCall_Disabler = { .call = LWM2MObjectHumidityInternalDisable };
static Lwm2m_Single_Resource_Update_T AsyncCall_Updater = { .set_single = LWM2MObjectHumidityInternalSetValue, .mutex = LWM2M_MUTEX_INIT_VALUE };

static retcode_t ResetMinMaxValues(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    (void) parser_ptr;
    (void) serializer_ptr;

    INIT_LWM2M_DYNAMIC_CHANGES(changes);

    LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectHumidityResources, minMeasuredValue, LWM2MObjectHumidityResources.sensorValue.data.f);
    LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectHumidityResources, maxMeasuredValue, LWM2MObjectHumidityResources.sensorValue.data.f);
    if (started)
        LWM2M_DYNAMIC_CHANGES_REPORT(changes, HumidityUriPath);

    printf("Humidity: reset min/max Values to current\r\n");
    return (RC_OK);
}

static void LWM2MObjectHumidityInternalEnable(float minRangeValue, float maxRangeValue)
{
    started = true;
    LWM2MObjectHumidityResources.units.data.s = "%";
    LWM2MObjectHumidityResources.minRangeValue.data.f = minRangeValue;
    LWM2MObjectHumidityResources.maxRangeValue.data.f = maxRangeValue;
    LWM2MObjectHumidityResources.minMeasuredValue.data.f = minRangeValue;
    LWM2MObjectHumidityResources.maxMeasuredValue.data.f = minRangeValue;
    LWM2MObjectHumidityResources.sensorValue.data.f = minRangeValue;
    MinMaxInit = false;
    /*lint -e(534) purposefully suppressing the warning:Ignoring return value of function,since in CONF mode this may fail many times */
    FLUSH_RESOURCES;
}

static void LWM2MObjectHumidityInternalDisable(void)
{
    started = true;
    LWM2MObjectHumidityResources.units.data.s = "";
    LWM2MObjectHumidityResources.minRangeValue.data.f = 0.0F;
    LWM2MObjectHumidityResources.maxRangeValue.data.f = 0.0F;
    LWM2MObjectHumidityResources.minMeasuredValue.data.f = 0.0F;
    LWM2MObjectHumidityResources.maxMeasuredValue.data.f = 0.0F;
    LWM2MObjectHumidityResources.sensorValue.data.f = 0.0F;
    MinMaxInit = false;
    /*lint -e(534) purposefully suppressing the warning:Ignoring return value of function,since in CONF mode this may fail many times */
    FLUSH_RESOURCES;
}

static void LWM2MObjectHumidityInternalSetValue(float sensorValue_prh)
{
    INIT_LWM2M_DYNAMIC_CHANGES(changes);

    if (MinMaxInit)
    {
        if (LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectHumidityResources, sensorValue, sensorValue_prh))
        {
            LWM2M_DYNAMIC_CHANGES_SET_FLOAT_MIN_VALUE(changes, LWM2MObjectHumidityResources, minMeasuredValue, sensorValue_prh);
            LWM2M_DYNAMIC_CHANGES_SET_FLOAT_MAX_VALUE(changes, LWM2MObjectHumidityResources, maxMeasuredValue, sensorValue_prh);
            if (started)
                LWM2M_DYNAMIC_CHANGES_REPORT(changes, HumidityUriPath);
        }
    }
    else
    {
        MinMaxInit = true;
        LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectHumidityResources, sensorValue, sensorValue_prh);
        LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectHumidityResources, minMeasuredValue, sensorValue_prh);
        LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectHumidityResources, maxMeasuredValue, sensorValue_prh);
        if (started)
            LWM2M_DYNAMIC_CHANGES_REPORT(changes, HumidityUriPath);
    }
}

/* global functions ********************************************************* */

/** Refer interface header for description */
void LWM2MObjectHumidity_Init(void)
{
    started = false;
    LWM2M_MUTEX_CREATE(AsyncCall_Updater.mutex);
    AsyncCall_Enabler.mutex = AsyncCall_Updater.mutex;
}

/** Refer interface header for description */
void LWM2MObjectHumidity_Enable(float minRangeValue, float maxRangeValue)
{
    LWM2MUtil_UpdatePairResources(minRangeValue, maxRangeValue, &AsyncCall_Enabler);
}

/** Refer interface header for description */
void LWM2MObjectHumidity_Disable(void)
{
    LWM2MUtil_Schedule(&AsyncCall_Disabler);
}

/** Refer interface header for description */
void LWM2MObjectHumidity_SetValue(float sensorValue_prh)
{
    LWM2MUtil_UpdateSingleResource(sensorValue_prh, &AsyncCall_Updater);
}

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
 * @brief This file provides the implementation of LWM2MObjectMagnetometer module.
 *
 * @file
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_LWM2MOBJECTMAGNETOMETER

/* own header files */
#include "LWM2MObjects.h"
#include "LWM2MUtil.h"
#include "LWM2M.h"

/* system header files */
#include <math.h>

/* additional interface header files */
#include <Serval_Exceptions.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAGNETOMETER_RESOURCES_INDEX(res) LWM2M_RESOURCES_INDEX(LWM2MObjectMagnetometerResources, res)

#define FLUSH_RESOURCES	Lwm2mReporting_multipleResourcesChanged(&MagnetometerUriPath, 7, \
			MAGNETOMETER_RESOURCES_INDEX(units),\
			MAGNETOMETER_RESOURCES_INDEX(minRangeValue),\
			MAGNETOMETER_RESOURCES_INDEX(maxRangeValue),\
			MAGNETOMETER_RESOURCES_INDEX(direction),\
			MAGNETOMETER_RESOURCES_INDEX(xValue),\
			MAGNETOMETER_RESOURCES_INDEX(yValue),\
			MAGNETOMETER_RESOURCES_INDEX(zValue))

static void LWM2MObjectMagnetometerInternalEnable(float minRangeValue, float maxRangeValue);
static void LWM2MObjectMagnetometerInternalDisable(void);
static void LWM2MObjectMagnetometerInternalSetValues(float xValue, float yValue, float zValue);

/*lint -e(956) const ?*/
static Lwm2m_URI_Path_T MagnetometerUriPath = { LWM2MOBJECTS_IX_MAGNETOMETER_0, LWM2MOBJECTS_IX_MAGNETOMETER_0, -1 };

static Lwm2m_Pair_Resource_Update_T AsyncCall_Enabler = { .set_pair = LWM2MObjectMagnetometerInternalEnable, .mutex = LWM2M_MUTEX_INIT_VALUE };
static Lwm2m_Call_T AsyncCall_Disabler = { .call = LWM2MObjectMagnetometerInternalDisable };
static Lwm2m_Tripple_Resource_Update_T AsyncCall_Updater = { .set_tripple = LWM2MObjectMagnetometerInternalSetValues, .mutex = LWM2M_MUTEX_INIT_VALUE };
static volatile bool Started = false;

LWM2MObjectMagnetometer_Resource_T LWM2MObjectMagnetometerResources =
        {
                { 5603, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, /* minRangeValue|R|Single|O|Float |   | */
                { 5604, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, /* maxRangeValue|R|Single|O|Float |   | */
                { 5701, LWM2M_STRING_RO("") }, /* unit 		|R|Single|O|String|ToDo:? >UCUM spec */
                { 5702, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, /* xValue		|R|Single|M|Float |	  | */
                { 5703, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, /* yValue		|R|Single|O|Float |   | */
                { 5704, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, /* zValue		|R|Single|O|Float |   | */
                { 5705, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, /* direction	|R|Single|O|Float |   | */
        };

static float CalculateDirection(float xValue, float yValue)
{
    double heading = atan2(yValue, xValue);
    heading += MAGNETOMETER_DECLANATIONANGLE;

    if (heading < 0)
        heading += 2 * M_PI;

    if (heading > 2 * M_PI)
        heading -= 2 * M_PI;

    return (float) (heading * 180 / M_PI);
}

static void LWM2MObjectMagnetometerInternalEnable(float minRangeValue, float maxRangeValue)
{
    Started = true;
    LWM2MObjectMagnetometerResources.minRangeValue.data.f = minRangeValue;
    LWM2MObjectMagnetometerResources.maxRangeValue.data.f = maxRangeValue;
    LWM2MObjectMagnetometerResources.units.data.s = "uT";
    LWM2MObjectMagnetometerResources.xValue.data.f = minRangeValue;
    LWM2MObjectMagnetometerResources.yValue.data.f = minRangeValue;
    LWM2MObjectMagnetometerResources.zValue.data.f = minRangeValue;
    LWM2MObjectMagnetometerResources.direction.data.f = CalculateDirection(LWM2MObjectMagnetometerResources.xValue.data.f, LWM2MObjectMagnetometerResources.yValue.data.f);
    /*lint -e(534) purposefully suppressing the warning:Ignoring return value of function,since in CONF mode this may fail many times */
    FLUSH_RESOURCES;
}

static void LWM2MObjectMagnetometerInternalDisable(void)
{
    Started = true;
    LWM2MObjectMagnetometerResources.minRangeValue.data.f = 0.0F;
    LWM2MObjectMagnetometerResources.maxRangeValue.data.f = 0.0F;
    LWM2MObjectMagnetometerResources.units.data.s = "";
    LWM2MObjectMagnetometerResources.xValue.data.f = 0.0F;
    LWM2MObjectMagnetometerResources.yValue.data.f = 0.0F;
    LWM2MObjectMagnetometerResources.zValue.data.f = 0.0F;
    LWM2MObjectMagnetometerResources.direction.data.f = 0.0F;
    /*lint -e(534) purposefully suppressing the warning:Ignoring return value of function,since in CONF mode this may fail many times */
    FLUSH_RESOURCES;
}

static void LWM2MObjectMagnetometerInternalSetValues(float xValue, float yValue, float zValue)
{
    INIT_LWM2M_DYNAMIC_CHANGES(changes);

    LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectMagnetometerResources, xValue, xValue);
    LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectMagnetometerResources, yValue, yValue);
    if (0 < changes.changes)
    {
        float heading = CalculateDirection(xValue, yValue);

        LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectMagnetometerResources, direction, heading);
    }

    LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectMagnetometerResources, zValue, zValue);
    if (Started)
        LWM2M_DYNAMIC_CHANGES_REPORT(changes, MagnetometerUriPath);
}

/* global functions ********************************************************* */

/** Refer interface header for description */
void LWM2MObjectMagnetometer_Init(void)
{
    Started = false;
    LWM2M_MUTEX_CREATE(AsyncCall_Updater.mutex);
    AsyncCall_Enabler.mutex = AsyncCall_Updater.mutex;
}

/** Refer interface header for description */
void LWM2MObjectMagnetometer_Enable(float minRangeValue, float maxRangeValue)
{
    LWM2MUtil_UpdatePairResources(minRangeValue, maxRangeValue, &AsyncCall_Enabler);
}

/** Refer interface header for description */
void LWM2MObjectMagnetometer_Disable(void)
{
    LWM2MUtil_Schedule(&AsyncCall_Disabler);
}

/** Refer interface header for description */
void LWM2MObjectMagnetometer_SetValues(float xValue, float yValue, float zValue)
{
    LWM2MUtil_UpdateTrippleResources(xValue, yValue, zValue, &AsyncCall_Updater);
}

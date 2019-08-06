/*
* Licensee agrees that the example code provided to Licensee has been developed and released by Bosch solely as an example to be used as a potential reference for Licensee�s application development. 
* Fitness and suitability of the example code for any use within Licensee�s applications need to be verified by Licensee on its own authority by taking appropriate state of the art actions and measures (e.g. by means of quality assurance measures).
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
 * @brief This file provides the implementation of LWM2MObjectIlluminance module.
 *
 * @file
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_LWM2MOBJECTILLUMINANCE
/* own header files */
#include "LWM2MObjects.h"
#include "LWM2MUtil.h"
#include "LWM2M.h"

/* system header files */
#include "BCDS_Basics.h"

/* additional interface header files */
#include <Serval_Exceptions.h>

#define ILLUMINANCE_RESOURCES_INDEX(res) LWM2M_RESOURCES_INDEX(LWM2MObjectIlluminanceResources, res)

#define FLUSH_RESOURCES	Lwm2mReporting_multipleResourcesChanged(&IlluminanceUriPath, 6, \
			ILLUMINANCE_RESOURCES_INDEX(minValue),\
			ILLUMINANCE_RESOURCES_INDEX(maxValue),\
			ILLUMINANCE_RESOURCES_INDEX(units),\
			ILLUMINANCE_RESOURCES_INDEX(value),\
			ILLUMINANCE_RESOURCES_INDEX(minRangeValue),\
			ILLUMINANCE_RESOURCES_INDEX(maxRangeValue))

static retcode_t ResetMinMax(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr);
static void LWM2MObjectIlluminanceInternalEnable(float minRange, float maxRange);
static void LWM2MObjectIlluminanceInternalDisable(void);
static void LWM2MObjectIlluminanceInternalSetValue(float currentValue);

LWM2MObjectIlluminance_Resource_T LWM2MObjectIlluminanceResources =
        {
                { 5601, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, // minValue			|R|Single|M|Float |	  |
                { 5602, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, // maxValue			|R|Single|O|Float |   |
                { 5603, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, // minRangeValue	|R|Single|O|Float |   |
                { 5604, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, // minRangeValue	|R|Single|O|Float |   |
                { 5605, LWM2M_FUNCTION(ResetMinMax) }, // reset			|R|Single|O|Float |   |
                { 5700, LWM2M_FLOAT(0.0F) | LWM2M_READ_ONLY }, // Value			|R|Single|O|Float |   |
                { 5701, LWM2M_STRING_RO("") }, // unit 			|R|Single|O|String|   |
        };

/*lint -e(956) */
static bool MinMaxInit = false;

/*lint -e(956) const ?*/
static Lwm2m_URI_Path_T IlluminanceUriPath = { LWM2MOBJECTS_IX_ILLUMINANCE_0, LWM2MOBJECTS_IX_ILLUMINANCE_0, -1 };

static Lwm2m_Single_Resource_Update_T AsyncCall_Updater = { .set_single = LWM2MObjectIlluminanceInternalSetValue, .mutex = LWM2M_MUTEX_INIT_VALUE };
static Lwm2m_Pair_Resource_Update_T AsyncCall_Enabler = { .set_pair = LWM2MObjectIlluminanceInternalEnable, .mutex = LWM2M_MUTEX_INIT_VALUE };
static Lwm2m_Call_T AsyncCall_Disabler = { .call = LWM2MObjectIlluminanceInternalDisable };
static volatile bool Started = false;

static retcode_t ResetMinMax(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    (void) parser_ptr;
    (void) serializer_ptr;

    INIT_LWM2M_DYNAMIC_CHANGES(changes);

    LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectIlluminanceResources, minValue, LWM2MObjectIlluminanceResources.value.data.f);
    LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectIlluminanceResources, maxValue, LWM2MObjectIlluminanceResources.value.data.f);
    if (Started)
        LWM2M_DYNAMIC_CHANGES_REPORT(changes, IlluminanceUriPath);

    return (RC_OK);
}

static void LWM2MObjectIlluminanceInternalEnable(float minRange, float maxRange)
{
    Started = true;
    LWM2MObjectIlluminanceResources.minValue.data.f = minRange;
    LWM2MObjectIlluminanceResources.maxValue.data.f = minRange;
    LWM2MObjectIlluminanceResources.minRangeValue.data.f = minRange;
    LWM2MObjectIlluminanceResources.maxRangeValue.data.f = maxRange;
    LWM2MObjectIlluminanceResources.value.data.f = minRange;
    LWM2MObjectIlluminanceResources.units.data.s = "mlx";
    MinMaxInit = false;
    /*lint -e(534) purposefully suppressing the warning:Ignoring return value of function,since in CONF mode this may fail many times */
    FLUSH_RESOURCES;
}

static void LWM2MObjectIlluminanceInternalDisable(void)
{
    Started = true;
    LWM2MObjectIlluminanceResources.minValue.data.f = 0.0F;
    LWM2MObjectIlluminanceResources.maxValue.data.f = 0.0F;
    LWM2MObjectIlluminanceResources.minRangeValue.data.f = 0.0F;
    LWM2MObjectIlluminanceResources.maxRangeValue.data.f = 0.0F;
    LWM2MObjectIlluminanceResources.value.data.f = 0.0F;
    LWM2MObjectIlluminanceResources.units.data.s = "";
    MinMaxInit = false;
    /*lint -e(534) purposefully suppressing the warning:Ignoring return value of function,since in CONF mode this may fail many times */
    FLUSH_RESOURCES;
}

static void LWM2MObjectIlluminanceInternalSetValue(float currentValue)
{
    INIT_LWM2M_DYNAMIC_CHANGES(changes);

    if (MinMaxInit)
    {
        if (LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectIlluminanceResources, value, currentValue))
        {
            LWM2M_DYNAMIC_CHANGES_SET_FLOAT_MIN_VALUE(changes, LWM2MObjectIlluminanceResources, minValue, currentValue);
            LWM2M_DYNAMIC_CHANGES_SET_FLOAT_MAX_VALUE(changes, LWM2MObjectIlluminanceResources, maxValue, currentValue);
            if (Started)
                LWM2M_DYNAMIC_CHANGES_REPORT(changes, IlluminanceUriPath);
        }
    }
    else
    {
        MinMaxInit = true;
        LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectIlluminanceResources, value, currentValue);
        LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectIlluminanceResources, minValue, currentValue);
        LWM2M_DYNAMIC_CHANGES_SET_FLOAT_VALUE(changes, LWM2MObjectIlluminanceResources, maxValue, currentValue);
        if (Started)
            LWM2M_DYNAMIC_CHANGES_REPORT(changes, IlluminanceUriPath);
    }
}

/* global functions ********************************************************* */

/** Refer interface header for description */
void LWM2MObjectIlluminance_Init(void)
{
    Started = false;
    LWM2M_MUTEX_CREATE(AsyncCall_Updater.mutex);
    AsyncCall_Enabler.mutex = AsyncCall_Updater.mutex;
}

/** Refer interface header for description */
void LWM2MObjectIlluminance_Enable(float minRange, float maxRange)
{
    LWM2MUtil_UpdatePairResources(minRange, maxRange, &AsyncCall_Enabler);
}

/** Refer interface header for description */
void LWM2MObjectIlluminance_Disable(void)
{
    LWM2MUtil_Schedule(&AsyncCall_Disabler);
}

/** Refer interface header for description */
void LWM2MObjectIlluminance_SetValue(float currentValue)
{
    LWM2MUtil_UpdateSingleResource(currentValue, &AsyncCall_Updater);
}

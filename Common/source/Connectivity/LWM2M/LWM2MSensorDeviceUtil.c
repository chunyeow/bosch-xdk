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
 * @brief This file provides the implementation of LWM2MSensorDeviceUtil module.
 *
 * @file
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_LWM2MSENSORDEVICEUTIL
/* own header files */
#include "LWM2MSensorDeviceUtil.h"

/* global functions ********************************************************* */

/** Refer interface header for description */
void LWM2MSensorDeviceUtil_ResetProcessDataFloat(SensorDeviceProcessDataFloat_T* data)
{
    int Index;
    for (Index = 0; Index < data->value_counter; ++Index)
    {
        data->data.values[Index] = 0.0;
    }
    data->sample_counter = 0;
}

/** Refer interface header for description */
void LWM2MSensorDeviceUtil_ProcessDataFloat(ProcessingMode_T mode, SensorDeviceProcessDataFloat_T* data, SensorDeviceSampleDataFloat_T* sample)
{
    int Index;

    if (!data->enabled)
        return;

    if (data->mode != mode)
    {
        data->mode = mode;
        LWM2MSensorDeviceUtil_ResetProcessDataFloat(data);
    }
    if (0 == data->sample_counter)
    {
        if (MAX == mode || MIN == mode)
            mode = CURRENT;
    }

    ++data->sample_counter;
    switch (mode)
    {
    case MAX:
        for (Index = 0; Index < data->value_counter; ++Index)
        {
            if (data->data.values[Index] < sample->values[Index])
                data->data.values[Index] = sample->values[Index];
        }
        break;
    case MIN:
        for (Index = 0; Index < data->value_counter; ++Index)
        {
            if (data->data.values[Index] > sample->values[Index])
                data->data.values[Index] = sample->values[Index];
        }
        break;
    case CURRENT:
        for (Index = 0; Index < data->value_counter; ++Index)
        {
            data->data.values[Index] = sample->values[Index];
        }
        break;
    case AVG:
        for (Index = 0; Index < data->value_counter; ++Index)
        {
            data->data.values[Index] += sample->values[Index];
        }
        break;
    }
}

/** Refer interface header for description */
bool LWM2MSensorDeviceUtil_GetDataFloat(SensorDeviceProcessDataFloat_T* data, SensorDeviceSampleDataFloat_T* sample)
{
    int Index;
    double Counter;

    if (!data->enabled || 0 == data->sample_counter)
        return false;
    switch (data->mode)
    {
    case MAX:
        case MIN:
        case CURRENT:
        for (Index = 0; Index < data->value_counter; ++Index)
        {
            sample->values[Index] = data->data.values[Index];
        }
        break;
    case AVG:
        Counter = data->sample_counter;
        for (Index = 0; Index < data->value_counter; ++Index)
        {
            sample->values[Index] = data->data.values[Index] / Counter;
        }
        break;
    }
    LWM2MSensorDeviceUtil_ResetProcessDataFloat(data);
    return true;
}

/**@} */

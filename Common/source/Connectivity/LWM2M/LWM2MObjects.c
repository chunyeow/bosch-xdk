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
 * @brief This file provides the implementation of LWM2MObjects module.
 *
 * @file
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_LWM2MOBJECTS

/* own header files */
#include "LWM2MObjects.h"

/* system header files */
#include <stdio.h>

/* additional interface header files */
#include <Serval_Lwm2m.h>

#define LWM2M_OBJECTID_FIRMWARE 5

/*lint -esym(956, ObjectInstances) ?*/
/* Rule: object instances sequence should keep increasing ObjectID's/ObjectID-Instances */
static Lwm2mObjectInstance_T LWM2MObjectInstances[] =
        {
                /* 0=LWM2MOBJECTS_IX_DEVICE_0 see header */
                { LWM2M_OBJECTID_DEVICE, /* 3 */
                LWM2M_SINGLE_INSTANCE,
                        LWM2M_RESOURCES(LWM2MObjectDeviceResources),
                        .permissions = { LWM2M_ACCESS_CONTROL_OWNER, 0x0, 0x0, 0x0 }
                },
                /* 1=LWM2MOBJECTS_IX_CONNECTIVITY_MONITORING_0 see header */
                { LWM2M_OBJECTID_CONNECTIVITY_MONITORING, /* 4 */
                LWM2M_SINGLE_INSTANCE,
                        LWM2M_RESOURCES(LWM2MObjectConnectivityMonitoringResources),
                        .permissions = { LWM2M_ACCESS_CONTROL_OWNER, 0x0, 0x0, 0x0 }
                },
                /* 2=LWM2MOBJECTS_IX_FIRMWARE_0 */
                { LWM2M_OBJECTID_FIRMWARE, /* 5 */
                LWM2M_SINGLE_INSTANCE,
                        LWM2M_RESOURCES(LWM2MObjectFirmwareUpdateResources),
                        .permissions = { LWM2M_ACCESS_CONTROL_OWNER, 0x0, 0x0, 0x0 }
                },
                /* 3=LWM2MOBJECTS_IX_ILLUMINANCE_0 */
                { LWM2M_OBJECTID_IPSO_ILLUMINANCE, /* 3301/0 */
                0,
                        LWM2M_RESOURCES(LWM2MObjectIlluminanceResources),
                        .permissions = { LWM2M_ACCESS_CONTROL_OWNER, 0x0, 0x0, 0x0 }
                },
                /* 4=LWM2MOBJECTS_IX_TEMPERATURE_0 */
                { LWM2M_OBJECTID_IPSO_TEMPERATURE, /* 3303/0 */
                0,
                        LWM2M_RESOURCES(LWM2MObjectTemperatureResources),
                        .permissions = { LWM2M_ACCESS_CONTROL_OWNER, 0x0, 0x0, 0x0 }
                },
                /* 5=LWM2MOBJECTS_IX_HUMIDITY_0 */
                { LWM2M_OBJECTID_IPSO_HUMIDITY, /* 3304/0 */
                0,
                        LWM2M_RESOURCES(LWM2MObjectHumidityResources),
                        .permissions = { LWM2M_ACCESS_CONTROL_OWNER, 0x0, 0x0, 0x0 }
                },
                /* 6=LWM2MOBJECTS_IX_LIGHTCONTROL_0 */
                { LWM2M_OBJECTID_IPSO_LIGHTCONTROL, /* 3310/0 */
                0,
                        LWM2M_RESOURCES(LightRedResources),
                        .permissions = { LWM2M_ACCESS_CONTROL_OWNER, 0x0, 0x0, 0x0 }
                },
                /* 7=LWM2MOBJECTS_IX_LIGHTCONTROL_1 */
                { LWM2M_OBJECTID_IPSO_LIGHTCONTROL, /* 3310/1 */
                1,
                        LWM2M_RESOURCES(LightOrangeResources),
                        .permissions = { LWM2M_ACCESS_CONTROL_OWNER, 0x0, 0x0, 0x0 }
                },
                /* 8=LWM2MOBJECTS_IX_LIGHTCONTROL_2 */
                { LWM2M_OBJECTID_IPSO_LIGHTCONTROL, /* 3310/2 */
                2,
                        LWM2M_RESOURCES(LightYellowResources),
                        .permissions = { LWM2M_ACCESS_CONTROL_OWNER, 0x0, 0x0, 0x0 }
                },
                /* 9=LWM2MOBJECTS_IX_ACCELEROMETER_0 */
                { LWM2M_OBJECTID_IPSO_ACCELEROMETER, /* 3313/0 */
                0,
                        LWM2M_RESOURCES(LWM2MObjectAccelerometerResources),
                        .permissions = { LWM2M_ACCESS_CONTROL_OWNER, 0x0, 0x0, 0x0 }
                },
                /* 10=LWM2MOBJECTS_IX_MAGNETOMETER_0 */
                { LWM2M_OBJECTID_IPSO_MAGNETOMETER, /* 3314/0 */
                0,
                        LWM2M_RESOURCES(LWM2MObjectMagnetometerResources),
                        .permissions = { LWM2M_ACCESS_CONTROL_OWNER, 0x0, 0x0, 0x0 }
                },
                /* 11=LWM2MOBJECTS_IX_BAROMETER_0 */
                { LWM2M_OBJECTID_IPSO_BAROMETER, /* 3315/0 */
                0,
                        LWM2M_RESOURCES(LWM2MObjectBarometerResources),
                        .permissions = { LWM2M_ACCESS_CONTROL_OWNER, 0x0, 0x0, 0x0 }
                },
                /* 12=LWM2MOBJECTS_IX_GYROSCOPE_0 */
                { LWM2M_OBJECTID_GYROSCOPE, /* 3334/0 */
                0,
                        LWM2M_RESOURCES(LWM2MObjectGyroscopeResources),
                        .permissions = { LWM2M_ACCESS_CONTROL_OWNER, 0x0, 0x0, 0x0 }
                },
                /* 13=LWM2MOBJECTS_IX_SENSORDEVICE_0 */
                { LWM2M_OBJECTID_SENSORDEVICE, /* 15000 */
                LWM2M_SINGLE_INSTANCE,
                        LWM2M_RESOURCES(LWM2MObjectSensorDeviceResources),
                        .permissions = { LWM2M_ACCESS_CONTROL_OWNER, 0x0, 0x0, 0x0 }
                },
                /* 14=LWM2MOBJECTS_IX_ALERTNOTIFICATION_0 */
                { LWM2M_OBJECTID_ALERTNOTIFICATION, /* 15003/0 */
                0,
                        LWM2M_RESOURCES(LWM2MObjectAlertNotificationResources),
                        .permissions = { LWM2M_ACCESS_CONTROL_OWNER, 0x0, 0x0, 0x0 }
                },
        };

/**< LWM2M node context */
Lwm2mDevice_T LWM2MDeviceResourceInfo =
        {
                .name = NULL,
                .binding = UDP,
                .sms = NULL,
                .numberOfObjectInstances = LWM2M_OBJECT_INSTANCE_COUNT(LWM2MObjectInstances),
                .objectInstances = LWM2MObjectInstances,
        };

static void LWM2MObjectsEnableConNotifies(Lwm2mObjectInstance_T* Object)
{
    int Index = 0;
    Lwm2mResource_T* Resources = Object->resources;
    for (; Index < Object->maxNumberOfResources; ++Index)
    {
        /*lint -e(641) Suppressing as this is related to serval stack implementation*/
        if (0 == (LWM2M_NOT_READABLE & Resources[Index].type))
        {
            Resources[Index].type |= LWM2M_CONFIRM_NOTIFY;
        }
    }
}

/* global functions ********************************************************* */

/** Refer interface header for description */
void LWM2MObjects_Init(bool ConNotifies)
{
    if (ConNotifies)
    {
        int Index = 0;
        for (; Index < LWM2MDeviceResourceInfo.numberOfObjectInstances; ++Index)
        {
            LWM2MObjectsEnableConNotifies(&(LWM2MDeviceResourceInfo.objectInstances[Index]));
        }
    }
}

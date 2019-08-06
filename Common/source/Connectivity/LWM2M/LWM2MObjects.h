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
 * @brief Interface header for LWM2MObjects file.
 *
 * @file
 */

#ifndef LWM2MOBJECTS_H_
#define LWM2MOBJECTS_H_

#include <Serval_Lwm2m.h>

/* public type and macro definitions */

/* see Lwm2mObjects.c: aligned to Lwm2mObjectInstance_T objectInstances */
#define LWM2MOBJECTS_IX_DEVICE_0					UINT32_C(0) /**< object or instance index for \link LWM2MObjectDevice.h \endlink in Lwm2m_URI_Path_T */
#define LWM2MOBJECTS_IX_CONN_MON_0					UINT32_C(1) /**< object or instance index for \link LWM2MObjectConnectivityMonitoring.h \endlink in Lwm2m_URI_Path_T */
#define LWM2MOBJECTS_IX_FIRMWARE_0                  UINT32_C(2) /**< object or instance index for \link LWM2MObjectConnectivityMonitoring.h \endlink in Lwm2m_URI_Path_T */
#define LWM2MOBJECTS_IX_ILLUMINANCE_0				UINT32_C(3) /**< object or instance index for \link LWM2MObjectIlluminance.h \endlink in Lwm2m_URI_Path_T */
#define LWM2MOBJECTS_IX_TEMPERATURE_0				UINT32_C(4) /**< object or instance index for \link LWM2MObjectTemperature.h \endlink in Lwm2m_URI_Path_T */
#define LWM2MOBJECTS_IX_HUMIDITY_0					UINT32_C(5) /**< object or instance index for \link LWM2MObjectHumidity.h \endlink in Lwm2m_URI_Path_T */
#define LWM2MOBJECTS_IX_LIGHTCONTROL_0				UINT32_C(6) /**< object or instance 0 index for \link LWM2MObjectLightControl.h \endlink in Lwm2m_URI_Path_T */
#define LWM2MOBJECTS_IX_LIGHTCONTROL_1				UINT32_C(7) /**< instance 1 index for \link LWM2MObjectLightControl.h \endlink in Lwm2m_URI_Path_T */
#define LWM2MOBJECTS_IX_LIGHTCONTROL_2				UINT32_C(8) /**< instance 2 index for \link LWM2MObjectLightControl.h \endlink in Lwm2m_URI_Path_T */
#define LWM2MOBJECTS_IX_ACCELEROMETER_0				UINT32_C(9) /**< object or instance index for \link LWM2MObjectAccelerometer.h \endlink in Lwm2m_URI_Path_T */
#define LWM2MOBJECTS_IX_MAGNETOMETER_0				UINT32_C(10) /**< object or instance index for \link LWM2MObjectMagnetometer.h \endlink in Lwm2m_URI_Path_T */
#define LWM2MOBJECTS_IX_BAROMETER_0					UINT32_C(11) /**< object or instance index for \link LWM2MObjectBarometer.h \endlink in Lwm2m_URI_Path_T */
#define LWM2MOBJECTS_IX_GYROSCOPE_0                 UINT32_C(12) /**< object or instance index for \link LWM2MObjectGyroscope.h \endlink in Lwm2m_URI_Path_T */
#define LWM2MOBJECTS_IX_SENSORDEVICE_0				UINT32_C(13) /**< object or instance index for \link LWM2MObjectSensorDevice.h \endlink in Lwm2m_URI_Path_T */
#define LWM2MOBJECTS_IX_ALERTNOTIFICATION_0			UINT32_C(14) /**< object or instance index for \link LWM2MObjectAlertNotification.h \endlink in Lwm2m_URI_Path_T */

/**
 * @brief Get resource index for a Lwm2m_URI_Path_T.
 *
 * @param[in] table
 * resource table e.g. \link accelerometerResources \endlink
 *
 * @param[in] res
 * resource in resource table e.g. xAxis
 */
#define LWM2M_RESOURCES_INDEX(table, res) (&table.res - (Lwm2mResource_T*)&table)

/*lint -esym(956, DeviceResourceInfo) ?*/
extern Lwm2mDevice_T LWM2MDeviceResourceInfo;

/* -- LWM2M Object Device   ----------------------- */
#include "LWM2MObjectDevice.h"
/* -- LWM2M Object Connectivity Monitoring -------- */
#include "LWM2MObjectConnectivityMonitoring.h"
/* -- LWM2M Object Firmware Update ---------------- */
#include "LWM2MObjectFirmwareUpdate.h"
/* -- LWM2M Object Gyroscope ---------------------- */
#include "LWM2MObjectGyroscope.h"
/* -- LWM2M Object Magnetometer -------- */
#include "LWM2MObjectMagnetometer.h"
/* -- LWM2M Object Illuminance --------- */
#include "LWM2MObjectIlluminance.h"
/* -- LWM2M Object Barometer------------ */
#include "LWM2MObjectBarometer.h"
/* -- LWM2M Object Temperature --------- */
#include "LWM2MObjectTemperature.h"
/* -- LWM2M Object Humidity ------------ */
#include "LWM2MObjectHumidity.h"
/* -- LWM2M Object SensorDevice -------- */
#include "LWM2MObjectSensorDevice.h"
/* -- LWM2M Object Accelerometer ------- */
#include "LWM2MObjectAccelerometer.h"
/* -- LWM2M Object Accelerometer ------- */
#include "LWM2MObjectLightControl.h"
/* -- LWM2M Object Alert Notification -- */
#include "LWM2MObjectAlertNotification.h"

/**
 * @brief Initializes all the LWM2M Objects for XDK.
 *
 * @param[in] ConNotifies
 * boolean to represent if the notifications is confirmable.
 */
void LWM2MObjects_Init(bool ConNotifies);

#endif /* LWM2MOBJECTS_H_ */

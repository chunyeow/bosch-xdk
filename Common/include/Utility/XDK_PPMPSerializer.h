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
 * @ingroup UTILITY
 *
 * @defgroup PPMPSERIALIZER PPMP Serializer
 * @{
 *
* @brief Interface header file for the PPMP Serializer feature.
 *
 * @details This is responsible for serializing PPMP payload.
 * For serializing this uses 3rd party cJson module
 * which allocates memory from the MCU heap.
 * This is dependent on the XDK_Sensor interface and
 * uses the #Sensor_Value_T naming for technical value.
 *
 * @file
 */

/* header definition ******************************************************** */
#ifndef XDK_PPMPSERIALIZER_H_
#define XDK_PPMPSERIALIZER_H_

/* local interface declaration ********************************************** */
#include "BCDS_Retcode.h"

#include "XDK_Sensor.h"
#include "XDK_ExternalSensor.h"

/* local type and macro definitions */

/**
 * @brief   Structure to represent the PPMP serializer setup features.
 */
struct PPMPSerializer_Setup_S
{
    const char * DeviceName; /**< Pointer to the PPMP device name. Must be lesser than 20 characters. */
};

/**
 * @brief   Typedef to represent the PPMP serializer setup feature.
 */
typedef struct PPMPSerializer_Setup_S PPMPSerializer_Setup_T;

/**
 * @brief   This will setup the PPMP serializer
 *
 * @param[in] setup
 * Pointer to the PPMP serializer setup feature
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - This must be the first API to be called if PPMP serializer feature is to be used.
 * - Do not call this API more than once.
 */
Retcode_T PPMPSerializer_Setup(PPMPSerializer_Setup_T * setup);

/**
 * @brief   This will enable the PPMP serializer
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #PPMPSerializer_Setup must have been successful prior.
 * - Do not call this API more than once.
 */
Retcode_T PPMPSerializer_Enable(void);

/**
 * @brief   This will construct and destruct the PPMP payload for XDK in-built sensors.
 *
 * @param[in] create
 * If "true" the payload will be Constructed by allocating memory from the MCU heap.
 * If "false" then any previously allocated memory will be destructed.
 *
 * @param[in] timezoneISO8601format
 * time zone in ISO-8601 format (string)
 *
 * @param[in] timeStampDelta
 * time stamp in milli seconds offset to timezoneISO8601format value
 *
 * @param[in] sensorValue
 * data collector last valid snap-shot to be serialized
 *
 * @param[in] serializeSensor
 * denotes which sensors needs to be serialized per request
 *
 * @return pointer to the constructed payload. Will be NULL in-case of failure.
 *
 * @note Every #PPMPSerializer_PayloadSensorSelective(true, ...) must follow with with #PPMPSerializer_PayloadSensorSelective(false, ...)
 */
char * PPMPSerializer_PayloadSensorSelective(bool create, char * timezoneISO8601format, uint32_t timeStampDelta, Sensor_Value_T * sensorValue, Sensor_Enable_T * serializeSensor);

/**
 * @brief   This will construct and destruct the PPMP payload for XDK interfaced external sensors.
 *
 * @param[in] create
 * If "true" the payload will be Constructed by allocating memory from the MCU heap.
 * If "false" then any previously allocated memory will be destructed.
 *
 * @param[in] timezoneISO8601format
 * time zone in ISO-8601 format (string)
 *
 * @param[in] timeStampDelta
 * time stamp in milli seconds offset to timezoneISO8601format value
 *
 * @param[in] sensorValue
 * data collector last valid snap-shot to be serialized
 *
 * @param[in] serializeSensor
 * denotes which sensors needs to be serialized per request
 *
 * @return pointer to the constructed payload. Will be NULL in-case of failure.
 *
 * @note Every #PPMPSerializer_PayloadExternalSensorSelective(true, ...) must follow with with #PPMPSerializer_PayloadExternalSensorSelective(false, ...)
 * Only LEM is supported, yet.
 */
char * PPMPSerializer_PayloadExternalSensorSelective(bool create, char * timezoneISO8601format, uint32_t timeStampDelta, ExternalSensor_Value_T * sensorValue, ExternalSensor_Enable_T * serializeSensor);

#endif /* XDK_PPMPSERIALIZER_H_ */
/**@}*/

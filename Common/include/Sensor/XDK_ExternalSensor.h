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
 * @ingroup SENSOR
 *
 * @defgroup EXTERNALSENSOR External sensor
 * @{
 *
 * @brief Interface header file for the External Sensor features.
 *
 * @details This provides the basic sensor sampling feature with the default configuration.
 * This supports only single thread for data sampling and reporting, yet.
 *
 * @file
 */
/* header definition ******************************************************** */
#ifndef XDK_EXTERNALSENSOR_H_
#define XDK_EXTERNALSENSOR_H_

/* local interface declaration ********************************************** */
#include "BCDS_Retcode.h"
#include "BCDS_CmdProcessor.h"

/**
 * @brief   Enum representing the external temperature supported.
 */
enum ExternalSensor_Max31865TempType_E
{
    SENSOR_TEMPERATURE_PT100, /**< PT100 based temperature sensor */
    SENSOR_TEMPERATURE_PT1000, /**< PT1000 based temperature sensor */
    SENSOR_TEMPERATURE_INVALID /**< Invalid Sensor Type */
};

/**
 * @brief   Typedef representing the the external temperature sensor..
 */
typedef enum ExternalSensor_Max31865TempType_E ExternalSensor_Max31865TempType_T;

/**
 * @brief   Structure to represent the Sensors to be enabled.
 */
struct ExternalSensor_Enable_S
{
    bool LemCurrent; /**< Boolean representing if Current sensor is to be enabled or not */
    bool LemVoltage; /**< Boolean representing if Voltage sensor is to be enabled or not */
    bool MaxTemp;/**< Boolean representing if external temperature sensor is to be enabled or not */
    bool MaxResistance;/**< Boolean representing if external resistance is to be enabled or not */
};

/**
 * @brief   Typedef to represent the Sensor to be enabled.
 */
typedef struct ExternalSensor_Enable_S ExternalSensor_Enable_T;

struct ExternalSensor_LemConfig_S
{
    float CurrentRatedTransformationRatio; /**< Current sensor (LEM) rated transformation ratio. Unused if Enable.Current is false. */
};

typedef struct ExternalSensor_LemConfig_S ExternalSensor_LemConfig_T;

struct ExternalSensor_Max31865Config_S
{
    ExternalSensor_Max31865TempType_T TempType;
};

typedef struct ExternalSensor_Max31865Config_S ExternalSensor_Max31865Config_T;

/**
 * @brief   Structure to represent the Sensor setup features.
 */
struct ExternalSensor_Setup_S
{
    CmdProcessor_T * CmdProcessorHandle;
    ExternalSensor_Enable_T Enable; /**< The Sensors to be enabled */
    ExternalSensor_LemConfig_T LemConfig;/**< LEM configuration for setting up current rated transformation ratio  */
    ExternalSensor_Max31865Config_T Max31865Config;/**< MAX31865 configuration for setting up the external temperature sensor type */
};

/**
 * @brief   Typedef to represent the Sensor setup features.
 */
typedef struct ExternalSensor_Setup_S ExternalSensor_Setup_T;

/**
 * @brief Structure to represent the Data Collector sampled node informations.
 */
struct ExternalSensor_Value_S
{
    float XLemI; /**< Current sensor value */
    float XLemV; /**< Voltage sensor value */
    float XMaxT;/**< External temperature sensor value */
    float XMaxR;/**< External resistance value */
};

/**
 * @brief   Enum representing the supported External sensors.
 */
enum ExternalSensor_Target_E
{
    XDK_EXTERNAL_LEM = 0,
    XDK_EXTERNAL_MAX31865,
    XDK_EXTERNAL_INVALID
};

/**
 * @brief   Typedef representing the supported External sensor.
 */
typedef enum ExternalSensor_Target_E ExternalSensor_Target_T;

/**
 * @brief   Typedef to represent the Data Collector sampled node information.
 */
typedef struct ExternalSensor_Value_S ExternalSensor_Value_T;

/**
 * @brief This will setup the External Sensor module.
 *
 * @param[in] setup
 * Pointer to the Sensor setup feature
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - This must be the first API to be called if this Sensor feature is to be used.
 * - Enable required sensor which is connected on external board.
 * - Do not call this API more than once.
 */
Retcode_T ExternalSensor_Setup(ExternalSensor_Setup_T * setup);

/**
 * @brief This will enable the External Sensor module
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - Based on setup->Enable value, it will enable the necessary sensors.
 * - #ExternalSensor_Setup must have been successful prior.
 * - Do not call this API more than once.
 */
Retcode_T ExternalSensor_Enable(void);

/**
 * @brief This will sample the necessary sensor nodes and update the data.
 *
 * @param[in/out] value
 * The sampled sensor node values. User must provide a valid buffer.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #ExternalSensor_Setup and #ExternalSensor_Enable must have been successful prior.
 * - This is a blocking call
 * - #ExternalSensor_GetData are not thread safe, yet.
 */
Retcode_T ExternalSensor_GetData(ExternalSensor_Value_T * value);

Retcode_T ExternalSensor_GetLemData(float * xLemI, float * xLemV);

Retcode_T ExternalSensor_GetMax31865Data(float * xMaxT, float * xMaxR);

Retcode_T ExternalSensor_HwStatusPinInit(void);

Retcode_T ExternalSensor_HwStatusPinDeInit(void);

/**
 * @brief This will validate if the Sensor is available or not
 *
 * @param[in] sensor
 * External sensor to be validated for availability
 *
 * @param[out] status
 * Pointer to the availability of the requested sensor availability status
 *
 * @note
 * More than one sensor availability can be validated in single request
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
Retcode_T ExternalSensor_IsAvailable(ExternalSensor_Target_T sensor, bool * status);

#endif /* XDK_EXTERNALSENSOR_H_ */

/**@} */

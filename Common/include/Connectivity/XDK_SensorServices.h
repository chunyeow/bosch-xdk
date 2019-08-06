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
 * @ingroup CONNECTIVITY
 *
 * @defgroup BLE SensorServices
 * @{
 *
 * @brief The Sensor Services is a set of custom services, that provides a raw byte array payload communication as well as several sensor data services.
 *
 * @details The Sensor Services is a set of custom services, that provides a raw byte array payload communication as well as several sensor data services,
 * see #SensorService_Type_E.
 * @file
 **/

#ifndef XDK_SENSORSERVICES_H
#define XDK_SENSORSERVICES_H

/* Include all headers which are needed by this file. */
#include "BCDS_Basics.h"

/* Put the type and macro definitions here */

/* Put the function declarations here */

/**
 * @brief   Typedef to represent the sensor service types
 */
enum SensorService_Type_E
{
    BLE_SENSOR_ACCELEROMETER = 1, /**< accelerometer sensor service */
    BLE_SENSOR_GYRO = 2, /**< gyro sensor service */
    BLE_SENSOR_LIGHT = 3, /**< light sensor service */
    BLE_SENSOR_NOISE = 4, /**< noise sensor service */
    BLE_SENSOR_MAGNETOMETER = 5, /**< magnetometer sensor service */
    BLE_SENSOR_ENVIRONMENTAL = 6, /**< environmental sensor service */
    BLE_HIGH_DATA_RATE = 7, /**< bidirectional raw data service */
    BLE_CONTROL_NODE = 8, /**< control service */
    BLE_SENSOR_MAX /**< Maximum limit for BLE Data Exchange service */
};

/**
 * @brief   Typedef to represent the sensor service type
 */
typedef enum SensorService_Type_E SensorService_Type_T;

/**
 * @brief   Enumeration to represent the sensor axis types for accelerometer and gyro services
 */
enum SensorServices_SensorAxisType_E
{
    SENSOR_AXIS_X = 0, /**< X axis */
    SENSOR_AXIS_Y = 1, /**< Y axis */
    SENSOR_AXIS_Z = 2, /**< Z axis */
    SENSOR_AXIS_MAX
};

/**
 * @brief   Typedef to represent the sensor axis type for accelerometer and gyro services
 */
typedef enum SensorServices_SensorAxisType_E SensorServices_SensorAxisType_T;

/**
 * @brief   Enumeration to represent the magnetometer service characteristics
 */
enum SensorServices_MagnetometerType_E
{
    SENSOR_MAGNETOMETER_AXIS_X = 0, /**< X axis */
    SENSOR_MAGNETOMETER_AXIS_Y = 1, /**< X axis */
    SENSOR_MAGNETOMETER_AXIS_Z = 2, /**< Z axis */
    SENSOR_MAGNETOMETER_RESISTANCE = 3, /**< resistance */
    SENSOR_MAGNETOMETER_MAX
};

/**
 * @brief   typedef to represent the magnetometer service characteristic
 */
typedef enum SensorServices_MagnetometerType_E SensorServices_MagnetometerType_T;

/**
 * @brief   Enumeration to represent the environmental service characteristics.
 */
enum SensorServices_EnvironmentalType_E
{
    SENSOR_ENVIRONMENTAL_PRESSURE = 0, /**< pressure sensor */
    SENSOR_ENVIRONMENTAL_TEMPERATURE = 1, /**< temperature sensor */
    SENSOR_ENVIRONMENTAL_HUMIDITY = 2, /**< humidity sensor */
};

/**
 * @brief   Typedef to represent the environmental service characteristic.
 */
typedef enum SensorServices_EnvironmentalType_E SensorServices_EnvironmentalType_T;

/**
 * @brief   Enumeration to represent the high data rate service characteristics.
 */
enum SensorServices_HighDataRateType_E
{
    HIGH_DATA_RATE_HIGH_PRIO = 0, /**< high priority array */
    HIGH_DATA_RATE_LOW_PRIO = 1, /**< low priority array */
    HIGH_DATA_RATE_MAX
};

/**
 * @brief   Typedef to represent the high data rate service characteristic.
 */
typedef enum SensorServices_HighDataRateType_E SensorServices_HighDataRateType_T;

/**
 * @brief   Enumeration to control the nodes.
 */
enum SensorServices_SensorControlNode_E
{
    CONTROL_NODE_START_SAMPLING = 0, /**< start sampling */
    CONTROL_NODE_CHANGE_SAMPLING_RATE = 1, /**< change sampling rate */
    CONTROL_NODE_REBOOT = 2, /**< reboot */
    CONTROL_NODE_GET_FW_VERSION = 3, /**< get firmware version */
    CONTROL_NODE_USE_SENSOR_FUSION = 4, /**< use sensor fusion */
    CONTROL_NODE_MAX
};

/**
 * @brief   Typedef to control the node.
 * @details   Typedef to control the node.
 */
typedef enum SensorServices_SensorControlNode_E SensorServices_SensorControlNode_T;

/**
 * @brief   Structure to represent the node sensor service informations.
 *
 * @note
 * The sensorServiceType will represent one of the valid SensorService_Type_T values.
 * Based on this the sensorServiceContent will change.
 *
 * |    sensorServiceType           |   sensorServiceContent                       |
 * |--------------------------------|----------------------------------------------|
 * |    BLE_SENSOR_ACCELEROMETER    |   SensorServices_SensorAxisType_T            |
 * |    BLE_SENSOR_GYRO             |   SensorServices_SensorAxisType_T            |
 * |    BLE_SENSOR_LIGHT            |   unused                                     |
 * |    BLE_SENSOR_NOISE            |   unused                                     |
 * |    BLE_SENSOR_MAGNETOMETER     |   SensorServices_MagnetometerType_T          |
 * |    BLE_SENSOR_ENVIRONMENTAL    |   SensorServices_EnvironmentalType_T         |
 * |    BLE_HIGH_DATA_RATE          |   SensorServices_HighDataRateType_T          |
 * |    BLE_CONTROL_NODE            |   SensorServices_SensorControlNode_T         |
 */
struct SensorServices_Info_S
{
    uint8_t sensorServicesType; /**< Sensor service type */
    uint8_t sensorServicesContent; /**< Sensor service content */
};

/**
 * @brief   Typedef to represent the node sensor service information.
 */
typedef struct SensorServices_Info_S SensorServices_Info_T;

#endif /* XDK_SENSORSERVICES_H */
/**@} */

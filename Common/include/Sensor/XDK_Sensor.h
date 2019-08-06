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
 * @defgroup INBUILDSENSOR  In-built Sensor
 * @{
 *
 * @brief Interface header file for the In-built Sensor features.
 *
 * @details This provides the basic sensor sampling feature with the default configuration.
 * This supports only single thread for data sampling and reporting, yet.
 *
 * @file
 */

/* header definition ******************************************************** */
#ifndef XDK_SENSOR_H_
#define XDK_SENSOR_H_

/* local interface declaration ********************************************** */
#include "BCDS_Retcode.h"
#include "BCDS_CmdProcessor.h"

/**
 * @brief   Structure to represent the Sensors to be enabled.
 */
struct Sensor_Enable_S
{
    bool Accel; /**< Boolean representing if Accelerometer sensor is to be enabled or not */
    bool Mag; /**< Boolean representing if Magnetometer sensor is to be enabled or not */
    bool Gyro; /**< Boolean representing if Gyroscope sensor is to be enabled or not */
    bool Humidity; /**< Boolean representing if Humidity sensor is to be enabled or not */
    bool Temp; /**< Boolean representing if Temperature sensor is to be enabled or not */
    bool Pressure; /**< Boolean representing if Pressure sensor is to be enabled or not */
    bool Light; /**< Boolean representing if Light sensor is to be enabled or not */
    bool Noise; /**< Boolean representing if Noise sensor is to be enabled or not */
};

/**
 * @brief   Typedef to represent the Sensor to be enabled.
 */
typedef struct Sensor_Enable_S Sensor_Enable_T;

/**
 * @brief   Enum representing the supported Accelerometer sensors.
 */
enum Sensor_AccelType_E
{
    SENSOR_ACCEL_BMA280, /**< In-built BMA280 sensor */
    SENSOR_ACCEL_BMI160, /**< In-built BMI160 sensor */
};

/**
 * @brief   Typedef representing the supported Accelerometer sensor.
 */
typedef enum Sensor_AccelType_E Sensor_AccelType_T;

/**
 * @brief Structure representing the accelerometer configurations
 *
 * @note In the accel configuration the interupt enabled is ACCELEROMETER_BMA280_SLOPE_DURATION4 and the interupt is enabled when there is change of slope
 * in y axis.
 * Interupt configurations can be changed in Sensor.c file.
 */
struct Sensor_AccelConfiguration_S
{
    Sensor_AccelType_T Type; /**< Sensor to sample Accelerometer data. Unused if Enable.Accel is false. By default in-built BMA280 sensor is used. */
    bool IsRawData; /**< To check whether Raw Accel data is required, if not converted G value is updated */
    bool IsInteruptEnabled;/**< To check whether interupt is enabled, if not enabled then AccelCallback is not needed */
    CmdProcessor_Func_T Callback;/**< Accelerometer callback when interupt is enabled */
};

/**
 * @brief Typedef representing the accelerometer configurations
 */
typedef struct Sensor_AccelConfiguration_S Sensor_AccelConfiguration_T;

/**
 * @brief   Enum representing the supported Gyroscope sensors.
 */
enum Sensor_GyroType_E
{
    SENSOR_GYRO_BMG160, /**< In-built BMG160 sensor */
    SENSOR_GYRO_BMI160, /**< In-built BMI160 sensor */
};

/**
 * @brief   Typedef representing the supported Gyroscope sensor.
 */
typedef enum Sensor_GyroType_E Sensor_GyroType_T;

/**
 * @brief Structure representing the Gyro configurations
 *
 */
struct Sensor_GyroConfiguration_S
{
    Sensor_GyroType_T Type; /**< Sensor to sample Gyroscope data. Unused if Enable.Gyro is false. By default in-built BMG160 sensor is used. */
    bool IsRawData; /**< To check whether Raw Gyro data is required, if not converted Degree Value is updated */
};

/**
 * @brief Typedef representing the Gyro configurations
 */
typedef struct Sensor_GyroConfiguration_S Sensor_GyroConfiguration_T;

/**
 * @brief Structure representing the Magnetometer configurations
 *
 */
struct Sensor_MagConfiguration_S
{
    bool IsRawData; /**< To check whether Raw Magnetometer data is required, if not micro tesla value is updated */
};

/**
 * @brief Typedef representing the Magnetometer configurations
 */
typedef struct Sensor_MagConfiguration_S Sensor_MagConfiguration_T;

/**
 * @brief   Structure representing the supported light sensor.
 *
 * @note The configuration for light interupt are default values and the upper and lower threshold values are defined in sensor.c.
 * Interupt configurations can be changed in Sensor.c file.
 *
 */
struct Sensor_LightConfiguration_S
{
    bool IsInteruptEnabled;
    CmdProcessor_Func_T Callback;
};

/**
 * @brief   Typedef representing the supported light sensor.
 */
typedef struct Sensor_LightConfiguration_S Sensor_LightConfiguration_T;

/**
 * @brief   Structure representing to configure Temperature sensor.
 *
 * @note The configuration for light interupt are default values and the upper and lower threshold values are defined in sensor.c.
 * Interupt configurations can be changed in Sensor.c file.
 *
 */
struct Sensor_TempConfiguration_S
{
    int32_t OffsetCorrection; /**< Temperature sensor offset correction value (in mDegC). Unused if Enable.Temp is false. */
};

/**
 * @brief   Typedef representing to configure Temperature sensor
 */
typedef struct Sensor_TempConfiguration_S Sensor_TempConfiguration_T;

struct Sensor_Configuration_S
{
    Sensor_AccelConfiguration_T Accel; /**< The accelerometer sensor configurations */
    Sensor_GyroConfiguration_T Gyro; /**< The gyro sensor configurations */
    Sensor_MagConfiguration_T Mag; /**< The magnetometer sensor configurations */
    Sensor_LightConfiguration_T Light;/**< The light sensor configurations */
    Sensor_TempConfiguration_T Temp; /**< The temperature sensor configurations */
};

typedef struct Sensor_Configuration_S Sensor_Configuration_T;

/**
 * @brief   Structure to represent the Sensor setup features.
 */
struct Sensor_Setup_S
{
    CmdProcessor_T * CmdProcessorHandle;
    Sensor_Enable_T Enable; /**< The Sensors to be enabled */
    Sensor_Configuration_T Config;
};

/**
 * @brief   Typedef to represent the Sensor setup features.
 */
typedef struct Sensor_Setup_S Sensor_Setup_T;

/**
 * @brief Structure to represent the Axis value
 */
struct Sensor_AxisValue_S
{
    int32_t X; /**< x-axis value */
    int32_t Y; /**< y-axis value */
    int32_t Z; /**< z-axis value */
};

/**
 * @brief   Typedef to represent the Accelerometer Data
 */
typedef struct Sensor_AxisValue_S Sensor_AccelValue_T;


/**
 * @brief   Typedef to represent the Gyroscope Data
 */
typedef struct Sensor_AxisValue_S Sensor_GyroValue_T;

/**
 * @brief Structure to represent the Magnetometer Data
 */
struct Sensor_MagValue_S
{
    int32_t X; /**< Magnetometer sensor x-axis value */
    int32_t Y; /**< Magnetometer sensor y-axis value */
    int32_t Z; /**< Magnetometer sensor z-axis value */
    int32_t R; /**< Magnetometer sensor resistance value */
};

/**
 * @brief   Typedef to represent the Magnetometer Data
 */
typedef struct Sensor_MagValue_S Sensor_MagValue_T;


/**
 * @brief Structure to represent the Data Collector sampled node informations.
 */
struct Sensor_Value_S
{
    Sensor_AccelValue_T Accel;
    Sensor_MagValue_T Mag;
    Sensor_GyroValue_T Gyro;
    uint32_t RH; /**< Environmental sensor relative humidity value */
    uint32_t Pressure; /**< Environmental sensor relative pressure value */
    double Temp; /**< Environmental sensor relative temperature value */
    uint32_t Light; /**< Light sensor value */
    float Noise; /**< Noise sensor value */
};

/**
 * @brief   Typedef to represent the Data Collector sampled node information.
 */
typedef struct Sensor_Value_S Sensor_Value_T;

/**
 * @brief This will setup the Sensor module
 *
 * @param[in] setup
 * Pointer to the Sensor setup feature
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - This must be the first API to be called if this Sensor feature is to be used.
 * - Do not call this API more than once.
 */
Retcode_T Sensor_Setup(Sensor_Setup_T * setup);

/**
 * @brief This will enable the Sensor module
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - Based on setup->Enable value, it will enable the necessary sensors.
 * - #Sensor_Setup must have been successful prior.
 * - Do not call this API more than once.
 */
Retcode_T Sensor_Enable(void);

/**
 * @brief This will sample the necessary sensor nodes.
 *
 * @param[in/out] value
 * The sampled sensor node values. User must provide a valid buffer.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #Sensor_Setup and #Sensor_Enable must have been successful prior.
 * - This is a blocking call
 * - #Sensor_GetData are not thread safe, yet.
 */
Retcode_T Sensor_GetData(Sensor_Value_T * value);

#endif /* XDK_SENSOR_H_ */
/**@} */

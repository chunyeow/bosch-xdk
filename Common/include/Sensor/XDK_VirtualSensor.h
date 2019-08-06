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
 * @defgroup VIRTUALSENSOR Virtual sensor
 * @{
 *
 * @brief Interface header file for the Virtual Sensor feature.
 *
 * @details This provides the basic sensor sampling feature with the default configuration.
 * This supports only single thread for data sampling and reporting.
 *
 * @file
 */
/* header definition ******************************************************** */
#ifndef VirtualSensor_h_
#define VirtualSensor_h_

/* local interface declaration ********************************************** */
#include "BCDS_Retcode.h"

/**
 * @brief   Structure to represent the Sensors to be enabled.
 */
struct VirtualSensor_Enable_S
{
    uint32_t Rotation :1; /**< Boolean representing if Rotation sensor is to be enabled or not */
    uint32_t Compass :1; /**< Boolean representing if Compass sensor is to be enabled or not */
    uint32_t AbsoluteHumidity :1; /**< Boolean representing if Humidity sensor is to be enabled or not */
    uint32_t CalibratedAccel :1; /**< Boolean representing if CalibratedAccel sensor is to be enabled or not */
    uint32_t CalibratedGyro :1; /**< Boolean representing if CalibratedGyro sensor is to be enabled or not */
    uint32_t CalibratedMag :1; /**< Boolean representing if CalibratedMag sensor is to be enabled or not */
    uint32_t Gravity :1; /**< Boolean representing if Gravity sensor is to be enabled or not */
    uint32_t StepCounter :1; /**< Boolean representing if StepCounter sensor is to be enabled or not */
    uint32_t FingerPrint :1; /**< Boolean representing if FingerPrint sensor is to be enabled or not */
    uint32_t LinearAccel :1; /**< Boolean representing if LinearAccel sensor is to be enabled or not */
    uint32_t Gesture :1; /**< Boolean representing if Gesture sensor is to be enabled or not */
};

/**
 * @brief   Typedef to represent the Virtual Sensor to be enabled.
 */
typedef struct VirtualSensor_Enable_S VirtualSensor_Enable_T;

/**
 * @brief   Structure to represent the Virtual Sensor setup params.
 */
struct VirtualSensor_Setup_S
{
    VirtualSensor_Enable_T Enable; /**< The Virtual Sensor to be enabled */
};

/**
 * @brief   Typedef to represent the Virtual Sensor setup params.
 */
typedef struct VirtualSensor_Setup_S VirtualSensor_Setup_T;

/**
 * @brief   Enum to represent the Rotation Sensor Mode
 */
enum VirtualSensor_RotationMode_E
{
    ROTATION_EULER_DEG = 0, /**< Rotation Euler Value in Degrees */
    ROTATION_EULER_RAD, /**< Rotation Euler Value in Radians */
    ROTATION_QUATERNION /**< Rotation Quaternion values */
};

/**
 * @brief   Typedef to represent the Rotation Sensor Mode
 */
typedef enum VirtualSensor_RotationMode_E VirtualSensor_RotationMode_T;

/**
 * @brief   Enum values to represent the different FingerPrint reference values
 */
enum VirtualSensor_FingerPrintNumber_E
{
    FINGERPRINT_REFVAL_1, /**< Specifies FingerPrint 1 to be recorded */
    FINGERPRINT_REFVAL_2, /**< Specifies FingerPrint 2 to be recorded */
    FINGERPRINT_REFVAL_3, /**< Specifies FingerPrint 3 to be recorded */
    FINGERPRINT_REFVAL_MAX
};

/**
 * @brief   Typedef to represent the different FingerPrint reference values
 */
typedef enum VirtualSensor_FingerPrintNumber_E VirtualSensor_FingerPrintNumber_T;

/**
 * @brief   Enum values that represent finger print storage state
 */
enum VirtualSensor_FingerPrintStorageState_E
{
    FINGERPRINT_STORAGE_EMPTY, /**< FingerPrint empty */
    FINGERPRINT_STORAGE_RECORDED /**< FingerPrint recorded */
};
typedef enum VirtualSensor_FingerPrintStorageState_E VirtualSensor_FingerPrintStorageState_T;

/**
 * @brief   Structure to represent the Integer value of Various Axis
 */
struct VirtualSensor_AxisIntData_S
{
    int32_t X;
    int32_t Y;
    int32_t Z;
};

/**
 * @brief   Typedef to represent the Integer value of Various Axis
 */
typedef struct VirtualSensor_AxisIntData_S VirtualSensor_AxisIntData_T;

/**
 * @brief   Structure to represent the Float value of Various Axis
 */
struct VirtualSensor_AxisFloatData_S
{
    float X;
    float Y;
    float Z;
};

/**
 * @brief   Typedef to represent the Float value of Various Axis
 */
typedef struct VirtualSensor_AxisFloatData_S VirtualSensor_AxisFloatData_T;

/**
 * @brief   Structure to represent the Rotation Quaternion data
 */
struct VirtualSensor_RotationQuaternion_S
{
    float W;
    float X;
    float Y;
    float Z;
};

/**
 * @brief   Typedef to represent the Rotation Quaternion data
 */
typedef struct VirtualSensor_RotationQuaternion_S VirtualSensor_RotationQuaternion_T;

/**
 * @brief   Structure to represent the Rotation Euler data
 */
struct VirtualSensor_RotationEuler_S
{
    float Heading;
    float Pitch;
    float Roll;
};

/**
 * @brief   Typedef to represent the Rotation Euler data
 */
typedef struct VirtualSensor_RotationEuler_S VirtualSensor_RotationEuler_T;

/** Data structure for FingerPrint monitoring output data  */
struct VirtualSensor_FingerPrintMonitorData_S
{
    float DistanceMatching; /**< holds FingerPrint distance monitoring output When matchingDistance = 0: measurement is 100% not matching FingerPrint reference value */
    float AngleMatching; /**< holds FingerPrint angle monitoring output. When matchinAngle < 1: measurement is not matching with FingerPrint reference value */
};
typedef struct VirtualSensor_FingerPrintMonitorData_S VirtualSensor_FingerPrintMonitorData_T;

/**
 * @brief   Structure to represent the various virtual sensor data
 */
union VirtualSensor_DataType_U
{
    VirtualSensor_AxisIntData_T AxisInteger;
    VirtualSensor_AxisFloatData_T AxisFloat;
    VirtualSensor_RotationQuaternion_T RotationQuaternion;
    VirtualSensor_RotationEuler_T RotationEuler;
    VirtualSensor_FingerPrintMonitorData_T FingerPrintMonitorData;
};

/**
 * @brief   Typedef to represent the various virtual sensor data
 */
typedef union VirtualSensor_DataType_U VirtualSensor_DataType_T;

/**
 * @brief   Enum to represent the Accelerometer Mode
 */
enum VirtualSensor_AccelMode_E
{
    ACCEL_LSB_MODE, /**< Accel Data in lsb */
    ACCEL_MPS2_MODE, /**< Accel Data in metre per second squared */
    ACCEL_G_MODE /**< Accel Data in gravity */
};

/**
 * @brief   Typedef to represent the Accelerometer Sensor Mode
 */
typedef enum VirtualSensor_AccelMode_E VirtualSensor_AccelMode_T;

/**
 * @brief   Enum to represent the Gravity Sensor Mode
 */
enum VirtualSensor_GravityMode_E
{
    GRAVITY_LSB_MODE, /**< Gravity Data in lsb */
    GRAVITY_MPS2_MODE, /**< Gravity Data in metre per second squared */
    GRAVITY_G_MODE /**< Gravity Data in gravity */
};

/**
 * @brief   Typedef to represent the Gravity Sensor Mode
 */
typedef enum VirtualSensor_GravityMode_E VirtualSensor_GravityMode_T;

/**
 * @brief   Enum to represent the Gyro Sensor Mode
 */
enum VirtualSensor_GyroMode_E
{
    GYRO_LSB_MODE, /**< Gyro data in lsb */
    GYRO_RPS_MODE, /**< Gyro data in radians per second */
    GYRO_DPS_MODE /**< Gyro data in degrees per second */
};

/**
 * @brief   Typedef to represent the Gyro Sensor Mode
 */
typedef enum VirtualSensor_GyroMode_E VirtualSensor_GyroMode_T;

/**
 * @brief   Enum to represent the Magnetometer Sensor Mode
 */
enum VirtualSensor_MagMode_E
{
    MAG_LSB_MODE, /**< Mag data in lsb */
    MAG_GAUSS_MODE, /**< Mag data in Gauss */
    MAG_MICROTESLA_MODE /**< Mag data in Micro tesla */
};

/**
 * @brief   Typedef to represent the Magnetometer Sensor Mode
 */
typedef enum VirtualSensor_MagMode_E VirtualSensor_MagMode_T;

/**
 * @brief This will setup the Virtual Sensor module
 *
 * @param[in] setup
 * Pointer to the Virtual Sensor setup feature
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - This must be the first API to be called if this Virtual Sensor feature is to be used.
 * - Only one sensor should be enabled. Do not call this API more than once.
 */
Retcode_T VirtualSensor_Setup(VirtualSensor_Setup_T * setup);

/**
 * @brief This will close the Virtual Sensor module
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_Close(void);

/**
 * @brief This will enable the Sensor Virtual module
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_Enable(void);

/**
 * @brief This will disable the Sensor Virtual module
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_Disable(void);

/**
 * @brief Get the rotation value from the rotation sensor
 *
 * @ param[out] data
 *  rotation sensor data based on given mode
 *
 * @ param[in] mode
 *  mode of the rotation sensor
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_GetRotationData(VirtualSensor_DataType_T *data, VirtualSensor_RotationMode_T mode);

/**
 * @brief Get the Heading angle from the compass sensor
 *
 * @ param[out] compass
 *  Heading angle data
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_GetCompassData(float *compass);

/**
 * @brief Get the absolute humidity value from the humidity sensor
 *
 * @ param[out] humidity
 *  absolute humidity value
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_GetAbsoluteHumidityData(float *humidity);

/**
 * @brief Get the step count value (Provides the number of user steps)
 *
 * @ param[out] stepCounter
 *  value of Step count
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_GetStepCounter(int16_t *stepCounter);

/**
 * @brief Get the gestures value (Provides the gestures detected)
 *
 * @ param[out] gestureCount
 *  value of Gestures
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_GetGestureCount(uint32_t *gestureCount);

/**
 * @brief Get the xyz values from linear acceleration for given accel mode
 *
 * @ param[out] data
 *  linear acceleration value
 *
 * @ param[in] mode
 *  linear acceleration mode to read data
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_GetLinearAccel(VirtualSensor_DataType_T *data, VirtualSensor_AccelMode_T mode);

/**
 * @brief Get the xyz values from Gravity sensor for given mode
 *
 * @ param[out] data
 *  gravity value
 *
 * @ param[in] mode
 *  gravity mode to read data
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_GetGravity(VirtualSensor_DataType_T *data, VirtualSensor_GravityMode_T mode);

/**
 * @brief Get the xyz values from calibrated acceleration for given accel mode
 *
 * @ param[out] data
 *  calibrated acceleration value
 *
 * @ param[in] mode
 *  calibrated acceleration mode to read data
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_GetCalibratedAccel(VirtualSensor_DataType_T *data, VirtualSensor_AccelMode_T mode);

/**
 * @brief Get the xyz values from calibrated gyro for given accel mode
 *
 * @ param[out] data
 *  calibrated gyro value
 *
 * @ param[in] mode
 *  calibrated gyro mode to read data
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_GetCalibratedGyro(VirtualSensor_DataType_T *data, VirtualSensor_GyroMode_T mode);

/**
 * @brief Get the xyz values from calibrated Magnetometer for given accel mode
 *
 * @ param[out] data
 *  magnetometer gyro value
 *
 * @ param[in] mode
 *  calibrated magnetometer mode to read data
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_GetCalibratedMag(VirtualSensor_DataType_T *data, VirtualSensor_MagMode_T mode);

/**
 * @brief Sets the reference value for the selected FingerPrint
 *
 * @ param[in] num
 * reference value that is set for the finger print sensor
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_SetFingerPrintValue(VirtualSensor_FingerPrintNumber_T num);

/**
 * @brief Clears the reference value for the selected FingerPrint.
 *
 * @ param[in] num
 * reference value that is used to clear finger print
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_ResetFingerPrintValue(VirtualSensor_FingerPrintNumber_T num);

/**
 * @brief Get current FingerPrint reference values from FingerPrint sensor.
 *
 * @ param[in] num
 * num for which finger print stored value has to be checked
 *
 * @ param[out] status
 * finger print value for given value
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_CheckFingerPrintStoredValue(VirtualSensor_FingerPrintNumber_T num, VirtualSensor_FingerPrintStorageState_T *status);

/**
 * @brief Monitors whether the measured data matches with stored FingerPrint values from FingerPrint sensor.
 *
 * @ param[out] result
 * Finger PrintMonitor Data
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T VirtualSensor_MonitorFingerPrint(VirtualSensor_DataType_T *result);

#endif /* VirtualSensor_h_ */

/**@}*/

/*----------------------------------------------------------------------------*/
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
/*
 * Copyright (C) Bosch Connected Devices and Solutions GmbH.
 * All Rights Reserved. Confidential.
 *
 * Distribution only to people who need to know this information in
 * order to do their job.(Need-to-know principle).
 * Distribution to persons outside the company, only if these persons
 * signed a non-disclosure agreement.
 * Electronic transmission, e.g. via electronic mail, must be made in
 * encrypted form.
 */
/*----------------------------------------------------------------------------*/

/**  @file
 *
 * Handles for the sensors will be returned through this module
 * 
 * ****************************************************************************/

/* module includes ********************************************************** */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_COMMON_ID_SENSORHANDLE

#include "XDK_SensorHandle.h"

/* additional interface header files */
#include "BCDS_Basics.h"
#include "BCDS_VirtualSensorHandle.h"
#include "sensorsHwConfig.h"
#include "BCDS_AxisRemap.h"
#include "BCDS_Bma280Utils.h"
#include "BCDS_Bme280Utils.h"
#include "BCDS_Bmi160Utils.h"
#include "BCDS_Bmg160Utils.h"
#include "BCDS_Bmm150Utils.h"
#include "BCDS_Max44009Utils.h"
#include "BSP_BoardType.h"

/* constant definitions ***************************************************** */

#define SENSOR_API_UNINITIALIZED UINT8_C(0)
#define SENSOR_API_INITIALIZED   UINT8_C(1)
#define BMA280_DEV_ADDR          UINT8_C(0x18)
#define BMG160_DEV_ADDR          UINT8_C(0x68)
#define BME280_DEV_ADDR          UINT8_C(0x76)
#define BMM150_DEV_ADDR          UINT8_C(0x10)
#define MAX44009_DEV_ADDR        UINT8_C(0x4A)
#define BMI160_DEV_ADDR          UINT8_C(0x69)


static Bma280Utils_Info_T bma280Sensor_S =
        {
                .BspSensorId = (uint32_t) BSP_XDK_SENSOR_BMA280,
                .remap =
                        {
                                .axisConfig = RMP_BMA2X2_X_AXIS_CONFIG | RMP_BMA2X2_Y_AXIS_CONFIG | RMP_BMA2X2_Z_AXIS_CONFIG,
                                .axisSign = RMP_BMA2X2_X_AXIS_SIGN_CONFIG | RMP_BMA2X2_Y_AXIS_SIGN_CONFIG | RMP_BMA2X2_Z_AXIS_SIGN_CONFIG,
                        },
                .ISRCallback = NULL,
                .dev_addr = BMA280_DEV_ADDR,
        };

static Accelerometer_Handle_T xdkAccelerometers_BMA280_S =
        {
                .SensorInformation.sensorID = ACCELEROMETER_BMA280,
                .SensorInformation.initializationStatus = SENSOR_API_UNINITIALIZED,
                .SensorPtr = &bma280Sensor_S,
                .IntChannel1.deferredCallback = NULL,
                .IntChannel1.rtCallback = NULL,
                .IntChannel2.deferredCallback = NULL,
                .IntChannel2.rtCallback = NULL,
        };

Accelerometer_HandlePtr_T XdkAccelerometer_BMA280_Handle = &xdkAccelerometers_BMA280_S;

/************************************************BMG Handle Info*********************************************************************/
static Bmg160Utils_Info_T bmg160Sensor_S =
        {

        .BspSensorId = (uint32_t) BSP_XDK_SENSOR_BMG160,
                .remap =
                        {
                                .axisConfig = RMP_BMG160_X_AXIS_CONFIG | RMP_BMG160_Y_AXIS_CONFIG | RMP_BMG160_Z_AXIS_CONFIG,
                                .axisSign = RMP_BMG160_X_AXIS_SIGN_CONFIG | RMP_BMG160_Y_AXIS_SIGN_CONFIG | RMP_BMG160_Z_AXIS_SIGN_CONFIG,
                        },
                .ISRCallback = NULL,
                .dev_addr = BMG160_DEV_ADDR,
        };

static Gyroscope_Handle_T xdkGyroscope_BMG160_S = {
        .sensorInfo.sensorID = BMG160_GYRO_SENSOR,
        .sensorInfo.initializationStatus = SENSOR_API_UNINITIALIZED,
        .sensorPtr = &bmg160Sensor_S,
};
Gyroscope_HandlePtr_T XdkGyroscope_BMG160_Handle = &xdkGyroscope_BMG160_S;

/************************************************BME Handle Info*********************************************************************/
static Bme280Utils_Info_T bme280Sensor_S =
        {
                .BspSensorId = (uint32_t) BSP_XDK_SENSOR_BME280,
                .dev_addr = BME280_DEV_ADDR,
        };

static Environmental_Handle_T xdkEnvironmental_BME280_S = {
        .SensorInformation.sensorID = ENVIRONMENTAL_BME280,
        .SensorInformation.initializationStatus = SENSOR_API_UNINITIALIZED,
        .SensorPtr = &bme280Sensor_S,

};
Environmental_HandlePtr_T XdkEnvironmental_BME280_Handle = &xdkEnvironmental_BME280_S;
/************************************************BMM Handle Info*********************************************************************/
static Bmm150Utils_Info_T bmm150Sensor_S =
        {
                .BspSensorId = (uint32_t) BSP_XDK_SENSOR_BMM150,
                .remap =
                        {
                                .axisConfig = RMP_BMM150_X_AXIS_CONFIG | RMP_BMM150_Y_AXIS_CONFIG | RMP_BMM150_Z_AXIS_CONFIG,
                                .axisSign = RMP_BMM150_X_AXIS_SIGN_CONFIG | RMP_BMM150_Y_AXIS_SIGN_CONFIG | RMP_BMM150_Z_AXIS_SIGN_CONFIG,
                        },
                .ISRCallback = NULL,
                .dev_addr = BMM150_DEV_ADDR,
        };
static Magnetometer_Handle_T xdkMagnetometer_BMM150_S = {
        .SensorInformation.sensorID = MAGNETOMETER_BMM150,
        .SensorInformation.initializationStatus = SENSOR_API_UNINITIALIZED,
        .SensorPtr = &bmm150Sensor_S,

};
Magnetometer_HandlePtr_T XdkMagnetometer_BMM150_Handle = &xdkMagnetometer_BMM150_S;

/************************************************MAX Handle Info*********************************************************************/
static Max44009Utils_Info_T max44009Sensor_S =
        {
                .BspSensorId = (uint32_t) BSP_XDK_SENSOR_MAX44009,
                .ISRCallback = NULL,
                .dev_addr = MAX44009_DEV_ADDR,

        };

static LightSensor_Handle_T LightSensor_Handle_S = {
        .SensorInformation.sensorID = LIGHTSENSOR_MAX44009,
        .SensorInformation.initializationStatus = SENSOR_API_UNINITIALIZED,
        .SensorPtr = &max44009Sensor_S,
};
LightSensor_HandlePtr_T XdkLightSensor_MAX44009_Handle = &LightSensor_Handle_S;

/************************************************BMI Handle Info*********************************************************************/
static Bmi160Utils_Info_T bmi160AccelSensor_S =
        {
                .BspSensorId = (uint32_t) BSP_XDK_SENSOR_BMI160,
                .remap =
                        {
                                .axisConfig = RMP_BMIACCEL_X_AXIS_CONFIG | RMP_BMIACCEL_Y_AXIS_CONFIG | RMP_BMIACCEL_Z_AXIS_CONFIG,
                                .axisSign = RMP_BMIACCEL_X_AXIS_SIGN_CONFIG | RMP_BMIACCEL_Y_AXIS_SIGN_CONFIG | RMP_BMIACCEL_Z_AXIS_SIGN_CONFIG,
                        },
                .ISRCallback = NULL,
                .dev_addr = BMI160_DEV_ADDR,

        };

static Accelerometer_Handle_T xdkAccelerometers_BMI160_S = {

.SensorPtr = &bmi160AccelSensor_S,
        .IntChannel1.deferredCallback = NULL,
        .IntChannel1.rtCallback = NULL,
        .IntChannel2.deferredCallback = NULL,
        .IntChannel2.rtCallback = NULL,
        .SensorInformation.initializationStatus = SENSOR_API_UNINITIALIZED,
        .SensorInformation.sensorID = ACCELEROMETER_BMI160,
};
Accelerometer_HandlePtr_T XdkAccelerometer_BMI160_Handle = &xdkAccelerometers_BMI160_S;

static Bmi160Utils_Info_T bmi160GyroSensor_S =
        {
                .BspSensorId = (uint32_t) BSP_XDK_SENSOR_BMI160,
                .remap =
                        {
                                .axisConfig = RMP_BMIGYRO_X_AXIS_CONFIG | RMP_BMIGYRO_Y_AXIS_CONFIG | RMP_BMIGYRO_Z_AXIS_CONFIG,
                                .axisSign = RMP_BMIGYRO_X_AXIS_SIGN_CONFIG | RMP_BMIGYRO_Y_AXIS_SIGN_CONFIG | RMP_BMIGYRO_Z_AXIS_SIGN_CONFIG,
                        },
                .ISRCallback = NULL,
                .dev_addr = BMI160_DEV_ADDR,
        };

static Gyroscope_Handle_T xdkGyroscope_BMI160_S = {
        .sensorPtr = &bmi160GyroSensor_S,
        .sensorInfo =
                {
                        .sensorID = BMI160_GYRO_SENSOR,
                        .initializationStatus = SENSOR_API_UNINITIALIZED,
                },
};

Gyroscope_HandlePtr_T XdkGyroscope_BMI160_Handle = &xdkGyroscope_BMI160_S;

static VirtualSensor_bsxBaseSensor_T xdkBsxBaseSensors_S =
{
    .accelerometerHandle = (Accelerometer_HandlePtr_T) & xdkAccelerometers_BMI160_S,
    .magnetometerHandle = (Magnetometer_HandlePtr_T) & xdkMagnetometer_BMM150_S,
    .gyroscopeHandle = (Gyroscope_HandlePtr_T) & xdkGyroscope_BMI160_S,
};

static VirtualSensor_FPBaseSensor_T xdkFingerprintBaseSensor_S =
{
    .magnetometerHandle = (Magnetometer_HandlePtr_T) & xdkMagnetometer_BMM150_S,
    .accelerometerHandle = NULL,
};

/************************************************END of Handle Info******************************************************************/
AbsoluteHumidity_HandlePtr_T XdkAbsoluteHumidity_ClbrSensor_Handle = (AbsoluteHumidity_HandlePtr_T) & xdkEnvironmental_BME280_S;
CalibratedAccel_HandlePtr_T XdkAccelerometer_ClbrSensor_Handle = (CalibratedAccel_HandlePtr_T) & xdkBsxBaseSensors_S;
Orientation_HandlePtr_T XdkOrientation_VirtualSensor_Handle = (Orientation_HandlePtr_T) & xdkBsxBaseSensors_S;
Rotation_HandlePtr_T XdkRotation_VirtualSensor_Handle = (Rotation_HandlePtr_T) & xdkBsxBaseSensors_S;
Gravity_HandlePtr_T XdkGravity_VirtualSensor_Handle = (Gravity_HandlePtr_T) & xdkBsxBaseSensors_S;
FingerPrint_HandlePtr_T XdkFingerprint_VirtualSensor_Handle = (FingerPrint_HandlePtr_T) & xdkFingerprintBaseSensor_S;
CalibratedGyro_HandlePtr_T XdkGyroscope_ClbrSensor_Handle = (CalibratedGyro_HandlePtr_T) & xdkBsxBaseSensors_S;
CalibratedMag_HandlePtr_T XdkMagnetometer_ClbrSensor_Handle = (CalibratedMag_HandlePtr_T) & xdkBsxBaseSensors_S;
StepCounter_HandlePtr_T XdkStepCounter_VirtualSensor_Handle = (StepCounter_HandlePtr_T) & xdkAccelerometers_BMI160_S;
LinearAcceleration_HandlePtr_T XdkLinearAcceleration_ClbrSensor_Handle = (LinearAcceleration_HandlePtr_T) & xdkBsxBaseSensors_S;
Gestures_HandlePtr_T XdkGesture_VirtualSensor_Handle = (Gestures_HandlePtr_T) & xdkBsxBaseSensors_S;


/* inline functions ********************************************************* */

/* local functions ********************************************************** */

/* global functions ********************************************************* */

/** ************************************************************************* */

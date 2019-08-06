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
 * @ingroup APPS_LIST
 *
 * @defgroup SensorsHandles Sensor Legacy Handles
 * @{
 * @brief Handles used for the Legacy Advanced Sensor APIs in example applications
 *
 * @details
 *
 * @file
 * ****************************************************************************/

#ifndef XDKSENSORHANDLE_H_
#define XDKSENSORHANDLE_H_

#include "XDK_SensorHandle.h"

/* Accelerometer Sensor Handlers */
/**
 * @brief Legacy Sensor Handler for the BMA280 Sensor
 */
#define xdkAccelerometers_BMA280_Handle XdkAccelerometer_BMA280_Handle

/**
 * @brief Legacy Sensor Handler for the BMI160 Accelerometer Sensor
 */
#define xdkAccelerometers_BMI160_Handle XdkAccelerometer_BMI160_Handle

/* Gyroscope Sensor Handler */
/**
 * @brief Legacy Sensor Handler for the BMG160 Sensor
 */
#define xdkGyroscope_BMG160_Handle XdkGyroscope_BMG160_Handle

/**
 * @brief Legacy Sensor Handler for the BMI160 Gyroscope Sensor
 */
#define xdkGyroscope_BMI160_Handle XdkGyroscope_BMI160_Handle

/* Light Sensor Handler */
/**
 * @brief Legacy Sensor Handler for the MAX44009 Sensor
 */
#define xdkLightSensor_MAX44009_Handle XdkLightSensor_MAX44009_Handle

/* Magnetometer Handler */
/**
 * @brief Legacy Sensor Handler for the BMM150 Sensor
 */
#define xdkMagnetometer_BMM150_Handle XdkMagnetometer_BMM150_Handle

/* Environmental sensor Handler */
/**
 * @brief Legacy Sensor Handler for the BME280 Sensor
 */
#define xdkEnvironmental_BME280_Handle XdkEnvironmental_BME280_Handle

/* Orientation Sensor Handler */
/**
 * @brief Legacy Sensor Handler for the Orientation Sensor
 */
#define xdkOrientationSensor_Handle XdkOrientation_VirtualSensor_Handle

/* Rotation Sensor Handler */
/**
 * @brief Legacy Sensor Handler for the Rotation Sensor
 */
#define xdkRotationSensor_Handle XdkRotation_VirtualSensor_Handle

/* Gravity Sensor Handler */
/**
 * @brief Legacy Sensor Handler for the Gravity Sensor
 */
#define xdkGravitySensor_Handle XdkGravity_VirtualSensor_Handle

/* Humidity Sensor Handler */
/**
 * @brief Legacy Sensor Handler for the humidity Sensor
 */
#define xdkHumiditySensor_Handle XdkAbsoluteHumidity_ClbrSensor_Handle

/* Calibrated Accelerometer Handler */
/**
 * @brief Legacy Sensor Handler for Calibrated accelerometer
 */
#define xdkCalibratedAccelerometer_Handle XdkAccelerometer_ClbrSensor_Handle

/* Calibrated Gyroscope Handler */
/**
 * @brief Sensor Handler for Calibrated gyroscope
 */
#define xdkCalibratedGyroscope_Handle XdkGyroscope_ClbrSensor_Handle

/* Calibrated Magnetometer Handler */
/**
 * @brief Legacy Sensor Handler for Calibrated magnetometer
 */
#define xdkCalibratedMagnetometer_Handle XdkMagnetometer_ClbrSensor_Handle

/* Step Counter Sensor Handler */
/**
 * @brief Legacy Sensor Handler for the Step Counter Sensor
 */
#define xdkStepCounterSensor_Handle XdkStepCounter_VirtualSensor_Handle

/* Fingerprint Sensor Handler */
/**
 * @brief Legacy Sensor Handler for Fingerprint Sensor
 */
#define xdkFingerprintSensor_Handle XdkFingerprint_VirtualSensor_Handle

/* Linear Acceleration Sensor Handler */
/**
 * @brief Legacy Sensor Handler for Linear Acceleration Sensor
 */
#define xdkLinearAccelSensor_Handle XdkLinearAcceleration_ClbrSensor_Handle

/* Gesture Sensor Handler */
/**
 * @brief Legacy Sensor Handler for Gesture Sensor
 */
#define xdkGestureSensor_Handle XdkGesture_VirtualSensor_Handle

#endif /* XDKSENSORHANDLE_H_ */
/**@}*/

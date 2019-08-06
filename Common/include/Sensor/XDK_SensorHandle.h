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
 * @defgroup SensorsHandles Sensor Handles
 * @{
 * @brief Handles used for the Advanced Sensor APIs in example applications
 *
 * @details
 *
 * @file
 * ****************************************************************************/

#ifndef XDK_SENSORHANDLE_H_
#define XDK_SENSORHANDLE_H_

#include "BCDS_Accelerometer.h"
#include "BCDS_Gyroscope.h"
#include "BCDS_LightSensor.h"
#include "BCDS_Magnetometer.h"
#include "BCDS_Environmental.h"
#include "BCDS_Orientation.h"
#include "BCDS_Rotation.h"
#include "BCDS_Gravity.h"
#include "BCDS_AbsoluteHumidity.h"
#include "BCDS_StepCounter.h"
#include "BCDS_CalibratedAccel.h"
#include "BCDS_CalibratedGyro.h"
#include "BCDS_CalibratedMag.h"
#include "BCDS_FingerPrint.h"
#include "BCDS_LinearAcceleration.h"
#include "BCDS_Gestures.h"

/* Accelerometer Sensor Handlers */
/**
* @brief Sensor Handler for the BMA280 Sensor
*/
extern Accelerometer_HandlePtr_T XdkAccelerometer_BMA280_Handle;

/**
* @brief Sensor Handler for the BMI160 Accelerometer Sensor
*/
extern Accelerometer_HandlePtr_T XdkAccelerometer_BMI160_Handle;

/* Gyroscope Sensor Handler */
/**
* @brief Sensor Handler for the BMG160 Sensor
*/
extern Gyroscope_HandlePtr_T XdkGyroscope_BMG160_Handle;

/**
* @brief Sensor Handler for the BMI160 Gyroscope Sensor
*/
extern Gyroscope_HandlePtr_T XdkGyroscope_BMI160_Handle;

/* Light Sensor Handler */
/**
* @brief Sensor Handler for the MAX44009 Sensor
*/
extern LightSensor_HandlePtr_T XdkLightSensor_MAX44009_Handle;

/* Magnetometer Handler */
/**
* @brief Sensor Handler for the BMM150 Sensor
*/
extern Magnetometer_HandlePtr_T XdkMagnetometer_BMM150_Handle;

/* Environmental sensor Handler */
/**
* @brief Sensor Handler for the BME280 Sensor
*/
extern Environmental_HandlePtr_T XdkEnvironmental_BME280_Handle;

/* Orientation Sensor Handler */
/**
* @brief Sensor Handler for the Orientation Sensor
*/
extern Orientation_HandlePtr_T XdkOrientation_VirtualSensor_Handle;

/* Rotation Sensor Handler */
/**
* @brief Sensor Handler for the Rotation Sensor
*/
extern Rotation_HandlePtr_T XdkRotation_VirtualSensor_Handle;

/* Gravity Sensor Handler */
/**
* @brief Sensor Handler for the Gravity Sensor
*/
extern Gravity_HandlePtr_T XdkGravity_VirtualSensor_Handle;

/* Humidity Sensor Handler */
/**
* @brief Sensor Handler for the humidity Sensor
*/
extern AbsoluteHumidity_HandlePtr_T XdkAbsoluteHumidity_ClbrSensor_Handle;

/* Calibrated Accelerometer Handler */
/**
 * @brief Sensor Handler for Calibrated accelerometer
 */
extern CalibratedAccel_HandlePtr_T XdkAccelerometer_ClbrSensor_Handle;

/* Calibrated Gyroscope Handler */
/**
 * @brief Sensor Handler for Calibrated gyroscope
 */
extern CalibratedGyro_HandlePtr_T XdkGyroscope_ClbrSensor_Handle;

/* Calibrated Magnetometer Handler */
/**
 * @brief Sensor Handler for Calibrated magnetometer
 */
extern CalibratedMag_HandlePtr_T XdkMagnetometer_ClbrSensor_Handle;

/* Step Counter Sensor Handler */
/**
* @brief Sensor Handler for the Step Counter Sensor
*/
extern StepCounter_HandlePtr_T XdkStepCounter_VirtualSensor_Handle;

/* Fingerprint Sensor Handler */
/**
* @brief Sensor Handler for Fingerprint Sensor
*/
extern FingerPrint_HandlePtr_T XdkFingerprint_VirtualSensor_Handle;

/* Linear Acceleration Sensor Handler */
/**
* @brief Sensor Handler for Linear Acceleration Sensor
*/
extern LinearAcceleration_HandlePtr_T XdkLinearAcceleration_ClbrSensor_Handle;

/* Gesture Sensor Handler */
/**
* @brief Sensor Handler for Gesture Sensor
*/
extern Gestures_HandlePtr_T XdkGesture_VirtualSensor_Handle;

#endif /* XDK_SENSORHANDLE_H_ */
/**@}*/

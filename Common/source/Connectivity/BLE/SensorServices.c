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

#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_SENSORSERVICES

#include "BCDS_BlePeripheral.h"
#include "BCDS_Basics.h"
#include "BCDS_Retcode.h"
#include "BCDS_BluetoothConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "SensorServices.h"
#include "BleTypes.h"
/*lint -e652 -e830 */
#include "attserver.h"
/*lint +e652 +e830 */

/* Put the type and macro definitions here */

#ifndef BLE_OS_DELAY_UPON_CONNECT
#define BLE_OS_DELAY_UPON_CONNECT       (UINT32_C(250))
#endif /* BLE_OS_DELAY_UPON_CONNECT */

#define SENSOR_SERVICES_UUID_MAX                        16U
#define SENSOR_SERVICES_HIGH_DATA_RATE_ATT_LEN_MAX      20U

/** Connection handle to client */
static uint16_t ConnectionHandle;
static bool IsFirstSendPending = false;
static TickType_t BleLastWakeTimeUponConnect = 0UL;
/**
 * @brief structure to hold the Characteristic properties of the Sensor Services
 */
struct SensorServices_Characteristic_S
{
    uint8_t CharacteristicUUID[SENSOR_SERVICES_UUID_MAX];
    AttUuid UUIDType;
    Att128BitCharacteristicAttribute Characteristic;
    AttAttribute CharacteristicAttribute;
};

/**
 * @brief Typedef to represent the Characteristic property of Sensor Services
 */
typedef struct SensorServices_Characteristic_S SensorServices_Characteristic_T;

/** Accelerometer Sensor Service UUID: 5a 21 1d 40 71 66 11 e4 82 f8 08 00 20 0c 9a 66 */
const uint8_t AccelerometerSensorServiceUUID[SENSOR_SERVICES_UUID_MAX] = { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x66, 0x71, 0x40, 0x1D, 0x21, 0x5A };

/** Gyro Sensor Service Sensor UUID: ac a9 6a 40 74 a4 11 e4 82 f8 08 00 20 0c 9a 66 */
const uint8_t GyroSensorServiceUUID[SENSOR_SERVICES_UUID_MAX] = { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0xA4, 0x74, 0x40, 0x6A, 0xA9, 0xAC };

/** Light Sensor Service UUID: 38eb02c0-7540-11e4-82f8-0800200c9a66*/
const uint8_t LightSensorServiceUUID[SENSOR_SERVICES_UUID_MAX] = { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x40, 0x75, 0xC0, 0x02, 0xEB, 0x38 };

/** Noise Sensor Service UUID: 01033830-754c-11e4-82f8-0800200c9a66*/
const uint8_t NoiseSensorServiceUUID[SENSOR_SERVICES_UUID_MAX] = { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x4C, 0x75, 0x30, 0x38, 0x03, 0x01 };

/** Magnetometer Sensor Service UUID: 651f4c00-7579-11e4-82f8-0800200c9a66 */
const uint8_t MagnetometerSensorServiceUUID[SENSOR_SERVICES_UUID_MAX] = { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x79, 0x75, 0x00, 0x4C, 0x1F, 0x65 };

/** Environment Sensor Service UUID: 92dab060-7634-11e4-82f8-0800200c9a66 */
const uint8_t EnvironmentSensorServiceUUID[SENSOR_SERVICES_UUID_MAX] = { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x34, 0x76, 0x60, 0xB0, 0xDA, 0x92 };

/** High Data Rate Service UUID: c2967210-7ba4-11e4-82f8-0800200c9a66 */
const uint8_t HighDataRateServiceUUID[SENSOR_SERVICES_UUID_MAX] = { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0xA4, 0x7B, 0x10, 0x72, 0x96, 0xC2 };

/** Control Node Service UUID: 55b741d0-7ada-11e4-82f8-0800200c9a66 */
const uint8_t ControlNodeServiceUUID[SENSOR_SERVICES_UUID_MAX] = { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0xDA, 0x7A, 0xD0, 0x41, 0xB7, 0x55 };

/**
 * Firmware version information
 * Format: {year, month, day, build-number}
 */
static const uint8_t FirmwareVersionCharacteristicValue[4U] = { 18, 10, 13, 01 };

/** Characteristic properties for the accelerometer sensor */
static SensorServices_Characteristic_T AccelerometerCharacteristicProperties[SENSOR_AXIS_MAX] =
        {
                /* X-Axis Characteristic properties */
                /* Accelerometer X-Axis sensor value UUID: 5a211d41-7166-11e4-82f8-0800200c9a66 */
                {
                        { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x66, 0x71, 0x41, 0x1D, 0x21, 0x5A },
                        { 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
                },
                /* Y-Axis Characteristic properties */
                /* Accelerometer Y-Axis sensor value UUID: 5a211d42-7166-11e4-82f8-0800200c9a66 */
                {
                        { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x66, 0x71, 0x42, 0x1D, 0x21, 0x5A },
                        { 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
                },
                /* Z-Axis Characteristic properties */
                /* Accelerometer Z-Axis sensor value UUID: 5a211d43-7166-11e4-82f8-0800200c9a66 */
                {
                        { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x66, 0x71, 0x43, 0x1D, 0x21, 0x5A },
                        { 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
                }
        };

/** Characteristic properties for the gyro sensor */
static SensorServices_Characteristic_T GyroCharacteristicProperties[SENSOR_AXIS_MAX] =
        {
                /* X-Axis Characteristic properties */
                /* Gyro X-Axis sensor value UUID: aca96a41-74a4-11e4-82f8-0800200c9a66 */
                {
                        { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0xA4, 0x74, 0x41, 0x6A, 0xA9, 0xAC },
                        { 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
                },
                /* Y-Axis Characteristic properties */
                /* Gyro Y-Axis sensor value UUID: aca96a42-74a4-11e4-82f8-0800200c9a66 */
                {
                        { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0xA4, 0x74, 0x42, 0x6A, 0xA9, 0xAC },
                        { 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
                },
                /* Z-Axis Characteristic properties */
                /* Gyro Z-Axis sensor value UUID: aca96a43-74a4-11e4-82f8-0800200c9a66 */
                {
                        { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0xA4, 0x74, 0x43, 0x6A, 0xA9, 0xAC },
                        { 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
                }
        };

/** Characteristic properties for the light sensor
 * UUID: 38eb02c1-7540-11e4-82f8-0800200c9a66
 */
static SensorServices_Characteristic_T LightSensorCharacteristicProperties =
        {
                { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x40, 0x75, 0xC1, 0x02, 0xEB, 0x38 },
                { 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
        };

/** Characteristic properties for the noise sensor
 * UUID: 01033831-754c-11e4-82f8-0800200c9a66
 */
static SensorServices_Characteristic_T NoiseSensorCharacteristicProperties =
        {
                { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x4C, 0x75, 0x31, 0x38, 0x03, 0x01 },
                { 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
        };

/** Characteristic properties for the magnetometer sensor */
static SensorServices_Characteristic_T MagnetometerSensorCharacteristicProperties[4] =
        {
                /* X-Axis Characteristic properties */
                /* Magnetometer X-Axis sensor value UUID: 651f4c01-7579-11e4-82f8-0800200c9a66 */
                {
                        { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x79, 0x75, 0x01, 0x4C, 0x1F, 0x65 },
                        { 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
                },
                /* Y-Axis Characteristic properties */
                /* Magnetometer Y-Axis sensor value UUID: 651f4c02-7579-11e4-82f8-0800200c9a66 */
                {
                        { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x79, 0x75, 0x02, 0x4C, 0x1F, 0x65 },
                        { 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
                },
                /* Z-Axis Characteristic properties */
                /* Magnetometer Z-Axis sensor value UUID: 651f4c03-7579-11e4-82f8-0800200c9a66 */
                {
                        { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x79, 0x75, 0x03, 0x4C, 0x1F, 0x65 },
                        { 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
                },
                /* Magnetometer resistance Characteristic properties */
                /* Magnetometer sensor resistance value UUID: 651f4c04-7579-11e4-82f8-0800200c9a66 */
                {
                        { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x79, 0x75, 0x04, 0x4C, 0x1F, 0x65 },
                        { 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
                }
        };

/** Characteristic properties for the environment sensor pressure value
 * UUID: 92dab061-7634-11e4-82f8-0800200c9a66
 */
static SensorServices_Characteristic_T EnvironmentSensorPressureCharacteristicProperties =
        {
                { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x34, 0x76, 0x61, 0xB0, 0xDA, 0x92 },
                { 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
        };

/** Characteristic properties for the environment sensor temperature value
 * UUID: 92dab062-7634-11e4-82f8-0800200c9a66
 */
static SensorServices_Characteristic_T EnvironmentSensorTemperatureCharacteristicProperties =
        {
                { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x34, 0x76, 0x62, 0xB0, 0xDA, 0x92 },
                { 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
        };

/** Characteristic properties for the environment sensor humidity value
 * UUID: 92dab063-7634-11e4-82f8-0800200c9a66
 */
static SensorServices_Characteristic_T EnvironmentSensorHumidityCharacteristicProperties =
        {
                { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0x34, 0x76, 0x63, 0xB0, 0xDA, 0x92 },
                { 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
        };

/** Characteristic properties for the high rate data service */
static SensorServices_Characteristic_T HighDataRateCharacteristicProperties[] =
        {
                /* high priority Characteristic properties */
                /* high priority value UUID: c2967211-7ba4-11e4-82f8-0800200c9a66 */
                {
                        { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0xA4, 0x7B, 0x11, 0x72, 0x96, 0xC2 },
                        { 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
                },
                /*low priority Characteristic properties */
                /*low priority value UUID: c2967212-7ba4-11e4-82f8-0800200c9a66 */
                {
                        { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0xA4, 0x7B, 0x12, 0x72, 0x96, 0xC2 },
                        { 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                        { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
                }
        };

/** Characteristic property for the starting sensor sampling Characteristic
 * UUID: 55b741d1-7ada-11e4-82f8-0800200c9a66
 */
static SensorServices_Characteristic_T StartSensorSamplingAndNotificationsCharacteristicProperties =
        {
                { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0xDA, 0x7A, 0xD1, 0x41, 0xB7, 0x55 },
                { 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
        };

/** Characteristic property for the setting the sensor sampling rate Characteristic
 * UUID: 55b741d2-7ada-11e4-82f8-0800200c9a66
 */
static SensorServices_Characteristic_T SetSamplingRateCharacteristicProperties =
        {
                { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0xDA, 0x7A, 0xD2, 0x41, 0xB7, 0x55 },
                { 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
        };

/** Characteristic property for reset the Node Characteristic
 * UUID: 55b741d3-7ada-11e4-82f8-0800200c9a66
 */
static SensorServices_Characteristic_T RebootNodeCharacteristicProperties =
        {
                { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0xDA, 0x7A, 0xD3, 0x41, 0xB7, 0x55 },
                { 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
        };

/** Characteristic property for getting the firmware version sampling Characteristic
 * UUID: 55b741d4-7ada-11e4-82f8-0800200c9a66
 */
static SensorServices_Characteristic_T GetNodeFirmwareVersionCharacteristicProperties =
        {
                { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0xDA, 0x7A, 0xD4, 0x41, 0xB7, 0x55 },
                { 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
        };

/** Characteristic property to switch between orientationData or accelData + gpsData
 * UUID: 55b741d5-7ada-11e4-82f8-0800200c9a66
 */
static SensorServices_Characteristic_T UseBuiltInSensorFusionCharacteristicProperties =
        {
                { 0x66, 0x9A, 0x0C, 0x20, 0x00, 0x08, 0xF8, 0x82, 0xE4, 0x11, 0xDA, 0x7A, 0xD5, 0x41, 0xB7, 0x55 },
                { 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
        };

/** Container for the accelerometer sensor values send via BLE */
static uint8_t AcceleratorSensorCharacteristicValues[SENSOR_AXIS_MAX][2] = { { 0 } };

/** Container for the gyro sensor values send via BLE */
static uint8_t GyroSensorCharacteristicValues[SENSOR_AXIS_MAX][2] = { { 0 } };

/** Container for the light sensor values send via BLE */
static uint8_t LightSensorCharacteristicValue[4] = { 0 };

/** Container for the noise sensor values send via BLE */
static uint8_t NoiseSensorCharacteristicValue[4] = { 0 };

/** Container for the magnetometer sensor values send via BLE */
static uint8_t MagnetometerSensorCharacteristicValues[SENSOR_MAGNETOMETER_MAX][2] = { { 0 } };

/** Container for the pressure sensor values send via BLE */
static uint8_t EnvironmentSensorPressureCharacteristicValue[4] = { 0 };

/** Container for the temperature sensor values send via BLE */
static uint8_t EnvironmentSensorTemperatureCharacteristicValue[4] = { 0 };

/** Container for the humidity sensor values send via BLE */
static uint8_t EnvironmentSensorHumidityCharacteristicValue[4] = { 0 };

/** Container for the high data rate values send via BLE */
static uint8_t HighDataRateCharacteristicValue[HIGH_DATA_RATE_MAX][SENSOR_SERVICES_HIGH_DATA_RATE_ATT_LEN_MAX] = { { 0 } };

/** Container for the start sensor sampling message received via BLE */
static uint8_t StartSensorSamplingCharacteristicValue[1] = { 0 };

/** Container for the sampling time message received via BLE */
static uint8_t SetSensorSamplingTimerCharacteristicValue[4] = { 0 };

/** Container for the reset Node message received via BLE */
static uint8_t RebootNodeCharacteristicValue[1] = { 0 };

/** Container for the reset Node message received via BLE */
static uint8_t SwitchNodeCharacteristicValue[1] = { 0 };

/* Put constant and variable definitions here */

/** Callback function of type Ble_DataReceivedCallBack to read the data received */
static SensorServices_DataReceivedCallBack DataReadCallBack = NULL;

static SensorServices_SendEventCallback SendCallBack = NULL;

/* Put private function declarations here */

static AttServiceAttribute AccelerometorSensorService; /** Attribute handle for the accelerometer sensor service */

static AttServiceAttribute GyroSensorService; /** Attribute handle for the gyro sensor service */

static AttServiceAttribute LightSensorService; /** Attribute handle for the light sensor service */

static AttServiceAttribute NoiseSensorService; /** Attribute handle for the noise sensor service */

static AttServiceAttribute MagnetometerSensorService; /** Attribute handle for the magnetometer sensor service */

static AttServiceAttribute EnvironmentSensorService; /** Attribute handle for the environment sensor service */

static AttServiceAttribute HighDataRateService; /** Attribute handle for the high data rate service */

static AttServiceAttribute ControlNodeService; /** Attribute handle for the Node control service */

/* Put function implementations here */

/**
 * @brief This function handles accelerometer sensor service events.
 * @param[in] serverCallbackParms : Parameters holding service event status
 */
static void AcceleratorSensorServiceCallback(AttServerCallbackParms *serverCallbackParms)
{
    SensorServices_Info_T sensorServiceInfo;

    switch (serverCallbackParms->event)
    {
    case ATTEVT_SERVER_WRITE_COMPLETE:
        {
        if (ATTSTATUS_SUCCESS == serverCallbackParms->status)
        {
            AttWriteCompleteData rxData = serverCallbackParms->parms.writeComplete;
            sensorServiceInfo.sensorServicesType = (uint8_t) BLE_SENSOR_ACCELEROMETER;
            const uint8_t *rxValue = rxData.value;
            uint16_t rxLength = rxData.length;
            if (serverCallbackParms->parms.writeComplete.attribute == &AccelerometerCharacteristicProperties[SENSOR_AXIS_X].CharacteristicAttribute)
            {
                sensorServiceInfo.sensorServicesContent = (uint8_t) SENSOR_AXIS_X;
            }
            else if (serverCallbackParms->parms.writeComplete.attribute == &AccelerometerCharacteristicProperties[SENSOR_AXIS_Y].CharacteristicAttribute)
            {
                sensorServiceInfo.sensorServicesContent = (uint8_t) SENSOR_AXIS_Y;
            }
            else if (serverCallbackParms->parms.writeComplete.attribute == &AccelerometerCharacteristicProperties[SENSOR_AXIS_Z].CharacteristicAttribute)
            {
                sensorServiceInfo.sensorServicesContent = (uint8_t) SENSOR_AXIS_Z;
            }
            else
            {
                sensorServiceInfo.sensorServicesContent = (uint8_t) SENSOR_AXIS_MAX;
            }
            if (NULL != DataReadCallBack)
            {
                DataReadCallBack((uint8_t *) rxValue, rxLength, &sensorServiceInfo);
            }
        }
    }
        break;
    case ATTEVT_SERVER_HVI_SENT:
        {
        /* serverCallbackParms->parms.readReq.attribute must map with
         * &AccelerometerCharacteristicProperties[SensorServices_SensorAxisType_T].CharacteristicAttribute */

        Retcode_T retcode = RETCODE_OK;
        if (ATTSTATUS_SUCCESS != serverCallbackParms->status)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_DATA_SEND_FAIL);
        }
        if (NULL != SendCallBack)
        {
            SendCallBack(retcode);
        }
    }
        break;
    default:
        break;
    }
}

/**
 * @brief This function registers the accelerometer sensor in the attribute
 * database with a dedicated service and the associated characteristics.
 *
 * @return The status of registration.
 */
static BleStatus SensorServicesRegisterAccelerometer(void)
{
    ATT_SERVER_SecureDatabaseAccess();

    if (ATT_SERVER_RegisterServiceAttribute(
    ATTPDU_SIZEOF_128_BIT_UUID, (uint8_t *) AccelerometerSensorServiceUUID,
            AcceleratorSensorServiceCallback,
            &(AccelerometorSensorService)) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    ATT_SERVER_ReleaseDatabaseAccess();

    for (uint8_t i = 0U; i < (uint8_t) SENSOR_AXIS_MAX; i++)
    {
        ATT_SERVER_SecureDatabaseAccess();

        AccelerometerCharacteristicProperties[i].UUIDType.size =
        ATT_UUID_SIZE_128;
        AccelerometerCharacteristicProperties[i].UUIDType.value.uuid128 =
                (uint8_t *) AccelerometerCharacteristicProperties[i].CharacteristicUUID;

        if (ATT_SERVER_AddCharacteristic(
        ATTPROPERTY_READ | ATTPROPERTY_NOTIFY,
                (Att16BitCharacteristicAttribute*) &AccelerometerCharacteristicProperties[i].Characteristic,
                &AccelerometerCharacteristicProperties[i].UUIDType,
                ATT_PERMISSIONS_ALLACCESS, 2U,
                (uint8_t *) &AcceleratorSensorCharacteristicValues[i][0], 0, 0,
                &AccelerometorSensorService,
                &AccelerometerCharacteristicProperties[i].CharacteristicAttribute) == BLESTATUS_FAILED)
        {
            ATT_SERVER_ReleaseDatabaseAccess();
            return BLESTATUS_FAILED;
        }

        ATT_SERVER_ReleaseDatabaseAccess();
    }
    return BLESTATUS_SUCCESS;
}

/**@brief This function handles gyro sensor service events.
 * @param[i] serverCallbackParms Parameters holding service event status
 */
static void GyroSensorServiceCallback(AttServerCallbackParms* serverCallbackParms)
{
    SensorServices_Info_T sensorServiceInfo;

    switch (serverCallbackParms->event)
    {
    case ATTEVT_SERVER_WRITE_COMPLETE:
        {
        if (ATTSTATUS_SUCCESS == serverCallbackParms->status)
        {
            AttWriteCompleteData rxData = serverCallbackParms->parms.writeComplete;
            sensorServiceInfo.sensorServicesType = (uint8_t) BLE_SENSOR_GYRO;
            const uint8_t *rxValue = rxData.value;
            uint16_t rxLength = rxData.length;
            if (serverCallbackParms->parms.writeComplete.attribute == &GyroCharacteristicProperties[SENSOR_AXIS_X].CharacteristicAttribute)
            {
                sensorServiceInfo.sensorServicesContent = (uint8_t) SENSOR_AXIS_X;
            }
            else if (serverCallbackParms->parms.writeComplete.attribute == &GyroCharacteristicProperties[SENSOR_AXIS_Y].CharacteristicAttribute)
            {
                sensorServiceInfo.sensorServicesContent = (uint8_t) SENSOR_AXIS_Y;
            }
            else if (serverCallbackParms->parms.writeComplete.attribute == &GyroCharacteristicProperties[SENSOR_AXIS_Z].CharacteristicAttribute)
            {
                sensorServiceInfo.sensorServicesContent = (uint8_t) SENSOR_AXIS_Z;
            }
            else
            {
                sensorServiceInfo.sensorServicesContent = (uint8_t) SENSOR_AXIS_MAX;
            }
            if (NULL != DataReadCallBack)
            {
                DataReadCallBack((uint8_t *) rxValue, rxLength, &sensorServiceInfo);
            }
        }
    }
        break;
    case ATTEVT_SERVER_HVI_SENT:
        {
        /* serverCallbackParms->parms.readReq.attribute must map with
         * &GyroCharacteristicProperties[SensorServices_SensorAxisType_T].CharacteristicAttribute */
        Retcode_T retcode = RETCODE_OK;
        if (ATTSTATUS_SUCCESS != serverCallbackParms->status)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_DATA_SEND_FAIL);
        }
        if (NULL != SendCallBack)
        {
            SendCallBack(retcode);
        }
    }
        break;
    default:
        break;
    }
}

/**This function register the gyro sensor in the attribute database with a dedicated service and
 * the associated characteristics.
 * @brief The function register gyro sensor into the attribute database
 */
static BleStatus SensorServicesRegisterGyro(void)
{
    ATT_SERVER_SecureDatabaseAccess();

    if (ATT_SERVER_RegisterServiceAttribute(
    ATTPDU_SIZEOF_128_BIT_UUID, (uint8_t *) GyroSensorServiceUUID,
            GyroSensorServiceCallback, &(GyroSensorService)) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    ATT_SERVER_ReleaseDatabaseAccess();

    for (uint8_t i = 0U; i < (uint8_t) SENSOR_AXIS_MAX; i++)
    {
        ATT_SERVER_SecureDatabaseAccess();

        GyroCharacteristicProperties[i].UUIDType.size = ATT_UUID_SIZE_128;
        GyroCharacteristicProperties[i].UUIDType.value.uuid128 =
                (uint8_t *) GyroCharacteristicProperties[i].CharacteristicUUID;

        if (ATT_SERVER_AddCharacteristic(
        ATTPROPERTY_READ | ATTPROPERTY_NOTIFY,
                (Att16BitCharacteristicAttribute*) &GyroCharacteristicProperties[i].Characteristic,
                &GyroCharacteristicProperties[i].UUIDType,
                ATT_PERMISSIONS_ALLACCESS,
                (sizeof(GyroSensorCharacteristicValues) / sizeof(uint8_t)),
                (uint8_t *) &GyroSensorCharacteristicValues[i][0], 0, 0,
                &GyroSensorService,
                &GyroCharacteristicProperties[i].CharacteristicAttribute) == BLESTATUS_FAILED)
        {
            ATT_SERVER_ReleaseDatabaseAccess();
            return BLESTATUS_FAILED;
        }

        ATT_SERVER_ReleaseDatabaseAccess();
    }

    return BLESTATUS_SUCCESS;
}

/**@brief This function handles light sensor service events.
 * @param[i] serverCallbackParms Parameters holding service event status
 */
static void LightSensorServiceCallback(AttServerCallbackParms* serverCallbackParms)
{
    SensorServices_Info_T sensorServiceInfo;

    switch (serverCallbackParms->event)
    {
    case ATTEVT_SERVER_WRITE_COMPLETE:
        {
        if (ATTSTATUS_SUCCESS == serverCallbackParms->status)
        {
            AttWriteCompleteData rxData = serverCallbackParms->parms.writeComplete;
            if (serverCallbackParms->parms.writeComplete.attribute == &LightSensorCharacteristicProperties.CharacteristicAttribute)
            {
                sensorServiceInfo.sensorServicesType = (uint8_t) BLE_SENSOR_LIGHT;
                sensorServiceInfo.sensorServicesContent = UINT8_MAX;
                const uint8_t *rxValue = rxData.value;
                uint16_t rxLength = rxData.length;
                if (NULL != DataReadCallBack)
                {
                    DataReadCallBack((uint8_t *) rxValue, rxLength, &sensorServiceInfo);
                }
            }
        }
    }
        break;
    case ATTEVT_SERVER_HVI_SENT:
        {
        /* serverCallbackParms->parms.readReq.attribute must map with
         * &LightSensorCharacteristicProperties.CharacteristicAttribute */
        Retcode_T retcode = RETCODE_OK;
        if (ATTSTATUS_SUCCESS != serverCallbackParms->status)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_DATA_SEND_FAIL);
        }
        if (NULL != SendCallBack)
        {
            SendCallBack(retcode);
        }
    }
        break;
    default:
        break;
    }
}

/**This function register the light sensor in the attribute database with a dedicated service and
 * the associated characteristics.
 * @brief The function register light sensor into the attribute database
 */
static BleStatus SensorServicesRegisterLightSensor(void)
{
    ATT_SERVER_SecureDatabaseAccess();

    if (ATT_SERVER_RegisterServiceAttribute(
    ATTPDU_SIZEOF_128_BIT_UUID, (uint8_t *) LightSensorServiceUUID,
            LightSensorServiceCallback,
            &(LightSensorService)) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    LightSensorCharacteristicProperties.UUIDType.size = ATT_UUID_SIZE_128;
    LightSensorCharacteristicProperties.UUIDType.value.uuid128 =
            (uint8_t *) LightSensorCharacteristicProperties.CharacteristicUUID;

    if (ATT_SERVER_AddCharacteristic(
    ATTPROPERTY_READ | ATTPROPERTY_NOTIFY,
            (Att16BitCharacteristicAttribute*) &LightSensorCharacteristicProperties.Characteristic,
            &LightSensorCharacteristicProperties.UUIDType,
            ATT_PERMISSIONS_ALLACCESS, 4U,
            (uint8_t *) &LightSensorCharacteristicValue[0], 0, 0,
            &LightSensorService,
            &LightSensorCharacteristicProperties.CharacteristicAttribute) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    ATT_SERVER_ReleaseDatabaseAccess();

    return BLESTATUS_SUCCESS;
}

/**@brief This function handles several sensor service events.
 * @param[i] serverCallbackParms Parameters holding service event status
 */
static void SensorServicesCallback(AttServerCallbackParms* serverCallbackParms)
{
    switch (serverCallbackParms->event)
    {
    case ATTEVT_SERVER_HVI_SENT:
        {
        Retcode_T retcode = RETCODE_OK;
        if (ATTSTATUS_SUCCESS != serverCallbackParms->status)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_DATA_SEND_FAIL);
        }
        if (NULL != SendCallBack)
        {
            SendCallBack(retcode);
        }
    }
        break;
    default:
        break;
    }
}

/**This function register the noise sensor in the attribute database with a dedicated service and
 * the associated characteristics.
 * @brief The function register noise sensor into the attribute database
 */
static BleStatus SensorServicesRegisterNoiseSensor(void)
{
    ATT_SERVER_SecureDatabaseAccess();

    if (ATT_SERVER_RegisterServiceAttribute(
    ATTPDU_SIZEOF_128_BIT_UUID, (uint8_t *) NoiseSensorServiceUUID,
            SensorServicesCallback, &(NoiseSensorService)) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    NoiseSensorCharacteristicProperties.UUIDType.size = ATT_UUID_SIZE_128;
    NoiseSensorCharacteristicProperties.UUIDType.value.uuid128 =
            (uint8_t *) NoiseSensorCharacteristicProperties.CharacteristicUUID;

    if (ATT_SERVER_AddCharacteristic(
    ATTPROPERTY_READ | ATTPROPERTY_NOTIFY,
            (Att16BitCharacteristicAttribute*) &NoiseSensorCharacteristicProperties.Characteristic,
            &NoiseSensorCharacteristicProperties.UUIDType,
            ATT_PERMISSIONS_ALLACCESS, 4U,
            (uint8_t *) &NoiseSensorCharacteristicValue[0], 0, 0,
            &NoiseSensorService,
            &NoiseSensorCharacteristicProperties.CharacteristicAttribute) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    ATT_SERVER_ReleaseDatabaseAccess();

    return BLESTATUS_SUCCESS;
}

/**This function register the magnetometer sensor in the attribute database with a dedicated service and
 * the associated characteristics.
 * @brief The function register magnetometer sensor into the attribute database
 */
static BleStatus SensorServicesRegisterMagnetometerSensor(void)
{
    ATT_SERVER_SecureDatabaseAccess();

    if (ATT_SERVER_RegisterServiceAttribute(
    ATTPDU_SIZEOF_128_BIT_UUID, (uint8_t *) MagnetometerSensorServiceUUID,
            SensorServicesCallback,
            &(MagnetometerSensorService)) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    ATT_SERVER_ReleaseDatabaseAccess();

    /* Add the magnetometer values for the 3 axis to the attribute database */
    for (uint8_t i = 0; i < (uint8_t) SENSOR_MAGNETOMETER_MAX; i++)
    {
        ATT_SERVER_SecureDatabaseAccess();

        MagnetometerSensorCharacteristicProperties[i].UUIDType.size =
        ATT_UUID_SIZE_128;
        MagnetometerSensorCharacteristicProperties[i].UUIDType.value.uuid128 =
                (uint8_t *) MagnetometerSensorCharacteristicProperties[i].CharacteristicUUID;

        if (ATT_SERVER_AddCharacteristic(
        ATTPROPERTY_READ | ATTPROPERTY_NOTIFY,
                (Att16BitCharacteristicAttribute*) &MagnetometerSensorCharacteristicProperties[i].Characteristic,
                &MagnetometerSensorCharacteristicProperties[i].UUIDType,
                ATT_PERMISSIONS_ALLACCESS, 2U,
                (uint8_t *) &MagnetometerSensorCharacteristicValues[i][0], 0, 0,
                &MagnetometerSensorService,
                &MagnetometerSensorCharacteristicProperties[i].CharacteristicAttribute) == BLESTATUS_FAILED)
        {
            ATT_SERVER_ReleaseDatabaseAccess();
            return BLESTATUS_FAILED;
        }

        ATT_SERVER_ReleaseDatabaseAccess();
    }
    return BLESTATUS_SUCCESS;
}

/**This function register the environment sensor in the attribute database with a dedicated service and
 * the associated characteristics.
 * @brief The function register environment sensor into the attribute database
 */
static BleStatus SensorServicesRegisterEnvironmentSensor(void)
{
    ATT_SERVER_SecureDatabaseAccess();

    if (ATT_SERVER_RegisterServiceAttribute(
    ATTPDU_SIZEOF_128_BIT_UUID, (uint8_t *) EnvironmentSensorServiceUUID,
            SensorServicesCallback,
            &(EnvironmentSensorService)) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    /* register pressure sensor Characteristic */

    EnvironmentSensorPressureCharacteristicProperties.UUIDType.size =
    ATT_UUID_SIZE_128;
    EnvironmentSensorPressureCharacteristicProperties.UUIDType.value.uuid128 =
            (uint8_t *) EnvironmentSensorPressureCharacteristicProperties.CharacteristicUUID;

    if (ATT_SERVER_AddCharacteristic(
    ATTPROPERTY_READ | ATTPROPERTY_NOTIFY,
            (Att16BitCharacteristicAttribute*) &EnvironmentSensorPressureCharacteristicProperties.Characteristic,
            &EnvironmentSensorPressureCharacteristicProperties.UUIDType,
            ATT_PERMISSIONS_ALLACCESS, 4U,
            (uint8_t *) &EnvironmentSensorPressureCharacteristicValue[0], 0, 0,
            &EnvironmentSensorService,
            &EnvironmentSensorPressureCharacteristicProperties.CharacteristicAttribute) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    /* register temperature sensor Characteristic */

    EnvironmentSensorTemperatureCharacteristicProperties.UUIDType.size =
    ATT_UUID_SIZE_128;
    EnvironmentSensorTemperatureCharacteristicProperties.UUIDType.value.uuid128 =
            (uint8_t *) EnvironmentSensorTemperatureCharacteristicProperties.CharacteristicUUID;

    if (ATT_SERVER_AddCharacteristic(
    ATTPROPERTY_READ | ATTPROPERTY_NOTIFY,
            (Att16BitCharacteristicAttribute*) &EnvironmentSensorTemperatureCharacteristicProperties.Characteristic,
            &EnvironmentSensorTemperatureCharacteristicProperties.UUIDType,
            ATT_PERMISSIONS_ALLACCESS, 4U,
            (uint8_t *) &EnvironmentSensorTemperatureCharacteristicValue[0], 0,
            0, &EnvironmentSensorService,
            &EnvironmentSensorTemperatureCharacteristicProperties.CharacteristicAttribute) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    /*  register humidity sensor Characteristic */

    EnvironmentSensorHumidityCharacteristicProperties.UUIDType.size =
    ATT_UUID_SIZE_128;
    EnvironmentSensorHumidityCharacteristicProperties.UUIDType.value.uuid128 =
            (uint8_t *) EnvironmentSensorHumidityCharacteristicProperties.CharacteristicUUID;

    if (ATT_SERVER_AddCharacteristic(
    ATTPROPERTY_READ | ATTPROPERTY_NOTIFY,
            (Att16BitCharacteristicAttribute*) &EnvironmentSensorHumidityCharacteristicProperties.Characteristic,
            &EnvironmentSensorHumidityCharacteristicProperties.UUIDType,
            ATT_PERMISSIONS_ALLACCESS, 4U,
            (uint8_t *) &EnvironmentSensorHumidityCharacteristicValue[0], 0, 0,
            &EnvironmentSensorService,
            &EnvironmentSensorHumidityCharacteristicProperties.CharacteristicAttribute) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    ATT_SERVER_ReleaseDatabaseAccess();

    return BLESTATUS_SUCCESS;
}

/**@brief This function handles high data rate service events.
 * @param[i] serverCallbackParms Parameters holding service event status
 */
static void HighDataRateServiceCallback(AttServerCallbackParms* serverCallbackParms)
{
    SensorServices_Info_T sensorServiceInfo;
    switch (serverCallbackParms->event)
    {
    case ATTEVT_SERVER_WRITE_COMPLETE:
        {
        if (ATTSTATUS_SUCCESS == serverCallbackParms->status)
        {
            AttWriteCompleteData rxData = serverCallbackParms->parms.writeComplete;
            sensorServiceInfo.sensorServicesType = (uint8_t) BLE_HIGH_DATA_RATE;
            const uint8_t *rxValue = rxData.value;
            uint16_t rxLength = rxData.length;
            if (serverCallbackParms->parms.writeComplete.attribute == &HighDataRateCharacteristicProperties[HIGH_DATA_RATE_HIGH_PRIO].CharacteristicAttribute)
            {
                sensorServiceInfo.sensorServicesContent = (uint8_t) HIGH_DATA_RATE_HIGH_PRIO;
            }
            else if (serverCallbackParms->parms.writeComplete.attribute == &HighDataRateCharacteristicProperties[HIGH_DATA_RATE_LOW_PRIO].CharacteristicAttribute)
            {
                sensorServiceInfo.sensorServicesContent = (uint8_t) HIGH_DATA_RATE_LOW_PRIO;
            }
            else
            {
                sensorServiceInfo.sensorServicesContent = (uint8_t) HIGH_DATA_RATE_MAX;
            }
            if (NULL != DataReadCallBack)
            {
                DataReadCallBack((uint8_t *) rxValue, rxLength, &sensorServiceInfo);
            }
        }
    }
        break;
    case ATTEVT_SERVER_HVI_SENT:
        {
        /* serverCallbackParms->parms.readReq.attribute must map with
         * &HighDataRateCharacteristicProperties[SensorServices_HighDataRateType_T].CharacteristicAttribute */
        Retcode_T retcode = RETCODE_OK;
        if (ATTSTATUS_SUCCESS != serverCallbackParms->status)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_DATA_SEND_FAIL);
        }
        if (NULL != SendCallBack)
        {
            SendCallBack(retcode);
        }
    }
        break;
    default:
        break;
    }
}

/**This function register the high data rate service in the attribute database with a dedicated service and
 * the associated characteristics.
 * @brief The function register the high data rate service in the attribute database
 */
static BleStatus SensorServicesRegisterHighDataRateService(void)
{
    ATT_SERVER_SecureDatabaseAccess();

    if (ATT_SERVER_RegisterServiceAttribute(
    ATTPDU_SIZEOF_128_BIT_UUID, (uint8_t *) HighDataRateServiceUUID,
            HighDataRateServiceCallback,
            &(HighDataRateService)) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    ATT_SERVER_ReleaseDatabaseAccess();

    for (uint8_t i = 0; i < 2; i++)
    {
        ATT_SERVER_SecureDatabaseAccess();

        HighDataRateCharacteristicProperties[i].UUIDType.size =
        ATT_UUID_SIZE_128;
        HighDataRateCharacteristicProperties[i].UUIDType.value.uuid128 =
                (uint8_t *) HighDataRateCharacteristicProperties[i].CharacteristicUUID;

        if (ATT_SERVER_AddCharacteristic(
        ATTPROPERTY_READ | ATTPROPERTY_NOTIFY,
                (Att16BitCharacteristicAttribute*) &HighDataRateCharacteristicProperties[i].Characteristic,
                &HighDataRateCharacteristicProperties[i].UUIDType,
                ATT_PERMISSIONS_ALLACCESS, SENSOR_SERVICES_HIGH_DATA_RATE_ATT_LEN_MAX,
                (uint8_t *) &HighDataRateCharacteristicValue[i][0], 0, 0,
                &HighDataRateService,
                &HighDataRateCharacteristicProperties[i].CharacteristicAttribute) == BLESTATUS_FAILED)
        {
            ATT_SERVER_ReleaseDatabaseAccess();
            return BLESTATUS_FAILED;
        }

        ATT_SERVER_ReleaseDatabaseAccess();
    }
    return BLESTATUS_SUCCESS;
}

/**@brief This function handles the Node control service events.
 * @param[i] serverCallbackParms Parameters holding service event status
 */
static void ControlXdkServiceCallback(AttServerCallbackParms* serverCallbackParms)
{
    SensorServices_Info_T sensorServiceInfo;
    switch (serverCallbackParms->event)
    {
    case ATTEVT_SERVER_WRITE_COMPLETE:
        {
        AttWriteCompleteData rxData = serverCallbackParms->parms.writeComplete;
        sensorServiceInfo.sensorServicesType = (uint8_t) BLE_CONTROL_NODE;
        const uint8_t *rxValue = rxData.value;
        uint16_t rxLength = rxData.length;
        if (serverCallbackParms->parms.writeComplete.attribute == &StartSensorSamplingAndNotificationsCharacteristicProperties.CharacteristicAttribute)
        {
            sensorServiceInfo.sensorServicesContent = (uint8_t) CONTROL_NODE_START_SAMPLING;
        }
        else if (serverCallbackParms->parms.writeComplete.attribute == &SetSamplingRateCharacteristicProperties.CharacteristicAttribute)
        {
            sensorServiceInfo.sensorServicesContent = (uint8_t) CONTROL_NODE_CHANGE_SAMPLING_RATE;
        }
        else if (serverCallbackParms->parms.writeComplete.attribute == &RebootNodeCharacteristicProperties.CharacteristicAttribute)
        {
            sensorServiceInfo.sensorServicesContent = (uint8_t) CONTROL_NODE_REBOOT;
        }
        else if (serverCallbackParms->parms.writeComplete.attribute == &GetNodeFirmwareVersionCharacteristicProperties.CharacteristicAttribute)
        {
            sensorServiceInfo.sensorServicesContent = (uint8_t) CONTROL_NODE_GET_FW_VERSION;
        }
        else if (serverCallbackParms->parms.writeComplete.attribute == &UseBuiltInSensorFusionCharacteristicProperties.CharacteristicAttribute)
        {
            sensorServiceInfo.sensorServicesContent = (uint8_t) CONTROL_NODE_USE_SENSOR_FUSION;
        }
        else
        {
            sensorServiceInfo.sensorServicesContent = (uint8_t) CONTROL_NODE_MAX;
        }
        if (NULL != DataReadCallBack)
        {
            DataReadCallBack((uint8_t *) rxValue, rxLength, &sensorServiceInfo);
        }

    }
        break;
    case ATTEVT_SERVER_HVI_SENT:
        {
        Retcode_T retcode = RETCODE_OK;
        if (ATTSTATUS_SUCCESS != serverCallbackParms->status)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_DATA_SEND_FAIL);
        }
        if (NULL != SendCallBack)
        {
            SendCallBack(retcode);
        }
    }
        break;
    default:
        break;
    }
}

/**This function register the Node control service in the attribute database with a dedicated service and
 * the associated characteristics.
 * @brief The function register Node control service into the attribute database
 */
static BleStatus SensorServicesRegisterControlSensor(void)
{
    ATT_SERVER_SecureDatabaseAccess();

    if (ATT_SERVER_RegisterServiceAttribute(
    ATTPDU_SIZEOF_128_BIT_UUID, (uint8_t *) ControlNodeServiceUUID,
            ControlXdkServiceCallback, &(ControlNodeService)) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    /* Start sensor sampling Characteristic */
    StartSensorSamplingAndNotificationsCharacteristicProperties.UUIDType.size =
    ATT_UUID_SIZE_128;
    StartSensorSamplingAndNotificationsCharacteristicProperties.UUIDType.value.uuid128 =
            (uint8_t *) StartSensorSamplingAndNotificationsCharacteristicProperties.CharacteristicUUID;

    if (ATT_SERVER_AddCharacteristic(
    ATTPROPERTY_WRITE,
            (Att16BitCharacteristicAttribute*) &StartSensorSamplingAndNotificationsCharacteristicProperties.Characteristic,
            &StartSensorSamplingAndNotificationsCharacteristicProperties.UUIDType,
            ATT_PERMISSIONS_ALLACCESS, 1U,
            (uint8_t *) &StartSensorSamplingCharacteristicValue[0], 0, 0,
            &ControlNodeService,
            &StartSensorSamplingAndNotificationsCharacteristicProperties.CharacteristicAttribute) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    /* Sampling Rate Characteristic */
    SetSamplingRateCharacteristicProperties.UUIDType.size = ATT_UUID_SIZE_128;
    SetSamplingRateCharacteristicProperties.UUIDType.value.uuid128 =
            (uint8_t *) SetSamplingRateCharacteristicProperties.CharacteristicUUID;

    if (ATT_SERVER_AddCharacteristic(
    ATTPROPERTY_WRITE,
            (Att16BitCharacteristicAttribute*) &SetSamplingRateCharacteristicProperties.Characteristic,
            &SetSamplingRateCharacteristicProperties.UUIDType,
            ATT_PERMISSIONS_ALLACCESS, 4U,
            (uint8_t *) &SetSensorSamplingTimerCharacteristicValue[0], 0, 0,
            &ControlNodeService,
            &SetSamplingRateCharacteristicProperties.CharacteristicAttribute) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    /* Reset Node Characteristic */
    RebootNodeCharacteristicProperties.UUIDType.size = ATT_UUID_SIZE_128;
    RebootNodeCharacteristicProperties.UUIDType.value.uuid128 =
            (uint8_t *) RebootNodeCharacteristicProperties.CharacteristicUUID;

    if (ATT_SERVER_AddCharacteristic(
    ATTPROPERTY_WRITE,
            (Att16BitCharacteristicAttribute*) &RebootNodeCharacteristicProperties.Characteristic,
            &RebootNodeCharacteristicProperties.UUIDType,
            ATT_PERMISSIONS_ALLACCESS, 1U,
            (uint8_t *) &RebootNodeCharacteristicValue[0], 0, 0,
            &ControlNodeService,
            &RebootNodeCharacteristicProperties.CharacteristicAttribute) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    /* Node Firmware Characteristic */

    GetNodeFirmwareVersionCharacteristicProperties.UUIDType.size =
    ATT_UUID_SIZE_128;
    GetNodeFirmwareVersionCharacteristicProperties.UUIDType.value.uuid128 =
            (uint8_t *) GetNodeFirmwareVersionCharacteristicProperties.CharacteristicUUID;

    if (ATT_SERVER_AddCharacteristic(
    ATTPROPERTY_WRITE,
            (Att16BitCharacteristicAttribute*) &GetNodeFirmwareVersionCharacteristicProperties.Characteristic,
            &GetNodeFirmwareVersionCharacteristicProperties.UUIDType,
            ATT_PERMISSIONS_ALLACCESS, 4U,
            (uint8_t *) &FirmwareVersionCharacteristicValue[0], 0, 0,
            &ControlNodeService,
            &GetNodeFirmwareVersionCharacteristicProperties.CharacteristicAttribute) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    UseBuiltInSensorFusionCharacteristicProperties.UUIDType.size = ATT_UUID_SIZE_128;
    UseBuiltInSensorFusionCharacteristicProperties.UUIDType.value.uuid128 = (uint8_t *) UseBuiltInSensorFusionCharacteristicProperties.CharacteristicUUID;

    if (ATT_SERVER_AddCharacteristic(
    ATTPROPERTY_WRITE,
            (Att16BitCharacteristicAttribute*) &UseBuiltInSensorFusionCharacteristicProperties.Characteristic,
            &UseBuiltInSensorFusionCharacteristicProperties.UUIDType,
            ATT_PERMISSIONS_ALLACCESS, 1U,
            (uint8_t *) &SwitchNodeCharacteristicValue[0], 0, 0,
            &ControlNodeService,
            &UseBuiltInSensorFusionCharacteristicProperties.CharacteristicAttribute
            ) == BLESTATUS_FAILED)
    {
        ATT_SERVER_ReleaseDatabaseAccess();
        return BLESTATUS_FAILED;
    }

    ATT_SERVER_ReleaseDatabaseAccess();

    return BLESTATUS_SUCCESS;
}

static Retcode_T WriteAndNotifyAccelerometerAxisValue(int16_t value, SensorServices_SensorAxisType_T axis)
{
    Retcode_T retcode = RETCODE_OK;

    if (axis >= SENSOR_AXIS_MAX)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_INVALID_PARAM);
    }
    else
    {
        BleStatus status;

        AcceleratorSensorCharacteristicValues[axis][0] = (uint8_t) value;
        AcceleratorSensorCharacteristicValues[axis][1] = (uint8_t) (value >> 8U);

        ATT_SERVER_SecureDatabaseAccess();

        status = ATT_SERVER_WriteAttributeValue(&AccelerometerCharacteristicProperties[axis].CharacteristicAttribute,
                (const uint8_t *) &AcceleratorSensorCharacteristicValues[axis][0], (sizeof(AcceleratorSensorCharacteristicValues[axis]) / sizeof(uint8_t)));
        if (BLESTATUS_SUCCESS != status)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_SENSORSERVICES_WRITE_ATT_VALUE_FAILED);
        }

        if (BLESTATUS_SUCCESS == status) /* send notification */
        {
            status = ATT_SERVER_SendNotification(&AccelerometerCharacteristicProperties[axis].CharacteristicAttribute, ConnectionHandle);
            /* BLESTATUS_SUCCESS and BLESTATUS_PENDING are fine */
            if ((BLESTATUS_FAILED == status) || (BLESTATUS_INVALID_PARMS == status))
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_SEND_NOTIFICATION_FAILED);
            }
        }

        ATT_SERVER_ReleaseDatabaseAccess();
    }
    return retcode;
}

static Retcode_T WriteAndNotifyGyroAxisValue(int16_t value, SensorServices_SensorAxisType_T axis)
{
    Retcode_T retcode = RETCODE_OK;

    if (axis >= SENSOR_AXIS_MAX)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_INVALID_PARAM);
    }
    else
    {
        BleStatus status;

        GyroSensorCharacteristicValues[axis][0] = (uint8_t) value;
        GyroSensorCharacteristicValues[axis][1] = (uint8_t) (value >> 8U);

        ATT_SERVER_SecureDatabaseAccess();

        status = ATT_SERVER_WriteAttributeValue(&GyroCharacteristicProperties[axis].CharacteristicAttribute,
                (const uint8_t *) &GyroSensorCharacteristicValues[axis][0], (sizeof(GyroSensorCharacteristicValues) / sizeof(uint8_t)));
        if (BLESTATUS_SUCCESS != status)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_SENSORSERVICES_WRITE_ATT_VALUE_FAILED);
        }

        if (status == BLESTATUS_SUCCESS) /* send notification */
        {
            status = ATT_SERVER_SendNotification(&GyroCharacteristicProperties[axis].CharacteristicAttribute, ConnectionHandle);
            /* BLESTATUS_SUCCESS and BLESTATUS_PENDING are fine */
            if ((BLESTATUS_FAILED == status) || (BLESTATUS_INVALID_PARMS == status))
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_SEND_NOTIFICATION_FAILED);
            }
        }

        ATT_SERVER_ReleaseDatabaseAccess();

    }
    return retcode;
}

static Retcode_T WriteAndNotifyLightSensorValue(uint32_t value)
{
    Retcode_T retcode = RETCODE_OK;
    BleStatus status = BLESTATUS_FAILED;

    LightSensorCharacteristicValue[0] = (uint8_t) value;
    LightSensorCharacteristicValue[1] = (uint8_t) (value >> 8U);
    LightSensorCharacteristicValue[2] = (uint8_t) (value >> 16U);
    LightSensorCharacteristicValue[3] = (uint8_t) (value >> 24U);

    ATT_SERVER_SecureDatabaseAccess();

    status = ATT_SERVER_WriteAttributeValue(&LightSensorCharacteristicProperties.CharacteristicAttribute,
            (const uint8_t *) &LightSensorCharacteristicValue[0], 4U);
    if (BLESTATUS_SUCCESS != status)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_SENSORSERVICES_WRITE_ATT_VALUE_FAILED);
    }

    if (status == BLESTATUS_SUCCESS) /* send notification */
    {
        status = ATT_SERVER_SendNotification(&LightSensorCharacteristicProperties.CharacteristicAttribute, ConnectionHandle);
        /* BLESTATUS_SUCCESS and BLESTATUS_PENDING are fine */
        if ((BLESTATUS_FAILED == status) || (BLESTATUS_INVALID_PARMS == status))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_SEND_NOTIFICATION_FAILED);
        }
    }

    ATT_SERVER_ReleaseDatabaseAccess();

    return retcode;
}

static Retcode_T WriteAndNotifyNoiseSensorValue(uint32_t value)
{
    Retcode_T retcode = RETCODE_OK;
    BleStatus status;

    NoiseSensorCharacteristicValue[0] = (uint8_t) value;
    NoiseSensorCharacteristicValue[1] = (uint8_t) (value >> 8U);
    NoiseSensorCharacteristicValue[2] = (uint8_t) (value >> 16U);
    NoiseSensorCharacteristicValue[3] = (uint8_t) (value >> 24U);

    ATT_SERVER_SecureDatabaseAccess();

    status = ATT_SERVER_WriteAttributeValue(&NoiseSensorCharacteristicProperties.CharacteristicAttribute,
            (const uint8_t *) &NoiseSensorCharacteristicValue[0], 4U);
    if (BLESTATUS_SUCCESS != status)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_SENSORSERVICES_WRITE_ATT_VALUE_FAILED);
    }

    if (status == BLESTATUS_SUCCESS) /* send notification */
    {
        status = ATT_SERVER_SendNotification(&NoiseSensorCharacteristicProperties.CharacteristicAttribute, ConnectionHandle);
        /* BLESTATUS_SUCCESS and BLESTATUS_PENDING are fine */
        if ((BLESTATUS_FAILED == status) || (BLESTATUS_INVALID_PARMS == status))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_SEND_NOTIFICATION_FAILED);
        }
    }

    ATT_SERVER_ReleaseDatabaseAccess();

    return retcode;
}

static Retcode_T WriteAndNotifyMagnetometerValue(int16_t value, SensorServices_MagnetometerType_T axis)
{
    Retcode_T retcode = RETCODE_OK;

    if (axis >= SENSOR_MAGNETOMETER_MAX)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_INVALID_PARAM);
    }
    else
    {
        BleStatus status;

        MagnetometerSensorCharacteristicValues[axis][0] = (uint8_t) (value);
        MagnetometerSensorCharacteristicValues[axis][1] = (uint8_t) (value >> 8U);

        ATT_SERVER_SecureDatabaseAccess();

        status = ATT_SERVER_WriteAttributeValue(&MagnetometerSensorCharacteristicProperties[axis].CharacteristicAttribute,
                (const uint8_t *) &MagnetometerSensorCharacteristicValues[axis][0], (sizeof(MagnetometerSensorCharacteristicValues[axis]) / sizeof(uint8_t)));
        if (BLESTATUS_SUCCESS != status)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_SENSORSERVICES_WRITE_ATT_VALUE_FAILED);
        }

        if (status == BLESTATUS_SUCCESS) /* send notification */
        {
            status = ATT_SERVER_SendNotification(&MagnetometerSensorCharacteristicProperties[axis].CharacteristicAttribute, ConnectionHandle);
            /* BLESTATUS_SUCCESS and BLESTATUS_PENDING are fine */
            if ((BLESTATUS_FAILED == status) || (BLESTATUS_INVALID_PARMS == status))
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_SEND_NOTIFICATION_FAILED);
            }
        }

        ATT_SERVER_ReleaseDatabaseAccess();

    }
    return retcode;
}

static Retcode_T WriteAndNotifyPressureSensorValue(uint32_t value)
{
    Retcode_T retcode = RETCODE_OK;
    BleStatus status;

    EnvironmentSensorPressureCharacteristicValue[0] = (uint8_t) value;
    EnvironmentSensorPressureCharacteristicValue[1] = (uint8_t) (value >> 8U);
    EnvironmentSensorPressureCharacteristicValue[2] = (uint8_t) (value >> 16U);
    EnvironmentSensorPressureCharacteristicValue[3] = (uint8_t) (value >> 24U);

    ATT_SERVER_SecureDatabaseAccess();

    status = ATT_SERVER_WriteAttributeValue(&EnvironmentSensorPressureCharacteristicProperties.CharacteristicAttribute,
            (const uint8_t *) &EnvironmentSensorPressureCharacteristicValue[0], 4U);
    if (BLESTATUS_SUCCESS != status)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_SENSORSERVICES_WRITE_ATT_VALUE_FAILED);
    }

    if (status == BLESTATUS_SUCCESS) /* send notification */
    {
        status = ATT_SERVER_SendNotification(&EnvironmentSensorPressureCharacteristicProperties.CharacteristicAttribute, ConnectionHandle);
        /* BLESTATUS_SUCCESS and BLESTATUS_PENDING are fine */
        if ((BLESTATUS_FAILED == status) || (BLESTATUS_INVALID_PARMS == status))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_SEND_NOTIFICATION_FAILED);
        }
    }

    ATT_SERVER_ReleaseDatabaseAccess();

    return retcode;
}

static Retcode_T WriteAndNotifyTemperatureSensorValue(int32_t value)
{
    Retcode_T retcode = RETCODE_OK;
    BleStatus status;

    EnvironmentSensorTemperatureCharacteristicValue[0] = (uint8_t) value;
    EnvironmentSensorTemperatureCharacteristicValue[1] = (uint8_t) (value >> 8U);
    EnvironmentSensorTemperatureCharacteristicValue[2] = (uint8_t) (value >> 16U);
    EnvironmentSensorTemperatureCharacteristicValue[3] = (uint8_t) (value >> 24U);

    ATT_SERVER_SecureDatabaseAccess();

    status = ATT_SERVER_WriteAttributeValue(&EnvironmentSensorTemperatureCharacteristicProperties.CharacteristicAttribute,
            (const uint8_t *) &EnvironmentSensorTemperatureCharacteristicValue[0], 4U);
    if (BLESTATUS_SUCCESS != status)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_SENSORSERVICES_WRITE_ATT_VALUE_FAILED);
    }

    if (status == BLESTATUS_SUCCESS) /* send notification */
    {
        status = ATT_SERVER_SendNotification(&EnvironmentSensorTemperatureCharacteristicProperties.CharacteristicAttribute, ConnectionHandle);
        /* BLESTATUS_SUCCESS and BLESTATUS_PENDING are fine */
        if ((BLESTATUS_FAILED == status) || (BLESTATUS_INVALID_PARMS == status))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_SEND_NOTIFICATION_FAILED);
        }
    }

    ATT_SERVER_ReleaseDatabaseAccess();

    return retcode;
}

static Retcode_T WriteAndNotifyHumiditySensorValue(uint32_t value)
{
    Retcode_T retcode = RETCODE_OK;
    BleStatus status;

    EnvironmentSensorHumidityCharacteristicValue[0] = (uint8_t) value;
    EnvironmentSensorHumidityCharacteristicValue[1] = (uint8_t) (value >> 8U);
    EnvironmentSensorHumidityCharacteristicValue[2] = (uint8_t) (value >> 16U);
    EnvironmentSensorHumidityCharacteristicValue[3] = (uint8_t) (value >> 24U);

    ATT_SERVER_SecureDatabaseAccess();

    status = ATT_SERVER_WriteAttributeValue(&EnvironmentSensorHumidityCharacteristicProperties.CharacteristicAttribute,
            (const uint8_t *) &EnvironmentSensorHumidityCharacteristicValue[0], 4U);
    if (BLESTATUS_SUCCESS != status)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_SENSORSERVICES_WRITE_ATT_VALUE_FAILED);
    }

    if (status == BLESTATUS_SUCCESS) /* send notification */
    {
        status = ATT_SERVER_SendNotification(&EnvironmentSensorHumidityCharacteristicProperties.CharacteristicAttribute, ConnectionHandle);
        /* BLESTATUS_SUCCESS and BLESTATUS_PENDING are fine */
        if ((BLESTATUS_FAILED == status) || (BLESTATUS_INVALID_PARMS == status))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_SEND_NOTIFICATION_FAILED);
        }
    }

    ATT_SERVER_ReleaseDatabaseAccess();

    return retcode;
}

static Retcode_T WriteAndNotifyPriorityData(uint8_t* buffer, U8 payloadLen, SensorServices_HighDataRateType_T priority)
{
    Retcode_T retcode = RETCODE_OK;

    if (priority >= HIGH_DATA_RATE_MAX)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_INVALID_PARAM);
    }
    else
    {
        BleStatus status;

        memset(HighDataRateCharacteristicValue[priority], 0, SENSOR_SERVICES_HIGH_DATA_RATE_ATT_LEN_MAX);
        memcpy(HighDataRateCharacteristicValue[priority], buffer, payloadLen);

        ATT_SERVER_SecureDatabaseAccess();

        status = ATT_SERVER_WriteAttributeValue(&HighDataRateCharacteristicProperties[priority].CharacteristicAttribute,
                (const uint8_t *) &HighDataRateCharacteristicValue[priority][0], SENSOR_SERVICES_HIGH_DATA_RATE_ATT_LEN_MAX);
        if (BLESTATUS_SUCCESS != status)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, (Retcode_T ) RETCODE_SENSORSERVICES_WRITE_ATT_VALUE_FAILED);
        }

        if (status == BLESTATUS_SUCCESS) /* send notification */
        {
            status = ATT_SERVER_SendNotification(&HighDataRateCharacteristicProperties[priority].CharacteristicAttribute, ConnectionHandle);
            /* BLESTATUS_SUCCESS and BLESTATUS_PENDING are fine */
            if ((BLESTATUS_FAILED == status) || (BLESTATUS_INVALID_PARMS == status))
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_SEND_NOTIFICATION_FAILED);
            }
        }

        ATT_SERVER_ReleaseDatabaseAccess();

    }
    return retcode;
}

/* The description is in the private header */
Retcode_T SensorServices_Init(SensorServices_DataReceivedCallBack readCallback, SensorServices_SendEventCallback sendCallback)
{
    if (NULL != readCallback)
    {
        DataReadCallBack = readCallback;
        SendCallBack = sendCallback;
        return RETCODE_OK;
    }
    else
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
}

Retcode_T SensorServices_Register(void)
{
    BleStatus registerStatus = BLESTATUS_SUCCESS;

    /* Accelerator sensor Service Register */
    registerStatus = SensorServicesRegisterAccelerometer();

    if (BLESTATUS_FAILED != registerStatus)
    {
        /* Gyro sensor Service Register */
        registerStatus = SensorServicesRegisterGyro();
    }

    if (BLESTATUS_FAILED != registerStatus)
    {
        /* Light sensor Service Register */
        registerStatus = SensorServicesRegisterLightSensor();
    }

    if (BLESTATUS_FAILED != registerStatus)
    {
        /* Noise sensor Service Register */
        registerStatus = SensorServicesRegisterNoiseSensor();
    }

    if (BLESTATUS_FAILED != registerStatus)
    {
        /* Magnetometer sensor Service Register */
        registerStatus = SensorServicesRegisterMagnetometerSensor();
    }

    if (BLESTATUS_FAILED != registerStatus)
    {
        /* Environment sensor Service Register */
        registerStatus = SensorServicesRegisterEnvironmentSensor();
    }

    if (BLESTATUS_FAILED != registerStatus)
    {
        /* High data rate Service Register */
        registerStatus = SensorServicesRegisterHighDataRateService();
    }

    if (BLESTATUS_FAILED != registerStatus)
    {
        /* Control Node Service Register */
        registerStatus = SensorServicesRegisterControlSensor();
    }

    if (BLESTATUS_SUCCESS == registerStatus)
    {
        return RETCODE_OK;
    }
    else
    {
        return RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_REGISTRATION_FAIL);
    }
}

/* The description is in the private header */
Retcode_T SensorServices_SendData(uint8_t* payload, uint8_t payloadLen, SensorServices_Info_T * sensorServiceInfo)
{
    Retcode_T retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
    /* check connection status */
    ConnectionHandle = BlePeripheral_GetConnectionHandle();
    if (0UL == ConnectionHandle)
    {
        /* if the remote device is not connected. */
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSORSERVICES_NO_DEVICE_PAIRED));
    }
    if ((NULL != payload) && (UINT8_C(20) >= payloadLen) && (UINT8_C(0) != payloadLen))
    {
        if (false == IsFirstSendPending)
        {
            vTaskDelayUntil(&BleLastWakeTimeUponConnect, pdMS_TO_TICKS(BLE_OS_DELAY_UPON_CONNECT));
            IsFirstSendPending = true;
        }
        switch ((SensorService_Type_T) sensorServiceInfo->sensorServicesType)
        {
        case BLE_SENSOR_ACCELEROMETER:
            retcode = WriteAndNotifyAccelerometerAxisValue(*(int16_t *) payload, (SensorServices_SensorAxisType_T) sensorServiceInfo->sensorServicesContent);
            break;
        case BLE_SENSOR_GYRO:
            retcode = WriteAndNotifyGyroAxisValue(*(int16_t *) payload, (SensorServices_SensorAxisType_T) sensorServiceInfo->sensorServicesContent);
            break;
        case BLE_SENSOR_LIGHT:
            retcode = WriteAndNotifyLightSensorValue(*(uint32_t *) payload);
            break;
        case BLE_SENSOR_NOISE:
            retcode = WriteAndNotifyNoiseSensorValue(*(uint32_t *) payload);
            break;
        case BLE_SENSOR_MAGNETOMETER:
            retcode = WriteAndNotifyMagnetometerValue(*(int16_t *) payload, (SensorServices_MagnetometerType_T) sensorServiceInfo->sensorServicesContent);
            break;
        case BLE_SENSOR_ENVIRONMENTAL:
            switch ((SensorServices_EnvironmentalType_T) sensorServiceInfo->sensorServicesContent)
            {
            case SENSOR_ENVIRONMENTAL_PRESSURE:
                retcode = WriteAndNotifyPressureSensorValue(*(uint32_t *) payload);
                break;
            case SENSOR_ENVIRONMENTAL_TEMPERATURE:
                retcode = WriteAndNotifyTemperatureSensorValue(*(int32_t *) payload);
                break;
            case SENSOR_ENVIRONMENTAL_HUMIDITY:
                retcode = WriteAndNotifyHumiditySensorValue(*(uint32_t *) payload);
                break;
            default:
                /* Do-not disturb retcode */
                break;
            }
            break;
        case BLE_HIGH_DATA_RATE:
            retcode = WriteAndNotifyPriorityData(payload, payloadLen, (SensorServices_HighDataRateType_T) sensorServiceInfo->sensorServicesContent);
            break;
        case BLE_CONTROL_NODE:
            /* Do-not disturb retcode */
            break;
        default:
            /* Do-not disturb retcode */
            break;
        }
    }
    return (retcode);
}

/* The description is in the private header */
void SensorServices_UpdateConnectionStatus(bool connectionStatus)
{
    if (!connectionStatus)
    {
        IsFirstSendPending = connectionStatus;
    }
    else
    {
        BleLastWakeTimeUponConnect = xTaskGetTickCount();
    }
}

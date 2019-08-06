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
 * @file
 *
 * This module handles the generic Sensor data sampling and reporting
 *
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_SENSOR

#if XDK_SENSOR_SENSOR

/* own header files */
#include "XDK_Sensor.h"

/* system header files */
#include <stdio.h>

/* additional interface header files */
#include "BSP_BoardType.h"
#include "BCDS_BSP_Board.h"
#include "BCDS_BSP_SensorNode.h"
#include "XDK_SensorHandle.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "XDK_NoiseSensor.h"

/* constant definitions ***************************************************** */

/* local variables ********************************************************** */

/**< Sensor setup information */
static Sensor_Setup_T SensorSetup;
#define UPPER_THRESHOLD_VALUE           UINT32_C(0x5a)      /**< upper threshold value */
#define LOWER_THRESHOLD_VALUE           UINT32_C(0x2a)      /**< lower threshold value */
#define THRESHOLD_TIMER_VALUE           UINT32_C(0X05)      /**< threshold timer value */
#define ADC_SAMPLING_FREQUENCY          (22050U)            /**< ADC sampling frequency in hertz (Hz)*/

/**
 * @brief   This will read the Accelerometer sensor data value.
 *
 * @param [in/out] accelData
 * Structure required to collect the XYZ axis data. User must provide memory.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T SensorAccelDataRead(Accelerometer_XyzData_T* accelData)
{
    Retcode_T retcode = RETCODE_OK;
    Accelerometer_HandlePtr_T accelHandle = XdkAccelerometer_BMA280_Handle; /* By default BMA280 is used */

    if (SENSOR_ACCEL_BMI160 == SensorSetup.Config.Accel.Type)
    {
        accelHandle = XdkAccelerometer_BMI160_Handle;
    }
    if (true == SensorSetup.Config.Accel.IsRawData)
    {
        retcode = Accelerometer_readXyzLsbValue(accelHandle, accelData);
    }
    else
    {
        retcode = Accelerometer_readXyzGValue(accelHandle, accelData);
    }
    return retcode;
}

/**
 * @brief   This will read the Gyroscope sensor data value.
 *
 * @param [in/out] gyroData
 * Structure required to collect the XYZ axis data. User must provide memory.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T SensorGyroDataRead(Gyroscope_XyzData_T* gyroData)
{
    Retcode_T retcode = RETCODE_OK;

    Gyroscope_HandlePtr_T gyroHandle = XdkGyroscope_BMG160_Handle; /* By default BMG160 is used */

    if (SENSOR_GYRO_BMI160 == SensorSetup.Config.Gyro.Type)
    {
        gyroHandle = XdkGyroscope_BMI160_Handle;
    }
    if (true == SensorSetup.Config.Gyro.IsRawData)
    {
        retcode = Gyroscope_readXyzValue(gyroHandle, gyroData);
    }
    else
    {
        retcode = Gyroscope_readXyzDegreeValue(gyroHandle, gyroData);
    }

    return retcode;
}

/**
 * @brief   This will read the Light sensor data value.
 *
 * @param [in/out] milliLuxData
 * Instance to hold sensor data in milli lux. User must provide memory.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T SensorLightDataRead(uint32_t* milliLuxData)
{
    Retcode_T retcode = RETCODE_OK;

    /* read sensor data in milli lux*/
    retcode = LightSensor_readLuxData(XdkLightSensor_MAX44009_Handle, milliLuxData);

    return retcode;
}

/**
 * @brief   This will read the Magnetometer sensor data value.
 *
 * @param [in/out] magData
 * Structure required to collect the XYZ axis data. User must provide memory.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T SensorMagDataRead(Magnetometer_XyzData_T* magData)
{
    Retcode_T retcode = RETCODE_OK;

    /* read magnetometer sensor data*/
    if (true == SensorSetup.Config.Mag.IsRawData)
    {
        retcode = Magnetometer_readXyzLsbData(XdkMagnetometer_BMM150_Handle, magData);
    }
    else
    {
        retcode = Magnetometer_readXyzTeslaData(XdkMagnetometer_BMM150_Handle, magData);
    }

    return retcode;
}

/**
 * @brief   This will read the Environmental sensor data value.
 *
 * @param [in/out] value
 * Structure required to for Environmental sensor data in standard units. User must provide memory.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T SensorEnvironmentalDataRead(Environmental_Data_T* value)
{
    Retcode_T retcode = RETCODE_OK;

    retcode = Environmental_readData(XdkEnvironmental_BME280_Handle, value);

    if (RETCODE_OK == retcode)
    {
        retcode = Environmental_compensateData(XdkEnvironmental_BME280_Handle, value);
    }

    return retcode;
}

/**
 * @brief   This will validate if atleast one sensor was setup and enabled.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T SensorSetupValidity(void)
{
    Retcode_T retcode = RETCODE_OK;

    if (!(SensorSetup.Enable.Accel || SensorSetup.Enable.Mag ||
            SensorSetup.Enable.Gyro || SensorSetup.Enable.Humidity ||
            SensorSetup.Enable.Temp || SensorSetup.Enable.Pressure ||
            SensorSetup.Enable.Light || SensorSetup.Enable.Noise))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSOR_NONE_IS_ENABLED);
    }
    return retcode;
}

static void AccelRealTimeISRCallback(void)
{
    Retcode_T retcode = RETCODE_OK;
    retcode = CmdProcessor_EnqueueFromIsr(SensorSetup.CmdProcessorHandle, SensorSetup.Config.Accel.Callback, NULL, 0UL);
    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseError(retcode);
    }
}

static void LightSensorCallback(void *param1, uint32_t param2)
{
    BCDS_UNUSED(param2);
    Retcode_T retcode = RETCODE_OK;
    LightSensor_ConfigInterrupt_T status = LIGHTSENSOR_CONFIG_INTERRUPT_OUT_OF_RANGE;

    retcode = LightSensor_getInterruptStatus(XdkLightSensor_MAX44009_Handle, (LightSensor_ConfigInterruptPtr_T) &status);
    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseError(retcode);
    }
    else
    {
        SensorSetup.Config.Light.Callback(param1, (uint32_t) status);
    }
}

static void LightRealTimeISRCallback(void)
{
    Retcode_T retcode = RETCODE_OK;
    retcode = CmdProcessor_EnqueueFromIsr(SensorSetup.CmdProcessorHandle, LightSensorCallback, NULL, 0UL);
    if (retcode != RETCODE_OK)
    {
        Retcode_RaiseError(retcode);
    }
}

/** Refer interface header for description */
Retcode_T Sensor_Setup(Sensor_Setup_T * setup)
{
    Retcode_T retcode = RETCODE_OK;

    if (NULL == setup)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        if ((NULL == setup->CmdProcessorHandle) &&
                (setup->Enable.Accel) &&
                (setup->Config.Accel.IsInteruptEnabled) &&
                (SENSOR_ACCEL_BMA280 != setup->Config.Accel.Type) &&
                (NULL == setup->Config.Accel.Callback))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
        }
        if ((RETCODE_OK == retcode) && ((setup->Config.Light.IsInteruptEnabled) &&
                (NULL == setup->Config.Light.Callback)))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
        }
        if (setup->Enable.Noise)
        {
            retcode = NoiseSensor_Setup(ADC_SAMPLING_FREQUENCY);
        }
        if (RETCODE_OK == retcode)
        {
            SensorSetup = *setup;
        }
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T Sensor_Enable(void)
{
    Retcode_T retcode = RETCODE_OK;
    Accelerometer_ConfigSlopeIntr_T slopeInterruptConfig;
    retcode = SensorSetupValidity();

    if (SensorSetup.Enable.Accel)
    {
        if (RETCODE_OK == retcode)
        {
            if (SENSOR_ACCEL_BMI160 == SensorSetup.Config.Accel.Type)
            {
                retcode = Accelerometer_init(XdkAccelerometer_BMI160_Handle);
            }
            else
            {
                retcode = Accelerometer_init(XdkAccelerometer_BMA280_Handle);
                if ((RETCODE_OK == retcode) && (SensorSetup.Config.Accel.IsInteruptEnabled))
                {
                    retcode = Accelerometer_regRealTimeCallback(XdkAccelerometer_BMA280_Handle, ACCELEROMETER_BMA280_INTERRUPT_CHANNEL1, AccelRealTimeISRCallback);
                    if (RETCODE_OK == retcode)
                    {
                        /*Configure interrupt conditions*/
                        slopeInterruptConfig.slopeDuration = ACCELEROMETER_BMA280_SLOPE_DURATION4;
                        slopeInterruptConfig.slopeThreshold = UINT8_C(100);
                        slopeInterruptConfig.slopeEnableX = INT8_C(0);
                        slopeInterruptConfig.slopeEnableY = INT8_C(1);
                        slopeInterruptConfig.slopeEnableZ = INT8_C(0);

                        retcode = Accelerometer_configInterrupt(XdkAccelerometer_BMA280_Handle, ACCELEROMETER_BMA280_INTERRUPT_CHANNEL1, ACCELEROMETER_BMA280_SLOPE_INTERRUPT, &slopeInterruptConfig);
                    }
                }
            }
        }
    }
    if (SensorSetup.Enable.Mag)
    {
        if (RETCODE_OK == retcode)
        {
            retcode = Magnetometer_init(XdkMagnetometer_BMM150_Handle);
        }
        if (RETCODE_OK == retcode)
        {
            retcode = Magnetometer_setPowerMode(XdkMagnetometer_BMM150_Handle, MAGNETOMETER_BMM150_POWERMODE_NORMAL);
        }
    }

    if (SensorSetup.Enable.Gyro)
    {
        if (RETCODE_OK == retcode)
        {
            if (SENSOR_GYRO_BMI160 == SensorSetup.Config.Gyro.Type)
            {
                retcode = Gyroscope_init(XdkGyroscope_BMI160_Handle);
            }
            else
            {
                retcode = Gyroscope_init(XdkGyroscope_BMG160_Handle);
                if (RETCODE_OK == retcode)
                {
                    retcode = Gyroscope_setBandwidth(XdkGyroscope_BMG160_Handle,GYROSCOPE_BMG160_BANDWIDTH_47HZ);
                }
            }
        }
    }

    if (SensorSetup.Enable.Temp || SensorSetup.Enable.Pressure || SensorSetup.Enable.Humidity)
    {
        if (RETCODE_OK == retcode)
        {
            retcode = Environmental_init(XdkEnvironmental_BME280_Handle);
        }
        if (RETCODE_OK == retcode)
        {
            retcode = Environmental_setTemperatureOffset(XdkEnvironmental_BME280_Handle, SensorSetup.Config.Temp.OffsetCorrection);
        }
    }

    if (SensorSetup.Enable.Light)
    {
        if (RETCODE_OK == retcode)
        {
            retcode = LightSensor_init(XdkLightSensor_MAX44009_Handle);
        }
        if (RETCODE_OK == retcode)
        {
            retcode = LightSensor_setManualMode(XdkLightSensor_MAX44009_Handle, LIGHTSENSOR_MAX44009_ENABLE_MODE);
        }
        /* @todo - Validate if the below add-on configurations are needed. */
        if (RETCODE_OK == retcode)
        {
            retcode = LightSensor_setIntegrationTime(XdkLightSensor_MAX44009_Handle, LIGHTSENSOR_100MS);
        }
        if (RETCODE_OK == retcode)
        {
            retcode = LightSensor_setBrightness(XdkLightSensor_MAX44009_Handle, LIGHTSENSOR_HIGH_BRIGHTNESS);
        }
        if (RETCODE_OK == retcode)
        {
            retcode = LightSensor_setContinuousMode(XdkLightSensor_MAX44009_Handle, LIGHTSENSOR_MAX44009_ENABLE_MODE);
        }
        if ((RETCODE_OK == retcode) && (SensorSetup.Config.Light.IsInteruptEnabled))
        {
            retcode = LightSensor_registerRealTimeCallback(XdkLightSensor_MAX44009_Handle, LightRealTimeISRCallback);
            if (RETCODE_OK == retcode)
            {
                /* Configure interrupt conditions */
                retcode = LightSensor_configureThresholdInterrupt(XdkLightSensor_MAX44009_Handle, UPPER_THRESHOLD_VALUE, LOWER_THRESHOLD_VALUE, THRESHOLD_TIMER_VALUE);
            }
        }
    }

    if (SensorSetup.Enable.Noise)
    {
        if (RETCODE_OK == retcode)
        {
            retcode = NoiseSensor_Enable();
        }
    }

    return retcode;
}

/** Refer interface header for description */
Retcode_T Sensor_GetData(Sensor_Value_T * value)
{
    Retcode_T retcode = RETCODE_OK;
    Sensor_Value_T sensorValueTemporary;
    Accelerometer_XyzData_T accelData = { INT16_C(0), INT16_C(0), INT16_C(0) };
    Gyroscope_XyzData_T gyroData = { INT16_C(0), INT16_C(0), INT16_C(0) };
    Magnetometer_XyzData_T magData = { INT32_C(0), INT32_C(0), INT32_C(0), UINT16_C(0) };
    Environmental_Data_T environmentData = { INT32_C(0), UINT32_C(0), UINT32_C(0) };

    if (NULL == value)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    memset(&sensorValueTemporary, 0x00, sizeof(sensorValueTemporary));
    retcode = SensorSetupValidity();

    if ((RETCODE_OK == retcode) && SensorSetup.Enable.Accel)
    {
        retcode = SensorAccelDataRead(&accelData);

        if (RETCODE_OK == retcode)
        {
            sensorValueTemporary.Accel.X = accelData.xAxisData;
            sensorValueTemporary.Accel.Y = accelData.yAxisData;
            sensorValueTemporary.Accel.Z = accelData.zAxisData;
        }
    }

    if ((RETCODE_OK == retcode) && SensorSetup.Enable.Gyro)
    {
        retcode = SensorGyroDataRead(&gyroData);

        if (RETCODE_OK == retcode)
        {
            sensorValueTemporary.Gyro.X = gyroData.xAxisData;
            sensorValueTemporary.Gyro.Y = gyroData.yAxisData;
            sensorValueTemporary.Gyro.Z = gyroData.zAxisData;
        }
    }

    if ((RETCODE_OK == retcode) && SensorSetup.Enable.Mag)
    {
        retcode = SensorMagDataRead(&magData);

        if (RETCODE_OK == retcode)
        {
            sensorValueTemporary.Mag.X = magData.xAxisData;
            sensorValueTemporary.Mag.Y = magData.yAxisData;
            sensorValueTemporary.Mag.Z = magData.zAxisData;
            sensorValueTemporary.Mag.R = magData.resistance;
        }

    }

    if ((RETCODE_OK == retcode) && (SensorSetup.Enable.Humidity || SensorSetup.Enable.Temp || SensorSetup.Enable.Pressure))
    {
        retcode = SensorEnvironmentalDataRead(&environmentData);

        if (RETCODE_OK == retcode)
        {
            if (SensorSetup.Enable.Humidity)
            {
                sensorValueTemporary.RH = environmentData.humidity;
            }
            if (SensorSetup.Enable.Temp)
            {
                sensorValueTemporary.Temp = environmentData.temperature;
            }
            if (SensorSetup.Enable.Pressure)
            {
                sensorValueTemporary.Pressure = environmentData.pressure;
            }
        }
    }

    if ((RETCODE_OK == retcode) && SensorSetup.Enable.Light)
    {
        /* read sensor data in milli lux */
        uint32_t milliLuxData = UINT32_C(0);

        retcode = SensorLightDataRead(&milliLuxData);
        if (RETCODE_OK == retcode)
        {
            sensorValueTemporary.Light = milliLuxData;
        }
    }

    if ((RETCODE_OK == retcode) && SensorSetup.Enable.Noise)
    {
        float noiseData = 0.0f;
        retcode = NoiseSensor_ReadRmsValue(&noiseData, 10U);
        if (RETCODE_OK == retcode)
        {
            sensorValueTemporary.Noise = noiseData;
        }
    }

    if (RETCODE_OK == retcode)
    {
        *value = sensorValueTemporary;
    }
    return retcode;
}

#endif /* XDK_SENSOR_SENSOR */

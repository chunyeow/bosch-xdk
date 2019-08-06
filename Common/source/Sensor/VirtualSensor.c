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
#define BCDS_MODULE_ID XDK_COMMON_ID_VIRTUALSENSOR

#if XDK_SENSOR_VIRTUALSENSOR
/* own header files */
#include "XDK_VirtualSensor.h"

/* system header files */
#include <stdio.h>


/* additional interface header files */
#include "BSP_BoardType.h"
#include "XDK_SensorHandle.h"
#include "BCDS_LinearAcceleration.h"
#include "FreeRTOS.h"
#include "timers.h"

/* constant definitions ***************************************************** */

/* local variables ********************************************************** */

/**< Sensor setup information */
static VirtualSensor_Setup_T VirtualSensorSetup;

Retcode_T VirtualSensor_Setup(VirtualSensor_Setup_T * setup)
{
    Retcode_T retcode = RETCODE_OK;
    if (NULL == setup)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }

    VirtualSensorSetup = *setup;
    if (VirtualSensorSetup.Enable.Rotation)
    {
        retcode = Rotation_init(XdkRotation_VirtualSensor_Handle);
    }
    else if (VirtualSensorSetup.Enable.Compass)
    {
        retcode = Orientation_init(XdkOrientation_VirtualSensor_Handle);
    }
    else if (VirtualSensorSetup.Enable.AbsoluteHumidity)
    {
        retcode = AbsoluteHumidity_init(XdkAbsoluteHumidity_ClbrSensor_Handle);
    }
    else if (VirtualSensorSetup.Enable.CalibratedAccel)
    {
        retcode = CalibratedAccel_init(XdkAccelerometer_ClbrSensor_Handle);
    }
    else if (VirtualSensorSetup.Enable.CalibratedGyro)
    {
        retcode = CalibratedGyro_init(XdkGyroscope_ClbrSensor_Handle);
    }
    else if (VirtualSensorSetup.Enable.CalibratedMag)
    {
        retcode = CalibratedMag_init(XdkMagnetometer_ClbrSensor_Handle);
    }
    else if (VirtualSensorSetup.Enable.Gravity)
    {
        retcode = Gravity_init(XdkGravity_VirtualSensor_Handle);
    }
    else if (VirtualSensorSetup.Enable.StepCounter)
    {
        retcode = StepCounter_init(XdkStepCounter_VirtualSensor_Handle);
    }
    else if (VirtualSensorSetup.Enable.FingerPrint)
    {
        retcode = FingerPrint_init(XdkFingerprint_VirtualSensor_Handle);
    }
    else if (VirtualSensorSetup.Enable.LinearAccel)
    {
        retcode = LinearAcceleration_init(XdkLinearAcceleration_ClbrSensor_Handle);
    }
    else if (VirtualSensorSetup.Enable.Gesture)
    {
        retcode = Gestures_init(XdkGesture_VirtualSensor_Handle);
    }
    else
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
    }
    return retcode;
}

Retcode_T VirtualSensor_Enable(void)
{
    return RETCODE_OK;
}

Retcode_T VirtualSensor_Disable(void)
{
    return RETCODE_OK;
}

Retcode_T VirtualSensor_Close(void)
{
    Retcode_T retcode = RETCODE_OK;

    if (VirtualSensorSetup.Enable.Rotation)
    {
        retcode = Rotation_deInit();
    }
    else if (VirtualSensorSetup.Enable.Compass)
    {
        retcode = Orientation_deInit();
    }
    else if (VirtualSensorSetup.Enable.AbsoluteHumidity)
    {
        retcode = AbsoluteHumidity_deInit();
    }
    else if (VirtualSensorSetup.Enable.CalibratedAccel)
    {
        retcode = CalibratedAccel_deInit();
    }
    else if (VirtualSensorSetup.Enable.CalibratedGyro)
    {
        retcode = CalibratedGyro_deInit();
    }
    else if (VirtualSensorSetup.Enable.CalibratedMag)
    {
        retcode = CalibratedMag_deInit();
    }
    else if (VirtualSensorSetup.Enable.Gravity)
    {
        retcode = Gravity_deInit();
    }
    else if (VirtualSensorSetup.Enable.StepCounter)
    {
        retcode = StepCounter_deInit(XdkStepCounter_VirtualSensor_Handle);
    }
    else if (VirtualSensorSetup.Enable.FingerPrint)
    {
        retcode = FingerPrint_deInit();
    }
    else if (VirtualSensorSetup.Enable.LinearAccel)
    {
        retcode = LinearAcceleration_deInit();
    }
    else if (VirtualSensorSetup.Enable.Gesture)
    {
        retcode = Gestures_deInit(XdkGesture_VirtualSensor_Handle);
    }

    return retcode;
}

Retcode_T VirtualSensor_GetRotationData(VirtualSensor_DataType_T *data, VirtualSensor_RotationMode_T mode)
{
    Retcode_T retcode = RETCODE_OK;

    if (NULL == data)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    switch (mode)
    {
    case ROTATION_QUATERNION:
        ;
        Rotation_QuaternionData_T quaternionValue;
        retcode = Rotation_readQuaternionValue(&quaternionValue);
        data->RotationQuaternion.W = quaternionValue.w;
        data->RotationQuaternion.X = quaternionValue.x;
        data->RotationQuaternion.Y = quaternionValue.y;
        data->RotationQuaternion.Z = quaternionValue.z;
        break;
    case ROTATION_EULER_DEG:
        case ROTATION_EULER_RAD:
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NOT_SUPPORTED);
        break;
    default:
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
    }
    return retcode;
}

Retcode_T VirtualSensor_GetCalibratedAccel(VirtualSensor_DataType_T *data, VirtualSensor_AccelMode_T mode)
{
    Retcode_T retcode;
    if (NULL == data)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    switch (mode)
    {
    case ACCEL_LSB_MODE:
        ;
        CalibratedAccel_XyzLsbData_T calibAccelLsbData;

        retcode = CalibratedAccel_readXyzLsbValue(&calibAccelLsbData);
        data->AxisInteger.X = calibAccelLsbData.xAxisData;
        data->AxisInteger.Y = calibAccelLsbData.yAxisData;
        data->AxisInteger.Z = calibAccelLsbData.zAxisData;
        break;
    case ACCEL_MPS2_MODE:
        ;
        CalibratedAccel_XyzMps2Data_T calibAccelMps2Data;

        retcode = CalibratedAccel_readXyzMps2Value(&calibAccelMps2Data);
        data->AxisFloat.X = calibAccelMps2Data.xAxisData;
        data->AxisFloat.Y = calibAccelMps2Data.yAxisData;
        data->AxisFloat.Z = calibAccelMps2Data.zAxisData;
        break;
    case ACCEL_G_MODE:
        ;
        CalibratedAccel_XyzGData_T calibAccelGData;

        retcode = CalibratedAccel_readXyzGValue(&calibAccelGData);
        data->AxisFloat.X = calibAccelGData.xAxisData;
        data->AxisFloat.Y = calibAccelGData.yAxisData;
        data->AxisFloat.Z = calibAccelGData.zAxisData;
        break;
    default:
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
    }
    return retcode;
}

Retcode_T VirtualSensor_GetCalibratedGyro(VirtualSensor_DataType_T *data, VirtualSensor_GyroMode_T mode)
{
    Retcode_T retcode;
    if (NULL == data)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    switch (mode)
    {
    case GYRO_LSB_MODE:
        ;
        CalibratedGyro_XyzLsbData_T calibGyroLsbData;

        retcode = CalibratedGyro_readXyzLsbValue(&calibGyroLsbData);
        data->AxisInteger.X = calibGyroLsbData.xAxisData;
        data->AxisInteger.Y = calibGyroLsbData.yAxisData;
        data->AxisInteger.Z = calibGyroLsbData.zAxisData;
        break;
    case GYRO_RPS_MODE:
        ;
        CalibratedGyro_XyzRpsData_T calibGyroRpsData;

        retcode = CalibratedGyro_readXyzRpsValue(&calibGyroRpsData);
        data->AxisFloat.X = calibGyroRpsData.xAxisData;
        data->AxisFloat.Y = calibGyroRpsData.yAxisData;
        data->AxisFloat.Z = calibGyroRpsData.zAxisData;
        break;
    case GYRO_DPS_MODE:
        ;
        CalibratedGyro_XyzDpsData_T calibGyroDpsData;

        retcode = CalibratedGyro_readXyzDpsValue(&calibGyroDpsData);
        data->AxisFloat.X = calibGyroDpsData.xAxisData;
        data->AxisFloat.Y = calibGyroDpsData.yAxisData;
        data->AxisFloat.Z = calibGyroDpsData.zAxisData;
        break;
    default:
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
    }
    return retcode;
}

Retcode_T VirtualSensor_GetCalibratedMag(VirtualSensor_DataType_T *data, VirtualSensor_MagMode_T mode)
{
    Retcode_T retcode;
    if (NULL == data)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    switch (mode)
    {
    case MAG_LSB_MODE:
        ;
        CalibratedMag_XyzLsbData_T calibMagLsbData;

        retcode = CalibratedMag_readXyzLsbVal(&calibMagLsbData);
        data->AxisInteger.X = calibMagLsbData.xAxisData;
        data->AxisInteger.Y = calibMagLsbData.yAxisData;
        data->AxisInteger.Z = calibMagLsbData.zAxisData;
        break;
    case MAG_GAUSS_MODE:
        ;
        CalibratedMag_XyzGaussData_T calibMagGaussData;

        retcode = CalibratedMag_readXyzGaussVal(&calibMagGaussData);
        data->AxisFloat.X = calibMagGaussData.xAxisData;
        data->AxisFloat.Y = calibMagGaussData.yAxisData;
        data->AxisFloat.Z = calibMagGaussData.zAxisData;
        break;
    case MAG_MICROTESLA_MODE:
        ;
        CalibratedMag_XyzMicroTesla_T calibMagMicroTeslaData;

        retcode = CalibratedMag_readMicroTeslaVal(&calibMagMicroTeslaData);
        data->AxisFloat.X = calibMagMicroTeslaData.xAxisData;
        data->AxisFloat.Y = calibMagMicroTeslaData.yAxisData;
        data->AxisFloat.Z = calibMagMicroTeslaData.zAxisData;
        break;
    default:
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
    }
    return retcode;
}

Retcode_T VirtualSensor_GetAbsoluteHumidityData(float *humidity)
{
    Retcode_T retcode = RETCODE_OK;
    if (NULL == humidity)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    retcode = AbsoluteHumidity_readValue(XdkAbsoluteHumidity_ClbrSensor_Handle, humidity);
    return retcode;
}

Retcode_T VirtualSensor_GetGravity(VirtualSensor_DataType_T *data, VirtualSensor_GravityMode_T mode)
{
    Retcode_T retcode;
    if (NULL == data)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    switch (mode)
    {
    case GRAVITY_LSB_MODE:
        ;
        Gravity_XyzLsbData_T gravityLsbData;

        retcode = Gravity_readXyzLsbValue(&gravityLsbData);
        data->AxisInteger.X = gravityLsbData.xAxisData;
        data->AxisInteger.Y = gravityLsbData.yAxisData;
        data->AxisInteger.Z = gravityLsbData.zAxisData;
        break;
    case GRAVITY_MPS2_MODE:
        ;
        Gravity_XyzMps2Data_T gravityMps2Data;

        retcode = Gravity_readXyzMps2Value(&gravityMps2Data);
        data->AxisFloat.X = gravityMps2Data.xAxisData;
        data->AxisFloat.Y = gravityMps2Data.yAxisData;
        data->AxisFloat.Z = gravityMps2Data.zAxisData;
        break;
    case GRAVITY_G_MODE:
        ;
        Gravity_XyzGData_T gravityGData;

        retcode = Gravity_readXyzGValue(&gravityGData);
        data->AxisFloat.X = gravityGData.xAxisData;
        data->AxisFloat.Y = gravityGData.yAxisData;
        data->AxisFloat.Z = gravityGData.zAxisData;
        break;
    default:
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
    }
    return retcode;
}

Retcode_T VirtualSensor_GetStepCounter(int16_t *stepCounter)
{
    Retcode_T retcode;
    if (NULL == stepCounter)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    retcode = StepCounter_read(XdkStepCounter_VirtualSensor_Handle, stepCounter);
    return retcode;
}

Retcode_T VirtualSensor_GetCompassData(float *compassPtr)
{
    Retcode_T retcode;
    Orientation_EulerData_T compassData;
    if (NULL == compassPtr)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    compassData.heading = 0.0f;
    retcode = Orientation_readEulerDegreeVal(&compassData);
    *compassPtr = compassData.heading;

    return retcode;
}

Retcode_T VirtualSensor_GetLinearAccel(VirtualSensor_DataType_T *data, VirtualSensor_AccelMode_T mode)
{
    Retcode_T retcode;
    if (NULL == data)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    switch (mode)
    {
    case ACCEL_LSB_MODE:
        ;
        LinearAcceleration_XyzLsbData_T linearAccelLsbData;

        retcode = LinearAcceleration_readXyzLsbValue(&linearAccelLsbData);
        data->AxisInteger.X = linearAccelLsbData.xAxisData;
        data->AxisInteger.Y = linearAccelLsbData.yAxisData;
        data->AxisInteger.Z = linearAccelLsbData.zAxisData;
        break;
    case ACCEL_MPS2_MODE:
        ;
        LinearAcceleration_XyzMps2Data_T linearAccelMps2Data;

        retcode = LinearAcceleration_readXyzMps2Value(&linearAccelMps2Data);
        data->AxisFloat.X = linearAccelMps2Data.xAxisData;
        data->AxisFloat.Y = linearAccelMps2Data.yAxisData;
        data->AxisFloat.Z = linearAccelMps2Data.zAxisData;
        break;
    case ACCEL_G_MODE:
        ;
        LinearAcceleration_XyzMps2Data_T linearAccelGData = { 0.0, 0.0, 0.0 };

        retcode = LinearAcceleration_readXyzMps2Value(&linearAccelGData);
        data->AxisFloat.X = linearAccelGData.xAxisData;
        data->AxisFloat.Y = linearAccelGData.yAxisData;
        data->AxisFloat.Z = linearAccelGData.zAxisData;
        break;
    default:
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
    }
    return retcode;
}

Retcode_T VirtualSensor_GetGestureCount(uint32_t *gestureCount)
{
    Retcode_T retcode = RETCODE_OK;
    if (NULL == gestureCount)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    retcode = Gestures_read(XdkGesture_VirtualSensor_Handle, gestureCount);
    return retcode;
}

Retcode_T VirtualSensor_SetFingerPrintValue(VirtualSensor_FingerPrintNumber_T num)
{
    return FingerPrint_setValue((FingerPrint_Number_T) num);
}

Retcode_T VirtualSensor_ResetFingerPrintValue(VirtualSensor_FingerPrintNumber_T num)
{
    return FingerPrint_resetValue((FingerPrint_Number_T) num);
}

Retcode_T VirtualSensor_CheckFingerPrintStoredValue(VirtualSensor_FingerPrintNumber_T num, VirtualSensor_FingerPrintStorageState_T *status)
{
    return FingerPrint_checkStoredValue((FingerPrint_Number_T) num, (FingerPrint_StorageStatusPtr_T) status);
}

Retcode_T VirtualSensor_MonitorFingerPrint(VirtualSensor_DataType_T *result)
{
    Retcode_T retcode = RETCODE_OK;
    FingerPrint_Monitor_Data_T data[FINGERPRINT_REF_MAX];

    retcode = FingerPrint_monitoring((FingerPrint_Monitor_DataPtr_T) data);

    for (uint32_t index = (uint32_t) FINGERPRINT_REF_VAL_1; index < (uint32_t) FINGERPRINT_REF_MAX; index++)
    {
        result[index].FingerPrintMonitorData.AngleMatching = data[index].fpAngleMatching;
        result[index].FingerPrintMonitorData.DistanceMatching = data[index].fpDistanceMatching;
    }

    return retcode;
}

#endif /* XDK_SENSOR_VIRTUALSENSOR */

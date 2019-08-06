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
#define BCDS_MODULE_ID XDK_COMMON_ID_EXTERNAL_SENSOR

#if XDK_SENSOR_EXTERNALSENSOR

/* own header files */
#include "XDK_ExternalSensor.h"
#include "BSP_LemSensor.h"

/* system header files */
#include <stdio.h>

/* additional interface header files */
#include "BSP_BoardType.h"
#include "BSP_LemSensor.h"
#include "BCDS_BSP_Board.h"
#include "BCDS_BSP_SensorNode.h"
#include "BCDS_Max44009.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "XDK_Lem.h"
#include "BCDS_Max31865.h"
#include "BCDS_BSP_Max31865.h"

/* constant definitions ***************************************************** */

/* local variables ********************************************************** */

/**< Sensor setup information */
static ExternalSensor_Setup_T SensorSetup;

/**
 * @brief   This will validate if atleast one sensor was setup and enabled.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T SensorSetupValidity(void)
{
    Retcode_T retcode = RETCODE_OK;

    if (!(SensorSetup.Enable.LemCurrent || SensorSetup.Enable.LemVoltage ||
                    SensorSetup.Enable.MaxTemp || SensorSetup.Enable.MaxResistance))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SENSOR_NONE_IS_ENABLED);
    }
    return retcode;
}
/**
 * @brief To setup the necessary modules for the max31865
 *
 * @retval RETCODE_OK in case off success and error code otherwise
 */
static Retcode_T Max31865_Setup(void)
{
    Retcode_T retcode = RETCODE_OK;
    switch (SensorSetup.Max31865Config.TempType)
    {
        case SENSOR_TEMPERATURE_PT100:
        retcode = MAX31865_SetSensorType(TEMPERATURE_SENSOR_PT100);
        break;
        case SENSOR_TEMPERATURE_PT1000:
        retcode = MAX31865_SetSensorType(TEMPERATURE_SENSOR_PT1000);
        break;
        default:
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
        break;
    }
    return retcode;
}

/**
 * @brief To enable the necessary modules for the max31865
 *
 * @retval RETCODE_OK in case off success and error code otherwise
 */
static Retcode_T Max31865_Enable(void)
{
    return MAX31865_Connect();
}

/** Refer interface header for description */
Retcode_T ExternalSensor_Setup(ExternalSensor_Setup_T * setup)
{
    Retcode_T retcode = RETCODE_OK;

    if (NULL == setup)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        if (NULL == setup->CmdProcessorHandle)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
        }
        if (RETCODE_OK == retcode)
        {
            if ((setup->Enable.MaxTemp) || (setup->Enable.MaxResistance))
            {
                retcode = Max31865_Setup();
            }
            if ((setup->Enable.LemVoltage) || (setup->Enable.LemCurrent))
            {
                retcode = Lem_Setup(setup->LemConfig.CurrentRatedTransformationRatio);
            }
        }
        if (RETCODE_OK == retcode)
        {
            SensorSetup = *setup;
        }
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T ExternalSensor_Enable(void)
{
    Retcode_T retcode = RETCODE_OK;
    retcode = SensorSetupValidity();

    if ((SensorSetup.Enable.LemCurrent) || (SensorSetup.Enable.LemVoltage))
    {
        if (RETCODE_OK == retcode)
        {
            retcode = Lem_Enable();
        }
    }

    if ((SensorSetup.Enable.MaxTemp) || (SensorSetup.Enable.MaxResistance))
    {
        if (RETCODE_OK == retcode)
        {
            retcode = Max31865_Enable();
        }
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T ExternalSensor_GetData(ExternalSensor_Value_T * value)
{
    Retcode_T retcode = RETCODE_OK;
    ExternalSensor_Value_T sensorValueTemporary =
    {   0, 0, 0, 0};

    if (NULL == value)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }

    retcode = SensorSetupValidity();

    if ((RETCODE_OK == retcode) && (SensorSetup.Enable.LemVoltage))
    {
        float rmsVoltage;

        retcode = Lem_GetRmsVoltage(&rmsVoltage);
        if (RETCODE_OK == retcode)
        {
            if (SensorSetup.Enable.LemVoltage)
            {
                sensorValueTemporary.XLemV = rmsVoltage;
            }
        }
    }
    if ((RETCODE_OK == retcode) && (SensorSetup.Enable.LemCurrent))
    {
        float rmsCurrent;

        retcode = Lem_GetRmsCurrent(&rmsCurrent);
        if (RETCODE_OK == retcode)
        {
            if (SensorSetup.Enable.LemCurrent)
            {
                sensorValueTemporary.XLemI = rmsCurrent;
            }
        }
    }

    if ((RETCODE_OK == retcode) && SensorSetup.Enable.MaxTemp)
    {
        float externalSensorData = 0;
        retcode = MAX31865_ReadTemperature(&externalSensorData);
        if (RETCODE_OK == retcode)
        {
            sensorValueTemporary.XMaxT = externalSensorData;
        }
    }

    if ((RETCODE_OK == retcode) && SensorSetup.Enable.MaxResistance)
    {
        float resistance;
        retcode = MAX31865_ReadResistance(&resistance);
        if (RETCODE_OK == retcode)
        {
            sensorValueTemporary.XMaxR = resistance;
        }
    }

    if (RETCODE_OK == retcode)
    {
        *value = sensorValueTemporary;
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T ExternalSensor_GetLemData(float * xLemI, float * xLemV)
{
    Retcode_T retcode = RETCODE_OK;

    float lemVolt = 0.0f;
    float lemCurrent = 0.0f;

    if ((NULL == xLemI) && (NULL == xLemV))
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    retcode = SensorSetupValidity();
    if (RETCODE_OK == retcode)
    {
        if ((SensorSetup.Enable.LemCurrent && (NULL == xLemI)) || (SensorSetup.Enable.LemVoltage && (NULL == xLemV)))
        {
            retcode = (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_UNINITIALIZED));
        }
    }
    if (RETCODE_OK == retcode)
    {
        retcode = Lem_GetRmsVoltage(&lemVolt);
    }
    if (RETCODE_OK == retcode)
    {
        if (true == SensorSetup.Enable.LemVoltage)
        {
            *xLemV = lemVolt;
        }
    }
    if (RETCODE_OK == retcode)
    {
        retcode = Lem_GetRmsCurrent(&lemCurrent);
    }
    if (RETCODE_OK == retcode)
    {
        if ((true == SensorSetup.Enable.LemCurrent))
        {
            *xLemI = lemCurrent;
        }
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T ExternalSensor_GetMax31865Data(float * xMaxT, float * xMaxR)
{
    Retcode_T retcode = RETCODE_OK;
    if ((NULL == xMaxT) && (NULL == xMaxR))
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    retcode = SensorSetupValidity();
    if (RETCODE_OK == retcode)
    {
        if ((SensorSetup.Enable.MaxTemp && (NULL == xMaxT)) || (SensorSetup.Enable.MaxResistance && (NULL == xMaxR)))
        {
            retcode = (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_UNINITIALIZED));
        }
    }
    if ((RETCODE_OK == retcode) && (NULL != xMaxT))
    {
        retcode = MAX31865_ReadTemperature(xMaxT);
    }
    if ((RETCODE_OK == retcode) && (NULL != xMaxR))
    {
        retcode = MAX31865_ReadResistance(xMaxR);
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T ExternalSensor_IsAvailable(ExternalSensor_Target_T sensor, bool * status)
{
    Retcode_T retcode = RETCODE_OK;
    uint8_t isConnected;
    if (NULL == status)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        switch (sensor)
        {
            case XDK_EXTERNAL_LEM:
            retcode = BSP_LemSensor_GetStatus(&isConnected);
            *status = (bool) isConnected;
            break;
            case XDK_EXTERNAL_MAX31865:
            retcode = BSP_Max31865_GetStatus(&isConnected);
            *status = (bool) isConnected;
            break;
            default:
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
            break;
        }
    }
    return (retcode);
}

Retcode_T ExternalSensor_HwStatusPinInit(void)
{
    Retcode_T retcode = RETCODE_OK;
    retcode = BSP_ExtensionPort_ConnectGpio(LEM_SENSOR_CONNECT_STATUS_PIN1);
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_ExtensionPort_ConnectGpio(LEM_SENSOR_CONNECT_STATUS_PIN2);
    }
    retcode = BSP_ExtensionPort_SetGpioConfig(LEM_SENSOR_CONNECT_STATUS_PIN1, BSP_EXTENSIONPORT_GPIO_PINMODE, (uint32_t) BSP_EXTENSIONPORT_INPUT_NOPULL, NULL);
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_ExtensionPort_SetGpioConfig(LEM_SENSOR_CONNECT_STATUS_PIN2, BSP_EXTENSIONPORT_GPIO_PINMODE, (uint32_t) BSP_EXTENSIONPORT_INPUT_NOPULL, NULL);
    }
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_ExtensionPort_SetGpioConfig(LEM_SENSOR_CONNECT_STATUS_PIN1, BSP_EXTENSIONPORT_GPIO_PINVALUE, (uint32_t) BSP_EXTENSIONPORT_GPIO_PIN_HIGH, NULL);
    }
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_ExtensionPort_SetGpioConfig(LEM_SENSOR_CONNECT_STATUS_PIN2, BSP_EXTENSIONPORT_GPIO_PINVALUE, (uint32_t) BSP_EXTENSIONPORT_GPIO_PIN_HIGH, NULL);
    }
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_ExtensionPort_EnableGpio(LEM_SENSOR_CONNECT_STATUS_PIN1);
    }
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_ExtensionPort_EnableGpio(LEM_SENSOR_CONNECT_STATUS_PIN2);
    }
    return retcode;

}

Retcode_T ExternalSensor_HwStatusPinDeInit(void)
{
    Retcode_T retcode = RETCODE_OK;
    retcode = BSP_ExtensionPort_DisableGpio(LEM_SENSOR_CONNECT_STATUS_PIN1);
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_ExtensionPort_DisableGpio(LEM_SENSOR_CONNECT_STATUS_PIN2);
    }
    retcode = BSP_ExtensionPort_DisconnectGpio(LEM_SENSOR_CONNECT_STATUS_PIN1);
    if (RETCODE_OK == retcode)
    {
        retcode = BSP_ExtensionPort_DisconnectGpio(LEM_SENSOR_CONNECT_STATUS_PIN2);
    }
    return retcode;
}

#endif /* XDK_SENSOR_EXTERNALSENSOR */

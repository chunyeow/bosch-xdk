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

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_CAYENNELPPSERIALIZER

#include "XDK_CayenneLPPSerializer.h"

/* constant definitions ***************************************************** */

#define DIGITAL_INPUT_SINGLE_PAYLOAD_SIZE           3U
#define DIGITAL_OUTPUT_SINGLE_PAYLOAD_SIZE          3U
#define ANALOG_INPUT_SINGLE_PAYLOAD_SIZE            4U
#define ANALOG_OUTPUT_SINGLE_PAYLOAD_SIZE           4U
#define ILLUMINANCE_SENSOR_SINGLE_PAYLOAD_SIZE      4U
#define PRESENCE_SENSOR_SINGLE_PAYLOAD_SIZE         3U
#define TEMPERATURE_SENSOR_SINGLE_PAYLOAD_SIZE      4U
#define HUMIDITY_SENSOR_SINGLE_PAYLOAD_SIZE         3U
#define ACCELEROMETER_SINGLE_PAYLOAD_SIZE           8U
#define BAROMETER_SINGLE_PAYLOAD_SIZE               4U
#define GYROMETER_SINGLE_PAYLOAD_SIZE               8U
#define GPS_LOCATION_SINGLE_PAYLOAD_SIZE            11U

/** Refer interface header for description */
Retcode_T CayenneLPPSerializer_SingleInstance(CayenneLPPSerializer_Input_T * input, CayenneLPPSerializer_Output_T * output)
{
    Retcode_T retcode = RETCODE_OK;

    if ((0UL == output->BufferLength) || (NULL == output->BufferPointer))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        switch (input->DataType)
        {
        case CAYENNE_LLP_SERIALIZER_DIGITAL_INPUT:
            if (output->BufferLength >= DIGITAL_INPUT_SINGLE_PAYLOAD_SIZE)
            {
                output->BufferPointer[2] = input->Data.DigitalInput.DigitalInputValue;
                output->BufferFilledLength = DIGITAL_INPUT_SINGLE_PAYLOAD_SIZE;
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
            break;

        case CAYENNE_LLP_SERIALIZER_DIGITAL_OUTPUT:
            if (output->BufferLength >= DIGITAL_OUTPUT_SINGLE_PAYLOAD_SIZE)
            {
                output->BufferPointer[2] = input->Data.DigitalOutput.DigitalOutputValue;
                output->BufferFilledLength = DIGITAL_OUTPUT_SINGLE_PAYLOAD_SIZE;
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
            break;

        case CAYENNE_LLP_SERIALIZER_ANALOG_INPUT:
            if (output->BufferLength >= ANALOG_INPUT_SINGLE_PAYLOAD_SIZE)
            {
                output->BufferPointer[2] = (uint8_t) (input->Data.AnalogInput.AnalogInputValue >> 8);
                output->BufferPointer[3] = (uint8_t) input->Data.AnalogInput.AnalogInputValue;
                output->BufferFilledLength = ANALOG_INPUT_SINGLE_PAYLOAD_SIZE;
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
            break;

        case CAYENNE_LLP_SERIALIZER_ANALOG_OUTPUT:
            if (output->BufferLength >= ANALOG_OUTPUT_SINGLE_PAYLOAD_SIZE)
            {
                output->BufferPointer[2] = (uint8_t) (input->Data.AnalogOutput.AnalogOutputValue >> 8);
                output->BufferPointer[3] = (uint8_t) input->Data.AnalogOutput.AnalogOutputValue;
                output->BufferFilledLength = ANALOG_OUTPUT_SINGLE_PAYLOAD_SIZE;
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
            break;

        case CAYENNE_LLP_SERIALIZER_ILLUMINANCE_SENSOR:
            if (output->BufferLength >= ILLUMINANCE_SENSOR_SINGLE_PAYLOAD_SIZE)
            {
                output->BufferPointer[2] = (uint8_t) (input->Data.IlluminanceSensor.IlluminanceSensorValue >> 8);
                output->BufferPointer[3] = (uint8_t) (input->Data.IlluminanceSensor.IlluminanceSensorValue);
                output->BufferFilledLength = ILLUMINANCE_SENSOR_SINGLE_PAYLOAD_SIZE;
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
            break;

        case CAYENNE_LLP_SERIALIZER_PRESENCE_SENSOR:
            if (output->BufferLength >= PRESENCE_SENSOR_SINGLE_PAYLOAD_SIZE)
            {
                output->BufferPointer[2] = input->Data.PresenceSensor.PresenceSensorValue;
                output->BufferFilledLength = PRESENCE_SENSOR_SINGLE_PAYLOAD_SIZE;
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
            break;

        case CAYENNE_LLP_SERIALIZER_TEMPERATURE_SENSOR:
            if (output->BufferLength >= TEMPERATURE_SENSOR_SINGLE_PAYLOAD_SIZE)
            {
                output->BufferPointer[2] = (uint8_t) (input->Data.TemperatureSensor.TemperatureSensorValue >> 8);
                output->BufferPointer[3] = (uint8_t) (input->Data.TemperatureSensor.TemperatureSensorValue);
                output->BufferFilledLength = TEMPERATURE_SENSOR_SINGLE_PAYLOAD_SIZE;
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
            break;

        case CAYENNE_LLP_SERIALIZER_HUMIDITY_SENSOR:
            if (output->BufferLength >= HUMIDITY_SENSOR_SINGLE_PAYLOAD_SIZE)
            {
                output->BufferPointer[2] = input->Data.HumiditySensor.HumiditySensorValue;
                output->BufferFilledLength = HUMIDITY_SENSOR_SINGLE_PAYLOAD_SIZE;
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
            break;

        case CAYENNE_LLP_SERIALIZER_ACCELEROMETER:
            if (output->BufferLength >= ACCELEROMETER_SINGLE_PAYLOAD_SIZE)
            {
                output->BufferPointer[2] = (uint8_t) (input->Data.Accelerometer.AccelerometerXValue >> 8);
                output->BufferPointer[3] = (uint8_t) input->Data.Accelerometer.AccelerometerXValue;
                output->BufferPointer[4] = (uint8_t) (input->Data.Accelerometer.AccelerometerYValue >> 8);
                output->BufferPointer[5] = (uint8_t) input->Data.Accelerometer.AccelerometerYValue;
                output->BufferPointer[6] = (uint8_t) (input->Data.Accelerometer.AccelerometerZValue >> 8);
                output->BufferPointer[7] = (uint8_t) input->Data.Accelerometer.AccelerometerZValue;
                output->BufferFilledLength = ACCELEROMETER_SINGLE_PAYLOAD_SIZE;
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
            break;

        case CAYENNE_LLP_SERIALIZER_BAROMETER:
            if (output->BufferLength >= BAROMETER_SINGLE_PAYLOAD_SIZE)
            {
                output->BufferPointer[2] = (uint8_t) (input->Data.Barometer.BarometerValue >> 8);
                output->BufferPointer[3] = (uint8_t) input->Data.Barometer.BarometerValue;
                output->BufferFilledLength = BAROMETER_SINGLE_PAYLOAD_SIZE;

            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
            break;

        case CAYENNE_LLP_SERIALIZER_GYROMETER:
            if (output->BufferLength >= GYROMETER_SINGLE_PAYLOAD_SIZE)
            {
                output->BufferPointer[2] = (uint8_t) (input->Data.Gyrometer.GyrometerXValue >> 8);
                output->BufferPointer[3] = (uint8_t) input->Data.Gyrometer.GyrometerXValue;
                output->BufferPointer[4] = (uint8_t) (input->Data.Gyrometer.GyrometerYValue >> 8);
                output->BufferPointer[5] = (uint8_t) input->Data.Gyrometer.GyrometerYValue;
                output->BufferPointer[6] = (uint8_t) (input->Data.Gyrometer.GyrometerZValue >> 8);
                output->BufferPointer[7] = (uint8_t) input->Data.Gyrometer.GyrometerZValue;
                output->BufferFilledLength = GYROMETER_SINGLE_PAYLOAD_SIZE;
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
            break;

        case CAYENNE_LLP_SERIALIZER_GPS_LOCATION:
            if (output->BufferLength >= GPS_LOCATION_SINGLE_PAYLOAD_SIZE)
            {
                output->BufferPointer[2] = (uint8_t) ((int32_t) input->Data.GPSLocation.Latitude >> 16);
                output->BufferPointer[3] = (uint8_t) (input->Data.GPSLocation.Latitude >> 8);
                output->BufferPointer[4] = (uint8_t) input->Data.GPSLocation.Latitude;
                output->BufferPointer[5] = (uint8_t) (input->Data.GPSLocation.Longitude >> 16);
                output->BufferPointer[6] = (uint8_t) (input->Data.GPSLocation.Longitude >> 8);
                output->BufferPointer[7] = (uint8_t) input->Data.GPSLocation.Longitude;
                output->BufferPointer[8] = (uint8_t) (input->Data.GPSLocation.Altitude >> 16);
                output->BufferPointer[9] = (uint8_t) (input->Data.GPSLocation.Altitude >> 8);
                output->BufferPointer[10] = (uint8_t) input->Data.GPSLocation.Altitude;
                output->BufferFilledLength = GPS_LOCATION_SINGLE_PAYLOAD_SIZE;
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
            break;
        }

        if (RETCODE_OK == retcode)
        {
            output->BufferPointer[0] = input->DataChannel;
            output->BufferPointer[1] = (uint8_t) input->DataType;
        }
    }
    return retcode;
}

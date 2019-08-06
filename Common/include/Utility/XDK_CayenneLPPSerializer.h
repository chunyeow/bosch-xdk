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
 * @ingroup UTILITY
 *
 * @defgroup CAYENNELPPSERIALIZER Cayenna LPP Serializer
 * @{
 *
 * @brief Interface header file for the Cayenne LPP Serializer feature.
 *
 * @details The Cayenne Low Power Payload (LPP) provides a convenient and easy way to
 * send data over LPWAN networks such as LoRaWAN. The Cayenne LPP is compliant
 * with the payload size restriction, which can be lowered down to 11 bytes,
 * and allows the device to send multiple sensor data at one time.
 *
 * Additionally, the Cayenne LPP allows the device to send different sensor data
 * in different frames. In order to do that, each sensor data must be prefixed
 * with two bytes:
 *      - Data Channel: Uniquely identifies each sensor in the device across frames, eg. “indoor sensor”
 *      - Data Type: Identifies the data type in the frame, eg. “temperature”.
 *
 * Payload structure
 *      1 Byte       1 Byte          N Bytes         1 Byte          1 Byte          M Bytes     ...
 *      Data1 Ch.    Data1 Type      Data1           Data2 Ch.       Data2 Type      Data2       ...
 *
 * Type                 IPSO    LPP     Hex     Data Size   Data Resolution per bit
 * Digital Input        3200    0       0       1           1
 * Digital Output       3201    1       1       1           1
 * Analog Input         3202    2       2       2           0.01 Signed
 * Analog Output        3203    3       3       2           0.01 Signed
 * Illuminance Sensor   3301    101     65      2           1 Lux Unsigned MSB
 * Presence Sensor      3302    102     66      1           1
 * Temperature Sensor   3303    103     67      2           0.1 °C Signed MSB
 * Humidity Sensor      3304    104     68      1           0.5 % Unsigned
 * Accelerometer        3313    113     71      6           0.001 G Signed MSB per axis
 * Barometer            3315    115     73      2           0.1 hPa Unsigned MSB
 * Gyrometer            3334    134     86      6           0.01 °/s Signed MSB per axis
 * GPS Location         3336    136     88      9           Latitude : 0.0001 ° Signed MSB
 *                                                          Longitude : 0.0001 ° Signed MSB
 *                                                          Altitude : 0.01 meter Signed MSB
 *
 * So for single instance serializer, the output payload size is two bytes in addition to the Data Size.
 *
 * @file
 */

/* header definition ******************************************************** */
#ifndef XDK_CAYENNELPPBUILDER_H_
#define XDK_CAYENNELPPBUILDER_H_

/* local interface declaration ********************************************** */
#include "BCDS_Retcode.h"

/* local type and macro definitions */

/**
 * @brief   Union to represent the supported Data payloads for Cayenne LPP.
 */
union CayenneLPPSerializer_Data_U
{
    struct
    {
        uint8_t DigitalInputValue;
    }__attribute__((packed)) DigitalInput;
    struct
    {
        uint8_t DigitalOutputValue;
    }__attribute__((packed)) DigitalOutput;
    struct
    {
        int16_t AnalogInputValue;
    }__attribute__((packed)) AnalogInput;
    struct
    {
        int16_t AnalogOutputValue;
    }__attribute__((packed)) AnalogOutput;
    struct
    {
        uint16_t IlluminanceSensorValue;
    }__attribute__((packed)) IlluminanceSensor;
    struct
    {
        uint8_t PresenceSensorValue;
    }__attribute__((packed)) PresenceSensor;
    struct
    {
        int16_t TemperatureSensorValue;
    }__attribute__((packed)) TemperatureSensor;
    struct
    {
        uint8_t HumiditySensorValue;
    }__attribute__((packed)) HumiditySensor;
    struct
    {
        int16_t AccelerometerXValue;
        int16_t AccelerometerYValue;
        int16_t AccelerometerZValue;
    }__attribute__((packed)) Accelerometer;
    struct
    {
        uint16_t BarometerValue;
    }__attribute__((packed)) Barometer;
    struct
    {
        int16_t GyrometerXValue;
        int16_t GyrometerYValue;
        int16_t GyrometerZValue;
    }__attribute__((packed)) Gyrometer;
    struct
    {
        signed Latitude :24;
        signed Longitude :24;
        signed Altitude :24;
    }__attribute__((packed)) GPSLocation;
};

/**
 * @brief   Typedef to represent the supported Data payload for Cayenne LPP.
 */
typedef union CayenneLPPSerializer_Data_U CayenneLPPSerializer_Data_T;

/**
 * @brief   Enum to represent the supported Data Types for Cayenne LPP.
 */
enum CayenneLPPSerializer_DataType_E
{
    CAYENNE_LLP_SERIALIZER_DIGITAL_INPUT = 0,
    CAYENNE_LLP_SERIALIZER_DIGITAL_OUTPUT = 1,
    CAYENNE_LLP_SERIALIZER_ANALOG_INPUT = 2,
    CAYENNE_LLP_SERIALIZER_ANALOG_OUTPUT = 3,
    CAYENNE_LLP_SERIALIZER_ILLUMINANCE_SENSOR = 101,
    CAYENNE_LLP_SERIALIZER_PRESENCE_SENSOR = 102,
    CAYENNE_LLP_SERIALIZER_TEMPERATURE_SENSOR = 103,
    CAYENNE_LLP_SERIALIZER_HUMIDITY_SENSOR = 104,
    CAYENNE_LLP_SERIALIZER_ACCELEROMETER = 113,
    CAYENNE_LLP_SERIALIZER_BAROMETER = 115,
    CAYENNE_LLP_SERIALIZER_GYROMETER = 134,
    CAYENNE_LLP_SERIALIZER_GPS_LOCATION = 136,
};

/**
 * @brief   Typedef to represent the supported Data Type for Cayenne LPP.
 */
typedef enum CayenneLPPSerializer_DataType_E CayenneLPPSerializer_DataType_T;

/**
 * @brief   Structure to represent the Input payload informations for serialization.
 */
struct CayenneLPPSerializer_Input_S
{
    uint8_t DataChannel; /* Uniquely identifies each sensor in the device across frames, eg. “indoor sensor” */
    CayenneLPPSerializer_DataType_T DataType; /* Identifies the data type in the frame, eg. “temperature” */
    CayenneLPPSerializer_Data_T Data; /* Data payload to be serialized. Must correspond to DataType */
};

/**
 * @brief   Typedef to represent the Input payload information for serialization.
 */
typedef struct CayenneLPPSerializer_Input_S CayenneLPPSerializer_Input_T;

/**
 * @brief   Structure to represent the Output payload informations after serialization.
 */
struct CayenneLPPSerializer_Output_S
{
    uint8_t * BufferPointer; /* Pointer to the output buffer which will be used by Serializer to construct the payload */
    uint32_t BufferLength; /* Length / Size of the output buffer upto which the Serializer can utilize to construct the payload */
    uint32_t BufferFilledLength; /* Actual Length / size of the output buffer that was utilized by Serializer upon construction of the payload */
};

/**
 * @brief   Typedef to represent the Output payload information after serialization.
 */
typedef struct CayenneLPPSerializer_Output_S CayenneLPPSerializer_Output_T;

/**
 * @brief   This will serialize the payload for single instance (only one data point per payload)
 *
 * @param[in] input
 * Input payload information for serialization.
 *
 * @param[in/out] output
 * Output payload information for and after serialization.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - The maximum output->BufferFilledLength that a single instance serialized
 *   will utilize is a maximum of 11 bytes.
 */
Retcode_T CayenneLPPSerializer_SingleInstance(CayenneLPPSerializer_Input_T * input, CayenneLPPSerializer_Output_T * output);

#endif /* XDK_CAYENNELPPBUILDER_H_ */
/**@}*/

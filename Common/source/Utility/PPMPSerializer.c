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
 * This module handles the PPMP serializer
 *
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_PPMPSERIALIZER

/* own header files */
#include "XDK_PPMPSerializer.h"

/* system header files */
#include "stdio.h"

/* additional header files */
#include "cJSON.h"

/* constant definitions ***************************************************** */

/* local variables ********************************************************** */

/**< PPMP serializer setup information */
static PPMPSerializer_Setup_T PPMPSerializerSetup;
/**< cJSON handle for root */
static cJSON *JsonRootSensorSelective;
/**< cJSON handle for device */
static cJSON *JsonDeviceSensorSelective;
/**< cJSON handle for data */
static cJSON *JsonDataSensorSelective;
/**< cJSON handle for sub data */
static cJSON *JsonSubDataSensorSelective;
/**< cJSON handle for super sub data */
static cJSON *JsonSuperSubDataSensorSelective;
/**< cJSON handle for time */
static cJSON *JsonTimeSensorSelective;
/**< cJSON handle for constructed final output */
static char *JsonSensorSelective;

/** Refer interface header for description */
Retcode_T PPMPSerializer_Setup(PPMPSerializer_Setup_T * setup)
{
    Retcode_T retcode = RETCODE_OK;

    if (NULL == setup)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        PPMPSerializerSetup = *setup;
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T PPMPSerializer_Enable(void)
{
    return RETCODE_OK;
}

/** Refer interface header for description */
char * PPMPSerializer_PayloadSensorSelective(bool create, char * timezoneISO8601format, uint32_t timeStampDelta, Sensor_Value_T * sensorValue, Sensor_Enable_T * serializeSensor)
{
    if (create)
    {
        if ((NULL == serializeSensor) || (NULL == timezoneISO8601format) || (NULL == sensorValue) || (NULL == serializeSensor))
        {
            Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
            return NULL;
        }
        /* Here we construct some JSON standards, based on our requirement. */

        if (serializeSensor->Accel || serializeSensor->Mag || serializeSensor->Gyro || serializeSensor->Humidity ||
                serializeSensor->Temp || serializeSensor->Pressure || serializeSensor->Light || serializeSensor->Noise)
        {
            JsonRootSensorSelective = cJSON_CreateObject();
            cJSON_AddItemToObject(JsonRootSensorSelective, "content-spec", cJSON_CreateString("urn:spec://eclipse.org/unide/measurement-message#v2"));
            cJSON_AddItemToObject(JsonRootSensorSelective, "device", JsonDeviceSensorSelective = cJSON_CreateObject());
            cJSON_AddStringToObject(JsonDeviceSensorSelective, "deviceID", PPMPSerializerSetup.DeviceName);
            cJSON_AddItemToObject(JsonRootSensorSelective, "measurements", JsonTimeSensorSelective = cJSON_CreateArray());

            printf("\r\nconstructed payload for Sensors : \r\n");

            if (true == serializeSensor->Accel)
            {
                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "ax", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "ax", (double )sensorValue->Accel.X);

                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "ay", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "ay", (double )sensorValue->Accel.Y);

                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "az", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "az", (double )sensorValue->Accel.Z);

                printf("\t- Accel \r\n");
            }

            if (true == serializeSensor->Mag)
            {
                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "mx", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "mx", (double )sensorValue->Mag.X);

                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "my", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "my", (double )sensorValue->Mag.Y);

                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "mz", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "mz", (double )sensorValue->Mag.Z);

                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "mr", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "mr", (double )sensorValue->Mag.R);

                printf("\t- Mag \r\n");
            }

            if (true == serializeSensor->Gyro)
            {
                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "gx", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "gx", (double )sensorValue->Gyro.X);

                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "gy", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "gy", (double )sensorValue->Gyro.Y);

                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "gz", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "gz", (double )sensorValue->Gyro.Z);

                printf("\t- Gyro \r\n");
            }

            if (true == serializeSensor->Humidity)
            {
                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "rh", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "rh", (double )sensorValue->RH);

                printf("\t- Humidity \r\n");
            }

            if (true == serializeSensor->Temp)
            {
                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "t", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "t", (double )sensorValue->Temp);

                printf("\t- Temp \r\n");
            }

            if (true == serializeSensor->Pressure)
            {
                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "p", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "p", (double )sensorValue->Pressure);

                printf("\t- Pressure \r\n");
            }

            if (true == serializeSensor->Light)
            {
                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "l", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "l", (double )sensorValue->Light);

                printf("\t- Light \r\n");
            }

            if (true == serializeSensor->Noise)
            {
                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "n", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "n", (double )sensorValue->Noise);

                printf("\t- Noise \r\n");
            }

            JsonSensorSelective = cJSON_Print(JsonRootSensorSelective);
            cJSON_Minify(JsonSensorSelective);
            printf("%s\r\n", JsonSensorSelective);
        }
        else
        {
            printf("Atleast one Sensor must be enabled for PPMPSerializer_PayloadSensorSelective\r\n");
            return NULL;
        }
    }
    else
    {
        // Clear JSON Buffer
        cJSON_Delete(JsonRootSensorSelective);
        free(JsonSensorSelective); /* Print to text, Delete the cJSON, print it, release the string. */
        printf("\r\nreleased buffers used for Sensors\r\n\r\n");
    }
    return (JsonSensorSelective);
}

/** Refer interface header for description */
char * PPMPSerializer_PayloadExternalSensorSelective(bool create, char * timezoneISO8601format, uint32_t timeStampDelta, ExternalSensor_Value_T * sensorValue, ExternalSensor_Enable_T * serializeSensor)
{
    if (create)
    {
        if ((NULL == serializeSensor) || (NULL == timezoneISO8601format) || (NULL == sensorValue) || (NULL == serializeSensor))
        {
            Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
            return NULL;
        }
        /* Here we construct some JSON standards, based on our requirement. */

        if (serializeSensor->LemCurrent || serializeSensor->LemVoltage)
        {
            JsonRootSensorSelective = cJSON_CreateObject();
            cJSON_AddItemToObject(JsonRootSensorSelective, "content-spec", cJSON_CreateString("urn:spec://eclipse.org/unide/measurement-message#v2"));
            cJSON_AddItemToObject(JsonRootSensorSelective, "device", JsonDeviceSensorSelective = cJSON_CreateObject());
            cJSON_AddStringToObject(JsonDeviceSensorSelective, "deviceID", PPMPSerializerSetup.DeviceName);
            cJSON_AddItemToObject(JsonRootSensorSelective, "measurements", JsonTimeSensorSelective = cJSON_CreateArray());

            printf("\r\nconstructed payload for Sensors : \r\n");

            if (true == serializeSensor->LemCurrent)
            {
                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "i", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "i", (double )sensorValue->XLemI);

                printf("\t- LEM Current \r\n");
            }

            if (true == serializeSensor->LemVoltage)
            {
                cJSON_AddItemToObject(JsonTimeSensorSelective, "ts", JsonDataSensorSelective = cJSON_CreateObject());
                cJSON_AddStringToObject(JsonDataSensorSelective, "ts", timezoneISO8601format);
                cJSON_AddItemToObject(JsonDataSensorSelective, "series", JsonSubDataSensorSelective = cJSON_CreateObject());
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "$_time", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "$_time", timeStampDelta);
                cJSON_AddItemToObject(JsonSubDataSensorSelective, "v", JsonSuperSubDataSensorSelective = cJSON_CreateArray());
                cJSON_AddNumberToObject(JsonSuperSubDataSensorSelective, "v", (double )sensorValue->XLemV);

                printf("\t- LEM Voltage \r\n");
            }

            JsonSensorSelective = cJSON_Print(JsonRootSensorSelective);
            cJSON_Minify(JsonSensorSelective);
            printf("%s\r\n", JsonSensorSelective);
        }
        else
        {
            printf("Atleast one Sensor must be enabled for PPMPSerializer_PayloadSensorSelective\r\n");
            return NULL;
        }
    }
    else
    {
        // Clear JSON Buffer
        cJSON_Delete(JsonRootSensorSelective);
        free(JsonSensorSelective); /* Print to text, Delete the cJSON, print it, release the string. */
        printf("\r\nreleased buffers used for Sensors\r\n\r\n");
    }
    return (JsonSensorSelective);
}

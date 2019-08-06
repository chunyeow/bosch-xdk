/*
* Licensee agrees that the example code provided to Licensee has been developed and released by Bosch solely as an example to be used as a potential reference for Licensee’s application development. 
* Fitness and suitability of the example code for any use within Licensee’s applications need to be verified by Licensee on its own authority by taking appropriate state of the art actions and measures (e.g. by means of quality assurance measures).
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
*
* Contributors:
* Bosch Software Innovations GmbH
*/
/*----------------------------------------------------------------------------*/

/**
 * @brief This file provides implementation for LWM2MObjectDevice module.
 *
 * @file
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"
#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_LWM2MOBJECTDEVICE

#include "LWM2MObjectDevice.h"

/* additional interface header files */
#include <Serval_Clock.h>

#include <em_system.h>
#include "em_usb.h"
#include <BCDS_FWContainer.h>
#include "BCDS_EFM32XXPartitionAgent.h"
#include <XdkVersion.h>
#include "LWM2MObjectSensorDevice.h"
#include "LWM2MObjects.h"
#include "BatteryMonitor.h"
#include "BCDS_BSP_Charger_BQ2407X.h"
#include "LWM2M.h"
#include "XDK_SNTP.h"
#include "XDK_Utils.h"

/** The maximum length occupied by a timezone from the IANA database is 32.
 * The length is set to 40 just to accommodate few more characters, if the database is updated in future.
 */
#define MAXIMUM_LENGTH_OF_TIMEZONE            UINT8_C(40)

/**
 * As per ISO 8061 Expected not exceed 9 bytes
 */
#define MAXIMUM_LENGTH_OF_UTCOFFSET           UINT8_C(10)

/**
 * Same as BSE ID which is 13 bytes
 */
#define MAXIMUM_LENGTH_OF_SERIALNUMBER        UINT8_C(17)

#define MAXIMUM_LENGTH_OF_FIRMWAREVERSION       UINT8_C(26)

/**
 * Unware which model number we're going to use (HW/SW)
 * but 16 bytes should be enough
 */
#define MAXIMUM_LENGTH_OF_MODELNUMBER         UINT8_C(16)
#define MAXIMUM_LENGTH_OF_FIRMVERSION         UINT8_C(16)

#define POWER_SOURCE_BATTERY   1  /* 1: Internal Battery */
#define POWER_SOURCE_USB       5  /* 1: USB Powered */

#define BATTERY_FULL_VOLTAGE   UINT32_C(4650) /* */

#define LWM2MOBJECTS_UDP_LENGTH                 UINT32_C(1)
#define LWM2MOBJECTS_UDP_QUEUED_LENGTH          UINT32_C(2)
#define LWM2MOBJECTS_SMS_LENGTH                 UINT32_C(1)
#define LWM2MOBJECTS_SMS_QUEUED_LENGTH          UINT32_C(2)
#define LWM2MOBJECTS_UDP_AND_SMS_LENGTH         UINT32_C(2)
#define LWM2MOBJECTS_UDP_QUEUED_AND_SMS_LENGTH  UINT32_C(3)
#define STRING_RESOURCES_MAX_LENGTH             UINT32_C(50)

#define LWM2M_ZERO_U32                        (UINT32_C(0))       /**< unsigned integer value 0 */
#define LWM2M_ONE_U32                         (UINT32_C(1))       /**< unsigned integer value 1 */
#define LWM2M_TWO_U32                         (UINT32_C(2))       /**< unsigned integer value 2 */
#define LWM2M_THREE_U32                       (UINT32_C(3))       /**< unsigned integer value 3 */
#define LWM2M_EIGHT_U32                       (UINT32_C(8))

#define LWM2M_STRING_NULL                     ("\0")               /**< string "X" representation */
#define LWM2M_STRING_CAPITAL_Q                ("Q")               /**< string "Q" representation */
#define LWM2M_STRING_CAPITAL_U                ("U")               /**< string "U" representation */
#define LWM2M_STRING_CAPITAL_UQ               ("UQ")              /**< string "UQ" representation */
#define LWM2M_STRING_CAPITAL_S                ("S")               /**< string "S" representation */
#define LWM2M_STRING_CAPITAL_SQ               ("SQ")              /**< string "SQ" representation */
#define LWM2M_STRING_CAPITAL_US               ("US")              /**< string "US" representation */
#define LWM2M_STRING_CAPITAL_UQS              ("UQS")             /**< string "UQS" representation */

#define STRING_LENGTH UINT8_C(19)

/* local variables ********************************************************* */

#define LWM2M_DEVICE_MANUFACTURER_NAME_S            "Bosch BCDS"
#define LWM2M_DEVICE_MODEL_NUMBER_S                 "XDK110"
#define LWM2M_DEVICE_UTC_OFFSET_DEFAULT_S           "UTC+2"
#define LWM2M_DEVICE_TIMEZONE_DEFAULT_S             "Europe/Berlin"

static const char DeviceManufacturer[] = LWM2M_DEVICE_MANUFACTURER_NAME_S;
static const char DeviceModelNumber[MAXIMUM_LENGTH_OF_MODELNUMBER] = LWM2M_DEVICE_MODEL_NUMBER_S;
static char DeviceSerialNumber[MAXIMUM_LENGTH_OF_SERIALNUMBER] = { 0 };
static uint8_t DeviceFirmwareVersion[MAXIMUM_LENGTH_OF_FIRMWAREVERSION] = { 0 };

static char DeviceUtcOffset[MAXIMUM_LENGTH_OF_UTCOFFSET] = LWM2M_DEVICE_UTC_OFFSET_DEFAULT_S;
static char DeviceTimezone[MAXIMUM_LENGTH_OF_TIMEZONE] = LWM2M_DEVICE_TIMEZONE_DEFAULT_S;

static StringDescr_T Lwm2m_utcOffset =
        {
                .start = DeviceUtcOffset,
                .length = sizeof(DeviceUtcOffset)
        };

static StringDescr_T Lwm2m_tzone =
        {
                .start = DeviceTimezone,
                .length = sizeof(DeviceTimezone)
        };

/**
 * @brief Structure for device binding attributes.
 * Refer enum Lwm2m_Binding_E under Serval_Lwm2m.h to
 * check the mapped stings.
 */
static const StringDescr_T Lwm2m_deviceBindingAttributes[] =
        {
                { LWM2M_STRING_NULL, LWM2M_ONE_U32 }, /* 0x00 */
                { LWM2M_STRING_CAPITAL_U, LWM2M_ONE_U32 }, /* 0x01 */
                { LWM2M_STRING_CAPITAL_S, LWM2M_ONE_U32 }, /* 0x02 */
                { LWM2M_STRING_CAPITAL_US, LWM2M_TWO_U32 }, /* 0x03 */
                { LWM2M_STRING_NULL, LWM2M_ONE_U32 }, /* 0x04 */
                { LWM2M_STRING_CAPITAL_UQ, LWM2M_TWO_U32 }, /* 0x05 */
                { LWM2M_STRING_CAPITAL_SQ, LWM2M_TWO_U32 }, /* 0x06 */
                { LWM2M_STRING_CAPITAL_UQS, LWM2M_THREE_U32 }, /* 0x07 */
        };

static volatile bool started = false;

/*lint -e(956) */
static int error_codes_count = 1;
/*lint -esym(956,error_codes) */
static int32_t error_codes[5] = { 0, 0, 0, 0, 0 };

//extern Lwm2mDevice_T LWM2MDeviceResourceInfo;

#define DEVICE_RESOURCES_INDEX(res) LWM2M_RESOURCES_INDEX(LWM2MObjectDeviceResources, res)

#define POWER_SOURCES_COUNT                     2

/* local functions ********************************************************** */

/**
 * @brief Convert Decimal/Hex value to String Format to display Version Information.
 *
 * @param[in] data
 * Number which needs to be convert into string to display in Decimal instead of HEX values.
 *
 * @param[in/out] *str
 * pointer to Store/write the Version number in String Format.
 *
 */
static uint8_t * convDecToStr(uint8_t data, uint8_t * str)
{
    uint8_t arr[3] =
            { 0, 0, 0 };
    uint8_t noOfDigits = 0x00;
    uint8_t index = 0x00;
    do
    {
        arr[index] = data % 10;
        data = data / 10;
        index++;
        noOfDigits++;
    } while (data != (0x00));

    for (int8_t i = (noOfDigits - 1); (i >= 0); i--)
    {
        *str = arr[i] + 0x30;
        str++;
    }
    return str;
}

/**
 * @brief This function is to convert the integer data into a string data
 *
 * @param[in] appVersion
 * 32 bit value containing the FOTA version information
 *
 * @param[in] xdkVersion
 * 32 bit value containing the XDK SW version information
 *
 * @param[in/out] str
 * pointer to the parsed version string
 *
 * @param [out] Returns
 * index where the application version starts
 *
 * @Note 1. FOTA capable device only has the Valid application firmware version in that case output format
 *          will be example:v3.0.1-0.0.1( XDK Version + Application version) <br>
 *       2. For Non- FOTA capable device version number will be v3.0.1-xx.xx.xx( XDK Version-xx.xx.xx) <br>
 *       3. Also note that the XDK version number is represented in Decimal format (i.e if XDK version is 3.0.1).<br>
 *          the following macros maintain to represent the XDK SW release version
 *              #define XDKVERSION_MAJOR        3
 *              #define XDKVERSION_MINOR        0
 *              #define XDKVERSION_PATCH        1
 *       4. Also note that the Application version number is represented in Decimal format.
 *       example: if user modifies the application version value macros under application.mk as follows
 *              MAJOR_SW_NO ?= 0xFF
 *              MINOR_SW_NO ?= 0xFF
 *              PATCH_SW_NO ?= 0xFF
 *      so, the appVersion will be represented as 255.255.255
 *
 */
uint8_t* convert32IntegerToVersionString(uint32_t appVersion, uint32_t xdkVersion, uint8_t* str)
{
    uint8_t shiftValue = UINT8_C(16);
    uint8_t* startPtrAppVersion = NULL;
    uint8_t byteVar = 0x00;
    *str = 'v';
    for (uint8_t i = 0; i < 3; i++)
    {
        str++;
        byteVar = ((xdkVersion >> shiftValue) & 0xFF);
        str = convDecToStr(byteVar, str);
        *str = '.';
        shiftValue = shiftValue - 8;
    }
    *str = '-';
    startPtrAppVersion = str;
    shiftValue = UINT8_C(16);

    for (uint8_t i = 0; i < 3; i++)
    {
        str++;
        byteVar = ((appVersion >> shiftValue) & 0xFF);
        str = convDecToStr(byteVar, str);
        *str = '.';
        shiftValue = shiftValue - 8;
    }
    *str = '\0';
    return (startPtrAppVersion + 1);
}

/** @brief
 *  This function is used to update the errorCode Resource value into the LWM2M Server*
 */
static retcode_t util_serialize_array(Lwm2mSerializer_T *serializer_ptr, int count, int32_t values[])
{
    int index;

    if (serializer_ptr == NULL)
        return (RC_LWM2M_METHOD_NOT_ALLOWED);
    retcode_t rc = Lwm2mSerializer_startSerializingResourceArray(serializer_ptr);
    if (rc != RC_OK)
        return rc;

    for (index = 0; index < count; ++index)
    {
        rc = Lwm2mSerializer_setResourceInstanceId(serializer_ptr, index);
        if (rc != RC_OK)
            return rc;

        rc = Lwm2mSerializer_serializeInt(serializer_ptr, values[index]);
        if (rc != RC_OK)
            return rc;
    }

    return Lwm2mSerializer_endSerializingResourceArray(serializer_ptr);
}

/** @brief
 * This function is used to set the device serial number resource with the XDK CoreID in the LWM2M Server*
 */
static retcode_t serialNumber_RO(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    if (parser_ptr != NULL)
    {
        return (RC_LWM2M_METHOD_NOT_ALLOWED);
    }
    memset(DeviceSerialNumber, 0, sizeof(DeviceSerialNumber));
    sprintf(DeviceSerialNumber, "%llX", SYSTEM_GetUnique());
    StringDescr_T strDescr_sn;
    StringDescr_set(&strDescr_sn, DeviceSerialNumber, strlen(DeviceSerialNumber));
    return (Lwm2mSerializer_serializeString(serializer_ptr, &strDescr_sn));
}

/**
 * @brief
 * This function is used to set the firmware version number present as part of the firmware binary from the Server
 */
static retcode_t firmwareVersionNumber_RO(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    if (parser_ptr != NULL)
    {
        return (RC_LWM2M_METHOD_NOT_ALLOWED);
    }
    Retcode_T retval = RETCODE_OK;

    memset(DeviceFirmwareVersion, 0, sizeof(DeviceFirmwareVersion));
    retval = Utils_GetXdkVersionString(DeviceFirmwareVersion);
    if (RETCODE_OK != retval)
    {
        return (RC_PLATFORM_ERROR);
    }
    StringDescr_T strDescr_sn;
    StringDescr_set(&strDescr_sn, (char*) DeviceFirmwareVersion, strlen((char*) DeviceFirmwareVersion));
    return (Lwm2mSerializer_serializeString(serializer_ptr, &strDescr_sn));
}

/** @brief
 *  This function is used to update the deviceReboot Resource value into the LWM2M Server*
 */
static retcode_t deviceRebootFunc(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    (void) parser_ptr;
    (void) serializer_ptr;
    Retcode_T retVal = RETCODE_OK;
    retVal = LWM2MObjectSensorDevice_Disable();
    if (RETCODE_OK != retVal)
    {
        printf("Unable to stop the sensor sampling timer \r\n");
    }
    printf("deviceReboot\r\n");
    LWM2M_Reboot();

    return (RC_OK);
}

/** @brief
 *  This function is used to update the factoryReset Resource value into the LWM2M Server*
 */
static retcode_t factoryResetFunc(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    BCDS_UNUSED(parser_ptr);
    BCDS_UNUSED(serializer_ptr);

    printf("factoryReset\r\n");
    return (RC_OK);
}

/** @brief
 *  This function is used to update the errorCode Resource value into the LWM2M Server*
 */
static retcode_t errorCodeFunc(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    retcode_t rc = RC_LWM2M_METHOD_NOT_ALLOWED;

    if ( NULL == parser_ptr)
    {
        rc = util_serialize_array(serializer_ptr, error_codes_count, error_codes);
    }
    return (rc);
}

/** @brief
 *  This function is used to update the resetErrorCode Resource value into the LWM2M Server*
 */
static retcode_t resetErrorCodeFunc(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    (void) parser_ptr;
    (void) serializer_ptr;

#ifdef USE_LOG_FOR_DEVICE
    printf("resetErrorCode\r\n");
#endif

    retcode_t rc = RC_OK;
    error_codes_count = 1;
    error_codes[0] = 0;

    if (started)
    {
        Lwm2m_URI_Path_T errorCodeUriPath = { LWM2MOBJECTS_IX_DEVICE_0, LWM2MOBJECTS_IX_DEVICE_0, DEVICE_RESOURCES_INDEX(errorCode) };
        rc = Lwm2mReporting_resourceChanged(&errorCodeUriPath);
    }

    return rc;
}

/** @brief
 *  This function is used to update the currentTime Resource value into the LWM2M Server*
 */
static retcode_t getCurrentTime(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    retcode_t rc = RC_OK;
    Retcode_T retcode = RETCODE_OK;
    uint64_t sntpTimeStamp = 0UL;
    uint32_t timeLapseInMs = 0UL;

    if (parser_ptr != NULL)
    {
        /* Update current time */
        int32_t newTime;
        rc = Lwm2mParser_getTime(parser_ptr, &newTime);
        if (rc != RC_OK)
        {
            return (rc);
        }
        if (newTime < 0)
        {
            return RC_PLATFORM_ERROR;
        }

        retcode = SNTP_SetTime((uint64_t) newTime);
        if (RETCODE_OK != retcode)
        {
            Retcode_RaiseError(retcode);
            rc = RC_PLATFORM_ERROR;
        }

        /* URI values from array by index:              /<ObjectId>         /<ObjectId-Inst>     /<ResourceId> */
        if (started)
        {
            Lwm2m_URI_Path_T currentTimeUriPath = { LWM2MOBJECTS_IX_DEVICE_0, LWM2MOBJECTS_IX_DEVICE_0, DEVICE_RESOURCES_INDEX(currentTime) };
            rc = Lwm2mReporting_resourceChanged(&currentTimeUriPath);
        }
        printf("currentTime adjusted\r\n");
        return rc;
    }
    else
    {
        retcode = SNTP_GetTimeFromSystem(&sntpTimeStamp, &timeLapseInMs);
        if (RETCODE_OK != retcode)
        {
            Retcode_RaiseError(retcode);
            rc = RC_PLATFORM_ERROR;
        }
        if (RC_OK == rc)
        {
            rc = Lwm2mSerializer_serializeTime(serializer_ptr, (uint32_t) sntpTimeStamp);
        }
    }
    return (rc);
}

/** The description is in the configuration header */
static retcode_t lwm2mDeviceUtcOffset(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    retcode_t rc = RC_OK;
    StringDescr_T newOffset = { 0, 0 };

    if (parser_ptr == NULL)
    {
        Lwm2m_utcOffset.length = strlen((const char *) DeviceUtcOffset);
        rc = Lwm2mSerializer_serializeString(serializer_ptr, &Lwm2m_utcOffset);
    }

    else
    {
        rc = Lwm2mParser_getString(parser_ptr, &newOffset);

        if (RC_OK == rc)
        {
            if (newOffset.length < MAXIMUM_LENGTH_OF_UTCOFFSET)
            {
                memset(DeviceUtcOffset, 0, sizeof(DeviceUtcOffset));
                strncpy((char *) Lwm2m_utcOffset.start, newOffset.start, newOffset.length);
            }
            else
            {
                rc = RC_LWM2M_ENTITY_TOO_LARGE;
            }
        }

    }

    return (rc);
}

/** @brief
 *  This function is used to update the tzone Resource value into the LWM2M Server*
 */
static retcode_t getTimeZone(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    retcode_t rc = RC_OK;
    StringDescr_T newTz = { 0, 0 };

    if (NULL == parser_ptr)
    {
        Lwm2m_tzone.length = strlen((const char *) DeviceTimezone);
        rc = Lwm2mSerializer_serializeString(serializer_ptr, &Lwm2m_tzone);
    }

    else
    {
        rc = Lwm2mParser_getString(parser_ptr, &newTz);

        if (RC_OK == rc)
        {
            if (newTz.length < MAXIMUM_LENGTH_OF_TIMEZONE)
            {
                memset(DeviceTimezone, 0, sizeof(DeviceTimezone));
                strncpy((char*) Lwm2m_tzone.start, newTz.start, newTz.length);
            }
            else
            {
                rc = RC_LWM2M_ENTITY_TOO_LARGE;
            }
        }
        if (RC_OK == rc)
        {
            Lwm2m_URI_Path_T timeZoneUriPath = { LWM2MOBJECTS_IX_DEVICE_0, LWM2MOBJECTS_IX_DEVICE_0, DEVICE_RESOURCES_INDEX(Timezone) };
            rc = Lwm2mReporting_resourceChanged(&timeZoneUriPath);
        }
    }

    return (rc);
}

/** @brief
 *  This function is used to update the getBinding Resource value into the LWM2M Server*
 */
static retcode_t lwm2mDeviceGetBinding(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    retcode_t rc = RC_OK;

    if (parser_ptr != NULL)
    {
        rc = RC_LWM2M_METHOD_NOT_ALLOWED;
    }

    else
    {
        StringDescr_T bindingObject;
        bindingObject.length = Lwm2m_deviceBindingAttributes[LWM2MDeviceResourceInfo.binding].length;
        bindingObject.start = (const char *) Lwm2m_deviceBindingAttributes[LWM2MDeviceResourceInfo.binding].start;

        rc = Lwm2mSerializer_serializeString(serializer_ptr,
                &bindingObject);
    }

    return (rc);

}

/** @brief
 *  This function is used to update the Available Power Sources Resource value into the LWM2M Server*
 */
static retcode_t availablePowerSourcesfunc(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    retcode_t rc = RC_LWM2M_METHOD_NOT_ALLOWED;
    uint32_t ResourceValue = POWER_SOURCE_BATTERY;/* Resource value is set to 1 as XDK has internal battery only */
    if (NULL != parser_ptr)
    {
        return (RC_LWM2M_METHOD_NOT_ALLOWED);
    }
    if ( NULL == parser_ptr)
    {
        rc = Lwm2mSerializer_startSerializingResourceArray(serializer_ptr);
        if (RC_OK == rc)
        {
            rc = Lwm2mSerializer_setResourceInstanceId(serializer_ptr, 0); // index=0
        }
        if (RC_OK == rc)
        {
            rc = Lwm2mSerializer_serializeInt(serializer_ptr, (uint32_t) ResourceValue);
        }
        if (RC_OK == rc)
        {
            rc = Lwm2mSerializer_setResourceInstanceId(serializer_ptr, 1); // index=1
        }
        if (RC_OK == rc)
        {
            ResourceValue = POWER_SOURCE_USB;
            rc = Lwm2mSerializer_serializeInt(serializer_ptr, (uint32_t) ResourceValue);
        }
        if (RC_OK == rc)
        {
            rc = Lwm2mSerializer_endSerializingResourceArray(serializer_ptr);
        }
    }
    return (rc);
}

/** @brief
 *  This function is used to update the Power source Voltage Resource value into the LWM2M Server*
 */
static retcode_t powerSourceVoltageFunc(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    retcode_t rc = RC_LWM2M_METHOD_NOT_ALLOWED;
    uint32_t OutputVoltage = 0UL;
    if (NULL != parser_ptr)
    {
        return (RC_LWM2M_METHOD_NOT_ALLOWED);
    }
    if ( NULL == parser_ptr)
    {
        rc = Lwm2mSerializer_startSerializingResourceArray(serializer_ptr);
        if (RC_OK == rc)
        {
            rc = Lwm2mSerializer_setResourceInstanceId(serializer_ptr, 0); //Index=0 Battery Voltage
        }
        if (RC_OK == rc)
        {
            Retcode_T retval = BatteryMonitor_MeasureSignal(&OutputVoltage);
            if (retval != RETCODE_OK)
            {
                rc = RC_PLATFORM_ERROR;
            }
        }
        if (RC_OK == rc)
        {
            rc = Lwm2mSerializer_serializeInt(serializer_ptr, (uint32_t) OutputVoltage);
        }
        if (RC_OK == rc)
        {
            rc = Lwm2mSerializer_setResourceInstanceId(serializer_ptr, 1); //Index=1 USB Voltage
        }
        if (RC_OK == rc)
        {
            OutputVoltage = 5000; /* USB voltage source is 5V */
            rc = Lwm2mSerializer_serializeInt(serializer_ptr, (uint32_t) OutputVoltage);
        }
        if (RC_OK == rc)
        {
            rc = Lwm2mSerializer_endSerializingResourceArray(serializer_ptr);
        }
    }
    return (rc);
}

/** @brief
 *  This function is used to update the batteryLevel Resource value into the LWM2M Server*
 */
static retcode_t batteryLevelFunc(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    retcode_t rc = RC_LWM2M_METHOD_NOT_ALLOWED;
    uint32_t OutputVoltage = 0UL;
    Retcode_T retval = RETCODE_OK;
    if (NULL != parser_ptr)
    {
        return (RC_LWM2M_METHOD_NOT_ALLOWED);
    }
    if ( NULL == parser_ptr)
    {
        retval = BatteryMonitor_MeasureSignal(&OutputVoltage);
        if (RETCODE_OK == retval)
        {
            if (OutputVoltage >= BATTERY_FULL_VOLTAGE)
            {
                rc = Lwm2mSerializer_serializeInt(serializer_ptr, 100);
            }
            else /*(OutputVoltage < BATTERY_FULL_VOLTAGE) */
            {
                OutputVoltage = (100 * OutputVoltage) / BATTERY_FULL_VOLTAGE;
                rc = Lwm2mSerializer_serializeInt(serializer_ptr, OutputVoltage);
            }
        }
        else
        {
            printf("Unable to get the battery level\r\n");
            rc = RC_PLATFORM_ERROR;
        }
    }
    return rc;
}

/** @brief
 *  This function is used to update the battery Status Resource value into the LWM2M Server*
 */
static retcode_t batteryStatusFunc(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    retcode_t rc = RC_LWM2M_METHOD_NOT_ALLOWED;
    Retcode_T retval = RETCODE_OK;
    BSP_ChargeState_T Batterystate = BSP_XDK_CHARGE_STATUS_LOW;
    USBD_State_TypeDef usbState = USBD_GetUsbState();
    uint32_t BatteryVoltage = 0UL;

    if (NULL != parser_ptr)
    {
        return (RC_LWM2M_METHOD_NOT_ALLOWED);
    }
    if ( NULL == parser_ptr)
    {
        retval = BatteryMonitor_MeasureSignal(&BatteryVoltage);
        if (RETCODE_OK == retval)
        {
            retval = BSP_Charger_BQ2407X_CheckStatus(&Batterystate, BatteryVoltage);
        }
        if (RETCODE_OK == retval)
        {
            switch (usbState)
            {
            case USBD_STATE_SUSPENDED:
                case USBD_STATE_NONE:
                case USBD_STATE_LASTMARKER:
                case USBD_STATE_DEFAULT:
                if ((Batterystate == BSP_XDK_CHARGE_STATUS_CRITICAL) || (Batterystate == BSP_XDK_CHARGE_STATUS_LOW))
                {
                    /*The battery is Low*/
                    rc = Lwm2mSerializer_serializeInt(serializer_ptr, INT32_C(4));
                }
                else
                {
                    /*The battery is Normal & not on Power */
                    rc = Lwm2mSerializer_serializeInt(serializer_ptr, INT32_C(0));
                }

                break;
            case USBD_STATE_ATTACHED:
                case USBD_STATE_POWERED:
                case USBD_STATE_ADDRESSED:
                case USBD_STATE_CONFIGURED:
                if (Batterystate != BSP_XDK_CHARGE_STATUS_FULL)
                {
                    /*The battery is charging currently*/
                    rc = Lwm2mSerializer_serializeInt(serializer_ptr, INT32_C(1));
                }
                else
                {
                    /*The battery is fully charged and usb is connected*/
                    rc = Lwm2mSerializer_serializeInt(serializer_ptr, INT32_C(2));
                }
                break;
            }
        }
        else
        {
            /*The battery information not available */
            rc = Lwm2mSerializer_serializeInt(serializer_ptr, INT32_C(6));
        }
    }
    return rc;
}

/* global variables ********************************************************* */

LWM2MObjectDevice_Resources_T LWM2MObjectDeviceResources =
        {
                { 0, LWM2M_STRING_RO((char *) DeviceManufacturer) },
                { 1, LWM2M_STRING_RO((char *) DeviceModelNumber) },
                { 2, LWM2M_DYNAMIC(serialNumber_RO) | LWM2M_READ_ONLY },
                { 3, LWM2M_DYNAMIC(firmwareVersionNumber_RO) | LWM2M_READ_ONLY },
                { 4, LWM2M_FUNCTION(deviceRebootFunc) },
                { 5, LWM2M_FUNCTION(factoryResetFunc) },
                { 6, LWM2M_DYNAMIC(availablePowerSourcesfunc) | LWM2M_READ_ONLY },
                { 7, LWM2M_DYNAMIC(powerSourceVoltageFunc) | LWM2M_READ_ONLY },
                { 9, LWM2M_DYNAMIC(batteryLevelFunc) | LWM2M_READ_ONLY },
                { 11, LWM2M_DYNAMIC_ARRAY(errorCodeFunc) | LWM2M_READ_ONLY },
                { 12, LWM2M_FUNCTION(resetErrorCodeFunc) },
                { 13, LWM2M_DYNAMIC(getCurrentTime) },
                { 14, LWM2M_DYNAMIC(lwm2mDeviceUtcOffset) },
                { 15, LWM2M_DYNAMIC(getTimeZone) },
                { 16, LWM2M_DYNAMIC(lwm2mDeviceGetBinding) | LWM2M_READ_ONLY },
                { 20, LWM2M_DYNAMIC(batteryStatusFunc) | LWM2M_READ_ONLY },
        };

/** Refer interface header for description */
void LWM2MObjectDevice_Init(void)
{
    Retcode_T retcode = BatteryMonitor_Init();
    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseError(retcode);
    }

    started = false;
}

/** Refer interface header for description */
void LWM2MObjectDevice_Enable(void)
{
    started = true;
}

/** Refer interface header for description */
void LWM2MObjectDevice_NotifyTimeChanged(void)
{
    if (started)
    {
        /* Here URI_Path is the path points to the "Current Time" Resource */
        Lwm2m_URI_Path_T currentTimeUriPath = { LWM2MOBJECTS_IX_DEVICE_0, LWM2MOBJECTS_IX_DEVICE_0, DEVICE_RESOURCES_INDEX(currentTime) };
        retcode_t rc = Lwm2mReporting_resourceChanged(&currentTimeUriPath);
        if (RC_OK != rc && RC_COAP_SERVER_SESSION_ALREADY_ACTIVE != rc)
        {
        printf("Could not send time notification " RC_RESOLVE_FORMAT_STR, RC_RESOLVE((int)rc));
        }
    }
}

/**@} */

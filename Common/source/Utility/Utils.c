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
#define BCDS_MODULE_ID XDK_COMMON_ID_UTILS

/* own header files */
#include "XDK_Utils.h"

/* system header files */
#include <stdio.h>
#include <string.h>
#include "XDK_FOTA.h"
#include "XdkVersion.h"
#include "BCDS_NVMConfig.h"
#include "BCDS_MCU_Watchdog.h"

/* additional interface header files */

/* constant definitions ***************************************************** */

/* inline functions ********************************************************* */

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
 * @param[in] appVersion - 32 bit value containing the FOTA version information
 *
 * @param[in] xdkVersion - 32 bit value containing the XDK SW version information
 *
 * @param[in/out] str - pointer to the parsed version string
 *
 * @param [out] Returns index where the application version starts
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
static uint8_t* convert32IntegerToVersionString(uint32_t appVersion, uint32_t xdkVersion, uint8_t* str)
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

/**
 * @brief Converts the IP address from little endian format to big endian format if necessary
 *
 * @param[in] uint32_t
 *            IP Address
 *
 * @param [out] Returns Ip address in big endian format
 */
static uint32_t InterChangeEndianIfNecessaryU32(uint32_t dataU32)
{
    uint32_t result = 0UL;
    uint32_t dataCopy = 1;
    uint8_t *ptr = (uint8_t *) &dataCopy;
    if (1 == ptr[0])
    {
        /* Converts little endian to big endian */
        ptr[0] = ((uint8_t*) &dataU32)[3];
        ptr[1] = ((uint8_t*) &dataU32)[2];
        ptr[2] = ((uint8_t*) &dataU32)[1];
        ptr[3] = ((uint8_t*) &dataU32)[0];
        result = dataCopy;
    }
    else
    {
        /* If the endianess is big endian, then the same data is stored  */
        result = dataU32;
    }
    return result;
}

/**
 * @brief This function to Get the Firmware Version by combining the XDK SW Release (i.e., Workbench Release) version (i.e., 3.0.1) & Fota Container Firmware Version (i.e., 0.0.1)
 *          So , the GetFwVersionInfo returns the Version Information as like (i.e., 3.0.1-0.0.1 )
 *
 * @param[out] versionNo - Firmware Version (i.e., 3.0.1-0.0.1 if new bootloader or 3.0.1-xx.xx.xx for old bootloader).
 */
Retcode_T Utils_GetXdkVersionString(uint8_t * verstring)
{
    Retcode_T retval = RETCODE_OK;
    uint32_t xdkVersion;
    uint32_t fwVersionValue = UINT32_C(0x00000000);
    uint8_t* startOfAppVersionPtr = NULL;
    uint8_t index = 0;
    if (NULL == verstring)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }

#if XDK_FOTA_ENABLED_BOOTLOADER == 1
    Fota_PartitionInfo_T partitionInfo;
    partitionInfo.Partition = FOTA_PARTITION_PRIMARY;
    retval = FOTA_ReadFirmwareVersion(partitionInfo, &fwVersionValue);
#endif
    if (RETCODE_OK == retval)
    {
        xdkVersion = XdkVersion_GetVersion();
        startOfAppVersionPtr = convert32IntegerToVersionString(fwVersionValue,
                xdkVersion, verstring);

        index = startOfAppVersionPtr - verstring;
        verstring[index - UINT8_C(1)] = '-';
#if XDK_FOTA_ENABLED_BOOTLOADER == 0
        /*Replace with default string in case the FOTA version is not there*/
        verstring[index] = 'x';
        verstring[index + UINT8_C(1)] = 'x';
        verstring[index + UINT8_C(2)] = '.';
        verstring[index + UINT8_C(3)] = 'x';
        verstring[index + UINT8_C(4)] = 'x';
        verstring[index + UINT8_C(5)] = '.';
        verstring[index + UINT8_C(6)] = 'x';
        verstring[index + UINT8_C(7)] = 'x';
        verstring[index + UINT8_C(8)] = '\0';
#endif
    }
    return retval;
}

/** Refer interface header for description */
Retcode_T Utils_GetMacInfoFromNVM(Utils_NVMMacInfo_T nvmMacInfo, uint8_t *data)
{
    uint8_t isAddressValid = UINT8_C(0);
    Retcode_T retcode = RETCODE_OK;
    uint8_t addressLength = UINT8_C(0x06);
    uint8_t bleMacUserBit = UINT8_C(0x02);
    uint8_t WifiMacUserBit = UINT8_C(0x01);

    retcode = NVM_ReadUInt8(&NVMUser, NVM_ITEM_CONTENT_INDEX, &isAddressValid);
    switch (nvmMacInfo)
    {
    case UTILS_BLE_MAC_DATA:
        if (RETCODE_OK == retcode)
        {
            if (bleMacUserBit == (isAddressValid & bleMacUserBit))
            {
                retcode = NVM_Read(&NVMUser, NVM_BTLE_MAC_INDEX, data, addressLength);
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NVM_GET_DATA_FAILED);
            }
        }
        break;
    case UTILS_WIFI_MAC_DATA:
        if (RETCODE_OK == retcode)
        {
            if (WifiMacUserBit == (isAddressValid & WifiMacUserBit))
            {
                retcode = NVM_Read(&NVMUser, NVM_WIFI_MAC_INDEX, data, addressLength);
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NVM_GET_DATA_FAILED);
            }
        }
        break;
    default:
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
        break;
    }
    return retcode;
}

/** Refer interface header for description */
void Utils_PrintResetCause(void)
{
    bool wdgResetFlag = MCU_Watchdog_IsResetCausedByWatchdog();
    if (true == wdgResetFlag)
    {
        printf("************RESET CAUSE: WATCHDOG************ \r\n");
    }
}

/** Refer interface header for description */
Retcode_T Utils_ConvertIpStringToNumber(const char* ipDottedQuad, uint32_t *ipAddress)
{
    Retcode_T retcode = RETCODE_OK;
    unsigned int byte0 = 0;
    unsigned int byte1 = 0;
    unsigned int byte2 = 0;
    unsigned int byte3 = 0;
    char dummyString[2];
    uint32_t value = 0UL;

    if ((NULL == ipDottedQuad)||(NULL == ipAddress))
    {
        return RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        /* The dummy string with specifier %1s searches for a non-whitespace char
         * after the last number. If it is found, the result of sscanf will be 5
         * instead of 4, indicating an erroneous format of the ip-address.
         */
        if (sscanf(ipDottedQuad, "%u.%u.%u.%u%1s",
                &byte3, &byte2, &byte1, &byte0, dummyString) == 4)
        {
            if ((byte3 < 256) && (byte2 < 256) && (byte1 < 256) && (byte0 < 256))
            {
                value = (byte3 << 24) + (byte2 << 16) + (byte1 << 8) + byte0;
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
            }
        }
        else
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
        }
    }
    if (RETCODE_OK == retcode)
    {
        value = InterChangeEndianIfNecessaryU32(value);
        *ipAddress = value;
    }
    else
    {
        *ipAddress = 0UL;
    }
    return retcode;
}

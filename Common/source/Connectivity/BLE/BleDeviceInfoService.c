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

/**  @brief
 *
 * This module contains Ble Device Information Service Register API along with the Characteristics
 *
 * ****************************************************************************/

/* module includes ********************************************************** */
/* Public Header ************************************************************ */
#include "BCDS_Basics.h"
#include "BCDS_Retcode.h"
#include "BleTypes.h"
/*lint -e652 -e830 */
#include "attserver.h"

/* own header files */
#include "BleDeviceInfoService.h"

/**
 * @brief Attribute handle for the Device Information service
 */
static AttServiceAttribute DeviceInformationServiceAttribute;

/**
 * Device Information Service UUID as per the BLE Specifications
 */
static const uint8_t DeviceInformationUUID[2] = { 0x0A, 0x18 };
/**
 * @brief structure to hold the Characteristic properties of the Device Information Service
 */
struct DeviceInfoService_Characteristic_S
{
    AttUuid UUIDType;
    Att16BitCharacteristicAttribute Characteristic;
    AttAttribute CharacteristicAttribute;
};

/**
 * @brief Typedef to represent the Characteristic property of Model Number
 */
typedef struct DeviceInfoService_Characteristic_S DeviceInfoService_Characteristic_T;

static DeviceInfoService_Characteristic_T GetModelNumberCharacteristicProperties =
        {
                { 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
        };

static DeviceInfoService_Characteristic_T GetManufacturerCharacteristicProperties =
        {
                { 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
        };
static DeviceInfoService_Characteristic_T GetSoftwareRevisionCharacteristicProperties =
        {
                { 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0, { 0 } },
                { 0, 0, 0, { 0 }, { 0 }, 0, 0, 0, 0 }
        };

/**
 * Model Number information
 * Format: {"MODEL NUMBER"}
 */
static uint8_t ModelNumberCharacteristicValue[6U];

/**
 * Manufacturer information
 * Format: {"Manufacturer"}
 */
static uint8_t ManufacturerCharacteristicValue[17U];

/**
 * Serial Number information
 * Format: {"Serial Number"}
 */
static uint8_t SoftwareRevisionCharacteristicValue[14U];

/**
 * Model Number String Characteristics as per the BLE Specifications
 */
static const uint16_t ModelNumberUUID = 0x2A24;

/**
 * Manufacturer String Characteristics as per the BLE Specifications
 */
static const uint16_t ManufacturerUUID = 0x2A29;

/**
 * SoftwareRevision String Characteristics as per the BLE Specifications
 */
static const uint16_t SoftwareRevisionUUID = 0x2A28;

/**@brief This function handles the Device Information service events.
 * @param[i] serverCallbackParms Parameters holding service event status
 */
static void DeviceInformationServiceCallback(AttServerCallbackParms* serverCallbackParms)
{
    BCDS_UNUSED(serverCallbackParms);
}

Retcode_T BleDeviceInformationService_Initialize(BleDeviceInformationService_CharacteristicValue_T characteristicValue)
{
    Retcode_T retVal = RETCODE_OK;
    if ((NULL == characteristicValue.ModelNumber) || (NULL == characteristicValue.Manufacturer) || (NULL == characteristicValue.SoftwareRevision))
    {
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    if (RETCODE_OK == retVal)
    {
        memcpy(ModelNumberCharacteristicValue, characteristicValue.ModelNumber, strlen((const char*)characteristicValue.ModelNumber));
        memcpy(ManufacturerCharacteristicValue, characteristicValue.Manufacturer, strlen((const char*)characteristicValue.Manufacturer));
        memcpy(SoftwareRevisionCharacteristicValue, characteristicValue.SoftwareRevision, strlen((const char*)characteristicValue.SoftwareRevision));
    }
    return retVal;
}

Retcode_T BleDeviceInformationService_Register(void)
{
    BleStatus registerStatus = BLESTATUS_SUCCESS;
    AttStatus attRequestStatus;
    Retcode_T retVal = RETCODE_OK;

    ATT_SERVER_SecureDatabaseAccess();

    attRequestStatus = ATT_SERVER_RegisterServiceAttribute(ATTPDU_SIZEOF_16_BIT_UUID, (uint8_t *) DeviceInformationUUID,
            DeviceInformationServiceCallback, (AttServiceAttribute *) &(DeviceInformationServiceAttribute));

    if (ATTSTATUS_SUCCESS != attRequestStatus)
    {
        /*
         * - ATTSTATUS_DATABASE_FROZEN (only in case ATT_HANDLES_ARE_FREEZABLE)
         *   if ATT_SERVER_FreezeDatabase() has already been called,
         *   in which case more change is accepted in the database.
         * - ATTSTATUS_BUSY if there is at least one link layer connection,
         *   in which case the database's structure shall not be modified.
         * - ATTSTATUS_DATABASE_FULL if there is no room in the handle's space
         *   to APPEND this attribute at the back of the database.
         * - ATTSTATUS_INVALID_PARM (only checked if ATT_PARMS_CHECK == 1 ;
         *   asserted otherwise if ATT_DEBUG == 1) if valueLen is not 2 or 16,
         *   or any pointer parameter is NULL.( always asserted) if database is not locked.
         * - ATTSSTATUS_FAILED: if the service memory is already in the database.
         */
        registerStatus = BLESTATUS_FAILED;
    }
    else
    {

        /* Model Number Characteristic */
        GetModelNumberCharacteristicProperties.UUIDType.size = ATT_UUID_SIZE_16;
        GetModelNumberCharacteristicProperties.UUIDType.value.uuid16 = ModelNumberUUID;
        if (ATT_SERVER_AddCharacteristic(
        ATTPROPERTY_READ,
                (Att16BitCharacteristicAttribute*) &GetModelNumberCharacteristicProperties.Characteristic,
                &GetModelNumberCharacteristicProperties.UUIDType,
                ATT_PERMISSIONS_ALLACCESS, strlen((const char*)ModelNumberCharacteristicValue),
                (uint8_t *) &ModelNumberCharacteristicValue[0], 0, 0,
                &DeviceInformationServiceAttribute,
                &GetModelNumberCharacteristicProperties.CharacteristicAttribute) == BLESTATUS_FAILED)
        {
            registerStatus = BLESTATUS_FAILED;
        }
        if (ATTSTATUS_SUCCESS == attRequestStatus)
        {
            /* Manufacturer Characteristic */
            GetManufacturerCharacteristicProperties.UUIDType.size = ATT_UUID_SIZE_16;
            GetManufacturerCharacteristicProperties.UUIDType.value.uuid16 = ManufacturerUUID;
            if (ATT_SERVER_AddCharacteristic(
            ATTPROPERTY_READ,
                    (Att16BitCharacteristicAttribute*) &GetManufacturerCharacteristicProperties.Characteristic,
                    &GetManufacturerCharacteristicProperties.UUIDType,
                    ATT_PERMISSIONS_ALLACCESS, strlen((const char*)ManufacturerCharacteristicValue),
                    (uint8_t *) &ManufacturerCharacteristicValue[0], 0, 0,
                    &DeviceInformationServiceAttribute,
                    &GetManufacturerCharacteristicProperties.CharacteristicAttribute) == BLESTATUS_FAILED)
            {
                registerStatus = BLESTATUS_FAILED;
            }
        }

        if (ATTSTATUS_SUCCESS == attRequestStatus)
        {
            /* Software Revision Characteristic */
            GetSoftwareRevisionCharacteristicProperties.UUIDType.size = ATT_UUID_SIZE_16;
            GetSoftwareRevisionCharacteristicProperties.UUIDType.value.uuid16 = SoftwareRevisionUUID;
            if (ATT_SERVER_AddCharacteristic(
            ATTPROPERTY_READ,
                    (Att16BitCharacteristicAttribute*) &GetSoftwareRevisionCharacteristicProperties.Characteristic,
                    &GetSoftwareRevisionCharacteristicProperties.UUIDType,
                    ATT_PERMISSIONS_ALLACCESS, strlen((const char*)SoftwareRevisionCharacteristicValue),
                    (uint8_t *) &SoftwareRevisionCharacteristicValue[0], 0, 0,
                    &DeviceInformationServiceAttribute,
                    &GetSoftwareRevisionCharacteristicProperties.CharacteristicAttribute) == BLESTATUS_FAILED)
            {
                registerStatus = BLESTATUS_FAILED;
            }
        }
    }

    ATT_SERVER_ReleaseDatabaseAccess();

    if (registerStatus != BLESTATUS_SUCCESS)
    {
        retVal = RETCODE(RETCODE_SEVERITY_ERROR,RETCODE_FAILURE);
    }
    else
    {
        retVal = RETCODE_OK;
    }
    return retVal;
}


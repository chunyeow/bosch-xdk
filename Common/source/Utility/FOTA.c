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
 * @brief This module will handle, FOTA related activities
 *       -Firmware validation in the SD Card
 *       -Firmware update from the SD Card
 *
 * @file
 */
/* module includes ********************************************************** */

#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_COMMON_ID_FOTA

/* system header files */
//lint -e49 error in standard libraries suppressed #include <stdint.h>
//lint -e309 error in third party libraries suppressed BCDS_Assert.h
//lint -esym(956,*) /* Suppressing Non const, non volatile static or external variable */
/* own header files */
#include "BCDS_Fota.h"
#include "XDK_FOTA.h"
/* additional interface header files */
#include "XDK_Storage.h"
#include "ff.h"
#include "BCDS_NVMConfig.h"
#include "BCDS_Retcode.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "BCDS_BSP_Board.h"
#include "BCDS_SDCard_Driver.h"
#include "BCDS_BSP_Board.h"
#include "BCDS_CRC.h"
#include "BCDS_MCU_Flash.h"

#define FIRMWARE_NAME               "firmware.bin" /**< Filename to open/write/read from SD-card */


/**
 * @brief Verifies the Header CRC & firmware size of the given fota container.
 *
 * @param[in] header
 * pointer to a buffer holding the fota container header data.
 *
 * @param[in] status
 * #RETCODE_OK if header CRC is passed & firmware size is within the maximum range or an error code otherwise.
 */
static Retcode_T VerifyHeader(FWContainer_Header_T* header)
{
    Retcode_T returnValue = RETCODE_OK;
    uint32_t calculatedCRC = 0;

    if (NULL == header)
    {
        return (RETCODE(RETCODE_SEVERITY_FATAL, (uint32_t ) RETCODE_NULL_POINTER));
    }

    /* We calculate the CRC now */
    CRC32_EATH_STD_INIT(calculatedCRC);

    if (RETCODE_OK == returnValue)
    {
        returnValue = CRC_32_Reverse(CRC32_ETHERNET_REVERSE_POLYNOMIAL, &calculatedCRC , (uint8_t *) header, (sizeof(FWContainer_Header_T) - 4));
        if (RETCODE_OK != returnValue)
        {
            returnValue = RETCODE(RETCODE_SEVERITY_ERROR, FOTA_FW_CRCRUN_FAIL);
        }
    }
    if (RETCODE_OK == returnValue)
    {
        CRC32_INVERSE(calculatedCRC);
    }
    if (RETCODE_OK == returnValue)
    {
        /* Now we can compare the 2 CRC32 */
        if (calculatedCRC != header->HeaderCRC)
        {
            returnValue = RETCODE(RETCODE_SEVERITY_ERROR, FOTA_FW_CRC_FAIL);
        }
    }

    if (RETCODE_OK == returnValue)
    {
        /* Verify if firmware is the right size */
        if (header->FirmwareSize > FOTA_FIRMWARE_MAX_SIZE)
        {
            returnValue = RETCODE(RETCODE_SEVERITY_ERROR, FOTA_RETCODE_FIRMWARE_SIZE_FAIL);
        }
    }
    return returnValue;
}

/**
 * @brief Verifies the firmware CRC of the given fota container partition.
 *
 * @param [in]  partitionInfo
 *              mention the partition information for which header firmware CRC needs to be verified. In-case of FOTA_PARTITION_DOWNLOADED specify the file name
 *              Incase of FOTA_PARTITION_PRIMARY file name is not used, can be NULL.
 *
 * @param [in]  optBuffer
 *              Operation buffer from the application where blocks of the firmware is kept for the CRC calculation.
 *
 * @note
 * -  Make sure minimum buffer size of optBuffer is sizeof(Fota_PartitionInfo_T).
 * -  Storage_Setup & Storage_Enable must be called prior to using this API.
 */
static Retcode_T VerifyImage(Fota_PartitionInfo_T* partitionInfo, uint8_t* optBuffer)
{
    Retcode_T returnValue = RETCODE_OK;
    FWContainer_Header_T *fotaHeader = (FWContainer_Header_T*) optBuffer;
    uint32_t firmwareCRC = 0;
    uint32_t firmwareSize = 0;
    uint32_t calculatedCRC = 0;
    uint32_t binLeft = 0;
    uint32_t i = 0;
    uint32_t addr = 0;

    Storage_Read_T SdcardRead =
            {
                    .FileName = NULL,
                    .ReadBuffer = optBuffer,
                    .BytesToRead = sizeof(FWContainer_Header_T),
                    .ActualBytesRead = 0,
                    .Offset = 0,
            };
    /*Reading the Fota Header info*/
    if (FOTA_PARTITION_PRIMARY == partitionInfo->Partition)
    {
        addr = FOTA_PRIMARY_PARTTITION_START_ADDRESS;
        returnValue = MCU_Flash_Read((uint8_t *) addr, (uint8_t *) optBuffer, sizeof(FWContainer_Header_T));
    }
    else if (FOTA_PARTITION_DOWNLOADED == partitionInfo->Partition)
    {
        SdcardRead.FileName = partitionInfo->PartitionFileName;
        returnValue = Storage_Read(STORAGE_MEDIUM_SD_CARD, &SdcardRead);
    }
    else
    {
        returnValue = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_FOTA_PARTITION_UNKNOWN);
    }

    if (RETCODE_OK == returnValue)
    {
        firmwareCRC = fotaHeader->FirmwareCRC;
        firmwareSize = fotaHeader->FirmwareSize;

        /* Define the loop conditions for the CRC-calculation */
        /* We calculate the CRC now */
        CRC32_EATH_STD_INIT(calculatedCRC);
    }
    if (RETCODE_OK == returnValue)
    {
        binLeft = firmwareSize;
        while (binLeft > 0)
        {
            SdcardRead.BytesToRead = FWCONTAINER_BLOCK_SIZE;
            if (binLeft < FWCONTAINER_BLOCK_SIZE)
            {
                SdcardRead.BytesToRead = binLeft;
            }
            SdcardRead.Offset = (uint32_t) sizeof(FWContainer_Header_T) + (i * (uint32_t) FWCONTAINER_BLOCK_SIZE);

            if (FOTA_PARTITION_DOWNLOADED == partitionInfo->Partition)
            {
                returnValue = Storage_Read(STORAGE_MEDIUM_SD_CARD, &SdcardRead);
            }
            else
            {
                addr += SdcardRead.Offset;
                returnValue = MCU_Flash_Read((uint8_t *) addr, (uint8_t *) optBuffer, sizeof(FWContainer_Header_T));
            }
            if (RETCODE_OK != returnValue)
            {
                break;
            }
            returnValue = CRC_32_Reverse(CRC32_ETHERNET_REVERSE_POLYNOMIAL, &calculatedCRC , (uint8_t *) optBuffer, SdcardRead.BytesToRead);
            if (RETCODE_OK != returnValue)
            {
                returnValue = RETCODE(RETCODE_SEVERITY_ERROR, FOTA_FW_CRCRUN_FAIL);
                break;
            }
            /* handle for next block */
            i += 1;
            binLeft -= SdcardRead.BytesToRead;
        }
    }
    if (RETCODE_OK == returnValue)
    {
        CRC32_INVERSE(calculatedCRC);
    }
    if (RETCODE_OK == returnValue)
    {
        /* Now we can compare the 2 CRC32 */
        if (calculatedCRC != firmwareCRC)
        {
            returnValue = RETCODE(RETCODE_SEVERITY_ERROR, FOTA_FW_CRC_FAIL);
        }
    }
    return returnValue;
}

/* The description is in interface header */
Retcode_T FOTA_ReadHeader(Fota_PartitionInfo_T partitionInfo, FWContainer_Header_T* Header)
{
    Retcode_T returnValue = RETCODE_OK;
    uint32_t addr = 0;
    Storage_Read_T SdcardRead;

    if (NULL == Header)
    {
        return (RETCODE(RETCODE_SEVERITY_FATAL, (uint32_t ) RETCODE_NULL_POINTER));
    }
    if (FOTA_PARTITION_DOWNLOADED == partitionInfo.Partition)
    {
        if (NULL == partitionInfo.PartitionFileName)
        {
            returnValue = RETCODE(RETCODE_SEVERITY_FATAL, (uint32_t ) RETCODE_NULL_POINTER);
        }
        else
        {
            SdcardRead.FileName = partitionInfo.PartitionFileName,
            SdcardRead.ReadBuffer = (uint8_t *) Header,
            SdcardRead.BytesToRead = sizeof(FWContainer_Header_T),
            SdcardRead.ActualBytesRead = 0,
            SdcardRead.Offset = 0,
            returnValue = Storage_Read(STORAGE_MEDIUM_SD_CARD, &SdcardRead);
        }
    }
    else if (FOTA_PARTITION_PRIMARY == partitionInfo.Partition)
    {
        addr = FOTA_PRIMARY_PARTTITION_START_ADDRESS;
        returnValue = MCU_Flash_Read((uint8_t *) addr, (uint8_t *) Header, sizeof(FWContainer_Header_T));
    }
    else
    {
        returnValue = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_FOTA_PARTITION_UNKNOWN);
    }
    return returnValue;
}

/* The description is in interface header */
Retcode_T FOTA_ReadFirmwareVersion(Fota_PartitionInfo_T partitionInfo, uint32_t * FirmwareVer)
{
    Retcode_T returnVal = RETCODE_OK;
    uint8_t buffer[sizeof(FWContainer_Header_T)];
    FWContainer_Header_T* fotaHeader = (FWContainer_Header_T *) buffer;

    if (NULL == FirmwareVer)
    {
        return (RETCODE(RETCODE_SEVERITY_FATAL, (uint32_t ) RETCODE_NULL_POINTER));
    }
    if (RETCODE_OK == returnVal)
    {
        returnVal = FOTA_ReadHeader(partitionInfo, fotaHeader);
    }
    if (RETCODE_OK == returnVal)
    {
        *FirmwareVer = fotaHeader->FirmwareVersion;
    }
    return returnVal;
}

/* The description is in interface header */
Retcode_T FOTA_ReadFirmwareCRC(Fota_PartitionInfo_T partitionInfo, uint32_t * firmwareCRC)
{
    Retcode_T returnVal = RETCODE_OK;
    uint8_t buffer[sizeof(FWContainer_Header_T)];
    FWContainer_Header_T* fotaHeader = (FWContainer_Header_T *) buffer;

    if (NULL == firmwareCRC)
    {
        return (RETCODE(RETCODE_SEVERITY_FATAL, (uint32_t ) RETCODE_NULL_POINTER));
    }
    if (RETCODE_OK == returnVal)
    {
        returnVal = FOTA_ReadHeader(partitionInfo, fotaHeader);
    }
    if (RETCODE_OK == returnVal)
    {
        *firmwareCRC = fotaHeader->FirmwareCRC;
    }
    return returnVal;
}

/* The description is in interface header */
Retcode_T FOTA_ValidateSdcardFw(const char* firmwareName)
{
    Retcode_T returnVal = RETCODE_OK;
    FRESULT fileSystemResult = FR_OK;
    uint8_t buffer[sizeof(FWContainer_Header_T)];
    FWContainer_Header_T* fotaHeader = (FWContainer_Header_T *) buffer;

    Fota_PartitionInfo_T partitionInfo =
            {
                    .Partition = FOTA_PARTITION_DOWNLOADED,
                    .PartitionFileName = firmwareName,
            };
    if (NULL == firmwareName)
    {
        return (RETCODE(RETCODE_SEVERITY_FATAL, (uint32_t ) RETCODE_NULL_POINTER));
    }

    if (RETCODE_OK == returnVal)
    {
        returnVal = FOTA_ReadHeader(partitionInfo, fotaHeader);
    }
    if (RETCODE_OK == returnVal)
    {
        returnVal = VerifyHeader(fotaHeader);
    }
    if (RETCODE_OK == returnVal)
    {
        returnVal = VerifyImage(&partitionInfo, buffer);
    }
    if (RETCODE_OK == returnVal)
    {
        fileSystemResult = f_rename(firmwareName, FIRMWARE_NAME);
        if (FR_EXIST == fileSystemResult)
        {
            fileSystemResult = f_unlink(FIRMWARE_NAME);
            if (FR_OK == fileSystemResult)
            {
                fileSystemResult = f_rename(firmwareName, FIRMWARE_NAME);
            }
        }
    }
    if (FR_OK != fileSystemResult)
    {
        returnVal = RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_FOTA_FILESYSTEM_ERROR);
    }
    return returnVal;
}

/* The description is in interface header */
Retcode_T FOTA_UpdateSdcardFw(void)
{
    Retcode_T returnVal = RETCODE_OK;
    uint8_t newFirmwareFlag = 1;
    FRESULT fileSystemResult = FR_OK;
    FILINFO fileInfo;
    Fota_PartitionInfo_T partitionInfo;
    uint32_t firmwareCRC = 0;
#if _USE_LFN
    fileInfo.lfname = NULL;
#endif

    fileSystemResult = f_stat(FIRMWARE_NAME, &fileInfo);

    if (FR_OK != fileSystemResult)
    {
        returnVal = RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_STORAGE_SDCARD_STAT_FAILED);
    }
    if (returnVal == RETCODE_OK)
    {
        partitionInfo.Partition = FOTA_PARTITION_DOWNLOADED;
        partitionInfo.PartitionFileName = FIRMWARE_NAME;
        returnVal = FOTA_ReadFirmwareCRC(partitionInfo, &firmwareCRC);
    }
    if (returnVal == RETCODE_OK)
    {
        returnVal = NVM_WriteUInt32(&NVMUser, NVM_ITEM_ID_NEW_FW_CRC, &firmwareCRC);
    }
    if (returnVal == RETCODE_OK)
    {
        returnVal = NVM_WriteUInt8(&NVMUser, NVM_ITEM_ID_IS_NEW_FW, &newFirmwareFlag);
    }
    if (returnVal == RETCODE_OK)
    {
        returnVal = NVM_Flush(&NVMUser);
    }
    if (returnVal == RETCODE_OK)
    {
        /*reboot device*/
        BSP_Board_SoftReset();
    }
    return returnVal;
}


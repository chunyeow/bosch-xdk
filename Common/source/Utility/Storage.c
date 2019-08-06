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
 * This module handles the Storage features. It supports :
 * - SD Card
 * - WiFi file system
 *
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_STORAGE

#if XDK_UTILITY_STORAGE
/* own header files */
#include "XDK_Storage.h"

/* additional interface header files */
#include "BCDS_WlanNetworkConnect.h"
#include "BCDS_SDCard_Driver.h"
#include "Protected/WLAN.h"
#include "FreeRTOS.h"
#include "task.h"
#include "WifiStorage.h"
#include "ff.h"
#include "fs.h"

static FATFS StorageSDCardFatFSObject; /** File system specific objects */
static bool isSdcardInitialized = false;

/* constant definitions ***************************************************** */

/**< Macro to define default logical drive */
#define STORAGE_DEFAULT_LOGICAL_DRIVE           ""

/**< Macro to define force mount */
#define STORAGE_SDCARD_FORCE_MOUNT              UINT8_C(1)

/**< SD Card Drive 0 location */
#define STORAGE_SDCARD_DRIVE_NUMBER             UINT8_C(0)

/**< File seek to the first location */
#define STORAGE_SEEK_FIRST_LOCATION             UINT8_C(0)

/* local variables ********************************************************** */

/**< Storage Setup */
static Storage_Setup_T StorageSetup;

/**< Storage enable setup */
static Storage_Setup_T StorageEnableStatus;
/* global variables ********************************************************* */

/**
 * @brief This API will transfer the data from one file to another in sdcard.
 *
 * @param[in] readCredentials
 * Pointer to the Storage read structure.
 *
 * @param[in] writeCredentials
 * Pointer to the Storage write structure.
 *
 *  @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T SdcardToSdcardTransfer(Storage_Read_T* readCredentials, Storage_Write_T* writeCredentials)
{
    Retcode_T retcode = RETCODE_OK;
    FRESULT fileSystemResult = FR_OK;
    FILINFO fileInfo;
#if _USE_LFN
    fileInfo.lfname = NULL;
#endif
    uint32_t blocks, rem = 0;
    fileSystemResult = f_stat(readCredentials->FileName, &fileInfo);
    if (FR_OK == fileSystemResult)
    {
        blocks = (fileInfo.fsize) / (readCredentials->BytesToRead);
        rem = (fileInfo.fsize) % (readCredentials->BytesToRead);
        readCredentials->Offset = 0;
        writeCredentials->Offset = 0;

        /* Copy source to destination */
        while (blocks--)
        {
            retcode = Storage_Read(STORAGE_MEDIUM_SD_CARD, readCredentials);
            if (RETCODE_OK == retcode)
            {
                retcode = Storage_Write(STORAGE_MEDIUM_SD_CARD, writeCredentials);
            }
            if (RETCODE_OK != retcode)
            {
                break;
            }
            readCredentials->Offset = readCredentials->Offset + (readCredentials->BytesToRead);
            writeCredentials->Offset = readCredentials->Offset;
        }
        if ((rem) && (RETCODE_OK == retcode))
        {
            readCredentials->BytesToRead = rem;
            retcode = Storage_Read(STORAGE_MEDIUM_SD_CARD, readCredentials);
            if (RETCODE_OK == retcode)
            {
                writeCredentials->BytesToWrite = rem;
                retcode = Storage_Write(STORAGE_MEDIUM_SD_CARD, writeCredentials);
            }
        }
    }
    else
    {
        retcode = RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_STORAGE_ERROR_IN_GETTING_FILE_STATUS);
    }
    return retcode;
}

/**
 * @brief This API will transfer the data from sdcard to wifi.
 *
 * @param[in] readCredentials
 * Pointer to the Storage read structure.
 *
 * @param[in] writeCredentials
 * Pointer to the Storage write structure.
 *
 *  @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T SdcardToWifiTransfer(Storage_Read_T* readCredentials, Storage_Write_T* writeCredentials)
{
    Retcode_T retcode = RETCODE_OK;
    FRESULT fileSystemResult = FR_OK;
    FILINFO fileInfo;
#if _USE_LFN
    fileInfo.lfname = NULL;
#endif
    uint32_t blocks = 0, rem = 0, length = 0;
    int32_t WifiFileHandle = INT32_C(-1);
    _i32 WifiRetVal = INT32_C(-1);
    fileSystemResult = f_stat(readCredentials->FileName, &fileInfo);
    if (FR_OK == fileSystemResult)
    {
        retcode = WifiStorage_GetFileStatus((const uint8_t *) writeCredentials->FileName, &length);
        /* checking if file present is WiFi file system is empty or not */
        if ((RETCODE_OK == retcode) || (Retcode_GetCode(retcode) == (Retcode_T) RETCODE_STORAGE_FILE_HAS_NOT_BEEN_CLOSE_CORRECTLY))
        {
            retcode = WifiStorage_FileDelete((const uint8_t *) writeCredentials->FileName, &WifiFileHandle);
        }
        if ((Retcode_GetCode(retcode) == (Retcode_T) RETCODE_STORAGE_FILE_DOES_NOT_EXIST_WIFI_MEDIA) || (RETCODE_OK == retcode))
        {
            retcode = WifiStorage_FileOpen((const uint8_t *) writeCredentials->FileName, &WifiFileHandle, WIFI_STORAGE_FOPEN_CREATE_MODE);
        }
    }
    if ((FR_OK == fileSystemResult) && (RETCODE_OK == retcode))
    {
        blocks = (fileInfo.fsize) / (readCredentials->BytesToRead);
        rem = (fileInfo.fsize) % (readCredentials->BytesToRead);
        readCredentials->Offset = 0;
        writeCredentials->Offset = 0;

        /* Copy source to destination */
        while (blocks--)
        {
            retcode = Storage_Read(STORAGE_MEDIUM_SD_CARD, readCredentials);
            if (RETCODE_OK == retcode)
            {
                WifiRetVal = sl_FsWrite(WifiFileHandle, writeCredentials->Offset, writeCredentials->WriteBuffer, writeCredentials->BytesToWrite);
                if ((_i32) readCredentials->BytesToRead != WifiRetVal)
                {
                    retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_ERROR_IN_FILE_WRITE);
                    break;
                }
            }
            if (RETCODE_OK != retcode)
            {
                break;
            }
            readCredentials->Offset = readCredentials->Offset + (readCredentials->BytesToRead);
            writeCredentials->Offset = readCredentials->Offset;
        }
    }
    if ((rem) && (FR_OK == fileSystemResult) && (RETCODE_OK == retcode))
    {
        readCredentials->BytesToRead = rem;
        retcode = Storage_Read(STORAGE_MEDIUM_SD_CARD, readCredentials);
        if (RETCODE_OK == retcode)
        {
            WifiRetVal = sl_FsWrite(WifiFileHandle, writeCredentials->Offset, writeCredentials->WriteBuffer, rem);
            if ((_i32) rem != WifiRetVal)
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_ERROR_IN_FILE_WRITE);
            }
        }
        if (RETCODE_OK == retcode)
        {
            retcode = WifiStorage_FileClose(WifiFileHandle);
        }
    }
    if (FR_OK != fileSystemResult)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_ERROR_IN_GETTING_FILE_STATUS);
    }
    return retcode;
}
/** Refer interface header for description */
Retcode_T Storage_Setup(Storage_Setup_T * setup)
{
    Retcode_T retcode = RETCODE_OK;
    if (NULL == setup)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        if ((false == setup->WiFiFileSystem) && (false == setup->SDCard))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NOT_SUPPORTED);
        }
        else
        {
            StorageEnableStatus.SDCard = false;
            StorageEnableStatus.WiFiFileSystem = false;
            StorageSetup = *setup;
        }
    }
    return (retcode);
}

/** Refer interface header for description */
Retcode_T Storage_Enable(void)
{
    Retcode_T retcode = RETCODE_OK;
    if (StorageSetup.WiFiFileSystem)
    {
        retcode = WlanNetworkConnect_Init(WLAN_GetConnectionCallbackHandle());
        if (RETCODE_OK == retcode)
        {
            StorageEnableStatus.WiFiFileSystem = true;
        }
    }
    if (RETCODE_OK == retcode)
    {
        if (StorageSetup.SDCard)
        {
            if (false == isSdcardInitialized)
            {
                retcode = SDCardDriver_Initialize();
                if (RETCODE_OK == retcode)
                {
                    isSdcardInitialized = true;
                }
            }
            if (RETCODE_OK == retcode)
            {
                if (SDCARD_INSERTED != SDCardDriver_GetDetectStatus())
                {
                    retcode = RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_STORAGE_SDCARD_NOT_AVAILABLE);
                }
            }
            if (RETCODE_OK == retcode)
            {
                retcode = SDCardDriver_DiskInitialize(STORAGE_SDCARD_DRIVE_NUMBER); /* Initialize SD card */
            }
            if (RETCODE_OK == retcode)
            {
                /* Initialize file system */
                if (FR_OK != f_mount(&StorageSDCardFatFSObject, STORAGE_DEFAULT_LOGICAL_DRIVE, STORAGE_SDCARD_FORCE_MOUNT))
                {
                    retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_SDCARD_MOUNT_FAILED);
                }
            }
            if (RETCODE_OK == retcode)
            {
                StorageEnableStatus.SDCard = true;
            }
        }
    }
    return (retcode);
}
/** Refer interface header for description */
Retcode_T Storage_IsAvailable(Storage_Medium_T medium, bool * status)
{
    Retcode_T retcode = RETCODE_OK;
    if (NULL == status)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        *status = false;
        switch (medium)
        {
        case STORAGE_MEDIUM_SD_CARD:
            if (SDCARD_INSERTED != SDCardDriver_GetDetectStatus())
            {
                retcode = RETCODE(RETCODE_SEVERITY_WARNING, RETCODE_STORAGE_SDCARD_NOT_AVAILABLE);
            }
            else if (!StorageEnableStatus.SDCard)
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_SDCARD_UNINITIALIZED);
            }
            else
            {
                *status = true;
            }
            break;
        case STORAGE_MEDIUM_WIFI_FILE_SYSTEM:
            if (!StorageEnableStatus.WiFiFileSystem)
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_WIFI_UNINITIALIZED);
            }
            else
            {
                *status = true;
            }
            break;
        default:
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
            break;
        }
    }
    return (retcode);
}

/** Refer interface header for description */
Retcode_T Storage_Read(Storage_Medium_T medium, Storage_Read_T * readCredentials)
{
    Retcode_T retcode, fileOpenRetcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_FAILURE);

    int32_t wifiFileHandle = INT32_C(-1);
    uint32_t wifiFileLength = 0UL;
    uint32_t wifiBytesRead = 0UL;
    bool status = false;

    FRESULT sdCardReturn, fileOpenReturn = FR_OK;
    FILINFO sdCardFileInfo;
#if _USE_LFN
    sdCardFileInfo.lfname = NULL;
#endif
    FIL fileReadHandle;
    UINT bytesRead;
    if ((NULL == readCredentials) ||
            (NULL == readCredentials->FileName) ||
            (NULL == readCredentials->ReadBuffer) ||
            (0UL == readCredentials->BytesToRead))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        retcode = Storage_IsAvailable(medium, &status);

        if ((RETCODE_OK == retcode) && (true == status))
        {
            switch (medium)
            {
            case STORAGE_MEDIUM_WIFI_FILE_SYSTEM:
                memset(readCredentials->ReadBuffer, 0, readCredentials->BytesToRead);
                retcode = WifiStorage_GetFileStatus((const uint8_t*) readCredentials->FileName, &wifiFileLength);
                if ((RETCODE_OK == retcode) && (0UL != wifiFileLength) && (readCredentials->BytesToRead < wifiFileLength))
                {
                    retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
                }
                else
                {
                    wifiBytesRead = wifiFileLength;
                }
                if (RETCODE_OK == retcode)
                {
                    fileOpenRetcode = WifiStorage_FileOpen((const uint8_t*) readCredentials->FileName, &wifiFileHandle, WIFI_STORAGE_FOPEN_READ_MODE);
                }
                if ((RETCODE_OK == retcode) && (RETCODE_OK == fileOpenRetcode))
                {
                    retcode = WifiStorage_FileRead(wifiFileHandle, readCredentials->ReadBuffer, &wifiBytesRead);
                }
                if (RETCODE_OK == fileOpenRetcode)
                {
                    retcode = WifiStorage_FileClose(wifiFileHandle);
                }
                if ((RETCODE_OK == retcode) && (RETCODE_OK == fileOpenRetcode))
                {
                    readCredentials->ActualBytesRead = wifiBytesRead;
                }
                else
                {
                    readCredentials->ActualBytesRead = 0UL;
                    retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_ERROR_IN_FILE_READ);
                }
                break;
            case STORAGE_MEDIUM_SD_CARD:
                sdCardReturn = f_stat(readCredentials->FileName, &sdCardFileInfo);
                if (FR_OK == sdCardReturn)
                {
                    fileOpenReturn = f_open(&fileReadHandle, readCredentials->FileName, FA_OPEN_EXISTING | FA_READ);
                }
                if ((FR_OK == sdCardReturn) && (FR_OK == fileOpenReturn))
                {
                    sdCardReturn = f_lseek(&fileReadHandle, readCredentials->Offset);
                }
                if ((FR_OK == sdCardReturn) && (FR_OK == fileOpenReturn))
                {
                    sdCardReturn = f_read(&fileReadHandle, readCredentials->ReadBuffer, readCredentials->BytesToRead, &bytesRead); /* Read a chunk of source file */
                    readCredentials->ActualBytesRead = bytesRead;
                }
                if (FR_OK == fileOpenReturn)
                {
                    fileOpenReturn = f_close(&fileReadHandle);
                }
                if ((FR_OK != sdCardReturn) || (FR_OK != fileOpenReturn))
                {
                    retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_SDCARD_READ_FAILED);
                }
                break;
            default:
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
                break;
            }
        }
    }
    return (retcode);
}

/** Refer interface header for description */
Retcode_T Storage_Write(Storage_Medium_T medium, Storage_Write_T *writeCredentials)
{
    Retcode_T retcode, fileOpenRetcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_FAILURE);
    uint32_t length = 0;
    int32_t WifiFileHandle = INT32_C(-1);
    FRESULT sdCardReturn = FR_OK, fileOpenReturn = FR_OK;
    FIL fileWriteHandle;
    UINT bytesWritten;
    bool status = false;

    if ((NULL == writeCredentials) ||
            (NULL == writeCredentials->FileName) ||
            (NULL == writeCredentials->WriteBuffer) ||
            (0UL == writeCredentials->BytesToWrite))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        retcode = Storage_IsAvailable(medium, &status);
        if ((RETCODE_OK == retcode) && (true == status))
        {
            switch (medium)
            {
            case STORAGE_MEDIUM_WIFI_FILE_SYSTEM:
                retcode = WifiStorage_GetFileStatus((const uint8_t *) writeCredentials->FileName, &length);
                /* checking if file present is WiFi file system is empty or not */
                if (RETCODE_OK == retcode)
                {
                    fileOpenRetcode = WifiStorage_FileOpen((const uint8_t *) writeCredentials->FileName, &WifiFileHandle, WIFI_STORAGE_FOPEN_WRITE_MODE);
                }
                else if (Retcode_GetCode(retcode) == (Retcode_T) RETCODE_STORAGE_FILE_DOES_NOT_EXIST_WIFI_MEDIA)
                {
                    fileOpenRetcode = WifiStorage_FileOpen((const uint8_t *) writeCredentials->FileName, &WifiFileHandle, WIFI_STORAGE_FOPEN_CREATE_MODE);
                    retcode = RETCODE_OK;
                }
                if ((RETCODE_OK == retcode) && (RETCODE_OK == fileOpenRetcode))
                {
                    length = writeCredentials->BytesToWrite;
                    retcode = WifiStorage_FileWrite(WifiFileHandle, writeCredentials->WriteBuffer, length);
                }
                if (RETCODE_OK == fileOpenRetcode)
                {
                    fileOpenRetcode = WifiStorage_FileClose(WifiFileHandle);
                }
                if (RETCODE_OK != fileOpenRetcode)
                {
                    retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_ERROR_IN_FILE_WRITE);
                }

                break;
            case STORAGE_MEDIUM_SD_CARD:
                fileOpenReturn = f_open(&fileWriteHandle, writeCredentials->FileName, FA_WRITE | FA_CREATE_ALWAYS);
                if (FR_OK == fileOpenReturn)
                {
                    sdCardReturn = f_lseek(&fileWriteHandle, writeCredentials->Offset);
                }
                if ((FR_OK == sdCardReturn) && (FR_OK == fileOpenReturn))
                {
                    sdCardReturn = f_write(&fileWriteHandle, writeCredentials->WriteBuffer, writeCredentials->BytesToWrite, &bytesWritten); /* Write it to the destination file */
                    writeCredentials->ActualBytesWritten = bytesWritten;
                }
                if (FR_OK == fileOpenReturn)
                {
                    fileOpenReturn = f_close(&fileWriteHandle);
                }
                if ((FR_OK != sdCardReturn) || (FR_OK != fileOpenReturn))
                {
                    retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_ERROR_IN_FILE_WRITE);
                }
                break;
                default:
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
                break;
            }
        }
    }
    return (retcode);
}

/** Refer interface header for description */
Retcode_T Storage_Rename(Storage_Medium_T medium, Storage_Rename_T *renameCredentials)
{
    Retcode_T retcode = RETCODE_OK;
    FRESULT sdCardReturn = FR_INVALID_PARAMETER;
    FILINFO fileinfo;
#if _USE_LFN
    fileinfo.lfname = NULL;
#endif
    if ((NULL == renameCredentials) ||
            (NULL == renameCredentials->OriginalFileName) ||
            (NULL == renameCredentials->NewFileName))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        if (STORAGE_MEDIUM_SD_CARD == medium)
        {
            /* Check if the new file already exists */
            sdCardReturn = f_stat((const TCHAR*) renameCredentials->NewFileName, &fileinfo);
            if (FR_OK == sdCardReturn)
            {
                /* Delete the previously existing file */
                sdCardReturn = f_unlink((const TCHAR*) renameCredentials->NewFileName);
            }
            else
            {
                /* New file did not exist, rename the file */
                sdCardReturn = FR_OK;
            }
            if (FR_OK == sdCardReturn)
            {
                sdCardReturn = f_rename((const TCHAR*) renameCredentials->OriginalFileName, (const TCHAR*) renameCredentials->NewFileName);
            }
            if (FR_OK != sdCardReturn)
            {
                retcode = RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_STORAGE_SDCARD_RENAME_FAILED);
            }
        }
        else if (STORAGE_MEDIUM_WIFI_FILE_SYSTEM == medium)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NOT_SUPPORTED);
        }
        else
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
        }
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T Storage_Transfer(Storage_Medium_T sourceMedium, Storage_Medium_T destinationMedium, Storage_Transfer_T *transferCredentials)
{
    Retcode_T retcode = RETCODE_OK;
    bool status = false;

    if ((NULL == transferCredentials) ||
            (NULL == transferCredentials->DestinationFileName) ||
            (NULL == transferCredentials->SourceFileName) ||
            (NULL == transferCredentials->TransferBuffer))
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }

    Storage_Read_T readCredentials =
            {
                    .FileName = transferCredentials->SourceFileName,
                    .ReadBuffer = transferCredentials->TransferBuffer,
                    .BytesToRead = transferCredentials->TransferBlockSizeSize,
                    .ActualBytesRead = 0UL,
                    .Offset = 0UL,
            };
    Storage_Write_T writeCredentials =
            {
                    .FileName = transferCredentials->DestinationFileName,
                    .WriteBuffer = transferCredentials->TransferBuffer,
                    .BytesToWrite = transferCredentials->TransferBlockSizeSize,
                    .ActualBytesWritten = 0UL,
                    .Offset = 0UL,
            };

    retcode = Storage_IsAvailable(sourceMedium, &status);
    if ((RETCODE_OK == retcode) && (true == status))
    {
        switch (sourceMedium)
        {
        case STORAGE_MEDIUM_SD_CARD:
            if (STORAGE_MEDIUM_WIFI_FILE_SYSTEM == destinationMedium)
            {
                retcode = SdcardToWifiTransfer(&readCredentials, &writeCredentials);
            }
            else if (STORAGE_MEDIUM_SD_CARD == destinationMedium)
            {
                retcode = SdcardToSdcardTransfer(&readCredentials, &writeCredentials);
            }
            break;
        case STORAGE_MEDIUM_WIFI_FILE_SYSTEM:
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NOT_SUPPORTED);
            break;
        default:
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
            break;
        }
    }
    return (retcode);
}

/** Refer interface header for description */
Retcode_T Storage_Delete(Storage_Medium_T medium, const char* fileName)
{
    if (NULL == fileName)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    Retcode_T retcode = RETCODE_OK;
    FRESULT sdCardReturn;
    FILINFO fileInfo;
    bool status = false;

#if _USE_LFN
    fileInfo.lfname = NULL;
#endif

    retcode = Storage_IsAvailable(medium, &status);
    if ((RETCODE_OK == retcode) && (true == status))
    {
        switch (medium)
        {
        case STORAGE_MEDIUM_SD_CARD:
            /* Check if the file exists */
            sdCardReturn = f_stat((const TCHAR*) fileName, &fileInfo);
            if (FR_OK == sdCardReturn)
            {
                /* Delete the previously existing file */
                sdCardReturn = f_unlink((const TCHAR*) fileName);
                if (FR_OK != sdCardReturn)
                {
                    retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_ERROR_IN_FILE_DELETE);
                }
            }
            else
            {
                /* New file did not exist, rename the file */
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_FILE_DOES_NOT_EXIST_SDCARD);
            }
            break;
        case STORAGE_MEDIUM_WIFI_FILE_SYSTEM:
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NOT_SUPPORTED);
            break;
        default:
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
            break;
        }
    }
    return (retcode);
}

/** Refer interface header for description */
Retcode_T Storage_Disable(Storage_Medium_T medium)
{
    Retcode_T retcode = RETCODE_OK;

    switch (medium)
    {
    case STORAGE_MEDIUM_SD_CARD:
        if ((StorageSetup.SDCard) && (StorageEnableStatus.SDCard) && (isSdcardInitialized))
        {
            retcode = SDCardDriver_Deinitialize();
            if (RETCODE_OK == retcode)
            {
                isSdcardInitialized = false;
                StorageEnableStatus.SDCard = false;
            }
        }
        else
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_NOT_ENABLED);
        }
        break;
    case STORAGE_MEDIUM_WIFI_FILE_SYSTEM:
        if ((StorageSetup.WiFiFileSystem) && (StorageEnableStatus.WiFiFileSystem))
        {
            retcode = WlanNetworkConnect_DeInit();
            if (RETCODE_OK == retcode)
            {
                StorageEnableStatus.WiFiFileSystem = false;
            }
        }
        else
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_NOT_ENABLED);
        }
        break;
    default:
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
        break;
    }
    return (retcode);
}

/** Refer interface header for description */
Retcode_T Storage_Close(void)
{
    Retcode_T retcode = RETCODE_OK;
    if ((true == StorageSetup.WiFiFileSystem) || (true == StorageSetup.SDCard))
    {
        StorageSetup.SDCard = false;
        StorageSetup.WiFiFileSystem = false;
    }
    else
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_UNINITIALIZED);

    }
    return (retcode);
}
#endif /* XDK_UTILITY_STORAGE */

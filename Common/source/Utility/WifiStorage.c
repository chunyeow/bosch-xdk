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

#include "XdkCommonInfo.h"
#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_WIFISTORAGE

#include "WifiStorage.h"
#include "stdio.h"
#include "fs.h"

/* Put the type and macro definitions here */

/* Put constant and variable definitions here */
#define WRITE_READ_ALIGN_VAL       1024
#define INVALID_FILE_HANDLE        INT32_C(-1)

/* Put private function declarations here */

/* Put function implementations here */
/* Refer interface details in <WifiStorage.h> file */
Retcode_T WifiStorage_FileOpen(const uint8_t* fileName, int32_t *fileHandle, WifiFileMode_T fileMode)
{
    Retcode_T retVal = RETCODE_OK;
    _i32 libRetVal = INT32_C(-1);
    _u32 accessModeVal = 0;
    if ((NULL == fileName) || (NULL == fileHandle))
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    if (WIFI_STORAGE_FOPEN_MAX <= fileMode)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM));
    }

    if (WIFI_STORAGE_FOPEN_CREATE_MODE == fileMode)
    {
        accessModeVal = FS_MODE_OPEN_CREATE(WIFI_STORAGE_FILE_SIZE, ( _u32 ) (_FS_FILE_OPEN_FLAG_COMMIT | _FS_FILE_PUBLIC_WRITE));
    }
    else if (WIFI_STORAGE_FOPEN_WRITE_MODE == fileMode)
    {
        accessModeVal = (_u32) FS_MODE_OPEN_WRITE;
    }
    else if (WIFI_STORAGE_FOPEN_READ_MODE == fileMode)
    {
        accessModeVal = (_u32) FS_MODE_OPEN_READ;
    }
    libRetVal = sl_FsOpen(fileName, accessModeVal, NULL, (_i32 *)fileHandle);
    if ( SL_FS_FILE_HAS_NOT_BEEN_CLOSE_CORRECTLY == libRetVal)
    {
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_FILE_HAS_NOT_BEEN_CLOSE_CORRECTLY);
    }
    else if ( SL_FS_ERR_BAD_FILE_MODE == libRetVal)
    {
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_WRONG_FILE_MODE);
    }
    else if (0x00 != libRetVal)
    {
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_FAILURE);
    }
    else
    {
        /* Do not return retVal */;
    }
    return retVal;
}

/* Refer interface details in <WifiStorage.h> file */
Retcode_T WifiStorage_FileWrite(const int32_t fileHandle, uint8_t *writeBuffer, uint32_t noOfBytesToWrite)
{
    Retcode_T retVal = RETCODE_OK;
    _i32 libRetVal = INT32_C(-1);
    uint32_t noOfChunks = 0;
    uint32_t remainingBytes = 0;
    uint32_t writeOffset = 0;
    if ((NULL == writeBuffer) || ( INT32_C(0) == fileHandle))
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    if (UINT32_C(0) == noOfBytesToWrite)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM));
    }
    remainingBytes = (noOfBytesToWrite % WRITE_READ_ALIGN_VAL);
    noOfChunks = (noOfBytesToWrite / WRITE_READ_ALIGN_VAL);
    for (uint32_t i = 0; i < noOfChunks; i++)
    {
        libRetVal = sl_FsWrite(fileHandle, writeOffset, (unsigned char *) &writeBuffer[writeOffset], WRITE_READ_ALIGN_VAL);
        if (WRITE_READ_ALIGN_VAL != libRetVal)
        {
            retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_ERROR_IN_FILE_WRITE);
            break;
        }
        else
        {
            writeOffset += libRetVal;
        }
    }
    if (RETCODE_OK == retVal)
    {
        libRetVal = sl_FsWrite(fileHandle, writeOffset, (unsigned char *) &writeBuffer[writeOffset], remainingBytes);
        if ((int32_t) remainingBytes != libRetVal)
        {
            retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_ERROR_IN_FILE_WRITE);
        }
    }
    return retVal;
}

/* Refer interface details in <WifiStorage.h> file */
Retcode_T WifiStorage_FileRead(const int32_t fileHandle, uint8_t *readBuffer, uint32_t *noOfBytesRead)
{
    Retcode_T retVal = RETCODE_OK;
    _i32 libRetVal = INT32_C(-1);
    uint32_t noOfChunks = 0;
    uint32_t readOffset = 0;
    uint32_t remainingBytes = 0;
    if ((NULL == readBuffer) || (NULL == noOfBytesRead) || (INT32_C(0) == fileHandle))
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    if (INT32_C(0) == *noOfBytesRead)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM));
    }
    remainingBytes = (*noOfBytesRead % WRITE_READ_ALIGN_VAL);
    noOfChunks = (*noOfBytesRead / WRITE_READ_ALIGN_VAL);
    *noOfBytesRead = 0;
    for (uint32_t i = 0; i < noOfChunks; i++)
    {
        libRetVal = sl_FsRead(fileHandle, readOffset, (unsigned char *) &readBuffer[readOffset], WRITE_READ_ALIGN_VAL);
        if (WRITE_READ_ALIGN_VAL == libRetVal)
        {
            readOffset += libRetVal;
            *noOfBytesRead += libRetVal;
        }
        else
        {
            retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_ERROR_IN_FILE_READ);
            break;
        }
    }
    if (RETCODE_OK == retVal)
    {
        libRetVal = sl_FsRead(fileHandle, readOffset, (unsigned char *) &readBuffer[readOffset], remainingBytes);
        if ((int32_t) remainingBytes == libRetVal)
        {
            *noOfBytesRead += libRetVal;
        }
        else
        {
            retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_ERROR_IN_FILE_READ);
        }
    }
    return retVal;
}

/* Refer interface details in <WifiStorage.h> file */
Retcode_T WifiStorage_GetFileStatus(const uint8_t* fileName, uint32_t *fileLength)
{
    Retcode_T retVal = RETCODE_OK;
    SlFsFileInfo_t fileInfo;
    _i16 libRetVal = INT16_C(-1);
    if ((NULL == fileName) || (NULL == fileLength))
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }
    libRetVal = sl_FsGetInfo((const _u8 *) fileName, 0, &fileInfo);
    if (INT16_C(0) == libRetVal)
    {
        *fileLength = fileInfo.FileLen;
    }
    else if (SL_FS_ERR_FILE_NOT_EXISTS == libRetVal)
    {
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_FILE_DOES_NOT_EXIST_WIFI_MEDIA);
    }
    else if ( SL_FS_FILE_HAS_NOT_BEEN_CLOSE_CORRECTLY == libRetVal)
    {
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_FILE_HAS_NOT_BEEN_CLOSE_CORRECTLY);
    }
    else
    {
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_ERROR_IN_GETTING_FILE_STATUS);
    }
    return retVal;
}

Retcode_T WifiStorage_FileClose(const int32_t fileHandle)
{
    Retcode_T retVal = RETCODE_OK;
    _i16 libRetVal = -1;
    libRetVal = sl_FsClose((const _i32) fileHandle, 0, 0, 0);
    if (INT16_C(0) != libRetVal)
    {
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_ERROR_IN_FILE_CLOSE_WIFI_MEDIA);
    }
    return retVal;
}

/* Refer interface details in <WifiStorage.h> file */
Retcode_T WifiStorage_FileDelete(const uint8_t* fileName, int32_t *fileHandle)
{
    Retcode_T retVal = RETCODE_OK;
    _i16 libRetVal = -1;
    if (NULL == fileName)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }

    libRetVal = sl_FsDel((const _u8 *) fileName, 0);
    if (INT16_C(0) == libRetVal)
    {
        *fileHandle = INVALID_FILE_HANDLE;
    }
    else
    {
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_STORAGE_ERROR_IN_FILE_DELETE);
    }
    return retVal;
}

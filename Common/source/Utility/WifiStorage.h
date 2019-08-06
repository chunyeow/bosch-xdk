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
 * @brief This contains the application interface for storage and retrieval of file inside the device storage medium .
 *
 * @details
 */
#ifndef _WIFISTORAGE_H_
#define _WIFISTORAGE_H_

/* Include all headers which are needed by this file. */
#include "BCDS_Retcode.h"
#include "XdkCommonInfo.h"
/* Put the type and macro definitions here */

enum FileMode_E
{
    WIFI_STORAGE_FOPEN_WRITE_MODE = 0,
    WIFI_STORAGE_FOPEN_READ_MODE,
    WIFI_STORAGE_FOPEN_CREATE_MODE,
    WIFI_STORAGE_FOPEN_MAX,
};

typedef enum FileMode_E WifiFileMode_T;

/* @todo need to analysiss and fix the size for wifi storage*/
#define WIFI_STORAGE_FILE_SIZE  UINT32_C(4096) //4 * 1024; //4 K


/* Put the function declarations here */
/**
 * @brief  Checks whether the requested file exist in the wifi media
 *
 * @details A prior call to WlanConnect_Init() is necessary for this function in order to operate, failing which the intended functionality
 *          is not guaranteed.
 *
 * @param [in] fileName       : Filename for which the status needs to be checked
 * @param [in/out] fileLength : Gives the size of the content in the file in bytes
 *
 * @retVal RETCODE_OK        If in case of the requested file already exist in the wifi media
 * @retVal RETCODE_FILE_DOES_NOT_EXIST_WIFI_MEDIA If in case of file requested is not present in the wifi media
 * @retVal RETCODE_ERROR_FILE_HAS_NOT_BEEN_CLOSE_CORRECTLY If in case this API is called before proper close of
 *         of file operation using API WifiStorage_FileClose() in the previous call in application
 */
Retcode_T WifiStorage_GetFileStatus(const uint8_t* fileName , uint32_t *fileLength);

/**
 * @brief  This Interface API is used to open a file in the wifi media in requested file mode
 *
 * @details A prior call to WlanConnect_Init() is necessary for this function in order to operate, failing which the intended functionality
 *          is not guaranteed.
 *
 * @param [in]     fileName        Filename which needs to be either created ,opened for read operation or opened for write operation
 * @param [in/out] fileHandle      Handle obtained as reference which needs to be used for performing file operation like WifiStorage_FileRead(), WifiStorage_FileWrite(), WifiStorage_FileClose()
 * @param [in]     fileMode        Specifies the file is opened for which mode for the modes supported please refer enum WifiFileMode_T
 *
 * @retVal RETCODE_OK        If in case of the requested file open operation is success based on the user selected filemode
 * @retVal RETCODE_WRONG_FILE_MODE If in case of wrong file mode is set while opening the file
 * @retVal RETCODE_ERROR_FILE_HAS_NOT_BEEN_CLOSE_CORRECTLY If in case this API is called before proper close of
 *         of file operation using API WifiStorage_FileClose() in the previous call in application
 *
 * @Note fileHandle obtained using this WifiStorage_FileOpen() needs to be protected in application while performing file operation in multi-threading environment.
 */
Retcode_T WifiStorage_FileOpen(const uint8_t* fileName , int32_t *fileHandle , WifiFileMode_T fileMode);

/**
 * @brief  This Interface API is used to close a file in the wifi media
 *
 * @details A prior call to WlanConnect_Init() is necessary for this function in order to operate, failing which the intended functionality
 *          is not guaranteed.
 *
 * @param [in] fileHandle      Handle obtained in WifiStorage_FileOpen() API call
 *
 * @retVal RETCODE_OK        If in case of the requested file close operation is success
 * @retVal RETCODE_ERROR_IN_FILE_CLOSE_WIFI_MEDIA If in case of failure in closing the requested file
 *
 * @Note WifiStorage_FileClose() must be called in the application in same execution context, even if in case WifiStorage_FileRead() or WifiStorage_FileWrite() operation fails for some reason
 * @Note WifiStorage_FileClose() must be called in application in same execution context where WifiStorage_FileOpen(), WifiStorage_FileRead() or WifiStorage_FileWrite() operations are performed
 * @Note If no other file operation is performed after calling WifiStorage_FileOpen() , close the file WifiStorage_FileClose() in the same context.
 *
 */
Retcode_T WifiStorage_FileClose(const int32_t fileHandle);

/**
 * @brief  This Interface API is used to read a file content from the wifi media
 *
 * @details A prior call to WlanConnect_Init() is necessary for this function in order to operate, failing which the intended functionality
 *          is not guaranteed.
 *
 * @param [in] fileHandle       Handle obtained in WifiStorage_FileOpen() API call in read mode
 * @param [in] readBuffer       pointer to buffer to collect the read data from the file from media
 * @param [in/out]noOfBytesRead The value pointed is used for mentioning the no of bytes to read and while returning,
 *                              the actual no of bytes read is updated
 *
 * @retVal RETCODE_OK                 If in case of the requested file read operation is success
 * @retVal RETCODE_ERROR_IN_FILE_READ If in case of failure in reading the requested file
 *
 * @Note Please note that the WifiStorage_FileClose() must be called in application in same execution context where WifiStorage_FileRead() operation returns either success or failure case
 *
 */
Retcode_T WifiStorage_FileRead(const int32_t fileHandle, uint8_t *readBuffer, uint32_t *noOfBytesRead);

/**
 * @brief  This Interface API is used to write user buffer file content to the wifi media
 *
 * @details A prior call to WlanConnect_Init() is necessary for this function in order to operate, failing which the intended functionality
 *          is not guaranteed.
 *
 * @param [in] fileHandle        Handle obtained in WifiStorage_FileOpen() API call in write mode
 * @param [in] writeBuffer       Pointer to user write buffer which needs to be written into the file in media
 * @param [in] noOfBytesToWrite  No of bytes to write
 *
 * @retVal RETCODE_OK                 If in case of the requested file write operation is success
 * @retVal RETCODE_ERROR_IN_FILE_WRITE If in case of failure in writing to the requested file
 *
 * @Note Please note that the WifiStorage_FileClose() must be called in application in same execution context where WifiStorage_FileWrite() operation returns either success or failure case
 */
Retcode_T WifiStorage_FileWrite(const int32_t fileHandle,uint8_t *writeBuffer, uint32_t noOfBytesToWrite);

/**
 * @brief  This Interface API is used to delete a file in the wifi media in requested file mode
 *
 * @details A prior call to WlanConnect_Init() is necessary for this function in order to operate, failing which the intended functionality
 *          is not guaranteed.
 *
 * @param [in]     fileName        Filename of the file in the wifi media which needs to be deleted
 * @param [in/out] fileHandle      Handle obtained in WifiStorage_FileOpen() API call
 *
 * @retVal RETCODE_OK        If in case of the requested file delete operation is success
 * @retVal RETCODE_ERROR_IN_FILE_DELETE  In case the delete operation is not successful
 *
 * @Note If file deleted successfully, then the  fileHandle will be made invalid on returning from this API call
 *
 */
Retcode_T WifiStorage_FileDelete(const uint8_t* fileName,int32_t *fileHandle);
#endif /* _WIFISTORAGE_H_ */


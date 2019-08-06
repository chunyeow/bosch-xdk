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
 * @defgroup STORAGE Storage
 * @{
 *
 * @brief This module handles the Storage features. It supports :
 *                                                              - SD Card
 *                                                              - WiFi file system
 *
 * @file
 */

/* header definition ******************************************************** */
#ifndef XDK_STORAGE_H_
#define XDK_STORAGE_H_

/* local interface declaration ********************************************** */
#include "BCDS_Retcode.h"

/**
 * @brief   Structure to represent the Storage setup features.
 */
struct Storage_Setup_S
{
    bool SDCard; /**< Boolean representing if SD card is to be enabled or not */
    bool WiFiFileSystem; /**< Boolean representing if WiFi file system is to be enabled or not */
};

/**
 * @brief   Typedef to represent the Storage setup feature.
 */
typedef struct Storage_Setup_S Storage_Setup_T;

/**
 * @brief   Typedef to represent the Storage status feature.
 */
typedef struct Storage_Setup_S Storage_Status_T;

/**
 * @brief   Enum to represent the Storage mediums.
 */
enum Storage_Medium_S
{
    STORAGE_MEDIUM_SD_CARD,
    STORAGE_MEDIUM_WIFI_FILE_SYSTEM,
};

/**
 * @brief   Typedef to represent the Storage medium.
 */
typedef enum Storage_Medium_S Storage_Medium_T;

/**
 * @brief   Structure to represent the Storage read features.
 */
struct Storage_Read_S
{
    const char * FileName; /**< Name of the file to be read */
    uint8_t * ReadBuffer; /**< Pointer to user buffer to store the read file content */
    uint32_t BytesToRead; /**< Number of bytes to read */
    uint32_t ActualBytesRead; /**< Size of the user buffer actually utilized to store the read file content */
    uint32_t Offset; /**<Byte offset from top of the file to set write pointer*/
};

/**
 * @brief   Typedef to represent the Storage read feature.
 */
typedef struct Storage_Read_S Storage_Read_T;

/**
 * @brief   Structure to represent the Storage write features.
 */
struct Storage_Write_S
{
    const char * FileName; /**< Name of the file to be write */
    uint8_t * WriteBuffer; /**< Pointer to user buffer to write the file content from */
    uint32_t BytesToWrite; /**< Number of bytes to write */
    uint32_t ActualBytesWritten; /**< Number of bytes to written */
    uint32_t Offset; /**<Byte offset from top of the file to set write pointer*/
};

/**
 * @brief   Typedef to represent the Storage write feature.
 */
typedef struct Storage_Write_S Storage_Write_T;

/**
 * @brief   Structure to represent the Storage rename features.
 */
struct Storage_Rename_S
{
    const char * OriginalFileName; /**< Original name of the file to be renamed */
    const char * NewFileName; /**< New name of the file upon rename */
};

/**
 * @brief   Typedef to represent the Storage rename feature.
 */
typedef struct Storage_Rename_S Storage_Rename_T;

/**
 * @brief   Structure to represent the Storage transfer features.
 */
struct Storage_Transfer_S
{
    const char * SourceFileName; /**< Source file name to be transfered */
    const char * DestinationFileName; /**< Destination file name upon transfer */
    uint8_t * TransferBuffer; /**< Pointer to user buffer to transfer the file content blocks */
    uint32_t TransferBlockSizeSize; /**< Block size for transfer (size of user provided TransferBuffer must be greater than or equal to this value )*/
};

/**
 * @brief   Typedef to represent the Storage transfer feature.
 */
typedef struct Storage_Transfer_S Storage_Transfer_T;

/**
 * @brief This will setup the Storage
 *
 * @param[in] setup
 * Pointer to the Storage setup feature
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - This must be the first API to be called if this storage feature is to be used.
 * - Do not call this API more than once.
 */
Retcode_T Storage_Setup(Storage_Setup_T * setup);

/**
 * @brief This will enable the Storage
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #Storage_Setup must have been successful prior.
 */
Retcode_T Storage_Enable(void);

/**
 * @brief This will validate if the Storage is available or not
 *
 * @param[in] medium
 * Storage medium to be validated for availability
 *
 * @param[out] status
 * Pointer to the availability of the requested storage medium status
 *
 * @note
 * - In case of failure the status will be false
 * - #Storage_Enable must have been successful prior.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
Retcode_T Storage_IsAvailable(Storage_Medium_T medium, bool * status);

/**
 * @brief This will read the data from the storage.
 *
 * @param[in] medium
 * Storage medium in while the file is to be renamed.
 *
 * @param[in] readCredentials
 * Pointer to the Storage read structure.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #Storage_Enable must have been successful prior.
 */
Retcode_T Storage_Read(Storage_Medium_T medium, Storage_Read_T * readCredentials);

/**
 * @brief This will write the data in the storage.
 *
 * @param[in] medium
 * Storage medium in while the file is to be renamed.
 *
 * @param[in] writeCredentials
 * Pointer to the storage write credentials.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #Storage_Enable must have been successful prior.
 */
Retcode_T Storage_Write(Storage_Medium_T medium, Storage_Write_T *writeCredentials);

/**
 * @brief This will rename the file in the storage.
 *
 * @param[in] medium
 * Storage medium in while the file is to be renamed.
 *
 * @param[in] renameCredentials
 * Pointer to the storage rename credentials.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #Storage_Enable must have been successful prior.
 */
Retcode_T Storage_Rename(Storage_Medium_T medium, Storage_Rename_T *renameCredentials);

/**
 * @brief This will delete the given file in the storage.
 *
 * @param[in] medium
 * Storage medium in which the file is to be deleted.
 *
 * @param[in] fileName
 * Name of the file which has to be deleted.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #Storage_Enable must have been successful prior.
 */
Retcode_T Storage_Delete(Storage_Medium_T medium, const char* fileName);

/**
 * @brief This will transfer the file from source to destination storage medium.
 *
 * @param[in] sourceMedium
 * Source Storage medium from which the file is to be transfered.
 *
 * @param[in] destinationMedium
 * Destination Storage medium to which the file is to be transfered.
 *
 * @param[in] transferCredentials
 * Pointer to the transfer credentials.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #Storage_Enable must have been successful prior.
 */
Retcode_T Storage_Transfer(Storage_Medium_T sourceMedium, Storage_Medium_T destinationMedium, Storage_Transfer_T *transferCredentials);

/**
 * @brief This will disable the Storage medium
 *
 * @param[in] medium
 *  Storage medium that has to be disabled.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #Storage_Enable must have been successful prior.
 */
Retcode_T Storage_Disable(Storage_Medium_T medium);

/**
 * @brief This will close the Storage
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #Storage_Setup must have been successful prior.
 * - If #Storage_Enable is done prior then it should be disabled by calling #Storage_Disable before calling #Storage_Close.
 * - After calling this API storage module can't be used.
 */
Retcode_T Storage_Close(void);

#endif /* XDK_STORAGE_H_ */
/**@}*/

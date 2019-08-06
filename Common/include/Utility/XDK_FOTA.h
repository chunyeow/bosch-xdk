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
 * @ingroup BCDS_FOTA
 *
 * @defgroup FOTA FOTA through SD Card
 * @{
 *
 * @brief This module will handle, FOTA related activities like Firmware validation and Firmware update from the SD Card.
 *
 * @file
 */
/* module includes ********************************************************** */

#ifndef XDK_FOTA_H_
#define XDK_FOTA_H_

#include "BCDS_FWContainer.h"

/**
 * @brief Enumeration to represent the Fota container partition.
 */
enum Fota_Partition_E
{
    FOTA_PARTITION_PRIMARY,     /**< Active running Fota container firmware in the controller FLASH memory */
    FOTA_PARTITION_DOWNLOADED,  /**< Fota container firmware in SDcard memory */
};

typedef enum Fota_Partition_E Fota_Partition_T;

/**
 * @brief   Struct to hold the partition information from which fota container API's performed.
 */
struct Fota_PartitionInfo_S
{
    Fota_Partition_T Partition;       /**< Partition information */
    const char * PartitionFileName;   /**< File name in-case of #FOTA_PARTITION_DOWNLOADED partition */
};
typedef struct Fota_PartitionInfo_S Fota_PartitionInfo_T;


/* public type and macro definitions */
/**
 * @brief   This function reads the Fota container header from the specified partition.
 *
 * @param [in]  partitionInfo
 *              mention the partition information from which header info needs to be read. In-case of FOTA_PARTITION_DOWNLOAD specify the file name
 *              Incase of FOTA_PARTITION_PRIMARY file name is not used can be NULL.
 *
 * @param [out] header
 *              Buffer pointer to hold the read Fota container header information.
 *
 * @note
 * -  Storage_Setup & Storage_Enable must be called prior to using this API.
 *
 * @return #RETCODE_OK if fota container header is read properly, or an error code otherwise.
 *
 */
Retcode_T FOTA_ReadHeader(Fota_PartitionInfo_T partitionInfo, FWContainer_Header_T* header);

/**
 * @brief   This function reads the Fota container firmware version on the fota header from the specified partition.
 *
 * @param [in]  partitionInfo
 *              mention the partition information from which Firmware version needs to be read. In-case of FOTA_PARTITION_DOWNLOAD specify the file name
 *              Incase of FOTA_PARTITION_PRIMARY file name is not used can be NULL.
 *
 * @param [out] firmwareVer
 *              pointer to hold the Fota container firmware version information.
 *
 * @note
 * -  Storage_Setup & Storage_Enable must be called prior to using this API.
 *
 * @return #RETCODE_OK if fota container firmware version is read properly, or an error code otherwise.
 *
 */
Retcode_T FOTA_ReadFirmwareVersion(Fota_PartitionInfo_T partitionInfo, uint32_t * firmwareVer);

/**
 * @brief   This function reads the Fota container firmware CRC on the fota header from the specified partition.
 *
 * @param [in]  partitionInfo
 *              mention the partition information from which firmware CRC info needs to be read. In-case of FOTA_PARTITION_DOWNLOAD specify the file name
 *              Incase of FOTA_PARTITION_PRIMARY file name is not used, can be NULL.
 *
 * @param [out] firmwareCRC
 *              pointer to hold the Fota container firmware CRC information.
 *
 * @note
 * -  Storage_Setup & Storage_Enable must be called prior to using this API.
 *
 * @return #RETCODE_OK if fota container firmware CRC is read properly, or an error code otherwise.
 *
 */
Retcode_T FOTA_ReadFirmwareCRC(Fota_PartitionInfo_T partitionInfo, uint32_t * firmwareCRC);
/**
 * @brief   This function Validates the firmware in the Sdcard
 *
 * @param [in]  firmwareName
 *              Name of the firmware which needs to be validated.
 *
 * @note
 * - Storage_Setup & Storage_Enable must be called prior to using this API.
 * - If Firmware is valid,this API will rename the firmware to "firmware.bin"
 * - In-case if "firmware.bin" is already available then it will be overwritten with the new firmware "firmwareName" .
 *
 * @return #RETCODE_OK if firmware is valid, or an error code otherwise.
 *
 */
Retcode_T FOTA_ValidateSdcardFw(const char* firmwareName);

/**
 * @brief   This function Triggers an firmware update from Sdcard.
 *
 * @note
 * - Firmware has to be validated by calling FOTA_ValidateSdcardFw() before updating.
 * - latest validated firmware will be updated.
 *
 * @return If firmware is valid.Device will be rebooted so no return value, or an error code otherwise.
 *
 */
Retcode_T FOTA_UpdateSdcardFw(void);

#endif /* XDK_FOTA_H_ */

/**@} */

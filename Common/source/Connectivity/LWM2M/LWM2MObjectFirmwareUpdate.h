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
 * @brief Interface header for LWM2MObjectFirmwareUpdate file.
 *
 * @file
 */
/* module includes ********************************************************** */

#ifndef LWM2MOBJECTFIRMWAREUPDATE_H_
#define LWM2MOBJECTFIRMWAREUPDATE_H_

#include "Serval_Lwm2m.h"
#include "BCDS_CmdProcessor.h"

/* public type and macro definitions */

/**
 * @brief Enum to represent the LWM2M standard firmware update result values.
 */
enum LWM2MObjectFirmwareUpdate_UpdateResult_E
{
    LWM2M_OBJ_FW_UPDATE_DEFAULT = (UINT8_C(0)),
    LWM2M_OBJ_FW_UPDATE_SUCCESS = (UINT8_C(1)),
    LWM2M_OBJ_FW_UPDATE_OUT_OF_STORAGE = (UINT8_C(2)),
    LWM2M_OBJ_FW_UPDATE_OUT_OF_MEMORY = (UINT8_C(3)),
    LWM2M_OBJ_FW_UPDATE_CONNECTION_LOST = (UINT8_C(4)),
    LWM2M_OBJ_FW_UPDATE_CRC_FAILED = (UINT8_C(5)),
    LWM2M_OBJ_FW_UPDATE_UNSUPPORTED_TYPE = (UINT8_C(6)),
    LWM2M_OBJ_FW_UPDATE_INVALID_URI = (UINT8_C(7)),
    LWM2M_OBJ_FW_UPDATE_FAILED = (UINT8_C(8))
};

/**
 * @brief Enum to represent the FOTA states.
 */
enum LWM2MObjectFirmwareUpdate_State_E
{
    LWM2M_OBJ_FW_UPDATE_STATE_IDLE = 0x00,
    LWM2M_OBJ_FW_UPDATE_STATE_DOWNLOADING = 0x01,
    LWM2M_OBJ_FW_UPDATE_STATE_DOWNLOADED = 0x02,
    LWM2M_OBJ_FW_UPDATE_STATE_UPDATING = 0x03
};

/**
 * @brief Typedef to represent the FOTA state.
 */
typedef enum LWM2MObjectFirmwareUpdate_State_E LWM2MObjectFirmwareUpdate_State_T, *LWM2MObjectFirmwareUpdate_StatePtr_T;

/**
 * @brief Structure to represent the LWM2M standard firmware update object resources.
 */
struct LWM2MObjectFirmwareUpdate_Resource_S
{
    Lwm2mResource_T firmwarePackage;
    Lwm2mResource_T uriFromWhereToDownload;
    Lwm2mResource_T update;
    Lwm2mResource_T state;
    Lwm2mResource_T updateSupportedObjects;
    Lwm2mResource_T updateResult;
};

/**
 * @brief Typedef to represent the LWM2M standard firmware update object resource.
 */
typedef struct LWM2MObjectFirmwareUpdate_Resource_S LWM2MObjectFirmwareUpdate_Resource_T;

/**
 * @brief Instance of the LWM2M standard firmware update object.
 */
extern LWM2MObjectFirmwareUpdate_Resource_T LWM2MObjectFirmwareUpdateResources;

/**
 * @brief   This function initializes the FOTA state machine module.
 * It is event driven internally.
 *
 * @note
 * - It is a pre-requisite that this is the first API to be called in case of  making use of the FOTA state machine.
 * - All the necessary FOTA related modules are initialized internally (necessary verification and storage agents).
 * - Application has to provide valid Handle to handle the FOTA internal state handling.
 *
 * @param [in]  cmdProcessorHandle
 * command processor handle for the FOTA thread execution.
 *
 * @return #RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T LWM2MObjectFirmwareUpdate_Init(CmdProcessor_T* cmdProcessorHandle);

/**
 * @brief   This function triggers the FOTA state machine by checking the various
 * FOTA NVM variables necessary, validating their integrity and running FOTA.
 *
 * @note
 * - The node must have been registered successfully, prior.
 * - This must be called only once upon successful LWM2MObjectFirmwareUpdate_Init.
 *
 * @return #RETCODE_OK on success, or an error code otherwise.
 */
Retcode_T LWM2MObjectFirmwareUpdate_Enable(void);

#endif /* LWM2MOBJECTFIRMWAREUPDATE_H_ */

/**@} */

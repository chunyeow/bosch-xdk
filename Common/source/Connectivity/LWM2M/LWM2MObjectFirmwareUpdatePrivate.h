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
 *  @file
 */

#ifndef FOTASTATEMACHINEINTERNAL_H_
#define FOTASTATEMACHINEINTERNAL_H_

#include <Serval_Coap.h>
#include "BCDS_DownloadClient.h"
/*  local interface declaration declaration */

/* local type and macro definitions */
#define FOTA_LWM2M_STATUS_MASK                  (UINT8_C(0x0F))      /* lwm2m status mask */

/* Resource ID Definition for firmware update Object */
#define FOTA_RESOURCE_PACKAGE                    (UINT8_C(0))
#define FOTA_RESOURCE_PACKAGE_URI                (UINT8_C(1))
#define FOTA_RESOURCE_UPDATE                     (UINT8_C(2))
#define FOTA_RESOURCE_UPDATE_STATE               (UINT8_C(3))
#define FOTA_RESOURCE_UPDATE_SUPPORT_OBJ         (UINT8_C(4))
#define FOTA_RESOURCE_UPDATE_RESULT              (UINT8_C(5))

/**
 * @brief  Enum to represent the Firmware internal download states.
 * The low nibble encodes the status that is advertised via LWM2M in the
 * state resource ( as per FotaStateMachine_FW_State_T ).
 * The high nibble will denote the state machine intermediate states.
 */
enum FotaStateMachine_State_E
{
    FOTA_IDLE = (uint32_t) LWM2M_OBJ_FW_UPDATE_STATE_IDLE | 0x00,
    /* Firmware download error notify state. After this sate it goes back to Idle state. */
    FOTA_FAILED_NOTIFY = (uint32_t) LWM2M_OBJ_FW_UPDATE_STATE_IDLE | 0x10,
    /* Firmware coap block request state. */
    FOTA_DOWNLOADING_READY_FOR_REQUEST = (uint32_t) LWM2M_OBJ_FW_UPDATE_STATE_DOWNLOADING | 0x00,
    /* Once Valid URI received on Idle state then moves to FOTA_DOWNLOADING_NOTIFY to do
     initial firmware download setup and goes to  FOTA_DOWNLOADING_READY_FOR_REQUEST state */
    FOTA_DOWNLOADING_NOTIFY = (uint32_t) LWM2M_OBJ_FW_UPDATE_STATE_DOWNLOADING | 0x10,
    /* Waiting in this state for coap block response. once received the coap block then
     go to FOTA_DOWNLOADING_PROCESSING_RESPONSE state */
    FOTA_DOWNLOADING_WAITING_FOR_RESPONSE = (uint32_t) LWM2M_OBJ_FW_UPDATE_STATE_DOWNLOADING | 0x20,
    /* Waiting in this state for processing the coap block response. once processed the coap block
     * then go back to FOTA_DOWNLOADING_READY_FOR_REQUEST state */
    FOTA_DOWNLOADING_PROCESSING_RESPONSE = (uint32_t) LWM2M_OBJ_FW_UPDATE_STATE_DOWNLOADING | 0x30,
    /* Suspend an ongoing download */
    FOTA_DOWNLOADING_SUSPEND = (uint32_t) LWM2M_OBJ_FW_UPDATE_STATE_DOWNLOADING | 0x40,
    /* Resume an ongoing download */
    FOTA_DOWNLOADING_RESUME = (uint32_t) LWM2M_OBJ_FW_UPDATE_STATE_DOWNLOADING | 0x50,
    /* Stays in FOTA_DOWNLOADED after notifying till receives update trigger from the server */
    FOTA_DOWNLOADED = (uint32_t) LWM2M_OBJ_FW_UPDATE_STATE_DOWNLOADED | 0x00,
    /* Notifies the server for fota update */
    FOTA_UPDATING_NOTIFY = (uint32_t) LWM2M_OBJ_FW_UPDATE_STATE_DOWNLOADED | 0x10,
    /* Notifies the application about the new firmware downloaded and to do the firmware update */
    FOTA_UPDATING = (uint32_t) LWM2M_OBJ_FW_UPDATE_STATE_UPDATING | 0x00,
    /* State to detect if the state machine is run before being initialized */
    FOTA_INVALID = 0x99,
    FOTA_STATE_NOT_IN_USERPAGE = 0xFF,
};

/**
 * @brief Enum to represent the ID's for FOTA state machine environment variables
 * to read/write (map) the Persistent memory (ex. NVM,EEPROM,etc)
 */
enum FotaStateMachine_ContextInfo_E
{
    FOTA_CONTEXT_STATE = 0,
    FOTA_CONTEXT_NEW_FIRMWARE,
    FOTA_CONTEXT_DOWNLOAD_IN_PROGRESS,
    FOTA_CONTEXT_URL,
    FOTA_CONTEXT_CURRENT_RESULT,
    FOTA_CONTEXT_DOWNLOAD_CRC,
    FOTA_CONTEXT_MAX
};

/**
 * @brief Typedef to represent the ID for FOTA state machine environment variables
 * to be read/written (mapped) to the Persistent memory (ex. NVM,EEPROM,etc)
 */
typedef enum FotaStateMachine_ContextInfo_E FotaStateMachine_ContextInfo_T;

/**
 * @brief  Typedef to represent the Firmware download state.
 */
typedef enum FotaStateMachine_State_E FotaStateMachine_State_T;

/**
 * @note  Below is a set of guard macros to represent the FOTA state machine environment variables.
 * These are mapped to persistent memory based on need.
 *
 * To improve the code efficiency, a bit-wise operator macro set
 * is used instead of other options (for ex. the bit fields in combination with union).
 *
 * All the necessary FOTA state machine environment variable changes are mapped into an unsigned 32 bit variable.
 * The overview is as below:
 * ---------------------------------------------------------------------------------------------------------------------------------
 * |b31|b30|b29|b28|b27|b26|b25|b24|b23|b22|b21|b20|b19|b18|b17|b16|b15|b14|b13|b12|b11|b10| b9| b8| b7| b6| b5| b4| b3| b2| b1| b0|
 * ---------------------------------------------------------------------------------------------------------------------------------
 *   |___________________________|   |___________|   |   |   |   |   |___________________________|   |___________________________|
 *                 |                       |         |   |   |   |                 |                               |
 *                 |                       |         |   |   |   |                 |                               v
 *                 |                       |         |   |   |   |                 |   bit fields representing if a FOTA state machine
 *                 |                       |         |   |   |   |                 |   environment variables needs to be changed.
 *                 |                       |         |   |   |   |                 |   Represents the FotaStateMachine_ContextInfo_T
 *                 |                       |         |   |   |   |                 |   from right to left (only b0 to b4 are used now,
 *                 |                       |         |   |   |   |                 |   b5 to b7 are used - reserved for future use).
 *                 |                       |         |   |   |   |                 v
 *                 |                       |         |   |   |   |   1 byte representing the FotaStateMachine_State_T value (FOTA_CONTEXT_STATE).
 *                 |                       |         |   |   |   v
 *                 |                       |         |   |   |   1 bit representing the value of the flag representing if a new firmware is available (FOTA_CONTEXT_NEW_FIRMWARE).
 *                 |                       |         |   |   v
 *                 |                       |         |   |   1 bit representing the value of the flag representing if download is in progress (FOTA_CONTEXT_DOWNLOAD_IN_PROGRESS).
 *                 |                       |         |   v
 *                 |                       |         |   1 bit representing the value of the flag representing if empty (on 0) or valid URL (on 1) is to be written (FOTA_CONTEXT_URL).
 *                 |                       |         v
 *                 |                       |         Unused (reserved for future use)
 *                 |                       v
 *                 |                       1 nibble representing the FOTA firmware update result value (LWM2MObjectFirmwareUpdate_UpdateResult_E)
 *                 v
 *                 unused (reserved for future use)
 *
 * Since there are 5 valid FotaStateMachine_ContextInfo_T values, there are 5 pair of macros to set and retrieve the individual values.
 */
#define FOTA_SET_STATE(CMD_PROCESSOR_PARAM_U32, STATE)  \
    CMD_PROCESSOR_PARAM_U32=(((uint32_t)((CMD_PROCESSOR_PARAM_U32 & 0xFFFF00FF) | ((0x0000FF00 & ((uint32_t) STATE << 8UL)))) | (1UL<<((uint32_t)FOTA_CONTEXT_STATE))))

#define FOTA_SET_NEW_FIRMWARE(CMD_PROCESSOR_PARAM_U32, NEW_FIRMWARE)  \
    CMD_PROCESSOR_PARAM_U32=(((uint32_t)((CMD_PROCESSOR_PARAM_U32 & 0xFFFEFFFF) | ((0x00010000 & ((uint32_t) NEW_FIRMWARE << 16UL)))) | (1UL<<((uint32_t)FOTA_CONTEXT_NEW_FIRMWARE))))

#define FOTA_SET_DL_IN_PROGRESS(CMD_PROCESSOR_PARAM_U32, DL_IN_PROGRESS)  \
    CMD_PROCESSOR_PARAM_U32=(((uint32_t)((CMD_PROCESSOR_PARAM_U32 & 0xFFFDFFFF) | ((0x00020000 & ((uint32_t) DL_IN_PROGRESS << 17UL)))) | (1UL<<((uint32_t)FOTA_CONTEXT_DOWNLOAD_IN_PROGRESS))))

#define FOTA_SET_URL_TYPE(CMD_PROCESSOR_PARAM_U32, URL_TYPE)  \
    CMD_PROCESSOR_PARAM_U32=(((uint32_t)((CMD_PROCESSOR_PARAM_U32 & 0xFFFBFFFF) | ((0x00040000 & ((uint32_t) URL_TYPE << 18UL)))) | (1UL<<((uint32_t)FOTA_CONTEXT_URL))))

#define FOTA_SET_UPDATE_RESULT(CMD_PROCESSOR_PARAM_U32, UPDATE_RESULT)  \
    CMD_PROCESSOR_PARAM_U32=(((uint32_t)((CMD_PROCESSOR_PARAM_U32 & 0xFF0FFFFF) | ((0x00F00000 & ((uint32_t) UPDATE_RESULT << 20UL)))) | (1UL<<((uint32_t)FOTA_CONTEXT_CURRENT_RESULT))))

#define FOTA_RETREIVE_STATE_INFO(CMD_PROCESSOR_PARAM_U32, STATE, IS_SET)    \
    STATE=((IS_SET = (((CMD_PROCESSOR_PARAM_U32) & (1UL<<((uint32_t)FOTA_CONTEXT_STATE))) == 0UL) ? false : true) == true)? (CMD_PROCESSOR_PARAM_U32 >> 8UL)&0xFF:CMD_PROCESSOR_PARAM_U32

#define FOTA_RETREIVE_NEW_FIRMWARE_INFO(CMD_PROCESSOR_PARAM_U32, NEW_FIRMWARE, IS_SET)    \
    NEW_FIRMWARE=((IS_SET=(((CMD_PROCESSOR_PARAM_U32) & (1UL<<((uint32_t)FOTA_CONTEXT_NEW_FIRMWARE))) == 0UL) ? false : true) == true)? (CMD_PROCESSOR_PARAM_U32 >> 16UL)&0x01:CMD_PROCESSOR_PARAM_U32

#define FOTA_RETREIVE_DL_IN_PROGRESS(CMD_PROCESSOR_PARAM_U32, DL_IN_PROGRESS, IS_SET)    \
    DL_IN_PROGRESS=((IS_SET=(((CMD_PROCESSOR_PARAM_U32) & (1UL<<((uint32_t)FOTA_CONTEXT_DOWNLOAD_IN_PROGRESS))) == 0UL) ? false : true) == true)? (CMD_PROCESSOR_PARAM_U32 >> 17UL)&0x01:CMD_PROCESSOR_PARAM_U32

#define FOTA_RETREIVE_URL_TYPE(CMD_PROCESSOR_PARAM_U32, URL_TYPE, IS_SET)    \
    URL_TYPE=((IS_SET=(((CMD_PROCESSOR_PARAM_U32) & (1UL<<((uint32_t)FOTA_CONTEXT_URL))) == 0UL) ? false : true) == true)? (CMD_PROCESSOR_PARAM_U32 >> 18UL)&0x01:CMD_PROCESSOR_PARAM_U32

#define FOTA_RETREIVE_UPDATE_RESULT_INFO(CMD_PROCESSOR_PARAM_U32, UPDATE_RESULT, IS_SET)    \
    UPDATE_RESULT=((IS_SET=(((CMD_PROCESSOR_PARAM_U32) & (1UL<<((uint32_t)FOTA_CONTEXT_CURRENT_RESULT))) == 0UL) ? false : true) == true)? (CMD_PROCESSOR_PARAM_U32 >> 20UL)&0x0F:CMD_PROCESSOR_PARAM_U32

/**
 * @brief   Fota Resource package definition which will check the parser pointer.
 *
 * @param[in] serializer_ptr
 *                  Reference to a CoapSerializer_T object which identifies the current
 *                  instance of serializing context which should be used for serializing. It has
 *                  to be a valid pointer.
 *
 * @param[in] parser_ptr
 *                  Reference to a parser_ptr object which represents current outgoing message.
 *
 * @retval  retcode_t
 *              RC_OK, if successful
 *              RC_LWM2M_BAD_REQUEST otherwise.
 *
 */
static retcode_t FotaPackageResource(Lwm2mSerializer_T *serializerPntr, Lwm2mParser_T *parserPntr);

/**
 * @brief   Notification callback for firmware upgrade to LightWeight M2M or application.
 *
 * @param[in] serializer_ptr
 *                  Reference to a CoapSerializer_T object which identifies the current
 *                  instance of serializing context which should be used for serializing. It has
 *                  to be a valid pointer.
 *
 * @param[in] parser_ptr
 *                  Reference to a parser_ptr object which represents current outgoing message.
 *
 * @retval  retcode_t
 *              RC_OK, if successful
 *              RC_LWM2M_BAD_REQUEST otherwise.
 *
 */
static retcode_t FotaStateMachine_Update(Lwm2mSerializer_T *serializerPntr, Lwm2mParser_T *parserPntr);

/**
 * @brief   This function is used to get URI path or new download location from server and
 *          parsing it to Download the firmware.
 *
 * @param[in] serializer_ptr
 *                  Reference to a CoapSerializer_T object which identifies the current
 *                  instance of serializing context which should be used for serializing. It has
 *                  to be a valid pointer.
 *
 * @param[in] parser_ptr
 *                  Reference to a parser_ptr object which represents current outgoing message.
 *
 * @retval  retcode_t
 *              RC_OK, if successful
 *              RC_LWM2M_PARSING_ERROR, if data could not be parsed properly.
 *              RC_LWM2M_METHOD_NOT_ALLOWED, if invalid pointer passed.
 *
 */
static retcode_t FotaStateMachine_UriDownload(Lwm2mSerializer_T *serializerPntr, Lwm2mParser_T *parserPntr);

/**
 * @brief   This function handles the complete FOTA state machine state transition.
 *
 * @param[in] state
 * state to be changed.
 *
 * @retval  bool
 * boolean representing if the state change is permitted or not.
 *
 * @see FotaStateTransitionTable
 */
static bool FotaIsStateChangePermitted(FotaStateMachine_State_T state);

#endif /* FOTASTATEMACHINEINTERNAL_H_ */

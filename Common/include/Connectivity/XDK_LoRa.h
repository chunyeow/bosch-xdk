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
 * @ingroup CONNECTIVITY
 *
 * @defgroup LoRa LoRa
 * @{
 *
 * @brief This module handles the LORA peripheral feature
 *
 * @file
 **/

#ifndef XDK_LORA_H_
#define XDK_LORA_H_

/* local interface declaration ********************************************** */
#include "BCDS_Retcode.h"

/**
 * Enumeration representing supported join procedures as in LoRaWAN
 * specification V1.0.1
 */
typedef enum LoRa_JoinType_E
{
    LORA_JOINTYPE_OTAA,
    LORA_JOINTYPE_ABP
} LoRa_JoinType_T;

/**
 * @brief LORA events for the supported drivers
 */
typedef enum LoRa_Event_E
{
    LORA_EVENT_RECEIVED_PACKET,
    LORA_EVENT_SEND_FAILED,
    LORA_EVENT_RECEIVE_FAILED,
} LoRa_Event_T;
/**
 * @brief   Typedef of the LORA event notification callback.
 *
 * @param [in] event
 * run-time event of the LORA stack
 */
typedef void (*LoRa_EventNotificationCB_T)(LoRa_Event_T event);

/**
 * @brief   Structure to represent the LORA setup features.
 */
struct LoRa_Setup_S
{
    char* CodingRate; /**< Coding rate used for LORA communication */
    uint64_t* DevEUI; /**< Unique ID of the end device. if it is NULL then Hardware EUI will be used  */
    uint64_t AppEUI;  /**< AppEUI is unique to the Application Server and each Application Server will have its own AppEUI */
    uint8_t* AppKey;  /**< AppKey is the data encryption key used to "encode" the messages between the end nodes and the Application Server */
    uint32_t RxFreq;  /**< Frequency used for the Receive window*/
    uint32_t Freq;    /**< Frequency band used for the LORA*/
    LoRa_EventNotificationCB_T EventCallback; /**< LORA event callback */
    LoRa_JoinType_T JoinType; /**< LORA Join Type */
};

/**
 * @brief   Typedef to represent the WLAN setup feature.
 */
typedef struct LoRa_Setup_S LoRa_Setup_T;

/**
 * @brief This will setup the LORA
 *
 * @param[in] setup
 * Pointer to the LORA setup feature
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - This must be the first API to be called if LORA feature is to be used.
 * - Do not call this API more than once.
 */
Retcode_T LoRa_Setup(LoRa_Setup_T * setup);

/**
 * @brief This will enable the LORA(by calling this the Rxwindow2 frequency,DevEUI,appEUI,appKey, Coding rate will be set and saved)
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
Retcode_T LoRa_Enable(void);

/**
 * @brief performs a join request if the device has not joined a network yet
 *
 * @return RETCODE_OK in case of success error code otherwise.
 */
Retcode_T LoRa_Join(void);

/**
 * @brief Sets the Data Rate used for the transmission
 *
 * @param[in] dataRate to be used according to LoRaWAN spec
 *
 * @return RETCODE_OK in case of success error code otherwise.
 */
Retcode_T LoRa_SetDataRate(uint8_t dataRate);

/**
 * @brief Sends unconfirmed data frame over LoRa
 *
 * @param[in] LoRaPort The LoraWan Port that will be used for sending
 *
 * @param[in] dataBuffer Pointer to the data frame that will be sent
 *
 * @param[in] dataBufferSize Length of the frame to send
 *
 * @return RETCODE_OK in case of success error code otherwise. This function
 * returns if an ACK has been received from the Network Server or if an error
 * or a timeout occurred
 */
Retcode_T LoRa_SendUnconfirmed(uint8_t LoRaPort, uint8_t *dataBuffer, uint32_t dataBufferSize);

/**
 * @brief Sends confirmed data over LoRa
 *
 * @param[in] LoRaPort The LoraWan Port that will be used for sending
 *
 * @param[in] dataBuffer Pointer to the data frame that will be sent
 *
 * @param[in] dataBufferSize Length of the frame to send
 *
 * @return RETCODE_OK in case of success error code otherwise
 */
Retcode_T LoRa_SendConfirmed(uint8_t LoRaPort, uint8_t *dataBuffer, uint32_t dataBufferSize);

/**
 * @brief This will disable the LoRa medium
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
Retcode_T LoRa_Disable(void);

/**
 * @brief This will close the LoRa medium
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
Retcode_T LoRa_Close(void);

/**
 * @brief This will read the hardware dev EUI of the LoRa medium
 *
 * @param[out] hwDevEUI place holder for Unique Identifier of the LoRa module
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
Retcode_T LoRa_GetHwEUI(uint64_t *hwDevEUI);

/**
 * @brief Sets the Adaptive Data Rate option to ON/OFF
 *
 * @param adr set to true if ADR is to be enabled, set to false otherwise.
 *
 * @return RETCODE_OK in case of success error code otherwise.
 */
Retcode_T LoRa_SetADR(bool enable);

/**
 * @brief Returns the status of the Adaptive data Rate
 *
 * @param[out] adr
 *
 * @return RETCODE_OK in case of success error code otherwise.
 */
Retcode_T LoRa_GetADR(bool* adr);

#endif /* XDK_LORA_H_ */


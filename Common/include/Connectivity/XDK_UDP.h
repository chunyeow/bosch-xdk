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
 * @defgroup UDP UDP
 * @{
 *
 * @brief This module handles the UDP services.
 *
 * @file
 */

/* header definition ******************************************************** */
#ifndef XDK_UDP_H_
#define XDK_UDP_H_

/* local interface declaration ********************************************** */
#include "BCDS_Retcode.h"

/**
 * @brief   Enum to represent the UDP setup features.
 */
enum UDP_Setup_E
{
    UDP_SETUP_USE_CC31XX_LAYER,
    UDP_SETUP_USE_SERVALPAL_LAYER,
};

/**
 * @brief   Typedef to represent the UDP setup feature.
 */
typedef enum UDP_Setup_E UDP_Setup_T;

/**
 * @brief   This will setup the UDP communication
 *
 * @param[in] setup
 * Pointer to the UDP setup feature
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - This must be the first API to be called if UDP feature is to be used.
 * - Do not call this API more than once.
 */
Retcode_T UDP_Setup(UDP_Setup_T setup);

/**
 * @brief   This will enable the UDP communication
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #UDP_Setup must have been successful prior.
 * - Do not call this API more than once.
 */
Retcode_T UDP_Enable(void);

/**
 * @brief   This will create a local end-point for communication
 *  It creates a new socket of UDP socket type and provides
 *  the handle to the user for further communication.
 *
 * @param[in/out] handle
 * Handle to the UDP socket. User must provide the memory.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T UDP_Open(int16_t *handle);

/**
 * @brief   This will send a message to the server.
 *
 * @param[in] handle
 * Handle to the UDP socket
 *
 * @param[in] serverIp
 * Server IP Address
 *
 * @param[in] serverPort
 * Server Port Number
 *
 * @param[in] dataPtr
 * Pointer to the buffer containing the message to be sent
 *
 * @param[in] dataLen
 * Length of the message to be sent
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note    Irrespective of send success or not, socket needs to be closed from application
 */
Retcode_T UDP_Send(int16_t handle, uint32_t serverIp, uint16_t serverPort, uint8_t * dataPtr, uint32_t dataLen);

/**
 * @brief   This function will delete the local end-point for communication
 *
 * @param[in] handle
 * Handle to the UDP socket
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */
Retcode_T UDP_Close(int16_t handle);

#endif /* XDK_UDP_H_ */

/**@} */

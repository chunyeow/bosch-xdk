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
 * @defgroup HTTPRESTCLIENT HTTP REST client
 * @{
 *
 * @brief This module handles the HTTP rest client services (POST and GET).
 *
 * @file
 */

/* header definition ******************************************************** */
#ifndef XDK_HTTPRESTCLIENT_H_
#define XDK_HTTPRESTCLIENT_H_

/* local interface declaration ********************************************** */
#include "BCDS_Retcode.h"
#include "BCDS_CmdProcessor.h"

/**
 * @brief   Structure to represent the HTTP Rest Client setup features.
 */
struct HTTPRestClient_Setup_S
{
    bool IsSecure; /**< Boolean representing if we will do a HTTP secure communication */
    CmdProcessor_T * CmdProcessor; /**< Commandprocessor Representation */
};

/**
 * @brief   Typedef to represent the HTTP Rest Client setup feature.
 */
typedef struct HTTPRestClient_Setup_S HTTPRestClient_Setup_T;

/**
 * @brief   Structure to represent the HTTP Rest Client POST/GET common configurations.
 */
struct HTTPRestClient_Config_S
{
    bool IsSecure; /**< Boolean representing if it is a HTTP secure communication */
    const char* DestinationServerUrl; /**< Pointer to the destination server URL */
    uint16_t DestinationServerPort; /**< Value of the destination server port number */
    uint32_t RequestMaxDownloadSize; /**< Value of the maximum amount of data we download in a single request (in bytes) */
};

/**
 * @brief   Typedef to represent the HTTP Rest Client POST/GET common configuration.
 */
typedef struct HTTPRestClient_Config_S HTTPRestClient_Config_T;

/**
 * @brief   Structure to represent the HTTP Rest Client POST configurations.
 */
struct HTTPRestClient_Post_S
{
    const char * Payload; /**< Pointer to the payload to be POST'ed */
    uint32_t PayloadLength; /**< Length of the payload to be POST'ed */
    const char * Url; /**< Pointer to the destination server POST URL extension */
    const char * RequestCustomHeader0; /**< Pointer to the POST request custom header 0 */
    const char * RequestCustomHeader1; /**< Pointer to the POST request custom header 1 */
};

/**
 * @brief   Typedef to represent the HTTP Rest Client POST configuration.
 */
typedef struct HTTPRestClient_Post_S HTTPRestClient_Post_T;

/**
 * @brief   Typedef of the HTTP GET payload callback.
 *
 * @param [in] responseContent
 * Pointer to the GET request response
 *
 * @param [in] responseContentLen
 * Length of the GET request response
 *
 * @param [in] isLastMessage
 * Boolean to represent if it is the last part of the ongoing message
 */
typedef void (*HTTPRestClient_GetCB_T)(const char* responseContent, uint32_t responseContentLen, bool isLastMessage);

/**
 * @brief   Structure to represent the HTTP Rest Client GET configurations.
 */
struct HTTPRestClient_Get_S
{
    const char * Url; /**< Pointer to the destination server GET URL extension */
    HTTPRestClient_GetCB_T GetCB; /**< Function Pointer to be notified of the GET responses */
    uint32_t GetOffset; /**< Offset for GET */
};

/**
 * @brief   Typedef to represent the HTTP Rest Client GET configuration.
 */
typedef struct HTTPRestClient_Get_S HTTPRestClient_Get_T;

/**
 * @brief This will setup the HTTP rest client
 *
 * @param[in] setup
 * Pointer to the HTTP Rest Client setup feature
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - If setup->IsSecure is enabled, then we will setup the HTTPs server certificates as well.
 * - This must be the first API to be called if HTTP Rest Client feature is to be used.
 * - Do not call this API more than once.
 */
Retcode_T HTTPRestClient_Setup(HTTPRestClient_Setup_T * setup);

/**
 * @brief This will enable the HTTP rest client
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - If setup->IsSecure was enabled at the time of #HTTPRestClient_Setup,
 *   then we will enable the HTTPs server certificates as well.
 * - #HTTPRestClient_Setup must have been successful prior.
 * - #WLAN_Enable must have been successful prior.
 * - Do not call this API more than once.
 */
Retcode_T HTTPRestClient_Enable(void);

/**
 * @brief This will do a HTTP rest client POST
 *
 * @param[in] config
 * Pointer to the HTTP Rest Client general configuration
 *
 * @param[in] post
 * Pointer to the HTTP Rest Client POST specific configuration
 *
 * @param[in] timeout
 * Timeout in milli-second to complete a POST cycle
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #HTTPRestClient_Setup and #HTTPRestClient_Enable must have been successful prior.
 * - This is a blocking call
 * - HTTP is on top of TCP for XDK, there is something called “KeepAlive” feature for the HTTP session
 *   which the serval stack currently doesn’t allow it to be configured. This is currently “false”.
 *   This means that every single time, when there is a HTTP POST, a new TCP socket is opened,
 *   HTTP session is established and data is POST’ed followed by socket closure.
 *   Now, from an application perspective, the moment a payload is POST’ed,
 *   we would receive a notification that this was successful.
 *   And then the ServalStack will proceed to close the session and the socket.
 *   This takes some time based on the system load.
 *   During this time, if we trigger another HTTP POST,
 *   since the socket is busy with “close procedure”, the trigger would fail.
 *   Worst case, 100 milli-second delay is expected to be provided by the user as a wait time
 *   if he is to POST continuously.
 */
Retcode_T HTTPRestClient_Post(HTTPRestClient_Config_T * config, HTTPRestClient_Post_T * post, uint32_t timeout);

/**
 * @brief This will do a HTTP rest client GET request
 *
 * @param[in] config
 * Pointer to the HTTP Rest Client general configuration
 *
 * @param[in] get
 * Pointer to the HTTP Rest Client GET specific configuration
 *
 * @param[in] timeout
 * Timeout in milli-second to complete a GET cycle
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #HTTPRestClient_Setup and #HTTPRestClient_Enable must have been successful prior.
 * - This is a blocking call
 * - If get->GetCB can be NULL, but then the application will not be notified of the response received from the server.
 *
 * @see #HTTPRestClient_Post note. GET is similar to POST w.r.t TCP socket layer.
 * Therefore 100 milli-second delay is expected to be provided by the user as a wait time
 * if he is to GET continuously.
 */
Retcode_T HTTPRestClient_Get(HTTPRestClient_Config_T * config, HTTPRestClient_Get_T * get, uint32_t timeout);

/**
 * @brief This will do Firmware download through http and stored it in sd card if the get->GetCB is NULL.
 *        If get->GetCB is defined, then firmware data will be passed to application.
 *
 * @param[in] config
 * Pointer to the HTTP Rest Client general configuration
 *
 * @param[in] get
 * Pointer to the HTTP Rest Client GET specific configuration
 *
 * @param[in] timeout
 * GET cycle response timeout in milli second
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - File system and sd card must be initialized before calling this api
 * - After successful completion of fota download, firmware verification also done in this function
 * - If get->GetCB can be NULL, but then the application will not be notified of the response received from the server.
 * - This is a blocking call
 *
 * - If get->GetCB is NULL then,
 *   - Provide the Url path from where can get the bin data. While downloading, this api will append the get size and offset to the given url
 *      - Example: if the given get->Url = "/range", then while downloading it will append get size and offset like "/range/512/0/"
 *          -- Here 512 is get size and 0 is bin file offset position
 *          -- Final url "/range/512/0/" only used to request the bin data from server
 *      - Firmware file is stored as firmware.bin after the verification. it will overwrite if it is already present.
 *
 * - If get->GetCB is defined then,
 *   - Provide the Url path from where can get the bin data along with size and offset
 *
 */
Retcode_T HTTPRestClient_GetFirmware(HTTPRestClient_Config_T * config, HTTPRestClient_Get_T * get, uint32_t timeout);

#endif /* XDK_HTTPRESTCLIENT_H_ */

/**@} */

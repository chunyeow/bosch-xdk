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
 * @defgroup SNTP SNTP
 * @{
 *
 * @brief This module handles the SNTP timestamp (from server and system).
 *
 * @file
 */

/* header definition ******************************************************** */
#ifndef XDK_SNTP_H_
#define XDK_SNTP_H_

/* local interface declaration ********************************************** */
#include "BCDS_Retcode.h"
/**
 * @brief   Structure to represent the SNTP setup features.
 */
struct SNTP_Setup_S
{
    const char* ServerUrl; /**< Pointer to the SNTP server URL */
    uint16_t ServerPort; /**< Port number of the SNTP server */
};

/**
 * @brief   typedef to represent the SNTP setup feature.
 */
typedef struct SNTP_Setup_S SNTP_Setup_T;

/**
 * @brief   This will setup the SNTP
 *
 * @param[in] setup
 * Pointer to the SNTP setup feature
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - If setup->UseServerUrl is enabled, then setup->ServerIpAddr is unused
 * - If setup->UseServerUrl is disabled, then setup->ServerUrl is unused
 * - This must be the first API to be called if SNTP feature is to be used.
 * - #WLAN_Setup must have been successful prior.
 * - #ServalPAL_Setup must have been successful prior.
 * - Do not call this API more than once.
 */
Retcode_T SNTP_Setup(SNTP_Setup_T * setup);

/**
 * @brief   This will enable the SNTP
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #WLAN_Enable must have been successful prior.
 * - #ServalPAL_Enable must have been successful prior.
 * - #SNTP_Setup must have been successful prior.
 * - Do not call this API more than once.
 */
Retcode_T SNTP_Enable(void);

/**
 * @brief   This will request SNTP time-stamp from the SNTP server and return the same.
 *
 * @param[in/out] sntpTimeStamp
 * SNTP time-stamp in UNIX format. 0 if server response was 0. Interface user must provide data memory.
 *
 * @param[in] timeout
 * Timeout for response from the server.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #SNTP_Setup and #SNTP_Enable must have been successful prior.
 * - Must be called prior to #SNTP_GetTimeFromSystem usage.
 */
Retcode_T SNTP_GetTimeFromServer(uint64_t * sntpTimeStamp, uint32_t timeout);

/**
 * @brief   This will provide SNTP time-stamp based on last synced SNTP server value and system time.
 *
 * @param[in/out] sntpTimeStamp
 * Pointer to SNTP time-stamp in UNIX format. In seconds. 0 if server response failed. Interface user must provide data memory.
 *
 * @param[in/out] timeLapseInMs
 * Pointer to time lapsed in millisecond. Interface user must provide data memory.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #SNTP_Setup and #SNTP_Enable must have been successful prior.
 * - #SNTP_GetTimeFromSystem must have be called prior to this.
 * - Accuracy is limited due to XDK's internal clock inaccuracies w.r.t real time.
 * - Perform #SNTP_GetTimeFromServer once in ~40 days atleast to avoid invalid time stamp
 *   since we use uint32_t system tick which will overflow otherwise.
 */
Retcode_T SNTP_GetTimeFromSystem(uint64_t * sntpTimeStamp, uint32_t * timeLapseInMs);

/**
 * @brief   This will set SNTP time-stamp for the device.
 *
 * @param[in] sntpTimeStamp
 * SNTP time-stamp in UNIX format. In seconds.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #SNTP_Setup and #SNTP_Enable must have been successful prior.
 * - Make sure that the right time is provided.
 */
Retcode_T SNTP_SetTime(uint64_t sntpTimeStamp);
/**
 * @brief   This will disable the SNTP
 *
 * @return  RETCODE_OK on success, or an error code otherwise for XDK_UTILITY_SERVALPAL enabled.
 *          RETCODE_NOT_SUPPORTED for XDK_UTILITY_SERVALPAL disabled.
 *
 * @note
 * - #SNTP_Enable must have been successful prior.
 */
Retcode_T SNTP_Disable(void);
#endif /* XDK_SNTP_H_ */
/**@}*/

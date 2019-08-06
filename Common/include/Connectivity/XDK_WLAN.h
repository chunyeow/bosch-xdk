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
 * @defgroup WLAN WLAN
 * @{
 *
 * @brief This module handles the WLAN services (Personal WPA2 and Enterprise WPA2 connection).
 *
 * @file
 */
/* header definition ******************************************************** */
#ifndef XDK_WLAN_H_
#define XDK_WLAN_H_

/* local interface declaration ********************************************** */
#include "BCDS_Retcode.h"

/**
 * @brief   Structure to represent the WLAN setup features.
 */
struct WLAN_Setup_S
{
    bool IsEnterprise; /**< Boolean representing if it is Enterprise WPA2 connection (If disabled Personal WPA2 connection is done) */
    bool IsHostPgmEnabled; /**< Boolean representing if dummy certificate is to be flashed for Enterprise WPA2 connection. Unused if IsEnterprise is disabled. */
    const char* SSID; /**< Pointer to the WLAN SSID */
    const char* Username; /**< Pointer to the WLAN username. Unused if IsEnterprise is disabled. */
    const char* Password; /**< Pointer to the WLAN password */
    bool IsStatic; /**< Boolean representing if it is static IPv4 setting (If disabled then DHCP is done) */
    uint32_t IpAddr; /**< IP address. Unused if IsStatic is disabled. */
    uint32_t GwAddr; /**< Gateway address. Unused if IsStatic is disabled. */
    uint32_t DnsAddr; /**< DNS address. Unused if IsStatic is disabled. */
    uint32_t Mask; /**< Mask for the IP address. Unused if IsStatic is disabled. */
};

/**
 * @brief   Typedef to represent the WLAN setup feature.
 */
typedef struct WLAN_Setup_S WLAN_Setup_T;

/**
 * @brief This will setup the WLAN
 *
 * @param[in] setup
 * Pointer to the WLAN setup feature
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - If setup->IsHostPgmEnabled is enabled, then WLAN chip programming for dummy certificate will be setup as well.
 * - This must be the first API to be called if WLAN feature is to be used.
 * - Do not call this API more than once.
 */
Retcode_T WLAN_Setup(WLAN_Setup_T * setup);

/**
 * @brief This will enable the WLAN
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - If setup->IsHostPgmEnabled was enabled at the time of #WLAN_Setup,
 *   then WLAN chip programming for dummy certificate will be enabled (programmed) as well.
 * - #WLAN_Setup must have been successful prior.
 * - This is a blocking call and will connect to the WLAN network.
 * - Do not call this API more than once.
 */
Retcode_T WLAN_Enable(void);

/**
 * @brief This will disable the WLAN
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - This will disconnect from the WLAN network.
 */
Retcode_T WLAN_Disable(void);

/**
 * @brief This will validate if the WLAN IP configuration changed since #WLAN_Enable
 *
 * @param[in/out] ipChangeStatus
 * Boolean if the IP changed
 *
 * @param[in/out] newIp
 * new IP if *ipChangeStatus is true
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #WLAN_Enable must have been successful prior.
 */
Retcode_T WLAN_IsIpChanged(bool * ipChangeStatus, uint32_t * newIp);

/**
 * This will converts a given uint32_t IP address to a null-terminated string, and
 * stores the result in the given buffer.
 *
 * @param[in] ipAddress
 * The IP address to be converted.
 *
 * @param[in/out] string
 * A valid pointer to the buffer which the IP string should be written to. The
 * given buffer must be larger than the maximal length of the IP string
 * presentation by at least 1 (for the null-character).
 * The written string will be NULL terminated.
 * This must have a buffer size greater than 15U bytes.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
Retcode_T WLAN_ConvertIPAddressToString(uint32_t ipAddress, char * string);

/**
 * @brief This will reconnect to the specified network if it is available
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 */

Retcode_T WLAN_Reconnect(void);

#endif /* XDK_WLAN_H_ */

/**@} */

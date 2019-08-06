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
 * @defgroup XDKUTILS Utils
 * @{
 *
 *  @brief Interface header file for the Utilities.
 *
 * @file
 */
/* header definition ******************************************************** */
#ifndef XDK_UTILS_H_
#define XDK_UTILS_H_

#include <stdio.h>

#include "BCDS_Assert.h"
#include "BCDS_Basics.h"
#include "BCDS_Retcode.h"
#include "BCDS_CmdProcessor.h"

/* Helper Macro to convert readable representation of IPv4 in terms of uint32_t variable */
#define XDK_NETWORK_IPV4(add_3,add_2,add_1,add_0)     \
    ((((uint32_t)add_0 << 24) & 0xFF000000) | \
        (((uint32_t)add_1 << 16) & 0xFF0000) | \
        (((uint32_t)add_2 << 8) & 0xFF00) | \
        ((uint32_t)add_3 & 0xFF) )

/**
 * @brief   Enum to represent the Nvm Data Info.
 */
enum Utils_NVMMacInfo_E
{
    UTILS_BLE_MAC_DATA,
    UTILS_WIFI_MAC_DATA
};

/**
 * @brief   Typedef to represent theNvm Data Info.
 */
typedef enum Utils_NVMMacInfo_E Utils_NVMMacInfo_T;

/**
 * @brief This function to Get the Version string by combining the XDK SW Release (#XdkVersion_GetVersion)
 *        i.e., Workbench Release version & Fota Container Application Firmware Version
 *        So, the combined Version Information will be like (i.e., vAA.BB.CC-aa.bb.cc )
 *
 * @param[out] verstring - XDK Version string.For Fota support XDK SW, verstring = vAA.BB.CC-aa.bb.cc,
 *                         XDK Version string.For Fota support XDK SW, verstring = vAA.BB.CC-aa.bb.cc, .
 *
 * @note
 * -  XDK SW Release version,        AA - Major; BB - Minor; CC - Patch
 * -  Application Firmware Version   aa - Major; bb - Minor; cc - Patch
 * -  If Fota supported XDK SW, then verstring = vAA.BB.CC-aa.bb.cc. for without Fota supported XDK SW verstring = vAA.BB.CC-xx.xx.xx
 * -  xx.xx.xx means not applicable.
 *
 * @return RETCODE_OK if success, or an error code otherwise.
 */
Retcode_T Utils_GetXdkVersionString(uint8_t * verstring);

/**
 * @brief This function reads the BLE/wifi MAC address from NVM module.
 *
 * @param [in]  nvmMacInfo
 * medium(BLE/WIFI) whose data should be read from NVM.
 *
 * @param [in]  data
 * Pointer to the buffer to store data.
 *
 * @return RETCODE_OK in success scenario, error case otherwise
 *
 * @note The buffer length for MAC address for BLE and WIFI is 6 bytes
 */
Retcode_T Utils_GetMacInfoFromNVM(Utils_NVMMacInfo_T nvmMacInfo, uint8_t *data);

/**
 * @brief This function prints if last reset was caused due to watchdog timer
 */
void Utils_PrintResetCause(void);

/**
 * @brief Convert human readable IPv4 address to UINT32 which is in big endian format
 *
 * @param[in] ipDottedQuad
 *        Input C string e.g. "192.168.0.1"
 *
 * @param[out] ipAddress
 *         ipAddress pointer to variable to hold the ip data
 *
 * @return RETCODE_OK in case of success
 *         #RETCODE_NULL_POINTER  when the ipDottedQuad or ipAddress is NULL
 *         #RETCODE_INVALID_PARAM when the parameter is invalid
 */
Retcode_T Utils_ConvertIpStringToNumber(const char* ipDottedQuad ,uint32_t *ipAddress);

#endif /* XDK_UTILS_H_ */

/**@}*/

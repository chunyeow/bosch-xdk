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

/* module includes ********************************************************** */

#include "XdkCommonInfo.h"
/* This is used to define the correct module ID for generation of module
 * error codes
 */
#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID (uint32_t)XDK_COMMON_ID_NETWORK_CONFIG

/* own header files */
#include "BCDS_NetworkConfig.h"
#include "BCDS_WlanNetworkConfig.h"

/* additional interface header files */
#include "simplelink.h"
#include "BCDS_WlanConnect.h"
#include "BCDS_BSP_Board.h"

/* constant definitions ***************************************************** */

/* local type and macro definitions */
#define NETWORKCONFIG_FAILURE                 INT32_C(-1) /**< Macro for defining -1*/
#define NETWORKCONFIG_SUCCESS                 INT32_C(0) /**< Macro for defining 0*/
#define NETWORKCONFIG_ZERO                    INT32_C(0) /**< Macro for defining 0*/
#define NETWORKCONFIG_ONE                     UINT8_C(1) /**< Macro for defining 1*/
#define NETWORKCONFIG_TIMEOUT                 UINT16_C(0xFF) /**< Macro for defining timeout*/
/* local function prototype declarations */

/* local variables ********************************************************** */

/* global variables ********************************************************* */
static volatile NetworkConfig_IpCallback_T IpConfigCallback; /**< Variable for storing the DHCP IP callback*/
static volatile NetworkConfig_IpStatus_T NetworkIpStatus; /**< Flag variable for IP obtained status */

/* inline functions ********************************************************* */

/* local functions ********************************************************** */

/* global functions ********************************************************* */
NetworkConfig_IpStatus_T NetworkConfig_GetIpStatus(void)
{
    return ((NetworkConfig_IpStatus_T)WlanNetworkConfig_GetIpStatus());
}

void NetworkConfig_SetIpStatus(NetworkConfig_IpStatus_T ipStatus)
{
    NetworkIpStatus = ipStatus;
}

Retcode_T NetworkConfig_GetIpSettings(NetworkConfig_IpSettings_T* myIpSettings)
{
     return (WlanNetworkConfig_GetIpSettings((WlanNetworkConfig_IpSettings_T*)myIpSettings));
}

Retcode_T NetworkConfig_SetIpStatic(NetworkConfig_IpSettings_T myIpSettings)
{
    WlanNetworkConfig_IpSettings_T *ipSettings;
    ipSettings = (WlanNetworkConfig_IpSettings_T *)&myIpSettings;

    return WlanNetworkConfig_SetIpStatic(*ipSettings);
}

Retcode_T NetworkConfig_SetIpDhcp(NetworkConfig_IpCallback_T myIpCallback)
{
    return (WlanNetworkConfig_SetIpDhcp((WlanNetworkConfig_IpCallback_T)myIpCallback));
}

uint32_t NetworkConfig_Ipv4Value(uint8_t add3, uint8_t add2, uint8_t add1, uint8_t add0)
{
    uint32_t returnValue;

    returnValue = SL_IPV4_VAL(add3, add2, add1, add0);

    return (returnValue);
}

uint8_t NetworkConfig_Ipv4Byte(uint32_t ipValue, uint8_t index)
{
    uint8_t returnValue;

    returnValue = SL_IPV4_BYTE(ipValue, index);

    return (returnValue);
}

Retcode_T NetworkConfig_GetIpAddress(uint8_t *url, uint32_t *destAddr)
{
    return WlanNetworkConfig_GetIpAddress(url,destAddr);
}

/** ************************************************************************* */

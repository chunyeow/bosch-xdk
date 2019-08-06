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
 * @file
 *
 * This module handles the WLAN services (Personal WPA2 and Enterprise WPA2 connection)
 *
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_WLAN

#if XDK_CONNECTIVITY_WLAN

/* own header files */
#include "XDK_WLAN.h"
#include "Protected/WLAN.h"

/* system header files */
#include <stdio.h>

/* additional interface header files */
#include "WLANHostPgm.h"
#include "BCDS_WlanNetworkConnect.h"
#include "BCDS_WlanNetworkConfig.h"
#include "simplelink.h"
#include "socket.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "wlan.h"

/* constant definitions ***************************************************** */

#define WLANNETWORKCONFIG_ONE                  UINT8_C(1) /**< Macro for defining 1 */
#define WLANNETWORKCONFIG_FAILURE              INT32_C(-1) /**< Macro for defining -1 */
#define WLANNETWORKCONFIG_SUCCESS              INT32_C(0) /**< Macro for defining 0 */
#define WLANNETWORKCONFIG_TIMEOUT              UINT16_C(0xFF) /**< Macro for defining timeout */
#define WLAN_MAC_ADDR_LENGTH                    UINT8_C(6) /**< Macro used to specify MAC address length */
#define WLAN_CONNECT_WPA_RETRY_MAX              INT32_C(3) /**< Macro used for maximum retry for WlanNetworkConnect_WPA API */
#define IPV4_BYTE(val,index)                    ((val >> (index*8)) & 0xFF) /**<Macro used changed IP address to Byte> */
#define IP_ADDR_LEN                             INT32_C(15) /**< Macro used for maximum IP address length */
#define WLANNETWORK_EVENT_TIMEOUT				UINT32_C(200000) /**< Macro for wlan event timeout for simplelink */

/* local variables */
static uint32_t WLANIpAddress = 0UL;
static SemaphoreHandle_t WlanEventSemaphore = NULL;
static SemaphoreHandle_t NetworkConfigSemaphore = NULL;

/**< WLAN Setup */
static WLAN_Setup_T WLANSetup;
/**< To keep the MAC address returned by the WLAN network device */
static char WLANMacAddress[] = "00:00:00:00:00:00";

/* global variables ********************************************************* */

/* local functions */

static void WlanConnectStatusCallback(WlanNetworkConnect_Status_T status);
static void NetworkIpConfigCallbackFunc(WlanNetworkConfig_IpStatus_T ipStatus);

static Retcode_T WlanEventSemaphoreHandle(void)
{
    Retcode_T retcode = RETCODE_OK;
    uint8_t count = 0;
    if (pdTRUE == xSemaphoreTake(WlanEventSemaphore, WLANNETWORK_EVENT_TIMEOUT))
    {
        do
        {
            if ((WLANNWCNF_IPSTATUS_IPV4_AQRD == WlanNetworkConfig_GetIpStatus()) && (WLANNWCT_STATUS_CONNECTED == WlanNetworkConnect_GetStatus()))
            {
                retcode = RETCODE_OK;
            }
            else
            {
                vTaskDelay(500);
                count++;
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_WLAN_CONNECT_FAILED);
            }
        } while ((RETCODE_OK != retcode)
                && (UINT8_C(5) >= count));
    }
    else
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR);
    }
    return retcode;
}

/**
 * @brief convert IP address to string
 */
static int ConvertIpAddrToString(uint32_t const *ipAddr_ptr, char *buffer)
{
    int len = 0;
    if ((NULL != ipAddr_ptr) && (NULL != buffer))
    {
        len = sprintf(buffer, "%d.%d.%d.%d", (int) IPV4_BYTE(*ipAddr_ptr, 0), (int) IPV4_BYTE(*ipAddr_ptr, 1),
                (int) IPV4_BYTE(*ipAddr_ptr, 2), (int) IPV4_BYTE(*ipAddr_ptr, 3));
    }

    return len;
}

/**
 * @brief Get IP address
 */
static uint32_t* GetMyIpAddr(void)
{
    return &WLANIpAddress;
}

/**
 * @brief Logs the WLAN chip version informations
 */
static void WLANVersionLog(void)
{
    SlVersionFull Version;
    _u8 ConfigOpt;
    _u8 ConfigLen = (_u8) sizeof(Version);
    _i32 ret;

    memset(&Version, 0, sizeof(Version));
    ConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ret = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ConfigOpt, &ConfigLen, (_u8 *) (&Version));
    if (SL_OS_RET_CODE_OK == ret)
    {
        if (Version.ChipFwAndPhyVersion.ChipId & 0x10)
            printf("\r\nThis is a CC3200");
        else
            printf("\r\nThis is a CC3100");

        if (Version.ChipFwAndPhyVersion.ChipId & 0x2)
            printf("Z device\r\n");
        else
            printf("R device\r\n");

        printf("CHIP 0x%lx\r\nMAC 31.%lu.%lu.%lu.%lu\r\nPHY %u.%u.%u.%u\r\nNWP %lu.%lu.%lu.%lu\r\nROM 0x%x\r\nHOST %lu.%lu.%lu.%lu\r\n",
                Version.ChipFwAndPhyVersion.ChipId,
                Version.ChipFwAndPhyVersion.FwVersion[0], Version.ChipFwAndPhyVersion.FwVersion[1],
                Version.ChipFwAndPhyVersion.FwVersion[2], Version.ChipFwAndPhyVersion.FwVersion[3],
                Version.ChipFwAndPhyVersion.PhyVersion[0], Version.ChipFwAndPhyVersion.PhyVersion[1],
                Version.ChipFwAndPhyVersion.PhyVersion[2], Version.ChipFwAndPhyVersion.PhyVersion[3],
                Version.NwpVersion[0], Version.NwpVersion[1], Version.NwpVersion[2], Version.NwpVersion[3],
                Version.RomVersion,
                SL_MAJOR_VERSION_NUM, SL_MINOR_VERSION_NUM, SL_VERSION_NUM, SL_SUB_VERSION_NUM);
    }
    else
    {
        printf("WLANVersionLog : sl_DevGet failed with value %d\r\n", (int) ret);
    }
}

/**
 * @brief Validates the WLAN IP credential informations and logs it
 */
static Retcode_T WLANIpCredentialsValidateAndLog(void)
{
    Retcode_T retcode = RETCODE_OK;
    int32_t Result = INT32_C(-1);
    WlanNetworkConfig_IpSettings_T myIpSettings;
    char ipAddress[IP_ADDR_LEN] = { 0 };
    uint32_t* IpaddressHex = GetMyIpAddr();
    uint32_t NewIpAddressBin;

    vTaskDelay(pdMS_TO_TICKS(1000)); /* Needed before retrieving IP upon connection */

    memset(&myIpSettings, (uint32_t) 0, sizeof(myIpSettings));
    retcode = WlanNetworkConfig_GetIpSettings(&myIpSettings);
    if (RETCODE_OK == retcode)
    {
        *IpaddressHex = htonl(myIpSettings.ipV4);
        Result = ConvertIpAddrToString(IpaddressHex, ipAddress);
        if (Result < 0)
        {
            printf("WLANIpCredentialsValidateAndLog : Couldn't convert the IP address to string format \r\n");
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_WLAN_SETUP_IP_CONV_FAILED);
        }
    }
    if (RETCODE_OK == retcode)
    {
        (void) ConvertIpAddrToString(IpaddressHex, ipAddress);
        printf("IP address of device  %s\r\n", ipAddress);
        NewIpAddressBin = ntohl(myIpSettings.ipV4Mask);
        (void) ConvertIpAddrToString(&NewIpAddressBin, ipAddress);
        printf("              Mask    %s\r\n", ipAddress);
        NewIpAddressBin = ntohl(myIpSettings.ipV4Gateway);
        (void) ConvertIpAddrToString(&NewIpAddressBin, ipAddress);
        printf("              Gateway %s\r\n", ipAddress);
        NewIpAddressBin = ntohl(myIpSettings.ipV4DnsServer);
        (void) ConvertIpAddrToString(&NewIpAddressBin, ipAddress);
        printf("              DNS     %s\r\n", ipAddress);
    }
    return retcode;
}

/**
 * @brief WLANEnterpriseWPA2Connect will setup Enterprise WPA2 connection
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T WLANEnterpriseWPA2Connect(void)
{
    Retcode_T retcode = RETCODE_OK;
    uint8_t _macVal[WLAN_MAC_ADDR_LENGTH];
    uint8_t _macAddressLen = WLAN_MAC_ADDR_LENGTH;
    WlanNetworkConfig_IpSettings_T myIpSettings;
    memset(&myIpSettings, (uint32_t) 0, sizeof(myIpSettings));
    WlanNetworkConnect_SSID_T connectSSID = (WlanNetworkConnect_SSID_T) WLANSetup.SSID;
    WlanNetworkConnect_Username_T connectUsername = (WlanNetworkConnect_Username_T) WLANSetup.Username;
    WlanNetworkConnect_PassPhrase_T connectPassPhrase = (WlanNetworkConnect_PassPhrase_T) WLANSetup.Password;

    /* Initialize the Wireless Network Driver */
    retcode = WlanNetworkConnect_Init(WlanConnectStatusCallback);
    if (RETCODE_OK != retcode)
    {
        printf("WLANEnterpriseWPA2Connect : Error occurred initializing WLAN \r\n");
        return retcode;
    }
    /* Semaphore take to flush the existing queue events without timeout. Hence no need to consider the return value */
    (void) xSemaphoreTake(WlanEventSemaphore, 0U);
    printf("WLANEnterpriseWPA2Connect : WLAN Module initialization succeeded \r\n");

    WLANVersionLog();

    if (WLANSetup.IsHostPgmEnabled)
    {
        retcode = WLANHostPgm_Enable();
    }

    if (WLANSetup.IsStatic)
    {
        /* Configure settings to static IP */
        WlanNetworkConfig_IpSettings_T myIpSet;
        myIpSet.isDHCP = (uint8_t) WLANNWCNF_DHCP_DISABLED;
        myIpSet.ipV4 = sl_Htonl(WLANSetup.IpAddr);
        myIpSet.ipV4DnsServer = sl_Htonl(WLANSetup.DnsAddr);
        myIpSet.ipV4Gateway = sl_Htonl(WLANSetup.GwAddr);
        myIpSet.ipV4Mask = sl_Htonl(WLANSetup.Mask);

        retcode = WlanNetworkConfig_SetIpStatic(myIpSet);
        if (RETCODE_OK == retcode)
        {
            printf("WLANEnterpriseWPA2Connect : Static IP is set to : %u.%u.%u.%u\r\n",
                    (unsigned int) (WlanNetworkConfig_Ipv4Byte(myIpSet.ipV4, 3)),
                    (unsigned int) (WlanNetworkConfig_Ipv4Byte(myIpSet.ipV4, 2)),
                    (unsigned int) (WlanNetworkConfig_Ipv4Byte(myIpSet.ipV4, 1)),
                    (unsigned int) (WlanNetworkConfig_Ipv4Byte(myIpSet.ipV4, 0)));
        }
        else
        {
            printf("WLANEnterpriseWPA2Connect : Static IP configuration failed\r\n");
            return retcode;
        }
    }
    else
    {
        /* Semaphore take to flush the existing queue events without timeout. Hence no need to consider the return value */
        (void) xSemaphoreTake(NetworkConfigSemaphore, 0U);
        retcode = WlanNetworkConfig_SetIpDhcp(NetworkIpConfigCallbackFunc);
        if (RETCODE_OK != retcode)
        {
            printf("WLANEnterpriseWPA2Connect : Error in setting IP to DHCP \r\n");
            return retcode;
        }
    }

    /* disable server authentication */
    unsigned char pValues;
    pValues = 0; //0 - Disable the server authentication | 1 - Enable (this is the default)
    if (0U != sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, 19, 1, &pValues))
    {
        return RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_WLAN_SL_SET_FAILED);
    }

    /* Connect to the Wireless WPA2-Enterprise Network */
    printf("WLANEnterpriseWPA2Connect : Connecting to %s... (It may take upto 2 - 3 minutes)\r\n", connectSSID);

    retcode = WlanNetworkConnect_EnterpriseWPA(connectSSID, connectUsername, connectPassPhrase);
    if (RETCODE_OK != retcode)
    {
        printf("WLANEnterpriseWPA2Connect : Error occurred while connecting %s \r\n ", connectSSID);
        return retcode;
    }
    else
    {
        if (WLANSetup.IsStatic)
        {
            retcode = WlanEventSemaphoreHandle();
        }
        else
        {
            if (pdTRUE == xSemaphoreTake(NetworkConfigSemaphore, WLANNETWORK_EVENT_TIMEOUT))
            {
                retcode = WlanEventSemaphoreHandle();
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR);
            }
        }
        if (RETCODE_OK != retcode)
        {
            return retcode;
        }
    }

    retcode = WLANIpCredentialsValidateAndLog();
    if (RETCODE_OK != retcode)
    {
        printf("WLANEnterpriseWPA2Connect : Error in getting IP settings \r\n ");
        return retcode;
    }
    else
    {
        printf("WLANEnterpriseWPA2Connect : Connected to WPA network successfully. \r\n");
    }

    /* Get the MAC Address */
    memset(_macVal, UINT8_C(0), WLAN_MAC_ADDR_LENGTH);
    int8_t _status = sl_NetCfgGet((_u8) SL_MAC_ADDRESS_GET, NULL, &_macAddressLen, (uint8_t *) _macVal);

    if (WLANNETWORKCONFIG_FAILURE == _status)
    {
        printf("WLANEnterpriseWPA2Connect : sl_NetCfgGet Failed\r\n");
        return RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_WLAN_SETUP_ENTERPRISE_NW_CFG_GET_FAILED);
    }
    else
    {
        /* Store the MAC Address into the Global Variable */
        memset(WLANMacAddress, UINT8_C(0), strlen(WLANMacAddress));
        sprintf(WLANMacAddress, "%02X:%02X:%02X:%02X:%02X:%02X", _macVal[0], _macVal[1], _macVal[2], _macVal[3], _macVal[4], _macVal[5]);
        printf("WLANEnterpriseWPA2Connect : MAC address of the device: %s \r\n", WLANMacAddress);
    }
    return RETCODE_OK;
}

/**
 * @brief WLANPersonalWPA2Connect will setup Personal WPA2 connection
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T WLANPersonalWPA2Connect(void)
{
    Retcode_T retcode = RETCODE_OK;
    WlanNetworkConnect_SSID_T connectSSID = (WlanNetworkConnect_SSID_T) WLANSetup.SSID;
    WlanNetworkConnect_PassPhrase_T connectPassPhrase = (WlanNetworkConnect_PassPhrase_T) WLANSetup.Password;
    int retries = 1;
    int slReturn = -1;
    SlSecParams_t secParams;
    retcode = WlanNetworkConnect_Init(WlanConnectStatusCallback);
    if (RETCODE_OK != retcode)
    {
        return retcode;
    }
    /* Semaphore take to flush the existing queue events without timeout. Hence no need to consider the return value */
    (void) xSemaphoreTake(WlanEventSemaphore, 0U);

    WLANVersionLog();

    if (WLANSetup.IsStatic)
    {
        /* Configure settings to static IP */
        WlanNetworkConfig_IpSettings_T myIpSet;
        myIpSet.isDHCP = (uint8_t) WLANNWCNF_DHCP_DISABLED;
        myIpSet.ipV4 = sl_Htonl(WLANSetup.IpAddr);
        myIpSet.ipV4DnsServer = sl_Htonl(WLANSetup.DnsAddr);
        myIpSet.ipV4Gateway = sl_Htonl(WLANSetup.GwAddr);
        myIpSet.ipV4Mask = sl_Htonl(WLANSetup.Mask);

        retcode = WlanNetworkConfig_SetIpStatic(myIpSet);
        if (RETCODE_OK == retcode)
        {
            printf("WLANPersonalWPA2Connect : Static IP is set to : %u.%u.%u.%u\r\n",
                    (unsigned int) (WlanNetworkConfig_Ipv4Byte(myIpSet.ipV4, 3)),
                    (unsigned int) (WlanNetworkConfig_Ipv4Byte(myIpSet.ipV4, 2)),
                    (unsigned int) (WlanNetworkConfig_Ipv4Byte(myIpSet.ipV4, 1)),
                    (unsigned int) (WlanNetworkConfig_Ipv4Byte(myIpSet.ipV4, 0)));
        }
        else
        {
            printf("WLANPersonalWPA2Connect : Static IP configuration failed\r\n");
            return retcode;
        }
    }
    else
    {
        /* Semaphore take to flush the existing queue events without timeout. Hence no need to consider the return value */
        (void) xSemaphoreTake(NetworkConfigSemaphore, 0U);
        retcode = WlanNetworkConfig_SetIpDhcp(NetworkIpConfigCallbackFunc);
        if (RETCODE_OK != retcode)
        {
            printf("WLANPersonalWPA2Connect : Error in setting IP to DHCP \r\n");
            return retcode;
        }
    }

    retcode = WlanNetworkConnect_PersonalWPA(connectSSID, connectPassPhrase);
    while (((uint32_t) RETCODE_ERROR_IP_NOT_ACQ == Retcode_GetCode(retcode) ||
            (uint32_t) RETCODE_ERROR_WRONG_PASSWORD == Retcode_GetCode(retcode))
            && WLAN_CONNECT_WPA_RETRY_MAX >= retries)
    {
        /* retry, sometimes RETCODE_ERROR_IP_NOT_ACQ or RETCODE_ERROR_WRONG_PASSWORD are reported */
        retcode = WlanNetworkConnect_PersonalWPA(connectSSID, connectPassPhrase);
        ++retries;
    }
    if (RETCODE_OK != retcode)
    {
        printf("WLANPersonalWPA2Connect : Error in WlanNetworkConnect_WPA \r\n");
        return retcode;
    }
    else
    {
        if (WLANSetup.IsStatic)
        {
            retcode = WlanEventSemaphoreHandle();
        }
        else
        {
            if (pdTRUE == xSemaphoreTake(NetworkConfigSemaphore, WLANNETWORK_EVENT_TIMEOUT))
            {
                retcode = WlanEventSemaphoreHandle();
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR);
            }
        }
        if (RETCODE_OK != retcode)
        {
            return retcode;
        }
    }

    printf("Connected to WPA network successfully\r\n");

    /* Failing on one sl_* function, we still proceed since WLANIpCredentialsValidateAndLog will report failures. */
    slReturn = sl_WlanProfileDel(0xFF);
    if (SL_RET_CODE_OK > slReturn)
    {
        printf("WLANPersonalWPA2Connect : failed to delete all profiles\r\n");
    }
    /* Set network parameters */
    secParams.Key = (signed char *) connectPassPhrase;
    secParams.KeyLen = strlen((const char*) connectPassPhrase);
    secParams.Type = 0x02; /* WLI_SECURITY_TYPE_WPA; */
    slReturn = sl_WlanProfileAdd((signed char *) connectSSID, strlen((const char*) connectSSID), 0, &secParams, 0, 1, 0);
    if (SL_RET_CODE_OK > slReturn)
    {
        printf("WLANPersonalWPA2Connect : failed to add profile\r\n");
    }
    slReturn = sl_WlanPolicySet(SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(0, 0, 0, 0, 0), NULL, 0); /* reset */
    if (SL_RET_CODE_OK > slReturn)
    {
        printf("WLANPersonalWPA2Connect : failed to reset connection policy\r\n");
    }
    slReturn = sl_WlanPolicySet(SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(1, 1, 0, 0, 0), NULL, 0); /* autoreconnect, fast */
    if (SL_RET_CODE_OK > slReturn)
    {
        printf("WLANPersonalWPA2Connect : failed to set connection policy\r\n");
    }
    slReturn = sl_WlanRxStatStart();
    if (SL_RET_CODE_OK > slReturn)
    {
        printf("WLANPersonalWPA2Connect : failed to start Rx Statistic\r\n");
    }

    retcode = WLANIpCredentialsValidateAndLog();
    if (RETCODE_OK != retcode)
    {
        printf("WLANPersonalWPA2Connect : Error in getting IP settings \r\n");
    }
    else
    {
        printf("WLANPersonalWPA2Connect : Connected to WPA network successfully. \r\n");
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T WLAN_Setup(WLAN_Setup_T * setup)
{
    Retcode_T retcode = RETCODE_OK;
    if (NULL == setup)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        if (setup->IsHostPgmEnabled)
        {
            retcode = WLANHostPgm_Setup();
        }
        if (RETCODE_OK == retcode)
        {
            WLANSetup = *setup;
            NetworkConfigSemaphore = xSemaphoreCreateBinary();
            if (NULL == NetworkConfigSemaphore)
            {
                printf("Failed to create Semaphore \r\n");
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR);
            }
            else
            {
                WlanEventSemaphore = xSemaphoreCreateBinary();
                if (NULL == WlanEventSemaphore)
                {
                    vSemaphoreDelete(NetworkConfigSemaphore);
                    printf("Failed to create Semaphore \r\n");
                    retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR);
                }
            }
        }
    }
    return (retcode);
}

/** Refer interface header for description */
Retcode_T WLAN_Enable(void)
{
    Retcode_T retcode = RETCODE_OK;
    if (WLANSetup.IsEnterprise)
    {
        printf("WLAN_Enable : Preparing for Enterprise WPA2 connection\r\n");
        retcode = WLANEnterpriseWPA2Connect();
    }
    else
    {
        printf("WLAN_Enable : Preparing for Personal WPA2 connection\r\n");
        retcode = WLANPersonalWPA2Connect();
    }
    return (retcode);
}

/** Refer interface header for description */
Retcode_T WLAN_Disable(void)
{
    Retcode_T retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_FAILURE);
    int slReturn = -1;

    slReturn = sl_WlanPolicySet(SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(0, 0, 0, 0, 0), NULL, 0); /* reset */
    if (SL_RET_CODE_OK > slReturn)
    {
        printf("WLAN_Disable : failed to reset connection policy\r\n");
    }

    if (WLANNWCT_IPSTATUS_DISCT_NAQRD
            != WlanNetworkConnect_GetIpStatus()
            && (WLANNWCT_IPSTATUS_DISCT_AQRD
                    != WlanNetworkConnect_GetIpStatus()))
    {
        /* Semaphore take to flush the existing queue events without timeout. Hence no need to consider the return value */
        (void) xSemaphoreTake(WlanEventSemaphore, 0U);
        retcode = WlanNetworkConnect_Disconnect();
        if (RETCODE_OK == retcode)
        {
            if (pdTRUE == xSemaphoreTake(WlanEventSemaphore, WLANNETWORK_EVENT_TIMEOUT))
            {
                if ((WLANNWCT_STATUS_DISCONNECTED == WlanNetworkConnect_GetStatus()))
                {
                    retcode = RETCODE_OK;
                }
                else
                {
                    retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_WLAN_DISCONNECT_FAILED);
                }
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR);
            }
        }

    }
    else
    {
        if (WLANNWCT_STATUS_DISCONNECTED == WlanNetworkConnect_GetStatus())
        {
            retcode = RETCODE_OK;
        }
    }
    return (retcode);
}

/** Refer interface header for description */
Retcode_T WLAN_IsIpChanged(bool * ipChangeStatus, uint32_t * newIp)
{
    Retcode_T retcode = RETCODE_OK;
    WlanNetworkConfig_IpSettings_T myIpSettings;
    uint32_t* existingIpAddress = GetMyIpAddr();
    uint32_t newIpAddress;

    *ipChangeStatus = false;
    *newIp = 0UL;

    retcode = WlanNetworkConfig_GetIpSettings(&myIpSettings);
    if (RETCODE_OK == retcode)
    {
        newIpAddress = ntohl(myIpSettings.ipV4);
        if (newIpAddress != *existingIpAddress)
        {
            *existingIpAddress = newIpAddress;
            *newIp = newIpAddress;
            *ipChangeStatus = true;
        }
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T WLAN_ConvertIPAddressToString(uint32_t ipAddress, char * string)
{
    Retcode_T retcode = RETCODE_OK;

    if (ConvertIpAddrToString(&ipAddress, string) < 0)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_FAILURE);
    }
    return retcode;
}

#endif /* XDK_CONNECTIVITY_WLAN */

#if (XDK_CONNECTIVITY_WLAN || XDK_UTILITY_STORAGE)

static void WlanConnectStatusCallback(WlanNetworkConnect_Status_T status)
{
    BCDS_UNUSED(status);
    if (NULL != WlanEventSemaphore)
    {
        (void) xSemaphoreGive(WlanEventSemaphore);
    }
}
/* callback function for Network IP config API's*/
static void NetworkIpConfigCallbackFunc(WlanNetworkConfig_IpStatus_T ipStatus)
{
    BCDS_UNUSED(ipStatus);
    if (NULL != NetworkConfigSemaphore)
    {
        (void) xSemaphoreGive(NetworkConfigSemaphore);
    }
}

/** Refer private header for description */
WlanNetworkConnect_Callback_T WLAN_GetConnectionCallbackHandle(void)
{
    return WlanConnectStatusCallback;
}

Retcode_T WLAN_Reconnect(void)
{
    Retcode_T retcode = RETCODE_OK;
    WlanNetworkConnect_ScanInterval_T scanInterval = 5;
    WlanNetworkConnect_ScanList_T scanList;
    WlanNetworkConnect_IpStatus_T nwStatus;
    bool networkStatusFlag = false;

    nwStatus = WlanNetworkConnect_GetIpStatus();

    if (WLANNWCT_IPSTATUS_CT_AQRD != nwStatus)
    {
        printf("Checking for network availability and trying to connect again\n\r");
        retcode = WlanNetworkConnect_ScanNetworks(scanInterval, &scanList);

        if (RETCODE_OK == retcode)
        {
            for (int i = 0U; i < WLANNWCT_MAX_SCAN_INFO_BUF; i++)
            {
                if (0U == strcmp((char *) WLANSetup.SSID, (char *) scanList.ScanData[i].Ssid))
                {
                    networkStatusFlag = true;
                    printf("Network with SSID  %s is available\n\r", WLANSetup.SSID);
                    retcode = WLAN_Enable();
                    if (RETCODE_OK != retcode)
                    {
                        printf("Not able to connect to the network\n\r");
                    }
                    break;
                }
                else
                {
                    networkStatusFlag = false;
                }
            }
            if (false == networkStatusFlag)
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_WLAN_NETWORK_NOT_AVAILABLE);
                printf("Network with SSID  %s is not available\n\r", WLANSetup.SSID);
            }
        }
        else if ((uint32_t) RETCODE_NO_NW_AVAILABLE == Retcode_GetCode(retcode))
        {
            printf("Network not available \n\r");
        }
    }
    else
    {
        printf("Network Connection is active \n\r");
    }
    return retcode;

}
#endif /* XDK_CONNECTIVITY_WLAN */

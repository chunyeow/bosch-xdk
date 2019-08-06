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
//lint -esym(956,*) /* Suppressing "Non const, non volatile static or external variable" lint warning*/
/* module includes ********************************************************** */

/* This is used to define the correct module ID for generation of module
 * error codes
 */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_WLAN_CONNECT
/* own header files */
#include "BCDS_WlanConnect.h"
#include "BCDS_WlanDriver.h"
#include "BCDS_NetworkConfig.h"
#include "BCDS_WlanNetworkConnect.h"

/* additional interface header files */
#include "BCDS_BSP_Board.h"
#include "simplelink.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* constant definitions ***************************************************** */
/* local type and macro definitions */
#define WLANCONNECT_SCAN_TABLE_SIZE         UINT8_C(20)
#define WLANCONNECT_SCAN_ENABLE             UINT8_C(1)
#define WLANCONNECT_SCAN_DISABLE            UINT8_C(0)
#define WLANCONNECT_FAILURE                 INT16_C(-1) /**< Macro for defining Failure*/
#define WLANCONNECT_SUCCESS                 UINT16_C(0) /**< Macro for defining SUCCESS*/
#define WLANCONNECT_ZERO                    UINT8_C(0) /**< Macro for defining 0*/
#define WLANCONNECT_ONE                     UINT8_C(1) /**< Macro for defining 1*/
#define WLANCONNECT_TIMEOUT_VAL             UINT8_C(0xFF) /**< Macro for timeout value*/
#define WLANCONNECT_ALL_PROFILES            UINT8_C(0xFF) /**< Macro for deleting all profiles */
#define WLANCONNECT_MAX_BUFFER              UINT8_C(10) /**< Macro for Simple Link status buffer */
#define WLANCONNECT_NO_OF_ENTRIES           UINT8_C(5) /**< Macro for scan function number of entries*/
#define WLANCONNECT_ONE_SEC_DELAY           (portTickType)(1000) /**< Macro for 1 second delay*/
#define WLANCONNECT_MAX_TRIES               UINT8_C(15) /**< Macro for Simple Link status buffer */
#define WLANCONNECT_TIMER_TICKS             UINT8_C(1000) /**< Macro for Simple Link status buffer */
#define TIMER_AUTORELOAD_ON                 UINT8_C(1)  /**< Auto reload of timer is disabled*/
#define TIMERBLOCKTIME                      UINT32_MAX     /**< Macro used to define blocktime of a timer*/

/* local variables ********************************************************** */
static volatile WlanConnect_Callback_T WlanConnectCallback = NULL; /**< Variable for storing the connection callback*/
static volatile WlanConnect_DisconnectCallback_T WlanDisconnectCallback = NULL; /**< Variable for storing the disconnection callback from disconnect function*/
static xTimerHandle ConnectTimerHandle = NULL;
static xTimerHandle DisConnectTimerHandle = NULL;
static Sl_WlanNetworkEntry_t NetEntries[WLANCONNECT_SCAN_TABLE_SIZE]; /**< Variable for storing the scanned network entries */
static volatile WlanConnect_Status_T WlanConnect_Status; /**< Flag variable for WLI connect status*/
static volatile uint8_t WlanConnect_InitStatus; /**< Flag variable for Wlan init status*/

/* global variables ********************************************************* */

/* inline functions ********************************************************* */

/* local functions ********************************************************** */

static void WlanConnectcallBack(WlanConnect_Status_T connectStatus)
{
    WlanConnect_Status = connectStatus;
}

static void disConnectNonBlockingCall(xTimerHandle xTimer)
{
    BCDS_UNUSED(xTimer);
    Retcode_T retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_RTOS_QUEUE_ERROR);

    static uint8_t Counter = UINT8_C(0);
    Counter++;
    WlanConnect_Status = WlanConnect_GetStatus();
    if (WLAN_DISCONNECTED == WlanConnect_Status)
    {
        WlanDisconnectCallback(WLAN_DISCONNECTED);
        Counter = UINT8_C(0);
        if (xTimerDelete(DisConnectTimerHandle, TIMERBLOCKTIME) != pdTRUE)
        {
            Retcode_RaiseError(retVal);
        }
        return;
    }
    else if ((WLAN_DISCONNECTED != WlanConnect_Status) && (Counter >= WLANCONNECT_MAX_TRIES))
    {
        /* Not connected */
        WlanDisconnectCallback(WLAN_DISCONNECT_ERROR);
    }
    if ((Counter >= WLANCONNECT_MAX_TRIES))
    {
        Counter = UINT8_C(0);
        if (xTimerDelete(DisConnectTimerHandle, TIMERBLOCKTIME) != pdTRUE)
        {
            Retcode_RaiseError(retVal);
        }
    }
}
/** This is a timer callback called from connect API's when connection needs to be non-blocking */
static void connectNonBlockingCall(xTimerHandle xTimer)
{
    BCDS_UNUSED(xTimer);
    Retcode_T retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_RTOS_QUEUE_ERROR);

    static uint8_t Counter = UINT8_C(0);
    WlanConnect_Status = WlanConnect_GetStatus();
    Counter++;
    if ((WLAN_CONNECTED == WlanConnect_Status)
            && (NETWORKCONFIG_IPV4_ACQUIRED == NetworkConfig_GetIpStatus()))
    {
        /* connected and IP acquired*/
        WlanConnectCallback(WLAN_CONNECTED);
        Counter = UINT8_C(0);
        if (xTimerDelete(ConnectTimerHandle, TIMERBLOCKTIME) != pdTRUE)
        {
            Retcode_RaiseError(retVal);
        }
        return;
    }
    else if ((WLAN_DISCONNECTED == WlanConnect_Status) && (Counter >= WLANCONNECT_MAX_TRIES))
    {
        /* Not connected */
        WlanConnectCallback(WLAN_CONNECTION_ERROR);
    }
    else if ((WLAN_CONNECTION_PWD_ERROR == WlanConnect_Status) && (Counter >= WLANCONNECT_MAX_TRIES))
    {
        /*Password error*/
        WlanConnectCallback(WLAN_CONNECTION_PWD_ERROR);
    }
    else if ((WLAN_CONNECTED == WlanConnect_Status)
            && (NETWORKCONFIG_IP_NOT_ACQUIRED == NetworkConfig_GetIpStatus()) && (Counter >= WLANCONNECT_MAX_TRIES))
    {
        /* connected but IP not acquired*/
        WlanConnectCallback(WLAN_CONNECTION_ERROR);
    }
    if ((Counter >= WLANCONNECT_MAX_TRIES))
    {
        Counter = UINT8_C(0);
        if (xTimerDelete(ConnectTimerHandle, TIMERBLOCKTIME) != pdTRUE)
        {
            Retcode_RaiseError(retVal);
        }
    }
}

/** This function is called by connect API's when connection needs to be blocking */
static Retcode_T connectBlockingCall(void)
{
    Retcode_T retVal = (Retcode_T) RETCODE_OK;
    uint8_t Counter = UINT8_C(0);

    while ((WLAN_DISCONNECTED == WlanConnect_Status)
            || (NETWORKCONFIG_IP_NOT_ACQUIRED == NetworkConfig_GetIpStatus()))

    {
        /* Timeout logic added to avoid blocking */
        vTaskDelay(WLANCONNECT_ONE_SEC_DELAY / portTICK_RATE_MS);
        Counter++;
        WlanConnect_Status = WlanConnect_GetStatus();
        /* After maximum retry it will come out of this loop*/
        if (WLANCONNECT_MAX_TRIES <= Counter)
        {
            break;
        }
    }
    if ((WLAN_CONNECTED == WlanConnect_Status) && (NETWORKCONFIG_IPV4_ACQUIRED == NetworkConfig_GetIpStatus()))
    {
        /* connected and IP acquired*/
        retVal = RETCODE_OK;
    }
    else if (WLAN_DISCONNECTED == WlanConnect_Status)
    {
        /* not connected */
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_CONNECTION_ERROR);
    }
    else if (WLAN_CONNECTION_PWD_ERROR == WlanConnect_Status)
    {
        /*Password error */
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_ERROR_WRONG_PASSWORD);
    }
    else
    {
        /* connected but IP not acquired*/
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_ERROR_IP_NOT_ACQ);
    }

    return retVal;
}

/**
 * @brief   Function that retrieves time stamp. Currently retrieving
 * @param   time_ptr
 *          Pointer containing time in seconds.
 * @return  returnClockStatus
 *          Return status for the SystemClock_getTime function
 */
static Retcode_T SystemClock_getTime(uint32_t *time_ptr)
{
    /* function should be adapted in order to provide real time stamps */
    Retcode_T returnClockStatus = (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_TIME_NULL));
    *time_ptr = (((xTaskGetTickCount()) / portTICK_RATE_MS) * 1000);
    if (WLANCONNECT_ZERO != *time_ptr)
    {
        returnClockStatus = RETCODE_OK;
    }
    return (returnClockStatus);
}

/* global functions ********************************************************* */

WlanConnect_Status_T WlanConnect_GetStatus(void)
{
    return (WlanConnect_Status_T)WlanNetworkConnect_GetStatus();
}

uint8_t WlanConnect_GetInitStatus(void)
{
    return WlanNetworkConnect_GetInitStatus();
}

Retcode_T WlanConnect_Init(void)
{
    return (WlanNetworkConnect_Init(WlanConnectcallBack));
}

Retcode_T WlanConnect_DeInit(void)
{
    return (WlanNetworkConnect_DeInit());
}

static Retcode_T NonBlockingTimer(void)
{
    Retcode_T retVal = (Retcode_T) RETCODE_OK;
    ConnectTimerHandle = xTimerCreate((const char * const ) "connect non-blocking call",
            (TickType_t) WLANCONNECT_TIMER_TICKS, TIMER_AUTORELOAD_ON, NULL, connectNonBlockingCall);
    if (NULL != ConnectTimerHandle)
    {
        if (xTimerStart(ConnectTimerHandle, TIMERBLOCKTIME) != pdTRUE)
        {
            retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_TIMERSTART_FAILURE);
        }
        else
        {
            retVal = RETCODE_OK;
        }
    }
    else
    {
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_TIMERCREATE_FAILURE);
    }
    return retVal;
}

Retcode_T WlanConnect_Open(WlanConnect_SSID_T connectSSID,
        WlanConnect_Callback_T connectCallback)
{
    WlanConnect_InitStatus = WlanConnect_GetInitStatus();
    if (WLANCONNECT_NOT_INITIALZED == WlanConnect_InitStatus)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_UNINITIALIZED));
    }
    if (NULL == connectSSID)
    {
        return (RETCODE(RETCODE_SEVERITY_FATAL, (uint32_t ) RETCODE_NULL_POINTER));
    }
    /* Local variables */
    Retcode_T retVal = (Retcode_T) RETCODE_OK;
    int16_t slStatus = WLANCONNECT_SUCCESS;
    SlSecParams_t secParams;

    /* Set network parameters */
    secParams.KeyLen = WLANCONNECT_ZERO;
    secParams.Type = SL_SEC_TYPE_OPEN;

    /* Store callback function */
    WlanConnectCallback = connectCallback;

    /* Call the connect function */
    slStatus = sl_WlanConnect(connectSSID, strlen(((char*) connectSSID)), NULL, &secParams, NULL);

    /* Determine return value*/
    if (WLANCONNECT_SUCCESS == slStatus)
    {
        if (NULL == connectCallback)
        {
            retVal = connectBlockingCall();
        }
        else
        {
            retVal = NonBlockingTimer();
        }
    }
    else
    {
        /* Simple Link function encountered an error.*/
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_SIMPLELINK_ERROR);
    }

    /* return API status*/
    return (retVal);
}

Retcode_T WlanConnect_WEP_Open(WlanConnect_SSID_T connectSSID,
        WlanConnect_PassPhrase_T connectPass, uint8_t passPhraseLength,
        WlanConnect_Callback_T connectCallback)
{
    WlanConnect_InitStatus = WlanConnect_GetInitStatus();
    if (WLANCONNECT_NOT_INITIALZED == WlanConnect_InitStatus)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_UNINITIALIZED));
    }
    if (UINT8_C(0) == passPhraseLength)
    {
        return (RETCODE(RETCODE_SEVERITY_FATAL, (uint32_t ) RETCODE_INVALID_PARAM));

    }
    if ((NULL == connectSSID) || (NULL == connectPass))
    {
        return (RETCODE(RETCODE_SEVERITY_FATAL, (uint32_t ) RETCODE_NULL_POINTER));
    }
    /* Local variables */
    Retcode_T retVal = (Retcode_T) RETCODE_OK;
    int16_t slStatus = WLANCONNECT_SUCCESS;
    SlSecParams_t secParams;

    /* Set network parameters */
    secParams.Key = connectPass;
    secParams.KeyLen = passPhraseLength;
    secParams.Type = SL_SEC_TYPE_WEP;

    /* Store callback function */
    WlanConnectCallback = connectCallback;

    /* Call the connect function */
    slStatus = sl_WlanConnect(connectSSID, strlen(((const char*) connectSSID)), NULL, &secParams, NULL);

    /* Determine return value*/
    if (WLANCONNECT_SUCCESS == slStatus)
    {
        if (NULL == connectCallback)
        {
            retVal = connectBlockingCall();
        }
        else
        {
            retVal = NonBlockingTimer();
        }
    }
    else
    {
        /* Simple Link function encountered an error.*/
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_SIMPLELINK_ERROR);
    }

    return (retVal);
}

Retcode_T WlanConnect_WPA(WlanConnect_SSID_T connectSSID,
        WlanConnect_PassPhrase_T connectPass,
        WlanConnect_Callback_T connectCallback)
{
    WlanConnect_InitStatus = WlanConnect_GetInitStatus();
    if (WLANCONNECT_NOT_INITIALZED == WlanConnect_InitStatus)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_UNINITIALIZED));
    }
    if ((NULL == connectSSID) || (NULL == connectPass))
    {
        return (RETCODE(RETCODE_SEVERITY_FATAL, (uint32_t ) RETCODE_NULL_POINTER));
    }
    /* Local variables */
    Retcode_T retVal = (Retcode_T) RETCODE_OK;
    int16_t slStatus = WLANCONNECT_SUCCESS;
    SlSecParams_t secParams;

    /* Set network parameters */
    secParams.Key = connectPass;
    secParams.KeyLen = strlen((const char*) connectPass);
    secParams.Type = SL_SEC_TYPE_WPA;

    /* Store callback function */
    WlanConnectCallback = connectCallback;

    /* Call the connect function */
    slStatus = sl_WlanConnect(connectSSID, strlen(((char*) connectSSID)), NULL, &secParams, NULL);

    /* Determine return value*/
    if (WLANCONNECT_SUCCESS == slStatus)
    {
        if (NULL == connectCallback)
        {
            retVal = connectBlockingCall();
        }
        else
        {
            retVal = NonBlockingTimer();
        }
    }
    else
    {
        /* Simple Link function encountered an error.*/
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_SIMPLELINK_ERROR);
    }

    return (retVal);
}

Retcode_T WlanConnect_WPS_PBC(WlanConnect_Callback_T connectCallback)
{
    WlanConnect_InitStatus = WlanConnect_GetInitStatus();
    if (WLANCONNECT_NOT_INITIALZED == WlanConnect_InitStatus)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_UNINITIALIZED));
    }
    /* Local variables */
    Retcode_T retVal = (Retcode_T) RETCODE_OK;
    int16_t slStatus = WLANCONNECT_SUCCESS;
    SlSecParams_t secParams;

    secParams.Key = NULL;
    secParams.KeyLen = WLANCONNECT_ZERO;
    secParams.Type = SL_SEC_TYPE_WPS_PBC;

    /* Store callback function */
    WlanConnectCallback = connectCallback;

    /* Call the connect function */
    slStatus = sl_WlanConnect((signed char*) WLANCONNECT_DUMMY_SSID, strlen(((char*) WLANCONNECT_DUMMY_SSID)), NULL, &secParams, NULL);

    /* Determine return value*/
    if (WLANCONNECT_SUCCESS == slStatus)
    {
        if (NULL == connectCallback)
        {
            retVal = connectBlockingCall();
        }
        else
        {
            retVal = NonBlockingTimer();
        }
    }
    else
    {
        /* Simple Link function encountered an error.*/
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_SIMPLELINK_ERROR);
    }

    /* return API status*/
    return (retVal);
}

Retcode_T WlanConnect_WPS_PIN(WlanConnect_Callback_T connectCallback)
{
    WlanConnect_InitStatus = WlanConnect_GetInitStatus();
    if (WLANCONNECT_NOT_INITIALZED == WlanConnect_InitStatus)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_UNINITIALIZED));
    }
    /* Local variables */
    Retcode_T retVal = (Retcode_T) RETCODE_OK;
    int16_t slStatus = WLANCONNECT_SUCCESS;
    SlSecParams_t secParams;

    secParams.Key = (signed char*) WLANCONNECT_ENROLLEE_PIN;
    secParams.KeyLen = strlen((const char*) WLANCONNECT_ENROLLEE_PIN);
    secParams.Type = SL_SEC_TYPE_WPS_PIN;

    /* Store callback function */
    WlanConnectCallback = connectCallback;

    /* Call the connect function */
    slStatus = sl_WlanConnect((signed char*) WLANCONNECT_DUMMY_SSID, strlen(((char*) WLANCONNECT_DUMMY_SSID)), NULL, &secParams, NULL);

    /* Determine return value*/
    if (WLANCONNECT_SUCCESS == slStatus)
    {
        if (NULL == connectCallback)
        {
            retVal = connectBlockingCall();
        }
        else
        {
            retVal = NonBlockingTimer();
        }
    }
    else
    {
        /* Simple Link function encountered an error.*/
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_SIMPLELINK_ERROR);
    }

    /* return API status*/
    return (retVal);

}

Retcode_T WlanConnect_EnterpriseWPA(WlanConnect_SSID_T connectSSID, WlanConnect_Username_T connectUsername, WlanConnect_PassPhrase_T connectPass, WlanConnect_Callback_T connectCallback)
{
    WlanConnect_InitStatus = WlanConnect_GetInitStatus();
    if (WLANCONNECT_NOT_INITIALZED == WlanConnect_InitStatus)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_UNINITIALIZED));
    }
    if ((NULL == connectSSID) || (NULL == connectUsername) || (NULL == connectPass))
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER));
    }

    /* Local variables */
    Retcode_T retVal = RETCODE_OK;
    int16_t slStatus = WLANCONNECT_SUCCESS;
    SlSecParams_t secParams;
    SlSecParamsExt_t eapParams;

    /* Set network parameters */
    secParams.Key = connectPass;
    secParams.KeyLen = strlen((const char *) connectPass);
    secParams.Type = SL_SEC_TYPE_WPA_ENT;

    /* Set network EAP parameters */
    eapParams.User = (signed char *) connectUsername;
    eapParams.UserLen = strlen((const char *) connectUsername);
    eapParams.AnonUser = (signed char*) connectUsername;
    eapParams.AnonUserLen = strlen((const char *) connectUsername);
    eapParams.CertIndex = UINT8_C(0);
    eapParams.EapMethod = SL_ENT_EAP_METHOD_PEAP0_MSCHAPv2;

    /* Store callback function */
    WlanConnectCallback = connectCallback;

    /* Call the connect function */
    slStatus = sl_WlanConnect((signed char *) connectSSID, strlen(((char *) connectSSID)),
            (unsigned char *) WLANCONNECT_NOT_INITIALZED, &secParams, &eapParams);

    /* Determine return value*/
    if (WLANCONNECT_SUCCESS == slStatus)
    {
        if (NULL == connectCallback)
        {
            retVal = connectBlockingCall();
        }
        else
        {
            retVal = NonBlockingTimer();
        }
    }
    else
    {
        /* Simple Link function encountered an error.*/
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SIMPLELINK_ERROR);
    }

    return (retVal);
}

Retcode_T WlanConnect_DeleteAllProfiles(void)
{
    WlanConnect_InitStatus = WlanConnect_GetInitStatus();
    if (WLANCONNECT_NOT_INITIALZED == WlanConnect_InitStatus)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_UNINITIALIZED));
    }
    Retcode_T retVal = (Retcode_T) RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_UNEXPECTED_BEHAVIOR);
    int16_t slStatus = WLANCONNECT_SUCCESS;

    /* Delete all the WLAN saved profiles */
    slStatus = sl_WlanProfileDel(WLANCONNECT_ALL_PROFILES);

    if (WLANCONNECT_SUCCESS == slStatus)
    {
        retVal = RETCODE_OK;
    }
    else if (WLANCONNECT_FAILURE == slStatus)
    {
        retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_SIMPLELINK_ERROR);
    }

    return (retVal);
}

Retcode_T WlanConnect_Disconnect(WlanConnect_DisconnectCallback_T disconnectCallback)
{
    WlanConnect_InitStatus = WlanConnect_GetInitStatus();
    if (WLANCONNECT_NOT_INITIALZED == WlanConnect_InitStatus)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_UNINITIALIZED));
    }
    /* Local variables */
    Retcode_T retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_UNEXPECTED_BEHAVIOR);
    int16_t slStatus = WLANCONNECT_SUCCESS;

    WlanDisconnectCallback = disconnectCallback;
    /* Disconnect from WLAN Network */
    slStatus = sl_WlanDisconnect();
    if (WLANCONNECT_SUCCESS == slStatus)
    {
        if (NULL == disconnectCallback)
        {
            uint8_t Counter = UINT8_C(0);
            while ((WLAN_CONNECTED == WlanConnect_Status) || (WLAN_CONNECTION_PWD_ERROR == WlanConnect_Status))
            {
                /* Timeout logic added to avoid blocking */
                vTaskDelay(WLANCONNECT_ONE_SEC_DELAY / portTICK_RATE_MS);
                Counter++;
                /* After maximum retry it will come out of this loop*/
                if (WLANCONNECT_MAX_TRIES <= Counter)
                {
                    break;
                }
            }
            if (WLAN_DISCONNECTED == WlanConnect_Status)
            {
                /* disconnected successfully*/
                retVal = RETCODE_OK;
            }
            if (WLAN_DISCONNECTED != WlanConnect_Status)
            {
                /* Not disconnected */
                retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_DISCONNECT_ERROR);
            }
        }
        else
        {
            DisConnectTimerHandle = xTimerCreate((const char * const ) "Disconnect non-blocking call",
                    (TickType_t) WLANCONNECT_TIMER_TICKS, TIMER_AUTORELOAD_ON, NULL, disConnectNonBlockingCall);
            if (NULL != DisConnectTimerHandle)
            {
                if (xTimerStart(DisConnectTimerHandle, TIMERBLOCKTIME) != pdTRUE)
                {
                    retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_TIMERSTART_FAILURE);
                }
                else
                {
                    return (RETCODE_OK);
                }
            }
            else
            {
                retVal = RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_TIMERCREATE_FAILURE);
            }
        }
    }
    else
    {
        if (NULL != disconnectCallback)
        {
            disconnectCallback(WLAN_DISCONNECTED);
        }
        retVal = RETCODE_OK;
    }

    return (retVal);
}

WlanConnect_CurrentNwStatus_T WlanConnect_GetCurrentNwStatus(void)
{
    WlanConnect_CurrentNwStatus_T retVal = DISCONNECTED_IP_NOT_ACQUIRED;

    if ((NETWORKCONFIG_IPV4_ACQUIRED == NetworkConfig_GetIpStatus())
            && (WLAN_CONNECTED == WlanConnect_Status))
    {
        retVal = CONNECTED_AND_IPV4_ACQUIRED;
    }
    else if ((NETWORKCONFIG_IP_NOT_ACQUIRED == NetworkConfig_GetIpStatus())
            && (WLAN_CONNECTED == WlanConnect_Status))
    {
        retVal = CONNECTED_AND_IP_NOT_ACQUIRED;
    }
    else if ((NETWORKCONFIG_IPV4_ACQUIRED == NetworkConfig_GetIpStatus())
            && (WLAN_DISCONNECTED == WlanConnect_Status))
    {
        retVal = DISCONNECTED_AND_IPV4_ACQUIRED;
    }
    else
    {
        retVal = DISCONNECTED_IP_NOT_ACQUIRED;
    }

    return (retVal);
}

Retcode_T WlanConnect_ScanNetworks(WlanConnect_ScanInterval_T scanInterval,
        WlanConnect_ScanList_T* scanList)
{
    WlanConnect_InitStatus = WlanConnect_GetInitStatus();
    if (WLANCONNECT_NOT_INITIALZED == WlanConnect_InitStatus)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_UNINITIALIZED));
    }
    if (NULL == scanList)
    {
        return (RETCODE(RETCODE_SEVERITY_FATAL, (uint32_t ) RETCODE_NULL_POINTER));
    }
    /* local variables*/
    int16_t slStatus = WLANCONNECT_SUCCESS;
    Retcode_T retVal = (Retcode_T) RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_UNEXPECTED_BEHAVIOR);
    uint16_t runningIdx = WLANCONNECT_ZERO;
    uint32_t numOfEntries = WLANCONNECT_NO_OF_ENTRIES;
    uint8_t copyflag;
    uint8_t listPosition;

    /* initialize scan structures */
    memset(NetEntries, 0, sizeof(NetEntries));
    scanList->NumOfScanEntries = WLANCONNECT_ZERO;
    scanList->TimeStamp = WLANCONNECT_ZERO;
    memset(scanList->ScanData, 0, sizeof(scanList->ScanData));

    /* make sure the connection policy is not set (so no scan is run in the background) */
    slStatus = sl_WlanPolicySet(SL_POLICY_CONNECTION,
            SL_CONNECTION_POLICY(WLANCONNECT_ZERO, WLANCONNECT_ZERO, WLANCONNECT_ZERO, WLANCONNECT_ZERO,
                    WLANCONNECT_ZERO), NULL, WLANCONNECT_ZERO);
    if (WLANCONNECT_SUCCESS == slStatus)
    {
        /* start scanning */
        slStatus = sl_WlanPolicySet(SL_POLICY_SCAN, WLANCONNECT_SCAN_ENABLE,
                (_u8*) &scanInterval, sizeof(scanInterval));
    }
    if (WLANCONNECT_SUCCESS == slStatus)
    {
        /* delay 1 second to make sure that scan is started */
        vTaskDelay(WLANCONNECT_ONE_SEC_DELAY / portTICK_RATE_MS);

        /* fill 20 network entries 2 by 2*/
        do
        {
            slStatus = sl_WlanGetNetworkList(runningIdx, numOfEntries,
                    &NetEntries[runningIdx]);
            runningIdx += slStatus;
            /* check status */
            if (WLANCONNECT_FAILURE == slStatus)
            {
                return (RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_SIMPLELINK_ERROR));
            }
        } while ((slStatus == (int16_t) numOfEntries)
                && (runningIdx < WLANCONNECT_SCAN_TABLE_SIZE));

        /* disable scan */
        slStatus = sl_WlanPolicySet(SL_POLICY_SCAN, WLANCONNECT_SCAN_DISABLE, NULL,
        WLANCONNECT_ZERO);
        /* check status */
        if (WLANCONNECT_FAILURE == slStatus)
        {
            return (RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_SIMPLELINK_ERROR));
        }

        if (WLANCONNECT_SUCCESS == slStatus)
        {
            /* first element of the list is unique, therefore it must be copied*/
            copyflag = WLANCONNECT_ONE;
        }
        /* initialize scan data list position --> when repeating element subtract from network list*/
        listPosition = WLANCONNECT_ZERO;

        /* fill user network entries*/
        for (int8_t i = WLANCONNECT_ZERO; i < WLANCONNECT_SCAN_TABLE_SIZE; i++)
        {
            /* Search for duplicate SISD */
            for (int8_t sCount = i; sCount > WLANCONNECT_ZERO; sCount--)
            {
                if (WLANCONNECT_ZERO == memcmp(NetEntries[i].ssid,
                        NetEntries[sCount - 1].ssid,
                        sizeof(NetEntries[i].ssid)))
                {
                    /* the compared SSID are equal --> do not copy and exit for*/
                    copyflag = WLANCONNECT_ZERO;
                    sCount = WLANCONNECT_ZERO;
                    listPosition++;
                }
                else
                {
                    /* the compared SSID are different*/
                    copyflag = WLANCONNECT_ONE;
                }
            }
            /* Copy only unique SSID scan data*/
            if ((WLANCONNECT_ONE == copyflag))
            {
                /* Cast and copy the SSID */
                for (int8_t j = WLANCONNECT_ZERO; j < WLANCONNECT_MAX_SSID_LENGTH; j++)
                {
                    scanList->ScanData[i - listPosition].Ssid[j] = NetEntries[i].ssid[j];
                }
                /* Cast and copy the BSSID */
                for (int8_t k = WLANCONNECT_ZERO; k < WLANCONNECT_MAX_MAC_ADDR_LEN; k++)
                {
                    scanList->ScanData[i - listPosition].Bssid[k] = NetEntries[i].bssid[k];
                }
                /* Cast and copy the SSID Lenght, Security Type and RSSI */
                scanList->ScanData[i - listPosition].SsidLength = NetEntries[i].ssid_len;
                scanList->ScanData[i - listPosition].SecurityType = NetEntries[i].sec_type;
                scanList->ScanData[i - listPosition].Rssi = NetEntries[i].rssi;

                /* Copy the number of valid entries */
                scanList->NumOfScanEntries = (i - listPosition + 1);
            }
        }
        /* Add the time stamp of the last scan*/
        slStatus = SystemClock_getTime(&scanList->TimeStamp);
    }

    /* check status */
    if ( WLANCONNECT_ZERO != slStatus)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_SIMPLELINK_ERROR));
    }

    /* Check if Scan function has not found any networks*/
    if (WLANCONNECT_ZERO == scanList->NumOfScanEntries)
    {
        return (RETCODE(RETCODE_SEVERITY_ERROR, (uint32_t ) RETCODE_NO_NW_AVAILABLE));
    }
    else
    {
        retVal = RETCODE_OK;
    }

    return (retVal);
}

/** ************************************************************************* */

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
 * This module handles the SNTP timestamp (from server and system).
 *
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_SNTP

#if XDK_UTILITY_SNTP
/* own header files */
#include "XDK_SNTP.h"
/* system header files */
#include <stdio.h>
#include <stddef.h>

/* additional interface header files */

#include "BCDS_WlanNetworkConfig.h"

#if XDK_UTILITY_SERVALPAL
#include "Serval_Sntp.h"
#else /* XDK_UTILITY_SERVALPAL */
#include "FreeRTOS.h"
#include "socket.h"
#include "simplelink.h"
#include "PIp.h"
#include "semphr.h"
#endif /* XDK_UTILITY_SERVALPAL */

/* constant definitions ***************************************************** */
/**< SNTP setup information */
static SNTP_Setup_T SNTPSetup;
/**< SNTP time-stamp from received from the server */
static uint32_t SNTPTimeStampFromServer = UINT32_C(0);
static SemaphoreHandle_t SNTPTimeRxHandle;
/**< System time upon request */
static uint32_t SNTPSystemTimeUponRequest = UINT32_C(0);
/**< System time upon last sync with the server */
static uint32_t SNTPSystemTimeUponLastSync = UINT32_C(0);

#if XDK_UTILITY_SERVALPAL == 0
#include "socket.h"
/* Receive retry interval */
#define SOC_MONITOR_PERIOD                  200UL
/* Subtract 70 years worth of seconds from the seconds since 1900. This leaves the seconds since the UNIX epoch of 1970. */
#define SNTP_DELTA_TIME                     2208988800UL
/**< SNTP socket id */
static int32_t SNTP_SocketId;
static SlSockAddrIn_t serverAddr;
/**< SNTP payload structure */
struct SNTP_Payload_S
{
    /* First 8 bits are preamble to the sntp payload as Leap indicator(LI-2 bits),
     * Version number of the protocol (VN- 3 bits),
     * Client mode (Mode 3 bits) */
    uint8_t LeapIndicator : 2; /*lint !e46 */
    uint8_t ProtocolVersion : 3; /*lint !e46 */
    uint8_t Mode : 3; /*lint !e46 */

    uint8_t Stratum;                /* Stratum level of the local clock */
    uint8_t Poll;                   /* Maximum interval between successive messages */
    uint8_t Precision;              /*  Precision of the local clock */

    uint32_t RootDelay;             /* Total round trip delay time */
    uint32_t RootDispersion;        /* Max error aloud from primary clock source */
    uint32_t RefId;                 /* Reference clock identifier */

    uint32_t RefTmSec;              /* Reference time-stamp seconds */
    uint32_t RefTmFrac;             /* Reference time-stamp fraction of a second */

    uint32_t OrigTmSec;             /* Originate time-stamp seconds */
    uint32_t OrigTmFrac;            /* Originate time-stamp fraction of a second */

    uint32_t RxTmSec;               /* Received time-stamp seconds */
    uint32_t RxTmFrac;              /* Received time-stamp fraction of a second */

    uint32_t TxTmSec;               /*  Transmit time-stamp seconds */
    uint32_t TxTmFrac;              /* Transmit time-stamp fraction of a second */
};
typedef struct __attribute__((packed)) SNTP_Payload_S SNTP_Payload_T;
#endif /* XDK_UTILITY_SERVALPAL == 0 */

/**
 * @brief Application callback to inform when the message was sent.
 *
 * @param[in] msg_ptr
 * Unused
 *
 * @param[in] status
 * Status of message whether it is sent or not
 */
#if XDK_UTILITY_SERVALPAL
static void SntpSentCallback(Msg_T *msg_ptr, retcode_t status)
{
    BCDS_UNUSED(msg_ptr);
    if (RC_OK == status)
    {
        printf("SntpSentCallback : Success\r\n");
    }
    else
    {
        printf("SntpSentCallback : failed\r\n");
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SNTP_SEND_FAILED));
        SNTPTimeStampFromServer = UINT32_C(0);
    }
}

/**
 * @brief SntpTimeCallback is callback to inform application about the received time stamp
 *
 * @param[in] sourceIp
 * SNTP server IP. Unused.
 *
 * @param[in] sourcePort
 * SNTP server port. Unused.
 *
 * @param[in] timestamp
 * Time stamp in sec from the server
 */
static void SntpTimeCallback(Ip_Address_T* sourceIp, Ip_Port_T sourcePort, uint32_t timestamp)
{
    BCDS_UNUSED(sourceIp);
    BCDS_UNUSED(sourcePort);

    printf("SntpTimeCallback : received\r\n");

    SNTPTimeStampFromServer = timestamp;
    SNTPSystemTimeUponLastSync = xTaskGetTickCount();

    if (pdTRUE != xSemaphoreGive(SNTPTimeRxHandle))
    {
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SNTP_CB_RECEIVED_AFTER_TIMEOUT));
    }
}
#endif /* XDK_UTILITY_SERVALPAL */

/** Refer interface header for description */
Retcode_T SNTP_Setup(SNTP_Setup_T * setup)
{
    Retcode_T retcode = RETCODE_OK;
    if (NULL == setup)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        SNTPTimeRxHandle = xSemaphoreCreateBinary();
        if (NULL == SNTPTimeRxHandle)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
        }
        else
        {
            SNTPSetup = *setup;
        }
    }
    return retcode;
}
/** Refer interface header for description */
#if XDK_UTILITY_SERVALPAL
Retcode_T SNTP_Enable(void)
{
    Retcode_T retcode = RETCODE_OK;
    Ip_Port_T destPort = (Ip_Port_T) SNTPSetup.ServerPort;
    if (RC_OK != Sntp_initialize())
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SNTP_INIT_FAILED);
    }
    if (RETCODE_OK == retcode)
    {
        if (RC_OK != Sntp_start(Ip_convertIntToPort(destPort), SntpTimeCallback))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SNTP_START_FAILED);
        }
    }
    return retcode;
}
#else /* XDK_UTILITY_SERVALPAL */
Retcode_T SNTP_Enable(void)
{
    Retcode_T retcode = RETCODE_OK;
    serverAddr.sin_family = SL_AF_INET;
    serverAddr.sin_port = sl_Htons(SNTPSetup.ServerPort);
    serverAddr.sin_addr.s_addr = 0;

    /* socket create */
    SNTP_SocketId = sl_Socket(SL_AF_INET, SL_SOCK_DGRAM, IPPROTO_UDP);
    if (SNTP_SocketId < INT16_C(0))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SNTP_INIT_FAILED);
    }
    /* socket set non blocking */
    if (RETCODE_OK == retcode)
    {
        SlSockNonblocking_t enableOption;
        enableOption.NonblockingEnabled = 1;
        if (sl_SetSockOpt(SNTP_SocketId, SOL_SOCKET, SL_SO_NONBLOCKING, &enableOption, sizeof(enableOption)) < 0)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SNTP_INIT_FAILED);
        }
    }
    /* socket bind to port */
    if (RETCODE_OK == retcode)
    {
        if (sl_Bind(SNTP_SocketId, (SlSockAddr_t *) &serverAddr, sizeof(SlSockAddrIn_t)) < 0)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SNTP_INIT_FAILED);
        }
    }
    if (RETCODE_OK != retcode)
    {
        (void) sl_Close(SNTP_SocketId);
    }

    return retcode;
}
#endif /* XDK_UTILITY_SERVALPAL */

/** Refer interface header for description */
#if XDK_UTILITY_SERVALPAL
Retcode_T SNTP_GetTimeFromServer(uint64_t * sntpTimeStamp, uint32_t timeout)
{
    Ip_Address_T sntpIpAddress = 0UL;
    Retcode_T retcode = RETCODE_OK;
    Ip_Port_T destPort = (Ip_Port_T) SNTPSetup.ServerPort;
    uint32_t timeStamp = 0UL;

    if (NULL == sntpTimeStamp)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        retcode = WlanNetworkConfig_GetIpAddress((uint8_t *) SNTPSetup.ServerUrl, &sntpIpAddress);
        if (RETCODE_OK == retcode)
        {
            /* This is a dummy take. In case of any callback received
             * after the previous timeout will be cleared here. */
            (void) xSemaphoreTake(SNTPTimeRxHandle, 0UL);

            if (RC_OK == Sntp_getTime(&sntpIpAddress, Ip_convertIntToPort(destPort), SntpSentCallback))
            {
                if (pdTRUE == xSemaphoreTake(SNTPTimeRxHandle, pdMS_TO_TICKS(timeout)))
                {
                    timeStamp = SNTPTimeStampFromServer;
                }
                else
                {
                    retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SNTP_CB_NOT_RECEIVED);
                }
            }
            else
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SNTP_GET_TIME_FAILED);
            }
        }
        if (RETCODE_OK == retcode)
        {
            *sntpTimeStamp = timeStamp;
        }
    }
    return retcode;
}
#else /* XDK_UTILITY_SERVALPAL */
Retcode_T SNTP_GetTimeFromServer(uint64_t * sntpTimeStamp, uint32_t timeout)
{
    Retcode_T retcode = RETCODE_OK;
    Ip_Address_T sntpIpAddress = 0UL;
    int16_t addrSize = sizeof(SlSockAddrIn_t);
    SNTP_Payload_T sntpPayload;
    memset(&sntpPayload, 0, sizeof(SNTP_Payload_T));
    int32_t receiveLen;

    if (NULL == sntpTimeStamp)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        retcode = WlanNetworkConfig_GetIpAddress((uint8_t *) SNTPSetup.ServerUrl, &sntpIpAddress);
        if (RETCODE_OK == retcode)
        {
            /* socket sent data to server */
            serverAddr.sin_addr.s_addr = sntpIpAddress;
            sntpPayload.LeapIndicator = 0x03;
            sntpPayload.ProtocolVersion = 0x04; /* V4 in 2010 */
            sntpPayload.Mode = 0x03;
            if (sl_SendTo(SNTP_SocketId, &sntpPayload, sizeof(sntpPayload), 0,
                    (SlSockAddr_t *) &serverAddr, sizeof(SlSockAddrIn_t)) != sizeof(sntpPayload))
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SNTP_SEND_FAILED);
            }
            else
            {
                printf("Sntp sent success\r\n");
            }
        }

        if (RETCODE_OK == retcode)
        {
            /* socket receive timestamp from server */
            uint32_t maxReceiveTry = timeout/SOC_MONITOR_PERIOD;
            for (uint8_t i = 0; i <= maxReceiveTry; i++)
            {
                receiveLen = sl_RecvFrom(SNTP_SocketId, &sntpPayload, sizeof(sntpPayload), 0,
                                    (SlSockAddr_t *) &serverAddr, (SlSocklen_t*) &addrSize);
                if (receiveLen <= 0)
                {
                    retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SNTP_RECEIVE_FAILED);
                }
                else
                {
                    retcode = RETCODE_OK;
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(SOC_MONITOR_PERIOD));
            }
        }
        if (RETCODE_OK == retcode)
        {
            sntpPayload.TxTmSec = ntohl( sntpPayload.TxTmSec );
            SNTPTimeStampFromServer = sntpPayload.TxTmSec - SNTP_DELTA_TIME;

            *sntpTimeStamp = (uint64_t) SNTPTimeStampFromServer;
            printf("Sntp receive success\r\n");
        }
    }
    return retcode;
}
#endif /* XDK_UTILITY_SERVALPAL */

/** Refer interface header for description */
Retcode_T SNTP_GetTimeFromSystem(uint64_t * sntpTimeStamp, uint32_t * timeLapseInMs)
{
    Retcode_T retcode = RETCODE_OK;
    uint64_t timeStamp = 0UL;
    uint32_t timeLapseInMilliSeconds = 0UL;
    uint64_t timeInSeconds = 0UL;

    if ((NULL == sntpTimeStamp) || (NULL == timeLapseInMs))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        SNTPSystemTimeUponRequest = xTaskGetTickCount();
        if ((SNTPSystemTimeUponRequest > SNTPSystemTimeUponLastSync) && (SNTPTimeStampFromServer != 0))
        {
            timeInSeconds = (SNTPSystemTimeUponRequest - SNTPSystemTimeUponLastSync) / 1000; /* milli-seconds to seconds conversion */
            timeLapseInMilliSeconds = (SNTPSystemTimeUponRequest - SNTPSystemTimeUponLastSync) % 1000;
            timeStamp = SNTPTimeStampFromServer + timeInSeconds;
        }
        else
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SNTP_SYSTEM_TIME_CALC_FAILED);
        }
        if (RETCODE_OK == retcode)
        {
            *sntpTimeStamp = timeStamp;
            *timeLapseInMs = timeLapseInMilliSeconds;
        }
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T SNTP_SetTime(uint64_t sntpTimeStamp)
{
    Retcode_T retcode = RETCODE_OK;

    if (0UL == sntpTimeStamp)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
    }
    else
    {
        SNTPTimeStampFromServer = sntpTimeStamp;
        SNTPSystemTimeUponLastSync = xTaskGetTickCount();
    }

    return retcode;
}
/** Refer interface header for description */
Retcode_T SNTP_Disable(void)
{
    Retcode_T retcode = RETCODE_OK;
#if XDK_UTILITY_SERVALPAL
    if (RC_OK != Sntp_stop())
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_FAILURE);
    }
#else /* XDK_UTILITY_SERVALPAL */
    if (0UL != sl_Close(SNTP_SocketId))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_FAILURE);
    }
#endif /* XDK_UTILITY_SERVALPAL */
    return retcode;

}
#endif /* XDK_UTILITY_SNTP */

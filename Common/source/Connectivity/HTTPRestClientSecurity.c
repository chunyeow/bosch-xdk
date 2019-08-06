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
 * This module handles the HTTP rest client secure services (Server certificate setup)
 *
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_HTTPRESTCLIENTSECURITY

/* own header files */
#include "HTTPRestClientSecurity.h"

/* system header files */
#include <stdio.h>

#if SERVAL_ENABLE_TLS_CLIENT
/* additional interface header files */
#include "XDK_SNTP.h"
#include "Serval_Security.h"
#include "ServerCA.h"
#include "x509.h"

/* constant definitions ***************************************************** */

/* local variables ********************************************************** */

/**
 * @brief HTTPRestClientSecurityGetTimeOfDay will retrieve the time from SNTP module.
 *
 * @param[in/out]  tv
 * Time structure for standard struct timeval.
 *
 * @param[in/out]  tzvp
 * Time structure for custom struct. Unused.
 *
 * @return  status of time calculation. 0 on success. Non-zero on error.
 */
int HTTPRestClientSecurityGetTimeOfDay(struct timeval *tv, void *tzvp)
{
    BCDS_UNUSED(tzvp);
    uint64_t t = 0UL; /* get uptime in nanoseconds */
    uint32_t ms = 0UL; /* get uptime in nanoseconds */
    (void) SNTP_GetTimeFromSystem(&t, &ms);
    BCDS_UNUSED(ms);
    tv->tv_sec = t / 1000; /* convert to seconds */
    tv->tv_usec = t % 1000; /* get remaining microseconds */
    return 0;
}

/**
 * @brief HTTPRestClientSecurityHandleCurrentTime will handle the current time.
 *
 * @param[in/out]  tokenData
 * Token data contains additional data to help identify what to provide to the
 * implementation as well as everything required to actually provide this data.
 * An application implementing the callback is expected to cast token data to
 * a type depending on the type field of the provided token.
 *
 * @return  status of time calculation
 */
static inline retcode_t HTTPRestClientSecurityHandleCurrentTime(SecurityData_T* tokenData)
{
    retcode_t rc = RC_OK;
    struct timeval t = { 0, 0 };
    if (0 == HTTPRestClientSecurityGetTimeOfDay(&t, 0))
    {
        if (0 == t.tv_sec)
        {
            printf("HTTPRestClientSecurityHandleCurrentTime : Failed since SNTP time was invalid.\r\n");
            rc = RC_DTLS_TOKEN_NOT_PROVIDED;
        }
        else
        {
            tokenData->timeData.timestamp = t.tv_sec + UNIX_TIMESTAMP_OFFSET;
        }
    }
    else
    {
        printf("HTTPRestClientSecurityHandleCurrentTime : Failed since HTTPRestClientSecurityGetTimeOfDay returned error.\r\n");
        rc = RC_DTLS_TOKEN_NOT_PROVIDED;
    }
    return rc;
}

/**
 * @brief HTTPRestClientSecurityCallback is a Callback function registered to the stack which will be
 * used to obtain security information such as pre-shared-keys, certificates, etc.
 *
 * @param[in]      token
 * The type of token that is requested.
 * The token.type field determines the data type of tokenData.
 * The other fields in token determine the validity of fields in that data type
 * and provide additional information to the application about the type of
 * connection involved.
 *
 * The mapping of the token.type field to data types of tokenData is
 * CIPHERS              -> CiphersData_T,
 * PSK_SERVER_ID_HINT   -> ServerIdHintData_T,
 * PSK_PEER_KEY_AND_ID  -> PeerKeyAndIdData_T,
 * CRT_PEER_NAME        -> PeerNameData_T,
 * CRT_CA               -> CaData_T,
 * CRT_OWN_CERTIFICATE  -> OwnCertificateData_T,
 *
 * @param[in/out]  tokenData
 * Token data contains additional data to help identify what to provide to the
 * implementation as well as everything required to actually provide this data.
 * An application implementing the callback is expected to cast token data to
 * a type depending on the type field of the provided token.
 *
 * The data that is provided in output buffers must be in a format that the
 * underlying SSL implementation can understand. No conversion is performed.
 *
 * @return  RC_OK if the token was provided
 *          RC_DTLS_TOKEN_NOT_PROVIDED if the application does not want to provide the token (e.g. client certificate)
 *          RC_DTLS_PEER_REJECTED if the application does not want to communicate with that peer
 *          RC_DTLS_UNSUPPORTED_TOKEN if the token type cannot be provided by the application
 *          RC_DTLS_INSUFFICIENT_MEMORY if the token does not fit into the allocated space
 */
retcode_t HTTPRestClientSecurityCallback(SecurityToken_T token, SecurityData_T* tokenData)
{
    retcode_t rc = RC_OK;

    if ((token.connection != TLS) || ((SecurityDeviceRole_t) token.deviceRole != CLIENT))
    {
        /* We are not interested */
        rc = RC_DTLS_UNSUPPORTED_TOKEN;
    }

    if (RC_OK == rc)
    {
        switch (token.type)
        {
        case CURRENT_TIME:
            printf("HTTPRestClientSecurityCallback - CURRENT_TIME request\r\n");
            rc = HTTPRestClientSecurityHandleCurrentTime(tokenData);
            break;

        case CRT_PEER_NAME:
            printf("HTTPRestClientSecurityCallback - CRT_PEER_NAME request\r\n");
            rc = RC_DTLS_TOKEN_NOT_PROVIDED;
            break;

        case CRT_CA:
            printf("HTTPRestClientSecurityCallback - CRT_CA request\r\n");
            /* @todo - Currently Certificate is parsed in MbedTLS Adaptor directly.
             * Need to adapt it to Seval security API 2 standard */
            break;

        case CRT_OWN_CERTIFICATE:
            printf("HTTPRestClientSecurityCallback - CRT_OWN_CERTIFICATE request\r\n");
            rc = RC_DTLS_TOKEN_NOT_PROVIDED;
            break;

        default:
            printf("HTTPRestClientSecurityCallback - default case entered\r\n");
            rc = RC_DTLS_UNSUPPORTED_TOKEN;
            break;
        }
    }
    return rc;
}

/** Refer interface header for description */
Retcode_T HTTPRestClientSecurity_Setup(void)
{
    return RETCODE_OK;
}

/** Refer interface header for description */
Retcode_T HTTPRestClientSecurity_Enable(void)
{
    Security_setCallback(HTTPRestClientSecurityCallback);
    return RETCODE_OK;
}

#endif /* SERVAL_ENABLE_TLS_CLIENT */

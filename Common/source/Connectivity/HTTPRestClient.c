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
 * This module handles the HTTP rest client services (POST and GET)
 *
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_HTTPRESTCLIENT

#if XDK_CONNECTIVITY_HTTPRESTCLIENT

/* own header files */
#include "XDK_HTTPRestClient.h"

/* system header files */
#include <stdio.h>

/* additional interface header files */
#include "MbedTLSAdapter.h"
#include "HTTPRestClientSecurity.h"
#include "BCDS_WlanNetworkConfig.h"
#include "Serval_Msg.h"
#include "Serval_Http.h"
#include "Serval_HttpClient.h"
#include "Serval_Types.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "BCDS_FWContainer.h"
#include "XDK_Storage.h"
#include "XDK_FOTA.h"

/* constant definitions ***************************************************** */

#define FIRMWARE_DOWNLOAD_SIZE      UINT32_C(512)   /**< Max firmware size for each HTTP GET */
#define DOWNLOAD_URL_LENGTH         UINT32_C(100)   /**< Max URL length for download */
#define HTTP_GET_TIME_OUT           UINT32_C(10000) /**< Timeout for HTTP GET */
#define HTTP_GET_PROCESS_DELAY      UINT32_C(10000)    /**< Delay for HTTP GET callback and next HTTP GET */

/* local variables ********************************************************** */

/**
 * @brief Enumeration to represent firmware download status.
 */
enum Firmware_Status_E
{
    FIRMWARE_DOWNLOAD_IDLE,         /**< Firmware download in idle state */
    FIRMWARE_DOWNLOAD_INPROGRESS,   /**< Firmware download in progress */
    FIRMWARE_DOWNLOAD_COMPLETED,    /**< Firmware download complete */
    FIRMWARE_CRC_FAIL,              /**< Firmware download crc validation fail */
    FIRMWARE_WRITE_ERROR,           /**< Firmware download crc validation fail */
    FIRMWARE_MAX,
};

typedef enum Firmware_Status_E Firmware_Status_T;

/**< Handle for HTTP rest client POST operation synchronization signal */
static SemaphoreHandle_t HTTPRestClientPostHandle;
/**< Handle for HTTP rest client GET operation synchronization signal */
static SemaphoreHandle_t HTTPRestClientGetHandle;
/**< Handle for Get firmware operation synchronization signal */
static SemaphoreHandle_t GetFirmwareHandle;
/**< HTTP rest client setup information */
static HTTPRestClient_Setup_T HTTPRestClientSetupInfo = {   false, NULL};
/**< HTTP rest client GET in progress flag */
static bool HTTPRestClientGetInProgress = false;
/**< HTTP rest client GET information */
static HTTPRestClient_GetCB_T HttpRestClientGetCB = NULL;
/**< HTTP rest client GET page offset */
static uint32_t HTTPRestClientGetPageOffset = 0UL;
/**< HTTP rest client POST information */
static HTTPRestClient_Post_T HttpRestClientPostInfo;
/**< HTTP rest client POST configuration */
static HTTPRestClient_Config_T HttpRestClientPostConfig;
/**< HTTP rest client POST status on callback from the stack */
static retcode_t HTTPRestClientPostRcOnCb = RC_OK;
/**< File offset position for fota download */
static uint32_t GetDataOffset = 0U;
/**< Status flag for http get status */
static bool GetStatusFlag = false;
/**< Buffer to update the url with download size and file offset position */
static uint8_t UrlOffsetBuffer[DOWNLOAD_URL_LENGTH] = { 0 };
static uint32_t FirmwareSize;

static volatile Firmware_Status_T FirmwareStatus = FIRMWARE_DOWNLOAD_IDLE;

static void StoreGetData(void * buff, uint32_t length)
{
    Retcode_T retcode = RETCODE_OK;
    FWContainer_Header_T * headerInfo;
    Storage_Write_T writeData =
    {
        .FileName = "fw.bin",
        .WriteBuffer = buff,
        .BytesToWrite = length,
        .ActualBytesWritten = 0UL,
        .Offset = (uint16_t) GetDataOffset * FIRMWARE_DOWNLOAD_SIZE,
    };
    retcode = Storage_Write(STORAGE_MEDIUM_SD_CARD,&writeData);
    if ((length == writeData.ActualBytesWritten) && (RETCODE_OK == retcode))
    {
        if (0U == GetDataOffset)
        {
            headerInfo = (FWContainer_Header_T *)buff;
            FirmwareSize = headerInfo->FirmwareSize;
        }
        if (FirmwareSize > (GetDataOffset * FIRMWARE_DOWNLOAD_SIZE))
        {
            GetDataOffset++;
            memset(UrlOffsetBuffer, '\0', sizeof(UrlOffsetBuffer));
            snprintf((char *)UrlOffsetBuffer, sizeof(UrlOffsetBuffer), "/%ld/%ld/", FIRMWARE_DOWNLOAD_SIZE, (GetDataOffset * FIRMWARE_DOWNLOAD_SIZE));
            if (FirmwareSize < (GetDataOffset * FIRMWARE_DOWNLOAD_SIZE))
            {
                memset(UrlOffsetBuffer, '\0', sizeof(UrlOffsetBuffer));
                snprintf((char *)UrlOffsetBuffer, sizeof(UrlOffsetBuffer), "/%ld/%ld/", (FIRMWARE_DOWNLOAD_SIZE - ((GetDataOffset * FIRMWARE_DOWNLOAD_SIZE) - FirmwareSize)), ((GetDataOffset * FIRMWARE_DOWNLOAD_SIZE)));
            }
        }
        else
        {
            GetDataOffset = 0;
            retcode = FOTA_ValidateSdcardFw(writeData.FileName);
            if(RETCODE_OK == retcode)
            {
                FirmwareStatus = FIRMWARE_DOWNLOAD_COMPLETED;
                printf("HttpRestClient: Firmware Verification successful\r\n");
            }
            else
            {
                printf("HttpRestClient: Downloaded firmware file corrupted \r\n");
                FirmwareStatus = FIRMWARE_CRC_FAIL;
            }
        }
    }
    else
    {
        printf("HttpRestClient: Data write error \r\n");
        FirmwareStatus = FIRMWARE_WRITE_ERROR;
        Retcode_RaiseError(retcode);
    }
    GetStatusFlag = false;
    (void) xSemaphoreGive(GetFirmwareHandle);
}

static void HTTPRestClientGetCallback(const char* responseContent, uint32_t responseContentLen, bool isLastMessage)
{
    BCDS_UNUSED(isLastMessage);
    Retcode_T retcode = RETCODE_OK;

    if ((NULL != responseContent) && (0UL != responseContentLen))
    {
        GetStatusFlag = true;
        retcode = CmdProcessor_Enqueue(HTTPRestClientSetupInfo.CmdProcessor, StoreGetData, (void *) responseContent, responseContentLen);
        if(RETCODE_OK != retcode)
        {
            Retcode_RaiseError(retcode);
        }
    }
}

/**
 * @brief   HttpRestClientGetResponseCallback is called once a response to the GET request is received.
 *
 * @param [in] httpSession
 * A pointer to the currently active HTTP session
 *
 * @param [in] httpMessage
 * A pointer to the HTTP message for which we're sending/receiving a response
 *
 * @param [in] @param status
 * The return code of the HTTP page download
 *
 * @return  The return code of the HTTP connect
 */
static retcode_t HttpRestClientGetResponseCallback(HttpSession_T *httpSession, Msg_T *httpMessage, retcode_t status)
{
    BCDS_UNUSED(httpSession);

    if (RC_OK != status)
    {
        printf("HttpRestClientGetResponseCallback: error while receiving response to GET request. error=%d\r\n", status);
        return RC_OK;
    }
    if (NULL == httpMessage)
    {
        printf("HttpRestClientGetResponseCallback: received NULL as HTTP message. This should not happen.\r\n");
        return RC_OK;
    }

    Http_StatusCode_T httpStatusCode = HttpMsg_getStatusCode(httpMessage);
    if (Http_StatusCode_OK != httpStatusCode)
    {
        printf("HttpRestClientGetResponseCallback: received HTTP status other than 200 OK. status=%d\r\n", httpStatusCode);
    }
    else
    {
        retcode_t retcode;
        bool isLastPartOfMessage;
        uint32_t pageContentSize;
        retcode = HttpMsg_getRange(httpMessage, UINT32_C(0), &pageContentSize, &isLastPartOfMessage);
        if (RC_OK != retcode)
        {
            printf("HttpRestClientGetResponseCallback: failed to get range from message. error=%d\r\n", retcode);
        }
        else
        {
            const char* responseContent;
            unsigned int responseContentLen;
            HttpMsg_getContent(httpMessage, &responseContent, &responseContentLen);
            if (NULL != HttpRestClientGetCB)
            {
                HttpRestClientGetCB(responseContent, responseContentLen, isLastPartOfMessage);
            }
            else
            {
                printf("HttpRestClientGetResponseCallback: successfully received a response: %.*s\r\n", responseContentLen, responseContent);
                if (FIRMWARE_DOWNLOAD_INPROGRESS == FirmwareStatus)
                {
                    HTTPRestClientGetCallback(responseContent, responseContentLen, isLastPartOfMessage);
                }
            }

            if (isLastPartOfMessage)
            {
                /* We're done with the GET request. */
                printf("HttpRestClientGetResponseCallback: HTTP rest client GET was successful.\r\n");
                HTTPRestClientGetInProgress = false;
                (void) xSemaphoreGive(HTTPRestClientGetHandle);
            }
            else
            {
                /* We're not done yet downloading the page - let's make another request. */
                printf("HttpRestClientGetResponseCallback: there is still more to GET. Making another request.\r\n");
                HTTPRestClientGetPageOffset += responseContentLen;
                (void) xSemaphoreGive(HTTPRestClientGetHandle);
            }
        }
    }

    return RC_OK;
}

/**
 * @brief   HttpRestClientPostPayloadSerializer serialize the HTTP request body of the POST request we're sending.
 *
 * @param [in] serializationHandover
 * The structure to store the body in
 *
 * @return indication whether we need more space or we are done
 */
static retcode_t HttpRestClientPostPayloadSerializer(OutMsgSerializationHandover_T* serializationHandover)
{
    /* We use a global variable which stores a previously set message.
     */
    const char* httpBodyBuffer = HttpRestClientPostInfo.Payload;

    uint32_t offset = serializationHandover->offset;
    uint32_t bytesLeft = HttpRestClientPostInfo.PayloadLength - offset;
    uint32_t bytesToCopy = serializationHandover->bufLen > bytesLeft ? bytesLeft : serializationHandover->bufLen;

    memcpy(serializationHandover->buf_ptr, httpBodyBuffer + offset, bytesToCopy);
    serializationHandover->len = bytesToCopy;

    if (bytesToCopy < bytesLeft)
    {
        return RC_MSG_FACTORY_INCOMPLETE;
    }
    else
    {
        return RC_OK;
    }
}

/**
 * @brief   HttpRestClientPostCustomHeaderSerializer adds two custom header to an HTTP request: X-AuthToken and X-Foobar.
 *
 * @param [in] serializationHandover
 * The serialization context we're adding the header to.
 *
 * @return status of serialization
 */
static retcode_t HttpRestClientPostCustomHeaderSerializer(OutMsgSerializationHandover_T* serializationHandover)
{
    retcode_t rc = RC_OK;

    if (serializationHandover == NULL)
    {
        printf("HttpRestClientPostCustomHeaderSerializer: serializationHandover is NULL. This should never happen.\r\n");
        rc = RC_APP_ERROR;
    }
    else
    {
        if (RC_OK == rc)
        {
            /* @todo - Validate if serializationHandover->len needs to be made '0'. If yes, then add the code line "serializationHandover->len = 0;" */

            switch (serializationHandover->position)
            {
            case 0:
                rc = TcpMsg_copyStaticContent(serializationHandover, HttpRestClientPostInfo.RequestCustomHeader0, strlen(HttpRestClientPostInfo.RequestCustomHeader0));
                if (RC_OK == rc)
                {
                    serializationHandover->position = 1;
                }
                break;
            case 1:
                rc = TcpMsg_copyContentAtomic(serializationHandover, HttpRestClientPostInfo.RequestCustomHeader1, strlen(HttpRestClientPostInfo.RequestCustomHeader1));
                if (RC_OK == rc)
                {
                    serializationHandover->position = 2;
                }
                break;
            default:
                rc = RC_OK;
            }
        }
    }

    return rc;
}

/**
 * @brief   HttpRestClientRequestSentCallback is called when the HTTP request was pushed.
 * The callerStatus passed to this function indicates whether the request was sent successfully or not.
 *
 * @param [in] caller
 * The callable to which this callback was assigned to
 *
 * @param [in] callerStatus
 * The status indicating whether the request was sent or not
 *
 * @return a retcode indicating the success of this callback
 */
static retcode_t HttpRestClientRequestSentCallback(Callable_T* caller, retcode_t callerStatus)
{
    BCDS_UNUSED(caller);

    if (RC_OK == callerStatus)
    {
        printf("HttpRestClientRequestSentCallback: HTTP request sent successfully.\r\n");
    }
    else
    {
        printf("HttpRestClientRequestSentCallback: HTTP request failed to send. error=%d (Note : RC_HTTP_CLIENT_NO_RESPONSE-%d , RC_HTTP_SEND_ERROR-%d) \r\n", (int) callerStatus, RC_HTTP_CLIENT_NO_RESPONSE, RC_HTTP_SEND_ERROR);
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_REQUEST_SEND_FAILED));
    }

    return RC_OK;
}

/**
 * @brief   HttpRestClientPostResponseCallback is called once a response to the POST request is received.
 *
 * @param [in] httpSession
 * A pointer to the currently active HTTP session
 *
 * @param [in] httpMessage
 * A pointer to the HTTP message we're sending/receiving a response for
 *
 * @param [in] status
 * The return code of the HTTP page download
 *
 * @return a retcode indicating the HTTP connect status
 */
static retcode_t HttpRestClientPostResponseCallback(HttpSession_T *httpSession, Msg_T *httpMessage, retcode_t status)
{
    BCDS_UNUSED(httpSession);

    if (NULL == httpMessage)
    {
        printf("HttpRestClientPostResponseCallback: received NULL as HTTP message. This should not happen.\r\n");
        return RC_APP_ERROR;
    }

    Http_StatusCode_T httpStatusCode = HttpMsg_getStatusCode(httpMessage);
    if (Http_StatusCode_OK != httpStatusCode)
    {
        printf("HttpRestClientPostResponseCallback: received HTTP status other than 200 OK. status=%d\r\n", httpStatusCode);
    }
    else
    {
        retcode_t retcode;
        bool isLastPartOfMessage;
        uint32_t pageContentSize;
        retcode = HttpMsg_getRange(httpMessage, UINT32_C(0), &pageContentSize, &isLastPartOfMessage);
        if (RC_OK != retcode)
        {
            printf("HttpRestClientPostResponseCallback: failed to get range from message. error=%d\r\n", retcode);
        }
        else
        {
            const char* responseContent;
            unsigned int responseContentLen;
            HttpMsg_getContent(httpMessage, &responseContent, &responseContentLen);
            printf("HttpRestClientPostResponseCallback: successfully received a response: %.*s\r\n", responseContentLen, responseContent);

            if (!isLastPartOfMessage)
            {
                /* We're not done yet downloading the page - let's make another request. */
                printf("HttpRestClientPostResponseCallback: server response was too large. This example application does not support POST responses larger than %d.\r\n", (int) HttpRestClientPostConfig.RequestMaxDownloadSize);
            }

            printf("HttpRestClientPostResponseCallback: POST request is done\r\n");
        }
    }

    HTTPRestClientPostRcOnCb = status;
    if (RC_OK == HTTPRestClientPostRcOnCb)
    {
        printf("HttpRestClientPostResponseCallback: HTTP rest client POST was successful.\r\n");
    }
    else
    {
        printf("HttpRestClientPostResponseCallback: error while receiving response to POST request. error=%d (Note : RC_HTTP_CLIENT_NO_RESPONSE-%d , RC_HTTP_SEND_ERROR-%d) \r\n", status, RC_HTTP_CLIENT_NO_RESPONSE, RC_HTTP_SEND_ERROR);
    }
    (void) xSemaphoreGive(HTTPRestClientPostHandle);
    return RC_OK;
}

/** Refer interface header for description */
Retcode_T HTTPRestClient_Setup(HTTPRestClient_Setup_T * setup)
{
    Retcode_T retcode = RETCODE_OK;
    if (NULL == setup)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        HTTPRestClientSetupInfo.CmdProcessor = setup->CmdProcessor;
        if (setup->IsSecure)
        {
#if SERVAL_ENABLE_TLS_CLIENT
#if SERVAL_ENABLE_TLS && SERVAL_TLS_MBEDTLS
            if(RC_OK != MbedTLSAdapter_Initialize())
            {
                printf("MbedTLSAdapter_Initialize : unable to initialize Mbedtls.\r\n");
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_INIT_REQUEST_FAILED);
            }
#endif /* SERVAL_ENABLE_TLS && SERVAL_TLS_MBEDTLS */
            if ( RETCODE_OK == retcode )
            {
                retcode = HTTPRestClientSecurity_Setup();
            }
#else /* SERVAL_ENABLE_TLS_CLIENT */
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_ENABLE_SERVAL_TLS_CLIENT);
#endif /* SERVAL_ENABLE_TLS_CLIENT */
        }
        if (RETCODE_OK == retcode)
        {
            HTTPRestClientPostHandle = xSemaphoreCreateBinary();
            if (NULL == HTTPRestClientPostHandle)
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
        }
        if (RETCODE_OK == retcode)
        {
            HTTPRestClientGetHandle = xSemaphoreCreateBinary();
            if (NULL == HTTPRestClientGetHandle)
            {
                vSemaphoreDelete(HTTPRestClientPostHandle);
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
        }
        if (RETCODE_OK == retcode)
        {
            GetFirmwareHandle = xSemaphoreCreateBinary();
            if (NULL == GetFirmwareHandle)
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
            }
        }
        if (RETCODE_OK == retcode)
        {
            HTTPRestClientSetupInfo = *setup;
        }
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T HTTPRestClient_Enable(void)
{
    Retcode_T retcode = RETCODE_OK;

    if (HTTPRestClientSetupInfo.IsSecure)
    {
#if SERVAL_ENABLE_TLS_CLIENT
        retcode = HTTPRestClientSecurity_Enable();
#else /* SERVAL_ENABLE_TLS_CLIENT */
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_ENABLE_SERVAL_TLS_CLIENT);
#endif /* SERVAL_ENABLE_TLS_CLIENT */
    }
    if (RETCODE_OK == retcode)
    {
        /* start client */
        if (RC_OK != HttpClient_initialize())
        {
            printf("HTTPRestClient_Enable : Failed to initialize http client \r\n");
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_CLIENT_INIT_FAILED);
        }
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T HTTPRestClient_Post(HTTPRestClient_Config_T * config, HTTPRestClient_Post_T * post, uint32_t timeout)
{
    Retcode_T retcode = RETCODE_OK;
    Msg_T* httpMessage =   {   0};
    retcode_t rc = RC_OK;
    Ip_Address_T destServerAddress;

    if ((NULL == config) || (NULL == post))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        if (RETCODE_OK == retcode)
        {
            /* Some forms of load-balancing yield a different IP address upon resolving the DNS name.
             * Thus, we should resolve the host name for each request. If we know that the service we're
             * making a request to does not exhibit such behavior, we can get the host IP address once
             * when the WLAN connection is established.
             */
            retcode = WlanNetworkConfig_GetIpAddress((uint8_t *) config->DestinationServerUrl, &destServerAddress);
            if (RETCODE_OK != retcode)
            {
                printf("HTTPRestClient_Post : unable to resolve hostname error=%d.\r\n", (int) retcode);
            }
        }
        if (RETCODE_OK == retcode)
        {
            if (config->IsSecure)
            {
#if SERVAL_ENABLE_TLS_CLIENT
                /* send Secure request */
                rc = HttpClient_initSecureRequest(&destServerAddress, Ip_convertIntToPort(config->DestinationServerPort), &httpMessage);
#else /* SERVAL_ENABLE_TLS_CLIENT */
                rc = RC_APP_ERROR; /* Enable SERVAL_ENABLE_TLS_CLIENT for secure communication */
#endif /* SERVAL_ENABLE_TLS_CLIENT */

            }
            else
            {

                /* send request */
                rc = HttpClient_initRequest(&destServerAddress, Ip_convertIntToPort(config->DestinationServerPort), &httpMessage);
            }
            if (RC_OK != rc)
            {
                printf("HTTPRestClient_Post : unable to create HTTP request. error=%d.\r\n", rc);
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_INIT_REQUEST_FAILED);
            }
        }
        if (RETCODE_OK == retcode)
        {
            HttpMsg_setReqMethod(httpMessage, Http_Method_Post);

            HttpMsg_setContentType(httpMessage, Http_ContentType_App_Json);

            rc = HttpMsg_setReqUrl(httpMessage, post->Url);
            if (RC_OK != rc)
            {
                printf("HTTPRestClient_Post : unable to set request URL. error=%d.\r\n", rc);
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_SET_REQ_URL_FAILED);
            }
        }
        if (RETCODE_OK == retcode)
        {
            /* It's important that we set the HOST header as many services use this header for routing. Without
             * this header your HTTP request is likely to fail/result in an undesired response.
             */
            rc = HttpMsg_setHost(httpMessage, config->DestinationServerUrl);
            if (RC_OK != rc)
            {
                printf("HTTPRestClient_Post : unable to set HOST header. error=%d.\r\n", rc);
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_SET_HOST_FAILED);
            }
        }
        if (RETCODE_OK == retcode)
        {
            /* We can only receive a certain amount of data per request. For the sake of brevity we do not implement
             * multiple POST requests and expect the response to fit in REQUEST_MAX_DOWNLOAD_SIZE. See the HTTP GET
             * part for an example of how to implement the proper behavior.
             */
            HttpMsg_setRange(httpMessage, HTTPRestClientGetPageOffset, config->RequestMaxDownloadSize);

            /*
             * If we want to send custom header, e.g. an authentication token, we have to provide those through a custom
             * header serialization function such as HttpRestClientPostCustomHeaderSerializer.
             */
            HttpMsg_serializeCustomHeaders(httpMessage, HttpRestClientPostCustomHeaderSerializer);

            /*
             * Providing the body of the HTTP request works similarly to custom headers: through a serializer. Please
             * have a closer look at its implementation to understand how it works with the buffers and serializes the
             * data one chunk at a time.
             */
            rc = TcpMsg_prependPartFactory(httpMessage, HttpRestClientPostPayloadSerializer);
            if (RC_OK != rc)
            {
                printf("HTTPRestClient_Post : unable to serialize request body. error=%d.\r\n", rc);
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_FAILURE);
            }
        }
        if (RETCODE_OK == retcode)
        {
            /* The rc of pushRequest does not tell us if the message was indeed sent, but rather if the
             * push was initialized. To find out of the HTTP request was actually sent we have to check the
             * callerStatus in the HttpRestClientRequestSentCallback function.
             *
             * When we receive a response from the server, the HttpRestClientPostResponseCallback function is called.
             */

            Callable_T httpRequestSentCallable;
            (void) Callable_assign(&httpRequestSentCallable, HttpRestClientRequestSentCallback);
            /* This is a dummy take. In case of any callback received
             * after the previous timeout will be cleared here. */
            (void) xSemaphoreTake(HTTPRestClientPostHandle, 0UL);
            HTTPRestClientPostRcOnCb = RC_OK;
            HttpRestClientPostInfo = *post;
            HttpRestClientPostConfig = *config;
            rc = HttpClient_pushRequest(httpMessage, &httpRequestSentCallable, HttpRestClientPostResponseCallback);
            if (RC_OK != rc)
            {
                printf("HTTPRestClient_Post: unable to push the HTTP request. error=%d.\r\n", rc);
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_PUSH_REQUEST_FAILED);
            }
        }

        if (RETCODE_OK == retcode)
        {
            if (pdTRUE != xSemaphoreTake(HTTPRestClientPostHandle, pdMS_TO_TICKS(timeout)))
            {
                printf("HTTPRestClient_Post : Failed since Post CB was not received \r\n");
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_POST_CB_NOT_RECEIVED);
            }
            else if (RC_OK != HTTPRestClientPostRcOnCb)
            {
                printf("HTTPRestClient_Post : Failed since in Post CB status was error\r\n");
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_POST_CB_STATUS_ERROR);
            }
        }
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T HTTPRestClient_Get(HTTPRestClient_Config_T * config, HTTPRestClient_Get_T * get, uint32_t timeout)
{
    Retcode_T retcode = RETCODE_OK;
    retcode_t rc = RC_OK;
    Msg_T* httpMessage =   {   0};

    if ((NULL == config) || (NULL == get))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        /* Some forms of load-balancing yield a different IP address upon resolving the DNS name.
         * Thus, we should resolve the host name for each request. If we know that the service we're
         * making a request to does not exhibit such behavior, we can get the host IP address once
         * when the WLAN connection is established.
         */
        Ip_Address_T destServerAddress;
        retcode = WlanNetworkConfig_GetIpAddress((uint8_t*) config->DestinationServerUrl, &destServerAddress);
        if (RETCODE_OK != retcode)
        {
            printf("HTTPRestClient_Get : unable to resolve hostname.\r\n");
        }
        else
        {
            HttpRestClientGetCB = get->GetCB;
            HTTPRestClientGetPageOffset = 0UL;
            HTTPRestClientGetInProgress = true;
            do
            {
                if (RETCODE_OK == retcode)
                {
                    if (config->IsSecure)
                    {
#if SERVAL_ENABLE_TLS_CLIENT
                        /* send Secure request */
                        rc = HttpClient_initSecureRequest(&destServerAddress, Ip_convertIntToPort(config->DestinationServerPort), &httpMessage);
#else /* SERVAL_ENABLE_TLS_CLIENT */
                        rc = RC_APP_ERROR; /* Enable SERVAL_ENABLE_TLS_CLIENT for secure communication */
#endif /* SERVAL_ENABLE_TLS_CLIENT */
                    }
                    else
                    {
                        /* send request */
                        rc = HttpClient_initRequest(&destServerAddress, Ip_convertIntToPort(config->DestinationServerPort), &httpMessage);
                    }
                    if (RC_OK != rc)
                    {
                        printf("HTTPRestClient_Get : unable to create HTTP request. error=%d.\r\n", rc);
                        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_INIT_REQUEST_FAILED);
                    }
                }
                if (RETCODE_OK == retcode)
                {
                    HttpMsg_setReqMethod(httpMessage, Http_Method_Get);

                    HttpMsg_setContentType(httpMessage, Http_ContentType_Text_Html);

                    rc = HttpMsg_setReqUrl(httpMessage, get->Url);
                    if (RC_OK != rc)
                    {
                        printf("HTTPRestClient_Get : unable to set request URL. error=%d.\r\n", rc);
                        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_SET_REQ_URL_FAILED);
                    }
                }
                /* It's important that we set the HOST header as many services use this header for routing. Without
                 * this header your HTTP request is likely to fail/result in an undesired response.
                 */
                if (RETCODE_OK == retcode)
                {
                    rc = HttpMsg_setHost(httpMessage, config->DestinationServerUrl);
                    if (RC_OK != rc)
                    {
                        printf("HTTPRestClient_Get : unable to set HOST header. error=%d.\r\n", rc);
                        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_SET_HOST_FAILED);
                    }
                }
                /* We can only receive a certain amount of data per request. Thus we may have to make multiple
                 * HTTP GET requests to download a page/response completely. We set the Range HTTP header to signal
                 * the other end which part of the page we'd like to download.
                 */
                if (RETCODE_OK == retcode)
                {
                    HttpMsg_setRange(httpMessage, HTTPRestClientGetPageOffset, config->RequestMaxDownloadSize);

                    /* The rc of pushRequest does not tell us if the message was indeed sent, but rather if the
                     * push was initialized. To find out of the HTTP request was actually sent we have to check the
                     * callerStatus in the HttpRestClientRequestSentCallback function.
                     *
                     * When we receive a response from the server, the HttpRestClientGetResponseCallback function is called.
                     */
                    Callable_T httpRequestSentCallable;
                    (void) Callable_assign(&httpRequestSentCallable, HttpRestClientRequestSentCallback);
                    /* This is a dummy take. In case of any callback received
                     * after the previous timeout will be cleared here. */
                    (void) xSemaphoreTake(HTTPRestClientGetHandle, 0UL);
                    rc = HttpClient_pushRequest(httpMessage, &httpRequestSentCallable, HttpRestClientGetResponseCallback);
                    if (RC_OK != rc)
                    {
                        printf("HTTPRestClient_Get : unable to push the HTTP request. error=%d.\r\n", rc);
                        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_PUSH_REQUEST_FAILED);
                    }
                }
                if (RETCODE_OK == retcode)
                {
                    if (pdTRUE != xSemaphoreTake(HTTPRestClientGetHandle, pdMS_TO_TICKS(timeout)))
                    {
                        printf("HTTPRestClient_Get : Failed since Get CB was not received \r\n");
                        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_POST_CB_NOT_RECEIVED);
                    }
                }
            }while ((RETCODE_OK == retcode) && (true == HTTPRestClientGetInProgress));
        }
    }

    return retcode;

}

Retcode_T HTTPRestClient_GetFirmware(HTTPRestClient_Config_T * config, HTTPRestClient_Get_T * get, uint32_t timeout)
{
    Retcode_T retcode = RETCODE_OK;
    uint8_t urlBuffer[DOWNLOAD_URL_LENGTH] = { 0 };
    const char * url;
    uint32_t getFailTime = 0U;
    if( NULL == get )
    {
        return RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }

    if ( (NULL == config) || (NULL == get->GetCB) || (NULL == get->Url))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    if (RETCODE_OK == retcode)
    {
        if (NULL != get->GetCB)
        {
            retcode = HTTPRestClient_Get(config, get, HTTP_GET_TIME_OUT);
        }
        else
        {
            FirmwareStatus = FIRMWARE_DOWNLOAD_INPROGRESS;
            GetDataOffset = 0U;
            url = get->Url;
            snprintf((char *)UrlOffsetBuffer, sizeof(UrlOffsetBuffer), "/%ld/%ld/", FIRMWARE_DOWNLOAD_SIZE, GetDataOffset);
            (void)xSemaphoreTake(GetFirmwareHandle, pdMS_TO_TICKS(0));
            do
            {
                if (false == GetStatusFlag && FIRMWARE_DOWNLOAD_INPROGRESS == FirmwareStatus)
                {
                    memset(urlBuffer, '\0', sizeof(urlBuffer));
                    snprintf((char *)urlBuffer, sizeof(urlBuffer), "%s%s", (const char *)url, UrlOffsetBuffer);
                    get->Url = (const char *)urlBuffer;
                    retcode = HTTPRestClient_Get(config, get, HTTP_GET_TIME_OUT);
                    if (RETCODE_OK != retcode)
                    {
                        getFailTime += HTTP_GET_TIME_OUT;
                        if(getFailTime >= timeout )
                        {
                            printf("HTTPRestClient_GetFirmware : Get response timeout \r\n");
                            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_POST_CB_NOT_RECEIVED);
                            break;
                        }
                        Retcode_RaiseError(retcode);
                    }
                    else
                    {
                        getFailTime = 0U;
                        if (pdTRUE != xSemaphoreTake(GetFirmwareHandle, pdMS_TO_TICKS(HTTP_GET_PROCESS_DELAY)))
                        {
                            printf("HTTPRestClient_GetFirmware : Failed since StoreData not processed \r\n");
                            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_FIRMWARE_WRITE_ERROR);
                            break;
                        }
                    }
                }

                if (FIRMWARE_DOWNLOAD_COMPLETED == FirmwareStatus)
                {
                    printf("HTTPRestClient_GetFirmware : Firmware Download completed \r\n");
                }

            } while (FIRMWARE_DOWNLOAD_INPROGRESS == FirmwareStatus);
            if (FIRMWARE_CRC_FAIL == FirmwareStatus)
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_FIRMWARE_CRC_ERROR);
            }
            else if (FIRMWARE_WRITE_ERROR == FirmwareStatus)
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_HTTP_FIRMWARE_WRITE_ERROR);
            }
        }
    }
    return retcode;
}

#endif /* XDK_CONNECTIVITY_HTTPRESTCLIENT */

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

#include <Serval_Defines.h>
#if SERVAL_ENABLE_DTLS && SERVAL_TLS_MBEDTLS

#include "MbedTLS_internal.h"
#include "MbedTLS_Connection.h"

#include <Serval_Types.h>
#include <Serval_Udp.h>
#include <Serval_CommBuff.h>
#include <Serval_StructCtx.h>
#include <Serval_Callable.h>
#include <Serval_Security.h>
#include <Serval_Clock.h>
#include <Serval_Scheduler.h>

#include <Serval_Log.h>

#if SERVAL_ENABLE_DTLS_PARALLEL_HANDSHAKE
#define DTLS_HSBUF(conn) (conn)->ctx.dtls.handshakeBuffer
#endif

//Management
static inline void CallUpperLayerRetryFunction(MbedConnection_T* secureConnection, retcode_t status);
static inline retcode_t SetUpperLayerRetryFunction(MbedConnection_T* secureConnection, Callable_T* function);

static inline void UdpDtls_retrySendingLater(MbedConnection_T* secureConnection);

//Sending Path
static retcode_t dtlsSendingFunction(Callable_T *callable_ptr, retcode_t status);
static inline void dtlsOnSent(MbedConnection_T* secureConnection);
static inline void scheduleUpperLayerRetryCallback(MbedConnection_T* secureConnection);

//Receiving Path
static retcode_t receiveCallback(Callable_T *callable_ptr, retcode_t status);

// Ensure that the packet that we receive is a Handshake (0x16) which contains a Client Hello (0x01)
// Only for those packets should a new session be initialized
#define GET_DTLS_CONTENT_TYPE(buf)   ( (CommBuff_getLength(buf) > 0) ? CommBuff_getPayload(buf)[0] : -1 )
#define GET_DTLS_HANDSHAKE_TYPE(buf) ( (CommBuff_getLength(buf) > DTLS_HEADER_SIZE) ? CommBuff_getPayload(buf)[DTLS_HEADER_SIZE] : -1 )

#define CONTAINS_DTLS_CONTENT_TYPE(buf, type)   ( (GET_DTLS_CONTENT_TYPE(buf)) == (type) )
#define CONTAINS_DTLS_HANDSHAKE_TYPE(buf, type) ( (GET_DTLS_HANDSHAKE_TYPE(buf)) == (type) )

void DtlsHandshakeThread(void* param)
{
    MbedConnection_T *secureConnection = (MbedConnection_T *) param;
    int ret = 0;

    vTaskSuspend(NULL);
    while (1)
    {
        if (secureConnection->state != MBED_CONNECTION_ALLOCATED)
        {
            printf("DtlsHandshakeThread: state not in MBED_CONNECTION_ALLOCATED\r\n");
            vTaskSuspend(NULL);
        }

        while ((ret = mbedtls_ssl_handshake(&secureConnection->ctx.dtls.mbedCtx.context)) != 0)
        {
            if ((ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE))
            {
                printf("failed  ! mbedtls_ssl_handshake returned -0x%04x", -ret);
                /* Make state as error */
                printf("MyHandshake break \r\n");
                CallUpperLayerRetryFunction(secureConnection, RC_UDP_SOCKET_ERROR);
                MbedTLS_freeConnection(secureConnection);
                break;
            }

            vTaskDelay(20);

        }

        secureConnection->pending = MBED_PACKET_NONE;
        secureConnection->state = MBED_CONNECTION_ESTABLISHED;
        CallUpperLayerRetryFunction(secureConnection, RC_OK);
        vTaskSuspend(NULL);
    }
}

retcode_t MbedTlsDtlsHandshake(Callable_T *callable_ptr, retcode_t status)
{
    (void) callable_ptr;
    (void) status;
    /* Place holder for event driven DTLS handshake */
    assert(0); /* Not implemented, yet */
    return RC_PLATFORM_ERROR;
}

/* Function is called from mbedTls-lib to send data via tcp or udp
 */
int mbedtls_net_send(void *ctx, const unsigned char *buf, size_t n)
{
    retcode_t rc = RC_OK;

    MbedConnection_T* secureConnection = (MbedConnection_T*) ctx;
    MsgSendingCtx_T* sendCtx = &(secureConnection->msgSendingCtx);

    assert(NULL != secureConnection);

    /* no send buffer from application exists
     * --> data to send are for dtls handshake
     */
    bool isHandshake = false;
    if (MBED_CONNECTION_ESTABLISHED != secureConnection->state)
    {
        isHandshake = true;
    }

    LOG_DEBUG("mbedtls_net_send: isHandshake=%d state=%d\n",
            isHandshake, secureConnection->state);

    if (1 == secureConnection->inSend)
    {
        LOG_DEBUG("mbedtls_net_send: INSEND->return \r\n");
        return MBEDTLS_ERR_SSL_WANT_WRITE;
    }

    secureConnection->inSend = 1;

    /* in case of handshake data we have to provide a sending buffer
     * from the network layer
     */
    if (false != isHandshake)
    {
        rc = Udp_prepareForSending(sendCtx->conn.udp, sendCtx);

        switch (rc)
        {
            case RC_OK:
            break;
            case RC_UDP_OUT_OF_MEMORY:
            case RC_UDP_SOCKET_BUSY:
            /* continue with retry later */
            /* @todo - Call equalent of Sec_retryLater */
            return MBEDTLS_ERR_SSL_WANT_WRITE;
            default:
            /* @todo - Call equalent of Sec_disconnect */
            return MBEDTLS_ERR_SSL_WANT_WRITE; //@todo need to check this MBEDTLS_ERR_SSL_WANT_WRITE return
        }
    }

    /* mbedTls wants more bytes to send, as we have network buffer size
     * we reduce it to network buffsize, but this may result failty behavior.
     */
    if ((unsigned) n > CommBuff_getSize(sendCtx->buffer))
    {
        n = CommBuff_getSize(sendCtx->buffer);
    }

    /* copy data into network buffer and send it via udp */
    CommBuff_setLength(sendCtx->buffer, n);
    memcpy(CommBuff_getPayload(sendCtx->buffer), buf, n);

    rc = Udp_sendTo(sendCtx->conn.udp, &sendCtx->destAddr, sendCtx->destPort, sendCtx->buffer, &secureConnection->msgSent);

    /* when handshake data, we have to free the communication buffer.
     * in other case buffer is own of the application and we dont free it
     */
    if (false != isHandshake)
    {
        sendCtx->buffer = NULL;
    }

    if (rc != RC_OK)
    {
        return MBEDTLS_ERR_SSL_WANT_WRITE; //@todo need to check this MBEDTLS_ERR_SSL_WANT_WRITE return
    }

    LOG_DEBUG("mbedtls_net_send: ready\n");

    if ((MBED_CONNECTION_ALLOCATED == secureConnection->state))
    {
        /* @todo - enable the below once event driven DTLS SSL handshake is made event driven (by controlling proper mbedTLS timeout) */
        /* Scheduler_enqueue(&secureConnection->DtlsHandshakeFunc, RC_OK); */
    }
    return n;
}

/* Function is called from mbedTls-lib to receive data from tcp or udp
 */
int mbedtls_net_recv(void *ctx, unsigned char *buf, size_t n)
{
    bool haveMoreData = false;
    bool needMoreData = false;

    MbedConnection_T* secureConnection = (MbedConnection_T*) ctx;

    assert(secureConnection != NULL);

    if (-1 == secureConnection->rxReadPosition)
    {
        LOG_DEBUG("mbedtls_net_recv: data all processed -> return MBEDTLS_ERR_SSL_WANT_READ\n");
        return MBEDTLS_ERR_SSL_WANT_READ;
    }

    /* not data received till now -> interrupt mbedTls-negotiate */
    if (NULL == secureConnection->receiveBuffer)
    {
        LOG_DEBUG("mbedtls_net_recv: data not ready -> interrupt mbedTls\n");
        return MBEDTLS_ERR_SSL_WANT_READ;
    }
    LOG_DEBUG("mbedtls_net_recv: datalen=%d n=%d\n",
            CommBuff_getLength(secureConnection->receiveBuffer), n);
    LOG_DEBUG("mbedtls_net_recv: rxReadPosition=%d\n",
            secureConnection->rxReadPosition);

    LOG_DEBUG("mbedtls_net_recv: RESET monitorCounter\n");

    // copy message into mbedTls buffer
    char *ptr = &(CommBuff_getPayload(secureConnection->receiveBuffer))
    [secureConnection->rxReadPosition];
    uint16_t len = CommBuff_getLength(secureConnection->receiveBuffer);

    if (n >= (size_t)(len - secureConnection->rxReadPosition))
    {
        needMoreData = true;
        n = len - secureConnection->rxReadPosition;
    }

    memcpy(buf, ptr, n);

    /* if mbedTls do not consume all data -> remember read position */
    if (len > secureConnection->rxReadPosition + n)
    {
        haveMoreData = true;
        secureConnection->rxReadPosition += n;
    }
    else
    {
        /* ready with processing received data */
        secureConnection->rxReadPosition = -1;
    }

    /* cleanup Comm_Buff if connection not established */
    if (-1 == secureConnection->rxReadPosition
            && secureConnection->state != MBED_CONNECTION_ESTABLISHED)
    {
        /* free receive buffer if not for application */
        CommBuff_free(secureConnection->receiveBuffer);
        secureConnection->receiveBuffer = NULL;
    }

    LOG_DEBUG("mbedtls_net_recv: rxReadPosition=%d\n",
            secureConnection->rxReadPosition);
    LOG_DEBUG("mbedtls_net_recv: data -> mbedTls len=%d\n", n);

    // We got more data, So call process again.
    if (((needMoreData == true) || (haveMoreData == true)) && (secureConnection->state == MBED_CONNECTION_ALLOCATED))
    {
        needMoreData = false;
        haveMoreData = false;
        /* @todo - enable the below once event driven DTLS SSL handshake is made event driven (by controlling proper mbedTLS timeout) */
        /* Scheduler_enqueue(&secureConnection->DtlsHandshakeFunc, RC_OK); */
    }
    return n;
}

/*  Management specific functions
 */
static inline void CallUpperLayerRetryFunction(MbedConnection_T* secureConnection, retcode_t status)
{
    if ( NULL != secureConnection)
    {
        if (Callable_isAssigned(secureConnection->upperLayerRetryFunction))
        {
            Callable_callback(secureConnection->upperLayerRetryFunction, status);
        }
        secureConnection->upperLayerRetryFunction = NULL;
    }
}

static inline retcode_t SetUpperLayerRetryFunction(MbedConnection_T* secureConnection, Callable_T* function)
{
    if ( NULL == secureConnection->upperLayerRetryFunction)
    {
        /* remember sending later */
        secureConnection->upperLayerRetryFunction = function;
    }
    else if (secureConnection->upperLayerRetryFunction != function)
    {
        /* not already pending => overloaded */
        return RC_UDP_OVERLOADED;
    }
    return RC_OK;
}

static inline void UdpDtls_retrySendingLater(MbedConnection_T* secureConnection)
{
    retcode_t rc = Udp_retrySendingLater(secureConnection->msgSendingCtx.conn.udp,
            &secureConnection->msgSendingCtx);
    if (RC_OK != rc)
    {
        secureConnection->ctx.dtls.retryLaterFailed = TRUE;
    }
}

#if SERVAL_DTLS_FLIGHT_MAX_RETRIES > 0
#define DTLS_RESET_FLIGHT_TIMEOUT(secureConnection) secureConnection->ctx.dtls.flightTimeoutMillis = 0
#define DTLS_SET_FLIGHT_TIMEOUT(secureConnection)   Clock_getTimeMillis(&secureConnection->ctx.dtls.flightTimeoutMillis); \
                                                    secureConnection->ctx.dtls.flightTimeoutMillis += EscDtlsInt_GetRetransmissionTimeout(DTLS_HSCTX(secureConnection));
#else
#define DTLS_RESET_FLIGHT_TIMEOUT(secureConnection)
#define DTLS_SET_FLIGHT_TIMEOUT(secureConnection)
#endif

static retcode_t dtlsOnSentFunction(Callable_T *callable_ptr, retcode_t status)
{
    (void) status;

    MbedConnection_T* secureConnection = GET_CONTEXT_OF_MEMBER(MbedConnection_T, msgSent, callable_ptr);

    if (MBED_CONNECTION_FREE == secureConnection->state
            || MBED_CONNECTION_CLOSED == secureConnection->state)
    {
        return RC_UDP_SOCKET_ERROR;
    }

    if (MBED_PACKET_NONE != secureConnection->sendingPacket)
    {
        dtlsOnSent(secureConnection);
        if (MBED_CONNECTION_CLOSED == secureConnection->state)
        {
            MbedTLS_freeConnection(secureConnection);
        }
        else if (MBED_PACKET_NONE != secureConnection->pending)
        {
            UdpDtls_retrySendingLater(secureConnection);
        }
    }

    if (MBED_CONNECTION_ALLOCATED == secureConnection->state)
    {
        secureConnection->inSend = 0;
    }

    return RC_OK;
}

/*  Sending path
 */
/*
 * Called from the underlying PAL to send a packet.
 * This function doubles as the function for sending and as the onSent callback
 * Essentially, this function only does some basic error handling and then
 * determines if it should go into the callback path or the sending path
 * or both.
 */
static retcode_t dtlsSendingFunction(Callable_T *callable_ptr, retcode_t status)
{
    if (status != RC_OK)
    return status;

    retcode_t rc = RC_OK;
    MbedConnection_T* secureConnection = GET_CONTEXT_OF_MEMBER(MbedConnection_T, msgSendingCtx.sendingFunc, callable_ptr);

    //@todo - the next lines should probably be locked
    if (MBED_CONNECTION_FREE == secureConnection->state
            || MBED_CONNECTION_CLOSED == secureConnection->state)
    {
        return RC_UDP_SOCKET_ERROR;
    }

    if (MBED_PACKET_NONE == secureConnection->sendingPacket
            && MBED_PACKET_NONE != secureConnection->pending)
    {
        if (MBED_CONNECTION_ALLOCATED == secureConnection->state)
        {
            vTaskResume(secureConnection->DtlsHandshakeTaskHandle);
            /* @todo - enable the below once event driven DTLS SSL handshake is made event driven (by controlling proper mbedTLS timeout) */
            /* Scheduler_enqueue(&secureConnection->DtlsHandshakeFunc, RC_OK); */
        }
        else if(MBED_CONNECTION_CLOSING == secureConnection->state)
        {
            CallUpperLayerRetryFunction(secureConnection, RC_UDP_SOCKET_ERROR);
            secureConnection->state = MBED_CONNECTION_CLOSED;
            MbedTLS_freeConnection(secureConnection);
        }
        else
        {
            //        rc = dtlsSendPacket(secureConnection);
            //        if (MBED_CONNECTION_CLOSED == secureConnection->state)
            //        {
            //            MbedTLS_freeConnection(secureConnection);
            //        }
        }
    }

    return rc;
}

static inline void dtlsOnSent(MbedConnection_T* secureConnection)
{
    switch (secureConnection->sendingPacket)
    {
        case MBED_PACKET_NONE:
        assert(false);
        break;
        case MBED_PACKET_ALERT:
        if (secureConnection->error == 1)
        {
            //A send attempt is pending. Inform the upper layer that it is not going to happen.
            CallUpperLayerRetryFunction(secureConnection, RC_UDP_SOCKET_ERROR);
            /* fatal alert has already closed the session */
            MbedTLS_freeConnection(secureConnection);
        }
        break;
        case MBED_PACKET_HANDSHAKE:
        if (secureConnection->state == MBED_CONNECTION_ESTABLISHED)
        {
            LOG_INFO("Connection established\n");
            scheduleUpperLayerRetryCallback(secureConnection);
        }
        break;
        case MBED_PACKET_CLOSE:
        //A send attempt is pending. Inform the upper layer that it is not going to happen.
        CallUpperLayerRetryFunction(secureConnection, RC_UDP_SOCKET_ERROR);
        secureConnection->state = MBED_CONNECTION_CLOSED;
        break;

        default:
        assert(false);
    }

    secureConnection->sendingPacket = MBED_PACKET_NONE;
}

static inline void scheduleUpperLayerRetryCallback(MbedConnection_T* secureConnection)
{
    assert(NULL != secureConnection);
    assert(MBED_CONNECTION_ESTABLISHED == secureConnection->state);

    retcode_t rc;
    MsgSendingCtx_T* ctx;

    if (!Callable_isAssigned(secureConnection->upperLayerRetryFunction))
    {
        return;
    }

    ctx = GET_CONTEXT_OF_MEMBER(MsgSendingCtx_T, sendingFunc,
            secureConnection->upperLayerRetryFunction);
    secureConnection->upperLayerRetryFunction = NULL;

    rc = Udp_retrySendingLater(ctx->conn.udp, ctx);
    if (rc != RC_OK)
    {
        /* @todo - Handle upon "could not retry upper layer retry callback" */
    }
}

static void Sec_ReceiveData(MbedConnection_T*secureConnection, retcode_t status);
static void Sec_ReceiveData(MbedConnection_T*secureConnection, retcode_t status)
{
    LOG_DEBUG("Sec_ReceiveData:\n");
    CommBuff_T commBuff = secureConnection->receiveBuffer;
    /* decrypt received data for application */
    int readLen = 0;
    int n = 0;
    while (true)
    {
        if (-1 == secureConnection->rxReadPosition)
        {
            secureConnection->rxReadPosition = 0;
        }
        n = mbedtls_ssl_read(&secureConnection->ctx.dtls.mbedCtx.context,
                (unsigned char *) &(CommBuff_getPayload(commBuff))[readLen],
                CommBuff_getSize(commBuff) - readLen);
        if (n > 0)
        {
            readLen += n;
        }

        LOG_DEBUG("Sec_ReceiveData: mbedTls_read n=%d\n", n);
        LOG_DEBUG("Sec_ReceiveData: rxReadPosition=%d\n",
                secureConnection->rxReadPosition);
        LOG_DEBUG("Sec_ReceiveData: Payload=%d (%d)\n",
                CommBuff_getPayload(commBuff), 2*SERVAL_MAX_SIZE_APP_PACKET);
        if (-1 == secureConnection->rxReadPosition)
        {
            secureConnection->rxReadPosition = 0;
            break;
        }
        if (n <= 0)
        break;
    }
    LOG_DEBUG("Sec_ReceiveData: readLen=%d n=%d\n", readLen, n);

    /* valid data -> send to application */
    if (readLen > 0)
    {
        CommBuff_setLength(secureConnection->receiveBuffer,
                (uint16_t) readLen);
        if (readLen >= 2 * SERVAL_MAX_SIZE_APP_PACKET)
        {
            CommBuff_free(secureConnection->receiveBuffer);
            secureConnection->receiveBuffer = NULL;
            status = RC_TCP_OUT_OF_MEMORY;
            (void) status;
            /* @todo - use status and validate if "if (readLen >= 2 * SERVAL_MAX_SIZE_APP_PACKET)" condition check is needed */
        }
        if (CommBuff_isValid(secureConnection->receiveBuffer)
                && secureConnection->socket->upperLayerOnReceiveCallback != NULL)
        {
            if (NULL != secureConnection->socket->upperLayerOnReceiveCallback)
            {
                if (secureConnection->state == MBED_CONNECTION_ESTABLISHED)
                {
                    Callable_callback(
                            secureConnection->socket->upperLayerOnReceiveCallback, RC_OK);
                    /* For MQTT the socket is maintained for its lifetime. This is a dirty hack to clean its communication buffers */
                    CommBuff_free(secureConnection->receiveBuffer);
                    secureConnection->receiveBuffer = NULL;
                }
                else
                {
                    CommBuff_free(secureConnection->receiveBuffer);
                    secureConnection->receiveBuffer = NULL;
                }
            }
        }

        if (n <= 0)
        {
            /* @todo - Do socket and session closure */
            if (NULL != secureConnection->receiveBuffer)
            {
                CommBuff_free(secureConnection->receiveBuffer);
                secureConnection->receiveBuffer = NULL;
            }
            if ((n != MBEDTLS_ERR_SSL_WANT_READ) && (n != MBEDTLS_ERR_SSL_WANT_WRITE))
            {
                /* @todo - Perform Sec_disconnect equalent */
            }
        }
    }
}

/*  Receiving path
 */
static retcode_t receiveCallback(Callable_T *callable_ptr, retcode_t status)
{
    if (RC_OK != status)
    return status;

    retcode_t rc = RC_OK;
    Ip_Address_T peerAddr;
    Ip_Port_T peerPort;
    CommBuff_T recvBuf;
    MbedSocket_T* secureSocket;
    MbedConnection_T * secureConnection;

    secureSocket = GET_CONTEXT_OF_MEMBER(MbedSocket_T, internalOnReceiveCallback, callable_ptr);

    rc = Udp_receive(*secureSocket->socket.udp, &peerAddr, &peerPort, &recvBuf);
    if (RC_OK != rc)
    {
        return rc;
    }

    secureConnection = Dtls_getMbedConnection(secureSocket, (const Ip_Address_T*) &peerAddr, peerPort);
    if ( NULL == secureConnection)
    {
        rc = RC_DTLS_UNEXPECTED_PACKET;
        CommBuff_free(recvBuf);
        return rc;
    }

    if (CommBuff_isValid(secureConnection->receiveBuffer))
    {
        CommBuff_free(recvBuf);
        return RC_OK;
    }
    secureConnection->receiveBuffer = recvBuf;
#if SERVAL_ENABLE_DTLS_HEADER_LOGGING
    secureConnection->ctx.dtls.dtlsLength = CommBuff_getLength(recvBuf);
    memcpy(secureConnection->ctx.dtls.dtlsHeader, CommBuff_getPayload(recvBuf), secureConnection->ctx.dtls.dtlsLength < DTLS_LOG_BUFFER_SIZE ? secureConnection->ctx.dtls.dtlsLength : DTLS_LOG_BUFFER_SIZE);
#endif

    /* check if Handshake -> call mbedTls negotiate */
    if (secureConnection->state == MBED_CONNECTION_ALLOCATED)
    {
        LOG_DEBUG("Sec_receiveCB: Negotiate (%p)\n", secureConnection);

        if (-1 == secureConnection->rxReadPosition)
        {
            secureConnection->rxReadPosition = 0;
        }
        secureConnection->inSend = 0;
        // Process the received handshake data
        /* @todo - enable the below once event driven DTLS SSL handshake is made event driven (by controlling proper mbedTLS timeout) */
        /* Scheduler_enqueue(&secureConnection->DtlsHandshakeFunc, RC_OK); */
    }
    else if (secureConnection->state == MBED_CONNECTION_ESTABLISHED)
    {
        Sec_ReceiveData(secureConnection, rc);

    }
    else
    {
        CommBuff_free(secureConnection->receiveBuffer);
        secureConnection->receiveBuffer = CommBuff_getInvalidBuffer();
        if (MBED_CONNECTION_CLOSED == secureConnection->state)
        {
            MbedTLS_freeConnection(secureConnection);
        }
    }

    return rc;
}

/*  Initialization for server and client use
 */
void Dtls_initializeSecureSocket(MbedSocket_T* secureSocket, Udp_Socket_T *socket_ptr,
        Callable_T *callable_ptr)
{
    (void) Callable_assign(&(secureSocket->internalOnReceiveCallback), receiveCallback);
    secureSocket->socket.udp = socket_ptr;
    secureSocket->upperLayerOnReceiveCallback = callable_ptr;
}

retcode_t Dtls_initializeSecureConnection(MbedConnection_T** secureConnection,
        const MbedSocket_T* socket,
        const Ip_Address_T *peerAddr_ptr,
        const Ip_Port_T peerPort)
{
    *secureConnection = MbedTLS_newConnection(socket);
    if ( NULL == *secureConnection)
    {
        return RC_DTLS_NO_MORE_CONNECTIONS_FREE;
    }

    (void) Callable_assign(&(*secureConnection)->msgSent, dtlsOnSentFunction);
    (void) Callable_assign(&(*secureConnection)->msgSendingCtx.sendingFunc, dtlsSendingFunction);

    (*secureConnection)->msgSendingCtx.destPort = peerPort;
    Ip_copyAddr(peerAddr_ptr, &(*secureConnection)->msgSendingCtx.destAddr);

    //TODO It is an assumption that this works. It is never documented that the UdpSocket has to be copyable.
    (*secureConnection)->msgSendingCtx.conn.udp = *socket->socket.udp;

    return RC_OK;
}

/* PAL interface replacement implementation
 */
retcode_t Dtls_prepareForSending(Udp_Socket_T const socket, MsgSendingCtx_T *sendingCtx_ptr)
{
    MbedSocket_T* secureSocket = Dtls_getMbedSocket(socket);

    if ( NULL != secureSocket)
    {
        MbedConnection_T* secureConnection = Dtls_getMbedConnection(
                secureSocket, (const Ip_Address_T*) &sendingCtx_ptr->destAddr,
                sendingCtx_ptr->destPort);

        if ( NULL == secureConnection)
        {
            return RC_DTLS_NO_CONNECTION;
        }

        switch (secureConnection->state)
        {
            case MBED_CONNECTION_FREE:
            case MBED_CONNECTION_CLOSED:
            return RC_DTLS_NO_CONNECTION;

            case MBED_CONNECTION_ALLOCATED:
            return RC_UDP_SOCKET_BUSY;

            case MBED_CONNECTION_ESTABLISHED:
            break;

            case MBED_CONNECTION_CLOSING:
            return RC_UDP_SOCKET_ERROR;
            case MBED_CONNECTION_PEER_CLOSED:
            return RC_UDP_SOCKET_ERROR;

            default:
            return RC_DTLS_NO_CONNECTION;
        }
    }

    return Udp_prepareForSending(socket, sendingCtx_ptr);
}

retcode_t Dtls_retrySendingLater(Udp_Socket_T const socket, MsgSendingCtx_T *sendingCtx_ptr)
{
    assert(NULL != sendingCtx_ptr);
    assert(Callable_isAssigned(&sendingCtx_ptr->sendingFunc));

    MbedSocket_T* secureSocket = Dtls_getMbedSocket(socket);

    if ( NULL != secureSocket)
    {
        MbedConnection_T* secureConnection = Dtls_getMbedConnection(
                secureSocket, (const Ip_Address_T*) &sendingCtx_ptr->destAddr,
                sendingCtx_ptr->destPort);
        if ( NULL == secureConnection)
        {
            return RC_DTLS_NO_CONNECTION;
        }

        switch (secureConnection->state)
        {
            case MBED_CONNECTION_FREE:
            case MBED_CONNECTION_CLOSED:
            return RC_DTLS_NO_CONNECTION;

            case MBED_CONNECTION_ALLOCATED:
            return SetUpperLayerRetryFunction(secureConnection, &sendingCtx_ptr->sendingFunc);

            case MBED_CONNECTION_ESTABLISHED:
            break;

            case MBED_CONNECTION_CLOSING:
            return RC_UDP_SOCKET_ERROR;
            case MBED_CONNECTION_PEER_CLOSED:
            return RC_UDP_SOCKET_ERROR;

            default:
            return RC_DTLS_NO_CONNECTION;
        }
    }

    return Udp_retrySendingLater(socket, sendingCtx_ptr);
}

retcode_t Dtls_sendTo(Udp_Socket_T socket, Ip_Address_T *addr, Ip_Port_T port, CommBuff_T packet,
        Callable_T *callback)
{
	unsigned int commBufDataLen = 0;

    MbedSocket_T* secureSocket = Dtls_getMbedSocket(socket);

    if ( NULL != secureSocket)
    {
        MbedConnection_T* secureConnection = Dtls_getMbedConnection(
                secureSocket, (const Ip_Address_T*) addr, port);

        if ( NULL == secureConnection)
        {
            CommBuff_free(packet);
            return RC_DTLS_NO_CONNECTION;
        }

        secureConnection->msgSendingCtx.buffer = packet;

        commBufDataLen = CommBuff_getLength(packet);
        int n = mbedtls_ssl_write(&secureConnection->ctx.dtls.mbedCtx.context,
                (unsigned char *) CommBuff_getPayload(packet),
				commBufDataLen);
        secureConnection->inSend = 0;

        if (n <= 0)
        {
            CommBuff_free(packet);
            Callable_callback(callback, RC_DTLS_OVERLOADED);
            return (RC_DTLS_OVERLOADED);
        } /* @todo - Check if the number of bytes actually sent is lesser than the size that the application wanted */

        else if((unsigned int)n < commBufDataLen)
        {
            CallUpperLayerRetryFunction(secureConnection, RC_UDP_SOCKET_ERROR);
            MbedTLS_freeConnection(secureConnection);
            return (RC_OK);
        }
        else
        {
            CommBuff_free(packet);
            Callable_callback(callback, RC_OK);
            return (RC_OK);
        }
    }

    return Udp_sendTo(socket, addr, port, packet, callback);
}

retcode_t Dtls_receive(Udp_Socket_T socket, Ip_Address_T *ipAddr_ptr, Ip_Port_T *port_ptr,
        CommBuff_T *packet_ptr)
{
    retcode_t rc = RC_OK;
    MbedSocket_T* secureSocket = Dtls_getMbedSocket(socket);

    if ( NULL == secureSocket)
    {
        return Udp_receive(socket, ipAddr_ptr, port_ptr, packet_ptr);
    }

    rc = Dtls_getFirstPendingDataPacket(secureSocket, ipAddr_ptr, port_ptr, packet_ptr);
    if (RC_OK != rc)
    {
        *packet_ptr = CommBuff_getInvalidBuffer();
        *port_ptr = 0;
        Ip_copyAddr(Ip_getUnspecifiedAddr(), ipAddr_ptr);
    }

    return rc;
}

retcode_t Dtls_delete(Udp_Socket_T socket)
{
	LOG_DEBUG("Dtls_delete: Entry \r\n");
    retcode_t rc = RC_OK;
    MbedSocket_T* secureSocket = Dtls_getMbedSocket(socket);

    if (secureSocket != NULL && MbedTLS_isAnyConnectionOpen())
    return RC_UDP_SOCKET_ERROR;

    rc = Udp_delete(socket);

    if (secureSocket != NULL)
    {
    	LOG_DEBUG("Dtls_delete: MbedTLS_freeSocket \r\n");
        /* We must ensure that this is called after the underlying socket is deleted.
         * otherwise, the plain socket would still be allowed to be used for communication
         * while the secure socket part is already cleaned up.
         */
        MbedTLS_freeSocket(secureSocket);
    }

    return rc;
}

/* PAL DTLS implementation
 */
retcode_t Udp_getSecureConnState(Udp_Socket_T const socket, Ip_Address_T *peerAddr_ptr,
        Ip_Port_T peerPort, SecureConnectionState_T *state)
{
    MbedSocket_T* secureSocket = Dtls_getMbedSocket(socket);
    MbedConnection_T* secureConnection = NULL;

    if ( NULL == secureSocket)
    {
        return RC_DTLS_PLAIN_SOCKET;
    }

    secureConnection = Dtls_getMbedConnection(secureSocket, (const Ip_Address_T*) peerAddr_ptr,
            peerPort);
    if ( NULL == secureConnection)
    {
        return RC_DTLS_NO_CONNECTION;
    }

    if (secureConnection->state == MBED_CONNECTION_PEER_CLOSED)
    {
        *state = SECURE_STATE_HALFCLOSED;
    }
    else
    {
        *state = (SecureConnectionState_T) secureConnection->state;
    }

    return RC_OK;
}

retcode_t Udp_getSecureConnError(Udp_Socket_T const socket, Ip_Address_T *destAddr_ptr,
        Ip_Port_T destPort, retcode_t *rc_ptr)
{
    MbedSocket_T* secureSocket = Dtls_getMbedSocket(socket);
    MbedConnection_T* secureConnection = NULL;

    if ( NULL == secureSocket)
    {
        return RC_DTLS_PLAIN_SOCKET;
    }

    secureConnection = Dtls_getMbedConnection(secureSocket, (const Ip_Address_T*) destAddr_ptr,
            destPort);
    if (secureConnection == NULL)
    {
        return RC_DTLS_NO_CONNECTION;
    }

    if (secureConnection->error == true)
    {
        *rc_ptr = RC_UDP_SOCKET_ERROR;
    }
    else
    {
        *rc_ptr = RC_OK;
    }

    return RC_OK;
}

retcode_t Udp_closeSecureConn(Udp_Socket_T const socket, Ip_Address_T *peerAddr_ptr,
        Ip_Port_T peerPort)
{
	LOG_DEBUG("Udp_closeSecureConn: Entry \r\n");
    MbedSocket_T* secureSocket = Dtls_getMbedSocket(socket);
    MbedConnection_T* secureConnection = NULL;

    if ( NULL == secureSocket)
    {
    	LOG_DEBUG("Udp_closeSecureConn: Invalid socket \r\n");
        return RC_DTLS_PLAIN_SOCKET;
    }

    secureConnection = Dtls_getMbedConnection(secureSocket, (const Ip_Address_T*) peerAddr_ptr,
            peerPort);
    if ( NULL == secureConnection)
    {
    	LOG_DEBUG("Udp_closeSecureConn: Invalid connection \r\n");
        return RC_DTLS_NO_CONNECTION;
    }

    if (secureConnection->error)
    {
        /* @todo - Log Error */
    }
    else
    {
    	LOG_DEBUG("Udp_closeSecureConn: closing connection \r\n");
        secureConnection->state = MBED_CONNECTION_CLOSING;
        secureConnection->pending = MBED_PACKET_CLOSE;
        UdpDtls_retrySendingLater(secureConnection);
    }
    return RC_OK;
}

#endif /* SERVAL_ENABLE_DTLS && SERVAL_TLS_MBEDTLS */

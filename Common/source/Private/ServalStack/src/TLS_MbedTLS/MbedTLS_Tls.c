/*----------------------------------------------------------------------------*/
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
/*
 * Copyright (C) Bosch Connected Devices and Solutions GmbH.
 * All Rights Reserved. Confidential.
 *
 * Distribution only to people who need to know this information in
 * order to do their job.(Need-to-know principle).
 * Distribution to persons outside the company, only if these persons
 * signed a non-disclosure agreement.
 * Electronic transmission, e.g. via electronic mail, must be made in
 * encrypted form.
 */
/*----------------------------------------------------------------------------*/

#include <Serval_Defines.h>

#if SERVAL_ENABLE_TLS && SERVAL_TLS_MBEDTLS

#include "MbedTLS_Tls.h"

#include <ResourceMonitor.h>

#include <Serval_Log.h>
#include <Serval_Timer.h>
#include <Serval_Thread.h>
#include <Serval_Scheduler.h>
#include <Serval_Callable.h>
#include "XDK_WLAN.h"

#define LOG_MODULE 		"SSL"

static SemaphoreHandle_t TlsHandshakeBufferMutex;

/* struct for secure socket context */
struct SecSocketCtx_S
{
    /* 0: free, 1: used */
    uint8_t inUse :1;
    /* 0: Udp/DTSL, 1: Tcp/TSL */
    uint8_t isTcp :1;
    /* remember socket status for tcp */
    uint8_t socketStatus :4;
    /* union holding UDP/TCP socket */
    union
    {
        Udp_Socket_T udp;
        Tcp_Listener_T tcp_listen;
        Tcp_Socket_T tcp;
    } socket;
    /* adapter internal used socket callback */
    Callable_T socketCb;
    /* remebered uppler layer socket callback */
    Callable_T *upperLayerSocketCb;
};
typedef struct SecSocketCtx_S SecSocketCtx_T;

struct SecConnectionCtx_S
{
    SecureConnectionState_T connState :4;
    /* 0: Udp/DTSL, 1: Tcp/TSL */
    uint8_t isTcp :1;
    /* remembere wantsend from mbedTls - lib */
    uint8_t wantSend :1;
    /* marker if insend -> protect recursiv calls */
    uint8_t inSend :1;
    /* marker if enableMultiInSend -> enables recursiv inSend calls */
    uint8_t enableMultiInSend :1;
    /* want send callback */
    int8_t countSendingCB :2;
    /* connection is under adapter control (for server: becomes under upper
     * controll when first data are sent to the upper layer
     */
    /* retry-callback is active */
    int8_t countRetryCB :3;
    uint8_t manage :1;
    /* timeout counter for pending negotiation */
    uint8_t monitorCounter;
    /* position marker if mbedTls dont read complete receive buffer */
    int16_t mbedTlsReadPosition;
    /* remembered last error happened on this connection */
    retcode_t lastError;
    /* holds data for tcp/udp sending functions */
    struct MsgSendingCtx_S sCtx;
    /* saved sendingCtx_ptr->sendingFunc from Udp_prepareForSending */
    Callable_T *upperLayerSendCB_ptr;
    /* saved sendingCtx_ptr->sendingFunc from UdpPlain_retrySendingLater */
    Callable_T *upperLayerRetryCB_ptr;
    /* adapter internal send callback function */
    Callable_T sending_Callable;
    /* saved buffer from receive callback */
    CommBuff_T receiveBuffer;
    /* pointer for mbedTls connection context */
    void *mbedTlsConnectionCtx_ptr;
    /* saved buffer for tcp retransmission*/
    CommBuff_T tcpRetransmissionBuffer;
    /* Pointer to socket context */
    SecSocketCtx_T *secSocketCtx_ptr;
    /* Tls Handshake callable func*/
    Callable_T TlsHandshakeFunc;
};

typedef struct SecConnectionCtx_S SecConnectionCtx_T;

/* List of connection contextes */
static SecConnectionCtx_T sec_conn[SERVAL_MAX_SECURE_CONNECTIONS];

/* List of socket contextes */
static SecSocketCtx_T sec_sockets[SERVAL_MAX_SECURE_SOCKETS];

/* periodic tmer for monitor */
static Timer_T monitorTimer;


#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"
#include "mbedtls/platform.h"

extern mbedtls_ssl_context ssl;

/* This function initialize the module and the mbedTls library
 */
static void Sec_initialize(void);

/* This function creates a new secure socket Context
 */
static SecSocketCtx_T *Sec_newSocketCtx(void);

/* This function search a socket Context by its Tls listener Socket
 */
static SecSocketCtx_T *Tls_getListenerCtx(
        Tcp_Listener_T tcpListener);

/* This function search a socket Context by its Tls  Socket
 */
static SecSocketCtx_T *Tls_getSocketCtx(
        Tcp_Socket_T tcpSocket);

/* This function search a connection Context by tcp socket
 */
static SecConnectionCtx_T *Tls_getConnectionCtx(
        Tcp_Socket_T const socket);

/* This function search a socket Context by its receive Callable
 */
static SecSocketCtx_T *Sec_getSocketCtx_byCallable(
        Callable_T *socketCb_ptr);

/* This function creates a new secure connection Context
 */
static SecConnectionCtx_T *Sec_newConnectionCtx(
        Udp_Socket_T *udp_socket,
        Tcp_Socket_T *tcp_socket,
        Ip_Address_T *peerAddr_ptr,
        Ip_Port_T peerPort);

/* This function search a connection Context by its retry-for-sending
 * callable
 */
static SecConnectionCtx_T *Sec_getConnectionCtx_retry_Callable(
        Callable_T *retry_Callable_ptr);

/* This function search a connection Context by its sending callable
 */
static SecConnectionCtx_T *Sec_getConnectionCtx_sending_Callable(
        Callable_T *sending_Callable_ptr);

/* This function does the server connection clean up
 */
static retcode_t SubMonitorFunc(Timer_T *timer_ptr);

/* This function handles the retry for sending during the
 * handshakes.
 */
static retcode_t Sec_retryCB(Callable_T *callable_ptr, retcode_t status);

/* Function is called from mbedTls-lib to receive data from tcp or udp
 */
int mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len);

/* Function is called from mbedTls-lib to send data via tcp or udp
 */
int mbedtls_net_send(void *ctx, const unsigned char *buf, size_t n);

/* Callbackfunction for Udp/Tcp_send
 */
static retcode_t Sec_sendingCB(Callable_T *callable_ptr, retcode_t status);

/* Receive-Callbackfunction received data from Udp/Tcp socket
 */
static retcode_t Sec_receiveCB(Callable_T *callable_ptr, retcode_t status);

/* Internal function to start disconnect of secure connections
 */
static void Sec_disconnect(SecConnectionCtx_T *secConnectionCtx_ptr);

/* Internal function for prcessing shutdown(teardown) of secure connections
 */
static void Sec_shutdown(SecConnectionCtx_T *secConnectionCtx_ptr);

/* Internal function for prepare for sending sending later in the security layer
 */
static retcode_t Sec_prepareForSending(SecConnectionCtx_T *secConnectionCtx_ptr);

/* Internal function for sending in the security layer
 */
static retcode_t Sec_sendTo(SecConnectionCtx_T *secConnectionCtx_ptr,
        Callable_T *sendCB_ptr);

/* Internal function for retry sending later in the security layer
 */
static retcode_t Sec_retryLater(SecConnectionCtx_T *secConnectionCtx_ptr);

/* Internal function for scheduling jobs in the security layer
 */
static void Sec_scheduleToNetwork(SecConnectionCtx_T *secConnectionCtx_ptr);

/* Internal function for read received data in the security layer
 */
static void Sec_ReceiveData(SecConnectionCtx_T *secConnectionCtx_ptr,
        retcode_t status);

/* Internal function for freeinng secure contexts
 */
static void Sec_FreeCtx(SecConnectionCtx_T *secConnectionCtx_ptr);

void Sec_upperLayerSendCB(SecConnectionCtx_T *secConnectionCtx_ptr,
        retcode_t rc);
void Sec_upperLayerRetryCB(SecConnectionCtx_T *secConnectionCtx_ptr,
        retcode_t rc);

static int _real_confirm(int verify_result)
{
    if ((verify_result & MBEDTLS_X509_BADCERT_EXPIRED) != 0)
    {
        // printf("! fail ! ERROR_CERTIFICATE_EXPIRED");
        return -1;
    }

    if ((verify_result & MBEDTLS_X509_BADCERT_REVOKED) != 0)
    {
        // printf("! fail ! server certificate has been revoked");
        return -1;
    }

    if ((verify_result & MBEDTLS_X509_BADCERT_NOT_TRUSTED) != 0)
    {
        // printf("! fail ! self-signed or not signed by a trusted CA");
        return -1;
    }

#if 0
    if ((verify_result & MBEDTLS_X509_BADCERT_CN_MISMATCH) != 0)
    {
        // printf("! fail ! CN mismatch");
        return -1;
    }
#endif
    return 0;
}

static retcode_t TlsHandshake(Callable_T *callable_ptr, retcode_t status)
{
    (void) status;
    SecConnectionCtx_T *secConnectionCtx_ptr = CALLABLE_GET_CONTEXT(struct SecConnectionCtx_S, TlsHandshakeFunc, callable_ptr);

    int ret = 0;
    retcode_t rc = RC_OK;

    if (secConnectionCtx_ptr->connState != SECURE_STATE_IN_NEGOTIATION)
    {
        return rc;
    }

    secConnectionCtx_ptr->enableMultiInSend = true;
    if (pdFALSE == xSemaphoreTake(TlsHandshakeBufferMutex, (TickType_t) UINT8_C(20)))
    {
        Scheduler_enqueue(&secConnectionCtx_ptr->TlsHandshakeFunc, RC_OK);
        return rc;
    }

    ret = mbedtls_ssl_handshake(&ssl);
    if (ret == MBEDTLS_ERR_SSL_WANT_WRITE)
    {
        ; /* Do Nothing, app will retry */
    }
    else if (ret == MBEDTLS_ERR_SSL_WANT_READ)
    {
        ; /* Do Nothing, wait for data */
        //Do nothing
    }
    else if (ret != 0)
    {
        secConnectionCtx_ptr->manage = 1;
        /* Make state as error */
        rc = RC_OK;
    }
    else
    {
        if (0 != (ret = _real_confirm(mbedtls_ssl_get_verify_result(&ssl))))
        {
#if XDK_MBEDTLS_PARSE_INFO==1
            secConnectionCtx_ptr->manage = 1;
#endif
        }
        else
        {
#if XDK_MBEDTLS_PARSE_INFO==1
            secConnectionCtx_ptr->enableMultiInSend = false;
            secConnectionCtx_ptr->connState = SECURE_STATE_ESTABLISHED;
            Sec_upperLayerRetryCB(secConnectionCtx_ptr, RC_OK);
#endif
        }
#if XDK_MBEDTLS_PARSE_INFO==0
        secConnectionCtx_ptr->enableMultiInSend = false;
        secConnectionCtx_ptr->connState = SECURE_STATE_ESTABLISHED;
        Sec_upperLayerRetryCB(secConnectionCtx_ptr, RC_OK);
#endif
    }
    if (pdFALSE == xSemaphoreGive(TlsHandshakeBufferMutex))
    {
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR));
    }
    return rc;
}

retcode_t Tls_SecureClient_initialize(void)
{
    retcode_t rc = RC_OK;
    Sec_initialize();

    TlsHandshakeBufferMutex = xSemaphoreCreateMutex();

    if (NULL == TlsHandshakeBufferMutex)
    {
        rc = RC_PLATFORM_ERROR;
    }
    return rc;
}

/* This function initialize the module and the mbedTls library
 */
static int Sec_inited = 0;
void Sec_initialize(void)
{
    if (Sec_inited == 1)
    {
        return;
    }
    Sec_inited = 1;
    memset(sec_conn, 0, sizeof(sec_conn));
    memset(sec_sockets, 0, sizeof(sec_sockets));

    Timer_init(&monitorTimer);
    Timer_config(&monitorTimer, (unsigned int) 2000, &SubMonitorFunc);
    Timer_start(&monitorTimer);
}

/* This function creates a new secure socket Context
 */
static SecSocketCtx_T *Sec_newSocketCtx(void)
{
    int index;
    for (index = 0; index < SERVAL_MAX_SECURE_SOCKETS; index++)
    {
        if (0 == sec_sockets[index].inUse)
        {
            memset(&sec_sockets[index], 0, sizeof(SecSocketCtx_T));
            sec_sockets[index].inUse = 1;
            return &sec_sockets[index];
        }
    }
    LOG_DEBUG("Sec_newSocketCtx: No free socket!!!\n");
    return NULL;
}

static SecSocketCtx_T *Tls_getListenerCtx(
        Tcp_Listener_T tcpListener)
{
    int index;
    for (index = 0; index < SERVAL_MAX_SECURE_SOCKETS; index++)
    {
        if (sec_sockets[index].isTcp == 0)
            continue;
        if (0 == memcmp(&tcpListener, &sec_sockets[index].socket.tcp_listen,
                sizeof(Tcp_Listener_T)))
        {
            return &sec_sockets[index];
        }
    }
    return NULL;
}

static SecSocketCtx_T *Tls_getSocketCtx(
        Tcp_Socket_T tcpSocket)
{
    int index;
    for (index = 0; index < SERVAL_MAX_SECURE_SOCKETS; index++)
    {
        if (sec_sockets[index].isTcp == 0)
            continue;
        if (0 == memcmp(&tcpSocket, &sec_sockets[index].socket.tcp,
                sizeof(Tcp_Socket_T)))
        {
            return &sec_sockets[index];
        }
    }
    return NULL;
}

/* This function search a connection Context by tcp socket
 */
static SecConnectionCtx_T *Tls_getConnectionCtx(
        Tcp_Socket_T const socket)
{
    int index;
    for (index = 0; index < SERVAL_MAX_SECURE_CONNECTIONS; index++)
    {
        if (SECURE_STATE_FREE == sec_conn[index].connState)
            continue;
        if (0 == sec_conn[index].isTcp)
            continue;
        if (0 != memcmp(&socket, &sec_conn[index].sCtx.conn.tcp,
                sizeof(Tcp_Socket_T)))
            continue;
        return &sec_conn[index];
    }
    LOG_DEBUG("Tls_getConnectionCtx: not CTX found\n");
    return NULL;
}

/* This function search a socket Context by its receive Callable
 */
static SecSocketCtx_T *Sec_getSocketCtx_byCallable(
        Callable_T *socketCb_ptr)
{
    int index;
    for (index = 0; index < SERVAL_MAX_SECURE_SOCKETS; index++)
    {
        if (socketCb_ptr == &sec_sockets[index].socketCb)
            return &sec_sockets[index];
    }
    return NULL;
}

/* this function creates a new Connection context entry */
static SecConnectionCtx_T *Sec_newConnectionCtx(
        Udp_Socket_T *udp_socket,
        Tcp_Socket_T *tcp_socket,
        Ip_Address_T *peerAddr_ptr,
        Ip_Port_T peerPort)
{
    int index;
    for (index = 0; index < SERVAL_MAX_SECURE_CONNECTIONS; index++)
    {
        if (SECURE_STATE_FREE == sec_conn[index].connState)
            break;
    }
    /* no free connection available */
    if (index >= SERVAL_MAX_SECURE_CONNECTIONS)
    {
        return NULL;
    }

    SecConnectionCtx_T *secConnectionCtx_ptr = &sec_conn[index];
    memset(secConnectionCtx_ptr, 0, sizeof(SecConnectionCtx_T));
    Callable_assign(&secConnectionCtx_ptr->TlsHandshakeFunc, &TlsHandshake);
    secConnectionCtx_ptr->connState = SECURE_STATE_IN_NEGOTIATION;
    if (NULL != udp_socket)
    {
        secConnectionCtx_ptr->sCtx.conn.udp = *udp_socket;
    }
    else if (NULL != tcp_socket)
    {
        secConnectionCtx_ptr->sCtx.conn.tcp = *tcp_socket;
        secConnectionCtx_ptr->isTcp = 1;
    }

    if (NULL != peerAddr_ptr)
    {
        Ip_copyAddr((const Ip_Address_T *) peerAddr_ptr,
                (Ip_Address_T *) &secConnectionCtx_ptr->sCtx.destAddr);
    }
    secConnectionCtx_ptr->sCtx.destPort = peerPort;
    Callable_assign(&secConnectionCtx_ptr->sending_Callable,
            &Sec_sendingCB);
    Callable_assign(&secConnectionCtx_ptr->sCtx.sendingFunc,
            &Sec_retryCB);
    return secConnectionCtx_ptr;
}

/* This function search a connection Context by its restry-for-sending
 * callable
 */
static SecConnectionCtx_T *Sec_getConnectionCtx_retry_Callable(
        Callable_T *retry_Callable_ptr)
{
    int index;
    for (index = 0; index < SERVAL_MAX_SECURE_CONNECTIONS; index++)
    {
        if (retry_Callable_ptr == &sec_conn[index].sCtx.sendingFunc)
            return &sec_conn[index];
    }
    return NULL;
}

/* This function search a connection Context by its sending callable
 */
static SecConnectionCtx_T *Sec_getConnectionCtx_sending_Callable(
        Callable_T *sending_Callable_ptr)
{
    int index;
    for (index = 0; index < SERVAL_MAX_SECURE_CONNECTIONS; index++)
    {
        if (sending_Callable_ptr == &sec_conn[index].sending_Callable)
            return &sec_conn[index];
    }
    return NULL;
}

/* Monitor for supervising negotiation and shutdown of connections,
 * it is calls periodical */
static retcode_t SubMonitorFunc(Timer_T *timer_ptr UNUSED)
{
    int index;
    for (index = 0; index < SERVAL_MAX_SECURE_CONNECTIONS; index++)
    {
        SecConnectionCtx_T *secConnectionCtx_ptr = &sec_conn[index];

        if ((SECURE_STATE_IN_NEGOTIATION == secConnectionCtx_ptr->connState) && (true == secConnectionCtx_ptr->manage))
        {
            /* try disconnect if timeout */
            if (secConnectionCtx_ptr->monitorCounter == 0)
            {
                secConnectionCtx_ptr->monitorCounter = 1;
            }
            /* hard shutdown if no communication ongoing */
            else
            {
                LOG_DEBUG("SecMonitor: Hard Shutdown!!!\n");
                /* free the secure connection context */
                secConnectionCtx_ptr->connState = SECURE_STATE_CLOSING;
                Sec_disconnect(secConnectionCtx_ptr);
            }
        }
        else if (SECURE_STATE_HALFCLOSED == secConnectionCtx_ptr->connState)
        {
            /* try disconnect if timeout */
            if (secConnectionCtx_ptr->monitorCounter == 0)
            {
                secConnectionCtx_ptr->monitorCounter = 1;
            }
            /* hard shutdown if no communication ongoing */
            else
            {
                LOG_DEBUG("SecMonitor: Hard Shutdown!!!\n");
                /* free the secure connection context */
                secConnectionCtx_ptr->connState = SECURE_STATE_CLOSING;
                Sec_shutdown(secConnectionCtx_ptr);
            }
        }
        else if (SECURE_STATE_CLOSING == secConnectionCtx_ptr->connState)
        {
            if (secConnectionCtx_ptr->monitorCounter == 0)
            {
                secConnectionCtx_ptr->monitorCounter = 1;
            }
            else
            {
                LOG_DEBUG("SecMonitor: FREE-CTX index=%d\n", index);
                Sec_shutdown(secConnectionCtx_ptr);
            }
        }
    }

    return RC_OK;
}

/* This function handles the retry for sending during the
 * handshakes.
 */
static retcode_t Sec_retryCB(Callable_T *callable_ptr, retcode_t status)
{
    SecConnectionCtx_T *secConnectionCtx_ptr =
            Sec_getConnectionCtx_retry_Callable(callable_ptr);
    assert(NULL != secConnectionCtx_ptr);
    if (secConnectionCtx_ptr->connState == SECURE_STATE_FREE)
    {
        return RC_OK;
    }

    secConnectionCtx_ptr->countRetryCB--;

    if (status != RC_OK)
    {
        Sec_disconnect(secConnectionCtx_ptr);
        return RC_OK;
    }

    switch (secConnectionCtx_ptr->connState)
    {
    case SECURE_STATE_IN_NEGOTIATION:
        Scheduler_enqueue(&secConnectionCtx_ptr->TlsHandshakeFunc, RC_OK);
        secConnectionCtx_ptr->wantSend = 0;
        break;

    case SECURE_STATE_ESTABLISHED:
        break;

    case SECURE_STATE_HALFCLOSED:
        /* Prepare the mbedTls-contextes and call mbedTls-shutdown */
        Sec_shutdown(secConnectionCtx_ptr);
        break;
    case SECURE_STATE_CLOSING:
        if (0 < secConnectionCtx_ptr->countSendingCB)
        {
            break;
        }
        if (secConnectionCtx_ptr->isTcp)
        {
            Tcp_SocketStatus_T stat;
            Tcp_getSocketStatus(
                    secConnectionCtx_ptr->sCtx.conn.tcp, &stat);
            if (stat == TCP_SOCKET_STATUS_CLOSED)
            {
                ;
            }
            else
            {
                Sec_shutdown(secConnectionCtx_ptr);
                return RC_OK;
            }
        }

        if (secConnectionCtx_ptr->countRetryCB <= 0)
        {
            Sec_FreeCtx(secConnectionCtx_ptr);
        }
        break;

    default:
        break;
    }

    return RC_OK;
}

/* Internal function to start disconnect of secure connections
 */
static void Sec_disconnect(SecConnectionCtx_T *secConnectionCtx_ptr)
{
    LOG_DEBUG("Sec_disconnect: Sec_shutdown\n");
    assert(NULL != secConnectionCtx_ptr);
    secConnectionCtx_ptr->sCtx.buffer = NULL;
    if (secConnectionCtx_ptr->connState == SECURE_STATE_FREE)
        return;
    if (secConnectionCtx_ptr->connState != SECURE_STATE_CLOSING)
    {
        secConnectionCtx_ptr->connState = SECURE_STATE_HALFCLOSED;
        Sec_upperLayerRetryCB(secConnectionCtx_ptr, RC_DTLS_OVERLOADED);
        Sec_upperLayerSendCB(secConnectionCtx_ptr, RC_OK);
        secConnectionCtx_ptr->countRetryCB = 0;
    }

    /* Shutdown mbedTls */
    Sec_shutdown(secConnectionCtx_ptr);

}

/* Internal function for prcessing shutdown(teardown) of secure connections
 */
static void Sec_shutdown(SecConnectionCtx_T *secConnectionCtx_ptr)
{
    int ret = 0;
    Tcp_SocketStatus_T stat;

    LOG_DEBUG("Sec_shutdown: secConnectionCtx_ptr=%p\n", secConnectionCtx_ptr);
    LOG_DEBUG("Sec_shutdown: connState=%d\n", secConnectionCtx_ptr->connState);

    if (secConnectionCtx_ptr->connState == SECURE_STATE_FREE)
        return;

    if (0 != secConnectionCtx_ptr->isTcp)
    {
        retcode_t rc = Tcp_getSocketStatus(secConnectionCtx_ptr->sCtx.conn.tcp,
                &stat);
        LOG_DEBUG("Sec_shutdown: Socket stat=%d rc=%d\n", stat, rc);
        if (rc != RC_OK || stat != TCP_SOCKET_STATUS_OPEN)
        {
            LOG_DEBUG("Sec_shutdown: direct close, no shutdown\n", stat);
            ret = 1;
        }
    }

    if ((ret != 1) && (1 == secConnectionCtx_ptr->isTcp))
    {
        ret = mbedtls_ssl_close_notify(&ssl);
        secConnectionCtx_ptr->inSend = 0;
    }

    if (0 != secConnectionCtx_ptr->isTcp)
    {

        if ((secConnectionCtx_ptr->countSendingCB == 0) || (ret == 0))
        {
            if (stat == TCP_SOCKET_STATUS_OPEN
                    || stat == TCP_SOCKET_STATUS_HALF_OPEN
                    || stat == TCP_SOCKET_STATUS_CLOSING)
            {
                LOG_DEBUG("Sec_shutdown: Tcp_close socket=%p\n",
                        secConnectionCtx_ptr->sCtx.conn.tcp);
                Tcp_close(secConnectionCtx_ptr->sCtx.conn.tcp);
                return;
            }
            else
            {
                secConnectionCtx_ptr->isTcp = 0;
            }
        }
    }
    if (1 == secConnectionCtx_ptr->inSend
            || secConnectionCtx_ptr->countSendingCB > 0)
    {
        LOG_DEBUG("Sec_shutdown: inSend->retry\n");
        secConnectionCtx_ptr->connState = SECURE_STATE_CLOSING;
        if (secConnectionCtx_ptr->countSendingCB == 0)
        {
            LOG_DEBUG("Sec_shutdown: inSend->retry\n");
            Sec_scheduleToNetwork(secConnectionCtx_ptr);
        }
    }
    else if (0 == secConnectionCtx_ptr->isTcp)
    {
        if (secConnectionCtx_ptr->countRetryCB <= 0)
        {
            Sec_FreeCtx(secConnectionCtx_ptr);
        }
    }
}

/* Function is called from mbedTls-lib to send data via tcp or udp
 */
int mbedtls_net_send(void *ctx, const unsigned char *buf, size_t n)
{
    retcode_t rc = RC_OK;

    SecConnectionCtx_T *secConnectionCtx_ptr = (SecConnectionCtx_T*) ctx;

    assert(NULL != secConnectionCtx_ptr);

    /* no send buffer from application exists
     * --> data to send are for dtls handshake
     */
    bool isHandshake = false;
    if (SECURE_STATE_ESTABLISHED != secConnectionCtx_ptr->connState)
        isHandshake = true;

    LOG_DEBUG("mbedtls_net_send: isHandshake=%d state=%d\n",
            isHandshake, secConnectionCtx_ptr->connState);

    if ((1 == secConnectionCtx_ptr->inSend) && (false == secConnectionCtx_ptr->enableMultiInSend))
    {
        LOG_DEBUG("mbedtls_net_send: INSEND->return \r\n");
        secConnectionCtx_ptr->wantSend = true;
        return MBEDTLS_ERR_SSL_WANT_WRITE;
    }

    secConnectionCtx_ptr->inSend = 1;

    /* in case of handshake data we have to provide a sending buffer
     * from the network layer
     */
    if (false != isHandshake)
    {
        if (secConnectionCtx_ptr->countSendingCB > 0)
        {
            LOG_DEBUG("mbedtls_net_send: countSendingCB>0 -> return\n");
            secConnectionCtx_ptr->wantSend = true;
            return MBEDTLS_ERR_SSL_WANT_WRITE;
        }

        rc = Sec_prepareForSending(secConnectionCtx_ptr);

        switch (rc)
        {
        case RC_OK:
            break;
        case RC_TCP_RETX:
            case RC_TCP_OUT_OF_MEMORY:
            case RC_TCP_SOCKET_BUSY:
            case RC_UDP_OUT_OF_MEMORY:
            case RC_UDP_SOCKET_BUSY:
            /* continue with retry later */
            rc = Sec_retryLater(secConnectionCtx_ptr);
            return MBEDTLS_ERR_SSL_WANT_WRITE;
        default:
            /* fatal error on this socket -> close connection */
            Sec_disconnect(secConnectionCtx_ptr);
            return MBEDTLS_ERR_SSL_WANT_WRITE; //@todo need to check this MBEDTLS_ERR_SSL_WANT_WRITE return
        }
    }

    /* use tcp-retransmission buffer if exist */
    if (NULL != secConnectionCtx_ptr->tcpRetransmissionBuffer)
    {
        LOG_FATAL("mbedtls_net_send: Use RETRANSMISSION\n");
        secConnectionCtx_ptr->sCtx.buffer = secConnectionCtx_ptr->tcpRetransmissionBuffer;
        secConnectionCtx_ptr->tcpRetransmissionBuffer = NULL;
    }

    /* mbedTls wants more bytes to send, as we have network buffer size
     * we reduce it to network buffsize, but this may result failty behavior.
     */
    if ((unsigned) n > CommBuff_getSize(secConnectionCtx_ptr->sCtx.buffer))
    {
        LOG_FATAL("mbedtls_net_send: Cut mbedTls send data.\n");
        LOG_FATAL("mbedtls_net_send: size=%d n=%d\n",
                CommBuff_getSize(secConnectionCtx_ptr->sCtx.buffer), n);
        n = CommBuff_getSize(secConnectionCtx_ptr->sCtx.buffer);
    }

    /* copy data into network buffer and send it via udp */
    CommBuff_setLength(secConnectionCtx_ptr->sCtx.buffer, n);
    memcpy(CommBuff_getPayload(secConnectionCtx_ptr->sCtx.buffer), buf, n);

    rc = Sec_sendTo(secConnectionCtx_ptr,
            &secConnectionCtx_ptr->sending_Callable);
    /* when handshake data, we have to free the communication buffer.
     * in other case buffer is own of the application and we dont free it
     */
    if (false != isHandshake)
    {
        secConnectionCtx_ptr->sCtx.buffer = NULL;
    }

    if (rc != RC_OK)
    {
        secConnectionCtx_ptr->lastError = rc;
        Sec_upperLayerSendCB(secConnectionCtx_ptr, rc);
        return MBEDTLS_ERR_SSL_WANT_WRITE; //@todo need to check this MBEDTLS_ERR_SSL_WANT_WRITE return
    }

    LOG_DEBUG("mbedtls_net_send: ready\n");

    if ((secConnectionCtx_ptr->connState == SECURE_STATE_IN_NEGOTIATION))
    {
        Scheduler_enqueue(&secConnectionCtx_ptr->TlsHandshakeFunc, RC_OK);
    }
    return n;
}

/* Function is called from mbedTls-lib to receive data from tcp or udp
 */
int mbedtls_net_recv(void *ctx, unsigned char *buf, size_t n)
{
    bool haveMoreData = false;
    bool needMoreData = false;

    SecConnectionCtx_T *secConnectionCtx_ptr = (SecConnectionCtx_T *) ctx;

    assert(secConnectionCtx_ptr != NULL);

    if (-1 == secConnectionCtx_ptr->mbedTlsReadPosition)
    {
        LOG_DEBUG("mbedtls_net_recv: data all processed -> return MBEDTLS_ERR_SSL_WANT_READ\n");
        return MBEDTLS_ERR_SSL_WANT_READ;
    }

    /* not data received till now -> interrupt mbedTls-negotiate */
    if (NULL == secConnectionCtx_ptr->receiveBuffer)
    {
        LOG_DEBUG("mbedtls_net_recv: data not ready -> interrupt mbedTls\n");
        return MBEDTLS_ERR_SSL_WANT_READ;
    }
    LOG_DEBUG("mbedtls_net_recv: datalen=%d n=%d\n",
            CommBuff_getLength(secConnectionCtx_ptr->receiveBuffer), n);
    LOG_DEBUG("mbedtls_net_recv: mbedTlsReadPosition=%d\n",
            secConnectionCtx_ptr->mbedTlsReadPosition);

    secConnectionCtx_ptr->monitorCounter = 0;
    LOG_DEBUG("mbedtls_net_recv: RESET monitorCounter\n");

    // copy message into mbedTls buffer
    char *ptr = &(CommBuff_getPayload(secConnectionCtx_ptr->receiveBuffer))
            [secConnectionCtx_ptr->mbedTlsReadPosition];
    uint16_t len = CommBuff_getLength(secConnectionCtx_ptr->receiveBuffer);

    if (n >= (size_t)(len - secConnectionCtx_ptr->mbedTlsReadPosition))
    {
        needMoreData = true;
        n = len - secConnectionCtx_ptr->mbedTlsReadPosition;
    }

    memcpy(buf, ptr, n);

    /* if mbedTls do not consume all data -> remember read position */
    if (len > secConnectionCtx_ptr->mbedTlsReadPosition + n)
    {
        haveMoreData = true;
        secConnectionCtx_ptr->mbedTlsReadPosition += n;
    }
    else
    {
        /* ready with processing received data */
        secConnectionCtx_ptr->mbedTlsReadPosition = -1;
    }

    /* cleanup Comm_Buff if connection not established */
    if (-1 == secConnectionCtx_ptr->mbedTlsReadPosition
            && secConnectionCtx_ptr->connState != SECURE_STATE_ESTABLISHED)
    {
        /* free receive buffer if not for application */
        CommBuff_free(secConnectionCtx_ptr->receiveBuffer);
        secConnectionCtx_ptr->receiveBuffer = NULL;
    }

    LOG_DEBUG("mbedtls_net_recv: mbedTlsReadPosition=%d\n",
            secConnectionCtx_ptr->mbedTlsReadPosition);
    LOG_DEBUG("mbedtls_net_recv: data -> mbedTls len=%d\n", n);

    // We got more data, So call process again.
    if (((needMoreData == true) || (haveMoreData == true)) && (secConnectionCtx_ptr->connState == SECURE_STATE_IN_NEGOTIATION))
    {
        needMoreData = false;
        haveMoreData = false;
        Scheduler_enqueue(&secConnectionCtx_ptr->TlsHandshakeFunc, RC_OK);
    }
    return n;
}

static void Sec_ReceiveData(SecConnectionCtx_T *secConnectionCtx_ptr,
        retcode_t status)
{
    LOG_DEBUG("Sec_ReceiveData:\n");
    CommBuff_T commBuff = secConnectionCtx_ptr->receiveBuffer;
    if (1 == secConnectionCtx_ptr->secSocketCtx_ptr->isTcp)
    {
        commBuff = CommBuff_alloc(2 * SERVAL_MAX_SIZE_APP_PACKET);
        if (NULL == commBuff)
        {
            LOG_DEBUG("Sec_ReceiveData: ERROR CommBuff_alloc\n");
            Sec_disconnect(secConnectionCtx_ptr);
            return;
        }
        LOG_DEBUG("Sec_ReceiveData: ALLOC Combuff=%p\n", commBuff);
    }
    /* decrypt received data for application */
    int readLen = 0;
    int n = 0;
    while (true)
    {
        if (-1 == secConnectionCtx_ptr->mbedTlsReadPosition)
        {
            secConnectionCtx_ptr->mbedTlsReadPosition = 0;
        }
        n = mbedtls_ssl_read(&ssl,
                (unsigned char *) &(CommBuff_getPayload(commBuff))[readLen],
                CommBuff_getSize(commBuff) - readLen);
        if (n > 0)
        {
            readLen += n;
        }

        LOG_DEBUG("Sec_ReceiveData: mbedTls_read n=%d\n", n);
        LOG_DEBUG("Sec_ReceiveData: mbedTlsReadPosition=%d\n",
                secConnectionCtx_ptr->mbedTlsReadPosition);
        LOG_DEBUG("Sec_ReceiveData: Payload=%d (%d)\n",
                CommBuff_getPayload(commBuff), 2*SERVAL_MAX_SIZE_APP_PACKET);
        if (-1 == secConnectionCtx_ptr->mbedTlsReadPosition)
        {
            secConnectionCtx_ptr->mbedTlsReadPosition = 0;
            break;
        }
        if (n <= 0)
            break;
    }
    LOG_DEBUG("Sec_ReceiveData: readLen=%d n=%d\n", readLen, n);

    if (1 == secConnectionCtx_ptr->secSocketCtx_ptr->isTcp)
    {
        if (NULL != secConnectionCtx_ptr->receiveBuffer)
        {
            CommBuff_free(secConnectionCtx_ptr->receiveBuffer);
        }
        secConnectionCtx_ptr->receiveBuffer = commBuff;
    }

    /* valid data -> send to application */
    if (readLen > 0)
    {
        CommBuff_setLength(secConnectionCtx_ptr->receiveBuffer,
                (uint16_t) readLen);
        if (readLen >= 2 * SERVAL_MAX_SIZE_APP_PACKET)
        {
            CommBuff_free(secConnectionCtx_ptr->receiveBuffer);
            secConnectionCtx_ptr->receiveBuffer = NULL;
            status = RC_TCP_OUT_OF_MEMORY;
        }
        if (NULL != secConnectionCtx_ptr->secSocketCtx_ptr->upperLayerSocketCb)
        {
            if (secConnectionCtx_ptr->connState == SECURE_STATE_ESTABLISHED)
            {
                Callable_callback(
                        secConnectionCtx_ptr->secSocketCtx_ptr->upperLayerSocketCb,
                        status);
#if XDK_CONNECTIVITY_MQTT
                /* For MQTT the socket is maintained for its lifetime. This is a dirty hack to clean its communication buffers */
                CommBuff_free(secConnectionCtx_ptr->receiveBuffer);
                secConnectionCtx_ptr->receiveBuffer = NULL;
#endif /* XDK_CONNECTIVITY_MQTT */
            }
            else
            {
                CommBuff_free(secConnectionCtx_ptr->receiveBuffer);
                secConnectionCtx_ptr->receiveBuffer = NULL;
            }
        }
    }

    if (n <= 0)
    {
        if (NULL != secConnectionCtx_ptr->receiveBuffer)
        {
            CommBuff_free(secConnectionCtx_ptr->receiveBuffer);
            secConnectionCtx_ptr->receiveBuffer = NULL;
        }
        if ((n != MBEDTLS_ERR_SSL_WANT_READ) && (n != MBEDTLS_ERR_SSL_WANT_WRITE))
        {
            if (1 == secConnectionCtx_ptr->secSocketCtx_ptr->isTcp
                    && NULL != secConnectionCtx_ptr->secSocketCtx_ptr->upperLayerSocketCb)
            {
                Callable_callback(secConnectionCtx_ptr->secSocketCtx_ptr->upperLayerSocketCb, status);
                secConnectionCtx_ptr->secSocketCtx_ptr->upperLayerSocketCb = NULL;
            }
        }
    }
}

/* Receive-Callbackfunction received data from Udp/Tcp socket
 */
retcode_t Sec_receiveCB(Callable_T *callable_ptr, retcode_t status)
{
    retcode_t rc = RC_OK;

    SecSocketCtx_T *secSocketCtx_ptr =
            Sec_getSocketCtx_byCallable(callable_ptr);
    assert(secSocketCtx_ptr != NULL);

    LOG_DEBUG("Sec_receiveCB:\n");

    SecConnectionCtx_T *secConnectionCtx_ptr = NULL;
    /* get connection ctx */
    if (1 == secSocketCtx_ptr->isTcp)
    {
        secConnectionCtx_ptr =
                Tls_getConnectionCtx(secSocketCtx_ptr->socket.tcp);
    }

    bool mutexRelease = false;

    if ((NULL != secConnectionCtx_ptr) && (SECURE_STATE_IN_NEGOTIATION == secConnectionCtx_ptr->connState))
    {
        if (pdFALSE == xSemaphoreTake(TlsHandshakeBufferMutex, (TickType_t) UINT8_C(30000)))
        {
            Scheduler_enqueue(&secSocketCtx_ptr->socketCb, RC_OK);
            return RC_PLATFORM_ERROR;
        }
        else
        {
            mutexRelease = true;
        }
    }

    CommBuff_T recvBuf;

    if (status != RC_OK && NULL != secSocketCtx_ptr->upperLayerSocketCb)
    {
        //todo: Errorhandling
        LOG_FATAL(RC_FORMAT_STR"\n", RC_RESOLVE(status));
        Callable_callback(secSocketCtx_ptr->upperLayerSocketCb, status);
        secSocketCtx_ptr->upperLayerSocketCb = NULL;
        if (secConnectionCtx_ptr != NULL)
        {
            secConnectionCtx_ptr->upperLayerRetryCB_ptr = NULL;
        }
        if (mutexRelease)
        {
            if (pdFALSE == xSemaphoreGive(TlsHandshakeBufferMutex))
            {
                Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR));
            }
        }
        return RC_OK;
    }

    /* receive encryped data */
    if (0 == secSocketCtx_ptr->isTcp)
    {
        assert(false);
    }
    else
    {
        Tcp_SocketStatus_T stat;
        Tcp_getSocketStatus(secSocketCtx_ptr->socket.tcp, &stat);
        secSocketCtx_ptr->socketStatus = stat;
        rc = Tcp_receive(secSocketCtx_ptr->socket.tcp, &recvBuf);
    }

    /* get connection ctx */
    if (0 == secSocketCtx_ptr->isTcp)
    {
        assert(false);
    }

    if ((rc != RC_OK) && (NULL != secSocketCtx_ptr->upperLayerSocketCb))
    {
        LOG_FATAL(RC_FORMAT_STR"\n", RC_RESOLVE(status));
        if (secConnectionCtx_ptr != NULL)
        {
            LOG_DEBUG("Sec_receiveCB: set upperLayerRetryCB_ptr to zero\n");
            secConnectionCtx_ptr->upperLayerRetryCB_ptr = NULL;
        }
        if ((secConnectionCtx_ptr->connState == SECURE_STATE_HALFCLOSED) ||
                (secConnectionCtx_ptr->connState == SECURE_STATE_CLOSING))
        {
            /* Call Ctx_Free */
            secConnectionCtx_ptr->isTcp = 0;
        }
        else
        {
            Callable_callback(secSocketCtx_ptr->upperLayerSocketCb, rc);
        }

        secSocketCtx_ptr->upperLayerSocketCb = NULL;
        if (mutexRelease)
        {
            if (pdFALSE == xSemaphoreGive(TlsHandshakeBufferMutex))
            {
                Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR));
            }
        }
        return RC_OK;
    }

    /* Check for FIN-ACK on tcp */
    if ((0 != secSocketCtx_ptr->isTcp) && (NULL == recvBuf))
    {

        if (NULL != secSocketCtx_ptr->upperLayerSocketCb)
        {
            Scheduler_enqueue(secSocketCtx_ptr->upperLayerSocketCb, RC_OK);
            secSocketCtx_ptr->upperLayerSocketCb = NULL;
        }
        else
        {
            Tls_delete(secSocketCtx_ptr->socket.tcp);
        }

        if (mutexRelease)
        {
            if (pdFALSE == xSemaphoreGive(TlsHandshakeBufferMutex))
            {
                Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR));
            }
        }
        return RC_OK;
    }

    LOG_DEBUG("Sec_receiveCB: secConnectionCtx_ptr=%p\n", secConnectionCtx_ptr);

    /* there is not a secure connection known to this endpoint */
    if (NULL == secConnectionCtx_ptr)
    {
        /* Experimental, extremely dirty fix for handling an ALERT as the first message
         * 0x15 is the TLS type for an alert.
         * mbedTls will yield a WANT_READ error code and we cannot tell if this is due
         * to actually requiring more data in an ongoing handshake or because of this alert. */
        if (CommBuff_getLength(recvBuf) > 1 && CommBuff_getPayload(recvBuf)[0] == 0x15)
        {
            LOG_DEBUG("Received ALERT as the first handshake packet. Discarding it\n");
            if (mutexRelease)
            {
                if (pdFALSE == xSemaphoreGive(TlsHandshakeBufferMutex))
                {
                    Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR));
                }
            }
            return RC_OK;
        }

        /* in TLS there must already exist a connection context */
        assert(0 == secSocketCtx_ptr->isTcp);

        if (mutexRelease)
        {
            if (pdFALSE == xSemaphoreGive(TlsHandshakeBufferMutex))
            {
                Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR));
            }
        }
        /* no server -> we dont listen and do not create connections
         * on incoming requests
         */
        return RC_OK;
    }

    if (NULL != secConnectionCtx_ptr->receiveBuffer)
    {
        LOG_FATAL("Sec_receiveCB: HORRIBLE Buffer full state=%d (%p)\n",
                secConnectionCtx_ptr->connState, secConnectionCtx_ptr);
    }

    secConnectionCtx_ptr->receiveBuffer = recvBuf;

    LOG_DEBUG("Sec_receiveCB: connState=%d\n", secConnectionCtx_ptr->connState);

    /* check if Handshake -> call mbedTls negotiate */
    if (secConnectionCtx_ptr->connState == SECURE_STATE_IN_NEGOTIATION)
    {
        LOG_DEBUG("Sec_receiveCB: Negotiate (%p)\n", secConnectionCtx_ptr);

        if (-1 == secConnectionCtx_ptr->mbedTlsReadPosition)
        {
            secConnectionCtx_ptr->mbedTlsReadPosition = 0;
        }
        secConnectionCtx_ptr->inSend = 0;
        // Process the received handshake data
        Scheduler_enqueue(&secConnectionCtx_ptr->TlsHandshakeFunc, RC_OK);
    }

    /* check if connection established -> read the decrypted data
     * from mbedTls decryption function */
    if (secConnectionCtx_ptr->connState == SECURE_STATE_ESTABLISHED)
    {
        secConnectionCtx_ptr->manage = 0;
        Sec_ReceiveData(secConnectionCtx_ptr, status);
    }
    if (mutexRelease)
    {
        if (pdFALSE == xSemaphoreGive(TlsHandshakeBufferMutex))
        {
            Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SEMAPHORE_ERROR));
        }
    }
    return RC_OK;
}

/* Callbackfunction for Udp/Tcp_send
 */
static retcode_t Sec_sendingCB(Callable_T *callable_ptr,
        retcode_t status)
{
    LOG_DEBUG("Sec_sendingCB:\n");
    SecConnectionCtx_T *secConnectionCtx_ptr =
            Sec_getConnectionCtx_sending_Callable(callable_ptr);

    assert(NULL != secConnectionCtx_ptr);

    if (status != RC_OK)
    {
        LOG_DEBUG("Sec_sendingCB: ERROR\n");
        LOG_FATAL(RC_FORMAT_STR"\n", RC_RESOLVE(status));
        /* tcp retransmission */
        if (status == RC_TCP_RETX)
        {
            if (NULL == secConnectionCtx_ptr->tcpRetransmissionBuffer)
            {
                /* no valid retransmission buffer -> disconnect */
                Sec_disconnect(secConnectionCtx_ptr);
            }
            else
            {
                mbedtls_net_send((void*) secConnectionCtx_ptr,
                        (unsigned char *) CommBuff_getPayload(
                                secConnectionCtx_ptr->tcpRetransmissionBuffer),
                        CommBuff_getLength(
                                secConnectionCtx_ptr->tcpRetransmissionBuffer));
                secConnectionCtx_ptr->tcpRetransmissionBuffer = NULL;
            }
            if (secConnectionCtx_ptr->connState == SECURE_STATE_IN_NEGOTIATION)
            {
                Scheduler_enqueue(&secConnectionCtx_ptr->TlsHandshakeFunc, RC_OK);
            }
            return RC_OK;
        }
        /* other send errors -> disconnect */
        Sec_disconnect(secConnectionCtx_ptr);
        return RC_OK;
    }
    if (NULL != secConnectionCtx_ptr->tcpRetransmissionBuffer)
    {
        CommBuff_free(secConnectionCtx_ptr->tcpRetransmissionBuffer);
        secConnectionCtx_ptr->tcpRetransmissionBuffer = NULL;
        LOG_DEBUG("Sec_sendingCB: RESET RETRANSMISSION\n");
    }

    secConnectionCtx_ptr->countSendingCB--;

    switch (secConnectionCtx_ptr->connState)
    {
    /* normal (application) data send calback */
    case SECURE_STATE_ESTABLISHED:
        /* send callback to application */
        if (0 == secConnectionCtx_ptr->countSendingCB)
        {
            Sec_upperLayerSendCB(secConnectionCtx_ptr, status);
        }
        break;
    case SECURE_STATE_WAIT_ESTABLISHED:
        /* send callback to application */
        if (0 == secConnectionCtx_ptr->countSendingCB)
        {
            secConnectionCtx_ptr->connState = SECURE_STATE_ESTABLISHED;
            LOG_DEBUG("Sec_sendingCB: Callback app\n");
            Sec_upperLayerRetryCB(secConnectionCtx_ptr, status);
        }
        break;
    case SECURE_STATE_IN_NEGOTIATION:
        /* call negotiate again, if there are data to send */
        if (1 == secConnectionCtx_ptr->wantSend)
        {
            secConnectionCtx_ptr->wantSend = 0;
            Sec_upperLayerRetryCB(secConnectionCtx_ptr, status);
            if (-1 == secConnectionCtx_ptr->mbedTlsReadPosition)
            {
                secConnectionCtx_ptr->mbedTlsReadPosition = 0;
            }
            secConnectionCtx_ptr->inSend = 0;
            Scheduler_enqueue(&secConnectionCtx_ptr->TlsHandshakeFunc, RC_OK);
        }
        break;
    case SECURE_STATE_HALFCLOSED:
        case SECURE_STATE_CLOSING:
        (void) Sec_scheduleToNetwork(secConnectionCtx_ptr);
        break;
    default:
        LOG_FATAL("Sec_sendingCB: ERROR default\n");
    }
    return RC_OK;
}

retcode_t Tls_unlisten(Tcp_Listener_T listener)
{
    retcode_t rc = RC_OK;
    rc = Tcp_unlisten(listener);
    SecSocketCtx_T *secSocketCtx_ptr = Tls_getListenerCtx(listener);
    if (NULL != secSocketCtx_ptr)
    {
        memset(secSocketCtx_ptr, 0, sizeof(SecSocketCtx_T));
    }
    return rc;
}

retcode_t Tcp_connectSecure(Ip_Address_T *ipAddr_ptr, Ip_Port_T port,
        Callable_T *callback_ptr, Tcp_Socket_T *socket_ptr)
{
    retcode_t rc = RC_OK;
    Retcode_T retcode = RETCODE_OK;

    SecConnectionCtx_T *secConnectionCtx_ptr = NULL;
    /* create secure socket context */
    LOG_DEBUG("Tls_connectSecure:\n");
    SecSocketCtx_T *secSocketCtx_ptr = Sec_newSocketCtx();
    if (NULL == secSocketCtx_ptr)
        rc = RC_TCP_NO_FREE_PORT;
    char ipaddrstring[16] =
            { 0 };

    retcode = WLAN_ConvertIPAddressToString(*ipAddr_ptr, ipaddrstring); /* @todo - Use API from ServalStack */
    if (RETCODE_OK == retcode)
    {
        mbedtls_ssl_set_hostname(&ssl, ipaddrstring);
    }
    else
    {
        rc = RC_WEBSERVER_URL_NOT_FOUND;
    }
    /* create secure connection context */
    if (rc == RC_OK)
    {
        secSocketCtx_ptr->isTcp = 1;
        /* create connection context for this connection */
        secConnectionCtx_ptr =
                Sec_newConnectionCtx(NULL, NULL, NULL, 0);
        if (NULL != secConnectionCtx_ptr)
        {
            secConnectionCtx_ptr->secSocketCtx_ptr = secSocketCtx_ptr;
            secConnectionCtx_ptr->isTcp = 1;
            mbedtls_ssl_set_bio(&ssl, secConnectionCtx_ptr, mbedtls_net_send, mbedtls_net_recv, NULL);
        }
        else
        {
            rc = RC_DTLS_NO_MORE_CONNECTIONS_FREE;
        }
    }
    /* secure connect */
    if (rc == RC_OK)
    {
        Callable_assign(&secSocketCtx_ptr->socketCb, &Sec_receiveCB);
        secSocketCtx_ptr->upperLayerSocketCb = callback_ptr;
        rc = Tcp_connect(ipAddr_ptr, port,
                &secSocketCtx_ptr->socketCb, &secSocketCtx_ptr->socket.tcp);
    }

    if (rc == RC_OK)
    {
        secConnectionCtx_ptr->sCtx.conn.tcp = secSocketCtx_ptr->socket.tcp;
    }

    if (rc != RC_OK)
    {
        if (NULL != secConnectionCtx_ptr)
        {
            if (NULL != secConnectionCtx_ptr->mbedTlsConnectionCtx_ptr)
            {
//                mbedTls_free(secConnectionCtx_ptr->mbedTlsConnectionCtx_ptr);
            }
            memset(secSocketCtx_ptr, 0, sizeof(SecSocketCtx_T));
        }
        if (NULL != secConnectionCtx_ptr)
        {
            memset(secConnectionCtx_ptr, 0, sizeof(SecConnectionCtx_T));
        }
        *socket_ptr = Tcp_getInvalidSocket();
    }
    else
    {
        *socket_ptr = secSocketCtx_ptr->socket.tcp;
    }
    LOG_DEBUG("Tls_connectSecure: rc = %d\n", rc);
    if (RC_OK == rc)
    {
        secConnectionCtx_ptr->mbedTlsConnectionCtx_ptr = secConnectionCtx_ptr;
        Scheduler_enqueue(&secConnectionCtx_ptr->TlsHandshakeFunc, RC_OK);
    }

    return rc;
}

retcode_t Tls_receive(Tcp_Socket_T socket, CommBuff_T *rcvdData_ptr)
{
    retcode_t rc = RC_OK;
    LOG_DEBUG("Tls_receive:\n");

    /* not a secure socket -> receive plain */
    SecConnectionCtx_T *secConnectionCtx_ptr = Tls_getConnectionCtx(socket);
    if (NULL == secConnectionCtx_ptr)
    {
        return Tcp_receive(socket, rcvdData_ptr);
    }

    /* received data found on secure connection
     * -> send them back to application
     */
    *rcvdData_ptr = secConnectionCtx_ptr->receiveBuffer;
#if XDK_CONNECTIVITY_MQTT
    /* For MQTT the socket is maintained for its lifetime. This is a dirty hack to clean its communication buffers */
#else /* XDK_CONNECTIVITY_MQTT */
    secConnectionCtx_ptr->receiveBuffer = NULL;
#endif /* XDK_CONNECTIVITY_MQTT */
    secConnectionCtx_ptr->secSocketCtx_ptr->socketStatus = TCP_SOCKET_STATUS_CLOSED;

    return rc;
}

retcode_t Tls_prepareForSending(
        Tcp_Socket_T const socket, MsgSendingCtx_T *sendingCtx_ptr)
{
    retcode_t rc = RC_OK;
    /* not a secure socket -> do it plain */
    SecConnectionCtx_T *secConnectionCtx_ptr = Tls_getConnectionCtx(socket);
    if (NULL == secConnectionCtx_ptr)
    {
        return Tcp_prepareForSending(socket, sendingCtx_ptr);
    }

    switch (secConnectionCtx_ptr->connState)
    {
    /* connection established -> we try to prepare sending */
    case SECURE_STATE_ESTABLISHED:
        rc = Tcp_prepareForSending(socket, sendingCtx_ptr);
        return (rc);

        /* connection not established -> we are busy -> appl. must try later */
    case SECURE_STATE_IN_NEGOTIATION:
        return RC_TCP_SOCKET_BUSY;
        /* connection closing -> we cannot send any more */
    case SECURE_STATE_HALFCLOSED:
        case SECURE_STATE_CLOSING:
        return RC_TCP_SOCKET_ERROR;

        /* connection already not existing -> return error */
    default:
        return RC_TCP_SOCKET_ERROR;
    }
    return RC_TCP_SOCKET_BUSY;
}

retcode_t Tls_retrySendingLater(
        Tcp_Socket_T const socket, MsgSendingCtx_T *sendingCtx_ptr)
{
    retcode_t rc = RC_OK;
    LOG_DEBUG("Tls_retrySendingLater:\n");

    /* not a secure socket -> do it plain */
    SecConnectionCtx_T *secConnectionCtx_ptr = Tls_getConnectionCtx(socket);
    if (NULL == secConnectionCtx_ptr)
    {
        return Tcp_retrySendingLater(socket, sendingCtx_ptr);
    }

    switch (secConnectionCtx_ptr->connState)
    {
    /* remember the application send callback */
    case SECURE_STATE_ESTABLISHED:
        break;
    case SECURE_STATE_IN_NEGOTIATION:
        LOG_DEBUG("Tls_retrySendingLater: set upperLayerRetryCB_ptr\n");
        secConnectionCtx_ptr->upperLayerRetryCB_ptr =
                &sendingCtx_ptr->sendingFunc;
        return RC_OK;
        /* on closing connection we cannot send any more */
    case SECURE_STATE_HALFCLOSED:
        case SECURE_STATE_CLOSING:
        return RC_TCP_SOCKET_ERROR;
        /* connection dose not exist -> return error */
    default:
        return RC_TCP_SOCKET_ERROR;
        break;
    }
    secConnectionCtx_ptr->countRetryCB++;
    rc = Tcp_retrySendingLater(socket, sendingCtx_ptr);
    return rc;
}

retcode_t Tls_send(Tcp_Socket_T socket, CommBuff_T data,
        Callable_T *callback_ptr)
{
    LOG_DEBUG("Tls_send: datasize=%d\n", CommBuff_getLength(data));
    assert(NULL != callback_ptr);
    /* not a secure socket -> do it plain */
    SecConnectionCtx_T *secConnectionCtx_ptr = Tls_getConnectionCtx(socket);
    if (NULL == secConnectionCtx_ptr)
    {
        LOG_DEBUG("Tls_send: Not secure -> Tcp_send\n");
        return Tcp_send(socket, data, callback_ptr);
    }

    if (secConnectionCtx_ptr->connState != SECURE_STATE_ESTABLISHED)
    {
        return (RC_TCP_SOCKET_BUSY);
    }

    /* write the plain data into mbedTls encryption function */
    secConnectionCtx_ptr->sCtx.buffer = data;
    secConnectionCtx_ptr->upperLayerSendCB_ptr = callback_ptr;

    secConnectionCtx_ptr->countSendingCB = 0;
    int n = mbedtls_ssl_write(&ssl,
            (unsigned char *) CommBuff_getPayload(secConnectionCtx_ptr->sCtx.buffer),
            CommBuff_getLength(secConnectionCtx_ptr->sCtx.buffer));
    secConnectionCtx_ptr->inSend = 0;
    if (n <= 0)
    {
        Sec_upperLayerSendCB(secConnectionCtx_ptr, RC_DTLS_OVERLOADED);
        return (RC_DTLS_OVERLOADED);
    }
    if (0 == secConnectionCtx_ptr->countSendingCB)
    {
        Sec_upperLayerSendCB(secConnectionCtx_ptr, RC_OK);
    }
    return RC_OK;
}

retcode_t Tls_close(Tcp_Socket_T socket)
{
    retcode_t rc = RC_OK;
    LOG_DEBUG("Tls_close: socket=0x%X\n", socket);
    SecConnectionCtx_T *secConnectionCtx_ptr = Tls_getConnectionCtx(socket);
    if (NULL == secConnectionCtx_ptr)
    {
        return Tcp_close(socket);
    }
    secConnectionCtx_ptr->sCtx.buffer = NULL;
    secConnectionCtx_ptr->connState = SECURE_STATE_HALFCLOSED;

    /* Shutdown mbedTls */
    LOG_DEBUG("Tls_close: Sec_shutdown\n");
    Sec_disconnect(secConnectionCtx_ptr);
    return rc;
}

retcode_t Tls_delete(Tcp_Socket_T socket)
{
    retcode_t rc = RC_OK;
    SecSocketCtx_T *secSocketCtx_ptr = Tls_getSocketCtx(socket);
    Tcp_SocketStatus_T stat;
    Tcp_getSocketStatus(socket, &stat);
    if (NULL == secSocketCtx_ptr)
    {
        return Tcp_delete(socket);
    }
    SecConnectionCtx_T *secConnectionCtx_ptr = Tls_getConnectionCtx(socket);
    LOG_DEBUG("Tls_delete: Socket stat=%d rc=%d\n", stat, rc);
    if (stat == TCP_SOCKET_STATUS_HALF_CLOSED)
    {
        secSocketCtx_ptr->upperLayerSocketCb = NULL;
    }
    else if ((stat == TCP_SOCKET_STATUS_CONNECTING) || (stat == TCP_SOCKET_STATUS_OPEN))
    {
        rc = Tcp_close(socket);
        rc = RC_OK;
    }
    else
    {
        LOG_DEBUG("Tcp_delete:\n");
        rc = Tcp_delete(socket);
        if (RC_OK == rc)
            Sec_FreeCtx(secConnectionCtx_ptr);
    }
    return rc;
}

retcode_t Tls_getSocketStatus(
        Tcp_Socket_T socket, Tcp_SocketStatus_T *status_ptr)
{
    retcode_t rc = RC_OK;
    SecSocketCtx_T *secSocketCtx_ptr = Tls_getSocketCtx(socket);
    if (NULL == secSocketCtx_ptr)
    {
        return Tcp_getSocketStatus(socket, status_ptr);
    }
    if (TCP_SOCKET_STATUS_CLOSED == secSocketCtx_ptr->socketStatus)
    {
        rc = Tcp_getSocketStatus(socket, status_ptr);
        LOG_DEBUG("Tls_getSocketStatus: direct status %d\n", *status_ptr);

        return rc;
    }
    *status_ptr = (Tcp_SocketStatus_T) secSocketCtx_ptr->socketStatus;
    LOG_DEBUG("Tls_getSocketStatus: remembered status %d\n", *status_ptr);
    return RC_OK;
}

/* Internal function for prepare for sending sending later in the security layer
 */
static retcode_t Sec_prepareForSending(
        SecConnectionCtx_T * secConnectionCtx_ptr)
{
    retcode_t rc = RC_OK;
    if (0 == secConnectionCtx_ptr->isTcp)
    {
        assert(false);
    }
    else
    {
        rc = Tcp_prepareForSending(
                secConnectionCtx_ptr->sCtx.conn.tcp,
                &secConnectionCtx_ptr->sCtx);
    }
    return rc;
}

/* Internal function for sending in the security layer
 */
static retcode_t Sec_sendTo(SecConnectionCtx_T *secConnectionCtx_ptr,
        Callable_T *sendCB_ptr)
{
    retcode_t rc = RC_OK;

    secConnectionCtx_ptr->countSendingCB++;
    LOG_DEBUG("Sec_sendTo:\n");

    if (0 == secConnectionCtx_ptr->isTcp)
    {
        assert(false);
    }
    else
    {
        /* remember sending data for retransmission */
        if (NULL == secConnectionCtx_ptr->tcpRetransmissionBuffer)
        {
            secConnectionCtx_ptr->tcpRetransmissionBuffer =
                    CommBuff_alloc(CommBuff_getLength(
                            secConnectionCtx_ptr->sCtx.buffer));
            if (NULL != secConnectionCtx_ptr->tcpRetransmissionBuffer)
            {
                memcpy(CommBuff_getPayload(
                        secConnectionCtx_ptr->tcpRetransmissionBuffer),
                        CommBuff_getPayload(
                                secConnectionCtx_ptr->sCtx.buffer),
                        CommBuff_getLength(secConnectionCtx_ptr->sCtx.buffer));
                CommBuff_setLength(
                        secConnectionCtx_ptr->tcpRetransmissionBuffer,
                        CommBuff_getLength(secConnectionCtx_ptr->sCtx.buffer));
            }
        }
        rc = Tcp_send(secConnectionCtx_ptr->sCtx.conn.tcp,
                secConnectionCtx_ptr->sCtx.buffer,
                sendCB_ptr);
        LOG_DEBUG("Tcp_send: rc=%d\n", rc);
    }

    if (rc != RC_OK)
    {
        secConnectionCtx_ptr->countSendingCB--;
        Sec_disconnect(secConnectionCtx_ptr);
    }
    else
    {
        LOG_DEBUG("Sec_sendTo: RESET monitorCounter\n");
        secConnectionCtx_ptr->monitorCounter = 0;
    }
    return rc;
}

/* Internal function for scheduling jobs in the security layer
 */
static retcode_t Sec_retryLater(SecConnectionCtx_T *secConnectionCtx_ptr)
{
    retcode_t rc = RC_OK;
    if (0 == secConnectionCtx_ptr->isTcp)
    {
        assert(false);
    }
    else
    {
        rc = Tcp_retrySendingLater(
                secConnectionCtx_ptr->sCtx.conn.tcp,
                &secConnectionCtx_ptr->sCtx);
    }

    LOG_DEBUG("Sec_retryLater: Sec_retryLater rc=%d\n", rc);
    if (rc != RC_OK)
    {
        LOG_DEBUG("Sec_retryLater: Tcp_retrySendingLater rc=%d\n", rc);
        secConnectionCtx_ptr->lastError = rc;
        Sec_upperLayerSendCB(secConnectionCtx_ptr, rc);
        Sec_disconnect(secConnectionCtx_ptr);
    }
    else
    {
        secConnectionCtx_ptr->countRetryCB++;
    }

    return rc;
}

static void Sec_scheduleToNetwork(SecConnectionCtx_T *secConnectionCtx_ptr)
{
    LOG_DEBUG("Sec_scheduleToNetwork countRetryCB=%d countSendingCB=%d\n",
            secConnectionCtx_ptr->countRetryCB,
            secConnectionCtx_ptr->countSendingCB);
    if (secConnectionCtx_ptr->countRetryCB > 0)
    {
        return;
    }

    if (secConnectionCtx_ptr->countSendingCB > 0)
    {
        return;
    }
    LOG_DEBUG("Sec_scheduleToNetwork Done\n");

    secConnectionCtx_ptr->countRetryCB++;
    Scheduler_enqueue(&secConnectionCtx_ptr->sCtx.sendingFunc, RC_OK);
}

/* Internal function for freeinng secure contexts
 */
static void Sec_FreeCtx(SecConnectionCtx_T *secConnectionCtx_ptr)
{
    LOG_DEBUG("Sec_FreeCtx: inSend=%d\n", secConnectionCtx_ptr->inSend);
    if (1 == secConnectionCtx_ptr->inSend
            || secConnectionCtx_ptr->countSendingCB > 0)
    {
        LOG_DEBUG("Sec_FreeCtx: inSend->retry\n");
        if (secConnectionCtx_ptr->countSendingCB == 0)
        {
            LOG_DEBUG("Sec_FreeCtx: inSend->retry\n");
        }
    }

    if (1 == secConnectionCtx_ptr->isTcp)
    {
        LOG_DEBUG("Sec_FreeCtx: FREE CTX SOCKET-CTX\n");
        memset(secConnectionCtx_ptr->secSocketCtx_ptr, 0,
                sizeof(SecSocketCtx_T));
        Callable_assign(&secConnectionCtx_ptr->secSocketCtx_ptr->socketCb,
                &Sec_receiveCB);
    }

    if (secConnectionCtx_ptr->mbedTlsConnectionCtx_ptr)
    {
        if (0 != mbedtls_ssl_session_reset(&ssl))
        {
            /* @todo - Log error */
        }
    }
    if (NULL != secConnectionCtx_ptr->tcpRetransmissionBuffer)
    {
        LOG_DEBUG("Sec_FreeCtx: RESET RETRANS\n");
        CommBuff_free(secConnectionCtx_ptr->tcpRetransmissionBuffer);
    }

    secConnectionCtx_ptr->secSocketCtx_ptr->inUse = 0;
    memset(secConnectionCtx_ptr, 0, sizeof(SecConnectionCtx_T));
    LOG_DEBUG("Sec_FreeCtx: FREE ALL\n");
}

void Sec_upperLayerSendCB(SecConnectionCtx_T *secConnectionCtx_ptr,
        retcode_t rc)
{
    if (NULL == secConnectionCtx_ptr->upperLayerSendCB_ptr)
        return;
    LOG_DEBUG("Sec_upperLayerSendCB: Callback app rc=%d\n", rc);
    Callable_T *cb = secConnectionCtx_ptr->upperLayerSendCB_ptr;
    secConnectionCtx_ptr->upperLayerSendCB_ptr = NULL;
    if (0 == secConnectionCtx_ptr->isTcp)
    {
        switch (rc)
        {
        case RC_DTLS_OVERLOADED:
            rc = RC_UDP_OVERLOADED;
            break;
        default:
            ;
        }
    }
    else
    {
        switch (rc)
        {
        case RC_DTLS_OVERLOADED:
            rc = RC_TCP_OVERLOADED;
            break;
        default:
            ;
        }
    }
    Callable_callback(cb, rc);
}

void Sec_upperLayerRetryCB(SecConnectionCtx_T *secConnectionCtx_ptr,
        retcode_t rc)
{
    if (NULL == secConnectionCtx_ptr->upperLayerRetryCB_ptr)
        return;
    Callable_T *cb = secConnectionCtx_ptr->upperLayerRetryCB_ptr;
    secConnectionCtx_ptr->upperLayerRetryCB_ptr = NULL;
    if (!Callable_isAssigned(cb))
        return;
    LOG_DEBUG("Sec_upperLayerRetryCB: cb=%p func=%p\n", cb, cb->func);
    if (0 == secConnectionCtx_ptr->isTcp)
    {
        switch (rc)
        {
        case RC_DTLS_OVERLOADED:
            rc = RC_UDP_OVERLOADED;
            break;
        default:
            ;
        }
    }
    else
    {
        switch (rc)
        {
        case RC_DTLS_OVERLOADED:
            rc = RC_TCP_OVERLOADED;
            break;
        default:
            ;
        }
    }
    Callable_callback(cb, rc);
}

#endif /* SERVAL_ENABLE_TLS && SERVAL_TLS_MBEDTLS */

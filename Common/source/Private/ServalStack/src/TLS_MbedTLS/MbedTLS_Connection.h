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

#ifndef MBEDTLS_CONNECTION_H_
#define MBEDTLS_CONNECTION_H_

#include <Serval_Defines.h>

#if SERVAL_ENABLE_DTLS && SERVAL_TLS_MBEDTLS
#include <Serval_Security.h>
#include "MbedTLS_Socket.h"
#include <Serval_Thread.h>

#if SERVAL_ENABLE_DTLS
#include "mbedtls/ssl.h"
#include "mbedtls/platform.h"
#include "mbedtls/sha256.h"
#include "mbedtls/debug.h"
#include "mbedtls/timing.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ssl_cookie.h"
#include "mbedtls/net_sockets.h"
#endif

#define MbedTLS_invalidConnection NULL

#if SERVAL_ENABLE_DTLS_PARALLEL_HANDSHAKE
#define DTLS_HSCTX(conn) (&((conn)->ctx.dtls.handshakeContext))
#define TLS_HSCTX(conn) (&((conn)->ctx.tls.handshakeContext))
#else /* SERVAL_ENABLE_DTLS_PARALLEL_HANDSHAKE */
#if SERVAL_ENABLE_DTLS
extern EscDtlsInt_HandshakeContext dtlsHandshakeContext;
#define DTLS_HSCTX(conn) (&dtlsHandshakeContext)
#endif /* SERVAL_ENABLE_DTLS */
#endif /* SERVAL_ENABLE_DTLS_PARALLEL_HANDSHAKE */

#define DTLS_HEADER_SIZE 13

#if SERVAL_ENABLE_DTLS_HEADER_LOGGING
#define DTLS_LOG_PAYLOAD_SIZE 6
#define DTLS_LOG_BUFFER_SIZE (DTLS_HEADER_SIZE + DTLS_LOG_PAYLOAD_SIZE)
#endif

/*
 * Because we want to use the same function as init and callback for sending, there are
 * two states that correspond to SEND (i.e. initiate it) and SENDING (i.e. in progress)
 */
enum MbedConnectionState_E
{
    /**
     * Connection is free
     */
    MBED_CONNECTION_FREE = SECURE_STATE_FREE,
    /**
     * Connection is allocated
     */
    MBED_CONNECTION_ALLOCATED = SECURE_STATE_IN_NEGOTIATION,
    /**
     * Connection is established, normal app data can be sent/received
     */
    MBED_CONNECTION_ESTABLISHED = SECURE_STATE_ESTABLISHED,
    /**
     * Connection is closing.
     */
    MBED_CONNECTION_CLOSING = SECURE_STATE_HALFCLOSED,
    /**
     * Connection is closed and can be cleaned up.
     * Note: This is not used, yet, but is useful for session resumption
     */
    MBED_CONNECTION_CLOSED = SECURE_STATE_CLOSING,
    /**
     * Connection is closed by the peer.
     */
    // This _has_ to be last for enumeration to work
    MBED_CONNECTION_PEER_CLOSED,

};
typedef enum MbedConnectionState_E MbedConnectionState_T;

enum MbedPacket_E
{
    MBED_PACKET_NONE = 0,
    MBED_PACKET_HANDSHAKE = 1,
    MBED_PACKET_ALERT = 2,
    MBED_PACKET_CLOSE = 3
};
typedef enum MbedPacket_E MbedPacket_T;

#if SERVAL_ENABLE_DTLS

typedef struct
{
    mbedtls_ssl_context context;
    mbedtls_ssl_config conf;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    mbedtls_net_context fd;
    mbedtls_timing_delay_context timer;
    mbedtls_ssl_cookie_ctx cookie_ctx;
}dtls_session_t;

struct DtlsCtx_S
{
    dtls_session_t mbedCtx;
    bool retryLaterFailed;
#if SERVAL_DTLS_FLIGHT_MAX_RETRIES > 0
    uint8_t flightRetryCounter;
    uint64_t flightTimeoutMillis;
#endif
#if SERVAL_ENABLE_DTLS_HEADER_LOGGING
    uint16_t dtlsLength;
    uint8_t dtlsHeader[DTLS_LOG_BUFFER_SIZE];
#endif
};

typedef struct DtlsCtx_S DtlsCtx_T;

#endif /* SERVAL_ENABLE_DTLS */

/* structure for additional data attached to a secured connection */
struct MbedConnection_S
{
    /* Current state of the connection */
    MbedConnectionState_T state :3;

    bool inSend :1;

    int32_t rxReadPosition;

    /* Indicates that an error has occurred */
    bool error :1;

    /* Stores the packet that is currently being sent */
    MbedPacket_T sendingPacket :2;

    /* Stores the currently pending packet to be sent */
    MbedPacket_T pending :2;

    /* Upper layer function passed to retrySendingLater */
    Callable_T* upperLayerRetryFunction;

    /* message sent callback. */
    Callable_T msgSent;

    /* Message sending Context.
     * Should be replaced by just a callable (internalSendingCallable above)
     * but this requires a PAL API change.
     * This wastes 1 socket, 1 pointer, 1 bool of memory (plus padding)
     */
    MsgSendingCtx_T msgSendingCtx;

    /* secure socket that this connection is attached to */
    const MbedSocket_T* socket;

#if SERVAL_ENABLE_DTLS
    /* Peer of this connection. Currently stored in msgSendingCtx to save at least some space */
//	Peer_T peer;
#endif

    /* Communication buffer of an incoming message */
    CommBuff_T receiveBuffer;

    union
    {
#if SERVAL_ENABLE_DTLS
        DtlsCtx_T dtls;
#endif
    }ctx;
    TaskHandle_t DtlsHandshakeTaskHandle;
    SemaphoreHandle_t DtlsHandshakeBufferMutex;
    Callable_T DtlsHandshakeFunc;
};
typedef struct MbedConnection_S MbedConnection_T;

/*
 * Initialize the connection management
 */
void MbedTLS_initializeConnections(void);

/* This function allocates a new secure connection on the given secure socket
 */
MbedConnection_T *MbedTLS_newConnection(const MbedSocket_T* socket);

/* Free a given Connection
 */
void MbedTLS_freeConnection(MbedConnection_T *connection);

/* Check if there is any connection that is not free
 */
bool MbedTLS_isAnyConnectionOpen(void);

#if SERVAL_ENABLE_DTLS
/* This function returns the cycur connection from the given UDP endpoint or NULL if none was found
 */
MbedConnection_T *Dtls_getMbedConnection(const MbedSocket_T* socket,
        const Ip_Address_T *peerAddr_ptr,
        const Ip_Port_T peerPort);

/* This function gets the first pending data packet from the given UDP socket.
 */
retcode_t Dtls_getFirstPendingDataPacket(const MbedSocket_T* socket,
        Ip_Address_T* ipAddr_ptr,
        Ip_Port_T* port_ptr,
        CommBuff_T* packet_ptr);

/* This function monitors the retransmission of the handshake flights.
 */
void Dtls_monitorRetransmission(void);

#endif /* SERVAL_ENABLE_DTLS */

#endif /* CYCURTLSCONNECTION_H_ */

#endif /* SERVAL_ENABLE_DTLS && SERVAL_TLS_MBEDTLS */

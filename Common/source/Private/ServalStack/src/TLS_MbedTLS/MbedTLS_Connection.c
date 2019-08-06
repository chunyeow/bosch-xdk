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

#include "MbedTLS_Connection.h"
#include "MbedTLS_Flags.h"
#include "MbedTLS_internal.h"

#include <Serval_Ip.h>
#include <Serval_CommBuff.h>
#include <MbedTLS_Flags.h>
#include <Serval_Clock.h>

#include "stdio.h"

static MbedConnection_T mbedConnectionPool[SERVAL_MAX_SECURE_CONNECTIONS];

static void my_custom_debug(void *ctx, int level, const char *file, int line, const char *str)
{
    ((void) ctx);
    ((void) level);
    printf("%s:%04d: %s", file, line, str);
}

static unsigned int _avRandom(void)
{
    return (((unsigned int) rand() << 16) + rand());
}

static int _ssl_random(void *p_rng, unsigned char *output, size_t output_len)
{
    ((void) p_rng);
    uint32_t rnglen = output_len;
    uint8_t rngoffset = 0;

    while (rnglen > 0)
    {
        *(output + rngoffset) = (unsigned char) _avRandom();
        rngoffset++;
        rnglen--;
    }
    return 0;
}

#include <XDK_SNTP.h>

static int CustomGetTimeOfDay(struct timeval *tv, void *tzvp);

static int CustomGetTimeOfDay(struct timeval *tv, void *tzvp)
{
    BCDS_UNUSED(tzvp);
    uint64_t t = 0UL; /* get uptime in nanoseconds */
    uint32_t ms = 0UL; /* get uptime in nanoseconds */
    (void) SNTP_GetTimeFromSystem(&t, &ms);
    BCDS_UNUSED(ms);
    tv->tv_sec = t; /* convert to seconds */
    tv->tv_usec = ms * 1000; /* get remaining microseconds */
    return 0;
}

unsigned long mbedtls_timing_get_timer(struct mbedtls_timing_hr_time *val, int reset)
{
    struct timeval *t = (struct timeval *) val;

    if (reset)
    {
        CustomGetTimeOfDay(t, NULL);
        return (0);
    }
    else
    {
        unsigned long delta;
        struct timeval now;
        CustomGetTimeOfDay(&now, NULL);
        delta = (now.tv_sec - t->tv_sec) * 1000ul + (now.tv_usec - t->tv_usec) / 1000;
        return (delta);
    }
}

/*
 * Set delays to watch
 */
void mbedtls_timing_set_delay(void *data, uint32_t int_ms, uint32_t fin_ms)
{
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *) data;

    ctx->int_ms = int_ms;
    ctx->fin_ms = fin_ms;

    if (fin_ms != 0)
    {
        (void) mbedtls_timing_get_timer(&ctx->timer, 1);
    }
}

/*
 * Get number of delays expired
 */
int mbedtls_timing_get_delay(void *data)
{
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *) data;
    unsigned long elapsed_ms;

    if (ctx->fin_ms == 0)
    return (-1);

    elapsed_ms = mbedtls_timing_get_timer(&ctx->timer, 0);

    if (elapsed_ms >= ctx->fin_ms)
    return (2);

    if (elapsed_ms >= ctx->int_ms)
    return (1);

    return (0);
}

void MbedTLS_initializeConnections()
{
    if (cycurTLSFlags.connectionsInitialized != 0)
    return;
    cycurTLSFlags.connectionsInitialized = 1;
    memset(mbedConnectionPool, 0, sizeof(mbedConnectionPool));
}

/*
 * TODO The state machine might need to be extended
 * ALLOCATED might be better than IN_NEGOTIATION
 * and CLOSED might be useful in case we want to support session resumption
 * Then the sessions will be reused opportunistically
 */
MbedConnection_T *MbedTLS_newConnection(const MbedSocket_T* socket)
{
    int index = 0;
    for (index = 0; index < SERVAL_MAX_SECURE_CONNECTIONS; index++)
    {
        if (mbedConnectionPool[index].state == MBED_CONNECTION_FREE)
        {
            memset(&mbedConnectionPool[index], 0, sizeof(MbedConnection_T));
            mbedConnectionPool[index].state = MBED_CONNECTION_ALLOCATED;
            mbedConnectionPool[index].socket = socket;
            if (NULL == mbedConnectionPool[index].DtlsHandshakeBufferMutex)
            {
                mbedConnectionPool[index].DtlsHandshakeBufferMutex = xSemaphoreCreateMutex();
                if (NULL == mbedConnectionPool[index].DtlsHandshakeBufferMutex)
                {
                    /* @todo - Log Error */
                    return MbedTLS_invalidConnection;
                }
            }
            if (NULL == mbedConnectionPool[index].DtlsHandshakeTaskHandle)
            {
                if (pdPASS != xTaskCreate(DtlsHandshakeThread, (const char *) "DTLS_HANDSHAKE", (uint16_t) 1200,
                                (void *) &mbedConnectionPool[index], 3, &mbedConnectionPool[index].DtlsHandshakeTaskHandle))
                {
                    /* @todo - Log Error */
                    return MbedTLS_invalidConnection;
                }
                if (NULL == mbedConnectionPool[index].DtlsHandshakeTaskHandle)
                {
                    /* @todo - Log Error */
                    return MbedTLS_invalidConnection;
                }
            }

            mbedtls_debug_set_threshold(0);

            mbedtls_ssl_init(&mbedConnectionPool[index].ctx.dtls.mbedCtx.context);
            mbedtls_ssl_config_init(&mbedConnectionPool[index].ctx.dtls.mbedCtx.conf);
            mbedtls_ssl_conf_rng(&mbedConnectionPool[index].ctx.dtls.mbedCtx.conf, _ssl_random, NULL);
            mbedtls_ssl_conf_dbg(&mbedConnectionPool[index].ctx.dtls.mbedCtx.conf, my_custom_debug, stdout);

            mbedtls_printf("  . Setting up the SSL/TLS structure...");
            if (0 != mbedtls_ssl_config_defaults(&mbedConnectionPool[index].ctx.dtls.mbedCtx.conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_PRESET_DEFAULT))
            {
                mbedtls_printf(" failed! mbedtls_ssl_config_defaults returned\n\r");
            }

            mbedtls_ssl_conf_max_version(&mbedConnectionPool[index].ctx.dtls.mbedCtx.conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);
            mbedtls_ssl_conf_min_version(&mbedConnectionPool[index].ctx.dtls.mbedCtx.conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);

            mbedtls_ssl_conf_authmode(&mbedConnectionPool[index].ctx.dtls.mbedCtx.conf, MBEDTLS_SSL_VERIFY_NONE); /* @todo - Check with OPTIONAL */

            mbedtls_ssl_cookie_init(&mbedConnectionPool[index].ctx.dtls.mbedCtx.cookie_ctx);

            mbedtls_ssl_set_timer_cb(&mbedConnectionPool[index].ctx.dtls.mbedCtx.context, &mbedConnectionPool[index].ctx.dtls.mbedCtx.timer, mbedtls_timing_set_delay, mbedtls_timing_get_delay);

            Callable_assign(&mbedConnectionPool[index].DtlsHandshakeFunc, &MbedTlsDtlsHandshake);

            return &mbedConnectionPool[index];
        }
    }
    return MbedTLS_invalidConnection;
}

void MbedTLS_freeConnection(MbedConnection_T *connection)
{
    if (CommBuff_isValid(connection->receiveBuffer))
    {
        CommBuff_free(connection->receiveBuffer);
    }

    mbedtls_ssl_session_reset(&connection->ctx.dtls.mbedCtx.context); /* <-- Might be redundant */
    mbedtls_ssl_free( &connection->ctx.dtls.mbedCtx.context );
    mbedtls_ssl_config_free( &connection->ctx.dtls.mbedCtx.conf );

    vSemaphoreDelete(connection->DtlsHandshakeBufferMutex);

    TaskHandle_t taskhandle = connection->DtlsHandshakeTaskHandle;
    memset(connection, 0, sizeof(MbedConnection_T));

    /* @todo - Add suspend as well */
    vTaskDelete(taskhandle);
}

bool MbedTLS_isAnyConnectionOpen()
{
    int index = 0;
    for (index = 0; index < SERVAL_MAX_SECURE_CONNECTIONS; index++)
    {
        if (mbedConnectionPool[index].state != MBED_CONNECTION_FREE)
        {
            return true;
        }
    }
    return false;
}

#if SERVAL_ENABLE_DTLS
static inline void findNextConnection(int index, MbedSocket_T* secureSocket, int16_t *iterator_ptr,
        bool onlyWithError, Ip_Address_T *ipAddr_ptr, Ip_Port_T *port_ptr)
{
    for (; index < SERVAL_MAX_SECURE_CONNECTIONS; index++)
    {
        if (mbedConnectionPool[index].state == MBED_CONNECTION_FREE
                || mbedConnectionPool[index].socket != secureSocket
        )
        {
            continue;
        }

        if (onlyWithError && mbedConnectionPool[index].error == 0)
        {
            continue;
        }
        Ip_copyAddr((const Ip_Address_T*) &mbedConnectionPool[index].msgSendingCtx.destAddr, ipAddr_ptr);
        *port_ptr = mbedConnectionPool[index].msgSendingCtx.destPort;
        *iterator_ptr = index;
    }
}

void Udp_iterateSecureConnections(int16_t *iterator_ptr,
        Udp_Socket_T const socket, bool onlyWithError,
        Ip_Address_T *ipAddr_ptr, Ip_Port_T *port_ptr)
{
    MbedSocket_T* secureSocket;
    int index;

    if (*iterator_ptr < 0)
    {
        index = 0;
    }
    else
    {
        index = *iterator_ptr + 1;
    }

    *iterator_ptr = -1;

    if (index >= SERVAL_MAX_SECURE_CONNECTIONS)
    {
        return;
    }

    secureSocket = Dtls_getMbedSocket(socket);
    if (secureSocket == NULL)
    {
        return;
    }

    findNextConnection(index, secureSocket, iterator_ptr, onlyWithError, ipAddr_ptr, port_ptr);
}

MbedConnection_T *Dtls_getMbedConnection(const MbedSocket_T* socket,
        const Ip_Address_T *peerAddr_ptr,
        const Ip_Port_T peerPort)
{
    int index = 0;
    for (index = 0; index < SERVAL_MAX_SECURE_CONNECTIONS; index++)
    {
        if (mbedConnectionPool[index].state == MBED_CONNECTION_FREE)
        {
            continue;
        }

        if (mbedConnectionPool[index].socket == socket
                && Ip_compareAddr((const Ip_Address_T*) &mbedConnectionPool[index].msgSendingCtx.destAddr, peerAddr_ptr)
                && mbedConnectionPool[index].msgSendingCtx.destPort == peerPort)
        {
            return &mbedConnectionPool[index];
        }
    }
    return MbedTLS_invalidConnection;
}

retcode_t Dtls_getFirstPendingDataPacket(const MbedSocket_T* socket,
        Ip_Address_T* ipAddr_ptr,
        Ip_Port_T* port_ptr,
        CommBuff_T* packet_ptr)
{
    int index = 0;
    for (index = 0; index < SERVAL_MAX_SECURE_CONNECTIONS; index++)
    {
        if (mbedConnectionPool[index].state == MBED_CONNECTION_FREE)
        {
            continue;
        }
        if (mbedConnectionPool[index].socket == socket
                && mbedConnectionPool[index].receiveBuffer != CommBuff_getInvalidBuffer())
        {
            Ip_copyAddr((const Ip_Address_T*) &mbedConnectionPool[index].msgSendingCtx.destAddr, ipAddr_ptr);
            *port_ptr = mbedConnectionPool[index].msgSendingCtx.destPort;
            *packet_ptr = mbedConnectionPool[index].receiveBuffer;
            mbedConnectionPool[index].receiveBuffer = CommBuff_getInvalidBuffer();
            return RC_OK;
        }
    }

    return RC_UDP_SOCKET_ERROR;
}

void Dtls_monitorRetransmission(void)
{
    bool retry;

#if SERVAL_DTLS_FLIGHT_MAX_RETRIES > 0
    uint64_t now;
    Clock_getTimeMillis(&now);
#endif

    for (int index = 0; index < SERVAL_MAX_SECURE_CONNECTIONS; index++)
    {
        if (mbedConnectionPool[index].state == MBED_CONNECTION_FREE)
        continue;

        retry = mbedConnectionPool[index].ctx.dtls.retryLaterFailed;

#if SERVAL_DTLS_FLIGHT_MAX_RETRIES > 0
        if (!retry)
        {
            if (mbedConnectionPool[index].ctx.dtls.flightTimeoutMillis == 0)
            continue;
            if (mbedConnectionPool[index].ctx.dtls.flightTimeoutMillis > now)
            continue;

            if (mbedConnectionPool[index].ctx.dtls.flightRetryCounter < SERVAL_DTLS_FLIGHT_MAX_RETRIES)
            {
                /* @todo - Prepare retransmission here */
                {
                    mbedConnectionPool[index].ctx.dtls.flightRetryCounter++;
                    mbedConnectionPool[index].pending = MBED_PACKET_HANDSHAKE;
                }
            }
            else
            {
                ; /* Log error */
            }

            {
                ; /* @todo - Do only in case of error */
                mbedConnectionPool[index].state = MBED_CONNECTION_CLOSING;
                mbedConnectionPool[index].pending = MBED_PACKET_CLOSE;
            }

            mbedConnectionPool[index].ctx.dtls.flightTimeoutMillis = 0;
            retry = true;
        }
#endif /* SERVAL_DTLS_FLIGHT_MAX_RETRIES > 0 */

        if (retry)
        {
            mbedConnectionPool[index].ctx.dtls.retryLaterFailed = FALSE;
            Callable_call(&mbedConnectionPool[index].msgSendingCtx.sendingFunc, RC_OK);
        }
    }
}

#endif /* SERVAL_ENABLE_DTLS */

#endif /* SERVAL_ENABLE_DTLS && SERVAL_TLS_MBEDTLS */

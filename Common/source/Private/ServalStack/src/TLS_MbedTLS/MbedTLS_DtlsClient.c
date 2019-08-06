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
#if SERVAL_ENABLE_DTLS_CLIENT && SERVAL_TLS_MBEDTLS

#include "MbedTLS_internal.h"
#include "MbedTLS_Credentials.h"
#include "XDK_LWM2M.h"
#include "net_sockets.h"

#include <Serval_Log.h>

LWM2M_Setup_T * LWM2MGetCredentials(void);

/* Deprecated */
retcode_t Udp_connectSecure(Udp_Socket_T const socket,
        Ip_Address_T *peerAddr_ptr, Ip_Port_T peerPort)
{
    retcode_t rc = RC_OK;
    MbedSocket_T* secureSocket;
    MbedConnection_T* secureConnection;

    secureSocket = Dtls_getMbedSocket(socket);
    if ( NULL == secureSocket)
    {
        return RC_DTLS_PLAIN_SOCKET;
    }

    secureConnection = Dtls_getMbedConnection(secureSocket,
            (const Ip_Address_T*) peerAddr_ptr,
            peerPort);

    if ( NULL != secureConnection)
    {
        switch (secureConnection->state)
        {
        case MBED_CONNECTION_CLOSING:
            case MBED_CONNECTION_PEER_CLOSED:
            case MBED_CONNECTION_CLOSED:
            return RC_UDP_OVERLOADED;
        default:
            return RC_DTLS_ALREADY_CONNECTED;
        }
    }

    rc = Dtls_initializeSecureConnection(&secureConnection, secureSocket, (const Ip_Address_T*) peerAddr_ptr, peerPort);
    if (RC_OK != rc)
    {
        return rc;
    }

#if SERVAL_ENABLE_DTLS_PSK
    LWM2M_Setup_T * Lwm2mSetup = LWM2MGetCredentials();
    mbedtls_ssl_conf_psk(&secureConnection->ctx.dtls.mbedCtx.conf, (const unsigned char *) Lwm2mSetup->SecurityPreSharedKey.Key, Lwm2mSetup->SecurityPreSharedKey.KeyLength, (const unsigned char *) Lwm2mSetup->SecurityPreSharedKey.Identity, strlen(Lwm2mSetup->SecurityPreSharedKey.Identity));

    if( mbedtls_ssl_setup( &secureConnection->ctx.dtls.mbedCtx.context, &secureConnection->ctx.dtls.mbedCtx.conf ) != 0 )
    {
        printf( " failed\n  ! mbedtls_ssl_setup returned \r\n");
        assert(0);
    }

    mbedtls_ssl_set_bio(&secureConnection->ctx.dtls.mbedCtx.context, secureConnection, mbedtls_net_send, mbedtls_net_recv, NULL);

#endif //SERVAL_ENABLE_DTLS_PSK

    if (rc != RC_OK)
    {
        MbedTLS_freeConnection(secureConnection);
        return RC_DTLS_ERROR_CONNECT;
    }

    secureConnection->pending = MBED_PACKET_HANDSHAKE;

    return Callable_call(&secureConnection->msgSendingCtx.sendingFunc, RC_OK);
}

retcode_t Udp_openSecureSocket(Callable_T *callback_ptr,
        Udp_Socket_T *socket_ptr)
{
    MbedSocket_T* secureSocket;
    retcode_t rc;

    secureSocket = MbedTLS_newSocket();

    if ( NULL == secureSocket)
    {
        return RC_UDP_NO_FREE_PORT;
    }

    Dtls_initializeSecureSocket(secureSocket, socket_ptr, callback_ptr);

    rc = Udp_openSocket(&(secureSocket->internalOnReceiveCallback), socket_ptr);
    if (RC_OK != rc)
    {
        MbedTLS_freeSocket(secureSocket);
        *socket_ptr = Udp_getInvalidSocket();
        return rc;
    }

    return RC_OK;
}

#endif /* SERVAL_ENABLE_DTLS_CLIENT && SERVAL_TLS_MBEDTLS */

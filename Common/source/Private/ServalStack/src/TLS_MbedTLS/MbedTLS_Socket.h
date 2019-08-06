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

#ifndef MBEDTLS_SOCKET_H_
#define MBEDTLS_SOCKET_H_

#include <Serval_Defines.h>
#include <Serval_Udp.h>
#include <Serval_Tcp.h>

#define MbedTLS_invalidSocket NULL
/* structure for additional data attached to a socket capable of secure communication */
struct MbedSocket_S
{
    /* flag to mark the socket as allocated */
    bool isAllocated :1;

    /* union holding the lower layer UDP/TCP socket */
    union
    {
#if SERVAL_ENABLE_DTLS
    Udp_Socket_T* udp;
#endif
} socket;

#if SERVAL_ENABLE_DTLS
/* adapter internal callback on reception of data */
Callable_T internalOnReceiveCallback;

/* remembered upper layer callback on reception of data */
Callable_T *upperLayerOnReceiveCallback;
#endif
};
typedef struct MbedSocket_S MbedSocket_T;

/*
 * Initialize the socket management
 */
void MbedTLS_initializeSockets(void);

/* This function allocates a new secure socket
 */
MbedSocket_T *MbedTLS_newSocket(void);

/* Free a given Connection
 */
void MbedTLS_freeSocket(MbedSocket_T *socket);

#if SERVAL_ENABLE_DTLS
/* This function returns the cycur socket for a given UDP socket or NULL if none is found
 */
MbedSocket_T *Dtls_getMbedSocket(Udp_Socket_T udpSocket);
#endif /* SERVAL_ENABLE_DTLS */

#endif /* MBEDTLS_SOCKET_H_ */

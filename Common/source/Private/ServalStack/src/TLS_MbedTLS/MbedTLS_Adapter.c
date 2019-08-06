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

#define SERVAL_COMPONENT CYCURTLS

#include <Serval_Defines.h>

#if SERVAL_ENABLE_DTLS && SERVAL_TLS_MBEDTLS

#include "MbedTLSAdapter.h"
#include "MbedTLS_internal.h"
#include "MbedTLS_Socket.h"
#include "MbedTLS_Connection.h"
#include <MbedTLS_Flags.h>
#include <ResourceMonitor.h>
#include <Serval_OpControl.h>

#include <Serval_Security.h>

#include "mbedtls/ssl.h"
#include "mbedtls/platform.h"
#include "mbedtls/sha256.h"
#include "mbedtls/debug.h"
#include "mbedtls/timing.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ssl_cookie.h"
#include "mbedtls/net_sockets.h"

#if defined(MBEDTLS_THREADING_ALT)
void threading_mutex_init(mbedtls_threading_mutex_t *mutex)
{
    if (mutex == NULL)
    {
        return;
    }

    mutex->mutex = xSemaphoreCreateMutex();
}

void threading_mutex_free(mbedtls_threading_mutex_t *mutex)
{
    if (mutex == NULL)
    {
        return;
    }

    vSemaphoreDelete(mutex->mutex);
}

int threading_mutex_lock(mbedtls_threading_mutex_t *mutex)
{
    if (mutex == NULL)
    {
        return (MBEDTLS_ERR_THREADING_BAD_INPUT_DATA);
    }

    if (xSemaphoreTake( mutex->mutex, portMAX_DELAY) != pdTRUE)
    {
        return (MBEDTLS_ERR_THREADING_MUTEX_ERROR);
    }

    return 0;
}

int threading_mutex_unlock(mbedtls_threading_mutex_t *mutex)
{
    if (mutex == NULL)
    {
        return (MBEDTLS_ERR_THREADING_BAD_INPUT_DATA);
    }

    if ( xSemaphoreGive( mutex->mutex ) != pdTRUE)
    {
        return (MBEDTLS_ERR_THREADING_MUTEX_ERROR);
    }

    return (0);
}

#endif /* MBEDTLS_THREADING_ALT */

#if defined(MBEDTLS_PLATFORM_MEMORY)

void* XdkMbedCalloc(size_t num, size_t size)
{
    void * memory = pvPortMalloc(num * size);

    memset(memory, 0, num * size);
    return memory;
}

void XdkMbedFree(void* ptr)
{
    vPortFree(ptr);
}

#endif /* MBEDTLS_PLATFORM_MEMORY */

retcode_t MbedTLSAdapter_Initialize(void)
{

#if defined(MBEDTLS_THREADING_ALT)
    mbedtls_threading_set_alt(threading_mutex_init,
            threading_mutex_free,
            threading_mutex_lock,
            threading_mutex_unlock);
#endif

    MbedTLS_initializeSockets();
    MbedTLS_initializeConnections();

    return RC_OK;
}
#endif /* SERVAL_ENABLE_DTLS && SERVAL_TLS_MBEDTLS */

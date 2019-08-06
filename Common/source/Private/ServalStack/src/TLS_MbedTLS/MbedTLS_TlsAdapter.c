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

#define SERVAL_COMPONENT MBEDTLS

#include <Serval_Defines.h>
#if SERVAL_ENABLE_TLS && SERVAL_TLS_MBEDTLS

#include <stdlib.h>
#include <string.h>

#include "MbedTLS_Tls.h"
#include "threading_alt.h"

#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/platform.h"

#include "ServerCA.h"

mbedtls_ssl_context ssl;

static mbedtls_ssl_config conf;
#if SERVAL_ENABLE_TLS
static mbedtls_x509_crt cacert;
#endif /* SERVAL_ENABLE_TLS */

static const char *tls_test_ca_pem = SERVER_CA;

static int _ssl_parse_crt(mbedtls_x509_crt *crt)
{
    char buf[1024];
    mbedtls_x509_crt *local_crt = crt;
    int i = 0;
    while (local_crt)
    {
        mbedtls_x509_crt_info(buf, sizeof(buf) - 1, "", local_crt);
        {
            char str[512];
            const char *start, *cur;
            start = buf;
            for (cur = buf; *cur != '\0'; cur++)
            {
                if (*cur == '\n')
                {
                    size_t len = cur - start + 1;
                    if (len > 511)
                    {
                        len = 511;
                    }
                    memcpy(str, start, len);
                    str[len] = '\0';
                    start = cur + 1;
                    mbedtls_printf("%s", str);
                }
            }
        }
        mbedtls_printf("crt content:%u", (unsigned int) strlen(buf));
        local_crt = local_crt->next;
        i++;
    }
    return i;
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

static void my_custom_debug(void *ctx, int level, const char *file, int line, const char *str)
{
    ((void) ctx);
    ((void) level);
    printf("%s:%04d: %s", file, line, str);
}

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
    retcode_t rc = RC_OK;
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_ssl_conf_rng(&conf, _ssl_random, NULL);
    mbedtls_ssl_conf_dbg(&conf, my_custom_debug, stdout);

#if SERVAL_ENABLE_TLS
    mbedtls_x509_crt_init(&cacert);
    if (0 != mbedtls_x509_crt_parse(&cacert, (const unsigned char *) tls_test_ca_pem, strlen(tls_test_ca_pem) + 1))
    {
        mbedtls_printf(" failed\n  !  mbedtls_x509_crt_parse returned failure\r\n");
        return RC_SERVAL_ERROR;
    }

    _ssl_parse_crt(&cacert);
#endif /* SERVAL_ENABLE_TLS */

    /*
     * 2. Setup stuff
     */
    mbedtls_printf("  . Setting up the SSL/TLS structure...");
    if (0 != mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT))
    {
        mbedtls_printf(" failed! mbedtls_ssl_config_defaults returned\n\r");
    }

    mbedtls_ssl_conf_max_version(&conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);
    mbedtls_ssl_conf_min_version(&conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);

    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);

    mbedtls_printf(" ok");

#if SERVAL_ENABLE_TLS
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
#endif /* SERVAL_ENABLE_TLS */
    mbedtls_printf(" success\n  !  mbedtls_x509_crt_parse Succeeded\r\n");

    if (0 != mbedtls_ssl_setup(&ssl, &conf))
    {
        mbedtls_printf("failed! mbedtls_ssl_setup returned");
        return RC_SERVAL_ERROR;
    }

#if SERVAL_ENABLE_TLS
    Tls_SecureClient_initialize();
#endif /* SERVAL_ENABLE_TLS */

    return rc;
}
#endif /* SERVAL_ENABLE_TLS && SERVAL_TLS_MBEDTLS */

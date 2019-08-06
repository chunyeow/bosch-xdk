/*
* Licensee agrees that the example code provided to Licensee has been developed and released by Bosch solely as an example to be used as a potential reference for Licensee’s application development. 
* Fitness and suitability of the example code for any use within Licensee’s applications need to be verified by Licensee on its own authority by taking appropriate state of the art actions and measures (e.g. by means of quality assurance measures).
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
*
* Contributors:
* Bosch Software Innovations GmbH
*/
/*----------------------------------------------------------------------------*/

/**
 * @brief This file provides the implementation of LWM2MObjectAlertNotification module.
 *
 * @file
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_LWM2MOBJECTALERTNOTIFICATION

/* own header files */
#include "LWM2MObjectAlertNotification.h"
#include "LWM2MUtil.h"
#include "LWM2MObjects.h"

/* additional interface header files */
#include <Serval_Exceptions.h>
#include <Serval_Lwm2m.h>

#define ALERTNOTIFICATION_RESOURCES_INDEX(res) LWM2M_RESOURCES_INDEX(LWM2MObjectAlertNotificationResources, res)

static LWM2M_MUTEX_INSTANCE(Mutex) = LWM2M_MUTEX_INIT_VALUE;

/*lint -e(956) const ?*/
static Lwm2m_URI_Path_T AlertUriPath = { LWM2MOBJECTS_IX_ALERTNOTIFICATION_0, LWM2MOBJECTS_IX_ALERTNOTIFICATION_0, ALERTNOTIFICATION_RESOURCES_INDEX(alert) };

/*lint -esym(956, AlertMessage) mutex used */
static char AlertMessage[128] = { 0 };
static volatile bool Started = false;

static retcode_t LWM2MResetAlertFunc(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    (void) parser_ptr;
    (void) serializer_ptr;

    printf("reset alert\r\n");
    if (LWM2M_MUTEX_LOCK(Mutex))
    {
        AlertMessage[0] = 0;
        LWM2M_MUTEX_UNLOCK(Mutex);
        if (Started)

            /*lint -e(534) purposefully suppressing the warning:Ignoring return value of function,since in CONF mode this may fail many times */
            Lwm2mReporting_resourceChanged(&AlertUriPath);
    }

    return (RC_OK);
}

static retcode_t LWM2MReadAlert(Lwm2mSerializer_T *serializer_ptr, Lwm2mParser_T *parser_ptr)
{
    if ( NULL == serializer_ptr || NULL != parser_ptr)
    {
        return (RC_LWM2M_METHOD_NOT_ALLOWED);
    }
    retcode_t result = RC_COAP_OVERLOADED;
    if (LWM2M_MUTEX_LOCK(Mutex))
    {
        StringDescr_T strDescr_sn;
        StringDescr_set(&strDescr_sn, AlertMessage, strlen(AlertMessage));
        result = Lwm2mSerializer_serializeString(serializer_ptr, &strDescr_sn);
        LWM2M_MUTEX_UNLOCK(Mutex);
    }
    return result;
}

/** Refer interface header for description */
void LWM2MObjectAlertNotification_Init(void)
{
    Started = false;
    AlertMessage[0] = 0;
    AlertMessage[sizeof(AlertMessage) - 1] = 0;
    LWM2M_MUTEX_CREATE(Mutex);
}

/** Refer interface header for description */
void LWM2MObjectAlertNotification_Enable(void)
{
    Started = true;
}

/** Refer interface header for description */
void LWM2MObjectAlertNotification_SetValue(const char* alert)
{
    if (strncmp(AlertMessage, alert, sizeof(AlertMessage) - 1) != 0)
    {
        if (LWM2M_MUTEX_LOCK(Mutex))
        {
            strncpy(AlertMessage, alert, sizeof(AlertMessage) - 1);
            printf("alert: %s\r\n", AlertMessage);
            LWM2M_MUTEX_UNLOCK(Mutex);
            if (Started)
                /*lint -e(534) purposefully suppressing the warning:Ignoring return value of function,since in CONF mode this may fail many times */
                Lwm2mReporting_resourceChanged(&AlertUriPath);
        }
    }
}

LWM2MAlertNotificationResource_T LWM2MObjectAlertNotificationResources =
        {
                { 0, LWM2M_DYNAMIC(LWM2MReadAlert) | LWM2M_READ_ONLY },
                { 1, LWM2M_FUNCTION(LWM2MResetAlertFunc) },
        };

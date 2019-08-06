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
 * @brief Interface header for LWM2MObjectAlertNotification file.
 *
 * @file
 */

/* header definition ******************************************************** */
#ifndef LWM2MOBJECTALERTNOTIFICATION_H_
#define LWM2MOBJECTALERTNOTIFICATION_H_

/**
 * @brief LWM2M object id for alert notification.
 * BOSCH propr. in range of 10241 â€“ 32768
 */
#define LWM2M_OBJECTID_ALERTNOTIFICATION 15003

#include <Serval_Lwm2m.h>

/**
 * @brief LWM2M resource data for alert notification.
 */
struct LWM2MAlertNotificationResource_S
{
    Lwm2mResource_T alert; /**< alert, textual information */
    Lwm2mResource_T resetAlert; /**< reset alert */
};

typedef struct LWM2MAlertNotificationResource_S LWM2MAlertNotificationResource_T;

/*lint -esym(956, LWM2MObjectAlertNotificationResources) mutex used */
/**
 * @brief LWM2M resource data for instance 0.
 */
extern LWM2MAlertNotificationResource_T LWM2MObjectAlertNotificationResources;

/**
 * @brief Initialize LWM2M object instance. Must be called before any other function call.
 */
void LWM2MObjectAlertNotification_Init(void);

/**
 * @brief Enable LWM2M object instance to start providing alerts.
 */
void LWM2MObjectAlertNotification_Enable(void);

/**
 * @brief Set alert.
 * Copies provided string into the alert message.
 *
 * @param[in] alert
 * alert information
 */
void LWM2MObjectAlertNotification_SetValue(const char* alert);

#endif /* LWM2MOBJECTALERTNOTIFICATION_H_ */

/** ************************************************************************* */

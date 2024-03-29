/*
* Licensee agrees that the example code provided to Licensee has been developed and released by Bosch solely as an example to be used as a potential reference for Licensee�s application development. 
* Fitness and suitability of the example code for any use within Licensee�s applications need to be verified by Licensee on its own authority by taking appropriate state of the art actions and measures (e.g. by means of quality assurance measures).
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
 * @brief Interface header for LWM2MObjectConnectivityMonitoring file.
 *
 * @file
 */

#ifndef LWM2MOBJECTCONNECTIVITYMONITORING_H_
#define LWM2MOBJECTCONNECTIVITYMONITORING_H_

/* additional interface header files */
#include <Serval_Lwm2m.h>

/* -- LWM2M Object Connectivity Monitoring ------ */

/* Connectivity Monitoring Object ID */
#define LWM2M_OBJECTID_CONNECTIVITY_MONITORING	4

struct LWM2MObjectConnectivityMonitoring_Resource_S
{
    Lwm2mResource_T networkBearer; /* IX=0 */
    Lwm2mResource_T availableNetWorkBearer; /* IX=1 */
    Lwm2mResource_T radioSignalStrength; /* IX=2 */
    Lwm2mResource_T ipAddresses; /* IX=3 */
};

typedef struct LWM2MObjectConnectivityMonitoring_Resource_S LWM2MObjectConnectivityMonitoring_Resource_T;

/*lint -esym(956, LWM2MObjectConnectivityMonitoringResources) mutex used*/
extern LWM2MObjectConnectivityMonitoring_Resource_T LWM2MObjectConnectivityMonitoringResources;

/**
 * @brief Initialize LWM2M object instance. Must be called before any other function call.
 */
void LWM2MObjectConnectivityMonitoring_Init(void);

/**
 * @brief Enable LWM2M object instance. Start reporting values.
 */
void LWM2MObjectConnectivityMonitoring_Enable(void);

/**
 * @brief Set radio signal strength.
 *
 * @param[in] rss
 * radio signal strength in decibel.
 */
void LWM2MObjectConnectivityMonitoring_SetRadioSignalStrength(int rss); // 0..64 in dBm in GSM case!

/**
 * @brief Set ip address.
 *
 * @param[in] ipAddr
 * ip address in literal format e.g. "192.168.10.106".
 */
void LWM2MObjectConnectivityMonitoring_SetIpAddress(const char* ipAddr);

#endif /* LWM2MOBJECTCONNECTIVITYMONITORING_H_ */

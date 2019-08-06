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

/**  @file
 *
 *
 *
 * ****************************************************************************/

/* module includes ********************************************************** */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_COMMON_ID_SERVALPALWIFI

/* additional interface header files */

#include "PAL_initialize_ih.h"
#include "PAL_socketMonitor_ih.h"
#include "BCDS_WlanNetworkConfig.h"
#include "BCDS_ServalPal.h"
#include "BCDS_ServalPalWiFi.h"

/* constant definitions ***************************************************** */

#define TASK_PRIORITY_SERVALPAL_CMD_PROC            UINT32_C(3)
#define TASK_STACK_SIZE_SERVALPAL_CMD_PROC          UINT32_C(600)
#define TASK_QUEUE_LEN_SERVALPAL_CMD_PROC           UINT32_C(10)

static CmdProcessor_T CommandProcessorHandle;

/* inline functions ********************************************************* */

/* local functions ********************************************************** */

/**
 * @brief This will initialize the ServalPAL module and its necessary resources.
 *
 * @return Status of initialization
 */
static Retcode_T ServalPalSetup(void)
{
    Retcode_T returnValue = RETCODE_OK;
    returnValue = CmdProcessor_Initialize(&CommandProcessorHandle, "Serval PAL", TASK_PRIORITY_SERVALPAL_CMD_PROC, TASK_STACK_SIZE_SERVALPAL_CMD_PROC, TASK_QUEUE_LEN_SERVALPAL_CMD_PROC);
    /* serval pal common init */
    if (RETCODE_OK == returnValue)
    {
        returnValue = ServalPal_Initialize(&CommandProcessorHandle);
    }
    return returnValue;
}

/* global functions ********************************************************* */

retcode_t PAL_initialize(void)
{
    retcode_t servalStackRetcode = RC_OK;
    Retcode_T returnValue = ServalPalSetup();

    if (RETCODE_OK != returnValue)
    {
        Retcode_RaiseError(returnValue);
        servalStackRetcode = RC_PLATFORM_ERROR;
    }

    return (servalStackRetcode);
}

retcode_t PAL_getIpaddress(uint8_t * URL, Ip_Address_T* destAddr)
{
    retcode_t servalStackRetcode = RC_OK;
    Retcode_T returnValue = RETCODE_OK;
    returnValue = WlanNetworkConfig_GetIpAddress(URL , destAddr);
    if (RETCODE_OK != returnValue)
    {
        Retcode_RaiseError(returnValue);
        servalStackRetcode = RC_PLATFORM_ERROR;
    }
    return (servalStackRetcode);
}

void PAL_socketMonitorInit(void)
{
    Retcode_T returnValue = RETCODE_OK;
    returnValue = ServalPalWiFi_Init();
    if (RETCODE_OK == returnValue)
    {
        returnValue = ServalPalWiFi_NotifyWiFiEvent(SERVALPALWIFI_CONNECTED, NULL);
    }
    if (RETCODE_OK != returnValue)
    {
        Retcode_RaiseError(returnValue);
    }
}

/** ************************************************************************* */

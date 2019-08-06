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

/**
 * @file
 *
 * This module handles the UDP services.
 *
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_UDP

#if XDK_CONNECTIVITY_UDP
/* own header files */
#include "XDK_UDP.h"

/* system header files */
#include <stdio.h>

/* additional interface header files */
#include "FreeRTOS.h"
#include "task.h"
#include "socket.h"

/* constant definitions ***************************************************** */

/* local variables ********************************************************** */

/* global variables ********************************************************* */

/** Refer interface header for description */
Retcode_T UDP_Open(int16_t *handle)
{
    Retcode_T retcode = RETCODE_OK;
    int16_t SockID = -1;

    /* The return value is a positive number if successful; other wise negative */
    SockID = sl_Socket(SL_AF_INET, SL_SOCK_DGRAM, IPPROTO_UDP);
    if (SockID < 0)
    {
        printf("UDP_Open : Error in socket opening\r\n");
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_UDP_OPEN_FAILED);
    }
    else
    {
        *handle = SockID;
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T UDP_Send(int16_t handle, uint32_t serverIp, uint16_t serverPort, uint8_t * dataPtr, uint32_t dataLen)
{
    Retcode_T retcode = RETCODE_OK;
    int16_t Status = 0;
    SlSockAddrIn_t Addr;
    uint16_t AddrSize = 0;
    Addr.sin_family = SL_AF_INET;
    Addr.sin_port = sl_Htons((uint16_t) serverPort);
    Addr.sin_addr.s_addr = serverIp;
    AddrSize = sizeof(SlSockAddrIn_t);

    /* The return value is a number of characters sent;negative if not successful */
    Status = sl_SendTo(handle, dataPtr, dataLen, 0UL, (SlSockAddr_t *) &Addr, AddrSize);
    /* Check if 0 transmitted bytes sent or error condition */
    if (Status < 0)
    {
		retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_UDP_SENDING_DATA_FAILED);
    }
    return retcode;

}

/** Refer interface header for description */
Retcode_T UDP_Close(int16_t handle)
{
    int16_t Status = 0;

    Retcode_T retcode = RETCODE_OK;
    Status = sl_Close(handle);
    if (Status < 0)
    {
		retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_UDP_CLOSE_FAILED);
    }
    return retcode;

}

/** Refer interface header for description */
Retcode_T UDP_Setup(UDP_Setup_T setup)
{
    Retcode_T retcode = RETCODE_OK;
    if (UDP_SETUP_USE_CC31XX_LAYER != setup)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NOT_SUPPORTED);
    }
    return (retcode);
}

/** Refer interface header for description */
Retcode_T UDP_Enable(void)
{
    return (RETCODE_OK);
}
#endif /* XDK_CONNECTIVITY_UDP */

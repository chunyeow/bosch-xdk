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
 * @ingroup SERVAL_PAL_WIFI
 *
 * @defgroup PAL_socketMonitor PAL SocketMonitor
 * @{
 *
 * @brief Interface header for the PAL_socketMonitor module.
 *
 * @details The interface header exports the PAL SocketMonitor Interface API's which are used to PAL Socket Monitor Initialization and Configurations.
 *
 * @file
 */

/* header definition ******************************************************** */
#ifndef PAL_SOCKETMONITOR_IH_H_
#define PAL_SOCKETMONITOR_IH_H_

/* public interface declaration ********************************************* */
#include "Serval_Callable.h"
#include <stdint.h>
#include "PIp.h"

typedef struct PAL_SocketMonitor_QData_s
{
    Callable_T* callable_ptr;
    retcode_t status;
} PAL_SocketMonitor_QData_t;

extern xTaskHandle PAL_socketMonitorTaskHandler_gdt;

/* Socket Monitor Error Code*/
#define PAL_SOCKET_HANDLE_SUCCESS 		INT8_C(0) 	/* value returned when an API success */
#define PAL_SOCKET_HANDLE_ERROR  		INT8_C(-1) 	/* Error value returned when an API fails */
#define PAL_SOCKET_HANDLE_QUEUE_FULL	INT8_C(-2)
#define PAL_SOCKET_MONITOR_ERR_NO_ERROR           (0L)

/* public global variable declarations */

/* inline function definitions */

/* public function prototype declarations */
/**
 * @brief    Initializes the structure holding socket related parameters.Presently it also
 *              creates the task which monitors the sockets.
 * @see PAL_socketMonitoring()
 *
 */
void PAL_socketMonitorInit(void);

#endif /* PAL_SOCKETMONITOR_IH_H_ */
/**@} */
/** ************************************************************************* */

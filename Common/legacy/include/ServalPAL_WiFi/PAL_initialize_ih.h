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
 * @defgroup PAL_init PAL Initialize
 * @{
 *
 * @brief Interface header for the PAL_initialize module.
 *
 * @details  The interface header exports the High-level PAL API's to Initialize PAL & PAL GetIPAddress.
 *
 * @file
 */

/* header definition ******************************************************** */
#ifndef PAL_INITIALIZE_IH_H_
#define PAL_INITIALIZE_IH_H_

/* public interface declaration ********************************************* */

/* public type and macro definitions */
#include <PIp.h>
#include <Serval_Types.h>

/* public function prototype declarations */

#define PAL_INITIALIZE_SUCCESS UINT8_C(0) /**<  Error occurred in initializing pal */
#define PAL_INITIALIZE_ERROR UINT8_C(1) /**<  Error occurred in initializing pal */
#define PAL_IP_ADDRESS_SIZE UINT8_C(15) /**< Maximum size of the ip address */

/* public global variable declarations */

/* inline function definitions */

/* global function functions  */

/**
 * @brief This API initializes the PAL modules required by the serval.
 *
 * @retval     #PAL_INITIALIZE_SUCCESS   - PAL was initialized successfully
 * @retval     #PAL_INITIALIZE_ERROR    -  Error occurred in initializing PAL
 *
*/
retcode_t PAL_initialize(void);

/**
 * @brief This API returns back the IP address by the host name
 *
 * @param[in]  URL - The string pointer holding the host name
 * @param[out] destAddr - The variable in which IP address to be stored
 *
 * @retval     #RC_OK  - IP address returned successfully
 * @retval     #RC_PLATFORM_ERROR - Error occurred in fetching the ip address
 *
*/
retcode_t PAL_getIpaddress(uint8_t * URL ,Ip_Address_T* destAddr);

#endif /* PAL_INITIALIZE_IH_H_ */
/**@} */
/** ************************************************************************* */

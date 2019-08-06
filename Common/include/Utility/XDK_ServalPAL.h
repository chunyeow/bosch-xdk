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
 * @ingroup UTILITY
 *
 * @defgroup SERVALPAL Serval PAL
 * @{
 *
 * @brief This module handles the Serval stack Platform Adaptation Layer setup.
 *
 * @file
 */
/* header definition ******************************************************** */
#ifndef XDK_SERVALPAL_H_
#define XDK_SERVALPAL_H_

/* local interface declaration ********************************************** */
#include "BCDS_Retcode.h"
#include "BCDS_CmdProcessor.h"

/**
 * @brief This will setup the Serval stack Platform Adaptation Layer
 *
 * @param[in] cmdProcessorHandle
 * Command processor handle to be used if needed by the Platform Adaptation Layer.
 *
 * @retval #RETCODE_OK if initialization was successful
 * @retval #RETCODE_NULL_POINTER if cmdProcessorHandle is NULL
 * @return In case of any other error refer #Retcode_General_E for error codes.
 *
 * @note
 * - This must be the first API to be called if Serval stack Platform Adaptation Layer feature is to be used.
 * - Do not call this API more than once.
 */
Retcode_T ServalPAL_Setup(CmdProcessor_T * cmdProcessorHandle);

/**
 * @brief This will enable the Serval stack Platform Adaptation Layer
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #ServalPAL_Setup must have been successful prior.
 * - #WLAN_Enable must have been successful prior.
 * - Do not call this API more than once.
 */
Retcode_T ServalPAL_Enable(void);

#endif /* XDK_SERVALPAL_H_ */

/**@}*/

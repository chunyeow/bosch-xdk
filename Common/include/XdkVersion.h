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
 * @ingroup COMMON
 *
 * @defgroup XDKVERSION XDK version
 * @{
 *
 * @brief This module gives the current version of xdk software.
 *
 * @file
 */

#ifndef XDKVERSION_H_
#define XDKVERSION_H_

/* public type and macro definitions */
/** This macro gives the major version of the XDK */
#define XDKVERSION_MAJOR        3
/** This macro gives the minor version of the XDK */
#define XDKVERSION_MINOR        6
/** This macro gives the version of latest patch applied to the XDK */
#define XDKVERSION_PATCH        0

/* public function prototype declarations */
/**
* @brief  This function is used to get the XDK Major version.
*
* @note   Major Version is of type uint8_t.
*
* @return The major number of the XDK.
*/
uint8_t XdkVersion_GetMajor(void);

/**
* @brief  This function is used to get the XDK Minor version.
*
* @note   Minor Version is of type uint8_t.
*
* @return The minor number of the XDK.
*/
uint8_t XdkVersion_GetMinor(void);
/**
* @brief  This function is used to get the XDK Patch version.
*
* @note   Patch Version is of type uint8_t.
*
* @return The patch number of the XDK.
*/

uint8_t XdkVersion_GetPatch(void);

/**
* @brief  This function is used to get the XDK version.
*
* @note   XDK Version is of type uint32_t.
*
*           B3      :B2   :B1   :B0
*           Reserved:Major:Minor:Patch
*
*               B0 : Patch
*               B1 : Minor
*               B2 : Major
*               B3 : Reserved (0x00)
*
* @return The version number of the XDK.
*
* Example: usage
* \code{.c}
*
* #include "XdkVersion.h"
* {
* uint32_t version= XdkVersion_GetVersion();
* printf("the current version is %x\n\r",(int)version);
* }
* \endcode
*/

uint32_t XdkVersion_GetVersion(void);

#endif /* XDKVERSION_H_ */
/**@}*/

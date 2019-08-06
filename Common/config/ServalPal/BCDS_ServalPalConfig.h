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
 * @brief Configuration header for the BCDS ServalPal components.
 *
 */

#ifndef BCDS_SERVALPALCONFIG_H_
#define BCDS_SERVALPALCONFIG_H_

/* Include all headers which are needed by this file. */

/* Put the type and macro definitions here */
#define SERVALPAL_SOCKET_COUNT_MAX		         UINT8_C(5)  /**< Maximum Number of Sockets */
#define SERVALPAL_NETIF_COUNT_MAX                UINT8_C(1)  /**< Maximum Number of Network Interfaces  */

/**
 * This is the number of buffers managed by the the buffer pool. These
 * buffers are used if an incoming message should be kept for longer
 * time, e.g. waiting for additional packets or for longer processing.
 *
 */
#ifndef SERVALPAL_MAX_NUM_ADDITIONAL_COMM_BUFFERS
#define SERVALPAL_MAX_NUM_ADDITIONAL_COMM_BUFFERS    8
#endif

/**
 * Dynamic memory maximum allowed for buffer pool
 *
 */
#define SERVALPAL_MAXIMUM_DYNAMIC_MEMORY (SERVALPAL_MAX_NUM_ADDITIONAL_COMM_BUFFERS * 1500)

#define BCDS_SERVALPAL_WIFI_SOC_MONITOR_TASK_SIZE               1600UL      /**< Macro to represent task size of the Socket Monitor task */
#define BCDS_SERVALPAL_WIFI_SOC_MONITOR_PRIORITY                3UL         /**< Macro to represent priority of the Socket Monitor task */
#define BCDS_SERVALPAL_WIFI_SOC_MONITOR_PERIOD                  100UL       /**< Macro to represent sleep time of the Socket Monitor task */

/* Put the function declarations here */

#endif /* BCDS_SERVALPALCONFIG_H_ */

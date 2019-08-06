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
 * @brief Utils config header.
 *
 * @details Provides configuration interface for the Utils components.
 */

#ifndef BCDS_UTILSCONFIG_H_
#define BCDS_UTILSCONFIG_H_

/** @brief Enable (1) or disable (0) the CmdLineDebugger feature. */
#define BCDS_FEATURE_CMDLINEDEBUGGER    1

/** @brief Enable (1) or disable (0) the CmdProcessor feature. */
#define BCDS_FEATURE_CMDPROCESSOR       1

/** @brief Enable (1) or disable (0) the Crc feature. */
#define BCDS_FEATURE_CRC                1

/** @brief Enable (1) or disable (0) the EventHub feature. */
#define BCDS_FEATURE_EVENTHUB           1

/** @brief Enable (1) or disable (0) the GuardedTask feature. */
#define BCDS_FEATURE_GUARDEDTASK        1

/** @brief Enable (1) or disable (0) the LeanB2Cap feature. */
#define BCDS_FEATURE_LEANB2CAP          1

/** @brief Enable (1) or disable (0) the ErrorLogger feature. */
#define BCDS_FEATURE_ERRORLOGGER        1

/** @brief Enable (1) or disable (0) the Logging feature. */
#define BCDS_FEATURE_LOGGING            1

/** @brief Enable (1) or disable (0) the Queue feature. */
#define BCDS_FEATURE_QUEUE              1

/** @brief Enable (1) or disable (0) the RingBuffer feature. */
#define BCDS_FEATURE_RINGBUFFER         1

/** @brief Enable (1) or disable (0) the SleepControl feature. */
#define BCDS_FEATURE_SLEEPCONTROL       1

/** @brief Enable (1) or disable (0) the TaskMonitor feature. */
#define BCDS_FEATURE_TASKMONITOR        1

/**
 * @brief BCDS_TASKMONITOR_MAX_TASKS
 * Maximum number of TaskMonitor tickets to reserve for the system.
 */
#define BCDS_TASKMONITOR_MAX_TASKS      10

/** @brief Enable (1) or disable (0) the Tlv feature. */
#define BCDS_FEATURE_TLV                1


/** @brief Enable (1) or disable (0) the UartTransceiver feature. */
#define BCDS_FEATURE_UARTTRANSCEIVER    1

/** @brief Enable (1) or disable (0) the XProtocol feature. */
#define BCDS_FEATURE_XPROTOCOL          1

/** @brief Enable (1) or disable (0) the I2C transceiver feature. */
#define BCDS_FEATURE_I2CTRANSCEIVER         1

#endif /* BCDS_UTILSCONFIG_H_ */

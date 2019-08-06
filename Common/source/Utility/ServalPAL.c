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
 * This module handles the Serval stack Platform Adaptation Layer setup.
 *
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_SERVALPAL

#if XDK_UTILITY_SERVALPAL

/* own header files */
#include "XDK_ServalPAL.h"

/* system header files */
#include "stdio.h"
#include <string.h>

/* additional interface header files */
#include "BCDS_ServalPal.h"
#include "BCDS_ServalPalWiFi.h"
#include <Serval_Log.h>

/* constant definitions ***************************************************** */
#define LOG_MODULE_BUFFER_MAX_SIZE      512U

/* local variables ********************************************************** */
static LogAppender_T Appender;

/**< Handle for Serval PAL thread command processor */
static CmdProcessor_T * ServalPALCmdProcessorHandle;

/* global variables ********************************************************* */

/**
 * @brief The given char vector will send over com port.
 *
 * @param[in] text_ptr
 * pointer to the char vector
 *
 * @param[in] len
 * length of vector
 */
static void PrintMessage(const char *text_ptr, int len, ...)
{
    /*if length is 0, extract len from string and send*/
    if (0 == len)
    {
        len = strnlen(text_ptr, UINT16_C(LOG_MODULE_BUFFER_MAX_SIZE));
        if ('\0' != text_ptr[len + 1])
        {
            printf("Error in LogAppenderPrint: A non null termination character");
            /*lint -e40 */
            fflush(stdout);
            /*lint +e40 */
            return;
        }
    }
    printf("%s\n\r", text_ptr);
    /*lint -e40 */
    fflush(stdout);
    /*lint +e40 */
}

/**
 * @brief The given char vector will send over com port
 *
 * @param[in] text
 * char vector that has to be \0 terminated
 *
 */
static void LogWrite(char const *text)
{
    PrintMessage(text, strnlen(text, LOG_MODULE_BUFFER_MAX_SIZE));
}

/**
 * @brief The given char vector will send over com port
 *
 * @param[in] text_ptr
 * pointer to the char vector
 *
 * @param[in] len
 * length of vector
 */
static void LogWriteLen(char const *text, unsigned int len)
{
    PrintMessage((const char *) text, len);
}

/* The description is in the interface header file. */
static LogAppender_T *ServalPAL_PrintAppender(void)
{
    Appender.write = LogWrite;
    Appender.writeLen = LogWriteLen;
    return (&Appender);
}

/** Refer interface header for description */
Retcode_T ServalPAL_Setup(CmdProcessor_T * cmdProcessorHandle)
{
    Retcode_T retcode = RETCODE_OK;

    if (NULL == cmdProcessorHandle)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    if (RETCODE_OK == retcode)
    {
        ServalPALCmdProcessorHandle = cmdProcessorHandle;
        retcode = ServalPal_Initialize(ServalPALCmdProcessorHandle);
    }
    if (RETCODE_OK == retcode)
    {
        retcode = ServalPalWiFi_Init();
    }
    if (RETCODE_OK == retcode)
    {
        Log_enable(ServalPAL_PrintAppender());
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T ServalPAL_Enable(void)
{
    Retcode_T retcode = RETCODE_OK;

    retcode = ServalPalWiFi_NotifyWiFiEvent(SERVALPALWIFI_CONNECTED, NULL);

    return retcode;
}

#endif /* XDK_UTILITY_SERVALPAL */

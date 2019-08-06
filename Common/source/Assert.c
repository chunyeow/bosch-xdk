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
 *  @file
 *
 *  @brief The program assertion module implements several debug features including an
 *  assertion library, which can be used to verify assumptions made by the
 *  program and send this information to the user and stop program
 *  execution if this assumption fails.
 *
 *  There are two important assertion types: Static compilation time assertions,
 *  and dynamic runtime assertions.
 *
 *  The program assertion library behaves differently in debug and release builds.
 *  Static assertions are always enabled, but all other features of the library is
 *  disabled in release builds.
 *
 *  Release builds must be built with the NDEBUG symbol defined in the
 *  makefile.
 *
 **/

#ifndef NDEBUG /* valid only for debug builds */

#include "BCDS_Essentials.h"
#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID BCDS_ESSENTIALS_MODULE_ID_ASSERT

/* own headers */
#include "BCDS_Assert.h"

/* system header files */

#if defined(ASSERT_USE_STD_EXIT)
//lint -e49 error in standard libraries suppressed
#include <stdlib.h>
#endif

/* local variables ********************************************************** */

static Assert_Callback_T assertCallback = NULL; /*< Variable to store the callback function pointer */

/* Static assert to test the compiler capabilities */
static_assert((1 != 0), "Testing static assert");

/* global functions ********************************************************* */

/* The description of the function is available in header file */
Retcode_T Assert_Initialize(Assert_Callback_T callback)
{
	Retcode_T retcode = RETCODE_OK;

	assertCallback = callback;

    if (NULL == assertCallback)
    {
    	retcode = RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_INVALID_PARAM);
    }

    return (retcode);
}

/* The description of the function is available in header file */
void Assert_Dynamic(const unsigned long line, const unsigned char * const file)
{
    if (NULL != assertCallback)
    {
    	assertCallback(line , file);
    }
#if defined(ASSERT_USE_STD_EXIT)
    /* exit application */
    exit(EXIT_FAILURE);
#else
    for (;;)
    {
        ; /* block application */
    }
#endif
}

/** LCOV_EXCL_START : Start of section where code coverage is ignored.
 * Ignore code coverage for the deprecated APIs.*/

/* Deprecated function. See header file for more information */
Retcode_T Assert_initialize(Assert_Callback_T callback)
{
    return (Assert_Initialize(callback));
}

/* Deprecated function. See header file for more information */
void Assert_dynamic(const unsigned long line, const unsigned char * const file)
{
    Assert_Dynamic(line,file);
}

/** LCOV_EXCL_STOP : End of section where code coverage is ignored. */

#endif /* NDEBUG */


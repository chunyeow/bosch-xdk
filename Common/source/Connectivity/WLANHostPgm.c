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
 * This module handles the WLAN Host Programming services.
 * (Flashing XDK dummy certificate for Enterprise WPA2 connection onto the WLAN chip)
 *
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_WLANHOSTPGM

/* own header files */
#include "WLANHostPgm.h"

/* system header files */
#include <stdio.h>

/* additional interface header files */
#include "BCDS_Assert.h"
#include "BCDS_CmdProcessor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "simplelink.h"
#include "XDKDummyCA.h"

/* constant definitions ***************************************************** */

#define WLAN_HOST_PGM_CA_CHUNK_LEN              (1024)
#define WLAN_HOST_PGM_CA_LEN_128KB              (131072)
#define WLAN_HOST_PGM_CA_FIND_MIN(a,b)          (((a) < (b)) ? (a) : (b))

/* local variables ********************************************************** */

/**< XDK dummy certificate to be flashed for enabling Enterprise WPA2 connection */
static const unsigned char WLANHostPgmXdkDummyCertificate[] = { XDK_DUMMY_CA };

/**
 * @brief   WLANHostPgmUploadCertificate will upload the WLANHostPgmXdkDummyCertificate to the WLAN chip.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 */
static Retcode_T WLANHostPgmUploadCertificate(void)
{
    Retcode_T retcode = RETCODE_OK;
    _i32 fileHandle = -1;
    _u32 token = 0;
    _i32 retVal;
    _u32 remainingLen, movingOffset, chunkLen;

    printf("WLANHostPgmUploadCertificate : writing certificate %s\r\n", WLAN_HOST_PGM_CERT_FILE_NAME);

    retVal = sl_FsOpen((_u8 *) WLAN_HOST_PGM_CERT_FILE_NAME, FS_MODE_OPEN_CREATE(WLAN_HOST_PGM_CA_LEN_128KB, (_u32) _FS_FILE_OPEN_FLAG_COMMIT | (_u32)_FS_FILE_PUBLIC_WRITE), &token, &fileHandle);
    if (retVal < 0)
    {
        /* open failed */
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_WLAN_HOST_PGM_FS_OPEN_FAILED);
    }

    if (RETCODE_OK == retcode)
    {
        /* program the Certificate */
        remainingLen = sizeof(WLANHostPgmXdkDummyCertificate);
        movingOffset = 0;
        chunkLen = (_u32) WLAN_HOST_PGM_CA_FIND_MIN(WLAN_HOST_PGM_CA_CHUNK_LEN, remainingLen);

        printf("WLANHostPgmUploadCertificate : Programming certificate file\r\n");

        /* Flashing must be done in 1024 bytes chunks */
        do
        {
            retVal = sl_FsWrite(fileHandle, movingOffset, (_u8 *) &WLANHostPgmXdkDummyCertificate[movingOffset], chunkLen);
            if (retVal < 0)
            {
                /* write failed */
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_WLAN_HOST_PGM_FS_WRITE_FAILED);
                break;
            }

            remainingLen -= chunkLen;
            movingOffset += chunkLen;
            chunkLen = (_u32) WLAN_HOST_PGM_CA_FIND_MIN(WLAN_HOST_PGM_CA_CHUNK_LEN, remainingLen);
        } while (chunkLen > 0);
    }

    if (RETCODE_OK == retcode)
    {
        /* close the certificateImage file */
        printf("WLANHostPgmUploadCertificate : Closing certificateImage file\r\n");
        retVal = sl_FsClose(fileHandle, 0, 0, 0);

        if (retVal < 0)
        {
            /* close failed */
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_WLAN_HOST_PGM_FS_CLOSE_FAILED);
        }
    }
    if (RETCODE_OK == retcode)
    {
        printf("WLANHostPgmUploadCertificate : Certificate successfully programmed\r\n");

        printf("WLANHostPgmUploadCertificate : Restarting CC3100... \r\n");

        /* Stop the CC3100 device */
        if (0 != sl_Stop(0xFF))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_WLAN_HOST_PGM_SL_STOP_FAILED);
        }
    }
    if (RETCODE_OK == retcode)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        /* Initializing the CC3100 device */
        if ((_i16) ROLE_STA != sl_Start(0, 0, 0))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_WLAN_HOST_PGM_SL_START_FAILED);
        }
    }

    if (RETCODE_OK == retcode)
    {
        printf("WLANHostPgmUploadCertificate : Restart was successful. \r\n");

        vTaskDelay(pdMS_TO_TICKS(1000)); /* Needed for Bootup */
    }

    return retcode;
}

/** Refer interface header for description */
Retcode_T WLANHostPgm_Setup(void)
{
    return RETCODE_OK;
}

/** Refer interface header for description */
Retcode_T WLANHostPgm_Enable(void)
{
    return WLANHostPgmUploadCertificate();
}

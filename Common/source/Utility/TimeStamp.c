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

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_TIMESTAMP

/* own header files */
#include "XDK_TimeStamp.h"

/* system header files */
#include <stdio.h>
/* additional interface header files */

/* constant definitions ***************************************************** */

#define TIME_OFFSET                         (946684800LL + 86400*(31+29))

#define NO_OF_DAYS_PER_400_Y                (365*400 + 97)
#define NO_OF_DAYS_PER_100_Y                (365*100 + 24)
#define NO_OF_DAYS_PER_4_Y                  (365*4   + 1)

#define TIME_OVERFLOW_INT_MIN_MULTIPLIER    (31622400LL)
#define TIME_OVERFLOW_INT_MAX_MULTIPLIER    (31622400LL)

#define SECS_TO_DAYS_FACTOR                 (86400LL)

/**
 * @brief Convert time to real time
 *
 * @param[in] struct tm * tmPtr
 * tm structure pointer
 */
static void tm_to_realtime(struct tm * tmPtr)
{
    //    tm_sec;     /* Seconds: 0-59 (K&R says 0-61?) */
    //    tm_min;     /* Minutes: 0-59 */
    //    tm_hour;    /* Hours since midnight: 0-23 */
    //    tm_mday;    /* Day of the month: 1-31 */
    //    tm_mon;     /* Months *since* january: 0-11 */
    //    tm_year;    /* Years since 1900 */
    //    tm_wday;    /* Days since Sunday (0-6) */
    //    tm_yday;    /* Days since Jan. 1: 0-365 */
    //    tm_isdst;   /* +1 Daylight Savings Time, 0 No DST,
    //                 * -1 don'timeInSeconds know */
    tmPtr->tm_year += 1900;
    tmPtr->tm_mon += 1;
}

/** Refer interface header for description */
Retcode_T TimeStamp_SecsToTm(int64_t timeInSeconds, struct tm *tm)
{
    Retcode_T retcode = RETCODE_OK;

    long long numberOfDays = 0, numberOfSecs = 0;
    int remainingDays = 0, remainingSecs = 0, remainingYears = 0;
    int qcCycles = 0, cCycles = 0, qCycles = 0;
    int numberOfYears = 0, numberOfMonths = 0;
    static const char days_in_month[] = { 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 31, 29 };

    if (NULL == tm)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        /* Reject timeInSeconds values whose year would overflow int */
        if (timeInSeconds < INT_MIN * TIME_OVERFLOW_INT_MIN_MULTIPLIER || timeInSeconds > INT_MAX * TIME_OVERFLOW_INT_MAX_MULTIPLIER)
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
        }
        if (RETCODE_OK == retcode)
        {
            numberOfSecs = timeInSeconds - TIME_OFFSET;
            numberOfDays = numberOfSecs / SECS_TO_DAYS_FACTOR;
            remainingSecs = numberOfSecs % SECS_TO_DAYS_FACTOR;

            if (remainingSecs < 0)
            {
                remainingSecs += SECS_TO_DAYS_FACTOR;
                numberOfDays--;
            }

            qcCycles = numberOfDays / NO_OF_DAYS_PER_400_Y;
            remainingDays = numberOfDays % NO_OF_DAYS_PER_400_Y;
            if (remainingDays < 0)
            {
                remainingDays += NO_OF_DAYS_PER_400_Y;
                qcCycles--;
            }

            cCycles = remainingDays / NO_OF_DAYS_PER_100_Y;
            if (cCycles == 4)
            {
                cCycles--;
            }
            remainingDays -= cCycles * NO_OF_DAYS_PER_100_Y;

            qCycles = remainingDays / NO_OF_DAYS_PER_4_Y;
            if (qCycles == 25)
            {
                qCycles--;
            }
            remainingDays -= qCycles * NO_OF_DAYS_PER_4_Y;

            remainingYears = remainingDays / 365;
            if (remainingYears == 4)
            {
                remainingYears--;
            }
            remainingDays -= remainingYears * 365;

            numberOfYears = remainingYears + 4 * qCycles + 100 * cCycles + 400 * qcCycles;

            for (numberOfMonths = 0; (int) days_in_month[numberOfMonths] <= remainingDays; numberOfMonths++)
            {
                remainingDays -= days_in_month[numberOfMonths];
            }

            if (numberOfYears + 100 > INT_MAX || numberOfYears + 100 < INT_MIN)
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
            }
        }

        if (RETCODE_OK == retcode)
        {
            tm->tm_year = numberOfYears + 100;
            tm->tm_mon = numberOfMonths + 2;
            if (tm->tm_mon >= 12)
            {
                tm->tm_mon -= 12;
                tm->tm_year++;
            }
            tm->tm_mday = remainingDays + 1;

            tm->tm_hour = remainingSecs / 3600;
            tm->tm_min = remainingSecs / 60 % 60;
            tm->tm_sec = remainingSecs % 60;
        }
    }

    return retcode;
}

/** Refer interface header for description */
Retcode_T TimeStamp_TmToIso8601(struct tm * tmPtr, char * iso8601Format, uint8_t bufferSize)
{
    Retcode_T retcode = RETCODE_OK;
    char timezoneISO8601format[40] = { 0 };

    if (NULL == tmPtr)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        tm_to_realtime(tmPtr);

        sprintf(timezoneISO8601format, "%s%d-", timezoneISO8601format, (int) tmPtr->tm_year);
        if (tmPtr->tm_mon < 10)
        {
            sprintf(timezoneISO8601format, "%s0%d-", timezoneISO8601format, (int) tmPtr->tm_mon);
        }
        else
        {
            sprintf(timezoneISO8601format, "%s%d-", timezoneISO8601format, (int) tmPtr->tm_mon);
        }
        if (tmPtr->tm_mday < 10)
        {
            sprintf(timezoneISO8601format, "%s0%dT", timezoneISO8601format, (int) tmPtr->tm_mday);
        }
        else
        {
            sprintf(timezoneISO8601format, "%s%dT", timezoneISO8601format, (int) tmPtr->tm_mday);
        }
        if (tmPtr->tm_hour < 10)
        {
            sprintf(timezoneISO8601format, "%s0%d:", timezoneISO8601format, (int) tmPtr->tm_hour);
        }
        else
        {
            sprintf(timezoneISO8601format, "%s%d:", timezoneISO8601format, (int) tmPtr->tm_hour);
        }
        if (tmPtr->tm_min < 10)
        {
            sprintf(timezoneISO8601format, "%s0%d:", timezoneISO8601format, (int) tmPtr->tm_min);
        }
        else
        {
            sprintf(timezoneISO8601format, "%s%d:", timezoneISO8601format, (int) tmPtr->tm_min);
        }
        if (tmPtr->tm_sec < 10)
        {
            sprintf(timezoneISO8601format, "%s0%dZ", timezoneISO8601format, (int) tmPtr->tm_sec);
        }
        else
        {
            sprintf(timezoneISO8601format, "%s%dZ", timezoneISO8601format, (int) tmPtr->tm_sec);
        }

        printf("timezoneISO8601format - %s", timezoneISO8601format);
        printf("\r\n");

        if (strlen(timezoneISO8601format) > bufferSize)
        {
            iso8601Format = NULL;
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
        }
        else
        {
            memcpy(iso8601Format, timezoneISO8601format, strlen(timezoneISO8601format));
        }
    }
    return (retcode);
}

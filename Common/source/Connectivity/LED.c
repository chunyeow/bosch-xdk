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
 * This module handles the LED features.
 *
 */

/* module includes ********************************************************** */

/* own header files */
#include "XdkCommonInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_COMMON_ID_LED

#if XDK_CONNECTIVITY_LED
/* own header files */
#include "XDK_LED.h"

/* additional interface header files */
#include "BSP_BoardType.h"
#include "BCDS_BSP_LED.h"
#include "em_gpio.h"
#include "FreeRTOS.h"
#include "timers.h"

/* constant definitions ***************************************************** */

/* @todo - Add support in BSP to read the status of IO instead of em_gpio.h usage */
#define LED_ORANGE_PIN              (1)
#define LED_ORANGE_PORT             (gpioPortB)

#define LED_RED_PIN                 (12)
#define LED_RED_PORT                (gpioPortA)

#define LED_YELLOW_PIN              (0)
#define LED_YELLOW_PORT             (gpioPortB)

#define LED_INBUILT_ALL             (LED_INBUILT_RED | LED_INBUILT_ORANGE | LED_INBUILT_YELLOW)

/* local variables ********************************************************** */

static TimerHandle_t LEDTimerHandle = NULL;
static LED_T LEDBlinkColour = LED_INBUILT_INVALID;
static bool LEDBlinkIsOnCycle = false;
static bool LEDBlinkRollingPatternIsEnabled = false;
static uint32_t LEDBlinkOnTimeInMs = 0UL;
static uint32_t LEDBlinkOffTimeInMs = 0UL;

/* local functions ********************************************************* */

static Retcode_T LEDPatternRunning(void)
{
    static uint8_t count = 0;

    Retcode_T retcode = RETCODE_OK;
    if (0UL == count)
    {
        retcode = LED_On(LED_INBUILT_ORANGE);
        count++;
    }
    else if (1UL == count)
    {
        retcode = LED_On(LED_INBUILT_RED);
        count++;
    }
    else if (2UL == count)
    {
        retcode = LED_On(LED_INBUILT_ORANGE);
        count++;
    }
    else
    {
        retcode = LED_On(LED_INBUILT_YELLOW);
        count = 0;
    }

    return retcode;
}

static void LEDTimerCallbackHandler(TimerHandle_t timer)
{
    BCDS_UNUSED(timer);

    Retcode_T retcode = RETCODE_OK;

    if ( pdTRUE != xTimerStop(LEDTimerHandle, 0))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_LED_BLINK_TIMER_OPERATION_FAILED);
    }
    if (RETCODE_OK == retcode)
    {
        if (LEDBlinkRollingPatternIsEnabled)
        {
            retcode = LED_Off(LED_INBUILT_ALL);
            if (RETCODE_OK == retcode)
            {
                retcode = LEDPatternRunning();
            }
        }
        else
        {
            retcode = LED_Toggle(LEDBlinkColour);
        }
    }
    if (RETCODE_OK == retcode)
    {
        if ( pdTRUE != xTimerChangePeriod(LEDTimerHandle,
                pdMS_TO_TICKS((LEDBlinkRollingPatternIsEnabled?LEDBlinkOnTimeInMs:(LEDBlinkIsOnCycle ? LEDBlinkOffTimeInMs : LEDBlinkOnTimeInMs))), 0))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_LED_BLINK_TIMER_OPERATION_FAILED);
        }
        else
        {
            LEDBlinkIsOnCycle = !LEDBlinkIsOnCycle;
        }
    }

    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseError(retcode);
    }
}

/**
 * @brief This function will enable the LED Blink operation
 * - If connected then Orange LED is turned ON
 * - If disconnected then Rolling LED pattern is provided
 *
 * @param[in]   ledToBlink
 * The LED's to Blink
 * Unused if 'ledPattern' is 'true'
 *
 * @param[in]   onTimeInMs
 * LED ON time while blinking or running a pattern in ms
 *
 * @param[in]   offTimeInMs
 * LED OFF time while blinking in ms
 * Unused if 'ledPattern' is 'true'
 *
 */
static Retcode_T LEDBlinkEnable(LED_T ledToBlink, bool ledPattern, uint32_t onTimeInMs, uint32_t offTimeInMs)
{
    Retcode_T retcode = RETCODE_OK;

    if (NULL == LEDTimerHandle)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_UNINITIALIZED);
    }
    if (RETCODE_OK == retcode)
    {
        retcode = LED_On(ledToBlink);
    }
    if (RETCODE_OK == retcode)
    {
        if ( pdFALSE != xTimerIsTimerActive(LEDTimerHandle))
        {
            if ( pdTRUE != xTimerStop(LEDTimerHandle, 0))
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_LED_BLINK_TIMER_OPERATION_FAILED);
            }
        }
    }
    if (RETCODE_OK == retcode)
    {
        LEDBlinkOnTimeInMs = onTimeInMs;
        LEDBlinkOffTimeInMs = offTimeInMs;
        LEDBlinkIsOnCycle = true;
        LEDBlinkColour = ledToBlink;
        LEDBlinkRollingPatternIsEnabled = ledPattern;

        if ( pdTRUE != xTimerChangePeriod(LEDTimerHandle, pdMS_TO_TICKS(LEDBlinkOnTimeInMs), 0))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_LED_BLINK_TIMER_OPERATION_FAILED);
        }
    }

    return retcode;
}

static Retcode_T LEDBlinkDisable(void)
{
    Retcode_T retcode = RETCODE_OK;

    LEDBlinkIsOnCycle = false;
    LEDBlinkRollingPatternIsEnabled = false;

    if (NULL == LEDTimerHandle)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_UNINITIALIZED);
    }

    if (RETCODE_OK == retcode)
    {
        if (xTimerIsTimerActive(LEDTimerHandle) != pdFALSE)
        {
            if ( pdTRUE != xTimerStop(LEDTimerHandle, 0))
            {
                retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_LED_BLINK_TIMER_OPERATION_FAILED);
            }
        }
    }

    if (RETCODE_OK == retcode)
    {
        retcode = LED_Off(LED_INBUILT_ALL);
    }

    return retcode;
}

/** Refer interface header for description */
Retcode_T LED_Setup(void)
{
    Retcode_T retcode = RETCODE_OK;

    if (NULL == (LEDTimerHandle = xTimerCreate("LED_Blink", pdMS_TO_TICKS(10) /* dummy value */, pdFALSE, NULL, LEDTimerCallbackHandler)))
    {
        retcode = RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_OUT_OF_RESOURCES);
    }

    if (RETCODE_OK == retcode)
    {
        retcode = BSP_LED_Connect();
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T LED_Enable(void)
{
    return (BSP_LED_EnableAll());
}

/** Refer interface header for description */
Retcode_T LED_On(LED_T led)
{
    Retcode_T retcode = RETCODE_OK;

    if (!((led & (LED_INBUILT_RED | LED_INBUILT_ORANGE | LED_INBUILT_YELLOW))))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
    }
    if ((RETCODE_OK == retcode) && (led & LED_INBUILT_RED))
    {
        retcode = BSP_LED_Switch((uint32_t) BSP_XDK_LED_R, (uint32_t) BSP_LED_COMMAND_ON);
    }
    if ((RETCODE_OK == retcode) && (led & LED_INBUILT_ORANGE))
    {
        retcode = BSP_LED_Switch((uint32_t) BSP_XDK_LED_O, (uint32_t) BSP_LED_COMMAND_ON);
    }
    if ((RETCODE_OK == retcode) && (led & LED_INBUILT_YELLOW))
    {
        retcode = BSP_LED_Switch((uint32_t) BSP_XDK_LED_Y, (uint32_t) BSP_LED_COMMAND_ON);
    }

    return retcode;
}

/** Refer interface header for description */
Retcode_T LED_Off(LED_T led)
{
    Retcode_T retcode = RETCODE_OK;

    if (!((led & (LED_INBUILT_RED | LED_INBUILT_ORANGE | LED_INBUILT_YELLOW))))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
    }
    if ((RETCODE_OK == retcode) && (led & LED_INBUILT_RED))
    {
        retcode = BSP_LED_Switch((uint32_t) BSP_XDK_LED_R, (uint32_t) BSP_LED_COMMAND_OFF);
    }
    if ((RETCODE_OK == retcode) && (led & LED_INBUILT_ORANGE))
    {
        retcode = BSP_LED_Switch((uint32_t) BSP_XDK_LED_O, (uint32_t) BSP_LED_COMMAND_OFF);
    }
    if ((RETCODE_OK == retcode) && (led & LED_INBUILT_YELLOW))
    {
        retcode = BSP_LED_Switch((uint32_t) BSP_XDK_LED_Y, (uint32_t) BSP_LED_COMMAND_OFF);
    }

    return retcode;
}

/** Refer interface header for description */
Retcode_T LED_Toggle(LED_T led)
{
    Retcode_T retcode = RETCODE_OK;

    if (!((led & (LED_INBUILT_RED | LED_INBUILT_ORANGE | LED_INBUILT_YELLOW))))
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
    }
    if ((RETCODE_OK == retcode) && (led & LED_INBUILT_RED))
    {
        retcode = BSP_LED_Switch((uint32_t) BSP_XDK_LED_R, (uint32_t) BSP_LED_COMMAND_TOGGLE);
    }
    if ((RETCODE_OK == retcode) && (led & LED_INBUILT_ORANGE))
    {
        retcode = BSP_LED_Switch((uint32_t) BSP_XDK_LED_O, (uint32_t) BSP_LED_COMMAND_TOGGLE);
    }
    if ((RETCODE_OK == retcode) && (led & LED_INBUILT_YELLOW))
    {
        retcode = BSP_LED_Switch((uint32_t) BSP_XDK_LED_Y, (uint32_t) BSP_LED_COMMAND_TOGGLE);
    }

    return retcode;
}

/** Refer interface header for description */
Retcode_T LED_Blink(bool enable, LED_T ledToBlink, uint32_t onTimeInMs, uint32_t offTimeInMs)
{
    Retcode_T retcode = RETCODE_OK;

    if (enable)
    {
        retcode = LEDBlinkEnable(ledToBlink, false, onTimeInMs, offTimeInMs);
    }
    else
    {
        retcode = LEDBlinkDisable();
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T LED_Pattern(bool enable, LED_Pattern_T pattern, uint32_t statusTimeInMs)
{
    Retcode_T retcode = RETCODE_OK;

    if (LED_PATTERN_ROLLING != pattern)
    {
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
    }
    else
    {
        if (enable)
        {
            retcode = LEDBlinkEnable(LED_INBUILT_ALL, true, statusTimeInMs, 0UL);
        }
        else
        {
            retcode = LEDBlinkDisable();
        }
    }
    return retcode;
}

/** Refer interface header for description */
Retcode_T LED_Status(LED_T led, uint8_t * status)
{
    Retcode_T retcode = RETCODE_OK;

    switch (led)
    {
    case LED_INBUILT_RED:
        *status = GPIO_PinOutGet(LED_RED_PORT, LED_RED_PIN);
        break;
    case LED_INBUILT_ORANGE:
        *status = GPIO_PinOutGet(LED_ORANGE_PORT, LED_ORANGE_PIN);
        break;
    case LED_INBUILT_YELLOW:
        *status = GPIO_PinOutGet(LED_YELLOW_PORT, LED_YELLOW_PIN);
        break;
    default:
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INVALID_PARAM);
        break;
    }
    return retcode;
}

#endif /* XDK_CONNECTIVITY_LED */

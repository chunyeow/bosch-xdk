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
*  @ingroup CONFIGURATION
*
*  @defgroup BCDS_HALCONFIG BCDS_HALConfig
*  @{
*
*  @brief Contains a default configuration to enable or disable HAL components.
*
*  @details This header file is usually included by the BCDS_HAL.h from the
*  HAL (Hardware Abstraction Layer) module. It is used to allow a per project
*  configuration of the features provided by the HAL component.
*  Features are enabled or disabled by defining a particular feature to either
*  1 or 0 e.g.
*  @code{.c}
*  #define BCDS_FEATURE_I2C    1   // Enable HAL feature I2C
*  #define BCDS_FEATURE_SPI    0   // Disable HAL feature SPI
*  @endcode
*  If no defines are made then all HAL features will be enabled.
*
*  @file
*
*/

#ifndef BCDS_HAL_CONFIG_H_
#define BCDS_HAL_CONFIG_H_

/* Define nothing, so all features are enabled. */
#define BCDS_FEATURE_BSP_SENSOR_NODE        1
#define BCDS_FEATURE_BSP_SD_CARD            1
#define BCDS_FEATURE_I2C                    1
#if (BCDS_EMLIB_INCLUDE_USB == 1)
#define BCDS_FEATURE_BSP_USB                1         /**< USB feature is enabled only if emlib usb is included  */
#else
#define BCDS_FEATURE_BSP_USB                0
#endif
#define BCDS_FEATURE_SPI                    1
#define BCDS_FEATURE_BSP_LED                1
#define BCDS_FEATURE_BSP_BUTTON             1
#define BCDS_FEATURE_HAL_TICK_HANDLER       1
#define BCDS_FEATURE_FLASH                  1
#define BCDS_FEATURE_BSP_SD_CARD            1
#define BCDS_FEATURE_BSP_SD_CARD_AUTO_DETECT 1
#define BCDS_FEATURE_BSP_SD_CARD_AUTO_DETECT_FORCE 0
#define BCDS_FEATURE_UART                   1
#define BCDS_FEATURE_LEUART                 0
#define BCDS_FEATURE_GPIO                   1
#define BCDS_FEATURE_BSP_EXTENSION_GPIO     1
#define BCDS_FEATURE_BSP_MAX31865           1
#define BCDS_FEATURE_BSP_BT_EM9301          1
#define BCDS_FEATURE_BSP_WIFI_CC3100MOD     1
#define BCDS_FEATURE_DMA                    1
#define BCDS_FEATURE_EFM32_BURTC            0
#define BCDS_FEATURE_TIMER                  1
#define BCDS_FEATURE_EFM32_RTC              1
#define BCDS_FEATURE_BSP_EXTENSIONPORT      1
#ifndef BCDS_FEATURE_BSP_LORA_RN2XX3
#define BCDS_FEATURE_BSP_LORA_RN2XX3        (1)
#endif
#define BCDS_FEATURE_BSP_CHARGER_BQ2407X    1
#define BCDS_FEATURE_ADC                    1
#define BCDS_FEATURE_BSP_LEM_SENSOR			1
#define BCDS_FEATURE_BSP_IRSENSOR_NODE      1
#define BCDS_FEATURE_BSP_MIC_AKU340			1
#define BCDS_FEATURE_WATCHDOG               1
#define BCDS_FEATURE_SLEEP                  1

/** @todo this change is done only to make Refboards to compile. need to revert back after fixing the build settings of refboards*/
#define BCDS_FEATURE_BSP_UMTS_LISAU2_UART 0

#define BCDS_I2C_COUNT 1
#define BCDS_SPI_COUNT 2
#define BCDS_UART_COUNT 2

#if BCDS_FEATURE_BSP_SENSOR_NODE
#define BSP_XDK_SENSOR_BMA280_ENABLE_INTERRUPT      1
#define BSP_XDK_SENSOR_MAX44009_ENABLE_INTERRUPT    1
#define BSP_XDK_SENSOR_BMA280_FORCE_INTERRUPT       0

#ifndef BSP_XDK_SENSOR_BMG160_ENABLE_INTERRUPT
#define BSP_XDK_SENSOR_BMG160_ENABLE_INTERRUPT      0
#endif

#ifndef BSP_XDK_SENSOR_BMM150_ENABLE_INTERRUPT
#define BSP_XDK_SENSOR_BMM150_ENABLE_INTERRUPT      0
#endif

#define BSP_XDK_SENSOR_BMM150_FORCE_INTERRUPT       0

#ifndef BSP_XDK_SENSOR_BMI160_ENABLE_INTERRUPT
#define BSP_XDK_SENSOR_BMI160_ENABLE_INTERRUPT      0
#endif

#define BSP_XDK_SENSOR_BMI160_FORCE_INTERRUPT       0
#endif

#if BCDS_FEATURE_BSP_BUTTON
#define BSP_XDK_KEY1_INTERRUPT                                                  1
#define BSP_XDK_KEY1_INTERRUPT_FORCE                                            1
#define BSP_XDK_KEY2_INTERRUPT                                                  1
#define BSP_XDK_KEY2_INTERRUPT_FORCE                                            0
#endif /* BCDS_FEATURE_BSP_BUTTON */

#endif /*BCDS_HAL_CONFIG_H_*/

/**@}*/

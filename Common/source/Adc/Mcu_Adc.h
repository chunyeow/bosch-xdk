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
 * @brief Add a brief description here.
 *
 * @details Put here the documentation of this header file. Explain the interface exposed
 * by this header, e.g. what is the purpose of use, how to use it, etc.
 */
#ifndef _ADC_MCU_ADC_H_
#define _ADC_MCU_ADC_H_

#include "BCDS_HAL.h"
#include "BCDS_Basics.h"
#include "em_adc.h"

/* Include all headers which are needed by this file. */
/**
 * @brief ADC_T is a generic hardware handle,which is void pointer guaranteed to store pointer to integer
 */
typedef HWHandle_T ADC_T;
/**
 * @brief Enumerator to represent the Input channel, This is applicable for single trigger mode only
 */
enum Adc_Channel_E
{
    ADC_ENABLE_CH4,
    ADC_ENABLE_CH5,
    ADC_ENABLE_CH6,
    ADC_ENABLE_CH7,
    ADC_ENABLE_TEMP,
    ADC_ENABLE_VDDDIV3,
    ADC_ENABLE_VDD,
    ADC_ENABLE_VSS,
    ADC_ENABLE_VREFDIV2,
    ADC_ENABLE_CH_MAX
};
typedef enum Adc_Channel_E Adc_Channel_T;

/**
 * @brief Enumerator to represent the reference voltage
 */
enum Adc_Reference_E
{
    ADC_REF_1V25 = _ADC_SINGLECTRL_REF_1V25,/**< Internal 1.25V reference. */
    ADC_REF_2V5 = _ADC_SINGLECTRL_REF_2V5,/** Internal 2.5V reference. */
    ADC_REF_VDD = _ADC_SINGLECTRL_REF_VDD,/** Buffered VDD. */
    ADC_REF_5VDIFF = _ADC_SINGLECTRL_REF_5VDIFF,/** Internal differential 5V reference. */
    ADC_REF_ExtSingle = _ADC_SINGLECTRL_REF_EXTSINGLE, /** Single ended ext. ref. from pin 6. */
    ADC_REF_ExtDiff = _ADC_SINGLECTRL_REF_2XEXTDIFF, /** Differential ext. ref. from pin 6 and 7. */
    ADC_REF_2xVDD = _ADC_SINGLECTRL_REF_2XVDD, /** Unbuffered 2xVDD. */
};
typedef enum Adc_Reference_E Adc_Reference_T;

/**
 * @brief Enumerator to represent the resolution
 */
enum Adc_Resolution_E
{
    ADC_RESOLUTION_12BIT = _ADC_SINGLECTRL_RES_12BIT, /**< 12 bit sampling. */
    ADC_RESOLUTION_8BIT = _ADC_SINGLECTRL_RES_8BIT, /**< 8 bit sampling. */
    ADC_RESOLUTION_6BIT = _ADC_SINGLECTRL_RES_6BIT, /**< 6 bit sampling. */
    ADC_RESOLUTION_OVS = _ADC_SINGLECTRL_RES_OVS, /**< Oversampling. */
};
typedef enum Adc_Resolution_E Adc_Resolution_T;

/**
 * @brief Enumerator to represent the Acquisition time
 */
enum Adc_AcqTime_E
{
    ADC_ACQ_TIME_1 = _ADC_SINGLECTRL_AT_1CYCLE, /**< 1 clock cycle. */
    ADC_ACQ_TIME_2 = _ADC_SINGLECTRL_AT_2CYCLES, /**< 2 clock cycle. */
    ADC_ACQ_TIME_4 = _ADC_SINGLECTRL_AT_4CYCLES, /**< 4 clock cycle. */
    ADC_ACQ_TIME_8 = _ADC_SINGLECTRL_AT_8CYCLES, /**< 8 clock cycle. */
    ADC_ACQ_TIME_16 = _ADC_SINGLECTRL_AT_16CYCLES, /**< 16 clock cycle. */
    ADC_ACQ_TIME_32 = _ADC_SINGLECTRL_AT_32CYCLES, /**< 32 clock cycle. */
    ADC_ACQ_TIME_64 = _ADC_SINGLECTRL_AT_64CYCLES, /**< 64 clock cycle. */
    ADC_ACQ_TIME_128 = _ADC_SINGLECTRL_AT_128CYCLES, /**< 128 clock cycle. */
    ADC_ACQ_TIME_256 = _ADC_SINGLECTRL_AT_256CYCLES, /**< 256 Clock cycle. */

};
typedef enum Adc_AcqTime_E Adc_AcqTime_T;
/**
 * @brief Constant values to represent the Input channel, This is applicable for scan trigger mode only
 * @note  For multiple channel selection User needs to bitwise OR the respective Scan channels mentioned below for
 *        Proper operation
 */
enum Adc_ScanChannelMask_E
{
    ADC_ENABLE_CH4_SCAN = ADC_SCANCTRL_INPUTMASK_CH4,
    ADC_ENABLE_CH5_SCAN = ADC_SCANCTRL_INPUTMASK_CH5,
    ADC_ENABLE_CH6_SCAN = ADC_SCANCTRL_INPUTMASK_CH6,
};

/**
 * @brief Structure to represent the events that can be received from the ADC in the callback function.
 */
struct Mcu_Adc_Event_S
{
    uint32_t ScanComplete :1;
    uint32_t SingleComplete :1;
    uint32_t ErrorInConversion :1;
    uint32_t ScanReadDataOverflow :1;
    uint32_t SingleReadDataOverflow :1;
};
typedef struct Mcu_Adc_Event_S Mcu_Adc_Event_T;

/**
 * @brief union to combine event bit field structure with a uint32_t value
 */
union Mcu_Adc_Event_U
{
    struct Mcu_Adc_Event_S Bitfield;
    uint32_t RegisterValue;
};

/**
 * @brief struct to configure ADC in single mode
 */
struct Mcu_Adc_ConfigSingle_S
{
    Adc_Channel_T Channel;
    uint16_t* BufferPtr;
    Adc_Reference_T Reference;
    Adc_Resolution_T Resolution;
    Adc_AcqTime_T AcqTime;
};
typedef struct Mcu_Adc_ConfigSingle_S Mcu_Adc_ConfigSingle_T, *Mcu_Adc_ConfigSinglePtr_T;

/**
 * @brief struct to configure ADC in scan mode
 */
struct Mcu_Adc_ConfigScan_S
{
    uint32_t ChannelScanMask;
    uint16_t *BufferPtr;
    uint32_t SamplingRateInHz;
    uint16_t NoOfSamples;
    Adc_Reference_T Reference;
    Adc_Resolution_T Resolution;
    Adc_AcqTime_T AcqTime;
};
typedef struct Mcu_Adc_ConfigScan_S Mcu_Adc_ConfigScan_T, *Mcu_Adc_ConfigScanPtr_T;

/**
 * @brief Application callback template
 * @param[in] adc       MCU handle for ADC
 * @param[in] event     Represents the various application events
 * @param[in] bufferPtr Represents the pointer value of the result buffer for processing
 */
typedef void (*Mcu_Adc_Callback_T)(ADC_T adc, Mcu_Adc_Event_T event, uint16_t *bufferPtr);

/* Put the type and macro definitions here */

/* Put the function declarations here */

/**
 * @brief   Initialize the ADC Interface.
 *
 * @param [in]  adc : ADC handle.
 *
 * @param [in] callback : Function pointer to a callback function
 *                        which handles the ADC events.
 *
 * @retval  RETCODE_OK upon successful execution, error otherwise
 */
Retcode_T Mcu_Adc_Initialize(ADC_T adc, Mcu_Adc_Callback_T callback);

/**
 * @brief   De-initialize and powers down the ADC interface.
 *
 * @param [in]  adc : ADC handle.
 *
 * @retval RETCODE_OK upon successful execution, error otherwise
 */
Retcode_T Mcu_Adc_DeInitialize(ADC_T adc);

/**
 * @brief   Start Single Mode on ADC Interface.
 *
 * @param [in]  adc : ADC handle.
 *
 * @param [in] configStart : Structure which holds the configurations related to single mode.
 *
 * @retval  RETCODE_OK upon successful execution, error otherwise
 */
Retcode_T Mcu_Adc_StartSingle(ADC_T adc, Mcu_Adc_ConfigSinglePtr_T configStart);

/**
 * @brief   Start Scan Mode on ADC Interface & Start the PRS Timer as producer.
 *
 * @param [in]  adc : ADC handle.
 *
 * @param [in] configScan : Structure which holds the configurations related to scan mode.
 *
 * @Note The sampling rate must be limited to  minimum and maximum range mentioned by macro ADC_MIN_SAMPLING_FREQUENCY and ADC_MAX_SAMPLING_FREQUENCY respectively
 *
 * @retval  RETCODE_OK upon successful execution, error otherwise
 */
Retcode_T Mcu_Adc_StartScan(ADC_T adc, Mcu_Adc_ConfigScanPtr_T configScan);

/**
 * @brief   Stop the Scan mode on ADC Interface and stops the PRS timer used for sampling adc channels.
 *
 * @param [in]  adc : ADC handle.
 *
 * @retval RETCODE_OK upon successful execution, error otherwise
 */
Retcode_T Mcu_Adc_StopScan(ADC_T adc);

#endif /* _ADC_MCU_ADC_H_ */


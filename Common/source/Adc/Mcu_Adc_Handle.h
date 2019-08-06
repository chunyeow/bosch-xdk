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
#ifndef MCU_ADC_HANDLE_H_
#define MCU_ADC_HANDLE_H_

/* Include all headers which are needed by this file. */
#include "Mcu_Adc.h"
#include "em_adc.h"

/* Put the type and macro definitions here */
typedef void (*ADC_IRQ_Callback_T)(ADC_T Adc);
typedef void (*ADC_DMA_Callback_T)(ADC_T Adc);

struct Mcu_Adc_Driver_S
{
    Mcu_Adc_Callback_T AppCallback;  /**< Callback function given in Mcu_Adc_Initialize to notify application */
    uint32_t NumOfSamples;
    uint32_t NumOfScanChannels;
    void *   PointerValue;                   /**< Hook anything to it ,Reserved For future use */
    uint16_t *SingleBuffer;
    uint16_t *ScanBuffer1;
    uint16_t *ScanBuffer2;
    bool     Initialized;              /**< driver initialization state */
};
typedef struct Mcu_Adc_Driver_S Mcu_Adc_Driver_T;

/**
* @brief   Structure used as ADC handle.
* @detail  A pointer to this structure is wrapped in ADC_T for interface functions.
*
* @note This handle is a forward declared structure in BSP and
*/
struct Mcu_Adc_Handle_S
{
    ADC_TypeDef* Instance;                       /**< HW instance for the Adc set by the BSP */
    enum BCDS_HAL_TransferMode_E TransferMode;   /**< Set by BSP to tell MCU whether to use interrupt-mode or DMA */
    ADC_IRQ_Callback_T IRQCallback;              /**< Function invoked by BSP in case IRQ and interrupt mode */
    ADC_DMA_Callback_T DmaSingleCallback;        /**< Function invoked by BSP in case IRQ and Single Conversion Complete DMA mode */
    ADC_DMA_Callback_T DmaScanCallback;          /**< Function invoked by BSP in case  IRQ and Scan Conversion Complete DMA mode  */
    void * Link1RegPtr;                                /**< general purpose link register 1 (used for e.g. DMA Single Conversion Complete handle) */
    void * Link2RegPtr;                                /**< general purpose link register 2 (used for e.g. DMA Scan Conversion Complete handle) */
    Mcu_Adc_Driver_T   _DriverCtx;               /**< context pointer to driver context, private member */
};


/* Put the function declarations here */

#endif /* MCU_ADC_HANDLE_H_ */


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
 * @ingroup COMMON
 *
 * @defgroup EXTENSIONPORTSPI Extension port SPI
 * @{
 *
 * @brief This Module contains necessary implementation in order to enable user to develop his application using the
 *        Extension port SPI communication interface. This module can be used a reference design for interfacing user device
 *        via SPI of extension port
 *
 * @file
 */
#ifndef EXTENSIONPORTSPI_H_
#define EXTENSIONPORTSPI_H_

/* Include all headers which are needed by this file. */

#include "BCDS_Retcode.h"

/**
 * @brief   Enum to represent the bit order for the data sent via the extension port SPI.
 */
enum ExtensionPortSpi_BitOrder_E
{
    EXTENSION_PORT_SPI_BIT_ORDER_MSB_FIRST,
    EXTENSION_PORT_SPI_BIT_ORDER_LSB_FIRST,
};

/**
 * @brief   Typedef to represent the bit order for the data sent via the extension port SPI.
 */
typedef enum ExtensionPortSpi_BitOrder_E ExtensionPortSpi_BitOrder_T;

/**
 * @brief   Enum to represent the clock mode of the extension port SPI.
 */
enum ExtensionPortSpi_ClockMode_E
{
    EXTENSION_PORT_SPI_CLOCK_MODE_0, /* SPI mode 0: CLKPOL=0, CLKPHA=0. */
    EXTENSION_PORT_SPI_CLOCK_MODE_1, /* SPI mode 1: CLKPOL=0, CLKPHA=1. */
    EXTENSION_PORT_SPI_CLOCK_MODE_2, /* SPI mode 2: CLKPOL=1, CLKPHA=0. */
    EXTENSION_PORT_SPI_CLOCK_MODE_3, /* SPI mode 3: CLKPOL=1, CLKPHA=1. */
};

/**
 * @brief   Typedef to represent the clock mode of the extension port SPI.
 */
typedef enum ExtensionPortSpi_ClockMode_E ExtensionPortSpi_ClockMode_T;

/**
 * @brief   Structure to represent the configurations of the extension port SPI.
 */
struct ExtensionPortSpi_Config_S
{
    uint32_t BaudRate; /* Number of signals per second */
    ExtensionPortSpi_BitOrder_T BitOrder;
    ExtensionPortSpi_ClockMode_T ClockMode;
};

/**
 * @brief   Structure to represent the configuration of the extension port SPI.
 */
typedef struct ExtensionPortSpi_Config_S ExtensionPortSpi_Config_T;

/* Put the type and macro definitions here */

/* Put the function declarations here */
/**
 * @brief  This API initialize the SPI Interface Driver
 *
 * @retval #RETCODE_OK  in case of Spi Initialization Success.
 * @retval #RETCODE_OUT_OF_RESOURCES in case of Failed to create Semaphore to handle the interrupt or Failed to Set the Spi Configurations or While enabling the SPI.
 * @retval #RETCODE_NULL_POINTER in case of SPI Handle NULL.
 *
 * @return   In case of other failures refer #BSP_ExtensionPort_Connect API's.
 *
 * @note  The return value consist of (First 1 MSByte represents Package ID,
 *        Next byte represents Severity and Last 2 LSBytes represents error code).
 *
 */
Retcode_T ExtensionPortSpi_Initialize(ExtensionPortSpi_Config_T * config);

/**
 * @brief  This API will Read the value from the SPI interface driver
 *
 * @param[in]  readVal
 *          Buffer to hold the data received over SPI
 * @param[in]  readLength
 *          No of bytes to be read over the SPI bus
 *
 * @retval #RETCODE_OK in case of Spi Read Register Success.
 * @retval #RETCODE_TIMEOUT in case of interrupt not triggered.
 * @retval #RETCODE_NULL_POINTER in case of SPI Handle NULL.
 *
 * @note  The return value consist of (First 1 MSByte represents Package ID,
 *        Next byte represents Severity and Last 2 LSBytes represents error code).
 *
 */
Retcode_T ExtensionPortSpi_Read(uint8_t *readVal, uint32_t readLength, uint32_t timeout);

/**
 * @brief  This API will Write the Value to the SPI interface driver
 *
 * @param[in]  writeVal
 *          Buffer to hold the data to be transmitted over SPI
 * @param[in]  writeLength
 *          No of bytes to be transmitted over the SPI bus
 *
 * @retval #RETCODE_OK in case of Spi Write value on passing Register Success.
 * @retval #RETCODE_TIMEOUT in case of interrupt not triggered.
 * @retval #RETCODE_NULL_POINTER in case of SPI Handle NULL.
 *
 * @note  The return value consist of (First 1 MSByte represents Package ID,
 *        Next byte represents Severity and Last 2 LSBytes represents error code).
 *
 */
Retcode_T ExtensionPortSpi_Write(uint8_t *writeVal, uint32_t writeLength, uint32_t timeout);

/**
 * @brief  This API Deinitialize the SPI interface driver
 *
 * @retval #RETCODE_OK in case of Spi De-Initialization Success.
 * @retval #RETCODE_UNINITIALIZED in case of spi not initialized.
 *
 * @note  The return value consist of (First 1 MSByte represents Package ID,
 *        Next byte represents Severity and Last 2 LSBytes represents error code).
 *
 */
Retcode_T ExtensionPortSpi_DeInitialize(void);

#endif /* EXTENSIONPORTSPI_H_ */
/**@}*/

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
 * @brief Configuration header for the Logging module.
 *
 */

#ifndef LOG_CONFIG_H
#define LOG_CONFIG_H

/* Configuration for the Asyncronous Recorder */
#define LOG_BUFFER_SIZE             (UINT16_C(256))
#define LOG_QUEUE_BUFFER_SIZE       (UINT16_C(2048))
#define LOG_TASK_STACK_SIZE         (UINT16_C(128))
#define LOG_TASK_PRIORITY           (UINT8_C(4))
#define LOG_FILTER_ITEM_COUNT       (UINT8_C(8))

/* Configuration for the UART Appender */
#define LOG_GPIO_TX_PRT             (gpioPortB)
#define LOG_GPIO_TX_PIN             (UINT8_C(3))
#define LOG_GPIO_TX_EN              (gpioModePushPullDrive)
#define LOG_GPIO_TX_DIS             (gpioModeDisabled)
#define LOG_SER_HWTYPE              (SER_HWTYPE_USART)
#define LOG_SER_PORT                (USART2)
#define LOG_SER_ROUTE_LOCATION      (USART_ROUTE_LOCATION_LOC1)
#define LOG_SER_PROTOCOL            (SER_UART_PROTOCOL)
#define LOG_SER_BAUDRATE            (UINT16_C(115200))
#define LOG_SER_HW_FLOW_CTRL        (UINT8_C(0))
#define LOG_SER_PARITY              (usartNoParity)
#define LOG_SER_DATABITS            (usartDatabits8)
#define LOG_SER_STOPBITS            (usartStopbits1)
#define LOG_SER_CMU_CLOCK           (cmuClock_USART2)
#define LOG_DMA_CHAN                (UINT8_C(0))

/* Default log level for package and module */
#define LOG_LEVEL_PACKAGE_DEFAULT   (LOG_LEVEL_INFO)
#define LOG_LEVEL_MODULE_DEFAULT    (LOG_LEVEL_INFO)

#endif /* LOG_CONFIG_H */

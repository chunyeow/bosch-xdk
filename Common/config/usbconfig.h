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

/**
 * @ingroup CONFIGURATION
 *
 * @defgroup USBCONFIG usbconfig
 * @{
 *
 * @brief Configuration header for the USB module
 *  \tableofcontents
 *  \section intro_sec USB
 *
 *  @details
 *
 *  @file
 *
 */
/* header definition ******************************************************** */
#ifndef XDK110_USBCONFIG_H_
#define XDK110_USBCONFIG_H_

/* public interface declaration ********************************************* */

/* public type and macro definitions */
#define USB_SERIAL_NUMBER_LENGTH       UINT8_C(16)
#define USB_DEVICE                                       /**< USB DEVICE mode or HOST mode must be defined */

#define USB_PRODUCT_ID 0x017B                            /**< The product Id is defined here*/

#define BCDS_PRODUCT_NAME 'X','D','K',' ','A','p','p','l','i','c','a','t','i','o','n',             /**< The product name is defined here*/
#define BCDS_SERIAL_NO '1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6'             /**< The Serial number is defined here*/

/**< retargetio.c must be included if you are enabling USB_USE_PRINTF */

#define USB_USE_PRINTF                                   /**< When using printf , the string is retargetted to USB . The transfer is blocking at present. */

#define NUM_EP_USED                     UINT8_C(3)       /**< Specify number of endpoints used (in addition to EP0).*/
#define USB_BULK_SIZE                   UINT8_C(64)      /**< This is the max. packet size.        */
#define NUM_APP_TIMERS                  UINT8_C(0)       /**< Specify number of application timers you need. Receving or Transmitting Data is not dependent of this macro. */
#define USB_SET_BAUDRATE                UINT32_C(0)      /**< Baudrate. Receving or Transmitting Data is not dependent of this macro.                                      */
#define USB_SET_STOPBITS_MODE           UINT8_C(0)       /**< Stop bits, 0=1 1=1.5 2=2. Receving or Transmitting Data is not dependent of this macro.                      */
#define USB_SET_PARITY                  UINT8_C(0)       /**< 0=None 1=Odd 2=Even 3=Mark 4=Space. Receving or Transmitting Data is not dependent of this macro.            */
#define USB_SET_DATA_BITS               UINT8_C(0)       /**< 0, 5, 6, 7, 8 or 16. Receving or Transmitting Data is not dependent of this macro.                           */
#define USB_TIMER                       USB_TIMER1       /**< Specify the TImer1 for the USB_TIMER  */

/* public function prototype declarations */

/* public global variable declarations */

/* inline function definitions */

#endif /* XDK110_USBCONFIG_H_ */

/** ************************************************************************* */

/**@}*/

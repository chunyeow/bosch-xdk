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
 * @defgroup SENSORHWCONFIG SensorHwConfig
 * @{
 *
 * @brief Sensors axis re-map configuration
 *
 * @details
 *
 * @file
 *
 */

/* header definition ******************************************************** */
#ifndef XDK110_SENSORSHWCONFIG_H_
#define XDK110_SENSORSHWCONFIG_H_

/* public interface declaration ********************************************* */

/* public type and macro definitions */
/********************BMA2X2**********************/
#define RMP_BMA2X2_X_AXIS_CONFIG			UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMA2X2_Y_AXIS_CONFIG			UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMA2X2_Z_AXIS_CONFIG			UINT8_C(0x00)   /**< @see RMP module */

#define RMP_BMA2X2_X_AXIS_SIGN_CONFIG		UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMA2X2_Y_AXIS_SIGN_CONFIG		UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMA2X2_Z_AXIS_SIGN_CONFIG		UINT8_C(0x00)   /**< @see RMP module */

#define BMA2x2SENSOR_CHIPID                 UINT8_C(0xFB)   /**< chip id of BMA2X2 sensor */

/********************BMG160**********************/
#define RMP_BMG160_X_AXIS_CONFIG            UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMG160_Y_AXIS_CONFIG            UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMG160_Z_AXIS_CONFIG            UINT8_C(0x00)   /**< @see RMP module */

#define RMP_BMG160_X_AXIS_SIGN_CONFIG       UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMG160_Y_AXIS_SIGN_CONFIG       UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMG160_Z_AXIS_SIGN_CONFIG       UINT8_C(0x00)   /**< @see RMP module */

#define BMG160S_CHIPID                      UINT8_C(0x0F)   /**< chip id of BMG160 sensor */

/********************BMM150**********************/
#define RMP_BMM150_X_AXIS_CONFIG			UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMM150_Y_AXIS_CONFIG			UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMM150_Z_AXIS_CONFIG			UINT8_C(0x00)   /**< @see RMP module */

#define RMP_BMM150_X_AXIS_SIGN_CONFIG		UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMM150_Y_AXIS_SIGN_CONFIG		UINT8_C(0x02)   /**< @see RMP module */
#define RMP_BMM150_Z_AXIS_SIGN_CONFIG		UINT8_C(0x04)   /**< @see RMP module */
#define BMM150_SENSORCHIP_ID                UINT8_C(0x32)   /**< chip id of BMM150 Sensor */

/********************BME280**********************/
#define BME280S_CHIPID                      UINT8_C(0x60)   /**< chip id of BME280 sensor */

/********************BMI160**********************/
#define BMI160S_CHIP_ID                     UINT8_C(0xD1)   /**< chip id of BMI160 sensor */

#define BMA280_START_UP_TIME 				UINT32_C(3)		/**< start up time delay for BMA280 sensor */
#define BME280_START_UP_TIME 				UINT32_C(10)	/**< start up time delay for BME280 sensor */
#define MAX44009_START_UP_TIME 				UINT32_C(10)	/**< start up time delay for MAX44009 sensor */
#define BMG160_START_UP_TIME 				UINT32_C(30)	/**< start up time delay for BMG160 sensor */
#define BMI160_ACCEL_START_UP_TIME 			UINT32_C(10)	/**< start up time delay for BMI160 Accel sensor */
#define BMI160_GYRO_START_UP_TIME 			UINT32_C(10)	/**< start up time delay for BMI160 Gyro sensor */
#define BMM150_START_UP_TIME                UINT32_C(4)     /**< start up time delay for BMM150 sensor */

#define RMP_BMIACCEL_X_AXIS_CONFIG          UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMIACCEL_Y_AXIS_CONFIG          UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMIACCEL_Z_AXIS_CONFIG          UINT8_C(0x00)   /**< @see RMP module */

#define RMP_BMIACCEL_X_AXIS_SIGN_CONFIG     UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMIACCEL_Y_AXIS_SIGN_CONFIG     UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMIACCEL_Z_AXIS_SIGN_CONFIG     UINT8_C(0x00)   /**< @see RMP module */

#define RMP_BMIGYRO_X_AXIS_CONFIG           UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMIGYRO_Y_AXIS_CONFIG           UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMIGYRO_Z_AXIS_CONFIG           UINT8_C(0x00)   /**< @see RMP module */

#define RMP_BMIGYRO_X_AXIS_SIGN_CONFIG      UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMIGYRO_Y_AXIS_SIGN_CONFIG      UINT8_C(0x00)   /**< @see RMP module */
#define RMP_BMIGYRO_Z_AXIS_SIGN_CONFIG      UINT8_C(0x00)   /**< @see RMP module */

/********************Grideye**********************/
/** Grid-EYE's number of image pixels */
#define GRIDEYE_IMAGEWIDTH               UINT8_C(8)
#define GRIDEYE_IMAGEHEIGHT              UINT8_C(8)
#define GRIDEYE_PIXEL_TEMPERATURE_REGSZ  UINT8_C(2)
#define GRIDEYE_PIXEL_SIZE               (GRIDEYE_IMAGEWIDTH*GRIDEYE_IMAGEHEIGHT*GRIDEYE_PIXEL_TEMPERATURE_REGSZ)

#endif/*(XDK110_SENSORSHWCONFIG_H_)*/

/** ************************************************************************* */

/**@}*/

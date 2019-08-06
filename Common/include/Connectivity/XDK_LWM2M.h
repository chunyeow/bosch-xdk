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
 * @ingroup CONNECTIVITY
 *
 * @defgroup LWM2M LWM2M
 * @{
 *
 * @brief Interface header file for the LWM2M feature.
 * It supports the following standard OMA objects
 * - Object ID 3 : Device
 * - Object ID 4 : Connectivity monitoring
 * - Object ID 5 : Firmware update
 * It supports the following standard IPSO (OMA standardized) objects
 * - Object ID 3301 : Illuminance
 * - Object ID 3303 : Temperature
 * - Object ID 3304 : Humidity
 * - Object ID 3310 : Light control
 * - Object ID 3313 : Accelerometer
 * - Object ID 3314 : Magnetometer
 * - Object ID 3315 : Barometer
 * - Object ID 3334 : Gyroscope
 * It supports the following standard XDK (BCDS vendor specific) objects
 * - Object ID 15000 : Sensor device
 * - Object ID 15003 : Alert notification
 *
 * @file
 */

/* header definition ******************************************************** */
#ifndef XDK_LWM2M_H_
#define XDK_LWM2M_H_

/* local interface declaration ********************************************** */
#include "BCDS_Retcode.h"
#include "BCDS_CmdProcessor.h"

/* public type and macro definitions */

/**
 * @brief   Enum to represent the supported LWM2M security modes.
 */
enum LWM2M_SecurityMode_E
{
    LWM2M_SECURITY_MODE_PRE_SHARED_KEY, /**< Pre-Shared Key */
    LWM2M_SECURITY_MODE_RAW_PUBLIC_KEY, /**< Raw Public Key. Only a place holder. Not supported, yet. */
    LWM2M_SECURITY_MODE_X_509_CERTIFICATE, /**< X.509 Certificate. Only a place holder. Not supported, yet. */
};

/**
 * @brief   Typedef to represent the supported LWM2M security mode.
 */
typedef enum LWM2M_SecurityMode_E LWM2M_SecurityMode_T;

/**
 * @brief   Structure to configure the LWM2M security mode -Pre-Shared Key values.
 */
struct LWM2M_SecurityPreSharedKeyInfo_S
{
    const char* Identity; /**< Pre-Shared Key - Identity value */
    const char* Key; /**< Pre-Shared Key - Key value */
    uint32_t KeyLength; /**< Pre-Shared Key - Key length */
};

/**
 * @brief   Typedef to configure the LWM2M security mode -Pre-Shared Key value.
 */
typedef struct LWM2M_SecurityPreSharedKeyInfo_S LWM2M_SecurityPreSharedKeyInfo_T;

/**
 * @brief   Enum to represent the LWM2M status notifications.
 */
enum LWM2M_StatusNotification_E
{
    LWM2M_REGISTRATION_SUCCESS, /**< LWM2M device registration succeeded */
    LWM2M_REGISTRATION_FAILURE, /**< LWM2M device registration failed */
    LWM2M_REGISTRATION_UPDATE_SUCCESS, /**< LWM2M device registration update succeeded */
    LWM2M_REGISTRATION_UPDATE_FAILURE, /**< LWM2M device registration update failed */
    LWM2M_DEREGISTRATION_SUCCESS, /**< LWM2M device de-registration succeeded */
    LWM2M_DEREGISTRATION_FAILURE, /**< LWM2M device de-registration failed */
    LWM2M_NOTIFICATION_SUCCESS, /**< LWM2M device notification succeeded */
    LWM2M_NOTIFICATION_FAILURE, /**< LWM2M device notification failed */
    LWM2M_UNCLASSIFIED_EVENT, /**< LWM2M device unclassified event */
};

/**
 * @brief   Enum to represent the LWM2M test mode.
 *
 * If test mode is LWM2M_TEST_MODE_NO:
 * - XDK in-built LEDs are explicitly controlled by the application and Light control object is denied access.
 * - Buttons are not expected to notify LWM2M alerts. If they do, they are discarded.
 * If test mode is LWM2M_TEST_MODE_YES:
 * - XDK In-built LEDs are explicitly controlled by Light control object (IPSO ID 3310).
 * - The LED's are used to indicate the LWM2M registration state.
 *   | LED | OFF | ON | blinking (1s/1s) | blinking (3s/1s) |
 *   |-----|-----|----|------------------|------------------|
 *   | Yellow | x | x | XDK is running | XDK is running, but configuration is not detected properly |
 *   | Orange | x | Device is registered in the LWM2M server | Device is updating the registration in the LWM2M server | x |
 *   | Red | x | Device is not registered in the LWM2M server | Device is registering in the LWM2M server | x |
 * - Buttons are expected to notify LWM2M alerts.
 * If test mode is LWM2M_TEST_MODE_MIX:
 * - XDK in-built LEDs are shared by the application and Light control object.
 * - When ever the application wants to change a state of an LED, then it is persistent for the below duration:
 *   - XDK In-built Red LED : 5 seconds
 *   - XDK In-built Orange LED : 5 seconds
 *   - XDK In-built Yellow LED : 2 seconds
 * - Buttons are expected to notify LWM2M alerts.
 */
enum LWM2M_TestMode_E
{
    LWM2M_TEST_MODE_NO = 0,
    LWM2M_TEST_MODE_YES = 1,
    LWM2M_TEST_MODE_MIX = 2,
};

/**
 * @brief   Enum to represent the LWM2M test mode.
 *
 */
typedef enum LWM2M_TestMode_E LWM2M_TestMode_T;

/**
 * @brief   Typedef to represent the LWM2M status notification.
 */
typedef enum LWM2M_StatusNotification_E LWM2M_StatusNotification_T;

/**
 * @brief   Typedef of the LWM2M status notification callback.
 *
 * @param [in] status
 * run-time status of the LWM2M stack
 */
typedef void (*LWM2M_StatusNotificationCB_T)(LWM2M_StatusNotification_T status);

/**
 * @brief   struct to represent the LWM2M setup features.
 */
struct LWM2M_Setup_S
{
    CmdProcessor_T * CmdProcessorHandle; /**< Command processor handle to handle LWM2M stack events and servicing. */
    bool IsSecure; /**< Boolean representing if we enable DTLS for secure communication. */
    LWM2M_SecurityMode_T SecurityMode; /**< LWM2M security mode. Unused if IsSecure is false. Only LWM2M_SECURITY_MODE_PRE_SHARED_KEY is supported, yet. */
    LWM2M_SecurityPreSharedKeyInfo_T SecurityPreSharedKey; /**< LWM2M security mode Pre-Shared Key configurations. Unused if IsSecure is false. */
    const char* EndPointName; /**< LWM2M device end point name */
    const char * Binding; /**< LWM2M Binding type. "U" for UDP or "UQ" for UDP-queued */
    uint32_t Lifetime; /**< LWM2M lifetime in seconds for a duration of which if there is no data exchange with the server, the registration is invalidated. */
    bool ConfirmableNotification; /**< boolean representing if the notifications are Confirmable */
    const char* MACAddr; /**< Defines the MAC address value for the sensor device object */
    LWM2M_TestMode_T TestMode; /**< Defines the mode for light control object */
    const char * ServerURL; /**< The URL pointing to the LWM2M sever without the protocol prefix (coap:// or coaps://) and port number suffix. */
    uint16_t ServerPort; /**< The port number of the LWM2M sever */
    uint16_t ClientPort; /**< The port number of the LWM2M client */
    LWM2M_StatusNotificationCB_T StatusNotificationCB; /**< LWM2M status notification callback. Can be NULL if the application is not interested. */
};

/**
 * @brief   Enum to represent the LWM2M Connectivity Monitoring parameters configurable by the user.
 */
enum LWM2M_ConnectivityMonitoring_E
{
    LWM2M_SET_IP_ADDRESS,
    LWM2M_SET_RADIO_SIGNAL_STRENGTH,
};

/**
 * @brief   Typedef to represent the LWM2M Connectivity Monitoring parameter configurable by the user.
 */
typedef enum LWM2M_ConnectivityMonitoring_E LWM2M_ConnectivityMonitoring_T;
/**
 * @brief   Typedef to represent the LWM2M setup feature.
 */
typedef struct LWM2M_Setup_S LWM2M_Setup_T;

/**
 * @brief   Enum to represent the possible safe LED configurations for the
 * application when the Light control is not confined only with the LWM2M module,
 * but also shared for the application.
 */
enum LWM2M_LedMixMode_E
{
    LWM2M_LED_MIX_MODE_RED_ON,
    LWM2M_LED_MIX_MODE_RED_OFF,
    LWM2M_LED_MIX_MODE_YELLOW_ON,
    LWM2M_LED_MIX_MODE_YELLOW_OFF,
    LWM2M_LED_MIX_MODE_ORANGE_ON,
    LWM2M_LED_MIX_MODE_ORANGE_OFF,
};

/**
 * @brief   Typedef to represent the possible safe LED configuration for the
 * application when the Light control is not confined only with the LWM2M module,
 * but also shared for the application.
 */
typedef enum LWM2M_LedMixMode_E LWM2M_LedMixMode_T;

/**
 * @brief This will setup the LWM2M
 *
 * @param[in] setup
 * Pointer to the LWM2M setup feature
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - This must be the first API to be called if LWM2M feature is to be used.
 * - Do not call this API more than once.
 */
Retcode_T LWM2M_Setup(LWM2M_Setup_T * setup);

/**
 * @brief This will enable the LWM2M
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #LWM2M_Setup must have been successful prior.
 * - Do not call this API more than once.
 */
Retcode_T LWM2M_Enable(void);

/**
 * @brief The function initiates the LWM2M registration of the node to the LWM2M Server.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #LWM2M_Enable must have been successful prior.
 * - #WLAN_Enable must have been successful prior.
 * - Do not call this API more than once.
 */
Retcode_T LWM2M_Register(void);

/**
 * @brief The function forcefully triggers the LWM2M registration of the node to the
 * LWM2M Server irrespective of the current node status.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #LWM2M_Enable must have been successful prior.
 */
Retcode_T LWM2M_TriggerForceRegister(void);

/**
 * @brief The function initiates an alert notification.
 *
 * @param[in] value
 * Pointer to the string to be notified to the server via Alert Notification object resource.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #LWM2M_Register must have been successful prior.
 * - The value is considers as a string and hence must be NULL terminated.
 * - The maximum size of value string must not exceed 128 bytes including the NULL terminator.
 */
Retcode_T LWM2M_AlertNotification(char * value);

/**
 * @brief The function sets LWM2M Connectivity Monitoring parameters.
 *
 * @param[in] notify
 * Parameter configurable by the user.
 *
 * @param[in] value
 * If notify is LWM2M_SET_IP_ADDRESS, this is a const char * string value.
 * If notify is LWM2M_SET_RADIO_SIGNAL_STRENGTH, this is a int value.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #LWM2M_Register must have been successful prior.
 */
Retcode_T LWM2M_ConnectivityMonitoring(LWM2M_ConnectivityMonitoring_T notify, void * value);

/**
 * @brief The function initiates provides a safe LED configuration for the application
 * when the Light control is not confined only with the LWM2M module,
 * but also shared for the application.
 *
 * @param[in] state
 * User expected LED state.
 *
 * @return  RETCODE_OK on success, or an error code otherwise.
 *
 * @note
 * - #LWM2M_Register must have been successful prior.
 * - Do not use this when .
 */
Retcode_T LWM2M_LedMixModeControl(LWM2M_LedMixMode_T state);

#endif /* XDK_LWM2M_H_ */

/**@} */

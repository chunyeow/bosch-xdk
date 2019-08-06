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
/*  @file
 *
 *  @brief
 *  This file permits the user to configure/customize the BLE stack and
 *  profile settings. This will override the previous definitions.
 * 
 */

/**
 *  @ingroup CONFIGURATION
 *
 *  @defgroup BLECUSTOMCONFIG BLECustomConfig
 *  @{
 *
 *  @brief This file permits the user to configure/customize the BLE stack and
 *  profile settings. This will override the previous definitions.
 *
 *  @details
 *
 *  @file
 *
 */

/* header definition ******************************************************** */
#ifndef BCDS_BLE_CUSTOM_CONFIG_PH_H_
#define BCDS_BLE_CUSTOM_CONFIG_PH_H_

/* local interface declaration ********************************************** */

/* local type and macro definitions */

/** Macros to override the platform specific options */
#define BLE_EM9301_LOADPATCH                                    0
#define BLE_PROTECT_ISR_FUNCTION_CALL                           1
#define BLE_USE_RESTRICTED_LOCAL_MEMORY                         1
#define BLETYPES_ALREADY_DEFINED                                0
#define BLECONTROLLER_NEED_SPECIFIC_INIT                        1
#define BLECONTROLLER_USED                       BLECONTROLLER_EM
#define BLE_PUBLIC_ADDRESS_GETTER_CALLBACK                      1

/** The declaration of the constant is to be placed in the code section */
#define CONST_DECL                                              const
#define UNUSED_PARAMETER(P)                                     (P = P)

/** Macros to override the core stack options */
#define BLE_ROLE_SCANNER                                        0
#define BLE_ROLE_ADVERTISER                                     1
#define BLE_ADVERTSING_TX_POWER_SUPPORT                         0
#define BLE_PARMS_CHECK                                         1
#define BLEERROR_HANDLER                                        0
#define BLE_CONNECTION_SUPPORT                                  1
#define BLE_RANDOM_ADDRESSING_SUPPORT                           1
#define BLE_SECURITY                                            1
#define BLE_SM_DYNAMIC_IO_CAPABILITIES                          0
#define BLE_SM_SIGNATURE_SUPPORT                                0
#define BLE_WHITE_LIST_SUPPORT                                  0
#define BLE_ENABLE_TEST_COMMANDS                                0
#define BLE_USE_HOST_CHANNEL_CLASSIFICATION                     0
#define BLE_ENABLE_VENDOR_SPECIFIC                              1
#define BLEL2CAP_ENABLE_API                                     0
#define SM_IO_CAPABILITIES                  SM_IO_NOINPUTNOOUTPUT
#define BLE_SM_SLAVE_RESOLVE_MASTER_RANDOM_ADDRESS              1
/** Macros to override the ATT options */
#define ATTRIBUTE_PROTOCOL                                      1
#define ATT_ROLE_CLIENT                                         0

/** Macros to override the GATT options */
#define GENERIC_ATTRIBUTE_PROFILE                               0
#define BLEGATT_SUPPORT_ALL_SERVICE_DISCOVERY                   0
#define BLEGATT_SUPPORT_SINGLE_SERVICE_DISCOVERY                1
#define BLEGATT_SUPPORT_RELATIONSHIP_DISCOVERY                  0
#define BLEGATT_SUPPORT_SINGLE_CHARACTERISTIC_DISCOVERY         0
#define BLEGATT_SUPPORT_ALL_CHARACTERISTIC_DISCOVERY            1
#define BLEGATT_SUPPORT_READ_CHARACTERISTIC_VALUE_BY_HANDLE     0
#define BLEGATT_SUPPORT_READ_CHARACTERISTIC_VALUE_BY_TYPE       1
#define BLEGATT_SUPPORT_READ_LONG_CHARACTERISTIC_VALUE          0
#define BLEGATT_SUPPORT_GET_CHARACTERISTIC_DESCRIPTOR_LIST      0
#define BLEGATT_SUPPORT_WRITE_CHARACTERISTIC_WITHOUT_RESPONSE   0
#define BLEGATT_SUPPORT_WRITE_CHARACTERISTIC_VALUE              1
#define BLEGATT_SUPPORT_WRITE_LONG_CHARACTERISTIC_VALUE         0
#define BLESPECIFICATION_COMPLIANCE                             BLESPECIFICATION_4_1
#define BLEL2CAP_CONNECTION_ORIENTED_CHANNELS                   0
#define BLE_ADVERTISING_DIRECTED_LOW_DUTY_CYCLE_SUPPORT         0
#define ATT_SERVER_SUPPORT_32_BIT_UUID                          0

/** Macros to override the different Alpwise options */
#define BLEIBEACON_SUPPORT_REPORTER                             0
#define BLEALERTNOTIFICATION_SUPPORT_CLIENT                     0
#define BLEALERTNOTIFICATION_SUPPORT_SERVER                     0
#define BLEALPWDATAEXCHANGE_SUPPORT_CLIEN                       0
#define BLEAPPLENOTIFICATIONCENTER_SUPPORT_CONSUMER             0
#define BLEBLOODPRESSURE_SUPPORT_COLLECTOR                      0
#define BLEBLOODPRESSURE_SUPPORT_SENSOR                         0
#define BLECYCLINGPOWER_SUPPORT_COLLECTOR                       0
#define BLECYCLINGPOWER_SUPPORT_SENSOR                          0
#define BLECYCLINGSPEEDANDCADENCE_SUPPORT_COLLECTOR             0
#define BLECYCLINGSPEEDANDCADENCE_SUPPORT_SENSOR                0
#define BLEFINDME_SUPPORT_LOCATOR                               0
#define BLEFINDME_SUPPORT_TARGET                                0
#define BLEGLUCOSE_SUPPORT_COLLECTOR                            0
#define BLEGLUCOSE_SUPPORT_SENSOR                               0
#define BLEHEALTHTHERMOMETER_SUPPORT_COLLECTOR                  0
#define BLEHEALTHTHERMOMETER_SUPPORT_THERMOMETER                0
#define BLEHEARTRATE_SUPPORT_COLLECTOR                          0
#define BLEHEARTRATE_SUPPORT_SENSOR                             0
#define BLEHID_SUPPORT_DEVICE                                   0
#define BLEHUMANINTERFACEDEVICE_SUPPORT_HOST                    0
#define BLELOCATIONANDNAVIGATION_SUPPORT_COLLECTOR              0
#define BLELOCATIONANDNAVIGATION_SUPPORT_SENSOR                 0
#define BLENETWORKAVAILABILITY_SUPPORT_MONITOR                  0
#define BLENETWORKAVAILABILITY_SUPPORT_REPORTER                 0
#define BLEPHONEALERTSTATUS_SUPPORT_CLIENT                      0
#define BLEPHONEALERTSTATUS_SUPPORT_SERVER                      0
#define BLEPROXIMITY_SUPPORT_MONITOR                            0
#define BLEPROXIMITY_SUPPORT_REPORTER                           0
#define BLERUNNINGSPEEDANDCADENCE_SUPPORT_COLLECTOR             0
#define BLERUNNINGSPEEDANDCADENCE_SUPPORT_SENSOR                0
#define BLETIME_SUPPORT_CLIENT                                  0
#define BLETIME_SUPPORT_SERVER                                  0
#define BLEWEIGHTSCALE_SUPPORT_COLLECTOR                        0
#define BLEWEIGHTSCALE_SUPPORT_SENSOR                           0

/** Macros to override the LOGGING options */
#define BLE_PRINT_DEBUG_TRACES                                  0
#define ATT_DEBUG_TRACES                                        0
#define SMP_DEBUG_TRACES                                        0
#if (BLE_PRINT_DEBUG_TRACES == 1)
#define BLEHCI_HANDLER                                          0
#endif //(BLE_PRINT_DEBUG_TRACES == 1)

/** Macros to override the PROFILES options */
#define BLEALPWDATAEXCHANGE_SUPPORT_SERVER                      1
#define BLE_SUPPORT_ALPWDATAEXCHANGE_SERVICE                    1
/* local function prototype declarations */

/* local module global variable declarations */

/* local inline function definitions */

#endif /* BCDS_BLE_CUSTOM_CONFIG_PH_H_ */
/**@}*/

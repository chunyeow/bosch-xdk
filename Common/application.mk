##################################################################
#      Makefile for generating the xdk application               #
##################################################################
MAKE_FILE_ROOT ?= .

BCDS_PLATFORM_PATH := $(MAKE_FILE_ROOT)/../Platform
BCDS_LIBRARIES_PATH := $(MAKE_FILE_ROOT)/../Libraries
BCDS_XDK_TOOLS_PATH := $(MAKE_FILE_ROOT)/../Tools
include $(dir $(BCDS_COMMON_MAKEFILE))common_settings.mk
include common_feature.mk

#XDK application specific Paths 
BCDS_XDK_COMMON_PATH =$(MAKE_FILE_ROOT)/../Common
BCDS_XDK_CONFIG_PATH = $(BCDS_XDK_COMMON_PATH)/config
FLASH_TOOL_PATH = $(BCDS_DEVELOPMENT_TOOLS)/JLinkDebugger/$(BCDS_JLINK_VERSION)/JLink
BCDS_PACKAGE_ID = 153
BCDS_XDK_DEBUG_DIR = debug
BCDS_XDK_RELEASE_DIR = release
BCDS_XDK_INCLUDE_DIR = $(BCDS_XDK_COMMON_PATH)/include
BCDS_XDK_SOURCE_DIR = $(BCDS_XDK_COMMON_PATH)/source
BCDS_XDK_LEGACY_INCLUDE_DIR = $(BCDS_XDK_COMMON_PATH)/legacy/include
BCDS_XDK_CERTS_DIR = $(BCDS_XDK_COMMON_PATH)/certs
BCDS_XDK_OBJECT_DIR = objects
BCDS_XDK_APP_PATH= $(BCDS_APP_DIR)
BCDS_APP_NAME_WITH_HEADER = $(BCDS_APP_NAME)_with_header

export XDK_FOTA_ENABLED_BOOTLOADER ?=0
export BCDS_DEVICE_ID = EFM32GG390F1024
export BCDS_SYSTEM_STARTUP_METHOD ?= DEFAULT_STARTUP
export BCDS_TOOL_CHAIN_PATH ?= $(BCDS_GCC_PATH)
export BCDS_TARGET_PLATFORM = efm32
export BCDS_DEVICE_TYPE = EFM32GG
# The below defined values is to update firmware version formed by given MAJOR MINOR and PATCH 
BCDS_FOTA_TOOL_PATH =  $(BCDS_XDK_TOOLS_PATH)/Fota_Tools
MAJOR_SW_NO ?= 0x00#Defines MAJOR number and maximum value is 255
MINOR_SW_NO ?= 0x00#Defines MINOR number and maximum value is 255
PATCH_SW_NO ?= 0x01#Defines PATCH number and maximum value is 255
HEADER_VERSION =0100#Defines the current used header version
PRODUCT_CLASS =0001#(Productcode[12bit]=001 for APLM, Minimum Hardwarerevision[4bit]
PRODUCT_VARIANT =0001
FIRMWARE_VERSION = $(subst 0x,,00$(MAJOR_SW_NO)$(MINOR_SW_NO)$(PATCH_SW_NO))
CREATE_CONTAINER_SCRIPT = $(BCDS_FOTA_TOOL_PATH)/create_fota_container.py

#Flag to enable temporary debug logging in the application. Recommended to be enabled for testing purpose only.
DEBUG_LOGGING = 0

# These serval macros should be above Libraies.mk
# This variable should fully specify the build configuration of the Serval 
# Stack library with regards the enabled and disabled features of TLS to use DTLS instead of TLS. 
SERVAL_ENABLE_TLS_CLIENT?=0
SERVAL_ENABLE_TLS_ECC?=0
SERVAL_ENABLE_TLS_PSK?=0
SERVAL_ENABLE_DTLS?=0
SERVAL_ENABLE_DTLS_PSK?=0
SERVAL_MAX_NUM_MESSAGES?=16
SERVAL_MAX_SIZE_APP_PACKET ?=600
SERVAL_ENABLE_TLS ?=0
SERVAL_ENABLE_DTLS_ECC ?=0
SERVAL_TLS_MBEDTLS ?= 0
XDK_MBEDTLS_PARSE_INFO ?= 0

# This variable should fully specify the build configuration of the Serval 
# Stack library with regards the enabled and disabled features as well as 
# the configuration of each enabled feature.
BCDS_SERVALSTACK_MACROS += \
	-D SERVAL_LOG_LEVEL=SERVAL_LOG_LEVEL_ERROR\
	-D SERVAL_ENABLE_HTTP_CLIENT=1\
	-D SERVAL_ENABLE_HTTP_SERVER=1\
	-D SERVAL_ENABLE_WEBSERVER=1\
	-D SERVAL_ENABLE_COAP_OBSERVE=1\
	-D SERVAL_ENABLE_COAP_CLIENT=1\
	-D SERVAL_ENABLE_COAP_SERVER=1\
	-D SERVAL_ENABLE_REST_CLIENT=1\
	-D SERVAL_ENABLE_REST_SERVER=1\
	-D SERVAL_ENABLE_REST_HTTP_BINDING=1\
	-D SERVAL_ENABLE_REST_COAP_BINDING=1\
	-D SERVAL_ENABLE_XUDP=1\
	-D SERVAL_ENABLE_DPWS=0\
	-D SERVAL_ENABLE_TLS_CLIENT=$(SERVAL_ENABLE_TLS_CLIENT)\
	-D SERVAL_ENABLE_TLS_SERVER=0\
	-D SERVAL_ENABLE_TLS_ECC=$(SERVAL_ENABLE_TLS_ECC)\
	-D SERVAL_ENABLE_TLS_PSK=$(SERVAL_ENABLE_TLS_PSK)\
	-D SERVAL_ENABLE_DTLS=$(SERVAL_ENABLE_DTLS)\
	-D SERVAL_ENABLE_DTLS_CLIENT=$(SERVAL_ENABLE_DTLS)\
	-D SERVAL_ENABLE_DTLS_SERVER=$(SERVAL_ENABLE_DTLS)\
	-D SERVAL_ENABLE_DTLS_PSK=$(SERVAL_ENABLE_DTLS_PSK)\
	-D SERVAL_ENABLE_HTTP_AUTH=1\
	-D SERVAL_ENABLE_HTTP_AUTH_DIGEST=1\
	-D SERVAL_ENABLE_DUTY_CYCLING=1\
	-D SERVAL_ENABLE_APP_DATA_ACCESS=0\
	-D SERVAL_ENABLE_COAP_COMBINED_SERVER_AND_CLIENT=$(SERVAL_ENABLE_DTLS)\
	-D SERVAL_ENABLE_COAP_OBSERVE=1\
	-D SERVAL_ENABLE_LWM2M=1\
	-D SERVAL_ENABLE_XTCP=1\
	-D SERVAL_ENABLE_XTCP_SERVER=1\
	-D SERVAL_ENABLE_XTCP_CLIENT=1\
	-D SERVAL_HTTP_MAX_NUM_SESSIONS=3\
	-D SERVAL_HTTP_SESSION_MONITOR_TIMEOUT=4000\
	-D SERVAL_MAX_NUM_MESSAGES=$(SERVAL_MAX_NUM_MESSAGES)\
	-D SERVAL_MAX_SIZE_APP_PACKET=$(SERVAL_MAX_SIZE_APP_PACKET)\
	-D SERVAL_MAX_SECURE_SOCKETS=5\
	-D SERVAL_MAX_SECURE_CONNECTIONS=5\
	-D SERVAL_SECURE_SERVER_CONNECTION_TIMEOUT=300000\
	-D SERVAL_DOWNGRADE_TLS=1\
	-D SERVAL_ENABLE_DTLS_ECC=$(SERVAL_ENABLE_DTLS_ECC) \
	-D SERVAL_ENABLE_DTLS_RSA=0 \
	-D COAP_MSG_MAX_LEN=224 \
	-D COAP_MAX_NUM_OBSERVATIONS=50 \
	-D LWM2M_MAX_NUM_OBSERVATIONS=50 \
	-D SERVAL_LWM2M_SECURITY_INFO_MAX_LENGTH=65 \
	-D LWM2M_IP_ADDRESS_MAX_LENGTH=65 \
	-D SERVAL_HTTP_MAX_LENGTH_URL=256 \
	-D SERVAL_TLS_MBEDTLS=$(SERVAL_TLS_MBEDTLS) \
	-D LWM2M_MAX_LENGTH_DEVICE_NAME=32 \
	-D SERVAL_ENABLE_DTLS_SESSION_ID=0 \
	-D LWM2M_DISABLE_CLIENT_QUEUEMODE=1 \
	-D PAL_MAX_NUM_ADDITIONAL_COMM_BUFFERS=6 \
	-D SERVAL_ENABLE_DTLS_HEADER_LOGGING=0 \
	-D SERVAL_DTLS_FLIGHT_MAX_RETRIES=4 \
	-D SERVAL_EXPERIMENTAL_DTLS_MONITOR_EXTERN=0 \
	-D SERVAL_POLICY_STACK_CALLS_TLS_API=1 \
	-D SERVAL_SECURITY_API_VERSION=2 \
	-D SERVAL_ENABLE_HTTP_RANGE_HANDLING=1 \
	-D SERVAL_ENABLE_MQTT=1 \
	-D SERVAL_ENABLE_TLS=$(SERVAL_ENABLE_TLS) \
	-D COAP_OVERLOAD_QUEUE_SIZE=15 \
	-D SERVAL_ENABLE_SNTP_CLIENT=1 \
	-D SERVAL_ENABLE_HTTP=1 \
	-D BCDS_SERVAL_COMMBUFF_SEND_BUFFER_MAX_LEN=1000 \
	-D SERVAL_XML_PARSER=1 \
	-D XDK_MBEDTLS_PARSE_INFO=$(XDK_MBEDTLS_PARSE_INFO)
	

include Libraries.mk

# Build chain settings
ifneq ("$(wildcard $(BCDS_TOOL_CHAIN_PATH))","")
CC = $(BCDS_TOOL_CHAIN_PATH)/arm-none-eabi-gcc
AR = $(BCDS_TOOL_CHAIN_PATH)/arm-none-eabi-ar
OBJCOPY = $(BCDS_TOOL_CHAIN_PATH)/arm-none-eabi-objcopy
else
CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
endif
RMDIRS := rm -rf

#This flag is used to generate dependency files 
DEPEDENCY_FLAGS = -MMD -MP -MF $(@:.o=.d)

#Path of the XDK debug object files
BCDS_XDK_APP_DEBUG_OBJECT_DIR = $(BCDS_XDK_APP_PATH)/$(BCDS_XDK_DEBUG_DIR)/$(BCDS_XDK_OBJECT_DIR)
BCDS_XDK_APP_DEBUG_DIR = $(BCDS_XDK_APP_PATH)/$(BCDS_XDK_DEBUG_DIR)

#Path of the XDK Release object files
BCDS_XDK_APP_RELEASE_OBJECT_DIR =  $(BCDS_XDK_APP_PATH)/$(BCDS_XDK_RELEASE_DIR)/$(BCDS_XDK_OBJECT_DIR)
BCDS_XDK_APP_RELEASE_DIR = $(BCDS_XDK_APP_PATH)/$(BCDS_XDK_RELEASE_DIR)

export BCDS_CFLAGS_COMMON += -std=c99 -Wall -Wextra -Wstrict-prototypes -D $(BCDS_DEVICE_ID) -D BCDS_TARGET_EFM32 \
-mcpu=cortex-m3 -mthumb -ffunction-sections -fdata-sections \
$(BCDS_SERVALSTACK_MACROS) -D $(BCDS_SYSTEM_STARTUP_METHOD) $(XDK_CFLAGS_COMMON) \
-D ENABLE_DMA -D ARM_MATH_CM3 -D XDK_FOTA_ENABLED_BOOTLOADER=$(XDK_FOTA_ENABLED_BOOTLOADER) \
-D BCDS_SERVALPAL_WIFI=$(BCDS_SERVALPAL_WIFI) -D BCDS_EMLIB_INCLUDE_USB=$(BCDS_EMLIB_INCLUDE_USB) \
-D BCDS_FREERTOS_INCLUDE_AWS=$(BCDS_FREERTOS_INCLUDE_AWS) -D DEBUG_LOGGING=$(DEBUG_LOGGING) -D mqttDO_NOT_USE_CUSTOM_CONFIG -D MBEDTLS_CONFIG_FILE='<MbedtlsConfig.h>' \
-D__SL__ -D__OSI__

LDFLAGS_DEBUG = -Xlinker -Map=$(BCDS_XDK_APP_DEBUG_DIR)/$(BCDS_APP_NAME).map \
-mcpu=cortex-m3 -mthumb -T $(BCDS_XDK_LD_FILE) -Wl,--gc-sections

ASMFLAGS = -x assembler-with-cpp -Wall -Wextra -mcpu=cortex-m3 -mthumb

export BCDS_CFLAGS_DEBUG_COMMON ?= $(BCDS_CFLAGS_COMMON) -O0 -g
export BCDS_CFLAGS_DEBUG = $(BCDS_CFLAGS_DEBUG_COMMON)
export BCDS_CFLAGS_DEBUG_WITHOUT_ASSERT = $(BCDS_CFLAGS_DEBUG_COMMON) -DNDEBUG

export BCDS_CFLAGS_RELEASE_COMMON ?= $(BCDS_CFLAGS_COMMON) -O0 -DNDEBUG
export BCDS_CFLAGS_RELEASE = $(BCDS_CFLAGS_RELEASE_COMMON)

LDFLAGS_RELEASE = -Xlinker -Map=$(BCDS_XDK_APP_RELEASE_DIR)/$(BCDS_APP_NAME).map \
-mcpu=cortex-m3 -mthumb -T $(BCDS_XDK_LD_FILE) -Wl,--gc-sections

LIBS = -Wl,--start-group -lgcc -lc -lm  -Wl,--end-group

#The static libraries of the platform and third party sources are grouped here. Inorder to scan the libraries
#for undefined reference again and again, the libraries are listed between the --start-group and --end-group.
BCDS_DEBUG_LIBS_GROUP = -Wl,--start-group $(BCDS_LIBS_DEBUG) $(BCDS_THIRD_PARTY_LIBS) -Wl,--end-group

BCDS_RELEASE_LIBS_GROUP = -Wl,--start-group $(BCDS_LIBS_RELEASE) $(BCDS_THIRD_PARTY_LIBS) -Wl,--end-group

ifneq ($(XDK_FOTA_ENABLED_BOOTLOADER),0)
XDK_APP_ADDRESS = 0x00020000 # @see efm32gg_new.ld, This is the flash start address if EA commander tool used for flashing 
BCDS_XDK_LD_FILE = efm32gg_new.ld
$(info new_bootloader)
else
XDK_APP_ADDRESS = 0x00010000 # @see efm32gg.ld , This is the flash start address if EA commander tool used for flashing 
BCDS_XDK_LD_FILE = efm32gg.ld
$(info old_bootloader)
endif

# Define the path for connectivity certificates (both server and the device).
# By default initialed to an empty certificate. Application is expected to overwrite the same, if needed.
XDK_APP_CERTIFICATE_NAME ?= Custom

# Define the path for include directories.
BCDS_XDK_INCLUDES += \
                  -I $(BCDS_XDK_INCLUDE_DIR) \
                  -I $(BCDS_XDK_INCLUDE_DIR)/Connectivity \
                  -I $(BCDS_XDK_INCLUDE_DIR)/Sensor \
                  -I $(BCDS_XDK_SOURCE_DIR)/Protected \
                  -I $(BCDS_XDK_INCLUDE_DIR)/Utility \
                  -I $(BCDS_XDK_LEGACY_INCLUDE_DIR) \
                  -I $(BCDS_XDK_LEGACY_INCLUDE_DIR)/ServalPAL_WiFi \
                  -I $(BCDS_XDK_LEGACY_INCLUDE_DIR)/BLE \
                  -I $(BCDS_XDK_CERTS_DIR)/XDKDummy \
                  -I $(BCDS_XDK_CERTS_DIR)/$(XDK_APP_CERTIFICATE_NAME) \
                  -I $(BCDS_XDK_CONFIG_PATH) \
                  -I $(BCDS_XDK_CONFIG_PATH)/Drivers \
                  -I $(BCDS_XDK_CONFIG_PATH)/FOTA \
                  -I $(BCDS_XDK_CONFIG_PATH)/Utils \
				  -I $(BCDS_XDK_CONFIG_PATH)/ServalPal \
				  -I $(BCDS_XDK_CONFIG_PATH)/AmazonFreeRTOS/FreeRTOS \
				  -I $(BCDS_XDK_CONFIG_PATH)/AmazonFreeRTOS \
	   		      -I $(BCDS_BLE_PATH)/include \
	   		      -I $(BCDS_BLE_PATH)/include/services \
                  -I $(BCDS_ESSENTIALS_PATH)/include \
                  -I $(BCDS_DRIVERS_PATH)/include \
                  -I $(BCDS_SENSORS_PATH)/include \
				  -I $(BCDS_SENSORS_UTILS_PATH)/include \
                  -I $(BCDS_SERVALPAL_PATH)/include \
                  -I $(BCDS_UTILS_PATH)/include \
                  -I $(BCDS_BSP_PATH)/include \
				  -I $(BCDS_SENSOR_TOOLBOX_PATH)/include \
				  -I $(BCDS_FOTA_PATH)/include \
				  -I $(BCDS_FOTA_PATH)/source/protected \
				  -I $(BCDS_WLAN_PATH)/include \
				  -I $(BCDS_XDK_CONFIG_PATH)/Essentials \
				  -I $(BCDS_XDK_CONFIG_PATH)/LoRaDrivers \
				  -I $(BCDS_ESSENTIALS_PATH)/include/bsp \
				  -I $(BCDS_ESSENTIALS_PATH)/include/mcu/efm32 \
				  -I $(BCDS_ESSENTIALS_PATH)/include/mcu \
				  -I $(BCDS_LORA_DRIVERS_PATH)/include \
				  -I $(BCDS_XDK_COMMON_PATH)/source/Adc \
				  -I $(BCDS_XDK_COMMON_PATH)/source \
				  -I $(BCDS_XDK_COMMON_PATH)/source/Private/ServalStack/src/TLS_MbedTLS \
				  -I $(BCDS_XDK_INCLUDE_DIR)/Connectivity/LWM2M \
				  -I $(BCDS_XDK_CONFIG_PATH)/MbedTLS

# By using -isystem, headers found in that direcwtory will be considered as system headers, because of that 
# all warnings, other than those generated by #warning, are suppressed.	
BCDS_XDK_EXT_INCLUDES += \
				  -isystem $(BCDS_SERVALSTACK_LIB_PATH)/include \
				  -isystem $(BCDS_SERVALSTACK_LIB_PATH)/3rd-party/ServalStack/api \
				  -isystem $(BCDS_SERVALSTACK_LIB_PATH)/3rd-party/ServalStack/pal \
				  -isystem $(BCDS_FREERTOS_PATH)/3rd-party/include \
				  -isystem $(BCDS_FREERTOS_PATH)/3rd-party/include/private \
				  -isystem $(BCDS_FREERTOS_PATH)/3rd-party/FreeRTOS/portable/GCC/ARM_CM3 \
                  -isystem $(BCDS_EMLIB_PATH)/3rd-party/EMLib/emlib/inc \
                  -isystem $(BCDS_EMLIB_PATH)/3rd-party/EMLib/Device/SiliconLabs/EFM32GG/Include \
                  -isystem $(BCDS_EMLIB_PATH)/3rd-party/EMLib/CMSIS/Include \
	              -isystem $(BCDS_FATFSLIB_PATH)/3rd-party/fatfs/src \
	              -isystem $(BCDS_BLE_CORE_PATH) \
                  -isystem $(BCDS_BLE_INTERFACE_PATH) \
                  -isystem $(BCDS_BLE_INTERFACE_PATH)/ATT \
                  -isystem $(BCDS_BLE_INTERFACE_PATH)/Services \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_ALPWDataExchange/Interfaces \
				  -isystem $(BCDS_WIFILIB_PATH)/3rd-party/TI/simplelink/include \
				  -isystem $(BCDS_WIFILIB_PATH)/include \
				  -isystem $(BCDS_WIFILIB_PATH)/3rd-party/TI \
				  -isystem $(BCDS_WIFILIB_PATH)/3rd-party/TI/oslib \
				  -isystem $(BCDS_WIFILIB_PATH)/3rd-party/TI/netapps \
				  -isystem $(BCDS_WIFILIB_PATH)/3rd-party/TI/netapps/http/client \
				  -isystem $(BCDS_WIFILIB_PATH)/3rd-party/TI/simplelink/source \
				  -isystem $(BCDS_BSX_LIB_PATH)/BSX4/Source/algo/algo_bsx/Inc \
				  -isystem $(BCDS_BSX_LIB_PATH)/BSX4/Source/device/API/BMI160_API \
				  -isystem $(BCDS_BSX_LIB_PATH)/BSX4/Source/device/API/BMM050_API \
				  -isystem $(BCDS_BSX_LIB_PATH)/BSX4/Source/device/API/BMG160_API \
				  -isystem $(BCDS_BSX_LIB_PATH)/BSX4/Source/device/API/BMA2x2_API \
				  -isystem $(BCDS_GRIDEYE_LIB_PATH)/3rd-party/GridEye/Source \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_AlertNotification/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_AppleNotificationCenter/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_BloodPressure/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_CyclingPower/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_CyclingSpeedAndCadence/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_FindMe/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_Glucose/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_HealthThermometer/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_HeartRate/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_HumanInterfaceDevice/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_iBeacon/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_LocationAndNavigation/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_PhoneAlertStatus/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_Proximity/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_RunningSpeedAndCadence/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_Time/Interfaces \
				  -isystem $(BCDS_BLE_SERVICE_PATH)/BLESW_WeightScale/Interfaces \
				  -isystem $(BCDS_SERVALSTACK_LIB_PATH)/3rd-party/ServalStack/src/inc \
				  -isystem $(BCDS_MBED_LIB_PATH)/3rd-party/mbedtls/include \
				  -isystem $(BCDS_MBED_LIB_PATH)/3rd-party/mbedtls/include/mbedtls

BCDS_XDK_PLATFORM_ALL_SOURCE_FILES += \
	$(wildcard $(BCDS_XDK_COMMON_PATH)/source/*.c) \
	$(wildcard $(BCDS_XDK_COMMON_PATH)/legacy/source/*.c) \
	$(wildcard $(BCDS_XDK_COMMON_PATH)/source/**/*.c) \
	$(wildcard $(BCDS_XDK_COMMON_PATH)/source/**/**/*.c) \
	$(wildcard $(BCDS_XDK_COMMON_PATH)/source/**/**/**/*.c) \
	$(wildcard $(BCDS_XDK_COMMON_PATH)/source/**/**/**/**/*.c)

# list of other Source files required for XDK application
ifneq ($(XDK_CONNECTIVITY_LWM2M),1)
BCDS_XDK_SKIP_FILES_LIST += $(wildcard $(BCDS_XDK_COMMON_PATH)/source/**/LWM2M/*.c)
endif

ifneq ($(SERVAL_TLS_MBEDTLS),1)
ifneq (,$(filter 1,$(SERVAL_ENABLE_TLS) $(SERVAL_ENABLE_DTLS)))
BCDS_XDK_SKIP_FILES_LIST += $(wildcard $(BCDS_XDK_COMMON_PATH)/source/Private/ServalStack/src/TLS_MbedTLS/*.c)
endif
endif  

BCDS_XDK_PLATFORM_SOURCE_FILES := \
	$(filter-out $(BCDS_XDK_SKIP_FILES_LIST), $(BCDS_XDK_PLATFORM_ALL_SOURCE_FILES))

ifeq ($(BCDS_EMLIB_INCLUDE_USB),1)		
BCDS_XDK_EXT_INCLUDES += \
	-isystem $(BCDS_EMLIB_PATH)/3rd-party/EMLib/usb/inc

BCDS_XDK_LIB_SOURCE_FILES += \
	$(BCDS_EMLIB_PATH)/3rd-party/EMLib/usb/src/em_usbdint.c
endif

# Startup file for XDK application	
BCDS_XDK_APP_STARTUP_FILES = \
	startup_efm32gg.S
	
# Debug Object files list for building XDK application
BCDS_XDK_PLATFORM_COMMOM_C_OBJECT_FILES = $(BCDS_XDK_PLATFORM_SOURCE_FILES:.c=.o)
BCDS_XDK_PLATFORM_COMMOM_C_OBJECT_FILES += $(BCDS_XDK_LIB_SOURCE_FILES:.c=.o)
BCDS_XDK_PLATFORM_C_OBJECT_FILES = $(subst $(BCDS_XDK_COMMON_PATH)/,,$(BCDS_XDK_PLATFORM_COMMOM_C_OBJECT_FILES))
BCDS_XDK_APP_S_OBJECT_FILES = $(BCDS_XDK_APP_STARTUP_FILES:.S=.o)
BCDS_XDK_APP_C_OBJECT_FILES = $(patsubst $(BCDS_APP_SOURCE_DIR)/%.c, %.o, $(BCDS_XDK_APP_SOURCE_FILES))
BCDS_XDK_APP_OBJECT_FILES =  $(BCDS_XDK_PLATFORM_C_OBJECT_FILES) $(BCDS_XDK_APP_C_OBJECT_FILES) $(BCDS_XDK_APP_S_OBJECT_FILES) 
BCDS_XDK_APP_OBJECT_FILES_DEBUG = $(addprefix $(BCDS_XDK_APP_DEBUG_OBJECT_DIR)/, $(BCDS_XDK_APP_OBJECT_FILES))

# Release Object files list for building XDK application
BCDS_XDK_APP_OBJECT_FILES_RELEASE = $(addprefix $(BCDS_XDK_APP_RELEASE_OBJECT_DIR)/, $(BCDS_XDK_APP_OBJECT_FILES))

# Dependency File List for building XDK application files 
BCDS_XDK_APP_DEPENDENCY_RELEASE_FILES = $(addprefix $(BCDS_XDK_APP_RELEASE_OBJECT_DIR)/, $(BCDS_XDK_APP_OBJECT_FILES:.o=.d))
BCDS_XDK_APP_DEPENDENCY_DEBUG_FILES = $(addprefix $(BCDS_XDK_APP_DEBUG_OBJECT_DIR)/, $(BCDS_XDK_APP_OBJECT_FILES:.o=.d))

#Create debug binary
.PHONY: debug 
debug: $(BCDS_XDK_APP_DEBUG_DIR)/$(BCDS_APP_NAME).bin

#Create release binary
.PHONY: release 
release: $(BCDS_XDK_APP_RELEASE_DIR)/$(BCDS_APP_NAME).bin


# Clean project
.PHONY: clean clean_libraries

clean: clean_libraries
	@echo "Cleaning project in app.mk"
	$(RMDIRS) $(BCDS_XDK_APP_DEBUG_DIR) $(BCDS_XDK_APP_RELEASE_DIR)

#Compile, assemble and link for debug target
#Compile the sources from plaform or library
$(BCDS_XDK_APP_DEBUG_OBJECT_DIR)/%.o: %.c
	@mkdir -p $(@D)
	@echo "Building file $<"
	$(CC) $(DEPEDENCY_FLAGS) $(BCDS_CFLAGS_DEBUG_COMMON) $(BCDS_XDK_INCLUDES) $(BCDS_XDK_EXT_INCLUDES) -DBCDS_PACKAGE_ID=$(BCDS_PACKAGE_ID) -c $< -o $@
	
#Compile the sources from application
$(BCDS_XDK_APP_DEBUG_OBJECT_DIR)/%.o: $(BCDS_APP_SOURCE_DIR)/%.c 
	@mkdir -p $(@D)
	@echo $(BCDS_XDK_APP_PATH)
	@echo "Building file $<"
	@$(CC) $(DEPEDENCY_FLAGS) $(BCDS_CFLAGS_DEBUG_COMMON) $(BCDS_XDK_INCLUDES) $(BCDS_XDK_EXT_INCLUDES) -DBCDS_PACKAGE_ID=$(BCDS_PACKAGE_ID) -c $< -o $@
	
$(BCDS_XDK_APP_DEBUG_OBJECT_DIR)/%.o: %.S
	@mkdir -p $(@D)
	@echo "Assembling $<"
	@$(CC) $(ASMFLAGS) $(BCDS_XDK_INCLUDES) $(BCDS_XDK_EXT_INCLUDES) -c $< -o $@
	
$(BCDS_XDK_APP_DEBUG_DIR)/$(BCDS_APP_NAME).out: $(BCDS_LIBS_DEBUG) $(BCDS_XDK_APP_OBJECT_FILES_DEBUG) 
	@echo "Creating .out $@"
	@$(CC) $(LDFLAGS_DEBUG) $(BCDS_XDK_APP_OBJECT_FILES_DEBUG) $(BCDS_DEBUG_LIBS_GROUP) $(LIBS) -o $@
	
$(BCDS_XDK_APP_DEBUG_DIR)/$(BCDS_APP_NAME).bin: $(BCDS_XDK_APP_DEBUG_DIR)/$(BCDS_APP_NAME).out
	@echo "Boot flag value is $(XDK_FOTA_ENABLED_BOOTLOADER)"
	@echo "Creating binary for debug $@"
	@$(OBJCOPY) -O binary $(BCDS_XDK_APP_DEBUG_DIR)/$(BCDS_APP_NAME).out $@
	

#Compile, assemble and link for release target
#Compile the sources from plaform or library
$(BCDS_XDK_APP_RELEASE_OBJECT_DIR)/%.o: %.c
	@mkdir -p $(@D)
	@echo "Building file $<"
	@$(CC) $(DEPEDENCY_FLAGS) $(BCDS_CFLAGS_RELEASE_COMMON) $(BCDS_XDK_INCLUDES) $(BCDS_XDK_EXT_INCLUDES)  -DBCDS_PACKAGE_ID=$(BCDS_PACKAGE_ID) -c $< -o $@

#Compile the sources from application
$(BCDS_XDK_APP_RELEASE_OBJECT_DIR)/%.o: $(BCDS_APP_SOURCE_DIR)/%.c
	@mkdir -p $(@D)
	@echo "Building file $<"
	@$(CC) $(DEPEDENCY_FLAGS) $(BCDS_CFLAGS_RELEASE_COMMON) $(BCDS_XDK_INCLUDES) $(BCDS_XDK_EXT_INCLUDES) -DBCDS_PACKAGE_ID=$(BCDS_PACKAGE_ID) -c $< -o $@

$(BCDS_XDK_APP_RELEASE_OBJECT_DIR)/%.o: %.S
	@mkdir -p $(@D)
	@echo "Assembling $<"
	@$(CC) $(ASMFLAGS) $(BCDS_XDK_INCLUDES) $(BCDS_XDK_EXT_INCLUDES) -c $< -o $@

$(BCDS_XDK_APP_RELEASE_DIR)/$(BCDS_APP_NAME).out: $(BCDS_LIBS_RELEASE) $(BCDS_XDK_APP_OBJECT_FILES_RELEASE) 
	@echo "Creating .out $@"
	@$(CC) $(LDFLAGS_RELEASE) $(BCDS_XDK_APP_OBJECT_FILES_RELEASE) $(BCDS_RELEASE_LIBS_GROUP) $(LIBS) -o $@

$(BCDS_XDK_APP_RELEASE_DIR)/$(BCDS_APP_NAME).bin: $(BCDS_XDK_APP_RELEASE_DIR)/$(BCDS_APP_NAME).out
	@echo "Creating binary $@"
	@$(OBJCOPY) -R .usrpg -O binary $(BCDS_XDK_APP_RELEASE_DIR)/$(BCDS_APP_NAME).out $@
	

#if the header file is changed, compiler considers the change in a header file and compiles. Here no need to clean the application
#to take changes of header file during compilation
-include $(BCDS_XDK_APP_DEPENDENCY_DEBUG_FILES)
-include $(BCDS_XDK_APP_DEPENDENCY_RELEASE_FILES)

#Flash the .bin file to the target
flash_debug_bin: $(BCDS_XDK_APP_DEBUG_DIR)/$(BCDS_APP_NAME).bin
	@echo "Speed 12000" > CommandFile.jlink                                     									  #speed
	@echo "r" >> CommandFile.jlink                                              									  #reset
	@echo "h" >> CommandFile.jlink                                             										  #halt
	@echo "loadbin "$(BCDS_XDK_APP_DEBUG_DIR)/$(BCDS_APP_NAME).bin" "$(XDK_APP_ADDRESS) >> CommandFile.jlink          #load bin
	@echo "verifybin "$(BCDS_XDK_APP_DEBUG_DIR)/$(BCDS_APP_NAME).bin" "$(XDK_APP_ADDRESS) >> CommandFile.jlink        #verify bin
	@echo "r" >> CommandFile.jlink                                             									      #reset
	@echo "qc" >> CommandFile.jlink                                
	@$(FLASH_TOOL_PATH) -device $(BCDS_EFM32_DEVICE_ID) -if SWD -CommanderScript CommandFile.jlink
	@echo "Flashing is completed successfully"

flash_release_bin: $(BCDS_XDK_APP_RELEASE_DIR)/$(BCDS_APP_NAME).bin
	@echo "Speed 12000" > CommandFile.jlink                                     									    #speed
	@echo "r" >> CommandFile.jlink                                              									    #reset
	@echo "h" >> CommandFile.jlink                                              										#halt
	@echo "loadbin "$(BCDS_XDK_APP_RELEASE_DIR)/$(BCDS_APP_NAME).bin" "$(XDK_APP_ADDRESS) >> CommandFile.jlink          #load bin
	@echo "verifybin "$(BCDS_XDK_APP_RELEASE_DIR)/$(BCDS_APP_NAME).bin" "$(XDK_APP_ADDRESS) >> CommandFile.jlink        #verify bin
	@echo "r" >> CommandFile.jlink                                 										    		    #reset
	@echo "qc" >> CommandFile.jlink                        
	@$(FLASH_TOOL_PATH) -device $(BCDS_EFM32_DEVICE_ID) -if SWD -CommanderScript CommandFile.jlink
	@echo "Flashing is completed successfully"
	
cdt:
	@echo "cdt"
	echo $(BCDS_CFLAGS_DEBUG_COMMON)
	$(CC) $(BCDS_CFLAGS_DEBUG_COMMON) $(patsubst %,-I%, $(abspath $(patsubst -I%,%, $(BCDS_XDK_INCLUDES) $(BCDS_XDK_EXT_INCLUDES))))  -E -P -v -dD -c ${CDT_INPUT_FILE}
		
.PHONY: cdt

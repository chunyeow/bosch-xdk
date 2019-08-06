################### Common Makefile for EFM32 ##################################

# The following variables are expected to be passed as input variables 
# from outside.
# BCDS_PACKAGE_NAME
# BCDS_PACKAGE_ID
# BCDS_PACKAGE_HOME
# BCDS_INCLUDES
# BCDS_SOURCE_FILES
# BCDS_PACKAGE_VERSION_MAJOR
# BCDS_PACKAGE_VERSION_MINOR
# BCDS_PACKAGE_VERSION_PATCH

# At the time the below variables are defined (read-in phase),
# source files are not yet necessarly declared. Those could get
# defined later (by the target platform make file for instance). 
# We therefore have to enable the .SECONDEXPANSION: feature of
# make. This will tell make to expand variables escaped with an
# additional "$" during the target-run phase.
.SECONDEXPANSION:

BCDS_TARGET_PLATFORM = efm32
BCDS_ARCH_CPU = ARM_CM3
BCDS_EFM32_DEVICE_TYPE ?= EFM32GG
BCDS_EFM32_DEVICE_ID ?= EFM32GG390F1024
BCDS_DOXYGEN_VERSION = v1.8.8_32Bit
BCDS_GCC_VERSION = v4.7_2014q2
BCDS_JLINK_VERSION = v5.10l

# Configuration to enable servalpal features
export BCDS_SERVALPAL_WIFI=1
export BCDS_SERVALPAL_ARCHIVE_SERVAL_STACK = 0
export BCDS_SERVALSTACK_EXTENSION=1
export BCDS_ADC_ENABLE=1
export BCDS_EMLIB_INCLUDE_USB=1

BCDS_FREERTOS_INCLUDE_AWS?=0

# The absolute path to the platform folder. 
BCDS_PLATFORM_PATH ?= $(BCDS_PACKAGE_HOME)/../../Platform
export BCDS_SHARED_PATH := $(BCDS_PLATFORM_PATH)

# The absolute path to the libraries folder. 
BCDS_LIBRARIES_PATH ?= $(BCDS_PACKAGE_HOME)/../../Libraries
export THIRD_PARTY_SHARED_PATH := $(BCDS_LIBRARIES_PATH)

# The absolute path to the tools folder. 
BCDS_TOOLS_PATH ?= $(BCDS_DEVELOPMENT_TOOLS)

#Root paths to 3rd party library packages on which xdk applications depends
BCDS_FREERTOS_PATH := $(BCDS_LIBRARIES_PATH)/FreeRTOS
BCDS_EMLIB_PATH := $(BCDS_LIBRARIES_PATH)/EMlib
BCDS_BSTLIB_PATH := $(BCDS_LIBRARIES_PATH)/BSTLib
BCDS_WIFILIB_PATH := $(BCDS_LIBRARIES_PATH)/WiFi
BCDS_FATFSLIB_PATH := $(BCDS_LIBRARIES_PATH)/FATfs
BCDS_BSX_LIB_PATH := $(BCDS_LIBRARIES_PATH)/BSX
BCDS_BLE_LIB_PATH := $(BCDS_LIBRARIES_PATH)/BLEStack
BCDS_BLE_SERVICE_PATH := $(BCDS_BLE_LIB_PATH)/3rd-party/Alpwise/ALPW-BLESDKCM3
BCDS_BLE_CORE_PATH := $(BCDS_BLE_SERVICE_PATH)/BLESW_CoreStack
BCDS_BLE_INTERFACE_PATH := $(BCDS_BLE_CORE_PATH)/Interfaces
BCDS_SERVALSTACK_LIB_PATH =$(BCDS_LIBRARIES_PATH)/ServalStack
BCDS_FATFS_LIB_PATH := $(THIRD_PARTY_SHARED_PATH)/FATfs
BCDS_BLE_STACK_PATH := $(THIRD_PARTY_SHARED_PATH)/BLEStack
BCDS_SIMPLELINK_PATH := $(THIRD_PARTY_SHARED_PATH)/WiFi/3rd-party/TI/simplelink
BCDS_GRIDEYE_LIB_PATH :=$(THIRD_PARTY_SHARED_PATH)/GridEye
BCDS_MBED_LIB_PATH :=$(THIRD_PARTY_SHARED_PATH)/MbedTLS

#Paths to shared packages on which this xdk applications depends
BCDS_ESSENTIALS_PATH := $(BCDS_PLATFORM_PATH)/Essentials
BCDS_UTILS_PATH := $(BCDS_PLATFORM_PATH)/Utils
BCDS_DRIVERS_PATH := $(BCDS_PLATFORM_PATH)/Drivers
BCDS_LORA_DRIVERS_PATH := $(BCDS_PLATFORM_PATH)/LoRaDrivers
BCDS_BLE_PATH := $(BCDS_PLATFORM_PATH)/BLE
BCDS_SENSORS_PATH := $(BCDS_PLATFORM_PATH)/Sensors
BCDS_SENSORS_UTILS_PATH := $(BCDS_PLATFORM_PATH)/SensorUtils
BCDS_BSP_PATH := $(BCDS_PLATFORM_PATH)/BSP
BCDS_SERVALPAL_PATH := $(BCDS_PLATFORM_PATH)/ServalPAL
BCDS_WLAN_PATH := $(BCDS_PLATFORM_PATH)/Wlan
BCDS_SENSOR_TOOLBOX_PATH := $(BCDS_PLATFORM_PATH)/SensorToolbox
BCDS_FOTA_PATH := $(BCDS_PLATFORM_PATH)/FOTA
BCDS_ALGO_PATH := $(BCDS_PLATFORM_PATH)/Algo
BCDS_LORADRIVERS_PATH := $(BCDS_PLATFORM_PATH)/LoRaDrivers

#Paths to Tools on which this xdk applications depends
BCDS_DOXYGEN_PATH := $(BCDS_TOOLS_PATH)/Doxygen/$(BCDS_DOXYGEN_VERSION)
BCDS_GCC_PATH := $(BCDS_TOOLS_PATH)/gcc-arm-none-eabi/$(BCDS_GCC_VERSION)/bin
BCDS_PYTHON_PATH =  $(PYTHON_34)/python

BCDS_XDK110_COMMON_PATH = $(BCDS_PACKAGE_HOME)/../../Common
# The absolute path to the common make file
BCDS_COMMON_MAKEFILE ?= $(BCDS_XDK110_COMMON_PATH)/common.mk

# The absolute path to the debug build destination folder. 
BCDS_DEBUG_PATH = $(BCDS_PACKAGE_HOME)/debug

# The absolute path to the release build destination folder. 
BCDS_RELEASE_PATH = $(BCDS_PACKAGE_HOME)/release

DOXYGEN ?= $(BCDS_DOXYGEN_PATH)/doxygen.exe
HHC ?= $(BCDS_DOXYGEN_VERSION)/hhc.exe
GRAPHVIZ ?= $(BCDS_TOOLS_PATH)/Graphviz/V2.38-1/bin

BCDS_DEBUG_LIB = $(BCDS_DEBUG_PATH)/lib$(BCDS_PACKAGE_NAME)_efm32_debug.a
BCDS_RELEASE_LIB = $(BCDS_RELEASE_PATH)/lib$(BCDS_PACKAGE_NAME)_efm32.a

# The absolute path to the configuration folder. 
BCDS_CONFIG_PATH = $(BCDS_XDK110_COMMON_PATH)/config
BCDS_ADC_DRIVER_PATH = $(BCDS_CONFIG_PATH)

# The absolute path to the BSP implementation folder. 
BCDS_BOARD_PATH = $(BCDS_PLATFORM_PATH)/BSP/

# The absolute path to the common gtest makefile
#BCDS_COMMON_GTEST_MAKEFILE = $(BCDS_XDK110_COMMON_PATH)/common_gtest.mk

# Compiler settings
# The variables BCDS_CFLAGS_DEBUG and BCDS_CFLAGS_RELEASE are usually defined 
# by the root makefile (e.g. project makefile) in order to pass the desired 
# compiler flags. If they are not passed, then the following default flags will
# be taken.
BCDS_CFLAGS_DEBUG ?= \
    -mcpu=cortex-m3 -mthumb \
    -std=c99 -Wall -Wextra -Wmissing-prototypes -Wimplicit-function-declaration -Wstrict-prototypes \
    -ffunction-sections -fdata-sections \
    -Wa,-ahlms=$(@:.o=.lst) \
    -O0 -g

BCDS_CFLAGS_RELEASE ?= \
    -mcpu=cortex-m3 -mthumb \
    -std=c99 -Wall -Wextra -Wmissing-prototypes -Wimplicit-function-declaration -Wstrict-prototypes \
    -ffunction-sections -fdata-sections \
    -Wa,-ahlms=$(@:.o=.lst) \
    -O0 -DNDEBUG

#This flag is used to generate dependency files 
DEPEDENCY_FLAGS = -MMD -MP -MF $(@:.o=.d)
LST_FLAGS = -Wa,-ahlms=$(@:.o=.lst)

BCDS_MACROS_DEBUG = \
	-D BCDS_PACKAGE_ID=$(BCDS_PACKAGE_ID) \
	-D BCDS_PACKAGE_NAME=$(BCDS_PACKAGE_NAME) \
	-D BCDS_PACKAGE_VERSION_MAJOR=$(BCDS_PACKAGE_VERSION_MAJOR) \
	-D BCDS_PACKAGE_VERSION_MINOR=$(BCDS_PACKAGE_VERSION_MINOR) \
	-D BCDS_PACKAGE_VERSION_PATCH=$(BCDS_PACKAGE_VERSION_PATCH) \
	-D BCDS_PACKAGE_REVISION_ID=$(BCDS_PACKAGE_REVISION_ID) \
	-D $(BCDS_TARGET_PLATFORM) -D BCDS_TARGET_PLATFORM=$(BCDS_TARGET_PLATFORM) \
	-D $(BCDS_EFM32_DEVICE_TYPE) -D $(BCDS_EFM32_DEVICE_ID) \
	-D BCDS_SERVALPAL_WIFI=$(BCDS_SERVALPAL_WIFI) -D BCDS_EMLIB_INCLUDE_USB=$(BCDS_EMLIB_INCLUDE_USB) \
	-D BCDS_FREERTOS_INCLUDE_AWS=$(BCDS_FREERTOS_INCLUDE_AWS) \
	 $(XDK_CFLAGS_COMMON) \

# we extend the macros here as per test needs
BCDS_MACROS_DEBUG += $(BCDS_TEST_MACROS)

# Note that BCDS_MACROS_RELEASE is set to be equal BCDS_MACROS_DEBUG
BCDS_MACROS_RELEASE = $(BCDS_MACROS_DEBUG)

CFLAGS_DEBUG = $(BCDS_CFLAGS_DEBUG) $(DEPEDENCY_FLAGS) $(LST_FLAGS)
CFLAGS_DEBUG += $(BCDS_MACROS_DEBUG)

CFLAGS_DEBUG_WITHOUT_ASSERT = $(BCDS_CFLAGS_DEBUG_WITHOUT_ASSERT) $(DEPEDENCY_FLAGS) $(LST_FLAGS)
CFLAGS_DEBUG_WITHOUT_ASSERT += $(BCDS_MACROS_DEBUG)

CFLAGS_RELEASE = $(BCDS_CFLAGS_RELEASE) $(DEPEDENCY_FLAGS) $(LST_FLAGS)
CFLAGS_RELEASE += $(BCDS_MACROS_RELEASE)

BCDS_ARFLAGS = -cr

# Build chain settings
ifneq ("$(wildcard $(BCDS_TOOLS_PATH))","")
CC = $(BCDS_GCC_PATH)/arm-none-eabi-gcc
AR = $(BCDS_GCC_PATH)/arm-none-eabi-ar
SIZE = $(BCDS_GCC_PATH)/arm-none-eabi-size
else
CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar
SIZE = arm-none-eabi-size
endif

RMDIRS = rm -rf
ARFLAGS = -cr

# Note that BCDS_SOURCE_FILES must be defined outside.
BCDS_OBJECT_FILES = \
	$(patsubst %.c, %.o, $(BCDS_SOURCE_FILES))

# Path of the generated static library and its object files
BCDS_DEBUG_OBJECT_PATH = $(BCDS_DEBUG_PATH)/object
BCDS_RELEASE_OBJECT_PATH = $(BCDS_RELEASE_PATH)/object

# including this file, will force the execution of the sourceFile.list
# rule.
include sourceFile.list

BCDS_DEBUG_OBJECT_FILES = \
	$(addprefix $(BCDS_DEBUG_OBJECT_PATH)/,$(BCDS_OBJECT_FILES))

BCDS_DEBUG_DEPENDENCY_FILES = \
	$(addprefix $(BCDS_DEBUG_OBJECT_PATH)/, $(BCDS_OBJECT_FILES:.o=.d))

BCDS_RELEASE_OBJECT_FILES = \
	$(addprefix $(BCDS_RELEASE_OBJECT_PATH)/,$(BCDS_OBJECT_FILES))

BCDS_RELEASE_DEPENDENCY_FILES = \
	$(addprefix $(BCDS_RELEASE_OBJECT_PATH)/, $(BCDS_OBJECT_FILES:.o=.d))

# target to force all variables to be populated before executing any rules
# and before including the dep files
.PHONY: sourceFile.list
sourceFile.list:
	@echo 'BCDS_SOURCE_FILES := $(BCDS_SOURCE_FILES)' > '$@'

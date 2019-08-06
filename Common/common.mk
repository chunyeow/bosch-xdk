################### Common Makefile for EFM32 ##################################

# The following variables are expected to be passed as input variables 
# from outside.
# BCDS_PACKAGE_NAME
# BCDS_PACKAGE_ID
# BCDS_PACKAGE_HOME
# BCDS_INCLUDES
# BCDS_SOURCE_FILES

include $(dir $(BCDS_COMMON_MAKEFILE))common_settings.mk

######################## Build Targets #######################################
# Generates Doxygen files and compresses it
doxygen:
	@echo Building doxygen
	cd ./docs && $(DOXYGEN) > doxygenDump.txt
	cd ./docs && $(HHC) html/index.hhp
	@echo Documentation ready

# Deletes the generated doxygen files
doxygen_clean:
	@echo Cleaning doxygen
	rm -rf ./docs/doxygenWarning.log 
	rm -rf ./docs/doxygenDump.txt 
	rm -rf ./docs/html
	rm -rf ./docs/*.tmp
	@echo Documentation cleaned

BCDS_XDK_INCLUDE = \
	-I $(THIRD_PARTY_SHARED_PATH)/FreeRTOS/3rd-party/include \
	-I $(THIRD_PARTY_SHARED_PATH)/FreeRTOS/3rd-party/include/private \
	-I $(THIRD_PARTY_SHARED_PATH)/FreeRTOS/3rd-party/FreeRTOS/portable/GCC/ARM_CM3 \
	-I $(BCDS_CONFIG_PATH)/AmazonFreeRTOS/FreeRTOS \
	-I $(BCDS_CONFIG_PATH)/AmazonFreeRTOS \
	-I $(BCDS_XDK110_COMMON_PATH)/legacy/include \
	-I $(BCDS_WIFILIB_PATH)/include

######################## Build Targets #######################################
.PHONY: check
check::
ifneq ($(BCDS_TARGET_PLATFORM),efm32)
	$(error Mismatch of target platformm, BCDS_TARGET_PLATFORM is $(BCDS_TARGET_PLATFORM) instead of efm32)
endif

ifndef BCDS_PACKAGE_NAME
	$(error BCDS_PACKAGE_NAME not defined)
endif

ifndef BCDS_PACKAGE_ID
	$(error BCDS_PACKAGE_ID not defined)
endif

ifndef BCDS_PACKAGE_HOME
	$(error BCDS_PACKAGE_HOME not defined)
endif

ifeq ($$(BCDS_INCLUDES),)
	$(error BCDS_INCLUDES not defined)
endif

ifeq ($$(BCDS_SOURCE_FILES),)
	$(error BCDS_SOURCE_FILES not defined)
endif

.PHONY: diagnosis
diagnosis::
	@echo "BCDS_PACKAGE_NAME:      "$(BCDS_PACKAGE_NAME)
	@echo "BCDS_PACKAGE_HOME:      "$(BCDS_PACKAGE_HOME)
	@echo "BCDS_PACKAGE_ID:        "$(BCDS_PACKAGE_ID)
	@echo "BCDS_TARGET_PLATFORM:   "$(BCDS_TARGET_PLATFORM)
	@echo "BCDS_ARCH_CPU:          "$(BCDS_ARCH_CPU)
	@echo "BCDS_PLATFORM_PATH:     "$(BCDS_PLATFORM_PATH)
	@echo "BCDS_LIBRARIES_PATH:    "$(BCDS_LIBRARIES_PATH)
	@echo "BCDS_TOOLS_PATH:        "$(BCDS_TOOLS_PATH)
	@echo "BCDS_CONFIG_PATH:       "$(BCDS_CONFIG_PATH)
	@echo "BCDS_DEBUG_PATH:        "$(BCDS_DEBUG_PATH)
	@echo "BCDS_DEBUG_OBJECT_PATH: "$(BCDS_DEBUG_OBJECT_PATH)
	@echo "BCDS_RELEASE_PATH:      "$(BCDS_RELEASE_PATH)
	@echo "BCDS_DEBUG_LIB:         "$(BCDS_DEBUG_LIB)
	@echo "BCDS_RELEASE_LIB:       "$(BCDS_RELEASE_LIB)
	@echo "BCDS_INCLUDES:          "
	@echo $(BCDS_INCLUDES)
	@echo ""
	@echo "BCDS_SOURCE_FILES:      "
	@echo $(BCDS_SOURCE_FILES)
	@echo ""
	@echo "BCDS_DEBUG_OBJECT_FILES: "
	@echo $(BCDS_DEBUG_OBJECT_FILES)
	@echo ""
	@echo "BCDS_CFLAGS_DEBUG:      "$(BCDS_CFLAGS_DEBUG)
	@echo "CFLAGS_DEBUG:           "$(CFLAGS_DEBUG)
	@echo "CFLAGS_DEBUG_WITHOUT_ASSERT"	$(CFLAGS_DEBUG_WITHOUT_ASSERT)
	@echo "BCDS_CFLAGS_RELEASE:    "$(BCDS_CFLAGS_RELEASE)
	@echo "CFLAGS_RELEASE:         "$(CFLAGS_RELEASE)

.PHONY: clean
clean:
	rm -rf $(BCDS_DEBUG_PATH) $(BCDS_RELEASE_PATH)

.PHONY: debug
debug: check $(BCDS_DEBUG_LIB)

.PHONY: release	
release: check $(BCDS_RELEASE_LIB)

$(BCDS_DEBUG_LIB): $$(BCDS_DEBUG_OBJECT_FILES)
	@mkdir -p $(@D)
	@$(AR) $(ARFLAGS) $@ $(BCDS_DEBUG_OBJECT_FILES)
	@echo "========================================"
	@echo "Library created: $@"
	@echo "========================================"
	
$(BCDS_RELEASE_LIB): $$(BCDS_RELEASE_OBJECT_FILES)
	@mkdir -p $(@D)
	@$(AR) $(ARFLAGS) $@ $(BCDS_RELEASE_OBJECT_FILES)
	@echo "========================================"
	@echo "Library created: $@"
	@echo "========================================"
	
# compile C files

ifeq ($(BCDS_PACKAGE_NAME),ServalStack)
$(BCDS_DEBUG_OBJECT_PATH)/%.o: %.c
	@mkdir -p $(@D)
	@echo "Build $<"
	@$(CC) -c $(CFLAGS_DEBUG_WITHOUT_ASSERT) -I . $(BCDS_INCLUDES) $(BCDS_XDK_INCLUDE) $< -o $@
else
$(BCDS_DEBUG_OBJECT_PATH)/%.o: %.c
	@mkdir -p $(@D)
	@echo "Build $<"
	@$(CC) -c $(CFLAGS_DEBUG) -I . $(BCDS_INCLUDES) $(BCDS_XDK_INCLUDE) $< -o $@
endif

$(BCDS_RELEASE_OBJECT_PATH)/%.o: %.c
	@mkdir -p $(@D)
	@echo "Build $<"
	@$(CC) -c $(CFLAGS_RELEASE) -I . $(BCDS_INCLUDES) $(BCDS_XDK_INCLUDE) $< -o $@


.PHONY: cdt
cdt:
	$(CC) $(CFLAGS_DEBUG) -O0 $(BCDS_INCLUDES) -E -P -v -dD -c ${CDT_INPUT_FILE}
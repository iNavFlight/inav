OFFICIAL_TARGETS  = ALIENFLIGHTF3 ALIENFLIGHTF4 ANYFCF7 BETAFLIGHTF3 BLUEJAYF4 CC3D FURYF4 NAZE REVO SIRINFPV SPARKY SPRACINGF3 SPRACINGF3EVO SPRACINGF3NEO SPRACINGF4EVO STM32F3DISCOVERY
SKIP_TARGETS     := ALIENWHOOP MOTOLABF4
ALT_TARGETS       = $(sort $(filter-out target, $(basename $(notdir $(wildcard $(ROOT)/src/main/target/*/*.mk)))))
OPBL_TARGETS      = $(filter %_OPBL, $(ALT_TARGETS))
OSD_SLAVE_TARGETS = SPRACINGF3OSD

VALID_TARGETS   = $(dir $(wildcard $(ROOT)/src/main/target/*/target.mk))
VALID_TARGETS  := $(subst /,, $(subst ./src/main/target/,, $(VALID_TARGETS)))
VALID_TARGETS  := $(VALID_TARGETS) $(ALT_TARGETS)
VALID_TARGETS  := $(sort $(VALID_TARGETS))
VALID_TARGETS  := $(filter-out $(SKIP_TARGETS), $(VALID_TARGETS))

ifeq ($(filter $(TARGET),$(SKIP_TARGETS)), $(TARGET))
ALTERNATES    := $(sort $(filter-out target, $(basename $(notdir $(wildcard $(ROOT)/src/main/target/$(TARGET)/*.mk)))))
$(error The target specified, $(TARGET), cannot be built. Use one of the ALT targets: $(ALTERNATES))
endif

GROUP_1_TARGETS := \
	AIRBOTF4 \
	AIRHEROF3 \
	ALIENFLIGHTF3 \
	ALIENFLIGHTF4 \
	ALIENFLIGHTNGF7 \
	ANYFC \
	ANYFCF7 \
	ANYFCM7 \
	BEEROTORF4 \
	BLUEJAYF4 \
	CC3D

GROUP_2_TARGETS := \
	CHEBUZZF3 \
	CJMCU \
	COLIBRI \
	COLIBRI_RACE \
	CRAZEPONYMINI \
	EUSTM32F103RC \
	F4BY \
	FALCORE \
	FISHDRONEF4 \
	FURYF3 \
	KFC32F3_INAV

GROUP_3_TARGETS := \
	KROOZX \
	LUX_RACE \
	MATEKF405 \
	MOTOLAB \
	NAZE \
	OLIMEXINO \
	OMNIBUS \
	OMNIBUSF4 \
	OMNIBUSF7 \
	PIKOBLX_limited \
	PIXRACER \
	PORT103R

GROUP_4_TARGETS := \
	RCEXPLORERF3 \
	REVO \
	RMDO \
	SPARKY \
	SPARKY2 \
	SPRACINGF3 \
	SPRACINGF3EVO \
	SPRACINGF3MINI \
	SPRACINGF3NEO \
	SPRACINGF4EVO \
	STM32F3DISCOVERY \
	YUPIF4 \


GROUP_OTHER_TARGETS := $(filter-out $(GROUP_1_TARGETS) $(GROUP_2_TARGETS) $(GROUP_3_TARGETS) $(GROUP_4_TARGETS), $(VALID_TARGETS))

ifeq ($(filter $(TARGET),$(ALT_TARGETS)), $(TARGET))
BASE_TARGET    := $(firstword $(subst /,, $(subst ./src/main/target/,, $(dir $(wildcard $(ROOT)/src/main/target/*/$(TARGET).mk)))))
include $(ROOT)/src/main/target/$(BASE_TARGET)/$(TARGET).mk
else
BASE_TARGET    := $(TARGET)
endif

ifeq ($(filter $(TARGET),$(OPBL_TARGETS)), $(TARGET))
OPBL            = yes
endif

ifeq ($(filter $(TARGET),$(OSD_SLAVE_TARGETS)), $(TARGET))
# build an OSD SLAVE
OSD_SLAVE       = yes
else
# build an FC
FC              = yes
endif

# silently ignore if the file is not present. Allows for target specific.
-include $(ROOT)/src/main/target/$(BASE_TARGET)/target.mk

F4_TARGETS      := $(F405_TARGETS) $(F411_TARGETS) $(F446_TARGETS)
F7_TARGETS      := $(F7X2RE_TARGETS) $(F7X5XE_TARGETS) $(F7X5XG_TARGETS) $(F7X5XI_TARGETS) $(F7X6XG_TARGETS)

ifeq ($(filter $(TARGET),$(VALID_TARGETS)),)
$(error Target '$(TARGET)' is not valid, must be one of $(VALID_TARGETS). Have you prepared a valid target.mk?)
endif

ifeq ($(filter $(TARGET),$(F1_TARGETS) $(F3_TARGETS) $(F4_TARGETS) $(F7_TARGETS) $(SITL_TARGETS)),)
$(error Target '$(TARGET)' has not specified a valid STM group, must be one of F1, F3, F405, F411 or F7x5. Have you prepared a valid target.mk?)
endif

ifeq ($(TARGET),$(filter $(TARGET),$(F3_TARGETS)))
TARGET_MCU := STM32F3

else ifeq ($(TARGET),$(filter $(TARGET), $(F4_TARGETS)))
TARGET_MCU := STM32F4

else ifeq ($(TARGET),$(filter $(TARGET), $(F7_TARGETS)))
TARGET_MCU := STM32F7

else ifeq ($(TARGET),$(filter $(TARGET), $(SITL_TARGETS)))
TARGET_MCU := SITL

else ifeq ($(TARGET),$(filter $(TARGET), $(F1_TARGETS)))
TARGET_MCU := STM32F1
else
$(error Unknown target MCU specified.)
endif

ifneq ($(BASE_TARGET), $(TARGET))
TARGET_FLAGS  	:= $(TARGET_FLAGS) -D$(BASE_TARGET)
endif

TARGET_FLAGS  	:= $(TARGET_FLAGS) -D$(TARGET_MCU)

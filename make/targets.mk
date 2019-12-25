ALT_TARGETS     = $(sort $(filter-out target, $(basename $(notdir $(wildcard $(ROOT)/src/main/target/*/*.mk)))))

VALID_TARGETS   = $(dir $(wildcard $(ROOT)/src/main/target/*/target.mk))
VALID_TARGETS  := $(subst /,, $(subst ./src/main/target/,, $(VALID_TARGETS)))
VALID_TARGETS  := $(VALID_TARGETS) $(ALT_TARGETS)
VALID_TARGETS  := $(sort $(VALID_TARGETS))

CLEAN_TARGETS   = $(addprefix clean_,$(VALID_TARGETS) )
TARGETS_CLEAN   = $(addsuffix _clean,$(VALID_TARGETS) )
STFLASH_TARGETS = $(addprefix st-flash_,$(VALID_TARGETS) )

ifeq ($(filter $(TARGET),$(ALT_TARGETS)), $(TARGET))
BASE_TARGET    := $(firstword $(subst /,, $(subst ./src/main/target/,, $(dir $(wildcard $(ROOT)/src/main/target/*/$(TARGET).mk)))))
-include $(ROOT)/src/main/target/$(BASE_TARGET)/$(TARGET).mk
else
BASE_TARGET    := $(TARGET)
endif

# silently ignore if the file is not present. Allows for target specific.
-include $(ROOT)/src/main/target/$(BASE_TARGET)/target.mk

F4_TARGETS      = $(F405_TARGETS) $(F411_TARGETS) $(F427_TARGETS) $(F446_TARGETS)
F7_TARGETS      = $(F7X2RE_TARGETS) $(F7X5XE_TARGETS) $(F7X5XG_TARGETS) $(F7X5XI_TARGETS) $(F7X6XG_TARGETS)

ifeq ($(filter $(TARGET),$(VALID_TARGETS)),)
$(error Target '$(TARGET)' is not valid, must be one of $(VALID_TARGETS). Have you prepared a valid target.mk?)
endif

ifeq ($(filter $(TARGET),$(F3_TARGETS) $(F4_TARGETS) $(F7_TARGETS)),)
$(error Target '$(TARGET)' has not specified a valid STM group, must be one of F3, F405, F411, F427 or F7x. Have you prepared a valid target.mk?)
endif

ifeq ($(TARGET),$(filter $(TARGET),$(F3_TARGETS)))
TARGET_MCU 			:= STM32F303
TARGET_MCU_GROUP 	:= STM32F3
else ifeq ($(TARGET),$(filter $(TARGET), $(F405_TARGETS)))
TARGET_MCU			:= STM32F405
TARGET_MCU_GROUP 	:= STM32F4
else ifeq ($(TARGET),$(filter $(TARGET), $(F411_TARGETS)))
TARGET_MCU			:= STM32F411
TARGET_MCU_GROUP 	:= STM32F4
else ifeq ($(TARGET),$(filter $(TARGET), $(F427_TARGETS)))
TARGET_MCU			:= STM32F427
TARGET_MCU_GROUP 	:= STM32F4
else ifeq ($(TARGET),$(filter $(TARGET), $(F446_TARGETS)))
TARGET_MCU			:= STM32F446
TARGET_MCU_GROUP 	:= STM32F4
else ifeq ($(TARGET),$(filter $(TARGET), $(F7X2RE_TARGETS)))
TARGET_MCU			:= STM32F7X2RE
TARGET_MCU_GROUP 	:= STM32F7
else ifeq ($(TARGET),$(filter $(TARGET), $(F7X5XE_TARGETS)))
TARGET_MCU			:= STM32F7X5XE
TARGET_MCU_GROUP 	:= STM32F7
else ifeq ($(TARGET),$(filter $(TARGET), $(F7X5XG_TARGETS)))
TARGET_MCU			:= STM32F7X5XG
TARGET_MCU_GROUP 	:= STM32F7
else ifeq ($(TARGET),$(filter $(TARGET), $(F7X5XI_TARGETS)))
TARGET_MCU			:= STM32F7X5XI
TARGET_MCU_GROUP 	:= STM32F7
else ifeq ($(TARGET),$(filter $(TARGET), $(F7X6XG_TARGETS)))
TARGET_MCU			:= STM32F7X6XG
TARGET_MCU_GROUP 	:= STM32F7
else
$(error Unknown target MCU specified.)
endif

GROUP_1_TARGETS := AIRHEROF3 AIRHEROF3_QUAD LUX_RACE SPARKY REVO SPARKY2 COLIBRI FALCORE FF_F35_LIGHTNING FF_FORTINIF4 FF_PIKOF4 FF_PIKOF4OSD
GROUP_2_TARGETS := SPRACINGF3 SPRACINGF3EVO SPRACINGF3EVO_1SS SPRACINGF3MINI SPRACINGF4EVO CLRACINGF4AIR CLRACINGF4AIRV2 BEEROTORF4 BETAFLIGHTF3 BETAFLIGHTF4 PIKOBLX
GROUP_3_TARGETS := OMNIBUS AIRBOTF4 BLUEJAYF4 OMNIBUSF4 OMNIBUSF4PRO FIREWORKSV2 SPARKY2 MATEKF405 OMNIBUSF7 DYSF4PRO OMNIBUSF4PRO_LEDSTRIPM5 OMNIBUSF7NXT OMNIBUSF7V2 ASGARD32F4
GROUP_4_TARGETS := ANYFC ANYFCF7 ANYFCF7_EXTERNAL_BARO ALIENFLIGHTNGF7 PIXRACER YUPIF7 MATEKF405SE MATEKF411 MATEKF722 MATEKF405OSD MATEKF405_SERVOS6 NOX
GROUP_5_TARGETS := ASGARD32F7 CLRACINGF4AIRV3 DALRCF405 DALRCF722DUAL DYSF4PROV2 F4BY FISHDRONEF4 FOXEERF405 FOXEERF722DUAL FRSKYF3 FRSKYF4 FURYF3 FURYF3_SPIFLASH FURYF4OSD
GROUP_6_TARGETS := MAMBAF405 OMNIBUSF4V3 OMNIBUSF4V3_S6_SS OMNIBUSF4V3_S5S6_SS OMNIBUSF4V3_S5_S6_2SS AIKONF4
GROUP_7_TARGETS := KAKUTEF4 KAKUTEF4V2 KAKUTEF7 KAKUTEF7MINI KFC32F3_INAV MATEKF411_RSSI MATEKF411_SFTSRL2 MATEKF722MINI MATEKF722SE MATEKF722_HEXSERVO
GROUP_8_TARGETS := MATEKF765 MATEKF722PX KAKUTEF7HDV
GROUP_OTHER_TARGETS := $(filter-out $(GROUP_1_TARGETS) $(GROUP_2_TARGETS) $(GROUP_3_TARGETS) $(GROUP_4_TARGETS) $(GROUP_5_TARGETS) $(GROUP_6_TARGETS) $(GROUP_7_TARGETS) $(GROUP_8_TARGETS), $(VALID_TARGETS))

## targets-group-1   : build some targets
targets-group-1: $(GROUP_1_TARGETS)

## targets-group-2   : build some targets
targets-group-2: $(GROUP_2_TARGETS)

## targets-group-3   : build some targets
targets-group-3: $(GROUP_3_TARGETS)

## targets-group-4   : build some targets
targets-group-4: $(GROUP_4_TARGETS)

## targets-group-5   : build some targets
targets-group-5: $(GROUP_5_TARGETS)

## targets-group-6   : build some targets
targets-group-6: $(GROUP_6_TARGETS)

## targets-group-7   : build some targets
targets-group-7: $(GROUP_7_TARGETS)

## targets-group-8   : build some targets
targets-group-8: $(GROUP_8_TARGETS)

## targets-group-rest: build the rest of the targets (not listed in group 1, 2 or 3)
targets-group-rest: $(GROUP_OTHER_TARGETS)

## targets           : print a list of all valid target platforms (for consumption by scripts)
targets:
	$(V0) @echo "Valid targets:      $(VALID_TARGETS)"
	$(V0) @echo "Target:             $(TARGET)"
	$(V0) @echo "Base target:        $(BASE_TARGET)"
	$(V0) @echo "targets-group-1:    $(GROUP_1_TARGETS)"
	$(V0) @echo "targets-group-2:    $(GROUP_2_TARGETS)"
	$(V0) @echo "targets-group-3:    $(GROUP_3_TARGETS)"
	$(V0) @echo "targets-group-4:    $(GROUP_4_TARGETS)"
	$(V0) @echo "targets-group-5:    $(GROUP_5_TARGETS)"
	$(V0) @echo "targets-group-6:    $(GROUP_6_TARGETS)"
	$(V0) @echo "targets-group-7:    $(GROUP_7_TARGETS)"
	$(V0) @echo "targets-group-8:    $(GROUP_8_TARGETS)"
	$(V0) @echo "targets-group-rest: $(GROUP_OTHER_TARGETS)"
	$(V0) @echo "Release targets:    $(RELEASE_TARGETS)"

.PHONY: .FORCE openocd-cfg $(OPENOCD_CFG) openocd-run openocd-flash

OPENOCD_CFG		?= $(TARGET_OBJ_DIR)/openocd.cfg
CLEAN_ARTIFACTS	+= $(OPENOCD_CFG)
OPENOCD_CMD 	?= openocd

STLINK			?= 2

ifeq ($(OPENOCD_INTERFACE),)
ifeq ($(STLINK),1)
OPENOCD_INTERFACE 	:= stlink-v1
else ifeq ($(STLINK),2)
OPENOCD_INTERFACE 	:= stlink-v2
else ifeq ($(STLINK),2.1)
OPENOCD_INTERFACE 	:= stlink-v2-1
else
$(error Uknown ST Link version $(STLINK))
endif
endif

ifeq ($(OPENOCD_TARGET),)
ifeq ($(TARGET_MCU_GROUP),STM32F3)
OPENOCD_TARGET := stm32f3x
else ifeq ($(TARGET_MCU_GROUP),STM32F4)
OPENOCD_TARGET := stm32f4x
else ifeq ($(TARGET_MCU_GROUP),STM32F7)
OPENOCD_TARGET := stm32f7x
endif
endif

ifeq ($(OPENOCD_TARGET),)
$(warning Unknown OPENOCD_TARGET)
endif

OPENOCD_CMDLINE := $(OPENOCD_CMD) -f $(OPENOCD_CFG)

openocd-cfg: $(OPENOCD_CFG)

$(OPENOCD_CFG): .FORCE
	$(V1) mkdir -p $(dir $@)
	$(V1) echo "source [find interface/$(OPENOCD_INTERFACE).cfg]" > $@
	$(V1) echo "source [find target/$(OPENOCD_TARGET).cfg]" >> $@

openocd-run: $(OPENOCD_CFG)
	$(OPENOCD_CMDLINE)

openocd-flash: $(TARGET_ELF) $(OPENOCD_CFG)
	(echo "halt; program $(realpath $<) verify reset" | nc -4 localhost 4444 2>/dev/null) || \
	$(OPENOCD_CMDLINE) -c "program $< verify reset exit"

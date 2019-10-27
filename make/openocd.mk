.PHONY: openocd

OPENOCD_CMD 		?= openocd
OPENOCD_INTERFACE 	?= stlink-v2

ifeq ($(TARGET_MCU),STM32F1)
OPENOCD_TARGET ?= stm32f1x
else ifeq ($(TARGET_MCU),STM32F3)
OPENOCD_TARGET ?= stm32f3x
else ifeq ($(TARGET_MCU),STM32F4)
OPENOCD_TARGET ?= stm32f4x
else ifeq ($(TARGET_MCU),STM32F7)
OPENOCD_TARGET ?= stm32f7x
endif

ifeq ($(OPENOCD_TARGET),)
$(warning Unknown OPENOCD_TARGET)
endif

OPENOCD_CMDLINE := $(OPENOCD_CMD) -f interface/$(OPENOCD_INTERFACE).cfg -f target/$(OPENOCD_TARGET).cfg

openocd-run:
	$(OPENOCD_CMDLINE)

openocd-flash: $(TARGET_ELF)
	(echo "halt; program $(realpath $<) verify reset" | nc -4 localhost 4444 2>/dev/null) || \
	$(OPENOCD_CMDLINE) -c "program $< verify reset exit"

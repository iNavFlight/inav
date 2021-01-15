include ${CHIBIOS}/os/hal/hal.mk

ifeq ($(USE_SMART_BUILD),yes)

# Configuration files directory
ifeq ($(CONFDIR),)
  CONFDIR = .
endif

HALCONF := $(strip $(shell cat $(CONFDIR)/halconf.h $(CONFDIR)/halconf_community.h | egrep -e "\#define"))

HALSRC_CONTRIB := ${CHIBIOS_CONTRIB}/os/hal/src/hal_community.c
ifneq ($(findstring HAL_USE_NAND TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_nand.c
endif
ifneq ($(findstring HAL_USE_ONEWIRE TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_onewire.c
endif
ifneq ($(findstring HAL_USE_EICU TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_eicu.c
endif
ifneq ($(findstring HAL_USE_CRC TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_crc.c
endif
ifneq ($(findstring HAL_USE_RNG TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_rng.c
endif
ifneq ($(findstring HAL_USE_USBH TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_usbh.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/usbh/hal_usbh_debug.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/usbh/hal_usbh_desciter.c
endif
ifneq ($(findstring HAL_USBH_USE_HUB TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/usbh/hal_usbh_hub.c
endif
ifneq ($(findstring HAL_USBH_USE_MSD TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/usbh/hal_usbh_msd.c
endif
ifneq ($(findstring HAL_USBH_USE_FTDI TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_usbh_ftdi.c 
endif
ifneq ($(findstring HAL_USBH_USE_AOA TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_usbh_aoa.c 
endif
ifneq ($(findstring HAL_USBH_USE_HID TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_usbh_hid.c 
endif
ifneq ($(findstring HAL_USBH_USE_UVC TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/usbh/hal_usbh_uvc.c
endif
ifneq ($(findstring HAL_USE_EEPROM TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_eeprom.c
ifneq ($(findstring EEPROM_USE_EE25XX TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_ee25xx.c
endif
ifneq ($(findstring EEPROM_USE_EE24XX TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_ee24xx.c
endif
endif
ifneq ($(findstring HAL_USE_TIMCAP TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_timcap.c
endif
ifneq ($(findstring HAL_USE_QEI TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_qei.c
endif
ifneq ($(findstring HAL_USE_USB_HID TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_usb_hid.c
endif
ifneq ($(findstring HAL_USE_USB_MSD TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_usb_msd.c
endif
ifneq ($(findstring HAL_USE_COMP TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_comp.c
endif
ifneq ($(findstring HAL_USE_OPAMP TRUE,$(HALCONF)),)
HALSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/src/hal_opamp.c
endif
else
HALSRC_CONTRIB := ${CHIBIOS_CONTRIB}/os/hal/src/hal_community.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/hal_nand.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/hal_onewire.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/hal_eicu.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/hal_crc.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/hal_rng.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/hal_usbh.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/usbh/hal_usbh_debug.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/usbh/hal_usbh_desciter.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/usbh/hal_usbh_hub.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/usbh/hal_usbh_msd.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/usbh/hal_usbh_ftdi.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/usbh/hal_usbh_aoa.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/usbh/hal_usbh_hid.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/usbh/hal_usbh_uvc.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/hal_ee24xx.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/hal_ee25xx.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/hal_eeprom.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/hal_timcap.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/hal_qei.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/hal_usb_hid.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/hal_usb_msd.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/hal_comp.c \
                  ${CHIBIOS_CONTRIB}/os/hal/src/hal_opamp.c
endif

HALINC_CONTRIB := ${CHIBIOS_CONTRIB}/os/hal/include

# Shared variables
ALLCSRC += $(HALSRC_CONTRIB)
ALLINC  += $(HALINC_CONTRIB)

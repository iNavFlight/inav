include ${CHIBIOS}/os/hal/hal.mk

HALSRC += ${CHIBIOS_CONTRIB}/os/hal/src/hal_community.c \
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
          ${CHIBIOS_CONTRIB}/os/hal/src/usbh/hal_usbh_uvc.c \
          ${CHIBIOS_CONTRIB}/os/hal/src/hal_ee24xx.c \
          ${CHIBIOS_CONTRIB}/os/hal/src/hal_ee25xx.c \
          ${CHIBIOS_CONTRIB}/os/hal/src/hal_eeprom.c \
          ${CHIBIOS_CONTRIB}/os/hal/src/hal_timcap.c \
          ${CHIBIOS_CONTRIB}/os/hal/src/hal_qei.c \
          ${CHIBIOS_CONTRIB}/os/hal/src/hal_usb_hid.c

HALINC += ${CHIBIOS_CONTRIB}/os/hal/include

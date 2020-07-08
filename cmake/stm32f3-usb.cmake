set(STM32_USBFS_DIR "${INAV_LIB_DIR}/main/STM32_USB-FS-Device_Driver")

set(STM32_USBFS_SRC
    usb_core.c
    usb_init.c
    usb_int.c
    usb_mem.c
    usb_regs.c
    usb_sil.c
)
list(TRANSFORM STM32_USBFS_SRC PREPEND "${STM32_USBFS_DIR}/src/")

set(STM32F3_USB_INCLUDE_DIRS
    ${STM32_USBFS_DIR}/inc
)
set(STM32F3_USB_SRC ${STM32_USBFS_SRC})

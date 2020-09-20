set(STM32_USBOTG_DIR "${MAIN_LIB_DIR}/main/STM32_USB_OTG_Driver")
set(STM32_USBCORE_DIR "${MAIN_LIB_DIR}/main/STM32_USB_Device_Library/Core")
set(STM32_USBCDC_DIR "${MAIN_LIB_DIR}/main/STM32_USB_Device_Library/Class/cdc")
set(STM32_USBHID_DIR "${MAIN_LIB_DIR}/main/STM32_USB_Device_Library/Class/hid")
set(STM32_USBWRAPPER_DIR "${MAIN_LIB_DIR}/main/STM32_USB_Device_Library/Class/hid_cdc_wrapper")
set(STM32_USBMSC_DIR "${MAIN_LIB_DIR}/main/STM32_USB_Device_Library/Class/msc")

set(STM32F4_USB_INCLUDE_DIRS
    "${STM32_USBOTG_DIR}/inc"
    "${STM32_USBCORE_DIR}/inc"
    "${STM32_USBCDC_DIR}/inc"
    "${STM32_USBHID_DIR}/inc"
    "${STM32_USBWRAPPER_DIR}/inc"
    "${STM32_USBMSC_DIR}/inc"
)

set(STM32_USBOTG_SRC
    usb_core.c
    usb_dcd.c
    usb_dcd_int.c
)
list(TRANSFORM STM32_USBOTG_SRC PREPEND "${STM32_USBOTG_DIR}/src/")

set(STM32_USBCORE_SRC
    usbd_core.c
    usbd_ioreq.c
    usbd_req.c
)
list(TRANSFORM STM32_USBCORE_SRC PREPEND  "${STM32_USBCORE_DIR}/src/")

set(STM32_USBCDC_SRC
    "${STM32_USBCDC_DIR}/src/usbd_cdc_core.c"
)

set(STM32_USBHID_SRC
    "${STM32_USBHID_DIR}/src/usbd_hid_core.c"
)

set(STM32_USBWRAPPER_SRC
    "${STM32_USBWRAPPER_DIR}/src/usbd_hid_cdc_wrapper.c"
)

set(STM32F4_USBMSC_SRC
    usbd_msc_bot.c
    usbd_msc_core.c
    usbd_msc_data.c
    usbd_msc_scsi.c
)
main_sources(STM32F4_MSC_SRC
    msc/usbd_msc_desc.c
)
list(TRANSFORM STM32F4_USBMSC_SRC PREPEND "${STM32_USBMSC_DIR}/src/")
list(APPEND STM32F4_USBMSC_SRC ${STM32F4_MSC_SRC})

list(APPEND STM32F4_USB_SRC ${STM32_USBOTG_SRC})
list(APPEND STM32F4_USB_SRC ${STM32_USBCORE_SRC})
list(APPEND STM32F4_USB_SRC ${STM32_USBCDC_SRC})
list(APPEND STM32F4_USB_SRC ${STM32_USBHID_SRC})
list(APPEND STM32F4_USB_SRC ${STM32_USBWRAPPER_SRC})

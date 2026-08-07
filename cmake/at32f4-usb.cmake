set(AT32_USBCORE_DIR "${MAIN_LIB_DIR}/main/AT32F43x/Middlewares/AT/AT32_USB_Device_Library/Core")
set(AT32_USBCDC_DIR "${MAIN_LIB_DIR}/main/AT32F43x/Middlewares/AT/AT32_USB_Device_Library/Class/usbd_class/cdc")
set(AT32_USBMSC_DIR "${MAIN_LIB_DIR}/main/AT32F43x/Middlewares/AT/AT32_USB_Device_Library/Class/usbd_class/msc")

set(AT32F4_USB_INCLUDE_DIRS
    "${AT32_USBCORE_DIR}/Inc"
    "${AT32_USBCDC_DIR}"
    "${AT32_USBMSC_DIR}"
)

set(AT32_USBCORE_SRC
    usb_core.c
    usbd_core.c
    usbd_int.c
    usbd_sdr.c
)
list(TRANSFORM AT32_USBCORE_SRC PREPEND  "${AT32_USBCORE_DIR}/Src/")


set(AT32_USBCDC_SRC
    "${AT32_USBCDC_DIR}/cdc_class.c"
    "${AT32_USBCDC_DIR}/cdc_desc.c"
)

main_sources(AT32F4_VCP_SRC 
    drivers/serial_usb_vcp_at32f43x.c 
    drivers/usb_io.c
)

set(AT32F4_USBMSC_SRC
    msc_desc.c
    msc_class.c
    msc_bot_scsi.c 
)

main_sources(AT32F4_MSC_SRC 
    drivers/usb_msc_at32f43x.c 
)

list(TRANSFORM AT32F4_USBMSC_SRC PREPEND "${AT32_USBMSC_DIR}/")
list(APPEND AT32F4_USBMSC_SRC ${AT32F4_MSC_SRC})

list(APPEND AT32F4_USB_SRC ${AT32F4_VCP_SRC})
list(APPEND AT32F4_USB_SRC ${AT32_USBCORE_SRC})
list(APPEND AT32F4_USB_SRC ${AT32_USBCDC_SRC})

/*
    ChibiOS - Copyright (C) 2016 Jonathan Struebel

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    hal_usb_hid.h
 * @brief   USB HID macros and structures.
 *
 * @addtogroup USB_HID
 * @{
 */

#ifndef HAL_USB_HID_H
#define HAL_USB_HID_H

#if (HAL_USE_USB_HID == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    HID specific messages.
 * @{
 */
#define HID_GET_REPORT                      0x01U
#define HID_GET_IDLE                        0x02U
#define HID_GET_PROTOCOL                    0x03U
#define HID_SET_REPORT                      0x09U
#define HID_SET_IDLE                        0x0AU
#define HID_SET_PROTOCOL                    0x0BU
/** @} */

/**
 * @name    HID classes
 * @{
 */
#define HID_INTERFACE_CLASS                 0x03U
/** @} */

/**
 * @name    HID subclasses
 * @{
 */
#define HID_BOOT_INTERFACE                  0x01U
/** @} */

/**
 * @name    HID descriptors
 * @{
 */
#define USB_DESCRIPTOR_HID                  0x21U
#define HID_REPORT                          0x22U
#define HID_PHYSICAL                        0x23U
/** @} */

/**
 * @name    HID Report items
 * @{
 */
#define HID_REPORT_INPUT                    0x80
#define HID_REPORT_OUTPUT                   0x90
#define HID_REPORT_COLLECTION               0xA0
#define HID_REPORT_FEATURE                  0xB0
#define HID_REPORT_END_COLLECTION           0xC0

#define HID_REPORT_USAGE_PAGE               0x04
#define HID_REPORT_LOGICAL_MINIMUM          0x14
#define HID_REPORT_LOGICAL_MAXIMUM          0x24
#define HID_REPORT_PHYSICAL_MINIMUM         0x34
#define HID_REPORT_PHYSICAL_MAXIMUM         0x44
#define HID_REPORT_UNIT_EXPONENT            0x54
#define HID_REPORT_UNIT                     0x64
#define HID_REPORT_REPORT_SIZE              0x74
#define HID_REPORT_REPORT_ID                0x84
#define HID_REPORT_REPORT_COUNT             0x94
#define HID_REPORT_REPORT_PUSH              0xA4
#define HID_REPORT_REPORT_POP               0xB4

#define HID_REPORT_USAGE                    0x08
#define HID_REPORT_USAGE_MINIMUM            0x18
#define HID_REPORT_USAGE_MAXIMUM            0x28
#define HID_REPORT_DESIGNATOR_INDEX         0x38
#define HID_REPORT_DESIGNATOR_MINUMUM       0x48
#define HID_REPORT_DESIGNATOR_MAXIMUM       0x58
#define HID_REPORT_STRING_INDEX             0x78
#define HID_REPORT_STRING_MINUMUM           0x88
#define HID_REPORT_STRING_MAXIMUM           0x98
#define HID_REPORT_DELIMITER                0xA8
/** @} */

/**
 * @name    HID Collection item definitions
 * @{
 */
#define HID_COLLECTION_PHYSICAL             0x00
#define HID_COLLECTION_APPLICATION          0x01
#define HID_COLLECTION_LOGICAL              0x02
#define HID_COLLECTION_REPORT               0x03
#define HID_COLLECTION_NAMED_ARRAY          0x04
#define HID_COLLECTION_USAGE_SWITCH         0x05
#define HID_COLLECTION_USAGE_MODIFIER       0x06
/** @} */

/**
 * @name    HID Usage Page item definitions
 * @{
 */
#define HID_USAGE_PAGE_GENERIC_DESKTOP      0x01
#define HID_USAGE_PAGE_SIMULATION           0x02
#define HID_USAGE_PAGE_VR                   0x03
#define HID_USAGE_PAGE_SPORT                0x04
#define HID_USAGE_PAGE_GAME                 0x05
#define HID_USAGE_PAGE_GENERIC_DEVICE       0x06
#define HID_USAGE_PAGE_KEYBOARD_KEYPAD      0x07
#define HID_USAGE_PAGE_LEDS                 0x08
#define HID_USAGE_PAGE_BUTTON               0x09
#define HID_USAGE_PAGE_ORDINAL              0x0A
#define HID_USAGE_PAGE_TELEPHONY            0x0B
#define HID_USAGE_PAGE_CONSUMER             0x0C
#define HID_USAGE_PAGE_DIGITIZER            0x0D
#define HID_USAGE_PAGE_PID                  0x0F
#define HID_USAGE_PAGE_UNICODE              0x10
#define HID_USAGE_PAGE_VENDOR               0xFF00
/** @} */

/**
 * @name    HID Usage item definitions
 * @{
 */
#define HID_USAGE_ALPHANUMERIC_DISPLAY      0x14
#define HID_USAGE_MEDICAL_INSTRUMENTS       0x40
#define HID_USAGE_MONITOR_PAGE1             0x80
#define HID_USAGE_MONITOR_PAGE2             0x81
#define HID_USAGE_MONITOR_PAGE3             0x82
#define HID_USAGE_MONITOR_PAGE4             0x83
#define HID_USAGE_POWER_PAGE1               0x84
#define HID_USAGE_POWER_PAGE2               0x85
#define HID_USAGE_POWER_PAGE3               0x86
#define HID_USAGE_POWER_PAGE4               0x87
#define HID_USAGE_BAR_CODE_SCANNER_PAGE     0x8C
#define HID_USAGE_SCALE_PAGE                0x8D
#define HID_USAGE_MSR_PAGE                  0x8E
#define HID_USAGE_CAMERA_PAGE               0x90
#define HID_USAGE_ARCADE_PAGE               0x91

#define HID_USAGE_POINTER                   0x01
#define HID_USAGE_MOUSE                     0x02
#define HID_USAGE_JOYSTICK                  0x04
#define HID_USAGE_GAMEPAD                   0x05
#define HID_USAGE_KEYBOARD                  0x06
#define HID_USAGE_KEYPAD                    0x07
#define HID_USAGE_MULTIAXIS_CONTROLLER      0x08

#define HID_USAGE_BUTTON1                   0x01
#define HID_USAGE_BUTTON2                   0x02
#define HID_USAGE_BUTTON3                   0x03
#define HID_USAGE_BUTTON4                   0x04
#define HID_USAGE_BUTTON5                   0x05
#define HID_USAGE_BUTTON6                   0x06
#define HID_USAGE_BUTTON7                   0x07
#define HID_USAGE_BUTTON8                   0x08

#define HID_USAGE_X                         0x30
#define HID_USAGE_Y                         0x31
#define HID_USAGE_Z                         0x32
#define HID_USAGE_RX                        0x33
#define HID_USAGE_RY                        0x34
#define HID_USAGE_RZ                        0x35
#define HID_USAGE_VX                        0x40
#define HID_USAGE_VY                        0x41
#define HID_USAGE_VZ                        0x42
#define HID_USAGE_VBRX                      0x43
#define HID_USAGE_VBRY                      0x44
#define HID_USAGE_VBRZ                      0x45
#define HID_USAGE_VNO                       0x46
/** @} */

/**
 * @name    HID item types definitions
 * @{
 */
#define HID_ITEM_DATA                       0x00
#define HID_ITEM_CNST                       0x01
#define HID_ITEM_ARR                        0x00
#define HID_ITEM_VAR                        0x02
#define HID_ITEM_ABS                        0x00
#define HID_ITEM_REL                        0x04
#define HID_ITEM_NWRP                       0x00
#define HID_ITEM_WRP                        0x08
#define HID_ITEM_LIN                        0x00
#define HID_ITEM_NLIN                       0x10
#define HID_ITEM_PRF                        0x00
#define HID_ITEM_NPRF                       0x20
#define HID_ITEM_NNUL                       0x00
#define HID_ITEM_NUL                        0x40
#define HID_ITEM_NVOL                       0x00
#define HID_ITEM_VOL                        0x80

#define HID_ITEM_DATA_VAR_ABS               (HID_ITEM_DATA |                \
                                             HID_ITEM_VAR |                 \
                                             HID_ITEM_ABS)
#define HID_ITEM_CNST_VAR_ABS               (HID_ITEM_CNST |                \
                                             HID_ITEM_VAR |                 \
                                             HID_ITEM_ABS)
#define HID_ITEM_DATA_VAR_REL               (HID_ITEM_DATA |                \
                                             HID_ITEM_VAR |                 \
                                             HID_ITEM_REL)
/** @} */

/**
 * @name    Helper macros for USB HID descriptors
 * @{
 */
/*
 * @define  HID Descriptor size.
 */
#define USB_DESC_HID_SIZE                   9U

/**
 * @brief   HID Descriptor helper macro.
 * @note    This macro can only be used with a single HID report descriptor
 */
#define USB_DESC_HID(bcdHID, bCountryCode, bNumDescriptors,                 \
                        bDescriptorType, wDescriptorLength)                 \
  USB_DESC_BYTE(USB_DESC_HID_SIZE),                                         \
  USB_DESC_BYTE(USB_DESCRIPTOR_HID),                                        \
  USB_DESC_BCD(bcdHID),                                                     \
  USB_DESC_BYTE(bCountryCode),                                              \
  USB_DESC_BYTE(bNumDescriptors),                                           \
  USB_DESC_BYTE(bDescriptorType),                                           \
  USB_DESC_WORD(wDescriptorLength)

/**
 * @brief   HID Report item helper macro (Single byte).
 */
#define HID_ITEM_B(id, value)                                               \
  USB_DESC_BYTE(id | 0x01),                                                 \
  USB_DESC_BYTE(value)

/**
 * @brief   HID Report item helper macro (Double byte).
 */
#define HID_ITEM_W(id, value)                                               \
  USB_DESC_BYTE(id | 0x02),                                                 \
  USB_DESC_WORD(value)

/**
 * @brief   HID Report Usage Page item helper macro (Single byte).
 */
#define HID_USAGE_PAGE_B(up)                                                \
  HID_ITEM_B(HID_REPORT_USAGE_PAGE, up)

/**
 * @brief   HID Report Usage Page item helper macro (Double byte).
 */
#define HID_USAGE_PAGE_W(up)                                                \
  HID_ITEM_W(HID_REPORT_USAGE_PAGE, up)

/**
 * @brief   HID Report Usage item helper macro (Single byte).
 */
#define HID_USAGE_B(u)                                                      \
  HID_ITEM_B(HID_REPORT_USAGE, u)

/**
 * @brief   HID Report Usage item helper macro (Double byte).
 */
#define HID_USAGE_W(u)                                                      \
  HID_ITEM_W(HID_REPORT_USAGE, u)

/**
 * @brief   HID Report Collection item helper macro (Single Byte).
 */
#define HID_COLLECTION_B(c)                                                 \
  HID_ITEM_B(HID_REPORT_COLLECTION, c)

/**
 * @brief   HID Report Collection item helper macro (Double Byte).
 */
#define HID_COLLECTION_W(c)                                                 \
  HID_ITEM_W(HID_REPORT_COLLECTION, c)

/**
 * @brief   HID Report End Collection item helper macro.
 */
#define HID_END_COLLECTION                                                  \
  USB_DESC_BYTE(HID_REPORT_END_COLLECTION)

/**
 * @brief   HID Report Usage Minimum item helper macro (Single byte).
 */
#define HID_USAGE_MINIMUM_B(x)                                              \
  HID_ITEM_B(HID_REPORT_USAGE_MINIMUM, x)
  
/**
 * @brief   HID Report Usage Minimum item helper macro (Double byte).
 */
#define HID_USAGE_MINIMUM_W(x)                                              \
  HID_ITEM_W(HID_REPORT_USAGE_MINIMUM, x)

/**
 * @brief   HID Report Usage Maximum item helper macro (Single byte).
 */
#define HID_USAGE_MAXIMUM_B(x)                                              \
  HID_ITEM_B(HID_REPORT_USAGE_MAXIMUM, x)
  
/**
 * @brief   HID Report Usage Maximum item helper macro (Double byte).
 */
#define HID_USAGE_MAXIMUM_W(x)                                              \
  HID_ITEM_W(HID_REPORT_USAGE_MAXIMUM, x)

/**
 * @brief   HID Report Logical Minimum item helper macro (Single byte).
 */
#define HID_LOGICAL_MINIMUM_B(x)                                            \
  HID_ITEM_B(HID_REPORT_LOGICAL_MINIMUM, x)

/**
 * @brief   HID Report Logical Minimum item helper macro (Double byte).
 */
#define HID_LOGICAL_MINIMUM_W(x)                                            \
  HID_ITEM_W(HID_REPORT_LOGICAL_MINIMUM, x)

/**
 * @brief   HID Report Logical Maximum item helper macro (Single byte).
 */
#define HID_LOGICAL_MAXIMUM_B(x)                                            \
  HID_ITEM_B(HID_REPORT_LOGICAL_MAXIMUM, x)

/**
 * @brief   HID Report Logical Maximum item helper macro (Double byte).
 */
#define HID_LOGICAL_MAXIMUM_W(x)                                            \
  HID_ITEM_W(HID_REPORT_LOGICAL_MAXIMUM, x)

/**
 * @brief   HID Report ID item helper macro (Single byte).
 */
#define HID_REPORT_ID_B(x)                                               \
  HID_ITEM_B(HID_REPORT_REPORT_ID, x)

/**
 * @brief   HID Report ID item helper macro (Double byte).
 */
#define HID_REPORT_ID_W(x)                                               \
  HID_ITEM_W(HID_REPORT_REPORT_ID, x)
  
/**
 * @brief   HID Report Count item helper macro (Single byte).
 */
#define HID_REPORT_COUNT_B(x)                                               \
  HID_ITEM_B(HID_REPORT_REPORT_COUNT, x)

/**
 * @brief   HID Report Count item helper macro (Double byte).
 */
#define HID_REPORT_COUNT_W(x)                                               \
  HID_ITEM_W(HID_REPORT_REPORT_COUNT, x)

/**
 * @brief   HID Report Size item helper macro (Single byte).
 */
#define HID_REPORT_SIZE_B(x)                                                \
  HID_ITEM_B(HID_REPORT_REPORT_SIZE, x)

/**
 * @brief   HID Report Size item helper macro (Double byte).
 */
#define HID_REPORT_SIZE_W(x)                                                \
  HID_ITEM_W(HID_REPORT_REPORT_SIZE, x)

/**
 * @brief   HID Report Input item helper macro (Single byte).
 */
#define HID_INPUT_B(x)                                                      \
  HID_ITEM_B(HID_REPORT_INPUT, x)

/**
 * @brief   HID Report Input item helper macro (Double byte).
 */
#define HID_INPUT_W(x)                                                      \
  HID_ITEM_W(HID_REPORT_INPUT, x)
/** @} */

/**
 * @brief   HID Report Output item helper macro (Single byte).
 */
#define HID_OUTPUT_B(x)                                                     \
  HID_ITEM_B(HID_REPORT_OUTPUT, x)

/**
 * @brief   HID Report Output item helper macro (Double byte).
 */
#define HID_OUTPUT_W(x)                                                     \
  HID_ITEM_W(HID_REPORT_OUTPUT, x)
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    USB HID configuration options
 * @{
 */
/**
 * @brief   USB HID buffers size.
 * @details Configuration parameter, the buffer size must be a multiple of
 *          the USB data endpoint maximum packet size.
 * @note    The default is 256 bytes for both the transmission and receive
 *          buffers.
 */
#if !defined(USB_HID_BUFFERS_SIZE) || defined(__DOXYGEN__)
#define USB_HID_BUFFERS_SIZE        256
#endif

/**
 * @brief   USB HID number of buffers.
 * @note    The default is 2 buffers.
 */
#if !defined(USB_HID_BUFFERS_NUMBER) || defined(__DOXYGEN__)
#define USB_HID_BUFFERS_NUMBER      2
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if HAL_USE_USB == FALSE
#error "USB HID Driver requires HAL_USE_USB"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief Driver state machine possible states.
 */
typedef enum {
  HID_UNINIT = 0,                   /**< Not initialized.                   */
  HID_STOP = 1,                     /**< Stopped.                           */
  HID_READY = 2                     /**< Ready.                             */
} hidstate_t;

/**
 * @brief   Structure representing a USB HID driver.
 */
typedef struct USBHIDDriver USBHIDDriver;

/**
 * @brief   USB HID Driver configuration structure.
 * @details An instance of this structure must be passed to @p hidStart()
 *          in order to configure and start the driver operations.
 */
typedef struct {
  /**
   * @brief   USB driver to use.
   */
  USBDriver                 *usbp;
  /**
   * @brief   Interrupt IN endpoint used for outgoing data transfer.
   */
  usbep_t                   int_in;
  /**
   * @brief   Interrupt OUT endpoint used for incoming data transfer.
   */
  usbep_t                   int_out;
} USBHIDConfig;

/**
 * @brief   @p USBHIDDriver specific data.
 */
#define _usb_hid_driver_data                                                \
  _base_asynchronous_channel_data                                           \
  /* Driver state.*/                                                        \
  hidstate_t                state;                                          \
  /* Input buffers queue.*/                                                 \
  input_buffers_queue_t     ibqueue;                                        \
  /* Output queue.*/                                                        \
  output_buffers_queue_t    obqueue;                                        \
  /* Input buffer.*/                                                        \
  uint8_t                   ib[BQ_BUFFER_SIZE(USB_HID_BUFFERS_NUMBER,       \
                                              USB_HID_BUFFERS_SIZE)];       \
  /* Output buffer.*/                                                       \
  uint8_t                   ob[BQ_BUFFER_SIZE(USB_HID_BUFFERS_NUMBER,       \
                                              USB_HID_BUFFERS_SIZE)];       \
  /* End of the mandatory fields.*/                                         \
  /* Current configuration data.*/                                          \
  const USBHIDConfig        *config;

/**
 * @brief   @p USBHIDDriver specific methods.
 */
#define _usb_hid_driver_methods                                             \
  _base_asynchronous_channel_methods                                        \
  /* Buffer flush method.*/                                                 \
  void (*flush)(void *instance);

/**
 * @extends BaseAsynchronousChannelVMT
 *
 * @brief   @p USBHIDDriver virtual methods table.
 */
struct USBHIDDriverVMT {
  _usb_hid_driver_methods
};

/**
 * @extends BaseAsynchronousChannel
 *
 * @brief   Full duplex USB HID driver class.
 * @details This class extends @p BaseAsynchronousChannel by adding physical
 *          I/O queues.
 */
struct USBHIDDriver {
  /** @brief Virtual Methods Table.*/
  const struct USBHIDDriverVMT *vmt;
  _usb_hid_driver_data
};

#define USB_DRIVER_EXT_FIELDS                                                 \
  USBHIDDriver hid

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/
#ifdef __cplusplus
extern "C" {
#endif
  void hidInit(void);
  void hidObjectInit(USBHIDDriver *uhdp);
  void hidStart(USBHIDDriver *uhdp, const USBHIDConfig *config);
  void hidStop(USBHIDDriver *uhdp);
  void hidDisconnectI(USBHIDDriver *uhdp);
  void hidConfigureHookI(USBHIDDriver *uhdp);
  bool hidRequestsHook(USBDriver *usbp);
  void hidDataTransmitted(USBDriver *usbp, usbep_t ep);
  void hidDataReceived(USBDriver *usbp, usbep_t ep);
  size_t hidWriteReport(USBHIDDriver *uhdp, uint8_t *bp, size_t n);
  size_t hidWriteReportt(USBHIDDriver *uhdp, uint8_t *bp, size_t n, systime_t timeout);
  size_t hidReadReport(USBHIDDriver *uhdp, uint8_t *bp, size_t n);
  size_t hidReadReportt(USBHIDDriver *uhdp, uint8_t *bp, size_t n, systime_t timeout);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_USB_HID */

#endif /* HAL_USB_HID_H */

/** @} */

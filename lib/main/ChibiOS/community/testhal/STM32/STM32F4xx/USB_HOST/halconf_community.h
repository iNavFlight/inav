/*
    ChibiOS - Copyright (C) 2014 Uladzimir Pylinsky aka barthess

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

#ifndef HALCONF_COMMUNITY_H
#define HALCONF_COMMUNITY_H

/**
 * @brief   Enables the community overlay.
 */
#if !defined(HAL_USE_COMMUNITY) || defined(__DOXYGEN__)
#define HAL_USE_COMMUNITY           TRUE
#endif

/**
 * @brief   Enables the FSMC subsystem.
 */
#if !defined(HAL_USE_FSMC) || defined(__DOXYGEN__)
#define HAL_USE_FSMC                FALSE
#endif

/**
 * @brief   Enables the NAND subsystem.
 */
#if !defined(HAL_USE_NAND) || defined(__DOXYGEN__)
#define HAL_USE_NAND                FALSE
#endif

/**
 * @brief   Enables the 1-wire subsystem.
 */
#if !defined(HAL_USE_ONEWIRE) || defined(__DOXYGEN__)
#define HAL_USE_ONEWIRE             FALSE
#endif

/**
 * @brief   Enables the EICU subsystem.
 */
#if !defined(HAL_USE_EICU) || defined(__DOXYGEN__)
#define HAL_USE_EICU                FALSE
#endif

/**
 * @brief   Enables the CRC subsystem.
 */
#if !defined(HAL_USE_CRC) || defined(__DOXYGEN__)
#define HAL_USE_CRC                 FALSE
#endif

/**
 * @brief   Enables the RNG subsystem.
 */
#if !defined(HAL_USE_RNG) || defined(__DOXYGEN__)
#define HAL_USE_RNG                 FALSE
#endif

/**
 * @brief   Enables the EEPROM subsystem.
 */
#if !defined(HAL_USE_EEPROM) || defined(__DOXYGEN__)
#define HAL_USE_EEPROM              FALSE
#endif

/**
 * @brief   Enables the TIMCAP subsystem.
 */
#if !defined(HAL_USE_TIMCAP) || defined(__DOXYGEN__)
#define HAL_USE_TIMCAP              FALSE
#endif

/**
 * @brief   Enables the TIMCAP subsystem.
 */
#if !defined(HAL_USE_COMP) || defined(__DOXYGEN__)
#define HAL_USE_COMP                FALSE
#endif

/**
 * @brief   Enables the QEI subsystem.
 */
#if !defined(HAL_USE_QEI) || defined(__DOXYGEN__)
#define HAL_USE_QEI                 FALSE
#endif

/**
 * @brief   Enables the USBH subsystem.
 */
#if !defined(HAL_USE_USBH) || defined(__DOXYGEN__)
#define HAL_USE_USBH                TRUE
#endif

/**
 * @brief   Enables the USB_MSD subsystem.
 */
#if !defined(HAL_USE_USB_MSD) || defined(__DOXYGEN__)
#define HAL_USE_USB_MSD             FALSE
#endif

/*===========================================================================*/
/* FSMCNAND driver related settings.                                         */
/*===========================================================================*/

/**
 * @brief   Enables the @p nandAcquireBus() and @p nanReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(NAND_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define NAND_USE_MUTUAL_EXCLUSION   TRUE
#endif

/*===========================================================================*/
/* 1-wire driver related settings.                                           */
/*===========================================================================*/
/**
 * @brief   Enables strong pull up feature.
 * @note    Disabling this option saves both code and data space.
 */
#define ONEWIRE_USE_STRONG_PULLUP   FALSE

/**
 * @brief   Enables search ROM feature.
 * @note    Disabling this option saves both code and data space.
 */
#define ONEWIRE_USE_SEARCH_ROM      TRUE

/*===========================================================================*/
/* QEI driver related settings.                                              */
/*===========================================================================*/

/**
 * @brief   Enables discard of overlow
 */
#if !defined(QEI_USE_OVERFLOW_DISCARD) || defined(__DOXYGEN__)
#define QEI_USE_OVERFLOW_DISCARD    FALSE
#endif

/**
 * @brief   Enables min max of overlow
 */
#if !defined(QEI_USE_OVERFLOW_MINMAX) || defined(__DOXYGEN__)
#define QEI_USE_OVERFLOW_MINMAX     FALSE
#endif

/*===========================================================================*/
/* EEProm driver related settings.                                           */
/*===========================================================================*/

/**
 * @brief   Enables 24xx series I2C eeprom device driver.
 * @note    Disabling this option saves both code and data space.
 */
#define EEPROM_USE_EE24XX FALSE
 /**
 * @brief   Enables 25xx series SPI eeprom device driver.
 * @note    Disabling this option saves both code and data space.
 */
#define EEPROM_USE_EE25XX FALSE

/*===========================================================================*/
/* USBH driver related settings.                                             */
/*===========================================================================*/

/* main driver */
#define HAL_USBH_PORT_DEBOUNCE_TIME                   200
#define HAL_USBH_PORT_RESET_TIMEOUT                   500
#define HAL_USBH_DEVICE_ADDRESS_STABILIZATION         20
#define HAL_USBH_CONTROL_REQUEST_DEFAULT_TIMEOUT	    OSAL_MS2I(1000)

/* MSD */
#define HAL_USBH_USE_MSD                              TRUE

#define HAL_USBHMSD_MAX_LUNS                          1
#define HAL_USBHMSD_MAX_INSTANCES                     1

/* FTDI */
#define HAL_USBH_USE_FTDI                             TRUE

#define HAL_USBHFTDI_MAX_PORTS                        1
#define HAL_USBHFTDI_MAX_INSTANCES                    1
#define HAL_USBHFTDI_DEFAULT_SPEED                    9600
#define HAL_USBHFTDI_DEFAULT_FRAMING                  (USBHFTDI_FRAMING_DATABITS_8 | USBHFTDI_FRAMING_PARITY_NONE | USBHFTDI_FRAMING_STOP_BITS_1)
#define HAL_USBHFTDI_DEFAULT_HANDSHAKE                USBHFTDI_HANDSHAKE_NONE
#define HAL_USBHFTDI_DEFAULT_XON                      0x11
#define HAL_USBHFTDI_DEFAULT_XOFF                     0x13

/* AOA */
#define HAL_USBH_USE_AOA                              TRUE

#define HAL_USBHAOA_MAX_INSTANCES                     1
/* Uncomment this if you need a filter for AOA devices:
 * #define HAL_USBHAOA_FILTER_CALLBACK            _try_aoa
 */
#define HAL_USBHAOA_DEFAULT_MANUFACTURER              "Diego MFG & Co."
#define HAL_USBHAOA_DEFAULT_MODEL                     "Diego's device"
#define HAL_USBHAOA_DEFAULT_DESCRIPTION               "Description of this device..."
#define HAL_USBHAOA_DEFAULT_VERSION                   "1.0"
#define HAL_USBHAOA_DEFAULT_URI                       NULL
#define HAL_USBHAOA_DEFAULT_SERIAL                    NULL
#define HAL_USBHAOA_DEFAULT_AUDIO_MODE                USBHAOA_AUDIO_MODE_DISABLED

/* UVC */
#define HAL_USBH_USE_UVC                              TRUE

#define HAL_USBHUVC_MAX_INSTANCES                     1
#define HAL_USBHUVC_MAX_MAILBOX_SZ                    70
#define HAL_USBHUVC_WORK_RAM_SIZE                     20000
#define HAL_USBHUVC_STATUS_PACKETS_COUNT              10

/* HID */
#define HAL_USBH_USE_HID                              TRUE
#define HAL_USBHHID_MAX_INSTANCES                     2
#define HAL_USBHHID_USE_INTERRUPT_OUT                 FALSE

/* HUB */
#define HAL_USBH_USE_HUB                              TRUE

#define HAL_USBHHUB_MAX_INSTANCES                     1
#define HAL_USBHHUB_MAX_PORTS                         6

#define HAL_USBH_USE_ADDITIONAL_CLASS_DRIVERS		  TRUE

/* debug */
#define USBH_DEBUG_ENABLE                             TRUE
#define USBH_DEBUG_USBHD                              USBHD1
#define USBH_DEBUG_SD                                 SD2
#define USBH_DEBUG_BUFFER                             25000

#define USBH_DEBUG_ENABLE_TRACE                       FALSE
#define USBH_DEBUG_ENABLE_INFO                        TRUE
#define USBH_DEBUG_ENABLE_WARNINGS                    TRUE
#define USBH_DEBUG_ENABLE_ERRORS                      TRUE

#define USBH_LLD_DEBUG_ENABLE_TRACE                   FALSE
#define USBH_LLD_DEBUG_ENABLE_INFO                    TRUE
#define USBH_LLD_DEBUG_ENABLE_WARNINGS                TRUE
#define USBH_LLD_DEBUG_ENABLE_ERRORS                  TRUE

#define USBHHUB_DEBUG_ENABLE_TRACE                    FALSE
#define USBHHUB_DEBUG_ENABLE_INFO                     TRUE
#define USBHHUB_DEBUG_ENABLE_WARNINGS                 TRUE
#define USBHHUB_DEBUG_ENABLE_ERRORS                   TRUE

#define USBHMSD_DEBUG_ENABLE_TRACE                    FALSE
#define USBHMSD_DEBUG_ENABLE_INFO                     TRUE
#define USBHMSD_DEBUG_ENABLE_WARNINGS                 TRUE
#define USBHMSD_DEBUG_ENABLE_ERRORS                   TRUE

#define USBHUVC_DEBUG_ENABLE_TRACE                    FALSE
#define USBHUVC_DEBUG_ENABLE_INFO                     TRUE
#define USBHUVC_DEBUG_ENABLE_WARNINGS                 TRUE
#define USBHUVC_DEBUG_ENABLE_ERRORS                   TRUE

#define USBHFTDI_DEBUG_ENABLE_TRACE                   FALSE
#define USBHFTDI_DEBUG_ENABLE_INFO                    TRUE
#define USBHFTDI_DEBUG_ENABLE_WARNINGS                TRUE
#define USBHFTDI_DEBUG_ENABLE_ERRORS                  TRUE

#define USBHAOA_DEBUG_ENABLE_TRACE                    FALSE
#define USBHAOA_DEBUG_ENABLE_INFO                     TRUE
#define USBHAOA_DEBUG_ENABLE_WARNINGS                 TRUE
#define USBHAOA_DEBUG_ENABLE_ERRORS                   TRUE

#define USBHHID_DEBUG_ENABLE_TRACE                    FALSE
#define USBHHID_DEBUG_ENABLE_INFO                     TRUE
#define USBHHID_DEBUG_ENABLE_WARNINGS                 TRUE
#define USBHHID_DEBUG_ENABLE_ERRORS                   TRUE

#endif /* HALCONF_COMMUNITY_H */

/** @} */

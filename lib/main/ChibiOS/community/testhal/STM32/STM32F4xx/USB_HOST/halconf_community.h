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

#ifndef _HALCONF_COMMUNITY_H_
#define _HALCONF_COMMUNITY_H_

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
 * @brief   Enables the TIMCAP subsystem.
 */
#if !defined(HAL_USE_TIMCAP) || defined(__DOXYGEN__)
#define HAL_USE_TIMCAP              FALSE
#endif

/**
 * @brief   Enables the USBH subsystem.
 */
#if !defined(HAL_USE_USBH) || defined(__DOXYGEN__)
#define HAL_USE_USBH                 TRUE
#endif


/*===========================================================================*/
/* USBH driver related settings.                                             */
/*===========================================================================*/

/* main driver */
#define HAL_USBH_PORT_DEBOUNCE_TIME                   200
#define HAL_USBH_PORT_RESET_TIMEOUT                   500
#define HAL_USBH_DEVICE_ADDRESS_STABILIZATION         20

/* MSD */
#define HAL_USBH_USE_MSD                              1

#define HAL_USBHMSD_MAX_LUNS                          1
#define HAL_USBHMSD_MAX_INSTANCES                     1

/* IAD */
#define HAL_USBH_USE_FTDI                             1

#define HAL_USBHFTDI_MAX_PORTS                        1
#define HAL_USBHFTDI_MAX_INSTANCES                    1
#define HAL_USBHFTDI_DEFAULT_SPEED                    9600
#define HAL_USBHFTDI_DEFAULT_FRAMING                  (USBHFTDI_FRAMING_DATABITS_8 | USBHFTDI_FRAMING_PARITY_NONE | USBHFTDI_FRAMING_STOP_BITS_1)
#define HAL_USBHFTDI_DEFAULT_HANDSHAKE                USBHFTDI_HANDSHAKE_NONE
#define HAL_USBHFTDI_DEFAULT_XON                      0x11
#define HAL_USBHFTDI_DEFAULT_XOFF                     0x13


/* IAD */
#define HAL_USBH_USE_IAD                              0

/* UVC */
#define HAL_USBH_USE_UVC                              0

#define HAL_USBHUVC_MAX_INSTANCES                     1
#define HAL_USBHUVC_MAX_MAILBOX_SZ                    70
#define HAL_USBHUVC_WORK_RAM_SIZE                     20000
#define HAL_USBHUVC_STATUS_PACKETS_COUNT              10


/* HUB */
#define HAL_USBH_USE_HUB                              1

#define HAL_USBHHUB_MAX_INSTANCES                     1
#define HAL_USBHHUB_MAX_PORTS                         6


/* debug */
#define USBH_DEBUG_ENABLE                             1
#define USBH_DEBUG_USBHD                              USBHD1
#define USBH_DEBUG_SD                                 SD2
#define USBH_DEBUG_BUFFER                             25000

#define USBH_DEBUG_ENABLE_TRACE                       0
#define USBH_DEBUG_ENABLE_INFO                        1
#define USBH_DEBUG_ENABLE_WARNINGS                    1
#define USBH_DEBUG_ENABLE_ERRORS                      1

#define USBH_LLD_DEBUG_ENABLE_TRACE                   0
#define USBH_LLD_DEBUG_ENABLE_INFO                    1
#define USBH_LLD_DEBUG_ENABLE_WARNINGS                1
#define USBH_LLD_DEBUG_ENABLE_ERRORS                  1

#define USBHHUB_DEBUG_ENABLE_TRACE                    0
#define USBHHUB_DEBUG_ENABLE_INFO                     1
#define USBHHUB_DEBUG_ENABLE_WARNINGS                 1
#define USBHHUB_DEBUG_ENABLE_ERRORS                   1

#define USBHMSD_DEBUG_ENABLE_TRACE                    0
#define USBHMSD_DEBUG_ENABLE_INFO                     1
#define USBHMSD_DEBUG_ENABLE_WARNINGS                 1
#define USBHMSD_DEBUG_ENABLE_ERRORS                   1

#define USBHUVC_DEBUG_ENABLE_TRACE                    0
#define USBHUVC_DEBUG_ENABLE_INFO                     1
#define USBHUVC_DEBUG_ENABLE_WARNINGS                 1
#define USBHUVC_DEBUG_ENABLE_ERRORS                   1

#define USBHFTDI_DEBUG_ENABLE_TRACE                   0
#define USBHFTDI_DEBUG_ENABLE_INFO                    1
#define USBHFTDI_DEBUG_ENABLE_WARNINGS                1
#define USBHFTDI_DEBUG_ENABLE_ERRORS                  1

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

#endif /* _HALCONF_COMMUNITY_H_ */

/** @} */

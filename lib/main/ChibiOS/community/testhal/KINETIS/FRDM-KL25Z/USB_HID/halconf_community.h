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
#define HAL_USE_USBH                FALSE
#endif

/**
 * @brief   Enables the USB_MSD subsystem.
 */
#if !defined(HAL_USE_USB_MSD) || defined(__DOXYGEN__)
#define HAL_USE_USB_MSD             FALSE
#endif

/**
 * @brief   Enables the USB_HID subsystem.
 */
#if !defined(HAL_USE_USB_HID) || defined(__DOXYGEN__)
#define HAL_USE_USB_HID             TRUE
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

#endif /* HALCONF_COMMUNITY_H */

/** @} */

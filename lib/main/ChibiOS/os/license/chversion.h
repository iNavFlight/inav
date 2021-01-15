/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    chversion.h
 * @brief   Version Module macros and structures.
 *
 * @addtogroup chibios_version
 * @details This module contains information about the ChibiOS release, it
 *          is common to all subsystems.
 * @{
 */

#ifndef CHVERSION_H
#define CHVERSION_H

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * @brief   ChibiOS product identification macro.
 */
#define _CHIBIOS_

/**
 * @brief   Stable release flag.
 */
#define CH_VERSION_STABLE       1

/**
 * @name    ChibiOS version identification
 * @{
 */
/**
 * @brief   ChibiOS version string.
 */
#define CH_VERSION              "19.1.3"

/**
 * @brief   ChibiOS version release year.
 */
#define CH_VERSION_YEAR         19

/**
 * @brief   ChibiOS version release month.
 */
#define CH_VERSION_MONTH        1

/**
 * @brief   ChibiOS version patch number.
 */
#define CH_VERSION_PATCH        3

/**
 * @brief   ChibiOS version nickname.
 */
#define CH_VERSION_NICKNAME     "Maiori"
/** @} */

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

#endif /* CHVERSION_H */

/** @} */

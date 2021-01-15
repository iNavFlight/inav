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
 * @file    chlicense.h
 * @brief   License Module macros and structures.
 *
 * @addtogroup chibios_license
 * @details This module contains all the definitions required for defining
 *          a licensing scheme for customers or public releases.
 * @{
 */

#ifndef CHLICENSE_H
#define CHLICENSE_H

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * @name   Allowed Features Levels
 * @{
 */
#define CH_FEATURES_BASIC                   0
#define CH_FEATURES_INTERMEDIATE            1
#define CH_FEATURES_FULL                    2
/** @} */

/**
 * @name    Deployment Options
 */
#define CH_DEPLOY_UNLIMITED                -1
#define CH_DEPLOY_NONE                      0
/** @} */

/**
 * @name    Licensing Options
 * @{
 */
#define CH_LICENSE_GPL                      0
#define CH_LICENSE_GPL_EXCEPTION            1
#define CH_LICENSE_COMMERCIAL_FREE          2
#define CH_LICENSE_COMMERCIAL_DEV_1000      3
#define CH_LICENSE_COMMERCIAL_DEV_5000      4
#define CH_LICENSE_COMMERCIAL_FULL          5
#define CH_LICENSE_COMMERCIAL_RUNTIME       6
#define CH_LICENSE_PARTNER                  7
/** @} */

#include "chcustomer.h"
#if CH_LICENSE == CH_LICENSE_PARTNER
#include "chpartner.h"
#endif
#if CH_LICENSE == CH_LICENSE_COMMERCIAL_RUNTIME
#include "chruntime.h"
#endif

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if (CH_LICENSE == CH_LICENSE_GPL) || defined(__DOXYGEN__)
/**
 * @brief   License identification string.
 * @details This string identifies the license in a machine-readable
 *          format.
 */
#define CH_LICENSE_TYPE_STRING              "GNU General Public License 3 (GPL3)"

/**
 * @brief   Customer identification string.
 * @details This information is only available for registered commercial users.
 */
#define CH_LICENSE_ID_STRING                "N/A"

/**
 * @brief   Customer code.
 * @details This information is only available for registered commercial users.
 */
#define CH_LICENSE_ID_CODE                  "N/A"

/**
 * @brief   Code modifiability restrictions.
 * @details This setting defines if the source code is user-modifiable or not.
 */
#define CH_LICENSE_MODIFIABLE_CODE          TRUE

/**
 * @brief   Code functionality restrictions.
 */
#define CH_LICENSE_FEATURES                 CH_FEATURES_FULL

/**
 * @brief   Code deploy restrictions.
 * @details This is the per-core deploy limit allowed under the current
 *          license scheme.
 */
#define CH_LICENSE_MAX_DEPLOY               CH_DEPLOY_UNLIMITED

#elif CH_LICENSE == CH_LICENSE_GPL_EXCEPTION
#define CH_LICENSE_TYPE_STRING              "GNU General Public License 3 (GPL3) + Exception"
#define CH_LICENSE_ID_STRING                "N/A"
#define CH_LICENSE_ID_CODE                  "N/A"
#define CH_LICENSE_MODIFIABLE_CODE          FALSE
#define CH_LICENSE_FEATURES                 CH_FEATURES_BASIC
#define CH_LICENSE_MAX_DEPLOY               CH_DEPLOY_UNLIMITED

#elif CH_LICENSE == CH_LICENSE_COMMERCIAL_FREE
#define CH_LICENSE_TYPE_STRING              "Zero Cost Registered License for 500 Cores"
#define CH_LICENSE_ID_STRING                "N/A"
#define CH_LICENSE_ID_CODE                  "2017-0000"
#define CH_LICENSE_MODIFIABLE_CODE          FALSE
#define CH_LICENSE_FEATURES                 CH_FEATURES_INTERMEDIATE
#define CH_LICENSE_MAX_DEPLOY               500

#elif CH_LICENSE == CH_LICENSE_COMMERCIAL_DEV_1000
#define CH_LICENSE_TYPE_STRING              "Developer Commercial License for 1000 Cores"
#define CH_LICENSE_ID_STRING                CH_CUSTOMER_ID_STRING
#define CH_LICENSE_ID_CODE                  CH_CUSTOMER_ID_CODE
#define CH_LICENSE_MODIFIABLE_CODE          TRUE
#define CH_LICENSE_FEATURES                 CH_FEATURES_FULL
#define CH_LICENSE_DEPLOY_LIMIT             1000

#elif CH_LICENSE == CH_LICENSE_COMMERCIAL_DEV_5000
#define CH_LICENSE_TYPE_STRING              "Developer Commercial License for 5000 Cores"
#define CH_LICENSE_ID_STRING                CH_CUSTOMER_ID_STRING
#define CH_LICENSE_ID_CODE                  CH_CUSTOMER_ID_CODE
#define CH_LICENSE_MODIFIABLE_CODE          TRUE
#define CH_LICENSE_FEATURES                 CH_FEATURES_FULL
#define CH_LICENSE_DEPLOY_LIMIT             5000

#elif CH_LICENSE == CH_LICENSE_COMMERCIAL_FULL
#define CH_LICENSE_TYPE_STRING              "Full Commercial License for Unlimited Deployment"
#define CH_LICENSE_ID_STRING                CH_CUSTOMER_ID_STRING
#define CH_LICENSE_ID_CODE                  CH_CUSTOMER_ID_CODE
#define CH_LICENSE_MODIFIABLE_CODE          TRUE
#define CH_LICENSE_FEATURES                 CH_FEATURES_FULL
#define CH_LICENSE_MAX_DEPLOY               CH_DEPLOY_UNLIMITED

#elif CH_LICENSE == CH_LICENSE_COMMERCIAL_RUNTIME
#define CH_LICENSE_TYPE_STRING              "Runtime Commercial License"
#define CH_LICENSE_ID_STRING                CH_CUSTOMER_ID_STRING
#define CH_LICENSE_ID_CODE                  CH_CUSTOMER_ID_CODE
#define CH_LICENSE_MODIFIABLE_CODE          TRUE
#define CH_LICENSE_FEATURES                 CH_FEATURES_FULL
#define CH_LICENSE_MAX_DEPLOY               CH_RUNTIME_MAX_DEPLOY

#elif CH_LICENSE == CH_LICENSE_PARTNER
#define CH_LICENSE_TYPE_STRING              "Partners Special Commercial License"
#define CH_LICENSE_ID_STRING                CH_CUSTOMER_ID_STRING
#define CH_LICENSE_ID_CODE                  CH_CUSTOMER_ID_CODE
#define CH_LICENSE_MODIFIABLE_CODE          CH_PARTNER_MODIFIABLE_CODE
#define CH_LICENSE_FEATURES                 CH_PARTNER_FEATURES
#define CH_LICENSE_MAX_DEPLOY               CH_PARTNER_MAX_DEPLOY

#else
#error "invalid licensing option"
#endif

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

#endif /* CHLICENSE_H */

/** @} */

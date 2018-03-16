/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio.

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
 * @addtogroup license
 * @{
 */

#ifndef _CHLICENSE_H_
#define _CHLICENSE_H_

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
#define CH_LICENSE_COMMERCIAL_DEVELOPER     3
#define CH_LICENSE_COMMERCIAL_FULL          4
#define CH_LICENSE_PARTNER                  5
/** @} */

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Current license.
 * @note    This setting is reserved to the copyright owner.
 * @note    Changing this setting invalidates the license.
 * @note    The license statement in the source headers is valid, applicable
 *          and binding regardless this setting.
 */
#define CH_LICENSE                          CH_LICENSE_GPL

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
 * @details This setting defines which features are available under the
 *          current licensing scheme. The possible settings are:
 *          - @p CH_FEATURES_FULL if all features are available.
 *          - @p CH_FEATURES_INTERMEDIATE means that the following
 *            functionalities are disabled:
 *            - High Resolution mode.
 *            - Time Measurement.
 *            - Statistics.
 *            .
 *          - @p CH_FEATURES_BASIC means that the following functionalities
 *            are disabled:
 *            - High Resolution mode.
 *            - Time Measurement.
 *            - Statistics.
 *            - Tickless mode.
 *            - Recursive Mutexes.
 *            - Condition Variables.
 *            - Dynamic threading.
 *            .
 *          .
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
#define CH_LICENSE_TYPE_STRING              "Zero Cost Registered License"
#define CH_LICENSE_ID_STRING                "N/A"
#define CH_LICENSE_ID_CODE                  "2015-0000"
#define CH_LICENSE_MODIFIABLE_CODE          FALSE
#define CH_LICENSE_FEATURES                 CH_FEATURES_INTERMEDIATE
#define CH_LICENSE_MAX_DEPLOY               500

#elif CH_LICENSE == CH_LICENSE_COMMERCIAL_DEVELOPER
#include "chcustomer.h"
#define CH_LICENSE_TYPE_STRING              "Developer-Only Commercial License"
#define CH_LICENSE_ID_STRING                CH_CUSTOMER_ID_STRING
#define CH_LICENSE_ID_CODE                  CH_CUSTOMER_ID_CODE
#define CH_LICENSE_MODIFIABLE_CODE          TRUE
#define CH_LICENSE_FEATURES                 CH_FEATURES_FULL
#define CH_LICENSE_DEPLOY_LIMIT             5000

#elif CH_LICENSE == CH_LICENSE_COMMERCIAL_FULL
#include "chcustomer.h"
#define CH_LICENSE_TYPE_STRING              "Full Commercial License"
#define CH_LICENSE_ID_STRING                CH_CUSTOMER_ID_STRING
#define CH_LICENSE_ID_CODE                  CH_CUSTOMER_ID_CODE
#define CH_LICENSE_MODIFIABLE_CODE          TRUE
#define CH_LICENSE_FEATURES                 CH_FEATURES_FULL
#define CH_LICENSE_MAX_DEPLOY               CH_DEPLOY_UNLIMITED

#elif CH_LICENSE == CH_LICENSE_PARTNER
#include "chpartner.h"
#define CH_LICENSE_TYPE_STRING              "Partners Special Commercial License"
#define CH_LICENSE_ID_STRING                CH_PARTNER_ID_STRING
#define CH_LICENSE_ID_CODE                  CH_PARTNER_ID_CODE
#define CH_LICENSE_MODIFIABLE_CODE          CH_PARTNER_MODIFIABLE_CODE
#define CH_LICENSE_FEATURES                 CH_PARTNER_FEATURES_FULL
#define CH_LICENSE_MAX_DEPLOY               CH_PARTNER_MAX_DEPLOY

#else
#error "invalid licensing option"
#endif

/* Checks on the enabled features.*/
#if CH_LICENSE_FEATURES == CH_FEATURES_FULL
  /* No restrictions in full mode.*/

#elif (CH_LICENSE_FEATURES == CH_FEATURES_INTERMEDIATE) ||                  \
      (CH_LICENSE_FEATURES == CH_FEATURES_BASIC)
  /* Restrictions in basic and intermediate modes.*/
  #if CH_CFG_ST_TIMEDELTA > 2
    #error "CH_CFG_ST_TIMEDELTA > 2, High Resolution Time functionality restricted"
  #endif

  #if CH_DBG_STATISTICS == TRUE
    #error "CH_DBG_STATISTICS == TRUE, Statistics functionality restricted"
  #endif

  #if CH_LICENSE_FEATURES == CH_FEATURES_BASIC
    /* Restrictions in basic mode.*/
    #if CH_CFG_ST_TIMEDELTA > 0
      #error "CH_CFG_ST_TIMEDELTA > 0, Tick-Less functionality restricted"
    #endif

    #if CH_CFG_USE_TM == TRUE
      #error "CH_CFG_USE_TM == TRUE, Time Measurement functionality restricted"
    #endif

    #if CH_CFG_USE_MUTEXES_RECURSIVE == TRUE
      #error "CH_CFG_USE_MUTEXES_RECURSIVE == TRUE, Recursive Mutexes functionality restricted"
    #endif

    #if CH_CFG_USE_CONDVARS == TRUE
      #error "CH_CFG_USE_CONDVARS == TRUE, Condition Variables functionality restricted"
    #endif

    #if CH_CFG_USE_DYNAMIC == TRUE
      #error "CH_CFG_USE_DYNAMIC == TRUE, Dynamic Threads functionality restricted"
    #endif
  #endif /* CH_LICENSE_FEATURES == CH_FEATURES_BASIC */

#else
  #error "invalid feature settings"
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

#endif /* _CHLICENSE_H_ */

/** @} */

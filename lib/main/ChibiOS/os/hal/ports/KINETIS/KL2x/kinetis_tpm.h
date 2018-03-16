/*
    ChibiOS - Copyright (C) 2014 Adam J. Porter

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
 * @file    KL2x/kinetis_tpm.h
 * @brief   Kinetis TPM registers layout header.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef _KINETIS_TPM_H_
#define _KINETIS_TPM_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    TPM_SC register
 * @{
 */
#define TPM_SC_CMOD_DISABLE       (0 << 3)
#define TPM_SC_CMOD_LPTPM_CLK     (1 << 3)
#define TPM_SC_CMOD_LPTPM_EXTCLK  (2 << 3)
#define TPM_SC_CPWMS              (1 << 5)
#define TPM_SC_TOIE               (1 << 6)
#define TPM_SC_TOF                (1 << 7)
#define TPM_SC_DMA                (1 << 8)
/** @} */

/**
 * @name    TPM_MOD register
 * @{
 */
#define TPM_MOD_MASK            (0xFFFF)
/** @} */

/**
 * @name    TPM_CnSC register
 * @{
 */
#define TPM_CnSC_DMA   (1 << 0)
#define TPM_CnSC_ELSA  (1 << 2)
#define TPM_CnSC_ELSB  (1 << 3)
#define TPM_CnSC_MSA   (1 << 4)
#define TPM_CnSC_MSB   (1 << 5)
#define TPM_CnSC_CHIE  (1 << 6)
#define TPM_CnSC_CHF   (1 << 7)
/** @} */

/**
 * @name    TPM_CnV register
 * @{
 */
#define TPM_CnV_VAL_MASK (0xFFFF)
/** @} */

/**
 * @name    TPM_STATUS register
 * @{
 */
#define TPM_STATUS_CH0F (1 << 0)
#define TPM_STATUS_CH1F (1 << 1)
#define TPM_STATUS_CH2F (1 << 2)
#define TPM_STATUS_CH3F (1 << 3)
#define TPM_STATUS_CH4F (1 << 4)
#define TPM_STATUS_CH5F (1 << 5)
#define TPM_STATUS_TOF  (1 << 8)
/** @} */

/**
 * @name    TPM_CONF register
 * @{
 */
#define TPM_CONF_DOZEEN         (1 << 5)
#define TPM_CONF_DBGMODE_CONT   (3 << 6)
#define TPM_CONF_DBGMODE_PAUSE  (0 << 6)
#define TPM_CONF_GTBEEN         (1 << 9)
#define TPM_CONF_CSOT           (1 << 16)
#define TPM_CONF_CSOO           (1 << 17)
#define TPM_CONF_CROT           (1 << 18)
#define TPM_CONF_TRGSEL(n)      ((n) << 24)
/** @{ */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/
#endif /* _KINETIS_TPM_H_ */

/** @} */

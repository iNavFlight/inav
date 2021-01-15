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
 * @file    chstats.h
 * @brief   Statistics module macros and structures.
 *
 * @addtogroup statistics
 * @{
 */

#ifndef CHSTATS_H
#define CHSTATS_H

#if (CH_DBG_STATISTICS == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

#if CH_CFG_USE_TM == FALSE
#error "CH_DBG_STATISTICS requires CH_CFG_USE_TM"
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a kernel statistics structure.
 */
typedef struct {
  ucnt_t                n_irq;      /**< @brief Number of IRQs.             */
  ucnt_t                n_ctxswc;   /**< @brief Number of context switches. */
  time_measurement_t    m_crit_thd; /**< @brief Measurement of threads
                                                critical zones duration.    */
  time_measurement_t    m_crit_isr; /**< @brief Measurement of ISRs critical
                                                zones duration.             */
} kernel_stats_t;

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void _stats_init(void);
  void _stats_increase_irq(void);
  void _stats_ctxswc(thread_t *ntp, thread_t *otp);
  void _stats_start_measure_crit_thd(void);
  void _stats_stop_measure_crit_thd(void);
  void _stats_start_measure_crit_isr(void);
  void _stats_stop_measure_crit_isr(void);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

#else /* CH_DBG_STATISTICS == FALSE */

/* Stub functions for when the statistics module is disabled. */
#define _stats_increase_irq()
#define _stats_ctxswc(old, new)
#define _stats_start_measure_crit_thd()
#define _stats_stop_measure_crit_thd()
#define _stats_start_measure_crit_isr()
#define _stats_stop_measure_crit_isr()

#endif /* CH_DBG_STATISTICS == FALSE */

#endif /* CHSTATS_H */

/** @} */

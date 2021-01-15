/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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
 * @file    PIOv1/hal_pal_lld.c
 * @brief   SAMA PAL low level driver code.
 *
 * @addtogroup PAL
 * @{
 */

#include "hal.h"

#if HAL_USE_PAL || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
#define __PIOA                                 ((Pio      *)0xFC038000U)

#define EVENTS_NUMBER                          (4 * 32)

/**
 * @brief   PIOA interrupt priority level setting.
 */
#define SAMA_PIOA_IRQ_PRIORITY                 2

/**
 * @brief   PIOB interrupt priority level setting.
 */
#define SAMA_PIOB_IRQ_PRIORITY                 2

/**
 * @brief   PIOC interrupt priority level setting.
 */
#define SAMA_PIOC_IRQ_PRIORITY                 2

/**
 * @brief   PIOD interrupt priority level setting.
 */
#define SAMA_PIOD_IRQ_PRIORITY                 2
/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/
/**
 * @brief   Event records for the GPIO interrupts.
 */
palevent_t _pal_events[EVENTS_NUMBER];

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
/**
 * @brief   PIOA interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_PIOA_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t sr, imr, is;
  uint8_t i, j;

  sr = pal_lld_read_status(PIOA);
  imr = pal_lld_read_int_mask(PIOA);

  is = sr & imr;
  for (j = 0, i = 0; i < 32; i++, j++) {
    if (!(is & (0x1 << j))) {
      continue;
    }
#if (PAL_USE_CALLBACKS == TRUE) || defined(__DOXYGEN__)
    if (_pal_events[i].cb != NULL) {
      _pal_events[i].cb(&is);
    }
#endif
  }
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   PIOB interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_PIOB_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t sr, imr, is;
  uint8_t i, j;

  sr = pal_lld_read_status(PIOB);
  imr = pal_lld_read_int_mask(PIOB);

  is = sr & imr;
  for (j = 0, i = 32; i < 64; i++, j++) {
    if (!(is & (0x1 << j))) {
      continue;
    }
#if (PAL_USE_CALLBACKS == TRUE) || defined(__DOXYGEN__)
    if (_pal_events[i].cb != NULL) {
      _pal_events[i].cb(&is);
    }
#endif
  }
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   PIOC interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_PIOC_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t sr, imr, is;
  uint8_t i, j;

  sr = pal_lld_read_status(PIOC);
  imr = pal_lld_read_int_mask(PIOC);

  is = sr & imr;
  for (j = 0, i = 64; i < 96; i++, j++) {
    if (!(is & (0x1 << j))) {
      continue;
    }
#if (PAL_USE_CALLBACKS == TRUE) || defined(__DOXYGEN__)
    if (_pal_events[i].cb != NULL) {
      _pal_events[i].cb(&is);
    }
#endif
  }
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   PIOD interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_PIOD_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t sr, imr, is;
  uint8_t i, j;

  sr = pal_lld_read_status(PIOD);
  imr = pal_lld_read_int_mask(PIOD);

  is = sr & imr;
  for (j = 0, i = 96; i < 128; i++, j++) {
    if (!(is & (0x1 << j))) {
      continue;
    }
#if (PAL_USE_CALLBACKS == TRUE) || defined(__DOXYGEN__)
    if (_pal_events[i].cb != NULL) {
      _pal_events[i].cb(&is);
    }
#endif
  }
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}
/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/
/**
 * @brief   PAL driver initialization.
 *
 * @notapi
 */
void _pal_lld_init(void) {

#if PAL_USE_CALLBACKS || PAL_USE_WAIT || defined(__DOXYGEN__)
  unsigned i;

  for (i = 0; i < EVENTS_NUMBER; i++) {
    _pal_init_event(i);
  }

  /*
   * Clears status register
   */
  pal_lld_read_status(PIOA);
  pal_lld_read_status(PIOB);
  pal_lld_read_status(PIOC);
  pal_lld_read_status(PIOD);

  aicSetSourcePriority(ID_PIOA, SAMA_PIOA_IRQ_PRIORITY);
  aicSetSourceHandler(ID_PIOA, SAMA_PIOA_HANDLER);
  aicEnableInt(ID_PIOA);

  aicSetSourcePriority(ID_PIOB, SAMA_PIOB_IRQ_PRIORITY);
  aicSetSourceHandler(ID_PIOB, SAMA_PIOB_HANDLER);
  aicEnableInt(ID_PIOB);

  aicSetSourcePriority(ID_PIOC, SAMA_PIOC_IRQ_PRIORITY);
  aicSetSourceHandler(ID_PIOC, SAMA_PIOC_HANDLER);
  aicEnableInt(ID_PIOC);

  aicSetSourcePriority(ID_PIOD, SAMA_PIOD_IRQ_PRIORITY);
  aicSetSourceHandler(ID_PIOD, SAMA_PIOD_HANDLER);
  aicEnableInt(ID_PIOD);
#endif /* #if PAL_USE_CALLBACKS || PAL_USE_WAIT || defined(__DOXYGEN__) */
}

/**
 * @brief   Pads mode setup.
 * @details This function programs a pads group belonging to the same port
 *          with the specified mode.
 * @note    @p PAL_MODE_UNCONNECTED is implemented as push pull at minimum
 *          speed.
 *
 * @param[in] port      the port identifier
 * @param[in] mask      the group mask
 * @param[in] mode      the mode
 *
 * @notapi
 */
void _pal_lld_setgroupmode(ioportid_t port,
                           ioportmask_t mask,
                           iomode_t mode) {

  uint32_t mskr = (mask);
  uint32_t cfgr = (mode & (PAL_SAMA_CFGR_MASK));

#if SAMA_HAL_IS_SECURE
  if(mode && PAL_SAMA_SECURE_MASK) {
    port->SIOSR = mask;
  }
  else {
    port->SIONR = mask;
  }
#endif /* SAMA_HAL_IS_SECURE */
  port->MSKR = mskr;
  port->CFGR = cfgr;
}

#if SAMA_HAL_IS_SECURE
/**
 * @brief   Configures debouncing time for pads.
 *
 * @param[in] db_time      debouncing time
 *
 * @api
 */
void pal_lld_cfg_debouncing_time(uint32_t db_time) {

  /*
   * Debouncing time configuration only in SECURE STATE
   */
  __PIOA->S_PIO_SCDR = db_time & 0x3FFF;
}
#endif

/**
 * @brief   Reads/Clears Interrupt Status Register.
 *
 * @param[in] port      port identifier
 *
 * @api
 */
uint32_t pal_lld_read_status(ioportid_t port) {
  return port->ISR;
}

/**
 * @brief   Reads Interrupt Mask Register.
 *
 * @param[in] port      port identifier
 *
 * @api
 */
uint32_t pal_lld_read_int_mask(ioportid_t port) {
  return port->IMR;
}

/**
 * @brief   Reads Configuration Register.
 *
 * @param[in] port      port identifier
 *
 * @api
 */
uint32_t pal_lld_read_cfgr(ioportid_t port) {
  return port->CFGR;
}

#if PAL_USE_CALLBACKS || PAL_USE_WAIT || defined(__DOXYGEN__)
/**
 * @brief   Pad event enable.
 * @note    Programming an unknown or unsupported mode is silently ignored.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 * @param[in] mode      pad event mode
 *
 * @notapi
 */
void _pal_lld_enablepadevent(ioportid_t port,
                             iopadid_t pad,
                             ioeventmode_t mode) {

  port->MSKR = pad;
  port->CFGR |= mode;
  port->IER = pad;
}

/**
 * @brief   Returns a PAL event structure associated to a pad.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @notapi
 */
palevent_t* pal_lld_get_pad_event(ioportid_t port, iopadid_t pad) {

  palevent_t* palevt = NULL;

  if (port == PIOA) {
    palevt = &_pal_events[pad];
  }
  else if (port == PIOB) {
    palevt = &_pal_events[32 + pad];
  }
  else if (port == PIOC) {
    palevt = &_pal_events[64 + pad];
  }
  else {
    palevt = &_pal_events[96 + pad];
  }

  return palevt;
}

/**
 * @brief   Pad event disable.
 * @details This function disables previously programmed event callbacks.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @notapi
 */
void _pal_lld_disablepadevent(ioportid_t port, iopadid_t pad) {

  port->IDR |= pad;

#if PAL_USE_CALLBACKS || PAL_USE_WAIT
  /* Callback cleared and/or thread reset.*/
  _pal_clear_event(pad);
#endif
}
#endif /* PAL_USE_CALLBACKS || PAL_USE_WAIT */

#endif /* HAL_USE_PAL */
/** @} */

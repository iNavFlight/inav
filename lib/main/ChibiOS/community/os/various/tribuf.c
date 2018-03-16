/*
    Copyright (C) 2014..2015 Andrea Zoppi

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

#include "osal.h"
#include "tribuf.h"

/**
 * @file    tribuf.c
 * @brief   Triple buffer handler source.
 *
 * @addtogroup TriBuf
 * @{
 */

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Initializes the tribuf handler object.
 *
 * @param[in] handler Pointer to the tribuf handler object.
 * @param[in] front   Pointer to the initial front buffer.
 * @param[in] back    Pointer to the initial back buffer.
 * @param[in] orphan  Pointer to the initial orphan buffer.
 *
 * @init
 */
void tribufObjectInit(tribuf_t *handler, void *front, void *back, void *orphan) {

  handler->front = front;
  handler->back = back;
  handler->orphan = orphan;
#if (TRIBUF_USE_WAIT == TRUE)
  chSemObjectInit(&handler->ready, (cnt_t)0);
#else
  handler->ready = false;
#endif
}

/**
 * @brief   Gets the current front buffer.
 *
 * @param[in] handler   Pointer to the tribuf handler object.
 * @return  Pointer to the current front buffer.
 *
 * @api
 */
void *tribufGetFront(tribuf_t *handler) {

  void *front;

  osalSysLock();
  front = tribufGetFrontI(handler);
  osalSysUnlock();
  return front;
}

/**
 * @brief   Swaps the current front buffer.
 *
 * @details Exchanges the pointer of the current front buffer, which will be
 *          dismissed, with the pointer of the current orphan buffer, which
 *          holds the content of the new front buffer.
 *
 * @pre   The orphan buffer holds new data, swapped by the back buffer.
 * @pre   The fron buffer is ready for swap.
 * @post  The orphan buffer can be used as new back buffer in the future.
 *
 * @param[in] handler   Pointer to the tribuf handler object.
 *
 * @iclass
 */
void tribufSwapFrontI(tribuf_t *handler) {

  void *front;

  osalDbgCheckClassI();

  front = handler->orphan;
  handler->orphan = handler->front;
  handler->front = front;
}

/**
 * @brief   Swaps the current front buffer.
 *
 * @details Exchanges the pointer of the current front buffer, which will be
 *          dismissed, with the pointer of the current orphan buffer, which
 *          holds the content of the new front buffer.
 *
 * @pre   The orphan buffer holds new data, swapped by the back buffer.
 * @pre   The fron buffer is ready for swap.
 * @post  The orphan buffer can be used as new back buffer in the future.
 *
 * @param[in] handler   Pointer to the tribuf handler object.
 *
 * @api
 */
void tribufSwapFront(tribuf_t *handler) {

  osalSysLock();
  tribufSwapFrontI(handler);
  osalSysUnlock();
}

/**
 * @brief   Gets the current back buffer.
 *
 * @param[in] handler   Pointer to the tribuf handler object.
 * @return Pointer to the current back buffer.
 *
 * @api
 */
void *tribufGetBack(tribuf_t *handler) {

  void *back;

  osalSysLock();
  back = tribufGetBackI(handler);
  osalSysUnlock();
  return back;
}

/**
 * @brief   Swaps the current back buffer.
 *
 * @details Exchanges the pointer of the current back buffer, which holds new
 *          useful data, with the pointer of the current orphan buffer.
 *
 * @pre   The orphan buffer holds no meaningful data.
 * @post  The orphan buffer is candidate for new front buffer.
 * @post  A new front buffer is ready and signaled.
 *
 * @param[in] handler   Pointer to the tribuf handler object.
 *
 * @iclass
 */
void tribufSwapBackI(tribuf_t *handler) {

  void *back;

  osalDbgCheckClassI();

  back = handler->orphan;
  handler->orphan = handler->back;
  handler->back = back;

#if (TRIBUF_USE_WAIT == TRUE)
  if (chSemGetCounterI(&handler->ready) < (cnt_t)1)
    chSemSignalI(&handler->ready);
#else
  handler->ready = true;
#endif
}

/**
 * @brief   Swaps the current back buffer.
 *
 * @details Exchanges the pointer of the current back buffer, which holds new
 *          useful data, with the pointer of the current orphan buffer.
 *
 * @pre   The orphan buffer holds no meaningful data.
 * @post  The orphan buffer is candidate for new front buffer.
 * @post  A new front buffer is ready and signaled.
 *
 * @param[in] handler   Pointer to the tribuf handler object.
 *
 * @api
 */
void tribufSwapBack(tribuf_t *handler) {

  osalSysLock();
  tribufSwapBackI(handler);
#if (TRIBUF_USE_WAIT == TRUE)
  osalOsRescheduleS();
#endif
  osalSysUnlock();
}

/** @} */

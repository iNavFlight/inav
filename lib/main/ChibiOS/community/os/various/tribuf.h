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

/**
 * @file    tribuf.h
 * @brief   Triple buffer handler header.
 *
 * @addtogroup TriBuf
 * @{
 */

#ifndef TRIBUF_H_
#define TRIBUF_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Triple buffer configuration options
 * @{
 */

/**
 * @brief   Triple buffers use blocking functions.
 */
#if !defined(TRIBUF_USE_WAIT) || defined(__DOXYGEN__)
#define TRIBUF_USE_WAIT       TRUE
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Triple buffer handler object.
 */
typedef struct {
  void *front;                /**< @brief Current front buffer pointer.*/
  void *back;                 /**< @brief Current back buffer pointer.*/
  void *orphan;               /**< @brief Current orphan buffer pointer.*/
#if (TRIBUF_USE_WAIT == TRUE)
  semaphore_t ready;          /**< @brief A new front buffer is ready.*/
#else
  bool ready;                 /**< @brief A new front buffer is ready.*/
#endif
} tribuf_t;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Checks if a new front buffer is ready.
 *
 * @param[in] handler   Pointer to the tribuf handler object.
 * @return  Availability of a new front buffer.
 *
 * @iclass
 */
static inline
bool tribufIsReadyI(tribuf_t *handler)
{
  osalDbgCheckClassI();

#if (TRIBUF_USE_WAIT == TRUE)
  return (0 != chSemGetCounterI(&handler->ready));
#else
  return handler->ready;
#endif
}

#if (TRIBUF_USE_WAIT == TRUE) || defined(__DOXYGEN__)

/**
 * @brief   Waits until a new front buffer is ready, with timeout.
 *
 * @post  The ready signal, result of the back buffer swap, is consumed.
 *
 * @param[in] handler   Pointer to the tribuf handler object.
 * @param[in] timeout   Timeout of the wait operation.
 * @return  Timeout error code, as from @p chSemWaitTimeoutS.
 *
 * @see chSemWaitTimeoutS
 * @sclass
 */
static inline
msg_t tribufWaitReadyTimeoutS(tribuf_t *handler, systime_t timeout)
{
  osalDbgCheckClassS();

  return chSemWaitTimeoutS(&handler->ready, timeout);
}

/**
 * @brief   Waits until a new front buffer is ready, with timeout.
 *
 * @post  The ready signal, result of the back buffer swap, is consumed.
 *
 * @param[in] handler   Pointer to the tribuf handler object.
 * @param[in] timeout   Timeout of the wait operation.
 * @return  Timeout error code, as from @p chSemWaitTimeout.
 *
 * @see chSemWaitTimeout
 * @api
 */
static inline
msg_t tribufWaitReadyTimeout(tribuf_t *handler, systime_t timeout)
{
  return chSemWaitTimeout(&handler->ready, timeout);
}

/**
 * @brief   Waits until a new front buffer is ready.
 *
 * @post  The ready signal, result of the back buffer swap, is consumed.
 *
 * @param[in] handler   Pointer to the tribuf handler object.
 * @return  Timeout error code, as from @p chSemWaitS.
 *
 * @see chSemWaitS
 * @sclass
 */
static inline
void tribufWaitReadyS(tribuf_t *handler)
{
  osalDbgCheckClassS();

  chSemWaitS(&handler->ready);
}

/**
 * @brief   Waits until a new front buffer is ready.
 *
 * @post  The ready signal, result of the back buffer swap, is consumed.
 *
 * @param[in] handler   Pointer to the tribuf handler object.
 * @return  Timeout error code, as from @p chSemWait.
 *
 * @see chSemWait
 * @api
 */
static inline
void tribufWaitReady(tribuf_t *handler)
{
  chSemWait(&handler->ready);
}

#endif  /* (TRIBUF_USE_WAIT == TRUE) || defined(__DOXYGEN__) */

/**
 * @brief   Gets the current front buffer.
 *
 * @param[in] handler   Pointer to the tribuf handler object.
 * @return  Pointer to the current front buffer.
 *
 * @iclass
 */
static inline
void *tribufGetFrontI(tribuf_t *handler) {

  osalDbgCheckClassI();

  return handler->front;
}

/**
 * @brief   Gets the current back buffer.
 *
 * @param[in] handler   Pointer to the tribuf handler object.
 * @return  Pointer to the current back buffer.
 *
 * @iclass
 */
static inline
void *tribufGetBackI(tribuf_t *handler) {

  osalDbgCheckClassI();

  return handler->back;
}

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void tribufObjectInit(tribuf_t *handler, void *front, void *back, void *orphan);
  void *tribufGetFront(tribuf_t *handler);
  void tribufSwapFrontI(tribuf_t *handler);
  void tribufSwapFront(tribuf_t *handler);
  void *tribufGetBack(tribuf_t *handler);
  void tribufSwapBackI(tribuf_t *handler);
  void tribufSwapBack(tribuf_t *handler);
#ifdef __cplusplus
}
#endif

#endif  /* TRIBUF_H_ */
/** @} */

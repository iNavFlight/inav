/*******************************************************************************/
/*                                                                             */
/* Copyright (c) Microsoft Corporation. All rights reserved.                   */
/*                                                                             */
/* This software is licensed under the Microsoft Software License              */
/* Terms for Microsoft Azure Defender for IoT. Full text of the license can be */
/* found in the LICENSE file at https://aka.ms/AzureDefenderForIoT_EULA        */
/* and in the root directory of this software.                                 */
/*                                                                             */
/*******************************************************************************/

#ifndef _IEVENT_LOOP_H_
#define _IEVENT_LOOP_H_

#include <stdbool.h>

#include <asc_config.h>

#define EVENT_LOOP_DEFAULT_MAX_COUNT_RUN_UNTIL 100

/**
 * @file ievent_loop.h
 * @brief Event loop API.
 *
 * This API provides an implementation of an event loop handling timers and
 * signals.
 */

#include <stdint.h>
#include <stdbool.h>

/** @brief  An opaque handler representing a timer. */
typedef uintptr_t event_loop_timer_handler;
/** @brief  A callback function called when the timer expires. */
typedef void (*event_loop_timer_cb_t)(event_loop_timer_handler h, void *ctx);

/** @brief  An opaque handler representing a signal. */
typedef uintptr_t event_loop_signal_handler;
/** @brief  A callback function called when the signal raised. */
typedef void (*event_loop_signal_cb_t)(event_loop_signal_handler h, int signal_num, void *ctx);

typedef struct {
/** @brief  Initialize the event loop. Returns true on success, otherwise false */
    bool (*init)(void);

/** 
 * @brief  Unitialize the event loop.
 * @param flush perform run_until loop on deinit
 * 
 * Returns false if there are unhandled event callbacks still exists, otherwise true.
 * If parameter (flush == false) always returns true.
 */
    bool (*deinit)(bool flush);

/**
 * @brief   Handle all events registered on the loop.
 *
 * This function will only return when no more outstanding event on the loop.
 */
    void (*run)(void);

/** @brief  Run only a single iteration of the loop. */
    bool (*run_once)(void);

/**
 * @brief  Run until there are no more active and referenced handles or requests. 
 * @param  max_count maximum count of iterations
 * @return true if there are no more active and referenced handles or requests.
*/
    bool (*run_until)(int max_count);

/** @brief  Stop the even loop and return from @ref event_loop_run. */
    void (*stop)(void);

/**
 * @brief   Set a new timer event.
 *
 * The timer event will expire after @a delay milli-seconds and then, if
 * not-zero will re-expire after @a repeat milli-seconds.
 *
 * @param cb        A callback function to be called when the timer expires
 * @param ctx       A context parameter to be passed to the callback function
 * @param delay     How long, in milli-seconds, until the timer expires
 * @param repeat    How long, in milli-seconds, until the timer expires in
 *                  after the initial expiration
 * @param self      Pointer to event_timer_handler - if address is not NULL the variable will be set to NULL on timer destroy
 * 
 * @return An object representing the timer. Used later for destroying it
 */
    event_loop_timer_handler (*timer_create)(event_loop_timer_cb_t cb, void *ctx, unsigned long delay,
        unsigned long repeat, event_loop_timer_handler *self);

/**
 * @brief   Delete a timer.
 *
 * Delete a timer and free its resources. This needs to be called for
 * single-shot timers as well.
 *
 * @param handler   The timer handler to destroy
 */
    void (*timer_delete)(event_loop_timer_handler handler);

/**
 * @brief   Create a new signal handler.
 *
 * The signal handler will be called when the requested signal is received by
 * the process.
 *
 * @param cb        A callback function to be called when the signal raised
 * @param ctx       A context parameter to be passed to the callback function
 * @param signal_num   The signal number to register on
 * @param self      Pointer to event_signal_handler - if address is not NULL the variable will be set to NULL on signal destroy
 * 
 * @return          An object representing the signal handler. Used later for destroying it
 * 
 */
    event_loop_signal_handler (*signal_create)(event_loop_signal_cb_t cb, void *ctx, int signal_num, event_loop_signal_handler *self);

/**
 * @brief   Destroy a signal handler.
 *
 * Destroy a signal handler and free its resources.
 *
 * @param s The signal handler object to destroy
 */
    void (*signal_delete)(event_loop_signal_handler handler);
    
} ievent_loop_t;

/**
 * @brief   Get global event loop object.
 *
 * @return  Global event loop object
 */
extern ievent_loop_t *ievent_loop_get_instance(void);

#endif //_IEVENT_LOOP_H_
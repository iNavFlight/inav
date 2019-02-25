/*
 * This file is part of INAV.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 * Parts of this code are based on Protothreads (coroutines)
 * written by Serge Zaitsev under MIT license
 *
 * Copyright (c) 2016 Serge Zaitsev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 */

#pragma once
#include "common/time.h"
#include "common/utils.h"
#include "drivers/time.h"

/*
    Protothreads are a extremely lightweight, stackless threads that provides a blocking context, without the overhead of per-thread stacks.
    The purpose of protothreads is to implement sequential flow of control without using complex state machines or full multi-threading.
    Protothreads provides conditional blocking inside a C function.

    A protothread runs within a single C function and cannot span over other functions. A protothread may call normal C functions,
    but cannot block inside a called function. Blocking inside nested function calls is instead made by spawning a separate protothread
    for each potentially blocking function. The advantage of this approach is that blocking is explicit: the programmer knows exactly which
    functions that may block that which functions that are not able block.

    A protothread is driven by repeated calls to the function in which the protothread is running. Each time the function is called, the
    protothread will run until it blocks or exits. Thus the scheduling of protothreads is done by the application that uses protothreads.

    WARNING:

    Because protothreads do not save the stack context across a blocking call, local variables are not preserved when the protothread blocks.
    This means that local variables should be used with extreme caution - ultimately protothread function shouldn't use local variables at all.
*/

struct ptState_s {
  int           line;
  int           returnCode;
  timeUs_t      startTime;  // Used for non-blocking delays
  timeDelta_t   delayTime;  // Used for non-blocking delays
};

// Define PT with given name
#define PROTOTHREAD(name)                                                   \
  static struct ptState_s name##_protothreadState = {0, 0, 0, 0};           \
  void name(void)                                                           \

#define STATIC_PROTOTHREAD(name)                                            \
  static struct ptState_s name##_protothreadState = {0, 0, 0, 0};           \
  static void name(void)                                                    \

// Must be the first line in each protothread
#define ptBegin(name)                                                       \
  struct ptState_s * currentPt = ptGetHandle(name);                         \
  bool ptYielded = true;                                                    \
  (void) ptYielded;                                                         \
  switch (currentPt->line) {                                                \
  case 0:

// Generate pt pointer
#define ptGetHandle(name)   (&name##_protothreadState)

// Restart protothread
#define ptRestart(handle)                                                   \
  do {                                                                      \
    (handle)->returnCode = 0;                                               \
    (handle)->line = 0;                                                     \
  } while(0)

// Returns true if protothread is stopped
#define ptIsStopped(handle) ((handle)->line < 0)

// Returns special value of protothread state (return value)
#define ptGetReturnCode(handle) ((handle)->returnCode)

// Low-level API to create continuation, normally should not be used
#define ptLabel()                                                           \
  do {                                                                      \
    currentPt->line = __LINE__;                                             \
    FALLTHROUGH;                                                            \
    case __LINE__:;                                                         \
  } while (0)

// Suspends protothread until cond becomes true
// Condition is evaluated each call and shouldn't use any local variables
#define ptWait(condition)                                                   \
  do {                                                                      \
    ptLabel();                                                              \
    if (!(condition)) {                                                     \
      return;                                                               \
    }                                                                       \
  } while (0)

#define ptWaitTimeout(condition, timeoutMs)                                 \
  do {                                                                      \
    currentPt->startTime = (timeUs_t)millis();                              \
    currentPt->delayTime = (timeoutMs);                                     \
    ptLabel();                                                              \
    if (!(condition) && ((timeDelta_t)(millis() - (currentPt)->startTime) <= (currentPt)->delayTime)) { \
      return;                                                               \
    }                                                                       \
  } while (0)

// Wait until thread finishes
#define ptWaitThread(name)                                                  \
  do {                                                                      \
    ptLabel();                                                              \
    name();                                                                 \
    if (!ptIsStopped(ptGetHandle(name))) {                                  \
      return;                                                               \
    }                                                                       \
  } while (0)

// Execute a protothread and wait for completion
#define ptSpawn(name)                                                       \
  do {                                                                      \
    ptRestart(ptGetHandle(name));                                           \
    ptWaitThread(name);                                                     \
  } while (0)

// Suspends protothread for a given amount of time
// Delay is evaluated only once
#define ptDelayMs(delay)                                                    \
  do {                                                                      \
    (currentPt)->startTime = (timeUs_t)millis();                            \
    (currentPt)->delayTime = (delay);                                       \
    ptLabel();                                                              \
    if ((timeDelta_t)(millis() - (currentPt)->startTime) <= (currentPt)->delayTime) {    \
      return;                                                               \
    }                                                                       \
  } while (0)

// Suspends protothread for a given amount of time
// Delay is evaluated only once
#define ptDelayUs(delay)                                                    \
  do {                                                                      \
    (currentPt)->startTime = (timeUs_t)micros();                            \
    (currentPt)->delayTime = (delay);                                       \
    ptLabel();                                                              \
    if ((timeDelta_t)(micros() - (currentPt)->startTime) <= (currentPt)->delayTime) {    \
      return;                                                               \
    }                                                                       \
  } while (0)

// Suspends protothread until it's called again
#define ptYield()                                                           \
  do {                                                                      \
    ptYielded = false;                                                      \
    ptLabel();                                                              \
    if (!ptYielded) {                                                       \
      return;                                                               \
    }                                                                       \
  } while (0)

// terminates current protothread pt with the given status. Protothread won't continue
#define ptStop(retCode)                                                     \
  do {                                                                      \
    currentPt->returnCode = retCode;                                        \
    currentPt->line = (-1);                                                 \
    return;                                                                 \
  } while (0)

// must be the last line in each protothread
#define ptEnd(retCode)                                                      \
    ptStop(retCode);                                                        \
    FALLTHROUGH;                                                            \
    default:                                                                \
      return;                                                               \
    }                                                                       \

//***************************
// Abstract semaphore code
typedef bool ptSemaphore_t;

#define ptSemaphoreInit(sem)    do { sem = false; } while (0)
#define ptSemaphoreWait(sem)    do { ptWait(sem); sem = false; } while (0)
#define ptSemaphoreSignal(sem)  do { sem = true; } while (0)

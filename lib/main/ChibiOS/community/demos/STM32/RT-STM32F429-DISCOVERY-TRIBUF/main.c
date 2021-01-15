/*
    Copyright (C) 2013-2015 Andrea Zoppi

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

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"
#if (HAL_USE_SERIAL_USB == TRUE)
#include "usbcfg.h"
#endif

#include "tribuf.h"
#include <string.h>
#include <stdlib.h>

#if (HAL_USE_SERIAL_USB == TRUE)
static BaseSequentialStream *const chout = (BaseSequentialStream *)&SDU1;
#else
static BaseSequentialStream *const chout = (BaseSequentialStream *)&SD1;
#endif

/*===========================================================================*/
/* Triple buffer related.                                                    */
/*===========================================================================*/

#define READER_STACK_SIZE   256
#define READER_WA_SIZE      THD_WORKING_AREA_SIZE(READER_STACK_SIZE)
#define READER_DELAY_MS     200
#define READER_PRIORITY     (NORMALPRIO + 2)

#define WRITER_STACK_SIZE   256
#define WRITER_WA_SIZE      THD_WORKING_AREA_SIZE(WRITER_STACK_SIZE)
#define WRITER_DELAY_MS     100
#define WRITER_PRIORITY     (NORMALPRIO + 1)

static thread_t *reader_tp;
static uint16_t reader_delay = READER_DELAY_MS;
static tprio_t reader_priority = READER_PRIORITY;
static bool reader_suspend = false;
static systime_t reader_timeout = TIME_INFINITE;

static thread_t *writer_tp;
static uint16_t writer_delay = WRITER_DELAY_MS;
static tprio_t writer_priority = WRITER_PRIORITY;
static bool writer_suspend = false;

static tribuf_t tribuf_handler;
static char buffer_a, buffer_b, buffer_c;

static const char text[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n";

/**
 * @brief   Reads from the front buffer.
 *
 * @return  Buffered character from @p text or special symbol.
 * @retval '.'  No new front buffer within timeout.
 */
static char read_front(void) {

  const char *front;
  msg_t error;
  char c;

  /* Wait until a new front buffer gets available with prepared data */
  error = tribufWaitReadyTimeout(&tribuf_handler, reader_timeout);
  if (error == MSG_OK) {
    /* Retrieve the new front buffer */
    tribufSwapFront(&tribuf_handler);
    front = (const char *)tribufGetFront(&tribuf_handler);

    /* Read data from the new front buffer */
    c = front[0];
  } else {
    c = '.';  /* Timeout placeholder */
  }
  return c;
}

/*
 * @brief   Overwrites the back buffer with the provided character.
 *
 * @param[in] c   Character to store into the current back buffer.
 */
static void write_back(char c) {

  char *back;

  /* Retrieve the current back buffer */
  back = (char *)tribufGetBack(&tribuf_handler);

  /* Prepare data onto the current back buffer */
  back[0] = c;

  /* Exchange the prepared buffer with a new one */
  tribufSwapBack(&tribuf_handler);
}

/*
 * Reads the front buffer and prints it.
 */
static THD_WORKING_AREA(reader_wa, READER_STACK_SIZE);
static THD_FUNCTION(reader_thread, arg) {

  thread_reference_t thread_ref;
  tprio_t old_priority;
  char c;
  (void)arg;

  chRegSetThreadName("reader_thread");
  old_priority = chThdGetPriorityX();

  for (;;) {
    /* Read from the fron buffer and print the retrieved character */
    c = read_front();
    chprintf(chout, "%c", c);

    /* Change priority, suspend or delay */
    osalSysLock();
    palTogglePad(GPIOG, GPIOG_LED3_GREEN);
    if (old_priority != reader_priority) {
      chThdSetPriority(reader_priority);
    }
    if (reader_suspend) {
      thread_ref = NULL;
      osalThreadSuspendS(&thread_ref);
      reader_suspend = false;
    } else {
      osalThreadSleepS(TIME_MS2I(reader_delay));
    }
    old_priority = chThdGetPriorityX();
    osalSysUnlock();
  }
}

/*
 * Overwrites the back buffer with a fixed text, character by character.
 */
static THD_WORKING_AREA(writer_wa, WRITER_STACK_SIZE);
static THD_FUNCTION(writer_thread, arg) {

  thread_reference_t thread_ref;
  tprio_t old_priority;
  size_t i;
  char c;
  (void)arg;

  chRegSetThreadName("writer_thread");
  old_priority = chThdGetPriorityX();

  for (;;) {
    for (i = 0; i < sizeof(text); ++i) {
      /* Write the next character on the current back buffer */
      c = text[i];
      write_back(c);

      /* Change priority, suspend or delay */
      osalSysLock();
      palTogglePad(GPIOG, GPIOG_LED4_RED);
      if (old_priority != writer_priority) {
        chThdSetPriority(writer_priority);
      }
      if (writer_suspend) {
        thread_ref = NULL;
        osalThreadSuspendS(&thread_ref);
        writer_suspend = false;
      } else {
        osalThreadSleepS(TIME_MS2I(writer_delay));
      }
      osalSysUnlock();
    }
  }
}

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define streq(s1, s2)   (strcmp((s1), (s2)) == 0)

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)
#define TEST_WA_SIZE    THD_WORKING_AREA_SIZE(256)

static void cmd_reset(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;

  if (argc > 0) {
    chprintf(chp, "Usage: reset\r\n");
    return;
  }

  chprintf(chp, "Will reset in 200ms\r\n");
  chThdSleepMilliseconds(200);
  NVIC_SystemReset();
}

static void cmd_run(BaseSequentialStream *chp, int argc, char *argv[]) {

  thread_reference_t thread_ref;
  const char *const usage = "Usage: run (reader|writer)\r\n";

  if (argc != 1) {
    chprintf(chp, usage);
    return;
  }

  if (streq(argv[0], "reader")) {
    osalSysLock();
    if (reader_suspend) {
      thread_ref = (thread_reference_t)reader_tp;
      osalThreadResumeS(&thread_ref, MSG_OK);
    }
    osalSysUnlock();
  }
  else if (streq(argv[0], "writer")) {
    osalSysLock();
    if (writer_suspend) {
      thread_ref = (thread_reference_t)writer_tp;
      osalThreadResumeS(&thread_ref, MSG_OK);
    }
    osalSysUnlock();
  }
  else {
    chprintf(chp, usage);
  }
}

static void cmd_stop(BaseSequentialStream *chp, int argc, char *argv[]) {

  const char *const usage = "Usage: stop (reader|writer)\r\n";

  if (argc != 1) {
    chprintf(chp, usage);
    return;
  }

  if (streq(argv[0], "reader")) {
    osalSysLock();
    reader_suspend = true;
    osalSysUnlock();
  }
  else if (streq(argv[0], "writer")) {
    osalSysLock();
    writer_suspend = true;
    osalSysUnlock();
  }
  else {
    chprintf(chp, usage);
  }
}

static void cmd_delay(BaseSequentialStream *chp, int argc, char *argv[]) {

  const char *const usage = "Usage: delay (reader|writer) DELAY_MS\r\n";
  uint16_t delay;

  if (argc != 2) {
    chprintf(chp, usage);
    return;
  }
  delay = (uint16_t)atoi(argv[1]);

  if (streq(argv[0], "reader")) {
    osalSysLock();
    reader_delay = delay;
    osalSysUnlock();
  }
  else if (streq(argv[0], "writer")) {
    osalSysLock();
    writer_delay = delay;
    osalSysUnlock();
  }
  else {
    chprintf(chp, usage);
  }
}

static void cmd_priority(BaseSequentialStream *chp, int argc, char *argv[]) {

  const char *const usage =
    "Usage: priority (reader|writer) THREAD_PRIORITY\r\n";
  tprio_t priority;

  if (argc != 2) {
    chprintf(chp, usage);
    return;
  }
  priority = (tprio_t)atoi(argv[1]);

  if (streq(argv[0], "reader")) {
    osalSysLock();
    reader_priority = priority;
    osalSysUnlock();
  }
  else if (streq(argv[0], "writer")) {
    osalSysLock();
    writer_priority = priority;
    osalSysUnlock();
  }
  else {
    chprintf(chp, usage);
  }
}

static void cmd_timeout(BaseSequentialStream *chp, int argc, char *argv[]) {

  const char *const usage = "Usage: timeout TIMEOUT_MS\r\n";
  systime_t timeout;

  if (argc != 1) {
    chprintf(chp, usage);
    return;
  }

  if (streq(argv[0], "-"))
    timeout = TIME_IMMEDIATE;
  else if (streq(argv[0], "*"))
    timeout = TIME_INFINITE;
  else
    timeout = (systime_t)atoi(argv[0]);

  osalSysLock();
  reader_timeout = timeout;
  osalSysUnlock();
}

static void cmd_params(BaseSequentialStream *chp, int argc, char *argv[]) {

  const char *const usage = "Usage: params\r\n";

  uint32_t reader_delay_;
  uint32_t reader_priority_;
  uint32_t reader_suspend_;
  uint32_t reader_timeout_;

  uint32_t writer_delay_;
  uint32_t writer_priority_;
  uint32_t writer_suspend_;

  (void)argv;
  if (argc != 0) {
    chprintf(chp, usage);
    return;
  }

  osalSysLock();
  reader_delay_     = (uint32_t)reader_delay;
  reader_priority_  = (uint32_t)reader_priority;
  reader_suspend_   = (uint32_t)reader_suspend;
  reader_timeout_   = (uint32_t)reader_timeout;

  writer_delay_     = (uint32_t)writer_delay;
  writer_priority_  = (uint32_t)writer_priority;
  writer_suspend_   = (uint32_t)writer_suspend;
  osalSysUnlock();

  chprintf(chp, "reader_delay       %U\r\n", reader_delay_);
  chprintf(chp, "reader_priority    %U\r\n", reader_priority_);
  chprintf(chp, "reader_suspend     %U\r\n", reader_suspend_);
  if (reader_timeout_ == TIME_IMMEDIATE)
    chprintf(chp, "reader_timeout     -\r\n");
  if (reader_timeout_ == TIME_INFINITE)
    chprintf(chp, "reader_timeout     *\r\n");
  else
    chprintf(chp, "reader_timeout     %U\r\n", reader_timeout_);

  chprintf(chp, "writer_delay       %U\r\n", writer_delay_);
  chprintf(chp, "writer_priority    %U\r\n", writer_priority_);
  chprintf(chp, "writer_suspend     %U\r\n", writer_suspend_);
}

static const ShellCommand commands[] = {
  {"reset", cmd_reset},
  {"run", cmd_run},
  {"stop", cmd_stop},
  {"delay", cmd_delay},
  {"priority", cmd_priority},
  {"timeout", cmd_timeout},
  {"params", cmd_params},
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
#if (HAL_USE_SERIAL_USB == TRUE)
  (BaseSequentialStream *)&SDU1,
#else
  (BaseSequentialStream *)&SD1,
#endif
  commands
};

/*===========================================================================*/
/* Initialization and main thread.                                           */
/*===========================================================================*/

/*
 * Application entry point.
 */
int main(void) {

  thread_t *shelltp = NULL;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

#if (HAL_USE_SERIAL_USB == TRUE)
  /*
   * Initializes a serial-over-USB CDC driver.
   */
  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1000);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);
#else
  /*
   * Initializes serial port.
   */
  sdStart(&SD1, NULL);
#endif /* HAL_USE_SERIAL_USB */

  /*
   * Writer and reader threads started for triple buffer demo.
   */
  tribufObjectInit(&tribuf_handler, &buffer_a, &buffer_b, &buffer_c);

  reader_tp = chThdCreateStatic(reader_wa, READER_WA_SIZE,
                                reader_priority, reader_thread, NULL);

  writer_tp = chThdCreateStatic(writer_wa, WRITER_WA_SIZE,
                                writer_priority, writer_thread, NULL);

  /*
   * Normal main() thread activity, in this demo it just performs
   * a shell respawn upon its termination.
   */
  for (;;) {
    if (!shelltp) {
#if (HAL_USE_SERIAL_USB == TRUE)
      if (SDU1.config->usbp->state == USB_ACTIVE) {
        /* Spawns a new shell.*/
        shelltp = chThdCreateFromHeap(NULL, SHELL_WA_SIZE, "shell", NORMALPRIO, shellThread, (void *) &shell_cfg1);
      }
#else
      shelltp = chThdCreateFromHeap(NULL, SHELL_WA_SIZE, "shell", NORMALPRIO, shellThread, (void *) &shell_cfg1);
#endif
    }
    else {
      /* If the previous shell exited.*/
      if (chThdTerminatedX(shelltp)) {
        /* Recovers memory of the previous shell.*/
        chThdRelease(shelltp);
        shelltp = NULL;
      }
    }
    chThdSleepMilliseconds(500);
  }
  return 0;
}


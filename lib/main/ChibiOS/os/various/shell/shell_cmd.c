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
 * @file    shell_cmd.c
 * @brief   Simple CLI shell common commands code.
 *
 * @addtogroup SHELL
 * @{
 */

#include <string.h>

#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "shell_cmd.h"
#include "chprintf.h"

#if (SHELL_CMD_TEST_ENABLED == TRUE) || defined(__DOXYGEN__)
#include "rt_test_root.h"
#include "oslib_test_root.h"
#endif

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

#if ((SHELL_CMD_EXIT_ENABLED == TRUE) && !defined(_CHIBIOS_NIL_)) ||        \
    defined(__DOXYGEN__)
static void cmd_exit(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    shellUsage(chp, "exit");
    return;
  }

  shellExit(MSG_OK);
}
#endif

#if (SHELL_CMD_INFO_ENABLED == TRUE) || defined(__DOXYGEN__)
static void cmd_info(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    shellUsage(chp, "info");
    return;
  }

  chprintf(chp, "Kernel:       %s" SHELL_NEWLINE_STR, CH_KERNEL_VERSION);
#ifdef PORT_COMPILER_NAME
  chprintf(chp, "Compiler:     %s" SHELL_NEWLINE_STR, PORT_COMPILER_NAME);
#endif
  chprintf(chp, "Architecture: %s" SHELL_NEWLINE_STR, PORT_ARCHITECTURE_NAME);
#ifdef PORT_CORE_VARIANT_NAME
  chprintf(chp, "Core Variant: %s" SHELL_NEWLINE_STR, PORT_CORE_VARIANT_NAME);
#endif
#ifdef PORT_INFO
  chprintf(chp, "Port Info:    %s" SHELL_NEWLINE_STR, PORT_INFO);
#endif
#ifdef PLATFORM_NAME
  chprintf(chp, "Platform:     %s" SHELL_NEWLINE_STR, PLATFORM_NAME);
#endif
#ifdef BOARD_NAME
  chprintf(chp, "Board:        %s" SHELL_NEWLINE_STR, BOARD_NAME);
#endif
#ifdef __DATE__
#ifdef __TIME__
  chprintf(chp, "Build time:   %s%s%s" SHELL_NEWLINE_STR, __DATE__, " - ", __TIME__);
#endif
#endif
}
#endif

#if (SHELL_CMD_ECHO_ENABLED == TRUE) || defined(__DOXYGEN__)
static void cmd_echo(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc != 1) {
    shellUsage(chp, "echo \"message\"");
    return;
  }
  chprintf(chp, "%s" SHELL_NEWLINE_STR, argv[0]);
}
#endif

#if (SHELL_CMD_SYSTIME_ENABLED == TRUE) || defined(__DOXYGEN__)
static void cmd_systime(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    shellUsage(chp, "systime");
    return;
  }
  chprintf(chp, "%lu" SHELL_NEWLINE_STR, (unsigned long)chVTGetSystemTime());
}
#endif

#if (SHELL_CMD_MEM_ENABLED == TRUE) || defined(__DOXYGEN__)
static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  size_t n, total, largest;

  (void)argv;
  if (argc > 0) {
    shellUsage(chp, "mem");
    return;
  }
  n = chHeapStatus(NULL, &total, &largest);
  chprintf(chp, "core free memory : %u bytes" SHELL_NEWLINE_STR, chCoreGetStatusX());
  chprintf(chp, "heap fragments   : %u" SHELL_NEWLINE_STR, n);
  chprintf(chp, "heap free total  : %u bytes" SHELL_NEWLINE_STR, total);
  chprintf(chp, "heap free largest: %u bytes" SHELL_NEWLINE_STR, largest);
}
#endif

#if (SHELL_CMD_THREADS_ENABLED == TRUE) || defined(__DOXYGEN__)
static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {CH_STATE_NAMES};
  thread_t *tp;

  (void)argv;
  if (argc > 0) {
    shellUsage(chp, "threads");
    return;
  }
  chprintf(chp, "stklimit    stack     addr refs prio     state         name\r\n" SHELL_NEWLINE_STR);
  tp = chRegFirstThread();
  do {
#if (CH_DBG_ENABLE_STACK_CHECK == TRUE) || (CH_CFG_USE_DYNAMIC == TRUE)
    uint32_t stklimit = (uint32_t)tp->wabase;
#else
    uint32_t stklimit = 0U;
#endif
    chprintf(chp, "%08lx %08lx %08lx %4lu %4lu %9s %12s" SHELL_NEWLINE_STR,
             stklimit, (uint32_t)tp->ctx.sp, (uint32_t)tp,
             (uint32_t)tp->refs - 1, (uint32_t)tp->prio, states[tp->state],
             tp->name == NULL ? "" : tp->name);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}
#endif

#if (SHELL_CMD_TEST_ENABLED == TRUE) || defined(__DOXYGEN__)
static THD_FUNCTION(test_rt, arg) {
  BaseSequentialStream *chp = (BaseSequentialStream *)arg;
  test_execute(chp, &rt_test_suite);
}

static THD_FUNCTION(test_oslib, arg) {
  BaseSequentialStream *chp = (BaseSequentialStream *)arg;
  test_execute(chp, &oslib_test_suite);
}

static void cmd_test(BaseSequentialStream *chp, int argc, char *argv[]) {
  thread_t *tp;
  tfunc_t tfp;

  (void)argv;
  if (argc != 1) {
    shellUsage(chp, "test rt|oslib");
    return;
  }
  if (!strcmp(argv[0], "rt")) {
    tfp = test_rt;
  }
  else if (!strcmp(argv[0], "oslib")) {
    tfp = test_oslib;
  }
  else {
    shellUsage(chp, "test rt|oslib");
    return;
  }
  tp = chThdCreateFromHeap(NULL, SHELL_CMD_TEST_WA_SIZE,
                           "test", chThdGetPriorityX(),
                           tfp, chp);
  if (tp == NULL) {
    chprintf(chp, "out of memory" SHELL_NEWLINE_STR);
    return;
  }
  chThdWait(tp);
}
#endif

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Array of the default commands.
 */
const ShellCommand shell_local_commands[] = {
#if (SHELL_CMD_EXIT_ENABLED == TRUE) && !defined(_CHIBIOS_NIL_)
  {"exit", cmd_exit},
#endif
#if SHELL_CMD_INFO_ENABLED == TRUE
  {"info", cmd_info},
#endif
#if SHELL_CMD_ECHO_ENABLED == TRUE
  {"echo", cmd_echo},
#endif
#if SHELL_CMD_SYSTIME_ENABLED == TRUE
  {"systime", cmd_systime},
#endif
#if SHELL_CMD_MEM_ENABLED == TRUE
  {"mem", cmd_mem},
#endif
#if SHELL_CMD_THREADS_ENABLED == TRUE
  {"threads", cmd_threads},
#endif
#if SHELL_CMD_TEST_ENABLED == TRUE
  {"test", cmd_test},
#endif
  {NULL, NULL}
};

/** @} */

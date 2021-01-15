/*
    Copyright (C) 2015 Andrea Zoppi

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
#include "tribuf.h"

#include <stdio.h>
#include <stdlib.h>

/*===========================================================================*/
/* TriBuf related.                                                           */
/*===========================================================================*/

#define WRITER_DELAY_MS        10
#define READER_DELAY_MS        20

#define WRITER_STACK_SIZE   4096
#define READER_STACK_SIZE   4096

#define WRITER_PRIORITY     (NORMALPRIO + 1)
#define READER_PRIORITY     (NORMALPRIO + 2)

static thread_t *writertp;
static thread_t *readertp;

static tribuf_t tribuf;
static char buffers[3];

static const char text[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n";

/*
 * Reads from the front buffer.
 */
static char read_front(void) {

  const char *front;
  msg_t error;

  /* Wait until a new front buffer gets available with prepared data */
  error = tribufWaitReadyTimeout(&tribuf, TIME_MS2I(1000));
  if (error == MSG_TIMEOUT)
    chSysHalt("ERROR: read_front() timed out");

  /* Retrieve the new front buffer */
  tribufSwapFront(&tribuf);
  front = (const char *)tribufGetFront(&tribuf);

  /* Read data from the new front buffer */
  return front[0];
}

/*
 * Overwrites the back buffer with the provided character.
 */
static void write_back(char c) {

  char *back;

  /* Retrieve the current back buffer */
  back = (char *)tribufGetBack(&tribuf);

  /* Prepare data onto the current back buffer */
  back[0] = c;

  /* Exchange the prepared buffer with a new one */
  tribufSwapBack(&tribuf);
}

/*
 * Overwrites the back buffer with a fixed text, character by character.
 */
static THD_WORKING_AREA(writer_wa, WRITER_STACK_SIZE);
static THD_FUNCTION(writer_thread, arg) {

  const uint32_t delay = (uint32_t)(msg_t)arg;
  size_t i;
  char c;

  chRegSetThreadName("writer_thread");
  for (;;) {
    for (i = 0; i < sizeof(text); ++i) {
      c = text[i];
      write_back(c);
      chThdSleepMilliseconds(delay);
    }
  }
}

/*
 * Reads the front buffer and prints it.
 */
static THD_WORKING_AREA(reader_wa, READER_STACK_SIZE);
static THD_FUNCTION(reader_thread, arg) {

  const uint32_t delay = (uint32_t)(msg_t)arg;
  char c;

  chRegSetThreadName("reader_thread");
  for (;;) {
    c = read_front();
    fprintf(stdout, "%c", c);
    chThdSleepMilliseconds(delay);
  }
}

/*===========================================================================*/
/* Initialization and main thread.                                           */
/*===========================================================================*/

/*
 * Simulator main.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Writer and reader threads started for triple buffer demo.
   */
  tribufObjectInit(&tribuf, &buffers[0], &buffers[1], &buffers[2]);

  readertp = chThdCreateStatic(reader_wa, sizeof(reader_wa),
                               READER_PRIORITY,
                               reader_thread, (void *)READER_DELAY_MS);

  writertp = chThdCreateStatic(writer_wa, sizeof(writer_wa),
                               WRITER_PRIORITY,
                               writer_thread, (void *)WRITER_DELAY_MS);

  /*
   * Let the threads process data.
   */
  for (;;)
    chThdSleepMilliseconds(1000);

  return 0;
}

/*
 * Critical error function.
 */
void halt(const char *reason) {

  fflush(stdout);
  fputs("\n", stdout);
  fputs(reason, stderr);
  fflush(stderr);
  exit(1);
}

/*
    ChibiOS/HAL - Copyright (C) 2016 Uladzimir Pylinsky aka barthess

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

#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "usbcfg.h"
#include "hal_usb_msd.h"

#include "ramdisk.h"
#include "romfs_img.h"

#define RAMDISK_BLOCK_SIZE    512U
#define RAMDISK_BLOCK_CNT     700U

/*
 * Red LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    systime_t time;

    time = USBD1.state == USB_ACTIVE ? 100 : 500;
    palSetPad(GPIOB, GPIOB_LED1);
    chThdSleepMilliseconds(time);
    palClearPad(GPIOB, GPIOB_LED1);
    chThdSleepMilliseconds(time);
  }
}

RamDisk ramdisk;
__attribute__((section("DATA_RAM"))) static uint8_t ramdisk_storage[RAMDISK_BLOCK_SIZE * RAMDISK_BLOCK_CNT];
static uint8_t blkbuf[RAMDISK_BLOCK_SIZE];

BaseSequentialStream *GlobalDebugChannel;

static const SerialConfig sercfg = {
    115200,
    0,
    0,
    0
};

/*
 * Application entry point.
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

  sdStart(&SD3, &sercfg);
  GlobalDebugChannel = (BaseSequentialStream *)&SD3;

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(&USBD1);
  chThdSleepMilliseconds(1500);
  usbStart(&USBD1, &usbcfg);

  /*
   * start RAM disk
   */
  ramdiskObjectInit(&ramdisk);
  memset(ramdisk_storage, 0x55, sizeof(ramdisk_storage));
  osalDbgCheck(sizeof(ramdisk_storage) >= romfs_bin_len);
  memcpy(ramdisk_storage, romfs_bin, romfs_bin_len);
  ramdiskStart(&ramdisk, ramdisk_storage, RAMDISK_BLOCK_SIZE,
               RAMDISK_BLOCK_CNT, false);

  /*
   * start mass storage
   */
  msdObjectInit(&USBMSD1);
  msdStart(&USBMSD1, &USBD1, (BaseBlockDevice *)&ramdisk, blkbuf, NULL, NULL);

  /*
   *
   */
  usbConnectBus(&USBD1);

  /*
   * Starting threads.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (true) {
    chThdSleepMilliseconds(1000);
  }

  msdStop(&USBMSD1);
}

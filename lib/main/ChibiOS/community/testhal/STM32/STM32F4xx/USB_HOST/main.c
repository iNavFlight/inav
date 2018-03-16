/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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
#include "ff.h"
#include "usbh.h"
#include <string.h>



#if HAL_USBH_USE_FTDI
#include "usbh/dev/ftdi.h"
#include "test.h"
#include "shell.h"
#include "chprintf.h"

static THD_WORKING_AREA(waTestFTDI, 1024);

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)
#define TEST_WA_SIZE    THD_WORKING_AREA_SIZE(256)

static uint8_t buf[] =
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";

static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  size_t n, size;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: mem\r\n");
    return;
  }
  n = chHeapStatus(NULL, &size);
  chprintf(chp, "core free memory : %u bytes\r\n", chCoreGetStatusX());
  chprintf(chp, "heap fragments   : %u\r\n", n);
  chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {CH_STATE_NAMES};
  thread_t *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: threads\r\n");
    return;
  }
  chprintf(chp, "    addr    stack prio refs     state\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%08lx %08lx %4lu %4lu %9s\r\n",
            (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
            (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
            states[tp->p_state]);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}

static void cmd_test(BaseSequentialStream *chp, int argc, char *argv[]) {
  thread_t *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: test\r\n");
    return;
  }
  tp = chThdCreateFromHeap(NULL, TEST_WA_SIZE, chThdGetPriorityX(),
                           TestThread, chp);
  if (tp == NULL) {
    chprintf(chp, "out of memory\r\n");
    return;
  }
  chThdWait(tp);
}

static void cmd_write(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: write\r\n");
    return;
  }

  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) != Q_TIMEOUT) {
	//flush
  }

  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chSequentialStreamWrite(&FTDIPD[0], buf, sizeof buf - 1);
  }
  chprintf(chp, "\r\n\nstopped\r\n");
}

static const ShellCommand commands[] = {
	{"mem", cmd_mem},
	{"threads", cmd_threads},
	{"test", cmd_test},
	{"write", cmd_write},
	{NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
	(BaseSequentialStream *)&FTDIPD[0],
	commands
};

static void ThreadTestFTDI(void *p) {
	(void)p;
	USBHFTDIPortDriver *const ftdipp = &FTDIPD[0];

	shellInit();

start:
	while (ftdipp->state != USBHFTDIP_STATE_ACTIVE) {
		chThdSleepMilliseconds(100);
	}

	usbDbgPuts("FTDI: Connected");

	USBHFTDIPortConfig config = {
		115200,
		USBHFTDI_FRAMING_DATABITS_8 | USBHFTDI_FRAMING_PARITY_NONE | USBHFTDI_FRAMING_STOP_BITS_1,
		USBHFTDI_HANDSHAKE_NONE,
		0,
		0
	};

	usbhftdipStart(ftdipp, &config);

	//loopback
	if (0) {
		for(;;) {
			msg_t m = chSequentialStreamGet(ftdipp);
			if (m < MSG_OK) {
				usbDbgPuts("FTDI: Disconnected");
				goto start;
			}
			chSequentialStreamPut(ftdipp, (uint8_t)m);
			if (m == 'q')
				break;
		}
	}

	//shell test
	if (1) {
		thread_t *shelltp = NULL;
		for(;;) {
			if (ftdipp->state != USBHFTDIP_STATE_READY)
				goto start;
			if (!shelltp) {
				shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
			} else if (chThdTerminatedX(shelltp)) {
				chThdRelease(shelltp);
				if (ftdipp->state != USBHFTDIP_STATE_READY)
					goto start;
				break;
			}
			chThdSleepMilliseconds(100);
		}
	}

	//FTDI uart RX to debug TX bridge
	if (0) {
		for(;;) {
			msg_t m = chSequentialStreamGet(ftdipp);
			if (m < MSG_OK) {
				usbDbgPuts("FTDI: Disconnected");
				goto start;
			}
			sdPut(&USBH_DEBUG_SD, (uint8_t)m);
			if (m == 'q')
				break;
		}
	}

	//write speed test
	if (1) {
		usbhftdipStop(ftdipp);
		config.speed = 3000000;
		usbhftdipStart(ftdipp, &config);

		systime_t st, et;
		int i;
		for (i = 0; i < 5; i++) {
			uint32_t bytes = config.speed / 10;
			uint32_t times = bytes / 1024;
			st = chVTGetSystemTimeX();
			while (times--) {
				if (chSequentialStreamWrite(ftdipp, buf, 1024) < 1024) {
					usbDbgPuts("FTDI: Disconnected");
					goto start;
				}
				bytes -= 1024;
			}
			if (bytes) {
				if (chSequentialStreamWrite(ftdipp, buf, bytes) < bytes) {
					usbDbgPuts("FTDI: Disconnected");
					goto start;
				}
			}
			et = chVTGetSystemTimeX();
			usbDbgPrintf("\tRate=%uB/s", (config.speed * 100) / (et - st));
		}
	}

	//single character write test (tests the timer)
	if (0) {
		for (;;) {
			if (chSequentialStreamPut(ftdipp, 'A') != MSG_OK) {
				usbDbgPuts("FTDI: Disconnected");
				goto start;
			}
			chThdSleepMilliseconds(100);
		}
	}

	usbhftdipStop(ftdipp);

	usbDbgPuts("FTDI: Tests done, restarting in 3s");
	chThdSleepMilliseconds(3000);

	goto start;
}
#endif



#if HAL_USBH_USE_MSD
#include "usbh/dev/msd.h"
#include "ff.h"

static FATFS MSDLUN0FS;
static uint8_t fbuff[10240];
static FIL file;

static FRESULT scan_files(BaseSequentialStream *chp, char *path) {
  FRESULT res;
  FILINFO fno;
  DIR dir;
  int i;
  char *fn;

#if _USE_LFN
  fno.lfname = 0;
  fno.lfsize = 0;
#endif
  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    i = strlen(path);
    for (;;) {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0)
        break;
      if (fno.fname[0] == '.')
        continue;
      fn = fno.fname;
      if (fno.fattrib & AM_DIR) {
        path[i++] = '/';
        strcpy(&path[i], fn);
        res = scan_files(chp, path);
        if (res != FR_OK)
          break;
        path[--i] = 0;
      } else {
    	  usbDbgPrintf("FS: %s/%s", path, fn);
      }
    }
  }
  return res;
}
static THD_WORKING_AREA(waTestMSD, 1024);
static void ThreadTestMSD(void *p) {
	(void)p;

	FATFS *fsp;
	uint32_t clusters;
	FRESULT res;
	BaseSequentialStream * const chp = (BaseSequentialStream *)&USBH_DEBUG_SD;
	blkstate_t state;
	systime_t st, et;
	uint32_t j;

start:
	for(;;) {
		chThdSleepMilliseconds(100);

		chSysLock();
		state = blkGetDriverState(&MSBLKD[0]);
		chSysUnlock();
		if (state != BLK_READY)
			continue;

		//raw read test
		if (1) {
#define RAW_READ_SZ_MB		1
#define NBLOCKS				(sizeof(fbuff) / 512)
#define NITERATIONS			((RAW_READ_SZ_MB * 1024UL * 1024UL) / sizeof(fbuff))
			uint32_t start = 0;
			chThdSetPriority(HIGHPRIO);
			usbDbgPrintf("BLK: Raw read test (%dMB, %dB blocks)", RAW_READ_SZ_MB, sizeof(fbuff));
			st = chVTGetSystemTime();
			for (j = 0; j < NITERATIONS; j++) {
				blkRead(&MSBLKD[0], start, fbuff, NBLOCKS);
				start += NBLOCKS;
			}
			et = chVTGetSystemTime();
			usbDbgPrintf("BLK: Raw read in %d ms, %dkB/s",
					et - st,
					(RAW_READ_SZ_MB * 1024UL * 1000) / (et - st));
			chThdSetPriority(NORMALPRIO);
		}

		usbDbgPuts("FS: Block driver ready, try mount...");

		res = f_mount(&MSDLUN0FS, "0:", 1);
		if (res != FR_OK) {
			usbDbgPuts("FS: Can't mount. Check file system.");
			continue;
		}
		usbDbgPuts("FS: Mounted.");

		res = f_getfree("0:", &clusters, &fsp);
		if (res != FR_OK) {
			usbDbgPuts("FS: f_getfree() failed");
			continue;
		}

		usbDbgPrintf("FS: %lu free clusters, %lu sectors per cluster, %lu bytes free",
				clusters, (uint32_t)MSDLUN0FS.csize,
				clusters * (uint32_t)MSDLUN0FS.csize * MSBLKD[0].info.blk_size);

		break;
	}

	//FATFS test
	if (1) {
		UINT bw;
		const uint8_t *src;
		const uint8_t *const start = (uint8_t *)0x08000000;
		const uint8_t *const top = (uint8_t *)0x08020000;

		//write test
		if (1) {
			usbDbgPuts("FS: Write test (create file /test.dat, 1MB)");
			f_open(&file, "/test.dat", FA_CREATE_ALWAYS | FA_WRITE);
			src = start;
			st = chVTGetSystemTime();
			for (j = 0; j < 2048; j++) {
				if (f_write(&file, src, 512, &bw) != FR_OK)
					goto start;
				src += bw;
				if (src >= top)
					src = start;
			}
			et = chVTGetSystemTime();
			usbDbgPrintf("FS: Written 1MB in %d ms, %dkB/s",
					et - st,
					(1024UL*1000) / (et - st));
			f_close(&file);
		}

		//read test
		if (1) {
			usbDbgPuts("FS: Read test (read file /test.dat, 1MB, compare)");
			f_open(&file, "/test.dat", FA_READ);
			src = start;
			st = chVTGetSystemTime();
			for (j = 0; j < 2048; j++) {
				if (f_read(&file, fbuff, 512, &bw) != FR_OK)
					goto start;
				if (memcmp(src, fbuff, bw)) {
					usbDbgPrintf("Compare error @%08x", (uint32_t)src);
					goto start;
				}
				src += bw;
				if (src >= top)
					src = start;
			}
			et = chVTGetSystemTime();
			usbDbgPrintf("FS: Read 1MB in %d ms, %dkB/s",
					et - st,
					(1024UL*1000) / (et - st));
			f_close(&file);
		}

		//scan files test
		if (1) {
			usbDbgPuts("FS: Scan files test");
			fbuff[0] = 0;
			scan_files(chp, (char *)fbuff);
		}
	}

	usbDbgPuts("FS: Tests done, restarting in 3s");
	chThdSleepMilliseconds(3000);

	goto start;

}
#endif






int main(void) {

	halInit();
	usbhInit();
	chSysInit();

	//PA2(TX) and PA3(RX) are routed to USART2
	sdStart(&SD2, NULL);
	palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));

#if STM32_USBH_USE_OTG1
	//VBUS - configured in board.h
	//USB_FS - configured in board.h
#endif

#if STM32_USBH_USE_OTG2
	//USB_HS
	//TODO: Initialize Pads
#endif

#if HAL_USBH_USE_MSD
	usbhmsdObjectInit(&USBHMSD[0]);
	usbhmsdLUNObjectInit(&MSBLKD[0]);
	chThdCreateStatic(waTestMSD, sizeof(waTestMSD), NORMALPRIO, ThreadTestMSD, 0);
#endif
#if HAL_USBH_USE_FTDI
	usbhftdiObjectInit(&USBHFTDID[0]);
	usbhftdipObjectInit(&FTDIPD[0]);
	chThdCreateStatic(waTestFTDI, sizeof(waTestFTDI), NORMALPRIO, ThreadTestFTDI, 0);
#endif

	//turn on USB power
	palClearPad(GPIOC, GPIOC_OTG_FS_POWER_ON);

	//start
#if STM32_USBH_USE_OTG1
	usbhStart(&USBHD1);
#endif
#if STM32_USBH_USE_OTG2
	usbhStart(&USBHD2);
#endif

	for(;;) {
#if STM32_USBH_USE_OTG1
		usbhMainLoop(&USBHD1);
#endif
#if STM32_USBH_USE_OTG2
		usbhMainLoop(&USBHD2);
#endif
		chThdSleepMilliseconds(100);
	}
}

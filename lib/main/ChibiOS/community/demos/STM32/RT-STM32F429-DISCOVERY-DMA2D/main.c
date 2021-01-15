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

#include "hal_fsmc_sdram.h"
#include "ili9341.h"
#include "hal_stm32_ltdc.h"
#include "hal_stm32_dma2d.h"

#include "res/wolf3d_vgagraph_chunk87.h"

/*===========================================================================*/
/* SDRAM related.                                                            */
/*===========================================================================*/

// TODO: Move constants below elsewhere, and normalize their name

/* SDRAM bank base address.*/
#define SDRAM_BANK_ADDR     ((uint32_t)0xD0000000)

/*
 *  FMC SDRAM Mode definition register defines
 */
#define FMC_SDCMR_MRD_BURST_LENGTH_1             ((uint16_t)0x0000)
#define FMC_SDCMR_MRD_BURST_LENGTH_2             ((uint16_t)0x0001)
#define FMC_SDCMR_MRD_BURST_LENGTH_4             ((uint16_t)0x0002)
#define FMC_SDCMR_MRD_BURST_LENGTH_8             ((uint16_t)0x0004)
#define FMC_SDCMR_MRD_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define FMC_SDCMR_MRD_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define FMC_SDCMR_MRD_CAS_LATENCY_2              ((uint16_t)0x0020)
#define FMC_SDCMR_MRD_CAS_LATENCY_3              ((uint16_t)0x0030)
#define FMC_SDCMR_MRD_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define FMC_SDCMR_MRD_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define FMC_SDCMR_MRD_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

/*
 * FMC_ReadPipe_Delay
 */
#define FMC_ReadPipe_Delay_0               ((uint32_t)0x00000000)
#define FMC_ReadPipe_Delay_1               ((uint32_t)0x00002000)
#define FMC_ReadPipe_Delay_2               ((uint32_t)0x00004000)
#define FMC_ReadPipe_Delay_Mask            ((uint32_t)0x00006000)

/*
 * FMC_Read_Burst
 */
#define FMC_Read_Burst_Disable             ((uint32_t)0x00000000)
#define FMC_Read_Burst_Enable              ((uint32_t)0x00001000)
#define FMC_Read_Burst_Mask                ((uint32_t)0x00001000)

/*
 * FMC_SDClock_Period
 */
#define FMC_SDClock_Disable                ((uint32_t)0x00000000)
#define FMC_SDClock_Period_2               ((uint32_t)0x00000800)
#define FMC_SDClock_Period_3               ((uint32_t)0x00000C00)
#define FMC_SDClock_Period_Mask            ((uint32_t)0x00000C00)

/*
 * FMC_ColumnBits_Number
 */
#define FMC_ColumnBits_Number_8b           ((uint32_t)0x00000000)
#define FMC_ColumnBits_Number_9b           ((uint32_t)0x00000001)
#define FMC_ColumnBits_Number_10b          ((uint32_t)0x00000002)
#define FMC_ColumnBits_Number_11b          ((uint32_t)0x00000003)

/*
 * FMC_RowBits_Number
 */
#define FMC_RowBits_Number_11b             ((uint32_t)0x00000000)
#define FMC_RowBits_Number_12b             ((uint32_t)0x00000004)
#define FMC_RowBits_Number_13b             ((uint32_t)0x00000008)

/*
 * FMC_SDMemory_Data_Width
 */
#define FMC_SDMemory_Width_8b                ((uint32_t)0x00000000)
#define FMC_SDMemory_Width_16b               ((uint32_t)0x00000010)
#define FMC_SDMemory_Width_32b               ((uint32_t)0x00000020)

/*
 * FMC_InternalBank_Number
 */
#define FMC_InternalBank_Number_2          ((uint32_t)0x00000000)
#define FMC_InternalBank_Number_4          ((uint32_t)0x00000040)

/*
 * FMC_CAS_Latency
 */
#define FMC_CAS_Latency_1                  ((uint32_t)0x00000080)
#define FMC_CAS_Latency_2                  ((uint32_t)0x00000100)
#define FMC_CAS_Latency_3                  ((uint32_t)0x00000180)

/*
 * FMC_Write_Protection
 */
#define FMC_Write_Protection_Disable       ((uint32_t)0x00000000)
#define FMC_Write_Protection_Enable        ((uint32_t)0x00000200)

/*
 * SDRAM driver configuration structure.
 */
static const SDRAMConfig sdram_cfg = {
  .sdcr = (uint32_t)(FMC_ColumnBits_Number_8b |
                     FMC_RowBits_Number_12b |
                     FMC_SDMemory_Width_16b |
                     FMC_InternalBank_Number_4 |
                     FMC_CAS_Latency_3 |
                     FMC_Write_Protection_Disable |
                     FMC_SDClock_Period_2 |
                     FMC_Read_Burst_Disable |
                     FMC_ReadPipe_Delay_1),

  .sdtr = (uint32_t)((2   - 1) |  // FMC_LoadToActiveDelay = 2 (TMRD: 2 Clock cycles)
                     (7 <<  4) |  // FMC_ExitSelfRefreshDelay = 7 (TXSR: min=70ns (7x11.11ns))
                     (4 <<  8) |  // FMC_SelfRefreshTime = 4 (TRAS: min=42ns (4x11.11ns) max=120k (ns))
                     (7 << 12) |  // FMC_RowCycleDelay = 7 (TRC:  min=70 (7x11.11ns))
                     (2 << 16) |  // FMC_WriteRecoveryTime = 2 (TWR:  min=1+ 7ns (1+1x11.11ns))
                     (2 << 20) |  // FMC_RPDelay = 2 (TRP:  20ns => 2x11.11ns)
                     (2 << 24)),  // FMC_RCDDelay = 2 (TRCD: 20ns => 2x11.11ns)

  .sdcmr = (uint32_t)(((4 - 1) << 5) |
                      ((FMC_SDCMR_MRD_BURST_LENGTH_2 |
                        FMC_SDCMR_MRD_BURST_TYPE_SEQUENTIAL |
                        FMC_SDCMR_MRD_CAS_LATENCY_3 |
                        FMC_SDCMR_MRD_OPERATING_MODE_STANDARD |
                        FMC_SDCMR_MRD_WRITEBURST_MODE_SINGLE) << 9)),

  /* if (STM32_SYSCLK == 180000000) ->
     64ms / 4096 = 15.625us
     15.625us * 90MHz = 1406 - 20 = 1386 */
  //.sdrtr = (1386 << 1),
  .sdrtr = (uint32_t)(683 << 1),
};

/* SDRAM size, in bytes.*/
#define IS42S16400J_SIZE             (8 * 1024 * 1024)

/*
 * Erases the whole SDRAM bank.
 */
static void sdram_bulk_erase(void) {

  volatile uint8_t *p = (volatile uint8_t *)SDRAM_BANK_ADDR;
  volatile uint8_t *end = p + IS42S16400J_SIZE;
  while (p < end)
    *p++ = 0;
}

/*
 * Red LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker1");
  while (true) {
    palClearPad(GPIOG, GPIOG_LED4_RED);
    chThdSleepMilliseconds(500);
    palSetPad(GPIOG, GPIOG_LED4_RED);
    chThdSleepMilliseconds(500);
  }
}

/*
 * Green LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread2, 128);
static THD_FUNCTION(Thread2, arg) {

  (void)arg;
  chRegSetThreadName("blinker2");
  while (true) {
    palClearPad(GPIOG, GPIOG_LED3_GREEN);
    chThdSleepMilliseconds(250);
    palSetPad(GPIOG, GPIOG_LED3_GREEN);
    chThdSleepMilliseconds(250);
  }
}

/*===========================================================================*/
/* LTDC related.                                                             */
/*===========================================================================*/

static uint8_t frame_buffer[240 * 320 * 3] __attribute__((section(".ram7")));

static uint8_t view_buffer[240 * 320];

extern const ltdc_color_t wolf3d_palette[256];

static const ltdc_window_t ltdc_fullscreen_wincfg = {
  0,
  240 - 1,
  0,
  320 - 1,
};

static const ltdc_frame_t ltdc_view_frmcfg1 = {
  view_buffer,
  240,
  320,
  240 * sizeof(uint8_t),
  LTDC_FMT_L8,
};

static const ltdc_laycfg_t ltdc_view_laycfg1 = {
  &ltdc_view_frmcfg1,
  &ltdc_fullscreen_wincfg,
  LTDC_COLOR_FUCHSIA,
  0xFF,
  0x980088,
  wolf3d_palette,
  256,
  LTDC_BLEND_FIX1_FIX2,
  (LTDC_LEF_ENABLE | LTDC_LEF_PALETTE),
};

static const ltdc_frame_t ltdc_screen_frmcfg1 = {
  frame_buffer,
  240,
  320,
  240 * 3,
  LTDC_FMT_RGB888,
};

static const ltdc_laycfg_t ltdc_screen_laycfg1 = {
  &ltdc_screen_frmcfg1,
  &ltdc_fullscreen_wincfg,
  LTDC_COLOR_FUCHSIA,
  0xFF,
  0x980088,
  NULL,
  0,
  LTDC_BLEND_FIX1_FIX2,
  LTDC_LEF_ENABLE,
};

static const LTDCConfig ltdc_cfg = {
  /* Display specifications.*/
  240,                              /**< Screen pixel width.*/
  320,                              /**< Screen pixel height.*/
  10,                               /**< Horizontal sync pixel width.*/
  2,                                /**< Vertical sync pixel height.*/
  20,                               /**< Horizontal back porch pixel width.*/
  2,                                /**< Vertical back porch pixel height.*/
  10,                               /**< Horizontal front porch pixel width.*/
  4,                                /**< Vertical front porch pixel height.*/
  0,                                /**< Driver configuration flags.*/

  /* ISR callbacks.*/
  NULL,                             /**< Line Interrupt ISR, or @p NULL.*/
  NULL,                             /**< Register Reload ISR, or @p NULL.*/
  NULL,                             /**< FIFO Underrun ISR, or @p NULL.*/
  NULL,                             /**< Transfer Error ISR, or @p NULL.*/

  /* Color and layer settings.*/
  LTDC_COLOR_TEAL,
  &ltdc_view_laycfg1,
  NULL,
};

extern LTDCDriver LTDCD1;

const SPIConfig spi_cfg5 = {
  false,
  NULL,
  GPIOC,
  GPIOC_SPI5_LCD_CS,
  (((1 << 3) & SPI_CR1_BR) | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR),
  0
};

extern SPIDriver SPID5;

const ILI9341Config ili9341_cfg = {
  &SPID5,
  GPIOD,
  GPIOD_LCD_WRX
};

static void initialize_lcd(void) {

  static const uint8_t pgamma[15] = {
    0x0F, 0x29, 0x24, 0x0C, 0x0E, 0x09, 0x4E, 0x78,
    0x3C, 0x09, 0x13, 0x05, 0x17, 0x11, 0x00
  };
  static const uint8_t ngamma[15] = {
    0x00, 0x16, 0x1B, 0x04, 0x11, 0x07, 0x31, 0x33,
    0x42, 0x05, 0x0C, 0x0A, 0x28, 0x2F, 0x0F
  };

  ILI9341Driver *const lcdp = &ILI9341D1;

  /* XOR-checkerboard texture.*/
  unsigned x, y;
  for (y = 0; y < 320; ++y)
    for (x = 0; x < 240; ++x)
      view_buffer[y * 240 + x] = (uint8_t)(x ^ y);

  ili9341AcquireBus(lcdp);
  ili9341Select(lcdp);

  ili9341WriteCommand(lcdp, ILI9341_SET_FRAME_CTL_NORMAL);
  ili9341WriteByte(lcdp, 0x00);
  ili9341WriteByte(lcdp, 0x1B);

  ili9341WriteCommand(lcdp, ILI9341_SET_FUNCTION_CTL);
  ili9341WriteByte(lcdp, 0x0A);
  ili9341WriteByte(lcdp, 0xA2);

  ili9341WriteCommand(lcdp, ILI9341_SET_POWER_CTL_1);
  ili9341WriteByte(lcdp, 0x10);

  ili9341WriteCommand(lcdp, ILI9341_SET_POWER_CTL_2);
  ili9341WriteByte(lcdp, 0x10);

  ili9341WriteCommand(lcdp, ILI9341_SET_VCOM_CTL_1);
  ili9341WriteByte(lcdp, 0x45);
  ili9341WriteByte(lcdp, 0x15);

  ili9341WriteCommand(lcdp, ILI9341_SET_VCOM_CTL_2);
  ili9341WriteByte(lcdp, 0x90);

  ili9341WriteCommand(lcdp, ILI9341_SET_MEM_ACS_CTL);
  ili9341WriteByte(lcdp, 0xC8);

  ili9341WriteCommand(lcdp, ILI9341_SET_RGB_IF_SIG_CTL);
  ili9341WriteByte(lcdp, 0xC2);

  ili9341WriteCommand(lcdp, ILI9341_SET_FUNCTION_CTL);
  ili9341WriteByte(lcdp, 0x0A);
  ili9341WriteByte(lcdp, 0xA7);
  ili9341WriteByte(lcdp, 0x27);
  ili9341WriteByte(lcdp, 0x04);

  ili9341WriteCommand(lcdp, ILI9341_SET_COL_ADDR);
  ili9341WriteByte(lcdp, 0x00);
  ili9341WriteByte(lcdp, 0x00);
  ili9341WriteByte(lcdp, 0x00);
  ili9341WriteByte(lcdp, 0xEF);

  ili9341WriteCommand(lcdp, ILI9341_SET_PAGE_ADDR);
  ili9341WriteByte(lcdp, 0x00);
  ili9341WriteByte(lcdp, 0x00);
  ili9341WriteByte(lcdp, 0x01);
  ili9341WriteByte(lcdp, 0x3F);

  ili9341WriteCommand(lcdp, ILI9341_SET_IF_CTL);
  ili9341WriteByte(lcdp, 0x01);
  ili9341WriteByte(lcdp, 0x00);
  ili9341WriteByte(lcdp, 0x06);

  ili9341WriteCommand(lcdp, ILI9341_SET_GAMMA);
  ili9341WriteByte(lcdp, 0x01);

  ili9341WriteCommand(lcdp, ILI9341_SET_PGAMMA);
  ili9341WriteChunk(lcdp, pgamma, 15);

  ili9341WriteCommand(lcdp, ILI9341_SET_NGAMMA);
  ili9341WriteChunk(lcdp, ngamma, 15);

  ili9341WriteCommand(lcdp, ILI9341_CMD_SLEEP_OFF);
  chThdSleepMilliseconds(10);

  ili9341WriteCommand(lcdp, ILI9341_CMD_DISPLAY_ON);
  ili9341WriteCommand(lcdp, ILI9341_SET_MEM);
  chThdSleepMilliseconds(10);

  ili9341Unselect(lcdp);
  ili9341ReleaseBus(lcdp);
}

static const DMA2DConfig dma2d_cfg = {
  /* ISR callbacks.*/
  NULL,     /**< Configuration error, or @p NULL.*/
  NULL,     /**< Palette transfer done, or @p NULL.*/
  NULL,     /**< Palette access error, or @p NULL.*/
  NULL,     /**< Transfer watermark, or @p NULL.*/
  NULL,     /**< Transfer complete, or @p NULL.*/
  NULL      /**< Transfer error, or @p NULL.*/
};

static const dma2d_palcfg_t dma2d_palcfg = {
  wolf3d_palette,
  256,
  DMA2D_FMT_ARGB8888
};

static const dma2d_laycfg_t dma2d_bg_laycfg = {
  view_buffer,
  0,
  DMA2D_FMT_L8,
  DMA2D_COLOR_RED,
  0xFF,
  &dma2d_palcfg
};

static const dma2d_laycfg_t dma2d_fg_laycfg = {
  (void *)wolf3d_vgagraph_chunk87,
  0,
  DMA2D_FMT_L8,
  DMA2D_COLOR_LIME,
  0xFF,
  &dma2d_palcfg
};

static const dma2d_laycfg_t dma2d_frame_laycfg = {
  frame_buffer,
  0,
  DMA2D_FMT_RGB888,
  DMA2D_COLOR_BLUE,
  0xFF,
  NULL
};

static void dma2d_test(void) {

  DMA2DDriver *const dma2dp = &DMA2DD1;
  LTDCDriver *const ltdcp = &LTDCD1;

  chThdSleepSeconds(1);

  ltdcBgSetConfig(ltdcp, &ltdc_screen_laycfg1);
  ltdcReload(ltdcp, TRUE);

  dma2dAcquireBus(dma2dp);

  /* Target the frame buffer by default.*/
  dma2dBgSetConfig(dma2dp, &dma2d_frame_laycfg);
  dma2dFgSetConfig(dma2dp, &dma2d_frame_laycfg);
  dma2dOutSetConfig(dma2dp, &dma2d_frame_laycfg);

  /* Copy the background.*/
  dma2dFgSetConfig(dma2dp, &dma2d_bg_laycfg);
  dma2dJobSetMode(dma2dp, DMA2D_JOB_CONVERT);
  dma2dJobSetSize(dma2dp, 240, 320);
  dma2dJobExecute(dma2dp);

  /* Draw the splashscren picture at (8, 0).*/
  dma2dFgSetConfig(dma2dp, &dma2d_fg_laycfg);
  dma2dOutSetAddress(dma2dp, dma2dComputeAddress(
    frame_buffer, ltdc_screen_frmcfg1.pitch, DMA2D_FMT_RGB888, 8, 0
  ));
  dma2dOutSetWrapOffset(dma2dp, ltdc_screen_frmcfg1.width - 200);
  dma2dJobSetMode(dma2dp, DMA2D_JOB_CONVERT);
  dma2dJobSetSize(dma2dp, 200, 320);
  dma2dJobExecute(dma2dp);

  dma2dReleaseBus(dma2dp);
}

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/


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

static const ShellCommand commands[] = {
  {"reset", cmd_reset},
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
#if (HAL_USE_SERIAL_USB == TRUE)
  (BaseSequentialStream *)&SDU2,
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

  /*
   * Shell manager initialization.
   */
  shellInit();

#if (HAL_USE_SERIAL_USB == TRUE)
  /*
   * Initializes a serial-over-USB CDC driver.
   */
  sduObjectInit(&SDU2);
  sduStart(&SDU2, &serusbcfg);

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
   * Initialise FSMC for SDRAM.
   */
  fsmcSdramInit();
  fsmcSdramStart(&SDRAMD, &sdram_cfg);
  sdram_bulk_erase();

  /*
   * Activates the LCD-related drivers.
   */
  spiStart(&SPID5, &spi_cfg5);
  ili9341ObjectInit(&ILI9341D1);
  ili9341Start(&ILI9341D1, &ili9341_cfg);
  initialize_lcd();
  ltdcInit();
  ltdcStart(&LTDCD1, &ltdc_cfg);

  /*
   * Activates the DMA2D-related drivers.
   */
  dma2dInit();
  dma2dStart(&DMA2DD1, &dma2d_cfg);
  dma2d_test();

  /*
   * Creating the blinker threads.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1),
                    NORMALPRIO + 10, Thread1, NULL);
  chThdCreateStatic(waThread2, sizeof(waThread2),
                    NORMALPRIO + 10, Thread2, NULL);

  /*
   * Normal main() thread activity, in this demo it just performs
   * a shell respawn upon its termination.
   */
  while (true) {
    if (!shelltp) {
#if (HAL_USE_SERIAL_USB == TRUE)
      if (SDU2.config->usbp->state == USB_ACTIVE) {
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
}

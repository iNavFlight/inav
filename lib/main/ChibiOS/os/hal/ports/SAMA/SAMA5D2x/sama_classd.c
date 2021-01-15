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
 * @file    SAMA5D2x/sama_classd.c
 * @brief   SAMA CLASSD support code.
 *
 * @addtogroup SAMA5D2x_CLASSD
 * @{
 */

#include "hal.h"

#if SAMA_USE_CLASSD || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local macros.                                                      */
/*===========================================================================*/

/**
 * @brief   Enable write protection on CLASSD Mode Register
 *          and Interpolator Mode Register.
 *
 * @param[in] classdp   pointer to a CLASSD register block
 *
 * @notapi
 */
#define classdEnableWP(classdp) {                                              \
  classdp->CLASSD_WPMR = CLASSD_WPMR_WPKEY_PASSWD | CLASSD_WPMR_WPEN;          \
}

/**
 * @brief   Disable write protection on CLASSD Mode Register
 *          and Interpolator Mode Register.
 *
 * @param[in] classdp   pointer to a CLASSD register block
 *
 * @notapi
 */
#define classdDisableWP(classdp) {                                             \
  classdp->CLASSD_WPMR = CLASSD_WPMR_WPKEY_PASSWD;                             \
}

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/
/**
 * @brief   CLASSD driver identifier.
 */
CLASSDDriver CLASSDD0;

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/
/**
 * @brief   Type of a structure representing PMC Audio configuration.
 */
typedef struct {
  /**
   * @brief   Loop Divider Ratio.
   */
  uint32_t nd;

  /**
   * @brief   Fractional Loop Divider Setting.
   */
  uint32_t fracr;

  /**
   * @brief   Output Divider Ratio for PMC Clock.
   */
  uint32_t qdpmc;

  /**
   * @brief   Divider Value.
   */
  uint32_t div;

  /**
   * @brief   Output Divider Ratio for Pad Clock.
   */
  uint32_t qdaudio;
} pmcAudioConf;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Configure DSPClock.
 *
 * @param[in] dsp_clk   DSP clock type (12.288 MHz or 11.2896 MHz).
 */
static void dspclkConfigure(uint32_t dsp_clk) {
  pmcAudioConf cfg;

  /* Pad Clock: not used */
  cfg.div = 0;
  cfg.qdaudio = 0;

  /* PMC Clock: */
  /* 12Mhz * (ND + 1 + FRACR/2^22) / (QDPMC + 1) = 8 * DSPCLK */
  switch (dsp_clk) {
    case CLASSD_INTPMR_DSPCLKFREQ_12M288:
#if SAMA_MOSCXTCLK == 12000000
      /* 12Mhz * (56 + 1 + 1442841/2^22) / (6 + 1) = 8 * 12.288Mhz */
      cfg.nd = 56;
      cfg.fracr = 1442841;
      cfg.qdpmc = 6;
#elif SAMA_MOSCXTCLK == 24000000
      /* 24Mhz * (56 + 1 + 1442841/2^22) / (6 + 1) = 8 * 12.288Mhz */
      cfg.nd = 27;
      cfg.fracr = 2796203;
      cfg.qdpmc = 6;
#else
      #error "FREQUENCY NOT SUPPORTED BY CLASSD"
#endif
    break;

    case CLASSD_INTPMR_DSPCLKFREQ_11M2896:
#if SAMA_MOSCXTCLK == 12000000
      /* 12Mhz * (59 + 1 + 885837/2^22) / (7 + 1) = 8 * 11.2896Mhz */
      cfg.nd = 59;
      cfg.fracr = 885837;
      cfg.qdpmc = 7;
#elif SAMA_MOSCXTCLK == 24000000
      /* 24Mhz * (59 + 1 + 885837/2^22) / (7 + 1) = 8 * 11.2896Mhz */
      cfg.nd = 28;
      cfg.fracr = 699050;
      cfg.qdpmc = 7;
#else
      #error "FREQUENCY NOT SUPPORTED BY CLASSD"
#endif
    break;

    default:
      osalDbgAssert(NULL, "errore mask configuration");
   }

  /* Configure and enable the generic clock. */
  pmcConfigAudio(cfg.nd, cfg.qdpmc, cfg.fracr, cfg.div, cfg.qdaudio);
  pmcEnableAudio(true, false);
}

/**
 * @brief   Shared end-of-tx service routine.
 *
 * @param[in] classdp   pointer to the @p CLASSDDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void classd_lld_serve_tx_interrupt(CLASSDDriver *classdp, uint32_t flags) {

  /* DMA errors handling.*/
#if defined(SAMA_CLASSD_DMA_ERROR_HOOK)
  (void)classdp;
  if ((flags & (XDMAC_CIS_WBEIS | XDMAC_CIS_ROIS)) != 0) {
    SAMA_CLASSD_DMA_ERROR_HOOK(classdp);
  }
#else
  (void)flags;
#endif

  if(classdp->config->callback != NULL) {
    classdp->config->callback(classdp);
  }
  classdMuteChannel(classdp, false, false);
  classdp->state = CLASSD_READY;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level CLASSD driver initialization.
 *
 * @notapi
 */
void classd_lld_init(void) {
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_CLASSD, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */
  /* Driver initialization. */
  classdObjectInit(&CLASSDD0);
  CLASSDD0.classd = CLASSD;
  CLASSDD0.dmatx     = NULL;
  CLASSDD0.txdmamode = XDMAC_CC_TYPE_PER_TRAN |
                       XDMAC_CC_MBSIZE_SINGLE |
                       XDMAC_CC_DSYNC_MEM2PER |
                       XDMAC_CC_PROT_SEC |
                       XDMAC_CC_CSIZE_CHK_1 |
                       XDMAC_CC_DWIDTH_WORD |
                       XDMAC_CC_SIF_AHB_IF0 |
                       XDMAC_CC_DIF_AHB_IF1 |
                       XDMAC_CC_SAM_INCREMENTED_AM |
                       XDMAC_CC_DAM_FIXED_AM |
                       XDMAC_CC_PERID(PERID_CLASSD_TX);
}

/**
 * @brief   Configures and activates the CLASSD peripheral.
 *
 * @param[in] classdp   pointer to the @p CLASSDDriver object
 *
 * @notapi
 */
void classd_lld_start(CLASSDDriver *classdp) {

  uint8_t i;
  uint32_t dsp_clk_set, frame_set;

 /* Configures the peripheral.*/
  if (classdp->state == CLASSD_STOP) {

    if (&CLASSDD0 == classdp) {
      classdp->dmatx = dmaChannelAllocate(SAMA_CLASSD_DMA_IRQ_PRIORITY,
                                       (sama_dmaisr_t)classd_lld_serve_tx_interrupt,
                                       (void *)classdp);
      osalDbgAssert(classdp->dmatx != NULL, "no channel allocated");
    }
  }

 /* Set DMA channel mode. */
  dmaChannelSetMode(classdp->dmatx, classdp->txdmamode);

  /* Set CLASSD DSP clock and Sample rate. */
  for(i = 0; i < 8; i++) {
    if ((audio_info[i].rate) == (classdp->config->frame)) {
      dsp_clk_set  = audio_info[i].dsp_clk;
      frame_set = audio_info[i].sample_rate;
      break;
    }
  }

  /* Enable the CLASSD0 clock. */
  pmcEnableCLASSD0();

  /* Configure PMC Audio structure. */
  dspclkConfigure(dsp_clk_set);

  /* Disable the CLASSD generic clock for now. */
  pmcDisableGclkCLASSD0();

  /* Configure the CLASSD generic clock */
  pmcConfigGclk(ID_CLASSD, PMC_PCR_GCKCSS_AUDIO_CLK, 1);

  /* Disable write protection. */
  classdDisableWP(classdp->classd);

  /* Perform soft reset. */
  CLASSD->CLASSD_CR  = CLASSD_CR_SWRST;
  CLASSD->CLASSD_IDR = CLASSD_IDR_DATRDY;

  /* Clean CLASSD Registers. */
  classdp->classd->CLASSD_MR = 0;
  classdp->classd->CLASSD_INTPMR = 0;

  /* CLASSD configuration. */
  classdp->classd->CLASSD_MR = classdp->config->left |
                               classdp->config->right |
                               classdp->config->left_mute |
                               classdp->config->right_mute |
                               classdp->config->pwm_mode |
                               classdp->config->non_overlap |
                               classdp->config->novrval;

  classdp->classd->CLASSD_INTPMR = classdp->config->attl |
                                   classdp->config->attr |
                                   classdp->config->deemp |
                                   classdp->config->swap |
                                   classdp->config->eqcfg |
                                   classdp->config->mono |
                                   classdp->config->mono_mode |
                                   dsp_clk_set | frame_set;

  /* Enable CLASSD generic clock. */
  pmcEnableGclkCLASSD0();

  /* Enable write protection. */
  classdEnableWP(classdp->classd);
}

/**
 * @brief   Deactivates the CLASSD peripheral.
 *
 * @param[in] classdp   pointer to the @p CLASSDDriver object
 *
 * @notapi
 */
void classd_lld_stop(CLASSDDriver *classdp) {

  /* Disable clocks. */
  pmcDisableAudio();
  pmcDisableGclkCLASSD0();
  pmcDisableCLASSD0();

  /* Disable write protection. */
  classdDisableWP(classdp->classd);

  /* Reset CLASSD. */
  classdp->classd->CLASSD_INTPMR = 0;
  classdp->classd->CLASSD_MR = 0;

  /* Enable write protection. */
  classdEnableWP(classdp->classd);

  /* Release and disable DMA channel. */
  dmaChannelRelease(classdp->dmatx);
}

/**
 *
 * @brief   Starts a CLASSD playback.
 *
 * @param[in] classdp   pointer to the @p CLASSDDriver
 * @param[in] txbuf     the pointer to the transmit buffer
 *
 * @notapi
 */
void classd_lld_send_audio(CLASSDDriver *classdp, const void *txbuf) {

  /* Get DMA transfert size. */
  size_t n = ((struct wav_header *)txbuf)->subchunk2_size / 4;

  osalDbgAssert(!((uint32_t) txbuf & (L1_CACHE_BYTES - 1)), "address not cache aligned");

#if 0
  osalDbgAssert(!(n & (L1_CACHE_BYTES - 1)), "size not multiple of cache line");
#endif

  /* L1 is enabled */
  cacheCleanRegion((uint8_t *) txbuf, n);

  /* Get source address. */
  uint32_t addrSource = sizeof(struct wav_header);

  /* Unmute left and right channel */
  classdMuteChannel(classdp, false, false);

  /* Writing channel */
  dmaChannelSetSource(classdp->dmatx, txbuf + addrSource);
  dmaChannelSetDestination(classdp->dmatx, &classdp->classd->CLASSD_THR);
  dmaChannelSetTransactionSize(classdp->dmatx, n);

  /* DMA start transfer. */
  dmaChannelEnable(classdp->dmatx);
}

/**
 * @brief   CLASSD mute/unmute channels.
 *
 * @param[in] classdp   pointer to the @p CLASSDDriver object
 *
 * @iclass
 */
void classd_mute_channel(CLASSDDriver *classdp, bool left, bool right) {

  /* Disable write protection. */
  classdDisableWP(classdp->classd);

  /* Mute or unmute left channel. */
  if (left)
    classdp->classd->CLASSD_MR |= CLASSD_MR_LMUTE;
  else
    classdp->classd->CLASSD_MR &= ~CLASSD_MR_LMUTE;

  /* Mute or unmute right channel. */
  if (right)
    classdp->classd->CLASSD_MR |= CLASSD_MR_RMUTE;
  else
    classdp->classd->CLASSD_MR &= ~CLASSD_MR_RMUTE;

  /* Enable write protection. */
  classdEnableWP(classdp->classd);
}

/**
 *
 * @brief   CLASSD Driver initialization.
 *
 * @init
 */
void classdInit(void) {

  classd_lld_init();
}

/**
 *
 * @brief   Initializes the standard part of a @p CLASSDDriver structure.
 *
 * @param[out] classdp  pointer to a @p CLASSDDriver object
 *
 * @init
 */
void classdObjectInit(CLASSDDriver *classdp) {
  classdp->state = CLASSD_STOP;
  classdp->config = NULL;
}

/**
 * @brief   Configures and activates the CLASSD peripheral.
 *
 * @param[in] classdp  pointer to a @p CLASSDDriver object
 * @param[in] config   pointer to a @p CLASSDConfig object
 *
 * @api
 */
void classdStart(CLASSDDriver *classdp, const CLASSDConfig *config) {

  osalDbgCheck((classdp != NULL) && (config != NULL));

  osalSysLock();
  osalDbgAssert((classdp->state == CLASSD_STOP) || (classdp->state == CLASSD_READY),
                "invalid state");
  classdp->config = config;
  classd_lld_start(classdp);
  classdp->state = CLASSD_READY;
  osalSysUnlock();
}

/**
 * @brief   Deactivates the CLASSD peripheral.
 *
 * @param[in] classdp   pointer to the @p CLASSDDriver object
 *
 * @api
 */
void classdStop(CLASSDDriver *classdp) {

  osalDbgCheck(classdp != NULL);

  osalSysLock();

  osalDbgAssert((classdp->state == CLASSD_STOP) || (classdp->state == CLASSD_READY || (classdp->state == CLASSD_ACTIVE)),
                "invalid state");

  classd_lld_stop(classdp);
  classdp->config = NULL;
  classdp->state  = CLASSD_STOP;

  osalSysUnlock();
}

/**
 *
 * @brief  Starts a CLASSD playback.
 *
 * @param[in] classdp   pointer to the @p CLASSDDriver
 * @param[in] txbuf     the pointer to the transmit buffer
 *
 * @note    This function can be used only in syslock state. todo: control comment!
 *
 * @notapi
 */
void classdSendAudioI(CLASSDDriver *classdp, const void *txbuf) {

  (classdp)->state = CLASSD_ACTIVE;
  classd_lld_send_audio(classdp, txbuf);
}

/**
 *
 * @brief  Starts a CLASSD playback.
 *
 * @param[in] classdp   pointer to the @p CLASSDDriver
 * @param[in] txbuf     the pointer to the transmit buffer
 *
 * @notapi
 */
void classdSendAudio(CLASSDDriver *classdp, const void *txbuf) {

  osalSysLock();
  osalDbgAssert(classdp->state == CLASSD_READY, "not ready");
  classdSendAudioI(classdp, txbuf);
  osalSysUnlock();
}

/**
 *
 * @brief  Set the sample frame from the structure of a wav file.
 *
 * @param[in] classdconfigp  pointer to the @p CLASSDConfig
 * @param[in] music_file     pointer to the wav file
 *
 * @notapi
 */
void classdSetSampleFrame(CLASSDConfig *classdconfigp, uint8_t *music_file) {
  classdconfigp->frame = ((struct wav_header*)music_file)->sample_rate;
}

/**
 * @brief   Mute/unmute CLASSD channel.
 *
 * @param[in] classdp  pointer to the @p CLASSDDriver object
 *
 * @iclass
 */
void classdMuteChannel(CLASSDDriver *classdp, bool left, bool right) {
  classd_mute_channel(classdp, left, right);
}

#endif /* SAMA_USE_CLASSD */

/** @} */

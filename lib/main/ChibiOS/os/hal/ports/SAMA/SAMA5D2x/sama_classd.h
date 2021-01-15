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
 * @file    SAMA5D2x/sama_classd.h
 * @brief   SAMA CLASSD support macros and structures.
 *
 * @addtogroup SAMA5D2x_CLASSD
 * @{
 */

#ifndef SAMA_CLASSD_LLD_H
#define SAMA_CLASSD_LLD_H

/**
 * @brief   Using the CLASSD driver.
 */
#if !defined(SAMA_USE_CLASSD) || defined(__DOXYGEN__)
#define SAMA_USE_CLASSD                          FALSE
#endif

#if SAMA_USE_CLASSD || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */

/**
 * @brief   CLASSD DMA interrupt priority level setting.
 */
#if !defined(SAMA_CLASSD_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_CLASSD_DMA_IRQ_PRIORITY               4
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !defined(SAMA_DMA_REQUIRED)
#define SAMA_DMA_REQUIRED
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  CLASSD_UNINIT = 0,                /**< Not initialized.                   */
  CLASSD_STOP = 1,                  /**< Stopped.                           */
  CLASSD_READY = 2,                 /**< Ready.                             */
  CLASSD_ACTIVE = 3,                /**< Exchanging data.                   */
} classdstate_t;

/**
 * @brief   Structure representing audio info.
 */
static const struct {
  /**
   * @brief   Contains the value of the Sample Rate.
   */
  uint32_t rate;
  /**
   * @brief   Contains a mask of the Sample Rate.
   */
  uint32_t sample_rate;
  /**
   * @brief   Contains a mask of the DSP Clock.
   */
  uint32_t dsp_clk;
} audio_info[] = {
    {8000,  CLASSD_INTPMR_FRAME_FRAME_8K, CLASSD_INTPMR_DSPCLKFREQ_12M288},
    {16000, CLASSD_INTPMR_FRAME_FRAME_16K, CLASSD_INTPMR_DSPCLKFREQ_12M288},
    {32000, CLASSD_INTPMR_FRAME_FRAME_32K, CLASSD_INTPMR_DSPCLKFREQ_12M288},
    {48000, CLASSD_INTPMR_FRAME_FRAME_48K, CLASSD_INTPMR_DSPCLKFREQ_12M288},
    {96000, CLASSD_INTPMR_FRAME_FRAME_96K, CLASSD_INTPMR_DSPCLKFREQ_12M288},
    {22050, CLASSD_INTPMR_FRAME_FRAME_22K, CLASSD_INTPMR_DSPCLKFREQ_11M2896},
    {44100, CLASSD_INTPMR_FRAME_FRAME_44K, CLASSD_INTPMR_DSPCLKFREQ_11M2896},
    {88200, CLASSD_INTPMR_FRAME_FRAME_88K, CLASSD_INTPMR_DSPCLKFREQ_11M2896},
};

/**
 * @brief   Type of a structure representing Standard WAV file header information.
 */
struct wav_header {
  /**
   * @brief   Contains the letters "RIFF" in ASCII form.
   */
  uint32_t                  chunk_id;
  /**
   * @brief   Size of the rest of the chunk following this number.
   */
  uint32_t                  chunk_size;
  /**
   * @brief   Contains the letters "WAVE".
   */
  uint32_t                  format;
  /**
   * @brief   Contains the letters "fmt ".
   */
  uint32_t                  subchunk1_id;
  /**
   * @brief   16 for PCM.  This is the size of the rest of the Subchunk which follows this number.
   */
  uint32_t                  subchunk1_size;
  /**
   * @brief   PCM = 1 (i.e. Linear quantization). Values other than 1 indicate some form of compression.
   */
  uint16_t                  audio_format;
  /**
   * @brief   Mono = 1, Stereo = 2, etc.
   */
  uint16_t                  num_channels;
  /**
   * @brief   8000, 44100, etc.
   */
  uint32_t                  sample_rate;
  /**
   * @brief   SampleRate * NumChannels * BitsPerSample/8
   */
  uint32_t                  byte_rate;
  /**
   * @brief   NumChannels * BitsPerSample/8
   */
  uint16_t                  block_align;
  /**
   * @brief   8 bits = 8, 16 bits = 16, etc.
   */
  uint16_t                  bits_per_sample;
  /**
   * @brief   Contains the letters "data".
   */
  uint32_t                  subchunk2_id;
  /**
   * @brief   Number of bytes in the data.
   */
  uint32_t                  subchunk2_size;
};

/**
 * @brief   Type of a structure representing an CLASSD driver.
 */
typedef struct CLASSDDriver CLASSDDriver;

/**
  * @brief   Type of a generic CLASSD callback.
  */
typedef void (*classdcb_t)(CLASSDDriver *classdp);

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief   Callback pointer.
   */
  classdcb_t                callback;
  /**
   * @brief   Configuration of the CLASSD left channel.
   */
  uint32_t                  left;
  /**
   * @brief   Configuration of the CLASSD right channel.
   */
  uint32_t                  right;
  /**
   * @brief   Configuration of the CLASSD left channel mute.
   */
  uint32_t                  left_mute;
  /**
   * @brief   Configuration of the CLASSD right channel mute.
   */
  uint32_t                  right_mute;
  /**
   * @brief   Configuration of the CLASSD PWM modulation type.
   */
  uint32_t                  pwm_mode;
  /**
   * @brief   Configuration of the CLASSD Non-Overlapping.
   */
  uint32_t                  non_overlap;
  /**
   * @brief   Configuration of the CLASSD Non-Overlapping value.
   */
  uint32_t                  novrval;
  /**
   * @brief   Configuration of the CLASSD left channel attenuation.
   */
  uint32_t                  attl;
  /**
   * @brief   Configuration of the CLASSD right channel attenuation.
   */
  uint32_t                  attr;
  /**
   * @brief   Configuration of the CLASSD de-emphasis filter.
   */
  uint32_t                  deemp;
  /**
   * @brief   Configuration of the CLASSD swap left right channel.
   */
  uint32_t                  swap;
  /**
   * @brief   Configuration of the CLASSD sample frequency.
   */
  uint32_t                  frame;
  /**
   * @brief   Configuration of the CLASSD EQ config.
   */
  uint32_t                  eqcfg;
  /**
   * @brief   Configuration of the CLASSD mono signal.
   */
  uint32_t                  mono;
  /**
   * @brief   Configuration of the CLASSD mono mode.
   */
  uint32_t                  mono_mode;
} CLASSDConfig;

/**
 * @brief   Structure representing an CLASSD driver.
 */
struct CLASSDDriver {
  /**
   * @brief   Driver state.
   */
  classdstate_t             state;
  /**
   * @brief   Current configuration data.
   */
  const CLASSDConfig        *config;
  /**
   * @brief   Pointer to the WDT registers block.
   */
  Classd                    *classd;
  /**
   * @brief   Transmit DMA stream.
   */
  sama_dma_channel_t        *dmatx;
  /**
   * @brief   TX DMA mode bit mask.
   */
  uint32_t                  txdmamode;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/
extern CLASSDDriver CLASSDD0;

#ifdef __cplusplus
extern "C" {
#endif
  void classdInit(void);
  void classdObjectInit(CLASSDDriver *classdp);
  void classdStart(CLASSDDriver *classdp, const CLASSDConfig *config);
  void classdStop(CLASSDDriver *classdp);
  void classdSetSampleFrame(CLASSDConfig *classdconfigp, uint8_t *music_file);
  void classdSendAudioI(CLASSDDriver *classdp, const void *txbuf);
  void classdSendAudio(CLASSDDriver *classdp, const void *txbuf);
  void classdMuteChannel(CLASSDDriver *classdp, bool left, bool right);
#ifdef __cplusplus
}
#endif

#endif /* SAMA_USE_CLASSD */

#endif /* SAMA_CLASSD_LLD_H */

/** @} */

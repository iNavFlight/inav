/*
    ChibiOS - Copyright (C) 2016 Andrew Wygle aka awygle

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
 * @file    MSP430X/hal_dma_lld.c
 * @brief   MSP430X DMA subsystem low level driver header.
 * @note    This driver is used as a DMA engine for the other
 *          low level drivers.
 *
 * @addtogroup MSP430X_DMA
 * @{
 */

#ifndef HAL_MSP430X_DMA_H
#define HAL_MSP430X_DMA_H

#if (HAL_USE_DMA == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define MSP430X_DMA_SINGLE DMADT_0
#define MSP430X_DMA_BLOCK DMADT_1
#define MSP430X_DMA_BURST DMADT_2

#define MSP430X_DMA_SRCINCR DMASRCINCR_3
#define MSP430X_DMA_SRCDECR DMASRCINCR_2
#define MSP430X_DMA_DSTINCR DMADSTINCR_3
#define MSP430X_DMA_DSTDECR DMADSTINCR_2

#define MSP430X_DMA_SRCBYTE DMASRCBYTE
#define MSP430X_DMA_DSTBYTE DMADSTBYTE
#define MSP430X_DMA_SRCWORD 0
#define MSP430X_DMA_DSTWORD 0

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !defined(DMA_BASE) && !defined(MSP430X_DMA_SOFTWARE)
#error "The MSP430 device in use does not support DMA. Explicitly enable"
#error "software emulation by defining MSP430X_DMA_SOFTWARE."
#endif

#if defined(__MSP430_HAS_DMAX_1__) || defined(__MSP430X_HAS_DMA_1__)
#define MSP430X_DMA_CHANNELS 1
#elif defined(__MSP430_HAS_DMAX_3__) || defined(__MSP430X_HAS_DMA_3__)
#define MSP430X_DMA_CHANNELS 3
#elif defined(__MSP430_HAS_DMAX_6__) || defined(__MSP430X_HAS_DMA_6__)
#define MSP430X_DMA_CHANNELS 6
#else
#error "Unexpected error - how many DMA channels does your MSP have?"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief     Type of DMA callback function pointer.
 */
typedef void (*msp430x_dma_cbp_t)(void * args);

/**
 * @brief     DMA callback, function and argument.
 */
typedef struct {
  msp430x_dma_cbp_t callback; /**< @brief Callback function pointer   */
  void * args;                /**< @brief Callback function arguments */
} msp430x_dma_cb_t;

/**
 * @brief     MSP430X DMA request structure.
 */
typedef struct {
  const void * source_addr;  /**< @brief Source address                       */
  void * dest_addr;          /**< @brief Destination address                  */
  uint16_t size;             /**< @brief Number of values to transfer         */
  uint16_t addr_mode;        /**< @brief Address manipulation mode            */
  uint16_t data_mode;        /**< @brief Data sizes (b2b, w2w, b2w, w2b)      */
  uint16_t transfer_mode;    /**< @brief Transfer mode (single, block, burst) */
  uint16_t trigger;          /**< @brief Triggering event (see datasheet)     */
  msp430x_dma_cb_t callback; /**< @brief Callback function and arguments      */
} msp430x_dma_req_t;

/**
 * @brief     MSP430X DMA channel register structure.
 */
typedef struct {
  volatile uint16_t ctl; /**< @brief Control register             */
  volatile uint32_t sa;  /**< @brief Source address register      */
  volatile uint32_t da;  /**< @brief Destination address register */
  volatile uint16_t sz;  /**< @brief Size register                */
  volatile uint16_t pad1;
  volatile uint16_t pad2;
} msp430x_dma_ch_reg_t;

/**
 * @brief     MSP430X DMA controller register structure.
 */
typedef struct {
  volatile uint8_t tsel0; /**< @brief Trigger select for channel 0 */
  volatile uint8_t tsel1; /**< @brief Trigger select for channel 1 */
  volatile uint8_t tsel2; /**< @brief Trigger select for channel 2 */
  volatile uint8_t tsel3; /**< @brief Trigger select for channel 3 */
  volatile uint8_t tsel4; /**< @brief Trigger select for channel 4 */
  volatile uint8_t tsel5; /**< @brief Trigger select for channel 5 */
  volatile uint8_t tsel6; /**< @brief Trigger select for channel 6 */
  volatile uint8_t tsel7; /**< @brief Trigger select for channel 7 */
  volatile uint16_t ctl4; /**< @brief Controller register 4        */
} msp430x_dma_ctl_reg_t;

/**
 * @brief     MSP430X DMA channel structure.
 */
typedef struct {
  msp430x_dma_ch_reg_t * registers; /**< @brief Pointer to channel registers */
  uint8_t index; /**< @brief Index of channel trigger control register    */
  msp430x_dma_cb_t * cb; /**< @brief Pointer to callback function  and args  */
} msp430x_dma_ch_t;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief     Identifies a DMA trigger using a mnemonic.
 *
 * @param[in] mnem    The mnemonic for the trigger, e.g. UCA0RXIFG to trigger
 *                    on UART receive.
 */
#define DMA_TRIGGER_MNEM(mnem) DMA0TSEL__##mnem

/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
void dmaInit(void);
int dmaRequestS(msp430x_dma_req_t * request, systime_t timeout);
bool dmaAcquireI(msp430x_dma_ch_t * channel, uint8_t index);
void dmaTransfer(msp430x_dma_ch_t * channel, msp430x_dma_req_t * request);
void dmaRelease(msp430x_dma_ch_t * channel);

#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_DMA == true */

#endif /* HAL_MSP430X_DMA_H */

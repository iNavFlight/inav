/*
    Copyright (C) 2014..2017 Marco Veeneman

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
 * @file    uDMA/tiva_udma.h
 * @brief   DMA helper driver header.
 *
 * @addtogroup TIVA_DMA
 * @{
 */

#ifndef TIVA_UDMA_H_
#define TIVA_UDMA_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   CHCTL XFERSIZE helper.
 */
#define UDMA_CHCTL_XFERSIZE(n)          (((n)-1) << 4)

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   UDMA software interrupt priority level setting.
 */
#if !defined(TIVA_UDMA_SW_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_UDMA_SW_IRQ_PRIORITY           5
#endif

/**
 * @brief   UDMA error interrupt priority level setting.
 */
#if !defined(TIVA_UDMA_ERR_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_UDMA_ERR_IRQ_PRIORITY          5
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   A structure that defines an entry in the channel control table.
 * @note    These fields are used by the uDMA controller and normally it is not
 *          necessary for software to directly read or write fields in the
 *          table.
 */
typedef struct __attribute__((packed))
{
    /**
     * @brief   The ending source address of the data transfer.
     */
    volatile void *srcendp;
    /**
     * @brief   The ending destination address of the data transfer.
     */
    volatile void *dstendp;
    /**
     * @brief   The channel control mode.
     */
    volatile uint32_t chctl;
    /**
     * @brief   An unused location.
     */
    volatile uint32_t unused;
} tiva_udma_table_entry_t;

typedef struct __attribute__((packed, aligned(1024)))
{
  union {
    struct {
      tiva_udma_table_entry_t primary[32];
      tiva_udma_table_entry_t alternate[32];
    };
    uint8_t raw[1024];
  };
} udmaControlTable_t ;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

#define dmaChannelEnable(dmach) {\
  HWREG(UDMA_ENASET) = (1 << dmach);\
}

#define dmaChannelDisable(dmach) { \
  HWREG(UDMA_ENACLR) = (1 << dmach); \
}

#define dmaChannelPrimary(dmach) {\
  HWREG(UDMA_ALTCLR) = (1 << dmach); \
}

#define dmaChannelAlternate(dmach) { \
  HWREG(UDMA_ALTSET) = (1 << dmach); \
}

#define dmaChannelSingleBurst(dmach) { \
  HWREG(UDMA_USEBURSTCLR) = (1 << dmach); \
}

#define dmaChannelBurstOnly(dmach) { \
  HWREG(UDMA_USEBURSTSET) = (1 << dmach); \
}

#define dmaChannelPriorityHigh(dmach) { \
  HWREG(UDMA_PRIOSET) = (1 << dmach); \
}

#define dmaChannelPriorityDefault(dmach) { \
  HWREG(UDMA_PRIOCLR) = (1 << dmach); \
}

#define dmaChannelEnableRequest(dmach) {\
  HWREG(UDMA_REQMASKCLR) = (1 << dmach); \
}

#define dmaChannelDisableRequest(dmach) {\
  HWREG(UDMA_REQMASKSET) = (1 << dmach); \
}

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern udmaControlTable_t udmaControlTable;

#ifdef __cplusplus
extern "C" {
#endif
  void udmaInit(void);
  bool udmaChannelAllocate(uint8_t dmach);
  void udmaChannelRelease(uint8_t dmach);
#ifdef __cplusplus
}
#endif

#endif /* TIVA_UDMA_H_ */

/** @} */

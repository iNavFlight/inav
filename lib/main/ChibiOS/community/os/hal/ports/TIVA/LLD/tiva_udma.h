/*
    Copyright (C) 2014..2016 Marco Veeneman

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

#ifndef TIVA_UDMA_H_
#define TIVA_UDMA_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    CHCTL register defines.
 * @{
 */
#define UDMA_CHCTL_DSTINC_MASK          0xC0000000
#define UDMA_CHCTL_DSTINC_0             0xC0000000
#define UDMA_CHCTL_DSTINC_8             0x00000000
#define UDMA_CHCTL_DSTINC_16            0x40000000
#define UDMA_CHCTL_DSTINC_32            0x80000000
#define UDMA_CHCTL_DSTSIZE_MASK         0x30000000
#define UDMA_CHCTL_DSTSIZE_8            0x00000000
#define UDMA_CHCTL_DSTSIZE_16           0x10000000
#define UDMA_CHCTL_DSTSIZE_32           0x20000000
#define UDMA_CHCTL_SRCINC_MASK          0x0C000000
#define UDMA_CHCTL_SRCINC_0             0x0C000000
#define UDMA_CHCTL_SRCINC_8             0x00000000
#define UDMA_CHCTL_SRCINC_16            0x04000000
#define UDMA_CHCTL_SRCINC_32            0x08000000
#define UDMA_CHCTL_SRCSIZE_MASK         0x03000000
#define UDMA_CHCTL_SRCSIZE_8            0x00000000
#define UDMA_CHCTL_SRCSIZE_16           0x01000000
#define UDMA_CHCTL_SRCSIZE_32           0x02000000
#define UDMA_CHCTL_ARBSIZE_MASK         0x0003C000
#define UDMA_CHCTL_ARBSIZE_1            0x00000000
#define UDMA_CHCTL_ARBSIZE_2            0x00004000
#define UDMA_CHCTL_ARBSIZE_4            0x00008000
#define UDMA_CHCTL_ARBSIZE_8            0x0000C000
#define UDMA_CHCTL_ARBSIZE_16           0x00010000
#define UDMA_CHCTL_ARBSIZE_32           0x00014000
#define UDMA_CHCTL_ARBSIZE_64           0x00018000
#define UDMA_CHCTL_ARBSIZE_128          0x0001C000
#define UDMA_CHCTL_ARBSIZE_256          0x00020000
#define UDMA_CHCTL_ARBSIZE_512          0x00024000
#define UDMA_CHCTL_ARBSIZE_1024         0x00028000
#define UDMA_CHCTL_XFERSIZE_MASK        0x00003FF0
#define UDMA_CHCTL_XFERSIZE(n)          ((n-1) << 4)
#define UDMA_CHCTL_NXTUSEBURST          0x00000008
#define UDMA_CHCTL_XFERMODE_MASK        0x00000007
#define UDMA_CHCTL_XFERMODE_STOP        0x00000000
#define UDMA_CHCTL_XFERMODE_BASIC       0x00000001
#define UDMA_CHCTL_XFERMODE_AUTO        0x00000002
#define UDMA_CHCTL_XFERMODE_PINGPONG    0x00000003
#define UDMA_CHCTL_XFERMODE_MSG         0x00000004
#define UDMA_CHCTL_XFERMODE_AMSG        0x00000005
#define UDMA_CHCTL_XFERMODE_PSG         0x00000006
#define UDMA_CHCTL_XFERMODE_APSG        0x00000007
/** @} */

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
  UDMA->ENASET = (1 << dmach);\
}

#define dmaChannelDisable(dmach) { \
  UDMA->ENACLR = (1 << dmach); \
}

#define dmaChannelPrimary(dmach) {\
  UDMA->ALTCLR = (1 << dmach); \
}

#define dmaChannelAlternate(dmach) { \
  UDMA->ALTSET = (1 << dmach); \
}

#define dmaChannelSingleBurst(dmach) { \
  UDMA->USEBURSTCLR = (1 << dmach); \
}

#define dmaChannelBurstOnly(dmach) { \
  UDMA->USEBURSTSET = (1 << dmach); \
}

#define dmaChannelPriorityHigh(dmach) { \
  UDMA->PRIOSET = (1 << dmach); \
}

#define dmaChannelPriorityDefault(dmach) { \
  UDMA->PRIOCLR = (1 << dmach); \
}

#define dmaChannelEnableRequest(dmach) {\
  UDMA->REQMASKCLR = (1 << dmach); \
}

#define dmaChannelDisableRequest(dmach) {\
  UDMA->REQMASKSET = (1 << dmach); \
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

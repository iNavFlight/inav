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
 * @file    uDMA/tiva_udma.c
 * @brief   DMA helper driver code.
 *
 * @addtogroup TIVA_DMA
 * @{
 */

#include "hal.h"

/* The following macro is only defined if some driver requiring DMA services
   has been enabled.*/
#if defined(TIVA_UDMA_REQUIRED) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

udmaControlTable_t udmaControlTable;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

static uint32_t udma_channel_mask;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if !defined(TIVA_UDMA_SW_HANDLER)
#error "TIVA_UDMA_SW_HANDLER not defined"
#endif
/**
 * @brief   UDMA software interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_UDMA_SW_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  /* TODO Process software transfer interrupts.*/

  OSAL_IRQ_EPILOGUE();
}

#if !defined(TIVA_UDMA_ERR_HANDLER)
#error "TIVA_UDMA_ERR_HANDLER not defined"
#endif
/**
 * @brief   UDMA error interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_UDMA_ERR_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  /* TODO Do we need to halt the system on a DMA error?*/

  if (HWREG(UDMA_ERRCLR)) {
    HWREG(UDMA_ERRCLR) = 1;
  }

  OSAL_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Initialize UDMA.
 *
 * @init
 */
void udmaInit(void)
{
  udma_channel_mask = 0;

  /* Enable UDMA module.*/
  HWREG(SYSCTL_RCGCDMA) = 1;
  while (!(HWREG(SYSCTL_PRDMA) & (1 << 0)))
    ;

  nvicEnableVector(TIVA_UDMA_ERR_NUMBER, TIVA_UDMA_ERR_IRQ_PRIORITY);
  nvicEnableVector(TIVA_UDMA_SW_NUMBER, TIVA_UDMA_SW_IRQ_PRIORITY);

  /* Enable UDMA controller.*/
  HWREG(UDMA_CFG) = UDMA_CFG_MASTEN;

  /* Set address of control table.*/
  HWREG(UDMA_CTLBASE) = (uint32_t)udmaControlTable.primary;
}

/**
 * @brief   Allocates a DMA channel.
 *
 * @special
 */
bool udmaChannelAllocate(uint8_t dmach)
{
  /* Checks if the channel is already taken.*/
  if ((udma_channel_mask & (1 << dmach)) != 0)
    return TRUE;

  /* Mark channel as used */
  udma_channel_mask |= (1 << dmach);

  return FALSE;
}

/**
 * @brief   Releases a DMA channel.
 *
 * @special
 */
void udmaChannelRelease(uint8_t dmach)
{
  /* Marks the channel as not used.*/
  udma_channel_mask &= ~(1 << dmach);
}

#endif

/** @} */

/*
    SPC5 HAL - Copyright (C) 2013 STMicroelectronics

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
 * @file    FlexCAN_v1/hal_can_lld.c
 * @brief   SPC5xx CAN subsystem low level driver source.
 *
 * @addtogroup CAN
 * @{
 */

#include "hal.h"

#if HAL_USE_CAN || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief CAN1 driver identifier.*/
#if SPC5_CAN_USE_FLEXCAN0 || defined(__DOXYGEN__)
CANDriver CAND1;
#endif

/** @brief CAN2 driver identifier.*/
#if SPC5_CAN_USE_FLEXCAN1 || defined(__DOXYGEN__)
CANDriver CAND2;
#endif

/** @brief CAN3 driver identifier.*/
#if SPC5_CAN_USE_FLEXCAN2 || defined(__DOXYGEN__)
CANDriver CAND3;
#endif

/** @brief CAN4 driver identifier.*/
#if SPC5_CAN_USE_FLEXCAN3 || defined(__DOXYGEN__)
CANDriver CAND4;
#endif

/** @brief CAN5 driver identifier.*/
#if SPC5_CAN_USE_FLEXCAN4 || defined(__DOXYGEN__)
CANDriver CAND5;
#endif

/** @brief CAN6 driver identifier.*/
#if SPC5_CAN_USE_FLEXCAN5 || defined(__DOXYGEN__)
CANDriver CAND6;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Common TX ISR handler.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
static void can_lld_tx_handler(CANDriver *canp) {
  uint32_t iflag1, iflag2;
  (void)iflag2;

  /* No more events until a message is transmitted.*/
  iflag1 = canp->flexcan->IFRL.R;
  canp->flexcan->IFRL.R = iflag1 & 0xFFFFFF00;

#if SPC5_CAN_USE_FLEXCAN0 && (SPC5_FLEXCAN0_MB == 64)
  if(&CAND1 == canp) {
    iflag2 = canp->flexcan->IFRH.R;
    canp->flexcan->IFRH.R = canp->flexcan->IFRH.R;
  }
#endif
#if SPC5_CAN_USE_FLEXCAN1 && (SPC5_FLEXCAN1_MB == 64)
  if(&CAND2 == canp) {
    iflag2 = canp->flexcan->IFRH.R;
    canp->flexcan->IFRH.R = canp->flexcan->IFRH.R;
  }
#endif
#if SPC5_CAN_USE_FLEXCAN2 && (SPC5_FLEXCAN2_MB == 64)
  if(&CAND3 == canp) {
    iflag2 = canp->flexcan->IFRH.R;
    canp->flexcan->IFRH.R = canp->flexcan->IFRH.R;
  }
#endif
#if SPC5_CAN_USE_FLEXCAN3 && (SPC5_FLEXCAN3_MB == 64)
  if(&CAND4 == canp) {
    iflag2 = canp->flexcan->IFRH.R;
    canp->flexcan->IFRH.R = canp->flexcan->IFRH.R;
  }
#endif
#if SPC5_CAN_USE_FLEXCAN4 && (SPC5_FLEXCAN4_MB == 64)
  if(&CAND5 == canp) {
    iflag2 = canp->flexcan->IFRH.R;
    canp->flexcan->IFRH.R = canp->flexcan->IFRH.R;
  }
#endif
#if SPC5_CAN_USE_FLEXCAN5 && (SPC5_FLEXCAN5_MB == 64)
  if(&CAND6 == canp) {
    iflag2 = canp->flexcan->IFRH.R;
    canp->flexcan->IFRH.R = canp->flexcan->IFRH.R;
  }
#endif

  osalSysLockFromISR();
  osalThreadDequeueAllI(&canp->txqueue, MSG_OK);

#if SPC5_CAN_USE_FLEXCAN0 && (SPC5_FLEXCAN0_MB == 32)
  if(&CAND1 == canp) {
    osalEventBroadcastFlagsI(&canp->txempty_event, iflag1 & 0xFFFFFF00);
  }
#elif SPC5_CAN_USE_FLEXCAN0 && (SPC5_FLEXCAN0_MB == 64)
  if(&CAND1 == canp) {
    osalEventBroadcastFlagsI(&canp->txempty_event, iflag2 | (iflag1 & 0xFFFFFF00));
  }
#endif
#if SPC5_CAN_USE_FLEXCAN1 && (SPC5_FLEXCAN1_MB == 32)
  if(&CAND2 == canp) {
    osalEventBroadcastFlagsI(&canp->txempty_event, iflag1 & 0xFFFFFF00);
  }
#elif SPC5_CAN_USE_FLEXCAN1 && (SPC5_FLEXCAN1_MB == 64)
  if(&CAND2 == canp) {
    osalEventBroadcastFlagsI(&canp->txempty_event, iflag2 | (iflag1 & 0xFFFFFF00));
  }
#endif
#if SPC5_CAN_USE_FLEXCAN2 && (SPC5_FLEXCAN2_MB == 32)
  if(&CAND3 == canp) {
    osalEventBroadcastFlagsI(&canp->txempty_event, iflag1 & 0xFFFFFF00);
  }
#elif SPC5_CAN_USE_FLEXCAN2 && (SPC5_FLEXCAN2_MB == 64)
  if(&CAND3 == canp) {
    osalEventBroadcastFlagsI(&canp->txempty_event, iflag2 | (iflag1 & 0xFFFFFF00));
  }
#endif
#if SPC5_CAN_USE_FLEXCAN3 && (SPC5_FLEXCAN3_MB == 32)
  if(&CAND4 == canp) {
    osalEventBroadcastFlagsI(&canp->txempty_event, iflag1 & 0xFFFFFF00);
  }
#elif SPC5_CAN_USE_FLEXCAN3 && (SPC5_FLEXCAN3_MB == 64)
  if(&CAND4 == canp) {
    osalEventBroadcastFlagsI(&canp->txempty_event, iflag2 | (iflag1 & 0xFFFFFF00));
  }
#endif
#if SPC5_CAN_USE_FLEXCAN4 && (SPC5_FLEXCAN4_MB == 32)
  if(&CAND5 == canp) {
    osalEventBroadcastFlagsI(&canp->txempty_event, iflag1 & 0xFFFFFF00);
  }
#elif SPC5_CAN_USE_FLEXCAN4 && (SPC5_FLEXCAN4_MB == 64)
  if(&CAND5 == canp) {
    osalEventBroadcastFlagsI(&canp->txempty_event, iflag2 | (iflag1 & 0xFFFFFF00));
  }
#endif
#if SPC5_CAN_USE_FLEXCAN5 && (SPC5_FLEXCAN5_MB == 32)
  if(&CAND6 == canp) {
    osalEventBroadcastFlagsI(&canp->txempty_event, iflag1 & 0xFFFFFF00);
  }
#elif SPC5_CAN_USE_FLEXCAN5 && (SPC5_FLEXCAN5_MB == 64)
  if(&CAND6 == canp) {
    osalEventBroadcastFlagsI(&canp->txempty_event, iflag2 | (iflag1 & 0xFFFFFF00));
  }
#endif

  osalSysUnlockFromISR();
}

/**
 * @brief   Common RX ISR handler.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
static void can_lld_rx_handler(CANDriver *canp) {
  uint32_t iflag1;

  iflag1 = canp->flexcan->IFRL.R;
  if ((iflag1 & 0x000000FF) != 0) {
    osalSysLockFromISR();
    osalThreadDequeueAllI(&canp->rxqueue, MSG_OK);
    osalEventBroadcastFlagsI(&canp->rxfull_event, iflag1 & 0x000000FF);
    osalSysUnlockFromISR();

    /* Release the mailbox.*/
    canp->flexcan->IFRL.R = iflag1 & 0x000000FF;
  }
}

/**
 * @brief   Common error ISR handler.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
static void can_lld_err_handler(CANDriver *canp) {

  uint32_t esr = canp->flexcan->ESR.R;
  eventflags_t flags = 0;

  /* Error event.*/
  if ((esr & CAN_ESR_TWRN_INT) || (esr & CAN_ESR_RWRN_INT)) {
    canp->flexcan->ESR.B.TXWRN = 1U;
    canp->flexcan->ESR.B.RXWRN = 1U;
    flags |= CAN_LIMIT_WARNING;
  }

  if (esr & CAN_ESR_BOFF_INT) {
    canp->flexcan->ESR.B.BOFFINT = 1U;
    flags |= CAN_BUS_OFF_ERROR;
  }

  if (esr & CAN_ESR_ERR_INT) {
    canp->flexcan->ESR.B.ERRINT = 1U;
    flags |= CAN_FRAMING_ERROR;
  }
  osalSysLockFromISR();
  osalEventBroadcastFlagsI(&canp->error_event, flags);
  osalSysUnlockFromISR();
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if SPC5_CAN_USE_FLEXCAN0 || defined(__DOXYGEN__)
#if !SPC5_FLEXCAN0_SHARED_IRQ
/**
 * @brief   CAN1 RX interrupt handler for MB 0.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_00_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 RX interrupt handler for MB 1.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_01_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 RX interrupt handler for MB 2.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_02_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 RX interrupt handler for MB 3.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_03_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 RX interrupt handler for MB 4.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_04_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 RX interrupt handler for MB 5.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_05_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 RX interrupt handler for MB 6.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_06_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 RX interrupt handler for MB 7.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_07_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 TX interrupt handler for MB 8.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_08_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 TX interrupt handler for MB 9.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_09_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 TX interrupt handler for MB 10.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_10_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 TX interrupt handler for MB 11.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_11_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 TX interrupt handler for MB 12.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_12_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 TX interrupt handler for MB 13.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_13_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 TX interrupt handler for MB 14.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_14_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 TX interrupt handler for MB 15.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_16_31_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN0_MB == 64)
/**
 * @brief   CAN1 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_32_63_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}
#endif

/**
 * @brief   CAN1 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_ESR_ERR_INT_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 ESR_BOFF interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_ESR_BOFF_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}
#else
/**
 * @brief   CAN1 TX interrupt handler for MB 8-11.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_08_11_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 TX interrupt handler for MB 12-15.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_12_15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_16_31_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN0_MB == 64)
/**
 * @brief   CAN1 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_32_63_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}
#endif

/*
 * @brief   CAN1 RX interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_00_03_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/*
 * @brief   CAN1 RX interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_BUF_04_07_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_ESR_ERR_INT_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN1 ESR_BOFF interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN0_FLEXCAN_ESR_BOFF_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND1);

  OSAL_IRQ_EPILOGUE();
}
#endif
#endif /* SPC5_CAN_USE_FLEXCAN0 */

#if SPC5_CAN_USE_FLEXCAN1 || defined(__DOXYGEN__)
#if !SPC5_FLEXCAN1_SHARED_IRQ
/**
 * @brief   CAN2 RX interrupt handler for MB 0.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_00_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 RX interrupt handler for MB 1.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_01_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 RX interrupt handler for MB 2.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_02_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 RX interrupt handler for MB 3.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_03_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 RX interrupt handler for MB 4.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_04_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 RX interrupt handler for MB 5.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_05_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 RX interrupt handler for MB 6.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_06_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 RX interrupt handler for MB 7.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_07_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 TX interrupt handler for MB 8.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_08_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 TX interrupt handler for MB 9.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_09_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 TX interrupt handler for MB 10.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_10_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 TX interrupt handler for MB 11.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_11_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 TX interrupt handler for MB 12.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_12_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 TX interrupt handler for MB 13.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_13_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 TX interrupt handler for MB 14.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_14_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 TX interrupt handler for MB 15.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_16_31_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN1_MB == 64)
/**
 * @brief   CAN2 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_32_63_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}
#endif

/**
 * @brief   CAN2 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_ESR_ERR_INT_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 ESR_BOFF interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_ESR_BOFF_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}
#else
/**
 * @brief   CAN2 TX interrupt handler for MB 8-11.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_08_11_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 TX interrupt handler for MB 12-15.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_12_15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_16_31_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN1_MB == 64)
/**
 * @brief   CAN2 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_32_63_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}
#endif

/*
 * @brief   CAN2 RX interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_00_03_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/*
 * @brief   CAN2 RX interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_BUF_04_07_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_ESR_ERR_INT_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN2 ESR_BOFF interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN1_FLEXCAN_ESR_BOFF_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND2);

  OSAL_IRQ_EPILOGUE();
}
#endif
#endif /* SPC5_CAN_USE_FLEXCAN1 */

#if SPC5_CAN_USE_FLEXCAN2 || defined(__DOXYGEN__)
#if !SPC5_FLEXCAN2_SHARED_IRQ
/**
 * @brief   CAN3 RX interrupt handler for MB 0.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_00_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 RX interrupt handler for MB 1.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_01_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 RX interrupt handler for MB 2.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_02_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 RX interrupt handler for MB 3.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_03_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 RX interrupt handler for MB 4.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_04_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 RX interrupt handler for MB 5.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_05_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 RX interrupt handler for MB 6.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_06_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 RX interrupt handler for MB 7.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_07_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 TX interrupt handler for MB 8.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_08_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 TX interrupt handler for MB 9.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_09_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 TX interrupt handler for MB 10.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_10_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 TX interrupt handler for MB 11.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_11_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 TX interrupt handler for MB 12.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_12_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 TX interrupt handler for MB 13.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_13_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 TX interrupt handler for MB 14.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_14_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 TX interrupt handler for MB 15.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_16_31_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN2_MB == 64)
/**
 * @brief   CAN3 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_32_63_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}
#endif

/**
 * @brief   CAN3 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_ESR_ERR_INT_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 ESR_BOFF interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_ESR_BOFF_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}
#else
/**
 * @brief   CAN3 TX interrupt handler for MB 8-11.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_08_11_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 TX interrupt handler for MB 12-15.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_12_15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_16_31_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN2_MB == 64)
/**
 * @brief   CAN3 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_32_63_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}
#endif

/*
 * @brief   CAN3 RX interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_00_03_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/*
 * @brief   CAN3 RX interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_BUF_04_07_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_ESR_ERR_INT_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN3 ESR_BOFF interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN2_FLEXCAN_ESR_BOFF_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND3);

  OSAL_IRQ_EPILOGUE();
}
#endif
#endif /* SPC5_CAN_USE_FLEXCAN2 */

#if SPC5_CAN_USE_FLEXCAN3 || defined(__DOXYGEN__)
#if !SPC5_FLEXCAN3_SHARED_IRQ
/**
 * @brief   CAN4 RX interrupt handler for MB 0.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_00_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 RX interrupt handler for MB 1.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_01_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 RX interrupt handler for MB 2.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_02_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 RX interrupt handler for MB 3.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_03_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 RX interrupt handler for MB 4.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_04_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 RX interrupt handler for MB 5.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_05_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 RX interrupt handler for MB 6.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_06_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 RX interrupt handler for MB 7.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_07_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 TX interrupt handler for MB 8.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_08_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 TX interrupt handler for MB 9.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_09_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 TX interrupt handler for MB 10.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_10_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 TX interrupt handler for MB 11.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_11_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 TX interrupt handler for MB 12.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_12_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 TX interrupt handler for MB 13.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_13_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 TX interrupt handler for MB 14.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_14_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 TX interrupt handler for MB 15.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_16_31_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN3_MB == 64)
/**
 * @brief   CAN4 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_32_63_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}
#endif

/**
 * @brief   CAN4 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_ESR_ERR_INT_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 ESR_BOFF interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_ESR_BOFF_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}
#else
/**
 * @brief   CAN4 TX interrupt handler for MB 8-11.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_08_11_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 TX interrupt handler for MB 12-15.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_12_15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_16_31_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN3_MB == 64)
/**
 * @brief   CAN4 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_32_63_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}
#endif

/*
 * @brief   CAN4 RX interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_00_03_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/*
 * @brief   CAN4 RX interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_BUF_04_07_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_ESR_ERR_INT_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN4 ESR_BOFF interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN3_FLEXCAN_ESR_BOFF_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND4);

  OSAL_IRQ_EPILOGUE();
}
#endif
#endif /* SPC5_CAN_USE_FLEXCAN3 */

#if SPC5_CAN_USE_FLEXCAN4 || defined(__DOXYGEN__)
#if !SPC5_FLEXCAN4_SHARED_IRQ
/**
 * @brief   CAN5 RX interrupt handler for MB 0.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_00_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 RX interrupt handler for MB 1.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_01_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 RX interrupt handler for MB 2.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_02_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 RX interrupt handler for MB 3.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_03_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 RX interrupt handler for MB 4.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_04_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 RX interrupt handler for MB 5.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_05_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 RX interrupt handler for MB 6.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_06_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 RX interrupt handler for MB 7.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_07_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 TX interrupt handler for MB 8.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_08_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 TX interrupt handler for MB 9.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_09_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 TX interrupt handler for MB 10.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_10_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 TX interrupt handler for MB 11.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_11_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 TX interrupt handler for MB 12.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_12_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 TX interrupt handler for MB 13.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_13_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 TX interrupt handler for MB 14.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_14_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 TX interrupt handler for MB 15.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_16_31_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN4_MB == 64)
/**
 * @brief   CAN5 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_32_63_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}
#endif

/**
 * @brief   CAN5 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_ESR_ERR_INT_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 ESR_BOFF interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_ESR_BOFF_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}
#else
/**
 * @brief   CAN5 TX interrupt handler for MB 8-11.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_08_11_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 TX interrupt handler for MB 12-15.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_12_15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_16_31_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN4_MB == 64)
/**
 * @brief   CAN5 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_32_63_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}
#endif

/*
 * @brief   CAN5 RX interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_00_03_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/*
 * @brief   CAN5 RX interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_BUF_04_07_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_ESR_ERR_INT_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN5 ESR_BOFF interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN4_FLEXCAN_ESR_BOFF_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND5);

  OSAL_IRQ_EPILOGUE();
}
#endif
#endif /* SPC5_CAN_USE_FLEXCAN4 */

#if SPC5_CAN_USE_FLEXCAN5 || defined(__DOXYGEN__)
#if !SPC5_FLEXCAN5_SHARED_IRQ
/**
 * @brief   CAN6 RX interrupt handler for MB 0.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_00_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 RX interrupt handler for MB 1.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_01_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 RX interrupt handler for MB 2.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_02_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 RX interrupt handler for MB 3.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_03_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 RX interrupt handler for MB 4.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_04_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 RX interrupt handler for MB 5.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_05_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 RX interrupt handler for MB 6.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_06_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 RX interrupt handler for MB 7.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_07_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 TX interrupt handler for MB 8.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_08_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 TX interrupt handler for MB 9.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_09_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 TX interrupt handler for MB 10.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_10_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 TX interrupt handler for MB 11.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_11_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 TX interrupt handler for MB 12.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_12_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 TX interrupt handler for MB 13.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_13_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 TX interrupt handler for MB 14.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_14_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 TX interrupt handler for MB 15.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_16_31_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN5_MB == 64)
/**
 * @brief   CAN6 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_32_63_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}
#endif

/**
 * @brief   CAN6 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_ESR_ERR_INT_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 ESR_BOFF interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_ESR_BOFF_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}
#else
/**
 * @brief   CAN6 TX interrupt handler for MB 8-11.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_08_11_HANDLER) {


  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 TX interrupt handler for MB 12-15.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_12_15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 TX interrupt handler for MB 16-31.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_16_31_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

#if (SPC5_FLEXCAN5_MB == 64)
/**
 * @brief   CAN6 TX interrupt handler for MB 32-63.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_32_63_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_tx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}
#endif

/*
 * @brief   CAN6 RX interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_00_03_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/*
 * @brief   CAN6 RX interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_BUF_04_07_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_rx_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 ESR_ERR_INT interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_ESR_ERR_INT_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   CAN6 ESR_BOFF interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_FLEXCAN5_FLEXCAN_ESR_BOFF_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  can_lld_err_handler(&CAND6);

  OSAL_IRQ_EPILOGUE();
}
#endif
#endif /* SPC5_CAN_USE_FLEXCAN5 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level CAN driver initialization.
 *
 * @notapi
 */
void can_lld_init(void) {

#if SPC5_CAN_USE_FLEXCAN0
  /* Driver initialization.*/
  canObjectInit(&CAND1);
  CAND1.flexcan = &SPC5_FLEXCAN_0;
#if !SPC5_FLEXCAN0_SHARED_IRQ
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_ESR_ERR_INT_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_ESR_BOFF_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_00_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_01_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_02_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_03_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_04_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_05_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_06_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_07_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_08_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_09_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_10_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_11_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_12_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_13_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_14_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_15_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_16_31_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
#if (SPC5_FLEXCAN0_MB == 64)
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_32_63_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
#endif
#else
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_ESR_ERR_INT_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_ESR_BOFF_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_00_03_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_04_07_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_08_11_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_12_15_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN0_FLEXCAN_BUF_16_31_NUMBER) =
      SPC5_CAN_FLEXCAN0_IRQ_PRIORITY;
#endif
#endif

#if SPC5_CAN_USE_FLEXCAN1
  /* Driver initialization.*/
  canObjectInit(&CAND2);
  CAND2.flexcan = &SPC5_FLEXCAN_1;
#if !SPC5_FLEXCAN1_SHARED_IRQ
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_ESR_ERR_INT_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_ESR_BOFF_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_00_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_01_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_02_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_03_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_04_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_05_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_06_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_07_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_08_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_09_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_10_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_11_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_12_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_13_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_14_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_15_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_16_31_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
#if (SPC5_FLEXCAN1_MB == 64)
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_32_63_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
#endif
#else
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_ESR_ERR_INT_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_ESR_BOFF_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_00_03_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_04_07_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_08_11_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_12_15_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN1_FLEXCAN_BUF_16_31_NUMBER) =
      SPC5_CAN_FLEXCAN1_IRQ_PRIORITY;
#endif
#endif

#if SPC5_CAN_USE_FLEXCAN2
  /* Driver initialization.*/
  canObjectInit(&CAND3);
  CAND3.flexcan = &SPC5_FLEXCAN_2;
#if !SPC5_FLEXCAN2_SHARED_IRQ
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_ESR_ERR_INT_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_ESR_BOFF_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_00_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_01_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_02_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_03_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_04_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_05_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_06_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_07_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_08_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_09_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_10_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_11_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_12_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_13_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_14_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_15_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_16_31_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_32_63_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
#else
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_ESR_ERR_INT_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_ESR_BOFF_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_00_03_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_04_07_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_08_11_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_12_15_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN2_FLEXCAN_BUF_16_31_NUMBER) =
      SPC5_CAN_FLEXCAN2_IRQ_PRIORITY;
#endif
#endif

#if SPC5_CAN_USE_FLEXCAN3
  /* Driver initialization.*/
  canObjectInit(&CAND4);
  CAND4.flexcan = &SPC5_FLEXCAN_3;
#if !SPC5_FLEXCAN3_SHARED_IRQ
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_ESR_ERR_INT_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_ESR_BOFF_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_00_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_01_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_02_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_03_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_04_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_05_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_06_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_07_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_08_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_09_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_10_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_11_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_12_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_13_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_14_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_15_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_16_31_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_32_63_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
#else
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_ESR_ERR_INT_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_ESR_BOFF_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_00_03_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_04_07_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_08_11_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_12_15_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN3_FLEXCAN_BUF_16_31_NUMBER) =
      SPC5_CAN_FLEXCAN3_IRQ_PRIORITY;
#endif
#endif

#if SPC5_CAN_USE_FLEXCAN4
  /* Driver initialization.*/
  canObjectInit(&CAND5);
  CAND5.flexcan = &SPC5_FLEXCAN_4;
#if !SPC5_FLEXCAN4_SHARED_IRQ
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_ESR_ERR_INT_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_ESR_BOFF_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_00_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_01_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_02_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_03_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_04_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_05_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_06_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_07_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_08_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_09_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_10_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_11_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_12_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_13_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_14_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_15_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_16_31_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_32_63_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
#else
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_ESR_ERR_INT_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_ESR_BOFF_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_00_03_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_04_07_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_08_11_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_12_15_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN4_FLEXCAN_BUF_16_31_NUMBER) =
      SPC5_CAN_FLEXCAN4_IRQ_PRIORITY;
#endif
#endif

#if SPC5_CAN_USE_FLEXCAN5
  /* Driver initialization.*/
  canObjectInit(&CAND6);
  CAND6.flexcan = &SPC5_FLEXCAN_5;
#if !SPC5_FLEXCAN5_SHARED_IRQ
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_ESR_ERR_INT_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_ESR_BOFF_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_00_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_01_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_02_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_03_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_04_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_05_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_06_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_07_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_08_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_09_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_10_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_11_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_12_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_13_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_14_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_15_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_16_31_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_32_63_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
#else
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_ESR_ERR_INT_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_ESR_BOFF_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_00_03_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_04_07_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_08_11_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_12_15_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
  INTC_PSR(SPC5_FLEXCAN5_FLEXCAN_BUF_16_31_NUMBER) =
      SPC5_CAN_FLEXCAN5_IRQ_PRIORITY;
#endif
#endif
}

/**
 * @brief   Configures and activates the CAN peripheral.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
void can_lld_start(CANDriver *canp) {

  uint8_t mb_index = 0;
#if SPC5_CAN_USE_FILTERS
  uint8_t id = 0;
#endif

  /* Entering initialization mode. */
  canp->state = CAN_STARTING;

  /* Clock activation.*/
#if SPC5_CAN_USE_FLEXCAN0
  /* Set peripheral clock mode.*/
  if(&CAND1 == canp) {
    SPC5_FLEXCAN0_ENABLE_CLOCK();
#if !SPC5_CAN_FLEXCAN0_USE_EXT_CLK
    canp->flexcan->CR.R |= CAN_CTRL_CLK_SRC;
#endif
  }
#endif
#if SPC5_CAN_USE_FLEXCAN1
  /* Set peripheral clock mode.*/
  if(&CAND2 == canp) {
    SPC5_FLEXCAN1_ENABLE_CLOCK();
#if !SPC5_CAN_FLEXCAN1_USE_EXT_CLK
    canp->flexcan->CR.R |= CAN_CTRL_CLK_SRC;
#endif
  }
#endif
#if SPC5_CAN_USE_FLEXCAN2
  /* Set peripheral clock mode.*/
  if(&CAND3 == canp) {
    SPC5_FLEXCAN2_ENABLE_CLOCK();
#if !SPC5_CAN_FLEXCAN2_USE_EXT_CLK
    canp->flexcan->CR.R |= CAN_CTRL_CLK_SRC;
#endif
  }
#endif
#if SPC5_CAN_USE_FLEXCAN3
  /* Set peripheral clock mode.*/
  if(&CAND4 == canp) {
    SPC5_FLEXCAN3_ENABLE_CLOCK();
#if !SPC5_CAN_FLEXCAN3_USE_EXT_CLK
    canp->flexcan->CR.R |= CAN_CTRL_CLK_SRC;
#endif
  }
#endif
#if SPC5_CAN_USE_FLEXCAN4
  /* Set peripheral clock mode.*/
  if(&CAND5 == canp) {
    SPC5_FLEXCAN4_ENABLE_CLOCK();
#if !SPC5_CAN_FLEXCAN4_USE_EXT_CLK
    canp->flexcan->CR.R |= CAN_CTRL_CLK_SRC;
#endif
  }
#endif
#if SPC5_CAN_USE_FLEXCAN5
  /* Set peripheral clock mode.*/
  if(&CAND6 == canp) {
    SPC5_FLEXCAN5_ENABLE_CLOCK();
#if !SPC5_CAN_FLEXCAN5_USE_EXT_CLK
    canp->flexcan->CR.R |= CAN_CTRL_CLK_SRC;
#endif
  }
#endif

  /* Enable the device.*/
  canp->flexcan->MCR.R &= ~CAN_MCR_MDIS;

  /*
   * Individual filtering per MB, disable frame self reception,
   * disable the FIFO, enable SuperVisor mode.
   */
#if SPC5_CAN_USE_FLEXCAN0
  if(&CAND1 == canp)
    canp->flexcan->MCR.R |= CAN_MCR_SUPV | CAN_MCR_MAXMB(SPC5_FLEXCAN0_MB - 1);
#endif

#if SPC5_CAN_USE_FLEXCAN1
  if(&CAND2 == canp)
    canp->flexcan->MCR.R |= CAN_MCR_SUPV | CAN_MCR_MAXMB(SPC5_FLEXCAN1_MB - 1);
#endif

#if SPC5_CAN_USE_FLEXCAN2
  if(&CAND3 == canp)
    canp->flexcan->MCR.R |= CAN_MCR_SUPV | CAN_MCR_MAXMB(SPC5_FLEXCAN2_MB - 1);
#endif

#if SPC5_CAN_USE_FLEXCAN3
  if(&CAND4 == canp)
    canp->flexcan->MCR.R |= CAN_MCR_SUPV | CAN_MCR_MAXMB(SPC5_FLEXCAN3_MB - 1);
#endif

#if SPC5_CAN_USE_FLEXCAN4
  if(&CAND5 == canp)
    canp->flexcan->MCR.R |= CAN_MCR_SUPV | CAN_MCR_MAXMB(SPC5_FLEXCAN4_MB - 1);
#endif

#if SPC5_CAN_USE_FLEXCAN5
  if(&CAND6 == canp)
    canp->flexcan->MCR.R |= CAN_MCR_SUPV | CAN_MCR_MAXMB(SPC5_FLEXCAN5_MB - 1);
#endif

  canp->flexcan->CR.R |= CAN_CTRL_TSYN | CAN_CTRL_RJW(3);

  /* TX MB initialization.*/
#if SPC5_CAN_USE_FLEXCAN0
  if(&CAND1 == canp) {
    for(mb_index = 0; mb_index < CAN_TX_MAILBOXES ; mb_index++) {
      canp->flexcan->BUF[mb_index + CAN_RX_MAILBOXES].CS.B.CODE = 8U;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN1
  if(&CAND2 == canp) {
	for(mb_index = 0; mb_index < CAN_TX_MAILBOXES ; mb_index++) {
      canp->flexcan->BUF[mb_index + CAN_RX_MAILBOXES].CS.B.CODE = 8U;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN2
  if(&CAND3 == canp) {
	for(mb_index = 0; mb_index < CAN_TX_MAILBOXES ; mb_index++) {
      canp->flexcan->BUF[mb_index + CAN_RX_MAILBOXES].CS.B.CODE = 8U;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN3
  if(&CAND4 == canp) {
	for(mb_index = 0; mb_index < CAN_TX_MAILBOXES ; mb_index++) {
      canp->flexcan->BUF[mb_index + CAN_RX_MAILBOXES].CS.B.CODE = 8U;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN4
  if(&CAND5 == canp) {
	for(mb_index = 0; mb_index < CAN_TX_MAILBOXES ; mb_index++) {
      canp->flexcan->BUF[mb_index + CAN_RX_MAILBOXES].CS.B.CODE = 8U;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN5
  if(&CAND6 == canp) {
	for(mb_index = 0; mb_index < CAN_TX_MAILBOXES ; mb_index++) {
      canp->flexcan->BUF[mb_index + CAN_RX_MAILBOXES].CS.B.CODE = 8U;
    }
  }
#endif

  /* Unlock Message buffers.*/
  (void) canp->flexcan->TIMER.R;

  /* MCR initialization.*/
  canp->flexcan->MCR.R |= canp->config->mcr;

  /* CTRL initialization.*/
  canp->flexcan->CR.R |= canp->config->ctrl;

  /* Interrupt sources initialization.*/
  canp->flexcan->MCR.R |= CAN_MCR_WRN_EN;

  canp->flexcan->CR.R |= CAN_CTRL_BOFF_MSK | CAN_CTRL_ERR_MSK  |
                         CAN_CTRL_TWRN_MSK | CAN_CTRL_RWRN_MSK;

#if !SPC5_CAN_USE_FILTERS
  /* RX MB initialization.*/
  for(mb_index = 0; mb_index < CAN_RX_MAILBOXES; mb_index++) {
    canp->flexcan->BUF[mb_index].CS.B.CODE = 0U;
    if(mb_index < 4) {
      canp->flexcan->BUF[mb_index].CS.B.IDE = 0U;
    }
    else {
      canp->flexcan->BUF[mb_index].CS.B.IDE = 1U;
    }
    canp->flexcan->BUF[mb_index].ID.R = 0U;
    canp->flexcan->BUF[mb_index].CS.B.CODE = 4U;
  }

  /* Receive all.*/
  canp->flexcan->RXGMASK.R = 0x00000000;
#else
  for (id = 0; id < CAN_RX_MAILBOXES; id++) {
    canp->flexcan->BUF[id].CS.B.CODE = 0U;
    if (canp->config->RxFilter[id].scale) {
      canp->flexcan->BUF[id].CS.B.IDE = 1U;
      canp->flexcan->BUF[id].ID.R = canp->config->RxFilter[id].register1;
    }
    else {
      canp->flexcan->BUF[id].CS.B.IDE = 0U;
      canp->flexcan->BUF[id].ID.B.STD_ID = canp->config->RxFilter[id].register1;
      canp->flexcan->BUF[id].ID.B.EXT_ID = 0U;
    }
    /* RX MB initialization.*/
    canp->flexcan->BUF[id].CS.B.CODE = 4U;
  }
  canp->flexcan->RXGMASK.R = 0x0FFFFFFF;
#endif

  /* Enable MBs interrupts.*/
#if SPC5_CAN_USE_FLEXCAN0
  if(&CAND1 == canp) {
    if(SPC5_FLEXCAN0_MB == 32) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
    }
    else if(SPC5_FLEXCAN0_MB == 64) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
      canp->flexcan->IMRH.R = 0xFFFFFFFF;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN1
  if(&CAND2 == canp) {
    if(SPC5_FLEXCAN1_MB == 32) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
    }
    else if(SPC5_FLEXCAN1_MB == 64) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
      canp->flexcan->IMRH.R = 0xFFFFFFFF;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN2
  if(&CAND3 == canp) {
    if(SPC5_FLEXCAN2_MB == 32) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
    }
    else if(SPC5_FLEXCAN2_MB == 64) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
      canp->flexcan->IMRH.R = 0xFFFFFFFF;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN3
  if(&CAND4 == canp) {
    if(SPC5_FLEXCAN3_MB == 32) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
    }
    else if(SPC5_FLEXCAN3_MB == 64) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
      canp->flexcan->IMRH.R = 0xFFFFFFFF;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN4
  if(&CAND5 == canp) {
    if(SPC5_FLEXCAN4_MB == 32) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
    }
    else if(SPC5_FLEXCAN4_MB == 64) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
      canp->flexcan->IMRH.R = 0xFFFFFFFF;
    }
  }
#endif
#if SPC5_CAN_USE_FLEXCAN5
  if(&CAND6 == canp) {
    if(SPC5_FLEXCAN5_MB == 32) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
    }
    else if(SPC5_FLEXCAN5_MB == 64) {
      canp->flexcan->IMRL.R = 0xFFFFFFFF;
      canp->flexcan->IMRH.R = 0xFFFFFFFF;
    }
  }
#endif

  /* CAN BUS synchronization.*/
  canp->flexcan->MCR.B.HALT = 0;
}

/**
 * @brief   Deactivates the CAN peripheral.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
void can_lld_stop(CANDriver *canp) {

  /* If in ready state then disables the CAN peripheral.*/
  if (canp->state == CAN_READY) {

    /* Disable Interrupt sources.*/
    canp->flexcan->MCR.R &= ~CAN_MCR_WRN_EN;
    canp->flexcan->CR.R &= ~(CAN_CTRL_BOFF_MSK | CAN_CTRL_ERR_MSK  |
                             CAN_CTRL_TWRN_MSK | CAN_CTRL_RWRN_MSK);
    canp->flexcan->IMRL.R = 0x00000000;

    canp->flexcan->MCR.R |= CAN_MCR_MDIS;

#if SPC5_CAN_USE_FLEXCAN0
  /* Set peripheral clock mode.*/
  if(&CAND1 == canp) {
    SPC5_FLEXCAN0_DISABLE_CLOCK();
  }
#endif
#if SPC5_CAN_USE_FLEXCAN1
  /* Set peripheral clock mode.*/
  if(&CAND2 == canp) {
    SPC5_FLEXCAN1_DISABLE_CLOCK();
  }
#endif
#if SPC5_CAN_USE_FLEXCAN2
  /* Set peripheral clock mode.*/
  if(&CAND3 == canp) {
    SPC5_FLEXCAN2_DISABLE_CLOCK();
  }
#endif
#if SPC5_CAN_USE_FLEXCAN3
  /* Set peripheral clock mode.*/
  if(&CAND4 == canp) {
    SPC5_FLEXCAN3_DISABLE_CLOCK();
  }
#endif
#if SPC5_CAN_USE_FLEXCAN4
  /* Set peripheral clock mode.*/
  if(&CAND5 == canp) {
    SPC5_FLEXCAN4_DISABLE_CLOCK();
  }
#endif
#if SPC5_CAN_USE_FLEXCAN5
  /* Set peripheral clock mode.*/
  if(&CAND6 == canp) {
    SPC5_FLEXCAN5_DISABLE_CLOCK();
  }
#endif
  }
}

/**
 * @brief   Determines whether a frame can be transmitted.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] mailbox   mailbox number, @p CAN_ANY_MAILBOX for any mailbox
 *
 * @return              The queue space availability.
 * @retval FALSE        no space in the transmit queue.
 * @retval TRUE         transmit slot available.
 *
 * @notapi
 */
bool can_lld_is_tx_empty(CANDriver *canp, canmbx_t mailbox) {

  uint8_t mbid = 0;

  if(mailbox == CAN_ANY_MAILBOX) {
#if SPC5_CAN_USE_FLEXCAN0
    if(&CAND1 == canp) {
      for (mbid = 0; mbid < CAN_TX_MAILBOXES; mbid++) {
        if (canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES].CS.B.CODE == 0x08) {
          return TRUE;
        }
      }
      return FALSE;
    }
#endif
#if SPC5_CAN_USE_FLEXCAN1
    if(&CAND2 == canp) {
      for (mbid = 0; mbid < CAN_TX_MAILBOXES; mbid++) {
        if (canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES].CS.B.CODE == 0x08) {
          return TRUE;
        }
      }
      return FALSE;
    }
#endif
#if SPC5_CAN_USE_FLEXCAN2
    if(&CAND3 == canp) {
      for (mbid = 0; mbid < CAN_TX_MAILBOXES; mbid++) {
        if (canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES].CS.B.CODE == 0x08) {
          return TRUE;
        }
      }
      return FALSE;
    }
#endif
#if SPC5_CAN_USE_FLEXCAN3
    if(&CAND4 == canp) {
      for (mbid = 0; mbid < CAN_TX_MAILBOXES; mbid++) {
        if (canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES].CS.B.CODE == 0x08) {
          return TRUE;
        }
      }
      return FALSE;
    }
#endif
#if SPC5_CAN_USE_FLEXCAN4
    if(&CAND5 == canp) {
      for (mbid = 0; mbid < CAN_TX_MAILBOXES; mbid++) {
        if (canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES].CS.B.CODE == 0x08) {
          return TRUE;
        }
      }
      return FALSE;
    }
#endif
#if SPC5_CAN_USE_FLEXCAN5
    if(&CAND6 == canp) {
      for (mbid = 0; mbid < CAN_TX_MAILBOXES; mbid++) {
        if (canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES].CS.B.CODE == 0x08) {
          return TRUE;
        }
      }
      return FALSE;
    }
#endif
  }
  else {
    return canp->flexcan->BUF[mailbox + CAN_RX_MAILBOXES -1].CS.B.CODE == 0x08;
  }
  return FALSE;
}

/**
 * @brief   Inserts a frame into the transmit queue.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] ctfp      pointer to the CAN frame to be transmitted
 * @param[in] mailbox   mailbox number,  @p CAN_ANY_MAILBOX for any mailbox
 *
 * @notapi
 */
void can_lld_transmit(CANDriver *canp,
                      canmbx_t mailbox,
                      const CANTxFrame *ctfp) {

  CAN_TxMailBox_TypeDef *tmbp = NULL;
  uint8_t mbid = 0;

  /* Pointer to a free transmission mailbox.*/
  if (mailbox == CAN_ANY_MAILBOX) {
#if SPC5_CAN_USE_FLEXCAN0
    if(&CAND1 == canp) {
      for (mbid = 0; mbid < CAN_TX_MAILBOXES; mbid++) {
        if ((canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES].CS.B.CODE & 8U) != 0) {
          tmbp = (CAN_TxMailBox_TypeDef *)&canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES];
          break;
        }
      }
    }
#endif
#if SPC5_CAN_USE_FLEXCAN1
    if(&CAND2 == canp) {
      for (mbid = 0; mbid < CAN_TX_MAILBOXES; mbid++) {
        if ((canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES].CS.B.CODE & 8U) != 0 ) {
          tmbp = (CAN_TxMailBox_TypeDef *)&canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES];
          break;
        }
      }
    }
#endif
#if SPC5_CAN_USE_FLEXCAN2
    if(&CAND3 == canp) {
      for (mbid = 0; mbid < CAN_TX_MAILBOXES; mbid++) {
        if ((canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES].CS.B.CODE & 8U) != 0 ) {
          tmbp = (CAN_TxMailBox_TypeDef *)&canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES];
          break;
        }
      }
    }
#endif
#if SPC5_CAN_USE_FLEXCAN3
    if(&CAND4 == canp) {
      for (mbid = 0; mbid < CAN_TX_MAILBOXES; mbid++) {
        if ((canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES].CS.B.CODE & 8U) != 0 ) {
          tmbp = (CAN_TxMailBox_TypeDef *)&canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES];
          break;
        }
      }
    }
#endif
#if SPC5_CAN_USE_FLEXCAN4
    if(&CAND5 == canp) {
      for (mbid = 0; mbid < CAN_TX_MAILBOXES; mbid++) {
        if ((canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES].CS.B.CODE & 8U) != 0 ) {
          tmbp = (CAN_TxMailBox_TypeDef *)&canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES];
          break;
        }
      }
    }
#endif
#if SPC5_CAN_USE_FLEXCAN5
    if(&CAND6 == canp) {
      for (mbid = 0; mbid < CAN_TX_MAILBOXES; mbid++) {
        if ((canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES].CS.B.CODE & 8U) != 0 ) {
          tmbp = (CAN_TxMailBox_TypeDef *)&canp->flexcan->BUF[mbid + CAN_RX_MAILBOXES];
          break;
        }
      }
    }
#endif
  }
  else {
    tmbp = (CAN_TxMailBox_TypeDef *)&canp->flexcan->BUF[mailbox + CAN_RX_MAILBOXES -1];
  }

  /* Preparing the message.*/
  if (ctfp->IDE) {
    tmbp->CS.B.IDE = 1U;
    tmbp->CS.B.RTR = 0U;
    tmbp->ID.R = ctfp->EID;
  }
  else {
    tmbp->CS.B.IDE = 0U;
    tmbp->CS.B.RTR = 0U;
    tmbp->ID.R = ctfp->SID << 18;
  }
  tmbp->CS.B.LENGTH = ctfp->LENGTH;
  tmbp->DATA[0] = ctfp->data32[0];
  tmbp->DATA[1] = ctfp->data32[1];
  tmbp->CS.B.CODE = 0x0C;
}

/**
 *
 * @brief   Determines whether a frame has been received.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] mailbox   mailbox number, @p CAN_ANY_MAILBOX for any mailbox
 *
 * @return              The queue space availability.
 * @retval FALSE        no space in the transmit queue.
 * @retval TRUE         transmit slot available.
 *
 * @notapi
 */

bool can_lld_is_rx_nonempty(CANDriver *canp, canmbx_t mailbox) {

  uint8_t mbid = 0;
  bool mb_status = FALSE;

  if (mailbox == CAN_ANY_MAILBOX){
	for (mbid = 0; mbid < CAN_RX_MAILBOXES; mbid++) {
	  if(canp->flexcan->BUF[mbid].CS.B.CODE == 2U) {
		mb_status = TRUE;
	  }
    }
	return mb_status;
  }
  else if (mailbox >0 && mailbox <= CAN_RX_MAILBOXES){
    return (canp->flexcan->BUF[mailbox-1].CS.B.CODE == 2U);
  }
  else{
	  return FALSE;
  }
}

/**
 * @brief   Receives a frame from the input queue.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] mailbox   mailbox number, @p CAN_ANY_MAILBOX for any mailbox
 * @param[out] crfp     pointer to the buffer where the CAN frame is copied
 *
 * @notapi
 */
void can_lld_receive(CANDriver *canp,
                     canmbx_t mailbox,
                     CANRxFrame *crfp) {

  uint32_t mbid = 0, index = 0;

  if(mailbox != CAN_ANY_MAILBOX) {
    mbid = mailbox - 1;
  }
  else {
    for (index = 0; index < CAN_RX_MAILBOXES; index++) {
      if(canp->flexcan->BUF[index].CS.B.CODE == 2U) {
        mbid = index;
        break;
      }
    }
  }

  /* Lock the RX MB.*/
  (void) canp->flexcan->BUF[mbid].CS.B.CODE;

  /* Fetches the message.*/
  crfp->data32[0] = canp->flexcan->BUF[mbid].DATA.W[0];
  crfp->data32[1] = canp->flexcan->BUF[mbid].DATA.W[1];

  /* Decodes the various fields in the RX frame.*/
  crfp->RTR = canp->flexcan->BUF[mbid].CS.B.RTR;
  crfp->IDE = canp->flexcan->BUF[mbid].CS.B.IDE;
  if (crfp->IDE)
    crfp->EID = canp->flexcan->BUF[mbid].ID.R & 0x1FFFFFFF;
  else
    crfp->SID = canp->flexcan->BUF[mbid].ID.B.STD_ID;
  crfp->LENGTH = canp->flexcan->BUF[mbid].CS.B.LENGTH;
  crfp->TIME = canp->flexcan->BUF[mbid].CS.B.TIMESTAMP;

  /* Unlock the RX MB.*/
  (void) canp->flexcan->TIMER.R;

  /* Reconfigure the RX MB in empty status.*/
  canp->flexcan->BUF[mbid].CS.B.CODE = 4U;
}

#if CAN_USE_SLEEP_MODE || defined(__DOXYGEN__)
/**
 * @brief   Enters the sleep mode.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
void can_lld_sleep(CANDriver *canp) {

  /*canp->can->MCR |= CAN_MCR_SLEEP;*/
}

/**
 * @brief   Enforces leaving the sleep mode.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 *
 * @notapi
 */
void can_lld_wakeup(CANDriver *canp) {

  /*canp->can->MCR &= ~CAN_MCR_SLEEP;*/
}
#endif /* CAN_USE_SLEEP_MODE */

#endif /* HAL_USE_CAN */

/** @} */

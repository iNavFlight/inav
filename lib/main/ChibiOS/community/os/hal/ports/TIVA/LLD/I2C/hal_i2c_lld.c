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
 * @file    I2C/hal_i2c_lld.c
 * @brief   TM4C123x/TM4C129x I2C subsystem low level driver source.
 *
 * @addtogroup I2C
 * @{
 */

#include "hal.h"

#if HAL_USE_I2C || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

// interrupt states
#define STATE_IDLE          0
#define STATE_WRITE_NEXT    1
#define STATE_WRITE_FINAL   2
#define STATE_WAIT_ACK      3
#define STATE_SEND_ACK      4
#define STATE_READ_ONE      5
#define STATE_READ_FIRST    6
#define STATE_READ_NEXT     7
#define STATE_READ_FINAL    8
#define STATE_READ_WAIT     9

#define TIVA_I2C_SIGNLE_SEND                (I2C_MCS_RUN | I2C_MCS_START | I2C_MCS_STOP)
#define TIVA_I2C_BURST_SEND_START           (I2C_MCS_RUN | I2C_MCS_START)
#define TIVA_I2C_BURST_SEND_CONTINUE        (I2C_MCS_RUN)
#define TIVA_I2C_BURST_SEND_FINISH          (I2C_MCS_RUN | I2C_MCS_STOP)
#define TIVA_I2C_BURST_SEND_STOP            (I2C_MCS_STOP)
#define TIVA_I2C_BURST_SEND_ERROR_STOP      (I2C_MCS_STOP)

#define TIVA_I2C_SINGLE_RECEIVE             (I2C_MCS_RUN | I2C_MCS_START | I2C_MCS_STOP)
#define TIVA_I2C_BURST_RECEIVE_START        (I2C_MCS_RUN | I2C_MCS_START | I2C_MCS_ACK)
#define TIVA_I2C_BURST_RECEIVE_CONTINUE     (I2C_MCS_RUN | I2C_MCS_ACK)
#define TIVA_I2C_BURST_RECEIVE_FINISH       (I2C_MCS_RUN | I2C_MCS_STOP)
#define TIVA_I2C_BURST_RECEIVE_ERROR_STOP   (I2C_MCS_STOP)

#define MTPR_VALUE          ((TIVA_SYSCLK/(2*(6+4)*i2cp->config->clock_speed))-1)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief I2C0 driver identifier.
 */
#if TIVA_I2C_USE_I2C0 || defined(__DOXYGEN__)
I2CDriver I2CD1;
#endif

/**
 * @brief I2C1 driver identifier.
 */
#if TIVA_I2C_USE_I2C1 || defined(__DOXYGEN__)
I2CDriver I2CD2;
#endif

/**
 * @brief I2C2 driver identifier.
 */
#if TIVA_I2C_USE_I2C2 || defined(__DOXYGEN__)
I2CDriver I2CD3;
#endif

/**
 * @brief I2C3 driver identifier.
 */
#if TIVA_I2C_USE_I2C3 || defined(__DOXYGEN__)
I2CDriver I2CD4;
#endif

/**
 * @brief I2C4 driver identifier.
 */
#if TIVA_I2C_USE_I2C4 || defined(__DOXYGEN__)
I2CDriver I2CD5;
#endif

/**
 * @brief I2C5 driver identifier.
 */
#if TIVA_I2C_USE_I2C5 || defined(__DOXYGEN__)
I2CDriver I2CD6;
#endif

/**
 * @brief I2C6 driver identifier.
 */
#if TIVA_I2C_USE_I2C6 || defined(__DOXYGEN__)
I2CDriver I2CD7;
#endif

/**
 * @brief I2C7 driver identifier.
 */
#if TIVA_I2C_USE_I2C7 || defined(__DOXYGEN__)
I2CDriver I2CD8;
#endif

/**
 * @brief I2C8 driver identifier.
 */
#if TIVA_I2C_USE_I2C8 || defined(__DOXYGEN__)
I2CDriver I2CD9;
#endif

/**
 * @brief I2C9 driver identifier.
 */
#if TIVA_I2C_USE_I2C9 || defined(__DOXYGEN__)
I2CDriver I2CD10;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   I2C shared ISR code.
 *
 * @param[in] i2cp  pointer to the @p I2CDriver object
 *
 * @notapi
 */
static void i2c_lld_serve_interrupt(I2CDriver *i2cp)
{
  uint32_t i2c = i2cp->i2c;
  uint32_t status;

  // clear MIS bit in MICR by writing 1
  HWREG(i2c + I2C_O_MICR) = 1;

  // read interrupt status
  status = HWREG(i2c + I2C_O_MCS);

  if (status & I2C_MCS_ERROR) {
    i2cp->errors |= I2C_BUS_ERROR;
  }
  if (status & I2C_MCS_ARBLST) {
    i2cp->errors |= I2C_ARBITRATION_LOST;
  }

  if (i2cp->errors == I2C_NO_ERROR) {
    // no error detected
    switch(i2cp->intstate) {
      case STATE_IDLE: {
        _i2c_wakeup_isr(i2cp);
        break;
      }
      case STATE_WRITE_NEXT: {
        if (i2cp->txbytes == 1) {
          i2cp->intstate = STATE_WRITE_FINAL;
        }
        HWREG(i2c + I2C_O_MDR) = *(i2cp->txbuf);
        i2cp->txbuf++;
        i2cp->txbytes--;
        // start transmission
        HWREG(i2c + I2C_O_MCS) = TIVA_I2C_BURST_SEND_CONTINUE;
        break;
      }
      case STATE_WRITE_FINAL: {
        if (i2cp->rxbytes == 0) {
          i2cp->intstate = STATE_IDLE;
        }
        else if (i2cp->rxbytes == 1) {
          i2cp->intstate = STATE_READ_ONE;
        }
        else {
          i2cp->intstate = STATE_READ_FIRST;
        }
        HWREG(i2c + I2C_O_MDR) = *(i2cp->txbuf);
        i2cp->txbuf++;
        // txbytes - 1
        i2cp->txbytes--;
        // start transmission
        HWREG(i2c + I2C_O_MCS) = TIVA_I2C_BURST_SEND_FINISH;
        break;
      }
      case STATE_WAIT_ACK: {
        break;
      }
      case STATE_SEND_ACK: {
        break;
      }
      case STATE_READ_ONE: {
        i2cp->intstate = STATE_READ_WAIT;
        // Initializes driver fields, LSB = 1 -> read.
         i2cp->addr |= 1;

         // set slave address
         HWREG(i2c + I2C_O_MSA) = i2cp->addr;
         i2cp->rxbytes--;
         //start receiving
         HWREG(i2c + I2C_O_MCS) = TIVA_I2C_SINGLE_RECEIVE;

        break;
      }
      case STATE_READ_FIRST: {
        if (i2cp->rxbytes == 2) {
          i2cp->intstate = STATE_READ_FINAL;
        }
        else {
          i2cp->intstate = STATE_READ_NEXT;
        }

        // Initializes driver fields, LSB = 1 -> read.
         i2cp->addr |= 1;

         // set slave address
         HWREG(i2c + I2C_O_MSA) = i2cp->addr;
         i2cp->rxbytes--;
         //start receiving
         HWREG(i2c + I2C_O_MCS) = TIVA_I2C_BURST_RECEIVE_START;

        break;
      }
      case STATE_READ_NEXT: {
        if(i2cp->rxbytes == 2) {
          i2cp->intstate = STATE_READ_FINAL;
        }
        *(i2cp->rxbuf) = HWREG(i2c + I2C_O_MDR);
        i2cp->rxbuf++;
        i2cp->rxbytes--;
        //start receiving
        HWREG(i2c + I2C_O_MCS) = TIVA_I2C_BURST_RECEIVE_CONTINUE;

        break;
      }
      case STATE_READ_FINAL: {
        i2cp->intstate = STATE_READ_WAIT;
        *(i2cp->rxbuf) = HWREG(i2c + I2C_O_MDR);
        i2cp->rxbuf++;
        i2cp->rxbytes--;
        //start receiving
        HWREG(i2c + I2C_O_MCS) = TIVA_I2C_BURST_RECEIVE_FINISH;

        break;
      }
      case STATE_READ_WAIT: {
        i2cp->intstate = STATE_IDLE;
        *(i2cp->rxbuf) = HWREG(i2c + I2C_O_MDR);
        i2cp->rxbuf++;
        _i2c_wakeup_isr(i2cp);
        break;
      }
    }
  }
  else {
    // error detected
    _i2c_wakeup_error_isr(i2cp);
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if TIVA_I2C_USE_I2C0 || defined(__DOXYGEN__)
/**
 * @brief   I2C0 interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(TIVA_I2C0_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  i2c_lld_serve_interrupt(&I2CD1);

  OSAL_IRQ_EPILOGUE();
}
#endif /* TIVA_I2C_USE_I2C0 */

#if TIVA_I2C_USE_I2C1 || defined(__DOXYGEN__)
/**
 * @brief   I2C1 interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(TIVA_I2C1_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  i2c_lld_serve_interrupt(&I2CD2);

  OSAL_IRQ_EPILOGUE();
}
#endif /* TIVA_I2C_USE_I2C1 */

#if TIVA_I2C_USE_I2C2 || defined(__DOXYGEN__)
/**
 * @brief   I2C2 interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(TIVA_I2C2_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  i2c_lld_serve_interrupt(&I2CD3);

  OSAL_IRQ_EPILOGUE();
}
#endif /* TIVA_I2C_USE_I2C2 */

#if TIVA_I2C_USE_I2C3 || defined(__DOXYGEN__)
/**
 * @brief   I2C3 interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(TIVA_I2C3_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  i2c_lld_serve_interrupt(&I2CD4);

  OSAL_IRQ_EPILOGUE();
}
#endif /* TIVA_I2C_USE_I2C3 */

#if TIVA_I2C_USE_I2C4 || defined(__DOXYGEN__)
/**
 * @brief   I2C4 interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(TIVA_I2C4_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  i2c_lld_serve_interrupt(&I2CD5);

  OSAL_IRQ_EPILOGUE();
}
#endif /* TIVA_I2C_USE_I2C4 */

#if TIVA_I2C_USE_I2C5 || defined(__DOXYGEN__)
/**
 * @brief   I2C5 interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(TIVA_I2C5_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  i2c_lld_serve_interrupt(&I2CD6);

  OSAL_IRQ_EPILOGUE();
}
#endif /* TIVA_I2C_USE_I2C5 */

#if TIVA_I2C_USE_I2C6 || defined(__DOXYGEN__)
/**
 * @brief   I2C6 interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(TIVA_I2C6_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  i2c_lld_serve_interrupt(&I2CD7);

  OSAL_IRQ_EPILOGUE();
}
#endif /* TIVA_I2C_USE_I2C6 */

#if TIVA_I2C_USE_I2C7 || defined(__DOXYGEN__)
/**
 * @brief   I2C7 interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(TIVA_I2C7_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  i2c_lld_serve_interrupt(&I2CD8);

  OSAL_IRQ_EPILOGUE();
}
#endif /* TIVA_I2C_USE_I2C7 */

#if TIVA_I2C_USE_I2C8 || defined(__DOXYGEN__)
/**
 * @brief   I2C8 interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(TIVA_I2C8_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  i2c_lld_serve_interrupt(&I2CD9);

  OSAL_IRQ_EPILOGUE();
}
#endif /* TIVA_I2C_USE_I2C8 */

#if TIVA_I2C_USE_I2C9 || defined(__DOXYGEN__)
/**
 * @brief   I2C9 interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(TIVA_I2C9_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  i2c_lld_serve_interrupt(&I2CD10);

  OSAL_IRQ_EPILOGUE();
}
#endif /* TIVA_I2C_USE_I2C9 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level I2C driver initialization.
 *
 * @notapi
 */
void i2c_lld_init(void) {

#if TIVA_I2C_USE_I2C0
  i2cObjectInit(&I2CD1);
  I2CD1.thread = NULL;
  I2CD1.i2c    = I2C0_BASE;
#endif /* TIVA_I2C_USE_I2C0 */

#if TIVA_I2C_USE_I2C1
  i2cObjectInit(&I2CD2);
  I2CD2.thread = NULL;
  I2CD2.i2c    = I2C1_BASE;
#endif /* TIVA_I2C_USE_I2C1 */

#if TIVA_I2C_USE_I2C2
  i2cObjectInit(&I2CD3);
  I2CD3.thread = NULL;
  I2CD3.i2c    = I2C2_BASE;
#endif /* TIVA_I2C_USE_I2C2 */

#if TIVA_I2C_USE_I2C3
  i2cObjectInit(&I2CD4);
  I2CD4.thread = NULL;
  I2CD4.i2c    = I2C3_BASE;
#endif /* TIVA_I2C_USE_I2C3 */

#if TIVA_I2C_USE_I2C4
  i2cObjectInit(&I2CD5);
  I2CD5.thread = NULL;
  I2CD5.i2c    = I2C4_BASE;
#endif /* TIVA_I2C_USE_I2C4 */

#if TIVA_I2C_USE_I2C5
  i2cObjectInit(&I2CD6);
  I2CD6.thread = NULL;
  I2CD6.i2c    = I2C5_BASE;
#endif /* TIVA_I2C_USE_I2C5 */

#if TIVA_I2C_USE_I2C6
  i2cObjectInit(&I2CD7);
  I2CD7.thread = NULL;
  I2CD7.i2c    = I2C6_BASE;
#endif /* TIVA_I2C_USE_I2C6 */

#if TIVA_I2C_USE_I2C7
  i2cObjectInit(&I2CD8);
  I2CD8.thread = NULL;
  I2CD8.i2c    = I2C7_BASE;
#endif /* TIVA_I2C_USE_I2C7 */

#if TIVA_I2C_USE_I2C8
  i2cObjectInit(&I2CD9);
  I2CD9.thread = NULL;
  I2CD9.i2c    = I2C8_BASE;
#endif /* TIVA_I2C_USE_I2C8 */

#if TIVA_I2C_USE_I2C9
  i2cObjectInit(&I2CD10);
  I2CD10.thread = NULL;
  I2CD10.i2c    = I2C9_BASE;
#endif /* TIVA_I2C_USE_I2C9 */
}

/**
 * @brief   Configures and activates the I2C peripheral.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
void i2c_lld_start(I2CDriver *i2cp)
{
  uint32_t i2c = i2cp->i2c;

  /* If in stopped state then enables the I2C clocks.*/
  if (i2cp->state == I2C_STOP) {
#if TIVA_I2C_USE_I2C0
    if (&I2CD1 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) |= (1 << 0);

      while (!(HWREG(SYSCTL_PRI2C) & (1 << 0)))
        ;

      nvicEnableVector(TIVA_I2C0_NUMBER, TIVA_I2C_I2C0_IRQ_PRIORITY);
    }
#endif /* TIVA_I2C_USE_I2C0 */

#if TIVA_I2C_USE_I2C1
    if (&I2CD2 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) |= (1 << 1);

      while (!(HWREG(SYSCTL_PRI2C) & (1 << 1)))
        ;

      nvicEnableVector(TIVA_I2C1_NUMBER, TIVA_I2C_I2C1_IRQ_PRIORITY);
    }
#endif /* TIVA_I2C_USE_I2C1 */

#if TIVA_I2C_USE_I2C2
    if (&I2CD3 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) |= (1 << 2);

      while (!(HWREG(SYSCTL_PRI2C) & (1 << 2)))
        ;

      nvicEnableVector(TIVA_I2C2_NUMBER, TIVA_I2C_I2C2_IRQ_PRIORITY);
    }
#endif /* TIVA_I2C_USE_I2C2 */

#if TIVA_I2C_USE_I2C3
    if (&I2CD4 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) |= (1 << 3);

      while (!(HWREG(SYSCTL_PRI2C) & (1 << 3)))
        ;

      nvicEnableVector(TIVA_I2C3_NUMBER, TIVA_I2C_I2C3_IRQ_PRIORITY);
    }
#endif /* TIVA_I2C_USE_I2C3 */

#if TIVA_I2C_USE_I2C4
    if (&I2CD5 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) |= (1 << 4);

      while (!(HWREG(SYSCTL_PRI2C) & (1 << 4)))
        ;

      nvicEnableVector(TIVA_I2C4_NUMBER, TIVA_I2C_I2C4_IRQ_PRIORITY);
    }
#endif /* TIVA_I2C_USE_I2C4 */

#if TIVA_I2C_USE_I2C5
    if (&I2CD6 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) |= (1 << 5);

      while (!(HWREG(SYSCTL_PRI2C) & (1 << 5)))
        ;

      nvicEnableVector(TIVA_I2C5_NUMBER, TIVA_I2C_I2C5_IRQ_PRIORITY);
    }
#endif /* TIVA_I2C_USE_I2C5 */

#if TIVA_I2C_USE_I2C6
    if (&I2CD7 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) |= (1 << 6);

      while (!(HWREG(SYSCTL_PRI2C) & (1 << 6)))
        ;

      nvicEnableVector(TIVA_I2C6_NUMBER, TIVA_I2C_I2C6_IRQ_PRIORITY);
    }
#endif /* TIVA_I2C_USE_I2C6 */

#if TIVA_I2C_USE_I2C7
    if (&I2CD8 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) |= (1 << 7);

      while (!(HWREG(SYSCTL_PRI2C) & (1 << 7)))
        ;

      nvicEnableVector(TIVA_I2C7_NUMBER, TIVA_I2C_I2C7_IRQ_PRIORITY);
    }
#endif /* TIVA_I2C_USE_I2C7 */

#if TIVA_I2C_USE_I2C8
    if (&I2CD9 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) |= (1 << 8);

      while (!(HWREG(SYSCTL_PRI2C) & (1 << 8)))
        ;

      nvicEnableVector(TIVA_I2C8_NUMBER, TIVA_I2C_I2C8_IRQ_PRIORITY);
    }
#endif /* TIVA_I2C_USE_I2C7 */

#if TIVA_I2C_USE_I2C9
    if (&I2CD10 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) |= (1 << 9);

      while (!(HWREG(SYSCTL_PRI2C) & (1 << 9)))
        ;

      nvicEnableVector(TIVA_I2C9_NUMBER, TIVA_I2C_I2C9_IRQ_PRIORITY);
    }
#endif /* TIVA_I2C_USE_I2C7 */
  }

  HWREG(i2c + I2C_O_MCR) = 0x10;
  HWREG(i2c + I2C_O_MTPR) = MTPR_VALUE;
}

/**
 * @brief   Deactivates the I2C peripheral.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
void i2c_lld_stop(I2CDriver *i2cp)
{
  uint32_t i2c = i2cp->i2c;

  /* If not in stopped state then disables the I2C clock.*/
  if (i2cp->state != I2C_STOP) {

    /* I2C disable.*/
    // TODO: abort i2c operation
    //i2c_lld_abort_operation(i2cp);

#if TIVA_I2C_USE_I2C0
    if (&I2CD1 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) &= ~(1 << 0);
      nvicDisableVector(TIVA_I2C0_NUMBER);
    }
#endif /* TIVA_I2C_USE_I2C0 */

#if TIVA_I2C_USE_I2C1
    if (&I2CD2 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) &= ~(1 << 1);
      nvicDisableVector(TIVA_I2C1_NUMBER);
    }
#endif /* TIVA_I2C_USE_I2C1 */

#if TIVA_I2C_USE_I2C2
    if (&I2CD3 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) &= ~(1 << 2);
      nvicDisableVector(TIVA_I2C2_NUMBER);
    }
#endif /* TIVA_I2C_USE_I2C2 */

#if TIVA_I2C_USE_I2C3
    if (&I2CD4 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) &= ~(1 << 3);
      nvicDisableVector(TIVA_I2C3_NUMBER);
    }
#endif /* TIVA_I2C_USE_I2C3 */

#if TIVA_I2C_USE_I2C4
    if (&I2CD5 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) &= ~(1 << 4);
      nvicDisableVector(TIVA_I2C4_NUMBER);
    }
#endif /* TIVA_I2C_USE_I2C4 */

#if TIVA_I2C_USE_I2C5
    if (&I2CD6 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) &= ~(1 << 5);
      nvicDisableVector(TIVA_I2C5_NUMBER);
    }
#endif /* TIVA_I2C_USE_I2C5 */

#if TIVA_I2C_USE_I2C6
    if (&I2CD7 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) &= ~(1 << 6);
      nvicDisableVector(TIVA_I2C6_NUMBER);
    }
#endif /* TIVA_I2C_USE_I2C6 */

#if TIVA_I2C_USE_I2C7
    if (&I2CD8 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) &= ~(1 << 7);
      nvicDisableVector(TIVA_I2C7_NUMBER);
    }
#endif /* TIVA_I2C_USE_I2C7 */

#if TIVA_I2C_USE_I2C8
    if (&I2CD9 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) &= ~(1 << 8);
      nvicDisableVector(TIVA_I2C8_NUMBER);
    }
#endif /* TIVA_I2C_USE_I2C8 */

#if TIVA_I2C_USE_I2C9
    if (&I2CD10 == i2cp) {
      HWREG(SYSCTL_RCGCI2C) &= ~(1 << 9);
      nvicDisableVector(TIVA_I2C9_NUMBER);
    }
#endif /* TIVA_I2C_USE_I2C9 */

  HWREG(i2c + I2C_O_MCR) = 0;
  HWREG(i2c + I2C_O_MTPR) = 0;
  }
}

/**
 * @brief   Receives data via the I2C bus as master.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] addr      slave device address
 * @param[out] rxbuf    pointer to the receive buffer
 * @param[in] rxbytes   number of bytes to be received
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The operation status.
 * @retval RDY_OK       if the function succeeded.
 * @retval RDY_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval RDY_TIMEOUT  if a timeout occurred before operation end. <b>After a
 *                      timeout the driver must be stopped and restarted
 *                      because the bus is in an uncertain state</b>.
 *
 * @notapi
 */
msg_t i2c_lld_master_receive_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                     uint8_t *rxbuf, size_t rxbytes,
                                     systime_t timeout)
{
  uint32_t i2c = i2cp->i2c;
  systime_t start, end;

  i2cp->rxbuf = rxbuf;
  i2cp->rxbytes = rxbytes;

  /* Resetting error flags for this transfer.*/
  i2cp->errors = I2C_NO_ERROR;

  /* Initializes driver fields, LSB = 1 -> receive.*/
  i2cp->addr = (addr << 1) | 0x01;

  /* Releases the lock from high level driver.*/
  osalSysUnlock();

  /* Calculating the time window for the timeout on the busy bus condition.*/
  start = osalOsGetSystemTimeX();
  end = start + OSAL_MS2I(TIVA_I2C_BUSY_TIMEOUT);

  /* Waits until BUSY flag is reset or, alternatively, for a timeout
     condition.*/
  while (true) {
    osalSysLock();

    /* If the bus is not busy then the operation can continue, note, the
       loop is exited in the locked state.*/
    if ((HWREG(i2c + I2C_O_MCS) & I2C_MCS_BUSY) == 0)
      break;

    /* If the system time went outside the allowed window then a timeout
       condition is returned.*/
    if (!osalTimeIsInRangeX(osalOsGetSystemTimeX(), start, end))
      return MSG_TIMEOUT;

    osalSysUnlock();
  }

  /* set slave address */
  HWREG(i2c + I2C_O_MSA) = addr;

  /* Starts the operation.*/
  HWREG(i2c + I2C_O_MCS) = TIVA_I2C_SINGLE_RECEIVE;

  /* Waits for the operation completion or a timeout.*/
  return osalThreadSuspendTimeoutS(&i2cp->thread, timeout);
}

/**
 * @brief   Transmits data via the I2C bus as master.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] addr      slave device address
 * @param[in] txbuf     pointer to the transmit buffer
 * @param[in] txbytes   number of bytes to be transmitted
 * @param[out] rxbuf    pointer to the receive buffer
 * @param[in] rxbytes   number of bytes to be received
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The operation status.
 * @retval RDY_OK       if the function succeeded.
 * @retval RDY_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval RDY_TIMEOUT  if a timeout occurred before operation end. <b>After a
 *                      timeout the driver must be stopped and restarted
 *                      because the bus is in an uncertain state</b>.
 *
 * @notapi
 */
msg_t i2c_lld_master_transmit_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                      const uint8_t *txbuf, size_t txbytes,
                                      uint8_t *rxbuf, size_t rxbytes,
                                      systime_t timeout)
{
  uint32_t i2c = i2cp->i2c;
  systime_t start, end;

  i2cp->rxbuf = rxbuf;
  i2cp->rxbytes = rxbytes;
  i2cp->txbuf = txbuf;
  i2cp->txbytes = txbytes;

  /* Resetting error flags for this transfer.*/
  i2cp->errors = I2C_NO_ERROR;

  /* Releases the lock from high level driver.*/
  osalSysUnlock();

  /* Calculating the time window for the timeout on the busy bus condition.*/
  start = osalOsGetSystemTimeX();
  end = start + OSAL_MS2I(TIVA_I2C_BUSY_TIMEOUT);

  /* Waits until BUSY flag is reset or, alternatively, for a timeout
     condition.*/
  while (true) {
    osalSysLock();

    /* If the bus is not busy then the operation can continue, note, the
       loop is exited in the locked state.*/
    if ((HWREG(i2c + I2C_O_MCS) & I2C_MCS_BUSY) == 0)
      break;

    /* If the system time went outside the allowed window then a timeout
       condition is returned.*/
    if (!osalTimeIsInRangeX(osalOsGetSystemTimeX(), start, end))

      return MSG_TIMEOUT;

    osalSysUnlock();
  }

  /* Initializes driver fields, LSB = 0 -> write.*/
  i2cp->addr = addr << 1 | 0;

  /* set slave address */
  HWREG(i2c + I2C_O_MSA) = i2cp->addr;

  /* enable interrupts */
  HWREG(i2c + I2C_O_MIMR) = I2C_MIMR_IM;

  /* put data in register */
  HWREG(i2c + I2C_O_MDR) = *(i2cp->txbuf);

  /* check if 1 or more bytes */
  if (i2cp->txbytes == 1) {
    if (i2cp->rxbytes == 1) {
      // one byte read
      i2cp->intstate = STATE_READ_ONE;
    }
    else {
      // multiple byte read
      i2cp->intstate = STATE_READ_FIRST;
    }
    // single byte send
    HWREG(i2c + I2C_O_MCS) = TIVA_I2C_SIGNLE_SEND;
  }
  else {
    if (i2cp->txbytes == 2) {
      // 2 bytes
      i2cp->intstate = STATE_WRITE_FINAL;
    }
    else {
      // more then 2 bytes
      i2cp->intstate = STATE_WRITE_NEXT;
    }
    // multiple bytes start send
    HWREG(i2c + I2C_O_MCS) = TIVA_I2C_BURST_SEND_START;
  }

  i2cp->txbuf++;
  i2cp->txbytes--;

  /* Waits for the operation completion or a timeout.*/
  return osalThreadSuspendTimeoutS(&i2cp->thread, timeout);
}

#endif /* HAL_USE_I2C */

/** @} */

/*
    ChibiOS - Copyright (C) 2006..2019 Giovanni Di Sirio
              Copyright (C) 2019 Fabien Poussin (fabien.poussin (at) google's mail)
              Copyright (C) 2019 Alexandre Bustico

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
 * @file    STM32/hal_opamp_lld.c
 * @brief   STM32 Operational Amplifier subsystem low level driver header.
 *
 * @addtogroup OPAMP
 * @{
 */

#include "hal.h"

#if HAL_USE_OPAMP || defined(__DOXYGEN__)

#include "hal_opamp.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   OPAMPD1 driver identifier.
 * @note    The driver OPAMPD1 allocates the comparator OPAMP1 when enabled.
 */
#if STM32_OPAMP_USE_OPAMP1 || defined(__DOXYGEN__)
OPAMPDriver OPAMPD1;
#endif

/**
 * @brief   OPAMPD2 driver identifier.
 * @note    The driver OPAMPD2 allocates the comparator OPAMP2 when enabled.
 */
#if STM32_OPAMP_USE_OPAMP2 || defined(__DOXYGEN__)
OPAMPDriver OPAMPD2;
#endif

/**
 * @brief   OPAMPD3 driver identifier.
 * @note    The driver OPAMPD3 allocates the comparator OPAMP3 when enabled.
 */
#if STM32_OPAMP_USE_OPAMP3 || defined(__DOXYGEN__)
OPAMPDriver OPAMPD3;
#endif

/**
 * @brief   OPAMPD4 driver identifier.
 * @note    The driver OPAMPD4 allocates the comparator OPAMP4 when enabled.
 */
#if STM32_OPAMP_USE_OPAMP4 || defined(__DOXYGEN__)
OPAMPDriver OPAMPD4;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/


/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/


/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level OPAMP driver initialization.
 *
 * @notapi
 */
void opamp_lld_init(void) {

#if STM32_OPAMP_USE_OPAMP1
  /* Driver initialization.*/
  opampObjectInit(&OPAMPD1);
  OPAMPD1.opamp = OPAMP;
  OPAMPD1.opamp->CSR = 0;
#endif

#if STM32_OPAMP_USE_OPAMP2
  /* Driver initialization.*/
  opampObjectInit(&OPAMPD2);
  OPAMPD2.opamp = OPAMP2;
  OPAMPD2.opamp->CSR = 0;
#endif

#if STM32_OPAMP_USE_OPAMP3
  /* Driver initialization.*/
  opampObjectInit(&OPAMPD3);
  OPAMPD3.opamp = OPAMP3;
  OPAMPD3.opamp->CSR = 0;
#endif

#if STM32_OPAMP_USE_OPAMP4
  /* Driver initialization.*/
  opampObjectInit(&OPAMPD4);
  OPAMPD4.opamp = OPAMP4;
  OPAMPD4.opamp->CSR = 0;
#endif

}

/**
 * @brief   Configures and activates the OPAMP peripheral.
 *
 * @param[in] opampp      pointer to the @p OPAMPDriver object
 *
 * @notapi
 */
void opamp_lld_start(OPAMPDriver *opampp) {

  // Apply CSR Execpt the enable bit.
  opampp->opamp->CSR = opampp->config->csr & ~OPAMP_CSR_OPAMPxEN;

}

/**
 * @brief   Deactivates the comp peripheral.
 *
 * @param[in] opampp      pointer to the @p OPAMPDriver object
 *
 * @notapi
 */
void opamp_lld_stop(OPAMPDriver *opampp) {

  if (opampp->state == OPAMP_ACTIVE) {

    opampp->opamp->CSR = 0;
  }

}

/**
 * @brief   Enables the output.
 *
 * @param[in] opampp      pointer to the @p OPAMPDriver object
 *
 * @notapi
 */
void opamp_lld_enable(OPAMPDriver *opampp) {

   opampp->opamp->CSR |= OPAMP_CSR_OPAMPxEN; /* Enable */
}

/**
 * @brief   Disables the output.
 *
 * @param[in] opampp      pointer to the @p OPAMPDriver object
 *
 * @notapi
 */
void opamp_lld_disable(OPAMPDriver *opampp) {

  opampp->opamp->CSR &= ~OPAMP_CSR_OPAMPxEN; /* Disable */
}

#if STM32_OPAMP_USER_TRIM_ENABLED

void opamp_lld_calibrate_once(void)
{
#if STM32_OPAMP_USE_OPAMP1
  uint32_t trimmingvaluen1 = 16U;
  uint32_t trimmingvaluep1 = 16U;
  OPAMPD1.state = OPAMP_CALIBRATING;
#define CSRm OPAMPD1.opamp->CSR
  /* Set Calibration mode */
  /* Non-inverting input connected to calibration reference voltage. */
  CSRm |= OPAMP_CSR_FORCEVP;
  /*  user trimming values are used for offset calibration */
  CSRm |= OPAMP_CSR_USERTRIM;
  /* Enable calibration */
  CSRm |= OPAMP_CSR_CALON;
  /* 1st calibration - N  Select 90U% VREF */
  MODIFY_REG(CSRm, OPAMP_CSR_CALSEL, OPAMPx_CSR_CALSEL_90P);
  /* Enable the opamps */
  CSRm |= OPAMP_CSR_OPAMPxEN;
#undef CSRm
#endif

#if STM32_OPAMP_USE_OPAMP2
  uint32_t trimmingvaluen2 = 16U;
  uint32_t trimmingvaluep2 = 16U;
  OPAMPD2.state = OPAMP_CALIBRATING;
#define CSRm OPAMPD2.opamp->CSR
  CSRm |= OPAMP_CSR_FORCEVP;
  CSRm |= OPAMP_CSR_USERTRIM;
  CSRm |= OPAMP_CSR_CALON;
  MODIFY_REG(CSRm, OPAMP_CSR_CALSEL, OPAMPx_CSR_CALSEL_90P);
  CSRm |= OPAMP_CSR_OPAMPxEN;
#undef CSRm
#endif

#if STM32_OPAMP_USE_OPAMP3
  uint32_t trimmingvaluen3 = 16U;
  uint32_t trimmingvaluep3 = 16U;
  OPAMPD3.state = OPAMP_CALIBRATING;
#define CSRm OPAMPD3.opamp->CSR
  CSRm |= OPAMP_CSR_FORCEVP;
  CSRm |= OPAMP_CSR_USERTRIM;
  CSRm |= OPAMP_CSR_CALON;
  MODIFY_REG(CSRm, OPAMP_CSR_CALSEL, OPAMPx_CSR_CALSEL_90P);
  CSRm |= OPAMP_CSR_OPAMPxEN;
#undef CSRm
#endif

#if STM32_OPAMP_USE_OPAMP4
  uint32_t trimmingvaluen4 = 16U;
  uint32_t trimmingvaluep4 = 16U;
  OPAMPD4.state = OPAMP_CALIBRATING;
#define CSRm OPAMPD4.opamp->CSR
  CSRm |= OPAMP_CSR_FORCEVP;
  CSRm |= OPAMP_CSR_USERTRIM;
  CSRm |= OPAMP_CSR_CALON;
  MODIFY_REG(CSRm, OPAMP_CSR_CALSEL, OPAMPx_CSR_CALSEL_90P);
  CSRm |= OPAMP_CSR_OPAMPxEN;
#undef CSRm
#endif

  chSysPolledDelayX(MS2RTC(STM32_SYSCLK, 20));
  uint32_t delta = 8U;

  while (delta != 0U) {
#if STM32_OPAMP_USE_OPAMP1
    MODIFY_REG(OPAMPD1.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
         trimmingvaluen1<<OPAMP_CSR_TRIMOFFSETN_Pos);
#endif
#if STM32_OPAMP_USE_OPAMP2
    MODIFY_REG(OPAMPD2.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
         trimmingvaluen2<<OPAMP_CSR_TRIMOFFSETN_Pos);
#endif
#if STM32_OPAMP_USE_OPAMP3
    MODIFY_REG(OPAMPD3.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
         trimmingvaluen3<<OPAMP_CSR_TRIMOFFSETN_Pos);
#endif
#if STM32_OPAMP_USE_OPAMP4
    MODIFY_REG(OPAMPD4.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
         trimmingvaluen4<<OPAMP_CSR_TRIMOFFSETN_Pos);
#endif

    /* OFFTRIMmax delay 2 ms as per datasheet (electrical characteristics */
    /* Offset trim time: during calibration, minimum time needed between */
    /* two steps to have 1 mV accuracy */
    chSysPolledDelayX(MS2RTC(STM32_SYSCLK, 2));

#if STM32_OPAMP_USE_OPAMP1
    if (OPAMPD1.opamp->CSR & OPAMP_CSR_OUTCAL)   {
      /* OPAMP_CSR_OUTCAL is HIGH try higher trimming */
      trimmingvaluen1 += delta;
    } else  {
      /* OPAMP_CSR_OUTCAL is LOW try lower trimming */
      trimmingvaluen1 -= delta;
    }
#endif

#if STM32_OPAMP_USE_OPAMP2
    if (OPAMPD2.opamp->CSR & OPAMP_CSR_OUTCAL)    {
      trimmingvaluen2 += delta;
    } else {
      trimmingvaluen2 -= delta;
    }
#endif

#if STM32_OPAMP_USE_OPAMP3
    if (OPAMPD3.opamp->CSR & OPAMP_CSR_OUTCAL)    {
      trimmingvaluen3 += delta;
    } else {
      trimmingvaluen3 -= delta;
    }
#endif

#if STM32_OPAMP_USE_OPAMP4
    if (OPAMPD4.opamp->CSR & OPAMP_CSR_OUTCAL)    {
      trimmingvaluen4 += delta;
    }  else {
      trimmingvaluen4 -= delta;
    }
#endif

    delta >>= 1U;

  }

  /* Still need to check if righ calibration is current value or un step below */
  /* Indeed the first value that causes the OUTCAL bit to change from 1 to 0U */
#if STM32_OPAMP_USE_OPAMP1
  MODIFY_REG(OPAMPD1.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
       trimmingvaluen1<<OPAMP_CSR_TRIMOFFSETN_Pos);
#endif
#if STM32_OPAMP_USE_OPAMP2
  MODIFY_REG(OPAMPD2.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
       trimmingvaluen2<<OPAMP_CSR_TRIMOFFSETN_Pos);
#endif
#if STM32_OPAMP_USE_OPAMP3
  MODIFY_REG(OPAMPD3.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
       trimmingvaluen3<<OPAMP_CSR_TRIMOFFSETN_Pos);
#endif
#if STM32_OPAMP_USE_OPAMP4
  MODIFY_REG(OPAMPD4.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
       trimmingvaluen4<<OPAMP_CSR_TRIMOFFSETN_Pos);
#endif

  /* OFFTRIMmax delay 2 ms as per datasheet (electrical characteristics */
  /* Offset trim time: during calibration, minimum time needed between */
  /* two steps to have 1 mV accuracy */
  chSysPolledDelayX(MS2RTC(STM32_SYSCLK, 2));

#if STM32_OPAMP_USE_OPAMP1
    if (OPAMPD1.opamp->CSR & OPAMP_CSR_OUTCAL)   {
      /* OPAMP_CSR_OUTCAL is HIGH try higher trimming */
      trimmingvaluen1  += (trimmingvaluen1 != 31);
      MODIFY_REG(OPAMPD1.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
     trimmingvaluen1<<OPAMP_CSR_TRIMOFFSETN_Pos);
    }
#endif

#if STM32_OPAMP_USE_OPAMP2
    if (OPAMPD2.opamp->CSR & OPAMP_CSR_OUTCAL)    {
      trimmingvaluen2  += (trimmingvaluen2 != 31);
      MODIFY_REG(OPAMPD2.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
     trimmingvaluen2<<OPAMP_CSR_TRIMOFFSETN_Pos);
    }
#endif

#if STM32_OPAMP_USE_OPAMP3
    if (OPAMPD3.opamp->CSR & OPAMP_CSR_OUTCAL)    {
      trimmingvaluen3  += (trimmingvaluen3 != 31);
      MODIFY_REG(OPAMPD3.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
     trimmingvaluen3<<OPAMP_CSR_TRIMOFFSETN_Pos);
    }
#endif

#if STM32_OPAMP_USE_OPAMP4
    if (OPAMPD4.opamp->CSR & OPAMP_CSR_OUTCAL)    {
      trimmingvaluen4  += (trimmingvaluen4 != 31);
      MODIFY_REG(OPAMPD4.opamp->CSR, OPAMP_CSR_TRIMOFFSETN,
     trimmingvaluen4<<OPAMP_CSR_TRIMOFFSETN_Pos);
    }
#endif

    /* 2nd calibration - P */
    /* Select 10U% VREF */
#if STM32_OPAMP_USE_OPAMP1
  MODIFY_REG(OPAMPD1.opamp->CSR, OPAMP_CSR_CALSEL, OPAMPx_CSR_CALSEL_10P);
#endif
#if STM32_OPAMP_USE_OPAMP2
  MODIFY_REG(OPAMPD2.opamp->CSR, OPAMP_CSR_CALSEL, OPAMPx_CSR_CALSEL_10P);
#endif
#if STM32_OPAMP_USE_OPAMP3
  MODIFY_REG(OPAMPD3.opamp->CSR, OPAMP_CSR_CALSEL, OPAMPx_CSR_CALSEL_10P);
#endif
#if STM32_OPAMP_USE_OPAMP4
  MODIFY_REG(OPAMPD4.opamp->CSR, OPAMP_CSR_CALSEL, OPAMPx_CSR_CALSEL_10P);
#endif

  delta = 8U;

  while (delta != 0U)   {
#if STM32_OPAMP_USE_OPAMP1
    MODIFY_REG(OPAMPD1.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
         trimmingvaluep1<<OPAMP_CSR_TRIMOFFSETP_Pos);
#endif
#if STM32_OPAMP_USE_OPAMP2
    MODIFY_REG(OPAMPD2.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
         trimmingvaluep2<<OPAMP_CSR_TRIMOFFSETP_Pos);
#endif
#if STM32_OPAMP_USE_OPAMP3
    MODIFY_REG(OPAMPD3.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
         trimmingvaluep3<<OPAMP_CSR_TRIMOFFSETP_Pos);
#endif
#if STM32_OPAMP_USE_OPAMP4
    MODIFY_REG(OPAMPD4.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
         trimmingvaluep4<<OPAMP_CSR_TRIMOFFSETP_Pos);
#endif


    /* OFFTRIMmax delay 2 ms as per datasheet (electrical characteristics */
    /* Offset trim time: during calibration, minimum time needed between */
    /* two steps to have 1 mV accuracy */
    chSysPolledDelayX(MS2RTC(STM32_SYSCLK, 2));
#if STM32_OPAMP_USE_OPAMP1
    if (OPAMPD1.opamp->CSR & OPAMP_CSR_OUTCAL)   {
      /* OPAMP_CSR_OUTCAL is HIGH try higher trimming */
      trimmingvaluep1 += delta;
    } else  {
      /* OPAMP_CSR_OUTCAL is LOW try lower trimming */
      trimmingvaluep1 -= delta;
    }
#endif

#if STM32_OPAMP_USE_OPAMP2
    if (OPAMPD2.opamp->CSR & OPAMP_CSR_OUTCAL)    {
      trimmingvaluep2 += delta;
    } else {
      trimmingvaluep2 -= delta;
    }
#endif

#if STM32_OPAMP_USE_OPAMP3
    if (OPAMPD3.opamp->CSR & OPAMP_CSR_OUTCAL)    {
      trimmingvaluep3 += delta;
    } else {
      trimmingvaluep3 -= delta;
    }
#endif

#if STM32_OPAMP_USE_OPAMP4
    if (OPAMPD4.opamp->CSR & OPAMP_CSR_OUTCAL)    {
      trimmingvaluep4 += delta;
    }  else {
      trimmingvaluep4 -= delta;
    }
#endif

    delta >>= 1U;
  }

  /* Still need to check if righ calibration is current value or un step below */
  /* Indeed the first value that causes the OUTCAL bit to change from 1 to 0U */
#if STM32_OPAMP_USE_OPAMP1
  MODIFY_REG(OPAMPD1.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
       trimmingvaluep1<<OPAMP_CSR_TRIMOFFSETP_Pos);
#endif
#if STM32_OPAMP_USE_OPAMP2
  MODIFY_REG(OPAMPD2.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
       trimmingvaluep2<<OPAMP_CSR_TRIMOFFSETP_Pos);
#endif
#if STM32_OPAMP_USE_OPAMP3
  MODIFY_REG(OPAMPD3.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
       trimmingvaluep3<<OPAMP_CSR_TRIMOFFSETP_Pos);
#endif
#if STM32_OPAMP_USE_OPAMP4
  MODIFY_REG(OPAMPD4.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
       trimmingvaluep4<<OPAMP_CSR_TRIMOFFSETP_Pos);
#endif

  /* OFFTRIMmax delay 2 ms as per datasheet (electrical characteristics */
  /* Offset trim time: during calibration, minimum time needed between */
  /* two steps to have 1 mV accuracy */
  chSysPolledDelayX(MS2RTC(STM32_SYSCLK, 2));

#if STM32_OPAMP_USE_OPAMP1
    if (OPAMPD1.opamp->CSR & OPAMP_CSR_OUTCAL)   {
      /* OPAMP_CSR_OUTCAL is HIGH try higher trimming */
      trimmingvaluep1 += (trimmingvaluep1 != 31);
      MODIFY_REG(OPAMPD1.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
     trimmingvaluep1<<OPAMP_CSR_TRIMOFFSETP_Pos);
    }
#endif

#if STM32_OPAMP_USE_OPAMP2
    if (OPAMPD2.opamp->CSR & OPAMP_CSR_OUTCAL)    {
      trimmingvaluep2 += (trimmingvaluep2 != 31);
      MODIFY_REG(OPAMPD2.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
     trimmingvaluep2<<OPAMP_CSR_TRIMOFFSETP_Pos);
    }
#endif

#if STM32_OPAMP_USE_OPAMP3
    if (OPAMPD3.opamp->CSR & OPAMP_CSR_OUTCAL)    {
      trimmingvaluep3 += (trimmingvaluep3 != 31);
      MODIFY_REG(OPAMPD3.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
     trimmingvaluep3<<OPAMP_CSR_TRIMOFFSETP_Pos);
    }
#endif

#if STM32_OPAMP_USE_OPAMP4
    if (OPAMPD4.opamp->CSR & OPAMP_CSR_OUTCAL)    {
      trimmingvaluep4 += (trimmingvaluep4 != 31);
      MODIFY_REG(OPAMPD4.opamp->CSR, OPAMP_CSR_TRIMOFFSETP,
     trimmingvaluep4<<OPAMP_CSR_TRIMOFFSETP_Pos);
    }
#endif

#if STM32_OPAMP_USE_OPAMP1
#define CSRm OPAMPD1.opamp->CSR
    /* Disable calibration */
    CSRm &= ~OPAMP_CSR_CALON;
    /* Disable the OPAMPs */
    CSRm &= ~OPAMP_CSR_OPAMPxEN;
    /* Set normal operating mode back */
    CSRm &= ~OPAMP_CSR_FORCEVP;
    /* Write calibration result N */
    OPAMPD1.trim_n = trimmingvaluen1;
    /* Write calibration result P */
    OPAMPD1.trim_p = trimmingvaluep1;
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETN,
         trimmingvaluen1<<OPAMP_CSR_TRIMOFFSETN_Pos);
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETP,
         trimmingvaluep1<<OPAMP_CSR_TRIMOFFSETP_Pos);
    OPAMPD1.state = OPAMP_STOP;
#undef CSRm
#endif

#if STM32_OPAMP_USE_OPAMP2
#define CSRm OPAMPD2.opamp->CSR
    /* Disable calibration */
    CSRm &= ~OPAMP_CSR_CALON;
    /* Disable the OPAMPs */
    CSRm &= ~OPAMP_CSR_OPAMPxEN;
    /* Set normal operating mode back */
    CSRm &= ~OPAMP_CSR_FORCEVP;
    /* Write calibration result N */
    OPAMPD2.trim_n = trimmingvaluen2;
    /* Write calibration result P */
    OPAMPD2.trim_p = trimmingvaluep2;
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETN,
         trimmingvaluen2<<OPAMP_CSR_TRIMOFFSETN_Pos);
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETP,
         trimmingvaluep2<<OPAMP_CSR_TRIMOFFSETP_Pos);
    OPAMPD2.state = OPAMP_STOP;
#undef CSRm
#endif

#if STM32_OPAMP_USE_OPAMP3
#define CSRm OPAMPD3.opamp->CSR
    /* Disable calibration */
    CSRm &= ~OPAMP_CSR_CALON;
    /* Disable the OPAMPs */
    CSRm &= ~OPAMP_CSR_OPAMPxEN;
    /* Set normal operating mode back */
    CSRm &= ~OPAMP_CSR_FORCEVP;
    /* Write calibration result N */
    OPAMPD3.trim_n = trimmingvaluen3;
    /* Write calibration result P */
    OPAMPD3.trim_p = trimmingvaluep3;
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETN,
         trimmingvaluen3<<OPAMP_CSR_TRIMOFFSETN_Pos);
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETP,
         trimmingvaluep3<<OPAMP_CSR_TRIMOFFSETP_Pos);
    OPAMPD3.state = OPAMP_STOP;
#undef CSRm
#endif

#if STM32_OPAMP_USE_OPAMP4
#define CSRm OPAMPD4.opamp->CSR
    /* Disable calibration */
    CSRm &= ~OPAMP_CSR_CALON;
    /* Disable the OPAMPs */
    CSRm &= ~OPAMP_CSR_OPAMPxEN;
    /* Set normal operating mode back */
    CSRm &= ~OPAMP_CSR_FORCEVP;
    /* Write calibration result N */
    OPAMPD4.trim_n = trimmingvaluen4;
    /* Write calibration result P */
    OPAMPD4.trim_p = trimmingvaluep4;
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETN,
         trimmingvaluen4<<OPAMP_CSR_TRIMOFFSETN_Pos);
    MODIFY_REG(CSRm, OPAMP_CSR_TRIMOFFSETP,
         trimmingvaluep4<<OPAMP_CSR_TRIMOFFSETP_Pos);
    OPAMPD4.state = OPAMP_STOP;
#undef CSRm
#endif
}

void opamp_lld_calibrate(void)
{
  uint8_t trim_n[4] = {255};
  uint8_t trim_p[4] = {255};
  bool done;
  do {
    done = true;
    opamp_lld_calibrate_once();
#if STM32_OPAMP_USE_OPAMP1
    done = done && (OPAMPD1.trim_n == trim_n[0]) &&  (OPAMPD1.trim_p == trim_p[0]);
    trim_n[0] = OPAMPD1.trim_n;
    trim_p[0] = OPAMPD1.trim_p;
#endif
#if STM32_OPAMP_USE_OPAMP2
    done = done && (OPAMPD2.trim_n == trim_n[1]) &&  (OPAMPD2.trim_p == trim_p[1]);
    trim_n[1] = OPAMPD2.trim_n;
    trim_p[1] = OPAMPD2.trim_p;
#endif
#if STM32_OPAMP_USE_OPAMP3
    done = done && (OPAMPD3.trim_n == trim_n[2]) &&  (OPAMPD3.trim_p == trim_p[2]);
    trim_n[2] = OPAMPD3.trim_n;
    trim_p[2] = OPAMPD3.trim_p;
#endif
#if STM32_OPAMP_USE_OPAMP4
    done = done && (OPAMPD4.trim_n == trim_n[3]) &&  (OPAMPD4.trim_p == trim_p[3]);
    trim_n[3] = OPAMPD4.trim_n;
    trim_p[3] = OPAMPD4.trim_p;
#endif
  } while (!done);

}
#endif // STM32_OPAMP_USER_TRIM_ENABLED
#endif /* HAL_USE_OPAMP */

/** @} */

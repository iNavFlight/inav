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
 * @file    SAMA5D2x/sama_secumod.c
 * @brief   SAMA SECUMOD support code.
 *
 * @addtogroup SAMA5D2x_SECUMOD
 * @{
 */

#include "hal.h"

#if SAMA_USE_SECUMOD || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/
/**
 * @brief SEC driver identifier.
 */
SECDriver SECD0;

/*===========================================================================*/
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver constant                                                           */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief   SECURAM interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_SECURAM_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t ramaccsr, sr, sysr;
  uint32_t rwx;
  uint32_t i;

  ramaccsr = SECUMOD->SECUMOD_RAMACCSR;
  sr = SECUMOD->SECUMOD_SR;

  for (i = 0; i < SECUMOD_RAM_REGIONS; i++) {
    rwx = (ramaccsr >> (2 * i)) & 3;
    if (rwx != RAMACCSR_NO_VIOLATION && SECD0.config->securam_callback != NULL)
      SECD0.config->securam_callback(&SECD0);
  }

  /* process the end of erase signalling */
  do {
    sysr = SECUMOD->SECUMOD_SYSR;
  } while (sysr & SECUMOD_SYSR_ERASE_ON);

  if (SECUMOD_SYSR_ERASE_DONE == (sysr & SECUMOD_SYSR_ERASE_DONE)) {
    /* Clear the flag ERASE_DONE */
    SECUMOD->SECUMOD_SYSR = SECUMOD_SYSR_ERASE_DONE;
    if (SECD0.config->erased_callback != NULL)
      SECD0.config->erased_callback(&SECD0);
  }

  /* wait at least one slow clock */
  chSysPolledDelayX(SAMA_PCK / SAMA_SLOW_CLK);

  /* Clear RAM access violation flags */
  SECUMOD->SECUMOD_RAMACCSR = ramaccsr;
  /* Clear corresponding alarm flag bit */
  SECUMOD->SECUMOD_SCR = sr;

  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   SECUMOD interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_SECUMOD_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t sr, nimpr;

  /* Read alarm status */
  sr = SECUMOD->SECUMOD_SR;
  nimpr = SECUMOD->SECUMOD_NIMPR;

  if ((sr & SECUMOD_SR_SHLDM) && (nimpr & SECUMOD_NIMPR_SHLDM)) {
    SECD0.secumod_callback(&SECD0, SEC_EVENT_SHLDM);
  }
  else if ((sr & SECUMOD_SR_DBLFM) && (nimpr & SECUMOD_NIMPR_DBLFM)) {
    SECD0.secumod_callback(&SECD0, SEC_EVENT_DBLFM);
  }
  else if ((sr & SECUMOD_SR_TST) && (nimpr & SECUMOD_NIMPR_TST)) {
    SECD0.secumod_callback(&SECD0, SEC_EVENT_TST);
  }
  else if ((sr & SECUMOD_SR_JTAG) && (nimpr & SECUMOD_NIMPR_JTAG)) {
    SECD0.secumod_callback(&SECD0, SEC_EVENT_JTAG);
  }
  else if ((sr & SECUMOD_SR_MCKM) && (nimpr & SECUMOD_NIMPR_MCKM)) {
    SECD0.secumod_callback(&SECD0, SEC_EVENT_MCKM);
  }
  else if ((sr & SECUMOD_SR_TPML) && (nimpr & SECUMOD_NIMPR_TPML)) {
    SECD0.secumod_callback(&SECD0, SEC_EVENT_TPML);
  }
  else if ((sr & SECUMOD_SR_TPMH) && (nimpr & SECUMOD_NIMPR_TPMH)) {
    SECD0.secumod_callback(&SECD0, SEC_EVENT_TPMH);
  }
  else if ((sr & SECUMOD_SR_VDDBUL) && (nimpr & SECUMOD_NIMPR_VDDBUL)) {
    SECD0.secumod_callback(&SECD0, SEC_EVENT_VDDBUL);
  }
  else if ((sr & SECUMOD_SR_VDDBUH) && (nimpr & SECUMOD_NIMPR_VDDBUH)) {
    SECD0.secumod_callback(&SECD0, SEC_EVENT_VDDBUH);
  }
  else if ((sr & SECUMOD_SR_VDDCOREL) && (nimpr & SECUMOD_NIMPR_VDDCOREL)) {
    SECD0.secumod_callback(&SECD0, SEC_EVENT_VDDCOREL);
  }
  else if ((sr & SECUMOD_SR_VDDCOREH) && (nimpr & SECUMOD_NIMPR_VDDCOREH)) {
    SECD0.secumod_callback(&SECD0, SEC_EVENT_VDDCOREH);
  }
  else if ((sr & SECUMOD_SR_DET0) && (nimpr & SECUMOD_NIMPR_DET0)) {
	    SECD0.secumod_callback(&SECD0, SEC_EVENT_PIOBU0);
  }
  else if ((sr & SECUMOD_SR_DET1) && (nimpr & SECUMOD_NIMPR_DET1)) {
	    SECD0.secumod_callback(&SECD0, SEC_EVENT_PIOBU1);
  }
  else if ((sr & SECUMOD_SR_DET2) && (nimpr & SECUMOD_NIMPR_DET2)) {
	    SECD0.secumod_callback(&SECD0, SEC_EVENT_PIOBU2);
  }
  else if ((sr & SECUMOD_SR_DET3) && (nimpr & SECUMOD_NIMPR_DET3)) {
	    SECD0.secumod_callback(&SECD0, SEC_EVENT_PIOBU3);
  }
  else if ((sr & SECUMOD_SR_DET4) && (nimpr & SECUMOD_NIMPR_DET4)) {
	    SECD0.secumod_callback(&SECD0, SEC_EVENT_PIOBU4);
  }
  else if ((sr & SECUMOD_SR_DET5) && (nimpr & SECUMOD_NIMPR_DET5)) {
	    SECD0.secumod_callback(&SECD0, SEC_EVENT_PIOBU5);
  }
  else if ((sr & SECUMOD_SR_DET6) && (nimpr & SECUMOD_NIMPR_DET6)) {
	    SECD0.secumod_callback(&SECD0, SEC_EVENT_PIOBU6);
  }
  else if ((sr & SECUMOD_SR_DET7) && (nimpr & SECUMOD_NIMPR_DET7)) {
	    SECD0.secumod_callback(&SECD0, SEC_EVENT_PIOBU7);
  }
  else {
	  (void) 0;
  }

  /* wait at least one slow clock */
  chSysPolledDelayX(SAMA_PCK / SAMA_SLOW_CLK);

  /* Clear corresponding alarm flag bit */
  SECUMOD->SECUMOD_SCR = sr;

  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
/**
 * @brief   Configures PIOBU pads according to configuration struct.
 *
 * @param[in] listp     pointer to a PIOBUConfig array
 * @param[in] length    length of array
 *
 * @notapi
 */
void piobu_config(PIOBUConfig *listp, size_t length) {
  uint8_t i;
  uint32_t cfg;
  uint32_t index;
  PIOBUConfig *piobup;

  for (i = 0; i < length; i++) {
    index = listp[i].pinIndex;
    piobup = &listp[i];

  /* AFV and RFV fields must be set to 0 when dynamic intrusion is selected. */
    if (piobup->dynamic) {
        cfg = 0;
    } else {
        cfg = (piobup->afv << SECUMOD_PIOBU_AFV_Pos ) | (piobup->rfv << SECUMOD_PIOBU_RFV_Pos);
    }

    if (piobup->mode) {
      cfg |= SECUMOD_PIOBU_OUTPUT;
      if (piobup->outputLevel)
        cfg |= SECUMOD_PIOBU_PIO_SOD;
    }

    cfg |= piobup->pullUpState << SECUMOD_PIOBU_PULLUP_Pos;

    if (piobup->scheduled)
      cfg |= SECUMOD_PIOBU_SCHEDULE;

    if (piobup->inputDefaultLevel)
      cfg |= SECUMOD_PIOBU_SWITCH;

    /* FILTER3_5 and DYNSTAT fields exist only for even PIOBUs */
    if (0 == (index & 0x01)) {
      if (piobup->dynamic)
        cfg |= SECUMOD_PIOBU_DYNSTAT;

      if (piobup->filter3_5)
        cfg |=  SECUMOD_PIOBU_FILTER3_5;
    }
    SECUMOD->SECUMOD_PIOBU[index] = cfg;
  }
}

/**
 * @brief   Low level SEC driver initialization.
 *
 * @notapi
 */
void sec_lld_init(void) {

  /* Driver initialization.*/
  secObjectInit(&SECD0);
  SECD0.sec = SECUMOD;
}

/**
 * @brief   Configures and activates the SEC peripheral.
 *
 * @param[in] secp      pointer to the @p SECDriver object
 *
 * @notapi
 */
void sec_lld_start(SECDriver *secp) {

  uint8_t i;

  if (secp->state == SEC_STOP) {
    /* Clock activation. */
    pmcEnableSEC();

    /* Register Reset */
    secp->sec->SECUMOD_NIDPR = SECUMOD_NIDPR_ALL;
    secumodSetNormalModeProtections(~SECUMOD_NMPR_ALL);

    /*
     * Configure interrupts
     */
    aicSetIntSourceType(ID_SECUMOD, INT_LEVEL_SENSITIVE);
    aicSetSourcePriority(ID_SECUMOD, SAMA_SECUMOD_IRQ_PRIORITY);
    aicSetSourceHandler(ID_SECUMOD, SAMA_SECUMOD_HANDLER);

    aicSetIntSourceType(ID_SECURAM, INT_LEVEL_SENSITIVE);
    aicSetSourcePriority(ID_SECURAM, SAMA_SECURAM_IRQ_PRIORITY);
    aicSetSourceHandler(ID_SECURAM, SAMA_SECURAM_HANDLER);

    /* Enabling interrupt. */
    aicEnableInt(ID_SECUMOD);
    aicEnableInt(ID_SECURAM);
    }

  uint32_t ramacc_cfg = 0;

  /* Select mode normal or backup*/
  secp->sec->SECUMOD_CR = secp->config->cr;

  /* Configure JTAGCR */
  secp->sec->SECUMOD_JTAGCR = secp->config->jtagcr;

  /* Configure region rights. */
  for (i = 0; i < SECUMOD_RAM_REGIONS; i++) {
    ramacc_cfg |= (secp->config->region[i].mode & 0x3u) << (i * 2);
  }
  secp->sec->SECUMOD_RAMACC = ramacc_cfg;

  /* Configure PIOBU pads. */
  piobu_config(secp->config->list, secp->config->length);
}

/**
 * @brief   Deactivates the SEC peripheral.
 *
 * @param[in] secp      pointer to the @p SECDriver object
 *
 * @notapi
 */
void sec_lld_stop(SECDriver *secp) {

  secp->sec->SECUMOD_NMPR &= ~(0xFF << 16);
  secp->sec->SECUMOD_NIDPR |= (0xFF << 16);

  aicDisableInt(ID_SECURAM);
  aicDisableInt(ID_SECUMOD);

  pmcDisableSEC();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/
/**
 * @brief   SEC Driver initialization.
 *
 * @init
 */
void secInit(void) {

  sec_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p SECDriver structure.
 *
 * @param[out] secp     pointer to a @p SECDriver object
 *
 * @init
 */
void secObjectInit(SECDriver *secp) {

  secp->state    = SEC_STOP;
  secp->config   = NULL;
}

/**
 * @brief   Configures and activates the SEC peripheral.
 *
 * @param[in] secp      pointer to a @p SECDriver object
 * @param[in] config    pointer to a @p SECConfig object
 *
 * @api
 */
void secStart(SECDriver *secp, const SECConfig *config) {

  osalDbgCheck((secp != NULL) && (config != NULL));

  osalSysLock();
  osalDbgAssert((secp->state == SEC_STOP), "invalid state");
  secp->config = config;
  sec_lld_start(secp);
  secp->state = SEC_ACTIVE;
  osalSysUnlock();
}

/**
 * @brief   Deactivates the SEC peripheral.
 *
 * @param[in] secp      pointer to a @p SECDriver object
 *
 * @api
 */
void secStop(SECDriver *secp) {

  osalDbgCheck(secp != NULL);

  osalSysLock();

  osalDbgAssert((secp->state == SEC_STOP) || (secp->state == SEC_ACTIVE),
                "invalid state");

  sec_lld_stop(secp);
  secp->config  = NULL;
  secp->state   = SEC_STOP;

  osalSysUnlock();
}

/**
 * @brief   Enables or disables SECUMOD callbacks.
 * @details This function enables or disables callbacks, use a @p NULL pointer
 *          in order to disable a callback.
 * @note    The function can be called from any context.
 *
 * @param[in] secp      pointer to SECUMOD driver structure
 * @param[in] sources   Bitwise OR of protections.
 * @param[in] callback  callback function pointer or @p NULL
 *
 * @api
 */
void secSetCallback(SECDriver *secp, uint32_t sources, secumod_callback_t callback) {

  osalDbgCheck(secp != NULL);

  syssts_t sts;

  /* Entering a reentrant critical zone.*/
  sts = osalSysGetStatusAndLockX();

  if (callback != NULL) {

    /* IRQ sources enabled only after setting up the callback.*/
    secp->secumod_callback = callback;
    secp->sec->SECUMOD_NIEPR = sources;
    if (SECUMOD->SECUMOD_NIMPR != sources) {
      secumodToggleProtectionReg();
      secp->sec->SECUMOD_NIEPR = sources;
    }
  }
  else {
    secp->sec->SECUMOD_NIDPR = sources;

    /* Callback set to NULL only after disabling the IRQ sources.*/
    secp->secumod_callback = NULL;
  }

  /* Leaving a reentrant critical zone.*/
  osalSysRestoreStatusX(sts);
}

/**
 * @brief Set JTAG protection options of SECUMOD.
 *
 * @param[in] reset         Whether preventing debug state and BSD (Boundary Scan Diagnostics) to work.
 * @param[in] permissions   Debug permissions.
 * @param[in] ack           Whether monitor the DBGACK signal.
 *
 * @api
 */
void secumodSetJtagProtection(bool reset, uint8_t permissions,
    bool ack) {

  uint32_t jtagcr;
  jtagcr = permissions << SECUMOD_JTAGCR_CA5_DEBUG_MODE_Pos;

  if (reset)
    jtagcr |= SECUMOD_JTAGCR_FNTRST;

  if (ack)
    jtagcr |= SECUMOD_JTAGCR_CA5_DEBUG_MON;

  SECUMOD->SECUMOD_JTAGCR = jtagcr;
}

/**
 * @brief Tuning dynamic signatures by period and threshold.
 *
 * @param[in] period         Signature Clock Period.
 * @param[in] detection_thr  Error Detection Threshold.
 * @param[in] reset_thr      Error Counter Reset Threshold.
 *
 * @api
 */
void secumodDynamicSignaturesTuning(uint16_t period,
    uint8_t detectionThr, uint8_t resetThr) {

  uint32_t dystune;

  dystune = SECUMOD->SECUMOD_DYSTUNE & SECUMOD_DYSTUNE_NOPA;
  dystune |= SECUMOD_DYSTUNE_PERIOD(period);
  dystune |= SECUMOD_DYSTUNE_RX_ERROR_THRESHOLD(detectionThr);
  dystune |= SECUMOD_DYSTUNE_RX_OK_CORREL_NUMBER(resetThr);
  SECUMOD->SECUMOD_DYSTUNE = dystune;
}

/**
 * @brief Enable/Disable alarm regenerated periodically while intrusion is maintained.
 *
 * @param[in] enable periodic alarm while intrusion is maintained,
 *            true: disable, false: enable.
 * @api
 */
void secumodPeriodicAlarm(bool enable) {

  uint32_t tmp;

  tmp = SECUMOD->SECUMOD_DYSTUNE & ~SECUMOD_DYSTUNE_NOPA;
  if (!enable)
    tmp |= SECUMOD_DYSTUNE_NOPA;
  SECUMOD->SECUMOD_DYSTUNE = tmp;
}

/**
 * @brief Set access rights for secure RAM in SECUMOD.
 *
 * @param[in] region  RAM region N,
 *                for N = 0 ~ 5: RAM range (N)Kbyte ~ (N+1)Kbyte;
 *                for N = 5: register bank 256bit.
 * @param rights  0: No access allowed;
 *                1: Only write access allowed;
 *                2: Only read access allowed;
 *                3: Read and write access allowed.
 * @api
 */
void secumodSetRamAccessRights(uint32_t region, uint8_t rights) {

  uint32_t tmp;

  tmp = SECUMOD->SECUMOD_RAMACC & ~SECUMOD_RAMACC_RWx_Msk(region);
  SECUMOD->SECUMOD_RAMACC = tmp | (rights << SECUMOD_RAMACC_RWx_Pos(region));
}

/**
 * @brief Read the SECUMOD internal memory from the specified address
 *
 * @param[in] data  Point to where the data read is stored
 * @param[in] addr memory address
 * @param[in] size The number of bytes to be read
 *
 * @return    Bytes read
 *
 * @api
 */
uint32_t secumodReadInternalMemory(uint8_t *data, uint32_t addr, uint32_t size) {

  uint32_t i;
  uint32_t region;
  uint32_t count;

  if (addr >= ((uint32_t)SECURAM))
    addr -= ((uint32_t)SECURAM);

  secumodMemReady();

  for (i = 0; i < size; i += count) {
    region = (addr + i) >> 10;
    if ((SECUMOD_RAMACC_RWx_NO_ACCESS(region) ==
        (SECUMOD->SECUMOD_RAMACC & SECUMOD_RAMACC_RWx_Msk(region))) ||
      (SECUMOD_RAMACC_RWx_WR_ACCESS(region) ==
        (SECUMOD->SECUMOD_RAMACC & SECUMOD_RAMACC_RWx_Msk(region)))) {
      break;
    }
    count = size;
    if (((region + 1) << 10 ) <= (addr + i + size)) {
      size = ((region + 1) << 10) - (addr + i);
    }
    memcpy(data + i, (uint8_t *)(((uint32_t)SECURAM) + addr + i), count);
  }
  return i;
}

/**
 * @brief Write data to the SECUMOD internal memory from the specified address
 *
 * @param[in] data Pointer to the data to be written
 * @param[in] addr memory address
 * @param[in] size The number of bytes to be be written
 *
 * @return    Bytes written
 *
 * @api
 */
uint32_t secumodWriteInternalMemory(uint8_t *data, uint32_t addr, uint32_t size) {

  uint32_t i;
  uint32_t region;
  uint32_t count;

  if (addr >= ((uint32_t)SECURAM))
    addr -= ((uint32_t)SECURAM);

  secumodMemReady();

  for (i = 0; i < size; i += count) {
    region = (addr + i) >> 10;
    if ((SECUMOD_RAMACC_RWx_NO_ACCESS(region) ==
        (SECUMOD->SECUMOD_RAMACC & SECUMOD_RAMACC_RWx_Msk(region))) ||
      (SECUMOD_RAMACC_RWx_RD_ACCESS(region) ==
        (SECUMOD->SECUMOD_RAMACC & SECUMOD_RAMACC_RWx_Msk(region)))) {
      break;
    }
    count = size;
    if (((region + 1) << 10 ) <= (addr + i + size)) {
      size = ((region + 1) << 10) - (addr + i);
    }
    memcpy((uint8_t *)(((uint32_t)SECURAM) + addr + i), data + i, count);
  }
  return i;
}

#endif /* SAMA_USE_SECUMOD */

/** @} */

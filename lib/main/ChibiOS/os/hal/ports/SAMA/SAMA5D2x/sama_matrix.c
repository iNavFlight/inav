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
 * @file    SAMA5D2x/sama_matrix.c
 * @brief   SAMA MATRIX support code.
 *
 * @addtogroup SAMA5D2x_MATRIX
 * @{
 */

#include "hal.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver constant                                                           */
/*===========================================================================*/
#define SCFG_OFFSET                         0x40u

/*===========================================================================*/
/* Driver local macros.                                                      */
/*===========================================================================*/
#define MATRIX_SCFG(value)                  (MATRIX_SCFG0 + (value * 4u))
#define MATRIX_SCFG_FIXED_DEFMSTR(value)    MATRIX_SCFG0_FIXED_DEFMSTR(value)
#define MATRIX_SCFG_DEFMSTR_TYPE(value)     MATRIX_SCFG0_DEFMSTR_TYPE(value)

/**
 * @brief   Enable write protection on MATRIX registers block.
 *
 * @param[in] mtxp       pointer to a MATRIX register block.
 *
 * @notapi
 */
#define mtxEnableWP(mtxp) {                                                \
  mtxp->MATRIX_WPMR = MATRIX_WPMR_WPKEY_PASSWD | MATRIX_WPMR_WPEN;         \
}

/**
 * @brief   Disable write protection on MATRIX registers block.
 *
 * @param[in] matxp      pointer to a MATRIX register block.
 *
 * @notapi
 */
#define mtxDisableWP(mtxp) {                                               \
  mtxp->MATRIX_WPMR = MATRIX_WPMR_WPKEY_PASSWD;                            \
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/
/**
 * @brief   Configures peripheral security
 *
 * @param[in] mtxp      pointer to a MATRIX register block.
 * @param[in] id        PERIPHERAL_ID.
 * @param[in] mode      SECURE_PER or NOT_SECURE_PER.
 *
 * @retval true         Peripheral is not secured.
 * @retval false        Peripheral is secured.
 *
 */
bool mtxConfigPeriphSecurity(Matrix *mtxp, uint32_t id, bool mode) {

  uint32_t mask;
  mask = id & 0x1F;

  mtxDisableWP(mtxp);
  if (mode) {
    mtxp->MATRIX_SPSELR[id / 32] |= (MATRIX_SPSELR_NSECP0 << mask);
  }
  else {
    mtxp->MATRIX_SPSELR[id / 32] &= ~(MATRIX_SPSELR_NSECP0 << mask);
  }
  mtxEnableWP(mtxp);

  return (MATRIX0->MATRIX_SPSELR[id / 32] & (MATRIX_SPSELR_NSECP0 << mask)) &
         (MATRIX1->MATRIX_SPSELR[id / 32] & (MATRIX_SPSELR_NSECP0 << mask));
}

/**
 * @brief    Associates slave with a kind of master
 * @note masterID is set only if type is fixed default master.
 *       Specifying the number of a master which is not connected
 *       to the selected slave is equivalent to clearing DEFMSTR_TYPE.
 *
 * @param[in] mtxp      pointer to a MATRIX register block.
 * @param[in] slaveID   Slave MATRIX ID.
 * @param[in] type      Select from
 *                        No default master,
 *                        Last access master,
 *                        Fixed default master.
 * @param[in] masterID  Master MATRIX ID.
 */
void mtxConfigDefaultMaster(Matrix *mtxp, uint8_t slaveID,
        uint8_t type, uint8_t masterID) {

  mtxDisableWP(mtxp);

  volatile uint32_t *scfgAddress = (uint32_t *) ((uint32_t) mtxp + SCFG_OFFSET + (4u * slaveID));
  *scfgAddress = MATRIX_SCFG_DEFMSTR_TYPE(type);

  if (type == FIXED_DEFAULT_MASTER) {
    *scfgAddress = MATRIX_SCFG_FIXED_DEFMSTR(masterID);
  }
  mtxEnableWP(mtxp);
}

/**
 * @brief   Configures slave security region
 *
 * @param[in] mtxp      pointer to a MATRIX register block.
 * @param[in] slaveID   Slave MATRIX ID.
 * @param[in] selMask   Securable area.
 * @param[in] readMask  Secure for read.
 * @param[in] writeMask Secure for write.
 */
void mtxConfigSlaveSec(Matrix *mtxp, uint8_t slaveID,
        uint8_t selMask, uint8_t readMask,
        uint8_t writeMask) {

  mtxDisableWP(mtxp);
  mtxp->MATRIX_SSR[slaveID] = selMask | (readMask << 8) |
                              (writeMask << 16);
  mtxEnableWP(mtxp);
}

/**
 * @brief   Configures split area of region
 *
 * @param[in] mtxp      pointer to a MATRIX register block.
 * @param[in] slaveID   Slave MATRIX ID.
 * @param[in] areaSize  Split size area.
 * @param[in] mask      Region securable area.
 */
void mtxSetSlaveSplitAddr(Matrix *mtxp, uint8_t slaveID,
         uint8_t areaSize, uint8_t mask) {

  mtxDisableWP(mtxp);
  uint8_t i = mask, j = 0;
  uint32_t value = 0;
  uint32_t pmask = 0;
  for (i = 1; (i <= mask) && (j < 32); i <<= 1, j += 4) {
    if (i & mask) {
      value |= areaSize << j;
      pmask |= 0x0F << j;
    }
  }
  mtxp->MATRIX_SASSR[slaveID] = (mtxp->MATRIX_SASSR[slaveID] & ~pmask) | value;
  mtxEnableWP(mtxp);
}

/**
 * @brief   Configures size area of region
 * @note Not applicable to internal security type
 *
 * @param[in] mtxp      pointer to a MATRIX register block.
 * @param[in] slaveID   Slave MATRIX ID.
 * @param[in] areaSize  Size of total area.
 * @param[in] mask      Region securable area.
 */
void mtxSetSlaveRegionSize(Matrix *mtxp, uint8_t slaveID,
          uint8_t areaSize, uint8_t mask) {

  osalDbgCheck(slaveID != 0);

  mtxDisableWP(mtxp);
  uint8_t i = mask, j = 0;
  uint32_t value = 0;
  uint32_t pmask = 0;
  for (i = 1; (i <= mask) && (j < 32 ); i <<= 1, j += 4) {
    if (i & mask) {
      value |= areaSize << j;
      pmask |= 0x0F << j;
    }
  }
  mtxp->MATRIX_SRTSR[slaveID] = (mtxp->MATRIX_SRTSR[slaveID] & ~pmask) | value;
  mtxEnableWP(mtxp);
}

/**
 * @brief   Changes the mapping of the chip so that the remap area
 *          mirrors the internal ROM or the EBI CS0.
 */
void mtxRemapRom(void) {

  AXIMX->AXIMX_REMAP = 0;

  /* Invalidate I-Cache*/
  L1C_InvalidateICacheAll();

  /* Invalidate Region */
  cacheInvalidateRegion((void*)0, IRAM_SIZE);
}

/**
 * @brief   Changes the mapping of the chip so that the remap area
 *          mirrors the internal ROM or the EBI CS0.
 */
void mtxRemapRam(void) {

  AXIMX->AXIMX_REMAP = AXIMX_REMAP_REMAP0;

  /* Invalidate I-Cache*/
  L1C_InvalidateICacheAll();

  /* Clean I-Region */
  cacheCleanRegion((void*)IRAM_ADDR, IRAM_SIZE);
  /* Invalidate Region */
  cacheInvalidateRegion((void*)0, IRAM_SIZE);
}

/** @} */

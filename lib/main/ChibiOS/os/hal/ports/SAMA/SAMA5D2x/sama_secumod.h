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
 * @file    SAMA5D2x/sama_secumod.h
 * @brief   SAMA SECUMOD support macros and structures.
 *
 * @addtogroup SAMA5D2x_SECUMOD
 * @{
 */

#ifndef SAMA_SECUMOD_LLD_H
#define SAMA_SECUMOD_LLD_H

/**
 * @brief   Using the SECUMOD driver.
 */
#if !defined(SAMA_USE_SECUMOD) || defined(__DOXYGEN__)
#define SAMA_USE_SECUMOD                         FALSE
#endif

#if SAMA_USE_SECUMOD || defined(__DOXYGEN__)

#include <string.h>
/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
/**
 * @name    RAM ACCESS mode macros
 * @{
 */
/**
 * @brief   No access allowed.
 */
#define RAMACC_NO_ACCESS                        0x0U

/**
 * @brief   Only write access allowed.
 */
#define RAMACC_WR_ACCESS                        0x1U

/**
 * @brief   Only read access allowed.
 */
#define RAMACC_RD_ACCESS                        0x2U

/**
 * @brief   Read and Write access allowed.
 */
#define RAMACC_WR_RD_ACCESS                     0x3U
/** @} */

/**
 * @name    SOURCE INTERRUPT macros
 * @{
 */
/*
 * @brief Shield Monitor Protection Interrupt Source.
 */
#define SECUMOD_SHLDM                           (0x1u << 0)

/*
 * @brief Double Frequency Monitor Protection Interrupt Source.
 */
#define SECUMOD_DBLFM                           (0x1u << 1)

/*
 * @brief Test Pin Protection Interrupt Source.
 */
#define SECUMOD_TST                             (0x1u << 2)

/*
 * @brief JTAG Pins Protection Interrupt Source.
 */
#define SECUMOD_JTAG                            (0x1u << 3)

/*
 * @brief Master Clock Monitor Protection Interrupt Source.
 */
#define SECUMOD_MCKM                            (0x1u << 5)

/*
 * @brief Low Temperature Monitor Protection Interrupt Source.
 */
#define SECUMOD_TPML                            (0x1u << 6)

/*
 * @brief High Temperature Monitor Protection Interrupt Source.
 */
#define SECUMOD_TPMH                            (0x1u << 7)

/*
 * @brief Low VDDBU Voltage Monitor Protection Interrupt Source.
 */
#define SECUMOD_VDDBUL                          (0x1u << 10)

/*
 * @brief High VDDBU Voltage Monitor Protection Interrupt Source.
 */
#define SECUMOD_VDDBUH                          (0x1u << 11)

/*
 * @brief Low VDDCORE Voltage Monitor Protection Interrupt Source.
 */
#define SECUMOD_VDDCOREL                        (0x1u << 12)

/*
 * @brief High VDDCORE Voltage Monitor Protection Interrupt Source.
 */
#define SECUMOD_VDDCOREH                        (0x1u << 13)

/*
 * @brief PIOBUx Intrusion Detector Protection Interrupt Source.
 */
#define SECUMOD_DET0                            (0x1u << 16)
#define SECUMOD_DET1                            (0x1u << 17)
#define SECUMOD_DET2                            (0x1u << 18)
#define SECUMOD_DET3                            (0x1u << 19)
#define SECUMOD_DET4                            (0x1u << 20)
#define SECUMOD_DET5                            (0x1u << 21)
#define SECUMOD_DET6                            (0x1u << 22)
#define SECUMOD_DET7                            (0x1u << 23)
/** @} */

/**
 * @name    RAM STATUS mode macros
 * @{
 */
/**
 * @brief   No access violation occurred.
 */
#define RAMACCSR_NO_VIOLATION                   0x0U

/**
 * @brief   Write access violation occurred.
 */
#define RAMACCSR_WR_VIOLATION                   0x1U

/**
 * @brief   Read access violation occurred.
 */
#define RAMACCSR_RD_VIOLATION                   0x2U

/**
 * @brief   Read and Write access violation occurred.
 */
#define RAMACCSR_WR_RD_VIOLATION                0x3U
/** @} */

/**
 * @name    SCRAMB mode macros
 * @{
 */
/**
 * @brief   SCRAMB ENABLE.
 */
#define SCRAMB_ENABLE                           0x1U

/**
 * @brief   SCRAMB DISABLE.
 */
#define SCRAMB_DISABLE                          0x2U
/** @} */

/*
 * @brief RAM regions of SECUMOD
 */
#define SECUMOD_RAM_REGIONS      6

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/
/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   SECUMOD interrupt priority level setting.
 */
#if !defined(SAMA_SECUMOD_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_SECUMOD_IRQ_PRIORITY           7
#endif

/**
 * @brief   SECURAM interrupt priority level setting.
 */
#if !defined(SAMA_SECURAM_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_SECURAM_IRQ_PRIORITY           7
#endif

/** @} */
/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/
/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  SEC_UNINIT = 0,                   /**< Not initialized.                   */
  SEC_STOP = 1,                     /**< Stopped.                           */
  SEC_ACTIVE = 2                    /**< Active.                            */
} secstate_t;

/**
  * @brief   Type of an SECUMOD event.
  */
typedef enum {
  SEC_EVENT_SHLDM    = 0,           /* Triggered on Shield Monitor.               */
  SEC_EVENT_DBLFM    = 1,           /* Triggered on Double Frequency Monitor.     */
  SEC_EVENT_TST      = 2,           /* Triggered on Test Pin Monitor.             */
  SEC_EVENT_JTAG     = 3,           /* Triggered on JTAG Pins Monitor.            */
  SEC_EVENT_MCKM     = 4,           /* Triggered on Master Clock Monitor.         */
  SEC_EVENT_TPML     = 5,           /* Triggered on Low Temperature Monitor.      */
  SEC_EVENT_TPMH     = 6,           /* Triggered on High Temperature Monitor.     */
  SEC_EVENT_VDDBUL   = 7,           /* Triggered on Low VDDBU Voltage Monitor.    */
  SEC_EVENT_VDDBUH   = 8,           /* Triggered on High VDDBU Voltage Monitor.   */
  SEC_EVENT_VDDCOREL = 9,           /* Triggered on Low VDDCORE Voltage Monitor.  */
  SEC_EVENT_VDDCOREH = 10,          /* Triggered on High VDDCORE Voltage Monitor. */
  SEC_EVENT_PIOBU0   = 11,          /* Triggered on PIOBUx intrusion.             */
  SEC_EVENT_PIOBU1   = 12,          /* Triggered on PIOBUx intrusion.             */
  SEC_EVENT_PIOBU2   = 13,          /* Triggered on PIOBUx intrusion.             */
  SEC_EVENT_PIOBU3   = 14,          /* Triggered on PIOBUx intrusion.             */
  SEC_EVENT_PIOBU4   = 15,          /* Triggered on PIOBUx intrusion.             */
  SEC_EVENT_PIOBU5   = 16,          /* Triggered on PIOBUx intrusion.             */
  SEC_EVENT_PIOBU6   = 17,          /* Triggered on PIOBUx intrusion.             */
  SEC_EVENT_PIOBU7   = 18           /* Triggered on PIOBUx intrusion.             */
} secevent_t;

/**
 * @brief   Type of a structure representing an SEC driver.
 */
typedef struct SECDriver SECDriver;

/**
 * @brief   SECURAM notification callback type.
 *
 * @param[in] secp      pointer to a @p SECDriver object
 */
typedef void (*securam_callback_t)(SECDriver *secp);

/**
 * @brief   SECUMOD notification callback type.
 *
 * @param[in] secp      pointer to a @p SECDriver object
 */
typedef void (*secumod_callback_t)(SECDriver *secp, secevent_t event);

/**
 * @brief   SECUMOD erase callback type.
 *
 * @param[in] secp      pointer to a @p SECDriver object
 */
typedef void (*erased_callback_t)(SECDriver *secp);

/**
 * @brief   Type of RAM access mode.
 */
typedef uint32_t ram_access_mode_t;

/**
 * @brief   PIOBU configuration structure.
 */
typedef struct {
  /*
   * @brief PIOBU pin's index
   */
  uint32_t pinIndex:3;
  /*
   * @brief alarm filter value
   */
  uint32_t afv:4;
  /*
   * @brief reset filter value
   */
  uint32_t rfv:4;
  /*
   * @brief I/O line mode
   * @note 0: pure input, 1: enabled in output
   */
  uint32_t mode:1;
  /*
   * @brief Configure the I/O line in output mode
   * @note 0: clear, 1: set
   */
  uint32_t outputLevel:1;
  /*
   * @brief programmable pull-up state
   * @note 0: none, 1: pull-up; 2: pull-down; 3: reserved
   */
  uint32_t pullUpState:2;
  /*
   * @brief Pull-up/Down Scheduled:
   * @note 0: no; 1: yes
   */
  uint32_t scheduled:1;
  /*
   * @brief switch input default state
   */
  uint32_t inputDefaultLevel:1;
  /*
   * @brief Mode of detection intrusion.
   * @note 0: static, 1: dynamic
   */
  uint32_t dynamic:1;
  /*
   * @brief filter for dynamic signatures input
   * @note 0: 3 stages majority vote, 1: 5 stages
   */
  uint32_t filter3_5:1;
} PIOBUConfig;

typedef struct {
  /**
   * @brief RAM Access Right
   */
  ram_access_mode_t         mode;
  /* End of the mandatory fields.*/
} RAMAccessConfig;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief Callback for memory violation
   */
  securam_callback_t        securam_callback;
  /**
   * @brief Callback after memory erasing
   */
  erased_callback_t          erased_callback;
  /**
   * @brief lenght of PIOBUConfig array
   * @note  Number of pads to configure
   */
  size_t                    length;
  /**
   * @brief pointer to PIOBUConfig array
   */
  PIOBUConfig               *list;
  /**
   * @brief RAM Access Rights
   */
  RAMAccessConfig           region[SECUMOD_RAM_REGIONS];
  /**
   * @brief SECUMOD CR register initialization data.
   */
  uint32_t                  cr;
  /**
   * @brief SECUMOD JTAGCR register initialization data.
   */
  uint32_t                  jtagcr;
} SECConfig;

/**
 * @brief   Structure representing a SEC driver.
 */
struct SECDriver {
  /**
   * @brief Driver state.
   */
  secstate_t                state;
  /**
   * @brief Current configuration data.
   */
  const SECConfig           *config;
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the SECUMOD registers block.
   */
  Secumod                   *sec;
  /**
   * @brief   Callback pointer.
   */
  secumod_callback_t        secumod_callback;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
/**
 * @brief   Switch to Backup Mode.
 *
 * @api
 */
#define secumodSwitch2BackupMode() {                                          \
  SECUMOD->SECUMOD_CR = SECUMOD_CR_BACKUP;                                    \
}

/**
 * @brief   Switch to Normal Mode.
 *
 * @api
 */
#define secumodSwitch2NormalMode() {                                           \
  SECUMOD->SECUMOD_CR = SECUMOD_CR_NORMAL;                                     \
}

/**
 * @brief   Start clear content of SECUMOD internal RAM 4Kbyte and 256bits.
 *
 * @api
 */
#define secumodSoftwareProtection() {                                          \
  SECUMOD->SECUMOD_CR = SECUMOD_CR_SWPROT;                                     \
}

/**
 * @brief Enable/Disable Auto-Backup.
 *
 * @param[in] enable Enable auto-backup if true, disable otherwise.
 *
 * @api
 */
#define secumodSetAutoBackup(enable) {                                         \
  if (enable) {                                                                \
    SECUMOD->SECUMOD_CR = SECUMOD_CR_AUTOBKP_AUTO_SWITCH;                      \
  }                                                                            \
  else {                                                                       \
    SECUMOD->SECUMOD_CR = SECUMOD_CR_AUTOBKP_SW_SWITCH;                        \
  }                                                                            \
}

/**
 * @brief Enable/Disable Memory Scrambling.
 *
 * @param[in] enable Enable memory scrambling if true, disable otherwise.
 *
 * @api
 */
#define secumodSetScrambling(enable) {                                         \
  if (enable) {                                                                \
    SECUMOD->SECUMOD_CR = SECUMOD_CR_SCRAMB_ENABLE;                            \
    }                                                                          \
  else {                                                                       \
    SECUMOD->SECUMOD_CR = SECUMOD_CR_SCRAMB_DISABLE;                           \
  }                                                                            \
}

/**
 * @brief Toggle normal or backup protection registers appear and disappear.
 *
 * @api
 */
#define secumodToggleProtectionReg() {                                         \
  SECUMOD->SECUMOD_CR = SECUMOD_CR_KEY_TOGGLE;                                 \
}

/**
 * @brief Set scrambling key for secure RAM in SECUMOD.
 *
 * @param[in] key Scrambling key.
 *
 * @api
 */
#define secumodSetScramblingKey(key) {                                         \
  SECUMOD->SECUMOD_SCRKEY = key;                                               \
}

/**
 * @brief Get scrambling key for secure RAM in SECUMOD.
 *
 * @return Scrambling key.
 *
 * @api
 */
#define secumodGetScramblingKey() {                                            \
  SECUMOD->SECUMOD_SCRKEY;                                                     \
}

/**
 * @brief Set protections enabled in backup mode.
 *
 * @note Make sure registers appears before call this function, to toggle the
 *       appearance of the registers using secumodToggleProtectionReg().
 *
 * @param[in] sources Bitwise OR of protections.
 *
 * @api
 */
#define secumodSetBackupModeProtections(sources) {                             \
  SECUMOD->SECUMOD_BMPR = sources;                                             \
  if (SECUMOD->SECUMOD_BMPR != sources) {                                      \
    secumodToggleProtectionReg();                                              \
    SECUMOD->SECUMOD_BMPR = sources;                                           \
  }                                                                            \
}

/**
 * @brief Set protections enabled in normal mode.
 *
 * @note Make sure registers appears before call this function, to toggle the
 *       appearance of the registers using secumodToggleProtectionReg().
 *
 * @param[in] sources Bitwise OR of protections.
 *
 * @api
 */
#define secumodSetNormalModeProtections(sources) {                             \
  SECUMOD->SECUMOD_NMPR = sources;                                             \
  if (SECUMOD->SECUMOD_NMPR != sources) {                                      \
    secumodToggleProtectionReg();                                              \
    SECUMOD->SECUMOD_NMPR = sources;                                           \
  }                                                                            \
}

/**
 * @brief Set protection sources which can cause wake up signal generated.
 *
 * @param[in] sources Bitwise OR of protection sources.
 *
 * @api
 */
#define secumodSetWakeupProtections(sources) {                                 \
  SECUMOD->SECUMOD_WKPR = sources;                                             \
}

/**
 * @brief Wait availability status of memory.
 *
 * @api
 */
#define secumodMemReady() {                                                    \
  while (!(SECUMOD->SECUMOD_RAMRDY & SECUMOD_RAMRDY_READY));                   \
}

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/
extern SECDriver SECD0;

#ifdef __cplusplus
extern "C" {
#endif
  void secInit(void);
  void secObjectInit(SECDriver *secp);
  void secStart(SECDriver *secp, const SECConfig *config);
  void secStop(SECDriver *secp);
  void secSetCallback(SECDriver *secp, uint32_t sources, secumod_callback_t callback);
  void secumodSetJtagProtection(bool reset, uint8_t permissions, bool ack);
  void secumodDynamicSignaturesTuning(uint16_t period, uint8_t detectionThr, uint8_t resetThr);
  void secumodPeriodicAlarm(bool enable);
  void secumodSetRamAccessRights(uint32_t region, uint8_t rights);
  uint32_t secumodReadInternalMemory(uint8_t *data, uint32_t addr, uint32_t size);
  uint32_t secumodWriteInternalMemory(uint8_t *data, uint32_t addr, uint32_t size);
#ifdef __cplusplus
}
#endif

#endif /* SAMA_USE_SECUMOD */

#endif /* SAMA_SECUMOD_LLD_H */

/** @} */

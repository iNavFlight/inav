/*
    ChibiOS/RT - Copyright (C) 2014 Uladzimir Pylinsky aka barthess

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
 * @file    hal_onewire.h
 * @brief   1-wire Driver macros and structures.
 *
 * @addtogroup onewire
 * @{
 */

#ifndef HAL_ONEWIRE_H_
#define HAL_ONEWIRE_H_

#if (HAL_USE_ONEWIRE == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
/**
 * @brief   Enable synthetic test for 'search ROM' procedure.
 * @note    Only for debugging/testing!
 */
#define ONEWIRE_SYNTH_SEARCH_TEST         FALSE

/**
 * @brief   Aliases for 1-wire protocol.
 */
#define ONEWIRE_CMD_READ_ROM              0x33
#define ONEWIRE_CMD_SEARCH_ROM            0xF0
#define ONEWIRE_CMD_MATCH_ROM             0x55
#define ONEWIRE_CMD_SKIP_ROM              0xCC
#define ONEWIRE_CMD_CONVERT_TEMP          0x44
#define ONEWIRE_CMD_READ_SCRATCHPAD       0xBE

/**
 * @brief   How many bits will be used for transaction length storage.
 */
#define ONEWIRE_REG_BYTES_WIDTH           16U

/**
 * @brief   Precalculated maximum transaction length.
 */
#define ONEWIRE_MAX_TRANSACTION_LEN       ((1U << ONEWIRE_REG_BYTES_WIDTH) - 1U)

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/
#if ONEWIRE_SYNTH_SEARCH_TEST && !ONEWIRE_USE_SEARCH_ROM
#error "Synthetic search rom test needs ONEWIRE_USE_SEARCH_ROM"
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/
#if !HAL_USE_PWM
#error "1-wire Driver requires HAL_USE_PWM"
#endif

#if !HAL_USE_PAL
#error "1-wire Driver requires HAL_USE_PAL"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/
#if ONEWIRE_USE_STRONG_PULLUP
/**
 * @brief   1-wire strong pull up assert callback type.
 */
typedef void (*onewire_pullup_assert_t)(void);

/**
 * @brief   1-wire strong pull up release callback type.
 */
typedef void (*onewire_pullup_release_t)(void);
#endif /* ONEWIRE_USE_STRONG_PULLUP */

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  ONEWIRE_UNINIT = 0,         /**< Not initialized.                         */
  ONEWIRE_STOP = 1,           /**< Stopped.                                 */
  ONEWIRE_READY = 2,          /**< Ready.                                   */
#if ONEWIRE_USE_STRONG_PULLUP
  ONEWIRE_PULL_UP             /**< Pull up asserted.                        */
#endif
} onewire_state_t;

#if ONEWIRE_USE_SEARCH_ROM
/**
 * @brief   Search ROM procedure possible state.
 */
typedef enum {
  ONEWIRE_SEARCH_ROM_SUCCESS = 0,   /**< ROM successfully discovered.       */
  ONEWIRE_SEARCH_ROM_LAST = 1,      /**< Last ROM successfully discovered.  */
  ONEWIRE_SEARCH_ROM_ERROR = 2      /**< Error happened during search.      */
} search_rom_result_t;

/**
 * @brief   Search ROM procedure iteration enum.
 */
typedef enum {
  ONEWIRE_SEARCH_ROM_FIRST = 0,     /**< First search run.                  */
  ONEWIRE_SEARCH_ROM_NEXT = 1       /**< Next search run.                   */
} search_iteration_t;
#endif /* ONEWIRE_USE_SEARCH_ROM */

/**
 * @brief   Driver configuration structure.
 */
typedef struct {
  /**
   * @brief Pointer to @p PWM driver used for communication.
   */
  PWMDriver                 *pwmd;
   /**
   * @brief Pointer to configuration structure for underlying PWM driver.
   * @note  It is NOT constant because 1-wire driver needs to change them
   *        during normal functioning.
   */
  PWMConfig                 *pwmcfg;
  /**
   * @brief   Active logic level for master channel.
   * @details Just set it to @p PWM_OUTPUT_ACTIVE_LOW when 1-wire bus
   *          connected to direct (not complementary) output of the timer.
   *          In opposite case you need to check documentation to choose
   *          correct value.
   */
  pwmmode_t                 pwmmode;
  /**
   * @brief Number of PWM channel used as master pulse generator.
   */
  size_t                    master_channel;
  /**
   * @brief Number of PWM channel used as sample interrupt generator.
   */
  size_t                    sample_channel;
  /**
   * @brief   Port Identifier.
   * @details This type can be a scalar or some kind of pointer, do not make
   *          any assumption about it, use the provided macros when populating
   *          variables of this type.
   */
  ioportid_t                port;
  /**
   * @brief Digital I/O port pad.
   */
  ioportmask_t              pad;
#if defined(STM32F1XX)
  /**
   * @brief   Digital I/O mode for idle bus.
   * @details This is a kind of workaround against F1x realization of alternate
   *          function. Alternate function mode will be activated only
   *          when you starts appropriate peripheral.
   */
  iomode_t                  pad_mode_idle;
#endif
  /**
   * @brief   Digital I/O mode for active bus.
   */
  iomode_t                  pad_mode_active;
#if ONEWIRE_USE_STRONG_PULLUP
  /**
   * @brief Pointer to function asserting of strong pull up.
   */
  onewire_pullup_assert_t   pullup_assert;
  /**
   * @brief Pointer to function releasing of strong pull up.
   */
  onewire_pullup_release_t  pullup_release;
#endif
} onewireConfig;

#if ONEWIRE_USE_SEARCH_ROM
/**
 * @brief     Search ROM registry. Contains small variables used
 *            in 'search ROM' procedure.
 */
typedef struct {
  /**
   * @brief Bool flag. True when bus has single slave device.
   */
  uint32_t      single_device: 1;
  /**
   * @brief Search iteration (@p search_iteration_t enum).
   */
  uint32_t      search_iter: 1;
  /**
   * @brief Result of discovery procedure (@p search_rom_result_t enum).
   */
  uint32_t      result: 2;
  /**
   * @brief One of 3 steps of bit discovery.
   * @details 0 - direct, 1 - complemented, 2 - generated by master.
   */
  uint32_t      bit_step: 2;
  /**
   * @brief Values acquired during bit discovery.
   */
  uint32_t      bit_buf: 2;
  /**
   * @brief Currently processing ROM bit.
   * @note  Must be big enough to store number 64.
   */
  uint32_t      rombit: 7;
  /**
   * @brief Total device count discovered on bus.
   * @note  Maximum 256.
   */
  uint32_t      devices_found: 8;
} search_rom_reg_t;

/**
 * @brief     Helper structure for 'search ROM' procedure
 */
typedef struct {
  /**
   * @brief   Search ROM registry.
   */
  search_rom_reg_t  reg;
  /**
   * @brief   Pointer to buffer with currently discovering ROM
   */
  uint8_t           *retbuf;
  /**
   * @brief   Previously discovered ROM.
   */
  uint8_t           prev_path[8];
  /**
   * @brief   Last zero turn branch.
   * @note    Negative values use to point out of device tree's root.
   */
  int8_t            last_zero_branch;
  /**
   * @brief   Previous zero turn branch.
   * @note    Negative values use to point out of device tree's root.
   */
  int8_t            prev_zero_branch;
} onewire_search_rom_t;
#endif /* ONEWIRE_USE_SEARCH_ROM */

/**
 * @brief     Onewire registry. Some small variables combined
 *            in single machine word to save RAM.
 */
typedef struct {
#if ONEWIRE_USE_STRONG_PULLUP
  /**
   * @brief   This flag will be asserted by driver to signalizes
   *          ISR part when strong pull up needed.
   */
  uint32_t      need_pullup: 1;
#endif
  /**
   * @brief   Bool flag. If @p true than at least one device presence on bus.
   */
  uint32_t      slave_present: 1;
  /**
   * @brief   Driver internal state (@p onewire_state_t enum).
   */
  uint32_t      state: 2;
  /**
   * @brief   Bit number in currently receiving/sending byte.
   * @note    Must be big enough to store 8.
   */
  uint32_t      bit: 4;
  /**
   * @brief   Bool flag for premature timer stop prevention.
   */
  uint32_t      final_timeslot: 1;
  /**
   * @brief   Bytes number to be processing in current transaction.
   */
  uint32_t      bytes: ONEWIRE_REG_BYTES_WIDTH;
} onewire_reg_t;

/**
 * @brief     Structure representing an 1-wire driver.
 */
typedef struct {
  /**
   * @brief   Onewire registry.
   */
  onewire_reg_t         reg;
  /**
   * @brief   Onewire config.
   */
  const onewireConfig   *config;
  /**
   * @brief   Pointer to I/O data buffer.
   */
  uint8_t               *buf;
#if ONEWIRE_USE_SEARCH_ROM
  /**
   * @brief   Search ROM helper structure.
   */
  onewire_search_rom_t  search_rom;
#endif /* ONEWIRE_USE_SEARCH_ROM */
  /**
   * @brief   Thread waiting for I/O completion.
   */
  thread_reference_t  thread;
} onewireDriver;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern onewireDriver OWD1;

#ifdef __cplusplus
extern "C" {
#endif
  void onewireObjectInit(onewireDriver *owp);
  void onewireStart(onewireDriver *owp, const onewireConfig *config);
  void onewireStop(onewireDriver *owp);
  bool onewireReset(onewireDriver *owp);
  void onewireRead(onewireDriver *owp, uint8_t *rxbuf, size_t rxbytes);
  uint8_t onewireCRC(const uint8_t *buf, size_t len);
  void onewireWrite(onewireDriver *owp, uint8_t *txbuf,
                    size_t txbytes, systime_t pullup_time);
#if ONEWIRE_USE_SEARCH_ROM
  size_t onewireSearchRom(onewireDriver *owp,
                          uint8_t *result, size_t max_rom_cnt);
#endif /* ONEWIRE_USE_SEARCH_ROM */
#if ONEWIRE_SYNTH_SEARCH_TEST
  void _synth_ow_write_bit(onewireDriver *owp, ioline_t bit);
  ioline_t _synth_ow_read_bit(void);
  void synthSearchRomTest(onewireDriver *owp);
#endif /* ONEWIRE_SYNTH_SEARCH_TEST */
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ONEWIRE */

#endif /* HAL_ONEWIRE_H_ */

/** @} */










/*
    ChibiOS/HAL - Copyright (C) 2014 Uladzimir Pylinsky aka barthess

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
 * @file    hal_fsmc.h
 * @brief   FSMC Driver subsystem low level driver header.
 *
 * @addtogroup FSMC
 * @{
 */

#ifndef HAL_FSMC_H_
#define HAL_FSMC_H_

#if (HAL_USE_FSMC == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*
 * (Re)define if needed base address constants supplied in ST's CMSIS
 */
#if (defined(STM32F427xx) || defined(STM32F437xx) || \
     defined(STM32F429xx) || defined(STM32F439xx) || \
     defined(STM32F745xx) || defined(STM32F746xx) || \
     defined(STM32F756xx) || defined(STM32F767xx) || \
     defined(STM32F769xx) || defined(STM32F777xx) || \
     defined(STM32F779xx))
  #if !defined(FSMC_Bank1_R_BASE)
  #define FSMC_Bank1_R_BASE               (FMC_R_BASE + 0x0000)
  #endif
  #if !defined(FSMC_Bank1E_R_BASE)
  #define FSMC_Bank1E_R_BASE              (FMC_R_BASE + 0x0104)
  #endif
  #if !defined(FSMC_Bank2_R_BASE)
  #define FSMC_Bank2_R_BASE               (FMC_R_BASE + 0x0060)
  #endif
  #if !defined(FSMC_Bank3_R_BASE)
  #define FSMC_Bank3_R_BASE               (FMC_R_BASE + 0x0080)
  #endif
  #if !defined(FSMC_Bank4_R_BASE)
  #define FSMC_Bank4_R_BASE               (FMC_R_BASE + 0x00A0)
  #endif
  #if !defined(FSMC_Bank5_R_BASE)
  #define FSMC_Bank5_6_R_BASE             (FMC_R_BASE + 0x0140)
  #endif
#else
  #if !defined(FSMC_Bank1_R_BASE)
  #define FSMC_Bank1_R_BASE               (FSMC_R_BASE + 0x0000)
  #endif
  #if !defined(FSMC_Bank1E_R_BASE)
  #define FSMC_Bank1E_R_BASE              (FSMC_R_BASE + 0x0104)
  #endif
  #if !defined(FSMC_Bank2_R_BASE)
  #define FSMC_Bank2_R_BASE               (FSMC_R_BASE + 0x0060)
  #endif
  #if !defined(FSMC_Bank3_R_BASE)
  #define FSMC_Bank3_R_BASE               (FSMC_R_BASE + 0x0080)
  #endif
  #if !defined(FSMC_Bank4_R_BASE)
  #define FSMC_Bank4_R_BASE               (FSMC_R_BASE + 0x00A0)
  #endif
#endif

/*
 * Base bank mappings
 */
#define FSMC_Bank1_MAP_BASE               ((uint32_t) 0x60000000)
#define FSMC_Bank2_MAP_BASE               ((uint32_t) 0x70000000)
#define FSMC_Bank3_MAP_BASE               ((uint32_t) 0x80000000)
#define FSMC_Bank4_MAP_BASE               ((uint32_t) 0x90000000)
#if (defined(STM32F427xx) || defined(STM32F437xx) || \
     defined(STM32F429xx) || defined(STM32F439xx) || \
     defined(STM32F7))
  #define FSMC_Bank5_MAP_BASE             ((uint32_t) 0xC0000000)
  #define FSMC_Bank6_MAP_BASE             ((uint32_t) 0xD0000000)
#endif

/*
 * Subbunks of bank1
 */
#define FSMC_SUBBUNK_OFFSET           (1024 * 1024 * 64)
#define FSMC_Bank1_1_MAP              (FSMC_Bank1_MAP_BASE)
#define FSMC_Bank1_2_MAP              (FSMC_Bank1_1_MAP + FSMC_SUBBUNK_OFFSET)
#define FSMC_Bank1_3_MAP              (FSMC_Bank1_2_MAP + FSMC_SUBBUNK_OFFSET)
#define FSMC_Bank1_4_MAP              (FSMC_Bank1_3_MAP + FSMC_SUBBUNK_OFFSET)

/*
 * Bank 2 (NAND)
 */
#define FSMC_Bank2_MAP_COMMON             (FSMC_Bank2_MAP_BASE + 0)
#define FSMC_Bank2_MAP_ATTR               (FSMC_Bank2_MAP_BASE + 0x8000000)

#define FSMC_Bank2_MAP_COMMON_DATA        (FSMC_Bank2_MAP_COMMON + 0)
#define FSMC_Bank2_MAP_COMMON_CMD         (FSMC_Bank2_MAP_COMMON + 0x10000)
#define FSMC_Bank2_MAP_COMMON_ADDR        (FSMC_Bank2_MAP_COMMON + 0x20000)

#define FSMC_Bank2_MAP_ATTR_DATA          (FSMC_Bank2_MAP_ATTR + 0)
#define FSMC_Bank2_MAP_ATTR_CMD           (FSMC_Bank2_MAP_ATTR + 0x10000)
#define FSMC_Bank2_MAP_ATTR_ADDR          (FSMC_Bank2_MAP_ATTR + 0x20000)

/*
 * Bank 3 (NAND)
 */
#define FSMC_Bank3_MAP_COMMON             (FSMC_Bank3_MAP_BASE + 0)
#define FSMC_Bank3_MAP_ATTR               (FSMC_Bank3_MAP_BASE + 0x8000000)

#define FSMC_Bank3_MAP_COMMON_DATA        (FSMC_Bank3_MAP_COMMON + 0)
#define FSMC_Bank3_MAP_COMMON_CMD         (FSMC_Bank3_MAP_COMMON + 0x10000)
#define FSMC_Bank3_MAP_COMMON_ADDR        (FSMC_Bank3_MAP_COMMON + 0x20000)

#define FSMC_Bank3_MAP_ATTR_DATA          (FSMC_Bank3_MAP_ATTR + 0)
#define FSMC_Bank3_MAP_ATTR_CMD           (FSMC_Bank3_MAP_ATTR + 0x10000)
#define FSMC_Bank3_MAP_ATTR_ADDR          (FSMC_Bank3_MAP_ATTR + 0x20000)

/*
 * Bank 4 (PC card)
 */
#define FSMC_Bank4_MAP_COMMON             (FSMC_Bank4_MAP_BASE + 0)
#define FSMC_Bank4_MAP_ATTR               (FSMC_Bank4_MAP_BASE + 0x8000000)
#define FSMC_Bank4_MAP_IO                 (FSMC_Bank4_MAP_BASE + 0xC000000)

/*
 * More convenient typedefs than CMSIS has
 */
typedef struct {
  __IO uint32_t PCR;        /**< NAND Flash control */
  __IO uint32_t SR;         /**< NAND Flash FIFO status and interrupt */
  __IO uint32_t PMEM;       /**< NAND Flash Common memory space timing */
  __IO uint32_t PATT;       /**< NAND Flash Attribute memory space timing  */
  uint32_t      RESERVED0;  /**< Reserved, 0x70     */
  __IO uint32_t ECCR;       /**< NAND Flash ECC result registers */
} FSMC_NAND_TypeDef;

typedef struct {
  __IO uint32_t PCR;       /**< PC Card  control */
  __IO uint32_t SR;        /**< PC Card  FIFO status and interrupt */
  __IO uint32_t PMEM;      /**< PC Card  Common memory space timing */
  __IO uint32_t PATT;      /**< PC Card  Attribute memory space timing */
  __IO uint32_t PIO;       /**< PC Card  I/O space timing */
} FSMC_PCCard_TypeDef;

typedef struct {
  __IO uint32_t BCR;          /**< SRAM/NOR chip-select control registers */
  __IO uint32_t BTR;          /**< SRAM/NOR chip-select timing registers */
  uint32_t      RESERVED[63]; /**< Reserved */
  __IO uint32_t BWTR;         /**< SRAM/NOR write timing registers */
} FSMC_SRAM_NOR_TypeDef;

#if (defined(STM32F427xx) || defined(STM32F437xx) || \
     defined(STM32F429xx) || defined(STM32F439xx) || \
     defined(STM32F7))

typedef struct {
  __IO uint32_t SDCR1;              /**< SDRAM control register (bank 1) */
  __IO uint32_t SDCR2;              /**< SDRAM control register (bank 2) */
  __IO uint32_t SDTR1;              /**< SDRAM timing register (bank 1) */
  __IO uint32_t SDTR2;              /**< SDRAM timing register (bank 2) */
  __IO uint32_t SDCMR;              /**< SDRAM comand mode register */
  __IO uint32_t SDRTR;              /**< SDRAM refresh timer register */
  __IO uint32_t SDSR;               /**< SDRAM status register */
} FSMC_SDRAM_TypeDef;

#endif

/**
 * @brief   PCR register
 */
#define  FSMC_PCR_PWAITEN         ((uint32_t)1 << 1)
#define  FSMC_PCR_PBKEN           ((uint32_t)1 << 2)
#define  FSMC_PCR_PTYP            ((uint32_t)1 << 3)
#define  FSMC_PCR_PWID_8          ((uint32_t)0 << 4)
#define  FSMC_PCR_PWID_16         ((uint32_t)1 << 4)
#define  FSMC_PCR_PWID_RESERVED1  ((uint32_t)2 << 4)
#define  FSMC_PCR_PWID_RESERVED2  ((uint32_t)3 << 4)
#define  FSMC_PCR_PWID_MASK       ((uint32_t)3 << 4)
#define  FSMC_PCR_ECCEN           ((uint32_t)1 << 6)
#define  FSMC_PCR_PTYP_PCCARD     0
#define  FSMC_PCR_PTYP_NAND       FSMC_PCR_PTYP

/**
 * @brief   SR register
 */
#define  FSMC_SR_IRS              ((uint8_t)0x01)
#define  FSMC_SR_ILS              ((uint8_t)0x02)
#define  FSMC_SR_IFS              ((uint8_t)0x04)
#define  FSMC_SR_IREN             ((uint8_t)0x08)
#define  FSMC_SR_ILEN             ((uint8_t)0x10)
#define  FSMC_SR_IFEN             ((uint8_t)0x20)
#define  FSMC_SR_FEMPT            ((uint8_t)0x40)
#define  FSMC_SR_ISR_MASK         (FSMC_SR_IRS | FSMC_SR_ILS | FSMC_SR_IFS)

/**
 * @brief   BCR register
 */
#define  FSMC_BCR_MBKEN           ((uint32_t)1 << 0)
#define  FSMC_BCR_MUXEN           ((uint32_t)1 << 1)
#define  FSMC_BCR_MTYP_SRAM       ((uint32_t)0 << 2)
#define  FSMC_BCR_MTYP_PSRAM      ((uint32_t)1 << 2)
#define  FSMC_BCR_MTYP_NOR_NAND   ((uint32_t)2 << 2)
#define  FSMC_BCR_MTYP_RESERVED   ((uint32_t)3 << 2)
#define  FSMC_BCR_MWID_8          ((uint32_t)0 << 4)
#define  FSMC_BCR_MWID_16         ((uint32_t)1 << 4)
#if (defined(STM32F427xx) || defined(STM32F437xx) || \
     defined(STM32F429xx) || defined(STM32F439xx) || \
     defined(STM32F7))
#define  FSMC_BCR_MWID_32         ((uint32_t)2 << 4)
#else
#define  FSMC_BCR_MWID_RESERVED1  ((uint32_t)2 << 4)
#endif
#define  FSMC_BCR_MWID_RESERVED2  ((uint32_t)3 << 4)
#define  FSMC_BCR_FACCEN          ((uint32_t)1 << 6)
#define  FSMC_BCR_BURSTEN         ((uint32_t)1 << 8)
#define  FSMC_BCR_WAITPOL         ((uint32_t)1 << 9)
#define  FSMC_BCR_WRAPMOD         ((uint32_t)1 << 10)
#define  FSMC_BCR_WAITCFG         ((uint32_t)1 << 11)
#define  FSMC_BCR_WREN            ((uint32_t)1 << 12)
#define  FSMC_BCR_WAITEN          ((uint32_t)1 << 13)
#define  FSMC_BCR_EXTMOD          ((uint32_t)1 << 14)
#define  FSMC_BCR_ASYNCWAIT       ((uint32_t)1 << 15)
#define  FSMC_BCR_CBURSTRW        ((uint32_t)1 << 19)
#if (defined(STM32F427xx) || defined(STM32F437xx) || \
     defined(STM32F429xx) || defined(STM32F439xx) || \
     defined(STM32F7))
#define  FSMC_BCR_CCLKEN          ((uint32_t)1 << 20)
#endif
#if (defined(STM32F7))
#define FSMC_BCR_WFDIS            ((uint32_t)1 << 21)
#endif

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   FSMC driver enable switch.
 * @details If set to @p TRUE the support for FSMC is included.
 */
#if !defined(STM32_FSMC_USE_FSMC1) || defined(__DOXYGEN__)
#define STM32_FSMC_USE_FSMC1             FALSE
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/
#if !STM32_FSMC_USE_FSMC1
#error "FSMC driver activated but no FSMC peripheral assigned"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a structure representing an FSMC driver.
 */
typedef struct FSMCDriver FSMCDriver;

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  FSMC_UNINIT = 0,                   /**< Not initialized.                   */
  FSMC_STOP = 1,                     /**< Stopped.                           */
  FSMC_READY = 2,                    /**< Ready.                             */
} fsmcstate_t;

/**
 * @brief   Structure representing an FSMC driver.
 */
struct FSMCDriver {
  /**
   * @brief Driver state.
   */
  fsmcstate_t               state;
  /* End of the mandatory fields.*/

#if STM32_SRAM_USE_FSMC_SRAM1
  FSMC_SRAM_NOR_TypeDef     *sram1;
#endif
#if STM32_SRAM_USE_FSMC_SRAM2
  FSMC_SRAM_NOR_TypeDef     *sram2;
#endif
#if STM32_SRAM_USE_FSMC_SRAM3
  FSMC_SRAM_NOR_TypeDef     *sram3;
#endif
#if STM32_SRAM_USE_FSMC_SRAM4
  FSMC_SRAM_NOR_TypeDef     *sram4;
#endif
#if STM32_NAND_USE_FSMC_NAND1
  FSMC_NAND_TypeDef         *nand1;
#endif
#if STM32_NAND_USE_FSMC_NAND2
  FSMC_NAND_TypeDef         *nand2;
#endif
#if (defined(STM32F427xx) || defined(STM32F437xx) || \
     defined(STM32F429xx) || defined(STM32F439xx) || \
     defined(STM32F7))
  #if STM32_USE_FSMC_SDRAM
    FSMC_SDRAM_TypeDef       *sdram;
  #endif
#endif
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if STM32_FSMC_USE_FSMC1 && !defined(__DOXYGEN__)
extern FSMCDriver FSMCD1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void fsmc_init(void);
  void fsmc_start(FSMCDriver *fsmcp);
  void fsmc_stop(FSMCDriver *fsmcp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_FSMC */

#endif /* HAL_FSMC_H_ */

/** @} */

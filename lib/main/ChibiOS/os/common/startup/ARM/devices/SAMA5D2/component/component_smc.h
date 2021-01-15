/* ---------------------------------------------------------------------------- */
/*                  Atmel Microcontroller Software Support                      */
/*                       SAM Software Package License                           */
/* ---------------------------------------------------------------------------- */
/* Copyright (c) 2015, Atmel Corporation                                        */
/*                                                                              */
/* All rights reserved.                                                         */
/*                                                                              */
/* Redistribution and use in source and binary forms, with or without           */
/* modification, are permitted provided that the following condition is met:    */
/*                                                                              */
/* - Redistributions of source code must retain the above copyright notice,     */
/* this list of conditions and the disclaimer below.                            */
/*                                                                              */
/* Atmel's name may not be used to endorse or promote products derived from     */
/* this software without specific prior written permission.                     */
/*                                                                              */
/* DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR   */
/* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE   */
/* DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,      */
/* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,  */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    */
/* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING         */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, */
/* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                           */
/* ---------------------------------------------------------------------------- */

#ifndef _SAMA5D2_SMC_COMPONENT_
#define _SAMA5D2_SMC_COMPONENT_

/* ============================================================================= */
/**  SOFTWARE API DEFINITION FOR Static Memory Controller */
/* ============================================================================= */
/** \addtogroup SAMA5D2_SMC Static Memory Controller */
/*@{*/

#if !(defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__))
/** \brief SmcCs_number hardware registers */
typedef struct {
  __IO uint32_t HSMC_SETUP;   /**< \brief (SmcCs_number Offset: 0x0) HSMC Setup Register */
  __IO uint32_t HSMC_PULSE;   /**< \brief (SmcCs_number Offset: 0x4) HSMC Pulse Register */
  __IO uint32_t HSMC_CYCLE;   /**< \brief (SmcCs_number Offset: 0x8) HSMC Cycle Register */
  __IO uint32_t HSMC_TIMINGS; /**< \brief (SmcCs_number Offset: 0xC) HSMC Timings Register */
  __IO uint32_t HSMC_MODE;    /**< \brief (SmcCs_number Offset: 0x10) HSMC Mode Register */
} SmcCs_number;
/** \brief SmcPmecc hardware registers */
typedef struct {
  __I uint32_t HSMC_PMECC[14]; /**< \brief (SmcPmecc Offset: 0x0) PMECC Redundancy x Register */
  __I uint32_t Reserved1[2];
} SmcPmecc;
/** \brief SmcRem hardware registers */
typedef struct {
  __I uint32_t HSMC_REM0_;  /**< \brief (SmcRem Offset: 0x0) PMECC Remainder 0 Register */
  __I uint32_t HSMC_REM1_;  /**< \brief (SmcRem Offset: 0x4) PMECC Remainder 1 Register */
  __I uint32_t HSMC_REM2_;  /**< \brief (SmcRem Offset: 0x8) PMECC Remainder 2 Register */
  __I uint32_t HSMC_REM3_;  /**< \brief (SmcRem Offset: 0xC) PMECC Remainder 3 Register */
  __I uint32_t HSMC_REM4_;  /**< \brief (SmcRem Offset: 0x10) PMECC Remainder 4 Register */
  __I uint32_t HSMC_REM5_;  /**< \brief (SmcRem Offset: 0x14) PMECC Remainder 5 Register */
  __I uint32_t HSMC_REM6_;  /**< \brief (SmcRem Offset: 0x18) PMECC Remainder 6 Register */
  __I uint32_t HSMC_REM7_;  /**< \brief (SmcRem Offset: 0x1C) PMECC Remainder 7 Register */
  __I uint32_t HSMC_REM8_;  /**< \brief (SmcRem Offset: 0x20) PMECC Remainder 8 Register */
  __I uint32_t HSMC_REM9_;  /**< \brief (SmcRem Offset: 0x24) PMECC Remainder 9 Register */
  __I uint32_t HSMC_REM10_; /**< \brief (SmcRem Offset: 0x28) PMECC Remainder 10 Register */
  __I uint32_t HSMC_REM11_; /**< \brief (SmcRem Offset: 0x2C) PMECC Remainder 11 Register */
  __I uint32_t HSMC_REM12_; /**< \brief (SmcRem Offset: 0x30) PMECC Remainder 12 Register */
  __I uint32_t HSMC_REM13_; /**< \brief (SmcRem Offset: 0x34) PMECC Remainder 13 Register */
  __I uint32_t HSMC_REM14_; /**< \brief (SmcRem Offset: 0x38) PMECC Remainder 14 Register */
  __I uint32_t HSMC_REM15_; /**< \brief (SmcRem Offset: 0x3C) PMECC Remainder 15 Register */
} SmcRem;
/** \brief Smc hardware registers */
#define SMCPMECC_NUMBER 8
#define SMCREM_NUMBER 8
#define SMCCS_NUMBER_NUMBER 4
typedef struct {
  __IO uint32_t     HSMC_CFG;                           /**< \brief (Smc Offset: 0x000) HSMC NFC Configuration Register */
  __O  uint32_t     HSMC_CTRL;                          /**< \brief (Smc Offset: 0x004) HSMC NFC Control Register */
  __I  uint32_t     HSMC_SR;                            /**< \brief (Smc Offset: 0x008) HSMC NFC Status Register */
  __O  uint32_t     HSMC_IER;                           /**< \brief (Smc Offset: 0x00C) HSMC NFC Interrupt Enable Register */
  __O  uint32_t     HSMC_IDR;                           /**< \brief (Smc Offset: 0x010) HSMC NFC Interrupt Disable Register */
  __I  uint32_t     HSMC_IMR;                           /**< \brief (Smc Offset: 0x014) HSMC NFC Interrupt Mask Register */
  __IO uint32_t     HSMC_ADDR;                          /**< \brief (Smc Offset: 0x018) HSMC NFC Address Cycle Zero Register */
  __IO uint32_t     HSMC_BANK;                          /**< \brief (Smc Offset: 0x01C) HSMC Bank Address Register */
  __I  uint32_t     Reserved1[20];
  __IO uint32_t     HSMC_PMECCFG;                       /**< \brief (Smc Offset: 0x070) PMECC Configuration Register */
  __IO uint32_t     HSMC_PMECCSAREA;                    /**< \brief (Smc Offset: 0x074) PMECC Spare Area Size Register */
  __IO uint32_t     HSMC_PMECCSADDR;                    /**< \brief (Smc Offset: 0x078) PMECC Start Address Register */
  __IO uint32_t     HSMC_PMECCEADDR;                    /**< \brief (Smc Offset: 0x07C) PMECC End Address Register */
  __I  uint32_t     Reserved2[1];
  __O  uint32_t     HSMC_PMECCTRL;                      /**< \brief (Smc Offset: 0x084) PMECC Control Register */
  __I  uint32_t     HSMC_PMECCSR;                       /**< \brief (Smc Offset: 0x088) PMECC Status Register */
  __O  uint32_t     HSMC_PMECCIER;                      /**< \brief (Smc Offset: 0x08C) PMECC Interrupt Enable register */
  __O  uint32_t     HSMC_PMECCIDR;                      /**< \brief (Smc Offset: 0x090) PMECC Interrupt Disable Register */
  __I  uint32_t     HSMC_PMECCIMR;                      /**< \brief (Smc Offset: 0x094) PMECC Interrupt Mask Register */
  __I  uint32_t     HSMC_PMECCISR;                      /**< \brief (Smc Offset: 0x098) PMECC Interrupt Status Register */
  __I  uint32_t     Reserved3[5];
       SmcPmecc     SMC_PMECC[SMCPMECC_NUMBER];         /**< \brief (Smc Offset: 0xB0) sec_num = 0 .. 7 */
       SmcRem       SMC_REM[SMCREM_NUMBER];             /**< \brief (Smc Offset: 0x2B0) sec_num = 0 .. 7 */
  __I  uint32_t     Reserved4[20];
  __IO uint32_t     HSMC_ELCFG;                         /**< \brief (Smc Offset: 0x500) PMECC Error Location Configuration Register */
  __I  uint32_t     HSMC_ELPRIM;                        /**< \brief (Smc Offset: 0x504) PMECC Error Location Primitive Register */
  __O  uint32_t     HSMC_ELEN;                          /**< \brief (Smc Offset: 0x508) PMECC Error Location Enable Register */
  __O  uint32_t     HSMC_ELDIS;                         /**< \brief (Smc Offset: 0x50C) PMECC Error Location Disable Register */
  __I  uint32_t     HSMC_ELSR;                          /**< \brief (Smc Offset: 0x510) PMECC Error Location Status Register */
  __O  uint32_t     HSMC_ELIER;                         /**< \brief (Smc Offset: 0x514) PMECC Error Location Interrupt Enable register */
  __O  uint32_t     HSMC_ELIDR;                         /**< \brief (Smc Offset: 0x518) PMECC Error Location Interrupt Disable Register */
  __I  uint32_t     HSMC_ELIMR;                         /**< \brief (Smc Offset: 0x51C) PMECC Error Location Interrupt Mask Register */
  __I  uint32_t     HSMC_ELISR;                         /**< \brief (Smc Offset: 0x520) PMECC Error Location Interrupt Status Register */
  __I  uint32_t     Reserved5[1];
  __IO uint32_t     HSMC_SIGMA[33];                     /**< \brief (Smc Offset: 0x528) PMECC Error Location SIGMA x Register */
  __I  uint32_t     HSMC_ERRLOC[32];                    /**< \brief (Smc Offset: 0x5AC) PMECC Error Location x Register */
  __I  uint32_t     Reserved6[53];
       SmcCs_number SMC_CS_NUMBER[SMCCS_NUMBER_NUMBER]; /**< \brief (Smc Offset: 0x700) CS_number = 0 .. 3 */
  __I  uint32_t     Reserved7[20];
  __IO uint32_t     HSMC_OCMS;                          /**< \brief (Smc Offset: 0x7A0) HSMC Off Chip Memory Scrambling Register */
  __O  uint32_t     HSMC_KEY1;                          /**< \brief (Smc Offset: 0x7A4) HSMC Off Chip Memory Scrambling KEY1 Register */
  __O  uint32_t     HSMC_KEY2;                          /**< \brief (Smc Offset: 0x7A8) HSMC Off Chip Memory Scrambling KEY2 Register */
  __I  uint32_t     Reserved8[14];
  __IO uint32_t     HSMC_WPMR;                          /**< \brief (Smc Offset: 0x7E4) HSMC Write Protection Mode Register */
  __I  uint32_t     HSMC_WPSR;                          /**< \brief (Smc Offset: 0x7E8) HSMC Write Protection Status Register */
  __I  uint32_t     Reserved9[4];
  __I  uint32_t     HSMC_VERSION;                       /**< \brief (Smc Offset: 0x7FC) HSMC Version Register */
} Smc;
#endif /* !(defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__)) */
/* -------- HSMC_CFG : (SMC Offset: 0x000) HSMC NFC Configuration Register -------- */
#define HSMC_CFG_PAGESIZE_Pos 0
#define HSMC_CFG_PAGESIZE_Msk (0x7u << HSMC_CFG_PAGESIZE_Pos) /**< \brief (HSMC_CFG) Page Size of the NAND Flash Device */
#define HSMC_CFG_PAGESIZE(value) ((HSMC_CFG_PAGESIZE_Msk & ((value) << HSMC_CFG_PAGESIZE_Pos)))
#define   HSMC_CFG_PAGESIZE_PS512 (0x0u << 0) /**< \brief (HSMC_CFG) Main area 512 bytes */
#define   HSMC_CFG_PAGESIZE_PS1024 (0x1u << 0) /**< \brief (HSMC_CFG) Main area 1024 bytes */
#define   HSMC_CFG_PAGESIZE_PS2048 (0x2u << 0) /**< \brief (HSMC_CFG) Main area 2048 bytes */
#define   HSMC_CFG_PAGESIZE_PS4096 (0x3u << 0) /**< \brief (HSMC_CFG) Main area 4096 bytes */
#define   HSMC_CFG_PAGESIZE_PS8192 (0x4u << 0) /**< \brief (HSMC_CFG) Main area 8192 bytes */
#define HSMC_CFG_WSPARE (0x1u << 8) /**< \brief (HSMC_CFG) Write Spare Area */
#define HSMC_CFG_RSPARE (0x1u << 9) /**< \brief (HSMC_CFG) Read Spare Area */
#define HSMC_CFG_EDGECTRL (0x1u << 12) /**< \brief (HSMC_CFG) Rising/Falling Edge Detection Control */
#define HSMC_CFG_RBEDGE (0x1u << 13) /**< \brief (HSMC_CFG) Ready/Busy Signal Edge Detection */
#define HSMC_CFG_DTOCYC_Pos 16
#define HSMC_CFG_DTOCYC_Msk (0xfu << HSMC_CFG_DTOCYC_Pos) /**< \brief (HSMC_CFG) Data Timeout Cycle Number */
#define HSMC_CFG_DTOCYC(value) ((HSMC_CFG_DTOCYC_Msk & ((value) << HSMC_CFG_DTOCYC_Pos)))
#define HSMC_CFG_DTOMUL_Pos 20
#define HSMC_CFG_DTOMUL_Msk (0x7u << HSMC_CFG_DTOMUL_Pos) /**< \brief (HSMC_CFG) Data Timeout Multiplier */
#define HSMC_CFG_DTOMUL(value) ((HSMC_CFG_DTOMUL_Msk & ((value) << HSMC_CFG_DTOMUL_Pos)))
#define   HSMC_CFG_DTOMUL_X1 (0x0u << 20) /**< \brief (HSMC_CFG) DTOCYC */
#define   HSMC_CFG_DTOMUL_X16 (0x1u << 20) /**< \brief (HSMC_CFG) DTOCYC x 16 */
#define   HSMC_CFG_DTOMUL_X128 (0x2u << 20) /**< \brief (HSMC_CFG) DTOCYC x 128 */
#define   HSMC_CFG_DTOMUL_X256 (0x3u << 20) /**< \brief (HSMC_CFG) DTOCYC x 256 */
#define   HSMC_CFG_DTOMUL_X1024 (0x4u << 20) /**< \brief (HSMC_CFG) DTOCYC x 1024 */
#define   HSMC_CFG_DTOMUL_X4096 (0x5u << 20) /**< \brief (HSMC_CFG) DTOCYC x 4096 */
#define   HSMC_CFG_DTOMUL_X65536 (0x6u << 20) /**< \brief (HSMC_CFG) DTOCYC x 65536 */
#define   HSMC_CFG_DTOMUL_X1048576 (0x7u << 20) /**< \brief (HSMC_CFG) DTOCYC x 1048576 */
#define HSMC_CFG_NFCSPARESIZE_Pos 24
#define HSMC_CFG_NFCSPARESIZE_Msk (0x7fu << HSMC_CFG_NFCSPARESIZE_Pos) /**< \brief (HSMC_CFG) NAND Flash Spare Area Size Retrieved by the Host Controller */
#define HSMC_CFG_NFCSPARESIZE(value) ((HSMC_CFG_NFCSPARESIZE_Msk & ((value) << HSMC_CFG_NFCSPARESIZE_Pos)))
/* -------- HSMC_CTRL : (SMC Offset: 0x004) HSMC NFC Control Register -------- */
#define HSMC_CTRL_NFCEN (0x1u << 0) /**< \brief (HSMC_CTRL) NAND Flash Controller Enable */
#define HSMC_CTRL_NFCDIS (0x1u << 1) /**< \brief (HSMC_CTRL) NAND Flash Controller Disable */
/* -------- HSMC_SR : (SMC Offset: 0x008) HSMC NFC Status Register -------- */
#define HSMC_SR_SMCSTS (0x1u << 0) /**< \brief (HSMC_SR) NAND Flash Controller Status (this field cannot be reset) */
#define HSMC_SR_RB_RISE (0x1u << 4) /**< \brief (HSMC_SR) Selected Ready Busy Rising Edge Detected */
#define HSMC_SR_RB_FALL (0x1u << 5) /**< \brief (HSMC_SR) Selected Ready Busy Falling Edge Detected */
#define HSMC_SR_NFCBUSY (0x1u << 8) /**< \brief (HSMC_SR) NFC Busy (this field cannot be reset) */
#define HSMC_SR_NFCWR (0x1u << 11) /**< \brief (HSMC_SR) NFC Write/Read Operation (this field cannot be reset) */
#define HSMC_SR_NFCSID_Pos 12
#define HSMC_SR_NFCSID_Msk (0x7u << HSMC_SR_NFCSID_Pos) /**< \brief (HSMC_SR) NFC Chip Select ID (this field cannot be reset) */
#define HSMC_SR_XFRDONE (0x1u << 16) /**< \brief (HSMC_SR) NFC Data Transfer Terminated */
#define HSMC_SR_CMDDONE (0x1u << 17) /**< \brief (HSMC_SR) Command Done */
#define HSMC_SR_DTOE (0x1u << 20) /**< \brief (HSMC_SR) Data Timeout Error */
#define HSMC_SR_UNDEF (0x1u << 21) /**< \brief (HSMC_SR) Undefined Area Error */
#define HSMC_SR_AWB (0x1u << 22) /**< \brief (HSMC_SR) Accessing While Busy */
#define HSMC_SR_NFCASE (0x1u << 23) /**< \brief (HSMC_SR) NFC Access Size Error */
#define HSMC_SR_RB_EDGE3 (0x1u << 27) /**< \brief (HSMC_SR) Ready/Busy Line 3 Edge Detected */
/* -------- HSMC_IER : (SMC Offset: 0x00C) HSMC NFC Interrupt Enable Register -------- */
#define HSMC_IER_RB_RISE (0x1u << 4) /**< \brief (HSMC_IER) Ready Busy Rising Edge Detection Interrupt Enable */
#define HSMC_IER_RB_FALL (0x1u << 5) /**< \brief (HSMC_IER) Ready Busy Falling Edge Detection Interrupt Enable */
#define HSMC_IER_XFRDONE (0x1u << 16) /**< \brief (HSMC_IER) Transfer Done Interrupt Enable */
#define HSMC_IER_CMDDONE (0x1u << 17) /**< \brief (HSMC_IER) Command Done Interrupt Enable */
#define HSMC_IER_DTOE (0x1u << 20) /**< \brief (HSMC_IER) Data Timeout Error Interrupt Enable */
#define HSMC_IER_UNDEF (0x1u << 21) /**< \brief (HSMC_IER) Undefined Area Access Interrupt Enable */
#define HSMC_IER_AWB (0x1u << 22) /**< \brief (HSMC_IER) Accessing While Busy Interrupt Enable */
#define HSMC_IER_NFCASE (0x1u << 23) /**< \brief (HSMC_IER) NFC Access Size Error Interrupt Enable */
#define HSMC_IER_RB_EDGE3 (0x1u << 27) /**< \brief (HSMC_IER) Ready/Busy Line 3 Interrupt Enable */
/* -------- HSMC_IDR : (SMC Offset: 0x010) HSMC NFC Interrupt Disable Register -------- */
#define HSMC_IDR_RB_RISE (0x1u << 4) /**< \brief (HSMC_IDR) Ready Busy Rising Edge Detection Interrupt Disable */
#define HSMC_IDR_RB_FALL (0x1u << 5) /**< \brief (HSMC_IDR) Ready Busy Falling Edge Detection Interrupt Disable */
#define HSMC_IDR_XFRDONE (0x1u << 16) /**< \brief (HSMC_IDR) Transfer Done Interrupt Disable */
#define HSMC_IDR_CMDDONE (0x1u << 17) /**< \brief (HSMC_IDR) Command Done Interrupt Disable */
#define HSMC_IDR_DTOE (0x1u << 20) /**< \brief (HSMC_IDR) Data Timeout Error Interrupt Disable */
#define HSMC_IDR_UNDEF (0x1u << 21) /**< \brief (HSMC_IDR) Undefined Area Access Interrupt Disable */
#define HSMC_IDR_AWB (0x1u << 22) /**< \brief (HSMC_IDR) Accessing While Busy Interrupt Disable */
#define HSMC_IDR_NFCASE (0x1u << 23) /**< \brief (HSMC_IDR) NFC Access Size Error Interrupt Disable */
#define HSMC_IDR_RB_EDGE3 (0x1u << 27) /**< \brief (HSMC_IDR) Ready/Busy Line 3 Interrupt Disable */
/* -------- HSMC_IMR : (SMC Offset: 0x014) HSMC NFC Interrupt Mask Register -------- */
#define HSMC_IMR_RB_RISE (0x1u << 4) /**< \brief (HSMC_IMR) Ready Busy Rising Edge Detection Interrupt Mask */
#define HSMC_IMR_RB_FALL (0x1u << 5) /**< \brief (HSMC_IMR) Ready Busy Falling Edge Detection Interrupt Mask */
#define HSMC_IMR_XFRDONE (0x1u << 16) /**< \brief (HSMC_IMR) Transfer Done Interrupt Mask */
#define HSMC_IMR_CMDDONE (0x1u << 17) /**< \brief (HSMC_IMR) Command Done Interrupt Mask */
#define HSMC_IMR_DTOE (0x1u << 20) /**< \brief (HSMC_IMR) Data Timeout Error Interrupt Mask */
#define HSMC_IMR_UNDEF (0x1u << 21) /**< \brief (HSMC_IMR) Undefined Area Access Interrupt Mask5 */
#define HSMC_IMR_AWB (0x1u << 22) /**< \brief (HSMC_IMR) Accessing While Busy Interrupt Mask */
#define HSMC_IMR_NFCASE (0x1u << 23) /**< \brief (HSMC_IMR) NFC Access Size Error Interrupt Mask */
#define HSMC_IMR_RB_EDGE3 (0x1u << 27) /**< \brief (HSMC_IMR) Ready/Busy Line 3 Interrupt Mask */
/* -------- HSMC_ADDR : (SMC Offset: 0x018) HSMC NFC Address Cycle Zero Register -------- */
#define HSMC_ADDR_ADDR_CYCLE0_Pos 0
#define HSMC_ADDR_ADDR_CYCLE0_Msk (0xffu << HSMC_ADDR_ADDR_CYCLE0_Pos) /**< \brief (HSMC_ADDR) NAND Flash Array Address Cycle 0 */
#define HSMC_ADDR_ADDR_CYCLE0(value) ((HSMC_ADDR_ADDR_CYCLE0_Msk & ((value) << HSMC_ADDR_ADDR_CYCLE0_Pos)))
/* -------- HSMC_BANK : (SMC Offset: 0x01C) HSMC Bank Address Register -------- */
#define HSMC_BANK_BANK (0x1u << 0) /**< \brief (HSMC_BANK) Bank Identifier */
/* -------- HSMC_PMECCFG : (SMC Offset: 0x070) PMECC Configuration Register -------- */
#define HSMC_PMECCFG_BCH_ERR_Pos 0
#define HSMC_PMECCFG_BCH_ERR_Msk (0x7u << HSMC_PMECCFG_BCH_ERR_Pos) /**< \brief (HSMC_PMECCFG) Error Correcting Capability */
#define HSMC_PMECCFG_BCH_ERR(value) ((HSMC_PMECCFG_BCH_ERR_Msk & ((value) << HSMC_PMECCFG_BCH_ERR_Pos)))
#define   HSMC_PMECCFG_BCH_ERR_BCH_ERR2 (0x0u << 0) /**< \brief (HSMC_PMECCFG) 2 errors */
#define   HSMC_PMECCFG_BCH_ERR_BCH_ERR4 (0x1u << 0) /**< \brief (HSMC_PMECCFG) 4 errors */
#define   HSMC_PMECCFG_BCH_ERR_BCH_ERR8 (0x2u << 0) /**< \brief (HSMC_PMECCFG) 8 errors */
#define   HSMC_PMECCFG_BCH_ERR_BCH_ERR12 (0x3u << 0) /**< \brief (HSMC_PMECCFG) 12 errors */
#define   HSMC_PMECCFG_BCH_ERR_BCH_ERR24 (0x4u << 0) /**< \brief (HSMC_PMECCFG) 24 errors */
#define   HSMC_PMECCFG_BCH_ERR_BCH_ERR32 (0x5u << 0) /**< \brief (HSMC_PMECCFG) 32 errors */
#define HSMC_PMECCFG_SECTORSZ (0x1u << 4) /**< \brief (HSMC_PMECCFG) Sector Size */
#define HSMC_PMECCFG_PAGESIZE_Pos 8
#define HSMC_PMECCFG_PAGESIZE_Msk (0x3u << HSMC_PMECCFG_PAGESIZE_Pos) /**< \brief (HSMC_PMECCFG) Number of Sectors in the Page */
#define HSMC_PMECCFG_PAGESIZE(value) ((HSMC_PMECCFG_PAGESIZE_Msk & ((value) << HSMC_PMECCFG_PAGESIZE_Pos)))
#define   HSMC_PMECCFG_PAGESIZE_PAGESIZE_1SEC (0x0u << 8) /**< \brief (HSMC_PMECCFG) 1 sector for main area (512 or 1024 bytes) */
#define   HSMC_PMECCFG_PAGESIZE_PAGESIZE_2SEC (0x1u << 8) /**< \brief (HSMC_PMECCFG) 2 sectors for main area (1024 or 2048 bytes) */
#define   HSMC_PMECCFG_PAGESIZE_PAGESIZE_4SEC (0x2u << 8) /**< \brief (HSMC_PMECCFG) 4 sectors for main area (2048 or 4096 bytes) */
#define   HSMC_PMECCFG_PAGESIZE_PAGESIZE_8SEC (0x3u << 8) /**< \brief (HSMC_PMECCFG) 8 sectors for main area (4096 or 8192 bytes) */
#define HSMC_PMECCFG_NANDWR (0x1u << 12) /**< \brief (HSMC_PMECCFG) NAND Write Access */
#define HSMC_PMECCFG_SPAREEN (0x1u << 16) /**< \brief (HSMC_PMECCFG) Spare Enable */
#define HSMC_PMECCFG_AUTO (0x1u << 20) /**< \brief (HSMC_PMECCFG) Automatic Mode Enable */
/* -------- HSMC_PMECCSAREA : (SMC Offset: 0x074) PMECC Spare Area Size Register -------- */
#define HSMC_PMECCSAREA_SPARESIZE_Pos 0
#define HSMC_PMECCSAREA_SPARESIZE_Msk (0x1ffu << HSMC_PMECCSAREA_SPARESIZE_Pos) /**< \brief (HSMC_PMECCSAREA) Spare Area Size */
#define HSMC_PMECCSAREA_SPARESIZE(value) ((HSMC_PMECCSAREA_SPARESIZE_Msk & ((value) << HSMC_PMECCSAREA_SPARESIZE_Pos)))
/* -------- HSMC_PMECCSADDR : (SMC Offset: 0x078) PMECC Start Address Register -------- */
#define HSMC_PMECCSADDR_STARTADDR_Pos 0
#define HSMC_PMECCSADDR_STARTADDR_Msk (0x1ffu << HSMC_PMECCSADDR_STARTADDR_Pos) /**< \brief (HSMC_PMECCSADDR) ECC Area Start Address */
#define HSMC_PMECCSADDR_STARTADDR(value) ((HSMC_PMECCSADDR_STARTADDR_Msk & ((value) << HSMC_PMECCSADDR_STARTADDR_Pos)))
/* -------- HSMC_PMECCEADDR : (SMC Offset: 0x07C) PMECC End Address Register -------- */
#define HSMC_PMECCEADDR_ENDADDR_Pos 0
#define HSMC_PMECCEADDR_ENDADDR_Msk (0x1ffu << HSMC_PMECCEADDR_ENDADDR_Pos) /**< \brief (HSMC_PMECCEADDR) ECC Area End Address */
#define HSMC_PMECCEADDR_ENDADDR(value) ((HSMC_PMECCEADDR_ENDADDR_Msk & ((value) << HSMC_PMECCEADDR_ENDADDR_Pos)))
/* -------- HSMC_PMECCTRL : (SMC Offset: 0x084) PMECC Control Register -------- */
#define HSMC_PMECCTRL_RST (0x1u << 0) /**< \brief (HSMC_PMECCTRL) Reset the PMECC Module */
#define HSMC_PMECCTRL_DATA (0x1u << 1) /**< \brief (HSMC_PMECCTRL) Start a Data Phase */
#define HSMC_PMECCTRL_USER (0x1u << 2) /**< \brief (HSMC_PMECCTRL) Start a User Mode Phase */
#define HSMC_PMECCTRL_ENABLE (0x1u << 4) /**< \brief (HSMC_PMECCTRL) PMECC Enable */
#define HSMC_PMECCTRL_DISABLE (0x1u << 5) /**< \brief (HSMC_PMECCTRL) PMECC Enable */
/* -------- HSMC_PMECCSR : (SMC Offset: 0x088) PMECC Status Register -------- */
#define HSMC_PMECCSR_BUSY (0x1u << 0) /**< \brief (HSMC_PMECCSR) The kernel of the PMECC is busy */
#define HSMC_PMECCSR_ENABLE (0x1u << 4) /**< \brief (HSMC_PMECCSR) PMECC Enable bit */
/* -------- HSMC_PMECCIER : (SMC Offset: 0x08C) PMECC Interrupt Enable register -------- */
#define HSMC_PMECCIER_ERRIE (0x1u << 0) /**< \brief (HSMC_PMECCIER) Error Interrupt Enable */
/* -------- HSMC_PMECCIDR : (SMC Offset: 0x090) PMECC Interrupt Disable Register -------- */
#define HSMC_PMECCIDR_ERRID (0x1u << 0) /**< \brief (HSMC_PMECCIDR) Error Interrupt Disable */
/* -------- HSMC_PMECCIMR : (SMC Offset: 0x094) PMECC Interrupt Mask Register -------- */
#define HSMC_PMECCIMR_ERRIM (0x1u << 0) /**< \brief (HSMC_PMECCIMR) Error Interrupt Mask */
/* -------- HSMC_PMECCISR : (SMC Offset: 0x098) PMECC Interrupt Status Register -------- */
#define HSMC_PMECCISR_ERRIS_Pos 0
#define HSMC_PMECCISR_ERRIS_Msk (0xffu << HSMC_PMECCISR_ERRIS_Pos) /**< \brief (HSMC_PMECCISR) Error Interrupt Status Register */
/* -------- HSMC_PMECC[14] : (SMC Offset: N/A) PMECC Redundancy x Register -------- */
#define HSMC_PMECC_ECC_Pos 0
#define HSMC_PMECC_ECC_Msk (0xffffffffu << HSMC_PMECC_ECC_Pos) /**< \brief (HSMC_PMECC[14]) BCH Redundancy */
/* -------- HSMC_REM0_ : (SMC Offset: N/A) PMECC Remainder 0 Register -------- */
#define HSMC_REM0__REM2NP1_Pos 0
#define HSMC_REM0__REM2NP1_Msk (0x3fffu << HSMC_REM0__REM2NP1_Pos) /**< \brief (HSMC_REM0_) BCH Remainder 2 * N + 1 */
#define HSMC_REM0__REM2NP3_Pos 16
#define HSMC_REM0__REM2NP3_Msk (0x3fffu << HSMC_REM0__REM2NP3_Pos) /**< \brief (HSMC_REM0_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_REM1_ : (SMC Offset: N/A) PMECC Remainder 1 Register -------- */
#define HSMC_REM1__REM2NP1_Pos 0
#define HSMC_REM1__REM2NP1_Msk (0x3fffu << HSMC_REM1__REM2NP1_Pos) /**< \brief (HSMC_REM1_) BCH Remainder 2 * N + 1 */
#define HSMC_REM1__REM2NP3_Pos 16
#define HSMC_REM1__REM2NP3_Msk (0x3fffu << HSMC_REM1__REM2NP3_Pos) /**< \brief (HSMC_REM1_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_REM2_ : (SMC Offset: N/A) PMECC Remainder 2 Register -------- */
#define HSMC_REM2__REM2NP1_Pos 0
#define HSMC_REM2__REM2NP1_Msk (0x3fffu << HSMC_REM2__REM2NP1_Pos) /**< \brief (HSMC_REM2_) BCH Remainder 2 * N + 1 */
#define HSMC_REM2__REM2NP3_Pos 16
#define HSMC_REM2__REM2NP3_Msk (0x3fffu << HSMC_REM2__REM2NP3_Pos) /**< \brief (HSMC_REM2_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_REM3_ : (SMC Offset: N/A) PMECC Remainder 3 Register -------- */
#define HSMC_REM3__REM2NP1_Pos 0
#define HSMC_REM3__REM2NP1_Msk (0x3fffu << HSMC_REM3__REM2NP1_Pos) /**< \brief (HSMC_REM3_) BCH Remainder 2 * N + 1 */
#define HSMC_REM3__REM2NP3_Pos 16
#define HSMC_REM3__REM2NP3_Msk (0x3fffu << HSMC_REM3__REM2NP3_Pos) /**< \brief (HSMC_REM3_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_REM4_ : (SMC Offset: N/A) PMECC Remainder 4 Register -------- */
#define HSMC_REM4__REM2NP1_Pos 0
#define HSMC_REM4__REM2NP1_Msk (0x3fffu << HSMC_REM4__REM2NP1_Pos) /**< \brief (HSMC_REM4_) BCH Remainder 2 * N + 1 */
#define HSMC_REM4__REM2NP3_Pos 16
#define HSMC_REM4__REM2NP3_Msk (0x3fffu << HSMC_REM4__REM2NP3_Pos) /**< \brief (HSMC_REM4_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_REM5_ : (SMC Offset: N/A) PMECC Remainder 5 Register -------- */
#define HSMC_REM5__REM2NP1_Pos 0
#define HSMC_REM5__REM2NP1_Msk (0x3fffu << HSMC_REM5__REM2NP1_Pos) /**< \brief (HSMC_REM5_) BCH Remainder 2 * N + 1 */
#define HSMC_REM5__REM2NP3_Pos 16
#define HSMC_REM5__REM2NP3_Msk (0x3fffu << HSMC_REM5__REM2NP3_Pos) /**< \brief (HSMC_REM5_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_REM6_ : (SMC Offset: N/A) PMECC Remainder 6 Register -------- */
#define HSMC_REM6__REM2NP1_Pos 0
#define HSMC_REM6__REM2NP1_Msk (0x3fffu << HSMC_REM6__REM2NP1_Pos) /**< \brief (HSMC_REM6_) BCH Remainder 2 * N + 1 */
#define HSMC_REM6__REM2NP3_Pos 16
#define HSMC_REM6__REM2NP3_Msk (0x3fffu << HSMC_REM6__REM2NP3_Pos) /**< \brief (HSMC_REM6_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_REM7_ : (SMC Offset: N/A) PMECC Remainder 7 Register -------- */
#define HSMC_REM7__REM2NP1_Pos 0
#define HSMC_REM7__REM2NP1_Msk (0x3fffu << HSMC_REM7__REM2NP1_Pos) /**< \brief (HSMC_REM7_) BCH Remainder 2 * N + 1 */
#define HSMC_REM7__REM2NP3_Pos 16
#define HSMC_REM7__REM2NP3_Msk (0x3fffu << HSMC_REM7__REM2NP3_Pos) /**< \brief (HSMC_REM7_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_REM8_ : (SMC Offset: N/A) PMECC Remainder 8 Register -------- */
#define HSMC_REM8__REM2NP1_Pos 0
#define HSMC_REM8__REM2NP1_Msk (0x3fffu << HSMC_REM8__REM2NP1_Pos) /**< \brief (HSMC_REM8_) BCH Remainder 2 * N + 1 */
#define HSMC_REM8__REM2NP3_Pos 16
#define HSMC_REM8__REM2NP3_Msk (0x3fffu << HSMC_REM8__REM2NP3_Pos) /**< \brief (HSMC_REM8_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_REM9_ : (SMC Offset: N/A) PMECC Remainder 9 Register -------- */
#define HSMC_REM9__REM2NP1_Pos 0
#define HSMC_REM9__REM2NP1_Msk (0x3fffu << HSMC_REM9__REM2NP1_Pos) /**< \brief (HSMC_REM9_) BCH Remainder 2 * N + 1 */
#define HSMC_REM9__REM2NP3_Pos 16
#define HSMC_REM9__REM2NP3_Msk (0x3fffu << HSMC_REM9__REM2NP3_Pos) /**< \brief (HSMC_REM9_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_REM10_ : (SMC Offset: N/A) PMECC Remainder 10 Register -------- */
#define HSMC_REM10__REM2NP1_Pos 0
#define HSMC_REM10__REM2NP1_Msk (0x3fffu << HSMC_REM10__REM2NP1_Pos) /**< \brief (HSMC_REM10_) BCH Remainder 2 * N + 1 */
#define HSMC_REM10__REM2NP3_Pos 16
#define HSMC_REM10__REM2NP3_Msk (0x3fffu << HSMC_REM10__REM2NP3_Pos) /**< \brief (HSMC_REM10_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_REM11_ : (SMC Offset: N/A) PMECC Remainder 11 Register -------- */
#define HSMC_REM11__REM2NP1_Pos 0
#define HSMC_REM11__REM2NP1_Msk (0x3fffu << HSMC_REM11__REM2NP1_Pos) /**< \brief (HSMC_REM11_) BCH Remainder 2 * N + 1 */
#define HSMC_REM11__REM2NP3_Pos 16
#define HSMC_REM11__REM2NP3_Msk (0x3fffu << HSMC_REM11__REM2NP3_Pos) /**< \brief (HSMC_REM11_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_REM12_ : (SMC Offset: N/A) PMECC Remainder 12 Register -------- */
#define HSMC_REM12__REM2NP1_Pos 0
#define HSMC_REM12__REM2NP1_Msk (0x3fffu << HSMC_REM12__REM2NP1_Pos) /**< \brief (HSMC_REM12_) BCH Remainder 2 * N + 1 */
#define HSMC_REM12__REM2NP3_Pos 16
#define HSMC_REM12__REM2NP3_Msk (0x3fffu << HSMC_REM12__REM2NP3_Pos) /**< \brief (HSMC_REM12_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_REM13_ : (SMC Offset: N/A) PMECC Remainder 13 Register -------- */
#define HSMC_REM13__REM2NP1_Pos 0
#define HSMC_REM13__REM2NP1_Msk (0x3fffu << HSMC_REM13__REM2NP1_Pos) /**< \brief (HSMC_REM13_) BCH Remainder 2 * N + 1 */
#define HSMC_REM13__REM2NP3_Pos 16
#define HSMC_REM13__REM2NP3_Msk (0x3fffu << HSMC_REM13__REM2NP3_Pos) /**< \brief (HSMC_REM13_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_REM14_ : (SMC Offset: N/A) PMECC Remainder 14 Register -------- */
#define HSMC_REM14__REM2NP1_Pos 0
#define HSMC_REM14__REM2NP1_Msk (0x3fffu << HSMC_REM14__REM2NP1_Pos) /**< \brief (HSMC_REM14_) BCH Remainder 2 * N + 1 */
#define HSMC_REM14__REM2NP3_Pos 16
#define HSMC_REM14__REM2NP3_Msk (0x3fffu << HSMC_REM14__REM2NP3_Pos) /**< \brief (HSMC_REM14_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_REM15_ : (SMC Offset: N/A) PMECC Remainder 15 Register -------- */
#define HSMC_REM15__REM2NP1_Pos 0
#define HSMC_REM15__REM2NP1_Msk (0x3fffu << HSMC_REM15__REM2NP1_Pos) /**< \brief (HSMC_REM15_) BCH Remainder 2 * N + 1 */
#define HSMC_REM15__REM2NP3_Pos 16
#define HSMC_REM15__REM2NP3_Msk (0x3fffu << HSMC_REM15__REM2NP3_Pos) /**< \brief (HSMC_REM15_) BCH Remainder 2 * N + 3 */
/* -------- HSMC_ELCFG : (SMC Offset: 0x500) PMECC Error Location Configuration Register -------- */
#define HSMC_ELCFG_SECTORSZ (0x1u << 0) /**< \brief (HSMC_ELCFG) Sector Size */
#define HSMC_ELCFG_ERRNUM_Pos 16
#define HSMC_ELCFG_ERRNUM_Msk (0x1fu << HSMC_ELCFG_ERRNUM_Pos) /**< \brief (HSMC_ELCFG) Number of Errors */
#define HSMC_ELCFG_ERRNUM(value) ((HSMC_ELCFG_ERRNUM_Msk & ((value) << HSMC_ELCFG_ERRNUM_Pos)))
/* -------- HSMC_ELPRIM : (SMC Offset: 0x504) PMECC Error Location Primitive Register -------- */
#define HSMC_ELPRIM_PRIMITIV_Pos 0
#define HSMC_ELPRIM_PRIMITIV_Msk (0xffffu << HSMC_ELPRIM_PRIMITIV_Pos) /**< \brief (HSMC_ELPRIM) Primitive Polynomial */
/* -------- HSMC_ELEN : (SMC Offset: 0x508) PMECC Error Location Enable Register -------- */
#define HSMC_ELEN_ENINIT_Pos 0
#define HSMC_ELEN_ENINIT_Msk (0x3fffu << HSMC_ELEN_ENINIT_Pos) /**< \brief (HSMC_ELEN) Error Location Enable */
#define HSMC_ELEN_ENINIT(value) ((HSMC_ELEN_ENINIT_Msk & ((value) << HSMC_ELEN_ENINIT_Pos)))
/* -------- HSMC_ELDIS : (SMC Offset: 0x50C) PMECC Error Location Disable Register -------- */
#define HSMC_ELDIS_DIS (0x1u << 0) /**< \brief (HSMC_ELDIS) Disable Error Location Engine */
/* -------- HSMC_ELSR : (SMC Offset: 0x510) PMECC Error Location Status Register -------- */
#define HSMC_ELSR_BUSY (0x1u << 0) /**< \brief (HSMC_ELSR) Error Location Engine Busy */
/* -------- HSMC_ELIER : (SMC Offset: 0x514) PMECC Error Location Interrupt Enable register -------- */
#define HSMC_ELIER_DONE (0x1u << 0) /**< \brief (HSMC_ELIER) Computation Terminated Interrupt Enable */
/* -------- HSMC_ELIDR : (SMC Offset: 0x518) PMECC Error Location Interrupt Disable Register -------- */
#define HSMC_ELIDR_DONE (0x1u << 0) /**< \brief (HSMC_ELIDR) Computation Terminated Interrupt Disable */
/* -------- HSMC_ELIMR : (SMC Offset: 0x51C) PMECC Error Location Interrupt Mask Register -------- */
#define HSMC_ELIMR_DONE (0x1u << 0) /**< \brief (HSMC_ELIMR) Computation Terminated Interrupt Mask */
/* -------- HSMC_ELISR : (SMC Offset: 0x520) PMECC Error Location Interrupt Status Register -------- */
#define HSMC_ELISR_DONE (0x1u << 0) /**< \brief (HSMC_ELISR) Computation Terminated Interrupt Status */
#define HSMC_ELISR_ERR_CNT_Pos 8
#define HSMC_ELISR_ERR_CNT_Msk (0x3fu << HSMC_ELISR_ERR_CNT_Pos) /**< \brief (HSMC_ELISR) Error Counter value */
/* -------- HSMC_SIGMA[33] : (SMC Offset: 0x528) PMECC Error Location SIGMA 0 Registers -------- */
#define HSMC_SIGMA_SIGMA_Pos 0
#define HSMC_SIGMA_SIGMA_Msk (0x3fffu << HSMC_SIGMA_SIGMA_Pos) /**< \brief (HSMC_SIGMA[33]) Coefficient of degree x in the SIGMA polynomial */
/* -------- HSMC_ERRLOC[32] : (SMC Offset: 0x5AC) PMECC Error Location 0 Register -------- */
#define HSMC_ERRLOC_ERRLOCN_Pos 0
#define HSMC_ERRLOC_ERRLOCN_Msk (0x3fffu << HSMC_ERRLOC_ERRLOCN_Pos) /**< \brief (HSMC_ERRLOC[32]) Error Position within the Set {sector area, spare area} */
/* -------- HSMC_SETUP : (SMC Offset: N/A) HSMC Setup Register -------- */
#define HSMC_SETUP_NWE_SETUP_Pos 0
#define HSMC_SETUP_NWE_SETUP_Msk (0x3fu << HSMC_SETUP_NWE_SETUP_Pos) /**< \brief (HSMC_SETUP) NWE Setup Length */
#define HSMC_SETUP_NWE_SETUP(value) ((HSMC_SETUP_NWE_SETUP_Msk & ((value) << HSMC_SETUP_NWE_SETUP_Pos)))
#define HSMC_SETUP_NCS_WR_SETUP_Pos 8
#define HSMC_SETUP_NCS_WR_SETUP_Msk (0x3fu << HSMC_SETUP_NCS_WR_SETUP_Pos) /**< \brief (HSMC_SETUP) NCS Setup Length in Write Access */
#define HSMC_SETUP_NCS_WR_SETUP(value) ((HSMC_SETUP_NCS_WR_SETUP_Msk & ((value) << HSMC_SETUP_NCS_WR_SETUP_Pos)))
#define HSMC_SETUP_NRD_SETUP_Pos 16
#define HSMC_SETUP_NRD_SETUP_Msk (0x3fu << HSMC_SETUP_NRD_SETUP_Pos) /**< \brief (HSMC_SETUP) NRD Setup Length */
#define HSMC_SETUP_NRD_SETUP(value) ((HSMC_SETUP_NRD_SETUP_Msk & ((value) << HSMC_SETUP_NRD_SETUP_Pos)))
#define HSMC_SETUP_NCS_RD_SETUP_Pos 24
#define HSMC_SETUP_NCS_RD_SETUP_Msk (0x3fu << HSMC_SETUP_NCS_RD_SETUP_Pos) /**< \brief (HSMC_SETUP) NCS Setup Length in Read Access */
#define HSMC_SETUP_NCS_RD_SETUP(value) ((HSMC_SETUP_NCS_RD_SETUP_Msk & ((value) << HSMC_SETUP_NCS_RD_SETUP_Pos)))
/* -------- HSMC_PULSE : (SMC Offset: N/A) HSMC Pulse Register -------- */
#define HSMC_PULSE_NWE_PULSE_Pos 0
#define HSMC_PULSE_NWE_PULSE_Msk (0x7fu << HSMC_PULSE_NWE_PULSE_Pos) /**< \brief (HSMC_PULSE) NWE Pulse Length */
#define HSMC_PULSE_NWE_PULSE(value) ((HSMC_PULSE_NWE_PULSE_Msk & ((value) << HSMC_PULSE_NWE_PULSE_Pos)))
#define HSMC_PULSE_NCS_WR_PULSE_Pos 8
#define HSMC_PULSE_NCS_WR_PULSE_Msk (0x7fu << HSMC_PULSE_NCS_WR_PULSE_Pos) /**< \brief (HSMC_PULSE) NCS Pulse Length in WRITE Access */
#define HSMC_PULSE_NCS_WR_PULSE(value) ((HSMC_PULSE_NCS_WR_PULSE_Msk & ((value) << HSMC_PULSE_NCS_WR_PULSE_Pos)))
#define HSMC_PULSE_NRD_PULSE_Pos 16
#define HSMC_PULSE_NRD_PULSE_Msk (0x7fu << HSMC_PULSE_NRD_PULSE_Pos) /**< \brief (HSMC_PULSE) NRD Pulse Length */
#define HSMC_PULSE_NRD_PULSE(value) ((HSMC_PULSE_NRD_PULSE_Msk & ((value) << HSMC_PULSE_NRD_PULSE_Pos)))
#define HSMC_PULSE_NCS_RD_PULSE_Pos 24
#define HSMC_PULSE_NCS_RD_PULSE_Msk (0x7fu << HSMC_PULSE_NCS_RD_PULSE_Pos) /**< \brief (HSMC_PULSE) NCS Pulse Length in READ Access */
#define HSMC_PULSE_NCS_RD_PULSE(value) ((HSMC_PULSE_NCS_RD_PULSE_Msk & ((value) << HSMC_PULSE_NCS_RD_PULSE_Pos)))
/* -------- HSMC_CYCLE : (SMC Offset: N/A) HSMC Cycle Register -------- */
#define HSMC_CYCLE_NWE_CYCLE_Pos 0
#define HSMC_CYCLE_NWE_CYCLE_Msk (0x1ffu << HSMC_CYCLE_NWE_CYCLE_Pos) /**< \brief (HSMC_CYCLE) Total Write Cycle Length */
#define HSMC_CYCLE_NWE_CYCLE(value) ((HSMC_CYCLE_NWE_CYCLE_Msk & ((value) << HSMC_CYCLE_NWE_CYCLE_Pos)))
#define HSMC_CYCLE_NRD_CYCLE_Pos 16
#define HSMC_CYCLE_NRD_CYCLE_Msk (0x1ffu << HSMC_CYCLE_NRD_CYCLE_Pos) /**< \brief (HSMC_CYCLE) Total Read Cycle Length */
#define HSMC_CYCLE_NRD_CYCLE(value) ((HSMC_CYCLE_NRD_CYCLE_Msk & ((value) << HSMC_CYCLE_NRD_CYCLE_Pos)))
/* -------- HSMC_TIMINGS : (SMC Offset: N/A) HSMC Timings Register -------- */
#define HSMC_TIMINGS_TCLR_Pos 0
#define HSMC_TIMINGS_TCLR_Msk (0xfu << HSMC_TIMINGS_TCLR_Pos) /**< \brief (HSMC_TIMINGS) CLE to REN Low Delay */
#define HSMC_TIMINGS_TCLR(value) ((HSMC_TIMINGS_TCLR_Msk & ((value) << HSMC_TIMINGS_TCLR_Pos)))
#define HSMC_TIMINGS_TADL_Pos 4
#define HSMC_TIMINGS_TADL_Msk (0xfu << HSMC_TIMINGS_TADL_Pos) /**< \brief (HSMC_TIMINGS) ALE to Data Start */
#define HSMC_TIMINGS_TADL(value) ((HSMC_TIMINGS_TADL_Msk & ((value) << HSMC_TIMINGS_TADL_Pos)))
#define HSMC_TIMINGS_TAR_Pos 8
#define HSMC_TIMINGS_TAR_Msk (0xfu << HSMC_TIMINGS_TAR_Pos) /**< \brief (HSMC_TIMINGS) ALE to REN Low Delay */
#define HSMC_TIMINGS_TAR(value) ((HSMC_TIMINGS_TAR_Msk & ((value) << HSMC_TIMINGS_TAR_Pos)))
#define HSMC_TIMINGS_OCMS (0x1u << 12) /**< \brief (HSMC_TIMINGS) Off Chip Memory Scrambling Enable */
#define HSMC_TIMINGS_TRR_Pos 16
#define HSMC_TIMINGS_TRR_Msk (0xfu << HSMC_TIMINGS_TRR_Pos) /**< \brief (HSMC_TIMINGS) Ready to REN Low Delay */
#define HSMC_TIMINGS_TRR(value) ((HSMC_TIMINGS_TRR_Msk & ((value) << HSMC_TIMINGS_TRR_Pos)))
#define HSMC_TIMINGS_TWB_Pos 24
#define HSMC_TIMINGS_TWB_Msk (0xfu << HSMC_TIMINGS_TWB_Pos) /**< \brief (HSMC_TIMINGS) WEN High to REN to Busy */
#define HSMC_TIMINGS_TWB(value) ((HSMC_TIMINGS_TWB_Msk & ((value) << HSMC_TIMINGS_TWB_Pos)))
#define HSMC_TIMINGS_RBNSEL_Pos 28
#define HSMC_TIMINGS_RBNSEL_Msk (0x7u << HSMC_TIMINGS_RBNSEL_Pos) /**< \brief (HSMC_TIMINGS) Ready/Busy Line Selection */
#define HSMC_TIMINGS_RBNSEL(value) ((HSMC_TIMINGS_RBNSEL_Msk & ((value) << HSMC_TIMINGS_RBNSEL_Pos)))
#define HSMC_TIMINGS_NFSEL (0x1u << 31) /**< \brief (HSMC_TIMINGS) NAND Flash Selection */
/* -------- HSMC_MODE : (SMC Offset: N/A) HSMC Mode Register -------- */
#define HSMC_MODE_READ_MODE (0x1u << 0) /**< \brief (HSMC_MODE) Selection of the Control Signal for Read Operation */
#define   HSMC_MODE_READ_MODE_NCS_CTRL (0x0u << 0) /**< \brief (HSMC_MODE) The Read operation is controlled by the NCS signal. */
#define   HSMC_MODE_READ_MODE_NRD_CTRL (0x1u << 0) /**< \brief (HSMC_MODE) The Read operation is controlled by the NRD signal. */
#define HSMC_MODE_WRITE_MODE (0x1u << 1) /**< \brief (HSMC_MODE) Selection of the Control Signal for Write Operation */
#define   HSMC_MODE_WRITE_MODE_NCS_CTRL (0x0u << 1) /**< \brief (HSMC_MODE) The Write operation is controller by the NCS signal. */
#define   HSMC_MODE_WRITE_MODE_NWE_CTRL (0x1u << 1) /**< \brief (HSMC_MODE) The Write operation is controlled by the NWE signal */
#define HSMC_MODE_EXNW_MODE_Pos 4
#define HSMC_MODE_EXNW_MODE_Msk (0x3u << HSMC_MODE_EXNW_MODE_Pos) /**< \brief (HSMC_MODE) NWAIT Mode */
#define HSMC_MODE_EXNW_MODE(value) ((HSMC_MODE_EXNW_MODE_Msk & ((value) << HSMC_MODE_EXNW_MODE_Pos)))
#define   HSMC_MODE_EXNW_MODE_DISABLED (0x0u << 4) /**< \brief (HSMC_MODE) Disabled-The NWAIT input signal is ignored on the corresponding Chip Select. */
#define   HSMC_MODE_EXNW_MODE_FROZEN (0x2u << 4) /**< \brief (HSMC_MODE) Frozen Mode-If asserted, the NWAIT signal freezes the current read or write cycle. After deassertion, the read/write cycle is resumed from the point where it was stopped. */
#define   HSMC_MODE_EXNW_MODE_READY (0x3u << 4) /**< \brief (HSMC_MODE) Ready Mode-The NWAIT signal indicates the availability of the external device at the end of the pulse of the controlling read or write signal, to complete the access. If high, the access normally completes. If low, the access is extended until NWAIT returns high. */
#define HSMC_MODE_BAT (0x1u << 8) /**< \brief (HSMC_MODE) Byte Access Type */
#define   HSMC_MODE_BAT_BYTE_SELECT (0x0u << 8) /**< \brief (HSMC_MODE) Byte select access type:- Write operation is controlled using NCS, NWE, NBS0, NBS1.- Read operation is controlled using NCS, NRD, NBS0, NBS1. */
#define   HSMC_MODE_BAT_BYTE_WRITE (0x1u << 8) /**< \brief (HSMC_MODE) Byte write access type:- Write operation is controlled using NCS, NWR0, NWR1.- Read operation is controlled using NCS and NRD. */
#define HSMC_MODE_DBW (0x1u << 12) /**< \brief (HSMC_MODE) Data Bus Width */
#define   HSMC_MODE_DBW_BIT_8 (0x0u << 12) /**< \brief (HSMC_MODE) 8-bit bus */
#define   HSMC_MODE_DBW_BIT_16 (0x1u << 12) /**< \brief (HSMC_MODE) 16-bit bus */
#define HSMC_MODE_TDF_CYCLES_Pos 16
#define HSMC_MODE_TDF_CYCLES_Msk (0xfu << HSMC_MODE_TDF_CYCLES_Pos) /**< \brief (HSMC_MODE) Data Float Time */
#define HSMC_MODE_TDF_CYCLES(value) ((HSMC_MODE_TDF_CYCLES_Msk & ((value) << HSMC_MODE_TDF_CYCLES_Pos)))
#define HSMC_MODE_TDF_MODE (0x1u << 20) /**< \brief (HSMC_MODE) TDF Optimization */
/* -------- HSMC_OCMS : (SMC Offset: 0x7A0) HSMC Off Chip Memory Scrambling Register -------- */
#define HSMC_OCMS_SMSE (0x1u << 0) /**< \brief (HSMC_OCMS) Static Memory Controller Scrambling Enable */
#define HSMC_OCMS_SRSE (0x1u << 1) /**< \brief (HSMC_OCMS) NFC Internal SRAM Scrambling Enable */
/* -------- HSMC_KEY1 : (SMC Offset: 0x7A4) HSMC Off Chip Memory Scrambling KEY1 Register -------- */
#define HSMC_KEY1_KEY1_Pos 0
#define HSMC_KEY1_KEY1_Msk (0xffffffffu << HSMC_KEY1_KEY1_Pos) /**< \brief (HSMC_KEY1) Off Chip Memory Scrambling (OCMS) Key Part 1 */
#define HSMC_KEY1_KEY1(value) ((HSMC_KEY1_KEY1_Msk & ((value) << HSMC_KEY1_KEY1_Pos)))
/* -------- HSMC_KEY2 : (SMC Offset: 0x7A8) HSMC Off Chip Memory Scrambling KEY2 Register -------- */
#define HSMC_KEY2_KEY2_Pos 0
#define HSMC_KEY2_KEY2_Msk (0xffffffffu << HSMC_KEY2_KEY2_Pos) /**< \brief (HSMC_KEY2) Off Chip Memory Scrambling (OCMS) Key Part 2 */
#define HSMC_KEY2_KEY2(value) ((HSMC_KEY2_KEY2_Msk & ((value) << HSMC_KEY2_KEY2_Pos)))
/* -------- HSMC_WPMR : (SMC Offset: 0x7E4) HSMC Write Protection Mode Register -------- */
#define HSMC_WPMR_WPEN (0x1u << 0) /**< \brief (HSMC_WPMR) Write Protection Enable */
#define HSMC_WPMR_WPKEY_Pos 8
#define HSMC_WPMR_WPKEY_Msk (0xffffffu << HSMC_WPMR_WPKEY_Pos) /**< \brief (HSMC_WPMR) Write Protection Key */
#define HSMC_WPMR_WPKEY(value) ((HSMC_WPMR_WPKEY_Msk & ((value) << HSMC_WPMR_WPKEY_Pos)))
#define   HSMC_WPMR_WPKEY_PASSWD (0x534D43u << 8) /**< \brief (HSMC_WPMR) Writing any other value in this field aborts the write operation of bit WPEN.Always reads as 0. */
/* -------- HSMC_WPSR : (SMC Offset: 0x7E8) HSMC Write Protection Status Register -------- */
#define HSMC_WPSR_WPVS (0x1u << 0) /**< \brief (HSMC_WPSR) Write Protection Violation Status */
#define HSMC_WPSR_WPVSRC_Pos 8
#define HSMC_WPSR_WPVSRC_Msk (0xffffu << HSMC_WPSR_WPVSRC_Pos) /**< \brief (HSMC_WPSR) Write Protection Violation Source */
/* -------- HSMC_VERSION : (SMC Offset: 0x7FC) HSMC Version Register -------- */
#define HSMC_VERSION_VERSION_Pos 0
#define HSMC_VERSION_VERSION_Msk (0xfffu << HSMC_VERSION_VERSION_Pos) /**< \brief (HSMC_VERSION) Hardware Version Number */
#define HSMC_VERSION_MFN_Pos 16
#define HSMC_VERSION_MFN_Msk (0x7u << HSMC_VERSION_MFN_Pos) /**< \brief (HSMC_VERSION) Metal Fix Number */

/*@}*/


#endif /* _SAMA5D2_SMC_COMPONENT_ */

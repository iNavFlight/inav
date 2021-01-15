/* ---------------------------------------------------------------------------- */
/*                  Atmel Microcontroller Software Support                      */
/*                       SAM Software Package License                           */
/* ---------------------------------------------------------------------------- */
/* Copyright (c) 2016, Atmel Corporation                                        */
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

#ifndef _SAMA5D2_BSC_COMPONENT_
#define _SAMA5D2_BSC_COMPONENT_

/* ============================================================================= */
/**  SOFTWARE API DEFINITION FOR Boot Sequence Control */
/* ============================================================================= */
/** \addtogroup SAMA5D2_BSC Boot Sequence Control */
/*@{*/

#if !(defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__))
/** \brief Bsc hardware registers */
typedef struct {
  __IO uint32_t BSC_CR; /**< \brief (Bsc Offset: 0x0) Boot Sequence Control Configuration Register */
} Bsc;
#endif /* !(defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__)) */
/* -------- BSC_CR : (BSC Offset: 0x0) Boot Sequence Control Configuration Register -------- */
#define BSC_CR_BUREG_INDEX_Pos 0
#define BSC_CR_BUREG_INDEX_Msk (0x3u << BSC_CR_BUREG_INDEX_Pos)
#define BSC_CR_BUREG_INDEX(value) (BSC_CR_BUREG_INDEX_Msk & ((value) << BSC_CR_BUREG_INDEX_Pos)
#define   BSC_CR_BUREG_0 (0x0u << 0)
#define   BSC_CR_BUREG_1 (0x1u << 0)
#define   BSC_CR_BUREG_2 (0x2u << 0)
#define   BSC_CR_BUREG_3 (0x3u << 0)
#define BSC_CR_BUREG_VALID (1 << 2)
#define BSC_CR_WPKEY_Pos 16
#define BSC_CR_WPKEY_Msk (0xffffu << BSC_CR_WPKEY_Pos)
#define BSC_CR_WPKEY (0x6683 << 16)

/* -------- BCW : Boot Control Word -------- */
#define BCW_QSPI_0_Pos 0
#define BCW_QSPI_0_Msk (0x3u << BCW_QSPI_0_Pos)
#define BCW_QSPI_0(value) (BCW_QSPI_0_Msk & ((value) << BCW_QSPI_0_Pos)
#define   BCW_QSPI_0_IOSET_1 (0x0u << 0)
#define   BCW_QSPI_0_IOSET_2 (0x1u << 0)
#define   BCW_QSPI_0_IOSET_3 (0x2u << 0)
#define   BCW_QSPI_0_DISABLED (0x3u << 0)
#define BCW_QSPI_1_Pos 2
#define BCW_QSPI_1_Msk (0x3u << BCW_QSPI_1_Pos)
#define BCW_QSPI_1(value) (BCW_QSPI_1_Msk & ((value) << BCW_QSPI_1_Pos)
#define   BCW_QSPI_1_IOSET_1 (0x0u << 2)
#define   BCW_QSPI_1_IOSET_2 (0x1u << 2)
#define   BCW_QSPI_1_IOSET_3 (0x2u << 2)
#define   BCW_QSPI_1_DISABLED (0x3u << 2)
#define BCW_SPI_0_Pos 4
#define BCW_SPI_0_Msk (0x3u << BCW_SPI_0_Pos)
#define BCW_SPI_0(value) (BCW_SPI_0_Msk & ((value) << BCW_SPI_0_Pos)
#define   BCW_SPI_0_IOSET_1 (0x0u << 4)
#define   BCW_SPI_0_IOSET_2 (0x1u << 4)
#define   BCW_SPI_0_DISABLED (0x3u << 4)
#define BCW_SPI_1_Pos 6
#define BCW_SPI_1_Msk (0x3u << BCW_SPI_1_Pos)
#define BCW_SPI_1(value) (BCW_SPI_1_Msk & ((value) << BCW_SPI_1_Pos)
#define   BCW_SPI_1_IOSET_1 (0x0u << 6)
#define   BCW_SPI_1_IOSET_2 (0x1u << 6)
#define   BCW_SPI_1_IOSET_3 (0x2u << 6)
#define   BCW_SPI_1_DISABLED (0x3u << 6)
#define BCW_NFC_Pos 8
#define BCW_NFC_Msk (0x3u << BCW_NFC_Pos)
#define BCW_NFC(value) (BCW_NFC_Msk & ((value) << BCW_NFC_Pos)
#define   BCW_NFC_IOSET_1 (0x0u << 8)
#define   BCW_NFC_IOSET_2 (0x1u << 8)
#define   BCW_NFC_DISABLED (0x3u << 8)
#define BCW_SDMMC_0_DISABLED (1 << 10)
#define BCW_SDMMC_1_DISABLED (1 << 11)
#define BCW_UART_CONSOLE_Pos 12
#define BCW_UART_CONSOLE_Msk (0xfu << BCW_UART_CONSOLE_Pos)
#define BCW_UART_CONSOLE(value) (BCW_UART_CONSOLE_Msk & ((value) << BCW_UART_CONSOLE_Pos)
#define   BCW_UART_CONSOLE_UART1_IOSET_1 (0x0u << 12)
#define   BCW_UART_CONSOLE_UART0_IOSET_1 (0x1u << 12)
#define   BCW_UART_CONSOLE_UART1_IOSET_2 (0x2u << 12)
#define   BCW_UART_CONSOLE_UART2_IOSET_1 (0x3u << 12)
#define   BCW_UART_CONSOLE_UART2_IOSET_2 (0x4u << 12)
#define   BCW_UART_CONSOLE_UART2_IOSET_3 (0x5u << 12)
#define   BCW_UART_CONSOLE_UART3_IOSET_1 (0x6u << 12)
#define   BCW_UART_CONSOLE_UART3_IOSET_2 (0x7u << 12)
#define   BCW_UART_CONSOLE_UART3_IOSET_3 (0x8u << 12)
#define   BCW_UART_CONSOLE_UART4_IOSET_1 (0x9u << 12)
#define   BCW_UART_CONSOLE_DISABLED (0xfu << 12)
#define BCW_JTAG_IO_SET_Pos 16
#define BCW_JTAG_IO_SET_Msk (0x3u << BCW_JTAG_IO_SET_Pos)
#define BCW_JTAG_IO_SET(value) (BCW_JTAG_IO_SET_Msk & ((value) << BCW_JTAG_IO_SET_Pos)
#define   BCW_JTAG_IOSET_1 (0x0u << 16)
#define   BCW_JTAG_IOSET_2 (0x1u << 16)
#define   BCW_JTAG_IOSET_3 (0x2u << 16)
#define   BCW_JTAG_IOSET_4 (0x3u << 16)
#define BCW_EXT_MEM_BOOT_ENABLE (1 << 18)
#define BCW_QSPI_XIP_MODE (1 << 21)
#define BCW_DISABLE_BSCR (1 << 22)
#define BCW_DISABLE_MONITOR (1 << 24)
#define BCW_SECURE_MODE (1 << 29)

/*@}*/


#endif /* _SAMA5D2_BSC_COMPONENT_ */

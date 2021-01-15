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

#ifndef _SAMA5D2_FLEXCOM_COMPONENT_
#define _SAMA5D2_FLEXCOM_COMPONENT_

/* ============================================================================= */
/**  SOFTWARE API DEFINITION FOR Flexible Serial Communication */
/* ============================================================================= */
/** \addtogroup SAMA5D2_FLEXCOM Flexible Serial Communication */
/*@{*/

#if !(defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__))

/** \brief Flexcom hardware registers */
typedef struct {
	__IO uint32_t FLEX_MR;          /**< \brief (Flexcom Offset: 0x000) FLEXCOM Mode Register */
	__I  uint32_t Reserved1[3];
	__I  uint32_t FLEX_RHR;         /**< \brief (Flexcom Offset: 0x010) FLEXCOM Receive Holding Register */
	__I  uint32_t Reserved2[3];
	__IO uint32_t FLEX_THR;         /**< \brief (Flexcom Offset: 0x020) FLEXCOM Transmit Holding Register */
} Flexcom;

#endif /* !(defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__)) */

/* -------- FLEX_MR : (FLEXCOM Offset: 0x000) FLEXCOM Mode Register -------- */
#define FLEX_MR_OPMODE_Pos 0
#define FLEX_MR_OPMODE_Msk (0x3u << FLEX_MR_OPMODE_Pos) /**< \brief (FLEX_MR) FLEXCOM Operating Mode */
#define FLEX_MR_OPMODE(value) ((FLEX_MR_OPMODE_Msk & ((value) << FLEX_MR_OPMODE_Pos)))
#define   FLEX_MR_OPMODE_NO_COM (0x0u << 0) /**< \brief (FLEX_MR) No communication */
#define   FLEX_MR_OPMODE_USART (0x1u << 0) /**< \brief (FLEX_MR) All related UART related protocols are selected (RS232, RS485, IrDA, ISO7816, LIN,)All SPI/TWI related registers are not accessible and have no impact on IOs. */
#define   FLEX_MR_OPMODE_SPI (0x2u << 0) /**< \brief (FLEX_MR) SPI operating mode is selected.All USART/TWI related registers are not accessible and have no impact on IOs. */
#define   FLEX_MR_OPMODE_TWI (0x3u << 0) /**< \brief (FLEX_MR) All related TWI protocols are selected (TWI, SMBus). All USART/SPI related registers are not accessible and have no impact on IOs. */
/* -------- FLEX_RHR : (FLEXCOM Offset: 0x010) FLEXCOM Receive Holding Register -------- */
#define FLEX_RHR_RXDATA_Pos 0
#define FLEX_RHR_RXDATA_Msk (0xffffu << FLEX_RHR_RXDATA_Pos) /**< \brief (FLEX_RHR) Receive Data */
/* -------- FLEX_THR : (FLEXCOM Offset: 0x020) FLEXCOM Transmit Holding Register -------- */
#define FLEX_THR_TXDATA_Pos 0
#define FLEX_THR_TXDATA_Msk (0xffffu << FLEX_THR_TXDATA_Pos) /**< \brief (FLEX_THR) Transmit Data */
#define FLEX_THR_TXDATA(value) ((FLEX_THR_TXDATA_Msk & ((value) << FLEX_THR_TXDATA_Pos)))

/*@}*/


#endif /* _SAMA5D2_FLEXCOM_COMPONENT_ */

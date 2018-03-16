/*
 * Copyright (C) 2013-2016 Fabio Utzig, http://fabioutzig.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _KL27ZXXX_H_
#define _KL27ZXXX_H_

/**
 * @brief KL2x Interrupt Number Definition, according to the selected device
 *        in @ref Library_configuration_section
 */
#define __MPU_PRESENT             0
#define __VTOR_PRESENT            1
#define __NVIC_PRIO_BITS          2
#define __Vendor_SysTickConfig    0

/*
 * ==============================================================
 * ---------- Interrupt Number Definition -----------------------
 * ==============================================================
 */
typedef enum IRQn
{
/******  Cortex-M0 Processor Exceptions Numbers ****************/
  Reset_IRQn                    = -15,
  NonMaskableInt_IRQn           = -14,
  HardFault_IRQn                = -13,
  SVCall_IRQn                   = -5,
  PendSV_IRQn                   = -2,
  SysTick_IRQn                  = -1,

/******  KL2x Specific Interrupt Numbers ***********************/
  DMA0_IRQn                     = 0,
  DMA1_IRQn                     = 1,
  DMA2_IRQn                     = 2,
  DMA3_IRQn                     = 3,
  Reserved0_IRQn                = 4,
  FTFA_IRQn                     = 5,
  PMC_IRQn                      = 6,
  LLWU_IRQn                     = 7,
  I2C0_IRQn                     = 8,
  I2C1_IRQn                     = 9,
  SPI0_IRQn                     = 10,
  SPI1_IRQn                     = 11,
  LPUART0_IRQn                  = 12,
  LPUART1_IRQn                  = 13,
  UART2_IRQn                    = 14,
  ADC0_IRQn                     = 15,
  CMP0_IRQn                     = 16,
  TPM0_IRQn                     = 17,
  TPM1_IRQn                     = 18,
  TPM2_IRQn                     = 19,
  RTC0_IRQn                     = 20,
  RTC1_IRQn                     = 21,
  PIT_IRQn                      = 22,
  I2S0_IRQn                     = 23,
  USB_IRQn                      = 24,
  DAC0_IRQn                     = 25,
  Reserved2_IRQn                = 26,
  Reserved3_IRQn                = 27,
  LPTMR0_IRQn                   = 28,
  Reserved4_IRQn                = 29,
  PINA_IRQn                     = 30,
  PINCD_IRQn                    = 31,
} IRQn_Type;

#include "core_cm0plus.h"        /* Cortex-M0+ processor and core peripherals */

/*
 * ==========================================================================
 * ----------- Processor and Core Peripheral Section ------------------------
 * ==========================================================================
 */

typedef struct
{
  __IO uint8_t  C1;
  __IO uint8_t  C2;
       uint8_t  RESERVED0[4];
  __I  uint8_t  S;
       uint8_t  RESERVED1[1];
  __IO uint8_t  SC;
       uint8_t  RESERVED2[15];
  __IO uint8_t  MC;
} MCGLite_TypeDef;

typedef struct
{
  __IO uint32_t SC;
  __IO uint32_t CNT;
  __IO uint32_t MOD;
  struct {                      // Channels
    __IO uint32_t SC;
    __IO uint32_t V;
  } C[6];
       uint32_t RESERVED0[5];
  __IO uint32_t STATUS;
       uint32_t RESERVED1[7];
  __IO uint32_t POL;
       uint32_t RESERVED2[4];
  __IO uint32_t CONF;
} TPM_TypeDef;

typedef struct
{
  __IO uint8_t  S;
  __IO uint8_t  BR;
  __IO uint8_t  C2;
  __IO uint8_t  C1;
  __IO uint8_t  ML;
  __IO uint8_t  MH;
  __IO uint8_t  DL;
  __IO uint8_t  DH;
       uint8_t  RESERVED0[2];
  __IO uint8_t  CI;
  __IO uint8_t  C3;
} SPI_TypeDef;

typedef struct
{
  __IO uint8_t  A1;
  __IO uint8_t  F;
  __IO uint8_t  C1;
  __IO uint8_t  S;
  __IO uint8_t  D;
  __IO uint8_t  C2;
  __IO uint8_t  FLT;
  __IO uint8_t  RA;
  __IO uint8_t  SMB;
  __IO uint8_t  A2;
  __IO uint8_t  SLTH;
  __IO uint8_t  SLTL;
  __IO uint8_t  S2;
} I2C_TypeDef;

typedef struct
{
  __IO uint32_t BAUD;
  __IO uint32_t STAT;
  __IO uint32_t CTRL;
  __IO uint32_t DATA;
  __IO uint32_t MATCH;
} LPUART_TypeDef;

typedef struct
{
  __IO uint8_t  BDH;
  __IO uint8_t  BDL;
  __IO uint8_t  C1;
  __IO uint8_t  C2;
  __I  uint8_t  S1;
  __IO uint8_t  S2;
  __IO uint8_t  C3;
  __IO uint8_t  D;
  __IO uint8_t  MA1;
  __IO uint8_t  MA2;
  __IO uint8_t  C4;
  __IO uint8_t  C5;
} UART_TypeDef;

typedef struct
{
  __I  uint32_t VERID;
  __I  uint32_t PARAM;
  __IO uint32_t CTRL;
       uint32_t RESERVED0[1];
  __IO uint32_t SHIFTSTAT;
  __IO uint32_t SHIFTERR;
  __IO uint32_t TIMSTAT;
       uint32_t RESERVED1[1];
  __IO uint32_t SHIFTSIEN;
  __IO uint32_t SHIFTEIEN;
  __IO uint32_t TIMIEN;
       uint32_t RESERVED2[1];
  __IO uint32_t SHIFTSDEN;
       uint32_t RESERVED3[19];
  __IO uint32_t SHIFTCTL[4];
       uint32_t RESERVED4[28];
  __IO uint32_t SHIFTCFG[4];
       uint32_t RESERVED5[60];
  __IO uint32_t SHIFTBUF[4];
       uint32_t RESERVED6[28];
  __IO uint32_t SHIFTBUFBIS[4];
       uint32_t RESERVED7[28];
  __IO uint32_t SHIFTBUFBYS[4];
       uint32_t RESERVED8[28];
  __IO uint32_t SHIFTBUFBBS[4];
       uint32_t RESERVED9[28];
  __IO uint32_t TIMCTL[4];
       uint32_t RESERVED10[28];
  __IO uint32_t TIMCFG[4];
       uint32_t RESERVED11[28];
  __IO uint32_t TIMCMP[4];
} FlexIO_TypeDef;

typedef struct
{
  __IO uint8_t TRM;
  __IO uint8_t SC;
} VREF_TypeDef;

typedef struct {
  __I  uint8_t  PERID;               // 0x00
       uint8_t  RESERVED0[3];
  __I  uint8_t  IDCOMP;              // 0x04
       uint8_t  RESERVED1[3];
  __I  uint8_t  REV;                 // 0x08
       uint8_t  RESERVED2[3];
  __I  uint8_t  ADDINFO;             // 0x0C
       uint8_t  RESERVED3[15];
  __IO uint8_t  OTGCTL;              // 0x1C
       uint8_t  RESERVED7[99];
  __IO uint8_t  ISTAT;               // 0x80
       uint8_t  RESERVED8[3];
  __IO uint8_t  INTEN;               // 0x84
       uint8_t  RESERVED9[3];
  __IO uint8_t  ERRSTAT;             // 0x88
       uint8_t  RESERVED10[3];
  __IO uint8_t  ERREN;               // 0x8C
       uint8_t  RESERVED11[3];
  __I  uint8_t  STAT;                // 0x90
       uint8_t  RESERVED12[3];
  __IO uint8_t  CTL;                 // 0x94
       uint8_t  RESERVED13[3];
  __IO uint8_t  ADDR;                // 0x98
       uint8_t  RESERVED14[3];
  __IO uint8_t  BDTPAGE1;            // 0x9C
       uint8_t  RESERVED15[3];
  __IO uint8_t  FRMNUML;             // 0xA0
       uint8_t  RESERVED16[3];
  __IO uint8_t  FRMNUMH;             // 0xA4
       uint8_t  RESERVED17[11];
  __IO uint8_t  BDTPAGE2;            // 0xB0
       uint8_t  RESERVED20[3];
  __IO uint8_t  BDTPAGE3;            // 0xB4
       uint8_t  RESERVED21[11];
  struct {
    __IO uint8_t  V;                 // 0xC0
         uint8_t  RESERVED[3];
  } ENDPT[16];
  __IO uint8_t  USBCTRL;             // 0x100
       uint8_t  RESERVED22[3];
  __I  uint8_t  OBSERVE;             // 0x104
       uint8_t  RESERVED23[3];
  __IO uint8_t  CONTROL;             // 0x108
       uint8_t  RESERVED24[3];
  __IO uint8_t  USBTRC0;             // 0x10C
       uint8_t  RESERVED25[7];
  __IO uint8_t  USBFRMADJUST;        // 0x114
       uint8_t  RESERVED26[43];
  __IO uint8_t  CLK_RECOVER_CTRL;    // 0x140
       uint8_t  RESERVED27[3];
  __IO uint8_t  CLK_RECOVER_IRC_EN;  // 0x144
       uint8_t  RESERVED28[15];
  __IO uint8_t  CLK_RECOVER_INT_EN;  // 0x154
       uint8_t  RESERVED29[7];
  __IO uint8_t  CLK_RECOVER_INT_STATUS; // 0x15c
} USBFS_TypeDef;

typedef struct
{
  __I  uint8_t  SRS0;
  __I  uint8_t  SRS1;
       uint8_t  RESERVED0[2];
  __IO uint8_t  RPFC;
  __IO uint8_t  RPFW;
  __IO uint8_t  FM;
  __IO uint8_t  MR;
  __IO uint8_t  SSRS0;
  __IO uint8_t  SSRS1;
} RCM_TypeDef;

typedef struct {
  __IO uint32_t TCSR;                // 0x00
       uint32_t RESERVED0[1];
  __IO uint32_t TCR2;                // 0x08
  __IO uint32_t TCR3;                // 0x0C
  __IO uint32_t TCR4;                // 0x10
  __IO uint32_t TCR5;                // 0x14
       uint32_t RESERVED1[2];
  __O  uint32_t TDR0;                // 0x20
       uint32_t RESERVED2[15];
  __IO uint32_t TMR;                 // 0x60
       uint32_t RESERVED3[7];
  __IO uint32_t RCSR;                // 0x80
       uint32_t RESERVED4[1];
  __IO uint32_t RCR2;                // 0x88
  __IO uint32_t RCR3;                // 0x8C
  __IO uint32_t RCR4;                // 0x90
  __IO uint32_t RCR5;                // 0x94
       uint32_t RESERVED5[2];
  __I  uint32_t RDR0;                // 0xA0
       uint32_t RESERVED6[15];
  __IO uint32_t RMR;                 // 0xE0
       uint32_t RESERVED7[7];
  __IO uint32_t MCR;                 // 0x100
} I2S_TypeDef;

/****************************************************************/
/*                  Peripheral memory map                       */
/****************************************************************/
#define DMA_BASE                ((uint32_t)0x40008100)
#define FTFA_BASE               ((uint32_t)0x40020000)
#define DMAMUX_BASE             ((uint32_t)0x40021000)
#define I2S0_BASE               ((uint32_t)0x4002F000) // TODO: registers not implemented
#define PIT_BASE                ((uint32_t)0x40037000)
#define LPTPM0_BASE             ((uint32_t)0x40038000)
#define LPTPM1_BASE             ((uint32_t)0x40039000)
#define LPTPM2_BASE             ((uint32_t)0x4003A000)
#define ADC0_BASE               ((uint32_t)0x4003B000)
#define RTC_BASE                ((uint32_t)0x4003D000)
#define DAC0_BASE               ((uint32_t)0x4003F000)
#define LPTMR0_BASE             ((uint32_t)0x40040000)
#define SRF_BASE                ((uint32_t)0x40041000)
#define SIM_BASE                ((uint32_t)0x40047000)
#define PORTA_BASE              ((uint32_t)0x40049000)
#define PORTB_BASE              ((uint32_t)0x4004A000)
#define PORTC_BASE              ((uint32_t)0x4004B000)
#define PORTD_BASE              ((uint32_t)0x4004C000)
#define PORTE_BASE              ((uint32_t)0x4004D000)
#define LPUART0_BASE            ((uint32_t)0x40054000)
#define LPUART1_BASE            ((uint32_t)0x40055000)
#define FLEXIO_BASE             ((uint32_t)0x4005F000) // TODO: register defs
#define MCGLITE_BASE            ((uint32_t)0x40064000)
#define OSC0_BASE               ((uint32_t)0x40065000)
#define I2C0_BASE               ((uint32_t)0x40066000)
#define I2C1_BASE               ((uint32_t)0x40067000)
#define UART2_BASE              ((uint32_t)0x4006C000)
#define USBFS_BASE              ((uint32_t)0x40072000)
#define CMP_BASE                ((uint32_t)0x40073000)
#define VREF_BASE               ((uint32_t)0x40074000)
#define SPI0_BASE               ((uint32_t)0x40076000)
#define SPI1_BASE               ((uint32_t)0x40077000)
#define LLWU_BASE               ((uint32_t)0x4007C000)
#define PMC_BASE                ((uint32_t)0x4007D000)
#define SMC_BASE                ((uint32_t)0x4007E000)
#define RCM_BASE                ((uint32_t)0x4007F000)
#define GPIOA_BASE              ((uint32_t)0x400FF000)
#define GPIOB_BASE              ((uint32_t)0x400FF040)
#define GPIOC_BASE              ((uint32_t)0x400FF080)
#define GPIOD_BASE              ((uint32_t)0x400FF0C0)
#define GPIOE_BASE              ((uint32_t)0x400FF100)
#define MCM_BASE                ((uint32_t)0xF0003000)

/****************************************************************/
/*                 Peripheral declaration                       */
/****************************************************************/
#define DMA                     ((DMA_TypeDef *)     DMA_BASE)
#define FTFA                    ((FTFA_TypeDef *)    FTFA_BASE)
#define DMAMUX                  ((DMAMUX_TypeDef *)  DMAMUX_BASE)
#define I2S0                    ((I2S_TypeDef *)     I2S0_BASE)
#define PIT                     ((PIT_TypeDef *)     PIT_BASE)
#define TPM0                    ((TPM_TypeDef *)     LPTPM0_BASE)
#define TPM1                    ((TPM_TypeDef *)     LPTPM1_BASE)
#define TPM2                    ((TPM_TypeDef *)     LPTPM2_BASE)
#define ADC0                    ((ADC_TypeDef *)     ADC0_BASE)
#define RTC0                    ((RTC_TypeDef *)     RTC0_BASE)
#define DAC0                    ((DAC_TypeDef *)     DAC0_BASE)
#define LPTMR0                  ((LPTMR_TypeDef *)   LPTMR0_BASE)
#define TSI0                    ((TSI_TypeDef *)     TSI0_BASE)
#define SIM                     ((SIM_TypeDef  *)    SIM_BASE)
#define LLWU                    ((LLWU_TypeDef  *)   LLWU_BASE)
#define PMC                     ((PMC_TypeDef  *)    PMC_BASE)
#define PORTA                   ((PORT_TypeDef  *)   PORTA_BASE)
#define PORTB                   ((PORT_TypeDef  *)   PORTB_BASE)
#define PORTC                   ((PORT_TypeDef  *)   PORTC_BASE)
#define PORTD                   ((PORT_TypeDef  *)   PORTD_BASE)
#define PORTE                   ((PORT_TypeDef  *)   PORTE_BASE)
#define USB0                    ((USBFS_TypeDef *)   USBFS_BASE)
#define CMP                     ((CMP_TypeDef *)     CMP_BASE)
#define VREF                    ((VREF_TypeDef *)    VREF_BASE)
#define MCG                     ((MCGLite_TypeDef *) MCGLITE_BASE)
#define OSC0                    ((OSC_TypeDef  *)    OSC0_BASE)
#define SPI0                    ((SPI_TypeDef *)     SPI0_BASE)
#define SPI1                    ((SPI_TypeDef *)     SPI1_BASE)
#define I2C0                    ((I2C_TypeDef *)     I2C0_BASE)
#define I2C1                    ((I2C_TypeDef *)     I2C1_BASE)
#define LPUART0                 ((LPUART_TypeDef *)  LPUART0_BASE)
#define LPUART1                 ((LPUART_TypeDef *)  LPUART1_BASE)
#define UART2                   ((UART_TypeDef *)    UART2_BASE)
#define FLEXIO                  ((FlexIO_TypeDef *)  FLEXIO_BASE)
#define SMC                     ((SMC_TypeDef  *)    SMC_BASE)
#define RCM                     ((RCM_TypeDef  *)    RCM_BASE)
#define SYSTEM_REGISTER_FILE    ((volatile uint8_t *) SRF_BASE) /* 32 bytes */
#define GPIOA                   ((GPIO_TypeDef  *)   GPIOA_BASE)
#define GPIOB                   ((GPIO_TypeDef  *)   GPIOB_BASE)
#define GPIOC                   ((GPIO_TypeDef  *)   GPIOC_BASE)
#define GPIOD                   ((GPIO_TypeDef  *)   GPIOD_BASE)
#define GPIOE                   ((GPIO_TypeDef  *)   GPIOE_BASE)
#define MCM                     ((MCM_TypeDef *)     MCM_BASE)

/****************************************************************/
/*           Peripheral Registers Bits Definition               */
/****************************************************************/

/****************************************************************/
/*                                                              */
/*             System Integration Module (SIM)                  */
/*                                                              */
/****************************************************************/
/*********  Bits definition for SIM_SOPT1 register  *************/
#define SIM_SOPT1_USBREGEN           ((uint32_t)0x80000000)    /*!< USB voltage regulator enable */
#define SIM_SOPT1_USBSSTBY           ((uint32_t)0x40000000)    /*!< USB voltage regulator in standby mode during Stop, VLPS, LLS and VLLS modes */
#define SIM_SOPT1_USBVSTBY           ((uint32_t)0x20000000)    /*!< USB voltage regulator in standby mode during VLPR and VLPW modes */
#define SIM_SOPT1_OSC32KSEL_SHIFT    18                                                                            			    /*!< 32K oscillator clock select (shift) */
#define SIM_SOPT1_OSC32KSEL_MASK     ((uint32_t)((uint32_t)0x03 << SIM_SOPT1_OSC32KSEL_SHIFT))                          	  /*!< 32K oscillator clock select (mask) */
#define SIM_SOPT1_OSC32KSEL(x)       ((uint32_t)(((uint32_t)(x) << SIM_SOPT1_OSC32KSEL_SHIFT) & SIM_SOPT1_OSC32KSEL_MASK))  /*!< 32K oscillator clock select */
#define SIM_SOPT1_OSC32KOUT_SHIFT    16                                                                                     /*!< 32K oscillator clock output (shift) */
#define SIM_SOPT1_OSC32KOUT_MASK     ((uint32_t)((uint32_t)0x03 << SIM_SOPT1_OSC32KSEL_SHIFT))                              /*!< 32K oscillator clock output (mask) */
#define SIM_SOPT1_OSC32KOUT(x)       ((uint32_t)(((uint32_t)(x) << SIM_SOPT1_OSC32KSEL_SHIFT) & SIM_SOPT1_OSC32KSEL_MASK))  /*!< 32K oscillator clock output */

/*******  Bits definition for SIM_SOPT1CFG register  ************/
#define SIM_SOPT1CFG_USSWE           ((uint32_t)0x04000000)    /*!< USB voltage regulator stop standby write enable */
#define SIM_SOPT1CFG_UVSWE           ((uint32_t)0x02000000)    /*!< USB voltage regulator VLP standby write enable */
#define SIM_SOPT1CFG_URWE            ((uint32_t)0x01000000)    /*!< USB voltage regulator voltage regulator write enable */

/*******  Bits definition for SIM_SOPT2 register  ************/
#define SIM_SOPT2_LPUART1SRC_SHIFT   28                                                                                       /*!< LPUART1 clock source select (shift) */
#define SIM_SOPT2_LPUART1SRC_MASK    ((uint32_t)((uint32_t)0x03 << SIM_SOPT2_LPUART1SRC_SHIFT))                               /*!< LPUART1 clock source select (mask) */
#define SIM_SOPT2_LPUART1SRC(x)      ((uint32_t)(((uint32_t)(x) << SIM_SOPT2_LPUART1SRC_SHIFT) & SIM_SOPT2_LPUART1SRC_MASK))  /*!< LPUART1 clock source select */
#define SIM_SOPT2_LPUART0SRC_SHIFT   26                                                                                       /*!< LPUART0 clock source select (shift) */
#define SIM_SOPT2_LPUART0SRC_MASK    ((uint32_t)((uint32_t)0x03 << SIM_SOPT2_LPUART0SRC_SHIFT))                               /*!< LPUART0 clock source select (mask) */
#define SIM_SOPT2_LPUART0SRC(x)      ((uint32_t)(((uint32_t)(x) << SIM_SOPT2_LPUART0SRC_SHIFT) & SIM_SOPT2_LPUART0SRC_MASK))  /*!< UART0 clock source select */
#define SIM_SOPT2_TPMSRC_SHIFT       24                                                                               /*!< TPM clock source select (shift) */
#define SIM_SOPT2_TPMSRC_MASK        ((uint32_t)((uint32_t)0x03 << SIM_SOPT2_TPMSRC_SHIFT))                           /*!< TPM clock source select (mask) */
#define SIM_SOPT2_TPMSRC(x)          ((uint32_t)(((uint32_t)(x) << SIM_SOPT2_TPMSRC_SHIFT) & SIM_SOPT2_TPMSRC_MASK))  /*!< TPM clock source select */
#define SIM_SOPT2_FLEXIOSRC_SHIFT    22                                                                               /*!< FlexIO Module Clock Source Select (shift) */
#define SIM_SOPT2_FLEXIOSRC_MASK     ((uint32_t)((uint32_t)0x03 << SIM_SOPT2_FLEXIO_SHIFT))                           /*!< FlexIO Module Clock Source Select (mask) */
#define SIM_SOPT2_FLEXIOSRC(x)       ((uint32_t)(((uint32_t)(x) << SIM_SOPT2_FLEXIO_SHIFT) & SIM_SOPT2_FLEXIO_MASK))  /*!< FlexIO Module Clock Source Select */
#define SIM_SOPT2_USBSRC             ((uint32_t)0x00040000)    /*!< USB clock source select */
#define SIM_SOPT2_CLKOUTSEL_SHIFT    5                                                                                      /*!< CLKOUT select (shift) */
#define SIM_SOPT2_CLKOUTSEL_MASK     ((uint32_t)((uint32_t)0x07 << SIM_SOPT2_CLKOUTSEL_SHIFT))                              /*!< CLKOUT select (mask) */
#define SIM_SOPT2_CLKOUTSEL(x)       ((uint32_t)(((uint32_t)(x) << SIM_SOPT2_CLKOUTSEL_SHIFT) & SIM_SOPT2_CLKOUTSEL_MASK))  /*!< CLKOUT select */
#define SIM_SOPT2_RTCCLKOUTSEL       ((uint32_t)0x00000010)    /*!< RTC clock out select */

/*******  Bits definition for SIM_SOPT4 register  ************/
#define SIM_SOPT4_TPM2CLKSEL         ((uint32_t)0x04000000)    /*!< TPM2 External Clock Pin Select */
#define SIM_SOPT4_TPM1CLKSEL         ((uint32_t)0x02000000)    /*!< TPM1 External Clock Pin Select */
#define SIM_SOPT4_TPM0CLKSEL         ((uint32_t)0x01000000)    /*!< TPM0 External Clock Pin Select */
#define SIM_SOPT4_TPM2CH0SRC         ((uint32_t)0x00100000)    /*!< TPM2 channel 0 input capture source select */
#define SIM_SOPT4_TPM1CH0SRC_SHIFT   18                                                                                       /*!< TPM1 channel 0 input capture source select (shift) */
#define SIM_SOPT4_TPM1CH0SRC_MASK    ((uint32_t)((uint32_t)0x03 << SIM_SOPT4_TPM1CH0SRC_SHIFT))                               /*!< TPM1 channel 0 input capture source select (mask) */
#define SIM_SOPT4_TPM1CH0SRC(x)      ((uint32_t)(((uint32_t)(x) << SIM_SOPT4_TPM1CH0SRC_SHIFT) & SIM_SOPT4_TPM1CH0SRC_MASK))  /*!< TPM1 channel 0 input capture source select */

/*******  Bits definition for SIM_SOPT5 register  ************/
#define SIM_SOPT5_UART2ODE           ((uint32_t)0x00040000)    /*!< UART2 Open Drain Enable */
#define SIM_SOPT5_LPUART1ODE         ((uint32_t)0x00020000)    /*!< LPUART1 Open Drain Enable */
#define SIM_SOPT5_LPUART0ODE         ((uint32_t)0x00010000)    /*!< LPUART0 Open Drain Enable */
#define SIM_SOPT5_LPUART1RXSRC       ((uint32_t)0x00000040)    /*!< LPUART1 receive data source select */
#define SIM_SOPT5_LPUART1TXSRC_SHIFT 4                                                                                            /*!< LPUART1 transmit data source select (shift) */
#define SIM_SOPT5_LPUART1TXSRC_MASK  ((uint32_t)((uint32_t)0x03 << SIM_SOPT5_LPUART1TXSRC_SHIFT))                                 /*!< LPUART1 transmit data source select (mask) */
#define SIM_SOPT5_LPUART1TXSRC(x)    ((uint32_t)(((uint32_t)(x) << SIM_SOPT5_LPUART1TXSRC_SHIFT) & SIM_SOPT5_LPUART1TXSRC_MASK))  /*!< LPUART1 transmit data source select */
#define SIM_SOPT5_LPUART0RXSRC       ((uint32_t)0x00000040)    /*!< LPUART0 receive data source select */
#define SIM_SOPT5_LPUART0TXSRC_SHIFT 0                                                                                            /*!< LPUART0 transmit data source select (shift) */
#define SIM_SOPT5_LPUART0TXSRC_MASK  ((uint32_t)((uint32_t)0x03 << SIM_SOPT5_LPUART0TXSRC_SHIFT))                                 /*!< LPUART0 transmit data source select (mask) */
#define SIM_SOPT5_LPUART0TXSRC(x)    ((uint32_t)(((uint32_t)(x) << SIM_SOPT5_LPUART0TXSRC_SHIFT) & SIM_SOPT5_LPUART0TXSRC_MASK))  /*!< LPUART0 transmit data source select */

/*******  Bits definition for SIM_SOPT7 register  ************/
#define SIM_SOPT7_ADC0ALTTRGEN       ((uint32_t)0x00000080)    /*!< ADC0 Alternate Trigger Enable */
#define SIM_SOPT7_ADC0PRETRGSEL      ((uint32_t)0x00000010)    /*!< ADC0 Pretrigger Select */
#define SIM_SOPT7_ADC0TRGSEL_SHIFT   0                                                                                        /*!< ADC0 Trigger Select (shift) */
#define SIM_SOPT7_ADC0TRGSEL_MASK    ((uint32_t)((uint32_t)0x0F << SIM_SOPT7_ADC0TRGSEL_SHIFT))                               /*!< ADC0 Trigger Select (mask) */
#define SIM_SOPT7_ADC0TRGSEL(x)      ((uint32_t)(((uint32_t)(x) << SIM_SOPT7_ADC0TRGSEL_SHIFT) & SIM_SOPT7_ADC0TRGSEL_MASK))  /*!< ADC0 Trigger Select */

/********  Bits definition for SIM_SDID register  ************/
#define SIM_SDID_FAMID_SHIFT         28                                                      /*!< Kinetis family ID (shift) */
#define SIM_SDID_FAMID_MASK          ((uint32_t)((uint32_t)0x0F << SIM_SDID_FAMID_SHIFT))    /*!< Kinetis family ID (mask) */
#define SIM_SDID_SUBFAMID_SHIFT      24                                                      /*!< Kinetis Sub-Family ID (shift) */
#define SIM_SDID_SUBFAMID_MASK       ((uint32_t)((uint32_t)0x0F << SIM_SDID_SUBFAMID_SHIFT)) /*!< Kinetis Sub-Family ID (mask) */
#define SIM_SDID_SERIESID_SHIFT      20                                                      /*!< Kinetis Series ID (shift) */
#define SIM_SDID_SERIESID_MASK       ((uint32_t)((uint32_t)0x0F << SIM_SDID_SERIESID_SHIFT)) /*!< Kinetis Series ID (mask) */
#define SIM_SDID_SRAMSIZE_SHIFT      16                                                      /*!< System SRAM Size (shift) */
#define SIM_SDID_SRAMSIZE_MASK       ((uint32_t)((uint32_t)0x0F << SIM_SDID_SRAMSIZE_SHIFT)) /*!< System SRAM Size (mask) */
#define SIM_SDID_REVID_SHIFT         12                                                      /*!< Device revision number (shift) */
#define SIM_SDID_REVID_MASK          ((uint32_t)((uint32_t)0x0F << SIM_SDID_REVID_SHIFT))    /*!< Device revision number (mask) */
#define SIM_SDID_PINID_SHIFT         0                                                       /*!< Pincount identification (shift) */
#define SIM_SDID_PINID_MASK          ((uint32_t)((uint32_t)0x0F << SIM_SDID_PINID_SHIFT))    /*!< Pincount identification (mask) */

/*******  Bits definition for SIM_SCGC4 register  ************/
#define SIM_SCGC4_SPI1               ((uint32_t)0x00800000)    /*!< SPI1 Clock Gate Control */
#define SIM_SCGC4_SPI0               ((uint32_t)0x00400000)    /*!< SPI0 Clock Gate Control */
#define SIM_SCGC4_VREF               ((uint32_t)0x00100000)    /*!< VREF Clock Gate Control */
#define SIM_SCGC4_CMP0               ((uint32_t)0x00080000)    /*!< Comparator Clock Gate Control */
#define SIM_SCGC4_USBFS              ((uint32_t)0x00040000)    /*!< USB Clock Gate Control */
#define SIM_SCGC4_UART2              ((uint32_t)0x00001000)    /*!< UART2 Clock Gate Control */
#define SIM_SCGC4_I2C1               ((uint32_t)0x00000080)    /*!< I2C1 Clock Gate Control */
#define SIM_SCGC4_I2C0               ((uint32_t)0x00000040)    /*!< I2C0 Clock Gate Control */

/*******  Bits definition for SIM_SCGC5 register  ************/
#define SIM_SCGC5_FLEXIO             ((uint32_t)0x80000000)    /*!< FlexIO Module */
#define SIM_SCGC5_LPUART1            ((uint32_t)0x00200000)    /*!< LPUART1 Clock Gate Control */
#define SIM_SCGC5_LPUART0            ((uint32_t)0x00100000)    /*!< LPUART0 Clock Gate Control */
#define SIM_SCGC5_PORTE              ((uint32_t)0x00002000)    /*!< Port E Clock Gate Control */
#define SIM_SCGC5_PORTD              ((uint32_t)0x00001000)    /*!< Port D Clock Gate Control */
#define SIM_SCGC5_PORTC              ((uint32_t)0x00000800)    /*!< Port C Clock Gate Control */
#define SIM_SCGC5_PORTB              ((uint32_t)0x00000400)    /*!< Port B Clock Gate Control */
#define SIM_SCGC5_PORTA              ((uint32_t)0x00000200)    /*!< Port A Clock Gate Control */
#define SIM_SCGC5_LPTMR              ((uint32_t)0x00000001)    /*!< Low Power Timer Access Control */

/*******  Bits definition for SIM_SCGC6 register  ************/
#define SIM_SCGC6_DAC0               ((uint32_t)0x80000000)    /*!< DAC0 Clock Gate Control */
#define SIM_SCGC6_RTC                ((uint32_t)0x20000000)    /*!< RTC Access Control */
#define SIM_SCGC6_ADC0               ((uint32_t)0x08000000)    /*!< ADC0 Clock Gate Control */
#define SIM_SCGC6_TPM2               ((uint32_t)0x04000000)    /*!< TPM2 Clock Gate Control */
#define SIM_SCGC6_TPM1               ((uint32_t)0x02000000)    /*!< TPM1 Clock Gate Control */
#define SIM_SCGC6_TPM0               ((uint32_t)0x01000000)    /*!< TPM0 Clock Gate Control */
#define SIM_SCGC6_PIT                ((uint32_t)0x00800000)    /*!< PIT Clock Gate Control */
#define SIM_SCGC6_I2S                ((uint32_t)0x00008000)    /*!< I2S0 Clock Gate Control */
#define SIM_SCGC6_DMAMUX             ((uint32_t)0x00000002)    /*!< DMA Mux Clock Gate Control */
#define SIM_SCGC6_FTF                ((uint32_t)0x00000001)    /*!< Flash Memory Clock Gate Control */

/*******  Bits definition for SIM_SCGC7 register  ************/
#define SIM_SCGC7_DMA                ((uint32_t)0x00000100)    /*!< DMA Clock Gate Control */

/******  Bits definition for SIM_CLKDIV1 register  ***********/
#define SIM_CLKDIV1_OUTDIV1_SHIFT    28                                                                            			 /*!< Clock 1 output divider value (shift) */
#define SIM_CLKDIV1_OUTDIV1_MASK     ((uint32_t)((uint32_t)0x0F << SIM_CLKDIV1_OUTDIV1_SHIFT))                          	 /*!< Clock 1 output divider value (mask) */
#define SIM_CLKDIV1_OUTDIV1(x)       ((uint32_t)(((uint32_t)(x) << SIM_CLKDIV1_OUTDIV1_SHIFT) & SIM_CLKDIV1_OUTDIV1_MASK))  /*!< Clock 1 output divider value */
#define SIM_CLKDIV1_OUTDIV4_SHIFT    16                                                                            			 /*!< Clock 4 output divider value (shift) */
#define SIM_CLKDIV1_OUTDIV4_MASK     ((uint32_t)((uint32_t)0x07 << SIM_CLKDIV1_OUTDIV4_SHIFT))                          	 /*!< Clock 4 output divider value (mask) */
#define SIM_CLKDIV1_OUTDIV4(x)       ((uint32_t)(((uint32_t)(x) << SIM_CLKDIV1_OUTDIV4_SHIFT) & SIM_CLKDIV1_OUTDIV4_MASK))  /*!< Clock 4 output divider value */

/*******  Bits definition for SIM_FCFG1 register  ************/
#define SIM_FCFG1_PFSIZE_SHIFT       24                                                       /*!< Program Flash Size (shift) */
#define SIM_FCFG1_PFSIZE_MASK        ((uint32_t)((uint32_t)0x0F << SIM_FCFG1_PFSIZE_SHIFT))   /*!< Program Flash Size (mask) */
#define SIM_FCFG1_FLASHDOZE          ((uint32_t)0x00000002)    /*!< Flash Doze */
#define SIM_FCFG1_FLASHDIS           ((uint32_t)0x00000001)    /*!< Flash Disable */

/*******  Bits definition for SIM_FCFG2 register  ************/
#define SIM_FCFG2_MAXADDR0_SHIFT     24                                                        /*!< Max address lock (shift) */
#define SIM_FCFG2_MAXADDR0_MASK      ((uint32_t)((uint32_t)0x7F << SIM_FCFG2_MAXADDR0_SHIFT))  /*!< Max address lock (mask) */
#define SIM_FCFG2_MAXADDR1_SHIFT     16                                                        /*!< Max address lock (block 1) (shift) */
#define SIM_FCFG2_MAXADDR1_MASK      ((uint32_t)((uint32_t)0x7F << SIM_FCFG2_MAXADDR1_SHIFT))  /*!< Max address lock (block 1) (mask) */

/*******  Bits definition for SIM_UIDMH register  ************/
#define SIM_UIDMH_UID_MASK           ((uint32_t)0x0000FFFF)   /*!< Unique Identification */

/*******  Bits definition for SIM_UIDML register  ************/
#define SIM_UIDML_UID_MASK           ((uint32_t)0xFFFFFFFF)   /*!< Unique Identification */

/*******  Bits definition for SIM_UIDL register  *************/
#define SIM_UIDL_UID_MASK            ((uint32_t)0xFFFFFFFF)   /*!< Unique Identification */

/*******  Bits definition for SIM_COPC register  *************/
#define SIM_COPC_COPCLKSEL_SHIFT     6                                                                                    /*!< COP Clock Select (shift) */
#define SIM_COPC_COPCLKSEL_MASK      ((uint32_t)((uint32_t)0x03 << SIM_COPC_COPCLKSEL_SHIFT))                             /*!< COP Clock Select (mask) */
#define SIM_COPC_COPCLKSEL(x)        ((uint32_t)(((uint32_t)(x) << SIM_COPC_COPCLKSEL_SHIFT) & SIM_COPC_COPCLKSEL_MASK))  /*!< COP Clock Select */
#define SIM_COPC_COPDBGEN            ((uint32_t)0x00000020)   /*!< COP Debug Enable */
#define SIM_COPC_COPSTPEN            ((uint32_t)0x00000010)   /*!< COP Stop Enable */
#define SIM_COPC_COPT_SHIFT          2                                                                          /*!< COP Watchdog Timeout (shift) */
#define SIM_COPC_COPT_MASK           ((uint32_t)((uint32_t)0x03 << SIM_COPC_COPT_SHIFT))                        /*!< COP Watchdog Timeout (mask) */
#define SIM_COPC_COPT(x)             ((uint32_t)(((uint32_t)(x) << SIM_COPC_COPT_SHIFT) & SIM_COPC_COPT_MASK))  /*!< COP Watchdog Timeout */
#define SIM_COPC_COPCLKS             ((uint32_t)0x00000002)   /*!< COP Clock Select */
#define SIM_COPC_COPW                ((uint32_t)0x00000001)   /*!< COP windowed mode */

/*******  Bits definition for SIM_SRVCOP register  ***********/
#define SIM_SRVCOP_SRVCOP_SHIFT      0                                                                                  /*!< Sevice COP Register (shift) */
#define SIM_SRVCOP_SRVCOP_MASK       ((uint32_t)((uint32_t)0xFF << SIM_SRVCOP_SRVCOP_SHIFT))                            /*!< Sevice COP Register (mask) */
#define SIM_SRVCOP_SRVCOP(x)         ((uint32_t)(((uint32_t)(x) << SIM_SRVCOP_SRVCOP_SHIFT) & SIM_SRVCOP_SRVCOP_MASK))  /*!< Sevice COP Register */


/****************************************************************/
/*                                                              */
/*              Low-Leakage Wakeup Unit (LLWU)                  */
/*                                                              */
/****************************************************************/

/* Device independent */

/****************************************************************/
/*                                                              */
/*           Port Control and interrupts (PORT)                 */
/*                                                              */
/****************************************************************/

/* Device independent */

/****************************************************************/
/*                                                              */
/*                   Oscillator (OSC)                           */
/*                                                              */
/****************************************************************/

/* Device independent */

/****************************************************************/
/*                                                              */
/*                 Direct Memory Access (DMA)                   */
/*                                                              */
/****************************************************************/

/* Device independent */

/****************************************************************/
/*                                                              */
/*         Direct Memory Access Multiplexer (DMAMUX)            */
/*                                                              */
/****************************************************************/

/* Device independent */

/****************************************************************/
/*                                                              */
/*               Periodic Interrupt Timer (PIT)                 */
/*                                                              */
/****************************************************************/

/* Device independent */

/****************************************************************/
/*                                                              */
/*              Analog-to-Digital Converter (ADC)               */
/*                                                              */
/****************************************************************/

/* Device independent */

/****************************************************************/
/*                                                              */
/*                   Low-Power Timer (LPTMR)                    */
/*                                                              */
/****************************************************************/

/* Device independent */

/****************************************************************/
/*                                                              */
/*         Multipurpose Clock Generator Lite (MCG_Lite)         */
/*                                                              */
/****************************************************************/
/***********  Bits definition for MCG_C1 register  **************/
#define MCG_C1_CLKS_SHIFT           6                                                           /*!< Clock source select (shift) */
#define MCG_C1_CLKS_MASK            ((uint8_t)((uint8_t)0x03 << MCG_C1_CLKS_SHIFT))             /*!< Clock source select (mask) */
#define MCG_C1_CLKS(x)              ((uint8_t)(((uint8_t)(x) << MCG_C1_CLKS_SHIFT) & MCG_C1_CLKS_MASK))  /*!< Clock source select */
#define MCG_C1_CLKS_HIRC            MCG_C1_CLKS(0)  /*!< HIRC */
#define MCG_C1_CLKS_LIRC            MCG_C1_CLKS(1)  /*!< LIRC (either LIRC2M or LIRC8M) */
#define MCG_C1_CLKS_EXT             MCG_C1_CLKS(2)  /*!< EXT (external ref) */
#define MCG_C1_IRCLKEN              ((uint8_t)((uint8_t)1 << 1))                                /*!< Internal Reference Clock Enable */
#define MCG_C1_IREFSTEN             ((uint8_t)((uint8_t)1 << 0))                                /*!< Internal Reference Stop Enable */

/***********  Bits definition for MCG_C2 register  **************/
#define MCG_C2_RANGE0_SHIFT         4                                                           /*!< Frequency Range Select (shift) */
#define MCG_C2_RANGE0_MASK          ((uint8_t)((uint8_t)0x03 << MCG_C2_RANGE0_SHIFT))           /*!< Frequency Range Select (mask) */
#define MCG_C2_RANGE0(x)            ((uint8_t)(((uint8_t)(x) << MCG_C2_RANGE0_SHIFT) & MCG_C2_RANGE0_MASK))  /*!< Frequency Range Select */
#define MCG_C2_HGO0                 ((uint8_t)((uint8_t)1 << 3))                                /*!< High Gain Oscillator Select (0=low power; 1=high gain) */
#define MCG_C2_EREFS0               ((uint8_t)((uint8_t)1 << 2))                                /*!< External Reference Select (0=clock; 1=oscillator) */
#define MCG_C2_IRCS                 ((uint8_t)((uint8_t)1 << 0))                                /*!< Internal Reference Clock Select (0=slow; 1=fast) */

/************  Bits definition for MCG_S register  **************/
#define MCG_S_CLKST_SHIFT           2                                                           /*!< Clock Mode Status (shift) */
#define MCG_S_CLKST_MASK            ((uint8_t)((uint8_t)0x03 << MCG_S_CLKST_SHIFT))             /*!< Clock Mode Status (mask) */
#define MCG_S_CLKST(x)              ((uint8_t)(((uint8_t)(x) << MCG_S_CLKST_SHIFT) & MCG_S_CLKST_MASK))  /*!< Clock Mode Status */
#define MCG_S_CLKST_HIRC            MCG_S_CLKST(0)
#define MCG_S_CLKST_LIRC            MCG_S_CLKST(1)
#define MCG_S_CLKST_EXT             MCG_S_CLKST(2)
#define MCG_S_OSCINIT0              ((uint8_t)((uint8_t)1 << 1))                                /*!< OSC Initialization */

/************  Bits definition for MCG_SC register  **************/
#define MCG_SC_FCRDIV_SHIFT         1                                                           /*!< Fast Clock Internal Reference Divider (shift) */
#define MCG_SC_FCRDIV_MASK          ((uint8_t)((uint8_t)0x07 << MCG_SC_FCRDIV_SHIFT))           /*!< Fast Clock Internal Reference Divider (mask) */
#define MCG_SC_FCRDIV(x)            ((uint8_t)(((uint8_t)(x) << MCG_SC_FCRDIV_SHIFT) & MCG_SC_FCRDIV_MASK))  /*!< Fast Clock Internal Reference Divider */
#define MCG_SC_FCRDIV_DIV1          MCG_SC_FCRDIV(0)  /*!< Divide Factor is 1 */
#define MCG_SC_FCRDIV_DIV2          MCG_SC_FCRDIV(1)  /*!< Divide Factor is 2 */
#define MCG_SC_FCRDIV_DIV4          MCG_SC_FCRDIV(2)  /*!< Divide Factor is 4 */
#define MCG_SC_FCRDIV_DIV8          MCG_SC_FCRDIV(3)  /*!< Divide Factor is 8 */
#define MCG_SC_FCRDIV_DIV16         MCG_SC_FCRDIV(4)  /*!< Divide Factor is 16 */
#define MCG_SC_FCRDIV_DIV32         MCG_SC_FCRDIV(5)  /*!< Divide Factor is 32 */
#define MCG_SC_FCRDIV_DIV64         MCG_SC_FCRDIV(6)  /*!< Divide Factor is 64 */
#define MCG_SC_FCRDIV_DIV128        MCG_SC_FCRDIV(7)  /*!< Divide Factor is 128 */

/************  Bits definition for MCG_MC register  *************/
#define MCG_MC_HIRCEN               ((uint8_t)0x80)   /*!< High-frequency IRC Enable */
#define MCG_MC_LIRC_DIV2_SHIFT      0                                                                              /*!< Second Low-frequency Internal Reference Clock Divider (shift) */
#define MCG_MC_LIRC_DIV2_MASK       ((uint8_t)((uint8_t)0x07 << MCG_MC_LIRC_DIV2_SHIFT))                           /*!< Second Low-frequency Internal Reference Clock Divider (mask) */
#define MCG_MC_LIRC_DIV2(x)         ((uint8_t)(((uint8_t)(x) << MCG_MC_LIRC_DIV2_SHIFT) & MCG_MC_LIRC_DIV2_MASK))  /*!< Second Low-frequency Internal Reference Clock Divider */
#define MCG_MC_LIRC_DIV2_DIV1       MCG_MC_LIRC_DIV2(0)  /*!< Divide Factor is 1 */
#define MCG_MC_LIRC_DIV2_DIV2       MCG_MC_LIRC_DIV2(1)  /*!< Divide Factor is 2 */
#define MCG_MC_LIRC_DIV2_DIV4       MCG_MC_LIRC_DIV2(2)  /*!< Divide Factor is 4 */
#define MCG_MC_LIRC_DIV2_DIV8       MCG_MC_LIRC_DIV2(3)  /*!< Divide Factor is 8 */
#define MCG_MC_LIRC_DIV2_DIV16      MCG_MC_LIRC_DIV2(4)  /*!< Divide Factor is 16 */
#define MCG_MC_LIRC_DIV2_DIV32      MCG_MC_LIRC_DIV2(5)  /*!< Divide Factor is 32 */
#define MCG_MC_LIRC_DIV2_DIV64      MCG_MC_LIRC_DIV2(6)  /*!< Divide Factor is 64 */
#define MCG_MC_LIRC_DIV2_DIV128     MCG_MC_LIRC_DIV2(7)  /*!< Divide Factor is 128 */

/****************************************************************/
/*                                                              */
/*             Serial Peripheral Interface (SPI)                */
/*                                                              */
/****************************************************************/
/***********  Bits definition for SPIx_S register  **************/
#define SPIx_S_SPRF                  ((uint8_t)0x80)    /*!< SPI Read Buffer Full Flag */
#define SPIx_S_SPMF                  ((uint8_t)0x40)    /*!< SPI Match Flag */
#define SPIx_S_SPTEF                 ((uint8_t)0x20)    /*!< SPI Transmit Buffer Empty Flag */
#define SPIx_S_MODF                  ((uint8_t)0x10)    /*!< Master Mode Fault Flag */
#define SPIx_S_RNFULLF               ((uint8_t)0x08)    /*!< Receive FIFO nearly full flag */
#define SPIx_S_TNEAREF               ((uint8_t)0x04)    /*!< Transmit FIFO nearly empty flag */
#define SPIx_S_TXFULLF               ((uint8_t)0x02)    /*!< Transmit FIFO full flag */
#define SPIx_S_RFIFOEF               ((uint8_t)0x01)    /*!< SPI read FIFO empty flag */

/***********  Bits definition for SPIx_BR register  *************/
#define SPIx_BR_SPPR_SHIFT           4                  /*!< SPI Baud rate Prescaler Divisor */
#define SPIx_BR_SPPR_MASK            ((uint8_t)((uint8_t)0x7 << SPIx_BR_SPPR_SHIFT))
#define SPIx_BR_SPPR(x)              ((uint8_t)(((uint8_t)(x) << SPIx_BR_SPPR_SHIFT) & SPIx_BR_SPPR_MASK))
#define SPIx_BR_SPR_SHIFT            0                  /*!< SPI Baud rate Divisor */
#define SPIx_BR_SPR_MASK             ((uint8_t)((uint8_t)0x0F << SPIx_BR_SPR_SHIFT))
#define SPIx_BR_SPR(x)               ((uint8_t)(((uint8_t)(x) << SPIx_BR_SPR_SHIFT) & SPIx_BR_SPR_MASK))

/***********  Bits definition for SPIx_C2 register  *************/
#define SPIx_C2_SPMIE                ((uint8_t)0x80)    /*!< SPI Match Interrupt Enable */
#define SPIx_C2_SPIMODE              ((uint8_t)0x40)    /*!< SPI 8-bit or 16-bit mode */
#define SPIx_C2_TXDMAE               ((uint8_t)0x20)    /*!< Transmit DMA Enable */
#define SPIx_C2_MODFEN               ((uint8_t)0x10)    /*!< Master Mode-Fault Function Enable */
#define SPIx_C2_BIDIROE              ((uint8_t)0x08)    /*!< Bidirectional Mode Output Enable */
#define SPIx_C2_RXDMAE               ((uint8_t)0x04)    /*!< Receive DMA Enable */
#define SPIx_C2_SPISWAI              ((uint8_t)0x02)    /*!< SPI Stop in Wait Mode */
#define SPIx_C2_SPC0                 ((uint8_t)0x01)    /*!< SPI Pin Control 0 */

/***********  Bits definition for SPIx_C1 register  *************/
#define SPIx_C1_SPIE                 ((uint8_t)0x80)    /*!< SPI Interrupt Enable */
#define SPIx_C1_SPE                  ((uint8_t)0x40)    /*!< SPI System Enable */
#define SPIx_C1_SPTIE                ((uint8_t)0x20)    /*!< SPI Transmit Interrupt Enable */
#define SPIx_C1_MSTR                 ((uint8_t)0x10)    /*!< Master/Slave Mode Select */
#define SPIx_C1_CPOL                 ((uint8_t)0x08)    /*!< Clock Polarity */
#define SPIx_C1_CPHA                 ((uint8_t)0x04)    /*!< Clock Phase */
#define SPIx_C1_SSOE                 ((uint8_t)0x02)    /*!< Slave Select Output Enable */
#define SPIx_C1_LSBFE                ((uint8_t)0x01)    /*!< LSB First */

/***********  Bits definition for SPIx_ML register  *************/
#define SPIx_ML_DATA_SHIFT           0                  /*!< SPI HW Compare value for Match - low byte */
#define SPIx_ML_DATA_MASK            ((uint8_t)((uint8_t)0xFF << SPIx_ML_DATA_SHIFT))
#define SPIx_ML_DATA(x)              ((uint8_t)(((uint8_t)(x) << SPIx_ML_DATA_SHIFT) & SPIx_ML_DATA_MASK))

/***********  Bits definition for SPIx_MH register  *************/
#define SPIx_MH_DATA_SHIFT           0                  /*!< SPI HW Compare value for Match - high byte */
#define SPIx_MH_DATA_MASK            ((uint8_t)((uint8_t)0xFF << SPIx_MH_DATA_SHIFT))
#define SPIx_MH_DATA(x)              ((uint8_t)(((uint8_t)(x) << SPIx_MH_DATA_SHIFT) & SPIx_MH_DATA_MASK))

/***********  Bits definition for SPIx_DL register  *************/
#define SPIx_DL_DATA_SHIFT            0                  /*!< Data - low byte */
#define SPIx_DL_DATA_MASK             ((uint8_t)((uint8_t)0xFF << SPIx_DL_DATA_SHIFT))
#define SPIx_DL_DATA(x)               ((uint8_t)(((uint8_t)(x) << SPIx_DL_DATA_SHIFT) & SPIx_DL_DATA_MASK))

/***********  Bits definition for SPIx_DH register  *************/
#define SPIx_DH_DATA_SHIFT            0                  /*!< Data - high byte */
#define SPIx_DH_DATA_MASK             ((uint8_t)((uint8_t)0xFF << SPIx_DH_DATA_SHIFT))
#define SPIx_DH_DATA(x)               ((uint8_t)(((uint8_t)(x) << SPIx_DH_DATA_SHIFT) & SPIx_DH_DATA_MASK))

/***********  Bits definition for SPIx_CI register  *************/
#define SPIx_CI_TXFERR                ((uint8_t)0x80)    /*!< Transmit FIFO error flag */
#define SPIx_CI_RXFERR                ((uint8_t)0x40)    /*!< Receive FIFO error flag */
#define SPIx_CI_TXFOF                 ((uint8_t)0x20)    /*!< Transmit FIFO overflow flag */
#define SPIx_CI_RXFOF                 ((uint8_t)0x10)    /*!< Receive FIFO overflow flag */
#define SPIx_CI_TNEAREFCI             ((uint8_t)0x08)    /*!< Transmit FIFO nearly empty flag clear interrupt */
#define SPIx_CI_RNFULLFCI             ((uint8_t)0x04)    /*!< Receive FIFO nearly full flag clear interrupt */
#define SPIx_CI_SPTEFCI               ((uint8_t)0x02)    /*!< Transmit FIFO empty flag clear interrupt */
#define SPIx_CI_SPRFCI                ((uint8_t)0x01)    /*!< Receive FIFO full flag clear interrupt */

/***********  Bits definition for SPIx_C3 register  *************/
#define SPIx_C3_TNEAREF_MARK          ((uint8_t)0x20)    /*!< Transmit FIFO nearly empty watermark */
#define SPIx_C3_RNFULLF_MARK          ((uint8_t)0x10)    /*!< Receive FIFO nearly full watermark */
#define SPIx_C3_INTCLR                ((uint8_t)0x08)    /*!< Interrupt clearing mechanism select */
#define SPIx_C3_TNEARIEN              ((uint8_t)0x04)    /*!< Transmit FIFO nearly empty interrupt enable */
#define SPIx_C3_RNFULLIEN             ((uint8_t)0x02)    /*!< Receive FIFO nearly full interrupt enable */
#define SPIx_C3_FIFOMODE              ((uint8_t)0x01)    /*!< FIFO mode enable */

/****************************************************************/
/*                                                              */
/*   Inter-Integrated Circuit (I2C): Device dependent part      */
/*                                                              */
/****************************************************************/
/***********  Bits definition for I2Cx_FLT register  ************/
#define I2Cx_FLT_SHEN                ((uint8_t)0x80)    /*!< Stop Hold Enable */
#define I2Cx_FLT_STOPF               ((uint8_t)0x40)    /*!< I2C Bus Stop Detect Flag */
#define I2Cx_FLT_SSIE                ((uint8_t)0x20)    /*!< I2C Bus Stop or Start Interrupt Enable */
#define I2Cx_FLT_STARTF              ((uint8_t)0x10)    /*!< I2C Bus Start Detect Flag */
#define I2Cx_FLT_FLT_SHIFT           0                  /*!< I2C Programmable Filter Factor */
#define I2Cx_FLT_FLT_MASK            ((uint8_t)((uint8_t)0x0F << I2Cx_FLT_FLT_SHIFT))
#define I2Cx_FLT_FLT(x)              ((uint8_t)(((uint8_t)(x) << I2Cx_FLT_FLT_SHIFT) & I2Cx_FLT_FLT_MASK))

/***********  Bits definition for I2Cx_S2 register  *************/
#define I2Cx_S2_ERROR                ((uint8_t)0x02)    /*!< Error flag */
#define I2Cx_S2_EMPTY                ((uint8_t)0x01)    /*!< Empty flag */

/****************************************************************/
/*                                                              */
/*     Universal Asynchronous Receiver/Transmitter (UART)       */
/*                                                              */
/****************************************************************/
/*********  Bits definition for UARTx_BDH register  *************/
#define UARTx_BDH_RXEDGIE            ((uint8_t)0x40)    /*!< RX Input Active Edge Interrupt Enable */
#define UARTx_BDH_SBR_SHIFT          0                  /*!< Baud Rate Modulo Divisor */
#define UARTx_BDH_SBR_MASK           ((uint8_t)((uint8_t)0x1F << UARTx_BDH_SBR_SHIFT))
#define UARTx_BDH_SBR(x)             ((uint8_t)(((uint8_t)(x) << UARTx_BDH_SBR_SHIFT) & UARTx_BDH_SBR_MASK))

/*********  Bits definition for UARTx_BDL register  *************/
#define UARTx_BDL_SBR_SHIFT          0                  /*!< Baud Rate Modulo Divisor */
#define UARTx_BDL_SBR_MASK           ((uint8_t)((uint8_t)0xFF << UARTx_BDL_SBR_SHIFT))
#define UARTx_BDL_SBR(x)             ((uint8_t)(((uint8_t)(x) << UARTx_BDL_SBR_SHIFT) & UARTx_BDL_SBR_MASK))

/*********  Bits definition for UARTx_C1 register  **************/
#define UARTx_C1_LOOPS               ((uint8_t)0x80)    /*!< Loop Mode Select */
#define UARTx_C1_RSRC                ((uint8_t)0x20)    /*!< Receiver Source Select */
#define UARTx_C1_M                   ((uint8_t)0x10)    /*!< 9-Bit or 8-Bit Mode Select */
#define UARTx_C1_WAKE                ((uint8_t)0x08)    /*!< Receiver Wakeup Method Select */
#define UARTx_C1_ILT                 ((uint8_t)0x04)    /*!< Idle Line Type Select */
#define UARTx_C1_PE                  ((uint8_t)0x02)    /*!< Parity Enable */
#define UARTx_C1_PT                  ((uint8_t)0x01)    /*!< Parity Type */

/*********  Bits definition for UARTx_C2 register  **************/
#define UARTx_C2_TIE                 ((uint8_t)0x80)    /*!< Transmit Interrupt Enable for TDRE */
#define UARTx_C2_TCIE                ((uint8_t)0x40)    /*!< Transmission Complete Interrupt Enable for TC */
#define UARTx_C2_RIE                 ((uint8_t)0x20)    /*!< Receiver Interrupt Enable for RDRF */
#define UARTx_C2_ILIE                ((uint8_t)0x10)    /*!< Idle Line Interrupt Enable for IDLE */
#define UARTx_C2_TE                  ((uint8_t)0x08)    /*!< Transmitter Enable */
#define UARTx_C2_RE                  ((uint8_t)0x04)    /*!< Receiver Enable */
#define UARTx_C2_RWU                 ((uint8_t)0x02)    /*!< Receiver Wakeup Control */
#define UARTx_C2_SBK                 ((uint8_t)0x01)    /*!< Send Break */

/*********  Bits definition for UARTx_S1 register  **************/
#define UARTx_S1_TDRE                ((uint8_t)0x80)    /*!< Transmit Data Register Empty Flag */
#define UARTx_S1_TC                  ((uint8_t)0x40)    /*!< Transmission Complete Flag */
#define UARTx_S1_RDRF                ((uint8_t)0x20)    /*!< Receiver Data Register Full Flag */
#define UARTx_S1_IDLE                ((uint8_t)0x10)    /*!< Idle Line Flag */
#define UARTx_S1_OR                  ((uint8_t)0x08)    /*!< Receiver Overrun Flag */
#define UARTx_S1_NF                  ((uint8_t)0x04)    /*!< Noise Flag */
#define UARTx_S1_FE                  ((uint8_t)0x02)    /*!< Framing Error Flag */
#define UARTx_S1_PF                  ((uint8_t)0x01)    /*!< Parity Error Flag */

/*********  Bits definition for UARTx_S2 register  **************/
#define UARTx_S2_RXEDGIF             ((uint8_t)0x40)    /*!< UART_RX Pin Active Edge Interrupt Flag */
#define UARTx_S2_MSBF                ((uint8_t)0x20)    /*!< MSB First */
#define UARTx_S2_RXINV               ((uint8_t)0x10)    /*!< Receive Data Inversion */
#define UARTx_S2_RWUID               ((uint8_t)0x08)    /*!< Receive Wake Up Idle Detect */
#define UARTx_S2_BRK13               ((uint8_t)0x04)    /*!< Break Character Generation Length */
#define UARTx_S2_RAF                 ((uint8_t)0x01)    /*!< Receiver Active Flag */

/*********  Bits definition for UARTx_C3 register  **************/
#define UARTx_C3_R8                  ((uint8_t)0x80)    /*!< Ninth Data Bit for Receiver */
#define UARTx_C3_T8                  ((uint8_t)0x40)    /*!< Ninth Data Bit for Transmitter */
#define UARTx_C3_TXDIR               ((uint8_t)0x20)    /*!< UART_TX Pin Direction in Single-Wire Mode */
#define UARTx_C3_TXINV               ((uint8_t)0x10)    /*!< Transmit Data Inversion */
#define UARTx_C3_ORIE                ((uint8_t)0x08)    /*!< Overrun Interrupt Enable */
#define UARTx_C3_NEIE                ((uint8_t)0x04)    /*!< Noise Error Interrupt Enable */
#define UARTx_C3_FEIE                ((uint8_t)0x02)    /*!< Framing Error Interrupt Enable */
#define UARTx_C3_PEIE                ((uint8_t)0x01)    /*!< Parity Error Interrupt Enable */

/*********  Bits definition for UARTx_D register  ***************/
#define UARTx_D_R7T7                 ((uint8_t)0x80)    /*!< Read receive data buffer 7 or write transmit data buffer 7 */
#define UARTx_D_R6T6                 ((uint8_t)0x40)    /*!< Read receive data buffer 6 or write transmit data buffer 6 */
#define UARTx_D_R5T5                 ((uint8_t)0x20)    /*!< Read receive data buffer 5 or write transmit data buffer 5 */
#define UARTx_D_R4T4                 ((uint8_t)0x10)    /*!< Read receive data buffer 4 or write transmit data buffer 4 */
#define UARTx_D_R3T3                 ((uint8_t)0x08)    /*!< Read receive data buffer 3 or write transmit data buffer 3 */
#define UARTx_D_R2T2                 ((uint8_t)0x04)    /*!< Read receive data buffer 2 or write transmit data buffer 2 */
#define UARTx_D_R1T1                 ((uint8_t)0x02)    /*!< Read receive data buffer 1 or write transmit data buffer 1 */
#define UARTx_D_R0T0                 ((uint8_t)0x01)    /*!< Read receive data buffer 0 or write transmit data buffer 0 */
#define UARTx_D_RT_SHIFT             0
#define UARTx_D_RT_MASK              ((uint8_t)0xFF)

/*********  Bits definition for UARTx_MA1 register  *************/
#define UARTx_MA1_MA_SHIFT           0                  /*!< Match Address */
#define UARTx_MA1_MA_MASK            ((uint8_t)((uint8_t)0xFF << UARTx_MA1_MA_SHIFT))
#define UARTx_MA1_MA(x)              ((uint8_t)(((uint8_t)(x) << UARTx_MA1_MA_SHIFT) & UARTx_MA1_MA_MASK))

/*********  Bits definition for UARTx_MA2 register  *************/
#define UARTx_MA2_MA_SHIFT           0                  /*!< Match Address */
#define UARTx_MA2_MA_MASK            ((uint8_t)((uint8_t)0xFF << UARTx_MA2_MA_SHIFT))
#define UARTx_MA2_MA(x)              ((uint8_t)(((uint8_t)(x) << UARTx_MA2_MA_SHIFT) & UARTx_MA2_MA_MASK))

/*********  Bits definition for UARTx_C4 register  **************/
#define UARTx_C4_MAEN1               ((uint8_t)0x80)    /*!< Match Address Mode Enable 1 */
#define UARTx_C4_MAEN2               ((uint8_t)0x40)    /*!< Match Address Mode Enable 2 */
#define UARTx_C4_M10                 ((uint8_t)0x20)    /*!< 10-bit Mode Select */
#define UARTx_C4_OSR_SHIFT           0                  /*!< Over Sampling Ratio */
#define UARTx_C4_OSR_MASK            ((uint8_t)((uint8_t)0x1F << UARTx_C4_OSR_SHIFT))
#define UARTx_C4_OSR(x)              ((uint8_t)(((uint8_t)(x) << UARTx_C4_OSR_SHIFT) & UARTx_C4_OSR_MASK))

/*********  Bits definition for UARTx_C5 register  **************/
#define UARTx_C5_TDMAE               ((uint8_t)0x80)    /*!< Transmitter DMA Enable */
#define UARTx_C5_RDMAE               ((uint8_t)0x20)    /*!< Receiver Full DMA Enable */
#define UARTx_C5_BOTHEDGE            ((uint8_t)0x02)    /*!< Both Edge Sampling */
#define UARTx_C5_RESYNCDIS           ((uint8_t)0x01)    /*!< Resynchronization Disable */

/****************************************************************/
/*                                                              */
/*Low Power Universal asynchronous receiver/transmitter (LPUART)*/
/*                                                              */
/****************************************************************/
/*********  Bits definition for LPUARTx_BAUD register  **********/
#define LPUARTx_BAUD_MAEN1           ((uint32_t)0x80000000)  /*!< Match Address Mode Enable 1 */
#define LPUARTx_BAUD_MAEN2           ((uint32_t)0x40000000)  /*!< Match Address Mode Enable 2 */
#define LPUARTx_BAUD_M10             ((uint32_t)0x20000000)  /*!< 10-bit Mode select */
#define LPUARTx_BAUD_OSR_SHIFT       24                                                                               /*!< Over Sampling Ratio (shift) */
#define LPUARTx_BAUD_OSR_MASK        ((uint32_t)((uint32_t)0x1F << LPUARTx_BAUD_OSR_SHIFT))                           /*!< Over Sampling Ratio (mask) */
#define LPUARTx_BAUD_OSR(x)          ((uint32_t)(((uint32_t)(x) << LPUARTx_BAUD_OSR_SHIFT) & LPUARTx_BAUD_OSR_MASK))  /*!< Over Sampling Ratio */
#define LPUARTx_BAUD_TDMAE           ((uint32_t)0x00800000)  /*!< Transmitter DMA Enable */
#define LPUARTx_BAUD_RDMAE           ((uint32_t)0x00200000)  /*!< Receiver Full DMA Enable */
#define LPUARTx_BAUD_MATCFG_SHIFT    18                                                                                     /*!< Match Configuration (shift) */
#define LPUARTx_BAUD_MATCFG_MASK     ((uint32_t)((uint32_t)0x03 << LPUARTx_BAUD_MATCFG_SHIFT))                              /*!< Match Configuration (mask) */
#define LPUARTx_BAUD_MATCFG(x)       ((uint32_t)(((uint32_t)(x) << LPUARTx_BAUD_MATCFG_SHIFT) & LPUARTx_BAUD_MATCFG_MASK))  /*!< Match Configuration */
#define LPUARTx_BAUD_BOTHEDGE        ((uint32_t)0x00020000)  /*!< Both Edge Sampling */
#define LPUARTx_BAUD_RESYNCDIS       ((uint32_t)0x00010000)  /*!< Resynchronization Disable */
#define LPUARTx_BAUD_LBKDIE          ((uint32_t)0x00008000)  /*!< LIN Break Detect Interrupt Enable */
#define LPUARTx_BAUD_RXEDGIE         ((uint32_t)0x00004000)  /*!< RX Input Active Edge Interrupt Enable */
#define LPUARTx_BAUD_SBNS            ((uint32_t)0x00002000)  /*!< Stop Bit Number Select */
#define LPUARTx_BAUD_SBR_SHIFT       0                                                                                /*!< Baud Rate Modulo Divisor (shift) */
#define LPUARTx_BAUD_SBR_MASK        ((uint32_t)((uint32_t)0x1FFF << LPUARTx_BAUD_SBR_SHIFT))                         /*!< Baud Rate Modulo Divisor (mask) */
#define LPUARTx_BAUD_SBR(x)          ((uint32_t)(((uint32_t)(x) << LPUARTx_BAUD_SBR_SHIFT) & LPUARTx_BAUD_SBR_MASK))  /*!< Baud Rate Modulo Divisor */

/*********  Bits definition for LPUARTx_STAT register  **********/
#define LPUARTx_STAT_LBKDIF          ((uint32_t)0x80000000)  /*!< LIN Break Detect Interrupt Flag */
#define LPUARTx_STAT_RXEDGIF         ((uint32_t)0x40000000)  /*!< LPUART_RX Pin Active Edge Interrupt Flag */
#define LPUARTx_STAT_MSBF            ((uint32_t)0x20000000)  /*!< MSB First */
#define LPUARTx_STAT_RXINV           ((uint32_t)0x10000000)  /*!< Receive Data Inversion */
#define LPUARTx_STAT_RWUID           ((uint32_t)0x08000000)  /*!< Receive Wake Up Idle Detect */
#define LPUARTx_STAT_BRK13           ((uint32_t)0x04000000)  /*!< Break Character Generation Length */
#define LPUARTx_STAT_LBKDE           ((uint32_t)0x02000000)  /*!< LIN Break Detection Enable */
#define LPUARTx_STAT_RAF             ((uint32_t)0x01000000)  /*!< Receiver Active Flag */
#define LPUARTx_STAT_TDRE            ((uint32_t)0x00800000)  /*!< Transmit Data Register Empty Flag */
#define LPUARTx_STAT_TC              ((uint32_t)0x00400000)  /*!< Transmission Complete Flag */
#define LPUARTx_STAT_RDRF            ((uint32_t)0x00200000)  /*!< Receive Data Register Full Flag */
#define LPUARTx_STAT_IDLE            ((uint32_t)0x00100000)  /*!< Idle Line Flag */
#define LPUARTx_STAT_OR              ((uint32_t)0x00080000)  /*!< Receiver Overrun Flag */
#define LPUARTx_STAT_NF              ((uint32_t)0x00040000)  /*!< Noise Flag */
#define LPUARTx_STAT_FE              ((uint32_t)0x00020000)  /*!< Framing Error Flag */
#define LPUARTx_STAT_PF              ((uint32_t)0x00010000)  /*!< Parity Error Flag */
#define LPUARTx_STAT_MA1F            ((uint32_t)0x00008000)  /*!< Match 1 Flag */
#define LPUARTx_STAT_MA2F            ((uint32_t)0x00004000)  /*!< Match 2 Flag */

/*********  Bits definition for LPUARTx_CTRL register  **********/
#define LPUARTx_CTRL_R8T9            ((uint32_t)0x80000000)  /*!< Receive Bit 8 / Transmit Bit 9 */
#define LPUARTx_CTRL_R9T8            ((uint32_t)0x40000000)  /*!< Receive Bit 9 / Transmit Bit 8 */
#define LPUARTx_CTRL_TXDIR           ((uint32_t)0x20000000)  /*!< LPUART_TX Pin Direction in Single-Wire Mode */
#define LPUARTx_CTRL_TXINV           ((uint32_t)0x10000000)  /*!< Transmit Data Inversion */
#define LPUARTx_CTRL_ORIE            ((uint32_t)0x08000000)  /*!< Overrun Interrupt Enable */
#define LPUARTx_CTRL_NEIE            ((uint32_t)0x04000000)  /*!< Noise Error Interrupt Enable */
#define LPUARTx_CTRL_FEIE            ((uint32_t)0x02000000)  /*!< Framing Error Interrupt Enable */
#define LPUARTx_CTRL_PEIE            ((uint32_t)0x01000000)  /*!< Parity Error Interrupt Enable */
#define LPUARTx_CTRL_TIE             ((uint32_t)0x00800000)  /*!< Transmit Interrupt Enable */
#define LPUARTx_CTRL_TCIE            ((uint32_t)0x00400000)  /*!< Transmission Complete Interrupt Enable */
#define LPUARTx_CTRL_RIE             ((uint32_t)0x00200000)  /*!< Receiver Interrupt Enable */
#define LPUARTx_CTRL_ILIE            ((uint32_t)0x00100000)  /*!< Idle Line Interrupt Enable */
#define LPUARTx_CTRL_TE              ((uint32_t)0x00080000)  /*!< Transmitter Enable */
#define LPUARTx_CTRL_RE              ((uint32_t)0x00040000)  /*!< Receiver Enable */
#define LPUARTx_CTRL_RWU             ((uint32_t)0x00020000)  /*!< Receiver Wakeup Control */
#define LPUARTx_CTRL_SBK             ((uint32_t)0x00010000)  /*!< Send Break */
#define LPUARTx_CTRL_MA1IE           ((uint32_t)0x00008000)  /*!< Match 1 Interrupt Enable */
#define LPUARTx_CTRL_MA2IE           ((uint32_t)0x00004000)  /*!< Match 2 Interrupt Enable */
#define LPUARTx_CTRL_IDLECFG_SHIFT   8                                                                                        /*!< Idle Configuration (shift) */
#define LPUARTx_CTRL_IDLECFG_MASK    ((uint32_t)((uint32_t)0x7 << LPUARTx_CTRL_IDLECFG_SHIFT))                                /*!< Idle Configuration (mask) */
#define LPUARTx_CTRL_IDLECFG(x)      ((uint32_t)(((uint32_t)(x) << LPUARTx_CTRL_IDLECFG_SHIFT) & LPUARTx_CTRL_IDLECFG_MASK))  /*!< Idle Configuration */
#define LPUARTx_CTRL_LOOPS           ((uint32_t)0x00000080)  /*!< Loop Mode Select */
#define LPUARTx_CTRL_DOZEEN          ((uint32_t)0x00000040)  /*!< Doze Enable */
#define LPUARTx_CTRL_RSRC            ((uint32_t)0x00000020)  /*!< Receiver Source Select */
#define LPUARTx_CTRL_M               ((uint32_t)0x00000010)  /*!< 9-Bit or 8-Bit Mode Select */
#define LPUARTx_CTRL_WAKE            ((uint32_t)0x00000008)  /*!< Receiver Wakeup Method Select */
#define LPUARTx_CTRL_ILT             ((uint32_t)0x00000004)  /*!< Idle Line Type Select */
#define LPUARTx_CTRL_PE              ((uint32_t)0x00000002)  /*!< Parity Enable */
#define LPUARTx_CTRL_PT              ((uint32_t)0x00000001)  /*!< Parity Type */

/*********  Bits definition for LPUARTx_DATA register  **********/
#define LPUARTx_DATA_NOISY           ((uint32_t)0x00008000)  /*!< The current received dataword contained in DATA[R9:R0] was received with noise */
#define LPUARTx_DATA_PARITYE         ((uint32_t)0x00004000)  /*!< The current received dataword contained in DATA[R9:R0] was received with a parity error */
#define LPUARTx_DATA_FRETSC          ((uint32_t)0x00002000)  /*!< Frame Error / Transmit Special Character */
#define LPUARTx_DATA_RXEMPT          ((uint32_t)0x00001000)  /*!< Receive Buffer Empty */
#define LPUARTx_DATA_IDLINE          ((uint32_t)0x00000800)  /*!< Idle Line */
#define LPUARTx_DATA_R9T9            ((uint32_t)0x00000200)  /*!< Read receive data buffer 9 or write transmit data buffer 9 */
#define LPUARTx_DATA_R8T8            ((uint32_t)0x00000100)  /*!< Read receive data buffer 8 or write transmit data buffer 8 */
#define LPUARTx_DATA_R7T7            ((uint32_t)0x00000080)  /*!< Read receive data buffer 7 or write transmit data buffer 7 */
#define LPUARTx_DATA_R6T6            ((uint32_t)0x00000040)  /*!< Read receive data buffer 6 or write transmit data buffer 6 */
#define LPUARTx_DATA_R5T5            ((uint32_t)0x00000020)  /*!< Read receive data buffer 5 or write transmit data buffer 5 */
#define LPUARTx_DATA_R4T4            ((uint32_t)0x00000010)  /*!< Read receive data buffer 4 or write transmit data buffer 4 */
#define LPUARTx_DATA_R3T3            ((uint32_t)0x00000008)  /*!< Read receive data buffer 3 or write transmit data buffer 3 */
#define LPUARTx_DATA_R2T2            ((uint32_t)0x00000004)  /*!< Read receive data buffer 2 or write transmit data buffer 2 */
#define LPUARTx_DATA_R1T1            ((uint32_t)0x00000002)  /*!< Read receive data buffer 1 or write transmit data buffer 1 */
#define LPUARTx_DATA_R0T0            ((uint32_t)0x00000001)  /*!< Read receive data buffer 0 or write transmit data buffer 0 */
#define LPUARTx_DATA_DATA_SHIFT      0                                                                                  /*!< Data (shift) */
#define LPUARTx_DATA_DATA_MASK       ((uint32_t)((uint32_t)0x3F << LPUARTx_DATA_DATA_SHIFT))                            /*!< Data (mask) */
#define LPUARTx_DATA_DATA(x)         ((uint32_t)(((uint32_t)(x) << LPUARTx_DATA_DATA_SHIFT) & LPUARTx_DATA_DATA_MASK))  /*!< Data */

/*********  Bits definition for LPUARTx_MATCH register  *********/
#define LPUARTx_MATCH_MA2_SHIFT      16                                                                                 /*!< Match Address 2 (shift) */
#define LPUARTx_MATCH_MA2_MASK       ((uint32_t)((uint32_t)0x3F << LPUARTx_MATCH_MA2_SHIFT))                            /*!< Match Address 2 (mask) */
#define LPUARTx_MATCH_MA2(x)         ((uint32_t)(((uint32_t)(x) << LPUARTx_MATCH_MA2_SHIFT) & LPUARTx_MATCH_MA2_MASK))  /*!< Match Address 2 */
#define LPUARTx_MATCH_MA1_SHIFT      0                                                                                  /*!< Match Address 1 (shift) */
#define LPUARTx_MATCH_MA1_MASK       ((uint32_t)((uint32_t)0x3F << LPUARTx_MATCH_MA1_SHIFT))                            /*!< Match Address 1 (mask) */
#define LPUARTx_MATCH_MA1(x)         ((uint32_t)(((uint32_t)(x) << LPUARTx_MATCH_MA1_SHIFT) & LPUARTx_MATCH_MA1_MASK))  /*!< Match Address 1 */

/****************************************************************/
/*                                                              */
/*             Power Management Controller (PMC)                */
/*                                                              */
/****************************************************************/

/* Device independent */

/****************************************************************/
/*                                                              */
/*                  Timer/PWM Module (TPM)                      */
/*                                                              */
/****************************************************************/
/**********  Bits definition for TPMx_SC register  ***************/
#define TPMx_SC_DMA                   ((uint32_t)0x100)  /*!< DMA Enable */
#define TPMx_SC_TOF                   ((uint32_t)0x80)   /*!< Timer Overflow Flag */
#define TPMx_SC_TOIE                  ((uint32_t)0x40)   /*!< Timer Overflow Interrupt Enable */
#define TPMx_SC_CPWMS                 ((uint32_t)0x20)   /*!< Center-aligned PWM Select */
#define TPMx_SC_CMOD_SHIFT            3                  /*!< Clock Mode Selection */
#define TPMx_SC_CMOD_MASK             ((uint32_t)((uint32_t)0x3 << TPMx_SC_CMOD_SHIFT))
#define TPMx_SC_CMOD(x)               ((uint32_t)(((uint32_t)(x) << TPMx_SC_CMOD_SHIFT) & TPMx_SC_CMOD_MASK))
#define TPMx_SC_PS_SHIFT              0                  /*!< Prescale Factor Selection */
#define TPMx_SC_PS_MASK               ((uint32_t)((uint32_t)0x7 << TPMx_SC_PS_SHIFT))
#define TPMx_SC_PS(x)                 ((uint32_t)(((uint32_t)(x) << TPMx_SC_PS_SHIFT) & TPMx_SC_PS_MASK))

/**********  Bits definition for TPMx_CNT register  **************/
#define TPMx_CNT_COUNT_SHIFT          0                  /*!< Counter Value */
#define TPMx_CNT_COUNT_MASK           ((uint32_t)((uint32_t)0xFFFF << TPMx_CNT_COUNT_SHIFT))
#define TPMx_CNT_COUNT(x)             ((uint32_t)(((uint32_t)(x) << TPMx_CNT_COUNT_SHIFT) & TPMx_CNT_COUNT_MASK))

/**********  Bits definition for TPMx_MOD register  **************/
#define TPMx_MOD_MOD_SHIFT            0                  /*!< Modulo Value */
#define TPMx_MOD_MOD_MASK             ((uint32_t)((uint32_t)0xFFFF << TPMx_MOD_MOD_SHIFT))
#define TPMx_MOD_MOD(x)               ((uint32_t)(((uint32_t)(x) << TPMx_MOD_MOD_SHIFT) & TPMx_MOD_MOD_MASK))

/**********  Bits definition for TPMx_CnSC register  *************/
#define TPMx_CnSC_CHF                 ((uint32_t)0x80)   /*!< Channel Flag */
#define TPMx_CnSC_CHIE                ((uint32_t)0x40)   /*!< Channel Interrupt Enable */
#define TPMx_CnSC_MSB                 ((uint32_t)0x20)   /*!< Channel Mode Select */
#define TPMx_CnSC_MSA                 ((uint32_t)0x10)   /*!< Channel Mode Select */
#define TPMx_CnSC_ELSB                ((uint32_t)0x8)    /*!< Edge or Level Select */
#define TPMx_CnSC_ELSA                ((uint32_t)0x4)    /*!< Edge or Level Select */
#define TPMx_CnSC_DMA                 ((uint32_t)0x1)    /*!< DMA Enable */

/**********  Bits definition for TPMx_CnV register  **************/
#define TPMx_CnV_VAL_SHIFT            0                  /*!< Channel Value */
#define TPMx_CnV_VAL_MASK             ((uint32_t)((uint32_t)0xFFFF << TPMx_CnV_VAL_SHIFT))
#define TPMx_CnV_VAL(x)               ((uint32_t)(((uint32_t)(x) << TPMx_CnV_VAL_SHIFT) & TPMx_CnV_VAL_MASK))

/*********  Bits definition for TPMx_STATUS register  ************/
#define TPMx_STATUS_TOF               ((uint32_t)0x100)  /*!< Timer Overflow Flag */
#define TPMx_STATUS_CH5F              ((uint32_t)0x20)   /*!< Channel 5 Flag */
#define TPMx_STATUS_CH4F              ((uint32_t)0x10)   /*!< Channel 4 Flag */
#define TPMx_STATUS_CH3F              ((uint32_t)0x8)    /*!< Channel 3 Flag */
#define TPMx_STATUS_CH2F              ((uint32_t)0x4)    /*!< Channel 2 Flag */
#define TPMx_STATUS_CH1F              ((uint32_t)0x2)    /*!< Channel 1 Flag */
#define TPMx_STATUS_CH0F              ((uint32_t)0x1)    /*!< Channel 0 Flag */

/**********  Bits definition for TPMx_POL register  **************/
#define TPMx_POL_POL5                 ((uint32_t)0x20)   /*!< Channel 5 Polarity */
#define TPMx_POL_POL4                 ((uint32_t)0x10)   /*!< Channel 4 Polarity */
#define TPMx_POL_POL3                 ((uint32_t)0x08)   /*!< Channel 3 Polarity */
#define TPMx_POL_POL2                 ((uint32_t)0x04)   /*!< Channel 2 Polarity */
#define TPMx_POL_POL1                 ((uint32_t)0x02)   /*!< Channel 1 Polarity */
#define TPMx_POL_POL0                 ((uint32_t)0x01)   /*!< Channel 0 Polarity */

/**********  Bits definition for TPMx_CONF register  *************/
#define TPMx_CONF_TRGSEL_SHIFT        24                   /*!< Trigger Select */
#define TPMx_CONF_TRGSEL_MASK         ((uint32_t)((uint32_t)0xF << TPMx_CONF_TRGSEL_SHIFT))
#define TPMx_CONF_TRGSEL(x)           ((uint32_t)(((uint32_t)(x) << TPMx_CONF_TRGSEL_SHIFT) & TPMx_CONF_TRGSEL_MASK))
#define TPMx_CONF_TRGSRC              ((uint32_t)0x800000) /*!< Trigger Source */
#define TPMx_CONF_TRGPOL              ((uint32_t)0x400000) /*!< Trigger Polarity */
#define TPMx_CONF_CPOT                ((uint32_t)0x80000)  /*!< Counter Pause On Trigger */
#define TPMx_CONF_CROT                ((uint32_t)0x40000)  /*!< Counter Reload On Trigger */
#define TPMx_CONF_CSOO                ((uint32_t)0x20000)  /*!< Counter Stop On Overflow */
#define TPMx_CONF_CSOT                ((uint32_t)0x10000)  /*!< Counter Start on Trigger */
#define TPMx_CONF_GTBEEN              ((uint32_t)0x200)    /*!< Global time base enable */
#define TPMx_CONF_GTBSYNC             ((uint32_t)0x100)    /*!< Global Time Base Synchronization */
#define TPMx_CONF_DBGMODE_SHIFT       6                    /*!< Debug Mode */
#define TPMx_CONF_DBGMODE_MASK        ((uint32_t)((uint32_t)0x3 << TPMx_CONF_DBGMODE_SHIFT))
#define TPMx_CONF_DBGMODE(x)          ((uint32_t)(((uint32_t)(x) << TPMx_CONF_DBGMODE_SHIFT) & TPMx_CONF_DBGMODE_MASK))
#define TPMx_CONF_DOZEEN              ((uint32_t)0x20)     /*!< Doze Enable */

/****************************************************************/
/*                                                              */
/*               USBFS: Device dependent parts                  */
/*                                                              */
/****************************************************************/
/******** Bits definition for USBx_USBTRC0 register *************/
#define USBx_USBTRC0_USB_CLK_RECOVERY_INT ((uint8_t)0x04) /* Combined USB Clock Recovery interrupt status */

/****** Bits definition for USBx_CLK_RECOVER_CTRL register ******/
#define USBx_CLK_RECOVER_CTRL_CLOCK_RECOVER_EN      ((uint8_t)0x80) /*!< Crystal-less USB enable */
#define USBx_CLK_RECOVER_CTRL_RESET_RESUME_ROUGH_EN ((uint8_t)0x40) /*!< Reset/resume to rough phase enable */
#define USBx_CLK_RECOVER_CTRL_RESTART_IFRTRIM_EN    ((uint8_t)0x20) /*!< Restart from IFR trim value */

/****** Bits definition for USBx_CLK_RECOVER_IRC_EN register ****/
#define USBx_CLK_RECOVER_IRC_EN_IRC_EN              ((uint8_t)0x02) /*!< IRC48M enable */

/****** Bits definition for USBx_CLK_RECOVER_INT_EN register ****/
#define USBx_CLK_RECOVER_INT_EN_OVF_ERROR_EN        ((uint8_t)0x10) /*!< Determines whether OVF_ERROR condition signal is used in generation of USB_CLK_RECOVERY_INT. */

/*** Bits definition for USBx_CLK_RECOVER_INT_STATUS register ***/
#define USBx_CLK_RECOVER_INT_STATUS_OVF_ERROR       ((uint8_t)0x10) /*!< frequency trim adjustment needed for the IRC48M output clock is outside the available TRIM_FINE adjustment range */

/****************************************************************/
/*                                                              */
/*                 Reset Control Module (RCM)                   */
/*                                                              */
/****************************************************************/
/* Device independent parts, plus: */
/*********** Bits definition for RCM_FM register ****************/
#define RCM_FM_FORCEROM_SHIFT         1                  /*!< Force ROM Boot */
#define RCM_FM_FORCEROM_MASK          ((uint8_t)((uint8_t)0x03 << RCM_FM_FORCEROM_SHIFT))
#define RCM_FM_FORCEROM(x)            ((uint8_t)(((uint8_t)(x) << RCM_FM_FORCEROM_SHIFT) & RCM_FM_FORCEROM_MASK))

/*********** Bits definition for RCM_MR register ****************/
#define RCM_MR_BOOTROM_SHIFT          1                  /*!< Boot ROM Configuration */
#define RCM_MR_BOOTROM_MASK           ((uint8_t)((uint8_t)0x03 << RCM_MR_BOOTROM_SHIFT))
#define RCM_MR_BOOTROM(x)             ((uint8_t)(((uint8_t)(x) << RCM_MR_BOOTROM_SHIFT) & RCM_MR_BOOTROM_MASK))
#define RCM_MR_BOOTROM_FROM_FLASH         RCM_MR_BOOTROM(0)
#define RCM_MR_BOOTROM_FROM_ROM_BOOTCFG0  RCM_MR_BOOTROM(1)
#define RCM_MR_BOOTROM_FROM_ROM_FOPT      RCM_MR_BOOTROM(2)
#define RCM_MR_BOOTROM_FROM_ROM_BOTH      RCM_MR_BOOTROM(3)

/**********  Bits definition for RCM_SSRS0 register  ************/
#define RCM_SSRS0_SPOR                ((uint8_t)0x80)   /*!< Sticky Power-On Reset */
#define RCM_SSRS0_SPIN                ((uint8_t)0x40)   /*!< Sticky External Reset Pin */
#define RCM_SSRS0_SWDOG               ((uint8_t)0x20)   /*!< Sticky Watchdog */
#define RCM_SSRS0_SLVD                ((uint8_t)0x02)   /*!< Sticky Low-Voltage Detect Reset */
#define RCM_SSRS0_SWAKEUP             ((uint8_t)0x01)   /*!< Sticky Low Leakage Wakeup Reset */

/**********  Bits definition for RCM_SSRS1 register  *************/
#define RCM_SSRS1_SSACKERR            ((uint8_t)0x20)   /*!< Sticky Stop Mode Acknowledge Error Reset */
#define RCM_SSRS1_SMDM_AP             ((uint8_t)0x08)   /*!< Sticky MDM-AP System Reset Request */
#define RCM_SSRS1_SSW                 ((uint8_t)0x04)   /*!< Sticky Software */
#define RCM_SSRS1_SLOCKUP             ((uint8_t)0x02)   /*!< Sticky Core Lockup */

/****************************************************************/
/*                                                              */
/*                System Mode Controller (SMC)                  */
/*                                                              */
/****************************************************************/

/* Device independent */

/****************************************************************/
/*                                                              */
/*              Digital-to-Analog Converter (DAC)               */
/*                                                              */
/****************************************************************/

/* Mostly Device independent */

#define DACx_C1_DACBFMD_SHIFT         1     /*!< DAC Buffer Work Mode Select */
#define DACx_C1_DACBFMD_MASK          ((uint8_t)((uint8_t)0x03 << DACx_C1_DACBFMD_  SHIFT))
#define DACx_C1_DACBFMD(x)            ((uint8_t)(((uint8_t)(x) << DACx_C1_DACBFMD_SHIFT) & DACx_C1_DACBFMD_MASK))

#define DACx_C1_DACBFMD_MODE_NORMAL   0x0
#define DACx_C1_DACBFMD_MODE_OTS      0x2
#define DACx_C1_DACBFMD_MODE_FIFO     0x3

/****************************************************************/
/*                                                              */
/*                     Real Time Clock (RTC)                    */
/*                                                              */
/****************************************************************/

/* Device independent */

/****************************************************************/
/*                                                              */
/*                       Comparator (CMP)                       */
/*                                                              */
/****************************************************************/

/* Device independent */

/****************************************************************/
/*                                                              */
/*                  Flash Memory Module (FTFA)                  */
/*                                                              */
/****************************************************************/

/* Device independent */

/****************************************************************/
/*                                                              */
/*                  Voltage Reference (VREFV1)                  */
/*                                                              */
/****************************************************************/
/********** Bits definition for VREF_TRM register ***************/
#define VREF_TRM_CHOPEN               ((uint8_t)0x40)   /*!< Chop oscillator enable. */
#define VREF_TRM_TRIM_SHIFT           0                 /*!< Trim bits */
#define VREF_TRM_TRIM_MASK            ((uint8_t)((uint8_t)0x3F << VREF_TRM_TRIM_SHIFT))
#define VREF_TRM_TRIM(x)              ((uint8_t)(((uint8_t)(x) << VREF_TRM_TRIM_SHIFT) & VREF_TRM_TRIM_MASK))

/********** Bits definition for VREF_SC register ****************/
#define VREF_SC_VREFEN                ((uint8_t)0x80)   /*!< Internal Voltage Reference enable */
#define VREF_SC_REGEN                 ((uint8_t)0x40)   /*!< Regulator enable */
#define VREF_SC_ICOMPEN               ((uint8_t)0x20)   /*!< Second order curvature compensation enable */
#define VREF_SC_VREFST                ((uint8_t)0x04)   /*!< Internal Voltage Reference stable */
#define VREF_SC_MODE_LV_SHIFT         0                 /*!< Buffer Mode selection */
#define VREF_SC_MODE_LV_MASK          ((uint8_t)((uint8_t)0x3 << VREF_SC_MODE_LV_SHIFT))
#define VREF_SC_MODE_LV(x)            ((uint8_t)(((uint8_t)(x) << VREF_SC_MODE_LV_SHIFT) & VREF_SC_MODE_LV_MASK))

#define VREF_SC_MODE_LV_BANDGAP_ONLY  VREF_SC_MODE_LV(0)
#define VREF_SC_MODE_LV_HIGH_POWER    VREF_SC_MODE_LV(1)
#define VREF_SC_MODE_LV_LOW_POWER     VREF_SC_MODE_LV(2)

#endif /* _KL27ZXXX_H_ */

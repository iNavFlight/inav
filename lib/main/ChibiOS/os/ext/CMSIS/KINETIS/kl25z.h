/*
 * Copyright (C) 2013-2014 Fabio Utzig, http://fabioutzig.com
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

#ifndef _KL25Z_H_
#define _KL25Z_H_

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
  UART0_IRQn                    = 12,
  UART1_IRQn                    = 13,
  UART2_IRQn                    = 14,
  ADC0_IRQn                     = 15,
  CMP0_IRQn                     = 16,
  TPM0_IRQn                     = 17,
  TPM1_IRQn                     = 18,
  TPM2_IRQn                     = 19,
  RTC0_IRQn                     = 20,
  RTC1_IRQn                     = 21,
  PIT_IRQn                      = 22,
  Reserved1_IRQn                = 23,
  USB_OTG_IRQn                  = 24,
  DAC0_IRQn                     = 25,
  TSI0_IRQn                     = 26,
  MCG_IRQn                      = 27,
  LPTMR0_IRQn                   = 28,
  Reserved2_IRQn                = 29,
  PINA_IRQn                     = 30,
  PIND_IRQn                     = 31,
} IRQn_Type;

/*
 * ==========================================================================
 * ----------- Processor and Core Peripheral Section ------------------------
 * ==========================================================================
 */

/**
 * @brief KL2x Interrupt Number Definition, according to the selected device
 *        in @ref Library_configuration_section
 */
#define __MPU_PRESENT             0
#define __VTOR_PRESENT            1
#define __NVIC_PRIO_BITS          2
#define __Vendor_SysTickConfig    0

#include "core_cm0plus.h"        /* Cortex-M0+ processor and core peripherals */

typedef struct
{
  __IO uint32_t SOPT1;
  __IO uint32_t SOPT1CFG;
       uint32_t RESERVED0[1023];
  __IO uint32_t SOPT2;
  __I  uint32_t RESERVED1[1];
  __IO uint32_t SOPT4;
  __IO uint32_t SOPT5;
       uint32_t RESERVED2[1];
  __IO uint32_t SOPT7;
       uint32_t RESERVED3[2];
  __IO uint32_t SDID;
       uint32_t RESERVED4[3];
  __IO uint32_t SCGC4;
  __IO uint32_t SCGC5;
  __IO uint32_t SCGC6;
  __IO uint32_t SCGC7;
  __IO uint32_t CLKDIV1;
       uint32_t RESERVED5[1];
  __IO uint32_t FCFG1;
  __IO uint32_t FCFG2;
       uint32_t RESERVED6[1];
  __IO uint32_t UIDMH;
  __IO uint32_t UIDML;
  __IO uint32_t UIDL;
       uint32_t RESERVED7[39];
  __IO uint32_t COPC;
  __IO uint32_t SRVCOP;
} SIM_TypeDef;

typedef struct
{
  __IO uint8_t  PE1;
  __IO uint8_t  PE2;
  __IO uint8_t  PE3;
  __IO uint8_t  PE4;
  __IO uint8_t  ME;
  __IO uint8_t  F1;
  __IO uint8_t  F2;
  __I  uint8_t  F3;
  __IO uint8_t  FILT1;
  __IO uint8_t  FILT2;
} LLWU_TypeDef;

typedef struct
{
  __IO uint32_t PCR[32];
  __IO uint32_t GPCLR;
  __IO uint32_t GPCHR;
       uint32_t RESERVED0[6];
  __IO uint32_t ISFR;
} PORT_TypeDef;

typedef struct
{
  __IO uint8_t  C1;
  __IO uint8_t  C2;
  __IO uint8_t  C3;
  __IO uint8_t  C4;
  __IO uint8_t  C5;
  __IO uint8_t  C6;
  __IO uint8_t  S;
       uint8_t  RESERVED0[1];
  __IO uint8_t  SC;
       uint8_t  RESERVED1[1];
  __IO uint8_t  ATCVH;
  __IO uint8_t  ATCVL;
  __IO uint8_t  C7;
  __IO uint8_t  C8;
  __IO uint8_t  C9;
  __IO uint8_t  C10;
} MCG_TypeDef;

typedef struct
{
  __IO uint8_t  CR;
} OSC_TypeDef;

typedef struct
{
  __IO uint32_t  SAR;
  __IO uint32_t  DAR;
  __IO uint32_t  DSR_BCR;
  __IO uint32_t  DCR;
} DMAChannel_TypeDef;

typedef struct
{
  DMAChannel_TypeDef ch[4];
} DMA_TypeDef;

typedef struct
{
  __IO uint8_t  CHCFG[4];
} DMAMUX_TypeDef;

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
       uint32_t RESERVED1[12];
  __IO uint32_t CONF;
} TPM_TypeDef;

typedef struct
{
  __IO uint32_t SC1A;           // ADC Status and Control Registers 1
  __IO uint32_t SC1B;           // ADC Status and Control Registers 1
  __IO uint32_t CFG1;           // ADC Configuration Register 1
  __IO uint32_t CFG2;           // ADC Configuration Register 2
  __I  uint32_t RA;             // ADC Data Result Register
  __I  uint32_t RB;             // ADC Data Result Register
  __IO uint32_t CV1;            // Compare Value Registers
  __IO uint32_t CV2;            // Compare Value Registers
  __IO uint32_t SC2;            // Status and Control Register 2
  __IO uint32_t SC3;            // Status and Control Register 3
  __IO uint32_t OFS;            // ADC Offset Correction Register
  __IO uint32_t PG;             // ADC Plus-Side Gain Register
  __IO uint32_t MG;             // ADC Minus-Side Gain Register
  __IO uint32_t CLPD;           // ADC Plus-Side General Calibration Value Register
  __IO uint32_t CLPS;           // ADC Plus-Side General Calibration Value Register
  __IO uint32_t CLP4;           // ADC Plus-Side General Calibration Value Register
  __IO uint32_t CLP3;           // ADC Plus-Side General Calibration Value Register
  __IO uint32_t CLP2;           // ADC Plus-Side General Calibration Value Register
  __IO uint32_t CLP1;           // ADC Plus-Side General Calibration Value Register
  __IO uint32_t CLP0;           // ADC Plus-Side General Calibration Value Register
       uint32_t RESERVED0[1];   // ADC Minus-Side General Calibration Value Register
  __IO uint32_t CLMD;           // ADC Minus-Side General Calibration Value Register
  __IO uint32_t CLMS;           // ADC Minus-Side General Calibration Value Register
  __IO uint32_t CLM4;           // ADC Minus-Side General Calibration Value Register
  __IO uint32_t CLM3;           // ADC Minus-Side General Calibration Value Register
  __IO uint32_t CLM2;           // ADC Minus-Side General Calibration Value Register
  __IO uint32_t CLM1;           // ADC Minus-Side General Calibration Value Register
  __IO uint32_t CLM0;           // ADC Minus-Side General Calibration Value Register
} ADC_TypeDef;

typedef struct
{
  __IO uint32_t CSR;
  __IO uint32_t PSR;
  __IO uint32_t CMR;
  __I  uint32_t CNR;
} LPTMR_TypeDef;

typedef struct
{
  __IO uint32_t GENCS;
  __IO uint32_t DATA;
  __IO uint32_t TSHD;
} TSI_TypeDef;

typedef struct
{
  __IO uint32_t PDOR;
  __IO uint32_t PSOR;
  __IO uint32_t PCOR;
  __IO uint32_t PTOR;
  __IO uint32_t PDIR;
  __IO uint32_t PDDR;
} GPIO_TypeDef;

typedef struct
{
  __IO uint8_t  C1;
  __IO uint8_t  C2;
  __IO uint8_t  BR;
  __IO uint8_t  S;
       uint8_t  RESERVED0[1];
  __IO uint8_t  D;
       uint8_t  RESERVED1[1];
  __IO uint8_t  M;
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
} I2C_TypeDef;

typedef struct
{
  __IO uint8_t  BDH;
  __IO uint8_t  BDL;
  __IO uint8_t  C1;
  __IO uint8_t  C2;
  __IO uint8_t  S1;
  __IO uint8_t  S2;
  __IO uint8_t  C3;
  __IO uint8_t  D;
  __IO uint8_t  C4;
} UART_TypeDef;

typedef struct
{
  __IO uint8_t  BDH;
  __IO uint8_t  BDL;
  __IO uint8_t  C1;
  __IO uint8_t  C2;
  __IO uint8_t  S1;
  __IO uint8_t  S2;
  __IO uint8_t  C3;
  __IO uint8_t  D;
  __IO uint8_t  MA1;
  __IO uint8_t  MA2;
  __IO uint8_t  C4;
  __IO uint8_t  C5;
} UARTLP_TypeDef;

typedef struct
{
  __IO uint8_t  LVDSC1;
  __IO uint8_t  LVDSC2;
  __IO uint8_t  REGSC;
} PMC_TypeDef;

/****************************************************************/
/*                  Peripheral memory map                       */
/****************************************************************/
#define DMA_BASE                ((uint32_t)0x40008100)
#define DMAMUX_BASE             ((uint32_t)0x40021000)
#define TPM0_BASE               ((uint32_t)0x40038000)
#define TPM1_BASE               ((uint32_t)0x40039000)
#define TPM2_BASE               ((uint32_t)0x4003A000)
#define ADC0_BASE               ((uint32_t)0x4003B000)
#define LPTMR0_BASE             ((uint32_t)0x40040000)
#define TSI0_BASE               ((uint32_t)0x40045000)
#define SIM_BASE                ((uint32_t)0x40047000)
#define PORTA_BASE              ((uint32_t)0x40049000)
#define PORTB_BASE              ((uint32_t)0x4004A000)
#define PORTC_BASE              ((uint32_t)0x4004B000)
#define PORTD_BASE              ((uint32_t)0x4004C000)
#define PORTE_BASE              ((uint32_t)0x4004D000)
#define MCG_BASE                ((uint32_t)0x40064000)
#define OSC0_BASE               ((uint32_t)0x40065000)
#define I2C0_BASE               ((uint32_t)0x40066000)
#define I2C1_BASE               ((uint32_t)0x40067000)
#define UART0_BASE              ((uint32_t)0x4006A000)
#define UART1_BASE              ((uint32_t)0x4006B000)
#define UART2_BASE              ((uint32_t)0x4006C000)
#define SPI0_BASE               ((uint32_t)0x40076000)
#define SPI1_BASE               ((uint32_t)0x40077000)
#define LLWU_BASE               ((uint32_t)0x4007C000)
#define PMC_BASE                ((uint32_t)0x4007D000)
#define GPIOA_BASE              ((uint32_t)0x400FF000)
#define GPIOB_BASE              ((uint32_t)0x400FF040)
#define GPIOC_BASE              ((uint32_t)0x400FF080)
#define GPIOD_BASE              ((uint32_t)0x400FF0C0)
#define GPIOE_BASE              ((uint32_t)0x400FF100)

/****************************************************************/
/*                 Peripheral declaration                       */
/****************************************************************/
#define DMA                     ((DMA_TypeDef *)     DMA_BASE)
#define DMAMUX                  ((DMAMUX_TypeDef *)  DMAMUX_BASE)
#define TPM0                    ((TPM_TypeDef *)     TPM0_BASE)
#define TPM1                    ((TPM_TypeDef *)     TPM1_BASE)
#define TPM2                    ((TPM_TypeDef *)     TPM2_BASE)
#define ADC0                    ((ADC_TypeDef *)     ADC0_BASE)
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
#define MCG                     ((MCG_TypeDef  *)    MCG_BASE)
#define OSC0                    ((OSC_TypeDef  *)    OSC0_BASE)
#define SPI0                    ((SPI_TypeDef *)     SPI0_BASE)
#define SPI1                    ((SPI_TypeDef *)     SPI1_BASE)
#define I2C0                    ((I2C_TypeDef *)     I2C0_BASE)
#define I2C1                    ((I2C_TypeDef *)     I2C1_BASE)
#define UART0                   ((UARTLP_TypeDef *)  UART0_BASE)
#define UART1                   ((UART_TypeDef *)    UART1_BASE)
#define UART2                   ((UART_TypeDef *)    UART2_BASE)
#define GPIOA                   ((GPIO_TypeDef  *)   GPIOA_BASE)
#define GPIOB                   ((GPIO_TypeDef  *)   GPIOB_BASE)
#define GPIOC                   ((GPIO_TypeDef  *)   GPIOC_BASE)
#define GPIOD                   ((GPIO_TypeDef  *)   GPIOD_BASE)
#define GPIOE                   ((GPIO_TypeDef  *)   GPIOE_BASE)

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
#define SIM_SOPT1_OSC32KSEL_SHIFT    18                                                                            			 /*!< 32K oscillator clock select (shift) */
#define SIM_SOPT1_OSC32KSEL_MASK     ((uint32_t)((uint32_t)0x03 << SIM_SOPT1_OSC32KSEL_SHIFT))                          	 /*!< 32K oscillator clock select (mask) */
#define SIM_SOPT1_OSC32KSEL(x)       ((uint32_t)(((uint32_t)(x) << SIM_SOPT1_OSC32KSEL_SHIFT) & SIM_SOPT1_OSC32KSEL_MASK))  /*!< 32K oscillator clock select */

/*******  Bits definition for SIM_SOPT1CFG register  ************/
#define SIM_SOPT1CFG_USSWE           ((uint32_t)0x04000000)    /*!< USB voltage regulator stop standby write enable */
#define SIM_SOPT1CFG_UVSWE           ((uint32_t)0x02000000)    /*!< USB voltage regulator VLP standby write enable */
#define SIM_SOPT1CFG_URWE            ((uint32_t)0x01000000)    /*!< USB voltage regulator voltage regulator write enable */

/*******  Bits definition for SIM_SOPT2 register  ************/
#define SIM_SOPT2_UART0SRC_SHIFT     26                                                                                   /*!< UART0 clock source select (shift) */
#define SIM_SOPT2_UART0SRC_MASK      ((uint32_t)((uint32_t)0x03 << SIM_SOPT2_UART0SRC_SHIFT))                             /*!< UART0 clock source select (mask) */
#define SIM_SOPT2_UART0SRC(x)        ((uint32_t)(((uint32_t)(x) << SIM_SOPT2_UART0SRC_SHIFT) & SIM_SOPT2_UART0SRC_MASK))  /*!< UART0 clock source select */
#define SIM_SOPT2_TPMSRC_SHIFT       24                                                                               /*!< TPM clock source select (shift) */
#define SIM_SOPT2_TPMSRC_MASK        ((uint32_t)((uint32_t)0x03 << SIM_SOPT2_TPMSRC_SHIFT))                           /*!< TPM clock source select (mask) */
#define SIM_SOPT2_TPMSRC(x)          ((uint32_t)(((uint32_t)(x) << SIM_SOPT2_TPMSRC_SHIFT) & SIM_SOPT2_TPMSRC_MASK))  /*!< TPM clock source select */
#define SIM_SOPT2_USBSRC             ((uint32_t)0x00040000)    /*!< USB clock source select */
#define SIM_SOPT2_PLLFLLSEL          ((uint32_t)0x00010000)    /*!< PLL/FLL clock select */
#define SIM_SOPT2_CLKOUTSEL_SHIFT    5                                                                                      /*!< CLKOUT select (shift) */
#define SIM_SOPT2_CLKOUTSEL_MASK     ((uint32_t)((uint32_t)0x07 << SIM_SOPT2_CLKOUTSEL_SHIFT))                              /*!< CLKOUT select (mask) */
#define SIM_SOPT2_CLKOUTSEL(x)       ((uint32_t)(((uint32_t)(x) << SIM_SOPT2_CLKOUTSEL_SHIFT) & SIM_SOPT2_CLKOUTSEL_MASK))  /*!< CLKOUT select */
#define SIM_SOPT2_RTCCLKOUTSEL       ((uint32_t)0x00000010)    /*!< RTC clock out select */

/*******  Bits definition for SIM_SCGC4 register  ************/
#define SIM_SCGC4_SPI1               ((uint32_t)0x00800000)    /*!< SPI1 Clock Gate Control */
#define SIM_SCGC4_SPI0               ((uint32_t)0x00400000)    /*!< SPI0 Clock Gate Control */
#define SIM_SCGC4_CMP                ((uint32_t)0x00080000)    /*!< Comparator Clock Gate Control */
#define SIM_SCGC4_USBOTG             ((uint32_t)0x00040000)    /*!< USB Clock Gate Control */
#define SIM_SCGC4_UART2              ((uint32_t)0x00001000)    /*!< UART2 Clock Gate Control */
#define SIM_SCGC4_UART1              ((uint32_t)0x00000800)    /*!< UART1 Clock Gate Control */
#define SIM_SCGC4_UART0              ((uint32_t)0x00000400)    /*!< UART0 Clock Gate Control */
#define SIM_SCGC4_I2C1               ((uint32_t)0x00000080)    /*!< I2C1 Clock Gate Control */
#define SIM_SCGC4_I2C0               ((uint32_t)0x00000040)    /*!< I2C0 Clock Gate Control */

/*******  Bits definition for SIM_SCGC5 register  ************/
#define SIM_SCGC5_PORTE              ((uint32_t)0x00002000)    /*!< Port E Clock Gate Control */
#define SIM_SCGC5_PORTD              ((uint32_t)0x00001000)    /*!< Port D Clock Gate Control */
#define SIM_SCGC5_PORTC              ((uint32_t)0x00000800)    /*!< Port C Clock Gate Control */
#define SIM_SCGC5_PORTB              ((uint32_t)0x00000400)    /*!< Port B Clock Gate Control */
#define SIM_SCGC5_PORTA              ((uint32_t)0x00000200)    /*!< Port A Clock Gate Control */
#define SIM_SCGC5_TSI                ((uint32_t)0x00000020)    /*!< TSI Access Control */
#define SIM_SCGC5_LPTMR              ((uint32_t)0x00000001)    /*!< Low Power Timer Access Control */

/*******  Bits definition for SIM_SCGC6 register  ************/
#define SIM_SCGC6_DAC0               ((uint32_t)0x80000000)    /*!< DAC0 Clock Gate Control */
#define SIM_SCGC6_RTC                ((uint32_t)0x20000000)    /*!< RTC Access Control */
#define SIM_SCGC6_ADC0               ((uint32_t)0x08000000)    /*!< ADC0 Clock Gate Control */
#define SIM_SCGC6_TPM2               ((uint32_t)0x04000000)    /*!< TPM2 Clock Gate Control */
#define SIM_SCGC6_TPM1               ((uint32_t)0x02000000)    /*!< TPM1 Clock Gate Control */
#define SIM_SCGC6_TPM0               ((uint32_t)0x01000000)    /*!< TPM0 Clock Gate Control */
#define SIM_SCGC6_PIT                ((uint32_t)0x00800000)    /*!< PIT Clock Gate Control */
#define SIM_SCGC6_DMAMUX             ((uint32_t)0x00000002)    /*!< DMA Mux Clock Gate Control */
#define SIM_SCGC6_FTF               ((uint32_t)0x00000001)    /*!< Flash Memory Clock Gate Control */

/*******  Bits definition for SIM_SCGC6 register  ************/
#define SIM_SCGC7_DMA                ((uint32_t)0x00000100)    /*!< DMA Clock Gate Control */

/******  Bits definition for SIM_CLKDIV1 register  ***********/
#define SIM_CLKDIV1_OUTDIV1_SHIFT    28                                                                            			 /*!< Clock 1 output divider value (shift) */
#define SIM_CLKDIV1_OUTDIV1_MASK     ((uint32_t)((uint32_t)0x0F << SIM_CLKDIV1_OUTDIV1_SHIFT))                          	 /*!< Clock 1 output divider value (mask) */
#define SIM_CLKDIV1_OUTDIV1(x)       ((uint32_t)(((uint32_t)(x) << SIM_CLKDIV1_OUTDIV1_SHIFT) & SIM_CLKDIV1_OUTDIV1_MASK))  /*!< Clock 1 output divider value */
#define SIM_CLKDIV1_OUTDIV4_SHIFT    16                                                                            			 /*!< Clock 4 output divider value (shift) */
#define SIM_CLKDIV1_OUTDIV4_MASK     ((uint32_t)((uint32_t)0x07 << SIM_CLKDIV1_OUTDIV4_SHIFT))                          	 /*!< Clock 4 output divider value (mask) */
#define SIM_CLKDIV1_OUTDIV4(x)       ((uint32_t)(((uint32_t)(x) << SIM_CLKDIV1_OUTDIV4_SHIFT) & SIM_CLKDIV1_OUTDIV4_MASK))  /*!< Clock 4 output divider value */

/****************************************************************/
/*                                                              */
/*              Low-Leakage Wakeup Unit (LLWU)                  */
/*                                                              */
/****************************************************************/
/**********  Bits definition for LLWU_PE1 register  *************/
#define LLWU_PE1_WUPE3_SHIFT        6                                                                          /*!< Wakeup Pin Enable for LLWU_P3 (shift) */
#define LLWU_PE1_WUPE3_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE1_WUPE3_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P3 (mask) */
#define LLWU_PE1_WUPE3(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE1_WUPE3_SHIFT) & LLWU_PE1_WUPE3_MASK))  /*!< Wakeup Pin Enable for LLWU_P3 */
#define LLWU_PE1_WUPE2_SHIFT        4                                                                          /*!< Wakeup Pin Enable for LLWU_P2 (shift) */
#define LLWU_PE1_WUPE2_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE1_WUPE2_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P2 (mask) */
#define LLWU_PE1_WUPE2(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE1_WUPE2_SHIFT) & LLWU_PE1_WUPE2_MASK))  /*!< Wakeup Pin Enable for LLWU_P2 */
#define LLWU_PE1_WUPE1_SHIFT        2                                                                          /*!< Wakeup Pin Enable for LLWU_P1 (shift) */
#define LLWU_PE1_WUPE1_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE1_WUPE1_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P1 (mask) */
#define LLWU_PE1_WUPE1(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE1_WUPE1_SHIFT) & LLWU_PE1_WUPE1_MASK))  /*!< Wakeup Pin Enable for LLWU_P1 */
#define LLWU_PE1_WUPE0_SHIFT        0                                                                          /*!< Wakeup Pin Enable for LLWU_P0 (shift) */
#define LLWU_PE1_WUPE0_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE1_WUPE0_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P0 (mask) */
#define LLWU_PE1_WUPE0(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE1_WUPE0_SHIFT) & LLWU_PE1_WUPE0_MASK))  /*!< Wakeup Pin Enable for LLWU_P0 */

/**********  Bits definition for LLWU_PE2 register  *************/
#define LLWU_PE2_WUPE7_SHIFT        6                                                                          /*!< Wakeup Pin Enable for LLWU_P7 (shift) */
#define LLWU_PE2_WUPE7_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE2_WUPE7_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P7 (mask) */
#define LLWU_PE2_WUPE7(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE2_WUPE7_SHIFT) & LLWU_PE2_WUPE7_MASK))  /*!< Wakeup Pin Enable for LLWU_P7 */
#define LLWU_PE2_WUPE6_SHIFT        4                                                                          /*!< Wakeup Pin Enable for LLWU_P6 (shift) */
#define LLWU_PE2_WUPE6_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE2_WUPE6_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P6 (mask) */
#define LLWU_PE2_WUPE6(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE2_WUPE6_SHIFT) & LLWU_PE2_WUPE6_MASK))  /*!< Wakeup Pin Enable for LLWU_P6 */
#define LLWU_PE2_WUPE5_SHIFT        2                                                                          /*!< Wakeup Pin Enable for LLWU_P5 (shift) */
#define LLWU_PE2_WUPE5_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE2_WUPE5_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P5 (mask) */
#define LLWU_PE2_WUPE5(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE2_WUPE5_SHIFT) & LLWU_PE2_WUPE5_MASK))  /*!< Wakeup Pin Enable for LLWU_P5 */
#define LLWU_PE2_WUPE4_SHIFT        0                                                                          /*!< Wakeup Pin Enable for LLWU_P4 (shift) */
#define LLWU_PE2_WUPE4_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE2_WUPE4_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P4 (mask) */
#define LLWU_PE2_WUPE4(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE2_WUPE4_SHIFT) & LLWU_PE2_WUPE4_MASK))  /*!< Wakeup Pin Enable for LLWU_P4 */

/**********  Bits definition for LLWU_PE3 register  *************/
#define LLWU_PE3_WUPE11_SHIFT       6                                                                            /*!< Wakeup Pin Enable for LLWU_P11 (shift) */
#define LLWU_PE3_WUPE11_MASK        ((uint8_t)((uint8_t)0x03 << LLWU_PE3_WUPE11_SHIFT))                          /*!< Wakeup Pin Enable for LLWU_P11 (mask) */
#define LLWU_PE3_WUPE11(x)          ((uint8_t)(((uint8_t)(x) << LLWU_PE3_WUPE11_SHIFT) & LLWU_PE3_WUPE11_MASK))  /*!< Wakeup Pin Enable for LLWU_P11 */
#define LLWU_PE3_WUPE10_SHIFT       4                                                                            /*!< Wakeup Pin Enable for LLWU_P10 (shift) */
#define LLWU_PE3_WUPE10_MASK        ((uint8_t)((uint8_t)0x03 << LLWU_PE3_WUPE10_SHIFT))                          /*!< Wakeup Pin Enable for LLWU_P10 (mask) */
#define LLWU_PE3_WUPE10(x)          ((uint8_t)(((uint8_t)(x) << LLWU_PE3_WUPE10_SHIFT) & LLWU_PE3_WUPE10_MASK))  /*!< Wakeup Pin Enable for LLWU_P10 */
#define LLWU_PE3_WUPE13_SHIFT        2                                                                          /*!< Wakeup Pin Enable for LLWU_P9 (shift) */
#define LLWU_PE3_WUPE13_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE3_WUPE13_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P9 (mask) */
#define LLWU_PE3_WUPE13(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE3_WUPE13_SHIFT) & LLWU_PE3_WUPE13_MASK))  /*!< Wakeup Pin Enable for LLWU_P9 */
#define LLWU_PE3_WUPE8_SHIFT        0                                                                          /*!< Wakeup Pin Enable for LLWU_P8 (shift) */
#define LLWU_PE3_WUPE8_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE3_WUPE8_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P8 (mask) */
#define LLWU_PE3_WUPE8(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE3_WUPE8_SHIFT) & LLWU_PE3_WUPE8_MASK))  /*!< Wakeup Pin Enable for LLWU_P8 */

/**********  Bits definition for LLWU_PE4 register  *************/
#define LLWU_PE4_WUPE15_SHIFT       6                                                                            /*!< Wakeup Pin Enable for LLWU_P15 (shift) */
#define LLWU_PE4_WUPE15_MASK        ((uint8_t)((uint8_t)0x03 << LLWU_PE4_WUPE15_SHIFT))                          /*!< Wakeup Pin Enable for LLWU_P15 (mask) */
#define LLWU_PE4_WUPE15(x)          ((uint8_t)(((uint8_t)(x) << LLWU_PE4_WUPE15_SHIFT) & LLWU_PE4_WUPE15_MASK))  /*!< Wakeup Pin Enable for LLWU_P15 */
#define LLWU_PE4_WUPE14_SHIFT       4                                                                            /*!< Wakeup Pin Enable for LLWU_P14 (shift) */
#define LLWU_PE4_WUPE14_MASK        ((uint8_t)((uint8_t)0x03 << LLWU_PE4_WUPE14_SHIFT))                          /*!< Wakeup Pin Enable for LLWU_P14 (mask) */
#define LLWU_PE4_WUPE14(x)          ((uint8_t)(((uint8_t)(x) << LLWU_PE4_WUPE14_SHIFT) & LLWU_PE4_WUPE14_MASK))  /*!< Wakeup Pin Enable for LLWU_P14 */
#define LLWU_PE4_WUPE13_SHIFT       2                                                                            /*!< Wakeup Pin Enable for LLWU_P13 (shift) */
#define LLWU_PE4_WUPE13_MASK        ((uint8_t)((uint8_t)0x03 << LLWU_PE4_WUPE13_SHIFT))                          /*!< Wakeup Pin Enable for LLWU_P13 (mask) */
#define LLWU_PE4_WUPE13(x)          ((uint8_t)(((uint8_t)(x) << LLWU_PE4_WUPE13_SHIFT) & LLWU_PE4_WUPE13_MASK))  /*!< Wakeup Pin Enable for LLWU_P13 */
#define LLWU_PE4_WUPE12_SHIFT       0                                                                            /*!< Wakeup Pin Enable for LLWU_P12 (shift) */
#define LLWU_PE4_WUPE12_MASK        ((uint8_t)((uint8_t)0x03 << LLWU_PE4_WUPE12_SHIFT))                          /*!< Wakeup Pin Enable for LLWU_P12 (mask) */
#define LLWU_PE4_WUPE12(x)          ((uint8_t)(((uint8_t)(x) << LLWU_PE4_WUPE12_SHIFT) & LLWU_PE4_WUPE12_MASK))  /*!< Wakeup Pin Enable for LLWU_P12 */

/**********  Bits definition for LLWU_ME register  *************/
#define LLWU_ME_WUME7               ((uint8_t)((uint8_t)1 << 7))    /*!< Wakeup Module Enable for Module 7 */
#define LLWU_ME_WUME6               ((uint8_t)((uint8_t)1 << 6))    /*!< Wakeup Module Enable for Module 6 */
#define LLWU_ME_WUME5               ((uint8_t)((uint8_t)1 << 5))    /*!< Wakeup Module Enable for Module 5 */
#define LLWU_ME_WUME4               ((uint8_t)((uint8_t)1 << 4))    /*!< Wakeup Module Enable for Module 4 */
#define LLWU_ME_WUME3               ((uint8_t)((uint8_t)1 << 3))    /*!< Wakeup Module Enable for Module 3 */
#define LLWU_ME_WUME2               ((uint8_t)((uint8_t)1 << 2))    /*!< Wakeup Module Enable for Module 2 */
#define LLWU_ME_WUME1               ((uint8_t)((uint8_t)1 << 1))    /*!< Wakeup Module Enable for Module 1 */
#define LLWU_ME_WUME0               ((uint8_t)((uint8_t)1 << 0))    /*!< Wakeup Module Enable for Module 0 */

/**********  Bits definition for LLWU_F1 register  *************/
#define LLWU_F1_WUF7                ((uint8_t)((uint8_t)1 << 7))    /*!< Wakeup Flag for LLWU_P7 */
#define LLWU_F1_WUF6                ((uint8_t)((uint8_t)1 << 6))    /*!< Wakeup Flag for LLWU_P6 */
#define LLWU_F1_WUF5                ((uint8_t)((uint8_t)1 << 5))    /*!< Wakeup Flag for LLWU_P5 */
#define LLWU_F1_WUF4                ((uint8_t)((uint8_t)1 << 4))    /*!< Wakeup Flag for LLWU_P4 */
#define LLWU_F1_WUF3                ((uint8_t)((uint8_t)1 << 3))    /*!< Wakeup Flag for LLWU_P3 */
#define LLWU_F1_WUF2                ((uint8_t)((uint8_t)1 << 2))    /*!< Wakeup Flag for LLWU_P2 */
#define LLWU_F1_WUF1                ((uint8_t)((uint8_t)1 << 1))    /*!< Wakeup Flag for LLWU_P1 */
#define LLWU_F1_WUF0                ((uint8_t)((uint8_t)1 << 0))    /*!< Wakeup Flag for LLWU_P0 */

/**********  Bits definition for LLWU_F2 register  *************/
#define LLWU_F2_WUF15               ((uint8_t)((uint8_t)1 << 7))    /*!< Wakeup Flag for LLWU_P15 */
#define LLWU_F2_WUF14               ((uint8_t)((uint8_t)1 << 6))    /*!< Wakeup Flag for LLWU_P14 */
#define LLWU_F2_WUF13               ((uint8_t)((uint8_t)1 << 5))    /*!< Wakeup Flag for LLWU_P13 */
#define LLWU_F2_WUF12               ((uint8_t)((uint8_t)1 << 4))    /*!< Wakeup Flag for LLWU_P12 */
#define LLWU_F2_WUF11               ((uint8_t)((uint8_t)1 << 3))    /*!< Wakeup Flag for LLWU_P11 */
#define LLWU_F2_WUF10               ((uint8_t)((uint8_t)1 << 2))    /*!< Wakeup Flag for LLWU_P10 */
#define LLWU_F2_WUF9                ((uint8_t)((uint8_t)1 << 1))    /*!< Wakeup Flag for LLWU_P9 */
#define LLWU_F2_WUF8                ((uint8_t)((uint8_t)1 << 0))    /*!< Wakeup Flag for LLWU_P8 */

/**********  Bits definition for LLWU_F3 register  *************/
#define LLWU_F3_MWUF7               ((uint8_t)((uint8_t)1 << 7))    /*!< Wakeup Flag for Module 7 */
#define LLWU_F3_MWUF6               ((uint8_t)((uint8_t)1 << 6))    /*!< Wakeup Flag for Module 6 */
#define LLWU_F3_MWUF5               ((uint8_t)((uint8_t)1 << 5))    /*!< Wakeup Flag for Module 5 */
#define LLWU_F3_MWUF4               ((uint8_t)((uint8_t)1 << 4))    /*!< Wakeup Flag for Module 4 */
#define LLWU_F3_MWUF3               ((uint8_t)((uint8_t)1 << 3))    /*!< Wakeup Flag for Module 3 */
#define LLWU_F3_MWUF2               ((uint8_t)((uint8_t)1 << 2))    /*!< Wakeup Flag for Module 2 */
#define LLWU_F3_MWUF1               ((uint8_t)((uint8_t)1 << 1))    /*!< Wakeup Flag for Module 1 */
#define LLWU_F3_MWUF0               ((uint8_t)((uint8_t)1 << 0))    /*!< Wakeup Flag for Module 0 */

/**********  Bits definition for LLWU_FILT1 register  *************/
#define LLWU_FILT1_FILTF            ((uint8_t)((uint8_t)1 << 7))    /*!< Filter Detect Flag */
#define LLWU_FILT1_FILTE_SHIFT      5                                                                              /*!< Digital Filter on External Pin (shift) */
#define LLWU_FILT1_FILTE_MASK       ((uint8_t)((uint8_t)0x03 << LLWU_FILT1_FILTE_SHIFT))                           /*!< Digital Filter on External Pin (mask) */
#define LLWU_FILT1_FILTE(x)         ((uint8_t)(((uint8_t)(x) << LLWU_FILT1_FILTE_SHIFT) & LLWU_FILT1_FILTE_MASK))  /*!< Digital Filter on External Pin */
#define LLWU_FILT1_FILTE_DISABLED   LLWU_FILT1_FILTE(0)  /*!< Filter disabled */
#define LLWU_FILT1_FILTE_POSEDGE    LLWU_FILT1_FILTE(1)  /*!< Filter posedge detect enabled */
#define LLWU_FILT1_FILTE_NEGEDGE    LLWU_FILT1_FILTE(2)  /*!< Filter negedge detect enabled */
#define LLWU_FILT1_FILTE_ANYEDGE    LLWU_FILT1_FILTE(3)  /*!< Filter any edge detect enabled */
#define LLWU_FILT1_FILTSEL_SHIFT    0                                                                                  /*!< Filter Pin Select (LLWU_P0 ... LLWU_P15) (shift) */
#define LLWU_FILT1_FILTSEL_MASK     ((uint8_t)((uint8_t)0x0F << LLWU_FILT1_FILTSEL_SHIFT))                             /*!< Filter Pin Select (LLWU_P0 ... LLWU_P15) (mask) */
#define LLWU_FILT1_FILTSEL(x)       ((uint8_t)(((uint8_t)(x) << LLWU_FILT1_FILTSEL_SHIFT) & LLWU_FILT1_FILTSEL_MASK))  /*!< Filter Pin Select (LLWU_P0 ... LLWU_P15) */

/**********  Bits definition for LLWU_FILT2 register  *************/
#define LLWU_FILT2_FILTF            ((uint8_t)((uint8_t)1 << 7))    /*!< Filter Detect Flag */
#define LLWU_FILT2_FILTE_SHIFT      5                                                                              /*!< Digital Filter on External Pin (shift) */
#define LLWU_FILT2_FILTE_MASK       ((uint8_t)((uint8_t)0x03 << LLWU_FILT2_FILTE_SHIFT))                           /*!< Digital Filter on External Pin (mask) */
#define LLWU_FILT2_FILTE(x)         ((uint8_t)(((uint8_t)(x) << LLWU_FILT2_FILTE_SHIFT) & LLWU_FILT2_FILTE_MASK))  /*!< Digital Filter on External Pin */
#define LLWU_FILT2_FILTE_DISABLED   LLWU_FILT2_FILTE(0)  /*!< Filter disabled */
#define LLWU_FILT2_FILTE_POSEDGE    LLWU_FILT2_FILTE(1)  /*!< Filter posedge detect enabled */
#define LLWU_FILT2_FILTE_NEGEDGE    LLWU_FILT2_FILTE(2)  /*!< Filter negedge detect enabled */
#define LLWU_FILT2_FILTE_ANYEDGE    LLWU_FILT2_FILTE(3)  /*!< Filter any edge detect enabled */
#define LLWU_FILT2_FILTSEL_SHIFT    0                                                                                  /*!< Filter Pin Select (LLWU_P0 ... LLWU_P15) (shift) */
#define LLWU_FILT2_FILTSEL_MASK     ((uint8_t)((uint8_t)0x0F << LLWU_FILT2_FILTSEL_SHIFT))                             /*!< Filter Pin Select (LLWU_P0 ... LLWU_P15) (mask) */
#define LLWU_FILT2_FILTSEL(x)       ((uint8_t)(((uint8_t)(x) << LLWU_FILT2_FILTSEL_SHIFT) & LLWU_FILT2_FILTSEL_MASK))  /*!< Filter Pin Select (LLWU_P0 ... LLWU_P15) */

/****************************************************************/
/*                                                              */
/*           Port Control and interrupts (PORT)                 */
/*                                                              */
/****************************************************************/
/********  Bits definition for PORTx_PCRn register  *************/
#define PORTx_PCRn_ISR               ((uint32_t)0x01000000)    /*!< Interrupt Status Flag */
#define PORTx_PCRn_IRQC_SHIFT        16
#define PORTx_PCRn_IRQC_MASK         ((uint32_t)0x000F0000)    /*!< Interrupt Configuration */
#define PORTx_PCRn_IRQC(x)           ((uint32_t)(((uint32_t)(x) << PORTx_PCRn_IRQC_SHIFT) & PORTx_PCRn_IRQC_MASK))
#define PORTx_PCRn_MUX_SHIFT         8                         /*!< Pin Mux Control (shift) */
#define PORTx_PCRn_MUX_MASK          ((uint32_t)0x00000700)    /*!< Pin Mux Control (mask) */
#define PORTx_PCRn_MUX(x)            ((uint32_t)(((uint32_t)(x) << PORTx_PCRn_MUX_SHIFT) & PORTx_PCRn_MUX_MASK))  /*!< Pin Mux Control */
#define PORTx_PCRn_DSE               ((uint32_t)0x00000040)    /*!< Drive Strength Enable */
#define PORTx_PCRn_PFE               ((uint32_t)0x00000010)    /*!< Passive Filter Enable */
#define PORTx_PCRn_SRE               ((uint32_t)0x00000004)    /*!< Slew Rate Enable */
#define PORTx_PCRn_PE                ((uint32_t)0x00000002)    /*!< Pull Enable */
#define PORTx_PCRn_PS                ((uint32_t)0x00000001)    /*!< Pull Select */

/****************************************************************/
/*                                                              */
/*                   Oscillator (OSC)                           */
/*                                                              */
/****************************************************************/
/***********  Bits definition for OSC_CR register  **************/
#define OSC_CR_ERCLKEN               ((uint8_t)0x80)    /*!< External Reference Enable */
#define OSC_CR_EREFSTEN              ((uint8_t)0x20)    /*!< External Reference Stop Enable */
#define OSC_CR_SC2P                  ((uint8_t)0x08)    /*!< Oscillator 2pF Capacitor Load Configure */
#define OSC_CR_SC4P                  ((uint8_t)0x04)    /*!< Oscillator 4pF Capacitor Load Configure */
#define OSC_CR_SC8P                  ((uint8_t)0x02)    /*!< Oscillator 8pF Capacitor Load Configure */
#define OSC_CR_SC16P                 ((uint8_t)0x01)    /*!< Oscillator 16pF Capacitor Load Configure */

/****************************************************************/
/*                                                              */
/*                 Direct Memory Access (DMA)                   */
/*                                                              */
/****************************************************************/
/***********  Bits definition for DMA_BCRn register  ************/
#define DMA_DSR_BCRn_CE          ((uint32_t)((uint32_t)1 << 30))    /*!< Configuration Error */
#define DMA_DSR_BCRn_BES         ((uint32_t)((uint32_t)1 << 29))    /*!< Bus Error on Source */
#define DMA_DSR_BCRn_BED         ((uint32_t)((uint32_t)1 << 28))    /*!< Bus Error on Destination */
#define DMA_DSR_BCRn_REQ         ((uint32_t)((uint32_t)1 << 26))    /*!< Request */
#define DMA_DSR_BCRn_BSY         ((uint32_t)((uint32_t)1 << 25))    /*!< Busy */
#define DMA_DSR_BCRn_DONE        ((uint32_t)((uint32_t)1 << 24))    /*!< Transactions done */
#define DMA_DSR_BCRn_BCR_SHIFT   0                                                                                /*!< Bytes yet to be transferred for block (shift) */
#define DMA_DSR_BCRn_BCR_MASK    ((uint32_t)((uint32_t)0x00FFFFFF << DMA_DSR_BCRn_BCR_SHIFT))                     /*!< Bytes yet to be transferred for block (mask) */
#define DMA_DSR_BCRn_BCR(x)      ((uint32_t)(((uint32_t)(x) << DMA_DSR_BCRn_BCR_SHIFT) & DMA_DSR_BCRn_BCR_MASK))  /*!< Bytes yet to be transferred for block */

/***********  Bits definition for DMA_DCRn register  ************/
#define DMA_DCRn_EINT            ((uint32_t)((uint32_t)1 << 31))         /*!< Enable interrupt on completion of transfer */
#define DMA_DCRn_ERQ             ((uint32_t)((uint32_t)1 << 30))         /*!< Enable peripheral request */
#define DMA_DCRn_CS              ((uint32_t)((uint32_t)1 << 29))         /*!< Cycle steal */
#define DMA_DCRn_AA              ((uint32_t)((uint32_t)1 << 28))         /*!< Auto-align */
#define DMA_DCRn_EADREQ          ((uint32_t)((uint32_t)1 << 23))         /*!< Enable asynchronous DMA requests */
#define DMA_DCRn_SINC            ((uint32_t)((uint32_t)1 << 22))        /*!< Source increment */
#define DMA_DCRn_SSIZE_SHIFT     20                                                               /*!< Source size (shift) */
#define DMA_DCRn_SSIZE_MASK      ((uint32_t)((uint32_t)0x03 << DMA_DCRn_SSIZE_SHIFT))                         /*!< Source size (mask) */
#define DMA_DCRn_SSIZE(x)        ((uint32_t)(((uint32_t)(x) << DMA_DCRn_SSIZE_SHIFT) & DMA_DCRn_SSIZE_MASK))  /*!< Source size */
#define DMA_DCRn_DINC            ((uint32_t)((uint32_t)1 << 19))                                              /*!< Destination increment */
#define DMA_DCRn_DSIZE_SHIFT     17                                                                           /*!< Destination size (shift) */
#define DMA_DCRn_DSIZE_MASK      ((uint32_t)((uint32_t)0x03 << DMA_DCRn_DSIZE_SHIFT))                         /*!< Destination size (mask) */
#define DMA_DCRn_DSIZE(x)        ((uint32_t)(((uint32_t)(x) << DMA_DCRn_DSIZE_SHIFT) & DMA_DCRn_DSIZE_MASK))  /*!< Destination size */
#define DMA_DCRn_START           ((uint32_t)((uint32_t)1 << 16))                                            /*!< Start transfer */
#define DMA_DCRn_SMOD_SHIFT      12                                                                         /*!< Source address modulo (shift) */
#define DMA_DCRn_SMOD_MASK       ((uint32_t)((uint32_t)0x0F << DMA_DCRn_SMOD_SHIFT))                        /*!< Source address modulo (mask) */
#define DMA_DCRn_SMOD(x)         ((uint32_t)(((uint32_t)(x) << DMA_DCRn_SMOD_SHIFT) & DMA_DCRn_SMOD_MASK))  /*!< Source address modulo */
#define DMA_DCRn_DMOD_SHIFT      8                                                                          /*!< Destination address modulo (shift) */
#define DMA_DCRn_DMOD_MASK       ((uint32_t)0x0F << DMA_DCRn_DMOD_SHIFT)                                    /*!< Destination address modulo (mask) */
#define DMA_DCRn_DMOD(x)         ((uint32_t)(((uint32_t)(x) << DMA_DCRn_DMOD_SHIFT) & DMA_DCRn_DMOD_MASK))  /*!< Destination address modulo */
#define DMA_DCRn_D_REQ           ((uint32_t)((uint32_t)1 <<  7))                                            /*!< Disable request */
#define DMA_DCRn_LINKCC_SHIFT    4                                                                              /*!< Link channel control (shift) */
#define DMA_DCRn_LINKCC_MASK     ((uint32_t)((uint32_t)0x03 << DMA_DCRn_LINKCC_SHIFT))                          /*!< Link channel control (mask) */
#define DMA_DCRn_LINKCC(x)       ((uint32_t)(((uint32_t)(x) << DMA_DCRn_LINKCC_SHIFT) & DMA_DCRn_LINKCC_MASK))  /*!< Link channel control */
#define DMA_DCRn_LCH1_SHIFT      2                                                                          /*!< Link channel 1 (shift) */
#define DMA_DCRn_LCH1_MASK       ((uint32_t)((uint32_t)0x03 << DMA_DCRn_LCH1_SHIFT))                        /*!< Link channel 1 (mask) */
#define DMA_DCRn_LCH1(x)         ((uint32_t)(((uint32_t)(x) << DMA_DCRn_LCH1_SHIFT) & DMA_DCRn_LCH1_MASK))  /*!< Link channel 1 */
#define DMA_DCRn_LCH2_SHIFT      0                                                                          /*!< Link channel 2 (shift) */
#define DMA_DCRn_LCH2_MASK       ((uint32_t)((uint32_t)0x03 << DMA_DCRn_LCH2_SHIFT))                        /*!< Link channel 2 (mask) */
#define DMA_DCRn_LCH2(x)         ((uint32_t)(((uint32_t)(x) << DMA_DCRn_LCH2_SHIFT) & DMA_DCRn_LCH2_MASK))  /*!< Link channel 2 */

/****************************************************************/
/*                                                              */
/*         Direct Memory Access Multiplexer (DMAMUX)            */
/*                                                              */
/****************************************************************/
/********  Bits definition for DMAMUX_CHCFGn register  **********/
#define DMAMUX_CHCFGn_ENBL           ((uint8_t)((uint8_t)1 << 7))  /*!< DMA Channel Enable */
#define DMAMUX_CHCFGn_TRIG           ((uint8_t)((uint8_t)1 << 6))  /*!< DMA Channel Trigger Enable */
#define DMAMUX_CHCFGn_SOURCE_SHIFT   0                                                                                      /*!< DMA Channel Source (Slot) (shift) */
#define DMAMUX_CHCFGn_SOURCE_MASK    ((uint8_t)((uint8_t)0x3F << DMAMUX_CHCFGn_SOURCE_SHIFT))                               /*!< DMA Channel Source (Slot) (mask) */
#define DMAMUX_CHCFGn_SOURCE(x)      ((uint8_t)(((uint8_t)(x) << DMAMUX_CHCFGn_SOURCE_SHIFT) & DMAMUX_CHCFGn_SOURCE_MASK))  /*!< DMA Channel Source (Slot) */

/****************************************************************/
/*                                                              */
/*              Analog-to-Digital Converter (ADC)               */
/*                                                              */
/****************************************************************/
/***********  Bits definition for ADCx_SC1n register  ***********/
#define ADCx_SC1n_COCO          ((uint32_t)((uint32_t)1 << 7))  /*!< Conversion Complete Flag */
#define ADCx_SC1n_AIEN          ((uint32_t)((uint32_t)1 << 6))  /*!< Interrupt Enable */
#define ADCx_SC1n_DIFF          ((uint32_t)((uint32_t)1 << 5))  /*!< Differential Mode Enable */
#define ADCx_SC1n_ADCH_SHIFT    0                                                                            /*!< Input channel select (shift) */
#define ADCx_SC1n_ADCH_MASK     ((uint32_t)((uint32_t)0x1F << ADCx_SC1n_ADCH_SHIFT))                         /*!< Input channel select (mask) */
#define ADCx_SC1n_ADCH(x)       ((uint32_t)(((uint32_t)(x) << ADCx_SC1n_ADCH_SHIFT) & ADCx_SC1n_ADCH_MASK))  /*!< Input channel select */

/***********  Bits definition for ADCx_CFG1 register  ***********/
#define ADCx_CFG1_ADLPC         ((uint32_t)((uint32_t)1 << 7))  /*!< Low-Power Configuration */
#define ADCx_CFG1_ADIV_SHIFT    5                                                                            /*!< Clock Divide Select (shift) */
#define ADCx_CFG1_ADIV_MASK     ((uint32_t)((uint32_t)0x03 << ADCx_CFG1_ADIV_SHIFT))                         /*!< Clock Divide Select (mask) */
#define ADCx_CFG1_ADIV(x)       ((uint32_t)(((uint32_t)(x) << ADCx_CFG1_ADIV_SHIFT) & ADCx_CFG1_ADIV_MASK))  /*!< Clock Divide Select */
#define ADCx_CFG1_ADLSMP        ((uint32_t)((uint32_t)1 << 4))  /*!< Sample time configuration */
#define ADCx_CFG1_MODE_SHIFT    2                                                                            /*!< Conversion mode (resolution) selection (shift) */
#define ADCx_CFG1_MODE_MASK     ((uint32_t)((uint32_t)0x03 << ADCx_CFG1_MODE_SHIFT))                         /*!< Conversion mode (resolution) selection (mask) */
#define ADCx_CFG1_MODE(x)       ((uint32_t)(((uint32_t)(x) << ADCx_CFG1_MODE_SHIFT) & ADCx_CFG1_MODE_MASK))  /*!< Conversion mode (resolution) selection */
#define ADCx_CFG1_ADICLK_SHIFT  0                                                                                /*!< Input Clock Select (shift) */
#define ADCx_CFG1_ADICLK_MASK   ((uint32_t)((uint32_t)0x03 << ADCx_CFG1_ADICLK_SHIFT))                           /*!< Input Clock Select (mask) */
#define ADCx_CFG1_ADICLK(x)     ((uint32_t)(((uint32_t)(x) << ADCx_CFG1_ADICLK_SHIFT) & ADCx_CFG1_ADICLK_MASK))  /*!< Input Clock Select */

/***********  Bits definition for ADCx_CFG2 register  ***********/
#define ADCx_CFG2_MUXSEL        ((uint32_t)((uint32_t)1 << 4))  /*!< ADC Mux Select */
#define ADCx_CFG2_ADACKEN       ((uint32_t)((uint32_t)1 << 3))  /*!< Asynchronous Clock Output Enable */
#define ADCx_CFG2_ADHSC         ((uint32_t)((uint32_t)1 << 2))  /*!< High-Speed Configuration */
#define ADCx_CFG2_ADLSTS_SHIFT  0                                                                                /*!< Long Sample Time Select (shift) */
#define ADCx_CFG2_ADLSTS_MASK   ((uint32_t)((uint32_t)0x03 << ADCx_CFG2_ADLSTS_SHIFT))                           /*!< Long Sample Time Select (mask) */
#define ADCx_CFG2_ADLSTS(x)     ((uint32_t)(((uint32_t)(x) << ADCx_CFG2_ADLSTS_SHIFT) & ADCx_CFG2_ADLSTS_MASK))  /*!< Long Sample Time Select */

/***********  Bits definition for ADCx_SC2 register  ***********/
#define ADCx_SC2_ADACT          ((uint32_t)((uint32_t)1 << 7))  /*!< Conversion Active */
#define ADCx_SC2_ADTRG          ((uint32_t)((uint32_t)1 << 6))  /*!< Conversion Trigger Select */
#define ADCx_SC2_ACFE           ((uint32_t)((uint32_t)1 << 5))  /*!< Compare Function Enable */
#define ADCx_SC2_ACFGT          ((uint32_t)((uint32_t)1 << 4))  /*!< Compare Function Greater Than Enable */
#define ADCx_SC2_ACREN          ((uint32_t)((uint32_t)1 << 3))  /*!< Compare Function Range Enable */
#define ADCx_SC2_DMAEN          ((uint32_t)((uint32_t)1 << 2))  /*!< DMA Enable */
#define ADCx_SC2_REFSEL_SHIFT   0                                                                              /*!< Voltage Reference Selection (shift) */
#define ADCx_SC2_REFSEL_MASK    ((uint32_t)((uint32_t)0x03 << ADCx_SC2_REFSEL_SHIFT))                          /*!< Voltage Reference Selection (mask) */
#define ADCx_SC2_REFSEL(x)      ((uint32_t)(((uint32_t)(x) << ADCx_SC2_REFSEL_SHIFT) & ADCx_SC2_REFSEL_MASK))  /*!< Voltage Reference Selection */

/***********  Bits definition for ADCx_SC3 register  ***********/
#define ADCx_SC3_CAL            ((uint32_t)((uint32_t)1 << 7))  /*!< Calibration */
#define ADCx_SC3_CALF           ((uint32_t)((uint32_t)1 << 6))  /*!< Calibration Failed Flag */
#define ADCx_SC3_ADCO           ((uint32_t)((uint32_t)1 << 3))  /*!< Continuous Conversion Enable */
#define ADCx_SC3_AVGE           ((uint32_t)((uint32_t)1 << 2))  /*!< Hardware Average Enable */
#define ADCx_SC3_AVGS_SHIFT     0                                                                          /*!< Hardware Average Select (shift) */
#define ADCx_SC3_AVGS_MASK      ((uint32_t)((uint32_t)0x03 << ADCx_SC3_AVGS_SHIFT))                        /*!< Hardware Average Select (mask) */
#define ADCx_SC3_AVGS(x)        ((uint32_t)(((uint32_t)(x) << ADCx_SC3_AVGS_SHIFT) & ADCx_SC3_AVGS_MASK))  /*!< Hardware Average Select */

/****************************************************************/
/*                                                              */
/*                   Low-Power Timer (LPTMR)                    */
/*                                                              */
/****************************************************************/
/**********  Bits definition for LPTMRx_CSR register  ***********/
#define LPTMRx_CSR_TCF              ((uint32_t)((uint32_t)1 << 7))  /*!< Timer Compare Flag */
#define LPTMRx_CSR_TIE              ((uint32_t)((uint32_t)1 << 6))  /*!< Timer Interrupt Enable */
#define LPTMRx_CSR_TPS_SHIFT        4                                                                            /*!< Timer Pin Select (shift) */
#define LPTMRx_CSR_TPS_MASK         ((uint32_t)((uint32_t)0x03 << LPTMRx_CSR_TPS_SHIFT))                         /*!< Timer Pin Select (mask) */
#define LPTMRx_CSR_TPS(x)           ((uint32_t)(((uint32_t)(x) << LPTMRx_CSR_TPS_SHIFT) & LPTMRx_CSR_TPS_MASK))  /*!< Timer Pin Select */
#define LPTMRx_CSR_TPP              ((uint32_t)((uint32_t)1 << 3))  /*!< Timer Pin Polarity */
#define LPTMRx_CSR_TFC              ((uint32_t)((uint32_t)1 << 2))  /*!< Timer Free-Running Counter */
#define LPTMRx_CSR_TMS              ((uint32_t)((uint32_t)1 << 1))  /*!< Timer Mode Select */
#define LPTMRx_CSR_TEN              ((uint32_t)((uint32_t)1 << 0))  /*!< Timer Enable */

/**********  Bits definition for LPTMRx_PSR register  ***********/
#define LPTMRx_PSR_PRESCALE_SHIFT   3                                                                                      /*!< Prescale Value (shift) */
#define LPTMRx_PSR_PRESCALE_MASK    ((uint32_t)((uint32_t)0x0F << LPTMRx_PSR_PRESCALE_SHIFT))                              /*!< Prescale Value (mask) */
#define LPTMRx_PSR_PRESCALE(x)      ((uint32_t)(((uint32_t)(x) << LPTMRx_PSR_PRESCALE_SHIFT) & LPTMRx_PSR_PRESCALE_MASK))  /*!< Prescale Value */
#define LPTMRx_PSR_PBYP             ((uint32_t)((uint32_t)1 << 2))  /*!< Prescaler Bypass */
#define LPTMRx_PSR_PCS_SHIFT        0                                                                            /*!< Prescaler Clock Select (shift) */
#define LPTMRx_PSR_PCS_MASK         ((uint32_t)((uint32_t)0x03 << LPTMRx_PSR_PCS_SHIFT))                         /*!< Prescaler Clock Select (mask) */
#define LPTMRx_PSR_PCS(x)           ((uint32_t)(((uint32_t)(x) << LPTMRx_PSR_PCS_SHIFT) & LPTMRx_PSR_PCS_MASK))  /*!< Prescaler Clock Select */

/**********  Bits definition for LPTMRx_CMR register  ***********/
#define LPTMRx_CMR_COMPARE_SHIFT    0                                                                                    /*!< Compare Value (shift) */
#define LPTMRx_CMR_COMPARE_MASK     ((uint32_t)((uint32_t)0xFFFF << LPTMRx_CMR_COMPARE_SHIFT))                           /*!< Compare Value (mask) */
#define LPTMRx_CMR_COMPARE(x)       ((uint32_t)(((uint32_t)(x) << LPTMRx_CMR_COMPARE_SHIFT) & LPTMRx_CMR_COMPARE_MASK))  /*!< Compare Value */

/**********  Bits definition for LPTMRx_CNR register  ***********/
#define LPTMRx_CNR_COUNTER_SHIFT    0                                                                                    /*!< Counter Value (shift) */
#define LPTMRx_CNR_COUNTER_MASK     ((uint32_t)((uint32_t)0xFFFF << LPTMRx_CNR_COUNTER_SHIFT))                           /*!< Counter Value (mask) */
#define LPTMRx_CNR_COUNTER(x)       ((uint32_t)(((uint32_t)(x) << LPTMRx_CNR_COUNTER_SHIFT) & LPTMRx_CNR_COUNTER_MASK))  /*!< Counter Value */

/****************************************************************/
/*                                                              */
/*                  Touch Sensing Input (TSI)                   */
/*                                                              */
/****************************************************************/
/**********  Bits definition for TSIx_GENCS register  ***********/
#define TSIx_GENCS_OUTRGF           ((uint32_t)((uint32_t)1 << 31))  /*!< Out of Range Flag */
#define TSIx_GENCS_ESOR             ((uint32_t)((uint32_t)1 << 28))  /*!< End-of-scan/Out-of-Range Interrupt Selection */
#define TSIx_GENCS_MODE_SHIFT       24                                                                                 /*!< TSI analog modes setup and status bits (shift) */
#define TSIx_GENCS_MODE_MASK        ((uint32_t)((uint32_t)0x0F << TSIx_GENCS_MODE_SHIFT))                            /*!< TSI analog modes setup and status bits (mask) */
#define TSIx_GENCS_MODE(x)          ((uint32_t)(((uint32_t)(x) << TSIx_GENCS_MODE_SHIFT) & TSIx_GENCS_MODE_MASK))  /*!< TSI analog modes setup and status bits */
#define TSIx_GENCS_REFCHRG_SHIFT    21                                                                                       /*!< Reference oscillator charge/discharge current (shift) */
#define TSIx_GENCS_REFCHRG_MASK     ((uint32_t)((uint32_t)0x07 << TSIx_GENCS_REFCHRG_SHIFT))                               /*!< Reference oscillator charge/discharge current (mask) */
#define TSIx_GENCS_REFCHRG(x)       ((uint32_t)(((uint32_t)(x) << TSIx_GENCS_REFCHRG_SHIFT) & TSIx_GENCS_REFCHRG_MASK))  /*!< Reference oscillator charge/discharge current */
#define TSIx_GENCS_DVOLT_SHIFT      19                                                                                   /*!< Oscillator voltage rails (shift) */
#define TSIx_GENCS_DVOLT_MASK       ((uint32_t)((uint32_t)0x03 << TSIx_GENCS_DVOLT_SHIFT))                             /*!< Oscillator voltage rails (mask) */
#define TSIx_GENCS_DVOLT(x)         ((uint32_t)(((uint32_t)(x) << TSIx_GENCS_DVOLT_SHIFT) & TSIx_GENCS_DVOLT_MASK))  /*!< Oscillator voltage rails */
#define TSIx_GENCS_EXTCHRG_SHIFT    16                                                                                       /*!< Electrode oscillator charge/discharge current (shift) */
#define TSIx_GENCS_EXTCHRG_MASK     ((uint32_t)((uint32_t)0x07 << TSIx_GENCS_EXTCHRG_SHIFT))                               /*!< Electrode oscillator charge/discharge current (mask) */
#define TSIx_GENCS_EXTCHRG(x)       ((uint32_t)(((uint32_t)(x) << TSIx_GENCS_EXTCHRG_SHIFT) & TSIx_GENCS_EXTCHRG_MASK))  /*!< Electrode oscillator charge/discharge current */
#define TSIx_GENCS_PS_SHIFT         13                                                                             /*!< Electrode oscillator prescaler (shift) */
#define TSIx_GENCS_PS_MASK          ((uint32_t)((uint32_t)0x07 << TSIx_GENCS_PS_SHIFT))                          /*!< Electrode oscillator prescaler (mask) */
#define TSIx_GENCS_PS(x)            ((uint32_t)(((uint32_t)(x) << TSIx_GENCS_PS_SHIFT) & TSIx_GENCS_PS_MASK))  /*!< Electrode oscillator prescaler */
#define TSIx_GENCS_NSCN_SHIFT       8                                                                                  /*!< Number of scans per electrode minus 1 (shift) */
#define TSIx_GENCS_NSCN_MASK        ((uint32_t)((uint32_t)0x1F << TSIx_GENCS_NSCN_SHIFT))                            /*!< Number of scans per electrode minus 1 (mask) */
#define TSIx_GENCS_NSCN(x)          ((uint32_t)(((uint32_t)(x) << TSIx_GENCS_NSCN_SHIFT) & TSIx_GENCS_NSCN_MASK))  /*!< Number of scans per electrode minus 1 */
#define TSIx_GENCS_TSIEN            ((uint32_t)((uint32_t)1 << 7))  /*!< TSI Module Enable */
#define TSIx_GENCS_TSIIEN           ((uint32_t)((uint32_t)1 << 6))  /*!< TSI Interrupt Enable */
#define TSIx_GENCS_STPE             ((uint32_t)((uint32_t)1 << 5))  /*!< TSI STOP Enable */
#define TSIx_GENCS_STM              ((uint32_t)((uint32_t)1 << 4))  /*!< Scan Trigger Mode (0=software; 1=hardware) */
#define TSIx_GENCS_SCNIP            ((uint32_t)((uint32_t)1 << 3))  /*!< Scan in Progress Status */
#define TSIx_GENCS_EOSF             ((uint32_t)((uint32_t)1 << 2))  /*!< End of Scan Flag */
#define TSIx_GENCS_CURSW            ((uint32_t)((uint32_t)1 << 1))  /*!< Swap electrode and reference current sources */

/**********  Bits definition for TSIx_DATA register  ************/
#define TSIx_DATA_TSICH_SHIFT       28                                                                             /*!< Specify channel to be measured (shift) */
#define TSIx_DATA_TSICH_MASK        ((uint32_t)((uint32_t)0x0F << TSIx_DATA_TSICH_SHIFT))                          /*!< Specify channel to be measured (mask) */
#define TSIx_DATA_TSICH(x)          ((uint32_t)(((uint32_t)(x) << TSIx_DATA_TSICH_SHIFT) & TSIx_DATA_TSICH_MASK))  /*!< Specify channel to be measured */
#define TSIx_DATA_DMAEN             ((uint32_t)((uint32_t)1 << 23))  /*!< DMA Transfer Enabled */
#define TSIx_DATA_SWTS              ((uint32_t)((uint32_t)1 << 22))  /*!< Software Trigger Start */
#define TSIx_DATA_TSICNT_SHIFT      0                                                                                /*!< TSI Conversion Counter Value (shift) */
#define TSIx_DATA_TSICNT_MASK       ((uint32_t)((uint32_t)0xFFFF << TSIx_DATA_TSICNT_SHIFT))                         /*!< TSI Conversion Counter Value (mask) */
#define TSIx_DATA_TSICNT(x)         ((uint32_t)(((uint32_t)(x) << TSIx_DATA_TSICNT_SHIFT) & TSIx_DATA_TSICNT_MASK))  /*!< TSI Conversion Counter Value */

/**********  Bits definition for TSIx_TSHD register  ************/
#define TSIx_TSHD_THRESH_SHIFT      16                                                                               /*!< TSI Wakeup Channel High-Threshold (shift) */
#define TSIx_TSHD_THRESH_MASK       ((uint32_t)((uint32_t)0xFFFF << TSIx_TSHD_THRESH_SHIFT))                         /*!< TSI Wakeup Channel High-Threshold (mask) */
#define TSIx_TSHD_THRESH(x)         ((uint32_t)(((uint32_t)(x) << TSIx_TSHD_THRESH_SHIFT) & TSIx_TSHD_THRESH_MASK))  /*!< TSI Wakeup Channel High-Threshold */
#define TSIx_TSHD_THRESL_SHIFT      0                                                                                /*!< TSI Wakeup Channel Low-Threshold (shift) */
#define TSIx_TSHD_THRESL_MASK       ((uint32_t)((uint32_t)0xFFFF << TSIx_TSHD_THRESL_SHIFT))                         /*!< TSI Wakeup Channel Low-Threshold (mask) */
#define TSIx_TSHD_THRESL(x)         ((uint32_t)(((uint32_t)(x) << TSIx_TSHD_THRESL_SHIFT) & TSIx_TSHD_THRESL_MASK))  /*!< TSI Wakeup Channel Low-Threshold */

/****************************************************************/
/*                                                              */
/*             Multipurpose Clock Generator (MCG)               */
/*                                                              */
/****************************************************************/
/***********  Bits definition for MCG_C1 register  **************/
#define MCG_C1_CLKS_SHIFT           6                                                           /*!< Clock source select (shift) */
#define MCG_C1_CLKS_MASK            ((uint8_t)((uint8_t)0x03 << MCG_C1_CLKS_SHIFT))             /*!< Clock source select (mask) */
#define MCG_C1_CLKS(x)              ((uint8_t)(((uint8_t)(x) << MCG_C1_CLKS_SHIFT) & MCG_C1_CLKS_MASK))  /*!< Clock source select */
#define MCG_C1_CLKS_FLLPLL          MCG_C1_CLKS(0)  /*!< Select output of FLL or PLL, depending on PLLS control bit */
#define MCG_C1_CLKS_IRCLK           MCG_C1_CLKS(1)  /*!< Select internal reference clock */
#define MCG_C1_CLKS_ERCLK           MCG_C1_CLKS(2)  /*!< Select external reference clock */
#define MCG_C1_FRDIV_SHIFT          3                                                           /*!< FLL External Reference Divider (shift) */
#define MCG_C1_FRDIV_MASK           ((uint8_t)((uint8_t)0x07 << MCG_C1_FRDIV_SHIFT))            /*!< FLL External Reference Divider (mask) */
#define MCG_C1_FRDIV(x)             ((uint8_t)(((uint8_t)(x) << MCG_C1_FRDIV_SHIFT) & MCG_C1_FRDIV_MASK))  /*!< FLL External Reference Divider */
#define MCG_C1_IREFS                ((uint8_t)((uint8_t)1 << 2))                                /*!< Internal Reference Select (0=ERCLK; 1=slow IRCLK) */
#define MCG_C1_IRCLKEN              ((uint8_t)((uint8_t)1 << 1))                                /*!< Internal Reference Clock Enable */
#define MCG_C1_IREFSTEN             ((uint8_t)((uint8_t)1 << 0))                                /*!< Internal Reference Stop Enable */

/***********  Bits definition for MCG_C2 register  **************/
#define MCG_C2_LOCRE0               ((uint8_t)((uint8_t)1 << 7))                                /*!< Loss of Clock Reset Enable */
#define MCG_C2_RANGE0_SHIFT         4                                                           /*!< Frequency Range Select (shift) */
#define MCG_C2_RANGE0_MASK          ((uint8_t)((uint8_t)0x03 << MCG_C2_RANGE0_SHIFT))           /*!< Frequency Range Select (mask) */
#define MCG_C2_RANGE0(x)            ((uint8_t)(((uint8_t)(x) << MCG_C2_RANGE0_SHIFT) & MCG_C2_RANGE0_MASK))  /*!< Frequency Range Select */
#define MCG_C2_HGO0                 ((uint8_t)((uint8_t)1 << 3))                                /*!< High Gain Oscillator Select (0=low power; 1=high gain) */
#define MCG_C2_EREFS0               ((uint8_t)((uint8_t)1 << 2))                                /*!< External Reference Select (0=clock; 1=oscillator) */
#define MCG_C2_LP                   ((uint8_t)((uint8_t)1 << 1))                                /*!< Low Power Select (1=FLL/PLL disabled in bypass modes) */
#define MCG_C2_IRCS                 ((uint8_t)((uint8_t)1 << 0))                                /*!< Internal Reference Clock Select (0=slow; 1=fast) */

/***********  Bits definition for MCG_C3 register  **************/
#define MCG_C3_SCTRIM_SHIFT         0                                                           /*!< Slow Internal Reference Clock Trim Setting (shift) */
#define MCG_C3_SCTRIM_MASK          ((uint8_t)((uint8_t)0xFF << MCG_C3_SCTRIM_SHIFT))           /*!< Slow Internal Reference Clock Trim Setting (mask) */
#define MCG_C3_SCTRIM(x)            ((uint8_t)(((uint8_t)(x) << MCG_C3_SCTRIM_SHIFT) & MCG_C3_SCTRIM_MASK))  /*!< Slow Internal Reference Clock Trim Setting */

/***********  Bits definition for MCG_C4 register  **************/
#define MCG_C4_DMX32                ((uint8_t)((uint8_t)1 << 7))                                /*!< DCO Maximum Frequency with 32.768 kHz Reference */
#define MCG_C4_DRST_DRS_SHIFT       5                                                           /*!< DCO Range Select (shift) */
#define MCG_C4_DRST_DRS_MASK        ((uint8_t)((uint8_t)0x03 << MCG_C4_DRST_DRS_SHIFT))         /*!< DCO Range Select (mask) */
#define MCG_C4_DRST_DRS(x)          ((uint8_t)(((uint8_t)(x) << MCG_C4_DRST_DRS_SHIFT) & MCG_C4_DRST_DRS_MASK))  /*!< DCO Range Select */
#define MCG_C4_FCTRIM_SHIFT         1                                                           /*!< Fast Internal Reference Clock Trim Setting (shift) */
#define MCG_C4_FCTRIM_MASK          ((uint8_t)((uint8_t)0x0F << MCG_C4_FCTRIM_SHIFT))           /*!< Fast Internal Reference Clock Trim Setting (mask) */
#define MCG_C4_FCTRIM(x)            ((uint8_t)(((uint8_t)(x) << MCG_C4_FCTRIM_SHIFT) & MCG_C4_FCTRIM_MASK))  /*!< Fast Internal Reference Clock Trim Setting */
#define MCG_C4_SCFTRIM              ((uint8_t)((uint8_t)1 << 0))                                /*!< Slow Internal Reference Clock Fine Trim */

/***********  Bits definition for MCG_C5 register  **************/
#define MCG_C5_PLLCLKEN0            ((uint8_t)((uint8_t)1 << 6))                                /*!< PLL Clock Enable */
#define MCG_C5_PLLSTEN0             ((uint8_t)((uint8_t)1 << 5))                                /*!< PLL Stop Enable */
#define MCG_C5_PRDIV0_SHIFT         0                                                           /*!< PLL External Reference Divider (shift) */
#define MCG_C5_PRDIV0_MASK          ((uint8_t)((uint8_t)0x1F << MCG_C5_PRDIV0_SHIFT))           /*!< PLL External Reference Divider (mask) */
#define MCG_C5_PRDIV0(x)            ((uint8_t)(((uint8_t)(x) << MCG_C5_PRDIV0_SHIFT) & MCG_C5_PRDIV0_MASK))  /*!< PLL External Reference Divider */

/***********  Bits definition for MCG_C6 register  **************/
#define MCG_C6_LOLIE0               ((uint8_t)((uint8_t)1 << 7))                                /*!< Loss of Lock Interrupt Enable */
#define MCG_C6_PLLS                 ((uint8_t)((uint8_t)1 << 6))                                /*!< PLL Select */
#define MCG_C6_CME0                 ((uint8_t)((uint8_t)1 << 5))                                /*!< Clock Monitor Enable */
#define MCG_C6_VDIV0_SHIFT          0                                                           /*!< VCO 0 Divider (shift) */
#define MCG_C6_VDIV0_MASK           ((uint8_t)((uint8_t)0x1F << MCG_C6_VDIV0_SHIFT))            /*!< VCO 0 Divider (mask) */
#define MCG_C6_VDIV0(x)             ((uint8_t)(((uint8_t)(x) << MCG_C6_VDIV0_SHIFT) & MCG_C6_VDIV0_MASK))  /*!< VCO 0 Divider */

/************  Bits definition for MCG_S register  **************/
#define MCG_S_LOLS                  ((uint8_t)((uint8_t)1 << 7))                                /*!< Loss of Lock Status */
#define MCG_S_LOCK0                 ((uint8_t)((uint8_t)1 << 6))                                /*!< Lock Status */
#define MCG_S_PLLST                 ((uint8_t)((uint8_t)1 << 5))                                /*!< PLL Select Status */
#define MCG_S_IREFST                ((uint8_t)((uint8_t)1 << 4))                                /*!< Internal Reference Status */
#define MCG_S_CLKST_SHIFT           2                                                           /*!< Clock Mode Status (shift) */
#define MCG_S_CLKST_MASK            ((uint8_t)((uint8_t)0x03 << MCG_S_CLKST_SHIFT))             /*!< Clock Mode Status (mask) */
#define MCG_S_CLKST(x)              ((uint8_t)(((uint8_t)(x) << MCG_S_CLKST_SHIFT) & MCG_S_CLKST_MASK))  /*!< Clock Mode Status */
#define MCG_S_CLKST_FLL             MCG_S_CLKST(0)   /*!< Output of the FLL is selected */
#define MCG_S_CLKST_IRCLK           MCG_S_CLKST(1)   /*!< Internal reference clock is selected */
#define MCG_S_CLKST_ERCLK           MCG_S_CLKST(2)   /*!< External reference clock is selected */
#define MCG_S_CLKST_PLL             MCG_S_CLKST(3)   /*!< Output of the PLL is selected */
#define MCG_S_OSCINIT0              ((uint8_t)((uint8_t)1 << 1))                                /*!< OSC Initialization */
#define MCG_S_IRCST                 ((uint8_t)((uint8_t)1 << 0))                                /*!< Internal Reference Clock Status */

/************  Bits definition for MCG_SC register  **************/
#define MCG_SC_ATME                 ((uint8_t)((uint8_t)1 << 7))                                /*!< Automatic Trim Machine Enable */
#define MCG_SC_ATMS                 ((uint8_t)((uint8_t)1 << 6))                                /*!< Automatic Trim Machine Select */
#define MCG_SC_ATMF                 ((uint8_t)((uint8_t)1 << 5))                                /*!< Automatic Trim Machine Fail Flag */
#define MCG_SC_FLTPRSRV             ((uint8_t)((uint8_t)1 << 4)                                 /*!< FLL Filter Preserve Enable */
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
#define MCG_SC_LOCS0                ((uint8_t)((uint8_t)1 << 0)                                 /*!< OSC0 Loss of Clock Status */

/***********  Bits definition for MCG_ATCVH register  ************/
#define MCG_ATCVH_ATCVH_SHIFT       0                                                           /*!< MCG Auto Trim Compare Value High Register (shift) */
#define MCG_ATCVH_ATCVH_MASK        ((uint8_t)((uint8_t)0xFF << MCG_ATCVH_ATCVH_SHIFT))         /*!< MCG Auto Trim Compare Value High Register (mask) */
#define MCG_ATCVH_ATCVH(x)          ((uint8_t)(((uint8_t)(x) << MCG_ATCVH_ATCVH_SHIFT) & MCG_ATCVH_ATCVH_MASK))  /*!< MCG Auto Trim Compare Value High Register */

/***********  Bits definition for MCG_ATCVL register  ************/
#define MCG_ATCVL_ATCVL_SHIFT       0                                                           /*!< MCG Auto Trim Compare Value Low Register (shift) */
#define MCG_ATCVL_ATCVL_MASK        ((uint8_t)((uint8_t)0xFF << MCG_ATCVL_ATCVL_SHIFT))         /*!< MCG Auto Trim Compare Value Low Register (mask) */
#define MCG_ATCVL_ATCVL(x)          ((uint8_t)(((uint8_t)(x) << MCG_ATCVL_ATCVL_SHIFT) & MCG_ATCVL_ATCVL_MASK))  /*!< MCG Auto Trim Compare Value Low Register */

/************  Bits definition for MCG_C7 register  **************/
/* All MCG_C7 bits are reserved on the KL25Z. */

/************  Bits definition for MCG_C8 register  **************/
#define MCG_C8_LOLRE                ((uint8_t)((uint8_t)1 << 6))                                /*!< PLL Loss of Lock Reset Enable */

/************  Bits definition for MCG_C9 register  **************/
/* All MCG_C9 bits are reserved on the KL25Z. */

/************  Bits definition for MCG_C10 register  *************/
/* All MCG_C10 bits are reserved on the KL25Z. */


/****************************************************************/
/*                                                              */
/*             Serial Peripheral Interface (SPI)                */
/*                                                              */
/****************************************************************/
/***********  Bits definition for SPIx_C1 register  *************/
#define SPIx_C1_SPIE                 ((uint8_t)0x80)    /*!< SPI Interrupt Enable */
#define SPIx_C1_SPE                  ((uint8_t)0x40)    /*!< SPI System Enable */
#define SPIx_C1_SPTIE                ((uint8_t)0x20)    /*!< SPI Transmit Interrupt Enable */
#define SPIx_C1_MSTR                 ((uint8_t)0x10)    /*!< Master/Slave Mode Select */
#define SPIx_C1_CPOL                 ((uint8_t)0x08)    /*!< Clock Polarity */
#define SPIx_C1_CPHA                 ((uint8_t)0x04)    /*!< Clock Phase */
#define SPIx_C1_SSOE                 ((uint8_t)0x02)    /*!< Slave Select Output Enable */
#define SPIx_C1_LSBFE                ((uint8_t)0x01)    /*!< LSB First */

/***********  Bits definition for SPIx_C2 register  *************/
#define SPIx_C2_SPMIE                ((uint8_t)0x80)    /*!< SPI Match Interrupt Enable */
#define SPIx_C2_TXDMAE               ((uint8_t)0x20)    /*!< Transmit DMA Enable */
#define SPIx_C2_MODFEN               ((uint8_t)0x10)    /*!< Master Mode-Fault Function Enable */
#define SPIx_C2_BIDIROE              ((uint8_t)0x08)    /*!< Bidirectional Mode Output Enable */
#define SPIx_C2_RXDMAE               ((uint8_t)0x04)    /*!< Receive DMA Enable */
#define SPIx_C2_SPISWAI              ((uint8_t)0x02)    /*!< SPI Stop in Wait Mode */
#define SPIx_C2_SPC0                 ((uint8_t)0x01)    /*!< SPI Pin Control 0 */

/***********  Bits definition for SPIx_BR register  *************/
#define SPIx_BR_SPPR                 ((uint8_t)0x70)    /*!< SPI Baud rate Prescaler Divisor */
#define SPIx_BR_SPR                  ((uint8_t)0x0F)    /*!< SPI Baud rate Divisor */

#define SPIx_BR_SPPR_SHIFT           4

/***********  Bits definition for SPIx_S register  **************/
#define SPIx_S_SPRF                  ((uint8_t)0x80)    /*!< SPI Read Buffer Full Flag */
#define SPIx_S_SPMF                  ((uint8_t)0x40)    /*!< SPI Match Flag */
#define SPIx_S_SPTEF                 ((uint8_t)0x20)    /*!< SPI Transmit Buffer Empty Flag */
#define SPIx_S_MODF                  ((uint8_t)0x10)    /*!< Master Mode Fault Flag */

/***********  Bits definition for SPIx_D register  **************/
#define SPIx_D_DATA                  ((uint8_t)0xFF)    /*!< Data */

/***********  Bits definition for SPIx_M register  **************/
#define SPIx_M_DATA                  ((uint8_t)0xFF)    /*!< SPI HW Compare value for Match */

/****************************************************************/
/*                                                              */
/*             Inter-Integrated Circuit (I2C)                   */
/*                                                              */
/****************************************************************/
/***********  Bits definition for I2Cx_A1 register  *************/
#define I2Cx_A1_AD                   ((uint8_t)0xFE)    /*!< Address [7:1] */

#define I2Cx_A1_AD_SHIT              1

/***********  Bits definition for I2Cx_F register  **************/
#define I2Cx_F_MULT                  ((uint8_t)0xC0)    /*!< Multiplier factor */
#define I2Cx_F_ICR                   ((uint8_t)0x3F)    /*!< Clock rate */

#define I2Cx_F_MULT_SHIFT            5

/***********  Bits definition for I2Cx_C1 register  *************/
#define I2Cx_C1_IICEN                ((uint8_t)0x80)    /*!< I2C Enable */
#define I2Cx_C1_IICIE                ((uint8_t)0x40)    /*!< I2C Interrupt Enable */
#define I2Cx_C1_MST                  ((uint8_t)0x20)    /*!< Master Mode Select */
#define I2Cx_C1_TX                   ((uint8_t)0x10)    /*!< Transmit Mode Select */
#define I2Cx_C1_TXAK                 ((uint8_t)0x08)    /*!< Transmit Acknowledge Enable */
#define I2Cx_C1_RSTA                 ((uint8_t)0x04)    /*!< Repeat START */
#define I2Cx_C1_WUEN                 ((uint8_t)0x02)    /*!< Wakeup Enable */
#define I2Cx_C1_DMAEN                ((uint8_t)0x01)    /*!< DMA Enable */

/***********  Bits definition for I2Cx_S register  **************/
#define I2Cx_S_TCF                   ((uint8_t)0x80)    /*!< Transfer Complete Flag */
#define I2Cx_S_IAAS                  ((uint8_t)0x40)    /*!< Addressed As A Slave */
#define I2Cx_S_BUSY                  ((uint8_t)0x20)    /*!< Bus Busy */
#define I2Cx_S_ARBL                  ((uint8_t)0x10)    /*!< Arbitration Lost */
#define I2Cx_S_RAM                   ((uint8_t)0x08)    /*!< Range Address Match */
#define I2Cx_S_SRW                   ((uint8_t)0x04)    /*!< Slave Read/Write */
#define I2Cx_S_IICIF                 ((uint8_t)0x02)    /*!< Interrupt Flag */
#define I2Cx_S_RXAK                  ((uint8_t)0x01)    /*!< Receive Acknowledge */

/***********  Bits definition for I2Cx_D register  **************/
#define I2Cx_D_DATA                  ((uint8_t)0xFF)    /*!< Data */

/***********  Bits definition for I2Cx_C2 register  *************/
#define I2Cx_C2_GCAEN                ((uint8_t)0x80)    /*!< General Call Address Enable */
#define I2Cx_C2_ADEXT                ((uint8_t)0x40)    /*!< Address Extension */
#define I2Cx_C2_HDRS                 ((uint8_t)0x20)    /*!< High Drive Select */
#define I2Cx_C2_SBRC                 ((uint8_t)0x10)    /*!< Slave Baud Rate Control */
#define I2Cx_C2_RMEN                 ((uint8_t)0x08)    /*!< Range Address Matching Enable */
#define I2Cx_C2_AD_10_8              ((uint8_t)0x03)    /*!< Slave Address [10:8] */

/***********  Bits definition for I2Cx_FLT register  ************/
#define I2Cx_FLT_SHEN                ((uint8_t)0x80)    /*!< Stop Hold Enable */
#define I2Cx_FLT_STOPF               ((uint8_t)0x40)    /*!< I2C Bus Stop Detect Flag */
#define I2Cx_FLT_STOPIE              ((uint8_t)0x20)    /*!< I2C Bus Stop Interrupt Enable */
#define I2Cx_FLT_FLT                 ((uint8_t)0x1F)    /*!< I2C Programmable Filter Factor */

/***********  Bits definition for I2Cx_RA register  *************/
#define I2Cx_RA_RAD                  ((uint8_t)0xFE)    /*!< Range Slave Address */

#define I2Cx_RA_RAD_SHIFT            1

/***********  Bits definition for I2Cx_SMB register  ************/
#define I2Cx_SMB_FACK                ((uint8_t)0x80)    /*!< Fast NACK/ACK Enable */
#define I2Cx_SMB_ALERTEN             ((uint8_t)0x40)    /*!< SMBus Alert Response Address Enable */
#define I2Cx_SMB_SIICAEN             ((uint8_t)0x20)    /*!< Second I2C Address Enable */
#define I2Cx_SMB_TCKSEL              ((uint8_t)0x10)    /*!< Timeout Counter Clock Select */
#define I2Cx_SMB_SLTF                ((uint8_t)0x08)    /*!< SCL Low Timeout Flag */
#define I2Cx_SMB_SHTF1               ((uint8_t)0x04)    /*!< SCL High Timeout Flag 1 */
#define I2Cx_SMB_SHTF2               ((uint8_t)0x02)    /*!< SCL High Timeout Flag 2 */
#define I2Cx_SMB_SHTF2IE             ((uint8_t)0x01)    /*!< SHTF2 Interrupt Enable */

/***********  Bits definition for I2Cx_A2 register  *************/
#define I2Cx_A2_SAD                  ((uint8_t)0xFE)    /*!< SMBus Address */

#define I2Cx_A2_SAD_SHIFT            1

/***********  Bits definition for I2Cx_SLTH register  ***********/
#define I2Cx_SLTH_SSLT               ((uint8_t)0xFF)    /*!< MSB of SCL low timeout value */

/***********  Bits definition for I2Cx_SLTL register  ***********/
#define I2Cx_SLTL_SSLT               ((uint8_t)0xFF)    /*!< LSB of SCL low timeout value */

/****************************************************************/
/*                                                              */
/*     Universal Asynchronous Receiver/Transmitter (UART)       */
/*                                                              */
/****************************************************************/
/*********  Bits definition for UARTx_BDH register  *************/
#define UARTx_BDH_LBKDIE             ((uint8_t)0x80)    /*!< LIN Break Detect Interrupt Enable */
#define UARTx_BDH_RXEDGIE            ((uint8_t)0x40)    /*!< RX Input Active Edge Interrupt Enable */
#define UARTx_BDH_SBNS               ((uint8_t)0x20)    /*!< Stop Bit Number Select */
#define UARTx_BDH_SBR                ((uint8_t)0x1F)    /*!< Baud Rate Modulo Divisor */

/*********  Bits definition for UARTx_BDL register  *************/
#define UARTx_BDL_SBR                ((uint8_t)0xFF)    /*!< Baud Rate Modulo Divisor */

/*********  Bits definition for UARTx_C1 register  **************/
#define UARTx_C1_LOOPS               ((uint8_t)0x80)    /*!< Loop Mode Select */
#define UARTx_C1_DOZEEN              ((uint8_t)0x40)    /*!< Doze Enable */
#define UARTx_C1_UARTSWAI            ((uint8_t)0x40)    /*!< UART Stops in Wait Mode */
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
#define UARTx_S2_LBKDIF              ((uint8_t)0x80)    /*!< LIN Break Detect Interrupt Flag */
#define UARTx_S2_RXEDGIF             ((uint8_t)0x40)    /*!< UART_RX Pin Active Edge Interrupt Flag */
#define UARTx_S2_MSBF                ((uint8_t)0x20)    /*!< MSB First */
#define UARTx_S2_RXINV               ((uint8_t)0x10)    /*!< Receive Data Inversion */
#define UARTx_S2_RWUID               ((uint8_t)0x08)    /*!< Receive Wake Up Idle Detect */
#define UARTx_S2_BRK13               ((uint8_t)0x04)    /*!< Break Character Generation Length */
#define UARTx_S2_LBKDE               ((uint8_t)0x02)    /*!< LIN Break Detect Enable */
#define UARTx_S2_RAF                 ((uint8_t)0x01)    /*!< Receiver Active Flag */

/*********  Bits definition for UARTx_C3 register  **************/
#define UARTx_C3_R8T9                ((uint8_t)0x80)    /*!< Receive Bit 8 / Transmit Bit 9 */
#define UARTx_C3_R8                  ((uint8_t)0x80)    /*!< Ninth Data Bit for Receiver */
#define UARTx_C3_R9T8                ((uint8_t)0x40)    /*!< Receive Bit 9 / Transmit Bit 8 */
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

/*********  Bits definition for UARTx_MA1 register  *************/
#define UARTx_MA1_MA                 ((uint8_t)0xFF)    /*!< Match Address */

/*********  Bits definition for UARTx_MA2 register  *************/
#define UARTx_MA2_MA                 ((uint8_t)0xFF)    /*!< Match Address */

/*********  Bits definition for UARTx_C4 register  **************/
#define UARTx_C4_MAEN1               ((uint8_t)0x80)    /*!< Match Address Mode Enable 1 */
#define UARTx_C4_TDMAS               ((uint8_t)0x80)    /*!< Transmitter DMA Select */
#define UARTx_C4_MAEN2               ((uint8_t)0x40)    /*!< Match Address Mode Enable 2 */
#define UARTx_C4_M10                 ((uint8_t)0x20)    /*!< 10-bit Mode Select */
#define UARTx_C4_RDMAS               ((uint8_t)0x80)    /*!< Receiver Full DMA Select */
#define UARTx_C4_OSR                 ((uint8_t)0x1F)    /*!< Over Sampling Ratio */

/*********  Bits definition for UARTx_C5 register  **************/
#define UARTx_C5_TDMAE               ((uint8_t)0x80)    /*!< Transmitter DMA Enable */
#define UARTx_C5_RDMAE               ((uint8_t)0x20)    /*!< Receiver Full DMA Enable */
#define UARTx_C5_BOTHEDGE            ((uint8_t)0x02)    /*!< Both Edge Sampling */
#define UARTx_C5_RESYNCDIS           ((uint8_t)0x01)    /*!< Resynchronization Disable */
/****************************************************************/
/*                                                              */
/*             Power Management Controller (PMC)                */
/*                                                              */
/****************************************************************/
/*********  Bits definition for PMC_LVDSC1 register  *************/
#define PMC_LVDSC1_LVDF               ((uint8_t)0x80)   /*!< Low-Voltage Detect Flag */
#define PMC_LVDSC1_LVDACK             ((uint8_t)0x40)   /*!< Low-Voltage Detect Acknowledge */
#define PMC_LVDSC1_LVDIE              ((uint8_t)0x20)   /*!< Low-Voltage Detect Interrupt Enable */
#define PMC_LVDSC1_LVDRE              ((uint8_t)0x10)   /*!< Low-Voltage Detect Reset Enable */
#define PMC_LVDSC1_LVDV_MASK          ((uint8_t)0x3)    /*!< Low-Voltage Detect Voltage Select */
#define PMC_LVDSC1_LVDV_SHIFT         0
#define PMC_LVDSC1_LVDV(x)            (((uint8_t)(((uint8_t)(x))<<PMC_LVDSC1_LVDV_SHIFT))&PMC_LVDSC1_LVDV_MASK)
/*********  Bits definition for PMC_LVDSC1 register  *************/
#define PMC_LVDSC2_LVWF               ((uint8_t)0x80)   /*!< Low-Voltage Warning Flag */
#define PMC_LVDSC2_LVWACK             ((uint8_t)0x40)   /*!< Low-Voltage Warning Acknowledge */
#define PMC_LVDSC2_LVWIE              ((uint8_t)0x20)   /*!< Low-Voltage Warning Interrupt Enable */
#define PMC_LVDSC2_LVWV_MASK          0x3               /*!< Low-Voltage Warning Voltage Select */
#define PMC_LVDSC2_LVWV_SHIFT         0
#define PMC_LVDSC2_LVWV(x)            (((uint8_t)(((uint8_t)(x))<<PMC_LVDSC2_LVWV_SHIFT))&PMC_LVDSC2_LVWV_MASK)
/*********  Bits definition for PMC_REGSC register  *************/
#define PMC_REGSC_BGEN                ((uint8_t)0x10)   /*!< Bandgap Enable In VLPx Operation */
#define PMC_REGSC_ACKISO              ((uint8_t)0x8)    /*!< Acknowledge Isolation */
#define PMC_REGSC_REGONS              ((uint8_t)0x4)    /*!< Regulator In Run Regulation Status */
#define PMC_REGSC_BGBE                ((uint8_t)0x1)    /*!< Bandgap Buffer Enable */

#endif

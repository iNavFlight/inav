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

#ifndef _KL25Z_H_
#define _KL25Z_H_

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
  __IO uint8_t  C3;
  __IO uint8_t  C4;
  __IO uint8_t  C5;
  __IO uint8_t  C6;
  __I  uint8_t  S;
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
  __IO uint32_t GENCS;
  __IO uint32_t DATA;
  __IO uint32_t TSHD;
} TSI_TypeDef;

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
  __I  uint8_t  S1;
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

typedef struct {
  __I  uint8_t  PERID;               // 0x00
       uint8_t  RESERVED0[3];
  __I  uint8_t  IDCOMP;              // 0x04
       uint8_t  RESERVED1[3];
  __I  uint8_t  REV;                 // 0x08
       uint8_t  RESERVED2[3];
  __I  uint8_t  ADDINFO;             // 0x0C
       uint8_t  RESERVED3[3];
  __IO uint8_t  OTGISTAT;            // 0x10
       uint8_t  RESERVED4[3];
  __IO uint8_t  OTGICR;              // 0x14
       uint8_t  RESERVED5[3];
  __IO uint8_t  OTGSTAT;             // 0x18
       uint8_t  RESERVED6[3];
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
       uint8_t  RESERVED17[3];
  __IO uint8_t  TOKEN;               // 0xA8
       uint8_t  RESERVED18[3];
  __IO uint8_t  SOFTHLD;             // 0xAC
       uint8_t  RESERVED19[3];
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
} USBOTG_TypeDef;

typedef struct
{
  __I  uint8_t  SRS0;
  __I  uint8_t  SRS1;
       uint8_t  RESERVED0[2];
  __IO uint8_t  RPFC;
  __IO uint8_t  RPFW;
} RCM_TypeDef;

/****************************************************************/
/*                  Peripheral memory map                       */
/****************************************************************/
#define DMA_BASE                ((uint32_t)0x40008100)
#define FTFA_BASE               ((uint32_t)0x40020000)
#define DMAMUX_BASE             ((uint32_t)0x40021000)
#define PIT_BASE                ((uint32_t)0x40037000)
#define TPM0_BASE               ((uint32_t)0x40038000)
#define TPM1_BASE               ((uint32_t)0x40039000)
#define TPM2_BASE               ((uint32_t)0x4003A000)
#define ADC0_BASE               ((uint32_t)0x4003B000)
#define RTC_BASE                ((uint32_t)0x4003D000)
#define DAC0_BASE               ((uint32_t)0x4003F000)
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
#define USBOTG_BASE             ((uint32_t)0x40072000)
#define CMP_BASE                ((uint32_t)0x40073000)
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
#define PIT                     ((PIT_TypeDef *)     PIT_BASE)
#define TPM0                    ((TPM_TypeDef *)     TPM0_BASE)
#define TPM1                    ((TPM_TypeDef *)     TPM1_BASE)
#define TPM2                    ((TPM_TypeDef *)     TPM2_BASE)
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
#define USB0                    ((USBOTG_TypeDef *)  USBOTG_BASE)
#define CMP                     ((CMP_TypeDef *)     CMP_BASE)
#define MCG                     ((MCG_TypeDef  *)    MCG_BASE)
#define OSC0                    ((OSC_TypeDef  *)    OSC0_BASE)
#define SPI0                    ((SPI_TypeDef *)     SPI0_BASE)
#define SPI1                    ((SPI_TypeDef *)     SPI1_BASE)
#define I2C0                    ((I2C_TypeDef *)     I2C0_BASE)
#define I2C1                    ((I2C_TypeDef *)     I2C1_BASE)
#define UART0                   ((UARTLP_TypeDef *)  UART0_BASE)
#define UART1                   ((UART_TypeDef *)    UART1_BASE)
#define UART2                   ((UART_TypeDef *)    UART2_BASE)
#define SMC                     ((SMC_TypeDef  *)    SMC_BASE)
#define RCM                     ((RCM_TypeDef  *)    RCM_BASE)
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

/*******  Bits definition for SIM_SOPT4 register  ************/
#define SIM_SOPT4_TPM2CLKSEL         ((uint32_t)0x04000000)    /*!< TPM2 External Clock Pin Select */
#define SIM_SOPT4_TPM1CLKSEL         ((uint32_t)0x02000000)    /*!< TPM1 External Clock Pin Select */
#define SIM_SOPT4_TPM0CLKSEL         ((uint32_t)0x01000000)    /*!< TPM0 External Clock Pin Select */
#define SIM_SOPT4_TPM2CH0SRC         ((uint32_t)0x00100000)    /*!< TPM2 channel 0 input capture source select */
#define SIM_SOPT4_TPM1CH0SRC         ((uint32_t)0x00040000)    /*!< TPM1 channel 0 input capture source select */

/*******  Bits definition for SIM_SOPT5 register  ************/
#define SIM_SOPT5_UART2ODE           ((uint32_t)0x00040000)    /*!< UART2 Open Drain Enable */
#define SIM_SOPT5_UART1ODE           ((uint32_t)0x00020000)    /*!< UART1 Open Drain Enable */
#define SIM_SOPT5_UART0ODE           ((uint32_t)0x00010000)    /*!< UART0 Open Drain Enable */
#define SIM_SOPT5_UART1RXSRC         ((uint32_t)0x00000040)    /*!< UART1 receive data source select */
#define SIM_SOPT5_UART1TXSRC_SHIFT   4                                                                                        /*!< UART1 transmit data source select (shift) */
#define SIM_SOPT5_UART1TXSRC_MASK    ((uint32_t)((uint32_t)0x03 << SIM_SOPT5_UART1TXSRC_SHIFT))                               /*!< UART1 transmit data source select (mask) */
#define SIM_SOPT5_UART1TXSRC(x)      ((uint32_t)(((uint32_t)(x) << SIM_SOPT5_UART1TXSRC_SHIFT) & SIM_SOPT5_UART1TXSRC_MASK))  /*!< UART1 transmit data source select */
#define SIM_SOPT5_UART0RXSRC         ((uint32_t)0x00000040)    /*!< UART0 receive data source select */
#define SIM_SOPT5_UART0TXSRC_SHIFT   0                                                                                        /*!< UART0 transmit data source select (shift) */
#define SIM_SOPT5_UART0TXSRC_MASK    ((uint32_t)((uint32_t)0x03 << SIM_SOPT5_UART0TXSRC_SHIFT))                               /*!< UART0 transmit data source select (mask) */
#define SIM_SOPT5_UART0TXSRC(x)      ((uint32_t)(((uint32_t)(x) << SIM_SOPT5_UART0TXSRC_SHIFT) & SIM_SOPT5_UART0TXSRC_MASK))  /*!< UART0 transmit data source select */

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
#define SIM_SDID_DIEID_SHIFT         7                                                       /*!< Device die number (shift) */
#define SIM_SDID_DIEID_MASK          ((uint32_t)((uint32_t)0x1F << SIM_SDID_DIEID_SHIFT))    /*!< Device die number (mask) */
#define SIM_SDID_PINID_SHIFT         0                                                       /*!< Pincount identification (shift) */
#define SIM_SDID_PINID_MASK          ((uint32_t)((uint32_t)0x0F << SIM_SDID_PINID_SHIFT))    /*!< Pincount identification (mask) */

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
#define SIM_FCFG2_MAXADDR0_SHIFT     24                                                        /*!< Max address block (shift) */
#define SIM_FCFG2_MAXADDR0_MASK      ((uint32_t)((uint32_t)0x7F << SIM_FCFG2_MAXADDR0_SHIFT))  /*!< Max address block (mask) */

/*******  Bits definition for SIM_UIDMH register  ************/
#define SIM_UIDMH_UID_MASK           ((uint32_t)0x0000FFFF)   /*!< Unique Identification */

/*******  Bits definition for SIM_UIDML register  ************/
#define SIM_UIDML_UID_MASK           ((uint32_t)0xFFFFFFFF)   /*!< Unique Identification */

/*******  Bits definition for SIM_UIDL register  *************/
#define SIM_UIDL_UID_MASK            ((uint32_t)0xFFFFFFFF)   /*!< Unique Identification */

/*******  Bits definition for SIM_COPC register  *************/
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
#define SPIx_BR_SPPR_SHIFT           4                  /*!< SPI Baud rate Prescaler Divisor */
#define SPIx_BR_SPPR_MASK            ((uint8_t)((uint8_t)0x7 << SPIx_BR_SPPR_SHIFT))
#define SPIx_BR_SPPR(x)              ((uint8_t)(((uint8_t)(x) << SPIx_BR_SPPR_SHIFT) & SPIx_BR_SPPR_MASK))
#define SPIx_BR_SPR_SHIFT            0                  /*!< SPI Baud rate Divisor */
#define SPIx_BR_SPR_MASK             ((uint8_t)((uint8_t)0x0F << SPIx_BR_SPR_SHIFT))
#define SPIx_BR_SPR(x)               ((uint8_t)(((uint8_t)(x) << SPIx_BR_SPR_SHIFT) & SPIx_BR_SPR_MASK))

/***********  Bits definition for SPIx_S register  **************/
#define SPIx_S_SPRF                  ((uint8_t)0x80)    /*!< SPI Read Buffer Full Flag */
#define SPIx_S_SPMF                  ((uint8_t)0x40)    /*!< SPI Match Flag */
#define SPIx_S_SPTEF                 ((uint8_t)0x20)    /*!< SPI Transmit Buffer Empty Flag */
#define SPIx_S_MODF                  ((uint8_t)0x10)    /*!< Master Mode Fault Flag */

/***********  Bits definition for SPIx_D register  **************/
#define SPIx_D_DATA_SHIFT            0                  /*!< Data */
#define SPIx_D_DATA_MASK             ((uint8_t)((uint8_t)0xFF << SPIx_D_DATA_SHIFT))
#define SPIx_D_DATA(x)               ((uint8_t)(((uint8_t)(x) << SPIx_D_DATA_SHIFT) & SPIx_D_DATA_MASK))

/***********  Bits definition for SPIx_M register  **************/
#define SPIx_M_DATA_SHIFT            0                  /*!< SPI HW Compare value for Match */
#define SPIx_M_DATA_MASK             ((uint8_t)((uint8_t)0xFF << SPIx_M_DATA_SHIFT))
#define SPIx_M_DATA(x)               ((uint8_t)(((uint8_t)(x) << SPIx_M_DATA_SHIFT) & SPIx_M_DATA_MASK))

/****************************************************************/
/*                                                              */
/*   Inter-Integrated Circuit (I2C): Device dependent part      */
/*                                                              */
/****************************************************************/
/***********  Bits definition for I2Cx_FLT register  ************/
#define I2Cx_FLT_SHEN                ((uint8_t)0x80)    /*!< Stop Hold Enable */
#define I2Cx_FLT_STOPF               ((uint8_t)0x40)    /*!< I2C Bus Stop Detect Flag */
#define I2Cx_FLT_STOPIE              ((uint8_t)0x20)    /*!< I2C Bus Stop Interrupt Enable */
#define I2Cx_FLT_FLT_SHIFT           0                  /*!< I2C Programmable Filter Factor */
#define I2Cx_FLT_FLT_MASK            ((uint8_t)((uint8_t)0x1F << I2Cx_FLT_FLT_SHIFT))
#define I2Cx_FLT_FLT(x)              ((uint8_t)(((uint8_t)(x) << I2Cx_FLT_FLT_SHIFT) & I2Cx_FLT_FLT_MASK))

/****************************************************************/
/*                                                              */
/*     Universal Asynchronous Receiver/Transmitter (UART)       */
/*                                                              */
/****************************************************************/
/*********  Bits definition for UARTx_BDH register  *************/
#define UARTx_BDH_LBKDIE             ((uint8_t)0x80)    /*!< LIN Break Detect Interrupt Enable */
#define UARTx_BDH_RXEDGIE            ((uint8_t)0x40)    /*!< RX Input Active Edge Interrupt Enable */
#define UARTx_BDH_SBNS               ((uint8_t)0x20)    /*!< Stop Bit Number Select */
#define UARTx_BDH_SBR_SHIFT          0                  /*!< Baud Rate Modulo Divisor */
#define UARTx_BDH_SBR_MASK           ((uint8_t)((uint8_t)0x1F << UARTx_BDH_SBR_SHIFT))
#define UARTx_BDH_SBR(x)             ((uint8_t)(((uint8_t)(x) << UARTx_BDH_SBR_SHIFT) & UARTx_BDH_SBR_MASK))

/*********  Bits definition for UARTx_BDL register  *************/
#define UARTx_BDL_SBR_SHIFT          0                  /*!< Baud Rate Modulo Divisor */
#define UARTx_BDL_SBR_MASK           ((uint8_t)((uint8_t)0xFF << UARTx_BDL_SBR_SHIFT))
#define UARTx_BDL_SBR(x)             ((uint8_t)(((uint8_t)(x) << UARTx_BDL_SBR_SHIFT) & UARTx_BDL_SBR_MASK))

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
#define UARTx_C4_TDMAS               ((uint8_t)0x80)    /*!< Transmitter DMA Select */
#define UARTx_C4_RDMAS               ((uint8_t)0x20)    /*!< Receiver Full DMA Select */
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

#define TPMx_SC_CMOD_DISABLE          TPMx_SC_CMOD(0)
#define TPMx_SC_CMOD_LPTPM_CLK        TPMx_SC_CMOD(1)
#define TPMx_SC_CMOD_LPTPM_EXTCLK     TPMx_SC_CMOD(2)

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

/**********  Bits definition for TPMx_CONF register  *************/
#define TPMx_CONF_TRGSEL_SHIFT        24                 /*!< Trigger Select */
#define TPMx_CONF_TRGSEL_MASK         ((uint32_t)((uint32_t)0xF << TPMx_CONF_TRGSEL_SHIFT))
#define TPMx_CONF_TRGSEL(x)           ((uint32_t)(((uint32_t)(x) << TPMx_CONF_TRGSEL_SHIFT) & TPMx_CONF_TRGSEL_MASK))
#define TPMx_CONF_CROT                ((uint32_t)0x40000) /*!< Counter Reload On Trigger */
#define TPMx_CONF_CSOO                ((uint32_t)0x20000) /*!< Counter Stop On Overflow */
#define TPMx_CONF_CSOT                ((uint32_t)0x10000) /*!< Counter Start on Trigger */
#define TPMx_CONF_GTBEEN              ((uint32_t)0x200)  /*!< Global time base enable */
#define TPMx_CONF_DBGMODE_SHIFT       6                  /*!< Debug Mode */
#define TPMx_CONF_DBGMODE_MASK        ((uint32_t)((uint32_t)0x3 << TPMx_CONF_DBGMODE_SHIFT))
#define TPMx_CONF_DBGMODE(x)          ((uint32_t)(((uint32_t)(x) << TPMx_CONF_DBGMODE_SHIFT) & TPMx_CONF_DBGMODE_MASK))
#define TPMx_CONF_DOZEEN              ((uint32_t)0x20)   /*!< Doze Enable */

#define TPMx_CONF_DBGMODE_CONT        TPMx_CONF_DBGMODE(3)
#define TPMx_CONF_DBGMODE_PAUSE       TPMx_CONF_DBGMODE(0)

/****************************************************************/
/*                                                              */
/*               USB OTG: device dependent parts                */
/*                                                              */
/****************************************************************/
/********  Bits definition for USBx_ADDINFO register  ***********/
#define USBx_ADDINFO_IRQNUM_SHIFT    6                  /*!< Assigned Interrupt Request Number */
#define USBx_ADDINFO_IRQNUM_MASK     ((uint8_t)((uint8_t)0x1F << USBx_ADDINFO_IRQNUM_SHIFT))

/********  Bits definition for USBx_OTGISTAT register  **********/
#define USBx_OTGISTAT_IDCHG          ((uint8_t)0x80)    /*!< Change in the ID Signal from the USB connector is sensed. */
#define USBx_OTGISTAT_ONEMSEC        ((uint8_t)0x40)    /*!< Set when the 1 millisecond timer expires. */
#define USBx_OTGISTAT_LINE_STATE_CHG ((uint8_t)0x20)    /*!< Set when the USB line state changes. */
#define USBx_OTGISTAT_SESSVLDCHG     ((uint8_t)0x08)    /*!< Set when a change in VBUS is detected indicating a session valid or a session no longer valid. */
#define USBx_OTGISTAT_B_SESS_CHG     ((uint8_t)0x04)    /*!< Set when a change in VBUS is detected on a B device. */
#define USBx_OTGISTAT_AVBUSCHG       ((uint8_t)0x01)    /*!< Set when a change in VBUS is detected on an A device. */

/********  Bits definition for USBx_OTGICR register  ************/
#define USBx_OTGICR_IDEN             ((uint8_t)0x80)    /*!< ID Interrupt Enable */
#define USBx_OTGICR_ONEMSECEN        ((uint8_t)0x40)    /*!< One Millisecond Interrupt Enable */
#define USBx_OTGICR_LINESTATEEN      ((uint8_t)0x20)    /*!< Line State Change Interrupt Enable */
#define USBx_OTGICR_SESSVLDEN        ((uint8_t)0x08)    /*!< Session Valid Interrupt Enable */
#define USBx_OTGICR_BSESSEN          ((uint8_t)0x04)    /*!< B Session END Interrupt Enable */
#define USBx_OTGICR_AVBUSEN          ((uint8_t)0x01)    /*!< A VBUS Valid Interrupt Enable */

/********  Bits definition for USBx_OTGSTAT register  ***********/
#define USBx_OTGSTAT_ID              ((uint8_t)0x80)    /*!< Indicates the current state of the ID pin on the USB connector */
#define USBx_OTGSTAT_ONEMSECEN       ((uint8_t)0x40)    /*!< This bit is reserved for the 1ms count, but it is not useful to software. */
#define USBx_OTGSTAT_LINESTATESTABLE ((uint8_t)0x20)    /*!< Indicates that the internal signals that control the LINE_STATE_CHG field of OTGISTAT are stable for at least 1 millisecond. */
#define USBx_OTGSTAT_SESS_VLD        ((uint8_t)0x08)    /*!< Session Valid */
#define USBx_OTGSTAT_BSESSEND        ((uint8_t)0x04)    /*!< B Session End */
#define USBx_OTGSTAT_AVBUSVLD        ((uint8_t)0x01)    /*!< A VBUS Valid */

/********  Bits definition for USBx_OTGCTL register  ************/
#define USBx_OTGCTL_DPLOW            ((uint8_t)0x20)    /*!< D+ Data Line pull-down resistor enable */
#define USBx_OTGCTL_DMLOW            ((uint8_t)0x10)    /*!< D– Data Line pull-down resistor enable */
#define USBx_OTGCTL_OTGEN            ((uint8_t)0x04)    /*!< On-The-Go pullup/pulldown resistor enable */

/********  Bits definition for USBx_ISTAT register  *************/
#define USBx_ISTAT_ATTACH            ((uint8_t)0x40) /*!< Attach interrupt */

/******** Bits definition for USBx_INTEN register ***************/
#define USBx_INTEN_ATTACHEN          ((uint8_t)0x40) /*!< ATTACH interrupt enable */

/******** Bits definition for USBx_CTL register *****************/
#define USBx_CTL_RESET               ((uint8_t)0x10) /*!< Generates an USB reset signal (host mode) */
#define USBx_CTL_HOSTMODEEN          ((uint8_t)0x08) /*!< Operate in Host mode */
#define USBx_CTL_RESUME              ((uint8_t)0x04) /*!< Executes resume signaling */

/******** Bits definition for USBx_ADDR register ****************/
#define USBx_ADDR_LSEN               ((uint8_t)0x80) /*!< Low Speed Enable bit */

/******** Bits definition for USBx_TOKEN register ***************/
#define USBx_TOKEN_TOKENPID_SHIFT    4               /*!< Contains the token type executed by the USB module. */
#define USBx_TOKEN_TOKENPID_MASK     ((uint8_t)((uint8_t)0x0F << USBx_TOKEN_TOKENPID_SHIFT))
#define USBx_TOKEN_TOKENPID(x)       ((uint8_t)(((uint8_t)(x) << USBx_TOKEN_TOKENPID_SHIFT) & USBx_TOKEN_TOKENPID_MASK))
#define USBx_TOKEN_TOKENENDPT_SHIFT  0               /*!< Holds the Endpoint address for the token command. */
#define USBx_TOKEN_TOKENENDPT_MASK   ((uint8_t)((uint8_t)0x0F << USBx_TOKEN_TOKENENDPT_SHIFT))
#define USBx_TOKEN_TOKENENDPT(x)     ((uint8_t)(((uint8_t)(x) << USBx_TOKEN_TOKENENDPT_SHIFT) & USBx_TOKEN_TOKENENDPT_MASK))
#define USBx_TOKEN_TOKENPID_OUT      0x1
#define USBx_TOKEN_TOKENPID_IN       0x9
#define USBx_TOKEN_TOKENPID_SETUP    0xD

/******** Bits definition for USBx_ENDPTn register **************/
#define USBx_ENDPTn_HOSTWOHUB        ((uint8_t)0x80)
#define USBx_ENDPTn_RETRYDIS         ((uint8_t)0x40)

/****************************************************************/
/*                                                              */
/*                 Reset Control Module (RCM)                   */
/*                                                              */
/****************************************************************/

/* Only device independent parts */

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

#define DACx_C1_DACBFMD_SHIFT         2     /*!< DAC Buffer Work Mode Select */
#define DACx_C1_DACBFMD_MASK          ((uint8_t)((uint8_t)0x01 << DACx_C1_DACBFMD_  SHIFT))
#define DACx_C1_DACBFMD(x)            ((uint8_t)(((uint8_t)(x) << DACx_C1_DACBFMD_SHIFT) & DACx_C1_DACBFMD_MASK))

#define DACx_C1_DACBFMD_MODE_NORMAL   0
#define DACx_C1_DACBFMD_MODE_OTS      1

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

#endif /* _KL25Z_H_ */

/*
 * Copyright (C) 2014-2016 Fabio Utzig, http://fabioutzig.com
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

#ifndef _K20x5_H_
#define _K20x5_H_

/*
 * ==============================================================
 * ---------- Interrupt Number Definition -----------------------
 * ==============================================================
 */
typedef enum IRQn
{
/******  Cortex-M0 Processor Exceptions Numbers ****************/
  InitialSP_IRQn                = -15,
  InitialPC_IRQn                = -15,
  NonMaskableInt_IRQn           = -14,
  HardFault_IRQn                = -13,
  MemoryManagement_IRQn         = -12,
  BusFault_IRQn                 = -11,
  UsageFault_IRQn               = -10,
  SVCall_IRQn                   = -5,
  DebugMonitor_IRQn             = -4,
  PendSV_IRQn                   = -2,
  SysTick_IRQn                  = -1,

/******  K20x Specific Interrupt Numbers ***********************/
  DMA0_IRQn                     = 0,  // Vector40
  DMA1_IRQn                     = 1,  // Vector44
  DMA2_IRQn                     = 2,  // Vector48
  DMA3_IRQn                     = 3,  // Vector4C
  DMAError_IRQn                 = 4,  // Vector50
  DMA_IRQn                      = 5,  // Vector54
  FlashMemComplete_IRQn         = 6,  // Vector58
  FlashMemReadCollision_IRQn    = 7,  // Vector5C
  LowVoltageWarning_IRQn        = 8,  // Vector60
  LLWU_IRQn                     = 9,  // Vector64
  WDOG_IRQn                     = 10, // Vector68
  I2C0_IRQn                     = 11, // Vector6C
  SPI0_IRQn                     = 12, // Vector70
  I2S0_IRQn                     = 13, // Vector74
  I2S1_IRQn                     = 14, // Vector78
  UART0LON_IRQn                 = 15, // Vector7C
  UART0Status_IRQn              = 16, // Vector80
  UART0Error_IRQn               = 17, // Vector84
  UART1Status_IRQn              = 18, // Vector88
  UART1Error_IRQn               = 19, // Vector8C
  UART2Status_IRQn              = 20, // Vector90
  UART2Error_IRQn               = 21, // Vector94
  ADC0_IRQn                     = 22, // Vector98
  CMP0_IRQn                     = 23, // Vector9C
  CMP1_IRQn                     = 24, // VectorA0
  FTM0_IRQn                     = 25, // VectorA4
  FTM1_IRQn                     = 26, // VectorA8
  CMT_IRQn                      = 27, // VectorAC
  RTCAlarm_IRQn                 = 28, // VectorB0
  RTCSeconds_IRQn               = 29, // VectorB4
  PITChannel0_IRQn              = 30, // VectorB8
  PITChannel1_IRQn              = 31, // VectorBC
  PITChannel2_IRQn              = 32, // VectorC0
  PITChannel3_IRQn              = 33, // VectorC4
  PDB_IRQn                      = 34, // VectorC8
  USB_OTG_IRQn                  = 35, // VectorCC
  USBChargerDetect_IRQn         = 36, // VectorD0
  TSI_IRQn                      = 37, // VectorD4
  MCG_IRQn                      = 38, // VectorD8
  LPTMR0_IRQn                   = 39, // VectorDC
  PINA_IRQn                     = 40, // VectorE0
  PINB_IRQn                     = 41, // VectorE4
  PINC_IRQn                     = 42, // VectorE8
  PIND_IRQn                     = 43, // VectorEC
  PINE_IRQn                     = 44, // VectorF0
  SoftInitInt_IRQn              = 45, // VectorF4
} IRQn_Type;

/*
 * ==========================================================================
 * ----------- Processor and Core Peripheral Section ------------------------
 * ==========================================================================
 */

/**
 * @brief K20x Interrupt Number Definition, according to the selected device
 *        in @ref Library_configuration_section
 */
#define __FPU_PRESENT             0
#define __MPU_PRESENT             0
#define __NVIC_PRIO_BITS          4
#define __Vendor_SysTickConfig    0

#include "core_cm4.h"            /* Cortex-M4 processor and core peripherals */

#include "k20xx.h"

typedef struct
{
  __IO uint32_t SOPT1;
  __IO uint32_t SOPT1CFG;
       uint32_t RESERVED0[1023];
  __IO uint32_t SOPT2;
       uint32_t RESERVED1[1];
  __IO uint32_t SOPT4;
  __IO uint32_t SOPT5;
       uint32_t RESERVED2[1];
  __IO uint32_t SOPT7;
       uint32_t RESERVED3[2];
  __I  uint32_t SDID;
       uint32_t RESERVED4[3];
  __IO uint32_t SCGC4;
  __IO uint32_t SCGC5;
  __IO uint32_t SCGC6;
  __IO uint32_t SCGC7;
  __IO uint32_t CLKDIV1;
  __IO uint32_t CLKDIV2;
  __I  uint32_t FCFG1;
  __I  uint32_t FCFG2;
  __I  uint32_t UIDH;
  __I  uint32_t UIDMH;
  __I  uint32_t UIDML;
  __I  uint32_t UIDL;
} SIM_TypeDef;

/****************************************************************/
/*                  Peripheral memory map                       */
/****************************************************************/
#define DMA_BASE                ((uint32_t)0x40008000)
#define FTFL_BASE               ((uint32_t)0x40020000)
#define DMAMUX_BASE             ((uint32_t)0x40021000)
#define SPI0_BASE               ((uint32_t)0x4002C000)
#define PIT_BASE                ((uint32_t)0x40037000)
#define FTM0_BASE               ((uint32_t)0x40038000)
#define FTM1_BASE               ((uint32_t)0x40039000)
#define ADC0_BASE               ((uint32_t)0x4003B000)
#define VBAT_BASE               ((uint32_t)0x4003E000)
#define LPTMR0_BASE             ((uint32_t)0x40040000)
#define SRF_BASE                ((uint32_t)0x40041000)
#define TSI0_BASE               ((uint32_t)0x40045000)
#define SIM_BASE                ((uint32_t)0x40047000)
#define PORTA_BASE              ((uint32_t)0x40049000)
#define PORTB_BASE              ((uint32_t)0x4004A000)
#define PORTC_BASE              ((uint32_t)0x4004B000)
#define PORTD_BASE              ((uint32_t)0x4004C000)
#define PORTE_BASE              ((uint32_t)0x4004D000)
#define WDOG_BASE               ((uint32_t)0x40052000)
#define MCG_BASE                ((uint32_t)0x40064000)
#define OSC0_BASE               ((uint32_t)0x40065000)
#define I2C0_BASE               ((uint32_t)0x40066000)
#define UART0_BASE              ((uint32_t)0x4006A000)
#define UART1_BASE              ((uint32_t)0x4006B000)
#define UART2_BASE              ((uint32_t)0x4006C000)
#define USBOTG_BASE             ((uint32_t)0x40072000)
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
#define FTFL                    ((FTFL_TypeDef *)    FTFL_BASE)
#define DMAMUX                  ((DMAMUX_TypeDef *)  DMAMUX_BASE)
#define PIT                     ((PIT_TypeDef *)     PIT_BASE)
#define FTM0                    ((FTM_TypeDef *)     FTM0_BASE)
#define FTM1                    ((FTM_TypeDef *)     FTM1_BASE)
#define ADC0                    ((ADC_TypeDef *)     ADC0_BASE)
#define VBAT                    ((volatile uint8_t *)VBAT_BASE) /* 32 bytes */
#define LPTMR0                  ((LPTMR_TypeDef *)   LPTMR0_BASE)
#define SYSTEM_REGISTER_FILE    ((volatile uint8_t *)SRF_BASE) /* 32 bytes */
#define TSI0                    ((TSI_TypeDef *)     TSI0_BASE)
#define SIM                     ((SIM_TypeDef  *)    SIM_BASE)
#define LLWU                    ((LLWU_TypeDef  *)   LLWU_BASE)
#define PMC                     ((PMC_TypeDef  *)    PMC_BASE)
#define PORTA                   ((PORT_TypeDef  *)   PORTA_BASE)
#define PORTB                   ((PORT_TypeDef  *)   PORTB_BASE)
#define PORTC                   ((PORT_TypeDef  *)   PORTC_BASE)
#define PORTD                   ((PORT_TypeDef  *)   PORTD_BASE)
#define PORTE                   ((PORT_TypeDef  *)   PORTE_BASE)
#define WDOG                    ((WDOG_TypeDef  *)   WDOG_BASE)
#define USB0                    ((USBOTG_TypeDef *)  USBOTG_BASE)
#define MCG                     ((MCG_TypeDef  *)    MCG_BASE)
#define OSC0                    ((OSC_TypeDef  *)    OSC0_BASE)
#define SPI0                    ((SPI_TypeDef *)     SPI0_BASE)
#define I2C0                    ((I2C_TypeDef *)     I2C0_BASE)
#define UART0                   ((UART_TypeDef *)    UART0_BASE)
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
#define SIM_SOPT1_OSC32KSEL_SHIFT    18                        /*!< 32K oscillator clock select (shift) */
#define SIM_SOPT1_OSC32KSEL_MASK     ((uint32_t)((uint32_t)0x3 << SIM_SOPT1_OSC32KSEL_SHIFT))                              /*!< 32K oscillator clock select (mask) */
#define SIM_SOPT1_OSC32KSEL(x)       ((uint32_t)(((uint32_t)(x) << SIM_SOPT1_OSC32KSEL_SHIFT) & SIM_SOPT1_OSC32KSEL_MASK))  /*!< 32K oscillator clock select */
#define SIM_SOPT1_RAMSIZE_SHIFT      12
#define SIM_SOPT1_RAMSIZE_MASK       ((uint32_t)((uint32_t)0xf << SIM_SOPT1_RAMSIZE_SHIFT))
#define SIM_SOPT1_RAMSIZE(x)         ((uint32_t)(((uint32_t)(x) << SIM_SOPT1_RAMSIZE_SHIFT) & SIM_SOPT1_RAMSIZE_MASK))

/*******  Bits definition for SIM_SOPT1CFG register  ************/
#define SIM_SOPT1CFG_USSWE           ((uint32_t)0x04000000)    /*!< USB voltage regulator stop standby write enable */
#define SIM_SOPT1CFG_UVSWE           ((uint32_t)0x02000000)    /*!< USB voltage regulator VLP standby write enable */
#define SIM_SOPT1CFG_URWE            ((uint32_t)0x01000000)    /*!< USB voltage regulator voltage regulator write enable */

/*******  Bits definition for SIM_SOPT2 register  ************/
#define SIM_SOPT2_USBSRC             ((uint32_t)0x00040000)    /*!< USB clock source select */
#define SIM_SOPT2_PLLFLLSEL          ((uint32_t)0x00010000)    /*!< PLL/FLL clock select */
#define SIM_SOPT2_TRACECLKSEL        ((uint32_t)0x00001000)
#define SIM_SOPT2_PTD7PAD            ((uint32_t)0x00000800)
#define SIM_SOPT2_CLKOUTSEL_SHIFT    5
#define SIM_SOPT2_CLKOUTSEL_MASK     ((uint32_t)((uint32_t)0x7 << SIM_SOPT2_CLKOUTSEL_SHIFT))
#define SIM_SOPT2_CLKOUTSEL(x)       ((uint32_t)(((uint32_t)(x) << SIM_SOPT2_CLKOUTSEL_SHIFT) & SIM_SOPT2_CLKOUTSEL_MASK))
#define SIM_SOPT2_RTCCLKOUTSEL       ((uint32_t)0x00000010)    /*!< RTC clock out select */

/*******  Bits definition for SIM_SCGC4 register  ************/
#define SIM_SCGC4_VREF               ((uint32_t)0x00100000)    /*!< VREF Clock Gate Control */
#define SIM_SCGC4_CMP                ((uint32_t)0x00080000)    /*!< Comparator Clock Gate Control */
#define SIM_SCGC4_USBOTG             ((uint32_t)0x00040000)    /*!< USB Clock Gate Control */
#define SIM_SCGC4_UART2              ((uint32_t)0x00001000)    /*!< UART2 Clock Gate Control */
#define SIM_SCGC4_UART1              ((uint32_t)0x00000800)    /*!< UART1 Clock Gate Control */
#define SIM_SCGC4_UART0              ((uint32_t)0x00000400)    /*!< UART0 Clock Gate Control */
#define SIM_SCGC4_I2C0               ((uint32_t)0x00000040)    /*!< I2C0 Clock Gate Control */
#define SIM_SCGC4_CMT                ((uint32_t)0x00000004)    /*!< CMT Clock Gate Control */
#define SIM_SCGC4_EMW                ((uint32_t)0x00000002)    /*!< EWM Clock Gate Control */

/*******  Bits definition for SIM_SCGC5 register  ************/
#define SIM_SCGC5_PORTE              ((uint32_t)0x00002000)    /*!< Port E Clock Gate Control */
#define SIM_SCGC5_PORTD              ((uint32_t)0x00001000)    /*!< Port D Clock Gate Control */
#define SIM_SCGC5_PORTC              ((uint32_t)0x00000800)    /*!< Port C Clock Gate Control */
#define SIM_SCGC5_PORTB              ((uint32_t)0x00000400)    /*!< Port B Clock Gate Control */
#define SIM_SCGC5_PORTA              ((uint32_t)0x00000200)    /*!< Port A Clock Gate Control */
#define SIM_SCGC5_TSI                ((uint32_t)0x00000020)    /*!< TSI Access Control */
#define SIM_SCGC5_LPTIMER            ((uint32_t)0x00000001)    /*!< Low Power Timer Access Control */

/*******  Bits definition for SIM_SCGC6 register  ************/
#define SIM_SCGC6_RTC                ((uint32_t)0x20000000)    /*!< RTC Access Control */
#define SIM_SCGC6_ADC0               ((uint32_t)0x08000000)    /*!< ADC0 Clock Gate Control */
#define SIM_SCGC6_FTM1               ((uint32_t)0x02000000)    /*!< FTM1 Clock Gate Control */
#define SIM_SCGC6_FTM0               ((uint32_t)0x01000000)    /*!< FTM0 Clock Gate Control */
#define SIM_SCGC6_PIT                ((uint32_t)0x00800000)    /*!< PIT Clock Gate Control */
#define SIM_SCGC6_PDB                ((uint32_t)0x00400000)    /*!< PDB Clock Gate Control */
#define SIM_SCGC6_USBDCD             ((uint32_t)0x00200000)    /*!< USB DCD Clock Gate Control */
#define SIM_SCGC6_CRC                ((uint32_t)0x00040000)    /*!< Low Power Timer Access Control */
#define SIM_SCGC6_I2S                ((uint32_t)0x00008000)    /*!< CRC Clock Gate Control */
#define SIM_SCGC6_SPI0               ((uint32_t)0x00001000)    /*!< SPI0 Clock Gate Control */
#define SIM_SCGC6_DMAMUX             ((uint32_t)0x00000002)    /*!< DMA Mux Clock Gate Control */
#define SIM_SCGC6_FTFL               ((uint32_t)0x00000001)    /*!< Flash Memory Clock Gate Control */

/*******  Bits definition for SIM_SCGC6 register  ************/
#define SIM_SCGC7_DMA                ((uint32_t)0x00000002)    /*!< DMA Clock Gate Control */

/******  Bits definition for SIM_CLKDIV1 register  ***********/
#define SIM_CLKDIV1_OUTDIV1_SHIFT    28
#define SIM_CLKDIV1_OUTDIV1_MASK     ((uint32_t)((uint32_t)0xF << SIM_CLKDIV1_OUTDIV1_SHIFT))
#define SIM_CLKDIV1_OUTDIV1(x)       ((uint32_t)(((uint32_t)(x) << SIM_CLKDIV1_OUTDIV1_SHIFT) & SIM_CLKDIV1_OUTDIV1_MASK))
#define SIM_CLKDIV1_OUTDIV2_SHIFT    24
#define SIM_CLKDIV1_OUTDIV2_MASK     ((uint32_t)((uint32_t)0xF << SIM_CLKDIV1_OUTDIV2_SHIFT))
#define SIM_CLKDIV1_OUTDIV2(x)       ((uint32_t)(((uint32_t)(x) << SIM_CLKDIV1_OUTDIV2_SHIFT) & SIM_CLKDIV1_OUTDIV2_MASK))
#define SIM_CLKDIV1_OUTDIV4_SHIFT    16
#define SIM_CLKDIV1_OUTDIV4_MASK     ((uint32_t)((uint32_t)0x7 << SIM_CLKDIV1_OUTDIV4_SHIFT))
#define SIM_CLKDIV1_OUTDIV4(x)       ((uint32_t)(((uint32_t)(x) << SIM_CLKDIV1_OUTDIV4_SHIFT) & SIM_CLKDIV1_OUTDIV4_MASK))

/******  Bits definition for SIM_CLKDIV2 register  ***********/
#define SIM_CLKDIV2_USBDIV_SHIFT     1
#define SIM_CLKDIV2_USBDIV_MASK      ((uint32_t)((uint32_t)0x7 << SIM_CLKDIV2_USBDIV_SHIFT))
#define SIM_CLKDIV2_USBDIV(x)        ((uint32_t)(((uint32_t)(x) << SIM_CLKDIV2_USBDIV_SHIFT) & SIM_CLKDIV2_USBDIV_MASK))
#define SIM_CLKDIV2_USBFRAC          ((uint32_t)0x00000001)

#endif

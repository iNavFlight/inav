/**
  ******************************************************************************
  * @file    stm32f107xc.h
  * @author  MCD Application Team
  * @version V4.0.1
  * @date    31-July-2015
  * @brief   CMSIS Cortex-M3 Device Peripheral Access Layer Header File. 
  *          This file contains all the peripheral register's definitions, bits 
  *          definitions and memory mapping for STM32F1xx devices.            
  *            
  *          This file contains:
  *           - Data structures and the address mapping for all peripherals
  *           - Peripheral's registers declarations and bits definition
  *           - Macros to access peripheral’s registers hardware
  *  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */


/** @addtogroup CMSIS
  * @{
  */

/** @addtogroup stm32f107xc
  * @{
  */
    
#ifndef __STM32F107xC_H
#define __STM32F107xC_H

#ifdef __cplusplus
 extern "C" {
#endif 

/** @addtogroup Configuration_section_for_CMSIS
  * @{
  */
/**
  * @brief Configuration of the Cortex-M3 Processor and Core Peripherals 
 */
 #define __MPU_PRESENT             0      /*!< Other STM32 devices does not provide an MPU  */
#define __CM3_REV                 0x0200  /*!< Core Revision r2p0                           */
#define __NVIC_PRIO_BITS          4       /*!< STM32 uses 4 Bits for the Priority Levels    */
#define __Vendor_SysTickConfig    0       /*!< Set to 1 if different SysTick Config is used */

/**
  * @}
  */

/** @addtogroup Peripheral_interrupt_number_definition
  * @{
  */

/**
 * @brief STM32F10x Interrupt Number Definition, according to the selected device 
 *        in @ref Library_configuration_section 
 */

 /*!< Interrupt Number Definition */
typedef enum
{
/******  Cortex-M3 Processor Exceptions Numbers ***************************************************/
  NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt                             */
  MemoryManagement_IRQn       = -12,    /*!< 4 Cortex-M3 Memory Management Interrupt              */
  BusFault_IRQn               = -11,    /*!< 5 Cortex-M3 Bus Fault Interrupt                      */
  UsageFault_IRQn             = -10,    /*!< 6 Cortex-M3 Usage Fault Interrupt                    */
  SVCall_IRQn                 = -5,     /*!< 11 Cortex-M3 SV Call Interrupt                       */
  DebugMonitor_IRQn           = -4,     /*!< 12 Cortex-M3 Debug Monitor Interrupt                 */
  PendSV_IRQn                 = -2,     /*!< 14 Cortex-M3 Pend SV Interrupt                       */
  SysTick_IRQn                = -1,     /*!< 15 Cortex-M3 System Tick Interrupt                   */

/******  STM32 specific Interrupt Numbers *********************************************************/
  WWDG_IRQn                   = 0,      /*!< Window WatchDog Interrupt                            */
  PVD_IRQn                    = 1,      /*!< PVD through EXTI Line detection Interrupt            */
  TAMPER_IRQn                 = 2,      /*!< Tamper Interrupt                                     */
  RTC_IRQn                    = 3,      /*!< RTC global Interrupt                                 */
  FLASH_IRQn                  = 4,      /*!< FLASH global Interrupt                               */
  RCC_IRQn                    = 5,      /*!< RCC global Interrupt                                 */
  EXTI0_IRQn                  = 6,      /*!< EXTI Line0 Interrupt                                 */
  EXTI1_IRQn                  = 7,      /*!< EXTI Line1 Interrupt                                 */
  EXTI2_IRQn                  = 8,      /*!< EXTI Line2 Interrupt                                 */
  EXTI3_IRQn                  = 9,      /*!< EXTI Line3 Interrupt                                 */
  EXTI4_IRQn                  = 10,     /*!< EXTI Line4 Interrupt                                 */
  DMA1_Channel1_IRQn          = 11,     /*!< DMA1 Channel 1 global Interrupt                      */
  DMA1_Channel2_IRQn          = 12,     /*!< DMA1 Channel 2 global Interrupt                      */
  DMA1_Channel3_IRQn          = 13,     /*!< DMA1 Channel 3 global Interrupt                      */
  DMA1_Channel4_IRQn          = 14,     /*!< DMA1 Channel 4 global Interrupt                      */
  DMA1_Channel5_IRQn          = 15,     /*!< DMA1 Channel 5 global Interrupt                      */
  DMA1_Channel6_IRQn          = 16,     /*!< DMA1 Channel 6 global Interrupt                      */
  DMA1_Channel7_IRQn          = 17,     /*!< DMA1 Channel 7 global Interrupt                      */
  ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
  CAN1_TX_IRQn                = 19,     /*!< CAN1 TX Interrupts                                   */
  CAN1_RX0_IRQn               = 20,     /*!< CAN1 RX0 Interrupts                                  */
  CAN1_RX1_IRQn               = 21,     /*!< CAN1 RX1 Interrupt                                   */
  CAN1_SCE_IRQn               = 22,     /*!< CAN1 SCE Interrupt                                   */
  EXTI9_5_IRQn                = 23,     /*!< External Line[9:5] Interrupts                        */
  TIM1_BRK_IRQn               = 24,     /*!< TIM1 Break Interrupt                                 */
  TIM1_UP_IRQn                = 25,     /*!< TIM1 Update Interrupt                                */
  TIM1_TRG_COM_IRQn           = 26,     /*!< TIM1 Trigger and Commutation Interrupt               */
  TIM1_CC_IRQn                = 27,     /*!< TIM1 Capture Compare Interrupt                       */
  TIM2_IRQn                   = 28,     /*!< TIM2 global Interrupt                                */
  TIM3_IRQn                   = 29,     /*!< TIM3 global Interrupt                                */
  TIM4_IRQn                   = 30,     /*!< TIM4 global Interrupt                                */
  I2C1_EV_IRQn                = 31,     /*!< I2C1 Event Interrupt                                 */
  I2C1_ER_IRQn                = 32,     /*!< I2C1 Error Interrupt                                 */
  I2C2_EV_IRQn                = 33,     /*!< I2C2 Event Interrupt                                 */
  I2C2_ER_IRQn                = 34,     /*!< I2C2 Error Interrupt                                 */
  SPI1_IRQn                   = 35,     /*!< SPI1 global Interrupt                                */
  SPI2_IRQn                   = 36,     /*!< SPI2 global Interrupt                                */
  USART1_IRQn                 = 37,     /*!< USART1 global Interrupt                              */
  USART2_IRQn                 = 38,     /*!< USART2 global Interrupt                              */
  USART3_IRQn                 = 39,     /*!< USART3 global Interrupt                              */
  EXTI15_10_IRQn              = 40,     /*!< External Line[15:10] Interrupts                      */
  RTC_Alarm_IRQn              = 41,     /*!< RTC Alarm through EXTI Line Interrupt                */
  OTG_FS_WKUP_IRQn            = 42,     /*!< USB OTG FS WakeUp from suspend through EXTI Line Interrupt */
  TIM5_IRQn                   = 50,     /*!< TIM5 global Interrupt                                */
  SPI3_IRQn                   = 51,     /*!< SPI3 global Interrupt                                */
  UART4_IRQn                  = 52,     /*!< UART4 global Interrupt                               */
  UART5_IRQn                  = 53,     /*!< UART5 global Interrupt                               */
  TIM6_IRQn                   = 54,     /*!< TIM6 global Interrupt                                */
  TIM7_IRQn                   = 55,     /*!< TIM7 global Interrupt                                */
  DMA2_Channel1_IRQn          = 56,     /*!< DMA2 Channel 1 global Interrupt                      */
  DMA2_Channel2_IRQn          = 57,     /*!< DMA2 Channel 2 global Interrupt                      */
  DMA2_Channel3_IRQn          = 58,     /*!< DMA2 Channel 3 global Interrupt                      */
  DMA2_Channel4_IRQn          = 59,     /*!< DMA2 Channel 4 global Interrupt                      */
  DMA2_Channel5_IRQn          = 60,     /*!< DMA2 Channel 5 global Interrupt                      */
  ETH_IRQn                    = 61,     /*!< Ethernet global Interrupt                            */
  ETH_WKUP_IRQn               = 62,     /*!< Ethernet Wakeup through EXTI line Interrupt          */
  CAN2_TX_IRQn                = 63,     /*!< CAN2 TX Interrupt                                    */
  CAN2_RX0_IRQn               = 64,     /*!< CAN2 RX0 Interrupt                                   */
  CAN2_RX1_IRQn               = 65,     /*!< CAN2 RX1 Interrupt                                   */
  CAN2_SCE_IRQn               = 66,     /*!< CAN2 SCE Interrupt                                   */
  OTG_FS_IRQn                 = 67      /*!< USB OTG FS global Interrupt                          */
} IRQn_Type;


/**
  * @}
  */

#include "core_cm3.h"
#include "system_stm32f1xx.h"
#include <stdint.h>

/** @addtogroup Peripheral_registers_structures
  * @{
  */   

/** 
  * @brief Analog to Digital Converter  
  */

typedef struct
{
  __IO uint32_t SR;
  __IO uint32_t CR1;
  __IO uint32_t CR2;
  __IO uint32_t SMPR1;
  __IO uint32_t SMPR2;
  __IO uint32_t JOFR1;
  __IO uint32_t JOFR2;
  __IO uint32_t JOFR3;
  __IO uint32_t JOFR4;
  __IO uint32_t HTR;
  __IO uint32_t LTR;
  __IO uint32_t SQR1;
  __IO uint32_t SQR2;
  __IO uint32_t SQR3;
  __IO uint32_t JSQR;
  __IO uint32_t JDR1;
  __IO uint32_t JDR2;
  __IO uint32_t JDR3;
  __IO uint32_t JDR4;
  __IO uint32_t DR;
} ADC_TypeDef;

/** 
  * @brief Backup Registers  
  */

typedef struct
{
  uint32_t  RESERVED0;
  __IO uint32_t DR1;
  __IO uint32_t DR2;
  __IO uint32_t DR3;
  __IO uint32_t DR4;
  __IO uint32_t DR5;
  __IO uint32_t DR6;
  __IO uint32_t DR7;
  __IO uint32_t DR8;
  __IO uint32_t DR9;
  __IO uint32_t DR10;
  __IO uint32_t RTCCR;
  __IO uint32_t CR;
  __IO uint32_t CSR;
  uint32_t  RESERVED13[2];
  __IO uint32_t DR11;
  __IO uint32_t DR12;
  __IO uint32_t DR13;
  __IO uint32_t DR14;
  __IO uint32_t DR15;
  __IO uint32_t DR16;
  __IO uint32_t DR17;
  __IO uint32_t DR18;
  __IO uint32_t DR19;
  __IO uint32_t DR20;
  __IO uint32_t DR21;
  __IO uint32_t DR22;
  __IO uint32_t DR23;
  __IO uint32_t DR24;
  __IO uint32_t DR25;
  __IO uint32_t DR26;
  __IO uint32_t DR27;
  __IO uint32_t DR28;
  __IO uint32_t DR29;
  __IO uint32_t DR30;
  __IO uint32_t DR31;
  __IO uint32_t DR32;
  __IO uint32_t DR33;
  __IO uint32_t DR34;
  __IO uint32_t DR35;
  __IO uint32_t DR36;
  __IO uint32_t DR37;
  __IO uint32_t DR38;
  __IO uint32_t DR39;
  __IO uint32_t DR40;
  __IO uint32_t DR41;
  __IO uint32_t DR42;
} BKP_TypeDef;
  
/** 
  * @brief Controller Area Network TxMailBox 
  */

typedef struct
{
  __IO uint32_t TIR;
  __IO uint32_t TDTR;
  __IO uint32_t TDLR;
  __IO uint32_t TDHR;
} CAN_TxMailBox_TypeDef;

/** 
  * @brief Controller Area Network FIFOMailBox 
  */
  
typedef struct
{
  __IO uint32_t RIR;
  __IO uint32_t RDTR;
  __IO uint32_t RDLR;
  __IO uint32_t RDHR;
} CAN_FIFOMailBox_TypeDef;

/** 
  * @brief Controller Area Network FilterRegister 
  */
  
typedef struct
{
  __IO uint32_t FR1;
  __IO uint32_t FR2;
} CAN_FilterRegister_TypeDef;

/** 
  * @brief Controller Area Network 
  */
  
typedef struct
{
  __IO uint32_t MCR;
  __IO uint32_t MSR;
  __IO uint32_t TSR;
  __IO uint32_t RF0R;
  __IO uint32_t RF1R;
  __IO uint32_t IER;
  __IO uint32_t ESR;
  __IO uint32_t BTR;
  uint32_t  RESERVED0[88];
  CAN_TxMailBox_TypeDef sTxMailBox[3];
  CAN_FIFOMailBox_TypeDef sFIFOMailBox[2];
  uint32_t  RESERVED1[12];
  __IO uint32_t FMR;
  __IO uint32_t FM1R;
  uint32_t  RESERVED2;
  __IO uint32_t FS1R;
  uint32_t  RESERVED3;
  __IO uint32_t FFA1R;
  uint32_t  RESERVED4;
  __IO uint32_t FA1R;
  uint32_t  RESERVED5[8];
  CAN_FilterRegister_TypeDef sFilterRegister[28];
} CAN_TypeDef;

/** 
  * @brief CRC calculation unit 
  */

typedef struct
{
  __IO uint32_t DR;           /*!< CRC Data register,                           Address offset: 0x00 */
  __IO uint8_t  IDR;          /*!< CRC Independent data register,               Address offset: 0x04 */
  uint8_t       RESERVED0;    /*!< Reserved,                                    Address offset: 0x05 */
  uint16_t      RESERVED1;    /*!< Reserved,                                    Address offset: 0x06 */  
  __IO uint32_t CR;           /*!< CRC Control register,                        Address offset: 0x08 */ 
} CRC_TypeDef;

/** 
  * @brief Digital to Analog Converter
  */

typedef struct
{
  __IO uint32_t CR;
  __IO uint32_t SWTRIGR;
  __IO uint32_t DHR12R1;
  __IO uint32_t DHR12L1;
  __IO uint32_t DHR8R1;
  __IO uint32_t DHR12R2;
  __IO uint32_t DHR12L2;
  __IO uint32_t DHR8R2;
  __IO uint32_t DHR12RD;
  __IO uint32_t DHR12LD;
  __IO uint32_t DHR8RD;
  __IO uint32_t DOR1;
  __IO uint32_t DOR2;
} DAC_TypeDef;

/** 
  * @brief Debug MCU
  */

typedef struct
{
  __IO uint32_t IDCODE;
  __IO uint32_t CR;
}DBGMCU_TypeDef;

/** 
  * @brief DMA Controller
  */

typedef struct
{
  __IO uint32_t CCR;
  __IO uint32_t CNDTR;
  __IO uint32_t CPAR;
  __IO uint32_t CMAR;
} DMA_Channel_TypeDef;

typedef struct
{
  __IO uint32_t ISR;
  __IO uint32_t IFCR;
} DMA_TypeDef;



/** 
  * @brief Ethernet MAC
  */

typedef struct
{
  __IO uint32_t MACCR;
  __IO uint32_t MACFFR;
  __IO uint32_t MACHTHR;
  __IO uint32_t MACHTLR;
  __IO uint32_t MACMIIAR;
  __IO uint32_t MACMIIDR;
  __IO uint32_t MACFCR;
  __IO uint32_t MACVLANTR;             /*    8 */
       uint32_t RESERVED0[2];
  __IO uint32_t MACRWUFFR;             /*   11 */
  __IO uint32_t MACPMTCSR;
       uint32_t RESERVED1[2];
  __IO uint32_t MACSR;                 /*   15 */
  __IO uint32_t MACIMR;
  __IO uint32_t MACA0HR;
  __IO uint32_t MACA0LR;
  __IO uint32_t MACA1HR;
  __IO uint32_t MACA1LR;
  __IO uint32_t MACA2HR;
  __IO uint32_t MACA2LR;
  __IO uint32_t MACA3HR;
  __IO uint32_t MACA3LR;               /*   24 */
       uint32_t RESERVED2[40];
  __IO uint32_t MMCCR;                 /*   65 */
  __IO uint32_t MMCRIR;
  __IO uint32_t MMCTIR;
  __IO uint32_t MMCRIMR;
  __IO uint32_t MMCTIMR;               /*   69 */
       uint32_t RESERVED3[14];
  __IO uint32_t MMCTGFSCCR;            /*   84 */
  __IO uint32_t MMCTGFMSCCR;
       uint32_t RESERVED4[5];
  __IO uint32_t MMCTGFCR;
       uint32_t RESERVED5[10];
  __IO uint32_t MMCRFCECR;
  __IO uint32_t MMCRFAECR;
       uint32_t RESERVED6[10];
  __IO uint32_t MMCRGUFCR;
       uint32_t RESERVED7[334];
  __IO uint32_t PTPTSCR;
  __IO uint32_t PTPSSIR;
  __IO uint32_t PTPTSHR;
  __IO uint32_t PTPTSLR;
  __IO uint32_t PTPTSHUR;
  __IO uint32_t PTPTSLUR;
  __IO uint32_t PTPTSAR;
  __IO uint32_t PTPTTHR;
  __IO uint32_t PTPTTLR;
       uint32_t RESERVED8[567];
  __IO uint32_t DMABMR;
  __IO uint32_t DMATPDR;
  __IO uint32_t DMARPDR;
  __IO uint32_t DMARDLAR;
  __IO uint32_t DMATDLAR;
  __IO uint32_t DMASR;
  __IO uint32_t DMAOMR;
  __IO uint32_t DMAIER;
  __IO uint32_t DMAMFBOCR;
       uint32_t RESERVED9[9];
  __IO uint32_t DMACHTDR;
  __IO uint32_t DMACHRDR;
  __IO uint32_t DMACHTBAR;
  __IO uint32_t DMACHRBAR;
} ETH_TypeDef;


/** 
  * @brief External Interrupt/Event Controller
  */

typedef struct
{
  __IO uint32_t IMR;
  __IO uint32_t EMR;
  __IO uint32_t RTSR;
  __IO uint32_t FTSR;
  __IO uint32_t SWIER;
  __IO uint32_t PR;
} EXTI_TypeDef;

/** 
  * @brief FLASH Registers
  */

typedef struct
{
  __IO uint32_t ACR;
  __IO uint32_t KEYR;
  __IO uint32_t OPTKEYR;
  __IO uint32_t SR;
  __IO uint32_t CR;
  __IO uint32_t AR;
  __IO uint32_t RESERVED;
  __IO uint32_t OBR;
  __IO uint32_t WRPR;
} FLASH_TypeDef;

/** 
  * @brief Option Bytes Registers
  */
  
typedef struct
{
  __IO uint16_t RDP;
  __IO uint16_t USER;
  __IO uint16_t Data0;
  __IO uint16_t Data1;
  __IO uint16_t WRP0;
  __IO uint16_t WRP1;
  __IO uint16_t WRP2;
  __IO uint16_t WRP3;
} OB_TypeDef;

/** 
  * @brief General Purpose I/O
  */

typedef struct
{
  __IO uint32_t CRL;
  __IO uint32_t CRH;
  __IO uint32_t IDR;
  __IO uint32_t ODR;
  __IO uint32_t BSRR;
  __IO uint32_t BRR;
  __IO uint32_t LCKR;
} GPIO_TypeDef;

/** 
  * @brief Alternate Function I/O
  */

typedef struct
{
  __IO uint32_t EVCR;
  __IO uint32_t MAPR;
  __IO uint32_t EXTICR[4];
  uint32_t RESERVED0;
  __IO uint32_t MAPR2;  
} AFIO_TypeDef;
/** 
  * @brief Inter Integrated Circuit Interface
  */

typedef struct
{
  __IO uint32_t CR1;
  __IO uint32_t CR2;
  __IO uint32_t OAR1;
  __IO uint32_t OAR2;
  __IO uint32_t DR;
  __IO uint32_t SR1;
  __IO uint32_t SR2;
  __IO uint32_t CCR;
  __IO uint32_t TRISE;
} I2C_TypeDef;

/** 
  * @brief Independent WATCHDOG
  */

typedef struct
{
  __IO uint32_t KR;           /*!< Key register,                                Address offset: 0x00 */
  __IO uint32_t PR;           /*!< Prescaler register,                          Address offset: 0x04 */
  __IO uint32_t RLR;          /*!< Reload register,                             Address offset: 0x08 */
  __IO uint32_t SR;           /*!< Status register,                             Address offset: 0x0C */
} IWDG_TypeDef;

/** 
  * @brief Power Control
  */

typedef struct
{
  __IO uint32_t CR;
  __IO uint32_t CSR;
} PWR_TypeDef;

/** 
  * @brief Reset and Clock Control
  */

typedef struct
{
  __IO uint32_t CR;
  __IO uint32_t CFGR;
  __IO uint32_t CIR;
  __IO uint32_t APB2RSTR;
  __IO uint32_t APB1RSTR;
  __IO uint32_t AHBENR;
  __IO uint32_t APB2ENR;
  __IO uint32_t APB1ENR;
  __IO uint32_t BDCR;
  __IO uint32_t CSR;

  __IO uint32_t AHBRSTR;
  __IO uint32_t CFGR2;

} RCC_TypeDef;

/** 
  * @brief Real-Time Clock
  */

typedef struct
{
  __IO uint32_t CRH;
  __IO uint32_t CRL;
  __IO uint32_t PRLH;
  __IO uint32_t PRLL;
  __IO uint32_t DIVH;
  __IO uint32_t DIVL;
  __IO uint32_t CNTH;
  __IO uint32_t CNTL;
  __IO uint32_t ALRH;
  __IO uint32_t ALRL;
} RTC_TypeDef;

/** 
  * @brief SD host Interface
  */

typedef struct
{
  __IO uint32_t POWER;
  __IO uint32_t CLKCR;
  __IO uint32_t ARG;
  __IO uint32_t CMD;
  __I uint32_t RESPCMD;
  __I uint32_t RESP1;
  __I uint32_t RESP2;
  __I uint32_t RESP3;
  __I uint32_t RESP4;
  __IO uint32_t DTIMER;
  __IO uint32_t DLEN;
  __IO uint32_t DCTRL;
  __I uint32_t DCOUNT;
  __I uint32_t STA;
  __IO uint32_t ICR;
  __IO uint32_t MASK;
  uint32_t  RESERVED0[2];
  __I uint32_t FIFOCNT;
  uint32_t  RESERVED1[13];
  __IO uint32_t FIFO;
} SDIO_TypeDef;

/** 
  * @brief Serial Peripheral Interface
  */

typedef struct
{
  __IO uint32_t CR1;
  __IO uint32_t CR2;
  __IO uint32_t SR;
  __IO uint32_t DR;
  __IO uint32_t CRCPR;
  __IO uint32_t RXCRCR;
  __IO uint32_t TXCRCR;
  __IO uint32_t I2SCFGR;
  __IO uint32_t I2SPR;
} SPI_TypeDef;

/**
  * @brief TIM Timers
  */
typedef struct
{
  __IO uint32_t CR1;             /*!< TIM control register 1,                      Address offset: 0x00 */
  __IO uint32_t CR2;             /*!< TIM control register 2,                      Address offset: 0x04 */
  __IO uint32_t SMCR;            /*!< TIM slave Mode Control register,             Address offset: 0x08 */
  __IO uint32_t DIER;            /*!< TIM DMA/interrupt enable register,           Address offset: 0x0C */
  __IO uint32_t SR;              /*!< TIM status register,                         Address offset: 0x10 */
  __IO uint32_t EGR;             /*!< TIM event generation register,               Address offset: 0x14 */
  __IO uint32_t CCMR1;           /*!< TIM  capture/compare mode register 1,        Address offset: 0x18 */
  __IO uint32_t CCMR2;           /*!< TIM  capture/compare mode register 2,        Address offset: 0x1C */
  __IO uint32_t CCER;            /*!< TIM capture/compare enable register,         Address offset: 0x20 */
  __IO uint32_t CNT;             /*!< TIM counter register,                        Address offset: 0x24 */
  __IO uint32_t PSC;             /*!< TIM prescaler register,                      Address offset: 0x28 */
  __IO uint32_t ARR;             /*!< TIM auto-reload register,                    Address offset: 0x2C */
  __IO uint32_t RCR;             /*!< TIM  repetition counter register,            Address offset: 0x30 */
  __IO uint32_t CCR1;            /*!< TIM capture/compare register 1,              Address offset: 0x34 */
  __IO uint32_t CCR2;            /*!< TIM capture/compare register 2,              Address offset: 0x38 */
  __IO uint32_t CCR3;            /*!< TIM capture/compare register 3,              Address offset: 0x3C */
  __IO uint32_t CCR4;            /*!< TIM capture/compare register 4,              Address offset: 0x40 */
  __IO uint32_t BDTR;            /*!< TIM break and dead-time register,            Address offset: 0x44 */
  __IO uint32_t DCR;             /*!< TIM DMA control register,                    Address offset: 0x48 */
  __IO uint32_t DMAR;            /*!< TIM DMA address for full transfer register,  Address offset: 0x4C */
  __IO uint32_t OR;              /*!< TIM option register,                         Address offset: 0x50 */
}TIM_TypeDef;


/** 
  * @brief Universal Synchronous Asynchronous Receiver Transmitter
  */
 
typedef struct
{
  __IO uint32_t SR;         /*!< USART Status register,                   Address offset: 0x00 */
  __IO uint32_t DR;         /*!< USART Data register,                     Address offset: 0x04 */
  __IO uint32_t BRR;        /*!< USART Baud rate register,                Address offset: 0x08 */
  __IO uint32_t CR1;        /*!< USART Control register 1,                Address offset: 0x0C */
  __IO uint32_t CR2;        /*!< USART Control register 2,                Address offset: 0x10 */
  __IO uint32_t CR3;        /*!< USART Control register 3,                Address offset: 0x14 */
  __IO uint32_t GTPR;       /*!< USART Guard time and prescaler register, Address offset: 0x18 */
} USART_TypeDef;


/** 
  * @brief __USB_OTG_Core_register
  */

typedef struct
{
  __IO uint32_t GOTGCTL;              /*!<  USB_OTG Control and Status Register       Address offset: 000h */
  __IO uint32_t GOTGINT;              /*!<  USB_OTG Interrupt Register                Address offset: 004h */
  __IO uint32_t GAHBCFG;              /*!<  Core AHB Configuration Register           Address offset: 008h */
  __IO uint32_t GUSBCFG;              /*!<  Core USB Configuration Register           Address offset: 00Ch */
  __IO uint32_t GRSTCTL;              /*!<  Core Reset Register                       Address offset: 010h */
  __IO uint32_t GINTSTS;              /*!<  Core Interrupt Register                   Address offset: 014h */
  __IO uint32_t GINTMSK;              /*!<  Core Interrupt Mask Register              Address offset: 018h */
  __IO uint32_t GRXSTSR;              /*!<  Receive Sts Q Read Register               Address offset: 01Ch */
  __IO uint32_t GRXSTSP;              /*!<  Receive Sts Q Read & POP Register         Address offset: 020h */
  __IO uint32_t GRXFSIZ;              /*!< Receive FIFO Size Register                 Address offset: 024h */
  __IO uint32_t DIEPTXF0_HNPTXFSIZ;   /*!<  EP0 / Non Periodic Tx FIFO Size Register  Address offset: 028h */
  __IO uint32_t HNPTXSTS;             /*!<  Non Periodic Tx FIFO/Queue Sts reg        Address offset: 02Ch */
  uint32_t Reserved30[2];             /*!< Reserved 030h*/
  __IO uint32_t GCCFG;                /*!< General Purpose IO Register                Address offset: 038h */
  __IO uint32_t CID;                  /*!< User ID Register                           Address offset: 03Ch */
  uint32_t  Reserved40[48];           /*!< Reserved 040h-0FFh */
  __IO uint32_t HPTXFSIZ;             /*!< Host Periodic Tx FIFO Size Reg             Address offset: 100h */
  __IO uint32_t DIEPTXF[0x0F];        /*!< dev Periodic Transmit FIFO                 Address offset: 0x104 */
} USB_OTG_GlobalTypeDef;

/** 
  * @brief __device_Registers
  */

typedef struct 
{
  __IO uint32_t DCFG;                 /*!< dev Configuration Register                 Address offset: 800h*/
  __IO uint32_t DCTL;                 /*!< dev Control Register                       Address offset: 804h*/
  __IO uint32_t DSTS;                 /*!< dev Status Register (RO)                   Address offset: 808h*/
  uint32_t Reserved0C;                /*!< Reserved 80Ch*/
  __IO uint32_t DIEPMSK;              /*!< dev IN Endpoint Mask                       Address offset: 810h*/
  __IO uint32_t DOEPMSK;              /*!< dev OUT Endpoint Mask                      Address offset: 814h*/
  __IO uint32_t DAINT;                /*!< dev All Endpoints Itr Reg                  Address offset: 818h*/
  __IO uint32_t DAINTMSK;             /*!< dev All Endpoints Itr Mask                 Address offset: 81Ch*/
  uint32_t  Reserved20;               /*!< Reserved 820h*/
  uint32_t Reserved9;                 /*!< Reserved 824h*/
  __IO uint32_t DVBUSDIS;             /*!< dev VBUS discharge Register                Address offset: 828h*/
  __IO uint32_t DVBUSPULSE;           /*!< dev VBUS Pulse Register                    Address offset: 82Ch*/
  __IO uint32_t DTHRCTL;              /*!< dev thr                                    Address offset: 830h*/
  __IO uint32_t DIEPEMPMSK;           /*!< dev empty msk                              Address offset: 834h*/
  __IO uint32_t DEACHINT;             /*!< dedicated EP interrupt                     Address offset: 838h*/
  __IO uint32_t DEACHMSK;             /*!< dedicated EP msk                           Address offset: 83Ch*/  
  uint32_t Reserved40;                /*!< dedicated EP mask                          Address offset: 840h*/
  __IO uint32_t DINEP1MSK;            /*!< dedicated EP mask                          Address offset: 844h*/
  uint32_t  Reserved44[15];           /*!< Reserved 844-87Ch*/
  __IO uint32_t DOUTEP1MSK;           /*!< dedicated EP msk                           Address offset: 884h*/
} USB_OTG_DeviceTypeDef;

/** 
  * @brief __IN_Endpoint-Specific_Register
  */

typedef struct 
{
  __IO uint32_t DIEPCTL;              /*!< dev IN Endpoint Control Reg                900h + (ep_num * 20h) + 00h*/
  uint32_t Reserved04;                /*!< Reserved                                   900h + (ep_num * 20h) + 04h*/
  __IO uint32_t DIEPINT;              /*!< dev IN Endpoint Itr Reg                    900h + (ep_num * 20h) + 08h*/
  uint32_t Reserved0C;                /*!< Reserved                                   900h + (ep_num * 20h) + 0Ch*/
  __IO uint32_t DIEPTSIZ;             /*!< IN Endpoint Txfer Size                     900h + (ep_num * 20h) + 10h*/
  __IO uint32_t DIEPDMA;              /*!< IN Endpoint DMA Address Reg                900h + (ep_num * 20h) + 14h*/
  __IO uint32_t DTXFSTS;              /*!< IN Endpoint Tx FIFO Status Reg             900h + (ep_num * 20h) + 18h*/
  uint32_t Reserved18;                /*!< Reserved                                   900h+(ep_num*20h)+1Ch-900h+ (ep_num * 20h) + 1Ch*/
} USB_OTG_INEndpointTypeDef;

/** 
  * @brief __OUT_Endpoint-Specific_Registers
  */

typedef struct 
{
  __IO uint32_t DOEPCTL;              /*!< dev OUT Endpoint Control Reg               B00h + (ep_num * 20h) + 00h*/
  uint32_t Reserved04;                /*!< Reserved                                   B00h + (ep_num * 20h) + 04h*/
  __IO uint32_t DOEPINT;              /*!< dev OUT Endpoint Itr Reg                   B00h + (ep_num * 20h) + 08h*/
  uint32_t Reserved0C;                /*!< Reserved                                   B00h + (ep_num * 20h) + 0Ch*/
  __IO uint32_t DOEPTSIZ;             /*!< dev OUT Endpoint Txfer Size                B00h + (ep_num * 20h) + 10h*/
  __IO uint32_t DOEPDMA;              /*!< dev OUT Endpoint DMA Address               B00h + (ep_num * 20h) + 14h*/
  uint32_t Reserved18[2];             /*!< Reserved                                   B00h + (ep_num * 20h) + 18h - B00h + (ep_num * 20h) + 1Ch*/
} USB_OTG_OUTEndpointTypeDef;

/** 
  * @brief __Host_Mode_Register_Structures
  */

typedef struct 
{
  __IO uint32_t HCFG;                 /*!< Host Configuration Register    400h*/
  __IO uint32_t HFIR;                 /*!< Host Frame Interval Register   404h*/
  __IO uint32_t HFNUM;                /*!< Host Frame Nbr/Frame Remaining 408h*/
  uint32_t Reserved40C;               /*!< Reserved                       40Ch*/
  __IO uint32_t HPTXSTS;              /*!< Host Periodic Tx FIFO/ Queue Status 410h*/
  __IO uint32_t HAINT;                /*!< Host All Channels Interrupt Register 414h*/
  __IO uint32_t HAINTMSK;             /*!< Host All Channels Interrupt Mask 418h*/
} USB_OTG_HostTypeDef;

/** 
  * @brief __Host_Channel_Specific_Registers
  */

typedef struct
{
  __IO uint32_t HCCHAR;
  __IO uint32_t HCSPLT;
  __IO uint32_t HCINT;
  __IO uint32_t HCINTMSK;
  __IO uint32_t HCTSIZ;
  __IO uint32_t HCDMA;
  uint32_t Reserved[2];
} USB_OTG_HostChannelTypeDef;

/** 
  * @brief Window WATCHDOG
  */

typedef struct
{
  __IO uint32_t CR;   /*!< WWDG Control register,       Address offset: 0x00 */
  __IO uint32_t CFR;  /*!< WWDG Configuration register, Address offset: 0x04 */
  __IO uint32_t SR;   /*!< WWDG Status register,        Address offset: 0x08 */
} WWDG_TypeDef;

/**
  * @}
  */
  
/** @addtogroup Peripheral_memory_map
  * @{
  */


#define FLASH_BASE            ((uint32_t)0x08000000) /*!< FLASH base address in the alias region */
#define FLASH_BANK1_END       ((uint32_t)0x0803FFFF) /*!< FLASH END address of bank1 */
#define SRAM_BASE             ((uint32_t)0x20000000) /*!< SRAM base address in the alias region */
#define PERIPH_BASE           ((uint32_t)0x40000000) /*!< Peripheral base address in the alias region */

#define SRAM_BB_BASE          ((uint32_t)0x22000000) /*!< SRAM base address in the bit-band region */
#define PERIPH_BB_BASE        ((uint32_t)0x42000000) /*!< Peripheral base address in the bit-band region */


/*!< Peripheral memory map */
#define APB1PERIPH_BASE       PERIPH_BASE
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x10000)
#define AHBPERIPH_BASE        (PERIPH_BASE + 0x20000)

#define TIM2_BASE             (APB1PERIPH_BASE + 0x0000)
#define TIM3_BASE             (APB1PERIPH_BASE + 0x0400)
#define TIM4_BASE             (APB1PERIPH_BASE + 0x0800)
#define TIM5_BASE             (APB1PERIPH_BASE + 0x0C00)
#define TIM6_BASE             (APB1PERIPH_BASE + 0x1000)
#define TIM7_BASE             (APB1PERIPH_BASE + 0x1400)
#define RTC_BASE              (APB1PERIPH_BASE + 0x2800)
#define WWDG_BASE             (APB1PERIPH_BASE + 0x2C00)
#define IWDG_BASE             (APB1PERIPH_BASE + 0x3000)
#define SPI2_BASE             (APB1PERIPH_BASE + 0x3800)
#define SPI3_BASE             (APB1PERIPH_BASE + 0x3C00)
#define USART2_BASE           (APB1PERIPH_BASE + 0x4400)
#define USART3_BASE           (APB1PERIPH_BASE + 0x4800)
#define UART4_BASE            (APB1PERIPH_BASE + 0x4C00)
#define UART5_BASE            (APB1PERIPH_BASE + 0x5000)
#define I2C1_BASE             (APB1PERIPH_BASE + 0x5400)
#define I2C2_BASE             (APB1PERIPH_BASE + 0x5800)
#define CAN1_BASE             (APB1PERIPH_BASE + 0x6400)
#define CAN2_BASE             (APB1PERIPH_BASE + 0x6800)
#define BKP_BASE              (APB1PERIPH_BASE + 0x6C00)
#define PWR_BASE              (APB1PERIPH_BASE + 0x7000)
#define DAC_BASE              (APB1PERIPH_BASE + 0x7400)
#define AFIO_BASE             (APB2PERIPH_BASE + 0x0000)
#define EXTI_BASE             (APB2PERIPH_BASE + 0x0400)
#define GPIOA_BASE            (APB2PERIPH_BASE + 0x0800)
#define GPIOB_BASE            (APB2PERIPH_BASE + 0x0C00)
#define GPIOC_BASE            (APB2PERIPH_BASE + 0x1000)
#define GPIOD_BASE            (APB2PERIPH_BASE + 0x1400)
#define GPIOE_BASE            (APB2PERIPH_BASE + 0x1800)
#define ADC1_BASE             (APB2PERIPH_BASE + 0x2400)
#define ADC2_BASE             (APB2PERIPH_BASE + 0x2800)
#define TIM1_BASE             (APB2PERIPH_BASE + 0x2C00)
#define SPI1_BASE             (APB2PERIPH_BASE + 0x3000)
#define USART1_BASE           (APB2PERIPH_BASE + 0x3800)

#define SDIO_BASE             (PERIPH_BASE + 0x18000)

#define DMA1_BASE             (AHBPERIPH_BASE + 0x0000)
#define DMA1_Channel1_BASE    (AHBPERIPH_BASE + 0x0008)
#define DMA1_Channel2_BASE    (AHBPERIPH_BASE + 0x001C)
#define DMA1_Channel3_BASE    (AHBPERIPH_BASE + 0x0030)
#define DMA1_Channel4_BASE    (AHBPERIPH_BASE + 0x0044)
#define DMA1_Channel5_BASE    (AHBPERIPH_BASE + 0x0058)
#define DMA1_Channel6_BASE    (AHBPERIPH_BASE + 0x006C)
#define DMA1_Channel7_BASE    (AHBPERIPH_BASE + 0x0080)
#define DMA2_BASE             (AHBPERIPH_BASE + 0x0400)
#define DMA2_Channel1_BASE    (AHBPERIPH_BASE + 0x0408)
#define DMA2_Channel2_BASE    (AHBPERIPH_BASE + 0x041C)
#define DMA2_Channel3_BASE    (AHBPERIPH_BASE + 0x0430)
#define DMA2_Channel4_BASE    (AHBPERIPH_BASE + 0x0444)
#define DMA2_Channel5_BASE    (AHBPERIPH_BASE + 0x0458)
#define RCC_BASE              (AHBPERIPH_BASE + 0x1000)
#define CRC_BASE              (AHBPERIPH_BASE + 0x3000)

#define FLASH_R_BASE          (AHBPERIPH_BASE + 0x2000) /*!< Flash registers base address */
#define OB_BASE               ((uint32_t)0x1FFFF800)    /*!< Flash Option Bytes base address */

#define ETH_BASE              (AHBPERIPH_BASE + 0x8000)
#define ETH_MAC_BASE          (ETH_BASE)
#define ETH_MMC_BASE          (ETH_BASE + 0x0100)
#define ETH_PTP_BASE          (ETH_BASE + 0x0700)
#define ETH_DMA_BASE          (ETH_BASE + 0x1000)


#define DBGMCU_BASE          ((uint32_t)0xE0042000) /*!< Debug MCU registers base address */


/*!< USB registers base address */
#define USB_OTG_FS_PERIPH_BASE               ((uint32_t )0x50000000)

#define USB_OTG_GLOBAL_BASE                  ((uint32_t )0x00000000)
#define USB_OTG_DEVICE_BASE                  ((uint32_t )0x00000800)
#define USB_OTG_IN_ENDPOINT_BASE             ((uint32_t )0x00000900)
#define USB_OTG_OUT_ENDPOINT_BASE            ((uint32_t )0x00000B00)
#define USB_OTG_EP_REG_SIZE                  ((uint32_t )0x00000020)
#define USB_OTG_HOST_BASE                    ((uint32_t )0x00000400)
#define USB_OTG_HOST_PORT_BASE               ((uint32_t )0x00000440)
#define USB_OTG_HOST_CHANNEL_BASE            ((uint32_t )0x00000500)
#define USB_OTG_HOST_CHANNEL_SIZE            ((uint32_t )0x00000020)
#define USB_OTG_PCGCCTL_BASE                 ((uint32_t )0x00000E00)
#define USB_OTG_FIFO_BASE                    ((uint32_t )0x00001000)
#define USB_OTG_FIFO_SIZE                    ((uint32_t )0x00001000)

/**
  * @}
  */
  
/** @addtogroup Peripheral_declaration
  * @{
  */  

#define TIM2                ((TIM_TypeDef *) TIM2_BASE)
#define TIM3                ((TIM_TypeDef *) TIM3_BASE)
#define TIM4                ((TIM_TypeDef *) TIM4_BASE)
#define TIM5                ((TIM_TypeDef *) TIM5_BASE)
#define TIM6                ((TIM_TypeDef *) TIM6_BASE)
#define TIM7                ((TIM_TypeDef *) TIM7_BASE)
#define RTC                 ((RTC_TypeDef *) RTC_BASE)
#define WWDG                ((WWDG_TypeDef *) WWDG_BASE)
#define IWDG                ((IWDG_TypeDef *) IWDG_BASE)
#define SPI2                ((SPI_TypeDef *) SPI2_BASE)
#define SPI3                ((SPI_TypeDef *) SPI3_BASE)
#define USART2              ((USART_TypeDef *) USART2_BASE)
#define USART3              ((USART_TypeDef *) USART3_BASE)
#define UART4               ((USART_TypeDef *) UART4_BASE)
#define UART5               ((USART_TypeDef *) UART5_BASE)
#define I2C1                ((I2C_TypeDef *) I2C1_BASE)
#define I2C2                ((I2C_TypeDef *) I2C2_BASE)
#define CAN1                ((CAN_TypeDef *) CAN1_BASE)
#define CAN2                ((CAN_TypeDef *) CAN2_BASE)
#define BKP                 ((BKP_TypeDef *) BKP_BASE)
#define PWR                 ((PWR_TypeDef *) PWR_BASE)
#define DAC                 ((DAC_TypeDef *) DAC_BASE)
#define AFIO                ((AFIO_TypeDef *) AFIO_BASE)
#define EXTI                ((EXTI_TypeDef *) EXTI_BASE)
#define GPIOA               ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB               ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC               ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD               ((GPIO_TypeDef *) GPIOD_BASE)
#define GPIOE               ((GPIO_TypeDef *) GPIOE_BASE)
#define ADC1                ((ADC_TypeDef *) ADC1_BASE)
#define ADC2                ((ADC_TypeDef *) ADC2_BASE)
#define TIM1                ((TIM_TypeDef *) TIM1_BASE)
#define SPI1                ((SPI_TypeDef *) SPI1_BASE)
#define USART1              ((USART_TypeDef *) USART1_BASE)
#define SDIO                ((SDIO_TypeDef *) SDIO_BASE)
#define DMA1                ((DMA_TypeDef *) DMA1_BASE)
#define DMA2                ((DMA_TypeDef *) DMA2_BASE)
#define DMA1_Channel1       ((DMA_Channel_TypeDef *) DMA1_Channel1_BASE)
#define DMA1_Channel2       ((DMA_Channel_TypeDef *) DMA1_Channel2_BASE)
#define DMA1_Channel3       ((DMA_Channel_TypeDef *) DMA1_Channel3_BASE)
#define DMA1_Channel4       ((DMA_Channel_TypeDef *) DMA1_Channel4_BASE)
#define DMA1_Channel5       ((DMA_Channel_TypeDef *) DMA1_Channel5_BASE)
#define DMA1_Channel6       ((DMA_Channel_TypeDef *) DMA1_Channel6_BASE)
#define DMA1_Channel7       ((DMA_Channel_TypeDef *) DMA1_Channel7_BASE)
#define DMA2_Channel1       ((DMA_Channel_TypeDef *) DMA2_Channel1_BASE)
#define DMA2_Channel2       ((DMA_Channel_TypeDef *) DMA2_Channel2_BASE)
#define DMA2_Channel3       ((DMA_Channel_TypeDef *) DMA2_Channel3_BASE)
#define DMA2_Channel4       ((DMA_Channel_TypeDef *) DMA2_Channel4_BASE)
#define DMA2_Channel5       ((DMA_Channel_TypeDef *) DMA2_Channel5_BASE)
#define RCC                 ((RCC_TypeDef *) RCC_BASE)
#define CRC                 ((CRC_TypeDef *) CRC_BASE)
#define FLASH               ((FLASH_TypeDef *) FLASH_R_BASE)
#define OB                  ((OB_TypeDef *) OB_BASE)
#define ETH                 ((ETH_TypeDef *) ETH_BASE)
#define DBGMCU              ((DBGMCU_TypeDef *) DBGMCU_BASE)

#define USB_OTG_FS          ((USB_OTG_GlobalTypeDef *) USB_OTG_FS_PERIPH_BASE)

/**
  * @}
  */

/** @addtogroup Exported_constants
  * @{
  */
  
  /** @addtogroup Peripheral_Registers_Bits_Definition
  * @{
  */
    
/******************************************************************************/
/*                         Peripheral Registers_Bits_Definition               */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                       CRC calculation unit (CRC)                           */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for CRC_DR register  *********************/
#define  CRC_DR_DR                           ((uint32_t)0xFFFFFFFF)        /*!< Data register bits */

/*******************  Bit definition for CRC_IDR register  ********************/
#define  CRC_IDR_IDR                         ((uint32_t)0xFF)              /*!< General-purpose 8-bit data register bits */

/********************  Bit definition for CRC_CR register  ********************/
#define  CRC_CR_RESET                        ((uint32_t)0x00000001)        /*!< RESET bit */

/******************************************************************************/
/*                                                                            */
/*                             Power Control                                  */
/*                                                                            */
/******************************************************************************/

/********************  Bit definition for PWR_CR register  ********************/
#define  PWR_CR_LPDS                         ((uint32_t)0x00000001)     /*!< Low-Power Deepsleep */
#define  PWR_CR_PDDS                         ((uint32_t)0x00000002)     /*!< Power Down Deepsleep */
#define  PWR_CR_CWUF                         ((uint32_t)0x00000004)     /*!< Clear Wakeup Flag */
#define  PWR_CR_CSBF                         ((uint32_t)0x00000008)     /*!< Clear Standby Flag */
#define  PWR_CR_PVDE                         ((uint32_t)0x00000010)     /*!< Power Voltage Detector Enable */

#define  PWR_CR_PLS                          ((uint32_t)0x000000E0)     /*!< PLS[2:0] bits (PVD Level Selection) */
#define  PWR_CR_PLS_0                        ((uint32_t)0x00000020)     /*!< Bit 0 */
#define  PWR_CR_PLS_1                        ((uint32_t)0x00000040)     /*!< Bit 1 */
#define  PWR_CR_PLS_2                        ((uint32_t)0x00000080)     /*!< Bit 2 */

/*!< PVD level configuration */
#define  PWR_CR_PLS_2V2                      ((uint32_t)0x00000000)     /*!< PVD level 2.2V */
#define  PWR_CR_PLS_2V3                      ((uint32_t)0x00000020)     /*!< PVD level 2.3V */
#define  PWR_CR_PLS_2V4                      ((uint32_t)0x00000040)     /*!< PVD level 2.4V */
#define  PWR_CR_PLS_2V5                      ((uint32_t)0x00000060)     /*!< PVD level 2.5V */
#define  PWR_CR_PLS_2V6                      ((uint32_t)0x00000080)     /*!< PVD level 2.6V */
#define  PWR_CR_PLS_2V7                      ((uint32_t)0x000000A0)     /*!< PVD level 2.7V */
#define  PWR_CR_PLS_2V8                      ((uint32_t)0x000000C0)     /*!< PVD level 2.8V */
#define  PWR_CR_PLS_2V9                      ((uint32_t)0x000000E0)     /*!< PVD level 2.9V */

#define  PWR_CR_DBP                          ((uint32_t)0x00000100)     /*!< Disable Backup Domain write protection */


/*******************  Bit definition for PWR_CSR register  ********************/
#define  PWR_CSR_WUF                         ((uint32_t)0x00000001)     /*!< Wakeup Flag */
#define  PWR_CSR_SBF                         ((uint32_t)0x00000002)     /*!< Standby Flag */
#define  PWR_CSR_PVDO                        ((uint32_t)0x00000004)     /*!< PVD Output */
#define  PWR_CSR_EWUP                        ((uint32_t)0x00000100)     /*!< Enable WKUP pin */

/******************************************************************************/
/*                                                                            */
/*                            Backup registers                                */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for BKP_DR1 register  ********************/
#define  BKP_DR1_D                           ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR2 register  ********************/
#define  BKP_DR2_D                           ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR3 register  ********************/
#define  BKP_DR3_D                           ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR4 register  ********************/
#define  BKP_DR4_D                           ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR5 register  ********************/
#define  BKP_DR5_D                           ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR6 register  ********************/
#define  BKP_DR6_D                           ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR7 register  ********************/
#define  BKP_DR7_D                           ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR8 register  ********************/
#define  BKP_DR8_D                           ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR9 register  ********************/
#define  BKP_DR9_D                           ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR10 register  *******************/
#define  BKP_DR10_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR11 register  *******************/
#define  BKP_DR11_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR12 register  *******************/
#define  BKP_DR12_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR13 register  *******************/
#define  BKP_DR13_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR14 register  *******************/
#define  BKP_DR14_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR15 register  *******************/
#define  BKP_DR15_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR16 register  *******************/
#define  BKP_DR16_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR17 register  *******************/
#define  BKP_DR17_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/******************  Bit definition for BKP_DR18 register  ********************/
#define  BKP_DR18_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR19 register  *******************/
#define  BKP_DR19_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR20 register  *******************/
#define  BKP_DR20_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR21 register  *******************/
#define  BKP_DR21_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR22 register  *******************/
#define  BKP_DR22_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR23 register  *******************/
#define  BKP_DR23_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR24 register  *******************/
#define  BKP_DR24_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR25 register  *******************/
#define  BKP_DR25_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR26 register  *******************/
#define  BKP_DR26_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR27 register  *******************/
#define  BKP_DR27_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR28 register  *******************/
#define  BKP_DR28_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR29 register  *******************/
#define  BKP_DR29_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR30 register  *******************/
#define  BKP_DR30_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR31 register  *******************/
#define  BKP_DR31_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR32 register  *******************/
#define  BKP_DR32_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR33 register  *******************/
#define  BKP_DR33_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR34 register  *******************/
#define  BKP_DR34_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR35 register  *******************/
#define  BKP_DR35_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR36 register  *******************/
#define  BKP_DR36_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR37 register  *******************/
#define  BKP_DR37_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR38 register  *******************/
#define  BKP_DR38_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR39 register  *******************/
#define  BKP_DR39_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR40 register  *******************/
#define  BKP_DR40_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR41 register  *******************/
#define  BKP_DR41_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

/*******************  Bit definition for BKP_DR42 register  *******************/
#define  BKP_DR42_D                          ((uint32_t)0x0000FFFF)     /*!< Backup data */

#define RTC_BKP_NUMBER 42

/******************  Bit definition for BKP_RTCCR register  *******************/
#define  BKP_RTCCR_CAL                       ((uint32_t)0x0000007F)     /*!< Calibration value */
#define  BKP_RTCCR_CCO                       ((uint32_t)0x00000080)     /*!< Calibration Clock Output */
#define  BKP_RTCCR_ASOE                      ((uint32_t)0x00000100)     /*!< Alarm or Second Output Enable */
#define  BKP_RTCCR_ASOS                      ((uint32_t)0x00000200)     /*!< Alarm or Second Output Selection */

/********************  Bit definition for BKP_CR register  ********************/
#define  BKP_CR_TPE                          ((uint32_t)0x00000001)     /*!< TAMPER pin enable */
#define  BKP_CR_TPAL                         ((uint32_t)0x00000002)     /*!< TAMPER pin active level */

/*******************  Bit definition for BKP_CSR register  ********************/
#define  BKP_CSR_CTE                         ((uint32_t)0x00000001)     /*!< Clear Tamper event */
#define  BKP_CSR_CTI                         ((uint32_t)0x00000002)     /*!< Clear Tamper Interrupt */
#define  BKP_CSR_TPIE                        ((uint32_t)0x00000004)     /*!< TAMPER Pin interrupt enable */
#define  BKP_CSR_TEF                         ((uint32_t)0x00000100)     /*!< Tamper Event Flag */
#define  BKP_CSR_TIF                         ((uint32_t)0x00000200)     /*!< Tamper Interrupt Flag */

/******************************************************************************/
/*                                                                            */
/*                         Reset and Clock Control                            */
/*                                                                            */
/******************************************************************************/

/********************  Bit definition for RCC_CR register  ********************/
#define  RCC_CR_HSION                        ((uint32_t)0x00000001)        /*!< Internal High Speed clock enable */
#define  RCC_CR_HSIRDY                       ((uint32_t)0x00000002)        /*!< Internal High Speed clock ready flag */
#define  RCC_CR_HSITRIM                      ((uint32_t)0x000000F8)        /*!< Internal High Speed clock trimming */
#define  RCC_CR_HSICAL                       ((uint32_t)0x0000FF00)        /*!< Internal High Speed clock Calibration */
#define  RCC_CR_HSEON                        ((uint32_t)0x00010000)        /*!< External High Speed clock enable */
#define  RCC_CR_HSERDY                       ((uint32_t)0x00020000)        /*!< External High Speed clock ready flag */
#define  RCC_CR_HSEBYP                       ((uint32_t)0x00040000)        /*!< External High Speed clock Bypass */
#define  RCC_CR_CSSON                        ((uint32_t)0x00080000)        /*!< Clock Security System enable */
#define  RCC_CR_PLLON                        ((uint32_t)0x01000000)        /*!< PLL enable */
#define  RCC_CR_PLLRDY                       ((uint32_t)0x02000000)        /*!< PLL clock ready flag */

 #define  RCC_CR_PLL2ON                       ((uint32_t)0x04000000)        /*!< PLL2 enable */
 #define  RCC_CR_PLL2RDY                      ((uint32_t)0x08000000)        /*!< PLL2 clock ready flag */
 #define  RCC_CR_PLL3ON                       ((uint32_t)0x10000000)        /*!< PLL3 enable */
 #define  RCC_CR_PLL3RDY                      ((uint32_t)0x20000000)        /*!< PLL3 clock ready flag */

/*******************  Bit definition for RCC_CFGR register  *******************/
/*!< SW configuration */
#define  RCC_CFGR_SW                         ((uint32_t)0x00000003)        /*!< SW[1:0] bits (System clock Switch) */
#define  RCC_CFGR_SW_0                       ((uint32_t)0x00000001)        /*!< Bit 0 */
#define  RCC_CFGR_SW_1                       ((uint32_t)0x00000002)        /*!< Bit 1 */

#define  RCC_CFGR_SW_HSI                     ((uint32_t)0x00000000)        /*!< HSI selected as system clock */
#define  RCC_CFGR_SW_HSE                     ((uint32_t)0x00000001)        /*!< HSE selected as system clock */
#define  RCC_CFGR_SW_PLL                     ((uint32_t)0x00000002)        /*!< PLL selected as system clock */

/*!< SWS configuration */
#define  RCC_CFGR_SWS                        ((uint32_t)0x0000000C)        /*!< SWS[1:0] bits (System Clock Switch Status) */
#define  RCC_CFGR_SWS_0                      ((uint32_t)0x00000004)        /*!< Bit 0 */
#define  RCC_CFGR_SWS_1                      ((uint32_t)0x00000008)        /*!< Bit 1 */

#define  RCC_CFGR_SWS_HSI                    ((uint32_t)0x00000000)        /*!< HSI oscillator used as system clock */
#define  RCC_CFGR_SWS_HSE                    ((uint32_t)0x00000004)        /*!< HSE oscillator used as system clock */
#define  RCC_CFGR_SWS_PLL                    ((uint32_t)0x00000008)        /*!< PLL used as system clock */

/*!< HPRE configuration */
#define  RCC_CFGR_HPRE                       ((uint32_t)0x000000F0)        /*!< HPRE[3:0] bits (AHB prescaler) */
#define  RCC_CFGR_HPRE_0                     ((uint32_t)0x00000010)        /*!< Bit 0 */
#define  RCC_CFGR_HPRE_1                     ((uint32_t)0x00000020)        /*!< Bit 1 */
#define  RCC_CFGR_HPRE_2                     ((uint32_t)0x00000040)        /*!< Bit 2 */
#define  RCC_CFGR_HPRE_3                     ((uint32_t)0x00000080)        /*!< Bit 3 */

#define  RCC_CFGR_HPRE_DIV1                  ((uint32_t)0x00000000)        /*!< SYSCLK not divided */
#define  RCC_CFGR_HPRE_DIV2                  ((uint32_t)0x00000080)        /*!< SYSCLK divided by 2 */
#define  RCC_CFGR_HPRE_DIV4                  ((uint32_t)0x00000090)        /*!< SYSCLK divided by 4 */
#define  RCC_CFGR_HPRE_DIV8                  ((uint32_t)0x000000A0)        /*!< SYSCLK divided by 8 */
#define  RCC_CFGR_HPRE_DIV16                 ((uint32_t)0x000000B0)        /*!< SYSCLK divided by 16 */
#define  RCC_CFGR_HPRE_DIV64                 ((uint32_t)0x000000C0)        /*!< SYSCLK divided by 64 */
#define  RCC_CFGR_HPRE_DIV128                ((uint32_t)0x000000D0)        /*!< SYSCLK divided by 128 */
#define  RCC_CFGR_HPRE_DIV256                ((uint32_t)0x000000E0)        /*!< SYSCLK divided by 256 */
#define  RCC_CFGR_HPRE_DIV512                ((uint32_t)0x000000F0)        /*!< SYSCLK divided by 512 */

/*!< PPRE1 configuration */
#define  RCC_CFGR_PPRE1                      ((uint32_t)0x00000700)        /*!< PRE1[2:0] bits (APB1 prescaler) */
#define  RCC_CFGR_PPRE1_0                    ((uint32_t)0x00000100)        /*!< Bit 0 */
#define  RCC_CFGR_PPRE1_1                    ((uint32_t)0x00000200)        /*!< Bit 1 */
#define  RCC_CFGR_PPRE1_2                    ((uint32_t)0x00000400)        /*!< Bit 2 */

#define  RCC_CFGR_PPRE1_DIV1                 ((uint32_t)0x00000000)        /*!< HCLK not divided */
#define  RCC_CFGR_PPRE1_DIV2                 ((uint32_t)0x00000400)        /*!< HCLK divided by 2 */
#define  RCC_CFGR_PPRE1_DIV4                 ((uint32_t)0x00000500)        /*!< HCLK divided by 4 */
#define  RCC_CFGR_PPRE1_DIV8                 ((uint32_t)0x00000600)        /*!< HCLK divided by 8 */
#define  RCC_CFGR_PPRE1_DIV16                ((uint32_t)0x00000700)        /*!< HCLK divided by 16 */

/*!< PPRE2 configuration */
#define  RCC_CFGR_PPRE2                      ((uint32_t)0x00003800)        /*!< PRE2[2:0] bits (APB2 prescaler) */
#define  RCC_CFGR_PPRE2_0                    ((uint32_t)0x00000800)        /*!< Bit 0 */
#define  RCC_CFGR_PPRE2_1                    ((uint32_t)0x00001000)        /*!< Bit 1 */
#define  RCC_CFGR_PPRE2_2                    ((uint32_t)0x00002000)        /*!< Bit 2 */

#define  RCC_CFGR_PPRE2_DIV1                 ((uint32_t)0x00000000)        /*!< HCLK not divided */
#define  RCC_CFGR_PPRE2_DIV2                 ((uint32_t)0x00002000)        /*!< HCLK divided by 2 */
#define  RCC_CFGR_PPRE2_DIV4                 ((uint32_t)0x00002800)        /*!< HCLK divided by 4 */
#define  RCC_CFGR_PPRE2_DIV8                 ((uint32_t)0x00003000)        /*!< HCLK divided by 8 */
#define  RCC_CFGR_PPRE2_DIV16                ((uint32_t)0x00003800)        /*!< HCLK divided by 16 */

/*!< ADCPPRE configuration */
#define  RCC_CFGR_ADCPRE                     ((uint32_t)0x0000C000)        /*!< ADCPRE[1:0] bits (ADC prescaler) */
#define  RCC_CFGR_ADCPRE_0                   ((uint32_t)0x00004000)        /*!< Bit 0 */
#define  RCC_CFGR_ADCPRE_1                   ((uint32_t)0x00008000)        /*!< Bit 1 */

#define  RCC_CFGR_ADCPRE_DIV2                ((uint32_t)0x00000000)        /*!< PCLK2 divided by 2 */
#define  RCC_CFGR_ADCPRE_DIV4                ((uint32_t)0x00004000)        /*!< PCLK2 divided by 4 */
#define  RCC_CFGR_ADCPRE_DIV6                ((uint32_t)0x00008000)        /*!< PCLK2 divided by 6 */
#define  RCC_CFGR_ADCPRE_DIV8                ((uint32_t)0x0000C000)        /*!< PCLK2 divided by 8 */

#define  RCC_CFGR_PLLSRC                     ((uint32_t)0x00010000)        /*!< PLL entry clock source */

#define  RCC_CFGR_PLLXTPRE                   ((uint32_t)0x00020000)        /*!< HSE divider for PLL entry */

/*!< PLLMUL configuration */
#define  RCC_CFGR_PLLMULL                    ((uint32_t)0x003C0000)        /*!< PLLMUL[3:0] bits (PLL multiplication factor) */
#define  RCC_CFGR_PLLMULL_0                  ((uint32_t)0x00040000)        /*!< Bit 0 */
#define  RCC_CFGR_PLLMULL_1                  ((uint32_t)0x00080000)        /*!< Bit 1 */
#define  RCC_CFGR_PLLMULL_2                  ((uint32_t)0x00100000)        /*!< Bit 2 */
#define  RCC_CFGR_PLLMULL_3                  ((uint32_t)0x00200000)        /*!< Bit 3 */

 #define  RCC_CFGR_PLLXTPRE_PREDIV1          ((uint32_t)0x00000000)        /*!< PREDIV1 clock not divided for PLL entry */
 #define  RCC_CFGR_PLLXTPRE_PREDIV1_DIV2     ((uint32_t)0x00020000)        /*!< PREDIV1 clock divided by 2 for PLL entry */

 #define  RCC_CFGR_PLLMULL4                  ((uint32_t)0x00080000)        /*!< PLL input clock * 4 */
 #define  RCC_CFGR_PLLMULL5                  ((uint32_t)0x000C0000)        /*!< PLL input clock * 5 */
 #define  RCC_CFGR_PLLMULL6                  ((uint32_t)0x00100000)        /*!< PLL input clock * 6 */
 #define  RCC_CFGR_PLLMULL7                  ((uint32_t)0x00140000)        /*!< PLL input clock * 7 */
 #define  RCC_CFGR_PLLMULL8                  ((uint32_t)0x00180000)        /*!< PLL input clock * 8 */
 #define  RCC_CFGR_PLLMULL9                  ((uint32_t)0x001C0000)        /*!< PLL input clock * 9 */
 #define  RCC_CFGR_PLLMULL6_5                ((uint32_t)0x00340000)        /*!< PLL input clock * 6.5 */
 
 #define  RCC_CFGR_OTGFSPRE                  ((uint32_t)0x00400000)        /*!< USB OTG FS prescaler */
 
/*!< MCO configuration */
 #define  RCC_CFGR_MCO                       ((uint32_t)0x0F000000)        /*!< MCO[3:0] bits (Microcontroller Clock Output) */
 #define  RCC_CFGR_MCO_0                     ((uint32_t)0x01000000)        /*!< Bit 0 */
 #define  RCC_CFGR_MCO_1                     ((uint32_t)0x02000000)        /*!< Bit 1 */
 #define  RCC_CFGR_MCO_2                     ((uint32_t)0x04000000)        /*!< Bit 2 */
 #define  RCC_CFGR_MCO_3                     ((uint32_t)0x08000000)        /*!< Bit 3 */

 #define  RCC_CFGR_MCO_NOCLOCK               ((uint32_t)0x00000000)        /*!< No clock */
 #define  RCC_CFGR_MCO_SYSCLK                ((uint32_t)0x04000000)        /*!< System clock selected as MCO source */
 #define  RCC_CFGR_MCO_HSI                   ((uint32_t)0x05000000)        /*!< HSI clock selected as MCO source */
 #define  RCC_CFGR_MCO_HSE                   ((uint32_t)0x06000000)        /*!< HSE clock selected as MCO source */
 #define  RCC_CFGR_MCO_PLLCLK_DIV2           ((uint32_t)0x07000000)        /*!< PLL clock divided by 2 selected as MCO source */
 #define  RCC_CFGR_MCO_PLL2CLK               ((uint32_t)0x08000000)        /*!< PLL2 clock selected as MCO source*/
 #define  RCC_CFGR_MCO_PLL3CLK_DIV2          ((uint32_t)0x09000000)        /*!< PLL3 clock divided by 2 selected as MCO source*/
 #define  RCC_CFGR_MCO_EXT_HSE               ((uint32_t)0x0A000000)        /*!< XT1 external 3-25 MHz oscillator clock selected as MCO source */
 #define  RCC_CFGR_MCO_PLL3CLK               ((uint32_t)0x0B000000)        /*!< PLL3 clock selected as MCO source */

/*!<******************  Bit definition for RCC_CIR register  ********************/
#define  RCC_CIR_LSIRDYF                     ((uint32_t)0x00000001)        /*!< LSI Ready Interrupt flag */
#define  RCC_CIR_LSERDYF                     ((uint32_t)0x00000002)        /*!< LSE Ready Interrupt flag */
#define  RCC_CIR_HSIRDYF                     ((uint32_t)0x00000004)        /*!< HSI Ready Interrupt flag */
#define  RCC_CIR_HSERDYF                     ((uint32_t)0x00000008)        /*!< HSE Ready Interrupt flag */
#define  RCC_CIR_PLLRDYF                     ((uint32_t)0x00000010)        /*!< PLL Ready Interrupt flag */
#define  RCC_CIR_CSSF                        ((uint32_t)0x00000080)        /*!< Clock Security System Interrupt flag */
#define  RCC_CIR_LSIRDYIE                    ((uint32_t)0x00000100)        /*!< LSI Ready Interrupt Enable */
#define  RCC_CIR_LSERDYIE                    ((uint32_t)0x00000200)        /*!< LSE Ready Interrupt Enable */
#define  RCC_CIR_HSIRDYIE                    ((uint32_t)0x00000400)        /*!< HSI Ready Interrupt Enable */
#define  RCC_CIR_HSERDYIE                    ((uint32_t)0x00000800)        /*!< HSE Ready Interrupt Enable */
#define  RCC_CIR_PLLRDYIE                    ((uint32_t)0x00001000)        /*!< PLL Ready Interrupt Enable */
#define  RCC_CIR_LSIRDYC                     ((uint32_t)0x00010000)        /*!< LSI Ready Interrupt Clear */
#define  RCC_CIR_LSERDYC                     ((uint32_t)0x00020000)        /*!< LSE Ready Interrupt Clear */
#define  RCC_CIR_HSIRDYC                     ((uint32_t)0x00040000)        /*!< HSI Ready Interrupt Clear */
#define  RCC_CIR_HSERDYC                     ((uint32_t)0x00080000)        /*!< HSE Ready Interrupt Clear */
#define  RCC_CIR_PLLRDYC                     ((uint32_t)0x00100000)        /*!< PLL Ready Interrupt Clear */
#define  RCC_CIR_CSSC                        ((uint32_t)0x00800000)        /*!< Clock Security System Interrupt Clear */

 #define  RCC_CIR_PLL2RDYF                    ((uint32_t)0x00000020)        /*!< PLL2 Ready Interrupt flag */
 #define  RCC_CIR_PLL3RDYF                    ((uint32_t)0x00000040)        /*!< PLL3 Ready Interrupt flag */
 #define  RCC_CIR_PLL2RDYIE                   ((uint32_t)0x00002000)        /*!< PLL2 Ready Interrupt Enable */
 #define  RCC_CIR_PLL3RDYIE                   ((uint32_t)0x00004000)        /*!< PLL3 Ready Interrupt Enable */
 #define  RCC_CIR_PLL2RDYC                    ((uint32_t)0x00200000)        /*!< PLL2 Ready Interrupt Clear */
 #define  RCC_CIR_PLL3RDYC                    ((uint32_t)0x00400000)        /*!< PLL3 Ready Interrupt Clear */

/*****************  Bit definition for RCC_APB2RSTR register  *****************/
#define  RCC_APB2RSTR_AFIORST                ((uint32_t)0x00000001)        /*!< Alternate Function I/O reset */
#define  RCC_APB2RSTR_IOPARST                ((uint32_t)0x00000004)        /*!< I/O port A reset */
#define  RCC_APB2RSTR_IOPBRST                ((uint32_t)0x00000008)        /*!< I/O port B reset */
#define  RCC_APB2RSTR_IOPCRST                ((uint32_t)0x00000010)        /*!< I/O port C reset */
#define  RCC_APB2RSTR_IOPDRST                ((uint32_t)0x00000020)        /*!< I/O port D reset */
#define  RCC_APB2RSTR_ADC1RST                ((uint32_t)0x00000200)        /*!< ADC 1 interface reset */

#define  RCC_APB2RSTR_ADC2RST                ((uint32_t)0x00000400)        /*!< ADC 2 interface reset */

#define  RCC_APB2RSTR_TIM1RST                ((uint32_t)0x00000800)        /*!< TIM1 Timer reset */
#define  RCC_APB2RSTR_SPI1RST                ((uint32_t)0x00001000)        /*!< SPI 1 reset */
#define  RCC_APB2RSTR_USART1RST              ((uint32_t)0x00004000)        /*!< USART1 reset */


#define  RCC_APB2RSTR_IOPERST               ((uint32_t)0x00000040)        /*!< I/O port E reset */




/*****************  Bit definition for RCC_APB1RSTR register  *****************/
#define  RCC_APB1RSTR_TIM2RST                ((uint32_t)0x00000001)        /*!< Timer 2 reset */
#define  RCC_APB1RSTR_TIM3RST                ((uint32_t)0x00000002)        /*!< Timer 3 reset */
#define  RCC_APB1RSTR_WWDGRST                ((uint32_t)0x00000800)        /*!< Window Watchdog reset */
#define  RCC_APB1RSTR_USART2RST              ((uint32_t)0x00020000)        /*!< USART 2 reset */
#define  RCC_APB1RSTR_I2C1RST                ((uint32_t)0x00200000)        /*!< I2C 1 reset */

#define  RCC_APB1RSTR_CAN1RST                ((uint32_t)0x02000000)        /*!< CAN1 reset */

#define  RCC_APB1RSTR_BKPRST                 ((uint32_t)0x08000000)        /*!< Backup interface reset */
#define  RCC_APB1RSTR_PWRRST                 ((uint32_t)0x10000000)        /*!< Power interface reset */

#define  RCC_APB1RSTR_TIM4RST               ((uint32_t)0x00000004)        /*!< Timer 4 reset */
#define  RCC_APB1RSTR_SPI2RST               ((uint32_t)0x00004000)        /*!< SPI 2 reset */
#define  RCC_APB1RSTR_USART3RST             ((uint32_t)0x00040000)        /*!< USART 3 reset */
#define  RCC_APB1RSTR_I2C2RST               ((uint32_t)0x00400000)        /*!< I2C 2 reset */


#define  RCC_APB1RSTR_TIM5RST                ((uint32_t)0x00000008)        /*!< Timer 5 reset */
#define  RCC_APB1RSTR_TIM6RST                ((uint32_t)0x00000010)        /*!< Timer 6 reset */
#define  RCC_APB1RSTR_TIM7RST                ((uint32_t)0x00000020)        /*!< Timer 7 reset */
#define  RCC_APB1RSTR_SPI3RST                ((uint32_t)0x00008000)        /*!< SPI 3 reset */
#define  RCC_APB1RSTR_UART4RST               ((uint32_t)0x00080000)        /*!< UART 4 reset */
#define  RCC_APB1RSTR_UART5RST               ((uint32_t)0x00100000)        /*!< UART 5 reset */



#define  RCC_APB1RSTR_CAN2RST                ((uint32_t)0x04000000)        /*!< CAN2 reset */

#define  RCC_APB1RSTR_DACRST                 ((uint32_t)0x20000000)        /*!< DAC interface reset */

/******************  Bit definition for RCC_AHBENR register  ******************/
#define  RCC_AHBENR_DMA1EN                   ((uint32_t)0x00000001)            /*!< DMA1 clock enable */
#define  RCC_AHBENR_SRAMEN                   ((uint32_t)0x00000004)            /*!< SRAM interface clock enable */
#define  RCC_AHBENR_FLITFEN                  ((uint32_t)0x00000010)            /*!< FLITF clock enable */
#define  RCC_AHBENR_CRCEN                    ((uint32_t)0x00000040)            /*!< CRC clock enable */

 #define  RCC_AHBENR_DMA2EN                  ((uint32_t)0x00000002)            /*!< DMA2 clock enable */


 #define  RCC_AHBENR_OTGFSEN                 ((uint32_t)0x00001000)         /*!< USB OTG FS clock enable */
 #define  RCC_AHBENR_ETHMACEN                ((uint32_t)0x00004000)         /*!< ETHERNET MAC clock enable */
 #define  RCC_AHBENR_ETHMACTXEN              ((uint32_t)0x00008000)         /*!< ETHERNET MAC Tx clock enable */
 #define  RCC_AHBENR_ETHMACRXEN              ((uint32_t)0x00010000)         /*!< ETHERNET MAC Rx clock enable */

/******************  Bit definition for RCC_APB2ENR register  *****************/
#define  RCC_APB2ENR_AFIOEN                  ((uint32_t)0x00000001)         /*!< Alternate Function I/O clock enable */
#define  RCC_APB2ENR_IOPAEN                  ((uint32_t)0x00000004)         /*!< I/O port A clock enable */
#define  RCC_APB2ENR_IOPBEN                  ((uint32_t)0x00000008)         /*!< I/O port B clock enable */
#define  RCC_APB2ENR_IOPCEN                  ((uint32_t)0x00000010)         /*!< I/O port C clock enable */
#define  RCC_APB2ENR_IOPDEN                  ((uint32_t)0x00000020)         /*!< I/O port D clock enable */
#define  RCC_APB2ENR_ADC1EN                  ((uint32_t)0x00000200)         /*!< ADC 1 interface clock enable */

#define  RCC_APB2ENR_ADC2EN                  ((uint32_t)0x00000400)         /*!< ADC 2 interface clock enable */

#define  RCC_APB2ENR_TIM1EN                  ((uint32_t)0x00000800)         /*!< TIM1 Timer clock enable */
#define  RCC_APB2ENR_SPI1EN                  ((uint32_t)0x00001000)         /*!< SPI 1 clock enable */
#define  RCC_APB2ENR_USART1EN                ((uint32_t)0x00004000)         /*!< USART1 clock enable */


#define  RCC_APB2ENR_IOPEEN                 ((uint32_t)0x00000040)         /*!< I/O port E clock enable */




/*****************  Bit definition for RCC_APB1ENR register  ******************/
#define  RCC_APB1ENR_TIM2EN                  ((uint32_t)0x00000001)        /*!< Timer 2 clock enabled*/
#define  RCC_APB1ENR_TIM3EN                  ((uint32_t)0x00000002)        /*!< Timer 3 clock enable */
#define  RCC_APB1ENR_WWDGEN                  ((uint32_t)0x00000800)        /*!< Window Watchdog clock enable */
#define  RCC_APB1ENR_USART2EN                ((uint32_t)0x00020000)        /*!< USART 2 clock enable */
#define  RCC_APB1ENR_I2C1EN                  ((uint32_t)0x00200000)        /*!< I2C 1 clock enable */

#define  RCC_APB1ENR_CAN1EN                  ((uint32_t)0x02000000)        /*!< CAN1 clock enable */

#define  RCC_APB1ENR_BKPEN                   ((uint32_t)0x08000000)        /*!< Backup interface clock enable */
#define  RCC_APB1ENR_PWREN                   ((uint32_t)0x10000000)        /*!< Power interface clock enable */

#define  RCC_APB1ENR_TIM4EN                 ((uint32_t)0x00000004)        /*!< Timer 4 clock enable */
#define  RCC_APB1ENR_SPI2EN                 ((uint32_t)0x00004000)        /*!< SPI 2 clock enable */
#define  RCC_APB1ENR_USART3EN               ((uint32_t)0x00040000)        /*!< USART 3 clock enable */
#define  RCC_APB1ENR_I2C2EN                 ((uint32_t)0x00400000)        /*!< I2C 2 clock enable */


#define  RCC_APB1ENR_TIM5EN                 ((uint32_t)0x00000008)        /*!< Timer 5 clock enable */
#define  RCC_APB1ENR_TIM6EN                 ((uint32_t)0x00000010)        /*!< Timer 6 clock enable */
#define  RCC_APB1ENR_TIM7EN                 ((uint32_t)0x00000020)        /*!< Timer 7 clock enable */
#define  RCC_APB1ENR_SPI3EN                 ((uint32_t)0x00008000)        /*!< SPI 3 clock enable */
#define  RCC_APB1ENR_UART4EN                ((uint32_t)0x00080000)        /*!< UART 4 clock enable */
#define  RCC_APB1ENR_UART5EN                ((uint32_t)0x00100000)        /*!< UART 5 clock enable */



#define  RCC_APB1ENR_CAN2EN                  ((uint32_t)0x04000000)        /*!< CAN2 clock enable */

#define  RCC_APB1ENR_DACEN                  ((uint32_t)0x20000000)        /*!< DAC interface clock enable */

/*******************  Bit definition for RCC_BDCR register  *******************/
#define  RCC_BDCR_LSEON                      ((uint32_t)0x00000001)        /*!< External Low Speed oscillator enable */
#define  RCC_BDCR_LSERDY                     ((uint32_t)0x00000002)        /*!< External Low Speed oscillator Ready */
#define  RCC_BDCR_LSEBYP                     ((uint32_t)0x00000004)        /*!< External Low Speed oscillator Bypass */

#define  RCC_BDCR_RTCSEL                     ((uint32_t)0x00000300)        /*!< RTCSEL[1:0] bits (RTC clock source selection) */
#define  RCC_BDCR_RTCSEL_0                   ((uint32_t)0x00000100)        /*!< Bit 0 */
#define  RCC_BDCR_RTCSEL_1                   ((uint32_t)0x00000200)        /*!< Bit 1 */

/*!< RTC congiguration */
#define  RCC_BDCR_RTCSEL_NOCLOCK             ((uint32_t)0x00000000)        /*!< No clock */
#define  RCC_BDCR_RTCSEL_LSE                 ((uint32_t)0x00000100)        /*!< LSE oscillator clock used as RTC clock */
#define  RCC_BDCR_RTCSEL_LSI                 ((uint32_t)0x00000200)        /*!< LSI oscillator clock used as RTC clock */
#define  RCC_BDCR_RTCSEL_HSE                 ((uint32_t)0x00000300)        /*!< HSE oscillator clock divided by 128 used as RTC clock */

#define  RCC_BDCR_RTCEN                      ((uint32_t)0x00008000)        /*!< RTC clock enable */
#define  RCC_BDCR_BDRST                      ((uint32_t)0x00010000)        /*!< Backup domain software reset  */

/*******************  Bit definition for RCC_CSR register  ********************/  
#define  RCC_CSR_LSION                       ((uint32_t)0x00000001)        /*!< Internal Low Speed oscillator enable */
#define  RCC_CSR_LSIRDY                      ((uint32_t)0x00000002)        /*!< Internal Low Speed oscillator Ready */
#define  RCC_CSR_RMVF                        ((uint32_t)0x01000000)        /*!< Remove reset flag */
#define  RCC_CSR_PINRSTF                     ((uint32_t)0x04000000)        /*!< PIN reset flag */
#define  RCC_CSR_PORRSTF                     ((uint32_t)0x08000000)        /*!< POR/PDR reset flag */
#define  RCC_CSR_SFTRSTF                     ((uint32_t)0x10000000)        /*!< Software Reset flag */
#define  RCC_CSR_IWDGRSTF                    ((uint32_t)0x20000000)        /*!< Independent Watchdog reset flag */
#define  RCC_CSR_WWDGRSTF                    ((uint32_t)0x40000000)        /*!< Window watchdog reset flag */
#define  RCC_CSR_LPWRRSTF                    ((uint32_t)0x80000000)        /*!< Low-Power reset flag */

/*******************  Bit definition for RCC_AHBRSTR register  ****************/
#define  RCC_AHBRSTR_OTGFSRST               ((uint32_t)0x00001000)         /*!< USB OTG FS reset */
#define  RCC_AHBRSTR_ETHMACRST              ((uint32_t)0x00004000)         /*!< ETHERNET MAC reset */

/*******************  Bit definition for RCC_CFGR2 register  ******************/
/*!< PREDIV1 configuration */
#define  RCC_CFGR2_PREDIV1                  ((uint32_t)0x0000000F)        /*!< PREDIV1[3:0] bits */
#define  RCC_CFGR2_PREDIV1_0                ((uint32_t)0x00000001)        /*!< Bit 0 */
#define  RCC_CFGR2_PREDIV1_1                ((uint32_t)0x00000002)        /*!< Bit 1 */
#define  RCC_CFGR2_PREDIV1_2                ((uint32_t)0x00000004)        /*!< Bit 2 */
#define  RCC_CFGR2_PREDIV1_3                ((uint32_t)0x00000008)        /*!< Bit 3 */

#define  RCC_CFGR2_PREDIV1_DIV1             ((uint32_t)0x00000000)        /*!< PREDIV1 input clock not divided */
#define  RCC_CFGR2_PREDIV1_DIV2             ((uint32_t)0x00000001)        /*!< PREDIV1 input clock divided by 2 */
#define  RCC_CFGR2_PREDIV1_DIV3             ((uint32_t)0x00000002)        /*!< PREDIV1 input clock divided by 3 */
#define  RCC_CFGR2_PREDIV1_DIV4             ((uint32_t)0x00000003)        /*!< PREDIV1 input clock divided by 4 */
#define  RCC_CFGR2_PREDIV1_DIV5             ((uint32_t)0x00000004)        /*!< PREDIV1 input clock divided by 5 */
#define  RCC_CFGR2_PREDIV1_DIV6             ((uint32_t)0x00000005)        /*!< PREDIV1 input clock divided by 6 */
#define  RCC_CFGR2_PREDIV1_DIV7             ((uint32_t)0x00000006)        /*!< PREDIV1 input clock divided by 7 */
#define  RCC_CFGR2_PREDIV1_DIV8             ((uint32_t)0x00000007)        /*!< PREDIV1 input clock divided by 8 */
#define  RCC_CFGR2_PREDIV1_DIV9             ((uint32_t)0x00000008)        /*!< PREDIV1 input clock divided by 9 */
#define  RCC_CFGR2_PREDIV1_DIV10            ((uint32_t)0x00000009)        /*!< PREDIV1 input clock divided by 10 */
#define  RCC_CFGR2_PREDIV1_DIV11            ((uint32_t)0x0000000A)        /*!< PREDIV1 input clock divided by 11 */
#define  RCC_CFGR2_PREDIV1_DIV12            ((uint32_t)0x0000000B)        /*!< PREDIV1 input clock divided by 12 */
#define  RCC_CFGR2_PREDIV1_DIV13            ((uint32_t)0x0000000C)        /*!< PREDIV1 input clock divided by 13 */
#define  RCC_CFGR2_PREDIV1_DIV14            ((uint32_t)0x0000000D)        /*!< PREDIV1 input clock divided by 14 */
#define  RCC_CFGR2_PREDIV1_DIV15            ((uint32_t)0x0000000E)        /*!< PREDIV1 input clock divided by 15 */
#define  RCC_CFGR2_PREDIV1_DIV16            ((uint32_t)0x0000000F)        /*!< PREDIV1 input clock divided by 16 */

/*!< PREDIV2 configuration */
#define  RCC_CFGR2_PREDIV2                  ((uint32_t)0x000000F0)        /*!< PREDIV2[3:0] bits */
#define  RCC_CFGR2_PREDIV2_0                ((uint32_t)0x00000010)        /*!< Bit 0 */
#define  RCC_CFGR2_PREDIV2_1                ((uint32_t)0x00000020)        /*!< Bit 1 */
#define  RCC_CFGR2_PREDIV2_2                ((uint32_t)0x00000040)        /*!< Bit 2 */
#define  RCC_CFGR2_PREDIV2_3                ((uint32_t)0x00000080)        /*!< Bit 3 */

#define  RCC_CFGR2_PREDIV2_DIV1             ((uint32_t)0x00000000)        /*!< PREDIV2 input clock not divided */
#define  RCC_CFGR2_PREDIV2_DIV2             ((uint32_t)0x00000010)        /*!< PREDIV2 input clock divided by 2 */
#define  RCC_CFGR2_PREDIV2_DIV3             ((uint32_t)0x00000020)        /*!< PREDIV2 input clock divided by 3 */
#define  RCC_CFGR2_PREDIV2_DIV4             ((uint32_t)0x00000030)        /*!< PREDIV2 input clock divided by 4 */
#define  RCC_CFGR2_PREDIV2_DIV5             ((uint32_t)0x00000040)        /*!< PREDIV2 input clock divided by 5 */
#define  RCC_CFGR2_PREDIV2_DIV6             ((uint32_t)0x00000050)        /*!< PREDIV2 input clock divided by 6 */
#define  RCC_CFGR2_PREDIV2_DIV7             ((uint32_t)0x00000060)        /*!< PREDIV2 input clock divided by 7 */
#define  RCC_CFGR2_PREDIV2_DIV8             ((uint32_t)0x00000070)        /*!< PREDIV2 input clock divided by 8 */
#define  RCC_CFGR2_PREDIV2_DIV9             ((uint32_t)0x00000080)        /*!< PREDIV2 input clock divided by 9 */
#define  RCC_CFGR2_PREDIV2_DIV10            ((uint32_t)0x00000090)        /*!< PREDIV2 input clock divided by 10 */
#define  RCC_CFGR2_PREDIV2_DIV11            ((uint32_t)0x000000A0)        /*!< PREDIV2 input clock divided by 11 */
#define  RCC_CFGR2_PREDIV2_DIV12            ((uint32_t)0x000000B0)        /*!< PREDIV2 input clock divided by 12 */
#define  RCC_CFGR2_PREDIV2_DIV13            ((uint32_t)0x000000C0)        /*!< PREDIV2 input clock divided by 13 */
#define  RCC_CFGR2_PREDIV2_DIV14            ((uint32_t)0x000000D0)        /*!< PREDIV2 input clock divided by 14 */
#define  RCC_CFGR2_PREDIV2_DIV15            ((uint32_t)0x000000E0)        /*!< PREDIV2 input clock divided by 15 */
#define  RCC_CFGR2_PREDIV2_DIV16            ((uint32_t)0x000000F0)        /*!< PREDIV2 input clock divided by 16 */

/*!< PLL2MUL configuration */
#define  RCC_CFGR2_PLL2MUL                  ((uint32_t)0x00000F00)        /*!< PLL2MUL[3:0] bits */
#define  RCC_CFGR2_PLL2MUL_0                ((uint32_t)0x00000100)        /*!< Bit 0 */
#define  RCC_CFGR2_PLL2MUL_1                ((uint32_t)0x00000200)        /*!< Bit 1 */
#define  RCC_CFGR2_PLL2MUL_2                ((uint32_t)0x00000400)        /*!< Bit 2 */
#define  RCC_CFGR2_PLL2MUL_3                ((uint32_t)0x00000800)        /*!< Bit 3 */

#define  RCC_CFGR2_PLL2MUL8                 ((uint32_t)0x00000600)        /*!< PLL2 input clock * 8 */
#define  RCC_CFGR2_PLL2MUL9                 ((uint32_t)0x00000700)        /*!< PLL2 input clock * 9 */
#define  RCC_CFGR2_PLL2MUL10                ((uint32_t)0x00000800)        /*!< PLL2 input clock * 10 */
#define  RCC_CFGR2_PLL2MUL11                ((uint32_t)0x00000900)        /*!< PLL2 input clock * 11 */
#define  RCC_CFGR2_PLL2MUL12                ((uint32_t)0x00000A00)        /*!< PLL2 input clock * 12 */
#define  RCC_CFGR2_PLL2MUL13                ((uint32_t)0x00000B00)        /*!< PLL2 input clock * 13 */
#define  RCC_CFGR2_PLL2MUL14                ((uint32_t)0x00000C00)        /*!< PLL2 input clock * 14 */
#define  RCC_CFGR2_PLL2MUL16                ((uint32_t)0x00000E00)        /*!< PLL2 input clock * 16 */
#define  RCC_CFGR2_PLL2MUL20                ((uint32_t)0x00000F00)        /*!< PLL2 input clock * 20 */

/*!< PLL3MUL configuration */
#define  RCC_CFGR2_PLL3MUL                  ((uint32_t)0x0000F000)        /*!< PLL3MUL[3:0] bits */
#define  RCC_CFGR2_PLL3MUL_0                ((uint32_t)0x00001000)        /*!< Bit 0 */
#define  RCC_CFGR2_PLL3MUL_1                ((uint32_t)0x00002000)        /*!< Bit 1 */
#define  RCC_CFGR2_PLL3MUL_2                ((uint32_t)0x00004000)        /*!< Bit 2 */
#define  RCC_CFGR2_PLL3MUL_3                ((uint32_t)0x00008000)        /*!< Bit 3 */

#define  RCC_CFGR2_PLL3MUL8                 ((uint32_t)0x00006000)        /*!< PLL3 input clock * 8 */
#define  RCC_CFGR2_PLL3MUL9                 ((uint32_t)0x00007000)        /*!< PLL3 input clock * 9 */
#define  RCC_CFGR2_PLL3MUL10                ((uint32_t)0x00008000)        /*!< PLL3 input clock * 10 */
#define  RCC_CFGR2_PLL3MUL11                ((uint32_t)0x00009000)        /*!< PLL3 input clock * 11 */
#define  RCC_CFGR2_PLL3MUL12                ((uint32_t)0x0000A000)        /*!< PLL3 input clock * 12 */
#define  RCC_CFGR2_PLL3MUL13                ((uint32_t)0x0000B000)        /*!< PLL3 input clock * 13 */
#define  RCC_CFGR2_PLL3MUL14                ((uint32_t)0x0000C000)        /*!< PLL3 input clock * 14 */
#define  RCC_CFGR2_PLL3MUL16                ((uint32_t)0x0000E000)        /*!< PLL3 input clock * 16 */
#define  RCC_CFGR2_PLL3MUL20                ((uint32_t)0x0000F000)        /*!< PLL3 input clock * 20 */

#define  RCC_CFGR2_PREDIV1SRC               ((uint32_t)0x00010000)        /*!< PREDIV1 entry clock source */
#define  RCC_CFGR2_PREDIV1SRC_PLL2          ((uint32_t)0x00010000)        /*!< PLL2 selected as PREDIV1 entry clock source */
#define  RCC_CFGR2_PREDIV1SRC_HSE           ((uint32_t)0x00000000)        /*!< HSE selected as PREDIV1 entry clock source */
#define  RCC_CFGR2_I2S2SRC                  ((uint32_t)0x00020000)        /*!< I2S2 entry clock source */
#define  RCC_CFGR2_I2S3SRC                  ((uint32_t)0x00040000)        /*!< I2S3 clock source */

 
/******************************************************************************/
/*                                                                            */
/*                General Purpose and Alternate Function I/O                  */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for GPIO_CRL register  *******************/
#define  GPIO_CRL_MODE                       ((uint32_t)0x33333333)        /*!< Port x mode bits */

#define  GPIO_CRL_MODE0                      ((uint32_t)0x00000003)        /*!< MODE0[1:0] bits (Port x mode bits, pin 0) */
#define  GPIO_CRL_MODE0_0                    ((uint32_t)0x00000001)        /*!< Bit 0 */
#define  GPIO_CRL_MODE0_1                    ((uint32_t)0x00000002)        /*!< Bit 1 */

#define  GPIO_CRL_MODE1                      ((uint32_t)0x00000030)        /*!< MODE1[1:0] bits (Port x mode bits, pin 1) */
#define  GPIO_CRL_MODE1_0                    ((uint32_t)0x00000010)        /*!< Bit 0 */
#define  GPIO_CRL_MODE1_1                    ((uint32_t)0x00000020)        /*!< Bit 1 */

#define  GPIO_CRL_MODE2                      ((uint32_t)0x00000300)        /*!< MODE2[1:0] bits (Port x mode bits, pin 2) */
#define  GPIO_CRL_MODE2_0                    ((uint32_t)0x00000100)        /*!< Bit 0 */
#define  GPIO_CRL_MODE2_1                    ((uint32_t)0x00000200)        /*!< Bit 1 */

#define  GPIO_CRL_MODE3                      ((uint32_t)0x00003000)        /*!< MODE3[1:0] bits (Port x mode bits, pin 3) */
#define  GPIO_CRL_MODE3_0                    ((uint32_t)0x00001000)        /*!< Bit 0 */
#define  GPIO_CRL_MODE3_1                    ((uint32_t)0x00002000)        /*!< Bit 1 */

#define  GPIO_CRL_MODE4                      ((uint32_t)0x00030000)        /*!< MODE4[1:0] bits (Port x mode bits, pin 4) */
#define  GPIO_CRL_MODE4_0                    ((uint32_t)0x00010000)        /*!< Bit 0 */
#define  GPIO_CRL_MODE4_1                    ((uint32_t)0x00020000)        /*!< Bit 1 */

#define  GPIO_CRL_MODE5                      ((uint32_t)0x00300000)        /*!< MODE5[1:0] bits (Port x mode bits, pin 5) */
#define  GPIO_CRL_MODE5_0                    ((uint32_t)0x00100000)        /*!< Bit 0 */
#define  GPIO_CRL_MODE5_1                    ((uint32_t)0x00200000)        /*!< Bit 1 */

#define  GPIO_CRL_MODE6                      ((uint32_t)0x03000000)        /*!< MODE6[1:0] bits (Port x mode bits, pin 6) */
#define  GPIO_CRL_MODE6_0                    ((uint32_t)0x01000000)        /*!< Bit 0 */
#define  GPIO_CRL_MODE6_1                    ((uint32_t)0x02000000)        /*!< Bit 1 */

#define  GPIO_CRL_MODE7                      ((uint32_t)0x30000000)        /*!< MODE7[1:0] bits (Port x mode bits, pin 7) */
#define  GPIO_CRL_MODE7_0                    ((uint32_t)0x10000000)        /*!< Bit 0 */
#define  GPIO_CRL_MODE7_1                    ((uint32_t)0x20000000)        /*!< Bit 1 */

#define  GPIO_CRL_CNF                        ((uint32_t)0xCCCCCCCC)        /*!< Port x configuration bits */

#define  GPIO_CRL_CNF0                       ((uint32_t)0x0000000C)        /*!< CNF0[1:0] bits (Port x configuration bits, pin 0) */
#define  GPIO_CRL_CNF0_0                     ((uint32_t)0x00000004)        /*!< Bit 0 */
#define  GPIO_CRL_CNF0_1                     ((uint32_t)0x00000008)        /*!< Bit 1 */

#define  GPIO_CRL_CNF1                       ((uint32_t)0x000000C0)        /*!< CNF1[1:0] bits (Port x configuration bits, pin 1) */
#define  GPIO_CRL_CNF1_0                     ((uint32_t)0x00000040)        /*!< Bit 0 */
#define  GPIO_CRL_CNF1_1                     ((uint32_t)0x00000080)        /*!< Bit 1 */

#define  GPIO_CRL_CNF2                       ((uint32_t)0x00000C00)        /*!< CNF2[1:0] bits (Port x configuration bits, pin 2) */
#define  GPIO_CRL_CNF2_0                     ((uint32_t)0x00000400)        /*!< Bit 0 */
#define  GPIO_CRL_CNF2_1                     ((uint32_t)0x00000800)        /*!< Bit 1 */

#define  GPIO_CRL_CNF3                       ((uint32_t)0x0000C000)        /*!< CNF3[1:0] bits (Port x configuration bits, pin 3) */
#define  GPIO_CRL_CNF3_0                     ((uint32_t)0x00004000)        /*!< Bit 0 */
#define  GPIO_CRL_CNF3_1                     ((uint32_t)0x00008000)        /*!< Bit 1 */

#define  GPIO_CRL_CNF4                       ((uint32_t)0x000C0000)        /*!< CNF4[1:0] bits (Port x configuration bits, pin 4) */
#define  GPIO_CRL_CNF4_0                     ((uint32_t)0x00040000)        /*!< Bit 0 */
#define  GPIO_CRL_CNF4_1                     ((uint32_t)0x00080000)        /*!< Bit 1 */

#define  GPIO_CRL_CNF5                       ((uint32_t)0x00C00000)        /*!< CNF5[1:0] bits (Port x configuration bits, pin 5) */
#define  GPIO_CRL_CNF5_0                     ((uint32_t)0x00400000)        /*!< Bit 0 */
#define  GPIO_CRL_CNF5_1                     ((uint32_t)0x00800000)        /*!< Bit 1 */

#define  GPIO_CRL_CNF6                       ((uint32_t)0x0C000000)        /*!< CNF6[1:0] bits (Port x configuration bits, pin 6) */
#define  GPIO_CRL_CNF6_0                     ((uint32_t)0x04000000)        /*!< Bit 0 */
#define  GPIO_CRL_CNF6_1                     ((uint32_t)0x08000000)        /*!< Bit 1 */

#define  GPIO_CRL_CNF7                       ((uint32_t)0xC0000000)        /*!< CNF7[1:0] bits (Port x configuration bits, pin 7) */
#define  GPIO_CRL_CNF7_0                     ((uint32_t)0x40000000)        /*!< Bit 0 */
#define  GPIO_CRL_CNF7_1                     ((uint32_t)0x80000000)        /*!< Bit 1 */

/*******************  Bit definition for GPIO_CRH register  *******************/
#define  GPIO_CRH_MODE                       ((uint32_t)0x33333333)        /*!< Port x mode bits */

#define  GPIO_CRH_MODE8                      ((uint32_t)0x00000003)        /*!< MODE8[1:0] bits (Port x mode bits, pin 8) */
#define  GPIO_CRH_MODE8_0                    ((uint32_t)0x00000001)        /*!< Bit 0 */
#define  GPIO_CRH_MODE8_1                    ((uint32_t)0x00000002)        /*!< Bit 1 */

#define  GPIO_CRH_MODE9                      ((uint32_t)0x00000030)        /*!< MODE9[1:0] bits (Port x mode bits, pin 9) */
#define  GPIO_CRH_MODE9_0                    ((uint32_t)0x00000010)        /*!< Bit 0 */
#define  GPIO_CRH_MODE9_1                    ((uint32_t)0x00000020)        /*!< Bit 1 */

#define  GPIO_CRH_MODE10                     ((uint32_t)0x00000300)        /*!< MODE10[1:0] bits (Port x mode bits, pin 10) */
#define  GPIO_CRH_MODE10_0                   ((uint32_t)0x00000100)        /*!< Bit 0 */
#define  GPIO_CRH_MODE10_1                   ((uint32_t)0x00000200)        /*!< Bit 1 */

#define  GPIO_CRH_MODE11                     ((uint32_t)0x00003000)        /*!< MODE11[1:0] bits (Port x mode bits, pin 11) */
#define  GPIO_CRH_MODE11_0                   ((uint32_t)0x00001000)        /*!< Bit 0 */
#define  GPIO_CRH_MODE11_1                   ((uint32_t)0x00002000)        /*!< Bit 1 */

#define  GPIO_CRH_MODE12                     ((uint32_t)0x00030000)        /*!< MODE12[1:0] bits (Port x mode bits, pin 12) */
#define  GPIO_CRH_MODE12_0                   ((uint32_t)0x00010000)        /*!< Bit 0 */
#define  GPIO_CRH_MODE12_1                   ((uint32_t)0x00020000)        /*!< Bit 1 */

#define  GPIO_CRH_MODE13                     ((uint32_t)0x00300000)        /*!< MODE13[1:0] bits (Port x mode bits, pin 13) */
#define  GPIO_CRH_MODE13_0                   ((uint32_t)0x00100000)        /*!< Bit 0 */
#define  GPIO_CRH_MODE13_1                   ((uint32_t)0x00200000)        /*!< Bit 1 */

#define  GPIO_CRH_MODE14                     ((uint32_t)0x03000000)        /*!< MODE14[1:0] bits (Port x mode bits, pin 14) */
#define  GPIO_CRH_MODE14_0                   ((uint32_t)0x01000000)        /*!< Bit 0 */
#define  GPIO_CRH_MODE14_1                   ((uint32_t)0x02000000)        /*!< Bit 1 */

#define  GPIO_CRH_MODE15                     ((uint32_t)0x30000000)        /*!< MODE15[1:0] bits (Port x mode bits, pin 15) */
#define  GPIO_CRH_MODE15_0                   ((uint32_t)0x10000000)        /*!< Bit 0 */
#define  GPIO_CRH_MODE15_1                   ((uint32_t)0x20000000)        /*!< Bit 1 */

#define  GPIO_CRH_CNF                        ((uint32_t)0xCCCCCCCC)        /*!< Port x configuration bits */

#define  GPIO_CRH_CNF8                       ((uint32_t)0x0000000C)        /*!< CNF8[1:0] bits (Port x configuration bits, pin 8) */
#define  GPIO_CRH_CNF8_0                     ((uint32_t)0x00000004)        /*!< Bit 0 */
#define  GPIO_CRH_CNF8_1                     ((uint32_t)0x00000008)        /*!< Bit 1 */

#define  GPIO_CRH_CNF9                       ((uint32_t)0x000000C0)        /*!< CNF9[1:0] bits (Port x configuration bits, pin 9) */
#define  GPIO_CRH_CNF9_0                     ((uint32_t)0x00000040)        /*!< Bit 0 */
#define  GPIO_CRH_CNF9_1                     ((uint32_t)0x00000080)        /*!< Bit 1 */

#define  GPIO_CRH_CNF10                      ((uint32_t)0x00000C00)        /*!< CNF10[1:0] bits (Port x configuration bits, pin 10) */
#define  GPIO_CRH_CNF10_0                    ((uint32_t)0x00000400)        /*!< Bit 0 */
#define  GPIO_CRH_CNF10_1                    ((uint32_t)0x00000800)        /*!< Bit 1 */

#define  GPIO_CRH_CNF11                      ((uint32_t)0x0000C000)        /*!< CNF11[1:0] bits (Port x configuration bits, pin 11) */
#define  GPIO_CRH_CNF11_0                    ((uint32_t)0x00004000)        /*!< Bit 0 */
#define  GPIO_CRH_CNF11_1                    ((uint32_t)0x00008000)        /*!< Bit 1 */

#define  GPIO_CRH_CNF12                      ((uint32_t)0x000C0000)        /*!< CNF12[1:0] bits (Port x configuration bits, pin 12) */
#define  GPIO_CRH_CNF12_0                    ((uint32_t)0x00040000)        /*!< Bit 0 */
#define  GPIO_CRH_CNF12_1                    ((uint32_t)0x00080000)        /*!< Bit 1 */

#define  GPIO_CRH_CNF13                      ((uint32_t)0x00C00000)        /*!< CNF13[1:0] bits (Port x configuration bits, pin 13) */
#define  GPIO_CRH_CNF13_0                    ((uint32_t)0x00400000)        /*!< Bit 0 */
#define  GPIO_CRH_CNF13_1                    ((uint32_t)0x00800000)        /*!< Bit 1 */

#define  GPIO_CRH_CNF14                      ((uint32_t)0x0C000000)        /*!< CNF14[1:0] bits (Port x configuration bits, pin 14) */
#define  GPIO_CRH_CNF14_0                    ((uint32_t)0x04000000)        /*!< Bit 0 */
#define  GPIO_CRH_CNF14_1                    ((uint32_t)0x08000000)        /*!< Bit 1 */

#define  GPIO_CRH_CNF15                      ((uint32_t)0xC0000000)        /*!< CNF15[1:0] bits (Port x configuration bits, pin 15) */
#define  GPIO_CRH_CNF15_0                    ((uint32_t)0x40000000)        /*!< Bit 0 */
#define  GPIO_CRH_CNF15_1                    ((uint32_t)0x80000000)        /*!< Bit 1 */

/*!<******************  Bit definition for GPIO_IDR register  *******************/
#define GPIO_IDR_IDR0                        ((uint32_t)0x0001)            /*!< Port input data, bit 0 */
#define GPIO_IDR_IDR1                        ((uint32_t)0x0002)            /*!< Port input data, bit 1 */
#define GPIO_IDR_IDR2                        ((uint32_t)0x0004)            /*!< Port input data, bit 2 */
#define GPIO_IDR_IDR3                        ((uint32_t)0x0008)            /*!< Port input data, bit 3 */
#define GPIO_IDR_IDR4                        ((uint32_t)0x0010)            /*!< Port input data, bit 4 */
#define GPIO_IDR_IDR5                        ((uint32_t)0x0020)            /*!< Port input data, bit 5 */
#define GPIO_IDR_IDR6                        ((uint32_t)0x0040)            /*!< Port input data, bit 6 */
#define GPIO_IDR_IDR7                        ((uint32_t)0x0080)            /*!< Port input data, bit 7 */
#define GPIO_IDR_IDR8                        ((uint32_t)0x0100)            /*!< Port input data, bit 8 */
#define GPIO_IDR_IDR9                        ((uint32_t)0x0200)            /*!< Port input data, bit 9 */
#define GPIO_IDR_IDR10                       ((uint32_t)0x0400)            /*!< Port input data, bit 10 */
#define GPIO_IDR_IDR11                       ((uint32_t)0x0800)            /*!< Port input data, bit 11 */
#define GPIO_IDR_IDR12                       ((uint32_t)0x1000)            /*!< Port input data, bit 12 */
#define GPIO_IDR_IDR13                       ((uint32_t)0x2000)            /*!< Port input data, bit 13 */
#define GPIO_IDR_IDR14                       ((uint32_t)0x4000)            /*!< Port input data, bit 14 */
#define GPIO_IDR_IDR15                       ((uint32_t)0x8000)            /*!< Port input data, bit 15 */

/*******************  Bit definition for GPIO_ODR register  *******************/
#define GPIO_ODR_ODR0                        ((uint32_t)0x0001)            /*!< Port output data, bit 0 */
#define GPIO_ODR_ODR1                        ((uint32_t)0x0002)            /*!< Port output data, bit 1 */
#define GPIO_ODR_ODR2                        ((uint32_t)0x0004)            /*!< Port output data, bit 2 */
#define GPIO_ODR_ODR3                        ((uint32_t)0x0008)            /*!< Port output data, bit 3 */
#define GPIO_ODR_ODR4                        ((uint32_t)0x0010)            /*!< Port output data, bit 4 */
#define GPIO_ODR_ODR5                        ((uint32_t)0x0020)            /*!< Port output data, bit 5 */
#define GPIO_ODR_ODR6                        ((uint32_t)0x0040)            /*!< Port output data, bit 6 */
#define GPIO_ODR_ODR7                        ((uint32_t)0x0080)            /*!< Port output data, bit 7 */
#define GPIO_ODR_ODR8                        ((uint32_t)0x0100)            /*!< Port output data, bit 8 */
#define GPIO_ODR_ODR9                        ((uint32_t)0x0200)            /*!< Port output data, bit 9 */
#define GPIO_ODR_ODR10                       ((uint32_t)0x0400)            /*!< Port output data, bit 10 */
#define GPIO_ODR_ODR11                       ((uint32_t)0x0800)            /*!< Port output data, bit 11 */
#define GPIO_ODR_ODR12                       ((uint32_t)0x1000)            /*!< Port output data, bit 12 */
#define GPIO_ODR_ODR13                       ((uint32_t)0x2000)            /*!< Port output data, bit 13 */
#define GPIO_ODR_ODR14                       ((uint32_t)0x4000)            /*!< Port output data, bit 14 */
#define GPIO_ODR_ODR15                       ((uint32_t)0x8000)            /*!< Port output data, bit 15 */

/******************  Bit definition for GPIO_BSRR register  *******************/
#define GPIO_BSRR_BS0                        ((uint32_t)0x00000001)        /*!< Port x Set bit 0 */
#define GPIO_BSRR_BS1                        ((uint32_t)0x00000002)        /*!< Port x Set bit 1 */
#define GPIO_BSRR_BS2                        ((uint32_t)0x00000004)        /*!< Port x Set bit 2 */
#define GPIO_BSRR_BS3                        ((uint32_t)0x00000008)        /*!< Port x Set bit 3 */
#define GPIO_BSRR_BS4                        ((uint32_t)0x00000010)        /*!< Port x Set bit 4 */
#define GPIO_BSRR_BS5                        ((uint32_t)0x00000020)        /*!< Port x Set bit 5 */
#define GPIO_BSRR_BS6                        ((uint32_t)0x00000040)        /*!< Port x Set bit 6 */
#define GPIO_BSRR_BS7                        ((uint32_t)0x00000080)        /*!< Port x Set bit 7 */
#define GPIO_BSRR_BS8                        ((uint32_t)0x00000100)        /*!< Port x Set bit 8 */
#define GPIO_BSRR_BS9                        ((uint32_t)0x00000200)        /*!< Port x Set bit 9 */
#define GPIO_BSRR_BS10                       ((uint32_t)0x00000400)        /*!< Port x Set bit 10 */
#define GPIO_BSRR_BS11                       ((uint32_t)0x00000800)        /*!< Port x Set bit 11 */
#define GPIO_BSRR_BS12                       ((uint32_t)0x00001000)        /*!< Port x Set bit 12 */
#define GPIO_BSRR_BS13                       ((uint32_t)0x00002000)        /*!< Port x Set bit 13 */
#define GPIO_BSRR_BS14                       ((uint32_t)0x00004000)        /*!< Port x Set bit 14 */
#define GPIO_BSRR_BS15                       ((uint32_t)0x00008000)        /*!< Port x Set bit 15 */

#define GPIO_BSRR_BR0                        ((uint32_t)0x00010000)        /*!< Port x Reset bit 0 */
#define GPIO_BSRR_BR1                        ((uint32_t)0x00020000)        /*!< Port x Reset bit 1 */
#define GPIO_BSRR_BR2                        ((uint32_t)0x00040000)        /*!< Port x Reset bit 2 */
#define GPIO_BSRR_BR3                        ((uint32_t)0x00080000)        /*!< Port x Reset bit 3 */
#define GPIO_BSRR_BR4                        ((uint32_t)0x00100000)        /*!< Port x Reset bit 4 */
#define GPIO_BSRR_BR5                        ((uint32_t)0x00200000)        /*!< Port x Reset bit 5 */
#define GPIO_BSRR_BR6                        ((uint32_t)0x00400000)        /*!< Port x Reset bit 6 */
#define GPIO_BSRR_BR7                        ((uint32_t)0x00800000)        /*!< Port x Reset bit 7 */
#define GPIO_BSRR_BR8                        ((uint32_t)0x01000000)        /*!< Port x Reset bit 8 */
#define GPIO_BSRR_BR9                        ((uint32_t)0x02000000)        /*!< Port x Reset bit 9 */
#define GPIO_BSRR_BR10                       ((uint32_t)0x04000000)        /*!< Port x Reset bit 10 */
#define GPIO_BSRR_BR11                       ((uint32_t)0x08000000)        /*!< Port x Reset bit 11 */
#define GPIO_BSRR_BR12                       ((uint32_t)0x10000000)        /*!< Port x Reset bit 12 */
#define GPIO_BSRR_BR13                       ((uint32_t)0x20000000)        /*!< Port x Reset bit 13 */
#define GPIO_BSRR_BR14                       ((uint32_t)0x40000000)        /*!< Port x Reset bit 14 */
#define GPIO_BSRR_BR15                       ((uint32_t)0x80000000)        /*!< Port x Reset bit 15 */

/*******************  Bit definition for GPIO_BRR register  *******************/
#define GPIO_BRR_BR0                         ((uint32_t)0x0001)            /*!< Port x Reset bit 0 */
#define GPIO_BRR_BR1                         ((uint32_t)0x0002)            /*!< Port x Reset bit 1 */
#define GPIO_BRR_BR2                         ((uint32_t)0x0004)            /*!< Port x Reset bit 2 */
#define GPIO_BRR_BR3                         ((uint32_t)0x0008)            /*!< Port x Reset bit 3 */
#define GPIO_BRR_BR4                         ((uint32_t)0x0010)            /*!< Port x Reset bit 4 */
#define GPIO_BRR_BR5                         ((uint32_t)0x0020)            /*!< Port x Reset bit 5 */
#define GPIO_BRR_BR6                         ((uint32_t)0x0040)            /*!< Port x Reset bit 6 */
#define GPIO_BRR_BR7                         ((uint32_t)0x0080)            /*!< Port x Reset bit 7 */
#define GPIO_BRR_BR8                         ((uint32_t)0x0100)            /*!< Port x Reset bit 8 */
#define GPIO_BRR_BR9                         ((uint32_t)0x0200)            /*!< Port x Reset bit 9 */
#define GPIO_BRR_BR10                        ((uint32_t)0x0400)            /*!< Port x Reset bit 10 */
#define GPIO_BRR_BR11                        ((uint32_t)0x0800)            /*!< Port x Reset bit 11 */
#define GPIO_BRR_BR12                        ((uint32_t)0x1000)            /*!< Port x Reset bit 12 */
#define GPIO_BRR_BR13                        ((uint32_t)0x2000)            /*!< Port x Reset bit 13 */
#define GPIO_BRR_BR14                        ((uint32_t)0x4000)            /*!< Port x Reset bit 14 */
#define GPIO_BRR_BR15                        ((uint32_t)0x8000)            /*!< Port x Reset bit 15 */

/******************  Bit definition for GPIO_LCKR register  *******************/
#define GPIO_LCKR_LCK0                       ((uint32_t)0x00000001)        /*!< Port x Lock bit 0 */
#define GPIO_LCKR_LCK1                       ((uint32_t)0x00000002)        /*!< Port x Lock bit 1 */
#define GPIO_LCKR_LCK2                       ((uint32_t)0x00000004)        /*!< Port x Lock bit 2 */
#define GPIO_LCKR_LCK3                       ((uint32_t)0x00000008)        /*!< Port x Lock bit 3 */
#define GPIO_LCKR_LCK4                       ((uint32_t)0x00000010)        /*!< Port x Lock bit 4 */
#define GPIO_LCKR_LCK5                       ((uint32_t)0x00000020)        /*!< Port x Lock bit 5 */
#define GPIO_LCKR_LCK6                       ((uint32_t)0x00000040)        /*!< Port x Lock bit 6 */
#define GPIO_LCKR_LCK7                       ((uint32_t)0x00000080)        /*!< Port x Lock bit 7 */
#define GPIO_LCKR_LCK8                       ((uint32_t)0x00000100)        /*!< Port x Lock bit 8 */
#define GPIO_LCKR_LCK9                       ((uint32_t)0x00000200)        /*!< Port x Lock bit 9 */
#define GPIO_LCKR_LCK10                      ((uint32_t)0x00000400)        /*!< Port x Lock bit 10 */
#define GPIO_LCKR_LCK11                      ((uint32_t)0x00000800)        /*!< Port x Lock bit 11 */
#define GPIO_LCKR_LCK12                      ((uint32_t)0x00001000)        /*!< Port x Lock bit 12 */
#define GPIO_LCKR_LCK13                      ((uint32_t)0x00002000)        /*!< Port x Lock bit 13 */
#define GPIO_LCKR_LCK14                      ((uint32_t)0x00004000)        /*!< Port x Lock bit 14 */
#define GPIO_LCKR_LCK15                      ((uint32_t)0x00008000)        /*!< Port x Lock bit 15 */
#define GPIO_LCKR_LCKK                       ((uint32_t)0x00010000)        /*!< Lock key */

/*----------------------------------------------------------------------------*/

/******************  Bit definition for AFIO_EVCR register  *******************/
#define AFIO_EVCR_PIN                        ((uint32_t)0x0000000F)               /*!< PIN[3:0] bits (Pin selection) */
#define AFIO_EVCR_PIN_0                      ((uint32_t)0x00000001)               /*!< Bit 0 */
#define AFIO_EVCR_PIN_1                      ((uint32_t)0x00000002)               /*!< Bit 1 */
#define AFIO_EVCR_PIN_2                      ((uint32_t)0x00000004)               /*!< Bit 2 */
#define AFIO_EVCR_PIN_3                      ((uint32_t)0x00000008)               /*!< Bit 3 */

/*!< PIN configuration */
#define AFIO_EVCR_PIN_PX0                    ((uint32_t)0x00000000)               /*!< Pin 0 selected */
#define AFIO_EVCR_PIN_PX1                    ((uint32_t)0x00000001)               /*!< Pin 1 selected */
#define AFIO_EVCR_PIN_PX2                    ((uint32_t)0x00000002)               /*!< Pin 2 selected */
#define AFIO_EVCR_PIN_PX3                    ((uint32_t)0x00000003)               /*!< Pin 3 selected */
#define AFIO_EVCR_PIN_PX4                    ((uint32_t)0x00000004)               /*!< Pin 4 selected */
#define AFIO_EVCR_PIN_PX5                    ((uint32_t)0x00000005)               /*!< Pin 5 selected */
#define AFIO_EVCR_PIN_PX6                    ((uint32_t)0x00000006)               /*!< Pin 6 selected */
#define AFIO_EVCR_PIN_PX7                    ((uint32_t)0x00000007)               /*!< Pin 7 selected */
#define AFIO_EVCR_PIN_PX8                    ((uint32_t)0x00000008)               /*!< Pin 8 selected */
#define AFIO_EVCR_PIN_PX9                    ((uint32_t)0x00000009)               /*!< Pin 9 selected */
#define AFIO_EVCR_PIN_PX10                   ((uint32_t)0x0000000A)               /*!< Pin 10 selected */
#define AFIO_EVCR_PIN_PX11                   ((uint32_t)0x0000000B)               /*!< Pin 11 selected */
#define AFIO_EVCR_PIN_PX12                   ((uint32_t)0x0000000C)               /*!< Pin 12 selected */
#define AFIO_EVCR_PIN_PX13                   ((uint32_t)0x0000000D)               /*!< Pin 13 selected */
#define AFIO_EVCR_PIN_PX14                   ((uint32_t)0x0000000E)               /*!< Pin 14 selected */
#define AFIO_EVCR_PIN_PX15                   ((uint32_t)0x0000000F)               /*!< Pin 15 selected */

#define AFIO_EVCR_PORT                       ((uint32_t)0x00000070)               /*!< PORT[2:0] bits (Port selection) */
#define AFIO_EVCR_PORT_0                     ((uint32_t)0x00000010)               /*!< Bit 0 */
#define AFIO_EVCR_PORT_1                     ((uint32_t)0x00000020)               /*!< Bit 1 */
#define AFIO_EVCR_PORT_2                     ((uint32_t)0x00000040)               /*!< Bit 2 */

/*!< PORT configuration */
#define AFIO_EVCR_PORT_PA                    ((uint32_t)0x00000000)               /*!< Port A selected */
#define AFIO_EVCR_PORT_PB                    ((uint32_t)0x00000010)               /*!< Port B selected */
#define AFIO_EVCR_PORT_PC                    ((uint32_t)0x00000020)               /*!< Port C selected */
#define AFIO_EVCR_PORT_PD                    ((uint32_t)0x00000030)               /*!< Port D selected */
#define AFIO_EVCR_PORT_PE                    ((uint32_t)0x00000040)               /*!< Port E selected */

#define AFIO_EVCR_EVOE                       ((uint32_t)0x00000080)               /*!< Event Output Enable */

/******************  Bit definition for AFIO_MAPR register  *******************/
#define AFIO_MAPR_SPI1_REMAP                 ((uint32_t)0x00000001)        /*!< SPI1 remapping */
#define AFIO_MAPR_I2C1_REMAP                 ((uint32_t)0x00000002)        /*!< I2C1 remapping */
#define AFIO_MAPR_USART1_REMAP               ((uint32_t)0x00000004)        /*!< USART1 remapping */
#define AFIO_MAPR_USART2_REMAP               ((uint32_t)0x00000008)        /*!< USART2 remapping */

#define AFIO_MAPR_USART3_REMAP               ((uint32_t)0x00000030)        /*!< USART3_REMAP[1:0] bits (USART3 remapping) */
#define AFIO_MAPR_USART3_REMAP_0             ((uint32_t)0x00000010)        /*!< Bit 0 */
#define AFIO_MAPR_USART3_REMAP_1             ((uint32_t)0x00000020)        /*!< Bit 1 */

/* USART3_REMAP configuration */
#define AFIO_MAPR_USART3_REMAP_NOREMAP       ((uint32_t)0x00000000)        /*!< No remap (TX/PB10, RX/PB11, CK/PB12, CTS/PB13, RTS/PB14) */
#define AFIO_MAPR_USART3_REMAP_PARTIALREMAP  ((uint32_t)0x00000010)        /*!< Partial remap (TX/PC10, RX/PC11, CK/PC12, CTS/PB13, RTS/PB14) */
#define AFIO_MAPR_USART3_REMAP_FULLREMAP     ((uint32_t)0x00000030)        /*!< Full remap (TX/PD8, RX/PD9, CK/PD10, CTS/PD11, RTS/PD12) */

#define AFIO_MAPR_TIM1_REMAP                 ((uint32_t)0x000000C0)        /*!< TIM1_REMAP[1:0] bits (TIM1 remapping) */
#define AFIO_MAPR_TIM1_REMAP_0               ((uint32_t)0x00000040)        /*!< Bit 0 */
#define AFIO_MAPR_TIM1_REMAP_1               ((uint32_t)0x00000080)        /*!< Bit 1 */

/*!< TIM1_REMAP configuration */
#define AFIO_MAPR_TIM1_REMAP_NOREMAP         ((uint32_t)0x00000000)        /*!< No remap (ETR/PA12, CH1/PA8, CH2/PA9, CH3/PA10, CH4/PA11, BKIN/PB12, CH1N/PB13, CH2N/PB14, CH3N/PB15) */
#define AFIO_MAPR_TIM1_REMAP_PARTIALREMAP    ((uint32_t)0x00000040)        /*!< Partial remap (ETR/PA12, CH1/PA8, CH2/PA9, CH3/PA10, CH4/PA11, BKIN/PA6, CH1N/PA7, CH2N/PB0, CH3N/PB1) */
#define AFIO_MAPR_TIM1_REMAP_FULLREMAP       ((uint32_t)0x000000C0)        /*!< Full remap (ETR/PE7, CH1/PE9, CH2/PE11, CH3/PE13, CH4/PE14, BKIN/PE15, CH1N/PE8, CH2N/PE10, CH3N/PE12) */

#define AFIO_MAPR_TIM2_REMAP                 ((uint32_t)0x00000300)        /*!< TIM2_REMAP[1:0] bits (TIM2 remapping) */
#define AFIO_MAPR_TIM2_REMAP_0               ((uint32_t)0x00000100)        /*!< Bit 0 */
#define AFIO_MAPR_TIM2_REMAP_1               ((uint32_t)0x00000200)        /*!< Bit 1 */

/*!< TIM2_REMAP configuration */
#define AFIO_MAPR_TIM2_REMAP_NOREMAP         ((uint32_t)0x00000000)        /*!< No remap (CH1/ETR/PA0, CH2/PA1, CH3/PA2, CH4/PA3) */
#define AFIO_MAPR_TIM2_REMAP_PARTIALREMAP1   ((uint32_t)0x00000100)        /*!< Partial remap (CH1/ETR/PA15, CH2/PB3, CH3/PA2, CH4/PA3) */
#define AFIO_MAPR_TIM2_REMAP_PARTIALREMAP2   ((uint32_t)0x00000200)        /*!< Partial remap (CH1/ETR/PA0, CH2/PA1, CH3/PB10, CH4/PB11) */
#define AFIO_MAPR_TIM2_REMAP_FULLREMAP       ((uint32_t)0x00000300)        /*!< Full remap (CH1/ETR/PA15, CH2/PB3, CH3/PB10, CH4/PB11) */

#define AFIO_MAPR_TIM3_REMAP                 ((uint32_t)0x00000C00)        /*!< TIM3_REMAP[1:0] bits (TIM3 remapping) */
#define AFIO_MAPR_TIM3_REMAP_0               ((uint32_t)0x00000400)        /*!< Bit 0 */
#define AFIO_MAPR_TIM3_REMAP_1               ((uint32_t)0x00000800)        /*!< Bit 1 */

/*!< TIM3_REMAP configuration */
#define AFIO_MAPR_TIM3_REMAP_NOREMAP         ((uint32_t)0x00000000)        /*!< No remap (CH1/PA6, CH2/PA7, CH3/PB0, CH4/PB1) */
#define AFIO_MAPR_TIM3_REMAP_PARTIALREMAP    ((uint32_t)0x00000800)        /*!< Partial remap (CH1/PB4, CH2/PB5, CH3/PB0, CH4/PB1) */
#define AFIO_MAPR_TIM3_REMAP_FULLREMAP       ((uint32_t)0x00000C00)        /*!< Full remap (CH1/PC6, CH2/PC7, CH3/PC8, CH4/PC9) */

#define AFIO_MAPR_TIM4_REMAP                 ((uint32_t)0x00001000)        /*!< TIM4_REMAP bit (TIM4 remapping) */

#define AFIO_MAPR_CAN_REMAP                  ((uint32_t)0x00006000)        /*!< CAN_REMAP[1:0] bits (CAN Alternate function remapping) */
#define AFIO_MAPR_CAN_REMAP_0                ((uint32_t)0x00002000)        /*!< Bit 0 */
#define AFIO_MAPR_CAN_REMAP_1                ((uint32_t)0x00004000)        /*!< Bit 1 */

/*!< CAN_REMAP configuration */
#define AFIO_MAPR_CAN_REMAP_REMAP1           ((uint32_t)0x00000000)        /*!< CANRX mapped to PA11, CANTX mapped to PA12 */
#define AFIO_MAPR_CAN_REMAP_REMAP2           ((uint32_t)0x00004000)        /*!< CANRX mapped to PB8, CANTX mapped to PB9 */
#define AFIO_MAPR_CAN_REMAP_REMAP3           ((uint32_t)0x00006000)        /*!< CANRX mapped to PD0, CANTX mapped to PD1 */

#define AFIO_MAPR_PD01_REMAP                 ((uint32_t)0x00008000)        /*!< Port D0/Port D1 mapping on OSC_IN/OSC_OUT */
#define AFIO_MAPR_TIM5CH4_IREMAP             ((uint32_t)0x00010000)        /*!< TIM5 Channel4 Internal Remap */

/*!< SWJ_CFG configuration */
#define AFIO_MAPR_SWJ_CFG                    ((uint32_t)0x07000000)        /*!< SWJ_CFG[2:0] bits (Serial Wire JTAG configuration) */
#define AFIO_MAPR_SWJ_CFG_0                  ((uint32_t)0x01000000)        /*!< Bit 0 */
#define AFIO_MAPR_SWJ_CFG_1                  ((uint32_t)0x02000000)        /*!< Bit 1 */
#define AFIO_MAPR_SWJ_CFG_2                  ((uint32_t)0x04000000)        /*!< Bit 2 */

#define AFIO_MAPR_SWJ_CFG_RESET              ((uint32_t)0x00000000)        /*!< Full SWJ (JTAG-DP + SW-DP) : Reset State */
#define AFIO_MAPR_SWJ_CFG_NOJNTRST           ((uint32_t)0x01000000)        /*!< Full SWJ (JTAG-DP + SW-DP) but without JNTRST */
#define AFIO_MAPR_SWJ_CFG_JTAGDISABLE        ((uint32_t)0x02000000)        /*!< JTAG-DP Disabled and SW-DP Enabled */
#define AFIO_MAPR_SWJ_CFG_DISABLE            ((uint32_t)0x04000000)        /*!< JTAG-DP Disabled and SW-DP Disabled */

/*!< ETH_REMAP configuration */
 #define AFIO_MAPR_ETH_REMAP                  ((uint32_t)0x00200000)        /*!< SPI3_REMAP bit (Ethernet MAC I/O remapping) */

/*!< CAN2_REMAP configuration */
 #define AFIO_MAPR_CAN2_REMAP                 ((uint32_t)0x00400000)        /*!< CAN2_REMAP bit (CAN2 I/O remapping) */

/*!< MII_RMII_SEL configuration */
 #define AFIO_MAPR_MII_RMII_SEL               ((uint32_t)0x00800000)        /*!< MII_RMII_SEL bit (Ethernet MII or RMII selection) */

/*!< SPI3_REMAP configuration */
 #define AFIO_MAPR_SPI3_REMAP                 ((uint32_t)0x10000000)        /*!< SPI3_REMAP bit (SPI3 remapping) */

/*!< TIM2ITR1_IREMAP configuration */
 #define AFIO_MAPR_TIM2ITR1_IREMAP            ((uint32_t)0x20000000)        /*!< TIM2ITR1_IREMAP bit (TIM2 internal trigger 1 remapping) */

/*!< PTP_PPS_REMAP configuration */
 #define AFIO_MAPR_PTP_PPS_REMAP              ((uint32_t)0x40000000)        /*!< PTP_PPS_REMAP bit (Ethernet PTP PPS remapping) */

/*****************  Bit definition for AFIO_EXTICR1 register  *****************/
#define AFIO_EXTICR1_EXTI0                   ((uint32_t)0x0000000F)            /*!< EXTI 0 configuration */
#define AFIO_EXTICR1_EXTI1                   ((uint32_t)0x000000F0)            /*!< EXTI 1 configuration */
#define AFIO_EXTICR1_EXTI2                   ((uint32_t)0x00000F00)            /*!< EXTI 2 configuration */
#define AFIO_EXTICR1_EXTI3                   ((uint32_t)0x0000F000)            /*!< EXTI 3 configuration */

/*!< EXTI0 configuration */
#define AFIO_EXTICR1_EXTI0_PA                ((uint32_t)0x00000000)            /*!< PA[0] pin */
#define AFIO_EXTICR1_EXTI0_PB                ((uint32_t)0x00000001)            /*!< PB[0] pin */
#define AFIO_EXTICR1_EXTI0_PC                ((uint32_t)0x00000002)            /*!< PC[0] pin */
#define AFIO_EXTICR1_EXTI0_PD                ((uint32_t)0x00000003)            /*!< PD[0] pin */
#define AFIO_EXTICR1_EXTI0_PE                ((uint32_t)0x00000004)            /*!< PE[0] pin */
#define AFIO_EXTICR1_EXTI0_PF                ((uint32_t)0x00000005)            /*!< PF[0] pin */
#define AFIO_EXTICR1_EXTI0_PG                ((uint32_t)0x00000006)            /*!< PG[0] pin */

/*!< EXTI1 configuration */
#define AFIO_EXTICR1_EXTI1_PA                ((uint32_t)0x00000000)            /*!< PA[1] pin */
#define AFIO_EXTICR1_EXTI1_PB                ((uint32_t)0x00000010)            /*!< PB[1] pin */
#define AFIO_EXTICR1_EXTI1_PC                ((uint32_t)0x00000020)            /*!< PC[1] pin */
#define AFIO_EXTICR1_EXTI1_PD                ((uint32_t)0x00000030)            /*!< PD[1] pin */
#define AFIO_EXTICR1_EXTI1_PE                ((uint32_t)0x00000040)            /*!< PE[1] pin */
#define AFIO_EXTICR1_EXTI1_PF                ((uint32_t)0x00000050)            /*!< PF[1] pin */
#define AFIO_EXTICR1_EXTI1_PG                ((uint32_t)0x00000060)            /*!< PG[1] pin */

/*!< EXTI2 configuration */  
#define AFIO_EXTICR1_EXTI2_PA                ((uint32_t)0x00000000)            /*!< PA[2] pin */
#define AFIO_EXTICR1_EXTI2_PB                ((uint32_t)0x00000100)            /*!< PB[2] pin */
#define AFIO_EXTICR1_EXTI2_PC                ((uint32_t)0x00000200)            /*!< PC[2] pin */
#define AFIO_EXTICR1_EXTI2_PD                ((uint32_t)0x00000300)            /*!< PD[2] pin */
#define AFIO_EXTICR1_EXTI2_PE                ((uint32_t)0x00000400)            /*!< PE[2] pin */
#define AFIO_EXTICR1_EXTI2_PF                ((uint32_t)0x00000500)            /*!< PF[2] pin */
#define AFIO_EXTICR1_EXTI2_PG                ((uint32_t)0x00000600)            /*!< PG[2] pin */

/*!< EXTI3 configuration */
#define AFIO_EXTICR1_EXTI3_PA                ((uint32_t)0x00000000)            /*!< PA[3] pin */
#define AFIO_EXTICR1_EXTI3_PB                ((uint32_t)0x00001000)            /*!< PB[3] pin */
#define AFIO_EXTICR1_EXTI3_PC                ((uint32_t)0x00002000)            /*!< PC[3] pin */
#define AFIO_EXTICR1_EXTI3_PD                ((uint32_t)0x00003000)            /*!< PD[3] pin */
#define AFIO_EXTICR1_EXTI3_PE                ((uint32_t)0x00004000)            /*!< PE[3] pin */
#define AFIO_EXTICR1_EXTI3_PF                ((uint32_t)0x00005000)            /*!< PF[3] pin */
#define AFIO_EXTICR1_EXTI3_PG                ((uint32_t)0x00006000)            /*!< PG[3] pin */

/*****************  Bit definition for AFIO_EXTICR2 register  *****************/
#define AFIO_EXTICR2_EXTI4                   ((uint32_t)0x0000000F)            /*!< EXTI 4 configuration */
#define AFIO_EXTICR2_EXTI5                   ((uint32_t)0x000000F0)            /*!< EXTI 5 configuration */
#define AFIO_EXTICR2_EXTI6                   ((uint32_t)0x00000F00)            /*!< EXTI 6 configuration */
#define AFIO_EXTICR2_EXTI7                   ((uint32_t)0x0000F000)            /*!< EXTI 7 configuration */

/*!< EXTI4 configuration */
#define AFIO_EXTICR2_EXTI4_PA                ((uint32_t)0x00000000)            /*!< PA[4] pin */
#define AFIO_EXTICR2_EXTI4_PB                ((uint32_t)0x00000001)            /*!< PB[4] pin */
#define AFIO_EXTICR2_EXTI4_PC                ((uint32_t)0x00000002)            /*!< PC[4] pin */
#define AFIO_EXTICR2_EXTI4_PD                ((uint32_t)0x00000003)            /*!< PD[4] pin */
#define AFIO_EXTICR2_EXTI4_PE                ((uint32_t)0x00000004)            /*!< PE[4] pin */
#define AFIO_EXTICR2_EXTI4_PF                ((uint32_t)0x00000005)            /*!< PF[4] pin */
#define AFIO_EXTICR2_EXTI4_PG                ((uint32_t)0x00000006)            /*!< PG[4] pin */

/* EXTI5 configuration */
#define AFIO_EXTICR2_EXTI5_PA                ((uint32_t)0x00000000)            /*!< PA[5] pin */
#define AFIO_EXTICR2_EXTI5_PB                ((uint32_t)0x00000010)            /*!< PB[5] pin */
#define AFIO_EXTICR2_EXTI5_PC                ((uint32_t)0x00000020)            /*!< PC[5] pin */
#define AFIO_EXTICR2_EXTI5_PD                ((uint32_t)0x00000030)            /*!< PD[5] pin */
#define AFIO_EXTICR2_EXTI5_PE                ((uint32_t)0x00000040)            /*!< PE[5] pin */
#define AFIO_EXTICR2_EXTI5_PF                ((uint32_t)0x00000050)            /*!< PF[5] pin */
#define AFIO_EXTICR2_EXTI5_PG                ((uint32_t)0x00000060)            /*!< PG[5] pin */

/*!< EXTI6 configuration */  
#define AFIO_EXTICR2_EXTI6_PA                ((uint32_t)0x00000000)            /*!< PA[6] pin */
#define AFIO_EXTICR2_EXTI6_PB                ((uint32_t)0x00000100)            /*!< PB[6] pin */
#define AFIO_EXTICR2_EXTI6_PC                ((uint32_t)0x00000200)            /*!< PC[6] pin */
#define AFIO_EXTICR2_EXTI6_PD                ((uint32_t)0x00000300)            /*!< PD[6] pin */
#define AFIO_EXTICR2_EXTI6_PE                ((uint32_t)0x00000400)            /*!< PE[6] pin */
#define AFIO_EXTICR2_EXTI6_PF                ((uint32_t)0x00000500)            /*!< PF[6] pin */
#define AFIO_EXTICR2_EXTI6_PG                ((uint32_t)0x00000600)            /*!< PG[6] pin */

/*!< EXTI7 configuration */
#define AFIO_EXTICR2_EXTI7_PA                ((uint32_t)0x00000000)            /*!< PA[7] pin */
#define AFIO_EXTICR2_EXTI7_PB                ((uint32_t)0x00001000)            /*!< PB[7] pin */
#define AFIO_EXTICR2_EXTI7_PC                ((uint32_t)0x00002000)            /*!< PC[7] pin */
#define AFIO_EXTICR2_EXTI7_PD                ((uint32_t)0x00003000)            /*!< PD[7] pin */
#define AFIO_EXTICR2_EXTI7_PE                ((uint32_t)0x00004000)            /*!< PE[7] pin */
#define AFIO_EXTICR2_EXTI7_PF                ((uint32_t)0x00005000)            /*!< PF[7] pin */
#define AFIO_EXTICR2_EXTI7_PG                ((uint32_t)0x00006000)            /*!< PG[7] pin */

/*****************  Bit definition for AFIO_EXTICR3 register  *****************/
#define AFIO_EXTICR3_EXTI8                   ((uint32_t)0x0000000F)            /*!< EXTI 8 configuration */
#define AFIO_EXTICR3_EXTI9                   ((uint32_t)0x000000F0)            /*!< EXTI 9 configuration */
#define AFIO_EXTICR3_EXTI10                  ((uint32_t)0x00000F00)            /*!< EXTI 10 configuration */
#define AFIO_EXTICR3_EXTI11                  ((uint32_t)0x0000F000)            /*!< EXTI 11 configuration */

/*!< EXTI8 configuration */
#define AFIO_EXTICR3_EXTI8_PA                ((uint32_t)0x00000000)            /*!< PA[8] pin */
#define AFIO_EXTICR3_EXTI8_PB                ((uint32_t)0x00000001)            /*!< PB[8] pin */
#define AFIO_EXTICR3_EXTI8_PC                ((uint32_t)0x00000002)            /*!< PC[8] pin */
#define AFIO_EXTICR3_EXTI8_PD                ((uint32_t)0x00000003)            /*!< PD[8] pin */
#define AFIO_EXTICR3_EXTI8_PE                ((uint32_t)0x00000004)            /*!< PE[8] pin */
#define AFIO_EXTICR3_EXTI8_PF                ((uint32_t)0x00000005)            /*!< PF[8] pin */
#define AFIO_EXTICR3_EXTI8_PG                ((uint32_t)0x00000006)            /*!< PG[8] pin */

/*!< EXTI9 configuration */
#define AFIO_EXTICR3_EXTI9_PA                ((uint32_t)0x00000000)            /*!< PA[9] pin */
#define AFIO_EXTICR3_EXTI9_PB                ((uint32_t)0x00000010)            /*!< PB[9] pin */
#define AFIO_EXTICR3_EXTI9_PC                ((uint32_t)0x00000020)            /*!< PC[9] pin */
#define AFIO_EXTICR3_EXTI9_PD                ((uint32_t)0x00000030)            /*!< PD[9] pin */
#define AFIO_EXTICR3_EXTI9_PE                ((uint32_t)0x00000040)            /*!< PE[9] pin */
#define AFIO_EXTICR3_EXTI9_PF                ((uint32_t)0x00000050)            /*!< PF[9] pin */
#define AFIO_EXTICR3_EXTI9_PG                ((uint32_t)0x00000060)            /*!< PG[9] pin */

/*!< EXTI10 configuration */  
#define AFIO_EXTICR3_EXTI10_PA               ((uint32_t)0x00000000)            /*!< PA[10] pin */
#define AFIO_EXTICR3_EXTI10_PB               ((uint32_t)0x00000100)            /*!< PB[10] pin */
#define AFIO_EXTICR3_EXTI10_PC               ((uint32_t)0x00000200)            /*!< PC[10] pin */
#define AFIO_EXTICR3_EXTI10_PD               ((uint32_t)0x00000300)            /*!< PD[10] pin */
#define AFIO_EXTICR3_EXTI10_PE               ((uint32_t)0x00000400)            /*!< PE[10] pin */
#define AFIO_EXTICR3_EXTI10_PF               ((uint32_t)0x00000500)            /*!< PF[10] pin */
#define AFIO_EXTICR3_EXTI10_PG               ((uint32_t)0x00000600)            /*!< PG[10] pin */

/*!< EXTI11 configuration */
#define AFIO_EXTICR3_EXTI11_PA               ((uint32_t)0x00000000)            /*!< PA[11] pin */
#define AFIO_EXTICR3_EXTI11_PB               ((uint32_t)0x00001000)            /*!< PB[11] pin */
#define AFIO_EXTICR3_EXTI11_PC               ((uint32_t)0x00002000)            /*!< PC[11] pin */
#define AFIO_EXTICR3_EXTI11_PD               ((uint32_t)0x00003000)            /*!< PD[11] pin */
#define AFIO_EXTICR3_EXTI11_PE               ((uint32_t)0x00004000)            /*!< PE[11] pin */
#define AFIO_EXTICR3_EXTI11_PF               ((uint32_t)0x00005000)            /*!< PF[11] pin */
#define AFIO_EXTICR3_EXTI11_PG               ((uint32_t)0x00006000)            /*!< PG[11] pin */

/*****************  Bit definition for AFIO_EXTICR4 register  *****************/
#define AFIO_EXTICR4_EXTI12                  ((uint32_t)0x0000000F)            /*!< EXTI 12 configuration */
#define AFIO_EXTICR4_EXTI13                  ((uint32_t)0x000000F0)            /*!< EXTI 13 configuration */
#define AFIO_EXTICR4_EXTI14                  ((uint32_t)0x00000F00)            /*!< EXTI 14 configuration */
#define AFIO_EXTICR4_EXTI15                  ((uint32_t)0x0000F000)            /*!< EXTI 15 configuration */

/* EXTI12 configuration */
#define AFIO_EXTICR4_EXTI12_PA               ((uint32_t)0x00000000)            /*!< PA[12] pin */
#define AFIO_EXTICR4_EXTI12_PB               ((uint32_t)0x00000001)            /*!< PB[12] pin */
#define AFIO_EXTICR4_EXTI12_PC               ((uint32_t)0x00000002)            /*!< PC[12] pin */
#define AFIO_EXTICR4_EXTI12_PD               ((uint32_t)0x00000003)            /*!< PD[12] pin */
#define AFIO_EXTICR4_EXTI12_PE               ((uint32_t)0x00000004)            /*!< PE[12] pin */
#define AFIO_EXTICR4_EXTI12_PF               ((uint32_t)0x00000005)            /*!< PF[12] pin */
#define AFIO_EXTICR4_EXTI12_PG               ((uint32_t)0x00000006)            /*!< PG[12] pin */

/* EXTI13 configuration */
#define AFIO_EXTICR4_EXTI13_PA               ((uint32_t)0x00000000)            /*!< PA[13] pin */
#define AFIO_EXTICR4_EXTI13_PB               ((uint32_t)0x00000010)            /*!< PB[13] pin */
#define AFIO_EXTICR4_EXTI13_PC               ((uint32_t)0x00000020)            /*!< PC[13] pin */
#define AFIO_EXTICR4_EXTI13_PD               ((uint32_t)0x00000030)            /*!< PD[13] pin */
#define AFIO_EXTICR4_EXTI13_PE               ((uint32_t)0x00000040)            /*!< PE[13] pin */
#define AFIO_EXTICR4_EXTI13_PF               ((uint32_t)0x00000050)            /*!< PF[13] pin */
#define AFIO_EXTICR4_EXTI13_PG               ((uint32_t)0x00000060)            /*!< PG[13] pin */

/*!< EXTI14 configuration */  
#define AFIO_EXTICR4_EXTI14_PA               ((uint32_t)0x00000000)            /*!< PA[14] pin */
#define AFIO_EXTICR4_EXTI14_PB               ((uint32_t)0x00000100)            /*!< PB[14] pin */
#define AFIO_EXTICR4_EXTI14_PC               ((uint32_t)0x00000200)            /*!< PC[14] pin */
#define AFIO_EXTICR4_EXTI14_PD               ((uint32_t)0x00000300)            /*!< PD[14] pin */
#define AFIO_EXTICR4_EXTI14_PE               ((uint32_t)0x00000400)            /*!< PE[14] pin */
#define AFIO_EXTICR4_EXTI14_PF               ((uint32_t)0x00000500)            /*!< PF[14] pin */
#define AFIO_EXTICR4_EXTI14_PG               ((uint32_t)0x00000600)            /*!< PG[14] pin */

/*!< EXTI15 configuration */
#define AFIO_EXTICR4_EXTI15_PA               ((uint32_t)0x00000000)            /*!< PA[15] pin */
#define AFIO_EXTICR4_EXTI15_PB               ((uint32_t)0x00001000)            /*!< PB[15] pin */
#define AFIO_EXTICR4_EXTI15_PC               ((uint32_t)0x00002000)            /*!< PC[15] pin */
#define AFIO_EXTICR4_EXTI15_PD               ((uint32_t)0x00003000)            /*!< PD[15] pin */
#define AFIO_EXTICR4_EXTI15_PE               ((uint32_t)0x00004000)            /*!< PE[15] pin */
#define AFIO_EXTICR4_EXTI15_PF               ((uint32_t)0x00005000)            /*!< PF[15] pin */
#define AFIO_EXTICR4_EXTI15_PG               ((uint32_t)0x00006000)            /*!< PG[15] pin */

/******************  Bit definition for AFIO_MAPR2 register  ******************/



/******************************************************************************/
/*                                                                            */
/*                               SystemTick                                   */
/*                                                                            */
/******************************************************************************/

/*****************  Bit definition for SysTick_CTRL register  *****************/
#define  SysTick_CTRL_ENABLE                 ((uint32_t)0x00000001)        /*!< Counter enable */
#define  SysTick_CTRL_TICKINT                ((uint32_t)0x00000002)        /*!< Counting down to 0 pends the SysTick handler */
#define  SysTick_CTRL_CLKSOURCE              ((uint32_t)0x00000004)        /*!< Clock source */
#define  SysTick_CTRL_COUNTFLAG              ((uint32_t)0x00010000)        /*!< Count Flag */

/*****************  Bit definition for SysTick_LOAD register  *****************/
#define  SysTick_LOAD_RELOAD                 ((uint32_t)0x00FFFFFF)        /*!< Value to load into the SysTick Current Value Register when the counter reaches 0 */

/*****************  Bit definition for SysTick_VAL register  ******************/
#define  SysTick_VAL_CURRENT                 ((uint32_t)0x00FFFFFF)        /*!< Current value at the time the register is accessed */

/*****************  Bit definition for SysTick_CALIB register  ****************/
#define  SysTick_CALIB_TENMS                 ((uint32_t)0x00FFFFFF)        /*!< Reload value to use for 10ms timing */
#define  SysTick_CALIB_SKEW                  ((uint32_t)0x40000000)        /*!< Calibration value is not exactly 10 ms */
#define  SysTick_CALIB_NOREF                 ((uint32_t)0x80000000)        /*!< The reference clock is not provided */

/******************************************************************************/
/*                                                                            */
/*                  Nested Vectored Interrupt Controller                      */
/*                                                                            */
/******************************************************************************/

/******************  Bit definition for NVIC_ISER register  *******************/
#define  NVIC_ISER_SETENA                    ((uint32_t)0xFFFFFFFF)        /*!< Interrupt set enable bits */
#define  NVIC_ISER_SETENA_0                  ((uint32_t)0x00000001)        /*!< bit 0 */
#define  NVIC_ISER_SETENA_1                  ((uint32_t)0x00000002)        /*!< bit 1 */
#define  NVIC_ISER_SETENA_2                  ((uint32_t)0x00000004)        /*!< bit 2 */
#define  NVIC_ISER_SETENA_3                  ((uint32_t)0x00000008)        /*!< bit 3 */
#define  NVIC_ISER_SETENA_4                  ((uint32_t)0x00000010)        /*!< bit 4 */
#define  NVIC_ISER_SETENA_5                  ((uint32_t)0x00000020)        /*!< bit 5 */
#define  NVIC_ISER_SETENA_6                  ((uint32_t)0x00000040)        /*!< bit 6 */
#define  NVIC_ISER_SETENA_7                  ((uint32_t)0x00000080)        /*!< bit 7 */
#define  NVIC_ISER_SETENA_8                  ((uint32_t)0x00000100)        /*!< bit 8 */
#define  NVIC_ISER_SETENA_9                  ((uint32_t)0x00000200)        /*!< bit 9 */
#define  NVIC_ISER_SETENA_10                 ((uint32_t)0x00000400)        /*!< bit 10 */
#define  NVIC_ISER_SETENA_11                 ((uint32_t)0x00000800)        /*!< bit 11 */
#define  NVIC_ISER_SETENA_12                 ((uint32_t)0x00001000)        /*!< bit 12 */
#define  NVIC_ISER_SETENA_13                 ((uint32_t)0x00002000)        /*!< bit 13 */
#define  NVIC_ISER_SETENA_14                 ((uint32_t)0x00004000)        /*!< bit 14 */
#define  NVIC_ISER_SETENA_15                 ((uint32_t)0x00008000)        /*!< bit 15 */
#define  NVIC_ISER_SETENA_16                 ((uint32_t)0x00010000)        /*!< bit 16 */
#define  NVIC_ISER_SETENA_17                 ((uint32_t)0x00020000)        /*!< bit 17 */
#define  NVIC_ISER_SETENA_18                 ((uint32_t)0x00040000)        /*!< bit 18 */
#define  NVIC_ISER_SETENA_19                 ((uint32_t)0x00080000)        /*!< bit 19 */
#define  NVIC_ISER_SETENA_20                 ((uint32_t)0x00100000)        /*!< bit 20 */
#define  NVIC_ISER_SETENA_21                 ((uint32_t)0x00200000)        /*!< bit 21 */
#define  NVIC_ISER_SETENA_22                 ((uint32_t)0x00400000)        /*!< bit 22 */
#define  NVIC_ISER_SETENA_23                 ((uint32_t)0x00800000)        /*!< bit 23 */
#define  NVIC_ISER_SETENA_24                 ((uint32_t)0x01000000)        /*!< bit 24 */
#define  NVIC_ISER_SETENA_25                 ((uint32_t)0x02000000)        /*!< bit 25 */
#define  NVIC_ISER_SETENA_26                 ((uint32_t)0x04000000)        /*!< bit 26 */
#define  NVIC_ISER_SETENA_27                 ((uint32_t)0x08000000)        /*!< bit 27 */
#define  NVIC_ISER_SETENA_28                 ((uint32_t)0x10000000)        /*!< bit 28 */
#define  NVIC_ISER_SETENA_29                 ((uint32_t)0x20000000)        /*!< bit 29 */
#define  NVIC_ISER_SETENA_30                 ((uint32_t)0x40000000)        /*!< bit 30 */
#define  NVIC_ISER_SETENA_31                 ((uint32_t)0x80000000)        /*!< bit 31 */

/******************  Bit definition for NVIC_ICER register  *******************/
#define  NVIC_ICER_CLRENA                   ((uint32_t)0xFFFFFFFF)        /*!< Interrupt clear-enable bits */
#define  NVIC_ICER_CLRENA_0                  ((uint32_t)0x00000001)        /*!< bit 0 */
#define  NVIC_ICER_CLRENA_1                  ((uint32_t)0x00000002)        /*!< bit 1 */
#define  NVIC_ICER_CLRENA_2                  ((uint32_t)0x00000004)        /*!< bit 2 */
#define  NVIC_ICER_CLRENA_3                  ((uint32_t)0x00000008)        /*!< bit 3 */
#define  NVIC_ICER_CLRENA_4                  ((uint32_t)0x00000010)        /*!< bit 4 */
#define  NVIC_ICER_CLRENA_5                  ((uint32_t)0x00000020)        /*!< bit 5 */
#define  NVIC_ICER_CLRENA_6                  ((uint32_t)0x00000040)        /*!< bit 6 */
#define  NVIC_ICER_CLRENA_7                  ((uint32_t)0x00000080)        /*!< bit 7 */
#define  NVIC_ICER_CLRENA_8                  ((uint32_t)0x00000100)        /*!< bit 8 */
#define  NVIC_ICER_CLRENA_9                  ((uint32_t)0x00000200)        /*!< bit 9 */
#define  NVIC_ICER_CLRENA_10                 ((uint32_t)0x00000400)        /*!< bit 10 */
#define  NVIC_ICER_CLRENA_11                 ((uint32_t)0x00000800)        /*!< bit 11 */
#define  NVIC_ICER_CLRENA_12                 ((uint32_t)0x00001000)        /*!< bit 12 */
#define  NVIC_ICER_CLRENA_13                 ((uint32_t)0x00002000)        /*!< bit 13 */
#define  NVIC_ICER_CLRENA_14                 ((uint32_t)0x00004000)        /*!< bit 14 */
#define  NVIC_ICER_CLRENA_15                 ((uint32_t)0x00008000)        /*!< bit 15 */
#define  NVIC_ICER_CLRENA_16                 ((uint32_t)0x00010000)        /*!< bit 16 */
#define  NVIC_ICER_CLRENA_17                 ((uint32_t)0x00020000)        /*!< bit 17 */
#define  NVIC_ICER_CLRENA_18                 ((uint32_t)0x00040000)        /*!< bit 18 */
#define  NVIC_ICER_CLRENA_19                 ((uint32_t)0x00080000)        /*!< bit 19 */
#define  NVIC_ICER_CLRENA_20                 ((uint32_t)0x00100000)        /*!< bit 20 */
#define  NVIC_ICER_CLRENA_21                 ((uint32_t)0x00200000)        /*!< bit 21 */
#define  NVIC_ICER_CLRENA_22                 ((uint32_t)0x00400000)        /*!< bit 22 */
#define  NVIC_ICER_CLRENA_23                 ((uint32_t)0x00800000)        /*!< bit 23 */
#define  NVIC_ICER_CLRENA_24                 ((uint32_t)0x01000000)        /*!< bit 24 */
#define  NVIC_ICER_CLRENA_25                 ((uint32_t)0x02000000)        /*!< bit 25 */
#define  NVIC_ICER_CLRENA_26                 ((uint32_t)0x04000000)        /*!< bit 26 */
#define  NVIC_ICER_CLRENA_27                 ((uint32_t)0x08000000)        /*!< bit 27 */
#define  NVIC_ICER_CLRENA_28                 ((uint32_t)0x10000000)        /*!< bit 28 */
#define  NVIC_ICER_CLRENA_29                 ((uint32_t)0x20000000)        /*!< bit 29 */
#define  NVIC_ICER_CLRENA_30                 ((uint32_t)0x40000000)        /*!< bit 30 */
#define  NVIC_ICER_CLRENA_31                 ((uint32_t)0x80000000)        /*!< bit 31 */

/******************  Bit definition for NVIC_ISPR register  *******************/
#define  NVIC_ISPR_SETPEND                   ((uint32_t)0xFFFFFFFF)        /*!< Interrupt set-pending bits */
#define  NVIC_ISPR_SETPEND_0                 ((uint32_t)0x00000001)        /*!< bit 0 */
#define  NVIC_ISPR_SETPEND_1                 ((uint32_t)0x00000002)        /*!< bit 1 */
#define  NVIC_ISPR_SETPEND_2                 ((uint32_t)0x00000004)        /*!< bit 2 */
#define  NVIC_ISPR_SETPEND_3                 ((uint32_t)0x00000008)        /*!< bit 3 */
#define  NVIC_ISPR_SETPEND_4                 ((uint32_t)0x00000010)        /*!< bit 4 */
#define  NVIC_ISPR_SETPEND_5                 ((uint32_t)0x00000020)        /*!< bit 5 */
#define  NVIC_ISPR_SETPEND_6                 ((uint32_t)0x00000040)        /*!< bit 6 */
#define  NVIC_ISPR_SETPEND_7                 ((uint32_t)0x00000080)        /*!< bit 7 */
#define  NVIC_ISPR_SETPEND_8                 ((uint32_t)0x00000100)        /*!< bit 8 */
#define  NVIC_ISPR_SETPEND_9                 ((uint32_t)0x00000200)        /*!< bit 9 */
#define  NVIC_ISPR_SETPEND_10                ((uint32_t)0x00000400)        /*!< bit 10 */
#define  NVIC_ISPR_SETPEND_11                ((uint32_t)0x00000800)        /*!< bit 11 */
#define  NVIC_ISPR_SETPEND_12                ((uint32_t)0x00001000)        /*!< bit 12 */
#define  NVIC_ISPR_SETPEND_13                ((uint32_t)0x00002000)        /*!< bit 13 */
#define  NVIC_ISPR_SETPEND_14                ((uint32_t)0x00004000)        /*!< bit 14 */
#define  NVIC_ISPR_SETPEND_15                ((uint32_t)0x00008000)        /*!< bit 15 */
#define  NVIC_ISPR_SETPEND_16                ((uint32_t)0x00010000)        /*!< bit 16 */
#define  NVIC_ISPR_SETPEND_17                ((uint32_t)0x00020000)        /*!< bit 17 */
#define  NVIC_ISPR_SETPEND_18                ((uint32_t)0x00040000)        /*!< bit 18 */
#define  NVIC_ISPR_SETPEND_19                ((uint32_t)0x00080000)        /*!< bit 19 */
#define  NVIC_ISPR_SETPEND_20                ((uint32_t)0x00100000)        /*!< bit 20 */
#define  NVIC_ISPR_SETPEND_21                ((uint32_t)0x00200000)        /*!< bit 21 */
#define  NVIC_ISPR_SETPEND_22                ((uint32_t)0x00400000)        /*!< bit 22 */
#define  NVIC_ISPR_SETPEND_23                ((uint32_t)0x00800000)        /*!< bit 23 */
#define  NVIC_ISPR_SETPEND_24                ((uint32_t)0x01000000)        /*!< bit 24 */
#define  NVIC_ISPR_SETPEND_25                ((uint32_t)0x02000000)        /*!< bit 25 */
#define  NVIC_ISPR_SETPEND_26                ((uint32_t)0x04000000)        /*!< bit 26 */
#define  NVIC_ISPR_SETPEND_27                ((uint32_t)0x08000000)        /*!< bit 27 */
#define  NVIC_ISPR_SETPEND_28                ((uint32_t)0x10000000)        /*!< bit 28 */
#define  NVIC_ISPR_SETPEND_29                ((uint32_t)0x20000000)        /*!< bit 29 */
#define  NVIC_ISPR_SETPEND_30                ((uint32_t)0x40000000)        /*!< bit 30 */
#define  NVIC_ISPR_SETPEND_31                ((uint32_t)0x80000000)        /*!< bit 31 */

/******************  Bit definition for NVIC_ICPR register  *******************/
#define  NVIC_ICPR_CLRPEND                   ((uint32_t)0xFFFFFFFF)        /*!< Interrupt clear-pending bits */
#define  NVIC_ICPR_CLRPEND_0                 ((uint32_t)0x00000001)        /*!< bit 0 */
#define  NVIC_ICPR_CLRPEND_1                 ((uint32_t)0x00000002)        /*!< bit 1 */
#define  NVIC_ICPR_CLRPEND_2                 ((uint32_t)0x00000004)        /*!< bit 2 */
#define  NVIC_ICPR_CLRPEND_3                 ((uint32_t)0x00000008)        /*!< bit 3 */
#define  NVIC_ICPR_CLRPEND_4                 ((uint32_t)0x00000010)        /*!< bit 4 */
#define  NVIC_ICPR_CLRPEND_5                 ((uint32_t)0x00000020)        /*!< bit 5 */
#define  NVIC_ICPR_CLRPEND_6                 ((uint32_t)0x00000040)        /*!< bit 6 */
#define  NVIC_ICPR_CLRPEND_7                 ((uint32_t)0x00000080)        /*!< bit 7 */
#define  NVIC_ICPR_CLRPEND_8                 ((uint32_t)0x00000100)        /*!< bit 8 */
#define  NVIC_ICPR_CLRPEND_9                 ((uint32_t)0x00000200)        /*!< bit 9 */
#define  NVIC_ICPR_CLRPEND_10                ((uint32_t)0x00000400)        /*!< bit 10 */
#define  NVIC_ICPR_CLRPEND_11                ((uint32_t)0x00000800)        /*!< bit 11 */
#define  NVIC_ICPR_CLRPEND_12                ((uint32_t)0x00001000)        /*!< bit 12 */
#define  NVIC_ICPR_CLRPEND_13                ((uint32_t)0x00002000)        /*!< bit 13 */
#define  NVIC_ICPR_CLRPEND_14                ((uint32_t)0x00004000)        /*!< bit 14 */
#define  NVIC_ICPR_CLRPEND_15                ((uint32_t)0x00008000)        /*!< bit 15 */
#define  NVIC_ICPR_CLRPEND_16                ((uint32_t)0x00010000)        /*!< bit 16 */
#define  NVIC_ICPR_CLRPEND_17                ((uint32_t)0x00020000)        /*!< bit 17 */
#define  NVIC_ICPR_CLRPEND_18                ((uint32_t)0x00040000)        /*!< bit 18 */
#define  NVIC_ICPR_CLRPEND_19                ((uint32_t)0x00080000)        /*!< bit 19 */
#define  NVIC_ICPR_CLRPEND_20                ((uint32_t)0x00100000)        /*!< bit 20 */
#define  NVIC_ICPR_CLRPEND_21                ((uint32_t)0x00200000)        /*!< bit 21 */
#define  NVIC_ICPR_CLRPEND_22                ((uint32_t)0x00400000)        /*!< bit 22 */
#define  NVIC_ICPR_CLRPEND_23                ((uint32_t)0x00800000)        /*!< bit 23 */
#define  NVIC_ICPR_CLRPEND_24                ((uint32_t)0x01000000)        /*!< bit 24 */
#define  NVIC_ICPR_CLRPEND_25                ((uint32_t)0x02000000)        /*!< bit 25 */
#define  NVIC_ICPR_CLRPEND_26                ((uint32_t)0x04000000)        /*!< bit 26 */
#define  NVIC_ICPR_CLRPEND_27                ((uint32_t)0x08000000)        /*!< bit 27 */
#define  NVIC_ICPR_CLRPEND_28                ((uint32_t)0x10000000)        /*!< bit 28 */
#define  NVIC_ICPR_CLRPEND_29                ((uint32_t)0x20000000)        /*!< bit 29 */
#define  NVIC_ICPR_CLRPEND_30                ((uint32_t)0x40000000)        /*!< bit 30 */
#define  NVIC_ICPR_CLRPEND_31                ((uint32_t)0x80000000)        /*!< bit 31 */

/******************  Bit definition for NVIC_IABR register  *******************/
#define  NVIC_IABR_ACTIVE                    ((uint32_t)0xFFFFFFFF)        /*!< Interrupt active flags */
#define  NVIC_IABR_ACTIVE_0                  ((uint32_t)0x00000001)        /*!< bit 0 */
#define  NVIC_IABR_ACTIVE_1                  ((uint32_t)0x00000002)        /*!< bit 1 */
#define  NVIC_IABR_ACTIVE_2                  ((uint32_t)0x00000004)        /*!< bit 2 */
#define  NVIC_IABR_ACTIVE_3                  ((uint32_t)0x00000008)        /*!< bit 3 */
#define  NVIC_IABR_ACTIVE_4                  ((uint32_t)0x00000010)        /*!< bit 4 */
#define  NVIC_IABR_ACTIVE_5                  ((uint32_t)0x00000020)        /*!< bit 5 */
#define  NVIC_IABR_ACTIVE_6                  ((uint32_t)0x00000040)        /*!< bit 6 */
#define  NVIC_IABR_ACTIVE_7                  ((uint32_t)0x00000080)        /*!< bit 7 */
#define  NVIC_IABR_ACTIVE_8                  ((uint32_t)0x00000100)        /*!< bit 8 */
#define  NVIC_IABR_ACTIVE_9                  ((uint32_t)0x00000200)        /*!< bit 9 */
#define  NVIC_IABR_ACTIVE_10                 ((uint32_t)0x00000400)        /*!< bit 10 */
#define  NVIC_IABR_ACTIVE_11                 ((uint32_t)0x00000800)        /*!< bit 11 */
#define  NVIC_IABR_ACTIVE_12                 ((uint32_t)0x00001000)        /*!< bit 12 */
#define  NVIC_IABR_ACTIVE_13                 ((uint32_t)0x00002000)        /*!< bit 13 */
#define  NVIC_IABR_ACTIVE_14                 ((uint32_t)0x00004000)        /*!< bit 14 */
#define  NVIC_IABR_ACTIVE_15                 ((uint32_t)0x00008000)        /*!< bit 15 */
#define  NVIC_IABR_ACTIVE_16                 ((uint32_t)0x00010000)        /*!< bit 16 */
#define  NVIC_IABR_ACTIVE_17                 ((uint32_t)0x00020000)        /*!< bit 17 */
#define  NVIC_IABR_ACTIVE_18                 ((uint32_t)0x00040000)        /*!< bit 18 */
#define  NVIC_IABR_ACTIVE_19                 ((uint32_t)0x00080000)        /*!< bit 19 */
#define  NVIC_IABR_ACTIVE_20                 ((uint32_t)0x00100000)        /*!< bit 20 */
#define  NVIC_IABR_ACTIVE_21                 ((uint32_t)0x00200000)        /*!< bit 21 */
#define  NVIC_IABR_ACTIVE_22                 ((uint32_t)0x00400000)        /*!< bit 22 */
#define  NVIC_IABR_ACTIVE_23                 ((uint32_t)0x00800000)        /*!< bit 23 */
#define  NVIC_IABR_ACTIVE_24                 ((uint32_t)0x01000000)        /*!< bit 24 */
#define  NVIC_IABR_ACTIVE_25                 ((uint32_t)0x02000000)        /*!< bit 25 */
#define  NVIC_IABR_ACTIVE_26                 ((uint32_t)0x04000000)        /*!< bit 26 */
#define  NVIC_IABR_ACTIVE_27                 ((uint32_t)0x08000000)        /*!< bit 27 */
#define  NVIC_IABR_ACTIVE_28                 ((uint32_t)0x10000000)        /*!< bit 28 */
#define  NVIC_IABR_ACTIVE_29                 ((uint32_t)0x20000000)        /*!< bit 29 */
#define  NVIC_IABR_ACTIVE_30                 ((uint32_t)0x40000000)        /*!< bit 30 */
#define  NVIC_IABR_ACTIVE_31                 ((uint32_t)0x80000000)        /*!< bit 31 */

/******************  Bit definition for NVIC_PRI0 register  *******************/
#define  NVIC_IPR0_PRI_0                     ((uint32_t)0x000000FF)        /*!< Priority of interrupt 0 */
#define  NVIC_IPR0_PRI_1                     ((uint32_t)0x0000FF00)        /*!< Priority of interrupt 1 */
#define  NVIC_IPR0_PRI_2                     ((uint32_t)0x00FF0000)        /*!< Priority of interrupt 2 */
#define  NVIC_IPR0_PRI_3                     ((uint32_t)0xFF000000)        /*!< Priority of interrupt 3 */

/******************  Bit definition for NVIC_PRI1 register  *******************/
#define  NVIC_IPR1_PRI_4                     ((uint32_t)0x000000FF)        /*!< Priority of interrupt 4 */
#define  NVIC_IPR1_PRI_5                     ((uint32_t)0x0000FF00)        /*!< Priority of interrupt 5 */
#define  NVIC_IPR1_PRI_6                     ((uint32_t)0x00FF0000)        /*!< Priority of interrupt 6 */
#define  NVIC_IPR1_PRI_7                     ((uint32_t)0xFF000000)        /*!< Priority of interrupt 7 */

/******************  Bit definition for NVIC_PRI2 register  *******************/
#define  NVIC_IPR2_PRI_8                     ((uint32_t)0x000000FF)        /*!< Priority of interrupt 8 */
#define  NVIC_IPR2_PRI_9                     ((uint32_t)0x0000FF00)        /*!< Priority of interrupt 9 */
#define  NVIC_IPR2_PRI_10                    ((uint32_t)0x00FF0000)        /*!< Priority of interrupt 10 */
#define  NVIC_IPR2_PRI_11                    ((uint32_t)0xFF000000)        /*!< Priority of interrupt 11 */

/******************  Bit definition for NVIC_PRI3 register  *******************/
#define  NVIC_IPR3_PRI_12                    ((uint32_t)0x000000FF)        /*!< Priority of interrupt 12 */
#define  NVIC_IPR3_PRI_13                    ((uint32_t)0x0000FF00)        /*!< Priority of interrupt 13 */
#define  NVIC_IPR3_PRI_14                    ((uint32_t)0x00FF0000)        /*!< Priority of interrupt 14 */
#define  NVIC_IPR3_PRI_15                    ((uint32_t)0xFF000000)        /*!< Priority of interrupt 15 */

/******************  Bit definition for NVIC_PRI4 register  *******************/
#define  NVIC_IPR4_PRI_16                    ((uint32_t)0x000000FF)        /*!< Priority of interrupt 16 */
#define  NVIC_IPR4_PRI_17                    ((uint32_t)0x0000FF00)        /*!< Priority of interrupt 17 */
#define  NVIC_IPR4_PRI_18                    ((uint32_t)0x00FF0000)        /*!< Priority of interrupt 18 */
#define  NVIC_IPR4_PRI_19                    ((uint32_t)0xFF000000)        /*!< Priority of interrupt 19 */

/******************  Bit definition for NVIC_PRI5 register  *******************/
#define  NVIC_IPR5_PRI_20                    ((uint32_t)0x000000FF)        /*!< Priority of interrupt 20 */
#define  NVIC_IPR5_PRI_21                    ((uint32_t)0x0000FF00)        /*!< Priority of interrupt 21 */
#define  NVIC_IPR5_PRI_22                    ((uint32_t)0x00FF0000)        /*!< Priority of interrupt 22 */
#define  NVIC_IPR5_PRI_23                    ((uint32_t)0xFF000000)        /*!< Priority of interrupt 23 */

/******************  Bit definition for NVIC_PRI6 register  *******************/
#define  NVIC_IPR6_PRI_24                    ((uint32_t)0x000000FF)        /*!< Priority of interrupt 24 */
#define  NVIC_IPR6_PRI_25                    ((uint32_t)0x0000FF00)        /*!< Priority of interrupt 25 */
#define  NVIC_IPR6_PRI_26                    ((uint32_t)0x00FF0000)        /*!< Priority of interrupt 26 */
#define  NVIC_IPR6_PRI_27                    ((uint32_t)0xFF000000)        /*!< Priority of interrupt 27 */

/******************  Bit definition for NVIC_PRI7 register  *******************/
#define  NVIC_IPR7_PRI_28                    ((uint32_t)0x000000FF)        /*!< Priority of interrupt 28 */
#define  NVIC_IPR7_PRI_29                    ((uint32_t)0x0000FF00)        /*!< Priority of interrupt 29 */
#define  NVIC_IPR7_PRI_30                    ((uint32_t)0x00FF0000)        /*!< Priority of interrupt 30 */
#define  NVIC_IPR7_PRI_31                    ((uint32_t)0xFF000000)        /*!< Priority of interrupt 31 */

/******************  Bit definition for SCB_CPUID register  *******************/
#define  SCB_CPUID_REVISION                  ((uint32_t)0x0000000F)        /*!< Implementation defined revision number */
#define  SCB_CPUID_PARTNO                    ((uint32_t)0x0000FFF0)        /*!< Number of processor within family */
#define  SCB_CPUID_Constant                  ((uint32_t)0x000F0000)        /*!< Reads as 0x0F */
#define  SCB_CPUID_VARIANT                   ((uint32_t)0x00F00000)        /*!< Implementation defined variant number */
#define  SCB_CPUID_IMPLEMENTER               ((uint32_t)0xFF000000)        /*!< Implementer code. ARM is 0x41 */

/*******************  Bit definition for SCB_ICSR register  *******************/
#define  SCB_ICSR_VECTACTIVE                 ((uint32_t)0x000001FF)        /*!< Active ISR number field */
#define  SCB_ICSR_RETTOBASE                  ((uint32_t)0x00000800)        /*!< All active exceptions minus the IPSR_current_exception yields the empty set */
#define  SCB_ICSR_VECTPENDING                ((uint32_t)0x003FF000)        /*!< Pending ISR number field */
#define  SCB_ICSR_ISRPENDING                 ((uint32_t)0x00400000)        /*!< Interrupt pending flag */
#define  SCB_ICSR_ISRPREEMPT                 ((uint32_t)0x00800000)        /*!< It indicates that a pending interrupt becomes active in the next running cycle */
#define  SCB_ICSR_PENDSTCLR                  ((uint32_t)0x02000000)        /*!< Clear pending SysTick bit */
#define  SCB_ICSR_PENDSTSET                  ((uint32_t)0x04000000)        /*!< Set pending SysTick bit */
#define  SCB_ICSR_PENDSVCLR                  ((uint32_t)0x08000000)        /*!< Clear pending pendSV bit */
#define  SCB_ICSR_PENDSVSET                  ((uint32_t)0x10000000)        /*!< Set pending pendSV bit */
#define  SCB_ICSR_NMIPENDSET                 ((uint32_t)0x80000000)        /*!< Set pending NMI bit */

/*******************  Bit definition for SCB_VTOR register  *******************/
#define  SCB_VTOR_TBLOFF                     ((uint32_t)0x1FFFFF80)        /*!< Vector table base offset field */
#define  SCB_VTOR_TBLBASE                    ((uint32_t)0x20000000)        /*!< Table base in code(0) or RAM(1) */

/*!<*****************  Bit definition for SCB_AIRCR register  *******************/
#define  SCB_AIRCR_VECTRESET                 ((uint32_t)0x00000001)        /*!< System Reset bit */
#define  SCB_AIRCR_VECTCLRACTIVE             ((uint32_t)0x00000002)        /*!< Clear active vector bit */
#define  SCB_AIRCR_SYSRESETREQ               ((uint32_t)0x00000004)        /*!< Requests chip control logic to generate a reset */

#define  SCB_AIRCR_PRIGROUP                  ((uint32_t)0x00000700)        /*!< PRIGROUP[2:0] bits (Priority group) */
#define  SCB_AIRCR_PRIGROUP_0                ((uint32_t)0x00000100)        /*!< Bit 0 */
#define  SCB_AIRCR_PRIGROUP_1                ((uint32_t)0x00000200)        /*!< Bit 1 */
#define  SCB_AIRCR_PRIGROUP_2                ((uint32_t)0x00000400)        /*!< Bit 2  */

/* prority group configuration */
#define  SCB_AIRCR_PRIGROUP0                 ((uint32_t)0x00000000)        /*!< Priority group=0 (7 bits of pre-emption priority, 1 bit of subpriority) */
#define  SCB_AIRCR_PRIGROUP1                 ((uint32_t)0x00000100)        /*!< Priority group=1 (6 bits of pre-emption priority, 2 bits of subpriority) */
#define  SCB_AIRCR_PRIGROUP2                 ((uint32_t)0x00000200)        /*!< Priority group=2 (5 bits of pre-emption priority, 3 bits of subpriority) */
#define  SCB_AIRCR_PRIGROUP3                 ((uint32_t)0x00000300)        /*!< Priority group=3 (4 bits of pre-emption priority, 4 bits of subpriority) */
#define  SCB_AIRCR_PRIGROUP4                 ((uint32_t)0x00000400)        /*!< Priority group=4 (3 bits of pre-emption priority, 5 bits of subpriority) */
#define  SCB_AIRCR_PRIGROUP5                 ((uint32_t)0x00000500)        /*!< Priority group=5 (2 bits of pre-emption priority, 6 bits of subpriority) */
#define  SCB_AIRCR_PRIGROUP6                 ((uint32_t)0x00000600)        /*!< Priority group=6 (1 bit of pre-emption priority, 7 bits of subpriority) */
#define  SCB_AIRCR_PRIGROUP7                 ((uint32_t)0x00000700)        /*!< Priority group=7 (no pre-emption priority, 8 bits of subpriority) */

#define  SCB_AIRCR_ENDIANESS                 ((uint32_t)0x00008000)        /*!< Data endianness bit */
#define  SCB_AIRCR_VECTKEY                   ((uint32_t)0xFFFF0000)        /*!< Register key (VECTKEY) - Reads as 0xFA05 (VECTKEYSTAT) */

/*******************  Bit definition for SCB_SCR register  ********************/
#define  SCB_SCR_SLEEPONEXIT                 ((uint32_t)0x00000002)        /*!< Sleep on exit bit */
#define  SCB_SCR_SLEEPDEEP                   ((uint32_t)0x00000004)        /*!< Sleep deep bit */
#define  SCB_SCR_SEVONPEND                   ((uint32_t)0x00000010)        /*!< Wake up from WFE */

/********************  Bit definition for SCB_CCR register  *******************/
#define  SCB_CCR_NONBASETHRDENA              ((uint32_t)0x00000001)        /*!< Thread mode can be entered from any level in Handler mode by controlled return value */
#define  SCB_CCR_USERSETMPEND                ((uint32_t)0x00000002)        /*!< Enables user code to write the Software Trigger Interrupt register to trigger (pend) a Main exception */
#define  SCB_CCR_UNALIGN_TRP                 ((uint32_t)0x00000008)        /*!< Trap for unaligned access */
#define  SCB_CCR_DIV_0_TRP                   ((uint32_t)0x00000010)        /*!< Trap on Divide by 0 */
#define  SCB_CCR_BFHFNMIGN                   ((uint32_t)0x00000100)        /*!< Handlers running at priority -1 and -2 */
#define  SCB_CCR_STKALIGN                    ((uint32_t)0x00000200)        /*!< On exception entry, the SP used prior to the exception is adjusted to be 8-byte aligned */

/*******************  Bit definition for SCB_SHPR register ********************/
#define  SCB_SHPR_PRI_N                      ((uint32_t)0x000000FF)        /*!< Priority of system handler 4,8, and 12. Mem Manage, reserved and Debug Monitor */
#define  SCB_SHPR_PRI_N1                     ((uint32_t)0x0000FF00)        /*!< Priority of system handler 5,9, and 13. Bus Fault, reserved and reserved */
#define  SCB_SHPR_PRI_N2                     ((uint32_t)0x00FF0000)        /*!< Priority of system handler 6,10, and 14. Usage Fault, reserved and PendSV */
#define  SCB_SHPR_PRI_N3                     ((uint32_t)0xFF000000)        /*!< Priority of system handler 7,11, and 15. Reserved, SVCall and SysTick */

/******************  Bit definition for SCB_SHCSR register  *******************/
#define  SCB_SHCSR_MEMFAULTACT               ((uint32_t)0x00000001)        /*!< MemManage is active */
#define  SCB_SHCSR_BUSFAULTACT               ((uint32_t)0x00000002)        /*!< BusFault is active */
#define  SCB_SHCSR_USGFAULTACT               ((uint32_t)0x00000008)        /*!< UsageFault is active */
#define  SCB_SHCSR_SVCALLACT                 ((uint32_t)0x00000080)        /*!< SVCall is active */
#define  SCB_SHCSR_MONITORACT                ((uint32_t)0x00000100)        /*!< Monitor is active */
#define  SCB_SHCSR_PENDSVACT                 ((uint32_t)0x00000400)        /*!< PendSV is active */
#define  SCB_SHCSR_SYSTICKACT                ((uint32_t)0x00000800)        /*!< SysTick is active */
#define  SCB_SHCSR_USGFAULTPENDED            ((uint32_t)0x00001000)        /*!< Usage Fault is pended */
#define  SCB_SHCSR_MEMFAULTPENDED            ((uint32_t)0x00002000)        /*!< MemManage is pended */
#define  SCB_SHCSR_BUSFAULTPENDED            ((uint32_t)0x00004000)        /*!< Bus Fault is pended */
#define  SCB_SHCSR_SVCALLPENDED              ((uint32_t)0x00008000)        /*!< SVCall is pended */
#define  SCB_SHCSR_MEMFAULTENA               ((uint32_t)0x00010000)        /*!< MemManage enable */
#define  SCB_SHCSR_BUSFAULTENA               ((uint32_t)0x00020000)        /*!< Bus Fault enable */
#define  SCB_SHCSR_USGFAULTENA               ((uint32_t)0x00040000)        /*!< UsageFault enable */

/*******************  Bit definition for SCB_CFSR register  *******************/
/*!< MFSR */
#define  SCB_CFSR_IACCVIOL                   ((uint32_t)0x00000001)        /*!< Instruction access violation */
#define  SCB_CFSR_DACCVIOL                   ((uint32_t)0x00000002)        /*!< Data access violation */
#define  SCB_CFSR_MUNSTKERR                  ((uint32_t)0x00000008)        /*!< Unstacking error */
#define  SCB_CFSR_MSTKERR                    ((uint32_t)0x00000010)        /*!< Stacking error */
#define  SCB_CFSR_MMARVALID                  ((uint32_t)0x00000080)        /*!< Memory Manage Address Register address valid flag */
/*!< BFSR */
#define  SCB_CFSR_IBUSERR                    ((uint32_t)0x00000100)        /*!< Instruction bus error flag */
#define  SCB_CFSR_PRECISERR                  ((uint32_t)0x00000200)        /*!< Precise data bus error */
#define  SCB_CFSR_IMPRECISERR                ((uint32_t)0x00000400)        /*!< Imprecise data bus error */
#define  SCB_CFSR_UNSTKERR                   ((uint32_t)0x00000800)        /*!< Unstacking error */
#define  SCB_CFSR_STKERR                     ((uint32_t)0x00001000)        /*!< Stacking error */
#define  SCB_CFSR_BFARVALID                  ((uint32_t)0x00008000)        /*!< Bus Fault Address Register address valid flag */
/*!< UFSR */
#define  SCB_CFSR_UNDEFINSTR                 ((uint32_t)0x00010000)        /*!< The processor attempt to execute an undefined instruction */
#define  SCB_CFSR_INVSTATE                   ((uint32_t)0x00020000)        /*!< Invalid combination of EPSR and instruction */
#define  SCB_CFSR_INVPC                      ((uint32_t)0x00040000)        /*!< Attempt to load EXC_RETURN into pc illegally */
#define  SCB_CFSR_NOCP                       ((uint32_t)0x00080000)        /*!< Attempt to use a coprocessor instruction */
#define  SCB_CFSR_UNALIGNED                  ((uint32_t)0x01000000)        /*!< Fault occurs when there is an attempt to make an unaligned memory access */
#define  SCB_CFSR_DIVBYZERO                  ((uint32_t)0x02000000)        /*!< Fault occurs when SDIV or DIV instruction is used with a divisor of 0 */

/*******************  Bit definition for SCB_HFSR register  *******************/
#define  SCB_HFSR_VECTTBL                    ((uint32_t)0x00000002)        /*!< Fault occurs because of vector table read on exception processing */
#define  SCB_HFSR_FORCED                     ((uint32_t)0x40000000)        /*!< Hard Fault activated when a configurable Fault was received and cannot activate */
#define  SCB_HFSR_DEBUGEVT                   ((uint32_t)0x80000000)        /*!< Fault related to debug */

/*******************  Bit definition for SCB_DFSR register  *******************/
#define  SCB_DFSR_HALTED                     ((uint32_t)0x00000001)        /*!< Halt request flag */
#define  SCB_DFSR_BKPT                       ((uint32_t)0x00000002)        /*!< BKPT flag */
#define  SCB_DFSR_DWTTRAP                    ((uint32_t)0x00000004)        /*!< Data Watchpoint and Trace (DWT) flag */
#define  SCB_DFSR_VCATCH                     ((uint32_t)0x00000008)        /*!< Vector catch flag */
#define  SCB_DFSR_EXTERNAL                   ((uint32_t)0x00000010)        /*!< External debug request flag */

/*******************  Bit definition for SCB_MMFAR register  ******************/
#define  SCB_MMFAR_ADDRESS                   ((uint32_t)0xFFFFFFFF)        /*!< Mem Manage fault address field */

/*******************  Bit definition for SCB_BFAR register  *******************/
#define  SCB_BFAR_ADDRESS                    ((uint32_t)0xFFFFFFFF)        /*!< Bus fault address field */

/*******************  Bit definition for SCB_afsr register  *******************/
#define  SCB_AFSR_IMPDEF                     ((uint32_t)0xFFFFFFFF)        /*!< Implementation defined */

/******************************************************************************/
/*                                                                            */
/*                    External Interrupt/Event Controller                     */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for EXTI_IMR register  *******************/
#define  EXTI_IMR_MR0                        ((uint32_t)0x00000001)        /*!< Interrupt Mask on line 0 */
#define  EXTI_IMR_MR1                        ((uint32_t)0x00000002)        /*!< Interrupt Mask on line 1 */
#define  EXTI_IMR_MR2                        ((uint32_t)0x00000004)        /*!< Interrupt Mask on line 2 */
#define  EXTI_IMR_MR3                        ((uint32_t)0x00000008)        /*!< Interrupt Mask on line 3 */
#define  EXTI_IMR_MR4                        ((uint32_t)0x00000010)        /*!< Interrupt Mask on line 4 */
#define  EXTI_IMR_MR5                        ((uint32_t)0x00000020)        /*!< Interrupt Mask on line 5 */
#define  EXTI_IMR_MR6                        ((uint32_t)0x00000040)        /*!< Interrupt Mask on line 6 */
#define  EXTI_IMR_MR7                        ((uint32_t)0x00000080)        /*!< Interrupt Mask on line 7 */
#define  EXTI_IMR_MR8                        ((uint32_t)0x00000100)        /*!< Interrupt Mask on line 8 */
#define  EXTI_IMR_MR9                        ((uint32_t)0x00000200)        /*!< Interrupt Mask on line 9 */
#define  EXTI_IMR_MR10                       ((uint32_t)0x00000400)        /*!< Interrupt Mask on line 10 */
#define  EXTI_IMR_MR11                       ((uint32_t)0x00000800)        /*!< Interrupt Mask on line 11 */
#define  EXTI_IMR_MR12                       ((uint32_t)0x00001000)        /*!< Interrupt Mask on line 12 */
#define  EXTI_IMR_MR13                       ((uint32_t)0x00002000)        /*!< Interrupt Mask on line 13 */
#define  EXTI_IMR_MR14                       ((uint32_t)0x00004000)        /*!< Interrupt Mask on line 14 */
#define  EXTI_IMR_MR15                       ((uint32_t)0x00008000)        /*!< Interrupt Mask on line 15 */
#define  EXTI_IMR_MR16                       ((uint32_t)0x00010000)        /*!< Interrupt Mask on line 16 */
#define  EXTI_IMR_MR17                       ((uint32_t)0x00020000)        /*!< Interrupt Mask on line 17 */
#define  EXTI_IMR_MR18                       ((uint32_t)0x00040000)        /*!< Interrupt Mask on line 18 */
#define  EXTI_IMR_MR19                       ((uint32_t)0x00080000)        /*!< Interrupt Mask on line 19 */

/*******************  Bit definition for EXTI_EMR register  *******************/
#define  EXTI_EMR_MR0                        ((uint32_t)0x00000001)        /*!< Event Mask on line 0 */
#define  EXTI_EMR_MR1                        ((uint32_t)0x00000002)        /*!< Event Mask on line 1 */
#define  EXTI_EMR_MR2                        ((uint32_t)0x00000004)        /*!< Event Mask on line 2 */
#define  EXTI_EMR_MR3                        ((uint32_t)0x00000008)        /*!< Event Mask on line 3 */
#define  EXTI_EMR_MR4                        ((uint32_t)0x00000010)        /*!< Event Mask on line 4 */
#define  EXTI_EMR_MR5                        ((uint32_t)0x00000020)        /*!< Event Mask on line 5 */
#define  EXTI_EMR_MR6                        ((uint32_t)0x00000040)        /*!< Event Mask on line 6 */
#define  EXTI_EMR_MR7                        ((uint32_t)0x00000080)        /*!< Event Mask on line 7 */
#define  EXTI_EMR_MR8                        ((uint32_t)0x00000100)        /*!< Event Mask on line 8 */
#define  EXTI_EMR_MR9                        ((uint32_t)0x00000200)        /*!< Event Mask on line 9 */
#define  EXTI_EMR_MR10                       ((uint32_t)0x00000400)        /*!< Event Mask on line 10 */
#define  EXTI_EMR_MR11                       ((uint32_t)0x00000800)        /*!< Event Mask on line 11 */
#define  EXTI_EMR_MR12                       ((uint32_t)0x00001000)        /*!< Event Mask on line 12 */
#define  EXTI_EMR_MR13                       ((uint32_t)0x00002000)        /*!< Event Mask on line 13 */
#define  EXTI_EMR_MR14                       ((uint32_t)0x00004000)        /*!< Event Mask on line 14 */
#define  EXTI_EMR_MR15                       ((uint32_t)0x00008000)        /*!< Event Mask on line 15 */
#define  EXTI_EMR_MR16                       ((uint32_t)0x00010000)        /*!< Event Mask on line 16 */
#define  EXTI_EMR_MR17                       ((uint32_t)0x00020000)        /*!< Event Mask on line 17 */
#define  EXTI_EMR_MR18                       ((uint32_t)0x00040000)        /*!< Event Mask on line 18 */
#define  EXTI_EMR_MR19                       ((uint32_t)0x00080000)        /*!< Event Mask on line 19 */

/******************  Bit definition for EXTI_RTSR register  *******************/
#define  EXTI_RTSR_TR0                       ((uint32_t)0x00000001)        /*!< Rising trigger event configuration bit of line 0 */
#define  EXTI_RTSR_TR1                       ((uint32_t)0x00000002)        /*!< Rising trigger event configuration bit of line 1 */
#define  EXTI_RTSR_TR2                       ((uint32_t)0x00000004)        /*!< Rising trigger event configuration bit of line 2 */
#define  EXTI_RTSR_TR3                       ((uint32_t)0x00000008)        /*!< Rising trigger event configuration bit of line 3 */
#define  EXTI_RTSR_TR4                       ((uint32_t)0x00000010)        /*!< Rising trigger event configuration bit of line 4 */
#define  EXTI_RTSR_TR5                       ((uint32_t)0x00000020)        /*!< Rising trigger event configuration bit of line 5 */
#define  EXTI_RTSR_TR6                       ((uint32_t)0x00000040)        /*!< Rising trigger event configuration bit of line 6 */
#define  EXTI_RTSR_TR7                       ((uint32_t)0x00000080)        /*!< Rising trigger event configuration bit of line 7 */
#define  EXTI_RTSR_TR8                       ((uint32_t)0x00000100)        /*!< Rising trigger event configuration bit of line 8 */
#define  EXTI_RTSR_TR9                       ((uint32_t)0x00000200)        /*!< Rising trigger event configuration bit of line 9 */
#define  EXTI_RTSR_TR10                      ((uint32_t)0x00000400)        /*!< Rising trigger event configuration bit of line 10 */
#define  EXTI_RTSR_TR11                      ((uint32_t)0x00000800)        /*!< Rising trigger event configuration bit of line 11 */
#define  EXTI_RTSR_TR12                      ((uint32_t)0x00001000)        /*!< Rising trigger event configuration bit of line 12 */
#define  EXTI_RTSR_TR13                      ((uint32_t)0x00002000)        /*!< Rising trigger event configuration bit of line 13 */
#define  EXTI_RTSR_TR14                      ((uint32_t)0x00004000)        /*!< Rising trigger event configuration bit of line 14 */
#define  EXTI_RTSR_TR15                      ((uint32_t)0x00008000)        /*!< Rising trigger event configuration bit of line 15 */
#define  EXTI_RTSR_TR16                      ((uint32_t)0x00010000)        /*!< Rising trigger event configuration bit of line 16 */
#define  EXTI_RTSR_TR17                      ((uint32_t)0x00020000)        /*!< Rising trigger event configuration bit of line 17 */
#define  EXTI_RTSR_TR18                      ((uint32_t)0x00040000)        /*!< Rising trigger event configuration bit of line 18 */
#define  EXTI_RTSR_TR19                      ((uint32_t)0x00080000)        /*!< Rising trigger event configuration bit of line 19 */

/******************  Bit definition for EXTI_FTSR register  *******************/
#define  EXTI_FTSR_TR0                       ((uint32_t)0x00000001)        /*!< Falling trigger event configuration bit of line 0 */
#define  EXTI_FTSR_TR1                       ((uint32_t)0x00000002)        /*!< Falling trigger event configuration bit of line 1 */
#define  EXTI_FTSR_TR2                       ((uint32_t)0x00000004)        /*!< Falling trigger event configuration bit of line 2 */
#define  EXTI_FTSR_TR3                       ((uint32_t)0x00000008)        /*!< Falling trigger event configuration bit of line 3 */
#define  EXTI_FTSR_TR4                       ((uint32_t)0x00000010)        /*!< Falling trigger event configuration bit of line 4 */
#define  EXTI_FTSR_TR5                       ((uint32_t)0x00000020)        /*!< Falling trigger event configuration bit of line 5 */
#define  EXTI_FTSR_TR6                       ((uint32_t)0x00000040)        /*!< Falling trigger event configuration bit of line 6 */
#define  EXTI_FTSR_TR7                       ((uint32_t)0x00000080)        /*!< Falling trigger event configuration bit of line 7 */
#define  EXTI_FTSR_TR8                       ((uint32_t)0x00000100)        /*!< Falling trigger event configuration bit of line 8 */
#define  EXTI_FTSR_TR9                       ((uint32_t)0x00000200)        /*!< Falling trigger event configuration bit of line 9 */
#define  EXTI_FTSR_TR10                      ((uint32_t)0x00000400)        /*!< Falling trigger event configuration bit of line 10 */
#define  EXTI_FTSR_TR11                      ((uint32_t)0x00000800)        /*!< Falling trigger event configuration bit of line 11 */
#define  EXTI_FTSR_TR12                      ((uint32_t)0x00001000)        /*!< Falling trigger event configuration bit of line 12 */
#define  EXTI_FTSR_TR13                      ((uint32_t)0x00002000)        /*!< Falling trigger event configuration bit of line 13 */
#define  EXTI_FTSR_TR14                      ((uint32_t)0x00004000)        /*!< Falling trigger event configuration bit of line 14 */
#define  EXTI_FTSR_TR15                      ((uint32_t)0x00008000)        /*!< Falling trigger event configuration bit of line 15 */
#define  EXTI_FTSR_TR16                      ((uint32_t)0x00010000)        /*!< Falling trigger event configuration bit of line 16 */
#define  EXTI_FTSR_TR17                      ((uint32_t)0x00020000)        /*!< Falling trigger event configuration bit of line 17 */
#define  EXTI_FTSR_TR18                      ((uint32_t)0x00040000)        /*!< Falling trigger event configuration bit of line 18 */
#define  EXTI_FTSR_TR19                      ((uint32_t)0x00080000)        /*!< Falling trigger event configuration bit of line 19 */

/******************  Bit definition for EXTI_SWIER register  ******************/
#define  EXTI_SWIER_SWIER0                   ((uint32_t)0x00000001)        /*!< Software Interrupt on line 0 */
#define  EXTI_SWIER_SWIER1                   ((uint32_t)0x00000002)        /*!< Software Interrupt on line 1 */
#define  EXTI_SWIER_SWIER2                   ((uint32_t)0x00000004)        /*!< Software Interrupt on line 2 */
#define  EXTI_SWIER_SWIER3                   ((uint32_t)0x00000008)        /*!< Software Interrupt on line 3 */
#define  EXTI_SWIER_SWIER4                   ((uint32_t)0x00000010)        /*!< Software Interrupt on line 4 */
#define  EXTI_SWIER_SWIER5                   ((uint32_t)0x00000020)        /*!< Software Interrupt on line 5 */
#define  EXTI_SWIER_SWIER6                   ((uint32_t)0x00000040)        /*!< Software Interrupt on line 6 */
#define  EXTI_SWIER_SWIER7                   ((uint32_t)0x00000080)        /*!< Software Interrupt on line 7 */
#define  EXTI_SWIER_SWIER8                   ((uint32_t)0x00000100)        /*!< Software Interrupt on line 8 */
#define  EXTI_SWIER_SWIER9                   ((uint32_t)0x00000200)        /*!< Software Interrupt on line 9 */
#define  EXTI_SWIER_SWIER10                  ((uint32_t)0x00000400)        /*!< Software Interrupt on line 10 */
#define  EXTI_SWIER_SWIER11                  ((uint32_t)0x00000800)        /*!< Software Interrupt on line 11 */
#define  EXTI_SWIER_SWIER12                  ((uint32_t)0x00001000)        /*!< Software Interrupt on line 12 */
#define  EXTI_SWIER_SWIER13                  ((uint32_t)0x00002000)        /*!< Software Interrupt on line 13 */
#define  EXTI_SWIER_SWIER14                  ((uint32_t)0x00004000)        /*!< Software Interrupt on line 14 */
#define  EXTI_SWIER_SWIER15                  ((uint32_t)0x00008000)        /*!< Software Interrupt on line 15 */
#define  EXTI_SWIER_SWIER16                  ((uint32_t)0x00010000)        /*!< Software Interrupt on line 16 */
#define  EXTI_SWIER_SWIER17                  ((uint32_t)0x00020000)        /*!< Software Interrupt on line 17 */
#define  EXTI_SWIER_SWIER18                  ((uint32_t)0x00040000)        /*!< Software Interrupt on line 18 */
#define  EXTI_SWIER_SWIER19                  ((uint32_t)0x00080000)        /*!< Software Interrupt on line 19 */

/*******************  Bit definition for EXTI_PR register  ********************/
#define  EXTI_PR_PR0                         ((uint32_t)0x00000001)        /*!< Pending bit for line 0 */
#define  EXTI_PR_PR1                         ((uint32_t)0x00000002)        /*!< Pending bit for line 1 */
#define  EXTI_PR_PR2                         ((uint32_t)0x00000004)        /*!< Pending bit for line 2 */
#define  EXTI_PR_PR3                         ((uint32_t)0x00000008)        /*!< Pending bit for line 3 */
#define  EXTI_PR_PR4                         ((uint32_t)0x00000010)        /*!< Pending bit for line 4 */
#define  EXTI_PR_PR5                         ((uint32_t)0x00000020)        /*!< Pending bit for line 5 */
#define  EXTI_PR_PR6                         ((uint32_t)0x00000040)        /*!< Pending bit for line 6 */
#define  EXTI_PR_PR7                         ((uint32_t)0x00000080)        /*!< Pending bit for line 7 */
#define  EXTI_PR_PR8                         ((uint32_t)0x00000100)        /*!< Pending bit for line 8 */
#define  EXTI_PR_PR9                         ((uint32_t)0x00000200)        /*!< Pending bit for line 9 */
#define  EXTI_PR_PR10                        ((uint32_t)0x00000400)        /*!< Pending bit for line 10 */
#define  EXTI_PR_PR11                        ((uint32_t)0x00000800)        /*!< Pending bit for line 11 */
#define  EXTI_PR_PR12                        ((uint32_t)0x00001000)        /*!< Pending bit for line 12 */
#define  EXTI_PR_PR13                        ((uint32_t)0x00002000)        /*!< Pending bit for line 13 */
#define  EXTI_PR_PR14                        ((uint32_t)0x00004000)        /*!< Pending bit for line 14 */
#define  EXTI_PR_PR15                        ((uint32_t)0x00008000)        /*!< Pending bit for line 15 */
#define  EXTI_PR_PR16                        ((uint32_t)0x00010000)        /*!< Pending bit for line 16 */
#define  EXTI_PR_PR17                        ((uint32_t)0x00020000)        /*!< Pending bit for line 17 */
#define  EXTI_PR_PR18                        ((uint32_t)0x00040000)        /*!< Pending bit for line 18 */
#define  EXTI_PR_PR19                        ((uint32_t)0x00080000)        /*!< Pending bit for line 19 */

/******************************************************************************/
/*                                                                            */
/*                             DMA Controller                                 */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for DMA_ISR register  ********************/
#define  DMA_ISR_GIF1                        ((uint32_t)0x00000001)        /*!< Channel 1 Global interrupt flag */
#define  DMA_ISR_TCIF1                       ((uint32_t)0x00000002)        /*!< Channel 1 Transfer Complete flag */
#define  DMA_ISR_HTIF1                       ((uint32_t)0x00000004)        /*!< Channel 1 Half Transfer flag */
#define  DMA_ISR_TEIF1                       ((uint32_t)0x00000008)        /*!< Channel 1 Transfer Error flag */
#define  DMA_ISR_GIF2                        ((uint32_t)0x00000010)        /*!< Channel 2 Global interrupt flag */
#define  DMA_ISR_TCIF2                       ((uint32_t)0x00000020)        /*!< Channel 2 Transfer Complete flag */
#define  DMA_ISR_HTIF2                       ((uint32_t)0x00000040)        /*!< Channel 2 Half Transfer flag */
#define  DMA_ISR_TEIF2                       ((uint32_t)0x00000080)        /*!< Channel 2 Transfer Error flag */
#define  DMA_ISR_GIF3                        ((uint32_t)0x00000100)        /*!< Channel 3 Global interrupt flag */
#define  DMA_ISR_TCIF3                       ((uint32_t)0x00000200)        /*!< Channel 3 Transfer Complete flag */
#define  DMA_ISR_HTIF3                       ((uint32_t)0x00000400)        /*!< Channel 3 Half Transfer flag */
#define  DMA_ISR_TEIF3                       ((uint32_t)0x00000800)        /*!< Channel 3 Transfer Error flag */
#define  DMA_ISR_GIF4                        ((uint32_t)0x00001000)        /*!< Channel 4 Global interrupt flag */
#define  DMA_ISR_TCIF4                       ((uint32_t)0x00002000)        /*!< Channel 4 Transfer Complete flag */
#define  DMA_ISR_HTIF4                       ((uint32_t)0x00004000)        /*!< Channel 4 Half Transfer flag */
#define  DMA_ISR_TEIF4                       ((uint32_t)0x00008000)        /*!< Channel 4 Transfer Error flag */
#define  DMA_ISR_GIF5                        ((uint32_t)0x00010000)        /*!< Channel 5 Global interrupt flag */
#define  DMA_ISR_TCIF5                       ((uint32_t)0x00020000)        /*!< Channel 5 Transfer Complete flag */
#define  DMA_ISR_HTIF5                       ((uint32_t)0x00040000)        /*!< Channel 5 Half Transfer flag */
#define  DMA_ISR_TEIF5                       ((uint32_t)0x00080000)        /*!< Channel 5 Transfer Error flag */
#define  DMA_ISR_GIF6                        ((uint32_t)0x00100000)        /*!< Channel 6 Global interrupt flag */
#define  DMA_ISR_TCIF6                       ((uint32_t)0x00200000)        /*!< Channel 6 Transfer Complete flag */
#define  DMA_ISR_HTIF6                       ((uint32_t)0x00400000)        /*!< Channel 6 Half Transfer flag */
#define  DMA_ISR_TEIF6                       ((uint32_t)0x00800000)        /*!< Channel 6 Transfer Error flag */
#define  DMA_ISR_GIF7                        ((uint32_t)0x01000000)        /*!< Channel 7 Global interrupt flag */
#define  DMA_ISR_TCIF7                       ((uint32_t)0x02000000)        /*!< Channel 7 Transfer Complete flag */
#define  DMA_ISR_HTIF7                       ((uint32_t)0x04000000)        /*!< Channel 7 Half Transfer flag */
#define  DMA_ISR_TEIF7                       ((uint32_t)0x08000000)        /*!< Channel 7 Transfer Error flag */

/*******************  Bit definition for DMA_IFCR register  *******************/
#define  DMA_IFCR_CGIF1                      ((uint32_t)0x00000001)        /*!< Channel 1 Global interrupt clear */
#define  DMA_IFCR_CTCIF1                     ((uint32_t)0x00000002)        /*!< Channel 1 Transfer Complete clear */
#define  DMA_IFCR_CHTIF1                     ((uint32_t)0x00000004)        /*!< Channel 1 Half Transfer clear */
#define  DMA_IFCR_CTEIF1                     ((uint32_t)0x00000008)        /*!< Channel 1 Transfer Error clear */
#define  DMA_IFCR_CGIF2                      ((uint32_t)0x00000010)        /*!< Channel 2 Global interrupt clear */
#define  DMA_IFCR_CTCIF2                     ((uint32_t)0x00000020)        /*!< Channel 2 Transfer Complete clear */
#define  DMA_IFCR_CHTIF2                     ((uint32_t)0x00000040)        /*!< Channel 2 Half Transfer clear */
#define  DMA_IFCR_CTEIF2                     ((uint32_t)0x00000080)        /*!< Channel 2 Transfer Error clear */
#define  DMA_IFCR_CGIF3                      ((uint32_t)0x00000100)        /*!< Channel 3 Global interrupt clear */
#define  DMA_IFCR_CTCIF3                     ((uint32_t)0x00000200)        /*!< Channel 3 Transfer Complete clear */
#define  DMA_IFCR_CHTIF3                     ((uint32_t)0x00000400)        /*!< Channel 3 Half Transfer clear */
#define  DMA_IFCR_CTEIF3                     ((uint32_t)0x00000800)        /*!< Channel 3 Transfer Error clear */
#define  DMA_IFCR_CGIF4                      ((uint32_t)0x00001000)        /*!< Channel 4 Global interrupt clear */
#define  DMA_IFCR_CTCIF4                     ((uint32_t)0x00002000)        /*!< Channel 4 Transfer Complete clear */
#define  DMA_IFCR_CHTIF4                     ((uint32_t)0x00004000)        /*!< Channel 4 Half Transfer clear */
#define  DMA_IFCR_CTEIF4                     ((uint32_t)0x00008000)        /*!< Channel 4 Transfer Error clear */
#define  DMA_IFCR_CGIF5                      ((uint32_t)0x00010000)        /*!< Channel 5 Global interrupt clear */
#define  DMA_IFCR_CTCIF5                     ((uint32_t)0x00020000)        /*!< Channel 5 Transfer Complete clear */
#define  DMA_IFCR_CHTIF5                     ((uint32_t)0x00040000)        /*!< Channel 5 Half Transfer clear */
#define  DMA_IFCR_CTEIF5                     ((uint32_t)0x00080000)        /*!< Channel 5 Transfer Error clear */
#define  DMA_IFCR_CGIF6                      ((uint32_t)0x00100000)        /*!< Channel 6 Global interrupt clear */
#define  DMA_IFCR_CTCIF6                     ((uint32_t)0x00200000)        /*!< Channel 6 Transfer Complete clear */
#define  DMA_IFCR_CHTIF6                     ((uint32_t)0x00400000)        /*!< Channel 6 Half Transfer clear */
#define  DMA_IFCR_CTEIF6                     ((uint32_t)0x00800000)        /*!< Channel 6 Transfer Error clear */
#define  DMA_IFCR_CGIF7                      ((uint32_t)0x01000000)        /*!< Channel 7 Global interrupt clear */
#define  DMA_IFCR_CTCIF7                     ((uint32_t)0x02000000)        /*!< Channel 7 Transfer Complete clear */
#define  DMA_IFCR_CHTIF7                     ((uint32_t)0x04000000)        /*!< Channel 7 Half Transfer clear */
#define  DMA_IFCR_CTEIF7                     ((uint32_t)0x08000000)        /*!< Channel 7 Transfer Error clear */

/*******************  Bit definition for DMA_CCR register   *******************/
#define  DMA_CCR_EN                          ((uint32_t)0x00000001)        /*!< Channel enable */
#define  DMA_CCR_TCIE                        ((uint32_t)0x00000002)        /*!< Transfer complete interrupt enable */
#define  DMA_CCR_HTIE                        ((uint32_t)0x00000004)        /*!< Half Transfer interrupt enable */
#define  DMA_CCR_TEIE                        ((uint32_t)0x00000008)        /*!< Transfer error interrupt enable */
#define  DMA_CCR_DIR                         ((uint32_t)0x00000010)        /*!< Data transfer direction */
#define  DMA_CCR_CIRC                        ((uint32_t)0x00000020)        /*!< Circular mode */
#define  DMA_CCR_PINC                        ((uint32_t)0x00000040)        /*!< Peripheral increment mode */
#define  DMA_CCR_MINC                        ((uint32_t)0x00000080)        /*!< Memory increment mode */

#define  DMA_CCR_PSIZE                       ((uint32_t)0x00000300)        /*!< PSIZE[1:0] bits (Peripheral size) */
#define  DMA_CCR_PSIZE_0                     ((uint32_t)0x00000100)        /*!< Bit 0 */
#define  DMA_CCR_PSIZE_1                     ((uint32_t)0x00000200)        /*!< Bit 1 */

#define  DMA_CCR_MSIZE                       ((uint32_t)0x00000C00)        /*!< MSIZE[1:0] bits (Memory size) */
#define  DMA_CCR_MSIZE_0                     ((uint32_t)0x00000400)        /*!< Bit 0 */
#define  DMA_CCR_MSIZE_1                     ((uint32_t)0x00000800)        /*!< Bit 1 */

#define  DMA_CCR_PL                          ((uint32_t)0x00003000)        /*!< PL[1:0] bits(Channel Priority level) */
#define  DMA_CCR_PL_0                        ((uint32_t)0x00001000)        /*!< Bit 0 */
#define  DMA_CCR_PL_1                        ((uint32_t)0x00002000)        /*!< Bit 1 */

#define  DMA_CCR_MEM2MEM                     ((uint32_t)0x00004000)        /*!< Memory to memory mode */

/******************  Bit definition for DMA_CNDTR  register  ******************/
#define  DMA_CNDTR_NDT                       ((uint32_t)0x0000FFFF)            /*!< Number of data to Transfer */

/******************  Bit definition for DMA_CPAR  register  *******************/
#define  DMA_CPAR_PA                         ((uint32_t)0xFFFFFFFF)        /*!< Peripheral Address */

/******************  Bit definition for DMA_CMAR  register  *******************/
#define  DMA_CMAR_MA                         ((uint32_t)0xFFFFFFFF)        /*!< Memory Address */

/******************************************************************************/
/*                                                                            */
/*                        Analog to Digital Converter                         */
/*                                                                            */
/******************************************************************************/

/********************  Bit definition for ADC_SR register  ********************/
#define  ADC_SR_AWD                          ((uint32_t)0x00000001)        /*!< Analog watchdog flag */
#define  ADC_SR_EOC                          ((uint32_t)0x00000002)        /*!< End of conversion */
#define  ADC_SR_JEOC                         ((uint32_t)0x00000004)        /*!< Injected channel end of conversion */
#define  ADC_SR_JSTRT                        ((uint32_t)0x00000008)        /*!< Injected channel Start flag */
#define  ADC_SR_STRT                         ((uint32_t)0x00000010)        /*!< Regular channel Start flag */

/*******************  Bit definition for ADC_CR1 register  ********************/
#define  ADC_CR1_AWDCH                       ((uint32_t)0x0000001F)        /*!< AWDCH[4:0] bits (Analog watchdog channel select bits) */
#define  ADC_CR1_AWDCH_0                     ((uint32_t)0x00000001)        /*!< Bit 0 */
#define  ADC_CR1_AWDCH_1                     ((uint32_t)0x00000002)        /*!< Bit 1 */
#define  ADC_CR1_AWDCH_2                     ((uint32_t)0x00000004)        /*!< Bit 2 */
#define  ADC_CR1_AWDCH_3                     ((uint32_t)0x00000008)        /*!< Bit 3 */
#define  ADC_CR1_AWDCH_4                     ((uint32_t)0x00000010)        /*!< Bit 4 */

#define  ADC_CR1_EOCIE                       ((uint32_t)0x00000020)        /*!< Interrupt enable for EOC */
#define  ADC_CR1_AWDIE                       ((uint32_t)0x00000040)        /*!< Analog Watchdog interrupt enable */
#define  ADC_CR1_JEOCIE                      ((uint32_t)0x00000080)        /*!< Interrupt enable for injected channels */
#define  ADC_CR1_SCAN                        ((uint32_t)0x00000100)        /*!< Scan mode */
#define  ADC_CR1_AWDSGL                      ((uint32_t)0x00000200)        /*!< Enable the watchdog on a single channel in scan mode */
#define  ADC_CR1_JAUTO                       ((uint32_t)0x00000400)        /*!< Automatic injected group conversion */
#define  ADC_CR1_DISCEN                      ((uint32_t)0x00000800)        /*!< Discontinuous mode on regular channels */
#define  ADC_CR1_JDISCEN                     ((uint32_t)0x00001000)        /*!< Discontinuous mode on injected channels */

#define  ADC_CR1_DISCNUM                     ((uint32_t)0x0000E000)        /*!< DISCNUM[2:0] bits (Discontinuous mode channel count) */
#define  ADC_CR1_DISCNUM_0                   ((uint32_t)0x00002000)        /*!< Bit 0 */
#define  ADC_CR1_DISCNUM_1                   ((uint32_t)0x00004000)        /*!< Bit 1 */
#define  ADC_CR1_DISCNUM_2                   ((uint32_t)0x00008000)        /*!< Bit 2 */

#define  ADC_CR1_DUALMOD                     ((uint32_t)0x000F0000)        /*!< DUALMOD[3:0] bits (Dual mode selection) */
#define  ADC_CR1_DUALMOD_0                   ((uint32_t)0x00010000)        /*!< Bit 0 */
#define  ADC_CR1_DUALMOD_1                   ((uint32_t)0x00020000)        /*!< Bit 1 */
#define  ADC_CR1_DUALMOD_2                   ((uint32_t)0x00040000)        /*!< Bit 2 */
#define  ADC_CR1_DUALMOD_3                   ((uint32_t)0x00080000)        /*!< Bit 3 */

#define  ADC_CR1_JAWDEN                      ((uint32_t)0x00400000)        /*!< Analog watchdog enable on injected channels */
#define  ADC_CR1_AWDEN                       ((uint32_t)0x00800000)        /*!< Analog watchdog enable on regular channels */

  
/*******************  Bit definition for ADC_CR2 register  ********************/
#define  ADC_CR2_ADON                        ((uint32_t)0x00000001)        /*!< A/D Converter ON / OFF */
#define  ADC_CR2_CONT                        ((uint32_t)0x00000002)        /*!< Continuous Conversion */
#define  ADC_CR2_CAL                         ((uint32_t)0x00000004)        /*!< A/D Calibration */
#define  ADC_CR2_RSTCAL                      ((uint32_t)0x00000008)        /*!< Reset Calibration */
#define  ADC_CR2_DMA                         ((uint32_t)0x00000100)        /*!< Direct Memory access mode */
#define  ADC_CR2_ALIGN                       ((uint32_t)0x00000800)        /*!< Data Alignment */

#define  ADC_CR2_JEXTSEL                     ((uint32_t)0x00007000)        /*!< JEXTSEL[2:0] bits (External event select for injected group) */
#define  ADC_CR2_JEXTSEL_0                   ((uint32_t)0x00001000)        /*!< Bit 0 */
#define  ADC_CR2_JEXTSEL_1                   ((uint32_t)0x00002000)        /*!< Bit 1 */
#define  ADC_CR2_JEXTSEL_2                   ((uint32_t)0x00004000)        /*!< Bit 2 */

#define  ADC_CR2_JEXTTRIG                    ((uint32_t)0x00008000)        /*!< External Trigger Conversion mode for injected channels */

#define  ADC_CR2_EXTSEL                      ((uint32_t)0x000E0000)        /*!< EXTSEL[2:0] bits (External Event Select for regular group) */
#define  ADC_CR2_EXTSEL_0                    ((uint32_t)0x00020000)        /*!< Bit 0 */
#define  ADC_CR2_EXTSEL_1                    ((uint32_t)0x00040000)        /*!< Bit 1 */
#define  ADC_CR2_EXTSEL_2                    ((uint32_t)0x00080000)        /*!< Bit 2 */

#define  ADC_CR2_EXTTRIG                     ((uint32_t)0x00100000)        /*!< External Trigger Conversion mode for regular channels */
#define  ADC_CR2_JSWSTART                    ((uint32_t)0x00200000)        /*!< Start Conversion of injected channels */
#define  ADC_CR2_SWSTART                     ((uint32_t)0x00400000)        /*!< Start Conversion of regular channels */
#define  ADC_CR2_TSVREFE                     ((uint32_t)0x00800000)        /*!< Temperature Sensor and VREFINT Enable */

/******************  Bit definition for ADC_SMPR1 register  *******************/
#define  ADC_SMPR1_SMP10                     ((uint32_t)0x00000007)        /*!< SMP10[2:0] bits (Channel 10 Sample time selection) */
#define  ADC_SMPR1_SMP10_0                   ((uint32_t)0x00000001)        /*!< Bit 0 */
#define  ADC_SMPR1_SMP10_1                   ((uint32_t)0x00000002)        /*!< Bit 1 */
#define  ADC_SMPR1_SMP10_2                   ((uint32_t)0x00000004)        /*!< Bit 2 */

#define  ADC_SMPR1_SMP11                     ((uint32_t)0x00000038)        /*!< SMP11[2:0] bits (Channel 11 Sample time selection) */
#define  ADC_SMPR1_SMP11_0                   ((uint32_t)0x00000008)        /*!< Bit 0 */
#define  ADC_SMPR1_SMP11_1                   ((uint32_t)0x00000010)        /*!< Bit 1 */
#define  ADC_SMPR1_SMP11_2                   ((uint32_t)0x00000020)        /*!< Bit 2 */

#define  ADC_SMPR1_SMP12                     ((uint32_t)0x000001C0)        /*!< SMP12[2:0] bits (Channel 12 Sample time selection) */
#define  ADC_SMPR1_SMP12_0                   ((uint32_t)0x00000040)        /*!< Bit 0 */
#define  ADC_SMPR1_SMP12_1                   ((uint32_t)0x00000080)        /*!< Bit 1 */
#define  ADC_SMPR1_SMP12_2                   ((uint32_t)0x00000100)        /*!< Bit 2 */

#define  ADC_SMPR1_SMP13                     ((uint32_t)0x00000E00)        /*!< SMP13[2:0] bits (Channel 13 Sample time selection) */
#define  ADC_SMPR1_SMP13_0                   ((uint32_t)0x00000200)        /*!< Bit 0 */
#define  ADC_SMPR1_SMP13_1                   ((uint32_t)0x00000400)        /*!< Bit 1 */
#define  ADC_SMPR1_SMP13_2                   ((uint32_t)0x00000800)        /*!< Bit 2 */

#define  ADC_SMPR1_SMP14                     ((uint32_t)0x00007000)        /*!< SMP14[2:0] bits (Channel 14 Sample time selection) */
#define  ADC_SMPR1_SMP14_0                   ((uint32_t)0x00001000)        /*!< Bit 0 */
#define  ADC_SMPR1_SMP14_1                   ((uint32_t)0x00002000)        /*!< Bit 1 */
#define  ADC_SMPR1_SMP14_2                   ((uint32_t)0x00004000)        /*!< Bit 2 */

#define  ADC_SMPR1_SMP15                     ((uint32_t)0x00038000)        /*!< SMP15[2:0] bits (Channel 15 Sample time selection) */
#define  ADC_SMPR1_SMP15_0                   ((uint32_t)0x00008000)        /*!< Bit 0 */
#define  ADC_SMPR1_SMP15_1                   ((uint32_t)0x00010000)        /*!< Bit 1 */
#define  ADC_SMPR1_SMP15_2                   ((uint32_t)0x00020000)        /*!< Bit 2 */

#define  ADC_SMPR1_SMP16                     ((uint32_t)0x001C0000)        /*!< SMP16[2:0] bits (Channel 16 Sample time selection) */
#define  ADC_SMPR1_SMP16_0                   ((uint32_t)0x00040000)        /*!< Bit 0 */
#define  ADC_SMPR1_SMP16_1                   ((uint32_t)0x00080000)        /*!< Bit 1 */
#define  ADC_SMPR1_SMP16_2                   ((uint32_t)0x00100000)        /*!< Bit 2 */

#define  ADC_SMPR1_SMP17                     ((uint32_t)0x00E00000)        /*!< SMP17[2:0] bits (Channel 17 Sample time selection) */
#define  ADC_SMPR1_SMP17_0                   ((uint32_t)0x00200000)        /*!< Bit 0 */
#define  ADC_SMPR1_SMP17_1                   ((uint32_t)0x00400000)        /*!< Bit 1 */
#define  ADC_SMPR1_SMP17_2                   ((uint32_t)0x00800000)        /*!< Bit 2 */

/******************  Bit definition for ADC_SMPR2 register  *******************/
#define  ADC_SMPR2_SMP0                      ((uint32_t)0x00000007)        /*!< SMP0[2:0] bits (Channel 0 Sample time selection) */
#define  ADC_SMPR2_SMP0_0                    ((uint32_t)0x00000001)        /*!< Bit 0 */
#define  ADC_SMPR2_SMP0_1                    ((uint32_t)0x00000002)        /*!< Bit 1 */
#define  ADC_SMPR2_SMP0_2                    ((uint32_t)0x00000004)        /*!< Bit 2 */

#define  ADC_SMPR2_SMP1                      ((uint32_t)0x00000038)        /*!< SMP1[2:0] bits (Channel 1 Sample time selection) */
#define  ADC_SMPR2_SMP1_0                    ((uint32_t)0x00000008)        /*!< Bit 0 */
#define  ADC_SMPR2_SMP1_1                    ((uint32_t)0x00000010)        /*!< Bit 1 */
#define  ADC_SMPR2_SMP1_2                    ((uint32_t)0x00000020)        /*!< Bit 2 */

#define  ADC_SMPR2_SMP2                      ((uint32_t)0x000001C0)        /*!< SMP2[2:0] bits (Channel 2 Sample time selection) */
#define  ADC_SMPR2_SMP2_0                    ((uint32_t)0x00000040)        /*!< Bit 0 */
#define  ADC_SMPR2_SMP2_1                    ((uint32_t)0x00000080)        /*!< Bit 1 */
#define  ADC_SMPR2_SMP2_2                    ((uint32_t)0x00000100)        /*!< Bit 2 */

#define  ADC_SMPR2_SMP3                      ((uint32_t)0x00000E00)        /*!< SMP3[2:0] bits (Channel 3 Sample time selection) */
#define  ADC_SMPR2_SMP3_0                    ((uint32_t)0x00000200)        /*!< Bit 0 */
#define  ADC_SMPR2_SMP3_1                    ((uint32_t)0x00000400)        /*!< Bit 1 */
#define  ADC_SMPR2_SMP3_2                    ((uint32_t)0x00000800)        /*!< Bit 2 */

#define  ADC_SMPR2_SMP4                      ((uint32_t)0x00007000)        /*!< SMP4[2:0] bits (Channel 4 Sample time selection) */
#define  ADC_SMPR2_SMP4_0                    ((uint32_t)0x00001000)        /*!< Bit 0 */
#define  ADC_SMPR2_SMP4_1                    ((uint32_t)0x00002000)        /*!< Bit 1 */
#define  ADC_SMPR2_SMP4_2                    ((uint32_t)0x00004000)        /*!< Bit 2 */

#define  ADC_SMPR2_SMP5                      ((uint32_t)0x00038000)        /*!< SMP5[2:0] bits (Channel 5 Sample time selection) */
#define  ADC_SMPR2_SMP5_0                    ((uint32_t)0x00008000)        /*!< Bit 0 */
#define  ADC_SMPR2_SMP5_1                    ((uint32_t)0x00010000)        /*!< Bit 1 */
#define  ADC_SMPR2_SMP5_2                    ((uint32_t)0x00020000)        /*!< Bit 2 */

#define  ADC_SMPR2_SMP6                      ((uint32_t)0x001C0000)        /*!< SMP6[2:0] bits (Channel 6 Sample time selection) */
#define  ADC_SMPR2_SMP6_0                    ((uint32_t)0x00040000)        /*!< Bit 0 */
#define  ADC_SMPR2_SMP6_1                    ((uint32_t)0x00080000)        /*!< Bit 1 */
#define  ADC_SMPR2_SMP6_2                    ((uint32_t)0x00100000)        /*!< Bit 2 */

#define  ADC_SMPR2_SMP7                      ((uint32_t)0x00E00000)        /*!< SMP7[2:0] bits (Channel 7 Sample time selection) */
#define  ADC_SMPR2_SMP7_0                    ((uint32_t)0x00200000)        /*!< Bit 0 */
#define  ADC_SMPR2_SMP7_1                    ((uint32_t)0x00400000)        /*!< Bit 1 */
#define  ADC_SMPR2_SMP7_2                    ((uint32_t)0x00800000)        /*!< Bit 2 */

#define  ADC_SMPR2_SMP8                      ((uint32_t)0x07000000)        /*!< SMP8[2:0] bits (Channel 8 Sample time selection) */
#define  ADC_SMPR2_SMP8_0                    ((uint32_t)0x01000000)        /*!< Bit 0 */
#define  ADC_SMPR2_SMP8_1                    ((uint32_t)0x02000000)        /*!< Bit 1 */
#define  ADC_SMPR2_SMP8_2                    ((uint32_t)0x04000000)        /*!< Bit 2 */

#define  ADC_SMPR2_SMP9                      ((uint32_t)0x38000000)        /*!< SMP9[2:0] bits (Channel 9 Sample time selection) */
#define  ADC_SMPR2_SMP9_0                    ((uint32_t)0x08000000)        /*!< Bit 0 */
#define  ADC_SMPR2_SMP9_1                    ((uint32_t)0x10000000)        /*!< Bit 1 */
#define  ADC_SMPR2_SMP9_2                    ((uint32_t)0x20000000)        /*!< Bit 2 */

/******************  Bit definition for ADC_JOFR1 register  *******************/
#define  ADC_JOFR1_JOFFSET1                  ((uint32_t)0x00000FFF)        /*!< Data offset for injected channel 1 */

/******************  Bit definition for ADC_JOFR2 register  *******************/
#define  ADC_JOFR2_JOFFSET2                  ((uint32_t)0x00000FFF)        /*!< Data offset for injected channel 2 */

/******************  Bit definition for ADC_JOFR3 register  *******************/
#define  ADC_JOFR3_JOFFSET3                  ((uint32_t)0x00000FFF)        /*!< Data offset for injected channel 3 */

/******************  Bit definition for ADC_JOFR4 register  *******************/
#define  ADC_JOFR4_JOFFSET4                  ((uint32_t)0x00000FFF)        /*!< Data offset for injected channel 4 */

/*******************  Bit definition for ADC_HTR register  ********************/
#define  ADC_HTR_HT                          ((uint32_t)0x00000FFF)        /*!< Analog watchdog high threshold */

/*******************  Bit definition for ADC_LTR register  ********************/
#define  ADC_LTR_LT                          ((uint32_t)0x00000FFF)        /*!< Analog watchdog low threshold */

/*******************  Bit definition for ADC_SQR1 register  *******************/
#define  ADC_SQR1_SQ13                       ((uint32_t)0x0000001F)        /*!< SQ13[4:0] bits (13th conversion in regular sequence) */
#define  ADC_SQR1_SQ13_0                     ((uint32_t)0x00000001)        /*!< Bit 0 */
#define  ADC_SQR1_SQ13_1                     ((uint32_t)0x00000002)        /*!< Bit 1 */
#define  ADC_SQR1_SQ13_2                     ((uint32_t)0x00000004)        /*!< Bit 2 */
#define  ADC_SQR1_SQ13_3                     ((uint32_t)0x00000008)        /*!< Bit 3 */
#define  ADC_SQR1_SQ13_4                     ((uint32_t)0x00000010)        /*!< Bit 4 */

#define  ADC_SQR1_SQ14                       ((uint32_t)0x000003E0)        /*!< SQ14[4:0] bits (14th conversion in regular sequence) */
#define  ADC_SQR1_SQ14_0                     ((uint32_t)0x00000020)        /*!< Bit 0 */
#define  ADC_SQR1_SQ14_1                     ((uint32_t)0x00000040)        /*!< Bit 1 */
#define  ADC_SQR1_SQ14_2                     ((uint32_t)0x00000080)        /*!< Bit 2 */
#define  ADC_SQR1_SQ14_3                     ((uint32_t)0x00000100)        /*!< Bit 3 */
#define  ADC_SQR1_SQ14_4                     ((uint32_t)0x00000200)        /*!< Bit 4 */

#define  ADC_SQR1_SQ15                       ((uint32_t)0x00007C00)        /*!< SQ15[4:0] bits (15th conversion in regular sequence) */
#define  ADC_SQR1_SQ15_0                     ((uint32_t)0x00000400)        /*!< Bit 0 */
#define  ADC_SQR1_SQ15_1                     ((uint32_t)0x00000800)        /*!< Bit 1 */
#define  ADC_SQR1_SQ15_2                     ((uint32_t)0x00001000)        /*!< Bit 2 */
#define  ADC_SQR1_SQ15_3                     ((uint32_t)0x00002000)        /*!< Bit 3 */
#define  ADC_SQR1_SQ15_4                     ((uint32_t)0x00004000)        /*!< Bit 4 */

#define  ADC_SQR1_SQ16                       ((uint32_t)0x000F8000)        /*!< SQ16[4:0] bits (16th conversion in regular sequence) */
#define  ADC_SQR1_SQ16_0                     ((uint32_t)0x00008000)        /*!< Bit 0 */
#define  ADC_SQR1_SQ16_1                     ((uint32_t)0x00010000)        /*!< Bit 1 */
#define  ADC_SQR1_SQ16_2                     ((uint32_t)0x00020000)        /*!< Bit 2 */
#define  ADC_SQR1_SQ16_3                     ((uint32_t)0x00040000)        /*!< Bit 3 */
#define  ADC_SQR1_SQ16_4                     ((uint32_t)0x00080000)        /*!< Bit 4 */

#define  ADC_SQR1_L                          ((uint32_t)0x00F00000)        /*!< L[3:0] bits (Regular channel sequence length) */
#define  ADC_SQR1_L_0                        ((uint32_t)0x00100000)        /*!< Bit 0 */
#define  ADC_SQR1_L_1                        ((uint32_t)0x00200000)        /*!< Bit 1 */
#define  ADC_SQR1_L_2                        ((uint32_t)0x00400000)        /*!< Bit 2 */
#define  ADC_SQR1_L_3                        ((uint32_t)0x00800000)        /*!< Bit 3 */

/*******************  Bit definition for ADC_SQR2 register  *******************/
#define  ADC_SQR2_SQ7                        ((uint32_t)0x0000001F)        /*!< SQ7[4:0] bits (7th conversion in regular sequence) */
#define  ADC_SQR2_SQ7_0                      ((uint32_t)0x00000001)        /*!< Bit 0 */
#define  ADC_SQR2_SQ7_1                      ((uint32_t)0x00000002)        /*!< Bit 1 */
#define  ADC_SQR2_SQ7_2                      ((uint32_t)0x00000004)        /*!< Bit 2 */
#define  ADC_SQR2_SQ7_3                      ((uint32_t)0x00000008)        /*!< Bit 3 */
#define  ADC_SQR2_SQ7_4                      ((uint32_t)0x00000010)        /*!< Bit 4 */

#define  ADC_SQR2_SQ8                        ((uint32_t)0x000003E0)        /*!< SQ8[4:0] bits (8th conversion in regular sequence) */
#define  ADC_SQR2_SQ8_0                      ((uint32_t)0x00000020)        /*!< Bit 0 */
#define  ADC_SQR2_SQ8_1                      ((uint32_t)0x00000040)        /*!< Bit 1 */
#define  ADC_SQR2_SQ8_2                      ((uint32_t)0x00000080)        /*!< Bit 2 */
#define  ADC_SQR2_SQ8_3                      ((uint32_t)0x00000100)        /*!< Bit 3 */
#define  ADC_SQR2_SQ8_4                      ((uint32_t)0x00000200)        /*!< Bit 4 */

#define  ADC_SQR2_SQ9                        ((uint32_t)0x00007C00)        /*!< SQ9[4:0] bits (9th conversion in regular sequence) */
#define  ADC_SQR2_SQ9_0                      ((uint32_t)0x00000400)        /*!< Bit 0 */
#define  ADC_SQR2_SQ9_1                      ((uint32_t)0x00000800)        /*!< Bit 1 */
#define  ADC_SQR2_SQ9_2                      ((uint32_t)0x00001000)        /*!< Bit 2 */
#define  ADC_SQR2_SQ9_3                      ((uint32_t)0x00002000)        /*!< Bit 3 */
#define  ADC_SQR2_SQ9_4                      ((uint32_t)0x00004000)        /*!< Bit 4 */

#define  ADC_SQR2_SQ10                       ((uint32_t)0x000F8000)        /*!< SQ10[4:0] bits (10th conversion in regular sequence) */
#define  ADC_SQR2_SQ10_0                     ((uint32_t)0x00008000)        /*!< Bit 0 */
#define  ADC_SQR2_SQ10_1                     ((uint32_t)0x00010000)        /*!< Bit 1 */
#define  ADC_SQR2_SQ10_2                     ((uint32_t)0x00020000)        /*!< Bit 2 */
#define  ADC_SQR2_SQ10_3                     ((uint32_t)0x00040000)        /*!< Bit 3 */
#define  ADC_SQR2_SQ10_4                     ((uint32_t)0x00080000)        /*!< Bit 4 */

#define  ADC_SQR2_SQ11                       ((uint32_t)0x01F00000)        /*!< SQ11[4:0] bits (11th conversion in regular sequence) */
#define  ADC_SQR2_SQ11_0                     ((uint32_t)0x00100000)        /*!< Bit 0 */
#define  ADC_SQR2_SQ11_1                     ((uint32_t)0x00200000)        /*!< Bit 1 */
#define  ADC_SQR2_SQ11_2                     ((uint32_t)0x00400000)        /*!< Bit 2 */
#define  ADC_SQR2_SQ11_3                     ((uint32_t)0x00800000)        /*!< Bit 3 */
#define  ADC_SQR2_SQ11_4                     ((uint32_t)0x01000000)        /*!< Bit 4 */

#define  ADC_SQR2_SQ12                       ((uint32_t)0x3E000000)        /*!< SQ12[4:0] bits (12th conversion in regular sequence) */
#define  ADC_SQR2_SQ12_0                     ((uint32_t)0x02000000)        /*!< Bit 0 */
#define  ADC_SQR2_SQ12_1                     ((uint32_t)0x04000000)        /*!< Bit 1 */
#define  ADC_SQR2_SQ12_2                     ((uint32_t)0x08000000)        /*!< Bit 2 */
#define  ADC_SQR2_SQ12_3                     ((uint32_t)0x10000000)        /*!< Bit 3 */
#define  ADC_SQR2_SQ12_4                     ((uint32_t)0x20000000)        /*!< Bit 4 */

/*******************  Bit definition for ADC_SQR3 register  *******************/
#define  ADC_SQR3_SQ1                        ((uint32_t)0x0000001F)        /*!< SQ1[4:0] bits (1st conversion in regular sequence) */
#define  ADC_SQR3_SQ1_0                      ((uint32_t)0x00000001)        /*!< Bit 0 */
#define  ADC_SQR3_SQ1_1                      ((uint32_t)0x00000002)        /*!< Bit 1 */
#define  ADC_SQR3_SQ1_2                      ((uint32_t)0x00000004)        /*!< Bit 2 */
#define  ADC_SQR3_SQ1_3                      ((uint32_t)0x00000008)        /*!< Bit 3 */
#define  ADC_SQR3_SQ1_4                      ((uint32_t)0x00000010)        /*!< Bit 4 */

#define  ADC_SQR3_SQ2                        ((uint32_t)0x000003E0)        /*!< SQ2[4:0] bits (2nd conversion in regular sequence) */
#define  ADC_SQR3_SQ2_0                      ((uint32_t)0x00000020)        /*!< Bit 0 */
#define  ADC_SQR3_SQ2_1                      ((uint32_t)0x00000040)        /*!< Bit 1 */
#define  ADC_SQR3_SQ2_2                      ((uint32_t)0x00000080)        /*!< Bit 2 */
#define  ADC_SQR3_SQ2_3                      ((uint32_t)0x00000100)        /*!< Bit 3 */
#define  ADC_SQR3_SQ2_4                      ((uint32_t)0x00000200)        /*!< Bit 4 */

#define  ADC_SQR3_SQ3                        ((uint32_t)0x00007C00)        /*!< SQ3[4:0] bits (3rd conversion in regular sequence) */
#define  ADC_SQR3_SQ3_0                      ((uint32_t)0x00000400)        /*!< Bit 0 */
#define  ADC_SQR3_SQ3_1                      ((uint32_t)0x00000800)        /*!< Bit 1 */
#define  ADC_SQR3_SQ3_2                      ((uint32_t)0x00001000)        /*!< Bit 2 */
#define  ADC_SQR3_SQ3_3                      ((uint32_t)0x00002000)        /*!< Bit 3 */
#define  ADC_SQR3_SQ3_4                      ((uint32_t)0x00004000)        /*!< Bit 4 */

#define  ADC_SQR3_SQ4                        ((uint32_t)0x000F8000)        /*!< SQ4[4:0] bits (4th conversion in regular sequence) */
#define  ADC_SQR3_SQ4_0                      ((uint32_t)0x00008000)        /*!< Bit 0 */
#define  ADC_SQR3_SQ4_1                      ((uint32_t)0x00010000)        /*!< Bit 1 */
#define  ADC_SQR3_SQ4_2                      ((uint32_t)0x00020000)        /*!< Bit 2 */
#define  ADC_SQR3_SQ4_3                      ((uint32_t)0x00040000)        /*!< Bit 3 */
#define  ADC_SQR3_SQ4_4                      ((uint32_t)0x00080000)        /*!< Bit 4 */

#define  ADC_SQR3_SQ5                        ((uint32_t)0x01F00000)        /*!< SQ5[4:0] bits (5th conversion in regular sequence) */
#define  ADC_SQR3_SQ5_0                      ((uint32_t)0x00100000)        /*!< Bit 0 */
#define  ADC_SQR3_SQ5_1                      ((uint32_t)0x00200000)        /*!< Bit 1 */
#define  ADC_SQR3_SQ5_2                      ((uint32_t)0x00400000)        /*!< Bit 2 */
#define  ADC_SQR3_SQ5_3                      ((uint32_t)0x00800000)        /*!< Bit 3 */
#define  ADC_SQR3_SQ5_4                      ((uint32_t)0x01000000)        /*!< Bit 4 */

#define  ADC_SQR3_SQ6                        ((uint32_t)0x3E000000)        /*!< SQ6[4:0] bits (6th conversion in regular sequence) */
#define  ADC_SQR3_SQ6_0                      ((uint32_t)0x02000000)        /*!< Bit 0 */
#define  ADC_SQR3_SQ6_1                      ((uint32_t)0x04000000)        /*!< Bit 1 */
#define  ADC_SQR3_SQ6_2                      ((uint32_t)0x08000000)        /*!< Bit 2 */
#define  ADC_SQR3_SQ6_3                      ((uint32_t)0x10000000)        /*!< Bit 3 */
#define  ADC_SQR3_SQ6_4                      ((uint32_t)0x20000000)        /*!< Bit 4 */

/*******************  Bit definition for ADC_JSQR register  *******************/
#define  ADC_JSQR_JSQ1                       ((uint32_t)0x0000001F)        /*!< JSQ1[4:0] bits (1st conversion in injected sequence) */  
#define  ADC_JSQR_JSQ1_0                     ((uint32_t)0x00000001)        /*!< Bit 0 */
#define  ADC_JSQR_JSQ1_1                     ((uint32_t)0x00000002)        /*!< Bit 1 */
#define  ADC_JSQR_JSQ1_2                     ((uint32_t)0x00000004)        /*!< Bit 2 */
#define  ADC_JSQR_JSQ1_3                     ((uint32_t)0x00000008)        /*!< Bit 3 */
#define  ADC_JSQR_JSQ1_4                     ((uint32_t)0x00000010)        /*!< Bit 4 */

#define  ADC_JSQR_JSQ2                       ((uint32_t)0x000003E0)        /*!< JSQ2[4:0] bits (2nd conversion in injected sequence) */
#define  ADC_JSQR_JSQ2_0                     ((uint32_t)0x00000020)        /*!< Bit 0 */
#define  ADC_JSQR_JSQ2_1                     ((uint32_t)0x00000040)        /*!< Bit 1 */
#define  ADC_JSQR_JSQ2_2                     ((uint32_t)0x00000080)        /*!< Bit 2 */
#define  ADC_JSQR_JSQ2_3                     ((uint32_t)0x00000100)        /*!< Bit 3 */
#define  ADC_JSQR_JSQ2_4                     ((uint32_t)0x00000200)        /*!< Bit 4 */

#define  ADC_JSQR_JSQ3                       ((uint32_t)0x00007C00)        /*!< JSQ3[4:0] bits (3rd conversion in injected sequence) */
#define  ADC_JSQR_JSQ3_0                     ((uint32_t)0x00000400)        /*!< Bit 0 */
#define  ADC_JSQR_JSQ3_1                     ((uint32_t)0x00000800)        /*!< Bit 1 */
#define  ADC_JSQR_JSQ3_2                     ((uint32_t)0x00001000)        /*!< Bit 2 */
#define  ADC_JSQR_JSQ3_3                     ((uint32_t)0x00002000)        /*!< Bit 3 */
#define  ADC_JSQR_JSQ3_4                     ((uint32_t)0x00004000)        /*!< Bit 4 */

#define  ADC_JSQR_JSQ4                       ((uint32_t)0x000F8000)        /*!< JSQ4[4:0] bits (4th conversion in injected sequence) */
#define  ADC_JSQR_JSQ4_0                     ((uint32_t)0x00008000)        /*!< Bit 0 */
#define  ADC_JSQR_JSQ4_1                     ((uint32_t)0x00010000)        /*!< Bit 1 */
#define  ADC_JSQR_JSQ4_2                     ((uint32_t)0x00020000)        /*!< Bit 2 */
#define  ADC_JSQR_JSQ4_3                     ((uint32_t)0x00040000)        /*!< Bit 3 */
#define  ADC_JSQR_JSQ4_4                     ((uint32_t)0x00080000)        /*!< Bit 4 */

#define  ADC_JSQR_JL                         ((uint32_t)0x00300000)        /*!< JL[1:0] bits (Injected Sequence length) */
#define  ADC_JSQR_JL_0                       ((uint32_t)0x00100000)        /*!< Bit 0 */
#define  ADC_JSQR_JL_1                       ((uint32_t)0x00200000)        /*!< Bit 1 */

/*******************  Bit definition for ADC_JDR1 register  *******************/
#define  ADC_JDR1_JDATA                      ((uint32_t)0x0000FFFF)        /*!< Injected data */

/*******************  Bit definition for ADC_JDR2 register  *******************/
#define  ADC_JDR2_JDATA                      ((uint32_t)0x0000FFFF)        /*!< Injected data */

/*******************  Bit definition for ADC_JDR3 register  *******************/
#define  ADC_JDR3_JDATA                      ((uint32_t)0x0000FFFF)        /*!< Injected data */

/*******************  Bit definition for ADC_JDR4 register  *******************/
#define  ADC_JDR4_JDATA                      ((uint32_t)0x0000FFFF)        /*!< Injected data */

/********************  Bit definition for ADC_DR register  ********************/
#define  ADC_DR_DATA                         ((uint32_t)0x0000FFFF)        /*!< Regular data */
#define  ADC_DR_ADC2DATA                     ((uint32_t)0xFFFF0000)        /*!< ADC2 data */
/******************************************************************************/
/*                                                                            */
/*                      Digital to Analog Converter                           */
/*                                                                            */
/******************************************************************************/

/********************  Bit definition for DAC_CR register  ********************/
#define  DAC_CR_EN1                          ((uint32_t)0x00000001)        /*!< DAC channel1 enable */
#define  DAC_CR_BOFF1                        ((uint32_t)0x00000002)        /*!< DAC channel1 output buffer disable */
#define  DAC_CR_TEN1                         ((uint32_t)0x00000004)        /*!< DAC channel1 Trigger enable */

#define  DAC_CR_TSEL1                        ((uint32_t)0x00000038)        /*!< TSEL1[2:0] (DAC channel1 Trigger selection) */
#define  DAC_CR_TSEL1_0                      ((uint32_t)0x00000008)        /*!< Bit 0 */
#define  DAC_CR_TSEL1_1                      ((uint32_t)0x00000010)        /*!< Bit 1 */
#define  DAC_CR_TSEL1_2                      ((uint32_t)0x00000020)        /*!< Bit 2 */

#define  DAC_CR_WAVE1                        ((uint32_t)0x000000C0)        /*!< WAVE1[1:0] (DAC channel1 noise/triangle wave generation enable) */
#define  DAC_CR_WAVE1_0                      ((uint32_t)0x00000040)        /*!< Bit 0 */
#define  DAC_CR_WAVE1_1                      ((uint32_t)0x00000080)        /*!< Bit 1 */

#define  DAC_CR_MAMP1                        ((uint32_t)0x00000F00)        /*!< MAMP1[3:0] (DAC channel1 Mask/Amplitude selector) */
#define  DAC_CR_MAMP1_0                      ((uint32_t)0x00000100)        /*!< Bit 0 */
#define  DAC_CR_MAMP1_1                      ((uint32_t)0x00000200)        /*!< Bit 1 */
#define  DAC_CR_MAMP1_2                      ((uint32_t)0x00000400)        /*!< Bit 2 */
#define  DAC_CR_MAMP1_3                      ((uint32_t)0x00000800)        /*!< Bit 3 */

#define  DAC_CR_DMAEN1                       ((uint32_t)0x00001000)        /*!< DAC channel1 DMA enable */
#define  DAC_CR_EN2                          ((uint32_t)0x00010000)        /*!< DAC channel2 enable */
#define  DAC_CR_BOFF2                        ((uint32_t)0x00020000)        /*!< DAC channel2 output buffer disable */
#define  DAC_CR_TEN2                         ((uint32_t)0x00040000)        /*!< DAC channel2 Trigger enable */

#define  DAC_CR_TSEL2                        ((uint32_t)0x00380000)        /*!< TSEL2[2:0] (DAC channel2 Trigger selection) */
#define  DAC_CR_TSEL2_0                      ((uint32_t)0x00080000)        /*!< Bit 0 */
#define  DAC_CR_TSEL2_1                      ((uint32_t)0x00100000)        /*!< Bit 1 */
#define  DAC_CR_TSEL2_2                      ((uint32_t)0x00200000)        /*!< Bit 2 */

#define  DAC_CR_WAVE2                        ((uint32_t)0x00C00000)        /*!< WAVE2[1:0] (DAC channel2 noise/triangle wave generation enable) */
#define  DAC_CR_WAVE2_0                      ((uint32_t)0x00400000)        /*!< Bit 0 */
#define  DAC_CR_WAVE2_1                      ((uint32_t)0x00800000)        /*!< Bit 1 */

#define  DAC_CR_MAMP2                        ((uint32_t)0x0F000000)        /*!< MAMP2[3:0] (DAC channel2 Mask/Amplitude selector) */
#define  DAC_CR_MAMP2_0                      ((uint32_t)0x01000000)        /*!< Bit 0 */
#define  DAC_CR_MAMP2_1                      ((uint32_t)0x02000000)        /*!< Bit 1 */
#define  DAC_CR_MAMP2_2                      ((uint32_t)0x04000000)        /*!< Bit 2 */
#define  DAC_CR_MAMP2_3                      ((uint32_t)0x08000000)        /*!< Bit 3 */

#define  DAC_CR_DMAEN2                       ((uint32_t)0x10000000)        /*!< DAC channel2 DMA enabled */


/*****************  Bit definition for DAC_SWTRIGR register  ******************/
#define  DAC_SWTRIGR_SWTRIG1                 ((uint32_t)0x00000001)        /*!< DAC channel1 software trigger */
#define  DAC_SWTRIGR_SWTRIG2                 ((uint32_t)0x00000002)        /*!< DAC channel2 software trigger */

/*****************  Bit definition for DAC_DHR12R1 register  ******************/
#define  DAC_DHR12R1_DACC1DHR                ((uint32_t)0x00000FFF)        /*!< DAC channel1 12-bit Right aligned data */

/*****************  Bit definition for DAC_DHR12L1 register  ******************/
#define  DAC_DHR12L1_DACC1DHR                ((uint32_t)0x0000FFF0)        /*!< DAC channel1 12-bit Left aligned data */

/******************  Bit definition for DAC_DHR8R1 register  ******************/
#define  DAC_DHR8R1_DACC1DHR                 ((uint32_t)0x000000FF)        /*!< DAC channel1 8-bit Right aligned data */

/*****************  Bit definition for DAC_DHR12R2 register  ******************/
#define  DAC_DHR12R2_DACC2DHR                ((uint32_t)0x00000FFF)        /*!< DAC channel2 12-bit Right aligned data */

/*****************  Bit definition for DAC_DHR12L2 register  ******************/
#define  DAC_DHR12L2_DACC2DHR                ((uint32_t)0x0000FFF0)        /*!< DAC channel2 12-bit Left aligned data */

/******************  Bit definition for DAC_DHR8R2 register  ******************/
#define  DAC_DHR8R2_DACC2DHR                 ((uint32_t)0x000000FF)        /*!< DAC channel2 8-bit Right aligned data */

/*****************  Bit definition for DAC_DHR12RD register  ******************/
#define  DAC_DHR12RD_DACC1DHR                ((uint32_t)0x00000FFF)        /*!< DAC channel1 12-bit Right aligned data */
#define  DAC_DHR12RD_DACC2DHR                ((uint32_t)0x0FFF0000)        /*!< DAC channel2 12-bit Right aligned data */

/*****************  Bit definition for DAC_DHR12LD register  ******************/
#define  DAC_DHR12LD_DACC1DHR                ((uint32_t)0x0000FFF0)        /*!< DAC channel1 12-bit Left aligned data */
#define  DAC_DHR12LD_DACC2DHR                ((uint32_t)0xFFF00000)        /*!< DAC channel2 12-bit Left aligned data */

/******************  Bit definition for DAC_DHR8RD register  ******************/
#define  DAC_DHR8RD_DACC1DHR                 ((uint32_t)0x000000FF)        /*!< DAC channel1 8-bit Right aligned data */
#define  DAC_DHR8RD_DACC2DHR                 ((uint32_t)0x0000FF00)        /*!< DAC channel2 8-bit Right aligned data */

/*******************  Bit definition for DAC_DOR1 register  *******************/
#define  DAC_DOR1_DACC1DOR                   ((uint32_t)0x00000FFF)        /*!< DAC channel1 data output */

/*******************  Bit definition for DAC_DOR2 register  *******************/
#define  DAC_DOR2_DACC2DOR                   ((uint32_t)0x00000FFF)        /*!< DAC channel2 data output */



/*****************************************************************************/
/*                                                                           */
/*                               Timers (TIM)                                */
/*                                                                           */
/*****************************************************************************/
/*******************  Bit definition for TIM_CR1 register  *******************/
#define  TIM_CR1_CEN                         ((uint32_t)0x00000001)            /*!<Counter enable */
#define  TIM_CR1_UDIS                        ((uint32_t)0x00000002)            /*!<Update disable */
#define  TIM_CR1_URS                         ((uint32_t)0x00000004)            /*!<Update request source */
#define  TIM_CR1_OPM                         ((uint32_t)0x00000008)            /*!<One pulse mode */
#define  TIM_CR1_DIR                         ((uint32_t)0x00000010)            /*!<Direction */

#define  TIM_CR1_CMS                         ((uint32_t)0x00000060)            /*!<CMS[1:0] bits (Center-aligned mode selection) */
#define  TIM_CR1_CMS_0                       ((uint32_t)0x00000020)            /*!<Bit 0 */
#define  TIM_CR1_CMS_1                       ((uint32_t)0x00000040)            /*!<Bit 1 */

#define  TIM_CR1_ARPE                        ((uint32_t)0x00000080)            /*!<Auto-reload preload enable */

#define  TIM_CR1_CKD                         ((uint32_t)0x00000300)            /*!<CKD[1:0] bits (clock division) */
#define  TIM_CR1_CKD_0                       ((uint32_t)0x00000100)            /*!<Bit 0 */
#define  TIM_CR1_CKD_1                       ((uint32_t)0x00000200)            /*!<Bit 1 */

/*******************  Bit definition for TIM_CR2 register  *******************/
#define  TIM_CR2_CCPC                        ((uint32_t)0x00000001)            /*!<Capture/Compare Preloaded Control */
#define  TIM_CR2_CCUS                        ((uint32_t)0x00000004)            /*!<Capture/Compare Control Update Selection */
#define  TIM_CR2_CCDS                        ((uint32_t)0x00000008)            /*!<Capture/Compare DMA Selection */

#define  TIM_CR2_MMS                         ((uint32_t)0x00000070)            /*!<MMS[2:0] bits (Master Mode Selection) */
#define  TIM_CR2_MMS_0                       ((uint32_t)0x00000010)            /*!<Bit 0 */
#define  TIM_CR2_MMS_1                       ((uint32_t)0x00000020)            /*!<Bit 1 */
#define  TIM_CR2_MMS_2                       ((uint32_t)0x00000040)            /*!<Bit 2 */

#define  TIM_CR2_TI1S                        ((uint32_t)0x00000080)            /*!<TI1 Selection */
#define  TIM_CR2_OIS1                        ((uint32_t)0x00000100)            /*!<Output Idle state 1 (OC1 output) */
#define  TIM_CR2_OIS1N                       ((uint32_t)0x00000200)            /*!<Output Idle state 1 (OC1N output) */
#define  TIM_CR2_OIS2                        ((uint32_t)0x00000400)            /*!<Output Idle state 2 (OC2 output) */
#define  TIM_CR2_OIS2N                       ((uint32_t)0x00000800)            /*!<Output Idle state 2 (OC2N output) */
#define  TIM_CR2_OIS3                        ((uint32_t)0x00001000)            /*!<Output Idle state 3 (OC3 output) */
#define  TIM_CR2_OIS3N                       ((uint32_t)0x00002000)            /*!<Output Idle state 3 (OC3N output) */
#define  TIM_CR2_OIS4                        ((uint32_t)0x00004000)            /*!<Output Idle state 4 (OC4 output) */

/*******************  Bit definition for TIM_SMCR register  ******************/
#define  TIM_SMCR_SMS                        ((uint32_t)0x00000007)            /*!<SMS[2:0] bits (Slave mode selection) */
#define  TIM_SMCR_SMS_0                      ((uint32_t)0x00000001)            /*!<Bit 0 */
#define  TIM_SMCR_SMS_1                      ((uint32_t)0x00000002)            /*!<Bit 1 */
#define  TIM_SMCR_SMS_2                      ((uint32_t)0x00000004)            /*!<Bit 2 */

#define  TIM_SMCR_OCCS                       ((uint32_t)0x00000008)            /*!< OCREF clear selection */

#define  TIM_SMCR_TS                         ((uint32_t)0x00000070)            /*!<TS[2:0] bits (Trigger selection) */
#define  TIM_SMCR_TS_0                       ((uint32_t)0x00000010)            /*!<Bit 0 */
#define  TIM_SMCR_TS_1                       ((uint32_t)0x00000020)            /*!<Bit 1 */
#define  TIM_SMCR_TS_2                       ((uint32_t)0x00000040)            /*!<Bit 2 */

#define  TIM_SMCR_MSM                        ((uint32_t)0x00000080)            /*!<Master/slave mode */

#define  TIM_SMCR_ETF                        ((uint32_t)0x00000F00)            /*!<ETF[3:0] bits (External trigger filter) */
#define  TIM_SMCR_ETF_0                      ((uint32_t)0x00000100)            /*!<Bit 0 */
#define  TIM_SMCR_ETF_1                      ((uint32_t)0x00000200)            /*!<Bit 1 */
#define  TIM_SMCR_ETF_2                      ((uint32_t)0x00000400)            /*!<Bit 2 */
#define  TIM_SMCR_ETF_3                      ((uint32_t)0x00000800)            /*!<Bit 3 */

#define  TIM_SMCR_ETPS                       ((uint32_t)0x00003000)            /*!<ETPS[1:0] bits (External trigger prescaler) */
#define  TIM_SMCR_ETPS_0                     ((uint32_t)0x00001000)            /*!<Bit 0 */
#define  TIM_SMCR_ETPS_1                     ((uint32_t)0x00002000)            /*!<Bit 1 */

#define  TIM_SMCR_ECE                        ((uint32_t)0x00004000)            /*!<External clock enable */
#define  TIM_SMCR_ETP                        ((uint32_t)0x00008000)            /*!<External trigger polarity */

/*******************  Bit definition for TIM_DIER register  ******************/
#define  TIM_DIER_UIE                        ((uint32_t)0x00000001)            /*!<Update interrupt enable */
#define  TIM_DIER_CC1IE                      ((uint32_t)0x00000002)            /*!<Capture/Compare 1 interrupt enable */
#define  TIM_DIER_CC2IE                      ((uint32_t)0x00000004)            /*!<Capture/Compare 2 interrupt enable */
#define  TIM_DIER_CC3IE                      ((uint32_t)0x00000008)            /*!<Capture/Compare 3 interrupt enable */
#define  TIM_DIER_CC4IE                      ((uint32_t)0x00000010)            /*!<Capture/Compare 4 interrupt enable */
#define  TIM_DIER_COMIE                      ((uint32_t)0x00000020)            /*!<COM interrupt enable */
#define  TIM_DIER_TIE                        ((uint32_t)0x00000040)            /*!<Trigger interrupt enable */
#define  TIM_DIER_BIE                        ((uint32_t)0x00000080)            /*!<Break interrupt enable */
#define  TIM_DIER_UDE                        ((uint32_t)0x00000100)            /*!<Update DMA request enable */
#define  TIM_DIER_CC1DE                      ((uint32_t)0x00000200)            /*!<Capture/Compare 1 DMA request enable */
#define  TIM_DIER_CC2DE                      ((uint32_t)0x00000400)            /*!<Capture/Compare 2 DMA request enable */
#define  TIM_DIER_CC3DE                      ((uint32_t)0x00000800)            /*!<Capture/Compare 3 DMA request enable */
#define  TIM_DIER_CC4DE                      ((uint32_t)0x00001000)            /*!<Capture/Compare 4 DMA request enable */
#define  TIM_DIER_COMDE                      ((uint32_t)0x00002000)            /*!<COM DMA request enable */
#define  TIM_DIER_TDE                        ((uint32_t)0x00004000)            /*!<Trigger DMA request enable */

/********************  Bit definition for TIM_SR register  *******************/
#define  TIM_SR_UIF                          ((uint32_t)0x00000001)            /*!<Update interrupt Flag */
#define  TIM_SR_CC1IF                        ((uint32_t)0x00000002)            /*!<Capture/Compare 1 interrupt Flag */
#define  TIM_SR_CC2IF                        ((uint32_t)0x00000004)            /*!<Capture/Compare 2 interrupt Flag */
#define  TIM_SR_CC3IF                        ((uint32_t)0x00000008)            /*!<Capture/Compare 3 interrupt Flag */
#define  TIM_SR_CC4IF                        ((uint32_t)0x00000010)            /*!<Capture/Compare 4 interrupt Flag */
#define  TIM_SR_COMIF                        ((uint32_t)0x00000020)            /*!<COM interrupt Flag */
#define  TIM_SR_TIF                          ((uint32_t)0x00000040)            /*!<Trigger interrupt Flag */
#define  TIM_SR_BIF                          ((uint32_t)0x00000080)            /*!<Break interrupt Flag */
#define  TIM_SR_CC1OF                        ((uint32_t)0x00000200)            /*!<Capture/Compare 1 Overcapture Flag */
#define  TIM_SR_CC2OF                        ((uint32_t)0x00000400)            /*!<Capture/Compare 2 Overcapture Flag */
#define  TIM_SR_CC3OF                        ((uint32_t)0x00000800)            /*!<Capture/Compare 3 Overcapture Flag */
#define  TIM_SR_CC4OF                        ((uint32_t)0x00001000)            /*!<Capture/Compare 4 Overcapture Flag */

/*******************  Bit definition for TIM_EGR register  *******************/
#define  TIM_EGR_UG                          ((uint32_t)0x00000001)               /*!<Update Generation */
#define  TIM_EGR_CC1G                        ((uint32_t)0x00000002)               /*!<Capture/Compare 1 Generation */
#define  TIM_EGR_CC2G                        ((uint32_t)0x00000004)               /*!<Capture/Compare 2 Generation */
#define  TIM_EGR_CC3G                        ((uint32_t)0x00000008)               /*!<Capture/Compare 3 Generation */
#define  TIM_EGR_CC4G                        ((uint32_t)0x00000010)               /*!<Capture/Compare 4 Generation */
#define  TIM_EGR_COMG                        ((uint32_t)0x00000020)               /*!<Capture/Compare Control Update Generation */
#define  TIM_EGR_TG                          ((uint32_t)0x00000040)               /*!<Trigger Generation */
#define  TIM_EGR_BG                          ((uint32_t)0x00000080)               /*!<Break Generation */

/******************  Bit definition for TIM_CCMR1 register  ******************/
#define  TIM_CCMR1_CC1S                      ((uint32_t)0x00000003)            /*!<CC1S[1:0] bits (Capture/Compare 1 Selection) */
#define  TIM_CCMR1_CC1S_0                    ((uint32_t)0x00000001)            /*!<Bit 0 */
#define  TIM_CCMR1_CC1S_1                    ((uint32_t)0x00000002)            /*!<Bit 1 */

#define  TIM_CCMR1_OC1FE                     ((uint32_t)0x00000004)            /*!<Output Compare 1 Fast enable */
#define  TIM_CCMR1_OC1PE                     ((uint32_t)0x00000008)            /*!<Output Compare 1 Preload enable */

#define  TIM_CCMR1_OC1M                      ((uint32_t)0x00000070)            /*!<OC1M[2:0] bits (Output Compare 1 Mode) */
#define  TIM_CCMR1_OC1M_0                    ((uint32_t)0x00000010)            /*!<Bit 0 */
#define  TIM_CCMR1_OC1M_1                    ((uint32_t)0x00000020)            /*!<Bit 1 */
#define  TIM_CCMR1_OC1M_2                    ((uint32_t)0x00000040)            /*!<Bit 2 */

#define  TIM_CCMR1_OC1CE                     ((uint32_t)0x00000080)            /*!<Output Compare 1Clear Enable */

#define  TIM_CCMR1_CC2S                      ((uint32_t)0x00000300)            /*!<CC2S[1:0] bits (Capture/Compare 2 Selection) */
#define  TIM_CCMR1_CC2S_0                    ((uint32_t)0x00000100)            /*!<Bit 0 */
#define  TIM_CCMR1_CC2S_1                    ((uint32_t)0x00000200)            /*!<Bit 1 */

#define  TIM_CCMR1_OC2FE                     ((uint32_t)0x00000400)            /*!<Output Compare 2 Fast enable */
#define  TIM_CCMR1_OC2PE                     ((uint32_t)0x00000800)            /*!<Output Compare 2 Preload enable */

#define  TIM_CCMR1_OC2M                      ((uint32_t)0x00007000)            /*!<OC2M[2:0] bits (Output Compare 2 Mode) */
#define  TIM_CCMR1_OC2M_0                    ((uint32_t)0x00001000)            /*!<Bit 0 */
#define  TIM_CCMR1_OC2M_1                    ((uint32_t)0x00002000)            /*!<Bit 1 */
#define  TIM_CCMR1_OC2M_2                    ((uint32_t)0x00004000)            /*!<Bit 2 */

#define  TIM_CCMR1_OC2CE                     ((uint32_t)0x00008000)            /*!<Output Compare 2 Clear Enable */

/*---------------------------------------------------------------------------*/

#define  TIM_CCMR1_IC1PSC                    ((uint32_t)0x0000000C)            /*!<IC1PSC[1:0] bits (Input Capture 1 Prescaler) */
#define  TIM_CCMR1_IC1PSC_0                  ((uint32_t)0x00000004)            /*!<Bit 0 */
#define  TIM_CCMR1_IC1PSC_1                  ((uint32_t)0x00000008)            /*!<Bit 1 */

#define  TIM_CCMR1_IC1F                      ((uint32_t)0x000000F0)            /*!<IC1F[3:0] bits (Input Capture 1 Filter) */
#define  TIM_CCMR1_IC1F_0                    ((uint32_t)0x00000010)            /*!<Bit 0 */
#define  TIM_CCMR1_IC1F_1                    ((uint32_t)0x00000020)            /*!<Bit 1 */
#define  TIM_CCMR1_IC1F_2                    ((uint32_t)0x00000040)            /*!<Bit 2 */
#define  TIM_CCMR1_IC1F_3                    ((uint32_t)0x00000080)            /*!<Bit 3 */

#define  TIM_CCMR1_IC2PSC                    ((uint32_t)0x00000C00)            /*!<IC2PSC[1:0] bits (Input Capture 2 Prescaler) */
#define  TIM_CCMR1_IC2PSC_0                  ((uint32_t)0x00000400)            /*!<Bit 0 */
#define  TIM_CCMR1_IC2PSC_1                  ((uint32_t)0x00000800)            /*!<Bit 1 */

#define  TIM_CCMR1_IC2F                      ((uint32_t)0x0000F000)            /*!<IC2F[3:0] bits (Input Capture 2 Filter) */
#define  TIM_CCMR1_IC2F_0                    ((uint32_t)0x00001000)            /*!<Bit 0 */
#define  TIM_CCMR1_IC2F_1                    ((uint32_t)0x00002000)            /*!<Bit 1 */
#define  TIM_CCMR1_IC2F_2                    ((uint32_t)0x00004000)            /*!<Bit 2 */
#define  TIM_CCMR1_IC2F_3                    ((uint32_t)0x00008000)            /*!<Bit 3 */

/******************  Bit definition for TIM_CCMR2 register  ******************/
#define  TIM_CCMR2_CC3S                      ((uint32_t)0x00000003)            /*!<CC3S[1:0] bits (Capture/Compare 3 Selection) */
#define  TIM_CCMR2_CC3S_0                    ((uint32_t)0x00000001)            /*!<Bit 0 */
#define  TIM_CCMR2_CC3S_1                    ((uint32_t)0x00000002)            /*!<Bit 1 */

#define  TIM_CCMR2_OC3FE                     ((uint32_t)0x00000004)            /*!<Output Compare 3 Fast enable */
#define  TIM_CCMR2_OC3PE                     ((uint32_t)0x00000008)            /*!<Output Compare 3 Preload enable */

#define  TIM_CCMR2_OC3M                      ((uint32_t)0x00000070)            /*!<OC3M[2:0] bits (Output Compare 3 Mode) */
#define  TIM_CCMR2_OC3M_0                    ((uint32_t)0x00000010)            /*!<Bit 0 */
#define  TIM_CCMR2_OC3M_1                    ((uint32_t)0x00000020)            /*!<Bit 1 */
#define  TIM_CCMR2_OC3M_2                    ((uint32_t)0x00000040)            /*!<Bit 2 */

#define  TIM_CCMR2_OC3CE                     ((uint32_t)0x00000080)            /*!<Output Compare 3 Clear Enable */

#define  TIM_CCMR2_CC4S                      ((uint32_t)0x00000300)            /*!<CC4S[1:0] bits (Capture/Compare 4 Selection) */
#define  TIM_CCMR2_CC4S_0                    ((uint32_t)0x00000100)            /*!<Bit 0 */
#define  TIM_CCMR2_CC4S_1                    ((uint32_t)0x00000200)            /*!<Bit 1 */

#define  TIM_CCMR2_OC4FE                     ((uint32_t)0x00000400)            /*!<Output Compare 4 Fast enable */
#define  TIM_CCMR2_OC4PE                     ((uint32_t)0x00000800)            /*!<Output Compare 4 Preload enable */

#define  TIM_CCMR2_OC4M                      ((uint32_t)0x00007000)            /*!<OC4M[2:0] bits (Output Compare 4 Mode) */
#define  TIM_CCMR2_OC4M_0                    ((uint32_t)0x00001000)            /*!<Bit 0 */
#define  TIM_CCMR2_OC4M_1                    ((uint32_t)0x00002000)            /*!<Bit 1 */
#define  TIM_CCMR2_OC4M_2                    ((uint32_t)0x00004000)            /*!<Bit 2 */

#define  TIM_CCMR2_OC4CE                     ((uint32_t)0x00008000)            /*!<Output Compare 4 Clear Enable */

/*---------------------------------------------------------------------------*/

#define  TIM_CCMR2_IC3PSC                    ((uint32_t)0x0000000C)            /*!<IC3PSC[1:0] bits (Input Capture 3 Prescaler) */
#define  TIM_CCMR2_IC3PSC_0                  ((uint32_t)0x00000004)            /*!<Bit 0 */
#define  TIM_CCMR2_IC3PSC_1                  ((uint32_t)0x00000008)            /*!<Bit 1 */

#define  TIM_CCMR2_IC3F                      ((uint32_t)0x000000F0)            /*!<IC3F[3:0] bits (Input Capture 3 Filter) */
#define  TIM_CCMR2_IC3F_0                    ((uint32_t)0x00000010)            /*!<Bit 0 */
#define  TIM_CCMR2_IC3F_1                    ((uint32_t)0x00000020)            /*!<Bit 1 */
#define  TIM_CCMR2_IC3F_2                    ((uint32_t)0x00000040)            /*!<Bit 2 */
#define  TIM_CCMR2_IC3F_3                    ((uint32_t)0x00000080)            /*!<Bit 3 */

#define  TIM_CCMR2_IC4PSC                    ((uint32_t)0x00000C00)            /*!<IC4PSC[1:0] bits (Input Capture 4 Prescaler) */
#define  TIM_CCMR2_IC4PSC_0                  ((uint32_t)0x00000400)            /*!<Bit 0 */
#define  TIM_CCMR2_IC4PSC_1                  ((uint32_t)0x00000800)            /*!<Bit 1 */

#define  TIM_CCMR2_IC4F                      ((uint32_t)0x0000F000)            /*!<IC4F[3:0] bits (Input Capture 4 Filter) */
#define  TIM_CCMR2_IC4F_0                    ((uint32_t)0x00001000)            /*!<Bit 0 */
#define  TIM_CCMR2_IC4F_1                    ((uint32_t)0x00002000)            /*!<Bit 1 */
#define  TIM_CCMR2_IC4F_2                    ((uint32_t)0x00004000)            /*!<Bit 2 */
#define  TIM_CCMR2_IC4F_3                    ((uint32_t)0x00008000)            /*!<Bit 3 */

/*******************  Bit definition for TIM_CCER register  ******************/
#define  TIM_CCER_CC1E                       ((uint32_t)0x00000001)            /*!<Capture/Compare 1 output enable */
#define  TIM_CCER_CC1P                       ((uint32_t)0x00000002)            /*!<Capture/Compare 1 output Polarity */
#define  TIM_CCER_CC1NE                      ((uint32_t)0x00000004)            /*!<Capture/Compare 1 Complementary output enable */
#define  TIM_CCER_CC1NP                      ((uint32_t)0x00000008)            /*!<Capture/Compare 1 Complementary output Polarity */
#define  TIM_CCER_CC2E                       ((uint32_t)0x00000010)            /*!<Capture/Compare 2 output enable */
#define  TIM_CCER_CC2P                       ((uint32_t)0x00000020)            /*!<Capture/Compare 2 output Polarity */
#define  TIM_CCER_CC2NE                      ((uint32_t)0x00000040)            /*!<Capture/Compare 2 Complementary output enable */
#define  TIM_CCER_CC2NP                      ((uint32_t)0x00000080)            /*!<Capture/Compare 2 Complementary output Polarity */
#define  TIM_CCER_CC3E                       ((uint32_t)0x00000100)            /*!<Capture/Compare 3 output enable */
#define  TIM_CCER_CC3P                       ((uint32_t)0x00000200)            /*!<Capture/Compare 3 output Polarity */
#define  TIM_CCER_CC3NE                      ((uint32_t)0x00000400)            /*!<Capture/Compare 3 Complementary output enable */
#define  TIM_CCER_CC3NP                      ((uint32_t)0x00000800)            /*!<Capture/Compare 3 Complementary output Polarity */
#define  TIM_CCER_CC4E                       ((uint32_t)0x00001000)            /*!<Capture/Compare 4 output enable */
#define  TIM_CCER_CC4P                       ((uint32_t)0x00002000)            /*!<Capture/Compare 4 output Polarity */
#define  TIM_CCER_CC4NP                      ((uint32_t)0x00008000)            /*!<Capture/Compare 4 Complementary output Polarity */

/*******************  Bit definition for TIM_CNT register  *******************/
#define  TIM_CNT_CNT                         ((uint32_t)0xFFFFFFFF)            /*!<Counter Value */

/*******************  Bit definition for TIM_PSC register  *******************/
#define  TIM_PSC_PSC                         ((uint32_t)0x0000FFFF)            /*!<Prescaler Value */

/*******************  Bit definition for TIM_ARR register  *******************/
#define  TIM_ARR_ARR                         ((uint32_t)0xFFFFFFFF)            /*!<actual auto-reload Value */

/*******************  Bit definition for TIM_RCR register  *******************/
#define  TIM_RCR_REP                         ((uint32_t)0x000000FF)               /*!<Repetition Counter Value */

/*******************  Bit definition for TIM_CCR1 register  ******************/
#define  TIM_CCR1_CCR1                       ((uint32_t)0x0000FFFF)            /*!<Capture/Compare 1 Value */

/*******************  Bit definition for TIM_CCR2 register  ******************/
#define  TIM_CCR2_CCR2                       ((uint32_t)0x0000FFFF)            /*!<Capture/Compare 2 Value */

/*******************  Bit definition for TIM_CCR3 register  ******************/
#define  TIM_CCR3_CCR3                       ((uint32_t)0x0000FFFF)            /*!<Capture/Compare 3 Value */

/*******************  Bit definition for TIM_CCR4 register  ******************/
#define  TIM_CCR4_CCR4                       ((uint32_t)0x0000FFFF)            /*!<Capture/Compare 4 Value */

/*******************  Bit definition for TIM_BDTR register  ******************/
#define  TIM_BDTR_DTG                        ((uint32_t)0x000000FF)            /*!<DTG[0:7] bits (Dead-Time Generator set-up) */
#define  TIM_BDTR_DTG_0                      ((uint32_t)0x00000001)            /*!<Bit 0 */
#define  TIM_BDTR_DTG_1                      ((uint32_t)0x00000002)            /*!<Bit 1 */
#define  TIM_BDTR_DTG_2                      ((uint32_t)0x00000004)            /*!<Bit 2 */
#define  TIM_BDTR_DTG_3                      ((uint32_t)0x00000008)            /*!<Bit 3 */
#define  TIM_BDTR_DTG_4                      ((uint32_t)0x00000010)            /*!<Bit 4 */
#define  TIM_BDTR_DTG_5                      ((uint32_t)0x00000020)            /*!<Bit 5 */
#define  TIM_BDTR_DTG_6                      ((uint32_t)0x00000040)            /*!<Bit 6 */
#define  TIM_BDTR_DTG_7                      ((uint32_t)0x00000080)            /*!<Bit 7 */

#define  TIM_BDTR_LOCK                       ((uint32_t)0x00000300)            /*!<LOCK[1:0] bits (Lock Configuration) */
#define  TIM_BDTR_LOCK_0                     ((uint32_t)0x00000100)            /*!<Bit 0 */
#define  TIM_BDTR_LOCK_1                     ((uint32_t)0x00000200)            /*!<Bit 1 */

#define  TIM_BDTR_OSSI                       ((uint32_t)0x00000400)            /*!<Off-State Selection for Idle mode */
#define  TIM_BDTR_OSSR                       ((uint32_t)0x00000800)            /*!<Off-State Selection for Run mode */
#define  TIM_BDTR_BKE                        ((uint32_t)0x00001000)            /*!<Break enable */
#define  TIM_BDTR_BKP                        ((uint32_t)0x00002000)            /*!<Break Polarity */
#define  TIM_BDTR_AOE                        ((uint32_t)0x00004000)            /*!<Automatic Output enable */
#define  TIM_BDTR_MOE                        ((uint32_t)0x00008000)            /*!<Main Output enable */

/*******************  Bit definition for TIM_DCR register  *******************/
#define  TIM_DCR_DBA                         ((uint32_t)0x0000001F)            /*!<DBA[4:0] bits (DMA Base Address) */
#define  TIM_DCR_DBA_0                       ((uint32_t)0x00000001)            /*!<Bit 0 */
#define  TIM_DCR_DBA_1                       ((uint32_t)0x00000002)            /*!<Bit 1 */
#define  TIM_DCR_DBA_2                       ((uint32_t)0x00000004)            /*!<Bit 2 */
#define  TIM_DCR_DBA_3                       ((uint32_t)0x00000008)            /*!<Bit 3 */
#define  TIM_DCR_DBA_4                       ((uint32_t)0x00000010)            /*!<Bit 4 */

#define  TIM_DCR_DBL                         ((uint32_t)0x00001F00)            /*!<DBL[4:0] bits (DMA Burst Length) */
#define  TIM_DCR_DBL_0                       ((uint32_t)0x00000100)            /*!<Bit 0 */
#define  TIM_DCR_DBL_1                       ((uint32_t)0x00000200)            /*!<Bit 1 */
#define  TIM_DCR_DBL_2                       ((uint32_t)0x00000400)            /*!<Bit 2 */
#define  TIM_DCR_DBL_3                       ((uint32_t)0x00000800)            /*!<Bit 3 */
#define  TIM_DCR_DBL_4                       ((uint32_t)0x00001000)            /*!<Bit 4 */

/*******************  Bit definition for TIM_DMAR register  ******************/
#define  TIM_DMAR_DMAB                       ((uint32_t)0x0000FFFF)            /*!<DMA register for burst accesses */

/*******************  Bit definition for TIM_OR register  ********************/

/******************************************************************************/
/*                                                                            */
/*                             Real-Time Clock                                */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for RTC_CRH register  ********************/
#define  RTC_CRH_SECIE                       ((uint32_t)0x00000001)               /*!< Second Interrupt Enable */
#define  RTC_CRH_ALRIE                       ((uint32_t)0x00000002)               /*!< Alarm Interrupt Enable */
#define  RTC_CRH_OWIE                        ((uint32_t)0x00000004)               /*!< OverfloW Interrupt Enable */

/*******************  Bit definition for RTC_CRL register  ********************/
#define  RTC_CRL_SECF                        ((uint32_t)0x00000001)               /*!< Second Flag */
#define  RTC_CRL_ALRF                        ((uint32_t)0x00000002)               /*!< Alarm Flag */
#define  RTC_CRL_OWF                         ((uint32_t)0x00000004)               /*!< OverfloW Flag */
#define  RTC_CRL_RSF                         ((uint32_t)0x00000008)               /*!< Registers Synchronized Flag */
#define  RTC_CRL_CNF                         ((uint32_t)0x00000010)               /*!< Configuration Flag */
#define  RTC_CRL_RTOFF                       ((uint32_t)0x00000020)               /*!< RTC operation OFF */

/*******************  Bit definition for RTC_PRLH register  *******************/
#define  RTC_PRLH_PRL                        ((uint32_t)0x0000000F)            /*!< RTC Prescaler Reload Value High */

/*******************  Bit definition for RTC_PRLL register  *******************/
#define  RTC_PRLL_PRL                        ((uint32_t)0x0000FFFF)            /*!< RTC Prescaler Reload Value Low */

/*******************  Bit definition for RTC_DIVH register  *******************/
#define  RTC_DIVH_RTC_DIV                    ((uint32_t)0x0000000F)            /*!< RTC Clock Divider High */

/*******************  Bit definition for RTC_DIVL register  *******************/
#define  RTC_DIVL_RTC_DIV                    ((uint32_t)0x0000FFFF)            /*!< RTC Clock Divider Low */

/*******************  Bit definition for RTC_CNTH register  *******************/
#define  RTC_CNTH_RTC_CNT                    ((uint32_t)0x0000FFFF)            /*!< RTC Counter High */

/*******************  Bit definition for RTC_CNTL register  *******************/
#define  RTC_CNTL_RTC_CNT                    ((uint32_t)0x0000FFFF)            /*!< RTC Counter Low */

/*******************  Bit definition for RTC_ALRH register  *******************/
#define  RTC_ALRH_RTC_ALR                    ((uint32_t)0x0000FFFF)            /*!< RTC Alarm High */

/*******************  Bit definition for RTC_ALRL register  *******************/
#define  RTC_ALRL_RTC_ALR                    ((uint32_t)0x0000FFFF)            /*!< RTC Alarm Low */

/******************************************************************************/
/*                                                                            */
/*                        Independent WATCHDOG (IWDG)                         */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for IWDG_KR register  ********************/
#define  IWDG_KR_KEY                         ((uint32_t)0x0000FFFF)            /*!< Key value (write only, read 0000h) */

/*******************  Bit definition for IWDG_PR register  ********************/
#define  IWDG_PR_PR                          ((uint32_t)0x00000007)               /*!< PR[2:0] (Prescaler divider) */
#define  IWDG_PR_PR_0                        ((uint32_t)0x00000001)               /*!< Bit 0 */
#define  IWDG_PR_PR_1                        ((uint32_t)0x00000002)               /*!< Bit 1 */
#define  IWDG_PR_PR_2                        ((uint32_t)0x00000004)               /*!< Bit 2 */

/*******************  Bit definition for IWDG_RLR register  *******************/
#define  IWDG_RLR_RL                         ((uint32_t)0x00000FFF)            /*!< Watchdog counter reload value */

/*******************  Bit definition for IWDG_SR register  ********************/
#define  IWDG_SR_PVU                         ((uint32_t)0x00000001)               /*!< Watchdog prescaler value update */
#define  IWDG_SR_RVU                         ((uint32_t)0x00000002)               /*!< Watchdog counter reload value update */

/******************************************************************************/
/*                                                                            */
/*                            Window WATCHDOG                                 */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for WWDG_CR register  ********************/
#define  WWDG_CR_T                           ((uint32_t)0x0000007F)               /*!< T[6:0] bits (7-Bit counter (MSB to LSB)) */
#define  WWDG_CR_T0                          ((uint32_t)0x00000001)               /*!< Bit 0 */
#define  WWDG_CR_T1                          ((uint32_t)0x00000002)               /*!< Bit 1 */
#define  WWDG_CR_T2                          ((uint32_t)0x00000004)               /*!< Bit 2 */
#define  WWDG_CR_T3                          ((uint32_t)0x00000008)               /*!< Bit 3 */
#define  WWDG_CR_T4                          ((uint32_t)0x00000010)               /*!< Bit 4 */
#define  WWDG_CR_T5                          ((uint32_t)0x00000020)               /*!< Bit 5 */
#define  WWDG_CR_T6                          ((uint32_t)0x00000040)               /*!< Bit 6 */

#define  WWDG_CR_WDGA                        ((uint32_t)0x00000080)               /*!< Activation bit */

/*******************  Bit definition for WWDG_CFR register  *******************/
#define  WWDG_CFR_W                          ((uint32_t)0x0000007F)            /*!< W[6:0] bits (7-bit window value) */
#define  WWDG_CFR_W0                         ((uint32_t)0x00000001)            /*!< Bit 0 */
#define  WWDG_CFR_W1                         ((uint32_t)0x00000002)            /*!< Bit 1 */
#define  WWDG_CFR_W2                         ((uint32_t)0x00000004)            /*!< Bit 2 */
#define  WWDG_CFR_W3                         ((uint32_t)0x00000008)            /*!< Bit 3 */
#define  WWDG_CFR_W4                         ((uint32_t)0x00000010)            /*!< Bit 4 */
#define  WWDG_CFR_W5                         ((uint32_t)0x00000020)            /*!< Bit 5 */
#define  WWDG_CFR_W6                         ((uint32_t)0x00000040)            /*!< Bit 6 */

#define  WWDG_CFR_WDGTB                      ((uint32_t)0x00000180)            /*!< WDGTB[1:0] bits (Timer Base) */
#define  WWDG_CFR_WDGTB0                     ((uint32_t)0x00000080)            /*!< Bit 0 */
#define  WWDG_CFR_WDGTB1                     ((uint32_t)0x00000100)            /*!< Bit 1 */

#define  WWDG_CFR_EWI                        ((uint32_t)0x00000200)            /*!< Early Wakeup Interrupt */

/*******************  Bit definition for WWDG_SR register  ********************/
#define  WWDG_SR_EWIF                        ((uint32_t)0x00000001)               /*!< Early Wakeup Interrupt Flag */


/******************************************************************************/
/*                                                                            */
/*                          SD host Interface                                 */
/*                                                                            */
/******************************************************************************/

/******************  Bit definition for SDIO_POWER register  ******************/
#define  SDIO_POWER_PWRCTRL                  ((uint32_t)0x03)               /*!< PWRCTRL[1:0] bits (Power supply control bits) */
#define  SDIO_POWER_PWRCTRL_0                ((uint32_t)0x01)               /*!< Bit 0 */
#define  SDIO_POWER_PWRCTRL_1                ((uint32_t)0x02)               /*!< Bit 1 */

/******************  Bit definition for SDIO_CLKCR register  ******************/
#define  SDIO_CLKCR_CLKDIV                   ((uint32_t)0x00FF)            /*!< Clock divide factor */
#define  SDIO_CLKCR_CLKEN                    ((uint32_t)0x0100)            /*!< Clock enable bit */
#define  SDIO_CLKCR_PWRSAV                   ((uint32_t)0x0200)            /*!< Power saving configuration bit */
#define  SDIO_CLKCR_BYPASS                   ((uint32_t)0x0400)            /*!< Clock divider bypass enable bit */

#define  SDIO_CLKCR_WIDBUS                   ((uint32_t)0x1800)            /*!< WIDBUS[1:0] bits (Wide bus mode enable bit) */
#define  SDIO_CLKCR_WIDBUS_0                 ((uint32_t)0x0800)            /*!< Bit 0 */
#define  SDIO_CLKCR_WIDBUS_1                 ((uint32_t)0x1000)            /*!< Bit 1 */

#define  SDIO_CLKCR_NEGEDGE                  ((uint32_t)0x2000)            /*!< SDIO_CK dephasing selection bit */
#define  SDIO_CLKCR_HWFC_EN                  ((uint32_t)0x4000)            /*!< HW Flow Control enable */

/*******************  Bit definition for SDIO_ARG register  *******************/
#define  SDIO_ARG_CMDARG                     ((uint32_t)0xFFFFFFFF)            /*!< Command argument */

/*******************  Bit definition for SDIO_CMD register  *******************/
#define  SDIO_CMD_CMDINDEX                   ((uint32_t)0x003F)            /*!< Command Index */

#define  SDIO_CMD_WAITRESP                   ((uint32_t)0x00C0)            /*!< WAITRESP[1:0] bits (Wait for response bits) */
#define  SDIO_CMD_WAITRESP_0                 ((uint32_t)0x0040)            /*!<  Bit 0 */
#define  SDIO_CMD_WAITRESP_1                 ((uint32_t)0x0080)            /*!<  Bit 1 */

#define  SDIO_CMD_WAITINT                    ((uint32_t)0x0100)            /*!< CPSM Waits for Interrupt Request */
#define  SDIO_CMD_WAITPEND                   ((uint32_t)0x0200)            /*!< CPSM Waits for ends of data transfer (CmdPend internal signal) */
#define  SDIO_CMD_CPSMEN                     ((uint32_t)0x0400)            /*!< Command path state machine (CPSM) Enable bit */
#define  SDIO_CMD_SDIOSUSPEND                ((uint32_t)0x0800)            /*!< SD I/O suspend command */
#define  SDIO_CMD_ENCMDCOMPL                 ((uint32_t)0x1000)            /*!< Enable CMD completion */
#define  SDIO_CMD_NIEN                       ((uint32_t)0x2000)            /*!< Not Interrupt Enable */
#define  SDIO_CMD_CEATACMD                   ((uint32_t)0x4000)            /*!< CE-ATA command */

/*****************  Bit definition for SDIO_RESPCMD register  *****************/
#define  SDIO_RESPCMD_RESPCMD                ((uint32_t)0x3F)               /*!< Response command index */

/******************  Bit definition for SDIO_RESP0 register  ******************/
#define  SDIO_RESP0_CARDSTATUS0              ((uint32_t)0xFFFFFFFF)        /*!< Card Status */

/******************  Bit definition for SDIO_RESP1 register  ******************/
#define  SDIO_RESP1_CARDSTATUS1              ((uint32_t)0xFFFFFFFF)        /*!< Card Status */

/******************  Bit definition for SDIO_RESP2 register  ******************/
#define  SDIO_RESP2_CARDSTATUS2              ((uint32_t)0xFFFFFFFF)        /*!< Card Status */

/******************  Bit definition for SDIO_RESP3 register  ******************/
#define  SDIO_RESP3_CARDSTATUS3              ((uint32_t)0xFFFFFFFF)        /*!< Card Status */

/******************  Bit definition for SDIO_RESP4 register  ******************/
#define  SDIO_RESP4_CARDSTATUS4              ((uint32_t)0xFFFFFFFF)        /*!< Card Status */

/******************  Bit definition for SDIO_DTIMER register  *****************/
#define  SDIO_DTIMER_DATATIME                ((uint32_t)0xFFFFFFFF)        /*!< Data timeout period. */

/******************  Bit definition for SDIO_DLEN register  *******************/
#define  SDIO_DLEN_DATALENGTH                ((uint32_t)0x01FFFFFF)        /*!< Data length value */

/******************  Bit definition for SDIO_DCTRL register  ******************/
#define  SDIO_DCTRL_DTEN                     ((uint32_t)0x0001)            /*!< Data transfer enabled bit */
#define  SDIO_DCTRL_DTDIR                    ((uint32_t)0x0002)            /*!< Data transfer direction selection */
#define  SDIO_DCTRL_DTMODE                   ((uint32_t)0x0004)            /*!< Data transfer mode selection */
#define  SDIO_DCTRL_DMAEN                    ((uint32_t)0x0008)            /*!< DMA enabled bit */

#define  SDIO_DCTRL_DBLOCKSIZE               ((uint32_t)0x00F0)            /*!< DBLOCKSIZE[3:0] bits (Data block size) */
#define  SDIO_DCTRL_DBLOCKSIZE_0             ((uint32_t)0x0010)            /*!< Bit 0 */
#define  SDIO_DCTRL_DBLOCKSIZE_1             ((uint32_t)0x0020)            /*!< Bit 1 */
#define  SDIO_DCTRL_DBLOCKSIZE_2             ((uint32_t)0x0040)            /*!< Bit 2 */
#define  SDIO_DCTRL_DBLOCKSIZE_3             ((uint32_t)0x0080)            /*!< Bit 3 */

#define  SDIO_DCTRL_RWSTART                  ((uint32_t)0x0100)            /*!< Read wait start */
#define  SDIO_DCTRL_RWSTOP                   ((uint32_t)0x0200)            /*!< Read wait stop */
#define  SDIO_DCTRL_RWMOD                    ((uint32_t)0x0400)            /*!< Read wait mode */
#define  SDIO_DCTRL_SDIOEN                   ((uint32_t)0x0800)            /*!< SD I/O enable functions */

/******************  Bit definition for SDIO_DCOUNT register  *****************/
#define  SDIO_DCOUNT_DATACOUNT               ((uint32_t)0x01FFFFFF)        /*!< Data count value */

/******************  Bit definition for SDIO_STA register  ********************/
#define  SDIO_STA_CCRCFAIL                   ((uint32_t)0x00000001)        /*!< Command response received (CRC check failed) */
#define  SDIO_STA_DCRCFAIL                   ((uint32_t)0x00000002)        /*!< Data block sent/received (CRC check failed) */
#define  SDIO_STA_CTIMEOUT                   ((uint32_t)0x00000004)        /*!< Command response timeout */
#define  SDIO_STA_DTIMEOUT                   ((uint32_t)0x00000008)        /*!< Data timeout */
#define  SDIO_STA_TXUNDERR                   ((uint32_t)0x00000010)        /*!< Transmit FIFO underrun error */
#define  SDIO_STA_RXOVERR                    ((uint32_t)0x00000020)        /*!< Received FIFO overrun error */
#define  SDIO_STA_CMDREND                    ((uint32_t)0x00000040)        /*!< Command response received (CRC check passed) */
#define  SDIO_STA_CMDSENT                    ((uint32_t)0x00000080)        /*!< Command sent (no response required) */
#define  SDIO_STA_DATAEND                    ((uint32_t)0x00000100)        /*!< Data end (data counter, SDIDCOUNT, is zero) */
#define  SDIO_STA_STBITERR                   ((uint32_t)0x00000200)        /*!< Start bit not detected on all data signals in wide bus mode */
#define  SDIO_STA_DBCKEND                    ((uint32_t)0x00000400)        /*!< Data block sent/received (CRC check passed) */
#define  SDIO_STA_CMDACT                     ((uint32_t)0x00000800)        /*!< Command transfer in progress */
#define  SDIO_STA_TXACT                      ((uint32_t)0x00001000)        /*!< Data transmit in progress */
#define  SDIO_STA_RXACT                      ((uint32_t)0x00002000)        /*!< Data receive in progress */
#define  SDIO_STA_TXFIFOHE                   ((uint32_t)0x00004000)        /*!< Transmit FIFO Half Empty: at least 8 words can be written into the FIFO */
#define  SDIO_STA_RXFIFOHF                   ((uint32_t)0x00008000)        /*!< Receive FIFO Half Full: there are at least 8 words in the FIFO */
#define  SDIO_STA_TXFIFOF                    ((uint32_t)0x00010000)        /*!< Transmit FIFO full */
#define  SDIO_STA_RXFIFOF                    ((uint32_t)0x00020000)        /*!< Receive FIFO full */
#define  SDIO_STA_TXFIFOE                    ((uint32_t)0x00040000)        /*!< Transmit FIFO empty */
#define  SDIO_STA_RXFIFOE                    ((uint32_t)0x00080000)        /*!< Receive FIFO empty */
#define  SDIO_STA_TXDAVL                     ((uint32_t)0x00100000)        /*!< Data available in transmit FIFO */
#define  SDIO_STA_RXDAVL                     ((uint32_t)0x00200000)        /*!< Data available in receive FIFO */
#define  SDIO_STA_SDIOIT                     ((uint32_t)0x00400000)        /*!< SDIO interrupt received */
#define  SDIO_STA_CEATAEND                   ((uint32_t)0x00800000)        /*!< CE-ATA command completion signal received for CMD61 */

/*******************  Bit definition for SDIO_ICR register  *******************/
#define  SDIO_ICR_CCRCFAILC                  ((uint32_t)0x00000001)        /*!< CCRCFAIL flag clear bit */
#define  SDIO_ICR_DCRCFAILC                  ((uint32_t)0x00000002)        /*!< DCRCFAIL flag clear bit */
#define  SDIO_ICR_CTIMEOUTC                  ((uint32_t)0x00000004)        /*!< CTIMEOUT flag clear bit */
#define  SDIO_ICR_DTIMEOUTC                  ((uint32_t)0x00000008)        /*!< DTIMEOUT flag clear bit */
#define  SDIO_ICR_TXUNDERRC                  ((uint32_t)0x00000010)        /*!< TXUNDERR flag clear bit */
#define  SDIO_ICR_RXOVERRC                   ((uint32_t)0x00000020)        /*!< RXOVERR flag clear bit */
#define  SDIO_ICR_CMDRENDC                   ((uint32_t)0x00000040)        /*!< CMDREND flag clear bit */
#define  SDIO_ICR_CMDSENTC                   ((uint32_t)0x00000080)        /*!< CMDSENT flag clear bit */
#define  SDIO_ICR_DATAENDC                   ((uint32_t)0x00000100)        /*!< DATAEND flag clear bit */
#define  SDIO_ICR_STBITERRC                  ((uint32_t)0x00000200)        /*!< STBITERR flag clear bit */
#define  SDIO_ICR_DBCKENDC                   ((uint32_t)0x00000400)        /*!< DBCKEND flag clear bit */
#define  SDIO_ICR_SDIOITC                    ((uint32_t)0x00400000)        /*!< SDIOIT flag clear bit */
#define  SDIO_ICR_CEATAENDC                  ((uint32_t)0x00800000)        /*!< CEATAEND flag clear bit */

/******************  Bit definition for SDIO_MASK register  *******************/
#define  SDIO_MASK_CCRCFAILIE                ((uint32_t)0x00000001)        /*!< Command CRC Fail Interrupt Enable */
#define  SDIO_MASK_DCRCFAILIE                ((uint32_t)0x00000002)        /*!< Data CRC Fail Interrupt Enable */
#define  SDIO_MASK_CTIMEOUTIE                ((uint32_t)0x00000004)        /*!< Command TimeOut Interrupt Enable */
#define  SDIO_MASK_DTIMEOUTIE                ((uint32_t)0x00000008)        /*!< Data TimeOut Interrupt Enable */
#define  SDIO_MASK_TXUNDERRIE                ((uint32_t)0x00000010)        /*!< Tx FIFO UnderRun Error Interrupt Enable */
#define  SDIO_MASK_RXOVERRIE                 ((uint32_t)0x00000020)        /*!< Rx FIFO OverRun Error Interrupt Enable */
#define  SDIO_MASK_CMDRENDIE                 ((uint32_t)0x00000040)        /*!< Command Response Received Interrupt Enable */
#define  SDIO_MASK_CMDSENTIE                 ((uint32_t)0x00000080)        /*!< Command Sent Interrupt Enable */
#define  SDIO_MASK_DATAENDIE                 ((uint32_t)0x00000100)        /*!< Data End Interrupt Enable */
#define  SDIO_MASK_STBITERRIE                ((uint32_t)0x00000200)        /*!< Start Bit Error Interrupt Enable */
#define  SDIO_MASK_DBCKENDIE                 ((uint32_t)0x00000400)        /*!< Data Block End Interrupt Enable */
#define  SDIO_MASK_CMDACTIE                  ((uint32_t)0x00000800)        /*!< Command Acting Interrupt Enable */
#define  SDIO_MASK_TXACTIE                   ((uint32_t)0x00001000)        /*!< Data Transmit Acting Interrupt Enable */
#define  SDIO_MASK_RXACTIE                   ((uint32_t)0x00002000)        /*!< Data receive acting interrupt enabled */
#define  SDIO_MASK_TXFIFOHEIE                ((uint32_t)0x00004000)        /*!< Tx FIFO Half Empty interrupt Enable */
#define  SDIO_MASK_RXFIFOHFIE                ((uint32_t)0x00008000)        /*!< Rx FIFO Half Full interrupt Enable */
#define  SDIO_MASK_TXFIFOFIE                 ((uint32_t)0x00010000)        /*!< Tx FIFO Full interrupt Enable */
#define  SDIO_MASK_RXFIFOFIE                 ((uint32_t)0x00020000)        /*!< Rx FIFO Full interrupt Enable */
#define  SDIO_MASK_TXFIFOEIE                 ((uint32_t)0x00040000)        /*!< Tx FIFO Empty interrupt Enable */
#define  SDIO_MASK_RXFIFOEIE                 ((uint32_t)0x00080000)        /*!< Rx FIFO Empty interrupt Enable */
#define  SDIO_MASK_TXDAVLIE                  ((uint32_t)0x00100000)        /*!< Data available in Tx FIFO interrupt Enable */
#define  SDIO_MASK_RXDAVLIE                  ((uint32_t)0x00200000)        /*!< Data available in Rx FIFO interrupt Enable */
#define  SDIO_MASK_SDIOITIE                  ((uint32_t)0x00400000)        /*!< SDIO Mode Interrupt Received interrupt Enable */
#define  SDIO_MASK_CEATAENDIE                ((uint32_t)0x00800000)        /*!< CE-ATA command completion signal received Interrupt Enable */

/*****************  Bit definition for SDIO_FIFOCNT register  *****************/
#define  SDIO_FIFOCNT_FIFOCOUNT              ((uint32_t)0x00FFFFFF)        /*!< Remaining number of words to be written to or read from the FIFO */

/******************  Bit definition for SDIO_FIFO register  *******************/
#define  SDIO_FIFO_FIFODATA                  ((uint32_t)0xFFFFFFFF)        /*!< Receive and transmit FIFO data */


/******************************************************************************/
/*                                                                            */
/*                         Controller Area Network                            */
/*                                                                            */
/******************************************************************************/

/*!< CAN control and status registers */
/*******************  Bit definition for CAN_MCR register  ********************/
#define  CAN_MCR_INRQ                        ((uint32_t)0x00000001)        /*!< Initialization Request */
#define  CAN_MCR_SLEEP                       ((uint32_t)0x00000002)        /*!< Sleep Mode Request */
#define  CAN_MCR_TXFP                        ((uint32_t)0x00000004)        /*!< Transmit FIFO Priority */
#define  CAN_MCR_RFLM                        ((uint32_t)0x00000008)        /*!< Receive FIFO Locked Mode */
#define  CAN_MCR_NART                        ((uint32_t)0x00000010)        /*!< No Automatic Retransmission */
#define  CAN_MCR_AWUM                        ((uint32_t)0x00000020)        /*!< Automatic Wakeup Mode */
#define  CAN_MCR_ABOM                        ((uint32_t)0x00000040)        /*!< Automatic Bus-Off Management */
#define  CAN_MCR_TTCM                        ((uint32_t)0x00000080)        /*!< Time Triggered Communication Mode */
#define  CAN_MCR_RESET                       ((uint32_t)0x00008000)        /*!< CAN software master reset */
#define  CAN_MCR_DBF                         ((uint32_t)0x00010000)        /*!< CAN Debug freeze */

/*******************  Bit definition for CAN_MSR register  ********************/
#define  CAN_MSR_INAK                        ((uint32_t)0x00000001)        /*!< Initialization Acknowledge */
#define  CAN_MSR_SLAK                        ((uint32_t)0x00000002)        /*!< Sleep Acknowledge */
#define  CAN_MSR_ERRI                        ((uint32_t)0x00000004)        /*!< Error Interrupt */
#define  CAN_MSR_WKUI                        ((uint32_t)0x00000008)        /*!< Wakeup Interrupt */
#define  CAN_MSR_SLAKI                       ((uint32_t)0x00000010)        /*!< Sleep Acknowledge Interrupt */
#define  CAN_MSR_TXM                         ((uint32_t)0x00000100)        /*!< Transmit Mode */
#define  CAN_MSR_RXM                         ((uint32_t)0x00000200)        /*!< Receive Mode */
#define  CAN_MSR_SAMP                        ((uint32_t)0x00000400)        /*!< Last Sample Point */
#define  CAN_MSR_RX                          ((uint32_t)0x00000800)        /*!< CAN Rx Signal */

/*******************  Bit definition for CAN_TSR register  ********************/
#define  CAN_TSR_RQCP0                       ((uint32_t)0x00000001)        /*!< Request Completed Mailbox0 */
#define  CAN_TSR_TXOK0                       ((uint32_t)0x00000002)        /*!< Transmission OK of Mailbox0 */
#define  CAN_TSR_ALST0                       ((uint32_t)0x00000004)        /*!< Arbitration Lost for Mailbox0 */
#define  CAN_TSR_TERR0                       ((uint32_t)0x00000008)        /*!< Transmission Error of Mailbox0 */
#define  CAN_TSR_ABRQ0                       ((uint32_t)0x00000080)        /*!< Abort Request for Mailbox0 */
#define  CAN_TSR_RQCP1                       ((uint32_t)0x00000100)        /*!< Request Completed Mailbox1 */
#define  CAN_TSR_TXOK1                       ((uint32_t)0x00000200)        /*!< Transmission OK of Mailbox1 */
#define  CAN_TSR_ALST1                       ((uint32_t)0x00000400)        /*!< Arbitration Lost for Mailbox1 */
#define  CAN_TSR_TERR1                       ((uint32_t)0x00000800)        /*!< Transmission Error of Mailbox1 */
#define  CAN_TSR_ABRQ1                       ((uint32_t)0x00008000)        /*!< Abort Request for Mailbox 1 */
#define  CAN_TSR_RQCP2                       ((uint32_t)0x00010000)        /*!< Request Completed Mailbox2 */
#define  CAN_TSR_TXOK2                       ((uint32_t)0x00020000)        /*!< Transmission OK of Mailbox 2 */
#define  CAN_TSR_ALST2                       ((uint32_t)0x00040000)        /*!< Arbitration Lost for mailbox 2 */
#define  CAN_TSR_TERR2                       ((uint32_t)0x00080000)        /*!< Transmission Error of Mailbox 2 */
#define  CAN_TSR_ABRQ2                       ((uint32_t)0x00800000)        /*!< Abort Request for Mailbox 2 */
#define  CAN_TSR_CODE                        ((uint32_t)0x03000000)        /*!< Mailbox Code */

#define  CAN_TSR_TME                         ((uint32_t)0x1C000000)        /*!< TME[2:0] bits */
#define  CAN_TSR_TME0                        ((uint32_t)0x04000000)        /*!< Transmit Mailbox 0 Empty */
#define  CAN_TSR_TME1                        ((uint32_t)0x08000000)        /*!< Transmit Mailbox 1 Empty */
#define  CAN_TSR_TME2                        ((uint32_t)0x10000000)        /*!< Transmit Mailbox 2 Empty */

#define  CAN_TSR_LOW                         ((uint32_t)0xE0000000)        /*!< LOW[2:0] bits */
#define  CAN_TSR_LOW0                        ((uint32_t)0x20000000)        /*!< Lowest Priority Flag for Mailbox 0 */
#define  CAN_TSR_LOW1                        ((uint32_t)0x40000000)        /*!< Lowest Priority Flag for Mailbox 1 */
#define  CAN_TSR_LOW2                        ((uint32_t)0x80000000)        /*!< Lowest Priority Flag for Mailbox 2 */

/*******************  Bit definition for CAN_RF0R register  *******************/
#define  CAN_RF0R_FMP0                       ((uint32_t)0x00000003)        /*!< FIFO 0 Message Pending */
#define  CAN_RF0R_FULL0                      ((uint32_t)0x00000008)        /*!< FIFO 0 Full */
#define  CAN_RF0R_FOVR0                      ((uint32_t)0x00000010)        /*!< FIFO 0 Overrun */
#define  CAN_RF0R_RFOM0                      ((uint32_t)0x00000020)        /*!< Release FIFO 0 Output Mailbox */

/*******************  Bit definition for CAN_RF1R register  *******************/
#define  CAN_RF1R_FMP1                       ((uint32_t)0x00000003)        /*!< FIFO 1 Message Pending */
#define  CAN_RF1R_FULL1                      ((uint32_t)0x00000008)        /*!< FIFO 1 Full */
#define  CAN_RF1R_FOVR1                      ((uint32_t)0x00000010)        /*!< FIFO 1 Overrun */
#define  CAN_RF1R_RFOM1                      ((uint32_t)0x00000020)        /*!< Release FIFO 1 Output Mailbox */

/********************  Bit definition for CAN_IER register  *******************/
#define  CAN_IER_TMEIE                       ((uint32_t)0x00000001)        /*!< Transmit Mailbox Empty Interrupt Enable */
#define  CAN_IER_FMPIE0                      ((uint32_t)0x00000002)        /*!< FIFO Message Pending Interrupt Enable */
#define  CAN_IER_FFIE0                       ((uint32_t)0x00000004)        /*!< FIFO Full Interrupt Enable */
#define  CAN_IER_FOVIE0                      ((uint32_t)0x00000008)        /*!< FIFO Overrun Interrupt Enable */
#define  CAN_IER_FMPIE1                      ((uint32_t)0x00000010)        /*!< FIFO Message Pending Interrupt Enable */
#define  CAN_IER_FFIE1                       ((uint32_t)0x00000020)        /*!< FIFO Full Interrupt Enable */
#define  CAN_IER_FOVIE1                      ((uint32_t)0x00000040)        /*!< FIFO Overrun Interrupt Enable */
#define  CAN_IER_EWGIE                       ((uint32_t)0x00000100)        /*!< Error Warning Interrupt Enable */
#define  CAN_IER_EPVIE                       ((uint32_t)0x00000200)        /*!< Error Passive Interrupt Enable */
#define  CAN_IER_BOFIE                       ((uint32_t)0x00000400)        /*!< Bus-Off Interrupt Enable */
#define  CAN_IER_LECIE                       ((uint32_t)0x00000800)        /*!< Last Error Code Interrupt Enable */
#define  CAN_IER_ERRIE                       ((uint32_t)0x00008000)        /*!< Error Interrupt Enable */
#define  CAN_IER_WKUIE                       ((uint32_t)0x00010000)        /*!< Wakeup Interrupt Enable */
#define  CAN_IER_SLKIE                       ((uint32_t)0x00020000)        /*!< Sleep Interrupt Enable */

/********************  Bit definition for CAN_ESR register  *******************/
#define  CAN_ESR_EWGF                        ((uint32_t)0x00000001)        /*!< Error Warning Flag */
#define  CAN_ESR_EPVF                        ((uint32_t)0x00000002)        /*!< Error Passive Flag */
#define  CAN_ESR_BOFF                        ((uint32_t)0x00000004)        /*!< Bus-Off Flag */

#define  CAN_ESR_LEC                         ((uint32_t)0x00000070)        /*!< LEC[2:0] bits (Last Error Code) */
#define  CAN_ESR_LEC_0                       ((uint32_t)0x00000010)        /*!< Bit 0 */
#define  CAN_ESR_LEC_1                       ((uint32_t)0x00000020)        /*!< Bit 1 */
#define  CAN_ESR_LEC_2                       ((uint32_t)0x00000040)        /*!< Bit 2 */

#define  CAN_ESR_TEC                         ((uint32_t)0x00FF0000)        /*!< Least significant byte of the 9-bit Transmit Error Counter */
#define  CAN_ESR_REC                         ((uint32_t)0xFF000000)        /*!< Receive Error Counter */

/*******************  Bit definition for CAN_BTR register  ********************/
#define  CAN_BTR_BRP                         ((uint32_t)0x000003FF)        /*!<Baud Rate Prescaler */
#define  CAN_BTR_TS1                         ((uint32_t)0x000F0000)        /*!<Time Segment 1 */
#define  CAN_BTR_TS1_0                       ((uint32_t)0x00010000)        /*!<Time Segment 1 (Bit 0) */
#define  CAN_BTR_TS1_1                       ((uint32_t)0x00020000)        /*!<Time Segment 1 (Bit 1) */
#define  CAN_BTR_TS1_2                       ((uint32_t)0x00040000)        /*!<Time Segment 1 (Bit 2) */
#define  CAN_BTR_TS1_3                       ((uint32_t)0x00080000)        /*!<Time Segment 1 (Bit 3) */
#define  CAN_BTR_TS2                         ((uint32_t)0x00700000)        /*!<Time Segment 2 */
#define  CAN_BTR_TS2_0                       ((uint32_t)0x00100000)        /*!<Time Segment 2 (Bit 0) */
#define  CAN_BTR_TS2_1                       ((uint32_t)0x00200000)        /*!<Time Segment 2 (Bit 1) */
#define  CAN_BTR_TS2_2                       ((uint32_t)0x00400000)        /*!<Time Segment 2 (Bit 2) */
#define  CAN_BTR_SJW                         ((uint32_t)0x03000000)        /*!<Resynchronization Jump Width */
#define  CAN_BTR_SJW_0                       ((uint32_t)0x01000000)        /*!<Resynchronization Jump Width (Bit 0) */
#define  CAN_BTR_SJW_1                       ((uint32_t)0x02000000)        /*!<Resynchronization Jump Width (Bit 1) */
#define  CAN_BTR_LBKM                        ((uint32_t)0x40000000)        /*!<Loop Back Mode (Debug) */
#define  CAN_BTR_SILM                        ((uint32_t)0x80000000)        /*!<Silent Mode */

/*!< Mailbox registers */
/******************  Bit definition for CAN_TI0R register  ********************/
#define  CAN_TI0R_TXRQ                       ((uint32_t)0x00000001)        /*!< Transmit Mailbox Request */
#define  CAN_TI0R_RTR                        ((uint32_t)0x00000002)        /*!< Remote Transmission Request */
#define  CAN_TI0R_IDE                        ((uint32_t)0x00000004)        /*!< Identifier Extension */
#define  CAN_TI0R_EXID                       ((uint32_t)0x001FFFF8)        /*!< Extended Identifier */
#define  CAN_TI0R_STID                       ((uint32_t)0xFFE00000)        /*!< Standard Identifier or Extended Identifier */

/******************  Bit definition for CAN_TDT0R register  *******************/
#define  CAN_TDT0R_DLC                       ((uint32_t)0x0000000F)        /*!< Data Length Code */
#define  CAN_TDT0R_TGT                       ((uint32_t)0x00000100)        /*!< Transmit Global Time */
#define  CAN_TDT0R_TIME                      ((uint32_t)0xFFFF0000)        /*!< Message Time Stamp */

/******************  Bit definition for CAN_TDL0R register  *******************/
#define  CAN_TDL0R_DATA0                     ((uint32_t)0x000000FF)        /*!< Data byte 0 */
#define  CAN_TDL0R_DATA1                     ((uint32_t)0x0000FF00)        /*!< Data byte 1 */
#define  CAN_TDL0R_DATA2                     ((uint32_t)0x00FF0000)        /*!< Data byte 2 */
#define  CAN_TDL0R_DATA3                     ((uint32_t)0xFF000000)        /*!< Data byte 3 */

/******************  Bit definition for CAN_TDH0R register  *******************/
#define  CAN_TDH0R_DATA4                     ((uint32_t)0x000000FF)        /*!< Data byte 4 */
#define  CAN_TDH0R_DATA5                     ((uint32_t)0x0000FF00)        /*!< Data byte 5 */
#define  CAN_TDH0R_DATA6                     ((uint32_t)0x00FF0000)        /*!< Data byte 6 */
#define  CAN_TDH0R_DATA7                     ((uint32_t)0xFF000000)        /*!< Data byte 7 */

/*******************  Bit definition for CAN_TI1R register  *******************/
#define  CAN_TI1R_TXRQ                       ((uint32_t)0x00000001)        /*!< Transmit Mailbox Request */
#define  CAN_TI1R_RTR                        ((uint32_t)0x00000002)        /*!< Remote Transmission Request */
#define  CAN_TI1R_IDE                        ((uint32_t)0x00000004)        /*!< Identifier Extension */
#define  CAN_TI1R_EXID                       ((uint32_t)0x001FFFF8)        /*!< Extended Identifier */
#define  CAN_TI1R_STID                       ((uint32_t)0xFFE00000)        /*!< Standard Identifier or Extended Identifier */

/*******************  Bit definition for CAN_TDT1R register  ******************/
#define  CAN_TDT1R_DLC                       ((uint32_t)0x0000000F)        /*!< Data Length Code */
#define  CAN_TDT1R_TGT                       ((uint32_t)0x00000100)        /*!< Transmit Global Time */
#define  CAN_TDT1R_TIME                      ((uint32_t)0xFFFF0000)        /*!< Message Time Stamp */

/*******************  Bit definition for CAN_TDL1R register  ******************/
#define  CAN_TDL1R_DATA0                     ((uint32_t)0x000000FF)        /*!< Data byte 0 */
#define  CAN_TDL1R_DATA1                     ((uint32_t)0x0000FF00)        /*!< Data byte 1 */
#define  CAN_TDL1R_DATA2                     ((uint32_t)0x00FF0000)        /*!< Data byte 2 */
#define  CAN_TDL1R_DATA3                     ((uint32_t)0xFF000000)        /*!< Data byte 3 */

/*******************  Bit definition for CAN_TDH1R register  ******************/
#define  CAN_TDH1R_DATA4                     ((uint32_t)0x000000FF)        /*!< Data byte 4 */
#define  CAN_TDH1R_DATA5                     ((uint32_t)0x0000FF00)        /*!< Data byte 5 */
#define  CAN_TDH1R_DATA6                     ((uint32_t)0x00FF0000)        /*!< Data byte 6 */
#define  CAN_TDH1R_DATA7                     ((uint32_t)0xFF000000)        /*!< Data byte 7 */

/*******************  Bit definition for CAN_TI2R register  *******************/
#define  CAN_TI2R_TXRQ                       ((uint32_t)0x00000001)        /*!< Transmit Mailbox Request */
#define  CAN_TI2R_RTR                        ((uint32_t)0x00000002)        /*!< Remote Transmission Request */
#define  CAN_TI2R_IDE                        ((uint32_t)0x00000004)        /*!< Identifier Extension */
#define  CAN_TI2R_EXID                       ((uint32_t)0x001FFFF8)        /*!< Extended identifier */
#define  CAN_TI2R_STID                       ((uint32_t)0xFFE00000)        /*!< Standard Identifier or Extended Identifier */

/*******************  Bit definition for CAN_TDT2R register  ******************/  
#define  CAN_TDT2R_DLC                       ((uint32_t)0x0000000F)        /*!< Data Length Code */
#define  CAN_TDT2R_TGT                       ((uint32_t)0x00000100)        /*!< Transmit Global Time */
#define  CAN_TDT2R_TIME                      ((uint32_t)0xFFFF0000)        /*!< Message Time Stamp */

/*******************  Bit definition for CAN_TDL2R register  ******************/
#define  CAN_TDL2R_DATA0                     ((uint32_t)0x000000FF)        /*!< Data byte 0 */
#define  CAN_TDL2R_DATA1                     ((uint32_t)0x0000FF00)        /*!< Data byte 1 */
#define  CAN_TDL2R_DATA2                     ((uint32_t)0x00FF0000)        /*!< Data byte 2 */
#define  CAN_TDL2R_DATA3                     ((uint32_t)0xFF000000)        /*!< Data byte 3 */

/*******************  Bit definition for CAN_TDH2R register  ******************/
#define  CAN_TDH2R_DATA4                     ((uint32_t)0x000000FF)        /*!< Data byte 4 */
#define  CAN_TDH2R_DATA5                     ((uint32_t)0x0000FF00)        /*!< Data byte 5 */
#define  CAN_TDH2R_DATA6                     ((uint32_t)0x00FF0000)        /*!< Data byte 6 */
#define  CAN_TDH2R_DATA7                     ((uint32_t)0xFF000000)        /*!< Data byte 7 */

/*******************  Bit definition for CAN_RI0R register  *******************/
#define  CAN_RI0R_RTR                        ((uint32_t)0x00000002)        /*!< Remote Transmission Request */
#define  CAN_RI0R_IDE                        ((uint32_t)0x00000004)        /*!< Identifier Extension */
#define  CAN_RI0R_EXID                       ((uint32_t)0x001FFFF8)        /*!< Extended Identifier */
#define  CAN_RI0R_STID                       ((uint32_t)0xFFE00000)        /*!< Standard Identifier or Extended Identifier */

/*******************  Bit definition for CAN_RDT0R register  ******************/
#define  CAN_RDT0R_DLC                       ((uint32_t)0x0000000F)        /*!< Data Length Code */
#define  CAN_RDT0R_FMI                       ((uint32_t)0x0000FF00)        /*!< Filter Match Index */
#define  CAN_RDT0R_TIME                      ((uint32_t)0xFFFF0000)        /*!< Message Time Stamp */

/*******************  Bit definition for CAN_RDL0R register  ******************/
#define  CAN_RDL0R_DATA0                     ((uint32_t)0x000000FF)        /*!< Data byte 0 */
#define  CAN_RDL0R_DATA1                     ((uint32_t)0x0000FF00)        /*!< Data byte 1 */
#define  CAN_RDL0R_DATA2                     ((uint32_t)0x00FF0000)        /*!< Data byte 2 */
#define  CAN_RDL0R_DATA3                     ((uint32_t)0xFF000000)        /*!< Data byte 3 */

/*******************  Bit definition for CAN_RDH0R register  ******************/
#define  CAN_RDH0R_DATA4                     ((uint32_t)0x000000FF)        /*!< Data byte 4 */
#define  CAN_RDH0R_DATA5                     ((uint32_t)0x0000FF00)        /*!< Data byte 5 */
#define  CAN_RDH0R_DATA6                     ((uint32_t)0x00FF0000)        /*!< Data byte 6 */
#define  CAN_RDH0R_DATA7                     ((uint32_t)0xFF000000)        /*!< Data byte 7 */

/*******************  Bit definition for CAN_RI1R register  *******************/
#define  CAN_RI1R_RTR                        ((uint32_t)0x00000002)        /*!< Remote Transmission Request */
#define  CAN_RI1R_IDE                        ((uint32_t)0x00000004)        /*!< Identifier Extension */
#define  CAN_RI1R_EXID                       ((uint32_t)0x001FFFF8)        /*!< Extended identifier */
#define  CAN_RI1R_STID                       ((uint32_t)0xFFE00000)        /*!< Standard Identifier or Extended Identifier */

/*******************  Bit definition for CAN_RDT1R register  ******************/
#define  CAN_RDT1R_DLC                       ((uint32_t)0x0000000F)        /*!< Data Length Code */
#define  CAN_RDT1R_FMI                       ((uint32_t)0x0000FF00)        /*!< Filter Match Index */
#define  CAN_RDT1R_TIME                      ((uint32_t)0xFFFF0000)        /*!< Message Time Stamp */

/*******************  Bit definition for CAN_RDL1R register  ******************/
#define  CAN_RDL1R_DATA0                     ((uint32_t)0x000000FF)        /*!< Data byte 0 */
#define  CAN_RDL1R_DATA1                     ((uint32_t)0x0000FF00)        /*!< Data byte 1 */
#define  CAN_RDL1R_DATA2                     ((uint32_t)0x00FF0000)        /*!< Data byte 2 */
#define  CAN_RDL1R_DATA3                     ((uint32_t)0xFF000000)        /*!< Data byte 3 */

/*******************  Bit definition for CAN_RDH1R register  ******************/
#define  CAN_RDH1R_DATA4                     ((uint32_t)0x000000FF)        /*!< Data byte 4 */
#define  CAN_RDH1R_DATA5                     ((uint32_t)0x0000FF00)        /*!< Data byte 5 */
#define  CAN_RDH1R_DATA6                     ((uint32_t)0x00FF0000)        /*!< Data byte 6 */
#define  CAN_RDH1R_DATA7                     ((uint32_t)0xFF000000)        /*!< Data byte 7 */

/*!< CAN filter registers */
/*******************  Bit definition for CAN_FMR register  ********************/
#define  CAN_FMR_FINIT                       ((uint32_t)0x00000001)        /*!< Filter Init Mode */
#define  CAN_FMR_CAN2SB                      ((uint32_t)0x00003F00)        /*!< CAN2 start bank */

/*******************  Bit definition for CAN_FM1R register  *******************/
#define  CAN_FM1R_FBM                        ((uint32_t)0x00003FFF)        /*!< Filter Mode */
#define  CAN_FM1R_FBM0                       ((uint32_t)0x00000001)        /*!< Filter Init Mode for filter 0 */
#define  CAN_FM1R_FBM1                       ((uint32_t)0x00000002)        /*!< Filter Init Mode for filter 1 */
#define  CAN_FM1R_FBM2                       ((uint32_t)0x00000004)        /*!< Filter Init Mode for filter 2 */
#define  CAN_FM1R_FBM3                       ((uint32_t)0x00000008)        /*!< Filter Init Mode for filter 3 */
#define  CAN_FM1R_FBM4                       ((uint32_t)0x00000010)        /*!< Filter Init Mode for filter 4 */
#define  CAN_FM1R_FBM5                       ((uint32_t)0x00000020)        /*!< Filter Init Mode for filter 5 */
#define  CAN_FM1R_FBM6                       ((uint32_t)0x00000040)        /*!< Filter Init Mode for filter 6 */
#define  CAN_FM1R_FBM7                       ((uint32_t)0x00000080)        /*!< Filter Init Mode for filter 7 */
#define  CAN_FM1R_FBM8                       ((uint32_t)0x00000100)        /*!< Filter Init Mode for filter 8 */
#define  CAN_FM1R_FBM9                       ((uint32_t)0x00000200)        /*!< Filter Init Mode for filter 9 */
#define  CAN_FM1R_FBM10                      ((uint32_t)0x00000400)        /*!< Filter Init Mode for filter 10 */
#define  CAN_FM1R_FBM11                      ((uint32_t)0x00000800)        /*!< Filter Init Mode for filter 11 */
#define  CAN_FM1R_FBM12                      ((uint32_t)0x00001000)        /*!< Filter Init Mode for filter 12 */
#define  CAN_FM1R_FBM13                      ((uint32_t)0x00002000)        /*!< Filter Init Mode for filter 13 */
#define  CAN_FM1R_FBM14                      ((uint32_t)0x00004000)        /*!< Filter Init Mode for filter 14 */
#define  CAN_FM1R_FBM15                      ((uint32_t)0x00008000)        /*!< Filter Init Mode for filter 15 */
#define  CAN_FM1R_FBM16                      ((uint32_t)0x00010000)        /*!< Filter Init Mode for filter 16 */
#define  CAN_FM1R_FBM17                      ((uint32_t)0x00020000)        /*!< Filter Init Mode for filter 17 */
#define  CAN_FM1R_FBM18                      ((uint32_t)0x00040000)        /*!< Filter Init Mode for filter 18 */
#define  CAN_FM1R_FBM19                      ((uint32_t)0x00080000)        /*!< Filter Init Mode for filter 19 */
#define  CAN_FM1R_FBM20                      ((uint32_t)0x00100000)        /*!< Filter Init Mode for filter 20 */
#define  CAN_FM1R_FBM21                      ((uint32_t)0x00200000)        /*!< Filter Init Mode for filter 21 */
#define  CAN_FM1R_FBM22                      ((uint32_t)0x00400000)        /*!< Filter Init Mode for filter 22 */
#define  CAN_FM1R_FBM23                      ((uint32_t)0x00800000)        /*!< Filter Init Mode for filter 23 */
#define  CAN_FM1R_FBM24                      ((uint32_t)0x01000000)        /*!< Filter Init Mode for filter 24 */
#define  CAN_FM1R_FBM25                      ((uint32_t)0x02000000)        /*!< Filter Init Mode for filter 25 */
#define  CAN_FM1R_FBM26                      ((uint32_t)0x04000000)        /*!< Filter Init Mode for filter 26 */
#define  CAN_FM1R_FBM27                      ((uint32_t)0x08000000)        /*!< Filter Init Mode for filter 27 */

/*******************  Bit definition for CAN_FS1R register  *******************/
#define  CAN_FS1R_FSC                        ((uint32_t)0x00003FFF)        /*!< Filter Scale Configuration */
#define  CAN_FS1R_FSC0                       ((uint32_t)0x00000001)        /*!< Filter Scale Configuration for filter 0 */
#define  CAN_FS1R_FSC1                       ((uint32_t)0x00000002)        /*!< Filter Scale Configuration for filter 1 */
#define  CAN_FS1R_FSC2                       ((uint32_t)0x00000004)        /*!< Filter Scale Configuration for filter 2 */
#define  CAN_FS1R_FSC3                       ((uint32_t)0x00000008)        /*!< Filter Scale Configuration for filter 3 */
#define  CAN_FS1R_FSC4                       ((uint32_t)0x00000010)        /*!< Filter Scale Configuration for filter 4 */
#define  CAN_FS1R_FSC5                       ((uint32_t)0x00000020)        /*!< Filter Scale Configuration for filter 5 */
#define  CAN_FS1R_FSC6                       ((uint32_t)0x00000040)        /*!< Filter Scale Configuration for filter 6 */
#define  CAN_FS1R_FSC7                       ((uint32_t)0x00000080)        /*!< Filter Scale Configuration for filter 7 */
#define  CAN_FS1R_FSC8                       ((uint32_t)0x00000100)        /*!< Filter Scale Configuration for filter 8 */
#define  CAN_FS1R_FSC9                       ((uint32_t)0x00000200)        /*!< Filter Scale Configuration for filter 9 */
#define  CAN_FS1R_FSC10                      ((uint32_t)0x00000400)        /*!< Filter Scale Configuration for filter 10 */
#define  CAN_FS1R_FSC11                      ((uint32_t)0x00000800)        /*!< Filter Scale Configuration for filter 11 */
#define  CAN_FS1R_FSC12                      ((uint32_t)0x00001000)        /*!< Filter Scale Configuration for filter 12 */
#define  CAN_FS1R_FSC13                      ((uint32_t)0x00002000)        /*!< Filter Scale Configuration for filter 13 */
#define  CAN_FS1R_FSC14                      ((uint32_t)0x00004000)        /*!< Filter Scale Configuration for filter 14 */
#define  CAN_FS1R_FSC15                      ((uint32_t)0x00008000)        /*!< Filter Scale Configuration for filter 15 */
#define  CAN_FS1R_FSC16                      ((uint32_t)0x00010000)        /*!< Filter Scale Configuration for filter 16 */
#define  CAN_FS1R_FSC17                      ((uint32_t)0x00020000)        /*!< Filter Scale Configuration for filter 17 */
#define  CAN_FS1R_FSC18                      ((uint32_t)0x00040000)        /*!< Filter Scale Configuration for filter 18 */
#define  CAN_FS1R_FSC19                      ((uint32_t)0x00080000)        /*!< Filter Scale Configuration for filter 19 */
#define  CAN_FS1R_FSC20                      ((uint32_t)0x00100000)        /*!< Filter Scale Configuration for filter 20 */
#define  CAN_FS1R_FSC21                      ((uint32_t)0x00200000)        /*!< Filter Scale Configuration for filter 21 */
#define  CAN_FS1R_FSC22                      ((uint32_t)0x00400000)        /*!< Filter Scale Configuration for filter 22 */
#define  CAN_FS1R_FSC23                      ((uint32_t)0x00800000)        /*!< Filter Scale Configuration for filter 23 */
#define  CAN_FS1R_FSC24                      ((uint32_t)0x01000000)        /*!< Filter Scale Configuration for filter 24 */
#define  CAN_FS1R_FSC25                      ((uint32_t)0x02000000)        /*!< Filter Scale Configuration for filter 25 */
#define  CAN_FS1R_FSC26                      ((uint32_t)0x04000000)        /*!< Filter Scale Configuration for filter 26 */
#define  CAN_FS1R_FSC27                      ((uint32_t)0x08000000)        /*!< Filter Scale Configuration for filter 27 */

/******************  Bit definition for CAN_FFA1R register  *******************/
#define  CAN_FFA1R_FFA                       ((uint32_t)0x00003FFF)        /*!< Filter FIFO Assignment */
#define  CAN_FFA1R_FFA0                      ((uint32_t)0x00000001)        /*!< Filter FIFO Assignment for filter 0 */
#define  CAN_FFA1R_FFA1                      ((uint32_t)0x00000002)        /*!< Filter FIFO Assignment for filter 1 */
#define  CAN_FFA1R_FFA2                      ((uint32_t)0x00000004)        /*!< Filter FIFO Assignment for filter 2 */
#define  CAN_FFA1R_FFA3                      ((uint32_t)0x00000008)        /*!< Filter FIFO Assignment for filter 3 */
#define  CAN_FFA1R_FFA4                      ((uint32_t)0x00000010)        /*!< Filter FIFO Assignment for filter 4 */
#define  CAN_FFA1R_FFA5                      ((uint32_t)0x00000020)        /*!< Filter FIFO Assignment for filter 5 */
#define  CAN_FFA1R_FFA6                      ((uint32_t)0x00000040)        /*!< Filter FIFO Assignment for filter 6 */
#define  CAN_FFA1R_FFA7                      ((uint32_t)0x00000080)        /*!< Filter FIFO Assignment for filter 7 */
#define  CAN_FFA1R_FFA8                      ((uint32_t)0x00000100)        /*!< Filter FIFO Assignment for filter 8 */
#define  CAN_FFA1R_FFA9                      ((uint32_t)0x00000200)        /*!< Filter FIFO Assignment for filter 9 */
#define  CAN_FFA1R_FFA10                     ((uint32_t)0x00000400)        /*!< Filter FIFO Assignment for filter 10 */
#define  CAN_FFA1R_FFA11                     ((uint32_t)0x00000800)        /*!< Filter FIFO Assignment for filter 11 */
#define  CAN_FFA1R_FFA12                     ((uint32_t)0x00001000)        /*!< Filter FIFO Assignment for filter 12 */
#define  CAN_FFA1R_FFA13                     ((uint32_t)0x00002000)        /*!< Filter FIFO Assignment for filter 13 */
#define  CAN_FFA1_FFA14                      ((uint32_t)0x00004000)        /*!< Filter FIFO Assignment for filter 14 */
#define  CAN_FFA1_FFA15                      ((uint32_t)0x00008000)        /*!< Filter FIFO Assignment for filter 15 */
#define  CAN_FFA1_FFA16                      ((uint32_t)0x00010000)        /*!< Filter FIFO Assignment for filter 16 */
#define  CAN_FFA1_FFA17                      ((uint32_t)0x00020000)        /*!< Filter FIFO Assignment for filter 17 */
#define  CAN_FFA1_FFA18                      ((uint32_t)0x00040000)        /*!< Filter FIFO Assignment for filter 18 */
#define  CAN_FFA1_FFA19                      ((uint32_t)0x00080000)        /*!< Filter FIFO Assignment for filter 19 */
#define  CAN_FFA1_FFA20                      ((uint32_t)0x00100000)        /*!< Filter FIFO Assignment for filter 20 */
#define  CAN_FFA1_FFA21                      ((uint32_t)0x00200000)        /*!< Filter FIFO Assignment for filter 21 */
#define  CAN_FFA1_FFA22                      ((uint32_t)0x00400000)        /*!< Filter FIFO Assignment for filter 22 */
#define  CAN_FFA1_FFA23                      ((uint32_t)0x00800000)        /*!< Filter FIFO Assignment for filter 23 */
#define  CAN_FFA1_FFA24                      ((uint32_t)0x01000000)        /*!< Filter FIFO Assignment for filter 24 */
#define  CAN_FFA1_FFA25                      ((uint32_t)0x02000000)        /*!< Filter FIFO Assignment for filter 25 */
#define  CAN_FFA1_FFA26                      ((uint32_t)0x04000000)        /*!< Filter FIFO Assignment for filter 26 */
#define  CAN_FFA1_FFA27                      ((uint32_t)0x08000000)        /*!< Filter FIFO Assignment for filter 27 */

/*******************  Bit definition for CAN_FA1R register  *******************/
#define  CAN_FA1R_FACT                       ((uint32_t)0x00003FFF)        /*!< Filter Active */
#define  CAN_FA1R_FACT0                      ((uint32_t)0x00000001)        /*!< Filter 0 Active */
#define  CAN_FA1R_FACT1                      ((uint32_t)0x00000002)        /*!< Filter 1 Active */
#define  CAN_FA1R_FACT2                      ((uint32_t)0x00000004)        /*!< Filter 2 Active */
#define  CAN_FA1R_FACT3                      ((uint32_t)0x00000008)        /*!< Filter 3 Active */
#define  CAN_FA1R_FACT4                      ((uint32_t)0x00000010)        /*!< Filter 4 Active */
#define  CAN_FA1R_FACT5                      ((uint32_t)0x00000020)        /*!< Filter 5 Active */
#define  CAN_FA1R_FACT6                      ((uint32_t)0x00000040)        /*!< Filter 6 Active */
#define  CAN_FA1R_FACT7                      ((uint32_t)0x00000080)        /*!< Filter 7 Active */
#define  CAN_FA1R_FACT8                      ((uint32_t)0x00000100)        /*!< Filter 8 Active */
#define  CAN_FA1R_FACT9                      ((uint32_t)0x00000200)        /*!< Filter 9 Active */
#define  CAN_FA1R_FACT10                     ((uint32_t)0x00000400)        /*!< Filter 10 Active */
#define  CAN_FA1R_FACT11                     ((uint32_t)0x00000800)        /*!< Filter 11 Active */
#define  CAN_FA1R_FACT12                     ((uint32_t)0x00001000)        /*!< Filter 12 Active */
#define  CAN_FA1R_FACT13                     ((uint32_t)0x00002000)        /*!< Filter 13 Active */
#define  CAN_FA1R_FACT14                      ((uint32_t)0x00004000)        /*!< Filter 14 Active */
#define  CAN_FA1R_FACT15                      ((uint32_t)0x00008000)        /*!< Filter 15 Active */
#define  CAN_FA1R_FACT16                      ((uint32_t)0x00010000)        /*!< Filter 16 Active */
#define  CAN_FA1R_FACT17                      ((uint32_t)0x00020000)        /*!< Filter 17 Active */
#define  CAN_FA1R_FACT18                      ((uint32_t)0x00040000)        /*!< Filter 18 Active */
#define  CAN_FA1R_FACT19                      ((uint32_t)0x00080000)        /*!< Filter 19 Active */
#define  CAN_FA1R_FACT20                      ((uint32_t)0x00100000)        /*!< Filter 20 Active */
#define  CAN_FA1R_FACT21                      ((uint32_t)0x00200000)        /*!< Filter 21 Active */
#define  CAN_FA1R_FACT22                      ((uint32_t)0x00400000)        /*!< Filter 22 Active */
#define  CAN_FA1R_FACT23                      ((uint32_t)0x00800000)        /*!< Filter 23 Active */
#define  CAN_FA1R_FACT24                      ((uint32_t)0x01000000)        /*!< Filter 24 Active */
#define  CAN_FA1R_FACT25                      ((uint32_t)0x02000000)        /*!< Filter 25 Active */
#define  CAN_FA1R_FACT26                      ((uint32_t)0x04000000)        /*!< Filter 26 Active */
#define  CAN_FA1R_FACT27                      ((uint32_t)0x08000000)        /*!< Filter 27 Active */

/*******************  Bit definition for CAN_F0R1 register  *******************/
#define  CAN_F0R1_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F0R1_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F0R1_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F0R1_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F0R1_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F0R1_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F0R1_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F0R1_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F0R1_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F0R1_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F0R1_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F0R1_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F0R1_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F0R1_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F0R1_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F0R1_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F0R1_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F0R1_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F0R1_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F0R1_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F0R1_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F0R1_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F0R1_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F0R1_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F0R1_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F0R1_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F0R1_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F0R1_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F0R1_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F0R1_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F0R1_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F0R1_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F1R1 register  *******************/
#define  CAN_F1R1_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F1R1_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F1R1_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F1R1_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F1R1_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F1R1_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F1R1_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F1R1_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F1R1_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F1R1_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F1R1_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F1R1_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F1R1_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F1R1_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F1R1_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F1R1_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F1R1_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F1R1_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F1R1_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F1R1_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F1R1_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F1R1_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F1R1_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F1R1_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F1R1_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F1R1_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F1R1_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F1R1_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F1R1_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F1R1_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F1R1_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F1R1_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F2R1 register  *******************/
#define  CAN_F2R1_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F2R1_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F2R1_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F2R1_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F2R1_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F2R1_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F2R1_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F2R1_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F2R1_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F2R1_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F2R1_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F2R1_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F2R1_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F2R1_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F2R1_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F2R1_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F2R1_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F2R1_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F2R1_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F2R1_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F2R1_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F2R1_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F2R1_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F2R1_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F2R1_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F2R1_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F2R1_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F2R1_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F2R1_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F2R1_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F2R1_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F2R1_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F3R1 register  *******************/
#define  CAN_F3R1_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F3R1_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F3R1_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F3R1_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F3R1_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F3R1_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F3R1_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F3R1_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F3R1_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F3R1_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F3R1_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F3R1_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F3R1_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F3R1_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F3R1_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F3R1_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F3R1_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F3R1_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F3R1_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F3R1_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F3R1_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F3R1_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F3R1_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F3R1_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F3R1_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F3R1_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F3R1_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F3R1_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F3R1_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F3R1_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F3R1_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F3R1_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F4R1 register  *******************/
#define  CAN_F4R1_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F4R1_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F4R1_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F4R1_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F4R1_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F4R1_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F4R1_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F4R1_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F4R1_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F4R1_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F4R1_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F4R1_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F4R1_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F4R1_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F4R1_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F4R1_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F4R1_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F4R1_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F4R1_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F4R1_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F4R1_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F4R1_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F4R1_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F4R1_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F4R1_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F4R1_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F4R1_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F4R1_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F4R1_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F4R1_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F4R1_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F4R1_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F5R1 register  *******************/
#define  CAN_F5R1_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F5R1_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F5R1_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F5R1_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F5R1_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F5R1_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F5R1_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F5R1_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F5R1_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F5R1_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F5R1_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F5R1_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F5R1_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F5R1_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F5R1_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F5R1_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F5R1_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F5R1_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F5R1_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F5R1_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F5R1_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F5R1_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F5R1_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F5R1_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F5R1_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F5R1_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F5R1_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F5R1_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F5R1_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F5R1_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F5R1_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F5R1_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F6R1 register  *******************/
#define  CAN_F6R1_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F6R1_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F6R1_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F6R1_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F6R1_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F6R1_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F6R1_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F6R1_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F6R1_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F6R1_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F6R1_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F6R1_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F6R1_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F6R1_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F6R1_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F6R1_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F6R1_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F6R1_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F6R1_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F6R1_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F6R1_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F6R1_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F6R1_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F6R1_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F6R1_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F6R1_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F6R1_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F6R1_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F6R1_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F6R1_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F6R1_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F6R1_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F7R1 register  *******************/
#define  CAN_F7R1_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F7R1_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F7R1_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F7R1_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F7R1_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F7R1_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F7R1_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F7R1_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F7R1_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F7R1_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F7R1_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F7R1_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F7R1_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F7R1_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F7R1_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F7R1_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F7R1_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F7R1_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F7R1_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F7R1_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F7R1_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F7R1_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F7R1_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F7R1_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F7R1_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F7R1_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F7R1_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F7R1_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F7R1_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F7R1_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F7R1_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F7R1_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F8R1 register  *******************/
#define  CAN_F8R1_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F8R1_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F8R1_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F8R1_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F8R1_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F8R1_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F8R1_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F8R1_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F8R1_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F8R1_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F8R1_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F8R1_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F8R1_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F8R1_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F8R1_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F8R1_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F8R1_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F8R1_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F8R1_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F8R1_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F8R1_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F8R1_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F8R1_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F8R1_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F8R1_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F8R1_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F8R1_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F8R1_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F8R1_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F8R1_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F8R1_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F8R1_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F9R1 register  *******************/
#define  CAN_F9R1_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F9R1_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F9R1_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F9R1_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F9R1_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F9R1_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F9R1_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F9R1_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F9R1_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F9R1_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F9R1_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F9R1_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F9R1_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F9R1_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F9R1_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F9R1_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F9R1_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F9R1_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F9R1_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F9R1_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F9R1_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F9R1_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F9R1_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F9R1_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F9R1_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F9R1_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F9R1_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F9R1_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F9R1_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F9R1_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F9R1_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F9R1_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F10R1 register  ******************/
#define  CAN_F10R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F10R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F10R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F10R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F10R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F10R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F10R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F10R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F10R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F10R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F10R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F10R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F10R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F10R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F10R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F10R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F10R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F10R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F10R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F10R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F10R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F10R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F10R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F10R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F10R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F10R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F10R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F10R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F10R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F10R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F10R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F10R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F11R1 register  ******************/
#define  CAN_F11R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F11R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F11R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F11R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F11R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F11R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F11R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F11R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F11R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F11R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F11R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F11R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F11R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F11R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F11R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F11R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F11R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F11R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F11R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F11R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F11R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F11R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F11R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F11R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F11R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F11R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F11R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F11R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F11R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F11R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F11R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F11R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F12R1 register  ******************/
#define  CAN_F12R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F12R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F12R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F12R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F12R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F12R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F12R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F12R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F12R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F12R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F12R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F12R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F12R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F12R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F12R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F12R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F12R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F12R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F12R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F12R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F12R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F12R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F12R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F12R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F12R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F12R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F12R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F12R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F12R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F12R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F12R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F12R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F13R1 register  ******************/
#define  CAN_F13R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F13R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F13R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F13R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F13R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F13R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F13R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F13R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F13R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F13R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F13R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F13R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F13R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F13R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F13R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F13R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F13R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F13R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F13R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F13R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F13R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F13R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F13R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F13R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F13R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F13R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F13R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F13R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F13R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F13R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F13R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F13R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F14R1 register  ******************/
#define  CAN_F14R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F14R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F14R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F14R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F14R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F14R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F14R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F14R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F14R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F14R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F14R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F14R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F14R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F14R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F14R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F14R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F14R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F14R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F14R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F14R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F14R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F14R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F14R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F14R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F14R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F14R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F14R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F14R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F14R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F14R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F14R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F14R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F15R1 register  ******************/
#define  CAN_F15R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F15R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F15R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F15R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F15R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F15R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F15R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F15R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F15R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F15R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F15R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F15R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F15R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F15R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F15R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F15R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F15R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F15R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F15R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F15R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F15R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F15R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F15R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F15R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F15R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F15R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F15R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F15R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F15R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F15R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F15R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F15R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F16R1 register  ******************/
#define  CAN_F16R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F16R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F16R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F16R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F16R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F16R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F16R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F16R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F16R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F16R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F16R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F16R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F16R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F16R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F16R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F16R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F16R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F16R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F16R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F16R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F16R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F16R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F16R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F16R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F16R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F16R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F16R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F16R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F16R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F16R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F16R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F16R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F17R1 register  ******************/
#define  CAN_F17R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F17R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F17R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F17R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F17R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F17R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F17R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F17R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F17R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F17R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F17R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F17R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F17R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F17R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F17R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F17R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F17R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F17R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F17R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F17R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F17R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F17R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F17R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F17R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F17R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F17R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F17R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F17R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F17R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F17R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F17R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F17R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F18R1 register  ******************/
#define  CAN_F18R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F18R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F18R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F18R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F18R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F18R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F18R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F18R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F18R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F18R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F18R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F18R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F18R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F18R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F18R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F18R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F18R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F18R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F18R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F18R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F18R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F18R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F18R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F18R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F18R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F18R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F18R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F18R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F18R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F18R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F18R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F18R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F19R1 register  ******************/
#define  CAN_F19R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F19R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F19R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F19R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F19R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F19R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F19R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F19R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F19R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F19R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F19R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F19R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F19R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F19R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F19R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F19R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F19R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F19R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F19R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F19R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F19R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F19R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F19R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F19R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F19R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F19R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F19R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F19R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F19R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F19R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F19R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F19R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F20R1 register  ******************/
#define  CAN_F20R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F20R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F20R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F20R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F20R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F20R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F20R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F20R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F20R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F20R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F20R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F20R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F20R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F20R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F20R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F20R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F20R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F20R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F20R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F20R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F20R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F20R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F20R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F20R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F20R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F20R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F20R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F20R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F20R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F20R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F20R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F20R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F21R1 register  ******************/
#define  CAN_F21R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F21R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F21R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F21R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F21R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F21R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F21R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F21R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F21R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F21R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F21R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F21R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F21R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F21R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F21R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F21R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F21R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F21R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F21R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F21R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F21R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F21R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F21R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F21R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F21R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F21R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F21R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F21R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F21R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F21R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F21R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F21R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F22R1 register  ******************/
#define  CAN_F22R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F22R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F22R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F22R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F22R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F22R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F22R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F22R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F22R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F22R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F22R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F22R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F22R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F22R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F22R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F22R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F22R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F22R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F22R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F22R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F22R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F22R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F22R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F22R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F22R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F22R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F22R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F22R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F22R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F22R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F22R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F22R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F23R1 register  ******************/
#define  CAN_F23R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F23R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F23R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F23R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F23R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F23R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F23R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F23R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F23R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F23R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F23R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F23R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F23R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F23R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F23R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F23R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F23R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F23R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F23R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F23R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F23R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F23R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F23R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F23R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F23R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F23R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F23R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F23R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F23R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F23R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F23R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F23R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F24R1 register  ******************/
#define  CAN_F24R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F24R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F24R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F24R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F24R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F24R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F24R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F24R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F24R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F24R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F24R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F24R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F24R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F24R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F24R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F24R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F24R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F24R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F24R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F24R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F24R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F24R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F24R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F24R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F24R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F24R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F24R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F24R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F24R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F24R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F24R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F24R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F25R1 register  ******************/
#define  CAN_F25R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F25R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F25R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F25R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F25R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F25R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F25R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F25R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F25R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F25R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F25R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F25R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F25R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F25R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F25R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F25R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F25R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F25R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F25R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F25R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F25R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F25R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F25R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F25R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F25R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F25R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F25R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F25R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F25R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F25R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F25R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F25R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F26R1 register  ******************/
#define  CAN_F26R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F26R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F26R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F26R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F26R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F26R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F26R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F26R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F26R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F26R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F26R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F26R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F26R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F26R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F26R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F26R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F26R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F26R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F26R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F26R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F26R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F26R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F26R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F26R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F26R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F26R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F26R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F26R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F26R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F26R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F26R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F26R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F27R1 register  ******************/
#define  CAN_F27R1_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F27R1_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F27R1_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F27R1_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F27R1_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F27R1_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F27R1_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F27R1_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F27R1_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F27R1_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F27R1_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F27R1_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F27R1_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F27R1_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F27R1_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F27R1_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F27R1_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F27R1_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F27R1_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F27R1_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F27R1_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F27R1_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F27R1_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F27R1_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F27R1_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F27R1_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F27R1_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F27R1_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F27R1_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F27R1_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F27R1_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F27R1_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F0R2 register  *******************/
#define  CAN_F0R2_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F0R2_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F0R2_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F0R2_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F0R2_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F0R2_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F0R2_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F0R2_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F0R2_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F0R2_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F0R2_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F0R2_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F0R2_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F0R2_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F0R2_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F0R2_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F0R2_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F0R2_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F0R2_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F0R2_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F0R2_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F0R2_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F0R2_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F0R2_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F0R2_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F0R2_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F0R2_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F0R2_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F0R2_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F0R2_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F0R2_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F0R2_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F1R2 register  *******************/
#define  CAN_F1R2_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F1R2_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F1R2_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F1R2_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F1R2_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F1R2_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F1R2_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F1R2_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F1R2_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F1R2_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F1R2_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F1R2_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F1R2_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F1R2_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F1R2_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F1R2_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F1R2_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F1R2_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F1R2_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F1R2_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F1R2_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F1R2_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F1R2_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F1R2_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F1R2_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F1R2_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F1R2_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F1R2_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F1R2_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F1R2_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F1R2_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F1R2_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F2R2 register  *******************/
#define  CAN_F2R2_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F2R2_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F2R2_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F2R2_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F2R2_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F2R2_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F2R2_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F2R2_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F2R2_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F2R2_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F2R2_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F2R2_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F2R2_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F2R2_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F2R2_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F2R2_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F2R2_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F2R2_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F2R2_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F2R2_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F2R2_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F2R2_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F2R2_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F2R2_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F2R2_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F2R2_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F2R2_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F2R2_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F2R2_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F2R2_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F2R2_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F2R2_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F3R2 register  *******************/
#define  CAN_F3R2_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F3R2_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F3R2_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F3R2_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F3R2_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F3R2_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F3R2_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F3R2_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F3R2_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F3R2_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F3R2_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F3R2_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F3R2_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F3R2_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F3R2_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F3R2_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F3R2_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F3R2_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F3R2_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F3R2_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F3R2_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F3R2_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F3R2_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F3R2_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F3R2_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F3R2_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F3R2_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F3R2_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F3R2_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F3R2_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F3R2_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F3R2_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F4R2 register  *******************/
#define  CAN_F4R2_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F4R2_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F4R2_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F4R2_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F4R2_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F4R2_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F4R2_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F4R2_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F4R2_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F4R2_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F4R2_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F4R2_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F4R2_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F4R2_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F4R2_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F4R2_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F4R2_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F4R2_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F4R2_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F4R2_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F4R2_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F4R2_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F4R2_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F4R2_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F4R2_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F4R2_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F4R2_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F4R2_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F4R2_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F4R2_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F4R2_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F4R2_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F5R2 register  *******************/
#define  CAN_F5R2_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F5R2_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F5R2_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F5R2_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F5R2_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F5R2_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F5R2_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F5R2_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F5R2_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F5R2_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F5R2_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F5R2_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F5R2_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F5R2_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F5R2_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F5R2_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F5R2_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F5R2_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F5R2_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F5R2_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F5R2_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F5R2_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F5R2_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F5R2_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F5R2_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F5R2_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F5R2_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F5R2_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F5R2_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F5R2_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F5R2_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F5R2_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F6R2 register  *******************/
#define  CAN_F6R2_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F6R2_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F6R2_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F6R2_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F6R2_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F6R2_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F6R2_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F6R2_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F6R2_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F6R2_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F6R2_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F6R2_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F6R2_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F6R2_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F6R2_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F6R2_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F6R2_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F6R2_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F6R2_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F6R2_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F6R2_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F6R2_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F6R2_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F6R2_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F6R2_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F6R2_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F6R2_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F6R2_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F6R2_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F6R2_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F6R2_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F6R2_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F7R2 register  *******************/
#define  CAN_F7R2_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F7R2_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F7R2_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F7R2_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F7R2_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F7R2_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F7R2_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F7R2_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F7R2_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F7R2_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F7R2_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F7R2_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F7R2_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F7R2_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F7R2_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F7R2_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F7R2_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F7R2_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F7R2_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F7R2_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F7R2_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F7R2_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F7R2_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F7R2_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F7R2_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F7R2_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F7R2_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F7R2_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F7R2_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F7R2_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F7R2_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F7R2_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F8R2 register  *******************/
#define  CAN_F8R2_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F8R2_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F8R2_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F8R2_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F8R2_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F8R2_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F8R2_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F8R2_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F8R2_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F8R2_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F8R2_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F8R2_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F8R2_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F8R2_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F8R2_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F8R2_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F8R2_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F8R2_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F8R2_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F8R2_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F8R2_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F8R2_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F8R2_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F8R2_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F8R2_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F8R2_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F8R2_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F8R2_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F8R2_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F8R2_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F8R2_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F8R2_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F9R2 register  *******************/
#define  CAN_F9R2_FB0                        ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F9R2_FB1                        ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F9R2_FB2                        ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F9R2_FB3                        ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F9R2_FB4                        ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F9R2_FB5                        ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F9R2_FB6                        ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F9R2_FB7                        ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F9R2_FB8                        ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F9R2_FB9                        ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F9R2_FB10                       ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F9R2_FB11                       ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F9R2_FB12                       ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F9R2_FB13                       ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F9R2_FB14                       ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F9R2_FB15                       ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F9R2_FB16                       ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F9R2_FB17                       ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F9R2_FB18                       ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F9R2_FB19                       ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F9R2_FB20                       ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F9R2_FB21                       ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F9R2_FB22                       ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F9R2_FB23                       ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F9R2_FB24                       ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F9R2_FB25                       ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F9R2_FB26                       ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F9R2_FB27                       ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F9R2_FB28                       ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F9R2_FB29                       ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F9R2_FB30                       ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F9R2_FB31                       ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F10R2 register  ******************/
#define  CAN_F10R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F10R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F10R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F10R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F10R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F10R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F10R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F10R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F10R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F10R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F10R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F10R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F10R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F10R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F10R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F10R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F10R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F10R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F10R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F10R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F10R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F10R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F10R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F10R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F10R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F10R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F10R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F10R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F10R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F10R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F10R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F10R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F11R2 register  ******************/
#define  CAN_F11R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F11R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F11R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F11R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F11R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F11R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F11R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F11R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F11R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F11R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F11R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F11R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F11R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F11R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F11R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F11R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F11R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F11R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F11R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F11R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F11R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F11R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F11R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F11R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F11R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F11R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F11R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F11R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F11R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F11R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F11R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F11R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F12R2 register  ******************/
#define  CAN_F12R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F12R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F12R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F12R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F12R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F12R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F12R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F12R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F12R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F12R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F12R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F12R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F12R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F12R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F12R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F12R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F12R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F12R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F12R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F12R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F12R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F12R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F12R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F12R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F12R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F12R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F12R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F12R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F12R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F12R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F12R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F12R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F13R2 register  ******************/
#define  CAN_F13R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F13R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F13R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F13R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F13R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F13R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F13R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F13R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F13R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F13R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F13R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F13R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F13R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F13R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F13R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F13R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F13R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F13R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F13R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F13R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F13R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F13R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F13R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F13R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F13R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F13R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F13R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F13R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F13R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F13R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F13R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F13R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F14R2 register  ******************/
#define  CAN_F14R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F14R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F14R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F14R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F14R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F14R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F14R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F14R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F14R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F14R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F14R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F14R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F14R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F14R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F14R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F14R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F14R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F14R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F14R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F14R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F14R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F14R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F14R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F14R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F14R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F14R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F14R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F14R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F14R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F14R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F14R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F14R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F15R2 register  ******************/
#define  CAN_F15R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F15R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F15R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F15R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F15R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F15R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F15R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F15R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F15R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F15R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F15R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F15R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F15R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F15R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F15R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F15R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F15R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F15R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F15R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F15R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F15R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F15R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F15R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F15R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F15R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F15R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F15R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F15R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F15R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F15R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F15R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F15R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F16R2 register  ******************/
#define  CAN_F16R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F16R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F16R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F16R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F16R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F16R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F16R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F16R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F16R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F16R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F16R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F16R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F16R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F16R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F16R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F16R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F16R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F16R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F16R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F16R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F16R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F16R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F16R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F16R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F16R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F16R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F16R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F16R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F16R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F16R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F16R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F16R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F17R2 register  ******************/
#define  CAN_F17R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F17R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F17R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F17R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F17R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F17R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F17R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F17R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F17R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F17R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F17R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F17R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F17R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F17R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F17R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F17R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F17R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F17R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F17R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F17R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F17R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F17R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F17R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F17R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F17R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F17R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F17R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F17R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F17R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F17R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F17R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F17R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F18R2 register  ******************/
#define  CAN_F18R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F18R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F18R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F18R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F18R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F18R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F18R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F18R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F18R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F18R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F18R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F18R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F18R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F18R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F18R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F18R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F18R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F18R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F18R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F18R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F18R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F18R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F18R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F18R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F18R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F18R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F18R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F18R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F18R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F18R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F18R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F18R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F19R2 register  ******************/
#define  CAN_F19R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F19R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F19R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F19R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F19R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F19R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F19R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F19R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F19R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F19R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F19R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F19R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F19R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F19R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F19R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F19R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F19R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F19R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F19R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F19R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F19R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F19R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F19R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F19R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F19R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F19R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F19R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F19R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F19R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F19R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F19R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F19R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F20R2 register  ******************/
#define  CAN_F20R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F20R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F20R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F20R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F20R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F20R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F20R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F20R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F20R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F20R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F20R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F20R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F20R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F20R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F20R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F20R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F20R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F20R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F20R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F20R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F20R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F20R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F20R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F20R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F20R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F20R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F20R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F20R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F20R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F20R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F20R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F20R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F21R2 register  ******************/
#define  CAN_F21R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F21R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F21R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F21R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F21R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F21R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F21R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F21R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F21R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F21R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F21R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F21R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F21R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F21R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F21R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F21R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F21R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F21R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F21R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F21R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F21R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F21R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F21R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F21R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F21R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F21R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F21R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F21R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F21R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F21R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F21R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F21R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F22R2 register  ******************/
#define  CAN_F22R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F22R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F22R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F22R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F22R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F22R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F22R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F22R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F22R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F22R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F22R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F22R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F22R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F22R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F22R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F22R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F22R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F22R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F22R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F22R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F22R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F22R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F22R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F22R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F22R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F22R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F22R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F22R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F22R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F22R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F22R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F22R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F23R2 register  ******************/
#define  CAN_F23R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F23R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F23R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F23R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F23R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F23R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F23R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F23R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F23R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F23R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F23R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F23R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F23R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F23R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F23R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F23R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F23R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F23R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F23R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F23R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F23R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F23R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F23R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F23R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F23R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F23R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F23R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F23R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F23R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F23R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F23R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F23R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F24R2 register  ******************/
#define  CAN_F24R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F24R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F24R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F24R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F24R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F24R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F24R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F24R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F24R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F24R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F24R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F24R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F24R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F24R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F24R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F24R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F24R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F24R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F24R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F24R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F24R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F24R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F24R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F24R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F24R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F24R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F24R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F24R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F24R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F24R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F24R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F24R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F25R2 register  ******************/
#define  CAN_F25R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F25R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F25R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F25R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F25R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F25R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F25R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F25R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F25R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F25R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F25R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F25R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F25R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F25R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F25R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F25R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F25R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F25R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F25R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F25R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F25R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F25R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F25R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F25R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F25R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F25R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F25R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F25R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F25R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F25R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F25R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F25R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F26R2 register  ******************/
#define  CAN_F26R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F26R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F26R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F26R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F26R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F26R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F26R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F26R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F26R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F26R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F26R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F26R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F26R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F26R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F26R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F26R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F26R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F26R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F26R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F26R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F26R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F26R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F26R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F26R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F26R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F26R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F26R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F26R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F26R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F26R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F26R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F26R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/*******************  Bit definition for CAN_F27R2 register  ******************/
#define  CAN_F27R2_FB0                       ((uint32_t)0x00000001)        /*!< Filter bit 0 */
#define  CAN_F27R2_FB1                       ((uint32_t)0x00000002)        /*!< Filter bit 1 */
#define  CAN_F27R2_FB2                       ((uint32_t)0x00000004)        /*!< Filter bit 2 */
#define  CAN_F27R2_FB3                       ((uint32_t)0x00000008)        /*!< Filter bit 3 */
#define  CAN_F27R2_FB4                       ((uint32_t)0x00000010)        /*!< Filter bit 4 */
#define  CAN_F27R2_FB5                       ((uint32_t)0x00000020)        /*!< Filter bit 5 */
#define  CAN_F27R2_FB6                       ((uint32_t)0x00000040)        /*!< Filter bit 6 */
#define  CAN_F27R2_FB7                       ((uint32_t)0x00000080)        /*!< Filter bit 7 */
#define  CAN_F27R2_FB8                       ((uint32_t)0x00000100)        /*!< Filter bit 8 */
#define  CAN_F27R2_FB9                       ((uint32_t)0x00000200)        /*!< Filter bit 9 */
#define  CAN_F27R2_FB10                      ((uint32_t)0x00000400)        /*!< Filter bit 10 */
#define  CAN_F27R2_FB11                      ((uint32_t)0x00000800)        /*!< Filter bit 11 */
#define  CAN_F27R2_FB12                      ((uint32_t)0x00001000)        /*!< Filter bit 12 */
#define  CAN_F27R2_FB13                      ((uint32_t)0x00002000)        /*!< Filter bit 13 */
#define  CAN_F27R2_FB14                      ((uint32_t)0x00004000)        /*!< Filter bit 14 */
#define  CAN_F27R2_FB15                      ((uint32_t)0x00008000)        /*!< Filter bit 15 */
#define  CAN_F27R2_FB16                      ((uint32_t)0x00010000)        /*!< Filter bit 16 */
#define  CAN_F27R2_FB17                      ((uint32_t)0x00020000)        /*!< Filter bit 17 */
#define  CAN_F27R2_FB18                      ((uint32_t)0x00040000)        /*!< Filter bit 18 */
#define  CAN_F27R2_FB19                      ((uint32_t)0x00080000)        /*!< Filter bit 19 */
#define  CAN_F27R2_FB20                      ((uint32_t)0x00100000)        /*!< Filter bit 20 */
#define  CAN_F27R2_FB21                      ((uint32_t)0x00200000)        /*!< Filter bit 21 */
#define  CAN_F27R2_FB22                      ((uint32_t)0x00400000)        /*!< Filter bit 22 */
#define  CAN_F27R2_FB23                      ((uint32_t)0x00800000)        /*!< Filter bit 23 */
#define  CAN_F27R2_FB24                      ((uint32_t)0x01000000)        /*!< Filter bit 24 */
#define  CAN_F27R2_FB25                      ((uint32_t)0x02000000)        /*!< Filter bit 25 */
#define  CAN_F27R2_FB26                      ((uint32_t)0x04000000)        /*!< Filter bit 26 */
#define  CAN_F27R2_FB27                      ((uint32_t)0x08000000)        /*!< Filter bit 27 */
#define  CAN_F27R2_FB28                      ((uint32_t)0x10000000)        /*!< Filter bit 28 */
#define  CAN_F27R2_FB29                      ((uint32_t)0x20000000)        /*!< Filter bit 29 */
#define  CAN_F27R2_FB30                      ((uint32_t)0x40000000)        /*!< Filter bit 30 */
#define  CAN_F27R2_FB31                      ((uint32_t)0x80000000)        /*!< Filter bit 31 */

/******************************************************************************/
/*                                                                            */
/*                        Serial Peripheral Interface                         */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for SPI_CR1 register  ********************/
#define  SPI_CR1_CPHA                        ((uint32_t)0x00000001)        /*!< Clock Phase */
#define  SPI_CR1_CPOL                        ((uint32_t)0x00000002)        /*!< Clock Polarity */
#define  SPI_CR1_MSTR                        ((uint32_t)0x00000004)        /*!< Master Selection */

#define  SPI_CR1_BR                          ((uint32_t)0x00000038)        /*!< BR[2:0] bits (Baud Rate Control) */
#define  SPI_CR1_BR_0                        ((uint32_t)0x00000008)        /*!< Bit 0 */
#define  SPI_CR1_BR_1                        ((uint32_t)0x00000010)        /*!< Bit 1 */
#define  SPI_CR1_BR_2                        ((uint32_t)0x00000020)        /*!< Bit 2 */

#define  SPI_CR1_SPE                         ((uint32_t)0x00000040)        /*!< SPI Enable */
#define  SPI_CR1_LSBFIRST                    ((uint32_t)0x00000080)        /*!< Frame Format */
#define  SPI_CR1_SSI                         ((uint32_t)0x00000100)        /*!< Internal slave select */
#define  SPI_CR1_SSM                         ((uint32_t)0x00000200)        /*!< Software slave management */
#define  SPI_CR1_RXONLY                      ((uint32_t)0x00000400)        /*!< Receive only */
#define  SPI_CR1_DFF                         ((uint32_t)0x00000800)        /*!< Data Frame Format */
#define  SPI_CR1_CRCNEXT                     ((uint32_t)0x00001000)        /*!< Transmit CRC next */
#define  SPI_CR1_CRCEN                       ((uint32_t)0x00002000)        /*!< Hardware CRC calculation enable */
#define  SPI_CR1_BIDIOE                      ((uint32_t)0x00004000)        /*!< Output enable in bidirectional mode */
#define  SPI_CR1_BIDIMODE                    ((uint32_t)0x00008000)        /*!< Bidirectional data mode enable */

/*******************  Bit definition for SPI_CR2 register  ********************/
#define  SPI_CR2_RXDMAEN                     ((uint32_t)0x00000001)        /*!< Rx Buffer DMA Enable */
#define  SPI_CR2_TXDMAEN                     ((uint32_t)0x00000002)        /*!< Tx Buffer DMA Enable */
#define  SPI_CR2_SSOE                        ((uint32_t)0x00000004)        /*!< SS Output Enable */
#define  SPI_CR2_ERRIE                       ((uint32_t)0x00000020)        /*!< Error Interrupt Enable */
#define  SPI_CR2_RXNEIE                      ((uint32_t)0x00000040)        /*!< RX buffer Not Empty Interrupt Enable */
#define  SPI_CR2_TXEIE                       ((uint32_t)0x00000080)        /*!< Tx buffer Empty Interrupt Enable */

/********************  Bit definition for SPI_SR register  ********************/
#define  SPI_SR_RXNE                         ((uint32_t)0x00000001)        /*!< Receive buffer Not Empty */
#define  SPI_SR_TXE                          ((uint32_t)0x00000002)        /*!< Transmit buffer Empty */
#define  SPI_SR_CHSIDE                       ((uint32_t)0x00000004)        /*!< Channel side */
#define  SPI_SR_UDR                          ((uint32_t)0x00000008)        /*!< Underrun flag */
#define  SPI_SR_CRCERR                       ((uint32_t)0x00000010)        /*!< CRC Error flag */
#define  SPI_SR_MODF                         ((uint32_t)0x00000020)        /*!< Mode fault */
#define  SPI_SR_OVR                          ((uint32_t)0x00000040)        /*!< Overrun flag */
#define  SPI_SR_BSY                          ((uint32_t)0x00000080)        /*!< Busy flag */

/********************  Bit definition for SPI_DR register  ********************/
#define  SPI_DR_DR                           ((uint32_t)0x0000FFFF)        /*!< Data Register */

/*******************  Bit definition for SPI_CRCPR register  ******************/
#define  SPI_CRCPR_CRCPOLY                   ((uint32_t)0x0000FFFF)        /*!< CRC polynomial register */

/******************  Bit definition for SPI_RXCRCR register  ******************/
#define  SPI_RXCRCR_RXCRC                    ((uint32_t)0x0000FFFF)        /*!< Rx CRC Register */

/******************  Bit definition for SPI_TXCRCR register  ******************/
#define  SPI_TXCRCR_TXCRC                    ((uint32_t)0x0000FFFF)        /*!< Tx CRC Register */

/******************  Bit definition for SPI_I2SCFGR register  *****************/
#define  SPI_I2SCFGR_CHLEN                   ((uint32_t)0x00000001)        /*!< Channel length (number of bits per audio channel) */

#define  SPI_I2SCFGR_DATLEN                  ((uint32_t)0x00000006)        /*!< DATLEN[1:0] bits (Data length to be transferred) */
#define  SPI_I2SCFGR_DATLEN_0                ((uint32_t)0x00000002)        /*!< Bit 0 */
#define  SPI_I2SCFGR_DATLEN_1                ((uint32_t)0x00000004)        /*!< Bit 1 */

#define  SPI_I2SCFGR_CKPOL                   ((uint32_t)0x00000008)        /*!< steady state clock polarity */

#define  SPI_I2SCFGR_I2SSTD                  ((uint32_t)0x00000030)        /*!< I2SSTD[1:0] bits (I2S standard selection) */
#define  SPI_I2SCFGR_I2SSTD_0                ((uint32_t)0x00000010)        /*!< Bit 0 */
#define  SPI_I2SCFGR_I2SSTD_1                ((uint32_t)0x00000020)        /*!< Bit 1 */

#define  SPI_I2SCFGR_PCMSYNC                 ((uint32_t)0x00000080)        /*!< PCM frame synchronization */

#define  SPI_I2SCFGR_I2SCFG                  ((uint32_t)0x00000300)        /*!< I2SCFG[1:0] bits (I2S configuration mode) */
#define  SPI_I2SCFGR_I2SCFG_0                ((uint32_t)0x00000100)        /*!< Bit 0 */
#define  SPI_I2SCFGR_I2SCFG_1                ((uint32_t)0x00000200)        /*!< Bit 1 */

#define  SPI_I2SCFGR_I2SE                    ((uint32_t)0x00000400)        /*!< I2S Enable */
#define  SPI_I2SCFGR_I2SMOD                  ((uint32_t)0x00000800)        /*!< I2S mode selection */

/******************  Bit definition for SPI_I2SPR register  *******************/
#define  SPI_I2SPR_I2SDIV                    ((uint32_t)0x000000FF)        /*!< I2S Linear prescaler */
#define  SPI_I2SPR_ODD                       ((uint32_t)0x00000100)        /*!< Odd factor for the prescaler */
#define  SPI_I2SPR_MCKOE                     ((uint32_t)0x00000200)        /*!< Master Clock Output Enable */

/******************************************************************************/
/*                                                                            */
/*                      Inter-integrated Circuit Interface                    */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for I2C_CR1 register  ********************/
#define  I2C_CR1_PE                          ((uint32_t)0x00000001)        /*!< Peripheral Enable */
#define  I2C_CR1_SMBUS                       ((uint32_t)0x00000002)        /*!< SMBus Mode */
#define  I2C_CR1_SMBTYPE                     ((uint32_t)0x00000008)        /*!< SMBus Type */
#define  I2C_CR1_ENARP                       ((uint32_t)0x00000010)        /*!< ARP Enable */
#define  I2C_CR1_ENPEC                       ((uint32_t)0x00000020)        /*!< PEC Enable */
#define  I2C_CR1_ENGC                        ((uint32_t)0x00000040)        /*!< General Call Enable */
#define  I2C_CR1_NOSTRETCH                   ((uint32_t)0x00000080)        /*!< Clock Stretching Disable (Slave mode) */
#define  I2C_CR1_START                       ((uint32_t)0x00000100)        /*!< Start Generation */
#define  I2C_CR1_STOP                        ((uint32_t)0x00000200)        /*!< Stop Generation */
#define  I2C_CR1_ACK                         ((uint32_t)0x00000400)        /*!< Acknowledge Enable */
#define  I2C_CR1_POS                         ((uint32_t)0x00000800)        /*!< Acknowledge/PEC Position (for data reception) */
#define  I2C_CR1_PEC                         ((uint32_t)0x00001000)        /*!< Packet Error Checking */
#define  I2C_CR1_ALERT                       ((uint32_t)0x00002000)        /*!< SMBus Alert */
#define  I2C_CR1_SWRST                       ((uint32_t)0x00008000)        /*!< Software Reset */

/*******************  Bit definition for I2C_CR2 register  ********************/
#define  I2C_CR2_FREQ                        ((uint32_t)0x0000003F)        /*!< FREQ[5:0] bits (Peripheral Clock Frequency) */
#define  I2C_CR2_FREQ_0                      ((uint32_t)0x00000001)        /*!< Bit 0 */
#define  I2C_CR2_FREQ_1                      ((uint32_t)0x00000002)        /*!< Bit 1 */
#define  I2C_CR2_FREQ_2                      ((uint32_t)0x00000004)        /*!< Bit 2 */
#define  I2C_CR2_FREQ_3                      ((uint32_t)0x00000008)        /*!< Bit 3 */
#define  I2C_CR2_FREQ_4                      ((uint32_t)0x00000010)        /*!< Bit 4 */
#define  I2C_CR2_FREQ_5                      ((uint32_t)0x00000020)        /*!< Bit 5 */

#define  I2C_CR2_ITERREN                     ((uint32_t)0x00000100)        /*!< Error Interrupt Enable */
#define  I2C_CR2_ITEVTEN                     ((uint32_t)0x00000200)        /*!< Event Interrupt Enable */
#define  I2C_CR2_ITBUFEN                     ((uint32_t)0x00000400)        /*!< Buffer Interrupt Enable */
#define  I2C_CR2_DMAEN                       ((uint32_t)0x00000800)        /*!< DMA Requests Enable */
#define  I2C_CR2_LAST                        ((uint32_t)0x00001000)        /*!< DMA Last Transfer */

/*******************  Bit definition for I2C_OAR1 register  *******************/
#define  I2C_OAR1_ADD1_7                     ((uint32_t)0x000000FE)        /*!< Interface Address */
#define  I2C_OAR1_ADD8_9                     ((uint32_t)0x00000300)        /*!< Interface Address */

#define  I2C_OAR1_ADD0                       ((uint32_t)0x00000001)        /*!< Bit 0 */
#define  I2C_OAR1_ADD1                       ((uint32_t)0x00000002)        /*!< Bit 1 */
#define  I2C_OAR1_ADD2                       ((uint32_t)0x00000004)        /*!< Bit 2 */
#define  I2C_OAR1_ADD3                       ((uint32_t)0x00000008)        /*!< Bit 3 */
#define  I2C_OAR1_ADD4                       ((uint32_t)0x00000010)        /*!< Bit 4 */
#define  I2C_OAR1_ADD5                       ((uint32_t)0x00000020)        /*!< Bit 5 */
#define  I2C_OAR1_ADD6                       ((uint32_t)0x00000040)        /*!< Bit 6 */
#define  I2C_OAR1_ADD7                       ((uint32_t)0x00000080)        /*!< Bit 7 */
#define  I2C_OAR1_ADD8                       ((uint32_t)0x00000100)        /*!< Bit 8 */
#define  I2C_OAR1_ADD9                       ((uint32_t)0x00000200)        /*!< Bit 9 */

#define  I2C_OAR1_ADDMODE                    ((uint32_t)0x00008000)        /*!< Addressing Mode (Slave mode) */

/*******************  Bit definition for I2C_OAR2 register  *******************/
#define  I2C_OAR2_ENDUAL                     ((uint32_t)0x00000001)        /*!< Dual addressing mode enable */
#define  I2C_OAR2_ADD2                       ((uint32_t)0x000000FE)        /*!< Interface address */

/*******************  Bit definition for I2C_SR1 register  ********************/
#define  I2C_SR1_SB                          ((uint32_t)0x00000001)        /*!< Start Bit (Master mode) */
#define  I2C_SR1_ADDR                        ((uint32_t)0x00000002)        /*!< Address sent (master mode)/matched (slave mode) */
#define  I2C_SR1_BTF                         ((uint32_t)0x00000004)        /*!< Byte Transfer Finished */
#define  I2C_SR1_ADD10                       ((uint32_t)0x00000008)        /*!< 10-bit header sent (Master mode) */
#define  I2C_SR1_STOPF                       ((uint32_t)0x00000010)        /*!< Stop detection (Slave mode) */
#define  I2C_SR1_RXNE                        ((uint32_t)0x00000040)        /*!< Data Register not Empty (receivers) */
#define  I2C_SR1_TXE                         ((uint32_t)0x00000080)        /*!< Data Register Empty (transmitters) */
#define  I2C_SR1_BERR                        ((uint32_t)0x00000100)        /*!< Bus Error */
#define  I2C_SR1_ARLO                        ((uint32_t)0x00000200)        /*!< Arbitration Lost (master mode) */
#define  I2C_SR1_AF                          ((uint32_t)0x00000400)        /*!< Acknowledge Failure */
#define  I2C_SR1_OVR                         ((uint32_t)0x00000800)        /*!< Overrun/Underrun */
#define  I2C_SR1_PECERR                      ((uint32_t)0x00001000)        /*!< PEC Error in reception */
#define  I2C_SR1_TIMEOUT                     ((uint32_t)0x00004000)        /*!< Timeout or Tlow Error */
#define  I2C_SR1_SMBALERT                    ((uint32_t)0x00008000)        /*!< SMBus Alert */

/*******************  Bit definition for I2C_SR2 register  ********************/
#define  I2C_SR2_MSL                         ((uint32_t)0x00000001)        /*!< Master/Slave */
#define  I2C_SR2_BUSY                        ((uint32_t)0x00000002)        /*!< Bus Busy */
#define  I2C_SR2_TRA                         ((uint32_t)0x00000004)        /*!< Transmitter/Receiver */
#define  I2C_SR2_GENCALL                     ((uint32_t)0x00000010)        /*!< General Call Address (Slave mode) */
#define  I2C_SR2_SMBDEFAULT                  ((uint32_t)0x00000020)        /*!< SMBus Device Default Address (Slave mode) */
#define  I2C_SR2_SMBHOST                     ((uint32_t)0x00000040)        /*!< SMBus Host Header (Slave mode) */
#define  I2C_SR2_DUALF                       ((uint32_t)0x00000080)        /*!< Dual Flag (Slave mode) */
#define  I2C_SR2_PEC                         ((uint32_t)0x0000FF00)        /*!< Packet Error Checking Register */

/*******************  Bit definition for I2C_CCR register  ********************/
#define  I2C_CCR_CCR                         ((uint32_t)0x00000FFF)        /*!< Clock Control Register in Fast/Standard mode (Master mode) */
#define  I2C_CCR_DUTY                        ((uint32_t)0x00004000)        /*!< Fast Mode Duty Cycle */
#define  I2C_CCR_FS                          ((uint32_t)0x00008000)        /*!< I2C Master Mode Selection */

/******************  Bit definition for I2C_TRISE register  *******************/
#define  I2C_TRISE_TRISE                     ((uint32_t)0x0000003F)        /*!< Maximum Rise Time in Fast/Standard mode (Master mode) */

/******************************************************************************/
/*                                                                            */
/*         Universal Synchronous Asynchronous Receiver Transmitter            */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for USART_SR register  *******************/
#define  USART_SR_PE                         ((uint32_t)0x00000001)            /*!< Parity Error */
#define  USART_SR_FE                         ((uint32_t)0x00000002)            /*!< Framing Error */
#define  USART_SR_NE                         ((uint32_t)0x00000004)            /*!< Noise Error Flag */
#define  USART_SR_ORE                        ((uint32_t)0x00000008)            /*!< OverRun Error */
#define  USART_SR_IDLE                       ((uint32_t)0x00000010)            /*!< IDLE line detected */
#define  USART_SR_RXNE                       ((uint32_t)0x00000020)            /*!< Read Data Register Not Empty */
#define  USART_SR_TC                         ((uint32_t)0x00000040)            /*!< Transmission Complete */
#define  USART_SR_TXE                        ((uint32_t)0x00000080)            /*!< Transmit Data Register Empty */
#define  USART_SR_LBD                        ((uint32_t)0x00000100)            /*!< LIN Break Detection Flag */
#define  USART_SR_CTS                        ((uint32_t)0x00000200)            /*!< CTS Flag */

/*******************  Bit definition for USART_DR register  *******************/
#define  USART_DR_DR                         ((uint32_t)0x000001FF)            /*!< Data value */

/******************  Bit definition for USART_BRR register  *******************/
#define  USART_BRR_DIV_Fraction              ((uint32_t)0x0000000F)            /*!< Fraction of USARTDIV */
#define  USART_BRR_DIV_Mantissa              ((uint32_t)0x0000FFF0)            /*!< Mantissa of USARTDIV */

/******************  Bit definition for USART_CR1 register  *******************/
#define  USART_CR1_SBK                       ((uint32_t)0x00000001)            /*!< Send Break */
#define  USART_CR1_RWU                       ((uint32_t)0x00000002)            /*!< Receiver wakeup */
#define  USART_CR1_RE                        ((uint32_t)0x00000004)            /*!< Receiver Enable */
#define  USART_CR1_TE                        ((uint32_t)0x00000008)            /*!< Transmitter Enable */
#define  USART_CR1_IDLEIE                    ((uint32_t)0x00000010)            /*!< IDLE Interrupt Enable */
#define  USART_CR1_RXNEIE                    ((uint32_t)0x00000020)            /*!< RXNE Interrupt Enable */
#define  USART_CR1_TCIE                      ((uint32_t)0x00000040)            /*!< Transmission Complete Interrupt Enable */
#define  USART_CR1_TXEIE                     ((uint32_t)0x00000080)            /*!< PE Interrupt Enable */
#define  USART_CR1_PEIE                      ((uint32_t)0x00000100)            /*!< PE Interrupt Enable */
#define  USART_CR1_PS                        ((uint32_t)0x00000200)            /*!< Parity Selection */
#define  USART_CR1_PCE                       ((uint32_t)0x00000400)            /*!< Parity Control Enable */
#define  USART_CR1_WAKE                      ((uint32_t)0x00000800)            /*!< Wakeup method */
#define  USART_CR1_M                         ((uint32_t)0x00001000)            /*!< Word length */
#define  USART_CR1_UE                        ((uint32_t)0x00002000)            /*!< USART Enable */

/******************  Bit definition for USART_CR2 register  *******************/
#define  USART_CR2_ADD                       ((uint32_t)0x0000000F)            /*!< Address of the USART node */
#define  USART_CR2_LBDL                      ((uint32_t)0x00000020)            /*!< LIN Break Detection Length */
#define  USART_CR2_LBDIE                     ((uint32_t)0x00000040)            /*!< LIN Break Detection Interrupt Enable */
#define  USART_CR2_LBCL                      ((uint32_t)0x00000100)            /*!< Last Bit Clock pulse */
#define  USART_CR2_CPHA                      ((uint32_t)0x00000200)            /*!< Clock Phase */
#define  USART_CR2_CPOL                      ((uint32_t)0x00000400)            /*!< Clock Polarity */
#define  USART_CR2_CLKEN                     ((uint32_t)0x00000800)            /*!< Clock Enable */

#define  USART_CR2_STOP                      ((uint32_t)0x00003000)            /*!< STOP[1:0] bits (STOP bits) */
#define  USART_CR2_STOP_0                    ((uint32_t)0x00001000)            /*!< Bit 0 */
#define  USART_CR2_STOP_1                    ((uint32_t)0x00002000)            /*!< Bit 1 */

#define  USART_CR2_LINEN                     ((uint32_t)0x00004000)            /*!< LIN mode enable */

/******************  Bit definition for USART_CR3 register  *******************/
#define  USART_CR3_EIE                       ((uint32_t)0x00000001)            /*!< Error Interrupt Enable */
#define  USART_CR3_IREN                      ((uint32_t)0x00000002)            /*!< IrDA mode Enable */
#define  USART_CR3_IRLP                      ((uint32_t)0x00000004)            /*!< IrDA Low-Power */
#define  USART_CR3_HDSEL                     ((uint32_t)0x00000008)            /*!< Half-Duplex Selection */
#define  USART_CR3_NACK                      ((uint32_t)0x00000010)            /*!< Smartcard NACK enable */
#define  USART_CR3_SCEN                      ((uint32_t)0x00000020)            /*!< Smartcard mode enable */
#define  USART_CR3_DMAR                      ((uint32_t)0x00000040)            /*!< DMA Enable Receiver */
#define  USART_CR3_DMAT                      ((uint32_t)0x00000080)            /*!< DMA Enable Transmitter */
#define  USART_CR3_RTSE                      ((uint32_t)0x00000100)            /*!< RTS Enable */
#define  USART_CR3_CTSE                      ((uint32_t)0x00000200)            /*!< CTS Enable */
#define  USART_CR3_CTSIE                     ((uint32_t)0x00000400)            /*!< CTS Interrupt Enable */

/******************  Bit definition for USART_GTPR register  ******************/
#define  USART_GTPR_PSC                      ((uint32_t)0x000000FF)            /*!< PSC[7:0] bits (Prescaler value) */
#define  USART_GTPR_PSC_0                    ((uint32_t)0x00000001)            /*!< Bit 0 */
#define  USART_GTPR_PSC_1                    ((uint32_t)0x00000002)            /*!< Bit 1 */
#define  USART_GTPR_PSC_2                    ((uint32_t)0x00000004)            /*!< Bit 2 */
#define  USART_GTPR_PSC_3                    ((uint32_t)0x00000008)            /*!< Bit 3 */
#define  USART_GTPR_PSC_4                    ((uint32_t)0x00000010)            /*!< Bit 4 */
#define  USART_GTPR_PSC_5                    ((uint32_t)0x00000020)            /*!< Bit 5 */
#define  USART_GTPR_PSC_6                    ((uint32_t)0x00000040)            /*!< Bit 6 */
#define  USART_GTPR_PSC_7                    ((uint32_t)0x00000080)            /*!< Bit 7 */

#define  USART_GTPR_GT                       ((uint32_t)0x0000FF00)            /*!< Guard time value */

/******************************************************************************/
/*                                                                            */
/*                                 Debug MCU                                  */
/*                                                                            */
/******************************************************************************/

/****************  Bit definition for DBGMCU_IDCODE register  *****************/
#define  DBGMCU_IDCODE_DEV_ID                ((uint32_t)0x00000FFF)        /*!< Device Identifier */

#define  DBGMCU_IDCODE_REV_ID                ((uint32_t)0xFFFF0000)        /*!< REV_ID[15:0] bits (Revision Identifier) */
#define  DBGMCU_IDCODE_REV_ID_0              ((uint32_t)0x00010000)        /*!< Bit 0 */
#define  DBGMCU_IDCODE_REV_ID_1              ((uint32_t)0x00020000)        /*!< Bit 1 */
#define  DBGMCU_IDCODE_REV_ID_2              ((uint32_t)0x00040000)        /*!< Bit 2 */
#define  DBGMCU_IDCODE_REV_ID_3              ((uint32_t)0x00080000)        /*!< Bit 3 */
#define  DBGMCU_IDCODE_REV_ID_4              ((uint32_t)0x00100000)        /*!< Bit 4 */
#define  DBGMCU_IDCODE_REV_ID_5              ((uint32_t)0x00200000)        /*!< Bit 5 */
#define  DBGMCU_IDCODE_REV_ID_6              ((uint32_t)0x00400000)        /*!< Bit 6 */
#define  DBGMCU_IDCODE_REV_ID_7              ((uint32_t)0x00800000)        /*!< Bit 7 */
#define  DBGMCU_IDCODE_REV_ID_8              ((uint32_t)0x01000000)        /*!< Bit 8 */
#define  DBGMCU_IDCODE_REV_ID_9              ((uint32_t)0x02000000)        /*!< Bit 9 */
#define  DBGMCU_IDCODE_REV_ID_10             ((uint32_t)0x04000000)        /*!< Bit 10 */
#define  DBGMCU_IDCODE_REV_ID_11             ((uint32_t)0x08000000)        /*!< Bit 11 */
#define  DBGMCU_IDCODE_REV_ID_12             ((uint32_t)0x10000000)        /*!< Bit 12 */
#define  DBGMCU_IDCODE_REV_ID_13             ((uint32_t)0x20000000)        /*!< Bit 13 */
#define  DBGMCU_IDCODE_REV_ID_14             ((uint32_t)0x40000000)        /*!< Bit 14 */
#define  DBGMCU_IDCODE_REV_ID_15             ((uint32_t)0x80000000)        /*!< Bit 15 */

/******************  Bit definition for DBGMCU_CR register  *******************/
#define  DBGMCU_CR_DBG_SLEEP                 ((uint32_t)0x00000001)        /*!< Debug Sleep Mode */
#define  DBGMCU_CR_DBG_STOP                  ((uint32_t)0x00000002)        /*!< Debug Stop Mode */
#define  DBGMCU_CR_DBG_STANDBY               ((uint32_t)0x00000004)        /*!< Debug Standby mode */
#define  DBGMCU_CR_TRACE_IOEN                ((uint32_t)0x00000020)        /*!< Trace Pin Assignment Control */

#define  DBGMCU_CR_TRACE_MODE                ((uint32_t)0x000000C0)        /*!< TRACE_MODE[1:0] bits (Trace Pin Assignment Control) */
#define  DBGMCU_CR_TRACE_MODE_0              ((uint32_t)0x00000040)        /*!< Bit 0 */
#define  DBGMCU_CR_TRACE_MODE_1              ((uint32_t)0x00000080)        /*!< Bit 1 */

#define  DBGMCU_CR_DBG_IWDG_STOP             ((uint32_t)0x00000100)        /*!< Debug Independent Watchdog stopped when Core is halted */
#define  DBGMCU_CR_DBG_WWDG_STOP             ((uint32_t)0x00000200)        /*!< Debug Window Watchdog stopped when Core is halted */
#define  DBGMCU_CR_DBG_TIM1_STOP             ((uint32_t)0x00000400)        /*!< TIM1 counter stopped when core is halted */
#define  DBGMCU_CR_DBG_TIM2_STOP             ((uint32_t)0x00000800)        /*!< TIM2 counter stopped when core is halted */
#define  DBGMCU_CR_DBG_TIM3_STOP             ((uint32_t)0x00001000)        /*!< TIM3 counter stopped when core is halted */
#define  DBGMCU_CR_DBG_TIM4_STOP             ((uint32_t)0x00002000)        /*!< TIM4 counter stopped when core is halted */
#define  DBGMCU_CR_DBG_CAN1_STOP             ((uint32_t)0x00004000)        /*!< Debug CAN1 stopped when Core is halted */
#define  DBGMCU_CR_DBG_I2C1_SMBUS_TIMEOUT    ((uint32_t)0x00008000)        /*!< SMBUS timeout mode stopped when Core is halted */
#define  DBGMCU_CR_DBG_I2C2_SMBUS_TIMEOUT    ((uint32_t)0x00010000)        /*!< SMBUS timeout mode stopped when Core is halted */
#define  DBGMCU_CR_DBG_TIM5_STOP             ((uint32_t)0x00040000)        /*!< TIM5 counter stopped when core is halted */
#define  DBGMCU_CR_DBG_TIM6_STOP             ((uint32_t)0x00080000)        /*!< TIM6 counter stopped when core is halted */
#define  DBGMCU_CR_DBG_TIM7_STOP             ((uint32_t)0x00100000)        /*!< TIM7 counter stopped when core is halted */
#define  DBGMCU_CR_DBG_CAN2_STOP             ((uint32_t)0x00200000)        /*!< Debug CAN2 stopped when Core is halted */
#define  DBGMCU_CR_DBG_TIM9_STOP             ((uint32_t)0x10000000)        /*!< Debug TIM9 stopped when Core is halted */
#define  DBGMCU_CR_DBG_TIM10_STOP            ((uint32_t)0x20000000)        /*!< Debug TIM10 stopped when Core is halted */
#define  DBGMCU_CR_DBG_TIM11_STOP            ((uint32_t)0x40000000)        /*!< Debug TIM11 stopped when Core is halted */

/******************************************************************************/
/*                                                                            */
/*                      FLASH and Option Bytes Registers                      */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for FLASH_ACR register  ******************/
#define  FLASH_ACR_LATENCY                   ((uint32_t)0x00000007)        /*!< LATENCY[2:0] bits (Latency) */
#define  FLASH_ACR_LATENCY_0                 ((uint32_t)0x00000001)        /*!< Bit 0 */
#define  FLASH_ACR_LATENCY_1                 ((uint32_t)0x00000002)        /*!< Bit 1 */
#define  FLASH_ACR_LATENCY_2                 ((uint32_t)0x00000004)        /*!< Bit 2 */

#define  FLASH_ACR_HLFCYA                    ((uint32_t)0x00000008)        /*!< Flash Half Cycle Access Enable */
#define  FLASH_ACR_PRFTBE                    ((uint32_t)0x00000010)        /*!< Prefetch Buffer Enable */
#define  FLASH_ACR_PRFTBS                    ((uint32_t)0x00000020)        /*!< Prefetch Buffer Status */

/******************  Bit definition for FLASH_KEYR register  ******************/
#define  FLASH_KEYR_FKEYR                    ((uint32_t)0xFFFFFFFF)        /*!< FPEC Key */

#define  RDP_KEY                             ((uint32_t)0x000000A5)        /*!< RDP Key */
#define  FLASH_KEY1                          ((uint32_t)0x45670123)        /*!< FPEC Key1 */
#define  FLASH_KEY2                          ((uint32_t)0xCDEF89AB)        /*!< FPEC Key2 */

/*****************  Bit definition for FLASH_OPTKEYR register  ****************/
#define  FLASH_OPTKEYR_OPTKEYR               ((uint32_t)0xFFFFFFFF)        /*!< Option Byte Key */

#define  FLASH_OPTKEY1                       FLASH_KEY1                    /*!< Option Byte Key1 */
#define  FLASH_OPTKEY2                       FLASH_KEY2                    /*!< Option Byte Key2 */

/******************  Bit definition for FLASH_SR register  ********************/
#define  FLASH_SR_BSY                        ((uint32_t)0x00000001)        /*!< Busy */
#define  FLASH_SR_PGERR                      ((uint32_t)0x00000004)        /*!< Programming Error */
#define  FLASH_SR_WRPRTERR                   ((uint32_t)0x00000010)        /*!< Write Protection Error */
#define  FLASH_SR_EOP                        ((uint32_t)0x00000020)        /*!< End of operation */

/*******************  Bit definition for FLASH_CR register  *******************/
#define  FLASH_CR_PG                         ((uint32_t)0x00000001)        /*!< Programming */
#define  FLASH_CR_PER                        ((uint32_t)0x00000002)        /*!< Page Erase */
#define  FLASH_CR_MER                        ((uint32_t)0x00000004)        /*!< Mass Erase */
#define  FLASH_CR_OPTPG                      ((uint32_t)0x00000010)        /*!< Option Byte Programming */
#define  FLASH_CR_OPTER                      ((uint32_t)0x00000020)        /*!< Option Byte Erase */
#define  FLASH_CR_STRT                       ((uint32_t)0x00000040)        /*!< Start */
#define  FLASH_CR_LOCK                       ((uint32_t)0x00000080)        /*!< Lock */
#define  FLASH_CR_OPTWRE                     ((uint32_t)0x00000200)        /*!< Option Bytes Write Enable */
#define  FLASH_CR_ERRIE                      ((uint32_t)0x00000400)        /*!< Error Interrupt Enable */
#define  FLASH_CR_EOPIE                      ((uint32_t)0x00001000)        /*!< End of operation interrupt enable */

/*******************  Bit definition for FLASH_AR register  *******************/
#define  FLASH_AR_FAR                        ((uint32_t)0xFFFFFFFF)        /*!< Flash Address */

/******************  Bit definition for FLASH_OBR register  *******************/
#define  FLASH_OBR_OPTERR                    ((uint32_t)0x00000001)        /*!< Option Byte Error */
#define  FLASH_OBR_RDPRT                     ((uint32_t)0x00000002)        /*!< Read protection */

#define  FLASH_OBR_IWDG_SW                   ((uint32_t)0x00000004)        /*!< IWDG SW */
#define  FLASH_OBR_nRST_STOP                 ((uint32_t)0x00000008)        /*!< nRST_STOP */
#define  FLASH_OBR_nRST_STDBY                ((uint32_t)0x00000010)        /*!< nRST_STDBY */
#define  FLASH_OBR_USER                      ((uint32_t)0x0000001C)        /*!< User Option Bytes */

/******************  Bit definition for FLASH_WRPR register  ******************/
#define  FLASH_WRPR_WRP                      ((uint32_t)0xFFFFFFFF)      /*!< Write Protect */

/*----------------------------------------------------------------------------*/

/******************  Bit definition for FLASH_RDP register  *******************/
#define  FLASH_RDP_RDP                       ((uint32_t)0x000000FF)        /*!< Read protection option byte */
#define  FLASH_RDP_nRDP                      ((uint32_t)0x0000FF00)        /*!< Read protection complemented option byte */

/******************  Bit definition for FLASH_USER register  ******************/
#define  FLASH_USER_USER                     ((uint32_t)0x00FF0000)        /*!< User option byte */
#define  FLASH_USER_nUSER                    ((uint32_t)0xFF000000)        /*!< User complemented option byte */

/******************  Bit definition for FLASH_Data0 register  *****************/
#define  FLASH_DATA0_DATA0                   ((uint32_t)0x000000FF)        /*!< User data storage option byte */
#define  FLASH_DATA0_nDATA0                  ((uint32_t)0x0000FF00)        /*!< User data storage complemented option byte */

/******************  Bit definition for FLASH_Data1 register  *****************/
#define  FLASH_DATA1_DATA1                   ((uint32_t)0x00FF0000)        /*!< User data storage option byte */
#define  FLASH_DATA1_nDATA1                  ((uint32_t)0xFF000000)        /*!< User data storage complemented option byte */

/******************  Bit definition for FLASH_WRP0 register  ******************/
#define  FLASH_WRP0_WRP0                     ((uint32_t)0x000000FF)        /*!< Flash memory write protection option bytes */
#define  FLASH_WRP0_nWRP0                    ((uint32_t)0x0000FF00)        /*!< Flash memory write protection complemented option bytes */

/******************  Bit definition for FLASH_WRP1 register  ******************/
#define  FLASH_WRP1_WRP1                     ((uint32_t)0x00FF0000)        /*!< Flash memory write protection option bytes */
#define  FLASH_WRP1_nWRP1                    ((uint32_t)0xFF000000)        /*!< Flash memory write protection complemented option bytes */

/******************  Bit definition for FLASH_WRP2 register  ******************/
#define  FLASH_WRP2_WRP2                     ((uint32_t)0x000000FF)        /*!< Flash memory write protection option bytes */
#define  FLASH_WRP2_nWRP2                    ((uint32_t)0x0000FF00)        /*!< Flash memory write protection complemented option bytes */

/******************  Bit definition for FLASH_WRP3 register  ******************/
#define  FLASH_WRP3_WRP3                     ((uint32_t)0x00FF0000)        /*!< Flash memory write protection option bytes */
#define  FLASH_WRP3_nWRP3                    ((uint32_t)0xFF000000)        /*!< Flash memory write protection complemented option bytes */

/******************************************************************************/
/*                Ethernet MAC Registers bits definitions                     */
/******************************************************************************/
/* Bit definition for Ethernet MAC Control Register register */
#define ETH_MACCR_WD            ((uint32_t)0x00800000)  /* Watchdog disable */
#define ETH_MACCR_JD            ((uint32_t)0x00400000)  /* Jabber disable */
#define ETH_MACCR_IFG           ((uint32_t)0x000E0000)  /* Inter-frame gap */
#define ETH_MACCR_IFG_96Bit     ((uint32_t)0x00000000)  /* Minimum IFG between frames during transmission is 96Bit */
#define ETH_MACCR_IFG_88Bit     ((uint32_t)0x00020000)  /* Minimum IFG between frames during transmission is 88Bit */
#define ETH_MACCR_IFG_80Bit     ((uint32_t)0x00040000)  /* Minimum IFG between frames during transmission is 80Bit */
#define ETH_MACCR_IFG_72Bit     ((uint32_t)0x00060000)  /* Minimum IFG between frames during transmission is 72Bit */
#define ETH_MACCR_IFG_64Bit     ((uint32_t)0x00080000)  /* Minimum IFG between frames during transmission is 64Bit */        
#define ETH_MACCR_IFG_56Bit     ((uint32_t)0x000A0000)  /* Minimum IFG between frames during transmission is 56Bit */
#define ETH_MACCR_IFG_48Bit     ((uint32_t)0x000C0000)  /* Minimum IFG between frames during transmission is 48Bit */
#define ETH_MACCR_IFG_40Bit     ((uint32_t)0x000E0000)  /* Minimum IFG between frames during transmission is 40Bit */              
#define ETH_MACCR_CSD           ((uint32_t)0x00010000)  /* Carrier sense disable (during transmission) */
#define ETH_MACCR_FES           ((uint32_t)0x00004000)  /* Fast ethernet speed */
#define ETH_MACCR_ROD           ((uint32_t)0x00002000)  /* Receive own disable */
#define ETH_MACCR_LM            ((uint32_t)0x00001000)  /* loopback mode */
#define ETH_MACCR_DM            ((uint32_t)0x00000800)  /* Duplex mode */
#define ETH_MACCR_IPCO          ((uint32_t)0x00000400)  /* IP Checksum offload */
#define ETH_MACCR_RD            ((uint32_t)0x00000200)  /* Retry disable */
#define ETH_MACCR_APCS          ((uint32_t)0x00000080)  /* Automatic Pad/CRC stripping */
#define ETH_MACCR_BL            ((uint32_t)0x00000060)  /* Back-off limit: random integer number (r) of slot time delays before rescheduling
                                                       a transmission attempt during retries after a collision: 0 =< r <2^k */
#define ETH_MACCR_BL_10         ((uint32_t)0x00000000)  /* k = min (n, 10) */
#define ETH_MACCR_BL_8          ((uint32_t)0x00000020)  /* k = min (n, 8) */
#define ETH_MACCR_BL_4          ((uint32_t)0x00000040)  /* k = min (n, 4) */
#define ETH_MACCR_BL_1          ((uint32_t)0x00000060)  /* k = min (n, 1) */ 
#define ETH_MACCR_DC            ((uint32_t)0x00000010)  /* Defferal check */
#define ETH_MACCR_TE            ((uint32_t)0x00000008)  /* Transmitter enable */
#define ETH_MACCR_RE            ((uint32_t)0x00000004)  /* Receiver enable */

/* Bit definition for Ethernet MAC Frame Filter Register */
#define ETH_MACFFR_RA           ((uint32_t)0x80000000)  /* Receive all */ 
#define ETH_MACFFR_HPF          ((uint32_t)0x00000400)  /* Hash or perfect filter */ 
#define ETH_MACFFR_SAF          ((uint32_t)0x00000200)  /* Source address filter enable */ 
#define ETH_MACFFR_SAIF         ((uint32_t)0x00000100)  /* SA inverse filtering */ 
#define ETH_MACFFR_PCF          ((uint32_t)0x000000C0)  /* Pass control frames: 3 cases */
#define ETH_MACFFR_PCF_BlockAll                ((uint32_t)0x00000040)  /* MAC filters all control frames from reaching the application */
#define ETH_MACFFR_PCF_ForwardAll              ((uint32_t)0x00000080)  /* MAC forwards all control frames to application even if they fail the Address Filter */
#define ETH_MACFFR_PCF_ForwardPassedAddrFilter ((uint32_t)0x000000C0)  /* MAC forwards control frames that pass the Address Filter. */ 
#define ETH_MACFFR_BFD          ((uint32_t)0x00000020)  /* Broadcast frame disable */ 
#define ETH_MACFFR_PAM          ((uint32_t)0x00000010)  /* Pass all mutlicast */ 
#define ETH_MACFFR_DAIF         ((uint32_t)0x00000008)  /* DA Inverse filtering */ 
#define ETH_MACFFR_HM           ((uint32_t)0x00000004)  /* Hash multicast */ 
#define ETH_MACFFR_HU           ((uint32_t)0x00000002)  /* Hash unicast */
#define ETH_MACFFR_PM           ((uint32_t)0x00000001)  /* Promiscuous mode */

/* Bit definition for Ethernet MAC Hash Table High Register */
#define ETH_MACHTHR_HTH         ((uint32_t)0xFFFFFFFF)  /* Hash table high */

/* Bit definition for Ethernet MAC Hash Table Low Register */
#define ETH_MACHTLR_HTL         ((uint32_t)0xFFFFFFFF)  /* Hash table low */

/* Bit definition for Ethernet MAC MII Address Register */
#define ETH_MACMIIAR_PA         ((uint32_t)0x0000F800)  /* Physical layer address */ 
#define ETH_MACMIIAR_MR         ((uint32_t)0x000007C0)  /* MII register in the selected PHY */ 
#define ETH_MACMIIAR_CR         ((uint32_t)0x0000001C)  /* CR clock range: 6 cases */ 
#define ETH_MACMIIAR_CR_DIV42   ((uint32_t)0x00000000)  /* HCLK:60-72 MHz; MDC clock= HCLK/42 */
#define ETH_MACMIIAR_CR_DIV16   ((uint32_t)0x00000008)  /* HCLK:20-35 MHz; MDC clock= HCLK/16 */
#define ETH_MACMIIAR_CR_DIV26   ((uint32_t)0x0000000C)  /* HCLK:35-60 MHz; MDC clock= HCLK/26 */
#define ETH_MACMIIAR_MW         ((uint32_t)0x00000002)  /* MII write */ 
#define ETH_MACMIIAR_MB         ((uint32_t)0x00000001)  /* MII busy */ 
  
/* Bit definition for Ethernet MAC MII Data Register */
#define ETH_MACMIIDR_MD         ((uint32_t)0x0000FFFF)  /* MII data: read/write data from/to PHY */

/* Bit definition for Ethernet MAC Flow Control Register */
#define ETH_MACFCR_PT           ((uint32_t)0xFFFF0000)  /* Pause time */
#define ETH_MACFCR_ZQPD         ((uint32_t)0x00000080)  /* Zero-quanta pause disable */
#define ETH_MACFCR_PLT          ((uint32_t)0x00000030)  /* Pause low threshold: 4 cases */
#define ETH_MACFCR_PLT_Minus4   ((uint32_t)0x00000000)  /* Pause time minus 4 slot times */
#define ETH_MACFCR_PLT_Minus28  ((uint32_t)0x00000010)  /* Pause time minus 28 slot times */
#define ETH_MACFCR_PLT_Minus144 ((uint32_t)0x00000020)  /* Pause time minus 144 slot times */
#define ETH_MACFCR_PLT_Minus256 ((uint32_t)0x00000030)  /* Pause time minus 256 slot times */      
#define ETH_MACFCR_UPFD         ((uint32_t)0x00000008)  /* Unicast pause frame detect */
#define ETH_MACFCR_RFCE         ((uint32_t)0x00000004)  /* Receive flow control enable */
#define ETH_MACFCR_TFCE         ((uint32_t)0x00000002)  /* Transmit flow control enable */
#define ETH_MACFCR_FCBBPA       ((uint32_t)0x00000001)  /* Flow control busy/backpressure activate */

/* Bit definition for Ethernet MAC VLAN Tag Register */
#define ETH_MACVLANTR_VLANTC    ((uint32_t)0x00010000)  /* 12-bit VLAN tag comparison */
#define ETH_MACVLANTR_VLANTI    ((uint32_t)0x0000FFFF)  /* VLAN tag identifier (for receive frames) */

/* Bit definition for Ethernet MAC Remote Wake-UpFrame Filter Register */ 
#define ETH_MACRWUFFR_D   ((uint32_t)0xFFFFFFFF)  /* Wake-up frame filter register data */
/* Eight sequential Writes to this address (offset 0x28) will write all Wake-UpFrame Filter Registers.
   Eight sequential Reads from this address (offset 0x28) will read all Wake-UpFrame Filter Registers. */
/* Wake-UpFrame Filter Reg0 : Filter 0 Byte Mask
   Wake-UpFrame Filter Reg1 : Filter 1 Byte Mask
   Wake-UpFrame Filter Reg2 : Filter 2 Byte Mask
   Wake-UpFrame Filter Reg3 : Filter 3 Byte Mask
   Wake-UpFrame Filter Reg4 : RSVD - Filter3 Command - RSVD - Filter2 Command - 
                              RSVD - Filter1 Command - RSVD - Filter0 Command
   Wake-UpFrame Filter Re5 : Filter3 Offset - Filter2 Offset - Filter1 Offset - Filter0 Offset
   Wake-UpFrame Filter Re6 : Filter1 CRC16 - Filter0 CRC16
   Wake-UpFrame Filter Re7 : Filter3 CRC16 - Filter2 CRC16 */

/* Bit definition for Ethernet MAC PMT Control and Status Register */ 
#define ETH_MACPMTCSR_WFFRPR ((uint32_t)0x80000000)  /* Wake-Up Frame Filter Register Pointer Reset */
#define ETH_MACPMTCSR_GU     ((uint32_t)0x00000200)  /* Global Unicast */
#define ETH_MACPMTCSR_WFR    ((uint32_t)0x00000040)  /* Wake-Up Frame Received */
#define ETH_MACPMTCSR_MPR    ((uint32_t)0x00000020)  /* Magic Packet Received */
#define ETH_MACPMTCSR_WFE    ((uint32_t)0x00000004)  /* Wake-Up Frame Enable */
#define ETH_MACPMTCSR_MPE    ((uint32_t)0x00000002)  /* Magic Packet Enable */
#define ETH_MACPMTCSR_PD     ((uint32_t)0x00000001)  /* Power Down */

/* Bit definition for Ethernet MAC Status Register */
#define ETH_MACSR_TSTS      ((uint32_t)0x00000200)  /* Time stamp trigger status */
#define ETH_MACSR_MMCTS     ((uint32_t)0x00000040)  /* MMC transmit status */
#define ETH_MACSR_MMMCRS    ((uint32_t)0x00000020)  /* MMC receive status */
#define ETH_MACSR_MMCS      ((uint32_t)0x00000010)  /* MMC status */
#define ETH_MACSR_PMTS      ((uint32_t)0x00000008)  /* PMT status */

/* Bit definition for Ethernet MAC Interrupt Mask Register */
#define ETH_MACIMR_TSTIM     ((uint32_t)0x00000200)  /* Time stamp trigger interrupt mask */
#define ETH_MACIMR_PMTIM     ((uint32_t)0x00000008)  /* PMT interrupt mask */

/* Bit definition for Ethernet MAC Address0 High Register */
#define ETH_MACA0HR_MACA0H   ((uint32_t)0x0000FFFF)  /* MAC address0 high */

/* Bit definition for Ethernet MAC Address0 Low Register */
#define ETH_MACA0LR_MACA0L   ((uint32_t)0xFFFFFFFF)  /* MAC address0 low */

/* Bit definition for Ethernet MAC Address1 High Register */
#define ETH_MACA1HR_AE               ((uint32_t)0x80000000)  /* Address enable */
#define ETH_MACA1HR_SA               ((uint32_t)0x40000000)  /* Source address */
#define ETH_MACA1HR_MBC              ((uint32_t)0x3F000000)  /* Mask byte control: bits to mask for comparison of the MAC Address bytes */
#define ETH_MACA1HR_MBC_HBits15_8    ((uint32_t)0x20000000)  /* Mask MAC Address high reg bits [15:8] */
#define ETH_MACA1HR_MBC_HBits7_0     ((uint32_t)0x10000000)  /* Mask MAC Address high reg bits [7:0] */
#define ETH_MACA1HR_MBC_LBits31_24   ((uint32_t)0x08000000)  /* Mask MAC Address low reg bits [31:24] */
#define ETH_MACA1HR_MBC_LBits23_16   ((uint32_t)0x04000000)  /* Mask MAC Address low reg bits [23:16] */
#define ETH_MACA1HR_MBC_LBits15_8    ((uint32_t)0x02000000)  /* Mask MAC Address low reg bits [15:8] */
#define ETH_MACA1HR_MBC_LBits7_0     ((uint32_t)0x01000000)  /* Mask MAC Address low reg bits [7:0] */ 
#define ETH_MACA1HR_MACA1H           ((uint32_t)0x0000FFFF)  /* MAC address1 high */

/* Bit definition for Ethernet MAC Address1 Low Register */
#define ETH_MACA1LR_MACA1L   ((uint32_t)0xFFFFFFFF)  /* MAC address1 low */

/* Bit definition for Ethernet MAC Address2 High Register */
#define ETH_MACA2HR_AE               ((uint32_t)0x80000000)  /* Address enable */
#define ETH_MACA2HR_SA               ((uint32_t)0x40000000)  /* Source address */
#define ETH_MACA2HR_MBC              ((uint32_t)0x3F000000)  /* Mask byte control */
#define ETH_MACA2HR_MBC_HBits15_8    ((uint32_t)0x20000000)  /* Mask MAC Address high reg bits [15:8] */
#define ETH_MACA2HR_MBC_HBits7_0     ((uint32_t)0x10000000)  /* Mask MAC Address high reg bits [7:0] */
#define ETH_MACA2HR_MBC_LBits31_24   ((uint32_t)0x08000000)  /* Mask MAC Address low reg bits [31:24] */
#define ETH_MACA2HR_MBC_LBits23_16   ((uint32_t)0x04000000)  /* Mask MAC Address low reg bits [23:16] */
#define ETH_MACA2HR_MBC_LBits15_8    ((uint32_t)0x02000000)  /* Mask MAC Address low reg bits [15:8] */
#define ETH_MACA2HR_MBC_LBits7_0     ((uint32_t)0x01000000)  /* Mask MAC Address low reg bits [70] */
#define ETH_MACA2HR_MACA2H           ((uint32_t)0x0000FFFF)  /* MAC address1 high */

/* Bit definition for Ethernet MAC Address2 Low Register */
#define ETH_MACA2LR_MACA2L           ((uint32_t)0xFFFFFFFF)  /* MAC address2 low */

/* Bit definition for Ethernet MAC Address3 High Register */
#define ETH_MACA3HR_AE               ((uint32_t)0x80000000)  /* Address enable */
#define ETH_MACA3HR_SA               ((uint32_t)0x40000000)  /* Source address */
#define ETH_MACA3HR_MBC              ((uint32_t)0x3F000000)  /* Mask byte control */
#define ETH_MACA3HR_MBC_HBits15_8    ((uint32_t)0x20000000)  /* Mask MAC Address high reg bits [15:8] */
#define ETH_MACA3HR_MBC_HBits7_0     ((uint32_t)0x10000000)  /* Mask MAC Address high reg bits [7:0] */
#define ETH_MACA3HR_MBC_LBits31_24   ((uint32_t)0x08000000)  /* Mask MAC Address low reg bits [31:24] */
#define ETH_MACA3HR_MBC_LBits23_16   ((uint32_t)0x04000000)  /* Mask MAC Address low reg bits [23:16] */
#define ETH_MACA3HR_MBC_LBits15_8    ((uint32_t)0x02000000)  /* Mask MAC Address low reg bits [15:8] */
#define ETH_MACA3HR_MBC_LBits7_0     ((uint32_t)0x01000000)  /* Mask MAC Address low reg bits [70] */
#define ETH_MACA3HR_MACA3H           ((uint32_t)0x0000FFFF)  /* MAC address3 high */

/* Bit definition for Ethernet MAC Address3 Low Register */
#define ETH_MACA3LR_MACA3L   ((uint32_t)0xFFFFFFFF)  /* MAC address3 low */

/******************************************************************************/
/*                Ethernet MMC Registers bits definition                      */
/******************************************************************************/

/* Bit definition for Ethernet MMC Contol Register */
#define ETH_MMCCR_MCF        ((uint32_t)0x00000008)  /* MMC Counter Freeze */
#define ETH_MMCCR_ROR        ((uint32_t)0x00000004)  /* Reset on Read */
#define ETH_MMCCR_CSR        ((uint32_t)0x00000002)  /* Counter Stop Rollover */
#define ETH_MMCCR_CR         ((uint32_t)0x00000001)  /* Counters Reset */

/* Bit definition for Ethernet MMC Receive Interrupt Register */
#define ETH_MMCRIR_RGUFS     ((uint32_t)0x00020000)  /* Set when Rx good unicast frames counter reaches half the maximum value */
#define ETH_MMCRIR_RFAES     ((uint32_t)0x00000040)  /* Set when Rx alignment error counter reaches half the maximum value */
#define ETH_MMCRIR_RFCES     ((uint32_t)0x00000020)  /* Set when Rx crc error counter reaches half the maximum value */

/* Bit definition for Ethernet MMC Transmit Interrupt Register */
#define ETH_MMCTIR_TGFS      ((uint32_t)0x00200000)  /* Set when Tx good frame count counter reaches half the maximum value */
#define ETH_MMCTIR_TGFMSCS   ((uint32_t)0x00008000)  /* Set when Tx good multi col counter reaches half the maximum value */
#define ETH_MMCTIR_TGFSCS    ((uint32_t)0x00004000)  /* Set when Tx good single col counter reaches half the maximum value */

/* Bit definition for Ethernet MMC Receive Interrupt Mask Register */
#define ETH_MMCRIMR_RGUFM    ((uint32_t)0x00020000)  /* Mask the interrupt when Rx good unicast frames counter reaches half the maximum value */
#define ETH_MMCRIMR_RFAEM    ((uint32_t)0x00000040)  /* Mask the interrupt when when Rx alignment error counter reaches half the maximum value */
#define ETH_MMCRIMR_RFCEM    ((uint32_t)0x00000020)  /* Mask the interrupt when Rx crc error counter reaches half the maximum value */

/* Bit definition for Ethernet MMC Transmit Interrupt Mask Register */
#define ETH_MMCTIMR_TGFM     ((uint32_t)0x00200000)  /* Mask the interrupt when Tx good frame count counter reaches half the maximum value */
#define ETH_MMCTIMR_TGFMSCM  ((uint32_t)0x00008000)  /* Mask the interrupt when Tx good multi col counter reaches half the maximum value */
#define ETH_MMCTIMR_TGFSCM   ((uint32_t)0x00004000)  /* Mask the interrupt when Tx good single col counter reaches half the maximum value */

/* Bit definition for Ethernet MMC Transmitted Good Frames after Single Collision Counter Register */
#define ETH_MMCTGFSCCR_TGFSCC     ((uint32_t)0xFFFFFFFF)  /* Number of successfully transmitted frames after a single collision in Half-duplex mode. */

/* Bit definition for Ethernet MMC Transmitted Good Frames after More than a Single Collision Counter Register */
#define ETH_MMCTGFMSCCR_TGFMSCC   ((uint32_t)0xFFFFFFFF)  /* Number of successfully transmitted frames after more than a single collision in Half-duplex mode. */

/* Bit definition for Ethernet MMC Transmitted Good Frames Counter Register */
#define ETH_MMCTGFCR_TGFC    ((uint32_t)0xFFFFFFFF)  /* Number of good frames transmitted. */

/* Bit definition for Ethernet MMC Received Frames with CRC Error Counter Register */
#define ETH_MMCRFCECR_RFCEC  ((uint32_t)0xFFFFFFFF)  /* Number of frames received with CRC error. */

/* Bit definition for Ethernet MMC Received Frames with Alignement Error Counter Register */
#define ETH_MMCRFAECR_RFAEC  ((uint32_t)0xFFFFFFFF)  /* Number of frames received with alignment (dribble) error */

/* Bit definition for Ethernet MMC Received Good Unicast Frames Counter Register */
#define ETH_MMCRGUFCR_RGUFC  ((uint32_t)0xFFFFFFFF)  /* Number of good unicast frames received. */

/******************************************************************************/
/*               Ethernet PTP Registers bits definition                       */
/******************************************************************************/

/* Bit definition for Ethernet PTP Time Stamp Contol Register */
#define ETH_PTPTSCR_TSARU    ((uint32_t)0x00000020)  /* Addend register update */
#define ETH_PTPTSCR_TSITE    ((uint32_t)0x00000010)  /* Time stamp interrupt trigger enable */
#define ETH_PTPTSCR_TSSTU    ((uint32_t)0x00000008)  /* Time stamp update */
#define ETH_PTPTSCR_TSSTI    ((uint32_t)0x00000004)  /* Time stamp initialize */
#define ETH_PTPTSCR_TSFCU    ((uint32_t)0x00000002)  /* Time stamp fine or coarse update */
#define ETH_PTPTSCR_TSE      ((uint32_t)0x00000001)  /* Time stamp enable */

/* Bit definition for Ethernet PTP Sub-Second Increment Register */
#define ETH_PTPSSIR_STSSI    ((uint32_t)0x000000FF)  /* System time Sub-second increment value */

/* Bit definition for Ethernet PTP Time Stamp High Register */
#define ETH_PTPTSHR_STS      ((uint32_t)0xFFFFFFFF)  /* System Time second */

/* Bit definition for Ethernet PTP Time Stamp Low Register */
#define ETH_PTPTSLR_STPNS    ((uint32_t)0x80000000)  /* System Time Positive or negative time */
#define ETH_PTPTSLR_STSS     ((uint32_t)0x7FFFFFFF)  /* System Time sub-seconds */

/* Bit definition for Ethernet PTP Time Stamp High Update Register */
#define ETH_PTPTSHUR_TSUS    ((uint32_t)0xFFFFFFFF)  /* Time stamp update seconds */

/* Bit definition for Ethernet PTP Time Stamp Low Update Register */
#define ETH_PTPTSLUR_TSUPNS  ((uint32_t)0x80000000)  /* Time stamp update Positive or negative time */
#define ETH_PTPTSLUR_TSUSS   ((uint32_t)0x7FFFFFFF)  /* Time stamp update sub-seconds */

/* Bit definition for Ethernet PTP Time Stamp Addend Register */
#define ETH_PTPTSAR_TSA      ((uint32_t)0xFFFFFFFF)  /* Time stamp addend */

/* Bit definition for Ethernet PTP Target Time High Register */
#define ETH_PTPTTHR_TTSH     ((uint32_t)0xFFFFFFFF)  /* Target time stamp high */

/* Bit definition for Ethernet PTP Target Time Low Register */
#define ETH_PTPTTLR_TTSL     ((uint32_t)0xFFFFFFFF)  /* Target time stamp low */

/******************************************************************************/
/*                 Ethernet DMA Registers bits definition                     */
/******************************************************************************/

/* Bit definition for Ethernet DMA Bus Mode Register */
#define ETH_DMABMR_AAB       ((uint32_t)0x02000000)  /* Address-Aligned beats */
#define ETH_DMABMR_FPM        ((uint32_t)0x01000000)  /* 4xPBL mode */
#define ETH_DMABMR_USP       ((uint32_t)0x00800000)  /* Use separate PBL */
#define ETH_DMABMR_RDP       ((uint32_t)0x007E0000)  /* RxDMA PBL */
#define ETH_DMABMR_RDP_1Beat    ((uint32_t)0x00020000)  /* maximum number of beats to be transferred in one RxDMA transaction is 1 */
#define ETH_DMABMR_RDP_2Beat    ((uint32_t)0x00040000)  /* maximum number of beats to be transferred in one RxDMA transaction is 2 */
#define ETH_DMABMR_RDP_4Beat    ((uint32_t)0x00080000)  /* maximum number of beats to be transferred in one RxDMA transaction is 4 */
#define ETH_DMABMR_RDP_8Beat    ((uint32_t)0x00100000)  /* maximum number of beats to be transferred in one RxDMA transaction is 8 */
#define ETH_DMABMR_RDP_16Beat   ((uint32_t)0x00200000)  /* maximum number of beats to be transferred in one RxDMA transaction is 16 */
#define ETH_DMABMR_RDP_32Beat   ((uint32_t)0x00400000)  /* maximum number of beats to be transferred in one RxDMA transaction is 32 */                
#define ETH_DMABMR_RDP_4xPBL_4Beat   ((uint32_t)0x01020000)  /* maximum number of beats to be transferred in one RxDMA transaction is 4 */
#define ETH_DMABMR_RDP_4xPBL_8Beat   ((uint32_t)0x01040000)  /* maximum number of beats to be transferred in one RxDMA transaction is 8 */
#define ETH_DMABMR_RDP_4xPBL_16Beat  ((uint32_t)0x01080000)  /* maximum number of beats to be transferred in one RxDMA transaction is 16 */
#define ETH_DMABMR_RDP_4xPBL_32Beat  ((uint32_t)0x01100000)  /* maximum number of beats to be transferred in one RxDMA transaction is 32 */
#define ETH_DMABMR_RDP_4xPBL_64Beat  ((uint32_t)0x01200000)  /* maximum number of beats to be transferred in one RxDMA transaction is 64 */
#define ETH_DMABMR_RDP_4xPBL_128Beat ((uint32_t)0x01400000)  /* maximum number of beats to be transferred in one RxDMA transaction is 128 */  
#define ETH_DMABMR_FB        ((uint32_t)0x00010000)  /* Fixed Burst */
#define ETH_DMABMR_RTPR      ((uint32_t)0x0000C000)  /* Rx Tx priority ratio */
#define ETH_DMABMR_RTPR_1_1     ((uint32_t)0x00000000)  /* Rx Tx priority ratio */
#define ETH_DMABMR_RTPR_2_1     ((uint32_t)0x00004000)  /* Rx Tx priority ratio */
#define ETH_DMABMR_RTPR_3_1     ((uint32_t)0x00008000)  /* Rx Tx priority ratio */
#define ETH_DMABMR_RTPR_4_1     ((uint32_t)0x0000C000)  /* Rx Tx priority ratio */  
#define ETH_DMABMR_PBL    ((uint32_t)0x00003F00)  /* Programmable burst length */
#define ETH_DMABMR_PBL_1Beat    ((uint32_t)0x00000100)  /* maximum number of beats to be transferred in one TxDMA (or both) transaction is 1 */
#define ETH_DMABMR_PBL_2Beat    ((uint32_t)0x00000200)  /* maximum number of beats to be transferred in one TxDMA (or both) transaction is 2 */
#define ETH_DMABMR_PBL_4Beat    ((uint32_t)0x00000400)  /* maximum number of beats to be transferred in one TxDMA (or both) transaction is 4 */
#define ETH_DMABMR_PBL_8Beat    ((uint32_t)0x00000800)  /* maximum number of beats to be transferred in one TxDMA (or both) transaction is 8 */
#define ETH_DMABMR_PBL_16Beat   ((uint32_t)0x00001000)  /* maximum number of beats to be transferred in one TxDMA (or both) transaction is 16 */
#define ETH_DMABMR_PBL_32Beat   ((uint32_t)0x00002000)  /* maximum number of beats to be transferred in one TxDMA (or both) transaction is 32 */                
#define ETH_DMABMR_PBL_4xPBL_4Beat   ((uint32_t)0x01000100)  /* maximum number of beats to be transferred in one TxDMA (or both) transaction is 4 */
#define ETH_DMABMR_PBL_4xPBL_8Beat   ((uint32_t)0x01000200)  /* maximum number of beats to be transferred in one TxDMA (or both) transaction is 8 */
#define ETH_DMABMR_PBL_4xPBL_16Beat  ((uint32_t)0x01000400)  /* maximum number of beats to be transferred in one TxDMA (or both) transaction is 16 */
#define ETH_DMABMR_PBL_4xPBL_32Beat  ((uint32_t)0x01000800)  /* maximum number of beats to be transferred in one TxDMA (or both) transaction is 32 */
#define ETH_DMABMR_PBL_4xPBL_64Beat  ((uint32_t)0x01001000)  /* maximum number of beats to be transferred in one TxDMA (or both) transaction is 64 */
#define ETH_DMABMR_PBL_4xPBL_128Beat ((uint32_t)0x01002000)  /* maximum number of beats to be transferred in one TxDMA (or both) transaction is 128 */
#define ETH_DMABMR_DSL       ((uint32_t)0x0000007C)  /* Descriptor Skip Length */
#define ETH_DMABMR_DA        ((uint32_t)0x00000002)  /* DMA arbitration scheme */
#define ETH_DMABMR_SR        ((uint32_t)0x00000001)  /* Software reset */

/* Bit definition for Ethernet DMA Transmit Poll Demand Register */
#define ETH_DMATPDR_TPD      ((uint32_t)0xFFFFFFFF)  /* Transmit poll demand */

/* Bit definition for Ethernet DMA Receive Poll Demand Register */
#define ETH_DMARPDR_RPD      ((uint32_t)0xFFFFFFFF)  /* Receive poll demand  */

/* Bit definition for Ethernet DMA Receive Descriptor List Address Register */
#define ETH_DMARDLAR_SRL     ((uint32_t)0xFFFFFFFF)  /* Start of receive list */

/* Bit definition for Ethernet DMA Transmit Descriptor List Address Register */
#define ETH_DMATDLAR_STL     ((uint32_t)0xFFFFFFFF)  /* Start of transmit list */

/* Bit definition for Ethernet DMA Status Register */
#define ETH_DMASR_TSTS       ((uint32_t)0x20000000)  /* Time-stamp trigger status */
#define ETH_DMASR_PMTS       ((uint32_t)0x10000000)  /* PMT status */
#define ETH_DMASR_MMCS       ((uint32_t)0x08000000)  /* MMC status */
#define ETH_DMASR_EBS        ((uint32_t)0x03800000)  /* Error bits status */
  /* combination with EBS[2:0] for GetFlagStatus function */
#define ETH_DMASR_EBS_DescAccess      ((uint32_t)0x02000000)  /* Error bits 0-data buffer, 1-desc. access */
#define ETH_DMASR_EBS_ReadTransf      ((uint32_t)0x01000000)  /* Error bits 0-write trnsf, 1-read transfr */
#define ETH_DMASR_EBS_DataTransfTx    ((uint32_t)0x00800000)  /* Error bits 0-Rx DMA, 1-Tx DMA */
#define ETH_DMASR_TPS         ((uint32_t)0x00700000)  /* Transmit process state */
#define ETH_DMASR_TPS_Stopped         ((uint32_t)0x00000000)  /* Stopped - Reset or Stop Tx Command issued  */
#define ETH_DMASR_TPS_Fetching        ((uint32_t)0x00100000)  /* Running - fetching the Tx descriptor */
#define ETH_DMASR_TPS_Waiting         ((uint32_t)0x00200000)  /* Running - waiting for status */
#define ETH_DMASR_TPS_Reading         ((uint32_t)0x00300000)  /* Running - reading the data from host memory */
#define ETH_DMASR_TPS_Suspended       ((uint32_t)0x00600000)  /* Suspended - Tx Descriptor unavailabe */
#define ETH_DMASR_TPS_Closing         ((uint32_t)0x00700000)  /* Running - closing Rx descriptor */
#define ETH_DMASR_RPS         ((uint32_t)0x000E0000)  /* Receive process state */
#define ETH_DMASR_RPS_Stopped         ((uint32_t)0x00000000)  /* Stopped - Reset or Stop Rx Command issued */
#define ETH_DMASR_RPS_Fetching        ((uint32_t)0x00020000)  /* Running - fetching the Rx descriptor */
#define ETH_DMASR_RPS_Waiting         ((uint32_t)0x00060000)  /* Running - waiting for packet */
#define ETH_DMASR_RPS_Suspended       ((uint32_t)0x00080000)  /* Suspended - Rx Descriptor unavailable */
#define ETH_DMASR_RPS_Closing         ((uint32_t)0x000A0000)  /* Running - closing descriptor */
#define ETH_DMASR_RPS_Queuing         ((uint32_t)0x000E0000)  /* Running - queuing the recieve frame into host memory */
#define ETH_DMASR_NIS        ((uint32_t)0x00010000)  /* Normal interrupt summary */
#define ETH_DMASR_AIS        ((uint32_t)0x00008000)  /* Abnormal interrupt summary */
#define ETH_DMASR_ERS        ((uint32_t)0x00004000)  /* Early receive status */
#define ETH_DMASR_FBES       ((uint32_t)0x00002000)  /* Fatal bus error status */
#define ETH_DMASR_ETS        ((uint32_t)0x00000400)  /* Early transmit status */
#define ETH_DMASR_RWTS       ((uint32_t)0x00000200)  /* Receive watchdog timeout status */
#define ETH_DMASR_RPSS       ((uint32_t)0x00000100)  /* Receive process stopped status */
#define ETH_DMASR_RBUS       ((uint32_t)0x00000080)  /* Receive buffer unavailable status */
#define ETH_DMASR_RS         ((uint32_t)0x00000040)  /* Receive status */
#define ETH_DMASR_TUS        ((uint32_t)0x00000020)  /* Transmit underflow status */
#define ETH_DMASR_ROS        ((uint32_t)0x00000010)  /* Receive overflow status */
#define ETH_DMASR_TJTS       ((uint32_t)0x00000008)  /* Transmit jabber timeout status */
#define ETH_DMASR_TBUS       ((uint32_t)0x00000004)  /* Transmit buffer unavailable status */
#define ETH_DMASR_TPSS       ((uint32_t)0x00000002)  /* Transmit process stopped status */
#define ETH_DMASR_TS         ((uint32_t)0x00000001)  /* Transmit status */

/* Bit definition for Ethernet DMA Operation Mode Register */
#define ETH_DMAOMR_DTCEFD    ((uint32_t)0x04000000)  /* Disable Dropping of TCP/IP checksum error frames */
#define ETH_DMAOMR_RSF       ((uint32_t)0x02000000)  /* Receive store and forward */
#define ETH_DMAOMR_DFRF      ((uint32_t)0x01000000)  /* Disable flushing of received frames */
#define ETH_DMAOMR_TSF       ((uint32_t)0x00200000)  /* Transmit store and forward */
#define ETH_DMAOMR_FTF       ((uint32_t)0x00100000)  /* Flush transmit FIFO */
#define ETH_DMAOMR_TTC       ((uint32_t)0x0001C000)  /* Transmit threshold control */
#define ETH_DMAOMR_TTC_64Bytes       ((uint32_t)0x00000000)  /* threshold level of the MTL Transmit FIFO is 64 Bytes */
#define ETH_DMAOMR_TTC_128Bytes      ((uint32_t)0x00004000)  /* threshold level of the MTL Transmit FIFO is 128 Bytes */
#define ETH_DMAOMR_TTC_192Bytes      ((uint32_t)0x00008000)  /* threshold level of the MTL Transmit FIFO is 192 Bytes */
#define ETH_DMAOMR_TTC_256Bytes      ((uint32_t)0x0000C000)  /* threshold level of the MTL Transmit FIFO is 256 Bytes */
#define ETH_DMAOMR_TTC_40Bytes       ((uint32_t)0x00010000)  /* threshold level of the MTL Transmit FIFO is 40 Bytes */
#define ETH_DMAOMR_TTC_32Bytes       ((uint32_t)0x00014000)  /* threshold level of the MTL Transmit FIFO is 32 Bytes */
#define ETH_DMAOMR_TTC_24Bytes       ((uint32_t)0x00018000)  /* threshold level of the MTL Transmit FIFO is 24 Bytes */
#define ETH_DMAOMR_TTC_16Bytes       ((uint32_t)0x0001C000)  /* threshold level of the MTL Transmit FIFO is 16 Bytes */
#define ETH_DMAOMR_ST        ((uint32_t)0x00002000)  /* Start/stop transmission command */
#define ETH_DMAOMR_FEF       ((uint32_t)0x00000080)  /* Forward error frames */
#define ETH_DMAOMR_FUGF      ((uint32_t)0x00000040)  /* Forward undersized good frames */
#define ETH_DMAOMR_RTC       ((uint32_t)0x00000018)  /* receive threshold control */
#define ETH_DMAOMR_RTC_64Bytes       ((uint32_t)0x00000000)  /* threshold level of the MTL Receive FIFO is 64 Bytes */
#define ETH_DMAOMR_RTC_32Bytes       ((uint32_t)0x00000008)  /* threshold level of the MTL Receive FIFO is 32 Bytes */
#define ETH_DMAOMR_RTC_96Bytes       ((uint32_t)0x00000010)  /* threshold level of the MTL Receive FIFO is 96 Bytes */
#define ETH_DMAOMR_RTC_128Bytes      ((uint32_t)0x00000018)  /* threshold level of the MTL Receive FIFO is 128 Bytes */
#define ETH_DMAOMR_OSF       ((uint32_t)0x00000004)  /* operate on second frame */
#define ETH_DMAOMR_SR        ((uint32_t)0x00000002)  /* Start/stop receive */

/* Bit definition for Ethernet DMA Interrupt Enable Register */
#define ETH_DMAIER_NISE      ((uint32_t)0x00010000)  /* Normal interrupt summary enable */
#define ETH_DMAIER_AISE      ((uint32_t)0x00008000)  /* Abnormal interrupt summary enable */
#define ETH_DMAIER_ERIE      ((uint32_t)0x00004000)  /* Early receive interrupt enable */
#define ETH_DMAIER_FBEIE     ((uint32_t)0x00002000)  /* Fatal bus error interrupt enable */
#define ETH_DMAIER_ETIE      ((uint32_t)0x00000400)  /* Early transmit interrupt enable */
#define ETH_DMAIER_RWTIE     ((uint32_t)0x00000200)  /* Receive watchdog timeout interrupt enable */
#define ETH_DMAIER_RPSIE     ((uint32_t)0x00000100)  /* Receive process stopped interrupt enable */
#define ETH_DMAIER_RBUIE     ((uint32_t)0x00000080)  /* Receive buffer unavailable interrupt enable */
#define ETH_DMAIER_RIE       ((uint32_t)0x00000040)  /* Receive interrupt enable */
#define ETH_DMAIER_TUIE      ((uint32_t)0x00000020)  /* Transmit Underflow interrupt enable */
#define ETH_DMAIER_ROIE      ((uint32_t)0x00000010)  /* Receive Overflow interrupt enable */
#define ETH_DMAIER_TJTIE     ((uint32_t)0x00000008)  /* Transmit jabber timeout interrupt enable */
#define ETH_DMAIER_TBUIE     ((uint32_t)0x00000004)  /* Transmit buffer unavailable interrupt enable */
#define ETH_DMAIER_TPSIE     ((uint32_t)0x00000002)  /* Transmit process stopped interrupt enable */
#define ETH_DMAIER_TIE       ((uint32_t)0x00000001)  /* Transmit interrupt enable */

/* Bit definition for Ethernet DMA Missed Frame and Buffer Overflow Counter Register */
#define ETH_DMAMFBOCR_OFOC   ((uint32_t)0x10000000)  /* Overflow bit for FIFO overflow counter */
#define ETH_DMAMFBOCR_MFA    ((uint32_t)0x0FFE0000)  /* Number of frames missed by the application */
#define ETH_DMAMFBOCR_OMFC   ((uint32_t)0x00010000)  /* Overflow bit for missed frame counter */
#define ETH_DMAMFBOCR_MFC    ((uint32_t)0x0000FFFF)  /* Number of frames missed by the controller */

/* Bit definition for Ethernet DMA Current Host Transmit Descriptor Register */
#define ETH_DMACHTDR_HTDAP   ((uint32_t)0xFFFFFFFF)  /* Host transmit descriptor address pointer */

/* Bit definition for Ethernet DMA Current Host Receive Descriptor Register */
#define ETH_DMACHRDR_HRDAP   ((uint32_t)0xFFFFFFFF)  /* Host receive descriptor address pointer */

/* Bit definition for Ethernet DMA Current Host Transmit Buffer Address Register */
#define ETH_DMACHTBAR_HTBAP  ((uint32_t)0xFFFFFFFF)  /* Host transmit buffer address pointer */

/* Bit definition for Ethernet DMA Current Host Receive Buffer Address Register */
#define ETH_DMACHRBAR_HRBAP  ((uint32_t)0xFFFFFFFF)  /* Host receive buffer address pointer */

/******************************************************************************/
/*                                                                            */
/*                                 USB_OTG                                    */
/*                                                                            */
/******************************************************************************/

/********************  Bit definition forUSB_OTG_GOTGCTL register  ********************/
#define USB_OTG_GOTGCTL_SRQSCS               ((uint32_t)0x00000001)        /*!< Session request success */
#define USB_OTG_GOTGCTL_SRQ                  ((uint32_t)0x00000002)        /*!< Session request */
#define USB_OTG_GOTGCTL_HNGSCS               ((uint32_t)0x00000100)        /*!< Host negotiation success */
#define USB_OTG_GOTGCTL_HNPRQ                ((uint32_t)0x00000200)        /*!< HNP request */
#define USB_OTG_GOTGCTL_HSHNPEN              ((uint32_t)0x00000400)        /*!< Host set HNP enable */
#define USB_OTG_GOTGCTL_DHNPEN               ((uint32_t)0x00000800)        /*!< Device HNP enabled */
#define USB_OTG_GOTGCTL_CIDSTS               ((uint32_t)0x00010000)        /*!< Connector ID status */
#define USB_OTG_GOTGCTL_DBCT                 ((uint32_t)0x00020000)        /*!< Long/short debounce time */
#define USB_OTG_GOTGCTL_ASVLD                ((uint32_t)0x00040000)        /*!< A-session valid */
#define USB_OTG_GOTGCTL_BSVLD                ((uint32_t)0x00080000)        /*!< B-session valid */

/********************  Bit definition forUSB_OTG_HCFG register  ********************/

#define USB_OTG_HCFG_FSLSPCS                 ((uint32_t)0x00000003)        /*!< FS/LS PHY clock select */
#define USB_OTG_HCFG_FSLSPCS_0               ((uint32_t)0x00000001)        /*!<Bit 0 */
#define USB_OTG_HCFG_FSLSPCS_1               ((uint32_t)0x00000002)        /*!<Bit 1 */
#define USB_OTG_HCFG_FSLSS                   ((uint32_t)0x00000004)        /*!< FS- and LS-only support */

/********************  Bit definition forUSB_OTG_DCFG register  ********************/

#define USB_OTG_DCFG_DSPD                    ((uint32_t)0x00000003)        /*!< Device speed */
#define USB_OTG_DCFG_DSPD_0                  ((uint32_t)0x00000001)        /*!<Bit 0 */
#define USB_OTG_DCFG_DSPD_1                  ((uint32_t)0x00000002)        /*!<Bit 1 */
#define USB_OTG_DCFG_NZLSOHSK                ((uint32_t)0x00000004)        /*!< Nonzero-length status OUT handshake */

#define USB_OTG_DCFG_DAD                     ((uint32_t)0x000007F0)        /*!< Device address */
#define USB_OTG_DCFG_DAD_0                   ((uint32_t)0x00000010)        /*!<Bit 0 */
#define USB_OTG_DCFG_DAD_1                   ((uint32_t)0x00000020)        /*!<Bit 1 */
#define USB_OTG_DCFG_DAD_2                   ((uint32_t)0x00000040)        /*!<Bit 2 */
#define USB_OTG_DCFG_DAD_3                   ((uint32_t)0x00000080)        /*!<Bit 3 */
#define USB_OTG_DCFG_DAD_4                   ((uint32_t)0x00000100)        /*!<Bit 4 */
#define USB_OTG_DCFG_DAD_5                   ((uint32_t)0x00000200)        /*!<Bit 5 */
#define USB_OTG_DCFG_DAD_6                   ((uint32_t)0x00000400)        /*!<Bit 6 */

#define USB_OTG_DCFG_PFIVL                   ((uint32_t)0x00001800)        /*!< Periodic (micro)frame interval */
#define USB_OTG_DCFG_PFIVL_0                 ((uint32_t)0x00000800)        /*!<Bit 0 */
#define USB_OTG_DCFG_PFIVL_1                 ((uint32_t)0x00001000)        /*!<Bit 1 */

#define USB_OTG_DCFG_PERSCHIVL               ((uint32_t)0x03000000)        /*!< Periodic scheduling interval */
#define USB_OTG_DCFG_PERSCHIVL_0             ((uint32_t)0x01000000)        /*!<Bit 0 */
#define USB_OTG_DCFG_PERSCHIVL_1             ((uint32_t)0x02000000)        /*!<Bit 1 */

/********************  Bit definition forUSB_OTG_PCGCR register  ********************/
#define USB_OTG_PCGCR_STPPCLK                ((uint32_t)0x00000001)        /*!< Stop PHY clock */
#define USB_OTG_PCGCR_GATEHCLK               ((uint32_t)0x00000002)        /*!< Gate HCLK */
#define USB_OTG_PCGCR_PHYSUSP                ((uint32_t)0x00000010)        /*!< PHY suspended */

/********************  Bit definition forUSB_OTG_GOTGINT register  ******************/
#define USB_OTG_GOTGINT_SEDET                ((uint32_t)0x00000004)        /*!< Session end detected */
#define USB_OTG_GOTGINT_SRSSCHG              ((uint32_t)0x00000100)        /*!< Session request success status change */
#define USB_OTG_GOTGINT_HNSSCHG              ((uint32_t)0x00000200)        /*!< Host negotiation success status change */
#define USB_OTG_GOTGINT_HNGDET               ((uint32_t)0x00020000)        /*!< Host negotiation detected */
#define USB_OTG_GOTGINT_ADTOCHG              ((uint32_t)0x00040000)        /*!< A-device timeout change */
#define USB_OTG_GOTGINT_DBCDNE               ((uint32_t)0x00080000)        /*!< Debounce done */

/********************  Bit definition forUSB_OTG_DCTL register  ********************/
#define USB_OTG_DCTL_RWUSIG                  ((uint32_t)0x00000001)        /*!< Remote wakeup signaling */
#define USB_OTG_DCTL_SDIS                    ((uint32_t)0x00000002)        /*!< Soft disconnect */
#define USB_OTG_DCTL_GINSTS                  ((uint32_t)0x00000004)        /*!< Global IN NAK status */
#define USB_OTG_DCTL_GONSTS                  ((uint32_t)0x00000008)        /*!< Global OUT NAK status */

#define USB_OTG_DCTL_TCTL                    ((uint32_t)0x00000070)        /*!< Test control */
#define USB_OTG_DCTL_TCTL_0                  ((uint32_t)0x00000010)        /*!<Bit 0 */
#define USB_OTG_DCTL_TCTL_1                  ((uint32_t)0x00000020)        /*!<Bit 1 */
#define USB_OTG_DCTL_TCTL_2                  ((uint32_t)0x00000040)        /*!<Bit 2 */
#define USB_OTG_DCTL_SGINAK                  ((uint32_t)0x00000080)        /*!< Set global IN NAK */
#define USB_OTG_DCTL_CGINAK                  ((uint32_t)0x00000100)        /*!< Clear global IN NAK */
#define USB_OTG_DCTL_SGONAK                  ((uint32_t)0x00000200)        /*!< Set global OUT NAK */
#define USB_OTG_DCTL_CGONAK                  ((uint32_t)0x00000400)        /*!< Clear global OUT NAK */
#define USB_OTG_DCTL_POPRGDNE                ((uint32_t)0x00000800)        /*!< Power-on programming done */

/********************  Bit definition forUSB_OTG_HFIR register  ********************/
#define USB_OTG_HFIR_FRIVL                   ((uint32_t)0x0000FFFF)        /*!< Frame interval */

/********************  Bit definition forUSB_OTG_HFNUM register  ********************/
#define USB_OTG_HFNUM_FRNUM                  ((uint32_t)0x0000FFFF)        /*!< Frame number */
#define USB_OTG_HFNUM_FTREM                  ((uint32_t)0xFFFF0000)        /*!< Frame time remaining */

/********************  Bit definition forUSB_OTG_DSTS register  ********************/
#define USB_OTG_DSTS_SUSPSTS                 ((uint32_t)0x00000001)        /*!< Suspend status */

#define USB_OTG_DSTS_ENUMSPD                 ((uint32_t)0x00000006)        /*!< Enumerated speed */
#define USB_OTG_DSTS_ENUMSPD_0               ((uint32_t)0x00000002)        /*!<Bit 0 */
#define USB_OTG_DSTS_ENUMSPD_1               ((uint32_t)0x00000004)        /*!<Bit 1 */
#define USB_OTG_DSTS_EERR                    ((uint32_t)0x00000008)        /*!< Erratic error */
#define USB_OTG_DSTS_FNSOF                   ((uint32_t)0x003FFF00)        /*!< Frame number of the received SOF */

/********************  Bit definition forUSB_OTG_GAHBCFG register  *****************/
#define USB_OTG_GAHBCFG_GINT                    ((uint32_t)0x00000001)     /*!< Global interrupt mask */

#define USB_OTG_GAHBCFG_HBSTLEN              ((uint32_t)0x0000001E)        /*!< Burst length/type */
#define USB_OTG_GAHBCFG_HBSTLEN_0            ((uint32_t)0x00000002)        /*!<Bit 0 */
#define USB_OTG_GAHBCFG_HBSTLEN_1            ((uint32_t)0x00000004)        /*!<Bit 1 */
#define USB_OTG_GAHBCFG_HBSTLEN_2            ((uint32_t)0x00000008)        /*!<Bit 2 */
#define USB_OTG_GAHBCFG_HBSTLEN_3            ((uint32_t)0x00000010)        /*!<Bit 3 */
#define USB_OTG_GAHBCFG_DMAEN                ((uint32_t)0x00000020)        /*!< DMA enable */
#define USB_OTG_GAHBCFG_TXFELVL              ((uint32_t)0x00000080)        /*!< TxFIFO empty level */
#define USB_OTG_GAHBCFG_PTXFELVL             ((uint32_t)0x00000100)        /*!< Periodic TxFIFO empty level */

/********************  Bit definition forUSB_OTG_GUSBCFG register  *****************/

#define USB_OTG_GUSBCFG_TOCAL                ((uint32_t)0x00000007)        /*!< FS timeout calibration */
#define USB_OTG_GUSBCFG_TOCAL_0              ((uint32_t)0x00000001)        /*!<Bit 0 */
#define USB_OTG_GUSBCFG_TOCAL_1              ((uint32_t)0x00000002)        /*!<Bit 1 */
#define USB_OTG_GUSBCFG_TOCAL_2              ((uint32_t)0x00000004)        /*!<Bit 2 */
#define USB_OTG_GUSBCFG_PHYSEL               ((uint32_t)0x00000040)        /*!< USB 2.0 high-speed ULPI PHY or USB 1.1 full-speed serial transceiver select */
#define USB_OTG_GUSBCFG_SRPCAP               ((uint32_t)0x00000100)        /*!< SRP-capable */
#define USB_OTG_GUSBCFG_HNPCAP               ((uint32_t)0x00000200)        /*!< HNP-capable */

#define USB_OTG_GUSBCFG_TRDT                 ((uint32_t)0x00003C00)        /*!< USB turnaround time */
#define USB_OTG_GUSBCFG_TRDT_0               ((uint32_t)0x00000400)        /*!<Bit 0 */
#define USB_OTG_GUSBCFG_TRDT_1               ((uint32_t)0x00000800)        /*!<Bit 1 */
#define USB_OTG_GUSBCFG_TRDT_2               ((uint32_t)0x00001000)        /*!<Bit 2 */
#define USB_OTG_GUSBCFG_TRDT_3               ((uint32_t)0x00002000)        /*!<Bit 3 */
#define USB_OTG_GUSBCFG_PHYLPCS              ((uint32_t)0x00008000)        /*!< PHY Low-power clock select */
#define USB_OTG_GUSBCFG_ULPIFSLS             ((uint32_t)0x00020000)        /*!< ULPI FS/LS select */
#define USB_OTG_GUSBCFG_ULPIAR               ((uint32_t)0x00040000)        /*!< ULPI Auto-resume */
#define USB_OTG_GUSBCFG_ULPICSM              ((uint32_t)0x00080000)        /*!< ULPI Clock SuspendM */
#define USB_OTG_GUSBCFG_ULPIEVBUSD           ((uint32_t)0x00100000)        /*!< ULPI External VBUS Drive */
#define USB_OTG_GUSBCFG_ULPIEVBUSI           ((uint32_t)0x00200000)        /*!< ULPI external VBUS indicator */
#define USB_OTG_GUSBCFG_TSDPS                ((uint32_t)0x00400000)        /*!< TermSel DLine pulsing selection */
#define USB_OTG_GUSBCFG_PCCI                 ((uint32_t)0x00800000)        /*!< Indicator complement */
#define USB_OTG_GUSBCFG_PTCI                 ((uint32_t)0x01000000)        /*!< Indicator pass through */
#define USB_OTG_GUSBCFG_ULPIIPD              ((uint32_t)0x02000000)        /*!< ULPI interface protect disable */
#define USB_OTG_GUSBCFG_FHMOD                ((uint32_t)0x20000000)        /*!< Forced host mode */
#define USB_OTG_GUSBCFG_FDMOD                ((uint32_t)0x40000000)        /*!< Forced peripheral mode */
#define USB_OTG_GUSBCFG_CTXPKT               ((uint32_t)0x80000000)        /*!< Corrupt Tx packet */

/********************  Bit definition forUSB_OTG_GRSTCTL register  *****************/
#define USB_OTG_GRSTCTL_CSRST                ((uint32_t)0x00000001)        /*!< Core soft reset */
#define USB_OTG_GRSTCTL_HSRST                ((uint32_t)0x00000002)        /*!< HCLK soft reset */
#define USB_OTG_GRSTCTL_FCRST                ((uint32_t)0x00000004)        /*!< Host frame counter reset */
#define USB_OTG_GRSTCTL_RXFFLSH              ((uint32_t)0x00000010)        /*!< RxFIFO flush */
#define USB_OTG_GRSTCTL_TXFFLSH              ((uint32_t)0x00000020)        /*!< TxFIFO flush */

#define USB_OTG_GRSTCTL_TXFNUM               ((uint32_t)0x000007C0)        /*!< TxFIFO number */
#define USB_OTG_GRSTCTL_TXFNUM_0             ((uint32_t)0x00000040)        /*!<Bit 0 */
#define USB_OTG_GRSTCTL_TXFNUM_1             ((uint32_t)0x00000080)        /*!<Bit 1 */
#define USB_OTG_GRSTCTL_TXFNUM_2             ((uint32_t)0x00000100)        /*!<Bit 2 */
#define USB_OTG_GRSTCTL_TXFNUM_3             ((uint32_t)0x00000200)        /*!<Bit 3 */
#define USB_OTG_GRSTCTL_TXFNUM_4             ((uint32_t)0x00000400)        /*!<Bit 4 */
#define USB_OTG_GRSTCTL_DMAREQ               ((uint32_t)0x40000000)        /*!< DMA request signal */
#define USB_OTG_GRSTCTL_AHBIDL               ((uint32_t)0x80000000)        /*!< AHB master idle */

/********************  Bit definition forUSB_OTG_DIEPMSK register  *****************/
#define USB_OTG_DIEPMSK_XFRCM                ((uint32_t)0x00000001)        /*!< Transfer completed interrupt mask */
#define USB_OTG_DIEPMSK_EPDM                 ((uint32_t)0x00000002)        /*!< Endpoint disabled interrupt mask */
#define USB_OTG_DIEPMSK_TOM                  ((uint32_t)0x00000008)        /*!< Timeout condition mask (nonisochronous endpoints) */
#define USB_OTG_DIEPMSK_ITTXFEMSK            ((uint32_t)0x00000010)        /*!< IN token received when TxFIFO empty mask */
#define USB_OTG_DIEPMSK_INEPNMM              ((uint32_t)0x00000020)        /*!< IN token received with EP mismatch mask */
#define USB_OTG_DIEPMSK_INEPNEM              ((uint32_t)0x00000040)        /*!< IN endpoint NAK effective mask */
#define USB_OTG_DIEPMSK_TXFURM               ((uint32_t)0x00000100)        /*!< FIFO underrun mask */
#define USB_OTG_DIEPMSK_BIM                  ((uint32_t)0x00000200)        /*!< BNA interrupt mask */

/********************  Bit definition forUSB_OTG_HPTXSTS register  *****************/
#define USB_OTG_HPTXSTS_PTXFSAVL             ((uint32_t)0x0000FFFF)        /*!< Periodic transmit data FIFO space available */
                                          
#define USB_OTG_HPTXSTS_PTXQSAV              ((uint32_t)0x00FF0000)        /*!< Periodic transmit request queue space available */
#define USB_OTG_HPTXSTS_PTXQSAV_0            ((uint32_t)0x00010000)        /*!<Bit 0 */
#define USB_OTG_HPTXSTS_PTXQSAV_1            ((uint32_t)0x00020000)        /*!<Bit 1 */
#define USB_OTG_HPTXSTS_PTXQSAV_2            ((uint32_t)0x00040000)        /*!<Bit 2 */
#define USB_OTG_HPTXSTS_PTXQSAV_3            ((uint32_t)0x00080000)        /*!<Bit 3 */
#define USB_OTG_HPTXSTS_PTXQSAV_4            ((uint32_t)0x00100000)        /*!<Bit 4 */
#define USB_OTG_HPTXSTS_PTXQSAV_5            ((uint32_t)0x00200000)        /*!<Bit 5 */
#define USB_OTG_HPTXSTS_PTXQSAV_6            ((uint32_t)0x00400000)        /*!<Bit 6 */
#define USB_OTG_HPTXSTS_PTXQSAV_7            ((uint32_t)0x00800000)        /*!<Bit 7 */

#define USB_OTG_HPTXSTS_PTXQTOP              ((uint32_t)0xFF000000)        /*!< Top of the periodic transmit request queue */
#define USB_OTG_HPTXSTS_PTXQTOP_0            ((uint32_t)0x01000000)        /*!<Bit 0 */
#define USB_OTG_HPTXSTS_PTXQTOP_1            ((uint32_t)0x02000000)        /*!<Bit 1 */
#define USB_OTG_HPTXSTS_PTXQTOP_2            ((uint32_t)0x04000000)        /*!<Bit 2 */
#define USB_OTG_HPTXSTS_PTXQTOP_3            ((uint32_t)0x08000000)        /*!<Bit 3 */
#define USB_OTG_HPTXSTS_PTXQTOP_4            ((uint32_t)0x10000000)        /*!<Bit 4 */
#define USB_OTG_HPTXSTS_PTXQTOP_5            ((uint32_t)0x20000000)        /*!<Bit 5 */
#define USB_OTG_HPTXSTS_PTXQTOP_6            ((uint32_t)0x40000000)        /*!<Bit 6 */
#define USB_OTG_HPTXSTS_PTXQTOP_7            ((uint32_t)0x80000000)        /*!<Bit 7 */

/********************  Bit definition forUSB_OTG_HAINT register  *******************/
#define USB_OTG_HAINT_HAINT                  ((uint32_t)0x0000FFFF)        /*!< Channel interrupts */

/********************  Bit definition forUSB_OTG_DOEPMSK register  *****************/
#define USB_OTG_DOEPMSK_XFRCM                ((uint32_t)0x00000001)        /*!< Transfer completed interrupt mask */
#define USB_OTG_DOEPMSK_EPDM                 ((uint32_t)0x00000002)        /*!< Endpoint disabled interrupt mask */
#define USB_OTG_DOEPMSK_STUPM                ((uint32_t)0x00000008)        /*!< SETUP phase done mask */
#define USB_OTG_DOEPMSK_OTEPDM               ((uint32_t)0x00000010)        /*!< OUT token received when endpoint disabled mask */
#define USB_OTG_DOEPMSK_B2BSTUP              ((uint32_t)0x00000040)        /*!< Back-to-back SETUP packets received mask */
#define USB_OTG_DOEPMSK_OPEM                 ((uint32_t)0x00000100)        /*!< OUT packet error mask */
#define USB_OTG_DOEPMSK_BOIM                 ((uint32_t)0x00000200)        /*!< BNA interrupt mask */

/********************  Bit definition forUSB_OTG_GINTSTS register  *****************/
#define USB_OTG_GINTSTS_CMOD                 ((uint32_t)0x00000001)        /*!< Current mode of operation */
#define USB_OTG_GINTSTS_MMIS                 ((uint32_t)0x00000002)        /*!< Mode mismatch interrupt */
#define USB_OTG_GINTSTS_OTGINT               ((uint32_t)0x00000004)        /*!< OTG interrupt */
#define USB_OTG_GINTSTS_SOF                  ((uint32_t)0x00000008)        /*!< Start of frame */
#define USB_OTG_GINTSTS_RXFLVL               ((uint32_t)0x00000010)        /*!< RxFIFO nonempty */
#define USB_OTG_GINTSTS_NPTXFE               ((uint32_t)0x00000020)        /*!< Nonperiodic TxFIFO empty */
#define USB_OTG_GINTSTS_GINAKEFF             ((uint32_t)0x00000040)        /*!< Global IN nonperiodic NAK effective */
#define USB_OTG_GINTSTS_BOUTNAKEFF           ((uint32_t)0x00000080)        /*!< Global OUT NAK effective */
#define USB_OTG_GINTSTS_ESUSP                ((uint32_t)0x00000400)        /*!< Early suspend */
#define USB_OTG_GINTSTS_USBSUSP              ((uint32_t)0x00000800)        /*!< USB suspend */
#define USB_OTG_GINTSTS_USBRST               ((uint32_t)0x00001000)        /*!< USB reset */
#define USB_OTG_GINTSTS_ENUMDNE              ((uint32_t)0x00002000)        /*!< Enumeration done */
#define USB_OTG_GINTSTS_ISOODRP              ((uint32_t)0x00004000)        /*!< Isochronous OUT packet dropped interrupt */
#define USB_OTG_GINTSTS_EOPF                 ((uint32_t)0x00008000)        /*!< End of periodic frame interrupt */
#define USB_OTG_GINTSTS_IEPINT               ((uint32_t)0x00040000)        /*!< IN endpoint interrupt */
#define USB_OTG_GINTSTS_OEPINT               ((uint32_t)0x00080000)        /*!< OUT endpoint interrupt */
#define USB_OTG_GINTSTS_IISOIXFR             ((uint32_t)0x00100000)        /*!< Incomplete isochronous IN transfer */
#define USB_OTG_GINTSTS_PXFR_INCOMPISOOUT    ((uint32_t)0x00200000)        /*!< Incomplete periodic transfer */
#define USB_OTG_GINTSTS_DATAFSUSP            ((uint32_t)0x00400000)        /*!< Data fetch suspended */
#define USB_OTG_GINTSTS_HPRTINT              ((uint32_t)0x01000000)        /*!< Host port interrupt */
#define USB_OTG_GINTSTS_HCINT                ((uint32_t)0x02000000)        /*!< Host channels interrupt */
#define USB_OTG_GINTSTS_PTXFE                ((uint32_t)0x04000000)        /*!< Periodic TxFIFO empty */
#define USB_OTG_GINTSTS_CIDSCHG              ((uint32_t)0x10000000)        /*!< Connector ID status change */
#define USB_OTG_GINTSTS_DISCINT              ((uint32_t)0x20000000)        /*!< Disconnect detected interrupt */
#define USB_OTG_GINTSTS_SRQINT               ((uint32_t)0x40000000)        /*!< Session request/new session detected interrupt */
#define USB_OTG_GINTSTS_WKUINT               ((uint32_t)0x80000000)        /*!< Resume/remote wakeup detected interrupt */

/********************  Bit definition forUSB_OTG_GINTMSK register  *****************/
#define USB_OTG_GINTMSK_MMISM                ((uint32_t)0x00000002)        /*!< Mode mismatch interrupt mask */
#define USB_OTG_GINTMSK_OTGINT               ((uint32_t)0x00000004)        /*!< OTG interrupt mask */
#define USB_OTG_GINTMSK_SOFM                 ((uint32_t)0x00000008)        /*!< Start of frame mask */
#define USB_OTG_GINTMSK_RXFLVLM              ((uint32_t)0x00000010)        /*!< Receive FIFO nonempty mask */
#define USB_OTG_GINTMSK_NPTXFEM              ((uint32_t)0x00000020)        /*!< Nonperiodic TxFIFO empty mask */
#define USB_OTG_GINTMSK_GINAKEFFM            ((uint32_t)0x00000040)        /*!< Global nonperiodic IN NAK effective mask */
#define USB_OTG_GINTMSK_GONAKEFFM            ((uint32_t)0x00000080)        /*!< Global OUT NAK effective mask */
#define USB_OTG_GINTMSK_ESUSPM               ((uint32_t)0x00000400)        /*!< Early suspend mask */
#define USB_OTG_GINTMSK_USBSUSPM             ((uint32_t)0x00000800)        /*!< USB suspend mask */
#define USB_OTG_GINTMSK_USBRST               ((uint32_t)0x00001000)        /*!< USB reset mask */
#define USB_OTG_GINTMSK_ENUMDNEM             ((uint32_t)0x00002000)        /*!< Enumeration done mask */
#define USB_OTG_GINTMSK_ISOODRPM             ((uint32_t)0x00004000)        /*!< Isochronous OUT packet dropped interrupt mask */
#define USB_OTG_GINTMSK_EOPFM                ((uint32_t)0x00008000)        /*!< End of periodic frame interrupt mask */
#define USB_OTG_GINTMSK_EPMISM               ((uint32_t)0x00020000)        /*!< Endpoint mismatch interrupt mask */
#define USB_OTG_GINTMSK_IEPINT               ((uint32_t)0x00040000)        /*!< IN endpoints interrupt mask */
#define USB_OTG_GINTMSK_OEPINT               ((uint32_t)0x00080000)        /*!< OUT endpoints interrupt mask */
#define USB_OTG_GINTMSK_IISOIXFRM            ((uint32_t)0x00100000)        /*!< Incomplete isochronous IN transfer mask */
#define USB_OTG_GINTMSK_PXFRM_IISOOXFRM      ((uint32_t)0x00200000)        /*!< Incomplete periodic transfer mask */
#define USB_OTG_GINTMSK_FSUSPM               ((uint32_t)0x00400000)        /*!< Data fetch suspended mask */
#define USB_OTG_GINTMSK_PRTIM                ((uint32_t)0x01000000)        /*!< Host port interrupt mask */
#define USB_OTG_GINTMSK_HCIM                 ((uint32_t)0x02000000)        /*!< Host channels interrupt mask */
#define USB_OTG_GINTMSK_PTXFEM               ((uint32_t)0x04000000)        /*!< Periodic TxFIFO empty mask */
#define USB_OTG_GINTMSK_CIDSCHGM             ((uint32_t)0x10000000)        /*!< Connector ID status change mask */
#define USB_OTG_GINTMSK_DISCINT              ((uint32_t)0x20000000)        /*!< Disconnect detected interrupt mask */
#define USB_OTG_GINTMSK_SRQIM                ((uint32_t)0x40000000)        /*!< Session request/new session detected interrupt mask */
#define USB_OTG_GINTMSK_WUIM                 ((uint32_t)0x80000000)        /*!< Resume/remote wakeup detected interrupt mask */

/********************  Bit definition forUSB_OTG_DAINT register  *******************/
#define USB_OTG_DAINT_IEPINT                 ((uint32_t)0x0000FFFF)        /*!< IN endpoint interrupt bits */
#define USB_OTG_DAINT_OEPINT                 ((uint32_t)0xFFFF0000)        /*!< OUT endpoint interrupt bits */

/********************  Bit definition forUSB_OTG_HAINTMSK register  ****************/
#define USB_OTG_HAINTMSK_HAINTM              ((uint32_t)0x0000FFFF)        /*!< Channel interrupt mask */

/********************  Bit definition for USB_OTG_GRXSTSP register  ****************/
#define USB_OTG_GRXSTSP_EPNUM                ((uint32_t)0x0000000F)        /*!< IN EP interrupt mask bits */
#define USB_OTG_GRXSTSP_BCNT                 ((uint32_t)0x00007FF0)        /*!< OUT EP interrupt mask bits */
#define USB_OTG_GRXSTSP_DPID                 ((uint32_t)0x00018000)        /*!< OUT EP interrupt mask bits */
#define USB_OTG_GRXSTSP_PKTSTS               ((uint32_t)0x001E0000)        /*!< OUT EP interrupt mask bits */

/********************  Bit definition forUSB_OTG_DAINTMSK register  ****************/
#define USB_OTG_DAINTMSK_IEPM                ((uint32_t)0x0000FFFF)        /*!< IN EP interrupt mask bits */
#define USB_OTG_DAINTMSK_OEPM                ((uint32_t)0xFFFF0000)        /*!< OUT EP interrupt mask bits */

/********************  Bit definition for OTG register  ****************************/
#define USB_OTG_CHNUM                        ((uint32_t)0x0000000F)        /*!< Channel number */
#define USB_OTG_CHNUM_0                      ((uint32_t)0x00000001)        /*!<Bit 0 */
#define USB_OTG_CHNUM_1                      ((uint32_t)0x00000002)        /*!<Bit 1 */
#define USB_OTG_CHNUM_2                      ((uint32_t)0x00000004)        /*!<Bit 2 */
#define USB_OTG_CHNUM_3                      ((uint32_t)0x00000008)        /*!<Bit 3 */
#define USB_OTG_BCNT                         ((uint32_t)0x00007FF0)        /*!< Byte count */

#define USB_OTG_DPID                         ((uint32_t)0x00018000)        /*!< Data PID */
#define USB_OTG_DPID_0                       ((uint32_t)0x00008000)        /*!<Bit 0 */
#define USB_OTG_DPID_1                       ((uint32_t)0x00010000)        /*!<Bit 1 */

#define USB_OTG_PKTSTS                       ((uint32_t)0x001E0000)        /*!< Packet status */
#define USB_OTG_PKTSTS_0                     ((uint32_t)0x00020000)        /*!<Bit 0 */
#define USB_OTG_PKTSTS_1                     ((uint32_t)0x00040000)        /*!<Bit 1 */
#define USB_OTG_PKTSTS_2                     ((uint32_t)0x00080000)        /*!<Bit 2 */
#define USB_OTG_PKTSTS_3                     ((uint32_t)0x00100000)        /*!<Bit 3 */

#define USB_OTG_EPNUM                        ((uint32_t)0x0000000F)        /*!< Endpoint number */
#define USB_OTG_EPNUM_0                      ((uint32_t)0x00000001)        /*!<Bit 0 */
#define USB_OTG_EPNUM_1                      ((uint32_t)0x00000002)        /*!<Bit 1 */
#define USB_OTG_EPNUM_2                      ((uint32_t)0x00000004)        /*!<Bit 2 */
#define USB_OTG_EPNUM_3                      ((uint32_t)0x00000008)        /*!<Bit 3 */

#define USB_OTG_FRMNUM                       ((uint32_t)0x01E00000)        /*!< Frame number */
#define USB_OTG_FRMNUM_0                     ((uint32_t)0x00200000)        /*!<Bit 0 */
#define USB_OTG_FRMNUM_1                     ((uint32_t)0x00400000)        /*!<Bit 1 */
#define USB_OTG_FRMNUM_2                     ((uint32_t)0x00800000)        /*!<Bit 2 */
#define USB_OTG_FRMNUM_3                     ((uint32_t)0x01000000)        /*!<Bit 3 */

/********************  Bit definition for OTG register  ****************************/
#define USB_OTG_CHNUM                        ((uint32_t)0x0000000F)        /*!< Channel number */
#define USB_OTG_CHNUM_0                      ((uint32_t)0x00000001)        /*!<Bit 0 */
#define USB_OTG_CHNUM_1                      ((uint32_t)0x00000002)        /*!<Bit 1 */
#define USB_OTG_CHNUM_2                      ((uint32_t)0x00000004)        /*!<Bit 2 */
#define USB_OTG_CHNUM_3                      ((uint32_t)0x00000008)        /*!<Bit 3 */
#define USB_OTG_BCNT                         ((uint32_t)0x00007FF0)        /*!< Byte count */

#define USB_OTG_DPID                         ((uint32_t)0x00018000)        /*!< Data PID */
#define USB_OTG_DPID_0                       ((uint32_t)0x00008000)        /*!<Bit 0 */
#define USB_OTG_DPID_1                       ((uint32_t)0x00010000)        /*!<Bit 1 */

#define USB_OTG_PKTSTS                       ((uint32_t)0x001E0000)        /*!< Packet status */
#define USB_OTG_PKTSTS_0                     ((uint32_t)0x00020000)        /*!<Bit 0 */
#define USB_OTG_PKTSTS_1                     ((uint32_t)0x00040000)        /*!<Bit 1 */
#define USB_OTG_PKTSTS_2                     ((uint32_t)0x00080000)        /*!<Bit 2 */
#define USB_OTG_PKTSTS_3                     ((uint32_t)0x00100000)        /*!<Bit 3 */

#define USB_OTG_EPNUM                        ((uint32_t)0x0000000F)        /*!< Endpoint number */
#define USB_OTG_EPNUM_0                      ((uint32_t)0x00000001)        /*!<Bit 0 */
#define USB_OTG_EPNUM_1                      ((uint32_t)0x00000002)        /*!<Bit 1 */
#define USB_OTG_EPNUM_2                      ((uint32_t)0x00000004)        /*!<Bit 2 */
#define USB_OTG_EPNUM_3                      ((uint32_t)0x00000008)        /*!<Bit 3 */

#define USB_OTG_FRMNUM                       ((uint32_t)0x01E00000)        /*!< Frame number */
#define USB_OTG_FRMNUM_0                     ((uint32_t)0x00200000)        /*!<Bit 0 */
#define USB_OTG_FRMNUM_1                     ((uint32_t)0x00400000)        /*!<Bit 1 */
#define USB_OTG_FRMNUM_2                     ((uint32_t)0x00800000)        /*!<Bit 2 */
#define USB_OTG_FRMNUM_3                     ((uint32_t)0x01000000)        /*!<Bit 3 */

/********************  Bit definition forUSB_OTG_GRXFSIZ register  *****************/
#define USB_OTG_GRXFSIZ_RXFD                 ((uint32_t)0x0000FFFF)        /*!< RxFIFO depth */

/********************  Bit definition forUSB_OTG_DVBUSDIS register  ****************/
#define USB_OTG_DVBUSDIS_VBUSDT              ((uint32_t)0x0000FFFF)        /*!< Device VBUS discharge time */

/********************  Bit definition for OTG register  ****************************/
#define USB_OTG_NPTXFSA                      ((uint32_t)0x0000FFFF)        /*!< Nonperiodic transmit RAM start address */
#define USB_OTG_NPTXFD                       ((uint32_t)0xFFFF0000)        /*!< Nonperiodic TxFIFO depth */
#define USB_OTG_TX0FSA                       ((uint32_t)0x0000FFFF)        /*!< Endpoint 0 transmit RAM start address */
#define USB_OTG_TX0FD                        ((uint32_t)0xFFFF0000)        /*!< Endpoint 0 TxFIFO depth */

/********************  Bit definition forUSB_OTG_DVBUSPULSE register  **************/
#define USB_OTG_DVBUSPULSE_DVBUSP            ((uint32_t)0x00000FFF)        /*!< Device VBUS pulsing time */

/********************  Bit definition forUSB_OTG_GNPTXSTS register  ****************/
#define USB_OTG_GNPTXSTS_NPTXFSAV            ((uint32_t)0x0000FFFF)        /*!< Nonperiodic TxFIFO space available */

#define USB_OTG_GNPTXSTS_NPTQXSAV            ((uint32_t)0x00FF0000)        /*!< Nonperiodic transmit request queue space available */
#define USB_OTG_GNPTXSTS_NPTQXSAV_0          ((uint32_t)0x00010000)        /*!<Bit 0 */
#define USB_OTG_GNPTXSTS_NPTQXSAV_1          ((uint32_t)0x00020000)        /*!<Bit 1 */
#define USB_OTG_GNPTXSTS_NPTQXSAV_2          ((uint32_t)0x00040000)        /*!<Bit 2 */
#define USB_OTG_GNPTXSTS_NPTQXSAV_3          ((uint32_t)0x00080000)        /*!<Bit 3 */
#define USB_OTG_GNPTXSTS_NPTQXSAV_4          ((uint32_t)0x00100000)        /*!<Bit 4 */
#define USB_OTG_GNPTXSTS_NPTQXSAV_5          ((uint32_t)0x00200000)        /*!<Bit 5 */
#define USB_OTG_GNPTXSTS_NPTQXSAV_6          ((uint32_t)0x00400000)        /*!<Bit 6 */
#define USB_OTG_GNPTXSTS_NPTQXSAV_7          ((uint32_t)0x00800000)        /*!<Bit 7 */

#define USB_OTG_GNPTXSTS_NPTXQTOP            ((uint32_t)0x7F000000)        /*!< Top of the nonperiodic transmit request queue */
#define USB_OTG_GNPTXSTS_NPTXQTOP_0          ((uint32_t)0x01000000)        /*!<Bit 0 */
#define USB_OTG_GNPTXSTS_NPTXQTOP_1          ((uint32_t)0x02000000)        /*!<Bit 1 */
#define USB_OTG_GNPTXSTS_NPTXQTOP_2          ((uint32_t)0x04000000)        /*!<Bit 2 */
#define USB_OTG_GNPTXSTS_NPTXQTOP_3          ((uint32_t)0x08000000)        /*!<Bit 3 */
#define USB_OTG_GNPTXSTS_NPTXQTOP_4          ((uint32_t)0x10000000)        /*!<Bit 4 */
#define USB_OTG_GNPTXSTS_NPTXQTOP_5          ((uint32_t)0x20000000)        /*!<Bit 5 */
#define USB_OTG_GNPTXSTS_NPTXQTOP_6          ((uint32_t)0x40000000)        /*!<Bit 6 */

/********************  Bit definition forUSB_OTG_DTHRCTL register  *****************/
#define USB_OTG_DTHRCTL_NONISOTHREN          ((uint32_t)0x00000001)        /*!< Nonisochronous IN endpoints threshold enable */
#define USB_OTG_DTHRCTL_ISOTHREN             ((uint32_t)0x00000002)        /*!< ISO IN endpoint threshold enable */

#define USB_OTG_DTHRCTL_TXTHRLEN             ((uint32_t)0x000007FC)        /*!< Transmit threshold length */
#define USB_OTG_DTHRCTL_TXTHRLEN_0           ((uint32_t)0x00000004)        /*!<Bit 0 */
#define USB_OTG_DTHRCTL_TXTHRLEN_1           ((uint32_t)0x00000008)        /*!<Bit 1 */
#define USB_OTG_DTHRCTL_TXTHRLEN_2           ((uint32_t)0x00000010)        /*!<Bit 2 */
#define USB_OTG_DTHRCTL_TXTHRLEN_3           ((uint32_t)0x00000020)        /*!<Bit 3 */
#define USB_OTG_DTHRCTL_TXTHRLEN_4           ((uint32_t)0x00000040)        /*!<Bit 4 */
#define USB_OTG_DTHRCTL_TXTHRLEN_5           ((uint32_t)0x00000080)        /*!<Bit 5 */
#define USB_OTG_DTHRCTL_TXTHRLEN_6           ((uint32_t)0x00000100)        /*!<Bit 6 */
#define USB_OTG_DTHRCTL_TXTHRLEN_7           ((uint32_t)0x00000200)        /*!<Bit 7 */
#define USB_OTG_DTHRCTL_TXTHRLEN_8           ((uint32_t)0x00000400)        /*!<Bit 8 */
#define USB_OTG_DTHRCTL_RXTHREN              ((uint32_t)0x00010000)        /*!< Receive threshold enable */

#define USB_OTG_DTHRCTL_RXTHRLEN             ((uint32_t)0x03FE0000)        /*!< Receive threshold length */
#define USB_OTG_DTHRCTL_RXTHRLEN_0           ((uint32_t)0x00020000)        /*!<Bit 0 */
#define USB_OTG_DTHRCTL_RXTHRLEN_1           ((uint32_t)0x00040000)        /*!<Bit 1 */
#define USB_OTG_DTHRCTL_RXTHRLEN_2           ((uint32_t)0x00080000)        /*!<Bit 2 */
#define USB_OTG_DTHRCTL_RXTHRLEN_3           ((uint32_t)0x00100000)        /*!<Bit 3 */
#define USB_OTG_DTHRCTL_RXTHRLEN_4           ((uint32_t)0x00200000)        /*!<Bit 4 */
#define USB_OTG_DTHRCTL_RXTHRLEN_5           ((uint32_t)0x00400000)        /*!<Bit 5 */
#define USB_OTG_DTHRCTL_RXTHRLEN_6           ((uint32_t)0x00800000)        /*!<Bit 6 */
#define USB_OTG_DTHRCTL_RXTHRLEN_7           ((uint32_t)0x01000000)        /*!<Bit 7 */
#define USB_OTG_DTHRCTL_RXTHRLEN_8           ((uint32_t)0x02000000)        /*!<Bit 8 */
#define USB_OTG_DTHRCTL_ARPEN                ((uint32_t)0x08000000)        /*!< Arbiter parking enable */

/********************  Bit definition forUSB_OTG_DIEPEMPMSK register  **************/
#define USB_OTG_DIEPEMPMSK_INEPTXFEM         ((uint32_t)0x0000FFFF)        /*!< IN EP Tx FIFO empty interrupt mask bits */

/********************  Bit definition forUSB_OTG_DEACHINT register  ****************/
#define USB_OTG_DEACHINT_IEP1INT             ((uint32_t)0x00000002)        /*!< IN endpoint 1interrupt bit */
#define USB_OTG_DEACHINT_OEP1INT             ((uint32_t)0x00020000)        /*!< OUT endpoint 1 interrupt bit */

/********************  Bit definition forUSB_OTG_GCCFG register  *******************/
#define USB_OTG_GCCFG_PWRDWN                 ((uint32_t)0x00010000)        /*!< Power down */
#define USB_OTG_GCCFG_I2CPADEN               ((uint32_t)0x00020000)        /*!< Enable I2C bus connection for the external I2C PHY interface */
#define USB_OTG_GCCFG_VBUSASEN               ((uint32_t)0x00040000)        /*!< Enable the VBUS sensing device */
#define USB_OTG_GCCFG_VBUSBSEN               ((uint32_t)0x00080000)        /*!< Enable the VBUS sensing device */
#define USB_OTG_GCCFG_SOFOUTEN               ((uint32_t)0x00100000)        /*!< SOF output enable */
#define USB_OTG_GCCFG_NOVBUSSENS             ((uint32_t)0x00200000)        /*!< VBUS sensing disable option */

/********************  Bit definition forUSB_OTG_DEACHINTMSK register  *************/
#define USB_OTG_DEACHINTMSK_IEP1INTM         ((uint32_t)0x00000002)        /*!< IN Endpoint 1 interrupt mask bit */
#define USB_OTG_DEACHINTMSK_OEP1INTM         ((uint32_t)0x00020000)        /*!< OUT Endpoint 1 interrupt mask bit */

/********************  Bit definition forUSB_OTG_CID register  *********************/
#define USB_OTG_CID_PRODUCT_ID               ((uint32_t)0xFFFFFFFF)        /*!< Product ID field */

/********************  Bit definition forUSB_OTG_DIEPEACHMSK1 register  ************/
#define USB_OTG_DIEPEACHMSK1_XFRCM           ((uint32_t)0x00000001)        /*!< Transfer completed interrupt mask */
#define USB_OTG_DIEPEACHMSK1_EPDM            ((uint32_t)0x00000002)        /*!< Endpoint disabled interrupt mask */
#define USB_OTG_DIEPEACHMSK1_TOM             ((uint32_t)0x00000008)        /*!< Timeout condition mask (nonisochronous endpoints) */
#define USB_OTG_DIEPEACHMSK1_ITTXFEMSK       ((uint32_t)0x00000010)        /*!< IN token received when TxFIFO empty mask */
#define USB_OTG_DIEPEACHMSK1_INEPNMM         ((uint32_t)0x00000020)        /*!< IN token received with EP mismatch mask */
#define USB_OTG_DIEPEACHMSK1_INEPNEM         ((uint32_t)0x00000040)        /*!< IN endpoint NAK effective mask */
#define USB_OTG_DIEPEACHMSK1_TXFURM          ((uint32_t)0x00000100)        /*!< FIFO underrun mask */
#define USB_OTG_DIEPEACHMSK1_BIM             ((uint32_t)0x00000200)        /*!< BNA interrupt mask */
#define USB_OTG_DIEPEACHMSK1_NAKM            ((uint32_t)0x00002000)        /*!< NAK interrupt mask */

/********************  Bit definition forUSB_OTG_HPRT register  ********************/
#define USB_OTG_HPRT_PCSTS                   ((uint32_t)0x00000001)        /*!< Port connect status */
#define USB_OTG_HPRT_PCDET                   ((uint32_t)0x00000002)        /*!< Port connect detected */
#define USB_OTG_HPRT_PENA                    ((uint32_t)0x00000004)        /*!< Port enable */
#define USB_OTG_HPRT_PENCHNG                 ((uint32_t)0x00000008)        /*!< Port enable/disable change */
#define USB_OTG_HPRT_POCA                    ((uint32_t)0x00000010)        /*!< Port overcurrent active */
#define USB_OTG_HPRT_POCCHNG                 ((uint32_t)0x00000020)        /*!< Port overcurrent change */
#define USB_OTG_HPRT_PRES                    ((uint32_t)0x00000040)        /*!< Port resume */
#define USB_OTG_HPRT_PSUSP                   ((uint32_t)0x00000080)        /*!< Port suspend */
#define USB_OTG_HPRT_PRST                    ((uint32_t)0x00000100)        /*!< Port reset */

#define USB_OTG_HPRT_PLSTS                   ((uint32_t)0x00000C00)        /*!< Port line status */
#define USB_OTG_HPRT_PLSTS_0                 ((uint32_t)0x00000400)        /*!<Bit 0 */
#define USB_OTG_HPRT_PLSTS_1                 ((uint32_t)0x00000800)        /*!<Bit 1 */
#define USB_OTG_HPRT_PPWR                    ((uint32_t)0x00001000)        /*!< Port power */

#define USB_OTG_HPRT_PTCTL                   ((uint32_t)0x0001E000)        /*!< Port test control */
#define USB_OTG_HPRT_PTCTL_0                 ((uint32_t)0x00002000)        /*!<Bit 0 */
#define USB_OTG_HPRT_PTCTL_1                 ((uint32_t)0x00004000)        /*!<Bit 1 */
#define USB_OTG_HPRT_PTCTL_2                 ((uint32_t)0x00008000)        /*!<Bit 2 */
#define USB_OTG_HPRT_PTCTL_3                 ((uint32_t)0x00010000)        /*!<Bit 3 */

#define USB_OTG_HPRT_PSPD                    ((uint32_t)0x00060000)        /*!< Port speed */
#define USB_OTG_HPRT_PSPD_0                  ((uint32_t)0x00020000)        /*!<Bit 0 */
#define USB_OTG_HPRT_PSPD_1                  ((uint32_t)0x00040000)        /*!<Bit 1 */

/********************  Bit definition forUSB_OTG_DOEPEACHMSK1 register  ************/
#define USB_OTG_DOEPEACHMSK1_XFRCM           ((uint32_t)0x00000001)        /*!< Transfer completed interrupt mask */
#define USB_OTG_DOEPEACHMSK1_EPDM            ((uint32_t)0x00000002)        /*!< Endpoint disabled interrupt mask */
#define USB_OTG_DOEPEACHMSK1_TOM             ((uint32_t)0x00000008)        /*!< Timeout condition mask */
#define USB_OTG_DOEPEACHMSK1_ITTXFEMSK       ((uint32_t)0x00000010)        /*!< IN token received when TxFIFO empty mask */
#define USB_OTG_DOEPEACHMSK1_INEPNMM         ((uint32_t)0x00000020)        /*!< IN token received with EP mismatch mask */
#define USB_OTG_DOEPEACHMSK1_INEPNEM         ((uint32_t)0x00000040)        /*!< IN endpoint NAK effective mask */
#define USB_OTG_DOEPEACHMSK1_TXFURM          ((uint32_t)0x00000100)        /*!< OUT packet error mask */
#define USB_OTG_DOEPEACHMSK1_BIM             ((uint32_t)0x00000200)        /*!< BNA interrupt mask */
#define USB_OTG_DOEPEACHMSK1_BERRM           ((uint32_t)0x00001000)        /*!< Bubble error interrupt mask */
#define USB_OTG_DOEPEACHMSK1_NAKM            ((uint32_t)0x00002000)        /*!< NAK interrupt mask */
#define USB_OTG_DOEPEACHMSK1_NYETM           ((uint32_t)0x00004000)        /*!< NYET interrupt mask */

/********************  Bit definition forUSB_OTG_HPTXFSIZ register  ****************/
#define USB_OTG_HPTXFSIZ_PTXSA               ((uint32_t)0x0000FFFF)        /*!< Host periodic TxFIFO start address */
#define USB_OTG_HPTXFSIZ_PTXFD               ((uint32_t)0xFFFF0000)        /*!< Host periodic TxFIFO depth */

/********************  Bit definition forUSB_OTG_DIEPCTL register  *****************/
#define USB_OTG_DIEPCTL_MPSIZ                ((uint32_t)0x000007FF)        /*!< Maximum packet size */
#define USB_OTG_DIEPCTL_USBAEP               ((uint32_t)0x00008000)        /*!< USB active endpoint */
#define USB_OTG_DIEPCTL_EONUM_DPID           ((uint32_t)0x00010000)        /*!< Even/odd frame */
#define USB_OTG_DIEPCTL_NAKSTS               ((uint32_t)0x00020000)        /*!< NAK status */

#define USB_OTG_DIEPCTL_EPTYP                ((uint32_t)0x000C0000)        /*!< Endpoint type */
#define USB_OTG_DIEPCTL_EPTYP_0              ((uint32_t)0x00040000)        /*!<Bit 0 */
#define USB_OTG_DIEPCTL_EPTYP_1              ((uint32_t)0x00080000)        /*!<Bit 1 */
#define USB_OTG_DIEPCTL_STALL                ((uint32_t)0x00200000)        /*!< STALL handshake */

#define USB_OTG_DIEPCTL_TXFNUM               ((uint32_t)0x03C00000)        /*!< TxFIFO number */
#define USB_OTG_DIEPCTL_TXFNUM_0             ((uint32_t)0x00400000)        /*!<Bit 0 */
#define USB_OTG_DIEPCTL_TXFNUM_1             ((uint32_t)0x00800000)        /*!<Bit 1 */
#define USB_OTG_DIEPCTL_TXFNUM_2             ((uint32_t)0x01000000)        /*!<Bit 2 */
#define USB_OTG_DIEPCTL_TXFNUM_3             ((uint32_t)0x02000000)        /*!<Bit 3 */
#define USB_OTG_DIEPCTL_CNAK                 ((uint32_t)0x04000000)        /*!< Clear NAK */
#define USB_OTG_DIEPCTL_SNAK                 ((uint32_t)0x08000000)        /*!< Set NAK */
#define USB_OTG_DIEPCTL_SD0PID_SEVNFRM       ((uint32_t)0x10000000)        /*!< Set DATA0 PID */
#define USB_OTG_DIEPCTL_SODDFRM              ((uint32_t)0x20000000)        /*!< Set odd frame */
#define USB_OTG_DIEPCTL_EPDIS                ((uint32_t)0x40000000)        /*!< Endpoint disable */
#define USB_OTG_DIEPCTL_EPENA                ((uint32_t)0x80000000)        /*!< Endpoint enable */

/********************  Bit definition forUSB_OTG_HCCHAR register  ******************/
#define USB_OTG_HCCHAR_MPSIZ                 ((uint32_t)0x000007FF)        /*!< Maximum packet size */

#define USB_OTG_HCCHAR_EPNUM                 ((uint32_t)0x00007800)        /*!< Endpoint number */
#define USB_OTG_HCCHAR_EPNUM_0               ((uint32_t)0x00000800)        /*!<Bit 0 */
#define USB_OTG_HCCHAR_EPNUM_1               ((uint32_t)0x00001000)        /*!<Bit 1 */
#define USB_OTG_HCCHAR_EPNUM_2               ((uint32_t)0x00002000)        /*!<Bit 2 */
#define USB_OTG_HCCHAR_EPNUM_3               ((uint32_t)0x00004000)        /*!<Bit 3 */
#define USB_OTG_HCCHAR_EPDIR                 ((uint32_t)0x00008000)        /*!< Endpoint direction */
#define USB_OTG_HCCHAR_LSDEV                 ((uint32_t)0x00020000)        /*!< Low-speed device */

#define USB_OTG_HCCHAR_EPTYP                 ((uint32_t)0x000C0000)        /*!< Endpoint type */
#define USB_OTG_HCCHAR_EPTYP_0               ((uint32_t)0x00040000)        /*!<Bit 0 */
#define USB_OTG_HCCHAR_EPTYP_1               ((uint32_t)0x00080000)        /*!<Bit 1 */
                                      
#define USB_OTG_HCCHAR_MC                    ((uint32_t)0x00300000)        /*!< Multi Count (MC) / Error Count (EC) */
#define USB_OTG_HCCHAR_MC_0                  ((uint32_t)0x00100000)        /*!<Bit 0 */
#define USB_OTG_HCCHAR_MC_1                  ((uint32_t)0x00200000)        /*!<Bit 1 */

#define USB_OTG_HCCHAR_DAD                   ((uint32_t)0x1FC00000)        /*!< Device address */
#define USB_OTG_HCCHAR_DAD_0                 ((uint32_t)0x00400000)        /*!<Bit 0 */
#define USB_OTG_HCCHAR_DAD_1                 ((uint32_t)0x00800000)        /*!<Bit 1 */
#define USB_OTG_HCCHAR_DAD_2                 ((uint32_t)0x01000000)        /*!<Bit 2 */
#define USB_OTG_HCCHAR_DAD_3                 ((uint32_t)0x02000000)        /*!<Bit 3 */
#define USB_OTG_HCCHAR_DAD_4                 ((uint32_t)0x04000000)        /*!<Bit 4 */
#define USB_OTG_HCCHAR_DAD_5                 ((uint32_t)0x08000000)        /*!<Bit 5 */
#define USB_OTG_HCCHAR_DAD_6                 ((uint32_t)0x10000000)        /*!<Bit 6 */
#define USB_OTG_HCCHAR_ODDFRM                ((uint32_t)0x20000000)        /*!< Odd frame */
#define USB_OTG_HCCHAR_CHDIS                 ((uint32_t)0x40000000)        /*!< Channel disable */
#define USB_OTG_HCCHAR_CHENA                 ((uint32_t)0x80000000)        /*!< Channel enable */

/********************  Bit definition forUSB_OTG_HCSPLT register  ******************/

#define USB_OTG_HCSPLT_PRTADDR               ((uint32_t)0x0000007F)        /*!< Port address */
#define USB_OTG_HCSPLT_PRTADDR_0             ((uint32_t)0x00000001)        /*!<Bit 0 */
#define USB_OTG_HCSPLT_PRTADDR_1             ((uint32_t)0x00000002)        /*!<Bit 1 */
#define USB_OTG_HCSPLT_PRTADDR_2             ((uint32_t)0x00000004)        /*!<Bit 2 */
#define USB_OTG_HCSPLT_PRTADDR_3             ((uint32_t)0x00000008)        /*!<Bit 3 */
#define USB_OTG_HCSPLT_PRTADDR_4             ((uint32_t)0x00000010)        /*!<Bit 4 */
#define USB_OTG_HCSPLT_PRTADDR_5             ((uint32_t)0x00000020)        /*!<Bit 5 */
#define USB_OTG_HCSPLT_PRTADDR_6             ((uint32_t)0x00000040)        /*!<Bit 6 */

#define USB_OTG_HCSPLT_HUBADDR               ((uint32_t)0x00003F80)        /*!< Hub address */
#define USB_OTG_HCSPLT_HUBADDR_0             ((uint32_t)0x00000080)        /*!<Bit 0 */
#define USB_OTG_HCSPLT_HUBADDR_1             ((uint32_t)0x00000100)        /*!<Bit 1 */
#define USB_OTG_HCSPLT_HUBADDR_2             ((uint32_t)0x00000200)        /*!<Bit 2 */
#define USB_OTG_HCSPLT_HUBADDR_3             ((uint32_t)0x00000400)        /*!<Bit 3 */
#define USB_OTG_HCSPLT_HUBADDR_4             ((uint32_t)0x00000800)        /*!<Bit 4 */
#define USB_OTG_HCSPLT_HUBADDR_5             ((uint32_t)0x00001000)        /*!<Bit 5 */
#define USB_OTG_HCSPLT_HUBADDR_6             ((uint32_t)0x00002000)        /*!<Bit 6 */

#define USB_OTG_HCSPLT_XACTPOS               ((uint32_t)0x0000C000)        /*!< XACTPOS */
#define USB_OTG_HCSPLT_XACTPOS_0             ((uint32_t)0x00004000)        /*!<Bit 0 */
#define USB_OTG_HCSPLT_XACTPOS_1             ((uint32_t)0x00008000)        /*!<Bit 1 */
#define USB_OTG_HCSPLT_COMPLSPLT             ((uint32_t)0x00010000)        /*!< Do complete split */
#define USB_OTG_HCSPLT_SPLITEN               ((uint32_t)0x80000000)        /*!< Split enable */

/********************  Bit definition forUSB_OTG_HCINT register  *******************/
#define USB_OTG_HCINT_XFRC                   ((uint32_t)0x00000001)        /*!< Transfer completed */
#define USB_OTG_HCINT_CHH                    ((uint32_t)0x00000002)        /*!< Channel halted */
#define USB_OTG_HCINT_AHBERR                 ((uint32_t)0x00000004)        /*!< AHB error */
#define USB_OTG_HCINT_STALL                  ((uint32_t)0x00000008)        /*!< STALL response received interrupt */
#define USB_OTG_HCINT_NAK                    ((uint32_t)0x00000010)        /*!< NAK response received interrupt */
#define USB_OTG_HCINT_ACK                    ((uint32_t)0x00000020)        /*!< ACK response received/transmitted interrupt */
#define USB_OTG_HCINT_NYET                   ((uint32_t)0x00000040)        /*!< Response received interrupt */
#define USB_OTG_HCINT_TXERR                  ((uint32_t)0x00000080)        /*!< Transaction error */
#define USB_OTG_HCINT_BBERR                  ((uint32_t)0x00000100)        /*!< Babble error */
#define USB_OTG_HCINT_FRMOR                  ((uint32_t)0x00000200)        /*!< Frame overrun */
#define USB_OTG_HCINT_DTERR                  ((uint32_t)0x00000400)        /*!< Data toggle error */

/********************  Bit definition forUSB_OTG_DIEPINT register  *****************/
#define USB_OTG_DIEPINT_XFRC                 ((uint32_t)0x00000001)        /*!< Transfer completed interrupt */
#define USB_OTG_DIEPINT_EPDISD               ((uint32_t)0x00000002)        /*!< Endpoint disabled interrupt */
#define USB_OTG_DIEPINT_TOC                  ((uint32_t)0x00000008)        /*!< Timeout condition */
#define USB_OTG_DIEPINT_ITTXFE               ((uint32_t)0x00000010)        /*!< IN token received when TxFIFO is empty */
#define USB_OTG_DIEPINT_INEPNE               ((uint32_t)0x00000040)        /*!< IN endpoint NAK effective */
#define USB_OTG_DIEPINT_TXFE                 ((uint32_t)0x00000080)        /*!< Transmit FIFO empty */
#define USB_OTG_DIEPINT_TXFIFOUDRN           ((uint32_t)0x00000100)        /*!< Transmit Fifo Underrun */
#define USB_OTG_DIEPINT_BNA                  ((uint32_t)0x00000200)        /*!< Buffer not available interrupt */
#define USB_OTG_DIEPINT_PKTDRPSTS            ((uint32_t)0x00000800)        /*!< Packet dropped status */
#define USB_OTG_DIEPINT_BERR                 ((uint32_t)0x00001000)        /*!< Babble error interrupt */
#define USB_OTG_DIEPINT_NAK                  ((uint32_t)0x00002000)        /*!< NAK interrupt */

/********************  Bit definition forUSB_OTG_HCINTMSK register  ****************/
#define USB_OTG_HCINTMSK_XFRCM               ((uint32_t)0x00000001)        /*!< Transfer completed mask */
#define USB_OTG_HCINTMSK_CHHM                ((uint32_t)0x00000002)        /*!< Channel halted mask */
#define USB_OTG_HCINTMSK_AHBERR              ((uint32_t)0x00000004)        /*!< AHB error */
#define USB_OTG_HCINTMSK_STALLM              ((uint32_t)0x00000008)        /*!< STALL response received interrupt mask */
#define USB_OTG_HCINTMSK_NAKM                ((uint32_t)0x00000010)        /*!< NAK response received interrupt mask */
#define USB_OTG_HCINTMSK_ACKM                ((uint32_t)0x00000020)        /*!< ACK response received/transmitted interrupt mask */
#define USB_OTG_HCINTMSK_NYET                ((uint32_t)0x00000040)        /*!< response received interrupt mask */
#define USB_OTG_HCINTMSK_TXERRM              ((uint32_t)0x00000080)        /*!< Transaction error mask */
#define USB_OTG_HCINTMSK_BBERRM              ((uint32_t)0x00000100)        /*!< Babble error mask */
#define USB_OTG_HCINTMSK_FRMORM              ((uint32_t)0x00000200)        /*!< Frame overrun mask */
#define USB_OTG_HCINTMSK_DTERRM              ((uint32_t)0x00000400)        /*!< Data toggle error mask */

/********************  Bit definition for USB_OTG_DIEPTSIZ register  ***************/
#define USB_OTG_DIEPTSIZ_XFRSIZ              ((uint32_t)0x0007FFFF)        /*!< Transfer size */
#define USB_OTG_DIEPTSIZ_PKTCNT              ((uint32_t)0x1FF80000)        /*!< Packet count */
#define USB_OTG_DIEPTSIZ_MULCNT              ((uint32_t)0x60000000)        /*!< Packet count */

/********************  Bit definition forUSB_OTG_HCTSIZ register  ******************/
#define USB_OTG_HCTSIZ_XFRSIZ                ((uint32_t)0x0007FFFF)        /*!< Transfer size */
#define USB_OTG_HCTSIZ_PKTCNT                ((uint32_t)0x1FF80000)        /*!< Packet count */
#define USB_OTG_HCTSIZ_DOPING                ((uint32_t)0x80000000)        /*!< Do PING */
#define USB_OTG_HCTSIZ_DPID                  ((uint32_t)0x60000000)        /*!< Data PID */
#define USB_OTG_HCTSIZ_DPID_0                ((uint32_t)0x20000000)        /*!<Bit 0 */
#define USB_OTG_HCTSIZ_DPID_1                ((uint32_t)0x40000000)        /*!<Bit 1 */

/********************  Bit definition forUSB_OTG_DIEPDMA register  *****************/
#define USB_OTG_DIEPDMA_DMAADDR              ((uint32_t)0xFFFFFFFF)        /*!< DMA address */

/********************  Bit definition forUSB_OTG_HCDMA register  *******************/
#define USB_OTG_HCDMA_DMAADDR                ((uint32_t)0xFFFFFFFF)        /*!< DMA address */

/********************  Bit definition forUSB_OTG_DTXFSTS register  *****************/
#define USB_OTG_DTXFSTS_INEPTFSAV            ((uint32_t)0x0000FFFF)        /*!< IN endpoint TxFIFO space avail */

/********************  Bit definition forUSB_OTG_DIEPTXF register  *****************/
#define USB_OTG_DIEPTXF_INEPTXSA             ((uint32_t)0x0000FFFF)        /*!< IN endpoint FIFOx transmit RAM start address */
#define USB_OTG_DIEPTXF_INEPTXFD             ((uint32_t)0xFFFF0000)        /*!< IN endpoint TxFIFO depth */

/********************  Bit definition forUSB_OTG_DOEPCTL register  *****************/
#define USB_OTG_DOEPCTL_MPSIZ                ((uint32_t)0x000007FF)        /*!< Maximum packet size */
#define USB_OTG_DOEPCTL_USBAEP               ((uint32_t)0x00008000)        /*!< USB active endpoint */
#define USB_OTG_DOEPCTL_NAKSTS               ((uint32_t)0x00020000)        /*!< NAK status */
#define USB_OTG_DOEPCTL_SD0PID_SEVNFRM       ((uint32_t)0x10000000)        /*!< Set DATA0 PID */
#define USB_OTG_DOEPCTL_SODDFRM              ((uint32_t)0x20000000)        /*!< Set odd frame */
#define USB_OTG_DOEPCTL_EPTYP                ((uint32_t)0x000C0000)        /*!< Endpoint type */
#define USB_OTG_DOEPCTL_EPTYP_0              ((uint32_t)0x00040000)        /*!<Bit 0 */
#define USB_OTG_DOEPCTL_EPTYP_1              ((uint32_t)0x00080000)        /*!<Bit 1 */
#define USB_OTG_DOEPCTL_SNPM                 ((uint32_t)0x00100000)        /*!< Snoop mode */
#define USB_OTG_DOEPCTL_STALL                ((uint32_t)0x00200000)        /*!< STALL handshake */
#define USB_OTG_DOEPCTL_CNAK                 ((uint32_t)0x04000000)        /*!< Clear NAK */
#define USB_OTG_DOEPCTL_SNAK                 ((uint32_t)0x08000000)        /*!< Set NAK */
#define USB_OTG_DOEPCTL_EPDIS                ((uint32_t)0x40000000)        /*!< Endpoint disable */
#define USB_OTG_DOEPCTL_EPENA                ((uint32_t)0x80000000)        /*!< Endpoint enable */

/********************  Bit definition forUSB_OTG_DOEPINT register  *****************/
#define USB_OTG_DOEPINT_XFRC                 ((uint32_t)0x00000001)        /*!< Transfer completed interrupt */
#define USB_OTG_DOEPINT_EPDISD               ((uint32_t)0x00000002)        /*!< Endpoint disabled interrupt */
#define USB_OTG_DOEPINT_STUP                 ((uint32_t)0x00000008)        /*!< SETUP phase done */
#define USB_OTG_DOEPINT_OTEPDIS              ((uint32_t)0x00000010)        /*!< OUT token received when endpoint disabled */
#define USB_OTG_DOEPINT_B2BSTUP              ((uint32_t)0x00000040)        /*!< Back-to-back SETUP packets received */
#define USB_OTG_DOEPINT_NYET                 ((uint32_t)0x00004000)        /*!< NYET interrupt */

/********************  Bit definition forUSB_OTG_DOEPTSIZ register  ****************/
#define USB_OTG_DOEPTSIZ_XFRSIZ              ((uint32_t)0x0007FFFF)        /*!< Transfer size */
#define USB_OTG_DOEPTSIZ_PKTCNT              ((uint32_t)0x1FF80000)        /*!< Packet count */

#define USB_OTG_DOEPTSIZ_STUPCNT             ((uint32_t)0x60000000)        /*!< SETUP packet count */
#define USB_OTG_DOEPTSIZ_STUPCNT_0           ((uint32_t)0x20000000)        /*!<Bit 0 */
#define USB_OTG_DOEPTSIZ_STUPCNT_1           ((uint32_t)0x40000000)        /*!<Bit 1 */

/********************  Bit definition for PCGCCTL register  ************************/
#define USB_OTG_PCGCCTL_STOPCLK              ((uint32_t)0x00000001)        /*!< SETUP packet count */
#define USB_OTG_PCGCCTL_GATECLK              ((uint32_t)0x00000002)        /*!<Bit 0 */
#define USB_OTG_PCGCCTL_PHYSUSP              ((uint32_t)0x00000010)        /*!<Bit 1 */

/**
  * @}
*/

/**
  * @}
*/ 

/** @addtogroup Exported_macro
  * @{
  */

/****************************** ADC Instances *********************************/
#define IS_ADC_ALL_INSTANCE(INSTANCE) (((INSTANCE) == ADC1) || \
                                       ((INSTANCE) == ADC2))

#define IS_ADC_MULTIMODE_MASTER_INSTANCE(INSTANCE) ((INSTANCE) == ADC1)

#define IS_ADC_DMA_CAPABILITY_INSTANCE(INSTANCE) ((INSTANCE) == ADC1)


/****************************** CAN Instances *********************************/   
#define IS_CAN_ALL_INSTANCE(INSTANCE) (((INSTANCE) == CAN1) || \
                                       ((INSTANCE) == CAN2))

/****************************** CRC Instances *********************************/
#define IS_CRC_ALL_INSTANCE(INSTANCE) ((INSTANCE) == CRC)

/****************************** DAC Instances *********************************/
#define IS_DAC_ALL_INSTANCE(INSTANCE) ((INSTANCE) == DAC)

/****************************** DMA Instances *********************************/
#define IS_DMA_ALL_INSTANCE(INSTANCE) (((INSTANCE) == DMA1_Channel1) || \
                                       ((INSTANCE) == DMA1_Channel2) || \
                                       ((INSTANCE) == DMA1_Channel3) || \
                                       ((INSTANCE) == DMA1_Channel4) || \
                                       ((INSTANCE) == DMA1_Channel5) || \
                                       ((INSTANCE) == DMA1_Channel6) || \
                                       ((INSTANCE) == DMA1_Channel7) || \
                                       ((INSTANCE) == DMA2_Channel1) || \
                                       ((INSTANCE) == DMA2_Channel2) || \
                                       ((INSTANCE) == DMA2_Channel3) || \
                                       ((INSTANCE) == DMA2_Channel4) || \
                                       ((INSTANCE) == DMA2_Channel5))
  
/******************************* GPIO Instances *******************************/
#define IS_GPIO_ALL_INSTANCE(INSTANCE) (((INSTANCE) == GPIOA) || \
                                        ((INSTANCE) == GPIOB) || \
                                        ((INSTANCE) == GPIOC) || \
                                        ((INSTANCE) == GPIOD) || \
                                        ((INSTANCE) == GPIOE))

/**************************** GPIO Alternate Function Instances ***************/
#define IS_GPIO_AF_INSTANCE(INSTANCE) IS_GPIO_ALL_INSTANCE(INSTANCE)

/**************************** GPIO Lock Instances *****************************/
#define IS_GPIO_LOCK_INSTANCE(INSTANCE) IS_GPIO_ALL_INSTANCE(INSTANCE)

/******************************** I2C Instances *******************************/
#define IS_I2C_ALL_INSTANCE(INSTANCE) (((INSTANCE) == I2C1) || \
                                       ((INSTANCE) == I2C2))

/******************************** I2S Instances *******************************/
#define IS_I2S_ALL_INSTANCE(INSTANCE) (((INSTANCE) == SPI2) || \
                                       ((INSTANCE) == SPI3))

/****************************** IWDG Instances ********************************/
#define IS_IWDG_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == IWDG)

/******************************** SPI Instances *******************************/
#define IS_SPI_ALL_INSTANCE(INSTANCE) (((INSTANCE) == SPI1) || \
                                       ((INSTANCE) == SPI2) || \
                                       ((INSTANCE) == SPI3))

/****************************** START TIM Instances ***************************/
/****************************** TIM Instances *********************************/
#define IS_TIM_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM6)    || \
   ((INSTANCE) == TIM7))

#define IS_TIM_CC1_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))

#define IS_TIM_CC2_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))

#define IS_TIM_CC3_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))

#define IS_TIM_CC4_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))

#define IS_TIM_CLOCKSOURCE_ETRMODE1_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))

#define IS_TIM_CLOCKSOURCE_ETRMODE2_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))

#define IS_TIM_CLOCKSOURCE_TIX_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))

#define IS_TIM_CLOCKSOURCE_ITRX_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))

#define IS_TIM_OCXREF_CLEAR_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))

#define IS_TIM_ENCODER_INTERFACE_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))

#define IS_TIM_XOR_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))

#define IS_TIM_MASTER_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM6)    || \
   ((INSTANCE) == TIM7))

#define IS_TIM_SLAVE_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))

#define IS_TIM_DMABURST_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))

#define IS_TIM_BREAK_INSTANCE(INSTANCE)\
  ((INSTANCE) == TIM1)

#define IS_TIM_CCX_INSTANCE(INSTANCE, CHANNEL) \
   ((((INSTANCE) == TIM1) &&                  \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3) ||          \
      ((CHANNEL) == TIM_CHANNEL_4)))           \
    ||                                         \
    (((INSTANCE) == TIM2) &&                   \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3) ||          \
      ((CHANNEL) == TIM_CHANNEL_4)))           \
    ||                                         \
    (((INSTANCE) == TIM3) &&                   \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3) ||          \
      ((CHANNEL) == TIM_CHANNEL_4)))           \
    ||                                         \
    (((INSTANCE) == TIM4) &&                   \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3) ||          \
      ((CHANNEL) == TIM_CHANNEL_4)))           \
    ||                                         \
    (((INSTANCE) == TIM5) &&                   \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3) ||          \
      ((CHANNEL) == TIM_CHANNEL_4))))

#define IS_TIM_CCXN_INSTANCE(INSTANCE, CHANNEL) \
    (((INSTANCE) == TIM1) &&                    \
     (((CHANNEL) == TIM_CHANNEL_1) ||           \
      ((CHANNEL) == TIM_CHANNEL_2) ||           \
      ((CHANNEL) == TIM_CHANNEL_3)))

#define IS_TIM_COUNTER_MODE_SELECT_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))

#define IS_TIM_REPETITION_COUNTER_INSTANCE(INSTANCE)\
  ((INSTANCE) == TIM1)

#define IS_TIM_CLOCK_DIVISION_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))

#define IS_TIM_DMA_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5)    || \
   ((INSTANCE) == TIM6)    || \
   ((INSTANCE) == TIM7))
    
#define IS_TIM_DMA_CC_INSTANCE(INSTANCE)\
  (((INSTANCE) == TIM1)    || \
   ((INSTANCE) == TIM2)    || \
   ((INSTANCE) == TIM3)    || \
   ((INSTANCE) == TIM4)    || \
   ((INSTANCE) == TIM5))
    
#define IS_TIM_COMMUTATION_EVENT_INSTANCE(INSTANCE)\
  ((INSTANCE) == TIM1)

/****************************** END TIM Instances *****************************/


/******************** USART Instances : Synchronous mode **********************/                                           
#define IS_USART_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                     ((INSTANCE) == USART2) || \
                                     ((INSTANCE) == USART3))

/******************** UART Instances : Asynchronous mode **********************/
#define IS_UART_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                    ((INSTANCE) == USART2) || \
                                    ((INSTANCE) == USART3) || \
                                    ((INSTANCE) == UART4)  || \
                                    ((INSTANCE) == UART5))

/******************** UART Instances : Half-Duplex mode **********************/
#define IS_UART_HALFDUPLEX_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                               ((INSTANCE) == USART2) || \
                                               ((INSTANCE) == USART3) || \
                                               ((INSTANCE) == UART4)  || \
                                               ((INSTANCE) == UART5))

/******************** UART Instances : LIN mode **********************/
#define IS_UART_LIN_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                        ((INSTANCE) == USART2) || \
                                        ((INSTANCE) == USART3) || \
                                        ((INSTANCE) == UART4)  || \
                                        ((INSTANCE) == UART5))

/****************** UART Instances : Hardware Flow control ********************/                                    
#define IS_UART_HWFLOW_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                           ((INSTANCE) == USART2) || \
                                           ((INSTANCE) == USART3))

/********************* UART Instances : Smard card mode ***********************/
#define IS_SMARTCARD_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                         ((INSTANCE) == USART2) || \
                                         ((INSTANCE) == USART3))

/*********************** UART Instances : IRDA mode ***************************/
#define IS_IRDA_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                    ((INSTANCE) == USART2) || \
                                    ((INSTANCE) == USART3) || \
                                    ((INSTANCE) == UART4)  || \
                                    ((INSTANCE) == UART5))

/***************** UART Instances : Multi-Processor mode **********************/
#define IS_UART_MULTIPROCESSOR_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                                   ((INSTANCE) == USART2) || \
                                                   ((INSTANCE) == USART3) || \
                                                   ((INSTANCE) == UART4)  || \
                                                   ((INSTANCE) == UART5))

/***************** UART Instances : DMA mode available **********************/
#define IS_UART_DMA_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                        ((INSTANCE) == USART2) || \
                                        ((INSTANCE) == USART3) || \
                                        ((INSTANCE) == UART4))

/****************************** RTC Instances *********************************/
#define IS_RTC_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == RTC)

/**************************** WWDG Instances *****************************/
#define IS_WWDG_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == WWDG)


/****************************** USB Instances ********************************/
#define IS_USB_ALL_INSTANCE(INSTANCE) ((INSTANCE) == USB_OTG_FS)

/****************************** USB Instances ********************************/
#define IS_ETH_ALL_INSTANCE(INSTANCE) ((INSTANCE) == ETH) 


/**
  * @}
*/ 
/******************************************************************************/
/*  For a painless codes migration between the STM32F1xx device product       */
/*  lines, the aliases defined below are put in place to overcome the         */
/*  differences in the interrupt handlers and IRQn definitions.               */
/*  No need to update developed interrupt code when moving across             */ 
/*  product lines within the same STM32F1 Family                              */
/******************************************************************************/

/* Aliases for __IRQn */
#define ADC1_IRQn               ADC1_2_IRQn
#define USB_LP_IRQn             CAN1_RX0_IRQn
#define USB_LP_CAN1_RX0_IRQn    CAN1_RX0_IRQn
#define USB_HP_CAN1_TX_IRQn     CAN1_TX_IRQn
#define USB_HP_IRQn             CAN1_TX_IRQn
#define DMA2_Channel4_5_IRQn    DMA2_Channel4_IRQn
#define USBWakeUp_IRQn          OTG_FS_WKUP_IRQn
#define CEC_IRQn                OTG_FS_WKUP_IRQn
#define TIM1_BRK_TIM15_IRQn     TIM1_BRK_IRQn
#define TIM1_BRK_TIM9_IRQn      TIM1_BRK_IRQn
#define TIM9_IRQn               TIM1_BRK_IRQn
#define TIM1_TRG_COM_TIM11_IRQn TIM1_TRG_COM_IRQn
#define TIM1_TRG_COM_TIM17_IRQn TIM1_TRG_COM_IRQn
#define TIM11_IRQn              TIM1_TRG_COM_IRQn
#define TIM10_IRQn              TIM1_UP_IRQn
#define TIM1_UP_TIM16_IRQn      TIM1_UP_IRQn
#define TIM1_UP_TIM10_IRQn      TIM1_UP_IRQn
#define TIM6_DAC_IRQn           TIM6_IRQn


/* Aliases for __IRQHandler */
#define ADC1_IRQHandler               ADC1_2_IRQHandler
#define USB_LP_IRQHandler             CAN1_RX0_IRQHandler
#define USB_LP_CAN1_RX0_IRQHandler    CAN1_RX0_IRQHandler
#define USB_HP_CAN1_TX_IRQHandler     CAN1_TX_IRQHandler
#define USB_HP_IRQHandler             CAN1_TX_IRQHandler
#define DMA2_Channel4_5_IRQHandler    DMA2_Channel4_IRQHandler
#define USBWakeUp_IRQHandler          OTG_FS_WKUP_IRQHandler
#define CEC_IRQHandler                OTG_FS_WKUP_IRQHandler
#define TIM1_BRK_TIM15_IRQHandler     TIM1_BRK_IRQHandler
#define TIM1_BRK_TIM9_IRQHandler      TIM1_BRK_IRQHandler
#define TIM9_IRQHandler               TIM1_BRK_IRQHandler
#define TIM1_TRG_COM_TIM11_IRQHandler TIM1_TRG_COM_IRQHandler
#define TIM1_TRG_COM_TIM17_IRQHandler TIM1_TRG_COM_IRQHandler
#define TIM11_IRQHandler              TIM1_TRG_COM_IRQHandler
#define TIM10_IRQHandler              TIM1_UP_IRQHandler
#define TIM1_UP_TIM16_IRQHandler      TIM1_UP_IRQHandler
#define TIM1_UP_TIM10_IRQHandler      TIM1_UP_IRQHandler
#define TIM6_DAC_IRQHandler           TIM6_IRQHandler


/**
  * @}
  */

/**
  * @}
  */


#ifdef __cplusplus
  }
#endif /* __cplusplus */
  
#endif /* __STM32F107xC_H */
  
  
  
  /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/*
** ###################################################################
**     Processors:          MK64FN1M0CAJ12
**                          MK64FN1M0VDC12
**                          MK64FN1M0VLL12
**                          MK64FN1M0VLQ12
**                          MK64FN1M0VMD12
**                          MK64FX512VDC12
**                          MK64FX512VLL12
**                          MK64FX512VLQ12
**                          MK64FX512VMD12
**
**     Compilers:           Keil ARM C/C++ Compiler
**                          Freescale C/C++ for Embedded ARM
**                          GNU C Compiler
**                          IAR ANSI C/C++ Compiler for ARM
**
**     Reference manual:    K64P144M120SF5RM, Rev.2, January 2014
**     Version:             rev. 2.9, 2016-03-21
**     Build:               b160321
**
**     Abstract:
**         CMSIS Peripheral Access Layer for MK64F12
**
**     Copyright (c) 1997 - 2016 Freescale Semiconductor, Inc.
**     All rights reserved.
**
**     Redistribution and use in source and binary forms, with or without modification,
**     are permitted provided that the following conditions are met:
**
**     o Redistributions of source code must retain the above copyright notice, this list
**       of conditions and the following disclaimer.
**
**     o Redistributions in binary form must reproduce the above copyright notice, this
**       list of conditions and the following disclaimer in the documentation and/or
**       other materials provided with the distribution.
**
**     o Neither the name of Freescale Semiconductor, Inc. nor the names of its
**       contributors may be used to endorse or promote products derived from this
**       software without specific prior written permission.
**
**     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
**     ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
**     WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
**     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
**     ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
**     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
**     ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
**     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
**     SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**     http:                 www.freescale.com
**     mail:                 support@freescale.com
**
**     Revisions:
**     - rev. 1.0 (2013-08-12)
**         Initial version.
**     - rev. 2.0 (2013-10-29)
**         Register accessor macros added to the memory map.
**         Symbols for Processor Expert memory map compatibility added to the memory map.
**         Startup file for gcc has been updated according to CMSIS 3.2.
**         System initialization updated.
**         MCG - registers updated.
**         PORTA, PORTB, PORTC, PORTE - registers for digital filter removed.
**     - rev. 2.1 (2013-10-30)
**         Definition of BITBAND macros updated to support peripherals with 32-bit acces disabled.
**     - rev. 2.2 (2013-12-09)
**         DMA - EARS register removed.
**         AIPS0, AIPS1 - MPRA register updated.
**     - rev. 2.3 (2014-01-24)
**         Update according to reference manual rev. 2
**         ENET, MCG, MCM, SIM, USB - registers updated
**     - rev. 2.4 (2014-02-10)
**         The declaration of clock configurations has been moved to separate header file system_MK64F12.h
**         Update of SystemInit() and SystemCoreClockUpdate() functions.
**     - rev. 2.5 (2014-02-10)
**         The declaration of clock configurations has been moved to separate header file system_MK64F12.h
**         Update of SystemInit() and SystemCoreClockUpdate() functions.
**         Module access macro module_BASES replaced by module_BASE_PTRS.
**     - rev. 2.6 (2014-08-28)
**         Update of system files - default clock configuration changed.
**         Update of startup files - possibility to override DefaultISR added.
**     - rev. 2.7 (2014-10-14)
**         Interrupt INT_LPTimer renamed to INT_LPTMR0, interrupt INT_Watchdog renamed to INT_WDOG_EWM.
**     - rev. 2.8 (2015-02-19)
**         Renamed interrupt vector LLW to LLWU.
**     - rev. 2.9 (2016-03-21)
**         Added MK64FN1M0CAJ12 part.
**         GPIO - renamed port instances: PTx -> GPIOx.
**
** ###################################################################
*/

/*!
 * @file MK64F12.h
 * @version 2.9
 * @date 2016-03-21
 * @brief CMSIS Peripheral Access Layer for MK64F12
 *
 * CMSIS Peripheral Access Layer for MK64F12
 */

#ifndef _MK64F12_H_
#define _MK64F12_H_                              /**< Symbol preventing repeated inclusion */

/** Memory map major version (memory maps with equal major version number are
 * compatible) */
#define MCU_MEM_MAP_VERSION 0x0200U
/** Memory map minor version */
#define MCU_MEM_MAP_VERSION_MINOR 0x0009U

/**
 * @brief Macro to calculate address of an aliased word in the peripheral
 *        bitband area for a peripheral register and bit (bit band region 0x40000000 to
 *        0x400FFFFF).
 * @param Reg Register to access.
 * @param Bit Bit number to access.
 * @return  Address of the aliased word in the peripheral bitband area.
 */
#define BITBAND_REGADDR(Reg,Bit) (0x42000000u + (32u*((uint32_t)&(Reg) - (uint32_t)0x40000000u)) + (4u*((uint32_t)(Bit))))
/**
 * @brief Macro to access a single bit of a peripheral register (bit band region
 *        0x40000000 to 0x400FFFFF) using the bit-band alias region access. Can
 *        be used for peripherals with 32bit access allowed.
 * @param Reg Register to access.
 * @param Bit Bit number to access.
 * @return Value of the targeted bit in the bit band region.
 */
#define BITBAND_REG32(Reg,Bit) (*((uint32_t volatile*)(BITBAND_REGADDR((Reg),(Bit)))))
#define BITBAND_REG(Reg,Bit) (BITBAND_REG32((Reg),(Bit)))
/**
 * @brief Macro to access a single bit of a peripheral register (bit band region
 *        0x40000000 to 0x400FFFFF) using the bit-band alias region access. Can
 *        be used for peripherals with 16bit access allowed.
 * @param Reg Register to access.
 * @param Bit Bit number to access.
 * @return Value of the targeted bit in the bit band region.
 */
#define BITBAND_REG16(Reg,Bit) (*((uint16_t volatile*)(BITBAND_REGADDR((Reg),(Bit)))))
/**
 * @brief Macro to access a single bit of a peripheral register (bit band region
 *        0x40000000 to 0x400FFFFF) using the bit-band alias region access. Can
 *        be used for peripherals with 8bit access allowed.
 * @param Reg Register to access.
 * @param Bit Bit number to access.
 * @return Value of the targeted bit in the bit band region.
 */
#define BITBAND_REG8(Reg,Bit) (*((uint8_t volatile*)(BITBAND_REGADDR((Reg),(Bit)))))

/* ----------------------------------------------------------------------------
   -- Interrupt vector numbers
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup Interrupt_vector_numbers Interrupt vector numbers
 * @{
 */

/** Interrupt Number Definitions */
#define NUMBER_OF_INT_VECTORS 102                /**< Number of interrupts in the Vector table */

typedef enum IRQn {
  /* Auxiliary constants */
  NotAvail_IRQn                = -128,             /**< Not available device specific interrupt */

  /* Core interrupts */
  NonMaskableInt_IRQn          = -14,              /**< Non Maskable Interrupt */
  HardFault_IRQn               = -13,              /**< Cortex-M4 SV Hard Fault Interrupt */
  MemoryManagement_IRQn        = -12,              /**< Cortex-M4 Memory Management Interrupt */
  BusFault_IRQn                = -11,              /**< Cortex-M4 Bus Fault Interrupt */
  UsageFault_IRQn              = -10,              /**< Cortex-M4 Usage Fault Interrupt */
  SVCall_IRQn                  = -5,               /**< Cortex-M4 SV Call Interrupt */
  DebugMonitor_IRQn            = -4,               /**< Cortex-M4 Debug Monitor Interrupt */
  PendSV_IRQn                  = -2,               /**< Cortex-M4 Pend SV Interrupt */
  SysTick_IRQn                 = -1,               /**< Cortex-M4 System Tick Interrupt */

  /* Device specific interrupts */
  DMA0_IRQn                    = 0,                /**< DMA Channel 0 Transfer Complete */
  DMA1_IRQn                    = 1,                /**< DMA Channel 1 Transfer Complete */
  DMA2_IRQn                    = 2,                /**< DMA Channel 2 Transfer Complete */
  DMA3_IRQn                    = 3,                /**< DMA Channel 3 Transfer Complete */
  DMA4_IRQn                    = 4,                /**< DMA Channel 4 Transfer Complete */
  DMA5_IRQn                    = 5,                /**< DMA Channel 5 Transfer Complete */
  DMA6_IRQn                    = 6,                /**< DMA Channel 6 Transfer Complete */
  DMA7_IRQn                    = 7,                /**< DMA Channel 7 Transfer Complete */
  DMA8_IRQn                    = 8,                /**< DMA Channel 8 Transfer Complete */
  DMA9_IRQn                    = 9,                /**< DMA Channel 9 Transfer Complete */
  DMA10_IRQn                   = 10,               /**< DMA Channel 10 Transfer Complete */
  DMA11_IRQn                   = 11,               /**< DMA Channel 11 Transfer Complete */
  DMA12_IRQn                   = 12,               /**< DMA Channel 12 Transfer Complete */
  DMA13_IRQn                   = 13,               /**< DMA Channel 13 Transfer Complete */
  DMA14_IRQn                   = 14,               /**< DMA Channel 14 Transfer Complete */
  DMA15_IRQn                   = 15,               /**< DMA Channel 15 Transfer Complete */
  DMA_Error_IRQn               = 16,               /**< DMA Error Interrupt */
  MCM_IRQn                     = 17,               /**< Normal Interrupt */
  FTFE_IRQn                    = 18,               /**< FTFE Command complete interrupt */
  Read_Collision_IRQn          = 19,               /**< Read Collision Interrupt */
  LVD_LVW_IRQn                 = 20,               /**< Low Voltage Detect, Low Voltage Warning */
  LLWU_IRQn                    = 21,               /**< Low Leakage Wakeup Unit */
  WDOG_EWM_IRQn                = 22,               /**< WDOG Interrupt */
  RNG_IRQn                     = 23,               /**< RNG Interrupt */
  I2C0_IRQn                    = 24,               /**< I2C0 interrupt */
  I2C1_IRQn                    = 25,               /**< I2C1 interrupt */
  SPI0_IRQn                    = 26,               /**< SPI0 Interrupt */
  SPI1_IRQn                    = 27,               /**< SPI1 Interrupt */
  I2S0_Tx_IRQn                 = 28,               /**< I2S0 transmit interrupt */
  I2S0_Rx_IRQn                 = 29,               /**< I2S0 receive interrupt */
  UART0_LON_IRQn               = 30,               /**< UART0 LON interrupt */
  UART0Status_IRQn             = 31,               /**< UART0 Receive/Transmit interrupt */
  UART0Error_IRQn              = 32,               /**< UART0 Error interrupt */
  UART1Status_IRQn             = 33,               /**< UART1 Receive/Transmit interrupt */
  UART1Error_IRQn              = 34,               /**< UART1 Error interrupt */
  UART2Status_IRQn             = 35,               /**< UART2 Receive/Transmit interrupt */
  UART2Error_IRQn              = 36,               /**< UART2 Error interrupt */
  UART3Status_IRQn             = 37,               /**< UART3 Receive/Transmit interrupt */
  UART3Error_IRQn              = 38,               /**< UART3 Error interrupt */
  ADC0_IRQn                    = 39,               /**< ADC0 interrupt */
  CMP0_IRQn                    = 40,               /**< CMP0 interrupt */
  CMP1_IRQn                    = 41,               /**< CMP1 interrupt */
  FTM0_IRQn                    = 42,               /**< FTM0 fault, overflow and channels interrupt */
  FTM1_IRQn                    = 43,               /**< FTM1 fault, overflow and channels interrupt */
  FTM2_IRQn                    = 44,               /**< FTM2 fault, overflow and channels interrupt */
  CMT_IRQn                     = 45,               /**< CMT interrupt */
  RTC_IRQn                     = 46,               /**< RTC interrupt */
  RTC_Seconds_IRQn             = 47,               /**< RTC seconds interrupt */
  PIT0_IRQn                    = 48,               /**< PIT timer channel 0 interrupt */
  PIT1_IRQn                    = 49,               /**< PIT timer channel 1 interrupt */
  PIT2_IRQn                    = 50,               /**< PIT timer channel 2 interrupt */
  PIT3_IRQn                    = 51,               /**< PIT timer channel 3 interrupt */
  PDB0_IRQn                    = 52,               /**< PDB0 Interrupt */
  USB0_IRQn                    = 53,               /**< USB0 interrupt */
  USBDCD_IRQn                  = 54,               /**< USBDCD Interrupt */
  Reserved71_IRQn              = 55,               /**< Reserved interrupt 71 */
  DAC0_IRQn                    = 56,               /**< DAC0 interrupt */
  MCG_IRQn                     = 57,               /**< MCG Interrupt */
  LPTMR0_IRQn                  = 58,               /**< LPTimer interrupt */
  PORTA_IRQn                   = 59,               /**< Port A interrupt */
  PORTB_IRQn                   = 60,               /**< Port B interrupt */
  PORTC_IRQn                   = 61,               /**< Port C interrupt */
  PORTD_IRQn                   = 62,               /**< Port D interrupt */
  PORTE_IRQn                   = 63,               /**< Port E interrupt */
  SWI_IRQn                     = 64,               /**< Software interrupt */
  SPI2_IRQn                    = 65,               /**< SPI2 Interrupt */
  UART4Status_IRQn             = 66,               /**< UART4 Receive/Transmit interrupt */
  UART4Error_IRQn              = 67,               /**< UART4 Error interrupt */
  UART5Status_IRQn             = 68,               /**< UART5 Receive/Transmit interrupt */
  UART5Error_IRQn              = 69,               /**< UART5 Error interrupt */
  CMP2_IRQn                    = 70,               /**< CMP2 interrupt */
  FTM3_IRQn                    = 71,               /**< FTM3 fault, overflow and channels interrupt */
  DAC1_IRQn                    = 72,               /**< DAC1 interrupt */
  ADC1_IRQn                    = 73,               /**< ADC1 interrupt */
  I2C2_IRQn                    = 74,               /**< I2C2 interrupt */
  CAN0_ORed_Message_buffer_IRQn = 75,              /**< CAN0 OR'd message buffers interrupt */
  CAN0_Bus_Off_IRQn            = 76,               /**< CAN0 bus off interrupt */
  CAN0_Error_IRQn              = 77,               /**< CAN0 error interrupt */
  CAN0_Tx_Warning_IRQn         = 78,               /**< CAN0 Tx warning interrupt */
  CAN0_Rx_Warning_IRQn         = 79,               /**< CAN0 Rx warning interrupt */
  CAN0_Wake_Up_IRQn            = 80,               /**< CAN0 wake up interrupt */
  SDHC_IRQn                    = 81,               /**< SDHC interrupt */
  ENET_1588_Timer_IRQn         = 82,               /**< Ethernet MAC IEEE 1588 Timer Interrupt */
  ENET_Transmit_IRQn           = 83,               /**< Ethernet MAC Transmit Interrupt */
  ENET_Receive_IRQn            = 84,               /**< Ethernet MAC Receive Interrupt */
  ENET_Error_IRQn              = 85                /**< Ethernet MAC Error and miscelaneous Interrupt */
} IRQn_Type;

/*!
 * @}
 */ /* end of group Interrupt_vector_numbers */


/* ----------------------------------------------------------------------------
   -- Cortex M4 Core Configuration
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup Cortex_Core_Configuration Cortex M4 Core Configuration
 * @{
 */

#define __MPU_PRESENT                  0         /**< Defines if an MPU is present or not */
#define __NVIC_PRIO_BITS               4         /**< Number of priority bits implemented in the NVIC */
#define __Vendor_SysTickConfig         0         /**< Vendor specific implementation of SysTickConfig is defined */
#define __FPU_PRESENT                  1         /**< Defines if an FPU is present or not */

#include "core_cm4.h"                  /* Core Peripheral Access Layer */
/* #include "system_MK64F12.h"         /+ Device specific configuration file */

/*!
 * @}
 */ /* end of group Cortex_Core_Configuration */


/* ----------------------------------------------------------------------------
   -- Mapping Information
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup Mapping_Information Mapping Information
 * @{
 */

/** Mapping Information */
/*!
 * @addtogroup edma_request
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Structure for the DMA hardware request
 *
 * Defines the structure for the DMA hardware request collections. The user can configure the
 * hardware request into DMAMUX to trigger the DMA transfer accordingly. The index
 * of the hardware request varies according  to the to SoC.
 */
typedef enum _dma_request_source
{
    kDmaRequestMux0Disable          = 0|0x100U,    /**< DMAMUX TriggerDisabled. */
    kDmaRequestMux0Reserved1        = 1|0x100U,    /**< Reserved1 */
    kDmaRequestMux0UART0Rx          = 2|0x100U,    /**< UART0 Receive. */
    kDmaRequestMux0UART0Tx          = 3|0x100U,    /**< UART0 Transmit. */
    kDmaRequestMux0UART1Rx          = 4|0x100U,    /**< UART1 Receive. */
    kDmaRequestMux0UART1Tx          = 5|0x100U,    /**< UART1 Transmit. */
    kDmaRequestMux0UART2Rx          = 6|0x100U,    /**< UART2 Receive. */
    kDmaRequestMux0UART2Tx          = 7|0x100U,    /**< UART2 Transmit. */
    kDmaRequestMux0UART3Rx          = 8|0x100U,    /**< UART3 Receive. */
    kDmaRequestMux0UART3Tx          = 9|0x100U,    /**< UART3 Transmit. */
    kDmaRequestMux0UART4            = 10|0x100U,   /**< UART4 Transmit or Receive. */
    kDmaRequestMux0UART5            = 11|0x100U,   /**< UART5 Transmit or Receive. */
    kDmaRequestMux0I2S0Rx           = 12|0x100U,   /**< I2S0 Receive. */
    kDmaRequestMux0I2S0Tx           = 13|0x100U,   /**< I2S0 Transmit. */
    kDmaRequestMux0SPI0Rx           = 14|0x100U,   /**< SPI0 Receive. */
    kDmaRequestMux0SPI0Tx           = 15|0x100U,   /**< SPI0 Transmit. */
    kDmaRequestMux0SPI1             = 16|0x100U,   /**< SPI1 Transmit or Receive. */
    kDmaRequestMux0SPI2             = 17|0x100U,   /**< SPI2 Transmit or Receive. */
    kDmaRequestMux0I2C0             = 18|0x100U,   /**< I2C0. */
    kDmaRequestMux0I2C1I2C2         = 19|0x100U,   /**< I2C1 and I2C2. */
    kDmaRequestMux0I2C1             = 19|0x100U,   /**< I2C1 and I2C2. */
    kDmaRequestMux0I2C2             = 19|0x100U,   /**< I2C1 and I2C2. */
    kDmaRequestMux0FTM0Channel0     = 20|0x100U,   /**< FTM0 C0V. */
    kDmaRequestMux0FTM0Channel1     = 21|0x100U,   /**< FTM0 C1V. */
    kDmaRequestMux0FTM0Channel2     = 22|0x100U,   /**< FTM0 C2V. */
    kDmaRequestMux0FTM0Channel3     = 23|0x100U,   /**< FTM0 C3V. */
    kDmaRequestMux0FTM0Channel4     = 24|0x100U,   /**< FTM0 C4V. */
    kDmaRequestMux0FTM0Channel5     = 25|0x100U,   /**< FTM0 C5V. */
    kDmaRequestMux0FTM0Channel6     = 26|0x100U,   /**< FTM0 C6V. */
    kDmaRequestMux0FTM0Channel7     = 27|0x100U,   /**< FTM0 C7V. */
    kDmaRequestMux0FTM1Channel0     = 28|0x100U,   /**< FTM1 C0V. */
    kDmaRequestMux0FTM1Channel1     = 29|0x100U,   /**< FTM1 C1V. */
    kDmaRequestMux0FTM2Channel0     = 30|0x100U,   /**< FTM2 C0V. */
    kDmaRequestMux0FTM2Channel1     = 31|0x100U,   /**< FTM2 C1V. */
    kDmaRequestMux0FTM3Channel0     = 32|0x100U,   /**< FTM3 C0V. */
    kDmaRequestMux0FTM3Channel1     = 33|0x100U,   /**< FTM3 C1V. */
    kDmaRequestMux0FTM3Channel2     = 34|0x100U,   /**< FTM3 C2V. */
    kDmaRequestMux0FTM3Channel3     = 35|0x100U,   /**< FTM3 C3V. */
    kDmaRequestMux0FTM3Channel4     = 36|0x100U,   /**< FTM3 C4V. */
    kDmaRequestMux0FTM3Channel5     = 37|0x100U,   /**< FTM3 C5V. */
    kDmaRequestMux0FTM3Channel6     = 38|0x100U,   /**< FTM3 C6V. */
    kDmaRequestMux0FTM3Channel7     = 39|0x100U,   /**< FTM3 C7V. */
    kDmaRequestMux0ADC0             = 40|0x100U,   /**< ADC0. */
    kDmaRequestMux0ADC1             = 41|0x100U,   /**< ADC1. */
    kDmaRequestMux0CMP0             = 42|0x100U,   /**< CMP0. */
    kDmaRequestMux0CMP1             = 43|0x100U,   /**< CMP1. */
    kDmaRequestMux0CMP2             = 44|0x100U,   /**< CMP2. */
    kDmaRequestMux0DAC0             = 45|0x100U,   /**< DAC0. */
    kDmaRequestMux0DAC1             = 46|0x100U,   /**< DAC1. */
    kDmaRequestMux0CMT              = 47|0x100U,   /**< CMT. */
    kDmaRequestMux0PDB              = 48|0x100U,   /**< PDB0. */
    kDmaRequestMux0PortA            = 49|0x100U,   /**< PTA. */
    kDmaRequestMux0PortB            = 50|0x100U,   /**< PTB. */
    kDmaRequestMux0PortC            = 51|0x100U,   /**< PTC. */
    kDmaRequestMux0PortD            = 52|0x100U,   /**< PTD. */
    kDmaRequestMux0PortE            = 53|0x100U,   /**< PTE. */
    kDmaRequestMux0IEEE1588Timer0   = 54|0x100U,   /**< ENET IEEE 1588 timer 0. */
    kDmaRequestMux0IEEE1588Timer1   = 55|0x100U,   /**< ENET IEEE 1588 timer 1. */
    kDmaRequestMux0IEEE1588Timer2   = 56|0x100U,   /**< ENET IEEE 1588 timer 2. */
    kDmaRequestMux0IEEE1588Timer3   = 57|0x100U,   /**< ENET IEEE 1588 timer 3. */
    kDmaRequestMux0AlwaysOn58       = 58|0x100U,   /**< DMAMUX Always Enabled slot. */
    kDmaRequestMux0AlwaysOn59       = 59|0x100U,   /**< DMAMUX Always Enabled slot. */
    kDmaRequestMux0AlwaysOn60       = 60|0x100U,   /**< DMAMUX Always Enabled slot. */
    kDmaRequestMux0AlwaysOn61       = 61|0x100U,   /**< DMAMUX Always Enabled slot. */
    kDmaRequestMux0AlwaysOn62       = 62|0x100U,   /**< DMAMUX Always Enabled slot. */
    kDmaRequestMux0AlwaysOn63       = 63|0x100U,   /**< DMAMUX Always Enabled slot. */
} dma_request_source_t;

/* @} */


/*!
 * @}
 */ /* end of group Mapping_Information */


/* ----------------------------------------------------------------------------
   -- Device Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup Peripheral_access_layer Device Peripheral Access Layer
 * @{
 */


/*
** Start of section using anonymous unions
*/

#if defined(__ARMCC_VERSION)
  #pragma push
  #pragma anon_unions
#elif defined(__CWCC__)
  #pragma push
  #pragma cpp_extensions on
#elif defined(__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined(__IAR_SYSTEMS_ICC__)
  #pragma language=extended
#else
  #error Not supported compiler type
#endif

/* ----------------------------------------------------------------------------
   -- ADC Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup ADC_Peripheral_Access_Layer ADC Peripheral Access Layer
 * @{
 */

/** ADC - Register Layout Typedef */
typedef struct {
  __IO uint32_t SC1[2];                            /**< ADC Status and Control Registers 1, array offset: 0x0, array step: 0x4 */
  __IO uint32_t CFG1;                              /**< ADC Configuration Register 1, offset: 0x8 */
  __IO uint32_t CFG2;                              /**< ADC Configuration Register 2, offset: 0xC */
  __I  uint32_t R[2];                              /**< ADC Data Result Register, array offset: 0x10, array step: 0x4 */
  __IO uint32_t CV1;                               /**< Compare Value Registers, offset: 0x18 */
  __IO uint32_t CV2;                               /**< Compare Value Registers, offset: 0x1C */
  __IO uint32_t SC2;                               /**< Status and Control Register 2, offset: 0x20 */
  __IO uint32_t SC3;                               /**< Status and Control Register 3, offset: 0x24 */
  __IO uint32_t OFS;                               /**< ADC Offset Correction Register, offset: 0x28 */
  __IO uint32_t PG;                                /**< ADC Plus-Side Gain Register, offset: 0x2C */
  __IO uint32_t MG;                                /**< ADC Minus-Side Gain Register, offset: 0x30 */
  __IO uint32_t CLPD;                              /**< ADC Plus-Side General Calibration Value Register, offset: 0x34 */
  __IO uint32_t CLPS;                              /**< ADC Plus-Side General Calibration Value Register, offset: 0x38 */
  __IO uint32_t CLP4;                              /**< ADC Plus-Side General Calibration Value Register, offset: 0x3C */
  __IO uint32_t CLP3;                              /**< ADC Plus-Side General Calibration Value Register, offset: 0x40 */
  __IO uint32_t CLP2;                              /**< ADC Plus-Side General Calibration Value Register, offset: 0x44 */
  __IO uint32_t CLP1;                              /**< ADC Plus-Side General Calibration Value Register, offset: 0x48 */
  __IO uint32_t CLP0;                              /**< ADC Plus-Side General Calibration Value Register, offset: 0x4C */
       uint8_t RESERVED_0[4];
  __IO uint32_t CLMD;                              /**< ADC Minus-Side General Calibration Value Register, offset: 0x54 */
  __IO uint32_t CLMS;                              /**< ADC Minus-Side General Calibration Value Register, offset: 0x58 */
  __IO uint32_t CLM4;                              /**< ADC Minus-Side General Calibration Value Register, offset: 0x5C */
  __IO uint32_t CLM3;                              /**< ADC Minus-Side General Calibration Value Register, offset: 0x60 */
  __IO uint32_t CLM2;                              /**< ADC Minus-Side General Calibration Value Register, offset: 0x64 */
  __IO uint32_t CLM1;                              /**< ADC Minus-Side General Calibration Value Register, offset: 0x68 */
  __IO uint32_t CLM0;                              /**< ADC Minus-Side General Calibration Value Register, offset: 0x6C */
} ADC_TypeDef;

/* ----------------------------------------------------------------------------
   -- ADC Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup ADC_Register_Masks ADC Register Masks
 * @{
 */

/*! @name SC1 - ADC Status and Control Registers 1 */
#define ADCx_SC1n_ADCH_MASK                      (0x1FU)
#define ADCx_SC1n_ADCH_SHIFT                     (0U)
#define ADCx_SC1n_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_SC1n_ADCH_SHIFT)) & ADCx_SC1n_ADCH_MASK)
#define ADCx_SC1n_DIFF                           (0x20U)
#define ADCx_SC1n_AIEN                           (0x40U)
#define ADCx_SC1n_COCO                           (0x80U)

/* The count of ADC_SC1 */
#define ADCx_SC1_COUNT                           (2U)

/*! @name CFG1 - ADC Configuration Register 1 */
#define ADCx_CFG1_ADICLK_MASK                    (0x3U)
#define ADCx_CFG1_ADICLK_SHIFT                   (0U)
#define ADCx_CFG1_ADICLK(x)                      (((uint32_t)(((uint32_t)(x)) << ADCx_CFG1_ADICLK_SHIFT)) & ADCx_CFG1_ADICLK_MASK)
#define ADCx_CFG1_MODE_MASK                      (0xCU)
#define ADCx_CFG1_MODE_SHIFT                     (2U)
#define ADCx_CFG1_MODE(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CFG1_MODE_SHIFT)) & ADCx_CFG1_MODE_MASK)
#define ADCx_CFG1_ADLSMP                         (0x10U)
#define ADCx_CFG1_ADIV_MASK                      (0x60U)
#define ADCx_CFG1_ADIV_SHIFT                     (5U)
#define ADCx_CFG1_ADIV(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CFG1_ADIV_SHIFT)) & ADCx_CFG1_ADIV_MASK)
#define ADCx_CFG1_ADLPC                          (0x80U)

/*! @name CFG2 - ADC Configuration Register 2 */
#define ADCx_CFG2_ADLSTS_MASK                    (0x3U)
#define ADCx_CFG2_ADLSTS_SHIFT                   (0U)
#define ADCx_CFG2_ADLSTS(x)                      (((uint32_t)(((uint32_t)(x)) << ADCx_CFG2_ADLSTS_SHIFT)) & ADCx_CFG2_ADLSTS_MASK)
#define ADCx_CFG2_ADHSC                          (0x4U)
#define ADCx_CFG2_ADACKEN                        (0x8U)
#define ADCx_CFG2_MUXSEL                         (0x10U)

/*! @name R - ADC Data Result Register */
#define ADCx_Rn_D_MASK                           (0xFFFFU)
#define ADCx_Rn_D_SHIFT                          (0U)
#define ADCx_Rn_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADCx_R_D_SHIFT)) & ADCx_R_D_MASK)

/* The count of ADC_R */
#define ADCx_R_COUNT                             (2U)

/*! @name CV1 - Compare Value Registers */
#define ADCx_CV1_CV_MASK                         (0xFFFFU)
#define ADCx_CV1_CV_SHIFT                        (0U)
#define ADCx_CV1_CV(x)                           (((uint32_t)(((uint32_t)(x)) << ADCx_CV1_CV_SHIFT)) & ADCx_CV1_CV_MASK)

/*! @name CV2 - Compare Value Registers */
#define ADCx_CV2_CV_MASK                         (0xFFFFU)
#define ADCx_CV2_CV_SHIFT                        (0U)
#define ADCx_CV2_CV(x)                           (((uint32_t)(((uint32_t)(x)) << ADCx_CV2_CV_SHIFT)) & ADCx_CV2_CV_MASK)

/*! @name SC2 - Status and Control Register 2 */
#define ADCx_SC2_REFSEL_MASK                     (0x3U)
#define ADCx_SC2_REFSEL_SHIFT                    (0U)
#define ADCx_SC2_REFSEL(x)                       (((uint32_t)(((uint32_t)(x)) << ADCx_SC2_REFSEL_SHIFT)) & ADCx_SC2_REFSEL_MASK)
#define ADCx_SC2_DMAEN                           (0x4U)
#define ADCx_SC2_ACREN                           (0x8U)
#define ADCx_SC2_ACFGT                           (0x10U)
#define ADCx_SC2_ACFE                            (0x20U)
#define ADCx_SC2_ADTRG                           (0x40U)
#define ADCx_SC2_ADACT                           (0x80U)

/*! @name SC3 - Status and Control Register 3 */
#define ADCx_SC3_AVGS_MASK                       (0x3U)
#define ADCx_SC3_AVGS_SHIFT                      (0U)
#define ADCx_SC3_AVGS(x)                         (((uint32_t)(((uint32_t)(x)) << ADCx_SC3_AVGS_SHIFT)) & ADCx_SC3_AVGS_MASK)
#define ADCx_SC3_AVGE                            (0x4U)
#define ADCx_SC3_ADCO                            (0x8U)
#define ADCx_SC3_CALF                            (0x40U)
#define ADCx_SC3_CAL                             (0x80U)

/*! @name OFS - ADC Offset Correction Register */
#define ADCx_OFS_OFS_MASK                        (0xFFFFU)
#define ADCx_OFS_OFS_SHIFT                       (0U)
#define ADCx_OFS_OFS(x)                          (((uint32_t)(((uint32_t)(x)) << ADCx_OFS_OFS_SHIFT)) & ADCx_OFS_OFS_MASK)

/*! @name PG - ADC Plus-Side Gain Register */
#define ADCx_PG_PG_MASK                          (0xFFFFU)
#define ADCx_PG_PG_SHIFT                         (0U)
#define ADCx_PG_PG(x)                            (((uint32_t)(((uint32_t)(x)) << ADCx_PG_PG_SHIFT)) & ADCx_PG_PG_MASK)

/*! @name MG - ADC Minus-Side Gain Register */
#define ADCx_MG_MG_MASK                          (0xFFFFU)
#define ADCx_MG_MG_SHIFT                         (0U)
#define ADCx_MG_MG(x)                            (((uint32_t)(((uint32_t)(x)) << ADCx_MG_MG_SHIFT)) & ADCx_MG_MG_MASK)

/*! @name CLPD - ADC Plus-Side General Calibration Value Register */
#define ADCx_CLPD_CLPD_MASK                      (0x3FU)
#define ADCx_CLPD_CLPD_SHIFT                     (0U)
#define ADCx_CLPD_CLPD(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CLPD_CLPD_SHIFT)) & ADCx_CLPD_CLPD_MASK)

/*! @name CLPS - ADC Plus-Side General Calibration Value Register */
#define ADCx_CLPS_CLPS_MASK                      (0x3FU)
#define ADCx_CLPS_CLPS_SHIFT                     (0U)
#define ADCx_CLPS_CLPS(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CLPS_CLPS_SHIFT)) & ADCx_CLPS_CLPS_MASK)

/*! @name CLP4 - ADC Plus-Side General Calibration Value Register */
#define ADCx_CLP4_CLP4_MASK                      (0x3FFU)
#define ADCx_CLP4_CLP4_SHIFT                     (0U)
#define ADCx_CLP4_CLP4(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CLP4_CLP4_SHIFT)) & ADCx_CLP4_CLP4_MASK)

/*! @name CLP3 - ADC Plus-Side General Calibration Value Register */
#define ADCx_CLP3_CLP3_MASK                      (0x1FFU)
#define ADCx_CLP3_CLP3_SHIFT                     (0U)
#define ADCx_CLP3_CLP3(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CLP3_CLP3_SHIFT)) & ADCx_CLP3_CLP3_MASK)

/*! @name CLP2 - ADC Plus-Side General Calibration Value Register */
#define ADCx_CLP2_CLP2_MASK                      (0xFFU)
#define ADCx_CLP2_CLP2_SHIFT                     (0U)
#define ADCx_CLP2_CLP2(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CLP2_CLP2_SHIFT)) & ADCx_CLP2_CLP2_MASK)

/*! @name CLP1 - ADC Plus-Side General Calibration Value Register */
#define ADCx_CLP1_CLP1_MASK                      (0x7FU)
#define ADCx_CLP1_CLP1_SHIFT                     (0U)
#define ADCx_CLP1_CLP1(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CLP1_CLP1_SHIFT)) & ADCx_CLP1_CLP1_MASK)

/*! @name CLP0 - ADC Plus-Side General Calibration Value Register */
#define ADCx_CLP0_CLP0_MASK                      (0x3FU)
#define ADCx_CLP0_CLP0_SHIFT                     (0U)
#define ADCx_CLP0_CLP0(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CLP0_CLP0_SHIFT)) & ADCx_CLP0_CLP0_MASK)

/*! @name CLMD - ADC Minus-Side General Calibration Value Register */
#define ADCx_CLMD_CLMD_MASK                      (0x3FU)
#define ADCx_CLMD_CLMD_SHIFT                     (0U)
#define ADCx_CLMD_CLMD(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CLMD_CLMD_SHIFT)) & ADCx_CLMD_CLMD_MASK)

/*! @name CLMS - ADC Minus-Side General Calibration Value Register */
#define ADCx_CLMS_CLMS_MASK                      (0x3FU)
#define ADCx_CLMS_CLMS_SHIFT                     (0U)
#define ADCx_CLMS_CLMS(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CLMS_CLMS_SHIFT)) & ADCx_CLMS_CLMS_MASK)

/*! @name CLM4 - ADC Minus-Side General Calibration Value Register */
#define ADCx_CLM4_CLM4_MASK                      (0x3FFU)
#define ADCx_CLM4_CLM4_SHIFT                     (0U)
#define ADCx_CLM4_CLM4(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CLM4_CLM4_SHIFT)) & ADCx_CLM4_CLM4_MASK)

/*! @name CLM3 - ADC Minus-Side General Calibration Value Register */
#define ADCx_CLM3_CLM3_MASK                      (0x1FFU)
#define ADCx_CLM3_CLM3_SHIFT                     (0U)
#define ADCx_CLM3_CLM3(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CLM3_CLM3_SHIFT)) & ADCx_CLM3_CLM3_MASK)

/*! @name CLM2 - ADC Minus-Side General Calibration Value Register */
#define ADCx_CLM2_CLM2_MASK                      (0xFFU)
#define ADCx_CLM2_CLM2_SHIFT                     (0U)
#define ADCx_CLM2_CLM2(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CLM2_CLM2_SHIFT)) & ADCx_CLM2_CLM2_MASK)

/*! @name CLM1 - ADC Minus-Side General Calibration Value Register */
#define ADCx_CLM1_CLM1_MASK                      (0x7FU)
#define ADCx_CLM1_CLM1_SHIFT                     (0U)
#define ADCx_CLM1_CLM1(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CLM1_CLM1_SHIFT)) & ADCx_CLM1_CLM1_MASK)

/*! @name CLM0 - ADC Minus-Side General Calibration Value Register */
#define ADCx_CLM0_CLM0_MASK                      (0x3FU)
#define ADCx_CLM0_CLM0_SHIFT                     (0U)
#define ADCx_CLM0_CLM0(x)                        (((uint32_t)(((uint32_t)(x)) << ADCx_CLM0_CLM0_SHIFT)) & ADCx_CLM0_CLM0_MASK)


/*!
 * @}
 */ /* end of group ADC_Register_Masks */


/* ADC - Peripheral instance base addresses */
/** Peripheral ADC0 base address */
#define ADC0_BASE                                (0x4003B000u)
/** Peripheral ADC0 base pointer */
#define ADC0                                     ((ADC_TypeDef *)ADC0_BASE)
/** Peripheral ADC1 base address */
#define ADC1_BASE                                (0x400BB000u)
/** Peripheral ADC1 base pointer */
#define ADC1                                     ((ADC_TypeDef *)ADC1_BASE)
/** Array initializer of ADC peripheral base addresses */
#define ADC_BASE_ADDRS                           { ADC0_BASE, ADC1_BASE }
/** Array initializer of ADC peripheral base pointers */
#define ADC_BASE_PTRS                            { ADC0, ADC1 }
/** Interrupt vectors for the ADC peripheral type */
#define ADC_IRQS                                 { ADC0_IRQn, ADC1_IRQn }

/*!
 * @}
 */ /* end of group ADC_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- AIPS Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup AIPS_Peripheral_Access_Layer AIPS Peripheral Access Layer
 * @{
 */

/** AIPS - Register Layout Typedef */
typedef struct {
  __IO uint32_t MPRA;                              /**< Master Privilege Register A, offset: 0x0 */
       uint8_t RESERVED_0[28];
  __IO uint32_t PACRA;                             /**< Peripheral Access Control Register, offset: 0x20 */
  __IO uint32_t PACRB;                             /**< Peripheral Access Control Register, offset: 0x24 */
  __IO uint32_t PACRC;                             /**< Peripheral Access Control Register, offset: 0x28 */
  __IO uint32_t PACRD;                             /**< Peripheral Access Control Register, offset: 0x2C */
       uint8_t RESERVED_1[16];
  __IO uint32_t PACRE;                             /**< Peripheral Access Control Register, offset: 0x40 */
  __IO uint32_t PACRF;                             /**< Peripheral Access Control Register, offset: 0x44 */
  __IO uint32_t PACRG;                             /**< Peripheral Access Control Register, offset: 0x48 */
  __IO uint32_t PACRH;                             /**< Peripheral Access Control Register, offset: 0x4C */
  __IO uint32_t PACRI;                             /**< Peripheral Access Control Register, offset: 0x50 */
  __IO uint32_t PACRJ;                             /**< Peripheral Access Control Register, offset: 0x54 */
  __IO uint32_t PACRK;                             /**< Peripheral Access Control Register, offset: 0x58 */
  __IO uint32_t PACRL;                             /**< Peripheral Access Control Register, offset: 0x5C */
  __IO uint32_t PACRM;                             /**< Peripheral Access Control Register, offset: 0x60 */
  __IO uint32_t PACRN;                             /**< Peripheral Access Control Register, offset: 0x64 */
  __IO uint32_t PACRO;                             /**< Peripheral Access Control Register, offset: 0x68 */
  __IO uint32_t PACRP;                             /**< Peripheral Access Control Register, offset: 0x6C */
       uint8_t RESERVED_2[16];
  __IO uint32_t PACRU;                             /**< Peripheral Access Control Register, offset: 0x80 */
} AIPS_TypeDef;

/* ----------------------------------------------------------------------------
   -- AIPS Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup AIPS_Register_Masks AIPS Register Masks
 * @{
 */

/*! @name MPRA - Master Privilege Register A */
#define AIPS_MPRA_MPL5                           (0x100U)
#define AIPS_MPRA_MTW5                           (0x200U)
#define AIPS_MPRA_MTR5                           (0x400U)
#define AIPS_MPRA_MPL4                           (0x1000U)
#define AIPS_MPRA_MTW4                           (0x2000U)
#define AIPS_MPRA_MTR4                           (0x4000U)
#define AIPS_MPRA_MPL3                           (0x10000U)
#define AIPS_MPRA_MTW3                           (0x20000U)
#define AIPS_MPRA_MTR3                           (0x40000U)
#define AIPS_MPRA_MPL2                           (0x100000U)
#define AIPS_MPRA_MTW2                           (0x200000U)
#define AIPS_MPRA_MTR2                           (0x400000U)
#define AIPS_MPRA_MPL1                           (0x1000000U)
#define AIPS_MPRA_MTW1                           (0x2000000U)
#define AIPS_MPRA_MTR1                           (0x4000000U)
#define AIPS_MPRA_MPL0                           (0x10000000U)
#define AIPS_MPRA_MTW0                           (0x20000000U)
#define AIPS_MPRA_MTR0                           (0x40000000U)

/*! @name PACRA - Peripheral Access Control Register */
#define AIPS_PACRA_TP7                           (0x1U)
#define AIPS_PACRA_WP7                           (0x2U)
#define AIPS_PACRA_SP7                           (0x4U)
#define AIPS_PACRA_TP6                           (0x10U)
#define AIPS_PACRA_WP6                           (0x20U)
#define AIPS_PACRA_SP6                           (0x40U)
#define AIPS_PACRA_TP5                           (0x100U)
#define AIPS_PACRA_WP5                           (0x200U)
#define AIPS_PACRA_SP5                           (0x400U)
#define AIPS_PACRA_TP4                           (0x1000U)
#define AIPS_PACRA_WP4                           (0x2000U)
#define AIPS_PACRA_SP4                           (0x4000U)
#define AIPS_PACRA_TP3                           (0x10000U)
#define AIPS_PACRA_WP3                           (0x20000U)
#define AIPS_PACRA_SP3                           (0x40000U)
#define AIPS_PACRA_TP2                           (0x100000U)
#define AIPS_PACRA_WP2                           (0x200000U)
#define AIPS_PACRA_SP2                           (0x400000U)
#define AIPS_PACRA_TP1                           (0x1000000U)
#define AIPS_PACRA_WP1                           (0x2000000U)
#define AIPS_PACRA_SP1                           (0x4000000U)
#define AIPS_PACRA_TP0                           (0x10000000U)
#define AIPS_PACRA_WP0                           (0x20000000U)
#define AIPS_PACRA_SP0                           (0x40000000U)

/*! @name PACRB - Peripheral Access Control Register */
#define AIPS_PACRB_TP7                           (0x1U)
#define AIPS_PACRB_WP7                           (0x2U)
#define AIPS_PACRB_SP7                           (0x4U)
#define AIPS_PACRB_TP6                           (0x10U)
#define AIPS_PACRB_WP6                           (0x20U)
#define AIPS_PACRB_SP6                           (0x40U)
#define AIPS_PACRB_TP5                           (0x100U)
#define AIPS_PACRB_WP5                           (0x200U)
#define AIPS_PACRB_SP5                           (0x400U)
#define AIPS_PACRB_TP4                           (0x1000U)
#define AIPS_PACRB_WP4                           (0x2000U)
#define AIPS_PACRB_SP4                           (0x4000U)
#define AIPS_PACRB_TP3                           (0x10000U)
#define AIPS_PACRB_WP3                           (0x20000U)
#define AIPS_PACRB_SP3                           (0x40000U)
#define AIPS_PACRB_TP2                           (0x100000U)
#define AIPS_PACRB_WP2                           (0x200000U)
#define AIPS_PACRB_SP2                           (0x400000U)
#define AIPS_PACRB_TP1                           (0x1000000U)
#define AIPS_PACRB_WP1                           (0x2000000U)
#define AIPS_PACRB_SP1                           (0x4000000U)
#define AIPS_PACRB_TP0                           (0x10000000U)
#define AIPS_PACRB_WP0                           (0x20000000U)
#define AIPS_PACRB_SP0                           (0x40000000U)

/*! @name PACRC - Peripheral Access Control Register */
#define AIPS_PACRC_TP7                           (0x1U)
#define AIPS_PACRC_WP7                           (0x2U)
#define AIPS_PACRC_SP7                           (0x4U)
#define AIPS_PACRC_TP6                           (0x10U)
#define AIPS_PACRC_WP6                           (0x20U)
#define AIPS_PACRC_SP6                           (0x40U)
#define AIPS_PACRC_TP5                           (0x100U)
#define AIPS_PACRC_WP5                           (0x200U)
#define AIPS_PACRC_SP5                           (0x400U)
#define AIPS_PACRC_TP4                           (0x1000U)
#define AIPS_PACRC_WP4                           (0x2000U)
#define AIPS_PACRC_SP4                           (0x4000U)
#define AIPS_PACRC_TP3                           (0x10000U)
#define AIPS_PACRC_WP3                           (0x20000U)
#define AIPS_PACRC_SP3                           (0x40000U)
#define AIPS_PACRC_TP2                           (0x100000U)
#define AIPS_PACRC_WP2                           (0x200000U)
#define AIPS_PACRC_SP2                           (0x400000U)
#define AIPS_PACRC_TP1                           (0x1000000U)
#define AIPS_PACRC_WP1                           (0x2000000U)
#define AIPS_PACRC_SP1                           (0x4000000U)
#define AIPS_PACRC_TP0                           (0x10000000U)
#define AIPS_PACRC_WP0                           (0x20000000U)
#define AIPS_PACRC_SP0                           (0x40000000U)

/*! @name PACRD - Peripheral Access Control Register */
#define AIPS_PACRD_TP7                           (0x1U)
#define AIPS_PACRD_WP7                           (0x2U)
#define AIPS_PACRD_SP7                           (0x4U)
#define AIPS_PACRD_TP6                           (0x10U)
#define AIPS_PACRD_WP6                           (0x20U)
#define AIPS_PACRD_SP6                           (0x40U)
#define AIPS_PACRD_TP5                           (0x100U)
#define AIPS_PACRD_WP5                           (0x200U)
#define AIPS_PACRD_SP5                           (0x400U)
#define AIPS_PACRD_TP4                           (0x1000U)
#define AIPS_PACRD_WP4                           (0x2000U)
#define AIPS_PACRD_SP4                           (0x4000U)
#define AIPS_PACRD_TP3                           (0x10000U)
#define AIPS_PACRD_WP3                           (0x20000U)
#define AIPS_PACRD_SP3                           (0x40000U)
#define AIPS_PACRD_TP2                           (0x100000U)
#define AIPS_PACRD_WP2                           (0x200000U)
#define AIPS_PACRD_SP2                           (0x400000U)
#define AIPS_PACRD_TP1                           (0x1000000U)
#define AIPS_PACRD_WP1                           (0x2000000U)
#define AIPS_PACRD_SP1                           (0x4000000U)
#define AIPS_PACRD_TP0                           (0x10000000U)
#define AIPS_PACRD_WP0                           (0x20000000U)
#define AIPS_PACRD_SP0                           (0x40000000U)

/*! @name PACRE - Peripheral Access Control Register */
#define AIPS_PACRE_TP7                           (0x1U)
#define AIPS_PACRE_WP7                           (0x2U)
#define AIPS_PACRE_SP7                           (0x4U)
#define AIPS_PACRE_TP6                           (0x10U)
#define AIPS_PACRE_WP6                           (0x20U)
#define AIPS_PACRE_SP6                           (0x40U)
#define AIPS_PACRE_TP5                           (0x100U)
#define AIPS_PACRE_WP5                           (0x200U)
#define AIPS_PACRE_SP5                           (0x400U)
#define AIPS_PACRE_TP4                           (0x1000U)
#define AIPS_PACRE_WP4                           (0x2000U)
#define AIPS_PACRE_SP4                           (0x4000U)
#define AIPS_PACRE_TP3                           (0x10000U)
#define AIPS_PACRE_WP3                           (0x20000U)
#define AIPS_PACRE_SP3                           (0x40000U)
#define AIPS_PACRE_TP2                           (0x100000U)
#define AIPS_PACRE_WP2                           (0x200000U)
#define AIPS_PACRE_SP2                           (0x400000U)
#define AIPS_PACRE_TP1                           (0x1000000U)
#define AIPS_PACRE_WP1                           (0x2000000U)
#define AIPS_PACRE_SP1                           (0x4000000U)
#define AIPS_PACRE_TP0                           (0x10000000U)
#define AIPS_PACRE_WP0                           (0x20000000U)
#define AIPS_PACRE_SP0                           (0x40000000U)

/*! @name PACRF - Peripheral Access Control Register */
#define AIPS_PACRF_TP7                           (0x1U)
#define AIPS_PACRF_WP7                           (0x2U)
#define AIPS_PACRF_SP7                           (0x4U)
#define AIPS_PACRF_TP6                           (0x10U)
#define AIPS_PACRF_WP6                           (0x20U)
#define AIPS_PACRF_SP6                           (0x40U)
#define AIPS_PACRF_TP5                           (0x100U)
#define AIPS_PACRF_WP5                           (0x200U)
#define AIPS_PACRF_SP5                           (0x400U)
#define AIPS_PACRF_TP4                           (0x1000U)
#define AIPS_PACRF_WP4                           (0x2000U)
#define AIPS_PACRF_SP4                           (0x4000U)
#define AIPS_PACRF_TP3                           (0x10000U)
#define AIPS_PACRF_WP3                           (0x20000U)
#define AIPS_PACRF_SP3                           (0x40000U)
#define AIPS_PACRF_TP2                           (0x100000U)
#define AIPS_PACRF_WP2                           (0x200000U)
#define AIPS_PACRF_SP2                           (0x400000U)
#define AIPS_PACRF_TP1                           (0x1000000U)
#define AIPS_PACRF_WP1                           (0x2000000U)
#define AIPS_PACRF_SP1                           (0x4000000U)
#define AIPS_PACRF_TP0                           (0x10000000U)
#define AIPS_PACRF_WP0                           (0x20000000U)
#define AIPS_PACRF_SP0                           (0x40000000U)

/*! @name PACRG - Peripheral Access Control Register */
#define AIPS_PACRG_TP7                           (0x1U)
#define AIPS_PACRG_WP7                           (0x2U)
#define AIPS_PACRG_SP7                           (0x4U)
#define AIPS_PACRG_TP6                           (0x10U)
#define AIPS_PACRG_WP6                           (0x20U)
#define AIPS_PACRG_SP6                           (0x40U)
#define AIPS_PACRG_TP5                           (0x100U)
#define AIPS_PACRG_WP5                           (0x200U)
#define AIPS_PACRG_SP5                           (0x400U)
#define AIPS_PACRG_TP4                           (0x1000U)
#define AIPS_PACRG_WP4                           (0x2000U)
#define AIPS_PACRG_SP4                           (0x4000U)
#define AIPS_PACRG_TP3                           (0x10000U)
#define AIPS_PACRG_WP3                           (0x20000U)
#define AIPS_PACRG_SP3                           (0x40000U)
#define AIPS_PACRG_TP2                           (0x100000U)
#define AIPS_PACRG_WP2                           (0x200000U)
#define AIPS_PACRG_SP2                           (0x400000U)
#define AIPS_PACRG_TP1                           (0x1000000U)
#define AIPS_PACRG_WP1                           (0x2000000U)
#define AIPS_PACRG_SP1                           (0x4000000U)
#define AIPS_PACRG_TP0                           (0x10000000U)
#define AIPS_PACRG_WP0                           (0x20000000U)
#define AIPS_PACRG_SP0                           (0x40000000U)

/*! @name PACRH - Peripheral Access Control Register */
#define AIPS_PACRH_TP7                           (0x1U)
#define AIPS_PACRH_WP7                           (0x2U)
#define AIPS_PACRH_SP7                           (0x4U)
#define AIPS_PACRH_TP6                           (0x10U)
#define AIPS_PACRH_WP6                           (0x20U)
#define AIPS_PACRH_SP6                           (0x40U)
#define AIPS_PACRH_TP5                           (0x100U)
#define AIPS_PACRH_WP5                           (0x200U)
#define AIPS_PACRH_SP5                           (0x400U)
#define AIPS_PACRH_TP4                           (0x1000U)
#define AIPS_PACRH_WP4                           (0x2000U)
#define AIPS_PACRH_SP4                           (0x4000U)
#define AIPS_PACRH_TP3                           (0x10000U)
#define AIPS_PACRH_WP3                           (0x20000U)
#define AIPS_PACRH_SP3                           (0x40000U)
#define AIPS_PACRH_TP2                           (0x100000U)
#define AIPS_PACRH_WP2                           (0x200000U)
#define AIPS_PACRH_SP2                           (0x400000U)
#define AIPS_PACRH_TP1                           (0x1000000U)
#define AIPS_PACRH_WP1                           (0x2000000U)
#define AIPS_PACRH_SP1                           (0x4000000U)
#define AIPS_PACRH_TP0                           (0x10000000U)
#define AIPS_PACRH_WP0                           (0x20000000U)
#define AIPS_PACRH_SP0                           (0x40000000U)

/*! @name PACRI - Peripheral Access Control Register */
#define AIPS_PACRI_TP7                           (0x1U)
#define AIPS_PACRI_WP7                           (0x2U)
#define AIPS_PACRI_SP7                           (0x4U)
#define AIPS_PACRI_TP6                           (0x10U)
#define AIPS_PACRI_WP6                           (0x20U)
#define AIPS_PACRI_SP6                           (0x40U)
#define AIPS_PACRI_TP5                           (0x100U)
#define AIPS_PACRI_WP5                           (0x200U)
#define AIPS_PACRI_SP5                           (0x400U)
#define AIPS_PACRI_TP4                           (0x1000U)
#define AIPS_PACRI_WP4                           (0x2000U)
#define AIPS_PACRI_SP4                           (0x4000U)
#define AIPS_PACRI_TP3                           (0x10000U)
#define AIPS_PACRI_WP3                           (0x20000U)
#define AIPS_PACRI_SP3                           (0x40000U)
#define AIPS_PACRI_TP2                           (0x100000U)
#define AIPS_PACRI_WP2                           (0x200000U)
#define AIPS_PACRI_SP2                           (0x400000U)
#define AIPS_PACRI_TP1                           (0x1000000U)
#define AIPS_PACRI_WP1                           (0x2000000U)
#define AIPS_PACRI_SP1                           (0x4000000U)
#define AIPS_PACRI_TP0                           (0x10000000U)
#define AIPS_PACRI_WP0                           (0x20000000U)
#define AIPS_PACRI_SP0                           (0x40000000U)

/*! @name PACRJ - Peripheral Access Control Register */
#define AIPS_PACRJ_TP7                           (0x1U)
#define AIPS_PACRJ_WP7                           (0x2U)
#define AIPS_PACRJ_SP7                           (0x4U)
#define AIPS_PACRJ_TP6                           (0x10U)
#define AIPS_PACRJ_WP6                           (0x20U)
#define AIPS_PACRJ_SP6                           (0x40U)
#define AIPS_PACRJ_TP5                           (0x100U)
#define AIPS_PACRJ_WP5                           (0x200U)
#define AIPS_PACRJ_SP5                           (0x400U)
#define AIPS_PACRJ_TP4                           (0x1000U)
#define AIPS_PACRJ_WP4                           (0x2000U)
#define AIPS_PACRJ_SP4                           (0x4000U)
#define AIPS_PACRJ_TP3                           (0x10000U)
#define AIPS_PACRJ_WP3                           (0x20000U)
#define AIPS_PACRJ_SP3                           (0x40000U)
#define AIPS_PACRJ_TP2                           (0x100000U)
#define AIPS_PACRJ_WP2                           (0x200000U)
#define AIPS_PACRJ_SP2                           (0x400000U)
#define AIPS_PACRJ_TP1                           (0x1000000U)
#define AIPS_PACRJ_WP1                           (0x2000000U)
#define AIPS_PACRJ_SP1                           (0x4000000U)
#define AIPS_PACRJ_TP0                           (0x10000000U)
#define AIPS_PACRJ_WP0                           (0x20000000U)
#define AIPS_PACRJ_SP0                           (0x40000000U)

/*! @name PACRK - Peripheral Access Control Register */
#define AIPS_PACRK_TP7                           (0x1U)
#define AIPS_PACRK_WP7                           (0x2U)
#define AIPS_PACRK_SP7                           (0x4U)
#define AIPS_PACRK_TP6                           (0x10U)
#define AIPS_PACRK_WP6                           (0x20U)
#define AIPS_PACRK_SP6                           (0x40U)
#define AIPS_PACRK_TP5                           (0x100U)
#define AIPS_PACRK_WP5                           (0x200U)
#define AIPS_PACRK_SP5                           (0x400U)
#define AIPS_PACRK_TP4                           (0x1000U)
#define AIPS_PACRK_WP4                           (0x2000U)
#define AIPS_PACRK_SP4                           (0x4000U)
#define AIPS_PACRK_TP3                           (0x10000U)
#define AIPS_PACRK_WP3                           (0x20000U)
#define AIPS_PACRK_SP3                           (0x40000U)
#define AIPS_PACRK_TP2                           (0x100000U)
#define AIPS_PACRK_WP2                           (0x200000U)
#define AIPS_PACRK_SP2                           (0x400000U)
#define AIPS_PACRK_TP1                           (0x1000000U)
#define AIPS_PACRK_WP1                           (0x2000000U)
#define AIPS_PACRK_SP1                           (0x4000000U)
#define AIPS_PACRK_TP0                           (0x10000000U)
#define AIPS_PACRK_WP0                           (0x20000000U)
#define AIPS_PACRK_SP0                           (0x40000000U)

/*! @name PACRL - Peripheral Access Control Register */
#define AIPS_PACRL_TP7                           (0x1U)
#define AIPS_PACRL_WP7                           (0x2U)
#define AIPS_PACRL_SP7                           (0x4U)
#define AIPS_PACRL_TP6                           (0x10U)
#define AIPS_PACRL_WP6                           (0x20U)
#define AIPS_PACRL_SP6                           (0x40U)
#define AIPS_PACRL_TP5                           (0x100U)
#define AIPS_PACRL_WP5                           (0x200U)
#define AIPS_PACRL_SP5                           (0x400U)
#define AIPS_PACRL_TP4                           (0x1000U)
#define AIPS_PACRL_WP4                           (0x2000U)
#define AIPS_PACRL_SP4                           (0x4000U)
#define AIPS_PACRL_TP3                           (0x10000U)
#define AIPS_PACRL_WP3                           (0x20000U)
#define AIPS_PACRL_SP3                           (0x40000U)
#define AIPS_PACRL_TP2                           (0x100000U)
#define AIPS_PACRL_WP2                           (0x200000U)
#define AIPS_PACRL_SP2                           (0x400000U)
#define AIPS_PACRL_TP1                           (0x1000000U)
#define AIPS_PACRL_WP1                           (0x2000000U)
#define AIPS_PACRL_SP1                           (0x4000000U)
#define AIPS_PACRL_TP0                           (0x10000000U)
#define AIPS_PACRL_WP0                           (0x20000000U)
#define AIPS_PACRL_SP0                           (0x40000000U)

/*! @name PACRM - Peripheral Access Control Register */
#define AIPS_PACRM_TP7                           (0x1U)
#define AIPS_PACRM_WP7                           (0x2U)
#define AIPS_PACRM_SP7                           (0x4U)
#define AIPS_PACRM_TP6                           (0x10U)
#define AIPS_PACRM_WP6                           (0x20U)
#define AIPS_PACRM_SP6                           (0x40U)
#define AIPS_PACRM_TP5                           (0x100U)
#define AIPS_PACRM_WP5                           (0x200U)
#define AIPS_PACRM_SP5                           (0x400U)
#define AIPS_PACRM_TP4                           (0x1000U)
#define AIPS_PACRM_WP4                           (0x2000U)
#define AIPS_PACRM_SP4                           (0x4000U)
#define AIPS_PACRM_TP3                           (0x10000U)
#define AIPS_PACRM_WP3                           (0x20000U)
#define AIPS_PACRM_SP3                           (0x40000U)
#define AIPS_PACRM_TP2                           (0x100000U)
#define AIPS_PACRM_WP2                           (0x200000U)
#define AIPS_PACRM_SP2                           (0x400000U)
#define AIPS_PACRM_TP1                           (0x1000000U)
#define AIPS_PACRM_WP1                           (0x2000000U)
#define AIPS_PACRM_SP1                           (0x4000000U)
#define AIPS_PACRM_TP0                           (0x10000000U)
#define AIPS_PACRM_WP0                           (0x20000000U)
#define AIPS_PACRM_SP0                           (0x40000000U)

/*! @name PACRN - Peripheral Access Control Register */
#define AIPS_PACRN_TP7                           (0x1U)
#define AIPS_PACRN_WP7                           (0x2U)
#define AIPS_PACRN_SP7                           (0x4U)
#define AIPS_PACRN_TP6                           (0x10U)
#define AIPS_PACRN_WP6                           (0x20U)
#define AIPS_PACRN_SP6                           (0x40U)
#define AIPS_PACRN_TP5                           (0x100U)
#define AIPS_PACRN_WP5                           (0x200U)
#define AIPS_PACRN_SP5                           (0x400U)
#define AIPS_PACRN_TP4                           (0x1000U)
#define AIPS_PACRN_WP4                           (0x2000U)
#define AIPS_PACRN_SP4                           (0x4000U)
#define AIPS_PACRN_TP3                           (0x10000U)
#define AIPS_PACRN_WP3                           (0x20000U)
#define AIPS_PACRN_SP3                           (0x40000U)
#define AIPS_PACRN_TP2                           (0x100000U)
#define AIPS_PACRN_WP2                           (0x200000U)
#define AIPS_PACRN_SP2                           (0x400000U)
#define AIPS_PACRN_TP1                           (0x1000000U)
#define AIPS_PACRN_WP1                           (0x2000000U)
#define AIPS_PACRN_SP1                           (0x4000000U)
#define AIPS_PACRN_TP0                           (0x10000000U)
#define AIPS_PACRN_WP0                           (0x20000000U)
#define AIPS_PACRN_SP0                           (0x40000000U)

/*! @name PACRO - Peripheral Access Control Register */
#define AIPS_PACRO_TP7                           (0x1U)
#define AIPS_PACRO_WP7                           (0x2U)
#define AIPS_PACRO_SP7                           (0x4U)
#define AIPS_PACRO_TP6                           (0x10U)
#define AIPS_PACRO_WP6                           (0x20U)
#define AIPS_PACRO_SP6                           (0x40U)
#define AIPS_PACRO_TP5                           (0x100U)
#define AIPS_PACRO_WP5                           (0x200U)
#define AIPS_PACRO_SP5                           (0x400U)
#define AIPS_PACRO_TP4                           (0x1000U)
#define AIPS_PACRO_WP4                           (0x2000U)
#define AIPS_PACRO_SP4                           (0x4000U)
#define AIPS_PACRO_TP3                           (0x10000U)
#define AIPS_PACRO_WP3                           (0x20000U)
#define AIPS_PACRO_SP3                           (0x40000U)
#define AIPS_PACRO_TP2                           (0x100000U)
#define AIPS_PACRO_WP2                           (0x200000U)
#define AIPS_PACRO_SP2                           (0x400000U)
#define AIPS_PACRO_TP1                           (0x1000000U)
#define AIPS_PACRO_WP1                           (0x2000000U)
#define AIPS_PACRO_SP1                           (0x4000000U)
#define AIPS_PACRO_TP0                           (0x10000000U)
#define AIPS_PACRO_WP0                           (0x20000000U)
#define AIPS_PACRO_SP0                           (0x40000000U)

/*! @name PACRP - Peripheral Access Control Register */
#define AIPS_PACRP_TP7                           (0x1U)
#define AIPS_PACRP_WP7                           (0x2U)
#define AIPS_PACRP_SP7                           (0x4U)
#define AIPS_PACRP_TP6                           (0x10U)
#define AIPS_PACRP_WP6                           (0x20U)
#define AIPS_PACRP_SP6                           (0x40U)
#define AIPS_PACRP_TP5                           (0x100U)
#define AIPS_PACRP_WP5                           (0x200U)
#define AIPS_PACRP_SP5                           (0x400U)
#define AIPS_PACRP_TP4                           (0x1000U)
#define AIPS_PACRP_WP4                           (0x2000U)
#define AIPS_PACRP_SP4                           (0x4000U)
#define AIPS_PACRP_TP3                           (0x10000U)
#define AIPS_PACRP_WP3                           (0x20000U)
#define AIPS_PACRP_SP3                           (0x40000U)
#define AIPS_PACRP_TP2                           (0x100000U)
#define AIPS_PACRP_WP2                           (0x200000U)
#define AIPS_PACRP_SP2                           (0x400000U)
#define AIPS_PACRP_TP1                           (0x1000000U)
#define AIPS_PACRP_WP1                           (0x2000000U)
#define AIPS_PACRP_SP1                           (0x4000000U)
#define AIPS_PACRP_TP0                           (0x10000000U)
#define AIPS_PACRP_WP0                           (0x20000000U)
#define AIPS_PACRP_SP0                           (0x40000000U)

/*! @name PACRU - Peripheral Access Control Register */
#define AIPS_PACRU_TP1                           (0x1000000U)
#define AIPS_PACRU_WP1                           (0x2000000U)
#define AIPS_PACRU_SP1                           (0x4000000U)
#define AIPS_PACRU_TP0                           (0x10000000U)
#define AIPS_PACRU_WP0                           (0x20000000U)
#define AIPS_PACRU_SP0                           (0x40000000U)


/*!
 * @}
 */ /* end of group AIPS_Register_Masks */


/* AIPS - Peripheral instance base addresses */
/** Peripheral AIPS0 base address */
#define AIPS0_BASE                               (0x40000000u)
/** Peripheral AIPS0 base pointer */
#define AIPS0                                    ((AIPS_TypeDef *)AIPS0_BASE)
/** Peripheral AIPS1 base address */
#define AIPS1_BASE                               (0x40080000u)
/** Peripheral AIPS1 base pointer */
#define AIPS1                                    ((AIPS_TypeDef *)AIPS1_BASE)
/** Array initializer of AIPS peripheral base addresses */
#define AIPS_BASE_ADDRS                          { AIPS0_BASE, AIPS1_BASE }
/** Array initializer of AIPS peripheral base pointers */
#define AIPS_BASE_PTRS                           { AIPS0, AIPS1 }

/*!
 * @}
 */ /* end of group AIPS_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- AXBS Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup AXBS_Peripheral_Access_Layer AXBS Peripheral Access Layer
 * @{
 */

/** AXBS - Register Layout Typedef */
typedef struct {
  struct {                                         /* offset: 0x0, array step: 0x100 */
    __IO uint32_t PRS;                               /**< Priority Registers Slave, array offset: 0x0, array step: 0x100 */
         uint8_t RESERVED_0[12];
    __IO uint32_t CRS;                               /**< Control Register, array offset: 0x10, array step: 0x100 */
         uint8_t RESERVED_1[236];
  } SLAVE[5];
       uint8_t RESERVED_0[768];
  __IO uint32_t MGPCR0;                            /**< Master General Purpose Control Register, offset: 0x800 */
       uint8_t RESERVED_1[252];
  __IO uint32_t MGPCR1;                            /**< Master General Purpose Control Register, offset: 0x900 */
       uint8_t RESERVED_2[252];
  __IO uint32_t MGPCR2;                            /**< Master General Purpose Control Register, offset: 0xA00 */
       uint8_t RESERVED_3[252];
  __IO uint32_t MGPCR3;                            /**< Master General Purpose Control Register, offset: 0xB00 */
       uint8_t RESERVED_4[252];
  __IO uint32_t MGPCR4;                            /**< Master General Purpose Control Register, offset: 0xC00 */
       uint8_t RESERVED_5[252];
  __IO uint32_t MGPCR5;                            /**< Master General Purpose Control Register, offset: 0xD00 */
} AXBS_TypeDef;

/* ----------------------------------------------------------------------------
   -- AXBS Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup AXBS_Register_Masks AXBS Register Masks
 * @{
 */

/*! @name PRS - Priority Registers Slave */
#define AXBS_PRS_M0_MASK                         (0x7U)
#define AXBS_PRS_M0_SHIFT                        (0U)
#define AXBS_PRS_M0(x)                           (((uint32_t)(((uint32_t)(x)) << AXBS_PRS_M0_SHIFT)) & AXBS_PRS_M0_MASK)
#define AXBS_PRS_M1_MASK                         (0x70U)
#define AXBS_PRS_M1_SHIFT                        (4U)
#define AXBS_PRS_M1(x)                           (((uint32_t)(((uint32_t)(x)) << AXBS_PRS_M1_SHIFT)) & AXBS_PRS_M1_MASK)
#define AXBS_PRS_M2_MASK                         (0x700U)
#define AXBS_PRS_M2_SHIFT                        (8U)
#define AXBS_PRS_M2(x)                           (((uint32_t)(((uint32_t)(x)) << AXBS_PRS_M2_SHIFT)) & AXBS_PRS_M2_MASK)
#define AXBS_PRS_M3_MASK                         (0x7000U)
#define AXBS_PRS_M3_SHIFT                        (12U)
#define AXBS_PRS_M3(x)                           (((uint32_t)(((uint32_t)(x)) << AXBS_PRS_M3_SHIFT)) & AXBS_PRS_M3_MASK)
#define AXBS_PRS_M4_MASK                         (0x70000U)
#define AXBS_PRS_M4_SHIFT                        (16U)
#define AXBS_PRS_M4(x)                           (((uint32_t)(((uint32_t)(x)) << AXBS_PRS_M4_SHIFT)) & AXBS_PRS_M4_MASK)
#define AXBS_PRS_M5_MASK                         (0x700000U)
#define AXBS_PRS_M5_SHIFT                        (20U)
#define AXBS_PRS_M5(x)                           (((uint32_t)(((uint32_t)(x)) << AXBS_PRS_M5_SHIFT)) & AXBS_PRS_M5_MASK)

/* The count of AXBS_PRS */
#define AXBS_PRS_COUNT                           (5U)

/*! @name CRS - Control Register */
#define AXBS_CRS_PARK_MASK                       (0x7U)
#define AXBS_CRS_PARK_SHIFT                      (0U)
#define AXBS_CRS_PARK(x)                         (((uint32_t)(((uint32_t)(x)) << AXBS_CRS_PARK_SHIFT)) & AXBS_CRS_PARK_MASK)
#define AXBS_CRS_PCTL_MASK                       (0x30U)
#define AXBS_CRS_PCTL_SHIFT                      (4U)
#define AXBS_CRS_PCTL(x)                         (((uint32_t)(((uint32_t)(x)) << AXBS_CRS_PCTL_SHIFT)) & AXBS_CRS_PCTL_MASK)
#define AXBS_CRS_ARB_MASK                        (0x300U)
#define AXBS_CRS_ARB_SHIFT                       (8U)
#define AXBS_CRS_ARB(x)                          (((uint32_t)(((uint32_t)(x)) << AXBS_CRS_ARB_SHIFT)) & AXBS_CRS_ARB_MASK)
#define AXBS_CRS_HLP                             (0x40000000U)
#define AXBS_CRS_RO                              (0x80000000U)

/* The count of AXBS_CRS */
#define AXBS_CRS_COUNT                           (5U)

/*! @name MGPCR0 - Master General Purpose Control Register */
#define AXBS_MGPCR0_AULB_MASK                    (0x7U)
#define AXBS_MGPCR0_AULB_SHIFT                   (0U)
#define AXBS_MGPCR0_AULB(x)                      (((uint32_t)(((uint32_t)(x)) << AXBS_MGPCR0_AULB_SHIFT)) & AXBS_MGPCR0_AULB_MASK)

/*! @name MGPCR1 - Master General Purpose Control Register */
#define AXBS_MGPCR1_AULB_MASK                    (0x7U)
#define AXBS_MGPCR1_AULB_SHIFT                   (0U)
#define AXBS_MGPCR1_AULB(x)                      (((uint32_t)(((uint32_t)(x)) << AXBS_MGPCR1_AULB_SHIFT)) & AXBS_MGPCR1_AULB_MASK)

/*! @name MGPCR2 - Master General Purpose Control Register */
#define AXBS_MGPCR2_AULB_MASK                    (0x7U)
#define AXBS_MGPCR2_AULB_SHIFT                   (0U)
#define AXBS_MGPCR2_AULB(x)                      (((uint32_t)(((uint32_t)(x)) << AXBS_MGPCR2_AULB_SHIFT)) & AXBS_MGPCR2_AULB_MASK)

/*! @name MGPCR3 - Master General Purpose Control Register */
#define AXBS_MGPCR3_AULB_MASK                    (0x7U)
#define AXBS_MGPCR3_AULB_SHIFT                   (0U)
#define AXBS_MGPCR3_AULB(x)                      (((uint32_t)(((uint32_t)(x)) << AXBS_MGPCR3_AULB_SHIFT)) & AXBS_MGPCR3_AULB_MASK)

/*! @name MGPCR4 - Master General Purpose Control Register */
#define AXBS_MGPCR4_AULB_MASK                    (0x7U)
#define AXBS_MGPCR4_AULB_SHIFT                   (0U)
#define AXBS_MGPCR4_AULB(x)                      (((uint32_t)(((uint32_t)(x)) << AXBS_MGPCR4_AULB_SHIFT)) & AXBS_MGPCR4_AULB_MASK)

/*! @name MGPCR5 - Master General Purpose Control Register */
#define AXBS_MGPCR5_AULB_MASK                    (0x7U)
#define AXBS_MGPCR5_AULB_SHIFT                   (0U)
#define AXBS_MGPCR5_AULB(x)                      (((uint32_t)(((uint32_t)(x)) << AXBS_MGPCR5_AULB_SHIFT)) & AXBS_MGPCR5_AULB_MASK)


/*!
 * @}
 */ /* end of group AXBS_Register_Masks */


/* AXBS - Peripheral instance base addresses */
/** Peripheral AXBS base address */
#define AXBS_BASE                                (0x40004000u)
/** Peripheral AXBS base pointer */
#define AXBS                                     ((AXBS_TypeDef *)AXBS_BASE)
/** Array initializer of AXBS peripheral base addresses */
#define AXBS_BASE_ADDRS                          { AXBS_BASE }
/** Array initializer of AXBS peripheral base pointers */
#define AXBS_BASE_PTRS                           { AXBS }

/*!
 * @}
 */ /* end of group AXBS_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- CAN Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup CAN_Peripheral_Access_Layer CAN Peripheral Access Layer
 * @{
 */

/** CAN - Register Layout Typedef */
typedef struct {
  __IO uint32_t MCR;                               /**< Module Configuration Register, offset: 0x0 */
  __IO uint32_t CTRL1;                             /**< Control 1 register, offset: 0x4 */
  __IO uint32_t TIMER;                             /**< Free Running Timer, offset: 0x8 */
       uint8_t RESERVED_0[4];
  __IO uint32_t RXMGMASK;                          /**< Rx Mailboxes Global Mask Register, offset: 0x10 */
  __IO uint32_t RX14MASK;                          /**< Rx 14 Mask register, offset: 0x14 */
  __IO uint32_t RX15MASK;                          /**< Rx 15 Mask register, offset: 0x18 */
  __IO uint32_t ECR;                               /**< Error Counter, offset: 0x1C */
  __IO uint32_t ESR1;                              /**< Error and Status 1 register, offset: 0x20 */
       uint8_t RESERVED_1[4];
  __IO uint32_t IMASK1;                            /**< Interrupt Masks 1 register, offset: 0x28 */
       uint8_t RESERVED_2[4];
  __IO uint32_t IFLAG1;                            /**< Interrupt Flags 1 register, offset: 0x30 */
  __IO uint32_t CTRL2;                             /**< Control 2 register, offset: 0x34 */
  __I  uint32_t ESR2;                              /**< Error and Status 2 register, offset: 0x38 */
       uint8_t RESERVED_3[8];
  __I  uint32_t CRCR;                              /**< CRC Register, offset: 0x44 */
  __IO uint32_t RXFGMASK;                          /**< Rx FIFO Global Mask register, offset: 0x48 */
  __I  uint32_t RXFIR;                             /**< Rx FIFO Information Register, offset: 0x4C */
       uint8_t RESERVED_4[48];
  struct {                                         /* offset: 0x80, array step: 0x10 */
    __IO uint32_t CS;                                /**< Message Buffer 0 CS Register..Message Buffer 15 CS Register, array offset: 0x80, array step: 0x10 */
    __IO uint32_t ID;                                /**< Message Buffer 0 ID Register..Message Buffer 15 ID Register, array offset: 0x84, array step: 0x10 */
    __IO uint32_t WORD0;                             /**< Message Buffer 0 WORD0 Register..Message Buffer 15 WORD0 Register, array offset: 0x88, array step: 0x10 */
    __IO uint32_t WORD1;                             /**< Message Buffer 0 WORD1 Register..Message Buffer 15 WORD1 Register, array offset: 0x8C, array step: 0x10 */
  } MB[16];
       uint8_t RESERVED_5[1792];
  __IO uint32_t RXIMR[16];                         /**< Rx Individual Mask Registers, array offset: 0x880, array step: 0x4 */
} CAN_TypeDef;

/* ----------------------------------------------------------------------------
   -- CAN Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup CAN_Register_Masks CAN Register Masks
 * @{
 */

/*! @name MCR - Module Configuration Register */
#define CAN_MCR_MAXMB_MASK                       (0x7FU)
#define CAN_MCR_MAXMB_SHIFT                      (0U)
#define CAN_MCR_MAXMB(x)                         (((uint32_t)(((uint32_t)(x)) << CAN_MCR_MAXMB_SHIFT)) & CAN_MCR_MAXMB_MASK)
#define CAN_MCR_IDAM_MASK                        (0x300U)
#define CAN_MCR_IDAM_SHIFT                       (8U)
#define CAN_MCR_IDAM(x)                          (((uint32_t)(((uint32_t)(x)) << CAN_MCR_IDAM_SHIFT)) & CAN_MCR_IDAM_MASK)
#define CAN_MCR_AEN                              (0x1000U)
#define CAN_MCR_LPRIOEN                          (0x2000U)
#define CAN_MCR_IRMQ                             (0x10000U)
#define CAN_MCR_SRXDIS                           (0x20000U)
#define CAN_MCR_WAKSRC                           (0x80000U)
#define CAN_MCR_LPMACK                           (0x100000U)
#define CAN_MCR_WRNEN                            (0x200000U)
#define CAN_MCR_SLFWAK                           (0x400000U)
#define CAN_MCR_SUPV                             (0x800000U)
#define CAN_MCR_FRZACK                           (0x1000000U)
#define CAN_MCR_SOFTRST                          (0x2000000U)
#define CAN_MCR_WAKMSK                           (0x4000000U)
#define CAN_MCR_NOTRDY                           (0x8000000U)
#define CAN_MCR_HALT                             (0x10000000U)
#define CAN_MCR_RFEN                             (0x20000000U)
#define CAN_MCR_FRZ                              (0x40000000U)
#define CAN_MCR_MDIS                             (0x80000000U)

/*! @name CTRL1 - Control 1 register */
#define CAN_CTRL1_PROPSEG_MASK                   (0x7U)
#define CAN_CTRL1_PROPSEG_SHIFT                  (0U)
#define CAN_CTRL1_PROPSEG(x)                     (((uint32_t)(((uint32_t)(x)) << CAN_CTRL1_PROPSEG_SHIFT)) & CAN_CTRL1_PROPSEG_MASK)
#define CAN_CTRL1_LOM                            (0x8U)
#define CAN_CTRL1_LBUF                           (0x10U)
#define CAN_CTRL1_TSYN                           (0x20U)
#define CAN_CTRL1_BOFFREC                        (0x40U)
#define CAN_CTRL1_SMP                            (0x80U)
#define CAN_CTRL1_RWRNMSK                        (0x400U)
#define CAN_CTRL1_TWRNMSK                        (0x800U)
#define CAN_CTRL1_LPB                            (0x1000U)
#define CAN_CTRL1_CLKSRC                         (0x2000U)
#define CAN_CTRL1_ERRMSK                         (0x4000U)
#define CAN_CTRL1_BOFFMSK                        (0x8000U)
#define CAN_CTRL1_PSEG2_MASK                     (0x70000U)
#define CAN_CTRL1_PSEG2_SHIFT                    (16U)
#define CAN_CTRL1_PSEG2(x)                       (((uint32_t)(((uint32_t)(x)) << CAN_CTRL1_PSEG2_SHIFT)) & CAN_CTRL1_PSEG2_MASK)
#define CAN_CTRL1_PSEG1_MASK                     (0x380000U)
#define CAN_CTRL1_PSEG1_SHIFT                    (19U)
#define CAN_CTRL1_PSEG1(x)                       (((uint32_t)(((uint32_t)(x)) << CAN_CTRL1_PSEG1_SHIFT)) & CAN_CTRL1_PSEG1_MASK)
#define CAN_CTRL1_RJW_MASK                       (0xC00000U)
#define CAN_CTRL1_RJW_SHIFT                      (22U)
#define CAN_CTRL1_RJW(x)                         (((uint32_t)(((uint32_t)(x)) << CAN_CTRL1_RJW_SHIFT)) & CAN_CTRL1_RJW_MASK)
#define CAN_CTRL1_PRESDIV_MASK                   (0xFF000000U)
#define CAN_CTRL1_PRESDIV_SHIFT                  (24U)
#define CAN_CTRL1_PRESDIV(x)                     (((uint32_t)(((uint32_t)(x)) << CAN_CTRL1_PRESDIV_SHIFT)) & CAN_CTRL1_PRESDIV_MASK)

/*! @name TIMER - Free Running Timer */
#define CAN_TIMER_TIMER_MASK                     (0xFFFFU)
#define CAN_TIMER_TIMER_SHIFT                    (0U)
#define CAN_TIMER_TIMER(x)                       (((uint32_t)(((uint32_t)(x)) << CAN_TIMER_TIMER_SHIFT)) & CAN_TIMER_TIMER_MASK)

/*! @name ECR - Error Counter */
#define CAN_ECR_TXERRCNT_MASK                    (0xFFU)
#define CAN_ECR_TXERRCNT_SHIFT                   (0U)
#define CAN_ECR_TXERRCNT(x)                      (((uint32_t)(((uint32_t)(x)) << CAN_ECR_TXERRCNT_SHIFT)) & CAN_ECR_TXERRCNT_MASK)
#define CAN_ECR_RXERRCNT_MASK                    (0xFF00U)
#define CAN_ECR_RXERRCNT_SHIFT                   (8U)
#define CAN_ECR_RXERRCNT(x)                      (((uint32_t)(((uint32_t)(x)) << CAN_ECR_RXERRCNT_SHIFT)) & CAN_ECR_RXERRCNT_MASK)

/*! @name ESR1 - Error and Status 1 register */
#define CAN_ESR1_WAKINT                          (0x1U)
#define CAN_ESR1_ERRINT                          (0x2U)
#define CAN_ESR1_BOFFINT                         (0x4U)
#define CAN_ESR1_RX                              (0x8U)
#define CAN_ESR1_FLTCONF_MASK                    (0x30U)
#define CAN_ESR1_FLTCONF_SHIFT                   (4U)
#define CAN_ESR1_FLTCONF(x)                      (((uint32_t)(((uint32_t)(x)) << CAN_ESR1_FLTCONF_SHIFT)) & CAN_ESR1_FLTCONF_MASK)
#define CAN_ESR1_TX                              (0x40U)
#define CAN_ESR1_IDLE                            (0x80U)
#define CAN_ESR1_RXWRN                           (0x100U)
#define CAN_ESR1_TXWRN                           (0x200U)
#define CAN_ESR1_STFERR                          (0x400U)
#define CAN_ESR1_FRMERR                          (0x800U)
#define CAN_ESR1_CRCERR                          (0x1000U)
#define CAN_ESR1_ACKERR                          (0x2000U)
#define CAN_ESR1_BIT0ERR                         (0x4000U)
#define CAN_ESR1_BIT1ERR                         (0x8000U)
#define CAN_ESR1_RWRNINT                         (0x10000U)
#define CAN_ESR1_TWRNINT                         (0x20000U)
#define CAN_ESR1_SYNCH                           (0x40000U)

/*! @name IFLAG1 - Interrupt Flags 1 register */
#define CAN_IFLAG1_BUF0I                         (0x1U)
#define CAN_IFLAG1_BUF4TO1I_MASK                 (0x1EU)
#define CAN_IFLAG1_BUF4TO1I_SHIFT                (1U)
#define CAN_IFLAG1_BUF4TO1I(x)                   (((uint32_t)(((uint32_t)(x)) << CAN_IFLAG1_BUF4TO1I_SHIFT)) & CAN_IFLAG1_BUF4TO1I_MASK)
#define CAN_IFLAG1_BUF5I                         (0x20U)
#define CAN_IFLAG1_BUF6I                         (0x40U)
#define CAN_IFLAG1_BUF7I                         (0x80U)
#define CAN_IFLAG1_BUF31TO8I_MASK                (0xFFFFFF00U)
#define CAN_IFLAG1_BUF31TO8I_SHIFT               (8U)
#define CAN_IFLAG1_BUF31TO8I(x)                  (((uint32_t)(((uint32_t)(x)) << CAN_IFLAG1_BUF31TO8I_SHIFT)) & CAN_IFLAG1_BUF31TO8I_MASK)

/*! @name CTRL2 - Control 2 register */
#define CAN_CTRL2_EACEN                          (0x10000U)
#define CAN_CTRL2_RRS                            (0x20000U)
#define CAN_CTRL2_MRP                            (0x40000U)
#define CAN_CTRL2_TASD_MASK                      (0xF80000U)
#define CAN_CTRL2_TASD_SHIFT                     (19U)
#define CAN_CTRL2_TASD(x)                        (((uint32_t)(((uint32_t)(x)) << CAN_CTRL2_TASD_SHIFT)) & CAN_CTRL2_TASD_MASK)
#define CAN_CTRL2_RFFN_MASK                      (0xF000000U)
#define CAN_CTRL2_RFFN_SHIFT                     (24U)
#define CAN_CTRL2_RFFN(x)                        (((uint32_t)(((uint32_t)(x)) << CAN_CTRL2_RFFN_SHIFT)) & CAN_CTRL2_RFFN_MASK)
#define CAN_CTRL2_WRMFRZ                         (0x10000000U)

/*! @name ESR2 - Error and Status 2 register */
#define CAN_ESR2_IMB                             (0x2000U)
#define CAN_ESR2_VPS                             (0x4000U)
#define CAN_ESR2_LPTM_MASK                       (0x7F0000U)
#define CAN_ESR2_LPTM_SHIFT                      (16U)
#define CAN_ESR2_LPTM(x)                         (((uint32_t)(((uint32_t)(x)) << CAN_ESR2_LPTM_SHIFT)) & CAN_ESR2_LPTM_MASK)

/*! @name CRCR - CRC Register */
#define CAN_CRCR_TXCRC_MASK                      (0x7FFFU)
#define CAN_CRCR_TXCRC_SHIFT                     (0U)
#define CAN_CRCR_TXCRC(x)                        (((uint32_t)(((uint32_t)(x)) << CAN_CRCR_TXCRC_SHIFT)) & CAN_CRCR_TXCRC_MASK)
#define CAN_CRCR_MBCRC_MASK                      (0x7F0000U)
#define CAN_CRCR_MBCRC_SHIFT                     (16U)
#define CAN_CRCR_MBCRC(x)                        (((uint32_t)(((uint32_t)(x)) << CAN_CRCR_MBCRC_SHIFT)) & CAN_CRCR_MBCRC_MASK)

/*! @name RXFIR - Rx FIFO Information Register */
#define CAN_RXFIR_IDHIT_MASK                     (0x1FFU)
#define CAN_RXFIR_IDHIT_SHIFT                    (0U)
#define CAN_RXFIR_IDHIT(x)                       (((uint32_t)(((uint32_t)(x)) << CAN_RXFIR_IDHIT_SHIFT)) & CAN_RXFIR_IDHIT_MASK)

/*! @name CS - Message Buffer 0 CS Register..Message Buffer 15 CS Register */
#define CAN_CS_TIME_STAMP_MASK                   (0xFFFFU)
#define CAN_CS_TIME_STAMP_SHIFT                  (0U)
#define CAN_CS_TIME_STAMP(x)                     (((uint32_t)(((uint32_t)(x)) << CAN_CS_TIME_STAMP_SHIFT)) & CAN_CS_TIME_STAMP_MASK)
#define CAN_CS_DLC_MASK                          (0xF0000U)
#define CAN_CS_DLC_SHIFT                         (16U)
#define CAN_CS_DLC(x)                            (((uint32_t)(((uint32_t)(x)) << CAN_CS_DLC_SHIFT)) & CAN_CS_DLC_MASK)
#define CAN_CS_RTR                               (0x100000U)
#define CAN_CS_IDE                               (0x200000U)
#define CAN_CS_SRR                               (0x400000U)
#define CAN_CS_CODE_MASK                         (0xF000000U)
#define CAN_CS_CODE_SHIFT                        (24U)
#define CAN_CS_CODE(x)                           (((uint32_t)(((uint32_t)(x)) << CAN_CS_CODE_SHIFT)) & CAN_CS_CODE_MASK)

/* The count of CAN_CS */
#define CAN_CS_COUNT                             (16U)

/*! @name ID - Message Buffer 0 ID Register..Message Buffer 15 ID Register */
#define CAN_ID_EXT_MASK                          (0x3FFFFU)
#define CAN_ID_EXT_SHIFT                         (0U)
#define CAN_ID_EXT(x)                            (((uint32_t)(((uint32_t)(x)) << CAN_ID_EXT_SHIFT)) & CAN_ID_EXT_MASK)
#define CAN_ID_STD_MASK                          (0x1FFC0000U)
#define CAN_ID_STD_SHIFT                         (18U)
#define CAN_ID_STD(x)                            (((uint32_t)(((uint32_t)(x)) << CAN_ID_STD_SHIFT)) & CAN_ID_STD_MASK)
#define CAN_ID_PRIO_MASK                         (0xE0000000U)
#define CAN_ID_PRIO_SHIFT                        (29U)
#define CAN_ID_PRIO(x)                           (((uint32_t)(((uint32_t)(x)) << CAN_ID_PRIO_SHIFT)) & CAN_ID_PRIO_MASK)

/* The count of CAN_ID */
#define CAN_ID_COUNT                             (16U)

/*! @name WORD0 - Message Buffer 0 WORD0 Register..Message Buffer 15 WORD0 Register */
#define CAN_WORD0_DATA_BYTE_3_MASK               (0xFFU)
#define CAN_WORD0_DATA_BYTE_3_SHIFT              (0U)
#define CAN_WORD0_DATA_BYTE_3(x)                 (((uint32_t)(((uint32_t)(x)) << CAN_WORD0_DATA_BYTE_3_SHIFT)) & CAN_WORD0_DATA_BYTE_3_MASK)
#define CAN_WORD0_DATA_BYTE_2_MASK               (0xFF00U)
#define CAN_WORD0_DATA_BYTE_2_SHIFT              (8U)
#define CAN_WORD0_DATA_BYTE_2(x)                 (((uint32_t)(((uint32_t)(x)) << CAN_WORD0_DATA_BYTE_2_SHIFT)) & CAN_WORD0_DATA_BYTE_2_MASK)
#define CAN_WORD0_DATA_BYTE_1_MASK               (0xFF0000U)
#define CAN_WORD0_DATA_BYTE_1_SHIFT              (16U)
#define CAN_WORD0_DATA_BYTE_1(x)                 (((uint32_t)(((uint32_t)(x)) << CAN_WORD0_DATA_BYTE_1_SHIFT)) & CAN_WORD0_DATA_BYTE_1_MASK)
#define CAN_WORD0_DATA_BYTE_0_MASK               (0xFF000000U)
#define CAN_WORD0_DATA_BYTE_0_SHIFT              (24U)
#define CAN_WORD0_DATA_BYTE_0(x)                 (((uint32_t)(((uint32_t)(x)) << CAN_WORD0_DATA_BYTE_0_SHIFT)) & CAN_WORD0_DATA_BYTE_0_MASK)

/* The count of CAN_WORD0 */
#define CAN_WORD0_COUNT                          (16U)

/*! @name WORD1 - Message Buffer 0 WORD1 Register..Message Buffer 15 WORD1 Register */
#define CAN_WORD1_DATA_BYTE_7_MASK               (0xFFU)
#define CAN_WORD1_DATA_BYTE_7_SHIFT              (0U)
#define CAN_WORD1_DATA_BYTE_7(x)                 (((uint32_t)(((uint32_t)(x)) << CAN_WORD1_DATA_BYTE_7_SHIFT)) & CAN_WORD1_DATA_BYTE_7_MASK)
#define CAN_WORD1_DATA_BYTE_6_MASK               (0xFF00U)
#define CAN_WORD1_DATA_BYTE_6_SHIFT              (8U)
#define CAN_WORD1_DATA_BYTE_6(x)                 (((uint32_t)(((uint32_t)(x)) << CAN_WORD1_DATA_BYTE_6_SHIFT)) & CAN_WORD1_DATA_BYTE_6_MASK)
#define CAN_WORD1_DATA_BYTE_5_MASK               (0xFF0000U)
#define CAN_WORD1_DATA_BYTE_5_SHIFT              (16U)
#define CAN_WORD1_DATA_BYTE_5(x)                 (((uint32_t)(((uint32_t)(x)) << CAN_WORD1_DATA_BYTE_5_SHIFT)) & CAN_WORD1_DATA_BYTE_5_MASK)
#define CAN_WORD1_DATA_BYTE_4_MASK               (0xFF000000U)
#define CAN_WORD1_DATA_BYTE_4_SHIFT              (24U)
#define CAN_WORD1_DATA_BYTE_4(x)                 (((uint32_t)(((uint32_t)(x)) << CAN_WORD1_DATA_BYTE_4_SHIFT)) & CAN_WORD1_DATA_BYTE_4_MASK)

/* The count of CAN_WORD1 */
#define CAN_WORD1_COUNT                          (16U)

/* The count of CAN_RXIMR */
#define CAN_RXIMR_COUNT                          (16U)

/*!
 * @}
 */ /* end of group CAN_Register_Masks */


/* CAN - Peripheral instance base addresses */
/** Peripheral CAN0 base address */
#define CAN0_BASE                                (0x40024000u)
/** Peripheral CAN0 base pointer */
#define CAN0                                     ((CAN_TypeDef *)CAN0_BASE)
/** Array initializer of CAN peripheral base addresses */
#define CAN_BASE_ADDRS                           { CAN0_BASE }
/** Array initializer of CAN peripheral base pointers */
#define CAN_BASE_PTRS                            { CAN0 }
/** Interrupt vectors for the CAN peripheral type */
#define CAN_Rx_Warning_IRQS                      { CAN0_Rx_Warning_IRQn }
#define CAN_Tx_Warning_IRQS                      { CAN0_Tx_Warning_IRQn }
#define CAN_Wake_Up_IRQS                         { CAN0_Wake_Up_IRQn }
#define CAN_Error_IRQS                           { CAN0_Error_IRQn }
#define CAN_Bus_Off_IRQS                         { CAN0_Bus_Off_IRQn }
#define CAN_ORed_Message_buffer_IRQS             { CAN0_ORed_Message_buffer_IRQn }

/*!
 * @}
 */ /* end of group CAN_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- CAU Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup CAU_Peripheral_Access_Layer CAU Peripheral Access Layer
 * @{
 */

/** CAU - Register Layout Typedef */
typedef struct {
  __O  uint32_t DIRECT[16];                        /**< Direct access register 0..Direct access register 15, array offset: 0x0, array step: 0x4 */
       uint8_t RESERVED_0[2048];
  __O  uint32_t LDR_CASR;                          /**< Status register - Load Register command, offset: 0x840 */
  __O  uint32_t LDR_CAA;                           /**< Accumulator register - Load Register command, offset: 0x844 */
  __O  uint32_t LDR_CA[9];                         /**< General Purpose Register 0 - Load Register command..General Purpose Register 8 - Load Register command, array offset: 0x848, array step: 0x4 */
       uint8_t RESERVED_1[20];
  __I  uint32_t STR_CASR;                          /**< Status register - Store Register command, offset: 0x880 */
  __I  uint32_t STR_CAA;                           /**< Accumulator register - Store Register command, offset: 0x884 */
  __I  uint32_t STR_CA[9];                         /**< General Purpose Register 0 - Store Register command..General Purpose Register 8 - Store Register command, array offset: 0x888, array step: 0x4 */
       uint8_t RESERVED_2[20];
  __O  uint32_t ADR_CASR;                          /**< Status register - Add Register command, offset: 0x8C0 */
  __O  uint32_t ADR_CAA;                           /**< Accumulator register - Add to register command, offset: 0x8C4 */
  __O  uint32_t ADR_CA[9];                         /**< General Purpose Register 0 - Add to register command..General Purpose Register 8 - Add to register command, array offset: 0x8C8, array step: 0x4 */
       uint8_t RESERVED_3[20];
  __O  uint32_t RADR_CASR;                         /**< Status register - Reverse and Add to Register command, offset: 0x900 */
  __O  uint32_t RADR_CAA;                          /**< Accumulator register - Reverse and Add to Register command, offset: 0x904 */
  __O  uint32_t RADR_CA[9];                        /**< General Purpose Register 0 - Reverse and Add to Register command..General Purpose Register 8 - Reverse and Add to Register command, array offset: 0x908, array step: 0x4 */
       uint8_t RESERVED_4[84];
  __O  uint32_t XOR_CASR;                          /**< Status register - Exclusive Or command, offset: 0x980 */
  __O  uint32_t XOR_CAA;                           /**< Accumulator register - Exclusive Or command, offset: 0x984 */
  __O  uint32_t XOR_CA[9];                         /**< General Purpose Register 0 - Exclusive Or command..General Purpose Register 8 - Exclusive Or command, array offset: 0x988, array step: 0x4 */
       uint8_t RESERVED_5[20];
  __O  uint32_t ROTL_CASR;                         /**< Status register - Rotate Left command, offset: 0x9C0 */
  __O  uint32_t ROTL_CAA;                          /**< Accumulator register - Rotate Left command, offset: 0x9C4 */
  __O  uint32_t ROTL_CA[9];                        /**< General Purpose Register 0 - Rotate Left command..General Purpose Register 8 - Rotate Left command, array offset: 0x9C8, array step: 0x4 */
       uint8_t RESERVED_6[276];
  __O  uint32_t AESC_CASR;                         /**< Status register - AES Column Operation command, offset: 0xB00 */
  __O  uint32_t AESC_CAA;                          /**< Accumulator register - AES Column Operation command, offset: 0xB04 */
  __O  uint32_t AESC_CA[9];                        /**< General Purpose Register 0 - AES Column Operation command..General Purpose Register 8 - AES Column Operation command, array offset: 0xB08, array step: 0x4 */
       uint8_t RESERVED_7[20];
  __O  uint32_t AESIC_CASR;                        /**< Status register - AES Inverse Column Operation command, offset: 0xB40 */
  __O  uint32_t AESIC_CAA;                         /**< Accumulator register - AES Inverse Column Operation command, offset: 0xB44 */
  __O  uint32_t AESIC_CA[9];                       /**< General Purpose Register 0 - AES Inverse Column Operation command..General Purpose Register 8 - AES Inverse Column Operation command, array offset: 0xB48, array step: 0x4 */
} CAU_TypeDef;

/* ----------------------------------------------------------------------------
   -- CAU Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup CAU_Register_Masks CAU Register Masks
 * @{
 */

/* The count of CAU_DIRECT */
#define CAU_DIRECT_COUNT                         (16U)

/*! @name LDR_CASR - Status register - Load Register command */
#define CAU_LDR_CASR_IC                          (0x1U)
#define CAU_LDR_CASR_DPE                         (0x2U)
#define CAU_LDR_CASR_VER_MASK                    (0xF0000000U)
#define CAU_LDR_CASR_VER_SHIFT                   (28U)
#define CAU_LDR_CASR_VER(x)                      (((uint32_t)(((uint32_t)(x)) << CAU_LDR_CASR_VER_SHIFT)) & CAU_LDR_CASR_VER_MASK)

/* The count of CAU_LDR_CA */
#define CAU_LDR_CA_COUNT                         (9U)

/*! @name STR_CASR - Status register - Store Register command */
#define CAU_STR_CASR_IC                          (0x1U)
#define CAU_STR_CASR_DPE                         (0x2U)
#define CAU_STR_CASR_VER_MASK                    (0xF0000000U)
#define CAU_STR_CASR_VER_SHIFT                   (28U)
#define CAU_STR_CASR_VER(x)                      (((uint32_t)(((uint32_t)(x)) << CAU_STR_CASR_VER_SHIFT)) & CAU_STR_CASR_VER_MASK)

/* The count of CAU_STR_CA */
#define CAU_STR_CA_COUNT                         (9U)

/*! @name ADR_CASR - Status register - Add Register command */
#define CAU_ADR_CASR_IC                          (0x1U)
#define CAU_ADR_CASR_DPE                         (0x2U)
#define CAU_ADR_CASR_VER_MASK                    (0xF0000000U)
#define CAU_ADR_CASR_VER_SHIFT                   (28U)
#define CAU_ADR_CASR_VER(x)                      (((uint32_t)(((uint32_t)(x)) << CAU_ADR_CASR_VER_SHIFT)) & CAU_ADR_CASR_VER_MASK)

/* The count of CAU_ADR_CA */
#define CAU_ADR_CA_COUNT                         (9U)

/*! @name RADR_CASR - Status register - Reverse and Add to Register command */
#define CAU_RADR_CASR_IC                         (0x1U)
#define CAU_RADR_CASR_DPE                        (0x2U)
#define CAU_RADR_CASR_VER_MASK                   (0xF0000000U)
#define CAU_RADR_CASR_VER_SHIFT                  (28U)
#define CAU_RADR_CASR_VER(x)                     (((uint32_t)(((uint32_t)(x)) << CAU_RADR_CASR_VER_SHIFT)) & CAU_RADR_CASR_VER_MASK)

/* The count of CAU_RADR_CA */
#define CAU_RADR_CA_COUNT                        (9U)

/*! @name XOR_CASR - Status register - Exclusive Or command */
#define CAU_XOR_CASR_IC                          (0x1U)
#define CAU_XOR_CASR_DPE                         (0x2U)
#define CAU_XOR_CASR_VER_MASK                    (0xF0000000U)
#define CAU_XOR_CASR_VER_SHIFT                   (28U)
#define CAU_XOR_CASR_VER(x)                      (((uint32_t)(((uint32_t)(x)) << CAU_XOR_CASR_VER_SHIFT)) & CAU_XOR_CASR_VER_MASK)

/* The count of CAU_XOR_CA */
#define CAU_XOR_CA_COUNT                         (9U)

/*! @name ROTL_CASR - Status register - Rotate Left command */
#define CAU_ROTL_CASR_IC                         (0x1U)
#define CAU_ROTL_CASR_DPE                        (0x2U)
#define CAU_ROTL_CASR_VER_MASK                   (0xF0000000U)
#define CAU_ROTL_CASR_VER_SHIFT                  (28U)
#define CAU_ROTL_CASR_VER(x)                     (((uint32_t)(((uint32_t)(x)) << CAU_ROTL_CASR_VER_SHIFT)) & CAU_ROTL_CASR_VER_MASK)

/* The count of CAU_ROTL_CA */
#define CAU_ROTL_CA_COUNT                        (9U)

/*! @name AESC_CASR - Status register - AES Column Operation command */
#define CAU_AESC_CASR_IC                         (0x1U)
#define CAU_AESC_CASR_DPE                        (0x2U)
#define CAU_AESC_CASR_VER_MASK                   (0xF0000000U)
#define CAU_AESC_CASR_VER_SHIFT                  (28U)
#define CAU_AESC_CASR_VER(x)                     (((uint32_t)(((uint32_t)(x)) << CAU_AESC_CASR_VER_SHIFT)) & CAU_AESC_CASR_VER_MASK)

/* The count of CAU_AESC_CA */
#define CAU_AESC_CA_COUNT                        (9U)

/*! @name AESIC_CASR - Status register - AES Inverse Column Operation command */
#define CAU_AESIC_CASR_IC                        (0x1U)
#define CAU_AESIC_CASR_DPE                       (0x2U)
#define CAU_AESIC_CASR_VER_MASK                  (0xF0000000U)
#define CAU_AESIC_CASR_VER_SHIFT                 (28U)
#define CAU_AESIC_CASR_VER(x)                    (((uint32_t)(((uint32_t)(x)) << CAU_AESIC_CASR_VER_SHIFT)) & CAU_AESIC_CASR_VER_MASK)

/* The count of CAU_AESIC_CA */
#define CAU_AESIC_CA_COUNT                       (9U)


/*!
 * @}
 */ /* end of group CAU_Register_Masks */


/* CAU - Peripheral instance base addresses */
/** Peripheral CAU base address */
#define CAU_BASE                                 (0xE0081000u)
/** Peripheral CAU base pointer */
#define CAU                                      ((CAU_TypeDef *)CAU_BASE)
/** Array initializer of CAU peripheral base addresses */
#define CAU_BASE_ADDRS                           { CAU_BASE }
/** Array initializer of CAU peripheral base pointers */
#define CAU_BASE_PTRS                            { CAU }

/*!
 * @}
 */ /* end of group CAU_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- CMP Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup CMP_Peripheral_Access_Layer CMP Peripheral Access Layer
 * @{
 */

/** CMP - Register Layout Typedef */
typedef struct {
  __IO uint8_t CR0;                                /**< CMP Control Register 0, offset: 0x0 */
  __IO uint8_t CR1;                                /**< CMP Control Register 1, offset: 0x1 */
  __IO uint8_t FPR;                                /**< CMP Filter Period Register, offset: 0x2 */
  __IO uint8_t SCR;                                /**< CMP Status and Control Register, offset: 0x3 */
  __IO uint8_t DACCR;                              /**< DAC Control Register, offset: 0x4 */
  __IO uint8_t MUXCR;                              /**< MUX Control Register, offset: 0x5 */
} CMP_TypeDef;

/* ----------------------------------------------------------------------------
   -- CMP Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup CMP_Register_Masks CMP Register Masks
 * @{
 */

/*! @name CR0 - CMP Control Register 0 */
#define CMP_CR0_HYSTCTR_MASK                     (0x3U)
#define CMP_CR0_HYSTCTR_SHIFT                    (0U)
#define CMP_CR0_HYSTCTR(x)                       (((uint8_t)(((uint8_t)(x)) << CMP_CR0_HYSTCTR_SHIFT)) & CMP_CR0_HYSTCTR_MASK)
#define CMP_CR0_FILTER_CNT_MASK                  (0x70U)
#define CMP_CR0_FILTER_CNT_SHIFT                 (4U)
#define CMP_CR0_FILTER_CNT(x)                    (((uint8_t)(((uint8_t)(x)) << CMP_CR0_FILTER_CNT_SHIFT)) & CMP_CR0_FILTER_CNT_MASK)

/*! @name CR1 - CMP Control Register 1 */
#define CMP_CR1_EN                               (0x1U)
#define CMP_CR1_OPE                              (0x2U)
#define CMP_CR1_COS                              (0x4U)
#define CMP_CR1_INV                              (0x8U)
#define CMP_CR1_PMODE                            (0x10U)
#define CMP_CR1_WE                               (0x40U)
#define CMP_CR1_SE                               (0x80U)

/*! @name FPR - CMP Filter Period Register */
#define CMP_FPR_FILT_PER_MASK                    (0xFFU)
#define CMP_FPR_FILT_PER_SHIFT                   (0U)
#define CMP_FPR_FILT_PER(x)                      (((uint8_t)(((uint8_t)(x)) << CMP_FPR_FILT_PER_SHIFT)) & CMP_FPR_FILT_PER_MASK)

/*! @name SCR - CMP Status and Control Register */
#define CMP_SCR_COUT                             (0x1U)
#define CMP_SCR_CFF                              (0x2U)
#define CMP_SCR_CFR                              (0x4U)
#define CMP_SCR_IEF                              (0x8U)
#define CMP_SCR_IER                              (0x10U)
#define CMP_SCR_DMAEN                            (0x40U)

/*! @name DACCR - DAC Control Register */
#define CMP_DACCR_VOSEL_MASK                     (0x3FU)
#define CMP_DACCR_VOSEL_SHIFT                    (0U)
#define CMP_DACCR_VOSEL(x)                       (((uint8_t)(((uint8_t)(x)) << CMP_DACCR_VOSEL_SHIFT)) & CMP_DACCR_VOSEL_MASK)
#define CMP_DACCR_VRSEL                          (0x40U)
#define CMP_DACCR_DACEN                          (0x80U)

/*! @name MUXCR - MUX Control Register */
#define CMP_MUXCR_MSEL_MASK                      (0x7U)
#define CMP_MUXCR_MSEL_SHIFT                     (0U)
#define CMP_MUXCR_MSEL(x)                        (((uint8_t)(((uint8_t)(x)) << CMP_MUXCR_MSEL_SHIFT)) & CMP_MUXCR_MSEL_MASK)
#define CMP_MUXCR_PSEL_MASK                      (0x38U)
#define CMP_MUXCR_PSEL_SHIFT                     (3U)
#define CMP_MUXCR_PSEL(x)                        (((uint8_t)(((uint8_t)(x)) << CMP_MUXCR_PSEL_SHIFT)) & CMP_MUXCR_PSEL_MASK)
#define CMP_MUXCR_PSTM                           (0x80U)


/*!
 * @}
 */ /* end of group CMP_Register_Masks */


/* CMP - Peripheral instance base addresses */
/** Peripheral CMP0 base address */
#define CMP0_BASE                                (0x40073000u)
/** Peripheral CMP0 base pointer */
#define CMP0                                     ((CMP_TypeDef *)CMP0_BASE)
/** Peripheral CMP1 base address */
#define CMP1_BASE                                (0x40073008u)
/** Peripheral CMP1 base pointer */
#define CMP1                                     ((CMP_TypeDef *)CMP1_BASE)
/** Peripheral CMP2 base address */
#define CMP2_BASE                                (0x40073010u)
/** Peripheral CMP2 base pointer */
#define CMP2                                     ((CMP_TypeDef *)CMP2_BASE)
/** Array initializer of CMP peripheral base addresses */
#define CMP_BASE_ADDRS                           { CMP0_BASE, CMP1_BASE, CMP2_BASE }
/** Array initializer of CMP peripheral base pointers */
#define CMP_BASE_PTRS                            { CMP0, CMP1, CMP2 }
/** Interrupt vectors for the CMP peripheral type */
#define CMP_IRQS                                 { CMP0_IRQn, CMP1_IRQn, CMP2_IRQn }

/*!
 * @}
 */ /* end of group CMP_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- CMT Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup CMT_Peripheral_Access_Layer CMT Peripheral Access Layer
 * @{
 */

/** CMT - Register Layout Typedef */
typedef struct {
  __IO uint8_t CGH1;                               /**< CMT Carrier Generator High Data Register 1, offset: 0x0 */
  __IO uint8_t CGL1;                               /**< CMT Carrier Generator Low Data Register 1, offset: 0x1 */
  __IO uint8_t CGH2;                               /**< CMT Carrier Generator High Data Register 2, offset: 0x2 */
  __IO uint8_t CGL2;                               /**< CMT Carrier Generator Low Data Register 2, offset: 0x3 */
  __IO uint8_t OC;                                 /**< CMT Output Control Register, offset: 0x4 */
  __IO uint8_t MSC;                                /**< CMT Modulator Status and Control Register, offset: 0x5 */
  __IO uint8_t CMD1;                               /**< CMT Modulator Data Register Mark High, offset: 0x6 */
  __IO uint8_t CMD2;                               /**< CMT Modulator Data Register Mark Low, offset: 0x7 */
  __IO uint8_t CMD3;                               /**< CMT Modulator Data Register Space High, offset: 0x8 */
  __IO uint8_t CMD4;                               /**< CMT Modulator Data Register Space Low, offset: 0x9 */
  __IO uint8_t PPS;                                /**< CMT Primary Prescaler Register, offset: 0xA */
  __IO uint8_t DMA;                                /**< CMT Direct Memory Access Register, offset: 0xB */
} CMT_TypeDef;

/* ----------------------------------------------------------------------------
   -- CMT Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup CMT_Register_Masks CMT Register Masks
 * @{
 */

/*! @name CGH1 - CMT Carrier Generator High Data Register 1 */
#define CMT_CGH1_PH_MASK                         (0xFFU)
#define CMT_CGH1_PH_SHIFT                        (0U)
#define CMT_CGH1_PH(x)                           (((uint8_t)(((uint8_t)(x)) << CMT_CGH1_PH_SHIFT)) & CMT_CGH1_PH_MASK)

/*! @name CGL1 - CMT Carrier Generator Low Data Register 1 */
#define CMT_CGL1_PL_MASK                         (0xFFU)
#define CMT_CGL1_PL_SHIFT                        (0U)
#define CMT_CGL1_PL(x)                           (((uint8_t)(((uint8_t)(x)) << CMT_CGL1_PL_SHIFT)) & CMT_CGL1_PL_MASK)

/*! @name CGH2 - CMT Carrier Generator High Data Register 2 */
#define CMT_CGH2_SH_MASK                         (0xFFU)
#define CMT_CGH2_SH_SHIFT                        (0U)
#define CMT_CGH2_SH(x)                           (((uint8_t)(((uint8_t)(x)) << CMT_CGH2_SH_SHIFT)) & CMT_CGH2_SH_MASK)

/*! @name CGL2 - CMT Carrier Generator Low Data Register 2 */
#define CMT_CGL2_SL_MASK                         (0xFFU)
#define CMT_CGL2_SL_SHIFT                        (0U)
#define CMT_CGL2_SL(x)                           (((uint8_t)(((uint8_t)(x)) << CMT_CGL2_SL_SHIFT)) & CMT_CGL2_SL_MASK)

/*! @name OC - CMT Output Control Register */
#define CMT_OC_IROPEN                            (0x20U)
#define CMT_OC_CMTPOL                            (0x40U)
#define CMT_OC_IROL                              (0x80U)

/*! @name MSC - CMT Modulator Status and Control Register */
#define CMT_MSC_MCGEN                            (0x1U)
#define CMT_MSC_EOCIE                            (0x2U)
#define CMT_MSC_FSK                              (0x4U)
#define CMT_MSC_BASE                             (0x8U)
#define CMT_MSC_EXSPC                            (0x10U)
#define CMT_MSC_CMTDIV_MASK                      (0x60U)
#define CMT_MSC_CMTDIV_SHIFT                     (5U)
#define CMT_MSC_CMTDIV(x)                        (((uint8_t)(((uint8_t)(x)) << CMT_MSC_CMTDIV_SHIFT)) & CMT_MSC_CMTDIV_MASK)
#define CMT_MSC_EOCF                             (0x80U)

/*! @name CMD1 - CMT Modulator Data Register Mark High */
#define CMT_CMD1_MB_MASK                         (0xFFU)
#define CMT_CMD1_MB_SHIFT                        (0U)
#define CMT_CMD1_MB(x)                           (((uint8_t)(((uint8_t)(x)) << CMT_CMD1_MB_SHIFT)) & CMT_CMD1_MB_MASK)

/*! @name CMD2 - CMT Modulator Data Register Mark Low */
#define CMT_CMD2_MB_MASK                         (0xFFU)
#define CMT_CMD2_MB_SHIFT                        (0U)
#define CMT_CMD2_MB(x)                           (((uint8_t)(((uint8_t)(x)) << CMT_CMD2_MB_SHIFT)) & CMT_CMD2_MB_MASK)

/*! @name CMD3 - CMT Modulator Data Register Space High */
#define CMT_CMD3_SB_MASK                         (0xFFU)
#define CMT_CMD3_SB_SHIFT                        (0U)
#define CMT_CMD3_SB(x)                           (((uint8_t)(((uint8_t)(x)) << CMT_CMD3_SB_SHIFT)) & CMT_CMD3_SB_MASK)

/*! @name CMD4 - CMT Modulator Data Register Space Low */
#define CMT_CMD4_SB_MASK                         (0xFFU)
#define CMT_CMD4_SB_SHIFT                        (0U)
#define CMT_CMD4_SB(x)                           (((uint8_t)(((uint8_t)(x)) << CMT_CMD4_SB_SHIFT)) & CMT_CMD4_SB_MASK)

/*! @name PPS - CMT Primary Prescaler Register */
#define CMT_PPS_PPSDIV_MASK                      (0xFU)
#define CMT_PPS_PPSDIV_SHIFT                     (0U)
#define CMT_PPS_PPSDIV(x)                        (((uint8_t)(((uint8_t)(x)) << CMT_PPS_PPSDIV_SHIFT)) & CMT_PPS_PPSDIV_MASK)

/*! @name DMA - CMT Direct Memory Access Register */
#define CMT_DMA_DMA                              (0x1U)


/*!
 * @}
 */ /* end of group CMT_Register_Masks */


/* CMT - Peripheral instance base addresses */
/** Peripheral CMT base address */
#define CMT_BASE                                 (0x40062000u)
/** Peripheral CMT base pointer */
#define CMT                                      ((CMT_TypeDef *)CMT_BASE)
/** Array initializer of CMT peripheral base addresses */
#define CMT_BASE_ADDRS                           { CMT_BASE }
/** Array initializer of CMT peripheral base pointers */
#define CMT_BASE_PTRS                            { CMT }
/** Interrupt vectors for the CMT peripheral type */
#define CMT_IRQS                                 { CMT_IRQn }

/*!
 * @}
 */ /* end of group CMT_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- CRC Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup CRC_Peripheral_Access_Layer CRC Peripheral Access Layer
 * @{
 */

/** CRC - Register Layout Typedef */
typedef struct {
  union {                                          /* offset: 0x0 */
    struct {                                         /* offset: 0x0 */
      __IO uint16_t DATAL;                             /**< CRC_DATAL register., offset: 0x0 */
      __IO uint16_t DATAH;                             /**< CRC_DATAH register., offset: 0x2 */
    } ACCESS16BIT;
    __IO uint32_t DATA;                              /**< CRC Data register, offset: 0x0 */
    struct {                                         /* offset: 0x0 */
      __IO uint8_t DATALL;                             /**< CRC_DATALL register., offset: 0x0 */
      __IO uint8_t DATALU;                             /**< CRC_DATALU register., offset: 0x1 */
      __IO uint8_t DATAHL;                             /**< CRC_DATAHL register., offset: 0x2 */
      __IO uint8_t DATAHU;                             /**< CRC_DATAHU register., offset: 0x3 */
    } ACCESS8BIT;
  };
  union {                                          /* offset: 0x4 */
    struct {                                         /* offset: 0x4 */
      __IO uint16_t GPOLYL;                            /**< CRC_GPOLYL register., offset: 0x4 */
      __IO uint16_t GPOLYH;                            /**< CRC_GPOLYH register., offset: 0x6 */
    } GPOLY_ACCESS16BIT;
    __IO uint32_t GPOLY;                             /**< CRC Polynomial register, offset: 0x4 */
    struct {                                         /* offset: 0x4 */
      __IO uint8_t GPOLYLL;                            /**< CRC_GPOLYLL register., offset: 0x4 */
      __IO uint8_t GPOLYLU;                            /**< CRC_GPOLYLU register., offset: 0x5 */
      __IO uint8_t GPOLYHL;                            /**< CRC_GPOLYHL register., offset: 0x6 */
      __IO uint8_t GPOLYHU;                            /**< CRC_GPOLYHU register., offset: 0x7 */
    } GPOLY_ACCESS8BIT;
  };
  union {                                          /* offset: 0x8 */
    __IO uint32_t CTRL;                              /**< CRC Control register, offset: 0x8 */
    struct {                                         /* offset: 0x8 */
           uint8_t RESERVED_0[3];
      __IO uint8_t CTRLHU;                             /**< CRC_CTRLHU register., offset: 0xB */
    } CTRL_ACCESS8BIT;
  };
} CRC_TypeDef;

/* ----------------------------------------------------------------------------
   -- CRC Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup CRC_Register_Masks CRC Register Masks
 * @{
 */

/*! @name DATAL - CRC_DATAL register. */
#define CRC_DATAL_DATAL_MASK                     (0xFFFFU)
#define CRC_DATAL_DATAL_SHIFT                    (0U)
#define CRC_DATAL_DATAL(x)                       (((uint16_t)(((uint16_t)(x)) << CRC_DATAL_DATAL_SHIFT)) & CRC_DATAL_DATAL_MASK)

/*! @name DATAH - CRC_DATAH register. */
#define CRC_DATAH_DATAH_MASK                     (0xFFFFU)
#define CRC_DATAH_DATAH_SHIFT                    (0U)
#define CRC_DATAH_DATAH(x)                       (((uint16_t)(((uint16_t)(x)) << CRC_DATAH_DATAH_SHIFT)) & CRC_DATAH_DATAH_MASK)

/*! @name DATA - CRC Data register */
#define CRC_DATA_LL_MASK                         (0xFFU)
#define CRC_DATA_LL_SHIFT                        (0U)
#define CRC_DATA_LL(x)                           (((uint32_t)(((uint32_t)(x)) << CRC_DATA_LL_SHIFT)) & CRC_DATA_LL_MASK)
#define CRC_DATA_LU_MASK                         (0xFF00U)
#define CRC_DATA_LU_SHIFT                        (8U)
#define CRC_DATA_LU(x)                           (((uint32_t)(((uint32_t)(x)) << CRC_DATA_LU_SHIFT)) & CRC_DATA_LU_MASK)
#define CRC_DATA_HL_MASK                         (0xFF0000U)
#define CRC_DATA_HL_SHIFT                        (16U)
#define CRC_DATA_HL(x)                           (((uint32_t)(((uint32_t)(x)) << CRC_DATA_HL_SHIFT)) & CRC_DATA_HL_MASK)
#define CRC_DATA_HU_MASK                         (0xFF000000U)
#define CRC_DATA_HU_SHIFT                        (24U)
#define CRC_DATA_HU(x)                           (((uint32_t)(((uint32_t)(x)) << CRC_DATA_HU_SHIFT)) & CRC_DATA_HU_MASK)

/*! @name DATALL - CRC_DATALL register. */
#define CRC_DATALL_DATALL_MASK                   (0xFFU)
#define CRC_DATALL_DATALL_SHIFT                  (0U)
#define CRC_DATALL_DATALL(x)                     (((uint8_t)(((uint8_t)(x)) << CRC_DATALL_DATALL_SHIFT)) & CRC_DATALL_DATALL_MASK)

/*! @name DATALU - CRC_DATALU register. */
#define CRC_DATALU_DATALU_MASK                   (0xFFU)
#define CRC_DATALU_DATALU_SHIFT                  (0U)
#define CRC_DATALU_DATALU(x)                     (((uint8_t)(((uint8_t)(x)) << CRC_DATALU_DATALU_SHIFT)) & CRC_DATALU_DATALU_MASK)

/*! @name DATAHL - CRC_DATAHL register. */
#define CRC_DATAHL_DATAHL_MASK                   (0xFFU)
#define CRC_DATAHL_DATAHL_SHIFT                  (0U)
#define CRC_DATAHL_DATAHL(x)                     (((uint8_t)(((uint8_t)(x)) << CRC_DATAHL_DATAHL_SHIFT)) & CRC_DATAHL_DATAHL_MASK)

/*! @name DATAHU - CRC_DATAHU register. */
#define CRC_DATAHU_DATAHU_MASK                   (0xFFU)
#define CRC_DATAHU_DATAHU_SHIFT                  (0U)
#define CRC_DATAHU_DATAHU(x)                     (((uint8_t)(((uint8_t)(x)) << CRC_DATAHU_DATAHU_SHIFT)) & CRC_DATAHU_DATAHU_MASK)

/*! @name GPOLYL - CRC_GPOLYL register. */
#define CRC_GPOLYL_GPOLYL_MASK                   (0xFFFFU)
#define CRC_GPOLYL_GPOLYL_SHIFT                  (0U)
#define CRC_GPOLYL_GPOLYL(x)                     (((uint16_t)(((uint16_t)(x)) << CRC_GPOLYL_GPOLYL_SHIFT)) & CRC_GPOLYL_GPOLYL_MASK)

/*! @name GPOLYH - CRC_GPOLYH register. */
#define CRC_GPOLYH_GPOLYH_MASK                   (0xFFFFU)
#define CRC_GPOLYH_GPOLYH_SHIFT                  (0U)
#define CRC_GPOLYH_GPOLYH(x)                     (((uint16_t)(((uint16_t)(x)) << CRC_GPOLYH_GPOLYH_SHIFT)) & CRC_GPOLYH_GPOLYH_MASK)

/*! @name GPOLY - CRC Polynomial register */
#define CRC_GPOLY_LOW_MASK                       (0xFFFFU)
#define CRC_GPOLY_LOW_SHIFT                      (0U)
#define CRC_GPOLY_LOW(x)                         (((uint32_t)(((uint32_t)(x)) << CRC_GPOLY_LOW_SHIFT)) & CRC_GPOLY_LOW_MASK)
#define CRC_GPOLY_HIGH_MASK                      (0xFFFF0000U)
#define CRC_GPOLY_HIGH_SHIFT                     (16U)
#define CRC_GPOLY_HIGH(x)                        (((uint32_t)(((uint32_t)(x)) << CRC_GPOLY_HIGH_SHIFT)) & CRC_GPOLY_HIGH_MASK)

/*! @name GPOLYLL - CRC_GPOLYLL register. */
#define CRC_GPOLYLL_GPOLYLL_MASK                 (0xFFU)
#define CRC_GPOLYLL_GPOLYLL_SHIFT                (0U)
#define CRC_GPOLYLL_GPOLYLL(x)                   (((uint8_t)(((uint8_t)(x)) << CRC_GPOLYLL_GPOLYLL_SHIFT)) & CRC_GPOLYLL_GPOLYLL_MASK)

/*! @name GPOLYLU - CRC_GPOLYLU register. */
#define CRC_GPOLYLU_GPOLYLU_MASK                 (0xFFU)
#define CRC_GPOLYLU_GPOLYLU_SHIFT                (0U)
#define CRC_GPOLYLU_GPOLYLU(x)                   (((uint8_t)(((uint8_t)(x)) << CRC_GPOLYLU_GPOLYLU_SHIFT)) & CRC_GPOLYLU_GPOLYLU_MASK)

/*! @name GPOLYHL - CRC_GPOLYHL register. */
#define CRC_GPOLYHL_GPOLYHL_MASK                 (0xFFU)
#define CRC_GPOLYHL_GPOLYHL_SHIFT                (0U)
#define CRC_GPOLYHL_GPOLYHL(x)                   (((uint8_t)(((uint8_t)(x)) << CRC_GPOLYHL_GPOLYHL_SHIFT)) & CRC_GPOLYHL_GPOLYHL_MASK)

/*! @name GPOLYHU - CRC_GPOLYHU register. */
#define CRC_GPOLYHU_GPOLYHU_MASK                 (0xFFU)
#define CRC_GPOLYHU_GPOLYHU_SHIFT                (0U)
#define CRC_GPOLYHU_GPOLYHU(x)                   (((uint8_t)(((uint8_t)(x)) << CRC_GPOLYHU_GPOLYHU_SHIFT)) & CRC_GPOLYHU_GPOLYHU_MASK)

/*! @name CTRL - CRC Control register */
#define CRC_CTRL_TCRC                            (0x1000000U)
#define CRC_CTRL_WAS                             (0x2000000U)
#define CRC_CTRL_FXOR                            (0x4000000U)
#define CRC_CTRL_TOTR_MASK                       (0x30000000U)
#define CRC_CTRL_TOTR_SHIFT                      (28U)
#define CRC_CTRL_TOTR(x)                         (((uint32_t)(((uint32_t)(x)) << CRC_CTRL_TOTR_SHIFT)) & CRC_CTRL_TOTR_MASK)
#define CRC_CTRL_TOT_MASK                        (0xC0000000U)
#define CRC_CTRL_TOT_SHIFT                       (30U)
#define CRC_CTRL_TOT(x)                          (((uint32_t)(((uint32_t)(x)) << CRC_CTRL_TOT_SHIFT)) & CRC_CTRL_TOT_MASK)

/*! @name CTRLHU - CRC_CTRLHU register. */
#define CRC_CTRLHU_TCRC                          (0x1U)
#define CRC_CTRLHU_WAS                           (0x2U)
#define CRC_CTRLHU_FXOR                          (0x4U)
#define CRC_CTRLHU_TOTR_MASK                     (0x30U)
#define CRC_CTRLHU_TOTR_SHIFT                    (4U)
#define CRC_CTRLHU_TOTR(x)                       (((uint8_t)(((uint8_t)(x)) << CRC_CTRLHU_TOTR_SHIFT)) & CRC_CTRLHU_TOTR_MASK)
#define CRC_CTRLHU_TOT_MASK                      (0xC0U)
#define CRC_CTRLHU_TOT_SHIFT                     (6U)
#define CRC_CTRLHU_TOT(x)                        (((uint8_t)(((uint8_t)(x)) << CRC_CTRLHU_TOT_SHIFT)) & CRC_CTRLHU_TOT_MASK)


/*!
 * @}
 */ /* end of group CRC_Register_Masks */


/* CRC - Peripheral instance base addresses */
/** Peripheral CRC base address */
#define CRC_BASE                                 (0x40032000u)
/** Peripheral CRC base pointer */
#define CRC0                                     ((CRC_TypeDef *)CRC_BASE)
/** Array initializer of CRC peripheral base addresses */
#define CRC_BASE_ADDRS                           { CRC_BASE }
/** Array initializer of CRC peripheral base pointers */
#define CRC_BASE_PTRS                            { CRC0 }

/*!
 * @}
 */ /* end of group CRC_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- DAC Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup DAC_Peripheral_Access_Layer DAC Peripheral Access Layer
 * @{
 */

/** DAC - Register Layout Typedef */
typedef struct {
  struct {                                         /* offset: 0x0, array step: 0x2 */
    __IO uint8_t DATL;                               /**< DAC Data Low Register, array offset: 0x0, array step: 0x2 */
    __IO uint8_t DATH;                               /**< DAC Data High Register, array offset: 0x1, array step: 0x2 */
  } DAT[16];
  __IO uint8_t SR;                                 /**< DAC Status Register, offset: 0x20 */
  __IO uint8_t C0;                                 /**< DAC Control Register, offset: 0x21 */
  __IO uint8_t C1;                                 /**< DAC Control Register 1, offset: 0x22 */
  __IO uint8_t C2;                                 /**< DAC Control Register 2, offset: 0x23 */
} DAC_TypeDef;

/* ----------------------------------------------------------------------------
   -- DAC Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup DAC_Register_Masks DAC Register Masks
 * @{
 */

/*! @name DATL - DAC Data Low Register */
#define DAC_DATL_DATA0_MASK                      (0xFFU)
#define DAC_DATL_DATA0_SHIFT                     (0U)
#define DAC_DATL_DATA0(x)                        (((uint8_t)(((uint8_t)(x)) << DAC_DATL_DATA0_SHIFT)) & DAC_DATL_DATA0_MASK)

/* The count of DAC_DATL */
#define DAC_DATL_COUNT                           (16U)

/*! @name DATH - DAC Data High Register */
#define DAC_DATH_DATA1_MASK                      (0xFU)
#define DAC_DATH_DATA1_SHIFT                     (0U)
#define DAC_DATH_DATA1(x)                        (((uint8_t)(((uint8_t)(x)) << DAC_DATH_DATA1_SHIFT)) & DAC_DATH_DATA1_MASK)

/* The count of DAC_DATH */
#define DAC_DATH_COUNT                           (16U)

/*! @name SR - DAC Status Register */
#define DAC_SR_DACBFRPBF                         (0x1U)
#define DAC_SR_DACBFRPTF                         (0x2U)
#define DAC_SR_DACBFWMF                          (0x4U)

/*! @name C0 - DAC Control Register */
#define DAC_C0_DACBBIEN                          (0x1U)
#define DAC_C0_DACBTIEN                          (0x2U)
#define DAC_C0_DACBWIEN                          (0x4U)
#define DAC_C0_LPEN                              (0x8U)
#define DAC_C0_DACSWTRG                          (0x10U)
#define DAC_C0_DACTRGSEL                         (0x20U)
#define DAC_C0_DACRFS                            (0x40U)
#define DAC_C0_DACEN                             (0x80U)

/*! @name C1 - DAC Control Register 1 */
#define DAC_C1_DACBFEN                           (0x1U)
#define DAC_C1_DACBFMD_MASK                      (0x6U)
#define DAC_C1_DACBFMD_SHIFT                     (1U)
#define DAC_C1_DACBFMD(x)                        (((uint8_t)(((uint8_t)(x)) << DAC_C1_DACBFMD_SHIFT)) & DAC_C1_DACBFMD_MASK)
#define DAC_C1_DACBFWM_MASK                      (0x18U)
#define DAC_C1_DACBFWM_SHIFT                     (3U)
#define DAC_C1_DACBFWM(x)                        (((uint8_t)(((uint8_t)(x)) << DAC_C1_DACBFWM_SHIFT)) & DAC_C1_DACBFWM_MASK)
#define DAC_C1_DMAEN                             (0x80U)

/*! @name C2 - DAC Control Register 2 */
#define DAC_C2_DACBFUP_MASK                      (0xFU)
#define DAC_C2_DACBFUP_SHIFT                     (0U)
#define DAC_C2_DACBFUP(x)                        (((uint8_t)(((uint8_t)(x)) << DAC_C2_DACBFUP_SHIFT)) & DAC_C2_DACBFUP_MASK)
#define DAC_C2_DACBFRP_MASK                      (0xF0U)
#define DAC_C2_DACBFRP_SHIFT                     (4U)
#define DAC_C2_DACBFRP(x)                        (((uint8_t)(((uint8_t)(x)) << DAC_C2_DACBFRP_SHIFT)) & DAC_C2_DACBFRP_MASK)


/*!
 * @}
 */ /* end of group DAC_Register_Masks */


/* DAC - Peripheral instance base addresses */
/** Peripheral DAC0 base address */
#define DAC0_BASE                                (0x400CC000u)
/** Peripheral DAC0 base pointer */
#define DAC0                                     ((DAC_TypeDef *)DAC0_BASE)
/** Peripheral DAC1 base address */
#define DAC1_BASE                                (0x400CD000u)
/** Peripheral DAC1 base pointer */
#define DAC1                                     ((DAC_TypeDef *)DAC1_BASE)
/** Array initializer of DAC peripheral base addresses */
#define DAC_BASE_ADDRS                           { DAC0_BASE, DAC1_BASE }
/** Array initializer of DAC peripheral base pointers */
#define DAC_BASE_PTRS                            { DAC0, DAC1 }
/** Interrupt vectors for the DAC peripheral type */
#define DAC_IRQS                                 { DAC0_IRQn, DAC1_IRQn }

/*!
 * @}
 */ /* end of group DAC_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- DMA Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup DMA_Peripheral_Access_Layer DMA Peripheral Access Layer
 * @{
 */

/** DMA - Register Layout Typedef */
typedef struct {
  __IO uint32_t CR;                                /**< Control Register, offset: 0x0 */
  __I  uint32_t ES;                                /**< Error Status Register, offset: 0x4 */
       uint8_t RESERVED_0[4];
  __IO uint32_t ERQ;                               /**< Enable Request Register, offset: 0xC */
       uint8_t RESERVED_1[4];
  __IO uint32_t EEI;                               /**< Enable Error Interrupt Register, offset: 0x14 */
  __O  uint8_t CEEI;                               /**< Clear Enable Error Interrupt Register, offset: 0x18 */
  __O  uint8_t SEEI;                               /**< Set Enable Error Interrupt Register, offset: 0x19 */
  __O  uint8_t CERQ;                               /**< Clear Enable Request Register, offset: 0x1A */
  __O  uint8_t SERQ;                               /**< Set Enable Request Register, offset: 0x1B */
  __O  uint8_t CDNE;                               /**< Clear DONE Status Bit Register, offset: 0x1C */
  __O  uint8_t SSRT;                               /**< Set START Bit Register, offset: 0x1D */
  __O  uint8_t CERR;                               /**< Clear Error Register, offset: 0x1E */
  __O  uint8_t CINT;                               /**< Clear Interrupt Request Register, offset: 0x1F */
       uint8_t RESERVED_2[4];
  __IO uint32_t INT;                               /**< Interrupt Request Register, offset: 0x24 */
       uint8_t RESERVED_3[4];
  __IO uint32_t ERR;                               /**< Error Register, offset: 0x2C */
       uint8_t RESERVED_4[4];
  __I  uint32_t HRS;                               /**< Hardware Request Status Register, offset: 0x34 */
       uint8_t RESERVED_5[200];
  __IO uint8_t DCHPRI3;                            /**< Channel n Priority Register, offset: 0x100 */
  __IO uint8_t DCHPRI2;                            /**< Channel n Priority Register, offset: 0x101 */
  __IO uint8_t DCHPRI1;                            /**< Channel n Priority Register, offset: 0x102 */
  __IO uint8_t DCHPRI0;                            /**< Channel n Priority Register, offset: 0x103 */
  __IO uint8_t DCHPRI7;                            /**< Channel n Priority Register, offset: 0x104 */
  __IO uint8_t DCHPRI6;                            /**< Channel n Priority Register, offset: 0x105 */
  __IO uint8_t DCHPRI5;                            /**< Channel n Priority Register, offset: 0x106 */
  __IO uint8_t DCHPRI4;                            /**< Channel n Priority Register, offset: 0x107 */
  __IO uint8_t DCHPRI11;                           /**< Channel n Priority Register, offset: 0x108 */
  __IO uint8_t DCHPRI10;                           /**< Channel n Priority Register, offset: 0x109 */
  __IO uint8_t DCHPRI9;                            /**< Channel n Priority Register, offset: 0x10A */
  __IO uint8_t DCHPRI8;                            /**< Channel n Priority Register, offset: 0x10B */
  __IO uint8_t DCHPRI15;                           /**< Channel n Priority Register, offset: 0x10C */
  __IO uint8_t DCHPRI14;                           /**< Channel n Priority Register, offset: 0x10D */
  __IO uint8_t DCHPRI13;                           /**< Channel n Priority Register, offset: 0x10E */
  __IO uint8_t DCHPRI12;                           /**< Channel n Priority Register, offset: 0x10F */
       uint8_t RESERVED_6[3824];
  struct {                                         /* offset: 0x1000, array step: 0x20 */
    __IO uint32_t SADDR;                             /**< TCD Source Address, array offset: 0x1000, array step: 0x20 */
    __IO uint16_t SOFF;                              /**< TCD Signed Source Address Offset, array offset: 0x1004, array step: 0x20 */
    __IO uint16_t ATTR;                              /**< TCD Transfer Attributes, array offset: 0x1006, array step: 0x20 */
    union {                                          /* offset: 0x1008, array step: 0x20 */
      __IO uint32_t NBYTES_MLNO;                       /**< TCD Minor Byte Count (Minor Loop Disabled), array offset: 0x1008, array step: 0x20 */
      __IO uint32_t NBYTES_MLOFFNO;                    /**< TCD Signed Minor Loop Offset (Minor Loop Enabled and Offset Disabled), array offset: 0x1008, array step: 0x20 */
      __IO uint32_t NBYTES_MLOFFYES;                   /**< TCD Signed Minor Loop Offset (Minor Loop and Offset Enabled), array offset: 0x1008, array step: 0x20 */
    };
    __IO uint32_t SLAST;                             /**< TCD Last Source Address Adjustment, array offset: 0x100C, array step: 0x20 */
    __IO uint32_t DADDR;                             /**< TCD Destination Address, array offset: 0x1010, array step: 0x20 */
    __IO uint16_t DOFF;                              /**< TCD Signed Destination Address Offset, array offset: 0x1014, array step: 0x20 */
    union {                                          /* offset: 0x1016, array step: 0x20 */
      __IO uint16_t CITER_ELINKNO;                     /**< TCD Current Minor Loop Link, Major Loop Count (Channel Linking Disabled), array offset: 0x1016, array step: 0x20 */
      __IO uint16_t CITER_ELINKYES;                    /**< TCD Current Minor Loop Link, Major Loop Count (Channel Linking Enabled), array offset: 0x1016, array step: 0x20 */
    };
    __IO uint32_t DLAST_SGA;                         /**< TCD Last Destination Address Adjustment/Scatter Gather Address, array offset: 0x1018, array step: 0x20 */
    __IO uint16_t CSR;                               /**< TCD Control and Status, array offset: 0x101C, array step: 0x20 */
    union {                                          /* offset: 0x101E, array step: 0x20 */
      __IO uint16_t BITER_ELINKNO;                     /**< TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Disabled), array offset: 0x101E, array step: 0x20 */
      __IO uint16_t BITER_ELINKYES;                    /**< TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Enabled), array offset: 0x101E, array step: 0x20 */
    };
  } TCD[16];
} DMA_TypeDef;

/* ----------------------------------------------------------------------------
   -- DMA Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup DMA_Register_Masks DMA Register Masks
 * @{
 */

/*! @name CR - Control Register */
#define DMA_CR_EDBG                              (0x2U)
#define DMA_CR_ERCA                              (0x4U)
#define DMA_CR_HOE                               (0x10U)
#define DMA_CR_HALT                              (0x20U)
#define DMA_CR_CLM                               (0x40U)
#define DMA_CR_EMLM                              (0x80U)
#define DMA_CR_ECX                               (0x10000U)
#define DMA_CR_CX                                (0x20000U)

/*! @name ES - Error Status Register */
#define DMA_ES_DBE                               (0x1U)
#define DMA_ES_SBE                               (0x2U)
#define DMA_ES_SGE                               (0x4U)
#define DMA_ES_NCE                               (0x8U)
#define DMA_ES_DOE                               (0x10U)
#define DMA_ES_DAE                               (0x20U)
#define DMA_ES_SOE                               (0x40U)
#define DMA_ES_SAE                               (0x80U)
#define DMA_ES_ERRCHN_MASK                       (0xF00U)
#define DMA_ES_ERRCHN_SHIFT                      (8U)
#define DMA_ES_ERRCHN(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_ES_ERRCHN_SHIFT)) & DMA_ES_ERRCHN_MASK)
#define DMA_ES_CPE                               (0x4000U)
#define DMA_ES_ECX                               (0x10000U)
#define DMA_ES_VLD                               (0x80000000U)

/*! @name CEEI - Clear Enable Error Interrupt Register */
#define DMA_CEEI_CEEI_MASK                       (0xFU)
#define DMA_CEEI_CEEI_SHIFT                      (0U)
#define DMA_CEEI_CEEI(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_CEEI_CEEI_SHIFT)) & DMA_CEEI_CEEI_MASK)
#define DMA_CEEI_CAEE                            (0x40U)
#define DMA_CEEI_NOP                             (0x80U)

/*! @name SEEI - Set Enable Error Interrupt Register */
#define DMA_SEEI_SEEI_MASK                       (0xFU)
#define DMA_SEEI_SEEI_SHIFT                      (0U)
#define DMA_SEEI_SEEI(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_SEEI_SEEI_SHIFT)) & DMA_SEEI_SEEI_MASK)
#define DMA_SEEI_SAEE                            (0x40U)
#define DMA_SEEI_NOP                             (0x80U)

/*! @name CERQ - Clear Enable Request Register */
#define DMA_CERQ_CERQ_MASK                       (0xFU)
#define DMA_CERQ_CERQ_SHIFT                      (0U)
#define DMA_CERQ_CERQ(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_CERQ_CERQ_SHIFT)) & DMA_CERQ_CERQ_MASK)
#define DMA_CERQ_CAER                            (0x40U)
#define DMA_CERQ_NOP                             (0x80U)

/*! @name SERQ - Set Enable Request Register */
#define DMA_SERQ_SERQ_MASK                       (0xFU)
#define DMA_SERQ_SERQ_SHIFT                      (0U)
#define DMA_SERQ_SERQ(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_SERQ_SERQ_SHIFT)) & DMA_SERQ_SERQ_MASK)
#define DMA_SERQ_SAER                            (0x40U)
#define DMA_SERQ_NOP                             (0x80U)

/*! @name CDNE - Clear DONE Status Bit Register */
#define DMA_CDNE_CDNE_MASK                       (0xFU)
#define DMA_CDNE_CDNE_SHIFT                      (0U)
#define DMA_CDNE_CDNE(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_CDNE_CDNE_SHIFT)) & DMA_CDNE_CDNE_MASK)
#define DMA_CDNE_CADN                            (0x40U)
#define DMA_CDNE_NOP                             (0x80U)

/*! @name SSRT - Set START Bit Register */
#define DMA_SSRT_SSRT_MASK                       (0xFU)
#define DMA_SSRT_SSRT_SHIFT                      (0U)
#define DMA_SSRT_SSRT(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_SSRT_SSRT_SHIFT)) & DMA_SSRT_SSRT_MASK)
#define DMA_SSRT_SAST                            (0x40U)
#define DMA_SSRT_NOP                             (0x80U)

/*! @name CERR - Clear Error Register */
#define DMA_CERR_CERR_MASK                       (0xFU)
#define DMA_CERR_CERR_SHIFT                      (0U)
#define DMA_CERR_CERR(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_CERR_CERR_SHIFT)) & DMA_CERR_CERR_MASK)
#define DMA_CERR_CAEI                            (0x40U)
#define DMA_CERR_NOP                             (0x80U)

/*! @name CINT - Clear Interrupt Request Register */
#define DMA_CINT_CINT_MASK                       (0xFU)
#define DMA_CINT_CINT_SHIFT                      (0U)
#define DMA_CINT_CINT(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_CINT_CINT_SHIFT)) & DMA_CINT_CINT_MASK)
#define DMA_CINT_CAIR                            (0x40U)
#define DMA_CINT_NOP                             (0x80U)

/*! @name DCHPRIn - Channel n Priority Register */
#define DMA_DCHPRIn_CHPRI_MASK                   (0xFU)
#define DMA_DCHPRIn_CHPRI_SHIFT                  (0U)
#define DMA_DCHPRI3_CHPRI(x)                     (((uint8_t)(((uint8_t)(x)) << DMA_DCHPRI3_CHPRI_SHIFT)) & DMA_DCHPRI3_CHPRI_MASK)
#define DMA_DCHPRIn_DPA                          (0x40U)
#define DMA_DCHPRIn_ECP                          (0x80U)

/*! @name SOFF - TCD Signed Source Address Offset */
#define DMA_SOFF_SOFF_MASK                       (0xFFFFU)
#define DMA_SOFF_SOFF_SHIFT                      (0U)
#define DMA_SOFF_SOFF(x)                         (((uint16_t)(((uint16_t)(x)) << DMA_SOFF_SOFF_SHIFT)) & DMA_SOFF_SOFF_MASK)

/*! @name ATTR - TCD Transfer Attributes */
#define DMA_ATTR_DSIZE_MASK                      (0x7U)
#define DMA_ATTR_DSIZE_SHIFT                     (0U)
#define DMA_ATTR_DSIZE(x)                        (((uint16_t)(((uint16_t)(x)) << DMA_ATTR_DSIZE_SHIFT)) & DMA_ATTR_DSIZE_MASK)
#define DMA_ATTR_DMOD_MASK                       (0xF8U)
#define DMA_ATTR_DMOD_SHIFT                      (3U)
#define DMA_ATTR_DMOD(x)                         (((uint16_t)(((uint16_t)(x)) << DMA_ATTR_DMOD_SHIFT)) & DMA_ATTR_DMOD_MASK)
#define DMA_ATTR_SSIZE_MASK                      (0x700U)
#define DMA_ATTR_SSIZE_SHIFT                     (8U)
#define DMA_ATTR_SSIZE(x)                        (((uint16_t)(((uint16_t)(x)) << DMA_ATTR_SSIZE_SHIFT)) & DMA_ATTR_SSIZE_MASK)
#define DMA_ATTR_SMOD_MASK                       (0xF800U)
#define DMA_ATTR_SMOD_SHIFT                      (11U)
#define DMA_ATTR_SMOD(x)                         (((uint16_t)(((uint16_t)(x)) << DMA_ATTR_SMOD_SHIFT)) & DMA_ATTR_SMOD_MASK)

/*! @name NBYTES_MLOFFNO - TCD Signed Minor Loop Offset (Minor Loop Enabled and Offset Disabled) */
#define DMA_NBYTES_MLOFFNO_NBYTES_MASK           (0x3FFFFFFFU)
#define DMA_NBYTES_MLOFFNO_NBYTES_SHIFT          (0U)
#define DMA_NBYTES_MLOFFNO_NBYTES(x)             (((uint32_t)(((uint32_t)(x)) << DMA_NBYTES_MLOFFNO_NBYTES_SHIFT)) & DMA_NBYTES_MLOFFNO_NBYTES_MASK)
#define DMA_NBYTES_MLOFFNO_DMLOE                 (0x40000000U)
#define DMA_NBYTES_MLOFFNO_SMLOE                 (0x80000000U)

/*! @name NBYTES_MLOFFYES - TCD Signed Minor Loop Offset (Minor Loop and Offset Enabled) */
#define DMA_NBYTES_MLOFFYES_NBYTES_MASK          (0x3FFU)
#define DMA_NBYTES_MLOFFYES_NBYTES_SHIFT         (0U)
#define DMA_NBYTES_MLOFFYES_NBYTES(x)            (((uint32_t)(((uint32_t)(x)) << DMA_NBYTES_MLOFFYES_NBYTES_SHIFT)) & DMA_NBYTES_MLOFFYES_NBYTES_MASK)
#define DMA_NBYTES_MLOFFYES_MLOFF_MASK           (0x3FFFFC00U)
#define DMA_NBYTES_MLOFFYES_MLOFF_SHIFT          (10U)
#define DMA_NBYTES_MLOFFYES_MLOFF(x)             (((uint32_t)(((uint32_t)(x)) << DMA_NBYTES_MLOFFYES_MLOFF_SHIFT)) & DMA_NBYTES_MLOFFYES_MLOFF_MASK)
#define DMA_NBYTES_MLOFFYES_DMLOE                (0x40000000U)
#define DMA_NBYTES_MLOFFYES_SMLOE                (0x80000000U)

/*! @name DOFF - TCD Signed Destination Address Offset */
#define DMA_DOFF_DOFF_MASK                       (0xFFFFU)
#define DMA_DOFF_DOFF_SHIFT                      (0U)
#define DMA_DOFF_DOFF(x)                         (((uint16_t)(((uint16_t)(x)) << DMA_DOFF_DOFF_SHIFT)) & DMA_DOFF_DOFF_MASK)

/*! @name CITER_ELINKNO - TCD Current Minor Loop Link, Major Loop Count (Channel Linking Disabled) */
#define DMA_CITER_ELINKNO_CITER_MASK             (0x7FFFU)
#define DMA_CITER_ELINKNO_CITER_SHIFT            (0U)
#define DMA_CITER_ELINKNO_CITER(x)               (((uint16_t)(((uint16_t)(x)) << DMA_CITER_ELINKNO_CITER_SHIFT)) & DMA_CITER_ELINKNO_CITER_MASK)
#define DMA_CITER_ELINKNO_ELINK                  (0x8000U)

/*! @name CITER_ELINKYES - TCD Current Minor Loop Link, Major Loop Count (Channel Linking Enabled) */
#define DMA_CITER_ELINKYES_CITER_MASK            (0x1FFU)
#define DMA_CITER_ELINKYES_CITER_SHIFT           (0U)
#define DMA_CITER_ELINKYES_CITER(x)              (((uint16_t)(((uint16_t)(x)) << DMA_CITER_ELINKYES_CITER_SHIFT)) & DMA_CITER_ELINKYES_CITER_MASK)
#define DMA_CITER_ELINKYES_LINKCH_MASK           (0x1E00U)
#define DMA_CITER_ELINKYES_LINKCH_SHIFT          (9U)
#define DMA_CITER_ELINKYES_LINKCH(x)             (((uint16_t)(((uint16_t)(x)) << DMA_CITER_ELINKYES_LINKCH_SHIFT)) & DMA_CITER_ELINKYES_LINKCH_MASK)
#define DMA_CITER_ELINKYES_ELINK                 (0x8000U)

/*! @name CSR - TCD Control and Status */
#define DMA_CSR_START                            (0x1U)
#define DMA_CSR_INTMAJOR                         (0x2U)
#define DMA_CSR_INTHALF                          (0x4U)
#define DMA_CSR_DREQ                             (0x8U)
#define DMA_CSR_ESG                              (0x10U)
#define DMA_CSR_MAJORELINK                       (0x20U)
#define DMA_CSR_ACTIVE                           (0x40U)
#define DMA_CSR_DONE                             (0x80U)
#define DMA_CSR_MAJORLINKCH_MASK                 (0xF00U)
#define DMA_CSR_MAJORLINKCH_SHIFT                (8U)
#define DMA_CSR_MAJORLINKCH(x)                   (((uint16_t)(((uint16_t)(x)) << DMA_CSR_MAJORLINKCH_SHIFT)) & DMA_CSR_MAJORLINKCH_MASK)
#define DMA_CSR_BWC_MASK                         (0xC000U)
#define DMA_CSR_BWC_SHIFT                        (14U)
#define DMA_CSR_BWC(x)                           (((uint16_t)(((uint16_t)(x)) << DMA_CSR_BWC_SHIFT)) & DMA_CSR_BWC_MASK)

/*! @name BITER_ELINKNO - TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Disabled) */
#define DMA_BITER_ELINKNO_BITER_MASK             (0x7FFFU)
#define DMA_BITER_ELINKNO_BITER_SHIFT            (0U)
#define DMA_BITER_ELINKNO_BITER(x)               (((uint16_t)(((uint16_t)(x)) << DMA_BITER_ELINKNO_BITER_SHIFT)) & DMA_BITER_ELINKNO_BITER_MASK)
#define DMA_BITER_ELINKNO_ELINK                  (0x8000U)

/*! @name BITER_ELINKYES - TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Enabled) */
#define DMA_BITER_ELINKYES_BITER_MASK            (0x1FFU)
#define DMA_BITER_ELINKYES_BITER_SHIFT           (0U)
#define DMA_BITER_ELINKYES_BITER(x)              (((uint16_t)(((uint16_t)(x)) << DMA_BITER_ELINKYES_BITER_SHIFT)) & DMA_BITER_ELINKYES_BITER_MASK)
#define DMA_BITER_ELINKYES_LINKCH_MASK           (0x1E00U)
#define DMA_BITER_ELINKYES_LINKCH_SHIFT          (9U)
#define DMA_BITER_ELINKYES_LINKCH(x)             (((uint16_t)(((uint16_t)(x)) << DMA_BITER_ELINKYES_LINKCH_SHIFT)) & DMA_BITER_ELINKYES_LINKCH_MASK)
#define DMA_BITER_ELINKYES_ELINK                 (0x8000U)

/* The count of DMA_TCD */
#define DMA_TCD_COUNT                            (16U)

/*!
 * @}
 */ /* end of group DMA_Register_Masks */


/* DMA - Peripheral instance base addresses */
/** Peripheral DMA base address */
#define DMA_BASE                                 (0x40008000u)
/** Peripheral DMA base pointer */
#define DMA0                                     ((DMA_TypeDef *)DMA_BASE)
/** Array initializer of DMA peripheral base addresses */
#define DMA_BASE_ADDRS                           { DMA_BASE }
/** Array initializer of DMA peripheral base pointers */
#define DMA_BASE_PTRS                            { DMA0 }
/** Interrupt vectors for the DMA peripheral type */
#define DMA_CHN_IRQS                             { DMA0_IRQn, DMA1_IRQn, DMA2_IRQn, DMA3_IRQn, DMA4_IRQn, DMA5_IRQn, DMA6_IRQn, DMA7_IRQn, DMA8_IRQn, DMA9_IRQn, DMA10_IRQn, DMA11_IRQn, DMA12_IRQn, DMA13_IRQn, DMA14_IRQn, DMA15_IRQn }
#define DMA_ERROR_IRQS                           { DMA_Error_IRQn }

/*!
 * @}
 */ /* end of group DMA_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- DMAMUX Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup DMAMUX_Peripheral_Access_Layer DMAMUX Peripheral Access Layer
 * @{
 */

/** DMAMUX - Register Layout Typedef */
typedef struct {
  __IO uint8_t CHCFG[16];                          /**< Channel Configuration register, array offset: 0x0, array step: 0x1 */
} DMAMUX_TypeDef;

/* ----------------------------------------------------------------------------
   -- DMAMUX Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup DMAMUX_Register_Masks DMAMUX Register Masks
 * @{
 */

/*! @name CHCFG - Channel Configuration register */
#define DMAMUX_CHCFG_SOURCE_MASK                 (0x3FU)
#define DMAMUX_CHCFG_SOURCE_SHIFT                (0U)
#define DMAMUX_CHCFG_SOURCE(x)                   (((uint8_t)(((uint8_t)(x)) << DMAMUX_CHCFG_SOURCE_SHIFT)) & DMAMUX_CHCFG_SOURCE_MASK)
#define DMAMUX_CHCFG_TRIG                        (0x40U)
#define DMAMUX_CHCFG_ENBL                        (0x80U)

/* The count of DMAMUX_CHCFG */
#define DMAMUX_CHCFG_COUNT                       (16U)


/*!
 * @}
 */ /* end of group DMAMUX_Register_Masks */


/* DMAMUX - Peripheral instance base addresses */
/** Peripheral DMAMUX base address */
#define DMAMUX_BASE                              (0x40021000u)
/** Peripheral DMAMUX base pointer */
#define DMAMUX                                   ((DMAMUX_TypeDef *)DMAMUX_BASE)
/** Array initializer of DMAMUX peripheral base addresses */
#define DMAMUX_BASE_ADDRS                        { DMAMUX_BASE }
/** Array initializer of DMAMUX peripheral base pointers */
#define DMAMUX_BASE_PTRS                         { DMAMUX }

/*!
 * @}
 */ /* end of group DMAMUX_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- ENET Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup ENET_Peripheral_Access_Layer ENET Peripheral Access Layer
 * @{
 */

/** ENET - Register Layout Typedef */
typedef struct {
       uint8_t RESERVED_0[4];
  __IO uint32_t EIR;                               /**< Interrupt Event Register, offset: 0x4 */
  __IO uint32_t EIMR;                              /**< Interrupt Mask Register, offset: 0x8 */
       uint8_t RESERVED_1[4];
  __IO uint32_t RDAR;                              /**< Receive Descriptor Active Register, offset: 0x10 */
  __IO uint32_t TDAR;                              /**< Transmit Descriptor Active Register, offset: 0x14 */
       uint8_t RESERVED_2[12];
  __IO uint32_t ECR;                               /**< Ethernet Control Register, offset: 0x24 */
       uint8_t RESERVED_3[24];
  __IO uint32_t MMFR;                              /**< MII Management Frame Register, offset: 0x40 */
  __IO uint32_t MSCR;                              /**< MII Speed Control Register, offset: 0x44 */
       uint8_t RESERVED_4[28];
  __IO uint32_t MIBC;                              /**< MIB Control Register, offset: 0x64 */
       uint8_t RESERVED_5[28];
  __IO uint32_t RCR;                               /**< Receive Control Register, offset: 0x84 */
       uint8_t RESERVED_6[60];
  __IO uint32_t TCR;                               /**< Transmit Control Register, offset: 0xC4 */
       uint8_t RESERVED_7[28];
  __IO uint32_t PALR;                              /**< Physical Address Lower Register, offset: 0xE4 */
  __IO uint32_t PAUR;                              /**< Physical Address Upper Register, offset: 0xE8 */
  __IO uint32_t OPD;                               /**< Opcode/Pause Duration Register, offset: 0xEC */
       uint8_t RESERVED_8[40];
  __IO uint32_t IAUR;                              /**< Descriptor Individual Upper Address Register, offset: 0x118 */
  __IO uint32_t IALR;                              /**< Descriptor Individual Lower Address Register, offset: 0x11C */
  __IO uint32_t GAUR;                              /**< Descriptor Group Upper Address Register, offset: 0x120 */
  __IO uint32_t GALR;                              /**< Descriptor Group Lower Address Register, offset: 0x124 */
       uint8_t RESERVED_9[28];
  __IO uint32_t TFWR;                              /**< Transmit FIFO Watermark Register, offset: 0x144 */
       uint8_t RESERVED_10[56];
  __IO uint32_t RDSR;                              /**< Receive Descriptor Ring Start Register, offset: 0x180 */
  __IO uint32_t TDSR;                              /**< Transmit Buffer Descriptor Ring Start Register, offset: 0x184 */
  __IO uint32_t MRBR;                              /**< Maximum Receive Buffer Size Register, offset: 0x188 */
       uint8_t RESERVED_11[4];
  __IO uint32_t RSFL;                              /**< Receive FIFO Section Full Threshold, offset: 0x190 */
  __IO uint32_t RSEM;                              /**< Receive FIFO Section Empty Threshold, offset: 0x194 */
  __IO uint32_t RAEM;                              /**< Receive FIFO Almost Empty Threshold, offset: 0x198 */
  __IO uint32_t RAFL;                              /**< Receive FIFO Almost Full Threshold, offset: 0x19C */
  __IO uint32_t TSEM;                              /**< Transmit FIFO Section Empty Threshold, offset: 0x1A0 */
  __IO uint32_t TAEM;                              /**< Transmit FIFO Almost Empty Threshold, offset: 0x1A4 */
  __IO uint32_t TAFL;                              /**< Transmit FIFO Almost Full Threshold, offset: 0x1A8 */
  __IO uint32_t TIPG;                              /**< Transmit Inter-Packet Gap, offset: 0x1AC */
  __IO uint32_t FTRL;                              /**< Frame Truncation Length, offset: 0x1B0 */
       uint8_t RESERVED_12[12];
  __IO uint32_t TACC;                              /**< Transmit Accelerator Function Configuration, offset: 0x1C0 */
  __IO uint32_t RACC;                              /**< Receive Accelerator Function Configuration, offset: 0x1C4 */
       uint8_t RESERVED_13[60];
  __I  uint32_t RMON_T_PACKETS;                    /**< Tx Packet Count Statistic Register, offset: 0x204 */
  __I  uint32_t RMON_T_BC_PKT;                     /**< Tx Broadcast Packets Statistic Register, offset: 0x208 */
  __I  uint32_t RMON_T_MC_PKT;                     /**< Tx Multicast Packets Statistic Register, offset: 0x20C */
  __I  uint32_t RMON_T_CRC_ALIGN;                  /**< Tx Packets with CRC/Align Error Statistic Register, offset: 0x210 */
  __I  uint32_t RMON_T_UNDERSIZE;                  /**< Tx Packets Less Than Bytes and Good CRC Statistic Register, offset: 0x214 */
  __I  uint32_t RMON_T_OVERSIZE;                   /**< Tx Packets GT MAX_FL bytes and Good CRC Statistic Register, offset: 0x218 */
  __I  uint32_t RMON_T_FRAG;                       /**< Tx Packets Less Than 64 Bytes and Bad CRC Statistic Register, offset: 0x21C */
  __I  uint32_t RMON_T_JAB;                        /**< Tx Packets Greater Than MAX_FL bytes and Bad CRC Statistic Register, offset: 0x220 */
  __I  uint32_t RMON_T_COL;                        /**< Tx Collision Count Statistic Register, offset: 0x224 */
  __I  uint32_t RMON_T_P64;                        /**< Tx 64-Byte Packets Statistic Register, offset: 0x228 */
  __I  uint32_t RMON_T_P65TO127;                   /**< Tx 65- to 127-byte Packets Statistic Register, offset: 0x22C */
  __I  uint32_t RMON_T_P128TO255;                  /**< Tx 128- to 255-byte Packets Statistic Register, offset: 0x230 */
  __I  uint32_t RMON_T_P256TO511;                  /**< Tx 256- to 511-byte Packets Statistic Register, offset: 0x234 */
  __I  uint32_t RMON_T_P512TO1023;                 /**< Tx 512- to 1023-byte Packets Statistic Register, offset: 0x238 */
  __I  uint32_t RMON_T_P1024TO2047;                /**< Tx 1024- to 2047-byte Packets Statistic Register, offset: 0x23C */
  __I  uint32_t RMON_T_P_GTE2048;                  /**< Tx Packets Greater Than 2048 Bytes Statistic Register, offset: 0x240 */
  __I  uint32_t RMON_T_OCTETS;                     /**< Tx Octets Statistic Register, offset: 0x244 */
       uint8_t RESERVED_14[4];
  __I  uint32_t IEEE_T_FRAME_OK;                   /**< Frames Transmitted OK Statistic Register, offset: 0x24C */
  __I  uint32_t IEEE_T_1COL;                       /**< Frames Transmitted with Single Collision Statistic Register, offset: 0x250 */
  __I  uint32_t IEEE_T_MCOL;                       /**< Frames Transmitted with Multiple Collisions Statistic Register, offset: 0x254 */
  __I  uint32_t IEEE_T_DEF;                        /**< Frames Transmitted after Deferral Delay Statistic Register, offset: 0x258 */
  __I  uint32_t IEEE_T_LCOL;                       /**< Frames Transmitted with Late Collision Statistic Register, offset: 0x25C */
  __I  uint32_t IEEE_T_EXCOL;                      /**< Frames Transmitted with Excessive Collisions Statistic Register, offset: 0x260 */
  __I  uint32_t IEEE_T_MACERR;                     /**< Frames Transmitted with Tx FIFO Underrun Statistic Register, offset: 0x264 */
  __I  uint32_t IEEE_T_CSERR;                      /**< Frames Transmitted with Carrier Sense Error Statistic Register, offset: 0x268 */
       uint8_t RESERVED_15[4];
  __I  uint32_t IEEE_T_FDXFC;                      /**< Flow Control Pause Frames Transmitted Statistic Register, offset: 0x270 */
  __I  uint32_t IEEE_T_OCTETS_OK;                  /**< Octet Count for Frames Transmitted w/o Error Statistic Register, offset: 0x274 */
       uint8_t RESERVED_16[12];
  __I  uint32_t RMON_R_PACKETS;                    /**< Rx Packet Count Statistic Register, offset: 0x284 */
  __I  uint32_t RMON_R_BC_PKT;                     /**< Rx Broadcast Packets Statistic Register, offset: 0x288 */
  __I  uint32_t RMON_R_MC_PKT;                     /**< Rx Multicast Packets Statistic Register, offset: 0x28C */
  __I  uint32_t RMON_R_CRC_ALIGN;                  /**< Rx Packets with CRC/Align Error Statistic Register, offset: 0x290 */
  __I  uint32_t RMON_R_UNDERSIZE;                  /**< Rx Packets with Less Than 64 Bytes and Good CRC Statistic Register, offset: 0x294 */
  __I  uint32_t RMON_R_OVERSIZE;                   /**< Rx Packets Greater Than MAX_FL and Good CRC Statistic Register, offset: 0x298 */
  __I  uint32_t RMON_R_FRAG;                       /**< Rx Packets Less Than 64 Bytes and Bad CRC Statistic Register, offset: 0x29C */
  __I  uint32_t RMON_R_JAB;                        /**< Rx Packets Greater Than MAX_FL Bytes and Bad CRC Statistic Register, offset: 0x2A0 */
       uint8_t RESERVED_17[4];
  __I  uint32_t RMON_R_P64;                        /**< Rx 64-Byte Packets Statistic Register, offset: 0x2A8 */
  __I  uint32_t RMON_R_P65TO127;                   /**< Rx 65- to 127-Byte Packets Statistic Register, offset: 0x2AC */
  __I  uint32_t RMON_R_P128TO255;                  /**< Rx 128- to 255-Byte Packets Statistic Register, offset: 0x2B0 */
  __I  uint32_t RMON_R_P256TO511;                  /**< Rx 256- to 511-Byte Packets Statistic Register, offset: 0x2B4 */
  __I  uint32_t RMON_R_P512TO1023;                 /**< Rx 512- to 1023-Byte Packets Statistic Register, offset: 0x2B8 */
  __I  uint32_t RMON_R_P1024TO2047;                /**< Rx 1024- to 2047-Byte Packets Statistic Register, offset: 0x2BC */
  __I  uint32_t RMON_R_P_GTE2048;                  /**< Rx Packets Greater than 2048 Bytes Statistic Register, offset: 0x2C0 */
  __I  uint32_t RMON_R_OCTETS;                     /**< Rx Octets Statistic Register, offset: 0x2C4 */
  __I  uint32_t IEEE_R_DROP;                       /**< Frames not Counted Correctly Statistic Register, offset: 0x2C8 */
  __I  uint32_t IEEE_R_FRAME_OK;                   /**< Frames Received OK Statistic Register, offset: 0x2CC */
  __I  uint32_t IEEE_R_CRC;                        /**< Frames Received with CRC Error Statistic Register, offset: 0x2D0 */
  __I  uint32_t IEEE_R_ALIGN;                      /**< Frames Received with Alignment Error Statistic Register, offset: 0x2D4 */
  __I  uint32_t IEEE_R_MACERR;                     /**< Receive FIFO Overflow Count Statistic Register, offset: 0x2D8 */
  __I  uint32_t IEEE_R_FDXFC;                      /**< Flow Control Pause Frames Received Statistic Register, offset: 0x2DC */
  __I  uint32_t IEEE_R_OCTETS_OK;                  /**< Octet Count for Frames Received without Error Statistic Register, offset: 0x2E0 */
       uint8_t RESERVED_18[284];
  __IO uint32_t ATCR;                              /**< Adjustable Timer Control Register, offset: 0x400 */
  __IO uint32_t ATVR;                              /**< Timer Value Register, offset: 0x404 */
  __IO uint32_t ATOFF;                             /**< Timer Offset Register, offset: 0x408 */
  __IO uint32_t ATPER;                             /**< Timer Period Register, offset: 0x40C */
  __IO uint32_t ATCOR;                             /**< Timer Correction Register, offset: 0x410 */
  __IO uint32_t ATINC;                             /**< Time-Stamping Clock Period Register, offset: 0x414 */
  __I  uint32_t ATSTMP;                            /**< Timestamp of Last Transmitted Frame, offset: 0x418 */
       uint8_t RESERVED_19[488];
  __IO uint32_t TGSR;                              /**< Timer Global Status Register, offset: 0x604 */
  struct {                                         /* offset: 0x608, array step: 0x8 */
    __IO uint32_t TCSR;                              /**< Timer Control Status Register, array offset: 0x608, array step: 0x8 */
    __IO uint32_t TCCR;                              /**< Timer Compare Capture Register, array offset: 0x60C, array step: 0x8 */
  } CHANNEL[4];
} ENET_TypeDef;

/* ----------------------------------------------------------------------------
   -- ENET Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup ENET_Register_Masks ENET Register Masks
 * @{
 */

/*! @name EIR - Interrupt Event Register */
#define ENET_EIR_TS_TIMER                        (0x8000U)
#define ENET_EIR_TS_AVAIL                        (0x10000U)
#define ENET_EIR_WAKEUP                          (0x20000U)
#define ENET_EIR_PLR                             (0x40000U)
#define ENET_EIR_UN                              (0x80000U)
#define ENET_EIR_RL                              (0x100000U)
#define ENET_EIR_LC                              (0x200000U)
#define ENET_EIR_EBERR                           (0x400000U)
#define ENET_EIR_MII                             (0x800000U)
#define ENET_EIR_RXB                             (0x1000000U)
#define ENET_EIR_RXF                             (0x2000000U)
#define ENET_EIR_TXB                             (0x4000000U)
#define ENET_EIR_TXF                             (0x8000000U)
#define ENET_EIR_GRA                             (0x10000000U)
#define ENET_EIR_BABT                            (0x20000000U)
#define ENET_EIR_BABR                            (0x40000000U)

/*! @name EIMR - Interrupt Mask Register */
#define ENET_EIMR_TS_TIMER                       (0x8000U)
#define ENET_EIMR_TS_AVAIL                       (0x10000U)
#define ENET_EIMR_WAKEUP                         (0x20000U)
#define ENET_EIMR_PLR                            (0x40000U)
#define ENET_EIMR_UN                             (0x80000U)
#define ENET_EIMR_RL                             (0x100000U)
#define ENET_EIMR_LC                             (0x200000U)
#define ENET_EIMR_EBERR                          (0x400000U)
#define ENET_EIMR_MII                            (0x800000U)
#define ENET_EIMR_RXB                            (0x1000000U)
#define ENET_EIMR_RXF                            (0x2000000U)
#define ENET_EIMR_TXB                            (0x4000000U)
#define ENET_EIMR_TXF                            (0x8000000U)
#define ENET_EIMR_GRA                            (0x10000000U)
#define ENET_EIMR_BABT                           (0x20000000U)
#define ENET_EIMR_BABR                           (0x40000000U)

/*! @name RDAR - Receive Descriptor Active Register */
#define ENET_RDAR_RDAR                           (0x1000000U)

/*! @name TDAR - Transmit Descriptor Active Register */
#define ENET_TDAR_TDAR                           (0x1000000U)

/*! @name ECR - Ethernet Control Register */
#define ENET_ECR_RESET                           (0x1U)
#define ENET_ECR_ETHEREN                         (0x2U)
#define ENET_ECR_MAGICEN                         (0x4U)
#define ENET_ECR_SLEEP                           (0x8U)
#define ENET_ECR_EN1588                          (0x10U)
#define ENET_ECR_DBGEN                           (0x40U)
#define ENET_ECR_STOPEN                          (0x80U)
#define ENET_ECR_DBSWP                           (0x100U)

/*! @name MMFR - MII Management Frame Register */
#define ENET_MMFR_DATA_MASK                      (0xFFFFU)
#define ENET_MMFR_DATA_SHIFT                     (0U)
#define ENET_MMFR_DATA(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_MMFR_DATA_SHIFT)) & ENET_MMFR_DATA_MASK)
#define ENET_MMFR_TA_MASK                        (0x30000U)
#define ENET_MMFR_TA_SHIFT                       (16U)
#define ENET_MMFR_TA(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_MMFR_TA_SHIFT)) & ENET_MMFR_TA_MASK)
#define ENET_MMFR_RA_MASK                        (0x7C0000U)
#define ENET_MMFR_RA_SHIFT                       (18U)
#define ENET_MMFR_RA(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_MMFR_RA_SHIFT)) & ENET_MMFR_RA_MASK)
#define ENET_MMFR_PA_MASK                        (0xF800000U)
#define ENET_MMFR_PA_SHIFT                       (23U)
#define ENET_MMFR_PA(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_MMFR_PA_SHIFT)) & ENET_MMFR_PA_MASK)
#define ENET_MMFR_OP_MASK                        (0x30000000U)
#define ENET_MMFR_OP_SHIFT                       (28U)
#define ENET_MMFR_OP(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_MMFR_OP_SHIFT)) & ENET_MMFR_OP_MASK)
#define ENET_MMFR_ST_MASK                        (0xC0000000U)
#define ENET_MMFR_ST_SHIFT                       (30U)
#define ENET_MMFR_ST(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_MMFR_ST_SHIFT)) & ENET_MMFR_ST_MASK)

/*! @name MSCR - MII Speed Control Register */
#define ENET_MSCR_MII_SPEED_MASK                 (0x7EU)
#define ENET_MSCR_MII_SPEED_SHIFT                (1U)
#define ENET_MSCR_MII_SPEED(x)                   (((uint32_t)(((uint32_t)(x)) << ENET_MSCR_MII_SPEED_SHIFT)) & ENET_MSCR_MII_SPEED_MASK)
#define ENET_MSCR_DIS_PRE                        (0x80U)
#define ENET_MSCR_HOLDTIME_MASK                  (0x700U)
#define ENET_MSCR_HOLDTIME_SHIFT                 (8U)
#define ENET_MSCR_HOLDTIME(x)                    (((uint32_t)(((uint32_t)(x)) << ENET_MSCR_HOLDTIME_SHIFT)) & ENET_MSCR_HOLDTIME_MASK)

/*! @name MIBC - MIB Control Register */
#define ENET_MIBC_MIB_CLEAR                      (0x20000000U)
#define ENET_MIBC_MIB_IDLE                       (0x40000000U)
#define ENET_MIBC_MIB_DIS                        (0x80000000U)

/*! @name RCR - Receive Control Register */
#define ENET_RCR_LOOP                            (0x1U)
#define ENET_RCR_DRT                             (0x2U)
#define ENET_RCR_MII_MODE                        (0x4U)
#define ENET_RCR_PROM                            (0x8U)
#define ENET_RCR_BC_REJ                          (0x10U)
#define ENET_RCR_FCE                             (0x20U)
#define ENET_RCR_RMII_MODE                       (0x100U)
#define ENET_RCR_RMII_10T                        (0x200U)
#define ENET_RCR_PADEN                           (0x1000U)
#define ENET_RCR_PAUFWD                          (0x2000U)
#define ENET_RCR_CRCFWD                          (0x4000U)
#define ENET_RCR_CFEN                            (0x8000U)
#define ENET_RCR_MAX_FL_MASK                     (0x3FFF0000U)
#define ENET_RCR_MAX_FL_SHIFT                    (16U)
#define ENET_RCR_MAX_FL(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_RCR_MAX_FL_SHIFT)) & ENET_RCR_MAX_FL_MASK)
#define ENET_RCR_NLC                             (0x40000000U)
#define ENET_RCR_GRS                             (0x80000000U)

/*! @name TCR - Transmit Control Register */
#define ENET_TCR_GTS                             (0x1U)
#define ENET_TCR_FDEN                            (0x4U)
#define ENET_TCR_TFC_PAUSE                       (0x8U)
#define ENET_TCR_RFC_PAUSE                       (0x10U)
#define ENET_TCR_ADDSEL_MASK                     (0xE0U)
#define ENET_TCR_ADDSEL_SHIFT                    (5U)
#define ENET_TCR_ADDSEL(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_TCR_ADDSEL_SHIFT)) & ENET_TCR_ADDSEL_MASK)
#define ENET_TCR_ADDINS                          (0x100U)
#define ENET_TCR_CRCFWD                          (0x200U)

/*! @name PAUR - Physical Address Upper Register */
#define ENET_PAUR_TYPE_MASK                      (0xFFFFU)
#define ENET_PAUR_TYPE_SHIFT                     (0U)
#define ENET_PAUR_TYPE(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_PAUR_TYPE_SHIFT)) & ENET_PAUR_TYPE_MASK)
#define ENET_PAUR_PADDR2_MASK                    (0xFFFF0000U)
#define ENET_PAUR_PADDR2_SHIFT                   (16U)
#define ENET_PAUR_PADDR2(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_PAUR_PADDR2_SHIFT)) & ENET_PAUR_PADDR2_MASK)

/*! @name OPD - Opcode/Pause Duration Register */
#define ENET_OPD_PAUSE_DUR_MASK                  (0xFFFFU)
#define ENET_OPD_PAUSE_DUR_SHIFT                 (0U)
#define ENET_OPD_PAUSE_DUR(x)                    (((uint32_t)(((uint32_t)(x)) << ENET_OPD_PAUSE_DUR_SHIFT)) & ENET_OPD_PAUSE_DUR_MASK)
#define ENET_OPD_OPCODE_MASK                     (0xFFFF0000U)
#define ENET_OPD_OPCODE_SHIFT                    (16U)
#define ENET_OPD_OPCODE(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_OPD_OPCODE_SHIFT)) & ENET_OPD_OPCODE_MASK)

/*! @name TFWR - Transmit FIFO Watermark Register */
#define ENET_TFWR_TFWR_MASK                      (0x3FU)
#define ENET_TFWR_TFWR_SHIFT                     (0U)
#define ENET_TFWR_TFWR(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_TFWR_TFWR_SHIFT)) & ENET_TFWR_TFWR_MASK)
#define ENET_TFWR_STRFWD                         (0x100U)

/*! @name RDSR - Receive Descriptor Ring Start Register */
#define ENET_RDSR_R_DES_START_MASK               (0xFFFFFFF8U)
#define ENET_RDSR_R_DES_START_SHIFT              (3U)
#define ENET_RDSR_R_DES_START(x)                 (((uint32_t)(((uint32_t)(x)) << ENET_RDSR_R_DES_START_SHIFT)) & ENET_RDSR_R_DES_START_MASK)

/*! @name TDSR - Transmit Buffer Descriptor Ring Start Register */
#define ENET_TDSR_X_DES_START_MASK               (0xFFFFFFF8U)
#define ENET_TDSR_X_DES_START_SHIFT              (3U)
#define ENET_TDSR_X_DES_START(x)                 (((uint32_t)(((uint32_t)(x)) << ENET_TDSR_X_DES_START_SHIFT)) & ENET_TDSR_X_DES_START_MASK)

/*! @name MRBR - Maximum Receive Buffer Size Register */
#define ENET_MRBR_R_BUF_SIZE_MASK                (0x3FF0U)
#define ENET_MRBR_R_BUF_SIZE_SHIFT               (4U)
#define ENET_MRBR_R_BUF_SIZE(x)                  (((uint32_t)(((uint32_t)(x)) << ENET_MRBR_R_BUF_SIZE_SHIFT)) & ENET_MRBR_R_BUF_SIZE_MASK)

/*! @name RSFL - Receive FIFO Section Full Threshold */
#define ENET_RSFL_RX_SECTION_FULL_MASK           (0xFFU)
#define ENET_RSFL_RX_SECTION_FULL_SHIFT          (0U)
#define ENET_RSFL_RX_SECTION_FULL(x)             (((uint32_t)(((uint32_t)(x)) << ENET_RSFL_RX_SECTION_FULL_SHIFT)) & ENET_RSFL_RX_SECTION_FULL_MASK)

/*! @name RSEM - Receive FIFO Section Empty Threshold */
#define ENET_RSEM_RX_SECTION_EMPTY_MASK          (0xFFU)
#define ENET_RSEM_RX_SECTION_EMPTY_SHIFT         (0U)
#define ENET_RSEM_RX_SECTION_EMPTY(x)            (((uint32_t)(((uint32_t)(x)) << ENET_RSEM_RX_SECTION_EMPTY_SHIFT)) & ENET_RSEM_RX_SECTION_EMPTY_MASK)
#define ENET_RSEM_STAT_SECTION_EMPTY_MASK        (0x1F0000U)
#define ENET_RSEM_STAT_SECTION_EMPTY_SHIFT       (16U)
#define ENET_RSEM_STAT_SECTION_EMPTY(x)          (((uint32_t)(((uint32_t)(x)) << ENET_RSEM_STAT_SECTION_EMPTY_SHIFT)) & ENET_RSEM_STAT_SECTION_EMPTY_MASK)

/*! @name RAEM - Receive FIFO Almost Empty Threshold */
#define ENET_RAEM_RX_ALMOST_EMPTY_MASK           (0xFFU)
#define ENET_RAEM_RX_ALMOST_EMPTY_SHIFT          (0U)
#define ENET_RAEM_RX_ALMOST_EMPTY(x)             (((uint32_t)(((uint32_t)(x)) << ENET_RAEM_RX_ALMOST_EMPTY_SHIFT)) & ENET_RAEM_RX_ALMOST_EMPTY_MASK)

/*! @name RAFL - Receive FIFO Almost Full Threshold */
#define ENET_RAFL_RX_ALMOST_FULL_MASK            (0xFFU)
#define ENET_RAFL_RX_ALMOST_FULL_SHIFT           (0U)
#define ENET_RAFL_RX_ALMOST_FULL(x)              (((uint32_t)(((uint32_t)(x)) << ENET_RAFL_RX_ALMOST_FULL_SHIFT)) & ENET_RAFL_RX_ALMOST_FULL_MASK)

/*! @name TSEM - Transmit FIFO Section Empty Threshold */
#define ENET_TSEM_TX_SECTION_EMPTY_MASK          (0xFFU)
#define ENET_TSEM_TX_SECTION_EMPTY_SHIFT         (0U)
#define ENET_TSEM_TX_SECTION_EMPTY(x)            (((uint32_t)(((uint32_t)(x)) << ENET_TSEM_TX_SECTION_EMPTY_SHIFT)) & ENET_TSEM_TX_SECTION_EMPTY_MASK)

/*! @name TAEM - Transmit FIFO Almost Empty Threshold */
#define ENET_TAEM_TX_ALMOST_EMPTY_MASK           (0xFFU)
#define ENET_TAEM_TX_ALMOST_EMPTY_SHIFT          (0U)
#define ENET_TAEM_TX_ALMOST_EMPTY(x)             (((uint32_t)(((uint32_t)(x)) << ENET_TAEM_TX_ALMOST_EMPTY_SHIFT)) & ENET_TAEM_TX_ALMOST_EMPTY_MASK)

/*! @name TAFL - Transmit FIFO Almost Full Threshold */
#define ENET_TAFL_TX_ALMOST_FULL_MASK            (0xFFU)
#define ENET_TAFL_TX_ALMOST_FULL_SHIFT           (0U)
#define ENET_TAFL_TX_ALMOST_FULL(x)              (((uint32_t)(((uint32_t)(x)) << ENET_TAFL_TX_ALMOST_FULL_SHIFT)) & ENET_TAFL_TX_ALMOST_FULL_MASK)

/*! @name TIPG - Transmit Inter-Packet Gap */
#define ENET_TIPG_IPG_MASK                       (0x1FU)
#define ENET_TIPG_IPG_SHIFT                      (0U)
#define ENET_TIPG_IPG(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_TIPG_IPG_SHIFT)) & ENET_TIPG_IPG_MASK)

/*! @name FTRL - Frame Truncation Length */
#define ENET_FTRL_TRUNC_FL_MASK                  (0x3FFFU)
#define ENET_FTRL_TRUNC_FL_SHIFT                 (0U)
#define ENET_FTRL_TRUNC_FL(x)                    (((uint32_t)(((uint32_t)(x)) << ENET_FTRL_TRUNC_FL_SHIFT)) & ENET_FTRL_TRUNC_FL_MASK)

/*! @name TACC - Transmit Accelerator Function Configuration */
#define ENET_TACC_SHIFT16                        (0x1U)
#define ENET_TACC_IPCHK                          (0x8U)
#define ENET_TACC_PROCHK                         (0x10U)

/*! @name RACC - Receive Accelerator Function Configuration */
#define ENET_RACC_PADREM                         (0x1U)
#define ENET_RACC_IPDIS                          (0x2U)
#define ENET_RACC_PRODIS                         (0x4U)
#define ENET_RACC_LINEDIS                        (0x40U)
#define ENET_RACC_SHIFT16                        (0x80U)

/*! @name RMON_T_PACKETS - Tx Packet Count Statistic Register */
#define ENET_RMON_T_PACKETS_TXPKTS_MASK          (0xFFFFU)
#define ENET_RMON_T_PACKETS_TXPKTS_SHIFT         (0U)
#define ENET_RMON_T_PACKETS_TXPKTS(x)            (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_PACKETS_TXPKTS_SHIFT)) & ENET_RMON_T_PACKETS_TXPKTS_MASK)

/*! @name RMON_T_BC_PKT - Tx Broadcast Packets Statistic Register */
#define ENET_RMON_T_BC_PKT_TXPKTS_MASK           (0xFFFFU)
#define ENET_RMON_T_BC_PKT_TXPKTS_SHIFT          (0U)
#define ENET_RMON_T_BC_PKT_TXPKTS(x)             (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_BC_PKT_TXPKTS_SHIFT)) & ENET_RMON_T_BC_PKT_TXPKTS_MASK)

/*! @name RMON_T_MC_PKT - Tx Multicast Packets Statistic Register */
#define ENET_RMON_T_MC_PKT_TXPKTS_MASK           (0xFFFFU)
#define ENET_RMON_T_MC_PKT_TXPKTS_SHIFT          (0U)
#define ENET_RMON_T_MC_PKT_TXPKTS(x)             (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_MC_PKT_TXPKTS_SHIFT)) & ENET_RMON_T_MC_PKT_TXPKTS_MASK)

/*! @name RMON_T_CRC_ALIGN - Tx Packets with CRC/Align Error Statistic Register */
#define ENET_RMON_T_CRC_ALIGN_TXPKTS_MASK        (0xFFFFU)
#define ENET_RMON_T_CRC_ALIGN_TXPKTS_SHIFT       (0U)
#define ENET_RMON_T_CRC_ALIGN_TXPKTS(x)          (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_CRC_ALIGN_TXPKTS_SHIFT)) & ENET_RMON_T_CRC_ALIGN_TXPKTS_MASK)

/*! @name RMON_T_UNDERSIZE - Tx Packets Less Than Bytes and Good CRC Statistic Register */
#define ENET_RMON_T_UNDERSIZE_TXPKTS_MASK        (0xFFFFU)
#define ENET_RMON_T_UNDERSIZE_TXPKTS_SHIFT       (0U)
#define ENET_RMON_T_UNDERSIZE_TXPKTS(x)          (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_UNDERSIZE_TXPKTS_SHIFT)) & ENET_RMON_T_UNDERSIZE_TXPKTS_MASK)

/*! @name RMON_T_OVERSIZE - Tx Packets GT MAX_FL bytes and Good CRC Statistic Register */
#define ENET_RMON_T_OVERSIZE_TXPKTS_MASK         (0xFFFFU)
#define ENET_RMON_T_OVERSIZE_TXPKTS_SHIFT        (0U)
#define ENET_RMON_T_OVERSIZE_TXPKTS(x)           (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_OVERSIZE_TXPKTS_SHIFT)) & ENET_RMON_T_OVERSIZE_TXPKTS_MASK)

/*! @name RMON_T_FRAG - Tx Packets Less Than 64 Bytes and Bad CRC Statistic Register */
#define ENET_RMON_T_FRAG_TXPKTS_MASK             (0xFFFFU)
#define ENET_RMON_T_FRAG_TXPKTS_SHIFT            (0U)
#define ENET_RMON_T_FRAG_TXPKTS(x)               (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_FRAG_TXPKTS_SHIFT)) & ENET_RMON_T_FRAG_TXPKTS_MASK)

/*! @name RMON_T_JAB - Tx Packets Greater Than MAX_FL bytes and Bad CRC Statistic Register */
#define ENET_RMON_T_JAB_TXPKTS_MASK              (0xFFFFU)
#define ENET_RMON_T_JAB_TXPKTS_SHIFT             (0U)
#define ENET_RMON_T_JAB_TXPKTS(x)                (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_JAB_TXPKTS_SHIFT)) & ENET_RMON_T_JAB_TXPKTS_MASK)

/*! @name RMON_T_COL - Tx Collision Count Statistic Register */
#define ENET_RMON_T_COL_TXPKTS_MASK              (0xFFFFU)
#define ENET_RMON_T_COL_TXPKTS_SHIFT             (0U)
#define ENET_RMON_T_COL_TXPKTS(x)                (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_COL_TXPKTS_SHIFT)) & ENET_RMON_T_COL_TXPKTS_MASK)

/*! @name RMON_T_P64 - Tx 64-Byte Packets Statistic Register */
#define ENET_RMON_T_P64_TXPKTS_MASK              (0xFFFFU)
#define ENET_RMON_T_P64_TXPKTS_SHIFT             (0U)
#define ENET_RMON_T_P64_TXPKTS(x)                (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_P64_TXPKTS_SHIFT)) & ENET_RMON_T_P64_TXPKTS_MASK)

/*! @name RMON_T_P65TO127 - Tx 65- to 127-byte Packets Statistic Register */
#define ENET_RMON_T_P65TO127_TXPKTS_MASK         (0xFFFFU)
#define ENET_RMON_T_P65TO127_TXPKTS_SHIFT        (0U)
#define ENET_RMON_T_P65TO127_TXPKTS(x)           (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_P65TO127_TXPKTS_SHIFT)) & ENET_RMON_T_P65TO127_TXPKTS_MASK)

/*! @name RMON_T_P128TO255 - Tx 128- to 255-byte Packets Statistic Register */
#define ENET_RMON_T_P128TO255_TXPKTS_MASK        (0xFFFFU)
#define ENET_RMON_T_P128TO255_TXPKTS_SHIFT       (0U)
#define ENET_RMON_T_P128TO255_TXPKTS(x)          (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_P128TO255_TXPKTS_SHIFT)) & ENET_RMON_T_P128TO255_TXPKTS_MASK)

/*! @name RMON_T_P256TO511 - Tx 256- to 511-byte Packets Statistic Register */
#define ENET_RMON_T_P256TO511_TXPKTS_MASK        (0xFFFFU)
#define ENET_RMON_T_P256TO511_TXPKTS_SHIFT       (0U)
#define ENET_RMON_T_P256TO511_TXPKTS(x)          (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_P256TO511_TXPKTS_SHIFT)) & ENET_RMON_T_P256TO511_TXPKTS_MASK)

/*! @name RMON_T_P512TO1023 - Tx 512- to 1023-byte Packets Statistic Register */
#define ENET_RMON_T_P512TO1023_TXPKTS_MASK       (0xFFFFU)
#define ENET_RMON_T_P512TO1023_TXPKTS_SHIFT      (0U)
#define ENET_RMON_T_P512TO1023_TXPKTS(x)         (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_P512TO1023_TXPKTS_SHIFT)) & ENET_RMON_T_P512TO1023_TXPKTS_MASK)

/*! @name RMON_T_P1024TO2047 - Tx 1024- to 2047-byte Packets Statistic Register */
#define ENET_RMON_T_P1024TO2047_TXPKTS_MASK      (0xFFFFU)
#define ENET_RMON_T_P1024TO2047_TXPKTS_SHIFT     (0U)
#define ENET_RMON_T_P1024TO2047_TXPKTS(x)        (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_P1024TO2047_TXPKTS_SHIFT)) & ENET_RMON_T_P1024TO2047_TXPKTS_MASK)

/*! @name RMON_T_P_GTE2048 - Tx Packets Greater Than 2048 Bytes Statistic Register */
#define ENET_RMON_T_P_GTE2048_TXPKTS_MASK        (0xFFFFU)
#define ENET_RMON_T_P_GTE2048_TXPKTS_SHIFT       (0U)
#define ENET_RMON_T_P_GTE2048_TXPKTS(x)          (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_P_GTE2048_TXPKTS_SHIFT)) & ENET_RMON_T_P_GTE2048_TXPKTS_MASK)

/*! @name IEEE_T_FRAME_OK - Frames Transmitted OK Statistic Register */
#define ENET_IEEE_T_FRAME_OK_COUNT_MASK          (0xFFFFU)
#define ENET_IEEE_T_FRAME_OK_COUNT_SHIFT         (0U)
#define ENET_IEEE_T_FRAME_OK_COUNT(x)            (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_FRAME_OK_COUNT_SHIFT)) & ENET_IEEE_T_FRAME_OK_COUNT_MASK)

/*! @name IEEE_T_1COL - Frames Transmitted with Single Collision Statistic Register */
#define ENET_IEEE_T_1COL_COUNT_MASK              (0xFFFFU)
#define ENET_IEEE_T_1COL_COUNT_SHIFT             (0U)
#define ENET_IEEE_T_1COL_COUNT(x)                (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_1COL_COUNT_SHIFT)) & ENET_IEEE_T_1COL_COUNT_MASK)

/*! @name IEEE_T_MCOL - Frames Transmitted with Multiple Collisions Statistic Register */
#define ENET_IEEE_T_MCOL_COUNT_MASK              (0xFFFFU)
#define ENET_IEEE_T_MCOL_COUNT_SHIFT             (0U)
#define ENET_IEEE_T_MCOL_COUNT(x)                (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_MCOL_COUNT_SHIFT)) & ENET_IEEE_T_MCOL_COUNT_MASK)

/*! @name IEEE_T_DEF - Frames Transmitted after Deferral Delay Statistic Register */
#define ENET_IEEE_T_DEF_COUNT_MASK               (0xFFFFU)
#define ENET_IEEE_T_DEF_COUNT_SHIFT              (0U)
#define ENET_IEEE_T_DEF_COUNT(x)                 (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_DEF_COUNT_SHIFT)) & ENET_IEEE_T_DEF_COUNT_MASK)

/*! @name IEEE_T_LCOL - Frames Transmitted with Late Collision Statistic Register */
#define ENET_IEEE_T_LCOL_COUNT_MASK              (0xFFFFU)
#define ENET_IEEE_T_LCOL_COUNT_SHIFT             (0U)
#define ENET_IEEE_T_LCOL_COUNT(x)                (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_LCOL_COUNT_SHIFT)) & ENET_IEEE_T_LCOL_COUNT_MASK)

/*! @name IEEE_T_EXCOL - Frames Transmitted with Excessive Collisions Statistic Register */
#define ENET_IEEE_T_EXCOL_COUNT_MASK             (0xFFFFU)
#define ENET_IEEE_T_EXCOL_COUNT_SHIFT            (0U)
#define ENET_IEEE_T_EXCOL_COUNT(x)               (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_EXCOL_COUNT_SHIFT)) & ENET_IEEE_T_EXCOL_COUNT_MASK)

/*! @name IEEE_T_MACERR - Frames Transmitted with Tx FIFO Underrun Statistic Register */
#define ENET_IEEE_T_MACERR_COUNT_MASK            (0xFFFFU)
#define ENET_IEEE_T_MACERR_COUNT_SHIFT           (0U)
#define ENET_IEEE_T_MACERR_COUNT(x)              (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_MACERR_COUNT_SHIFT)) & ENET_IEEE_T_MACERR_COUNT_MASK)

/*! @name IEEE_T_CSERR - Frames Transmitted with Carrier Sense Error Statistic Register */
#define ENET_IEEE_T_CSERR_COUNT_MASK             (0xFFFFU)
#define ENET_IEEE_T_CSERR_COUNT_SHIFT            (0U)
#define ENET_IEEE_T_CSERR_COUNT(x)               (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_CSERR_COUNT_SHIFT)) & ENET_IEEE_T_CSERR_COUNT_MASK)

/*! @name IEEE_T_FDXFC - Flow Control Pause Frames Transmitted Statistic Register */
#define ENET_IEEE_T_FDXFC_COUNT_MASK             (0xFFFFU)
#define ENET_IEEE_T_FDXFC_COUNT_SHIFT            (0U)
#define ENET_IEEE_T_FDXFC_COUNT(x)               (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_FDXFC_COUNT_SHIFT)) & ENET_IEEE_T_FDXFC_COUNT_MASK)

/*! @name RMON_R_PACKETS - Rx Packet Count Statistic Register */
#define ENET_RMON_R_PACKETS_COUNT_MASK           (0xFFFFU)
#define ENET_RMON_R_PACKETS_COUNT_SHIFT          (0U)
#define ENET_RMON_R_PACKETS_COUNT(x)             (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_PACKETS_COUNT_SHIFT)) & ENET_RMON_R_PACKETS_COUNT_MASK)

/*! @name RMON_R_BC_PKT - Rx Broadcast Packets Statistic Register */
#define ENET_RMON_R_BC_PKT_COUNT_MASK            (0xFFFFU)
#define ENET_RMON_R_BC_PKT_COUNT_SHIFT           (0U)
#define ENET_RMON_R_BC_PKT_COUNT(x)              (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_BC_PKT_COUNT_SHIFT)) & ENET_RMON_R_BC_PKT_COUNT_MASK)

/*! @name RMON_R_MC_PKT - Rx Multicast Packets Statistic Register */
#define ENET_RMON_R_MC_PKT_COUNT_MASK            (0xFFFFU)
#define ENET_RMON_R_MC_PKT_COUNT_SHIFT           (0U)
#define ENET_RMON_R_MC_PKT_COUNT(x)              (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_MC_PKT_COUNT_SHIFT)) & ENET_RMON_R_MC_PKT_COUNT_MASK)

/*! @name RMON_R_CRC_ALIGN - Rx Packets with CRC/Align Error Statistic Register */
#define ENET_RMON_R_CRC_ALIGN_COUNT_MASK         (0xFFFFU)
#define ENET_RMON_R_CRC_ALIGN_COUNT_SHIFT        (0U)
#define ENET_RMON_R_CRC_ALIGN_COUNT(x)           (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_CRC_ALIGN_COUNT_SHIFT)) & ENET_RMON_R_CRC_ALIGN_COUNT_MASK)

/*! @name RMON_R_UNDERSIZE - Rx Packets with Less Than 64 Bytes and Good CRC Statistic Register */
#define ENET_RMON_R_UNDERSIZE_COUNT_MASK         (0xFFFFU)
#define ENET_RMON_R_UNDERSIZE_COUNT_SHIFT        (0U)
#define ENET_RMON_R_UNDERSIZE_COUNT(x)           (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_UNDERSIZE_COUNT_SHIFT)) & ENET_RMON_R_UNDERSIZE_COUNT_MASK)

/*! @name RMON_R_OVERSIZE - Rx Packets Greater Than MAX_FL and Good CRC Statistic Register */
#define ENET_RMON_R_OVERSIZE_COUNT_MASK          (0xFFFFU)
#define ENET_RMON_R_OVERSIZE_COUNT_SHIFT         (0U)
#define ENET_RMON_R_OVERSIZE_COUNT(x)            (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_OVERSIZE_COUNT_SHIFT)) & ENET_RMON_R_OVERSIZE_COUNT_MASK)

/*! @name RMON_R_FRAG - Rx Packets Less Than 64 Bytes and Bad CRC Statistic Register */
#define ENET_RMON_R_FRAG_COUNT_MASK              (0xFFFFU)
#define ENET_RMON_R_FRAG_COUNT_SHIFT             (0U)
#define ENET_RMON_R_FRAG_COUNT(x)                (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_FRAG_COUNT_SHIFT)) & ENET_RMON_R_FRAG_COUNT_MASK)

/*! @name RMON_R_JAB - Rx Packets Greater Than MAX_FL Bytes and Bad CRC Statistic Register */
#define ENET_RMON_R_JAB_COUNT_MASK               (0xFFFFU)
#define ENET_RMON_R_JAB_COUNT_SHIFT              (0U)
#define ENET_RMON_R_JAB_COUNT(x)                 (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_JAB_COUNT_SHIFT)) & ENET_RMON_R_JAB_COUNT_MASK)

/*! @name RMON_R_P64 - Rx 64-Byte Packets Statistic Register */
#define ENET_RMON_R_P64_COUNT_MASK               (0xFFFFU)
#define ENET_RMON_R_P64_COUNT_SHIFT              (0U)
#define ENET_RMON_R_P64_COUNT(x)                 (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_P64_COUNT_SHIFT)) & ENET_RMON_R_P64_COUNT_MASK)

/*! @name RMON_R_P65TO127 - Rx 65- to 127-Byte Packets Statistic Register */
#define ENET_RMON_R_P65TO127_COUNT_MASK          (0xFFFFU)
#define ENET_RMON_R_P65TO127_COUNT_SHIFT         (0U)
#define ENET_RMON_R_P65TO127_COUNT(x)            (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_P65TO127_COUNT_SHIFT)) & ENET_RMON_R_P65TO127_COUNT_MASK)

/*! @name RMON_R_P128TO255 - Rx 128- to 255-Byte Packets Statistic Register */
#define ENET_RMON_R_P128TO255_COUNT_MASK         (0xFFFFU)
#define ENET_RMON_R_P128TO255_COUNT_SHIFT        (0U)
#define ENET_RMON_R_P128TO255_COUNT(x)           (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_P128TO255_COUNT_SHIFT)) & ENET_RMON_R_P128TO255_COUNT_MASK)

/*! @name RMON_R_P256TO511 - Rx 256- to 511-Byte Packets Statistic Register */
#define ENET_RMON_R_P256TO511_COUNT_MASK         (0xFFFFU)
#define ENET_RMON_R_P256TO511_COUNT_SHIFT        (0U)
#define ENET_RMON_R_P256TO511_COUNT(x)           (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_P256TO511_COUNT_SHIFT)) & ENET_RMON_R_P256TO511_COUNT_MASK)

/*! @name RMON_R_P512TO1023 - Rx 512- to 1023-Byte Packets Statistic Register */
#define ENET_RMON_R_P512TO1023_COUNT_MASK        (0xFFFFU)
#define ENET_RMON_R_P512TO1023_COUNT_SHIFT       (0U)
#define ENET_RMON_R_P512TO1023_COUNT(x)          (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_P512TO1023_COUNT_SHIFT)) & ENET_RMON_R_P512TO1023_COUNT_MASK)

/*! @name RMON_R_P1024TO2047 - Rx 1024- to 2047-Byte Packets Statistic Register */
#define ENET_RMON_R_P1024TO2047_COUNT_MASK       (0xFFFFU)
#define ENET_RMON_R_P1024TO2047_COUNT_SHIFT      (0U)
#define ENET_RMON_R_P1024TO2047_COUNT(x)         (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_P1024TO2047_COUNT_SHIFT)) & ENET_RMON_R_P1024TO2047_COUNT_MASK)

/*! @name RMON_R_P_GTE2048 - Rx Packets Greater than 2048 Bytes Statistic Register */
#define ENET_RMON_R_P_GTE2048_COUNT_MASK         (0xFFFFU)
#define ENET_RMON_R_P_GTE2048_COUNT_SHIFT        (0U)
#define ENET_RMON_R_P_GTE2048_COUNT(x)           (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_P_GTE2048_COUNT_SHIFT)) & ENET_RMON_R_P_GTE2048_COUNT_MASK)

/*! @name IEEE_R_DROP - Frames not Counted Correctly Statistic Register */
#define ENET_IEEE_R_DROP_COUNT_MASK              (0xFFFFU)
#define ENET_IEEE_R_DROP_COUNT_SHIFT             (0U)
#define ENET_IEEE_R_DROP_COUNT(x)                (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_R_DROP_COUNT_SHIFT)) & ENET_IEEE_R_DROP_COUNT_MASK)

/*! @name IEEE_R_FRAME_OK - Frames Received OK Statistic Register */
#define ENET_IEEE_R_FRAME_OK_COUNT_MASK          (0xFFFFU)
#define ENET_IEEE_R_FRAME_OK_COUNT_SHIFT         (0U)
#define ENET_IEEE_R_FRAME_OK_COUNT(x)            (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_R_FRAME_OK_COUNT_SHIFT)) & ENET_IEEE_R_FRAME_OK_COUNT_MASK)

/*! @name IEEE_R_CRC - Frames Received with CRC Error Statistic Register */
#define ENET_IEEE_R_CRC_COUNT_MASK               (0xFFFFU)
#define ENET_IEEE_R_CRC_COUNT_SHIFT              (0U)
#define ENET_IEEE_R_CRC_COUNT(x)                 (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_R_CRC_COUNT_SHIFT)) & ENET_IEEE_R_CRC_COUNT_MASK)

/*! @name IEEE_R_ALIGN - Frames Received with Alignment Error Statistic Register */
#define ENET_IEEE_R_ALIGN_COUNT_MASK             (0xFFFFU)
#define ENET_IEEE_R_ALIGN_COUNT_SHIFT            (0U)
#define ENET_IEEE_R_ALIGN_COUNT(x)               (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_R_ALIGN_COUNT_SHIFT)) & ENET_IEEE_R_ALIGN_COUNT_MASK)

/*! @name IEEE_R_MACERR - Receive FIFO Overflow Count Statistic Register */
#define ENET_IEEE_R_MACERR_COUNT_MASK            (0xFFFFU)
#define ENET_IEEE_R_MACERR_COUNT_SHIFT           (0U)
#define ENET_IEEE_R_MACERR_COUNT(x)              (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_R_MACERR_COUNT_SHIFT)) & ENET_IEEE_R_MACERR_COUNT_MASK)

/*! @name IEEE_R_FDXFC - Flow Control Pause Frames Received Statistic Register */
#define ENET_IEEE_R_FDXFC_COUNT_MASK             (0xFFFFU)
#define ENET_IEEE_R_FDXFC_COUNT_SHIFT            (0U)
#define ENET_IEEE_R_FDXFC_COUNT(x)               (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_R_FDXFC_COUNT_SHIFT)) & ENET_IEEE_R_FDXFC_COUNT_MASK)

/*! @name ATCR - Adjustable Timer Control Register */
#define ENET_ATCR_EN                             (0x1U)
#define ENET_ATCR_OFFEN                          (0x4U)
#define ENET_ATCR_OFFRST                         (0x8U)
#define ENET_ATCR_PEREN                          (0x10U)
#define ENET_ATCR_PINPER                         (0x80U)
#define ENET_ATCR_RESTART                        (0x200U)
#define ENET_ATCR_CAPTURE                        (0x800U)
#define ENET_ATCR_SLAVE                          (0x2000U)

/*! @name ATCOR - Timer Correction Register */
#define ENET_ATCOR_COR_MASK                      (0x7FFFFFFFU)
#define ENET_ATCOR_COR_SHIFT                     (0U)
#define ENET_ATCOR_COR(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_ATCOR_COR_SHIFT)) & ENET_ATCOR_COR_MASK)

/*! @name ATINC - Time-Stamping Clock Period Register */
#define ENET_ATINC_INC_MASK                      (0x7FU)
#define ENET_ATINC_INC_SHIFT                     (0U)
#define ENET_ATINC_INC(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_ATINC_INC_SHIFT)) & ENET_ATINC_INC_MASK)
#define ENET_ATINC_INC_CORR_MASK                 (0x7F00U)
#define ENET_ATINC_INC_CORR_SHIFT                (8U)
#define ENET_ATINC_INC_CORR(x)                   (((uint32_t)(((uint32_t)(x)) << ENET_ATINC_INC_CORR_SHIFT)) & ENET_ATINC_INC_CORR_MASK)

/*! @name TGSR - Timer Global Status Register */
#define ENET_TGSR_TF0                            (0x1U)
#define ENET_TGSR_TF1                            (0x2U)
#define ENET_TGSR_TF2                            (0x4U)
#define ENET_TGSR_TF3                            (0x8U)

/*! @name TCSR - Timer Control Status Register */
#define ENET_TCSR_TDRE                           (0x1U)
#define ENET_TCSR_TMODE_MASK                     (0x3CU)
#define ENET_TCSR_TMODE_SHIFT                    (2U)
#define ENET_TCSR_TMODE(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_TCSR_TMODE_SHIFT)) & ENET_TCSR_TMODE_MASK)
#define ENET_TCSR_TIE                            (0x40U)
#define ENET_TCSR_TF                             (0x80U)

/* The count of ENET_TCSR */
#define ENET_TCSR_COUNT                          (4U)

/* The count of ENET_TCCR */
#define ENET_TCCR_COUNT                          (4U)


/*!
 * @}
 */ /* end of group ENET_Register_Masks */


/* ENET - Peripheral instance base addresses */
/** Peripheral ENET base address */
#define ENET_BASE                                (0x400C0000u)
/** Peripheral ENET base pointer */
#define ENET                                     ((ENET_TypeDef *)ENET_BASE)
/** Array initializer of ENET peripheral base addresses */
#define ENET_BASE_ADDRS                          { ENET_BASE }
/** Array initializer of ENET peripheral base pointers */
#define ENET_BASE_PTRS                           { ENET }
/** Interrupt vectors for the ENET peripheral type */
#define ENET_Transmit_IRQS                       { ENET_Transmit_IRQn }
#define ENET_Receive_IRQS                        { ENET_Receive_IRQn }
#define ENET_Error_IRQS                          { ENET_Error_IRQn }
#define ENET_1588_Timer_IRQS                     { ENET_1588_Timer_IRQn }

/*!
 * @}
 */ /* end of group ENET_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- EWM Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup EWM_Peripheral_Access_Layer EWM Peripheral Access Layer
 * @{
 */

/** EWM - Register Layout Typedef */
typedef struct {
  __IO uint8_t CTRL;                               /**< Control Register, offset: 0x0 */
  __O  uint8_t SERV;                               /**< Service Register, offset: 0x1 */
  __IO uint8_t CMPL;                               /**< Compare Low Register, offset: 0x2 */
  __IO uint8_t CMPH;                               /**< Compare High Register, offset: 0x3 */
} EWM_TypeDef;

/* ----------------------------------------------------------------------------
   -- EWM Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup EWM_Register_Masks EWM Register Masks
 * @{
 */

/*! @name CTRL - Control Register */
#define EWM_CTRL_EWMEN                           (0x1U)
#define EWM_CTRL_ASSIN                           (0x2U)
#define EWM_CTRL_INEN                            (0x4U)
#define EWM_CTRL_INTEN                           (0x8U)

/*! @name SERV - Service Register */
#define EWM_SERV_SERVICE_MASK                    (0xFFU)
#define EWM_SERV_SERVICE_SHIFT                   (0U)
#define EWM_SERV_SERVICE(x)                      (((uint8_t)(((uint8_t)(x)) << EWM_SERV_SERVICE_SHIFT)) & EWM_SERV_SERVICE_MASK)

/*! @name CMPL - Compare Low Register */
#define EWM_CMPL_COMPAREL_MASK                   (0xFFU)
#define EWM_CMPL_COMPAREL_SHIFT                  (0U)
#define EWM_CMPL_COMPAREL(x)                     (((uint8_t)(((uint8_t)(x)) << EWM_CMPL_COMPAREL_SHIFT)) & EWM_CMPL_COMPAREL_MASK)

/*! @name CMPH - Compare High Register */
#define EWM_CMPH_COMPAREH_MASK                   (0xFFU)
#define EWM_CMPH_COMPAREH_SHIFT                  (0U)
#define EWM_CMPH_COMPAREH(x)                     (((uint8_t)(((uint8_t)(x)) << EWM_CMPH_COMPAREH_SHIFT)) & EWM_CMPH_COMPAREH_MASK)


/*!
 * @}
 */ /* end of group EWM_Register_Masks */


/* EWM - Peripheral instance base addresses */
/** Peripheral EWM base address */
#define EWM_BASE                                 (0x40061000u)
/** Peripheral EWM base pointer */
#define EWM                                      ((EWM_TypeDef *)EWM_BASE)
/** Array initializer of EWM peripheral base addresses */
#define EWM_BASE_ADDRS                           { EWM_BASE }
/** Array initializer of EWM peripheral base pointers */
#define EWM_BASE_PTRS                            { EWM }
/** Interrupt vectors for the EWM peripheral type */
#define EWM_IRQS                                 { WDOG_EWM_IRQn }

/*!
 * @}
 */ /* end of group EWM_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- FB Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup FB_Peripheral_Access_Layer FB Peripheral Access Layer
 * @{
 */

/** FB - Register Layout Typedef */
typedef struct {
  struct {                                         /* offset: 0x0, array step: 0xC */
    __IO uint32_t CSAR;                              /**< Chip Select Address Register, array offset: 0x0, array step: 0xC */
    __IO uint32_t CSMR;                              /**< Chip Select Mask Register, array offset: 0x4, array step: 0xC */
    __IO uint32_t CSCR;                              /**< Chip Select Control Register, array offset: 0x8, array step: 0xC */
  } CS[6];
       uint8_t RESERVED_0[24];
  __IO uint32_t CSPMCR;                            /**< Chip Select port Multiplexing Control Register, offset: 0x60 */
} FB_TypeDef;

/* ----------------------------------------------------------------------------
   -- FB Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup FB_Register_Masks FB Register Masks
 * @{
 */

/*! @name CSAR - Chip Select Address Register */
#define FB_CSAR_BA_MASK                          (0xFFFF0000U)
#define FB_CSAR_BA_SHIFT                         (16U)
#define FB_CSAR_BA(x)                            (((uint32_t)(((uint32_t)(x)) << FB_CSAR_BA_SHIFT)) & FB_CSAR_BA_MASK)

/* The count of FB_CSAR */
#define FB_CSAR_COUNT                            (6U)

/*! @name CSMR - Chip Select Mask Register */
#define FB_CSMR_V                                (0x1U)
#define FB_CSMR_WP                               (0x100U)
#define FB_CSMR_BAM_MASK                         (0xFFFF0000U)
#define FB_CSMR_BAM_SHIFT                        (16U)
#define FB_CSMR_BAM(x)                           (((uint32_t)(((uint32_t)(x)) << FB_CSMR_BAM_SHIFT)) & FB_CSMR_BAM_MASK)

/* The count of FB_CSMR */
#define FB_CSMR_COUNT                            (6U)

/*! @name CSCR - Chip Select Control Register */
#define FB_CSCR_BSTW                             (0x8U)
#define FB_CSCR_BSTR                             (0x10U)
#define FB_CSCR_BEM                              (0x20U)
#define FB_CSCR_PS_MASK                          (0xC0U)
#define FB_CSCR_PS_SHIFT                         (6U)
#define FB_CSCR_PS(x)                            (((uint32_t)(((uint32_t)(x)) << FB_CSCR_PS_SHIFT)) & FB_CSCR_PS_MASK)
#define FB_CSCR_AA                               (0x100U)
#define FB_CSCR_BLS                              (0x200U)
#define FB_CSCR_WS_MASK                          (0xFC00U)
#define FB_CSCR_WS_SHIFT                         (10U)
#define FB_CSCR_WS(x)                            (((uint32_t)(((uint32_t)(x)) << FB_CSCR_WS_SHIFT)) & FB_CSCR_WS_MASK)
#define FB_CSCR_WRAH_MASK                        (0x30000U)
#define FB_CSCR_WRAH_SHIFT                       (16U)
#define FB_CSCR_WRAH(x)                          (((uint32_t)(((uint32_t)(x)) << FB_CSCR_WRAH_SHIFT)) & FB_CSCR_WRAH_MASK)
#define FB_CSCR_RDAH_MASK                        (0xC0000U)
#define FB_CSCR_RDAH_SHIFT                       (18U)
#define FB_CSCR_RDAH(x)                          (((uint32_t)(((uint32_t)(x)) << FB_CSCR_RDAH_SHIFT)) & FB_CSCR_RDAH_MASK)
#define FB_CSCR_ASET_MASK                        (0x300000U)
#define FB_CSCR_ASET_SHIFT                       (20U)
#define FB_CSCR_ASET(x)                          (((uint32_t)(((uint32_t)(x)) << FB_CSCR_ASET_SHIFT)) & FB_CSCR_ASET_MASK)
#define FB_CSCR_EXTS                             (0x400000U)
#define FB_CSCR_SWSEN                            (0x800000U)
#define FB_CSCR_SWS_MASK                         (0xFC000000U)
#define FB_CSCR_SWS_SHIFT                        (26U)
#define FB_CSCR_SWS(x)                           (((uint32_t)(((uint32_t)(x)) << FB_CSCR_SWS_SHIFT)) & FB_CSCR_SWS_MASK)

/* The count of FB_CSCR */
#define FB_CSCR_COUNT                            (6U)

/*! @name CSPMCR - Chip Select port Multiplexing Control Register */
#define FB_CSPMCR_GROUP5_MASK                    (0xF000U)
#define FB_CSPMCR_GROUP5_SHIFT                   (12U)
#define FB_CSPMCR_GROUP5(x)                      (((uint32_t)(((uint32_t)(x)) << FB_CSPMCR_GROUP5_SHIFT)) & FB_CSPMCR_GROUP5_MASK)
#define FB_CSPMCR_GROUP4_MASK                    (0xF0000U)
#define FB_CSPMCR_GROUP4_SHIFT                   (16U)
#define FB_CSPMCR_GROUP4(x)                      (((uint32_t)(((uint32_t)(x)) << FB_CSPMCR_GROUP4_SHIFT)) & FB_CSPMCR_GROUP4_MASK)
#define FB_CSPMCR_GROUP3_MASK                    (0xF00000U)
#define FB_CSPMCR_GROUP3_SHIFT                   (20U)
#define FB_CSPMCR_GROUP3(x)                      (((uint32_t)(((uint32_t)(x)) << FB_CSPMCR_GROUP3_SHIFT)) & FB_CSPMCR_GROUP3_MASK)
#define FB_CSPMCR_GROUP2_MASK                    (0xF000000U)
#define FB_CSPMCR_GROUP2_SHIFT                   (24U)
#define FB_CSPMCR_GROUP2(x)                      (((uint32_t)(((uint32_t)(x)) << FB_CSPMCR_GROUP2_SHIFT)) & FB_CSPMCR_GROUP2_MASK)
#define FB_CSPMCR_GROUP1_MASK                    (0xF0000000U)
#define FB_CSPMCR_GROUP1_SHIFT                   (28U)
#define FB_CSPMCR_GROUP1(x)                      (((uint32_t)(((uint32_t)(x)) << FB_CSPMCR_GROUP1_SHIFT)) & FB_CSPMCR_GROUP1_MASK)


/*!
 * @}
 */ /* end of group FB_Register_Masks */


/* FB - Peripheral instance base addresses */
/** Peripheral FB base address */
#define FB_BASE                                  (0x4000C000u)
/** Peripheral FB base pointer */
#define FB                                       ((FB_TypeDef *)FB_BASE)
/** Array initializer of FB peripheral base addresses */
#define FB_BASE_ADDRS                            { FB_BASE }
/** Array initializer of FB peripheral base pointers */
#define FB_BASE_PTRS                             { FB }

/*!
 * @}
 */ /* end of group FB_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- FMC Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup FMC_Peripheral_Access_Layer FMC Peripheral Access Layer
 * @{
 */

/** FMC - Register Layout Typedef */
typedef struct {
  __IO uint32_t PFAPR;                             /**< Flash Access Protection Register, offset: 0x0 */
  __IO uint32_t PFB0CR;                            /**< Flash Bank 0 Control Register, offset: 0x4 */
  __IO uint32_t PFB1CR;                            /**< Flash Bank 1 Control Register, offset: 0x8 */
       uint8_t RESERVED_0[244];
  __IO uint32_t TAGVDW0S[4];                       /**< Cache Tag Storage, array offset: 0x100, array step: 0x4 */
  __IO uint32_t TAGVDW1S[4];                       /**< Cache Tag Storage, array offset: 0x110, array step: 0x4 */
  __IO uint32_t TAGVDW2S[4];                       /**< Cache Tag Storage, array offset: 0x120, array step: 0x4 */
  __IO uint32_t TAGVDW3S[4];                       /**< Cache Tag Storage, array offset: 0x130, array step: 0x4 */
       uint8_t RESERVED_1[192];
  struct {                                         /* offset: 0x200, array step: index*0x20, index2*0x8 */
    __IO uint32_t DATA_U;                            /**< Cache Data Storage (upper word), array offset: 0x200, array step: index*0x20, index2*0x8 */
    __IO uint32_t DATA_L;                            /**< Cache Data Storage (lower word), array offset: 0x204, array step: index*0x20, index2*0x8 */
  } SET[4][4];
} FMC_TypeDef;

/* ----------------------------------------------------------------------------
   -- FMC Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup FMC_Register_Masks FMC Register Masks
 * @{
 */

/*! @name PFAPR - Flash Access Protection Register */
#define FMC_PFAPR_M0AP_MASK                      (0x3U)
#define FMC_PFAPR_M0AP_SHIFT                     (0U)
#define FMC_PFAPR_M0AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M0AP_SHIFT)) & FMC_PFAPR_M0AP_MASK)
#define FMC_PFAPR_M1AP_MASK                      (0xCU)
#define FMC_PFAPR_M1AP_SHIFT                     (2U)
#define FMC_PFAPR_M1AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M1AP_SHIFT)) & FMC_PFAPR_M1AP_MASK)
#define FMC_PFAPR_M2AP_MASK                      (0x30U)
#define FMC_PFAPR_M2AP_SHIFT                     (4U)
#define FMC_PFAPR_M2AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M2AP_SHIFT)) & FMC_PFAPR_M2AP_MASK)
#define FMC_PFAPR_M3AP_MASK                      (0xC0U)
#define FMC_PFAPR_M3AP_SHIFT                     (6U)
#define FMC_PFAPR_M3AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M3AP_SHIFT)) & FMC_PFAPR_M3AP_MASK)
#define FMC_PFAPR_M4AP_MASK                      (0x300U)
#define FMC_PFAPR_M4AP_SHIFT                     (8U)
#define FMC_PFAPR_M4AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M4AP_SHIFT)) & FMC_PFAPR_M4AP_MASK)
#define FMC_PFAPR_M5AP_MASK                      (0xC00U)
#define FMC_PFAPR_M5AP_SHIFT                     (10U)
#define FMC_PFAPR_M5AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M5AP_SHIFT)) & FMC_PFAPR_M5AP_MASK)
#define FMC_PFAPR_M6AP_MASK                      (0x3000U)
#define FMC_PFAPR_M6AP_SHIFT                     (12U)
#define FMC_PFAPR_M6AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M6AP_SHIFT)) & FMC_PFAPR_M6AP_MASK)
#define FMC_PFAPR_M7AP_MASK                      (0xC000U)
#define FMC_PFAPR_M7AP_SHIFT                     (14U)
#define FMC_PFAPR_M7AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M7AP_SHIFT)) & FMC_PFAPR_M7AP_MASK)
#define FMC_PFAPR_M0PFD                          (0x10000U)
#define FMC_PFAPR_M1PFD                          (0x20000U)
#define FMC_PFAPR_M2PFD                          (0x40000U)
#define FMC_PFAPR_M3PFD                          (0x80000U)
#define FMC_PFAPR_M4PFD                          (0x100000U)
#define FMC_PFAPR_M5PFD                          (0x200000U)
#define FMC_PFAPR_M6PFD                          (0x400000U)
#define FMC_PFAPR_M7PFD                          (0x800000U)

/*! @name PFB0CR - Flash Bank 0 Control Register */
#define FMC_PFB0CR_B0SEBE                        (0x1U)
#define FMC_PFB0CR_B0IPE                         (0x2U)
#define FMC_PFB0CR_B0DPE                         (0x4U)
#define FMC_PFB0CR_B0ICE                         (0x8U)
#define FMC_PFB0CR_B0DCE                         (0x10U)
#define FMC_PFB0CR_CRC_MASK                      (0xE0U)
#define FMC_PFB0CR_CRC_SHIFT                     (5U)
#define FMC_PFB0CR_CRC(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_CRC_SHIFT)) & FMC_PFB0CR_CRC_MASK)
#define FMC_PFB0CR_B0MW_MASK                     (0x60000U)
#define FMC_PFB0CR_B0MW_SHIFT                    (17U)
#define FMC_PFB0CR_B0MW(x)                       (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_B0MW_SHIFT)) & FMC_PFB0CR_B0MW_MASK)
#define FMC_PFB0CR_S_B_INV                       (0x80000U)
#define FMC_PFB0CR_CINV_WAY_MASK                 (0xF00000U)
#define FMC_PFB0CR_CINV_WAY_SHIFT                (20U)
#define FMC_PFB0CR_CINV_WAY(x)                   (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_CINV_WAY_SHIFT)) & FMC_PFB0CR_CINV_WAY_MASK)
#define FMC_PFB0CR_CLCK_WAY_MASK                 (0xF000000U)
#define FMC_PFB0CR_CLCK_WAY_SHIFT                (24U)
#define FMC_PFB0CR_CLCK_WAY(x)                   (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_CLCK_WAY_SHIFT)) & FMC_PFB0CR_CLCK_WAY_MASK)
#define FMC_PFB0CR_B0RWSC_MASK                   (0xF0000000U)
#define FMC_PFB0CR_B0RWSC_SHIFT                  (28U)
#define FMC_PFB0CR_B0RWSC(x)                     (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_B0RWSC_SHIFT)) & FMC_PFB0CR_B0RWSC_MASK)

/*! @name PFB1CR - Flash Bank 1 Control Register */
#define FMC_PFB1CR_B1SEBE                        (0x1U)
#define FMC_PFB1CR_B1IPE                         (0x2U)
#define FMC_PFB1CR_B1DPE                         (0x4U)
#define FMC_PFB1CR_B1ICE                         (0x8U)
#define FMC_PFB1CR_B1DCE                         (0x10U)
#define FMC_PFB1CR_B1MW_MASK                     (0x60000U)
#define FMC_PFB1CR_B1MW_SHIFT                    (17U)
#define FMC_PFB1CR_B1MW(x)                       (((uint32_t)(((uint32_t)(x)) << FMC_PFB1CR_B1MW_SHIFT)) & FMC_PFB1CR_B1MW_MASK)
#define FMC_PFB1CR_B1RWSC_MASK                   (0xF0000000U)
#define FMC_PFB1CR_B1RWSC_SHIFT                  (28U)
#define FMC_PFB1CR_B1RWSC(x)                     (((uint32_t)(((uint32_t)(x)) << FMC_PFB1CR_B1RWSC_SHIFT)) & FMC_PFB1CR_B1RWSC_MASK)

/*! @name TAGVDW0S - Cache Tag Storage */
#define FMC_TAGVDW0S_valid                       (0x1U)
#define FMC_TAGVDW0S_tag_MASK                    (0x7FFE0U)
#define FMC_TAGVDW0S_tag_SHIFT                   (5U)
#define FMC_TAGVDW0S_tag(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_TAGVDW0S_tag_SHIFT)) & FMC_TAGVDW0S_tag_MASK)

/* The count of FMC_TAGVDW0S */
#define FMC_TAGVDW0S_COUNT                       (4U)

/*! @name TAGVDW1S - Cache Tag Storage */
#define FMC_TAGVDW1S_valid                       (0x1U)
#define FMC_TAGVDW1S_tag_MASK                    (0x7FFE0U)
#define FMC_TAGVDW1S_tag_SHIFT                   (5U)
#define FMC_TAGVDW1S_tag(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_TAGVDW1S_tag_SHIFT)) & FMC_TAGVDW1S_tag_MASK)

/* The count of FMC_TAGVDW1S */
#define FMC_TAGVDW1S_COUNT                       (4U)

/*! @name TAGVDW2S - Cache Tag Storage */
#define FMC_TAGVDW2S_valid                       (0x1U)
#define FMC_TAGVDW2S_tag_MASK                    (0x7FFE0U)
#define FMC_TAGVDW2S_tag_SHIFT                   (5U)
#define FMC_TAGVDW2S_tag(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_TAGVDW2S_tag_SHIFT)) & FMC_TAGVDW2S_tag_MASK)

/* The count of FMC_TAGVDW2S */
#define FMC_TAGVDW2S_COUNT                       (4U)

/*! @name TAGVDW3S - Cache Tag Storage */
#define FMC_TAGVDW3S_valid                       (0x1U)
#define FMC_TAGVDW3S_tag_MASK                    (0x7FFE0U)
#define FMC_TAGVDW3S_tag_SHIFT                   (5U)
#define FMC_TAGVDW3S_tag(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_TAGVDW3S_tag_SHIFT)) & FMC_TAGVDW3S_tag_MASK)

/* The count of FMC_TAGVDW3S */
#define FMC_TAGVDW3S_COUNT                       (4U)

/* The count of FMC_DATA_U */
#define FMC_DATA_U_COUNT                         (4U)

/* The count of FMC_DATA_U */
#define FMC_DATA_U_COUNT2                        (4U)

/* The count of FMC_DATA_L */
#define FMC_DATA_L_COUNT                         (4U)

/* The count of FMC_DATA_L */
#define FMC_DATA_L_COUNT2                        (4U)


/*!
 * @}
 */ /* end of group FMC_Register_Masks */


/* FMC - Peripheral instance base addresses */
/** Peripheral FMC base address */
#define FMC_BASE                                 (0x4001F000u)
/** Peripheral FMC base pointer */
#define FMC                                      ((FMC_TypeDef *)FMC_BASE)
/** Array initializer of FMC peripheral base addresses */
#define FMC_BASE_ADDRS                           { FMC_BASE }
/** Array initializer of FMC peripheral base pointers */
#define FMC_BASE_PTRS                            { FMC }

/*!
 * @}
 */ /* end of group FMC_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- FTFE Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup FTFE_Peripheral_Access_Layer FTFE Peripheral Access Layer
 * @{
 */

/** FTFE - Register Layout Typedef */
typedef struct {
  __IO uint8_t FSTAT;                              /**< Flash Status Register, offset: 0x0 */
  __IO uint8_t FCNFG;                              /**< Flash Configuration Register, offset: 0x1 */
  __I  uint8_t FSEC;                               /**< Flash Security Register, offset: 0x2 */
  __I  uint8_t FOPT;                               /**< Flash Option Register, offset: 0x3 */
  __IO uint8_t FCCOB3;                             /**< Flash Common Command Object Registers, offset: 0x4 */
  __IO uint8_t FCCOB2;                             /**< Flash Common Command Object Registers, offset: 0x5 */
  __IO uint8_t FCCOB1;                             /**< Flash Common Command Object Registers, offset: 0x6 */
  __IO uint8_t FCCOB0;                             /**< Flash Common Command Object Registers, offset: 0x7 */
  __IO uint8_t FCCOB7;                             /**< Flash Common Command Object Registers, offset: 0x8 */
  __IO uint8_t FCCOB6;                             /**< Flash Common Command Object Registers, offset: 0x9 */
  __IO uint8_t FCCOB5;                             /**< Flash Common Command Object Registers, offset: 0xA */
  __IO uint8_t FCCOB4;                             /**< Flash Common Command Object Registers, offset: 0xB */
  __IO uint8_t FCCOBB;                             /**< Flash Common Command Object Registers, offset: 0xC */
  __IO uint8_t FCCOBA;                             /**< Flash Common Command Object Registers, offset: 0xD */
  __IO uint8_t FCCOB9;                             /**< Flash Common Command Object Registers, offset: 0xE */
  __IO uint8_t FCCOB8;                             /**< Flash Common Command Object Registers, offset: 0xF */
  __IO uint8_t FPROT3;                             /**< Program Flash Protection Registers, offset: 0x10 */
  __IO uint8_t FPROT2;                             /**< Program Flash Protection Registers, offset: 0x11 */
  __IO uint8_t FPROT1;                             /**< Program Flash Protection Registers, offset: 0x12 */
  __IO uint8_t FPROT0;                             /**< Program Flash Protection Registers, offset: 0x13 */
       uint8_t RESERVED_0[2];
  __IO uint8_t FEPROT;                             /**< EEPROM Protection Register, offset: 0x16 */
  __IO uint8_t FDPROT;                             /**< Data Flash Protection Register, offset: 0x17 */
} FTFE_TypeDef;

/* ----------------------------------------------------------------------------
   -- FTFE Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup FTFE_Register_Masks FTFE Register Masks
 * @{
 */

/*! @name FSTAT - Flash Status Register */
#define FTFE_FSTAT_MGSTAT0                       (0x1U)
#define FTFE_FSTAT_FPVIOL                        (0x10U)
#define FTFE_FSTAT_ACCERR                        (0x20U)
#define FTFE_FSTAT_RDCOLERR                      (0x40U)
#define FTFE_FSTAT_CCIF                          (0x80U)

/*! @name FCNFG - Flash Configuration Register */
#define FTFE_FCNFG_EEERDY                        (0x1U)
#define FTFE_FCNFG_RAMRDY                        (0x2U)
#define FTFE_FCNFG_PFLSH                         (0x4U)
#define FTFE_FCNFG_SWAP                          (0x8U)
#define FTFE_FCNFG_ERSSUSP                       (0x10U)
#define FTFE_FCNFG_ERSAREQ                       (0x20U)
#define FTFE_FCNFG_RDCOLLIE                      (0x40U)
#define FTFE_FCNFG_CCIE                          (0x80U)

/*! @name FSEC - Flash Security Register */
#define FTFE_FSEC_SEC_MASK                       (0x3U)
#define FTFE_FSEC_SEC_SHIFT                      (0U)
#define FTFE_FSEC_SEC(x)                         (((uint8_t)(((uint8_t)(x)) << FTFE_FSEC_SEC_SHIFT)) & FTFE_FSEC_SEC_MASK)
#define FTFE_FSEC_FSLACC_MASK                    (0xCU)
#define FTFE_FSEC_FSLACC_SHIFT                   (2U)
#define FTFE_FSEC_FSLACC(x)                      (((uint8_t)(((uint8_t)(x)) << FTFE_FSEC_FSLACC_SHIFT)) & FTFE_FSEC_FSLACC_MASK)
#define FTFE_FSEC_MEEN_MASK                      (0x30U)
#define FTFE_FSEC_MEEN_SHIFT                     (4U)
#define FTFE_FSEC_MEEN(x)                        (((uint8_t)(((uint8_t)(x)) << FTFE_FSEC_MEEN_SHIFT)) & FTFE_FSEC_MEEN_MASK)
#define FTFE_FSEC_KEYEN_MASK                     (0xC0U)
#define FTFE_FSEC_KEYEN_SHIFT                    (6U)
#define FTFE_FSEC_KEYEN(x)                       (((uint8_t)(((uint8_t)(x)) << FTFE_FSEC_KEYEN_SHIFT)) & FTFE_FSEC_KEYEN_MASK)

/*! @name FOPT - Flash Option Register */
#define FTFE_FOPT_OPT_MASK                       (0xFFU)
#define FTFE_FOPT_OPT_SHIFT                      (0U)
#define FTFE_FOPT_OPT(x)                         (((uint8_t)(((uint8_t)(x)) << FTFE_FOPT_OPT_SHIFT)) & FTFE_FOPT_OPT_MASK)

/*! @name FCCOB3 - Flash Common Command Object Registers */
#define FTFE_FCCOB3_CCOBn_MASK                   (0xFFU)
#define FTFE_FCCOB3_CCOBn_SHIFT                  (0U)
#define FTFE_FCCOB3_CCOBn(x)                     (((uint8_t)(((uint8_t)(x)) << FTFE_FCCOB3_CCOBn_SHIFT)) & FTFE_FCCOB3_CCOBn_MASK)

/*! @name FCCOB2 - Flash Common Command Object Registers */
#define FTFE_FCCOB2_CCOBn_MASK                   (0xFFU)
#define FTFE_FCCOB2_CCOBn_SHIFT                  (0U)
#define FTFE_FCCOB2_CCOBn(x)                     (((uint8_t)(((uint8_t)(x)) << FTFE_FCCOB2_CCOBn_SHIFT)) & FTFE_FCCOB2_CCOBn_MASK)

/*! @name FCCOB1 - Flash Common Command Object Registers */
#define FTFE_FCCOB1_CCOBn_MASK                   (0xFFU)
#define FTFE_FCCOB1_CCOBn_SHIFT                  (0U)
#define FTFE_FCCOB1_CCOBn(x)                     (((uint8_t)(((uint8_t)(x)) << FTFE_FCCOB1_CCOBn_SHIFT)) & FTFE_FCCOB1_CCOBn_MASK)

/*! @name FCCOB0 - Flash Common Command Object Registers */
#define FTFE_FCCOB0_CCOBn_MASK                   (0xFFU)
#define FTFE_FCCOB0_CCOBn_SHIFT                  (0U)
#define FTFE_FCCOB0_CCOBn(x)                     (((uint8_t)(((uint8_t)(x)) << FTFE_FCCOB0_CCOBn_SHIFT)) & FTFE_FCCOB0_CCOBn_MASK)

/*! @name FCCOB7 - Flash Common Command Object Registers */
#define FTFE_FCCOB7_CCOBn_MASK                   (0xFFU)
#define FTFE_FCCOB7_CCOBn_SHIFT                  (0U)
#define FTFE_FCCOB7_CCOBn(x)                     (((uint8_t)(((uint8_t)(x)) << FTFE_FCCOB7_CCOBn_SHIFT)) & FTFE_FCCOB7_CCOBn_MASK)

/*! @name FCCOB6 - Flash Common Command Object Registers */
#define FTFE_FCCOB6_CCOBn_MASK                   (0xFFU)
#define FTFE_FCCOB6_CCOBn_SHIFT                  (0U)
#define FTFE_FCCOB6_CCOBn(x)                     (((uint8_t)(((uint8_t)(x)) << FTFE_FCCOB6_CCOBn_SHIFT)) & FTFE_FCCOB6_CCOBn_MASK)

/*! @name FCCOB5 - Flash Common Command Object Registers */
#define FTFE_FCCOB5_CCOBn_MASK                   (0xFFU)
#define FTFE_FCCOB5_CCOBn_SHIFT                  (0U)
#define FTFE_FCCOB5_CCOBn(x)                     (((uint8_t)(((uint8_t)(x)) << FTFE_FCCOB5_CCOBn_SHIFT)) & FTFE_FCCOB5_CCOBn_MASK)

/*! @name FCCOB4 - Flash Common Command Object Registers */
#define FTFE_FCCOB4_CCOBn_MASK                   (0xFFU)
#define FTFE_FCCOB4_CCOBn_SHIFT                  (0U)
#define FTFE_FCCOB4_CCOBn(x)                     (((uint8_t)(((uint8_t)(x)) << FTFE_FCCOB4_CCOBn_SHIFT)) & FTFE_FCCOB4_CCOBn_MASK)

/*! @name FCCOBB - Flash Common Command Object Registers */
#define FTFE_FCCOBB_CCOBn_MASK                   (0xFFU)
#define FTFE_FCCOBB_CCOBn_SHIFT                  (0U)
#define FTFE_FCCOBB_CCOBn(x)                     (((uint8_t)(((uint8_t)(x)) << FTFE_FCCOBB_CCOBn_SHIFT)) & FTFE_FCCOBB_CCOBn_MASK)

/*! @name FCCOBA - Flash Common Command Object Registers */
#define FTFE_FCCOBA_CCOBn_MASK                   (0xFFU)
#define FTFE_FCCOBA_CCOBn_SHIFT                  (0U)
#define FTFE_FCCOBA_CCOBn(x)                     (((uint8_t)(((uint8_t)(x)) << FTFE_FCCOBA_CCOBn_SHIFT)) & FTFE_FCCOBA_CCOBn_MASK)

/*! @name FCCOB9 - Flash Common Command Object Registers */
#define FTFE_FCCOB9_CCOBn_MASK                   (0xFFU)
#define FTFE_FCCOB9_CCOBn_SHIFT                  (0U)
#define FTFE_FCCOB9_CCOBn(x)                     (((uint8_t)(((uint8_t)(x)) << FTFE_FCCOB9_CCOBn_SHIFT)) & FTFE_FCCOB9_CCOBn_MASK)

/*! @name FCCOB8 - Flash Common Command Object Registers */
#define FTFE_FCCOB8_CCOBn_MASK                   (0xFFU)
#define FTFE_FCCOB8_CCOBn_SHIFT                  (0U)
#define FTFE_FCCOB8_CCOBn(x)                     (((uint8_t)(((uint8_t)(x)) << FTFE_FCCOB8_CCOBn_SHIFT)) & FTFE_FCCOB8_CCOBn_MASK)

/*! @name FPROT3 - Program Flash Protection Registers */
#define FTFE_FPROT3_PROT_MASK                    (0xFFU)
#define FTFE_FPROT3_PROT_SHIFT                   (0U)
#define FTFE_FPROT3_PROT(x)                      (((uint8_t)(((uint8_t)(x)) << FTFE_FPROT3_PROT_SHIFT)) & FTFE_FPROT3_PROT_MASK)

/*! @name FPROT2 - Program Flash Protection Registers */
#define FTFE_FPROT2_PROT_MASK                    (0xFFU)
#define FTFE_FPROT2_PROT_SHIFT                   (0U)
#define FTFE_FPROT2_PROT(x)                      (((uint8_t)(((uint8_t)(x)) << FTFE_FPROT2_PROT_SHIFT)) & FTFE_FPROT2_PROT_MASK)

/*! @name FPROT1 - Program Flash Protection Registers */
#define FTFE_FPROT1_PROT_MASK                    (0xFFU)
#define FTFE_FPROT1_PROT_SHIFT                   (0U)
#define FTFE_FPROT1_PROT(x)                      (((uint8_t)(((uint8_t)(x)) << FTFE_FPROT1_PROT_SHIFT)) & FTFE_FPROT1_PROT_MASK)

/*! @name FPROT0 - Program Flash Protection Registers */
#define FTFE_FPROT0_PROT_MASK                    (0xFFU)
#define FTFE_FPROT0_PROT_SHIFT                   (0U)
#define FTFE_FPROT0_PROT(x)                      (((uint8_t)(((uint8_t)(x)) << FTFE_FPROT0_PROT_SHIFT)) & FTFE_FPROT0_PROT_MASK)

/*! @name FEPROT - EEPROM Protection Register */
#define FTFE_FEPROT_EPROT_MASK                   (0xFFU)
#define FTFE_FEPROT_EPROT_SHIFT                  (0U)
#define FTFE_FEPROT_EPROT(x)                     (((uint8_t)(((uint8_t)(x)) << FTFE_FEPROT_EPROT_SHIFT)) & FTFE_FEPROT_EPROT_MASK)

/*! @name FDPROT - Data Flash Protection Register */
#define FTFE_FDPROT_DPROT_MASK                   (0xFFU)
#define FTFE_FDPROT_DPROT_SHIFT                  (0U)
#define FTFE_FDPROT_DPROT(x)                     (((uint8_t)(((uint8_t)(x)) << FTFE_FDPROT_DPROT_SHIFT)) & FTFE_FDPROT_DPROT_MASK)


/*!
 * @}
 */ /* end of group FTFE_Register_Masks */


/* FTFE - Peripheral instance base addresses */
/** Peripheral FTFE base address */
#define FTFE_BASE                                (0x40020000u)
/** Peripheral FTFE base pointer */
#define FTFE                                     ((FTFE_TypeDef *)FTFE_BASE)
/** Array initializer of FTFE peripheral base addresses */
#define FTFE_BASE_ADDRS                          { FTFE_BASE }
/** Array initializer of FTFE peripheral base pointers */
#define FTFE_BASE_PTRS                           { FTFE }
/** Interrupt vectors for the FTFE peripheral type */
#define FTFE_COMMAND_COMPLETE_IRQS               { FTFE_IRQn }
#define FTFE_READ_COLLISION_IRQS                 { Read_Collision_IRQn }

/*!
 * @}
 */ /* end of group FTFE_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- FTM Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup FTM_Peripheral_Access_Layer FTM Peripheral Access Layer
 * @{
 */

/** FTM - Register Layout Typedef */
typedef struct {
  __IO uint32_t SC;                                /**< Status And Control, offset: 0x0 */
  __IO uint32_t CNT;                               /**< Counter, offset: 0x4 */
  __IO uint32_t MOD;                               /**< Modulo, offset: 0x8 */
  struct {                                         /* offset: 0xC, array step: 0x8 */
    __IO uint32_t CnSC;                              /**< Channel (n) Status And Control, array offset: 0xC, array step: 0x8 */
    __IO uint32_t CnV;                               /**< Channel (n) Value, array offset: 0x10, array step: 0x8 */
  } CONTROLS[8];
  __IO uint32_t CNTIN;                             /**< Counter Initial Value, offset: 0x4C */
  __IO uint32_t STATUS;                            /**< Capture And Compare Status, offset: 0x50 */
  __IO uint32_t MODE;                              /**< Features Mode Selection, offset: 0x54 */
  __IO uint32_t SYNC;                              /**< Synchronization, offset: 0x58 */
  __IO uint32_t OUTINIT;                           /**< Initial State For Channels Output, offset: 0x5C */
  __IO uint32_t OUTMASK;                           /**< Output Mask, offset: 0x60 */
  __IO uint32_t COMBINE;                           /**< Function For Linked Channels, offset: 0x64 */
  __IO uint32_t DEADTIME;                          /**< Deadtime Insertion Control, offset: 0x68 */
  __IO uint32_t EXTTRIG;                           /**< FTM External Trigger, offset: 0x6C */
  __IO uint32_t POL;                               /**< Channels Polarity, offset: 0x70 */
  __IO uint32_t FMS;                               /**< Fault Mode Status, offset: 0x74 */
  __IO uint32_t FILTER;                            /**< Input Capture Filter Control, offset: 0x78 */
  __IO uint32_t FLTCTRL;                           /**< Fault Control, offset: 0x7C */
  __IO uint32_t QDCTRL;                            /**< Quadrature Decoder Control And Status, offset: 0x80 */
  __IO uint32_t CONF;                              /**< Configuration, offset: 0x84 */
  __IO uint32_t FLTPOL;                            /**< FTM Fault Input Polarity, offset: 0x88 */
  __IO uint32_t SYNCONF;                           /**< Synchronization Configuration, offset: 0x8C */
  __IO uint32_t INVCTRL;                           /**< FTM Inverting Control, offset: 0x90 */
  __IO uint32_t SWOCTRL;                           /**< FTM Software Output Control, offset: 0x94 */
  __IO uint32_t PWMLOAD;                           /**< FTM PWM Load, offset: 0x98 */
} FTM_TypeDef;

/* ----------------------------------------------------------------------------
   -- FTM Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup FTM_Register_Masks FTM Register Masks
 * @{
 */

/*! @name SC - Status And Control */
#define FTM_SC_PS_MASK                           (0x7U)
#define FTM_SC_PS_SHIFT                          (0U)
#define FTM_SC_PS(x)                             (((uint32_t)(((uint32_t)(x)) << FTM_SC_PS_SHIFT)) & FTM_SC_PS_MASK)
#define FTM_SC_CLKS_MASK                         (0x18U)
#define FTM_SC_CLKS_SHIFT                        (3U)
#define FTM_SC_CLKS(x)                           (((uint32_t)(((uint32_t)(x)) << FTM_SC_CLKS_SHIFT)) & FTM_SC_CLKS_MASK)
#define FTM_SC_CPWMS                             (0x20U)
#define FTM_SC_TOIE                              (0x40U)
#define FTM_SC_TOF                               (0x80U)

/*! @name CNT - Counter */
#define FTM_CNT_COUNT_MASK                       (0xFFFFU)
#define FTM_CNT_COUNT_SHIFT                      (0U)
#define FTM_CNT_COUNT(x)                         (((uint32_t)(((uint32_t)(x)) << FTM_CNT_COUNT_SHIFT)) & FTM_CNT_COUNT_MASK)

/*! @name MOD - Modulo */
#define FTM_MOD_MOD_MASK                         (0xFFFFU)
#define FTM_MOD_MOD_SHIFT                        (0U)
#define FTM_MOD_MOD(x)                           (((uint32_t)(((uint32_t)(x)) << FTM_MOD_MOD_SHIFT)) & FTM_MOD_MOD_MASK)

/*! @name CnSC - Channel (n) Status And Control */
#define FTM_CnSC_DMA                             (0x1U)
#define FTM_CnSC_ELSA                            (0x4U)
#define FTM_CnSC_ELSB                            (0x8U)
#define FTM_CnSC_MSA                             (0x10U)
#define FTM_CnSC_MSB                             (0x20U)
#define FTM_CnSC_CHIE                            (0x40U)
#define FTM_CnSC_CHF                             (0x80U)

/* The count of FTM_CnSC */
#define FTM_CnSC_COUNT                           (8U)

/*! @name CnV - Channel (n) Value */
#define FTM_CnV_VAL_MASK                         (0xFFFFU)
#define FTM_CnV_VAL_SHIFT                        (0U)
#define FTM_CnV_VAL(x)                           (((uint32_t)(((uint32_t)(x)) << FTM_CnV_VAL_SHIFT)) & FTM_CnV_VAL_MASK)

/* The count of FTM_CnV */
#define FTM_CnV_COUNT                            (8U)

/*! @name CNTIN - Counter Initial Value */
#define FTM_CNTIN_INIT_MASK                      (0xFFFFU)
#define FTM_CNTIN_INIT_SHIFT                     (0U)
#define FTM_CNTIN_INIT(x)                        (((uint32_t)(((uint32_t)(x)) << FTM_CNTIN_INIT_SHIFT)) & FTM_CNTIN_INIT_MASK)

/*! @name STATUS - Capture And Compare Status */
#define FTM_STATUS_CH0F                          (0x1U)
#define FTM_STATUS_CH1F                          (0x2U)
#define FTM_STATUS_CH2F                          (0x4U)
#define FTM_STATUS_CH3F                          (0x8U)
#define FTM_STATUS_CH4F                          (0x10U)
#define FTM_STATUS_CH5F                          (0x20U)
#define FTM_STATUS_CH6F                          (0x40U)
#define FTM_STATUS_CH7F                          (0x80U)

/*! @name MODE - Features Mode Selection */
#define FTM_MODE_FTMEN                           (0x1U)
#define FTM_MODE_INIT                            (0x2U)
#define FTM_MODE_WPDIS                           (0x4U)
#define FTM_MODE_PWMSYNC                         (0x8U)
#define FTM_MODE_CAPTEST                         (0x10U)
#define FTM_MODE_FAULTM_MASK                     (0x60U)
#define FTM_MODE_FAULTM_SHIFT                    (5U)
#define FTM_MODE_FAULTM(x)                       (((uint32_t)(((uint32_t)(x)) << FTM_MODE_FAULTM_SHIFT)) & FTM_MODE_FAULTM_MASK)
#define FTM_MODE_FAULTIE                         (0x80U)

/*! @name SYNC - Synchronization */
#define FTM_SYNC_CNTMIN                          (0x1U)
#define FTM_SYNC_CNTMAX                          (0x2U)
#define FTM_SYNC_REINIT                          (0x4U)
#define FTM_SYNC_SYNCHOM                         (0x8U)
#define FTM_SYNC_TRIG0                           (0x10U)
#define FTM_SYNC_TRIG1                           (0x20U)
#define FTM_SYNC_TRIG2                           (0x40U)
#define FTM_SYNC_SWSYNC                          (0x80U)

/*! @name OUTINIT - Initial State For Channels Output */
#define FTM_OUTINIT_CH0OI                        (0x1U)
#define FTM_OUTINIT_CH1OI                        (0x2U)
#define FTM_OUTINIT_CH2OI                        (0x4U)
#define FTM_OUTINIT_CH3OI                        (0x8U)
#define FTM_OUTINIT_CH4OI                        (0x10U)
#define FTM_OUTINIT_CH5OI                        (0x20U)
#define FTM_OUTINIT_CH6OI                        (0x40U)
#define FTM_OUTINIT_CH7OI                        (0x80U)

/*! @name OUTMASK - Output Mask */
#define FTM_OUTMASK_CH0OM                        (0x1U)
#define FTM_OUTMASK_CH1OM                        (0x2U)
#define FTM_OUTMASK_CH2OM                        (0x4U)
#define FTM_OUTMASK_CH3OM                        (0x8U)
#define FTM_OUTMASK_CH4OM                        (0x10U)
#define FTM_OUTMASK_CH5OM                        (0x20U)
#define FTM_OUTMASK_CH6OM                        (0x40U)
#define FTM_OUTMASK_CH7OM                        (0x80U)

/*! @name COMBINE - Function For Linked Channels */
#define FTM_COMBINE_COMBINE0                     (0x1U)
#define FTM_COMBINE_COMP0                        (0x2U)
#define FTM_COMBINE_DECAPEN0                     (0x4U)
#define FTM_COMBINE_DECAP0                       (0x8U)
#define FTM_COMBINE_DTEN0                        (0x10U)
#define FTM_COMBINE_SYNCEN0                      (0x20U)
#define FTM_COMBINE_FAULTEN0                     (0x40U)
#define FTM_COMBINE_COMBINE1                     (0x100U)
#define FTM_COMBINE_COMP1                        (0x200U)
#define FTM_COMBINE_DECAPEN1                     (0x400U)
#define FTM_COMBINE_DECAP1                       (0x800U)
#define FTM_COMBINE_DTEN1                        (0x1000U)
#define FTM_COMBINE_SYNCEN1                      (0x2000U)
#define FTM_COMBINE_FAULTEN1                     (0x4000U)
#define FTM_COMBINE_COMBINE2                     (0x10000U)
#define FTM_COMBINE_COMP2                        (0x20000U)
#define FTM_COMBINE_DECAPEN2                     (0x40000U)
#define FTM_COMBINE_DECAP2                       (0x80000U)
#define FTM_COMBINE_DTEN2                        (0x100000U)
#define FTM_COMBINE_SYNCEN2                      (0x200000U)
#define FTM_COMBINE_FAULTEN2                     (0x400000U)
#define FTM_COMBINE_COMBINE3                     (0x1000000U)
#define FTM_COMBINE_COMP3                        (0x2000000U)
#define FTM_COMBINE_DECAPEN3                     (0x4000000U)
#define FTM_COMBINE_DECAP3                       (0x8000000U)
#define FTM_COMBINE_DTEN3                        (0x10000000U)
#define FTM_COMBINE_SYNCEN3                      (0x20000000U)
#define FTM_COMBINE_FAULTEN3                     (0x40000000U)

/*! @name DEADTIME - Deadtime Insertion Control */
#define FTM_DEADTIME_DTVAL_MASK                  (0x3FU)
#define FTM_DEADTIME_DTVAL_SHIFT                 (0U)
#define FTM_DEADTIME_DTVAL(x)                    (((uint32_t)(((uint32_t)(x)) << FTM_DEADTIME_DTVAL_SHIFT)) & FTM_DEADTIME_DTVAL_MASK)
#define FTM_DEADTIME_DTPS_MASK                   (0xC0U)
#define FTM_DEADTIME_DTPS_SHIFT                  (6U)
#define FTM_DEADTIME_DTPS(x)                     (((uint32_t)(((uint32_t)(x)) << FTM_DEADTIME_DTPS_SHIFT)) & FTM_DEADTIME_DTPS_MASK)

/*! @name EXTTRIG - FTM External Trigger */
#define FTM_EXTTRIG_CH2TRIG                      (0x1U)
#define FTM_EXTTRIG_CH3TRIG                      (0x2U)
#define FTM_EXTTRIG_CH4TRIG                      (0x4U)
#define FTM_EXTTRIG_CH5TRIG                      (0x8U)
#define FTM_EXTTRIG_CH0TRIG                      (0x10U)
#define FTM_EXTTRIG_CH1TRIG                      (0x20U)
#define FTM_EXTTRIG_INITTRIGEN                   (0x40U)
#define FTM_EXTTRIG_TRIGF                        (0x80U)

/*! @name POL - Channels Polarity */
#define FTM_POL_POL0                             (0x1U)
#define FTM_POL_POL1                             (0x2U)
#define FTM_POL_POL2                             (0x4U)
#define FTM_POL_POL3                             (0x8U)
#define FTM_POL_POL4                             (0x10U)
#define FTM_POL_POL5                             (0x20U)
#define FTM_POL_POL6                             (0x40U)
#define FTM_POL_POL7                             (0x80U)

/*! @name FMS - Fault Mode Status */
#define FTM_FMS_FAULTF0                          (0x1U)
#define FTM_FMS_FAULTF1                          (0x2U)
#define FTM_FMS_FAULTF2                          (0x4U)
#define FTM_FMS_FAULTF3                          (0x8U)
#define FTM_FMS_FAULTIN                          (0x20U)
#define FTM_FMS_WPEN                             (0x40U)
#define FTM_FMS_FAULTF                           (0x80U)

/*! @name FILTER - Input Capture Filter Control */
#define FTM_FILTER_CH0FVAL_MASK                  (0xFU)
#define FTM_FILTER_CH0FVAL_SHIFT                 (0U)
#define FTM_FILTER_CH0FVAL(x)                    (((uint32_t)(((uint32_t)(x)) << FTM_FILTER_CH0FVAL_SHIFT)) & FTM_FILTER_CH0FVAL_MASK)
#define FTM_FILTER_CH1FVAL_MASK                  (0xF0U)
#define FTM_FILTER_CH1FVAL_SHIFT                 (4U)
#define FTM_FILTER_CH1FVAL(x)                    (((uint32_t)(((uint32_t)(x)) << FTM_FILTER_CH1FVAL_SHIFT)) & FTM_FILTER_CH1FVAL_MASK)
#define FTM_FILTER_CH2FVAL_MASK                  (0xF00U)
#define FTM_FILTER_CH2FVAL_SHIFT                 (8U)
#define FTM_FILTER_CH2FVAL(x)                    (((uint32_t)(((uint32_t)(x)) << FTM_FILTER_CH2FVAL_SHIFT)) & FTM_FILTER_CH2FVAL_MASK)
#define FTM_FILTER_CH3FVAL_MASK                  (0xF000U)
#define FTM_FILTER_CH3FVAL_SHIFT                 (12U)
#define FTM_FILTER_CH3FVAL(x)                    (((uint32_t)(((uint32_t)(x)) << FTM_FILTER_CH3FVAL_SHIFT)) & FTM_FILTER_CH3FVAL_MASK)

/*! @name FLTCTRL - Fault Control */
#define FTM_FLTCTRL_FAULT0EN                     (0x1U)
#define FTM_FLTCTRL_FAULT1EN                     (0x2U)
#define FTM_FLTCTRL_FAULT2EN                     (0x4U)
#define FTM_FLTCTRL_FAULT3EN                     (0x8U)
#define FTM_FLTCTRL_FFLTR0EN                     (0x10U)
#define FTM_FLTCTRL_FFLTR1EN                     (0x20U)
#define FTM_FLTCTRL_FFLTR2EN                     (0x40U)
#define FTM_FLTCTRL_FFLTR3EN                     (0x80U)
#define FTM_FLTCTRL_FFVAL_MASK                   (0xF00U)
#define FTM_FLTCTRL_FFVAL_SHIFT                  (8U)
#define FTM_FLTCTRL_FFVAL(x)                     (((uint32_t)(((uint32_t)(x)) << FTM_FLTCTRL_FFVAL_SHIFT)) & FTM_FLTCTRL_FFVAL_MASK)

/*! @name QDCTRL - Quadrature Decoder Control And Status */
#define FTM_QDCTRL_QUADEN                        (0x1U)
#define FTM_QDCTRL_TOFDIR                        (0x2U)
#define FTM_QDCTRL_QUADIR                        (0x4U)
#define FTM_QDCTRL_QUADMODE                      (0x8U)
#define FTM_QDCTRL_PHBPOL                        (0x10U)
#define FTM_QDCTRL_PHAPOL                        (0x20U)
#define FTM_QDCTRL_PHBFLTREN                     (0x40U)
#define FTM_QDCTRL_PHAFLTREN                     (0x80U)

/*! @name CONF - Configuration */
#define FTM_CONF_NUMTOF_MASK                     (0x1FU)
#define FTM_CONF_NUMTOF_SHIFT                    (0U)
#define FTM_CONF_NUMTOF(x)                       (((uint32_t)(((uint32_t)(x)) << FTM_CONF_NUMTOF_SHIFT)) & FTM_CONF_NUMTOF_MASK)
#define FTM_CONF_BDMMODE_MASK                    (0xC0U)
#define FTM_CONF_BDMMODE_SHIFT                   (6U)
#define FTM_CONF_BDMMODE(x)                      (((uint32_t)(((uint32_t)(x)) << FTM_CONF_BDMMODE_SHIFT)) & FTM_CONF_BDMMODE_MASK)
#define FTM_CONF_GTBEEN                          (0x200U)
#define FTM_CONF_GTBEOUT                         (0x400U)

/*! @name FLTPOL - FTM Fault Input Polarity */
#define FTM_FLTPOL_FLT0POL                       (0x1U)
#define FTM_FLTPOL_FLT1POL                       (0x2U)
#define FTM_FLTPOL_FLT2POL                       (0x4U)
#define FTM_FLTPOL_FLT3POL                       (0x8U)

/*! @name SYNCONF - Synchronization Configuration */
#define FTM_SYNCONF_HWTRIGMODE                   (0x1U)
#define FTM_SYNCONF_CNTINC                       (0x4U)
#define FTM_SYNCONF_INVC                         (0x10U)
#define FTM_SYNCONF_SWOC                         (0x20U)
#define FTM_SYNCONF_SYNCMODE                     (0x80U)
#define FTM_SYNCONF_SWRSTCNT                     (0x100U)
#define FTM_SYNCONF_SWWRBUF                      (0x200U)
#define FTM_SYNCONF_SWOM                         (0x400U)
#define FTM_SYNCONF_SWINVC                       (0x800U)
#define FTM_SYNCONF_SWSOC                        (0x1000U)
#define FTM_SYNCONF_HWRSTCNT                     (0x10000U)
#define FTM_SYNCONF_HWWRBUF                      (0x20000U)
#define FTM_SYNCONF_HWOM                         (0x40000U)
#define FTM_SYNCONF_HWINVC                       (0x80000U)
#define FTM_SYNCONF_HWSOC                        (0x100000U)

/*! @name INVCTRL - FTM Inverting Control */
#define FTM_INVCTRL_INV0EN                       (0x1U)
#define FTM_INVCTRL_INV1EN                       (0x2U)
#define FTM_INVCTRL_INV2EN                       (0x4U)
#define FTM_INVCTRL_INV3EN                       (0x8U)

/*! @name SWOCTRL - FTM Software Output Control */
#define FTM_SWOCTRL_CH0OC                        (0x1U)
#define FTM_SWOCTRL_CH1OC                        (0x2U)
#define FTM_SWOCTRL_CH2OC                        (0x4U)
#define FTM_SWOCTRL_CH3OC                        (0x8U)
#define FTM_SWOCTRL_CH4OC                        (0x10U)
#define FTM_SWOCTRL_CH5OC                        (0x20U)
#define FTM_SWOCTRL_CH6OC                        (0x40U)
#define FTM_SWOCTRL_CH7OC                        (0x80U)
#define FTM_SWOCTRL_CH0OCV                       (0x100U)
#define FTM_SWOCTRL_CH1OCV                       (0x200U)
#define FTM_SWOCTRL_CH2OCV                       (0x400U)
#define FTM_SWOCTRL_CH3OCV                       (0x800U)
#define FTM_SWOCTRL_CH4OCV                       (0x1000U)
#define FTM_SWOCTRL_CH5OCV                       (0x2000U)
#define FTM_SWOCTRL_CH6OCV                       (0x4000U)
#define FTM_SWOCTRL_CH7OCV                       (0x8000U)

/*! @name PWMLOAD - FTM PWM Load */
#define FTM_PWMLOAD_CH0SEL                       (0x1U)
#define FTM_PWMLOAD_CH1SEL                       (0x2U)
#define FTM_PWMLOAD_CH2SEL                       (0x4U)
#define FTM_PWMLOAD_CH3SEL                       (0x8U)
#define FTM_PWMLOAD_CH4SEL                       (0x10U)
#define FTM_PWMLOAD_CH5SEL                       (0x20U)
#define FTM_PWMLOAD_CH6SEL                       (0x40U)
#define FTM_PWMLOAD_CH7SEL                       (0x80U)
#define FTM_PWMLOAD_LDOK                         (0x200U)


/*!
 * @}
 */ /* end of group FTM_Register_Masks */


/* FTM - Peripheral instance base addresses */
/** Peripheral FTM0 base address */
#define FTM0_BASE                                (0x40038000u)
/** Peripheral FTM0 base pointer */
#define FTM0                                     ((FTM_TypeDef *)FTM0_BASE)
/** Peripheral FTM1 base address */
#define FTM1_BASE                                (0x40039000u)
/** Peripheral FTM1 base pointer */
#define FTM1                                     ((FTM_TypeDef *)FTM1_BASE)
/** Peripheral FTM2 base address */
#define FTM2_BASE                                (0x4003A000u)
/** Peripheral FTM2 base pointer */
#define FTM2                                     ((FTM_TypeDef *)FTM2_BASE)
/** Peripheral FTM3 base address */
#define FTM3_BASE                                (0x400B9000u)
/** Peripheral FTM3 base pointer */
#define FTM3                                     ((FTM_TypeDef *)FTM3_BASE)
/** Array initializer of FTM peripheral base addresses */
#define FTM_BASE_ADDRS                           { FTM0_BASE, FTM1_BASE, FTM2_BASE, FTM3_BASE }
/** Array initializer of FTM peripheral base pointers */
#define FTM_BASE_PTRS                            { FTM0, FTM1, FTM2, FTM3 }
/** Interrupt vectors for the FTM peripheral type */
#define FTM_IRQS                                 { FTM0_IRQn, FTM1_IRQn, FTM2_IRQn, FTM3_IRQn }

/*!
 * @}
 */ /* end of group FTM_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- GPIO Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup GPIO_Peripheral_Access_Layer GPIO Peripheral Access Layer
 * @{
 */

/** GPIO - Register Layout Typedef */
typedef struct {
  __IO uint32_t PDOR;                              /**< Port Data Output Register, offset: 0x0 */
  __O  uint32_t PSOR;                              /**< Port Set Output Register, offset: 0x4 */
  __O  uint32_t PCOR;                              /**< Port Clear Output Register, offset: 0x8 */
  __O  uint32_t PTOR;                              /**< Port Toggle Output Register, offset: 0xC */
  __I  uint32_t PDIR;                              /**< Port Data Input Register, offset: 0x10 */
  __IO uint32_t PDDR;                              /**< Port Data Direction Register, offset: 0x14 */
} GPIO_TypeDef;

/* GPIO - Peripheral instance base addresses */
/** Peripheral GPIOA base address */
#define GPIOA_BASE                               (0x400FF000u)
/** Peripheral GPIOA base pointer */
#define GPIOA                                    ((GPIO_TypeDef *)GPIOA_BASE)
/** Peripheral GPIOB base address */
#define GPIOB_BASE                               (0x400FF040u)
/** Peripheral GPIOB base pointer */
#define GPIOB                                    ((GPIO_TypeDef *)GPIOB_BASE)
/** Peripheral GPIOC base address */
#define GPIOC_BASE                               (0x400FF080u)
/** Peripheral GPIOC base pointer */
#define GPIOC                                    ((GPIO_TypeDef *)GPIOC_BASE)
/** Peripheral GPIOD base address */
#define GPIOD_BASE                               (0x400FF0C0u)
/** Peripheral GPIOD base pointer */
#define GPIOD                                    ((GPIO_TypeDef *)GPIOD_BASE)
/** Peripheral GPIOE base address */
#define GPIOE_BASE                               (0x400FF100u)
/** Peripheral GPIOE base pointer */
#define GPIOE                                    ((GPIO_TypeDef *)GPIOE_BASE)
/** Array initializer of GPIO peripheral base addresses */
#define GPIO_BASE_ADDRS                          { GPIOA_BASE, GPIOB_BASE, GPIOC_BASE, GPIOD_BASE, GPIOE_BASE }
/** Array initializer of GPIO peripheral base pointers */
#define GPIO_BASE_PTRS                           { GPIOA, GPIOB, GPIOC, GPIOD, GPIOE }

/*!
 * @}
 */ /* end of group GPIO_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- I2C Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup I2C_Peripheral_Access_Layer I2C Peripheral Access Layer
 * @{
 */

/** I2C - Register Layout Typedef */
typedef struct {
  __IO uint8_t A1;                                 /**< I2C Address Register 1, offset: 0x0 */
  __IO uint8_t F;                                  /**< I2C Frequency Divider register, offset: 0x1 */
  __IO uint8_t C1;                                 /**< I2C Control Register 1, offset: 0x2 */
  __IO uint8_t S;                                  /**< I2C Status register, offset: 0x3 */
  __IO uint8_t D;                                  /**< I2C Data I/O register, offset: 0x4 */
  __IO uint8_t C2;                                 /**< I2C Control Register 2, offset: 0x5 */
  __IO uint8_t FLT;                                /**< I2C Programmable Input Glitch Filter register, offset: 0x6 */
  __IO uint8_t RA;                                 /**< I2C Range Address register, offset: 0x7 */
  __IO uint8_t SMB;                                /**< I2C SMBus Control and Status register, offset: 0x8 */
  __IO uint8_t A2;                                 /**< I2C Address Register 2, offset: 0x9 */
  __IO uint8_t SLTH;                               /**< I2C SCL Low Timeout Register High, offset: 0xA */
  __IO uint8_t SLTL;                               /**< I2C SCL Low Timeout Register Low, offset: 0xB */
} I2C_TypeDef;

/* ----------------------------------------------------------------------------
   -- I2C Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup I2C_Register_Masks I2C Register Masks
 * @{
 */

/*! @name A1 - I2C Address Register 1 */
#define I2C_A1_AD_MASK                           (0xFEU)
#define I2C_A1_AD_SHIFT                          (1U)
#define I2C_A1_AD(x)                             (((uint8_t)(((uint8_t)(x)) << I2C_A1_AD_SHIFT)) & I2C_A1_AD_MASK)

/*! @name F - I2C Frequency Divider register */
#define I2C_F_ICR_MASK                           (0x3FU)
#define I2C_F_ICR_SHIFT                          (0U)
#define I2C_F_ICR(x)                             (((uint8_t)(((uint8_t)(x)) << I2C_F_ICR_SHIFT)) & I2C_F_ICR_MASK)
#define I2C_F_MULT_MASK                          (0xC0U)
#define I2C_F_MULT_SHIFT                         (6U)
#define I2C_F_MULT(x)                            (((uint8_t)(((uint8_t)(x)) << I2C_F_MULT_SHIFT)) & I2C_F_MULT_MASK)

/*! @name C1 - I2C Control Register 1 */
#define I2C_C1_DMAEN                             (0x1U)
#define I2C_C1_WUEN                              (0x2U)
#define I2C_C1_RSTA                              (0x4U)
#define I2C_C1_TXAK                              (0x8U)
#define I2C_C1_TX                                (0x10U)
#define I2C_C1_MST                               (0x20U)
#define I2C_C1_IICIE                             (0x40U)
#define I2C_C1_IICEN                             (0x80U)

/*! @name S - I2C Status register */
#define I2C_S_RXAK                               (0x1U)
#define I2C_S_IICIF                              (0x2U)
#define I2C_S_SRW                                (0x4U)
#define I2C_S_RAM                                (0x8U)
#define I2C_S_ARBL                               (0x10U)
#define I2C_S_BUSY                               (0x20U)
#define I2C_S_IAAS                               (0x40U)
#define I2C_S_TCF                                (0x80U)

/*! @name C2 - I2C Control Register 2 */
#define I2C_C2_AD_MASK                           (0x7U)
#define I2C_C2_AD_SHIFT                          (0U)
#define I2C_C2_AD(x)                             (((uint8_t)(((uint8_t)(x)) << I2C_C2_AD_SHIFT)) & I2C_C2_AD_MASK)
#define I2C_C2_RMEN                              (0x8U)
#define I2C_C2_SBRC                              (0x10U)
#define I2C_C2_HDRS                              (0x20U)
#define I2C_C2_ADEXT                             (0x40U)
#define I2C_C2_GCAEN                             (0x80U)

/*! @name FLT - I2C Programmable Input Glitch Filter register */
#define I2C_FLT_FLT_MASK                         (0xFU)
#define I2C_FLT_FLT_SHIFT                        (0U)
#define I2C_FLT_FLT(x)                           (((uint8_t)(((uint8_t)(x)) << I2C_FLT_FLT_SHIFT)) & I2C_FLT_FLT_MASK)
#define I2C_FLT_STARTF                           (0x10U)
#define I2C_FLT_SSIE                             (0x20U)
#define I2C_FLT_STOPF                            (0x40U)
#define I2C_FLT_SHEN                             (0x80U)

/*! @name RA - I2C Range Address register */
#define I2C_RA_RAD_MASK                          (0xFEU)
#define I2C_RA_RAD_SHIFT                         (1U)
#define I2C_RA_RAD(x)                            (((uint8_t)(((uint8_t)(x)) << I2C_RA_RAD_SHIFT)) & I2C_RA_RAD_MASK)

/*! @name SMB - I2C SMBus Control and Status register */
#define I2C_SMB_SHTF2IE                          (0x1U)
#define I2C_SMB_SHTF2                            (0x2U)
#define I2C_SMB_SHTF1                            (0x4U)
#define I2C_SMB_SLTF                             (0x8U)
#define I2C_SMB_TCKSEL                           (0x10U)
#define I2C_SMB_SIICAEN                          (0x20U)
#define I2C_SMB_ALERTEN                          (0x40U)
#define I2C_SMB_FACK                             (0x80U)

/*! @name A2 - I2C Address Register 2 */
#define I2C_A2_SAD_MASK                          (0xFEU)
#define I2C_A2_SAD_SHIFT                         (1U)
#define I2C_A2_SAD(x)                            (((uint8_t)(((uint8_t)(x)) << I2C_A2_SAD_SHIFT)) & I2C_A2_SAD_MASK)

/*!
 * @}
 */ /* end of group I2C_Register_Masks */


/* I2C - Peripheral instance base addresses */
/** Peripheral I2C0 base address */
#define I2C0_BASE                                (0x40066000u)
/** Peripheral I2C0 base pointer */
#define I2C0                                     ((I2C_TypeDef *)I2C0_BASE)
/** Peripheral I2C1 base address */
#define I2C1_BASE                                (0x40067000u)
/** Peripheral I2C1 base pointer */
#define I2C1                                     ((I2C_TypeDef *)I2C1_BASE)
/** Peripheral I2C2 base address */
#define I2C2_BASE                                (0x400E6000u)
/** Peripheral I2C2 base pointer */
#define I2C2                                     ((I2C_TypeDef *)I2C2_BASE)
/** Array initializer of I2C peripheral base addresses */
#define I2C_BASE_ADDRS                           { I2C0_BASE, I2C1_BASE, I2C2_BASE }
/** Array initializer of I2C peripheral base pointers */
#define I2C_BASE_PTRS                            { I2C0, I2C1, I2C2 }
/** Interrupt vectors for the I2C peripheral type */
#define I2C_IRQS                                 { I2C0_IRQn, I2C1_IRQn, I2C2_IRQn }

/*!
 * @}
 */ /* end of group I2C_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- I2S Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup I2S_Peripheral_Access_Layer I2S Peripheral Access Layer
 * @{
 */

/** I2S - Register Layout Typedef */
typedef struct {
  __IO uint32_t TCSR;                              /**< SAI Transmit Control Register, offset: 0x0 */
  __IO uint32_t TCR1;                              /**< SAI Transmit Configuration 1 Register, offset: 0x4 */
  __IO uint32_t TCR2;                              /**< SAI Transmit Configuration 2 Register, offset: 0x8 */
  __IO uint32_t TCR3;                              /**< SAI Transmit Configuration 3 Register, offset: 0xC */
  __IO uint32_t TCR4;                              /**< SAI Transmit Configuration 4 Register, offset: 0x10 */
  __IO uint32_t TCR5;                              /**< SAI Transmit Configuration 5 Register, offset: 0x14 */
       uint8_t RESERVED_0[8];
  __O  uint32_t TDR[2];                            /**< SAI Transmit Data Register, array offset: 0x20, array step: 0x4 */
       uint8_t RESERVED_1[24];
  __I  uint32_t TFR[2];                            /**< SAI Transmit FIFO Register, array offset: 0x40, array step: 0x4 */
       uint8_t RESERVED_2[24];
  __IO uint32_t TMR;                               /**< SAI Transmit Mask Register, offset: 0x60 */
       uint8_t RESERVED_3[28];
  __IO uint32_t RCSR;                              /**< SAI Receive Control Register, offset: 0x80 */
  __IO uint32_t RCR1;                              /**< SAI Receive Configuration 1 Register, offset: 0x84 */
  __IO uint32_t RCR2;                              /**< SAI Receive Configuration 2 Register, offset: 0x88 */
  __IO uint32_t RCR3;                              /**< SAI Receive Configuration 3 Register, offset: 0x8C */
  __IO uint32_t RCR4;                              /**< SAI Receive Configuration 4 Register, offset: 0x90 */
  __IO uint32_t RCR5;                              /**< SAI Receive Configuration 5 Register, offset: 0x94 */
       uint8_t RESERVED_4[8];
  __I  uint32_t RDR[2];                            /**< SAI Receive Data Register, array offset: 0xA0, array step: 0x4 */
       uint8_t RESERVED_5[24];
  __I  uint32_t RFR[2];                            /**< SAI Receive FIFO Register, array offset: 0xC0, array step: 0x4 */
       uint8_t RESERVED_6[24];
  __IO uint32_t RMR;                               /**< SAI Receive Mask Register, offset: 0xE0 */
       uint8_t RESERVED_7[28];
  __IO uint32_t MCR;                               /**< SAI MCLK Control Register, offset: 0x100 */
  __IO uint32_t MDR;                               /**< SAI MCLK Divide Register, offset: 0x104 */
} I2S_TypeDef;

/* ----------------------------------------------------------------------------
   -- I2S Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup I2S_Register_Masks I2S Register Masks
 * @{
 */

/*! @name TCSR - SAI Transmit Control Register */
#define I2S_TCSR_FRDE                            (0x1U)
#define I2S_TCSR_FWDE                            (0x2U)
#define I2S_TCSR_FRIE                            (0x100U)
#define I2S_TCSR_FWIE                            (0x200U)
#define I2S_TCSR_FEIE                            (0x400U)
#define I2S_TCSR_SEIE                            (0x800U)
#define I2S_TCSR_WSIE                            (0x1000U)
#define I2S_TCSR_FRF                             (0x10000U)
#define I2S_TCSR_FWF                             (0x20000U)
#define I2S_TCSR_FEF                             (0x40000U)
#define I2S_TCSR_SEF                             (0x80000U)
#define I2S_TCSR_WSF                             (0x100000U)
#define I2S_TCSR_SR                              (0x1000000U)
#define I2S_TCSR_FR                              (0x2000000U)
#define I2S_TCSR_BCE                             (0x10000000U)
#define I2S_TCSR_DBGE                            (0x20000000U)
#define I2S_TCSR_STOPE                           (0x40000000U)
#define I2S_TCSR_TE                              (0x80000000U)

/*! @name TCR1 - SAI Transmit Configuration 1 Register */
#define I2S_TCR1_TFW_MASK                        (0x7U)
#define I2S_TCR1_TFW_SHIFT                       (0U)
#define I2S_TCR1_TFW(x)                          (((uint32_t)(((uint32_t)(x)) << I2S_TCR1_TFW_SHIFT)) & I2S_TCR1_TFW_MASK)

/*! @name TCR2 - SAI Transmit Configuration 2 Register */
#define I2S_TCR2_DIV_MASK                        (0xFFU)
#define I2S_TCR2_DIV_SHIFT                       (0U)
#define I2S_TCR2_DIV(x)                          (((uint32_t)(((uint32_t)(x)) << I2S_TCR2_DIV_SHIFT)) & I2S_TCR2_DIV_MASK)
#define I2S_TCR2_BCD                             (0x1000000U)
#define I2S_TCR2_BCP                             (0x2000000U)
#define I2S_TCR2_MSEL_MASK                       (0xC000000U)
#define I2S_TCR2_MSEL_SHIFT                      (26U)
#define I2S_TCR2_MSEL(x)                         (((uint32_t)(((uint32_t)(x)) << I2S_TCR2_MSEL_SHIFT)) & I2S_TCR2_MSEL_MASK)
#define I2S_TCR2_BCI                             (0x10000000U)
#define I2S_TCR2_BCS                             (0x20000000U)
#define I2S_TCR2_SYNC_MASK                       (0xC0000000U)
#define I2S_TCR2_SYNC_SHIFT                      (30U)
#define I2S_TCR2_SYNC(x)                         (((uint32_t)(((uint32_t)(x)) << I2S_TCR2_SYNC_SHIFT)) & I2S_TCR2_SYNC_MASK)

/*! @name TCR3 - SAI Transmit Configuration 3 Register */
#define I2S_TCR3_WDFL_MASK                       (0x1FU)
#define I2S_TCR3_WDFL_SHIFT                      (0U)
#define I2S_TCR3_WDFL(x)                         (((uint32_t)(((uint32_t)(x)) << I2S_TCR3_WDFL_SHIFT)) & I2S_TCR3_WDFL_MASK)
#define I2S_TCR3_TCE_MASK                        (0x30000U)
#define I2S_TCR3_TCE_SHIFT                       (16U)
#define I2S_TCR3_TCE(x)                          (((uint32_t)(((uint32_t)(x)) << I2S_TCR3_TCE_SHIFT)) & I2S_TCR3_TCE_MASK)

/*! @name TCR4 - SAI Transmit Configuration 4 Register */
#define I2S_TCR4_FSD                             (0x1U)
#define I2S_TCR4_FSP                             (0x2U)
#define I2S_TCR4_FSE                             (0x8U)
#define I2S_TCR4_MF                              (0x10U)
#define I2S_TCR4_SYWD_MASK                       (0x1F00U)
#define I2S_TCR4_SYWD_SHIFT                      (8U)
#define I2S_TCR4_SYWD(x)                         (((uint32_t)(((uint32_t)(x)) << I2S_TCR4_SYWD_SHIFT)) & I2S_TCR4_SYWD_MASK)
#define I2S_TCR4_FRSZ_MASK                       (0x1F0000U)
#define I2S_TCR4_FRSZ_SHIFT                      (16U)
#define I2S_TCR4_FRSZ(x)                         (((uint32_t)(((uint32_t)(x)) << I2S_TCR4_FRSZ_SHIFT)) & I2S_TCR4_FRSZ_MASK)

/*! @name TCR5 - SAI Transmit Configuration 5 Register */
#define I2S_TCR5_FBT_MASK                        (0x1F00U)
#define I2S_TCR5_FBT_SHIFT                       (8U)
#define I2S_TCR5_FBT(x)                          (((uint32_t)(((uint32_t)(x)) << I2S_TCR5_FBT_SHIFT)) & I2S_TCR5_FBT_MASK)
#define I2S_TCR5_W0W_MASK                        (0x1F0000U)
#define I2S_TCR5_W0W_SHIFT                       (16U)
#define I2S_TCR5_W0W(x)                          (((uint32_t)(((uint32_t)(x)) << I2S_TCR5_W0W_SHIFT)) & I2S_TCR5_W0W_MASK)
#define I2S_TCR5_WNW_MASK                        (0x1F000000U)
#define I2S_TCR5_WNW_SHIFT                       (24U)
#define I2S_TCR5_WNW(x)                          (((uint32_t)(((uint32_t)(x)) << I2S_TCR5_WNW_SHIFT)) & I2S_TCR5_WNW_MASK)

/* The count of I2S_TDR */
#define I2S_TDR_COUNT                            (2U)

/*! @name TFR - SAI Transmit FIFO Register */
#define I2S_TFR_RFP_MASK                         (0xFU)
#define I2S_TFR_RFP_SHIFT                        (0U)
#define I2S_TFR_RFP(x)                           (((uint32_t)(((uint32_t)(x)) << I2S_TFR_RFP_SHIFT)) & I2S_TFR_RFP_MASK)
#define I2S_TFR_WFP_MASK                         (0xF0000U)
#define I2S_TFR_WFP_SHIFT                        (16U)
#define I2S_TFR_WFP(x)                           (((uint32_t)(((uint32_t)(x)) << I2S_TFR_WFP_SHIFT)) & I2S_TFR_WFP_MASK)

/* The count of I2S_TFR */
#define I2S_TFR_COUNT                            (2U)

/*! @name RCSR - SAI Receive Control Register */
#define I2S_RCSR_FRDE                            (0x1U)
#define I2S_RCSR_FWDE                            (0x2U)
#define I2S_RCSR_FRIE                            (0x100U)
#define I2S_RCSR_FWIE                            (0x200U)
#define I2S_RCSR_FEIE                            (0x400U)
#define I2S_RCSR_SEIE                            (0x800U)
#define I2S_RCSR_WSIE                            (0x1000U)
#define I2S_RCSR_FRF                             (0x10000U)
#define I2S_RCSR_FWF                             (0x20000U)
#define I2S_RCSR_FEF                             (0x40000U)
#define I2S_RCSR_SEF                             (0x80000U)
#define I2S_RCSR_WSF                             (0x100000U)
#define I2S_RCSR_SR                              (0x1000000U)
#define I2S_RCSR_FR                              (0x2000000U)
#define I2S_RCSR_BCE                             (0x10000000U)
#define I2S_RCSR_DBGE                            (0x20000000U)
#define I2S_RCSR_STOPE                           (0x40000000U)
#define I2S_RCSR_RE                              (0x80000000U)

/*! @name RCR1 - SAI Receive Configuration 1 Register */
#define I2S_RCR1_RFW_MASK                        (0x7U)
#define I2S_RCR1_RFW_SHIFT                       (0U)
#define I2S_RCR1_RFW(x)                          (((uint32_t)(((uint32_t)(x)) << I2S_RCR1_RFW_SHIFT)) & I2S_RCR1_RFW_MASK)

/*! @name RCR2 - SAI Receive Configuration 2 Register */
#define I2S_RCR2_DIV_MASK                        (0xFFU)
#define I2S_RCR2_DIV_SHIFT                       (0U)
#define I2S_RCR2_DIV(x)                          (((uint32_t)(((uint32_t)(x)) << I2S_RCR2_DIV_SHIFT)) & I2S_RCR2_DIV_MASK)
#define I2S_RCR2_BCD                             (0x1000000U)
#define I2S_RCR2_BCP                             (0x2000000U)
#define I2S_RCR2_MSEL_MASK                       (0xC000000U)
#define I2S_RCR2_MSEL_SHIFT                      (26U)
#define I2S_RCR2_MSEL(x)                         (((uint32_t)(((uint32_t)(x)) << I2S_RCR2_MSEL_SHIFT)) & I2S_RCR2_MSEL_MASK)
#define I2S_RCR2_BCI                             (0x10000000U)
#define I2S_RCR2_BCS                             (0x20000000U)
#define I2S_RCR2_SYNC_MASK                       (0xC0000000U)
#define I2S_RCR2_SYNC_SHIFT                      (30U)
#define I2S_RCR2_SYNC(x)                         (((uint32_t)(((uint32_t)(x)) << I2S_RCR2_SYNC_SHIFT)) & I2S_RCR2_SYNC_MASK)

/*! @name RCR3 - SAI Receive Configuration 3 Register */
#define I2S_RCR3_WDFL_MASK                       (0x1FU)
#define I2S_RCR3_WDFL_SHIFT                      (0U)
#define I2S_RCR3_WDFL(x)                         (((uint32_t)(((uint32_t)(x)) << I2S_RCR3_WDFL_SHIFT)) & I2S_RCR3_WDFL_MASK)
#define I2S_RCR3_RCE_MASK                        (0x30000U)
#define I2S_RCR3_RCE_SHIFT                       (16U)
#define I2S_RCR3_RCE(x)                          (((uint32_t)(((uint32_t)(x)) << I2S_RCR3_RCE_SHIFT)) & I2S_RCR3_RCE_MASK)

/*! @name RCR4 - SAI Receive Configuration 4 Register */
#define I2S_RCR4_FSD                             (0x1U)
#define I2S_RCR4_FSP                             (0x2U)
#define I2S_RCR4_FSE                             (0x8U)
#define I2S_RCR4_MF                              (0x10U)
#define I2S_RCR4_SYWD_MASK                       (0x1F00U)
#define I2S_RCR4_SYWD_SHIFT                      (8U)
#define I2S_RCR4_SYWD(x)                         (((uint32_t)(((uint32_t)(x)) << I2S_RCR4_SYWD_SHIFT)) & I2S_RCR4_SYWD_MASK)
#define I2S_RCR4_FRSZ_MASK                       (0x1F0000U)
#define I2S_RCR4_FRSZ_SHIFT                      (16U)
#define I2S_RCR4_FRSZ(x)                         (((uint32_t)(((uint32_t)(x)) << I2S_RCR4_FRSZ_SHIFT)) & I2S_RCR4_FRSZ_MASK)

/*! @name RCR5 - SAI Receive Configuration 5 Register */
#define I2S_RCR5_FBT_MASK                        (0x1F00U)
#define I2S_RCR5_FBT_SHIFT                       (8U)
#define I2S_RCR5_FBT(x)                          (((uint32_t)(((uint32_t)(x)) << I2S_RCR5_FBT_SHIFT)) & I2S_RCR5_FBT_MASK)
#define I2S_RCR5_W0W_MASK                        (0x1F0000U)
#define I2S_RCR5_W0W_SHIFT                       (16U)
#define I2S_RCR5_W0W(x)                          (((uint32_t)(((uint32_t)(x)) << I2S_RCR5_W0W_SHIFT)) & I2S_RCR5_W0W_MASK)
#define I2S_RCR5_WNW_MASK                        (0x1F000000U)
#define I2S_RCR5_WNW_SHIFT                       (24U)
#define I2S_RCR5_WNW(x)                          (((uint32_t)(((uint32_t)(x)) << I2S_RCR5_WNW_SHIFT)) & I2S_RCR5_WNW_MASK)

/* The count of I2S_RDR */
#define I2S_RDR_COUNT                            (2U)

/*! @name RFR - SAI Receive FIFO Register */
#define I2S_RFR_RFP_MASK                         (0xFU)
#define I2S_RFR_RFP_SHIFT                        (0U)
#define I2S_RFR_RFP(x)                           (((uint32_t)(((uint32_t)(x)) << I2S_RFR_RFP_SHIFT)) & I2S_RFR_RFP_MASK)
#define I2S_RFR_WFP_MASK                         (0xF0000U)
#define I2S_RFR_WFP_SHIFT                        (16U)
#define I2S_RFR_WFP(x)                           (((uint32_t)(((uint32_t)(x)) << I2S_RFR_WFP_SHIFT)) & I2S_RFR_WFP_MASK)

/* The count of I2S_RFR */
#define I2S_RFR_COUNT                            (2U)

/*! @name MCR - SAI MCLK Control Register */
#define I2S_MCR_MICS_MASK                        (0x3000000U)
#define I2S_MCR_MICS_SHIFT                       (24U)
#define I2S_MCR_MICS(x)                          (((uint32_t)(((uint32_t)(x)) << I2S_MCR_MICS_SHIFT)) & I2S_MCR_MICS_MASK)
#define I2S_MCR_MOE                              (0x40000000U)
#define I2S_MCR_DUF                              (0x80000000U)

/*! @name MDR - SAI MCLK Divide Register */
#define I2S_MDR_DIVIDE_MASK                      (0xFFFU)
#define I2S_MDR_DIVIDE_SHIFT                     (0U)
#define I2S_MDR_DIVIDE(x)                        (((uint32_t)(((uint32_t)(x)) << I2S_MDR_DIVIDE_SHIFT)) & I2S_MDR_DIVIDE_MASK)
#define I2S_MDR_FRACT_MASK                       (0xFF000U)
#define I2S_MDR_FRACT_SHIFT                      (12U)
#define I2S_MDR_FRACT(x)                         (((uint32_t)(((uint32_t)(x)) << I2S_MDR_FRACT_SHIFT)) & I2S_MDR_FRACT_MASK)


/*!
 * @}
 */ /* end of group I2S_Register_Masks */


/* I2S - Peripheral instance base addresses */
/** Peripheral I2S0 base address */
#define I2S0_BASE                                (0x4002F000u)
/** Peripheral I2S0 base pointer */
#define I2S0                                     ((I2S_TypeDef *)I2S0_BASE)
/** Array initializer of I2S peripheral base addresses */
#define I2S_BASE_ADDRS                           { I2S0_BASE }
/** Array initializer of I2S peripheral base pointers */
#define I2S_BASE_PTRS                            { I2S0 }
/** Interrupt vectors for the I2S peripheral type */
#define I2S_RX_IRQS                              { I2S0_Rx_IRQn }
#define I2S_TX_IRQS                              { I2S0_Tx_IRQn }

/*!
 * @}
 */ /* end of group I2S_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- LLWU Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup LLWU_Peripheral_Access_Layer LLWU Peripheral Access Layer
 * @{
 */

/** LLWU - Register Layout Typedef */
typedef struct {
  __IO uint8_t PE1;                                /**< LLWU Pin Enable 1 register, offset: 0x0 */
  __IO uint8_t PE2;                                /**< LLWU Pin Enable 2 register, offset: 0x1 */
  __IO uint8_t PE3;                                /**< LLWU Pin Enable 3 register, offset: 0x2 */
  __IO uint8_t PE4;                                /**< LLWU Pin Enable 4 register, offset: 0x3 */
  __IO uint8_t ME;                                 /**< LLWU Module Enable register, offset: 0x4 */
  __IO uint8_t F1;                                 /**< LLWU Flag 1 register, offset: 0x5 */
  __IO uint8_t F2;                                 /**< LLWU Flag 2 register, offset: 0x6 */
  __I  uint8_t F3;                                 /**< LLWU Flag 3 register, offset: 0x7 */
  __IO uint8_t FILT1;                              /**< LLWU Pin Filter 1 register, offset: 0x8 */
  __IO uint8_t FILT2;                              /**< LLWU Pin Filter 2 register, offset: 0x9 */
  __IO uint8_t RST;                                /**< LLWU Reset Enable register, offset: 0xA */
} LLWU_TypeDef;

/* ----------------------------------------------------------------------------
   -- LLWU Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup LLWU_Register_Masks LLWU Register Masks
 * @{
 */

/*! @name PE1 - LLWU Pin Enable 1 register */
#define LLWU_PE1_WUPE0_MASK                      (0x3U)
#define LLWU_PE1_WUPE0_SHIFT                     (0U)
#define LLWU_PE1_WUPE0(x)                        (((uint8_t)(((uint8_t)(x)) << LLWU_PE1_WUPE0_SHIFT)) & LLWU_PE1_WUPE0_MASK)
#define LLWU_PE1_WUPE1_MASK                      (0xCU)
#define LLWU_PE1_WUPE1_SHIFT                     (2U)
#define LLWU_PE1_WUPE1(x)                        (((uint8_t)(((uint8_t)(x)) << LLWU_PE1_WUPE1_SHIFT)) & LLWU_PE1_WUPE1_MASK)
#define LLWU_PE1_WUPE2_MASK                      (0x30U)
#define LLWU_PE1_WUPE2_SHIFT                     (4U)
#define LLWU_PE1_WUPE2(x)                        (((uint8_t)(((uint8_t)(x)) << LLWU_PE1_WUPE2_SHIFT)) & LLWU_PE1_WUPE2_MASK)
#define LLWU_PE1_WUPE3_MASK                      (0xC0U)
#define LLWU_PE1_WUPE3_SHIFT                     (6U)
#define LLWU_PE1_WUPE3(x)                        (((uint8_t)(((uint8_t)(x)) << LLWU_PE1_WUPE3_SHIFT)) & LLWU_PE1_WUPE3_MASK)

/*! @name PE2 - LLWU Pin Enable 2 register */
#define LLWU_PE2_WUPE4_MASK                      (0x3U)
#define LLWU_PE2_WUPE4_SHIFT                     (0U)
#define LLWU_PE2_WUPE4(x)                        (((uint8_t)(((uint8_t)(x)) << LLWU_PE2_WUPE4_SHIFT)) & LLWU_PE2_WUPE4_MASK)
#define LLWU_PE2_WUPE5_MASK                      (0xCU)
#define LLWU_PE2_WUPE5_SHIFT                     (2U)
#define LLWU_PE2_WUPE5(x)                        (((uint8_t)(((uint8_t)(x)) << LLWU_PE2_WUPE5_SHIFT)) & LLWU_PE2_WUPE5_MASK)
#define LLWU_PE2_WUPE6_MASK                      (0x30U)
#define LLWU_PE2_WUPE6_SHIFT                     (4U)
#define LLWU_PE2_WUPE6(x)                        (((uint8_t)(((uint8_t)(x)) << LLWU_PE2_WUPE6_SHIFT)) & LLWU_PE2_WUPE6_MASK)
#define LLWU_PE2_WUPE7_MASK                      (0xC0U)
#define LLWU_PE2_WUPE7_SHIFT                     (6U)
#define LLWU_PE2_WUPE7(x)                        (((uint8_t)(((uint8_t)(x)) << LLWU_PE2_WUPE7_SHIFT)) & LLWU_PE2_WUPE7_MASK)

/*! @name PE3 - LLWU Pin Enable 3 register */
#define LLWU_PE3_WUPE8_MASK                      (0x3U)
#define LLWU_PE3_WUPE8_SHIFT                     (0U)
#define LLWU_PE3_WUPE8(x)                        (((uint8_t)(((uint8_t)(x)) << LLWU_PE3_WUPE8_SHIFT)) & LLWU_PE3_WUPE8_MASK)
#define LLWU_PE3_WUPE9_MASK                      (0xCU)
#define LLWU_PE3_WUPE9_SHIFT                     (2U)
#define LLWU_PE3_WUPE9(x)                        (((uint8_t)(((uint8_t)(x)) << LLWU_PE3_WUPE9_SHIFT)) & LLWU_PE3_WUPE9_MASK)
#define LLWU_PE3_WUPE10_MASK                     (0x30U)
#define LLWU_PE3_WUPE10_SHIFT                    (4U)
#define LLWU_PE3_WUPE10(x)                       (((uint8_t)(((uint8_t)(x)) << LLWU_PE3_WUPE10_SHIFT)) & LLWU_PE3_WUPE10_MASK)
#define LLWU_PE3_WUPE11_MASK                     (0xC0U)
#define LLWU_PE3_WUPE11_SHIFT                    (6U)
#define LLWU_PE3_WUPE11(x)                       (((uint8_t)(((uint8_t)(x)) << LLWU_PE3_WUPE11_SHIFT)) & LLWU_PE3_WUPE11_MASK)

/*! @name PE4 - LLWU Pin Enable 4 register */
#define LLWU_PE4_WUPE12_MASK                     (0x3U)
#define LLWU_PE4_WUPE12_SHIFT                    (0U)
#define LLWU_PE4_WUPE12(x)                       (((uint8_t)(((uint8_t)(x)) << LLWU_PE4_WUPE12_SHIFT)) & LLWU_PE4_WUPE12_MASK)
#define LLWU_PE4_WUPE13_MASK                     (0xCU)
#define LLWU_PE4_WUPE13_SHIFT                    (2U)
#define LLWU_PE4_WUPE13(x)                       (((uint8_t)(((uint8_t)(x)) << LLWU_PE4_WUPE13_SHIFT)) & LLWU_PE4_WUPE13_MASK)
#define LLWU_PE4_WUPE14_MASK                     (0x30U)
#define LLWU_PE4_WUPE14_SHIFT                    (4U)
#define LLWU_PE4_WUPE14(x)                       (((uint8_t)(((uint8_t)(x)) << LLWU_PE4_WUPE14_SHIFT)) & LLWU_PE4_WUPE14_MASK)
#define LLWU_PE4_WUPE15_MASK                     (0xC0U)
#define LLWU_PE4_WUPE15_SHIFT                    (6U)
#define LLWU_PE4_WUPE15(x)                       (((uint8_t)(((uint8_t)(x)) << LLWU_PE4_WUPE15_SHIFT)) & LLWU_PE4_WUPE15_MASK)

/*! @name ME - LLWU Module Enable register */
#define LLWU_ME_WUME0                            (0x1U)
#define LLWU_ME_WUME1                            (0x2U)
#define LLWU_ME_WUME2                            (0x4U)
#define LLWU_ME_WUME3                            (0x8U)
#define LLWU_ME_WUME4                            (0x10U)
#define LLWU_ME_WUME5                            (0x20U)
#define LLWU_ME_WUME6                            (0x40U)
#define LLWU_ME_WUME7                            (0x80U)

/*! @name F1 - LLWU Flag 1 register */
#define LLWU_F1_WUF0                             (0x1U)
#define LLWU_F1_WUF1                             (0x2U)
#define LLWU_F1_WUF2                             (0x4U)
#define LLWU_F1_WUF3                             (0x8U)
#define LLWU_F1_WUF4                             (0x10U)
#define LLWU_F1_WUF5                             (0x20U)
#define LLWU_F1_WUF6                             (0x40U)
#define LLWU_F1_WUF7                             (0x80U)

/*! @name F2 - LLWU Flag 2 register */
#define LLWU_F2_WUF8                             (0x1U)
#define LLWU_F2_WUF9                             (0x2U)
#define LLWU_F2_WUF10                            (0x4U)
#define LLWU_F2_WUF11                            (0x8U)
#define LLWU_F2_WUF12                            (0x10U)
#define LLWU_F2_WUF13                            (0x20U)
#define LLWU_F2_WUF14                            (0x40U)
#define LLWU_F2_WUF15                            (0x80U)

/*! @name F3 - LLWU Flag 3 register */
#define LLWU_F3_MWUF0                            (0x1U)
#define LLWU_F3_MWUF1                            (0x2U)
#define LLWU_F3_MWUF2                            (0x4U)
#define LLWU_F3_MWUF3                            (0x8U)
#define LLWU_F3_MWUF4                            (0x10U)
#define LLWU_F3_MWUF5                            (0x20U)
#define LLWU_F3_MWUF6                            (0x40U)
#define LLWU_F3_MWUF7                            (0x80U)

/*! @name FILT1 - LLWU Pin Filter 1 register */
#define LLWU_FILT1_FILTSEL_MASK                  (0xFU)
#define LLWU_FILT1_FILTSEL_SHIFT                 (0U)
#define LLWU_FILT1_FILTSEL(x)                    (((uint8_t)(((uint8_t)(x)) << LLWU_FILT1_FILTSEL_SHIFT)) & LLWU_FILT1_FILTSEL_MASK)
#define LLWU_FILT1_FILTE_MASK                    (0x60U)
#define LLWU_FILT1_FILTE_SHIFT                   (5U)
#define LLWU_FILT1_FILTE(x)                      (((uint8_t)(((uint8_t)(x)) << LLWU_FILT1_FILTE_SHIFT)) & LLWU_FILT1_FILTE_MASK)
#define LLWU_FILT1_FILTF                         (0x80U)

/*! @name FILT2 - LLWU Pin Filter 2 register */
#define LLWU_FILT2_FILTSEL_MASK                  (0xFU)
#define LLWU_FILT2_FILTSEL_SHIFT                 (0U)
#define LLWU_FILT2_FILTSEL(x)                    (((uint8_t)(((uint8_t)(x)) << LLWU_FILT2_FILTSEL_SHIFT)) & LLWU_FILT2_FILTSEL_MASK)
#define LLWU_FILT2_FILTE_MASK                    (0x60U)
#define LLWU_FILT2_FILTE_SHIFT                   (5U)
#define LLWU_FILT2_FILTE(x)                      (((uint8_t)(((uint8_t)(x)) << LLWU_FILT2_FILTE_SHIFT)) & LLWU_FILT2_FILTE_MASK)
#define LLWU_FILT2_FILTF                         (0x80U)

/*! @name RST - LLWU Reset Enable register */
#define LLWU_RST_RSTFILT                         (0x1U)
#define LLWU_RST_LLRSTE                          (0x2U)


/*!
 * @}
 */ /* end of group LLWU_Register_Masks */


/* LLWU - Peripheral instance base addresses */
/** Peripheral LLWU base address */
#define LLWU_BASE                                (0x4007C000u)
/** Peripheral LLWU base pointer */
#define LLWU                                     ((LLWU_TypeDef *)LLWU_BASE)
/** Array initializer of LLWU peripheral base addresses */
#define LLWU_BASE_ADDRS                          { LLWU_BASE }
/** Array initializer of LLWU peripheral base pointers */
#define LLWU_BASE_PTRS                           { LLWU }
/** Interrupt vectors for the LLWU peripheral type */
#define LLWU_IRQS                                { LLWU_IRQn }

/*!
 * @}
 */ /* end of group LLWU_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- LPTMR Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup LPTMR_Peripheral_Access_Layer LPTMR Peripheral Access Layer
 * @{
 */

/** LPTMR - Register Layout Typedef */
typedef struct {
  __IO uint32_t CSR;                               /**< Low Power Timer Control Status Register, offset: 0x0 */
  __IO uint32_t PSR;                               /**< Low Power Timer Prescale Register, offset: 0x4 */
  __IO uint32_t CMR;                               /**< Low Power Timer Compare Register, offset: 0x8 */
  __IO uint32_t CNR;                               /**< Low Power Timer Counter Register, offset: 0xC */
} LPTMR_TypeDef;

/* ----------------------------------------------------------------------------
   -- LPTMR Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup LPTMR_Register_Masks LPTMR Register Masks
 * @{
 */

/*! @name CSR - Low Power Timer Control Status Register */
#define LPTMR_CSR_TEN                            (0x1U)
#define LPTMR_CSR_TMS                            (0x2U)
#define LPTMR_CSR_TFC                            (0x4U)
#define LPTMR_CSR_TPP                            (0x8U)
#define LPTMR_CSR_TPS_MASK                       (0x30U)
#define LPTMR_CSR_TPS_SHIFT                      (4U)
#define LPTMR_CSR_TPS(x)                         (((uint32_t)(((uint32_t)(x)) << LPTMR_CSR_TPS_SHIFT)) & LPTMR_CSR_TPS_MASK)
#define LPTMR_CSR_TIE                            (0x40U)
#define LPTMR_CSR_TCF                            (0x80U)

/*! @name PSR - Low Power Timer Prescale Register */
#define LPTMR_PSR_PCS_MASK                       (0x3U)
#define LPTMR_PSR_PCS_SHIFT                      (0U)
#define LPTMR_PSR_PCS(x)                         (((uint32_t)(((uint32_t)(x)) << LPTMR_PSR_PCS_SHIFT)) & LPTMR_PSR_PCS_MASK)
#define LPTMR_PSR_PBYP                           (0x4U)
#define LPTMR_PSR_PRESCALE_MASK                  (0x78U)
#define LPTMR_PSR_PRESCALE_SHIFT                 (3U)
#define LPTMR_PSR_PRESCALE(x)                    (((uint32_t)(((uint32_t)(x)) << LPTMR_PSR_PRESCALE_SHIFT)) & LPTMR_PSR_PRESCALE_MASK)

/*! @name CMR - Low Power Timer Compare Register */
#define LPTMR_CMR_COMPARE_MASK                   (0xFFFFU)
#define LPTMR_CMR_COMPARE_SHIFT                  (0U)
#define LPTMR_CMR_COMPARE(x)                     (((uint32_t)(((uint32_t)(x)) << LPTMR_CMR_COMPARE_SHIFT)) & LPTMR_CMR_COMPARE_MASK)

/*! @name CNR - Low Power Timer Counter Register */
#define LPTMR_CNR_COUNTER_MASK                   (0xFFFFU)
#define LPTMR_CNR_COUNTER_SHIFT                  (0U)
#define LPTMR_CNR_COUNTER(x)                     (((uint32_t)(((uint32_t)(x)) << LPTMR_CNR_COUNTER_SHIFT)) & LPTMR_CNR_COUNTER_MASK)


/*!
 * @}
 */ /* end of group LPTMR_Register_Masks */


/* LPTMR - Peripheral instance base addresses */
/** Peripheral LPTMR0 base address */
#define LPTMR0_BASE                              (0x40040000u)
/** Peripheral LPTMR0 base pointer */
#define LPTMR0                                   ((LPTMR_TypeDef *)LPTMR0_BASE)
/** Array initializer of LPTMR peripheral base addresses */
#define LPTMR_BASE_ADDRS                         { LPTMR0_BASE }
/** Array initializer of LPTMR peripheral base pointers */
#define LPTMR_BASE_PTRS                          { LPTMR0 }
/** Interrupt vectors for the LPTMR peripheral type */
#define LPTMR_IRQS                               { LPTMR0_IRQn }

/*!
 * @}
 */ /* end of group LPTMR_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- MCG Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup MCG_Peripheral_Access_Layer MCG Peripheral Access Layer
 * @{
 */

/** MCG - Register Layout Typedef */
typedef struct {
  __IO uint8_t C1;                                 /**< MCG Control 1 Register, offset: 0x0 */
  __IO uint8_t C2;                                 /**< MCG Control 2 Register, offset: 0x1 */
  __IO uint8_t C3;                                 /**< MCG Control 3 Register, offset: 0x2 */
  __IO uint8_t C4;                                 /**< MCG Control 4 Register, offset: 0x3 */
  __IO uint8_t C5;                                 /**< MCG Control 5 Register, offset: 0x4 */
  __IO uint8_t C6;                                 /**< MCG Control 6 Register, offset: 0x5 */
  __IO uint8_t S;                                  /**< MCG Status Register, offset: 0x6 */
       uint8_t RESERVED_0[1];
  __IO uint8_t SC;                                 /**< MCG Status and Control Register, offset: 0x8 */
       uint8_t RESERVED_1[1];
  __IO uint8_t ATCVH;                              /**< MCG Auto Trim Compare Value High Register, offset: 0xA */
  __IO uint8_t ATCVL;                              /**< MCG Auto Trim Compare Value Low Register, offset: 0xB */
  __IO uint8_t C7;                                 /**< MCG Control 7 Register, offset: 0xC */
  __IO uint8_t C8;                                 /**< MCG Control 8 Register, offset: 0xD */
} MCG_TypeDef;

/* ----------------------------------------------------------------------------
   -- MCG Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup MCG_Register_Masks MCG Register Masks
 * @{
 */

/*! @name C1 - MCG Control 1 Register */
#define MCG_C1_IREFSTEN                          (0x1U)
#define MCG_C1_IRCLKEN                           (0x2U)
#define MCG_C1_IREFS                             (0x4U)
#define MCG_C1_FRDIV_MASK                        (0x38U)
#define MCG_C1_FRDIV_SHIFT                       (3U)
#define MCG_C1_FRDIV(x)                          (((uint8_t)(((uint8_t)(x)) << MCG_C1_FRDIV_SHIFT)) & MCG_C1_FRDIV_MASK)
#define MCG_C1_CLKS_MASK                         (0xC0U)
#define MCG_C1_CLKS_SHIFT                        (6U)
#define MCG_C1_CLKS(x)                           (((uint8_t)(((uint8_t)(x)) << MCG_C1_CLKS_SHIFT)) & MCG_C1_CLKS_MASK)

/*! @name C2 - MCG Control 2 Register */
#define MCG_C2_IRCS                              (0x1U)
#define MCG_C2_LP                                (0x2U)
#define MCG_C2_EREFS                             (0x4U)
#define MCG_C2_EREFS0                            MCG_C2_EREFS
#define MCG_C2_HGO                               (0x8U)
#define MCG_C2_RANGE_MASK                        (0x30U)
#define MCG_C2_RANGE_SHIFT                       (4U)
#define MCG_C2_RANGE(x)                          (((uint8_t)(((uint8_t)(x)) << MCG_C2_RANGE_SHIFT)) & MCG_C2_RANGE_MASK)
#define MCG_C2_FCFTRIM                           (0x40U)
#define MCG_C2_LOCRE0                            (0x80U)

/*! @name C3 - MCG Control 3 Register */
#define MCG_C3_SCTRIM_MASK                       (0xFFU)
#define MCG_C3_SCTRIM_SHIFT                      (0U)
#define MCG_C3_SCTRIM(x)                         (((uint8_t)(((uint8_t)(x)) << MCG_C3_SCTRIM_SHIFT)) & MCG_C3_SCTRIM_MASK)

/*! @name C4 - MCG Control 4 Register */
#define MCG_C4_SCFTRIM                           (0x1U)
#define MCG_C4_FCTRIM_MASK                       (0x1EU)
#define MCG_C4_FCTRIM_SHIFT                      (1U)
#define MCG_C4_FCTRIM(x)                         (((uint8_t)(((uint8_t)(x)) << MCG_C4_FCTRIM_SHIFT)) & MCG_C4_FCTRIM_MASK)
#define MCG_C4_DRST_DRS_MASK                     (0x60U)
#define MCG_C4_DRST_DRS_SHIFT                    (5U)
#define MCG_C4_DRST_DRS(x)                       (((uint8_t)(((uint8_t)(x)) << MCG_C4_DRST_DRS_SHIFT)) & MCG_C4_DRST_DRS_MASK)
#define MCG_C4_DMX32                             (0x80U)

/*! @name C5 - MCG Control 5 Register */
#define MCG_C5_PRDIV0_MASK                       (0x1FU)
#define MCG_C5_PRDIV0_SHIFT                      (0U)
#define MCG_C5_PRDIV0(x)                         (((uint8_t)(((uint8_t)(x)) << MCG_C5_PRDIV0_SHIFT)) & MCG_C5_PRDIV0_MASK)
#define MCG_C5_PLLSTEN0                          (0x20U)
#define MCG_C5_PLLCLKEN0                         (0x40U)

/*! @name C6 - MCG Control 6 Register */
#define MCG_C6_VDIV0_MASK                        (0x1FU)
#define MCG_C6_VDIV0_SHIFT                       (0U)
#define MCG_C6_VDIV0(x)                          (((uint8_t)(((uint8_t)(x)) << MCG_C6_VDIV0_SHIFT)) & MCG_C6_VDIV0_MASK)
#define MCG_C6_CME0                              (0x20U)
#define MCG_C6_PLLS                              (0x40U)
#define MCG_C6_LOLIE0                            (0x80U)

/*! @name S - MCG Status Register */
#define MCG_S_IRCST                              (0x1U)
#define MCG_S_OSCINIT0                           (0x2U)
#define MCG_S_CLKST_MASK                         (0xCU)
#define MCG_S_CLKST_SHIFT                        (2U)
#define MCG_S_CLKST(x)                           (((uint8_t)(((uint8_t)(x)) << MCG_S_CLKST_SHIFT)) & MCG_S_CLKST_MASK)
#define MCG_S_CLKST_FLL                          MCG_S_CLKST(0)
#define MCG_S_CLKST_INT                          MCG_S_CLKST(1)
#define MCG_S_CLKST_EXT                          MCG_S_CLKST(2)
#define MCG_S_CLKST_PLL                          MCG_S_CLKST(3)
#define MCG_S_IREFST                             (0x10U)
#define MCG_S_PLLST                              (0x20U)
#define MCG_S_LOCK0                              (0x40U)
#define MCG_S_LOLS0                              (0x80U)

/*! @name SC - MCG Status and Control Register */
#define MCG_SC_LOCS0                             (0x1U)
#define MCG_SC_FCRDIV_MASK                       (0xEU)
#define MCG_SC_FCRDIV_SHIFT                      (1U)
#define MCG_SC_FCRDIV(x)                         (((uint8_t)(((uint8_t)(x)) << MCG_SC_FCRDIV_SHIFT)) & MCG_SC_FCRDIV_MASK)
#define MCG_SC_FLTPRSRV                          (0x10U)
#define MCG_SC_ATMF                              (0x20U)
#define MCG_SC_ATMS                              (0x40U)
#define MCG_SC_ATME                              (0x80U)

/*! @name ATCVH - MCG Auto Trim Compare Value High Register */
#define MCG_ATCVH_ATCVH_MASK                     (0xFFU)
#define MCG_ATCVH_ATCVH_SHIFT                    (0U)
#define MCG_ATCVH_ATCVH(x)                       (((uint8_t)(((uint8_t)(x)) << MCG_ATCVH_ATCVH_SHIFT)) & MCG_ATCVH_ATCVH_MASK)

/*! @name ATCVL - MCG Auto Trim Compare Value Low Register */
#define MCG_ATCVL_ATCVL_MASK                     (0xFFU)
#define MCG_ATCVL_ATCVL_SHIFT                    (0U)
#define MCG_ATCVL_ATCVL(x)                       (((uint8_t)(((uint8_t)(x)) << MCG_ATCVL_ATCVL_SHIFT)) & MCG_ATCVL_ATCVL_MASK)

/*! @name C7 - MCG Control 7 Register */
#define MCG_C7_OSCSEL_MASK                       (0x3U)
#define MCG_C7_OSCSEL_SHIFT                      (0U)
#define MCG_C7_OSCSEL(x)                         (((uint8_t)(((uint8_t)(x)) << MCG_C7_OSCSEL_SHIFT)) & MCG_C7_OSCSEL_MASK)

/*! @name C8 - MCG Control 8 Register */
#define MCG_C8_LOCS1                             (0x1U)
#define MCG_C8_CME1                              (0x20U)
#define MCG_C8_LOLRE                             (0x40U)
#define MCG_C8_LOCRE1                            (0x80U)


/*!
 * @}
 */ /* end of group MCG_Register_Masks */


/* MCG - Peripheral instance base addresses */
/** Peripheral MCG base address */
#define MCG_BASE                                 (0x40064000u)
/** Peripheral MCG base pointer */
#define MCG                                      ((MCG_TypeDef *)MCG_BASE)
/** Array initializer of MCG peripheral base addresses */
#define MCG_BASE_ADDRS                           { MCG_BASE }
/** Array initializer of MCG peripheral base pointers */
#define MCG_BASE_PTRS                            { MCG }

/*!
 * @}
 */ /* end of group MCG_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- MCM Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup MCM_Peripheral_Access_Layer MCM Peripheral Access Layer
 * @{
 */

/** MCM - Register Layout Typedef */
typedef struct {
       uint8_t RESERVED_0[8];
  __I  uint16_t PLASC;                             /**< Crossbar Switch (AXBS) Slave Configuration, offset: 0x8 */
  __I  uint16_t PLAMC;                             /**< Crossbar Switch (AXBS) Master Configuration, offset: 0xA */
  __IO uint32_t CR;                                /**< Control Register, offset: 0xC */
  __IO uint32_t ISCR;                              /**< Interrupt Status Register, offset: 0x10 */
  __IO uint32_t ETBCC;                             /**< ETB Counter Control register, offset: 0x14 */
  __IO uint32_t ETBRL;                             /**< ETB Reload register, offset: 0x18 */
  __I  uint32_t ETBCNT;                            /**< ETB Counter Value register, offset: 0x1C */
       uint8_t RESERVED_1[16];
  __IO uint32_t PID;                               /**< Process ID register, offset: 0x30 */
} MCM_TypeDef;

/* ----------------------------------------------------------------------------
   -- MCM Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup MCM_Register_Masks MCM Register Masks
 * @{
 */

/*! @name PLASC - Crossbar Switch (AXBS) Slave Configuration */
#define MCM_PLASC_ASC_MASK                       (0xFFU)
#define MCM_PLASC_ASC_SHIFT                      (0U)
#define MCM_PLASC_ASC(x)                         (((uint16_t)(((uint16_t)(x)) << MCM_PLASC_ASC_SHIFT)) & MCM_PLASC_ASC_MASK)

/*! @name PLAMC - Crossbar Switch (AXBS) Master Configuration */
#define MCM_PLAMC_AMC_MASK                       (0xFFU)
#define MCM_PLAMC_AMC_SHIFT                      (0U)
#define MCM_PLAMC_AMC(x)                         (((uint16_t)(((uint16_t)(x)) << MCM_PLAMC_AMC_SHIFT)) & MCM_PLAMC_AMC_MASK)

/*! @name CR - Control Register */
#define MCM_CR_SRAMUAP_MASK                      (0x3000000U)
#define MCM_CR_SRAMUAP_SHIFT                     (24U)
#define MCM_CR_SRAMUAP(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_CR_SRAMUAP_SHIFT)) & MCM_CR_SRAMUAP_MASK)
#define MCM_CR_SRAMUWP                           (0x4000000U)
#define MCM_CR_SRAMLAP_MASK                      (0x30000000U)
#define MCM_CR_SRAMLAP_SHIFT                     (28U)
#define MCM_CR_SRAMLAP(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_CR_SRAMLAP_SHIFT)) & MCM_CR_SRAMLAP_MASK)
#define MCM_CR_SRAMLWP                           (0x40000000U)

/*! @name ISCR - Interrupt Status Register */
#define MCM_ISCR_IRQ                             (0x2U)
#define MCM_ISCR_NMI                             (0x4U)
#define MCM_ISCR_DHREQ                           (0x8U)
#define MCM_ISCR_FIOC                            (0x100U)
#define MCM_ISCR_FDZC                            (0x200U)
#define MCM_ISCR_FOFC                            (0x400U)
#define MCM_ISCR_FUFC                            (0x800U)
#define MCM_ISCR_FIXC                            (0x1000U)
#define MCM_ISCR_FIDC                            (0x8000U)
#define MCM_ISCR_FIOCE                           (0x1000000U)
#define MCM_ISCR_FDZCE                           (0x2000000U)
#define MCM_ISCR_FOFCE                           (0x4000000U)
#define MCM_ISCR_FUFCE                           (0x8000000U)
#define MCM_ISCR_FIXCE                           (0x10000000U)
#define MCM_ISCR_FIDCE                           (0x80000000U)

/*! @name ETBCC - ETB Counter Control register */
#define MCM_ETBCC_CNTEN                          (0x1U)
#define MCM_ETBCC_RSPT_MASK                      (0x6U)
#define MCM_ETBCC_RSPT_SHIFT                     (1U)
#define MCM_ETBCC_RSPT(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_ETBCC_RSPT_SHIFT)) & MCM_ETBCC_RSPT_MASK)
#define MCM_ETBCC_RLRQ                           (0x8U)
#define MCM_ETBCC_ETDIS                          (0x10U)
#define MCM_ETBCC_ITDIS                          (0x20U)

/*! @name ETBRL - ETB Reload register */
#define MCM_ETBRL_RELOAD_MASK                    (0x7FFU)
#define MCM_ETBRL_RELOAD_SHIFT                   (0U)
#define MCM_ETBRL_RELOAD(x)                      (((uint32_t)(((uint32_t)(x)) << MCM_ETBRL_RELOAD_SHIFT)) & MCM_ETBRL_RELOAD_MASK)

/*! @name ETBCNT - ETB Counter Value register */
#define MCM_ETBCNT_COUNTER_MASK                  (0x7FFU)
#define MCM_ETBCNT_COUNTER_SHIFT                 (0U)
#define MCM_ETBCNT_COUNTER(x)                    (((uint32_t)(((uint32_t)(x)) << MCM_ETBCNT_COUNTER_SHIFT)) & MCM_ETBCNT_COUNTER_MASK)

/*! @name PID - Process ID register */
#define MCM_PID_PID_MASK                         (0xFFU)
#define MCM_PID_PID_SHIFT                        (0U)
#define MCM_PID_PID(x)                           (((uint32_t)(((uint32_t)(x)) << MCM_PID_PID_SHIFT)) & MCM_PID_PID_MASK)


/*!
 * @}
 */ /* end of group MCM_Register_Masks */


/* MCM - Peripheral instance base addresses */
/** Peripheral MCM base address */
#define MCM_BASE                                 (0xE0080000u)
/** Peripheral MCM base pointer */
#define MCM                                      ((MCM_TypeDef *)MCM_BASE)
/** Array initializer of MCM peripheral base addresses */
#define MCM_BASE_ADDRS                           { MCM_BASE }
/** Array initializer of MCM peripheral base pointers */
#define MCM_BASE_PTRS                            { MCM }
/** Interrupt vectors for the MCM peripheral type */
#define MCM_IRQS                                 { MCM_IRQn }

/*!
 * @}
 */ /* end of group MCM_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- MPU Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup MPU_Peripheral_Access_Layer MPU Peripheral Access Layer
 * @{
 */

/** MPU - Register Layout Typedef */
typedef struct {
  __IO uint32_t CESR;                              /**< Control/Error Status Register, offset: 0x0 */
       uint8_t RESERVED_0[12];
  struct {                                         /* offset: 0x10, array step: 0x8 */
    __I  uint32_t EAR;                               /**< Error Address Register, slave port n, array offset: 0x10, array step: 0x8 */
    __I  uint32_t EDR;                               /**< Error Detail Register, slave port n, array offset: 0x14, array step: 0x8 */
  } SP[5];
       uint8_t RESERVED_1[968];
  __IO uint32_t WORD[12][4];                       /**< Region Descriptor n, Word 0..Region Descriptor n, Word 3, array offset: 0x400, array step: index*0x10, index2*0x4 */
       uint8_t RESERVED_2[832];
  __IO uint32_t RGDAAC[12];                        /**< Region Descriptor Alternate Access Control n, array offset: 0x800, array step: 0x4 */
} MPU_TypeDef;

/* ----------------------------------------------------------------------------
   -- MPU Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup MPU_Register_Masks MPU Register Masks
 * @{
 */

/*! @name CESR - Control/Error Status Register */
#define MPU_CESR_VLD                             (0x1U)
#define MPU_CESR_NRGD_MASK                       (0xF00U)
#define MPU_CESR_NRGD_SHIFT                      (8U)
#define MPU_CESR_NRGD(x)                         (((uint32_t)(((uint32_t)(x)) << MPU_CESR_NRGD_SHIFT)) & MPU_CESR_NRGD_MASK)
#define MPU_CESR_NSP_MASK                        (0xF000U)
#define MPU_CESR_NSP_SHIFT                       (12U)
#define MPU_CESR_NSP(x)                          (((uint32_t)(((uint32_t)(x)) << MPU_CESR_NSP_SHIFT)) & MPU_CESR_NSP_MASK)
#define MPU_CESR_HRL_MASK                        (0xF0000U)
#define MPU_CESR_HRL_SHIFT                       (16U)
#define MPU_CESR_HRL(x)                          (((uint32_t)(((uint32_t)(x)) << MPU_CESR_HRL_SHIFT)) & MPU_CESR_HRL_MASK)
#define MPU_CESR_SPERR_MASK                      (0xF8000000U)
#define MPU_CESR_SPERR_SHIFT                     (27U)
#define MPU_CESR_SPERR(x)                        (((uint32_t)(((uint32_t)(x)) << MPU_CESR_SPERR_SHIFT)) & MPU_CESR_SPERR_MASK)

/* The count of MPU_EAR */
#define MPU_EAR_COUNT                            (5U)

/*! @name EDR - Error Detail Register, slave port n */
#define MPU_EDR_ERW                              (0x1U)
#define MPU_EDR_EATTR_MASK                       (0xEU)
#define MPU_EDR_EATTR_SHIFT                      (1U)
#define MPU_EDR_EATTR(x)                         (((uint32_t)(((uint32_t)(x)) << MPU_EDR_EATTR_SHIFT)) & MPU_EDR_EATTR_MASK)
#define MPU_EDR_EMN_MASK                         (0xF0U)
#define MPU_EDR_EMN_SHIFT                        (4U)
#define MPU_EDR_EMN(x)                           (((uint32_t)(((uint32_t)(x)) << MPU_EDR_EMN_SHIFT)) & MPU_EDR_EMN_MASK)
#define MPU_EDR_EPID_MASK                        (0xFF00U)
#define MPU_EDR_EPID_SHIFT                       (8U)
#define MPU_EDR_EPID(x)                          (((uint32_t)(((uint32_t)(x)) << MPU_EDR_EPID_SHIFT)) & MPU_EDR_EPID_MASK)
#define MPU_EDR_EACD_MASK                        (0xFFFF0000U)
#define MPU_EDR_EACD_SHIFT                       (16U)
#define MPU_EDR_EACD(x)                          (((uint32_t)(((uint32_t)(x)) << MPU_EDR_EACD_SHIFT)) & MPU_EDR_EACD_MASK)

/* The count of MPU_EDR */
#define MPU_EDR_COUNT                            (5U)

/*! @name WORD - Region Descriptor n, Word 0..Region Descriptor n, Word 3 */
#define MPU_WORD_VLD                             (0x1U)
#define MPU_WORD_M0UM_MASK                       (0x7U)
#define MPU_WORD_M0UM_SHIFT                      (0U)
#define MPU_WORD_M0UM(x)                         (((uint32_t)(((uint32_t)(x)) << MPU_WORD_M0UM_SHIFT)) & MPU_WORD_M0UM_MASK)
#define MPU_WORD_M0SM_MASK                       (0x18U)
#define MPU_WORD_M0SM_SHIFT                      (3U)
#define MPU_WORD_M0SM(x)                         (((uint32_t)(((uint32_t)(x)) << MPU_WORD_M0SM_SHIFT)) & MPU_WORD_M0SM_MASK)
#define MPU_WORD_M0PE                            (0x20U)
#define MPU_WORD_ENDADDR_MASK                    (0xFFFFFFE0U)
#define MPU_WORD_ENDADDR_SHIFT                   (5U)
#define MPU_WORD_ENDADDR(x)                      (((uint32_t)(((uint32_t)(x)) << MPU_WORD_ENDADDR_SHIFT)) & MPU_WORD_ENDADDR_MASK)
#define MPU_WORD_SRTADDR_MASK                    (0xFFFFFFE0U)
#define MPU_WORD_SRTADDR_SHIFT                   (5U)
#define MPU_WORD_SRTADDR(x)                      (((uint32_t)(((uint32_t)(x)) << MPU_WORD_SRTADDR_SHIFT)) & MPU_WORD_SRTADDR_MASK)
#define MPU_WORD_M1UM_MASK                       (0x1C0U)
#define MPU_WORD_M1UM_SHIFT                      (6U)
#define MPU_WORD_M1UM(x)                         (((uint32_t)(((uint32_t)(x)) << MPU_WORD_M1UM_SHIFT)) & MPU_WORD_M1UM_MASK)
#define MPU_WORD_M1SM_MASK                       (0x600U)
#define MPU_WORD_M1SM_SHIFT                      (9U)
#define MPU_WORD_M1SM(x)                         (((uint32_t)(((uint32_t)(x)) << MPU_WORD_M1SM_SHIFT)) & MPU_WORD_M1SM_MASK)
#define MPU_WORD_M1PE                            (0x800U)
#define MPU_WORD_M2UM_MASK                       (0x7000U)
#define MPU_WORD_M2UM_SHIFT                      (12U)
#define MPU_WORD_M2UM(x)                         (((uint32_t)(((uint32_t)(x)) << MPU_WORD_M2UM_SHIFT)) & MPU_WORD_M2UM_MASK)
#define MPU_WORD_M2SM_MASK                       (0x18000U)
#define MPU_WORD_M2SM_SHIFT                      (15U)
#define MPU_WORD_M2SM(x)                         (((uint32_t)(((uint32_t)(x)) << MPU_WORD_M2SM_SHIFT)) & MPU_WORD_M2SM_MASK)
#define MPU_WORD_PIDMASK_MASK                    (0xFF0000U)
#define MPU_WORD_PIDMASK_SHIFT                   (16U)
#define MPU_WORD_PIDMASK(x)                      (((uint32_t)(((uint32_t)(x)) << MPU_WORD_PIDMASK_SHIFT)) & MPU_WORD_PIDMASK_MASK)
#define MPU_WORD_M2PE                            (0x20000U)
#define MPU_WORD_M3UM_MASK                       (0x1C0000U)
#define MPU_WORD_M3UM_SHIFT                      (18U)
#define MPU_WORD_M3UM(x)                         (((uint32_t)(((uint32_t)(x)) << MPU_WORD_M3UM_SHIFT)) & MPU_WORD_M3UM_MASK)
#define MPU_WORD_M3SM_MASK                       (0x600000U)
#define MPU_WORD_M3SM_SHIFT                      (21U)
#define MPU_WORD_M3SM(x)                         (((uint32_t)(((uint32_t)(x)) << MPU_WORD_M3SM_SHIFT)) & MPU_WORD_M3SM_MASK)
#define MPU_WORD_M3PE                            (0x800000U)
#define MPU_WORD_PID_MASK                        (0xFF000000U)
#define MPU_WORD_PID_SHIFT                       (24U)
#define MPU_WORD_PID(x)                          (((uint32_t)(((uint32_t)(x)) << MPU_WORD_PID_SHIFT)) & MPU_WORD_PID_MASK)
#define MPU_WORD_M4WE                            (0x1000000U)
#define MPU_WORD_M4RE                            (0x2000000U)
#define MPU_WORD_M5WE                            (0x4000000U)
#define MPU_WORD_M5RE                            (0x8000000U)
#define MPU_WORD_M6WE                            (0x10000000U)
#define MPU_WORD_M6RE                            (0x20000000U)
#define MPU_WORD_M7WE                            (0x40000000U)
#define MPU_WORD_M7RE                            (0x80000000U)

/* The count of MPU_WORD */
#define MPU_WORD_COUNT                           (12U)

/* The count of MPU_WORD */
#define MPU_WORD_COUNT2                          (4U)

/*! @name RGDAAC - Region Descriptor Alternate Access Control n */
#define MPU_RGDAAC_M0UM_MASK                     (0x7U)
#define MPU_RGDAAC_M0UM_SHIFT                    (0U)
#define MPU_RGDAAC_M0UM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M0UM_SHIFT)) & MPU_RGDAAC_M0UM_MASK)
#define MPU_RGDAAC_M0SM_MASK                     (0x18U)
#define MPU_RGDAAC_M0SM_SHIFT                    (3U)
#define MPU_RGDAAC_M0SM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M0SM_SHIFT)) & MPU_RGDAAC_M0SM_MASK)
#define MPU_RGDAAC_M0PE                          (0x20U)
#define MPU_RGDAAC_M1UM_MASK                     (0x1C0U)
#define MPU_RGDAAC_M1UM_SHIFT                    (6U)
#define MPU_RGDAAC_M1UM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M1UM_SHIFT)) & MPU_RGDAAC_M1UM_MASK)
#define MPU_RGDAAC_M1SM_MASK                     (0x600U)
#define MPU_RGDAAC_M1SM_SHIFT                    (9U)
#define MPU_RGDAAC_M1SM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M1SM_SHIFT)) & MPU_RGDAAC_M1SM_MASK)
#define MPU_RGDAAC_M1PE                          (0x800U)
#define MPU_RGDAAC_M2UM_MASK                     (0x7000U)
#define MPU_RGDAAC_M2UM_SHIFT                    (12U)
#define MPU_RGDAAC_M2UM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M2UM_SHIFT)) & MPU_RGDAAC_M2UM_MASK)
#define MPU_RGDAAC_M2SM_MASK                     (0x18000U)
#define MPU_RGDAAC_M2SM_SHIFT                    (15U)
#define MPU_RGDAAC_M2SM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M2SM_SHIFT)) & MPU_RGDAAC_M2SM_MASK)
#define MPU_RGDAAC_M2PE                          (0x20000U)
#define MPU_RGDAAC_M3UM_MASK                     (0x1C0000U)
#define MPU_RGDAAC_M3UM_SHIFT                    (18U)
#define MPU_RGDAAC_M3UM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M3UM_SHIFT)) & MPU_RGDAAC_M3UM_MASK)
#define MPU_RGDAAC_M3SM_MASK                     (0x600000U)
#define MPU_RGDAAC_M3SM_SHIFT                    (21U)
#define MPU_RGDAAC_M3SM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M3SM_SHIFT)) & MPU_RGDAAC_M3SM_MASK)
#define MPU_RGDAAC_M3PE                          (0x800000U)
#define MPU_RGDAAC_M4WE                          (0x1000000U)
#define MPU_RGDAAC_M4RE                          (0x2000000U)
#define MPU_RGDAAC_M5WE                          (0x4000000U)
#define MPU_RGDAAC_M5RE                          (0x8000000U)
#define MPU_RGDAAC_M6WE                          (0x10000000U)
#define MPU_RGDAAC_M6RE                          (0x20000000U)
#define MPU_RGDAAC_M7WE                          (0x40000000U)
#define MPU_RGDAAC_M7RE                          (0x80000000U)

/* The count of MPU_RGDAAC */
#define MPU_RGDAAC_COUNT                         (12U)


/*!
 * @}
 */ /* end of group MPU_Register_Masks */


/* MPU - Peripheral instance base addresses */
/** Peripheral MPU base address */
#define MPU_BASE                                 (0x4000D000u)
/** Peripheral MPU base pointer */
#define MPU                                      ((MPU_TypeDef *)MPU_BASE)
/** Array initializer of MPU peripheral base addresses */
#define MPU_BASE_ADDRS                           { MPU_BASE }
/** Array initializer of MPU peripheral base pointers */
#define MPU_BASE_PTRS                            { MPU }

/*!
 * @}
 */ /* end of group MPU_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- NV Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup NV_Peripheral_Access_Layer NV Peripheral Access Layer
 * @{
 */

/** NV - Register Layout Typedef */
typedef struct {
  __I  uint8_t BACKKEY3;                           /**< Backdoor Comparison Key 3., offset: 0x0 */
  __I  uint8_t BACKKEY2;                           /**< Backdoor Comparison Key 2., offset: 0x1 */
  __I  uint8_t BACKKEY1;                           /**< Backdoor Comparison Key 1., offset: 0x2 */
  __I  uint8_t BACKKEY0;                           /**< Backdoor Comparison Key 0., offset: 0x3 */
  __I  uint8_t BACKKEY7;                           /**< Backdoor Comparison Key 7., offset: 0x4 */
  __I  uint8_t BACKKEY6;                           /**< Backdoor Comparison Key 6., offset: 0x5 */
  __I  uint8_t BACKKEY5;                           /**< Backdoor Comparison Key 5., offset: 0x6 */
  __I  uint8_t BACKKEY4;                           /**< Backdoor Comparison Key 4., offset: 0x7 */
  __I  uint8_t FPROT3;                             /**< Non-volatile P-Flash Protection 1 - Low Register, offset: 0x8 */
  __I  uint8_t FPROT2;                             /**< Non-volatile P-Flash Protection 1 - High Register, offset: 0x9 */
  __I  uint8_t FPROT1;                             /**< Non-volatile P-Flash Protection 0 - Low Register, offset: 0xA */
  __I  uint8_t FPROT0;                             /**< Non-volatile P-Flash Protection 0 - High Register, offset: 0xB */
  __I  uint8_t FSEC;                               /**< Non-volatile Flash Security Register, offset: 0xC */
  __I  uint8_t FOPT;                               /**< Non-volatile Flash Option Register, offset: 0xD */
  __I  uint8_t FEPROT;                             /**< Non-volatile EERAM Protection Register, offset: 0xE */
  __I  uint8_t FDPROT;                             /**< Non-volatile D-Flash Protection Register, offset: 0xF */
} NV_TypeDef;

/* ----------------------------------------------------------------------------
   -- NV Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup NV_Register_Masks NV Register Masks
 * @{
 */

/*! @name FSEC - Non-volatile Flash Security Register */
#define NV_FSEC_SEC_MASK                         (0x3U)
#define NV_FSEC_SEC_SHIFT                        (0U)
#define NV_FSEC_SEC(x)                           (((uint8_t)(((uint8_t)(x)) << NV_FSEC_SEC_SHIFT)) & NV_FSEC_SEC_MASK)
#define NV_FSEC_FSLACC_MASK                      (0xCU)
#define NV_FSEC_FSLACC_SHIFT                     (2U)
#define NV_FSEC_FSLACC(x)                        (((uint8_t)(((uint8_t)(x)) << NV_FSEC_FSLACC_SHIFT)) & NV_FSEC_FSLACC_MASK)
#define NV_FSEC_MEEN_MASK                        (0x30U)
#define NV_FSEC_MEEN_SHIFT                       (4U)
#define NV_FSEC_MEEN(x)                          (((uint8_t)(((uint8_t)(x)) << NV_FSEC_MEEN_SHIFT)) & NV_FSEC_MEEN_MASK)
#define NV_FSEC_KEYEN_MASK                       (0xC0U)
#define NV_FSEC_KEYEN_SHIFT                      (6U)
#define NV_FSEC_KEYEN(x)                         (((uint8_t)(((uint8_t)(x)) << NV_FSEC_KEYEN_SHIFT)) & NV_FSEC_KEYEN_MASK)

/*! @name FOPT - Non-volatile Flash Option Register */
#define NV_FOPT_LPBOOT                           (0x1U)
#define NV_FOPT_EZPORT_DIS                       (0x2U)

/*!
 * @}
 */ /* end of group NV_Register_Masks */


/* NV - Peripheral instance base addresses */
/** Peripheral FTFE_FlashConfig base address */
#define FTFE_FlashConfig_BASE                    (0x400u)
/** Peripheral FTFE_FlashConfig base pointer */
#define FTFE_FlashConfig                         ((NV_TypeDef *)FTFE_FlashConfig_BASE)
/** Array initializer of NV peripheral base addresses */
#define NV_BASE_ADDRS                            { FTFE_FlashConfig_BASE }
/** Array initializer of NV peripheral base pointers */
#define NV_BASE_PTRS                             { FTFE_FlashConfig }

/*!
 * @}
 */ /* end of group NV_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- OSC Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup OSC_Peripheral_Access_Layer OSC Peripheral Access Layer
 * @{
 */

/** OSC - Register Layout Typedef */
typedef struct {
  __IO uint8_t CR;                                 /**< OSC Control Register, offset: 0x0 */
} OSC_TypeDef;

/* ----------------------------------------------------------------------------
   -- OSC Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup OSC_Register_Masks OSC Register Masks
 * @{
 */

/*! @name CR - OSC Control Register */
#define OSC_CR_SC16P                             (0x1U)
#define OSC_CR_SC8P                              (0x2U)
#define OSC_CR_SC4P                              (0x4U)
#define OSC_CR_SC2P                              (0x8U)
#define OSC_CR_EREFSTEN                          (0x20U)
#define OSC_CR_ERCLKEN                           (0x80U)


/*!
 * @}
 */ /* end of group OSC_Register_Masks */


/* OSC - Peripheral instance base addresses */
/** Peripheral OSC base address */
#define OSC_BASE                                 (0x40065000u)
#define OSC0_BASE                                OSC_BASE
/** Peripheral OSC base pointer */
#define OSC                                      ((OSC_TypeDef *)OSC_BASE)
#define OSC0                                     OSC
/** Array initializer of OSC peripheral base addresses */
#define OSC_BASE_ADDRS                           { OSC_BASE }
/** Array initializer of OSC peripheral base pointers */
#define OSC_BASE_PTRS                            { OSC }

/*!
 * @}
 */ /* end of group OSC_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- PDB Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup PDB_Peripheral_Access_Layer PDB Peripheral Access Layer
 * @{
 */

/** PDB - Register Layout Typedef */
typedef struct {
  __IO uint32_t SC;                                /**< Status and Control register, offset: 0x0 */
  __IO uint32_t MOD;                               /**< Modulus register, offset: 0x4 */
  __I  uint32_t CNT;                               /**< Counter register, offset: 0x8 */
  __IO uint32_t IDLY;                              /**< Interrupt Delay register, offset: 0xC */
  struct {                                         /* offset: 0x10, array step: 0x28 */
    __IO uint32_t C1;                                /**< Channel n Control register 1, array offset: 0x10, array step: 0x28 */
    __IO uint32_t S;                                 /**< Channel n Status register, array offset: 0x14, array step: 0x28 */
    __IO uint32_t DLY[2];                            /**< Channel n Delay 0 register..Channel n Delay 1 register, array offset: 0x18, array step: index*0x28, index2*0x4 */
         uint8_t RESERVED_0[24];
  } CH[2];
       uint8_t RESERVED_0[240];
  struct {                                         /* offset: 0x150, array step: 0x8 */
    __IO uint32_t INTC;                              /**< DAC Interval Trigger n Control register, array offset: 0x150, array step: 0x8 */
    __IO uint32_t INT;                               /**< DAC Interval n register, array offset: 0x154, array step: 0x8 */
  } DAC[2];
       uint8_t RESERVED_1[48];
  __IO uint32_t POEN;                              /**< Pulse-Out n Enable register, offset: 0x190 */
  __IO uint32_t PODLY[3];                          /**< Pulse-Out n Delay register, array offset: 0x194, array step: 0x4 */
} PDB_TypeDef;

/* ----------------------------------------------------------------------------
   -- PDB Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup PDB_Register_Masks PDB Register Masks
 * @{
 */

/*! @name SC - Status and Control register */
#define PDB_SC_LDOK                              (0x1U)
#define PDB_SC_CONT                              (0x2U)
#define PDB_SC_MULT_MASK                         (0xCU)
#define PDB_SC_MULT_SHIFT                        (2U)
#define PDB_SC_MULT(x)                           (((uint32_t)(((uint32_t)(x)) << PDB_SC_MULT_SHIFT)) & PDB_SC_MULT_MASK)
#define PDB_SC_PDBIE                             (0x20U)
#define PDB_SC_PDBIF                             (0x40U)
#define PDB_SC_PDBEN                             (0x80U)
#define PDB_SC_TRGSEL_MASK                       (0xF00U)
#define PDB_SC_TRGSEL_SHIFT                      (8U)
#define PDB_SC_TRGSEL(x)                         (((uint32_t)(((uint32_t)(x)) << PDB_SC_TRGSEL_SHIFT)) & PDB_SC_TRGSEL_MASK)
#define PDB_SC_PRESCALER_MASK                    (0x7000U)
#define PDB_SC_PRESCALER_SHIFT                   (12U)
#define PDB_SC_PRESCALER(x)                      (((uint32_t)(((uint32_t)(x)) << PDB_SC_PRESCALER_SHIFT)) & PDB_SC_PRESCALER_MASK)
#define PDB_SC_DMAEN                             (0x8000U)
#define PDB_SC_SWTRIG                            (0x10000U)
#define PDB_SC_PDBEIE                            (0x20000U)
#define PDB_SC_LDMOD_MASK                        (0xC0000U)
#define PDB_SC_LDMOD_SHIFT                       (18U)
#define PDB_SC_LDMOD(x)                          (((uint32_t)(((uint32_t)(x)) << PDB_SC_LDMOD_SHIFT)) & PDB_SC_LDMOD_MASK)

/*! @name MOD - Modulus register */
#define PDB_MOD_MOD_MASK                         (0xFFFFU)
#define PDB_MOD_MOD_SHIFT                        (0U)
#define PDB_MOD_MOD(x)                           (((uint32_t)(((uint32_t)(x)) << PDB_MOD_MOD_SHIFT)) & PDB_MOD_MOD_MASK)

/*! @name CNT - Counter register */
#define PDB_CNT_CNT_MASK                         (0xFFFFU)
#define PDB_CNT_CNT_SHIFT                        (0U)
#define PDB_CNT_CNT(x)                           (((uint32_t)(((uint32_t)(x)) << PDB_CNT_CNT_SHIFT)) & PDB_CNT_CNT_MASK)

/*! @name IDLY - Interrupt Delay register */
#define PDB_IDLY_IDLY_MASK                       (0xFFFFU)
#define PDB_IDLY_IDLY_SHIFT                      (0U)
#define PDB_IDLY_IDLY(x)                         (((uint32_t)(((uint32_t)(x)) << PDB_IDLY_IDLY_SHIFT)) & PDB_IDLY_IDLY_MASK)

/*! @name C1 - Channel n Control register 1 */
#define PDB_C1_EN_MASK                           (0xFFU)
#define PDB_C1_EN_SHIFT                          (0U)
#define PDB_C1_EN(x)                             (((uint32_t)(((uint32_t)(x)) << PDB_C1_EN_SHIFT)) & PDB_C1_EN_MASK)
#define PDB_C1_TOS_MASK                          (0xFF00U)
#define PDB_C1_TOS_SHIFT                         (8U)
#define PDB_C1_TOS(x)                            (((uint32_t)(((uint32_t)(x)) << PDB_C1_TOS_SHIFT)) & PDB_C1_TOS_MASK)
#define PDB_C1_BB_MASK                           (0xFF0000U)
#define PDB_C1_BB_SHIFT                          (16U)
#define PDB_C1_BB(x)                             (((uint32_t)(((uint32_t)(x)) << PDB_C1_BB_SHIFT)) & PDB_C1_BB_MASK)

/* The count of PDB_C1 */
#define PDB_C1_COUNT                             (2U)

/*! @name S - Channel n Status register */
#define PDB_S_ERR_MASK                           (0xFFU)
#define PDB_S_ERR_SHIFT                          (0U)
#define PDB_S_ERR(x)                             (((uint32_t)(((uint32_t)(x)) << PDB_S_ERR_SHIFT)) & PDB_S_ERR_MASK)
#define PDB_S_CF_MASK                            (0xFF0000U)
#define PDB_S_CF_SHIFT                           (16U)
#define PDB_S_CF(x)                              (((uint32_t)(((uint32_t)(x)) << PDB_S_CF_SHIFT)) & PDB_S_CF_MASK)

/* The count of PDB_S */
#define PDB_S_COUNT                              (2U)

/*! @name DLY - Channel n Delay 0 register..Channel n Delay 1 register */
#define PDB_DLY_DLY_MASK                         (0xFFFFU)
#define PDB_DLY_DLY_SHIFT                        (0U)
#define PDB_DLY_DLY(x)                           (((uint32_t)(((uint32_t)(x)) << PDB_DLY_DLY_SHIFT)) & PDB_DLY_DLY_MASK)

/* The count of PDB_DLY */
#define PDB_DLY_COUNT                            (2U)

/* The count of PDB_DLY */
#define PDB_DLY_COUNT2                           (2U)

/*! @name INTC - DAC Interval Trigger n Control register */
#define PDB_INTC_TOE                             (0x1U)
#define PDB_INTC_EXT                             (0x2U)

/* The count of PDB_INTC */
#define PDB_INTC_COUNT                           (2U)

/*! @name INT - DAC Interval n register */
#define PDB_INT_INT_MASK                         (0xFFFFU)
#define PDB_INT_INT_SHIFT                        (0U)
#define PDB_INT_INT(x)                           (((uint32_t)(((uint32_t)(x)) << PDB_INT_INT_SHIFT)) & PDB_INT_INT_MASK)

/* The count of PDB_INT */
#define PDB_INT_COUNT                            (2U)

/*! @name POEN - Pulse-Out n Enable register */
#define PDB_POEN_POEN_MASK                       (0xFFU)
#define PDB_POEN_POEN_SHIFT                      (0U)
#define PDB_POEN_POEN(x)                         (((uint32_t)(((uint32_t)(x)) << PDB_POEN_POEN_SHIFT)) & PDB_POEN_POEN_MASK)

/*! @name PODLY - Pulse-Out n Delay register */
#define PDB_PODLY_DLY2_MASK                      (0xFFFFU)
#define PDB_PODLY_DLY2_SHIFT                     (0U)
#define PDB_PODLY_DLY2(x)                        (((uint32_t)(((uint32_t)(x)) << PDB_PODLY_DLY2_SHIFT)) & PDB_PODLY_DLY2_MASK)
#define PDB_PODLY_DLY1_MASK                      (0xFFFF0000U)
#define PDB_PODLY_DLY1_SHIFT                     (16U)
#define PDB_PODLY_DLY1(x)                        (((uint32_t)(((uint32_t)(x)) << PDB_PODLY_DLY1_SHIFT)) & PDB_PODLY_DLY1_MASK)

/* The count of PDB_PODLY */
#define PDB_PODLY_COUNT                          (3U)


/*!
 * @}
 */ /* end of group PDB_Register_Masks */


/* PDB - Peripheral instance base addresses */
/** Peripheral PDB0 base address */
#define PDB0_BASE                                (0x40036000u)
/** Peripheral PDB0 base pointer */
#define PDB0                                     ((PDB_TypeDef *)PDB0_BASE)
/** Array initializer of PDB peripheral base addresses */
#define PDB_BASE_ADDRS                           { PDB0_BASE }
/** Array initializer of PDB peripheral base pointers */
#define PDB_BASE_PTRS                            { PDB0 }
/** Interrupt vectors for the PDB peripheral type */
#define PDB_IRQS                                 { PDB0_IRQn }

/*!
 * @}
 */ /* end of group PDB_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- PIT Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup PIT_Peripheral_Access_Layer PIT Peripheral Access Layer
 * @{
 */

/** PIT - Register Layout Typedef */
typedef struct {
  __IO uint32_t MCR;                               /**< PIT Module Control Register, offset: 0x0 */
       uint8_t RESERVED_0[252];
  struct {                                         /* offset: 0x100, array step: 0x10 */
    __IO uint32_t LDVAL;                             /**< Timer Load Value Register, array offset: 0x100, array step: 0x10 */
    __I  uint32_t CVAL;                              /**< Current Timer Value Register, array offset: 0x104, array step: 0x10 */
    __IO uint32_t TCTRL;                             /**< Timer Control Register, array offset: 0x108, array step: 0x10 */
    __IO uint32_t TFLG;                              /**< Timer Flag Register, array offset: 0x10C, array step: 0x10 */
  } CHANNEL[4];
} PIT_TypeDef;

/* ----------------------------------------------------------------------------
   -- PIT Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup PIT_Register_Masks PIT Register Masks
 * @{
 */

/*! @name MCR - PIT Module Control Register */
#define PIT_MCR_FRZ                              (0x1U)
#define PIT_MCR_MDIS                             (0x2U)

/* The count of PIT_LDVAL */
#define PIT_LDVAL_COUNT                          (4U)

/* The count of PIT_CVAL */
#define PIT_CVAL_COUNT                           (4U)

/*! @name TCTRL - Timer Control Register */
#define PIT_TCTRL_TEN                            (0x1U)
#define PIT_TCTRL_TIE                            (0x2U)
#define PIT_TCTRL_CHN                            (0x4U)

/* The count of PIT_TCTRL */
#define PIT_TCTRL_COUNT                          (4U)

/*! @name TFLG - Timer Flag Register */
#define PIT_TFLG_TIF                             (0x1U)

/* The count of PIT_TFLG */
#define PIT_TFLG_COUNT                           (4U)


/*!
 * @}
 */ /* end of group PIT_Register_Masks */


/* PIT - Peripheral instance base addresses */
/** Peripheral PIT base address */
#define PIT_BASE                                 (0x40037000u)
/** Peripheral PIT base pointer */
#define PIT                                      ((PIT_TypeDef *)PIT_BASE)
/** Array initializer of PIT peripheral base addresses */
#define PIT_BASE_ADDRS                           { PIT_BASE }
/** Array initializer of PIT peripheral base pointers */
#define PIT_BASE_PTRS                            { PIT }
/** Interrupt vectors for the PIT peripheral type */
#define PIT_IRQS                                 { PIT0_IRQn, PIT1_IRQn, PIT2_IRQn, PIT3_IRQn }

/*!
 * @}
 */ /* end of group PIT_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- PMC Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup PMC_Peripheral_Access_Layer PMC Peripheral Access Layer
 * @{
 */

/** PMC - Register Layout Typedef */
typedef struct {
  __IO uint8_t LVDSC1;                             /**< Low Voltage Detect Status And Control 1 register, offset: 0x0 */
  __IO uint8_t LVDSC2;                             /**< Low Voltage Detect Status And Control 2 register, offset: 0x1 */
  __IO uint8_t REGSC;                              /**< Regulator Status And Control register, offset: 0x2 */
} PMC_TypeDef;

/* ----------------------------------------------------------------------------
   -- PMC Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup PMC_Register_Masks PMC Register Masks
 * @{
 */

/*! @name LVDSC1 - Low Voltage Detect Status And Control 1 register */
#define PMC_LVDSC1_LVDV_MASK                     (0x3U)
#define PMC_LVDSC1_LVDV_SHIFT                    (0U)
#define PMC_LVDSC1_LVDV(x)                       (((uint8_t)(((uint8_t)(x)) << PMC_LVDSC1_LVDV_SHIFT)) & PMC_LVDSC1_LVDV_MASK)
#define PMC_LVDSC1_LVDRE                         (0x10U)
#define PMC_LVDSC1_LVDIE                         (0x20U)
#define PMC_LVDSC1_LVDACK                        (0x40U)
#define PMC_LVDSC1_LVDF                          (0x80U)

/*! @name LVDSC2 - Low Voltage Detect Status And Control 2 register */
#define PMC_LVDSC2_LVWV_MASK                     (0x3U)
#define PMC_LVDSC2_LVWV_SHIFT                    (0U)
#define PMC_LVDSC2_LVWV(x)                       (((uint8_t)(((uint8_t)(x)) << PMC_LVDSC2_LVWV_SHIFT)) & PMC_LVDSC2_LVWV_MASK)
#define PMC_LVDSC2_LVWIE                         (0x20U)
#define PMC_LVDSC2_LVWACK                        (0x40U)
#define PMC_LVDSC2_LVWF                          (0x80U)

/*! @name REGSC - Regulator Status And Control register */
#define PMC_REGSC_BGBE                           (0x1U)
#define PMC_REGSC_REGONS                         (0x4U)
#define PMC_REGSC_ACKISO                         (0x8U)
#define PMC_REGSC_BGEN                           (0x10U)


/*!
 * @}
 */ /* end of group PMC_Register_Masks */


/* PMC - Peripheral instance base addresses */
/** Peripheral PMC base address */
#define PMC_BASE                                 (0x4007D000u)
/** Peripheral PMC base pointer */
#define PMC                                      ((PMC_TypeDef *)PMC_BASE)
/** Array initializer of PMC peripheral base addresses */
#define PMC_BASE_ADDRS                           { PMC_BASE }
/** Array initializer of PMC peripheral base pointers */
#define PMC_BASE_PTRS                            { PMC }
/** Interrupt vectors for the PMC peripheral type */
#define PMC_IRQS                                 { LVD_LVW_IRQn }

/*!
 * @}
 */ /* end of group PMC_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- PORT Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup PORT_Peripheral_Access_Layer PORT Peripheral Access Layer
 * @{
 */

/** PORT - Register Layout Typedef */
typedef struct {
  __IO uint32_t PCR[32];                           /**< Pin Control Register n, array offset: 0x0, array step: 0x4 */
  __O  uint32_t GPCLR;                             /**< Global Pin Control Low Register, offset: 0x80 */
  __O  uint32_t GPCHR;                             /**< Global Pin Control High Register, offset: 0x84 */
       uint8_t RESERVED_0[24];
  __IO uint32_t ISFR;                              /**< Interrupt Status Flag Register, offset: 0xA0 */
       uint8_t RESERVED_1[28];
  __IO uint32_t DFER;                              /**< Digital Filter Enable Register, offset: 0xC0 */
  __IO uint32_t DFCR;                              /**< Digital Filter Clock Register, offset: 0xC4 */
  __IO uint32_t DFWR;                              /**< Digital Filter Width Register, offset: 0xC8 */
} PORT_TypeDef;

/* ----------------------------------------------------------------------------
   -- PORT Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup PORT_Register_Masks PORT Register Masks
 * @{
 */

/*! @name PCR - Pin Control Register n */
#define PORTx_PCRn_PS                           (0x1U)
#define PORTx_PCRn_PE                           (0x2U)
#define PORTx_PCRn_SRE                          (0x4U)
#define PORTx_PCRn_PFE                          (0x10U)
#define PORTx_PCRn_ODE                          (0x20U)
#define PORTx_PCRn_DSE                          (0x40U)
#define PORTx_PCRn_MUX_MASK                     (0x700U)
#define PORTx_PCRn_MUX_SHIFT                    (8U)
#define PORTx_PCRn_MUX(x)                       (((uint32_t)(((uint32_t)(x)) << PORTx_PCRn_MUX_SHIFT)) & PORTx_PCRn_MUX_MASK)
#define PORTx_PCRn_LK                           (0x8000U)
#define PORTx_PCRn_IRQC_MASK                    (0xF0000U)
#define PORTx_PCRn_IRQC_SHIFT                   (16U)
#define PORTx_PCRn_IRQC(x)                      (((uint32_t)(((uint32_t)(x)) << PORTx_PCRn_IRQC_SHIFT)) & PORTx_PCRn_IRQC_MASK)
#define PORTx_PCRn_ISF                          (0x1000000U)

/* The count of PORT_PCR */
#define PORT_PCR_COUNT                           (32U)

/*! @name GPCLR - Global Pin Control Low Register */
#define PORT_GPCLR_GPWD_MASK                     (0xFFFFU)
#define PORT_GPCLR_GPWD_SHIFT                    (0U)
#define PORT_GPCLR_GPWD(x)                       (((uint32_t)(((uint32_t)(x)) << PORT_GPCLR_GPWD_SHIFT)) & PORT_GPCLR_GPWD_MASK)
#define PORT_GPCLR_GPWE_MASK                     (0xFFFF0000U)
#define PORT_GPCLR_GPWE_SHIFT                    (16U)
#define PORT_GPCLR_GPWE(x)                       (((uint32_t)(((uint32_t)(x)) << PORT_GPCLR_GPWE_SHIFT)) & PORT_GPCLR_GPWE_MASK)

/*! @name GPCHR - Global Pin Control High Register */
#define PORT_GPCHR_GPWD_MASK                     (0xFFFFU)
#define PORT_GPCHR_GPWD_SHIFT                    (0U)
#define PORT_GPCHR_GPWD(x)                       (((uint32_t)(((uint32_t)(x)) << PORT_GPCHR_GPWD_SHIFT)) & PORT_GPCHR_GPWD_MASK)
#define PORT_GPCHR_GPWE_MASK                     (0xFFFF0000U)
#define PORT_GPCHR_GPWE_SHIFT                    (16U)
#define PORT_GPCHR_GPWE(x)                       (((uint32_t)(((uint32_t)(x)) << PORT_GPCHR_GPWE_SHIFT)) & PORT_GPCHR_GPWE_MASK)

/*! @name DFCR - Digital Filter Clock Register */
#define PORT_DFCR_CS                             (0x1U)

/*! @name DFWR - Digital Filter Width Register */
#define PORT_DFWR_FILT_MASK                      (0x1FU)
#define PORT_DFWR_FILT_SHIFT                     (0U)
#define PORT_DFWR_FILT(x)                        (((uint32_t)(((uint32_t)(x)) << PORT_DFWR_FILT_SHIFT)) & PORT_DFWR_FILT_MASK)


/*!
 * @}
 */ /* end of group PORT_Register_Masks */


/* PORT - Peripheral instance base addresses */
/** Peripheral PORTA base address */
#define PORTA_BASE                               (0x40049000u)
/** Peripheral PORTA base pointer */
#define PORTA                                    ((PORT_TypeDef *)PORTA_BASE)
/** Peripheral PORTB base address */
#define PORTB_BASE                               (0x4004A000u)
/** Peripheral PORTB base pointer */
#define PORTB                                    ((PORT_TypeDef *)PORTB_BASE)
/** Peripheral PORTC base address */
#define PORTC_BASE                               (0x4004B000u)
/** Peripheral PORTC base pointer */
#define PORTC                                    ((PORT_TypeDef *)PORTC_BASE)
/** Peripheral PORTD base address */
#define PORTD_BASE                               (0x4004C000u)
/** Peripheral PORTD base pointer */
#define PORTD                                    ((PORT_TypeDef *)PORTD_BASE)
/** Peripheral PORTE base address */
#define PORTE_BASE                               (0x4004D000u)
/** Peripheral PORTE base pointer */
#define PORTE                                    ((PORT_TypeDef *)PORTE_BASE)
/** Array initializer of PORT peripheral base addresses */
#define PORT_BASE_ADDRS                          { PORTA_BASE, PORTB_BASE, PORTC_BASE, PORTD_BASE, PORTE_BASE }
/** Array initializer of PORT peripheral base pointers */
#define PORT_BASE_PTRS                           { PORTA, PORTB, PORTC, PORTD, PORTE }
/** Interrupt vectors for the PORT peripheral type */
#define PORT_IRQS                                { PORTA_IRQn, PORTB_IRQn, PORTC_IRQn, PORTD_IRQn, PORTE_IRQn }

/*!
 * @}
 */ /* end of group PORT_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- RCM Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup RCM_Peripheral_Access_Layer RCM Peripheral Access Layer
 * @{
 */

/** RCM - Register Layout Typedef */
typedef struct {
  __I  uint8_t SRS0;                               /**< System Reset Status Register 0, offset: 0x0 */
  __I  uint8_t SRS1;                               /**< System Reset Status Register 1, offset: 0x1 */
       uint8_t RESERVED_0[2];
  __IO uint8_t RPFC;                               /**< Reset Pin Filter Control register, offset: 0x4 */
  __IO uint8_t RPFW;                               /**< Reset Pin Filter Width register, offset: 0x5 */
       uint8_t RESERVED_1[1];
  __I  uint8_t MR;                                 /**< Mode Register, offset: 0x7 */
} RCM_TypeDef;

/* ----------------------------------------------------------------------------
   -- RCM Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup RCM_Register_Masks RCM Register Masks
 * @{
 */

/*! @name SRS0 - System Reset Status Register 0 */
#define RCM_SRS0_WAKEUP                          (0x1U)
#define RCM_SRS0_LVD                             (0x2U)
#define RCM_SRS0_LOC                             (0x4U)
#define RCM_SRS0_LOL                             (0x8U)
#define RCM_SRS0_WDOG                            (0x20U)
#define RCM_SRS0_PIN                             (0x40U)
#define RCM_SRS0_POR                             (0x80U)

/*! @name SRS1 - System Reset Status Register 1 */
#define RCM_SRS1_JTAG                            (0x1U)
#define RCM_SRS1_LOCKUP                          (0x2U)
#define RCM_SRS1_SW                              (0x4U)
#define RCM_SRS1_MDM_AP                          (0x8U)
#define RCM_SRS1_EZPT                            (0x10U)
#define RCM_SRS1_SACKERR                         (0x20U)

/*! @name RPFC - Reset Pin Filter Control register */
#define RCM_RPFC_RSTFLTSRW_MASK                  (0x3U)
#define RCM_RPFC_RSTFLTSRW_SHIFT                 (0U)
#define RCM_RPFC_RSTFLTSRW(x)                    (((uint8_t)(((uint8_t)(x)) << RCM_RPFC_RSTFLTSRW_SHIFT)) & RCM_RPFC_RSTFLTSRW_MASK)
#define RCM_RPFC_RSTFLTSS                        (0x4U)

/*! @name RPFW - Reset Pin Filter Width register */
#define RCM_RPFW_RSTFLTSEL_MASK                  (0x1FU)
#define RCM_RPFW_RSTFLTSEL_SHIFT                 (0U)
#define RCM_RPFW_RSTFLTSEL(x)                    (((uint8_t)(((uint8_t)(x)) << RCM_RPFW_RSTFLTSEL_SHIFT)) & RCM_RPFW_RSTFLTSEL_MASK)

/*! @name MR - Mode Register */
#define RCM_MR_EZP_MS                            (0x2U)


/*!
 * @}
 */ /* end of group RCM_Register_Masks */


/* RCM - Peripheral instance base addresses */
/** Peripheral RCM base address */
#define RCM_BASE                                 (0x4007F000u)
/** Peripheral RCM base pointer */
#define RCM                                      ((RCM_TypeDef *)RCM_BASE)
/** Array initializer of RCM peripheral base addresses */
#define RCM_BASE_ADDRS                           { RCM_BASE }
/** Array initializer of RCM peripheral base pointers */
#define RCM_BASE_PTRS                            { RCM }

/*!
 * @}
 */ /* end of group RCM_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- RFSYS Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup RFSYS_Peripheral_Access_Layer RFSYS Peripheral Access Layer
 * @{
 */

/** RFSYS - Register Layout Typedef */
typedef struct {
  __IO uint32_t REG[8];                            /**< Register file register, array offset: 0x0, array step: 0x4 */
} RFSYS_TypeDef;

/* ----------------------------------------------------------------------------
   -- RFSYS Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup RFSYS_Register_Masks RFSYS Register Masks
 * @{
 */

/*! @name REG - Register file register */
#define RFSYS_REG_LL_MASK                        (0xFFU)
#define RFSYS_REG_LL_SHIFT                       (0U)
#define RFSYS_REG_LL(x)                          (((uint32_t)(((uint32_t)(x)) << RFSYS_REG_LL_SHIFT)) & RFSYS_REG_LL_MASK)
#define RFSYS_REG_LH_MASK                        (0xFF00U)
#define RFSYS_REG_LH_SHIFT                       (8U)
#define RFSYS_REG_LH(x)                          (((uint32_t)(((uint32_t)(x)) << RFSYS_REG_LH_SHIFT)) & RFSYS_REG_LH_MASK)
#define RFSYS_REG_HL_MASK                        (0xFF0000U)
#define RFSYS_REG_HL_SHIFT                       (16U)
#define RFSYS_REG_HL(x)                          (((uint32_t)(((uint32_t)(x)) << RFSYS_REG_HL_SHIFT)) & RFSYS_REG_HL_MASK)
#define RFSYS_REG_HH_MASK                        (0xFF000000U)
#define RFSYS_REG_HH_SHIFT                       (24U)
#define RFSYS_REG_HH(x)                          (((uint32_t)(((uint32_t)(x)) << RFSYS_REG_HH_SHIFT)) & RFSYS_REG_HH_MASK)

/* The count of RFSYS_REG */
#define RFSYS_REG_COUNT                          (8U)


/*!
 * @}
 */ /* end of group RFSYS_Register_Masks */


/* RFSYS - Peripheral instance base addresses */
/** Peripheral RFSYS base address */
#define RFSYS_BASE                               (0x40041000u)
/** Peripheral RFSYS base pointer */
#define RFSYS                                    ((RFSYS_TypeDef *)RFSYS_BASE)
/** Array initializer of RFSYS peripheral base addresses */
#define RFSYS_BASE_ADDRS                         { RFSYS_BASE }
/** Array initializer of RFSYS peripheral base pointers */
#define RFSYS_BASE_PTRS                          { RFSYS }

/*!
 * @}
 */ /* end of group RFSYS_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- RFVBAT Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup RFVBAT_Peripheral_Access_Layer RFVBAT Peripheral Access Layer
 * @{
 */

/** RFVBAT - Register Layout Typedef */
typedef struct {
  __IO uint32_t REG[8];                            /**< VBAT register file register, array offset: 0x0, array step: 0x4 */
} RFVBAT_TypeDef;

/* ----------------------------------------------------------------------------
   -- RFVBAT Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup RFVBAT_Register_Masks RFVBAT Register Masks
 * @{
 */

/*! @name REG - VBAT register file register */
#define RFVBAT_REG_LL_MASK                       (0xFFU)
#define RFVBAT_REG_LL_SHIFT                      (0U)
#define RFVBAT_REG_LL(x)                         (((uint32_t)(((uint32_t)(x)) << RFVBAT_REG_LL_SHIFT)) & RFVBAT_REG_LL_MASK)
#define RFVBAT_REG_LH_MASK                       (0xFF00U)
#define RFVBAT_REG_LH_SHIFT                      (8U)
#define RFVBAT_REG_LH(x)                         (((uint32_t)(((uint32_t)(x)) << RFVBAT_REG_LH_SHIFT)) & RFVBAT_REG_LH_MASK)
#define RFVBAT_REG_HL_MASK                       (0xFF0000U)
#define RFVBAT_REG_HL_SHIFT                      (16U)
#define RFVBAT_REG_HL(x)                         (((uint32_t)(((uint32_t)(x)) << RFVBAT_REG_HL_SHIFT)) & RFVBAT_REG_HL_MASK)
#define RFVBAT_REG_HH_MASK                       (0xFF000000U)
#define RFVBAT_REG_HH_SHIFT                      (24U)
#define RFVBAT_REG_HH(x)                         (((uint32_t)(((uint32_t)(x)) << RFVBAT_REG_HH_SHIFT)) & RFVBAT_REG_HH_MASK)

/* The count of RFVBAT_REG */
#define RFVBAT_REG_COUNT                         (8U)


/*!
 * @}
 */ /* end of group RFVBAT_Register_Masks */


/* RFVBAT - Peripheral instance base addresses */
/** Peripheral RFVBAT base address */
#define RFVBAT_BASE                              (0x4003E000u)
/** Peripheral RFVBAT base pointer */
#define RFVBAT                                   ((RFVBAT_TypeDef *)RFVBAT_BASE)
/** Array initializer of RFVBAT peripheral base addresses */
#define RFVBAT_BASE_ADDRS                        { RFVBAT_BASE }
/** Array initializer of RFVBAT peripheral base pointers */
#define RFVBAT_BASE_PTRS                         { RFVBAT }

/*!
 * @}
 */ /* end of group RFVBAT_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- RNG Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup RNG_Peripheral_Access_Layer RNG Peripheral Access Layer
 * @{
 */

/** RNG - Register Layout Typedef */
typedef struct {
  __IO uint32_t CR;                                /**< RNGA Control Register, offset: 0x0 */
  __I  uint32_t SR;                                /**< RNGA Status Register, offset: 0x4 */
  __O  uint32_t ER;                                /**< RNGA Entropy Register, offset: 0x8 */
  __I  uint32_t OR;                                /**< RNGA Output Register, offset: 0xC */
} RNG_TypeDef;

/* ----------------------------------------------------------------------------
   -- RNG Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup RNG_Register_Masks RNG Register Masks
 * @{
 */

/*! @name CR - RNGA Control Register */
#define RNG_CR_GO                                (0x1U)
#define RNG_CR_HA                                (0x2U)
#define RNG_CR_INTM                              (0x4U)
#define RNG_CR_CLRI                              (0x8U)
#define RNG_CR_SLP                               (0x10U)

/*! @name SR - RNGA Status Register */
#define RNG_SR_SECV                              (0x1U)
#define RNG_SR_LRS                               (0x2U)
#define RNG_SR_ORU                               (0x4U)
#define RNG_SR_ERRI                              (0x8U)
#define RNG_SR_SLP                               (0x10U)
#define RNG_SR_OREG_LVL_MASK                     (0xFF00U)
#define RNG_SR_OREG_LVL_SHIFT                    (8U)
#define RNG_SR_OREG_LVL(x)                       (((uint32_t)(((uint32_t)(x)) << RNG_SR_OREG_LVL_SHIFT)) & RNG_SR_OREG_LVL_MASK)
#define RNG_SR_OREG_SIZE_MASK                    (0xFF0000U)
#define RNG_SR_OREG_SIZE_SHIFT                   (16U)
#define RNG_SR_OREG_SIZE(x)                      (((uint32_t)(((uint32_t)(x)) << RNG_SR_OREG_SIZE_SHIFT)) & RNG_SR_OREG_SIZE_MASK)

/*!
 * @}
 */ /* end of group RNG_Register_Masks */


/* RNG - Peripheral instance base addresses */
/** Peripheral RNG base address */
#define RNG_BASE                                 (0x40029000u)
/** Peripheral RNG base pointer */
#define RNG                                      ((RNG_TypeDef *)RNG_BASE)
/** Array initializer of RNG peripheral base addresses */
#define RNG_BASE_ADDRS                           { RNG_BASE }
/** Array initializer of RNG peripheral base pointers */
#define RNG_BASE_PTRS                            { RNG }
/** Interrupt vectors for the RNG peripheral type */
#define RNG_IRQS                                 { RNG_IRQn }

/*!
 * @}
 */ /* end of group RNG_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- RTC Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup RTC_Peripheral_Access_Layer RTC Peripheral Access Layer
 * @{
 */

/** RTC - Register Layout Typedef */
typedef struct {
  __IO uint32_t TSR;                               /**< RTC Time Seconds Register, offset: 0x0 */
  __IO uint32_t TPR;                               /**< RTC Time Prescaler Register, offset: 0x4 */
  __IO uint32_t TAR;                               /**< RTC Time Alarm Register, offset: 0x8 */
  __IO uint32_t TCR;                               /**< RTC Time Compensation Register, offset: 0xC */
  __IO uint32_t CR;                                /**< RTC Control Register, offset: 0x10 */
  __IO uint32_t SR;                                /**< RTC Status Register, offset: 0x14 */
  __IO uint32_t LR;                                /**< RTC Lock Register, offset: 0x18 */
  __IO uint32_t IER;                               /**< RTC Interrupt Enable Register, offset: 0x1C */
       uint8_t RESERVED_0[2016];
  __IO uint32_t WAR;                               /**< RTC Write Access Register, offset: 0x800 */
  __IO uint32_t RAR;                               /**< RTC Read Access Register, offset: 0x804 */
} RTC_TypeDef;

/* ----------------------------------------------------------------------------
   -- RTC Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup RTC_Register_Masks RTC Register Masks
 * @{
 */

/*! @name TPR - RTC Time Prescaler Register */
#define RTC_TPR_TPR_MASK                         (0xFFFFU)
#define RTC_TPR_TPR_SHIFT                        (0U)
#define RTC_TPR_TPR(x)                           (((uint32_t)(((uint32_t)(x)) << RTC_TPR_TPR_SHIFT)) & RTC_TPR_TPR_MASK)

/*! @name TCR - RTC Time Compensation Register */
#define RTC_TCR_TCR_MASK                         (0xFFU)
#define RTC_TCR_TCR_SHIFT                        (0U)
#define RTC_TCR_TCR(x)                           (((uint32_t)(((uint32_t)(x)) << RTC_TCR_TCR_SHIFT)) & RTC_TCR_TCR_MASK)
#define RTC_TCR_CIR_MASK                         (0xFF00U)
#define RTC_TCR_CIR_SHIFT                        (8U)
#define RTC_TCR_CIR(x)                           (((uint32_t)(((uint32_t)(x)) << RTC_TCR_CIR_SHIFT)) & RTC_TCR_CIR_MASK)
#define RTC_TCR_TCV_MASK                         (0xFF0000U)
#define RTC_TCR_TCV_SHIFT                        (16U)
#define RTC_TCR_TCV(x)                           (((uint32_t)(((uint32_t)(x)) << RTC_TCR_TCV_SHIFT)) & RTC_TCR_TCV_MASK)
#define RTC_TCR_CIC_MASK                         (0xFF000000U)
#define RTC_TCR_CIC_SHIFT                        (24U)
#define RTC_TCR_CIC(x)                           (((uint32_t)(((uint32_t)(x)) << RTC_TCR_CIC_SHIFT)) & RTC_TCR_CIC_MASK)

/*! @name CR - RTC Control Register */
#define RTC_CR_SWR                               (0x1U)
#define RTC_CR_WPE                               (0x2U)
#define RTC_CR_SUP                               (0x4U)
#define RTC_CR_UM                                (0x8U)
#define RTC_CR_WPS                               (0x10U)
#define RTC_CR_OSCE                              (0x100U)
#define RTC_CR_CLKO                              (0x200U)
#define RTC_CR_SC16P                             (0x400U)
#define RTC_CR_SC8P                              (0x800U)
#define RTC_CR_SC4P                              (0x1000U)
#define RTC_CR_SC2P                              (0x2000U)

/*! @name SR - RTC Status Register */
#define RTC_SR_TIF                               (0x1U)
#define RTC_SR_TOF                               (0x2U)
#define RTC_SR_TAF                               (0x4U)
#define RTC_SR_TCE                               (0x10U)

/*! @name LR - RTC Lock Register */
#define RTC_LR_TCL                               (0x8U)
#define RTC_LR_CRL                               (0x10U)
#define RTC_LR_SRL                               (0x20U)
#define RTC_LR_LRL                               (0x40U)

/*! @name IER - RTC Interrupt Enable Register */
#define RTC_IER_TIIE                             (0x1U)
#define RTC_IER_TOIE                             (0x2U)
#define RTC_IER_TAIE                             (0x4U)
#define RTC_IER_TSIE                             (0x10U)
#define RTC_IER_WPON                             (0x80U)

/*! @name WAR - RTC Write Access Register */
#define RTC_WAR_TSRW                             (0x1U)
#define RTC_WAR_TPRW                             (0x2U)
#define RTC_WAR_TARW                             (0x4U)
#define RTC_WAR_TCRW                             (0x8U)
#define RTC_WAR_CRW                              (0x10U)
#define RTC_WAR_SRW                              (0x20U)
#define RTC_WAR_LRW                              (0x40U)
#define RTC_WAR_IERW                             (0x80U)

/*! @name RAR - RTC Read Access Register */
#define RTC_RAR_TSRR                             (0x1U)
#define RTC_RAR_TPRR                             (0x2U)
#define RTC_RAR_TARR                             (0x4U)
#define RTC_RAR_TCRR                             (0x8U)
#define RTC_RAR_CRR                              (0x10U)
#define RTC_RAR_SRR                              (0x20U)
#define RTC_RAR_LRR                              (0x40U)
#define RTC_RAR_IERR                             (0x80U)


/*!
 * @}
 */ /* end of group RTC_Register_Masks */


/* RTC - Peripheral instance base addresses */
/** Peripheral RTC base address */
#define RTC_BASE                                 (0x4003D000u)
/** Peripheral RTC base pointer */
#define RTC                                      ((RTC_TypeDef *)RTC_BASE)
/** Array initializer of RTC peripheral base addresses */
#define RTC_BASE_ADDRS                           { RTC_BASE }
/** Array initializer of RTC peripheral base pointers */
#define RTC_BASE_PTRS                            { RTC }
/** Interrupt vectors for the RTC peripheral type */
#define RTC_IRQS                                 { RTC_IRQn }
#define RTC_SECONDS_IRQS                         { RTC_Seconds_IRQn }

/*!
 * @}
 */ /* end of group RTC_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- SDHC Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup SDHC_Peripheral_Access_Layer SDHC Peripheral Access Layer
 * @{
 */

/** SDHC - Register Layout Typedef */
typedef struct {
  __IO uint32_t DSADDR;                            /**< DMA System Address register, offset: 0x0 */
  __IO uint32_t BLKATTR;                           /**< Block Attributes register, offset: 0x4 */
  __IO uint32_t CMDARG;                            /**< Command Argument register, offset: 0x8 */
  __IO uint32_t XFERTYP;                           /**< Transfer Type register, offset: 0xC */
  __I  uint32_t CMDRSP[4];                         /**< Command Response 0..Command Response 3, array offset: 0x10, array step: 0x4 */
  __IO uint32_t DATPORT;                           /**< Buffer Data Port register, offset: 0x20 */
  __I  uint32_t PRSSTAT;                           /**< Present State register, offset: 0x24 */
  __IO uint32_t PROCTL;                            /**< Protocol Control register, offset: 0x28 */
  __IO uint32_t SYSCTL;                            /**< System Control register, offset: 0x2C */
  __IO uint32_t IRQSTAT;                           /**< Interrupt Status register, offset: 0x30 */
  __IO uint32_t IRQSTATEN;                         /**< Interrupt Status Enable register, offset: 0x34 */
  __IO uint32_t IRQSIGEN;                          /**< Interrupt Signal Enable register, offset: 0x38 */
  __I  uint32_t AC12ERR;                           /**< Auto CMD12 Error Status Register, offset: 0x3C */
  __I  uint32_t HTCAPBLT;                          /**< Host Controller Capabilities, offset: 0x40 */
  __IO uint32_t WML;                               /**< Watermark Level Register, offset: 0x44 */
       uint8_t RESERVED_0[8];
  __O  uint32_t FEVT;                              /**< Force Event register, offset: 0x50 */
  __I  uint32_t ADMAES;                            /**< ADMA Error Status register, offset: 0x54 */
  __IO uint32_t ADSADDR;                           /**< ADMA System Addressregister, offset: 0x58 */
       uint8_t RESERVED_1[100];
  __IO uint32_t VENDOR;                            /**< Vendor Specific register, offset: 0xC0 */
  __IO uint32_t MMCBOOT;                           /**< MMC Boot register, offset: 0xC4 */
       uint8_t RESERVED_2[52];
  __I  uint32_t HOSTVER;                           /**< Host Controller Version, offset: 0xFC */
} SDHC_TypeDef;

/* ----------------------------------------------------------------------------
   -- SDHC Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup SDHC_Register_Masks SDHC Register Masks
 * @{
 */

/*! @name BLKATTR - Block Attributes register */
#define SDHC_BLKATTR_BLKSIZE_MASK                (0x1FFFU)
#define SDHC_BLKATTR_BLKSIZE_SHIFT               (0U)
#define SDHC_BLKATTR_BLKSIZE(x)                  (((uint32_t)(((uint32_t)(x)) << SDHC_BLKATTR_BLKSIZE_SHIFT)) & SDHC_BLKATTR_BLKSIZE_MASK)
#define SDHC_BLKATTR_BLKCNT_MASK                 (0xFFFF0000U)
#define SDHC_BLKATTR_BLKCNT_SHIFT                (16U)
#define SDHC_BLKATTR_BLKCNT(x)                   (((uint32_t)(((uint32_t)(x)) << SDHC_BLKATTR_BLKCNT_SHIFT)) & SDHC_BLKATTR_BLKCNT_MASK)

/*! @name XFERTYP - Transfer Type register */
#define SDHC_XFERTYP_DMAEN                       (0x1U)
#define SDHC_XFERTYP_BCEN                        (0x2U)
#define SDHC_XFERTYP_AC12EN                      (0x4U)
#define SDHC_XFERTYP_DTDSEL                      (0x10U)
#define SDHC_XFERTYP_MSBSEL                      (0x20U)
#define SDHC_XFERTYP_RSPTYP_MASK                 (0x30000U)
#define SDHC_XFERTYP_RSPTYP_SHIFT                (16U)
#define SDHC_XFERTYP_RSPTYP(x)                   (((uint32_t)(((uint32_t)(x)) << SDHC_XFERTYP_RSPTYP_SHIFT)) & SDHC_XFERTYP_RSPTYP_MASK)
#define SDHC_XFERTYP_CCCEN                       (0x80000U)
#define SDHC_XFERTYP_CICEN                       (0x100000U)
#define SDHC_XFERTYP_DPSEL                       (0x200000U)
#define SDHC_XFERTYP_CMDTYP_MASK                 (0xC00000U)
#define SDHC_XFERTYP_CMDTYP_SHIFT                (22U)
#define SDHC_XFERTYP_CMDTYP(x)                   (((uint32_t)(((uint32_t)(x)) << SDHC_XFERTYP_CMDTYP_SHIFT)) & SDHC_XFERTYP_CMDTYP_MASK)
#define SDHC_XFERTYP_CMDINX_MASK                 (0x3F000000U)
#define SDHC_XFERTYP_CMDINX_SHIFT                (24U)
#define SDHC_XFERTYP_CMDINX(x)                   (((uint32_t)(((uint32_t)(x)) << SDHC_XFERTYP_CMDINX_SHIFT)) & SDHC_XFERTYP_CMDINX_MASK)

/* The count of SDHC_CMDRSP */
#define SDHC_CMDRSP_COUNT                        (4U)

/*! @name PRSSTAT - Present State register */
#define SDHC_PRSSTAT_CIHB                        (0x1U)
#define SDHC_PRSSTAT_CDIHB                       (0x2U)
#define SDHC_PRSSTAT_DLA                         (0x4U)
#define SDHC_PRSSTAT_SDSTB                       (0x8U)
#define SDHC_PRSSTAT_IPGOFF                      (0x10U)
#define SDHC_PRSSTAT_HCKOFF                      (0x20U)
#define SDHC_PRSSTAT_PEROFF                      (0x40U)
#define SDHC_PRSSTAT_SDOFF                       (0x80U)
#define SDHC_PRSSTAT_WTA                         (0x100U)
#define SDHC_PRSSTAT_RTA                         (0x200U)
#define SDHC_PRSSTAT_BWEN                        (0x400U)
#define SDHC_PRSSTAT_BREN                        (0x800U)
#define SDHC_PRSSTAT_CINS                        (0x10000U)
#define SDHC_PRSSTAT_CLSL                        (0x800000U)
#define SDHC_PRSSTAT_DLSL_MASK                   (0xFF000000U)
#define SDHC_PRSSTAT_DLSL_SHIFT                  (24U)
#define SDHC_PRSSTAT_DLSL(x)                     (((uint32_t)(((uint32_t)(x)) << SDHC_PRSSTAT_DLSL_SHIFT)) & SDHC_PRSSTAT_DLSL_MASK)

/*! @name PROCTL - Protocol Control register */
#define SDHC_PROCTL_LCTL                         (0x1U)
#define SDHC_PROCTL_DTW_MASK                     (0x6U)
#define SDHC_PROCTL_DTW_SHIFT                    (1U)
#define SDHC_PROCTL_DTW(x)                       (((uint32_t)(((uint32_t)(x)) << SDHC_PROCTL_DTW_SHIFT)) & SDHC_PROCTL_DTW_MASK)
#define SDHC_PROCTL_D3CD                         (0x8U)
#define SDHC_PROCTL_EMODE_MASK                   (0x30U)
#define SDHC_PROCTL_EMODE_SHIFT                  (4U)
#define SDHC_PROCTL_EMODE(x)                     (((uint32_t)(((uint32_t)(x)) << SDHC_PROCTL_EMODE_SHIFT)) & SDHC_PROCTL_EMODE_MASK)
#define SDHC_PROCTL_CDTL                         (0x40U)
#define SDHC_PROCTL_CDSS                         (0x80U)
#define SDHC_PROCTL_DMAS_MASK                    (0x300U)
#define SDHC_PROCTL_DMAS_SHIFT                   (8U)
#define SDHC_PROCTL_DMAS(x)                      (((uint32_t)(((uint32_t)(x)) << SDHC_PROCTL_DMAS_SHIFT)) & SDHC_PROCTL_DMAS_MASK)
#define SDHC_PROCTL_SABGREQ                      (0x10000U)
#define SDHC_PROCTL_CREQ                         (0x20000U)
#define SDHC_PROCTL_RWCTL                        (0x40000U)
#define SDHC_PROCTL_IABG                         (0x80000U)
#define SDHC_PROCTL_WECINT                       (0x1000000U)
#define SDHC_PROCTL_WECINS                       (0x2000000U)
#define SDHC_PROCTL_WECRM                        (0x4000000U)

/*! @name SYSCTL - System Control register */
#define SDHC_SYSCTL_IPGEN                        (0x1U)
#define SDHC_SYSCTL_HCKEN                        (0x2U)
#define SDHC_SYSCTL_PEREN                        (0x4U)
#define SDHC_SYSCTL_SDCLKEN                      (0x8U)
#define SDHC_SYSCTL_DVS_MASK                     (0xF0U)
#define SDHC_SYSCTL_DVS_SHIFT                    (4U)
#define SDHC_SYSCTL_DVS(x)                       (((uint32_t)(((uint32_t)(x)) << SDHC_SYSCTL_DVS_SHIFT)) & SDHC_SYSCTL_DVS_MASK)
#define SDHC_SYSCTL_SDCLKFS_MASK                 (0xFF00U)
#define SDHC_SYSCTL_SDCLKFS_SHIFT                (8U)
#define SDHC_SYSCTL_SDCLKFS(x)                   (((uint32_t)(((uint32_t)(x)) << SDHC_SYSCTL_SDCLKFS_SHIFT)) & SDHC_SYSCTL_SDCLKFS_MASK)
#define SDHC_SYSCTL_DTOCV_MASK                   (0xF0000U)
#define SDHC_SYSCTL_DTOCV_SHIFT                  (16U)
#define SDHC_SYSCTL_DTOCV(x)                     (((uint32_t)(((uint32_t)(x)) << SDHC_SYSCTL_DTOCV_SHIFT)) & SDHC_SYSCTL_DTOCV_MASK)
#define SDHC_SYSCTL_RSTA                         (0x1000000U)
#define SDHC_SYSCTL_RSTC                         (0x2000000U)
#define SDHC_SYSCTL_RSTD                         (0x4000000U)
#define SDHC_SYSCTL_INITA                        (0x8000000U)

/*! @name IRQSTAT - Interrupt Status register */
#define SDHC_IRQSTAT_CC                          (0x1U)
#define SDHC_IRQSTAT_TC                          (0x2U)
#define SDHC_IRQSTAT_BGE                         (0x4U)
#define SDHC_IRQSTAT_DINT                        (0x8U)
#define SDHC_IRQSTAT_BWR                         (0x10U)
#define SDHC_IRQSTAT_BRR                         (0x20U)
#define SDHC_IRQSTAT_CINS                        (0x40U)
#define SDHC_IRQSTAT_CRM                         (0x80U)
#define SDHC_IRQSTAT_CINT                        (0x100U)
#define SDHC_IRQSTAT_CTOE                        (0x10000U)
#define SDHC_IRQSTAT_CCE                         (0x20000U)
#define SDHC_IRQSTAT_CEBE                        (0x40000U)
#define SDHC_IRQSTAT_CIE                         (0x80000U)
#define SDHC_IRQSTAT_DTOE                        (0x100000U)
#define SDHC_IRQSTAT_DCE                         (0x200000U)
#define SDHC_IRQSTAT_DEBE                        (0x400000U)
#define SDHC_IRQSTAT_AC12E                       (0x1000000U)
#define SDHC_IRQSTAT_DMAE                        (0x10000000U)

/*! @name IRQSTATEN - Interrupt Status Enable register */
#define SDHC_IRQSTATEN_CCSEN                     (0x1U)
#define SDHC_IRQSTATEN_TCSEN                     (0x2U)
#define SDHC_IRQSTATEN_BGESEN                    (0x4U)
#define SDHC_IRQSTATEN_DINTSEN                   (0x8U)
#define SDHC_IRQSTATEN_BWRSEN                    (0x10U)
#define SDHC_IRQSTATEN_BRRSEN                    (0x20U)
#define SDHC_IRQSTATEN_CINSEN                    (0x40U)
#define SDHC_IRQSTATEN_CRMSEN                    (0x80U)
#define SDHC_IRQSTATEN_CINTSEN                   (0x100U)
#define SDHC_IRQSTATEN_CTOESEN                   (0x10000U)
#define SDHC_IRQSTATEN_CCESEN                    (0x20000U)
#define SDHC_IRQSTATEN_CEBESEN                   (0x40000U)
#define SDHC_IRQSTATEN_CIESEN                    (0x80000U)
#define SDHC_IRQSTATEN_DTOESEN                   (0x100000U)
#define SDHC_IRQSTATEN_DCESEN                    (0x200000U)
#define SDHC_IRQSTATEN_DEBESEN                   (0x400000U)
#define SDHC_IRQSTATEN_AC12ESEN                  (0x1000000U)
#define SDHC_IRQSTATEN_DMAESEN                   (0x10000000U)

/*! @name IRQSIGEN - Interrupt Signal Enable register */
#define SDHC_IRQSIGEN_CCIEN                      (0x1U)
#define SDHC_IRQSIGEN_TCIEN                      (0x2U)
#define SDHC_IRQSIGEN_BGEIEN                     (0x4U)
#define SDHC_IRQSIGEN_DINTIEN                    (0x8U)
#define SDHC_IRQSIGEN_BWRIEN                     (0x10U)
#define SDHC_IRQSIGEN_BRRIEN                     (0x20U)
#define SDHC_IRQSIGEN_CINSIEN                    (0x40U)
#define SDHC_IRQSIGEN_CRMIEN                     (0x80U)
#define SDHC_IRQSIGEN_CINTIEN                    (0x100U)
#define SDHC_IRQSIGEN_CTOEIEN                    (0x10000U)
#define SDHC_IRQSIGEN_CCEIEN                     (0x20000U)
#define SDHC_IRQSIGEN_CEBEIEN                    (0x40000U)
#define SDHC_IRQSIGEN_CIEIEN                     (0x80000U)
#define SDHC_IRQSIGEN_DTOEIEN                    (0x100000U)
#define SDHC_IRQSIGEN_DCEIEN                     (0x200000U)
#define SDHC_IRQSIGEN_DEBEIEN                    (0x400000U)
#define SDHC_IRQSIGEN_AC12EIEN                   (0x1000000U)
#define SDHC_IRQSIGEN_DMAEIEN                    (0x10000000U)

/*! @name AC12ERR - Auto CMD12 Error Status Register */
#define SDHC_AC12ERR_AC12NE                      (0x1U)
#define SDHC_AC12ERR_AC12TOE                     (0x2U)
#define SDHC_AC12ERR_AC12EBE                     (0x4U)
#define SDHC_AC12ERR_AC12CE                      (0x8U)
#define SDHC_AC12ERR_AC12IE                      (0x10U)
#define SDHC_AC12ERR_CNIBAC12E                   (0x80U)

/*! @name HTCAPBLT - Host Controller Capabilities */
#define SDHC_HTCAPBLT_MBL_MASK                   (0x70000U)
#define SDHC_HTCAPBLT_MBL_SHIFT                  (16U)
#define SDHC_HTCAPBLT_MBL(x)                     (((uint32_t)(((uint32_t)(x)) << SDHC_HTCAPBLT_MBL_SHIFT)) & SDHC_HTCAPBLT_MBL_MASK)
#define SDHC_HTCAPBLT_ADMAS                      (0x100000U)
#define SDHC_HTCAPBLT_HSS                        (0x200000U)
#define SDHC_HTCAPBLT_DMAS                       (0x400000U)
#define SDHC_HTCAPBLT_SRS                        (0x800000U)
#define SDHC_HTCAPBLT_VS33                       (0x1000000U)

/*! @name WML - Watermark Level Register */
#define SDHC_WML_RDWML_MASK                      (0xFFU)
#define SDHC_WML_RDWML_SHIFT                     (0U)
#define SDHC_WML_RDWML(x)                        (((uint32_t)(((uint32_t)(x)) << SDHC_WML_RDWML_SHIFT)) & SDHC_WML_RDWML_MASK)
#define SDHC_WML_WRWML_MASK                      (0xFF0000U)
#define SDHC_WML_WRWML_SHIFT                     (16U)
#define SDHC_WML_WRWML(x)                        (((uint32_t)(((uint32_t)(x)) << SDHC_WML_WRWML_SHIFT)) & SDHC_WML_WRWML_MASK)

/*! @name FEVT - Force Event register */
#define SDHC_FEVT_AC12NE                         (0x1U)
#define SDHC_FEVT_AC12TOE                        (0x2U)
#define SDHC_FEVT_AC12CE                         (0x4U)
#define SDHC_FEVT_AC12EBE                        (0x8U)
#define SDHC_FEVT_AC12IE                         (0x10U)
#define SDHC_FEVT_CNIBAC12E                      (0x80U)
#define SDHC_FEVT_CTOE                           (0x10000U)
#define SDHC_FEVT_CCE                            (0x20000U)
#define SDHC_FEVT_CEBE                           (0x40000U)
#define SDHC_FEVT_CIE                            (0x80000U)
#define SDHC_FEVT_DTOE                           (0x100000U)
#define SDHC_FEVT_DCE                            (0x200000U)
#define SDHC_FEVT_DEBE                           (0x400000U)
#define SDHC_FEVT_AC12E                          (0x1000000U)
#define SDHC_FEVT_DMAE                           (0x10000000U)
#define SDHC_FEVT_CINT                           (0x80000000U)

/*! @name ADMAES - ADMA Error Status register */
#define SDHC_ADMAES_ADMAES_MASK                  (0x3U)
#define SDHC_ADMAES_ADMAES_SHIFT                 (0U)
#define SDHC_ADMAES_ADMAES(x)                    (((uint32_t)(((uint32_t)(x)) << SDHC_ADMAES_ADMAES_SHIFT)) & SDHC_ADMAES_ADMAES_MASK)
#define SDHC_ADMAES_ADMALME                      (0x4U)
#define SDHC_ADMAES_ADMADCE                      (0x8U)

/*! @name VENDOR - Vendor Specific register */
#define SDHC_VENDOR_EXTDMAEN                     (0x1U)
#define SDHC_VENDOR_EXBLKNU                      (0x2U)
#define SDHC_VENDOR_INTSTVAL_MASK                (0xFF0000U)
#define SDHC_VENDOR_INTSTVAL_SHIFT               (16U)
#define SDHC_VENDOR_INTSTVAL(x)                  (((uint32_t)(((uint32_t)(x)) << SDHC_VENDOR_INTSTVAL_SHIFT)) & SDHC_VENDOR_INTSTVAL_MASK)

/*! @name MMCBOOT - MMC Boot register */
#define SDHC_MMCBOOT_DTOCVACK_MASK               (0xFU)
#define SDHC_MMCBOOT_DTOCVACK_SHIFT              (0U)
#define SDHC_MMCBOOT_DTOCVACK(x)                 (((uint32_t)(((uint32_t)(x)) << SDHC_MMCBOOT_DTOCVACK_SHIFT)) & SDHC_MMCBOOT_DTOCVACK_MASK)
#define SDHC_MMCBOOT_BOOTACK                     (0x10U)
#define SDHC_MMCBOOT_BOOTMODE                    (0x20U)
#define SDHC_MMCBOOT_BOOTEN                      (0x40U)
#define SDHC_MMCBOOT_AUTOSABGEN                  (0x80U)
#define SDHC_MMCBOOT_BOOTBLKCNT_MASK             (0xFFFF0000U)
#define SDHC_MMCBOOT_BOOTBLKCNT_SHIFT            (16U)
#define SDHC_MMCBOOT_BOOTBLKCNT(x)               (((uint32_t)(((uint32_t)(x)) << SDHC_MMCBOOT_BOOTBLKCNT_SHIFT)) & SDHC_MMCBOOT_BOOTBLKCNT_MASK)

/*! @name HOSTVER - Host Controller Version */
#define SDHC_HOSTVER_SVN_MASK                    (0xFFU)
#define SDHC_HOSTVER_SVN_SHIFT                   (0U)
#define SDHC_HOSTVER_SVN(x)                      (((uint32_t)(((uint32_t)(x)) << SDHC_HOSTVER_SVN_SHIFT)) & SDHC_HOSTVER_SVN_MASK)
#define SDHC_HOSTVER_VVN_MASK                    (0xFF00U)
#define SDHC_HOSTVER_VVN_SHIFT                   (8U)
#define SDHC_HOSTVER_VVN(x)                      (((uint32_t)(((uint32_t)(x)) << SDHC_HOSTVER_VVN_SHIFT)) & SDHC_HOSTVER_VVN_MASK)


/*!
 * @}
 */ /* end of group SDHC_Register_Masks */


/* SDHC - Peripheral instance base addresses */
/** Peripheral SDHC base address */
#define SDHC_BASE                                (0x400B1000u)
/** Peripheral SDHC base pointer */
#define SDHC                                     ((SDHC_TypeDef *)SDHC_BASE)
/** Array initializer of SDHC peripheral base addresses */
#define SDHC_BASE_ADDRS                          { SDHC_BASE }
/** Array initializer of SDHC peripheral base pointers */
#define SDHC_BASE_PTRS                           { SDHC }
/** Interrupt vectors for the SDHC peripheral type */
#define SDHC_IRQS                                { SDHC_IRQn }

/*!
 * @}
 */ /* end of group SDHC_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- SIM Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup SIM_Peripheral_Access_Layer SIM Peripheral Access Layer
 * @{
 */

/** SIM - Register Layout Typedef */
typedef struct {
  __IO uint32_t SOPT1;                             /**< System Options Register 1, offset: 0x0 */
  __IO uint32_t SOPT1CFG;                          /**< SOPT1 Configuration Register, offset: 0x4 */
       uint8_t RESERVED_0[4092];
  __IO uint32_t SOPT2;                             /**< System Options Register 2, offset: 0x1004 */
       uint8_t RESERVED_1[4];
  __IO uint32_t SOPT4;                             /**< System Options Register 4, offset: 0x100C */
  __IO uint32_t SOPT5;                             /**< System Options Register 5, offset: 0x1010 */
       uint8_t RESERVED_2[4];
  __IO uint32_t SOPT7;                             /**< System Options Register 7, offset: 0x1018 */
       uint8_t RESERVED_3[8];
  __I  uint32_t SDID;                              /**< System Device Identification Register, offset: 0x1024 */
  __IO uint32_t SCGC1;                             /**< System Clock Gating Control Register 1, offset: 0x1028 */
  __IO uint32_t SCGC2;                             /**< System Clock Gating Control Register 2, offset: 0x102C */
  __IO uint32_t SCGC3;                             /**< System Clock Gating Control Register 3, offset: 0x1030 */
  __IO uint32_t SCGC4;                             /**< System Clock Gating Control Register 4, offset: 0x1034 */
  __IO uint32_t SCGC5;                             /**< System Clock Gating Control Register 5, offset: 0x1038 */
  __IO uint32_t SCGC6;                             /**< System Clock Gating Control Register 6, offset: 0x103C */
  __IO uint32_t SCGC7;                             /**< System Clock Gating Control Register 7, offset: 0x1040 */
  __IO uint32_t CLKDIV1;                           /**< System Clock Divider Register 1, offset: 0x1044 */
  __IO uint32_t CLKDIV2;                           /**< System Clock Divider Register 2, offset: 0x1048 */
  __IO uint32_t FCFG1;                             /**< Flash Configuration Register 1, offset: 0x104C */
  __I  uint32_t FCFG2;                             /**< Flash Configuration Register 2, offset: 0x1050 */
  __I  uint32_t UIDH;                              /**< Unique Identification Register High, offset: 0x1054 */
  __I  uint32_t UIDMH;                             /**< Unique Identification Register Mid-High, offset: 0x1058 */
  __I  uint32_t UIDML;                             /**< Unique Identification Register Mid Low, offset: 0x105C */
  __I  uint32_t UIDL;                              /**< Unique Identification Register Low, offset: 0x1060 */
} SIM_TypeDef;

/* ----------------------------------------------------------------------------
   -- SIM Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup SIM_Register_Masks SIM Register Masks
 * @{
 */

/*! @name SOPT1 - System Options Register 1 */
#define SIM_SOPT1_RAMSIZE_MASK                   (0xF000U)
#define SIM_SOPT1_RAMSIZE_SHIFT                  (12U)
#define SIM_SOPT1_RAMSIZE(x)                     (((uint32_t)(((uint32_t)(x)) << SIM_SOPT1_RAMSIZE_SHIFT)) & SIM_SOPT1_RAMSIZE_MASK)
#define SIM_SOPT1_OSC32KSEL_MASK                 (0xC0000U)
#define SIM_SOPT1_OSC32KSEL_SHIFT                (18U)
#define SIM_SOPT1_OSC32KSEL(x)                   (((uint32_t)(((uint32_t)(x)) << SIM_SOPT1_OSC32KSEL_SHIFT)) & SIM_SOPT1_OSC32KSEL_MASK)
#define SIM_SOPT1_USBVSTBY                       (0x20000000U)
#define SIM_SOPT1_USBSSTBY                       (0x40000000U)
#define SIM_SOPT1_USBREGEN                       (0x80000000U)

/*! @name SOPT1CFG - SOPT1 Configuration Register */
#define SIM_SOPT1CFG_URWE                        (0x1000000U)
#define SIM_SOPT1CFG_UVSWE                       (0x2000000U)
#define SIM_SOPT1CFG_USSWE                       (0x4000000U)

/*! @name SOPT2 - System Options Register 2 */
#define SIM_SOPT2_RTCCLKOUTSEL                   (0x10U)
#define SIM_SOPT2_CLKOUTSEL_MASK                 (0xE0U)
#define SIM_SOPT2_CLKOUTSEL_SHIFT                (5U)
#define SIM_SOPT2_CLKOUTSEL(x)                   (((uint32_t)(((uint32_t)(x)) << SIM_SOPT2_CLKOUTSEL_SHIFT)) & SIM_SOPT2_CLKOUTSEL_MASK)
#define SIM_SOPT2_FBSL_MASK                      (0x300U)
#define SIM_SOPT2_FBSL_SHIFT                     (8U)
#define SIM_SOPT2_FBSL(x)                        (((uint32_t)(((uint32_t)(x)) << SIM_SOPT2_FBSL_SHIFT)) & SIM_SOPT2_FBSL_MASK)
#define SIM_SOPT2_PTD7PAD                        (0x800U)
#define SIM_SOPT2_TRACECLKSEL                    (0x1000U)
#define SIM_SOPT2_PLLFLLSEL_MASK                 (0x30000U)
#define SIM_SOPT2_PLLFLLSEL_SHIFT                (16U)
#define SIM_SOPT2_PLLFLLSEL(x)                   (((uint32_t)(((uint32_t)(x)) << SIM_SOPT2_PLLFLLSEL_SHIFT)) & SIM_SOPT2_PLLFLLSEL_MASK)
#define SIM_SOPT2_PLLFLLSEL_MCGFLL               SIM_SOPT2_PLLFLLSEL(0)
#define SIM_SOPT2_PLLFLLSEL_MCGPLL               SIM_SOPT2_PLLFLLSEL(1)
#define SIM_SOPT2_PLLFLLSEL_IRC48M               SIM_SOPT2_PLLFLLSEL(3)
#define SIM_SOPT2_USBSRC                         (0x40000U)
#define SIM_SOPT2_RMIISRC                        (0x80000U)
#define SIM_SOPT2_TIMESRC_MASK                   (0x300000U)
#define SIM_SOPT2_TIMESRC_SHIFT                  (20U)
#define SIM_SOPT2_TIMESRC(x)                     (((uint32_t)(((uint32_t)(x)) << SIM_SOPT2_TIMESRC_SHIFT)) & SIM_SOPT2_TIMESRC_MASK)
#define SIM_SOPT2_SDHCSRC_MASK                   (0x30000000U)
#define SIM_SOPT2_SDHCSRC_SHIFT                  (28U)
#define SIM_SOPT2_SDHCSRC(x)                     (((uint32_t)(((uint32_t)(x)) << SIM_SOPT2_SDHCSRC_SHIFT)) & SIM_SOPT2_SDHCSRC_MASK)

/*! @name SOPT4 - System Options Register 4 */
#define SIM_SOPT4_FTM0FLT0                       (0x1U)
#define SIM_SOPT4_FTM0FLT1                       (0x2U)
#define SIM_SOPT4_FTM0FLT2                       (0x4U)
#define SIM_SOPT4_FTM1FLT0                       (0x10U)
#define SIM_SOPT4_FTM2FLT0                       (0x100U)
#define SIM_SOPT4_FTM3FLT0                       (0x1000U)
#define SIM_SOPT4_FTM1CH0SRC_MASK                (0xC0000U)
#define SIM_SOPT4_FTM1CH0SRC_SHIFT               (18U)
#define SIM_SOPT4_FTM1CH0SRC(x)                  (((uint32_t)(((uint32_t)(x)) << SIM_SOPT4_FTM1CH0SRC_SHIFT)) & SIM_SOPT4_FTM1CH0SRC_MASK)
#define SIM_SOPT4_FTM2CH0SRC_MASK                (0x300000U)
#define SIM_SOPT4_FTM2CH0SRC_SHIFT               (20U)
#define SIM_SOPT4_FTM2CH0SRC(x)                  (((uint32_t)(((uint32_t)(x)) << SIM_SOPT4_FTM2CH0SRC_SHIFT)) & SIM_SOPT4_FTM2CH0SRC_MASK)
#define SIM_SOPT4_FTM0CLKSEL                     (0x1000000U)
#define SIM_SOPT4_FTM1CLKSEL                     (0x2000000U)
#define SIM_SOPT4_FTM2CLKSEL                     (0x4000000U)
#define SIM_SOPT4_FTM3CLKSEL                     (0x8000000U)
#define SIM_SOPT4_FTM0TRG0SRC                    (0x10000000U)
#define SIM_SOPT4_FTM0TRG1SRC                    (0x20000000U)
#define SIM_SOPT4_FTM3TRG0SRC                    (0x40000000U)
#define SIM_SOPT4_FTM3TRG1SRC                    (0x80000000U)

/*! @name SOPT5 - System Options Register 5 */
#define SIM_SOPT5_UART0TXSRC_MASK                (0x3U)
#define SIM_SOPT5_UART0TXSRC_SHIFT               (0U)
#define SIM_SOPT5_UART0TXSRC(x)                  (((uint32_t)(((uint32_t)(x)) << SIM_SOPT5_UART0TXSRC_SHIFT)) & SIM_SOPT5_UART0TXSRC_MASK)
#define SIM_SOPT5_UART0RXSRC_MASK                (0xCU)
#define SIM_SOPT5_UART0RXSRC_SHIFT               (2U)
#define SIM_SOPT5_UART0RXSRC(x)                  (((uint32_t)(((uint32_t)(x)) << SIM_SOPT5_UART0RXSRC_SHIFT)) & SIM_SOPT5_UART0RXSRC_MASK)
#define SIM_SOPT5_UART1TXSRC_MASK                (0x30U)
#define SIM_SOPT5_UART1TXSRC_SHIFT               (4U)
#define SIM_SOPT5_UART1TXSRC(x)                  (((uint32_t)(((uint32_t)(x)) << SIM_SOPT5_UART1TXSRC_SHIFT)) & SIM_SOPT5_UART1TXSRC_MASK)
#define SIM_SOPT5_UART1RXSRC_MASK                (0xC0U)
#define SIM_SOPT5_UART1RXSRC_SHIFT               (6U)
#define SIM_SOPT5_UART1RXSRC(x)                  (((uint32_t)(((uint32_t)(x)) << SIM_SOPT5_UART1RXSRC_SHIFT)) & SIM_SOPT5_UART1RXSRC_MASK)

/*! @name SOPT7 - System Options Register 7 */
#define SIM_SOPT7_ADC0TRGSEL_MASK                (0xFU)
#define SIM_SOPT7_ADC0TRGSEL_SHIFT               (0U)
#define SIM_SOPT7_ADC0TRGSEL(x)                  (((uint32_t)(((uint32_t)(x)) << SIM_SOPT7_ADC0TRGSEL_SHIFT)) & SIM_SOPT7_ADC0TRGSEL_MASK)
#define SIM_SOPT7_ADC0PRETRGSEL                  (0x10U)
#define SIM_SOPT7_ADC0ALTTRGEN                   (0x80U)
#define SIM_SOPT7_ADC1TRGSEL_MASK                (0xF00U)
#define SIM_SOPT7_ADC1TRGSEL_SHIFT               (8U)
#define SIM_SOPT7_ADC1TRGSEL(x)                  (((uint32_t)(((uint32_t)(x)) << SIM_SOPT7_ADC1TRGSEL_SHIFT)) & SIM_SOPT7_ADC1TRGSEL_MASK)
#define SIM_SOPT7_ADC1PRETRGSEL                  (0x1000U)
#define SIM_SOPT7_ADC1ALTTRGEN                   (0x8000U)

/*! @name SDID - System Device Identification Register */
#define SIM_SDID_PINID_MASK                      (0xFU)
#define SIM_SDID_PINID_SHIFT                     (0U)
#define SIM_SDID_PINID(x)                        (((uint32_t)(((uint32_t)(x)) << SIM_SDID_PINID_SHIFT)) & SIM_SDID_PINID_MASK)
#define SIM_SDID_FAMID_MASK                      (0x70U)
#define SIM_SDID_FAMID_SHIFT                     (4U)
#define SIM_SDID_FAMID(x)                        (((uint32_t)(((uint32_t)(x)) << SIM_SDID_FAMID_SHIFT)) & SIM_SDID_FAMID_MASK)
#define SIM_SDID_DIEID_MASK                      (0xF80U)
#define SIM_SDID_DIEID_SHIFT                     (7U)
#define SIM_SDID_DIEID(x)                        (((uint32_t)(((uint32_t)(x)) << SIM_SDID_DIEID_SHIFT)) & SIM_SDID_DIEID_MASK)
#define SIM_SDID_REVID_MASK                      (0xF000U)
#define SIM_SDID_REVID_SHIFT                     (12U)
#define SIM_SDID_REVID(x)                        (((uint32_t)(((uint32_t)(x)) << SIM_SDID_REVID_SHIFT)) & SIM_SDID_REVID_MASK)
#define SIM_SDID_SERIESID_MASK                   (0xF00000U)
#define SIM_SDID_SERIESID_SHIFT                  (20U)
#define SIM_SDID_SERIESID(x)                     (((uint32_t)(((uint32_t)(x)) << SIM_SDID_SERIESID_SHIFT)) & SIM_SDID_SERIESID_MASK)
#define SIM_SDID_SUBFAMID_MASK                   (0xF000000U)
#define SIM_SDID_SUBFAMID_SHIFT                  (24U)
#define SIM_SDID_SUBFAMID(x)                     (((uint32_t)(((uint32_t)(x)) << SIM_SDID_SUBFAMID_SHIFT)) & SIM_SDID_SUBFAMID_MASK)
#define SIM_SDID_FAMILYID_MASK                   (0xF0000000U)
#define SIM_SDID_FAMILYID_SHIFT                  (28U)
#define SIM_SDID_FAMILYID(x)                     (((uint32_t)(((uint32_t)(x)) << SIM_SDID_FAMILYID_SHIFT)) & SIM_SDID_FAMILYID_MASK)

/*! @name SCGC1 - System Clock Gating Control Register 1 */
#define SIM_SCGC1_I2C2                           (0x40U)
#define SIM_SCGC1_UART4                          (0x400U)
#define SIM_SCGC1_UART5                          (0x800U)

/*! @name SCGC2 - System Clock Gating Control Register 2 */
#define SIM_SCGC2_ENET                           (0x1U)
#define SIM_SCGC2_DAC0                           (0x1000U)
#define SIM_SCGC2_DAC1                           (0x2000U)

/*! @name SCGC3 - System Clock Gating Control Register 3 */
#define SIM_SCGC3_RNGA                           (0x1U)
#define SIM_SCGC3_SPI2                           (0x1000U)
#define SIM_SCGC3_SDHC                           (0x20000U)
#define SIM_SCGC3_FTM2                           (0x1000000U)
#define SIM_SCGC3_FTM3                           (0x2000000U)
#define SIM_SCGC3_ADC1                           (0x8000000U)

/*! @name SCGC4 - System Clock Gating Control Register 4 */
#define SIM_SCGC4_EWM                            (0x2U)
#define SIM_SCGC4_CMT                            (0x4U)
#define SIM_SCGC4_I2C0                           (0x40U)
#define SIM_SCGC4_I2C1                           (0x80U)
#define SIM_SCGC4_UART0                          (0x400U)
#define SIM_SCGC4_UART1                          (0x800U)
#define SIM_SCGC4_UART2                          (0x1000U)
#define SIM_SCGC4_UART3                          (0x2000U)
#define SIM_SCGC4_USBOTG                         (0x40000U)
#define SIM_SCGC4_CMP                            (0x80000U)
#define SIM_SCGC4_VREF                           (0x100000U)

/*! @name SCGC5 - System Clock Gating Control Register 5 */
#define SIM_SCGC5_LPTMR                          (0x1U)
#define SIM_SCGC5_PORTA                          (0x200U)
#define SIM_SCGC5_PORTB                          (0x400U)
#define SIM_SCGC5_PORTC                          (0x800U)
#define SIM_SCGC5_PORTD                          (0x1000U)
#define SIM_SCGC5_PORTE                          (0x2000U)

/*! @name SCGC6 - System Clock Gating Control Register 6 */
#define SIM_SCGC6_FTF                            (0x1U)
#define SIM_SCGC6_DMAMUX                         (0x2U)
#define SIM_SCGC6_FLEXCAN0                       (0x10U)
#define SIM_SCGC6_RNGA                           (0x200U)
#define SIM_SCGC6_SPI0                           (0x1000U)
#define SIM_SCGC6_SPI1                           (0x2000U)
#define SIM_SCGC6_I2S                            (0x8000U)
#define SIM_SCGC6_CRC                            (0x40000U)
#define SIM_SCGC6_USBDCD                         (0x200000U)
#define SIM_SCGC6_PDB                            (0x400000U)
#define SIM_SCGC6_PIT                            (0x800000U)
#define SIM_SCGC6_FTM0                           (0x1000000U)
#define SIM_SCGC6_FTM1                           (0x2000000U)
#define SIM_SCGC6_FTM2                           (0x4000000U)
#define SIM_SCGC6_ADC0                           (0x8000000U)
#define SIM_SCGC6_RTC                            (0x20000000U)
#define SIM_SCGC6_DAC0                           (0x80000000U)

/*! @name SCGC7 - System Clock Gating Control Register 7 */
#define SIM_SCGC7_FLEXBUS                        (0x1U)
#define SIM_SCGC7_DMA                            (0x2U)
#define SIM_SCGC7_MPU                            (0x4U)

/*! @name CLKDIV1 - System Clock Divider Register 1 */
#define SIM_CLKDIV1_OUTDIV4_MASK                 (0xF0000U)
#define SIM_CLKDIV1_OUTDIV4_SHIFT                (16U)
#define SIM_CLKDIV1_OUTDIV4(x)                   (((uint32_t)(((uint32_t)(x)) << SIM_CLKDIV1_OUTDIV4_SHIFT)) & SIM_CLKDIV1_OUTDIV4_MASK)
#define SIM_CLKDIV1_OUTDIV3_MASK                 (0xF00000U)
#define SIM_CLKDIV1_OUTDIV3_SHIFT                (20U)
#define SIM_CLKDIV1_OUTDIV3(x)                   (((uint32_t)(((uint32_t)(x)) << SIM_CLKDIV1_OUTDIV3_SHIFT)) & SIM_CLKDIV1_OUTDIV3_MASK)
#define SIM_CLKDIV1_OUTDIV2_MASK                 (0xF000000U)
#define SIM_CLKDIV1_OUTDIV2_SHIFT                (24U)
#define SIM_CLKDIV1_OUTDIV2(x)                   (((uint32_t)(((uint32_t)(x)) << SIM_CLKDIV1_OUTDIV2_SHIFT)) & SIM_CLKDIV1_OUTDIV2_MASK)
#define SIM_CLKDIV1_OUTDIV1_MASK                 (0xF0000000U)
#define SIM_CLKDIV1_OUTDIV1_SHIFT                (28U)
#define SIM_CLKDIV1_OUTDIV1(x)                   (((uint32_t)(((uint32_t)(x)) << SIM_CLKDIV1_OUTDIV1_SHIFT)) & SIM_CLKDIV1_OUTDIV1_MASK)

/*! @name CLKDIV2 - System Clock Divider Register 2 */
#define SIM_CLKDIV2_USBFRAC                      (0x1U)
#define SIM_CLKDIV2_USBDIV_MASK                  (0xEU)
#define SIM_CLKDIV2_USBDIV_SHIFT                 (1U)
#define SIM_CLKDIV2_USBDIV(x)                    (((uint32_t)(((uint32_t)(x)) << SIM_CLKDIV2_USBDIV_SHIFT)) & SIM_CLKDIV2_USBDIV_MASK)

/*! @name FCFG1 - Flash Configuration Register 1 */
#define SIM_FCFG1_FLASHDIS                       (0x1U)
#define SIM_FCFG1_FLASHDOZE                      (0x2U)
#define SIM_FCFG1_DEPART_MASK                    (0xF00U)
#define SIM_FCFG1_DEPART_SHIFT                   (8U)
#define SIM_FCFG1_DEPART(x)                      (((uint32_t)(((uint32_t)(x)) << SIM_FCFG1_DEPART_SHIFT)) & SIM_FCFG1_DEPART_MASK)
#define SIM_FCFG1_EESIZE_MASK                    (0xF0000U)
#define SIM_FCFG1_EESIZE_SHIFT                   (16U)
#define SIM_FCFG1_EESIZE(x)                      (((uint32_t)(((uint32_t)(x)) << SIM_FCFG1_EESIZE_SHIFT)) & SIM_FCFG1_EESIZE_MASK)
#define SIM_FCFG1_PFSIZE_MASK                    (0xF000000U)
#define SIM_FCFG1_PFSIZE_SHIFT                   (24U)
#define SIM_FCFG1_PFSIZE(x)                      (((uint32_t)(((uint32_t)(x)) << SIM_FCFG1_PFSIZE_SHIFT)) & SIM_FCFG1_PFSIZE_MASK)
#define SIM_FCFG1_NVMSIZE_MASK                   (0xF0000000U)
#define SIM_FCFG1_NVMSIZE_SHIFT                  (28U)
#define SIM_FCFG1_NVMSIZE(x)                     (((uint32_t)(((uint32_t)(x)) << SIM_FCFG1_NVMSIZE_SHIFT)) & SIM_FCFG1_NVMSIZE_MASK)

/*! @name FCFG2 - Flash Configuration Register 2 */
#define SIM_FCFG2_MAXADDR1_MASK                  (0x7F0000U)
#define SIM_FCFG2_MAXADDR1_SHIFT                 (16U)
#define SIM_FCFG2_MAXADDR1(x)                    (((uint32_t)(((uint32_t)(x)) << SIM_FCFG2_MAXADDR1_SHIFT)) & SIM_FCFG2_MAXADDR1_MASK)
#define SIM_FCFG2_PFLSH                          (0x800000U)
#define SIM_FCFG2_MAXADDR0_MASK                  (0x7F000000U)
#define SIM_FCFG2_MAXADDR0_SHIFT                 (24U)
#define SIM_FCFG2_MAXADDR0(x)                    (((uint32_t)(((uint32_t)(x)) << SIM_FCFG2_MAXADDR0_SHIFT)) & SIM_FCFG2_MAXADDR0_MASK)


/*!
 * @}
 */ /* end of group SIM_Register_Masks */


/* SIM - Peripheral instance base addresses */
/** Peripheral SIM base address */
#define SIM_BASE                                 (0x40047000u)
/** Peripheral SIM base pointer */
#define SIM                                      ((SIM_TypeDef *)SIM_BASE)
/** Array initializer of SIM peripheral base addresses */
#define SIM_BASE_ADDRS                           { SIM_BASE }
/** Array initializer of SIM peripheral base pointers */
#define SIM_BASE_PTRS                            { SIM }

/*!
 * @}
 */ /* end of group SIM_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- SMC Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup SMC_Peripheral_Access_Layer SMC Peripheral Access Layer
 * @{
 */

/** SMC - Register Layout Typedef */
typedef struct {
  __IO uint8_t PMPROT;                             /**< Power Mode Protection register, offset: 0x0 */
  __IO uint8_t PMCTRL;                             /**< Power Mode Control register, offset: 0x1 */
  __IO uint8_t VLLSCTRL;                           /**< VLLS Control register, offset: 0x2 */
  __I  uint8_t PMSTAT;                             /**< Power Mode Status register, offset: 0x3 */
} SMC_TypeDef;

/* ----------------------------------------------------------------------------
   -- SMC Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup SMC_Register_Masks SMC Register Masks
 * @{
 */

/*! @name PMPROT - Power Mode Protection register */
#define SMC_PMPROT_AVLLS                         (0x2U)
#define SMC_PMPROT_ALLS                          (0x8U)
#define SMC_PMPROT_AVLP                          (0x20U)

/*! @name PMCTRL - Power Mode Control register */
#define SMC_PMCTRL_STOPM_MASK                    (0x7U)
#define SMC_PMCTRL_STOPM_SHIFT                   (0U)
#define SMC_PMCTRL_STOPM(x)                      (((uint8_t)(((uint8_t)(x)) << SMC_PMCTRL_STOPM_SHIFT)) & SMC_PMCTRL_STOPM_MASK)
#define SMC_PMCTRL_STOPA                         (0x8U)
#define SMC_PMCTRL_RUNM_MASK                     (0x60U)
#define SMC_PMCTRL_RUNM_SHIFT                    (5U)
#define SMC_PMCTRL_RUNM(x)                       (((uint8_t)(((uint8_t)(x)) << SMC_PMCTRL_RUNM_SHIFT)) & SMC_PMCTRL_RUNM_MASK)
#define SMC_PMCTRL_LPWUI                         (0x80U)

/*! @name VLLSCTRL - VLLS Control register */
#define SMC_VLLSCTRL_VLLSM_MASK                  (0x7U)
#define SMC_VLLSCTRL_VLLSM_SHIFT                 (0U)
#define SMC_VLLSCTRL_VLLSM(x)                    (((uint8_t)(((uint8_t)(x)) << SMC_VLLSCTRL_VLLSM_SHIFT)) & SMC_VLLSCTRL_VLLSM_MASK)
#define SMC_VLLSCTRL_PORPO                       (0x20U)

/*! @name PMSTAT - Power Mode Status register */
#define SMC_PMSTAT_PMSTAT_MASK                   (0x7FU)
#define SMC_PMSTAT_PMSTAT_SHIFT                  (0U)
#define SMC_PMSTAT_PMSTAT(x)                     (((uint8_t)(((uint8_t)(x)) << SMC_PMSTAT_PMSTAT_SHIFT)) & SMC_PMSTAT_PMSTAT_MASK)


/*!
 * @}
 */ /* end of group SMC_Register_Masks */


/* SMC - Peripheral instance base addresses */
/** Peripheral SMC base address */
#define SMC_BASE                                 (0x4007E000u)
/** Peripheral SMC base pointer */
#define SMC                                      ((SMC_TypeDef *)SMC_BASE)
/** Array initializer of SMC peripheral base addresses */
#define SMC_BASE_ADDRS                           { SMC_BASE }
/** Array initializer of SMC peripheral base pointers */
#define SMC_BASE_PTRS                            { SMC }

/*!
 * @}
 */ /* end of group SMC_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- SPI Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup SPI_Peripheral_Access_Layer SPI Peripheral Access Layer
 * @{
 */

/** SPI - Register Layout Typedef */
typedef struct {
  __IO uint32_t MCR;                               /**< Module Configuration Register, offset: 0x0 */
       uint8_t RESERVED_0[4];
  __IO uint32_t TCR;                               /**< Transfer Count Register, offset: 0x8 */
  union {                                          /* offset: 0xC */
    __IO uint32_t CTAR[2];                           /**< Clock and Transfer Attributes Register (In Master Mode), array offset: 0xC, array step: 0x4 */
    __IO uint32_t CTAR_SLAVE[1];                     /**< Clock and Transfer Attributes Register (In Slave Mode), array offset: 0xC, array step: 0x4 */
  };
       uint8_t RESERVED_1[24];
  __IO uint32_t SR;                                /**< Status Register, offset: 0x2C */
  __IO uint32_t RSER;                              /**< DMA/Interrupt Request Select and Enable Register, offset: 0x30 */
  union {                                          /* offset: 0x34 */
    __IO uint32_t PUSHR;                             /**< PUSH TX FIFO Register In Master Mode, offset: 0x34 */
    __IO uint32_t PUSHR_SLAVE;                       /**< PUSH TX FIFO Register In Slave Mode, offset: 0x34 */
  };
  __I  uint32_t POPR;                              /**< POP RX FIFO Register, offset: 0x38 */
  __I  uint32_t TXFR0;                             /**< Transmit FIFO Registers, offset: 0x3C */
  __I  uint32_t TXFR1;                             /**< Transmit FIFO Registers, offset: 0x40 */
  __I  uint32_t TXFR2;                             /**< Transmit FIFO Registers, offset: 0x44 */
  __I  uint32_t TXFR3;                             /**< Transmit FIFO Registers, offset: 0x48 */
       uint8_t RESERVED_2[48];
  __I  uint32_t RXFR0;                             /**< Receive FIFO Registers, offset: 0x7C */
  __I  uint32_t RXFR1;                             /**< Receive FIFO Registers, offset: 0x80 */
  __I  uint32_t RXFR2;                             /**< Receive FIFO Registers, offset: 0x84 */
  __I  uint32_t RXFR3;                             /**< Receive FIFO Registers, offset: 0x88 */
} SPI_TypeDef;

/* ----------------------------------------------------------------------------
   -- SPI Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup SPI_Register_Masks SPI Register Masks
 * @{
 */

/*! @name MCR - Module Configuration Register */
#define SPI_MCR_HALT                             (0x1U)
#define SPI_MCR_SMPL_PT_MASK                     (0x300U)
#define SPI_MCR_SMPL_PT_SHIFT                    (8U)
#define SPI_MCR_SMPL_PT(x)                       (((uint32_t)(((uint32_t)(x)) << SPI_MCR_SMPL_PT_SHIFT)) & SPI_MCR_SMPL_PT_MASK)
#define SPI_MCR_CLR_RXF                          (0x400U)
#define SPI_MCR_CLR_TXF                          (0x800U)
#define SPI_MCR_DIS_RXF                          (0x1000U)
#define SPI_MCR_DIS_TXF                          (0x2000U)
#define SPI_MCR_MDIS                             (0x4000U)
#define SPI_MCR_DOZE                             (0x8000U)
#define SPI_MCR_PCSIS_MASK                       (0x3F0000U)
#define SPI_MCR_PCSIS_SHIFT                      (16U)
#define SPI_MCR_PCSIS(x)                         (((uint32_t)(((uint32_t)(x)) << SPI_MCR_PCSIS_SHIFT)) & SPI_MCR_PCSIS_MASK)
#define SPI_MCR_ROOE                             (0x1000000U)
#define SPI_MCR_PCSSE                            (0x2000000U)
#define SPI_MCR_MTFE                             (0x4000000U)
#define SPI_MCR_FRZ                              (0x8000000U)
#define SPI_MCR_DCONF_MASK                       (0x30000000U)
#define SPI_MCR_DCONF_SHIFT                      (28U)
#define SPI_MCR_DCONF(x)                         (((uint32_t)(((uint32_t)(x)) << SPI_MCR_DCONF_SHIFT)) & SPI_MCR_DCONF_MASK)
#define SPI_MCR_CONT_SCKE                        (0x40000000U)
#define SPI_MCR_MSTR                             (0x80000000U)

/*! @name TCR - Transfer Count Register */
#define SPI_TCR_SPI_TCNT_MASK                    (0xFFFF0000U)
#define SPI_TCR_SPI_TCNT_SHIFT                   (16U)
#define SPI_TCR_SPI_TCNT(x)                      (((uint32_t)(((uint32_t)(x)) << SPI_TCR_SPI_TCNT_SHIFT)) & SPI_TCR_SPI_TCNT_MASK)

/*! @name CTAR - Clock and Transfer Attributes Register (In Master Mode) */
#define SPI_CTAR_BR_MASK                         (0xFU)
#define SPI_CTAR_BR_SHIFT                        (0U)
#define SPI_CTAR_BR(x)                           (((uint32_t)(((uint32_t)(x)) << SPI_CTAR_BR_SHIFT)) & SPI_CTAR_BR_MASK)
#define SPI_CTAR_DT_MASK                         (0xF0U)
#define SPI_CTAR_DT_SHIFT                        (4U)
#define SPI_CTAR_DT(x)                           (((uint32_t)(((uint32_t)(x)) << SPI_CTAR_DT_SHIFT)) & SPI_CTAR_DT_MASK)
#define SPI_CTAR_ASC_MASK                        (0xF00U)
#define SPI_CTAR_ASC_SHIFT                       (8U)
#define SPI_CTAR_ASC(x)                          (((uint32_t)(((uint32_t)(x)) << SPI_CTAR_ASC_SHIFT)) & SPI_CTAR_ASC_MASK)
#define SPI_CTAR_CSSCK_MASK                      (0xF000U)
#define SPI_CTAR_CSSCK_SHIFT                     (12U)
#define SPI_CTAR_CSSCK(x)                        (((uint32_t)(((uint32_t)(x)) << SPI_CTAR_CSSCK_SHIFT)) & SPI_CTAR_CSSCK_MASK)
#define SPI_CTAR_PBR_MASK                        (0x30000U)
#define SPI_CTAR_PBR_SHIFT                       (16U)
#define SPI_CTAR_PBR(x)                          (((uint32_t)(((uint32_t)(x)) << SPI_CTAR_PBR_SHIFT)) & SPI_CTAR_PBR_MASK)
#define SPI_CTAR_PDT_MASK                        (0xC0000U)
#define SPI_CTAR_PDT_SHIFT                       (18U)
#define SPI_CTAR_PDT(x)                          (((uint32_t)(((uint32_t)(x)) << SPI_CTAR_PDT_SHIFT)) & SPI_CTAR_PDT_MASK)
#define SPI_CTAR_PASC_MASK                       (0x300000U)
#define SPI_CTAR_PASC_SHIFT                      (20U)
#define SPI_CTAR_PASC(x)                         (((uint32_t)(((uint32_t)(x)) << SPI_CTAR_PASC_SHIFT)) & SPI_CTAR_PASC_MASK)
#define SPI_CTAR_PCSSCK_MASK                     (0xC00000U)
#define SPI_CTAR_PCSSCK_SHIFT                    (22U)
#define SPI_CTAR_PCSSCK(x)                       (((uint32_t)(((uint32_t)(x)) << SPI_CTAR_PCSSCK_SHIFT)) & SPI_CTAR_PCSSCK_MASK)
#define SPI_CTAR_LSBFE                           (0x1000000U)
#define SPI_CTAR_CPHA                            (0x2000000U)
#define SPI_CTAR_CPOL                            (0x4000000U)
#define SPI_CTAR_FMSZ_MASK                       (0x78000000U)
#define SPI_CTAR_FMSZ_SHIFT                      (27U)
#define SPI_CTAR_FMSZ(x)                         (((uint32_t)(((uint32_t)(x)) << SPI_CTAR_FMSZ_SHIFT)) & SPI_CTAR_FMSZ_MASK)
#define SPI_CTAR_DBR                             (0x80000000U)

/* The count of SPI_CTAR */
#define SPI_CTAR_COUNT                           (2U)

/*! @name CTAR_SLAVE - Clock and Transfer Attributes Register (In Slave Mode) */
#define SPI_CTAR_SLAVE_CPHA                      (0x2000000U)
#define SPI_CTAR_SLAVE_CPOL                      (0x4000000U)
#define SPI_CTAR_SLAVE_FMSZ_MASK                 (0xF8000000U)
#define SPI_CTAR_SLAVE_FMSZ_SHIFT                (27U)
#define SPI_CTAR_SLAVE_FMSZ(x)                   (((uint32_t)(((uint32_t)(x)) << SPI_CTAR_SLAVE_FMSZ_SHIFT)) & SPI_CTAR_SLAVE_FMSZ_MASK)

/* The count of SPI_CTAR_SLAVE */
#define SPI_CTAR_SLAVE_COUNT                     (1U)

/*! @name SR - Status Register */
#define SPI_SR_POPNXTPTR_MASK                    (0xFU)
#define SPI_SR_POPNXTPTR_SHIFT                   (0U)
#define SPI_SR_POPNXTPTR(x)                      (((uint32_t)(((uint32_t)(x)) << SPI_SR_POPNXTPTR_SHIFT)) & SPI_SR_POPNXTPTR_MASK)
#define SPI_SR_RXCTR_MASK                        (0xF0U)
#define SPI_SR_RXCTR_SHIFT                       (4U)
#define SPI_SR_RXCTR(x)                          (((uint32_t)(((uint32_t)(x)) << SPI_SR_RXCTR_SHIFT)) & SPI_SR_RXCTR_MASK)
#define SPI_SR_TXNXTPTR_MASK                     (0xF00U)
#define SPI_SR_TXNXTPTR_SHIFT                    (8U)
#define SPI_SR_TXNXTPTR(x)                       (((uint32_t)(((uint32_t)(x)) << SPI_SR_TXNXTPTR_SHIFT)) & SPI_SR_TXNXTPTR_MASK)
#define SPI_SR_TXCTR_MASK                        (0xF000U)
#define SPI_SR_TXCTR_SHIFT                       (12U)
#define SPI_SR_TXCTR(x)                          (((uint32_t)(((uint32_t)(x)) << SPI_SR_TXCTR_SHIFT)) & SPI_SR_TXCTR_MASK)
#define SPI_SR_RFDF                              (0x20000U)
#define SPI_SR_RFOF                              (0x80000U)
#define SPI_SR_TFFF                              (0x2000000U)
#define SPI_SR_TFUF                              (0x8000000U)
#define SPI_SR_EOQF                              (0x10000000U)
#define SPI_SR_TXRXS                             (0x40000000U)
#define SPI_SR_TCF                               (0x80000000U)

/*! @name RSER - DMA/Interrupt Request Select and Enable Register */
#define SPI_RSER_RFDF_DIRS                       (0x10000U)
#define SPI_RSER_RFDF_RE                         (0x20000U)
#define SPI_RSER_RFOF_RE                         (0x80000U)
#define SPI_RSER_TFFF_DIRS                       (0x1000000U)
#define SPI_RSER_TFFF_RE                         (0x2000000U)
#define SPI_RSER_TFUF_RE                         (0x8000000U)
#define SPI_RSER_EOQF_RE                         (0x10000000U)
#define SPI_RSER_TCF_RE                          (0x80000000U)

/*! @name PUSHR - PUSH TX FIFO Register In Master Mode */
#define SPI_PUSHR_TXDATA_MASK                    (0xFFFFU)
#define SPI_PUSHR_TXDATA_SHIFT                   (0U)
#define SPI_PUSHR_TXDATA(x)                      (((uint32_t)(((uint32_t)(x)) << SPI_PUSHR_TXDATA_SHIFT)) & SPI_PUSHR_TXDATA_MASK)
#define SPI_PUSHR_PCS_MASK                       (0x3F0000U)
#define SPI_PUSHR_PCS_SHIFT                      (16U)
#define SPI_PUSHR_PCS(x)                         (((uint32_t)(((uint32_t)(x)) << SPI_PUSHR_PCS_SHIFT)) & SPI_PUSHR_PCS_MASK)
#define SPI_PUSHR_CTCNT                          (0x4000000U)
#define SPI_PUSHR_EOQ                            (0x8000000U)
#define SPI_PUSHR_CTAS_MASK                      (0x70000000U)
#define SPI_PUSHR_CTAS_SHIFT                     (28U)
#define SPI_PUSHR_CTAS(x)                        (((uint32_t)(((uint32_t)(x)) << SPI_PUSHR_CTAS_SHIFT)) & SPI_PUSHR_CTAS_MASK)
#define SPI_PUSHR_CONT                           (0x80000000U)

/*! @name TXFR0 - Transmit FIFO Registers */
#define SPI_TXFR0_TXDATA_MASK                    (0xFFFFU)
#define SPI_TXFR0_TXDATA_SHIFT                   (0U)
#define SPI_TXFR0_TXDATA(x)                      (((uint32_t)(((uint32_t)(x)) << SPI_TXFR0_TXDATA_SHIFT)) & SPI_TXFR0_TXDATA_MASK)
#define SPI_TXFR0_TXCMD_TXDATA_MASK              (0xFFFF0000U)
#define SPI_TXFR0_TXCMD_TXDATA_SHIFT             (16U)
#define SPI_TXFR0_TXCMD_TXDATA(x)                (((uint32_t)(((uint32_t)(x)) << SPI_TXFR0_TXCMD_TXDATA_SHIFT)) & SPI_TXFR0_TXCMD_TXDATA_MASK)

/*! @name TXFR1 - Transmit FIFO Registers */
#define SPI_TXFR1_TXDATA_MASK                    (0xFFFFU)
#define SPI_TXFR1_TXDATA_SHIFT                   (0U)
#define SPI_TXFR1_TXDATA(x)                      (((uint32_t)(((uint32_t)(x)) << SPI_TXFR1_TXDATA_SHIFT)) & SPI_TXFR1_TXDATA_MASK)
#define SPI_TXFR1_TXCMD_TXDATA_MASK              (0xFFFF0000U)
#define SPI_TXFR1_TXCMD_TXDATA_SHIFT             (16U)
#define SPI_TXFR1_TXCMD_TXDATA(x)                (((uint32_t)(((uint32_t)(x)) << SPI_TXFR1_TXCMD_TXDATA_SHIFT)) & SPI_TXFR1_TXCMD_TXDATA_MASK)

/*! @name TXFR2 - Transmit FIFO Registers */
#define SPI_TXFR2_TXDATA_MASK                    (0xFFFFU)
#define SPI_TXFR2_TXDATA_SHIFT                   (0U)
#define SPI_TXFR2_TXDATA(x)                      (((uint32_t)(((uint32_t)(x)) << SPI_TXFR2_TXDATA_SHIFT)) & SPI_TXFR2_TXDATA_MASK)
#define SPI_TXFR2_TXCMD_TXDATA_MASK              (0xFFFF0000U)
#define SPI_TXFR2_TXCMD_TXDATA_SHIFT             (16U)
#define SPI_TXFR2_TXCMD_TXDATA(x)                (((uint32_t)(((uint32_t)(x)) << SPI_TXFR2_TXCMD_TXDATA_SHIFT)) & SPI_TXFR2_TXCMD_TXDATA_MASK)

/*! @name TXFR3 - Transmit FIFO Registers */
#define SPI_TXFR3_TXDATA_MASK                    (0xFFFFU)
#define SPI_TXFR3_TXDATA_SHIFT                   (0U)
#define SPI_TXFR3_TXDATA(x)                      (((uint32_t)(((uint32_t)(x)) << SPI_TXFR3_TXDATA_SHIFT)) & SPI_TXFR3_TXDATA_MASK)
#define SPI_TXFR3_TXCMD_TXDATA_MASK              (0xFFFF0000U)
#define SPI_TXFR3_TXCMD_TXDATA_SHIFT             (16U)
#define SPI_TXFR3_TXCMD_TXDATA(x)                (((uint32_t)(((uint32_t)(x)) << SPI_TXFR3_TXCMD_TXDATA_SHIFT)) & SPI_TXFR3_TXCMD_TXDATA_MASK)

/*!
 * @}
 */ /* end of group SPI_Register_Masks */


/* SPI - Peripheral instance base addresses */
/** Peripheral SPI0 base address */
#define SPI0_BASE                                (0x4002C000u)
/** Peripheral SPI0 base pointer */
#define SPI0                                     ((SPI_TypeDef *)SPI0_BASE)
/** Peripheral SPI1 base address */
#define SPI1_BASE                                (0x4002D000u)
/** Peripheral SPI1 base pointer */
#define SPI1                                     ((SPI_TypeDef *)SPI1_BASE)
/** Peripheral SPI2 base address */
#define SPI2_BASE                                (0x400AC000u)
/** Peripheral SPI2 base pointer */
#define SPI2                                     ((SPI_TypeDef *)SPI2_BASE)
/** Array initializer of SPI peripheral base addresses */
#define SPI_BASE_ADDRS                           { SPI0_BASE, SPI1_BASE, SPI2_BASE }
/** Array initializer of SPI peripheral base pointers */
#define SPI_BASE_PTRS                            { SPI0, SPI1, SPI2 }
/** Interrupt vectors for the SPI peripheral type */
#define SPI_IRQS                                 { SPI0_IRQn, SPI1_IRQn, SPI2_IRQn }

/*!
 * @}
 */ /* end of group SPI_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- UART Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup UART_Peripheral_Access_Layer UART Peripheral Access Layer
 * @{
 */

/** UART - Register Layout Typedef */
typedef struct {
  __IO uint8_t BDH;                                /**< UART Baud Rate Registers: High, offset: 0x0 */
  __IO uint8_t BDL;                                /**< UART Baud Rate Registers: Low, offset: 0x1 */
  __IO uint8_t C1;                                 /**< UART Control Register 1, offset: 0x2 */
  __IO uint8_t C2;                                 /**< UART Control Register 2, offset: 0x3 */
  __I  uint8_t S1;                                 /**< UART Status Register 1, offset: 0x4 */
  __IO uint8_t S2;                                 /**< UART Status Register 2, offset: 0x5 */
  __IO uint8_t C3;                                 /**< UART Control Register 3, offset: 0x6 */
  __IO uint8_t D;                                  /**< UART Data Register, offset: 0x7 */
  __IO uint8_t MA1;                                /**< UART Match Address Registers 1, offset: 0x8 */
  __IO uint8_t MA2;                                /**< UART Match Address Registers 2, offset: 0x9 */
  __IO uint8_t C4;                                 /**< UART Control Register 4, offset: 0xA */
  __IO uint8_t C5;                                 /**< UART Control Register 5, offset: 0xB */
  __I  uint8_t ED;                                 /**< UART Extended Data Register, offset: 0xC */
  __IO uint8_t MODEM;                              /**< UART Modem Register, offset: 0xD */
  __IO uint8_t IR;                                 /**< UART Infrared Register, offset: 0xE */
       uint8_t RESERVED_0[1];
  __IO uint8_t PFIFO;                              /**< UART FIFO Parameters, offset: 0x10 */
  __IO uint8_t CFIFO;                              /**< UART FIFO Control Register, offset: 0x11 */
  __IO uint8_t SFIFO;                              /**< UART FIFO Status Register, offset: 0x12 */
  __IO uint8_t TWFIFO;                             /**< UART FIFO Transmit Watermark, offset: 0x13 */
  __I  uint8_t TCFIFO;                             /**< UART FIFO Transmit Count, offset: 0x14 */
  __IO uint8_t RWFIFO;                             /**< UART FIFO Receive Watermark, offset: 0x15 */
  __I  uint8_t RCFIFO;                             /**< UART FIFO Receive Count, offset: 0x16 */
       uint8_t RESERVED_1[1];
  __IO uint8_t C7816;                              /**< UART 7816 Control Register, offset: 0x18 */
  __IO uint8_t IE7816;                             /**< UART 7816 Interrupt Enable Register, offset: 0x19 */
  __IO uint8_t IS7816;                             /**< UART 7816 Interrupt Status Register, offset: 0x1A */
  union {                                          /* offset: 0x1B */
    __IO uint8_t WP7816T0;                           /**< UART 7816 Wait Parameter Register, offset: 0x1B */
    __IO uint8_t WP7816T1;                           /**< UART 7816 Wait Parameter Register, offset: 0x1B */
  };
  __IO uint8_t WN7816;                             /**< UART 7816 Wait N Register, offset: 0x1C */
  __IO uint8_t WF7816;                             /**< UART 7816 Wait FD Register, offset: 0x1D */
  __IO uint8_t ET7816;                             /**< UART 7816 Error Threshold Register, offset: 0x1E */
  __IO uint8_t TL7816;                             /**< UART 7816 Transmit Length Register, offset: 0x1F */
} UART_TypeDef;

/* ----------------------------------------------------------------------------
   -- UART Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup UART_Register_Masks UART Register Masks
 * @{
 */

/*! @name BDH - UART Baud Rate Registers: High */
#define UARTx_BDH_SBR_MASK                       (0x1FU)
#define UARTx_BDH_SBR_SHIFT                      (0U)
#define UARTx_BDH_SBR(x)                         (((uint8_t)(((uint8_t)(x)) << UARTx_BDH_SBR_SHIFT)) & UARTx_BDH_SBR_MASK)
#define UARTx_BDH_SBNS                           (0x20U)
#define UARTx_BDH_RXEDGIE                        (0x40U)
#define UARTx_BDH_LBKDIE                         (0x80U)

/*! @name BDL - UART Baud Rate Registers: Low */
#define UARTx_BDL_SBR_MASK                       (0xFFU)
#define UARTx_BDL_SBR_SHIFT                      (0U)
#define UARTx_BDL_SBR(x)                         (((uint8_t)(((uint8_t)(x)) << UARTx_BDL_SBR_SHIFT)) & UARTx_BDL_SBR_MASK)

/*! @name C1 - UART Control Register 1 */
#define UARTx_C1_PT                              (0x1U)
#define UARTx_C1_PE                              (0x2U)
#define UARTx_C1_ILT                             (0x4U)
#define UARTx_C1_WAKE                            (0x8U)
#define UARTx_C1_M                               (0x10U)
#define UARTx_C1_RSRC                            (0x20U)
#define UARTx_C1_UARTSWAI                        (0x40U)
#define UARTx_C1_LOOPS                           (0x80U)

/*! @name C2 - UART Control Register 2 */
#define UARTx_C2_SBK                             (0x1U)
#define UARTx_C2_RWU                             (0x2U)
#define UARTx_C2_RE                              (0x4U)
#define UARTx_C2_TE                              (0x8U)
#define UARTx_C2_ILIE                            (0x10U)
#define UARTx_C2_RIE                             (0x20U)
#define UARTx_C2_TCIE                            (0x40U)
#define UARTx_C2_TIE                             (0x80U)

/*! @name S1 - UART Status Register 1 */
#define UARTx_S1_PF                              (0x1U)
#define UARTx_S1_FE                              (0x2U)
#define UARTx_S1_NF                              (0x4U)
#define UARTx_S1_OR                              (0x8U)
#define UARTx_S1_IDLE                            (0x10U)
#define UARTx_S1_RDRF                            (0x20U)
#define UARTx_S1_TC                              (0x40U)
#define UARTx_S1_TDRE                            (0x80U)

/*! @name S2 - UART Status Register 2 */
#define UARTx_S2_RAF                             (0x1U)
#define UARTx_S2_LBKDE                           (0x2U)
#define UARTx_S2_BRK13                           (0x4U)
#define UARTx_S2_RWUID                           (0x8U)
#define UARTx_S2_RXINV                           (0x10U)
#define UARTx_S2_MSBF                            (0x20U)
#define UARTx_S2_RXEDGIF                         (0x40U)
#define UARTx_S2_LBKDIF                          (0x80U)

/*! @name C3 - UART Control Register 3 */
#define UARTx_C3_PEIE                            (0x1U)
#define UARTx_C3_FEIE                            (0x2U)
#define UARTx_C3_NEIE                            (0x4U)
#define UARTx_C3_ORIE                            (0x8U)
#define UARTx_C3_TXINV                           (0x10U)
#define UARTx_C3_TXDIR                           (0x20U)
#define UARTx_C3_T8                              (0x40U)
#define UARTx_C3_R8                              (0x80U)

/*! @name C4 - UART Control Register 4 */
#define UARTx_C4_BRFA_MASK                       (0x1FU)
#define UARTx_C4_BRFA_SHIFT                      (0U)
#define UARTx_C4_BRFA(x)                         (((uint8_t)(((uint8_t)(x)) << UARTx_C4_BRFA_SHIFT)) & UARTx_C4_BRFA_MASK)
#define UARTx_C4_M10                             (0x20U)
#define UARTx_C4_MAEN2                           (0x40U)
#define UARTx_C4_MAEN1                           (0x80U)

/*! @name C5 - UART Control Register 5 */
#define UARTx_C5_LBKDDMAS                        (0x8U)
#define UARTx_C5_ILDMAS                          (0x10U)
#define UARTx_C5_RDMAS                           (0x20U)
#define UARTx_C5_TCDMAS                          (0x40U)
#define UARTx_C5_TDMAS                           (0x80U)

/*! @name ED - UART Extended Data Register */
#define UARTx_ED_PARITYE                         (0x40U)
#define UARTx_ED_NOISY                           (0x80U)

/*! @name MODEM - UART Modem Register */
#define UARTx_MODEM_TXCTSE                       (0x1U)
#define UARTx_MODEM_TXRTSE                       (0x2U)
#define UARTx_MODEM_TXRTSPOL                     (0x4U)
#define UARTx_MODEM_RXRTSE                       (0x8U)

/*! @name IR - UART Infrared Register */
#define UART_IR_TNP_MASK                         (0x3U)
#define UART_IR_TNP_SHIFT                        (0U)
#define UART_IR_TNP(x)                           (((uint8_t)(((uint8_t)(x)) << UART_IR_TNP_SHIFT)) & UART_IR_TNP_MASK)
#define UART_IR_IREN                             (0x4U)

/*! @name PFIFO - UART FIFO Parameters */
#define UART_PFIFO_RXFIFOSIZE_MASK               (0x7U)
#define UART_PFIFO_RXFIFOSIZE_SHIFT              (0U)
#define UART_PFIFO_RXFIFOSIZE(x)                 (((uint8_t)(((uint8_t)(x)) << UART_PFIFO_RXFIFOSIZE_SHIFT)) & UART_PFIFO_RXFIFOSIZE_MASK)
#define UART_PFIFO_RXFE                          (0x8U)
#define UART_PFIFO_TXFIFOSIZE_MASK               (0x70U)
#define UART_PFIFO_TXFIFOSIZE_SHIFT              (4U)
#define UART_PFIFO_TXFIFOSIZE(x)                 (((uint8_t)(((uint8_t)(x)) << UART_PFIFO_TXFIFOSIZE_SHIFT)) & UART_PFIFO_TXFIFOSIZE_MASK)
#define UART_PFIFO_TXFE                          (0x80U)

/*! @name CFIFO - UART FIFO Control Register */
#define UART_CFIFO_RXUFE                         (0x1U)
#define UART_CFIFO_TXOFE                         (0x2U)
#define UART_CFIFO_RXOFE                         (0x4U)
#define UART_CFIFO_RXFLUSH                       (0x40U)
#define UART_CFIFO_TXFLUSH                       (0x80U)

/*! @name SFIFO - UART FIFO Status Register */
#define UART_SFIFO_RXUF                          (0x1U)
#define UART_SFIFO_TXOF                          (0x2U)
#define UART_SFIFO_RXOF                          (0x4U)
#define UART_SFIFO_RXEMPT                        (0x40U)
#define UART_SFIFO_TXEMPT                        (0x80U)

/*! @name TWFIFO - UART FIFO Transmit Watermark */
#define UART_TWFIFO_TXWATER_MASK                 (0xFFU)
#define UART_TWFIFO_TXWATER_SHIFT                (0U)
#define UART_TWFIFO_TXWATER(x)                   (((uint8_t)(((uint8_t)(x)) << UART_TWFIFO_TXWATER_SHIFT)) & UART_TWFIFO_TXWATER_MASK)

/*! @name TCFIFO - UART FIFO Transmit Count */
#define UART_TCFIFO_TXCOUNT_MASK                 (0xFFU)
#define UART_TCFIFO_TXCOUNT_SHIFT                (0U)
#define UART_TCFIFO_TXCOUNT(x)                   (((uint8_t)(((uint8_t)(x)) << UART_TCFIFO_TXCOUNT_SHIFT)) & UART_TCFIFO_TXCOUNT_MASK)

/*! @name RWFIFO - UART FIFO Receive Watermark */
#define UART_RWFIFO_RXWATER_MASK                 (0xFFU)
#define UART_RWFIFO_RXWATER_SHIFT                (0U)
#define UART_RWFIFO_RXWATER(x)                   (((uint8_t)(((uint8_t)(x)) << UART_RWFIFO_RXWATER_SHIFT)) & UART_RWFIFO_RXWATER_MASK)

/*! @name RCFIFO - UART FIFO Receive Count */
#define UART_RCFIFO_RXCOUNT_MASK                 (0xFFU)
#define UART_RCFIFO_RXCOUNT_SHIFT                (0U)
#define UART_RCFIFO_RXCOUNT(x)                   (((uint8_t)(((uint8_t)(x)) << UART_RCFIFO_RXCOUNT_SHIFT)) & UART_RCFIFO_RXCOUNT_MASK)

/*! @name C7816 - UART 7816 Control Register */
#define UART_C7816_ISO_7816E                     (0x1U)
#define UART_C7816_TTYPE                         (0x2U)
#define UART_C7816_INIT                          (0x4U)
#define UART_C7816_ANACK                         (0x8U)
#define UART_C7816_ONACK                         (0x10U)

/*! @name IE7816 - UART 7816 Interrupt Enable Register */
#define UART_IE7816_RXTE                         (0x1U)
#define UART_IE7816_TXTE                         (0x2U)
#define UART_IE7816_GTVE                         (0x4U)
#define UART_IE7816_INITDE                       (0x10U)
#define UART_IE7816_BWTE                         (0x20U)
#define UART_IE7816_CWTE                         (0x40U)
#define UART_IE7816_WTE                          (0x80U)

/*! @name IS7816 - UART 7816 Interrupt Status Register */
#define UART_IS7816_RXT                          (0x1U)
#define UART_IS7816_TXT                          (0x2U)
#define UART_IS7816_GTV                          (0x4U)
#define UART_IS7816_INITD                        (0x10U)
#define UART_IS7816_BWT                          (0x20U)
#define UART_IS7816_CWT                          (0x40U)
#define UART_IS7816_WT                           (0x80U)

/*! @name WP7816T0 - UART 7816 Wait Parameter Register */
#define UART_WP7816T0_WI_MASK                    (0xFFU)
#define UART_WP7816T0_WI_SHIFT                   (0U)
#define UART_WP7816T0_WI(x)                      (((uint8_t)(((uint8_t)(x)) << UART_WP7816T0_WI_SHIFT)) & UART_WP7816T0_WI_MASK)

/*! @name WP7816T1 - UART 7816 Wait Parameter Register */
#define UART_WP7816T1_BWI_MASK                   (0xFU)
#define UART_WP7816T1_BWI_SHIFT                  (0U)
#define UART_WP7816T1_BWI(x)                     (((uint8_t)(((uint8_t)(x)) << UART_WP7816T1_BWI_SHIFT)) & UART_WP7816T1_BWI_MASK)
#define UART_WP7816T1_CWI_MASK                   (0xF0U)
#define UART_WP7816T1_CWI_SHIFT                  (4U)
#define UART_WP7816T1_CWI(x)                     (((uint8_t)(((uint8_t)(x)) << UART_WP7816T1_CWI_SHIFT)) & UART_WP7816T1_CWI_MASK)

/*! @name WN7816 - UART 7816 Wait N Register */
#define UART_WN7816_GTN_MASK                     (0xFFU)
#define UART_WN7816_GTN_SHIFT                    (0U)
#define UART_WN7816_GTN(x)                       (((uint8_t)(((uint8_t)(x)) << UART_WN7816_GTN_SHIFT)) & UART_WN7816_GTN_MASK)

/*! @name WF7816 - UART 7816 Wait FD Register */
#define UART_WF7816_GTFD_MASK                    (0xFFU)
#define UART_WF7816_GTFD_SHIFT                   (0U)
#define UART_WF7816_GTFD(x)                      (((uint8_t)(((uint8_t)(x)) << UART_WF7816_GTFD_SHIFT)) & UART_WF7816_GTFD_MASK)

/*! @name ET7816 - UART 7816 Error Threshold Register */
#define UART_ET7816_RXTHRESHOLD_MASK             (0xFU)
#define UART_ET7816_RXTHRESHOLD_SHIFT            (0U)
#define UART_ET7816_RXTHRESHOLD(x)               (((uint8_t)(((uint8_t)(x)) << UART_ET7816_RXTHRESHOLD_SHIFT)) & UART_ET7816_RXTHRESHOLD_MASK)
#define UART_ET7816_TXTHRESHOLD_MASK             (0xF0U)
#define UART_ET7816_TXTHRESHOLD_SHIFT            (4U)
#define UART_ET7816_TXTHRESHOLD(x)               (((uint8_t)(((uint8_t)(x)) << UART_ET7816_TXTHRESHOLD_SHIFT)) & UART_ET7816_TXTHRESHOLD_MASK)

/*! @name TL7816 - UART 7816 Transmit Length Register */
#define UART_TL7816_TLEN_MASK                    (0xFFU)
#define UART_TL7816_TLEN_SHIFT                   (0U)
#define UART_TL7816_TLEN(x)                      (((uint8_t)(((uint8_t)(x)) << UART_TL7816_TLEN_SHIFT)) & UART_TL7816_TLEN_MASK)


/*!
 * @}
 */ /* end of group UART_Register_Masks */


/* UART - Peripheral instance base addresses */
/** Peripheral UART0 base address */
#define UART0_BASE                               (0x4006A000u)
/** Peripheral UART0 base pointer */
#define UART0                                    ((UART_TypeDef *)UART0_BASE)
/** Peripheral UART1 base address */
#define UART1_BASE                               (0x4006B000u)
/** Peripheral UART1 base pointer */
#define UART1                                    ((UART_TypeDef *)UART1_BASE)
/** Peripheral UART2 base address */
#define UART2_BASE                               (0x4006C000u)
/** Peripheral UART2 base pointer */
#define UART2                                    ((UART_TypeDef *)UART2_BASE)
/** Peripheral UART3 base address */
#define UART3_BASE                               (0x4006D000u)
/** Peripheral UART3 base pointer */
#define UART3                                    ((UART_TypeDef *)UART3_BASE)
/** Peripheral UART4 base address */
#define UART4_BASE                               (0x400EA000u)
/** Peripheral UART4 base pointer */
#define UART4                                    ((UART_TypeDef *)UART4_BASE)
/** Peripheral UART5 base address */
#define UART5_BASE                               (0x400EB000u)
/** Peripheral UART5 base pointer */
#define UART5                                    ((UART_TypeDef *)UART5_BASE)
/** Array initializer of UART peripheral base addresses */
#define UART_BASE_ADDRS                          { UART0_BASE, UART1_BASE, UART2_BASE, UART3_BASE, UART4_BASE, UART5_BASE }
/** Array initializer of UART peripheral base pointers */
#define UART_BASE_PTRS                           { UART0, UART1, UART2, UART3, UART4, UART5 }
/** Interrupt vectors for the UART peripheral type */
#define UARTStatus_IRQS                          { UART0Status_IRQn, UART1Status_IRQn, UART2Status_IRQn, UART3Status_IRQn, UART4Status_IRQn, UART5Status_IRQn }
#define UARTError_IRQS                           { UART0Error_IRQn, UART1Error_IRQn, UART2Error_IRQn, UART3Error_IRQn, UART4Error_IRQn, UART5Error_IRQn }
#define UART_LON_IRQS                            { UART0_LON_IRQn, NotAvail_IRQn, NotAvail_IRQn, NotAvail_IRQn, NotAvail_IRQn, NotAvail_IRQn }

/*!
 * @}
 */ /* end of group UART_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- USB Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup USB_Peripheral_Access_Layer USB Peripheral Access Layer
 * @{
 */

/** USB - Register Layout Typedef */
typedef struct {
  __I  uint8_t PERID;                              /**< Peripheral ID register, offset: 0x0 */
       uint8_t RESERVED_0[3];
  __I  uint8_t IDCOMP;                             /**< Peripheral ID Complement register, offset: 0x4 */
       uint8_t RESERVED_1[3];
  __I  uint8_t REV;                                /**< Peripheral Revision register, offset: 0x8 */
       uint8_t RESERVED_2[3];
  __I  uint8_t ADDINFO;                            /**< Peripheral Additional Info register, offset: 0xC */
       uint8_t RESERVED_3[3];
  __IO uint8_t OTGISTAT;                           /**< OTG Interrupt Status register, offset: 0x10 */
       uint8_t RESERVED_4[3];
  __IO uint8_t OTGICR;                             /**< OTG Interrupt Control register, offset: 0x14 */
       uint8_t RESERVED_5[3];
  __IO uint8_t OTGSTAT;                            /**< OTG Status register, offset: 0x18 */
       uint8_t RESERVED_6[3];
  __IO uint8_t OTGCTL;                             /**< OTG Control register, offset: 0x1C */
       uint8_t RESERVED_7[99];
  __IO uint8_t ISTAT;                              /**< Interrupt Status register, offset: 0x80 */
       uint8_t RESERVED_8[3];
  __IO uint8_t INTEN;                              /**< Interrupt Enable register, offset: 0x84 */
       uint8_t RESERVED_9[3];
  __IO uint8_t ERRSTAT;                            /**< Error Interrupt Status register, offset: 0x88 */
       uint8_t RESERVED_10[3];
  __IO uint8_t ERREN;                              /**< Error Interrupt Enable register, offset: 0x8C */
       uint8_t RESERVED_11[3];
  __I  uint8_t STAT;                               /**< Status register, offset: 0x90 */
       uint8_t RESERVED_12[3];
  __IO uint8_t CTL;                                /**< Control register, offset: 0x94 */
       uint8_t RESERVED_13[3];
  __IO uint8_t ADDR;                               /**< Address register, offset: 0x98 */
       uint8_t RESERVED_14[3];
  __IO uint8_t BDTPAGE1;                           /**< BDT Page register 1, offset: 0x9C */
       uint8_t RESERVED_15[3];
  __IO uint8_t FRMNUML;                            /**< Frame Number register Low, offset: 0xA0 */
       uint8_t RESERVED_16[3];
  __IO uint8_t FRMNUMH;                            /**< Frame Number register High, offset: 0xA4 */
       uint8_t RESERVED_17[3];
  __IO uint8_t TOKEN;                              /**< Token register, offset: 0xA8 */
       uint8_t RESERVED_18[3];
  __IO uint8_t SOFTHLD;                            /**< SOF Threshold register, offset: 0xAC */
       uint8_t RESERVED_19[3];
  __IO uint8_t BDTPAGE2;                           /**< BDT Page Register 2, offset: 0xB0 */
       uint8_t RESERVED_20[3];
  __IO uint8_t BDTPAGE3;                           /**< BDT Page Register 3, offset: 0xB4 */
       uint8_t RESERVED_21[11];
  struct {                                         /* offset: 0xC0, array step: 0x4 */
    __IO uint8_t ENDPT;                              /**< Endpoint Control register, array offset: 0xC0, array step: 0x4 */
         uint8_t RESERVED_0[3];
  } ENDPOINT[16];
  __IO uint8_t USBCTRL;                            /**< USB Control register, offset: 0x100 */
       uint8_t RESERVED_22[3];
  __I  uint8_t OBSERVE;                            /**< USB OTG Observe register, offset: 0x104 */
       uint8_t RESERVED_23[3];
  __IO uint8_t CONTROL;                            /**< USB OTG Control register, offset: 0x108 */
       uint8_t RESERVED_24[3];
  __IO uint8_t USBTRC0;                            /**< USB Transceiver Control register 0, offset: 0x10C */
       uint8_t RESERVED_25[7];
  __IO uint8_t USBFRMADJUST;                       /**< Frame Adjust Register, offset: 0x114 */
       uint8_t RESERVED_26[43];
  __IO uint8_t CLK_RECOVER_CTRL;                   /**< USB Clock recovery control, offset: 0x140 */
       uint8_t RESERVED_27[3];
  __IO uint8_t CLK_RECOVER_IRC_EN;                 /**< IRC48M oscillator enable register, offset: 0x144 */
       uint8_t RESERVED_28[23];
  __IO uint8_t CLK_RECOVER_INT_STATUS;             /**< Clock recovery separated interrupt status, offset: 0x15C */
} USB_TypeDef;

/* ----------------------------------------------------------------------------
   -- USB Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup USB_Register_Masks USB Register Masks
 * @{
 */

/*! @name PERID - Peripheral ID register */
#define USB_PERID_ID_MASK                        (0x3FU)
#define USB_PERID_ID_SHIFT                       (0U)
#define USB_PERID_ID(x)                          (((uint8_t)(((uint8_t)(x)) << USB_PERID_ID_SHIFT)) & USB_PERID_ID_MASK)

/*! @name IDCOMP - Peripheral ID Complement register */
#define USB_IDCOMP_NID_MASK                      (0x3FU)
#define USB_IDCOMP_NID_SHIFT                     (0U)
#define USB_IDCOMP_NID(x)                        (((uint8_t)(((uint8_t)(x)) << USB_IDCOMP_NID_SHIFT)) & USB_IDCOMP_NID_MASK)

/*! @name REV - Peripheral Revision register */
#define USB_REV_REV_MASK                         (0xFFU)
#define USB_REV_REV_SHIFT                        (0U)
#define USB_REV_REV(x)                           (((uint8_t)(((uint8_t)(x)) << USB_REV_REV_SHIFT)) & USB_REV_REV_MASK)

/*! @name ADDINFO - Peripheral Additional Info register */
#define USB_ADDINFO_IEHOST                       (0x1U)
#define USB_ADDINFO_IRQNUM_MASK                  (0xF8U)
#define USB_ADDINFO_IRQNUM_SHIFT                 (3U)
#define USB_ADDINFO_IRQNUM(x)                    (((uint8_t)(((uint8_t)(x)) << USB_ADDINFO_IRQNUM_SHIFT)) & USB_ADDINFO_IRQNUM_MASK)

/*! @name OTGISTAT - OTG Interrupt Status register */
#define USB_OTGISTAT_AVBUSCHG                    (0x1U)
#define USB_OTGISTAT_B_SESS_CHG                  (0x4U)
#define USB_OTGISTAT_SESSVLDCHG                  (0x8U)
#define USB_OTGISTAT_LINE_STATE_CHG              (0x20U)
#define USB_OTGISTAT_ONEMSEC                     (0x40U)
#define USB_OTGISTAT_IDCHG                       (0x80U)

/*! @name OTGICR - OTG Interrupt Control register */
#define USB_OTGICR_AVBUSEN                       (0x1U)
#define USB_OTGICR_BSESSEN                       (0x4U)
#define USB_OTGICR_SESSVLDEN                     (0x8U)
#define USB_OTGICR_LINESTATEEN                   (0x20U)
#define USB_OTGICR_ONEMSECEN                     (0x40U)
#define USB_OTGICR_IDEN                          (0x80U)

/*! @name OTGSTAT - OTG Status register */
#define USB_OTGSTAT_AVBUSVLD                     (0x1U)
#define USB_OTGSTAT_BSESSEND                     (0x4U)
#define USB_OTGSTAT_SESS_VLD                     (0x8U)
#define USB_OTGSTAT_LINESTATESTABLE              (0x20U)
#define USB_OTGSTAT_ONEMSECEN                    (0x40U)
#define USB_OTGSTAT_ID                           (0x80U)

/*! @name OTGCTL - OTG Control register */
#define USB_OTGCTL_OTGEN                         (0x4U)
#define USB_OTGCTL_DMLOW                         (0x10U)
#define USB_OTGCTL_DPLOW                         (0x20U)
#define USB_OTGCTL_DPHIGH                        (0x80U)

/*! @name ISTAT - Interrupt Status register */
#define USB_ISTAT_USBRST                         (0x1U)
#define USB_ISTAT_ERROR                          (0x2U)
#define USB_ISTAT_SOFTOK                         (0x4U)
#define USB_ISTAT_TOKDNE                         (0x8U)
#define USB_ISTAT_SLEEP                          (0x10U)
#define USB_ISTAT_RESUME                         (0x20U)
#define USB_ISTAT_ATTACH                         (0x40U)
#define USB_ISTAT_STALL                          (0x80U)

/*! @name INTEN - Interrupt Enable register */
#define USB_INTEN_USBRSTEN                       (0x1U)
#define USB_INTEN_ERROREN                        (0x2U)
#define USB_INTEN_SOFTOKEN                       (0x4U)
#define USB_INTEN_TOKDNEEN                       (0x8U)
#define USB_INTEN_SLEEPEN                        (0x10U)
#define USB_INTEN_RESUMEEN                       (0x20U)
#define USB_INTEN_ATTACHEN                       (0x40U)
#define USB_INTEN_STALLEN                        (0x80U)

/*! @name ERRSTAT - Error Interrupt Status register */
#define USB_ERRSTAT_PIDERR                       (0x1U)
#define USB_ERRSTAT_CRC5EOF                      (0x2U)
#define USB_ERRSTAT_CRC16                        (0x4U)
#define USB_ERRSTAT_DFN8                         (0x8U)
#define USB_ERRSTAT_BTOERR                       (0x10U)
#define USB_ERRSTAT_DMAERR                       (0x20U)
#define USB_ERRSTAT_BTSERR                       (0x80U)

/*! @name ERREN - Error Interrupt Enable register */
#define USB_ERREN_PIDERREN                       (0x1U)
#define USB_ERREN_CRC5EOFEN                      (0x2U)
#define USB_ERREN_CRC16EN                        (0x4U)
#define USB_ERREN_DFN8EN                         (0x8U)
#define USB_ERREN_BTOERREN                       (0x10U)
#define USB_ERREN_DMAERREN                       (0x20U)
#define USB_ERREN_BTSERREN                       (0x80U)

/*! @name STAT - Status register */
#define USB_STAT_ODD                             (0x4U)
#define USB_STAT_TX                              (0x8U)
#define USB_STAT_ENDP_MASK                       (0xF0U)
#define USB_STAT_ENDP_SHIFT                      (4U)
#define USB_STAT_ENDP(x)                         (((uint8_t)(((uint8_t)(x)) << USB_STAT_ENDP_SHIFT)) & USB_STAT_ENDP_MASK)

/*! @name CTL - Control register */
#define USB_CTL_USBENSOFEN                       (0x1U)
#define USB_CTL_ODDRST                           (0x2U)
#define USB_CTL_RESUME                           (0x4U)
#define USB_CTL_HOSTMODEEN                       (0x8U)
#define USB_CTL_RESET                            (0x10U)
#define USB_CTL_TXSUSPENDTOKENBUSY               (0x20U)
#define USB_CTL_SE0                              (0x40U)
#define USB_CTL_JSTATE                           (0x80U)

/*! @name ADDR - Address register */
#define USB_ADDR_ADDR_MASK                       (0x7FU)
#define USB_ADDR_ADDR_SHIFT                      (0U)
#define USB_ADDR_ADDR(x)                         (((uint8_t)(((uint8_t)(x)) << USB_ADDR_ADDR_SHIFT)) & USB_ADDR_ADDR_MASK)
#define USB_ADDR_LSEN                            (0x80U)

/*! @name BDTPAGE1 - BDT Page register 1 */
#define USB_BDTPAGE1_BDTBA_MASK                  (0xFEU)
#define USB_BDTPAGE1_BDTBA_SHIFT                 (1U)
#define USB_BDTPAGE1_BDTBA(x)                    (((uint8_t)(((uint8_t)(x)) << USB_BDTPAGE1_BDTBA_SHIFT)) & USB_BDTPAGE1_BDTBA_MASK)

/*! @name FRMNUML - Frame Number register Low */
#define USB_FRMNUML_FRM_MASK                     (0xFFU)
#define USB_FRMNUML_FRM_SHIFT                    (0U)
#define USB_FRMNUML_FRM(x)                       (((uint8_t)(((uint8_t)(x)) << USB_FRMNUML_FRM_SHIFT)) & USB_FRMNUML_FRM_MASK)

/*! @name FRMNUMH - Frame Number register High */
#define USB_FRMNUMH_FRM_MASK                     (0x7U)
#define USB_FRMNUMH_FRM_SHIFT                    (0U)
#define USB_FRMNUMH_FRM(x)                       (((uint8_t)(((uint8_t)(x)) << USB_FRMNUMH_FRM_SHIFT)) & USB_FRMNUMH_FRM_MASK)

/*! @name TOKEN - Token register */
#define USB_TOKEN_TOKENENDPT_MASK                (0xFU)
#define USB_TOKEN_TOKENENDPT_SHIFT               (0U)
#define USB_TOKEN_TOKENENDPT(x)                  (((uint8_t)(((uint8_t)(x)) << USB_TOKEN_TOKENENDPT_SHIFT)) & USB_TOKEN_TOKENENDPT_MASK)
#define USB_TOKEN_TOKENPID_MASK                  (0xF0U)
#define USB_TOKEN_TOKENPID_SHIFT                 (4U)
#define USB_TOKEN_TOKENPID(x)                    (((uint8_t)(((uint8_t)(x)) << USB_TOKEN_TOKENPID_SHIFT)) & USB_TOKEN_TOKENPID_MASK)

/*! @name SOFTHLD - SOF Threshold register */
#define USB_SOFTHLD_CNT_MASK                     (0xFFU)
#define USB_SOFTHLD_CNT_SHIFT                    (0U)
#define USB_SOFTHLD_CNT(x)                       (((uint8_t)(((uint8_t)(x)) << USB_SOFTHLD_CNT_SHIFT)) & USB_SOFTHLD_CNT_MASK)

/*! @name BDTPAGE2 - BDT Page Register 2 */
#define USB_BDTPAGE2_BDTBA_MASK                  (0xFFU)
#define USB_BDTPAGE2_BDTBA_SHIFT                 (0U)
#define USB_BDTPAGE2_BDTBA(x)                    (((uint8_t)(((uint8_t)(x)) << USB_BDTPAGE2_BDTBA_SHIFT)) & USB_BDTPAGE2_BDTBA_MASK)

/*! @name BDTPAGE3 - BDT Page Register 3 */
#define USB_BDTPAGE3_BDTBA_MASK                  (0xFFU)
#define USB_BDTPAGE3_BDTBA_SHIFT                 (0U)
#define USB_BDTPAGE3_BDTBA(x)                    (((uint8_t)(((uint8_t)(x)) << USB_BDTPAGE3_BDTBA_SHIFT)) & USB_BDTPAGE3_BDTBA_MASK)

/*! @name ENDPT - Endpoint Control register */
#define USB_ENDPT_EPHSHK                         (0x1U)
#define USB_ENDPT_EPSTALL                        (0x2U)
#define USB_ENDPT_EPTXEN                         (0x4U)
#define USB_ENDPT_EPRXEN                         (0x8U)
#define USB_ENDPT_EPCTLDIS                       (0x10U)
#define USB_ENDPT_RETRYDIS                       (0x40U)
#define USB_ENDPT_HOSTWOHUB                      (0x80U)

/* The count of USB_ENDPT */
#define USB_ENDPT_COUNT                          (16U)

/*! @name USBCTRL - USB Control register */
#define USB_USBCTRL_PDE                          (0x40U)
#define USB_USBCTRL_SUSP                         (0x80U)

/*! @name OBSERVE - USB OTG Observe register */
#define USB_OBSERVE_DMPD                         (0x10U)
#define USB_OBSERVE_DPPD                         (0x40U)
#define USB_OBSERVE_DPPU                         (0x80U)

/*! @name CONTROL - USB OTG Control register */
#define USB_CONTROL_DPPULLUPNONOTG               (0x10U)

/*! @name USBTRC0 - USB Transceiver Control register 0 */
#define USB_USBTRC0_USB_RESUME_INT               (0x1U)
#define USB_USBTRC0_SYNC_DET                     (0x2U)
#define USB_USBTRC0_USB_CLK_RECOVERY_INT         (0x4U)
#define USB_USBTRC0_USBRESMEN                    (0x20U)
#define USB_USBTRC0_USBRESET                     (0x80U)

/*! @name USBFRMADJUST - Frame Adjust Register */
#define USB_USBFRMADJUST_ADJ_MASK                (0xFFU)
#define USB_USBFRMADJUST_ADJ_SHIFT               (0U)
#define USB_USBFRMADJUST_ADJ(x)                  (((uint8_t)(((uint8_t)(x)) << USB_USBFRMADJUST_ADJ_SHIFT)) & USB_USBFRMADJUST_ADJ_MASK)

/*! @name CLK_RECOVER_CTRL - USB Clock recovery control */
#define USB_CLK_RECOVER_CTRL_RESTART_IFRTRIM_EN      (0x20U)
#define USB_CLK_RECOVER_CTRL_RESET_RESUME_ROUGH_EN      (0x40U)
#define USB_CLK_RECOVER_CTRL_CLOCK_RECOVER_EN      (0x80U)

/*! @name CLK_RECOVER_IRC_EN - IRC48M oscillator enable register */
#define USB_CLK_RECOVER_IRC_EN_REG_EN            (0x1U)
#define USB_CLK_RECOVER_IRC_EN_IRC_EN            (0x2U)

/*! @name CLK_RECOVER_INT_STATUS - Clock recovery separated interrupt status */
#define USB_CLK_RECOVER_INT_STATUS_OVF_ERROR      (0x10U)


/*!
 * @}
 */ /* end of group USB_Register_Masks */


/* USB - Peripheral instance base addresses */
/** Peripheral USB0 base address */
#define USB0_BASE                                (0x40072000u)
/** Peripheral USB0 base pointer */
#define USB0                                     ((USB_TypeDef *)USB0_BASE)
/** Array initializer of USB peripheral base addresses */
#define USB_BASE_ADDRS                           { USB0_BASE }
/** Array initializer of USB peripheral base pointers */
#define USB_BASE_PTRS                            { USB0 }
/** Interrupt vectors for the USB peripheral type */
#define USB_IRQS                                 { USB0_IRQn }

/*!
 * @}
 */ /* end of group USB_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- USBDCD Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup USBDCD_Peripheral_Access_Layer USBDCD Peripheral Access Layer
 * @{
 */

/** USBDCD - Register Layout Typedef */
typedef struct {
  __IO uint32_t CONTROL;                           /**< Control register, offset: 0x0 */
  __IO uint32_t CLOCK;                             /**< Clock register, offset: 0x4 */
  __I  uint32_t STATUS;                            /**< Status register, offset: 0x8 */
       uint8_t RESERVED_0[4];
  __IO uint32_t TIMER0;                            /**< TIMER0 register, offset: 0x10 */
  __IO uint32_t TIMER1;                            /**< TIMER1 register, offset: 0x14 */
  union {                                          /* offset: 0x18 */
    __IO uint32_t TIMER2_BC11;                       /**< TIMER2_BC11 register, offset: 0x18 */
    __IO uint32_t TIMER2_BC12;                       /**< TIMER2_BC12 register, offset: 0x18 */
  };
} USBDCD_TypeDef;

/* ----------------------------------------------------------------------------
   -- USBDCD Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup USBDCD_Register_Masks USBDCD Register Masks
 * @{
 */

/*! @name CONTROL - Control register */
#define USBDCD_CONTROL_IACK                      (0x1U)
#define USBDCD_CONTROL_IF                        (0x100U)
#define USBDCD_CONTROL_IE                        (0x10000U)
#define USBDCD_CONTROL_BC12                      (0x20000U)
#define USBDCD_CONTROL_START                     (0x1000000U)
#define USBDCD_CONTROL_SR                        (0x2000000U)

/*! @name CLOCK - Clock register */
#define USBDCD_CLOCK_CLOCK_UNIT                  (0x1U)
#define USBDCD_CLOCK_CLOCK_SPEED_MASK            (0xFFCU)
#define USBDCD_CLOCK_CLOCK_SPEED_SHIFT           (2U)
#define USBDCD_CLOCK_CLOCK_SPEED(x)              (((uint32_t)(((uint32_t)(x)) << USBDCD_CLOCK_CLOCK_SPEED_SHIFT)) & USBDCD_CLOCK_CLOCK_SPEED_MASK)

/*! @name STATUS - Status register */
#define USBDCD_STATUS_SEQ_RES_MASK               (0x30000U)
#define USBDCD_STATUS_SEQ_RES_SHIFT              (16U)
#define USBDCD_STATUS_SEQ_RES(x)                 (((uint32_t)(((uint32_t)(x)) << USBDCD_STATUS_SEQ_RES_SHIFT)) & USBDCD_STATUS_SEQ_RES_MASK)
#define USBDCD_STATUS_SEQ_STAT_MASK              (0xC0000U)
#define USBDCD_STATUS_SEQ_STAT_SHIFT             (18U)
#define USBDCD_STATUS_SEQ_STAT(x)                (((uint32_t)(((uint32_t)(x)) << USBDCD_STATUS_SEQ_STAT_SHIFT)) & USBDCD_STATUS_SEQ_STAT_MASK)
#define USBDCD_STATUS_ERR                        (0x100000U)
#define USBDCD_STATUS_TO                         (0x200000U)
#define USBDCD_STATUS_ACTIVE                     (0x400000U)

/*! @name TIMER0 - TIMER0 register */
#define USBDCD_TIMER0_TUNITCON_MASK              (0xFFFU)
#define USBDCD_TIMER0_TUNITCON_SHIFT             (0U)
#define USBDCD_TIMER0_TUNITCON(x)                (((uint32_t)(((uint32_t)(x)) << USBDCD_TIMER0_TUNITCON_SHIFT)) & USBDCD_TIMER0_TUNITCON_MASK)
#define USBDCD_TIMER0_TSEQ_INIT_MASK             (0x3FF0000U)
#define USBDCD_TIMER0_TSEQ_INIT_SHIFT            (16U)
#define USBDCD_TIMER0_TSEQ_INIT(x)               (((uint32_t)(((uint32_t)(x)) << USBDCD_TIMER0_TSEQ_INIT_SHIFT)) & USBDCD_TIMER0_TSEQ_INIT_MASK)

/*! @name TIMER1 - TIMER1 register */
#define USBDCD_TIMER1_TVDPSRC_ON_MASK            (0x3FFU)
#define USBDCD_TIMER1_TVDPSRC_ON_SHIFT           (0U)
#define USBDCD_TIMER1_TVDPSRC_ON(x)              (((uint32_t)(((uint32_t)(x)) << USBDCD_TIMER1_TVDPSRC_ON_SHIFT)) & USBDCD_TIMER1_TVDPSRC_ON_MASK)
#define USBDCD_TIMER1_TDCD_DBNC_MASK             (0x3FF0000U)
#define USBDCD_TIMER1_TDCD_DBNC_SHIFT            (16U)
#define USBDCD_TIMER1_TDCD_DBNC(x)               (((uint32_t)(((uint32_t)(x)) << USBDCD_TIMER1_TDCD_DBNC_SHIFT)) & USBDCD_TIMER1_TDCD_DBNC_MASK)

/*! @name TIMER2_BC11 - TIMER2_BC11 register */
#define USBDCD_TIMER2_BC11_CHECK_DM_MASK         (0xFU)
#define USBDCD_TIMER2_BC11_CHECK_DM_SHIFT        (0U)
#define USBDCD_TIMER2_BC11_CHECK_DM(x)           (((uint32_t)(((uint32_t)(x)) << USBDCD_TIMER2_BC11_CHECK_DM_SHIFT)) & USBDCD_TIMER2_BC11_CHECK_DM_MASK)
#define USBDCD_TIMER2_BC11_TVDPSRC_CON_MASK      (0x3FF0000U)
#define USBDCD_TIMER2_BC11_TVDPSRC_CON_SHIFT     (16U)
#define USBDCD_TIMER2_BC11_TVDPSRC_CON(x)        (((uint32_t)(((uint32_t)(x)) << USBDCD_TIMER2_BC11_TVDPSRC_CON_SHIFT)) & USBDCD_TIMER2_BC11_TVDPSRC_CON_MASK)

/*! @name TIMER2_BC12 - TIMER2_BC12 register */
#define USBDCD_TIMER2_BC12_TVDMSRC_ON_MASK       (0x3FFU)
#define USBDCD_TIMER2_BC12_TVDMSRC_ON_SHIFT      (0U)
#define USBDCD_TIMER2_BC12_TVDMSRC_ON(x)         (((uint32_t)(((uint32_t)(x)) << USBDCD_TIMER2_BC12_TVDMSRC_ON_SHIFT)) & USBDCD_TIMER2_BC12_TVDMSRC_ON_MASK)
#define USBDCD_TIMER2_BC12_TWAIT_AFTER_PRD_MASK  (0x3FF0000U)
#define USBDCD_TIMER2_BC12_TWAIT_AFTER_PRD_SHIFT (16U)
#define USBDCD_TIMER2_BC12_TWAIT_AFTER_PRD(x)    (((uint32_t)(((uint32_t)(x)) << USBDCD_TIMER2_BC12_TWAIT_AFTER_PRD_SHIFT)) & USBDCD_TIMER2_BC12_TWAIT_AFTER_PRD_MASK)


/*!
 * @}
 */ /* end of group USBDCD_Register_Masks */


/* USBDCD - Peripheral instance base addresses */
/** Peripheral USBDCD base address */
#define USBDCD_BASE                              (0x40035000u)
/** Peripheral USBDCD base pointer */
#define USBDCD                                   ((USBDCD_TypeDef *)USBDCD_BASE)
/** Array initializer of USBDCD peripheral base addresses */
#define USBDCD_BASE_ADDRS                        { USBDCD_BASE }
/** Array initializer of USBDCD peripheral base pointers */
#define USBDCD_BASE_PTRS                         { USBDCD }
/** Interrupt vectors for the USBDCD peripheral type */
#define USBDCD_IRQS                              { USBDCD_IRQn }

/*!
 * @}
 */ /* end of group USBDCD_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- VREF Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup VREF_Peripheral_Access_Layer VREF Peripheral Access Layer
 * @{
 */

/** VREF - Register Layout Typedef */
typedef struct {
  __IO uint8_t TRM;                                /**< VREF Trim Register, offset: 0x0 */
  __IO uint8_t SC;                                 /**< VREF Status and Control Register, offset: 0x1 */
} VREF_TypeDef;

/* ----------------------------------------------------------------------------
   -- VREF Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup VREF_Register_Masks VREF Register Masks
 * @{
 */

/*! @name TRM - VREF Trim Register */
#define VREF_TRM_TRIM_MASK                       (0x3FU)
#define VREF_TRM_TRIM_SHIFT                      (0U)
#define VREF_TRM_TRIM(x)                         (((uint8_t)(((uint8_t)(x)) << VREF_TRM_TRIM_SHIFT)) & VREF_TRM_TRIM_MASK)
#define VREF_TRM_CHOPEN                          (0x40U)

/*! @name SC - VREF Status and Control Register */
#define VREF_SC_MODE_LV_MASK                     (0x3U)
#define VREF_SC_MODE_LV_SHIFT                    (0U)
#define VREF_SC_MODE_LV(x)                       (((uint8_t)(((uint8_t)(x)) << VREF_SC_MODE_LV_SHIFT)) & VREF_SC_MODE_LV_MASK)
#define VREF_SC_VREFST                           (0x4U)
#define VREF_SC_ICOMPEN                          (0x20U)
#define VREF_SC_REGEN                            (0x40U)
#define VREF_SC_VREFEN                           (0x80U)


/*!
 * @}
 */ /* end of group VREF_Register_Masks */


/* VREF - Peripheral instance base addresses */
/** Peripheral VREF base address */
#define VREF_BASE                                (0x40074000u)
/** Peripheral VREF base pointer */
#define VREF                                     ((VREF_TypeDef *)VREF_BASE)
/** Array initializer of VREF peripheral base addresses */
#define VREF_BASE_ADDRS                          { VREF_BASE }
/** Array initializer of VREF peripheral base pointers */
#define VREF_BASE_PTRS                           { VREF }

/*!
 * @}
 */ /* end of group VREF_Peripheral_Access_Layer */


/* ----------------------------------------------------------------------------
   -- WDOG Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup WDOG_Peripheral_Access_Layer WDOG Peripheral Access Layer
 * @{
 */

/** WDOG - Register Layout Typedef */
typedef struct {
  __IO uint16_t STCTRLH;                           /**< Watchdog Status and Control Register High, offset: 0x0 */
  __IO uint16_t STCTRLL;                           /**< Watchdog Status and Control Register Low, offset: 0x2 */
  __IO uint16_t TOVALH;                            /**< Watchdog Time-out Value Register High, offset: 0x4 */
  __IO uint16_t TOVALL;                            /**< Watchdog Time-out Value Register Low, offset: 0x6 */
  __IO uint16_t WINH;                              /**< Watchdog Window Register High, offset: 0x8 */
  __IO uint16_t WINL;                              /**< Watchdog Window Register Low, offset: 0xA */
  __IO uint16_t REFRESH;                           /**< Watchdog Refresh register, offset: 0xC */
  __IO uint16_t UNLOCK;                            /**< Watchdog Unlock register, offset: 0xE */
  __IO uint16_t TMROUTH;                           /**< Watchdog Timer Output Register High, offset: 0x10 */
  __IO uint16_t TMROUTL;                           /**< Watchdog Timer Output Register Low, offset: 0x12 */
  __IO uint16_t RSTCNT;                            /**< Watchdog Reset Count register, offset: 0x14 */
  __IO uint16_t PRESC;                             /**< Watchdog Prescaler register, offset: 0x16 */
} WDOG_TypeDef;

/* ----------------------------------------------------------------------------
   -- WDOG Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup WDOG_Register_Masks WDOG Register Masks
 * @{
 */

/*! @name STCTRLH - Watchdog Status and Control Register High */
#define WDOG_STCTRLH_WDOGEN                      (0x1U)
#define WDOG_STCTRLH_CLKSRC                      (0x2U)
#define WDOG_STCTRLH_IRQRSTEN                    (0x4U)
#define WDOG_STCTRLH_WINEN                       (0x8U)
#define WDOG_STCTRLH_ALLOWUPDATE                 (0x10U)
#define WDOG_STCTRLH_DBGEN                       (0x20U)
#define WDOG_STCTRLH_STOPEN                      (0x40U)
#define WDOG_STCTRLH_WAITEN                      (0x80U)
#define WDOG_STCTRLH_TESTWDOG                    (0x400U)
#define WDOG_STCTRLH_TESTSEL                     (0x800U)
#define WDOG_STCTRLH_BYTESEL_MASK                (0x3000U)
#define WDOG_STCTRLH_BYTESEL_SHIFT               (12U)
#define WDOG_STCTRLH_BYTESEL(x)                  (((uint16_t)(((uint16_t)(x)) << WDOG_STCTRLH_BYTESEL_SHIFT)) & WDOG_STCTRLH_BYTESEL_MASK)
#define WDOG_STCTRLH_DISTESTWDOG                 (0x4000U)

/*! @name STCTRLL - Watchdog Status and Control Register Low */
#define WDOG_STCTRLL_INTFLG                      (0x8000U)

/*! @name PRESC - Watchdog Prescaler register */
#define WDOG_PRESC_PRESCVAL_MASK                 (0x700U)
#define WDOG_PRESC_PRESCVAL_SHIFT                (8U)
#define WDOG_PRESC_PRESCVAL(x)                   (((uint16_t)(((uint16_t)(x)) << WDOG_PRESC_PRESCVAL_SHIFT)) & WDOG_PRESC_PRESCVAL_MASK)


/*!
 * @}
 */ /* end of group WDOG_Register_Masks */


/* WDOG - Peripheral instance base addresses */
/** Peripheral WDOG base address */
#define WDOG_BASE                                (0x40052000u)
/** Peripheral WDOG base pointer */
#define WDOG                                     ((WDOG_TypeDef *)WDOG_BASE)
/** Array initializer of WDOG peripheral base addresses */
#define WDOG_BASE_ADDRS                          { WDOG_BASE }
/** Array initializer of WDOG peripheral base pointers */
#define WDOG_BASE_PTRS                           { WDOG }
/** Interrupt vectors for the WDOG peripheral type */
#define WDOG_IRQS                                { WDOG_EWM_IRQn }

/*!
 * @}
 */ /* end of group WDOG_Peripheral_Access_Layer */


/*
** End of section using anonymous unions
*/

#if defined(__ARMCC_VERSION)
  #pragma pop
#elif defined(__CWCC__)
  #pragma pop
#elif defined(__GNUC__)
  /* leave anonymous unions enabled */
#elif defined(__IAR_SYSTEMS_ICC__)
  #pragma language=default
#else
  #error Not supported compiler type
#endif

/*!
 * @}
 */ /* end of group Peripheral_access_layer */


/* ----------------------------------------------------------------------------
   -- SDK Compatibility
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup SDK_Compatibility_Symbols SDK Compatibility
 * @{
 */

#define ENET_RMON_R_DROP_REG(base)               ENET_IEEE_R_DROP_REG(base)
#define ENET_RMON_R_FRAME_OK_REG(base)           ENET_IEEE_R_FRAME_OK_REG(base)
#define MCG_C2_EREFS0_MASK                       MCG_C2_EREFS_MASK
#define MCG_C2_EREFS0_SHIFT                      MCG_C2_EREFS_SHIFT
#define MCG_C2_HGO0_MASK                         MCG_C2_HGO_MASK
#define MCG_C2_HGO0_SHIFT                        MCG_C2_HGO_SHIFT
#define MCG_C2_RANGE0_MASK                       MCG_C2_RANGE_MASK
#define MCG_C2_RANGE0_SHIFT                      MCG_C2_RANGE_SHIFT
#define MCG_C2_RANGE0(x)                         MCG_C2_RANGE(x)
#define MCM_ISR_REG(base)                        MCM_ISCR_REG(base)
#define MCM_ISR_FIOC_MASK                        MCM_ISCR_FIOC_MASK
#define MCM_ISR_FIOC_SHIFT                       MCM_ISCR_FIOC_SHIFT
#define MCM_ISR_FDZC_MASK                        MCM_ISCR_FDZC_MASK
#define MCM_ISR_FDZC_SHIFT                       MCM_ISCR_FDZC_SHIFT
#define MCM_ISR_FOFC_MASK                        MCM_ISCR_FOFC_MASK
#define MCM_ISR_FOFC_SHIFT                       MCM_ISCR_FOFC_SHIFT
#define MCM_ISR_FUFC_MASK                        MCM_ISCR_FUFC_MASK
#define MCM_ISR_FUFC_SHIFT                       MCM_ISCR_FUFC_SHIFT
#define MCM_ISR_FIXC_MASK                        MCM_ISCR_FIXC_MASK
#define MCM_ISR_FIXC_SHIFT                       MCM_ISCR_FIXC_SHIFT
#define MCM_ISR_FIDC_MASK                        MCM_ISCR_FIDC_MASK
#define MCM_ISR_FIDC_SHIFT                       MCM_ISCR_FIDC_SHIFT
#define MCM_ISR_FIOCE_MASK                       MCM_ISCR_FIOCE_MASK
#define MCM_ISR_FIOCE_SHIFT                      MCM_ISCR_FIOCE_SHIFT
#define MCM_ISR_FDZCE_MASK                       MCM_ISCR_FDZCE_MASK
#define MCM_ISR_FDZCE_SHIFT                      MCM_ISCR_FDZCE_SHIFT
#define MCM_ISR_FOFCE_MASK                       MCM_ISCR_FOFCE_MASK
#define MCM_ISR_FOFCE_SHIFT                      MCM_ISCR_FOFCE_SHIFT
#define MCM_ISR_FUFCE_MASK                       MCM_ISCR_FUFCE_MASK
#define MCM_ISR_FUFCE_SHIFT                      MCM_ISCR_FUFCE_SHIFT
#define MCM_ISR_FIXCE_MASK                       MCM_ISCR_FIXCE_MASK
#define MCM_ISR_FIXCE_SHIFT                      MCM_ISCR_FIXCE_SHIFT
#define MCM_ISR_FIDCE_MASK                       MCM_ISCR_FIDCE_MASK
#define MCM_ISR_FIDCE_SHIFT                      MCM_ISCR_FIDCE_SHIFT
#define DSPI0                                    SPI0
#define DSPI1                                    SPI1
#define DSPI2                                    SPI2
#define FLEXCAN0                                 CAN0
#define PTA_BASE                                 GPIOA_BASE
#define PTA                                      GPIOA
#define PTB_BASE                                 GPIOB_BASE
#define PTB                                      GPIOB
#define PTC_BASE                                 GPIOC_BASE
#define PTC                                      GPIOC
#define PTD_BASE                                 GPIOD_BASE
#define PTD                                      GPIOD
#define PTE_BASE                                 GPIOE_BASE
#define PTE                                      GPIOE
#define UART_WP7816_T_TYPE0_REG(base)            UART_WP7816T0_REG(base)
#define UART_WP7816_T_TYPE1_REG(base)            UART_WP7816T1_REG(base)
#define UART_WP7816_T_TYPE0_WI_MASK              UART_WP7816T0_WI_MASK
#define UART_WP7816_T_TYPE0_WI_SHIFT             UART_WP7816T0_WI_SHIFT
#define UART_WP7816_T_TYPE0_WI(x)                UART_WP7816T0_WI(x)
#define UART_WP7816_T_TYPE1_BWI_MASK             UART_WP7816T1_BWI_MASK
#define UART_WP7816_T_TYPE1_BWI_SHIFT            UART_WP7816T1_BWI_SHIFT
#define UART_WP7816_T_TYPE1_BWI(x)               UART_WP7816T1_BWI(x)
#define UART_WP7816_T_TYPE1_CWI_MASK             UART_WP7816T1_CWI_MASK
#define UART_WP7816_T_TYPE1_CWI_SHIFT            UART_WP7816T1_CWI_SHIFT
#define UART_WP7816_T_TYPE1_CWI(x)               UART_WP7816T1_CWI(x)
#define Watchdog_IRQn                            WDOG_EWM_IRQn
#define Watchdog_IRQHandler                      WDOG_EWM_IRQHandler
#define LPTimer_IRQn                             LPTMR0_IRQn
#define LPTimer_IRQHandler                       LPTMR0_IRQHandler
#define LLW_IRQn                                 LLWU_IRQn
#define LLW_IRQHandler                           LLWU_IRQHandler
#define DMAMUX0                                  DMAMUX
#define WDOG0                                    WDOG
#define MCM0                                     MCM
#define RTC0                                     RTC

/*!
 * @}
 */ /* end of group SDK_Compatibility_Symbols */


#endif  /* _MK64F12_H_ */


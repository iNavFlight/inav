/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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
 * @file    SAM/sama5d2x.h
 * @brief   SAM A5 D2x inclusion header.
 *
 * @addtogroup SAMA5D2
 * @{
 */
    
#ifndef _SAMA5D2X_H_
#define _SAMA5D2X_H_

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "ARMCA5.h"
/**
  * @brief SAMA5D2 Family
  */
#if !defined  (SAMA5D2)
#define SAMA5D2
#endif /* SAMA5D2 */

/** 
  * @addtogroup Device_Included
  * @{
  */
#if defined(SAMA5D21)
  #include "sama5d21.h"
#elif defined(SAMA5D22)
  #include "sama5d22.h"
#elif defined(SAMA5D23)
  #include "sama5d23.h"
#elif defined(SAMA5D24)
  #include "sama5d24.h"
#elif defined(SAMA5D26)
  #include "sama5d26.h"
#elif defined(SAMA5D27)
  #include "sama5d27.h" 
#elif defined(SAMA5D28)
  #include "sama5d28.h"  
#else
 #error "Please select first the target SAMA5D2x device used in your application (in sama5d2x.h file)"
#endif

/**@} */

/* TODO: to delete */
#define Aes wc_Aes

/**
  * @brief SAMA5D2 Family
  * @{
  */
#define ID_SAIC_FIQ     ( 0) /**< \brief FIQ Interrupt ID (SAIC_FIQ) */
#define ID_PMC          ( 1) /**< \brief Power Management Controller (PMC) */
#define ID_ARM_PMU      ( 2) /**< \brief Performance Monitor Unit (PMU) (ARM_PMU) */
#define ID_PIT          ( 3) /**< \brief Periodic Interval Timer Interrupt (PIT) */
#define ID_WDT          ( 4) /**< \brief Watchdog timer Interrupt (WDT) */
#define ID_GMAC0        ( 5) /**< \brief Ethernet MAC (GMAC0) */
#define ID_XDMAC0       ( 6) /**< \brief DMA Controller 0 (XDMAC0) */
#define ID_XDMAC1       ( 7) /**< \brief DMA Controller 1 (XDMAC1) */
#define ID_ICM          ( 8) /**< \brief Integritry Check Monitor (ICM) */
#define ID_AES          ( 9) /**< \brief Advanced Enion Standard (AES) */
#define ID_AESB         (10) /**< \brief AES bridge (AESB) */
#define ID_TDES         (11) /**< \brief Triple Data Enion Standard (TDES) */
#define ID_SHA          (12) /**< \brief SHA Signature (SHA) */
#define ID_MPDDRC       (13) /**< \brief MPDDR controller (MPDDRC) */
#define ID_MATRIX1      (14) /**< \brief H32MX, 32-bit AHB Matrix (MATRIX1) */
#define ID_MATRIX0      (15) /**< \brief H64MX, 64-bit AHB Matrix (MATRIX0) */
#define ID_SECUMOD      (16) /**< \brief Secure Module (SECUMOD) */
#define ID_HSMC         (17) /**< \brief Multi-bit ECC Interrupt (HSMC) */
#define ID_PIOA         (18) /**< \brief Parallel I/O Controller (PIOA) */
#define ID_FLEXCOM0     (19) /**< \brief FLEXCOM 0 (FLEXCOM0) */
#define ID_USART0       (19) /**< \brief USART (USART0) from FLEXCOM0 */
#define ID_FCOMSPI0     (19) /**< \brief Serial Peripheral Interface (SPI0) from FLEXCOM0 */
#define ID_TWI0         (19) /**< \brief Two-Wire Interface (TWI0) from FLEXCOM0 */
#define ID_FLEXCOM1     (20) /**< \brief FLEXCOM 1 (FLEXCOM1) */
#define ID_USART1       (20) /**< \brief USART (USART1) from FLEXCOM1 */
#define ID_FCOMSPI1     (20) /**< \brief Serial Peripheral Interface (SPI1) from FLEXCOM1 */
#define ID_TWI1         (20) /**< \brief Two-Wire Interface (TWI1) from FLEXCOM1 */
#define ID_FLEXCOM2     (21) /**< \brief FLEXCOM 1 (FLEXCOM1) */
#define ID_USART2       (21) /**< \brief USART (USART1) from FLEXCOM1 */
#define ID_FCOMSPI2     (21) /**< \brief Serial Peripheral Interface (SPI1) from FLEXCOM1 */
#define ID_TWI2         (21) /**< \brief Two-Wire Interface (TWI1) from FLEXCOM1 */
#define ID_FLEXCOM3     (22) /**< \brief FLEXCOM 3 (FLEXCOM3) */
#define ID_USART3       (22) /**< \brief USART (USART3) from FLEXCOM3 */
#define ID_FCOMSPI3     (22) /**< \brief Serial Peripheral Interface (SPI3) from FLEXCOM3 */
#define ID_TWI3         (22) /**< \brief Two-Wire Interface (TWI3) from FLEXCOM3 */
#define ID_FLEXCOM4     (23) /**< \brief FLEXCOM 4 (FLEXCOM4) */
#define ID_USART4       (23) /**< \brief USART (USART4) from FLEXCOM4 */
#define ID_FCOMSPI4     (23) /**< \brief Serial Peripheral Interface (SPI4) from FLEXCOM4 */
#define ID_TWI4         (23) /**< \brief Two-Wire Interface (TWI4) from FLEXCOM4 */
#define ID_UART0        (24) /**< \brief UART 0 (UART0) */
#define ID_UART1        (25) /**< \brief UART 1 (UART1) */
#define ID_UART2        (26) /**< \brief UART 2 (UART2) */
#define ID_UART3        (27) /**< \brief UART 3 (UART3) */
#define ID_UART4        (28) /**< \brief UART 4 (UART4) */
#define ID_TWIHS0       (29) /**< \brief Two-Wire Interface 0 (TWIHS0) */
#define ID_TWIHS1       (30) /**< \brief Two-Wire Interface 1 (TWIHS1) */
#define ID_SDMMC0       (31) /**< \brief Secure Digital Multimedia Card Controller 0 (SDMMC0) */
#define ID_SDMMC1       (32) /**< \brief Secure Digital Multimedia Card Controller 1 (SDMMC1) */
#define ID_SPI0         (33) /**< \brief Serial Peripheral Interface 0 (SPI0) */
#define ID_SPI1         (34) /**< \brief Serial Peripheral Interface 1 (SPI1) */
#define ID_TC0          (35) /**< \brief Timer Counter 0 (ch. 0, 1, 2) (TC0) */
#define ID_TC1          (36) /**< \brief Timer Counter 1 (ch. 3, 4, 5) (TC1) */
#define ID_PWM          (38) /**< \brief Pulse Width Modulation Controller0 (ch. 0, 1, 2, 3) (PWM) */
#define ID_ADC          (40) /**< \brief Touch Screen ADC Controller (ADC) */
#define ID_UHPHS        (41) /**< \brief USB Host High Speed (UHPHS) */
#define ID_UDPHS        (42) /**< \brief USB Device High Speed (UDPHS) */
#define ID_SSC0         (43) /**< \brief Synchronous Serial Controller 0 (SSC0) */
#define ID_SSC1         (44) /**< \brief Synchronous Serial Controller 1 (SSC1) */
#define ID_LCDC         (45) /**< \brief LCD Controller (LCDC) */
#define ID_ISC          (46) /**< \brief Camera Interface (ISC) */
#define ID_TRNG         (47) /**< \brief True Random Number Generator (TRNG) */
#define ID_PDMIC        (48) /**< \brief Pulse Density Modulation Interface Controller (PDMIC) */
#define ID_AIC_IRQ      (49) /**< \brief IRQ Interrupt ID (AIC_IRQ) */
#define ID_SFC          (50) /**< \brief Fuse Controller (SFC) */
#define ID_SECURAM      (51) /**< \brief Secured RAM (SECURAM) */
#define ID_QSPI0        (52) /**< \brief QSPI 0 (QSPI0) */
#define ID_QSPI1        (53) /**< \brief QSPI 1 (QSPI1) */
#define ID_I2SC0        (54) /**< \brief Inter-IC Sound Controller 0 (I2SC0) */
#define ID_I2SC1        (55) /**< \brief Inter-IC Sound Controller 1 (I2SC1) */
#define ID_CAN0_INT0    (56) /**< \brief MCAN 0 Interrupt0 (CAN0_INT0) */
#define ID_CAN1_INT0    (57) /**< \brief MCAN 1 Interrupt0 (CAN1_INT0) */
#define ID_CLASSD       (59) /**< \brief Audio Class D amplifier (CLASSD) */
#define ID_SFR          (60) /**< \brief Special Function Register  (SFR) */
#define ID_SAIC         (61) /**< \brief Secured Advanced Interrupt Controller  (SAIC) */
#define ID_AIC          (62) /**< \brief Advanced Interrupt Controller  (AIC) */
#define ID_L2CC         (63) /**< \brief L2 Cache Controller (L2CC) */
#define ID_CAN0_INT1    (64) /**< \brief MCAN 0 Interrupt1 (CAN0_INT1) */
#define ID_CAN1_INT1    (65) /**< \brief MCAN 1 Interrupt1 (CAN1_INT1) */
#define ID_GMAC0_Q1     (66) /**< \brief GMAC Queue 1 Interrupt (GMAC0_Q1) */
#define ID_GMAC0_Q2     (67) /**< \brief GMAC Queue 2 Interrupt (GMAC0_Q2) */
#define ID_PIOB         (68) /**< \brief  (PIOB) */
#define ID_PIOC         (69) /**< \brief  (PIOC) */
#define ID_PIOD         (70) /**< \brief  (PIOD) */
#define ID_SDMMC0_TIMER (71) /**< \brief  (SDMMC0_TIMER) */
#define ID_SDMMC1_TIMER (72) /**< \brief  (SDMMC1_TIMER) */
#define ID_RSTC         (73) /**< \brief Reset Controller (RSTC) */
#define ID_SYSC         (74) /**< \brief System Controller Interrupt, RTC, RSTC, PMC (SYSC) */
#define ID_ACC          (75) /**< \brief Analog Comparator (ACC) */
#define ID_RXLP         (76) /**< \brief Uart Low Power (RXLP) */
#define ID_SFRBU        (77) /**< \brief Special Function Register Backup (SFRBU) */
#define ID_CHIPID       (78) /**< \brief Chip ID (CHIPID) */

#define ID_PERIPH_COUNT (79) /**< \brief Number of peripheral IDs */

/* XDMA Peripheral Interface Number */

#define PERID_TWIHS0_TX    0
#define PERID_TWIHS0_RX    1
#define PERID_TWIHS1_TX    2
#define PERID_TWIHS1_RX    3
#define PERID_QSPI0_TX     4
#define PERID_QSPI0_RX     5
#define PERID_SPI0_TX      6
#define PERID_SPI0_RX      7
#define PERID_SPI1_TX      8
#define PERID_SPI1_RX      9
#define PERID_PWM_TX       10
#define PERID_PWM_RX       0XFF
#define PERID_FLEXCOM0_TX  11
#define PERID_FLEXCOM0_RX  12
#define PERID_FLEXCOM1_TX  13
#define PERID_FLEXCOM1_RX  14
#define PERID_FLEXCOM2_TX  15
#define PERID_FLEXCOM2_RX  16
#define PERID_FLEXCOM3_TX  17
#define PERID_FLEXCOM3_RX  18
#define PERID_FLEXCOM4_TX  19
#define PERID_FLEXCOM4_RX  20
#define PERID_SSC0_TX      21
#define PERID_SSC0_RX      22
#define PERID_SSC1_TX      23
#define PERID_SSC1_RX      24
#define PERID_ADC_TX       0XFF
#define PERID_ADC_RX       25
#define PERID_AES_TX       26
#define PERID_AES_RX       27
#define PERID_TDES_TX      28
#define PERID_TDES_RX      29
#define PERID_SHA_TX       30
#define PERID_SHA_RX       0XFF
#define PERID_I2SC0_TX     31
#define PERID_I2SC0_RX     32
#define PERID_I2SC1_TX     33
#define PERID_I2SC1_RX     34
#define PERID_UART0_TX     35
#define PERID_UART0_RX     36
#define PERID_UART1_TX     37
#define PERID_UART1_RX     38
#define PERID_UART2_TX     39
#define PERID_UART2_RX     40
#define PERID_UART3_TX     41
#define PERID_UART3_RX     42
#define PERID_UART4_TX     43
#define PERID_UART4_RX     44
#define PERID_TC0_TX       0XFF
#define PERID_TC0_RX       45
#define PERID_TC1_TX       0XFF
#define PERID_TC1_RX       46
#define PERID_CLASSD_TX    47
#define PERID_CLASSD_RX    0XFF
#define PERID_QSPI1_TX     48
#define PERID_QSPI1_RX     49
#define PERID_PDMIC_TX     0XFF
#define PERID_PDMIC_RX     50

#define ID_SAIC_FIQ_MSK       (1 << (ID_SAIC_FIQ & 0x1F))
#define ID_PMC_MSK            (1 << (ID_PMC & 0x1F))
#define ID_ARM_PMU_MSK        (1 << (ID_ARM_PMU & 0x1F))
#define ID_PIT_MSK            (1 << (ID_PIT & 0x1F))
#define ID_WDT_MSK            (1 << (ID_WDT & 0x1F))
#define ID_GMAC0_MSK          (1 << (ID_GMAC0 & 0x1F))
#define ID_XDMAC0_MSK         (1 << (ID_XDMAC0 & 0x1F))
#define ID_XDMAC1_MSK         (1 << (ID_XDMAC1 & 0x1F))
#define ID_ICM_MSK            (1 << (ID_ICM & 0x1F))
#define ID_AES_MSK            (1 << (ID_AES & 0x1F))
#define ID_AESB_MSK           (1 << (ID_AESB & 0x1F))
#define ID_TDES_MSK           (1 << (ID_TDES & 0x1F))
#define ID_SHA_MSK            (1 << (ID_SHA & 0x1F))
#define ID_MPDDRC_MSK         (1 << (ID_MPDDRC & 0x1F))
#define ID_MATRIX1_MSK        (1 << (ID_MATRIX1 & 0x1F))
#define ID_MATRIX0_MSK        (1 << (ID_MATRIX0 & 0x1F))
#define ID_SECUMOD_MSK        (1 << (ID_SECUMOD & 0x1F))
#define ID_HSMC_MSK           (1 << (ID_HSMC & 0x1F))
#define ID_PIOA_MSK           (1 << (ID_PIOA & 0x1F))
#define ID_FLEXCOM0_MSK       (1 << (ID_FLEXCOM0 & 0x1F))
#define ID_USART0_MSK         (1 << (ID_USART0 & 0x1F))
#define ID_FCOMSPI0_MSK       (1 << (ID_FCOMSPI0 & 0x1F))
#define ID_TWI0_MSK           (1 << (ID_TWI0 & 0x1F))
#define ID_FLEXCOM1_MSK       (1 << (ID_FLEXCOM1 & 0x1F))
#define ID_USART1_MSK         (1 << (ID_USART1 & 0x1F))
#define ID_FCOMSPI1_MSK       (1 << (ID_FCOMSPI1 & 0x1F))
#define ID_TWI1_MSK           (1 << (ID_TWI1 & 0x1F))
#define ID_FLEXCOM2_MSK       (1 << (ID_FLEXCOM2 & 0x1F))
#define ID_USART2_MSK         (1 << (ID_USART2 & 0x1F))
#define ID_FCOMSPI2_MSK       (1 << (ID_FCOMSPI2 & 0x1F))
#define ID_TWI2_MSK           (1 << (ID_TWI2 & 0x1F))
#define ID_FLEXCOM3_MSK       (1 << (ID_FLEXCOM3 & 0x1F))
#define ID_USART3_MSK         (1 << (ID_USART3 & 0x1F))
#define ID_FCOMSPI3_MSK       (1 << (ID_FCOMSPI3 & 0x1F))
#define ID_TWI3_MSK           (1 << (ID_TWI3 & 0x1F))
#define ID_FLEXCOM4_MSK       (1 << (ID_FLEXCOM4 & 0x1F))
#define ID_USART4_MSK         (1 << (ID_USART4 & 0x1F))
#define ID_FCOMSPI4_MSK       (1 << (ID_FCOMSPI4 & 0x1F))
#define ID_TWI4_MSK           (1 << (ID_TWI4 & 0x1F))
#define ID_UART0_MSK          (1 << (ID_UART0 & 0x1F))
#define ID_UART1_MSK          (1 << (ID_UART1 & 0x1F))
#define ID_UART2_MSK          (1 << (ID_UART2 & 0x1F))
#define ID_UART3_MSK          (1 << (ID_UART3 & 0x1F))
#define ID_UART4_MSK          (1 << (ID_UART4 & 0x1F))
#define ID_TWIHS0_MSK         (1 << (ID_TWIHS0 & 0x1F))
#define ID_TWIHS1_MSK         (1 << (ID_TWIHS1 & 0x1F))
#define ID_SDMMC0_MSK         (1 << (ID_SDMMC0 & 0x1F))
#define ID_SDMMC1_MSK         (1 << (ID_SDMMC1 & 0x1F))
#define ID_SPI0_MSK           (1 << (ID_SPI0 & 0x1F))
#define ID_SPI1_MSK           (1 << (ID_SPI1 & 0x1F))
#define ID_TC0_MSK            (1 << (ID_TC0 & 0x1F))
#define ID_TC1_MSK            (1 << (ID_TC1 & 0x1F))
#define ID_PWM_MSK            (1 << (ID_PWM & 0x1F))
#define ID_ADC_MSK            (1 << (ID_ADC & 0x1F))
#define ID_UHPHS_MSK          (1 << (ID_UHPHS & 0x1F))
#define ID_UDPHS_MSK          (1 << (ID_UDPHS & 0x1F))
#define ID_SSC0_MSK           (1 << (ID_SSC0 & 0x1F))
#define ID_SSC1_MSK           (1 << (ID_SSC1 & 0x1F))
#define ID_LCDC_MSK           (1 << (ID_LCDC & 0x1F))
#define ID_ISC_MSK            (1 << (ID_ISC & 0x1F))
#define ID_TRNG_MSK           (1 << (ID_TRNG & 0x1F))
#define ID_PDMIC_MSK          (1 << (ID_PDMIC & 0x1F))
#define ID_AIC_IRQ_MSK        (1 << (ID_AIC_IRQ & 0x1F))
#define ID_SFC_MSK            (1 << (ID_SFC & 0x1F))
#define ID_SECURAM_MSK        (1 << (ID_SECURAM & 0x1F))
#define ID_QSPI0_MSK          (1 << (ID_QSPI0 & 0x1F))
#define ID_QSPI1_MSK          (1 << (ID_QSPI1 & 0x1F))
#define ID_I2SC0_MSK          (1 << (ID_I2SC0 & 0x1F))
#define ID_I2SC1_MSK          (1 << (ID_I2SC1 & 0x1F))
#define ID_CAN0_INT0_MSK      (1 << (ID_CAN0_INT0 & 0x1F))
#define ID_CAN1_INT0_MSK      (1 << (ID_CAN1_INT0 & 0x1F))
#define ID_CLASSD_MSK         (1 << (ID_CLASSD & 0x1F))
#define ID_SFR_MSK            (1 << (ID_SFR & 0x1F))
#define ID_SAIC_MSK           (1 << (ID_SAIC & 0x1F))
#define ID_AIC_MSK            (1 << (ID_AIC & 0x1F))
#define ID_L2CC_MSK           (1 << (ID_L2CC & 0x1F))
#define ID_CAN0_INT1_MSK      (1 << (ID_CAN0_INT1 & 0x1F))
#define ID_CAN1_INT1_MSK      (1 << (ID_CAN1_INT1 & 0x1F))
#define ID_GMAC0_Q1_MSK       (1 << (ID_GMAC0_Q1 & 0x1F))
#define ID_GMAC0_Q2_MSK       (1 << (ID_GMAC0_Q2 & 0x1F))
#define ID_PIOB_MSK           (1 << (ID_PIOB & 0x1F))
#define ID_PIOC_MSK           (1 << (ID_PIOC & 0x1F))
#define ID_PIOD_MSK           (1 << (ID_PIOD & 0x1F))
#define ID_SDMMC0_TIMER_MSK   (1 << (ID_SDMMC0_TIMER & 0x1F))
#define ID_SDMMC1_TIMER_MSK   (1 << (ID_SDMMC1_TIMER & 0x1F))
#define ID_RSTC_MSK           (1 << (ID_RSTC & 0x1F)
#define ID_SYSC_MSK           (1 << (ID_SYSC & 0x1F))
#define ID_ACC_MSK            (1 << (ID_ACC & 0x1F))
#define ID_RXLP_MSK           (1 << (ID_RXLP & 0x1F))
#define ID_SFRBU_MSK          (1 << (ID_SFRBU & 0x1F))
#define ID_CHIPID_MSK         (1 << (ID_CHIPID & 0x1F))

/* MASTER MATRIX ID DEFINITION FOR SAMA5D2x */

#define H64MX_MASTER_BRIDGE_FROM_AXI  0
#define H64MX_MASTER_XDMAC0_0         1
#define H64MX_MASTER_XDMAC0_1         2
#define H64MX_MASTER_XDMAC1_0         3
#define H64MX_MASTER_XDMAC1_1         4
#define H64MX_MASTER_LCDC_DMA_0       5
#define H64MX_MASTER_LCDC_DMA_1       6
#define H64MX_MASTER_SDMMC0           7
#define H64MX_MASTER_SDMMC1           8
#define H64MX_MASTER_ISC_DMA          9
#define H64MX_MASTER_AESB            10
#define H64MX_MASTER_BRIDGE_H64MX    11

#define H32MX_MASTER_BRIDGE_H32MX     0
#define H32MX_MASTER_ICM              1
#define H32MX_MASTER_UHPHS_EHCI_DMA   2
#define H32MX_MASTER_UHPHS_OHCI_DMA   3
#define H32MX_MASTER_UDPHS_DMA        4
#define H32MX_MASTER_GMAC_DMA         5
#define H32MX_MASTER_CAN0_DMA         6
#define H32MX_MASTER_CAN1_DMA         7

 /* SLAVE MATRIX ID DEFINITIONS FOR SAMA5D2x */

 #define H64MX_SLAVE_BRIDGE_H32MX     0
 #define H64MX_SLAVE_APB              1
 #define H64MX_SLAVE_SDMMC            1
 #define H64MX_SLAVE_DDR_PORT0        2
 #define H64MX_SLAVE_DDR_PORT1        3
 #define H64MX_SLAVE_DDR_PORT2        4
 #define H64MX_SLAVE_DDR_PORT3        5
 #define H64MX_SLAVE_DDR_PORT4        6
 #define H64MX_SLAVE_DDR_PORT5        7
 #define H64MX_SLAVE_DDR_PORT6        8
 #define H64MX_SLAVE_DDR_PORT7        9
 #define H64MX_SLAVE_SRAM            10
 #define H64MX_SLAVE_L2C_SRAM        11
 #define H64MX_SLAVE_QSPI0           12
 #define H64MX_SLAVE_QSPI1           13
 #define H64MX_SLAVE_AESB            14

 #define H32MX_SLAVE_BRIDGE_H64MX     0
 #define H32MX_SLAVE_APB0             1
 #define H32MX_SLAVE_APB1             2
 #define H32MX_SLAVE_EBI              3
 #define H32MX_SLAVE_NFC_CMD          3
 #define H32MX_SLAVE_NFC_SRAM         4
 #define H32MX_SLAVE_USB              5

#ifdef __cplusplus
}
#endif /* __cplusplus */
/**@} */
#endif /* __SAMA5D2X_H */


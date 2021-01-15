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

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Setup for Atmel SAM A5 D27 Xplained Ultra board.
 */

/*
 * Board identifier.
 */
#define BOARD_ATSAM5D2_XULT
#define BOARD_NAME                  "Atmel SAM A5 D27 Xplained Ultra"

/*
 * Ethernet PHY type.
 */
#define BOARD_PHY_ID                MII_KSZ8081_ID
#define BOARD_PHY_RMII

/*
 * Board oscillators-related settings.
 */
#if !defined(SAMA_OSCXTCLK)
#define SAMA_OSCXTCLK               32768U
#endif

#if !defined(SAMA_MOSCXTCLK)
#define SAMA_MOSCXTCLK              12000000U
#endif

/*
 * MCU type as defined in the Atmel header.
 */
#define SAMA5D27

/**
 * Port identifiers.
 */
#define SAMA_PIOA                   0U
#define SAMA_PIOB                   1U
#define SAMA_PIOC                   2U
#define SAMA_PIOD                   3U

/*
 * Forms a line identifier. In this driver the pad number is encoded in the
 * lower 5 bits of line and the port in sixth and seventh bits.
 */
#define SAMA_LINE(port, pad)                                                \
  ((uint32_t)((uint32_t)(port << 5U)) | ((uint32_t)(pad)))

/*
 * Decodes a port identifier from a line identifier.
 */
#define SAMA_PORT(line)                                                     \
  ((uint32_t)((line & 0xFFFFFFE0U) >> 5U)

/**
 * Decodes a pad identifier from a line identifier.
 */
#define SAMA_PAD(line)                                                      \
  ((uint32_t)(line & 0x0000001FU))


/*
 * IO pins assignments.
 */
#define PIOA_SDMMC0_CK              0U
#define PIOA_SDMMC0_CMD             1U
#define PIOA_SDMMC0_DAT0            2U
#define PIOA_SDMMC0_DAT1            3U
#define PIOA_SDMMC0_DAT2            4U
#define PIOA_SDMMC0_DAT3            5U
#define PIOA_SDMMC0_DAT4            6U
#define PIOA_SDMMC0_DAT5            7U
#define PIOA_SDMMC0_DAT6            8U
#define PIOA_SDMMC0_DAT7            9U
#define PIOA_SDMMC0_RSTN            10U
#define PIOA_PIN11                  11U
#define PIOA_PIN12                  12U
#define PIOA_SDMMC0_CD              13U
#define PIOA_PIN14                  14U
#define PIOA_PIN15                  15U
#define PIOA_PIN16                  16U
#define PIOA_PIN17                  17U
#define PIOA_SDMMC1_DAT0            18U
#define PIOA_SDMMC1_DAT1            19U
#define PIOA_SDMMC1_DAT2            20U
#define PIOA_SDMMC1_DAT3            21U
#define PIOA_SDMMC1_CK              22U
#define PIOA_PIN23                  23U
#define PIOA_PIN24                  24U
#define PIOA_PIN25                  25U
#define PIOA_PIN26                  26U
#define PIOA_PIN27                  27U
#define PIOA_SDMMC1_CMD             28U
#define PIOA_PIN29                  29U
#define PIOA_SDMMC1_CD              30U
#define PIOA_PIN31                  31U

#define PIOB_LED_BLUE               0U
#define PIOB_CLASSD_R0              1U
#define PIOB_CLASSD_R1              2U
#define PIOB_CLASSD_R2              3U
#define PIOB_CLASSD_R3              4U
#define PIOB_LED_GREEN              5U
#define PIOB_LED_RED                6U
#define PIOB_PIN7                   7U
#define PIOB_PIN8                   8U
#define PIOB_USER_PB                9U
#define PIOB_PIN10                  10U
#define PIOB_PIN11                  11U
#define PIOB_PIN12                  12U
#define PIOB_PIN13                  13U
#define PIOB_ETH_GTXCK              14U
#define PIOB_ETH_GTXEN              15U
#define PIOB_ETH_GRXDV              16U
#define PIOB_ETH_GRXER              17U
#define PIOB_ETH_GRX0               18U
#define PIOB_ETH_GRX1               19U
#define PIOB_ETH_GTX0               20U
#define PIOB_ETH_GTX1               21U
#define PIOB_ETH_GMDC               22U
#define PIOB_ETH_GMDIO              23U
#define PIOB_PIN24                  24U
#define PIOB_PIN25                  25U
#define PIOB_URXD0                  26U
#define PIOB_UTXD0                  27U
#define PIOB_UART_FLEXCOM0_IO0      28U
#define PIOB_UART_FLEXCOM0_IO1      29U
#define PIOB_PIN30                  30U
#define PIOB_PIN31                  31U

#define PIOC_PIO0                   0U
#define PIOC_PIN1                   1U
#define PIOC_PIN2                   2U
#define PIOC_PIN3                   3U
#define PIOC_PIN4                   4U
#define PIOC_PIN5                   5U
#define PIOC_PIN6                   6U
#define PIOC_PIN7                   7U
#define PIOC_PIN8                   8U
#define PIOC_PIN9                   9U
#define PIOC_PIN10                  10U
#define PIOC_PIN11                  11U
#define PIOC_PIN12                  12U
#define PIOC_PIN13                  13U
#define PIOC_PIN14                  14U
#define PIOC_PIN15                  15U
#define PIOC_PIN16                  16U
#define PIOC_PIN17                  17U
#define PIOC_PIN18                  18U
#define PIOC_PIN19                  19U
#define PIOC_PIN20                  20U
#define PIOC_PIN21                  21U
#define PIOC_PIN22                  22U
#define PIOC_PIN23                  23U
#define PIOC_PIN24                  24U
#define PIOC_PIN25                  25U
#define PIOC_PIN26                  26U
#define PIOC_PIN27                  27U
#define PIOC_PIN28                  28U
#define PIOC_PIN29                  29U
#define PIOC_PIN30                  30U
#define PIOC_PIN31                  31U

#define PIOD_PIO0                   0U
#define PIOD_PIN1                   1U
#define PIOD_URXD1                  2U
#define PIOD_UTXD1                  3U
#define PIOD_URXD2                  4U
#define PIOD_UTXD2                  5U
#define PIOD_PIN6                   6U
#define PIOD_PIN7                   7U
#define PIOD_PIN8                   8U
#define PIOD_PIN9                   9U
#define PIOD_PIN10                  10U
#define PIOD_TIOA1                  11U
#define PIOD_PIN12                  12U
#define PIOD_PIN13                  13U
#define PIOD_PIN14                  14U
#define PIOD_PIN15                  15U
#define PIOD_PIN16                  16U
#define PIOD_PIN17                  17U
#define PIOD_PIN18                  18U
#define PIOD_TWD1                   19U
#define PIOD_TWCK1                  20U
#define PIOD_UART_FLEXCOM4_IO0      21U
#define PIOD_UART_FLEXCOM4_IO1      22U
#define PIOD_PIN23                  23U
#define PIOD_UART_FLEXCOM4_IO3      24U
#define PIOD_UART_FLEXCOM4_IO4      25U
#define PIOD_SPI_FLEXCOM2_IO0       26U
#define PIOD_SPI_FLEXCOM2_IO1       27U
#define PIOD_SPI_FLEXCOM2_IO2       28U
#define PIOD_SPI_FLEXCOM2_IO3       29U
#define PIOD_PIN30                  30U
#define PIOD_PIN31                  31U

/*
 * IO lines assignments.
 */
#define BOARD_LINE(port, pad)                                               \
  ((uint32_t)((uint32_t)(port)) | ((uint32_t)(pad)))

#define LINE_LED_BLUE               BOARD_LINE(PIOB, 0U)
#define LINE_LED_GREEN              BOARD_LINE(PIOB, 5U)
#define LINE_LED_RED                BOARD_LINE(PIOB, 6U)
#define LINE_USER_PB                BOARD_LINE(PIOB, 9U)

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */

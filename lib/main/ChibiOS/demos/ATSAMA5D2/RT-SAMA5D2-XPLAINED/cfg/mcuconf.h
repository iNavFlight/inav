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

#ifndef MCUCONF_H
#define MCUCONF_H

#define SAMA5D2x_MCUCONF

/*
 * HAL driver system settings.
 */
#define SAMA_HAL_IS_SECURE                  TRUE
#define SAMA_NO_INIT                        TRUE
#define SAMA_MOSCRC_ENABLED                 FALSE
#define SAMA_MOSCXT_ENABLED                 TRUE
#define SAMA_MOSC_SEL                       SAMA_MOSC_MOSCXT
#define SAMA_OSC_SEL                        SAMA_OSC_OSCXT
#define SAMA_MCK_SEL                        SAMA_MCK_PLLA_CLK
#define SAMA_MCK_PRES_VALUE                 1
#define SAMA_MCK_MDIV_VALUE                 3
#define SAMA_PLLA_MUL_VALUE                 83
#define SAMA_PLLADIV2_EN                    TRUE
#define SAMA_H64MX_H32MX_RATIO              2

/*
 * CLASSD driver system settings.
 */
#define SAMA_USE_CLASSD                     FALSE
#define SAMA_CLASSD_DMA_IRQ_PRIORITY        4
#define SAMA_CLASSD_DMA_ERROR_HOOK(classdp) osalSysHalt("DMA failure")

/*
 * CRY driver system settings.
 */
#define PLATFORM_CRY_USE_CRY1               FALSE

/*
 * I2C driver system settings.
 */
#define SAMA_I2C_USE_TWIHS0                 FALSE
#define SAMA_I2C_USE_TWIHS1                 FALSE
#define SAMA_I2C_BUSY_TIMEOUT               50
#define SAMA_I2C_TWIHS0_IRQ_PRIORITY        6
#define SAMA_I2C_TWIHS1_IRQ_PRIORITY        6
#define SAMA_I2C_TWIHS0_DMA_IRQ_PRIORITY    6
#define SAMA_I2C_TWIHS1_DMA_IRQ_PRIORITY    6
#define SAMA_I2C_DMA_ERROR_HOOK(i2cp)       osalSysHalt("DMA failure")

/*
 * L2CC related defines.
 */
#define SAMA_L2CC_ASSUME_ENABLED            0
#define SAMA_L2CC_ENABLE                    1

/*
 * ONEWIRE driver system settings.
 */
#define SAMA_USE_ONEWIRE                    FALSE

/*
 * LCDC driver system settings.
 */
#define SAMA_USE_LCDC                       FALSE

/*
 * SDMMC driver system settings.
 */
#define SAMA_USE_SDMMC                      FALSE
#define PLATFORM_SDMMC_USE_SDMMC1           FALSE

/*
 * SECUMOD driver system settings.
 */
#define SAMA_USE_SECUMOD                    FALSE

/*
 * SERIAL driver system settings.
 */
#define SAMA_SERIAL_USE_UART0               FALSE
#define SAMA_SERIAL_USE_UART1               TRUE
#define SAMA_SERIAL_USE_UART2               FALSE
#define SAMA_SERIAL_USE_UART3               FALSE
#define SAMA_SERIAL_USE_UART4               FALSE
#define SAMA_SERIAL_USE_FLEXCOM0            FALSE
#define SAMA_SERIAL_USE_FLEXCOM1            FALSE
#define SAMA_SERIAL_USE_FLEXCOM2            FALSE
#define SAMA_SERIAL_USE_FLEXCOM3            FALSE
#define SAMA_SERIAL_USE_FLEXCOM4            FALSE
#define SAMA_SERIAL_UART0_IRQ_PRIORITY      4
#define SAMA_SERIAL_UART1_IRQ_PRIORITY      4
#define SAMA_SERIAL_UART2_IRQ_PRIORITY      4
#define SAMA_SERIAL_UART3_IRQ_PRIORITY      4
#define SAMA_SERIAL_UART4_IRQ_PRIORITY      4
#define SAMA_SERIAL_FLEXCOM0_IRQ_PRIORITY   4
#define SAMA_SERIAL_FLEXCOM1_IRQ_PRIORITY   4
#define SAMA_SERIAL_FLEXCOM2_IRQ_PRIORITY   4
#define SAMA_SERIAL_FLEXCOM3_IRQ_PRIORITY   4
#define SAMA_SERIAL_FLEXCOM4_IRQ_PRIORITY   4

/*
 * SPI driver system settings.
 */
#define SAMA_SPI_USE_SPI0                   FALSE
#define SAMA_SPI0_USE_GCLK                  FALSE
#define SAMA_SPI0_GCLK_SOURCE               SAMA_GCLK_MCK_CLK
#define SAMA_SPI0_GCLK_DIV                  21

#define SAMA_SPI_USE_SPI1                   FALSE
#define SAMA_SPI1_USE_GCLK                  FALSE
#define SAMA_SPI1_GCLK_SOURCE               SAMA_GCLK_MCK_CLK
#define SAMA_SPI1_GCLK_DIV                  21

#define SAMA_SPI_USE_FLEXCOM0               FALSE
#define SAMA_FSPI0_USE_GCLK                 FALSE
#define SAMA_FSPI0_GCLK_SOURCE              SAMA_GCLK_MCK_CLK
#define SAMA_FSPI0_GCLK_DIV                 21

#define SAMA_SPI_USE_FLEXCOM1               FALSE
#define SAMA_FSPI1_USE_GCLK                 FALSE
#define SAMA_FSPI1_GCLK_SOURCE              SAMA_GCLK_MCK_CLK
#define SAMA_FSPI1_GCLK_DIV                 21

#define SAMA_SPI_USE_FLEXCOM2               FALSE
#define SAMA_FSPI2_USE_GCLK                 FALSE
#define SAMA_FSPI2_GCLK_SOURCE              SAMA_GCLK_MCK_CLK
#define SAMA_FSPI2_GCLK_DIV                 21

#define SAMA_SPI_USE_FLEXCOM3               FALSE
#define SAMA_FSPI3_USE_GCLK                 FALSE
#define SAMA_FSPI3_GCLK_SOURCE              SAMA_GCLK_MCK_CLK
#define SAMA_FSPI3_GCLK_DIV                 21

#define SAMA_SPI_USE_FLEXCOM4               FALSE
#define SAMA_FSPI4_USE_GCLK                 FALSE
#define SAMA_FSPI4_GCLK_SOURCE              SAMA_GCLK_MCK_CLK
#define SAMA_FSPI4_GCLK_DIV                 21

#define SAMA_SPI_DMA_ERROR_HOOK(spip)       osalSysHalt("DMA failure")
#define SAMA_SPI_CACHE_USER_MANAGED         FALSE

/*
 * ST driver settings.
 */
#define SAMA_ST_USE_PIT                     TRUE
#define SAMA_ST_USE_TC0                     FALSE
#define SAMA_ST_USE_TC1                     FALSE

/*
 * TC driver system settings.
 */
#define SAMA_USE_TC                         FALSE
#define SAMA_USE_TC0                        FALSE
#define SAMA_USE_TC1                        FALSE
#define SAMA_TC0_IRQ_PRIORITY               2
#define SAMA_TC1_IRQ_PRIORITY               2

/*
 * TRNG driver system settings.
 */
#define SAMA_TRNG_USE_TRNG0                 FALSE

/*
 * UART driver system settings.
 */
#define SAMA_UART_USE_UART0                 FALSE
#define SAMA_UART_USE_UART1                 FALSE
#define SAMA_UART_USE_UART2                 FALSE
#define SAMA_UART_USE_UART3                 FALSE
#define SAMA_UART_USE_UART4                 FALSE
#define SAMA_UART_USE_FLEXCOM0              FALSE
#define SAMA_UART_USE_FLEXCOM1              FALSE
#define SAMA_UART_USE_FLEXCOM2              FALSE
#define SAMA_UART_USE_FLEXCOM3              FALSE
#define SAMA_UART_USE_FLEXCOM4              FALSE
#define SAMA_UART_UART0_IRQ_PRIORITY        4
#define SAMA_UART_UART1_IRQ_PRIORITY        4
#define SAMA_UART_UART2_IRQ_PRIORITY        4
#define SAMA_UART_UART3_IRQ_PRIORITY        4
#define SAMA_UART_UART4_IRQ_PRIORITY        4
#define SAMA_UART_FLEXCOM0_IRQ_PRIORITY     4
#define SAMA_UART_FLEXCOM1_IRQ_PRIORITY     4
#define SAMA_UART_FLEXCOM2_IRQ_PRIORITY     4
#define SAMA_UART_FLEXCOM3_IRQ_PRIORITY     4
#define SAMA_UART_FLEXCOM4_IRQ_PRIORITY     4
#define SAMA_UART_UART0_DMA_IRQ_PRIORITY    4
#define SAMA_UART_UART1_DMA_IRQ_PRIORITY    4
#define SAMA_UART_UART2_DMA_IRQ_PRIORITY    4
#define SAMA_UART_UART3_DMA_IRQ_PRIORITY    4
#define SAMA_UART_UART4_DMA_IRQ_PRIORITY    4
#define SAMA_UART_FLEXCOM0_DMA_IRQ_PRIORITY 4
#define SAMA_UART_FLEXCOM1_DMA_IRQ_PRIORITY 4
#define SAMA_UART_FLEXCOM2_DMA_IRQ_PRIORITY 4
#define SAMA_UART_FLEXCOM3_DMA_IRQ_PRIORITY 4
#define SAMA_UART_FLEXCOM4_DMA_IRQ_PRIORITY 4
#define SAMA_UART_DMA_ERROR_HOOK(uartp)     osalSysHalt("DMA failure")
#define SAMA_UART_CACHE_USER_MANAGED        FALSE

/*
 * WSPI driver system settings.
 */
#define SAMA_WSPI_USE_QUADSPI0              FALSE
#define SAMA_WSPI_USE_QUADSPI1              FALSE
#define SAMA_WSPI_QUADSPI0_IRQ_PRIORITY     7
#define SAMA_WSPI_QUADSPI1_IRQ_PRIORITY     7
#define SAMA_WSPI_QUADSPI0_DMA_IRQ_PRIORITY 7
#define SAMA_WSPI_QUADSPI0_DMA_IRQ_PRIORITY 7
#define SAMA_WSPI_DMA_ERROR_HOOK(qspip)     osalSysHalt("DMA failure")
#define SAMA_WSPI_CACHE_USER_MANAGED        FALSE

#endif /* MCUCONF_H */

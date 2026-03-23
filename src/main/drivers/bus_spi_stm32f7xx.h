/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * STM32F7 SPI pin alternate function lookup table.
 *
 * Usage: SPI_PIN_AF_HELPER(3, PB5) expands to SPI_PIN_AF_SPI3_PB5,
 * which is defined below as GPIO_AF6_SPI3.
 *
 * AF assignments from STM32F722 datasheet (DS11853) and STM32F745 datasheet
 * (DS10916), Table 9.
 *
 * NOTE: The F7 AF table differs from the H7 for SPI3/PB5:
 *   On F7, PB5 SPI3_MOSI is AF6 (the default).
 *   On H7, PB5 SPI3_MOSI is AF7 (an exception — see bus_spi_stm32h7xx.h).
 *
 * SPI1, SPI2, SPI4 pins are all AF5 with no exceptions.
 * SPI3 SCK/MISO are AF6. SPI3 MOSI has pin-dependent exceptions:
 *   PB2 uses AF7 (not AF6). PD6 uses AF5 (not AF6). PC12 and PB5 use AF6.
 */

#pragma once

#include "common/utils.h"

// Resolves to SPI_PIN_AF_SPIn_Pxy, defined below for each valid pin.
// If a pin is not in the table the build will fail with "undefined identifier",
// which is preferable to silently applying the wrong AF.
#define SPI_PIN_AF_HELPER(spi, pin)  CONCAT4(SPI_PIN_AF_SPI, spi, _, pin)

/* SPI1 — all data pins use AF5 */
#define SPI_PIN_AF_SPI1_PA5    GPIO_AF5_SPI1   // SCK
#define SPI_PIN_AF_SPI1_PA6    GPIO_AF5_SPI1   // MISO
#define SPI_PIN_AF_SPI1_PA7    GPIO_AF5_SPI1   // MOSI
#define SPI_PIN_AF_SPI1_PB3    GPIO_AF5_SPI1   // SCK
#define SPI_PIN_AF_SPI1_PB4    GPIO_AF5_SPI1   // MISO
#define SPI_PIN_AF_SPI1_PB5    GPIO_AF5_SPI1   // MOSI

/* SPI2 — all data pins use AF5 */
#define SPI_PIN_AF_SPI2_PB13   GPIO_AF5_SPI2   // SCK
#define SPI_PIN_AF_SPI2_PB14   GPIO_AF5_SPI2   // MISO
#define SPI_PIN_AF_SPI2_PB15   GPIO_AF5_SPI2   // MOSI
#define SPI_PIN_AF_SPI2_PC1    GPIO_AF5_SPI2   // MOSI
#define SPI_PIN_AF_SPI2_PC2    GPIO_AF5_SPI2   // MISO
#define SPI_PIN_AF_SPI2_PC3    GPIO_AF5_SPI2   // MOSI

/*
 * SPI3 — SCK and MISO use AF6, but MOSI has pin-dependent exceptions.
 * PB2 carries SPI3_MOSI on AF7 (not AF6). PD6 uses AF5. PB5 and PC12 use AF6.
 */
#define SPI_PIN_AF_SPI3_PB2    GPIO_AF7_SPI3   // MOSI — exception: AF7, not AF6
#define SPI_PIN_AF_SPI3_PB3    GPIO_AF6_SPI3   // SCK
#define SPI_PIN_AF_SPI3_PB4    GPIO_AF6_SPI3   // MISO
#define SPI_PIN_AF_SPI3_PB5    GPIO_AF6_SPI3   // MOSI (AF6 on F7; H7 uses AF7 for this pin)
#define SPI_PIN_AF_SPI3_PC10   GPIO_AF6_SPI3   // SCK
#define SPI_PIN_AF_SPI3_PC11   GPIO_AF6_SPI3   // MISO
#define SPI_PIN_AF_SPI3_PC12   GPIO_AF6_SPI3   // MOSI
#define SPI_PIN_AF_SPI3_PD6    GPIO_AF5_SPI3   // MOSI — exception: AF5, not AF6

/* SPI4 — all data pins use AF5 */
#define SPI_PIN_AF_SPI4_PE2    GPIO_AF5_SPI4   // SCK
#define SPI_PIN_AF_SPI4_PE5    GPIO_AF5_SPI4   // MISO
#define SPI_PIN_AF_SPI4_PE6    GPIO_AF5_SPI4   // MOSI
#define SPI_PIN_AF_SPI4_PE12   GPIO_AF5_SPI4   // SCK
#define SPI_PIN_AF_SPI4_PE13   GPIO_AF5_SPI4   // MISO
#define SPI_PIN_AF_SPI4_PE14   GPIO_AF5_SPI4   // MOSI

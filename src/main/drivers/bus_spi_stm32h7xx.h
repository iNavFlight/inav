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
 * STM32H7 SPI pin alternate function lookup table.
 *
 * Usage: SPI_PIN_AF_HELPER(3, PB5) expands to SPI_PIN_AF_SPI3_PB5,
 * which is defined below as GPIO_AF7_SPI3.
 *
 * This allows bus_spi_hal_ll.c to resolve the correct AF per pin automatically,
 * without requiring each target to define SPI*_SCK_AF / SPI*_MISO_AF / SPI*_MOSI_AF
 * manually in target.h. Targets may still override individual values if needed.
 *
 * Alternate function assignments are from the STM32H743 datasheet (DS12110),
 * Tables 10-14.
 */

#pragma once

#include "common/utils.h"

// Resolves to SPI_PIN_AF_SPIn_Pxy, which is defined below for each valid pin.
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
#define SPI_PIN_AF_SPI1_PD7    GPIO_AF5_SPI1   // MOSI

/* SPI2 — all data pins use AF5 */
#define SPI_PIN_AF_SPI2_PA9    GPIO_AF5_SPI2   // SCK
#define SPI_PIN_AF_SPI2_PB10   GPIO_AF5_SPI2   // SCK
#define SPI_PIN_AF_SPI2_PB13   GPIO_AF5_SPI2   // SCK
#define SPI_PIN_AF_SPI2_PB14   GPIO_AF5_SPI2   // MISO
#define SPI_PIN_AF_SPI2_PB15   GPIO_AF5_SPI2   // MOSI
#define SPI_PIN_AF_SPI2_PC1    GPIO_AF5_SPI2   // MOSI
#define SPI_PIN_AF_SPI2_PC2    GPIO_AF5_SPI2   // MISO
#define SPI_PIN_AF_SPI2_PC3    GPIO_AF5_SPI2   // MOSI
#define SPI_PIN_AF_SPI2_PD3    GPIO_AF5_SPI2   // SCK

/*
 * SPI3 — SCK and MISO use AF6, but MOSI has pin-dependent exceptions.
 * PB2 and PB5 carry SPI3_MOSI on AF7 (not AF6). PD6 uses AF5.
 * This is the only SPI bus on STM32H743 where a single-AF fallback is wrong.
 */
#define SPI_PIN_AF_SPI3_PB2    GPIO_AF7_SPI3   // MOSI — exception: AF7, not AF6
#define SPI_PIN_AF_SPI3_PB3    GPIO_AF6_SPI3   // SCK
#define SPI_PIN_AF_SPI3_PB4    GPIO_AF6_SPI3   // MISO
#define SPI_PIN_AF_SPI3_PB5    GPIO_AF7_SPI3   // MOSI — exception: AF7, not AF6
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

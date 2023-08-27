/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Dominic Clifton
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#ifdef USE_QUADSPI

#include "drivers/bus_quadspi.h"
#include "drivers/bus_quadspi_impl.h"
#include "drivers/exti.h"
#include "drivers/io.h"
#include "drivers/rcc.h"

static const quadSpiConfig_t quadSpiConfig[] = {
#ifdef USE_QUADSPI_DEVICE_1
        { .device = QUADSPIDEV_1, .clk = IO_TAG(QUADSPI1_SCK_PIN),
          .bk1CS = IO_TAG(QUADSPI1_BK1_CS_PIN),
          .bk1IO0 = IO_TAG(QUADSPI1_BK1_IO0_PIN), .bk1IO1 = IO_TAG(QUADSPI1_BK1_IO1_PIN), .bk1IO2 = IO_TAG(QUADSPI1_BK1_IO2_PIN), .bk1IO3 = IO_TAG(QUADSPI1_BK1_IO3_PIN),
          .bk2CS = IO_TAG(QUADSPI1_BK2_CS_PIN),
          .bk2IO0 = IO_TAG(QUADSPI1_BK2_IO0_PIN), .bk2IO1 = IO_TAG(QUADSPI1_BK2_IO1_PIN), .bk2IO2 = IO_TAG(QUADSPI1_BK2_IO2_PIN), .bk2IO3 = IO_TAG(QUADSPI1_BK2_IO3_PIN),
          .mode = QUADSPI1_MODE, .csFlags = QUADSPI1_CS_FLAGS
        }
#endif
};

quadSpiDevice_t quadSpiDevice[QUADSPIDEV_COUNT] = { 0 };

QUADSPIDevice quadSpiDeviceByInstance(QUADSPI_TypeDef *instance)
{
#ifdef USE_QUADSPI_DEVICE_1
    if (instance == QUADSPI) {
        return QUADSPIDEV_1;
    }
#endif

    return QUADSPIINVALID;
}

QUADSPI_TypeDef *quadSpiInstanceByDevice(QUADSPIDevice device)
{
    if (device == QUADSPIINVALID || device >= QUADSPIDEV_COUNT) {
        return NULL;
    }

    return quadSpiDevice[device].dev;
}

bool quadSpiInit(QUADSPIDevice device)
{
    switch (device) {
    case QUADSPIINVALID:
        return false;
    case QUADSPIDEV_1:
#ifdef USE_QUADSPI_DEVICE_1
        quadSpiInitDevice(device);
        return true;
#else
        break;
#endif
    }
    return false;
}

uint32_t quadSpiTimeoutUserCallback(QUADSPI_TypeDef *instance)
{
    QUADSPIDevice device = quadSpiDeviceByInstance(instance);
    if (device == QUADSPIINVALID) {
        return -1;
    }
    quadSpiDevice[device].errorCount++;
    return quadSpiDevice[device].errorCount;
}

uint16_t quadSpiGetErrorCounter(QUADSPI_TypeDef *instance)
{
    QUADSPIDevice device = quadSpiDeviceByInstance(instance);
    if (device == QUADSPIINVALID) {
        return 0;
    }
    return quadSpiDevice[device].errorCount;
}

void quadSpiResetErrorCounter(QUADSPI_TypeDef *instance)
{
    QUADSPIDevice device = quadSpiDeviceByInstance(instance);
    if (device != QUADSPIINVALID) {
        quadSpiDevice[device].errorCount = 0;
    }
}

const quadSpiHardware_t quadSpiHardware[] = {
#ifdef STM32H7
    {
        .device = QUADSPIDEV_1,
        .reg = QUADSPI,
        .clkPins = {
            { DEFIO_TAG_E(PB2),  GPIO_AF9_QUADSPI },
        },
        .bk1IO0Pins = {
            { DEFIO_TAG_E(PC9),  GPIO_AF9_QUADSPI },
            { DEFIO_TAG_E(PD11), GPIO_AF9_QUADSPI },
            { DEFIO_TAG_E(PF8),  GPIO_AF10_QUADSPI },
        },
        .bk1IO1Pins = {
            { DEFIO_TAG_E(PC10), GPIO_AF9_QUADSPI },
            { DEFIO_TAG_E(PD12), GPIO_AF9_QUADSPI },
            { DEFIO_TAG_E(PF9),  GPIO_AF10_QUADSPI },
        },
        .bk1IO2Pins = {
            { DEFIO_TAG_E(PE2),  GPIO_AF9_QUADSPI },
            { DEFIO_TAG_E(PF7),  GPIO_AF9_QUADSPI },
        },
        .bk1IO3Pins = {
            { DEFIO_TAG_E(PA1),  GPIO_AF9_QUADSPI },
            { DEFIO_TAG_E(PD13), GPIO_AF9_QUADSPI },
            { DEFIO_TAG_E(PF6),  GPIO_AF9_QUADSPI },
        },
        .bk1CSPins = {
            { DEFIO_TAG_E(PB6),  GPIO_AF10_QUADSPI },
            { DEFIO_TAG_E(PB10), GPIO_AF9_QUADSPI },
            { DEFIO_TAG_E(PG6),  GPIO_AF10_QUADSPI },
        },
        .bk2IO0Pins = {
            { DEFIO_TAG_E(PE7),  GPIO_AF10_QUADSPI },
            //{ DEFIO_TAG_E(PH7),  GPIO_AF9_QUADSPI }, // FIXME regenerate io_def_generated with support for GPIO 'H'
        },
        .bk2IO1Pins = {
            { DEFIO_TAG_E(PE8),  GPIO_AF10_QUADSPI },
            //{ DEFIO_TAG_E(PH3),  GPIO_AF9_QUADSPI }, // FIXME regenerate io_def_generated with support for GPIO 'H'
        },
        .bk2IO2Pins = {
            { DEFIO_TAG_E(PE9),  GPIO_AF10_QUADSPI },
            { DEFIO_TAG_E(PG9),  GPIO_AF9_QUADSPI },
        },
        .bk2IO3Pins = {
            { DEFIO_TAG_E(PE10),  GPIO_AF10_QUADSPI },
            { DEFIO_TAG_E(PG14),  GPIO_AF9_QUADSPI },
        },
        .bk2CSPins = {
            { DEFIO_TAG_E(PC11),  GPIO_AF9_QUADSPI },
        },
        .rcc = RCC_AHB3(QSPI),
    },
#endif
};

const quadSpiConfig_t * getQuadSpiConfig(QUADSPIDevice device)
{
    const quadSpiConfig_t * pConfig = NULL;

    for (size_t configIndex = 0; configIndex < ARRAYLEN(quadSpiConfig); configIndex++) {
        if (quadSpiConfig[configIndex].device == device) {
            pConfig = &quadSpiConfig[configIndex];
            break;
        }
    }

    return pConfig;
}


void quadSpiPinConfigure(QUADSPIDevice device)
{
    if (device == QUADSPIINVALID) {
        return;
    }

    const quadSpiConfig_t * pConfig = getQuadSpiConfig(device);

    if (pConfig == NULL) {
        return;
    }

    for (size_t hwindex = 0; hwindex < ARRAYLEN(quadSpiHardware); hwindex++) {
        const quadSpiHardware_t *hw = &quadSpiHardware[hwindex];

        if (hw->device != device) {
            continue;
        }

        quadSpiDevice_t *pDev = &quadSpiDevice[device];

        for (int pindex = 0; pindex < MAX_QUADSPI_PIN_SEL; pindex++) {
            if (pConfig->clk == hw->clkPins[pindex].pin) {
                pDev->clk = hw->clkPins[pindex].pin;
            }
            //
            // BK1
            //
            if (pConfig->bk1IO0 == hw->bk1IO0Pins[pindex].pin) {
                pDev->bk1IO0 = hw->bk1IO0Pins[pindex].pin;
                pDev->bk1IO0AF = hw->bk1IO0Pins[pindex].af;
            }
            if (pConfig->bk1IO1 == hw->bk1IO1Pins[pindex].pin) {
                pDev->bk1IO1 = hw->bk1IO1Pins[pindex].pin;
                pDev->bk1IO1AF = hw->bk1IO1Pins[pindex].af;
            }
            if (pConfig->bk1IO2 == hw->bk1IO2Pins[pindex].pin) {
                pDev->bk1IO2 = hw->bk1IO2Pins[pindex].pin;
                pDev->bk1IO2AF = hw->bk1IO2Pins[pindex].af;
            }
            if (pConfig->bk1IO3 == hw->bk1IO3Pins[pindex].pin) {
                pDev->bk1IO3 = hw->bk1IO3Pins[pindex].pin;
                pDev->bk1IO3AF = hw->bk1IO3Pins[pindex].af;
            }
            if (pConfig->bk1CS == hw->bk1CSPins[pindex].pin) {
                pDev->bk1CS = hw->bk1CSPins[pindex].pin;
                pDev->bk1CSAF = hw->bk1CSPins[pindex].af;
            }
            //
            // BK2
            //
            if (pConfig->bk2IO0 == hw->bk2IO0Pins[pindex].pin) {
                pDev->bk2IO0 = hw->bk2IO0Pins[pindex].pin;
                pDev->bk2IO0AF = hw->bk2IO0Pins[pindex].af;
            }
            if (pConfig->bk2IO1 == hw->bk2IO1Pins[pindex].pin) {
                pDev->bk2IO1 = hw->bk2IO1Pins[pindex].pin;
                pDev->bk2IO1AF = hw->bk2IO1Pins[pindex].af;
            }
            if (pConfig->bk2IO1 == hw->bk2IO2Pins[pindex].pin) {
                pDev->bk2IO2 = hw->bk2IO2Pins[pindex].pin;
                pDev->bk2IO2AF = hw->bk2IO2Pins[pindex].af;
            }
            if (pConfig->bk2IO2 == hw->bk2IO3Pins[pindex].pin) {
                pDev->bk2IO3 = hw->bk2IO3Pins[pindex].pin;
                pDev->bk2IO3AF = hw->bk2IO3Pins[pindex].af;
            }
            if (pConfig->bk2IO3 == hw->bk2CSPins[pindex].pin) {
                pDev->bk2CS = hw->bk2CSPins[pindex].pin;
                pDev->bk2CSAF = hw->bk2CSPins[pindex].af;
            }
        }

        if ((pConfig->csFlags & QUADSPI_BK1_CS_MASK) == QUADSPI_BK1_CS_SOFTWARE) {
            pDev->bk1CS = pConfig->bk1CS;
        }
        if ((pConfig->csFlags & QUADSPI_BK2_CS_MASK) == QUADSPI_BK2_CS_SOFTWARE) {
            pDev->bk2CS = pConfig->bk2CS;
        }

        bool haveResources = true;

        // clock pins

        haveResources = haveResources && pDev->clk;

        // data pins

        bool needBK1 = (pConfig->mode == QUADSPI_MODE_DUAL_FLASH) || (pConfig->mode == QUADSPI_MODE_BK1_ONLY);
        if (needBK1) {
            bool haveBK1Resources = pDev->bk1IO0 && pDev->bk1IO1 && pDev->bk1IO2 && pDev->bk1IO3 && pDev->bk1CS;
            haveResources = haveResources && haveBK1Resources;
        }

        bool needBK2 = (pConfig->mode == QUADSPI_MODE_DUAL_FLASH) || (pConfig->mode == QUADSPI_MODE_BK2_ONLY);
        if (needBK2) {
            bool haveBK2Resources = pDev->bk2IO0 && pDev->bk2IO1 && pDev->bk2IO2 && pDev->bk2IO3;
            haveResources = haveResources && haveBK2Resources;
        }

        // cs pins

        if (needBK1) {
            haveResources = haveResources && pDev->bk1CS;
        }

        bool needBK2CS =
            (pConfig->mode == QUADSPI_MODE_DUAL_FLASH && (pConfig->csFlags & QUADSPI_CS_MODE_MASK) == QUADSPI_CS_MODE_SEPARATE) ||
            (pConfig->mode == QUADSPI_MODE_BK2_ONLY);

        if (needBK2CS) {
            haveResources = haveResources && pDev->bk2CS;
        }

        if (haveResources) {
            pDev->dev = hw->reg;
            pDev->rcc = hw->rcc;
        }

        // copy mode and flags
        pDev->mode = pConfig->mode;
        pDev->csFlags = pConfig->csFlags;
    }
}

#endif

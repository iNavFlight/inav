/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

/*
 * RP2350 I2C driver — blocking, Pico SDK hardware_i2c.
 *
 * Implements the INAV bus_i2c.h interface using the Pico SDK blocking API.
 * No interrupts are used; all transfers complete before returning.
 *
 * Hardware mapping (Option C pin plan):
 *   INAV I2CDEV_1 → RP2350 i2c1 → GP18 (SDA = PB2) / GP19 (SCL = PB3)
 *
 * The allowRawAccess parameter:
 *   When reg == 0xFF and allowRawAccess == true, the register sub-address byte
 *   is omitted, allowing raw I2C transactions (used by some mag drivers).
 *
 * Reference: Pico SDK hardware_i2c API
 *            INAV src/main/drivers/bus_i2c_hal.c (STM32 reference)
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#ifdef USE_I2C

#include "drivers/bus_i2c.h"
#include "drivers/io.h"
#include "drivers/io_def.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"

#define RP2350_I2C_TIMEOUT_US   10000U   /* 10 ms — generous for 100–800 kHz */

/*
 * Maximum data payload for a register-prefixed write.
 * Sensor registers never exceed this in INAV's driver set.
 */
#define I2C_MAX_WRITE_LEN       64U

/* INAV I2CSpeed enum → baud rate (Hz).
 * Enum values are non-sequential; index by enum directly. */
static const uint32_t speedToBaud[] = {
    [I2C_SPEED_400KHZ] = 400000U,
    [I2C_SPEED_800KHZ] = 800000U,
    [I2C_SPEED_100KHZ] = 100000U,
    [I2C_SPEED_200KHZ] = 200000U,
};
#define SPEED_TABLE_COUNT (sizeof(speedToBaud) / sizeof(speedToBaud[0]))

static uint32_t i2cBaudrate  = 400000U;
static uint16_t i2cErrorCount = 0;

typedef struct {
    i2c_inst_t *hw;
    bool        initialised;
} rp2350_i2c_state_t;

static rp2350_i2c_state_t i2cState[I2CDEV_COUNT];

/*
 * Map a GPIO number to i2c0 or i2c1.
 * RP2350 I2C mux pattern: (gpio / 2) is odd → i2c1, even → i2c0.
 *   GP0/1   → i2c0,  GP2/3   → i2c1,  GP4/5   → i2c0,  GP6/7   → i2c1
 *   GP8/9   → i2c0,  GP10/11 → i2c1,  GP12/13 → i2c0,  GP14/15 → i2c1
 *   GP16/17 → i2c0,  GP18/19 → i2c1,  GP20/21 → i2c0,  GP22/23 → i2c1
 */
static i2c_inst_t *gpioToI2cHw(uint gpio)
{
    return ((gpio / 2u) & 1u) ? i2c1 : i2c0;
}

static inline uint ioTagToGpio(ioTag_t tag)
{
    return (uint)(DEFIO_TAG_GPIOID(tag) * 16u + DEFIO_TAG_PIN(tag));
}

/* ─── Public API ──────────────────────────────────────────────────────────── */

void i2cSetSpeed(uint8_t speed)
{
    if ((size_t)speed < SPEED_TABLE_COUNT && speedToBaud[speed] != 0) {
        i2cBaudrate = speedToBaud[speed];
    }
}

void i2cInit(I2CDevice device)
{
    if (device < 0 || device >= I2CDEV_COUNT) {
        return;
    }

    ioTag_t sdaTag, sclTag;

    switch (device) {
#ifdef USE_I2C_DEVICE_1
    case I2CDEV_1:
        sdaTag = IO_TAG(I2C1_SDA);
        sclTag = IO_TAG(I2C1_SCL);
        break;
#endif
#ifdef USE_I2C_DEVICE_2
    case I2CDEV_2:
        sdaTag = IO_TAG(I2C2_SDA);
        sclTag = IO_TAG(I2C2_SCL);
        break;
#endif
    default:
        return;
    }

    const uint sdaGpio = ioTagToGpio(sdaTag);
    const uint sclGpio = ioTagToGpio(sclTag);
    i2c_inst_t *hw     = gpioToI2cHw(sdaGpio);

    /* On-chip pull-ups are safe defaults; external resistors (typically 4.7 kΩ)
     * will dominate if fitted on the sensor board. */
    gpio_set_function(sdaGpio, GPIO_FUNC_I2C);
    gpio_set_function(sclGpio, GPIO_FUNC_I2C);
    gpio_pull_up(sdaGpio);
    gpio_pull_up(sclGpio);

    i2c_init(hw, i2cBaudrate);

    i2cState[device].hw          = hw;
    i2cState[device].initialised = true;
}

bool i2cWriteBuffer(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len_,
                    const uint8_t *data, bool allowRawAccess)
{
    if (device < 0 || device >= I2CDEV_COUNT || !i2cState[device].initialised) {
        return false;
    }

    i2c_inst_t *hw = i2cState[device].hw;
    int ret;

    if (reg_ == 0xFF && allowRawAccess) {
        ret = i2c_write_timeout_us(hw, addr_, data, len_, false, RP2350_I2C_TIMEOUT_US);
    } else {
        if (len_ > I2C_MAX_WRITE_LEN) {
            return false;
        }
        uint8_t buf[I2C_MAX_WRITE_LEN + 1];
        buf[0] = reg_;
        memcpy(&buf[1], data, len_);
        ret = i2c_write_timeout_us(hw, addr_, buf, (size_t)(len_ + 1u),
                                   false, RP2350_I2C_TIMEOUT_US);
    }

    if (ret < 0) {
        i2cErrorCount++;
        return false;
    }
    return true;
}

bool i2cWrite(I2CDevice device, uint8_t addr_, uint8_t reg, uint8_t data,
              bool allowRawAccess)
{
    return i2cWriteBuffer(device, addr_, reg, 1, &data, allowRawAccess);
}

bool i2cRead(I2CDevice device, uint8_t addr_, uint8_t reg, uint8_t len,
             uint8_t *buf, bool allowRawAccess)
{
    if (device < 0 || device >= I2CDEV_COUNT || !i2cState[device].initialised) {
        return false;
    }

    i2c_inst_t *hw = i2cState[device].hw;
    int ret;

    if (reg == 0xFF && allowRawAccess) {
        ret = i2c_read_timeout_us(hw, addr_, buf, len, false, RP2350_I2C_TIMEOUT_US);
    } else {
        ret = i2c_write_timeout_us(hw, addr_, &reg, 1,
                                   true /* nostop — generates repeated start */,
                                   RP2350_I2C_TIMEOUT_US);
        if (ret < 0) {
            i2cErrorCount++;
            return false;
        }
        ret = i2c_read_timeout_us(hw, addr_, buf, len, false, RP2350_I2C_TIMEOUT_US);
    }

    if (ret < 0) {
        i2cErrorCount++;
        return false;
    }
    return true;
}

bool i2cBusy(I2CDevice device, bool *error)
{
    UNUSED(device);
    if (error) {
        *error = false;
    }
    /* Blocking implementation — transfers complete before returning */
    return false;
}

uint16_t i2cGetErrorCounter(void)
{
    return i2cErrorCount;
}

#endif /* USE_I2C */

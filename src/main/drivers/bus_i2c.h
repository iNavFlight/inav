/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

/*
#define I2C_SHORT_TIMEOUT            ((uint32_t)0x1000)
#define I2C_LONG_TIMEOUT             ((uint32_t)(10 * I2C_SHORT_TIMEOUT))
#define I2C_DEFAULT_TIMEOUT          I2C_LONG_TIMEOUT
*/
#define I2C_TIMEOUT                  (10000)

#include "drivers/io_types.h"
#include "drivers/rcc_types.h"

typedef enum {  // Weird mapping to keep config compatible with previos version
    I2C_SPEED_100KHZ    = 2,
    I2C_SPEED_200KHZ    = 3,
    I2C_SPEED_400KHZ    = 0,
    I2C_SPEED_800KHZ    = 1,
} I2CSpeed;

typedef enum I2CDevice {
    I2CINVALID = -1,
    I2CDEV_EMULATED = -1,   // Hack until we have proper I2C abstraction
    I2CDEV_1   = 0,
    I2CDEV_2,
    I2CDEV_3,
#ifdef USE_I2C_DEVICE_4
    I2CDEV_4,
#endif
    I2CDEV_COUNT
} I2CDevice;

typedef struct i2cDevice_s {
#if defined(AT32F43x) 
    i2c_type *dev;
#else
    I2C_TypeDef *dev;
#endif
    ioTag_t scl;
    ioTag_t sda;
    rccPeriphTag_t rcc;
    I2CSpeed speed;
#if defined(STM32F4) || defined(STM32F7) || defined(STM32H7) || defined(AT32F43x)
    uint8_t ev_irq;
    uint8_t er_irq;
    uint8_t af;
#endif
} i2cDevice_t;

void i2cSetSpeed(uint8_t speed);
void i2cInit(I2CDevice device);
bool i2cWriteBuffer(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len_, const uint8_t *data, bool allowRawAccess);
bool i2cWrite(I2CDevice device, uint8_t addr_, uint8_t reg, uint8_t data, bool allowRawAccess);
bool i2cRead(I2CDevice device, uint8_t addr_, uint8_t reg, uint8_t len, uint8_t* buf, bool allowRawAccess);

// Non-blocking transfers: start returns immediately (false if the bus is busy or the transfer could not be started),
// completion and the result are polled with i2cBusy(). Platforms without an asynchronous driver complete the
// transfer synchronously inside the start call and report the bus as idle.
bool i2cReadStart(I2CDevice device, uint8_t addr_, uint8_t reg, uint8_t len, uint8_t* buf, bool allowRawAccess);
bool i2cWriteStart(I2CDevice device, uint8_t addr_, uint8_t reg, uint8_t data, bool allowRawAccess);
bool i2cWriteBufferStart(I2CDevice device, uint8_t addr_, uint8_t reg, uint8_t len, const uint8_t *data, bool allowRawAccess);   // data must stay valid until i2cBusy() reports idle
bool i2cBusy(I2CDevice device, uint8_t addr_, bool *error);   // pending state and outcome of the given slave's transfer

// Outcome of the last completed non-blocking transfer of every 7-bit slave address on one bus. The platform drivers
// keep one per bus so that a device still reads its own result after another device has used the bus in between.
typedef struct {
    volatile uint32_t failed[4];    // one bit per address, set = the last transfer to that address failed
} i2cAddrResults_t;

static inline void i2cAddrResultSet(i2cAddrResults_t *results, uint8_t addr_, bool error)
{
    const uint32_t mask = 1u << (addr_ & 31);
    if (error) {
        results->failed[(addr_ >> 5) & 3] |= mask;
    } else {
        results->failed[(addr_ >> 5) & 3] &= ~mask;
    }
}

static inline bool i2cAddrResultFailed(const i2cAddrResults_t *results, uint8_t addr_)
{
    return (results->failed[(addr_ >> 5) & 3] >> (addr_ & 31)) & 1u;
}

uint16_t i2cGetErrorCounter(void);

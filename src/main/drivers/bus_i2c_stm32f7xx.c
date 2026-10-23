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

// bus_i2c_hal.c's API on the F7 I2C registers: the HAL I2C module costs F722 targets flash they don't have

#include <stdbool.h>
#include <stdint.h>

#include <platform.h>

#include "common/maths.h"

#include "drivers/io.h"
#include "drivers/time.h"

#include "drivers/bus_i2c.h"
#include "drivers/nvic.h"
#include "io_impl.h"
#include "rcc.h"

#if !defined(SOFT_I2C) && defined(USE_I2C)

static void i2cUnstick(IO_t scl, IO_t sda);

#if defined(USE_I2C_PULLUP)
#define IOCFG_I2C IO_CONFIG(GPIO_MODE_AF_OD, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_PULLUP)
#else
#define IOCFG_I2C IOCFG_AF_OD
#endif

#ifndef I2C1_SCL
#define I2C1_SCL PB6
#endif
#ifndef I2C1_SDA
#define I2C1_SDA PB7
#endif
#ifndef I2C2_SCL
#define I2C2_SCL PB10
#endif
#ifndef I2C2_SDA
#define I2C2_SDA PB11
#endif
#ifndef I2C3_SCL
#define I2C3_SCL PA8
#endif
#ifndef I2C3_SDA
#define I2C3_SDA PB4
#endif
#if defined(USE_I2C_DEVICE_4)
#ifndef I2C4_SCL
#define I2C4_SCL PD12
#endif
#ifndef I2C4_SDA
#define I2C4_SDA PD13
#endif
#endif

static i2cDevice_t i2cHardwareMap[I2CDEV_COUNT] = {
    { .dev = I2C1, .scl = IO_TAG(I2C1_SCL), .sda = IO_TAG(I2C1_SDA), .rcc = RCC_APB1(I2C1), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C1_EV_IRQn, .er_irq = I2C1_ER_IRQn, .af = GPIO_AF4_I2C1 },
    { .dev = I2C2, .scl = IO_TAG(I2C2_SCL), .sda = IO_TAG(I2C2_SDA), .rcc = RCC_APB1(I2C2), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C2_EV_IRQn, .er_irq = I2C2_ER_IRQn, .af = GPIO_AF4_I2C2 },
    { .dev = I2C3, .scl = IO_TAG(I2C3_SCL), .sda = IO_TAG(I2C3_SDA), .rcc = RCC_APB1(I2C3), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C3_EV_IRQn, .er_irq = I2C3_ER_IRQn, .af = GPIO_AF4_I2C3 },
#if defined(USE_I2C_DEVICE_4)
    { .dev = I2C4, .scl = IO_TAG(I2C4_SCL), .sda = IO_TAG(I2C4_SDA), .rcc = RCC_APB1(I2C4), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C4_EV_IRQn, .er_irq = I2C4_ER_IRQn, .af = GPIO_AF4_I2C4 }
#endif
};

static volatile uint16_t i2cErrorCount = 0;

#define I2C_ERRORS (I2C_ISR_NACKF | I2C_ISR_BERR | I2C_ISR_ARLO | I2C_ISR_OVR)

static bool i2cInitialised[I2CDEV_COUNT];

void i2cSetSpeed(uint8_t speed)
{
    for (unsigned int i = 0; i < ARRAYLEN(i2cHardwareMap); i++) {
        i2cHardwareMap[i].speed = speed;
    }
}

static bool i2cHandleHardwareFailure(I2CDevice device)
{
    i2cErrorCount++;
    i2cInit(device);
    return false;
}

// False on a NACK, a bus error, or when the whole transfer has taken longer than I2C_TIMEOUT
static bool i2cWaitFor(I2C_TypeDef *i2c, uint32_t flag, timeUs_t start)
{
    while (!(i2c->ISR & flag)) {
        if ((i2c->ISR & I2C_ERRORS) || cmpTimeUs(micros(), start) > I2C_TIMEOUT) {
            return false;
        }
    }
    return true;
}

// One transfer in one direction: AUTOEND sends the stop, otherwise TC holds the bus for a repeated start
static bool i2cTransfer(I2C_TypeDef *i2c, uint8_t addr_, bool read, const uint8_t *out, uint8_t *in, uint8_t len,
                        bool stop, timeUs_t start)
{
    i2c->CR2 = (addr_ << 1) | (read ? I2C_CR2_RD_WRN : 0) | ((uint32_t)len << I2C_CR2_NBYTES_Pos) |
               (stop ? I2C_CR2_AUTOEND : 0) | I2C_CR2_START;
    for (uint8_t i = 0; i < len; i++) {
        if (!i2cWaitFor(i2c, read ? I2C_ISR_RXNE : I2C_ISR_TXIS, start)) {
            return false;
        }
        if (read) {
            in[i] = i2c->RXDR;
        } else {
            i2c->TXDR = out[i];
        }
    }
    // The stop that follows a NACK can be there before the NACK is looked at
    if (!i2cWaitFor(i2c, stop ? I2C_ISR_STOPF : I2C_ISR_TC, start) || (i2c->ISR & I2C_ERRORS)) {
        return false;
    }
    i2c->ICR = I2C_ICR_STOPCF;
    return true;
}

#ifdef USE_I2C_ASYNC
#define I2C_ASYNC_IE (I2C_CR1_TXIE | I2C_CR1_RXIE | I2C_CR1_TCIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_ERRIE)

// A longer gap between looks means the main loop was busy, not the transfer stuck: it counts as this much
#define I2C_ASYNC_LOOK_GAP_US 1000

// A read started by i2cReadAsync(): the interrupts carry it, i2cAsyncUpdate() notices its end
typedef struct {
    volatile bool done;
    volatile bool failed;
    volatile uint8_t left;
    bool busy;
    uint8_t addr;
    uint8_t reg;
    uint8_t len;
    uint8_t seen;
    uint8_t *buf;
    i2cAsyncState_e state;
    uint32_t stuckUs;
    timeUs_t lookUs;
} i2cAsync_t;

static i2cAsync_t i2cAsync[I2CDEV_COUNT];

static void i2cIrq(I2CDevice device)
{
    i2cAsync_t *async = &i2cAsync[device];
    I2C_TypeDef *i2c = i2cHardwareMap[device].dev;
    const uint32_t isr = i2c->ISR;

    if (isr & I2C_ERRORS) {
        i2c->CR1 &= ~I2C_ASYNC_IE;
        async->failed = true;
        async->done = true;
    } else if (isr & I2C_ISR_TXIS) {
        i2c->TXDR = async->reg;
    } else if (isr & I2C_ISR_RXNE) {
        *async->buf++ = i2c->RXDR;
        async->left--;
    } else if (isr & I2C_ISR_TC) {
        // The register went out: a repeated start reads
        i2c->CR2 = (async->addr << 1) | I2C_CR2_RD_WRN | ((uint32_t)async->len << I2C_CR2_NBYTES_Pos) |
                   I2C_CR2_AUTOEND | I2C_CR2_START;
    } else if (isr & I2C_ISR_STOPF) {
        i2c->ICR = I2C_ICR_STOPCF;
        i2c->CR1 &= ~I2C_ASYNC_IE;
        async->done = true;
    }
}

void I2C1_ER_IRQHandler(void) { i2cIrq(I2CDEV_1); }
void I2C1_EV_IRQHandler(void) { i2cIrq(I2CDEV_1); }
void I2C2_ER_IRQHandler(void) { i2cIrq(I2CDEV_2); }
void I2C2_EV_IRQHandler(void) { i2cIrq(I2CDEV_2); }
void I2C3_ER_IRQHandler(void) { i2cIrq(I2CDEV_3); }
void I2C3_EV_IRQHandler(void) { i2cIrq(I2CDEV_3); }
#ifdef USE_I2C_DEVICE_4
void I2C4_ER_IRQHandler(void) { i2cIrq(I2CDEV_4); }
void I2C4_EV_IRQHandler(void) { i2cIrq(I2CDEV_4); }
#endif

static void i2cAsyncUpdate(I2CDevice device)
{
    i2cAsync_t *async = &i2cAsync[device];

    if (!async->busy) {
        return;
    }

    if (async->done) {
        async->busy = false;
        async->state = async->failed ? I2C_ASYNC_FAILED : I2C_ASYNC_OK;
        if (async->failed) {
            // A NACK too, as for a blocking transfer
            i2cHandleHardwareFailure(device);
        }
        return;
    }

    const timeUs_t now = micros();
    if (async->left != async->seen) {
        async->seen = async->left;
        async->stuckUs = 0;
    } else if ((async->stuckUs += MIN(cmpTimeUs(now, async->lookUs), I2C_ASYNC_LOOK_GAP_US)) > I2C_TIMEOUT) {
        i2cHardwareMap[device].dev->CR1 &= ~I2C_ASYNC_IE;
        async->busy = false;
        async->state = I2C_ASYNC_FAILED;
        i2cHandleHardwareFailure(device);
        return;
    }
    async->lookUs = now;
}

bool i2cReadAsync(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len, uint8_t *buf, bool allowRawAccess)
{
    if (device == I2CINVALID || !i2cInitialised[device]) {
        return false;
    }

    i2cAsync_t *async = &i2cAsync[device];
    I2C_TypeDef *i2c = i2cHardwareMap[device].dev;

    i2cAsyncUpdate(device);
    if (async->busy) {
        return false;
    }

    const bool raw = reg_ == 0xFF && allowRawAccess;

    async->addr = addr_;
    // As the HAL refused them: a register read of nothing, or a bus held by someone else
    if ((len == 0 && !raw) || (i2c->ISR & I2C_ISR_BUSY)) {
        async->state = I2C_ASYNC_FAILED;
        return i2cHandleHardwareFailure(device);
    }

    async->reg = reg_;
    async->len = len;
    async->left = async->seen = len;
    async->buf = buf;
    async->done = async->failed = false;
    async->stuckUs = 0;
    async->lookUs = micros();
    async->state = I2C_ASYNC_BUSY;
    async->busy = true;

    // The interrupt reads the fields above: they are stored before it can run
    __asm__ volatile("" ::: "memory");
    i2c->CR1 |= I2C_ASYNC_IE;
    if (raw) {
        i2c->CR2 = (addr_ << 1) | I2C_CR2_RD_WRN | ((uint32_t)len << I2C_CR2_NBYTES_Pos) | I2C_CR2_AUTOEND |
                   I2C_CR2_START;
    } else {
        i2c->CR2 = (addr_ << 1) | (1U << I2C_CR2_NBYTES_Pos) | I2C_CR2_START;
    }
    return true;
}

i2cAsyncState_e i2cAsyncState(I2CDevice device, uint8_t addr_)
{
    if (device == I2CINVALID) {
        return I2C_ASYNC_FAILED;
    }

    i2cAsyncUpdate(device);
    const i2cAsync_t *async = &i2cAsync[device];
    return async->addr == addr_ ? async->state : I2C_ASYNC_IDLE;
}
#endif

static I2C_TypeDef *i2cReady(I2CDevice device)
{
    if (device == I2CINVALID || !i2cInitialised[device]) {
        return NULL;
    }

#ifdef USE_I2C_ASYNC
    // A blocking transfer lets a read started by i2cReadAsync() end first
    while (i2cAsync[device].busy) {
        i2cAsyncUpdate(device);
    }
#endif

    return i2cHardwareMap[device].dev;
}

bool i2cWriteBuffer(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len_, const uint8_t *data, bool allowRawAccess)
{
    I2C_TypeDef *i2c = i2cReady(device);
    if (!i2c) {
        return false;
    }

    const timeUs_t start = micros();
    bool ok;

    if ((reg_ == 0xFF || len_ == 0) && allowRawAccess) {
        ok = i2cTransfer(i2c, addr_, false, data, NULL, len_, true, start);
    } else if (len_ == 0) {
        // HAL_I2C_Mem_Write refused it
        ok = false;
    } else if (len_ == 0xFF) {
        // The register byte would make 256, one more than NBYTES holds
        return false;
    } else {
        i2c->CR2 = (addr_ << 1) | ((uint32_t)(len_ + 1) << I2C_CR2_NBYTES_Pos) | I2C_CR2_AUTOEND | I2C_CR2_START;
        ok = i2cWaitFor(i2c, I2C_ISR_TXIS, start);
        if (ok) {
            i2c->TXDR = reg_;
        }
        for (uint8_t i = 0; ok && i < len_; i++) {
            ok = i2cWaitFor(i2c, I2C_ISR_TXIS, start);
            if (ok) {
                i2c->TXDR = data[i];
            }
        }
        ok = ok && i2cWaitFor(i2c, I2C_ISR_STOPF, start) && !(i2c->ISR & I2C_ERRORS);
        i2c->ICR = I2C_ICR_STOPCF;
    }

    return ok || i2cHandleHardwareFailure(device);
}

bool i2cWrite(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t data, bool allowRawAccess)
{
    return i2cWriteBuffer(device, addr_, reg_, 1, &data, allowRawAccess);
}

bool i2cRead(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len, uint8_t* buf, bool allowRawAccess)
{
    I2C_TypeDef *i2c = i2cReady(device);
    if (!i2c) {
        return false;
    }

    const timeUs_t start = micros();
    const bool raw = reg_ == 0xFF && allowRawAccess;
    // A register read of nothing was refused by HAL_I2C_Mem_Read
    const bool ok = (raw || (len > 0 && i2cTransfer(i2c, addr_, false, &reg_, NULL, 1, false, start))) &&
                    i2cTransfer(i2c, addr_, true, NULL, buf, len, true, start);

    return ok || i2cHandleHardwareFailure(device);
}

// SCLDEL, SDADEL, SCLH and SCLL for TIMINGR: bus_i2c_hal.c's computation with the analog filter on and no digital one
static uint32_t i2cClockTIMINGR(uint32_t pclkFreq, int i2cFreqKhz)
{
    const bool fastPlus = i2cFreqKhz > 400;
    const float trmax = fastPlus ? 120 : 300;
    const float tsuDATmin = fastPlus ? 50 : 100;
    const float tI2cclk = 1000000000.0f / pclkFreq;
    const float tSCL = 1000000.0f / i2cFreqKhz;
    // Rise or fall time 100 ns, analog filter 70 ns, three clocks of synchronisation
    const float tsync = 100 + 70 + 3 * tI2cclk;

    for (int presc = 1; presc < 15; presc++) {
        const float unit = (presc + 1) * tI2cclk;
        const uint32_t scldel = (trmax + tsuDATmin) / unit - 1;
        const uint32_t sdadel = (trmax - 50 - 3 * tI2cclk) / unit;
        const float sclhl = (tSCL - 2 * tsync) / unit - 1;
        const uint32_t sclh = sclhl / 4.75f;
        const uint32_t scll = (uint32_t)(sclhl + 0.5f) - sclh;

        if (scldel < 16 && sdadel < 16 && sclh - 1 < 256 && scll - 1 < 256) {
            return (presc << 28) | (scldel << 20) | (sdadel << 16) | ((sclh - 1) << 8) | (scll - 1);
        }
    }
    return 0;
}

void i2cInit(I2CDevice device)
{
    i2cDevice_t *hardware = &i2cHardwareMap[device];
    I2C_TypeDef *i2c = hardware->dev;

    if (i2c == NULL) {
        return;
    }

    RCC_ClockCmd(hardware->rcc, ENABLE);

    IO_t scl = IOGetByTag(hardware->scl);
    IO_t sda = IOGetByTag(hardware->sda);
    IOInit(scl, OWNER_I2C, RESOURCE_I2C_SCL, RESOURCE_INDEX(device));
    IOInit(sda, OWNER_I2C, RESOURCE_I2C_SDA, RESOURCE_INDEX(device));
    i2cUnstick(scl, sda);
    IOConfigGPIOAF(scl, IOCFG_I2C, hardware->af);
    IOConfigGPIOAF(sda, IOCFG_I2C, hardware->af);

    static const uint16_t khz[] = {
        [I2C_SPEED_100KHZ] = 100, [I2C_SPEED_200KHZ] = 200, [I2C_SPEED_400KHZ] = 400, [I2C_SPEED_800KHZ] = 800
    };

    // PE=0 resets the state machine and the flags; ANFOFF=0 keeps the analog filter on, which TIMINGR assumes
    i2c->CR1 = 0;
    // I2C1-4 run on PCLK1, as target/system_stm32f7xx.c sets them
    const int speedKhz = hardware->speed < ARRAYLEN(khz) ? khz[hardware->speed] : 400;
    i2c->TIMINGR = i2cClockTIMINGR(HAL_RCC_GetPCLK1Freq(), speedKhz);
    i2c->OAR1 = 0;
    i2c->OAR2 = 0;
    i2c->CR1 = I2C_CR1_PE;

#ifdef USE_I2C_ASYNC
    HAL_NVIC_SetPriority(hardware->er_irq, NVIC_PRIO_I2C_ER, 0);
    HAL_NVIC_EnableIRQ(hardware->er_irq);
    HAL_NVIC_SetPriority(hardware->ev_irq, NVIC_PRIO_I2C_EV, 0);
    HAL_NVIC_EnableIRQ(hardware->ev_irq);
#endif

    i2cInitialised[device] = true;
}

uint16_t i2cGetErrorCounter(void)
{
    return i2cErrorCount;
}

static void i2cUnstick(IO_t scl, IO_t sda)
{
    IOHi(scl);
    IOHi(sda);
    IOConfigGPIO(scl, IOCFG_OUT_OD);
    IOConfigGPIO(sda, IOCFG_OUT_OD);

    // Analog Devices AN-686: 9 clock pulses, then a stop
    for (int i = 0; i < 9; i++) {
        int timeout = 100;
        while (!IORead(scl) && timeout) {
            delayMicroseconds(5);
            timeout--;
        }
        IOLo(scl);
        delayMicroseconds(5);
        IOHi(scl);
        delayMicroseconds(5);
    }
    IOLo(scl);
    delayMicroseconds(5);
    IOLo(sda);
    delayMicroseconds(5);
    IOHi(scl);
    delayMicroseconds(5);
    IOHi(sda);
}

#endif

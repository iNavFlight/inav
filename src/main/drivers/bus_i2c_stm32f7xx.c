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
 *
 * Interrupt driven I2C master for STM32F7 on the peripheral registers. The
 * ST HAL I2C module costs the F722 targets several KB of flash they do not
 * have, and the F7 I2C peripheral (RM0385 / RM0431, chapter "I2C") is simple
 * enough to drive directly: one CR2 write starts a transfer of up to 255
 * bytes and the hardware sends the STOP itself, at the end (AUTOEND) or
 * after a NACK.
 *
 * Same structure as bus_i2c_stm32f40x.c: a transfer is started from task
 * context and completed by the I2Cx_EV and I2Cx_ER interrupts. i2cRead() /
 * i2cWrite() block until the transfer is done, i2cReadStart() /
 * i2cWriteStart() return immediately and the caller polls i2cBusy(). A
 * blocking call issued while a non-blocking transfer is in progress waits
 * for it to finish first, so the two can share a bus.
 *
 * The handler never waits on the bus or reinitialises the peripheral. A
 * transfer that stops making progress (a slave stretching SCL, a START that
 * cannot go out because a slave holds SDA) is caught by the progress timeout
 * in i2cBusy() and the blocking waits, from task context. A bus error or an
 * arbitration loss flags the bus for a reset that the next i2cBusy() or
 * start call performs from task context.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <platform.h>

#include "build/debug.h"

#include "common/utils.h"
#include "common/maths.h"

#include "drivers/io.h"
#include "drivers/time.h"

#include "drivers/bus_i2c.h"
#include "drivers/nvic.h"
#include "io_impl.h"
#include "rcc.h"

#if !defined(SOFT_I2C) && defined(USE_I2C)

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

// Interrupts the handler needs during a transfer, enabled in CR1 per transfer. ERRIE covers BERR, ARLO and OVR
#define I2C_IRQ_ENABLES     (I2C_CR1_TXIE | I2C_CR1_RXIE | I2C_CR1_TCIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_ERRIE)
#define I2C_ISR_BUS_ERRORS  (I2C_ISR_BERR | I2C_ISR_ARLO | I2C_ISR_OVR)

typedef struct i2cBusState_s {
    bool            initialized;
    volatile bool   busy;           // transfer in progress
    volatile bool   error;          // last transfer failed (NACK, bus error, arbitration lost, overrun or timeout)
    volatile bool   needsReset;     // the handler gave up on a transfer, i2cServiceReset() has to reinitialise the peripheral
    uint16_t        lastProgress;   // byte index and register phase at the previous look, a change means the transfer moves
    uint32_t        stuckUs;        // time looked at without progress, a long gap between looks counts as I2C_STUCK_LOOK_GAP_US
    timeUs_t        lookUs;         // time of the previous look

    // Active transfer
    uint8_t         addr;           // 7-bit address
    uint8_t         reg;
    uint8_t         bytes;          // data bytes, the register byte not included
    uint8_t *       readBuf;
    const uint8_t * writeBuf;
    uint8_t         txByte;         // storage for the data of a non-blocking single byte write

    // Handler state
    volatile bool   regSent;        // register byte phase done (or none, for raw access)
    volatile uint8_t index;         // next data byte
} i2cBusState_t;

static volatile uint16_t i2cErrorCount = 0;

// Transfer statistics shown with debug_mode = I2C, shared by all buses
typedef struct {
    timeUs_t    startUs;        // when the current transfer was started
    uint16_t    eventIrqs;      // interrupts during the current / last transfer
    uint16_t    errorIrqs;      // interrupts that found an error flag, since boot
    uint16_t    lastTransferUs; // start to completion of the last transfer
    uint16_t    maxIrqUs;       // longest single handler run
    uint16_t    lastErrorIsr;   // ISR flags found by the handler at the last error: NACKF 16, BERR 256, ARLO 512, OVR 1024
    uint16_t    startCallUs;    // time spent in the last i2cStartTransfer() call
} i2cDebugStats_t;

static i2cDebugStats_t i2cStats;

static void i2cDebugPublish(void)
{
    DEBUG_SET(DEBUG_I2C, 0, i2cStats.eventIrqs);
    DEBUG_SET(DEBUG_I2C, 1, i2cStats.errorIrqs);
    DEBUG_SET(DEBUG_I2C, 2, i2cStats.lastTransferUs);
    DEBUG_SET(DEBUG_I2C, 3, i2cStats.maxIrqUs);
    DEBUG_SET(DEBUG_I2C, 5, i2cErrorCount);
    DEBUG_SET(DEBUG_I2C, 6, i2cStats.lastErrorIsr);
    DEBUG_SET(DEBUG_I2C, 7, i2cStats.startCallUs);
}

static i2cDevice_t i2cHardwareMap[] = {
    { .dev = I2C1, .scl = IO_TAG(I2C1_SCL), .sda = IO_TAG(I2C1_SDA), .rcc = RCC_APB1(I2C1), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C1_EV_IRQn, .er_irq = I2C1_ER_IRQn, .af = GPIO_AF4_I2C1 },
    { .dev = I2C2, .scl = IO_TAG(I2C2_SCL), .sda = IO_TAG(I2C2_SDA), .rcc = RCC_APB1(I2C2), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C2_EV_IRQn, .er_irq = I2C2_ER_IRQn, .af = GPIO_AF4_I2C2 },
    { .dev = I2C3, .scl = IO_TAG(I2C3_SCL), .sda = IO_TAG(I2C3_SDA), .rcc = RCC_APB1(I2C3), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C3_EV_IRQn, .er_irq = I2C3_ER_IRQn, .af = GPIO_AF4_I2C3 },
#if defined(USE_I2C_DEVICE_4)
    { .dev = I2C4, .scl = IO_TAG(I2C4_SCL), .sda = IO_TAG(I2C4_SDA), .rcc = RCC_APB1(I2C4), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C4_EV_IRQn, .er_irq = I2C4_ER_IRQn, .af = GPIO_AF4_I2C4 },
#endif
};

static i2cBusState_t busState[ARRAYLEN(i2cHardwareMap)];

// Per address outcome of the last transfer. Kept outside busState so that the reset in i2cInit() does not wipe the
// results of the other devices on the bus
static i2cAddrResults_t busAddrResults[ARRAYLEN(i2cHardwareMap)];

static void i2cUnstick(IO_t scl, IO_t sda);
static void i2cIrqHandler(I2CDevice device);

static bool i2cIsValidDevice(I2CDevice device)
{
    return device != I2CINVALID && (unsigned)device < ARRAYLEN(i2cHardwareMap);
}

// The event and the error vector share one handler: all flags are in ISR and one pass handles whatever is set
void I2C1_ER_IRQHandler(void)
{
    i2cIrqHandler(I2CDEV_1);
}

void I2C1_EV_IRQHandler(void)
{
    i2cIrqHandler(I2CDEV_1);
}

void I2C2_ER_IRQHandler(void)
{
    i2cIrqHandler(I2CDEV_2);
}

void I2C2_EV_IRQHandler(void)
{
    i2cIrqHandler(I2CDEV_2);
}

void I2C3_ER_IRQHandler(void)
{
    i2cIrqHandler(I2CDEV_3);
}

void I2C3_EV_IRQHandler(void)
{
    i2cIrqHandler(I2CDEV_3);
}

#if defined(USE_I2C_DEVICE_4)
void I2C4_ER_IRQHandler(void)
{
    i2cIrqHandler(I2CDEV_4);
}

void I2C4_EV_IRQHandler(void)
{
    i2cIrqHandler(I2CDEV_4);
}
#endif

// Reinitialise the peripheral and clock the bus free. Always returns false so it can be used as the failure result.
static bool i2cHandleHardwareFailure(I2CDevice device)
{
    i2cErrorCount++;
    i2cInit(device);
    return false;
}

// Reinitialise the peripheral after the handler gave up on a transfer. Runs from task context before the bus is looked
// at or used again, so the unstick never holds off the higher priority interrupts
static void i2cServiceReset(I2CDevice device)
{
    if (busState[device].needsReset) {
        i2cInit(device);    // the failure was counted by the handler, i2cInit() clears needsReset with the rest of the state
    }
}

void i2cSetSpeed(uint8_t speed)
{
    for (unsigned int i = 0; i < ARRAYLEN(i2cHardwareMap); i++) {
        i2cHardwareMap[i].speed = speed;
    }
}

// PRESC, SCLDEL, SDADEL, SCLH and SCLL for TIMINGR (RM0385 "I2C master mode" timing): the computation of bus_i2c_hal.c
// with the analog filter on, no digital filter and the rise and fall times estimated at 100 ns
static uint32_t i2cClockTIMINGR(uint32_t pclkFreq, int i2cFreqKhz)
{
    const bool fastModePlus = i2cFreqKhz > 400;
    const float trMax = fastModePlus ? 120 : 300;       // ns, rise / fall time maximum of the I2C specification
    const float tsuDatMin = fastModePlus ? 50 : 100;    // ns, SDA setup time minimum
    const float tI2cclk = 1000000000.0f / pclkFreq;     // ns
    const float tScl = 1000000.0f / i2cFreqKhz;         // ns
    // Rise or fall time 100 ns, analog filter delay 70 ns, three I2CCLK cycles of synchronisation on each SCL edge
    const float tSync = 100 + 70 + 3 * tI2cclk;

    for (int presc = 1; presc < 15; presc++) {
        const float tPresc = (presc + 1) * tI2cclk;
        const uint32_t scldel = (trMax + tsuDatMin) / tPresc - 1;
        const uint32_t sdadel = (trMax - 50 - 3 * tI2cclk) / tPresc;       // fall time - analog filter minimum - 3 I2CCLK
        const float sclhl = (tScl - 2 * tSync) / tPresc - 1;
        const uint32_t sclh = sclhl / 4.75f;                                // STM32CubeMX seems to use a value like this
        const uint32_t scll = (uint32_t)(sclhl + 0.5f) - sclh;

        // All fields have to fit, otherwise increase the prescaler and try again
        if (scldel < 16 && sdadel < 16 && sclh - 1 < 256 && scll - 1 < 256) {
            return (presc << 28) | (scldel << 20) | (sdadel << 16) | ((sclh - 1) << 8) | (scll - 1);
        }
    }
    return 0;   // not reached for the supported clocks
}

void i2cInit(I2CDevice device)
{
    if (!i2cIsValidDevice(device)) {
        return;
    }

    i2cDevice_t *i2c = &(i2cHardwareMap[device]);
    i2cBusState_t *state = &busState[device];
    I2C_TypeDef *I2Cx = i2c->dev;

    RCC_ClockCmd(i2c->rcc, ENABLE);

    // No handler run in the middle of this: one could put PE back while the lines are clocked by hand below
    HAL_NVIC_DisableIRQ(i2c->er_irq);
    HAL_NVIC_DisableIRQ(i2c->ev_irq);

    // PE=0 is the software reset (RM0385 "Software reset"): it drops the transfer in flight, clears the flags and BUSY
    // and takes the peripheral off the lines while the unstick clocks them. It has to stay low for at least three APB
    // clocks, the pin setup below takes far longer. The interrupt enables go with it
    I2Cx->CR1 = 0;
    HAL_NVIC_ClearPendingIRQ(i2c->er_irq);
    HAL_NVIC_ClearPendingIRQ(i2c->ev_irq);

    IO_t scl = IOGetByTag(i2c->scl);
    IO_t sda = IOGetByTag(i2c->sda);

    IOInit(scl, OWNER_I2C, RESOURCE_I2C_SCL, RESOURCE_INDEX(device));
    IOInit(sda, OWNER_I2C, RESOURCE_I2C_SDA, RESOURCE_INDEX(device));

    // Release a slave that may still be driving SDA after a reset in the middle of a transfer
    i2cUnstick(scl, sda);

    IOConfigGPIOAF(scl, IOCFG_I2C, i2c->af);
    IOConfigGPIOAF(sda, IOCFG_I2C, i2c->af);

    int speedKhz;
    switch (i2c->speed) {
        case I2C_SPEED_400KHZ:
        default:
            speedKhz = 400;
            break;

        case I2C_SPEED_800KHZ:
            speedKhz = 800;
            break;

        case I2C_SPEED_100KHZ:
            speedKhz = 100;
            break;

        case I2C_SPEED_200KHZ:
            speedKhz = 200;
            break;
    }

    // I2C1-4 are clocked from PCLK1 (target/system_stm32f7xx.c). The analog filter is on and the digital one off, the
    // CR1 reset values TIMINGR assumes. OAR1 and OAR2 keep their reset value: no own address, the peripheral never
    // answers as a slave
    I2Cx->TIMINGR = i2cClockTIMINGR(HAL_RCC_GetPCLK1Freq(), speedKhz);
    I2Cx->CR1 = I2C_CR1_PE;

    // The interrupts are enabled in the peripheral per transfer, the NVIC side stays on
    HAL_NVIC_SetPriority(i2c->er_irq, NVIC_PRIO_I2C_ER, 0);
    HAL_NVIC_EnableIRQ(i2c->er_irq);
    HAL_NVIC_SetPriority(i2c->ev_irq, NVIC_PRIO_I2C_EV, 0);
    HAL_NVIC_EnableIRQ(i2c->ev_irq);

    memset(state, 0, sizeof(*state));
    state->initialized = true;
}

uint16_t i2cGetErrorCounter(void)
{
    return i2cErrorCount;
}

// A transfer is stuck when it made no progress for I2C_TIMEOUT while being looked at. A gap between two looks longer
// than I2C_STUCK_LOOK_GAP_US counts as that much only: a main loop that was busy elsewhere for a while (a settings save)
// does not make the transfer stuck, the interrupts carry it on however seldom somebody looks
#define I2C_STUCK_LOOK_GAP_US   1000

static uint16_t i2cTransferProgress(const i2cBusState_t *state)
{
    // The data byte index and the register phase are all the handler moves between START and STOP
    return state->index | (state->regSent ? 0x100 : 0);
}

static bool i2cTransferStuck(i2cBusState_t *state)
{
    const timeUs_t now = micros();
    const uint16_t progress = i2cTransferProgress(state);

    if (progress != state->lastProgress) {
        state->lastProgress = progress;
        state->stuckUs = 0;
    }
    else {
        state->stuckUs += MIN((uint32_t)cmpTimeUs(now, state->lookUs), (uint32_t)I2C_STUCK_LOOK_GAP_US);
    }
    state->lookUs = now;

    return state->stuckUs >= I2C_TIMEOUT;
}

// Set up and start a transfer. Returns false if the bus is busy or the transfer cannot be done.
static bool i2cStartTransfer(I2CDevice device, uint8_t addr, uint8_t reg, bool allowRawAccess, bool reading, uint8_t len, uint8_t *readBuf, const uint8_t *writeBuf)
{
    if (!i2cIsValidDevice(device)) {
        return false;
    }

    i2cBusState_t *state = &busState[device];
    I2C_TypeDef *I2Cx = i2cHardwareMap[device].dev;

    i2cServiceReset(device);

    if (!state->initialized || state->busy) {
        return false;
    }

    const bool rawAccess = (reg == 0xFF) && allowRawAccess;

    if (len == 0 && (reading || rawAccess)) {
        return false;   // nothing to receive or send, only the register byte alone is a transfer of no data
    }
    if (len == 255 && !reading && !rawAccess) {
        return false;   // the register byte would make 256, one more than NBYTES holds
    }

    const timeUs_t callStartUs = micros();
    i2cStats.eventIrqs = 0;
    i2cStats.startUs = callStartUs;

    state->addr = addr;
    state->reg = reg;
    state->bytes = len;
    state->readBuf = readBuf;
    state->writeBuf = writeBuf;
    state->regSent = rawAccess;
    state->index = 0;
    state->lastProgress = i2cTransferProgress(state);
    state->stuckUs = 0;
    state->lookUs = callStartUs;
    state->error = false;
    state->busy = true;

    // One CR2 write starts the transfer: address, direction, length and the STOP at its end (AUTOEND). A register read
    // first sends the register byte alone without AUTOEND, the handler issues the repeated START at TC. The START goes
    // out once the bus is free, a bus that never frees shows up as a stuck transfer
    uint32_t cr2 = ((uint32_t)addr << 1) | I2C_CR2_START;
    if (reading && !rawAccess) {
        cr2 |= 1U << I2C_CR2_NBYTES_Pos;
    }
    else {
        cr2 |= ((uint32_t)(rawAccess ? len : len + 1) << I2C_CR2_NBYTES_Pos) | I2C_CR2_AUTOEND | (reading ? I2C_CR2_RD_WRN : 0);
    }

    __DMB();                                // the handler reads the state above, it is stored before the start goes out
    I2Cx->CR1 |= I2C_IRQ_ENABLES;
    I2Cx->CR2 = cr2;

    i2cStats.startCallUs = micros() - callStartUs;
    i2cDebugPublish();

    return true;
}

// Reset the peripheral after a transfer that did not complete and record the failure for the transfer's owner
static void i2cRecoverStuckTransfer(I2CDevice device)
{
    const uint8_t stuckAddr = busState[device].addr;        // i2cInit() clears the state

    i2cHandleHardwareFailure(device);
    busState[device].error = true;
    i2cAddrResultSet(&busAddrResults[device], stuckAddr, true);
}

// Block until the bus is idle. Returns false if the transfer in progress had to be aborted.
static bool i2cWaitForIdle(I2CDevice device)
{
    i2cBusState_t *state = &busState[device];

    while (state->busy) {
        if (i2cTransferStuck(state)) {
            i2cRecoverStuckTransfer(device);
            return false;
        }
    }

    return true;
}

bool i2cBusy(I2CDevice device, uint8_t addr_, bool *error)
{
    if (!i2cIsValidDevice(device)) {
        if (error) {
            *error = true;
        }
        return false;
    }

    i2cBusState_t *state = &busState[device];

    i2cServiceReset(device);

    if (state->busy && i2cTransferStuck(state)) {
        // No progress within the timeout, the transfer is stuck - reset the peripheral
        i2cRecoverStuckTransfer(device);
    }

    // Only the owner of the transfer on the bus is pending, everybody else gets the outcome of their own last transfer
    const bool pending = state->busy && state->addr == addr_;

    if (error) {
        *error = !pending && i2cAddrResultFailed(&busAddrResults[device], addr_);
    }

    i2cDebugPublish();

    return pending;
}

bool i2cReadStart(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len, uint8_t* buf, bool allowRawAccess)
{
    return i2cStartTransfer(device, addr_, reg_, allowRawAccess, true, len, buf, NULL);
}

bool i2cWriteBufferStart(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len_, const uint8_t *data, bool allowRawAccess)
{
    return i2cStartTransfer(device, addr_, reg_, allowRawAccess, false, len_, NULL, data);
}

bool i2cWriteStart(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t data, bool allowRawAccess)
{
    if (!i2cIsValidDevice(device) || busState[device].busy) {
        return false;   // don't touch txByte while it may still be going out
    }

    busState[device].txByte = data;
    return i2cStartTransfer(device, addr_, reg_, allowRawAccess, false, 1, NULL, &busState[device].txByte);
}

bool i2cWriteBuffer(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len_, const uint8_t *data, bool allowRawAccess)
{
    if (!i2cIsValidDevice(device) || !busState[device].initialized) {
        return false;
    }

    // Let a non-blocking transfer of another device finish first
    if (!i2cWaitForIdle(device)) {
        return false;
    }

    if (!i2cStartTransfer(device, addr_, reg_, allowRawAccess, false, len_, NULL, data)) {
        return false;
    }

    return i2cWaitForIdle(device) && !busState[device].error;
}

bool i2cWrite(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t data, bool allowRawAccess)
{
    return i2cWriteBuffer(device, addr_, reg_, 1, &data, allowRawAccess);
}

bool i2cRead(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len, uint8_t* buf, bool allowRawAccess)
{
    if (!i2cIsValidDevice(device) || !busState[device].initialized) {
        return false;
    }

    // Let a non-blocking transfer of another device finish first
    if (!i2cWaitForIdle(device)) {
        return false;
    }

    if (!i2cStartTransfer(device, addr_, reg_, allowRawAccess, true, len, buf, NULL)) {
        return false;
    }

    return i2cWaitForIdle(device) && !busState[device].error;
}

// The transfer is over and the bus idle: hand the outcome to the owner and silence the peripheral until the next start
static void i2cTransferDone(I2CDevice device, bool error)
{
    i2cBusState_t *state = &busState[device];

    i2cHardwareMap[device].dev->CR1 &= ~I2C_IRQ_ENABLES;
    i2cStats.lastTransferUs = microsISR() - i2cStats.startUs;
    if (error) {
        i2cErrorCount++;        // keep the "I2C Errors" count of status / MSP meaningful for non-blocking transfers
    }
    state->error = error;
    i2cAddrResultSet(&busAddrResults[device], state->addr, error);
    state->busy = false;
}

static void i2cIrqHandlerBody(I2CDevice device)
{
    I2C_TypeDef *I2Cx = i2cHardwareMap[device].dev;
    i2cBusState_t *state = &busState[device];
    const uint32_t isr = I2Cx->ISR;

    if (isr & I2C_ISR_BUS_ERRORS) {
        // A misplaced START or STOP, SDA taken away from under the master (ARLO) or an overrun leaves the peripheral
        // anywhere in the transfer: give up on it and let i2cServiceReset() reinitialise the peripheral and clock the
        // bus free from task context, the unstick takes too long for here
        I2Cx->ICR = I2C_ICR_BERRCF | I2C_ICR_ARLOCF | I2C_ICR_OVRCF | I2C_ICR_NACKCF | I2C_ICR_STOPCF;
        i2cStats.errorIrqs++;
        i2cStats.lastErrorIsr = isr;
        state->needsReset = true;
        i2cTransferDone(device, true);
        return;
    }

    if (isr & I2C_ISR_NACKF) {
        // The slave did not answer. The hardware sends the STOP itself, the transfer ends at STOPF below, usually in
        // this same run
        I2Cx->ICR = I2C_ICR_NACKCF;
        i2cStats.errorIrqs++;
        i2cStats.lastErrorIsr = isr;
        state->error = true;
    }

    if (isr & I2C_ISR_TXIS) {                                   // the hardware wants the next byte to send
        if (state->regSent) {
            I2Cx->TXDR = state->writeBuf[state->index++];
        }
        else {
            I2Cx->TXDR = state->reg;
            state->regSent = true;
        }
    }
    else if (isr & I2C_ISR_RXNE) {                              // a byte came in
        state->readBuf[state->index++] = (uint8_t)I2Cx->RXDR;
    }
    else if (isr & I2C_ISR_TC) {                                // the register byte of a read is out: repeated START, read the data
        I2Cx->CR2 = ((uint32_t)state->addr << 1) | I2C_CR2_RD_WRN | ((uint32_t)state->bytes << I2C_CR2_NBYTES_Pos) | I2C_CR2_AUTOEND | I2C_CR2_START;
    }

    if (isr & I2C_ISR_STOPF) {                                  // the STOP is out, after the last byte or after a NACK
        I2Cx->ICR = I2C_ICR_STOPCF;
        i2cTransferDone(device, state->error);
    }
}

static void i2cIrqHandler(I2CDevice device)
{
    const timeUs_t entryUs = microsISR();

    i2cStats.eventIrqs++;
    i2cIrqHandlerBody(device);

    const timeUs_t spentUs = microsISR() - entryUs;
    if (spentUs > i2cStats.maxIrqUs) {
        i2cStats.maxIrqUs = spentUs;
    }
}

static void i2cUnstick(IO_t scl, IO_t sda)
{
    int i;

    IOHi(scl);
    IOHi(sda);

    IOConfigGPIO(scl, IOCFG_OUT_OD);
    IOConfigGPIO(sda, IOCFG_OUT_OD);

    // Analog Devices AN-686
    // We need 9 clock pulses + STOP condition
    for (i = 0; i < 9; i++) {
        // Wait for any clock stretching to finish
        int timeout = 100;
        while (!IORead(scl) && timeout) {
            delayMicroseconds(5);
            timeout--;
        }

        // Pull low
        IOLo(scl); // Set bus low
        delayMicroseconds(5);
        IOHi(scl); // Set bus high
        delayMicroseconds(5);
    }

    // Generate a stop condition in case there was none
    IOLo(scl);
    delayMicroseconds(5);
    IOLo(sda);
    delayMicroseconds(5);

    IOHi(scl); // Set bus scl high
    delayMicroseconds(5);
    IOHi(sda); // Set bus sda high
}

#endif

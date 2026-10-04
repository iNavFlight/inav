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
 * Interrupt driven I2C master for STM32F4. The transfer engine (event and
 * error handlers) is derived from the Betaflight bus_i2c_stm32f4xx.c driver.
 *
 * A transfer is started from task context and completed by the I2Cx_EV and
 * I2Cx_ER interrupts. i2cRead()/i2cWrite() block until the transfer is done,
 * i2cReadStart()/i2cWriteStart() return immediately and the caller polls
 * i2cBusy(). A blocking call issued while a non-blocking transfer is in
 * progress waits for it to finish first, so the two can share a bus.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <platform.h>

#include "build/debug.h"

#include "common/utils.h"

#include "drivers/io.h"
#include "drivers/time.h"

#include "drivers/bus_i2c.h"
#include "drivers/nvic.h"
#include "io_impl.h"
#include "rcc.h"

#ifndef SOFT_I2C

// Upper bound for the short waits on START/STOP bit release inside the handlers.
// Normally these clear within one SCL period; a longer wait means a stuck bus.
#define I2C_BIT_RELEASE_TIMEOUT_US  1000

#define GPIO_AF_I2C GPIO_AF_I2C1

#if defined(USE_I2C_PULLUP)
#define IOCFG_I2C IO_CONFIG(GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_OD, GPIO_PuPd_UP)
#else
#define IOCFG_I2C IOCFG_AF_OD
#endif

#ifndef I2C1_SCL
#define I2C1_SCL PB8
#endif
#ifndef I2C1_SDA
#define I2C1_SDA PB9
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

typedef struct i2cBusState_s {
    bool            initialized;
    volatile bool   busy;           // transfer in progress
    volatile bool   error;          // last transfer failed (NACK, bus error, arbitration lost, overrun or timeout)
    timeUs_t        startUs;        // start of the current transfer, used to detect one that never completes

    // Active transfer
    bool            reading;
    bool            rawAccess;      // no register address phase
    uint8_t         addr;           // 7-bit address, pre-shifted
    uint8_t         reg;
    int16_t         bytes;
    uint8_t *       readBuf;
    const uint8_t * writeBuf;
    uint8_t         txByte;         // storage for the data of a non-blocking single byte write

    // Event handler state
    bool            subaddressSent;
    int16_t         index;          // next byte index, -1 = register address goes out next
} i2cBusState_t;

static volatile uint16_t i2cErrorCount = 0;

// Transfer statistics shown with debug_mode = I2C, shared by all buses
typedef struct {
    timeUs_t    startUs;        // when the current transfer was started
    uint16_t    eventIrqs;      // event interrupts during the current / last transfer
    uint16_t    errorIrqs;      // error interrupts since boot
    uint16_t    lastTransferUs; // start to completion of the last transfer
    uint16_t    maxIrqUs;       // longest single event handler run
    uint16_t    maxBitWaitUs;   // longest wait for a START / STOP bit release
    uint16_t    startCr2;       // interrupt enables found in CR2 when a transfer was started, should be 0
    uint16_t    startCallUs;    // time spent in the last i2cStartTransfer() call
} i2cDebugStats_t;

static i2cDebugStats_t i2cStats;

static void i2cDebugPublish(void)
{
    DEBUG_SET(DEBUG_I2C, 0, i2cStats.eventIrqs);
    DEBUG_SET(DEBUG_I2C, 1, i2cStats.errorIrqs);
    DEBUG_SET(DEBUG_I2C, 2, i2cStats.lastTransferUs);
    DEBUG_SET(DEBUG_I2C, 3, i2cStats.maxIrqUs);
    DEBUG_SET(DEBUG_I2C, 4, i2cStats.maxBitWaitUs);
    DEBUG_SET(DEBUG_I2C, 5, i2cErrorCount);
    DEBUG_SET(DEBUG_I2C, 6, i2cStats.startCr2);
    DEBUG_SET(DEBUG_I2C, 7, i2cStats.startCallUs);
}

static i2cDevice_t i2cHardwareMap[] = {
    { .dev = I2C1, .scl = IO_TAG(I2C1_SCL), .sda = IO_TAG(I2C1_SDA), .rcc = RCC_APB1(I2C1), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C1_EV_IRQn, .er_irq = I2C1_ER_IRQn },
    { .dev = I2C2, .scl = IO_TAG(I2C2_SCL), .sda = IO_TAG(I2C2_SDA), .rcc = RCC_APB1(I2C2), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C2_EV_IRQn, .er_irq = I2C2_ER_IRQn },
    { .dev = I2C3, .scl = IO_TAG(I2C3_SCL), .sda = IO_TAG(I2C3_SDA), .rcc = RCC_APB1(I2C3), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C3_EV_IRQn, .er_irq = I2C3_ER_IRQn },
};

static i2cBusState_t busState[ARRAYLEN(i2cHardwareMap)];

static void i2cUnstick(IO_t scl, IO_t sda);
static void i2cEventHandler(I2CDevice device);
static void i2cErrorHandler(I2CDevice device);

static bool i2cIsValidDevice(I2CDevice device)
{
    return device != I2CINVALID && (unsigned)device < ARRAYLEN(i2cHardwareMap);
}

void I2C1_ER_IRQHandler(void)
{
    i2cErrorHandler(I2CDEV_1);
}

void I2C1_EV_IRQHandler(void)
{
    i2cEventHandler(I2CDEV_1);
}

void I2C2_ER_IRQHandler(void)
{
    i2cErrorHandler(I2CDEV_2);
}

void I2C2_EV_IRQHandler(void)
{
    i2cEventHandler(I2CDEV_2);
}

void I2C3_ER_IRQHandler(void)
{
    i2cErrorHandler(I2CDEV_3);
}

void I2C3_EV_IRQHandler(void)
{
    i2cEventHandler(I2CDEV_3);
}

// Reinitialise the peripheral and clock the bus free. Always returns false so it can be used as the failure result.
static bool i2cHandleHardwareFailure(I2CDevice device)
{
    i2cErrorCount++;
    i2cInit(device);
    return false;
}

// Wait for the hardware to release a CR1 control bit (START / STOP). Safe to call from the handlers.
static bool i2cWaitBitRelease(I2C_TypeDef *I2Cx, uint32_t cr1Mask)
{
    const timeUs_t startUs = microsISR();

    while (I2Cx->CR1 & cr1Mask) {
        if (cmpTimeUs(microsISR(), startUs) >= I2C_BIT_RELEASE_TIMEOUT_US) {
            return false;
        }
    }

    const timeUs_t waitedUs = microsISR() - startUs;
    if (waitedUs > i2cStats.maxBitWaitUs) {
        i2cStats.maxBitWaitUs = waitedUs;
    }

    return true;
}

void i2cSetSpeed(uint8_t speed)
{
    for (unsigned int i = 0; i < ARRAYLEN(i2cHardwareMap); i++) {
        i2cHardwareMap[i].speed = speed;
    }
}

void i2cInit(I2CDevice device)
{
    if (!i2cIsValidDevice(device)) {
        return;
    }

    i2cDevice_t *i2c = &(i2cHardwareMap[device]);
    i2cBusState_t *state = &busState[device];

    IO_t scl = IOGetByTag(i2c->scl);
    IO_t sda = IOGetByTag(i2c->sda);

    RCC_ClockCmd(i2c->rcc, ENABLE);

    I2C_ITConfig(i2c->dev, I2C_IT_EVT | I2C_IT_ERR | I2C_IT_BUF, DISABLE);

    IOInit(scl, OWNER_I2C, RESOURCE_I2C_SCL, RESOURCE_INDEX(device));
    IOInit(sda, OWNER_I2C, RESOURCE_I2C_SDA, RESOURCE_INDEX(device));

    // Release a slave that may still be driving SDA after a reset in the middle of a transfer
    i2cUnstick(scl, sda);

    IOConfigGPIOAF(scl, IOCFG_I2C, GPIO_AF_I2C);
    IOConfigGPIOAF(sda, IOCFG_I2C, GPIO_AF_I2C);

    I2C_DeInit(i2c->dev);

    I2C_InitTypeDef i2cInit;
    I2C_StructInit(&i2cInit);

    i2cInit.I2C_Mode = I2C_Mode_I2C;
    i2cInit.I2C_DutyCycle = I2C_DutyCycle_2;
    i2cInit.I2C_OwnAddress1 = 0x00;
    i2cInit.I2C_Ack = I2C_Ack_Enable;
    i2cInit.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    switch (i2c->speed) {
        case I2C_SPEED_400KHZ:
        default:
            i2cInit.I2C_ClockSpeed = 400000;
            break;

        case I2C_SPEED_800KHZ:
            i2cInit.I2C_ClockSpeed = 800000;
            break;

        case I2C_SPEED_100KHZ:
            i2cInit.I2C_ClockSpeed = 100000;
            break;

        case I2C_SPEED_200KHZ:
            i2cInit.I2C_ClockSpeed = 200000;
            break;
    }

    I2C_Init(i2c->dev, &i2cInit);
    I2C_StretchClockCmd(i2c->dev, ENABLE);
    I2C_Cmd(i2c->dev, ENABLE);

    // Event and error interrupts are enabled in the peripheral per transfer, the NVIC side stays on
    NVIC_SetPriority((IRQn_Type)i2c->er_irq, NVIC_PRIO_I2C_ER);
    NVIC_EnableIRQ((IRQn_Type)i2c->er_irq);
    NVIC_SetPriority((IRQn_Type)i2c->ev_irq, NVIC_PRIO_I2C_EV);
    NVIC_EnableIRQ((IRQn_Type)i2c->ev_irq);

    memset(state, 0, sizeof(*state));
    state->initialized = true;
}

uint16_t i2cGetErrorCounter(void)
{
    return i2cErrorCount;
}

// Set up and start a transfer. Returns false if the bus is busy or the peripheral failed to start.
static bool i2cStartTransfer(I2CDevice device, uint8_t addr, uint8_t reg, bool allowRawAccess, bool reading, uint8_t len, uint8_t *readBuf, const uint8_t *writeBuf)
{
    if (!i2cIsValidDevice(device)) {
        return false;
    }

    i2cBusState_t *state = &busState[device];
    I2C_TypeDef *I2Cx = i2cHardwareMap[device].dev;

    if (!state->initialized || state->busy) {
        return false;
    }

    if (reading && len == 0) {
        return false;   // nothing to receive, the engine would run past the buffer
    }

    const timeUs_t callStartUs = micros();
    i2cStats.startCr2 = I2Cx->CR2 & (I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR);
    i2cStats.eventIrqs = 0;
    i2cStats.startUs = callStartUs;

    state->addr = addr << 1;
    state->reg = reg;
    state->rawAccess = (reg == 0xFF) && allowRawAccess;
    state->reading = reading;
    state->bytes = len;
    state->readBuf = readBuf;
    state->writeBuf = writeBuf;
    state->subaddressSent = false;
    state->index = 0;
    state->startUs = callStartUs;
    state->error = false;
    state->busy = true;

    if (!(I2Cx->CR2 & I2C_IT_EVT)) {                                    // interrupts are off between transfers
        if (!(I2Cx->CR1 & I2C_CR1_START)) {                             // ensure sending a start
            const timeUs_t startUs = micros();
            while (I2Cx->CR1 & I2C_CR1_STOP) {                          // wait for any stop to finish sending
                if (cmpTimeUs(micros(), startUs) >= I2C_TIMEOUT) {
                    return i2cHandleHardwareFailure(device);
                }
            }
            I2C_GenerateSTART(I2Cx, ENABLE);                            // send the start for the new job
        }
        I2C_ITConfig(I2Cx, I2C_IT_EVT | I2C_IT_ERR, ENABLE);            // the handlers take it from here
    }

    i2cStats.startCallUs = micros() - callStartUs;
    i2cDebugPublish();

    return true;
}

// Block until the bus is idle. Returns false if the transfer in progress had to be aborted.
static bool i2cWaitForIdle(I2CDevice device)
{
    i2cBusState_t *state = &busState[device];
    const timeUs_t startUs = micros();

    while (state->busy) {
        if (cmpTimeUs(micros(), startUs) >= I2C_TIMEOUT) {
            return i2cHandleHardwareFailure(device);
        }
    }

    return true;
}

bool i2cBusy(I2CDevice device, bool *error)
{
    if (!i2cIsValidDevice(device)) {
        if (error) {
            *error = true;
        }
        return false;
    }

    i2cBusState_t *state = &busState[device];

    if (state->busy && cmpTimeUs(micros(), state->startUs) >= I2C_TIMEOUT) {
        // No completion interrupt within the timeout, the transfer is stuck - reset the peripheral
        i2cHandleHardwareFailure(device);
        state->error = true;
    }

    if (error) {
        *error = state->error;
    }

    i2cDebugPublish();

    return state->busy;
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

static void i2cErrorHandler(I2CDevice device)
{
    I2C_TypeDef *I2Cx = i2cHardwareMap[device].dev;
    i2cBusState_t *state = &busState[device];

    const uint32_t SR1Register = I2Cx->SR1;

    i2cStats.errorIrqs++;

    if (SR1Register & (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR)) {
        state->error = true;
    }

    // If AF, BERR or ARLO, abandon the current job
    if (SR1Register & (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF)) {
        (void)I2Cx->SR2;                                                        // read second status register to clear ADDR if it is set (note that BTF will not be set after a NACK)
        I2C_ITConfig(I2Cx, I2C_IT_BUF, DISABLE);                                // disable the RXNE/TXE interrupt - prevent the ISR tailchaining onto the ER (hopefully)
        if (!(SR1Register & I2C_SR1_ARLO) && !(I2Cx->CR1 & I2C_CR1_STOP)) {     // if we dont have an ARLO error, ensure sending of a stop
            if (I2Cx->CR1 & I2C_CR1_START) {                                    // We are currently trying to send a start, this is very bad as start, stop will hang the peripheral
                i2cWaitBitRelease(I2Cx, I2C_CR1_START);                         // wait for any start to finish sending
                I2C_GenerateSTOP(I2Cx, ENABLE);                                 // send stop to finalise bus transaction
                i2cWaitBitRelease(I2Cx, I2C_CR1_STOP);                          // wait for stop to finish sending
                i2cHandleHardwareFailure(device);                               // reset and configure the hardware
                state->error = true;                                            // the reset cleared the state, the transfer still failed
            }
            else {
                I2C_GenerateSTOP(I2Cx, ENABLE);                                 // stop to free up the bus
                I2C_ITConfig(I2Cx, I2C_IT_EVT | I2C_IT_ERR, DISABLE);           // Disable EVT and ERR interrupts while bus inactive
            }
        }
    }

    I2Cx->SR1 &= ~(I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR);     // reset all the error bits to clear the interrupt
    state->busy = false;
}

static void i2cEventHandlerBody(I2CDevice device)
{
    I2C_TypeDef *I2Cx = i2cHardwareMap[device].dev;
    i2cBusState_t *state = &busState[device];

    // Only the low byte carries the event flags, the error flags in the high byte are handled by the error handler
    const uint8_t SReg_1 = I2Cx->SR1;

    if (SReg_1 & I2C_SR1_SB) {                                                  // we just sent a start - EV5 in ref manual
        I2Cx->CR1 &= ~I2C_CR1_POS;                                              // reset the POS bit so ACK/NACK applied to the current byte
        I2C_AcknowledgeConfig(I2Cx, ENABLE);                                    // make sure ACK is on
        state->index = 0;                                                       // reset the index
        if (state->reading && (state->subaddressSent || state->rawAccess)) {   // we have sent the subaddr
            state->subaddressSent = true;                                       // make sure this is set in case of no subaddress, so following code runs correctly
            if (state->bytes == 2) {
                I2Cx->CR1 |= I2C_CR1_POS;                                       // set the POS bit so NACK applied to the final byte in the two byte read
            }
            I2C_Send7bitAddress(I2Cx, state->addr, I2C_Direction_Receiver);     // send the address and set hardware mode
        }
        else {                                                                  // direction is Tx, or we havent sent the sub and rep start
            I2C_Send7bitAddress(I2Cx, state->addr, I2C_Direction_Transmitter);  // send the address and set hardware mode
            if (!state->rawAccess) {
                state->index = -1;                                              // send a subaddress
            }
        }
    }
    else if (SReg_1 & I2C_SR1_ADDR) {                                           // we just sent the address - EV6 in ref manual
        // Read SR1,2 to clear ADDR
        __DMB();                                                                // memory fence to control hardware
        if (state->bytes == 1 && state->reading && state->subaddressSent) {     // we are receiving 1 byte - EV6_3
            I2C_AcknowledgeConfig(I2Cx, DISABLE);                               // turn off ACK
            __DMB();
            (void)I2Cx->SR2;                                                    // clear ADDR after ACK is turned off
            I2C_GenerateSTOP(I2Cx, ENABLE);                                     // program the stop
            I2C_ITConfig(I2Cx, I2C_IT_BUF, ENABLE);                             // allow us to have an EV7
        }
        else {                                                                  // EV6 and EV6_1
            (void)I2Cx->SR2;                                                    // clear the ADDR here
            __DMB();
            if (state->bytes == 2 && state->reading && state->subaddressSent) { // rx 2 bytes - EV6_1
                I2C_AcknowledgeConfig(I2Cx, DISABLE);                           // turn off ACK
                I2C_ITConfig(I2Cx, I2C_IT_BUF, DISABLE);                        // disable TXE to allow the buffer to fill
            }
            else if (state->bytes == 3 && state->reading && state->subaddressSent) {    // rx 3 bytes
                I2C_ITConfig(I2Cx, I2C_IT_BUF, DISABLE);                        // make sure RXNE disabled so we get a BTF in two bytes time
            }
            else {                                                              // receiving greater than three bytes, sending subaddress, or transmitting
                I2C_ITConfig(I2Cx, I2C_IT_BUF, ENABLE);
            }
        }
    }
    else if (SReg_1 & I2C_SR1_BTF) {                                            // Byte transfer finished - EV7_2, EV7_3 or EV8_2
        if (state->reading && state->subaddressSent) {                          // EV7_2, EV7_3
            if (state->bytes > 2) {                                             // EV7_2
                I2C_AcknowledgeConfig(I2Cx, DISABLE);                           // turn off ACK
                state->readBuf[state->index++] = (uint8_t)I2Cx->DR;             // read data N-2
                I2C_GenerateSTOP(I2Cx, ENABLE);                                 // program the Stop
                state->readBuf[state->index++] = (uint8_t)I2Cx->DR;             // read data N-1
                I2C_ITConfig(I2Cx, I2C_IT_BUF, ENABLE);                         // enable TXE to allow the final EV7
            }
            else {                                                              // EV7_3
                I2C_GenerateSTOP(I2Cx, ENABLE);                                 // program the Stop
                state->readBuf[state->index++] = (uint8_t)I2Cx->DR;             // read data N-1
                state->readBuf[state->index++] = (uint8_t)I2Cx->DR;             // read data N
                state->index++;                                                 // to show job completed
            }
        }
        else {                                                                  // EV8_2, which may be due to a subaddress sent or a write completion
            if (state->subaddressSent || !state->reading) {
                I2C_GenerateSTOP(I2Cx, ENABLE);                                 // program the Stop
                state->index++;                                                 // to show that the job is complete
            }
            else {                                                              // We need to send a subaddress
                I2C_GenerateSTART(I2Cx, ENABLE);                                // program the repeated Start
                state->subaddressSent = true;                                   // this is set back to zero upon completion of the current task
            }
        }
        // we must wait for the start to clear, otherwise we get constant BTF
        if (!i2cWaitBitRelease(I2Cx, I2C_CR1_START)) {
            i2cHandleHardwareFailure(device);
            state->error = true;
            return;
        }
    }
    else if (SReg_1 & I2C_SR1_RXNE) {                                           // Byte received - EV7
        state->readBuf[state->index++] = (uint8_t)I2Cx->DR;
        if (state->bytes == (state->index + 3)) {
            I2C_ITConfig(I2Cx, I2C_IT_BUF, DISABLE);                            // disable TXE to allow the buffer to flush so we can get an EV7_2
        }
        if (state->bytes == state->index) {                                     // We have completed a final EV7
            state->index++;                                                     // to show job is complete
        }
    }
    else if (SReg_1 & I2C_SR1_TXE) {                                            // Byte transmitted EV8 / EV8_1
        if (state->index != -1) {                                               // we dont have a subaddress to send
            if (state->index < state->bytes) {
                I2Cx->DR = state->writeBuf[state->index++];
                if (state->bytes == state->index) {                             // we have sent all the data
                    I2C_ITConfig(I2Cx, I2C_IT_BUF, DISABLE);                    // disable TXE to allow the buffer to flush
                }
            }
            else {                                                              // raw write without data - nothing to send, end the transfer
                I2C_ITConfig(I2Cx, I2C_IT_BUF, DISABLE);
                I2C_GenerateSTOP(I2Cx, ENABLE);
                state->index = state->bytes + 1;                                // to show that the job is complete
            }
        }
        else {
            state->index++;
            I2Cx->DR = state->reg;                                              // send the subaddress
            if (state->reading || !(state->bytes)) {                            // if receiving or sending 0 bytes, flush now
                I2C_ITConfig(I2Cx, I2C_IT_BUF, DISABLE);                        // disable TXE to allow the buffer to flush
            }
        }
    }

    if (state->index == state->bytes + 1) {                                     // we have completed the current job
        state->subaddressSent = false;                                          // reset this here
        I2C_ITConfig(I2Cx, I2C_IT_EVT | I2C_IT_ERR, DISABLE);                   // bus is inactive, disable interrupts to prevent BTF
        i2cStats.lastTransferUs = microsISR() - i2cStats.startUs;
        state->busy = false;
    }
}

static void i2cEventHandler(I2CDevice device)
{
    const timeUs_t entryUs = microsISR();

    i2cStats.eventIrqs++;
    i2cEventHandlerBody(device);

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

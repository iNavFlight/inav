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
 * RP2350 I2C driver — Pico SDK hardware_i2c.
 *
 * Blocking transfers (i2cRead / i2cWrite) use the Pico SDK blocking API.
 * Non-blocking transfers (i2cReadStart / i2cWriteStart) queue the commands into the controller FIFO and are
 * completed from the I2C interrupt (STOP_DET / TX_ABRT), following the Betaflight RP2350 driver. Reads longer
 * than the FIFO are refilled in batches on RX_FULL. A blocking call waits for a pending non-blocking one.
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

#include "build/debug.h"

#include "drivers/bus_i2c.h"
#include "drivers/io.h"
#include "drivers/io_def.h"
#include "drivers/time.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"

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

// Interrupts used by a non-blocking transfer
#define I2C_XFER_INTR   (I2C_IC_INTR_STAT_R_STOP_DET_BITS | I2C_IC_INTR_STAT_R_TX_ABRT_BITS | \
                         I2C_IC_INTR_STAT_R_TX_OVER_BITS | I2C_IC_INTR_STAT_R_RX_OVER_BITS)
#define I2C_FIFO_DEPTH  16U

typedef enum {
    I2C_XFER_IDLE = 0,
    I2C_XFER_ACTIVE,        // all commands are in the FIFO, waiting for STOP_DET
    I2C_XFER_READ_BATCHES,  // long read, more read commands go in as the RX FIFO fills
} i2cXferState_e;

typedef struct {
    i2c_inst_t *hw;
    bool        initialised;
    bool        irqInstalled;

    // Non-blocking transfer context, shared with the interrupt handler
    volatile i2cXferState_e xferState;
    volatile bool error;        // last non-blocking transfer failed
    bool        read;
    uint8_t    *data;
    uint8_t     len;
    volatile uint8_t transferred;
    uint8_t     addr;           // 7-bit address of the current / last non-blocking transfer
    timeUs_t    startUs;        // start time, used to detect a transfer that never completes
    uint8_t     txByte;         // data of a non-blocking single byte write, must outlive the call
    i2cAddrResults_t addrResults;   // outcome per slave address, survives i2cInit()
} rp2350_i2c_state_t;

static rp2350_i2c_state_t i2cState[I2CDEV_COUNT];

// Which INAV bus sits on i2c0 / i2c1, for the interrupt handlers
static I2CDevice i2cDeviceForInstance[2] = { I2CINVALID, I2CINVALID };

// Transfer statistics shown with debug_mode = I2C, shared by all buses
typedef struct {
    uint16_t    eventIrqs;      // interrupts during the current / last transfer
    uint16_t    errorIrqs;      // error interrupts / aborted transfers since boot
    uint16_t    lastTransferUs; // start to completion of the last non-blocking transfer
    uint16_t    startCallUs;    // time spent in the last i2cStartTransfer() call
} i2cDebugStats_t;

static i2cDebugStats_t i2cStats;

static void i2cDebugPublish(void)
{
    DEBUG_SET(DEBUG_I2C, 0, i2cStats.eventIrqs);
    DEBUG_SET(DEBUG_I2C, 1, i2cStats.errorIrqs);
    DEBUG_SET(DEBUG_I2C, 2, i2cStats.lastTransferUs);
    DEBUG_SET(DEBUG_I2C, 5, i2cErrorCount);
    DEBUG_SET(DEBUG_I2C, 7, i2cStats.startCallUs);
}

static void i2cQueueReadCommands(i2c_hw_t *hw, uint8_t count, bool restartFirst, bool stopLast)
{
    for (uint8_t i = 0; i < count; i++) {
        uint32_t cmd = I2C_IC_DATA_CMD_CMD_BITS;
        if (i == 0 && restartFirst) {
            cmd |= I2C_IC_DATA_CMD_RESTART_BITS;
        }
        if (i == count - 1 && stopLast) {
            cmd |= I2C_IC_DATA_CMD_STOP_BITS;
        }
        hw->data_cmd = cmd;
    }
}

static void i2cDrainRxFifo(rp2350_i2c_state_t *state, i2c_hw_t *hw)
{
    while (hw->rxflr > 0 && state->transferred < state->len) {
        state->data[state->transferred++] = (uint8_t)(hw->data_cmd & I2C_IC_DATA_CMD_DAT_BITS);
    }
}

static void i2cFinishTransfer(rp2350_i2c_state_t *state, i2c_hw_t *hw, bool error)
{
    hw->intr_mask = 0;
    state->error = error;
    if (error) {
        i2cErrorCount++;
        i2cStats.errorIrqs++;
    }
    i2cStats.lastTransferUs = micros() - state->startUs;
    i2cAddrResultSet(&state->addrResults, state->addr, error);
    state->xferState = I2C_XFER_IDLE;
}

static void i2cIrqHandler(I2CDevice device)
{
    if (device == I2CINVALID) {
        return;
    }

    rp2350_i2c_state_t *state = &i2cState[device];
    i2c_hw_t *hw = i2c_get_hw(state->hw);
    const uint32_t intrStat = hw->intr_stat;

    i2cStats.eventIrqs++;

    if (intrStat & I2C_IC_INTR_STAT_R_TX_ABRT_BITS) {
        (void)hw->clr_tx_abrt;              // NACK or arbitration loss, the controller flushed the FIFO
        i2cFinishTransfer(state, hw, true);
        return;
    }

    if (intrStat & I2C_IC_INTR_STAT_R_TX_OVER_BITS) {
        (void)hw->clr_tx_over;
        i2cFinishTransfer(state, hw, true);
        return;
    }

    if (intrStat & I2C_IC_INTR_STAT_R_RX_OVER_BITS) {
        (void)hw->clr_rx_over;
        i2cFinishTransfer(state, hw, true);
        return;
    }

    if (intrStat & I2C_IC_INTR_STAT_R_STOP_DET_BITS) {
        (void)hw->clr_stop_det;
        if (state->read) {
            i2cDrainRxFifo(state, hw);
        }
        i2cFinishTransfer(state, hw, state->read && state->transferred < state->len);
        return;
    }

    if ((intrStat & I2C_IC_INTR_STAT_R_RX_FULL_BITS) && state->xferState == I2C_XFER_READ_BATCHES) {
        i2cDrainRxFifo(state, hw);
        const uint8_t remaining = state->len - state->transferred;
        if (remaining > 0) {
            const bool finalBatch = remaining <= I2C_FIFO_DEPTH;
            i2cQueueReadCommands(hw, finalBatch ? remaining : I2C_FIFO_DEPTH, false, finalBatch);
            if (finalBatch) {
                hw->intr_mask = I2C_XFER_INTR;      // no more refills, just wait for the STOP
            }
        }
    }
}

static void i2c0IrqHandler(void)
{
    i2cIrqHandler(i2cDeviceForInstance[0]);
}

static void i2c1IrqHandler(void)
{
    i2cIrqHandler(i2cDeviceForInstance[1]);
}

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

    i2c_hw_t *regs = i2c_get_hw(hw);
    regs->intr_mask = 0;                    // interrupts are enabled per non-blocking transfer
    regs->rx_tl = I2C_FIFO_DEPTH - 2;       // RX_FULL fires with 15 bytes queued, in time to refill a long read

    const uint instance = i2c_hw_index(hw);
    i2cDeviceForInstance[instance] = device;

    if (!i2cState[device].irqInstalled) {
        const uint irq = (instance == 0) ? I2C0_IRQ : I2C1_IRQ;
        irq_set_exclusive_handler(irq, (instance == 0) ? i2c0IrqHandler : i2c1IrqHandler);
        irq_set_enabled(irq, true);
        i2cState[device].irqInstalled = true;
    }

    i2cState[device].xferState   = I2C_XFER_IDLE;
    i2cState[device].error       = false;
    i2cState[device].hw          = hw;
    i2cState[device].initialised = true;
}

// Reset a transfer that did not complete and record the failure for its owner. Returns true while a transfer is on the bus.
static bool i2cPollTransfer(I2CDevice device)
{
    rp2350_i2c_state_t *state = &i2cState[device];

    if (state->xferState != I2C_XFER_IDLE && cmpTimeUs(micros(), state->startUs) >= I2C_TIMEOUT) {
        // No STOP_DET within the timeout, the transfer is stuck - reset the controller
        i2c_get_hw(state->hw)->intr_mask = 0;
        state->xferState = I2C_XFER_IDLE;
        i2cErrorCount++;
        i2cInit(device);
        state->error = true;
        i2cAddrResultSet(&state->addrResults, state->addr, true);
    }

    return state->xferState != I2C_XFER_IDLE;
}

// Block until no non-blocking transfer is in progress
static void i2cWaitForIdle(I2CDevice device)
{
    while (i2cPollTransfer(device)) {
    }
}

/*
 * Queue a transfer into the controller FIFO and let the interrupt finish it.
 * Returns false if the bus is busy or the transfer does not fit.
 */
static bool i2cStartTransfer(I2CDevice device, uint8_t addr_, uint8_t reg_, bool allowRawAccess, bool reading, uint8_t len, uint8_t *buf)
{
    if (device < 0 || device >= I2CDEV_COUNT || !i2cState[device].initialised) {
        return false;
    }

    rp2350_i2c_state_t *state = &i2cState[device];
    i2c_hw_t *hw = i2c_get_hw(state->hw);
    const bool useRegister = !(reg_ == 0xFF && allowRawAccess);

    if (reading && len == 0) {
        return false;
    }

    if (!reading && len + (useRegister ? 1U : 0U) > I2C_FIFO_DEPTH) {
        return false;   // writes are not refilled, they must fit the FIFO
    }

    if (i2cPollTransfer(device) || (hw->status & I2C_IC_STATUS_ACTIVITY_BITS)) {
        return false;   // previous transfer still on the bus
    }

    const timeUs_t callStartUs = micros();
    i2cStats.eventIrqs = 0;

    state->read = reading;
    state->data = buf;
    state->len = len;
    state->transferred = 0;
    state->error = false;
    state->addr = addr_;
    state->startUs = callStartUs;

    hw->enable = 0;
    hw->tar = addr_;
    hw->enable = 1;

    // Drop flags left behind by blocking SDK transfers, they would end this transfer prematurely
    (void)hw->clr_intr;
    (void)hw->clr_tx_abrt;

    if (useRegister) {
        hw->data_cmd = (uint32_t)reg_ | ((!reading && len == 0) ? I2C_IC_DATA_CMD_STOP_BITS : 0);
    }

    if (reading) {
        // The register byte takes one FIFO slot, the first batch of read commands fills the rest
        const uint8_t firstBatch = (len <= I2C_FIFO_DEPTH - 1) ? len : (I2C_FIFO_DEPTH - 1);
        const bool singleBatch = (firstBatch == len);
        state->xferState = singleBatch ? I2C_XFER_ACTIVE : I2C_XFER_READ_BATCHES;
        i2cQueueReadCommands(hw, firstBatch, useRegister, singleBatch);
        hw->intr_mask = I2C_XFER_INTR | (singleBatch ? 0 : I2C_IC_INTR_STAT_R_RX_FULL_BITS);
    }
    else {
        state->xferState = I2C_XFER_ACTIVE;
        for (uint8_t i = 0; i < len; i++) {
            hw->data_cmd = (uint32_t)buf[i] | ((i == len - 1) ? I2C_IC_DATA_CMD_STOP_BITS : 0);
        }
        hw->intr_mask = I2C_XFER_INTR;
    }

    i2cStats.startCallUs = micros() - callStartUs;
    i2cDebugPublish();

    return true;
}

bool i2cReadStart(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len, uint8_t* buf, bool allowRawAccess)
{
    return i2cStartTransfer(device, addr_, reg_, allowRawAccess, true, len, buf);
}

bool i2cWriteBufferStart(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len_, const uint8_t *data, bool allowRawAccess)
{
    // the platform API takes a non-const buffer, the data is only read
    return i2cStartTransfer(device, addr_, reg_, allowRawAccess, false, len_, (uint8_t *)data);
}

bool i2cWriteStart(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t data, bool allowRawAccess)
{
    if (device < 0 || device >= I2CDEV_COUNT || i2cPollTransfer(device)) {
        return false;   // don't touch txByte while it may still be going out
    }

    i2cState[device].txByte = data;
    return i2cStartTransfer(device, addr_, reg_, allowRawAccess, false, 1, &i2cState[device].txByte);
}

bool i2cWriteBuffer(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len_,
                    const uint8_t *data, bool allowRawAccess)
{
    if (device < 0 || device >= I2CDEV_COUNT || !i2cState[device].initialised) {
        return false;
    }

    // Let a non-blocking transfer of another device finish first
    i2cWaitForIdle(device);

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

    // Let a non-blocking transfer of another device finish first
    i2cWaitForIdle(device);

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

bool i2cBusy(I2CDevice device, uint8_t addr_, bool *error)
{
    if (device < 0 || device >= I2CDEV_COUNT) {
        if (error) {
            *error = true;
        }
        return false;
    }

    rp2350_i2c_state_t *state = &i2cState[device];

    // Only the owner of the transfer on the bus is pending, everybody else gets the outcome of their own last transfer
    const bool pending = i2cPollTransfer(device) && state->addr == addr_;

    if (error) {
        *error = !pending && i2cAddrResultFailed(&state->addrResults, addr_);
    }

    i2cDebugPublish();

    return pending;
}

uint16_t i2cGetErrorCounter(void)
{
    return i2cErrorCount;
}

#endif /* USE_I2C */

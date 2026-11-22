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
 * TDK InvenSense ICP-201xx (ICP-20100) barometer driver.
 *
 * Based on the ArduPilot driver (AP_Baro_ICP201XX, GPLv3) and the
 * TDK InvenSense reference driver. The sensor runs in continuous mode
 * (operation mode 1: 120 Hz ODR, 30 Hz bandwidth) and its FIFO is
 * read out and averaged by the INAV barometer task.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <platform.h>

#if defined(USE_BARO) && defined(USE_BARO_ICP201XX)

#include "common/utils.h"

#include "drivers/bus.h"
#include "drivers/time.h"
#include "drivers/barometer/barometer.h"
#include "drivers/barometer/barometer_icp201xx.h"

#define ICP201XX_ID                 0x63

#define REG_EMPTY                   0x00
#define REG_TRIM1_MSB               0x05
#define REG_TRIM2_LSB               0x06
#define REG_TRIM2_MSB               0x07
#define REG_DEVICE_ID               0x0C
#define REG_OTP_MTP_OTP_CFG1        0xAC
#define REG_OTP_MTP_MR_LSB          0xAD
#define REG_OTP_MTP_MR_MSB          0xAE
#define REG_OTP_MTP_MRA_LSB         0xAF
#define REG_OTP_MTP_MRA_MSB         0xB0
#define REG_OTP_MTP_MRB_LSB         0xB1
#define REG_OTP_MTP_MRB_MSB         0xB2
#define REG_OTP_MTP_OTP_ADDR        0xB5
#define REG_OTP_MTP_OTP_CMD         0xB6
#define REG_OTP_MTP_RD_DATA         0xB8
#define REG_OTP_MTP_OTP_STATUS      0xB9
#define REG_OTP_DEBUG2              0xBC
#define REG_MASTER_LOCK             0xBE
#define REG_OTP_MTP_OTP_STATUS2     0xBF
#define REG_MODE_SELECT             0xC0
#define REG_INTERRUPT_MASK          0xC2
#define REG_FIFO_CONFIG             0xC3
#define REG_FIFO_FILL               0xC4
#define REG_DEVICE_STATUS           0xCD
#define REG_VERSION                 0xD3
#define REG_FIFO_BASE               0xFA

#define VERSION_A1                  0x00
#define VERSION_B2                  0xB2

#define FIFO_FILL_MASK              0x1F
#define FIFO_FLUSH                  0x80
#define FIFO_MAX_PACKETS            16
#define FIFO_PACKET_SIZE            6       // 3 bytes pressure + 3 bytes temperature

// MODE_SELECT: op mode 1 (120 Hz ODR, 30 Hz BW), continuous, normal power, FIFO pressure+temperature
#define MODE_STANDBY                0x00
#define MODE_POWER_ACTIVE           0x04
#define MODE_CONTINUOUS_120HZ       ((1 << 5) | (1 << 3))

#define FIR_SETTLING_SAMPLES        14
#define TIMEOUT_MS                  50

typedef struct {
    int32_t pressure;       // Pa
    int32_t temperature;    // centi-degrees C
} icp201xxState_t;

static icp201xxState_t icpState;

/*
 * The vendor reference driver performs a dummy read of register 0x00
 * after every transaction. ArduPilot does the same - keep it.
 */
static void dummyRead(busDevice_t *busDev)
{
    uint8_t val;
    busRead(busDev, REG_EMPTY, &val);
}

static bool readReg(busDevice_t *busDev, uint8_t reg, uint8_t *buf, uint8_t len)
{
    const bool ack = busReadBuf(busDev, reg, buf, len);
    dummyRead(busDev);
    return ack;
}

static bool readReg8(busDevice_t *busDev, uint8_t reg, uint8_t *val)
{
    return readReg(busDev, reg, val, 1);
}

static bool writeReg(busDevice_t *busDev, uint8_t reg, uint8_t val)
{
    const bool ack = busWrite(busDev, reg, val);
    dummyRead(busDev);
    return ack;
}

static bool modeSelect(busDevice_t *busDev, uint8_t mode)
{
    // MODE_SELECT may only be written while the mode sync bit is set
    const timeMs_t start = millis();
    uint8_t status = 0;

    do {
        if (readReg8(busDev, REG_DEVICE_STATUS, &status) && (status & 0x01)) {
            return writeReg(busDev, REG_MODE_SELECT, mode);
        }
        delay(1);
    } while (millis() - start < TIMEOUT_MS);

    return false;
}

static bool flushFifo(busDevice_t *busDev)
{
    uint8_t val;

    if (!readReg8(busDev, REG_FIFO_FILL, &val)) {
        return false;
    }

    return writeReg(busDev, REG_FIFO_FILL, val | FIFO_FLUSH);
}

static bool readOtp(busDevice_t *busDev, uint8_t addr, uint8_t *val)
{
    if (!writeReg(busDev, REG_OTP_MTP_OTP_ADDR, addr) || !writeReg(busDev, REG_OTP_MTP_OTP_CMD, 0x10)) {
        return false;
    }

    const timeMs_t start = millis();
    uint8_t status = 0xFF;

    do {
        if (readReg8(busDev, REG_OTP_MTP_OTP_STATUS, &status) && status == 0) {
            return readReg8(busDev, REG_OTP_MTP_RD_DATA, val);
        }
        delayMicroseconds(10);
    } while (millis() - start < TIMEOUT_MS);

    return false;
}

static bool updateBits(busDevice_t *busDev, uint8_t reg, uint8_t mask, uint8_t value)
{
    uint8_t val;

    if (!readReg8(busDev, reg, &val)) {
        return false;
    }

    return writeReg(busDev, reg, (val & ~mask) | (value & mask));
}

static void softReset(busDevice_t *busDev)
{
    modeSelect(busDev, MODE_STANDBY);
    delay(2);
    flushFifo(busDev);
    writeReg(busDev, REG_FIFO_CONFIG, 0x00);
    writeReg(busDev, REG_INTERRUPT_MASK, 0xFF);
}

/*
 * A1 silicon needs the trim values copied from OTP into the main
 * registers after every power-up. B2 silicon does that by itself.
 */
static bool bootSequence(busDevice_t *busDev)
{
    uint8_t version;
    uint8_t bootStatus;

    if (!readReg8(busDev, REG_VERSION, &version)) {
        return false;
    }

    if (version == VERSION_B2) {
        return true;
    }

    if (!readReg8(busDev, REG_OTP_MTP_OTP_STATUS2, &bootStatus)) {
        return false;
    }

    if (bootStatus & 0x01) {
        return true;    // already done since power-up
    }

    // Power mode on to activate the OTP power domain, unlock main registers
    modeSelect(busDev, MODE_POWER_ACTIVE);
    delay(4);
    writeReg(busDev, REG_MASTER_LOCK, 0x1F);

    // Enable OTP and write switch, toggle OTP reset
    updateBits(busDev, REG_OTP_MTP_OTP_CFG1, 0x03, 0x03);
    delayMicroseconds(10);
    updateBits(busDev, REG_OTP_DEBUG2, 0x80, 0x80);
    delayMicroseconds(10);
    updateBits(busDev, REG_OTP_DEBUG2, 0x80, 0x00);
    delayMicroseconds(10);

    // Program redundant read
    writeReg(busDev, REG_OTP_MTP_MRA_LSB, 0x04);
    writeReg(busDev, REG_OTP_MTP_MRA_MSB, 0x04);
    writeReg(busDev, REG_OTP_MTP_MRB_LSB, 0x21);
    writeReg(busDev, REG_OTP_MTP_MRB_MSB, 0x20);
    writeReg(busDev, REG_OTP_MTP_MR_LSB, 0x10);
    writeReg(busDev, REG_OTP_MTP_MR_MSB, 0x80);

    uint8_t offset = 0, gain = 0, hfosc = 0;
    bool ok = readOtp(busDev, 0xF8, &offset) &&
              readOtp(busDev, 0xF9, &gain) &&
              readOtp(busDev, 0xFA, &hfosc);
    delayMicroseconds(10);

    // Copy trim values into the main registers
    ok = ok && updateBits(busDev, REG_TRIM1_MSB, 0x3F, offset);
    ok = ok && updateBits(busDev, REG_TRIM2_MSB, 0x70, (uint8_t)((gain & 0x07) << 4));
    ok = ok && updateBits(busDev, REG_TRIM2_LSB, 0x7F, hfosc);
    delayMicroseconds(10);

    // Remember that the boot sequence has been run
    if (ok) {
        ok = updateBits(busDev, REG_OTP_MTP_OTP_STATUS2, 0x01, 0x01);
    }

    // Disable OTP and write switch, lock main registers, back to standby
    updateBits(busDev, REG_OTP_MTP_OTP_CFG1, 0x03, 0x00);
    writeReg(busDev, REG_MASTER_LOCK, 0x00);
    modeSelect(busDev, MODE_STANDBY);

    return ok;
}

static uint8_t fifoFill(busDevice_t *busDev)
{
    uint8_t val = 0;
    readReg8(busDev, REG_FIFO_FILL, &val);
    return val & FIFO_FILL_MASK;
}

static bool waitFifo(busDevice_t *busDev, uint8_t packets, timeMs_t timeoutMs)
{
    const timeMs_t start = millis();

    while (fifoFill(busDev) < packets) {
        if (millis() - start > timeoutMs) {
            return false;
        }
        delay(5);
    }

    return true;
}

static int32_t signExtend20(const uint8_t *p)
{
    int32_t v = ((int32_t)(p[2] & 0x0F) << 16) | ((int32_t)p[1] << 8) | p[0];

    if (v & 0x080000) {
        v |= (int32_t)0xFFF00000;
    }

    return v;
}

static bool deviceReadMeasurement(baroDev_t *baro)
{
    uint8_t packets = fifoFill(baro->busDev);

    if (packets == 0) {
        return false;
    }

    if (packets > FIFO_MAX_PACKETS) {
        flushFifo(baro->busDev);
        return false;
    }

    uint8_t buf[FIFO_MAX_PACKETS * FIFO_PACKET_SIZE];

    if (!readReg(baro->busDev, REG_FIFO_BASE, buf, packets * FIFO_PACKET_SIZE)) {
        return false;
    }

    int64_t pressSum = 0;
    int64_t tempSum = 0;

    for (int i = 0; i < packets; i++) {
        pressSum += signExtend20(&buf[i * FIFO_PACKET_SIZE]);
        tempSum += signExtend20(&buf[i * FIFO_PACKET_SIZE + 3]);
    }

    // P[Pa] = POUT / 2^17 * 40000 + 70000
    const float pressRaw = (float)pressSum / packets;
    icpState.pressure = (int32_t)(pressRaw * 40000.0f / 131072.0f + 70000.0f);

    // T[cdeg] = (TOUT / 2^18 * 65 + 25) * 100
    const float tempRaw = (float)tempSum / packets;
    icpState.temperature = (int32_t)((tempRaw * 65.0f / 262144.0f + 25.0f) * 100.0f);

    return true;
}

static bool deviceCalculate(baroDev_t *baro, int32_t *pressure, int32_t *temperature)
{
    UNUSED(baro);

    if (pressure) {
        *pressure = icpState.pressure;
    }

    if (temperature) {
        *temperature = icpState.temperature;
    }

    return true;
}

#define DETECTION_MAX_RETRY_COUNT   5

static bool deviceDetect(busDevice_t *busDev)
{
    for (int retry = 0; retry < DETECTION_MAX_RETRY_COUNT; retry++) {
        uint8_t id = 0;
        uint8_t version = 0xFF;

        delay(10);

        // First access after power-up may return garbage - read the ID twice
        readReg8(busDev, REG_DEVICE_ID, &id);

        if (readReg8(busDev, REG_DEVICE_ID, &id) && id == ICP201XX_ID &&
            readReg8(busDev, REG_VERSION, &version) &&
            (version == VERSION_A1 || version == VERSION_B2)) {
            return true;
        }
    }

    return false;
}

static bool deviceConfigure(busDevice_t *busDev)
{
    delay(10);

    softReset(busDev);

    if (!bootSequence(busDev)) {
        return false;
    }

    if (!modeSelect(busDev, MODE_CONTINUOUS_120HZ)) {
        return false;
    }

    // Discard the first samples while the internal FIR filter settles
    if (!waitFifo(busDev, FIR_SETTLING_SAMPLES, 500)) {
        return false;
    }

    flushFifo(busDev);

    return waitFifo(busDev, 1, 100);
}

bool baroICP201XXDetect(baroDev_t *baro)
{
    baro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_ICP201XX, 0, OWNER_BARO);
    if (baro->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(baro->busDev) || !deviceConfigure(baro->busDev)) {
        busDeviceDeInit(baro->busDev);
        return false;
    }

    memset(&icpState, 0, sizeof(icpState));

    // 120 Hz ODR - read the FIFO at ~60 Hz and average 2 samples
    baro->ut_delay = 0;
    baro->start_ut = NULL;
    baro->get_ut = NULL;

    baro->up_delay = 1000000 / 60;
    baro->start_up = NULL;
    baro->get_up = deviceReadMeasurement;

    baro->calculate = deviceCalculate;

    return true;
}

#endif

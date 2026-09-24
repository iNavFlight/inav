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
 * Driver for the TDK InvenSense ICM-56686 6-axis accel/gyro.
 *
 * Ported from the Betaflight driver
 * (src/main/drivers/accgyro/accgyro_spi_icm56686.c), including the
 * initialisation-order fix from betaflight/betaflight#15750:
 *   - filters (SRC / UI LPF / notch), ODR/FSR and the INT1 pin are programmed
 *     while both sensors are still powered off, as required by DS-000563;
 *   - the sensors are then switched to Low-Noise mode;
 *   - the DRDY interrupt source is enabled last, after all driver state has
 *     been set up.
 *
 * The ICM-56686 shares the two-tier register architecture of the ICM-456xx
 * family (see accgyro_icm45686.c) but its direct register map differs
 * (PWR_MGMT0, INT1_*, ACCEL/GYRO_CONFIG0 and the IREG filter addresses are at
 * different locations), hence a separate driver:
 *
 *   - DREG_BANK1 : directly addressable over SPI (sensor data, power, ODR/FSR,
 *                  interrupt and FIFO configuration).
 *   - IREG       : indirect register access for everything else (filters,
 *                  offsets, SREG_CTRL). The host writes the 16-bit target
 *                  address into IREG_ADDR_15_8 / IREG_ADDR_7_0, then writes or
 *                  reads IREG_DATA. A minimum 4us gap is required between
 *                  consecutive IREG accesses; completion is signalled by the
 *                  IREG_DONE bit in REG_MISC2.
 *
 * IREG bank base addresses (added to the per-bank register offset):
 *   IPREG_SYS1 = 0xA400 (gyro filters/offsets)
 *   IPREG_SYS2 = 0xA500 (accel filters/offsets)
 *   IPREG_TOP1 = 0xA200 (SREG_CTRL, trim, ...)
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/system.h"
#include "drivers/time.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro_mpu.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_icm56686.h"

#if defined(USE_IMU_ICM56686)

// ---------------------------------------------------------------------------
// DREG_BANK1 - directly addressable registers
// ---------------------------------------------------------------------------
#define ICM56686_ACCEL_DATA_X1                  0x00 // 6 bytes accel (X,Y,Z)
#define ICM56686_GYRO_DATA_X1                   0x06 // 6 bytes gyro  (X,Y,Z)
#define ICM56686_TEMP_DATA0                     0x0C // 2 bytes temperature

#define ICM56686_PWR_MGMT0                      0x14
#define ICM56686_INT1_CONFIG0                   0x1A
#define ICM56686_INT1_CONFIG2                   0x1C
#define ICM56686_ACCEL_CONFIG0                  0x1F
#define ICM56686_GYRO_CONFIG0                   0x20
#define ICM56686_WHO_AM_I                       0x72

#define ICM56686_REG_IREG_ADDR_15_8             0x7C
#define ICM56686_REG_IREG_ADDR_7_0              0x7D
#define ICM56686_REG_IREG_DATA                  0x7E
#define ICM56686_REG_MISC2                      0x7F

// REG_MISC2 (0x7F) bits
#define ICM56686_SOFT_RESET                     (1 << 1)
#define ICM56686_BIT_IREG_DONE                  (1 << 0)

// PWR_MGMT0 (0x14): GYRO_MODE[3:2], ACCEL_MODE[1:0], 0b11 = Low-Noise
#define ICM56686_GYRO_MODE_OFF                  (0x00 << 2)
#define ICM56686_GYRO_MODE_LN                   (0x03 << 2)
#define ICM56686_ACCEL_MODE_OFF                 (0x00)
#define ICM56686_ACCEL_MODE_LN                  (0x03)

// ACCEL_CONFIG0 (0x1F): AP_ACCEL_FS_SEL[6:4], ACCEL_ODR[3:0]
#define ICM56686_ACCEL_FS_SEL_16G               (0x01 << 4)
#define ICM56686_ACCEL_ODR_1K6_LN               0x05

// GYRO_CONFIG0 (0x20): AP_GYRO_FS_SEL[7:4], GYRO_ODR[3:0]
#define ICM56686_GYRO_FS_SEL_2000DPS            (0x01 << 4)
#define ICM56686_GYRO_ODR_6K4_LN                0x03 // 0..3 all map to 6.4 kHz

// INT1_CONFIG0 (0x1A) - interrupt source enables
#define ICM56686_INT1_STATUS_EN_DRDY            (1 << 2)

// INT1_CONFIG2 (0x1C) - pin electrical configuration
#define ICM56686_INT1_DRIVE_CIRCUIT_PP          (0 << 2)
#define ICM56686_INT1_MODE_PULSED               (0 << 1)
#define ICM56686_INT1_POLARITY_ACTIVE_HIGH      (1 << 0)

// ---------------------------------------------------------------------------
// IREG (indirect) register addresses = bank base + register offset
// ---------------------------------------------------------------------------
#define ICM56686_IPREG_SYS1_BASE                0xA400
#define ICM56686_IPREG_SYS2_BASE                0xA500
#define ICM56686_IPREG_TOP1_BASE                0xA200

// SREG_CTRL : IPREG_TOP1 offset 0x60. Sensor data resolution/endianness.
// Reset value 0x0A is 20-bit big-endian; 0x00 selects 16-bit little-endian.
#define ICM56686_SREG_CTRL_IREG_ADDR            (ICM56686_IPREG_TOP1_BASE + 0x60)
#define ICM56686_SREG_CTRL_16BIT_LE             0x00

// Gyro SRC control : IPREG_SYS1 offset 0x9A, GYRO_SRC_CTRL[3:2]
#define ICM56686_GYRO_SRC_CTRL_IREG_ADDR        (ICM56686_IPREG_SYS1_BASE + 0x9A)
#define ICM56686_GYRO_SRC_CTRL_MASK             (0x03 << 2)
#define ICM56686_GYRO_SRC_CTRL_SRC_PREFILT_ON   (0x02 << 2) // SRC on + pre-filter on

// Gyro notch : IPREG_SYS1 offset 0x9D bit7. Reset is 0 (notch enabled).
#define ICM56686_GYRO_NOTCH_CFG_IREG_ADDR       (ICM56686_IPREG_SYS1_BASE + 0x9D)
#define ICM56686_GYRO_NOTCH_BYPASS              (1 << 7)

// Gyro UI LPF : IPREG_SYS1 offset 0x9E, GYRO_UI_LPFBW_SEL[6:4]
#define ICM56686_GYRO_UI_LPF_CFG_IREG_ADDR      (ICM56686_IPREG_SYS1_BASE + 0x9E)
#define ICM56686_GYRO_UI_LPFBW_SHIFT            4
#define ICM56686_GYRO_UI_LPFBW_MASK             (0x07 << ICM56686_GYRO_UI_LPFBW_SHIFT)

// Accel SRC control : IPREG_SYS2 offset 0x6D, ACCEL_SRC_CTRL[1:0]
#define ICM56686_ACCEL_SRC_CTRL_IREG_ADDR       (ICM56686_IPREG_SYS2_BASE + 0x6D)
#define ICM56686_ACCEL_SRC_CTRL_MASK            (0x03 << 0)
#define ICM56686_ACCEL_SRC_CTRL_SRC_PREFILT_ON  (0x02 << 0)

// Accel UI LPF : IPREG_SYS2 offset 0x70, ACCEL_UI_LPFBW_SEL[2:0]
#define ICM56686_ACCEL_UI_LPF_CFG_IREG_ADDR     (ICM56686_IPREG_SYS2_BASE + 0x70)
#define ICM56686_ACCEL_UI_LPFBW_MASK            (0x07 << 0)

// UI LPF bandwidth selections (ODR-relative). Encoding is identical for the
// gyro [6:4] and accel [2:0] fields.
#define ICM56686_UI_LPFBW_BYPASS                0x00
#define ICM56686_UI_LPFBW_ODR_DIV_4             0x01
#define ICM56686_UI_LPFBW_ODR_DIV_8             0x02
#define ICM56686_UI_LPFBW_ODR_DIV_16            0x03
#define ICM56686_UI_LPFBW_ODR_DIV_32            0x04
#define ICM56686_UI_LPFBW_ODR_DIV_64            0x05
#define ICM56686_UI_LPFBW_ODR_DIV_128           0x06

// ---------------------------------------------------------------------------
// Timing / misc
// ---------------------------------------------------------------------------
#define ICM56686_RESET_TIMEOUT_US               20000 // power-on reset is 5 ms typ
#define ICM56686_IREG_TIMEOUT_US                5000
#define ICM56686_IREG_MIN_GAP_US                4
#define ICM56686_GYRO_STARTUP_TIME_MS           35    // Low-noise gyro startup, typ. (accel is 10 ms typ.)

#define ICM56686_DATA_LENGTH                    6     // 3 axes * 2 bytes

#define ICM56686_CHIP_MAGIC                     0x5668

static const gyroFilterAndRateConfig_t icm56686GyroConfigs[] = {
    /*   LPF          ODR   { UI LPF BW,                    GYRO_ODR } */
    { GYRO_LPF_NONE,  6400, { ICM56686_UI_LPFBW_BYPASS,     ICM56686_GYRO_ODR_6K4_LN } },
    { GYRO_LPF_256HZ, 6400, { ICM56686_UI_LPFBW_ODR_DIV_16, ICM56686_GYRO_ODR_6K4_LN } }, // ~400 Hz
    { GYRO_LPF_188HZ, 6400, { ICM56686_UI_LPFBW_ODR_DIV_32, ICM56686_GYRO_ODR_6K4_LN } }, // ~200 Hz
    { GYRO_LPF_98HZ,  6400, { ICM56686_UI_LPFBW_ODR_DIV_64, ICM56686_GYRO_ODR_6K4_LN } }, // ~100 Hz
    { GYRO_LPF_42HZ,  6400, { ICM56686_UI_LPFBW_ODR_DIV_128, ICM56686_GYRO_ODR_6K4_LN } }, // ~50 Hz
};

// ---------------------------------------------------------------------------
// IREG access helpers
// ---------------------------------------------------------------------------

// Wait for IREG_DONE (REG_MISC2 bit0). IREG_DONE reads 1 at idle, so polling
// immediately after the access could observe a stale "done" before the
// internal transfer has started. Honour the datasheet minimum 4us gap first.
static bool icm56686WaitIregDone(const busDevice_t *dev)
{
    delayMicroseconds(ICM56686_IREG_MIN_GAP_US);

    for (uint32_t waitedUs = 0; waitedUs < ICM56686_IREG_TIMEOUT_US; waitedUs += 10) {
        uint8_t misc2 = 0;
        busRead(dev, ICM56686_REG_MISC2, &misc2);
        if (misc2 & ICM56686_BIT_IREG_DONE) {
            return true;
        }
        delayMicroseconds(10);
    }

    return false;
}

// Write one byte to an indirect (IREG) register. The write must be a single
// auto-incrementing SPI burst (IREG_ADDR_15_8, IREG_ADDR_7_0, IREG_DATA) with
// CS held low; writing the three registers as separate transactions does not
// trigger the internal transfer on this device.
static bool icm56686WriteIREG(const busDevice_t *dev, uint16_t reg, uint8_t value)
{
    const uint8_t buf[3] = { (uint8_t)((reg >> 8) & 0xFF), (uint8_t)(reg & 0xFF), value };
    busWriteBuf(dev, ICM56686_REG_IREG_ADDR_15_8, buf, sizeof(buf));

    return icm56686WaitIregDone(dev);
}

// Read one byte from an indirect (IREG) register. The host programs the target
// address, the device pre-fetches it into IREG_DATA and, after IREG_DONE, the
// host reads IREG_DATA.
static bool icm56686ReadIREG(const busDevice_t *dev, uint16_t reg, uint8_t *value)
{
    const uint8_t buf[2] = { (uint8_t)((reg >> 8) & 0xFF), (uint8_t)(reg & 0xFF) };
    busWriteBuf(dev, ICM56686_REG_IREG_ADDR_15_8, buf, sizeof(buf));

    if (!icm56686WaitIregDone(dev)) {
        return false;
    }

    busRead(dev, ICM56686_REG_IREG_DATA, value);
    // The address auto-increments and a new pre-fetch is triggered; let it settle.
    icm56686WaitIregDone(dev);

    return true;
}

// Read-modify-write a field of an IREG register, preserving reserved bits at
// their reset values.
static bool icm56686ModifyIREG(const busDevice_t *dev, uint16_t reg, uint8_t mask, uint8_t value)
{
    uint8_t cur = 0;
    if (!icm56686ReadIREG(dev, reg, &cur)) {
        return icm56686WriteIREG(dev, reg, value & mask);
    }

    return icm56686WriteIREG(dev, reg, (cur & ~mask) | (value & mask));
}

// ---------------------------------------------------------------------------
// Accel
// ---------------------------------------------------------------------------

// Accel scale only. Hardware ODR/SRC/LPF are programmed from
// icm56686AccAndGyroInit while the sensors are still off.
static void icm56686AccInit(accDev_t *acc)
{
    acc->acc_1G = 512 * 4; // 16-bit mode, +/-16 g -> 2048 LSB/g
}

static bool icm56686AccRead(accDev_t *acc)
{
    uint8_t data[ICM56686_DATA_LENGTH];

    const bool ack = busReadBuf(acc->busDev, ICM56686_ACCEL_DATA_X1, data, ICM56686_DATA_LENGTH);
    if (!ack) {
        return false;
    }

    acc->ADCRaw[X] = (float) int16_val_little_endian(data, 0);
    acc->ADCRaw[Y] = (float) int16_val_little_endian(data, 1);
    acc->ADCRaw[Z] = (float) int16_val_little_endian(data, 2);

    return true;
}

// ---------------------------------------------------------------------------
// Gyro
// ---------------------------------------------------------------------------

static bool icm56686GyroRead(gyroDev_t *gyro)
{
    uint8_t data[ICM56686_DATA_LENGTH];

    const bool ack = busReadBuf(gyro->busDev, ICM56686_GYRO_DATA_X1, data, ICM56686_DATA_LENGTH);
    if (!ack) {
        return false;
    }

    gyro->gyroADCRaw[X] = (float) int16_val_little_endian(data, 0);
    gyro->gyroADCRaw[Y] = (float) int16_val_little_endian(data, 1);
    gyro->gyroADCRaw[Z] = (float) int16_val_little_endian(data, 2);

    return true;
}

static bool icm56686ReadTemperature(gyroDev_t *gyro, int16_t *temp)
{
    uint8_t data[2];

    const bool ack = busReadBuf(gyro->busDev, ICM56686_TEMP_DATA0, data, 2);
    if (!ack) {
        return false;
    }

    // Temperature in degC = (TEMP_DATA / 128) + 25; stored as degC * 10
    *temp = (int16_val_little_endian(data, 0) / 12.8f) + 250.0f;

    return true;
}

// Program filters, ODR/FSR and INT1 while both sensors are off, then power up.
// DRDY is enabled last, after all driver state has been set (betaflight#15750).
static void icm56686AccAndGyroInit(gyroDev_t *gyro)
{
    busDevice_t *dev = gyro->busDev;
    const gyroFilterAndRateConfig_t *config = chooseGyroConfig(gyro->lpf, 1000000 / gyro->requestedSampleIntervalUs,
                                                               &icm56686GyroConfigs[0], ARRAYLEN(icm56686GyroConfigs));

    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);

    // Sensors are off here: detection left PWR_MGMT0 at 0 after soft reset.
    // Per DS-000563 a field may be written while its sensor is on only when
    // the register description says so; SRC, the UI LPF and the notch do not.
    busWrite(dev, ICM56686_PWR_MGMT0, ICM56686_GYRO_MODE_OFF | ICM56686_ACCEL_MODE_OFF);

    // 16-bit little-endian output. Clearing sreg_sifs_20bits_en is also
    // required for FS_SEL to set the digital full-scale.
    icm56686WriteIREG(dev, ICM56686_SREG_CTRL_IREG_ADDR, ICM56686_SREG_CTRL_16BIT_LE);

    // Gyro: SRC + pre-filter, UI LPF, bypass the undocumented-frequency notch so
    // GYRO_UI_LPFBW_SEL is the only hardware lowpass.
    icm56686ModifyIREG(dev, ICM56686_GYRO_SRC_CTRL_IREG_ADDR,
                       ICM56686_GYRO_SRC_CTRL_MASK, ICM56686_GYRO_SRC_CTRL_SRC_PREFILT_ON);
    icm56686ModifyIREG(dev, ICM56686_GYRO_UI_LPF_CFG_IREG_ADDR,
                       ICM56686_GYRO_UI_LPFBW_MASK, (uint8_t)(config->gyroConfigValues[0] << ICM56686_GYRO_UI_LPFBW_SHIFT));
    icm56686ModifyIREG(dev, ICM56686_GYRO_NOTCH_CFG_IREG_ADDR,
                       ICM56686_GYRO_NOTCH_BYPASS, ICM56686_GYRO_NOTCH_BYPASS);

    // Accel: SRC + pre-filter, UI LPF at ODR/8 (~200 Hz with the 1.6 kHz ODR below).
    icm56686ModifyIREG(dev, ICM56686_ACCEL_SRC_CTRL_IREG_ADDR,
                       ICM56686_ACCEL_SRC_CTRL_MASK, ICM56686_ACCEL_SRC_CTRL_SRC_PREFILT_ON);
    icm56686ModifyIREG(dev, ICM56686_ACCEL_UI_LPF_CFG_IREG_ADDR,
                       ICM56686_ACCEL_UI_LPFBW_MASK, ICM56686_UI_LPFBW_ODR_DIV_8);

    // ODR and full-scale; program before enabling so the startup interval
    // runs against the final configuration.
    busWrite(dev, ICM56686_ACCEL_CONFIG0, ICM56686_ACCEL_FS_SEL_16G | ICM56686_ACCEL_ODR_1K6_LN);
    busWrite(dev, ICM56686_GYRO_CONFIG0, ICM56686_GYRO_FS_SEL_2000DPS | config->gyroConfigValues[1]);

    // INT1_CONFIG0 resets to 0x80 (RESET_DONE enabled). INT1_MODE and
    // INT1_POLARITY may only be changed while every source on this interface
    // is disabled, so clear the sources before configuring the pin.
    busWrite(dev, ICM56686_INT1_CONFIG0, 0x00);
    busWrite(dev, ICM56686_INT1_CONFIG2, ICM56686_INT1_MODE_PULSED | ICM56686_INT1_DRIVE_CIRCUIT_PP |
                                         ICM56686_INT1_POLARITY_ACTIVE_HIGH);

    // Power both sensors up in Low-Noise mode and wait for the gyro to start.
    busWrite(dev, ICM56686_PWR_MGMT0, ICM56686_GYRO_MODE_LN | ICM56686_ACCEL_MODE_LN);
    delay(ICM56686_GYRO_STARTUP_TIME_MS);

    gyro->sampleRateIntervalUs = 1000000 / config->gyroRateHz;

    // Enable the data-ready interrupt last, once all driver state is valid.
    busWrite(dev, ICM56686_INT1_CONFIG0, ICM56686_INT1_STATUS_EN_DRDY);

    busSetSpeed(dev, BUS_SPEED_FAST);
}

// ---------------------------------------------------------------------------
// Detection
// ---------------------------------------------------------------------------

static bool icm56686DeviceDetect(busDevice_t *dev)
{
    uint8_t tmp = 0xFF;
    uint8_t attemptsRemaining = 20;

    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);

    // The ICM-56686 has no bank-select register; soft reset is REG_MISC2 bit 1.
    busWrite(dev, ICM56686_REG_MISC2, ICM56686_SOFT_RESET);

    // Wait for the soft-reset bit to self-clear (power-on reset is ~5 ms).
    for (uint32_t waitedUs = 0; ; waitedUs += 10) {
        busRead(dev, ICM56686_REG_MISC2, &tmp);
        if (!(tmp & ICM56686_SOFT_RESET)) {
            break;
        }
        if (waitedUs >= ICM56686_RESET_TIMEOUT_US) {
            return false;
        }
        delayMicroseconds(10);
    }

    // Put power management into a known (off) state after reset.
    busWrite(dev, ICM56686_PWR_MGMT0, ICM56686_GYRO_MODE_OFF | ICM56686_ACCEL_MODE_OFF);

    do {
        delay(1);
        busRead(dev, ICM56686_WHO_AM_I, &tmp);
        if (tmp == ICM56686_WHO_AM_I_CONST) {
            return true;
        }
    } while (attemptsRemaining--);

    return false;
}

bool icm56686AccDetect(accDev_t *acc)
{
    acc->busDev = busDeviceOpen(BUSTYPE_ANY, DEVHW_ICM56686, acc->imuSensorToUse);
    if (acc->busDev == NULL) {
        return false;
    }

    mpuContextData_t *ctx = busDeviceGetScratchpadMemory(acc->busDev);
    if (ctx->chipMagicNumber != ICM56686_CHIP_MAGIC) {
        return false;
    }

    acc->initFn = icm56686AccInit;
    acc->readFn = icm56686AccRead;
    acc->accAlign = acc->busDev->param;

    return true;
}

bool icm56686GyroDetect(gyroDev_t *gyro)
{
    gyro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_ICM56686, gyro->imuSensorToUse, OWNER_MPU);
    if (gyro->busDev == NULL) {
        return false;
    }

    if (!icm56686DeviceDetect(gyro->busDev)) {
        busDeviceDeInit(gyro->busDev);
        return false;
    }

    // Magic number for ACC detection to indicate that we have detected an ICM-56686 gyro
    mpuContextData_t *ctx = busDeviceGetScratchpadMemory(gyro->busDev);
    ctx->chipMagicNumber = ICM56686_CHIP_MAGIC;

    gyro->initFn = icm56686AccAndGyroInit;
    gyro->readFn = icm56686GyroRead;
    gyro->intStatusFn = gyroCheckDataReady;
    gyro->temperatureFn = icm56686ReadTemperature;
    gyro->scale = 1.0f / 16.4f;     // 16.4 LSB/dps at +/-2000 dps
    gyro->gyroAlign = gyro->busDev->param;

    return true;
}

#endif // USE_IMU_ICM56686

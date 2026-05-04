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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"
#include "common/log.h"

#include "drivers/system.h"
#include "drivers/time.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro_mpu.h"
#include "drivers/accgyro/accgyro.h"
#include "accgyro_icm45686.h"

#if defined(USE_IMU_ICM45686)
/*
reference: https://github.com/tdk-invn-oss/motion.mcu.icm45686.driver
Datasheet: https://invensense.tdk.com/wp-content/uploads/documentation/DS-000577_ICM-45686.pdf
Datasheet: https://invensense.tdk.com/wp-content/uploads/documentation/DS-000576_ICM-45605.pdf

Note: ICM456xx has two modes of operation: Low-Power Mode Low-Noise Mode
Note: Now implemented only UI Interface with Low-Noise Mode

 The following diagram shows the signal path for each mode:
 The cut-off frequency of the filters is determined by the ODR setting.

                   Low-Noise Mode
     +------+     +--------------+    +-------------+    +--------------+    +------------------+
     | ADC  |---->| Anti-Alias   |--->| Interpolator|--->|     LPF      |--->| Sensor Registers |---> UI Interface
     |      |     | Filter (AAF) |    |             | +->| & ODR Select |    |                  |
     +--|---+     +--------------+    +-------------+ |  +--------------+    +------------------+
        |                                             |
        |           Low-Power Mode                    |
        |         +--------------+                    |
        |-------->| Notch Filter |--------------------|
        |         |              |
        |         +--------------+
        |
        |
     +--|---+           +--------------+       +------+       +------+      +------------------+
     | ADC  | --------> | Notch Filter | --->  | HPF  | --->  | LPF  | ---> | Sensor Registers | ---> AUX1 Interface
     |      |           |              |       |      |       |      |      |                  |
     +------+           +--------------+       +------+       +------+      +------------------+

 The AUX1 interface default configuration can be checked by read only register IOC_PAD_SCENARIO through host interface.
 By default, AUX1 interface is enabled, and default interface for AUX1 is SPI3W or I3CSM.

 In Low-Noise Mode, the ADC output is sent through an Anti-Alias Filter (AAF). The AAF is an FIR filter with fixed
 coefficients (not user configurable). The AAF can be enabled or disabled by the user using GYRO_SRC_CTRL and
 ACCEL_SRC_CTRL.

 The AUX1 signal path includes a Notch Filter. The notch filter is not user programmable. The usage of the notch
 filter in the auxiliary path is recommended for sharper roll-off and for the cases where user is asynchronously
 sampling the auxiliary interface data output at integer multiples of 1 kHz rate. The notch filter may be bypassed
 using GYRO_OIS_M6_BYP.

 The notch filter is followed by an HPF on the AUX1 signal path. HPF cut-off frequency can be selected using
 GYRO_OIS_HPFBW_SEL and ACCEL_OIS_HPFBW_SEL. HPF can be bypassed using GYRO_OIS_HPF1_BYP and
 ACCEL_OIS_HPF1_BYP.

 The HPF is followed by LPF on the AUX1 signal path. The AUX1 LPF BW is set by register bit field
 GYRO_OIS_LPF1BW_SEL and ACCEL_OIS_LPF1BW_SEL for gyroscope and accelerometer respectively. This is
 followed by Full Scale Range (FSR) selection based on user configurable settings for register fields
 GYRO_AUX1_FS_SEL and ACCEL_AUX1_FS_SEL. AUX1 output is fixed at 6.4kHz ODR.
*/

// NOTE: ICM-45686 does NOT have a bank select register like ICM-426xx
// The ICM-45686 uses Indirect Register (IREG) access for internal registers
// Register 0x75 is RESERVED/UNDEFINED in the ICM-45686 datasheet
// DO NOT use bank switching on this device

// Register map User Bank 0 (UI Interface)
#define ICM456XX_WHO_AM_REGISTER                0x72
#define ICM456XX_REG_MISC2                      0x7F
#define ICM456XX_INT1_CONFIG0                   0x16
#define ICM456XX_INT1_CONFIG2                   0x18
#define ICM456XX_INT1_STATUS0                   0x19
#define ICM456XX_INT1_STATUS1                   0x1A
#define ICM456XX_GYRO_CONFIG0                   0x1C
#define ICM456XX_ACCEL_CONFIG0                  0x1B
#define ICM456XX_PWR_MGMT0                      0x10
#define ICM456XX_TEMP_DATA1                     0x0C
// MGMT0 - 0x10 - Gyro
#define ICM456XX_GYRO_MODE_OFF                  (0x00 << 2)
#define ICM456XX_GYRO_MODE_STANDBY              (0x01 << 2)
#define ICM456XX_GYRO_MODE_LP                   (0x02 << 2)  // Low Power Mode
#define ICM456XX_GYRO_MODE_LN                   (0x03 << 2)  // Low Noise Mode

// MGMT0 - 0x10 - Accel
#define ICM456XX_ACCEL_MODE_OFF                 (0x00)
#define ICM456XX_ACCEL_MODE_OFF2                (0x01)
#define ICM456XX_ACCEL_MODE_LP                  (0x02) // Low Power Mode
#define ICM456XX_ACCEL_MODE_LN                  (0x03) // Low Noise Mode

// INT1_CONFIG0 - 0x16
#define ICM456XX_INT1_STATUS_EN_RESET_DONE      (1 << 7)
#define ICM456XX_INT1_STATUS_EN_AUX1_AGC_RDY    (1 << 6)
#define ICM456XX_INT1_STATUS_EN_AP_AGC_RDY      (1 << 5)
#define ICM456XX_INT1_STATUS_EN_AP_FSYNC        (1 << 4)
#define ICM456XX_INT1_STATUS_EN_AUX1_DRDY       (1 << 3)
#define ICM456XX_INT1_STATUS_EN_DRDY            (1 << 2)
#define ICM456XX_INT1_STATUS_EN_FIFO_THS        (1 << 1)
#define ICM456XX_INT1_STATUS_EN_FIFO_FULL       (1 << 0)

// INT1_CONFIG2 - 0x18
#define ICM456XX_INT1_DRIVE_CIRCUIT_PP          (0 << 2)
#define ICM456XX_INT1_DRIVE_CIRCUIT_OD          (1 << 2)
#define ICM456XX_INT1_MODE_PULSED               (0 << 1)
#define ICM456XX_INT1_MODE_LATCHED              (1 << 1)
#define ICM456XX_INT1_POLARITY_ACTIVE_LOW       (0 << 0)
#define ICM456XX_INT1_POLARITY_ACTIVE_HIGH      (1 << 0)

// INT1_STATUS0 - 0x19
#define ICM456XX_INT1_STATUS_RESET_DONE         (1 << 7)
#define ICM456XX_INT1_STATUS_AUX1_AGC_RDY       (1 << 6)
#define ICM456XX_INT1_STATUS_AP_AGC_RDY         (1 << 5)
#define ICM456XX_INT1_STATUS_AP_FSYNC           (1 << 4)
#define ICM456XX_INT1_STATUS_AUX1_DRDY          (1 << 3)
#define ICM456XX_INT1_STATUS_DRDY               (1 << 2)
#define ICM456XX_INT1_STATUS_FIFO_THS           (1 << 1)
#define ICM456XX_INT1_STATUS_FIFO_FULL          (1 << 0)

// REG_MISC2 - 0x7F
#define ICM456XX_SOFT_RESET                     (1 << 1)
#define ICM456XX_RESET_TIMEOUT_US               20000  // 20 ms

#define ICM456XX_ACCEL_DATA_X1_UI               0x00
#define ICM456XX_GYRO_DATA_X1_UI                0x06

// ACCEL_CONFIG0 - 0x1B
#define ICM456XX_ACCEL_FS_SEL_32G               (0x00 << 4)
#define ICM456XX_ACCEL_FS_SEL_16G               (0x01 << 4)
#define ICM456XX_ACCEL_FS_SEL_8G                (0x02 << 4)
#define ICM456XX_ACCEL_FS_SEL_4G                (0x03 << 4)
#define ICM456XX_ACCEL_FS_SEL_2G                (0x04 << 4)

// ACCEL_CONFIG0 - 0x1B
#define ICM456XX_ACCEL_ODR_6K4_LN               0x03
#define ICM456XX_ACCEL_ODR_3K2_LN               0x04
#define ICM456XX_ACCEL_ODR_1K6_LN               0x05
#define ICM456XX_ACCEL_ODR_800_LN               0x06
#define ICM456XX_ACCEL_ODR_400_LP_LN            0x07
#define ICM456XX_ACCEL_ODR_200_LP_LN            0x08
#define ICM456XX_ACCEL_ODR_100_LP_LN            0x09
#define ICM456XX_ACCEL_ODR_50_LP_LN             0x0A
#define ICM456XX_ACCEL_ODR_25_LP_LN             0x0B
#define ICM456XX_ACCEL_ODR_12_5_LP_LN           0x0C
#define ICM456XX_ACCEL_ODR_6_25_LP              0x0D
#define ICM456XX_ACCEL_ODR_3_125_LP             0x0E
#define ICM456XX_ACCEL_ODR_1_5625_LP            0x0F

// GYRO_CONFIG0 - 0x1C
#define ICM456XX_GYRO_FS_SEL_4000DPS            (0x00 << 4)
#define ICM456XX_GYRO_FS_SEL_2000DPS            (0x01 << 4)
#define ICM456XX_GYRO_FS_SEL_1000DPS            (0x02 << 4)
#define ICM456XX_GYRO_FS_SEL_500DPS             (0x03 << 4)
#define ICM456XX_GYRO_FS_SEL_250DPS             (0x04 << 4)
#define ICM456XX_GYRO_FS_SEL_125DPS             (0x05 << 4)
#define ICM456XX_GYRO_FS_SEL_62_5DPS            (0x06 << 4)
#define ICM456XX_GYRO_FS_SEL_31_25DPS           (0x07 << 4)
#define ICM456XX_GYRO_FS_SEL_15_625DPS          (0x08 << 4)

// GYRO_CONFIG0 - 0x1C
#define ICM456XX_GYRO_ODR_6K4_LN                0x03
#define ICM456XX_GYRO_ODR_3K2_LN                0x04
#define ICM456XX_GYRO_ODR_1K6_LN                0x05
#define ICM456XX_GYRO_ODR_800_LN                0x06
#define ICM456XX_GYRO_ODR_400_LP_LN             0x07
#define ICM456XX_GYRO_ODR_200_LP_LN             0x08
#define ICM456XX_GYRO_ODR_100_LP_LN             0x09
#define ICM456XX_GYRO_ODR_50_LP_LN              0x0A
#define ICM456XX_GYRO_ODR_25_LP_LN              0x0B
#define ICM456XX_GYRO_ODR_12_5_LP_LN            0x0C
#define ICM456XX_GYRO_ODR_6_25_LP               0x0D
#define ICM456XX_GYRO_ODR_3_125_LP              0x0E
#define ICM456XX_GYRO_ODR_1_5625_LP             0x0F

// Accel IPREG_SYS2_REG_123 - 0x7B
#define ICM456XX_SRC_CTRL_AAF_ENABLE_BIT        (1 << 0) // Anti-Alias Filter - AAF
#define ICM456XX_SRC_CTRL_INTERP_ENABLE_BIT     (1 << 1) // Interpolator

// IPREG_SYS2_REG_123 - 0x7B
#define ICM456XX_ACCEL_SRC_CTRL_IREG_ADDR       0xA57B // To access register in IPREG_SYS2, add base address 0xA500 + offset

// IPREG_SYS1_REG_166 - 0xA6
#define ICM456XX_GYRO_SRC_CTRL_IREG_ADDR        0xA4A6 // To access register in IPREG_SYS1, add base address 0xA400 + offset

// HOST INDIRECT ACCESS REGISTER (IREG)
#define ICM456XX_REG_IREG_ADDR_15_8             0x7C
#define ICM456XX_REG_IREG_ADDR_7_0              0x7D
#define ICM456XX_REG_IREG_DATA                  0x7E


// IPREG_SYS1_REG_172 - 0xAC
#define ICM456XX_GYRO_UI_LPF_CFG_IREG_ADDR       0xA4AC // To access register in IPREG_SYS1, add base address 0xA400 + offset

// LPF UI - 0xAC PREG_SYS1_REG_172 (bits 2:0)
#define ICM456XX_GYRO_UI_LPFBW_BYPASS            0x00
#define ICM456XX_GYRO_UI_LPFBW_ODR_DIV_4         0x01 // 1600 Hz ODR = 6400 Hz:
#define ICM456XX_GYRO_UI_LPFBW_ODR_DIV_8         0x02 // 800 Hz ODR = 6400 Hz:
#define ICM456XX_GYRO_UI_LPFBW_ODR_DIV_16        0x03 // 400 Hz ODR = 6400 Hz:
#define ICM456XX_GYRO_UI_LPFBW_ODR_DIV_32        0x04 // 200 Hz ODR = 6400 Hz
#define ICM456XX_GYRO_UI_LPFBW_ODR_DIV_64        0x05 // 100 Hz ODR = 6400 Hz
#define ICM456XX_GYRO_UI_LPFBW_ODR_DIV_128       0x06 // 50 Hz ODR = 6400 Hz

// IPREG_SYS2_REG_131 - 0x83
#define ICM456XX_ACCEL_UI_LPF_CFG_IREG_ADDR      0xA583 // To access register in IPREG_SYS2, add base address 0xA500 + offset

// Accel UI path LPF - 0x83 IPREG_SYS2_REG_131 (bits 2:0)
#define ICM456XX_ACCEL_UI_LPFBW_BYPASS           0x00
#define ICM456XX_ACCEL_UI_LPFBW_ODR_DIV_4        0x01 // 400 Hz ODR = 1600 Hz:
#define ICM456XX_ACCEL_UI_LPFBW_ODR_DIV_8        0x02 // 200 Hz ODR = 1600 Hz:
#define ICM456XX_ACCEL_UI_LPFBW_ODR_DIV_16       0x03 // 100 Hz ODR = 1600 Hz:
#define ICM456XX_ACCEL_UI_LPFBW_ODR_DIV_32       0x04 // 50 Hz ODR = 1600 Hz
#define ICM456XX_ACCEL_UI_LPFBW_ODR_DIV_64       0x05 // 25 Hz ODR = 1600 Hz
#define ICM456XX_ACCEL_UI_LPFBW_ODR_DIV_128      0x06 // 12.5 Hz ODR = 1600 Hz

#ifndef ICM456XX_CLOCK
// Default: 24 MHz max SPI frequency
#define ICM456XX_MAX_SPI_CLK_HZ                 24000000
#else
#define ICM456XX_MAX_SPI_CLK_HZ                 ICM456XX_CLOCK
#endif

#define HZ_TO_US(hz)                            ((int32_t)((1000 * 1000) / (hz)))

#define ICM456XX_BIT_IREG_DONE                  (1 << 0)

// Startup timing constants (DS-000577 Table 9-6)
#define ICM456XX_ACCEL_STARTUP_TIME_MS          10  // Min accel startup from OFF/STANDBY/LP
#define ICM456XX_GYRO_STARTUP_TIME_MS           35  // Min gyro startup from OFF/STANDBY/LP
#define ICM456XX_SENSOR_ENABLE_DELAY_MS         1   // Allow sensors to power on and stabilize
#define ICM456XX_INT_CONFIG_DELAY_MS            15  // Register settle time after interrupt config (matches ICM-426xx convention)
#define ICM456XX_IREG_TIMEOUT_US                5000 // IREG operation timeout (5ms max)

#define ICM456XX_DATA_LENGTH                    6  // 3 axes * 2 bytes per axis
#define ICM456XX_SPI_BUFFER_SIZE                (1 + ICM456XX_DATA_LENGTH) // 1 byte register + 6 bytes data

static const gyroFilterAndRateConfig_t icm45xxGyroConfigs[] = {
    /*   LPF          ODR   { lpfBits,                            odrReg } */
    { GYRO_LPF_NONE,  6000, { ICM456XX_GYRO_UI_LPFBW_BYPASS,      0x03 } },
    { GYRO_LPF_256HZ, 6000, { ICM456XX_GYRO_UI_LPFBW_ODR_DIV_16,  0x03 } }, // ≈400 Hz
    { GYRO_LPF_188HZ, 6000, { ICM456XX_GYRO_UI_LPFBW_ODR_DIV_32,  0x03 } }, // ≈200 Hz
    { GYRO_LPF_98HZ,  6000, { ICM456XX_GYRO_UI_LPFBW_ODR_DIV_64,  0x03 } }, // ≈100 Hz
    { GYRO_LPF_42HZ,  6000, { ICM456XX_GYRO_UI_LPFBW_ODR_DIV_128, 0x03 } }, // ≈50 Hz
};

/**
 * @brief This function follows the IREG WRITE procedure (Section 14.1-14.4 of the datasheet)
 * using indirect addressing via IREG_ADDR_15_8, IREG_ADDR_7_0, and IREG_DATA registers.
 * After writing, an internal operation transfers the data to the target IREG address.
 * Ensures compliance with the required minimum time gap and checks the IREG_DONE bit.
 *
 * @param dev   Pointer to the SPI device structure.
 * @param reg   16-bit internal IREG register address.
 * @param value Value to be written to the register.
 * @return true if the write was successful
 */
static bool icm45686WriteIREG(const busDevice_t *dev, uint16_t reg, uint8_t value)
{
    const uint8_t msb = (reg >> 8) & 0xFF;
    const uint8_t lsb = reg & 0xFF;

    busWrite(dev, ICM456XX_REG_IREG_ADDR_15_8, msb);
    busWrite(dev, ICM456XX_REG_IREG_ADDR_7_0, lsb);
    busWrite(dev, ICM456XX_REG_IREG_DATA, value);

    // Check IREG_DONE (bit 0 of REG_MISC2 = 0x7F) with elapsed-time tracking
    for (uint32_t waited_us = 0; waited_us < ICM456XX_IREG_TIMEOUT_US; waited_us += 10) {
        uint8_t misc2 = 0;
        busRead(dev, ICM456XX_REG_MISC2, &misc2);
        if (misc2 & ICM456XX_BIT_IREG_DONE) {
            return true;
        }
        delayMicroseconds(10);
    }

    return false; // timeout
}

static void icm45686AccInit(accDev_t *acc)
{
    acc->acc_1G = 512 * 4; // 16g scale
}

static bool icm45686AccRead(accDev_t *acc)
{
    uint8_t data[6];

    const bool ack = busReadBuf(acc->busDev, ICM456XX_ACCEL_DATA_X1_UI, data, 6);
    if (!ack) {
        return false;
    }

    acc->ADCRaw[X] = (float) int16_val_little_endian(data, 0);
    acc->ADCRaw[Y] = (float) int16_val_little_endian(data, 1);
    acc->ADCRaw[Z] = (float) int16_val_little_endian(data, 2);

    return true;
}

static bool icm45686GyroRead(gyroDev_t *gyro)
{
    uint8_t data[6];

    const bool ack = busReadBuf(gyro->busDev, ICM456XX_GYRO_DATA_X1_UI, data, 6);
    if (!ack) {
        return false;
    }

    gyro->gyroADCRaw[X] = (float) int16_val_little_endian(data, 0);
    gyro->gyroADCRaw[Y] = (float) int16_val_little_endian(data, 1);
    gyro->gyroADCRaw[Z] = (float) int16_val_little_endian(data, 2);

    return true;
}

static bool icm45686ReadTemperature(gyroDev_t *gyro, int16_t * temp)
{
    uint8_t data[2];

    const bool ack = busReadBuf(gyro->busDev, ICM456XX_TEMP_DATA1, data, 2);
    if (!ack) {
        return false;
    }
    // From datasheet: Temperature in Degrees Centigrade = (TEMP_DATA / 128) + 25 
    *temp = ( int16_val_little_endian(data, 0) / 12.8f ) + 250.0f; // Temperature stored as degC*10

    return true;
}

static void icm45686AccAndGyroInit(gyroDev_t *gyro)
{
    busDevice_t * dev = gyro->busDev;
    const gyroFilterAndRateConfig_t * config = chooseGyroConfig(gyro->lpf, 1000000 / gyro->requestedSampleIntervalUs,
                                                                &icm45xxGyroConfigs[0], ARRAYLEN(icm45xxGyroConfigs));
    gyro->sampleRateIntervalUs = 1000000 / config->gyroRateHz;
        
    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);

    // enable sensors
    busWrite(dev, ICM456XX_PWR_MGMT0, (ICM456XX_GYRO_MODE_LN | ICM456XX_ACCEL_MODE_LN));
    // Allow sensors to power on and stabilize
    delay(ICM456XX_SENSOR_ENABLE_DELAY_MS);
    // Configure accelerometer full-scale range (16g mode)
    busWrite(dev, ICM456XX_ACCEL_CONFIG0, ICM456XX_ACCEL_FS_SEL_16G | ICM456XX_ACCEL_ODR_1K6_LN);
    // Per datasheet Table 9-6: 10ms minimum startup time
    delay(ICM456XX_ACCEL_STARTUP_TIME_MS);
    // gyro filters
    // Enable Anti-Alias (AAF) Filter and Interpolator for Gyro (Section 7.2 of datasheet)
    if (!icm45686WriteIREG(dev, ICM456XX_GYRO_SRC_CTRL_IREG_ADDR, ICM456XX_SRC_CTRL_AAF_ENABLE_BIT | ICM456XX_SRC_CTRL_INTERP_ENABLE_BIT)) {
        // AAF/Interpolator initialization failed, fallback to disabled state
        icm45686WriteIREG(dev, ICM456XX_GYRO_SRC_CTRL_IREG_ADDR, 0);
    }
    // Set the Gyro UI LPF bandwidth cut-off (Section 7.3 of datasheet)
    if (!icm45686WriteIREG(dev, ICM456XX_GYRO_UI_LPF_CFG_IREG_ADDR, config->gyroConfigValues[0])) {
        // If LPF configuration fails, fallback to BYPASS
        icm45686WriteIREG(dev, ICM456XX_GYRO_UI_LPF_CFG_IREG_ADDR, ICM456XX_GYRO_UI_LPFBW_BYPASS);
    }
    // accel filters
    // Enable Anti-Alias Filter and Interpolator for Accel (Section 7.2 of datasheet)
    if (!icm45686WriteIREG(dev, ICM456XX_ACCEL_SRC_CTRL_IREG_ADDR, ICM456XX_SRC_CTRL_AAF_ENABLE_BIT | ICM456XX_SRC_CTRL_INTERP_ENABLE_BIT)) {
        icm45686WriteIREG(dev, ICM456XX_ACCEL_SRC_CTRL_IREG_ADDR, 0);
    }
    // Set the Accel UI LPF bandwidth cut-off to ODR/8 (Section 7.3 of datasheet)
    if (!icm45686WriteIREG(dev, ICM456XX_ACCEL_UI_LPF_CFG_IREG_ADDR, ICM456XX_ACCEL_UI_LPFBW_ODR_DIV_8)) {
        // If LPF configuration fails, fallback to BYPASS
        icm45686WriteIREG(dev, ICM456XX_ACCEL_UI_LPF_CFG_IREG_ADDR, ICM456XX_ACCEL_UI_LPFBW_BYPASS);
    }
    // Setup scale and odr values for gyro
    busWrite(dev, ICM456XX_GYRO_CONFIG0, ICM456XX_GYRO_FS_SEL_2000DPS | config->gyroConfigValues[1]);
    // Per datasheet Table 9-6: 35ms minimum startup time
    delay(ICM456XX_GYRO_STARTUP_TIME_MS);
    
    busWrite(dev, ICM456XX_INT1_CONFIG2, ICM456XX_INT1_MODE_PULSED | ICM456XX_INT1_DRIVE_CIRCUIT_PP |
                                            ICM456XX_INT1_POLARITY_ACTIVE_HIGH);
    busWrite(dev, ICM456XX_INT1_CONFIG0, ICM456XX_INT1_STATUS_EN_DRDY);

    delay(ICM456XX_INT_CONFIG_DELAY_MS);
    busSetSpeed(dev, BUS_SPEED_FAST);
}

static bool icm45686DeviceDetect(busDevice_t * dev)
{
    uint8_t tmp = 0xFF;
    uint8_t attemptsRemaining = 5;
    uint32_t waitedMs = 0;

    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);
    // ICM-45686 does not use bank switching (register 0x75 is reserved)
    // Perform soft reset directly
    // Soft reset
    busWrite(dev, ICM456XX_REG_MISC2, ICM456XX_SOFT_RESET);
    // Poll until soft reset completes (SOFT_RESET bit clears) per datasheet Section 9.4
    do {
        busRead(dev, ICM456XX_REG_MISC2, &tmp);
        if (!(tmp & ICM456XX_SOFT_RESET)) {
            break;
        }
        delay(1);
        waitedMs++;
    } while (waitedMs < 20);

    if (tmp & ICM456XX_SOFT_RESET) {
        return false;
    }
    // Initialize power management to a known state after reset
    // This ensures sensors are off and ready for proper initialization
    busWrite(dev, ICM456XX_PWR_MGMT0, 0x00);

    do {
        delay(150);
        busRead(dev, ICM456XX_WHO_AM_REGISTER, &tmp);
        if (tmp == ICM45686_WHO_AM_I_CONST) {
            return true;
        }
    } while (attemptsRemaining--);

    return false;
}

bool icm45686AccDetect(accDev_t *acc)
{
    acc->busDev = busDeviceOpen(BUSTYPE_ANY, DEVHW_ICM45686, acc->imuSensorToUse);
    if (acc->busDev == NULL) {
        return false;
    }

    mpuContextData_t * ctx = busDeviceGetScratchpadMemory(acc->busDev);
    if (ctx->chipMagicNumber != 0x4265) {
        return false;
    }

    acc->initFn = icm45686AccInit;
    acc->readFn = icm45686AccRead;
    acc->accAlign = acc->busDev->param;

    return true;
}

bool icm45686GyroDetect(gyroDev_t *gyro)
{
    gyro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_ICM45686, gyro->imuSensorToUse, OWNER_MPU);
    if (gyro->busDev == NULL) {
        return false;
    }

    if (!icm45686DeviceDetect(gyro->busDev)) {
        busDeviceDeInit(gyro->busDev);
        return false;
    }

    // Magic number for ACC detection to indicate that we have detected icm45686 gyro
    mpuContextData_t * ctx = busDeviceGetScratchpadMemory(gyro->busDev);
    ctx->chipMagicNumber = 0x4265;

    gyro->initFn = icm45686AccAndGyroInit;
    gyro->readFn = icm45686GyroRead;
    gyro->intStatusFn = gyroCheckDataReady;
    gyro->temperatureFn = icm45686ReadTemperature;
    gyro->scale = 1.0f / 16.4f;     // 16.4 dps/lsb scalefactor
    gyro->gyroAlign = gyro->busDev->param;

    return true;
}


#endif

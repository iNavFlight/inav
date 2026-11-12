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

// Regression test for the LSM6DSL/LSM6DSO/LSM6DS3 attitude-drift bug reported on Discord
// (custom STM32F405 + LSM6DSL board, INAV 9.1.0): roll/pitch drifted for ~10-20s after
// boot because lsm6dxxConfig() enabled the chip's on-chip gyro high-pass filter at its
// slowest (16mHz) cutoff, which has a ~10s RC time constant and keeps settling the raw
// gyro output for ~20-30s after every power-on/reset - well past INAV's ~2s startup
// gyro calibration window. This test drives the real, unmodified production entry point
// (lsm6dGyroDetect() -> gyro->initFn() -> lsm6dxxConfig()) against a fake register-map
// "chip" and asserts the byte actually written to CTRL7_G does not enable the on-chip HPF.

#include <cstring>
#include <cstdint>

extern "C" {
#include "platform.h"

#include "common/utils.h"
#include "drivers/time.h"
#include "drivers/resource.h"
#include "drivers/bus.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_mpu.h"
#include "drivers/accgyro/accgyro_lsm6dxx.h"
}

#include "gtest/gtest.h"

extern "C" {

bool lsm6dGyroDetect(gyroDev_t *gyro);

// --- Fake "chip": a plain byte-addressed register map, backing busRead()/busWrite() ---
static uint8_t fakeRegs[256];
static busDevice_t fakeBusDevice;
static uint32_t fakeScratchpad[(BUS_SCRATCHPAD_MEMORY_SIZE + 3) / 4];

busDevice_t * busDeviceInit(busType_e bus, devHardwareType_e hw, uint8_t tag, resourceOwner_e owner)
{
    UNUSED(bus); UNUSED(hw); UNUSED(tag); UNUSED(owner);
    return &fakeBusDevice;
}

busDevice_t * busDeviceOpen(busType_e bus, devHardwareType_e hw, uint8_t tag)
{
    UNUSED(bus); UNUSED(hw); UNUSED(tag);
    return &fakeBusDevice;
}

void busDeviceDeInit(busDevice_t *dev)
{
    UNUSED(dev);
}

void * busDeviceGetScratchpadMemory(const busDevice_t *dev)
{
    UNUSED(dev);
    return fakeScratchpad;
}

void busSetSpeed(const busDevice_t *dev, busSpeed_e speed)
{
    UNUSED(dev); UNUSED(speed);
}

bool busWrite(const busDevice_t *dev, uint8_t reg, uint8_t data)
{
    UNUSED(dev);
    fakeRegs[reg] = data;
    return true;
}

bool busRead(const busDevice_t *dev, uint8_t reg, uint8_t *data)
{
    UNUSED(dev);
    *data = fakeRegs[reg];
    return true;
}

bool busWriteBuf(const busDevice_t *dev, uint8_t reg, const uint8_t *data, uint8_t length)
{
    UNUSED(dev);
    for (uint8_t i = 0; i < length; i++) {
        fakeRegs[reg + i] = data[i];
    }
    return true;
}

bool busReadBuf(const busDevice_t *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
    UNUSED(dev);
    for (uint8_t i = 0; i < length; i++) {
        data[i] = fakeRegs[reg + i];
    }
    return true;
}

void delay(timeMs_t ms)
{
    UNUSED(ms);
}

void delayMicroseconds(timeUs_t us)
{
    UNUSED(us);
}

// Real mpuChooseGyroConfig() only needs to pick a table row for boards sharing the MPU
// family; it's irrelevant to the CTRL7_G behavior under test, so return a fixed, valid
// config regardless of input.
const gyroFilterAndRateConfig_t * mpuChooseGyroConfig(uint8_t desiredLpf, uint16_t desiredRateHz)
{
    UNUSED(desiredLpf); UNUSED(desiredRateHz);
    static const gyroFilterAndRateConfig_t fakeConfig = { 0, 1000, { 0, 0 } };
    return &fakeConfig;
}

bool gyroCheckDataReady(gyroDev_t *gyro)
{
    UNUSED(gyro);
    return true;
}

} // extern "C"

class LSM6DxxConfigTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        memset(fakeRegs, 0, sizeof(fakeRegs));
        memset(&fakeBusDevice, 0, sizeof(fakeBusDevice));
        memset(&gyro, 0, sizeof(gyro));
        gyro.lpf = GYRO_HARDWARE_LPF_NORMAL;
        gyro.requestedSampleIntervalUs = 1000;
    }

    gyroDev_t gyro;
};

// Drives the real production path for the LSM6DSL chip (the exact chip reported in the
// bug: attitude drift over ~10-20s caused by raw gyro X stepping after calibration closed).
TEST_F(LSM6DxxConfigTest, LSM6DSLDoesNotEnableOnChipGyroHighPassFilter)
{
    fakeRegs[LSM6DXX_REG_WHO_AM_I] = 0x6A; // LSM6DSL_CHIP_ID

    ASSERT_TRUE(lsm6dGyroDetect(&gyro));
    ASSERT_NE(gyro.initFn, nullptr);

    gyro.initFn(&gyro); // -> lsm6dxxSpiGyroInit() -> lsm6dxxConfig()

    const uint8_t ctrl7g = fakeRegs[LSM6DXX_REG_CTRL7_G];
    EXPECT_EQ(ctrl7g & LSM6DXX_MASK_CTRL7_G, 0)
        << "CTRL7_G HPF bits were written as 0x" << std::hex << (int)(ctrl7g & LSM6DXX_MASK_CTRL7_G)
        << " - the on-chip gyro high-pass filter must stay disabled. Its slowest cutoff has a "
        << "~10s RC time constant, so re-enabling it reproduces the reported ~10-20s attitude "
        << "drift as the chip's own filter output keeps settling well past INAV's startup "
        << "gyro calibration window.";
    EXPECT_EQ(ctrl7g & LSM6DXX_VAL_CTRL7_G_HP_EN_G, 0) << "on-chip gyro HPF enable bit must not be set";
}

// Same assertion for LSM6DSO, which shares lsm6dxxConfig()'s legacy (non-"Gen V") path.
TEST_F(LSM6DxxConfigTest, LSM6DSODoesNotEnableOnChipGyroHighPassFilter)
{
    fakeRegs[LSM6DXX_REG_WHO_AM_I] = 0x6C; // LSM6DSO_CHIP_ID

    ASSERT_TRUE(lsm6dGyroDetect(&gyro));
    ASSERT_NE(gyro.initFn, nullptr);

    gyro.initFn(&gyro);

    const uint8_t ctrl7g = fakeRegs[LSM6DXX_REG_CTRL7_G];
    EXPECT_EQ(ctrl7g & LSM6DXX_MASK_CTRL7_G, 0);
    EXPECT_EQ(ctrl7g & LSM6DXX_VAL_CTRL7_G_HP_EN_G, 0);
}

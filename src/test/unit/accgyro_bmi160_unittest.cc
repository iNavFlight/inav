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

// Checks that the real accgyro_bmi160.c detect functions apply the target's IMU_BMI160_ALIGN
// (the bus device "param") to both the gyro and the accelerometer, so the two stay aligned.

#include <cstring>
#include <cstdint>

extern "C" {
#include "platform.h"

#include "common/utils.h"
#include "common/axis.h"
#include "drivers/time.h"
#include "drivers/system.h"
#include "drivers/resource.h"
#include "drivers/bus.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_bmi160.h"
}

#include "gtest/gtest.h"

extern "C" {

static busDevice_t fakeBusDevice;
static uint32_t fakeScratchpad[(BUS_SCRATCHPAD_MEMORY_SIZE + 3) / 4];

busDevice_t * busDeviceInit(busType_e, devHardwareType_e, uint8_t, resourceOwner_e) { return &fakeBusDevice; }
busDevice_t * busDeviceOpen(busType_e, devHardwareType_e, uint8_t) { return &fakeBusDevice; }
void busDeviceDeInit(busDevice_t *) {}
void * busDeviceGetScratchpadMemory(const busDevice_t *) { return fakeScratchpad; }
void busSetSpeed(const busDevice_t *, busSpeed_e) {}

// Chip ID register (0x00) reads as BMI160's 0xD1
bool busRead(const busDevice_t *, uint8_t, uint8_t *data) { *data = 0xD1; return true; }
bool busWrite(const busDevice_t *, uint8_t, uint8_t) { return true; }
bool busReadBuf(const busDevice_t *, uint8_t, uint8_t *, uint8_t) { return true; }

void delay(timeMs_t) {}
void failureMode(failureMode_e) {}
const gyroFilterAndRateConfig_t * chooseGyroConfig(uint8_t, uint16_t, const gyroFilterAndRateConfig_t *configs, int) { return configs; }
bool gyroCheckDataReady(gyroDev_t *) { return true; }

} // extern "C"

TEST(BMI160AlignTest, AccAlignMatchesTargetParamAndGyroAlign)
{
    memset(&fakeBusDevice, 0, sizeof(fakeBusDevice));
    memset(fakeScratchpad, 0, sizeof(fakeScratchpad));
    fakeBusDevice.param = CW270_DEG;

    gyroDev_t gyro;
    accDev_t acc;
    memset(&gyro, 0, sizeof(gyro));
    memset(&acc, 0, sizeof(acc));
    acc.accAlign = ALIGN_DEFAULT; // what accDetect() presets

    // Gyro detect first: it writes the 0xB160 magic that acc detect looks for.
    ASSERT_TRUE(bmi160GyroDetect(&gyro));
    ASSERT_TRUE(bmi160AccDetect(&acc));

    EXPECT_EQ(gyro.gyroAlign, CW270_DEG);
    EXPECT_EQ(acc.accAlign, CW270_DEG)
        << "bmi160AccDetect() must apply the target IMU_BMI160_ALIGN to acc->accAlign";
    EXPECT_EQ(gyro.gyroAlign, acc.accAlign) << "gyro and accelerometer alignment must agree";
}

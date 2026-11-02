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

#include "gtest/gtest.h"
#include "unittest_macros.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

extern "C" {

#include "common/time.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/serial.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "io/gps.h"
#include "io/gps_private.h"

extern gpsReceiverData_t gpsState;
extern gpsSolutionData_t gpsSolDRV;
extern gpsSolutionData_t gpsSol;
extern gpsConfig_t gpsConfig_System;

const uint32_t baudRates[] = { 0, 1200, 2400, 4800, 9600, 19200, 38400,
    57600, 115200, 230400, 250000, 460800, 921600,
    1000000, 1500000, 2000000, 2470000 };

serialConfig_t serialConfig_System;

uint32_t millis(void) { return 5000; }
uint32_t micros(void) { return 5000000; }

static uint32_t fakeFeatureMask = 0;

bool feature(uint32_t mask)      { return (fakeFeatureMask & mask) != 0; }
void featureSet(uint32_t mask)   { fakeFeatureMask |=  mask; }
void featureClear(uint32_t mask) { fakeFeatureMask &= ~mask; }

static uint32_t fakeSensorMask = 0;

bool sensors(uint32_t mask)      { return (fakeSensorMask & mask) != 0; }
void sensorsSet(uint32_t mask)   { fakeSensorMask |=  mask; }
void sensorsClear(uint32_t mask) { fakeSensorMask &= ~mask; }

uint32_t serialRxBytesWaiting(const serialPort_t *instance) { UNUSED(instance); return 0; }
uint32_t serialTxBytesFree(const serialPort_t *instance) { UNUSED(instance); return 256; }
bool isSerialTransmitBufferEmpty(const serialPort_t *instance) { UNUSED(instance); return true; }
uint8_t serialRead(serialPort_t *instance) { UNUSED(instance); return 0; }
void serialWrite(serialPort_t *instance, uint8_t ch) { UNUSED(instance); UNUSED(ch); }
void serialWriteBuf(serialPort_t *instance, const uint8_t *data, int count) { UNUSED(instance); UNUSED(data); UNUSED(count); }
void serialPrint(serialPort_t *instance, const char *str) { UNUSED(instance); UNUSED(str); }
void serialSetMode(serialPort_t *instance, portMode_t mode) { UNUSED(instance); UNUSED(mode); }
void serialSetBaudRate(serialPort_t *instance, uint32_t baudRate) { UNUSED(instance); UNUSED(baudRate); }

serialPort_t *openSerialPort(serialPortIdentifier_e identifier, serialPortFunction_e function,
                             serialReceiveCallbackPtr rxCallback, void *rxCallbackData,
                             uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    UNUSED(identifier); UNUSED(function); UNUSED(rxCallback);
    UNUSED(rxCallbackData); UNUSED(baudRate); UNUSED(mode); UNUSED(options);
    return NULL;
}

void closeSerialPort(serialPort_t *serialPort) { UNUSED(serialPort); }
serialPortConfig_t *findSerialPortConfig(serialPortFunction_e function) { UNUSED(function); return NULL; }
void waitForSerialPortToFinishTransmitting(serialPort_t *serialPort) { UNUSED(serialPort); }

uint32_t stateFlags  = 0;
uint32_t armingFlags = 0;

bool isMPUSoftReset(void) { return false; }

void LED0_ON(void)     {}
void LED0_OFF(void)    {}
void LED1_ON(void)     {}
void LED1_OFF(void)    {}
void LED1_TOGGLE(void) {}

void onNewGPSData(void) {}

bool rtcHasTime(void) { return false; }
bool rtcSetDateTime(dateTime_t *dt) { UNUSED(dt); return true; }

bool baroIsHealthy(void)  { return false; }
bool pitotIsHealthy(void) { return false; }

void gpsRestartUBLOX(void) {}
void gpsHandleUBLOX(void) {}

} // extern "C"

class GpsHeartbeatTest : public ::testing::Test {
protected:
    void SetUp() override {
        memset(&gpsSol,    0, sizeof(gpsSol));
        memset(&gpsSolDRV, 0, sizeof(gpsSolDRV));
        memset(&gpsConfig_System, 0, sizeof(gpsConfig_System));
        gpsConfig_System.provider = GPS_UBLOX;
        gpsState.gpsConfig     = &gpsConfig_System;
        gpsState.baseTimeoutMs = 1000;
        stateFlags  = 0;
        armingFlags = 0;
    }

    void TearDown() override {
        fakeFeatureMask = 0;
        fakeSensorMask  = 0;
    }
};

TEST_F(GpsHeartbeatTest, HeartbeatTogglesOncePerSolution)
{
    bool prev = gpsSol.flags.gpsHeartbeat;
    for (int i = 0; i < 4; i++) {
        gpsProcessNewDriverData();
        gpsProcessNewSolutionData(false);
        EXPECT_NE(prev, gpsSol.flags.gpsHeartbeat);
        prev = gpsSol.flags.gpsHeartbeat;
    }
}

// MSP/CRSF/DroneCAN copy in another task than the toggle
TEST_F(GpsHeartbeatTest, DriverCopyKeepsHeartbeat)
{
    gpsProcessNewDriverData();
    gpsProcessNewSolutionData(false);
    const bool before = gpsSol.flags.gpsHeartbeat;
    gpsProcessNewDriverData();
    EXPECT_EQ(before, gpsSol.flags.gpsHeartbeat);
}

/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify this software
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * INAV is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 */

#include "gtest/gtest.h"

extern "C" {
    #include <stdbool.h>
    #include <stdint.h>
    #include <string.h>

    #include "build/debug.h"
    #include "common/time.h"
    #include "config/feature.h"
    #include "drivers/ina226.h"
    #include "fc/config.h"
    #include "fc/runtime_config.h"
    #include "flight/mixer.h"
    #include "io/beeper.h"
    #include "navigation/navigation.h"
    #include "navigation/navigation_private.h"
    #include "rx/rx.h"
    #include "sensors/battery.h"
}

#define INA226_REG_SHUNT_VOLTAGE     0x01
#define INA226_REG_BUS_VOLTAGE       0x02
#define INA226_REG_MANUFACTURER_ID   0xFE
#define INA226_REG_DIE_ID            0xFF

static busDevice_t fakeBusDevice;
static uint16_t fakeRegisters[256];
static bool fakeRegisterReadable[256];
static uint32_t fakeFeatures;
static timeUs_t fakeTimeUs;

extern "C" {
    void pgResetFn_batteryProfiles(batteryProfile_t *instance);
    extern const batteryMetersConfig_t pgResetTemplate_batteryMetersConfig;
}

static void setFakeRegister(uint8_t reg, uint16_t value)
{
    fakeRegisters[reg] = value;
    fakeRegisterReadable[reg] = true;
}

static void resetFakeBus(void)
{
    memset(&fakeBusDevice, 0, sizeof(fakeBusDevice));
    memset(fakeRegisters, 0, sizeof(fakeRegisters));
    memset(fakeRegisterReadable, 0, sizeof(fakeRegisterReadable));

    fakeBusDevice.busType = BUSTYPE_I2C;
    setFakeRegister(INA226_REG_MANUFACTURER_ID, 0x5449);
    setFakeRegister(INA226_REG_DIE_ID, 0x2260);
}

static void clearFakeRegister(uint8_t reg)
{
    fakeRegisterReadable[reg] = false;
}

static void settleVoltage(void)
{
    for (int i = 0; i < 5; i++) {
        batteryUpdate(10000000);
    }
}

static void settleCurrent(void)
{
    for (int i = 0; i < 5; i++) {
        currentMeterUpdate(10000000);
    }
}

static void resetBatteryTestState(void)
{
    fakeFeatures = FEATURE_VBAT | FEATURE_CURRENT_METER;
    fakeTimeUs = 0;
    memset(&systemConfig_System, 0, sizeof(systemConfig_System));
    memset(&navConfig_System, 0, sizeof(navConfig_System));

    pgResetFn_batteryProfiles(*batteryProfiles_array());
    *batteryMetersConfigMutable() = pgResetTemplate_batteryMetersConfig;
    currentBatteryProfile = batteryProfiles(0);

    batteryMetersConfigMutable()->voltage.type = VOLTAGE_SENSOR_INA226;
    batteryMetersConfigMutable()->current.type = CURRENT_SENSOR_INA226;
    batteryMetersConfigMutable()->ina226.shuntResistanceMicroOhm = 2000;
    batteryMetersConfigMutable()->voltageSource = BAT_VOLTAGE_RAW;

    currentBatteryProfileMutable->cells = 0;
    currentBatteryProfileMutable->voltage.cellDetect = 425;
    currentBatteryProfileMutable->voltage.cellMax = 420;
    currentBatteryProfileMutable->voltage.cellWarning = 350;
    currentBatteryProfileMutable->voltage.cellMin = 330;

    resetFakeBus();
    batteryInit();

    clearFakeRegister(INA226_REG_BUS_VOLTAGE);
    clearFakeRegister(INA226_REG_SHUNT_VOLTAGE);
    settleVoltage();
    settleCurrent();
}

extern "C" {
    uint32_t armingFlags = 0;
    int16_t rcCommand[4] = {};
    navigationPosControl_t posControl = {};
    int32_t debug[DEBUG32_VALUE_COUNT] = {};
    uint8_t debugMode = 0;
    systemConfig_t systemConfig_System = {};
    systemConfig_t systemConfig_Copy = {};
    navConfig_t navConfig_System = {};
    navConfig_t navConfig_Copy = {};

    busDevice_t *busDeviceInit(busType_e bus, devHardwareType_e hw, uint8_t tag, resourceOwner_e owner)
    {
        EXPECT_EQ(BUSTYPE_I2C, bus);
        EXPECT_EQ(DEVHW_INA226, hw);
        EXPECT_EQ(0, tag);
        EXPECT_EQ(OWNER_CURRENT_METER, owner);
        return &fakeBusDevice;
    }

    void busDeviceDeInit(busDevice_t *dev)
    {
        EXPECT_EQ(&fakeBusDevice, dev);
    }

    bool busReadBuf(const busDevice_t *busdev, uint8_t reg, uint8_t *data, uint8_t length)
    {
        EXPECT_EQ(&fakeBusDevice, busdev);
        EXPECT_EQ(2, length);

        if (length != 2 || !fakeRegisterReadable[reg]) {
            return false;
        }

        data[0] = fakeRegisters[reg] >> 8;
        data[1] = fakeRegisters[reg] & 0xFF;
        return true;
    }

    uint16_t adcGetChannel(uint8_t)
    {
        return 0;
    }

    bool feature(uint32_t mask)
    {
        return fakeFeatures & mask;
    }

    timeUs_t micros(void)
    {
        fakeTimeUs += 1000;
        return fakeTimeUs;
    }

    bool failsafeIsActive(void)
    {
        return false;
    }

    void beeper(beeperMode_e)
    {
    }

    bool setConfigProfile(uint8_t)
    {
        return true;
    }

    navigationFSMStateFlags_t navGetCurrentStateFlags(void)
    {
        return (navigationFSMStateFlags_t)0;
    }

    bool throttleStickIsLow(void)
    {
        return true;
    }

    bool ifMotorstopFeatureEnabled(void)
    {
        return false;
    }

    float getFlightTime(void)
    {
        return 1.0f;
    }

    uint32_t getFlyingEnergy(void)
    {
        return 0;
    }

    uint32_t getTotalTravelDistance(void)
    {
        return 1;
    }
}

TEST(BatteryINA226, UpdatesBatteryVoltageFromIna226BusVoltage)
{
    resetBatteryTestState();
    setFakeRegister(INA226_REG_BUS_VOLTAGE, 9600);

    settleVoltage();

    EXPECT_EQ(1200, getBatteryRawVoltage());
}

TEST(BatteryINA226, UpdatesCurrentFromIna226ShuntVoltage)
{
    resetBatteryTestState();
    setFakeRegister(INA226_REG_SHUNT_VOLTAGE, 8000);

    settleCurrent();

    EXPECT_NEAR(1000, getAmperage(), 2);
}

TEST(BatteryINA226, UsesConfiguredMicroOhmShuntForCurrent)
{
    resetBatteryTestState();
    batteryMetersConfigMutable()->ina226.shuntResistanceMicroOhm = 300;
    setFakeRegister(INA226_REG_SHUNT_VOLTAGE, 1200);

    settleCurrent();

    EXPECT_NEAR(1000, getAmperage(), 2);
}

TEST(BatteryINA226, ClearsVoltageAndCurrentOnReadFailure)
{
    resetBatteryTestState();
    setFakeRegister(INA226_REG_BUS_VOLTAGE, 9600);
    setFakeRegister(INA226_REG_SHUNT_VOLTAGE, 8000);

    settleVoltage();
    settleCurrent();
    EXPECT_EQ(1200, getBatteryRawVoltage());
    EXPECT_NEAR(1000, getAmperage(), 2);

    clearFakeRegister(INA226_REG_BUS_VOLTAGE);
    clearFakeRegister(INA226_REG_SHUNT_VOLTAGE);

    settleVoltage();
    settleCurrent();

    EXPECT_EQ(0, getBatteryRawVoltage());
    EXPECT_EQ(0, getAmperage());
}

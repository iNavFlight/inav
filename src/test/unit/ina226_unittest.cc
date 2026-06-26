/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify this software
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * INAV is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 */

#include "gtest/gtest.h"

extern "C" {
    #include <stdbool.h>
    #include <stdint.h>
    #include <string.h>

    #include "drivers/ina226.h"
}

#define INA226_REG_SHUNT_VOLTAGE     0x01
#define INA226_REG_BUS_VOLTAGE       0x02
#define INA226_REG_MANUFACTURER_ID   0xFE
#define INA226_REG_DIE_ID            0xFF

static busDevice_t fakeBusDevice;
static uint16_t fakeRegisters[256];
static bool fakeRegisterReadable[256];
static bool fakeInitSucceeds;
static bool fakeDeInitCalled;

static void resetFakeBus(void)
{
    memset(&fakeBusDevice, 0, sizeof(fakeBusDevice));
    memset(fakeRegisters, 0, sizeof(fakeRegisters));
    memset(fakeRegisterReadable, 0, sizeof(fakeRegisterReadable));

    fakeBusDevice.busType = BUSTYPE_I2C;
    fakeInitSucceeds = true;
    fakeDeInitCalled = false;
}

static void setFakeRegister(uint8_t reg, uint16_t value)
{
    fakeRegisters[reg] = value;
    fakeRegisterReadable[reg] = true;
}

extern "C" {
    busDevice_t *busDeviceInit(busType_e bus, devHardwareType_e hw, uint8_t tag, resourceOwner_e owner)
    {
        EXPECT_EQ(BUSTYPE_I2C, bus);
        EXPECT_EQ(DEVHW_INA226, hw);
        EXPECT_EQ(0, tag);
        EXPECT_EQ(OWNER_CURRENT_METER, owner);

        return fakeInitSucceeds ? &fakeBusDevice : nullptr;
    }

    void busDeviceDeInit(busDevice_t *dev)
    {
        EXPECT_EQ(&fakeBusDevice, dev);
        fakeDeInitCalled = true;
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
}

TEST(INA226, DetectsExpectedIds)
{
    resetFakeBus();
    setFakeRegister(INA226_REG_MANUFACTURER_ID, 0x5449);
    setFakeRegister(INA226_REG_DIE_ID, 0x2260);

    ina226Dev_t dev = {};
    EXPECT_TRUE(ina226Init(&dev));
    EXPECT_EQ(&fakeBusDevice, dev.busDev);
    EXPECT_FALSE(fakeDeInitCalled);
}

TEST(INA226, RejectsUnexpectedIds)
{
    resetFakeBus();
    setFakeRegister(INA226_REG_MANUFACTURER_ID, 0x1234);
    setFakeRegister(INA226_REG_DIE_ID, 0x2260);

    ina226Dev_t dev = {};
    EXPECT_FALSE(ina226Init(&dev));
    EXPECT_EQ(nullptr, dev.busDev);
    EXPECT_TRUE(fakeDeInitCalled);
}

TEST(INA226, RejectsReadFailureDuringDetection)
{
    resetFakeBus();
    setFakeRegister(INA226_REG_MANUFACTURER_ID, 0x5449);

    ina226Dev_t dev = {};
    EXPECT_FALSE(ina226Init(&dev));
    EXPECT_EQ(nullptr, dev.busDev);
    EXPECT_TRUE(fakeDeInitCalled);
}

TEST(INA226, ConvertsBusVoltage)
{
    EXPECT_EQ(0, ina226BusVoltageToCentivolts(0));
    EXPECT_EQ(1200, ina226BusVoltageToCentivolts(9600));
    EXPECT_EQ(583, ina226BusVoltageToCentivolts(0x1234));
}

TEST(INA226, ReadsBusVoltageBigEndian)
{
    resetFakeBus();
    setFakeRegister(INA226_REG_BUS_VOLTAGE, 9600);

    ina226Dev_t dev = {};
    dev.busDev = &fakeBusDevice;
    uint16_t centiVolts = 0;

    EXPECT_TRUE(ina226ReadBusVoltage(&dev, &centiVolts));
    EXPECT_EQ(1200, centiVolts);
}

TEST(INA226, ConvertsShuntVoltageToCurrent)
{
    EXPECT_EQ(1000, ina226ShuntVoltageToCentiamps(8000, 2000));
    EXPECT_EQ(-1000, ina226ShuntVoltageToCentiamps(-8000, 2000));
    EXPECT_EQ(0, ina226ShuntVoltageToCentiamps(8000, 0));
}

TEST(INA226, ReadsNegativeShuntCurrentBigEndian)
{
    resetFakeBus();
    setFakeRegister(INA226_REG_SHUNT_VOLTAGE, 0xE0C0);

    ina226Dev_t dev = {};
    dev.busDev = &fakeBusDevice;
    int16_t centiAmps = 0;

    EXPECT_TRUE(ina226ReadShuntCurrent(&dev, 2000, &centiAmps));
    EXPECT_EQ(-1000, centiAmps);
}

TEST(INA226, ReportsReadFailures)
{
    resetFakeBus();

    ina226Dev_t dev = {};
    dev.busDev = &fakeBusDevice;
    uint16_t centiVolts = 1;
    int16_t centiAmps = 1;

    EXPECT_FALSE(ina226ReadBusVoltage(&dev, &centiVolts));
    EXPECT_FALSE(ina226ReadShuntCurrent(&dev, 2000, &centiAmps));
}

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

// Fake non-blocking bus: one transfer in flight at a time. busReadBufStart() queues it, busIsBusy() reports
// it busy for one poll and then completes it into the caller's buffer, or fails it when the register is
// unreadable. Like the real I2C driver, the failure stays reported until the next transfer is started.
static bool fakeTransferPending;
static bool fakeLastTransferFailed;
static bool fakeBusTaken;               // another device holds the bus, starts are refused
static uint8_t fakeTransferReg;
static uint8_t *fakeTransferData;
static int fakeBusyPolls;
static int fakeTransferStarts;

static void resetFakeBus(void)
{
    memset(&fakeBusDevice, 0, sizeof(fakeBusDevice));
    memset(fakeRegisters, 0, sizeof(fakeRegisters));
    memset(fakeRegisterReadable, 0, sizeof(fakeRegisterReadable));

    fakeBusDevice.busType = BUSTYPE_I2C;
    fakeInitSucceeds = true;
    fakeDeInitCalled = false;

    fakeTransferPending = false;
    fakeLastTransferFailed = false;
    fakeBusTaken = false;
    fakeTransferReg = 0;
    fakeTransferData = nullptr;
    fakeBusyPolls = 0;
    fakeTransferStarts = 0;
}

static void setFakeRegister(uint8_t reg, uint16_t value)
{
    fakeRegisters[reg] = value;
    fakeRegisterReadable[reg] = true;
}

static void pumpUpdates(ina226Dev_t *dev, int calls)
{
    for (int i = 0; i < calls; i++) {
        ina226Update(dev);
    }
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

    bool busReadBufStart(const busDevice_t *busdev, uint8_t reg, uint8_t *data, uint8_t length)
    {
        EXPECT_EQ(&fakeBusDevice, busdev);
        EXPECT_EQ(2, length);

        if (fakeTransferPending || fakeBusTaken) {
            return false;
        }

        fakeTransferPending = true;
        fakeLastTransferFailed = false;
        fakeTransferReg = reg;
        fakeTransferData = data;
        fakeBusyPolls = 1;
        fakeTransferStarts++;
        return true;
    }

    bool busIsBusy(const busDevice_t *busdev, bool *error)
    {
        EXPECT_EQ(&fakeBusDevice, busdev);

        if (fakeTransferPending && fakeBusyPolls-- > 0) {
            *error = false;
            return true;
        }

        if (fakeTransferPending) {
            fakeTransferPending = false;
            if (fakeRegisterReadable[fakeTransferReg]) {
                fakeTransferData[0] = fakeRegisters[fakeTransferReg] >> 8;
                fakeTransferData[1] = fakeRegisters[fakeTransferReg] & 0xFF;
            } else {
                fakeLastTransferFailed = true;
            }
        }

        *error = fakeLastTransferFailed;
        return false;
    }
}

TEST(INA226, DetectsExpectedIds)
{
    resetFakeBus();
    setFakeRegister(INA226_REG_MANUFACTURER_ID, 0x5449);
    setFakeRegister(INA226_REG_DIE_ID, 0x2260);

    ina226Dev_t dev = {};
    EXPECT_TRUE(ina226Init(&dev, 2, 0x45));
    EXPECT_EQ(&fakeBusDevice, dev.busDev);
    EXPECT_EQ(I2CDEV_2, dev.busDev->busdev.i2c.i2cBus);
    EXPECT_EQ(0x45, dev.busDev->busdev.i2c.address);
    EXPECT_FALSE(fakeDeInitCalled);
}

TEST(INA226, RejectsUnexpectedIds)
{
    resetFakeBus();
    setFakeRegister(INA226_REG_MANUFACTURER_ID, 0x1234);
    setFakeRegister(INA226_REG_DIE_ID, 0x2260);

    ina226Dev_t dev = {};
    EXPECT_FALSE(ina226Init(&dev, 1, INA226_DEFAULT_I2C_ADDRESS));
    EXPECT_EQ(nullptr, dev.busDev);
    EXPECT_TRUE(fakeDeInitCalled);
}

TEST(INA226, RejectsReadFailureDuringDetection)
{
    resetFakeBus();
    setFakeRegister(INA226_REG_MANUFACTURER_ID, 0x5449);

    ina226Dev_t dev = {};
    EXPECT_FALSE(ina226Init(&dev, 1, INA226_DEFAULT_I2C_ADDRESS));
    EXPECT_EQ(nullptr, dev.busDev);
    EXPECT_TRUE(fakeDeInitCalled);
}

TEST(INA226, SeedsReadingsSynchronouslyAtInit)
{
    resetFakeBus();
    setFakeRegister(INA226_REG_MANUFACTURER_ID, 0x5449);
    setFakeRegister(INA226_REG_DIE_ID, 0x2260);
    setFakeRegister(INA226_REG_BUS_VOLTAGE, 9600);
    setFakeRegister(INA226_REG_SHUNT_VOLTAGE, 8000);

    ina226Dev_t dev = {};
    ASSERT_TRUE(ina226Init(&dev, 1, INA226_DEFAULT_I2C_ADDRESS));

    // Both readings are available before the background sampling has run
    uint16_t centiVolts = 0;
    int16_t centiAmps = 0;
    EXPECT_EQ(0, fakeTransferStarts);
    EXPECT_TRUE(ina226ReadBusVoltage(&dev, &centiVolts));
    EXPECT_EQ(1200, centiVolts);
    EXPECT_TRUE(ina226ReadShuntCurrent(&dev, 2000, &centiAmps));
    EXPECT_EQ(1000, centiAmps);
}

TEST(INA226, ConvertsBusVoltage)
{
    EXPECT_EQ(0, ina226BusVoltageToCentivolts(0));
    EXPECT_EQ(1200, ina226BusVoltageToCentivolts(9600));
    EXPECT_EQ(583, ina226BusVoltageToCentivolts(0x1234));
}

TEST(INA226, RejectsInvalidOrDisabledI2cConfiguration)
{
    resetFakeBus();

    ina226Dev_t dev = {};
    EXPECT_FALSE(ina226Init(&dev, 0, INA226_DEFAULT_I2C_ADDRESS));
    EXPECT_FALSE(ina226Init(&dev, 3, INA226_DEFAULT_I2C_ADDRESS));
    EXPECT_FALSE(ina226Init(&dev, 1, 0x3F));
    EXPECT_FALSE(ina226Init(&dev, 1, 0x50));
}

TEST(INA226, ReadsBusVoltageOverNonBlockingTransfer)
{
    resetFakeBus();
    setFakeRegister(INA226_REG_BUS_VOLTAGE, 9600);

    ina226Dev_t dev = {};
    dev.busDev = &fakeBusDevice;
    uint16_t centiVolts = 0;

    // First update starts the bus voltage read, nothing is available yet
    ina226Update(&dev);
    EXPECT_EQ(1, fakeTransferStarts);
    EXPECT_EQ(INA226_REG_BUS_VOLTAGE, fakeTransferReg);
    EXPECT_FALSE(ina226ReadBusVoltage(&dev, &centiVolts));

    // Still on the bus: no second transfer may be started
    ina226Update(&dev);
    EXPECT_EQ(1, fakeTransferStarts);
    EXPECT_FALSE(ina226ReadBusVoltage(&dev, &centiVolts));

    // Completed: parsed big endian, and the rotation moves on to the shunt register
    ina226Update(&dev);
    EXPECT_TRUE(ina226ReadBusVoltage(&dev, &centiVolts));
    EXPECT_EQ(1200, centiVolts);
    EXPECT_EQ(2, fakeTransferStarts);
    EXPECT_EQ(INA226_REG_SHUNT_VOLTAGE, fakeTransferReg);
}

TEST(INA226, ReadsNegativeShuntCurrentOverNonBlockingTransfer)
{
    resetFakeBus();
    setFakeRegister(INA226_REG_SHUNT_VOLTAGE, 0xE0C0);

    ina226Dev_t dev = {};
    dev.busDev = &fakeBusDevice;
    dev.lastReg = INA226_REG_BUS_VOLTAGE;    // shunt voltage is next in the rotation
    int16_t centiAmps = 0;

    ina226Update(&dev);     // shunt read started
    EXPECT_EQ(INA226_REG_SHUNT_VOLTAGE, fakeTransferReg);

    pumpUpdates(&dev, 2);   // busy, complete
    EXPECT_TRUE(ina226ReadShuntCurrent(&dev, 2000, &centiAmps));
    EXPECT_EQ(-1000, centiAmps);
}

TEST(INA226, ConvertsShuntVoltageToCurrent)
{
    EXPECT_EQ(1000, ina226ShuntVoltageToCentiamps(8000, 2000));
    EXPECT_EQ(-1000, ina226ShuntVoltageToCentiamps(-8000, 2000));
    EXPECT_EQ(0, ina226ShuntVoltageToCentiamps(8000, 0));
}

TEST(INA226, SupportsFullUint32ShuntResistanceRange)
{
    EXPECT_EQ(0, ina226ShuntVoltageToCentiamps(INT16_MAX, UINT32_MAX));
    EXPECT_EQ(0, ina226ShuntVoltageToCentiamps(INT16_MIN, UINT32_MAX));
}

TEST(INA226, SaturatesCurrentToInt16Range)
{
    EXPECT_EQ(INT16_MAX, ina226ShuntVoltageToCentiamps(INT16_MAX, 1));
    EXPECT_EQ(INT16_MIN, ina226ShuntVoltageToCentiamps(INT16_MIN, 1));
}

TEST(INA226, FailedTransferInvalidatesReading)
{
    resetFakeBus();
    setFakeRegister(INA226_REG_BUS_VOLTAGE, 9600);

    ina226Dev_t dev = {};
    dev.busDev = &fakeBusDevice;
    uint16_t centiVolts = 0;
    int16_t centiAmps = 0;

    pumpUpdates(&dev, 3);   // bus voltage read completes, shunt read starts
    ASSERT_TRUE(ina226ReadBusVoltage(&dev, &centiVolts));
    EXPECT_FALSE(ina226ReadShuntCurrent(&dev, 2000, &centiAmps));   // never read successfully

    // The device stops answering: the next bus voltage read fails and the stale value is dropped
    fakeRegisterReadable[INA226_REG_BUS_VOLTAGE] = false;
    pumpUpdates(&dev, 4);   // shunt read fails, bus voltage read starts, busy, fails (and the shunt read starts again)
    EXPECT_EQ(4, fakeTransferStarts);
    EXPECT_FALSE(ina226ReadBusVoltage(&dev, &centiVolts));
    EXPECT_FALSE(ina226ReadShuntCurrent(&dev, 2000, &centiAmps));

    // Readings come back once the device answers again
    fakeRegisterReadable[INA226_REG_BUS_VOLTAGE] = true;
    pumpUpdates(&dev, 4);   // shunt read fails, bus voltage read starts, busy, completes
    EXPECT_TRUE(ina226ReadBusVoltage(&dev, &centiVolts));
    EXPECT_EQ(1200, centiVolts);
}

TEST(INA226, RetriesWhenBusStartIsRefused)
{
    resetFakeBus();
    setFakeRegister(INA226_REG_BUS_VOLTAGE, 9600);

    ina226Dev_t dev = {};
    dev.busDev = &fakeBusDevice;
    uint16_t centiVolts = 0;

    // Another device holds the bus: nothing is started and nothing is pending
    fakeBusTaken = true;
    pumpUpdates(&dev, 2);
    EXPECT_EQ(0, fakeTransferStarts);
    EXPECT_FALSE(ina226ReadBusVoltage(&dev, &centiVolts));

    // The read starts on the next call once the bus is free
    fakeBusTaken = false;
    pumpUpdates(&dev, 3);
    EXPECT_EQ(2, fakeTransferStarts);
    EXPECT_TRUE(ina226ReadBusVoltage(&dev, &centiVolts));
    EXPECT_EQ(1200, centiVolts);
}

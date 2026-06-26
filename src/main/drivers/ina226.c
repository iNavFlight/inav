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

#include <stddef.h>
#include <stdint.h>

#include "platform.h"

#if defined(USE_INA226)

#include "drivers/ina226.h"

#define INA226_REG_SHUNT_VOLTAGE     0x01
#define INA226_REG_BUS_VOLTAGE       0x02
#define INA226_REG_MANUFACTURER_ID   0xFE
#define INA226_REG_DIE_ID            0xFF

#define INA226_MANUFACTURER_ID       0x5449
#define INA226_DIE_ID                0x2260
#define INA226_DIE_ID_MASK           0xFFF0

static bool ina226ReadRegister(ina226Dev_t *dev, uint8_t reg, uint16_t *value)
{
    if (!dev || !dev->busDev || !value) {
        return false;
    }

    uint8_t buffer[2];
    if (!busReadBuf(dev->busDev, reg, buffer, sizeof(buffer))) {
        return false;
    }

    *value = ((uint16_t)buffer[0] << 8) | buffer[1];
    return true;
}

uint16_t ina226BusVoltageToCentivolts(uint16_t rawBusVoltage)
{
    // INA226 bus voltage LSB is 1.25mV: raw * 125 / 1000 = centivolts.
    return ((uint32_t)rawBusVoltage * 125 + 500) / 1000;
}

int16_t ina226ShuntVoltageToCentiamps(int16_t rawShuntVoltage, uint32_t shuntMicroOhm)
{
    if (shuntMicroOhm == 0) {
        return 0;
    }

    const int32_t centiAmps = (int32_t)rawShuntVoltage * 250 / (int32_t)shuntMicroOhm;

    if (centiAmps > INT16_MAX) {
        return INT16_MAX;
    }

    if (centiAmps < INT16_MIN) {
        return INT16_MIN;
    }

    return centiAmps;
}

bool ina226Detect(ina226Dev_t *dev)
{
    uint16_t manufacturerId;
    uint16_t dieId;

    return ina226ReadRegister(dev, INA226_REG_MANUFACTURER_ID, &manufacturerId)
        && ina226ReadRegister(dev, INA226_REG_DIE_ID, &dieId)
        && manufacturerId == INA226_MANUFACTURER_ID
        && (dieId & INA226_DIE_ID_MASK) == INA226_DIE_ID;
}

bool ina226Init(ina226Dev_t *dev)
{
    if (!dev) {
        return false;
    }

    dev->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_INA226, 0, OWNER_CURRENT_METER);

    if (!dev->busDev) {
        return false;
    }

    if (!ina226Detect(dev)) {
        busDeviceDeInit(dev->busDev);
        dev->busDev = NULL;
        return false;
    }

    return true;
}

bool ina226ReadBusVoltage(ina226Dev_t *dev, uint16_t *centiVolts)
{
    uint16_t rawBusVoltage;

    if (!centiVolts || !ina226ReadRegister(dev, INA226_REG_BUS_VOLTAGE, &rawBusVoltage)) {
        return false;
    }

    *centiVolts = ina226BusVoltageToCentivolts(rawBusVoltage);
    return true;
}

bool ina226ReadShuntCurrent(ina226Dev_t *dev, uint32_t shuntMicroOhm, int16_t *centiAmps)
{
    uint16_t rawShuntVoltage;

    if (!centiAmps || !ina226ReadRegister(dev, INA226_REG_SHUNT_VOLTAGE, &rawShuntVoltage)) {
        return false;
    }

    *centiAmps = ina226ShuntVoltageToCentiamps((int16_t)rawShuntVoltage, shuntMicroOhm);
    return true;
}

#endif

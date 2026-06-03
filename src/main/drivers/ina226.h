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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "drivers/bus.h"

#define INA226_DEFAULT_I2C_ADDRESS 0x40

typedef struct ina226Dev_s {
    busDevice_t *busDev;
} ina226Dev_t;

bool ina226Init(ina226Dev_t *dev);
bool ina226Detect(ina226Dev_t *dev);
bool ina226ReadBusVoltage(ina226Dev_t *dev, uint16_t *centiVolts);
bool ina226ReadShuntCurrent(ina226Dev_t *dev, uint32_t shuntMicroOhm, int16_t *centiAmps);

uint16_t ina226BusVoltageToCentivolts(uint16_t rawBusVoltage);
int16_t ina226ShuntVoltageToCentiamps(int16_t rawShuntVoltage, uint32_t shuntMicroOhm);

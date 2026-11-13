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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#ifdef USE_SMBUS_BATT

/*
 * SMBus Smart Battery driver (e.g. TI BQ40Z50).
 *
 * The SMBus protocol runs on top of the standard I2C peripheral. It adds a
 * CRC-8 (PEC) byte covering the whole transaction (address bytes included)
 * and limits the clock to 100kHz (handled by the I2C speed override in
 * fc_init.c when a SMBus meter type is selected).
 *
 * Code structure follows barometer.c (init + detect + state-machine update)
 * and reuses the I2C bus device abstraction (busReadBuf/busWrite) rather than
 * calling i2cRead directly.
 */

/* SMBus Smart Battery command addresses */
#define SMBUS_BATT_CMD_TEMPERATURE          0x08    /* 0.1 K                */
#define SMBUS_BATT_CMD_VOLTAGE             0x09    /* mV                   */
#define SMBUS_BATT_CMD_CURRENT             0x0A    /* mA, signed           */
#define SMBUS_BATT_CMD_RELATIVE_SOC        0x0D    /* %                    */
#define SMBUS_BATT_CMD_REMAINING_CAPACITY  0x0F    /* mAh                  */
#define SMBUS_BATT_CMD_FULL_CHARGE_CAP     0x10    /* mAh                  */
#define SMBUS_BATT_CMD_RUN_TIME_TO_EMPTY   0x11    /* min                  */
#define SMBUS_BATT_CMD_AVERAGE_TIME_EMPTY  0x12    /* min                  */
#define SMBUS_BATT_CMD_CYCLE_COUNT         0x17    /* cycles               */
#define SMBUS_BATT_CMD_RELATIVE_SOH        0x4F    /* %                    */

/* Kelvin -> Celsius offset for 0.1K resolution data (273.15K = 2731 in 0.1K) */
#define SMBUS_BATT_KELVIN_TO_CELSIUS_0p1   2731

/* Initialise the bus device and probe for a smart battery.
 * Returns false if no device responds (non-fatal, getters will report invalid). */
bool smbusBattInit(void);

/* State-machine driven update. Reads one parameter per call so the battery
 * task is never blocked. Returns the delay (us) until the next call, or 0
 * to keep the default task period. */
uint32_t smbusBattUpdate(void);

/* True when either voltage or current meter type is set to SMBUS. */
bool isSmbusBatteryEnabled(void);

/* True when a battery has been detected on the bus and comms are OK. */
bool smbusBattIsDetected(void);

/* Effective scale multiplier applied to current/capacity values
 * (batteryMetersConfig()->smbusScaleFactor, user-configured). */
uint16_t smbusBattGetScaleFactor(void);

/* Getters. Each returns false if the value is not currently valid.
 * Current and capacity values are already un-scaled by the BQ40Z80 Scale
 * Factor, so wider types are used (a scaled pack can exceed 16 bits). */
bool smbusBattGetVoltage(uint16_t *mV);
bool smbusBattGetCurrent(int32_t *mA);
bool smbusBattGetPercent(uint8_t *percent);
bool smbusBattGetRemainingCapacity(uint32_t *mAh);
bool smbusBattGetFullChargeCapacity(uint32_t *mAh);
bool smbusBattGetTemperature(int16_t *deciDegC);   /* 0.1 C */
bool smbusBattGetDischargeTimeMin(uint16_t *min);
bool smbusBattGetCycleCount(uint16_t *cycles);
bool smbusBattGetHealth(uint8_t *percent);

#endif  // USE_SMBUS_BATT

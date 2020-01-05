/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdbool.h"
#include "stdint.h"
#include <string.h>

#include "platform.h"

#include "common/maths.h"
#include "common/printf.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/1-wire.h"
#include "drivers/temperature/temperature.h"
#include "drivers/temperature/lm75.h"
#include "drivers/temperature/ds18b20.h"

#include "fc/runtime_config.h"

#include "sensors/sensors.h"
#include "sensors/temperature.h"
#include "sensors/gyro.h"
#include "sensors/barometer.h"

#include "scheduler/protothreads.h"


PG_REGISTER_ARRAY(tempSensorConfig_t, MAX_TEMP_SENSORS, tempSensorConfig, PG_TEMP_SENSOR_CONFIG, 2);

#define MPU_TEMP_VALID_BIT 0
#define BARO_TEMP_VALID_BIT 1
#define MPU_TEMP_VALID (mpuBaroTempValid & (1 << MPU_TEMP_VALID_BIT))
#define BARO_TEMP_VALID (mpuBaroTempValid & (1 << BARO_TEMP_VALID_BIT))

static uint16_t mpuTemperature, baroTemperature;
static uint8_t mpuBaroTempValid = 0;

#ifdef USE_TEMPERATURE_SENSOR
static int16_t  tempSensorValue[MAX_TEMP_SENSORS];

// Each bit corresponds to a sensor LSB = first sensor. 1 = valid value
static uint8_t sensorStatus[MAX_TEMP_SENSORS / 8 + (MAX_TEMP_SENSORS % 8 ? 1 : 0)];

#ifdef USE_TEMPERATURE_LM75
static temperatureDev_t lm75Dev[8];
#endif

#ifdef DS18B20_DRIVER_AVAILABLE
static owDev_t *owDev;
#endif

static void newSensorCheckAndEnter(uint8_t type, uint64_t addr)
{
    int8_t foundConfigIndex = -1, firstFreeConfigSlot = -1;

    // try to find sensor or free config slot
    for (uint8_t configIndex = 0; configIndex < MAX_TEMP_SENSORS; ++configIndex) {

        const tempSensorConfig_t *configSlot = tempSensorConfig(configIndex);

        if ((configSlot->type == type) && (configSlot->address == addr)) {
            foundConfigIndex = configIndex;
            break;
        }

        if ((firstFreeConfigSlot == -1) && !configSlot->type) firstFreeConfigSlot = configIndex;

    }

    // if new sensor and have free config slot enter new sensor
    if ((foundConfigIndex == -1) && (firstFreeConfigSlot != -1)) {
        tempSensorConfig_t *configSlot = tempSensorConfigMutable(firstFreeConfigSlot);
        configSlot->type = type;
        configSlot->address = addr;
        configSlot->osdSymbol = 0;
        configSlot->label[0] = '\0';
        configSlot->alarm_min = -200;
        configSlot->alarm_max = 600;
    }
}

void temperatureInit(void)
{
    memset(sensorStatus, 0, sizeof(sensorStatus) * sizeof(*sensorStatus));

    sensorsSet(SENSOR_TEMP);

#ifdef USE_TEMPERATURE_LM75
    memset(lm75Dev, 0, sizeof(lm75Dev));
    for (uint8_t lm75Addr = 0; lm75Addr < 8; ++lm75Addr) {
        if (lm75Detect(lm75Dev + lm75Addr, lm75Addr))
            newSensorCheckAndEnter(TEMP_SENSOR_LM75, lm75Addr);
    }
#endif

#ifdef DS18B20_DRIVER_AVAILABLE
    owDev = getOwDev();
    if (owDev) {
        uint64_t ds18b20_rom_table[MAX_TEMP_SENSORS];
        uint8_t ds18b20_rom_count = MAX_TEMP_SENSORS;
        ds18b20Enumerate(owDev, ds18b20_rom_table, &ds18b20_rom_count);

        for (uint8_t rom_index = 0; rom_index < ds18b20_rom_count; ++rom_index)
            newSensorCheckAndEnter(TEMP_SENSOR_DS18B20, ds18b20_rom_table[rom_index]);
    }
#endif

    // configure sensors
    for (uint8_t configIndex = 0; configIndex < MAX_TEMP_SENSORS; ++configIndex) {
        const tempSensorConfig_t *configSlot = tempSensorConfig(configIndex);

        switch (configSlot->type) {
#ifdef DS18B20_DRIVER_AVAILABLE
            case TEMP_SENSOR_DS18B20:
                if (owDev) {
                    tempSensorValue[configIndex] = TEMPERATURE_INVALID_VALUE;
                    ds18b20Configure(owDev, configSlot->address, DS18B20_CONFIG_9BIT);
                }
                break;
#endif

            default:;
        }
    }
}

static bool temperatureSensorValueIsValid(uint8_t temperatureUpdateSensorIndex)
{
    uint8_t mask = 1 << (temperatureUpdateSensorIndex % 8);
    uint8_t byteIndex = temperatureUpdateSensorIndex / 8;
    return sensorStatus[byteIndex] & mask;
}

// returns decidegrees centigrade
bool getSensorTemperature(uint8_t temperatureUpdateSensorIndex, int16_t *temperature)
{
    *temperature = tempSensorValue[temperatureUpdateSensorIndex];
    return temperatureSensorValueIsValid(temperatureUpdateSensorIndex);
}

// Converts 64bit integer address to hex format
// hex_address must be at least 17 bytes long (16 chars + NULL)
void tempSensorAddressToString(uint64_t address, char *hex_address)
{
    if (address < 8)
        tfp_sprintf(hex_address, "%d", (int)address);
    else {
        uint32_t *address32 = (uint32_t *)&address;
        tfp_sprintf(hex_address, "%08lx%08lx", address32[1], address32[0]);
    }
}

// Converts address string in hex format to unsigned integer
// the hex_address parameter must be NULL or space terminated
bool tempSensorStringToAddress(const char *hex_address, uint64_t *address)
{
    uint16_t char_count = 0;
    *address = 0;
    while (*hex_address && (*hex_address != ' ')) {
        if (++char_count > 16) return false;
        char byte = *hex_address++;
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
        else return false;
        *address = (*address << 4) | (byte & 0xF);
    }
    return true;
}
#endif /* USE_TEMPERATURE_SENSOR */

#ifdef USE_TEMPERATURE_SENSOR

static uint8_t temperatureUpdateSensorIndex;
static bool temperatureUpdateValueValid;

#ifdef DS18B20_DRIVER_AVAILABLE
static uint8_t temperatureUpdateIndex;
static uint8_t temperatureUpdateBuf[9];
#endif

#endif /* defined(USE_TEMPERATURE_SENSOR) */

PROTOTHREAD(temperatureUpdate)
{
    ptBegin(temperatureUpdate);

    while (1) {

        if (gyroReadTemperature()) {
            mpuTemperature = gyroGetTemperature();
            mpuBaroTempValid |= (1 << MPU_TEMP_VALID_BIT);
        } else
            mpuBaroTempValid &= ~(1 << MPU_TEMP_VALID_BIT);

        #ifdef USE_BARO
        if (sensors(SENSOR_BARO)) {
            baroTemperature = baroGetTemperature();
            mpuBaroTempValid |= (1 << BARO_TEMP_VALID_BIT);
        } else
            mpuBaroTempValid &= ~(1 << BARO_TEMP_VALID_BIT);
        #endif

#ifdef USE_TEMPERATURE_SENSOR

        temperatureUpdateSensorIndex = 0;
        do {
            const tempSensorConfig_t *configSlot = tempSensorConfig(temperatureUpdateSensorIndex);
            temperatureUpdateValueValid = false;

#ifdef USE_TEMPERATURE_LM75
            if (configSlot->type == TEMP_SENSOR_LM75) {
                if (configSlot->address < 8) {
                    temperatureDev_t *dev = lm75Dev + configSlot->address;
                    if (dev->read && dev->read(dev, &tempSensorValue[temperatureUpdateSensorIndex])) temperatureUpdateValueValid = true;
                }
            }
#endif

#ifdef DS18B20_DRIVER_AVAILABLE
            if ((configSlot->type == TEMP_SENSOR_DS18B20) && owDev) {
                bool ack = owDev->owResetCommand(owDev);
                if (!ack) goto temperatureUpdateError;
                ptWait(owDev->owBusReady(owDev));

                ack = owDev->owMatchRomCommand(owDev);
                if (!ack) goto temperatureUpdateError;
                ptWait(owDev->owBusReady(owDev));

                temperatureUpdateIndex = 0;
                do {
                    ack = owDev->owWriteByteCommand(owDev, ((uint8_t *)&tempSensorConfig(temperatureUpdateSensorIndex)->address)[temperatureUpdateIndex]);
                    if (!ack) goto temperatureUpdateError;
                    ptWait(owDev->owBusReady(owDev));
                } while (++temperatureUpdateIndex < 8);

                ack = ds18b20ReadScratchpadCommand(owDev);
                if (!ack) goto temperatureUpdateError;
                ptWait(owDev->owBusReady(owDev));

                temperatureUpdateIndex = 0;
                do {
                    ack = owDev->owReadByteCommand(owDev);
                    if (!ack) goto temperatureUpdateError;
                    ptWait(owDev->owBusReady(owDev));
                    ack = owDev->owReadByteResult(owDev, temperatureUpdateBuf + temperatureUpdateIndex);
                    if (!ack) goto temperatureUpdateError;
                } while (++temperatureUpdateIndex < 9);

                int16_t temperature;
                if (ds18b20ReadTemperatureFromScratchPadBuf(temperatureUpdateBuf, &temperature)) {
                    if (temperatureSensorValueIsValid(temperatureUpdateSensorIndex) || (tempSensorValue[temperatureUpdateSensorIndex] == -1240)) {
                        tempSensorValue[temperatureUpdateSensorIndex] = temperature;
                        temperatureUpdateValueValid = true;
                    } else
                        tempSensorValue[temperatureUpdateSensorIndex] = -1240;
                } else
                    tempSensorValue[temperatureUpdateSensorIndex] = TEMPERATURE_INVALID_VALUE;
            }

temperatureUpdateError:;
#endif

           uint8_t statusMask = 1 << (temperatureUpdateSensorIndex % 8);
           uint8_t byteIndex = temperatureUpdateSensorIndex / 8;
           if (temperatureUpdateValueValid)
               sensorStatus[byteIndex] |= statusMask;
           else
               sensorStatus[byteIndex] &= ~statusMask;

           ptYield();

        } while (++temperatureUpdateSensorIndex < MAX_TEMP_SENSORS);

#ifdef DS18B20_DRIVER_AVAILABLE
        if (owDev) {
            bool ack = owDev->owResetCommand(owDev);
            if (!ack) goto ds18b20StartConversionError;
            ptWait(owDev->owBusReady(owDev));

            ack = owDev->owSkipRomCommand(owDev);
            if (!ack) goto ds18b20StartConversionError;
            ptWait(owDev->owBusReady(owDev));

            ds18b20StartConversionCommand(owDev);
        }

ds18b20StartConversionError:;
#endif

#endif /* defined(USE_TEMPERATURE_SENSOR) */

        ptDelayMs(100); // DS18B20 sensors take 94ms for a temperature conversion with 9bit resolution

    }

    ptEnd(0);
}

// returns decidegrees centigrade
bool getIMUTemperature(int16_t *temperature)
{
    *temperature = mpuTemperature;
    return MPU_TEMP_VALID;
}

// returns decidegrees centigrade
bool getBaroTemperature(int16_t *temperature)
{
    *temperature = baroTemperature;
    return BARO_TEMP_VALID;
}

void resetTempSensorConfig(void)
{
    memset(tempSensorConfigMutable(0), 0, sizeof(tempSensorConfig_t) * MAX_TEMP_SENSORS);
}

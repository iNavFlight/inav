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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#ifdef USE_SMBUS_BATT

#include "build/build_config.h"

#include "drivers/bus.h"
#include "drivers/resource.h"
#include "drivers/time.h"

#include "common/crc.h"

#include "sensors/battery.h"
#include "sensors/battery_config_structs.h"
#include "sensors/smbus.h"

/* Set to 0 to skip PEC verification (for devices that don't support it). BQ40Z50 enables PEC by default. */
#ifndef SMBUS_BATT_VERIFY_PEC
#define SMBUS_BATT_VERIFY_PEC 1
#endif

#define SMBUS_BATT_UPDATE_INTERVAL_US   10000   /* 10ms between successive parameter reads */

/* Backoff when the battery comms line is unplugged: every read would time out
 * (~ms each), so after this many consecutive failures fall back to one probe
 * read per interval. Hot-plugging the line is picked up within the interval. */
#define SMBUS_BATT_BACKOFF_FAILURES     18      /* ~2 full rounds of all-9 reads failed */
#define SMBUS_BATT_BACKOFF_RETRY_MS     500
#define SMBUS_BATT_STALE_FAILURES       9       /* one full round failed: comms lost, invalidate held values */

typedef struct smbusBattData_s {
    uint16_t voltage;              /* mV                                 */
    int32_t  current;              /* mA, signed, scale factor applied   */
    uint8_t  percent;              /* %                                  */
    uint32_t remainingCapacity;    /* mAh, scale factor applied          */
    uint32_t fullChargeCapacity;   /* mAh, scale factor applied          */
    int16_t  temperature;          /* 0.1 C                              */
    uint16_t runTimeToEmpty;       /* min                                */
    uint16_t cycleCount;           /* cycles                             */
    uint8_t  health;               /* %                                  */
    bool voltageValid;
    bool currentValid;
    bool percentValid;
    bool remainingValid;
    bool fullChargeValid;
    bool temperatureValid;
    bool runTimeValid;
    bool cycleValid;
    bool healthValid;
} smbusBattData_t;

static busDevice_t *smbusBusDev = NULL;
static smbusBattData_t smbusData;
static uint8_t consecutiveFailures = 0;

/* PEC covers the whole transaction including the address bytes (with R/W bit).
 * i2cRead applies the 7->8 bit shift internally, so we use address<<1 here. */
static bool smbusReadWord(busDevice_t *busdev, uint8_t cmd, uint16_t *value)
{
#if SMBUS_BATT_VERIFY_PEC
    uint8_t buf[3];
    if (!busReadBuf(busdev, cmd, buf, sizeof(buf))) {
        return false;
    }
    const uint8_t addr8w = busdev->busdev.i2c.address << 1;
    const uint8_t pecBuf[5] = { addr8w, cmd, (uint8_t)(addr8w | 1), buf[0], buf[1] };
    if (crc8_update(0, pecBuf, sizeof(pecBuf)) != buf[2]) {
        return false;   /* PEC mismatch */
    }
    *value = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
    return true;
#else
    uint8_t buf[2];
    if (!busReadBuf(busdev, cmd, buf, sizeof(buf))) {
        return false;
    }
    *value = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
    return true;
#endif
}

bool smbusBattInit(void)
{
    memset(&smbusData, 0, sizeof(smbusData));

    /* A SMBus Smart Battery (BQ40Z50) needs no initialisation: it is a bus-powered
     * slave with its own gauge MCU, always ready to serve reads. Just grab the
     * bus handle - smbusBattUpdate() starts polling right away, so a battery
     * connected after boot is picked up automatically (hot-plug). */
    smbusBusDev = busDeviceInit(BUSTYPE_I2C, DEVHW_SMBUS_BATT, 0, OWNER_SMBUS_BATT);

    return smbusBusDev != NULL;
}

bool isSmbusBatteryEnabled(void)
{
    /* current.type is always compiled in; voltage.type only exists when USE_ADC is on */
    if (batteryMetersConfig()->current.type == CURRENT_SENSOR_SMBUS) {
        return true;
    }
#ifdef USE_ADC
    if (batteryMetersConfig()->voltage.type == VOLTAGE_SENSOR_SMBUS) {
        return true;
    }
#endif
    return false;
}

bool smbusBattIsDetected(void)
{
    return consecutiveFailures < SMBUS_BATT_BACKOFF_FAILURES;
}

uint16_t smbusBattGetScaleFactor(void)
{
    return batteryMetersConfig()->smbusScaleFactor;
}

/* State machine: read one parameter per call so the battery task is never blocked.
 * Pattern borrowed from barometer.c::baroUpdate(). */
typedef enum {
    SMBUS_STATE_VOLTAGE = 0,
    SMBUS_STATE_CURRENT,
    SMBUS_STATE_PERCENT,
    SMBUS_STATE_REMAINING,
    SMBUS_STATE_FULL_CHARGE,
    SMBUS_STATE_TEMPERATURE,
    SMBUS_STATE_RUN_TIME,
    SMBUS_STATE_CYCLES,
    SMBUS_STATE_HEALTH,
    SMBUS_STATE_COUNT
} smbusState_e;

static smbusState_e smbusState = SMBUS_STATE_VOLTAGE;

uint32_t smbusBattUpdate(void)
{
    if (smbusBusDev == NULL) {
        return 0;   /* target has no SMBus device registered */
    }

    /* Backoff: with the comms line unplugged every read times out (~ms each).
     * Don't hammer the bus - probe a single read per backoff interval. */
    static timeMs_t lastBackoffReadMs = 0;
    if (consecutiveFailures >= SMBUS_BATT_BACKOFF_FAILURES) {
        const timeMs_t nowMs = millis();
        if (nowMs - lastBackoffReadMs < SMBUS_BATT_BACKOFF_RETRY_MS) {
            return 0;
        }
        lastBackoffReadMs = nowMs;
    }

    uint16_t word = 0;
    bool readOk = false;

    switch (smbusState) {
        case SMBUS_STATE_VOLTAGE:
            readOk = smbusReadWord(smbusBusDev, SMBUS_BATT_CMD_VOLTAGE, &word);
            if (readOk) {
                smbusData.voltage = word;
            }
            smbusData.voltageValid |= readOk;
            smbusState = SMBUS_STATE_CURRENT;
            break;

        case SMBUS_STATE_CURRENT:
            readOk = smbusReadWord(smbusBusDev, SMBUS_BATT_CMD_CURRENT, &word);
            if (readOk) {
                /* SMBus: positive = discharge. Undo the gauge scale factor
                 * (user-configured, see batteryMetersConfig()->smbusScaleFactor);
                 * int32 to survive 16-bit overflow after scaling. */
                smbusData.current = (int32_t)(int16_t)word * batteryMetersConfig()->smbusScaleFactor;
            }
            smbusData.currentValid |= readOk;
            smbusState = SMBUS_STATE_PERCENT;
            break;

        case SMBUS_STATE_PERCENT:
            readOk = smbusReadWord(smbusBusDev, SMBUS_BATT_CMD_RELATIVE_SOC, &word);
            if (readOk) {
                smbusData.percent = (uint8_t)(word <= 100 ? word : 100);
            }
            smbusData.percentValid |= readOk;
            smbusState = SMBUS_STATE_REMAINING;
            break;

        case SMBUS_STATE_REMAINING:
            readOk = smbusReadWord(smbusBusDev, SMBUS_BATT_CMD_REMAINING_CAPACITY, &word);
            if (readOk) {
                smbusData.remainingCapacity = (uint32_t)word * batteryMetersConfig()->smbusScaleFactor;
            }
            smbusData.remainingValid |= readOk;
            smbusState = SMBUS_STATE_FULL_CHARGE;
            break;

        case SMBUS_STATE_FULL_CHARGE:
            readOk = smbusReadWord(smbusBusDev, SMBUS_BATT_CMD_FULL_CHARGE_CAP, &word);
            if (readOk) {
                smbusData.fullChargeCapacity = (uint32_t)word * batteryMetersConfig()->smbusScaleFactor;
            }
            smbusData.fullChargeValid |= readOk;
            smbusState = SMBUS_STATE_TEMPERATURE;
            break;

        case SMBUS_STATE_TEMPERATURE:
            readOk = smbusReadWord(smbusBusDev, SMBUS_BATT_CMD_TEMPERATURE, &word);
            if (readOk) {
                /* 0.1 K -> 0.1 C (273.15 K offset, integer 2731) */
                smbusData.temperature = (int16_t)word - SMBUS_BATT_KELVIN_TO_CELSIUS_0p1;
            }
            smbusData.temperatureValid |= readOk;
            smbusState = SMBUS_STATE_RUN_TIME;
            break;

        case SMBUS_STATE_RUN_TIME:
            readOk = smbusReadWord(smbusBusDev, SMBUS_BATT_CMD_RUN_TIME_TO_EMPTY, &word);
            if (readOk) {
                smbusData.runTimeToEmpty = word;     /* 0xFFFF = not discharging */
            }
            smbusData.runTimeValid |= readOk;
            smbusState = SMBUS_STATE_CYCLES;
            break;

        case SMBUS_STATE_CYCLES:
            readOk = smbusReadWord(smbusBusDev, SMBUS_BATT_CMD_CYCLE_COUNT, &word);
            if (readOk) {
                smbusData.cycleCount = word;
            }
            smbusData.cycleValid |= readOk;
            smbusState = SMBUS_STATE_HEALTH;
            break;

        case SMBUS_STATE_HEALTH:
            readOk = smbusReadWord(smbusBusDev, SMBUS_BATT_CMD_RELATIVE_SOH, &word);
            if (readOk) {
                smbusData.health = (uint8_t)(word <= 100 ? word : 100);
            }
            smbusData.healthValid |= readOk;
            smbusState = SMBUS_STATE_VOLTAGE;    /* wrap around */
            break;

        default:
            smbusState = SMBUS_STATE_VOLTAGE;
            readOk = true;    /* nothing was read - don't count as a failure */
            break;
    }

    if (readOk) {
        consecutiveFailures = 0;
    } else {
        if (consecutiveFailures < 0xFF) {
            consecutiveFailures++;
        }
        /* A full round of failed reads means comms are lost - drop all held
         * values so consumers see invalid data instead of stale numbers.
         * A single transient failure keeps the last good value (stale-hold). */
        if (consecutiveFailures >= SMBUS_BATT_STALE_FAILURES) {
            smbusData.voltageValid = false;
            smbusData.currentValid = false;
            smbusData.percentValid = false;
            smbusData.remainingValid = false;
            smbusData.fullChargeValid = false;
            smbusData.temperatureValid = false;
            smbusData.runTimeValid = false;
            smbusData.cycleValid = false;
            smbusData.healthValid = false;
        }
    }

    return SMBUS_BATT_UPDATE_INTERVAL_US;
}

bool smbusBattGetVoltage(uint16_t *mV)
{
    if (!smbusData.voltageValid) return false;
    *mV = smbusData.voltage;
    return true;
}

bool smbusBattGetCurrent(int32_t *mA)
{
    if (!smbusData.currentValid) return false;
    *mA = smbusData.current;
    return true;
}

bool smbusBattGetPercent(uint8_t *percent)
{
    if (!smbusData.percentValid) return false;
    *percent = smbusData.percent;
    return true;
}

bool smbusBattGetRemainingCapacity(uint32_t *mAh)
{
    if (!smbusData.remainingValid) return false;
    *mAh = smbusData.remainingCapacity;
    return true;
}

bool smbusBattGetFullChargeCapacity(uint32_t *mAh)
{
    if (!smbusData.fullChargeValid) return false;
    *mAh = smbusData.fullChargeCapacity;
    return true;
}

bool smbusBattGetTemperature(int16_t *deciDegC)
{
    if (!smbusData.temperatureValid) return false;
    *deciDegC = smbusData.temperature;
    return true;
}

bool smbusBattGetDischargeTimeMin(uint16_t *min)
{
    if (!smbusData.runTimeValid) return false;
    *min = smbusData.runTimeToEmpty;
    return true;
}

bool smbusBattGetCycleCount(uint16_t *cycles)
{
    if (!smbusData.cycleValid) return false;
    *cycles = smbusData.cycleCount;
    return true;
}

bool smbusBattGetHealth(uint8_t *percent)
{
    if (!smbusData.healthValid) return false;
    *percent = smbusData.health;
    return true;
}

#endif  // USE_SMBUS_BATT

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
#include "stdlib.h"

#include "platform.h"

#include "common/maths.h"
#include "common/filter.h"

#include "config/config_reset.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/adc.h"
#include "drivers/time.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "config/feature.h"

#include "sensors/battery.h"

#include "rx/rx.h"

#include "fc/rc_controls.h"

#include "io/beeper.h"

#include "build/debug.h"

#define ADCVREF 3300                 // in mV (3300 = 3.3V)

#define VBATT_CELL_FULL_MAX_DIFF 10  // Max difference with cell max voltage for the battery to be considered full (10mV steps)
#define VBATT_PRESENT_THRESHOLD 100  // Minimum voltage to consider battery present
#define VBATT_STABLE_DELAY 40        // Delay after connecting battery to begin monitoring
#define VBATT_HYSTERESIS 10          // Batt Hysteresis of +/-100mV for changing battery state
#define VBATT_LPF_FREQ  1            // Battery voltage filtering cutoff
#define AMPERAGE_LPF_FREQ  1         // Battery current filtering cutoff


// Battery monitoring stuff
static uint8_t batteryCellCount = 3;       // cell count
static uint16_t batteryFullVoltage;
static uint16_t batteryWarningVoltage;
static uint16_t batteryCriticalVoltage;
static uint32_t batteryRemainingCapacity = 0;
static bool batteryUseCapacityThresholds = false;
static bool batteryFullWhenPluggedIn = false;
static bool profileAutoswitchDisable = false;

static uint16_t vbat = 0;                   // battery voltage in 0.1V steps (filtered)
static uint16_t vbatLatestADC = 0;          // most recent unsmoothed raw reading from vbat ADC
static uint16_t amperageLatestADC = 0;      // most recent raw reading from current ADC
static uint16_t powerSupplyImpedance = 0;   // calculated impedance in milliohm
static uint16_t sagCompensatedVBat = 0;     // calculated no load vbat

static int32_t amperage = 0;               // amperage read by current sensor in centiampere (1/100th A)
static int32_t power = 0;                  // power draw in cW (0.01W resolution)
static int32_t mAhDrawn = 0;               // milliampere hours drawn from the battery since start
static int32_t mWhDrawn = 0;               // energy (milliWatt hours) drawn from the battery since start

batteryState_e batteryState;
const batteryProfile_t *currentBatteryProfile;

PG_REGISTER_ARRAY_WITH_RESET_FN(batteryProfile_t, MAX_BATTERY_PROFILE_COUNT, batteryProfiles, PG_BATTERY_PROFILES, 0);

void pgResetFn_batteryProfiles(batteryProfile_t *instance)
{
    for (int i = 0; i < MAX_BATTERY_PROFILE_COUNT; i++) {
        RESET_CONFIG(batteryProfile_t, &instance[i],
            .cells = 0,

            .voltage = {
                .cellDetect = 430,
                .cellMax = 420,
                .cellMin = 330,
                .cellWarning = 350
            },

            .capacity = {
                .value = 0,
                .warning = 0,
                .critical = 0,
                .unit = BAT_CAPACITY_UNIT_MAH,
            }
        );
    }
}

PG_REGISTER_WITH_RESET_TEMPLATE(batteryMetersConfig_t, batteryMetersConfig, PG_BATTERY_METERS_CONFIG, 0);

PG_RESET_TEMPLATE(batteryMetersConfig_t, batteryMetersConfig,

    .profile_autoswitch = false,

    .voltage_scale = VBAT_SCALE_DEFAULT,

    .current = {
        .type = CURRENT_SENSOR_ADC,
        .scale = CURRENT_METER_SCALE,
        .offset = CURRENT_METER_OFFSET
    },

    .voltageSource = BAT_VOLTAGE_RAW

);

uint16_t batteryAdcToVoltage(uint16_t src)
{
    // calculate battery voltage based on ADC reading
    // result is Vbatt in 0.01V steps. 3.3V = ADC Vref, 0xFFF = 12bit adc, 1100 = 11:1 voltage divider (10k:1k)
    return((uint64_t)src * batteryMetersConfig()->voltage_scale * ADCVREF / (0xFFF * 1000));
}

int32_t currentSensorToCentiamps(uint16_t src)
{
    int32_t microvolts = ((uint32_t)src * ADCVREF * 100) / 0xFFF * 10 - (int32_t)batteryMetersConfig()->current.offset * 1000;
    return microvolts / batteryMetersConfig()->current.scale; // current in 0.01A steps
}

void batteryInit(void)
{
    batteryState = BATTERY_NOT_PRESENT;
    batteryCellCount = 1;
    batteryFullVoltage = 0;
    batteryWarningVoltage = 0;
    batteryCriticalVoltage = 0;
}

// profileDetect() profile sorting compare function
static int profile_compare(profile_comp_t *a, profile_comp_t *b) {
    if (a->max_voltage < b->max_voltage)
        return -1;
    else if (a->max_voltage > b->max_voltage)
        return 1;
    else
        return 0;
}

// Find profile matching plugged battery for profile_autoselect
static int8_t profileDetect() {
    profile_comp_t profile_comp_array[MAX_BATTERY_PROFILE_COUNT];

    // Prepare profile sort
    for (uint8_t i = 0; i < MAX_BATTERY_PROFILE_COUNT; ++i) {
        const batteryProfile_t *profile = batteryProfiles(i);
        profile_comp_array[i].profile_index = i;
        profile_comp_array[i].max_voltage = profile->cells * profile->voltage.cellDetect;
    }

    // Sort profiles by max voltage
    qsort(profile_comp_array, MAX_BATTERY_PROFILE_COUNT, sizeof(*profile_comp_array), (int (*)(const void *, const void *))profile_compare);

    // Return index of the first profile where vbat <= profile_max_voltage
    uint16_t vbatLatest = batteryAdcToVoltage(vbatLatestADC);
    for (uint8_t i = 0; i < MAX_BATTERY_PROFILE_COUNT; ++i)
        if ((profile_comp_array[i].max_voltage > 0) && (vbatLatest <= profile_comp_array[i].max_voltage))
            return profile_comp_array[i].profile_index;

    // No matching profile found
    return -1;
}

void setBatteryProfile(uint8_t profileIndex)
{
    if (profileIndex >= MAX_BATTERY_PROFILE_COUNT) {
        profileIndex = 0;
    }
    currentBatteryProfile = batteryProfiles(profileIndex);
}

void activateBatteryProfile(void)
{
    batteryInit();
}

static void updateBatteryVoltage(timeUs_t timeDelta)
{
    uint16_t vbatSample;
    static pt1Filter_t vbatFilterState;

    // store the battery voltage with some other recent battery voltage readings
    vbatSample = vbatLatestADC = adcGetChannel(ADC_BATTERY);
    vbatSample = pt1FilterApply4(&vbatFilterState, vbatSample, VBATT_LPF_FREQ, timeDelta * 1e-6f);
    vbat = batteryAdcToVoltage(vbatSample);
}

void batteryUpdate(timeUs_t timeDelta)
{
    updateBatteryVoltage(timeDelta);

    /* battery has just been connected*/
    if (batteryState == BATTERY_NOT_PRESENT && vbat > VBATT_PRESENT_THRESHOLD)
    {
        /* Actual battery state is calculated below, this is really BATTERY_PRESENT */
        batteryState = BATTERY_OK;
        /* wait for VBatt to stabilise then we can calc number of cells
        (using the filtered value takes a long time to ramp up)
        We only do this on the ground so don't care if we do block, not
        worse than original code anyway*/
        delay(VBATT_STABLE_DELAY);
        updateBatteryVoltage(timeDelta);

        int8_t detectedProfileIndex = -1;
        if (batteryMetersConfig()->profile_autoswitch && (!profileAutoswitchDisable))
            detectedProfileIndex = profileDetect();

        if (detectedProfileIndex != -1) {
            systemConfigMutable()->current_battery_profile_index = detectedProfileIndex;
            setBatteryProfile(detectedProfileIndex);
            batteryCellCount = currentBatteryProfile->cells;
        } else if (currentBatteryProfile->cells > 0)
            batteryCellCount = currentBatteryProfile->cells;
        else {
            batteryCellCount = (batteryAdcToVoltage(vbatLatestADC) / currentBatteryProfile->voltage.cellDetect) + 1;
            if (batteryCellCount > 8) batteryCellCount = 8; // something is wrong, we expect 8 cells maximum (and autodetection will be problematic at 6+ cells)
        }

        batteryFullVoltage = batteryCellCount * currentBatteryProfile->voltage.cellMax;
        batteryWarningVoltage = batteryCellCount * currentBatteryProfile->voltage.cellWarning;
        batteryCriticalVoltage = batteryCellCount * currentBatteryProfile->voltage.cellMin;

        batteryFullWhenPluggedIn = batteryAdcToVoltage(vbatLatestADC) >= (batteryFullVoltage - batteryCellCount * VBATT_CELL_FULL_MAX_DIFF);
        batteryUseCapacityThresholds = feature(FEATURE_CURRENT_METER) && batteryFullWhenPluggedIn && (currentBatteryProfile->capacity.value > 0) &&
                                           (currentBatteryProfile->capacity.warning > 0) && (currentBatteryProfile->capacity.critical > 0);

    }
    /* battery has been disconnected - can take a while for filter cap to disharge so we use a threshold of VBATT_PRESENT_THRESHOLD */
    else if (batteryState != BATTERY_NOT_PRESENT && vbat <= VBATT_PRESENT_THRESHOLD) {
        batteryState = BATTERY_NOT_PRESENT;
        batteryCellCount = 0;
        batteryWarningVoltage = 0;
        batteryCriticalVoltage = 0;
    }

    if (batteryState != BATTERY_NOT_PRESENT) {

        if ((currentBatteryProfile->capacity.value > 0) && batteryFullWhenPluggedIn) {
            uint32_t capacityDiffBetweenFullAndEmpty = currentBatteryProfile->capacity.value - currentBatteryProfile->capacity.critical;
            int32_t drawn = (currentBatteryProfile->capacity.unit == BAT_CAPACITY_UNIT_MWH ? mWhDrawn : mAhDrawn);
            batteryRemainingCapacity = (drawn > (int32_t)capacityDiffBetweenFullAndEmpty ? 0 : capacityDiffBetweenFullAndEmpty - drawn);
        }

        if (batteryUseCapacityThresholds) {
            if (batteryRemainingCapacity == 0)
                batteryState = BATTERY_CRITICAL;
            else if (batteryRemainingCapacity <= currentBatteryProfile->capacity.warning - currentBatteryProfile->capacity.critical)
                batteryState = BATTERY_WARNING;
        } else {
            uint16_t stateVoltage = getBatteryVoltage();
            switch (batteryState)
            {
                case BATTERY_OK:
                    if (stateVoltage <= (batteryWarningVoltage - VBATT_HYSTERESIS))
                        batteryState = BATTERY_WARNING;
                    break;
                case BATTERY_WARNING:
                    if (stateVoltage <= (batteryCriticalVoltage - VBATT_HYSTERESIS)) {
                        batteryState = BATTERY_CRITICAL;
                    } else if (stateVoltage > (batteryWarningVoltage + VBATT_HYSTERESIS)){
                        batteryState = BATTERY_OK;
                    }
                    break;
                case BATTERY_CRITICAL:
                    if (stateVoltage > (batteryCriticalVoltage + VBATT_HYSTERESIS))
                        batteryState = BATTERY_WARNING;
                    break;
                default:
                    break;
            }
        }

        // handle beeper
        switch (batteryState) {
            case BATTERY_WARNING:
                beeper(BEEPER_BAT_LOW);
                break;
            case BATTERY_CRITICAL:
                beeper(BEEPER_BAT_CRIT_LOW);
                break;
            default:
                break;
        }
    }
}

batteryState_e getBatteryState(void)
{
    return batteryState;
}

bool batteryWasFullWhenPluggedIn(void)
{
    return batteryFullWhenPluggedIn;
}

bool batteryUsesCapacityThresholds(void)
{
    return batteryUseCapacityThresholds;
}

bool isBatteryVoltageConfigured(void)
{
    return feature(FEATURE_VBAT);
}

uint16_t getBatteryVoltage(void)
{
    if (batteryMetersConfig()->voltageSource == BAT_VOLTAGE_SAG_COMP) {
        return sagCompensatedVBat;
    }

    return vbat;
}

uint16_t getBatteryRawVoltage(void)
{
    return vbat;
}

uint16_t getBatterySagCompensatedVoltage(void)
{
    return sagCompensatedVBat;
}

float calculateThrottleCompensationFactor(void)
{
    return batteryFullVoltage / sagCompensatedVBat;
}

uint16_t getBatteryVoltageLatestADC(void)
{
    return vbatLatestADC;
}

uint16_t getBatteryWarningVoltage(void)
{
    return batteryWarningVoltage;
}

uint8_t getBatteryCellCount(void)
{
    return batteryCellCount;
}

uint16_t getBatteryAverageCellVoltage(void)
{
    if (batteryCellCount > 0) {
        return getBatteryVoltage() / batteryCellCount;
    }
    return 0;
}

uint16_t getBatteryRawAverageCellVoltage(void)
{
    if (batteryCellCount > 0) {
        return vbat / batteryCellCount;
    }
    return 0;
}

uint16_t getBatterySagCompensatedAverageCellVoltage(void)
{
    if (batteryCellCount > 0) {
        return sagCompensatedVBat / batteryCellCount;
    }
    return 0;
}

uint32_t getBatteryRemainingCapacity(void)
{
    return batteryRemainingCapacity;
}

bool isAmperageConfigured(void)
{
    return feature(FEATURE_CURRENT_METER) && batteryMetersConfig()->current.type != CURRENT_SENSOR_NONE;
}

int32_t getAmperage(void)
{
    return amperage;
}

int32_t getAmperageLatestADC(void)
{
    return amperageLatestADC;
}

int32_t getPower(void)
{
    return power;
}

int32_t getMAhDrawn(void)
{
    return mAhDrawn;
}

int32_t getMWhDrawn(void)
{
    return mWhDrawn;
}


void currentMeterUpdate(timeUs_t timeDelta)
{
    static pt1Filter_t amperageFilterState;
    static int64_t mAhdrawnRaw = 0;

    switch (batteryMetersConfig()->current.type) {
        case CURRENT_SENSOR_ADC:
            amperageLatestADC = adcGetChannel(ADC_CURRENT);
            amperageLatestADC = pt1FilterApply4(&amperageFilterState, amperageLatestADC, AMPERAGE_LPF_FREQ, timeDelta * 1e-6f);
            amperage = currentSensorToCentiamps(amperageLatestADC);
            break;
        case CURRENT_SENSOR_VIRTUAL:
            amperage = batteryMetersConfig()->current.offset;
            if (ARMING_FLAG(ARMED)) {
                throttleStatus_e throttleStatus = calculateThrottleStatus();
                int32_t throttleOffset = ((throttleStatus == THROTTLE_LOW) && feature(FEATURE_MOTOR_STOP)) ? 0 : (int32_t)rcCommand[THROTTLE] - 1000;
                int32_t throttleFactor = throttleOffset + (throttleOffset * throttleOffset / 50);
                amperage += throttleFactor * batteryMetersConfig()->current.scale / 1000;
            }
            break;
        case CURRENT_SENSOR_NONE:
            amperage = 0;
            break;
    }

    mAhdrawnRaw += (amperage * timeDelta) / 1000;
    mAhDrawn = mAhdrawnRaw / (3600 * 100);
}

void powerMeterUpdate(timeUs_t timeDelta)
{
    static int64_t mWhDrawnRaw = 0;
    power = amperage * vbat / 100; // power unit is cW (0.01W resolution)
    int32_t heatLossesCompensatedPower_mW = amperage * vbat / 10 + sq((int64_t)amperage) * powerSupplyImpedance / 10000;
    mWhDrawnRaw += (int64_t)heatLossesCompensatedPower_mW * timeDelta / 10000;
    mWhDrawn = mWhDrawnRaw / (3600 * 100);
}

void sagCompensatedVBatUpdate(timeUs_t currentTime)
{
    static timeUs_t recordTimestamp = 0;
    static int32_t amperageRecord;
    static uint16_t vbatRecord;

    if (batteryState == BATTERY_NOT_PRESENT) {

        recordTimestamp = 0;
        powerSupplyImpedance = 0;
        sagCompensatedVBat = vbat;

    } else {

        if (cmpTimeUs(currentTime, recordTimestamp) > 20000000)
            recordTimestamp = 0;

        if (!recordTimestamp) {
            amperageRecord = amperage;
            vbatRecord = vbat;
            recordTimestamp = currentTime;
        } else if ((amperage - amperageRecord >= 400) && ((int16_t)vbatRecord - vbat >= 10)) {
            powerSupplyImpedance = (int32_t)(vbatRecord - vbat) * 1000 / (amperage - amperageRecord);
            amperageRecord = amperage;
            vbatRecord = vbat;
            recordTimestamp = currentTime;
        }

        sagCompensatedVBat = MIN(batteryFullVoltage, vbat + powerSupplyImpedance * amperage / 1000);

    }

    DEBUG_SET(DEBUG_SAG_COMP_VOLTAGE, 0, powerSupplyImpedance);
    DEBUG_SET(DEBUG_SAG_COMP_VOLTAGE, 1, sagCompensatedVBat);
}

uint8_t calculateBatteryPercentage(void)
{
    if (batteryState == BATTERY_NOT_PRESENT)
        return 0;

    if (batteryFullWhenPluggedIn && feature(FEATURE_CURRENT_METER) && (currentBatteryProfile->capacity.value > 0) && (currentBatteryProfile->capacity.critical > 0)) {
        uint32_t capacityDiffBetweenFullAndEmpty = currentBatteryProfile->capacity.value - currentBatteryProfile->capacity.critical;
        return constrain(batteryRemainingCapacity * 100 / capacityDiffBetweenFullAndEmpty, 0, 100);
    } else
        return constrain((vbat - batteryCriticalVoltage) * 100L / (batteryFullVoltage - batteryCriticalVoltage), 0, 100);
}

void batteryDisableProfileAutoswitch(void) {
    profileAutoswitchDisable = true;
}

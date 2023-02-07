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

#pragma once

#include "config/parameter_group.h"

#include "drivers/time.h"

#include "fc/settings.h"

#include "sensors/battery_config_structs.h"

#ifndef VBAT_SCALE_DEFAULT
#define VBAT_SCALE_DEFAULT 1100
#endif

#ifndef CURRENT_METER_SCALE
#define CURRENT_METER_SCALE 400 // for Allegro ACS758LCB-100U (40mV/A)
#endif

#ifndef CURRENT_METER_OFFSET
#define CURRENT_METER_OFFSET 0
#endif

#ifndef MAX_BATTERY_PROFILE_COUNT
#define MAX_BATTERY_PROFILE_COUNT SETTING_CONSTANT_MAX_BATTERY_PROFILE_COUNT
#endif

typedef struct {
  uint8_t profile_index;
  uint16_t max_voltage;
} profile_comp_t;

PG_DECLARE(batteryMetersConfig_t, batteryMetersConfig);
PG_DECLARE_ARRAY(batteryProfile_t, MAX_BATTERY_PROFILE_COUNT, batteryProfiles);

extern const batteryProfile_t *currentBatteryProfile;

#define currentBatteryProfileMutable ((batteryProfile_t*)currentBatteryProfile)

typedef enum {
    BATTERY_OK = 0,
    BATTERY_WARNING,
    BATTERY_CRITICAL,
    BATTERY_NOT_PRESENT
} batteryState_e;


uint16_t batteryAdcToVoltage(uint16_t src);
batteryState_e getBatteryState(void);
batteryState_e checkBatteryVoltageState(void);
bool batteryWasFullWhenPluggedIn(void);
bool batteryUsesCapacityThresholds(void);
void batteryInit(void);
void setBatteryProfile(uint8_t profileIndex);
void activateBatteryProfile(void);
void batteryDisableProfileAutoswitch(void);

bool isBatteryVoltageConfigured(void);
bool isPowerSupplyImpedanceValid(void);
uint16_t getBatteryVoltage(void);
uint16_t getBatteryRawVoltage(void);
uint16_t getBatterySagCompensatedVoltage(void);
uint16_t getBatteryWarningVoltage(void);
uint8_t getBatteryCellCount(void);
uint16_t getBatteryRawAverageCellVoltage(void);
uint16_t getBatteryAverageCellVoltage(void);
uint16_t getBatterySagCompensatedAverageCellVoltage(void);
uint32_t getBatteryRemainingCapacity(void);
uint16_t getPowerSupplyImpedance(void);

bool isAmperageConfigured(void);
int16_t getAmperage(void);
int16_t getAmperageSample(void);
int32_t getPower(void);
int32_t getMAhDrawn(void);
int32_t getMWhDrawn(void);

#ifdef USE_ADC
void batteryUpdate(timeUs_t timeDelta);
void sagCompensatedVBatUpdate(timeUs_t currentTime, timeUs_t timeDelta);
void powerMeterUpdate(timeUs_t timeDelta);
uint16_t getVBatSample(void);
#endif

void currentMeterUpdate(timeUs_t timeDelta);

uint8_t calculateBatteryPercentage(void);
float calculateThrottleCompensationFactor(void);
int32_t calculateAveragePower(void);
int32_t calculateAverageEfficiency(void);
int32_t heatLossesCompensatedPower(int32_t power);

/*
 * This file is part of INAV.
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
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "platform.h"

#define MAX_HEADTRACKER_DATA_AGE_US HZ2US(25)
#define HEADTRACKER_RANGE_MIN   -2048
#define HEADTRACKER_RANGE_MAX   2047


#include <stdint.h>

#include "common/time.h"

#include "drivers/time.h"

#include "config/feature.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HEADTRACKER_NONE   = 0,
    HEADTRACKER_SERIAL = 1,
    HEADTRACKER_MSP    = 2,
    HEADTRACKER_UNKNOWN = 0xFF
} headTrackerDevType_e;


struct headTrackerVTable_s;

typedef struct headTrackerDevice_s {
    const struct headTrackerVTable_s *vTable;
    int pan;
    int tilt;
    int roll;
    timeUs_t expires;
} headTrackerDevice_t;

// {set,get}BandAndChannel: band and channel are 1 origin
// {set,get}PowerByIndex: 0 = Power OFF, 1 = device dependent
// {set,get}PitMode: 0 = OFF, 1 = ON

typedef struct headTrackerVTable_s {
    void (*process)(headTrackerDevice_t *headTrackerDevice, timeUs_t currentTimeUs);
    headTrackerDevType_e (*getDeviceType)(const headTrackerDevice_t *headTrackerDevice);
    bool (*isReady)(const headTrackerDevice_t *headTrackerDevice);
    bool (*isValid)(const headTrackerDevice_t *headTrackerDevice);
    int (*getPanPWM)(const headTrackerDevice_t *headTrackerDevice);
    int (*getTiltPWM)(const headTrackerDevice_t *headTrackerDevice);
    int (*getRollPWM)(const headTrackerDevice_t *headTrackerDevice);
    int (*getPan)(const headTrackerDevice_t *headTrackerDevice);
    int (*getTilt)(const headTrackerDevice_t *headTrackerDevice);
    int (*getRoll)(const headTrackerDevice_t *headTrackerDevice);
} headTrackerVTable_t;


typedef struct headTrackerConfig_s {
    headTrackerDevType_e devType;
    float pan_ratio;
    float tilt_ratio;
    float roll_ratio;
} headTrackerConfig_t;

#ifdef USE_HEADTRACKER

PG_DECLARE(headTrackerConfig_t, headTrackerConfig);

void headTrackerCommonInit(void);
void headTrackerCommonSetDevice(headTrackerDevice_t *headTrackerDevice);
headTrackerDevice_t *headTrackerCommonDevice(void);

// VTable functions
void headTrackerCommonProcess(headTrackerDevice_t *headTrackerDevice, timeUs_t currentTimeUs);
headTrackerDevType_e headTrackerCommonGetDeviceType(const headTrackerDevice_t *headTrackerDevice);
bool headTrackerCommonIsReady(const headTrackerDevice_t *headtrackerDevice);
bool headTrackerCommonIsValid(const headTrackerDevice_t *headtrackerDevice);

// Scaled value, constrained to PWM_RANGE_MIN~PWM_RANGE_MAX
int headTrackerCommonGetPanPWM(const headTrackerDevice_t *headTrackerDevice);
int headTrackerCommonGetTiltPWM(const headTrackerDevice_t *headTrackerDevice);
int headTrackerCommonGetRollPWM(const headTrackerDevice_t *headTrackerDevice);

// Scaled value, constrained to -2048~2047
int headTrackerCommonGetPan(const headTrackerDevice_t *headTrackerDevice);
int headTrackerCommonGetTilt(const headTrackerDevice_t *headTrackerDevice);
int headTrackerCommonGetRoll(const headTrackerDevice_t *headTrackerDevice);

void taskUpdateHeadTracker(timeUs_t currentTimeUs);

bool headtrackerCommonIsEnabled(void);


#ifdef __cplusplus
}
#endif

#endif
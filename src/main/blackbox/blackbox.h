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

#include "blackbox/blackbox_fielddefs.h"

#include "config/parameter_group.h"

typedef enum {
    BLACKBOX_FEATURE_NAV_ACC            = 1 << 0,
    BLACKBOX_FEATURE_NAV_POS            = 1 << 1,
    BLACKBOX_FEATURE_NAV_PID            = 1 << 2,
    BLACKBOX_FEATURE_MAG                = 1 << 3,
    BLACKBOX_FEATURE_ACC                = 1 << 4,
    BLACKBOX_FEATURE_ATTITUDE           = 1 << 5,
    BLACKBOX_FEATURE_RC_DATA            = 1 << 6,
    BLACKBOX_FEATURE_RC_COMMAND         = 1 << 7,
    BLACKBOX_FEATURE_MOTORS             = 1 << 8,
    BLACKBOX_FEATURE_GYRO_RAW           = 1 << 9,
    BLACKBOX_FEATURE_GYRO_PEAKS_ROLL    = 1 << 10,
    BLACKBOX_FEATURE_GYRO_PEAKS_PITCH   = 1 << 11,
    BLACKBOX_FEATURE_GYRO_PEAKS_YAW     = 1 << 12,
    BLACKBOX_FEATURE_SERVOS             = 1 << 13,
} blackboxFeatureMask_e;

typedef enum BlackboxState {
    BLACKBOX_STATE_DISABLED = 0,
    BLACKBOX_STATE_STOPPED,
    BLACKBOX_STATE_PREPARE_LOG_FILE,
    BLACKBOX_STATE_SEND_HEADER,
    BLACKBOX_STATE_SEND_MAIN_FIELD_HEADER,
    BLACKBOX_STATE_SEND_GPS_H_HEADER,
    BLACKBOX_STATE_SEND_GPS_G_HEADER,
    BLACKBOX_STATE_SEND_SLOW_HEADER,
    BLACKBOX_STATE_SEND_SYSINFO,
    BLACKBOX_STATE_PAUSED,
    BLACKBOX_STATE_RUNNING,
    BLACKBOX_STATE_SHUTTING_DOWN
} BlackboxState;

typedef struct blackboxConfig_s {
    uint16_t rate_num;
    uint16_t rate_denom;
    uint8_t device;
    uint8_t invertedCardDetection;
    uint32_t includeFlags;
    int8_t arm_control;
} blackboxConfig_t;

PG_DECLARE(blackboxConfig_t, blackboxConfig);

void blackboxLogEvent(FlightLogEvent event, flightLogEventData_t *data);

void blackboxInit(void);
void blackboxUpdate(timeUs_t currentTimeUs);
void blackboxStart(void);
void blackboxFinish(void);
bool blackboxMayEditConfig(void);
void blackboxIncludeFlagSet(uint32_t mask);
void blackboxIncludeFlagClear(uint32_t mask);
bool blackboxIncludeFlag(uint32_t mask);
BlackboxState getBlackboxState(void);

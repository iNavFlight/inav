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

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "fc/runtime_config.h"

#include "io/beeper.h"

EXTENDED_FASTRAM uint32_t armingFlags = 0;
EXTENDED_FASTRAM uint32_t stateFlags = 0;
EXTENDED_FASTRAM uint32_t flightModeFlags = 0;

static EXTENDED_FASTRAM uint32_t enabledSensors = 0;

#if !defined(CLI_MINIMAL_VERBOSITY)
const char *armingDisableFlagNames[]= {
    "FS", "ANGLE", "CAL", "OVRLD", "NAV", "COMPASS",
    "ACC", "ARMSW", "HWFAIL", "BOXFS", "KILLSW", "RX",
    "THR", "CLI", "CMS", "OSD", "ROLL/PITCH", "AUTOTRIM", "OOM",
    "SETTINGFAIL", "PWMOUT"
};
#endif

armingFlag_e isArmingDisabledReason(void)
{
    armingFlag_e flag;
    armingFlag_e reasons = armingFlags & ARMING_DISABLED_ALL_FLAGS;
    for (unsigned ii = 0; ii < sizeof(armingFlag_e) * 8; ii++) {
        flag = 1u << ii;
        if (flag & reasons) {
            return flag;
        }
    }
    return 0;
}

/**
 * Enables the given flight mode.  A beep is sounded if the flight mode
 * has changed.  Returns the new 'flightModeFlags' value.
 */
uint32_t enableFlightMode(flightModeFlags_e mask)
{
    uint32_t oldVal = flightModeFlags;

    flightModeFlags |= (mask);
    if (flightModeFlags != oldVal)
        beeperConfirmationBeeps(1);
    return flightModeFlags;
}

/**
 * Disables the given flight mode.  A beep is sounded if the flight mode
 * has changed.  Returns the new 'flightModeFlags' value.
 */
uint32_t disableFlightMode(flightModeFlags_e mask)
{
    uint32_t oldVal = flightModeFlags;

    flightModeFlags &= ~(mask);
    if (flightModeFlags != oldVal)
        beeperConfirmationBeeps(1);
    return flightModeFlags;
}

bool FAST_CODE NOINLINE sensors(uint32_t mask)
{
    return enabledSensors & mask;
}

void sensorsSet(uint32_t mask)
{
    enabledSensors |= mask;
}

void sensorsClear(uint32_t mask)
{
    enabledSensors &= ~(mask);
}

uint32_t sensorsMask(void)
{
    return enabledSensors;
}

flightModeForTelemetry_e getFlightModeForTelemetry(void)
{
    if (FLIGHT_MODE(FAILSAFE_MODE))
        return FLM_FAILSAFE;

    if (FLIGHT_MODE(MANUAL_MODE))
        return FLM_MANUAL;

    if (FLIGHT_MODE(NAV_RTH_MODE))
        return FLM_RTH;

    if (FLIGHT_MODE(NAV_POSHOLD_MODE))
        return FLM_POSITION_HOLD;

    if (FLIGHT_MODE(NAV_CRUISE_MODE))
        return FLM_CRUISE;

    if (FLIGHT_MODE(NAV_WP_MODE))
        return FLM_MISSION;

    if (FLIGHT_MODE(NAV_ALTHOLD_MODE))
        return FLM_ALTITUDE_HOLD;

    if (FLIGHT_MODE(ANGLE_MODE))
        return FLM_ANGLE;

    if (FLIGHT_MODE(HORIZON_MODE))
        return FLM_HORIZON;

    if (FLIGHT_MODE(NAV_LAUNCH_MODE))
        return FLM_LAUNCH;

    return STATE(AIRMODE_ACTIVE) ? FLM_ACRO_AIR : FLM_ACRO;
}

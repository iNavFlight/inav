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
#include <math.h>

/* Speeds above which the aircraft is treated as moving fast enough to be
 * flying, so tune them here rather than at the call sites.
 *
 * The tally uses only the 3D speed: vel3D includes vertical speed, so it is
 * never below velXY and a separate velXY check could not change the outcome.
 * Its value (300) is the one the emergency-rearm gate already used before the
 * tally existed (isProbablyStillFlying()). The XY threshold is used only by
 * isFixedWingFlying(), where it is combined with airspeed. */
#define FW_FLIGHT_MIN_AIRSPEED_CMS      350.0f
#define FW_FLIGHT_MIN_VEL_XY_CMS        350.0f
#define FW_FLIGHT_MIN_VEL_3D_CMS        300.0f

/* Baro altitude change from the disarmed baseline that counts as flight.
 * Must stay well above baro drift and arming pressure transients. */
#define FW_FLIGHT_MIN_BARO_CHANGE_CM    500.0f

/* Weights: GPS heading 3, airspeed 3, baro altitude change 2 (max 8).
 * Velocity (2) counts only when GPS heading is not valid: it comes from the
 * same GPS measurement, so counting both would double-count it. Then >=3 is
 * GPS heading or airspeed alone; >=5 needs two independent sources; >=6 needs
 * both GPS heading and airspeed; <=0 is none.
 *
 * Pass 0 for any source that is unavailable (no healthy pitot or baro). */
static inline int8_t fwFlightTallyCompute(
    const bool gpsHeadingValid,
    const float airspeedCms,
    const float vel3DCms,
    const float baroChangeCm)
{
    int8_t tally = 0;

    if (gpsHeadingValid) {
        tally += 3;
    } else if (vel3DCms > FW_FLIGHT_MIN_VEL_3D_CMS) {
        tally += 2;
    }

    if (airspeedCms > FW_FLIGHT_MIN_AIRSPEED_CMS) {
        tally += 3;
    }

    if (fabsf(baroChangeCm) > FW_FLIGHT_MIN_BARO_CHANGE_CM) {
        tally += 2;
    }

    return tally;
}

/* Takeoff signature, shaped like the launch controller's detection
 * (navigation_fw_launch.c): every launch path accelerates the aircraft toward
 * the nose (body +X), whether it is thrown level or steeply nose-up.
 *
 * - Bungee: forward acceleration above the launch threshold while almost
 *   level, so gravity on +X when nose-up cannot satisfy it.
 * - Swing / forward-GPS: a magnitude excess over g (gravity alone cannot
 *   produce one at any attitude) that is also toward the nose.
 *
 * Magnitude excess alone is not enough: a vertical bump or a sideways shake
 * has one but is not a launch. */
static inline bool fwFlightTakeoffSignature(
    const float accelForwardCmss,
    const float accelNormSqCmss,
    const float gravityCmss,
    const float accelThreshCmss,
    const bool isAlmostLevel,
    const float swingVelocityCms,
    const float velThreshCms,
    const bool gpsHeadingValid,
    const float groundSpeedCms)
{
    const bool realAccel = (accelNormSqCmss - gravityCmss * gravityCmss) > accelThreshCmss * accelThreshCmss
                           && accelForwardCmss > 0.0f;

    const bool isBungeeLaunched = accelForwardCmss > accelThreshCmss && isAlmostLevel;
    const bool isSwingLaunched = swingVelocityCms > velThreshCms && realAccel;
    const bool isForwardLaunched = gpsHeadingValid && groundSpeedCms > velThreshCms && realAccel;

    return isBungeeLaunched || isSwingLaunched || isForwardLaunched;
}

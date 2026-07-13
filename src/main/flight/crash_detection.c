/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <math.h>
#include <stdbool.h>

#include <platform.h>

#ifdef USE_ORIENTATION_HOLD

#include "common/maths.h"
#include "common/vector.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/crash_detection.h"
#include "flight/mixer.h"

#include "io/gps.h"

#include "navigation/navigation.h"

#include "rx/rx.h"

#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/gyro.h"

PG_REGISTER_WITH_RESET_TEMPLATE(crashDetectionConfig_t, crashDetectionConfig, PG_CRASH_DETECTION_CONFIG, 0);

PG_RESET_TEMPLATE(crashDetectionConfig_t, crashDetectionConfig,
    .crashGThreshold = SETTING_CRASH_G_THRESHOLD_DEFAULT,
);

// In-flight latch: the detector must never fire while the armed aircraft is
// carried to the strip or waits for a hand launch (it IS still then). It
// arms once the aircraft is clearly flying: nav launch completed, or the
// throttle held above cruise level for a moment.
#define CRASH_INFLIGHT_THROTTLE_US   1350
#define CRASH_INFLIGHT_HOLD_S        1.0f

// Impact -> stillness confirmation window. A flying aircraft is never
// still, so aggressive maneuvers (snap, spin, gust) cannot confirm; the
// window must close before the post-figure flight smooths out (a level
// line a few seconds after a hard snap IS rate-still and 1 g).
#define CRASH_WINDOW_S               3.0f
// Stillness must exclude QUASI-STEADY FLIGHT, not only maneuvering: a
// smooth mushing climb also has low rates and ~1 g (found as a false
// positive in SITL - a hard pull spiked the impact latch and the steady
// climb after it read as "lying still"). The vertical-motion condition is
// the discriminator: a crashed aircraft has a FROZEN baro, flight does
// not. The RAW baro rate is used, not the fused vertical speed: the
// impact spike corrupts the INS for ~4.5 s (bench-measured, 12 g pulse),
// far beyond the window, while the baro is honest half a second after
// the airframe stops.
#define CRASH_STILL_RATE_DPS         15.0f
#define CRASH_STILL_ACC_G_LO         0.9f
#define CRASH_STILL_ACC_G_HI         1.1f
#define CRASH_STILL_VZ_CMS          100.0f
#define CRASH_STILL_CONFIRM_S        1.0f
#define CRASH_BARO_RATE_TAU_S        0.5f
#define CRASH_STILL_GS_CMS           300

static bool inFlight = false;
static float inFlightTimerS;
static float impactWindowS;
static float stillTimerS;
static bool motorCut = false;
static bool cutAckLow = false;
static float baroRateCms;
static float lastBaroAltCm;

static float crashVerticalRateCms(float dT)
{
#ifdef USE_BARO
    if (sensors(SENSOR_BARO)) {
        const float baroAltCm = baro.BaroAlt;
        const float rawRate = (baroAltCm - lastBaroAltCm) / dT;
        lastBaroAltCm = baroAltCm;
        baroRateCms += (rawRate - baroRateCms) * MIN(dT / CRASH_BARO_RATE_TAU_S, 1.0f);
        return baroRateCms;
    }
#endif
    return getEstimatedActualVelocity(Z);
}

void crashDetectionUpdate(float dT)
{
    if (crashDetectionConfig()->crashGThreshold == 0
        || !STATE(AIRPLANE)
        || !ARMING_FLAG(ARMED)) {
        inFlight = false;
        inFlightTimerS = 0.0f;
        impactWindowS = 0.0f;
        stillTimerS = 0.0f;
        motorCut = false;
        cutAckLow = false;
        baroRateCms = 0.0f;
#ifdef USE_BARO
        lastBaroAltCm = baro.BaroAlt;
#endif
        return;
    }

    // after a crash the motor stays CUT (not disarmed) until the pilot
    // acknowledges: throttle to zero, then up again re-allows the motor -
    // short bursts help locating the aircraft in high grass or corn
    if (motorCut) {
        const bool thrLow = rcCommand[THROTTLE] < getThrottleIdleValue() + 50;
        if (!cutAckLow) {
            cutAckLow = thrLow;
        } else if (!thrLow) {
            motorCut = false;
            cutAckLow = false;
            impactWindowS = 0.0f;
            stillTimerS = 0.0f;
        }
        return;
    }

    // in-flight latch (hand launch rule)
    if (!inFlight) {
        if (isNavLaunchEnabled()) {
            inFlight = fixedWingLaunchStatus() >= FW_LAUNCH_FLYING;
        } else if (rcCommand[THROTTLE] > CRASH_INFLIGHT_THROTTLE_US) {
            inFlightTimerS += dT;
            inFlight = inFlightTimerS > CRASH_INFLIGHT_HOLD_S;
        } else {
            inFlightTimerS = 0.0f;
        }
        if (!inFlight) {
            return;
        }
    }

    const float vertRateCms = crashVerticalRateCms(dT);

    fpVector3_t accG;
    accGetMeasuredAcceleration(&accG);          // cm/s^2
    const float accMagG = fast_fsqrtf(sq(accG.x) + sq(accG.y) + sq(accG.z)) / GRAVITY_CMSS;

    // impact latches the confirmation window
    if (accMagG > crashDetectionConfig()->crashGThreshold / 10.0f) {
        impactWindowS = CRASH_WINDOW_S;
        stillTimerS = 0.0f;
    }
    if (impactWindowS <= 0.0f) {
        return;
    }
    impactWindowS -= dT;

    const float rateMagDps = fast_fsqrtf(sq((float)gyroRateDps(FD_ROLL))
                                       + sq((float)gyroRateDps(FD_PITCH))
                                       + sq((float)gyroRateDps(FD_YAW)));
    bool still = rateMagDps < CRASH_STILL_RATE_DPS
              && accMagG > CRASH_STILL_ACC_G_LO
              && accMagG < CRASH_STILL_ACC_G_HI
              && fabsf(vertRateCms) < CRASH_STILL_VZ_CMS;
#ifdef USE_GPS
    // a valid fix adds the discriminator IMU + baro cannot provide: a
    // coordinated line or shallow turn right after a hard pull is
    // rate-still, 1 g and baro-flat - but it MOVES, a crashed airframe
    // does not. Without GPS (or without a fix) the g threshold has to
    // separate figures from impacts on its own.
    if (still && sensors(SENSOR_GPS) && gpsSol.fixType >= GPS_FIX_3D) {
        still = gpsSol.groundSpeed < CRASH_STILL_GS_CMS;
    }
#endif

    if (still) {
        stillTimerS += dT;
        if (stillTimerS > CRASH_STILL_CONFIRM_S) {
            // impact followed by stillness: the flight is over, cut the motor
            motorCut = true;
            cutAckLow = false;
        }
    } else {
        stillTimerS = 0.0f;
    }
}

bool crashDetectionMotorCut(void)
{
    return motorCut;
}

#endif // USE_ORIENTATION_HOLD

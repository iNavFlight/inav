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

#include <stdbool.h>
#include <math.h>

#include <platform.h>

#ifdef USE_ORIENTATION_HOLD

#include "common/axis.h"
#include "common/maths.h"
#include "common/quaternion.h"
#include "common/vector.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/rc_controls.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/hover_throttle.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/orientation_hold.h"

#include "navigation/navigation.h"

#include "rx/rx.h"

PG_REGISTER_WITH_RESET_TEMPLATE(hoverThrottleConfig_t, hoverThrottleConfig, PG_HOVER_THROTTLE_CONFIG, 0);

PG_RESET_TEMPLATE(hoverThrottleConfig_t, hoverThrottleConfig,
    .pGain = SETTING_OHOLD_HOVER_THR_P_DEFAULT,
    .iGain = SETTING_OHOLD_HOVER_THR_I_DEFAULT,
    .dGain = SETTING_OHOLD_HOVER_THR_D_DEFAULT,
    .minThrottle = SETTING_OHOLD_HOVER_THR_MIN_DEFAULT,
);

// Engage only when the nose is this close to the zenith; once engaged,
// stay active down to the release threshold. Without the hysteresis the
// attitude wobble around the hang flaps the controller and every
// re-engage captures a NEW altitude target at the current height -- a
// ratcheting drift.
#define HOVER_ENGAGE_NOSE_ELEVATION_DEG  60.0f
#define HOVER_RELEASE_NOSE_ELEVATION_DEG 45.0f

// The target latches only once the vertical motion has settled: engaging
// mid pull-up (the normal way to enter a hang) must not freeze the target
// at some fly-through altitude
#define HOVER_LATCH_CLIMB_CMS 200.0f

static bool hoverActive = false;
static bool hoverLatched = false;
static float targetAltCm;
static float iTermUs;
static timeUs_t lastUpdateUs;

static float noseElevationDeg(void)
{
    fpVector3_t nose = { .v = { 1.0f, 0.0f, 0.0f } };
    quaternionRotateVectorInv(&nose, &nose, &orientation);   // body -> earth
    return RADIANS_TO_DEGREES(asin_approx(constrainf(-nose.z, -1.0f, 1.0f)));
}

int16_t hoverThrottleApply(int16_t pilotThrottle)
{
    const float elevDeg = noseElevationDeg();
    const float elevGate = hoverActive ? HOVER_RELEASE_NOSE_ELEVATION_DEG
                                       : HOVER_ENGAGE_NOSE_ELEVATION_DEG;

    if (!ARMING_FLAG(ARMED)
        || !orientationHoldIsPropHang()
        || !navIsAltitudeEstimateTrusted()
        || elevDeg < elevGate) {
        hoverActive = false;
        return pilotThrottle;
    }

    const float z = getEstimatedActualPosition(Z);

    // pilot throttle outside the mid deadband: direct control, target follows
    if (ABS(pilotThrottle - PWM_RANGE_MIDDLE) > rcControlsConfig()->mid_throttle_deadband) {
        hoverActive = false;
        return pilotThrottle;
    }

    if (!hoverActive) {
        hoverActive = true;
        hoverLatched = false;
        targetAltCm = z;
        // seed the I-term with the last pilot throttle: learns the model's
        // hover throttle online instead of requiring a setting
        iTermUs = pilotThrottle;
        lastUpdateUs = micros();
    }

    const timeUs_t nowUs = micros();
    const float dT = constrainf((nowUs - lastUpdateUs) * 1e-6f, 0.0f, 0.1f);
    lastUpdateUs = nowUs;

    const float climbCms = getEstimatedActualVelocity(Z);
    if (!hoverLatched) {
        targetAltCm = z;                       // follow until motion settles
        if (fabsf(climbCms) < HOVER_LATCH_CLIMB_CMS) {
            hoverLatched = true;
        }
    }

    const float zErrM = (targetAltCm - z) / 100.0f;
    const float climbMs = climbCms / 100.0f;

    // throttle floor: never cut the throttle below what keeps the prop wash
    // (and with it the control authority) alive -- excess lift, e.g. in an
    // updraft, is accepted as a climb instead
    const int16_t floorThrottle = MAX(getThrottleIdleValue(),
                                      (int16_t)hoverThrottleConfig()->minThrottle);

    iTermUs = constrainf(iTermUs + hoverThrottleConfig()->iGain * zErrM * dT,
                         floorThrottle, getMaxThrottle());

    // thrust supports the weight with its vertical component only:
    // compensate the tilt away from the zenith (capped, the elevation
    // gate keeps this bounded anyway)
    const float vertical = constrainf(sin_approx(DEGREES_TO_RADIANS(elevDeg)), 0.5f, 1.0f);
    const float correction = (hoverThrottleConfig()->pGain * zErrM
                              - hoverThrottleConfig()->dGain * climbMs) / vertical;

    return constrain(lrintf(iTermUs + correction), floorThrottle, getMaxThrottle());
}

#endif // USE_ORIENTATION_HOLD

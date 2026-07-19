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

#ifdef USE_SOARING

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/vector.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/soaring.h"
#include "flight/wind_estimator.h"

#include "navigation/navigation.h"

#include "sensors/acceleration.h"
#include "sensors/pitotmeter.h"
#include "sensors/sensors.h"

PG_REGISTER_WITH_RESET_TEMPLATE(soaringConfig_t, soaringConfig, PG_SOARING_CONFIG, 0);

PG_RESET_TEMPLATE(soaringConfig_t, soaringConfig,
    .varioTriggerCms = SETTING_SOAR_VARIO_TRIGGER_DEFAULT,
    .varioExitCms = SETTING_SOAR_VARIO_EXIT_DEFAULT,
    .altMinM = SETTING_SOAR_ALT_MIN_DEFAULT,
    .altMaxM = SETTING_SOAR_ALT_MAX_DEFAULT,
    .bankDeg = SETTING_SOAR_BANK_DEFAULT,
    .sinkLevelCms = SETTING_SOAR_SINK_LEVEL_DEFAULT,
    .centreGainPct = SETTING_SOAR_CENTRE_GAIN_DEFAULT,
);

// Slow low-pass for the DC part of the net vario: the sin/cos gradient
// correlates the DEVIATION from this mean, so a strong-but-uniform column
// does not bias the shift. A few circle periods.
#define SOAR_VARIO_MEAN_TAU_S   10.0f
// Gradient low-pass ~ one circle period: it takes a full turn to see which
// side of the circle climbs best.
#define SOAR_GRAD_TAU_S          6.0f
// Converts the vario gradient [m/s] into a centre shift rate [cm/s] at
// centreGainPct = 100. The bench tunes centreGainPct on top; small, because
// "slowly" - a circle that chases noise eier and loses the thermal.
#define SOAR_CENTRE_GAIN_SCALE  60.0f
// Never let the estimate run more than this from where the climb started
// (a runaway gradient would walk the loiter out of the sky).
#define SOAR_CENTRE_MAX_DRIFT_CM 30000.0f   // 300 m

static bool soarActive = false;
static bool thermalling = false;
static fpVector3_t thermalCentre;   // earth frame, cm from home (XY loiter)
static fpVector3_t breachAnchor;    // where the climb was first found
static float varioMean = 0.0f;
static float gradN = 0.0f, gradE = 0.0f;
static float vPrev = 0.0f;
static float netVarioCms = 0.0f;

static float computeNetVarioMs(float dT)
{
    // total-energy variometer: the air's vertical motion, our own polar
    // sink compensated out. e = h + v^2/2g ; the pitot gives v, so airspeed
    // transients (the phantom climb on a pull-out) cancel - the reason a
    // pitot is required. sink uses the EXACT cos of the actual bank (ArduSoar
    // uses a small-angle approximation that drifts at the 35-45 deg thermal
    // bank; this does not).
    float v = getAirspeedEstimate() / 100.0f;               // m/s
    v = constrainf(v, 3.0f, 100.0f);
    const float hdot = getEstimatedActualVelocity(Z) / 100.0f;   // m/s, up +
    const float vdot = (v - vPrev) / dT;
    vPrev = v;
    float cosRoll = fabsf(cos_approx(DECIDEGREES_TO_RADIANS(attitude.values.roll)));
    cosRoll = MAX(0.2f, cosRoll);
    const float energyRate = hdot + v * vdot / GRAVITY_MSS;      // m/s
    const float sink = (soaringConfig()->sinkLevelCms / 100.0f)
                     / (cosRoll * fast_fsqrtf(cosRoll));         // /cos^1.5
    return energyRate + sink;                                   // air w [m/s]
}

void soaringUpdate(float dT)
{
    // The pilot's SOARING mode arms it; a pitot is required (the net vario
    // is meaningless without airspeed) and only fixed wing soars. Without
    // any of these the module is inert - the FC behaves exactly as upstream.
    if (!IS_RC_MODE_ACTIVE(BOXSOARING) || !ARMING_FLAG(ARMED)
        || !STATE(AIRPLANE) || !sensors(SENSOR_PITOT)) {
        if (thermalling) {
            navForcedPosholdClear();   // release the loiter
        }
        soarActive = false;
        thermalling = false;
        return;
    }
    soarActive = true;

    const float netVario = computeNetVarioMs(dT);
    netVarioCms = netVario * 100.0f;
    varioMean += (netVario - varioMean) * MIN(dT / SOAR_VARIO_MEAN_TAU_S, 1.0f);

    const float alt = getEstimatedActualPosition(Z) / 100.0f;   // m
    const float triggerMs = soaringConfig()->varioTriggerCms / 100.0f;
    const float exitMs = soaringConfig()->varioExitCms / 100.0f;

    if (!thermalling) {
        // enter a thermal: net lift over the trigger, inside the altitude band
        if (netVario > triggerMs
            && alt > soaringConfig()->altMinM && alt < soaringConfig()->altMaxM) {
            thermalling = true;
            breachAnchor.x = thermalCentre.x = getEstimatedActualPosition(X);
            breachAnchor.y = thermalCentre.y = getEstimatedActualPosition(Y);
            breachAnchor.z = thermalCentre.z = getEstimatedActualPosition(Z);
            gradN = gradE = 0.0f;
            // hand the loiter to the real nav machinery, anchored here
            navForcedPosholdActivateAt(&thermalCentre);
        }
        return;
    }

    // CENTERING: correlate the vario deviation against the bearing from the
    // current centre estimate to the aircraft over one turn (sin/cos = the
    // first harmonic, pointing at the strongest climb), then slide the centre
    // that way AND with the wind (the thermal is locked to the air mass -
    // wind * dt is the exact drift, no climb/strength scaling).
    const float dx = getEstimatedActualPosition(X) - thermalCentre.x;   // north cm
    const float dy = getEstimatedActualPosition(Y) - thermalCentre.y;   // east cm
    const float bearing = atan2_approx(dy, dx);
    const float dv = netVario - varioMean;
    const float a = MIN(dT / SOAR_GRAD_TAU_S, 1.0f);
    gradN += (dv * cos_approx(bearing) - gradN) * a;
    gradE += (dv * sin_approx(bearing) - gradE) * a;

    const float k = (soaringConfig()->centreGainPct / 100.0f) * SOAR_CENTRE_GAIN_SCALE;
    thermalCentre.x += (k * gradN + getEstimatedWindSpeed(X)) * dT;   // cm
    thermalCentre.y += (k * gradE + getEstimatedWindSpeed(Y)) * dT;
    // keep the loiter anchored on the moving centre estimate (the POSHOLD
    // initialize re-fires per RX cycle and would otherwise re-anchor "here")
    navForcedPosholdAssert(&thermalCentre);

    // clamp the estimate to a sane radius around where the climb was found
    const float driftX = thermalCentre.x - breachAnchor.x;
    const float driftY = thermalCentre.y - breachAnchor.y;
    const float drift = calc_length_pythagorean_2D(driftX, driftY);
    if (drift > SOAR_CENTRE_MAX_DRIFT_CM) {
        const float s = SOAR_CENTRE_MAX_DRIFT_CM / drift;
        thermalCentre.x = breachAnchor.x + driftX * s;
        thermalCentre.y = breachAnchor.y + driftY * s;
    }

#if defined(SITL_BUILD)
    debug[0] = lrintf(netVarioCms);           // net vario [cm/s]
    debug[1] = lrintf(drift);                 // |centre - anchor| [cm]
    debug[2] = lrintf(driftX);                // centre shift north [cm]
    debug[3] = lrintf(driftY);                // centre shift east [cm]
    debug[4] = getThrottleIdleValue();        // applied idle throttle [us]
#endif

    // leave the thermal: lift collapsed, or out of the altitude band
    if (netVario < exitMs
        || alt > soaringConfig()->altMaxM || alt < soaringConfig()->altMinM) {
        thermalling = false;
        navForcedPosholdClear();   // hand the loiter back to the pilot / cruise
    }
}

bool soaringActive(void)
{
    return soarActive;
}

bool soaringThermalling(void)
{
    return thermalling;
}

void soaringThermalCentre(fpVector3_t *centre)
{
    *centre = thermalCentre;
}

float soaringNetVarioCms(void)
{
    return netVarioCms;
}

int16_t soaringThrottleApply(int16_t throttle)
{
    // While circling a thermal the glider soars on the lift with the motor
    // idled: a throttle-to-idle override, NOT a motor stop - the control
    // surfaces keep flying the loiter. Normal throttle returns the instant
    // the thermal is left or the aircraft sinks below soar_alt_min, both of
    // which drop 'thermalling'.
    if (thermalling) {
        return getThrottleIdleValue();
    }
    return throttle;
}

#endif // USE_SOARING

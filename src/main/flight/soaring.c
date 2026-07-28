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
#include <string.h>
#include <math.h>

#include <platform.h>

#ifdef USE_SOARING

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/vector.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/soaring.h"
#include "flight/wind_estimator.h"

#include "navigation/navigation.h"

#include "rx/rx.h"

#include "sensors/acceleration.h"
#include "sensors/battery.h"
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

// Centre shift rate [cm/s] along the history gradient at centreGainPct = 100.
// Small, because "slowly" - a circle that chases noise loses the thermal.
#define SOAR_CENTRE_GAIN_SCALE  60.0f
// Never let the estimate run more than this from where the climb started
// (a runaway would walk the loiter out of the sky). Sized to the lift grid:
// the map may legitimately lead the circle up to its own half-width.
#define SOAR_CENTRE_MAX_DRIFT_CM 45000.0f   // 450 m ~ grid half-width
// The trigger must be SUSTAINED: a transient crossing of a weak lift edge
// never holds this long, the wide band around a real core does (measured:
// an instant trigger anchored the circle 520 m off-core on a transient).
#define SOAR_TRIGGER_SUSTAIN_S  2.0f
// ---- Lift-grid centering (flight contract, centering v3) -------------------
// A COARSE CHECKERBOARD of max-netto cells (Daniel): 32 x 32 cells of ~30 m
// = 1 KB covering ~1 km x 1 km. The MAP remembers where the lift was - a
// handful of ring samples is degenerate for a gradient (measured: anchored
// 600 m off-core, local gradient zero), the grid steers toward the best
// KNOWN lift even where the local gradient is blind. The grid window RIDES
// THE WIND (origin moves with the air mass - zero data movement) and ROLLS
// with the aircraft: flying out of the window discards the farthest line and
// reuses its memory for the new near side (toroidal indexing, no copying).
#define SOAR_GRID_N            32
#define SOAR_GRID_CELL_CM      3000.0f  // ~30 m cells
#define SOAR_GRID_LIFT_FLOOR   130      // cell value that counts as real lift (~ +0.2 m/s)
#define SOAR_GRID_DECAY_S      2.0f     // 0.1 m/s fade per this period: dead thermals age out
#define SOAR_EXPLORE_FRAC      0.2f    // exploration offset, fraction of the loiter radius
#define SOAR_EXPLORE_ADVANCE_RAD 1.9f  // exploration direction advance per round (never repeats)

static bool soarActive = false;
static bool thermalling = false;
static fpVector3_t thermalCentre;   // earth frame, cm from home (XY loiter)
static fpVector3_t breachAnchor;    // where the climb was first found
static float vPrev = 0.0f;
static float netVarioCms = 0.0f;
// lift grid (rolling, wind-riding) + exploration + circle-averaged exit
static uint8_t liftGrid[SOAR_GRID_N][SOAR_GRID_N];  // max netto seen; 0 = unknown,
                                                    // else clamp(netto*10 + 128)
static fpVector3_t gridOriginCm;    // ground pos of window corner [0][0]; RIDES THE WIND
static int16_t gridBaseI = 0, gridBaseJ = 0;  // toroidal base of the rolling window
static float gridDecayTimerS = 0.0f;
static float exploreAngleRad = 0.0f;
static float roundAccumRad = 0.0f;
static float exitMeanMs = 0.0f;            // circle-averaged netto for the exit

// rolling the window costs exactly ONE line: the line that falls out of the
// far side is re-initialised and becomes the new near side - nothing copies
static void soarGridClearI(int16_t arrI)
{
    for (int j = 0; j < SOAR_GRID_N; j++) {
        liftGrid[arrI][j] = 0;
    }
}
static void soarGridClearJ(int16_t arrJ)
{
    for (int i = 0; i < SOAR_GRID_N; i++) {
        liftGrid[i][arrJ] = 0;
    }
}

static bool netVarioValid = false;   // false while the motor blinds the vario

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
    // MOTOR-AWARE (the entry gate, flight contract: only TRUE AIR LIFT may
    // trigger or paint the grid): the polar-sink compensation is only valid
    // with the motor off - at cruise the motor cancels the airframe's sink
    // and the vario would read +sink of phantom lift in plain level flight
    // (measured: thermalling triggered 600 m from any thermal on exactly
    // that). While THERMALLING the FW forces the motor off -> full
    // compensation; otherwise blend it out toward the cruise throttle, and
    // above cruise (a deliberate powered climb) the vario is blind.
    float motorFactor = 0.0f;
    if (!thermalling) {
        const int16_t thrUs = rcCommand[THROTTLE];
        const int16_t idleUs = getThrottleIdleValue();
        const int16_t cruiseUs = currentBatteryProfile->nav.fw.cruise_throttle;
        motorFactor = constrainf((float)(thrUs - idleUs) / MAX(1, cruiseUs - idleUs), 0.0f, 1.0f);
        netVarioValid = thrUs <= cruiseUs + 25;
    } else {
        netVarioValid = true;
    }
    const float sink = (soaringConfig()->sinkLevelCms / 100.0f)
                     / (cosRoll * fast_fsqrtf(cosRoll))          // /cos^1.5
                     * (1.0f - motorFactor);
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

    const float alt = getEstimatedActualPosition(Z) / 100.0f;   // m
    const float triggerMs = soaringConfig()->varioTriggerCms / 100.0f;
    const float exitMs = soaringConfig()->varioExitCms / 100.0f;

    if (!thermalling) {
        // enter a thermal: TRUE AIR lift over the trigger (netVarioValid -
        // a powered climb never triggers), SUSTAINED (a transient edge
        // crossing never holds SOAR_TRIGGER_SUSTAIN_S), inside the band
        static float triggerHoldS = 0.0f;
        if (netVarioValid && netVario > triggerMs
            && alt > soaringConfig()->altMinM && alt < soaringConfig()->altMaxM) {
            triggerHoldS += dT;
        } else {
            triggerHoldS = 0.0f;
        }
        if (triggerHoldS >= SOAR_TRIGGER_SUSTAIN_S) {
            triggerHoldS = 0.0f;
            thermalling = true;
            breachAnchor.x = thermalCentre.x = getEstimatedActualPosition(X);
            breachAnchor.y = thermalCentre.y = getEstimatedActualPosition(Y);
            breachAnchor.z = thermalCentre.z = getEstimatedActualPosition(Z);
            memset(liftGrid, 0, sizeof(liftGrid));
            gridBaseI = gridBaseJ = 0;
            gridDecayTimerS = 0.0f;
            gridOriginCm.x = thermalCentre.x - (SOAR_GRID_N / 2) * SOAR_GRID_CELL_CM;
            gridOriginCm.y = thermalCentre.y - (SOAR_GRID_N / 2) * SOAR_GRID_CELL_CM;
            exploreAngleRad = roundAccumRad = 0.0f;
            exitMeanMs = netVario;   // seed: never an instant exit at entry
            // hand the loiter to the real nav machinery, anchored here
            navForcedPosholdActivateAt(&thermalCentre);
        }
        return;
    }

    // circle period from the commanded bank and the current speed:
    // omega = g * tan(bank) / v, T = 2 pi / omega
    const float vMs = constrainf(getAirspeedEstimate() / 100.0f, 5.0f, 60.0f);
    const float omega = GRAVITY_MSS
        * tan_approx(DEGREES_TO_RADIANS((float)soaringConfig()->bankDeg)) / vMs;
    const float periodS = 2.0f * M_PIf / omega;

    // CENTERING v3 (flight contract): the coarse lift grid. The window rides
    // the WIND (origin moves with the air mass - the map is anchored to the
    // column by construction, zero data movement) ...
    gridOriginCm.x += getEstimatedWindSpeed(X) * dT;
    gridOriginCm.y += getEstimatedWindSpeed(Y) * dT;
    // ... and ROLLS with the aircraft: leaving the window costs exactly one
    // line - the farthest falls out, its memory is re-initialised as the new
    // near side (toroidal base index, nothing copies)
    int16_t ci = (int16_t)floorf((getEstimatedActualPosition(X) - gridOriginCm.x) / SOAR_GRID_CELL_CM);
    int16_t cj = (int16_t)floorf((getEstimatedActualPosition(Y) - gridOriginCm.y) / SOAR_GRID_CELL_CM);
    while (ci >= SOAR_GRID_N) {
        soarGridClearI(gridBaseI);
        gridBaseI = (gridBaseI + 1) % SOAR_GRID_N;
        gridOriginCm.x += SOAR_GRID_CELL_CM;
        ci--;
    }
    while (ci < 0) {
        gridBaseI = (gridBaseI - 1 + SOAR_GRID_N) % SOAR_GRID_N;
        soarGridClearI(gridBaseI);
        gridOriginCm.x -= SOAR_GRID_CELL_CM;
        ci++;
    }
    while (cj >= SOAR_GRID_N) {
        soarGridClearJ(gridBaseJ);
        gridBaseJ = (gridBaseJ + 1) % SOAR_GRID_N;
        gridOriginCm.y += SOAR_GRID_CELL_CM;
        cj--;
    }
    while (cj < 0) {
        gridBaseJ = (gridBaseJ - 1 + SOAR_GRID_N) % SOAR_GRID_N;
        soarGridClearJ(gridBaseJ);
        gridOriginCm.y -= SOAR_GRID_CELL_CM;
        cj++;
    }
    // record: each cell keeps the MAX netto seen there (robust against the
    // turbulent instant value - the best pass through a cell is the truth)
    {
        const uint8_t ai = (gridBaseI + ci) % SOAR_GRID_N;
        const uint8_t aj = (gridBaseJ + cj) % SOAR_GRID_N;
        const uint8_t val = constrain(lrintf(netVario * 10.0f) + 128, 1, 255);
        if (val > liftGrid[ai][aj]) {
            liftGrid[ai][aj] = val;
        }
    }
    // slow fade so a dead thermal ages out of the map (0.1 m/s per period)
    gridDecayTimerS += dT;
    if (gridDecayTimerS >= SOAR_GRID_DECAY_S) {
        gridDecayTimerS = 0.0f;
        for (int i = 0; i < SOAR_GRID_N; i++) {
            for (int j = 0; j < SOAR_GRID_N; j++) {
                if (liftGrid[i][j] > 0) {
                    liftGrid[i][j]--;
                }
            }
        }
    }
    // steer toward the BEST KNOWN lift on the map (argmax over the window):
    // works even where the local gradient is blind - the map remembers
    {
        uint8_t best = SOAR_GRID_LIFT_FLOOR;
        int16_t bwi = -1, bwj = -1;
        for (int16_t wi = 0; wi < SOAR_GRID_N; wi++) {
            const int16_t ai = (gridBaseI + wi) % SOAR_GRID_N;
            for (int16_t wj = 0; wj < SOAR_GRID_N; wj++) {
                const uint8_t v = liftGrid[ai][(gridBaseJ + wj) % SOAR_GRID_N];
                if (v > best) {
                    best = v;
                    bwi = wi;
                    bwj = wj;
                }
            }
        }
        if (bwi >= 0) {
            const float tx = gridOriginCm.x + (bwi + 0.5f) * SOAR_GRID_CELL_CM;
            const float ty = gridOriginCm.y + (bwj + 0.5f) * SOAR_GRID_CELL_CM;
            const float dvx = tx - thermalCentre.x;
            const float dvy = ty - thermalCentre.y;
            const float dist = calc_length_pythagorean_2D(dvx, dvy);
            if (dist > SOAR_GRID_CELL_CM * 0.5f) {
                const float k = (soaringConfig()->centreGainPct / 100.0f) * SOAR_CENTRE_GAIN_SCALE;
                thermalCentre.x += k * (dvx / dist) * dT;   // cm
                thermalCentre.y += k * (dvy / dist) * dT;
            }
        }
    }
    // the circle itself always rides the wind (exact drift of the column)
    thermalCentre.x += getEstimatedWindSpeed(X) * dT;
    thermalCentre.y += getEstimatedWindSpeed(Y) * dT;

    // EXPLORATION (contract: "mit jeder Runde etwas daneben fliegen"): each
    // completed round the loiter anchor moves a little off the centre in a
    // new direction, so the next round samples fresh gradient information
    roundAccumRad += omega * dT;
    if (roundAccumRad >= 2.0f * M_PIf) {
        roundAccumRad -= 2.0f * M_PIf;
        exploreAngleRad += SOAR_EXPLORE_ADVANCE_RAD;
    }
    fpVector3_t anchor = thermalCentre;
    const float exploreCm = SOAR_EXPLORE_FRAC * navConfig()->fw.loiter_radius;
    anchor.x += exploreCm * cos_approx(exploreAngleRad);
    anchor.y += exploreCm * sin_approx(exploreAngleRad);
    // keep the loiter anchored on the moving estimate (the POSHOLD
    // initialize re-fires per RX cycle and would otherwise re-anchor "here")
    navForcedPosholdAssert(&anchor);

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
    debug[4] = PWM_RANGE_MIN;                 // applied throttle: motor OFF [us]
#endif

    // leave the thermal on the CIRCLE-AVERAGED climb (flight contract,
    // "mitteln ist gut": one turbulent half-circle never bails out; the time
    // constant is one round, derived from bank + speed - no extra parameter),
    // or on the altitude band
    exitMeanMs += (netVario - exitMeanMs) * MIN(dT / periodS, 1.0f);
    if (exitMeanMs < exitMs
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
    // MOTOR FULLY OFF while thermalling (flight contract): a folding prop
    // needs a minimum rpm - idling below it lets the blades flutter and beat
    // the fuselage; fully off they fold cleanly against it. The control
    // surfaces keep flying the loiter. Normal throttle returns the instant
    // the thermal is left or the aircraft sinks below soar_alt_min, both of
    // which drop 'thermalling'.
    if (thermalling) {
        return PWM_RANGE_MIN;
    }
    return throttle;
}

#endif // USE_SOARING

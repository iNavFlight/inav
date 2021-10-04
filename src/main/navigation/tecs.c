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
#include <math.h>

#include "platform.h"

#if defined(USE_NAV)

#include "common/maths.h"
#include "common/filter.h"

#include "sensors/acceleration.h"

#include "fc/rc_controls.h"

#include "navigation/navigation_private.h"

// tecs:Total Energy Control System

// Base frequencies for smoothing pitch
#define NAV_FW_BASE_PITCH_CUTOFF_FREQUENCY_HZ     2.0f

// Calculates the cutoff frequency for smoothing out roll/pitch commands
// control_smoothness valid range from 0 to 9
// resulting cutoff_freq ranging from baseFreq downwards to ~0.11Hz
float getSmoothnessCutoffFreq(float baseFreq)
{
    uint16_t smoothness = 10 - navConfig()->fw.control_smoothness;
    return 0.001f * baseFreq * (float)(smoothness*smoothness*smoothness) + 0.1f;
}

// Position to velocity controller for Z axis
void updateAltitudeVelocityAndPitchController_FW(timeDelta_t deltaMicros)
{
    static pt1Filter_t velzFilterState;

    // On a fixed wing we might not have a reliable climb rate source (if no BARO available), so we can't apply PID controller to
    // velocity error. We use PID controller on altitude error and calculate desired pitch angle

    // Update energies
    const float demSPE = (posControl.desiredState.pos.z * 0.01f) * GRAVITY_MSS;
    const float demSKE = 0.0f;

    const float estSPE = (navGetCurrentActualPositionAndVelocity()->pos.z * 0.01f) * GRAVITY_MSS;
    const float estSKE = 0.0f;

    // speedWeight controls balance between potential and kinetic energy used for pitch controller
    //  speedWeight = 1.0 : pitch will only control airspeed and won't control altitude
    //  speedWeight = 0.5 : pitch will be used to control both airspeed and altitude
    //  speedWeight = 0.0 : pitch will only control altitude
    const float speedWeight = 0.0f; // no speed sensing for now

    const float demSEB = demSPE * (1.0f - speedWeight) - demSKE * speedWeight;
    const float estSEB = estSPE * (1.0f - speedWeight) - estSKE * speedWeight;

    // SEB to pitch angle gain to account for airspeed (with respect to specified reference (tuning) speed
    const float pitchGainInv = 1.0f / 1.0f;

    // Here we use negative values for dive for better clarity
    const float maxClimbDeciDeg = DEGREES_TO_DECIDEGREES(navConfig()->fw.max_climb_angle);
    const float minDiveDeciDeg = -DEGREES_TO_DECIDEGREES(navConfig()->fw.max_dive_angle);

    // PID controller to translate energy balance error [J] into pitch angle [decideg]
    float targetPitchAngle = navPidApply3(&posControl.pids.fw_alt, demSEB, estSEB, US2S(deltaMicros), minDiveDeciDeg, maxClimbDeciDeg, 0, pitchGainInv, 1.0f);

    // Apply low-pass filter to prevent rapid correction
    targetPitchAngle = pt1FilterApply4(&velzFilterState, targetPitchAngle, getSmoothnessCutoffFreq(NAV_FW_BASE_PITCH_CUTOFF_FREQUENCY_HZ), US2S(deltaMicros));

    // Reconstrain pitch angle ( >0 - climb, <0 - dive)
    targetPitchAngle = constrainf(targetPitchAngle, minDiveDeciDeg, maxClimbDeciDeg);
    posControl.rcAdjustment[PITCH] = targetPitchAngle;
}

#endif
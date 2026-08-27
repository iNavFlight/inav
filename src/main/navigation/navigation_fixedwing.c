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

#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/filter.h"

#include "drivers/time.h"

#include "sensors/sensors.h"
#include "sensors/acceleration.h"
#include "sensors/boardalignment.h"
#include "sensors/gyro.h"
#include "sensors/pitotmeter.h"

#include "flight/pid.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/mixer_profile.h"

#include "fc/config.h"
#include "fc/control_profile.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"

#include "navigation/navigation.h"
#include "navigation/navigation_fixedwing_autospeed_logic.h"
#include "navigation/navigation_private.h"

#include "programming/logic_condition.h"

#include "rx/rx.h"

#include "sensors/battery.h"

// Base frequency for smoothing the pitch command and the pitch-to-throttle correction
#define NAV_FW_BASE_PITCH_CUTOFF_FREQUENCY_HZ     2.0f

// Roll-command S-curve smoothing: control_smoothness (0..9) -> easing window = n*100 ms (0 = off,
// max 900 ms). Triggered only on an abrupt commanded-bank step (>20% of the configured roll
// rate between nav loops), then eased over the window and passed 1:1 afterwards. Unlike the previous
// PT1 low-pass this never lags steady tracking, so the controller command stays deterministic.
#define NAV_FW_SMOOTH_TCONST_PER_STEP_MS  100.0f
#define NAV_FW_SMOOTH_TCONST_MAX_MS       900.0f
#define NAV_FW_SMOOTH_STEP_FRACTION       0.2f

// If we are going slower than the minimum ground speed (navConfig()->general.min_ground_speed) - boost throttle to fight against the wind
#define NAV_FW_THROTTLE_SPEED_BOOST_GAIN        1.5f

// FW waypoint turn predictor: clamps for the coordinated-turn radius used to time the FLY_BY turn
#define NAV_FW_TURN_MIN_SPEED        500.0f     // [cm/s] speed floor for the radius calc (low-speed noise guard)
#define NAV_FW_TURN_RADIUS_MIN       1000.0f    // [cm] 10 m  lower clamp
#define NAV_FW_TURN_RADIUS_MAX       30000.0f   // [cm] 300 m upper clamp (matches loiter_radius max)
#define NAV_FW_LOITER_RADIUS_DECAY   100.0f     // [cm/s] max rate the held loiter radius eases back down (1 m/s)
#define NAV_FW_TURN_LEAD_TAN_MAX     3.7f       // tan(half turn angle) cap (~150 deg) to bound the lead distance
#define NAV_FW_ARC_MIN_TURN_ANGLE_CD 3000       // [centideg] only fly the coordinated arc for turns sharper than 30 deg
#define NAV_FW_ARC_RADIAL_GAIN       0.5f       // [centideg bank / cm radial error] pull back onto the arc radius (TBD from flight)
#define NAV_FW_ARC_HEADING_GAIN      0.3f       // [centideg bank / centideg tangent heading error] align to the arc (TBD from flight)
#define NAV_FW_ARC_EXIT_GAIN         2.0f       // [centideg bank / centideg heading error] proportional roll-out capture: bank -> 0 as cog reaches the out-leg (no overshoot)
#define NAV_FW_ARC_EXIT_HANDOFF_CD   300        // [centideg] hand back to the PID within this heading error of the out-leg
                                                // (residual bank is bounded by the separate bank gate, not by this)
#define NAV_FW_ARC_AWAY_TIMEOUT_FACTOR 1.5f     // half away-circle traverse times, before the away arc is abandoned
#define NAV_FW_ARC_SHARP_TURN_CD     15000      // [centideg] beyond this the tangent points explode toward the 180 deg reversal -> capture-only turn

// FW energy/altitude bank guard thresholds (conservative; observable via DEBUG_FW_TURN)
#define NAV_FW_GUARD_PHI_FLOOR_DEG       15.0f    // minimum effective bank limit
#define NAV_FW_GUARD_MIN_BANK_DEG        10.0f    // "banked" threshold
#define NAV_FW_GUARD_VZ_CLIMB_MIN        50.0f    // only guard when commanding a climb above this
#define NAV_FW_GUARD_VZ_DEFICIT_ENTER    150.0f   // filtered Vz deficit to start guarding (Schmitt high)
#define NAV_FW_GUARD_VZ_DEFICIT_EXIT     50.0f    // filtered Vz deficit to stop guarding  (Schmitt low)
#define NAV_FW_GUARD_VZ_FILTER_HZ        0.5f     // deficit/rise low-pass cutoff (rejects wind/thermal/noise)
#define NAV_FW_GUARD_PITCH_FRAC          0.85f    // "near max climb pitch" fraction
#define NAV_FW_GUARD_THROTTLE_MARGIN     50       // "near max throttle" margin
#define NAV_FW_GUARD_REDUCE_RATE_DPS     20.0f    // bank-limit reduce rate
#define NAV_FW_GUARD_RECOVER_RATE_DPS    5.0f     // bank-limit recover rate
#define NAV_FW_GUARD_RECOVER_HOLDOFF_MS  1000     // healthy time before recovery starts

#define NAV_FW_CROSSTRACK_RATE_CUTOFF_HZ 3.0f     // cross-track error rate LPF

// Turn-coordination feed-forward: heading-error window over which the WP-turn FF tapers in (centideg)
#define NAV_FW_FF_HEADING_DEADBAND_CD    500.0f   // below this heading error: no WP-turn FF
#define NAV_FW_FF_HEADING_FULL_CD        3000.0f  // heading error for full WP-turn FF
#define NAV_FW_FF_SLEW_FRACTION          0.2f     // FF slew ceiling as a fraction of the roll rate
#define NAV_FW_LOITER_CAPTURE_BAND       0.15f    // fraction of R: engage the loiter circle controller inside this radial band
#define NAV_FW_LOITER_CAPTURE_ALIGN_CD   4500     // [centideg] and only roughly tangential to the circle
#define NAV_FW_LOITER_RELEASE_BAND       0.5f     // fraction of R: grossly displaced -> hand back to the carrot guidance

// If this is enabled navigation won't be applied if velocity is below 3 m/s
//#define NAV_FW_LIMIT_MIN_FLY_VELOCITY

static bool isPitchAdjustmentValid = false;
static bool isRollAdjustmentValid = false;
static bool isYawAdjustmentValid = false;
static float throttleSpeedAdjustment = 0;
static bool isAutoThrottleManuallyIncreased = false;
static float navCrossTrackError;
static bool fwRollSmoothReseed = false;     // re-sync the roll S-curve smoother on the next frame (after a controller reset)
static float fwRollSmoothSeedCd = 0.0f;     // baseline the smoother re-seeds to (set by the controller reset)
static float fwLastNavRollCmdCd = 0.0f;     // last applied nav roll command [centideg] + timestamp, to tell a
static timeUs_t fwLastNavRollCmdTimeUs = 0; // nav-to-nav transition apart from a pilot handover at reset time
static float fwEffectiveBankLimit = 0.0f;   // adaptive nav bank limit (energy guard), deg; 0 = not yet initialised
static float fwActiveLoiterRadius = 0.0f;   // effective loiter radius in use (cm), for the loiter circle controller
static bool fwArcActive = false;            // arc turn coordinator is driving the turn (-> bank headroom, suppress cross-track, roll override)
static bool fwArcEngaged = false;           // arc coordinator latch across loops; must be cleared on controller reset or a stale arc resumes after a nav interruption
static int32_t fwArcPrevLegBearing = -1;    // last seen WP leg bearing [centideg] for leg-change detection (-1 = unseeded)
static bool fwFlyByCappedLatch = false;     // the pending FLY_BY turn hit the lead-time cap -> fly it direct, not as an arc
static float fwArcBankCmd = 0.0f;           // direct-radius arc bank command [centideg] (Approach B), applied to roll while fwArcActive
static float fwArcHandbackCmdCd = 0.0f;     // arc command at release; the PID/FF command is faded in from it so the
static float fwArcHandbackMs = 0.0f;        // seam is continuous regardless of nav_fw_control_smoothness (0 = no fade)
static float fwArcHandbackDurMs = 0.0f;
static float fwArcEaseMs = 0.0f;            // ease time of the arc in progress, sizes the handback fade
static bool fwArcWasActive = false;         // arc drove the roll last frame, to catch the release edge
static float fwTurnFFCmdCd = 0.0f;          // slew-limited turn feed-forward command [centideg]
static bool fwTurnFFArmed = false;          // FF assists the turn a leg change begins, not later tracking corrections
static int32_t fwTurnFFPrevLegBearing = -1; // last leg bearing seen by the FF arming logic (-1 = unseeded)
static float fwArcPickupAlong = 0.0f;       // along-track distance to the second-arc pickup [cm], for the log
static int8_t loiterDirYaw = 1;
static bool needToCalculateCircularLoiter;
static bool autoSpeedIsActive = false;
static bool autoSpeedAirspeedBoost = false;
static bool autoSpeedNeedsFreshSample = true;
static uint16_t autoSpeedThrottleCommand = PWM_RANGE_MIDDLE;
static timeUs_t autoSpeedLastUpdateTimeUs = 0;
static pt1Filter_t speedToThrFilterState;

// Calculates the cutoff frequency for smoothing out roll/pitch commands
// control_smoothness valid range from 0 to 9
// resulting cutoff_freq ranging from baseFreq downwards to ~0.11Hz
static float getSmoothnessCutoffFreq(float baseFreq)
{
    uint16_t smoothness = 10 - navConfig()->fw.control_smoothness;
    return 0.001f * baseFreq * (float)(smoothness*smoothness*smoothness) + 0.1f;
}

// Calculates the cutoff frequency for smoothing out pitchToThrottleCorrection
// pitch_to_throttle_smooth valid range from 0 to 9
// resulting cutoff_freq ranging from baseFreq downwards to ~0.01Hz
static float getPitchToThrottleSmoothnessCutoffFreq(float baseFreq)
{
    uint16_t smoothness = 10 - navConfig()->fw.pitch_to_throttle_smooth;
    return 0.001f * baseFreq * (float)(smoothness*smoothness*smoothness) + 0.01f;
}

/*-----------------------------------------------------------
 * Altitude controller
 *-----------------------------------------------------------*/
static pt1Filter_t pitchFilterState;
static pt1Filter_t pitchToThrFilterState;

void setupFixedWingAltitudeController(void)
{
    // TODO
}

void resetFixedWingAltitudeController(void)
{
    navPidReset(&posControl.pids.fw_alt);
    posControl.rcAdjustment[PITCH] = 0;
    isPitchAdjustmentValid = false;
    throttleSpeedAdjustment = 0;

    pt1FilterSetCutoff(&pitchFilterState, getSmoothnessCutoffFreq(NAV_FW_BASE_PITCH_CUTOFF_FREQUENCY_HZ));
    pt1FilterReset(&pitchFilterState, 0.0f);
    pt1FilterSetCutoff(&pitchToThrFilterState, getPitchToThrottleSmoothnessCutoffFreq(NAV_FW_BASE_PITCH_CUTOFF_FREQUENCY_HZ));
    pt1FilterReset(&pitchToThrFilterState, 0.0f);
}

bool adjustFixedWingAltitudeFromRCInput(void)
{
    int16_t rcAdjustment = applyDeadbandRescaled(rcCommand[PITCH], rcControlsConfig()->alt_hold_deadband, -500, 500);

    if (rcAdjustment) {
        // set velocity proportional to stick movement
        float rcClimbRate = -rcAdjustment * navConfig()->fw.max_manual_climb_rate / (500.0f - rcControlsConfig()->alt_hold_deadband);
        updateClimbRateToAltitudeController(rcClimbRate, 0, ROC_TO_ALT_CONSTANT);
        return true;
    }
    else {
        // Adjusting finished - reset desired position to stay exactly where pilot released the stick
        if (posControl.flags.isAdjustingAltitude) {
            updateClimbRateToAltitudeController(0, 0, ROC_TO_ALT_CURRENT);
        }
        return false;
    }
}

// Position to velocity controller for Z axis
static void updateAltitudeVelocityAndPitchController_FW(timeDelta_t deltaMicros)
{

    float desiredClimbRate = getDesiredClimbRate(posControl.desiredState.pos.z, deltaMicros);

    // Reduce max allowed climb rate by 2/3 if performing loiter (stall prevention)
    if (navGetCurrentStateFlags() & NAV_CTL_HOLD && desiredClimbRate > 0.67f * navConfig()->fw.max_auto_climb_rate) {
        desiredClimbRate = 0.67f * navConfig()->fw.max_auto_climb_rate;
    }

    // Here we use negative values for dive for better clarity
    const float maxClimbDeciDeg = DEGREES_TO_DECIDEGREES(navConfig()->fw.max_climb_angle);
    const float minDiveDeciDeg = -DEGREES_TO_DECIDEGREES(navConfig()->fw.max_dive_angle);

    // Default control based on climb rate (velocity)
    float targetValue = desiredClimbRate;
    float measuredValue = navGetCurrentActualPositionAndVelocity()->vel.z;

    // Optional control based on altitude (position)
    if (pidProfile()->fwAltControlUsePos) {
        static float currentDesiredPosZ = 0.0f;
        static float trackingAltitude = 0.0f;
        static bool altitudeRateControlActive = false;

        float desiredAltitude = posControl.desiredState.pos.z;
        float currentAltitude = navGetCurrentActualPositionAndVelocity()->pos.z;

        /* Determine if altitude rate control required based on magnitude of change in target altitude.
         * No rate control used during trackback to allow max climb rates based on pitch limits */
        if (fabsf(currentDesiredPosZ - desiredAltitude) > 100.0f || !isPitchAdjustmentValid) {
            altitudeRateControlActive = desiredClimbRate && fabsf(desiredAltitude - currentAltitude) > navConfig()->fw.max_auto_climb_rate &&
                                        !posControl.flags.rthTrackbackActive;
            trackingAltitude = currentAltitude;
        }
        currentDesiredPosZ = desiredAltitude;

        if (posControl.flags.rocToAltMode == ROC_TO_ALT_CONSTANT || altitudeRateControlActive) {
            /* Adjustment factor based on vertical acceleration used to better control altitude position change
             * driving vertical velocity control. Helps avoid lag induced overcontrol by PID loop. */
            static float absLastClimbRate = 0.0f;
            float absClimbRate = fabsf(measuredValue);
            float accelerationZ = (absClimbRate - absLastClimbRate) / US2S(deltaMicros);
            absLastClimbRate = absClimbRate;
            float adjustmentFactor = constrainf(scaleRangef(accelerationZ, 0.0f, 200.0f, 1.0f, 0.0f), 0.0f, 1.0f);

            if (posControl.flags.rocToAltMode == ROC_TO_ALT_CONSTANT) {
                posControl.desiredState.pos.z += adjustmentFactor * desiredClimbRate * US2S(deltaMicros);
                desiredAltitude = posControl.desiredState.pos.z;
            } else {
                /* Disable rate control if no longer required, i.e. remaining altitude change is small */
                altitudeRateControlActive = fabsf(desiredAltitude - currentAltitude) > MAX(fabsf(desiredClimbRate), 50.0f);

                /* Tracking altitude used to control altitude rate where changing posControl.desiredState.pos.z not possible */
                trackingAltitude += adjustmentFactor * desiredClimbRate * US2S(deltaMicros);
                desiredAltitude = trackingAltitude;
            }
        } else {
            desiredClimbRate = 0;
        }

        targetValue = desiredAltitude;
        measuredValue = currentAltitude;
    }

    // PID controller to translate desired target error (velocity or position) into pitch angle [decideg]
    float targetPitchAngle = navPidApply2(&posControl.pids.fw_alt, targetValue, measuredValue, US2S(deltaMicros), minDiveDeciDeg, maxClimbDeciDeg, 0);

    // Apply low-pass filter to prevent rapid correction
    targetPitchAngle = pt1FilterApply3(&pitchFilterState, targetPitchAngle, US2S(deltaMicros));

    // Reconstrain pitch angle (> 0 - climb, < 0 - dive)
    targetPitchAngle = constrainf(targetPitchAngle, minDiveDeciDeg, maxClimbDeciDeg);
    posControl.rcAdjustment[PITCH] = targetPitchAngle;

    posControl.desiredState.vel.z = desiredClimbRate;
    navDesiredVelocity[Z] = constrain(lrintf(posControl.desiredState.vel.z), -32678, 32767);
}

void applyFixedWingAltitudeAndThrottleController(timeUs_t currentTimeUs)
{
    static timeUs_t previousTimePositionUpdate = 0;         // Occurs @ altitude sensor update rate (max MAX_ALTITUDE_UPDATE_RATE_HZ)

    if ((posControl.flags.estAltStatus >= EST_USABLE)) {
        if (posControl.flags.verticalPositionDataNew) {
            const timeDeltaLarge_t deltaMicrosPositionUpdate = currentTimeUs - previousTimePositionUpdate;
            previousTimePositionUpdate = currentTimeUs;

            // Check if last correction was not too long ago
            if (deltaMicrosPositionUpdate < MAX_POSITION_UPDATE_INTERVAL_US) {
                updateAltitudeVelocityAndPitchController_FW(deltaMicrosPositionUpdate);
                isPitchAdjustmentValid = true;
            }
            else {
                // Position update has not occurred in time (first iteration or glitch), reset altitude controller
                resetFixedWingAltitudeController();
            }

            // Indicate that information is no longer usable
            posControl.flags.verticalPositionDataConsumed = true;
        }
    }
    else {
        // No valid altitude sensor data, don't adjust pitch automatically, rcCommand[PITCH] is passed through to PID controller
        isPitchAdjustmentValid = false;
    }
}

/*-----------------------------------------------------------
 * Adjusts desired heading from pilot's input
 *-----------------------------------------------------------*/
bool adjustFixedWingHeadingFromRCInput(void)
{
    if (ABS(rcCommand[YAW]) > rcControlsConfig()->pos_hold_deadband) {
        return true;
    }

    return false;
}

/*-----------------------------------------------------------
 * XY-position controller
 *-----------------------------------------------------------*/
static fpVector3_t virtualDesiredPosition;
static pt1Filter_t fwCrossTrackErrorRateFilterState;

/*
 * TODO Currently this function resets both FixedWing and Rover & Boat position controller
 */
void resetFixedWingPositionController(void)
{
    virtualDesiredPosition.x = 0;
    virtualDesiredPosition.y = 0;
    virtualDesiredPosition.z = 0;
    fwArcActive = false;
    fwArcEngaged = false;
    fwArcWasActive = false;
    fwArcHandbackDurMs = 0.0f;
    fwTurnFFCmdCd = 0.0f;
    fwTurnFFArmed = false;
    fwTurnFFPrevLegBearing = -1;
    fwArcPrevLegBearing = -1;
    fwFlyByCappedLatch = false;

    navPidReset(&posControl.pids.fw_nav);
    navPidReset(&posControl.pids.fw_heading);
    posControl.rcAdjustment[ROLL] = 0;
    posControl.rcAdjustment[YAW] = 0;
    isRollAdjustmentValid = false;
    isYawAdjustmentValid = false;

    // Re-seed the roll S-curve smoother. If nav commanded roll until just now (nav-mode to nav-mode
    // transition, e.g. RTH -> CRUISE) seed from the last applied command so the level-off/turn change
    // is eased; after a pilot-flown phase seed neutral so a roll-out in progress is not re-commanded.
    fwRollSmoothSeedCd = ((micros() - fwLastNavRollCmdTimeUs) < MAX_POSITION_UPDATE_INTERVAL_US) ? fwLastNavRollCmdCd : 0.0f;
    fwRollSmoothReseed = true;

    // Reset the energy-guard bank limit; 0 = use full ceiling until the guard re-syncs on its next run
    fwEffectiveBankLimit = 0.0f;

    pt1FilterSetCutoff(&fwCrossTrackErrorRateFilterState, NAV_FW_CROSSTRACK_RATE_CUTOFF_HZ);
    pt1FilterReset(&fwCrossTrackErrorRateFilterState, 0.0f);
}

static int8_t loiterDirection(void) {
    int8_t dir = 1; //NAV_LOITER_RIGHT

    if (navConfig()->fw.loiter_direction == NAV_LOITER_LEFT) {
        dir = -1;
    }

    if (navConfig()->fw.loiter_direction == NAV_LOITER_YAW) {

        if (rcCommand[YAW] < -250) {
            loiterDirYaw = 1; //RIGHT //yaw is contrariwise
        }

        if (rcCommand[YAW] > 250) {
            loiterDirYaw = -1; //LEFT  //see annexCode in fc_core.c
        }

        dir = loiterDirYaw;
    }

    if (IS_RC_MODE_ACTIVE(BOXLOITERDIRCHN)) {
        dir *= -1;
    }

#ifdef USE_GEOZONE
    if (geozone.loiterDir != 0) {
        dir = geozone.loiterDir;
    }
#endif

    return dir;
}

// Triggered S-curve roll-in [centideg]: on an abrupt commanded-bank step (new heading), ease toward the
// target with a smoothstep over a control_smoothness-derived time constant, then pass 1:1. The timer is
// not reset by further steps mid-ramp, so we never get stuck damping steady tracking.
static float applyFwRollInSmoothing(float rollTargetCd, timeDelta_t deltaMicros, bool reseed)
{
    static float prevTarget = 0.0f;
    static float prevRate = 0.0f;
    static float rampStart = 0.0f;
    static float prevOut = 0.0f;
    static float elapsedMs = 0.0f;
    static bool active = false;

    if (reseed) {                                               // controller reset: re-seed to the baseline chosen at reset time
        active = false;                                          // (last nav command on a nav-to-nav transition, else neutral), so
        elapsedMs = 0.0f;                                        // the cross-mode command step is detected and eased while stale
        prevTarget = fwRollSmoothSeedCd;                         // state can never fire a spurious ramp
        prevRate = 0.0f;
        prevOut = fwRollSmoothSeedCd;
    }

    const float tConstMs = MIN((float)navConfig()->fw.control_smoothness * NAV_FW_SMOOTH_TCONST_PER_STEP_MS, NAV_FW_SMOOTH_TCONST_MAX_MS);
    const float dtS = US2S(deltaMicros);
    if (tConstMs <= 0.0f || dtS <= 0.0f) {                       // smoothing off: pass through
        active = false;
        prevTarget = rollTargetCd;
        prevRate = 0.0f;
        prevOut = rollTargetCd;
        return rollTargetCd;
    }

    const float cmdRate = (rollTargetCd - prevTarget) / dtS;     // commanded bank rate [centideg/s]
    const float stepThreshold = NAV_FW_SMOOTH_STEP_FRACTION * (currentControlProfile->stabilized.rates[FD_ROLL] * 10.0f) * 100.0f;  // 20% of roll rate [centideg/s]
    if (!active && fabsf(cmdRate - prevRate) > stepThreshold) {  // abrupt setpoint-rate change -> start the S-curve
        active = true;
        elapsedMs = 0.0f;
        rampStart = prevOut;
    }

    float out = rollTargetCd;                                    // default: 1:1 pass-through
    if (active) {
        elapsedMs += dtS * 1000.0f;                             // timer does NOT reset on further steps
        if (elapsedMs >= tConstMs) {
            active = false;                                      // window elapsed -> back to 1:1
        } else {
            const float p = elapsedMs / tConstMs;
            const float s = p * p * (3.0f - 2.0f * p);          // smoothstep (S-curve)
            out = rampStart + s * (rollTargetCd - rampStart);
        }
    }

    prevTarget = rollTargetCd;
    prevRate = cmdRate;
    prevOut = out;
    return out;
}

// Hard roll ceiling [deg]: the global angle-mode limit (max_angle_inclination_rll)
static float getFwBankCeilingDeg(void)
{
    return (float)pidProfile()->max_angle_inclination[FD_ROLL] / 10.0f;
}

// Output bank ceiling [deg]: hard ceiling reduced by the energy guard; nav_fw_bank_angle is only the planning target
static float getFwEffectiveBankLimit(void)
{
    const float ceiling = getFwBankCeilingDeg();
    return (fwEffectiveBankLimit > 0.0f) ? MIN(fwEffectiveBankLimit, ceiling) : ceiling;
}

// Planning bank [deg] for sizing turn/loiter radii: the target, capped by the guard ceiling
static float getFwPlanningBankDeg(void)
{
    return MIN((float)navConfig()->fw.max_bank_angle, getFwEffectiveBankLimit());
}

// Roll-command bank limit [deg]: an active arc may use the reserve up to the ceiling to hold the radius against wind; everywhere else the target
static float getFwControlBankLimit(void)
{
    return fwArcActive ? getFwEffectiveBankLimit() : getFwPlanningBankDeg();
}

// Reduce the bank ceiling when a commanded climb stalls near the pitch/throttle limit while banked, so the turn widens and the climb recovers
static void updateFwEnergyBankGuard(timeUs_t currentTimeUs, uint16_t autoThrottleValue)
{
    static timeUs_t lastUpdateUs = 0;
    static timeUs_t lastTriggerUs = 0;
    static bool deficitLatched = false;
    static bool bankedPrev = false;
    static float targetVzBaseline = 0.0f;
    static pt1Filter_t deficitFilter;
    static pt1Filter_t riseFilter;

    const float maxBank = getFwBankCeilingDeg();
    const float targetBank = MIN((float)navConfig()->fw.max_bank_angle, maxBank);   // planning target = guard snap level
    if (fwEffectiveBankLimit <= 0.0f) {
        fwEffectiveBankLimit = maxBank;
    }

    const timeDeltaLarge_t dtUs = currentTimeUs - lastUpdateUs;
    lastUpdateUs = currentTimeUs;
    // First call / gap (controller was inactive): resync, skip integration this step.
    if (dtUs <= 0 || dtUs > MAX_POSITION_UPDATE_INTERVAL_US) {
        targetVzBaseline = posControl.desiredState.vel.z;
        pt1FilterSetCutoff(&deficitFilter, NAV_FW_GUARD_VZ_FILTER_HZ);
        pt1FilterSetCutoff(&riseFilter, NAV_FW_GUARD_VZ_FILTER_HZ);
        pt1FilterReset(&deficitFilter, 0.0f);
        pt1FilterReset(&riseFilter, 0.0f);
        deficitLatched = false;
        lastTriggerUs = currentTimeUs;
        return;
    }
    const float dtSec = US2S(dtUs);

    const float bankDeg = fabsf((float)posControl.rcAdjustment[ROLL]) / 10.0f;  // rcAdjustment is decidegrees
    const bool banked = bankDeg > NAV_FW_GUARD_MIN_BANK_DEG;

    // Latch target Vz at bank entry as the pre-bank reference.
    if (banked && !bankedPrev) {
        targetVzBaseline = posControl.desiredState.vel.z;
    }
    bankedPrev = banked;

    const float targetVz = posControl.desiredState.vel.z;                       // cm/s
    const float actualVz = navGetCurrentActualPositionAndVelocity()->vel.z;     // cm/s

    // Signal A: unmet climb demand. Signal B: bank-induced rise of the demand since bank entry.
    const float deficit = pt1FilterApply3(&deficitFilter, targetVz - actualVz, dtSec);
    const float rise    = pt1FilterApply3(&riseFilter, targetVz - targetVzBaseline, dtSec);

    // Schmitt trigger + deadband on the filtered deficit (rejects fluctuations).
    if (deficit > NAV_FW_GUARD_VZ_DEFICIT_ENTER) {
        deficitLatched = true;
    } else if (deficit < NAV_FW_GUARD_VZ_DEFICIT_EXIT) {
        deficitLatched = false;
    }

    const float maxClimbDeciDeg = DEGREES_TO_DECIDEGREES((float)navConfig()->fw.max_climb_angle);
    const bool nearPitchLimit = (float)posControl.rcAdjustment[PITCH] >= NAV_FW_GUARD_PITCH_FRAC * maxClimbDeciDeg;
    // AUTO throttle demand only: manual full throttle must not permanently arm the guard
    const bool nearThrottleLimit = autoThrottleValue >= (currentBatteryProfile->nav.fw.max_throttle - NAV_FW_GUARD_THROTTLE_MARGIN);
    const bool climbCommanded = targetVz > NAV_FW_GUARD_VZ_CLIMB_MIN;

    const bool trigger = banked && climbCommanded && deficitLatched && (nearPitchLimit || nearThrottleLimit);

    // Bank-induced rise (signal B) -> react faster.
    const float reduceRate = (rise > NAV_FW_GUARD_VZ_DEFICIT_ENTER) ? (2.0f * NAV_FW_GUARD_REDUCE_RATE_DPS) : NAV_FW_GUARD_REDUCE_RATE_DPS;

    if (trigger) {
        fwEffectiveBankLimit = MIN(fwEffectiveBankLimit, targetBank);    // drop headroom at once: snap to the planning target
        fwEffectiveBankLimit -= reduceRate * dtSec;                      // then keep easing down toward the floor
        lastTriggerUs = currentTimeUs;
    } else if ((currentTimeUs - lastTriggerUs) > ((timeUs_t)NAV_FW_GUARD_RECOVER_HOLDOFF_MS * 1000)) {
        fwEffectiveBankLimit += NAV_FW_GUARD_RECOVER_RATE_DPS * dtSec;
    }
    fwEffectiveBankLimit = constrainf(fwEffectiveBankLimit, NAV_FW_GUARD_PHI_FLOOR_DEG, maxBank);

    DEBUG_SET(DEBUG_FW_TURN, 6, lrintf(fwEffectiveBankLimit));  // energy-guard bank ceiling [deg]
}

// Coordinated-turn radius R = V^2/(g*tan(phi)) [cm], clamped. Times the FLY_BY turn for any speed.
static float getFwCoordinatedTurnRadius(void)
{
    const float speed = MAX(posControl.actualState.velXY, NAV_FW_TURN_MIN_SPEED);   // cm/s
    const float bankRad = DEGREES_TO_RADIANS(getFwPlanningBankDeg());                // plan for the target bank
    const float radius = (speed * speed) / (GRAVITY_CMSS * tan_approx(bankRad));     // cm
    return constrainf(radius, NAV_FW_TURN_RADIUS_MIN, NAV_FW_TURN_RADIUS_MAX);
}

// Coordinated-turn feed-forward bank [centideg] for the active WP turn (loiter has its own circle controller).
static float getFwTurnFeedForward(int32_t navHeadingError, timeDelta_t deltaMicros)
{
    const uint8_t ffGain = navConfig()->fw.turn_ff_gain;
    if (ffGain == 0 || posControl.actualState.velXY <= NAV_FW_TURN_MIN_SPEED
        || (navConfig()->fw.wp_turn_mode == NAV_FW_WP_TURN_DIRECT && posControl.navState != NAV_STATE_FW_LANDING_APPROACH)) {
        return 0.0f;                                    // DIRECT = pure legacy PID (landing approach forces FLY_BY)
    }

    // Turn-context gate: arm on a leg change, disarm once aligned. A later heading error on the same
    // leg is a path-tracking correction (cross-track carrot), and the full coordinated bank on top of
    // the PID turns every leg capture into an S-shaped swing.
    const int32_t ffLegBearing = posControl.activeWaypoint.bearing;
    if (fwTurnFFPrevLegBearing < 0 || ABS(wrap_18000(ffLegBearing - fwTurnFFPrevLegBearing)) > 500) {
        fwTurnFFArmed = true;
    }
    fwTurnFFPrevLegBearing = ffLegBearing;
    if (ABS(navHeadingError) <= NAV_FW_FF_HEADING_DEADBAND_CD) {
        fwTurnFFArmed = false;
    }

    float ffRadius = 0.0f;
    float ffSign = 0.0f;
    if (fwTurnFFArmed && !needToCalculateCircularLoiter && isWaypointNavTrackingActive() && ABS(navHeadingError) > NAV_FW_FF_HEADING_DEADBAND_CD) {
        ffRadius = getFwCoordinatedTurnRadius();        // WP turn: dynamic radius, tapered by heading error
        // Taper from the deadband edge, not from zero: measuring from zero leaves a step at the gate
        ffSign = (navHeadingError > 0 ? 1.0f : -1.0f)
               * constrainf(((float)ABS(navHeadingError) - NAV_FW_FF_HEADING_DEADBAND_CD)
                            / (NAV_FW_FF_HEADING_FULL_CD - NAV_FW_FF_HEADING_DEADBAND_CD), 0.0f, 1.0f);
    }

    float rollFF = 0.0f;
    if (ffRadius > 0.0f) {
        const float v = posControl.actualState.velXY;
        const float phiFFcd = DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(atan2_approx(v * v, GRAVITY_CMSS * ffRadius)));
        rollFF = ffSign * phiFFcd * (ffGain / 100.0f);
    }

    // Slew limit, not a filter: heading error and velXY arrive at GPS rate while this runs at nav rate,
    // so the FF is a staircase. Rate limiting spreads the steps without lagging a settled command.
    const float maxStepCd = NAV_FW_FF_SLEW_FRACTION * (currentControlProfile->stabilized.rates[FD_ROLL] * 10.0f)
                          * 100.0f * US2S(deltaMicros);
    fwTurnFFCmdCd += constrainf(rollFF - fwTurnFFCmdCd, -maxStepCd, maxStepCd);

    DEBUG_SET(DEBUG_FW_TURN, 5, lrintf(fwTurnFFCmdCd));         // turn/loiter roll feed-forward [centideg]
    return fwTurnFFCmdCd;
}

// Stabilised loiter-radius floor [cm]: the raw requirement swings with wind (v^2) and would make the
// tracker thrash - ratchet up instantly, hold the peak one revolution, ease down at <= DECAY
static uint32_t getFwStableLoiterRadius(uint32_t configuredRadius, float bearingFromCenterRad, bool loiterActive, timeDelta_t deltaMicros)
{
    static bool  active = false;
    static float commandedHold = 0.0f;
    static float decayTarget = 0.0f;
    static float revPeak = 0.0f;
    static float netAngle = 0.0f;
    static float prevBearing = 0.0f;

    const float speed = MAX(posControl.actualState.velXY, NAV_FW_TURN_MIN_SPEED);
    const float required = constrainf((speed * speed) / (GRAVITY_CMSS * tan_approx(DEGREES_TO_RADIANS(getFwPlanningBankDeg()))),
                                      NAV_FW_TURN_RADIUS_MIN, NAV_FW_TURN_RADIUS_MAX);

    if (!loiterActive) {
        active = false;
        commandedHold = required;                       // transit / WP turn: track instantaneously
    } else {
        if (!active) {                                  // loiter entry: seed (no decay until the first revolution)
            active = true;
            commandedHold = required;
            decayTarget = NAV_FW_TURN_RADIUS_MAX;
            revPeak = required;
            netAngle = 0.0f;
            prevBearing = bearingFromCenterRad;
        }
        revPeak = MAX(revPeak, required);
        commandedHold = MAX(commandedHold, required);   // ratchet up immediately (safety)

        float dAng = bearingFromCenterRad - prevBearing;
        if (dAng >  M_PIf) dAng -= 2.0f * M_PIf;
        if (dAng < -M_PIf) dAng += 2.0f * M_PIf;
        netAngle += dAng;                               // signed net rotation about the centre
        prevBearing = bearingFromCenterRad;

        if (fabsf(netAngle) >= 2.0f * M_PIf) {          // a full revolution -> this revolution's peak is the decay target
            decayTarget = revPeak;
            revPeak = required;
            netAngle = 0.0f;
        }

        if (commandedHold > decayTarget) {              // ease down gradually, never below the current need
            commandedHold -= NAV_FW_LOITER_RADIUS_DECAY * US2S(deltaMicros);
            commandedHold = MAX(commandedHold, MAX(decayTarget, required));
        }
    }

    const uint32_t out = (uint32_t)MAX((float)configuredRadius, commandedHold);
    DEBUG_SET(DEBUG_FW_TURN, 0, lrintf(out));           // active turn/loiter radius [cm] (overridden by FLY_BY/arc writers)
    return out;
}

// Roll-in/out ease time [ms] from roll rate, control_smoothness and the servo/inertia margin -
// single source of the turn's roll dynamics (the S-curve smoother is bypassed during the arc)
static float fwTurnEaseTimeMs(float phiNomDeg)
{
    const float rollRateDps = currentControlProfile->stabilized.rates[FD_ROLL] * 10.0f;
    const float rollMs = (rollRateDps > 1.0f) ? (1.5f * phiNomDeg / rollRateDps * 1000.0f) : 0.0f;
    const float csMs = MIN((float)navConfig()->fw.control_smoothness * NAV_FW_SMOOTH_TCONST_PER_STEP_MS, NAV_FW_SMOOTH_TCONST_MAX_MS);
    return rollMs + csMs + (float)navConfig()->fw.wp_turn_control_ease;
}

// Crossfade out of an arc handback: the arc leaves the wings near level while the PID and turn FF
// still see the full carrot error, so adopting their command directly is a step of several degrees.
static float applyFwArcHandbackFade(float rollTargetCd, timeDelta_t deltaMicros)
{
    if (fwArcHandbackDurMs <= 0.0f) {
        return rollTargetCd;
    }

    fwArcHandbackMs += US2S(deltaMicros) * 1000.0f;
    const float p = constrainf(fwArcHandbackMs / fwArcHandbackDurMs, 0.0f, 1.0f);
    if (p >= 1.0f) {
        fwArcHandbackDurMs = 0.0f;
        return rollTargetCd;
    }

    const float s = p * p * (3.0f - 2.0f * p);
    return fwArcHandbackCmdCd + (rollTargetCd - fwArcHandbackCmdCd) * s;
}

// Arc turn coordinator: bank ramp -> coordinated arc (radius + tangent feedback) -> predictive
// capture roll-out. Sets fwArcActive (drives the roll directly).
static void updateFwTurnArc(timeDelta_t deltaMicros)
{
    enum { ARC_RAMP_IN = 0, ARC_STEADY, ARC_CAPTURE };
    static uint8_t phase;
    static float   arcCx, arcCy, arcR;
    static int8_t  arcDir;
    static int32_t arcOutBearing;
    static bool    arcToLegLine;        // fallback capture: converge onto the leg line itself, not just its course
    static float   phiNomCd;            // coordinated nominal bank for this turn [centideg]
    static float   tEaseMs;             // roll-in ease time
    static float   rampMs;              // elapsed time in the ramp-in phase
    static float   rampStartCd;         // bank the ramp blends from (the live nav command on entry - a banked
                                        // loiter exit must not level off first; the pickup blends mid-arc)
    static float   steadyMs;            // elapsed time in the steady phase
    static float   steadyStartCd;       // bank the steady law blends from (the ramp's final command)

    // S sequencer (FLY_INTO / FLY_OVER-tracking): first arc, roll-reversal gap, second arc
    enum { FW_INTO_IDLE = 0, FW_INTO_AWAY, FW_INTO_MAIN, FW_INTO_DONE };
    static uint8_t intoStage;
    static float   intoEx, intoEy;      // second-arc pickup point (internal-tangent touch)
    static float   intoO2x, intoO2y;    // second-arc centre
    static float   intoR;
    static int32_t intoBOut;
    static int8_t  intoDir;
    static float   intoAwayMs;          // time spent in the away arc, to bound a pickup that never triggers

    fwArcActive = false;

    // Landing approach always flies coordinated FLY_BY turns, whatever mode is configured
    navFwWpTurnMode_e turnMode = navConfig()->fw.wp_turn_mode;
    if (posControl.navState == NAV_STATE_FW_LANDING_APPROACH) {
        turnMode = NAV_FW_WP_TURN_COORD_FLY_BY;
    }

    const bool wpTracking = isWaypointNavTrackingActive() && !needToCalculateCircularLoiter;
    if (turnMode == NAV_FW_WP_TURN_DIRECT || !wpTracking) {
        fwArcEngaged = false;
        fwArcPrevLegBearing = -1;
        intoStage = FW_INTO_IDLE;
        DEBUG_SET(DEBUG_FW_TURN, 1, 0);     // clear, else the log keeps showing the last arc phase for minutes
        DEBUG_SET(DEBUG_FW_TURN, 3, 0);
        return;
    }

    const int32_t legBearing = posControl.activeWaypoint.bearing;
    const int32_t cog = posControl.actualState.cog;
    const fpVector3_t *pos = &navGetCurrentActualPositionAndVelocity()->pos;
    const float v = posControl.actualState.velXY;

    if (!fwArcEngaged) {
        arcToLegLine = false;
        /* Unseeded (loiter/reset wipes the reference every cycle) means a WP advance landing on the
         * first tracked cycle is invisible - the loiter exit then never engages and the PID+FF fly
         * the whole turn unshaped. Unseeded + grossly off the leg course = a pending turn. */
        const bool legChanged = (fwArcPrevLegBearing >= 0)
            ? (ABS(wrap_18000(legBearing - fwArcPrevLegBearing)) > 500)
            : (ABS(wrap_18000(legBearing - cog)) > NAV_FW_ARC_MIN_TURN_ANGLE_CD);
        fwArcPrevLegBearing = legBearing;
        if (legChanged) {
            intoStage = FW_INTO_IDLE;                           // a new leg invalidates any staged S geometry
            const bool capped = fwFlyByCappedLatch;             // lead-time-capped FLY_BY: the tangent geometry no longer fits
            fwFlyByCappedLatch = false;                         // consume the latch on any leg change
            if (turnMode == NAV_FW_WP_TURN_COORD_FLY_OVER) {
                // FLY_OVER: circle pinned at the overfly point. Tracking OFF: exit on the tangent
                // through the next WP; tracking ON: bounded-intercept S onto the new leg itself.
                const float px = posControl.activeWaypoint.pos.x;
                const float py = posControl.activeWaypoint.pos.y;
                const int32_t brgToWp = wrap_36000(lrintf(DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(atan2_approx(py - pos->y, px - pos->x)))));
                const int32_t toWpErr = wrap_18000(brgToWp - cog);
                if (ABS(toWpErr) > NAV_FW_ARC_MIN_TURN_ANGLE_CD) {
                    const float arcRtmp = getFwCoordinatedTurnRadius();
                    const float phiTmp = DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(atan2_approx(v * v, GRAVITY_CMSS * arcRtmp)));
                    const float tTmp = fwTurnEaseTimeMs(CENTIDEGREES_TO_DEGREES(phiTmp));
                    const int8_t dirTmp = (toWpErr > 0) ? 1 : -1;
                    const float cogRad = CENTIDEGREES_TO_RADIANS((float)cog);
                    // Pin ahead by the roll-in drift so the ramp ends ON the circle
                    const float leadDist = 1.5f * v * (tTmp / 1000.0f);
                    const float cx = pos->x + leadDist * cos_approx(cogRad) + arcRtmp * cos_approx(cogRad + dirTmp * (M_PIf * 0.5f));
                    const float cy = pos->y + leadDist * sin_approx(cogRad) + arcRtmp * sin_approx(cogRad + dirTmp * (M_PIf * 0.5f));
                    if (navConfig()->fw.wp_tracking_accuracy && (navGetCurrentStateFlags() & NAV_AUTO_WP)) {
                        // Tracking ON: exit the main arc onto a BOUNDED intercept course (<= 45 deg to the
                        // leg, gamma = half the turn for shallow corners), short straight for the reverse
                        // roll, then a standard corner-cut arc rolls out ON the line
                        const float legRad = CENTIDEGREES_TO_RADIANS((float)legBearing);
                        const float ux = cos_approx(legRad), uy = sin_approx(legRad);
                        const int32_t turnCd = wrap_18000(legBearing - cog);
                        const float gammaRad = CENTIDEGREES_TO_RADIANS(constrainf(0.5f * (float)ABS(turnCd), 2000.0f, 4500.0f));
                        const float icptRad = legRad + (float)dirTmp * gammaRad;
                        const float d1x = cos_approx(icptRad), d1y = sin_approx(icptRad);
                        const float nAng = icptRad - (float)dirTmp * (M_PIf * 0.5f);
                        const float p1x = cx + 2.0f * arcRtmp * cos_approx(nAng);   // intercept line shifted R toward the counter side
                        const float p1y = cy + 2.0f * arcRtmp * sin_approx(nAng);
                        const float lAng = legRad - (float)dirTmp * (M_PIf * 0.5f);
                        const float b0x = px + arcRtmp * cos_approx(lAng);
                        const float b0y = py + arcRtmp * sin_approx(lAng);
                        const float cross = d1x * uy - d1y * ux;
                        if (fabsf(cross) > 0.17f) {             // gamma >= 20 deg keeps the lines well separated
                            const float tt = ((b0x - p1x) * uy - (b0y - p1y) * ux) / cross;
                            const float o2x = p1x + tt * d1x;
                            const float o2y = p1y + tt * d1y;
                            const float rollAlong = (o2x - px) * ux + (o2y - py) * uy;
                            const float legLen = calc_length_pythagorean_2D(px - pos->x, py - pos->y);
                            // roll-out tangency must lie ahead of us and leave straight leg to the WP
                            if (rollAlong < 0.0f && -rollAlong < legLen && -rollAlong > 2.0f * arcRtmp) {
                                intoEx = o2x - arcRtmp * cos_approx(nAng);      // pickup = tangency on the intercept line
                                intoEy = o2y - arcRtmp * sin_approx(nAng);
                                intoO2x = o2x; intoO2y = o2y;
                                intoR = arcRtmp; intoBOut = legBearing; intoDir = -dirTmp;
                                fwArcEngaged = true;
                                phase = ARC_RAMP_IN;
                                rampMs = 0.0f;
                                rampStartCd = fwLastNavRollCmdCd;
                                arcR = arcRtmp;
                                arcDir = dirTmp;
                                arcCx = cx;
                                arcCy = cy;
                                arcOutBearing = wrap_36000(lrintf(DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(icptRad))));
                                phiNomCd = phiTmp;
                                tEaseMs = tTmp;
                                intoStage = FW_INTO_AWAY;       // second arc staged: the shared pickup logic takes over
                            }
                        }
                    }
                    const float dCP = calc_length_pythagorean_2D(px - cx, py - cy);
                    if (!fwArcEngaged && dCP > 1.05f * arcRtmp) {   // next WP outside the circle: a tangent exists
                        const float alphaCP = atan2_approx(py - cy, px - cx);
                        const float phiT = acos_approx(constrainf(arcRtmp / dCP, 0.0f, 1.0f));
                        for (int8_t s = -1; s <= 1; s += 2) {   // of the two tangent points, exit where the tangent points at the WP
                            const float th = alphaCP + (float)s * phiT;
                            const float tx = cx + arcRtmp * cos_approx(th);
                            const float ty = cy + arcRtmp * sin_approx(th);
                            if ((px - tx) * (-dirTmp * sin_approx(th)) + (py - ty) * (dirTmp * cos_approx(th)) > 0.0f) {
                                fwArcEngaged = true;
                                phase = ARC_RAMP_IN;
                                rampMs = 0.0f;
                                rampStartCd = fwLastNavRollCmdCd;
                                arcR = arcRtmp;
                                arcDir = dirTmp;
                                arcCx = cx;
                                arcCy = cy;
                                arcOutBearing = wrap_36000(lrintf(DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(atan2_approx(py - ty, px - tx)))));
                                phiNomCd = phiTmp;
                                tEaseMs = tTmp;
                            }
                        }
                    }
                }
            }
            const int32_t hdgErr = wrap_18000(legBearing - cog);
            if (turnMode != NAV_FW_WP_TURN_COORD_FLY_OVER && ABS(hdgErr) > NAV_FW_ARC_MIN_TURN_ANGLE_CD) {
                const float arcRtmp = getFwCoordinatedTurnRadius();
                const float phiTmp = DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(atan2_approx(v * v, GRAVITY_CMSS * arcRtmp)));
                const float tTmp = fwTurnEaseTimeMs(CENTIDEGREES_TO_DEGREES(phiTmp));
                const float omegaNomCds = DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(v / arcRtmp));   // v/R == g*tan(phi)/v
                const float psiTmp = 0.5f * omegaNomCds * (tTmp / 1000.0f);
                if (capped || ABS(hdgErr) > NAV_FW_ARC_SHARP_TURN_CD) {
                    // Capped or near-reversal: no valid tangent circle - fly the bounded capture directly
                    fwArcEngaged = true;
                    phase = ARC_CAPTURE;
                    fwArcBankCmd = fwLastNavRollCmdCd;          // blend from the current command: no engage step
                    rampMs = 0.0f;
                    rampStartCd = 0.0f;
                    arcR = arcRtmp;                             // capture ignores it, but the away timeout is sized from it
                    arcDir = (hdgErr > 0) ? 1 : -1;
                    arcOutBearing = legBearing;
                    arcToLegLine = true;
                    phiNomCd = phiTmp;
                    tEaseMs = tTmp;
                } else if (2.0f * psiTmp < (float)ABS(hdgErr)) {   // enough turn left for a steady arc between the ease ramps
                    fwArcEngaged = true;
                    phase = ARC_RAMP_IN;
                    rampMs = 0.0f;
                    rampStartCd = fwLastNavRollCmdCd;
                    arcR = arcRtmp;
                    arcDir = (hdgErr > 0) ? 1 : -1;
                    arcOutBearing = legBearing;
                    phiNomCd = phiTmp;
                    tEaseMs = tTmp;
                    // Centre = intersection of both legs shifted R inside (inscribed circle), so the exit lands ON the out-leg
                    const float cogRad = CENTIDEGREES_TO_RADIANS((float)cog);
                    const float legRad = CENTIDEGREES_TO_RADIANS((float)legBearing);
                    const float d1x = cos_approx(cogRad), d1y = sin_approx(cogRad);
                    const float d2x = cos_approx(legRad), d2y = sin_approx(legRad);
                    const float cross = d1x * d2y - d1y * d2x;
                    const float p1x = pos->x + arcRtmp * cos_approx(cogRad + arcDir * (M_PIf * 0.5f));
                    const float p1y = pos->y + arcRtmp * sin_approx(cogRad + arcDir * (M_PIf * 0.5f));
                    if (fabsf(cross) > 0.087f) {                // legs not near-parallel (30..160 deg turn)
                        const float p2x = posControl.activeWaypoint.pos.x + arcRtmp * cos_approx(legRad + arcDir * (M_PIf * 0.5f));
                        const float p2y = posControl.activeWaypoint.pos.y + arcRtmp * sin_approx(legRad + arcDir * (M_PIf * 0.5f));
                        const float tt = ((p2x - p1x) * d2y - (p2y - p1y) * d2x) / cross;
                        arcCx = p1x + tt * d1x;
                        arcCy = p1y + tt * d1y;
                    } else {                                    // degenerate -> tangent at the entry point
                        arcCx = p1x;
                        arcCy = p1y;
                    }
                }
            }
        }
        if (!fwArcEngaged && turnMode != NAV_FW_WP_TURN_COORD_FLY_BY && (navGetCurrentStateFlags() & NAV_AUTO_WP)) {
            if (intoStage == FW_INTO_MAIN) {
                // crossing flown: re-arm once the leg has switched (normally it already has, mid-arc)
                intoStage = (ABS(wrap_18000(legBearing - intoBOut)) < 500) ? FW_INTO_IDLE : FW_INTO_DONE;
            }
            if (intoStage == FW_INTO_IDLE && turnMode == NAV_FW_WP_TURN_COORD_FLY_INTO) {
                const int32_t nta = posControl.activeWaypoint.nextTurnAngle;
                if (nta != -1 && ABS(nta) > NAV_FW_ARC_MIN_TURN_ANGLE_CD) {
                    const float arcRtmp = getFwCoordinatedTurnRadius();
                    const float phiTmp = DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(atan2_approx(v * v, GRAVITY_CMSS * arcRtmp)));
                    const float tTmp = fwTurnEaseTimeMs(CENTIDEGREES_TO_DEGREES(phiTmp));
                    const int8_t dirM = (nta > 0) ? 1 : -1;
                    const int32_t bOut = wrap_36000(legBearing + nta);
                    const float bOutRad = CENTIDEGREES_TO_RADIANS((float)bOut);
                    const float bInRad = CENTIDEGREES_TO_RADIANS((float)legBearing);
                    const float ux = cos_approx(bInRad), uy = sin_approx(bInRad);
                    // Main circle pinned at the WP, counter circle on the inbound leg; centers
                    // sqrt((2R)^2+Ls^2) apart so an Ls gap gives the roll swing room
                    const float Ls = 2.0f * v * (tTmp / 1000.0f);
                    const float o2x = posControl.activeWaypoint.pos.x + arcRtmp * cos_approx(bOutRad + dirM * (M_PIf * 0.5f));
                    const float o2y = posControl.activeWaypoint.pos.y + arcRtmp * sin_approx(bOutRad + dirM * (M_PIf * 0.5f));
                    const float ax = posControl.activeWaypoint.pos.x + arcRtmp * cos_approx(bInRad - dirM * (M_PIf * 0.5f));
                    const float ay = posControl.activeWaypoint.pos.y + arcRtmp * sin_approx(bInRad - dirM * (M_PIf * 0.5f));
                    const float wx = o2x - ax, wy = o2y - ay;
                    const float wu = wx * ux + wy * uy;
                    const float disc = wu * wu - (wx * wx + wy * wy) + 4.0f * arcRtmp * arcRtmp + Ls * Ls;
                    if (disc > 0.0f) {
                        const float s = wu - fast_fsqrtf(disc); // signed along-leg offset of the S start from the WP
                        const float triggerDist = -s + 1.5f * v * (tTmp / 1000.0f);
                        if (s < 0.0f && posControl.wpDistance < triggerDist) {
                            const float o1x = ax + s * ux;
                            const float o1y = ay + s * uy;
                            const float cAng = atan2_approx(o2y - o1y, o2x - o1x);
                            const float beta = atan2_approx(Ls, 2.0f * arcRtmp);
                            const float nAng = cAng + (float)dirM * beta;
                            intoEx = o2x - arcRtmp * cos_approx(nAng);          // main-arc pickup = internal-tangent touch on the main circle
                            intoEy = o2y - arcRtmp * sin_approx(nAng);
                            intoO2x = o2x; intoO2y = o2y;
                            intoR = arcRtmp; intoBOut = bOut; intoDir = dirM;
                            fwArcEngaged = true;
                            phase = ARC_RAMP_IN;
                            rampMs = 0.0f;
                            rampStartCd = fwLastNavRollCmdCd;
                            arcR = arcRtmp;
                            arcDir = -dirM;                     // counter-arc first
                            arcCx = o1x;
                            arcCy = o1y;
                            arcOutBearing = wrap_36000(lrintf(DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(
                                cAng - (float)dirM * (M_PIf * 0.5f - beta)))));  // internal-tangent course: the away arc rolls out onto it
                            phiNomCd = phiTmp;
                            tEaseMs = tTmp;
                            intoStage = FW_INTO_AWAY;
                        }
                    }
                }
            } else if (intoStage == FW_INTO_AWAY) {
                intoStage = FW_INTO_IDLE;               // only reachable via a controller reset mid-S: geometry is stale
            }
        }
        if (!fwArcEngaged) {
            DEBUG_SET(DEBUG_FW_TURN, 1, intoStage);
            return;
        }
    } else {
        // Mission advanced mid-arc: retarget the bounded capture onto the new leg instead of the stale
        // out-bearing - unless the arc already flies toward that leg (the FLY_INTO commit above advances
        // the WP mid-main-arc by design; retargeting would degrade the shaped arc to a bare capture)
        if (ABS(wrap_18000(legBearing - fwArcPrevLegBearing)) > 500
            && ABS(wrap_18000(legBearing - arcOutBearing)) > 500) {
            fwFlyByCappedLatch = false;
            arcOutBearing = legBearing;
            arcToLegLine = true;
            phase = ARC_CAPTURE;
            if (intoStage == FW_INTO_AWAY) {
                intoStage = FW_INTO_DONE;                       // staged S is stale: release the hand-back block
            }
        }
        fwArcPrevLegBearing = legBearing;

        // Pick up the second arc at its tangency point. Along-track distance so a lateral residual
        // cannot miss it; the heading gate blocks the trigger early in the first arc, where the
        // pickup point still lies behind the exit course.
        const float outRadE = CENTIDEGREES_TO_RADIANS((float)arcOutBearing);
        fwArcPickupAlong = (intoEx - pos->x) * cos_approx(outRadE) + (intoEy - pos->y) * sin_approx(outRadE);

        // The away arc blocks the hand-back, so a pickup that never triggers strands the aircraft with
        // path tracking suppressed. Bound it by the time to fly half the away circle.
        if (intoStage == FW_INTO_AWAY) {
            intoAwayMs += US2S(deltaMicros) * 1000.0f;
            if (intoAwayMs > NAV_FW_ARC_AWAY_TIMEOUT_FACTOR * 1000.0f * M_PIf * arcR / MAX(v, NAV_FW_TURN_MIN_SPEED)) {
                intoStage = FW_INTO_DONE;                       // release the block; the capture finishes on the leg
                arcOutBearing = legBearing;
                arcToLegLine = true;
                phase = ARC_CAPTURE;
            }
        } else {
            intoAwayMs = 0.0f;
        }

        if (intoStage == FW_INTO_AWAY
            && ABS(wrap_18000(arcOutBearing - cog)) < 4500
            && fwArcPickupAlong <= 1.5f * v * (tEaseMs / 1000.0f)) {
            phase = ARC_RAMP_IN;
            rampMs = 0.0f;
            rampStartCd = fwArcBankCmd;                         // blend from the current bank
            arcDir = intoDir;
            // Radius from CURRENT groundspeed (the arming value may be unflyable downwind), and the
            // circle re-anchored along the leg line through the actual position: wind drift becomes
            // an along-track shift instead of a parallel roll-out offset
            const float r2 = getFwCoordinatedTurnRadius();
            const float legR2 = CENTIDEGREES_TO_RADIANS((float)intoBOut);
            const float u2x = cos_approx(legR2), u2y = sin_approx(legR2);
            const float b0x = posControl.activeWaypoint.pos.x + r2 * cos_approx(legR2 + (float)intoDir * (M_PIf * 0.5f));
            const float b0y = posControl.activeWaypoint.pos.y + r2 * sin_approx(legR2 + (float)intoDir * (M_PIf * 0.5f));
            const float w2x = pos->x - b0x, w2y = pos->y - b0y;
            const float w2u = w2x * u2x + w2y * u2y;
            const float disc2 = w2u * w2u - (w2x * w2x + w2y * w2y) + r2 * r2;
            if (disc2 > 0.0f) {
                const float t2 = w2u + fast_fsqrtf(disc2);
                arcR = r2;
                arcCx = b0x + t2 * u2x;
                arcCy = b0y + t2 * u2y;
                phiNomCd = DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(atan2_approx(v * v, GRAVITY_CMSS * r2)));
                tEaseMs = fwTurnEaseTimeMs(CENTIDEGREES_TO_DEGREES(phiNomCd));
            } else {                                            // drifted beyond the line: keep the planned circle
                arcR = intoR;
                arcCx = intoO2x;
                arcCy = intoO2y;
            }
            arcOutBearing = intoBOut;
            intoStage = FW_INTO_MAIN;
            if (turnMode == NAV_FW_WP_TURN_COORD_FLY_INTO) {
                /* Committed onto the outbound leg: mark the WP reached (as FLY_BY does at turn start).
                 * The corner cut never crosses the passage plane through the WP, so no geometric check
                 * can fire - the stale carrot would steer back toward the old leg after hand-back. */
                posControl.flags.wpTurnSmoothingActive = true;
            }
        }
    }

    // Leg-line capture (path tracking on): steer onto the track itself, not merely parallel to it -
    // a reversal fallback otherwise ends a turn-diameter off the leg. Intercept angle tapers with
    // the cross-track offset (1 cd/cm), capped at the tracker's own convergence limit.
    if (arcToLegLine && navConfig()->fw.wp_tracking_accuracy) {
        const float legRadT = CENTIDEGREES_TO_RADIANS((float)legBearing);
        const float offLeg = (pos->x - posControl.activeWaypoint.pos.x) * (-sin_approx(legRadT))
                           + (pos->y - posControl.activeWaypoint.pos.y) * cos_approx(legRadT);
        const float gammaCd = constrainf(fabsf(offLeg), 0.0f, DEGREES_TO_CENTIDEGREES(navConfig()->fw.wp_tracking_max_angle));
        arcOutBearing = wrap_36000(legBearing - lrintf(SIGN(offLeg) * gammaCd));
    }

    rampMs += US2S(deltaMicros) * 1000.0f;
    fwArcEaseMs = tEaseMs;                                      // published for the handback fade
    const int32_t hdgErrOut = wrap_18000(arcOutBearing - cog);

    // Roll-out lead: heading consumed by the shaped down-ramp plus the angle-P tail and servo delay
    const float bankNowRad = CENTIDEGREES_TO_RADIANS((float)ABS(attitude.values.roll) * 10.0f);
    const float omegaCds = DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(GRAVITY_CMSS * tan_approx(bankNowRad) / MAX(v, NAV_FW_TURN_MIN_SPEED)));
    const float levelGain = pidBank()->pid[PID_LEVEL].P * FP_PID_LEVEL_P_MULTIPLIER;    // [1/s]
    const float rollOutS = ((levelGain > 0.1f) ? (1.0f / levelGain) : 1.0f) + (float)navConfig()->fw.wp_turn_control_ease * 0.001f;
    const float psiLeadCd = omegaCds * rollOutS + 0.5f * omegaCds * (tEaseMs / 1000.0f);

    switch (phase) {
    case ARC_RAMP_IN: {
        // Rate-consistent: a swing spanning 2*phi takes twice the standard ease time
        const float rampSpanCd = fabsf((float)arcDir * phiNomCd - rampStartCd);
        const float rampDurMs = MAX(tEaseMs * rampSpanCd / MAX(phiNomCd, 1.0f), 0.5f * tEaseMs);
        const float p = (rampDurMs > 1.0f) ? constrainf(rampMs / rampDurMs, 0.0f, 1.0f) : 1.0f;
        const float s = p * p * (3.0f - 2.0f * p);             // smoothstep up
        fwArcBankCmd = rampStartCd + ((float)arcDir * phiNomCd - rampStartCd) * s;
        if (p >= 1.0f) {                                       // roll-in done -> track the pre-placed tangent circle
            phase = ARC_STEADY;
            steadyMs = 0.0f;                                   // ramp-in does not steer onto the circle, so the arc
            steadyStartCd = fwArcBankCmd;                      // law starts displaced: blend into it, don't step
        }
        break;
    }
    case ARC_STEADY: {
        const float dx = pos->x - arcCx;
        const float dy = pos->y - arcCy;
        const float eR = calc_length_pythagorean_2D(dx, dy) - arcR;             // [cm], + = outside the arc
        const float alpha = atan2_approx(dy, dx);                               // azimuth on the arc
        const int32_t tangentBearing = lrintf(DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(atan2_approx(arcDir * cos_approx(alpha), -arcDir * sin_approx(alpha)))));
        const int32_t eH = wrap_18000(tangentBearing - cog);                    // [centideg] heading error to the arc tangent
        // FF from current groundspeed: wind changes v along the arc
        const float phiLiveCd = DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(atan2_approx(v * v, GRAVITY_CMSS * arcR)));
        const float steadyCd = arcDir * (phiLiveCd + NAV_FW_ARC_RADIAL_GAIN * eR) + NAV_FW_ARC_HEADING_GAIN * (float)eH;
        steadyMs += US2S(deltaMicros) * 1000.0f;
        const float q = (tEaseMs > 1.0f) ? constrainf(steadyMs / tEaseMs, 0.0f, 1.0f) : 1.0f;
        const float s = q * q * (3.0f - 2.0f * q);
        // Two-sided slew limit: on a tight arc the tangent bearing is ill-conditioned near the centre
        // and eH can invert between two updates, which would otherwise go straight to the servos
        const float blended = steadyStartCd + (steadyCd - steadyStartCd) * s;
        const float maxStepCd = phiNomCd * (US2S(deltaMicros) * 1000.0f) / MAX(tEaseMs, 1.0f);
        fwArcBankCmd += constrainf(blended - fwArcBankCmd, -maxStepCd, maxStepCd);
        if (NAV_FW_ARC_EXIT_GAIN * (float)ABS(hdgErrOut) <= ABS(fwArcBankCmd)
            || (float)ABS(hdgErrOut) <= psiLeadCd) {           // remaining heading fits the shaped roll-out -> start it
            phase = ARC_CAPTURE;
        }
        break;
    }
    case ARC_CAPTURE:
    default: {
        // No-overshoot envelope; slewed at the ramp rate in BOTH directions - the rise used to be
        // unlimited, which stepped the servos at fresh capped engages and mid-capture mission advances
        int32_t captureErr = hdgErrOut;
        if (ABS(captureErr) > 17000) {
            captureErr = arcDir * ABS(captureErr);              // ambiguous reversal: hold the engagement direction
        }
        const float errLeadCd = MAX((float)ABS(captureErr) - psiLeadCd, 0.0f);
        const float cmd = constrainf(NAV_FW_ARC_EXIT_GAIN * ((captureErr > 0) ? errLeadCd : -errLeadCd), -phiNomCd, phiNomCd);
        const float maxStepCd = phiNomCd * (US2S(deltaMicros) * 1000.0f) / MAX(tEaseMs, 1.0f);
        fwArcBankCmd += constrainf(cmd - fwArcBankCmd, -maxStepCd, maxStepCd);
        // Mid-S the gap between the arcs stays engaged: handing back there would give the PID and
        // path tracking a moment of control while we sit a full turn diameter off the leg.
        if (ABS(hdgErrOut) <= NAV_FW_ARC_EXIT_HANDOFF_CD && fabsf(fwArcBankCmd) <= phiNomCd * 0.1f
            && intoStage != FW_INTO_AWAY) {                     // aligned and nearly level -> hand back
            fwArcEngaged = false;
            DEBUG_SET(DEBUG_FW_TURN, 1, 0);                     // released: don't leave stale values in the log
            DEBUG_SET(DEBUG_FW_TURN, 3, 0);
            return;
        }
        break;
    }
    }

    // Clamp to the flyable ceiling: rate limits, handoff checks and the smoother seed must not
    // run on a command the airframe cannot reach (wind can drive eR arbitrarily large)
    const float cmdLimitCd = DEGREES_TO_CENTIDEGREES(getFwEffectiveBankLimit());
    fwArcBankCmd = constrainf(fwArcBankCmd, -cmdLimitCd, cmdLimitCd);

    DEBUG_SET(DEBUG_FW_TURN, 0, lrintf(arcR));                  // active arc radius [cm]
    DEBUG_SET(DEBUG_FW_TURN, 1, (phase + 1) * 10 + intoStage);  // coordinator state: (arc phase + 1)*10 + S stage
    DEBUG_SET(DEBUG_FW_TURN, 2, arcOutBearing);                 // exit course [centideg]
    DEBUG_SET(DEBUG_FW_TURN, 3, hdgErrOut);                     // remaining heading to the exit course [centideg]
    DEBUG_SET(DEBUG_FW_TURN, 4, lrintf(fwArcBankCmd));          // arc bank command [centideg]
    DEBUG_SET(DEBUG_FW_TURN, 7, lrintf(tEaseMs));               // roll ease time [ms] -> sizes the turn leads
    if (intoStage == FW_INTO_AWAY) {
        DEBUG_SET(DEBUG_FW_TURN, 6, lrintf(fwArcPickupAlong));  // away arc: along-track distance to the pickup [cm]
    }
    fwArcActive = true;
}

// Loiter circle controller: once established on the hold circle, the steady arc law replaces the
// carrot PID - live FF bank plus radial/tangent feedback hold the stabilised radius exactly
static void updateFwLoiterArc(timeDelta_t deltaMicros)
{
    static bool established = false;

    if (!needToCalculateCircularLoiter || fwArcEngaged) {
        established = false;
        return;
    }

    const fpVector3_t *pos = &navGetCurrentActualPositionAndVelocity()->pos;
    const float dcx = pos->x - posControl.desiredState.pos.x;
    const float dcy = pos->y - posControl.desiredState.pos.y;
    const float dist = calc_length_pythagorean_2D(dcx, dcy);
    const float arcRadius = MAX(fwActiveLoiterRadius, (float)NAV_FW_TURN_RADIUS_MIN);
    const int8_t dir = loiterDirection();
    const float alpha = atan2_approx(dcy, dcx);
    const int32_t tangentBearing = lrintf(DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(atan2_approx(dir * cos_approx(alpha), -dir * sin_approx(alpha)))));
    const int32_t eH = wrap_18000(tangentBearing - posControl.actualState.cog);
    const float eR = dist - arcRadius;

    if (!established) {
        if (fabsf(eR) < NAV_FW_LOITER_CAPTURE_BAND * arcRadius && ABS(eH) < NAV_FW_LOITER_CAPTURE_ALIGN_CD) {
            established = true;
            fwArcBankCmd = fwLastNavRollCmdCd;      // blend from the current command: no engage step
        } else {
            DEBUG_SET(DEBUG_FW_TURN, 1, 4);         // loiter approach, carrot guidance
            return;
        }
    } else if (fabsf(eR) > NAV_FW_LOITER_RELEASE_BAND * arcRadius) {
        established = false;                        // grossly displaced: hand back to the carrot
        return;
    }

    const float v = posControl.actualState.velXY;
    const float phiLiveCd = DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(atan2_approx(v * v, GRAVITY_CMSS * arcRadius)));
    const float targetCd = dir * (phiLiveCd + NAV_FW_ARC_RADIAL_GAIN * eR) + NAV_FW_ARC_HEADING_GAIN * (float)eH;
    const float tEase = fwTurnEaseTimeMs(CENTIDEGREES_TO_DEGREES(phiLiveCd));
    const float maxStepCd = phiLiveCd * (US2S(deltaMicros) * 1000.0f) / MAX(tEase, 1.0f);
    fwArcBankCmd += constrainf(targetCd - fwArcBankCmd, -maxStepCd, maxStepCd);
    const float cmdLimitCd = DEGREES_TO_CENTIDEGREES(getFwEffectiveBankLimit());
    fwArcBankCmd = constrainf(fwArcBankCmd, -cmdLimitCd, cmdLimitCd);

    DEBUG_SET(DEBUG_FW_TURN, 1, 40);                // loiter circle controller engaged
    DEBUG_SET(DEBUG_FW_TURN, 4, lrintf(fwArcBankCmd));
    fwArcActive = true;
}

static void calculateVirtualPositionTarget_FW(float trackingPeriod, timeDelta_t deltaMicros)
{
    if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE) || posControl.navState == NAV_STATE_FW_LANDING_GLIDE || posControl.navState == NAV_STATE_FW_LANDING_FLARE) {
        return;
    }

    float posErrorX = posControl.desiredState.pos.x - navGetCurrentActualPositionAndVelocity()->pos.x;
    float posErrorY = posControl.desiredState.pos.y - navGetCurrentActualPositionAndVelocity()->pos.y;

    float distanceToActualTarget = calc_length_pythagorean_2D(posErrorX, posErrorY);

    // Limit minimum forward velocity to 1 m/s
    float trackingDistance = trackingPeriod * MAX(posControl.actualState.velXY, 100.0f);

    uint32_t navLoiterRadius = getLoiterRadius(navigationGetLoiterRadius());

    /* Loiter-radius floor with per-revolution peak hold (see getFwStableLoiterRadius): keep the circle
     * stable for the fixed-radius loiter tracker instead of chasing the wind-varying instantaneous value. */
    const bool inLoiter = (navGetCurrentStateFlags() & NAV_CTL_HOLD);
    const float bearingFromCenter = atan2_approx(-posErrorY, -posErrorX);
    navLoiterRadius = getFwStableLoiterRadius(navLoiterRadius, bearingFromCenter, inLoiter, deltaMicros);
    fwActiveLoiterRadius = (float)navLoiterRadius;   // expose to the turn feed-forward

    fpVector3_t loiterCenterPos = posControl.desiredState.pos;
    int8_t loiterTurnDirection = loiterDirection();

    // Detemine if a circular loiter is required.
    // For waypoints only use circular loiter when angular visibility is > 30 degs, otherwise head straight toward target
    #define TAN_15DEG    0.26795f

    bool loiterApproachActive = isNavHoldPositionActive() &&
                                distanceToActualTarget <= (navLoiterRadius / TAN_15DEG) &&
                                distanceToActualTarget > 50.0f;
    needToCalculateCircularLoiter = loiterApproachActive || (navGetCurrentStateFlags() & NAV_CTL_HOLD);

    //if vtol landing is required, fly straight to homepoint
    if ((posControl.navState == NAV_STATE_RTH_HEAD_HOME) && navigationRTHAllowsLanding() && checkMixerATRequired(MIXERAT_REQUEST_LAND)){
        needToCalculateCircularLoiter = false;
    }

    /* FLY_BY corner cut: start the turn R*tan(angle/2) before the WP so the arc joins the next leg
     * at any speed. FLY_BY legs only - the landing approach forces FLY_BY in every mode. */
    int32_t waypointTurnAngle = posControl.activeWaypoint.nextTurnAngle == -1 ? -1 : ABS(posControl.activeWaypoint.nextTurnAngle);
    posControl.flags.wpTurnSmoothingActive = false;
    const bool flyByLeg = navConfig()->fw.wp_turn_mode == NAV_FW_WP_TURN_COORD_FLY_BY
                          || posControl.navState == NAV_STATE_FW_LANDING_APPROACH;
    if (flyByLeg && waypointTurnAngle > 3000 && waypointTurnAngle < 16000 && isWaypointNavTrackingActive() && !needToCalculateCircularLoiter) {
        const float turnRadius = getFwCoordinatedTurnRadius();
        const float halfAngleTan = constrainf(tan_approx(CENTIDEGREES_TO_RADIANS(waypointTurnAngle / 2.0f)), 0.0f, NAV_FW_TURN_LEAD_TAN_MAX);
        // Roll-in lead: the ramp is back-loaded and the ground track lags the bank (k calibrated in flight)
        const float easeLeadDistance = posControl.actualState.velXY * (fwTurnEaseTimeMs(getFwPlanningBankDeg()) / 1000.0f) * 1.5f;
        float turnStartDistance = easeLeadDistance + turnRadius * halfAngleTan;
        // Cap how early the turn may begin (nav_fw_wp_turn_max_lead_time): never more than N ms of flight before the WP.
        const float maxLeadDistance = posControl.actualState.velXY * (float)navConfig()->fw.wp_turn_max_lead_time * 0.001f;
        const bool turnCapped = turnStartDistance > maxLeadDistance;
        turnStartDistance = MIN(turnStartDistance, maxLeadDistance);
        DEBUG_SET(DEBUG_FW_TURN, 0, lrintf(turnRadius));        // FLY_BY planning radius while approaching
        if (posControl.wpDistance < turnStartDistance) {
            posControl.flags.wpTurnSmoothingActive = true;
            fwFlyByCappedLatch = turnCapped;   // capped corner cut -> the arc coordinator flies it direct instead
        }
    }

    // We are closing in on a waypoint, calculate circular loiter if required
    if (needToCalculateCircularLoiter) {
        float loiterAngle = atan2_approx(-posErrorY, -posErrorX) + DEGREES_TO_RADIANS(loiterTurnDirection * 45.0f);
        float loiterTargetX = loiterCenterPos.x + navLoiterRadius * cos_approx(loiterAngle);
        float loiterTargetY = loiterCenterPos.y + navLoiterRadius * sin_approx(loiterAngle);

        // We have temporary loiter target. Recalculate distance and position error
        posErrorX = loiterTargetX - navGetCurrentActualPositionAndVelocity()->pos.x;
        posErrorY = loiterTargetY - navGetCurrentActualPositionAndVelocity()->pos.y;
        distanceToActualTarget = calc_length_pythagorean_2D(posErrorX, posErrorY);
    }

    // Arc turn coordinator: manages the turn state and commands the roll bank directly
    updateFwTurnArc(deltaMicros);
    updateFwLoiterArc(deltaMicros);

    // Calculate virtual waypoint
    virtualDesiredPosition.x = navGetCurrentActualPositionAndVelocity()->pos.x + posErrorX * (trackingDistance / distanceToActualTarget);
    virtualDesiredPosition.y = navGetCurrentActualPositionAndVelocity()->pos.y + posErrorY * (trackingDistance / distanceToActualTarget);

    // Shift position according to pilot's ROLL input (up to max_manual_speed velocity)
    if (posControl.flags.isAdjustingPosition) {
        int16_t rcRollAdjustment = applyDeadbandRescaled(rcCommand[ROLL], rcControlsConfig()->pos_hold_deadband, -500, 500);

        if (rcRollAdjustment) {
            float rcShiftY = rcRollAdjustment * navConfig()->general.max_manual_speed / 500.0f * trackingPeriod;

            // Rotate this target shift from body frame to to earth frame and apply to position target
            virtualDesiredPosition.x += -rcShiftY * posControl.actualState.sinYaw;
            virtualDesiredPosition.y +=  rcShiftY * posControl.actualState.cosYaw;
        }
    }
}

bool adjustFixedWingPositionFromRCInput(void)
{
    int16_t rcRollAdjustment = applyDeadbandRescaled(rcCommand[ROLL], rcControlsConfig()->pos_hold_deadband, -500, 500);
    return (rcRollAdjustment);
}

float processHeadingYawController(timeDelta_t deltaMicros, int32_t navHeadingError, bool errorIsDecreasing) {
    static float limit = 0.0f;

    if (limit == 0.0f) {
        limit = pidProfile()->navFwPosHdgPidsumLimit * 100.0f;
    }

    const pidControllerFlags_e yawPidFlags = errorIsDecreasing ? PID_SHRINK_INTEGRATOR : 0;

    const float yawAdjustment = navPidApply2(
        &posControl.pids.fw_heading,
        0,
        applyDeadband(navHeadingError, navConfig()->fw.yawControlDeadband * 100),
        US2S(deltaMicros),
        -limit,
        limit,
        yawPidFlags
        ) * 0.01f;

    DEBUG_SET(DEBUG_NAV_YAW, 0, posControl.pids.fw_heading.proportional);
    DEBUG_SET(DEBUG_NAV_YAW, 1, posControl.pids.fw_heading.integral);
    DEBUG_SET(DEBUG_NAV_YAW, 2, posControl.pids.fw_heading.derivative);
    DEBUG_SET(DEBUG_NAV_YAW, 3, navHeadingError);
    DEBUG_SET(DEBUG_NAV_YAW, 4, posControl.pids.fw_heading.output_constrained);

    return yawAdjustment;
}

static void updatePositionHeadingController_FW(timeUs_t currentTimeUs, timeDelta_t deltaMicros)
{
    static timeUs_t previousTimeMonitoringUpdate;
    static int32_t previousHeadingError;
    static bool errorIsDecreasing;
    static bool forceTurnDirection = false;
    int32_t virtualTargetBearing;

    if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE) || posControl.navState == NAV_STATE_FW_LANDING_GLIDE || posControl.navState == NAV_STATE_FW_LANDING_FLARE) {
        virtualTargetBearing = posControl.desiredState.yaw;
    } else {
        // We have virtual position target, calculate heading error
        virtualTargetBearing = calculateBearingToDestination(&virtualDesiredPosition);
    }

    if (isWaypointNavTrackingActive()) {
        /* Cross-track controller state. Scoped here, not in the control branch, so the
         * else branch can re-seed it while the controller is disengaged.
         * fwCrossTrackErrorRateFilterState itself is file-scope (see above); only the
         * plain statics below need this wider scope. */
        static float crossTrackErrorRate;
        static timeUs_t previousCrossTrackErrorUpdateTime;
        static float previousCrossTrackError = 0.0f;

        /* Calculate cross track error */
        posControl.wpDistance = calculateDistanceToDestination(&posControl.activeWaypoint.pos);

        fpVector3_t virtualCoursePoint;
        virtualCoursePoint.x = posControl.activeWaypoint.pos.x -
                               posControl.wpDistance * cos_approx(CENTIDEGREES_TO_RADIANS(posControl.activeWaypoint.bearing));
        virtualCoursePoint.y = posControl.activeWaypoint.pos.y -
                               posControl.wpDistance * sin_approx(CENTIDEGREES_TO_RADIANS(posControl.activeWaypoint.bearing));
        navCrossTrackError = calculateDistanceToDestination(&virtualCoursePoint);

        /* If waypoint tracking enabled force craft toward and closely track along waypoint course line.
         * Suppressed while the arc coordinator drives a turn (it tracks the arc, not the straight leg). */
        if (navConfig()->fw.wp_tracking_accuracy && !needToCalculateCircularLoiter && !fwArcActive) {
            if ((currentTimeUs - previousCrossTrackErrorUpdateTime) >= HZ2US(20) && fabsf(previousCrossTrackError - navCrossTrackError) > 10.0f) {
                const float crossTrackErrorDtSec =  US2S(currentTimeUs - previousCrossTrackErrorUpdateTime);
                if (fabsf(previousCrossTrackError - navCrossTrackError) < 500.0f) {
                    crossTrackErrorRate = (previousCrossTrackError - navCrossTrackError) / crossTrackErrorDtSec;
                }
                // Cutoff set here, not only at controller reset: an unset RC leaves alpha at 1 (no filtering)
                pt1FilterSetCutoff(&fwCrossTrackErrorRateFilterState, NAV_FW_CROSSTRACK_RATE_CUTOFF_HZ);
                crossTrackErrorRate = pt1FilterApply3(&fwCrossTrackErrorRateFilterState, crossTrackErrorRate, crossTrackErrorDtSec);
                previousCrossTrackErrorUpdateTime = currentTimeUs;
                previousCrossTrackError = navCrossTrackError;
            }

            uint16_t trackingDeadband = METERS_TO_CENTIMETERS(navConfig()->fw.wp_tracking_accuracy);

            if ((ABS(wrap_18000(virtualTargetBearing - posControl.actualState.cog)) < 9000 || posControl.wpDistance < 1000.0f) && navCrossTrackError > trackingDeadband) {
                float adjustmentFactor = wrap_18000(posControl.activeWaypoint.bearing - virtualTargetBearing);
                uint16_t angleLimit = DEGREES_TO_CENTIDEGREES(navConfig()->fw.wp_tracking_max_angle);

                /* Apply heading adjustment to match crossTrackErrorRate with fixed convergence speed profile */
                float maxApproachSpeed = posControl.actualState.velXY * sin_approx(CENTIDEGREES_TO_RADIANS(angleLimit));
                float desiredApproachSpeed = constrainf(navCrossTrackError / 3.0f, 50.0f, maxApproachSpeed);
                adjustmentFactor = SIGN(adjustmentFactor) * navCrossTrackError * ((desiredApproachSpeed - crossTrackErrorRate) / desiredApproachSpeed);

                /* Calculate final adjusted virtualTargetBearing */
                uint16_t limit = constrainf(navCrossTrackError, 1000.0f, angleLimit);
                adjustmentFactor = constrainf(adjustmentFactor, -limit, limit);
                virtualTargetBearing = wrap_36000(posControl.activeWaypoint.bearing - adjustmentFactor);
            }
        } else {
            /* Keep state synced to the current error while not steering, and seed the convergence
             * estimate from the geometric closing speed: re-engaging with a zero rate reads as
             * "not converging" and commands the full correction in one step (arc hand-back kick). */
            previousCrossTrackError = navCrossTrackError;
            previousCrossTrackErrorUpdateTime = currentTimeUs;
            const fpVector3_t *trackPos = &navGetCurrentActualPositionAndVelocity()->pos;
            const float legRad = CENTIDEGREES_TO_RADIANS((float)posControl.activeWaypoint.bearing);
            const float offLeg = (trackPos->x - virtualCoursePoint.x) * (-sin_approx(legRad))
                               + (trackPos->y - virtualCoursePoint.y) * cos_approx(legRad);
            const float cogOffRad = CENTIDEGREES_TO_RADIANS((float)wrap_18000(posControl.actualState.cog - posControl.activeWaypoint.bearing));
            crossTrackErrorRate = -SIGN(offLeg) * posControl.actualState.velXY * sin_approx(cogOffRad);
            pt1FilterReset(&fwCrossTrackErrorRateFilterState, crossTrackErrorRate);
        }
    }
    /*
     * Calculate NAV heading error
     * Units are centidegrees
     */
    int32_t navHeadingError = wrap_18000(virtualTargetBearing - posControl.actualState.cog);

    // Forced turn direction
    // If heading error is close to 180 deg we initiate forced turn and only disable it when heading error goes below 90 deg
    if (ABS(navHeadingError) > 17000) {
        forceTurnDirection = true;
    }
    else if (ABS(navHeadingError) < 9000 && forceTurnDirection) {
        forceTurnDirection = false;
    }

    // If forced turn direction flag is enabled we fix the sign of the direction
    if (forceTurnDirection) {
        navHeadingError = loiterDirection() * ABS(navHeadingError);
    }

    // Slow error monitoring (2Hz rate)
    if ((currentTimeUs - previousTimeMonitoringUpdate) >= HZ2US(NAV_FW_CONTROL_MONITORING_RATE)) {
        // Check if error is decreasing over time
        errorIsDecreasing = (ABS(previousHeadingError) > ABS(navHeadingError));

        // Save values for next iteration
        previousHeadingError = navHeadingError;
        previousTimeMonitoringUpdate = currentTimeUs;
    }

    // Only allow PID integrator to shrink if error is decreasing over time.
    // Freeze the integrator while the arc drives the turn - the carrot error keeps one sign and winds it up
    const pidControllerFlags_e pidFlags = PID_DTERM_FROM_ERROR
                                        | (errorIsDecreasing ? PID_SHRINK_INTEGRATOR : 0)
                                        | (fwArcActive ? PID_FREEZE_INTEGRATOR : 0);

    // Input error in (deg*100), output roll angle (deg*100)
    const float navBankLimit = getFwControlBankLimit();      // planning target on WP turns, guard ceiling in loiter
    float rollAdjustment = navPidApply2(&posControl.pids.fw_nav, posControl.actualState.cog + navHeadingError, posControl.actualState.cog, US2S(deltaMicros),
                                       -DEGREES_TO_CENTIDEGREES(navBankLimit),
                                        DEGREES_TO_CENTIDEGREES(navBankLimit),
                                        pidFlags);

    // Arc bank overrides the PID; its ramps are already shaped, so the S-curve smoother is bypassed
    // and re-seeded at handback to avoid a stale-state step
    if (fwArcActive) {
        rollAdjustment = fwArcBankCmd;
        fwRollSmoothSeedCd = fwArcBankCmd;      // else the handback re-seeds from a stale reset value (brief roll twitch)
        fwRollSmoothReseed = true;
        fwArcHandbackDurMs = 0.0f;              // an arc that re-engages cancels a fade still in progress
        fwArcWasActive = true;
        // The FF is not called while the arc drives: clear and disarm it, and consume leg changes the
        // arc handles itself - the carrot error left at hand-back is a capture correction, not a turn
        fwTurnFFCmdCd = 0.0f;
        fwTurnFFArmed = false;
        fwTurnFFPrevLegBearing = posControl.activeWaypoint.bearing;
    } else {
        if (fwArcWasActive) {                   // falling edge: arm the crossfade from the arc's last command
            fwArcHandbackCmdCd = fwLastNavRollCmdCd;
            fwArcHandbackMs = 0.0f;
            fwArcHandbackDurMs = fwArcEaseMs;
            fwArcWasActive = false;
        }
        // Coordinated-turn feed-forward: command the bank for the active turn radius so the PID only trims.
        rollAdjustment += getFwTurnFeedForward(navHeadingError, deltaMicros);
        rollAdjustment = applyFwArcHandbackFade(rollAdjustment, deltaMicros);
        rollAdjustment = applyFwRollInSmoothing(rollAdjustment, deltaMicros, fwRollSmoothReseed);
        fwRollSmoothReseed = false;
    }
    rollAdjustment = constrainf(rollAdjustment, -DEGREES_TO_CENTIDEGREES(navBankLimit), DEGREES_TO_CENTIDEGREES(navBankLimit));

    // Convert rollAdjustment to decidegrees (rcAdjustment holds decidegrees)
    posControl.rcAdjustment[ROLL] = CENTIDEGREES_TO_DECIDEGREES(rollAdjustment);
    fwLastNavRollCmdCd = rollAdjustment;
    fwLastNavRollCmdTimeUs = currentTimeUs;

    /*
     * Yaw adjustment
     * It is working in relative mode and we aim to keep error at zero
     */
    if (STATE(FW_HEADING_USE_YAW)) {
        posControl.rcAdjustment[YAW] = processHeadingYawController(deltaMicros, navHeadingError, errorIsDecreasing);
    } else {
        posControl.rcAdjustment[YAW] = 0;
    }
}

void applyFixedWingPositionController(timeUs_t currentTimeUs)
{
    static timeUs_t previousTimePositionUpdate = 0;         // Occurs @ GPS update rate

    // Apply controller only if position source is valid. In absence of valid pos sensor (GPS loss), we'd stick in forced ANGLE mode
    if ((posControl.flags.estPosStatus >= EST_USABLE)) {
        // If we have new position - update velocity and acceleration controllers
        if (posControl.flags.horizontalPositionDataNew) {
            const timeDeltaLarge_t deltaMicrosPositionUpdate = currentTimeUs - previousTimePositionUpdate;
            previousTimePositionUpdate = currentTimeUs;

            if (deltaMicrosPositionUpdate < MAX_POSITION_UPDATE_INTERVAL_US) {
                // Calculate virtual position target at a distance of forwardVelocity * HZ2S(POSITION_TARGET_UPDATE_RATE_HZ)
                // Account for pilot's roll input (move position target left/right at max of max_manual_speed)
                // POSITION_TARGET_UPDATE_RATE_HZ should be chosen keeping in mind that position target shouldn't be reached until next pos update occurs
                // FIXME: verify the above
                calculateVirtualPositionTarget_FW(HZ2S(MIN_POSITION_UPDATE_RATE_HZ) * 2, deltaMicrosPositionUpdate);
                updatePositionHeadingController_FW(currentTimeUs, deltaMicrosPositionUpdate);
                needToCalculateCircularLoiter = false;
            }
            else {
                // Position update has not occurred in time (first iteration or glitch), reset altitude controller
                resetFixedWingPositionController();
            }

            // Indicate that information is no longer usable
            posControl.flags.horizontalPositionDataConsumed = true;
        }

        isRollAdjustmentValid = true;
        isYawAdjustmentValid = true;
    }
    else {
        // No valid pos sensor data, don't adjust pitch automatically, rcCommand[ROLL] is passed through to PID controller
        isRollAdjustmentValid = false;
        isYawAdjustmentValid = false;
    }
}

int16_t applyFixedWingMinSpeedController(timeUs_t currentTimeUs)
{
    static timeUs_t previousTimePositionUpdate = 0;         // Occurs @ GPS update rate

    // If we have new position - update min speed controller
    if (posControl.flags.horizontalPositionDataNew) {
        const timeDeltaLarge_t deltaMicrosPositionUpdate = currentTimeUs - previousTimePositionUpdate;
        previousTimePositionUpdate = currentTimeUs;

        if (deltaMicrosPositionUpdate < MAX_POSITION_UPDATE_INTERVAL_US) {
            float velThrottleBoost = ((getMinGroundSpeed(navConfig()->general.min_ground_speed) * 100.0f) - posControl.actualState.velXY) *
                                     NAV_FW_THROTTLE_SPEED_BOOST_GAIN * US2S(deltaMicrosPositionUpdate);

            throttleSpeedAdjustment += velThrottleBoost;
            throttleSpeedAdjustment = constrainf(throttleSpeedAdjustment, 0.0f, 500.0f);
        }
        else {
            // Reset if position update has not occurred in time (first iteration or glitch)
            throttleSpeedAdjustment = 0;
        }
        // Indicate that information is no longer usable
        posControl.flags.horizontalPositionDataConsumed = true;
    }

    return throttleSpeedAdjustment;
}

int16_t fixedWingPitchToThrottleCorrection(int16_t pitch, timeUs_t currentTimeUs)
{
    static timeUs_t previousTimePitchToThrCorr = 0;
    const timeDeltaLarge_t deltaMicrosPitchToThrCorr = currentTimeUs -  previousTimePitchToThrCorr;
    previousTimePitchToThrCorr = currentTimeUs;


    // Apply low-pass filter to pitch angle to smooth throttle correction
    int16_t filteredPitch = pt1FilterApply3(&pitchToThrFilterState, pitch, US2S(deltaMicrosPitchToThrCorr));

    int16_t pitchToThrottle = currentBatteryProfile->nav.fw.pitch_to_throttle;

#ifdef USE_FW_AUTOLAND
    if (pitch < 0 && posControl.fwLandState.landState == FW_AUTOLAND_STATE_FINAL_APPROACH) {
        pitchToThrottle *= navFwAutolandConfig()->finalApproachPitchToThrottleMod / 100.0f;
    }
#endif

    if (ABS(pitch - filteredPitch) > navConfig()->fw.pitch_to_throttle_thresh) {
        // Unfiltered throttle correction outside of pitch deadband
        return DECIDEGREES_TO_DEGREES(pitch) * pitchToThrottle;
    }
    else {
        // Filtered throttle correction inside of pitch deadband
        return DECIDEGREES_TO_DEGREES(filteredPitch) * pitchToThrottle;
    }
}

void applyFixedWingPitchRollThrottleController(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs)
{
    const uint16_t cruiseThrottle = currentBatteryProfile->nav.fw.cruise_throttle;
    const uint16_t minThrottle = currentBatteryProfile->nav.fw.min_throttle;
    const uint16_t maxThrottle = currentBatteryProfile->nav.fw.max_throttle;

    int16_t minThrottleCorrection = minThrottle - cruiseThrottle;
    int16_t maxThrottleCorrection = maxThrottle - cruiseThrottle;

    if (isRollAdjustmentValid && (navStateFlags & NAV_CTL_POS)) {
        // ROLL >0 right, <0 left
        const int16_t navBankLimitDeciDeg = (int16_t)lrintf(DEGREES_TO_DECIDEGREES(getFwControlBankLimit()));
        int16_t rollCorrection = constrain(posControl.rcAdjustment[ROLL], -navBankLimitDeciDeg, navBankLimitDeciDeg);
        rcCommand[ROLL] = pidAngleToRcCommand(rollCorrection, pidProfile()->max_angle_inclination[FD_ROLL]);
    }

    if (isYawAdjustmentValid && (navStateFlags & NAV_CTL_POS)) {
        rcCommand[YAW] = posControl.rcAdjustment[YAW];
    }

    if (isPitchAdjustmentValid && (navStateFlags & NAV_CTL_ALT)) {
        // PITCH >0 dive, <0 climb
        int16_t pitchCorrection = constrain(posControl.rcAdjustment[PITCH], -DEGREES_TO_DECIDEGREES(navConfig()->fw.max_dive_angle), DEGREES_TO_DECIDEGREES(navConfig()->fw.max_climb_angle));
        rcCommand[PITCH] = -pidAngleToRcCommand(pitchCorrection, pidProfile()->max_angle_inclination[FD_PITCH]);

        if (isFixedwingAutoSpeedActive()) {
            isAutoThrottleManuallyIncreased = false;
        } else {
            int16_t throttleCorrection = fixedWingPitchToThrottleCorrection(pitchCorrection, currentTimeUs);

            if (navStateFlags & NAV_CTL_LAND) {
            // During LAND we do not allow to raise THROTTLE when nose is up to reduce speed
                throttleCorrection = constrain(throttleCorrection, minThrottleCorrection, 0);
            } else {
                throttleCorrection = constrain(throttleCorrection, minThrottleCorrection, maxThrottleCorrection);
            }

            // Min Speed controller - only apply in POS mode when NOT NAV_CTL_LAND
            if (posControl.flags.estPosStatus >= EST_USABLE && (navStateFlags & NAV_CTL_POS) && !(navStateFlags & NAV_CTL_LAND)) {
                bool speedBoostRequired = (getMinGroundSpeed(navConfig()->general.min_ground_speed) * 100.0f - posControl.actualState.velXY > 0) && !throttleSpeedAdjustment;
                if (speedBoostRequired || throttleSpeedAdjustment) {
                    throttleCorrection += applyFixedWingMinSpeedController(currentTimeUs);
                    throttleCorrection = constrain(throttleCorrection, minThrottleCorrection, maxThrottleCorrection);
                }
            }

            uint16_t correctedThrottleValue = constrain(cruiseThrottle + throttleCorrection, minThrottle, maxThrottle);
            const uint16_t autoThrottleValue = correctedThrottleValue;   // auto demand before manual increase, for the energy guard

            // Manual throttle increase
            if (navConfig()->fw.allow_manual_thr_increase && !FLIGHT_MODE(FAILSAFE_MODE) && !FLIGHT_MODE(NAV_FW_AUTOLAND)) {
                if (rcCommand[THROTTLE] < PWM_RANGE_MIN + (PWM_RANGE_MAX - PWM_RANGE_MIN) * 0.95){
                    correctedThrottleValue += MAX(0, rcCommand[THROTTLE] - cruiseThrottle);
                } else {
                    correctedThrottleValue = getMaxThrottle();
                }
                isAutoThrottleManuallyIncreased = (rcCommand[THROTTLE] > cruiseThrottle);
            } else {
                isAutoThrottleManuallyIncreased = false;
            }

            rcCommand[THROTTLE] = setDesiredThrottle(correctedThrottleValue, false);

            // Update the energy guard now that this cycle's pitch + throttle commands are known.
            updateFwEnergyBankGuard(currentTimeUs, autoThrottleValue);
        }
    }

#ifdef USE_FW_AUTOLAND
    // Advanced autoland
    if (posControl.navState == NAV_STATE_FW_LANDING_GLIDE || posControl.navState == NAV_STATE_FW_LANDING_FLARE || STATE(LANDING_DETECTED)) {
        // Set motor to min. throttle and stop it when MOTOR_STOP feature is enabled
        ENABLE_STATE(NAV_MOTOR_STOP_OR_IDLE);

        if (posControl.navState == NAV_STATE_FW_LANDING_GLIDE) {
            rcCommand[PITCH] = pidAngleToRcCommand(-DEGREES_TO_DECIDEGREES(navFwAutolandConfig()->glidePitch), pidProfile()->max_angle_inclination[FD_PITCH]);
        }

        if (posControl.navState == NAV_STATE_FW_LANDING_FLARE) {
            rcCommand[PITCH] = pidAngleToRcCommand(-DEGREES_TO_DECIDEGREES(navFwAutolandConfig()->flarePitch), pidProfile()->max_angle_inclination[FD_PITCH]);
        }
    }
#endif
    // "Traditional" landing as fallback option
    if (navStateFlags & NAV_CTL_LAND) {
        int32_t finalAltitude = navConfig()->general.land_slowdown_minalt + posControl.rthState.homeTmpWaypoint.z;

        if ((posControl.flags.estAltStatus >= EST_USABLE && navGetCurrentActualPositionAndVelocity()->pos.z <= finalAltitude) ||
           (posControl.flags.estAglStatus == EST_TRUSTED && posControl.actualState.agl.pos.z <= navConfig()->general.land_slowdown_minalt)) {

            // Set motor to min. throttle and stop it when MOTOR_STOP feature is enabled
            ENABLE_STATE(NAV_MOTOR_STOP_OR_IDLE);

            // Stabilize ROLL axis on 0 degrees banking regardless of loiter radius and position
            rcCommand[ROLL] = 0;

            // Stabilize PITCH angle into shallow dive as specified by the nav_fw_land_dive_angle setting (default value is 2 - defined in navigation.c).
            rcCommand[PITCH] = pidAngleToRcCommand(DEGREES_TO_DECIDEGREES(navConfig()->fw.land_dive_angle), pidProfile()->max_angle_inclination[FD_PITCH]);
        }
    }
}

bool isFixedWingAutoThrottleManuallyIncreased(void)
{
    return isAutoThrottleManuallyIncreased;
}

bool isFixedWingFlying(void)
{
    float airspeed = 0.0f;
#ifdef USE_PITOT
    if (sensors(SENSOR_PITOT) && pitotIsHealthy()) {
        airspeed = getAirspeedEstimate();
    }
#endif
    bool throttleCondition = getMotorCount() == 0 || rcCommand[THROTTLE] > currentBatteryProfile->nav.fw.cruise_throttle;
    bool velCondition = posControl.actualState.velXY > 350.0f || airspeed > 350.0f;
    bool altCondition = fabsf(posControl.actualState.abs.pos.z - getTakeoffAltitude()) > 500.0f;
    bool launchCondition = isNavLaunchEnabled() && fixedWingLaunchStatus() == FW_LAUNCH_FLYING;

    return (isGPSHeadingValid() && throttleCondition && velCondition && altCondition) || launchCondition;
}

/*-----------------------------------------------------------
 * FixedWing land detector
 *-----------------------------------------------------------*/
bool isFixedWingLandingDetected(void)
{
    DEBUG_SET(DEBUG_LANDING, 4, 0);
    static bool fixAxisCheck = false;

    // Basic condition to start looking for landing
    bool startCondition = (navGetCurrentStateFlags() & (NAV_CTL_LAND | NAV_CTL_EMERG))
                          || FLIGHT_MODE(FAILSAFE_MODE)
                          || FLIGHT_MODE(NAV_FW_AUTOLAND)
                          || (!navigationIsControllingThrottle() && throttleStickIsLow());

    if (!startCondition || posControl.flags.resetLandingDetector) {
        return fixAxisCheck = posControl.flags.resetLandingDetector = false;
    }
    DEBUG_SET(DEBUG_LANDING, 4, 1);

    static timeMs_t fwLandingTimerStartAt;
    static int16_t fwLandSetRollDatum;
    static int16_t fwLandSetPitchDatum;
    const float sensitivity = navConfig()->general.land_detect_sensitivity / 5.0f;

    const timeMs_t currentTimeMs = millis();

    // Check horizontal and vertical velocities are low (cm/s)
    bool velCondition = fabsf(navGetCurrentActualPositionAndVelocity()->vel.z) < (50.0f * sensitivity) &&
                        ( posControl.actualState.velXY < (100.0f * sensitivity));
    // Check angular rates are low (degs/s)
    bool gyroCondition = averageAbsGyroRates() < (2.0f * sensitivity);
    DEBUG_SET(DEBUG_LANDING, 2, velCondition);
    DEBUG_SET(DEBUG_LANDING, 3, gyroCondition);

    if (velCondition && gyroCondition){
        DEBUG_SET(DEBUG_LANDING, 4, 2);
        DEBUG_SET(DEBUG_LANDING, 5, fixAxisCheck);
        if (!fixAxisCheck) {        // capture roll and pitch angles to be used as datums to check for absolute change
            fwLandSetRollDatum = attitude.values.roll;  //0.1 deg increments
            fwLandSetPitchDatum = attitude.values.pitch;
            fixAxisCheck = true;
            fwLandingTimerStartAt = currentTimeMs;
        } else {
            const uint8_t angleLimit = 5 * sensitivity;
            bool isRollAxisStatic = ABS(fwLandSetRollDatum - attitude.values.roll) < angleLimit;
            bool isPitchAxisStatic = ABS(fwLandSetPitchDatum - attitude.values.pitch) < angleLimit;
            DEBUG_SET(DEBUG_LANDING, 6, isRollAxisStatic);
            DEBUG_SET(DEBUG_LANDING, 7, isPitchAxisStatic);
            if (isRollAxisStatic && isPitchAxisStatic) {
                /* Probably landed, low horizontal and vertical velocities and no axis rotation in Roll and Pitch
                 * Conditions need to be held for fixed safety time + optional extra delay.
                 * Fixed time increased if velocities invalid to provide extra safety margin against false triggers */
                const uint16_t safetyTime = posControl.flags.estAltStatus == EST_NONE || posControl.flags.estVelStatus == EST_NONE ? 5000 : 1000;
                timeMs_t safetyTimeDelay = safetyTime + navConfig()->general.auto_disarm_delay;
                return currentTimeMs - fwLandingTimerStartAt > safetyTimeDelay;
            } else {
                fixAxisCheck = false;
            }
        }
    }
    return false;
}

/*-----------------------------------------------------------
 * FixedWing emergency landing
 *-----------------------------------------------------------*/
void applyFixedWingEmergencyLandingController(timeUs_t currentTimeUs)
{
    rcCommand[THROTTLE] = setDesiredThrottle(currentBatteryProfile->failsafe_throttle, true);

    if (posControl.flags.estAltStatus >= EST_USABLE) {
        // target min descent rate at distance 2 x emerg descent rate above takeoff altitude
        updateClimbRateToAltitudeController(0, 2.0f * navConfig()->general.emerg_descent_rate, ROC_TO_ALT_TARGET);
        applyFixedWingAltitudeAndThrottleController(currentTimeUs);

        int16_t pitchCorrection = constrain(posControl.rcAdjustment[PITCH], -DEGREES_TO_DECIDEGREES(navConfig()->fw.max_dive_angle), DEGREES_TO_DECIDEGREES(navConfig()->fw.max_climb_angle));
        rcCommand[PITCH] = -pidAngleToRcCommand(pitchCorrection, pidProfile()->max_angle_inclination[FD_PITCH]);
    } else {
        rcCommand[PITCH] = pidAngleToRcCommand(failsafeConfig()->failsafe_fw_pitch_angle, pidProfile()->max_angle_inclination[FD_PITCH]);
    }

    if (posControl.flags.estPosStatus >= EST_USABLE) {  // Hold position if possible
        applyFixedWingPositionController(currentTimeUs);
        int16_t rollCorrection = constrain(posControl.rcAdjustment[ROLL],
                                            -DEGREES_TO_DECIDEGREES(navConfig()->fw.max_bank_angle),
                                            DEGREES_TO_DECIDEGREES(navConfig()->fw.max_bank_angle));
        rcCommand[ROLL] = pidAngleToRcCommand(rollCorrection, pidProfile()->max_angle_inclination[FD_ROLL]);
        rcCommand[YAW] = 0;
    } else {
        rcCommand[ROLL] = pidAngleToRcCommand(failsafeConfig()->failsafe_fw_roll_angle, pidProfile()->max_angle_inclination[FD_ROLL]);
        rcCommand[YAW] = -pidRateToRcCommand(failsafeConfig()->failsafe_fw_yaw_rate, currentControlProfile->stabilized.rates[FD_YAW]);
    }
}

/*-----------------------------------------------------------
 * Auto Speed mode control
 *-----------------------------------------------------------*/
bool isFixedwingAutoSpeedActive(void)
{
    return navFixedWingAutoSpeedMayControlThrottle(
        STATE(AIRPLANE),
        autoSpeedIsActive,
        mixerATIsActive());
}

static int8_t isAutoSpeedRequiredByNav(void)
{
    int8_t result = -1;
    if (FLIGHT_MODE(NAV_WP_MODE) && getActiveSpeed() > 0) {
        result = FW_AUTO_SPD_GROUND;
    }

    return result;
}

static bool isAutoSpeedEnabled(void)
{
    bool thrStickEmergStop = navConfig()->fw.auto_speed_channel != (THROTTLE + 1) && throttleStickIsLow();
    bool autoSpeedRequested = IS_RC_MODE_ACTIVE(BOXAUTOSPEED) || isAutoSpeedRequiredByNav() >= 0;

    return ARMING_FLAG(ARMED) && autoSpeedRequested && isProbablyStillFlying() && !thrStickEmergStop &&
           !FLIGHT_MODE(FAILSAFE_MODE) && !FLIGHT_MODE(SOARING_MODE) && !FLIGHT_MODE(MANUAL_MODE) &&
           posControl.flags.estVelStatus == EST_TRUSTED && posControl.flags.estAltStatus == EST_TRUSTED &&
           !(navigationRequiresAutoThrottleMode() && !(navGetCurrentStateFlags() & NAV_CTL_SPEED));
}

static void deactivateFixedWingAutoSpeed(int16_t throttleCommand)
{
    autoSpeedThrottleCommand = constrain(throttleCommand, PWM_RANGE_MIN, PWM_RANGE_MAX);
    autoSpeedLastUpdateTimeUs = 0;
    autoSpeedNeedsFreshSample = true;

    if (autoSpeedIsActive) {
        navPidReset(&posControl.pids.fw_autoSpeed);
    }

    // A later Auto Speed session starts from the throttle currently owned by
    // navigation or the VTOL transition instead of an old controller output.
    pt1FilterReset(&speedToThrFilterState, autoSpeedThrottleCommand - PWM_RANGE_MIDDLE);
    autoSpeedAirspeedBoost = false;
    autoSpeedIsActive = false;
}

void applyAutoSpeedThrottleDemand(int16_t *throttleCommand, timeUs_t currentTimeUs)
{
    const bool isFixedWing = STATE(AIRPLANE);
    const bool autoSpeedMayRun = navFixedWingAutoSpeedMayControlThrottle(
        isFixedWing,
        isFixedWing && isAutoSpeedEnabled(),
        mixerATIsActive());
    if (!autoSpeedMayRun) {
        deactivateFixedWingAutoSpeed(*throttleCommand);
        return;
    }
    autoSpeedIsActive = true;

    if (posControl.flags.horizontalPositionDataNew && posControl.flags.verticalPositionDataNew) {
        if (autoSpeedNeedsFreshSample) {
            autoSpeedLastUpdateTimeUs = currentTimeUs;
            autoSpeedNeedsFreshSample = false;
            if (!speedToThrFilterState.RC) {
                pt1FilterSetCutoff(&speedToThrFilterState, 1.0f / navConfig()->fw.auto_speed_thr_smoothing);
            }
            return;
        }

        timeUs_t dT = currentTimeUs - autoSpeedLastUpdateTimeUs;
        autoSpeedLastUpdateTimeUs = currentTimeUs;

        if (dT > MAX_POSITION_UPDATE_INTERVAL_US) {
            autoSpeedNeedsFreshSample = true;
            return;
        }

        uint16_t minSpeed = 100 * navConfig()->fw.auto_speed_min_speed;
        uint16_t maxSpeed = 100 * navConfig()->fw.auto_speed_max_speed;
        uint16_t minThrottle = MAX(getThrottleIdleValue(), currentBatteryProfile->nav.fw.min_throttle);
        uint16_t maxThrottle = currentBatteryProfile->nav.fw.max_throttle;

        bool useAirSpeed = !LOGIC_CONDITION_GLOBAL_FLAG(LOGIC_CONDITION_GLOBAL_FLAG_DISABLE_AUTOSPEED_AIRSPEED);
        if (IS_RC_MODE_ACTIVE(BOXAUTOSPEED)) {
            posControl.desiredState.autoSpeedDemand = scaleRange(rxGetChannelValue(navConfig()->fw.auto_speed_channel - 1), PWM_RANGE_MIN, PWM_RANGE_MAX, minSpeed, maxSpeed);
        } else {
            posControl.desiredState.autoSpeedDemand = constrain(getActiveSpeed(), minSpeed, maxSpeed);
            useAirSpeed = isAutoSpeedRequiredByNav() == FW_AUTO_SPD_AIR;
        }

        uint16_t actualSpeed = posControl.actualState.vel3D;
        posControl.autoSpeedSpdSource = FW_AUTO_SPD_GROUND;
        uint16_t groundSpeedBoost = 0;

#ifdef USE_PITOT
        if (pitotGetValidForAirspeed()) {
            // Pitot available and airspeed source selected or low airspeed boost applied when using ground speed source
            if (useAirSpeed || autoSpeedAirspeedBoost) {
                actualSpeed = getAirspeedEstimate();
                if (autoSpeedAirspeedBoost && actualSpeed > minSpeed) {
                    autoSpeedAirspeedBoost = false;
                }
                posControl.autoSpeedSpdSource = FW_AUTO_SPD_AIR;
            } else {    // Ground speed source selected
                // groundSpeedBoost used to increase ground speed demand when flying downwind using airspeed for correction
                const uint16_t airSpeed = getAirspeedEstimate();
                const int16_t tailWind = actualSpeed - airSpeed;
                if (tailWind > 0) {
                    groundSpeedBoost = MAX(tailWind - posControl.desiredState.autoSpeedDemand + minSpeed, 0);
                }

                if (airSpeed < 0.95 * minSpeed) {
                    autoSpeedAirspeedBoost = true;
                }
            }
            if (autoSpeedAirspeedBoost || groundSpeedBoost > 300) {
                posControl.autoSpeedSpdSource = FW_AUTO_SPD_GROUND_OVERRIDE;
            }
        } else
#endif
        {   // Ground speed source with no pitot - set minimum throttle to auto_speed_min_throttle with pitch2throttle correction to prevent downwind stall
            minThrottle = constrain(currentBatteryProfile->nav.fw.auto_speed_level_min_thr + fixedWingPitchToThrottleCorrection(-attitude.values.pitch, currentTimeUs),
                          minThrottle, maxThrottle);
        }

        const uint16_t desiredSpeed = posControl.desiredState.autoSpeedDemand + groundSpeedBoost;
        int16_t throttleCorr = navPidApply2(&posControl.pids.fw_autoSpeed, desiredSpeed, actualSpeed, US2S(dT), -PWM_RANGE_HALF, PWM_RANGE_HALF, 0);
        throttleCorr = pt1FilterApply3(&speedToThrFilterState, throttleCorr, US2S(dT));

        autoSpeedThrottleCommand = PWM_RANGE_MIDDLE + throttleCorr;

        // Boost throttle if ground speed too low
        bool speedBoostReq = (getMinGroundSpeed(navConfig()->general.min_ground_speed) * 100.0f - posControl.actualState.velXY > 0) && !throttleSpeedAdjustment;
        if (speedBoostReq || throttleSpeedAdjustment) {
            autoSpeedThrottleCommand += applyFixedWingMinSpeedController(currentTimeUs);
        }

        autoSpeedThrottleCommand = constrain(autoSpeedThrottleCommand, minThrottle, maxThrottle);

        // Indicate that information is no longer usable
        posControl.flags.horizontalPositionDataConsumed = true;

        // Blackbox speed demand uses target velocity that is otherwise unused for FW: X used for airspeed, Y for ground speed
        if (posControl.autoSpeedSpdSource == FW_AUTO_SPD_AIR) {
            navDesiredVelocity[X] = constrain(posControl.desiredState.autoSpeedDemand, 0, 32767);
            navDesiredVelocity[Y] = 0;
        } else {
            navDesiredVelocity[X] = posControl.autoSpeedSpdSource == FW_AUTO_SPD_GROUND_OVERRIDE ? 1 : 0;  // indicate airspeed overriding
            navDesiredVelocity[Y] = constrain(posControl.desiredState.autoSpeedDemand, 0, 32767);
        }
    }

    *throttleCommand = autoSpeedThrottleCommand;
}

/*-----------------------------------------------------------
 * Calculate loiter target based on current position and velocity
 *-----------------------------------------------------------*/
void calculateFixedWingInitialHoldPosition(fpVector3_t * pos)
{
    // TODO: stub, this should account for velocity and target loiter radius
    *pos = navGetCurrentActualPositionAndVelocity()->pos;
}

void resetFixedWingHeadingController(void)
{
    updateHeadingHoldTarget(CENTIDEGREES_TO_DEGREES(posControl.actualState.cog));
}

void applyFixedWingNavigationController(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs)
{
    if (navStateFlags & NAV_CTL_LAUNCH) {
        applyFixedWingLaunchController(currentTimeUs);
    }
    else if (navStateFlags & NAV_CTL_EMERG) {
        applyFixedWingEmergencyLandingController(currentTimeUs);
    }
    else {
#ifdef NAV_FW_LIMIT_MIN_FLY_VELOCITY
        // Don't apply anything if ground speed is too low (<3m/s)
        if (posControl.actualState.velXY > 300) {
#else
        if (true) {
#endif
            if (navStateFlags & NAV_CTL_ALT) {
                if (getMotorStatus() == MOTOR_STOPPED_USER || FLIGHT_MODE(SOARING_MODE)) {
                    // Motor has been stopped by user or soaring mode enabled to override altitude control
                    resetFixedWingAltitudeController();
                    setDesiredPosition(&navGetCurrentActualPositionAndVelocity()->pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);
                } else {
                    applyFixedWingAltitudeAndThrottleController(currentTimeUs);
                }
            }

            if (navStateFlags & NAV_CTL_POS) {
                applyFixedWingPositionController(currentTimeUs);
            }

        } else {
            posControl.rcAdjustment[PITCH] = 0;
            posControl.rcAdjustment[ROLL] = 0;
        }

        if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE) && posControl.flags.isAdjustingPosition) {
            rcCommand[ROLL] = applyDeadbandRescaled(rcCommand[ROLL], rcControlsConfig()->pos_hold_deadband, -500, 500);
        }

        //if (navStateFlags & NAV_CTL_YAW)
        if ((navStateFlags & NAV_CTL_ALT) || (navStateFlags & NAV_CTL_POS)) {
            applyFixedWingPitchRollThrottleController(navStateFlags, currentTimeUs);
        }

        if (FLIGHT_MODE(SOARING_MODE) && navConfig()->general.flags.soaring_motor_stop) {
            ENABLE_STATE(NAV_MOTOR_STOP_OR_IDLE);
        }
    }
}

float navigationGetCrossTrackError(void)
{
    return navCrossTrackError;
}

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

#include "build/build_config.h"

#include "build/debug.h"

#include "common/axis.h"

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "io/beeper.h"

#include "fc/fc_core.h"
#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/controlrate_profile.h"
#include "fc/settings.h"

#include "flight/failsafe.h"
#include "flight/mixer.h"
#include "flight/pid.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "rx/rx.h"

#include "sensors/battery.h"
#include "sensors/sensors.h"

/*
 * Usage:
 *
 * failsafeInit() and failsafeReset() must be called before the other methods are used.
 *
 * failsafeInit() and failsafeReset() can be called in any order.
 * failsafeInit() should only be called once.
 *
 * enable() should be called after system initialisation.
 */

static failsafeState_t failsafeState;

PG_REGISTER_WITH_RESET_TEMPLATE(failsafeConfig_t, failsafeConfig, PG_FAILSAFE_CONFIG, 3);

PG_RESET_TEMPLATE(failsafeConfig_t, failsafeConfig,
    .failsafe_delay = SETTING_FAILSAFE_DELAY_DEFAULT,                                   // 0.5 sec
    .failsafe_recovery_delay = SETTING_FAILSAFE_RECOVERY_DELAY_DEFAULT,                 // 0.5 seconds (plus 200ms explicit delay)
    .failsafe_off_delay = SETTING_FAILSAFE_OFF_DELAY_DEFAULT,                           // 20sec
    .failsafe_throttle_low_delay = SETTING_FAILSAFE_THROTTLE_LOW_DELAY_DEFAULT,         // default throttle low delay for "just disarm" on failsafe condition
    .failsafe_procedure = SETTING_FAILSAFE_PROCEDURE_DEFAULT,                           // default full failsafe procedure
    .failsafe_fw_roll_angle = SETTING_FAILSAFE_FW_ROLL_ANGLE_DEFAULT,                   // 20 deg left
    .failsafe_fw_pitch_angle = SETTING_FAILSAFE_FW_PITCH_ANGLE_DEFAULT,                 // 10 deg dive (yes, positive means dive)
    .failsafe_fw_yaw_rate = SETTING_FAILSAFE_FW_YAW_RATE_DEFAULT,                       // 45 deg/s left yaw (left is negative, 8s for full turn)
    .failsafe_stick_motion_threshold = SETTING_FAILSAFE_STICK_THRESHOLD_DEFAULT,
    .failsafe_min_distance = SETTING_FAILSAFE_MIN_DISTANCE_DEFAULT,                     // No minimum distance for failsafe by default
    .failsafe_min_distance_procedure = SETTING_FAILSAFE_MIN_DISTANCE_PROCEDURE_DEFAULT, // default minimum distance failsafe procedure
    .failsafe_mission_delay = SETTING_FAILSAFE_MISSION_DELAY_DEFAULT,                   // Time delay before Failsafe activated during WP mission (s)
#ifdef USE_GPS_FIX_ESTIMATION
    .failsafe_gps_fix_estimation_delay = SETTING_FAILSAFE_GPS_FIX_ESTIMATION_DELAY_DEFAULT, // Time delay before Failsafe activated when GPS Fix estimation is allied
#endif
);

typedef enum {
    FAILSAFE_CHANNEL_HOLD,      // Hold last known good value
    FAILSAFE_CHANNEL_NEUTRAL,   // RPY = zero, THR = zero
} failsafeChannelBehavior_e;

typedef struct {
    bool                        bypassNavigation;
    bool                        forceAngleMode;
    failsafeChannelBehavior_e   channelBehavior[4];
} failsafeProcedureLogic_t;

static const failsafeProcedureLogic_t failsafeProcedureLogic[] = {
    [FAILSAFE_PROCEDURE_AUTO_LANDING] = {
            .forceAngleMode = true,
            .bypassNavigation = false,
            .channelBehavior = {
                FAILSAFE_CHANNEL_NEUTRAL,       // ROLL
                FAILSAFE_CHANNEL_NEUTRAL,       // PITCH
                FAILSAFE_CHANNEL_NEUTRAL,       // YAW
                FAILSAFE_CHANNEL_HOLD           // THROTTLE
            }
    },

    [FAILSAFE_PROCEDURE_DROP_IT] = {
            .bypassNavigation = true,
            .forceAngleMode = true,
            .channelBehavior = {
                FAILSAFE_CHANNEL_NEUTRAL,       // ROLL
                FAILSAFE_CHANNEL_NEUTRAL,       // PITCH
                FAILSAFE_CHANNEL_NEUTRAL,       // YAW
                FAILSAFE_CHANNEL_NEUTRAL        // THROTTLE
            }
    },

    [FAILSAFE_PROCEDURE_RTH] = {
            .bypassNavigation = false,
            .forceAngleMode = true,
            .channelBehavior = {
                FAILSAFE_CHANNEL_NEUTRAL,       // ROLL
                FAILSAFE_CHANNEL_NEUTRAL,       // PITCH
                FAILSAFE_CHANNEL_NEUTRAL,       // YAW
                FAILSAFE_CHANNEL_HOLD           // THROTTLE
            }
    },

    [FAILSAFE_PROCEDURE_NONE] = {
            .bypassNavigation = false,
            .forceAngleMode = false,
            .channelBehavior = {
                FAILSAFE_CHANNEL_HOLD,          // ROLL
                FAILSAFE_CHANNEL_HOLD,          // PITCH
                FAILSAFE_CHANNEL_HOLD,          // YAW
                FAILSAFE_CHANNEL_HOLD           // THROTTLE
            }
    }
};

/*
 * Should called when the failsafe config needs to be changed - e.g. a different profile has been selected.
 */
void failsafeReset(void)
{
    if (failsafeState.active) {  // can't reset if still active
        return;
    }

    failsafeState.rxDataFailurePeriod = PERIOD_RXDATA_FAILURE + failsafeConfig()->failsafe_delay * MILLIS_PER_TENTH_SECOND;
    failsafeState.rxDataRecoveryPeriod = PERIOD_RXDATA_RECOVERY + failsafeConfig()->failsafe_recovery_delay * MILLIS_PER_TENTH_SECOND;
    failsafeState.validRxDataReceivedAt = 0;
    failsafeState.validRxDataFailedAt = 0;
    failsafeState.throttleLowPeriod = 0;
    failsafeState.landingShouldBeFinishedAt = 0;
    failsafeState.receivingRxDataPeriod = 0;
    failsafeState.receivingRxDataPeriodPreset = 0;
    failsafeState.phase = FAILSAFE_IDLE;
    failsafeState.rxLinkState = FAILSAFE_RXLINK_DOWN;
    failsafeState.activeProcedure = failsafeConfig()->failsafe_procedure;
    failsafeState.controlling = false;

    failsafeState.lastGoodRcCommand[ROLL] = 0;
    failsafeState.lastGoodRcCommand[PITCH] = 0;
    failsafeState.lastGoodRcCommand[YAW] = 0;
    failsafeState.lastGoodRcCommand[THROTTLE] = 1000;
}

void failsafeInit(void)
{
    failsafeState.events = 0;
    failsafeState.monitoring = false;
    failsafeState.suspended = false;
}

bool failsafeBypassNavigation(void)
{
    return failsafeState.active &&
           failsafeState.controlling &&
           failsafeProcedureLogic[failsafeState.activeProcedure].bypassNavigation;
}

bool failsafeMayRequireNavigationMode(void)
{
    return (failsafeConfig()->failsafe_procedure == FAILSAFE_PROCEDURE_RTH) ||
           (failsafeConfig()->failsafe_min_distance_procedure == FAILSAFE_PROCEDURE_RTH);
}

failsafePhase_e failsafePhase(void)
{
    return failsafeState.phase;
}

bool failsafeIsMonitoring(void)
{
    return failsafeState.monitoring;
}

bool failsafeIsActive(void)
{
    return failsafeState.active;
}

bool failsafeShouldApplyControlInput(void)
{
    return failsafeState.controlling;
}

bool failsafeRequiresAngleMode(void)
{
    return failsafeState.active &&
           failsafeState.controlling &&
           failsafeProcedureLogic[failsafeState.activeProcedure].forceAngleMode;
}

void failsafeStartMonitoring(void)
{
    failsafeState.monitoring = true;
}

static bool failsafeShouldHaveCausedLandingByNow(void)
{
    return failsafeConfig()->failsafe_off_delay && (millis() > failsafeState.landingShouldBeFinishedAt);
}

static void failsafeSetActiveProcedure(failsafeProcedure_e procedure)
{
    failsafeState.activeProcedure = procedure;
}

static void failsafeActivate(failsafePhase_e newPhase)
{
    failsafeState.active = true;
    failsafeState.controlling = true;
    failsafeState.phase = newPhase;
    ENABLE_FLIGHT_MODE(FAILSAFE_MODE);
    failsafeState.landingShouldBeFinishedAt = millis() + failsafeConfig()->failsafe_off_delay * MILLIS_PER_TENTH_SECOND;

#ifdef USE_FW_AUTOLAND
    posControl.fwLandState.landAborted = false;
#endif

    failsafeState.events++;
}

void failsafeUpdateRcCommandValues(void)
{
    if (!failsafeState.active) {
        for (int idx = 0; idx < 4; idx++) {
            failsafeState.lastGoodRcCommand[idx] = rcCommand[idx];
        }
    }
}

void failsafeApplyControlInput(void)
{
    // Apply channel values
    for (int idx = 0; idx < 4; idx++) {
        switch (failsafeProcedureLogic[failsafeState.activeProcedure].channelBehavior[idx]) {
            case FAILSAFE_CHANNEL_HOLD:
                rcCommand[idx] = failsafeState.lastGoodRcCommand[idx];
                break;

            case FAILSAFE_CHANNEL_NEUTRAL:
                switch (idx) {
                    case ROLL:
                    case PITCH:
                    case YAW:
                        rcCommand[idx] = 0;
                        break;

                    case THROTTLE:
                        rcCommand[idx] = feature(FEATURE_REVERSIBLE_MOTORS) ? PWM_RANGE_MIDDLE : getThrottleIdleValue();
                        break;
                }
                break;
        }
    }
}

bool failsafeIsReceivingRxData(void)
{
    return (failsafeState.rxLinkState == FAILSAFE_RXLINK_UP);
}

void failsafeOnRxSuspend(void)
{
    failsafeState.suspended = true;
}

bool failsafeIsSuspended(void)
{
    return failsafeState.suspended;
}

void failsafeOnRxResume(void)
{
    failsafeState.suspended = false;                                    // restart monitoring
    failsafeState.validRxDataReceivedAt = millis();                     // prevent RX link down trigger, restart rx link up
    failsafeState.rxLinkState = FAILSAFE_RXLINK_UP;                     // do so while rx link is up
}

void failsafeOnValidDataReceived(void)
{
    failsafeState.validRxDataReceivedAt = millis();
    if ((failsafeState.validRxDataReceivedAt - failsafeState.validRxDataFailedAt) > failsafeState.rxDataRecoveryPeriod) {
        failsafeState.rxLinkState = FAILSAFE_RXLINK_UP;
    }
}

void failsafeOnValidDataFailed(void)
{
    failsafeState.validRxDataFailedAt = millis();
    if ((failsafeState.validRxDataFailedAt - failsafeState.validRxDataReceivedAt) > failsafeState.rxDataFailurePeriod) {
        failsafeState.rxLinkState = FAILSAFE_RXLINK_DOWN;
    }
}

static bool failsafeCheckStickMotion(void)
{
    if (failsafeConfig()->failsafe_stick_motion_threshold > 0) {
        uint32_t totalRcDelta = 0;

        totalRcDelta += ABS(rxGetChannelValue(ROLL) - PWM_RANGE_MIDDLE);
        totalRcDelta += ABS(rxGetChannelValue(PITCH) - PWM_RANGE_MIDDLE);
        totalRcDelta += ABS(rxGetChannelValue(YAW) - PWM_RANGE_MIDDLE);

        return totalRcDelta >= failsafeConfig()->failsafe_stick_motion_threshold;
    }
    else {
        return true;
    }
}

static failsafeProcedure_e failsafeChooseFailsafeProcedure(void)
{
    if ((FLIGHT_MODE(NAV_WP_MODE) || isWaypointMissionRTHActive()) && failsafeConfig()->failsafe_mission_delay) {
        if (!failsafeState.wpModeDelayedFailsafeStart) {
            failsafeState.wpModeDelayedFailsafeStart = millis();
            return FAILSAFE_PROCEDURE_NONE;
        } else if ((millis() - failsafeState.wpModeDelayedFailsafeStart < (MILLIS_PER_SECOND * (uint16_t)failsafeConfig()->failsafe_mission_delay)) ||
                   failsafeConfig()->failsafe_mission_delay == -1) {
            return FAILSAFE_PROCEDURE_NONE;
        }
    }

    // Inhibit Failsafe if emergency landing triggered manually or if landing is detected
    if (posControl.flags.manualEmergLandActive || STATE(LANDING_DETECTED)) {
        return FAILSAFE_PROCEDURE_NONE;
    }

    // Craft is closer than minimum failsafe procedure distance (if set to non-zero)
    // GPS must also be working, and home position set
    if (failsafeConfig()->failsafe_min_distance > 0 &&
            ((sensors(SENSOR_GPS) && STATE(GPS_FIX))
#ifdef USE_GPS_FIX_ESTIMATION
                || STATE(GPS_ESTIMATED_FIX)
#endif
                ) && STATE(GPS_FIX_HOME)) {

        // get the distance to the original arming point
        uint32_t distance = calculateDistanceToDestination(&posControl.rthState.originalHomePosition);
        if (distance < failsafeConfig()->failsafe_min_distance) {
            // Use the alternate, minimum distance failsafe procedure instead
            return failsafeConfig()->failsafe_min_distance_procedure;
        }
    }

    return failsafeConfig()->failsafe_procedure;
}

#ifdef USE_GPS_FIX_ESTIMATION
bool checkGPSFixFailsafe(void)
{
    if (STATE(GPS_ESTIMATED_FIX) && (FLIGHT_MODE(NAV_WP_MODE) || isWaypointMissionRTHActive()) && (failsafeConfig()->failsafe_gps_fix_estimation_delay >= 0)) {
        if (!failsafeState.wpModeGPSFixEstimationDelayedFailsafeStart) {
            failsafeState.wpModeGPSFixEstimationDelayedFailsafeStart = millis();
        } else if ((millis() - failsafeState.wpModeGPSFixEstimationDelayedFailsafeStart) > (MILLIS_PER_SECOND * (uint16_t)MAX(failsafeConfig()->failsafe_gps_fix_estimation_delay,7))) {
            if ( !posControl.flags.forcedRTHActivated ) {
                failsafeSetActiveProcedure(FAILSAFE_PROCEDURE_RTH);
                failsafeActivate(FAILSAFE_RETURN_TO_HOME);
                activateForcedRTH();
                return true;
            }
        }
    } else {
        failsafeState.wpModeGPSFixEstimationDelayedFailsafeStart = 0;
    }
    return false;
}
#endif


void failsafeUpdateState(void)
{
    if (!failsafeIsMonitoring() || failsafeIsSuspended()) {
        return;
    }

    const bool receivingRxDataAndNotFailsafeMode = failsafeIsReceivingRxData() && !IS_RC_MODE_ACTIVE(BOXFAILSAFE);
    const bool armed = ARMING_FLAG(ARMED);
    const bool sticksAreMoving = failsafeCheckStickMotion();
    beeperMode_e beeperMode = BEEPER_SILENCE;

    // Beep RX lost only if we are not seeing data and we have been armed earlier
    if (!receivingRxDataAndNotFailsafeMode && ARMING_FLAG(WAS_EVER_ARMED)) {
        beeperMode = BEEPER_RX_LOST;
    }

    bool reprocessState;

    do {
        reprocessState = false;

        switch (failsafeState.phase) {
            case FAILSAFE_IDLE:
                if (armed) {
                    // Track throttle command below minimum time
                    if (!throttleStickIsLow()) {
                        failsafeState.throttleLowPeriod = millis() + failsafeConfig()->failsafe_throttle_low_delay * MILLIS_PER_TENTH_SECOND;
                    }

#ifdef USE_GPS_FIX_ESTIMATION
                    if ( checkGPSFixFailsafe() ) {
                        reprocessState = true;
                    } else
#endif
                    if (!receivingRxDataAndNotFailsafeMode) {
                        if ((failsafeConfig()->failsafe_throttle_low_delay && (millis() > failsafeState.throttleLowPeriod)) || STATE(NAV_MOTOR_STOP_OR_IDLE)) {
                            // JustDisarm: throttle was LOW for at least 'failsafe_throttle_low_delay' seconds or waiting for launch
                            // Don't disarm at all if `failsafe_throttle_low_delay` is set to zero
                            failsafeSetActiveProcedure(FAILSAFE_PROCEDURE_DROP_IT);
                            failsafeActivate(FAILSAFE_LANDED);  // skip auto-landing procedure
                            failsafeState.receivingRxDataPeriodPreset = PERIOD_OF_3_SECONDS; // require 3 seconds of valid rxData
                        } else {
                            failsafeState.phase = FAILSAFE_RX_LOSS_DETECTED;
                            failsafeState.wpModeDelayedFailsafeStart = 0;
                        }
                        reprocessState = true;
                    }
                } else {
                    // When NOT armed, show rxLinkState of failsafe switch in GUI (failsafe mode)
                    if (!receivingRxDataAndNotFailsafeMode) {
                        ENABLE_FLIGHT_MODE(FAILSAFE_MODE);
                    } else {
                        DISABLE_FLIGHT_MODE(FAILSAFE_MODE);
                    }
                    // Throttle low period expired (= low long enough for JustDisarm)
                    failsafeState.throttleLowPeriod = 0;
                }
                break;

            case FAILSAFE_RX_LOSS_DETECTED:
                if (receivingRxDataAndNotFailsafeMode) {
                    failsafeState.phase = FAILSAFE_RX_LOSS_RECOVERED;
                } else {
                    // Set active failsafe procedure
                    failsafeSetActiveProcedure(failsafeChooseFailsafeProcedure());

                    switch (failsafeState.activeProcedure) {
                        case FAILSAFE_PROCEDURE_AUTO_LANDING:
                            // Use Emergency Landing if Nav defined (otherwise stabilize and set Throttle to specified level).
                            failsafeActivate(FAILSAFE_LANDING);
                            activateForcedEmergLanding();
                            break;

                        case FAILSAFE_PROCEDURE_DROP_IT:
                            // Drop the craft
                            failsafeActivate(FAILSAFE_LANDED);      // skip auto-landing procedure
                            failsafeState.receivingRxDataPeriodPreset = PERIOD_OF_3_SECONDS; // require 3 seconds of valid rxData
                            break;

                        case FAILSAFE_PROCEDURE_RTH:
                            // Proceed to handling & monitoring RTH navigation
                            failsafeActivate(FAILSAFE_RETURN_TO_HOME);
                            activateForcedRTH();
                            break;
                        case FAILSAFE_PROCEDURE_NONE:
                        default:
                            // Do nothing procedure
                            failsafeActivate(FAILSAFE_RX_LOSS_IDLE);
                            break;
                    }
                }
                reprocessState = true;
                break;

            /* A very simple do-nothing failsafe procedure. The only thing it will do is monitor the receiver state and switch out of FAILSAFE condition */
            case FAILSAFE_RX_LOSS_IDLE:
                if (receivingRxDataAndNotFailsafeMode && sticksAreMoving) {
                    failsafeState.phase = FAILSAFE_RX_LOSS_RECOVERED;
                    reprocessState = true;
                } else if (failsafeChooseFailsafeProcedure() != FAILSAFE_PROCEDURE_NONE) {  // trigger new failsafe procedure if changed
                    failsafeState.phase = FAILSAFE_RX_LOSS_DETECTED;
                    reprocessState = true;
                }
#ifdef USE_GPS_FIX_ESTIMATION
                else {
                    if ( checkGPSFixFailsafe() ) {
                        reprocessState = true;
                    }
                }
#endif

                break;

            case FAILSAFE_RETURN_TO_HOME:
                if (receivingRxDataAndNotFailsafeMode && sticksAreMoving) {
                    abortForcedRTH();
                    failsafeState.phase = FAILSAFE_RX_LOSS_RECOVERED;
                    reprocessState = true;
                }
                else {
                    if (armed) {
                        beeperMode = BEEPER_RX_LOST_LANDING;
                    }
                    bool rthLanded = false;
                    switch (getStateOfForcedRTH()) {
                        case RTH_IN_PROGRESS:
                            break;

                        case RTH_HAS_LANDED:
                            rthLanded = true;
                            break;

                        case RTH_IDLE:
                        default:
                            // This shouldn't happen. If RTH was somehow aborted during failsafe - fallback to FAILSAFE_LANDING procedure
                            abortForcedRTH();
                            failsafeSetActiveProcedure(FAILSAFE_PROCEDURE_AUTO_LANDING);
                            failsafeActivate(FAILSAFE_LANDING);
                            activateForcedEmergLanding();
                            reprocessState = true;
                            break;
                    }
                    if (rthLanded || !armed) {
                        failsafeState.receivingRxDataPeriodPreset = PERIOD_OF_30_SECONDS; // require 30 seconds of valid rxData
                        failsafeState.phase = FAILSAFE_LANDED;
                        reprocessState = true;
                    }
                }
                break;

            case FAILSAFE_LANDING:
                if (receivingRxDataAndNotFailsafeMode && sticksAreMoving) {
                    abortForcedEmergLanding();
                    failsafeState.phase = FAILSAFE_RX_LOSS_RECOVERED;
                    reprocessState = true;
                } else {
                    if (armed) {
                        beeperMode = BEEPER_RX_LOST_LANDING;
                    }
                    bool emergLanded = false;
                    switch (getStateOfForcedEmergLanding()) {
                        case EMERG_LAND_IN_PROGRESS:
                            break;

                        case EMERG_LAND_HAS_LANDED:
                            emergLanded = true;
                            break;

                        case EMERG_LAND_IDLE:
                        default:
                            // If emergency landing was somehow aborted during failsafe - fallback to FAILSAFE_PROCEDURE_DROP_IT
                            abortForcedEmergLanding();
                            failsafeSetActiveProcedure(FAILSAFE_PROCEDURE_DROP_IT);
                            failsafeActivate(FAILSAFE_LANDED);
                            reprocessState = true;
                            break;
                    }
                    if (emergLanded || failsafeShouldHaveCausedLandingByNow() || !armed) {
                        failsafeState.receivingRxDataPeriodPreset = PERIOD_OF_30_SECONDS; // require 30 seconds of valid rxData
                        failsafeState.phase = FAILSAFE_LANDED;
                        reprocessState = true;
                    }
                }
                break;

            case FAILSAFE_LANDED:
                ENABLE_ARMING_FLAG(ARMING_DISABLED_FAILSAFE_SYSTEM); // To prevent accidently rearming by an intermittent rx link
                disarm(DISARM_FAILSAFE);
                failsafeState.receivingRxDataPeriod = millis() + failsafeState.receivingRxDataPeriodPreset; // set required period of valid rxData
                failsafeState.phase = FAILSAFE_RX_LOSS_MONITORING;
                failsafeState.controlling = false;  // Failsafe no longer in control of the machine - release control to pilot
                reprocessState = true;
                break;

            case FAILSAFE_RX_LOSS_MONITORING:
                // Monitoring the rx link to allow rearming when it has become good for > `receivingRxDataPeriodPreset` time.
                if (receivingRxDataAndNotFailsafeMode) {
                    if (millis() > failsafeState.receivingRxDataPeriod) {
                        // rx link is good now, when arming via ARM switch, it must be OFF first
                        if (!IS_RC_MODE_ACTIVE(BOXARM)) {
                            // XXX: Requirements for removing the ARMING_DISABLED_FAILSAFE_SYSTEM flag
                            // are tested by osd.c to show the user how to re-arm. If these
                            // requirements change, update osdArmingDisabledReasonMessage().
                            DISABLE_ARMING_FLAG(ARMING_DISABLED_FAILSAFE_SYSTEM);
                            failsafeState.phase = FAILSAFE_RX_LOSS_RECOVERED;
                            reprocessState = true;
                        }
                    }
                } else {
                    failsafeState.receivingRxDataPeriod = millis() + failsafeState.receivingRxDataPeriodPreset;
                }
                break;

            case FAILSAFE_RX_LOSS_RECOVERED:
                // Entering IDLE with the requirement that throttle first must be at min_check for failsafe_throttle_low_delay period.
                // This is to prevent that JustDisarm is activated on the next iteration.
                // Because that would have the effect of shutting down failsafe handling on intermittent connections.
                failsafeState.throttleLowPeriod = millis() + failsafeConfig()->failsafe_throttle_low_delay * MILLIS_PER_TENTH_SECOND;
                failsafeState.phase = FAILSAFE_IDLE;
                failsafeState.active = false;
                failsafeState.controlling = false;
                DISABLE_FLIGHT_MODE(FAILSAFE_MODE);
                reprocessState = true;
                break;

            default:
                break;
        }
    } while (reprocessState);

    if (beeperMode != BEEPER_SILENCE) {
        beeper(beeperMode);
    }
}

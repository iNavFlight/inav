#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"
#include "config/config_reset.h"

#include "drivers/pwm_output.h"
#include "drivers/pwm_mapping.h"
#include "drivers/time.h"
#include "common/maths.h"
#include "flight/mixer.h"
#include "common/axis.h"
#include "flight/pid.h"
#include "flight/servos.h"
#include "flight/failsafe.h"
#include "navigation/navigation.h"
#include "navigation/navigation_private.h"
#include "sensors/pitotmeter.h"
#include "sensors/sensors.h"

#include "fc/fc_core.h"
#include "fc/config.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"
#include "fc/rc_modes.h"
#include "fc/cli.h"

#include "programming/logic_condition.h"
#include "navigation/navigation.h"

#include "common/log.h"
#include "build/debug.h"

mixerConfig_t currentMixerConfig;
int currentMixerProfileIndex;
bool isMixerTransitionMixing;
bool isMixerTransitionMixing_requested;
mixerProfileAT_t mixerProfileAT;
int nextMixerProfileIndex;
static bool manualTransitionModeWasActive;
static bool manualTransitionReadyForEdge = true;
static bool manualTransitionSessionLatched;

PG_REGISTER_ARRAY_WITH_RESET_FN(mixerProfile_t, MAX_MIXER_PROFILE_COUNT, mixerProfiles, PG_MIXER_PROFILE, 4);

void pgResetFn_mixerProfiles(mixerProfile_t *instance)
{
    for (int i = 0; i < MAX_MIXER_PROFILE_COUNT; i++)
    {
        RESET_CONFIG(mixerProfile_t, &instance[i],
                     .mixer_config = {
                         .motorDirectionInverted = SETTING_MOTOR_DIRECTION_INVERTED_DEFAULT,
                         .platformType = SETTING_PLATFORM_TYPE_DEFAULT,
                         .hasFlaps = SETTING_HAS_FLAPS_DEFAULT,
                         .appliedMixerPreset = SETTING_MODEL_PREVIEW_TYPE_DEFAULT, // This flag is not available in CLI and used by Configurator only
                         .motorstopOnLow = SETTING_MOTORSTOP_ON_LOW_DEFAULT,
                         .controlProfileLinking = SETTING_MIXER_CONTROL_PROFILE_LINKING_DEFAULT,
                         .automated_switch = SETTING_MIXER_AUTOMATED_SWITCH_DEFAULT,
                         .switchTransitionTimer =  SETTING_MIXER_SWITCH_TRANS_TIMER_DEFAULT,
                         .vtolTransitionDynamicMixer = SETTING_MIXER_VTOL_TRANSITION_DYNAMIC_MIXER_DEFAULT,
                         .manualVtolTransitionController = SETTING_MIXER_VTOL_MANUALSWITCH_AUTOTRANSITION_CONTROLLER_DEFAULT,
                         .vtolTransitionAirspeedTimeoutMs = SETTING_MIXER_VTOL_TRANSITION_AIRSPEED_TIMEOUT_MS_DEFAULT,
                         .vtolTransitionScaleRampTimeMs = SETTING_MIXER_VTOL_TRANSITION_SCALE_RAMP_TIME_MS_DEFAULT,
                         .tailsitterOrientationOffset = SETTING_TAILSITTER_ORIENTATION_OFFSET_DEFAULT,
                         .transition_PID_mmix_multiplier_roll = SETTING_TRANSITION_PID_MMIX_MULTIPLIER_ROLL_DEFAULT,
                         .transition_PID_mmix_multiplier_pitch = SETTING_TRANSITION_PID_MMIX_MULTIPLIER_PITCH_DEFAULT,
                         .transition_PID_mmix_multiplier_yaw = SETTING_TRANSITION_PID_MMIX_MULTIPLIER_YAW_DEFAULT
                     });
        for (int j = 0; j < MAX_SUPPORTED_MOTORS; j++)
        {
            RESET_CONFIG(motorMixer_t, &instance[i].MotorMixers[j],
                         .throttle = 0,
                         .roll = 0,
                         .pitch = 0,
                         .yaw = 0);
        }
        for (int j = 0; j < MAX_SERVO_RULES; j++)
        {
            RESET_CONFIG(servoMixer_t, &instance[i].ServoMixers[j],
                         .targetChannel = 0,
                         .inputSource = 0,
                         .rate = 0,
                         .speed = 0
#ifdef USE_PROGRAMMING_FRAMEWORK
                         ,
                         .conditionId = -1,
#endif
            );
        }
    }
}

void activateMixerConfig(void){
    currentMixerProfileIndex = getConfigMixerProfile();
    currentMixerConfig = *mixerConfig();
    nextMixerProfileIndex = (currentMixerProfileIndex + 1) % MAX_MIXER_PROFILE_COUNT;
}

void mixerConfigInit(void)
{
    activateMixerConfig();
    servosInit();
    mixerUpdateStateFlags();
    mixerInit();
    if (currentMixerConfig.controlProfileLinking)
    {
        // LOG_INFO(PWM, "mixer switch pidInit");
        setConfigProfile(getConfigMixerProfile());
        pidInit();
        pidInitFilters();
        pidResetErrorAccumulators(); //should be safer to reset error accumulators
        schedulePidGainsUpdate();
        navigationUsePIDs(); // set navigation pid gains
    }
}

void setMixerProfileAT(void)
{
    const timeMs_t now = millis();

    mixerProfileAT.transitionStartTime = now;
    mixerProfileAT.aborted = false;
    mixerProfileAT.abortedByAirspeedTimeout = false;
    mixerProfileAT.hotSwitchDone = false;
    mixerProfileAT.usedAirspeed = false;
    mixerProfileAT.transitionStartAirspeedCaptured = false;
    mixerProfileAT.progress = 0.0f;
    mixerProfileAT.scalingProgress = 0.0f;
    mixerProfileAT.transitionStartAirspeedCmS = 0.0f;
    mixerProfileAT.blendToFw = mixerProfileAT.direction == MIXERAT_DIRECTION_TO_FW ? 0.0f : 1.0f;
    mixerProfileAT.pusherScale = 1.0f;
    mixerProfileAT.liftScale = 1.0f;
    mixerProfileAT.mcAuthorityScale = 1.0f;
    mixerProfileAT.fwAuthorityScale = 1.0f;
}

static bool requestTransitionsToFixedWing(const mixerProfileATRequest_e required_action)
{
    return required_action == MIXERAT_REQUEST_RTH ||
           required_action == MIXERAT_REQUEST_MISSION_TO_FW ||
           required_action == MIXERAT_REQUEST_MANUAL_TO_FW;
}

static mixerProfileATDirection_e directionForRequest(const mixerProfileATRequest_e required_action)
{
    if (requestTransitionsToFixedWing(required_action)) {
        return MIXERAT_DIRECTION_TO_FW;
    }

    if (required_action == MIXERAT_REQUEST_LAND ||
        required_action == MIXERAT_REQUEST_MISSION_TO_MC ||
        required_action == MIXERAT_REQUEST_MANUAL_TO_MC) {
        return MIXERAT_DIRECTION_TO_MC;
    }

    return MIXERAT_DIRECTION_NONE;
}

static void resetTransitionScales(void)
{
    mixerProfileAT.progress = 0.0f;
    mixerProfileAT.scalingProgress = 0.0f;
    mixerProfileAT.blendToFw = 0.0f;
    mixerProfileAT.pusherScale = 0.0f;
    mixerProfileAT.liftScale = 1.0f;
    mixerProfileAT.mcAuthorityScale = 1.0f;
    mixerProfileAT.fwAuthorityScale = 1.0f;
}

static void setLegacyTransitionScales(void)
{
    mixerProfileAT.progress = 1.0f;
    mixerProfileAT.scalingProgress = 1.0f;
    mixerProfileAT.blendToFw = 1.0f;
    mixerProfileAT.pusherScale = 1.0f;
    mixerProfileAT.liftScale = 1.0f;
    mixerProfileAT.mcAuthorityScale = 1.0f;
    mixerProfileAT.fwAuthorityScale = 1.0f;
}

static float blendScale(float from, float to, float progress)
{
    return from + (to - from) * constrainf(progress, 0.0f, 1.0f);
}

static float getScalingProgress(void)
{
    if (!currentMixerConfig.vtolTransitionDynamicMixer) {
        return 1.0f;
    }

    if (mixerProfileAT.usedAirspeed) {
        mixerProfileAT.scalingProgress = constrainf(mixerProfileAT.progress, 0.0f, 1.0f);
        return mixerProfileAT.scalingProgress;
    }

    if (currentMixerConfig.vtolTransitionScaleRampTimeMs > 0) {
        const uint32_t elapsedMs = millis() - mixerProfileAT.transitionStartTime;
        const float rampProgress = constrainf((float)elapsedMs / (float)currentMixerConfig.vtolTransitionScaleRampTimeMs, 0.0f, 1.0f);

        // Preserve already-applied scaling if pitot drops out mid-transition.
        mixerProfileAT.scalingProgress = MAX(mixerProfileAT.scalingProgress, rampProgress);
        return mixerProfileAT.scalingProgress;
    }

    // Last-resort compatibility path: with no trusted pitot and no dedicated
    // scaling ramp configured, reuse transition progress/timer behavior.
    mixerProfileAT.scalingProgress = MAX(mixerProfileAT.scalingProgress, constrainf(mixerProfileAT.progress, 0.0f, 1.0f));
    return mixerProfileAT.scalingProgress;
}

static bool hasTrustedPitotAirspeed(float *airspeedCmS)
{
#ifdef USE_PITOT
    if (!sensors(SENSOR_PITOT) || !pitotValidForAirspeed() || pitotHasFailed()) {
        return false;
    }

    if (detectedSensors[SENSOR_INDEX_PITOT] == PITOT_NONE ||
        detectedSensors[SENSOR_INDEX_PITOT] == PITOT_VIRTUAL) {
        return false;
    }

    *airspeedCmS = pitot.airSpeed;
    return true;
#else
    UNUSED(airspeedCmS);
    return false;
#endif
}

static bool hasPitotSensorForManualProtection(void)
{
#ifdef USE_PITOT
    if (!sensors(SENSOR_PITOT) || pitotHasFailed()) {
        return false;
    }

    if (detectedSensors[SENSOR_INDEX_PITOT] == PITOT_NONE ||
        detectedSensors[SENSOR_INDEX_PITOT] == PITOT_VIRTUAL) {
        return false;
    }

    return true;
#else
    return false;
#endif
}

static uint16_t getAirspeedThresholdForDirection(const mixerProfileATDirection_e direction)
{
    if (direction == MIXERAT_DIRECTION_TO_FW) {
        return systemConfig()->vtolTransitionToFwMinAirspeed;
    }

    if (direction == MIXERAT_DIRECTION_TO_MC) {
        return systemConfig()->vtolTransitionToMcMaxAirspeed;
    }

    return 0;
}

static bool shouldBlockManualDirectSwitchToFixedWing(const bool manualControllerEnabled, const int requestedProfileIndex)
{
    if (!manualControllerEnabled || !STATE(MULTIROTOR) || requestedProfileIndex == currentMixerProfileIndex) {
        return false;
    }

    if (mixerConfigByIndex(requestedProfileIndex)->platformType != PLATFORM_AIRPLANE) {
        return false;
    }

    const uint16_t thresholdCmS = getAirspeedThresholdForDirection(MIXERAT_DIRECTION_TO_FW);
    if (thresholdCmS == 0 || !hasPitotSensorForManualProtection()) {
        return false;
    }

    float airspeedCmS = 0.0f;
    if (!hasTrustedPitotAirspeed(&airspeedCmS)) {
        return true;
    }

    return airspeedCmS < thresholdCmS;
}

static bool shouldRequestManualFwToMcProtection(const bool manualControllerEnabled)
{
    if (!manualControllerEnabled || !STATE(AIRPLANE)) {
        return false;
    }

    const uint16_t thresholdCmS = systemConfig()->vtolFwToMcAutoSwitchAirspeed;
    if (thresholdCmS == 0 || !hasPitotSensorForManualProtection()) {
        return false;
    }

    float airspeedCmS = 0.0f;
    if (!hasTrustedPitotAirspeed(&airspeedCmS)) {
        return false;
    }

    return airspeedCmS <= thresholdCmS;
}

static void updateTransitionScales(void)
{
    if (!currentMixerConfig.vtolTransitionDynamicMixer) {
        mixerProfileAT.blendToFw = 1.0f;
        mixerProfileAT.pusherScale = 1.0f;
        mixerProfileAT.liftScale = 1.0f;
        mixerProfileAT.mcAuthorityScale = 1.0f;
        mixerProfileAT.fwAuthorityScale = 1.0f;
        return;
    }

    const float liftFloor = constrainf(systemConfig()->vtolTransitionLiftEndPercent / 100.0f, 0.0f, 1.0f);
    const float mcFloor = constrainf(systemConfig()->vtolTransitionMcAuthorityEndPercent / 100.0f, 0.0f, 1.0f);
    const float fwFloor = constrainf(systemConfig()->vtolTransitionFwAuthorityStartPercent / 100.0f, 0.0f, 1.0f);
    const float scaleProgress = getScalingProgress();

    if (mixerProfileAT.direction == MIXERAT_DIRECTION_TO_FW) {
        mixerProfileAT.pusherScale = blendScale(0.0f, 1.0f, scaleProgress);
        mixerProfileAT.liftScale = blendScale(1.0f, liftFloor, scaleProgress);
        mixerProfileAT.mcAuthorityScale = blendScale(1.0f, mcFloor, scaleProgress);
        mixerProfileAT.fwAuthorityScale = blendScale(fwFloor, 1.0f, scaleProgress);
    } else if (mixerProfileAT.direction == MIXERAT_DIRECTION_TO_MC) {
        mixerProfileAT.pusherScale = blendScale(1.0f, 0.0f, scaleProgress);
        mixerProfileAT.liftScale = blendScale(liftFloor, 1.0f, scaleProgress);
        mixerProfileAT.mcAuthorityScale = blendScale(mcFloor, 1.0f, scaleProgress);
        mixerProfileAT.fwAuthorityScale = blendScale(1.0f, fwFloor, scaleProgress);
    }

    mixerProfileAT.blendToFw = constrainf(mixerProfileAT.fwAuthorityScale, 0.0f, 1.0f);
}

static void abortTransition(const bool byAirspeedTimeout)
{
    const bool wasActive = mixerProfileAT.phase != MIXERAT_PHASE_IDLE;
    isMixerTransitionMixing_requested = false;
    mixerProfileAT.phase = MIXERAT_PHASE_IDLE;
    mixerProfileAT.aborted = wasActive;
    mixerProfileAT.abortedByAirspeedTimeout = wasActive && byAirspeedTimeout;
    mixerProfileAT.hotSwitchDone = false;
    mixerProfileAT.request = MIXERAT_REQUEST_NONE;
    mixerProfileAT.direction = MIXERAT_DIRECTION_NONE;
    mixerProfileAT.usedAirspeed = false;
    mixerProfileAT.transitionStartAirspeedCaptured = false;
    mixerProfileAT.transitionStartAirspeedCmS = 0.0f;
    resetTransitionScales();
}

static bool mixerATReadyForHotSwitch(const mixerProfileATRequest_e required_action)
{
    const mixerProfileATDirection_e direction = directionForRequest(required_action);
    const uint16_t airspeedThresholdCmS = getAirspeedThresholdForDirection(direction);
    const uint32_t elapsedMs = millis() - mixerProfileAT.transitionStartTime;
    const uint32_t transitionTimerMs = MAX(0, currentMixerConfig.switchTransitionTimer) * 100;
    float airspeedCmS = 0.0f;

    if (direction == MIXERAT_DIRECTION_NONE) {
        mixerProfileAT.progress = 0.0f;
        mixerProfileAT.usedAirspeed = false;
        return false;
    }

    if (airspeedThresholdCmS > 0 && hasTrustedPitotAirspeed(&airspeedCmS)) {
        mixerProfileAT.usedAirspeed = true;
        if (direction == MIXERAT_DIRECTION_TO_FW) {
            mixerProfileAT.progress = constrainf(airspeedCmS / airspeedThresholdCmS, 0.0f, 1.0f);
            return airspeedCmS >= airspeedThresholdCmS;
        }

        if (!mixerProfileAT.transitionStartAirspeedCaptured) {
            mixerProfileAT.transitionStartAirspeedCmS = airspeedCmS;
            mixerProfileAT.transitionStartAirspeedCaptured = true;
        }

        const float startAirspeed = mixerProfileAT.transitionStartAirspeedCmS;
        const float thresholdAirspeed = airspeedThresholdCmS;
        if (startAirspeed <= thresholdAirspeed) {
            mixerProfileAT.progress = 1.0f;
        } else {
            mixerProfileAT.progress = constrainf((startAirspeed - airspeedCmS) / (startAirspeed - thresholdAirspeed), 0.0f, 1.0f);
        }

        return airspeedCmS <= airspeedThresholdCmS;
    }

    mixerProfileAT.usedAirspeed = false;
    if (transitionTimerMs > 0) {
        mixerProfileAT.progress = constrainf((float)elapsedMs / (float)transitionTimerMs, 0.0f, 1.0f);
    } else {
        mixerProfileAT.progress = 1.0f;
    }

    return elapsedMs >= transitionTimerMs;
}

bool platformTypeConfigured(flyingPlatformType_e platformType)
{   
    if (!isModeActivationConditionPresent(BOXMIXERPROFILE)){
        return false;
    }
    return mixerConfigByIndex(nextMixerProfileIndex)->platformType == platformType;
}

static bool missionTransitionToMultirotorTypeConfigured(void)
{
    if (!isModeActivationConditionPresent(BOXMIXERPROFILE)) {
        return false;
    }

    const flyingPlatformType_e nextPlatformType = mixerConfigByIndex(nextMixerProfileIndex)->platformType;
    return nextPlatformType == PLATFORM_MULTIROTOR ||
           nextPlatformType == PLATFORM_TRICOPTER ||
           nextPlatformType == PLATFORM_HELICOPTER;
}

bool checkMixerATRequired(mixerProfileATRequest_e required_action)
{
    //return false if mixerAT condition is not required or setting is not valid
    if ((!STATE(AIRPLANE)) && (!STATE(MULTIROTOR)))
    {
        return false;
    }
    if (!isModeActivationConditionPresent(BOXMIXERPROFILE))
    {
        return false;
    }

    switch (required_action) {
    case MIXERAT_REQUEST_RTH:
        return currentMixerConfig.automated_switch && STATE(MULTIROTOR) && platformTypeConfigured(PLATFORM_AIRPLANE);

    case MIXERAT_REQUEST_LAND:
        return currentMixerConfig.automated_switch && STATE(AIRPLANE) && missionTransitionToMultirotorTypeConfigured();

    case MIXERAT_REQUEST_MISSION_TO_FW:
        return STATE(MULTIROTOR) && platformTypeConfigured(PLATFORM_AIRPLANE);

    case MIXERAT_REQUEST_MISSION_TO_MC:
        return STATE(AIRPLANE) && missionTransitionToMultirotorTypeConfigured();

    case MIXERAT_REQUEST_MANUAL_TO_FW:
        return STATE(MULTIROTOR) && platformTypeConfigured(PLATFORM_AIRPLANE);

    case MIXERAT_REQUEST_MANUAL_TO_MC:
        return STATE(AIRPLANE) && missionTransitionToMultirotorTypeConfigured();

    default:
        return false;
    }
}

bool mixerATUpdateState(mixerProfileATRequest_e required_action)
{   
    //return true if mixerAT is done or not required
    bool reprocessState;
    do
    {   
        reprocessState=false;
        if (required_action == MIXERAT_REQUEST_ABORT) {
            abortTransition(false);
            return true;
        }
        switch (mixerProfileAT.phase) {
        case MIXERAT_PHASE_IDLE:
            //check if mixerAT is required
            if (checkMixerATRequired(required_action)) {
                mixerProfileAT.request = required_action;
                mixerProfileAT.direction = directionForRequest(required_action);
                mixerProfileAT.phase = MIXERAT_PHASE_TRANSITION_INITIALIZE;
                reprocessState = true;
            } else {
                resetTransitionScales();
            }
            break;
        case MIXERAT_PHASE_TRANSITION_INITIALIZE:
            mixerProfileAT.request = required_action;
            mixerProfileAT.direction = directionForRequest(required_action);
            setMixerProfileAT();
            mixerProfileAT.phase = MIXERAT_PHASE_TRANSITIONING;
            reprocessState = true;
            break;
        case MIXERAT_PHASE_TRANSITIONING:
            isMixerTransitionMixing_requested = true;
            if (required_action != MIXERAT_REQUEST_NONE && required_action != mixerProfileAT.request) {
                abortTransition(false);
                return true;
            }

            if (mixerATReadyForHotSwitch(mixerProfileAT.request)) {
                isMixerTransitionMixing_requested = false;
                mixerProfileAT.progress = 1.0f;
                updateTransitionScales();
                if (!outputProfileHotSwitch(nextMixerProfileIndex)) {
                    abortTransition(false);
                    return true;
                }
                mixerProfileAT.hotSwitchDone = true;
                mixerProfileAT.phase = MIXERAT_PHASE_IDLE;
                mixerProfileAT.request = MIXERAT_REQUEST_NONE;
                mixerProfileAT.direction = MIXERAT_DIRECTION_NONE;
                reprocessState = true;
            } else if (mixerProfileAT.usedAirspeed &&
                       currentMixerConfig.vtolTransitionAirspeedTimeoutMs > 0 &&
                       (millis() - mixerProfileAT.transitionStartTime) >= currentMixerConfig.vtolTransitionAirspeedTimeoutMs) {
                abortTransition(true);
                return true;
            }

            updateTransitionScales();
            return false;
            break;
        default:
            break;
        }
    }
    while (reprocessState);
    return true;
}

bool checkMixerProfileHotSwitchAvalibility(void)
{
    if (MAX_MIXER_PROFILE_COUNT != 2)
    {
        return false;
    }
    return true;
}

void outputProfileUpdateTask(timeUs_t currentTimeUs)
{   
    UNUSED(currentTimeUs);
    if (cliMode) {
        return;
    }

    bool mixerAT_inuse = mixerATIsActive();
    const bool transitionModeActive = IS_RC_MODE_ACTIVE(BOXMIXERTRANSITION);
    const bool transitionModeRisingEdge = transitionModeActive && !manualTransitionModeWasActive;
    const bool transitionModeFallingEdge = !transitionModeActive && manualTransitionModeWasActive;
    const bool manualTransitionAllowed = (posControl.navState == NAV_STATE_IDLE) ||
                                         (posControl.navState == NAV_STATE_ALTHOLD_IN_PROGRESS);
    const bool missionActive = (navGetCurrentStateFlags() & NAV_AUTO_WP) != 0;
    const bool manualControllerConfigured = currentMixerConfig.manualVtolTransitionController && !missionActive;
    bool manualControllerEnabled = manualControllerConfigured || manualTransitionSessionLatched;
    const bool mixerProfileModePresent = isModeActivationConditionPresent(BOXMIXERPROFILE);
    const int requestedProfileIndex = IS_RC_MODE_ACTIVE(BOXMIXERPROFILE) == 0 ? 0 : 1;

    if (manualControllerConfigured && transitionModeRisingEdge) {
        manualTransitionSessionLatched = true;
    }

    if (transitionModeFallingEdge) {
        manualTransitionSessionLatched = false;
    }

    if (mixerAT_inuse && (!ARMING_FLAG(ARMED) || FLIGHT_MODE(FAILSAFE_MODE) || areSensorsCalibrating())) {
        abortTransition(false);
        manualTransitionSessionLatched = false;
        mixerAT_inuse = false;
    }

    // For manual auto-transition control, suppress direct profile hotswitch while transition trigger is active.
    const bool suppressDirectProfileSwitch = manualControllerEnabled && transitionModeActive;
    if (!FLIGHT_MODE(FAILSAFE_MODE) && !mixerAT_inuse && !suppressDirectProfileSwitch)
    {
        if (mixerProfileModePresent &&
            !shouldBlockManualDirectSwitchToFixedWing(manualControllerEnabled, requestedProfileIndex)) {
            outputProfileHotSwitch(requestedProfileIndex);
        }
    }

    // Recompute after a potential direct profile hot-switch because this flag is per-mixer-profile.
    manualControllerEnabled = (currentMixerConfig.manualVtolTransitionController && !missionActive) || manualTransitionSessionLatched;

    if (!mixerAT_inuse &&
        shouldRequestManualFwToMcProtection(manualControllerEnabled) &&
        checkMixerATRequired(MIXERAT_REQUEST_MANUAL_TO_MC)) {
        mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
        mixerAT_inuse = mixerATIsActive();
    }

    if (!manualControllerEnabled) {
        // Backward-compatible manual path: level-controlled transition mixing request.
        if (!FLIGHT_MODE(FAILSAFE_MODE) && (!mixerAT_inuse)) {
            isMixerTransitionMixing_requested = transitionModeActive;
            if (isMixerTransitionMixing_requested) {
                setLegacyTransitionScales();
            }
        }
        manualTransitionReadyForEdge = true;
    } else {
        if (!transitionModeActive) {
            manualTransitionSessionLatched = false;
            manualTransitionReadyForEdge = true;
            if (!mixerAT_inuse) {
                isMixerTransitionMixing_requested = false;
            }
        } else if (transitionModeRisingEdge && manualTransitionReadyForEdge && manualTransitionAllowed && !mixerAT_inuse) {
            manualTransitionReadyForEdge = false;
            if (STATE(MULTIROTOR)) {
                mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_FW);
            } else if (STATE(AIRPLANE)) {
                mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
            }
            mixerAT_inuse = mixerATIsActive();
        }

        if (!transitionModeActive &&
            mixerAT_inuse &&
            !mixerProfileAT.hotSwitchDone &&
            (mixerProfileAT.request == MIXERAT_REQUEST_MANUAL_TO_FW || mixerProfileAT.request == MIXERAT_REQUEST_MANUAL_TO_MC)) {
            abortTransition(false);
            mixerAT_inuse = false;
        }

        if (mixerAT_inuse &&
            (mixerProfileAT.request == MIXERAT_REQUEST_MANUAL_TO_FW || mixerProfileAT.request == MIXERAT_REQUEST_MANUAL_TO_MC)) {
            mixerATUpdateState(mixerProfileAT.request);
            mixerAT_inuse = mixerATIsActive();
        }
    }

    manualTransitionModeWasActive = transitionModeActive;

    isMixerTransitionMixing = isMixerTransitionMixing_requested &&
        ((posControl.navState == NAV_STATE_IDLE) || mixerAT_inuse || (posControl.navState == NAV_STATE_ALTHOLD_IN_PROGRESS));

    const uint32_t transitionDebugFlags =
        ((uint32_t)mixerProfileAT.direction & 0x3U) |
        (mixerATIsActive() ? 1U << 2 : 0U) |
        (isMixerTransitionMixing ? 1U << 3 : 0U) |
        (transitionModeActive ? 1U << 4 : 0U) |
        (mixerProfileAT.usedAirspeed ? 1U << 5 : 0U) |
        (mixerProfileAT.hotSwitchDone ? 1U << 6 : 0U) |
        (mixerProfileAT.aborted ? 1U << 7 : 0U) |
        (currentMixerConfig.manualVtolTransitionController ? 1U << 8 : 0U) |
        (currentMixerConfig.vtolTransitionDynamicMixer ? 1U << 9 : 0U) |
        (((uint32_t)currentMixerProfileIndex & 0x3U) << 10) |
        (((uint32_t)nextMixerProfileIndex & 0x3U) << 12) |
        (manualTransitionAllowed ? 1U << 14 : 0U) |
        (missionActive ? 1U << 15 : 0U) |
        (isMixerTransitionMixing_requested ? 1U << 16 : 0U) |
        (FLIGHT_MODE(FAILSAFE_MODE) ? 1U << 17 : 0U) |
        (manualControllerEnabled ? 1U << 18 : 0U) |
        (IS_RC_MODE_ACTIVE(BOXMIXERPROFILE) ? 1U << 19 : 0U) |
        (manualTransitionSessionLatched ? 1U << 20 : 0U);

    // VTOL transition debug channels (DEBUG_VTOL_TRANSITION):
    // [0] phase, [1] request, [2] packed transition flags, [3] progress x1000,
    // [4] pusherScale x1000, [5] liftScale x1000, [6] mcAuthorityScale x1000,
    // [7] transition_PID_mmix_multiplier_pitch from currentMixerConfig
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 0, mixerProfileAT.phase);
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 1, mixerProfileAT.request);
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 2, (int32_t)transitionDebugFlags);
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 3, lrintf(constrainf(mixerProfileAT.progress, 0.0f, 1.0f) * 1000.0f));
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 4, lrintf(constrainf(mixerProfileAT.pusherScale, 0.0f, 1.0f) * 1000.0f));
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 5, lrintf(constrainf(mixerProfileAT.liftScale, 0.0f, 1.0f) * 1000.0f));
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 6, lrintf(constrainf(mixerProfileAT.mcAuthorityScale, 0.0f, 1.0f) * 1000.0f));
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 7, currentMixerConfig.transition_PID_mmix_multiplier_pitch);

    if (!isMixerTransitionMixing) {
        resetTransitionScales();
    }
}

bool mixerATIsActive(void)
{
    return mixerProfileAT.phase != MIXERAT_PHASE_IDLE;
}

bool mixerATWasAborted(void)
{
    return mixerProfileAT.aborted;
}

bool mixerATWasAbortedByAirspeedTimeout(void)
{
    return mixerProfileAT.abortedByAirspeedTimeout;
}

float mixerATGetPusherScale(void)
{
    return constrainf(mixerProfileAT.pusherScale, 0.0f, 1.0f);
}

float mixerATGetLiftScale(void)
{
    return constrainf(mixerProfileAT.liftScale, 0.0f, 1.0f);
}

float mixerATGetMcAuthorityScale(void)
{
    return constrainf(mixerProfileAT.mcAuthorityScale, 0.0f, 1.0f);
}

float mixerATGetFwAuthorityScale(void)
{
    return constrainf(mixerProfileAT.fwAuthorityScale, 0.0f, 1.0f);
}

float mixerATGetBlendToFw(void)
{
    return constrainf(mixerProfileAT.blendToFw, 0.0f, 1.0f);
}

bool isMixerProfile2ModeReportedActive(void)
{
#if (MAX_MIXER_PROFILE_COUNT > 1)
    return currentMixerProfileIndex > 0;
#else
    return false;
#endif
}

bool isMixerTransitionModeReportedActive(void)
{
    // Transition is actively running in the internal controller.
    if (mixerATIsActive()) {
        return true;
    }

    // With manual auto-transition enabled (or session latched), treat RC as trigger only.
    if (currentMixerConfig.manualVtolTransitionController || manualTransitionSessionLatched) {
        return false;
    }

    return IS_RC_MODE_ACTIVE(BOXMIXERTRANSITION);
}

// switch mixerprofile without reboot
bool outputProfileHotSwitch(int profile_index)
{
    static bool allow_hot_switch = true;
    // LOG_INFO(PWM, "OutputProfileHotSwitch");
    if (!allow_hot_switch)
    {
        return false;
    }
    if (currentMixerProfileIndex == profile_index)
    {
        return false;
    }
    if (profile_index < 0 || profile_index >= MAX_MIXER_PROFILE_COUNT)
    { // sanity check
        // LOG_INFO(PWM, "invalid mixer profile index");
        return false;
    }
    if (areSensorsCalibrating())
    { // it seems like switching before sensors calibration complete will cause pid stops to respond, especially in D
        return false;
    }
    if (!checkMixerProfileHotSwitchAvalibility())
    {
        // LOG_INFO(PWM, "mixer switch failed, checkMixerProfileHotSwitchAvalibility");
        return false;
    }
    if  ((posControl.navState != NAV_STATE_IDLE) && (posControl.navState != NAV_STATE_MIXERAT_IN_PROGRESS))
    {
        // LOG_INFO(PWM, "mixer switch failed, navState != NAV_STATE_IDLE");
        return false;
    }
    if (!setConfigMixerProfile(profile_index))
    {
        // LOG_INFO(PWM, "mixer switch failed to set config");
        return false;
    }
    mixerConfigInit();
    return true;
}

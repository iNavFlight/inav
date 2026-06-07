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
#ifdef USE_AUTO_TRANSITION
static bool manualTransitionModeWasActive;
static bool manualTransitionReadyForEdge = true;
static bool manualTransitionSessionLatched;
static bool manualFwToMcProtectionLatched;
#endif

// Keep PG version split because USE_AUTO_TRANSITION changes the stored mixer profile layout only on >512 KB targets.
#ifdef USE_AUTO_TRANSITION
PG_REGISTER_ARRAY_WITH_RESET_FN(mixerProfile_t, MAX_MIXER_PROFILE_COUNT, mixerProfiles, PG_MIXER_PROFILE, 4);
#else
PG_REGISTER_ARRAY_WITH_RESET_FN(mixerProfile_t, MAX_MIXER_PROFILE_COUNT, mixerProfiles, PG_MIXER_PROFILE, 1);
#endif

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
#ifdef USE_AUTO_TRANSITION
                         .vtolTransitionDynamicMixer = SETTING_MIXER_VTOL_TRANSITION_DYNAMIC_MIXER_DEFAULT,
                         .manualVtolTransitionController = SETTING_MIXER_VTOL_MANUALSWITCH_AUTOTRANSITION_CONTROLLER_DEFAULT,
                         .vtolTransitionAirspeedTimeoutMs = SETTING_MIXER_VTOL_TRANSITION_AIRSPEED_TIMEOUT_MS_DEFAULT,
                         .vtolTransitionScaleRampTimeMs = SETTING_MIXER_VTOL_TRANSITION_SCALE_RAMP_TIME_MS_DEFAULT,
#endif
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
#ifdef USE_AUTO_TRANSITION
    const timeMs_t now = millis();

    mixerProfileAT.transitionStartTime = now;
    mixerProfileAT.aborted = false;
    mixerProfileAT.abortedByAirspeedTimeout = false;
    mixerProfileAT.hotSwitchDone = false;
    mixerProfileAT.usedAirspeed = false;
    mixerProfileAT.transitionStartAirspeedCaptured = false;
    mixerProfileAT.progress = 0.0f;
    mixerProfileAT.handoffScalingProgress = 0.0f;
    mixerProfileAT.transitionStartAirspeedCmS = 0.0f;
    mixerProfileAT.blendToFw = mixerProfileAT.direction == MIXERAT_DIRECTION_TO_FW ? 0.0f : 1.0f;
    mixerProfileAT.pusherScale = 1.0f;
    mixerProfileAT.liftScale = 1.0f;
    mixerProfileAT.mcAuthorityScale = 1.0f;
    mixerProfileAT.fwAuthorityScale = 1.0f;
#else
    mixerProfileAT.transitionStartTime = millis();
    mixerProfileAT.transitionTransEndTime = mixerProfileAT.transitionStartTime + (timeMs_t)currentMixerConfig.switchTransitionTimer * 100;
#endif
}

#ifdef USE_AUTO_TRANSITION
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
    mixerProfileAT.handoffScalingProgress = 0.0f;
    mixerProfileAT.blendToFw = 0.0f;
    mixerProfileAT.pusherScale = 0.0f;
    mixerProfileAT.liftScale = 1.0f;
    mixerProfileAT.mcAuthorityScale = 1.0f;
    mixerProfileAT.fwAuthorityScale = 1.0f;
}

static void setLegacyTransitionScales(void)
{
    mixerProfileAT.progress = 1.0f;
    mixerProfileAT.handoffScalingProgress = 1.0f;
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

static float getMotorRampProgress(void)
{
    if (!currentMixerConfig.vtolTransitionDynamicMixer) {
        return 1.0f;
    }

    if (currentMixerConfig.vtolTransitionScaleRampTimeMs <= 0) {
        return 1.0f;
    }

    const uint32_t elapsedMs = millis() - mixerProfileAT.transitionStartTime;
    return constrainf((float)elapsedMs / (float)currentMixerConfig.vtolTransitionScaleRampTimeMs, 0.0f, 1.0f);
}

static float getHandoffScalingProgress(void)
{
    if (!currentMixerConfig.vtolTransitionDynamicMixer) {
        return 1.0f;
    }

    if (mixerProfileAT.usedAirspeed) {
        mixerProfileAT.handoffScalingProgress = constrainf(mixerProfileAT.progress, 0.0f, 1.0f);
        return mixerProfileAT.handoffScalingProgress;
    }

    // Preserve already-applied handoff scaling if pitot drops out mid-transition.
    // Without trusted pitot, handoff returns to the normal transition timer/progress behavior.
    mixerProfileAT.handoffScalingProgress = MAX(mixerProfileAT.handoffScalingProgress, constrainf(mixerProfileAT.progress, 0.0f, 1.0f));
    return mixerProfileAT.handoffScalingProgress;
}

static bool hasTrustedPitotAirspeed(float *airspeedCmS)
{
#ifdef USE_PITOT
    if (!sensors(SENSOR_PITOT) || !pitotGetValidForAirspeed() || pitotHasFailed()) {
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

    const float liftFloor = constrainf(systemConfig()->vtolTransitionLiftMinPercent / 100.0f, 0.0f, 1.0f);
    const float mcFloor = constrainf(systemConfig()->vtolTransitionMcAuthorityMinPercent / 100.0f, 0.0f, 1.0f);
    const float fwFloor = constrainf(systemConfig()->vtolTransitionFwAuthorityMinPercent / 100.0f, 0.0f, 1.0f);
    const float handoffProgress = getHandoffScalingProgress();

    if (mixerProfileAT.direction == MIXERAT_DIRECTION_TO_FW) {
        const float pusherProgress = getMotorRampProgress();

        mixerProfileAT.pusherScale = blendScale(0.0f, 1.0f, pusherProgress);
        mixerProfileAT.liftScale = blendScale(1.0f, liftFloor, handoffProgress);
        mixerProfileAT.mcAuthorityScale = blendScale(1.0f, mcFloor, handoffProgress);
        mixerProfileAT.fwAuthorityScale = blendScale(fwFloor, 1.0f, handoffProgress);
    } else if (mixerProfileAT.direction == MIXERAT_DIRECTION_TO_MC) {
        const float liftRampProgress = getMotorRampProgress();

        mixerProfileAT.pusherScale = blendScale(1.0f, 0.0f, handoffProgress);
        mixerProfileAT.liftScale = blendScale(liftFloor, 1.0f, liftRampProgress);
        mixerProfileAT.mcAuthorityScale = blendScale(mcFloor, 1.0f, handoffProgress);
        mixerProfileAT.fwAuthorityScale = blendScale(1.0f, fwFloor, handoffProgress);
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
#endif

bool platformTypeConfigured(flyingPlatformType_e platformType)
{   
    if (!isModeActivationConditionPresent(BOXMIXERPROFILE)){
        return false;
    }
    return mixerConfigByIndex(nextMixerProfileIndex)->platformType == platformType;
}

#ifdef USE_AUTO_TRANSITION
static bool missionTransitionToMultirotorTypeConfigured(void)
{
    if (!isModeActivationConditionPresent(BOXMIXERPROFILE)) {
        return false;
    }

    const flyingPlatformType_e nextPlatformType = mixerConfigByIndex(nextMixerProfileIndex)->platformType;
    return isMultirotorTypePlatform(nextPlatformType);
}
#endif

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

#ifdef USE_AUTO_TRANSITION
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
#else
    if(currentMixerConfig.automated_switch){
        if ((required_action == MIXERAT_REQUEST_RTH) && STATE(MULTIROTOR))
        {
            return true;
        }
        if ((required_action == MIXERAT_REQUEST_LAND) && STATE(AIRPLANE))
        {
            return true;
        }
    }
    return false;
#endif
}

bool mixerATUpdateState(mixerProfileATRequest_e required_action)
{   
#ifdef USE_AUTO_TRANSITION
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
#else
    //return true if mixerAT is done or not required
    bool reprocessState;
    do
    {
        reprocessState=false;
        if (required_action==MIXERAT_REQUEST_ABORT){
            isMixerTransitionMixing_requested = false;
            mixerProfileAT.phase = MIXERAT_PHASE_IDLE;
            return true;
        }
        switch (mixerProfileAT.phase){
        case MIXERAT_PHASE_IDLE:
            //check if mixerAT is required
            if (checkMixerATRequired(required_action)){
                mixerProfileAT.phase=MIXERAT_PHASE_TRANSITION_INITIALIZE;
                reprocessState = true;
            }
            break;
        case MIXERAT_PHASE_TRANSITION_INITIALIZE:
            setMixerProfileAT();
            mixerProfileAT.phase = MIXERAT_PHASE_TRANSITIONING;
            reprocessState = true;
            break;
        case MIXERAT_PHASE_TRANSITIONING:
            isMixerTransitionMixing_requested = true;
            if (millis() > mixerProfileAT.transitionTransEndTime){
                isMixerTransitionMixing_requested = false;
                outputProfileHotSwitch(nextMixerProfileIndex);
                mixerProfileAT.phase = MIXERAT_PHASE_IDLE;
                reprocessState = true;
                //transition is done
            }
            return false;
            break;
        default:
            break;
        }
    }
    while (reprocessState);
    return true;
#endif
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

#ifdef USE_AUTO_TRANSITION
    bool mixerAT_inuse = mixerATIsActive();
    const bool transitionModeActive = IS_RC_MODE_ACTIVE(BOXMIXERTRANSITION);
    const bool transitionModeRisingEdge = transitionModeActive && !manualTransitionModeWasActive;
    const bool transitionModeFallingEdge = !transitionModeActive && manualTransitionModeWasActive;
    const bool manualTransitionAllowed = (posControl.navState == NAV_STATE_IDLE) ||
                                         (posControl.navState == NAV_STATE_ALTHOLD_IN_PROGRESS);
    const bool missionActive = (navGetCurrentStateFlags() & NAV_AUTO_WP) != 0;
    const bool manualControllerConfigured = currentMixerConfig.manualVtolTransitionController && !missionActive;
    bool manualControllerEnabled = manualControllerConfigured || manualTransitionSessionLatched;
    const bool transitionControllerOwnsProfileSwitch = manualControllerEnabled && transitionModeActive;
    const bool mixerProfileModePresent = isModeActivationConditionPresent(BOXMIXERPROFILE);
    const int requestedProfileIndex = IS_RC_MODE_ACTIVE(BOXMIXERPROFILE) == 0 ? 0 : 1;
    const bool requestedMultirotorProfile = mixerProfileModePresent &&
        isMultirotorTypePlatform(mixerConfigByIndex(requestedProfileIndex)->platformType);
    // If low-speed protection already moved the model back to MC, keep direct
    // switching from forcing FW again until the pilot makes a new manual choice.
    const bool fwToMcProtectionOwnsProfileSwitch = manualFwToMcProtectionLatched &&
        STATE(MULTIROTOR) &&
        !requestedMultirotorProfile;

    if (manualControllerConfigured && transitionModeRisingEdge) {
        manualTransitionSessionLatched = true;
    }

    if (transitionModeRisingEdge) {
        manualFwToMcProtectionLatched = false;
    }

    if (transitionModeFallingEdge) {
        manualTransitionSessionLatched = false;
    }

    if (requestedMultirotorProfile || (!mixerAT_inuse && !STATE(MULTIROTOR))) {
        manualFwToMcProtectionLatched = false;
    }

    if (mixerAT_inuse && (!ARMING_FLAG(ARMED) || FLIGHT_MODE(FAILSAFE_MODE) || areSensorsCalibrating())) {
        abortTransition(false);
        manualTransitionSessionLatched = false;
        manualFwToMcProtectionLatched = false;
        mixerAT_inuse = false;
    }

    if (!FLIGHT_MODE(FAILSAFE_MODE) && !mixerAT_inuse)
    {
        if (mixerProfileModePresent && !transitionControllerOwnsProfileSwitch && !fwToMcProtectionOwnsProfileSwitch) {
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
        if (mixerAT_inuse || STATE(MULTIROTOR)) {
            manualFwToMcProtectionLatched = true;
        }
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
#else
    bool mixerAT_inuse = mixerProfileAT.phase != MIXERAT_PHASE_IDLE;
    // transition mode input for servo mix and motor mix
    if (!FLIGHT_MODE(FAILSAFE_MODE) && (!mixerAT_inuse))
    {
        if (isModeActivationConditionPresent(BOXMIXERPROFILE)){
            outputProfileHotSwitch(IS_RC_MODE_ACTIVE(BOXMIXERPROFILE) == 0 ? 0 : 1);
        }
        isMixerTransitionMixing_requested = IS_RC_MODE_ACTIVE(BOXMIXERTRANSITION);
    }
    isMixerTransitionMixing = isMixerTransitionMixing_requested && ((posControl.navState == NAV_STATE_IDLE) || mixerAT_inuse ||(posControl.navState == NAV_STATE_ALTHOLD_IN_PROGRESS));
#endif
}

bool mixerATIsActive(void)
{
    return mixerProfileAT.phase != MIXERAT_PHASE_IDLE;
}

bool mixerATWasAborted(void)
{
#ifdef USE_AUTO_TRANSITION
    return mixerProfileAT.aborted;
#else
    return false;
#endif
}

bool mixerATWasAbortedByAirspeedTimeout(void)
{
#ifdef USE_AUTO_TRANSITION
    return mixerProfileAT.abortedByAirspeedTimeout;
#else
    return false;
#endif
}

float mixerATGetPusherScale(void)
{
#ifdef USE_AUTO_TRANSITION
    return constrainf(mixerProfileAT.pusherScale, 0.0f, 1.0f);
#else
    return 1.0f;
#endif
}

float mixerATGetLiftScale(void)
{
#ifdef USE_AUTO_TRANSITION
    return constrainf(mixerProfileAT.liftScale, 0.0f, 1.0f);
#else
    return 1.0f;
#endif
}

float mixerATGetMcAuthorityScale(void)
{
#ifdef USE_AUTO_TRANSITION
    return constrainf(mixerProfileAT.mcAuthorityScale, 0.0f, 1.0f);
#else
    return 1.0f;
#endif
}

float mixerATGetFwAuthorityScale(void)
{
#ifdef USE_AUTO_TRANSITION
    return constrainf(mixerProfileAT.fwAuthorityScale, 0.0f, 1.0f);
#else
    return 1.0f;
#endif
}

float mixerATGetBlendToFw(void)
{
#ifdef USE_AUTO_TRANSITION
    return constrainf(mixerProfileAT.blendToFw, 0.0f, 1.0f);
#else
    return 1.0f;
#endif
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
#ifdef USE_AUTO_TRANSITION
    // Transition is actively running in the internal controller.
    if (mixerATIsActive()) {
        return true;
    }

    // With manual auto-transition enabled (or session latched), treat RC as trigger only.
    if (currentMixerConfig.manualVtolTransitionController || manualTransitionSessionLatched) {
        return false;
    }

    return IS_RC_MODE_ACTIVE(BOXMIXERTRANSITION);
#else
    return IS_RC_MODE_ACTIVE(BOXMIXERTRANSITION);
#endif
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

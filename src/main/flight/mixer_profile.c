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
#include "common/utils.h"
#include "flight/mixer.h"
#include "flight/mixer_profile.h"
#include "flight/mixer_transition_logic.h"
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

#ifdef USE_AUTO_TRANSITION
#define MIXER_TRANSITION_OSD_EVENT_DISPLAY_MS 3000
#define MIXER_TRANSITION_MC_SPEED_HIGH_MARGIN_CM_S 100.0f
#endif

mixerConfig_t currentMixerConfig;
int currentMixerProfileIndex;
bool isMixerTransitionMixing;
bool isMixerTransitionMixing_requested;
mixerProfileAT_t mixerProfileAT;
int nextMixerProfileIndex;
#ifdef USE_AUTO_TRANSITION
static bool manualTransitionModeWasActive;
static bool manualTransitionReadyForEdge = true;
static mixerTransitionManualSessionMode_e manualTransitionSessionMode;
static bool manualFwToMcProtectionLatched;
static int16_t mixerTransitionServoInput;
static mixerProfileATOsdEvent_e mixerATOsdEvent;
static timeMs_t mixerATOsdEventUntil;
static mixerProfileATDirection_e mixerATOsdSwitchReminderDirection;
static bool navigationProfileSwitchWasOwned;
static bool navigationProfileHandbackPending;
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

    if (currentMixerConfig.controlProfileLinking) {
        setConfigProfile(getConfigMixerProfile());
    }

    // Reinitialize the active controller on every mixer hot-switch so AUTO PID
    // selection follows the new platform type and no stale FW/MC integrator
    // state leaks across the completed transition.
    pidInit();
    pidInitFilters();
    pidResetErrorAccumulators();
    schedulePidGainsUpdate();
    navigationUsePIDs();
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
    mixerProfileAT.waitReason = MIXERAT_WAIT_REASON_NONE;
    mixerProfileAT.transitionStartAirspeedCaptured = false;
    mixerProfileAT.progress = 0.0f;
    mixerProfileAT.handoffScalingProgress = 0.0f;
    mixerProfileAT.motorRampProgress = 0.0f;
    mixerProfileAT.transitionStartAirspeedCmS = 0.0f;
    mixerProfileAT.blendToFw = mixerProfileAT.direction == MIXERAT_DIRECTION_TO_FW ? 0.0f : 1.0f;
    mixerProfileAT.pusherScale = 1.0f;
    mixerProfileAT.liftScale = 1.0f;
    mixerProfileAT.mcAuthorityScale = 1.0f;
    mixerProfileAT.fwAuthorityScale = 1.0f;
    mixerProfileAT.postSwitchFadeProgress = 0.0f;
    mixerProfileAT.postSwitchFadeInitialScale = 0.0f;
    mixerProfileAT.postSwitchFadeMotorMask = 0;
    mixerProfileAT.postSwitchFadeToCurrentMotorMask = 0;
    mixerProfileAT.postSwitchFadeDurationMs = 0;
    mixerProfileAT.postSwitchFadeStartTime = 0;
    memset(mixerProfileAT.postSwitchFadeMotorOutput, 0, sizeof(mixerProfileAT.postSwitchFadeMotorOutput));
    mixerProfileAT.servoHandoffMask = 0;
    mixerProfileAT.servoHandoffDurationMs = 0;
    mixerProfileAT.servoHandoffHoldDurationMs = 0;
    mixerProfileAT.servoHandoffStartTime = 0;
    mixerProfileAT.servoHandoffHoldStartTime = 0;
    memset(mixerProfileAT.servoHandoffOutput, 0, sizeof(mixerProfileAT.servoHandoffOutput));
#else
    mixerProfileAT.transitionStartTime = millis();
    mixerProfileAT.transitionTransEndTime = mixerProfileAT.transitionStartTime + (timeMs_t)currentMixerConfig.switchTransitionTimer * 100;
#endif
}

#ifdef USE_AUTO_TRANSITION
static void setMixerATOsdEvent(const mixerProfileATOsdEvent_e event)
{
    mixerATOsdEvent = event;
    mixerATOsdEventUntil = event == MIXERAT_OSD_EVENT_NONE ? 0 : millis() + MIXER_TRANSITION_OSD_EVENT_DISPLAY_MS;
}

static void updateMixerATSwitchReminder(
    const bool transitionModeActive,
    const int requestedProfileIndex)
{
    mixerATOsdSwitchReminderDirection = mixerTransitionManualSwitchReminderDirection(
        manualTransitionSessionMode,
        mixerATIsActive(),
        mixerProfileAT.hotSwitchDone,
        transitionModeActive,
        currentMixerProfileIndex,
        requestedProfileIndex,
        isMultirotorTypePlatform(currentMixerConfig.platformType));

    if (mixerATOsdSwitchReminderDirection == MIXERAT_DIRECTION_NONE && navigationProfileHandbackPending) {
        mixerATOsdSwitchReminderDirection = isMultirotorTypePlatform(currentMixerConfig.platformType) ?
            MIXERAT_DIRECTION_TO_MC :
            MIXERAT_DIRECTION_TO_FW;
    }
}

static void clearServoHandoffFade(void)
{
    mixerProfileAT.servoHandoffMask = 0;
    mixerProfileAT.servoHandoffDurationMs = 0;
    mixerProfileAT.servoHandoffHoldDurationMs = 0;
    mixerProfileAT.servoHandoffStartTime = 0;
    mixerProfileAT.servoHandoffHoldStartTime = 0;
    memset(mixerProfileAT.servoHandoffOutput, 0, sizeof(mixerProfileAT.servoHandoffOutput));
}

static bool isServoTransitionLinkedInputSource(const uint8_t inputSource)
{
    return inputSource == INPUT_MIXER_TRANSITION ||
           (inputSource >= INPUT_AUTOTRANSITION_TARGET_STABILIZED_ROLL &&
            inputSource <= INPUT_AUTOTRANSITION_TARGET_STABILIZED_YAW_MINUS);
}

static bool isServoAutoTransitionTargetInputSource(const uint8_t inputSource)
{
    return inputSource >= INPUT_AUTOTRANSITION_TARGET_STABILIZED_ROLL &&
           inputSource <= INPUT_AUTOTRANSITION_TARGET_STABILIZED_YAW_MINUS;
}

static uint32_t collectServoTargetMask(const servoMixer_t *rules)
{
    uint32_t mask = 0;

    for (int i = 0; i < MAX_SERVO_RULES; i++) {
        if (rules[i].rate == 0) {
            break;
        }

        if (rules[i].targetChannel < MAX_SUPPORTED_SERVOS) {
            mask |= 1U << rules[i].targetChannel;
        }
    }

    return mask;
}

static uint32_t collectTransitionLinkedServoTargetMask(const servoMixer_t *rules)
{
    uint32_t mask = 0;

    for (int i = 0; i < MAX_SERVO_RULES; i++) {
        if (rules[i].rate == 0) {
            break;
        }

        if (rules[i].targetChannel >= MAX_SUPPORTED_SERVOS ||
            !isServoTransitionLinkedInputSource(rules[i].inputSource)) {
            continue;
        }

        mask |= 1U << rules[i].targetChannel;
    }

    return mask;
}

static uint32_t collectAutoTransitionTargetServoMask(const servoMixer_t *rules)
{
    uint32_t mask = 0;

    for (int i = 0; i < MAX_SERVO_RULES; i++) {
        if (rules[i].rate == 0) {
            break;
        }

        if (rules[i].targetChannel >= MAX_SUPPORTED_SERVOS ||
            !isServoAutoTransitionTargetInputSource(rules[i].inputSource)) {
            continue;
        }

        mask |= 1U << rules[i].targetChannel;
    }

    return mask;
}

static bool servoMixerRuleMatches(const servoMixer_t *lhs, const servoMixer_t *rhs)
{
    return lhs->targetChannel == rhs->targetChannel &&
           lhs->inputSource == rhs->inputSource &&
           lhs->rate == rhs->rate &&
           lhs->speed == rhs->speed
#ifdef USE_PROGRAMMING_FRAMEWORK
           && lhs->conditionId == rhs->conditionId
#endif
        ;
}

static int collectServoRulesForTarget(const servoMixer_t *rules, const uint8_t targetChannel, const servoMixer_t *targetRules[MAX_SERVO_RULES])
{
    int ruleCount = 0;

    for (int i = 0; i < MAX_SERVO_RULES; i++) {
        if (rules[i].rate == 0) {
            break;
        }

        if (rules[i].targetChannel != targetChannel) {
            continue;
        }

        targetRules[ruleCount++] = &rules[i];
    }

    return ruleCount;
}

static bool servoTargetRulesDiffer(const servoMixer_t *currentRules, const servoMixer_t *targetRules, const uint8_t targetChannel)
{
    const servoMixer_t *currentTargetRules[MAX_SERVO_RULES];
    const servoMixer_t *nextTargetRules[MAX_SERVO_RULES];
    const int currentRuleCount = collectServoRulesForTarget(currentRules, targetChannel, currentTargetRules);
    const int nextRuleCount = collectServoRulesForTarget(targetRules, targetChannel, nextTargetRules);

    if (currentRuleCount != nextRuleCount) {
        return true;
    }

    for (int i = 0; i < currentRuleCount; i++) {
        if (!servoMixerRuleMatches(currentTargetRules[i], nextTargetRules[i])) {
            return true;
        }
    }

    return false;
}

static uint32_t collectServoProfileDifferenceMask(const servoMixer_t *currentRules, const servoMixer_t *targetRules)
{
    const uint32_t allTargetsMask = collectServoTargetMask(currentRules) | collectServoTargetMask(targetRules);
    uint32_t differenceMask = 0;

    for (uint8_t i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
        if ((allTargetsMask & (1U << i)) == 0) {
            continue;
        }

        if (servoTargetRulesDiffer(currentRules, targetRules, i)) {
            differenceMask |= 1U << i;
        }
    }

    return differenceMask;
}

static uint16_t getServoHandoffDurationMs(void);

static uint32_t collectServoHandoffMask(const int targetProfileIndex, const bool includeProfileDifferences)
{
    const servoMixer_t *currentRules = mixerServoMixersByIndex(currentMixerProfileIndex);
    uint32_t handoffMask = collectTransitionLinkedServoTargetMask(currentRules);

    if (targetProfileIndex < 0 || targetProfileIndex >= MAX_MIXER_PROFILE_COUNT) {
        return handoffMask;
    }

    const servoMixer_t *targetRules = mixerServoMixersByIndex(targetProfileIndex);

    handoffMask |= collectTransitionLinkedServoTargetMask(targetRules);

    if (includeProfileDifferences) {
        handoffMask |= collectServoProfileDifferenceMask(currentRules, targetRules);
    }

    return handoffMask;
}

static uint32_t collectTransitionEntryServoHandoffMask(const int targetProfileIndex)
{
    const servoMixer_t *currentRules = mixerServoMixersByIndex(currentMixerProfileIndex);
    uint32_t handoffMask = collectAutoTransitionTargetServoMask(currentRules);

    if (targetProfileIndex < 0 || targetProfileIndex >= MAX_MIXER_PROFILE_COUNT) {
        return handoffMask;
    }

    const servoMixer_t *targetRules = mixerServoMixersByIndex(targetProfileIndex);
    handoffMask |= collectAutoTransitionTargetServoMask(targetRules);

    return handoffMask;
}

static void prepareServoHandoffFade(const uint32_t handoffMask)
{
    clearServoHandoffFade();

    const uint16_t handoffDurationMs = getServoHandoffDurationMs();

    if (handoffMask == 0 || handoffDurationMs == 0) {
        return;
    }

    mixerProfileAT.servoHandoffMask = handoffMask;
    mixerProfileAT.servoHandoffDurationMs = handoffDurationMs;

    for (uint8_t i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
        if ((handoffMask & (1U << i)) == 0) {
            continue;
        }

        mixerProfileAT.servoHandoffOutput[i] = servo[i];
    }
}

static void startServoHandoffFade(void)
{
    if (mixerProfileAT.servoHandoffMask == 0 || mixerProfileAT.servoHandoffDurationMs == 0) {
        return;
    }

    mixerProfileAT.servoHandoffHoldDurationMs = 0;
    mixerProfileAT.servoHandoffHoldStartTime = 0;
    mixerProfileAT.servoHandoffStartTime = millis();
}

static void startTransitionEntryServoHandoffFade(void)
{
    // Smooth only target-preview surfaces at transition entry. Tilt servos that
    // use INPUT_MIXER_TRANSITION already have their own transition trajectory.
    prepareServoHandoffFade(collectTransitionEntryServoHandoffMask(nextMixerProfileIndex));
    startServoHandoffFade();
}

static bool outputProfileDirectSwitch(const int profileIndex)
{
    clearServoHandoffFade();
    const bool switched = outputProfileHotSwitch(profileIndex);
    clearServoHandoffFade();
    return switched;
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
        required_action == MIXERAT_REQUEST_MANUAL_TO_MC ||
        required_action == MIXERAT_REQUEST_FW_TO_MC_PROTECTION) {
        return MIXERAT_DIRECTION_TO_MC;
    }

    return MIXERAT_DIRECTION_NONE;
}

static void resetTransitionScales(void)
{
    mixerProfileAT.waitReason = MIXERAT_WAIT_REASON_NONE;
    mixerProfileAT.progress = 0.0f;
    mixerProfileAT.handoffScalingProgress = 0.0f;
    mixerProfileAT.motorRampProgress = 0.0f;
    mixerProfileAT.blendToFw = 0.0f;
    mixerProfileAT.pusherScale = 0.0f;
    mixerProfileAT.liftScale = 1.0f;
    mixerProfileAT.mcAuthorityScale = 1.0f;
    mixerProfileAT.fwAuthorityScale = 1.0f;
    mixerProfileAT.postSwitchFadeProgress = 0.0f;
    mixerProfileAT.postSwitchFadeInitialScale = 0.0f;
    mixerProfileAT.postSwitchFadeMotorMask = 0;
    mixerProfileAT.postSwitchFadeToCurrentMotorMask = 0;
    mixerProfileAT.postSwitchFadeDurationMs = 0;
    mixerProfileAT.postSwitchFadeStartTime = 0;
    memset(mixerProfileAT.postSwitchFadeMotorOutput, 0, sizeof(mixerProfileAT.postSwitchFadeMotorOutput));
}

static void setLegacyTransitionScales(void)
{
    mixerProfileAT.progress = 1.0f;
    mixerProfileAT.handoffScalingProgress = 1.0f;
    mixerProfileAT.motorRampProgress = 1.0f;
    mixerProfileAT.blendToFw = 1.0f;
    mixerProfileAT.pusherScale = 1.0f;
    mixerProfileAT.liftScale = 1.0f;
    mixerProfileAT.mcAuthorityScale = 1.0f;
    mixerProfileAT.fwAuthorityScale = 1.0f;
    mixerProfileAT.postSwitchFadeProgress = 1.0f;
    mixerProfileAT.postSwitchFadeInitialScale = 0.0f;
    mixerProfileAT.postSwitchFadeMotorMask = 0;
    mixerProfileAT.postSwitchFadeToCurrentMotorMask = 0;
    mixerProfileAT.postSwitchFadeDurationMs = 0;
    mixerProfileAT.postSwitchFadeStartTime = 0;
    memset(mixerProfileAT.postSwitchFadeMotorOutput, 0, sizeof(mixerProfileAT.postSwitchFadeMotorOutput));
}

static float getMotorRampProgress(void)
{
    const uint32_t elapsedMs = millis() - mixerProfileAT.transitionStartTime;
    mixerProfileAT.motorRampProgress = mixerTransitionComputeMotorRampProgress(
        currentMixerConfig.vtolTransitionDynamicMixer,
        currentMixerConfig.vtolTransitionScaleRampTimeMs,
        elapsedMs);
    return mixerProfileAT.motorRampProgress;
}

static uint16_t getServoHandoffDurationMs(void)
{
    const uint32_t elapsedMs = millis() - mixerProfileAT.transitionStartTime;

    return mixerTransitionComputeServoHandoffDurationMs(
        currentMixerConfig.vtolTransitionDynamicMixer,
        currentMixerConfig.vtolTransitionScaleRampTimeMs,
        elapsedMs);
}

static float getHandoffScalingProgress(void)
{
    mixerProfileAT.handoffScalingProgress = mixerTransitionResolveHandoffProgress(
        currentMixerConfig.vtolTransitionDynamicMixer,
        mixerProfileAT.usedAirspeed,
        mixerProfileAT.handoffScalingProgress,
        mixerProfileAT.progress);
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

static void preparePostSwitchFade(const int targetProfileIndex)
{
    mixerProfileAT.postSwitchFadeMotorMask = 0;
    mixerProfileAT.postSwitchFadeToCurrentMotorMask = 0;
    mixerProfileAT.postSwitchFadeProgress = 0.0f;
    mixerProfileAT.postSwitchFadeInitialScale = 0.0f;
    mixerProfileAT.postSwitchFadeDurationMs = 0;
    mixerProfileAT.postSwitchFadeStartTime = 0;
    memset(mixerProfileAT.postSwitchFadeMotorOutput, 0, sizeof(mixerProfileAT.postSwitchFadeMotorOutput));

    if (!currentMixerConfig.vtolTransitionDynamicMixer ||
        currentMixerConfig.vtolTransitionScaleRampTimeMs == 0 ||
        mixerProfileAT.direction == MIXERAT_DIRECTION_NONE ||
        targetProfileIndex < 0 ||
        targetProfileIndex >= MAX_MIXER_PROFILE_COUNT) {
        return;
    }

    const motorMixer_t *currentMotorMixer = mixerMotorMixersByIndex(currentMixerProfileIndex);
    const motorMixer_t *targetMotorMixer = mixerMotorMixersByIndex(targetProfileIndex);
    const bool currentProfileIsMultirotor = isMultirotorTypePlatform(currentMixerConfig.platformType);
    const uint8_t count = getMotorCount();
    const mixerTransitionPostSwitchFadeMask_t fadeMask = mixerTransitionComputePostSwitchFadeMask(
        currentMixerConfig.vtolTransitionDynamicMixer,
        currentMixerConfig.vtolTransitionScaleRampTimeMs,
        mixerProfileAT.direction,
        currentProfileIsMultirotor,
        count,
        currentMotorMixer,
        targetMotorMixer);

    mixerProfileAT.postSwitchFadeMotorMask = fadeMask.motorMask;
    mixerProfileAT.postSwitchFadeToCurrentMotorMask = fadeMask.toCurrentMotorMask;

    for (uint8_t i = 0; i < count && i < MAX_SUPPORTED_MOTORS; i++) {
        if (mixerProfileAT.postSwitchFadeMotorMask & (1U << i)) {
            mixerProfileAT.postSwitchFadeMotorOutput[i] = motor[i];
        }
    }

    if (mixerProfileAT.postSwitchFadeMotorMask == 0) {
        return;
    }

    mixerProfileAT.postSwitchFadeDurationMs = currentMixerConfig.vtolTransitionScaleRampTimeMs;
    mixerProfileAT.postSwitchFadeInitialScale = mixerProfileAT.direction == MIXERAT_DIRECTION_TO_FW ?
        constrainf(mixerProfileAT.liftScale, 0.0f, 1.0f) :
        constrainf(mixerProfileAT.pusherScale, 0.0f, 1.0f);
}

static bool startPostSwitchFade(void)
{
    if (mixerProfileAT.postSwitchFadeMotorMask == 0 ||
        mixerProfileAT.postSwitchFadeDurationMs == 0) {
        return false;
    }

    mixerProfileAT.postSwitchFadeStartTime = millis();
    mixerProfileAT.postSwitchFadeProgress = 0.0f;
    mixerProfileAT.phase = MIXERAT_PHASE_POST_SWITCH_FADE;
    isMixerTransitionMixing_requested = false;
    return true;
}

static bool updatePostSwitchFade(void)
{
    if (mixerProfileAT.phase != MIXERAT_PHASE_POST_SWITCH_FADE) {
        return true;
    }

    if (mixerProfileAT.postSwitchFadeDurationMs == 0) {
        mixerProfileAT.postSwitchFadeProgress = 1.0f;
    } else {
        const uint32_t elapsedMs = millis() - mixerProfileAT.postSwitchFadeStartTime;
        mixerProfileAT.postSwitchFadeProgress = constrainf((float)elapsedMs / (float)mixerProfileAT.postSwitchFadeDurationMs, 0.0f, 1.0f);
    }

    const float remainingScale = mixerProfileAT.postSwitchFadeInitialScale * (1.0f - mixerProfileAT.postSwitchFadeProgress);
    if (mixerProfileAT.direction == MIXERAT_DIRECTION_TO_FW) {
        mixerProfileAT.pusherScale = 1.0f;
        mixerProfileAT.liftScale = remainingScale;
        mixerProfileAT.mcAuthorityScale = 0.0f;
        mixerProfileAT.fwAuthorityScale = 1.0f;
        mixerProfileAT.blendToFw = 1.0f;
    } else if (mixerProfileAT.direction == MIXERAT_DIRECTION_TO_MC) {
        mixerProfileAT.pusherScale = remainingScale;
        mixerProfileAT.liftScale = 1.0f;
        mixerProfileAT.mcAuthorityScale = 1.0f;
        mixerProfileAT.fwAuthorityScale = 0.0f;
        mixerProfileAT.blendToFw = 0.0f;
    }

    if (mixerProfileAT.postSwitchFadeProgress < 1.0f) {
        return false;
    }

    setMixerATOsdEvent(MIXERAT_OSD_EVENT_DONE);
    mixerProfileAT.phase = MIXERAT_PHASE_IDLE;
    mixerProfileAT.request = MIXERAT_REQUEST_NONE;
    mixerProfileAT.direction = MIXERAT_DIRECTION_NONE;
    mixerProfileAT.waitReason = MIXERAT_WAIT_REASON_NONE;
    mixerProfileAT.postSwitchFadeMotorMask = 0;
    mixerProfileAT.postSwitchFadeToCurrentMotorMask = 0;
    return true;
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

    return mixerTransitionFwToMcProtectionTriggered(ARMING_FLAG(ARMED), STATE(AIRPLANE), thresholdCmS, true, airspeedCmS);
}

static void updateTransitionScales(void)
{
    const mixerTransitionScaleState_t scales = mixerTransitionComputeScales(
        currentMixerConfig.vtolTransitionDynamicMixer,
        mixerProfileAT.direction,
        systemConfig()->vtolTransitionLiftMinPercent / 100.0f,
        systemConfig()->vtolTransitionMcAuthorityMinPercent / 100.0f,
        systemConfig()->vtolTransitionFwAuthorityMinPercent / 100.0f,
        getHandoffScalingProgress(),
        getMotorRampProgress());

    mixerProfileAT.blendToFw = scales.blendToFw;
    mixerProfileAT.pusherScale = scales.pusherScale;
    mixerProfileAT.liftScale = scales.liftScale;
    mixerProfileAT.mcAuthorityScale = scales.mcAuthorityScale;
    mixerProfileAT.fwAuthorityScale = scales.fwAuthorityScale;
}

static void abortTransition(const bool byAirspeedTimeout, const bool directSwitchAbort)
{
    const bool wasActive = mixerProfileAT.phase != MIXERAT_PHASE_IDLE;
    const bool hotSwitchAlreadyDone = mixerProfileAT.hotSwitchDone;
    const uint32_t servoHandoffMask = (wasActive && !hotSwitchAlreadyDone && !directSwitchAbort) ?
        collectServoHandoffMask(nextMixerProfileIndex, false) :
        0;

    if (servoHandoffMask != 0) {
        prepareServoHandoffFade(servoHandoffMask);
    }

    if (wasActive) {
        setMixerATOsdEvent(byAirspeedTimeout ? MIXERAT_OSD_EVENT_AIRSPEED_TIMEOUT : MIXERAT_OSD_EVENT_ABORTED);
    }

    isMixerTransitionMixing_requested = false;
    mixerProfileAT.phase = MIXERAT_PHASE_IDLE;
    mixerProfileAT.aborted = wasActive;
    mixerProfileAT.abortedByAirspeedTimeout = wasActive && byAirspeedTimeout;
    mixerProfileAT.hotSwitchDone = false;
    mixerProfileAT.request = MIXERAT_REQUEST_NONE;
    mixerProfileAT.direction = MIXERAT_DIRECTION_NONE;
    mixerProfileAT.usedAirspeed = false;
    mixerProfileAT.waitReason = MIXERAT_WAIT_REASON_NONE;
    mixerProfileAT.transitionStartAirspeedCaptured = false;
    mixerProfileAT.transitionStartAirspeedCmS = 0.0f;
    resetTransitionScales();

    if (servoHandoffMask != 0) {
        startServoHandoffFade();
    }
}

static bool mixerATReadyForHotSwitch(const mixerProfileATRequest_e required_action)
{
    const mixerProfileATDirection_e direction = directionForRequest(required_action);
    const uint16_t airspeedThresholdCmS = getAirspeedThresholdForDirection(direction);
    const uint32_t elapsedMs = millis() - mixerProfileAT.transitionStartTime;
    const uint32_t transitionTimerMs = MAX(0, currentMixerConfig.switchTransitionTimer) * 100;
    float airspeedCmS = 0.0f;
    const bool trustedAirspeedAvailable =
        airspeedThresholdCmS > 0 && hasTrustedPitotAirspeed(&airspeedCmS);
    const mixerTransitionHotSwitchProgress_t hotSwitchProgress = mixerTransitionEvaluateHotSwitch(
        direction,
        airspeedThresholdCmS,
        trustedAirspeedAvailable,
        airspeedCmS,
        mixerProfileAT.transitionStartAirspeedCaptured,
        mixerProfileAT.transitionStartAirspeedCmS,
        elapsedMs,
        transitionTimerMs);

    mixerProfileAT.usedAirspeed = hotSwitchProgress.usedAirspeed;
    mixerProfileAT.transitionStartAirspeedCaptured = hotSwitchProgress.transitionStartAirspeedCaptured;
    mixerProfileAT.transitionStartAirspeedCmS = hotSwitchProgress.transitionStartAirspeedCmS;
    mixerProfileAT.progress = hotSwitchProgress.progress;

    mixerProfileAT.waitReason = MIXERAT_WAIT_REASON_NONE;
    if (!hotSwitchProgress.readyForHotSwitch &&
        direction == MIXERAT_DIRECTION_TO_MC &&
        airspeedThresholdCmS > 0) {
        if (!trustedAirspeedAvailable) {
            mixerProfileAT.waitReason = MIXERAT_WAIT_REASON_NO_SPEED;
        } else if (airspeedCmS > (float)airspeedThresholdCmS + MIXER_TRANSITION_MC_SPEED_HIGH_MARGIN_CM_S) {
            mixerProfileAT.waitReason = MIXERAT_WAIT_REASON_MC_SPEED_HIGH;
        } else {
            mixerProfileAT.waitReason = MIXERAT_WAIT_REASON_MC_SPEED;
        }
    }

    return hotSwitchProgress.readyForHotSwitch;
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

#ifdef USE_AUTO_TRANSITION
static bool isLegacyManualTransitionSessionActive(void)
{
    return manualTransitionSessionMode == MIXER_TRANSITION_MANUAL_SESSION_LEGACY;
}
#endif

bool checkMixerATRequired(mixerProfileATRequest_e required_action)
{
#ifdef USE_AUTO_TRANSITION
    return mixerTransitionIsRequestAllowed(
        required_action,
        STATE(AIRPLANE),
        STATE(MULTIROTOR),
        isModeActivationConditionPresent(BOXMIXERPROFILE),
        currentMixerConfig.automated_switch,
        platformTypeConfigured(PLATFORM_AIRPLANE),
        missionTransitionToMultirotorTypeConfigured());
#else
    // Legacy 512 KB targets keep the original automated-switch behaviour.
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
            if (mixerProfileAT.phase == MIXERAT_PHASE_POST_SWITCH_FADE && mixerProfileAT.hotSwitchDone) {
                // Once the target profile is active, abort cannot safely undo
                // the hot-switch. Keep fading the old propulsion output toward
                // the safe post-switch destination instead of freezing it.
                mixerProfileAT.request = MIXERAT_REQUEST_NONE;
                updatePostSwitchFade();
                return true;
            }
            abortTransition(false, false);
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
            startTransitionEntryServoHandoffFade();
            mixerProfileAT.phase = MIXERAT_PHASE_TRANSITIONING;
            reprocessState = true;
            break;
        case MIXERAT_PHASE_TRANSITIONING:
            isMixerTransitionMixing_requested = true;
            if (required_action != MIXERAT_REQUEST_NONE && required_action != mixerProfileAT.request) {
                abortTransition(false, false);
                return true;
            }

            if (mixerATReadyForHotSwitch(mixerProfileAT.request)) {
                isMixerTransitionMixing_requested = false;
                mixerProfileAT.progress = 1.0f;
                updateTransitionScales();
                prepareServoHandoffFade(collectServoHandoffMask(nextMixerProfileIndex, true));
                preparePostSwitchFade(nextMixerProfileIndex);
                if (!outputProfileHotSwitch(nextMixerProfileIndex)) {
                    abortTransition(false, false);
                    return true;
                }
                mixerProfileAT.hotSwitchDone = true;
                startServoHandoffFade();
                if (!startPostSwitchFade()) {
                    setMixerATOsdEvent(MIXERAT_OSD_EVENT_DONE);
                    mixerProfileAT.phase = MIXERAT_PHASE_IDLE;
                    mixerProfileAT.request = MIXERAT_REQUEST_NONE;
                    mixerProfileAT.direction = MIXERAT_DIRECTION_NONE;
                    mixerProfileAT.waitReason = MIXERAT_WAIT_REASON_NONE;
                }
                return true;
            } else if (mixerProfileAT.usedAirspeed &&
                       currentMixerConfig.vtolTransitionAirspeedTimeoutMs > 0 &&
                       (millis() - mixerProfileAT.transitionStartTime) >= currentMixerConfig.vtolTransitionAirspeedTimeoutMs) {
                abortTransition(true, false);
                return true;
            }

            updateTransitionScales();
            return false;
            break;
        case MIXERAT_PHASE_POST_SWITCH_FADE:
            isMixerTransitionMixing_requested = false;
            updatePostSwitchFade();
            return true;
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
    const navigationFSMStateFlags_t navStateFlags = navGetCurrentStateFlags();
    const bool missionActive = (navStateFlags & NAV_AUTO_WP) != 0;
    const bool manualControllerConfigured = currentMixerConfig.manualVtolTransitionController && !missionActive;
    bool manualControllerEnabled = mixerTransitionManualControllerEnabled(manualControllerConfigured, manualTransitionSessionMode);
    const bool mixerProfileModePresent = isModeActivationConditionPresent(BOXMIXERPROFILE);
    const int requestedProfileIndex = IS_RC_MODE_ACTIVE(BOXMIXERPROFILE) == 0 ? 0 : 1;
    const bool keepCompletedAutoSession = mixerTransitionKeepCompletedAutoSession(
        manualTransitionSessionMode,
        transitionModeFallingEdge,
        mixerProfileAT.hotSwitchDone,
        currentMixerProfileIndex,
        requestedProfileIndex);
    const bool requestedMultirotorProfile = mixerProfileModePresent &&
        isMultirotorTypePlatform(mixerConfigByIndex(requestedProfileIndex)->platformType);
    // If low-speed protection already moved the model back to MC, keep direct
    // switching from forcing FW again until the pilot makes a new manual choice.
    const bool fwToMcProtectionOwnsProfileSwitch = manualFwToMcProtectionLatched &&
        STATE(MULTIROTOR) &&
        !requestedMultirotorProfile;
    const bool vtolProfilePairConfigured =
        (isMultirotorTypePlatform(currentMixerConfig.platformType) && platformTypeConfigured(PLATFORM_AIRPLANE)) ||
        (currentMixerConfig.platformType == PLATFORM_AIRPLANE && missionTransitionToMultirotorTypeConfigured());
    const bool navigationOwnsProfileSwitch = mixerTransitionNavigationOwnsProfileSwitch(
        ARMING_FLAG(ARMED),
        vtolProfilePairConfigured,
        (navStateFlags & NAV_AUTO_WP) != 0,
        (navStateFlags & NAV_AUTO_RTH) != 0,
        (navStateFlags & NAV_CTL_LAND) != 0,
        (navStateFlags & NAV_MIXERAT) != 0);

    manualTransitionSessionMode = mixerTransitionUpdateManualSessionMode(
        manualTransitionSessionMode,
        transitionModeRisingEdge,
        transitionModeFallingEdge && !keepCompletedAutoSession,
        manualControllerConfigured,
        false);

    if (transitionModeRisingEdge) {
        manualFwToMcProtectionLatched = false;
    }

    if (!transitionModeActive &&
        manualTransitionSessionMode == MIXER_TRANSITION_MANUAL_SESSION_AUTO &&
        mixerProfileAT.hotSwitchDone &&
        requestedProfileIndex == currentMixerProfileIndex) {
        manualTransitionSessionMode = MIXER_TRANSITION_MANUAL_SESSION_NONE;
    }

    if (requestedMultirotorProfile || (!mixerAT_inuse && !STATE(MULTIROTOR))) {
        manualFwToMcProtectionLatched = false;
    }

    const bool failsafeShouldAbortTransition = FLIGHT_MODE(FAILSAFE_MODE) &&
        mixerTransitionShouldAbortForFailsafe(
            mixerProfileAT.request,
            mixerProfileAT.phase == MIXERAT_PHASE_POST_SWITCH_FADE,
            mixerProfileAT.hotSwitchDone);

    if (mixerAT_inuse && (!ARMING_FLAG(ARMED) || failsafeShouldAbortTransition || areSensorsCalibrating())) {
        abortTransition(false, false);
        manualTransitionSessionMode = MIXER_TRANSITION_MANUAL_SESSION_NONE;
        manualFwToMcProtectionLatched = false;
        mixerAT_inuse = false;
    }

    if (mixerProfileAT.phase == MIXERAT_PHASE_POST_SWITCH_FADE) {
        mixerATUpdateState(MIXERAT_REQUEST_NONE);
        mixerAT_inuse = mixerATIsActive();
    }

    if (!ARMING_FLAG(ARMED)) {
        navigationProfileSwitchWasOwned = false;
        navigationProfileHandbackPending = false;
    } else if (mixerTransitionNavigationHandbackShouldClear(
                   navigationOwnsProfileSwitch,
                   transitionModeRisingEdge,
                   transitionModeActive,
                   currentMixerProfileIndex,
                   requestedProfileIndex)) {
        navigationProfileHandbackPending = false;
    } else if (mixerTransitionNavigationHandbackShouldHoldProfile(
                   navigationProfileSwitchWasOwned,
                   navigationOwnsProfileSwitch,
                   mixerProfileModePresent,
                   mixerAT_inuse,
                   transitionModeActive,
                   currentMixerProfileIndex,
                   requestedProfileIndex)) {
        navigationProfileHandbackPending = true;
    }
    navigationProfileSwitchWasOwned = navigationOwnsProfileSwitch;

    const bool transitionControllerOwnsProfileSwitch = manualControllerEnabled && transitionModeActive;
    const bool completedAutoSessionOwnsProfileSwitch = mixerTransitionCompletedAutoSessionOwnsProfileSwitch(
        manualTransitionSessionMode,
        mixerProfileAT.hotSwitchDone,
        currentMixerProfileIndex,
        requestedProfileIndex);

    if (!FLIGHT_MODE(FAILSAFE_MODE) && !mixerAT_inuse)
    {
        if (mixerProfileModePresent &&
            !transitionControllerOwnsProfileSwitch &&
            !completedAutoSessionOwnsProfileSwitch &&
            !navigationOwnsProfileSwitch &&
            !navigationProfileHandbackPending &&
            !fwToMcProtectionOwnsProfileSwitch) {
            if (requestedProfileIndex != currentMixerProfileIndex) {
                outputProfileDirectSwitch(requestedProfileIndex);
            }
        }
    }

    // Recompute after a potential direct profile hot-switch because this flag is per-mixer-profile.
    manualControllerEnabled = mixerTransitionManualControllerEnabled(
        currentMixerConfig.manualVtolTransitionController && !missionActive,
        manualTransitionSessionMode);

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
            manualTransitionReadyForEdge = true;
            if (!mixerAT_inuse) {
                isMixerTransitionMixing_requested = false;
            }
        } else if (mixerTransitionShouldClearCompletedAutoMixingRequest(
                       transitionModeActive,
                       transitionModeRisingEdge,
                       mixerAT_inuse,
                       mixerProfileAT.hotSwitchDone)) {
            isMixerTransitionMixing_requested = false;
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
            abortTransition(false, true);
            mixerAT_inuse = false;
            if (!FLIGHT_MODE(FAILSAFE_MODE) &&
                mixerProfileModePresent &&
                !navigationOwnsProfileSwitch &&
                requestedProfileIndex != currentMixerProfileIndex) {
                outputProfileDirectSwitch(requestedProfileIndex);
            }
        }

        if (mixerAT_inuse &&
            (mixerProfileAT.request == MIXERAT_REQUEST_MANUAL_TO_FW || mixerProfileAT.request == MIXERAT_REQUEST_MANUAL_TO_MC)) {
            mixerATUpdateState(mixerProfileAT.request);
            mixerAT_inuse = mixerATIsActive();
        }
    }

    updateMixerATSwitchReminder(transitionModeActive, requestedProfileIndex);

    manualTransitionModeWasActive = transitionModeActive;

    isMixerTransitionMixing = isMixerTransitionMixing_requested &&
        ((posControl.navState == NAV_STATE_IDLE) || mixerAT_inuse || (posControl.navState == NAV_STATE_ALTHOLD_IN_PROGRESS));

    uint32_t transitionDebugFlags =
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
        (manualTransitionSessionMode != MIXER_TRANSITION_MANUAL_SESSION_NONE ? 1U << 20 : 0U) |
        (isLegacyManualTransitionSessionActive() ? 1U << 30 : 0U) |
        (mixerProfileAT.phase == MIXERAT_PHASE_POST_SWITCH_FADE ? 1U << 29 : 0U);

    uint8_t targetInputMode = 0U;
    if (isMixerTransitionMixing && mixerATIsActive()) {
        if (mixerProfileAT.direction == MIXERAT_DIRECTION_TO_FW) {
            targetInputMode = FLIGHT_MODE(MANUAL_MODE) ? 1U : 2U;
        } else if (mixerProfileAT.direction == MIXERAT_DIRECTION_TO_MC) {
            targetInputMode = 3U;
        }
    }
    const uint16_t progressScaled = lrintf(constrainf(mixerProfileAT.progress, 0.0f, 1.0f) * 1000.0f);
    const uint16_t handoffScaled = lrintf(constrainf(mixerProfileAT.handoffScalingProgress, 0.0f, 1.0f) * 1000.0f);
    const uint16_t motorRampScaled = lrintf(constrainf(mixerProfileAT.motorRampProgress, 0.0f, 1.0f) * 1000.0f);
    const uint16_t postFadeScaled = lrintf(constrainf(mixerProfileAT.postSwitchFadeProgress, 0.0f, 1.0f) * 1000.0f);
    const uint16_t pusherScaled = lrintf(constrainf(mixerProfileAT.pusherScale, 0.0f, 1.0f) * 1000.0f);
    const uint16_t liftScaled = lrintf(constrainf(mixerProfileAT.liftScale, 0.0f, 1.0f) * 1000.0f);
    const uint16_t mcAuthorityScaled = lrintf(constrainf(mixerProfileAT.mcAuthorityScale, 0.0f, 1.0f) * 1000.0f);
    const uint16_t fwAuthorityScaled = lrintf(constrainf(mixerProfileAT.fwAuthorityScale, 0.0f, 1.0f) * 1000.0f);
    const uint32_t packedProgress =
        ((uint32_t)MIN(handoffScaled, 1000) & 0x3FFU) |
        (((uint32_t)MIN(motorRampScaled, 1000) & 0x3FFU) << 10) |
        (((uint32_t)MIN(postFadeScaled, 1000) & 0x3FFU) << 20);

    transitionDebugFlags |= (((uint32_t)currentMixerConfig.platformType & 0xFU) << 21);
    transitionDebugFlags |= (((uint32_t)pidIndexGetType(PID_ROLL) & 0x3U) << 25);
    transitionDebugFlags |= (((uint32_t)targetInputMode & 0x3U) << 27);

    // VTOL transition debug channels (DEBUG_VTOL_TRANSITION):
    // [0] phase
    // [1] request | (direction << 8) | (waitReason << 16)
    // [2] packed transition flags | active platform type | active PID type | target input mode
    // [3] raw transition progress x1000
    // [4] pusherScale x1000
    // [5] liftScale x1000
    // [6] mcAuthorityScale x1000 | (fwAuthorityScale x1000 << 16)
    // [7] handoffProgress 10-bit | motorRampProgress 10-bit | postSwitchFadeProgress 10-bit
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 0, mixerProfileAT.phase);
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 1, (int32_t)(((uint32_t)mixerProfileAT.request & 0xFFU) | (((uint32_t)mixerProfileAT.direction & 0xFFU) << 8) | (((uint32_t)mixerProfileAT.waitReason & 0xFFU) << 16)));
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 2, (int32_t)transitionDebugFlags);
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 3, progressScaled);
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 4, pusherScaled);
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 5, liftScaled);
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 6, (int32_t)((uint32_t)mcAuthorityScaled | ((uint32_t)fwAuthorityScaled << 16)));
    DEBUG_SET(DEBUG_VTOL_TRANSITION, 7, (int32_t)packedProgress);

    if (!isMixerTransitionMixing && !mixerATIsActive()) {
        mixerTransitionServoInput = 0;
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

bool NOINLINE mixerATIsActive(void)
{
    return mixerProfileAT.phase != MIXERAT_PHASE_IDLE;
}

#ifdef USE_AUTO_TRANSITION
bool mixerATGetOsdStatus(mixerProfileATOsdStatus_t *status)
{
    if (!status) {
        return false;
    }

    const bool active = mixerATIsActive();
    const bool eventActive = mixerATOsdEvent != MIXERAT_OSD_EVENT_NONE && cmp32(millis(), mixerATOsdEventUntil) < 0;
    const bool switchReminderActive = mixerATOsdSwitchReminderDirection != MIXERAT_DIRECTION_NONE;

    if (!active && !eventActive && !switchReminderActive) {
        return false;
    }

    status->active = active;
    status->phase = mixerProfileAT.phase;
    status->direction = mixerProfileAT.direction;
    status->request = mixerProfileAT.request;
    status->event = eventActive ? mixerATOsdEvent : MIXERAT_OSD_EVENT_NONE;
    status->waitReason = active ? mixerProfileAT.waitReason : MIXERAT_WAIT_REASON_NONE;
    status->switchReminderDirection = switchReminderActive ? mixerATOsdSwitchReminderDirection : MIXERAT_DIRECTION_NONE;

    if (!eventActive) {
        mixerATOsdEvent = MIXERAT_OSD_EVENT_NONE;
        mixerATOsdEventUntil = 0;
    }

    return true;
}
#endif

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

float NOINLINE mixerATGetPusherScale(void)
{
#ifdef USE_AUTO_TRANSITION
    return constrainf(mixerProfileAT.pusherScale, 0.0f, 1.0f);
#else
    return 1.0f;
#endif
}

float NOINLINE mixerATGetLiftScale(void)
{
#ifdef USE_AUTO_TRANSITION
    return constrainf(mixerProfileAT.liftScale, 0.0f, 1.0f);
#else
    return 1.0f;
#endif
}

float NOINLINE mixerATGetMcAuthorityScale(void)
{
#ifdef USE_AUTO_TRANSITION
    return constrainf(mixerProfileAT.mcAuthorityScale, 0.0f, 1.0f);
#else
    return 1.0f;
#endif
}

float NOINLINE mixerATGetFwAuthorityScale(void)
{
#ifdef USE_AUTO_TRANSITION
    return constrainf(mixerProfileAT.fwAuthorityScale, 0.0f, 1.0f);
#else
    return 1.0f;
#endif
}

float NOINLINE mixerATGetBlendToFw(void)
{
#ifdef USE_AUTO_TRANSITION
    return constrainf(mixerProfileAT.blendToFw, 0.0f, 1.0f);
#else
    return 1.0f;
#endif
}

int16_t NOINLINE mixerATGetTransitionServoInput(void)
{
#ifdef USE_AUTO_TRANSITION
    const bool postSwitchFadeToFwActive =
        mixerProfileAT.phase == MIXERAT_PHASE_POST_SWITCH_FADE &&
        mixerProfileAT.direction == MIXERAT_DIRECTION_TO_FW;
    const uint32_t elapsedMs = millis() - mixerProfileAT.transitionStartTime;
    const float servoBlendToFw = mixerTransitionComputeServoBlendToFw(
        isLegacyManualTransitionSessionActive(),
        isMixerTransitionMixing,
        mixerATIsActive(),
        postSwitchFadeToFwActive,
        currentMixerConfig.vtolTransitionDynamicMixer,
        mixerProfileAT.direction,
        currentMixerConfig.vtolTransitionScaleRampTimeMs,
        elapsedMs);

    mixerTransitionServoInput = mixerTransitionUpdateServoInput(
        mixerTransitionServoInput,
        isLegacyManualTransitionSessionActive(),
        isMixerTransitionMixing,
        mixerATIsActive(),
        postSwitchFadeToFwActive,
        mixerProfileAT.direction == MIXERAT_DIRECTION_TO_FW,
        servoBlendToFw);

    return mixerTransitionServoInput;
#else
    return isMixerTransitionMixing ? 500 : 0;
#endif
}

#ifdef USE_AUTO_TRANSITION
bool mixerATGetServoHandoffOutput(uint8_t servoIndex, int16_t currentOutput, int16_t *output)
{
    if (servoIndex >= MAX_SUPPORTED_SERVOS ||
        mixerProfileAT.servoHandoffMask == 0 ||
        (mixerProfileAT.servoHandoffMask & (1U << servoIndex)) == 0) {
        return false;
    }

    const float progress = mixerProfileAT.servoHandoffDurationMs == 0 ?
        1.0f :
        constrainf((float)(millis() - mixerProfileAT.servoHandoffStartTime) / (float)mixerProfileAT.servoHandoffDurationMs, 0.0f, 1.0f);

    *output = mixerTransitionBlendCapturedServoOutput(
        mixerProfileAT.servoHandoffOutput[servoIndex],
        currentOutput,
        progress);

    if (progress >= 1.0f) {
        clearServoHandoffFade();
    }

    return true;
}

bool NOINLINE mixerATGetPostSwitchFadeMotorOutput(uint8_t motorIndex, int16_t idleOutput, int16_t currentOutput, int16_t *output)
{
    if (mixerProfileAT.phase != MIXERAT_PHASE_POST_SWITCH_FADE ||
        motorIndex >= MAX_SUPPORTED_MOTORS ||
        (mixerProfileAT.postSwitchFadeMotorMask & (1U << motorIndex)) == 0) {
        return false;
    }

    const int16_t capturedOutput = mixerProfileAT.postSwitchFadeMotorOutput[motorIndex];
    const bool fadeToCurrentOutput = (mixerProfileAT.postSwitchFadeToCurrentMotorMask & (1U << motorIndex)) != 0;
    const int16_t targetOutput = fadeToCurrentOutput ? currentOutput : idleOutput;

    *output = mixerTransitionBlendCapturedMotorOutput(
        capturedOutput,
        targetOutput,
        mixerProfileAT.postSwitchFadeProgress);
    return true;
}

float mixerATGetPostSwitchFadeProgress(void)
{
    return constrainf(mixerProfileAT.postSwitchFadeProgress, 0.0f, 1.0f);
}
#endif

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
    if (mixerTransitionManualControllerEnabled(currentMixerConfig.manualVtolTransitionController, manualTransitionSessionMode)) {
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

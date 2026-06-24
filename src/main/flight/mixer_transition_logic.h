#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "flight/mixer_profile.h"

typedef enum {
    MIXER_TRANSITION_MANUAL_SESSION_NONE = 0,
    MIXER_TRANSITION_MANUAL_SESSION_LEGACY,
    MIXER_TRANSITION_MANUAL_SESSION_AUTO,
} mixerTransitionManualSessionMode_e;

#ifdef USE_AUTO_TRANSITION
typedef struct {
    bool readyForHotSwitch;
    bool usedAirspeed;
    bool transitionStartAirspeedCaptured;
    float transitionStartAirspeedCmS;
    float progress;
} mixerTransitionHotSwitchProgress_t;

typedef struct {
    float pusherScale;
    float liftScale;
    float mcAuthorityScale;
    float fwAuthorityScale;
    float blendToFw;
} mixerTransitionScaleState_t;

typedef struct {
    uint16_t motorMask;
    uint16_t toCurrentMotorMask;
} mixerTransitionPostSwitchFadeMask_t;
#endif

static inline mixerTransitionManualSessionMode_e mixerTransitionUpdateManualSessionMode(
    mixerTransitionManualSessionMode_e currentMode,
    bool transitionModeRisingEdge,
    bool transitionModeFallingEdge,
    bool manualControllerConfigured,
    bool clearSession)
{
    if (clearSession || transitionModeFallingEdge) {
        return MIXER_TRANSITION_MANUAL_SESSION_NONE;
    }

    if (transitionModeRisingEdge) {
        return manualControllerConfigured ?
            MIXER_TRANSITION_MANUAL_SESSION_AUTO :
            MIXER_TRANSITION_MANUAL_SESSION_LEGACY;
    }

    return currentMode;
}

static inline float mixerTransitionClamp(float value, float low, float high)
{
    if (value < low) {
        return low;
    }

    if (value > high) {
        return high;
    }

    return value;
}

static inline bool mixerTransitionManualControllerEnabled(
    bool manualControllerConfigured,
    mixerTransitionManualSessionMode_e sessionMode)
{
    return sessionMode == MIXER_TRANSITION_MANUAL_SESSION_AUTO ||
           (sessionMode != MIXER_TRANSITION_MANUAL_SESSION_LEGACY && manualControllerConfigured);
}

#ifdef USE_AUTO_TRANSITION
static inline bool mixerTransitionKeepCompletedAutoSession(
    mixerTransitionManualSessionMode_e sessionMode,
    bool transitionModeFallingEdge,
    bool hotSwitchDone,
    int currentProfileIndex,
    int requestedProfileIndex)
{
    return sessionMode == MIXER_TRANSITION_MANUAL_SESSION_AUTO &&
           !transitionModeFallingEdge &&
           hotSwitchDone &&
           currentProfileIndex != requestedProfileIndex;
}

static inline bool mixerTransitionCompletedAutoSessionOwnsProfileSwitch(
    mixerTransitionManualSessionMode_e sessionMode,
    bool hotSwitchDone,
    int currentProfileIndex,
    int requestedProfileIndex)
{
    return sessionMode == MIXER_TRANSITION_MANUAL_SESSION_AUTO &&
           hotSwitchDone &&
           currentProfileIndex != requestedProfileIndex;
}

static inline mixerProfileATDirection_e mixerTransitionManualSwitchReminderDirection(
    mixerTransitionManualSessionMode_e sessionMode,
    bool autoTransitionActive,
    bool hotSwitchDone,
    bool transitionModeActive,
    int currentProfileIndex,
    int requestedProfileIndex,
    bool currentProfileIsMultirotor)
{
    if (sessionMode != MIXER_TRANSITION_MANUAL_SESSION_AUTO ||
        autoTransitionActive ||
        !hotSwitchDone) {
        return MIXERAT_DIRECTION_NONE;
    }

    if (!transitionModeActive && currentProfileIndex == requestedProfileIndex) {
        return MIXERAT_DIRECTION_NONE;
    }

    return currentProfileIsMultirotor ? MIXERAT_DIRECTION_TO_MC : MIXERAT_DIRECTION_TO_FW;
}

static inline bool mixerTransitionNavigationHandbackShouldHoldProfile(
    bool navigationOwnedProfileSwitchPreviousUpdate,
    bool navigationOwnsProfileSwitch,
    bool mixerProfileModePresent,
    bool autoTransitionActive,
    bool transitionModeActive,
    int currentProfileIndex,
    int requestedProfileIndex)
{
    return navigationOwnedProfileSwitchPreviousUpdate &&
           !navigationOwnsProfileSwitch &&
           mixerProfileModePresent &&
           !autoTransitionActive &&
           (transitionModeActive || currentProfileIndex != requestedProfileIndex);
}

static inline bool mixerTransitionNavigationHandbackShouldClear(
    bool navigationOwnsProfileSwitch,
    bool transitionModeRisingEdge,
    bool transitionModeActive,
    int currentProfileIndex,
    int requestedProfileIndex)
{
    return navigationOwnsProfileSwitch ||
           transitionModeRisingEdge ||
           (!transitionModeActive && currentProfileIndex == requestedProfileIndex);
}

static inline bool mixerTransitionShouldClearCompletedAutoMixingRequest(
    bool transitionModeActive,
    bool transitionModeRisingEdge,
    bool autoTransitionActive,
    bool hotSwitchDone)
{
    return transitionModeActive && !transitionModeRisingEdge && !autoTransitionActive && hotSwitchDone;
}

static inline bool mixerTransitionNavigationOwnsProfileSwitch(
    bool armed,
    bool vtolProfilePairConfigured,
    bool navWaypointActive,
    bool navRthActive,
    bool navLandingActive,
    bool navMixerAtActive)
{
    return armed &&
           vtolProfilePairConfigured &&
           (navWaypointActive || navRthActive || navLandingActive || navMixerAtActive);
}
#endif

static inline bool mixerTransitionIsRequestAllowed(
    mixerProfileATRequest_e requiredAction,
    bool stateAirplane,
    bool stateMultirotor,
    bool mixerProfileModePresent,
    bool automatedSwitch,
    bool targetProfileIsAirplane,
    bool targetProfileIsMultirotor)
{
    if ((!stateAirplane && !stateMultirotor) || !mixerProfileModePresent) {
        return false;
    }

    switch (requiredAction) {
    case MIXERAT_REQUEST_RTH:
        return automatedSwitch && stateMultirotor && targetProfileIsAirplane;

    case MIXERAT_REQUEST_LAND:
        return automatedSwitch && stateAirplane && targetProfileIsMultirotor;

#ifdef USE_AUTO_TRANSITION
    case MIXERAT_REQUEST_MISSION_TO_FW:
    case MIXERAT_REQUEST_MANUAL_TO_FW:
        return stateMultirotor && targetProfileIsAirplane;

    case MIXERAT_REQUEST_MISSION_TO_MC:
    case MIXERAT_REQUEST_MANUAL_TO_MC:
        return stateAirplane && targetProfileIsMultirotor;

    case MIXERAT_REQUEST_FW_TO_MC_PROTECTION:
        return automatedSwitch && stateAirplane && targetProfileIsMultirotor;
#endif

    default:
        return false;
    }
}

static inline bool mixerTransitionRequestAllowedDuringFailsafe(mixerProfileATRequest_e requiredAction)
{
    switch (requiredAction) {
    case MIXERAT_REQUEST_RTH:
    case MIXERAT_REQUEST_LAND:
        return true;

#ifdef USE_AUTO_TRANSITION
    case MIXERAT_REQUEST_FW_TO_MC_PROTECTION:
        return true;
#endif

    default:
        return false;
    }
}

#ifdef USE_AUTO_TRANSITION
static inline bool mixerTransitionFwToMcProtectionTriggered(
    const bool armed,
    const bool stateAirplane,
    const uint16_t thresholdCmS,
    const bool trustedAirspeedAvailable,
    const float airspeedCmS)
{
    return armed &&
           stateAirplane &&
           thresholdCmS > 0 &&
           trustedAirspeedAvailable &&
           airspeedCmS <= thresholdCmS;
}
#endif

static inline bool mixerTransitionShouldAbortForFailsafe(
    const mixerProfileATRequest_e requiredAction,
    const bool postSwitchActive,
    const bool hotSwitchDone)
{
    if (mixerTransitionRequestAllowedDuringFailsafe(requiredAction)) {
        return false;
    }

    // Once the target profile is already active, finishing the remaining output
    // ramp is safer than cancelling into an abrupt scale reset.
    return !(postSwitchActive && hotSwitchDone);
}

#ifdef USE_AUTO_TRANSITION
static inline float mixerTransitionComputeMotorRampProgress(
    bool dynamicMixerEnabled,
    uint16_t scaleRampTimeMs,
    uint32_t elapsedMs)
{
    if (!dynamicMixerEnabled || scaleRampTimeMs == 0) {
        return 1.0f;
    }

    return mixerTransitionClamp((float)elapsedMs / (float)scaleRampTimeMs, 0.0f, 1.0f);
}

static inline uint16_t mixerTransitionComputeServoHandoffDurationMs(
    bool dynamicMixerEnabled,
    uint16_t scaleRampTimeMs,
    uint32_t elapsedMs)
{
    (void)dynamicMixerEnabled;
    (void)elapsedMs;

    if (scaleRampTimeMs == 0) {
        return 0;
    }

    return scaleRampTimeMs;
}

static inline int16_t mixerTransitionRoundFloatToInt16(float value)
{
    return (int16_t)(value >= 0.0f ? value + 0.5f : value - 0.5f);
}

static inline float mixerTransitionResolveHandoffProgress(
    bool dynamicMixerEnabled,
    bool usedAirspeed,
    float previousHandoffProgress,
    float rawProgress)
{
    const float clampedProgress = mixerTransitionClamp(rawProgress, 0.0f, 1.0f);

    if (!dynamicMixerEnabled) {
        return 1.0f;
    }

    if (usedAirspeed) {
        return clampedProgress;
    }

    return previousHandoffProgress > clampedProgress ? previousHandoffProgress : clampedProgress;
}

static inline float mixerTransitionBlendScale(float from, float to, float progress)
{
    return from + (to - from) * mixerTransitionClamp(progress, 0.0f, 1.0f);
}

static inline int16_t mixerTransitionBlendCapturedMotorOutput(
    int16_t capturedOutput,
    int16_t targetOutput,
    float progress)
{
    const float holdScale = 1.0f - mixerTransitionClamp(progress, 0.0f, 1.0f);
    const int32_t blendedOutput = mixerTransitionRoundFloatToInt16(
        targetOutput + (capturedOutput - targetOutput) * holdScale);
    const int16_t low = targetOutput < capturedOutput ? targetOutput : capturedOutput;
    const int16_t high = targetOutput > capturedOutput ? targetOutput : capturedOutput;

    if (blendedOutput < low) {
        return low;
    }

    if (blendedOutput > high) {
        return high;
    }

    return (int16_t)blendedOutput;
}

static inline mixerTransitionScaleState_t mixerTransitionComputeScales(
    bool dynamicMixerEnabled,
    mixerProfileATDirection_e direction,
    float liftFloor,
    float mcFloor,
    float fwFloor,
    float handoffProgress,
    float motorRampProgress)
{
    mixerTransitionScaleState_t scales = {
        .pusherScale = 1.0f,
        .liftScale = 1.0f,
        .mcAuthorityScale = 1.0f,
        .fwAuthorityScale = 1.0f,
        .blendToFw = 1.0f,
    };

    if (!dynamicMixerEnabled) {
        return scales;
    }

    liftFloor = mixerTransitionClamp(liftFloor, 0.0f, 1.0f);
    mcFloor = mixerTransitionClamp(mcFloor, 0.0f, 1.0f);
    fwFloor = mixerTransitionClamp(fwFloor, 0.0f, 1.0f);

    if (direction == MIXERAT_DIRECTION_TO_FW) {
        scales.pusherScale = mixerTransitionBlendScale(0.0f, 1.0f, motorRampProgress);
        scales.liftScale = mixerTransitionBlendScale(1.0f, liftFloor, handoffProgress);
        scales.mcAuthorityScale = mixerTransitionBlendScale(1.0f, mcFloor, handoffProgress);
        scales.fwAuthorityScale = mixerTransitionBlendScale(fwFloor, 1.0f, handoffProgress);
    } else if (direction == MIXERAT_DIRECTION_TO_MC) {
        scales.pusherScale = mixerTransitionBlendScale(1.0f, 0.0f, motorRampProgress);
        scales.liftScale = mixerTransitionBlendScale(liftFloor, 1.0f, motorRampProgress);
        scales.mcAuthorityScale = mixerTransitionBlendScale(mcFloor, 1.0f, motorRampProgress);
        scales.fwAuthorityScale = mixerTransitionBlendScale(1.0f, fwFloor, handoffProgress);
    }

    scales.blendToFw = mixerTransitionClamp(scales.fwAuthorityScale, 0.0f, 1.0f);
    return scales;
}

static inline mixerTransitionHotSwitchProgress_t mixerTransitionEvaluateHotSwitch(
    mixerProfileATDirection_e direction,
    uint16_t airspeedThresholdCmS,
    bool trustedAirspeedAvailable,
    float currentAirspeedCmS,
    bool transitionStartAirspeedCaptured,
    float transitionStartAirspeedCmS,
    uint32_t elapsedMs,
    uint32_t transitionTimerMs)
{
    mixerTransitionHotSwitchProgress_t result = {
        .readyForHotSwitch = false,
        .usedAirspeed = false,
        .transitionStartAirspeedCaptured = transitionStartAirspeedCaptured,
        .transitionStartAirspeedCmS = transitionStartAirspeedCmS,
        .progress = 0.0f,
    };

    if (direction == MIXERAT_DIRECTION_NONE) {
        return result;
    }

    if (airspeedThresholdCmS > 0 && trustedAirspeedAvailable) {
        result.usedAirspeed = true;

        if (direction == MIXERAT_DIRECTION_TO_FW) {
            result.progress = mixerTransitionClamp(currentAirspeedCmS / airspeedThresholdCmS, 0.0f, 1.0f);
            result.readyForHotSwitch = currentAirspeedCmS >= airspeedThresholdCmS;
            return result;
        }

        if (!result.transitionStartAirspeedCaptured) {
            result.transitionStartAirspeedCmS = currentAirspeedCmS;
            result.transitionStartAirspeedCaptured = true;
        }

        if (result.transitionStartAirspeedCmS <= airspeedThresholdCmS) {
            result.progress = 1.0f;
        } else {
            result.progress = mixerTransitionClamp(
                (result.transitionStartAirspeedCmS - currentAirspeedCmS) /
                (result.transitionStartAirspeedCmS - airspeedThresholdCmS),
                0.0f,
                1.0f);
        }

        result.readyForHotSwitch = currentAirspeedCmS <= airspeedThresholdCmS;
        return result;
    }

    result.progress = transitionTimerMs > 0 ?
        mixerTransitionClamp((float)elapsedMs / (float)transitionTimerMs, 0.0f, 1.0f) :
        1.0f;
    result.readyForHotSwitch = elapsedMs >= transitionTimerMs;
    return result;
}

static inline mixerTransitionPostSwitchFadeMask_t mixerTransitionComputePostSwitchFadeMask(
    bool dynamicMixerEnabled,
    uint16_t scaleRampTimeMs,
    mixerProfileATDirection_e direction,
    bool currentProfileIsMultirotor,
    uint8_t motorCount,
    const motorMixer_t *currentMotorMixer,
    const motorMixer_t *targetMotorMixer)
{
    mixerTransitionPostSwitchFadeMask_t mask = { 0, 0 };

    if (!dynamicMixerEnabled ||
        scaleRampTimeMs == 0 ||
        direction == MIXERAT_DIRECTION_NONE ||
        !currentMotorMixer ||
        !targetMotorMixer) {
        return mask;
    }

    for (uint8_t i = 0; i < motorCount && i < MAX_SUPPORTED_MOTORS; i++) {
        const bool currentMotorActive = currentMotorMixer[i].throttle > 0.0f;
        const bool targetMotorActive = targetMotorMixer[i].throttle > 0.0f;
        const bool oldLiftMotor = direction == MIXERAT_DIRECTION_TO_FW &&
                                  currentProfileIsMultirotor &&
                                  currentMotorActive &&
                                  !targetMotorActive;
        const bool oldPusherMotor = direction == MIXERAT_DIRECTION_TO_MC &&
                                    !currentProfileIsMultirotor &&
                                    currentMotorActive &&
                                    !targetMotorActive;
        const bool targetFwPusherMotor = direction == MIXERAT_DIRECTION_TO_FW &&
                                         currentProfileIsMultirotor &&
                                         !currentMotorActive &&
                                         targetMotorActive;

        if (oldLiftMotor || oldPusherMotor || targetFwPusherMotor) {
            mask.motorMask |= (1U << i);
            if (targetFwPusherMotor) {
                mask.toCurrentMotorMask |= (1U << i);
            }
        }
    }

    return mask;
}
#endif

#ifdef USE_AUTO_TRANSITION
static inline float mixerTransitionComputeServoBlendToFw(
    bool legacyManualTransitionActive,
    bool transitionMixingActive,
    bool autoTransitionActive,
    bool postSwitchFadeToFwActive,
    bool dynamicMixerEnabled,
    mixerProfileATDirection_e direction,
    uint16_t scaleRampTimeMs,
    uint32_t elapsedMs)
{
    if (legacyManualTransitionActive) {
        return transitionMixingActive ? 1.0f : 0.0f;
    }

    if (postSwitchFadeToFwActive) {
        return 1.0f;
    }

    if (!autoTransitionActive || direction == MIXERAT_DIRECTION_NONE) {
        return 0.0f;
    }

    if (!dynamicMixerEnabled) {
        return transitionMixingActive ? 1.0f : 0.0f;
    }

    const float progress = mixerTransitionComputeMotorRampProgress(
        dynamicMixerEnabled,
        scaleRampTimeMs,
        elapsedMs);

    if (direction == MIXERAT_DIRECTION_TO_MC) {
        return 1.0f - progress;
    }

    return progress;
}
#endif

static inline int16_t mixerTransitionBlendToServoInput(float blendToFw)
{
    if (blendToFw <= 0.0f) {
        return 0;
    }

    if (blendToFw >= 1.0f) {
        return 500;
    }

    return (int16_t)(blendToFw * 500.0f + 0.5f);
}

static inline int16_t mixerTransitionUpdateServoInput(
    int16_t previousInput,
    bool legacyManualTransitionActive,
    bool transitionMixingActive,
    bool autoTransitionActive,
    bool postSwitchFadeToFwActive,
    bool transitionToFw,
    float blendToFw)
{
    int16_t desiredInput = 0;

    if (legacyManualTransitionActive) {
        desiredInput = transitionMixingActive ? 500 : 0;
    } else if (postSwitchFadeToFwActive) {
        desiredInput = 500;
    } else if (transitionMixingActive) {
        desiredInput = autoTransitionActive ? mixerTransitionBlendToServoInput(blendToFw) : 500;
    } else if (autoTransitionActive && transitionToFw) {
        desiredInput = mixerTransitionBlendToServoInput(blendToFw);
    }

    // Only the new auto-transition MC->FW path needs the anti-reverse latch.
    // Legacy mode must keep its original fixed 0/500 endpoint behaviour.
    if (transitionToFw &&
        desiredInput < previousInput &&
        !legacyManualTransitionActive &&
        (transitionMixingActive || autoTransitionActive || postSwitchFadeToFwActive)) {
        return previousInput;
    }

    return desiredInput;
}

static inline int16_t mixerTransitionBlendCapturedServoOutput(
    int16_t capturedOutput,
    int16_t currentOutput,
    float progress)
{
    const float holdScale = 1.0f - mixerTransitionClamp(progress, 0.0f, 1.0f);
    const float fadedOutput = currentOutput + (capturedOutput - currentOutput) * holdScale;
    const int16_t minOutput = capturedOutput < currentOutput ? capturedOutput : currentOutput;
    const int16_t maxOutput = capturedOutput > currentOutput ? capturedOutput : currentOutput;

    if (fadedOutput <= minOutput) {
        return minOutput;
    }

    if (fadedOutput >= maxOutput) {
        return maxOutput;
    }

    return (int16_t)(fadedOutput + (fadedOutput >= 0.0f ? 0.5f : -0.5f));
}

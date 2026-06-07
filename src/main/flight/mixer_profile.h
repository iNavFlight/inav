#pragma once

#include "config/parameter_group.h"
#include "flight/failsafe.h"
#include "flight/mixer.h"
#include "flight/servos.h"

#ifndef MAX_MIXER_PROFILE_COUNT
#define MAX_MIXER_PROFILE_COUNT 2
#endif

typedef struct mixerConfig_s {
    int8_t motorDirectionInverted;
    uint8_t platformType;
    bool hasFlaps;
    int16_t appliedMixerPreset;
    bool motorstopOnLow;
    bool controlProfileLinking;
    bool automated_switch;
    int16_t switchTransitionTimer;
#ifdef USE_AUTO_TRANSITION
    bool vtolTransitionDynamicMixer;
    bool manualVtolTransitionController;
    uint16_t vtolTransitionAirspeedTimeoutMs;
    uint16_t vtolTransitionScaleRampTimeMs;
#endif
    bool tailsitterOrientationOffset;
    int16_t transition_PID_mmix_multiplier_roll;
    int16_t transition_PID_mmix_multiplier_pitch;
    int16_t transition_PID_mmix_multiplier_yaw;
} mixerConfig_t;
typedef struct mixerProfile_s {
    mixerConfig_t mixer_config;
    motorMixer_t MotorMixers[MAX_SUPPORTED_MOTORS];
    servoMixer_t ServoMixers[MAX_SERVO_RULES];
} mixerProfile_t;

PG_DECLARE_ARRAY(mixerProfile_t, MAX_MIXER_PROFILE_COUNT, mixerProfiles);
typedef enum {
    MIXERAT_REQUEST_NONE, //no request, stats checking only
    MIXERAT_REQUEST_RTH,
    MIXERAT_REQUEST_LAND,
#ifdef USE_AUTO_TRANSITION
    MIXERAT_REQUEST_MISSION_TO_FW,
    MIXERAT_REQUEST_MISSION_TO_MC,
    MIXERAT_REQUEST_MANUAL_TO_FW,
    MIXERAT_REQUEST_MANUAL_TO_MC,
#endif
    MIXERAT_REQUEST_ABORT,
} mixerProfileATRequest_e;

#ifdef USE_AUTO_TRANSITION
typedef enum {
    MIXERAT_DIRECTION_NONE = 0,
    MIXERAT_DIRECTION_TO_FW,
    MIXERAT_DIRECTION_TO_MC,
} mixerProfileATDirection_e;
#endif

//mixerProfile Automated Transition PHASE
typedef enum {
    MIXERAT_PHASE_IDLE,
    MIXERAT_PHASE_TRANSITION_INITIALIZE,
    MIXERAT_PHASE_TRANSITIONING,
#ifndef USE_AUTO_TRANSITION
    MIXERAT_PHASE_DONE,
#endif
} mixerProfileATState_e;

typedef struct mixerProfileAT_s {
    mixerProfileATState_e phase;
#ifdef USE_AUTO_TRANSITION
    mixerProfileATDirection_e direction;
    mixerProfileATRequest_e request;
    bool aborted;
    bool abortedByAirspeedTimeout;
    bool hotSwitchDone;
    bool usedAirspeed;
    bool transitionStartAirspeedCaptured;
    float progress;
    float handoffScalingProgress;
    float transitionStartAirspeedCmS;
    float blendToFw;
    float pusherScale;
    float liftScale;
    float mcAuthorityScale;
    float fwAuthorityScale;
    timeMs_t transitionStartTime;
#else
    bool transitionInputMixing;
    timeMs_t transitionStartTime;
    timeMs_t transitionStabEndTime;
    timeMs_t transitionTransEndTime;
#endif
} mixerProfileAT_t;
extern mixerProfileAT_t mixerProfileAT;
bool checkMixerATRequired(mixerProfileATRequest_e required_action);
bool mixerATUpdateState(mixerProfileATRequest_e required_action);
bool mixerATIsActive(void);
bool mixerATWasAborted(void);
bool mixerATWasAbortedByAirspeedTimeout(void);
float mixerATGetPusherScale(void);
float mixerATGetLiftScale(void);
float mixerATGetMcAuthorityScale(void);
float mixerATGetFwAuthorityScale(void);
float mixerATGetBlendToFw(void);
bool isMixerProfile2ModeReportedActive(void);
bool isMixerTransitionModeReportedActive(void);

extern mixerConfig_t currentMixerConfig;
extern int currentMixerProfileIndex;
extern int nextMixerProfileIndex;
extern bool isMixerTransitionMixing;
#define mixerConfig() (&(mixerProfiles(systemConfig()->current_mixer_profile_index)->mixer_config))
#define mixerConfigMutable() ((mixerConfig_t *) mixerConfig())

#define primaryMotorMixer(_index) (&(mixerProfiles(systemConfig()->current_mixer_profile_index)->MotorMixers)[_index])
#define primaryMotorMixerMutable(_index) ((motorMixer_t *)primaryMotorMixer(_index))
#define customServoMixers(_index) (&(mixerProfiles(systemConfig()->current_mixer_profile_index)->ServoMixers)[_index])
#define customServoMixersMutable(_index) ((servoMixer_t *)customServoMixers(_index))

static inline const mixerProfile_t* mixerProfiles_CopyArray_by_index(int _index) { return &mixerProfiles_CopyArray[_index]; }
#define primaryMotorMixer_CopyArray() (mixerProfiles_CopyArray_by_index(systemConfig()->current_mixer_profile_index)->MotorMixers)
#define customServoMixers_CopyArray() (mixerProfiles_CopyArray_by_index(systemConfig()->current_mixer_profile_index)->ServoMixers)

#define mixerConfigByIndex(index) (&(mixerProfiles(index)->mixer_config))
#define mixerMotorMixersByIndex(index) (mixerProfiles(index)->MotorMixers)
#define mixerServoMixersByIndex(index) (mixerProfiles(index)->ServoMixers)

bool platformTypeConfigured(flyingPlatformType_e platformType);
bool outputProfileHotSwitch(int profile_index);
bool checkMixerProfileHotSwitchAvalibility(void);
void activateMixerConfig(void);
void mixerConfigInit(void);
void outputProfileUpdateTask(timeUs_t currentTimeUs);

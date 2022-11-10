#pragma once

#include "config/parameter_group.h"

#include "flight/mixer.h"
#include "flight/servos.h"

#ifndef MAX_MIXER_PROFILE_COUNT
#define MAX_MIXER_PROFILE_COUNT 2
#endif

typedef struct mixerProfile_s {
    mixerConfig_t mixer_config;
    motorMixer_t MotorMixers[MAX_SUPPORTED_MOTORS];
    servoMixer_t ServoMixers[MAX_SERVO_RULES];
} mixerProfile_t;

PG_DECLARE_ARRAY(mixerProfile_t, MAX_MIXER_PROFILE_COUNT, mixerProfiles);

#define mixerConfig() (&(mixerProfiles(systemConfig()->current_mixer_profile_index)->mixer_config))
#define mixerConfigMutable() ((mixerConfig_t *) mixerConfig())
#define primaryMotorMixer(_index) (&(mixerProfiles(systemConfig()->current_mixer_profile_index)->MotorMixers)[_index])
#define primaryMotorMixerMutable(_index) ((motorMixer_t *)primaryMotorMixer(_index))

static inline const mixerProfile_t* mixerProfiles_CopyArray_macro(int _index) { return &mixerProfiles_CopyArray[_index]; }
#define primaryMotorMixer_CopyArray() (mixerProfiles_CopyArray_macro(systemConfig()->current_mixer_profile_index)->MotorMixers)

#define mixerConfigByIndex(index) (&(mixerProfiles(index)->mixer_config))

bool OutputProfileHotSwitch(int profile_index);

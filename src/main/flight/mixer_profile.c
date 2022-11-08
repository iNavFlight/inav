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
#include "flight/mixer.h"
// #include "flight/pid.h"
#include "flight/servos.h"

#include "fc/config.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "common/log.h"

PG_REGISTER_ARRAY_WITH_RESET_FN(mixerProfile_t, MAX_MIXER_PROFILE_COUNT, mixerProfiles, PG_MIXER_PROFILE, 1);

void pgResetFn_mixerProfiles(mixerProfile_t *instance)
{
    for (int i = 0; i < MAX_MIXER_PROFILE_COUNT; i++) {
        RESET_CONFIG(mixerProfile_t, &instance[i],
            .mixer_config = {
                .motorDirectionInverted = SETTING_MOTOR_DIRECTION_INVERTED_DEFAULT,
                .platformType = SETTING_PLATFORM_TYPE_DEFAULT,
                .hasFlaps = SETTING_HAS_FLAPS_DEFAULT,
                .appliedMixerPreset = SETTING_MODEL_PREVIEW_TYPE_DEFAULT, //This flag is not available in CLI and used by Configurator only
                .outputMode = SETTING_OUTPUT_MODE_DEFAULT,
            }
        );
        motorMixer_t tmp_mixer = {.throttle=0,.roll=0,.pitch=0,.yaw=0};
        for (int j = 0; j < MAX_SUPPORTED_MOTORS; j++) {
            instance->MotorMixers[j] = tmp_mixer;
        }
    }
}

// PG_REGISTER_ARRAY(motorMixer_t, MAX_SUPPORTED_MOTORS, primaryMotorMixer, PG_MOTOR_MIXER, 0);

int min_ab(int a,int b)
{
    return a > b ? b : a;
}

bool OutputProfileHotSwitch(int profile_index)
{   
#ifdef ENABLE_MIXER_PROFILE_HOTSWAP
    // does not work with timerHardwareOverride
    LOG_INFO(PWM, "OutputProfileHotSwitch");

    //store current output for check
    uint8_t old_platform_type=mixerConfig()->platformType;
    timMotorServoHardware_t old_timOutputs;
    pwmBuildTimerOutputList(&old_timOutputs, isMixerUsingServos());
    if (!setConfigMixerProfile(profile_index)){
        return false;
    }
    stopMotors();
    // delay(3000); //check motor stop
    servosInit();
    mixerUpdateStateFlags();
    mixerInit();

    // timMotorServoHardware_t timOutputs;
    // pwmBuildTimerOutputList(&timOutputs, isMixerUsingServos());
    // bool motor_output_type_not_changed = old_timOutputs.maxTimMotorCount == timOutputs.maxTimMotorCount;
    // bool servo_output_type_not_changed = old_timOutputs.maxTimServoCount == timOutputs.maxTimServoCount;
    // LOG_INFO(PWM, "motor_output_type_not_changed:%d,%d,%d",motor_output_type_not_changed,old_timOutputs.maxTimMotorCount,timOutputs.maxTimMotorCount);
    // for (int i; i < min_ab(old_timOutputs.maxTimMotorCount,timOutputs.maxTimMotorCount); i++)
    // {
    //     LOG_INFO(PWM, "motor_output_type_not_changed:%d,%d",i,motor_output_type_not_changed);
    //     motor_output_type_not_changed &= old_timOutputs.timMotors[i]->tag==timOutputs.timMotors[i]->tag;
    // }
    // LOG_INFO(PWM, "motor_output_type_not_changed:%d",motor_output_type_not_changed);

    // LOG_INFO(PWM, "servo_output_type_not_changed:%d,%d,%d",servo_output_type_not_changed,old_timOutputs.maxTimServoCount,timOutputs.maxTimServoCount);
    // for (int i; i < min_ab(old_timOutputs.maxTimServoCount,timOutputs.maxTimServoCount); i++)
    // {
    //     LOG_INFO(PWM, "servo_output_type_not_changed:%d,%d",i,servo_output_type_not_changed);
    //     servo_output_type_not_changed &= old_timOutputs.timServos[i]->tag==timOutputs.timServos[i]->tag;
    // }
    // LOG_INFO(PWM, "servo_output_type_not_changed:%d",servo_output_type_not_changed);

    // if(!motor_output_type_not_changed || !servo_output_type_not_changed){
    //     LOG_INFO(PWM, "pwmMotorAndServoHotInit");
    //     pwmMotorAndServoHotInit(&timOutputs);
    // }
    if(old_platform_type!=mixerConfig()->platformType)
    {
        
    }
    return true;
#else
    profile_index=0; //prevent compilier warning: parameter 'profile_index' set but not used
    return (bool) profile_index;
#endif
}


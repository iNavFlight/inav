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
#include "common/axis.h"
#include "flight/pid.h"
#include "flight/servos.h"

#include "fc/fc_core.h"
#include "fc/config.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "programming/logic_condition.h"
#include "navigation/navigation.h"

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
        for (int j = 0; j < MAX_SUPPORTED_MOTORS; j++) {
            RESET_CONFIG(motorMixer_t, &instance[i].MotorMixers[j],
                .throttle=0,
                .roll=0,
                .pitch=0,
                .yaw=0
            );
        }
        for (int j = 0; j < MAX_SERVO_RULES; j++) {
            RESET_CONFIG(servoMixer_t, &instance[i].ServoMixers[j],
            .targetChannel = 0,
            .inputSource = 0,
            .rate = 0,
            .speed = 0
#ifdef USE_PROGRAMMING_FRAMEWORK
            ,.conditionId = -1
#endif
            );
        }
    }
}

static int computeMotorCountByMixerProfileIndex(int index)
{
    int motorCount = 0;
    const motorMixer_t* temp_motormixers=mixerMotorMixersByIndex(index)[0];
    for (int i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
        // check if done
        if (temp_motormixers[i].throttle == 0.0f) {
            break;
        }
        motorCount++;
    }
    return motorCount;
}

static int computeServoCountByMixerProfileIndex(int index)
{
    int servoRuleCount = 0;
    int minServoIndex = 255;
    int maxServoIndex = 0;

    const servoMixer_t* temp_servomixers=mixerServoMixersByIndex(index)[0];
    for (int i = 0; i < MAX_SERVO_RULES; i++) {
        // mixerServoMixersByIndex(index)[i]->targetChannel will occour problem after i=1
        // LOG_INFO(PWM, "i:%d, targetChannel:%d, inputSource:%d, rate:%d",i,mixerServoMixersByIndex(index)[i]->targetChannel,mixerServoMixersByIndex(index)[i]->inputSource,mixerServoMixersByIndex(index)[i]->rate);
        // LOG_INFO(PWM, "i:%d, targetChannel:%d, inputSource:%d, rate:%d",i,mixerProfiles_SystemArray[index].ServoMixers[i].targetChannel,mixerProfiles_SystemArray[index].ServoMixers[i].inputSource,mixerProfiles_SystemArray[index].ServoMixers[i].rate);
        // LOG_INFO(PWM, "i:%d, targetChannel:%d, inputSource:%d, rate:%d",i,temp_servomixers[i].targetChannel,temp_servomixers[i].inputSource,temp_servomixers[i].rate);
        if (temp_servomixers[i].rate == 0)
            break;

        if (temp_servomixers[i].targetChannel < minServoIndex) {
            minServoIndex = temp_servomixers[i].targetChannel;
        }

        if (temp_servomixers[i].targetChannel > maxServoIndex) {
            maxServoIndex = temp_servomixers[i].targetChannel;
        }
        // LOG_INFO(PWM, "i:%d, minServoIndex:%d, maxServoIndex:%d",i,minServoIndex,maxServoIndex);
        servoRuleCount++;
    }
    if (servoRuleCount) {
        return 1 + maxServoIndex - minServoIndex;
    }
    else {
        return 0;
    }
}

//pid init will be done by the following pid profile change
static bool CheckIfPidInitNeededInSwitch(void)
{
    static bool ret = true;
    if (!ret)
    {
        return false;
    }
    for (uint8_t i = 0; i < MAX_LOGIC_CONDITIONS; i++)
    {
        const int activatorValue = logicConditionGetValue(logicConditions(i)->activatorId);
        const logicOperand_t *operandA = &(logicConditions(i)->operandA);
        if (logicConditions(i)->enabled && activatorValue && logicConditions(i)->operation == LOGIC_CONDITION_SET_PROFILE &&
            operandA->type == LOGIC_CONDITION_OPERAND_TYPE_FLIGHT && operandA->value == LOGIC_CONDITION_OPERAND_FLIGHT_ACTIVE_MIXER_PROFILE &&
            logicConditions(i)->flags == 0)
        {
            ret = false;
            return false;
        }
    }
    return true;
}

bool OutputProfileHotSwitch(int profile_index)
{
    // does not work with timerHardwareOverride
    LOG_INFO(PWM, "OutputProfileHotSwitch");
    if (profile_index < 0 || profile_index >= MAX_MIXER_PROFILE_COUNT)
    { // sanity check
        LOG_INFO(PWM, "invalid mixer profile index");
        return false;
    }
    if (getConfigMixerProfile() == profile_index)
    {
        return false;
    }
    if (areSensorsCalibrating()) {//it seems like switching before sensors calibration complete will cause pid stops to respond, especially in D
        return false;
    }
    //do not allow switching in navigation mode
    if (ARMING_FLAG(ARMED) && navigationInAnyMode()){
        LOG_INFO(PWM, "mixer switch failed, navModesEnabled");
        return false;
    }
    //do not allow switching between multi rotor and non multi rotor
#ifdef ENABLE_MIXER_PROFILE_MCFW_HOTSWAP
    bool MCFW_hotswap_unavailable = false;
#else
    bool MCFW_hotswap_unavailable = true;
#endif
    uint8_t old_platform_type = mixerConfig()->platformType;
    uint8_t new_platform_type = mixerConfigByIndex(profile_index)->platformType;
    bool old_platform_type_mc = old_platform_type == PLATFORM_MULTIROTOR || old_platform_type == PLATFORM_TRICOPTER;
    bool new_platform_type_mc = new_platform_type == PLATFORM_MULTIROTOR || new_platform_type == PLATFORM_TRICOPTER;
    bool is_mcfw_switching = old_platform_type_mc ^ new_platform_type_mc;
    if (MCFW_hotswap_unavailable && is_mcfw_switching)
    {
        LOG_INFO(PWM, "mixer MCFW_hotswap_unavailable");
        return false;
    }
    //do not allow switching if motor or servos counts has changed
    if ((getMotorCount() != computeMotorCountByMixerProfileIndex(profile_index)) || (getServoCount() != computeServoCountByMixerProfileIndex(profile_index)))
    {
        LOG_INFO(PWM, "mixer switch failed, motor/servo count will change");
        // LOG_INFO(PWM, "old motor/servo count:%d,%d",getMotorCount(),getServoCount());
        // LOG_INFO(PWM, "new motor/servo count:%d,%d",computeMotorCountByMixerProfileIndex(profile_index),computeServoCountByMixerProfileIndex(profile_index));
        return false;
    }
    if (!setConfigMixerProfile(profile_index)){
        LOG_INFO(PWM, "mixer switch failed to set config");
        return false;
    }
    // stopMotors();
    writeAllMotors(feature(FEATURE_REVERSIBLE_MOTORS) ? reversibleMotorsConfig()->neutral : motorConfig()->mincommand);//stop motors without delay
    servosInit();
    mixerUpdateStateFlags();
    mixerInit();

    if(old_platform_type!=mixerConfig()->platformType)
    {
        navigationYawControlInit();
        if (CheckIfPidInitNeededInSwitch())
        {
            LOG_INFO(PWM, "mixer switch pidInit");
            pidInit();
            pidInitFilters();
            schedulePidGainsUpdate();
            navigationUsePIDs();
        }
    }
    return true;
}

// static int min_ab(int a,int b)
// {
//     return a > b ? b : a;
// }

// void checkOutputMapping(int profile_index)//debug purpose
// {
//     timMotorServoHardware_t old_timOutputs;
//     pwmBuildTimerOutputList(&old_timOutputs, isMixerUsingServos());
//     stopMotors();
//     delay(1000); //check motor stop
//     if (!setConfigMixerProfile(profile_index)){
//         LOG_INFO(PWM, "failed to set config");
//         return;
//     }
//     servosInit();
//     mixerUpdateStateFlags();
//     mixerInit();
//     timMotorServoHardware_t timOutputs;
//     pwmBuildTimerOutputList(&timOutputs, isMixerUsingServos());
//     bool motor_output_type_not_changed = old_timOutputs.maxTimMotorCount == timOutputs.maxTimMotorCount;
//     bool servo_output_type_not_changed = old_timOutputs.maxTimServoCount == timOutputs.maxTimServoCount;
//     LOG_INFO(PWM, "maxTimMotorCount:%d,%d",old_timOutputs.maxTimMotorCount,timOutputs.maxTimMotorCount);
//     for (int i; i < min_ab(old_timOutputs.maxTimMotorCount,timOutputs.maxTimMotorCount); i++)
//     {
//         LOG_INFO(PWM, "motor_output_type_not_changed:%d,%d",i,motor_output_type_not_changed);
//         motor_output_type_not_changed &= old_timOutputs.timMotors[i]->tag==timOutputs.timMotors[i]->tag;
//     }
//     LOG_INFO(PWM, "motor_output_type_not_changed:%d",motor_output_type_not_changed);

//     LOG_INFO(PWM, "maxTimServoCount:%d,%d",old_timOutputs.maxTimServoCount,timOutputs.maxTimServoCount);
//     for (int i; i < min_ab(old_timOutputs.maxTimServoCount,timOutputs.maxTimServoCount); i++)
//     {
//         LOG_INFO(PWM, "servo_output_type_not_changed:%d,%d",i,servo_output_type_not_changed);
//         servo_output_type_not_changed &= old_timOutputs.timServos[i]->tag==timOutputs.timServos[i]->tag;
//     }
//     LOG_INFO(PWM, "servo_output_type_not_changed:%d",servo_output_type_not_changed);

//     if(!motor_output_type_not_changed || !servo_output_type_not_changed){
//         LOG_INFO(PWM, "pwm output mapping has changed");
//     }
// }



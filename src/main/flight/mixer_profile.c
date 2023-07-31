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
#include "flight/failsafe.h"
#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "fc/fc_core.h"
#include "fc/config.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"
#include "fc/rc_modes.h"

#include "programming/logic_condition.h"
#include "navigation/navigation.h"

#include "common/log.h"

mixerConfig_t currentMixerConfig;
int currentMixerProfileIndex;
bool isMixerTransitionMixing;
bool isMixerTransitionMixing_requested;
mixerProfileAT_t mixerProfileAT;

PG_REGISTER_ARRAY_WITH_RESET_FN(mixerProfile_t, MAX_MIXER_PROFILE_COUNT, mixerProfiles, PG_MIXER_PROFILE, 1);

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
                         .outputMode = SETTING_OUTPUT_MODE_DEFAULT,
                         .motorstopOnLow = SETTING_MOTORSTOP_ON_LOW_DEFAULT,
                         .PIDProfileLinking = SETTING_MIXER_PID_PROFILE_LINKING_DEFAULT,
                         .switchOnFSRTH = SETTING_MIXER_SWITCH_ON_FS_RTH_DEFAULT,
                         .switchOnFSLand = SETTING_MIXER_SWITCH_ON_FS_LAND_DEFAULT,
                         .switchOnFSStabilizationTimer = SETTING_MIXER_SWITCH_ON_FS_STAB_TIMER_DEFAULT,
                         .switchOnFSTransitionTimer =  SETTING_MIXER_SWITCH_ON_FS_TRANS_TIMER_DEFAULT,
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

void mixerConfigInit(void)
{
    currentMixerProfileIndex = getConfigMixerProfile();
    currentMixerConfig = *mixerConfig();
    servosInit();
    mixerUpdateStateFlags();
    mixerInit();
    if (currentMixerConfig.PIDProfileLinking)
    {
        LOG_INFO(PWM, "mixer switch pidInit");
        setConfigProfile(getConfigMixerProfile());
        pidInit();
        pidInitFilters();
        pidResetErrorAccumulators();
        schedulePidGainsUpdate();
        navigationUsePIDs(); // set navigation pid gains
    }
}

// static int computeMotorCountByMixerProfileIndex(int index)
// {
//     int motorCount = 0;
//     const motorMixer_t* temp_motormixers=mixerMotorMixersByIndex(index)[0];
//     for (int i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
//         // check if done
//         if (temp_motormixers[i].throttle == 0.0f) {
//             break;
//         }
//         motorCount++;
//     }
//     return motorCount;
// }

// static int computeServoCountByMixerProfileIndex(int index)
// {
//     int servoRuleCount = 0;
//     int minServoIndex = 255;
//     int maxServoIndex = 0;

//     const servoMixer_t* temp_servomixers=mixerServoMixersByIndex(index)[0];
//     for (int i = 0; i < MAX_SERVO_RULES; i++) {
//         if (temp_servomixers[i].rate == 0)
//             break;

//         if (temp_servomixers[i].targetChannel < minServoIndex) {
//             minServoIndex = temp_servomixers[i].targetChannel;
//         }

//         if (temp_servomixers[i].targetChannel > maxServoIndex) {
//             maxServoIndex = temp_servomixers[i].targetChannel;
//         }
//         // LOG_INFO(PWM, "i:%d, minServoIndex:%d, maxServoIndex:%d",i,minServoIndex,maxServoIndex);
//         servoRuleCount++;
//     }
//     if (servoRuleCount) {
//         return 1 + maxServoIndex - minServoIndex;
//     }
//     else {
//         return 0;
//     }
// }

bool mixerATRequiresAngleMode(void)
{
    return (mixerProfileAT.phase == MIXERAT_PHASE_TRANSITIONING) || (mixerProfileAT.phase == MIXERAT_PHASE_STAB_AND_CLIMB);
}

void setMixerProfileAT(void)
{
    mixerProfileAT.transitionStartTime = millis();
    mixerProfileAT.transitionStabEndTime = mixerProfileAT.transitionStartTime + (timeMs_t)currentMixerConfig.switchOnFSStabilizationTimer * 100;
    mixerProfileAT.transitionTransEndTime = mixerProfileAT.transitionStabEndTime + (timeMs_t)currentMixerConfig.switchOnFSTransitionTimer * 100;
    activateMIXERATHelper();
}

void performMixerProfileAT(int nextProfileIndex)
{
    abortMIXERATHelper();
    isMixerTransitionMixing_requested = false;
    outputProfileHotSwitch(nextProfileIndex);
}

void abortMixerProfileAT(void)
{
    abortMIXERATHelper();
    isMixerTransitionMixing_requested = false;
}

bool mixerATUpdateState(mixerProfileATRequest_t required_action)
{   
    //return true if mixerAT condition is met or setting is not valid
    //set mixer profile automated transition according to failsafe phase
    //on non vtol setups , behave as normal  
    if ((!STATE(AIRPLANE)) && (!STATE(MULTIROTOR)))
    {
        return true;
    }
    if (!isModeActivationConditionPresent(BOXMIXERPROFILE))
    {
        return true;
    }
    int nextProfileIndex = 0;
    bool reprocessState;
    do
    {   
        reprocessState=false;
        nextProfileIndex = (currentMixerProfileIndex + 1) % MAX_MIXER_PROFILE_COUNT;
        if (required_action==MIXERAT_REQUEST_ABORT){
            abortMixerProfileAT();
            mixerProfileAT.phase = MIXERAT_PHASE_IDLE;
        }
        switch (mixerProfileAT.phase)
        {
        case MIXERAT_PHASE_IDLE:
            // LOG_INFO(PWM, "MIXERAT_PHASE_IDLE");
            //check if mixerAT is required
            if ((required_action == MIXERAT_REQUEST_RTH) && currentMixerConfig.switchOnFSRTH && STATE(MULTIROTOR))
            {
                if(!mixerConfigByIndex(nextProfileIndex)->switchOnFSRTH)//check next mixer_profile setting is valid
                {
                    mixerProfileAT.phase=MIXERAT_PHASE_TRANSITION_INITIALIZE;
                    reprocessState = true;
                }
            }
            else if ((required_action == MIXERAT_REQUEST_LAND) && currentMixerConfig.switchOnFSLand && STATE(AIRPLANE))
            {
                if(!mixerConfigByIndex(nextProfileIndex)->switchOnFSLand)//check next mixer_profile setting is valid
                {
                    mixerProfileAT.phase=MIXERAT_PHASE_TRANSITION_INITIALIZE;
                    reprocessState = true;
                }
            }
            break;
        case MIXERAT_PHASE_TRANSITION_INITIALIZE:
            // LOG_INFO(PWM, "MIXERAT_PHASE_IDLE");
            setMixerProfileAT();
            mixerProfileAT.phase = MIXERAT_PHASE_STAB_AND_CLIMB;
            reprocessState = true;
            break;
        case MIXERAT_PHASE_STAB_AND_CLIMB:
            isMixerTransitionMixing_requested = false;
            if (millis() > mixerProfileAT.transitionStabEndTime){
                mixerProfileAT.phase = MIXERAT_PHASE_TRANSITIONING;
                reprocessState = true;
            }
            return false;
            break;
        case MIXERAT_PHASE_TRANSITIONING:
            isMixerTransitionMixing_requested = true;
            if (millis() > mixerProfileAT.transitionTransEndTime){
                performMixerProfileAT(nextProfileIndex);
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
}

bool checkMixerProfileHotSwitchAvalibility(void)
{
    static int allow_hot_switch = -1;
    // pwm mapping maps outputs based on platformtype, check if mapping remain unchanged after the switch
    // do not allow switching between multi rotor and non multi rotor if sannity check fails
    if (MAX_MIXER_PROFILE_COUNT != 2)
    {
        return false;
    }
    if (allow_hot_switch == 0)
    {
        return false;
    }
    if (allow_hot_switch == 1)
    {
        return true;
    }
#ifdef ENABLE_MIXER_PROFILE_MCFW_HOTSWAP
    bool MCFW_hotswap_available = true;
#else
    bool MCFW_hotswap_available = false;
#endif
    uint8_t platform_type0 = mixerConfigByIndex(0)->platformType;
    uint8_t platform_type1 = mixerConfigByIndex(1)->platformType;
    bool platform_type_mc0 = (platform_type0 == PLATFORM_MULTIROTOR) || (platform_type0 == PLATFORM_TRICOPTER);
    bool platform_type_mc1 = (platform_type1 == PLATFORM_MULTIROTOR) || (platform_type1 == PLATFORM_TRICOPTER);
    bool is_mcfw_switching = platform_type_mc0 ^ platform_type_mc1;
    if ((!MCFW_hotswap_available) && is_mcfw_switching)
    {
        allow_hot_switch = 0;
        return false;
    }
    // not necessary when map motor/servos of all mixer profiles on the first boot
    // do not allow switching if motor or servos counts are different
    //  if ((computeMotorCountByMixerProfileIndex(0) != computeMotorCountByMixerProfileIndex(1)) || (computeServoCountByMixerProfileIndex(0) != computeServoCountByMixerProfileIndex(1)))
    //  {
    //      allow_hot_switch = 0;
    //      return false;
    //  }
    allow_hot_switch = 1;
    return true;
}

bool isNavBoxModesEnabled(void)
{
    return IS_RC_MODE_ACTIVE(BOXNAVRTH) || IS_RC_MODE_ACTIVE(BOXNAVWP) || IS_RC_MODE_ACTIVE(BOXNAVPOSHOLD) || (STATE(FIXED_WING_LEGACY) && IS_RC_MODE_ACTIVE(BOXNAVALTHOLD)) || (STATE(FIXED_WING_LEGACY) && (IS_RC_MODE_ACTIVE(BOXNAVCOURSEHOLD) || IS_RC_MODE_ACTIVE(BOXNAVCRUISE)));
}

void outputProfileUpdateTask(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    // transition mode input for servo mix and motor mix
    if (failsafePhase() == FAILSAFE_IDLE)
    {
        isMixerTransitionMixing_requested = IS_RC_MODE_ACTIVE(BOXMIXERTRANSITION)  && (!isNavBoxModesEnabled()); // update BOXMIXERTRANSITION_input
    }
    isMixerTransitionMixing = isMixerTransitionMixing_requested && ((posControl.navState == NAV_STATE_IDLE) ||(posControl.navState == NAV_STATE_MIXERAT_IN_PROGRESS));

    if (failsafePhase() == FAILSAFE_IDLE)
    {
        // do not allow switching when user activated navigation mode
        if (!isNavBoxModesEnabled())
        {
            outputProfileHotSwitch((int)IS_RC_MODE_ACTIVE(BOXMIXERPROFILE));
        }
    }
}

// switch mixerprofile without reboot
bool outputProfileHotSwitch(int profile_index)
{
    static bool allow_hot_switch = true;
    // does not work with timerHardwareOverride,need to set mixerConfig()->outputMode == OUTPUT_MODE_AUTO
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
    { // it seems like switching before sensors calibration complete will cause pid stops to respond, especially in D,TODO
        return false;
    }
    if (!checkMixerProfileHotSwitchAvalibility())
    {
        // LOG_INFO(PWM, "mixer switch failed, checkMixerProfileHotSwitchAvalibility");
        return false;
    }
    if  (posControl.navState != NAV_STATE_IDLE)
    {
        // LOG_INFO(PWM, "mixer switch failed, navState != NAV_STATE_IDLE");
        return false;
    }
    if (!setConfigMixerProfile(profile_index))
    {
        // LOG_INFO(PWM, "mixer switch failed to set config");
        return false;
    }
    stopMotorsNoDelay(); // not necessary, but just in case something goes wrong. But will a short period of stop command cause problem?
    mixerConfigInit();
    return true;
}

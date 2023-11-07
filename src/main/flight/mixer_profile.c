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
#include "fc/cli.h"

#include "programming/logic_condition.h"
#include "navigation/navigation.h"

#include "common/log.h"

mixerConfig_t currentMixerConfig;
int currentMixerProfileIndex;
bool isMixerTransitionMixing;
bool isMixerTransitionMixing_requested;
mixerProfileAT_t mixerProfileAT;
int nextMixerProfileIndex;

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
                         .motorstopOnLow = SETTING_MOTORSTOP_ON_LOW_DEFAULT,
                         .PIDProfileLinking = SETTING_MIXER_PID_PROFILE_LINKING_DEFAULT,
                         .automated_switch = SETTING_MIXER_AUTOMATED_SWITCH_DEFAULT,
                         .switchTransitionTimer =  SETTING_MIXER_SWITCH_TRANS_TIMER_DEFAULT,
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

void activateMixerConfig(){
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
    if (currentMixerConfig.PIDProfileLinking)
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
    mixerProfileAT.transitionStartTime = millis();
    mixerProfileAT.transitionTransEndTime = mixerProfileAT.transitionStartTime + (timeMs_t)currentMixerConfig.switchTransitionTimer * 100;
}

bool platformTypeConfigured(flyingPlatformType_e platformType)
{   
    if (!isModeActivationConditionPresent(BOXMIXERPROFILE)){
        return false;
    }
    return mixerConfigByIndex(nextMixerProfileIndex)->platformType == platformType;
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
}

bool mixerATUpdateState(mixerProfileATRequest_e required_action)
{   
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
            // LOG_INFO(PWM, "MIXERAT_PHASE_IDLE");
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
    if(cliMode) return;
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

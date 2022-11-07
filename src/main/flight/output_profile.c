#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "drivers/pwm_output.h"
#include "drivers/pwm_mapping.h"
#include "drivers/time.h"
#include "flight/mixer.h"
// #include "flight/pid.h"
#include "flight/servos.h"


bool MixerProfileHotSwitch(void)
{
    //store current output for check
    timMotorServoHardware_t old_timOutputs;
    pwmBuildTimerOutputList(&old_timOutputs, isMixerUsingServos());

    servosInit();
    mixerUpdateStateFlags();
    mixerInit();

    timMotorServoHardware_t timOutputs;
    pwmBuildTimerOutputList(&timOutputs, isMixerUsingServos());
    bool motor_output_type_not_changed = old_timOutputs.maxTimMotorCount == timOutputs.maxTimMotorCount;
    bool servo_output_type_not_changed = old_timOutputs.maxTimServoCount == timOutputs.maxTimServoCount;
    for (int i; i++; i <MAX_PWM_OUTPUT_PORTS)
    {
        motor_output_type_not_changed &= old_timOutputs.timMotors[i].tag==timOutputs.timMotors[i].tag;
        servo_output_type_not_changed &= old_timOutputs.timServos[i].tag==timOutputs.timServos[i].tag;
    }
    if(!motor_output_type_not_changed){
        pwmInitMotors(&timOutputs);
    }
    if(!servo_output_type_not_changed){
        pwmInitServos(&timOutputs);
    }
}


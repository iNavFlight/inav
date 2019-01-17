#include "common/maths.h"

#include "fc/rc_command.h"

#include "rx/rx.h"

void rcCommandReset(rcCommand_t *cmd)
{
    cmd->roll = 0;
    cmd->pitch = 0;
    cmd->yaw = 0;
    cmd->throttle = 0;
}

void rcCommandRotate(rcCommand_t *dst, const rcCommand_t *src, float radians)
{
    const float cosDiff = cos_approx(radians);
    const float sinDiff = sin_approx(radians);

    float pitch = src->pitch * cosDiff + src->roll * sinDiff;
    dst->roll = src->roll * cosDiff -  src->pitch * sinDiff;
    dst->pitch = pitch;
}

void rcCommandCopy(rcCommand_t *dst, const rcCommand_t *src)
{
    dst->roll = src->roll;
    dst->pitch = src->pitch;
    dst->yaw = src->yaw;
    dst->throttle = src->throttle;
}

float rcCommandMapPWMValue(int16_t value)
{
    return constrainf(scaleRangef(value, PWM_RANGE_MIN, PWM_RANGE_MAX, RC_COMMAND_MIN, RC_COMMAND_MAX), RC_COMMAND_MIN, RC_COMMAND_MAX);
}

int16_t rcCommandToPWMValue(float cmd)
{
    int16_t value = cmd * ((PWM_RANGE_MAX - PWM_RANGE_MIN) / RC_COMMAND_RANGE) + PWM_RANGE_MIDDLE;
    return constrain(value, PWM_RANGE_MIN, PWM_RANGE_MAX);
}

float rcCommandMapUnidirectionalPWMValue(int16_t value)
{
    return constrainf(scaleRangef(value, PWM_RANGE_MIN, PWM_RANGE_MAX, RC_COMMAND_CENTER, RC_COMMAND_MAX), RC_COMMAND_CENTER, RC_COMMAND_MAX);
}

float rcCommandConvertPWMDeadband(uint8_t deadband)
{
    // Deadband are specified as PWM units, which
    // have a range of 1000. To map them to [-1, 1]
    // we have a range of 2 so we divide by 500
    return deadband / 500.0f;
}
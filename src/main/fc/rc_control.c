#include <stdint.h>
#include <string.h>

#include "common/axis.h"
#include "common/maths.h"

#include "fc/controlrate_profile.h"
#include "fc/rc_control.h"
#include "fc/rc_controls.h"
#include "fc/rc_curves.h"
#include "fc/runtime_config.h"

#include "rx/rx.h"

static rcControl_t control;

static float getAxisRcCommand(float input, int16_t rate, float deadband)
{
    float constrained = constrainf(input, RC_COMMAND_MIN, RC_COMMAND_MAX);
    float stickDeflection = applyDeadbandf(constrained, deadband);

    return rcCurveApplyExpo(stickDeflection, rate);
}

static bool throttleIsBidirectional(void)
{
    return false;
}

void rcControlUpdateFromRX(void)
{
    const controlRateAxes_t *axes = FLIGHT_MODE(MANUAL_MODE) ? &currentControlRateProfile->manual : &currentControlRateProfile->stabilized;

    int16_t roll = rxGetChannelValue(ROLL);
    int16_t pitch = rxGetChannelValue(PITCH);
    int16_t yaw = rxGetChannelValue(YAW);
    int16_t throttle = rxGetChannelValue(THROTTLE);

    // Update input
    control.input.roll = rcCommandMapPWMValue(roll);
    control.input.pitch = rcCommandMapPWMValue(pitch);
    control.input.yaw = rcCommandMapPWMValue(yaw);
    if (throttleIsBidirectional()) {
        float deadband3d_throttle = rcCommandConvertPWMDeadband(rcControlsConfig()->deadband3d_throttle);
        control.input.throttle = applyDeadbandf(rcCommandMapPWMValue(throttle), deadband3d_throttle);
    } else {
        if (throttle < rxConfig()->mincheck) {
            control.input.throttle = 0;
        } else {
            // Normalize to from [MINCHECK;2000] to [1000;2000]
            int16_t normalizeThrottle = scaleRange(throttle, rxConfig()->mincheck, PWM_RANGE_MAX, PWM_RANGE_MIN, PWM_RANGE_MAX);
            control.input.throttle = rcCommandMapUnidirectionalPWMValue(normalizeThrottle);
        }
    }

    // Compute output ROLL PITCH and YAW command
    control.output.roll = getAxisRcCommand(control.input.roll, axes->rcExpo8, rcCommandConvertPWMDeadband(rcControlsConfig()->deadband));
    control.output.pitch = getAxisRcCommand(control.input.pitch, axes->rcExpo8, rcCommandConvertPWMDeadband(rcControlsConfig()->deadband));
    // Invert YAW, since GYRO/ACC measures CCW but we want input yaw
    // to produce CW rotation (i.e. stick to the right rotates to the right).
    control.output.yaw = -getAxisRcCommand(control.input.yaw, axes->rcYawExpo8, rcCommandConvertPWMDeadband(rcControlsConfig()->yaw_deadband));

    // Apply manual control rates
    if (FLIGHT_MODE(MANUAL_MODE)) {
        control.output.roll *= currentControlRateProfile->manual.rates[FD_ROLL] / 100L;
        control.output.pitch *= currentControlRateProfile->manual.rates[FD_PITCH] / 100L;
        control.output.yaw *= currentControlRateProfile->manual.rates[FD_YAW] / 100L;
    }

    // Compute output THROTTLE command
    control.output.throttle = rcCurveApplyThrottleExpo(control.input.throttle);

    // Set the initial source as the RX
    control.source = RC_CONTROL_SOURCE_RX;
}

const rcCommand_t *rcControlGetInput(void)
{
    return &control.input;
}

float rcControlGetInputAxis(rc_alias_e axis)
{
    if (axis >= ROLL && axis <= THROTTLE) {
        return control.input.axes[axis];
    }
    return 0;
}

float rcControlGetInputAxisApplyingPosholdDeadband(rc_alias_e axis)
{
    float deadband = rcCommandConvertPWMDeadband(rcControlsConfig()->pos_hold_deadband);
    return applyDeadbandf(rcControlGetInputAxis(axis), deadband) / (RC_COMMAND_MAX - deadband);
}

const rcCommand_t *rcControlGetOutput(void)
{
    return &control.output;
}

float rcControlGetOutputAxis(rc_alias_e axis)
{
    if (axis >= ROLL && axis <= THROTTLE) {
        return control.output.axes[axis];
    }
    return 0;
}

void rcControlUpdateOutput(const rcCommand_t *cmd, rcControlSource_e source)
{
    memcpy(&control.output, cmd, sizeof(*cmd));
    if (source > RC_CONTROL_SOURCE_KEEP) {
        control.source = source;
    }
}

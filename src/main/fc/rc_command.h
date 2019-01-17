#pragma once

#include <stdint.h>

#define RC_COMMAND_AXES_COUNT 4

#define RC_COMMAND_MIN -1.0f
#define RC_COMMAND_CENTER 0.0f
#define RC_COMMAND_MAX 1.0f
#define RC_COMMAND_RANGE (RC_COMMAND_MAX - RC_COMMAND_MIN)

typedef struct rcCommand_s {
    union {
        float axes[RC_COMMAND_AXES_COUNT];
        struct {
            float roll;
            float pitch;
            float yaw;
            float throttle;
        };
    };
} rcCommand_t;

// Sets all values to neutral
void rcCommandReset(rcCommand_t *cmd);

// Rotates the given command pitch and roll by the given angle in
// radians. Used mainly for HEADFREE mode.
void rcCommandRotate(rcCommand_t *dst, const rcCommand_t *src, float radians);
void rcCommandCopy(rcCommand_t *dst, const rcCommand_t *src);

float rcCommandMapPWMValue(int16_t value);
float rcCommandMapUnidirectionalPWMValue(int16_t value);

int16_t rcCommandToPWMValue(float cmd);


float rcCommandConvertPWMDeadband(uint8_t deadband);

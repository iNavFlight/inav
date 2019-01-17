#pragma once

#include "fc/rc_command.h"
#include "fc/rc_controls.h"

typedef enum {
    RC_CONTROL_SOURCE_KEEP = 0, // Keep the current source
    RC_CONTROL_SOURCE_RX,
    RC_CONTROL_SOURCE_HEADFREE,
    RC_CONTROL_SOURCE_FAILSAFE,
    RC_CONTROL_SOURCE_NAVIGATION,
} rcControlSource_e;

typedef struct rcControl_s {
    rcCommand_t input; // Input from user
    rcCommand_t output; // Output might be set by multiple subsystems, see rcCommandOutputSource_e
    rcControlSource_e source; // Source that set the output
} rcControl_t;

void rcControlUpdateFromRX(void);
const rcCommand_t *rcControlGetInput(void);
float rcControlGetInputAxis(rc_alias_e axis);
float rcControlGetInputAxisApplyingPosholdDeadband(rc_alias_e axis);
const rcCommand_t *rcControlGetOutput(void);
float rcControlGetOutputAxis(rc_alias_e axis);
void rcControlUpdateOutput(const rcCommand_t *cmd, rcControlSource_e source);

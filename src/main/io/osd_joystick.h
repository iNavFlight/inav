#pragma once

#include "config/parameter_group.h"

#ifdef USE_RCDEVICE 
#ifdef USE_LED_STRIP

typedef struct osdJoystickConfig_s {
    bool osd_joystick_enabled;
    uint8_t osd_joystick_down;
    uint8_t osd_joystick_up;
    uint8_t osd_joystick_left;
    uint8_t osd_joystick_right;
    uint8_t osd_joystick_enter;
} osdJoystickConfig_t;

PG_DECLARE(osdJoystickConfig_t, osdJoystickConfig);

bool osdJoystickEnabled(void);

// 5 key osd cable simulation
void osdJoystickSimulate5KeyButtonPress(uint8_t operation);
void osdJoystickSimulate5KeyButtonRelease(void);

#endif
#endif
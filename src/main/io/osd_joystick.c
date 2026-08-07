#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "common/crc.h"
#include "common/maths.h"
#include "common/streambuf.h"
#include "common/utils.h"

#include "build/build_config.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"
#include "fc/settings.h"
#include "fc/runtime_config.h"

#include "drivers/time.h"
#include "drivers/light_ws2811strip.h"

#include "io/serial.h"
#include "io/rcdevice.h"

#include "osd_joystick.h"

#ifdef USE_RCDEVICE
#ifdef USE_LED_STRIP


PG_REGISTER_WITH_RESET_TEMPLATE(osdJoystickConfig_t, osdJoystickConfig, PG_OSD_JOYSTICK_CONFIG, 0);

PG_RESET_TEMPLATE(osdJoystickConfig_t, osdJoystickConfig,
    .osd_joystick_enabled = SETTING_OSD_JOYSTICK_ENABLED_DEFAULT,
    .osd_joystick_down = SETTING_OSD_JOYSTICK_DOWN_DEFAULT,
    .osd_joystick_up = SETTING_OSD_JOYSTICK_UP_DEFAULT,
    .osd_joystick_left = SETTING_OSD_JOYSTICK_LEFT_DEFAULT,
    .osd_joystick_right = SETTING_OSD_JOYSTICK_RIGHT_DEFAULT,
    .osd_joystick_enter = SETTING_OSD_JOYSTICK_ENTER_DEFAULT
);

bool osdJoystickEnabled(void) {
    return osdJoystickConfig()->osd_joystick_enabled;
}


void osdJoystickSimulate5KeyButtonPress(uint8_t operation) {
    switch (operation) {
        case RCDEVICE_CAM_KEY_ENTER:
            ledPinStartPWM( osdJoystickConfig()->osd_joystick_enter );
            break;
        case RCDEVICE_CAM_KEY_LEFT:
            ledPinStartPWM( osdJoystickConfig()->osd_joystick_left );
            break;
        case RCDEVICE_CAM_KEY_UP:
            ledPinStartPWM( osdJoystickConfig()->osd_joystick_up );
            break;
        case RCDEVICE_CAM_KEY_RIGHT:
            ledPinStartPWM( osdJoystickConfig()->osd_joystick_right );
            break;
        case RCDEVICE_CAM_KEY_DOWN:
            ledPinStartPWM( osdJoystickConfig()->osd_joystick_down );
            break;
    }
}


void osdJoystickSimulate5KeyButtonRelease(void) {
    ledPinStopPWM();
}


#endif
#endif

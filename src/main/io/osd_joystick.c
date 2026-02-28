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
#include "drivers/pinio.h"

#include "io/serial.h"
#include "io/rcdevice.h"

#include "osd_joystick.h"

#ifdef USE_RCDEVICE
#ifdef USE_PINIO


PG_REGISTER_WITH_RESET_TEMPLATE(osdJoystickConfig_t, osdJoystickConfig, PG_OSD_JOYSTICK_CONFIG, 1);

PG_RESET_TEMPLATE(osdJoystickConfig_t, osdJoystickConfig,
    .osd_joystick_enabled = SETTING_OSD_JOYSTICK_ENABLED_DEFAULT,
    .pinio_channel = SETTING_OSD_JOYSTICK_PINIO_CHANNEL_DEFAULT,
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
    const int ch = osdJoystickConfig()->pinio_channel;
    switch (operation) {
        case RCDEVICE_CAM_KEY_ENTER:
            pinioSetDuty(ch, osdJoystickConfig()->osd_joystick_enter);
            break;
        case RCDEVICE_CAM_KEY_LEFT:
            pinioSetDuty(ch, osdJoystickConfig()->osd_joystick_left);
            break;
        case RCDEVICE_CAM_KEY_UP:
            pinioSetDuty(ch, osdJoystickConfig()->osd_joystick_up);
            break;
        case RCDEVICE_CAM_KEY_RIGHT:
            pinioSetDuty(ch, osdJoystickConfig()->osd_joystick_right);
            break;
        case RCDEVICE_CAM_KEY_DOWN:
            pinioSetDuty(ch, osdJoystickConfig()->osd_joystick_down);
            break;
    }
}


void osdJoystickSimulate5KeyButtonRelease(void) {
    pinioSetDuty(osdJoystickConfig()->pinio_channel, 0);
}


#endif
#endif

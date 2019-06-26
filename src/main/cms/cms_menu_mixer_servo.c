/* This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

//
// Mixer and servo menu
//

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "platform.h"

#ifdef USE_CMS

#include "common/utils.h"

#include "build/version.h"

#include "flight/mixer.h"
#include "flight/servos.h"

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms_menu_mixer_servo.h"

#include "fc/runtime_config.h"
#include "fc/settings.h"

static uint8_t currentMotorMixerIndex = 0;
static uint8_t tmpcurrentMotorMixerIndex = 1;
static uint16_t tmpMotorMixerThrottle;
static int16_t tmpMotorMixerRoll;
static int16_t tmpMotorMixerYaw;
static int16_t tmpMotorMixerPitch;
static uint8_t currentServoMixerIndex = 0;
static uint8_t tmpcurrentServoMixerIndex = 1;
static servoMixer_t tmpServoMixer;
static uint8_t currentServoIndex = 0;
static uint8_t oldServoIndex = 0;
static servoParam_t tmpServoParam;

static void loadServoSettings(void)
{
    tmpServoParam.middle = servoParams(currentServoIndex)->middle;
    tmpServoParam.min = servoParams(currentServoIndex)->min;
    tmpServoParam.max = servoParams(currentServoIndex)->max;
    tmpServoParam.rate = servoParams(currentServoIndex)->rate;
}

static void saveServoSettings(uint8_t idx)
{
    servoParamsMutable(idx)->middle = tmpServoParam.middle;
    servoParamsMutable(idx)->min = tmpServoParam.min;
    servoParamsMutable(idx)->max = tmpServoParam.max;
    servoParamsMutable(idx)->rate = tmpServoParam.rate;
}

static long cmsx_menuServo_onEnter(const OSD_Entry *from)
{
    UNUSED(from);

    loadServoSettings();

    return 0;
}

static long cmsx_menuServo_onExit(const OSD_Entry *from)
{
    UNUSED(from);

    saveServoSettings(currentServoIndex);

    return 0;
}

static long cmsx_menuServoIndexOnChange(displayPort_t *displayPort, const void *ptr)
{
    UNUSED(displayPort);
    UNUSED(ptr);

    saveServoSettings(oldServoIndex);
    loadServoSettings();
    oldServoIndex = currentServoIndex;

    return 0;
}

static const OSD_Entry cmsx_menuServoEntries[] =
{
       OSD_LABEL_ENTRY("-- SERVOS --"),
       OSD_UINT8_CALLBACK_ENTRY("SERVO", cmsx_menuServoIndexOnChange, (&(const OSD_UINT8_t){ &currentServoIndex, 0, MAX_SUPPORTED_SERVOS - 1, 1})),
       OSD_INT16_DYN_ENTRY("MID", (&(const OSD_INT16_t){&tmpServoParam.middle, 500, 2500, 1})),
       OSD_INT16_DYN_ENTRY("MIN", (&(const OSD_INT16_t){&tmpServoParam.min, 500, 2500, 1})),
       OSD_INT16_DYN_ENTRY("MAX", (&(const OSD_INT16_t){&tmpServoParam.max, 500, 2500, 1})),
       OSD_INT8_DYN_ENTRY("RATE", (&(const OSD_INT8_t){&tmpServoParam.rate, -125, 125, 1})),
       OSD_BACK_AND_END_ENTRY
};

const CMS_Menu cmsx_menuServo = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUSERVO",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = cmsx_menuServo_onEnter,
    .onExit = cmsx_menuServo_onExit,
    .onGlobalExit = NULL,
    .entries = cmsx_menuServoEntries
};

#define SERVO_MIXER_INPUT_CMS_NAMES_COUNT 23
const char * const servoMixerInputCmsNames[SERVO_MIXER_INPUT_CMS_NAMES_COUNT] = {
        "STAB ROLL",
        "STAB PITCH",
        "STAB YAW",
        "STAB THROTTLE",
        "RC ROLL",
        "RC PITCH",
        "RC YAW",
        "RC THROTTLE",
        "RC CHAN 5",
        "RC CHAN 6",
        "RC CHAN 7",
        "RC CHAN 8",
        "GIMB PITCH",
        "GIMB ROLL",
        "FLAPS",
        "RC CHAN 9",
        "RC CHAN 10",
        "RC CHAN 11",
        "RC CHAN 12",
        "RC CHAN 13",
        "RC CHAN 14",
        "RC CHAN 15",
        "RC CHAN 16"
};

static void loadServoMixerSettings(void)
{
    tmpServoMixer.targetChannel = customServoMixers(currentServoMixerIndex)->targetChannel;
    tmpServoMixer.inputSource = customServoMixers(currentServoMixerIndex)->inputSource;
    tmpServoMixer.rate = customServoMixers(currentServoMixerIndex)->rate;
    tmpServoMixer.speed = customServoMixers(currentServoMixerIndex)->speed;
}

static void saveServoMixerSettings(void)
{
    customServoMixersMutable(currentServoMixerIndex)->targetChannel = tmpServoMixer.targetChannel;
    customServoMixersMutable(currentServoMixerIndex)->inputSource = tmpServoMixer.inputSource;
    customServoMixersMutable(currentServoMixerIndex)->rate = tmpServoMixer.rate;
    customServoMixersMutable(currentServoMixerIndex)->speed = tmpServoMixer.speed;

}

static long cmsx_menuServoMixer_onEnter(const OSD_Entry *from)
{
    UNUSED(from);

    loadServoMixerSettings();

    return 0;
}

static long cmsx_menuServoMixer_onExit(const OSD_Entry *from)
{
    UNUSED(from);

    saveServoMixerSettings();

    return 0;
}

static long cmsx_menuServoMixerIndexOnChange(displayPort_t *displayPort, const void *ptr)
{
    UNUSED(displayPort);
    UNUSED(ptr);

    saveServoMixerSettings();
    currentServoMixerIndex = tmpcurrentServoMixerIndex - 1;
    loadServoMixerSettings();

    return 0;
}

static const OSD_Entry cmsx_menuServoMixerEntries[] =
{
       OSD_LABEL_ENTRY("-- SERVO MIXER --"),
       OSD_UINT8_CALLBACK_ENTRY("SERVO MIX", cmsx_menuServoMixerIndexOnChange, (&(const OSD_UINT8_t){ &tmpcurrentServoMixerIndex, 1, MAX_SERVO_RULES, 1})),
       OSD_UINT8_DYN_ENTRY("SERVO", (&(const OSD_UINT8_t){ &tmpServoMixer.targetChannel, 0, MAX_SUPPORTED_SERVOS, 1})),
       OSD_TAB_DYN_ENTRY("INPUT", (&(const OSD_TAB_t){ &tmpServoMixer.inputSource, SERVO_MIXER_INPUT_CMS_NAMES_COUNT - 1, servoMixerInputCmsNames})),
       OSD_INT16_DYN_ENTRY("WEIGHT", (&(const OSD_INT16_t){&tmpServoMixer.rate, 0, 1000, 1})),
       OSD_UINT8_DYN_ENTRY("SPEED", (&(const OSD_UINT8_t){&tmpServoMixer.speed, 0, 255, 1})),
       OSD_BACK_AND_END_ENTRY
};

const CMS_Menu cmsx_menuServoMixer = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUSERVOMIXER",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = cmsx_menuServoMixer_onEnter,
    .onExit = cmsx_menuServoMixer_onExit,
    .onGlobalExit = NULL,
    .entries = cmsx_menuServoMixerEntries
};

static void loadMotorMixerSettings(void)
{
    tmpMotorMixerThrottle = primaryMotorMixer(currentMotorMixerIndex)->throttle * 1000;
    tmpMotorMixerRoll = primaryMotorMixer(currentMotorMixerIndex)->roll * 1000;
    tmpMotorMixerPitch = primaryMotorMixer(currentMotorMixerIndex)->pitch * 1000;
    tmpMotorMixerYaw = primaryMotorMixer(currentMotorMixerIndex)->yaw * 1000;
}

static void saveMotorMixerSettings(void)
{
    primaryMotorMixerMutable(currentMotorMixerIndex)->throttle = tmpMotorMixerThrottle / 1000.0f;
    primaryMotorMixerMutable(currentMotorMixerIndex)->roll = tmpMotorMixerRoll / 1000.0f;
    primaryMotorMixerMutable(currentMotorMixerIndex)->pitch = tmpMotorMixerPitch / 1000.0f;
    primaryMotorMixerMutable(currentMotorMixerIndex)->yaw = tmpMotorMixerYaw / 1000.0f;
}

static long cmsx_menuMotorMixer_onEnter(const OSD_Entry *from)
{
    UNUSED(from);

    loadMotorMixerSettings();

    return 0;
}

static long cmsx_menuMotorMixer_onExit(const OSD_Entry *self)
{
    UNUSED(self);

    saveMotorMixerSettings();

    return 0;
}

static long cmsx_menuMotorMixerIndexOnChange(displayPort_t *displayPort, const void *ptr)
{
    UNUSED(displayPort);
    UNUSED(ptr);

    saveMotorMixerSettings();
    currentMotorMixerIndex = tmpcurrentMotorMixerIndex - 1;
    loadMotorMixerSettings();

    return 0;
}

static const OSD_Entry cmsx_motorMixerEntries[] =
{
        OSD_LABEL_ENTRY("-- MOTOR MIXER --"),
        OSD_UINT8_CALLBACK_ENTRY ("MOTOR", cmsx_menuMotorMixerIndexOnChange, (&(const OSD_UINT8_t){ &tmpcurrentMotorMixerIndex, 1, MAX_SUPPORTED_MOTORS, 1})),
        OSD_UINT16_DYN_ENTRY("THROTTLE", (&(const OSD_UINT16_t){ &tmpMotorMixerThrottle, 0, 1000, 1 })),
        OSD_INT16_DYN_ENTRY("ROLL", (&(const OSD_INT16_t){ &tmpMotorMixerRoll, -2000, 2000, 1 })),
        OSD_INT16_DYN_ENTRY("PITCH", (&(const OSD_INT16_t){ &tmpMotorMixerPitch, -2000, 2000, 1 })),
        OSD_INT16_DYN_ENTRY("YAW", (&(const OSD_INT16_t){ &tmpMotorMixerYaw, -2000, 2000, 1 })),
        OSD_BACK_AND_END_ENTRY
};

const CMS_Menu cmsx_menuMotorMixer = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUMMOTORMIXER",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = cmsx_menuMotorMixer_onEnter,
    .onExit = cmsx_menuMotorMixer_onExit,
    .onGlobalExit = NULL,
    .entries = cmsx_motorMixerEntries
};

static const OSD_Entry cmsx_mixerServoEntries[] =
{
        OSD_LABEL_ENTRY("-- MIXER AND SERVOS --"),
        OSD_SETTING_ENTRY("PLATFORM", SETTING_PLATFORM_TYPE),
        OSD_SETTING_ENTRY("HAS FLAPS", SETTING_HAS_FLAPS),
        OSD_SUBMENU_ENTRY("MOTOR MIXER", &cmsx_menuMotorMixer),
        OSD_SUBMENU_ENTRY("SERVO MIXER", &cmsx_menuServoMixer),
        OSD_SUBMENU_ENTRY("SERVOS", &cmsx_menuServo),
        OSD_BACK_AND_END_ENTRY
};

const CMS_Menu cmsx_menuMixerServo = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUMIXERSERVO",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = cmsx_mixerServoEntries
};

#endif

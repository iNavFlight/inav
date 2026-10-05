#pragma once
/*
 * Wire support types, copied verbatim from INAV by
 * generator/extract_wire_types.py. Do not edit, and do not reproduce these
 * from the schema: a bitfield struct and a compiler-padded struct cannot be
 * restated without guessing at a layout INAV already defines.
 *
 * A firmware build must NOT include this file -- it already has these from
 * sensors/esc_sensor.h, io/ledstrip.h and fc/rc_modes.h, and two declarations
 * of the same type conflict. msp_msgs.h therefore does not include it either.
 *
 * Everyone else (the Arduino library, standalone C users) includes this before
 * msp_msgs.h.
 */

#include <stdint.h>

#include "bitarray.h"   /* boxBitmask_t is a bitarray */


/* ---- from src/main/sensors/esc_sensor.h ---- */

typedef struct {
    uint8_t dataAge;
    int16_t temperature;
    int16_t voltage;
    int32_t current;
    uint32_t rpm;
} escSensorData_t;


/* ---- from src/main/io/ledstrip.h ---- */

#define LED_POS_BITCNT                  8

#define LED_FUNCTION_BITCNT             8

#define LED_OVERLAY_BITCNT              8

#define LED_COLOR_BITCNT                4

#define LED_DIRECTION_BITCNT            6

#define LED_PARAMS_BITCNT               6

typedef struct ledConfig_s {
    uint16_t led_position  : LED_POS_BITCNT;
    uint16_t led_function  : LED_FUNCTION_BITCNT;
    uint16_t led_overlay   : LED_OVERLAY_BITCNT;
    uint16_t led_color     : LED_COLOR_BITCNT;
    uint16_t led_direction : LED_DIRECTION_BITCNT;
    uint16_t led_params    : LED_PARAMS_BITCNT;
} __attribute__((packed)) ledConfig_t;


/* ---- from src/main/fc/rc_modes.h ---- */

typedef enum {
    BOXARM           = 0,
    BOXANGLE         = 1,
    BOXHORIZON       = 2,
    BOXNAVALTHOLD    = 3,    // old BOXBARO
    BOXHEADINGHOLD   = 4,    // old MAG
    BOXHEADFREE      = 5,
    BOXHEADADJ       = 6,
    BOXCAMSTAB       = 7,
    BOXNAVRTH        = 8,    // old GPSHOME
    BOXNAVPOSHOLD    = 9,    // old GPSHOLD
    BOXMANUAL        = 10,
    BOXBEEPERON      = 11,
    BOXLEDLOW        = 12,
    BOXLIGHTS        = 13,
    BOXNAVLAUNCH     = 14,
    BOXOSD           = 15,
    BOXTELEMETRY     = 16,
    BOXBLACKBOX      = 17,
    BOXFAILSAFE      = 18,
    BOXNAVWP         = 19,
    BOXAIRMODE       = 20,
    BOXHOMERESET     = 21,
    BOXGCSNAV        = 22,
    BOXSURFACE       = 24,
    BOXFLAPERON      = 25,
    BOXTURNASSIST    = 26,
    BOXAUTOTRIM      = 27,
    BOXAUTOTUNE      = 28,
    BOXCAMERA1       = 29,
    BOXCAMERA2       = 30,
    BOXCAMERA3       = 31,
    BOXOSDALT1       = 32,
    BOXOSDALT2       = 33,
    BOXOSDALT3       = 34,
    BOXNAVCOURSEHOLD = 35,
    BOXBRAKING       = 36,
    BOXUSER1         = 37,
    BOXUSER2         = 38,
    BOXFPVANGLEMIX   = 39,
    BOXLOITERDIRCHN  = 40,
    BOXMSPRCOVERRIDE = 41,
    BOXPREARM        = 42,
    BOXTURTLE        = 43,
    BOXNAVCRUISE     = 44,
    BOXAUTOLEVEL     = 45,
    BOXPLANWPMISSION = 46,
    BOXSOARING       = 47,
    BOXUSER3         = 48,
    BOXUSER4         = 49,
    BOXCHANGEMISSION = 50,
    BOXBEEPERMUTE    = 51,
    BOXMULTIFUNCTION = 52,
    BOXMIXERPROFILE  = 53,
    BOXMIXERTRANSITION = 54,
    BOXANGLEHOLD     = 55,
    BOXGIMBALTLOCK   = 56,
    BOXGIMBALRLOCK   = 57,
    BOXGIMBALCENTER  = 58,
    BOXGIMBALHTRK    = 59,
    BOXAUTOSPEED     = 60,
    BOXTERRAINAGLHOLD = 61,
    BOXINFLIGHTMENU  = 62,
    BOXTHRUSTREVERSE = 63,
    CHECKBOX_ITEM_COUNT
} boxId_e;

typedef struct boxBitmask_s { BITARRAY_DECLARE(bits, CHECKBOX_ITEM_COUNT); } boxBitmask_t;

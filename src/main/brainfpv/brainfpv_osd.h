
#ifndef BRAINFPV_OSD_H
#define BRAINFPV_OSD_H

#include <stdint.h>
#include "brainfpv/video.h"
#include "config/parameter_group.h"

typedef struct bfOsdConfig_s {
    uint8_t sync_threshold;
    uint8_t white_level;
    uint8_t black_level;
    int8_t x_offset;
    uint8_t x_scale;
    uint8_t sbs_3d_enabled;
    uint8_t sbs_3d_right_eye_offset;
    uint8_t font;
    uint8_t ir_system;
    uint16_t ir_trackmate_id;
    uint32_t ir_ilap_id;
    uint8_t ahi_steps;
    uint8_t altitude_scale;
    uint8_t speed_scale;
    uint16_t radar_max_dist_m;
    uint8_t sticks_display;
    uint8_t show_logo_on_arm;
    uint8_t show_pilot_logo;
    uint8_t invert;
    int8_t center_mark_offset;
} bfOsdConfig_t;

PG_DECLARE(bfOsdConfig_t, bfOsdConfig);

void brainFpvOsdInit(void);
void brainFpvOsdMain(void);
void resetBfOsdConfig(bfOsdConfig_t *bfOsdConfig);

void brainFpvOsdArtificialHorizon(void);
void brainFpvOsdCenterMark(void);
void brainFpvOsdUserLogo(uint16_t x, uint16_t y);
void brainFpvOsdMainLogo(uint16_t x, uint16_t y);
void brainFfpvOsdHomeArrow(int16_t home_dir, uint16_t x, uint16_t y);
void brainFpvRadarMap(void);
void brainFpvOsdHeadingGraph(uint16_t x, uint16_t y);

#endif /* BRAINFPV_OSD */

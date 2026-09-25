/*
 * This file is part of INAV.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define CHIRP_START_HZ 2.0f
#define CHIRP_END_HZ 60.0f
#define CHIRP_DURATION_US 20000000U
#define CHIRP_SETTLE_US 2000000U
#define CHIRP_MAX_INTERVAL_US 10000U

typedef enum {
    CHIRP_IDLE,
    CHIRP_READY,
    CHIRP_SETTLING,
    CHIRP_RUNNING,
    CHIRP_DONE,
    CHIRP_ABORTED,
} chirpPhase_e;

typedef enum {
    CHIRP_INHIBIT_NONE = 0,
    CHIRP_INHIBIT_FLIGHT = 1 << 0,
    CHIRP_INHIBIT_PILOT = 1 << 1,
    CHIRP_INHIBIT_LOGGING = 1 << 2,
    CHIRP_INHIBIT_MOTION = 1 << 3,
    CHIRP_INHIBIT_SATURATION = 1 << 4,
    CHIRP_INHIBIT_CONFIG = 1 << 5,
    CHIRP_INHIBIT_TIMING = 1 << 6,
    CHIRP_INHIBIT_SWITCH = 1 << 7,
} chirpInhibit_e;

typedef struct chirpState_s {
    chirpPhase_e phase;
    uint32_t startedAt;
    uint32_t updatedAt;
    uint16_t inhibit;
    float frequency;
    float angle;
    float output;
} chirpState_t;

// A low switch must be observed in valid flight conditions before every run.
// Neither booting with the switch high nor recovering from an abort starts a run.
void chirpUpdate(chirpState_t *state, uint32_t now, bool enabled, bool switchLow,
    bool switchHigh, uint16_t inhibit, float amplitude);

/*
 * This file is part of INAV.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "config/parameter_group.h"

typedef enum {
    CHIRP_AXIS_OFF,
    CHIRP_AXIS_ROLL,
    CHIRP_AXIS_PITCH,
    CHIRP_AXIS_YAW,
} chirpAxis_e;

typedef struct chirpConfig_s {
    uint8_t axis;
    uint8_t triggerChannel; // Receiver channel number, starting at 1 (5 = AUX1).
    uint8_t amplitude; // Peak rate perturbation in degrees/second.
} chirpConfig_t;

PG_DECLARE(chirpConfig_t, chirpConfig);

void chirpFlightUpdate(float dt, bool controllerReady);
float chirpApplyRate(int axis, float rate);
void chirpLogResponse(int axis, float setpoint, float measurement, float output);

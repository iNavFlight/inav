/*
 * This file is part of INAV.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <math.h>

#include "flight/chirp.h"

#define CHIRP_TWO_PI 6.28318530718f
#define CHIRP_LOG_FREQUENCY_RATIO 3.40119738166f // log(60 / 2)
#define CHIRP_FADE_US 1000000U

void chirpUpdate(chirpState_t *state, uint32_t now, bool enabled, bool switchLow,
    bool switchHigh, uint16_t inhibit, float amplitude)
{
    state->output = 0;

    if (!enabled) {
        *state = (chirpState_t){0};
        return;
    }

    if (!isfinite(amplitude) || amplitude <= 0 || amplitude > 30) {
        inhibit |= CHIRP_INHIBIT_CONFIG;
    }
    if (state->phase == CHIRP_SETTLING || state->phase == CHIRP_RUNNING) {
        if (now - state->updatedAt > CHIRP_MAX_INTERVAL_US) {
            inhibit |= CHIRP_INHIBIT_TIMING;
        }
    }

    if (inhibit) {
        state->phase = CHIRP_ABORTED;
        state->inhibit = inhibit;
        return;
    }

    if (switchLow) {
        *state = (chirpState_t){ .phase = CHIRP_READY, .updatedAt = now };
        return;
    }

    if (!switchHigh) {
        state->phase = CHIRP_ABORTED;
        state->inhibit = CHIRP_INHIBIT_SWITCH;
        return;
    }

    if (state->phase == CHIRP_READY) {
        state->phase = CHIRP_SETTLING;
        state->startedAt = now;
        state->updatedAt = now;
    } else if (state->phase == CHIRP_IDLE) {
        state->phase = CHIRP_ABORTED;
        state->inhibit = CHIRP_INHIBIT_SWITCH;
    }

    if (state->phase == CHIRP_SETTLING) {
        state->updatedAt = now;
        if (now - state->startedAt < CHIRP_SETTLE_US) {
            return;
        }
        state->phase = CHIRP_RUNNING;
        state->startedAt = now;
        state->frequency = CHIRP_START_HZ;
        state->angle = 0;
    }

    if (state->phase != CHIRP_RUNNING) {
        return;
    }

    const uint32_t elapsed = now - state->startedAt;
    if (elapsed >= CHIRP_DURATION_US) {
        state->phase = CHIRP_DONE;
        return;
    }

    const float frequency = CHIRP_START_HZ * expf(CHIRP_LOG_FREQUENCY_RATIO * elapsed / CHIRP_DURATION_US);
    const float dt = (now - state->updatedAt) * 1e-6f;
    // Integrate with bounded phase to avoid loss of precision late in the sweep.
    state->angle = fmodf(state->angle + CHIRP_TWO_PI * 0.5f * (state->frequency + frequency) * dt, CHIRP_TWO_PI);
    state->updatedAt = now;
    state->frequency = frequency;

    float envelope = 1;
    if (elapsed < CHIRP_FADE_US) {
        envelope = (float)elapsed / CHIRP_FADE_US;
    } else if (CHIRP_DURATION_US - elapsed < CHIRP_FADE_US) {
        envelope = (float)(CHIRP_DURATION_US - elapsed) / CHIRP_FADE_US;
    }
    state->output = amplitude * envelope * sinf(state->angle);
}

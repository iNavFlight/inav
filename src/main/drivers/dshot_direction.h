#pragma once

#include <stdbool.h>
#include <stdint.h>

// Configuration commands 7/8 change the stored direction; 20/21 are temporary
// overrides used by turtle mode. Never use the latter to configure an ESC.
typedef struct {
    uint32_t lastUs;
    uint8_t phase; // 0 idle, 1 stopping, 2 direction, 3 gap, 4 save, 5 settling, 6 sent
    uint8_t repeats;
    uint8_t motor;
    uint8_t reverse;
    uint8_t token;
    uint32_t testStartedUs;
    uint8_t testMotor;
    uint8_t testToken;
    bool testActive;
} dshotDirection_t;

static inline void dshotDirectionBegin(dshotDirection_t *s, uint32_t now, uint8_t motor, uint8_t reverse, uint8_t token)
{
    const uint8_t previousTestToken = s->testToken;
    *s = (dshotDirection_t){ .lastUs = now, .phase = 1, .motor = motor, .reverse = reverse,
        .token = token, .testToken = previousTestToken };
}

// Called only when the driver can transmit a frame. All unselected motors stay
// at zero during the operation, including between commands.
static inline int16_t dshotDirectionFrame(dshotDirection_t *s, uint32_t now)
{
    const uint32_t elapsed = now - s->lastUs;
    switch (s->phase) {
    case 1:
        if (elapsed >= 1000000) {
            s->phase = 2;
            s->lastUs = now;
        }
        break;
    case 2:
    case 4:
        if (elapsed >= 1000) {
            const uint8_t command = s->phase == 2 ? (s->reverse ? 8 : 7) : 12;
            s->lastUs = now;
            if (++s->repeats == 10) {
                s->phase++;
                s->repeats = 0;
            }
            return command;
        }
        return -1; // Do not interrupt consecutive command repetitions with zero frames.
    case 3:
        if (elapsed >= 10000) {
            s->phase = 4;
            s->lastUs = now;
        }
        break;
    case 5:
        if (elapsed >= 35000) {
            s->phase = 6;
        }
        break;
    default:
        break;
    }
    return 0;
}

// A finite pulse, not a latched throttle command: loss of USB/UI cannot leave
// a motor running. Duplicate requests never extend or restart a pulse.
static inline bool dshotDirectionTestBegin(dshotDirection_t *s, uint32_t now, uint8_t motor, uint8_t token)
{
    if (token == 0 || (s->phase > 0 && s->phase < 6)) return false;
    if (token == s->testToken) return motor == s->testMotor;
    if (s->testActive) return false;
    s->testStartedUs = now;
    s->testMotor = motor;
    s->testToken = token;
    s->testActive = true;
    return true;
}

static inline uint16_t dshotDirectionTestFrame(dshotDirection_t *s, uint32_t now)
{
    if (s->testActive && (uint32_t)(now - s->testStartedUs) >= 1500000) s->testActive = false;
    // Fixed low test output (~3.6% of the DShot throttle range); no arbitrary
    // throttle value is accepted over this configuration-only interface.
    return s->testActive ? 120 : 0;
}

static inline bool dshotDirectionBusy(const dshotDirection_t *s)
{
    return s->testActive || (s->phase > 0 && s->phase < 6);
}

// Arming remains governed by the existing flight-controller rules. Never let a
// configuration operation replace armed motor output; retain tokens so retries
// cannot restart the cancelled operation after disarming.
static inline void dshotDirectionCancel(dshotDirection_t *s)
{
    s->phase = 0;
    s->repeats = 0;
    s->testActive = false;
}

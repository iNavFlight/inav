#include <assert.h>
#include <stdio.h>
#include "drivers/dshot_direction.h"

static void sequence(uint32_t start, uint8_t reverse, uint32_t step)
{
    dshotDirection_t s = {0};
    assert(dshotDirectionFrame(&s, start) == 0);
    dshotDirectionBegin(&s, start, 2, reverse, 3);
    unsigned direction = 0, save = 0;
    uint32_t lastDirection = 0, lastSave = 0;
    for (uint32_t elapsed = 0; elapsed < 2000000; elapsed += step) {
        int16_t frame = dshotDirectionFrame(&s, start + elapsed);
        assert(frame == -1 || frame == 0 || frame == (reverse ? 8 : 7) || frame == 12);
        if (frame == (reverse ? 8 : 7)) {
            assert(elapsed >= 1000000);
            assert(save == 0);
            if (direction) assert(elapsed - lastDirection >= 1000);
            lastDirection = elapsed;
            direction++;
        }
        if (frame == 12) {
            assert(direction == 10);
            assert(elapsed - lastDirection >= 10000);
            if (save) assert(elapsed - lastSave >= 1000);
            lastSave = elapsed;
            save++;
        }
        if (frame == 0) {
            // No zero frame may break a group of repeated commands.
            assert(direction == 0 || direction == 10);
            assert(save == 0 || save == 10);
        }
        if (s.phase == 6) {
            assert(elapsed - lastSave >= 35000);
        }
    }
    assert(direction == 10 && save == 10 && s.phase == 6);
    assert(s.motor == 2 && s.reverse == reverse && s.token == 3);
    assert(dshotDirectionFrame(&s, start + 3000000) == 0);
}
static void pulse(uint32_t start)
{
    dshotDirection_t s = {0};
    assert(!dshotDirectionTestBegin(&s, start, 0, 0));
    assert(dshotDirectionTestBegin(&s, start, 2, 1));
    assert(dshotDirectionBusy(&s) && s.testActive);
    assert(dshotDirectionTestFrame(&s, start) == 120);
    assert(!dshotDirectionTestBegin(&s, start + 1000, 1, 2));
    assert(dshotDirectionTestBegin(&s, start + 1000000, 2, 1));
    assert(dshotDirectionTestFrame(&s, start + 1499999) == 120);
    assert(dshotDirectionTestFrame(&s, start + 1500000) == 0);
    assert(dshotDirectionTestBegin(&s, start + 1600000, 2, 1));
    assert(!s.testActive); // Retry cannot restart an expired pulse.
    assert(dshotDirectionTestBegin(&s, start + 1700000, 1, 2));
    s.testActive = false; // Explicit release/close.
    assert(dshotDirectionTestBegin(&s, start + 1800000, 1, 2));
    assert(!s.testActive);
    dshotDirectionBegin(&s, start + 1800000, 1, 1, 1);
    assert(!dshotDirectionTestBegin(&s, start + 1800000, 1, 3));
    assert(s.testToken == 2);
    dshotDirectionCancel(&s);
    assert(!dshotDirectionBusy(&s));
    assert(dshotDirectionFrame(&s, start + 1900000) == 0);
    assert(dshotDirectionTestFrame(&s, start + 1900000) == 0);
    assert(s.token == 1 && s.testToken == 2);
    assert(dshotDirectionTestBegin(&s, start + 2000000, 0, 3));
    dshotDirectionCancel(&s);
    assert(!dshotDirectionBusy(&s));
    assert(dshotDirectionTestBegin(&s, start + 2100000, 0, 3));
    assert(!s.testActive); // A delayed retry cannot restart an aborted pulse.
}
int main(void)
{
    for (uint8_t reverse = 0; reverse < 2; reverse++) {
        sequence(0, reverse, 125);
        sequence(UINT32_MAX - 500000, reverse, 500);
        sequence(0, reverse, 2300);
    }
    pulse(0);
    pulse(UINT32_MAX - 500000);
    puts("DShot direction: sequence, timing, repetition, completion and clock wrap tests passed");
}

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
static void pulseRequestAtDeadline(uint32_t start)
{
    dshotDirection_t s = {0};
    assert(dshotDirectionTestBegin(&s, start, 2, 1));
    assert(!dshotDirectionTestBegin(&s, start + 1499999, 1, 2));
    // No output update occurs between the requests.
    assert(dshotDirectionTestBegin(&s, start + 1500000, 1, 2));
    assert(s.testActive && s.testMotor == 1 && s.testToken == 2);
    assert(s.testStartedUs == start + 1500000);
    // A duplicate token after expiry must acknowledge without restarting.
    assert(dshotDirectionTestBegin(&s, start + 3000000, 1, 2));
    assert(!s.testActive && s.testStartedUs == start + 1500000);
    assert(!dshotDirectionTestBegin(&s, start + 3000001, 2, 2));
    assert(dshotDirectionTestBegin(&s, start + 3000002, 2, 3));
    // Status uses the same expiry helper and must retain the last token.
    assert(!dshotDirectionExpireTest(&s, start + 4500001));
    assert(dshotDirectionExpireTest(&s, start + 4500002));
    assert(!dshotDirectionBusy(&s) && s.testToken == 3 && s.testMotor == 2);
    assert(!dshotDirectionExpireTest(&s, start + 4500003));
    assert(dshotDirectionTestBegin(&s, start + 4500004, 2, 3));
    assert(!s.testActive);
}

static void outputOwnership(void)
{
    dshotDirection_t s = {0};
    const uint16_t normal[4] = {0, 0, 0, 0};
    assert(dshotDirectionTestBegin(&s, 0, 2, 1));
    dshotDirectionOutput_t output = dshotDirectionOutput(&s, 1, false);
    assert(output.active && output.ready && !output.telemetry);
    for (uint8_t i = 0; i < 4; i++) {
        assert(dshotDirectionMotorValue(&output, i, normal[i]) == (i == 2 ? 120 : 0));
        assert(normal[i] == 0); // Preparing a frame never overwrites normal output.
    }
    s.testActive = false;
    output = dshotDirectionOutput(&s, 2, false);
    assert(!output.active && dshotDirectionMotorValue(&output, 2, normal[2]) == 0);
    assert(dshotDirectionTestBegin(&s, 3, 2, 2));
    output = dshotDirectionOutput(&s, 4, true);
    assert(!output.active && !dshotDirectionBusy(&s));
    assert(dshotDirectionMotorValue(&output, 2, 456) == 456); // Fresh armed mixer wins.
    assert(dshotDirectionTestBegin(&s, 5, 2, 3));
    output = dshotDirectionOutput(&s, 1500005, false);
    assert(output.active && output.value == 0); // Expiry emits a final zero frame.
    output = dshotDirectionOutput(&s, 1500006, false);
    assert(!output.active);
    dshotDirectionBegin(&s, 2000000, 1, 1, 1);
    output = dshotDirectionOutput(&s, 3000000, false);
    output = dshotDirectionOutput(&s, 3001000, false);
    assert(output.telemetry && dshotDirectionMotorValue(&output, 1, 0) == 8);
    output = dshotDirectionOutput(&s, 3001001, false);
    assert(!output.ready); // Suppress intervening zero frames during repeats.
    output = dshotDirectionOutput(&s, 3001002, true);
    assert(!output.active && dshotDirectionMotorValue(&output, 1, 321) == 321);
}

int main(void)
{
    outputOwnership();
    for (uint8_t reverse = 0; reverse < 2; reverse++) {
        sequence(0, reverse, 125);
        sequence(UINT32_MAX - 500000, reverse, 500);
        sequence(0, reverse, 2300);
    }
    pulseRequestAtDeadline(0);
    pulseRequestAtDeadline(UINT32_MAX - 500000);
    pulse(0);
    pulse(UINT32_MAX - 500000);
    puts("DShot direction: sequence, timing, repetition, completion and clock wrap tests passed");
}

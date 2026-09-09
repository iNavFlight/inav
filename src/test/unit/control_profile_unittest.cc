/*
 * This file is part of Cleanflight.
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

#include <stdint.h>

#include "gtest/gtest.h"

extern "C" {
#include "fc/control_profile.h"

// control_profile.c's activateControlConfig() calls generateThrottleCurve()
// (declared in fc/rc_curves.h). That real implementation lives in
// fc/rc_curves.c, which pulls in flight/mixer.c, rx/rx.c and a large chunk
// of the mixer/RX subsystem -- none of which is exercised by this test
// (we never call activateControlConfig()/changeControlProfile()). Provide a
// trivial stand-in so the real, unmodified fc/control_profile.c still links.
struct controlConfig_s;
void generateThrottleCurve(const struct controlConfig_s *controlConfig)
{
    (void)controlConfig;
}
}

// This test exercises the real (unmodified) fc/control_profile.c.
TEST(ControlProfileTest, GetCurrentControlProfileTracksSetControlProfile)
{
    for (uint8_t index = 0; index < MAX_CONTROL_PROFILE_COUNT; index++) {
        setControlProfile(index);
        EXPECT_EQ(getCurrentControlProfile(), index)
            << "getCurrentControlProfile() should report the profile index "
               "most recently passed to setControlProfile()";
    }
}

TEST(ControlProfileTest, SetControlProfileClampsOutOfRangeIndexToZero)
{
    // setControlProfile() clamps any index >= MAX_CONTROL_PROFILE_COUNT to 0
    // (see fc/control_profile.c). getCurrentControlProfile() should reflect
    // that clamped value, not the out-of-range value that was requested.
    setControlProfile(1);
    ASSERT_EQ(getCurrentControlProfile(), 1);

    setControlProfile(MAX_CONTROL_PROFILE_COUNT);
    EXPECT_EQ(getCurrentControlProfile(), 0);
}

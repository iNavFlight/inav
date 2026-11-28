#include <gtest/gtest.h>

extern "C" {
#include "navigation/navigation_fixedwing_flight_tally_logic.h"
}

// GPS heading 3, airspeed 3, baro altitude change 2; velocity counts 2 only
// when GPS heading is not valid (it comes from the same GPS measurement).

static int8_t tally(bool gpsHeading, float airspeed, float vel3D, float baroChange)
{
    return fwFlightTallyCompute(gpsHeading, airspeed, vel3D, baroChange);
}

TEST(FwFlightTally, NoSignalsIsZero)
{
    EXPECT_EQ(0, tally(false, 0, 0, 0));
}

TEST(FwFlightTally, GpsHeadingAndVelocityDoNotDoubleCount)
{
    // Weak/wandering GPS can report heading validity and velocity together;
    // that is one measurement and must not reach the autotrim bar of 5.
    EXPECT_EQ(3, tally(true, 0, 1000, 0));
}

TEST(FwFlightTally, VelocityCountsWhenGpsHeadingInvalid)
{
    EXPECT_EQ(2, tally(false, 0, 400, 0));
}

TEST(FwFlightTally, GpsPlusBaroChangeReachesFive)
{
    EXPECT_EQ(5, tally(true, 0, 400, 1000));
}

TEST(FwFlightTally, AirspeedPlusBaroChangeReachesFive)
{
    EXPECT_EQ(5, tally(false, 1000, 0, 1000));
}

TEST(FwFlightTally, AirspeedPlusVelocityDuringGpsLossReachesFive)
{
    EXPECT_EQ(5, tally(false, 1000, 400, 0));
}

TEST(FwFlightTally, GpsPlusAirspeedReachesSix)
{
    EXPECT_EQ(6, tally(true, 1000, 400, 0));
}

TEST(FwFlightTally, AllSignalsIsEight)
{
    EXPECT_EQ(8, tally(true, 1000, 400, 1000));
}

TEST(FwFlightTally, NoPitotNeverReachesSix)
{
    EXPECT_LE(tally(true, 0, 1000, 1000), 5);
}

TEST(FwFlightTally, BaroChangeBelowFiveMetersIgnored)
{
    EXPECT_EQ(0, tally(false, 0, 0, 499.0f));
    EXPECT_EQ(0, tally(false, 0, 0, 500.0f));
}

TEST(FwFlightTally, BaroDescentCountsLikeClimb)
{
    // Launch off a cliff can end below the arming altitude.
    EXPECT_EQ(2, tally(false, 0, 0, -600.0f));
}

TEST(FwFlightTally, SpeedThresholdsAreStrict)
{
    EXPECT_EQ(0, tally(false, FW_FLIGHT_MIN_AIRSPEED_CMS, FW_FLIGHT_MIN_VEL_3D_CMS, 0));
    EXPECT_EQ(5, tally(false, FW_FLIGHT_MIN_AIRSPEED_CMS + 1, FW_FLIGHT_MIN_VEL_3D_CMS + 1, 0));
}

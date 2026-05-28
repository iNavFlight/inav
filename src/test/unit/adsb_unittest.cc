/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "gtest/gtest.h"
#include "unittest_macros.h"

extern "C" {
#include "io/adsb.h"
// navigation_private.h uses _Static_assert (C11 keyword); map it to the C++
// equivalent before inclusion so this translation unit compiles cleanly.
#define _Static_assert static_assert
#include "navigation/navigation_private.h"
#undef _Static_assert
#include "io/osd.h"

// Expose the internal vehicle dictionary for direct test setup
extern adsbVehicle_t adsbVehiclesDictionary[];

// ---------------------------------------------------------------------------
// Globals required by adsb.c (and headers it pulls in)
// ---------------------------------------------------------------------------

gpsSolutionData_t     gpsSol;
gpsLocation_t         GPS_home;
uint32_t              GPS_distanceToHome;
int16_t               GPS_directionToHome;
bool                  autoThrottleManuallyIncreased;
uint32_t              stateFlags;
navigationPosControl_t posControl;

// PG system globals for osdConfig (accessed via inline osdConfig() from io/osd.h)
osdConfig_t osdConfig_System;
osdConfig_t osdConfig_Copy;

// ---------------------------------------------------------------------------
// Stub functions called by adsb.c
// ---------------------------------------------------------------------------

bool geoConvertGeodeticToLocal(fpVector3_t *pos, const gpsOrigin_t *origin,
                               const gpsLocation_t *llh,
                               geoAltitudeConversionMode_e altConv)
{
    (void)origin; (void)llh; (void)altConv;
    pos->x = 0.0f;
    pos->y = 0.0f;
    pos->z = 0.0f;
    return true;
}

uint32_t calculateDistanceToDestination(const fpVector3_t *destinationPos)
{
    (void)destinationPos;
    return 0;
}

int32_t calculateBearingToDestination(const fpVector3_t *destinationPos)
{
    (void)destinationPos;
    return 0;
}

float getEstimatedActualPosition(int axis)
{
    (void)axis;
    return 0.0f;
}

} // extern "C"

// ---------------------------------------------------------------------------
// Common test distances (centimetres unless noted)
// ---------------------------------------------------------------------------
static const uint32_t ALERT_CM   = 50000;   //  500 m
static const uint32_t WARNING_CM = 200000;  // 2000 m
static const int32_t  VERT_CM    = 10000;   //  100 m

// ---------------------------------------------------------------------------
// Base fixture: resets the vehicle dictionary and osdConfig before each test
// ---------------------------------------------------------------------------
class AdsbTest : public ::testing::Test {
protected:
    void SetUp() override {
        memset(adsbVehiclesDictionary, 0, sizeof(adsbVehicle_t) * MAX_ADSB_VEHICLES);
        memset(&osdConfig_System, 0, sizeof(osdConfig_System));
    }

    // Helper: populate one slot with pre-calculated values so tests bypass
    // adsbNewVehicle / recalculateVehicle entirely.
    void setVehicle(int idx, uint32_t distCm, int32_t vertDistCm,
                    int32_t meetDistM = 0, int32_t meetTimeSec = 0)
    {
        adsbVehicle_t *v = &adsbVehiclesDictionary[idx];
        v->ttl = 10;
        v->calculatedVehicleValues.valid           = true;
        v->calculatedVehicleValues.dist            = distCm;
        v->calculatedVehicleValues.verticalDistance = vertDistCm;
        v->calculatedVehicleValues.meetPointDistance = meetDistM;
        v->calculatedVehicleValues.meetPointTime    = meetTimeSec;
    }
};

// ===========================================================================
// Scenario 1 – No CPA  (adsb_calculation_use_cpa = false)
//
// Sub-scenarios: warning, no alert, alert
// ===========================================================================

class AdsbNoCPATest : public AdsbTest {
protected:
    void SetUp() override {
        AdsbTest::SetUp();
        osdConfig_System.adsb_calculation_use_cpa = false;
    }
};

// --- Sub-scenario: warning ---------------------------------------------------
// A vehicle whose current distance is inside the warning zone triggers a
// warning through findVehicleForWarning().

TEST_F(AdsbNoCPATest, Warning_VehicleInWarningZone)
{
    setVehicle(0, WARNING_CM - 1, 0);
    adsbVehicle_t *result = findVehicleForWarning(WARNING_CM, VERT_CM);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result, &adsbVehiclesDictionary[0]);
}

TEST_F(AdsbNoCPATest, Warning_VerticalFilterExcludesVehicle)
{
    // Vehicle is within warning distance but too high → filtered out
    setVehicle(0, WARNING_CM / 2, VERT_CM + 1);
    EXPECT_EQ(findVehicleForWarning(WARNING_CM, VERT_CM), nullptr);
}

TEST_F(AdsbNoCPATest, Warning_NoActiveVehicles)
{
    EXPECT_EQ(findVehicleForWarning(WARNING_CM, VERT_CM), nullptr);
}

// --- Sub-scenario: no alert --------------------------------------------------
// With CPA disabled, a vehicle that is in the warning zone but outside the
// alert zone must NOT trigger an alert.

TEST_F(AdsbNoCPATest, NoAlert_VehicleInWarningZoneOnly)
{
    // Between alert and warning thresholds: no CPA → no alert
    setVehicle(0, (ALERT_CM + WARNING_CM) / 2, 0);
    EXPECT_EQ(findVehicleForAlert(ALERT_CM, WARNING_CM, VERT_CM), nullptr);
}

TEST_F(AdsbNoCPATest, NoAlert_VehicleBeyondWarningZone)
{
    setVehicle(0, WARNING_CM + 1, 0);
    EXPECT_EQ(findVehicleForAlert(ALERT_CM, WARNING_CM, VERT_CM), nullptr);
}

TEST_F(AdsbNoCPATest, NoAlert_NoActiveVehicles)
{
    EXPECT_EQ(findVehicleForAlert(ALERT_CM, WARNING_CM, VERT_CM), nullptr);
}

// --- Sub-scenario: alert -----------------------------------------------------
// A vehicle already inside the alert distance must be returned immediately.

TEST_F(AdsbNoCPATest, Alert_VehicleInsideAlertDistance)
{
    setVehicle(0, ALERT_CM - 1, 0);
    adsbVehicle_t *result = findVehicleForAlert(ALERT_CM, WARNING_CM, VERT_CM);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result, &adsbVehiclesDictionary[0]);
}

TEST_F(AdsbNoCPATest, Alert_VerticalFilterExcludesVehicle)
{
    // Vehicle is inside alert distance but above vertical limit → no alert
    setVehicle(0, ALERT_CM / 2, VERT_CM + 1);
    EXPECT_EQ(findVehicleForAlert(ALERT_CM, WARNING_CM, VERT_CM), nullptr);
}

TEST_F(AdsbNoCPATest, Alert_InactiveVehicleIgnored)
{
    // Vehicle with ttl = 0 must be invisible to all queries
    adsbVehiclesDictionary[0].ttl = 0;
    adsbVehiclesDictionary[0].calculatedVehicleValues.valid = true;
    adsbVehiclesDictionary[0].calculatedVehicleValues.dist  = ALERT_CM / 2;
    EXPECT_EQ(findVehicleForAlert(ALERT_CM, WARNING_CM, VERT_CM), nullptr);
    EXPECT_EQ(findVehicleForWarning(WARNING_CM, VERT_CM), nullptr);
}

// ===========================================================================
// Scenario 2 – CPA  (adsb_calculation_use_cpa = true)
//
// Closest Point of Approach (CPA) extends threat detection: a vehicle
// currently outside the alert zone may still be flagged if its trajectory
// will bring it inside that zone.
//
// Sub-scenarios: warning, no alert, alert
// ===========================================================================

class AdsbCPATest : public AdsbTest {
protected:
    void SetUp() override {
        AdsbTest::SetUp();
        osdConfig_System.adsb_calculation_use_cpa = true;
    }
};

// --- Sub-scenario: warning ---------------------------------------------------
// findVehicleForWarning() checks only current distance, not CPA projection.

TEST_F(AdsbCPATest, Warning_VehicleInWarningZone)
{
    // CPA data present but irrelevant – current distance triggers warning
    setVehicle(0, WARNING_CM - 1, 0, /*meetDistM=*/1000, /*meetTimeSec=*/120);
    adsbVehicle_t *result = findVehicleForWarning(WARNING_CM, VERT_CM);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result, &adsbVehiclesDictionary[0]);
}

TEST_F(AdsbCPATest, Warning_CPACloseButCurrentDistanceBeyondWarning)
{
    // CPA would be within alert range, but the vehicle is too far right now
    setVehicle(0, WARNING_CM + 1, 0, /*meetDistM=*/10, /*meetTimeSec=*/30);
    EXPECT_EQ(findVehicleForWarning(WARNING_CM, VERT_CM), nullptr);
}

TEST_F(AdsbCPATest, Warning_VerticalFilterExcludesVehicle)
{
    setVehicle(0, WARNING_CM / 2, VERT_CM + 1);
    EXPECT_EQ(findVehicleForWarning(WARNING_CM, VERT_CM), nullptr);
}

// --- Sub-scenario: no alert --------------------------------------------------

TEST_F(AdsbCPATest, NoAlert_CPAOutsideAlertZone)
{
    // Vehicle is in warning zone, but its CPA stays outside the alert zone
    uint32_t distInWarningZone = (ALERT_CM + WARNING_CM) / 2;
    // meetPointDistance in metres; multiply by 100 gives cm.
    // Choose a distance clearly beyond the alert threshold (in cm).
    int32_t meetDistM = static_cast<int32_t>(ALERT_CM / 100) + 100; // alert_cm/100 m + 100 m
    setVehicle(0, distInWarningZone, 0, meetDistM, 60);
    EXPECT_EQ(findVehicleForAlert(ALERT_CM, WARNING_CM, VERT_CM), nullptr);
}

TEST_F(AdsbCPATest, NoAlert_VehicleBeyondWarningZone)
{
    // Too far for CPA consideration (dist > warningDistanceCm)
    int32_t meetDistM = static_cast<int32_t>(ALERT_CM / 100) - 10; // would be inside alert
    setVehicle(0, WARNING_CM + 1, 0, meetDistM, 30);
    EXPECT_EQ(findVehicleForAlert(ALERT_CM, WARNING_CM, VERT_CM), nullptr);
}

TEST_F(AdsbCPATest, NoAlert_NoActiveVehicles)
{
    EXPECT_EQ(findVehicleForAlert(ALERT_CM, WARNING_CM, VERT_CM), nullptr);
}

TEST_F(AdsbCPATest, NoAlert_VerticalFilterExcludesVehicle)
{
    // Vehicle is inside alert distance but above the vertical limit
    setVehicle(0, ALERT_CM / 2, VERT_CM + 1);
    EXPECT_EQ(findVehicleForAlert(ALERT_CM, WARNING_CM, VERT_CM), nullptr);
}

// --- Sub-scenario: alert -----------------------------------------------------

TEST_F(AdsbCPATest, Alert_VehicleAlreadyInsideAlertZone)
{
    // Immediate alert: dist <= alertDistanceCm (timeToAlert = 0)
    setVehicle(0, ALERT_CM / 2, 0, /*meetDistM=*/100, /*meetTimeSec=*/30);
    adsbVehicle_t *result = findVehicleForAlert(ALERT_CM, WARNING_CM, VERT_CM);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result, &adsbVehiclesDictionary[0]);
}

TEST_F(AdsbCPATest, Alert_ViaCPAProjection)
{
    // Vehicle is in the warning zone (outside alert), but its CPA will bring
    // it inside the alert zone → alert via CPA
    uint32_t distInWarningZone = (ALERT_CM + WARNING_CM) / 2;
    // meetPointDistance (m) × 100 must be <= ALERT_CM (cm)
    int32_t meetDistM = static_cast<int32_t>(ALERT_CM / 100) - 1; // just inside alert
    setVehicle(0, distInWarningZone, 0, meetDistM, 60);
    adsbVehicle_t *result = findVehicleForAlert(ALERT_CM, WARNING_CM, VERT_CM);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result, &adsbVehiclesDictionary[0]);
}

TEST_F(AdsbCPATest, Alert_CloserVehiclePrioritisedOverFarther)
{
    // Two vehicles both inside alert zone: the closer one (idx=1) must be
    // returned because findVehicleForAlert picks the smallest dist when
    // timeToAlert is equal (both 0).
    setVehicle(0, ALERT_CM - 2000, 0);  // farther
    setVehicle(1, ALERT_CM - 5000, 0);  // closer
    adsbVehicle_t *result = findVehicleForAlert(ALERT_CM, WARNING_CM, VERT_CM);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result, &adsbVehiclesDictionary[1]);
}

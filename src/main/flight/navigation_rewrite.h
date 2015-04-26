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

#pragma once

#include "common/axis.h"
#include "common/maths.h"

#include "flight/pid.h"

#include "sensors/barometer.h"

#include "io/rc_controls.h"
#include "io/escservo.h"

#define NAV
#if defined(BLACKBOX)
#define NAV_BLACKBOX
#endif

// Maximum number of waypoints, special waypoint 0 = home,
#define NAV_MAX_WAYPOINTS       15

// navigation mode
typedef enum navigationMode_e {
    NAV_MODE_NONE       = 0,
    NAV_MODE_ALTHOLD,
    NAV_MODE_POSHOLD_2D,
    NAV_MODE_POSHOLD_3D,
    NAV_MODE_WP,
    NAV_MODE_RTH,
    NAV_MODE_RTH_2D,
} navigationMode_t;

enum {
    NAV_GPS_ATTI    = 0,                    // Pitch/roll stick controls attitude (pitch/roll lean angles)
    NAV_GPS_CRUISE  = 1                     // Pitch/roll stick controls velocity (forward/right speed)
};

enum {
    NAV_RTH_NO_ALT          = 0,            // Maintain current altitude
    NAX_RTH_EXTRA_ALT       = 1,            // Maintain current altitude + predefined safety margin
    NAV_RTH_CONST_ALT       = 2,            // Climb/descend to predefined altitude
    NAV_RTH_MAX_ALT         = 3,            // Track maximum altitude and climb to it when RTH
    NAV_RTH_AT_LEAST_ALT    = 4,            // Climb to predefined altitude if below it
};

typedef enum {
    RTH_IDLE = 0,               // RTH is waiting
    RTH_IN_PROGRESS_OK,         // RTH is active
    RTH_IN_PROGRESS_LOST_GPS,   // RTH is active but has lost GPS lock
    RTH_HAS_LANDED              // RTH is active and has landed.
} rthState_e;

typedef struct navConfig_s {
    struct {
        uint8_t use_midrc_for_althold;      // Don't remember throttle when althold was initiated, assume that throttle is at middle = zero climb rate
        uint8_t throttle_tilt_comp;         // Calculate and use automatic throttle tilt compensation
        uint8_t lock_nav_until_takeoff;     // Easy mode, NAV won't mess up with controls on the ground
        uint8_t user_control_mode;          // NAV_GPS_ATTI or NAV_GPS_CRUISE
        uint8_t rth_alt_control_style;      // Controls how RTH controls altitude
    } flags;

    struct {
        uint8_t enable_dead_reckoning;
        uint16_t gps_delay_ms;

        float w_z_baro_p;   // Weight (cutoff frequency) for barometer altitude measurements
        float w_z_baro_v;   // Weight (cutoff frequency) for barometer velocity measurements

        float w_z_sonar_p;  // Weight (cutoff frequency) for sonar altitude measurements
        float w_z_sonar_v;  // Weight (cutoff frequency) for sonar velocity measurements

        float w_z_gps_p;    // GPS altitude data is very noisy and should be used only on airplanes
        float w_z_gps_v;    // Weight (cutoff frequency) for GPS climb rate measurements

        float w_xy_gps_p;   // Weight (cutoff frequency) for GPS position measurements
        float w_xy_gps_v;   // Weight (cutoff frequency) for GPS velocity measurements

        float w_xy_dr_p;    // When we are using dead reckoning, loosely assume that we don't move and stay at the same place
        float w_xy_dr_v;

        float w_z_res_v;    // When velocity sources lost slowly decrease estimated velocity with this weight
        float w_xy_res_v;

        float max_eph_epv;  // Max estimated position error acceptable for estimation (cm)
        float sonar_epv;    // Sonar position error
        float baro_epv;     // Baro position error
    } inav;

    uint16_t waypoint_radius;               // if we are within this distance to a waypoint then we consider it reached (distance is in cm)
    uint16_t max_speed;                     // autonomous navigation speed cm/sec
    uint16_t max_manual_speed;              // manual velocity control max horizontal speed
    uint16_t max_manual_climb_rate;         // manual velocity control max vertical speed
    uint16_t rth_altitude;                  // altitude to maintain when RTH is active (depends on rth_alt_control_style) (cm)
    uint16_t min_rth_distance;              // 0 Disables. Minimal distance for RTL in cm, otherwise it will just autoland
    uint8_t  pterm_cut_hz;                  // Low pass filter cut frequency for P-term calculation (default 20Hz)
    uint8_t  dterm_cut_hz;                  // Low pass filter cut frequency for D-term calculation (default 5Hz)
    uint8_t  pos_hold_deadband;             // Adds ability to adjust the Hold-position when moving the sticks (assisted mode)
    uint8_t  alt_hold_deadband;             // Defines the neutral zone of throttle stick during altitude hold
} navConfig_t;

// LLH Location in NEU axis system
typedef struct gpsLocation_s {
    int32_t lat;    // Lattitude * 1e+7
    int32_t lon;    // Longitude * 1e+7
    int32_t alt;    // Altitude in centimeters (meters * 100)
} gpsLocation_t;

typedef struct gpsOrigin_s {
    bool    valid;
    float   scale;
    int32_t lat;    // Lattitude * 1e+7
    int32_t lon;    // Longitude * 1e+7
    int32_t alt;    // Altitude in centimeters (meters * 100)
} gpsOrigin_s;

typedef struct {
    t_fp_vector pos;
    int32_t     yaw;
} navWaypointPosition_t;

#if defined(NAV)

void navigationUsePIDs(pidProfile_t *pidProfile);
void navigationUseConfig(navConfig_t *navConfigToUse);
void navigationUseRcControlsConfig(rcControlsConfig_t *initialRcControlsConfig);
void navigationInit(navConfig_t *initialnavConfig,
                    pidProfile_t *initialPidProfile,
                    rcControlsConfig_t *initialRcControlsConfig);

void onNewGPSData(int32_t lat, int32_t lon, int32_t alt);

void updateWaypointsAndNavigationMode(void);
void updatePositionEstimator(void);
void applyWaypointNavigationAndAltitudeHold(void);
void resetHomePosition(void);
void updateHomePosition(void);

bool naivationRequiresAngleMode(void);
bool naivationControlsHeadingNow(void);
bool navigationControlsThrottleAngleCorrection(void);

float getEstimatedActualVelocity(int axis);
float getEstimatedActualPosition(int axis);

void getWaypoint(uint8_t wpNumber, int32_t * wpLat, int32_t * wpLon, int32_t * wpAlt);
void setWaypoint(uint8_t wpNumber, int32_t wpLat, int32_t wpLon, int32_t wpAlt);

void gpsConvertGeodeticToLocal(gpsOrigin_s * origin, gpsLocation_t * llh, t_fp_vector * pos);
void gpsConvertLocalToGeodetic(gpsOrigin_s * origin, t_fp_vector * pos, gpsLocation_t * llh);

bool canActivateForcedRTH(void);
void activateForcedRTH(void);
void abortForcedRTH(void);
rthState_e getStateOfForcedRTH(void);

extern gpsLocation_t GPS_home;
extern uint16_t      GPS_distanceToHome;        // distance to home point in meters
extern int16_t       GPS_directionToHome;       // direction to home point in degrees

#if defined(BLACKBOX)
extern int16_t navCurrentMode;
extern int32_t navLatestActualPosition[3];
extern int16_t navActualVelocity[3];
extern int16_t navDesiredVelocity[3];
extern int16_t navLatestPositionError[3];
extern int16_t navActualHeading;
extern int16_t navDesiredHeading;
extern int16_t navTargetPosition[3];
extern int16_t navDebug[4];
extern uint16_t navFlags;
#define NAV_BLACKBOX_DEBUG(x,y) navDebug[x] = constrain((y), -32678, 32767)
#else
#define NAV_BLACKBOX_DEBUG(x,y)
#endif

#else

#define naivationRequiresAngleMode() (0)
#define naivationControlsHeadingNow() (0)
#define navigationControlsThrottleAngleCorrection() (0)

#endif

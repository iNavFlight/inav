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

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include "common/axis.h"
#include "common/maths.h"
#include "common/filter.h"
#include "common/calibration.h"

#include "sensors/sensors.h"

#define INAV_GPS_DEFAULT_EPH                200.0f  // 2m GPS HDOP  (gives about 1.6s of dead-reckoning if GPS is temporary lost)
#define INAV_GPS_DEFAULT_EPV                500.0f  // 5m GPS VDOP

#define INAV_GPS_ACCEPTANCE_EPE             500.0f  // 5m acceptance radius

#define INAV_ACC_BIAS_ACCEPTANCE_VALUE      (GRAVITY_CMSS * 0.25f)   // Max accepted bias correction of 0.25G - unlikely we are going to be that much off anyway

#define INAV_GPS_GLITCH_RADIUS              250.0f  // 2.5m GPS glitch radius
#define INAV_GPS_GLITCH_ACCEL               1000.0f // 10m/s/s max possible acceleration for GPS glitch detection

#define INAV_POSITION_PUBLISH_RATE_HZ       50      // Publish position updates at this rate
#define INAV_PITOT_UPDATE_RATE              10

#define INAV_GPS_TIMEOUT_MS                 1500    // GPS timeout
#define INAV_BARO_TIMEOUT_MS                200     // Baro timeout
#define INAV_SURFACE_TIMEOUT_MS             400     // Surface timeout    (missed 3 readings in a row)
#define INAV_FLOW_TIMEOUT_MS                200

#define CALIBRATING_GRAVITY_TIME_MS         2000

// Time constants for calculating Baro/Sonar averages. Should be the same value to impose same amount of group delay
#define INAV_BARO_AVERAGE_HZ                1.0f
#define INAV_SURFACE_AVERAGE_HZ             1.0f

#define INAV_ACC_CLIPPING_RC_CONSTANT           (0.010f)    // Reduce acc weight for ~10ms after clipping

#define RANGEFINDER_RELIABILITY_RC_CONSTANT     (0.47802f)
#define RANGEFINDER_RELIABILITY_LIGHT_THRESHOLD (0.15f)
#define RANGEFINDER_RELIABILITY_LOW_THRESHOLD   (0.33f)
#define RANGEFINDER_RELIABILITY_HIGH_THRESHOLD  (0.75f)

typedef struct {
    timeUs_t    lastTriggeredTime;
    timeUs_t    deltaTime;
} navigationTimer_t;

typedef struct {
    timeUs_t    lastUpdateTime; // Last update time (us)
#if defined(NAV_GPS_GLITCH_DETECTION)
    bool        glitchDetected;
    bool        glitchRecovery;
#endif
    fpVector3_t pos;            // GPS position in NEU coordinate system (cm)
    fpVector3_t vel;            // GPS velocity (cms)
    float       eph;
    float       epv;
} navPositionEstimatorGPS_t;

typedef struct {
    timeUs_t    lastUpdateTime; // Last update time (us)
    pt1Filter_t avgFilter;
    float       alt;            // Raw barometric altitude (cm)
    float       epv;
} navPositionEstimatorBARO_t;

typedef struct {
    timeUs_t    lastUpdateTime; // Last update time (us)
    float       airspeed;            // airspeed (cm/s)
} navPositionEstimatorPITOT_t;

typedef enum {
    SURFACE_QUAL_LOW,   // Surface sensor signal lost long ago - most likely surface distance is incorrect
    SURFACE_QUAL_MID,   // Surface sensor is not available but we can somewhat trust the estimate
    SURFACE_QUAL_HIGH   // All good
} navAGLEstimateQuality_e;

typedef struct {
    timeUs_t    lastUpdateTime; // Last update time (us)
    pt1Filter_t avgFilter;
    float       alt;            // Raw altitude measurement (cm)
    float       reliability;
} navPositionEstimatorSURFACE_t;

typedef struct {
    timeUs_t    lastUpdateTime; // Last update time (us)
    bool        isValid;
    float       quality;
    float       flowRate[2];
    float       bodyRate[2];
} navPositionEstimatorFLOW_t;

typedef struct {
    timeUs_t    lastUpdateTime; // Last update time (us)

    // 3D position, velocity and confidence
    fpVector3_t pos;
    fpVector3_t vel;
    float       eph;
    float       epv;

    // AGL
    navAGLEstimateQuality_e aglQual;
    float                   aglOffset;  // Offset between surface and pos.Z
    float                   aglAlt;
    float                   aglVel;

    // FLOW
    float                   flowCoordinates[2];
} navPositionEstimatorESTIMATE_t;

typedef struct {
     timeUs_t               lastUpdateTime;
    fpVector3_t             accelNEU;
    fpVector3_t             accelBias;
    float                   calibratedGravityCMSS;
    float                   accWeightFactor;
    zeroCalibrationScalar_t gravityCalibration;
} navPosisitonEstimatorIMU_t;

typedef enum {
    EST_GPS_XY_VALID            = (1 << 0),
    EST_GPS_Z_VALID             = (1 << 1),
    EST_BARO_VALID              = (1 << 2),
    EST_SURFACE_VALID           = (1 << 3),
    EST_FLOW_VALID              = (1 << 4),
    EST_XY_VALID                = (1 << 5),
    EST_Z_VALID                 = (1 << 6),
} navPositionEstimationFlags_e;

typedef struct {
    timeUs_t    baroGroundTimeout;
    float       baroGroundAlt;
    bool        isBaroGroundValid;
} navPositionEstimatorSTATE_t;


typedef struct {
    uint32_t                    flags;

    // Data sources
    navPositionEstimatorGPS_t   gps;
    navPositionEstimatorBARO_t  baro;
    navPositionEstimatorSURFACE_t surface;
    navPositionEstimatorPITOT_t pitot;
    navPositionEstimatorFLOW_t  flow;

    // IMU data
    navPosisitonEstimatorIMU_t  imu;

    // Estimate
    navPositionEstimatorESTIMATE_t  est;

    // Extra state variables
    navPositionEstimatorSTATE_t state;
} navigationPosEstimator_t;

typedef struct {
    float dt;
    uint32_t newFlags;
    float newEPV;
    float newEPH;
    fpVector3_t estPosCorr;
    fpVector3_t estVelCorr;
    fpVector3_t accBiasCorr;
} estimationContext_t;

extern float updateEPE(const float oldEPE, const float dt, const float newEPE, const float w);
extern void estimationCalculateAGL(estimationContext_t * ctx);
extern bool estimationCalculateCorrection_XY_FLOW(estimationContext_t * ctx);
extern float navGetAccelerometerWeight(void);


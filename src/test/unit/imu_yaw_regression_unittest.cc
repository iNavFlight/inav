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

/*
 * Investigation for INAV issue #11585: "transient yaw-estimation errors on
 * multirotors after fast (~180 deg) yaw maneuvers".
 *
 * This is a REPRODUCTION test only. It drives the real, unmodified
 * imu.c AHRS fusion code (imuUpdateAttitude() -> imuCalculateEstimatedAttitude()
 * -> imuMahonyAHRSupdate()) with a synthetic but internally-consistent sensor
 * stream representing a multirotor:
 *   - cruising forward at ~12 m/s (pitched forward, so the "thrust vector"
 *     used by imuCalculateMcCogWeight()'s CoG comparison for multirotors is
 *     non-degenerate),
 *   - executing a fast (~565 deg/s peak) 180 degree yaw-only flip over 0.5s,
 *   - while its GPS ground-course/ground-speed (i.e. its actual momentum)
 *     does NOT instantaneously follow the new heading -- exactly as it
 *     wouldn't in real flight, since a pure yaw rotation does not redirect
 *     an aircraft's momentum.
 *
 * The gyro fed to the FC is derived by numerically differentiating the
 * "true" attitude quaternion trajectory (constant roll=0, constant pitch
 * lean, scripted yaw(t)), so it is *exactly* consistent with that trajectory
 * -- i.e. if there were no magnetometer/GPS-CoG correction at all, the FC's
 * integrated estimate would track the true yaw perfectly, with zero error.
 * The magnetometer is likewise fed a perfectly accurate reading of the true
 * heading throughout (no injected mag interference). This isolates exactly
 * one thing: the interaction between imuCalculateMcCogWeight() dropping to
 * ~0 during the fast yaw and snapping back afterward, combined with the
 * GPS-CoG reference vector becoming stale/wrong (pointing at the old,
 * pre-yaw travel direction) once the vehicle has yawed away from it.
 *
 * Any resulting deviation between the FC's estimated yaw and the true yaw
 * is therefore attributable purely to this fusion-architecture interaction,
 * not to simulated sensor noise, gyro bias, or GPS wobble.
 */

#include <stdint.h>
#include <limits.h>
#include <math.h>
#include <cstdio>
#include <cstring>

extern "C" {
    #include "sensors/gyro.h"
    #include "sensors/compass.h"
    #include "sensors/acceleration.h"

    #include "scheduler/scheduler.h"
    #include "fc/runtime_config.h"

    #include "io/gps.h"
    #include "io/beeper.h"
    #include "flight/pid.h"
    #include "flight/imu.h"
}

#include "unittest_macros.h"
#include "gtest/gtest.h"

extern "C" {
STATIC_UNIT_TESTED void imuComputeQuaternionFromRPY(int16_t initialRoll, int16_t initialPitch, int16_t initialYaw);
}

static uint32_t gSensorsMask = 0;
static uint32_t gSimMillis = 60000; // starts well past the 20s "fast gains" boot window

namespace {

// Saves/restores the FC's live `orientation` global around a call that
// (ab)uses imuComputeQuaternionFromRPY() purely as a trig-correct RPY->quaternion
// converter to build our independent "ground truth" trajectory.
fpQuaternion_t computeTrueQuaternion(int16_t rollDeciDeg, int16_t pitchDeciDeg, int16_t yawDeciDeg)
{
    // imuComputeQuaternionFromRPY() is a production function that (as a side
    // effect) also overwrites the FC's live `rMat` via imuComputeRotationMatrix().
    // We only want its trig-correct RPY->quaternion conversion for building our
    // independent ground-truth trajectory, so both globals it touches must be
    // saved/restored -- not just `orientation`.
    fpQuaternion_t savedOrientation = orientation;
    float savedRMat[3][3];
    memcpy(savedRMat, rMat, sizeof(rMat));

    imuComputeQuaternionFromRPY(rollDeciDeg, pitchDeciDeg, yawDeciDeg);
    fpQuaternion_t result = orientation;

    orientation = savedOrientation;
    memcpy(rMat, savedRMat, sizeof(rMat));
    return result;
}

// Scripted true yaw trajectory (degrees): hold at 0, ramp (raised-cosine,
// smooth start/stop) to 180 over Tman seconds, then hold at 180.
double trueYawDeg(double t, double t0, double tman, double totalYawDeg)
{
    if (t < t0) return 0.0;
    if (t < t0 + tman) {
        double s = (t - t0) / tman;
        return (totalYawDeg / 2.0) * (1.0 - cos(M_PI * s));
    }
    return totalYawDeg;
}

struct SimResult {
    double maxYawErrorDegAfterManeuver = 0.0;  // largest |estYaw - trueYaw| observed after the yaw completes
    double yawErrorAtManeuverEnd = 0.0;
    double finalYawErrorDeg = 0.0;
    double settleTimeSec = -1.0;               // time after maneuver end for error to drop below 2 deg and stay there
};

// Runs the scenario. If injectStaleCoG is false, the GPS course is kept
// aligned with the true heading throughout (sanity/control run). If true,
// the GPS ground course/velocity is frozen at the pre-maneuver heading,
// modelling a real quad's momentum not instantly following a yaw-only flip.
SimResult runYawFlipScenario(bool useGps, bool injectStaleCoG, const char *csvPath)
{
    // ---- Sensor / state configuration -------------------------------------------------
    stateFlags = MULTIROTOR | ACCELEROMETER_CALIBRATED | COMPASS_CALIBRATED;
    if (useGps) {
        stateFlags |= GPS_FIX;
    }
    armingFlags = ARMED;
    flightModeFlags = 0;

    gSensorsMask = SENSOR_ACC | SENSOR_MAG;
    if (useGps) {
        gSensorsMask |= SENSOR_GPS;
    }

    gpsSol.numSat = 10;
    gpsSol.groundSpeed = 1200;   // cm/s (~12 m/s) -- fully engages wCoG's speed-based weighting
    gpsSol.groundCourse = 0;     // decidegrees -- initial travel direction matches initial nose heading
    gpsSol.velNED[0] = 1200;
    gpsSol.velNED[1] = 0;
    gpsSol.velNED[2] = 0;
    gpsSol.flags.gpsHeartbeat = false;

    // imuConfig_t is a PG-registered struct; PG defaults are normally loaded via
    // pgResetAll() at boot, which this host test does not link (it pulls in
    // linker-script-only symbols like __pg_registry_start that don't exist for a
    // plain host binary). Without ANY initialization, imuConfig()->acc_ignore_rate
    // stays zero, which makes imuCalculateMcCogWeight()'s scaleRangef(x, 0, 0, ...)
    // divide by zero (0/0 == NaN) on every single call -- a test-harness bug that
    // looks deceptively like a frozen/broken AHRS. So: set the fields that matter
    // to their real settings.yaml defaults by hand.
    imuConfigMutable()->dcm_kp_acc = 2000;
    imuConfigMutable()->dcm_ki_acc = 50;
    imuConfigMutable()->dcm_kp_mag = 2000;
    imuConfigMutable()->dcm_ki_mag = 50;
    imuConfigMutable()->small_angle = 25;
    imuConfigMutable()->acc_ignore_rate = 15;
    imuConfigMutable()->acc_ignore_slope = 5;
    imuConfigMutable()->gps_yaw_windcomp = 1;
    imuConfigMutable()->inertia_comp_method = COMPMETHOD_ADAPTIVE;
    imuConfigMutable()->gps_yaw_weight = 100;

    imuConfigure();
    imuInit();
    imuUpdateAccelerometer(); // marks isAccelUpdatedAtLeastOnce so imuUpdateAttitude() actually runs fusion

    const int16_t pitchLeanDeciDeg = 150; // 15 deg constant forward lean (cruise flight)
    // Seed the FC's actual estimate to the true t=0 attitude (level roll, pitch-leaned
    // cruise, yaw=0) so we're observing the maneuver's effect, not a startup convergence
    // transient from imuInit()'s default level/identity orientation.
    imuComputeQuaternionFromRPY(0, pitchLeanDeciDeg, 0);
    const double t0 = 1.0;                // maneuver start
    const double tman = 0.5;              // maneuver duration
    const double totalYawDeg = 180.0;
    const double dt = 0.001;              // 1 kHz
    const double totalT = 6.0;

    timeUs_t nowUs = 0;
    FILE *csv = csvPath ? fopen(csvPath, "w") : nullptr;
    if (csv) {
        fprintf(csv, "t,trueYaw,estYaw,err,groundCourseUsed\n");
    }

    SimResult result;
    bool maneuverEnded = false;
    bool settled = false;

    const int steps = (int)(totalT / dt);
    for (int i = 0; i < steps; i++) {
        double t = i * dt;
        double tNext = t + dt;

        double yawNowDeg = trueYawDeg(t, t0, tman, totalYawDeg);
        double yawNextDeg = trueYawDeg(tNext, t0, tman, totalYawDeg);

        fpQuaternion_t qNow = computeTrueQuaternion(0, pitchLeanDeciDeg, (int16_t)lround(yawNowDeg * 10.0));
        fpQuaternion_t qNext = computeTrueQuaternion(0, pitchLeanDeciDeg, (int16_t)lround(yawNextDeg * 10.0));

        // Numerically differentiate the true trajectory to get the exact
        // body-frame angular rate that would carry qNow -> qNext under the
        // firmware's own integration convention (orientation *= deltaQ).
        fpQuaternion_t qNowConj;
        quaternionConjugate(&qNowConj, &qNow);
        fpQuaternion_t qDelta;
        quaternionMultiply(&qDelta, &qNowConj, &qNext);
        quaternionNormalize(&qDelta, &qDelta);
        fpAxisAngle_t aa;
        quaternionToAxisAngle(&aa, &qDelta);

        fpVector3_t bodyRateRad = {{ aa.axis.x * aa.angle / dt,
                                      aa.axis.y * aa.angle / dt,
                                      aa.axis.z * aa.angle / dt }};

        gyro.gyroADCf[X] = (float)RADIANS_TO_DEGREES(bodyRateRad.x);
        gyro.gyroADCf[Y] = (float)RADIANS_TO_DEGREES(bodyRateRad.y);
        gyro.gyroADCf[Z] = (float)RADIANS_TO_DEGREES(bodyRateRad.z);

        // Accelerometer: rotate the EF unit-gravity vector into body frame
        // under the TRUE attitude -- a perfectly consistent accelerometer
        // reading for this trajectory (no vibration/noise).
        fpVector3_t gravityEFUnit = {{0.0f, 0.0f, 1.0f}};
        fpVector3_t accBFUnit;
        quaternionRotateVector(&accBFUnit, &gravityEFUnit, &qNow);
        acc.accADCf[X] = accBFUnit.x;
        acc.accADCf[Y] = accBFUnit.y;
        acc.accADCf[Z] = accBFUnit.z;

        // Magnetometer: rotate EF magnetic-north vector into body frame
        // under the TRUE attitude -- perfectly accurate, undisturbed mag.
        fpVector3_t magEF = {{1024.0f, 0.0f, 0.0f}};
        fpVector3_t magBF;
        quaternionRotateVector(&magBF, &magEF, &qNow);
        mag.magADC[X] = magBF.x;
        mag.magADC[Y] = magBF.y;
        mag.magADC[Z] = magBF.z;

        // GPS ground course: either tracks true heading (control run) or
        // stays frozen at the pre-maneuver heading (momentum not yet caught
        // up with the new nose direction -- the physically-correct behavior
        // for a yaw-only maneuver).
        double courseUsedDeg;
        if (!injectStaleCoG) {
            courseUsedDeg = yawNowDeg;
        } else {
            courseUsedDeg = (t < t0) ? yawNowDeg : 0.0;
        }
        gpsSol.groundCourse = (int16_t)lround(courseUsedDeg * 10.0);
        gpsSol.flags.gpsHeartbeat = !gpsSol.flags.gpsHeartbeat;

        nowUs += (timeUs_t)(dt * 1.0e6);
        gSimMillis = 60000 + (uint32_t)(t * 1000.0);
        imuUpdateAttitude(nowUs);

        double estYawDeg = DECIDEGREES_TO_DEGREES((double)attitude.values.yaw);
        // shortest-path angular error in degrees
        double err = estYawDeg - yawNowDeg;
        while (err > 180.0) err -= 360.0;
        while (err < -180.0) err += 360.0;

        if (csv) {
            fprintf(csv, "%.4f,%.3f,%.3f,%.3f,%.3f\n", t, yawNowDeg, estYawDeg, err, courseUsedDeg);
        }

        if (t >= t0 + tman) {
            if (!maneuverEnded) {
                maneuverEnded = true;
                result.yawErrorAtManeuverEnd = err;
            }
            if (fabs(err) > fabs(result.maxYawErrorDegAfterManeuver)) {
                result.maxYawErrorDegAfterManeuver = err;
            }
            if (!settled && fabs(err) < 2.0) {
                settled = true;
                result.settleTimeSec = t - (t0 + tman);
            } else if (settled && fabs(err) >= 2.0) {
                settled = false; // re-diverged, keep looking
                result.settleTimeSec = -1.0;
            }
        }
        result.finalYawErrorDeg = err;
    }

    if (csv) fclose(csv);
    return result;
}

} // namespace

// Control run: fast yaw flip, but GPS-CoG tracks the true heading throughout
// (as if the vehicle's momentum re-aligned instantly -- not physically real,
// but isolates whether the wCoG-weight snap ALONE, with a fully-consistent
// CoG reference, causes any artifact).
TEST(ImuYawRegressionTest, FastYawWithConsistentCoG_NoArtifactExpected)
{
    SimResult r = runYawFlipScenario(/*useGps=*/true, /*injectStaleCoG=*/false,
        "/home/raymorris/Documents/planes/inavflight/claude/developer/workspace/investigate-yaw-estimation-regression-80/consistent_cog.csv");

    printf("[consistent CoG] err@maneuverEnd=%.3f maxErrAfter=%.3f finalErr=%.3f settle=%.3f\n",
        r.yawErrorAtManeuverEnd, r.maxYawErrorDegAfterManeuver, r.finalYawErrorDeg, r.settleTimeSec);

    EXPECT_LT(fabs(r.maxYawErrorDegAfterManeuver), 3.0)
        << "Fast yaw with a self-consistent (non-stale) GPS-CoG reference should not "
           "produce a large yaw estimate error just from the wCoG weight transition.";
}

// Reproduction run: fast yaw flip while GPS-CoG (momentum) stays frozen at
// the pre-maneuver heading, exactly as real inertia would behave. This is
// the scenario theorized in issue #11585 / PR #9387's mechanism.
TEST(ImuYawRegressionTest, FastYawWithStaleCoGAfterFlip_LooksForStepDiscontinuity)
{
    SimResult r = runYawFlipScenario(/*useGps=*/true, /*injectStaleCoG=*/true,
        "/home/raymorris/Documents/planes/inavflight/claude/developer/workspace/investigate-yaw-estimation-regression-80/stale_cog.csv");

    printf("[stale CoG] err@maneuverEnd=%.3f maxErrAfter=%.3f finalErr=%.3f settle=%.3f\n",
        r.yawErrorAtManeuverEnd, r.maxYawErrorDegAfterManeuver, r.finalYawErrorDeg, r.settleTimeSec);

    // No EXPECT_* assertion of "should be small" here on purpose -- we are
    // observing/reporting, not asserting a pass/fail bug fix. The magnitude
    // is inspected in the test report / CSV.
    RecordProperty("maxYawErrorDegAfterManeuver", std::to_string(r.maxYawErrorDegAfterManeuver));
    RecordProperty("yawErrorAtManeuverEnd", std::to_string(r.yawErrorAtManeuverEnd));
    RecordProperty("finalYawErrorDeg", std::to_string(r.finalYawErrorDeg));
}

// No-GPS control: mag-only fusion, no CoG term at all (useCOG=false).
// Confirms the AHRS is numerically well-behaved through the same fast yaw
// when the CoG mechanism is entirely absent.
TEST(ImuYawRegressionTest, FastYawMagOnly_NoGpsAtAll)
{
    // NOTE: imu.c has several function-local `static` variables (e.g. the GPS
    // acceleration estimator's lastGPSNewDataTime/lastGPSvel, imuMahonyAHRSupdate's
    // vGyroDriftEstimate/prevOrientation) that are NOT reset by imuInit() -- only
    // by process (re)start. When this test runs in the same process after the two
    // GPS/CoG tests above, in gtest's default (non-shuffled) order, that leaked
    // state occasionally produces a much larger (~25 deg) transient than when this
    // test runs alone or in a different order (observed: -0.1 deg standalone,
    // ~25.8 deg after the two CoG tests in default order). This is a test-harness
    // isolation limitation (shared C statics across TEST() cases in one binary),
    // not a GPS-CoG-specific finding -- this scenario has no GPS/CoG input at all --
    // so it's reported via RecordProperty rather than asserted on.
    SimResult r = runYawFlipScenario(/*useGps=*/false, /*injectStaleCoG=*/false, "/home/raymorris/Documents/planes/inavflight/claude/developer/workspace/investigate-yaw-estimation-regression-80/mag_only.csv");

    printf("[mag only] err@maneuverEnd=%.3f maxErrAfter=%.3f finalErr=%.3f\n",
        r.yawErrorAtManeuverEnd, r.maxYawErrorDegAfterManeuver, r.finalYawErrorDeg);
    RecordProperty("maxYawErrorDegAfterManeuver", std::to_string(r.maxYawErrorDegAfterManeuver));
}

// STUBS

extern "C" {

uint32_t stateFlags;
uint32_t flightModeFlags;
uint32_t armingFlags;

acc_t acc;
mag_t mag;

gpsSolutionData_t gpsSol;

compassConfig_t compassConfig_System;

pidProfile_t* pidProfile_ProfileCurrent;

uint8_t detectedSensors[] = { GYRO_NONE, ACC_NONE };

bool isMixerTransitionMixing = false;

bool sensors(uint32_t mask)
{
    return (gSensorsMask & mask) == mask;
};
uint32_t millis(void) { return gSimMillis; }
timeDelta_t getLooptime(void) { return gyro.targetLooptime; }
timeDelta_t getGyroLooptime(void) { return gyro.targetLooptime; }
void schedulerResetTaskStatistics(cfTaskId_e) {}
void sensorsSet(uint32_t) {}
bool compassIsHealthy(void) { return true; }
void accGetVibrationLevels(fpVector3_t *accVibeLevels)
{
    accVibeLevels->x = fast_fsqrtf(acc.accVibeSq[X]);
    accVibeLevels->y = fast_fsqrtf(acc.accVibeSq[Y]);
    accVibeLevels->z = fast_fsqrtf(acc.accVibeSq[Z]);
}
void accGetMeasuredAcceleration(fpVector3_t *measuredAcc)
{
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        measuredAcc->v[axis] = acc.accADCf[axis] * GRAVITY_CMSS;
    }
}
uint32_t accGetClipCount(void)
{
    return acc.accClipCount;
}
void accUpdate(void)
{
}
void resetHeadingHoldTarget(int16_t heading)
{
    UNUSED(heading);
}
bool isGPSHeadingValid(void) { return (gSensorsMask & SENSOR_GPS) != 0; }
void beeper(beeperMode_e mode) { UNUSED(mode); }
}

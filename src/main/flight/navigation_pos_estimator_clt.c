#define CLT_BARO_POSZ_NOISE_DEFAULT     (1.0f * 100.0f)         // This is the RMS value of noise in the altitude measurement (cm)
#define CLT_BARO_VELZ_NOISE_DEFAULT     (1.0f * 100.0f)         // RMS of BARO altitude derivative (noise of climb rate measurement) (cm/s)
#define CLT_SONAR_POSZ_NOISE_DEFAULT    (0.01f * 100.0f)
#define CLT_SONAR_VELZ_NOISE_DEFAULT    (0.01f * 100.0f)
#define CLT_GPS_POSXY_NOISE_DEFAULT     (0.5f * 100.0f)         // This is the RMS value of noise in the GPS horizontal position measurements (cm)
#define CLT_GPS_POSZ_NOISE_DEFAULT      (0.7f * 100.0f)         // This is the RMS value of noise in the GPS vertical position measurements (cm)
#define CLT_GPS_VELXY_NOISE_DEFAULT     (0.5f * 100.0f)         // RMS of GPS_POSXY derivative (noise of GPS XY velocity measurement) (cm/s)
#define CLT_GPS_VELZ_NOISE_DEFAULT      (0.7f * 100.0f)         // RMS of GPS_POSZ derivative (noise of GPS Z velocity measurement) (cm/s)
#define CLT_ACC_NOISE_DEFAULT           (0.25f * 100.0f)        // Growth of estimated error due to accelerometer measurement errors (cm/s/s)
#define CLT_SONAR_MAX_RANGE             (300.0f)

typedef struct {
    bool available;
    float variance;
    float value;
} navCLTAxisValue_t;

typedef struct {
    // GPS origin
    bool isGpsOriginValid;
    navLocation_t gpsOrigin;     // GPS origin (LLH coordinates)

    // CLT state estimations
    struct {
        // For GPS we are doing only altitude
        navCLTAxisValue_t   pos[XYZ_AXIS_COUNT];    // FIXME: Only altitude (Z) is used until we move to relativePos estimator
        navCLTAxisValue_t   vel[XYZ_AXIS_COUNT];    // FIXME: Only XY-velocity is used until GPS will return altitude and climb rate in cms
    } gps;

    struct {
        navCLTAxisValue_t   pos_z;
        navCLTAxisValue_t   vel_z;
    } baro;

    struct {
        navCLTAxisValue_t   pos_z;
        navCLTAxisValue_t   vel_z;
    } sonar;

    struct {
        navCLTAxisValue_t   vel[XYZ_AXIS_COUNT];
    } imu;

    struct {
        navCLTAxisValue_t   pos[XYZ_AXIS_COUNT];
        navCLTAxisValue_t   vel[XYZ_AXIS_COUNT];
    } estimated;
} navCLTFilterState_t;

static navCLTFilterState_t cltState;

static void cltSetGPSOrigin(navLocation_t * llh)
{
    cltState.gpsOrigin.lat = llh->lat;
    cltState.gpsOrigin.lon = llh->lon;
    cltState.gpsOrigin.alt = llh->alt;
    cltState.isGpsOriginValid = true;
}

static void cltGPSAbsoluteToRelativePosition(navLocation_t * llh, t_fp_vector * xyz)
{
    if (cltState.isGpsOriginValid) {
        float gpsScaleLonDown = constrainf(cos_approx((ABS(llh->lat) / 10000000.0f) * 0.0174532925f), 0.01f, 1.0f);
        xyz->V.X = (llh->lat - cltState.gpsOrigin.lat) * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;
        xyz->V.Y = (llh->lon - cltState.gpsOrigin.lon) * gpsScaleLonDown * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;
        xyz->V.Z = (llg->alt - cltState.gpsOrigin.alt);
    }
    else {
        xyz->V.X = 0.0f;
        xyz->V.Y = 0.0f;
        xyz->V.Z = 0.0f;
    }
}

static void cltFilterReset(void)
{
    int axis;

    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        cltState.gps.pos[axis].available = false;
        cltState.gps.vel[axis].available = false;
        cltState.imu.vel[axis].available = false;
        cltState.estimated.pos[axis].available = false;
        cltState.estimated.vel[axis].available = false;
    }

    cltState.sonar.pos_z.available = false;
    cltState.sonar.vel_z.available = false;
    cltState.baro.pos_z.available = false;
    cltState.baro.vel_z.available = false;
}

static void cltFilterConstrainStatesAndVariances(void)
{
    // Position variances
    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) cltState.gps.pos[axis].variance = constrainf(cltState.gps.pos[axis].variance, 0.0f, 1.0e6f);
    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) cltState.estimate.pos[axis].variance = constrainf(cltState.estimate.pos[axis].variance, 0.0f, 1.0e6f);
    cltState.sonar.pos_z.variance = constrainf(cltState.sonar.pos_z.variance, 0.0f, 1.0e6f);
    cltState.baro.pos_z.variance = constrainf(cltState.baro.pos_z.variance, 0.0f, 1.0e6f);

    // Velocity variances
    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) cltState.gps.vel[axis].variance = constrainf(cltState.gps.vel[axis].variance, 0.0f, 1.0e3f);
    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) cltState.imu.vel[axis].variance = constrainf(cltState.imu.vel[axis].variance, 0.0f, 1.0e3f);
    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) cltState.estimate.vel[axis].variance = constrainf(cltState.estimate.vel[axis].variance, 0.0f, 1.0e3f);
    cltState.sonar.vel_z.variance = constrainf(cltState.sonar.vel_z.variance, 0.0f, 1.0e3f);
    cltState.baro.vel_z.variance = constrainf(cltState.baro.vel_z.variance, 0.0f, 1.0e3f);
}

static void cltFilterUpdateEstimate(void)
{
    int axis;
    float new_variance;
    float new_value;
    bool new_available;

    cltFilterConstrainStatesAndVariances();

    // Update position estimate
    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        new_variance = 0;
        new_value = 0;
        new_available = false;

        // GPS
        if (cltState.gps.alt.available) {
            new_available = true;
            new_variance += 1.0f / cltState.gps.pos[axis].variance;
            new_value += (float)cltState.gps.pos[axis].value / cltState.gps.pos[axis].variance;
        }

        // For Z axis use BARO and SONAR
        if (axis == Z) {
            // Baro
            if (cltState.baro.pos_z.available) {
                new_available = true;
                new_variance += 1.0f / cltState.baro.pos_z.variance;
                new_value += (float)cltState.baro.pos_z.value / cltState.baro.pos_z.variance;
            }

            // Sonar
            if (cltState.sonar.pos_z.available) {
                new_available = true;
                new_variance += 1.0f / cltState.sonar.pos_z.variance;
                new_value += (float)cltState.sonar.pos_z.value / cltState.sonar.pos_z.variance;
            }
        }

        // Update CLT estimate
        cltState.estimated.pos[axis].available = new_available;
        if (new_available) {
            cltState.estimated.pos[axis].variance = 1.0f / new_variance;
            cltState.estimated.pos[axis].value = new_value / new_variance;
        }
    }

    // Update velocity estimate
    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        new_variance = 0;
        new_value = 0;
        new_available = false;

        // GPS velocity
        if (cltState.gps.vel[axis].available) {
            new_available = true;
            new_variance += 1.0f / cltState.gps.vel[axis].variance;
            new_value += (float)cltState.gps.vel[axis].value / cltState.gps.vel[axis].variance;
        }

        // IMU
        if (cltState.imu.vel[axis].available) {
            new_available = true;
            new_variance += 1.0f / cltState.imu.vel[axis].variance;
            new_value += (float)cltState.imu.vel[axis].value / cltState.imu.vel[axis].variance;
        }

        // For Z axis use BARO and SONAR
        if (axis == Z) {
            // Baro
            if (cltState.baro.vel_z.available) {
                new_available = true;
                new_variance += 1.0f / cltState.baro.vel_z.variance;
                new_value += (float)cltState.baro.vel_z.value / cltState.baro.vel_z.variance;
            }

            // Sonar
            if (cltState.sonar.vel_z.available) {
                new_available = true;
                new_variance += 1.0f / cltState.sonar.vel_z.variance;
                new_value += (float)cltState.sonar.vel_z.value / cltState.sonar.vel_z.variance;
            }
        }

        // Update estimate
        cltState.estimated.vel[axis].available = new_available;
        if (new_available) {
            cltState.estimated.vel[axis].variance = 1.0f / new_variance;
            cltState.estimated.vel[axis].value = new_value / new_variance;
        }
    }
}

static void cltFilterUpdateNAV(void)
{
/*
    // Altitude
    if (cltState.estimated.alt.available)
        updateActualAltitude(cltState.estimated.alt.value);

    // Vertical velocity
    if (cltState.estimated.vel[Z].available)
        updateActualVerticalVelocity(cltState.estimated.vel[Z].value);

    // Horizontal velocity
    if (cltState.estimated.vel[X].available && cltState.estimated.vel[Y].available)
        updateActualHorizontalVelocity(cltState.estimated.vel[X].value, cltState.estimated.vel[Y].value);
*/
}

/*
 * Update CLT filter state from accelerometer integration result.
 */
static void cltFilterUpdate_IMU(float dT)
{
    int axis;

    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        if (cltState.estimated.vel[axis].available) {
            cltState.imu.vel[axis].available = true;
            // This is tricky. We take the most recent estimate value and update it using IMU
            cltState.imu.vel[axis].value = cltState.estimated.vel[axis].value + imuAverageAcceleration[axis] * dT;
            // Position uncertainty is increased
            cltState.imu.vel[axis].variance = cltState.estimated.vel[axis].variance + 2 * sq(dT * CLT_ACC_NOISE_DEFAULT);
        }
        else {
            cltState.imu.vel[axis].available = false;
        }
    }
}

static void cltFilterUpdate_GPS(t_fp_vector * pos, t_fp_vector * vel)
{
    // Position
    cltState.gps.pos[X].available = false;
    cltState.gps.pos[X].variance = sq(CLT_GPS_POSXY_NOISE_DEFAULT);
    cltState.gps.pos[X].value = pos.V.X;

    cltState.gps.pos[Y].available = false;
    cltState.gps.pos[Y].variance = sq(CLT_GPS_POSXY_NOISE_DEFAULT);
    cltState.gps.pos[Y].value = pos.V.Y;

    cltState.gps.pos[Z].available = false;
    cltState.gps.pos[Z].variance = sq(CLT_GPS_POSZ_NOISE_DEFAULT);
    cltState.gps.pos[Z].value = pos.V.Z;

    // Velocity
    cltState.gps.vel[X].available = false;
    cltState.gps.vel[X].variance = sq(CLT_GPS_VELXY_NOISE_DEFAULT);
    cltState.gps.vel[X].value = vel.V.X;

    cltState.gps.vel[Y].available = false;
    cltState.gps.vel[Y].variance = sq(CLT_GPS_VELXY_NOISE_DEFAULT);
    cltState.gps.vel[Y].value = vel.V.Y;

    cltState.gps.vel[Z].available = false;
    cltState.gps.vel[Z].variance = sq(CLT_GPS_VELZ_NOISE_DEFAULT);
    cltState.gps.vel[Z].value = vel.V.Z;
}

static void cltFilterUpdate_BARO(float alt, float vel)
{
    cltState.baro.pos_z.available = true;
    cltState.baro.pos_z.variance = sq(CLT_BARO_POSZ_NOISE_DEFAULT);
    cltState.baro.pos_z.value = alt;

    cltState.baro.vel_z.available = true;
    cltState.baro.vel_z.variance = sq(CLT_BARO_VELZ_NOISE_DEFAULT);
    cltState.baro.vel_z.value = vel;
}

static void cltFilterUpdate_SONAR(float alt, float vel)
{
    cltState.sonar.pos_z.available = true;
    cltState.sonar.pos_z.variance = sq(CLT_SONAR_POSZ_NOISE_DEFAULT);
    cltState.sonar.pos_z.value = alt;

    cltState.sonar.vel_z.available = true;
    cltState.sonar.vel_z.variance = sq(CLT_SONAR_VELZ_NOISE_DEFAULT);
    cltState.sonar.vel_z.value = vel;

    // FIXME: We should gracefully transition from SONAR to BARO/GPS
    // To do this sonar variance should be gracefully increased at the edge of sonar range
    if (alt > 0.667f * CLT_SONAR_MAX_RANGE) {
        float linearRate = constrainf((alt - 0.667f * CLT_SONAR_MAX_RANGE) / (CLT_SONAR_MAX_RANGE - 0.667f * CLT_SONAR_MAX_RANGE), 0.0f, 1.0f);
        cltState.sonar.pos_z.variance += 1.0e6f * linearRate;
        cltState.sonar.vel_z.variance += 1.0e3f * linearRate;
    }
}
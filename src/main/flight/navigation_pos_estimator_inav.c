#define INAV_USE_FAST_RELATIVE_TRANSFORMATION   // Use faster, but less precise transformation from LLH to NEU coordinates
#define INAV_W_Z_BARO       0.5f    // Weight (cutoff frequency) for barometer altitude measurements.
#define INAV_W_Z_GPS_P      0.005f  // GPS altitude data is very noisy and should be used only as slow correction for baro offset.
#define INAV_W_Z_GPS_V      0.0f    // Weight (cutoff frequency) for GPS altitude velocity measurements.
#define INAV_W_XY_GPS_P     1.0f    // Weight (cutoff frequency) for GPS position measurements.
#define INAV_W_XY_GPS_V     2.0f    // Weight (cutoff frequency) for GPS velocity measurements.
#define INAV_W_Z_RES_V      0.5f    // When velocity sources lost slowly decrease estimated horizontal velocity with this weight.
#define INAV_W_XY_RES_V     0.5f    // When velocity sources lost slowly decrease estimated horizontal velocity with this weight.
#define INAV_SKIP_UPDATES   3       // If this number of updates is skipped assume velocity source is lost

typedef struct {
    float pos;
    float vel;
} navPositionAxis_t;

/* Inertial filter, based on PX4 implementation by Anton Babushkin	<rk3dov@gmail.com> */
static void inavFilterPredict(navPositionAxis_t * state, float dt, float acc)
{
    state->pos += state->vel * dt + acc * dt * dt / 2.0f;
    state->vel += acc * dt;
}

static void inavFilterCorrectPos(navPositionAxis_t * state, float dt, float e, float w)
{
    float ewdt = e * w * dt;
    state->pos += ewdt;
    state->vel += w * ewdt;
}

static void inavFilterCorrectVel(navPositionAxis_t * state, float dt, float e, float w)
{
    state->vel += e * w * dt;
}

typedef struct {
    float accel[XYZ_AXIS_COUNT];
    float accelCount[XYZ_AXIS_COUNT];
} navPositionEstimatorIMU_t;

typedef struct {
    bool            isUpdated;
    bool            isOriginValid;
    navLocation_t   originPos;      // GPS origin (LLH coordinates)
    navLocation_t   rawPos;         // GPS coordinates (LLH)

    t_fp_vector     posNEU;         // GPS position in NEU coordinate system
    t_fp_vector     velNEU;         // GPS velocity (NEU)
} navPositionEstimatorGPS_t;

typedef struct {
    bool        isUpdated;
    bool        isOffsetValid;
    float       offset;
    float       rawAlt;
} navPositionEstimatorBARO_t;

typedef struct {
    /* Updates from various sources */
    navPositionEstimatorGPS_t   gps;
    navPositionEstimatorBARO_t  baro;
    navPositionEstimatorIMU_t   imu;

    /* Estimated position */
    navPositionAxis_t estimate[XYZ_AXIS_COUNT];

    /* Flags */
    bool isUpdatesXY;
    bool isUpdatedZ;
} navPositionEstimator_t;

static navPositionEstimator_t   navPosState;

/* Update acceleration average accumulators */
void inavUpdateIMUData(void)
{
    int axis;

    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        navPosState.imu.accel[axis] = imuAverageAcceleration[axis];
        navPosState.imu.accelCount[axis]++;
    }
}

/* Update state from barometer */
void inavUpdateBaroData(void)
{
    navPosState.baro.isUpdated = true;
    navPosState.baro.rawAlt = baroCalculateAltitude();

    if (!navPosState.baro.isOffsetValid) {
        navPosState.baro.isOffsetValid = true;
        navPosState.baro.offset = navPosState.baro.rawAlt;
    }
}

/* Update state from GPS */
void inavUpdateGPSData(int32_t newLat, int32_t newLon, int32_t newAlt)
{
    if (STATE(GPS_FIX) && GPS_numSat >= 5) {
        navPosState.gps.isUpdated = true;

        /* Update GPS raw pos data */
        navPosState.gps.rawPos.lat = newLat;
        navPosState.gps.rawPos.lon = newLon;
        navPosState.gps.rawPos.alt = newAlt;

        /* Convert GPS LLH to NEU position */
        inavGpsLocationToRelativePosition(&navPosState.gps.origin, &navPosState.gps.rawPos, &navPosState.gps.posNEU);

        /* Update GPS velocity data */
        // FIXME
        navPosState.gps.velNEU.V.X = 0;
        navPosState.gps.velNEU.V.X = 0;
        navPosState.gps.velNEU.V.X = 0;

        /* Update GPS origin */
        if (!navPosState.gps.isOriginValid) {
            navPosState.gps.originPos = navPosState.gps.rawPos;
            navPosState.gps.isOriginValid = true;
        }
    }
    else {
        navPosState.gps.isUpdated = false;
    }
}

#define INAV_CONSTANTS_RADIUS_OF_EARTH      (6371000 * 100)     // cm
void inavGpsLocationToRelativePosition(navLocation_t * origin, navLocation_t * llh, t_fp_vector * xyz)
{
#if INAV_USE_FAST_RELATIVE_TRANSFORMATION
    float gpsScaleLonDown = constrainf(cos_approx((ABS(llh->lat) / 10000000.0f) * 0.0174532925f), 0.01f, 1.0f);
    xyz->V.X = (llh->lat - origin->lat) * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;
    xyz->V.Y = (llh->lon - origin->lon) * gpsScaleLonDown * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;
    xyz->V.Z = (llg->alt - origin->alt);
#else
    float originLatRad = origin->lat / 10000000.0f * 0.0174532925f;
	float originLonRad = origin->lon / 10000000.0f * 0.0174532925f;
	float originSinLat = sin_approx(lat_rad);
	float originCosLat = cos_approx(lat_rad);

	float latRad = llh->lat / 10000000.0f * 0.0174532925f;
	float lonRad = llh->lon / 10000000.0f * 0.0174532925f;

	float sinLat = sin(lat_rad);
	float cosLat = cos(lat_rad);
	float conDeltaLon = cos(lonRad - originLonRad);

	float c = acos_approx(originSinLat * sinLat + originCosLat * cosLat * conDeltaLon);
	float k = (fabsf(c) < 1e-6) ? 1.0 : (c / sin_approx(c));

	xyz->V.X = k * (originCosLat * sinLat - originSinLat * cosLat * cosDeltaLon) * INAV_CONSTANTS_RADIUS_OF_EARTH;
	xyz->V.Y = k * cosLat * sin_approx(lonRad - originLonRad) * INAV_CONSTANTS_RADIUS_OF_EARTH;
    xyz->V.Z = (llg->alt - origin->alt);
#endif
}

// Updated at loop rate
void inavUpdateEstimate(void)
{
    bool useGpsZ = false;
    bool useGpsXY = false;
    bool useBaroZ = false;

    static uint32_t skippedEstimateXY = 0;
    static uint32_t skippedEstimateZ = 0;

    bool canEstimateXY = false;
    bool canEstimateZ = false;

    float correctionBaro;
    float correctionGPSPos[XYZ_AXIS_COUNT];
    float correctionGPSVel[XYZ_AXIS_COUNT];

    /* Convert GPS position to relative position */
    if (navPosState.gps.isUpdated) {
        /* Calculate GPS position corrections */
        correctionGPSPos[X] = navPosState.gps.posNEU.V.X - navPosState.estimate[X].pos;
        correctionGPSPos[Y] = navPosState.gps.posNEU.V.Y - navPosState.estimate[Y].pos;
        correctionGPSPos[Z] = navPosState.gps.posNEU.V.Z - navPosState.estimate[Z].pos;

        /* Calculate GPS velocity corrections */
        correctionGPSVel[X] = navPosState.gps.velNEU.V.X - navPosState.estimate[X].vel;
        correctionGPSVel[Y] = navPosState.gps.velNEU.V.Y - navPosState.estimate[Y].vel;
        correctionGPSVel[Z] = navPosState.gps.velNEU.V.Z - navPosState.estimate[Z].vel;

        useGpsZ = true;
        useGpsXY = true;
        canEstimateXY = true;

        navPosState.gps.isUpdated = false;
    }

    /* Preprocess baro data */
    if (navPosState.baro.isUpdated) {
        useBaroZ = true;
        canEstimateZ = true;

        navPosState.baro.isUpdated = false;
    }

    /* We update this only if baro is updated */
    if (canEstimateZ) {
        /* Calculate average acceleration */
        float inavAvgAccelZ = navPosState.imu.accel[Z] / navPosState.imu.accelCount[Z];
        navPosState.imu.accel[Z] = 0.0f;
        navPosState.imu.accelCount[Z] = 0;

        /* Correct baro offset */
        if (useGpsZ) {
            navPosState.baro.offset -= correctionGPSPos[X] * INAV_W_Z_GPS_P * dt;
        }

        /* Calculate altitude correction */
        correctionBaro = (navPosState.baro.rawAlt - navPosState.baro.offset) - navPosState.estimate[Z].pos;

        /* Calculate new Z-axis estimate */
        inavFilterPredict(&navPosState.estimate[Z], dt, inavAvgAccelZ);

        /* Correct Z-axis estimate from barometer */
        inavFilterCorrectPos(&navPosState.estimate[Z], dt, correctionBaro, INAV_W_Z_BARO);

        /* If GPS available, correct Z-axis estimate from GPS data */
        if (useGpsZ) {
            inavFilterCorrectPos(&navPosState.estimate[Z], dt, correctionGPSPos[Z], INAV_W_Z_GPS_P);
            inavFilterCorrectVel(&navPosState.estimate[Z], dt, correctionGPSVel[Z], INAV_W_Z_GPS_V);
        }

        /* Reset skipped updates counter */
        skippedEstimateZ = 0;

        /* Indicate that estimate was updated */
        navPosState.isUpdatesZ = true;
    }
    else {
        skippedEstimateZ++;

        if (skippedEstimateZ > INAV_SKIPPED_UPDATES_THRESHOLD) {
            /* gradually reset z velocity estimates */
            inavFilterCorrectVel(&navPosState.estimate[Z], dt, -navPosState.estimate[Z].vel, INAV_W_Z_RES_V);
        }
    }

    if (canEstimateXY) {
        /* Calculate average acceleration */
        float inavAvgAccelX = navPosState.imu.accel[X] / navPosState.imu.accelCount[X];
        float inavAvgAccelY = navPosState.imu.accel[Y] / navPosState.imu.accelCount[Y];
        navPosState.imu.accel[X] = navPosState.imu.accel[Y] = 0.0f;
        navPosState.imu.accelCount[X] = navPosState.imu.accelCount[Y] = 0;

        /* inertial filter prediction for position */
        inavFilterPredict(&navPosState.estimate[X], dt, inavAvgAccelX);
        inavFilterPredict(&navPosState.estimate[Y], dt, inavAvgAccelY);

        if (useGpsXY) {
            inavFilterCorrectPos(&navPosState.estimate[X], dt, correctionGPSPos[X], INAV_W_XY_GPS_P);
            inavFilterCorrectPos(&navPosState.estimate[Y], dt, correctionGPSPos[Y], INAV_W_XY_GPS_P);

            inavFilterCorrectVel(&navPosState.estimate[X], dt, correctionGPSVel[X], INAV_W_XY_GPS_V);
            inavFilterCorrectVel(&navPosState.estimate[Y], dt, correctionGPSVel[Y], INAV_W_XY_GPS_V);
        }

        /* Reset skipped updates counter */
        skippedEstimateXY = 0;

        /* Indicate that estimate was updated */
        navPosState.isUpdatesXY = true;
    }
    else {
        skippedEstimateXY++;

        if (skippedEstimateXY > INAV_SKIPPED_UPDATES_THRESHOLD) {
            /* gradually reset xy velocity estimates */
            inavFilterCorrectVel(&navPosState.estimate[X], dt, -navPosState.estimate[X].vel, INAV_W_XY_RES_V);
            inavFilterCorrectVel(&navPosState.estimate[Y], dt, -navPosState.estimate[Y].vel, INAV_W_XY_RES_V);
        }
    }
}

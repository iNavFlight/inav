#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "drivers/time.h"

#include "flight/governor.h"
#include "flight/variable_pitch.h"

#include "fc/settings.h"
#include "fc/runtime_config.h"
#include "sensors/esc_sensor.h"

#include "common/maths.h"


#define GOV_SAMPLE_TIME_US   50000      // 50000us = 50ms corresponding to 20Hz
#define GOV_TIME_CONSTANT_US 100000     // time constant tau 

#define GOV_KP               30.000f    // 5.000f
#define GOV_KI               15.000f    // 2.500f
#define GOV_KD               3.000f     // 0.500f
#define GOV_RPM_LIMIT_MIN    500.0f
#define GOV_RPM_LIMIT_MAX    30500.0f
#define GOV_ITERM_LIMIT      (GOV_RPM_LIMIT_MAX - GOV_RPM_LIMIT_MIN) * 0.5f     // 15000.0f

#define GOV_RPM_SETTLE_TIME  200000     // 200000us = 200ms time for the RPM to settle after throttle change
#define GOV_RPM_CORRECTION   5.0f       // target RPM correction in percent (why????)
#define GOV_DELTA_THROTTLE   1

#if defined(USE_ESC_SENSOR) && defined(USE_VARIABLE_PITCH)

static bool isGovernorActive = false;
static bool isNormalizingRpm = false;
static uint16_t governorThrottle = 0;
static uint16_t currentThrottle = 0;

static float targetRpm  = 5000.0f;
static timeUs_t deltaTime = 0;
static uint16_t sampleCount = 0;
static float rpmSamples = 0;

static govPid_t govPid = {
    .Kp = GOV_KP,
    .Ki = GOV_KI,
    .Kd = GOV_KD,
    .minIterm = -GOV_ITERM_LIMIT,
    .maxIterm = +GOV_ITERM_LIMIT,
    .minRpm = GOV_RPM_LIMIT_MIN, 
    .maxRpm = GOV_RPM_LIMIT_MAX,
    .dTerm = 0.0f,
    .iTerm = 0.0f,
    .prevError = 0.0f,
    .prevRpm = 0.0f,
    .newRpm = 0.0f,
    .T = GOV_SAMPLE_TIME_US * 0.000001f,       // in seconds
    .tau = GOV_TIME_CONSTANT_US * 0.000001f,
};


static void governorInit(govPid_t *pid) {
    pid->iTerm = 0.0f;
    pid->dTerm = 0.0f;
    pid->prevError = 0.0f;
    pid->prevRpm = 0.0f;
    pid->newRpm = 0.0f;
    pid->T = GOV_SAMPLE_TIME_US * 0.000001f;
    pid->tau =GOV_TIME_CONSTANT_US * 0.000001f;
    pid->Kp = helicopterConfig()->hc_gov_pid_P;
    pid->Ki = helicopterConfig()->hc_gov_pid_I;
    pid->Kd = helicopterConfig()->hc_gov_pid_D;
}


static float applyGovernorPidController(govPid_t *pid, float targetRpm, float currentRpm) {
    
    /* Error */
    float error = targetRpm - currentRpm;

    /* Proportional */
    float pTerm = pid->Kp * error;

    /* Integral */
    pid->iTerm += 0.5f * pid->Ki * pid->T * (error + pid->prevError);
    pid->iTerm = constrainf(pid->iTerm, pid->minIterm, pid->maxIterm);

    /* Derivative */
    pid->dTerm  = -(2.0f * pid->Kd * (currentRpm - pid->prevRpm)    // derivative on measurement, not on error to avoid nasty peaks
                +  (2.0f * pid->tau - pid->T) * pid->dTerm)
                /  (2.0f * pid->tau + pid->T);

    /* Compute output + apply limits */
    pid->newRpm = constrainf(pTerm + pid->iTerm + pid->dTerm, pid->minRpm, pid->maxRpm);

    /* Store current state for later use */
    pid->prevError = error;
    pid->prevRpm = currentRpm;

    /* Return result */
    return pid->newRpm;
}


uint16_t governorApply(uint16_t throttle) {

    govPid_t *pid = &govPid;

    /* Governor is not used. Return throttle as is. */
    if (helicopterConfig()->hc_governor_type == HC_GOVERNOR_OFF) {
        return throttle;
    }
    
    /* Target throttle near minimum or maximum, exit governor */
    if (throttle < 1050 || throttle > 1950 || !hasFullySpooledUp()) {
        isGovernorActive = false;
        governorInit(pid);
        return throttle;
    }

    /* Governor takes over throttle */
    if (!isGovernorActive) {
        governorInit(pid);
        deltaTime = microsISR() + GOV_SAMPLE_TIME_US;
        currentThrottle = throttle; 
        governorThrottle = throttle;
        isGovernorActive = true;
        isNormalizingRpm = true;
        rpmSamples = 0;
        sampleCount = 0;
    }

    /* Throttle input has changed. Wait until the new RPM
     * should have settled, then change setpoint accordingly. */
    if (throttle != currentThrottle && helicopterConfig()->hc_governor_type == HC_GOVERNOR_SIMPLE) {
        governorInit(pid);
        currentThrottle = throttle;
        governorThrottle = throttle;
        deltaTime = microsISR() + GOV_RPM_SETTLE_TIME;
        isNormalizingRpm = true;
        rpmSamples = 0;
        sampleCount = 0;
    }

    /* Get the current RPM from the ESC sensor */
    const escSensorData_t * escSensor = getEscTelemetry(getMainMotorNumber());
    uint32_t currentRpm = escSensor->rpm;

    /* Measured RPM value is zero, this means RPM is either
     * not available or unreliable, so do not bother with it. */
    if (currentRpm == 0) {
        isGovernorActive = false;
        return throttle;
    }

    /* Is the data reported by ESC sensor valid? */
    if (escSensor->dataAge <= ESC_DATA_MAX_AGE) {
        rpmSamples += currentRpm;
        sampleCount++;
    }

    /* Apply PID controller on average RPMs per sample time */
    if (microsISR() >= deltaTime && sampleCount > 0) {
        float averageRpm = rpmSamples / sampleCount;

        /* Set target RPM */
        if (isNormalizingRpm && helicopterConfig()->hc_governor_type == HC_GOVERNOR_SIMPLE) {
            /* Simple governor */
            targetRpm = averageRpm + (averageRpm * GOV_RPM_CORRECTION * 0.01f);
            isNormalizingRpm = false;
        } else if (helicopterConfig()->hc_governor_type == HC_GOVERNOR_SET) {
            /* Set governor */
            targetRpm = getHelicopterDesiredHeadspeed();
            targetRpm += (targetRpm * GOV_RPM_CORRECTION * 0.01f);
        }

        float newRpm = applyGovernorPidController(pid, targetRpm, averageRpm);
        rpmSamples = 0;
        sampleCount = 0;
        deltaTime = microsISR() + GOV_SAMPLE_TIME_US;
    
        /* Calculate new throttle */
        if(newRpm > averageRpm) {
            governorThrottle += GOV_DELTA_THROTTLE;
        } else if (newRpm < averageRpm) {
            governorThrottle -= GOV_DELTA_THROTTLE;
        }
        governorThrottle = constrain(governorThrottle, 1050, 1950);
    }

    return governorThrottle;
}

#endif
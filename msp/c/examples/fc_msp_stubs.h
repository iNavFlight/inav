#pragma once
/* The INAV symbols these handlers use. A real firmware build gets them from
 * the usual headers; this file exists only so the example is compilable on its
 * own and the handler bodies can be type-checked. Nothing here is generated. */
#include <stdint.h>
#include <math.h>

typedef enum { MSP_RESULT_ACK = 1, MSP_RESULT_ERROR = -1,
               MSP_RESULT_NO_REPLY = 0, MSP_RESULT_CMD_UNKNOWN = -2 } mspResult_e;

#define FC_VERSION_MAJOR 9
#define FC_VERSION_MINOR 1
#define FC_VERSION_PATCH_LEVEL 0
#define FD_ROLL 0
#define FD_PITCH 1
#define FD_YAW 2
#define SETTING_CONSTANT_ROLL_PITCH_RATE_MIN 6
#define SETTING_CONSTANT_ROLL_PITCH_RATE_MAX 180
#define SETTING_YAW_RATE_MIN 2
#define SETTING_YAW_RATE_MAX 180
#define SETTING_TPA_RATE_MAX 100
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define constrain(v, lo, hi) ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))

typedef struct { uint8_t rcExpo8, rcYawExpo8, rates[3]; } controlRates_t;
typedef struct { uint8_t rcMid8, rcExpo8, dynPID; uint16_t pa_breakpoint; } throttleRates_t;
typedef struct { controlRates_t stabilized, manual; throttleRates_t throttle; } controlConfig_t;
extern const controlConfig_t *currentControlProfile;
void schedulePidGainsUpdate(void);

typedef struct { uint8_t acc_hardware; } accelerometerConfig_t;
const accelerometerConfig_t *accelerometerConfig(void);

typedef struct { float accADCf[3]; } acc_t;
extern acc_t acc;
float gyroRateDps(int axis);

bool mspApplyGlobalTarget(int32_t lat, int32_t lon, int32_t alt,
                          uint8_t datum, int32_t loiterRadius);

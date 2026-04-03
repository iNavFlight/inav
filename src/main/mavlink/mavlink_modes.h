#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "fc/runtime_config.h"

/** @brief A mapping of plane flight modes for custom_mode field of heartbeat. */
typedef enum APM_PLANE_MODE
{
   PLANE_MODE_MANUAL=0,
   PLANE_MODE_CIRCLE=1,
   PLANE_MODE_STABILIZE=2,
   PLANE_MODE_TRAINING=3,
   PLANE_MODE_ACRO=4,
   PLANE_MODE_FLY_BY_WIRE_A=5,
   PLANE_MODE_FLY_BY_WIRE_B=6,
   PLANE_MODE_CRUISE=7,
   PLANE_MODE_AUTOTUNE=8,
   PLANE_MODE_AUTO=10,
   PLANE_MODE_RTL=11,
   PLANE_MODE_LOITER=12,
   PLANE_MODE_TAKEOFF=13,
   PLANE_MODE_AVOID_ADSB=14,
   PLANE_MODE_GUIDED=15,
   PLANE_MODE_INITIALIZING=16,
   PLANE_MODE_QSTABILIZE=17,
   PLANE_MODE_QHOVER=18,
   PLANE_MODE_QLOITER=19,
   PLANE_MODE_QLAND=20,
   PLANE_MODE_QRTL=21,
   PLANE_MODE_QAUTOTUNE=22,
   PLANE_MODE_QACRO=23,
   PLANE_MODE_THERMAL=24,
   PLANE_MODE_LOITER_ALT_QLAND=25,
   PLANE_MODE_AUTOLAND=26,
   PLANE_MODE_ENUM_END=27,
} APM_PLANE_MODE;

/** @brief A mapping of copter flight modes for custom_mode field of heartbeat. */
typedef enum APM_COPTER_MODE
{
   COPTER_MODE_STABILIZE=0,
   COPTER_MODE_ACRO=1,
   COPTER_MODE_ALT_HOLD=2,
   COPTER_MODE_AUTO=3,
   COPTER_MODE_GUIDED=4,
   COPTER_MODE_LOITER=5,
   COPTER_MODE_RTL=6,
   COPTER_MODE_CIRCLE=7,
   COPTER_MODE_LAND=9,
   COPTER_MODE_DRIFT=11,
   COPTER_MODE_SPORT=13,
   COPTER_MODE_FLIP=14,
   COPTER_MODE_AUTOTUNE=15,
   COPTER_MODE_POSHOLD=16,
   COPTER_MODE_BRAKE=17,
   COPTER_MODE_THROW=18,
   COPTER_MODE_AVOID_ADSB=19,
   COPTER_MODE_GUIDED_NOGPS=20,
   COPTER_MODE_SMART_RTL=21,
   COPTER_MODE_FLOWHOLD=22,
   COPTER_MODE_FOLLOW=23,
   COPTER_MODE_ZIGZAG=24,
   COPTER_MODE_SYSTEMID=25,
   COPTER_MODE_AUTOROTATE=26,
   COPTER_MODE_AUTO_RTL=27,
   COPTER_MODE_TURTLE=28,
   COPTER_MODE_ENUM_END=29,
} APM_COPTER_MODE;

typedef struct mavlinkModeSelection_s {
    flightModeForTelemetry_e flightMode;
    uint8_t customMode;
} mavlinkModeSelection_t;

bool mavlinkIsFixedWingVehicle(void);
uint8_t mavlinkGetVehicleType(void);
uint8_t mavlinkGetAutopilotEnum(void);
mavlinkModeSelection_t mavlinkSelectMode(void);
void mavlinkSendAvailableModesForCurrentMode(void);
void mavlinkSendCurrentMode(const mavlinkModeSelection_t *modeSelection);

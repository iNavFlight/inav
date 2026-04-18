#include "mavlink/mavlink_internal.h"

#include "mavlink/mavlink_modes.h"
#include "mavlink/mavlink_runtime.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

typedef struct mavlinkModeDescriptor_s {
    uint8_t customMode;
    const char *name;
} mavlinkModeDescriptor_t;

static COPTER_MODE inavToArduCopterMap(flightModeForTelemetry_e flightMode)
{
    switch (flightMode)
    {
        case FLM_ACRO:          return COPTER_MODE_ACRO;
        case FLM_ACRO_AIR:      return COPTER_MODE_ACRO;
        case FLM_ANGLE:         return COPTER_MODE_STABILIZE;
        case FLM_HORIZON:       return COPTER_MODE_STABILIZE;
        case FLM_ANGLEHOLD:     return COPTER_MODE_STABILIZE;
        case FLM_ALTITUDE_HOLD: return COPTER_MODE_ALT_HOLD;
        case FLM_POSITION_HOLD:
            {
                if (isGCSValid()) {
                    return COPTER_MODE_GUIDED;
                } else {
                    return COPTER_MODE_POSHOLD;
                }
            }
        case FLM_RTH:           return COPTER_MODE_RTL;
        case FLM_MISSION:       return COPTER_MODE_AUTO;
        case FLM_LAUNCH:        return COPTER_MODE_THROW;
        case FLM_FAILSAFE:
            {
                if (failsafePhase() == FAILSAFE_RETURN_TO_HOME) {
                    return COPTER_MODE_RTL;
                } else if (failsafePhase() == FAILSAFE_LANDING) {
                    return COPTER_MODE_LAND;
                } else {
                    return COPTER_MODE_RTL;
                }
            }
        default:                return COPTER_MODE_STABILIZE;
    }
}

static PLANE_MODE inavToArduPlaneMap(flightModeForTelemetry_e flightMode)
{
    switch (flightMode)
    {
        case FLM_MANUAL:        return PLANE_MODE_MANUAL;
        case FLM_ACRO:          return PLANE_MODE_ACRO;
        case FLM_ACRO_AIR:      return PLANE_MODE_ACRO;
        case FLM_ANGLE:         return PLANE_MODE_FLY_BY_WIRE_A;
        case FLM_HORIZON:       return PLANE_MODE_STABILIZE;
        case FLM_ANGLEHOLD:     return PLANE_MODE_STABILIZE;
        case FLM_ALTITUDE_HOLD: return PLANE_MODE_FLY_BY_WIRE_B;
        case FLM_POSITION_HOLD:
            {
                if (isGCSValid()) {
                    return PLANE_MODE_GUIDED;
                } else {
                    return PLANE_MODE_LOITER;
                }
            }
        case FLM_RTH:           return PLANE_MODE_RTL;
        case FLM_MISSION:       return PLANE_MODE_AUTO;
        case FLM_CRUISE:        return PLANE_MODE_CRUISE;
        case FLM_LAUNCH:        return PLANE_MODE_TAKEOFF;
        case FLM_FAILSAFE:
            {
                if (failsafePhase() == FAILSAFE_RETURN_TO_HOME) {
                    return PLANE_MODE_RTL;
                } else if (failsafePhase() == FAILSAFE_LANDING) {
                    return PLANE_MODE_AUTOLAND;
                } else {
                    return PLANE_MODE_RTL;
                }
            }
        default:                return PLANE_MODE_MANUAL;
    }
}

static const mavlinkModeDescriptor_t planeModes[] = {
    { PLANE_MODE_MANUAL,        "MANUAL" },
    { PLANE_MODE_ACRO,          "ACRO" },
    { PLANE_MODE_STABILIZE,     "STABILIZE" },
    { PLANE_MODE_FLY_BY_WIRE_A, "FBWA" },
    { PLANE_MODE_FLY_BY_WIRE_B, "FBWB" },
    { PLANE_MODE_CRUISE,        "CRUISE" },
    { PLANE_MODE_AUTO,          "AUTO" },
    { PLANE_MODE_RTL,           "RTL" },
    { PLANE_MODE_LOITER,        "LOITER" },
    { PLANE_MODE_TAKEOFF,       "TAKEOFF" },
    { PLANE_MODE_GUIDED,        "GUIDED" },
};

static const mavlinkModeDescriptor_t copterModes[] = {
    { COPTER_MODE_ACRO,      "ACRO" },
    { COPTER_MODE_STABILIZE, "STABILIZE" },
    { COPTER_MODE_ALT_HOLD,  "ALT_HOLD" },
    { COPTER_MODE_POSHOLD,   "POSHOLD" },
    { COPTER_MODE_LOITER,    "LOITER" },
    { COPTER_MODE_AUTO,      "AUTO" },
    { COPTER_MODE_GUIDED,    "GUIDED" },
    { COPTER_MODE_RTL,       "RTL" },
    { COPTER_MODE_LAND,      "LAND" },
    { COPTER_MODE_BRAKE,     "BRAKE" },
    { COPTER_MODE_THROW,     "THROW" },
    { COPTER_MODE_SMART_RTL, "SMART_RTL" },
    { COPTER_MODE_DRIFT,     "DRIFT" },
};

static bool mavlinkPlaneModeIsConfigured(uint8_t customMode)
{
    switch ((PLANE_MODE)customMode) {
        case PLANE_MODE_MANUAL:
            return isModeActivationConditionPresent(BOXMANUAL);
        case PLANE_MODE_ACRO:
            return true;
        case PLANE_MODE_STABILIZE:
            return isModeActivationConditionPresent(BOXHORIZON) ||
                isModeActivationConditionPresent(BOXANGLEHOLD);
        case PLANE_MODE_FLY_BY_WIRE_A:
            return isModeActivationConditionPresent(BOXANGLE);
        case PLANE_MODE_FLY_BY_WIRE_B:
            return isModeActivationConditionPresent(BOXNAVALTHOLD);
        case PLANE_MODE_CRUISE:
            return isModeActivationConditionPresent(BOXNAVCRUISE) ||
                (isModeActivationConditionPresent(BOXNAVCOURSEHOLD) &&
                isModeActivationConditionPresent(BOXNAVALTHOLD));
        case PLANE_MODE_AUTO:
            return isModeActivationConditionPresent(BOXNAVWP);
        case PLANE_MODE_RTL:
            return isModeActivationConditionPresent(BOXNAVRTH);
        case PLANE_MODE_GUIDED:
            return isModeActivationConditionPresent(BOXNAVPOSHOLD) &&
                isModeActivationConditionPresent(BOXGCSNAV);
        case PLANE_MODE_LOITER:
            return isModeActivationConditionPresent(BOXNAVPOSHOLD);
        case PLANE_MODE_TAKEOFF:
            return isModeActivationConditionPresent(BOXNAVLAUNCH);
        default:
            return false;
    }
}

static bool mavlinkCopterModeIsConfigured(uint8_t customMode)
{
    switch ((COPTER_MODE)customMode) {
        case COPTER_MODE_ACRO:
            return true;
        case COPTER_MODE_STABILIZE:
            return isModeActivationConditionPresent(BOXANGLE) ||
                isModeActivationConditionPresent(BOXHORIZON) ||
                isModeActivationConditionPresent(BOXANGLEHOLD);
        case COPTER_MODE_ALT_HOLD:
            return isModeActivationConditionPresent(BOXNAVALTHOLD);
        case COPTER_MODE_GUIDED:
            return isModeActivationConditionPresent(BOXNAVPOSHOLD) &&
                isModeActivationConditionPresent(BOXGCSNAV);
        case COPTER_MODE_POSHOLD:
            return isModeActivationConditionPresent(BOXNAVPOSHOLD);
        case COPTER_MODE_RTL:
            return isModeActivationConditionPresent(BOXNAVRTH);
        case COPTER_MODE_AUTO:
            return isModeActivationConditionPresent(BOXNAVWP);
        case COPTER_MODE_THROW:
            return isModeActivationConditionPresent(BOXNAVLAUNCH);
        case COPTER_MODE_BRAKE:
            return isModeActivationConditionPresent(BOXBRAKING);
        default:
            return false;
    }
}

static void mavlinkSendAvailableModes(const mavlinkModeDescriptor_t *modes, uint8_t count, uint8_t currentCustom,
    bool (*isModeConfigured)(uint8_t customMode))
{
    uint8_t availableCount = 0;
    for (uint8_t i = 0; i < count; i++) {
        if (isModeConfigured(modes[i].customMode)) {
            availableCount++;
        }
    }

    if (availableCount == 0) {
        return;
    }

    uint8_t modeIndex = 0;
    for (uint8_t i = 0; i < count; i++) {
        if (!isModeConfigured(modes[i].customMode)) {
            continue;
        }

        modeIndex++;

        mavlink_msg_available_modes_pack(
            mavSystemId,
            mavComponentId,
            &mavSendMsg,
            availableCount,
            modeIndex,
            MAV_STANDARD_MODE_NON_STANDARD,
            modes[i].customMode,
            0,
            modes[i].name);

        mavlinkSendMessage();

        if (modes[i].customMode == currentCustom) {
            const mavlinkModeSelection_t modeSelection = {
                .flightMode = FLM_MANUAL,
                .customMode = currentCustom,
            };
            mavlinkSendCurrentMode(&modeSelection);
        }
    }
}

bool mavlinkIsFixedWingVehicle(void)
{
    return STATE(FIXED_WING_LEGACY) || mixerConfig()->platformType == PLATFORM_AIRPLANE;
}

uint8_t mavlinkGetVehicleType(void)
{
    switch (mixerConfig()->platformType)
    {
        case PLATFORM_MULTIROTOR:
            return MAV_TYPE_QUADROTOR;
        case PLATFORM_TRICOPTER:
            return MAV_TYPE_TRICOPTER;
        case PLATFORM_AIRPLANE:
            return MAV_TYPE_FIXED_WING;
        case PLATFORM_ROVER:
            return MAV_TYPE_GROUND_ROVER;
        case PLATFORM_BOAT:
            return MAV_TYPE_SURFACE_BOAT;
        case PLATFORM_HELICOPTER:
            return MAV_TYPE_HELICOPTER;
        default:
            return MAV_TYPE_GENERIC;
    }
}

uint8_t mavlinkGetAutopilotEnum(void)
{
    if (mavAutopilotType == MAVLINK_AUTOPILOT_ARDUPILOT) {
        return MAV_AUTOPILOT_ARDUPILOTMEGA;
    }

    return MAV_AUTOPILOT_GENERIC;
}

mavlinkModeSelection_t mavlinkSelectMode(void)
{
    mavlinkModeSelection_t modeSelection;
    modeSelection.flightMode = getFlightModeForTelemetry();

    if (mavlinkIsFixedWingVehicle()) {
        modeSelection.customMode = (uint8_t)inavToArduPlaneMap(modeSelection.flightMode);
    } else {
        modeSelection.customMode = (uint8_t)inavToArduCopterMap(modeSelection.flightMode);
    }

    return modeSelection;
}

void mavlinkSendAvailableModesForCurrentMode(void)
{
    const mavlinkModeSelection_t modeSelection = mavlinkSelectMode();

    if (mavlinkIsFixedWingVehicle()) {
        mavlinkSendAvailableModes(planeModes, (uint8_t)ARRAYLEN(planeModes), modeSelection.customMode, mavlinkPlaneModeIsConfigured);
    } else {
        mavlinkSendAvailableModes(copterModes, (uint8_t)ARRAYLEN(copterModes), modeSelection.customMode, mavlinkCopterModeIsConfigured);
    }
}

void mavlinkSendCurrentMode(const mavlinkModeSelection_t *modeSelection)
{
    mavlink_msg_current_mode_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        MAV_STANDARD_MODE_NON_STANDARD,
        modeSelection->customMode,
        modeSelection->customMode);
    mavlinkSendMessage();
}

#endif

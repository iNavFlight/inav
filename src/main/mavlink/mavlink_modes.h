#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "fc/runtime_config.h"

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

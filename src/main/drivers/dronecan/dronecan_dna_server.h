#pragma once

#include <stdint.h>
#include "config/parameter_group.h"
#include "libcanard/canard.h"

#ifdef USE_DRONECAN

extern CanardInstance canard;

#define DNA_UNIQUE_ID_LENGTH       16
#define DRONECAN_DNA_MAX_NODE_ID  125  /* 126-127 reserved for network maintenance tools */

typedef struct {
    uint8_t uniqueId[DNA_UNIQUE_ID_LENGTH];
    uint8_t nodeId;
} dnaAllocationEntry_t;

typedef struct {
    dnaAllocationEntry_t entries[DRONECAN_MAX_NODES];
} dnaServerData_t;

PG_DECLARE(dnaServerData_t, dnaServerData);

void dronecanDnaHandleAllocation(CanardInstance *ins, CanardRxTransfer *transfer);

#endif // USE_DRONECAN

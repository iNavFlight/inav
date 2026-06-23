#pragma once

#include "mavlink/mavlink_types.h"

typedef enum {
    MAVLINK_FC_DISPATCH_NOT_HANDLED = 0,
    MAVLINK_FC_DISPATCH_HANDLED_NO_ACTIVITY,
    MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY,
} mavlinkFcDispatchResult_e;

mavlinkFcDispatchResult_e mavlinkFcDispatchIncomingMessage(uint8_t ingressPortIndex);

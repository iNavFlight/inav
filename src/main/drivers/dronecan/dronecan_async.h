#pragma once

#include "libcanard/canard.h"

#ifdef USE_DRONECAN

/* Called from onTransferReceived() for every CanardTransferTypeResponse
   frame - matches it against the single in-flight async request slot
   (dronecanAsyncSlot, declared in dronecan.h) and decodes the response. */
void dronecanAsyncHandleServiceResponse(CanardInstance *ins, CanardRxTransfer *transfer);

/* Called once per dronecanUpdate() tick while in STATE_DRONECAN_NORMAL -
   expires a pending request that never got a response. */
void dronecanAsyncCheckTimeout(void);

#endif // USE_DRONECAN

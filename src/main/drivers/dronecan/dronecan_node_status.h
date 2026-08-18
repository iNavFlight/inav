#pragma once

#include "common/time.h"
#include "libcanard/canard.h"

#ifdef USE_DRONECAN

#include <dronecan_msgs.h>

/* Dispatch target for incoming NodeStatus broadcasts from other nodes -
   called from dronecan.c's onTransferReceived(). Updates the live node
   table also consulted by dronecan_dna_server.c (via dronecanGetNode(),
   declared in dronecan.h) to avoid handing out node IDs already seen on
   the bus. */
void dronecanNodeStatusHandleBroadcast(CanardInstance *ins, CanardRxTransfer *transfer);

/* Broadcasts our own NodeStatus heartbeat and prunes nodes we haven't
   heard a NodeStatus from in DRONECAN_NODE_STALE_TIMEOUT_MS. Called once
   per second from dronecan.c's process1HzTasks(). */
void dronecanNodeStatusUpdate(timeUs_t timestamp_usec);

/* Our own current NodeStatus (uptime refreshed on each call), for
   dronecan.c's handle_GetNodeInfo() to embed in a GetNodeInfo response. */
struct uavcan_protocol_NodeStatus dronecanGetOwnNodeStatus(void);

#endif // USE_DRONECAN

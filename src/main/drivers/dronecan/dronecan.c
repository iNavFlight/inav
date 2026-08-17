#include "platform.h"
#include "common/log.h"
#include "common/time.h"
#include "drivers/time.h"
#include "drivers/nvic.h"
#include "build/atomic.h"
#include <stdint.h>
#include <stdlib.h>
#include "fc/settings.h"
#include "build/version.h"
#include "sensors/diagnostics.h"
#include "fc/runtime_config.h"
#if defined(USE_DRONECAN)

#include "io/gps.h"
#include "sensors/battery_sensor_dronecan.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "libcanard/canard_stm32_driver.h"
#include "libcanard/canard.h"
#include "dronecan.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <dronecan_msgs.h>

/* Private variables ---------------------------------------------------------*/

static CanardInstance canard;
static uint8_t memory_pool[1024];
static struct uavcan_protocol_NodeStatus node_status;

PG_REGISTER_WITH_RESET_TEMPLATE(dronecanConfig_t, dronecanConfig, PG_DRONECAN_CONFIG, 0);

PG_RESET_TEMPLATE(dronecanConfig_t, dronecanConfig,
    .nodeID = SETTING_DRONECAN_NODE_ID_DEFAULT,
    .bitRateKbps = SETTING_DRONECAN_BITRATE_KBPS_DEFAULT
);

static dronecanState_e dronecanState = STATE_DRONECAN_INIT;
dronecanAsyncSlot_t dronecanAsyncSlot = { .state = DRONECAN_ASYNC_IDLE };

#ifdef UNIT_TEST
uint8_t activeNodeCount = 0;
dronecanNodeInfo_t nodeTable[DRONECAN_MAX_NODES];
static volatile uint32_t txErrCount = 0;
static uint32_t busOffCount = 0;
#else
static uint8_t activeNodeCount = 0;
static dronecanNodeInfo_t nodeTable[DRONECAN_MAX_NODES];
static volatile uint32_t txErrCount = 0;
static uint32_t busOffCount = 0;
#endif

/* Forward declarations ------------------------------------------------------*/

static void processCanardTxQueueSafe(void);
static void process1HzTasks(timeUs_t timestamp_usec);
#ifdef UNIT_TEST
bool shouldAcceptTransfer(const CanardInstance *ins, uint64_t *out_data_type_signature, uint16_t data_type_id, CanardTransferType transfer_type, uint8_t source_node_id);
void handle_NodeStatus(CanardInstance *ins, CanardRxTransfer *transfer);
void onTransferReceived(CanardInstance *ins, CanardRxTransfer *transfer);
#else
static bool shouldAcceptTransfer(const CanardInstance *ins, uint64_t *out_data_type_signature, uint16_t data_type_id, CanardTransferType transfer_type, uint8_t source_node_id);
static void onTransferReceived(CanardInstance *ins, CanardRxTransfer *transfer);
#endif

// ---- Public API -------------------------------------------------------------

void dronecanInit(void)
{
    uint32_t bitrate = 500000;

    switch (dronecanConfig()->bitRateKbps){
        case DRONECAN_BITRATE_125KBPS:
            bitrate = 125000;
            break;

        case DRONECAN_BITRATE_250KBPS:
            bitrate = 250000;
            break;

        case DRONECAN_BITRATE_500KBPS:
            bitrate = 500000;
            break;

        case DRONECAN_BITRATE_1000KBPS:
            bitrate = 1000000;
            break;

        case DRONECAN_BITRATE_COUNT:
            LOG_ERROR(SYSTEM, "Undefined bitrate set in configuration. 500kbps selected");
            bitrate = 500000;
            break;

        default:
            LOG_ERROR(SYSTEM, "Invalid bitrate setting, defaulting to 500kbps");
            bitrate = 500000;
            break;
    }
    if(canardSTM32CAN1_Init(bitrate) != CANARD_OK)
    {
        LOG_ERROR(CAN, "Unable to initialize the CAN peripheral");
        dronecanState = STATE_DRONECAN_FAILED;
        return;
    }
    /*
    Initializing the Libcanard instance.
    */
    canardInit(&canard,
               memory_pool,
               sizeof(memory_pool),
               onTransferReceived,
               shouldAcceptTransfer,
               NULL);

    // Could use DNA (Dynamic Node Allocation) by following example in esc_node.c but that requires a lot of setup and I'm not too sure of what advantage it brings
    // Instead, set a different NODE_ID for each device on the CAN bus by configuring node_settings
    if (dronecanConfig()->nodeID > 0) {
	      canardSetLocalNodeID(&canard, dronecanConfig()->nodeID);
    } else {
	      LOG_DEBUG(CAN, "Node ID is 0, this node is anonymous and can't transmit most messages. Please update this in config");
    }
}

void dronecanUpdate(timeUs_t currentTimeUs)
{
    static timeUs_t next_1hz_service_at = 0;
    static timeUs_t busoffTimeUs = 0;
    CanardCANFrame rx_frame;
    int numMessagesToProcess = 0;
    canardProtocolStatus_t protocolStatus = {};
    uint64_t timestamp;
    int16_t rx_res;

    switch(dronecanState) {
        case STATE_DRONECAN_INIT:
            next_1hz_service_at = currentTimeUs + 1000000ULL;  // First 1Hz tick in 1 second
            dronecanState = STATE_DRONECAN_NORMAL;
            break;

        case STATE_DRONECAN_NORMAL:
            processCanardTxQueueSafe();

            // Check for and expire any pending async requests that have timed out.
            if (dronecanAsyncSlot.state == DRONECAN_ASYNC_PENDING &&
                millis() - dronecanAsyncSlot.requested_at_ms >= DRONECAN_ASYNC_TIMEOUT_MS) {
                dronecanAsyncSlot.state = DRONECAN_ASYNC_ERROR;
            }

             for (numMessagesToProcess = canardSTM32GetRxFifoFillLevel(); numMessagesToProcess > 0; numMessagesToProcess--)
             {
	            timestamp = millis() * 1000ULL;
	            rx_res = canardSTM32Receive(&rx_frame);

	             if (rx_res < 0) {
		             LOG_DEBUG(CAN, "Receive error %d", rx_res);
	             }
	             else if (rx_res > 0)        // Success - process the frame
	             {
		             ATOMIC_BLOCK(NVIC_PRIO_CAN) {
		                 canardHandleRxFrame(&canard, &rx_frame, timestamp);
		             }
	             }
             }
            // Drain any TX frames queued by RX handlers (e.g. GetNodeInfo responses)
            // in the same task cycle so multi-frame transfers complete before timeout.
            processCanardTxQueueSafe();

            if (currentTimeUs >= next_1hz_service_at)
            {
		        next_1hz_service_at += 1000000ULL;
		        process1HzTasks(currentTimeUs);
                processCanardTxQueueSafe();

                canardSTM32GetProtocolStatus(&protocolStatus);
                if (protocolStatus.BusOff != 0 || protocolStatus.ErrorPassive != 0) {
                    LOG_DEBUG(CAN, "CAN status: BusOff=%" PRIu32 " ErrorPassive=%" PRIu32, protocolStatus.BusOff, protocolStatus.ErrorPassive);
                }

                uint32_t rxDrops = canardSTM32GetAndClearRxDropCount();
                uint32_t txErrs;
                ATOMIC_BLOCK(NVIC_PRIO_CAN) {
                    txErrs = txErrCount;
                    txErrCount = 0;
                }
                if (rxDrops > 0) {
                    LOG_DEBUG(CAN, "RX drops: %" PRIu32, rxDrops);
                }
                if (txErrs > 0) {
                    LOG_DEBUG(CAN, "TX errors: %" PRIu32, txErrs);
                }

                if (protocolStatus.BusOff != 0) {
                    dronecanState = STATE_DRONECAN_BUS_OFF;
                    busoffTimeUs = currentTimeUs;
                    busOffCount++;
                }
            }
            break;

        case STATE_DRONECAN_BUS_OFF:
            if(currentTimeUs > (busoffTimeUs + 20000)) { // Wait 20ms: worst-case 128x11 recovery is 11.264ms at 125kbps
                static uint8_t busoff_retries = 0;
                canardSTM32RecoverFromBusOff();
                busoffTimeUs = currentTimeUs;
                canardSTM32GetProtocolStatus(&protocolStatus);
                if(protocolStatus.BusOff == 0) {
                    busoff_retries = 0;
                    dronecanState = STATE_DRONECAN_NORMAL;
                } else if (++busoff_retries >= 50) {
                    // ~1 second of 20ms recovery attempts with no success — permanent fault
                    busoff_retries = 0;
                    dronecanState = STATE_DRONECAN_FAILED;
                    LOG_DEBUG(CAN, "DroneCAN: bus-off recovery failed after 50 attempts, entering FAILED state");
                }
            }
            break;

        case STATE_DRONECAN_FAILED:
            break;

        default:
            break;

    }

}

dronecanState_e dronecanGetState(void)
{
    return dronecanState;
}

uint8_t dronecanGetNodeCount(void)
{
    return activeNodeCount;
}

uint32_t dronecanGetBitrateKbps(void)
{
    switch (dronecanConfig()->bitRateKbps){
        case DRONECAN_BITRATE_125KBPS:
            return 125;

        case DRONECAN_BITRATE_250KBPS:
            return 250;

        case DRONECAN_BITRATE_500KBPS:
            return 500;

        case DRONECAN_BITRATE_1000KBPS:
            return 1000;

        case DRONECAN_BITRATE_COUNT:
        default:
            return 500;
    }
}

const dronecanNodeInfo_t *dronecanGetNode(uint8_t index) {
    if (index < activeNodeCount) return &nodeTable[index];
    return NULL;
}

uint32_t dronecanGetBusOffCount(void)
{
    return busOffCount;
}

CanardPoolAllocatorStatistics dronecanGetPoolStats(void)
{
    return canardGetPoolAllocatorStatistics(&canard);
}

// ---- ISR / HAL callbacks ----------------------------------------------------

#if defined(STM32H7) || defined(STM32F7)
/* Called from TX-complete ISR only. Already in interrupt context — no NVIC masking needed.
   For main-loop use, call processCanardTxQueueSafe() instead. */
static void processCanardTxQueue(void) {
	// Transmitting
	for (const CanardCANFrame *tx_frame ; (tx_frame = canardPeekTxQueue(&canard)) != NULL;)
    {
        const int16_t tx_res = canardSTM32Transmit(tx_frame);

		if (tx_res < 0) {
			txErrCount++;  // logged from main loop at 1Hz
			canardPopTxQueue(&canard);  // Error - discard frame
		} else if (tx_res > 0) {
			canardPopTxQueue(&canard);  // Success - remove from queue
		} else {
			// tx_res == 0: TX FIFO full, retry later
			break;
		}
	}
}
#endif

#if defined(STM32H7)
void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes)
{
    UNUSED(hfdcan);
    UNUSED(BufferIndexes);
    processCanardTxQueue();
}
#endif
#if defined(STM32F7)
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) { UNUSED(hcan); processCanardTxQueue(); }
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan) { UNUSED(hcan); processCanardTxQueue(); }
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan) { UNUSED(hcan); processCanardTxQueue(); }
#endif

// ---- Internal helpers -------------------------------------------------------

static void processCanardTxQueueSafe(void) {
    for (;;) {
        bool queueEmpty = false;
        bool hwFull = false;

        ATOMIC_BLOCK(NVIC_PRIO_CAN) {
            const CanardCANFrame *tx_frame = canardPeekTxQueue(&canard);
            if (tx_frame == NULL) {
                queueEmpty = true;
            } else {
                const int16_t tx_res = canardSTM32Transmit(tx_frame);  // HAL register write, ~1µs
                if (tx_res != 0) {
                    if (tx_res < 0) {
                        LOG_DEBUG(CAN, "Transmit error %d", tx_res);
                    }
                    canardPopTxQueue(&canard);
                } else {
                    hwFull = true;  // HW TX full, ISR will refill when a slot opens
                }
            }
        }

        if (queueEmpty || hwFull) {
            break;
        }
    }
}

// NOTE: All canard handlers and senders are based on this reference: https://dronecan.github.io/Specification/7._List_of_standard_data_types/
// Alternatively, you can look at the corresponding generated header file in the dsdlc_generated folder

static dronecanNodeInfo_t *findNodeByID(uint8_t nodeID) {
    for (uint8_t i = 0; i < activeNodeCount; i++) {
        if (nodeTable[i].nodeID == nodeID) {
            return &nodeTable[i];
        }
    }
    return NULL;
}

const dronecanNodeInfo_t *dronecanGetNodeByID(uint8_t nodeID) {
    return findNodeByID(nodeID);
}


bool dronecanAsyncRequest(uint8_t service_id, uint8_t node_id, const void *payload)
{
    if (dronecanAsyncSlot.state == DRONECAN_ASYNC_PENDING &&
        millis() - dronecanAsyncSlot.requested_at_ms < DRONECAN_ASYNC_TIMEOUT_MS) {
        return false;
    }

    // PARAM_GETSET_REQUEST is the largest payload; zero-init prevents garbage in UAVCAN reserved bits
    uint8_t buffer[UAVCAN_PROTOCOL_PARAM_GETSET_REQUEST_MAX_SIZE];
    memset(buffer, 0, sizeof(buffer));
    uint16_t len = 0;
    uint64_t signature = 0;
    const uint8_t *buf_ptr = NULL;

    switch (service_id) {
        case DRONECAN_SERVICE_GETNODEINFO:
            signature = UAVCAN_PROTOCOL_GETNODEINFO_SIGNATURE;
            len = 0;
            break;

        case DRONECAN_SERVICE_PARAM_GETSET: {
            if (!payload) return false;
            const dronecanParamRequest_t *req = (const dronecanParamRequest_t *)payload;
            struct uavcan_protocol_param_GetSetRequest getset;
            memset(&getset, 0, sizeof(getset));
            getset.index = req->index;
            if (req->is_write) {
                getset.value.union_tag = (enum uavcan_protocol_param_Value_type_t)req->value_type;
                switch (req->value_type) {
                    case DRONECAN_PARAM_TYPE_INT:
                        getset.value.integer_value = req->value_int;
                        break;
                    case DRONECAN_PARAM_TYPE_FLOAT:
                        getset.value.real_value = req->value_float;
                        break;
                    case DRONECAN_PARAM_TYPE_BOOL:
                        getset.value.boolean_value = req->value_bool;
                        break;
                    case DRONECAN_PARAM_TYPE_STRING: {
                        uint8_t slen = req->value_str_len < sizeof(getset.value.string_value.data)
                                       ? req->value_str_len : sizeof(getset.value.string_value.data);
                        getset.value.string_value.len = slen;
                        memcpy(getset.value.string_value.data, req->value_str, slen);
                        break;
                    }
                    default:
                        getset.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY;
                        break;
                }
            }
            uint8_t nlen = req->req_name_len < sizeof(getset.name.data)
                           ? req->req_name_len : sizeof(getset.name.data);
            getset.name.len = nlen;
            memcpy(getset.name.data, req->req_name, nlen);
            len = uavcan_protocol_param_GetSetRequest_encode(&getset, buffer);
            buf_ptr = buffer;
            signature = UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE;
            break;
        }

        case DRONECAN_SERVICE_EXECUTE_OPCODE: {
            if (!payload) return false;
            const uint8_t *opcode = (const uint8_t *)payload;
            struct uavcan_protocol_param_ExecuteOpcodeRequest req;
            memset(&req, 0, sizeof(req));
            req.opcode = *opcode;
            req.argument = 0;
            len = uavcan_protocol_param_ExecuteOpcodeRequest_encode(&req, buffer);
            buf_ptr = buffer;
            signature = UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_SIGNATURE;
            break;
        }

        case DRONECAN_SERVICE_RESTART_NODE: {
            struct uavcan_protocol_RestartNodeRequest req;
            memset(&req, 0, sizeof(req));
            req.magic_number = UAVCAN_PROTOCOL_RESTARTNODE_REQUEST_MAGIC_NUMBER;
            len = uavcan_protocol_RestartNodeRequest_encode(&req, buffer);
            buf_ptr = buffer;
            signature = UAVCAN_PROTOCOL_RESTARTNODE_SIGNATURE;
            break;
        }

        default:
            return false;
    }

    // buf_ptr remains NULL only for GETNODEINFO (zero-length request); libcanard accepts NULL with len=0
    int16_t res;
    ATOMIC_BLOCK(NVIC_PRIO_CAN) {
        res = canardRequestOrRespond(&canard, node_id, signature, service_id,
            &dronecanAsyncSlot.transfer_id, CANARD_TRANSFER_PRIORITY_MEDIUM, CanardRequest,
            buf_ptr, len);
    }

    if (res < 0) {
        LOG_WARNING(CAN, "dronecanAsyncRequest: service %u node %u failed: %d", service_id, node_id, res);
        return false;
    }

    dronecanAsyncSlot.state = DRONECAN_ASYNC_PENDING;
    dronecanAsyncSlot.seq++;
    dronecanAsyncSlot.service_id = service_id;
    dronecanAsyncSlot.node_id = node_id;
    dronecanAsyncSlot.requested_at_ms = millis();
    return true;
}

/*
    Handle responses for any pending async service request
    (GETNODEINFO, PARAM_GETSET, EXECUTE_OPCODE, RESTART_NODE).
    A single handler serialises all on-demand service requests through
    one shared slot, avoiding the need for per-service response queues.
*/
static void handle_AsyncServiceResponse(CanardInstance *ins, CanardRxTransfer *transfer)
{
    UNUSED(ins);

    if (dronecanAsyncSlot.state != DRONECAN_ASYNC_PENDING) // timed out or already received
        return;
    if (transfer->data_type_id != dronecanAsyncSlot.service_id) // response service_id does not match the pending request
        return;
    if (transfer->source_node_id != dronecanAsyncSlot.node_id) // response received for different node_id
        return;
    // UAVCAN requires matching transfer_id to guard against stale frames (e.g. after bus-off recovery).
    // canardRequestOrRespond increments the slot's transfer_id after sending, so the in-flight id is (transfer_id-1) mod 32.
    if (transfer->transfer_id != ((dronecanAsyncSlot.transfer_id - 1) & 0x1F))
        return;

    switch (dronecanAsyncSlot.service_id) {
        case DRONECAN_SERVICE_GETNODEINFO: {
            struct uavcan_protocol_GetNodeInfoResponse resp;
            if (uavcan_protocol_GetNodeInfoResponse_decode(transfer, &resp)) {
                LOG_WARNING(CAN, "GetNodeInfoResponse decode failed");
                dronecanAsyncSlot.state = DRONECAN_ASYNC_ERROR;
                return;
            }
            dronecanGetNodeInfoResult_t *r = &dronecanAsyncSlot.result.node_info;
            uint8_t len = resp.name.len < (sizeof(r->name) - 1) ? resp.name.len : (sizeof(r->name) - 1);
            r->name_len = len;
            memcpy(r->name, resp.name.data, len);
            r->name[len] = '\0';
            r->sw_major = resp.software_version.major;
            r->sw_minor = resp.software_version.minor;
            r->sw_optional_field_flags = resp.software_version.optional_field_flags;
            r->sw_vcs_commit = (resp.software_version.optional_field_flags &
                                UAVCAN_PROTOCOL_SOFTWAREVERSION_OPTIONAL_FIELD_FLAG_VCS_COMMIT)
                               ? resp.software_version.vcs_commit : 0;
            r->hw_major = resp.hardware_version.major;
            r->hw_minor = resp.hardware_version.minor;
            memcpy(r->hw_unique_id, resp.hardware_version.unique_id, 16);
            dronecanAsyncSlot.state = DRONECAN_ASYNC_READY;
            break;
        }

        case DRONECAN_SERVICE_PARAM_GETSET: {
            struct uavcan_protocol_param_GetSetResponse resp;
            if (uavcan_protocol_param_GetSetResponse_decode(transfer, &resp)) {
                LOG_WARNING(CAN, "ParamGetSetResponse decode failed");
                dronecanAsyncSlot.state = DRONECAN_ASYNC_ERROR;
                return;
            }
            dronecanParamResult_t *r = &dronecanAsyncSlot.result.param;
            uint8_t name_len = resp.name.len < (sizeof(r->name) - 1) ? resp.name.len : (sizeof(r->name) - 1);
            r->name_len = name_len;
            memcpy(r->name, resp.name.data, name_len);
            r->name[name_len] = '\0';
            r->type = (uint8_t)resp.value.union_tag;
            switch (resp.value.union_tag) {
                case UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE:
                    r->value_int = resp.value.integer_value;
                    break;
                case UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE:
                    r->value_float = resp.value.real_value;
                    break;
                case UAVCAN_PROTOCOL_PARAM_VALUE_BOOLEAN_VALUE:
                    r->value_bool = resp.value.boolean_value;
                    break;
                case UAVCAN_PROTOCOL_PARAM_VALUE_STRING_VALUE: {
                    uint8_t slen = resp.value.string_value.len < (sizeof(r->value_str) - 1)
                                   ? resp.value.string_value.len : (sizeof(r->value_str) - 1);
                    r->value_str_len = slen;
                    memcpy(r->value_str, resp.value.string_value.data, slen);
                    r->value_str[slen] = '\0';
                    break;
                }
                default:
                    r->type = DRONECAN_PARAM_TYPE_EMPTY;
                    break;
            }
            r->min_type = DRONECAN_PARAM_TYPE_EMPTY;
            r->min_int  = 0;
            r->min_float = 0.0f;
            switch (resp.min_value.union_tag) {
                case UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_INTEGER_VALUE:
                    r->min_type = DRONECAN_PARAM_TYPE_INT;
                    r->min_int  = resp.min_value.integer_value;
                    break;
                case UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_REAL_VALUE:
                    r->min_type  = DRONECAN_PARAM_TYPE_FLOAT;
                    r->min_float = resp.min_value.real_value;
                    break;
                default:
                    break;
            }
            r->max_type  = DRONECAN_PARAM_TYPE_EMPTY;
            r->max_int   = 0;
            r->max_float = 0.0f;
            switch (resp.max_value.union_tag) {
                case UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_INTEGER_VALUE:
                    r->max_type = DRONECAN_PARAM_TYPE_INT;
                    r->max_int  = resp.max_value.integer_value;
                    break;
                case UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_REAL_VALUE:
                    r->max_type  = DRONECAN_PARAM_TYPE_FLOAT;
                    r->max_float = resp.max_value.real_value;
                    break;
                default:
                    break;
            }
            dronecanAsyncSlot.state = DRONECAN_ASYNC_READY;
            break;
        }

        case DRONECAN_SERVICE_EXECUTE_OPCODE: {
            struct uavcan_protocol_param_ExecuteOpcodeResponse resp;
            if (uavcan_protocol_param_ExecuteOpcodeResponse_decode(transfer, &resp)) {
                LOG_WARNING(CAN, "ExecuteOpcodeResponse decode failed");
                dronecanAsyncSlot.state = DRONECAN_ASYNC_ERROR;
                return;
            }
            dronecanAsyncSlot.result.simple.ok = resp.ok;
            dronecanAsyncSlot.state = DRONECAN_ASYNC_READY;
            break;
        }

        case DRONECAN_SERVICE_RESTART_NODE: {
            struct uavcan_protocol_RestartNodeResponse resp;
            if (uavcan_protocol_RestartNodeResponse_decode(transfer, &resp)) {
                LOG_WARNING(CAN, "RestartNodeResponse decode failed");
                dronecanAsyncSlot.state = DRONECAN_ASYNC_ERROR;
                return;
            }
            dronecanAsyncSlot.result.simple.ok = resp.ok;
            dronecanAsyncSlot.state = DRONECAN_ASYNC_READY;
            break;
        }

        default:
            break;
    }
}

// Canard Handlers and Senders


/*
  send the 1Hz NodeStatus message. This is what allows a node to show
  up in the DroneCAN GUI tool and in the flight controller logs
 */
static void send_NodeStatus(void) {
    uint8_t buffer[UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE];

    node_status.uptime_sec = millis() / 1000UL;
    if(isHardwareHealthy()){
        node_status.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
    }
    else {
        node_status.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_CRITICAL;
    }

    node_status.mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;  // Indicates that node is able to communicate over CAN, not that it is in flight.
    node_status.sub_mode = 0; // Not currently used in dronecan

    // put whatever you like in here for display in GUI
    node_status.vendor_specific_status_code = (uint16_t)(armingFlags & 0xFFFF);  /* field is 16-bit by UAVCAN spec; bits 16-30 of armingFlags are not transmitted */

    uint32_t len = uavcan_protocol_NodeStatus_encode(&node_status, buffer);

    // we need a static variable for the transfer ID. This is
    // incremented on each transfer, allowing for detection of packet
    // loss
    static uint8_t transfer_id;

    int16_t bc_res;
    ATOMIC_BLOCK(NVIC_PRIO_CAN) {
        bc_res = canardBroadcast(&canard,
                        UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
                        UAVCAN_PROTOCOL_NODESTATUS_ID,
                        &transfer_id,
                        CANARD_TRANSFER_PRIORITY_LOW,
                        buffer,
                        len);
    }
    if (bc_res < 0) {
        LOG_DEBUG(CAN, "NodeStatus broadcast failed: %d", bc_res);
    }

}

/*
  This function is called at 1 Hz rate from the main loop.
*/
static void process1HzTasks(timeUs_t timestamp_usec)
{
   /*
      Purge transfers that are no longer transmitted. This can free up some memory
    */
    ATOMIC_BLOCK(NVIC_PRIO_CAN) {
        canardCleanupStaleTransfers(&canard, timestamp_usec);
    }

    // Remove nodes that have stopped broadcasting NodeStatus
    for (uint8_t i = 0; i < activeNodeCount; ) {
        if (millis() - nodeTable[i].last_seen_ms > DRONECAN_NODE_STALE_TIMEOUT_MS) {
            nodeTable[i] = nodeTable[activeNodeCount - 1];
            activeNodeCount--;
        } else {
            i++;
        }
    }

    /*
      Transmit the node status message
    */
    send_NodeStatus();
}

/*
 This callback is invoked by the library when it detects beginning of a new transfer on the bus that can be received
 by the local node.
 If the callback returns true, the library will receive the transfer.
 If the callback returns false, the library will ignore the transfer.
 All transfers that are addressed to other nodes are always ignored.

 This function must fill in the out_data_type_signature to be the signature of the message.
 */
#ifdef UNIT_TEST
bool shouldAcceptTransfer(const CanardInstance *ins,
#else
static bool shouldAcceptTransfer(const CanardInstance *ins,
#endif
                                 uint64_t *out_data_type_signature,
                                 uint16_t data_type_id,
                                 CanardTransferType transfer_type,
                                 uint8_t source_node_id)
{
    UNUSED(ins);
    UNUSED(source_node_id);
    if (transfer_type == CanardTransferTypeRequest) {
        switch (data_type_id) {
        case UAVCAN_PROTOCOL_GETNODEINFO_ID:
            *out_data_type_signature = UAVCAN_PROTOCOL_GETNODEINFO_REQUEST_SIGNATURE;
            return true;
        }
    }
    if (transfer_type == CanardTransferTypeResponse) {
        switch (data_type_id) {
        case UAVCAN_PROTOCOL_GETNODEINFO_ID:
            *out_data_type_signature = UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_SIGNATURE;
            return true;
        case UAVCAN_PROTOCOL_PARAM_GETSET_ID:
            *out_data_type_signature = UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE;
            return true;
        case UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_ID:
            *out_data_type_signature = UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_SIGNATURE;
            return true;
        case UAVCAN_PROTOCOL_RESTARTNODE_ID:
            *out_data_type_signature = UAVCAN_PROTOCOL_RESTARTNODE_SIGNATURE;
            return true;
        }
    }
    if (transfer_type == CanardTransferTypeBroadcast) {
        switch (data_type_id) {
        case UAVCAN_PROTOCOL_NODESTATUS_ID:
            *out_data_type_signature = UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE;
            return true;
        case UAVCAN_EQUIPMENT_GNSS_AUXILIARY_ID:
            *out_data_type_signature = UAVCAN_EQUIPMENT_GNSS_AUXILIARY_SIGNATURE;
            return true;
        case UAVCAN_EQUIPMENT_GNSS_FIX_ID:
            *out_data_type_signature = UAVCAN_EQUIPMENT_GNSS_FIX_SIGNATURE;
            return true;
        case UAVCAN_EQUIPMENT_GNSS_FIX2_ID:
            *out_data_type_signature = UAVCAN_EQUIPMENT_GNSS_FIX2_SIGNATURE;
            return true;
        case UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_ID:
            *out_data_type_signature = UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_SIGNATURE;
            return true;
        case UAVCAN_EQUIPMENT_POWER_BATTERYINFO_ID:
            *out_data_type_signature = UAVCAN_EQUIPMENT_POWER_BATTERYINFO_SIGNATURE;
            return true;
        }
    }
    return false;
}

// Canard Handlers ( Many have code copied from libcanard esc_node example: https://github.com/dronecan/libcanard/blob/master/examples/ESCNode/esc_node.c )

#ifdef UNIT_TEST
void handle_NodeStatus(CanardInstance *ins, CanardRxTransfer *transfer) {
#else
static void handle_NodeStatus(CanardInstance *ins, CanardRxTransfer *transfer) {
#endif
    UNUSED(ins);
    struct uavcan_protocol_NodeStatus nodeStatus;

	if (uavcan_protocol_NodeStatus_decode(transfer, &nodeStatus)) {
		LOG_DEBUG(CAN, "NodeStatus decode failed");
		return;
	}

	uint8_t nodeId = transfer->source_node_id;
    dronecanNodeInfo_t *node = findNodeByID(nodeId);
    if (node) {
        node->health = nodeStatus.health;
        node->mode = nodeStatus.mode;
        node->uptime_sec = nodeStatus.uptime_sec;
        node->vendor_status_code = nodeStatus.vendor_specific_status_code;
        node->last_seen_ms = millis();
        return;
    }
    // new node
    if (activeNodeCount < DRONECAN_MAX_NODES) {
        memset(&nodeTable[activeNodeCount], 0, sizeof(dronecanNodeInfo_t));
        nodeTable[activeNodeCount].nodeID = nodeId;
        nodeTable[activeNodeCount].health = nodeStatus.health;
        nodeTable[activeNodeCount].mode = nodeStatus.mode;
        nodeTable[activeNodeCount].uptime_sec = nodeStatus.uptime_sec;
        nodeTable[activeNodeCount].vendor_status_code = nodeStatus.vendor_specific_status_code;
        nodeTable[activeNodeCount].last_seen_ms = millis();
        activeNodeCount++;

    } else {
        LOG_DEBUG(CAN, "DroneCAN: node table full (%u nodes), ignoring node %u", DRONECAN_MAX_NODES, nodeId);
    }
}

static void handle_GNSSAuxiliary(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    if (gpsConfig()->provider != GPS_DRONECAN) return;
    struct uavcan_equipment_gnss_Auxiliary gnssAuxiliary;

	if (uavcan_equipment_gnss_Auxiliary_decode(transfer, &gnssAuxiliary)) {
		LOG_DEBUG(CAN, "GNSSAuxiliary decode failed");
		return;
	}
    dronecanGPSReceiveGNSSAuxiliary(&gnssAuxiliary);
}

static void handle_GNSSFix(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    if (gpsConfig()->provider != GPS_DRONECAN) return;
    struct uavcan_equipment_gnss_Fix gnssFix;

	if (uavcan_equipment_gnss_Fix_decode(transfer, &gnssFix)) {
		LOG_DEBUG(CAN, "GNSSFix decode failed");
		return;
	}
    dronecanGPSReceiveGNSSFix(&gnssFix);
}

static void handle_GNSSFix2(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    if (gpsConfig()->provider != GPS_DRONECAN) return;
    struct uavcan_equipment_gnss_Fix2 gnssFix2;

	if (uavcan_equipment_gnss_Fix2_decode(transfer, &gnssFix2)) {
		LOG_DEBUG(CAN, "GNSSFix2 decode failed");
		return;
	}
    dronecanGPSReceiveGNSSFix2(&gnssFix2);
}

static void handle_GNSSRCTMStream(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
	UNUSED(transfer);
    /* RTCM forwarding not yet implemented. Accepted in shouldAcceptTransfer for future use. */
}

static void handle_BatteryInfo(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    struct uavcan_equipment_power_BatteryInfo batteryInfo;

	if (uavcan_equipment_power_BatteryInfo_decode(transfer, &batteryInfo)) {
		LOG_DEBUG(CAN, "BatteryInfo decode failed");
		return;
	}
    dronecanBatterySensorReceiveInfo(&batteryInfo);
}

/*
  handle a GetNodeInfo request
*/

// TODO: All the data in here is temporary for testing. If actually need to send valid data, edit accordingly.
static void handle_GetNodeInfo(CanardInstance *ins, CanardRxTransfer *transfer) {
	uint8_t buffer[UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE];
	struct uavcan_protocol_GetNodeInfoResponse pkt;

	memset(&pkt, 0, sizeof(pkt));

	node_status.uptime_sec = millis() / 1000ULL;
	pkt.status = node_status;

	// fill in your major and minor firmware version
	pkt.software_version.major = FC_VERSION_MAJOR;
	pkt.software_version.minor = FC_VERSION_MINOR;
	pkt.software_version.optional_field_flags = FC_VERSION_PATCH_LEVEL;
	pkt.software_version.vcs_commit = strtoul(shortGitRevision, NULL, 16); // need to convert string to integer put git hash in here

	// should fill in hardware version
	pkt.hardware_version.major = 1;
	pkt.hardware_version.minor = 0;

	// just setting all 16 bytes to 1 for testing
	canardSTM32GetUniqueID(pkt.hardware_version.unique_id);

	strncpy((char*)pkt.name.data, FC_FIRMWARE_NAME, sizeof(pkt.name.data));
	pkt.name.len = strnlen((char*)pkt.name.data, sizeof(pkt.name.data));

	uint16_t total_size = uavcan_protocol_GetNodeInfoResponse_encode(&pkt, buffer);

    int16_t rr_res;
    ATOMIC_BLOCK(NVIC_PRIO_CAN) {
        rr_res = canardRequestOrRespond(ins,
						   transfer->source_node_id,
						   UAVCAN_PROTOCOL_GETNODEINFO_SIGNATURE,
						   UAVCAN_PROTOCOL_GETNODEINFO_ID,
						   &transfer->transfer_id,
						   transfer->priority,
						   CanardResponse,
						   &buffer[0],
						   total_size);
    }
    if (rr_res < 0) {
        LOG_DEBUG(CAN, "GetNodeInfo response failed: %d", rr_res);
    }
}

/*
 This callback is invoked by the library when a new message or request or response is received.
*/
#ifdef UNIT_TEST
void onTransferReceived(CanardInstance *ins, CanardRxTransfer *transfer) {
#else
static void onTransferReceived(CanardInstance *ins, CanardRxTransfer *transfer) {
#endif
	// switch on data type ID to pass to the right handler function
	if (transfer->transfer_type == CanardTransferTypeRequest) {
		// check if we want to handle a specific service request
		switch (transfer->data_type_id) {
		case UAVCAN_PROTOCOL_GETNODEINFO_ID: {
			handle_GetNodeInfo(ins, transfer);
			break;
		}
		}
	}

    if (transfer->transfer_type == CanardTransferTypeResponse) {
        handle_AsyncServiceResponse(&canard, transfer);
    }

	if (transfer->transfer_type == CanardTransferTypeBroadcast) {
		// check if we want to handle a specific broadcast message
		switch (transfer->data_type_id) {

            case UAVCAN_PROTOCOL_NODESTATUS_ID:
                handle_NodeStatus(ins, transfer);
                break;


            case UAVCAN_EQUIPMENT_GNSS_AUXILIARY_ID:
                handle_GNSSAuxiliary(ins, transfer);
                break;

            case UAVCAN_EQUIPMENT_GNSS_FIX_ID:
                handle_GNSSFix(ins, transfer);
                break;

            case UAVCAN_EQUIPMENT_GNSS_FIX2_ID:
                handle_GNSSFix2(ins, transfer);
                break;

            case UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_ID:
                handle_GNSSRCTMStream(ins, transfer);
                break;

            case UAVCAN_EQUIPMENT_POWER_BATTERYINFO_ID:
                handle_BatteryInfo(ins, transfer);
                break;
        }
	}
}

#endif

#include "platform.h"

#if defined(USE_DRONECAN)

#include <dronecan_msgs.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "build/atomic.h"
#include "build/version.h"

#include "common/log.h"
#include "common/time.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/nvic.h"
#include "drivers/time.h"

#include "dronecan.h"
#include "dronecan_async.h"
#include "dronecan_dna_server.h"
#include "dronecan_node_status.h"

#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "io/gps.h"
#include "io/gps_dronecan.h"

#include "libcanard/canard.h"
#include "libcanard/canard_stm32_driver.h"

#include "sensors/battery_sensor_dronecan.h"
#include "sensors/diagnostics.h"

/* Private variables ---------------------------------------------------------*/

CanardInstance canard; /* non-static: dronecan_async.c needs extern access */
static uint8_t memory_pool[1024];

PG_REGISTER_WITH_RESET_TEMPLATE(dronecanConfig_t, dronecanConfig, PG_DRONECAN_CONFIG, 2);

PG_RESET_TEMPLATE(dronecanConfig_t, dronecanConfig,
    .nodeID = SETTING_DRONECAN_NODE_ID_DEFAULT,
    .bitRateKbps = SETTING_DRONECAN_BITRATE_KBPS_DEFAULT,
    .dronecanUseDNAServer = SETTING_DRONECAN_USE_DNA_SERVER_DEFAULT,
    .batteryId = SETTING_DRONECAN_BATTERY_ID_DEFAULT,
    .gpsNodeId = SETTING_DRONECAN_GPS_NODE_ID_DEFAULT
);

#ifdef UNIT_TEST
static uint32_t busOffCount = 0;
static volatile uint32_t txErrCount = 0;
dronecanState_e dronecanState = STATE_DRONECAN_INIT;
timeUs_t next_1hz_service_at = 0;
#else
static uint32_t busOffCount = 0;
static volatile uint32_t txErrCount = 0;
static dronecanState_e dronecanState = STATE_DRONECAN_INIT;
static timeUs_t next_1hz_service_at = 0;
#endif

/* Forward declarations ------------------------------------------------------*/

static void processCanardTxQueueSafe(void);
static void process1HzTasks(timeUs_t timestamp_usec);
#ifdef UNIT_TEST
bool shouldAcceptTransfer(const CanardInstance *ins, uint64_t *out_data_type_signature, uint16_t data_type_id, CanardTransferType transfer_type, uint8_t source_node_id);
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
	      LOG_WARNING(CAN, "Node ID is 0, this node is anonymous and can't transmit most messages. Please update this in config");
    }
}

void dronecanUpdate(timeUs_t currentTimeUs)
{
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

             for (numMessagesToProcess = canardSTM32GetRxFifoFillLevel(); numMessagesToProcess > 0; numMessagesToProcess--)
             {
	            timestamp = millis() * 1000ULL;
	            rx_res = canardSTM32Receive(&rx_frame);

	             if (rx_res < 0) {
		             LOG_WARNING(CAN, "Receive error %d", rx_res);
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

            // Check for async request timeout only after this tick's RX frames have
            // been processed, so a response already queued this tick can complete
            // the request before it's considered expired.
            dronecanAsyncCheckTimeout();

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
                    LOG_ERROR(CAN, "DroneCAN: bus-off recovery failed after 50 attempts, entering FAILED state");
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
                        LOG_WARNING(CAN, "Transmit error %d", tx_res);
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

// Canard Handlers and Senders

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

    dronecanNodeStatusUpdate(timestamp_usec);
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
        case UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_ID:
            *out_data_type_signature = UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_SIGNATURE;
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

static void handle_GNSSAuxiliary(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    if (gpsConfig()->provider != GPS_DRONECAN) return;
    struct uavcan_equipment_gnss_Auxiliary gnssAuxiliary;

	if (uavcan_equipment_gnss_Auxiliary_decode(transfer, &gnssAuxiliary)) {
		LOG_WARNING(CAN, "GNSSAuxiliary decode failed");
		return;
	}
    dronecanGPSReceiveGNSSAuxiliary(&gnssAuxiliary, transfer->source_node_id);
}

static void handle_GNSSFix(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    if (gpsConfig()->provider != GPS_DRONECAN) return;
    struct uavcan_equipment_gnss_Fix gnssFix;

	if (uavcan_equipment_gnss_Fix_decode(transfer, &gnssFix)) {
		LOG_WARNING(CAN, "GNSSFix decode failed");
		return;
	}
    dronecanGPSReceiveGNSSFix(&gnssFix);
}

static void handle_GNSSFix2(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    if (gpsConfig()->provider != GPS_DRONECAN) return;
    struct uavcan_equipment_gnss_Fix2 gnssFix2;

	if (uavcan_equipment_gnss_Fix2_decode(transfer, &gnssFix2)) {
		LOG_WARNING(CAN, "GNSSFix2 decode failed");
		return;
	}
    dronecanGPSReceiveGNSSFix2(&gnssFix2, transfer->source_node_id);
}

static void handle_GNSSRCTMStream(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    if (gpsConfig()->provider != GPS_DRONECAN) return;
    struct uavcan_equipment_gnss_RTCMStream gnssRTCMStream;

	if (uavcan_equipment_gnss_RTCMStream_decode(transfer, &gnssRTCMStream)) {
		LOG_WARNING(CAN, "RTCMStream decode failed");
		return;
	}
}

static void handle_BatteryInfo(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    struct uavcan_equipment_power_BatteryInfo batteryInfo;

	if (uavcan_equipment_power_BatteryInfo_decode(transfer, &batteryInfo)) {
		LOG_WARNING(CAN, "BatteryInfo decode failed");
		return;
	}
    if (batteryInfo.battery_id != dronecanConfig()->batteryId) {
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

	pkt.status = dronecanGetOwnNodeStatus();

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
        dronecanAsyncHandleServiceResponse(&canard, transfer);
    }

	if (transfer->transfer_type == CanardTransferTypeBroadcast) {
		// check if we want to handle a specific broadcast message
		switch (transfer->data_type_id) {

            case UAVCAN_PROTOCOL_NODESTATUS_ID:
                dronecanNodeStatusHandleBroadcast(ins, transfer);
                break;

            case UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_ID:
                if (dronecanConfig()->dronecanUseDNAServer) {
                    ATOMIC_BLOCK(NVIC_PRIO_CAN) {
                        dronecanDnaHandleAllocation(ins, transfer);
                    }
                }
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

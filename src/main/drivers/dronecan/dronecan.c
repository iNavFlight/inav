#include "platform.h"
#include "common/log.h"
#include "common/time.h"
#include "drivers/time.h"
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
static uint8_t activeNodeCount = 0;
static dronecanNodeInfo_t nodeTable[DRONECAN_MAX_NODES];

#if defined(STM32H7)
static inline void dronecanMaskTxISR(void)   { NVIC_DisableIRQ(FDCAN1_IT0_IRQn); }
static inline void dronecanUnmaskTxISR(void) { NVIC_EnableIRQ(FDCAN1_IT0_IRQn); }
#elif defined(STM32F7)
static inline void dronecanMaskTxISR(void)   { NVIC_DisableIRQ(CAN1_TX_IRQn); }
static inline void dronecanUnmaskTxISR(void) { NVIC_EnableIRQ(CAN1_TX_IRQn); }
#else
static inline void dronecanMaskTxISR(void)   {}
static inline void dronecanUnmaskTxISR(void) {}
#endif

// NOTE: All canard handlers and senders are based on this reference: https://dronecan.github.io/Specification/7._List_of_standard_data_types/
// Alternatively, you can look at the corresponding generated header file in the dsdlc_generated folder

// Canard Handlers ( Many have code copied from libcanard esc_node example: https://github.com/dronecan/libcanard/blob/master/examples/ESCNode/esc_node.c )

void handle_NodeStatus(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    struct uavcan_protocol_NodeStatus nodeStatus;

	if (uavcan_protocol_NodeStatus_decode(transfer, &nodeStatus)) {
		LOG_DEBUG(CAN, "NodeStatus decode failed");
		return;
	}

	uint8_t nodeId = transfer->source_node_id;                                                                                                                                                                                                
    for (uint8_t i = 0; i < activeNodeCount; i++) {
        if (nodeTable[i].nodeID == nodeId) { 
            // update health, mode, uptime, vendor_status_code, last_seen_ms                                                                                                                                                                                                 
            nodeTable[i].health = nodeStatus.health;
            nodeTable[i].mode = nodeStatus.mode;
            nodeTable[i].uptime_sec = nodeStatus.uptime_sec;
            nodeTable[i].vendor_status_code = nodeStatus.vendor_specific_status_code;
            nodeTable[i].last_seen_ms = millis();                    
            return;                                                                                                                                                                                                                           
        }                                                                                       
    }                                                                                                                                                                                                                                         
    // new node                                                                                 
    if (activeNodeCount < DRONECAN_MAX_NODES) {
        nodeTable[activeNodeCount].nodeID = nodeId;
        nodeTable[activeNodeCount].health = nodeStatus.health;
        nodeTable[activeNodeCount].mode = nodeStatus.mode;
        nodeTable[activeNodeCount].uptime_sec = nodeStatus.uptime_sec;
        nodeTable[activeNodeCount].vendor_status_code = nodeStatus.vendor_specific_status_code;
        nodeTable[activeNodeCount].name_len = 0;
        nodeTable[activeNodeCount].name[0] = 0;
        nodeTable[activeNodeCount].last_seen_ms = millis();     
        activeNodeCount++;                                                                                                                                                               
    }

}

void handle_GNSSAuxiliary(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    if (gpsConfig()->provider != GPS_DRONECAN) return;
    struct uavcan_equipment_gnss_Auxiliary gnssAuxiliary;

	if (uavcan_equipment_gnss_Auxiliary_decode(transfer, &gnssAuxiliary)) {
		LOG_DEBUG(CAN, "GNSSAuxiliary decode failed");
		return;
	}
    dronecanGPSReceiveGNSSAuxiliary(&gnssAuxiliary);
}

void handle_GNSSFix(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    if (gpsConfig()->provider != GPS_DRONECAN) return;
    struct uavcan_equipment_gnss_Fix gnssFix;

	if (uavcan_equipment_gnss_Fix_decode(transfer, &gnssFix)) {
		LOG_DEBUG(CAN, "GNSSFix decode failed");
		return;
	}
    dronecanGPSReceiveGNSSFix(&gnssFix);
}

void handle_GNSSFix2(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    if (gpsConfig()->provider != GPS_DRONECAN) return;
    struct uavcan_equipment_gnss_Fix2 gnssFix2;

	if (uavcan_equipment_gnss_Fix2_decode(transfer, &gnssFix2)) {
		LOG_DEBUG(CAN, "GNSSFix2 decode failed");
		return;
	}
    dronecanGPSReceiveGNSSFix2(&gnssFix2);
}

void handle_GNSSRCTMStream(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    if (gpsConfig()->provider != GPS_DRONECAN) return;
    struct uavcan_equipment_gnss_RTCMStream gnssRTCMStream;

	if (uavcan_equipment_gnss_RTCMStream_decode(transfer, &gnssRTCMStream)) {
		LOG_DEBUG(CAN, "RTCMStream decode failed");
		return;
	}
}

void handle_BatteryInfo(CanardInstance *ins, CanardRxTransfer *transfer) {
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
void handle_GetNodeInfo(CanardInstance *ins, CanardRxTransfer *transfer) {
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

    dronecanMaskTxISR();
	canardRequestOrRespond(ins,
						   transfer->source_node_id,
						   UAVCAN_PROTOCOL_GETNODEINFO_SIGNATURE,
						   UAVCAN_PROTOCOL_GETNODEINFO_ID,
						   &transfer->transfer_id,
						   transfer->priority,
						   CanardResponse,
						   &buffer[0],
						   total_size);
    dronecanUnmaskTxISR();
}

// Canard Senders

/*
  send the 1Hz NodeStatus message. This is what allows a node to show
  up in the DroneCAN GUI tool and in the flight controller logs
 */
void send_NodeStatus(void) {
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
    node_status.vendor_specific_status_code = armingFlags;

    uint32_t len = uavcan_protocol_NodeStatus_encode(&node_status, buffer);

    // we need a static variable for the transfer ID. This is
    // incremeneted on each transfer, allowing for detection of packet
    // loss
    static uint8_t transfer_id;

    dronecanMaskTxISR();
    canardBroadcast(&canard,
                    UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
                    UAVCAN_PROTOCOL_NODESTATUS_ID,
                    &transfer_id,
                    CANARD_TRANSFER_PRIORITY_LOW,
                    buffer,
                    len);
    dronecanUnmaskTxISR();

}

// Canard Util
/*
 This callback is invoked by the library when it detects beginning of a new transfer on the bus that can be received
 by the local node.
 If the callback returns true, the library will receive the transfer.
 If the callback returns false, the library will ignore the transfer.
 All transfers that are addressed to other nodes are always ignored.

 This function must fill in the out_data_type_signature to be the signature of the message.
 */

bool shouldAcceptTransfer(const CanardInstance *ins,
                                 uint64_t *out_data_type_signature,
                                 uint16_t data_type_id,
                                 CanardTransferType transfer_type,
                                 uint8_t source_node_id)
{
	UNUSED(ins);
    UNUSED(source_node_id);
    if (transfer_type == CanardTransferTypeRequest) {
	// check if we want to handle a specific service request
		switch (data_type_id) {
		case UAVCAN_PROTOCOL_GETNODEINFO_ID: {
			*out_data_type_signature = UAVCAN_PROTOCOL_GETNODEINFO_REQUEST_SIGNATURE;
			return true;
		    }
		}
	}
	if (transfer_type == CanardTransferTypeResponse) {
		// check if we want to handle a specific service request
		switch (data_type_id) {
		}
	}
	if (transfer_type == CanardTransferTypeBroadcast) {
		// see if we want to handle a specific broadcast packet
		switch (data_type_id) {

		case UAVCAN_PROTOCOL_NODESTATUS_ID: {
			*out_data_type_signature = UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE;
			return true;
		}
        case UAVCAN_EQUIPMENT_GNSS_AUXILIARY_ID: {
            *out_data_type_signature = UAVCAN_EQUIPMENT_GNSS_AUXILIARY_SIGNATURE;
            return true;
        }
        case UAVCAN_EQUIPMENT_GNSS_FIX_ID: {
            *out_data_type_signature = UAVCAN_EQUIPMENT_GNSS_FIX_SIGNATURE;
            return true;
        }
        case UAVCAN_EQUIPMENT_GNSS_FIX2_ID: {
            *out_data_type_signature = UAVCAN_EQUIPMENT_GNSS_FIX2_SIGNATURE;
            return true;
        }
        case UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_ID: {
            *out_data_type_signature = UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_SIGNATURE;
            return true;
        }
        case UAVCAN_EQUIPMENT_POWER_BATTERYINFO_ID: {
            *out_data_type_signature = UAVCAN_EQUIPMENT_POWER_BATTERYINFO_SIGNATURE;
            return true;
        }
		}
	}
	// we don't want any other messages
	return false;
}

/*
 This callback is invoked by the library when a new message or request or response is received.
*/
void onTransferReceived(CanardInstance *ins, CanardRxTransfer *transfer) {
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
		switch (transfer->data_type_id) {
		}
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

void processCanardTxQueue(void) {
	// Transmitting
	for (const CanardCANFrame *tx_frame ; (tx_frame = canardPeekTxQueue(&canard)) != NULL;)
    {
        const int16_t tx_res = canardSTM32Transmit(tx_frame);

		if (tx_res < 0) {
			LOG_DEBUG(CAN, "Transmit error %d", tx_res);
			canardPopTxQueue(&canard);  // Error - discard frame
		} else if (tx_res > 0) {
			canardPopTxQueue(&canard);  // Success - remove from queue
		} else {
			// tx_res == 0: TX FIFO full, retry later
			break;
		}
	}
}

static void processCanardTxQueueSafe(void) {
    for (;;) {
        // Mask only for the linked-list peek — not for the HAL transmit call
        dronecanMaskTxISR();
        const CanardCANFrame *tx_frame = canardPeekTxQueue(&canard);
        if (tx_frame == NULL) {
            dronecanUnmaskTxISR();
            break;
        }
        const CanardCANFrame frame_copy = *tx_frame;
        dronecanUnmaskTxISR();

        const int16_t tx_res = canardSTM32Transmit(&frame_copy);
        if (tx_res == 0) {
            break;  // HW TX full, ISR will refill when a slot opens
        }

        // Re-mask to pop. If the ISR fired during the transmit call and already
        // popped this frame, peek will return a different pointer — skip the pop.
        dronecanMaskTxISR();
        if (canardPeekTxQueue(&canard) == tx_frame) {
            if (tx_res < 0) {
                LOG_DEBUG(CAN, "Transmit error %d", tx_res);
            }
            canardPopTxQueue(&canard);
        }
        dronecanUnmaskTxISR();
    }
}

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


/*
  This function is called at 1 Hz rate from the main loop.
*/
void process1HzTasks(timeUs_t timestamp_usec)
{
   /*
      Purge transfers that are no longer transmitted. This can free up some memory
    */
    dronecanMaskTxISR();
    canardCleanupStaleTransfers(&canard, timestamp_usec);
    dronecanUnmaskTxISR();

    /*
      Transmit the node status message
    */
    send_NodeStatus();
}

void dronecanInit(void)
{
    uint32_t bitrate = 500000; // At least define 500000

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

             for (numMessagesToProcess = canardSTM32GetRxFifoFillLevel(); numMessagesToProcess > 0; numMessagesToProcess--)
             {
	            timestamp = millis() * 1000ULL;
	            rx_res = canardSTM32Receive(&rx_frame);

	             if (rx_res < 0) {
		             LOG_DEBUG(CAN, "Receive error %d", rx_res);
	             }
	             else if (rx_res > 0)        // Success - process the frame
	             {
		             canardHandleRxFrame(&canard, &rx_frame, timestamp);
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
                if (protocolStatus.BusOff != 0) {
                    dronecanState = STATE_DRONECAN_BUS_OFF;
                    busoffTimeUs = currentTimeUs;
                }
            }
            break;

        case STATE_DRONECAN_BUS_OFF:
            if(currentTimeUs > (busoffTimeUs + 20000)) { // Wait 20ms: worst-case 128x11 recovery is 11.264ms at 125kbps
                canardSTM32RecoverFromBusOff();
                busoffTimeUs = currentTimeUs;
                canardSTM32GetProtocolStatus(&protocolStatus);
                if(protocolStatus.BusOff == 0) {
                    dronecanState = STATE_DRONECAN_NORMAL;
                }
            }
            break;

        case STATE_DRONECAN_FAILED:
            break;

        case STATE_DRONECAN_COUNT:
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
            return 0;
    }
    return 0;
}

const dronecanNodeInfo_t *dronecanGetNode(uint8_t index) {
    if (index < activeNodeCount) return &nodeTable[index];
    return NULL;
}
#endif

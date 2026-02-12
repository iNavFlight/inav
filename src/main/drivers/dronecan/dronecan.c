#include "platform.h"
#include "common/log.h"
#include "common/time.h"
#include <stdint.h>
#include "fc/settings.h"
#include "build/version.h"
#if defined(USE_DRONECAN) && !defined(SITL_BUILD)

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
#include "dronecan_msgs.h"

/* Private variables ---------------------------------------------------------*/

CanardInstance canard;
uint8_t memory_pool[1024];
static struct uavcan_protocol_NodeStatus node_status;
enum dronecanState_e {
    STATE_DRONECAN_INIT,
    STATE_DRONECAN_NORMAL,
    STATE_DRONECAN_BUS_OFF
};

PG_REGISTER_WITH_RESET_TEMPLATE(dronecanConfig_t, dronecanConfig, PG_DRONECAN_CONFIG, 0);

PG_RESET_TEMPLATE(dronecanConfig_t, dronecanConfig,
    .nodeID = SETTING_DRONECAN_NODE_ID_DEFAULT,
    .bitRateKbps = SETTING_DRONECAN_BITRATE_KBPS_DEFAULT
);

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

	// LOG_DEBUG(CAN, "Node Health ");

	switch (nodeStatus.health) {
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK:
		// LOG_DEBUG(CAN, "OK");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_WARNING:
		// LOG_DEBUG(CAN, "WARNING");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_ERROR:
		// LOG_DEBUG(CAN, "ERROR");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_CRITICAL:
		// LOG_DEBUG(CAN, "CRITICAL");
		break;
	default:
		// LOG_DEBUG(CAN, "UNKNOWN?");
		break;
	}

	// LOG_DEBUG(CAN, "Node Mode ");

	switch(nodeStatus.mode) {
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL:
		// LOG_DEBUG(CAN, "OPERATIONAL");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_INITIALIZATION:
		// LOG_DEBUG(CAN, "INITIALIZATION");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_MAINTENANCE:
		// LOG_DEBUG(CAN, "MAINTENANCE");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_SOFTWARE_UPDATE:
		// LOG_DEBUG(CAN, "SOFTWARE UPDATE");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_OFFLINE:
		// LOG_DEBUG(CAN, "OFFLINE");
		break;
	default:
		// LOG_DEBUG(CAN, "UNKNOWN?");
		break;
	}
}

void handle_GNSSAuxiliary(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    struct uavcan_equipment_gnss_Auxiliary gnssAuxiliary;

	if (uavcan_equipment_gnss_Auxiliary_decode(transfer, &gnssAuxiliary)) {
		LOG_DEBUG(CAN, "GNSSAuxiliary decode failed");
		return;
	}
    LOG_DEBUG(CAN, "GNSS Auxiliary: Sats=%d HDOP=%.1f", gnssAuxiliary.sats_used, (double)gnssAuxiliary.hdop);
}

void handle_GNSSFix(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    struct uavcan_equipment_gnss_Fix gnssFix;

	if (uavcan_equipment_gnss_Fix_decode(transfer, &gnssFix)) {
		LOG_DEBUG(CAN, "GNSSFix decode failed");
		return;
	}
    dronecanGPSReceiveGNSSFix(&gnssFix);
    LOG_DEBUG(CAN, "GNSS Fix received");
}

void handle_GNSSFix2(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    struct uavcan_equipment_gnss_Fix2 gnssFix2;

	if (uavcan_equipment_gnss_Fix2_decode(transfer, &gnssFix2)) {
		LOG_DEBUG(CAN, "GNSSFix2 decode failed");
		return;
	}
    dronecanGPSReceiveGNSSFix2(&gnssFix2);
    LOG_DEBUG(CAN, "GNSS Fix2 received");
}

void handle_GNSSRCTMStream(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    struct uavcan_equipment_gnss_RTCMStream gnssRTCMStream;

	if (uavcan_equipment_gnss_RTCMStream_decode(transfer, &gnssRTCMStream)) {
		LOG_DEBUG(CAN, "RTCMStream decode failed");
		return;
	}
    LOG_DEBUG(CAN, "GNSS RTCM");
}

void handle_BatteryInfo(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    struct uavcan_equipment_power_BatteryInfo batteryInfo;

	if (uavcan_equipment_power_BatteryInfo_decode(transfer, &batteryInfo)) {
		LOG_DEBUG(CAN, "BatteryInfo decode failed");
		return;
	}
    dronecanBatterySensorReceiveInfo(&batteryInfo);
    LOG_DEBUG(CAN, "Battery Info");
}

/*
  handle a GetNodeInfo request
*/

// TODO: All the data in here is temporary for testing. If actually need to send valid data, edit accordingly.
void handle_GetNodeInfo(CanardInstance *ins, CanardRxTransfer *transfer) {
	printf("GetNodeInfo request from %d", transfer->source_node_id);

	uint8_t buffer[UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE];
	struct uavcan_protocol_GetNodeInfoResponse pkt;

	memset(&pkt, 0, sizeof(pkt));

	node_status.uptime_sec = HAL_GetTick() / 1000ULL;
	pkt.status = node_status;

	// fill in your major and minor firmware version
	pkt.software_version.major = FC_VERSION_MAJOR;
	pkt.software_version.minor = FC_VERSION_MINOR;
	pkt.software_version.optional_field_flags = FC_VERSION_PATCH_LEVEL;
	pkt.software_version.vcs_commit = 0; // shortGitRevision; // need to convert string to integer put git hash in here

	// should fill in hardware version
	pkt.hardware_version.major = 1;
	pkt.hardware_version.minor = 0;

	// just setting all 16 bytes to 1 for testing
	canardSTM32GetUniqueID(pkt.hardware_version.unique_id);

	strncpy((char*)pkt.name.data, FC_FIRMWARE_NAME, sizeof(pkt.name.data));
	pkt.name.len = strnlen((char*)pkt.name.data, sizeof(pkt.name.data));

	uint16_t total_size = uavcan_protocol_GetNodeInfoResponse_encode(&pkt, buffer);

	canardRequestOrRespond(ins,
						   transfer->source_node_id,
						   UAVCAN_PROTOCOL_GETNODEINFO_SIGNATURE,
						   UAVCAN_PROTOCOL_GETNODEINFO_ID,
						   &transfer->transfer_id,
						   transfer->priority,
						   CanardResponse,
						   &buffer[0],
						   total_size);
}

// Canard Senders

/*
  send the 1Hz NodeStatus message. This is what allows a node to show
  up in the DroneCAN GUI tool and in the flight controller logs
 */
void send_NodeStatus(void) {
    uint8_t buffer[UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE];

    // LOG_DEBUG(CAN, "Sending Node Status");
    node_status.uptime_sec = HAL_GetTick() / 1000UL;
    node_status.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
    node_status.mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
    node_status.sub_mode = 0;

    // put whatever you like in here for display in GUI
    node_status.vendor_specific_status_code = 1234;

    uint32_t len = uavcan_protocol_NodeStatus_encode(&node_status, buffer);

    // we need a static variable for the transfer ID. This is
    // incremeneted on each transfer, allowing for detection of packet
    // loss
    static uint8_t transfer_id;

    canardBroadcast(&canard,
                    UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
                    UAVCAN_PROTOCOL_NODESTATUS_ID,
                    &transfer_id,
                    CANARD_TRANSFER_PRIORITY_LOW,
                    buffer,
                    len);
    // PrintCanStatus();
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
    LOG_DEBUG(CAN, "Transfer type: %u, Transfer ID: %u ", transfer->transfer_type, transfer->data_type_id);
	//LOG_DEBUG(CAN, "0x");
    //LOG_BUFFER_ERROR(SYSTEM, transfer->payload_head, transfer->payload_len);
	//	for (int i = 0; i < transfer->payload_len; i++) {
	//		LOG_DEBUG(CAN,"%02x", transfer->payload_head[i]);
	//	}

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
                LOG_DEBUG(CAN, "Battery Info");
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
			// LOG_DEBUG(CAN, "Successfully transmitted message");
			canardPopTxQueue(&canard);  // Success - remove from queue
		} else {
			// tx_res == 0: TX FIFO full, retry later
			break;
		}
	}

}

/*
  This function is called at 1 Hz rate from the main loop.
*/
void process1HzTasks(timeUs_t timestamp_usec)
{
   /*
      Purge transfers that are no longer transmitted. This can free up some memory
    */
    canardCleanupStaleTransfers(&canard, timestamp_usec);

    /*
      Transmit the node status message
    */
    send_NodeStatus();
}

void dronecanInit(void)
{
    LOG_DEBUG(CAN, "dronecan Init");
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
    canardSTM32CAN1_Init(bitrate);  // TODO: Handle error and disable CAN if this call fails.
    /*
    Initializing the Libcanard instance.
    */
    LOG_DEBUG(CAN, "canardInit");
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
    static enum dronecanState_e dronecanState = STATE_DRONECAN_INIT;
    canardProtocolStatus_t protocolStatus = {};
    uint64_t timestamp;
    int16_t rx_res;

    switch(dronecanState) {
        case STATE_DRONECAN_INIT:
            next_1hz_service_at = currentTimeUs + 1000000ULL;  // First 1Hz tick in 1 second
            dronecanState = STATE_DRONECAN_NORMAL;
            break;

        case STATE_DRONECAN_NORMAL:
            processCanardTxQueue();

             for (numMessagesToProcess = canardSTM32GetRxFifoFillLevel(); numMessagesToProcess > 0; numMessagesToProcess--)
             {
                 //LOG_DEBUG(CAN, "Received a message");
                 //LOG_DEBUG(CAN, "Rx FIFO Fill Level: %lu", canardSTM32GetRxFifoFillLevel());
	            timestamp = HAL_GetTick() * 1000ULL;
	            rx_res = canardSTM32Recieve(&rx_frame);

	             if (rx_res < 0) {
		             LOG_DEBUG(CAN, "Receive error %d", rx_res);
	             }
	             else if (rx_res > 0)        // Success - process the frame
	             {
		             canardHandleRxFrame(&canard, &rx_frame, timestamp);
	             }
             }
            if (currentTimeUs >= next_1hz_service_at)
            {
		        next_1hz_service_at += 1000000ULL;
		        process1HzTasks(currentTimeUs);
        
            }

            canardSTM32GetProtocolStatus(&protocolStatus);
            if(protocolStatus.BusOff != 0) {
                dronecanState = STATE_DRONECAN_BUS_OFF;
                busoffTimeUs = currentTimeUs;
            }
            break;

        case STATE_DRONECAN_BUS_OFF:
            if(currentTimeUs > (busoffTimeUs + 100000)) { // Wait 100 mS
                canardSTM32RecoverFromBusOff();
                busoffTimeUs = currentTimeUs;
            }
            canardSTM32GetProtocolStatus(&protocolStatus);
            if(protocolStatus.BusOff == 0) {
                dronecanState = STATE_DRONECAN_NORMAL;
            }
            break;
    
    }
    
}
#endif

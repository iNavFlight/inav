#include "platform.h"
#include "common/log.h"
#include "common/time.h"
#include <stdint.h>
#include "fc/settings.h"
#include "build/version.h"
#if defined(USE_DRONECAN) && !defined(SITL_BUILD)

#include "io/gps.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "libcanard/canard_stm32_driver.h"
#include "libcanard/canard.h"
#include "dronecan.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "dronecan_msgs.h"
#include "nodesettings.h"

/* Private variables ---------------------------------------------------------*/

FDCAN_HandleTypeDef hfdcan1;

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
		return;
	}

//	LOG_DEBUG(CAN, "Node health: %u", nodeStatus.health);
//    LOG_DEBUG(CAN, "Node Mode: %u", nodeStatus.mode);
	LOG_DEBUG(CAN, "Node Health ");

	switch (nodeStatus.health) {
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK:
		LOG_DEBUG(CAN, "OK");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_WARNING:
		LOG_DEBUG(CAN, "WARNING");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_ERROR:
		LOG_DEBUG(CAN, "ERROR");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_HEALTH_CRITICAL:
		LOG_DEBUG(CAN, "CRITICAL");
		break;
	default:
		LOG_DEBUG(CAN, "UNKNOWN?");
		break;
	}

	LOG_DEBUG(CAN, "Node Mode ");

	switch(nodeStatus.mode) {
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL:
		LOG_DEBUG(CAN, "OPERATIONAL");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_INITIALIZATION:
		LOG_DEBUG(CAN, "INITIALIZATION");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_MAINTENANCE:
		LOG_DEBUG(CAN, "MAINTENANCE");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_SOFTWARE_UPDATE:
		LOG_DEBUG(CAN, "SOFTWARE UPDATE");
		break;
	case UAVCAN_PROTOCOL_NODESTATUS_MODE_OFFLINE:
		LOG_DEBUG(CAN, "OFFLINE");
		break;
	default:
		LOG_DEBUG(CAN, "UNKNOWN?");
		break;
	}
}

void handle_GNSSAuxiliary(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    struct uavcan_equipment_gnss_Auxiliary gnssAuxiliary;

	if (uavcan_equipment_gnss_Auxiliary_decode(transfer, &gnssAuxiliary)) {
		return;
	}
    LOG_DEBUG(CAN, "GNSS Auxiliary: Num Sats: %d, HDOP %.2f", gnssAuxiliary.sats_used, (double)gnssAuxiliary.hdop);
}

void handle_GNSSFix(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    struct uavcan_equipment_gnss_Fix gnssFix;

	if (uavcan_equipment_gnss_Fix_decode(transfer, &gnssFix)) {
		return;
	}
    dronecanGPSReceiveGNSSFix(&gnssFix);
    LOG_DEBUG(CAN, "GNSS Fix: Longitude: %lld, Latitude %lld", gnssFix.longitude_deg_1e8, gnssFix.latitude_deg_1e8);
}

void handle_GNSSFix2(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    struct uavcan_equipment_gnss_Fix2 gnssFix2;

	if (uavcan_equipment_gnss_Fix2_decode(transfer, &gnssFix2)) {
		return;
	}
    dronecanGPSReceiveGNSSFix2(&gnssFix2);
    LOG_DEBUG(CAN, "GNSS Fix2: Longitude: %lld, Latitude %lld", gnssFix2.longitude_deg_1e8, gnssFix2.latitude_deg_1e8);
}

void handle_GNSSRCTMStream(CanardInstance *ins, CanardRxTransfer *transfer) {
	UNUSED(ins);
    struct uavcan_equipment_gnss_RTCMStream gnssRTCMStream;

	if (uavcan_equipment_gnss_RTCMStream_decode(transfer, &gnssRTCMStream)) {
		return;
	}
    LOG_DEBUG(CAN, "GNSS RTCM");
}
/*
void handle_NotifyState(CanardInstance *ins, CanardRxTransfer *transfer) {
	struct ardupilot_indication_NotifyState notifyState;

	if (ardupilot_indication_NotifyState_decode(transfer, &notifyState)) {
		return;
	}

	uint32_t nl = notifyState.vehicle_state & 0xFFFFFFFF;  // ignoring the last 32 bits for printing since the highest vehicle_state value right now is 23 even though they're allowed to be up to 64bit unsigned integer

	printf("Vehicle State: %lu ", nl);

	if (notifyState.aux_data.len > 0) {
		printf("Aux Data: 0x");

		for (int i = 0; i < notifyState.aux_data.len; i++) {
			printf("%02x", notifyState.aux_data.data[i]);
		}
	}

	printf("\n");

}
*/
/*
  handle a ESC RawCommand request
*/
/*
void handle_RawCommand(CanardInstance *ins, CanardRxTransfer *transfer)
{
    struct uavcan_equipment_esc_RawCommand rawCommand;
    if (uavcan_equipment_esc_RawCommand_decode(transfer, &rawCommand)) {
        return;
    }
    // see if it is for us
    if (rawCommand.cmd.len <= ESC_INDEX) {
        return;
    }
    // convert throttle to -1.0 to 1.0 range
//    printf("Throttle: %f \n", rawCommand.cmd.data[ESC_INDEX]/8192.0);
}
*/
/*
  get a 16 byte unique ID for this node, this should be based on the CPU unique ID or other unique ID
 */
void getUniqueID(uint8_t id[16]) {
    uint32_t HALUniqueIDs[3];
    // Make Unique ID out of the 96-bit STM32 UID and fill the rest with 0s
    memset(id, 0, 16);
    HALUniqueIDs[0] = HAL_GetUIDw0();
    HALUniqueIDs[1] = HAL_GetUIDw1();
    HALUniqueIDs[2] = HAL_GetUIDw2();
    memcpy(id, HALUniqueIDs, 12);
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
	getUniqueID(pkt.hardware_version.unique_id);

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
    uint8_t buffer[UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE];

    LOG_DEBUG(CAN, "Sending Node Status");
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
	LOG_DEBUG(CAN, "0x");
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

		case UAVCAN_PROTOCOL_NODESTATUS_ID: {
			handle_NodeStatus(ins, transfer);
			break;
		}

        case UAVCAN_EQUIPMENT_GNSS_AUXILIARY_ID: {
            handle_GNSSAuxiliary(ins, transfer);
            break;
        }
        case UAVCAN_EQUIPMENT_GNSS_FIX_ID: {
            handle_GNSSFix(ins, transfer);
            break;
        }
        case UAVCAN_EQUIPMENT_GNSS_FIX2_ID: {
            handle_GNSSFix2(ins, transfer);
            break;
        }
        case UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_ID: {
            handle_GNSSRCTMStream(ins, transfer);
            break;
        }

		}
	}
}


void processCanardTxQueue(FDCAN_HandleTypeDef *hfdcan) {
	// Transmitting
	for (const CanardCANFrame *tx_frame ; (tx_frame = canardPeekTxQueue(&canard)) != NULL;)
    {
        // LOG_DEBUG(CAN, "Found transmit frame");
		FDCAN_ProtocolStatusTypeDef protocolStatus = {};

         HAL_FDCAN_GetProtocolStatus(hfdcan, &protocolStatus);
         LOG_DEBUG(CAN, "BusOff: %lu", protocolStatus.BusOff);
         LOG_DEBUG(CAN, "ErrorPassive: %lu", protocolStatus.ErrorPassive);
        const int16_t tx_res = canardSTM32Transmit(hfdcan, tx_frame);

		if (tx_res < 0) {
			LOG_DEBUG(CAN, "Transmit error %d", tx_res);
		} else if (tx_res > 0) {
			// LOG_DEBUG(CAN, "Successfully transmitted message");
		}
        else
        {
            LOG_DEBUG(CAN, "hfderror %"PRIu32"", hfdcan->ErrorCode);
        }
		// Pop canardTxQueue either way
		canardPopTxQueue(&canard);
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
    canardSTM32_FDCAN1_Init(&hfdcan1, bitrate);  // TODO: Handle error and disable CAN if this call fails.
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

    // uint64_t next_50hz_service_at = HAL_GetTick();

    // Could use DNA (Dynamic Node Allocation) by following example in esc_node.c but that requires a lot of setup and I'm not too sure of what advantage it brings
    // Instead, set a different NODE_ID for each device on the CAN bus by configuring node_settings
    if (dronecanConfig()->nodeID > 0) {
	      canardSetLocalNodeID(&canard, dronecanConfig()->nodeID);
    } else {
	      LOG_DEBUG(CAN, "Node ID is 0, this node is anonymous and can't transmit most messages. Please update this in config");
    }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
// void assert_failed(uint8_t *file, uint32_t line)
// {
//   /* USER CODE BEGIN 6 */
//   /* User can add his own implementation to report the file name and line number,
//      ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
//   /* USER CODE END 6 */
// }
#endif /* USE_FULL_ASSERT */

void dronecanUpdate(timeUs_t currentTimeUs)
{
    static timeUs_t next_1hz_service_at = 0;
    static timeUs_t busoffTimeUs = 0;
    CanardCANFrame rx_frame;
    int numMessagesToProcess = 0;
    static enum dronecanState_e dronecanState = STATE_DRONECAN_INIT;
    FDCAN_ProtocolStatusTypeDef protocolStatus = {};

    switch(dronecanState) {
        case STATE_DRONECAN_INIT:
            dronecanState = STATE_DRONECAN_NORMAL;
            break;

        case STATE_DRONECAN_NORMAL:
            processCanardTxQueue(&hfdcan1);

            numMessagesToProcess = HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0); 
            for (numMessagesToProcess = HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0); numMessagesToProcess > 0; numMessagesToProcess--)
            {
                //LOG_DEBUG(CAN, "Received a message");
                LOG_DEBUG(CAN, "Rx FIFO Fill Level: %lu", HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0));
	            const uint64_t timestamp = HAL_GetTick() * 1000ULL;
	            const int16_t rx_res = canardSTM32Recieve(&hfdcan1, FDCAN_RX_FIFO0, &rx_frame);

	            if (rx_res < 0) {
		            LOG_DEBUG(CAN, "Receive error %d", rx_res);
	            }
	            else if (rx_res > 0)        // Success - process the frame
	            {
		            canardHandleRxFrame(&canard, &rx_frame, timestamp);
	            }
                numMessagesToProcess--;
            }
            if (currentTimeUs >= next_1hz_service_at)
            {
		        next_1hz_service_at += 1000000ULL;
		        process1HzTasks(currentTimeUs);
        
            }

            HAL_FDCAN_GetProtocolStatus(&hfdcan1, &protocolStatus);
            if(protocolStatus.BusOff != 0) {
                dronecanState = STATE_DRONECAN_BUS_OFF;
                busoffTimeUs = currentTimeUs;
            }
            break;

        case STATE_DRONECAN_BUS_OFF:
            if(currentTimeUs > (busoffTimeUs + 100000)) { // Wait 100 mS
                CLEAR_BIT(hfdcan1.Instance->CCCR, FDCAN_CCCR_INIT);  // Clear INIT bit to recover from Bus-Off
                busoffTimeUs = currentTimeUs;
            }
            HAL_FDCAN_GetProtocolStatus(&hfdcan1, &protocolStatus);
            if(protocolStatus.BusOff == 0) {
                dronecanState = STATE_DRONECAN_NORMAL;
            }
            break;
    }
    
}
#endif

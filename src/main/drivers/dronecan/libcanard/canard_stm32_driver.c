/*
 * canard_stm32_driver.c
 *
 *  Created on: Jul 8, 2024
 *      Author: Roni Kant
 */

#include "common/log.h"
#include "common/time.h"
#include "canard.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_def.h"
#include "stm32h7xx_hal_fdcan.h"
// #include "stm32h7xx_hal_conf.h"
#include <string.h>
#include <stdint.h>

/**
  * @brief  Process CAN message from RxLocation FIFO into rx_frame
  * @param  hfdcan pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  RxLocation Location of the received message to be read.
  *         This parameter can be a value of @arg FDCAN_Rx_location.
  * @param  rx_frame pointer to a CanardCANFrame structure where the received CAN message will be
  * 		stored.
  * @retval ret == 1: OK, ret < 0: CANARD_ERROR, ret == 0: Check hfdcan->ErrorCode
  */
int16_t canardSTM32Recieve(FDCAN_HandleTypeDef *hfdcan, uint32_t RxLocation, CanardCANFrame *const rx_frame) {
	if (rx_frame == NULL) {
		return -CANARD_ERROR_INVALID_ARGUMENT;
	}

	FDCAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[8];

	if (HAL_FDCAN_GetRxMessage(hfdcan, RxLocation, &RxHeader, RxData) == HAL_OK) {

		//LOG_DEBUG(SYSTEM, "Received message: ID=%lu, DLC=%lu", RxHeader.Identifier, RxHeader.DataLength);
		
		//LOG_BUF_DEBUG(SYSTEM, RxData, RxHeader.DataLength);
		//	for (int i = 0; i < RxHeader.DataLength; i++) {
		//		printf("%02x", RxData[i]);
		//	}
		//	printf("\n");

		// Process ID to canard format
		rx_frame->id = RxHeader.Identifier;

		if (RxHeader.IdType == FDCAN_EXTENDED_ID) { // canard will only process the message if it is extended ID
			rx_frame->id |= CANARD_CAN_FRAME_EFF;
		}

		if (RxHeader.RxFrameType == FDCAN_REMOTE_FRAME) { // canard won't process the message if it is a remote frame
			rx_frame->id |= CANARD_CAN_FRAME_RTR;
		}

		rx_frame->data_len = RxHeader.DataLength;
		memcpy(rx_frame->data, RxData, RxHeader.DataLength);

		// assume a single interface
		rx_frame->iface_id = 0;

		return 1;
	}

	// Either no CAN msg to be read, or an error that can be read from hfdcan->ErrorCode
	return 0;
}

/**
  * @brief  Process tx_frame CAN message into Tx FIFO/Queue and transmit it
  * @param  hfdcan pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  tx_frame pointer to a CanardCANFrame structure that contains the CAN message to
  * 		transmit.
  * @retval ret == 1: OK, ret < 0: CANARD_ERROR, ret == 0: Check hfdcan->ErrorCode
  */
int16_t canardSTM32Transmit(FDCAN_HandleTypeDef *hfdcan, const CanardCANFrame* const tx_frame) {
	if (tx_frame == NULL) {
		return -CANARD_ERROR_INVALID_ARGUMENT;
	}

	if (tx_frame->id & CANARD_CAN_FRAME_ERR) {
		return -CANARD_ERROR_INVALID_ARGUMENT; // unsupported frame format
	}

	FDCAN_TxHeaderTypeDef TxHeader;
	uint8_t TxData[8];

	// Process canard id to STM FDCAN header format
	if (tx_frame->id & CANARD_CAN_FRAME_EFF) {
		TxHeader.IdType = FDCAN_EXTENDED_ID;
		TxHeader.Identifier = tx_frame->id & CANARD_CAN_EXT_ID_MASK;
	} else {
		TxHeader.IdType = FDCAN_STANDARD_ID;
		TxHeader.Identifier = tx_frame->id & CANARD_CAN_STD_ID_MASK;
	}

	TxHeader.DataLength = tx_frame->data_len;

	if (tx_frame->id & CANARD_CAN_FRAME_RTR) {
		TxHeader.TxFrameType = FDCAN_REMOTE_FRAME;
	} else {
		TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	}

	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE; // unsure about this one
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF; // Disabling FDCAN (using CAN 2.0)
	TxHeader.FDFormat = FDCAN_CLASSIC_CAN; // Disabling FDCAN (using CAN 2.0)
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS; // unsure about this one
	TxHeader.MessageMarker = 0; // unsure about this one
	memcpy(TxData, tx_frame->data, TxHeader.DataLength);

	if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &TxHeader, TxData) == HAL_OK) {
		LOG_DEBUG(SYSTEM, "Successfully sent message with id: %lu", TxHeader.Identifier);
		return 1;
	}

	LOG_DEBUG(SYSTEM, "Failed at adding message with id: %lu to Tx Queue", TxHeader.Identifier);
    
	// This might be for many reasons including the Tx Fifo being full, the error can be read from hfdcan->ErrorCode
	return 0;
}

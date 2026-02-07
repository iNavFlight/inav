/*
 * canard_stm32_driver.h
 *
 *  Created on: Jul 9, 2024
 *      Author: ronik
 */

#ifndef INC_CANARD_STM32_DRIVER_H_
#define INC_CANARD_STM32_DRIVER_H_
#ifdef USE_DRONECAN
#include "canard.h"

int16_t canardSTM32_FDCAN1_Init(FDCAN_HandleTypeDef *hfdcan1, uint32_t bitrate);

int16_t canardSTM32Recieve(FDCAN_HandleTypeDef *hfdcan, uint32_t RxLocation, CanardCANFrame *const rx_frame);
int16_t canardSTM32Transmit(FDCAN_HandleTypeDef *hfdcan, const CanardCANFrame* const tx_frame);

#endif
#endif /* INC_CANARD_STM32_DRIVER_H_ */

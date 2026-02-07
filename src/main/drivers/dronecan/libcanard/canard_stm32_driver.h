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

typedef struct {
    uint32_t BusOff;
    uint32_t ErrorPassive;
} canardProtocolStatus_t;

int16_t canardSTM32CAN1_Init(uint32_t bitrate);

int16_t canardSTM32Recieve(CanardCANFrame *const rx_frame);
int16_t canardSTM32Transmit(const CanardCANFrame* const tx_frame);
void canardSTM32GetProtocolStatus(canardProtocolStatus_t *pProtocolStat);
int32_t canardSTM32GetRxFifoFillLevel(void);
void canardSTM32RecoverFromBusOff(void);

#endif
#endif /* INC_CANARD_STM32_DRIVER_H_ */

/*
 * canard_stm32_driver.h
 *
 *  Created on: Jul 9, 2024
 *      Author: ronik
 */

#ifndef INC_CANARD_STM32_DRIVER_H_
#define INC_CANARD_STM32_DRIVER_H_
#include "canard.h"

typedef struct {
    uint32_t BusOff;
    uint32_t ErrorPassive;
    uint8_t  tec;        // Transmit Error Counter (ESR[23:16])
    uint8_t  rec;        // Receive Error Counter (ESR[31:24])
    uint8_t  lec;        // Last Error Code (ESR[6:4])
    uint16_t tx_dropped;    // Frames dropped due to SW TX queue full
    uint8_t  tx_queue_hwm;  // TX SW queue high water mark
    uint8_t  rx_buffer_hwm; // RX buffer high water mark
} canardProtocolStatus_t;

#ifdef USE_DRONECAN

int16_t canardSTM32CAN1_Init(uint32_t bitrate);

int16_t canardSTM32Receive(CanardCANFrame *const rx_frame);
int16_t canardSTM32Transmit(const CanardCANFrame* const tx_frame);
void canardSTM32GetProtocolStatus(canardProtocolStatus_t *pProtocolStat);
int32_t canardSTM32GetRxFifoFillLevel(void);
int32_t canardSTM32GetTxQueueFillLevel(void);
void canardSTM32RecoverFromBusOff(void);
void canardSTM32GetUniqueID(uint8_t id[16]);


#endif
#endif /* INC_CANARD_STM32_DRIVER_H_ */

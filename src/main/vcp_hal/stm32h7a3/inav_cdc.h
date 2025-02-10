#pragma once

#include <stdint.h>

#include "usbd_cdc_if.h"

extern uint32_t rxAvailable;

extern USBD_HandleTypeDef hUsbDeviceHS;
extern uint8_t UserRxBufferHS[APP_RX_DATA_SIZE];
extern uint8_t UserTxBufferHS[APP_TX_DATA_SIZE];

extern USBD_CDC_LineCodingTypeDef LineCoding;


extern uint32_t rxAvailable;
extern uint8_t* rxBuffPtr;


uint32_t CDC_Receive_DATA(uint8_t* recvBuf, uint32_t len);
uint32_t CDC_Send_DATA(const uint8_t *ptrBuffer, uint32_t sendLength);
uint32_t CDC_Receive_DATA(uint8_t* recvBuf, uint32_t len);
uint32_t CDC_Receive_BytesAvailable(void);
uint32_t CDC_Send_FreeBytes(void);
uint32_t CDC_BaudRate(void)


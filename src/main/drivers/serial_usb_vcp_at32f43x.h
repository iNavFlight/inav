/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

typedef struct {
    serialPort_t port;

    // Buffer used during bulk writes.
    uint8_t txBuf[20];
    uint8_t txAt;
    // Set if the port is in bulk write mode and can buffer.
    bool buffering;
} vcpPort_t;

#define APP_TX_DATA_SIZE 2048
#define APP_TX_BLOCK_SIZE 512

volatile uint8_t UserTxBuffer[APP_TX_DATA_SIZE];/* Received Data over UART (CDC interface) are stored in this buffer */
uint32_t BuffLength;
volatile uint32_t UserTxBufPtrIn = 0;/* Increment this pointer or roll it back to
                               start address when data are received over USART */
volatile uint32_t UserTxBufPtrOut = 0; /* Increment this pointer or roll it back to
                                 start address when data are sent over USB */
tmr_type * usbTxTmr= TMR20;
#define  CDC_POLLING_INTERVAL 50

/*
    APP RX is the circular buffer for data that is transmitted from the APP (host)
    to the USB device (flight controller).
*/
#define APP_RX_DATA_SIZE  2048
static uint8_t APP_Rx_Buffer[APP_RX_DATA_SIZE]; //rx buffer,convert usb read to the usbvcpRead
static uint32_t APP_Rx_ptr_out = 0; //serail read out, back pointer
static uint32_t APP_Rx_ptr_in = 0; //usb Read in, front pointer

void usbVcpInitHardware(void);
serialPort_t *usbVcpOpen(void);
struct serialPort_s;
uint32_t usbVcpGetBaudRate(struct serialPort_s *instance);
 

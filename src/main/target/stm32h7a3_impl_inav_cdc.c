
#include <stdint.h>
#include <stdbool.h>

#include "platform.h"

#include "build/atomic.h"

#include "drivers/time.h"
#include "drivers/nvic.h"

#include "target/stm32h7a3_impl_usbd_cdc_if.h"
#include "target/stm32h7a3_impl_usb_device.h"

uint8_t usbIsConfigured(void)
{
    return (hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED);
}

uint8_t usbIsConnected(void)
{
    return (hUsbDeviceHS.dev_state != USBD_STATE_DEFAULT);
}

uint32_t CDC_Send_FreeBytes(void)
{
    uint32_t freeBytes;

    uint32_t buffptr;
    uint32_t buffsize;

    ATOMIC_BLOCK(NVIC_PRIO_VCP)
    {
        if (UserTxBufPtrOutHs != UserTxBufPtrInHs) {
            if (UserTxBufPtrOutHs > UserTxBufPtrInHs) /* Roll-back */
            {
                buffsize = APP_RX_DATA_SIZE - UserTxBufPtrOutHs;
            } else {
                buffsize = UserTxBufPtrInHs - UserTxBufPtrOutHs;
            }

            buffptr = UserTxBufPtrOutHs;

            USBD_CDC_SetTxBuffer(&hUsbDeviceHS, (uint8_t *)&UserTxBufferHS[buffptr], buffsize);

            if (USBD_CDC_TransmitPacket(&hUsbDeviceHS) == USBD_OK) {
                UserTxBufPtrOutHs += buffsize;
                if (UserTxBufPtrOutHs == APP_TX_DATA_SIZE) {
                    UserTxBufPtrOutHs = 0;
                }
            }
        }
    }


    ATOMIC_BLOCK(NVIC_PRIO_VCP) {
        freeBytes = ((UserTxBufPtrOutHs - UserTxBufPtrInHs) + (-((int)(UserTxBufPtrOutHs <= UserTxBufPtrInHs)) & APP_TX_DATA_SIZE)) - 1;
    }

    return freeBytes;
}

uint32_t CDC_Send_DATA(const uint8_t *ptrBuffer, uint32_t sendLength)
{
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;

    while (hcdc->TxState != 0);

    for (uint32_t i = 0; i < sendLength; i++)
    {
        UserTxBufferHS[UserTxBufPtrInHs] = ptrBuffer[i];
        UserTxBufPtrInHs = (UserTxBufPtrInHs + 1) % APP_TX_DATA_SIZE;
        while (CDC_Send_FreeBytes() == 0) {
            delay(1);
        }
    }
    return sendLength;
}

uint32_t CDC_Receive_DATA(uint8_t *recvBuf, uint32_t len)
{
    uint32_t count = 0;
    if ((rxBuffPtrHs != NULL)) {
        while ((rxAvailableHs > 0) && count < len) {
            recvBuf[count] = rxBuffPtrHs[0];
            rxBuffPtrHs++;
            rxAvailableHs--;
            count++;
            if (rxAvailableHs < 1)
                USBD_CDC_ReceivePacket(&hUsbDeviceHS);
        }
    }
    return count;
}

uint32_t CDC_Receive_BytesAvailable(void)
{
    return rxAvailableHs;
}

uint32_t CDC_BaudRate(void)
{
    return 115200;
}
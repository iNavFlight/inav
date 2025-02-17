#include "inav_cdc.h"
#include "usbd_cdc_if.h"

#include "drivers/time.h"
#include "drivers/nvic.h"
#include "build/atomic.h"


static void (*baudRateCb)(void *context, uint32_t baud);
static void *baudRateCbContext;

static void (*ctrlLineStateCb)(void *context, uint16_t ctrlLineState);
static void *ctrlLineStateCbContext;

uint32_t rxAvailable = 0;
uint8_t* rxBuffPtr = NULL;


uint32_t CDC_Receive_DATA(uint8_t* recvBuf, uint32_t len)
{
    uint32_t count = 0;
    if ( (rxBuffPtr != NULL))
    {
        while ((rxAvailable > 0) && count < len)
        {
            recvBuf[count] = rxBuffPtr[0];
            rxBuffPtr++;
            rxAvailable--;
            count++;
            if (rxAvailable < 1)
                USBD_CDC_ReceivePacket(&hUsbDeviceHS);
        }
    }
    return count;
}

uint32_t CDC_Receive_BytesAvailable(void)
{
    return rxAvailable;
}

uint32_t CDC_Send_FreeBytes(void)
{
    uint32_t freeBytes;

    ATOMIC_BLOCK(NVIC_PRIO_VCP) {
        freeBytes = ((UserTxBufPtrOut - UserTxBufPtrIn) + (-((int)(UserTxBufPtrOut <= UserTxBufPtrIn)) & APP_TX_DATA_SIZE)) - 1;
    }

    return freeBytes;
}

/**
 * @brief  CDC_Send_DATA
 *         CDC received data to be send over USB IN endpoint are managed in
 *         this function.
 * @param  ptrBuffer: Buffer of data to be sent
 * @param  sendLength: Number of data to be sent (in bytes)
 * @retval Bytes sent
 */
uint32_t CDC_Send_DATA(const uint8_t *ptrBuffer, uint32_t sendLength)
{
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
    while (hcdc->TxState != 0);

    for (uint32_t i = 0; i < sendLength; i++)
    {
        UserTxBufferHS[UserTxBufPtrIn] = ptrBuffer[i];
        UserTxBufPtrIn = (UserTxBufPtrIn + 1) % APP_TX_DATA_SIZE;
        while (CDC_Send_FreeBytes() == 0) {
            delay(1);
        }
    }
    return sendLength;
}


/*******************************************************************************
 * Function Name  : usbIsConfigured.
 * Description    : Determines if USB VCP is configured or not
 * Input          : None.
 * Output         : None.
 * Return         : True if configured.
 *******************************************************************************/
uint8_t usbIsConfigured(void)
{
    return (hUsbDeviceHS.dev_state == USBD_STATE_CONFIGURED);
}

/*******************************************************************************
 * Function Name  : usbIsConnected.
 * Description    : Determines if USB VCP is connected ot not
 * Input          : None.
 * Output         : None.
 * Return         : True if connected.
 *******************************************************************************/
uint8_t usbIsConnected(void)
{
    return (hUsbDeviceHS.dev_state != USBD_STATE_DEFAULT);
}

/*******************************************************************************
 * Function Name  : CDC_BaudRate.
 * Description    : Get the current baud rate
 * Input          : None.
 * Output         : None.
 * Return         : Baud rate in bps
 *******************************************************************************/
uint32_t CDC_BaudRate(void)
{
    return LineCoding.bitrate;
}

/*******************************************************************************
 * Function Name  : CDC_SetBaudRateCb
 * Description    : Set a callback to call when baud rate changes
 * Input          : callback function and context.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void CDC_SetBaudRateCb(void (*cb)(void *context, uint32_t baud), void *context)
{
    baudRateCbContext = context;
    baudRateCb = cb;
}

/*******************************************************************************
 * Function Name  : CDC_SetCtrlLineStateCb
 * Description    : Set a callback to call when control line state changes
 * Input          : callback function and context.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void CDC_SetCtrlLineStateCb(void (*cb)(void *context, uint16_t ctrlLineState), void *context)
{
    ctrlLineStateCbContext = context;
    ctrlLineStateCb = cb;
}

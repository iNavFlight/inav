#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "platform.h"

#include "build_config.h"

#include "common/maths.h"
#include "usb_core.h"
#include "usbd_cdc_vcp.h"
#include "usb_regs.h"


#include "usbd_cdc_vcp.h"

#include "drivers/serial_usb_vcp.h"

#include "usb_cdc.h"

bool usbcdc_SendStatus = true;

// initialize USB subsystem
void USBCDC_Init(void)
{
	USBD_Init(&USB_OTG_dev,
             USB_OTG_FS_CORE_ID,
             &USR_desc,
             &USBD_CDC_cb,
             &USR_cb);
}

// check if we are connected to host
bool USBCDC_IsConnected(void)
{
    return usbIsConnected() && usbIsConfigured();
}

// prepare new data to be sent if IN endpoint is empty
/**
 ******************************************************************************
 * @file    usbd_cdc_vcp.c
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    22-July-2011
 * @brief   Generic media access Layer.
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#pragma     data_alignment = 4
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_vcp.h"
#include "stm32f4xx_conf.h"

LINE_CODING g_lc;

extern __IO uint8_t USB_Tx_State;
__IO uint32_t bDeviceState = UNCONNECTED; /* USB device status */

/* These are external variables imported from CDC core to be used for IN
 transfer management. */
extern uint8_t APP_Rx_Buffer[]; /* Write CDC received data in this buffer.
 These data will be sent over USB IN endpoint
 in the CDC core functions. */
extern uint32_t APP_Rx_ptr_out;
extern uint32_t APP_Rx_ptr_in; /* Increment this pointer or roll it back to
 start address when writing received data
 in the buffer APP_Rx_Buffer. */
uint8_t receiveBuffer[1024];
uint8_t receiveIndex=0;
__IO uint32_t receiveLength=0;
/* Private function prototypes -----------------------------------------------*/
static uint16_t VCP_Init(void);
static uint16_t VCP_DeInit(void);
static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataTx(uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataRx(uint8_t* Buf, uint32_t Len);

CDC_IF_Prop_TypeDef VCP_fops = {VCP_Init, VCP_DeInit, VCP_Ctrl, VCP_DataTx, VCP_DataRx };

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  VCP_Init
 *         Initializes the Media on the STM32
 * @param  None
 * @retval Result of the opeartion (USBD_OK in all cases)
 */
static uint16_t VCP_Init(void)
{
	bDeviceState = CONFIGURED;
	return USBD_OK;
}

/**
 * @brief  VCP_DeInit
 *         DeInitializes the Media on the STM32
 * @param  None
 * @retval Result of the opeartion (USBD_OK in all cases)
 */
static uint16_t VCP_DeInit(void)
{
	bDeviceState = UNCONNECTED;
	return USBD_OK;
}

void ust_cpy(LINE_CODING* plc2, const LINE_CODING* plc1)
{
   plc2->bitrate    = plc1->bitrate;
   plc2->format     = plc1->format;
   plc2->paritytype = plc1->paritytype;
   plc2->datatype   = plc1->datatype;
}

/**
 * @brief  VCP_Ctrl
 *         Manage the CDC class requests
 * @param  Cmd: Command code
 * @param  Buf: Buffer containing command data (request parameters)
 * @param  Len: Number of data to be sent (in bytes)
 * @retval Result of the opeartion (USBD_OK in all cases)
 */
static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t* Buf, uint32_t Len)
{
   LINE_CODING* plc = (LINE_CODING*)Buf;

   assert_param(Len>=sizeof(LINE_CODING));

   switch (Cmd) {
       /* Not  needed for this driver, AT modem commands */
      case SEND_ENCAPSULATED_COMMAND:
      case GET_ENCAPSULATED_RESPONSE:
         break;

      // Not needed for this driver
      case SET_COMM_FEATURE:
      case GET_COMM_FEATURE:
      case CLEAR_COMM_FEATURE:
         break;


      //Note - hw flow control on UART 1-3 and 6 only
      case SET_LINE_CODING:
         ust_cpy(&g_lc, plc);           //Copy into structure to save for later
         break;


      case GET_LINE_CODING:
         ust_cpy(plc, &g_lc);
         break;


      case SET_CONTROL_LINE_STATE:
         /* Not  needed for this driver */
         //RSW - This tells how to set RTS and DTR
         break;

      case SEND_BREAK:
         /* Not  needed for this driver */
         break;

      default:
         break;
	}

   return USBD_OK;
}

// prepare new data to be sent if IN endpoint is empty
void USBCDC_TryTx(void)
{
    //if((GetEPTxStatus(ENDP_TX) & EPTX_STAT) == EP_TX_NAK)
    { //check if endpoint is empty
        uint8_t *txData;
        int txLen = vcpGetTxData(&vcpPort, &txData);
        if(txLen > 0 || usbcdc_SendStatus) {
            usbcdc_SendStatus = false;
#if EMU_FTDI
            txLen = MIN(txLen, 64 - 2  / 2);
            uint8_t status[2] = {0x01, 0x60};
            while((txLen+APP_Rx_ptr_in+2) >= APP_RX_DATA_SIZE)
            {
            	txLen--;
            }
            memcpy(status,APP_Rx_Buffer[APP_Rx_ptr_in],2)
            APP_Rx_ptr_in+=2;
            memcpy(txData, APP_Rx_Buffer[APP_Rx_ptr_in], txLen);
#else
            txLen = MIN(txLen, 64 / 2);
            while((txLen+APP_Rx_ptr_in) >= APP_RX_DATA_SIZE)
            {
            	txLen--;
            }
            memcpy(txData, APP_Rx_Buffer[APP_Rx_ptr_in], txLen);
#endif
            APP_Rx_ptr_in+=txLen;
			if(APP_Rx_ptr_in == APP_RX_DATA_SIZE) {
				APP_Rx_ptr_in=0;
			}
            // ack transmitted data to serial driver
            vcpAckTxData(&vcpPort, txLen);
        }
    }
}

/**
 * @brief  VCP_DataRx
 *         Data received over USB OUT endpoint are sent over CDC interface
 *         through this function.
 *
 *         @note
 *         This function will block any OUT packet reception on USB endpoint
 *         until exiting this function. If you exit this function before transfer
 *         is complete on CDC interface (ie. using DMA controller) it will result
 *         in receiving more data while previous ones are still not sent.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
 */
static uint16_t VCP_DataRx(uint8_t* Buf, uint32_t Len)
{
    while(Len > 0) {
        uint8_t *rxDataPtr;
        int rxChunk = vcpGetRxDataBuffer(&vcpPort, &rxDataPtr);
        if(rxChunk <= 0) {
            // receive bufer if full or some error
            break;
        }
        rxChunk = MIN(rxChunk, Len);
        memcpy(rxDataPtr, Buf, rxChunk);
        vcpAckRxData(&vcpPort, rxChunk);
    }

   return USBD_OK;
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
    return (bDeviceState == CONFIGURED);
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
    return (bDeviceState != UNCONNECTED);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

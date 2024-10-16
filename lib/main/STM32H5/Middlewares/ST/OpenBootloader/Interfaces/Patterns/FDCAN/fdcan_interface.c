/**
  ******************************************************************************
  * @file    fdcan_interface.c
  * @author  MCD Application Team
  * @brief   Contains FDCAN HW configuration
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "interfaces_conf.h"
#include "openbl_core.h"
#include "openbl_fdcan_cmd.h"
#include "fdcan_interface.h"
#include "iwdg_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static FDCAN_HandleTypeDef hfdcan;
static FDCAN_FilterTypeDef sFilterConfig;
static FDCAN_TxHeaderTypeDef TxHeader;
static FDCAN_RxHeaderTypeDef RxHeader;
static uint8_t FdcanDetected = 0U;

/* Exported variables --------------------------------------------------------*/
uint8_t TxData[FDCAN_RAM_BUFFER_SIZE];
uint8_t RxData[FDCAN_RAM_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
static void OPENBL_FDCAN_Init(void);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  This function is used to initialize the used FDCAN instance.
  * @retval None.
  */
static void OPENBL_FDCAN_Init(void)
{
  /*                Bit time configuration:
    Bit time parameter         | Nominal      |  Data
    ---------------------------|--------------|----------------
    fdcan_ker_ck               | 20 MHz       | 20 MHz
    Time_quantum (tq)          | 50 ns        | 50 ns
    Synchronization_segment    | 1 tq         | 1 tq
    Propagation_segment        | 23 tq        | 1 tq
    Phase_segment_1            | 8 tq         | 4 tq
    Phase_segment_2            | 8 tq         | 4 tq
    Synchronization_Jump_width | 8 tq         | 4 tq
    Bit_length                 | 40 tq = 2 us | 10 tq = 0.5 us
    Bit_rate                   | 0.5 MBit/s   | 2 MBit/s
  */

  hfdcan.Instance                  = FDCANx;
  hfdcan.Init.FrameFormat          = FDCAN_FRAME_FD_BRS;
  hfdcan.Init.Mode                 = FDCAN_MODE_NORMAL;
  hfdcan.Init.AutoRetransmission   = ENABLE;
  hfdcan.Init.TransmitPause        = DISABLE;
  hfdcan.Init.ProtocolException    = ENABLE;
  hfdcan.Init.NominalPrescaler     = 0x1;
  hfdcan.Init.NominalSyncJumpWidth = 0x10;
  hfdcan.Init.NominalTimeSeg1      = 0x3F;
  hfdcan.Init.NominalTimeSeg2      = 0x10;
  hfdcan.Init.DataPrescaler        = 0x1;
  hfdcan.Init.DataSyncJumpWidth    = 0x4;
  hfdcan.Init.DataTimeSeg1         = 0xF;
  hfdcan.Init.DataTimeSeg2         = 0x4;
  hfdcan.Init.StdFiltersNbr        = 1;
  hfdcan.Init.ExtFiltersNbr        = 0;
  hfdcan.Init.TxFifoQueueMode      = FDCAN_TX_FIFO_OPERATION;

  if (HAL_FDCAN_Init(&hfdcan) != HAL_OK)
  {
    while (1);
  }

  /* Configure Rx filter */
  sFilterConfig.IdType       = FDCAN_STANDARD_ID;
  sFilterConfig.FilterIndex  = 0;
  sFilterConfig.FilterType   = FDCAN_FILTER_MASK;
  sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  sFilterConfig.FilterID1    = 0x111;
  sFilterConfig.FilterID2    = 0x7FF;
  HAL_FDCAN_ConfigFilter(&hfdcan, &sFilterConfig);

  /* Prepare Tx Header */
  TxHeader.Identifier          = 0x111;
  TxHeader.IdType              = FDCAN_STANDARD_ID;
  TxHeader.TxFrameType         = FDCAN_DATA_FRAME;
  TxHeader.DataLength          = FDCAN_DLC_BYTES_64;
  TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxHeader.BitRateSwitch       = FDCAN_BRS_ON;
  TxHeader.FDFormat            = FDCAN_FD_CAN;
  TxHeader.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
  TxHeader.MessageMarker       = 0;

  /* Start the FDCAN module */
  HAL_FDCAN_Start(&hfdcan);
}

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  This function is used to configure FDCAN pins and then initialize the used FDCAN instance.
  * @retval None.
  */
void OPENBL_FDCAN_Configuration(void)
{
  /* Enable all resources clocks --------------------------------------------*/
  /* Enable used GPIOx clocks */
  FDCANx_GPIO_CLK_ENABLE();

  OPENBL_FDCAN_Init();
}

/**
  * @brief  This function is used to De-initialize the FDCAN pins and instance.
  * @retval None.
  */
void OPENBL_FDCAN_DeInit(void)
{
  /* Only de-initialize the FDCAN if it is not the current detected interface */
  if (FdcanDetected == 0U)
  {
    FDCANx_FORCE_RESET();
    FDCANx_RELEASE_RESET();
    HAL_GPIO_DeInit(FDCANx_TX_GPIO_PORT, FDCANx_TX_PIN);
    HAL_GPIO_DeInit(FDCANx_RX_GPIO_PORT, FDCANx_RX_PIN);

    FDCANx_CLK_DISABLE();
  }
}

/**
  * @brief  This function is used to detect if there is any activity on FDCAN protocol.
  * @retval Returns 1 if interface is detected else 0..
  */
uint8_t OPENBL_FDCAN_ProtocolDetection(void)
{
  /* check if FIFO 0 receive at least one message */
  if (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan, FDCAN_RX_FIFO0) > 0)
  {
    FdcanDetected = 1;
  }
  else
  {
    FdcanDetected = 0;
  }

  return FdcanDetected;
}

/**
  * @brief  This function is used to get the command opcode from the host.
  * @retval Returns the command.
  */
uint8_t OPENBL_FDCAN_GetCommandOpcode(void)
{
  uint8_t command_opc      = 0x0;
  HAL_StatusTypeDef status = HAL_OK;

  /* check if FIFO 0 receive at least one message */
  while (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan, FDCAN_RX_FIFO0) < 1)
  {}

  /* Retrieve Rx messages from RX FIFO0 */
  status = HAL_FDCAN_GetRxMessage(&hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData);

  /* Check for errors */
  if (status == HAL_ERROR)
  {
    command_opc = ERROR_COMMAND;
  }
  else
  {
    command_opc         = RxHeader.Identifier;
    TxHeader.Identifier = RxHeader.Identifier;
  }

  return command_opc;
}

/**
  * @brief  This function is used to read one byte from FDCAN pipe.
  * @retval Returns the read byte.
  */
uint8_t OPENBL_FDCAN_ReadByte(void)
{
  uint8_t byte;

  /* check if FIFO 0 receive at least one message */
  while (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan, FDCAN_RX_FIFO0) < 1)
  {
    OPENBL_IWDG_Refresh();
  }

  /* Retrieve Rx messages from RX FIFO0 */
  HAL_FDCAN_GetRxMessage(&hfdcan, FDCAN_RX_FIFO0, &RxHeader, &byte);

  return byte;
}

/**
  * @brief  This function is used to read bytes from FDCAN pipe.
  * @retval None.
  */
void OPENBL_FDCAN_ReadBytes(uint8_t *Buffer, uint32_t BufferSize)
{
  /* check if FIFO 0 receive at least one message */
  while (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan, FDCAN_RX_FIFO0) < 1)
  {
    OPENBL_IWDG_Refresh();
  }

  /* Retrieve Rx messages from RX FIFO0 */
  HAL_FDCAN_GetRxMessage(&hfdcan, FDCAN_RX_FIFO0, &RxHeader, Buffer);
}

/**
  * @brief  This function is used to send one byte through FDCAN pipe.
  * @param  Byte The byte to be sent.
  * @retval None.
  */
void OPENBL_FDCAN_SendByte(uint8_t Byte)
{
  TxHeader.DataLength = FDCAN_DLC_BYTES_1;

  while (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan) == 0)
  {}

  HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan, &TxHeader, &Byte);

  /* Wait that the data is completely sent (sent FIFO empty) */
  while (((&hfdcan)->Instance->IR & FDCAN_IR_TFE) != FDCAN_IR_TFE)
  {}

  /* Clear the complete flag */
  (&hfdcan)->Instance->IR &= FDCAN_IR_TFE;
}

/**
  * @brief  This function is used to send a buffer using FDCAN.
  * @param  Buffer The data buffer to be sent.
  * @param  BufferSize The size of the data buffer to be sent.
  * @retval None.
  */
void OPENBL_FDCAN_SendBytes(uint8_t *Buffer, uint32_t BufferSize)
{
  TxHeader.DataLength = BufferSize;

  while (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan) == 0)
  {}

  HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan, &TxHeader, Buffer);

  /* Wait that the data is completely sent (sent FIFO empty) */
  while (((&hfdcan)->Instance->IR & FDCAN_IR_TFE) != FDCAN_IR_TFE)
  {}

  /* Clear the complete flag */
  (&hfdcan)->Instance->IR &= FDCAN_IR_TFE;
}

/**
  * @brief  This function is used to process and execute the special commands.
  *         The user must define the special commands routine here.
  * @retval Returns NACK status in case of error else returns ACK status.
  */
void OPENBL_FDCAN_SpecialCommandProcess(OPENBL_SpecialCmdTypeDef *Frame)
{
  switch (Frame->OpCode)
  {
    /* Unknown command opcode */
    default:
      if (Frame->CmdType == OPENBL_SPECIAL_CMD)
      {
        /* Send NULL data size */
        TxData[0] = 0x0;
        TxData[1] = 0x0;

        /* Send NULL status size */
        TxData[2] = 0x0;
        TxData[3] = 0x0;

        OPENBL_FDCAN_SendBytes(TxData, FDCAN_DLC_BYTES_4);
      }
      else if (Frame->CmdType == OPENBL_EXTENDED_SPECIAL_CMD)
      {
        /* Send NULL status size */
        TxData[0] = 0x0;
        TxData[1] = 0x0;

        OPENBL_FDCAN_SendBytes(TxData, FDCAN_DLC_BYTES_2);
      }
      break;
  }
}

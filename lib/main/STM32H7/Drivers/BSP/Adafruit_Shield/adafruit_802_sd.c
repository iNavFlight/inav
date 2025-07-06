/**
  ******************************************************************************
  * @file    adafruit_802_sd.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the SD card
  *          mounted on the Adafruit 1.8" TFT LCD shield (reference ID 802),
  *          that is used with the STM32 Nucleo board through SPI interface.
  *          It implements a high level communication layer for read and write
  *          from/to this memory. The needed STM32XXxx hardware resources (SPI and
  *          GPIO) are defined in adafruit_802_conf_template.h file which should
  *          be copied to application, renamed to adafruit_802_conf.h and updated
  *          depending on the used nucleo board.
  *          The initialization is performed in SD_IO_Init() function, statecly defined
  *          in this file, which call BUS_SPIx_Init() function defined within
  *          adafruit_802_bus.c file.
  *          You can easily tailor this driver to any other development board,
  *          by just adapting the defines for hardware resources adafruit_802_conf.h
  *
  *          +-------------------------------------------------------+
  *          |                     Pin assignment                    |
  *          +-------------------------+---------------+-------------+
  *          |  STM32XXxx SPI Pins     |     SD        |    Pin      |
  *          +-------------------------+---------------+-------------+
  *          | SD_SPI_CS_PIN           |   ChipSelect  |    1        |
  *          | SD_SPI_MOSI_PIN / MOSI  |   DataIn      |    2        |
  *          |                         |   GND         |    3 (0 V)  |
  *          |                         |   VDD         |    4 (3.3 V)|
  *          | SD_SPI_SCK_PIN / SCLK   |   Clock       |    5        |
  *          |                         |   GND         |    6 (0 V)  |
  *          | SD_SPI_MISO_PIN / MISO  |   DataOut     |    7        |
  *          +-------------------------+---------------+-------------+
  ******************************************************************************
  @verbatim
  How To use this driver:
  --------------------------
   - This driver does not need a specific component driver for the micro SD device
     to be included with.

  Driver description:
  ---------------------
  + Initialization steps:
     o Initialize the micro SD card using the ADAFRUIT_802_SD_Init() function.
     o Checking the SD card presence is not managed because SD detection pin is
       not physically mapped on the Adafruit shield.
     o The function ADAFRUIT_802_SD_GetCardInfo() is used to get the micro SD card information
       which is stored in the structure "SD_CardInfo".

  + Micro SD card operations
     o The micro SD card can be accessed with read/write block(s) operations once
       it is ready for access. The access can be performed in polling
       mode by calling the functions ADAFRUIT_802_SD_ReadBlocks()/ADAFRUIT_802_SD_WriteBlocks()

     o The SD erase block(s) is performed using the function ADAFRUIT_802_SD_Erase() with
       specifying the number of blocks to erase.
     o The SD runtime status is returned when calling the function ADAFRUIT_802_SD_GetCardState().

  Note:
  --------
    - Regarding the "Instance" parameter, needed for all functions, it is used to select
      an LCD instance. On the "Adafruit 1.8" TFT LCD shield", only one SD instance is availble. Then, this
      parameter should be 0.

  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "adafruit_802_sd.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup ADAFRUIT_802
  * @{
  */

/** @addtogroup ADAFRUIT_802_SD
  * @{
  */

/** @defgroup ADAFRUIT_802_SD_Private_Types_Definitions SD Private Types Definitions
  * @{
  */

/**
  * @brief  SD ansewer format
  */
typedef enum
{
  SD_ANSWER_R1_EXPECTED,
  SD_ANSWER_R1B_EXPECTED,
  SD_ANSWER_R2_EXPECTED,
  SD_ANSWER_R3_EXPECTED,
  SD_ANSWER_R4R5_EXPECTED,
  SD_ANSWER_R7_EXPECTED,
}SD_Answer_t;

/**
  * @brief  SD reponses and error flags
  */
typedef enum
{
/* R1 answer value */
  SD_R1_NO_ERROR            = 0x00U,
  SD_R1_IN_IDLE_STATE       = 0x01U,
  SD_R1_ERASE_RESET         = 0x02U,
  SD_R1_ILLEGAL_COMMAND     = 0x04U,
  SD_R1_COM_CRC_ERROR       = 0x08U,
  SD_R1_ERASE_SEQUENCE_ERROR= 0x10U,
  SD_R1_ADDRESS_ERROR       = 0x20U,
  SD_R1_PARAMETER_ERROR     = 0x40U,

/* R2 answer value */
  SD_R2_NO_ERROR            = 0x00U,
  SD_R2_CARD_LOCKED         = 0x01U,
  SD_R2_LOCKUNLOCK_ERROR    = 0x02U,
  SD_R2_ERROR               = 0x04U,
  SD_R2_CC_ERROR            = 0x08U,
  SD_R2_CARD_ECC_FAILED     = 0x10U,
  SD_R2_WP_VIOLATION        = 0x20U,
  SD_R2_ERASE_PARAM         = 0x40U,
  SD_R2_OUTOFRANGE          = 0x80U,

/**
  * @brief  Data response error
  */
  SD_DATA_OK                = 0x05U,
  SD_DATA_CRC_ERROR         = 0x0BU,
  SD_DATA_WRITE_ERROR       = 0x0DU,
  SD_DATA_OTHER_ERROR       = 0xFFU
} SD_Error_t;

/**
  * @}
  */


/** @defgroup ADAFRUIT_802_SD_Private_Defines SD Private Defines
  * @{
  */
#define SD_DUMMY_BYTE            0xFFU
#define SD_CMD_LENGTH               6U
#define SD_MAX_TRY                100U    /* Number of try */

/**
  * @brief  Start Data tokens:
  *         Tokens (necessary because at nop/idle (and CS active) only 0xff is
  *         on the data/command line)
  */
#define SD_TOKEN_START_DATA_SINGLE_BLOCK_READ    0xFEU  /* Data token start byte, Start Single Block Read */
#define SD_TOKEN_START_DATA_MULTIPLE_BLOCK_READ  0xFEU  /* Data token start byte, Start Multiple Block Read */
#define SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE   0xFEU  /* Data token start byte, Start Single Block Write */
#define SD_TOKEN_START_DATA_MULTIPLE_BLOCK_WRITE 0xFCU  /* Data token start byte, Start Multiple Block Write */
#define SD_TOKEN_STOP_DATA_MULTIPLE_BLOCK_WRITE  0xFDU  /* Data token stop byte, Stop Multiple Block Write */

/**
  * @brief  Commands: CMDxx = CMD-number | 0x40
  */
#define SD_CMD_GO_IDLE_STATE          0U   /* CMD0 = 0x40  */
#define SD_CMD_SEND_OP_COND           1U   /* CMD1 = 0x41  */
#define SD_CMD_SEND_IF_COND           8U   /* CMD8 = 0x48  */
#define SD_CMD_SEND_CSD               9U   /* CMD9 = 0x49  */
#define SD_CMD_SEND_CID               10U  /* CMD10 = 0x4A */
#define SD_CMD_STOP_TRANSMISSION      12U  /* CMD12 = 0x4C */
#define SD_CMD_SEND_STATUS            13U  /* CMD13 = 0x4D */
#define SD_CMD_SET_BLOCKLEN           16U  /* CMD16 = 0x50 */
#define SD_CMD_READ_SINGLE_BLOCK      17U  /* CMD17 = 0x51 */
#define SD_CMD_READ_MULT_BLOCK        18U  /* CMD18 = 0x52 */
#define SD_CMD_SET_BLOCK_COUNT        23U  /* CMD23 = 0x57 */
#define SD_CMD_WRITE_SINGLE_BLOCK     24U  /* CMD24 = 0x58 */
#define SD_CMD_WRITE_MULT_BLOCK       25U  /* CMD25 = 0x59 */
#define SD_CMD_PROG_CSD               27U  /* CMD27 = 0x5B */
#define SD_CMD_SET_WRITE_PROT         28U  /* CMD28 = 0x5C */
#define SD_CMD_CLR_WRITE_PROT         29U  /* CMD29 = 0x5D */
#define SD_CMD_SEND_WRITE_PROT        30U  /* CMD30 = 0x5E */
#define SD_CMD_SD_ERASE_GRP_START     32U  /* CMD32 = 0x60 */
#define SD_CMD_SD_ERASE_GRP_END       33U  /* CMD33 = 0x61 */
#define SD_CMD_UNTAG_SECTOR           34U  /* CMD34 = 0x62 */
#define SD_CMD_ERASE_GRP_START        35U  /* CMD35 = 0x63 */
#define SD_CMD_ERASE_GRP_END          36U  /* CMD36 = 0x64 */
#define SD_CMD_UNTAG_ERASE_GROUP      37U  /* CMD37 = 0x65 */
#define SD_CMD_ERASE                  38U  /* CMD38 = 0x66 */
#define SD_CMD_SD_APP_OP_COND         41U  /* CMD41 = 0x69 */
#define SD_CMD_APP_CMD                55U  /* CMD55 = 0x77 */
#define SD_CMD_READ_OCR               58U  /* CMD55 = 0x79 */
/**
  * @}
  */

/** @defgroup ADAFRUIT_802_SD_Private_Variables SD Private Variables
  * @{
  */
static uint32_t CardType = ADAFRUIT_802_CARD_SDSC;

/**
  * @}
  */

/** @defgroup ADAFRUIT_802_SD_Private_Function_Prototypes SD Private Function Prototypes
  * @{
  */
static int32_t SD_IO_Init(void);
static void SD_IO_DeInit(void);
static void SD_IO_CSState(uint32_t Value);
static int32_t SD_GetCIDRegister(SD_CardIdData_t* Cid);
static int32_t SD_GetCSDRegister(SD_CardSpecificData_t* Csd);
static int32_t SD_GetDataResponse(uint8_t *DataResponse);
static int32_t SD_GoIdleState(void);
//static SD_CmdAnswer_t SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc, uint8_t Answer);
static uint32_t SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc, uint8_t Answer);
static int32_t SD_WaitData(uint8_t data);
static int32_t SD_ReadData(uint8_t *Data);
static void SPI_IO_Delay(uint32_t Delay);
/**
  * @}
  */

/** @addtogroup ADAFRUIT_802_SD_Exported_Functions
  * @{
  */
/**
  * @brief  Initializes the SD card device.
  * @param  Instance      SD Instance
  * @retval BSP status
  */
int32_t ADAFRUIT_802_SD_Init(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  } /* Configure IO functionalities for SD pin */
  else if(SD_IO_Init() != BSP_ERROR_NONE)
  {
    ret = BSP_ERROR_NO_INIT;
  }
  else
  {
    /* SD initialized and set to SPI mode properly */
    if(SD_GoIdleState() != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_UNKNOWN_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  DeInitializes the SD card device.
  * @param  Instance      SD Instance
  * @retval SD status
  */
int32_t ADAFRUIT_802_SD_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    SD_IO_DeInit();
  }

  return ret;
}

/**
  * @brief  Configures Interrupt mode for SD1 detection pin.
  * @param Instance      SD Instance
  * @retval BSP error status
  */
int32_t ADAFRUIT_802_SD_DetectITConfig(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  return BSP_ERROR_FEATURE_NOT_SUPPORTED;
}

/**
 * @brief  Detects if SD card is correctly plugged in the memory slot or not.
  * @param Instance  SD Instance
 * @retval BSP error status
 */
int32_t ADAFRUIT_802_SD_IsDetected(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  return BSP_ERROR_FEATURE_NOT_SUPPORTED;
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be read
  * @param  BlocksNbr  Number of SD blocks to read
  * @retval BSP status
  */
int32_t ADAFRUIT_802_SD_ReadBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t response, offset = 0, blocks_nbr = BlocksNbr;
  uint8_t tmp;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and
    Check if the SD acknowledged the set block length command: R1 response (0x00: no errors) */
    response = SD_SendCmd(SD_CMD_SET_BLOCKLEN, ADAFRUIT_SD_BLOCK_SIZE, 0xFF, (uint8_t)SD_ANSWER_R1_EXPECTED);
    SD_IO_CSState(1);
    if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      if ((uint8_t)(response & 0xFFU) != (uint8_t)SD_R1_NO_ERROR)
      {
        /* Send dummy byte: 8 Clock pulses of delay */
        SD_IO_CSState(1);
        if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }

      if(ret == BSP_ERROR_NONE)
      {
        /* Data transfer */
        do
        {
          /* Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
          /* Check if the SD acknowledged the read block command: R1 response (0x00: no errors) */
          response = SD_SendCmd(SD_CMD_READ_SINGLE_BLOCK, (BlockIdx + offset) * ((CardType == ADAFRUIT_802_CARD_SDHC) ? 1U: ADAFRUIT_SD_BLOCK_SIZE), 0xFFU, (uint8_t)SD_ANSWER_R1_EXPECTED);
          if ((uint8_t)(response & 0xFFU) != (uint8_t)SD_R1_NO_ERROR)
          {
            /* Send dummy byte: 8 Clock pulses of delay */
            SD_IO_CSState(1);
            if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
            {
              ret = BSP_ERROR_PERIPH_FAILURE;
            }
          }

          if(ret == BSP_ERROR_NONE)
          {
            /* Now look for the data token to signify the start of the data */
            if (SD_WaitData(SD_TOKEN_START_DATA_SINGLE_BLOCK_READ) == BSP_ERROR_NONE)
            {
              /* Read the SD block data : read NumByteToRead data */
              if(BUS_SPIx_Recv((uint8_t*)pData + offset, ADAFRUIT_SD_BLOCK_SIZE) != BSP_ERROR_NONE)
              {
                ret = BSP_ERROR_PERIPH_FAILURE;
              }
              else
              {
                /* Set next read address*/
                offset += ADAFRUIT_SD_BLOCK_SIZE;
                blocks_nbr--;

                /* get CRC bytes (not really needed by us, but required by SD) */
                if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
                {
                  ret = BSP_ERROR_PERIPH_FAILURE;
                }
                else if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
                {
                  ret = BSP_ERROR_PERIPH_FAILURE;
                }
                else
                {
                  /* Send dummy byte: 8 Clock pulses of delay */
                  SD_IO_CSState(1);
                  if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
                  {
                    ret = BSP_ERROR_PERIPH_FAILURE;
                  }
                }
                if(ret == BSP_ERROR_NONE)
                {
                  /* End the command data read cycle */
                  SD_IO_CSState(1);
                  if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
                  {
                    ret = BSP_ERROR_PERIPH_FAILURE;
                  }
                }
              }
            }
          }
        }while ((blocks_nbr != 0U) && (ret == BSP_ERROR_NONE));
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in polling mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be written
  * @param  BlocksNbr  Number of SD blocks to write
  * @retval BSP status
  */
int32_t ADAFRUIT_802_SD_WriteBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t response, offset = 0, blocks_nbr = BlocksNbr;
  uint8_t tmp, data_response;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and
    Check if the SD acknowledged the set block length command: R1 response (0x00: no errors) */
    response = SD_SendCmd(SD_CMD_SET_BLOCKLEN, ADAFRUIT_SD_BLOCK_SIZE, 0xFF, (uint8_t)SD_ANSWER_R1_EXPECTED);
    SD_IO_CSState(1);
    tmp = SD_DUMMY_BYTE;
    if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      if ((uint8_t)(response & 0xFFU) != (uint8_t)SD_R1_NO_ERROR)
      {
        /* Send dummy byte: 8 Clock pulses of delay */
        SD_IO_CSState(1);
        if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }

      if(ret == BSP_ERROR_NONE)
      {
        /* Data transfer */
        do
        {
          /* Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write blocks  and
          Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
          response = SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, (BlockIdx + offset) * ((CardType == ADAFRUIT_802_CARD_SDHC) ? 1U : ADAFRUIT_SD_BLOCK_SIZE), 0xFFU, (uint8_t)SD_ANSWER_R1_EXPECTED);
          if ((uint8_t)(response & 0xFFU) != (uint8_t)SD_R1_NO_ERROR)
          {
            /* Send dummy byte: 8 Clock pulses of delay */
            SD_IO_CSState(1);
            if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
            {
              ret = BSP_ERROR_PERIPH_FAILURE;
            }
          }

          if(ret == BSP_ERROR_NONE)
          {
            /* Send dummy byte for NWR timing : one byte between CMDWRITE and TOKEN */
            if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
            {
              ret = BSP_ERROR_PERIPH_FAILURE;
            }
            else if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
            {
              ret = BSP_ERROR_PERIPH_FAILURE;
            }
            else
            {
              /* Send the data token to signify the start of the data */
              tmp = SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE;
              if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
              {
                ret = BSP_ERROR_PERIPH_FAILURE;
              }/* Write the block data to SD */
              else if(BUS_SPIx_Send((uint8_t*)pData + offset, ADAFRUIT_SD_BLOCK_SIZE) != BSP_ERROR_NONE)
              {
                ret = BSP_ERROR_PERIPH_FAILURE;
              }
              else
              {
                /* Set next write address */
                offset += ADAFRUIT_SD_BLOCK_SIZE;
                blocks_nbr--;

                /* get CRC bytes (not really needed by us, but required by SD) */
                tmp = SD_DUMMY_BYTE;
                if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
                {
                  ret = BSP_ERROR_PERIPH_FAILURE;
                }
                else if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
                {
                  ret = BSP_ERROR_PERIPH_FAILURE;
                }/* Read data response */
                else if(SD_GetDataResponse(&data_response) != BSP_ERROR_NONE)
                {
                  ret = BSP_ERROR_UNKNOWN_FAILURE;
                }
                else
                {
                  if (data_response != (uint8_t)SD_DATA_OK)
                  {
                    /* Set response value to failure */
                    /* Send dummy byte: 8 Clock pulses of delay */
                    SD_IO_CSState(1);
                    if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
                    {
                      ret = BSP_ERROR_PERIPH_FAILURE;
                    }
                  }
                }
              }
            }
            if(ret == BSP_ERROR_NONE)
            {
              SD_IO_CSState(1);
              if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
              {
                ret = BSP_ERROR_PERIPH_FAILURE;
              }
            }
          }
        }while ((blocks_nbr != 0U) && (ret == BSP_ERROR_NONE));
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in DMA mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be read
  * @param  BlocksNbr  Number of SD blocks to read
  * @retval BSP status
  */
int32_t ADAFRUIT_802_SD_ReadBlocks_DMA(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret;
  /* Prevent unused argument(s) compilation warning */
  UNUSED(pData);
  UNUSED(BlockIdx);
  UNUSED(BlocksNbr);

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be written
  * @param  BlocksNbr  Number of SD blocks to write
  * @retval BSP status
  */
int32_t ADAFRUIT_802_SD_WriteBlocks_DMA(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret;
  /* Prevent unused argument(s) compilation warning */
  UNUSED(pData);
  UNUSED(BlockIdx);
  UNUSED(BlocksNbr);

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in DMA mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be read
  * @param  BlocksNbr  Number of SD blocks to read
  * @retval SD status
  */
int32_t ADAFRUIT_802_SD_ReadBlocks_IT(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret;
  /* Prevent unused argument(s) compilation warning */
  UNUSED(pData);
  UNUSED(BlockIdx);
  UNUSED(BlocksNbr);

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to the buffer that will contain the data to transmit
  * @param  BlockIdx   Block index from where data is to be written
  * @param  BlocksNbr  Number of SD blocks to write
  * @retval SD status
  */
int32_t ADAFRUIT_802_SD_WriteBlocks_IT(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret;
  /* Prevent unused argument(s) compilation warning */
  UNUSED(pData);
  UNUSED(BlockIdx);
  UNUSED(BlocksNbr);

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Erases the specified memory area of the given SD card.
  * @param  Instance   SD Instance
  * @param  BlockIdx   Block index from where data is to be
  * @param  BlocksNbr  Number of SD blocks to erase
  * @retval SD status
  */
int32_t ADAFRUIT_802_SD_Erase(uint32_t Instance, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  int32_t ret = BSP_ERROR_UNKNOWN_FAILURE;
  uint32_t response;
  uint8_t tmp;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Send CMD32 (Erase group start) and check if the SD acknowledged the erase command: R1 response (0x00: no errors) */
    response = SD_SendCmd(SD_CMD_SD_ERASE_GRP_START, BlockIdx * ((CardType == ADAFRUIT_802_CARD_SDHC) ? 1U : ADAFRUIT_SD_BLOCK_SIZE), 0xFFU, (uint8_t)SD_ANSWER_R1_EXPECTED);
    SD_IO_CSState(1);
    tmp = SD_DUMMY_BYTE;
    if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      if ((uint8_t)(response & 0xFFU) == (uint8_t)SD_R1_NO_ERROR)
      {
        /* Send CMD33 (Erase group end) and Check if the SD acknowledged the erase command: R1 response (0x00: no errors) */
        response = SD_SendCmd(SD_CMD_SD_ERASE_GRP_END, ((BlockIdx + BlocksNbr)*ADAFRUIT_SD_BLOCK_SIZE) * ((CardType == ADAFRUIT_802_CARD_SDHC) ? 1U : ADAFRUIT_SD_BLOCK_SIZE), 0xFFU, (uint8_t)SD_ANSWER_R1_EXPECTED);
        SD_IO_CSState(1);
        if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else
        {
          if((uint8_t)(response & 0xFFU) == (uint8_t)SD_R1_NO_ERROR)
          {
            /* Send CMD38 (Erase) and Check if the SD acknowledged the erase command: R1 response (0x00: no errors) */
            response = SD_SendCmd(SD_CMD_ERASE, 0U, 0xFFU, (uint8_t)SD_ANSWER_R1B_EXPECTED);
            if ((uint8_t)(response & 0xFFU) == (uint8_t)SD_R1_NO_ERROR)
            {
              if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
              {
                ret = BSP_ERROR_PERIPH_FAILURE;
              }
              else
              {
                ret = BSP_ERROR_NONE;
              }
            }
          }
        }
      }
    }
  }

  /* Return Status */
  return ret;
}

/**
  * @brief  Gets the current SD card data status.
  * @param  Instance  SD Instance
  * @retval Data transfer state.
  *          This value can be one of the following values:
  *            @arg  SD_TRANSFER_OK: No data transfer is acting
  *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
  */
int32_t ADAFRUIT_802_SD_GetCardState(uint32_t Instance)
{
  int32_t ret;
  uint32_t response;
  uint8_t tmp;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Send CMD13 (SD_SEND_STATUS) to get SD status */
    response = SD_SendCmd(SD_CMD_SEND_STATUS, 0, 0xFF, (uint8_t)SD_ANSWER_R2_EXPECTED);
    SD_IO_CSState(1);
    tmp = SD_DUMMY_BYTE;
    if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }/* Find SD status according to card state */
    else if(((uint8_t)(response & 0xFFU) == (uint8_t)SD_R1_NO_ERROR) && ((uint8_t)((response & 0xFF00UL) >> 8) == (uint8_t)SD_R2_NO_ERROR))
    {
      ret = (int32_t)SD_TRANSFER_OK;
    }
    else
    {
      ret = (int32_t)SD_TRANSFER_BUSY;
    }
  }

  /* Return Status */
  return ret;
}

/**
  * @brief  Returns information about specific card.
  * @param  CardInfo: Pointer to a SD_CardInfo structure that contains all SD
  *         card information.
  * @retval The SD Response:
  *         - MSD_ERROR: Sequence failed
  *         - MSD_OK: Sequence succeed
  */
int32_t ADAFRUIT_802_SD_GetCardInfo(uint32_t Instance, ADAFRUIT_802_SD_CardInfo_t *CardInfo)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= SD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(SD_GetCSDRegister(&(CardInfo->Csd)) != BSP_ERROR_NONE)
  {
    ret = BSP_ERROR_UNKNOWN_FAILURE;
  }
  else if(SD_GetCIDRegister(&(CardInfo->Cid)) != BSP_ERROR_NONE)
  {
    ret = BSP_ERROR_UNKNOWN_FAILURE;
  }
  else if(CardType == ADAFRUIT_802_CARD_SDHC)
  {
    CardInfo->LogBlockSize = 512U;
    CardInfo->CardBlockSize = 512U;
    CardInfo->CardCapacity = (CardInfo->Csd.version.v2.DeviceSize + 1U) * 1024U * CardInfo->LogBlockSize;
    CardInfo->LogBlockNbr = (CardInfo->CardCapacity) / (CardInfo->LogBlockSize);
  }
  else
  {
    CardInfo->CardCapacity = ((uint32_t)CardInfo->Csd.version.v1.DeviceSize + 1U) ;
    CardInfo->CardCapacity *= (1UL << ((CardInfo->Csd.version.v1.DeviceSizeMul + 2U) & 0x1FU));
    CardInfo->LogBlockSize = 512U;
    CardInfo->CardBlockSize = (1UL << (uint32_t)(CardInfo->Csd.RdBlockLen & 0x1FU));
    CardInfo->CardCapacity *= CardInfo->CardBlockSize;
    CardInfo->LogBlockNbr = (CardInfo->CardCapacity) / (CardInfo->LogBlockSize);
  }

  return ret;
}

/**
  * @}
  */

/** @defgroup ADAFRUIT_802_SD_Private_Functions SD Private Functions
  * @{
  */
/**
  * @brief  Initializes the SD Card and put it into StandBy State (Ready for
  *         data transfer).
  * @retval BSP status
  */
static int32_t SD_IO_Init(void)
{
  int32_t ret = BSP_ERROR_NONE;
  uint8_t counter = 0, tmp;
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* SD_CS_GPIO Periph clock enable */
  ADAFRUIT_802_SD_CS_GPIO_CLK_ENABLE();

  /* Configure SD_CS_PIN pin: SD Card CS pin */
  GPIO_InitStruct.Pin   = ADAFRUIT_802_SD_CS_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(ADAFRUIT_802_SD_CS_GPIO_PORT, &GPIO_InitStruct);

  /*  LCD chip select line perturbs SD also when the LCD is not used */
  /*  this is a workaround to avoid sporadic failures during r/w operations */
  ADAFRUIT_802_LCD_CS_GPIO_CLK_ENABLE();
  GPIO_InitStruct.Pin   = ADAFRUIT_802_LCD_CS_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(ADAFRUIT_802_LCD_CS_GPIO_PORT, &GPIO_InitStruct);
  ADAFRUIT_802_LCD_CS_HIGH();


  /*------------Put SD in SPI mode--------------*/
  /* SD SPI Config */
  if(BUS_SPIx_Init() != BSP_ERROR_NONE)
  {
    ret = BSP_ERROR_NO_INIT;
  }
  else
  {
    /* SD chip select high */
    ADAFRUIT_802_SD_CS_HIGH();

    /* Send dummy byte 0xFF, 10 times with CS high */
    /* Rise CS and MOSI for 80 clocks cycles */
    tmp = SD_DUMMY_BYTE;

    do
    {
      /* Send dummy byte 0xFF */
      if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
        break;
      }
      counter++;
    }while(counter <= 9U);
  }

  return ret;
}

/**
  * @brief  Initializes the SD Card and put it into StandBy State (Ready for
  *         data transfer).
  * @retval None
  */
static void SD_IO_DeInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* SD chip select low */
  ADAFRUIT_802_SD_CS_LOW();

  /* Configure SD_CS_PIN pin: SD Card CS pin */
  GPIO_InitStruct.Pin = ADAFRUIT_802_SD_CS_PIN;
  HAL_GPIO_DeInit(ADAFRUIT_802_SD_CS_GPIO_PORT, GPIO_InitStruct.Pin);

  /* SD_CS_GPIO Periph clock enable */
  ADAFRUIT_802_SD_CS_GPIO_CLK_DISABLE();
}

/**
  * @brief  Set the SD_CS pin.
  * @param  Value pin's value.
  * @retval None
  */
static void SD_IO_CSState(uint32_t Value)
{
  if(Value == 1U)
  {
    ADAFRUIT_802_SD_CS_HIGH();
  }
  else
  {
    ADAFRUIT_802_SD_CS_LOW();
  }
}

/**
  * @brief  Reads the SD card SCD register.
  *         Reading the contents of the CSD register in SPI mode is a simple
  *         read-block transaction.
  * @param  Csd pointer on an SCD register structure
  * @retval SD status
  */
static int32_t SD_GetCSDRegister(SD_CardSpecificData_t* Csd)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t counter;
  uint32_t CSD_Tab[16];
  uint32_t response;
  uint8_t tmp;

  /* Send CMD9 (CSD register) or CMD10(CSD register) and Wait for response in the R1 format (0x00 is no errors) */
  response = SD_SendCmd(SD_CMD_SEND_CSD, 0U, 0xFFU, (uint8_t)SD_ANSWER_R1_EXPECTED);
  if((uint8_t)(response & 0xFFU) == (uint8_t)SD_R1_NO_ERROR)
  {
    if(SD_WaitData(SD_TOKEN_START_DATA_SINGLE_BLOCK_READ) == BSP_ERROR_NONE)
    {
      tmp = SD_DUMMY_BYTE;
      for (counter = 0U; counter < 16U; counter++)
      {
        /* Store CSD register value on CSD_Tab */
        if(BUS_SPIx_SendRecv(&tmp, (uint8_t*)&CSD_Tab[counter], 1U) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
          break;
        }
      }

      if(ret == BSP_ERROR_NONE)
      {
        /* Get CRC bytes (not really needed by us, but required by SD) */
        if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else
        {
          /*************************************************************************
          CSD header decoding
          *************************************************************************/

          /* Byte 0 */
          Csd->CSDStruct = (CSD_Tab[0] & 0xC0U) >> 6U;
          Csd->Reserved1 =  CSD_Tab[0] & 0x3FU;

          /* Byte 1 */
          Csd->TAAC = CSD_Tab[1];

          /* Byte 2 */
          Csd->NSAC = CSD_Tab[2];

          /* Byte 3 */
          Csd->MaxBusClkFrec = CSD_Tab[3];

          /* Byte 4/5 */
          Csd->CardComdClasses = (uint16_t)(((uint16_t)CSD_Tab[4] << 4U) | ((uint16_t)(CSD_Tab[5] & 0xF0U) >> 4U));
          Csd->RdBlockLen = CSD_Tab[5] & 0x0FU;

          /* Byte 6 */
          Csd->PartBlockRead   = (CSD_Tab[6] & 0x80U) >> 7U;
          Csd->WrBlockMisalign = (CSD_Tab[6] & 0x40U) >> 6U;
          Csd->RdBlockMisalign = (CSD_Tab[6] & 0x20U) >> 5U;
          Csd->DSRImpl         = (CSD_Tab[6] & 0x10U) >> 4U;

          /*************************************************************************
          CSD v1/v2 decoding
          *************************************************************************/

          if(CardType == ADAFRUIT_802_CARD_SDSC)
          {
            Csd->version.v1.Reserved1 = ((CSD_Tab[6] & 0x0CU) >> 2U);

            Csd->version.v1.DeviceSize =  ((CSD_Tab[6] & 0x03U) << 10U) | (CSD_Tab[7] << 2U) | ((CSD_Tab[8] & 0xC0U) >> 6U);
            Csd->version.v1.MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38U) >> 3U;
            Csd->version.v1.MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07U);
            Csd->version.v1.MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0U) >> 5U;
            Csd->version.v1.MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1CU) >> 2U;
            Csd->version.v1.DeviceSizeMul = ((CSD_Tab[9] & 0x03U) << 1U) | ((CSD_Tab[10] & 0x80U) >> 7U);
          }
          else
          {
            Csd->version.v2.Reserved1 = ((CSD_Tab[6] & 0x0FU) << 2U) | ((CSD_Tab[7] & 0xC0U) >> 6U);
            Csd->version.v2.DeviceSize= ((CSD_Tab[7] & 0x3FU) << 16U) | (CSD_Tab[8] << 8U) | CSD_Tab[9];
            Csd->version.v2.Reserved2 = ((CSD_Tab[10] & 0x80U) >> 8U);
          }

          Csd->EraseSingleBlockEnable = (CSD_Tab[10] & 0x40U) >> 6U;
          Csd->EraseSectorSize   = ((CSD_Tab[10] & 0x3FU) << 1U) | ((CSD_Tab[11] & 0x80U) >> 7U);
          Csd->WrProtectGrSize   = (CSD_Tab[11] & 0x7FU);
          Csd->WrProtectGrEnable = (CSD_Tab[12] & 0x80U) >> 7U;
          Csd->Reserved2         = (CSD_Tab[12] & 0x60U) >> 5U;
          Csd->WrSpeedFact       = (CSD_Tab[12] & 0x1CU) >> 2U;
          Csd->MaxWrBlockLen     = ((CSD_Tab[12] & 0x03U) << 2U) | ((CSD_Tab[13] & 0xC0U) >> 6U);
          Csd->WriteBlockPartial = (CSD_Tab[13] & 0x20U) >> 5U;
          Csd->Reserved3         = (CSD_Tab[13] & 0x1FU);
          Csd->FileFormatGrouop  = (CSD_Tab[14] & 0x80U) >> 7U;
          Csd->CopyFlag          = (CSD_Tab[14] & 0x40U) >> 6U;
          Csd->PermWrProtect     = (CSD_Tab[14] & 0x20U) >> 5U;
          Csd->TempWrProtect     = (CSD_Tab[14] & 0x10U) >> 4U;
          Csd->FileFormat        = (CSD_Tab[14] & 0x0CU) >> 2U;
          Csd->Reserved4         = (CSD_Tab[14] & 0x03U);
          Csd->crc               = (CSD_Tab[15] & 0xFEU) >> 1U;
          Csd->Reserved5         = (CSD_Tab[15] & 0x01U);

          ret = BSP_ERROR_NONE;
        }
      }
    }
  }
  else
  {
    ret = BSP_ERROR_UNKNOWN_FAILURE;
  }

  if(ret == BSP_ERROR_NONE)
  {
    /* Send dummy byte: 8 Clock pulses of delay */
    SD_IO_CSState(1);

    if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  /* Return the reponse */
  return ret;
}

/**
  * @brief  Reads the SD card CID register.
  *         Reading the contents of the CID register in SPI mode is a simple
  *         read-block transaction.
  * @param  Cid: pointer on an CID register structure
  * @retval SD status
  */
static int32_t SD_GetCIDRegister(SD_CardIdData_t* Cid)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t CID_Tab[16];
  uint32_t response;
  uint32_t counter;
  uint8_t tmp;

  /* Send CMD10 (CID register) and Wait for response in the R1 format (0x00 is no errors) */
  response = SD_SendCmd(SD_CMD_SEND_CID, 0U, 0xFFU, (uint8_t)SD_ANSWER_R1_EXPECTED);
  if((uint8_t)(response & 0xFFU) == (uint8_t)SD_R1_NO_ERROR)
  {
    if(SD_WaitData(SD_TOKEN_START_DATA_SINGLE_BLOCK_READ) == BSP_ERROR_NONE)
    {
      tmp = SD_DUMMY_BYTE;

      /* Store CID register value on CID_Tab */
      for (counter = 0U; counter < 16U; counter++)
      {
        if(BUS_SPIx_SendRecv(&tmp, (uint8_t*)&CID_Tab[counter], 1U) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
          break;
        }
      }

      if(ret == BSP_ERROR_NONE)
      {
        /* Get CRC bytes (not really needed by us, but required by SD) */
        if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else
        {
          /* Byte 0 */
          Cid->ManufacturerID = CID_Tab[0];

          /* Byte 1 */
          Cid->OEM_AppliID = CID_Tab[1] << 8U;

          /* Byte 2 */
          Cid->OEM_AppliID |= CID_Tab[2];

          /* Byte 3 */
          Cid->ProdName1 = CID_Tab[3] << 24U;

          /* Byte 4 */
          Cid->ProdName1 |= CID_Tab[4] << 16U;

          /* Byte 5 */
          Cid->ProdName1 |= CID_Tab[5] << 8U;

          /* Byte 6 */
          Cid->ProdName1 |= CID_Tab[6];

          /* Byte 7 */
          Cid->ProdName2 = CID_Tab[7];

          /* Byte 8 */
          Cid->ProdRev = CID_Tab[8];

          /* Byte 9 */
          Cid->ProdSN = CID_Tab[9] << 24U;

          /* Byte 10 */
          Cid->ProdSN |= CID_Tab[10] << 16U;

          /* Byte 11 */
          Cid->ProdSN |= CID_Tab[11] << 8U;

          /* Byte 12 */
          Cid->ProdSN |= CID_Tab[12];

          /* Byte 13 */
          Cid->Reserved1 |= (CID_Tab[13] & 0xF0U) >> 4U;
          Cid->ManufactDate = (CID_Tab[13] & 0x0FU) << 8U;

          /* Byte 14 */
          Cid->ManufactDate |= CID_Tab[14];

          /* Byte 15 */
          Cid->CID_CRC = (CID_Tab[15] & 0xFEU) >> 1U;
          Cid->Reserved2 = 1U;

          ret = BSP_ERROR_NONE;
        }
      }
    }
  }
  else
  {
    ret = BSP_ERROR_UNKNOWN_FAILURE;
  }

  if(ret == BSP_ERROR_NONE)
  {
    /* Send dummy byte: 8 Clock pulses of delay */
    SD_IO_CSState(1);

    if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  /* Return the reponse */
  return ret;
}

/**
  * @brief  Sends 5 bytes command to the SD card and get response
  * @param  Cmd The user expected command to send to SD card.
  * @param  Arg The command argument.
  * @param  Crc The CRC.
  * @param  Answer SD_ANSWER_NOT_EXPECTED or SD_ANSWER_EXPECTED
  * @retval SD response or 0xFFFF in case of error
  */
static uint32_t SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc, uint8_t Answer)
{
  uint8_t frame[SD_CMD_LENGTH], frameout[SD_CMD_LENGTH];
  uint32_t response = 0xFFFF;
  uint8_t tmp, response_tmp;

  /* R1 Lenght = NCS(0)+ 6 Bytes command + NCR(min1 max8) + 1 Bytes answer + NEC(0) = 15bytes */
  /* R1b identical to R1 + Busy information                                                   */
  /* R2 Lenght = NCS(0)+ 6 Bytes command + NCR(min1 max8) + 2 Bytes answer + NEC(0) = 16bytes */

  /* Prepare Frame to send */
  frame[0] = (Cmd | 0x40U);         /* Construct byte 1 */
  frame[1] = (uint8_t)(Arg >> 24U); /* Construct byte 2 */
  frame[2] = (uint8_t)(Arg >> 16U); /* Construct byte 3 */
  frame[3] = (uint8_t)(Arg >> 8U);  /* Construct byte 4 */
  frame[4] = (uint8_t)(Arg);        /* Construct byte 5 */
  frame[5] = (Crc | 0x01U);         /* Construct byte 6 */

  /* Send the command */
  SD_IO_CSState(0);
  /* Send the Cmd bytes */
  if(BUS_SPIx_SendRecv(frame, frameout, SD_CMD_LENGTH) != BSP_ERROR_NONE)
  {
    return 0xFFFF;
  }
  tmp = SD_DUMMY_BYTE;
  switch(Answer)
  {
  case SD_ANSWER_R1_EXPECTED :
    /* Sends one byte command to the SD card and get response */
    if(SD_ReadData(&response_tmp) != BSP_ERROR_NONE)
    {
      return 0xFFFF;
    }
    response = response_tmp;
    break;
  case SD_ANSWER_R1B_EXPECTED :
    /* Sends first byte command to the SD card and get response */
    if(SD_ReadData(&response_tmp) != BSP_ERROR_NONE)
    {
      return 0xFFFF;
    }
    response = response_tmp;
    /* Sends second byte command to the SD card and get response */
    if(BUS_SPIx_SendRecv(&tmp, &response_tmp, 1U) != BSP_ERROR_NONE)
    {
      return 0xFFFF;
    }
    response |= response_tmp;
    /* Set CS High */
    SD_IO_CSState(1);
    SPI_IO_Delay(1);
    /* Set CS Low */
    SD_IO_CSState(0);

    /* Wait IO line return 0xFF */
    if(BUS_SPIx_SendRecv(&tmp, &response_tmp, 1U) != BSP_ERROR_NONE)
    {
      return 0xFFFF;
    }
    while(response_tmp != 0xFFU)
    {
      if(BUS_SPIx_SendRecv(&tmp, &response_tmp, 1U) != BSP_ERROR_NONE)
      {
        return 0xFFFF;
      }
    }
    break;
  case SD_ANSWER_R2_EXPECTED :
    /* Sends first byte command to the SD card and get response */
    if(SD_ReadData(&response_tmp) != BSP_ERROR_NONE)
    {
      return 0xFFFF;
    }
    response = response_tmp;
    /* Sends second byte command to the SD card and get response */
    if(BUS_SPIx_SendRecv(&tmp, &response_tmp, 1U) != BSP_ERROR_NONE)
    {
      return 0xFFFF;
    }
    response |= ((uint32_t)response_tmp << 8);
    break;
  case SD_ANSWER_R3_EXPECTED :
  case SD_ANSWER_R7_EXPECTED :
    /* Sends first byte command to the SD card and get response */
    if(SD_ReadData(&response_tmp) != BSP_ERROR_NONE)
    {
      return 0xFFFF;
    }
    response = response_tmp;
    /* Sends second byte command to the SD card and get response */
    if(BUS_SPIx_SendRecv(&tmp, &response_tmp, 1U) != BSP_ERROR_NONE)
    {
      return 0xFFFF;
    }
    /* Only first and second responses are required */
    response |= ((uint32_t)response_tmp << 8U);
    /* Sends third byte command to the SD card and get response */
    if(BUS_SPIx_SendRecv(&tmp, &response_tmp, 1U) != BSP_ERROR_NONE)
    {
      return 0xFFFF;
    }
    /* Sends fourth byte command to the SD card and get response */
    if(BUS_SPIx_SendRecv(&tmp, &response_tmp, 1U) != BSP_ERROR_NONE)
    {
      return 0xFFFF;
    }
    /* Sends fifth byte command to the SD card and get response */
    if(BUS_SPIx_SendRecv(&tmp, &response_tmp, 1U) != BSP_ERROR_NONE)
    {
      return 0xFFFF;
    }

    break;
  default :
    break;
  }
  return response;
}

/**
  * @brief  Gets the SD card data response and check the busy flag.
  * @param  None
  * @retval The SD status: Read data response xxx0<status>1
  *         - status 010: Data accecpted
  *         - status 101: Data rejected due to a crc error
  *         - status 110: Data rejected due to a Write error.
  *         - status 111: Data rejected due to other error.
  */
static int32_t SD_GetDataResponse(uint8_t *DataResponse)
{
  uint8_t dataresponse, tmp, tmp1;
  *DataResponse = (uint8_t)SD_DATA_OTHER_ERROR;

  tmp = SD_DUMMY_BYTE;
  if(BUS_SPIx_SendRecv(&tmp, &dataresponse, 1U) != BSP_ERROR_NONE)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  /* read the busy response byte*/
  if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    /* Mask unused bits */
    switch (dataresponse & 0x1FU)
    {
    case SD_DATA_OK:
      *DataResponse = (uint8_t)SD_DATA_OK;

      /* Set CS High */
      SD_IO_CSState(1);
      /* Set CS Low */
      SD_IO_CSState(0);
      tmp = SD_DUMMY_BYTE;

      /* Wait IO line return 0xFF */
        if(BUS_SPIx_SendRecv(&tmp, &tmp1, 1U) != BSP_ERROR_NONE)
        {
          return BSP_ERROR_PERIPH_FAILURE;
        }
      while(tmp1 != 0xFFU)
      {
        if(BUS_SPIx_SendRecv(&tmp, &tmp1, 1U) != BSP_ERROR_NONE)
        {
          return BSP_ERROR_PERIPH_FAILURE;
        }
      }
      break;
    case SD_DATA_CRC_ERROR:
      *DataResponse =  (uint8_t)SD_DATA_CRC_ERROR;
      break;
    case SD_DATA_WRITE_ERROR:
      *DataResponse = (uint8_t)SD_DATA_WRITE_ERROR;
      break;
    default:
      break;
    }
  }

  /* Return response */
  return BSP_ERROR_NONE;
}

/**
  * @brief  Put the SD in Idle state.
  * @param  None
  * @retval SD status
  */
static int32_t SD_GoIdleState(void)
{
  uint32_t response;
  __IO uint8_t counter = 0;
  uint8_t tmp = SD_DUMMY_BYTE;

  /* Send CMD0 (SD_CMD_GO_IDLE_STATE) to put SD in SPI mode and
  wait for In Idle State Response (R1 Format) equal to 0x01 */
  do
  {
    counter++;
    response = SD_SendCmd(SD_CMD_GO_IDLE_STATE, 0U, 0x95U, (uint8_t)SD_ANSWER_R1_EXPECTED);
    SD_IO_CSState(1);
    if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
    {
      return BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      if(counter >= SD_MAX_TRY)
      {
        return BSP_ERROR_UNKNOWN_FAILURE;
      }
    }
  }while((uint8_t)(response & 0xFFU) != (uint8_t)SD_R1_IN_IDLE_STATE);

  /* Send CMD8 (SD_CMD_SEND_IF_COND) to check the power supply status
  and wait until response (R7 Format) equal to 0xAA and */
  response = SD_SendCmd(SD_CMD_SEND_IF_COND, 0x1AAU, 0x87U, (uint8_t)SD_ANSWER_R7_EXPECTED);
  SD_IO_CSState(1);
  if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  else if(((uint8_t)(response & 0xFFU)  & (uint8_t)SD_R1_ILLEGAL_COMMAND) == (uint8_t)SD_R1_ILLEGAL_COMMAND)
  {
    /* initialise card V1 */
    do
    {
      /* initialise card V1 */
      /* Send CMD55 (SD_CMD_APP_CMD) before any ACMD command: R1 response (0x00: no errors) */
      (void)SD_SendCmd(SD_CMD_APP_CMD, 0x00U, 0xFFU, (uint8_t)SD_ANSWER_R1_EXPECTED);
      SD_IO_CSState(1);
      if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
      {
        return BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        /* Send ACMD41 (SD_CMD_SD_APP_OP_COND) to initialize SDHC or SDXC cards: R1 response (0x00: no errors) */
        response = SD_SendCmd(SD_CMD_SD_APP_OP_COND, 0x00U, 0xFFU, (uint8_t)SD_ANSWER_R1_EXPECTED);
        SD_IO_CSState(1);
        if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
        {
          return BSP_ERROR_PERIPH_FAILURE;
        }
      }
    }
    while((uint8_t)(response & 0xFFU) == (uint8_t)SD_R1_IN_IDLE_STATE);
    CardType = ADAFRUIT_802_CARD_SDSC;
  }
  else if((uint8_t)(response & 0xFFU) == (uint8_t)SD_R1_IN_IDLE_STATE)
  {
    /* initialise card V2 */
    do {
      /* Send CMD55 (SD_CMD_APP_CMD) before any ACMD command: R1 response (0x00: no errors) */
      (void)SD_SendCmd(SD_CMD_APP_CMD, 0, 0xFFU, (uint8_t)SD_ANSWER_R1_EXPECTED);
      SD_IO_CSState(1);
      if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
      {
        return BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        /* Send ACMD41 (SD_CMD_SD_APP_OP_COND) to initialize SDHC or SDXC cards: R1 response (0x00: no errors) */
        response = SD_SendCmd(SD_CMD_SD_APP_OP_COND, 0x40000000U, 0xFFU, (uint8_t)SD_ANSWER_R1_EXPECTED);
        SD_IO_CSState(1);
        if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
        {
          return BSP_ERROR_PERIPH_FAILURE;
        }
      }
    }
    while(((uint8_t)response & 0xFFU) == (uint8_t)SD_R1_IN_IDLE_STATE);

    if(((uint8_t)(response & 0xFFU) & (uint8_t)SD_R1_ILLEGAL_COMMAND) == (uint8_t)SD_R1_ILLEGAL_COMMAND)
    {
      do {
        /* Send CMD55 (SD_CMD_APP_CMD) before any ACMD command: R1 response (0x00: no errors) */
        (void)SD_SendCmd(SD_CMD_APP_CMD, 0U, 0xFFU, (uint8_t)SD_ANSWER_R1_EXPECTED);
        SD_IO_CSState(1);
        if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
        {
          return BSP_ERROR_PERIPH_FAILURE;
        }
        else
        {
          if((uint8_t)(response & 0xFFU) != (uint8_t)SD_R1_IN_IDLE_STATE)
          {
            return BSP_ERROR_UNKNOWN_FAILURE;
          }
          /* Send ACMD41 (SD_CMD_SD_APP_OP_COND) to initialize SDHC or SDXC cards: R1 response (0x00: no errors) */
          response = SD_SendCmd(SD_CMD_SD_APP_OP_COND, 0x00U, 0xFFU, (uint8_t)SD_ANSWER_R1_EXPECTED);
          SD_IO_CSState(1);
          if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
          {
            return BSP_ERROR_PERIPH_FAILURE;
          }
        }
      }
      while((uint8_t)(response & 0xFFU) == (uint8_t)SD_R1_IN_IDLE_STATE);
    }

    /* Send CMD58 (SD_CMD_READ_OCR) to initialize SDHC or SDXC cards: R3 response (0x00: no errors) */
    response = SD_SendCmd(SD_CMD_READ_OCR, 0x00U, 0xFFU, (uint8_t)SD_ANSWER_R3_EXPECTED);
    SD_IO_CSState(1);
    if(BUS_SPIx_Send(&tmp, 1U) != BSP_ERROR_NONE)
    {
      return BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      if((uint8_t)(response & 0xFFU) != (uint8_t)SD_R1_NO_ERROR)
      {
        return BSP_ERROR_UNKNOWN_FAILURE;
      }
      CardType = (uint32_t)(((response >> 8U) & 0x40U) >> 6U);
    }
  }
  else
  {
    return BSP_ERROR_BUSY;
  }

  return BSP_ERROR_NONE;
}

/**
  * @brief  Waits a data until a value different from SD_DUMMY_BITE
  * @param  Data data to be read
  * @retval BSP status
  */
static int32_t SD_ReadData(uint8_t *Data)
{
  uint8_t timeout = 0x08U;
  uint8_t tmp;

  tmp = SD_DUMMY_BYTE;
  /* Check if response is got or a timeout is happen */
  do
  {
    if(BUS_SPIx_SendRecv(&tmp, Data, 1) != BSP_ERROR_NONE)
    {
      return BSP_ERROR_PERIPH_FAILURE;
    }
    timeout--;

  }while ((*Data == SD_DUMMY_BYTE) && (timeout != 0U));

  if (timeout == 0U)
  {
    /* After time out */
    return BSP_ERROR_BUSY;
  }

  /* Right response got */
  return BSP_ERROR_NONE;
}

/**
  * @brief  Waits a data from the SD card
  * @param  data  Expected data from the SD card
  * @retval BSP status
  */
static int32_t SD_WaitData(uint8_t Data)
{
  uint16_t timeout = 0xFFFF;
  uint8_t readvalue, tmp;

  tmp = SD_DUMMY_BYTE;

  /* Check if response is got or a timeout is happen */
  do
  {
    if(BUS_SPIx_SendRecv(&tmp, &readvalue, 1) != BSP_ERROR_NONE)
    {
      return BSP_ERROR_PERIPH_FAILURE;
    }
    timeout--;
  }while ((readvalue != Data) && (timeout != 0U));

  if (timeout == 0U)
  {
    /* After time out */
    return BSP_ERROR_BUSY;
  }

  /* Right response got */
  return BSP_ERROR_NONE;
}

/**
  * @brief  SPI IO delay
  * @param  Delay  Delay in ms
  * @retval None
  */
static void SPI_IO_Delay(uint32_t Delay)
{
  int32_t tickstart;
  tickstart = BSP_GetTick();
  while((BSP_GetTick() - tickstart) < (int32_t)Delay)
  {
  }
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

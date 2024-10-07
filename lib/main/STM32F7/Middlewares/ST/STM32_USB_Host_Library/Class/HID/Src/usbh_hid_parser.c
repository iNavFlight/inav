/**
  ******************************************************************************
  * @file    usbh_hid_parser.c
  * @author  MCD Application Team
  * @brief   This file is the HID Layer Handlers for USB Host HID class.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
- "stm32xxxxx_{eval}{discovery}{adafruit}_lcd.c"
- "stm32xxxxx_{eval}{discovery}_sdram.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid_parser.h"


/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_HID_CLASS
  * @{
  */

/** @defgroup USBH_HID_PARSER
  * @brief    This file includes HID parsers for USB Host HID class.
  * @{
  */

/** @defgroup USBH_HID_PARSER_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_HID_PARSER_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_HID_PARSER_Private_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_HID_PARSER_Private_FunctionPrototypes
  * @{
  */

/**
  * @}
  */


/** @defgroup USBH_HID_PARSER_Private_Variables
  * @{
  */

/**
  * @}
  */


/** @defgroup USBH_HID_PARSER_Private_Functions
  * @{
  */

/**
  * @brief  HID_ReadItem
  *         The function read a report item.
  * @param  ri: report item
  * @param  ndx: report index
  * @retval status (0 : fail / otherwise: item value)
  */
uint32_t HID_ReadItem(HID_Report_ItemTypedef *ri, uint8_t ndx)
{
  uint32_t val = 0U;
  uint32_t x = 0U;
  uint32_t bofs;
  uint8_t *data = ri->data;
  uint8_t shift = ri->shift;

  /* get the logical value of the item */

  /* if this is an array, we may need to offset ri->data.*/
  if (ri->count > 0U)
  {
    /* If app tries to read outside of the array. */
    if (ri->count <= ndx)
    {
      return (0U);
    }

    /* calculate bit offset */
    bofs = ndx * ri->size;
    bofs += shift;
    /* calculate byte offset + shift pair from bit offset. */
    data += bofs / 8U;
    shift = (uint8_t)(bofs % 8U);
  }
  /* read data bytes in little endian order */
  for (x = 0U; x < (((ri->size & 0x7U) != 0U) ? ((ri->size / 8U) + 1U) : (ri->size / 8U)); x++)
  {
    val = (uint32_t)((uint32_t)(*data) << (x * 8U));
  }
  val = (val >> shift) & (((uint32_t)1U << ri->size) - 1U);

  if ((val < ri->logical_min) || (val > ri->logical_max))
  {
    return (0U);
  }

  /* convert logical value to physical value */
  /* See if the number is negative or not. */
  if ((ri->sign != 0U) && ((val & ((uint32_t)1U << (ri->size - 1U))) != 0U))
  {
    /* yes, so sign extend value to 32 bits. */
    uint32_t vs = (uint32_t)((0xffffffffU & ~((1U << (ri->size)) - 1U)) | val);

    if (ri->resolution == 1U)
    {
      return ((uint32_t)vs);
    }
    return ((uint32_t)(vs * ri->resolution));
  }
  else
  {
    if (ri->resolution == 1U)
    {
      return (val);
    }
    return (val * ri->resolution);
  }
}

/**
  * @brief  HID_WriteItem
  *         The function write a report item.
  * @param  ri: report item
  * @param  ndx: report index
  * @retval status (1: fail/ 0 : Ok)
  */
uint32_t HID_WriteItem(HID_Report_ItemTypedef *ri, uint32_t value, uint8_t ndx)
{
  uint32_t x;
  uint32_t mask;
  uint32_t bofs;
  uint8_t *data = ri->data;
  uint8_t shift = ri->shift;

  if ((value < ri->physical_min) || (value > ri->physical_max))
  {
    return (1U);
  }

  /* if this is an array, we may need to offset ri->data.*/
  if (ri->count > 0U)
  {
    /* If app tries to read outside of the array. */
    if (ri->count >= ndx)
    {
      return (0U);
    }
    /* calculate bit offset */
    bofs = ndx * ri->size;
    bofs += shift;
    /* calculate byte offset + shift pair from bit offset. */
    data += bofs / 8U;
    shift = (uint8_t)(bofs % 8U);

  }

  /* Convert physical value to logical value. */
  if (ri->resolution != 1U)
  {
    value = value / ri->resolution;
  }

  /* Write logical value to report in little endian order. */
  mask = ((uint32_t)1U << ri->size) - 1U;
  value = (value & mask) << shift;

  for (x = 0U; x < (((ri->size & 0x7U) != 0U) ? ((ri->size / 8U) + 1U) : (ri->size / 8U)); x++)
  {
    *(ri->data + x) = (uint8_t)((*(ri->data + x) & ~(mask >> (x * 8U))) |
                                ((value >> (x * 8U)) & (mask >> (x * 8U))));
  }

  return 0U;
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


/**
  * @}
  */


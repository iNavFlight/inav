/**
  ******************************************************************************
  * @file    otp_interface.h
  * @author  MCD Application Team
  * @brief   Header for otp_interface.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef OTP_INTERFACE_H
#define OTP_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint8_t OPENBL_OTP_Read(uint32_t Address);
void OPENBL_OTP_Write(uint32_t Address, uint8_t *Data, uint32_t DataLength);

#ifdef __cplusplus
}
#endif

#endif /* OPTIONBYTES_INTERFACE_H */

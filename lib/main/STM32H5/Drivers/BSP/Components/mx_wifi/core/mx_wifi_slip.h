/**
  ******************************************************************************
  * @file    mx_wifi_slip.h
  * @author  MCD Application Team
  * @brief   Header for mx_wifi_slip.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MX_WIFI_SLIP_H
#define MX_WIFI_SLIP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/*
 * CONFIGURATIONS
 */

/* SLIP_PACKET
 * |--------+---------+--------|
 * | START  | PAYLOAD | END    |
 * |--------+---------+--------|
 * | 1Bytes | nBytes  | 1Bytes |
 * |--------+---------+--------|
 */

/*
 * API
 */

/**
  * @brief  transfer HCI data to SLIP packet
  *
  * @param  data: data to be transfer
  * @param  len: size of the data to be transfer
  * @retval the SLIP packet
  */
uint8_t *slip_transfer(uint8_t data[], uint16_t len, uint16_t *outlen);


/**
  * @brief  Feed one serial byte to SLIP
  * @note   use slip_buf_free to free slip buffer if data process finished
  *
  * @param  data: one serial byte
  * @retval new SLIP frame, NULL if no new frame
  */
mx_buf_t *slip_input_byte(uint8_t data);


/**
  * @brief  free slip frame buffer returned by slip_input_byte
  *
  * @param  buf: slip frame buffer
  */
void slip_buf_free(uint8_t *buf);


enum
{
  SLIP_START        = 0xC0,
  SLIP_END          = 0xD0,
  SLIP_ESCAPE       = 0xDB,
  SLIP_ESCAPE_START = 0xDC,
  SLIP_ESCAPE_ES    = 0xDD,
  SLIP_ESCAPE_END   = 0xDE
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MX_WIFI_SLIP_H */

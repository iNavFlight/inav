/* USER CODE BEGIN Header */
/**
  *  Portions COPYRIGHT 2018 STMicroelectronics, All Rights Reserved
  *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
  *
  ******************************************************************************
  * @file    rng_alt_template.c
  * @author  MCD Application Team
  * @brief   mbedtls alternate entropy data function.
  *          the mbedtls_hardware_poll() is customized to use the STM32 RNG
  *          to generate random data, required for TLS encryption algorithms.
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Apache 2.0 license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  * https://opensource.org/licenses/Apache-2.0
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "mbedtls/entropy.h"
#include "mbedtls/entropy_poll.h"
#include "mbedtls/platform.h"

#ifdef MBEDTLS_ENTROPY_HARDWARE_ALT
/*
 * include the correct headerfile depending on the STM32 family */

#include "stm32XXXXX_hal.h"
#include <string.h>

static __IO uint32_t isInitialized = 0;

static RNG_HandleTypeDef RNG_Handle;


static void RNG_Init(void);
/* RNG init function */
static void RNG_Init(void)
{
  if (isInitialized == 0)
  {
      RNG_Handle.Instance = RNG;
      /* DeInitialize the RNG peripheral */
      if (HAL_RNG_DeInit(&RNG_Handle) != HAL_OK)
      {
        return;
      }

      /* Initialize the RNG peripheral */
      if (HAL_RNG_Init(&RNG_Handle) != HAL_OK)
      {
        return;
      }
      isInitialized = 1;
  }
}



int mbedtls_hardware_poll( void *Data, unsigned char *Output, size_t Len, size_t *oLen )
{
  __IO uint8_t random_value[4];
  int ret = 0;

  RNG_Init();

  if (isInitialized == 0)
  {
    ret = MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
  }
  else
  {
      *oLen = 0;
      while ((*oLen < Len) && (ret == 0))
      {
        if (HAL_RNG_GenerateRandomNumber(&RNG_Handle, (uint32_t *)random_value)) == HAL_OK)
        {
          for (uint8_t i = 0; (i < sizeof(uint32_t)) && (*oLen < Len) ; i++)
          {
            Output[*oLen] = random_value[i];
            *oLen += 1;
          }
        }
        else
        {
          ret = MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
        }
      }

      /* Just be extra sure that we didn't do it wrong */
      if ((__HAL_RNG_GET_FLAG(&RNG_Handle, (RNG_FLAG_CECS | RNG_FLAG_SECS))) != 0)
      {
        *oLen = 0;
        ret = MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
      }
  }

  return ret;
}

#if 0

/*
 * HAL_RNG_MspInit() and HAL_RNG_MspDeInit() are put here as reference
 */


/**
  * @brief RNG MSP Initialization
  *   This function configures the hardware resources used in this application:
  *   - Peripheral's clock enable
  * @param hrng: RNG handle pointer
  * @retval None
  */

void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

  /*Select PLL output as RNG clock source */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RNG;
  PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_PLL;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  /* RNG Peripheral clock enable */
  __RNG_CLK_ENABLE();

}
/**
  * @brief RNG MSP De-Initialization
  *        This function freeze the hardware resources used in this application:
  *          - Disable the Peripheral's clock
  * @param hrng: RNG handle pointer
  * @retval None
  */
void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng)
{
  /* Enable RNG reset state */
  __HAL_RCC_RNG_FORCE_RESET();

  /* Release RNG from reset state */
  __HAL_RCC_RNG_RELEASE_RESET();
}
#endif

#endif /*MBEDTLS_ENTROPY_HARDWARE_ALT*/

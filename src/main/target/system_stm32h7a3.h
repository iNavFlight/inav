/**
  ******************************************************************************
  * @file    system_stm32f7xx.h
  * @author  MCD Application Team
  * @version V1.6.1
  * @date    21-October-2015
  * @brief   CMSIS Cortex-M4 Device System Source File for STM32F4xx devices.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

#ifndef __SYSTEM_STM32F7XX_H
#define __SYSTEM_STM32F7XX_H

#ifdef __cplusplus
 extern "C" {
#endif

extern uint32_t SystemCoreClock;          /*!< System Clock Frequency (Core Clock) */
extern void SystemInit(void);
extern void SystemClock_Config(void);

void PeriphCommonClock_Config(void);
extern void MPU_Config(void);
extern void MX_GPIO_Init(void);
extern void MX_DMA_Init(void);
extern void MX_BDMA1_Init(void);
extern void MX_BDMA2_Init(void);
extern void MX_SPI1_Init(void);
extern void MX_SPI2_Init(void);
extern void MX_UART4_Init(void);
extern void MX_USART2_UART_Init(void);
extern void MX_USART3_UART_Init(void);
extern void MX_USART6_UART_Init(void);
extern void MX_TIM1_Init(void);
extern void MX_TIM2_Init(void);
extern void MX_UART5_Init(void);
extern void MX_TIM4_Init(void);
extern void MX_TIM3_Init(void);

#ifdef __cplusplus
}
#endif

#endif /*__SYSTEM_STM32F7XX_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

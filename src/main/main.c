/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"
#include "build/debug.h"
#include "drivers/serial.h"
#include "drivers/serial_softserial.h"

#include "fc/fc_init.h"

#include "scheduler/scheduler.h"

#if defined(SITL_BUILD)
#include "target/SITL/serial_proxy.h"
#endif


#ifdef SOFTSERIAL_LOOPBACK
serialPort_t *loopbackPort;
#endif


static void loopbackInit(void)
{
#ifdef SOFTSERIAL_LOOPBACK
    loopbackPort = softSerialLoopbackPort();
    serialPrint(loopbackPort, "LOOPBACK\r\n");
#endif
}

static void processLoopback(void)
{
#ifdef SOFTSERIAL_LOOPBACK
    if (loopbackPort) {
        uint8_t bytesWaiting;
        while ((bytesWaiting = serialRxBytesWaiting(loopbackPort))) {
            uint8_t b = serialRead(loopbackPort);
            serialWrite(loopbackPort, b);
        };
    }
#endif
}

#ifdef STM32H7A3xx_NONONONONO
ADC_HandleTypeDef hadc1;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart6;

DMA_HandleTypeDef hdma_memtomem_bdma1_channel0;
DMA_HandleTypeDef hdma_bdma_generator0;
DMA_HandleTypeDef hdma_dma_generator0;
DMA_HandleTypeDef hdma_dma_generator2;

 void MX_GPIO_Init(void);
 void MX_DMA_Init(void);
 void MX_BDMA1_Init(void);
 void MX_BDMA2_Init(void);
 void MX_SPI1_Init(void);
 void MX_SPI2_Init(void);
 void MX_UART4_Init(void);
 void MX_USART2_UART_Init(void);
 void MX_USART3_UART_Init(void);
 void MX_USART6_UART_Init(void);
 void MX_USB_DEVICE_Init(void);
 void MX_TIM1_Init(void);
 void MX_TIM2_Init(void);
 void MX_UART5_Init(void);
 void MX_TIM4_Init(void);
 void MX_TIM3_Init(void);
 void MX_USART1_UART_Init(void);
 void MX_ADC1_Init(void);
#endif

#if defined(SITL_BUILD)
int main(int argc, char *argv[])
{
    parseArguments(argc, argv);
#else
int main(void){
#endif

#ifdef STM32H7A3xx_NONONONONON
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_BDMA1_Init();
  MX_BDMA2_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_UART4_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_USART6_UART_Init();
  MX_USB_DEVICE_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_UART5_Init();
  MX_TIM4_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  HAL_GPIO_TogglePin (GPIOC, GPIO_PIN_13);
	  HAL_Delay (1);   /* Insert delay 1 ms */
	  // HAL_GPIO_TogglePin (GPIOC, GPIO_PIN_0);
	  //CDC_Transmit_HS((uint8_t *)"Hello!\r\n", 8);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
#else
    init();
    loopbackInit();

    while (true) {
#if defined(SITL_BUILD)
        serialProxyProcess();
#endif
        scheduler();
        processLoopback();
    }
#endif
}

#pragma once

#include "stm32h7xx_hal.h"

#ifdef TEST_FULL_FLASH
#define FLASH_TEST_START_ADDRESS 	((uint8_t *)0x08000000)
#define FLASH_TEST_SIZE				(2048 * 1024)
#else
#define FLASH_TEST_START_ADDRESS 	((uint8_t *)0x081C0000)
#define FLASH_TEST_SIZE				(256 * 1024)
#endif
#define FLASH_TEST_END_ADDRESS	(FLASH_TEST_START_ADDRESS + FLASH_TEST_SIZE)


extern ADC_HandleTypeDef hadc1;

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;

extern DMA_HandleTypeDef hdma_memtomem_bdma1_channel0;
extern DMA_HandleTypeDef hdma_bdma_generator0;
extern DMA_HandleTypeDef hdma_dma_generator0;
extern DMA_HandleTypeDef hdma_dma_generator2;


void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

void Error_Handler(void);


void SystemClock_Config(void);

void PeriphCommonClock_Config(void);

void stm32h7a3_rcc_clock_config(void);

void MPU_Config(void);
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
void MX_TIM1_Init(void);
void MX_TIM2_Init(void);
void MX_UART5_Init(void);
void MX_TIM4_Init(void);
void MX_TIM3_Init(void);
void MX_USART1_UART_Init(void);
void MX_ADC1_Init(void);
void MX_RTC_Init(void);
void MX_I2C3_Init(void);

int stm32h7a3_main(void);
void stm32h7a3_init(void);

uint8_t _checksum(uint8_t current, uint8_t newData);
uint8_t readAllFlash(void);

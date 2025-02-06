#pragma once

#include <stdint.h>

#ifdef STM32H73Axx
#include "stm32h73a_hal_conf.h"
#elif defined(STM32H7)
#include "stm32h7xx_hal_conf.h"
#else
#error "Unknown MCU"
#endif

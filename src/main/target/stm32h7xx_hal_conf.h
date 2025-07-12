#pragma once

#include <stdint.h>

#ifdef STM32H7A3xx
#include "stm32h7a3_hal_conf.h"
#elif defined(STM32H7)
#include "stm32h743_hal_conf.h"
#else
#error "Unknown MCU"
#endif

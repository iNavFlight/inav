#pragma once

#include <stdint.h>
#include <stdbool.h>

/*
 * Shared, HAL-free CAN bit-timing solver used by both the STM32 bxCAN (F7)
 * and FDCAN (H7) DroneCAN drivers. Each driver applies its own
 * peripheral-specific SJW and BS1/BS2 register-offset convention on top of
 * this solution — see canardSTM32ComputeTimings() in
 * canard_stm32f7xx_driver.c and canard_stm32h7xx_driver.c.
 */

typedef struct {
    uint16_t prescaler;
    uint8_t  bs1;   // raw solved value, 1..16, no register offset applied
    uint8_t  bs2;   // raw solved value, 1..8, no register offset applied
} CanardCanTimingSolution;

bool canardComputeCanTimingSolution(uint32_t pclk, uint32_t target_bitrate, CanardCanTimingSolution *out_solution);

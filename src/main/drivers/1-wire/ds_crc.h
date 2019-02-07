
#pragma once

#include <stdint.h>

uint8_t ds_crc8_bit_update(uint8_t crc, uint8_t bit);
uint8_t ds_crc8_byte_update(uint8_t crc, uint8_t byte);
uint8_t ds_crc8(const uint8_t *addr, uint8_t len);

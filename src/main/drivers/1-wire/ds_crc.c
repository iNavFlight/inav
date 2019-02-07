
#include <stdint.h>

uint8_t ds_crc8_bit_update(uint8_t crc8, uint8_t bit)
{
    uint8_t crc8_base = (crc8 >> 1) & 0x73;
    uint8_t xor1 = (crc8 & 1) ^ bit;
    return crc8_base | ((((crc8 >> 3) & 1) ^ xor1) << 2) | ((((crc8 >> 4) & 1) ^ xor1) << 3) | (xor1 << 7);
}

uint8_t ds_crc8_byte_update(uint8_t crc8, uint8_t byte)
{
    for (uint8_t i = 0; i < 8; ++i) {
        uint8_t mix = (crc8 ^ byte) & 0x01;
        crc8 >>= 1;
        if (mix) crc8 ^= 0x8C;
        byte >>= 1;
    }
    return crc8;
}

uint8_t ds_crc8(const uint8_t *addr, uint8_t len)
{
     uint8_t crc8 = 0;
     for (uint8_t i=0; i < len; ++i) crc8 = ds_crc8_byte_update(crc8, addr[i]);
     return crc8;
}


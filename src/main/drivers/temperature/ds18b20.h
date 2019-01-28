
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "drivers/1-wire.h"

#if defined(USE_1WIRE) && defined(USE_1WIRE_DS2482) && defined(USE_TEMPERATURE_DS18B20)

#define DS18B20_DRIVER_AVAILABLE

#define DS18B20_CONFIG_9BIT 0x1F
#define DS18B20_CONFIG_10BIT 0x3F
#define DS18B20_CONFIG_11BIT 0x5F
#define DS18B20_CONFIG_12BIT 0x7F

// milliseconds
#define DS18B20_9BIT_CONVERSION_TIME 94
#define DS18B20_10BIT_CONVERSION_TIME 188
#define DS18B20_11BIT_CONVERSION_TIME 375
#define DS18B20_12BIT_CONVERSION_TIME 750


bool ds18b20_enumerate(uint64_t *rom_table, uint8_t *rom_table_len);
bool ds18b20_configure(uint64_t rom, uint8_t config);
bool ds18b20_wait_for_conversion();
bool ds18b20_start_conversion();
bool ds18b20_wait_for_conversion();
bool ds18b20_read_temperature(uint64_t rom, int16_t *temperature);

#endif /* defined(USE_1WIRE) && defined(USE_1WIRE_DS2482) && defined(USE_TEMPERATURE_DS18B20) */

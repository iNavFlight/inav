/*
 * Eeprom emulation for K20x chips.
 * (c) 2015 flabbergast
 * Most of the code is from PJRC/Teensyduino (license in eeprom.c)
 */

#ifndef _EEPROM_H_
#define _EEPROM_H_

void eeprom_initialize(void);
int eeprom_is_ready(void);
uint8_t eeprom_read_byte(const uint8_t *addr);
uint16_t eeprom_read_word(const uint16_t *addr);
uint32_t eeprom_read_dword(const uint32_t *addr);
void eeprom_read_block(void *buf, const void *addr, uint32_t len);
void eeprom_write_byte(uint8_t *addr, uint8_t data);
void eeprom_write_word(uint16_t *addr, uint16_t value);
void eeprom_write_dword(uint32_t *addr, uint32_t value);
void eeprom_write_block(const void *buf, void *addr, uint32_t len);

#endif /* _EEPROM_H_ */
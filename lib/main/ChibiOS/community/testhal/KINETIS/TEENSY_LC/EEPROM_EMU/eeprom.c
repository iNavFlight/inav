/*
 * Eeprom emulation for KL2x chips.
 * (c) 2015 flabbergast
 * Most of the code is from PJRC/Teensyduino (license below)
 *
 * Notes: Some wear-levelling is done:
 *  - emulating 128 bytes of eeprom; i.e. 7 bit "eeprom addresses"
 *  - using 2048 bytes of flash
 *  - new values are written consecutively into flash
 *    as 16bit ("eeprom address",value) pairs
 *  - if all 2048 bytes of flash is used, it is erased and writes
 *    start from the beginning again
 *  - the 2048 bytes of flash used are at the end of the flash
 *  - BEWARE: there is no protection! Use a custom .ld script
 *    to make sure this area is never used for code!
 */

/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows 
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ch.h"
#include "hal.h"

#define SYMVAL(sym) (uint32_t)(((uint8_t *)&(sym)) - ((uint8_t *)0))

extern uint32_t __eeprom_workarea_start__;
extern uint32_t __eeprom_workarea_end__;

#define EEPROM_SIZE 128

static uint32_t flashend = 0;

void eeprom_initialize(void)
{
	const uint16_t *p = (uint16_t *)SYMVAL(__eeprom_workarea_start__);

	do {
		if (*p++ == 0xFFFF) {
			flashend = (uint32_t)(p - 2);
			return;
		}
	} while (p < (uint16_t *)SYMVAL(__eeprom_workarea_end__));
	flashend = (uint32_t)((uint16_t *)SYMVAL(__eeprom_workarea_end__) - 1);
}

uint8_t eeprom_read_byte(const uint8_t *addr)
{
	uint32_t offset = (uint32_t)addr;
	const uint16_t *p = (uint16_t *)SYMVAL(__eeprom_workarea_start__);
	const uint16_t *end = (const uint16_t *)((uint32_t)flashend);
	uint16_t val;
	uint8_t data=0xFF;

	if (!end) {
		eeprom_initialize();
		end = (const uint16_t *)((uint32_t)flashend);
	}
	if (offset < EEPROM_SIZE) {
		while (p <= end) {
			val = *p++;
			if ((val & 255) == offset) data = val >> 8;
		}
	}
	return data;
}

static void flash_write(const uint16_t *code, uint32_t addr, uint32_t data)
{
	// with great power comes great responsibility....
	uint32_t stat;
	*(uint32_t *)&(FTFA->FCCOB3) = 0x06000000 | (addr & 0x00FFFFFC);
	*(uint32_t *)&(FTFA->FCCOB7) = data;
	__disable_irq();
	(*((void (*)(volatile uint8_t *))((uint32_t)code | 1)))(&(FTFA->FSTAT));
	__enable_irq();
	stat = FTFA->FSTAT & (FTFA_FSTAT_RDCOLERR|FTFA_FSTAT_ACCERR|FTFA_FSTAT_FPVIOL);
	if (stat) {
		FTFA->FSTAT = stat;
	}
	MCM->PLACR |= MCM_PLACR_CFCC;
}

void eeprom_write_byte(uint8_t *addr, uint8_t data)
{
	uint32_t offset = (uint32_t)addr;
	const uint16_t *p, *end = (const uint16_t *)((uint32_t)flashend);
	uint32_t i, val, flashaddr;
	uint16_t do_flash_cmd[] = {
		0x2380, 0x7003, 0x7803, 0xb25b, 0x2b00, 0xdafb, 0x4770};
	uint8_t buf[EEPROM_SIZE];

	if (offset >= EEPROM_SIZE) return;
	if (!end) {
		eeprom_initialize();
		end = (const uint16_t *)((uint32_t)flashend);
	}
	if (++end < (uint16_t *)SYMVAL(__eeprom_workarea_end__)) {
		val = (data << 8) | offset;
		flashaddr = (uint32_t)end;
		flashend = flashaddr;
		if ((flashaddr & 2) == 0) {
			val |= 0xFFFF0000;
		} else {
			val <<= 16;
			val |= 0x0000FFFF;
		}
		flash_write(do_flash_cmd, flashaddr, val);
	} else {
		for (i=0; i < EEPROM_SIZE; i++) {
			buf[i] = 0xFF;
		}
		for (p = (uint16_t *)SYMVAL(__eeprom_workarea_start__); p < (uint16_t *)SYMVAL(__eeprom_workarea_end__); p++) {
			val = *p;
			if ((val & 255) < EEPROM_SIZE) {
				buf[val & 255] = val >> 8;
			}
		}
		buf[offset] = data;
		for (flashaddr=(uint32_t)(uint16_t *)SYMVAL(__eeprom_workarea_start__); flashaddr < (uint32_t)(uint16_t *)SYMVAL(__eeprom_workarea_end__); flashaddr += 1024) {
			*(uint32_t *)&(FTFA->FCCOB3) = 0x09000000 | flashaddr;
			__disable_irq();
			(*((void (*)(volatile uint8_t *))((uint32_t)do_flash_cmd | 1)))(&(FTFA->FSTAT));
			__enable_irq();
			val = FTFA->FSTAT & (FTFA_FSTAT_RDCOLERR|FTFA_FSTAT_ACCERR|FTFA_FSTAT_FPVIOL);;
			if (val) FTFA->FSTAT = val;
			MCM->PLACR |= MCM_PLACR_CFCC;
		}
		flashaddr=(uint32_t)(uint16_t *)SYMVAL(__eeprom_workarea_start__);
		for (i=0; i < EEPROM_SIZE; i++) {
			if (buf[i] == 0xFF) continue;
			if ((flashaddr & 2) == 0) {
				val = (buf[i] << 8) | i;
			} else {
				val = val | (buf[i] << 24) | (i << 16);
				flash_write(do_flash_cmd, flashaddr, val);
			}
			flashaddr += 2;
		}
		flashend = flashaddr;
		if ((flashaddr & 2)) {
			val |= 0xFFFF0000;
			flash_write(do_flash_cmd, flashaddr, val);
		}
	}
}

/*
void do_flash_cmd(volatile uint8_t *fstat)
{
        *fstat = 0x80;
        while ((*fstat & 0x80) == 0) ; // wait
}
00000000 <do_flash_cmd>:
   0:	2380      	movs	r3, #128	; 0x80
   2:	7003      	strb	r3, [r0, #0]
   4:	7803      	ldrb	r3, [r0, #0]
   6:	b25b      	sxtb	r3, r3
   8:	2b00      	cmp	r3, #0
   a:	dafb      	bge.n	4 <do_flash_cmd+0x4>
   c:	4770      	bx	lr
*/


uint16_t eeprom_read_word(const uint16_t *addr)
{
	const uint8_t *p = (const uint8_t *)addr;
	return eeprom_read_byte(p) | (eeprom_read_byte(p+1) << 8);
}

uint32_t eeprom_read_dword(const uint32_t *addr)
{
	const uint8_t *p = (const uint8_t *)addr;
	return eeprom_read_byte(p) | (eeprom_read_byte(p+1) << 8)
		| (eeprom_read_byte(p+2) << 16) | (eeprom_read_byte(p+3) << 24);
}

void eeprom_read_block(void *buf, const void *addr, uint32_t len)
{
	const uint8_t *p = (const uint8_t *)addr;
	uint8_t *dest = (uint8_t *)buf;
	while (len--) {
		*dest++ = eeprom_read_byte(p++);
	}
}

int eeprom_is_ready(void)
{
	return 1;
}

void eeprom_write_word(uint16_t *addr, uint16_t value)
{
	uint8_t *p = (uint8_t *)addr;
	eeprom_write_byte(p++, value);
	eeprom_write_byte(p, value >> 8);
}

void eeprom_write_dword(uint32_t *addr, uint32_t value)
{
	uint8_t *p = (uint8_t *)addr;
	eeprom_write_byte(p++, value);
	eeprom_write_byte(p++, value >> 8);
	eeprom_write_byte(p++, value >> 16);
	eeprom_write_byte(p, value >> 24);
}

void eeprom_write_block(const void *buf, void *addr, uint32_t len)
{
	uint8_t *p = (uint8_t *)addr;
	const uint8_t *src = (const uint8_t *)buf;
	while (len--) {
		eeprom_write_byte(p++, *src++);
	}
}

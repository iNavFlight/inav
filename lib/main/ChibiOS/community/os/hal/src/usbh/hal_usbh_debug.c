/*
    ChibiOS - Copyright (C) 2006..2017 Giovanni Di Sirio
              Copyright (C) 2015..2017 Diego Ismirlian, (dismirlian (at) google's mail)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "hal.h"

#if HAL_USE_USBH && USBH_DEBUG_ENABLE

#include "ch.h"
#include "usbh/debug.h"
#include <stdarg.h>
#include "chprintf.h"

#define MAX_FILLER 11
#define FLOAT_PRECISION 9
#define MPRINTF_USE_FLOAT 0

static char *long_to_string_with_divisor(char *p, long num, unsigned radix, long divisor) 
{
	int i;
	char *q;
	long l, ll;

	l = num;
	if (divisor == 0) {
		ll = num;
	} else {
		ll = divisor;
	}

	q = p + MAX_FILLER;
	do {
		i = (int)(l % radix);
		i += '0';
		if (i > '9') {
			i += 'A' - '0' - 10;
		}
		*--q = i;
		l /= radix;
	} while ((ll /= radix) != 0);

	i = (int)(p + MAX_FILLER - q);
	do {
		*p++ = *q++;
	} while (--i);

	return p;
}

static char *ltoa(char *p, long num, unsigned radix) {

	return long_to_string_with_divisor(p, num, radix, 0);
}

#if MPRINTF_USE_FLOAT
static const long _pow10[FLOAT_PRECISION] = {10, 100, 1000, 10000, 100000, 1000000,
	10000000, 100000000, 1000000000};
static const double m10[FLOAT_PRECISION] = {5.0/100, 5.0/1000, 5.0/10000, 5.0/100000, 5.0/1000000,
	5.0/10000000, 5.0/100000000, 5.0/1000000000, 5.0/10000000000};

static char *ftoa(char *p, double num, unsigned long precision, bool dot) {
	long l;
	char *q;
	double r;


	if (precision == 0) {
		l = (long)(num + 0.5);
		return long_to_string_with_divisor(p, l, 10, 0);
	} else {
		if (precision > FLOAT_PRECISION) precision = FLOAT_PRECISION;
		r = m10[precision - 1];
		precision = _pow10[precision - 1];

		l = (long)num;
		p = long_to_string_with_divisor(p, l, 10, 0);
		if (dot) *p++ = '.';
		l = (long)((num - l + r) * precision);
		q = long_to_string_with_divisor(p, l, 10, precision / 10) - 1;

		while (q > p) {
			if (*q != '0') {
				break;
			}
			--q;
		}
		return ++q;
	}

	


}
#endif

static inline void _wr(input_queue_t *iqp, char c) {
	*iqp->q_wrptr++ = c;
	if (iqp->q_wrptr >= iqp->q_top)
		iqp->q_wrptr = iqp->q_buffer;
}

static inline void _put(char c) {
	input_queue_t *iqp = &USBH_DEBUG_USBHD.iq;
	if (sizeof(USBH_DEBUG_USBHD.dbg_buff) - iqp->q_counter <= 1)
		return;
	iqp->q_counter++;
	_wr(iqp, c);
}

int _dbg_printf(const char *fmt, va_list ap) {
	char *p, *s, c, filler;
	int i, precision, width;
	int n = 0;
	bool is_long, left_align, sign;
	long l;
#if MPRINTF_USE_FLOAT
	double f;
	char tmpbuf[2*MAX_FILLER + 1];
#else
	char tmpbuf[MAX_FILLER + 1];
#endif

	for (;;) {

		//agarrar nuevo caracter de formato
		c = *fmt++;

		//chequeo eos
		if (c == 0) return n;

		//copio los caracteres comunes
		if (c != '%') {
			_put(c);
			n++;
			continue;
		}

		//encontré un '%'
		p = tmpbuf;
		s = tmpbuf;

		//left align
		left_align = FALSE;
		if (*fmt == '-') {
			fmt++;
			left_align = TRUE;
		}
		
		sign = FALSE;
		if (*fmt == '+') {
			fmt++;
			sign = TRUE;
		}

		//filler
		filler = ' ';
		if (*fmt == '0') {
			fmt++;
			filler = '0';
		}

		//width
		width = 0;
		while (TRUE) {
			c = *fmt++;
			if (c >= '0' && c <= '9')
				c -= '0';
			else if (c == '*')
				c = va_arg(ap, int);
			else
				break;
			width = width * 10 + c;
		}

		//precision
		precision = 0;
		if (c == '.') {

			if (*fmt == 'n') {
				fmt++;

			}
			while (TRUE) {
				c = *fmt++;
				if (c >= '0' && c <= '9')
					c -= '0';
				else if (c == '*')
					c = va_arg(ap, int);
				else
					break;
				precision = precision * 10 + c;
			}
		}

		//long modifier
		if (c == 'l' || c == 'L') {
			is_long = TRUE;
			if (*fmt)
				c = *fmt++;
		}
		else
			is_long = (c >= 'A') && (c <= 'Z');

		/* Command decoding.*/
		switch (c) {
		//char
		case 'c':
			filler = ' ';
			*p++ = va_arg(ap, int);
			break;

		//string
		case 's':
			filler = ' ';
			if ((s = va_arg(ap, char *)) == 0)
				s = (char *)"(null)";
			if (precision == 0)
				precision = 32767;

			//strlen con límite hasta precision
			for (p = s; *p && (--precision >= 0); p++)
				;
			break;



		case 'D':
		case 'd':
		case 'I':
		case 'i':
			if (is_long)
				l = va_arg(ap, long);
			else
				l = va_arg(ap, int);
			if (l < 0) {
				*p++ = '-';
				l = -l;
				sign = TRUE;
			} else if (sign) {
				*p++ = '+';
			}
			p = ltoa(p, l, 10);
			break;

#if MPRINTF_USE_FLOAT
		case 'f':
			f = va_arg(ap, double);
			if (f < 0) {
				*p++ = '-';
				f = -f;
				sign = TRUE;
			} else if (sign) {
				*p++ = '+';
			}
			if (prec == FALSE) precision = 6;
			p = ftoa(p, f, precision, dot);
			break;
#endif


		case 'X':
		case 'x':
			c = 16;
			goto unsigned_common;
		case 'U':
		case 'u':
			c = 10;
			goto unsigned_common;
		case 'O':
		case 'o':
			c = 8;

unsigned_common:
			if (is_long)
				l = va_arg(ap, unsigned long);
			else
				l = va_arg(ap, unsigned int);
			p = ltoa(p, l, c);
			break;

		//copiar
		default:
			*p++ = c;
			break;
		}

		//longitud
		i = (int)(p - s);

		//calculo cuántos caracteres de filler debo poner
		if ((width -= i) < 0)
			width = 0;

		if (left_align == FALSE)
			width = -width;

		if (width < 0) {
			//alineado a la derecha

			//poner el signo adelante
			if (sign && filler == '0') {
				_put(*s++);
				n++;
				i--;
			}

			//fill a la izquierda
			do {
				_put(filler);
				n++;
			} while (++width != 0);
		}

		//copiar los caracteres
		while (--i >= 0) {
			_put(*s++);
			n++;
		}

		//fill a la derecha
		while (width) {
			_put(filler);
			n++;
			width--;
		}
	}

	//return n; // can raise 'code is unreachable' warning

}

static systime_t first, last;
static bool ena;
static uint32_t hdr[2];

static void _build_hdr(void) {
	uint32_t hfnum = USBH_DEBUG_USBHD.otg->HFNUM;
	uint16_t hfir = USBH_DEBUG_USBHD.otg->HFIR;
	last = osalOsGetSystemTimeX();
	if (ena) {
		first = last;
	}

	if (((hfnum & 0x3fff) == 0x3fff) && (hfir == (hfnum >> 16))) {
		hdr[0] = 0xfeff;
		hdr[1] = last - first;
		ena = FALSE;
	} else {
		hdr[0] = 0xffff | (hfir << 16);
		hdr[1] = hfnum;
		ena = TRUE;
	}
}

static void _print_hdr(void)
{
	_put(hdr[0] & 0xff);
	_put((hdr[0] >> 8) & 0xff);
	_put((hdr[0] >> 16) & 0xff);
	_put((hdr[0] >> 24) & 0xff);
	_put(hdr[1] & 0xff);
	_put((hdr[1] >> 8) & 0xff);
	_put((hdr[1] >> 16) & 0xff);
	_put((hdr[1] >> 24) & 0xff);
}

void usbDbgPrintf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	syssts_t sts = chSysGetStatusAndLockX();
	input_queue_t *iqp = &USBH_DEBUG_USBHD.iq;
	int rem = sizeof(USBH_DEBUG_USBHD.dbg_buff) - iqp->q_counter;
	if (rem >= 9) {
		_build_hdr();
		_print_hdr();
		_dbg_printf(fmt, ap);
		iqp->q_counter++;
		_wr(iqp, 0);
		chThdDequeueNextI(&USBH_DEBUG_USBHD.iq.q_waiting, Q_OK);
	}
	chSysRestoreStatusX(sts);
	va_end(ap);
}


void usbDbgPuts(const char *s)
{
	_build_hdr();
	uint8_t *p = (uint8_t *)hdr;
	uint8_t *top = p + 8;

	syssts_t sts = chSysGetStatusAndLockX();
	input_queue_t *iqp = &USBH_DEBUG_USBHD.iq;
	int rem = sizeof(USBH_DEBUG_USBHD.dbg_buff) - iqp->q_counter;
	if (rem >= 9) {
		while (rem) {
			_wr(iqp, *p);
			if (++p == top) break;
		}
		rem -= 9;
		while (rem && *s) {
			_wr(iqp, *s);
			rem--;
			s++;
		}
		_wr(iqp, 0);
		iqp->q_counter = sizeof(USBH_DEBUG_USBHD.dbg_buff) - rem;
		chThdDequeueNextI(&USBH_DEBUG_USBHD.iq.q_waiting, Q_OK);
	}
	chSysRestoreStatusX(sts);
}

void usbDbgReset(void) {
	const char *msg = "\r\n\r\n==== DEBUG OUTPUT RESET ====\r\n";

	syssts_t sts = chSysGetStatusAndLockX();
	iqResetI(&USBH_DEBUG_USBHD.iq);
	oqResetI(&USBH_DEBUG_SD.oqueue);
	while (*msg) {
		*USBH_DEBUG_SD.oqueue.q_wrptr++ = *msg++;
		USBH_DEBUG_SD.oqueue.q_counter--;
	}
	chSysRestoreStatusX(sts);
}

static int _get(void) {
	if (!USBH_DEBUG_USBHD.iq.q_counter) return -1;
	USBH_DEBUG_USBHD.iq.q_counter--;
	uint8_t b = *USBH_DEBUG_USBHD.iq.q_rdptr++;
	if (USBH_DEBUG_USBHD.iq.q_rdptr >= USBH_DEBUG_USBHD.iq.q_top) {
		USBH_DEBUG_USBHD.iq.q_rdptr = USBH_DEBUG_USBHD.iq.q_buffer;
	}
	return b;
}

void usbDbgSystemHalted(void) {
	while (true) {
		if (!((bool)((USBH_DEBUG_SD.oqueue.q_wrptr == USBH_DEBUG_SD.oqueue.q_rdptr) && (USBH_DEBUG_SD.oqueue.q_counter != 0U))))
			break;
		USBH_DEBUG_SD.oqueue.q_counter++;
		while (!(USBH_DEBUG_SD.usart->SR & USART_SR_TXE));
		USBH_DEBUG_SD.usart->DR = *USBH_DEBUG_SD.oqueue.q_rdptr++;
		if (USBH_DEBUG_SD.oqueue.q_rdptr >= USBH_DEBUG_SD.oqueue.q_top) {
			USBH_DEBUG_SD.oqueue.q_rdptr = USBH_DEBUG_SD.oqueue.q_buffer;
		}
	}

	int c;
	int state = 0;
	for (;;) {
		c = _get(); if (c < 0) break;

		if (state == 0) {
			if (c == 0xff) state = 1;
		} else if (state == 1) {
			if (c == 0xff) state = 2;
			else (state = 0);
		} else {
			c = _get(); if (c < 0) return;
			c = _get(); if (c < 0) return;
			c = _get(); if (c < 0) return;
			c = _get(); if (c < 0) return;
			c = _get(); if (c < 0) return;

			while (true) {
				c = _get(); if (c < 0) return;
				if (!c) {
					while (!(USBH_DEBUG_SD.usart->SR & USART_SR_TXE));
					USBH_DEBUG_SD.usart->DR = '\r';
					while (!(USBH_DEBUG_SD.usart->SR & USART_SR_TXE));
					USBH_DEBUG_SD.usart->DR = '\n';
					state = 0;
					break;
				}
				while (!(USBH_DEBUG_SD.usart->SR & USART_SR_TXE));
				USBH_DEBUG_SD.usart->DR = c;
			}
		}
	}
}

static void usb_debug_thread(void *arg) {
	USBHDriver *host = (USBHDriver *)arg;
	uint8_t state = 0;

	chRegSetThreadName("USBH_DBG");
	while (true) {
		msg_t c = iqGet(&host->iq);
		if (c < 0) goto reset;

		if (state == 0) {
			if (c == 0xff) state = 1;
		} else if (state == 1) {
			if (c == 0xff) state = 2;
			else if (c == 0xfe) state = 3;
			else (state = 0);
		} else if (state == 2) {
			uint16_t hfir;
			uint32_t hfnum;

			hfir = c;
			c = iqGet(&host->iq); if (c < 0) goto reset;
			hfir |= c << 8;

			c = iqGet(&host->iq); if (c < 0) goto reset;
			hfnum = c;
			c = iqGet(&host->iq); if (c < 0) goto reset;
			hfnum |= c << 8;
			c = iqGet(&host->iq); if (c < 0) goto reset;
			hfnum |= c << 16;
			c = iqGet(&host->iq); if (c < 0) goto reset;
			hfnum |= c << 24;

			uint32_t f = hfnum & 0xffff;
			uint32_t p = 1000 - ((hfnum >> 16) / (hfir / 1000));
			chprintf((BaseSequentialStream *)&USBH_DEBUG_SD, "%05d.%03d ", f, p);
			state = 4;
		} else if (state == 3) {
			uint32_t t;

			c = iqGet(&host->iq); if (c < 0) goto reset;
			c = iqGet(&host->iq); if (c < 0) goto reset;

			t = c;
			c = iqGet(&host->iq); if (c < 0) goto reset;
			t |= c << 8;
			c = iqGet(&host->iq); if (c < 0) goto reset;
			t |= c << 16;
			c = iqGet(&host->iq); if (c < 0) goto reset;
			t |= c << 24;

			chprintf((BaseSequentialStream *)&USBH_DEBUG_SD, "+%08d ", t);
			state = 4;
		} else {
			while (true) {
				if (!c) {
					sdPut(&USBH_DEBUG_SD, '\r');
					sdPut(&USBH_DEBUG_SD, '\n');
					state = 0;
					break;
				}
				sdPut(&USBH_DEBUG_SD, (uint8_t)c);
				c = iqGet(&host->iq); if (c < 0) goto reset;
			}
		}

		continue;
reset:
		state = 0;
	}
}

void usbDbgInit(USBHDriver *host) {
	if (host != &USBH_DEBUG_USBHD)
		return;
	iqObjectInit(&USBH_DEBUG_USBHD.iq, USBH_DEBUG_USBHD.dbg_buff, sizeof(USBH_DEBUG_USBHD.dbg_buff), 0, 0);
	chThdCreateStatic(USBH_DEBUG_USBHD.waDebug, sizeof(USBH_DEBUG_USBHD.waDebug), NORMALPRIO, usb_debug_thread, &USBH_DEBUG_USBHD);
}

#endif

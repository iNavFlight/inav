/*
 * This file is part of INAV
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "io/osd_utils.h"

#include "common/maths.h"
#include "common/typeconversion.h"
#include "drivers/osd_symbols.h"
#include "io/displayport_msp_bf_compat.h"

#if defined(USE_OSD) || defined(OSD_UNIT_TEST)

int digitCount(int32_t value)
{
    int digits = 1;
    while(1) {
        value = value / 10;
        if (value == 0) {
            break;
        }
        digits++;
    }
    return digits;
}


bool osdFormatCentiNumber(char *buff, int32_t centivalue, uint32_t scale, int maxDecimals, int maxScaledDecimals, int length)
{
    char *ptr = buff;
    char *dec;
    int decimals = maxDecimals;
    bool negative = false;
    bool scaled = false;
    bool explicitDecimal = isBfCompatibleVideoSystem(osdConfig());

    buff[length] = '\0';

    if (centivalue < 0) {
        negative = true;
        centivalue = -centivalue;
        length--;
    }

    int32_t integerPart = centivalue / 100;
    // 3 decimal digits
    int32_t millis = (centivalue % 100) * 10;

    int digits = digitCount(integerPart);
    int remaining = length - digits;
    if (explicitDecimal) {
        remaining--;
    }

    if (remaining < 0 && scale > 0) {
        // Reduce by scale
        scaled = true;
        decimals = maxScaledDecimals;
        integerPart = integerPart / scale;
        // Multiply by 10 to get 3 decimal digits
        millis = ((centivalue % (100 * scale)) * 10) / scale;
        digits = digitCount(integerPart);
        remaining = length - digits;
        if (explicitDecimal) {
            remaining--;
        }
    }

    // 3 decimals at most
    decimals = MIN(remaining, MIN(decimals, 3));
    remaining -= decimals;

    // Done counting. Time to write the characters.
    // Write spaces at the start
    while (remaining > 0) {
        *ptr = SYM_BLANK;
        ptr++;
        remaining--;
    }

    // Keep number right aligned and correct length
    if(explicitDecimal && decimals == 0) {
        uint8_t blank_spaces = ptr - buff;
        int8_t rem_spaces = length - (digits + blank_spaces);
        // Add any needed remaining leading spaces
        while(rem_spaces > 0)
        {
            *ptr = SYM_BLANK;
            ptr++;
            remaining--;
            rem_spaces--;
        }
    }

    // Write the minus sign if required
    if (negative) {
        *ptr = '-';
        ptr++;
    }
    // Now write the digits.
    ui2a(integerPart, 10, 0, ptr);
    ptr += digits;

    if (decimals > 0) {
        if (explicitDecimal) {
            *ptr = '.';
            ptr++;
        } else {
            *(ptr - 1) += SYM_ZERO_HALF_TRAILING_DOT - '0';
        }
        dec = ptr;
        int factor = 3; // we're getting the decimal part in millis first
        while (decimals < factor) {
            factor--;
            millis /= 10;
        }
        int decimalDigits = digitCount(millis);
        while (decimalDigits < decimals) {
            decimalDigits++;
            *ptr = '0';
            ptr++;
        }
        ui2a(millis, 10, 0, ptr);
        if (!explicitDecimal) {
            *dec += SYM_ZERO_HALF_LEADING_DOT - '0';
        }
    }
    return scaled;
}
#endif
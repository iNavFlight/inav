/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdint.h>

#include "streambuf.h"

sbuf_t *sbufInit(sbuf_t *sbuf, uint8_t *ptr, uint8_t *end)
{
    sbuf->ptr = ptr;
    sbuf->end = end;
    return sbuf;
}

void sbufWriteU8(sbuf_t *dst, uint8_t val)
{
    *dst->ptr++ = val;
}

void sbufWriteU16(sbuf_t *dst, uint16_t val)
{
    sbufWriteU8(dst, val >> 0);
    sbufWriteU8(dst, val >> 8);
}

void sbufWriteU32(sbuf_t *dst, uint32_t val)
{
    sbufWriteU8(dst, val >> 0);
    sbufWriteU8(dst, val >> 8);
    sbufWriteU8(dst, val >> 16);
    sbufWriteU8(dst, val >> 24);
}

void sbufWriteU16BigEndian(sbuf_t *dst, uint16_t val)
{
    sbufWriteU8(dst, val >> 8);
    sbufWriteU8(dst, val >> 0);
}

void sbufWriteU32BigEndian(sbuf_t *dst, uint32_t val)
{
    sbufWriteU8(dst, val >> 24);
    sbufWriteU8(dst, val >> 16);
    sbufWriteU8(dst, val >> 8);
    sbufWriteU8(dst, val >> 0);
}

void sbufFill(sbuf_t *dst, uint8_t data, int len)
{
    memset(dst->ptr, data, len);
    dst->ptr += len;
}

void sbufWriteData(sbuf_t *dst, const void *data, int len)
{
    memcpy(dst->ptr, data, len);
    dst->ptr += len;
}

bool sbufWriteDataSafe(sbuf_t *dst, const void *data, int len)
{
    // only write if data does not overflow buffer
    if (sbufBytesRemaining(dst) >= len) {
        memcpy(dst->ptr, data, len);
        dst->ptr += len;
        return true;
    }
    return false;
}

void sbufWriteString(sbuf_t *dst, const char *string)
{
    sbufWriteData(dst, string, strlen(string));
}

void sbufWriteStringWithZeroTerminator(sbuf_t *dst, const char *string)
{
    sbufWriteData(dst, string, strlen(string) + 1);
}

uint8_t sbufReadU8(sbuf_t *src)
{
    return *src->ptr++;
}

uint16_t sbufReadU16(sbuf_t *src)
{
    uint16_t ret;
    ret = sbufReadU8(src);
    ret |= sbufReadU8(src) << 8;
    return ret;
}

uint32_t sbufReadU32(sbuf_t *src)
{
    uint32_t ret;
    ret = sbufReadU8(src);
    ret |= sbufReadU8(src) <<  8;
    ret |= sbufReadU8(src) << 16;
    ret |= sbufReadU8(src) << 24;
    return ret;
}

void sbufReadData(const sbuf_t *src, void *data, int len)
{
    memcpy(data, src->ptr, len);
}

bool sbufReadU8Safe(uint8_t *dst, sbuf_t *src)
{
    if (sbufBytesRemaining(src)) {
        const uint8_t value = sbufReadU8(src);
        if (dst) {
            *dst = value;
        }
        return true;
    }
    return false;
}

bool sbufReadI8Safe(int8_t *dst, sbuf_t *src)
{
    return sbufReadU8Safe((uint8_t*)dst, src);
}

bool sbufReadU16Safe(uint16_t *dst, sbuf_t *src)
{
    if (sbufBytesRemaining(src) >= 2) { // check there are enough bytes left in the buffer to read a uint16_t
        const uint16_t value = sbufReadU16(src);
        if (dst) {
            *dst = value;
        }
        return true;
    }
    return false;
}

bool sbufReadI16Safe(int16_t *dst, sbuf_t *src)
{
    return sbufReadU16Safe((uint16_t*)dst, src);
}

bool sbufReadU32Safe(uint32_t *dst, sbuf_t *src)
{
    if (sbufBytesRemaining(src) >= 4) { // check there are enough bytes left in the buffer to read a uint32_t
        const uint32_t value = sbufReadU32(src);
        if (dst) {
            *dst = value;
        }
        return true;
    }
    return false;
}

bool sbufReadI32Safe(int32_t *dst, sbuf_t *src)
{
    return sbufReadU32Safe((uint32_t*)dst, src);
}

bool sbufReadDataSafe(const sbuf_t *src, void *data, int len)
{
    if (sbufBytesRemaining(src) >= len) {
        sbufReadData(src, data, len);
        return true;
    }
    return false;
}

// reader - return bytes remaining in buffer
// writer - return available space
int sbufBytesRemaining(const sbuf_t *buf)
{
    return buf->end - buf->ptr;
}

uint8_t* sbufPtr(sbuf_t *buf)
{
    return buf->ptr;
}

const uint8_t* sbufConstPtr(const sbuf_t *buf)
{
    return buf->ptr;
}

// advance buffer pointer
// reader - skip data
// writer - commit written data
void sbufAdvance(sbuf_t *buf, int size)
{
    buf->ptr += size;
}

// modifies streambuf so that written data are prepared for reading
void sbufSwitchToReader(sbuf_t *buf, uint8_t *base)
{
    buf->end = buf->ptr;
    buf->ptr = base;
}

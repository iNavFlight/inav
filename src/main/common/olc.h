#pragma once

#include <stddef.h>
#include <stdint.h>

#define OLC_DEG_MULTIPLIER ((olc_coord_t)10000000LL) // 1e7

typedef int32_t olc_coord_t;
typedef uint32_t uolc_coord_t;

// olc_encodes the given coordinates in lat and lon (deg * OLC_DEG_MULTIPLIER)
// as an OLC code of the given length. It returns the number of characters
// written to buf.
int olc_encode(olc_coord_t lat, olc_coord_t lon, size_t length, char *buf, size_t bufsize);
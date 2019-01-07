#include <math.h>

#include "common/maths.h"
#include "common/olc.h"

// This is a port of https://github.com/google/open-location-code/blob/master/c/olc.c
// to avoid double floating point math and use integer math as much as possible.

#define SEPARATOR_CHAR '+'
#define SEPARATOR_POS 8
#define PADDING_CHAR '0'

#define ENCODING_BASE 20
#define PAIR_CODE_LEN 10u
#define CODE_LEN_MAX 15u

#define GRID_COLS 4
#define GRID_ROWS (ENCODING_BASE / GRID_COLS)


#define LAT_MAX (90 * OLC_DEG_MULTIPLIER)
#define LON_MAX (180 * OLC_DEG_MULTIPLIER)

static const char alphabet[] = "23456789CFGHJMPQRVWX";

// Initialized via init_constants()
static olc_coord_t grid_size;
static olc_coord_t initial_resolution;

static void init_constants(void)
{
    static int inited = 0;
    if (inited) {
        return;
    }
    inited = 1;

    // Work out the encoding base exponent necessary to represent 360 degrees.
    olc_coord_t initial_exponent = floorf(logf(2 * (LON_MAX / OLC_DEG_MULTIPLIER)) / logf(ENCODING_BASE));

    // Work out the enclosing resolution (in degrees) for the grid algorithm.
    grid_size = (1 / powf(ENCODING_BASE, PAIR_CODE_LEN / 2 - (initial_exponent + 1))) * OLC_DEG_MULTIPLIER;

    // Work out the initial resolution
    initial_resolution = powf(ENCODING_BASE, initial_exponent) * OLC_DEG_MULTIPLIER;
}

// Raises a number to an exponent, handling negative exponents.
static float pow_negf(float base, float exponent)
{
    if (exponent == 0) {
        return 1;
    }

    if (exponent > 0) {
        return powf(base, exponent);
    }

    return 1 / powf(base, -exponent);
}

// Compute the latitude precision value for a given code length.  Lengths <= 10
// have the same precision for latitude and longitude, but lengths > 10 have
// different precisions due to the grid method having fewer columns than rows.
static float compute_precision_for_length(int length)
{
    // Magic numbers!
    if (length <= (int)PAIR_CODE_LEN) {
        return pow_negf(ENCODING_BASE, floorf((length / -2) + 2));
    }

    return pow_negf(ENCODING_BASE, -3) / powf(5, length - (int)PAIR_CODE_LEN);
}

static olc_coord_t adjust_latitude(olc_coord_t lat, size_t code_len)
{
    if (lat < -LAT_MAX) {
        lat = -LAT_MAX;
    }
    if (lat > LAT_MAX) {
        lat = LAT_MAX;
    }
    if (lat >= LAT_MAX) {
        // Subtract half the code precision to get the latitude into the code area.
        olc_coord_t precision = compute_precision_for_length(code_len) * OLC_DEG_MULTIPLIER;
        lat -= precision / 2;
    }
    return lat;
}

static olc_coord_t normalize_longitude(olc_coord_t lon)
{
    while (lon < -LON_MAX) {
        lon += LON_MAX;
        lon += LON_MAX;
    }
    while (lon >= LON_MAX) {
        lon -= LON_MAX;
        lon -= LON_MAX;
    }
    return lon;
}

// Encodes positive range lat,lon into a sequence of OLC lat/lon pairs.  This
// uses pairs of characters (latitude and longitude in that order) to represent
// each step in a 20x20 grid.  Each code, therefore, has 1/400th the area of
// the previous code.
static unsigned encode_pairs(uolc_coord_t lat, uolc_coord_t lon, size_t length, char *buf, size_t bufsize)
{
    if ((length + 1) >= bufsize) {
        buf[0] = '\0';
        return 0;
    }

    unsigned pos = 0;
    olc_coord_t resolution = initial_resolution;
    // Add two digits on each pass.
    for (size_t digit_count = 0;
         digit_count < length;
         digit_count += 2, resolution /= ENCODING_BASE) {
        size_t digit_value;

        // Do the latitude - gets the digit for this place and subtracts that
        // for the next digit.
        digit_value = lat / resolution;
        lat -= digit_value * resolution;
        buf[pos++] = alphabet[digit_value];

        // Do the longitude - gets the digit for this place and subtracts that
        // for the next digit.
        digit_value = lon / resolution;
        lon -= digit_value * resolution;
        buf[pos++] = alphabet[digit_value];

        // Should we add a separator here?
        if (pos == SEPARATOR_POS && pos < length) {
            buf[pos++] = SEPARATOR_CHAR;
        }
    }
    while (pos < SEPARATOR_POS) {
        buf[pos++] = PADDING_CHAR;
    }
    if (pos == SEPARATOR_POS) {
        buf[pos++] = SEPARATOR_CHAR;
    }
    buf[pos] = '\0';
    return pos;
}

// Encodes a location using the grid refinement method into an OLC string.  The
// grid refinement method divides the area into a grid of 4x5, and uses a
// single character to refine the area.  The grid squares use the OLC
// characters in order to number the squares as follows:
//
//   R V W X
//   J M P Q
//   C F G H
//   6 7 8 9
//   2 3 4 5
//
// This allows default accuracy OLC codes to be refined with just a single
// character.
static int encode_grid(uolc_coord_t lat, uolc_coord_t lon, size_t length,
                       char *buf, size_t bufsize)
{
    if ((length + 1) >= bufsize) {
        buf[0] = '\0';
        return 0;
    }

    int pos = 0;

    olc_coord_t lat_grid_size = grid_size;
    olc_coord_t lon_grid_size = grid_size;

    lat %= lat_grid_size;
    lon %= lon_grid_size;

    for (size_t i = 0; i < length; i++) {
        olc_coord_t lat_div = lat_grid_size / GRID_ROWS;
        olc_coord_t lon_div = lon_grid_size / GRID_COLS;

        if (lat_div == 0 || lon_div == 0) {
            // This case happens when OLC_DEG_MULTIPLIER doesn't have enough
            // precision for the requested length.
            break;
        }

        // Work out the row and column.
        size_t row = lat / lat_div;
        size_t col = lon / lon_div;
        lat_grid_size /= GRID_ROWS;
        lon_grid_size /= GRID_COLS;
        lat -= row * lat_grid_size;
        lon -= col * lon_grid_size;
        buf[pos++] = alphabet[row * GRID_COLS + col];
    }
    buf[pos] = '\0';
    return pos;
}

int olc_encode(olc_coord_t lat, olc_coord_t lon, size_t length, char *buf, size_t bufsize)
{
    int pos = 0;

    length = MIN(length, CODE_LEN_MAX);

    // Adjust latitude and longitude so they fall into positive ranges.
    uolc_coord_t alat = adjust_latitude(lat, length) + LAT_MAX;
    uolc_coord_t alon = normalize_longitude(lon) + LON_MAX;

    init_constants();

    pos += encode_pairs(alat, alon, MIN(length, PAIR_CODE_LEN), buf + pos, bufsize - pos);
    // If the requested length indicates we want grid refined codes.
    if (length > PAIR_CODE_LEN) {
        pos += encode_grid(alat, alon, length - PAIR_CODE_LEN, buf + pos, bufsize - pos);
    }
    buf[pos] = '\0';
    return pos;
}
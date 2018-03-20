/**
 ******************************************************************************
 * @addtogroup Tau Labs Modules
 * @{
 * @addtogroup OnScreenDisplay OSD Module
 * @brief OSD Utility Functions
 * @{
 *
 * @file       osd_utils.h
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2013-2015
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2014.
 * @brief      OSD Utility Functions
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef OSDUTILS_H
#define OSDUTILS_H

#include "fonts.h"
#include "images.h"
#include "video.h"

// Size of an array (num items.)
#define SIZEOF_ARRAY(x) (sizeof(x) / sizeof((x)[0]))

#define HUD_VSCALE_FLAG_CLEAR       1
#define HUD_VSCALE_FLAG_NO_NEGATIVE 2

#if defined(VIDEO_SPLITBUFFER)
#define PIXELS_PER_BIT 8
#define CALC_BIT_IN_WORD(x) ((x) & 7)
#define CALC_BIT_MASK(x) (1 << (7 - ((x) & 7)))
// Horizontal line calculations.
// Edge cases.
#define COMPUTE_HLINE_EDGE_L_MASK(b)      ((1 << (8 - (b))) - 1)
#define COMPUTE_HLINE_EDGE_R_MASK(b)      (~((1 << (7 - (b))) - 1))
#else
#if VIDEO_BITS_PER_PIXEL != 2
#error "Only 2 bits / pixel is currently supported"
#endif
#define PIXELS_PER_BIT (8 / VIDEO_BITS_PER_PIXEL)
#define CALC_BIT_IN_WORD(x) (2 * ((x) & 3))
#define CALC_BITSHIFT_WORD(x) (2 * ((x) & 3))
#define CALC_BIT_MASK(x) (3 << (6 - CALC_BITSHIFT_WORD(x)))
#define PACK_BITS(mask, level) (level << 7 | mask << 6 | level << 5 | mask << 4 | level << 3 | mask << 2 | level << 1 | mask)
#define CALC_BIT0_IN_WORD(x)  (2 * ((x) & 3))
#define CALC_BIT1_IN_WORD(x)  (2 * ((x) & 3) + 1)
// Horizontal line calculations.
// Edge cases.
#define COMPUTE_HLINE_EDGE_L_MASK(b)      ((1 << (7 - (b))) - 1)
#define COMPUTE_HLINE_EDGE_R_MASK(b)      (~((1 << (6 - (b))) - 1))
#endif /* defined(VIDEO_SPLITBUFFER) */

// Macros for computing addresses and bit positions.
#define CALC_BUFF_ADDR(x, y) (((x) / PIXELS_PER_BIT) + ((y) * BUFFER_WIDTH))
#define DEBUG_DELAY
// Macro for writing a word with a mode (NAND = clear, OR = set, XOR = toggle)
// at a given position
#define WRITE_WORD_MODE(buff, addr, mask, mode) \
	switch (mode) { \
	case 0: buff[addr] &= ~mask; break; \
	case 1: buff[addr] |= mask; break; \
	case 2: buff[addr] ^= mask; break; }

#define WRITE_WORD_NAND(buff, addr, mask) { buff[addr] &= ~mask; DEBUG_DELAY; }
#define WRITE_WORD_OR(buff, addr, mask)   { buff[addr] |= mask; DEBUG_DELAY; }
#define WRITE_WORD_XOR(buff, addr, mask)  { buff[addr] ^= mask; DEBUG_DELAY; }
#define WRITE_WORD(buff, addr, mask, value)  { buff[addr] = (buff[addr] & ~mask) | (value & mask);}


// This computes an island mask.
#define COMPUTE_HLINE_ISLAND_MASK(b0, b1) (COMPUTE_HLINE_EDGE_L_MASK(b0) ^ COMPUTE_HLINE_EDGE_L_MASK(b1));

// Macro for initializing stroke/fill modes. Add new modes here
// if necessary.
#define SETUP_STROKE_FILL(stroke, fill, mode) \
	stroke = 0; fill = 0; \
	if (mode == 0) { stroke = 0; fill = 1; } \
	if (mode == 1) { stroke = 1; fill = 0; } \

// Line endcaps (for horizontal and vertical lines.)
#define ENDCAP_NONE  0
#define ENDCAP_ROUND 1
#define ENDCAP_FLAT  2

#define DRAW_ENDCAP_HLINE(e, x, y, s, f, l) \
	if ((e) == ENDCAP_ROUND) /* single pixel endcap */ \
{ write_pixel_lm(x, y, f, l); } \
	else if ((e) == ENDCAP_FLAT) /* flat endcap: FIXME, quicker to draw a vertical line(?) */ \
{ write_pixel_lm(x, y - 1, s, l); write_pixel_lm(x, y, s, l); write_pixel_lm(x, y + 1, s, l); }

#define DRAW_ENDCAP_VLINE(e, x, y, s, f, l) \
	if ((e) == ENDCAP_ROUND) /* single pixel endcap */ \
{ write_pixel_lm(x, y, f, l); } \
	else if ((e) == ENDCAP_FLAT) /* flat endcap: FIXME, quicker to draw a horizontal line(?) */ \
{ write_pixel_lm(x - 1, y, s, l); write_pixel_lm(x, y, s, l); write_pixel_lm(x + 1, y, s, l); }

// Macros for writing pixels in a midpoint circle algorithm.
#define CIRCLE_PLOT_8(buff, cx, cy, x, y, mode) \
	CIRCLE_PLOT_4(buff, cx, cy, x, y, mode); \
	if ((x) != (y)) { CIRCLE_PLOT_4(buff, cx, cy, y, x, mode); }

#define CIRCLE_PLOT_4(buff, cx, cy, x, y, mode) \
	write_pixel(buff, (cx) + (x), (cy) + (y), mode); \
	write_pixel(buff, (cx) - (x), (cy) + (y), mode); \
	write_pixel(buff, (cx) + (x), (cy) - (y), mode); \
	write_pixel(buff, (cx) - (x), (cy) - (y), mode);

// Font flags.
#define FONT_BOLD      1               // bold text (no outline)
#define FONT_INVERT    2               // invert: border white, inside black
// Text alignments.
#define TEXT_VA_TOP    0
#define TEXT_VA_MIDDLE 1
#define TEXT_VA_BOTTOM 2
#define TEXT_HA_LEFT   0
#define TEXT_HA_CENTER 1
#define TEXT_HA_RIGHT  2

// Max/Min macros
#define MAX3(a, b, c)                MAX(a, MAX(b, c))
#define MIN3(a, b, c)                MIN(a, MIN(b, c))
#define LIMIT(x, l, h)               MAX(l, MIN(x, h))

// Check if coordinates are valid. If not, return. Assumes signed coordinates for working correct also with values lesser than 0.
#define CHECK_COORDS(x, y)           if (x < GRAPHICS_LEFT || x > GRAPHICS_RIGHT || y < GRAPHICS_TOP || y > GRAPHICS_BOTTOM) { return; }
#define CHECK_COORD_X(x)             if (x < GRAPHICS_LEFT || x > GRAPHICS_RIGHT) { return; }
#define CHECK_COORD_Y(y)             if (y < GRAPHICS_TOP  || y > GRAPHICS_BOTTOM) { return; }

// Clip coordinates out of range. Assumes signed coordinates for working correct also with values lesser than 0.
#define CLIP_COORDS(x, y)            { CLIP_COORD_X(x); CLIP_COORD_Y(y); }
#define CLIP_COORD_X(x)              { x = x < GRAPHICS_LEFT ? GRAPHICS_LEFT : x > GRAPHICS_RIGHT ? GRAPHICS_RIGHT : x; }
#define CLIP_COORD_Y(y)              { y = y < GRAPHICS_TOP ? GRAPHICS_TOP : y > GRAPHICS_BOTTOM ? GRAPHICS_BOTTOM : y; }

// Macro to swap two variables using XOR swap.
#define SWAP(a, b)                   { a ^= b; b ^= a; a ^= b; }


// Text dimension structures.
struct FontDimensions {
	int width, height;
};

// Structure for a point
typedef struct {
	int16_t x;
	int16_t y;
} point_t;

void clearGraphics();
void draw_image(uint16_t x, uint16_t y, const struct Image * image);
void plotFourQuadrants(int32_t centerX, int32_t centerY, int32_t deltaX, int32_t deltaY);
void ellipse(int centerX, int centerY, int horizontalRadius, int verticalRadius);
void drawArrow(uint16_t x, uint16_t y, uint16_t angle, uint16_t size_quarter);
void drawBox(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void write_pixel_lm(int x, int y, int mmode, int lmode);
void write_hline_lm(int x0, int x1, int y, int lmode, int mmode);
void write_hline_outlined(int x0, int x1, int y, int endcap0, int endcap1, int mode, int mmode);
void write_vline_lm(int x, int y0, int y1, int lmode, int mmode);
void write_vline_outlined(int x, int y0, int y1, int endcap0, int endcap1, int mode, int mmode);
void write_filled_rectangle_lm(int x, int y, int width, int height, int lmode, int mmode);
void write_rectangle_outlined(int x, int y, int width, int height, int mode, int mmode);
void write_circle_outlined(int cx, int cy, int r, int dashp, int bmode, int mode, int mmode);

void write_line_lm(int x0, int y0, int x1, int y1, int mmode, int lmode);
void write_line_outlined(int x0, int y0, int x1, int y1,
						 __attribute__((unused)) int endcap0, __attribute__((unused)) int endcap1,
						 int mode, int mmode);
void write_line_outlined_dashed(int x0, int y0, int x1, int y1,
								__attribute__((unused)) int endcap0, __attribute__((unused)) int endcap1,
								int mode, int mmode, int dots);
const struct FontEntry* get_font_info(int font);
void calc_text_dimensions(const char *str, const struct FontEntry *font, int xs, int ys, struct FontDimensions *dim);
void write_string(const char *str, int x, int y, int xs, int ys, int va, int ha, int font);
void draw_polygon(int16_t x, int16_t y, float angle, const point_t * points, uint8_t n_points, int mode, int mmode);

void osd_draw_vertical_scale(int v, int range, int halign, int x, int y, int height, int mintick_step, int majtick_step, int mintick_len,
                             int majtick_len, int boundtick_len, int flags);
#endif /* OSDUTILS_H */



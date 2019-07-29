/**
 ******************************************************************************
 * @addtogroup Tau Labs Modules
 * @{
 * @addtogroup OnScreenDisplay OSD Module
 * @brief OSD Utility Functions
 * @{
 *
 * @file       osd_utils.c
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
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
 *
 * Additional note on redistribution: The copyright and license notices above
 * must be maintained in each individual source file that is a derivative work
 * of this source file; otherwise redistribution is prohibited.
 */

#include <string.h>
#include <math.h>
#include <stdlib.h>


#include "video.h"
#include "fonts.h"
#include "osd_utils.h"
#include "common/printf.h"

extern struct FontEntry* fonts[NUM_FONTS];

#if defined(VIDEO_SPLITBUFFER)
extern uint8_t *draw_buffer_level;
extern uint8_t *draw_buffer_mask;
extern uint8_t *disp_buffer_level;
extern uint8_t *disp_buffer_mask;
#else
extern uint8_t *draw_buffer;
extern uint8_t *disp_buffer;
#endif /* defined(VIDEO_SPLITBUFFER) */


void clearGraphics(void)
{
#if defined(VIDEO_SPLITBUFFER)
	memset((uint8_t *)draw_buffer_mask, 0, BUFFER_HEIGHT * BUFFER_WIDTH);
	memset((uint8_t *)draw_buffer_level, 0, BUFFER_HEIGHT * BUFFER_WIDTH);
#else
	memset((uint8_t *)draw_buffer, 0, BUFFER_HEIGHT * BUFFER_WIDTH);
#endif /* defined(VIDEO_SPLITBUFFER) */
}

void draw_image(uint16_t x, uint16_t y, const struct Image * image)
{
	CHECK_COORDS(x + image->width, y);
	uint8_t byte_width = image->width / 4;
	uint8_t pixel_offset = 2 * (x % 4);
	uint8_t mask1 = 0xFF;
	uint8_t mask2 = 0x00;

	if (pixel_offset > 0) {
		for (uint8_t i = 0; i<pixel_offset; i++) {
			mask2 |= 0x01 << i;
		}
		mask1 = ~mask2;
	}

	for (uint16_t yp = 0; yp < image->height; yp++) {
		if ((y + yp) >  GRAPHICS_BOTTOM)
			break;
		for (uint16_t xp = 0; xp < byte_width; xp++) {
			draw_buffer[(y + yp) * BUFFER_WIDTH + xp + x / 4] |= (image->data[yp * byte_width + xp] & mask1) >> pixel_offset;
			if (pixel_offset > 0) {
				draw_buffer[(y + yp) * BUFFER_WIDTH + xp + x / 4 + 1] |= (image->data[yp * byte_width + xp] & mask2) <<
						(8 - pixel_offset);
			}
		}
	}
}

/// Draws four points relative to the given center point.
///
/// \li centerX + X, centerY + Y
/// \li centerX + X, centerY - Y
/// \li centerX - X, centerY + Y
/// \li centerX - X, centerY - Y
///
/// \param centerX the x coordinate of the center point
/// \param centerY the y coordinate of the center point
/// \param deltaX the difference between the centerX coordinate and each pixel drawn
/// \param deltaY the difference between the centerY coordinate and each pixel drawn
/// \param color the color to draw the pixels with.
void plotFourQuadrants(int32_t centerX, int32_t centerY, int32_t deltaX, int32_t deltaY)
{
	write_pixel_lm(centerX + deltaX, centerY + deltaY, 1, 1); // Ist      Quadrant
	write_pixel_lm(centerX - deltaX, centerY + deltaY, 1, 1); // IInd     Quadrant
	write_pixel_lm(centerX - deltaX, centerY - deltaY, 1, 1); // IIIrd    Quadrant
	write_pixel_lm(centerX + deltaX, centerY - deltaY, 1, 1); // IVth     Quadrant
}

/// Implements the midpoint ellipse drawing algorithm which is a bresenham
/// style DDF.
///
/// \param centerX the x coordinate of the center of the ellipse
/// \param centerY the y coordinate of the center of the ellipse
/// \param horizontalRadius the horizontal radius of the ellipse
/// \param verticalRadius the vertical radius of the ellipse
/// \param color the color of the ellipse border
void ellipse(int centerX, int centerY, int horizontalRadius, int verticalRadius)
{
	int64_t doubleHorizontalRadius = horizontalRadius * horizontalRadius;
	int64_t doubleVerticalRadius   = verticalRadius * verticalRadius;

	int64_t error = doubleVerticalRadius - doubleHorizontalRadius * verticalRadius + (doubleVerticalRadius >> 2);

	int x = 0;
	int y = verticalRadius;
	int deltaX = 0;
	int deltaY = (doubleHorizontalRadius << 1) * y;

	plotFourQuadrants(centerX, centerY, x, y);

	while (deltaY >= deltaX) {
		x++;
		deltaX += (doubleVerticalRadius << 1);

		error  += deltaX + doubleVerticalRadius;

		if (error >= 0) {
			y--;
			deltaY -= (doubleHorizontalRadius << 1);

			error  -= deltaY;
		}
		plotFourQuadrants(centerX, centerY, x, y);
	}

	error = (int64_t)(doubleVerticalRadius * (x + 1 / 2.0f) * (x + 1 / 2.0f) + doubleHorizontalRadius * (y - 1) * (y - 1) - doubleHorizontalRadius * doubleVerticalRadius);

	while (y >= 0) {
		error  += doubleHorizontalRadius;
		y--;
		deltaY -= (doubleHorizontalRadius << 1);
		error  -= deltaY;

		if (error <= 0) {
			x++;
			deltaX += (doubleVerticalRadius << 1);
			error  += deltaX;
		}

		plotFourQuadrants(centerX, centerY, x, y);
	}
}

void drawArrow(uint16_t x, uint16_t y, uint16_t angle, uint16_t size_quarter)
{
	float sin_angle = sinf(angle * (float)(M_PI / 180));
	float cos_angle = cosf(angle * (float)(M_PI / 180));
	int16_t peak_x  = (int16_t)(sin_angle * size_quarter * 2);
	int16_t peak_y  = (int16_t)(cos_angle * size_quarter * 2);
	int16_t d_end_x = (int16_t)(cos_angle * size_quarter);
	int16_t d_end_y = (int16_t)(sin_angle * size_quarter);

	write_line_lm(x + peak_x, y - peak_y, x - peak_x - d_end_x, y + peak_y - d_end_y, 1, 1);
	write_line_lm(x + peak_x, y - peak_y, x - peak_x + d_end_x, y + peak_y + d_end_y, 1, 1);
	write_line_lm(x, y, x - peak_x - d_end_x, y + peak_y - d_end_y, 1, 1);
	write_line_lm(x, y, x - peak_x + d_end_x, y + peak_y + d_end_y, 1, 1);
}

void drawBox(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	write_line_lm(x1, y1, x2, y1, 1, 1); // top
	write_line_lm(x1, y1, x1, y2, 1, 1); // left
	write_line_lm(x2, y1, x2, y2, 1, 1); // right
	write_line_lm(x1, y2, x2, y2, 1, 1); // bottom
}

#if defined(VIDEO_SPLITBUFFER)
/**
 * write_pixel: Write a pixel at an x,y position to a given surface.
 *
 * @param       buff    pointer to buffer to write in
 * @param       x               x coordinate
 * @param       y               y coordinate
 * @param       mode    0 = clear bit, 1 = set bit, 2 = toggle bit
 */
void write_pixel(uint8_t *buff, int x, int y, int mode)
{
	CHECK_COORDS(x, y);
	// Determine the bit in the word to be set and the word
	// index to set it in.
	int wordnum = CALC_BUFF_ADDR(x, y);
	uint8_t mask = CALC_BIT_MASK(x);
	WRITE_WORD_MODE(buff, wordnum, mask, mode);
}
#else
void write_pixel(int x, int y, uint8_t value)
{
	CHECK_COORDS(x, y);
	// Determine the bit in the word to be set and the word
	// index to set it in.
	int wordnum = CALC_BUFF_ADDR(x, y);
	uint8_t mask = CALC_BIT_MASK(x);
	WRITE_WORD(draw_buffer, wordnum, mask, value);
}
#endif /* VIDEO_SPLITBUFFER */

/**
 * write_pixel_lm: write the pixel on both surfaces (level and mask.)
 * Uses current draw buffer.
 *
 * @param       x               x coordinate
 * @param       y               y coordinate
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 * @param       lmode   0 = black, 1 = white, 2 = toggle
 */
void write_pixel_lm(int x, int y, int mmode, int lmode)
{
	CHECK_COORDS(x, y);
	// Determine the bit in the word to be set and the word
	// index to set it in.
	int addr   = CALC_BUFF_ADDR(x, y);
	uint8_t mask = CALC_BIT_MASK(x);
#if defined(VIDEO_SPLITBUFFER)
	WRITE_WORD_MODE(draw_buffer_mask, addr, mask, mmode);
	WRITE_WORD_MODE(draw_buffer_level, addr, mask, lmode);
#else
	uint8_t value = PACK_BITS(mmode, lmode);
	WRITE_WORD(draw_buffer, addr, mask, value);
#endif /* defined(VIDEO_SPLITBUFFER) */
}

/**
 * write_hline: optimised horizontal line writing algorithm
 *
 * @param       buff    pointer to buffer to write in
 * @param       x0      x0 coordinate
 * @param       x1      x1 coordinate
 * @param       y       y coordinate
 * @param       mode    0 = clear, 1 = set, 2 = toggle
 */
#if defined(VIDEO_SPLITBUFFER)
void write_hline(uint8_t *buff, int x0, int x1, int y, int mode)
{
	CHECK_COORD_Y(y);
	CLIP_COORD_X(x0);
	CLIP_COORD_X(x1);
	if (x0 > x1) {
		SWAP(x0, x1);
	}
	if (x0 == x1) {
		return;
	}
	/* This is an optimised algorithm for writing horizontal lines.
	 * We begin by finding the addresses of the x0 and x1 points. */
	int addr0     = CALC_BUFF_ADDR(x0, y);
	int addr1     = CALC_BUFF_ADDR(x1, y);
	int addr0_bit = CALC_BIT_IN_WORD(x0);
	int addr1_bit = CALC_BIT_IN_WORD(x1);
	int mask, mask_l, mask_r, i;
	/* If the addresses are equal, we only need to write one word
	 * which is an island. */
	if (addr0 == addr1) {
		mask = COMPUTE_HLINE_ISLAND_MASK(addr0_bit, addr1_bit);
		WRITE_WORD_MODE(buff, addr0, mask, mode);
	} else {
		/* Otherwise we need to write the edges and then the middle. */
		mask_l = COMPUTE_HLINE_EDGE_L_MASK(addr0_bit);
		mask_r = COMPUTE_HLINE_EDGE_R_MASK(addr1_bit);
		WRITE_WORD_MODE(buff, addr0, mask_l, mode);
		WRITE_WORD_MODE(buff, addr1, mask_r, mode);
		// Now write 0xffff words from start+1 to end-1.
		for (i = addr0 + 1; i <= addr1 - 1; i++) {
			uint8_t m = 0xff;
			WRITE_WORD_MODE(buff, i, m, mode);
		}
	}
}
#else
void write_hline(int x0, int x1, int y, uint8_t value)
{
	CHECK_COORD_Y(y);
	CLIP_COORD_X(x0);
	CLIP_COORD_X(x1);
	if (x0 > x1) {
		SWAP(x0, x1);
	}
	if (x0 == x1) {
		return;
	}
	/* This is an optimised algorithm for writing horizontal lines.
	 * We begin by finding the addresses of the x0 and x1 points. */
	int addr0     = CALC_BUFF_ADDR(x0, y);
	int addr1     = CALC_BUFF_ADDR(x1, y);
	int addr0_bit = CALC_BIT1_IN_WORD(x0);
	int addr1_bit = CALC_BIT0_IN_WORD(x1);
	int mask, mask_l, mask_r, i;
	/* If the addresses are equal, we only need to write one word
	 * which is an island. */
	if (addr0 == addr1) {
		mask = COMPUTE_HLINE_ISLAND_MASK(addr0_bit, addr1_bit);
		WRITE_WORD(draw_buffer, addr0, mask, value);
	} else {
		/* Otherwise we need to write the edges and then the middle. */
		mask_l = COMPUTE_HLINE_EDGE_L_MASK(addr0_bit);
		mask_r = COMPUTE_HLINE_EDGE_R_MASK(addr1_bit);
		WRITE_WORD(draw_buffer, addr0, mask_l, value);
		WRITE_WORD(draw_buffer, addr1, mask_r, value);
		// Now write 0xffff words from start+1 to end-1.
		for (i = addr0 + 1; i <= addr1 - 1; i++) {
			uint8_t m = 0xff;
			WRITE_WORD(draw_buffer, i, m, value);
		}
	}
}
#endif /* defined(VIDEO_SPLITBUFFER) */


/**
 * write_hline_lm: write both level and mask buffers.
 *
 * @param       x0              x0 coordinate
 * @param       x1              x1 coordinate
 * @param       y               y coordinate
 * @param       lmode   0 = clear, 1 = set, 2 = toggle
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 */
void write_hline_lm(int x0, int x1, int y, int lmode, int mmode)
{
#if defined(VIDEO_SPLITBUFFER)
	// TODO: an optimisation would compute the masks and apply to
	// both buffers simultaneously.
	write_hline(draw_buffer_level, x0, x1, y, lmode);
	write_hline(draw_buffer_mask, x0, x1, y, mmode);
#else
	uint8_t value = PACK_BITS(mmode, lmode);
	write_hline(x0, x1, y, value);
#endif /* defined(VIDEO_SPLITBUFFER) */
}

/**
 * write_hline_outlined: outlined horizontal line with varying endcaps
 * Always uses draw buffer.
 *
 * @param       x0                      x0 coordinate
 * @param       x1                      x1 coordinate
 * @param       y                       y coordinate
 * @param       endcap0         0 = none, 1 = single pixel, 2 = full cap
 * @param       endcap1         0 = none, 1 = single pixel, 2 = full cap
 * @param       mode            0 = black outline, white body, 1 = white outline, black body
 * @param       mmode           0 = clear, 1 = set, 2 = toggle
 */
void write_hline_outlined(int x0, int x1, int y, int endcap0, int endcap1, int mode, int mmode)
{
	int stroke, fill;

	SETUP_STROKE_FILL(stroke, fill, mode);
	if (x0 > x1) {
		SWAP(x0, x1);
	}
	// Draw the main body of the line.
	write_hline_lm(x0 + 1, x1 - 1, y - 1, stroke, mmode);
	write_hline_lm(x0 + 1, x1 - 1, y + 1, stroke, mmode);
	write_hline_lm(x0 + 1, x1 - 1, y, fill, mmode);
	// Draw the endcaps, if any.
	DRAW_ENDCAP_HLINE(endcap0, x0, y, stroke, fill, mmode);
	DRAW_ENDCAP_HLINE(endcap1, x1, y, stroke, fill, mmode);
}

/**
 * write_vline: optimised vertical line writing algorithm
 *
 * @param       buff    pointer to buffer to write in
 * @param       x       x coordinate
 * @param       y0      y0 coordinate
 * @param       y1      y1 coordinate
 * @param       mode    0 = clear, 1 = set, 2 = toggle
 */
#if defined(VIDEO_SPLITBUFFER)
void write_vline(uint8_t *buff, int x, int y0, int y1, int mode)
{
	CHECK_COORD_X(x);
	CLIP_COORD_Y(y0);
	CLIP_COORD_Y(y1);
	if (y0 > y1) {
		SWAP(y0, y1);
	}
	if (y0 == y1) {
		return;
	}
	/* This is an optimised algorithm for writing vertical lines.
	 * We begin by finding the addresses of the x,y0 and x,y1 points. */
	int addr0  = CALC_BUFF_ADDR(x, y0);
	int addr1  = CALC_BUFF_ADDR(x, y1);
	/* Then we calculate the pixel data to be written. */
	uint8_t mask = CALC_BIT_MASK(x);
	/* Run from addr0 to addr1 placing pixels. Increment by the number
	 * of words n each graphics line. */
	for (int a = addr0; a <= addr1; a += BUFFER_WIDTH) {
		WRITE_WORD_MODE(buff, a, mask, mode);
	}
}
#else
void write_vline(int x, int y0, int y1, uint8_t value)
{
	CHECK_COORD_X(x);
	CLIP_COORD_Y(y0);
	CLIP_COORD_Y(y1);
	if (y0 > y1) {
		SWAP(y0, y1);
	}
	if (y0 == y1) {
		return;
	}
	/* This is an optimised algorithm for writing vertical lines.
	 * We begin by finding the addresses of the x,y0 and x,y1 points. */
	int addr0  = CALC_BUFF_ADDR(x, y0);
	int addr1  = CALC_BUFF_ADDR(x, y1);
	/* Then we calculate the pixel data to be written. */
	uint8_t mask = CALC_BIT_MASK(x);
	/* Run from addr0 to addr1 placing pixels. Increment by the number
	 * of words n each graphics line. */
	for (int a = addr0; a <= addr1; a += BUFFER_WIDTH) {
		WRITE_WORD(draw_buffer, a, mask, value);
	}
}
#endif /* defined(VIDEO_SPLITBUFFER) */


/**
 * write_vline_lm: write both level and mask buffers.
 *
 * @param       x               x coordinate
 * @param       y0              y0 coordinate
 * @param       y1              y1 coordinate
 * @param       lmode   0 = clear, 1 = set, 2 = toggle
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 */
void write_vline_lm(int x, int y0, int y1, int lmode, int mmode)
{
#if defined(VIDEO_SPLITBUFFER)
	// TODO: an optimisation would compute the masks and apply to
	// both buffers simultaneously.
	write_vline(draw_buffer_level, x, y0, y1, lmode);
	write_vline(draw_buffer_mask, x, y0, y1, mmode);
#else
	uint8_t value = PACK_BITS(mmode, lmode);
	write_vline(x, y0, y1, value);
#endif /* defined(VIDEO_SPLITBUFFER) */
}

/**
 * write_vline_outlined: outlined vertical line with varying endcaps
 * Always uses draw buffer.
 *
 * @param       x                       x coordinate
 * @param       y0                      y0 coordinate
 * @param       y1                      y1 coordinate
 * @param       endcap0         0 = none, 1 = single pixel, 2 = full cap
 * @param       endcap1         0 = none, 1 = single pixel, 2 = full cap
 * @param       mode            0 = black outline, white body, 1 = white outline, black body
 * @param       mmode           0 = clear, 1 = set, 2 = toggle
 */
void write_vline_outlined(int x, int y0, int y1, int endcap0, int endcap1, int mode, int mmode)
{
	int stroke, fill;

	if (y0 > y1) {
		SWAP(y0, y1);
	}
	SETUP_STROKE_FILL(stroke, fill, mode);
	// Draw the main body of the line.
	write_vline_lm(x - 1, y0 + 1, y1 - 1, stroke, mmode);
	write_vline_lm(x + 1, y0 + 1, y1 - 1, stroke, mmode);
	write_vline_lm(x, y0 + 1, y1 - 1, fill, mmode);
	// Draw the endcaps, if any.
	DRAW_ENDCAP_VLINE(endcap0, x, y0, stroke, fill, mmode);
	DRAW_ENDCAP_VLINE(endcap1, x, y1, stroke, fill, mmode);
}

/**
 * write_filled_rectangle: draw a filled rectangle.
 *
 * Uses an optimised algorithm which is similar to the horizontal
 * line writing algorithm, but optimised for writing the lines
 * multiple times without recalculating lots of stuff.
 *
 * @param       buff    pointer to buffer to write in
 * @param       x               x coordinate (left)
 * @param       y               y coordinate (top)
 * @param       width   rectangle width
 * @param       height  rectangle height
 * @param       mode    0 = clear, 1 = set, 2 = toggle
 */
#if defined(VIDEO_SPLITBUFFER)
void write_filled_rectangle(uint8_t *buff, int x, int y, int width, int height, int mode)
{
	int yy, addr0_old, addr1_old;

	CHECK_COORDS(x, y);
	CHECK_COORDS(x + width, y + height);
	if (width <= 0 || height <= 0) {
		return;
	}
	// Calculate as if the rectangle was only a horizontal line. We then
	// step these addresses through each row until we iterate `height` times.
	int addr0     = CALC_BUFF_ADDR(x, y);
	int addr1     = CALC_BUFF_ADDR(x + width, y);
	int addr0_bit = CALC_BIT_IN_WORD(x);
	int addr1_bit = CALC_BIT_IN_WORD(x + width);
	int mask, mask_l, mask_r, i;
	// If the addresses are equal, we need to write one word vertically.
	if (addr0 == addr1) {
		mask = COMPUTE_HLINE_ISLAND_MASK(addr0_bit, addr1_bit);
		while (height--) {
			WRITE_WORD_MODE(buff, addr0, mask, mode);
			addr0 += BUFFER_WIDTH;
		}
	} else {
		// Otherwise we need to write the edges and then the middle repeatedly.
		mask_l    = COMPUTE_HLINE_EDGE_L_MASK(addr0_bit);
		mask_r    = COMPUTE_HLINE_EDGE_R_MASK(addr1_bit);
		// Write edges first.
		yy        = 0;
		addr0_old = addr0;
		addr1_old = addr1;
		while (yy < height) {
			WRITE_WORD_MODE(buff, addr0, mask_l, mode);
			WRITE_WORD_MODE(buff, addr1, mask_r, mode);
			addr0 += BUFFER_WIDTH;
			addr1 += BUFFER_WIDTH;
			yy++;
		}
		// Now write 0xffff words from start+1 to end-1 for each row.
		yy    = 0;
		addr0 = addr0_old;
		addr1 = addr1_old;
		while (yy < height) {
			for (i = addr0 + 1; i <= addr1 - 1; i++) {
				uint8_t m = 0xff;
				WRITE_WORD_MODE(buff, i, m, mode);
			}
			addr0 += BUFFER_WIDTH;
			addr1 += BUFFER_WIDTH;
			yy++;
		}
	}
}
#else
void write_filled_rectangle(int x, int y, int width, int height, uint8_t value)
{
	int yy, addr0_old, addr1_old;

	CHECK_COORDS(x, y);
	CHECK_COORDS(x + width, y + height);
	if (width <= 0 || height <= 0) {
		return;
	}
	// Calculate as if the rectangle was only a horizontal line. We then
	// step these addresses through each row until we iterate `height` times.
	int addr0     = CALC_BUFF_ADDR(x, y);
	int addr1     = CALC_BUFF_ADDR(x + width, y);
    int addr0_bit = CALC_BIT1_IN_WORD(x);
    int addr1_bit = CALC_BIT0_IN_WORD(x + width);
	int mask, mask_l, mask_r, i;
	// If the addresses are equal, we need to write one word vertically.
	if (addr0 == addr1) {
		mask = COMPUTE_HLINE_ISLAND_MASK(addr0_bit, addr1_bit);
		while (height--) {
			WRITE_WORD(draw_buffer, addr0, mask, value);
			addr0 += BUFFER_WIDTH;
		}
	} else {
		// Otherwise we need to write the edges and then the middle repeatedly.
		mask_l    = COMPUTE_HLINE_EDGE_L_MASK(addr0_bit);
		mask_r    = COMPUTE_HLINE_EDGE_R_MASK(addr1_bit);
		// Write edges first.
		yy        = 0;
		addr0_old = addr0;
		addr1_old = addr1;
		while (yy < height) {
			WRITE_WORD(draw_buffer, addr0, mask_l, value);
			WRITE_WORD(draw_buffer, addr1, mask_r, value);
			addr0 += BUFFER_WIDTH;
			addr1 += BUFFER_WIDTH;
			yy++;
		}
		// Now write 0xffff words from start+1 to end-1 for each row.
		yy    = 0;
		addr0 = addr0_old;
		addr1 = addr1_old;
		while (yy < height) {
			for (i = addr0 + 1; i <= addr1 - 1; i++) {
				uint8_t m = 0xff;
				WRITE_WORD(draw_buffer, i, m, value);
			}
			addr0 += BUFFER_WIDTH;
			addr1 += BUFFER_WIDTH;
			yy++;
		}
	}
}
#endif /* defined(VIDEO_SPLITBUFFER) */

/**
 * write_filled_rectangle_lm: draw a filled rectangle on both draw buffers.
 *
 * @param       x               x coordinate (left)
 * @param       y               y coordinate (top)
 * @param       width   rectangle width
 * @param       height  rectangle height
 * @param       lmode   0 = clear, 1 = set, 2 = toggle
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 */
void write_filled_rectangle_lm(int x, int y, int width, int height, int lmode, int mmode)
{
#if defined(VIDEO_SPLITBUFFER)
	write_filled_rectangle(draw_buffer_mask, x, y, width, height, mmode);
	write_filled_rectangle(draw_buffer_level, x, y, width, height, lmode);
#else
	uint8_t value = PACK_BITS(mmode, lmode);
	write_filled_rectangle(x, y, width, height, value);
#endif /* defined(VIDEO_SPLITBUFFER) */
}

/**
 * write_rectangle_outlined: draw an outline of a rectangle. Essentially
 * a convenience wrapper for draw_hline_outlined and draw_vline_outlined.
 *
 * @param       x               x coordinate (left)
 * @param       y               y coordinate (top)
 * @param       width   rectangle width
 * @param       height  rectangle height
 * @param       mode    0 = black outline, white body, 1 = white outline, black body
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 */
void write_rectangle_outlined(int x, int y, int width, int height, int mode, int mmode)
{
	write_hline_outlined(x, x + width, y, ENDCAP_ROUND, ENDCAP_ROUND, mode, mmode);
	write_hline_outlined(x, x + width, y + height, ENDCAP_ROUND, ENDCAP_ROUND, mode, mmode);
	write_vline_outlined(x, y, y + height, ENDCAP_ROUND, ENDCAP_ROUND, mode, mmode);
	write_vline_outlined(x + width, y, y + height, ENDCAP_ROUND, ENDCAP_ROUND, mode, mmode);
}

#if defined(VIDEO_SPLITBUFFER)
/**
 * write_circle: draw the outline of a circle on a given buffer,
 * with an optional dash pattern for the line instead of a normal line.
 *
 * @param       buff    pointer to buffer to write in
 * @param       cx              origin x coordinate
 * @param       cy              origin y coordinate
 * @param       r               radius
 * @param       dashp   dash period (pixels) - zero for no dash
 * @param       mode    0 = clear, 1 = set, 2 = toggle
 */
void write_circle(uint8_t *buff, int cx, int cy, int r, int dashp, int mode)
{
	CHECK_COORDS(cx, cy);
	int error = -r, x = r, y = 0;
	while (x >= y) {
		if (dashp == 0 || (y % dashp) < (dashp / 2)) {
			CIRCLE_PLOT_8(buff, cx, cy, x, y, mode);
		}
		error += (y * 2) + 1;
		y++;
		if (error >= 0) {
			--x;
			error -= x * 2;
		}
	}
}

/**
 * write_circle_outlined: draw an outlined circle on the draw buffer.
 *
 * @param       cx              origin x coordinate
 * @param       cy              origin y coordinate
 * @param       r               radius
 * @param       dashp   dash period (pixels) - zero for no dash
 * @param       bmode   0 = 4-neighbour border, 1 = 8-neighbour border
 * @param       mode    0 = black outline, white body, 1 = white outline, black body
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 */
void write_circle_outlined(int cx, int cy, int r, int dashp, int bmode, int mode, int mmode)
{
	int stroke, fill;

	CHECK_COORDS(cx, cy);
	SETUP_STROKE_FILL(stroke, fill, mode);
	// This is a two step procedure. First, we draw the outline of the
	// circle, then we draw the inner part.
	int error = -r, x = r, y = 0;
	while (x >= y) {
		if (dashp == 0 || (y % dashp) < (dashp / 2)) {
			CIRCLE_PLOT_8(draw_buffer_mask, cx, cy, x + 1, y, mmode);
			CIRCLE_PLOT_8(draw_buffer_level, cx, cy, x + 1, y, stroke);
			CIRCLE_PLOT_8(draw_buffer_mask, cx, cy, x, y + 1, mmode);
			CIRCLE_PLOT_8(draw_buffer_level, cx, cy, x, y + 1, stroke);
			CIRCLE_PLOT_8(draw_buffer_mask, cx, cy, x - 1, y, mmode);
			CIRCLE_PLOT_8(draw_buffer_level, cx, cy, x - 1, y, stroke);
			CIRCLE_PLOT_8(draw_buffer_mask, cx, cy, x, y - 1, mmode);
			CIRCLE_PLOT_8(draw_buffer_level, cx, cy, x, y - 1, stroke);
			if (bmode == 1) {
				CIRCLE_PLOT_8(draw_buffer_mask, cx, cy, x + 1, y + 1, mmode);
				CIRCLE_PLOT_8(draw_buffer_level, cx, cy, x + 1, y + 1, stroke);
				CIRCLE_PLOT_8(draw_buffer_mask, cx, cy, x - 1, y - 1, mmode);
				CIRCLE_PLOT_8(draw_buffer_level, cx, cy, x - 1, y - 1, stroke);
			}
		}
		error += (y * 2) + 1;
		y++;
		if (error >= 0) {
			--x;
			error -= x * 2;
		}
	}
	error = -r;
	x     = r;
	y     = 0;
	while (x >= y) {
		if (dashp == 0 || (y % dashp) < (dashp / 2)) {
			CIRCLE_PLOT_8(draw_buffer_mask, cx, cy, x, y, mmode);
			CIRCLE_PLOT_8(draw_buffer_level, cx, cy, x, y, fill);
		}
		error += (y * 2) + 1;
		y++;
		if (error >= 0) {
			--x;
			error -= x * 2;
		}
	}
}

/**
 * write_circle_filled: fill a circle on a given buffer.
 *
 * @param       buff    pointer to buffer to write in
 * @param       cx              origin x coordinate
 * @param       cy              origin y coordinate
 * @param       r               radius
 * @param       mode    0 = clear, 1 = set, 2 = toggle
 */
void write_circle_filled(uint8_t *buff, int cx, int cy, int r, int mode)
{
	CHECK_COORDS(cx, cy);
	int error = -r, x = r, y = 0, xch = 0;
	// It turns out that filled circles can take advantage of the midpoint
	// circle algorithm. We simply draw very fast horizontal lines across each
	// pair of X,Y coordinates. In some cases, this can even be faster than
	// drawing an outlined circle!
	//
	// Due to multiple writes to each set of pixels, we have a special exception
	// for when using the toggling draw mode.
	while (x >= y) {
		if (y != 0) {
			write_hline(buff, cx - x, cx + x, cy + y, mode);
			write_hline(buff, cx - x, cx + x, cy - y, mode);
			if (mode != 2 || (mode == 2 && xch && (cx - x) != (cx - y))) {
				write_hline(buff, cx - y, cx + y, cy + x, mode);
				write_hline(buff, cx - y, cx + y, cy - x, mode);
				xch = 0;
			}
		}
		error += (y * 2) + 1;
		y++;
		if (error >= 0) {
			--x;
			xch    = 1;
			error -= x * 2;
		}
	}
	// Handle toggle mode.
	if (mode == 2) {
		write_hline(buff, cx - r, cx + r, cy, mode);
	}
}
#endif /* defined(VIDEO_SPLITBUFFER) */

/**
 * write_line: Draw a line of arbitrary angle.
 *
 * @param       buff    pointer to buffer to write in
 * @param       x0              first x coordinate
 * @param       y0              first y coordinate
 * @param       x1              second x coordinate
 * @param       y1              second y coordinate
 * @param       mode    0 = clear, 1 = set, 2 = toggle
 */
#if defined(VIDEO_SPLITBUFFER)
void write_line(uint8_t *buff, int x0, int y0, int x1, int y1, int mode)
{
	// Based on http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	int steep = abs(y1 - y0) > abs(x1 - x0);

	if (steep) {
		SWAP(x0, y0);
		SWAP(x1, y1);
	}
	if (x0 > x1) {
		SWAP(x0, x1);
		SWAP(y0, y1);
	}
	int deltax     = x1 - x0;
	int deltay = abs(y1 - y0);
	int error      = deltax / 2;
	int ystep;
	int y = y0;
	int x; // , lasty = y, stox = 0;
	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}
	for (x = x0; x < x1; x++) {
		if (steep) {
			write_pixel(buff, y, x, mode);
		} else {
			write_pixel(buff, x, y, mode);
		}
		error -= deltay;
		if (error < 0) {
			y     += ystep;
			error += deltax;
		}
	}
}
#else
void write_line(int x0, int y0, int x1, int y1, uint8_t value)
{
	// Based on http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	int steep = abs(y1 - y0) > abs(x1 - x0);

	if (steep) {
		SWAP(x0, y0);
		SWAP(x1, y1);
	}
	if (x0 > x1) {
		SWAP(x0, x1);
		SWAP(y0, y1);
	}
	int deltax     = x1 - x0;
	int deltay = abs(y1 - y0);
	int error      = deltax / 2;
	int ystep;
	int y = y0;
	int x; // , lasty = y, stox = 0;
	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}
	for (x = x0; x < x1; x++) {
		if (steep) {
			write_pixel(y, x, value);
		} else {
			write_pixel(x, y, value);
		}
		error -= deltay;
		if (error < 0) {
			y     += ystep;
			error += deltax;
		}
	}
}
#endif /* defined(VIDEO_SPLITBUFFER) */

/**
 * write_line_lm: Draw a line of arbitrary angle.
 *
 * @param       x0              first x coordinate
 * @param       y0              first y coordinate
 * @param       x1              second x coordinate
 * @param       y1              second y coordinate
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 * @param       lmode   0 = clear, 1 = set, 2 = toggle
 */
void write_line_lm(int x0, int y0, int x1, int y1, int mmode, int lmode)
{
#if defined(VIDEO_SPLITBUFFER)
	write_line(draw_buffer_mask, x0, y0, x1, y1, mmode);
	write_line(draw_buffer_level, x0, y0, x1, y1, lmode);
#else
	uint8_t value = PACK_BITS(mmode, lmode);
	write_line(x0, y0, x1, y1, value);
#endif /* defined(VIDEO_SPLITBUFFER) */
}

/**
 * write_line_outlined: Draw a line of arbitrary angle, with an outline.
 *
 * @param       x0                      first x coordinate
 * @param       y0                      first y coordinate
 * @param       x1                      second x coordinate
 * @param       y1                      second y coordinate
 * @param       endcap0         0 = none, 1 = single pixel, 2 = full cap
 * @param       endcap1         0 = none, 1 = single pixel, 2 = full cap
 * @param       mode            0 = black outline, white body, 1 = white outline, black body
 * @param       mmode           0 = clear, 1 = set, 2 = toggle
 */
void write_line_outlined(int x0, int y0, int x1, int y1,
						 __attribute__((unused)) int endcap0, __attribute__((unused)) int endcap1,
						 int mode, int mmode)
{
	// Based on http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	// This could be improved for speed.
	int omode, imode;

	if (mode == 0) {
		omode = 0;
		imode = 1;
	} else {
		omode = 1;
		imode = 0;
	}
	int steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		SWAP(x0, y0);
		SWAP(x1, y1);
	}
	if (x0 > x1) {
		SWAP(x0, x1);
		SWAP(y0, y1);
	}
	int deltax     = x1 - x0;
	int deltay = abs(y1 - y0);
	int error      = deltax / 2;
	int ystep;
	int y = y0;
	int x;
	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}
	// Draw the outline.
	for (x = x0; x < x1; x++) {
		if (steep) {
			write_pixel_lm(y - 1, x, mmode, omode);
			write_pixel_lm(y + 1, x, mmode, omode);
			write_pixel_lm(y, x - 1, mmode, omode);
			write_pixel_lm(y, x + 1, mmode, omode);
		} else {
			write_pixel_lm(x - 1, y, mmode, omode);
			write_pixel_lm(x + 1, y, mmode, omode);
			write_pixel_lm(x, y - 1, mmode, omode);
			write_pixel_lm(x, y + 1, mmode, omode);
		}
		error -= deltay;
		if (error < 0) {
			y     += ystep;
			error += deltax;
		}
	}
	// Now draw the innards.
	error = deltax / 2;
	y     = y0;
	for (x = x0; x < x1; x++) {
		if (steep) {
			write_pixel_lm(y, x, mmode, imode);
		} else {
			write_pixel_lm(x, y, mmode, imode);
		}
		error -= deltay;
		if (error < 0) {
			y     += ystep;
			error += deltax;
		}
	}
}

/**
 * write_line_outlined_dashed: Draw a line of arbitrary angle, with an outline, potentially dashed.
 *
 * @param       x0              first x coordinate
 * @param       y0              first y coordinate
 * @param       x1              second x coordinate
 * @param       y1              second y coordinate
 * @param       endcap0         0 = none, 1 = single pixel, 2 = full cap
 * @param       endcap1         0 = none, 1 = single pixel, 2 = full cap
 * @param       mode            0 = black outline, white body, 1 = white outline, black body
 * @param       mmode           0 = clear, 1 = set, 2 = toggle
 * @param       dots			0 = not dashed, > 0 = # of set/unset dots for the dashed innards
 */
void write_line_outlined_dashed(int x0, int y0, int x1, int y1,
								__attribute__((unused)) int endcap0, __attribute__((unused)) int endcap1,
								int mode, int mmode, int dots)
{
	// Based on http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	// This could be improved for speed.
	int omode, imode;

	if (mode == 0) {
		omode = 0;
		imode = 1;
	} else {
		omode = 1;
		imode = 0;
	}
	int steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		SWAP(x0, y0);
		SWAP(x1, y1);
	}
	if (x0 > x1) {
		SWAP(x0, x1);
		SWAP(y0, y1);
	}
	int deltax = x1 - x0;
	int deltay = abs(y1 - y0);
	int error  = deltax / 2;
	int ystep;
	int y = y0;
	int x;
	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}
	// Draw the outline.
	int dot_cnt = 0;
	int draw    = 1;
	for (x = x0; x < x1; x++) {
		if (dots && !(dot_cnt++ % dots)) {
			draw++;
		}
		if (draw % 2) {
			if (steep) {
				write_pixel_lm(y - 1, x, mmode, omode);
				write_pixel_lm(y + 1, x, mmode, omode);
				write_pixel_lm(y, x - 1, mmode, omode);
				write_pixel_lm(y, x + 1, mmode, omode);
			} else {
				write_pixel_lm(x - 1, y, mmode, omode);
				write_pixel_lm(x + 1, y, mmode, omode);
				write_pixel_lm(x, y - 1, mmode, omode);
				write_pixel_lm(x, y + 1, mmode, omode);
			}
		}
		error -= deltay;
		if (error < 0) {
			y     += ystep;
			error += deltax;
		}
	}
	// Now draw the innards.
	error = deltax / 2;
	y     = y0;
	dot_cnt = 0;
	draw    = 1;
	for (x = x0; x < x1; x++) {
		if (dots && !(dot_cnt++ % dots)) {
			draw++;
		}
		if (draw % 2) {
			if (steep) {
				write_pixel_lm(y, x, mmode, imode);
			} else {
				write_pixel_lm(x, y, mmode, imode);
			}
		}
		error -= deltay;
		if (error < 0) {
			y     += ystep;
			error += deltax;
		}
	}
}

/**
 * write_word_misaligned_NAND: Write a misaligned word across two addresses
 * with an x offset, using a NAND mask.
 *
 * This allows for many pixels to be set in one write.
 *
 * @param       buff    buffer to write in
 * @param       word    word to write (16 bits)
 * @param       addr    address of first word
 * @param       xoff    x offset (0-15)
 *
 * This is identical to calling write_word_misaligned with a mode of 0 but
 * it doesn't go through a lot of switch logic which slows down text writing
 * a lot.
 */
void write_word_misaligned_NAND(uint8_t *buff, uint16_t word, unsigned int addr, unsigned int xoff)
{
	uint16_t firstmask = word >> xoff;
	uint16_t lastmask  = word << (16 - xoff);

	WRITE_WORD_NAND(buff, addr + 1, firstmask & 0x00ff);
	WRITE_WORD_NAND(buff, addr, (firstmask & 0xff00) >> 8);
	if (xoff > 0) {
		WRITE_WORD_NAND(buff, addr + 2, (lastmask & 0xff00) >> 8);
	}
}

/**
 * write_word_misaligned_OR: Write a misaligned word across two addresses
 * with an x offset, using an OR mask.
 *
 * This allows for many pixels to be set in one write.
 *
 * @param       buff    buffer to write in
 * @param       word    word to write (16 bits)
 * @param       addr    address of first word
 * @param       xoff    x offset (0-15)
 *
 * This is identical to calling write_word_misaligned with a mode of 1 but
 * it doesn't go through a lot of switch logic which slows down text writing
 * a lot.
 */
void write_word_misaligned_OR(uint8_t *buff, uint16_t word, unsigned int addr, unsigned int xoff)
{
	uint16_t firstmask = word >> xoff;
	uint16_t lastmask  = word << (16 - xoff);

	WRITE_WORD_OR(buff, addr + 1, firstmask & 0x00ff);
	WRITE_WORD_OR(buff, addr, (firstmask & 0xff00) >> 8);
	if (xoff > 0) {
		WRITE_WORD_OR(buff, addr + 2, (lastmask & 0xff00) >> 8);
	}
}

/**
 * write_word_misaligned_OR: Write a misaligned word across two addresses
 * with an x offset, using an OR mask.
 *
 * This allows for many pixels to be set in one write.
 *
 * @param       buff    buffer to write in
 * @param       word    word to write (16 bits)
 * @param       addr    address of first word
 * @param       xoff    x offset (0-15)
 *
 * This is identical to calling write_word_misaligned with a mode of 1 but
 * it doesn't go through a lot of switch logic which slows down text writing
 * a lot.
 */
void write_word_misaligned_MASKED(uint8_t *buff, uint16_t word, uint16_t mask, unsigned int addr, unsigned int xoff)
{
	uint16_t firstword = (word >> xoff);
	uint16_t lastword  = word << (16 - xoff);
	uint16_t firstmask = (mask >> xoff);
	uint16_t lastmask  = mask << (16 - xoff);

	WRITE_WORD(buff, addr + 1, firstmask & 0x00ff, firstword & 0x00ff);
	WRITE_WORD(buff, addr, (firstmask & 0xff00) >> 8, (firstword & 0xff00) >> 8);
	if (xoff > 0) {
		WRITE_WORD(buff, addr + 2, (lastmask & 0xff00) >> 8, (lastword & 0xff00) >> 8);
	}
}


/**
 * write_char: Draw a character on the current draw buffer.
 *
 * @param       ch           character to write
 * @param       x            x coordinate (left)
 * @param       y            y coordinate (top)
 * @param       font_info    font to use
 */
void write_char(uint8_t ch, int x, int y, const struct FontEntry *font_info)
{
	int yy, row;
#if defined(VIDEO_SPLITBUFFER)
	uint16_t levels;
#else
	uint16_t data16;
#endif

	uint16_t mask;
	ch = font_info->lookup[ch];
	if (ch == 255)
		return;

	// check if char is partly out of boundary
	uint8_t partly_out = (x < GRAPHICS_LEFT) || (x + font_info->width > GRAPHICS_RIGHT) || (y < GRAPHICS_TOP) || (y + font_info->height > GRAPHICS_BOTTOM);
	// check if char is totally out of boundary, if so return
	if (partly_out && ((x + font_info->width < GRAPHICS_LEFT) || (x > GRAPHICS_RIGHT) || (y + font_info->height < GRAPHICS_TOP) || (y > GRAPHICS_BOTTOM))) {
		return;
	}

	// Compute starting address of character
	int addr = CALC_BUFF_ADDR(x, y);
	int wbit = CALC_BIT_IN_WORD(x);
	row = ch * font_info->height;

	if (font_info->width > 8) {
		uint32_t data;
		for (yy = y; yy < y + font_info->height; yy++) {
			if (!partly_out || ((x >= GRAPHICS_LEFT) && (x + font_info->width <= GRAPHICS_RIGHT) && (yy >= GRAPHICS_TOP) && (yy <= GRAPHICS_BOTTOM))) {
				data = ((uint32_t*)font_info->data)[row];
#if defined(VIDEO_SPLITBUFFER)
				mask = data & 0xFFFF;
				levels   = (data >> 16) & 0xFFFF;
				// mask
				write_word_misaligned_OR(draw_buffer_mask, mask, addr, wbit);
				// level
				write_word_misaligned_OR(draw_buffer_level, mask, addr, wbit);
				mask = (mask & levels);
				write_word_misaligned_NAND(draw_buffer_level, mask, addr, wbit);
#else
				data16 = (data & 0xFFFF0000) >> 16;
				mask = data16 | (data16 << 1);
				write_word_misaligned_MASKED(draw_buffer, data16, mask, addr, wbit);
				data16 = (data & 0x0000FFFF);
				mask = data16 | (data16 << 1);
				write_word_misaligned_MASKED(draw_buffer, data16, mask, addr + 2, wbit);
#endif /* defined(VIDEO_SPLITBUFFER) */
			}
			addr += BUFFER_WIDTH;
			row++;
		}
	}
	else {
		uint16_t data;
		for (yy = y; yy < y + font_info->height; yy++) {
			if (!partly_out || ((x >= GRAPHICS_LEFT) && (x + font_info->width <= GRAPHICS_RIGHT) && (yy >= GRAPHICS_TOP) && (yy <= GRAPHICS_BOTTOM))) {
				data = font_info->data[row];
#if defined(VIDEO_SPLITBUFFER)
				levels = data & 0xFF00;
				mask = (data & 0x00FF) << 8;
				// mask
				write_word_misaligned_OR(draw_buffer_mask, mask, addr, wbit);
				// level
				write_word_misaligned_OR(draw_buffer_level, mask, addr, wbit);
				mask = (mask & levels);
				write_word_misaligned_NAND(draw_buffer_level, mask, addr, wbit);
#else
				mask = data | (data << 1);
				write_word_misaligned_MASKED(draw_buffer, data, mask, addr, wbit);
#endif /* defined(VIDEO_SPLITBUFFER) */
			}
			addr += BUFFER_WIDTH;
			row++;
		}
	}
}


/**
 * fetch_font_info: Fetch font info structs.
 *
 * @param       font    font id
 */
const struct FontEntry * get_font_info(int font)
{
	if (font >= NUM_FONTS)
		return NULL;
	return fonts[font];
}

/**
 * calc_text_dimensions: Calculate the dimensions of a
 * string in a given font. Supports new lines and
 * carriage returns in text.
 *
 * @param       str                     string to calculate dimensions of
 * @param       font_info       font info structure
 * @param       xs                      horizontal spacing
 * @param       ys                      vertical spacing
 * @param       dim                     return result: struct FontDimensions
 */
void calc_text_dimensions(const char *str, const struct FontEntry *font, int xs, int ys, struct FontDimensions *dim)
{
	int max_length = 0, line_length = 0, lines = 1;

	while (*str != 0) {
		line_length++;
		if (*str == '\n' || *str == '\r') {
			if (line_length > max_length) {
				max_length = line_length;
			}
			line_length = 0;
			lines++;
		}
		str++;
	}
	if (line_length > max_length) {
		max_length = line_length;
	}
	dim->width  = max_length * (font->width + xs);
	dim->height = lines * (font->height + ys);
}

/**
 * write_string: Draw a string on the screen with certain
 * alignment parameters.
 *
 * @param       str             string to write
 * @param       x               x coordinate
 * @param       y               y coordinate
 * @param       xs              horizontal spacing
 * @param       ys              horizontal spacing
 * @param       va              vertical align
 * @param       ha              horizontal align
 * @param       font    font
 */
void write_string(const char *str, int x, int y, int xs, int ys, int va, int ha, int font)
{
	int xx = 0, yy = 0, xx_original = 0;
	const struct FontEntry *font_info;
	struct FontDimensions dim;

	font_info = get_font_info(font);

	calc_text_dimensions(str, font_info, xs, ys, &dim);
	switch (va) {
	case TEXT_VA_TOP:
		yy = y;
		break;
	case TEXT_VA_MIDDLE:
		yy = y - (dim.height / 2) + 1;
		break;
	case TEXT_VA_BOTTOM:
		yy = y - dim.height;
		break;
	}

	switch (ha) {
	case TEXT_HA_LEFT:
		xx = x;
		break;
	case TEXT_HA_CENTER:
		xx = x - (dim.width / 2);
		break;
	case TEXT_HA_RIGHT:
		xx = x - dim.width;
		break;
	}
	// Then write each character.
	xx_original = xx;
	while (*str != 0) {
		if (*str == '\n' || *str == '\r') {
			yy += ys + font_info->height;
			xx  = xx_original;
		} else {
			if (xx >= 0 && xx < GRAPHICS_WIDTH_REAL) {
				write_char(*str, xx, yy, font_info);
			}
			xx += font_info->width + xs;
		}
		str++;
	}
}

/**
 * Draw a polygon
 *
 */
void draw_polygon(int16_t x, int16_t y, float angle, const point_t * points, uint8_t n_points, int mode, int mmode)
{
	float sin_angle, cos_angle;
	int16_t x1, y1, x2, y2;

	sin_angle    = sinf(angle * (float)(M_PI / 180));
	cos_angle    = cosf(angle * (float)(M_PI / 180));

	x1 = roundf(cos_angle * points[0].x - sin_angle * points[0].y);
	y1 = roundf(sin_angle * points[0].x + cos_angle * points[0].y);
	x2 = 0; // so compiler doesn't give a warning
	y2 = 0;

	for (int i=0; i<n_points-1; i++)
	{
		x2 = roundf(cos_angle * points[i + 1].x - sin_angle * points[i + 1].y);
		y2 = roundf(sin_angle * points[i + 1].x + cos_angle * points[i + 1].y);

		write_line_outlined(x + x1, y + y1, x + x2, y + y2, 2, 2, mode, mmode);
		x1 = x2;
		y1 = y2;
	}

	x1 = roundf(cos_angle * points[0].x - sin_angle * points[0].y);
	y1 = roundf(sin_angle * points[0].x + cos_angle * points[0].y);
	write_line_outlined(x + x1, y + y1, x + x2, y + y2, 2, 2, mode, mmode);

	for (int i=0; i<n_points-1; i++)
	{
		x2 = roundf(cos_angle * points[i + 1].x - sin_angle * points[i + 1].y);
		y2 = roundf(sin_angle * points[i + 1].x + cos_angle * points[i + 1].y);

		write_line_lm(x + x1, y + y1, x + x2, y + y2, 1, 1);
		x1 = x2;
		y1 = y2;
	}

	x1 = roundf(cos_angle * points[0].x - sin_angle * points[0].y);
	y1 = roundf(sin_angle * points[0].x + cos_angle * points[0].y);

	write_line_lm( x + x1, y + y1, x + x2, y + y2, 1, 1);
}


void draw_polygon_simple(int16_t x, int16_t y, float angle, const point_t * points, uint8_t n_points, uint8_t color)
{
    float sin_angle, cos_angle;
    int16_t x1, y1, x2, y2;

    sin_angle    = sinf(angle * (float)(M_PI / 180));
    cos_angle    = cosf(angle * (float)(M_PI / 180));

    x1 = roundf(cos_angle * points[0].x - sin_angle * points[0].y);
    y1 = roundf(sin_angle * points[0].x + cos_angle * points[0].y);
    x2 = 0; // so compiler doesn't give a warning
    y2 = 0;

    for (int i=0; i<n_points-1; i++)
    {
        x2 = roundf(cos_angle * points[i + 1].x - sin_angle * points[i + 1].y);
        y2 = roundf(sin_angle * points[i + 1].x + cos_angle * points[i + 1].y);

        write_line_lm(x + x1, y + y1, x + x2, y + y2, 1, color);
        x1 = x2;
        y1 = y2;
    }

    x1 = roundf(cos_angle * points[0].x - sin_angle * points[0].y);
    y1 = roundf(sin_angle * points[0].x + cos_angle * points[0].y);
    write_line_lm(x + x1, y + y1, x + x2, y + y2, 1, color);
}


/**
 * hud_draw_vertical_scale: Draw a vertical scale.
 *
 * @param       v                   value to display as an integer
 * @param       range               range about value to display (+/- range/2 each direction)
 * @param       halign              horizontal alignment: -1 = left, +1 = right.
 * @param       x                   x displacement
 * @param       y                   y displacement
 * @param       height              height of scale
 * @param       mintick_step        how often a minor tick is shown
 * @param       majtick_step        how often a major tick is shown
 * @param       mintick_len         minor tick length
 * @param       majtick_len         major tick length
 * @param       boundtick_len       boundary tick length
 * @param       max_val             maximum expected value (used to compute size of arrow ticker)
 * @param       flags               special flags (see hud.h.)
 */
// #define VERTICAL_SCALE_BRUTE_FORCE_BLANK_OUT
#define VERTICAL_SCALE_FILLED_NUMBER
#define VSCALE_FONT FONT8X10
void osd_draw_vertical_scale(int v, int range, int halign, int x, int y, int height, int mintick_step, int majtick_step, int mintick_len,
                             int majtick_len, int boundtick_len, int flags)
{
    char temp[15];
    const struct FontEntry *font_info;
    struct FontDimensions dim;
    // Compute the position of the elements.
    int majtick_start = 0, majtick_end = 0, mintick_start = 0, mintick_end = 0, boundtick_start = 0, boundtick_end = 0;

    majtick_start   = x;
    mintick_start   = x;
    boundtick_start = x;
    if (halign == -1) {
        majtick_end     = x + majtick_len;
        mintick_end     = x + mintick_len;
        boundtick_end   = x + boundtick_len;
    } else if (halign == +1) {
        majtick_end     = x - majtick_len;
        mintick_end     = x - mintick_len;
        boundtick_end   = x - boundtick_len;
    }
    // Retrieve width of large font (font #0); from this calculate the x spacing.
    font_info = get_font_info(VSCALE_FONT);
    if (font_info == NULL)
        return;
    int arrow_len      = (font_info->height / 2) + 1;
    int text_x_spacing = (font_info->width / 2);
    int max_text_y     = 0, text_length = 0;
    int small_font_char_width = font_info->width + 1; // +1 for horizontal spacing = 1
    // For -(range / 2) to +(range / 2), draw the scale.
    int range_2 = range / 2; // , height_2 = height / 2;
    int r = 0, rr = 0, rv = 0, ys = 0, style = 0; // calc_ys = 0,
    // Iterate through each step.
    for (r = -range_2; r <= +range_2; r++) {
        style = 0;
        rr    = r + range_2 - v; // normalise range for modulo, subtract value to move ticker tape
        rv    = -rr + range_2; // for number display
        if (flags & HUD_VSCALE_FLAG_NO_NEGATIVE) {
            rr += majtick_step / 2;
        }
        if (rr % majtick_step == 0) {
            style = 1; // major tick
        } else if (rr % mintick_step == 0) {
            style = 2; // minor tick
        } else {
            style = 0;
        }
        if (flags & HUD_VSCALE_FLAG_NO_NEGATIVE && rv < 0) {
            continue;
        }
        if (style) {
            // Calculate y position.
            ys = ((long int)(r * height) / (long int)range) + y;
            // Depending on style, draw a minor or a major tick.
            if (style == 1) {
                write_hline_outlined(majtick_start, majtick_end, ys, 2, 2, 0, 1);
                memset(temp, ' ', 10);
                tfp_sprintf(temp, "%d", rv);
                text_length = (strlen(temp) + 1) * small_font_char_width; // add 1 for margin
                if (text_length > max_text_y) {
                    max_text_y = text_length;
                }
                if (halign == -1) {
                    write_string(temp, majtick_end + text_x_spacing + 1, ys, 1, 0, TEXT_VA_MIDDLE, TEXT_HA_LEFT, FONT_OUTLINED8X8);
                } else {
                    write_string(temp, majtick_end - text_x_spacing + 1, ys, 1, 0, TEXT_VA_MIDDLE, TEXT_HA_RIGHT, FONT_OUTLINED8X8);
                }
            } else if (style == 2) {
                write_hline_outlined(mintick_start, mintick_end, ys, 2, 2, 0, 1);
            }
        }
    }
    // Generate the string for the value, as well as calculating its dimensions.
    memset(temp, ' ', 10);
    // my_itoa(v, temp);
    tfp_sprintf(temp, "%02d", v);
    // TODO: add auto-sizing.
    calc_text_dimensions(temp, font_info, 1, 0, &dim);
    int xx = 0, i = 0;
    if (halign == -1) {
        xx = majtick_end + text_x_spacing;
    } else {
        xx = majtick_end - text_x_spacing;
    }
    y++;
    uint8_t width =  dim.width + 4;
    // Draw an arrow from the number to the point.
    for (i = 0; i < arrow_len; i++) {
        if (halign == -1) {
            write_pixel_lm(xx - arrow_len + i, y - i - 1, 1, 1);
            write_pixel_lm(xx - arrow_len + i, y + i - 1, 1, 1);
#ifdef VERTICAL_SCALE_FILLED_NUMBER
            write_hline_lm(xx + width - 1, xx - arrow_len + i + 1, y - i - 1, 0, 1);
            write_hline_lm(xx + width - 1, xx - arrow_len + i + 1, y + i - 1, 0, 1);
#else
            write_hline_lm(xx + width - 1, xx - arrow_len + i + 1, y - i - 1, 0, 0);
            write_hline_lm(xx + width - 1, xx - arrow_len + i + 1, y + i - 1, 0, 0);
#endif
        } else {
            write_pixel_lm(xx + arrow_len - i, y - i - 1, 1, 1);
            write_pixel_lm(xx + arrow_len - i, y + i - 1, 1, 1);
#ifdef VERTICAL_SCALE_FILLED_NUMBER
            write_hline_lm(xx - width - 1, xx + arrow_len - i - 1, y - i - 1, 0, 1);
            write_hline_lm(xx - width - 1, xx + arrow_len - i - 1, y + i - 1, 0, 1);
#else
            write_hline_lm(xx - width - 1, xx + arrow_len - i - 1, y - i - 1, 0, 0);
            write_hline_lm(xx - width - 1, xx + arrow_len - i - 1, y + i - 1, 0, 0);
#endif
        }
    }
    if (halign == -1) {
        write_hline_lm(xx, xx + width -1, y - arrow_len, 1, 1);
        write_hline_lm(xx, xx + width - 1, y + arrow_len - 2, 1, 1);
        write_vline_lm(xx + width - 1, y - arrow_len, y + arrow_len - 2, 1, 1);
    } else {
        write_hline_lm(xx, xx - width - 1, y - arrow_len, 1, 1);
        write_hline_lm(xx, xx - width - 1, y + arrow_len - 2, 1, 1);
        write_vline_lm(xx - width - 1, y - arrow_len, y + arrow_len - 2, 1, 1);
    }
    // Draw the text.
    if (halign == -1) {
        write_string(temp, xx + width / 2, y - 1, 1, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, VSCALE_FONT);
    } else {
        write_string(temp, xx - width / 2, y - 1, 1, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, VSCALE_FONT);
    }
#ifdef VERTICAL_SCALE_BRUTE_FORCE_BLANK_OUT
    // This is a bad brute force method destuctive to other things that maybe drawn underneath like e.g. the artificial horizon:
    // Then, add a slow cut off on the edges, so the text doesn't sharply
    // disappear. We simply clear the areas above and below the ticker, and we
    // use little markers on the edges.
    if (halign == -1) {
        write_filled_rectangle_lm(majtick_end + text_x_spacing, y + (height / 2) - (font_info->height / 2), max_text_y - boundtick_start,
                                  font_info->height, 0, 0);
        write_filled_rectangle_lm(majtick_end + text_x_spacing, y - (height / 2) - (font_info->height / 2), max_text_y - boundtick_start,
                                  font_info->height, 0, 0);
    } else {
        write_filled_rectangle_lm(majtick_end - text_x_spacing - max_text_y, y + (height / 2) - (font_info->height / 2), max_text_y,
                                  font_info->height, 0, 0);
        write_filled_rectangle_lm(majtick_end - text_x_spacing - max_text_y, y - (height / 2) - (font_info->height / 2), max_text_y,
                                  font_info->height, 0, 0);
    }
#endif
    y--;
    write_hline_outlined(boundtick_start, boundtick_end, y + (height / 2), 2, 2, 0, 1);
    write_hline_outlined(boundtick_start, boundtick_end, y - (height / 2), 2, 2, 0, 1);
}

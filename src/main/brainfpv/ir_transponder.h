/**
 ******************************************************************************
 * @file       pios_ir_transponder.h
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @brief      Generate packets for various infrared lap timin protocols
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


#ifndef IR_PROTOCOLS_H
#define IR_PROTOCOLS_H

#include <stdint.h>

void ir_generate_ilap_packet(uint32_t ilap_id, uint8_t * data, uint8_t data_len);
void ir_generate_trackmate_packet(uint16_t trackmate_id, uint8_t * data, uint8_t data_len);
uint16_t ir_next_valid_trackmateid(uint16_t trackmate_id, int16_t step);
#endif /* IR_PROTOCOLS_H */









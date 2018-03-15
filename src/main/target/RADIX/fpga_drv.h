/**
 ******************************************************************************
 * @addtogroup dRonin Targets
 * @{
 * @addtogroup BrainRE1 support files
 * @{
 *
 * @file       fpga_drv.h
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @brief      Driver for the RE1 custom FPGA
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

#ifndef BRAINFPVFPGA_H
#define BRAINFPVFPGA_H

#include <stdbool.h>
#include <stdint.h>

enum re1fpga_led_colors {
    BRAINFPVFPGA_STATUS_BLUE_CUSTOM_GREEN,
    BRAINFPVFPGA_STATUS_GREEN_CUSTOM_BLUE,
};

enum re1fpga_ir_protocols {
    BRAINFPVFPGA_IR_PROTOCOL_OFF,
    BRAINFPVFPGA_IR_PROTOCOL_ILAP,
    BRAINFPVFPGA_IR_PROTOCOL_TRACKMATE,
};

enum re1fpga_buzzer_types {
    BRAINFPVFPGA_BUZZER_DC,
    BRAINFPVFPGA_BUZZER_AC,
};


int32_t BRAINFPVFPGA_Init(bool load_config);
uint8_t BRAINFPVFPGA_GetHWRevision();
int32_t BRAINFPVFPGA_SerialRxInvert(bool invert);
int32_t BRAINFPVFPGA_MPTxPinMode(bool bidrectional, bool invert);
int32_t BRAINFPVFPGA_MPTxPinPullUpDown(bool enable, bool pullup);
int32_t BRAINFPVFPGA_SetBuzzerType(enum re1fpga_buzzer_types type);
int32_t BRAINFPVFPGA_Buzzer(bool enable);
int32_t BRAINFPVFPGA_BuzzerToggle();
int32_t BRAINFPVFPGA_AlarmLED(bool enable);
int32_t BRAINFPVFPGA_AlarmLEDToggle();
int32_t BRAINFPVFPGA_SetNotificationLedColor(enum re1fpga_led_colors led_colors);
void BRAINFPVFPGA_SetBwLevels(uint8_t black, uint8_t white);
int32_t BRAINFPVFPGA_SetSyncThreshold(uint8_t threshold);
void BRAINFPVFPGA_SetXOffset(int8_t x_offset);
void BRAINFPVFPGA_SetXScale(uint8_t x_scale);
void BRAINFPVFPGA_Set3DConfig(bool enabled, uint8_t x_shift_right);
int32_t BRAINFPVFPGA_SetLEDs(const uint8_t * led_data, uint16_t n_leds);
int32_t BRAINFPVFPGA_SetLEDColor(uint16_t n_leds, uint8_t red, uint8_t green, uint8_t blue);
int32_t BRAINFPVFPGA_SetIRProtocol(enum re1fpga_ir_protocols ir_protocol);
int32_t BRAINFPVFPGA_SetIRData(const uint8_t * ir_data, uint8_t n_bytes);
#endif /* BRAINFPVFPGA_H */

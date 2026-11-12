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
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// Message ids below are generated from the MSP YAML schema by msp/generator/gen_c.py.

// radar commands

#define MSP2_COMMON_TZ                     0x1001
#define MSP2_COMMON_SET_TZ                 0x1002
#define MSP2_COMMON_SETTING                0x1003
#define MSP2_COMMON_SET_SETTING            0x1004
#define MSP2_COMMON_MOTOR_MIXER            0x1005
#define MSP2_COMMON_SET_MOTOR_MIXER        0x1006
#define MSP2_COMMON_SETTING_INFO           0x1007
#define MSP2_COMMON_PG_LIST                0x1008
#define MSP2_COMMON_SERIAL_CONFIG          0x1009
#define MSP2_COMMON_SET_SERIAL_CONFIG      0x100A
#define MSP2_COMMON_SET_RADAR_POS          0x100B
#define MSP2_COMMON_SET_RADAR_ITD          0x100C
#define MSP2_COMMON_SET_MSP_RC_LINK_STATS  0x100D
#define MSP2_COMMON_SET_MSP_RC_INFO        0x100E
#define MSP2_COMMON_GET_RADAR_GPS          0x100F
#define MSP2_BETAFLIGHT_BIND               0x3000
#define MSP2_RX_BIND                       0x3001

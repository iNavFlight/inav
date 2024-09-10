/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */
/* Created by geoffsim */

#ifndef _VTX_MSP_H
#define _VTX_MSP_H


#define VTX_MSP_TIMEOUT         250 // ms
#define VTX_MSP_BAND_COUNT      5
#define VTX_MSP_CHANNEL_COUNT   8
#define VTX_MSP_POWER_COUNT     4

bool vtxMspInit(void);

#endif // _VTX_MSP_H

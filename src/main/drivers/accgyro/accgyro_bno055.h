/*
 * This file is part of INAV.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#define BNO055_ADDR_PWR_MODE 0x3E
#define BNO055_ADDR_OPR_MODE 0x3D

#define BNO055_PWR_MODE_NORMAL  0x00
#define BNO055_OPR_MODE_NDOF    0x0C

#define BNO055_ADDR_EUL_ROLL_LSB 0x1C
#define BNO055_ADDR_EUL_ROLL_MSB 0x1D

bool bno055Init(void);
fpVector3_t bno055GetEurlerAngles(void)
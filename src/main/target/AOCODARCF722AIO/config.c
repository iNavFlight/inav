/*
 * @Author: g05047
 * @Date: 2023-03-22 17:15:53
 * @LastEditors: g05047
 * @LastEditTime: 2023-03-23 16:21:45
 * @Description: file content
 */
/*
 * This file is part of INAV.
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

#include <stdint.h>
#include "platform.h"

#include "fc/fc_msp_box.h"
#include "io/piniobox.h"
#include "sensors/boardalignment.h"
#include "sensors/barometer.h"
#include "sensors/compass.h"
#include "sensors/acceleration.h"

void targetConfiguration(void)
{

   compassConfigMutable()->mag_align = CW90_DEG;
   
   // barometerConfigMutable()->baro_hardware = BARO_DPS310;

   boardAlignmentMutable()->rollDeciDegrees = -450;

}

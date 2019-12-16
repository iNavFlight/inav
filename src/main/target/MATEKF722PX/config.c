/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include "platform.h"
#include "io/piniobox.h"
#include "config/config_master.h"
#include "config/feature.h"
#include "io/serial.h"
#include "io/frsky_osd.h"

void targetConfiguration(void)
{
    pinioBoxConfigMutable()->permanentId[0] = 47;
    pinioBoxConfigMutable()->permanentId[1] = 48;

    serialConfigMutable()->portConfigs[6].functionMask = FUNCTION_FRSKY_OSD;
}

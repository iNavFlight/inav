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

#include "fc/fc_msp_box.h"
#include "fc/config.h"

#include "rx/rx.h"

#include "io/piniobox.h"
#include "drivers/serial.h"
#include "io/serial.h"

void targetConfiguration(void)
{
    serialConfigMutable()->portConfigs[1].functionMask = FUNCTION_GPS;
    serialConfigMutable()->portConfigs[1].gps_baudrateIndex = BAUD_115200;

    serialConfigMutable()->portConfigs[5].functionMask = FUNCTION_RX_SERIAL;
    rxConfigMutable()->receiverType = RX_TYPE_SERIAL;
    rxConfigMutable()->serialrx_provider = SERIALRX_SBUS;

    serialConfigMutable()->portConfigs[6].functionMask = FUNCTION_MSP;
    serialConfigMutable()->portConfigs[6].msp_baudrateIndex = BAUD_115200;

    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;
    pinioBoxConfigMutable()->permanentId[1] = BOX_PERMANENT_ID_USER2;
    pinioBoxConfigMutable()->permanentId[2] = BOX_PERMANENT_ID_USER3;
    pinioBoxConfigMutable()->permanentId[3] = BOX_PERMANENT_ID_USER4;
    beeperConfigMutable()->pwmMode = true;
}

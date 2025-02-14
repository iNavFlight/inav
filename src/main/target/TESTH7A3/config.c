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

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "fc/fc_msp_box.h"
#include "fc/config.h"

#include "io/piniobox.h"
#include "io/serial.h"

#include "drivers/serial.h"
#include "drivers/serial_uart.h"

void targetConfiguration(void)
{
    //pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;
    //pinioBoxConfigMutable()->permanentId[1] = BOX_PERMANENT_ID_USER2;
    beeperConfigMutable()->pwmMode = true;
    serialPortConfig_t *uart1Config = serialFindPortConfiguration(SERIAL_PORT_USART1);
    if (uart1Config && uart1Config->functionMask == 0) {
        uart1Config->functionMask = FUNCTION_MSP;
        uart1Config->msp_baudrateIndex = BAUD_115200;
    }

    serialPortConfig_t *uart6Config = serialFindPortConfiguration(SERIAL_PORT_USART6);
    if (uart6Config && uart6Config->functionMask == 0) {
        uart6Config->functionMask = FUNCTION_MSP;
        uart6Config->msp_baudrateIndex = BAUD_115200;
    }
}

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
#include "io/serial.h"
#include "sensors/compass.h"

void targetConfiguration(void)
{
#ifndef MICOAIR743V3_EXTMAG
    compassConfigMutable()->mag_align = CW90_DEG;
#endif

    // Use identifiers: V3 has no UART5, unlike MICOAIR743V2.
    serialFindPortConfiguration(SERIAL_PORT_USART2)->functionMask = FUNCTION_MSP_OSD;
    serialFindPortConfiguration(SERIAL_PORT_USART3)->functionMask = FUNCTION_GPS;
    serialFindPortConfiguration(SERIAL_PORT_USART7)->functionMask = FUNCTION_ESCSERIAL;
    serialFindPortConfiguration(SERIAL_PORT_USART8)->functionMask = FUNCTION_MSP; // Bluetooth

    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1; // VTX power control
}

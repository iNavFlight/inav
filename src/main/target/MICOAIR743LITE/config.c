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

#include "platform.h"

#include "io/serial.h"

void targetConfiguration(void)
{
    serialFindPortConfiguration(SERIAL_PORT_USART2)->functionMask = FUNCTION_MSP_OSD;
    serialFindPortConfiguration(SERIAL_PORT_USART3)->functionMask = FUNCTION_GPS;
    serialFindPortConfiguration(SERIAL_PORT_USART7)->functionMask = FUNCTION_ESCSERIAL;
    serialFindPortConfiguration(SERIAL_PORT_USART8)->functionMask = FUNCTION_MSP; // Bluetooth
}

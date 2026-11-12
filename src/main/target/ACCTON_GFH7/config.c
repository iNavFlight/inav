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
#include "sensors/gyro.h"

void targetConfiguration(void)
{
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART1)].functionMask = FUNCTION_ESCSERIAL;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART3)].functionMask = FUNCTION_GPS;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART4)].functionMask = FUNCTION_RX_SERIAL;

    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;

    // PINIO1 (PC12) drives the 12V BEC enable and is active LOW, so the rail is
    // powered as soon as pinioInit() drives the pin low. Activating the USER1
    // box therefore turns the 12V rail OFF.

    // Prefer the LSM6DSK320X, matching the Betaflight target default.
    gyroConfigMutable()->gyro_to_use = 1;

    compassConfigMutable()->mag_align = CW270_DEG;
}

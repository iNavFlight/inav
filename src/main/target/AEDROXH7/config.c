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

#include <stdbool.h>
#include <stdint.h>

#include <platform.h>

#include "config/config_master.h"

#include "fc/fc_msp_box.h"
#include "fc/config.h"

#include "io/serial.h"
#include "io/piniobox.h"

void targetConfiguration(void)
{
    // GPS on UART2
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART2)].functionMask = FUNCTION_GPS;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART2)].gps_baudrateIndex = BAUD_115200;

    // ESC telemetry on UART7 (RX only)
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART7)].functionMask = FUNCTION_ESCSERIAL;

    // HD OSD via MSP DisplayPort on UART8
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART8)].functionMask = FUNCTION_MSP;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART8)].msp_baudrateIndex = BAUD_115200;

    // PINIO boxes: USER1-4 mapped to switch boxes
    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;
    pinioBoxConfigMutable()->permanentId[1] = BOX_PERMANENT_ID_USER2;
    pinioBoxConfigMutable()->permanentId[2] = BOX_PERMANENT_ID_USER3;
    pinioBoxConfigMutable()->permanentId[3] = BOX_PERMANENT_ID_USER4;

    // Enable PWM drive for passive beeper on PA7 / TIM3_CH2
    beeperConfigMutable()->pwmMode = true;
}

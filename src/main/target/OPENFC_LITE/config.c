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

#include <platform.h>

#include "drivers/pwm_mapping.h"

#include "fc/fc_msp_box.h"

#include "flight/mixer.h"

#include "io/piniobox.h"
#include "io/serial.h"

#include "sensors/gyro.h"

void targetConfiguration(void)
{
    // PID at 1 kHz: measured on the RP2350_PICO the PID task takes ~390 us, the
    // 500 us default leaves no room for GPS/nav. The gyro task is not affected,
    // it follows the BMI270 output data rate (3.2 kHz).
    gyroConfigMutable()->looptime = 1000;

    // The update rate of DSHOT300 (8 kHz max) is plenty for a 1 kHz loop
    motorConfigMutable()->motorPwmProtocol = PWM_TYPE_DSHOT300;

    // 10 V VTX rail: on after boot, USER1 switches it off
    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;

    // Digital VTX connector
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART1)].functionMask = FUNCTION_MSP_OSD;
    // Telemetry pin of the ESC connector
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART4)].functionMask = FUNCTION_ESCSERIAL;
}

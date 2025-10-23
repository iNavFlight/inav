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
#include <stdbool.h>
#include <platform.h>
#include "fc/fc_msp_box.h"
#include "io/piniobox.h"
#include "io/serial.h"
#include "common/axis.h"
#include "config/config_master.h"
#include "config/feature.h"
#include "drivers/sensor.h"
#include "drivers/pwm_esc_detect.h"
#include "drivers/pwm_output.h"
#include "drivers/pwm_mapping.h"
#include "drivers/serial.h"
#include "fc/rc_controls.h"
#include "flight/failsafe.h"
#include "flight/mixer.h"
#include "flight/pid.h"
#include "rx/rx.h"
#include "io/serial.h"
#include "sensors/battery.h"
#include "sensors/sensors.h"
#include "telemetry/telemetry.h"


void targetConfiguration(void)
{
    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;
    pinioBoxConfigMutable()->permanentId[1] = BOX_PERMANENT_ID_USER2;

    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART1)].functionMask = FUNCTION_ESCSERIAL;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART6)].functionMask = FUNCTION_GPS;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART5)].functionMask = FUNCTION_RX_SERIAL;
}

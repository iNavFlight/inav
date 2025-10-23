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

#include "common/axis.h"

#include "config/config_master.h"
#include "config/feature.h"

#include "drivers/sensor.h"
#include "drivers/pwm_esc_detect.h"
#include "drivers/pwm_output.h"
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

#include "fc/fc_msp_box.h"

#include "io/piniobox.h"


#define BLUETOOTH_MSP_BAUDRATE      BAUD_115200

void targetConfiguration(void)
{
    pinioBoxConfigMutable()->permanentId[0] = BOXARM;
    pinioBoxConfigMutable()->permanentId[1] = BOX_PERMANENT_ID_USER1;

    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART3)].functionMask = FUNCTION_MSP;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART3)].msp_baudrateIndex = BLUETOOTH_MSP_BAUDRATE;
}


/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "drivers/pwm_mapping.h"
#include "fc/fc_msp_box.h"
#include "io/piniobox.h"
#include "io/serial.h"
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
#include "sensors/sensors.h"
#include "telemetry/telemetry.h"

void targetConfiguration(void)
{
    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;
    // serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USBVCP)].functionMask = FUNCTION_MSP;
    // serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USBVCP)].msp_baudrateIndex = BAUD_115200;

 
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART2)].functionMask = FUNCTION_RX_SERIAL;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART4)].functionMask = FUNCTION_MSP;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART4)].msp_baudrateIndex = BAUD_115200;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART4)].functionMask = FUNCTION_ESCSERIAL;

}

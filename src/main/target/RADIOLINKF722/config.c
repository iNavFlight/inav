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

#include "fc/fc_msp_box.h"
#include "io/serial.h"
#include "drivers/pwm_mapping.h"

void targetConfiguration(void)
{
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART4)].functionMask = FUNCTION_ESCSERIAL;
    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;
    timerOverridesMutable(timer2id(TIM2))->outputMode = OUTPUT_MODE_MOTORS;
    timerOverridesMutable(timer2id(TIM3))->outputMode = OUTPUT_MODE_MOTORS;
    timerOverridesMutable(timer2id(TIM4))->outputMode = OUTPUT_MODE_MOTORS;
}

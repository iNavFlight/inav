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

#include "io/serial.h"
#include "rx/rx.h"
#include "drivers/pwm_mapping.h"
#include "sensors/boardalignment.h"

void targetConfiguration(void)
{
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART6)].functionMask = FUNCTION_DJI_HD_OSD;

    // To improve backwards compatibility with INAV versions 6.x and older
    timerOverridesMutable(timer2id(TIM2))->outputMode = OUTPUT_MODE_MOTORS;

#if defined(SKYSTARSF405AIO)
    boardAlignmentMutable()->yawDeciDegrees = 450;
#endif

}

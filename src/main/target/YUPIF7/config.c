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

#include <stdbool.h>
#include <stdint.h>
#include <platform.h>

#include "config/feature.h"
#include "drivers/pwm_output.h"
#include "blackbox/blackbox.h"
#include "fc/config.h"
#include "fc/controlrate_profile.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "io/serial.h"
#include "rx/rx.h"
#include "sensors/sensors.h"
#include "sensors/gyro.h"
#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/compass.h"
#include "sensors/boardalignment.h"
#include "flight/pid.h"
#include "flight/mixer.h"
#include "flight/servos.h"
#include "flight/imu.h"
#include "flight/failsafe.h"
#include "drivers/sound_beeper.h"
#include "navigation/navigation.h"

void targetConfiguration(void)
{
    /* Specific PID values for YupiF7 */
    setConfigProfile(0);
    pidProfileMutable()->bank_mc.pid[PID_ROLL].P = 30;
    pidProfileMutable()->bank_mc.pid[PID_ROLL].I = 45;
    pidProfileMutable()->bank_mc.pid[PID_ROLL].D = 20;
    pidProfileMutable()->bank_mc.pid[PID_PITCH].P = 30;
    pidProfileMutable()->bank_mc.pid[PID_PITCH].I = 50;
    pidProfileMutable()->bank_mc.pid[PID_PITCH].D = 20;
    pidProfileMutable()->bank_mc.pid[PID_YAW].P = 40;
    pidProfileMutable()->bank_mc.pid[PID_YAW].I = 50;
    pidProfileMutable()->bank_mc.pid[PID_YAW].D = 0;
    pidProfileMutable()->bank_mc.pid[PID_LEVEL].P = 20;
    pidProfileMutable()->bank_mc.pid[PID_LEVEL].I = 10;
    pidProfileMutable()->bank_mc.pid[PID_LEVEL].D = 75;

}

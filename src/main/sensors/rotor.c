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

#include "sensors/rotor.h"
#include "stm32f4xx_exti.h"
// drivers/exti.h needs this?
#include <stdbool.h>
#include "drivers/exti.h"
#include "drivers/io.h"
#include "drivers/time.h"
#include "target.h"

typedef struct rotor_s {
  extiCallbackRec_t callback;
  IO_t io;
  timeDelta_t fall, rise, width, interval, bigtick;
  int currentTick, circleTicks;
  int rpm;
} rotor_t;

#define US_PER_MINUTE 60000000
void rotorCallback(rotor_t *r) {
  timeDelta_t t = micros();
  if (!IORead(r->io)) {
    r->rise = t;
  } else {
    timeDelta_t width = t - r->rise;
    r->currentTick++;
    // Was big tick?
    if (width * 2 > r->width * 3) {
      r->circleTicks = r->currentTick;
      r->currentTick = 0;
      r->rpm = US_PER_MINUTE / (t - r->bigtick);
      r->bigtick = t;
    }
    r->width = width;
    r->interval = t - r->fall;
    r->fall = t;
  }
}

static void rotorInitX(IO_t io, rotor_t* r) {
  EXTIHandlerInit(&r->callback, (extiHandlerCallback*) rotorCallback); 
  EXTIConfig(io, &r->callback, 1, EXTI_Trigger_Rising_Falling);
  EXTIEnable(io, 1);
  r->io = io;
  r->circleTicks = 2;
}

static int rotorAngleX(rotor_t *r) {
  timeDelta_t interval = micros() - r->fall;
  if (interval > r->interval) {
    // expected another tick by now.  assume it's exactly there.
    return ((r->currentTick + 1) % r->circleTicks)
           * 360 / r->circleTicks;
  } else {
    return r->currentTick * 360 / r->circleTicks +
           interval * 360 / r->circleTicks / r->interval;
  }
}

static int rotorRPMX(rotor_t* r) {
  return r->rpm;
}

static rotor_t rotor;

void rotorInit(void) { rotorInitX(IOGetByTag(IO_TAG(ROTOR_PIN)), &rotor); }
int rotorRPM(void) { return rotorRPMX(&rotor); }
int rotorAngle(void) { return rotorAngleX(&rotor); }

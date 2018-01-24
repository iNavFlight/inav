#include "stdbool.h"
#include "stdint.h"
#include "stdlib.h"

#include "platform.h"

#include "common/utils.h"

#include "drivers/time.h"
/*#include "drivers/io.h"*/
#include "drivers/lights_hal.h"

#include "rx/rx.h"

#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"

#include "scheduler/scheduler.h"

#include "config/feature.h"

#include "io/lights.h"

#ifdef USE_LIGHTS

static bool lights_on = false;

#ifdef USE_FAILSAFE_LIGHTS
  static timeUs_t last_status_change = 0;
#endif

bool lightsSetStatus(bool status)
{
    if (status != lights_on) {
        lights_on = status;
        lightsHardwareSetStatus(status);
        return(true);
    } else
        return(false);
}

/*
 * Lights handler function to be called periodically in loop. Updates lights
 * state via time schedule.
 */
void lightsUpdate(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);
#ifdef USE_FAILSAFE_LIGHTS
    if (FLIGHT_MODE(FAILSAFE_MODE) && ARMING_FLAG(WAS_EVER_ARMED)) {
        bool new_lights_status = lights_on;
        if (lights_on) {
            if (currentTimeUs - last_status_change > FAILSAFE_LIGHTS_ON_TIME * 1000)
                new_lights_status = false;
        } else {
            if (currentTimeUs - last_status_change > FAILSAFE_LIGHTS_OFF_TIME * 1000)
                new_lights_status = true;
        }
        if (new_lights_status != lights_on) {
            lightsSetStatus(new_lights_status);
            last_status_change = currentTimeUs;
        }
    } else {
        if (lightsSetStatus(IS_RC_MODE_ACTIVE(BOXLIGHTS)))
            last_status_change = currentTimeUs;
    }
#else
    lightsSetStatus(IS_RC_MODE_ACTIVE(BOXLIGHTS));
#endif /* USE_FAILSAFE_LIGHTS */
}

void lightsInit()
{
    setTaskEnabled(TASK_LIGHTS, lightsHardwareInit());
}

#endif /* USE_LIGHTS */

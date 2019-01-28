
#include "drivers/1-wire.h"
#include "drivers/1-wire/ds2482.h"

#include "drivers/logging.h"


#ifdef USE_1WIRE

#ifdef USE_1WIRE_DS2482
bool ds2482_detected = false;
_1WireDev_t ds2482Dev;
#endif

void _1WireInit(void)
{
    addBootlogEvent2(BOOT_EVENT_TEMP_SENSOR_DETECTION, BOOT_EVENT_FLAGS_NONE);
#ifdef USE_1WIRE_DS2482
    if (ds2482Detect(&ds2482Dev)) {
        ds2482_detected = true;
        ds2482Init(&ds2482Dev);
    }
#endif
}

#endif /* USE_1WIRE */


#include <string.h>

#include "drivers/1-wire.h"
#include "drivers/1-wire/ds2482.h"

#ifdef USE_1WIRE

#ifdef USE_1WIRE_DS2482
static bool ds2482Detected = false;
static owDev_t ds2482Dev;
#endif

void owInit(void)
{
    memset(&ds2482Dev, 0, sizeof(ds2482Dev));
#ifdef USE_1WIRE_DS2482
    if (ds2482Detect(&ds2482Dev)) ds2482Detected = true;
#endif
}

owDev_t *getOwDev(void)
{
    return ds2482Detected ? &ds2482Dev : NULL;
}

#endif /* USE_1WIRE */

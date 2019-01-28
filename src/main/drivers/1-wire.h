
#pragma once

#include "drivers/bus.h"

#ifdef USE_1WIRE

typedef struct _1WireDev_s {
    busDevice_t *busDev;
} _1WireDev_t;

#if defined(USE_1WIRE) && defined(USE_1WIRE_DS2482)
extern bool ds2482_detected;
extern _1WireDev_t ds2482Dev;
#endif

void _1WireInit(void);

#endif /* defined(USE_1WIRE) && defined(USE_1WIRE_DS2482) */

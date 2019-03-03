#pragma once

#include "drivers/osd.h"

typedef struct displayPort_s displayPort_t;

displayPort_t *aghOSDDisplayPortInit(const videoSystem_e videoSystem);

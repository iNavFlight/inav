#pragma once

#include "drivers/osd.h"

typedef struct displayPort_s displayPort_t;

displayPort_t *frskyOSDDisplayPortInit(const videoSystem_e videoSystem);

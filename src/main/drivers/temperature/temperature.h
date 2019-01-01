#pragma once

#include "drivers/sensor.h"

typedef struct temperatureDev_s {
    busDevice_t *busDev;
    sensorTempReadFuncPtr read;
} temperatureDev_t;

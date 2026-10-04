#pragma once

#include "drivers/sensor.h"

typedef struct temperatureDev_s {
    busDevice_t *busDev;
    sensorTempReadStartFuncPtr readStart;   // optional: start a non-blocking read, read() then only parses
    sensorTempReadFuncPtr read;
} temperatureDev_t;

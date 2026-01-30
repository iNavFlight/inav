#include "common/time.h"

typedef struct dronecanHardware_s {
    ioTag_t     ioTag;
    ioConfig_t  ioMode;
    uint8_t     alternate;
} dronecanHardware_t;

extern const dronecanHardware_t dronecanHardware[];
extern const int dronecanHardwareCount;

void dronecanInit(void);
void dronecanUpdate(timeUs_t currentTimeUs);

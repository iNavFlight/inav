#include "drivers/lights_hal.h"
#include "drivers/io.h"

#ifdef USE_LIGHTS

#if (!defined(LIGHTS_USE_PCA9685_OUTPUT)) && (!defined(LIGHTS_OUTPUT_MODE))
    #define LIGHTS_OUTPUT_MODE IOCFG_OUT_PP
#endif


static IO_t lightsIO = DEFIO_IO(NONE);

bool lightsHardwareInit()
{
#ifdef LIGHTS_USE_PCA9685_OUTPUT
#else
    lightsIO = IOGetByTag(IO_TAG(LIGHTS_PIN));

    if (lightsIO) {
        IOInit(lightsIO, OWNER_LED, RESOURCE_OUTPUT, 0);
        IOConfigGPIO(lightsIO, LIGHTS_OUTPUT_MODE);
        return(true);
    } else
        return(false);
#endif
}

void lightsHardwareSetStatus(bool status)
{
    IOWrite(lightsIO, status);
}

#endif /* USE_LIGHTS */

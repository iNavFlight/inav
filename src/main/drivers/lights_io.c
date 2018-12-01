#include "drivers/lights_io.h"
#include "drivers/io.h"

#ifdef USE_LIGHTS

#if (!defined(LIGHTS_USE_PCA9685_OUTPUT)) && (!defined(LIGHTS_OUTPUT_MODE))
    #define LIGHTS_OUTPUT_MODE IOCFG_OUT_PP
#endif


static IO_t lightsIO = DEFIO_IO(NONE);

bool lightsHardwareInit()
{
    lightsIO = IOGetByTag(IO_TAG(LIGHTS_PIN));

    if (lightsIO) {
        IOInit(lightsIO, OWNER_LED, RESOURCE_OUTPUT, 0);
        IOConfigGPIO(lightsIO, LIGHTS_OUTPUT_MODE);
        return(true);
    } else
        return(false);
}

void lightsHardwareSetStatus(bool status)
{
    if (lightsIO)
        IOWrite(lightsIO, status);
}

#endif /* USE_LIGHTS */

#include <stdbool.h>
#include <stdint.h>

#include <platform.h>

#include "io/serial.h"
#include "sensors/boardalignment.h"

void targetConfiguration(void)
{
    // UART1 is in the plug for vtx
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART1)].functionMask = FUNCTION_MSP_OSD;

    boardAlignmentMutable()->yawDeciDegrees = 1350;
}
#include <stdbool.h>
#include <stdint.h>

#include <platform.h>

#include "io/serial.h"
#include "io/ledstrip.h"
#include "sensors/boardalignment.h"

void targetConfiguration(void)
{
    // UART1 is in the plug for vtx
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART1)].functionMask = FUNCTION_MSP_OSD;

    boardAlignmentMutable()->yawDeciDegrees = 1350;

    DEFINE_LED(ledStripConfigMutable()->ledConfigs, 0, 0, COLOR_GREEN, 0, LED_FUNCTION_ARM_STATE, LED_FLAG_OVERLAY(LED_OVERLAY_WARNING), 0);
}
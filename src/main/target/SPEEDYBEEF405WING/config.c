#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "fc/fc_msp_box.h"
#include "io/serial.h"
#include "io/piniobox.h"

void targetConfiguration(void)
{
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART1)].functionMask = FUNCTION_RX_SERIAL;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART3)].functionMask = FUNCTION_GPS;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART6)].functionMask = FUNCTION_MSP;

    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;
}

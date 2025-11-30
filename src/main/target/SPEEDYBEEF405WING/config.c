#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "fc/fc_msp_box.h"
#include "io/serial.h"
#include "io/piniobox.h"

// Только САМЫЕ БАЗОВЫЕ includes
#include "fc/config.h"

void targetConfiguration(void)
{
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART1)].functionMask = FUNCTION_RX_SERIAL;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART3)].functionMask = FUNCTION_GPS;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART6)].functionMask = FUNCTION_MSP;

    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;

    // САМЫЕ БАЗОВЫЕ НАСТРОЙКИ
    featureSet(FEATURE_MOTOR_STOP);
    featureSet(FEATURE_PWM_OUTPUT_ENABLE);
    featureSet(FEATURE_FW_AUTOTRIM);
}

#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "drivers/pwm_mapping.h"
#include "fc/fc_msp_box.h"
#include "io/piniobox.h"
#include "io/serial.h"
#include "common/axis.h"
#include "config/config_master.h"
#include "config/feature.h"
#include "drivers/sensor.h"
#include "drivers/pwm_esc_detect.h"
#include "drivers/pwm_output.h"
#include "drivers/serial.h"
#include "fc/rc_controls.h"
#include "flight/failsafe.h"
#include "flight/mixer.h"
#include "flight/pid.h"
#include "rx/rx.h"
#include "sensors/sensors.h"
#include "telemetry/telemetry.h"

void targetConfiguration(void)
{
    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;
	pinioBoxConfigMutable()->permanentId[1] = BOX_PERMANENT_ID_USER2;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART2)].functionMask = FUNCTION_RX_SERIAL;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART3)].functionMask = FUNCTION_MSP;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART3)].msp_baudrateIndex = BAUD_115200;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART4)].functionMask = FUNCTION_ESCSERIAL;

}

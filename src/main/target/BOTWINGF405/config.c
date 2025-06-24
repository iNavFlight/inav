
 #include <stdbool.h>
 #include <stdint.h>
 
 #include "platform.h"
 
 #include "fc/fc_msp_box.h"
 #include "io/serial.h"
 
 void targetConfiguration(void)
 {
 
     pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;
     pinioBoxConfigMutable()->permanentId[1] = BOX_PERMANENT_ID_USER2;
     serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART1)].functionMask = FUNCTION_RX_SERIAL;
     serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART4)].functionMask = FUNCTION_MSP;
     serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART4)].msp_baudrateIndex = BAUD_115200;
     serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART2)].functionMask = FUNCTION_ESCSERIAL;
 }
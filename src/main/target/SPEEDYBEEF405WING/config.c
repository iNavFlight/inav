#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "fc/fc_msp_box.h"
#include "io/serial.h"
#include "io/piniobox.h"

// Только проверенные includes
#include "fc/config.h"
#include "sensors/boardalignment.h"
#include "sensors/gyro.h"
#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "navigation/navigation.h"
#include "flight/pid.h"
#include "osd/osd.h"

void targetConfiguration(void)
{
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART1)].functionMask = FUNCTION_RX_SERIAL;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART3)].functionMask = FUNCTION_GPS;
    serialConfigMutable()->portConfigs[findSerialPortIndexByIdentifier(SERIAL_PORT_USART6)].functionMask = FUNCTION_MSP;

    pinioBoxConfigMutable()->permanentId[0] = BOX_PERMANENT_ID_USER1;

    // === БАЗОВЫЕ НАСТРОЙКИ (без проблемных includes) ===
    
    // Features
    featureSet(FEATURE_MOTOR_STOP);
    featureSet(FEATURE_PWM_OUTPUT_ENABLE);
    featureSet(FEATURE_FW_AUTOTRIM);

    // Gyro
    gyroConfigMutable()->gyro_main_lpf_hz = 25;

    // Board Alignment
    boardAlignmentMutable()->yawDegrees = 1800;

    // Small Angle
    systemConfigMutable()->small_angle = 180;

    // GPS
    gpsConfigMutable()->sbasMode = SBAS_AUTO;

    // Navigation
    navigationConfigMutable()->extra_arming_safety = true;
    navigationConfigMutable()->rth_altitude = 5000;

    // OSD
    osdConfigMutable()->rssi_alarm = 2;
    osdConfigMutable()->alt_alarm = 3000;

    // Pilot Name
    strncpy(pilotConfigMutable()->name, "FIRE!!!", MAX_NAME_LENGTH);
}

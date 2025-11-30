#include "platform.h"

#ifdef TARGET_SPEEDYBEEF405WING

#include "fc/config.h"
#include "fc/controlrate_profile.h"
#include "fc/rc.h"
#include "io/serial.h"
#include "osd/osd.h"
#include "sensors/boardalignment.h"
#include "sensors/gyro.h"
#include "sensors/acceleration.h"
#include "sensors/mag.h"
#include "sensors/barometer.h"

void targetCustomConfiguration(void) {
    // === Features ===
    featureSet(FEATURE_MOTOR_STOP);
    featureSet(FEATURE_PWM_OUTPUT_ENABLE);
    featureSet(FEATURE_FW_AUTOTRIM);

    // === Serial Ports ===
    serialConfigMutable()->portConfigs[0].functionMask = FUNCTION_TELEMETRY_SMARTPORT;
    serialConfigMutable()->portConfigs[3].functionMask = FUNCTION_GPS;

    // === System Configuration ===
    systemConfigMutable()->emf_avoidance = 1;
    systemConfigMutable()->speaker_mode = 1;

    // === Gyro & ACC ===
    gyroConfigMutable()->gyro_main_lpf_hz = 25;
    gyroConfigMutable()->dynamic_gyro_notch_q = 250;
    gyroConfigMutable()->dynamic_gyro_notch_min_hz = 30;
    
    accelerometerConfigMutable()->acc_hardware = ACC_ICM42605;
    accelerometerConfigMutable()->acc_lpf_hz = 25;

    // === Magnetometer ===
    compassConfigMutable()->mag_hardware = MAG_QMC5883;
    compassConfigMutable()->mag_align = ALIGN_CW0_FLIP;

    // === Barometer ===
    barometerConfigMutable()->baro_hardware = BARO_SPL06;

    // === RC & Deadband ===
    rxConfigMutable()->deadband = 32;
    rxConfigMutable()->mincheck = 1050;
    rxConfigMutable()->maxcheck = 1900;

    // === Failsafe ===
    failsafeConfigMutable()->failsafe_procedure = FAILSAFE_PROCEDURE_AUTO_LANDING;

    // === Board Alignment ===
    boardAlignmentMutable()->yawDegrees = 1800;

    // === Current Meter ===
    currentMeterMutable()->scale = 160;

    // === Small Angle ===
    systemConfigMutable()->small_angle = 180;

    // === GPS Configuration ===
    gpsConfigMutable()->sbasMode = SBAS_AUTO;
    gpsConfigMutable()->ublox_use_galileo = true;
    gpsConfigMutable()->ublox_use_beidou = true;
    gpsConfigMutable()->ublox_use_glonass = true;

    // === Navigation ===
    navigationConfigMutable()->extra_arming_safety = true;
    navigationConfigMutable()->rth_altitude = 5000; // 50m
    navigationConfigMutable()->rth_allow_landing = RTH_LAND_FS_ONLY;
    navigationConfigMutable()->fw_wp_radius = 5000;
    navigationConfigMutable()->fw_max_safe_distance = 500;
    navigationConfigMutable()->fw_climb_angle = 12;
    navigationConfigMutable()->fw_control_smoothness = 2;
    navigationConfigMutable()->fw_launch_motor_delay = 1;
    navigationConfigMutable()->fw_launch_spinup_time = 250;
    navigationConfigMutable()->fw_launch_end_time = 5000;
    navigationConfigMutable()->fw_launch_timeout = 50000;
    navigationConfigMutable()->fw_launch_max_altitude = 5000;
    navigationConfigMutable()->fw_launch_climb_angle = 15;

    // === OSD Configuration ===
    osdConfigMutable()->rssi_alarm = 2;
    osdConfigMutable()->alt_alarm = 3000; // 30m
    osdConfigMutable()->link_quality_alarm = 2;
    osdConfigMutable()->crosshairs_style = CROSSHAIRS_STYLE_TYPE7;

    // === Pilot Name ===
    strncpy(pilotConfigMutable()->name, "FIRE!!!", MAX_NAME_LENGTH);

    // === Fixed Wing PID ===
    pidProfileMutable()->fw_p_pitch = 15;
    pidProfileMutable()->fw_i_pitch = 5;
    pidProfileMutable()->fw_d_pitch = 5;
    pidProfileMutable()->fw_ff_pitch = 80;
    pidProfileMutable()->fw_p_roll = 13;
    pidProfileMutable()->fw_i_roll = 3;
    pidProfileMutable()->dterm_lpf_hz = 10;
    pidProfileMutable()->fw_turn_assist_yaw_gain = 200; // 2.00
    pidProfileMutable()->fw_turn_assist_pitch_gain = 60; // 0.60

    // === Rates ===
    controlRateConfigMutable()->rcExpo8 = 30;
    controlRateConfigMutable()->rcYawExpo8 = 30;
    controlRateConfigMutable()->rates[FD_ROLL] = 18;
    controlRateConfigMutable()->rates[FD_PITCH] = 9;
    controlRateConfigMutable()->rates[FD_YAW] = 3;

    // === Battery ===
    batteryConfigMutable()->vbatmaxcellvoltage = 445; // 4.45V
    batteryConfigMutable()->vbatmincellvoltage = 280; // 2.80V
    batteryConfigMutable()->vbatwarningcellvoltage = 280;

    // === Throttle ===
    motorConfigMutable()->throttle_idle = 50; // 5.0%

    // === Navigation Throttle ===
    navigationConfigMutable()->fw_cruise_throttle = 1650;
    navigationConfigMutable()->fw_min_throttle = 1550;
    navigationConfigMutable()->fw_max_throttle = 1800;
    navigationConfigMutable()->fw_launch_throttle = 2000;
    navigationConfigMutable()->fw_launch_idle_throttle = 1750;
    navigationConfigMutable()->fw_pitch_to_throttle = 6;
}

#endif

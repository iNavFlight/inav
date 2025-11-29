#include "platform.h"
#include "fc/fc_core.h"
#include "fc/settings.h"

void applyCustomDefaults(void) {

    // === Твои настройки из diff all ===
    // Копируй сюда всё, что было после "set " в твоём diff

    pgResetCopy(&gyroConfig_System, PG_GYRO_CONFIG);
    gyroConfigMutable()->gyro_hardware_lpf = GYRO_HARDWARE_LPF_256HZ;
    gyroConfigMutable()->gyro_main_lpf_hz = 25;
    gyroConfigMutable()->gyro_main_lpf_type = GYRO_MAIN_LPF_BIQUAD;
    gyroConfigMutable()->dynamic_gyro_notch_enabled = 1;
    gyroConfigMutable()->dynamic_gyro_notch_q = 250;
    gyroConfigMutable()->dynamic_gyro_notch_min_hz = 30;

    accelerometerConfigMutable()->acc_hardware = ACC_ICM42605;
    accelerometerConfigMutable()->acc_lpf_hz = 15;

    magConfigMutable()->mag_hardware = MAG_QMC5883;
    boardAlignmentMutable()->board_yaw = 1800;

    baroConfigMutable()->baro_hardware = BARO_SPL06;
    rangefinderConfigMutable()->rangefinder_hardware = RANGEFINDER_BENEWAKE;

    motorConfigMutable()->motor_pwm_protocol = PWM_TYPE_STANDARD;
    motorConfigMutable()->max_throttle = 2000;
    motorConfigMutable()->min_command = 1000;

    currentMeterConfigMutable()->scale = 160;

    failsafeConfigMutable()->failsafe_procedure = FAILSAFE_PROCEDURE_NONE;

    airplaneConfigMutable()->hasFlaps = 1;

    setPilotName("FIRE!!!");

    // 6S батарея
    batteryConfigMutable()->batteryCellCount = 6;
    batteryConfigMutable()->vbatmaxcellvoltage = 445;
    batteryConfigMutable()->vbatmincellvoltage = 280;
    batteryConfigMutable()->vbatwarningcellvoltage = 280;

    // Тяги для крыла
    fixedWingConfigMutable()->cruise_throttle = 1650;
    fixedWingConfigMutable()->min_throttle = 1550;
    fixedWingConfigMutable()->max_throttle = 1800;
    fixedWingConfigMutable()->pitch_to_throttle = 6;

    // PID и фильтры
    pidProfileMutable(1)->fw_p_pitch = 15;
    pidProfileMutable(1)->fw_i_pitch = 5;
    pidProfileMutable(1)->fw_d_pitch = 5;
    pidProfileMutable(1)->fw_ff_pitch = 80;
    pidProfileMutable(1)->fw_p_roll = 13;
    pidProfileMutable(1)->fw_i_roll = 3;
    pidProfileMutable(1)->max_angle_inclination[FD_ROLL] = 450;

    // Калибровки (гиро, аксель, магнит) — твои точные значения
    gyroConfigMutable()->gyro_zero_x = -1;
    gyroConfigMutable()->gyro_zero_y = -2;
    insGravityCalibrationCmss = 977.575f;
    accelerometerConfigMutable()->accZero.raw[Y] = -2;
    accelerometerConfigMutable()->accZero.raw[Z] = 20;
    accelerometerConfigMutable()->accGain.raw[X] = 4091;
    accelerometerConfigMutable()->accGain.raw[Y] = 4093;
    accelerometerConfigMutable()->accGain.raw[Z] = 4094;
    magConfigMutable()->magZero.raw[X] = -143;
    magConfigMutable()->magZero.raw[Y] = -757;
    magConfigMutable()->magZero.raw[Z] = -298;
    magConfigMutable()->magGain.raw[X] = 2219;
    magConfigMutable()->magGain.raw[Y] = 1888;
    magConfigMutable()->magGain.raw[Z] = 1568;

    // Servo limits (твои точные)
    servoParamsMutable(1)->min = 850;  servoParamsMutable(1)->max = 2150;  servoParamsMutable(1)->middle = 1670;
    servoParamsMutable(2)->min = 1000; servoParamsMutable(2)->max = 2000;  servoParamsMutable(2)->middle = 1500;
    servoParamsMutable(3)->min = 1000; servoParamsMutable(3)->max = 2000;  servoParamsMutable(3)->middle = 1500;
    servoParamsMutable(4)->min = 1150; servoParamsMutable(4)->max = 1825;  servoParamsMutable(4)->middle = 1650;

    // smix и mmix — если хочешь, добавишь сюда же (чуть сложнее, но могу дописать)
}
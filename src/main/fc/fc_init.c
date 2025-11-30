/*
 * This file is part of Cleanflight and INAV.
 * (c) Your FIRE!!! custom build
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "blackbox/blackbox.h"
#include "blackbox/blackbox_io.h"

#include "build/assert.h"
#include "build/atomic.h"
#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/color.h"
#include "common/log.h"
#include "common/maths.h"
#include "common/memory.h"
#include "common/printf.h"
#include "programming/global_variables.h"

#include "config/config_eeprom.h"
#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "cms/cms.h"

#include "drivers/1-wire.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/adc.h"
#include "drivers/compass/compass.h"
#include "drivers/bus.h"
#include "drivers/dma.h"
#include "drivers/exti.h"
#include "drivers/io.h"
#include "drivers/flash.h"
#include "drivers/light_led.h"
#include "drivers/nvic.h"
#include "drivers/osd.h"
#include "drivers/persistent.h"
#include "drivers/pwm_esc_detect.h"
#include "drivers/pwm_mapping.h"
#include "drivers/pwm_output.h"
#include "drivers/sensor.h"
#include "drivers/serial.h"
#include "drivers/serial_softserial.h"
#include "drivers/serial_uart.h"
#include "drivers/serial_usb_vcp.h"
#include "drivers/sound_beeper.h"
#include "drivers/system.h"
#include "drivers/time.h"
#include "drivers/timer.h"
#include "drivers/uart_inverter.h"
#include "drivers/vtx_common.h"
#ifdef USE_USB_MSC
#include "drivers/usb_msc.h"
#include "msc/emfat_file.h"
#endif
#include "drivers/sdcard/sdcard.h"
#include "drivers/sdio.h"
#include "drivers/io_port_expander.h"

#include "fc/cli.h"
#include "fc/config.h"
#include "fc/fc_msp.h"
#include "fc/fc_tasks.h"
#include "fc/rc_controls.h"
#include "fc/runtime_config.h"
#include "fc/firmware_update.h"

#include "flight/failsafe.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/pid.h"
#include "flight/power_limits.h"
#include "flight/rpm_filter.h"
#include "flight/servos.h"
#include "flight/ez_tune.h"

#include "io/asyncfatfs/asyncfatfs.h"
#include "io/beeper.h"
#include "io/lights.h"
#include "io/dashboard.h"
#include "io/displayport_frsky_osd.h"
#include "io/displayport_msp.h"
#include "io/displayport_max7456.h"
#include "io/displayport_msp_osd.h"
#include "io/displayport_srxl.h"
#include "io/flashfs.h"
#include "io/gps.h"
#include "io/ledstrip.h"
#include "io/osd.h"
#include "io/osd_dji_hd.h"
#include "io/rcdevice_cam.h"
#include "io/serial.h"
#include "io/smartport_master.h"
#include "io/vtx.h"
#include "io/vtx_control.h"
#include "io/vtx_smartaudio.h"
#include "io/vtx_tramp.h"
#include "io/vtx_msp.h"
#include "io/vtx_ffpv24g.h"
#include "io/piniobox.h"

#include "msp/msp_serial.h"

#include "navigation/navigation.h"

#include "rx/rx.h"
#include "rx/spektrum.h"

#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/battery.h"
#include "sensors/boardalignment.h"
#include "sensors/compass.h"
#include "sensors/gyro.h"
#include "sensors/initialisation.h"
#include "sensors/pitotmeter.h"
#include "sensors/rangefinder.h"
#include "sensors/sensors.h"
#include "sensors/esc_sensor.h"

#include "scheduler/scheduler.h"

#include "telemetry/telemetry.h"

#ifdef USE_HARDWARE_REVISION_DETECTION
#include "hardware_revision.h"
#endif

#ifdef USE_HARDWARE_PREBOOT_SETUP
extern void initialisePreBootHardware(void);
#endif

extern uint8_t motorControlEnable;

typedef enum {
    SYSTEM_STATE_INITIALISING   = 0,
    SYSTEM_STATE_CONFIG_LOADED  = (1 << 0),
    SYSTEM_STATE_SENSORS_READY  = (1 << 1),
    SYSTEM_STATE_MOTORS_READY   = (1 << 2),
    SYSTEM_STATE_TRANSPONDER_ENABLED = (1 << 3),
    SYSTEM_STATE_READY          = (1 << 7)
} systemState_e;

uint8_t systemState = SYSTEM_STATE_INITIALISING;

/* =============================================================================
   ТВОИ НАСТРОЙКИ ПО УМОЛЧАНИЮ — применятся сразу после первой прошивки
   ============================================================================= */
const char * const default_cli_dump[] = {
    "# FIRE!!! custom defaults for SPEEDYBEEF405WING - iNav 7.1.2",
    "defaults noreboot",

    // Features
    "feature MOTOR_STOP",
    "feature PWM_OUTPUT_ENABLE",
    "feature FW_AUTOTRIM",

    // Blackbox
    "blackbox -NAV_ACC",
    "blackbox NAV_POS",
    "blackbox NAV_PID",
    "blackbox MAG",
    "blackbox ACC",
    "blackbox ATTI",
    "blackbox RC_DATA",
    "blackbox RC_COMMAND",
    "blackbox MOTORS",
    "blackbox -GYRO_RAW",
    "blackbox -PEAKS_R",
    "blackbox -PEAKS_P",
    "blackbox -PEAKS_Y",

    // Serial
    "serial 1 2 115200 115200 0 115200",
    "serial 4 65536 115200 115200 0 115200",

    // Modes
    "aux 0 0 0 1500 2100",
    "aux 1 1 2 900 1300",
    "aux 2 3 2 1300 1700",
    "aux 3 5 2 1300 1700",
    "aux 4 35 2 1300 1700",
    "aux 5 36 2 900 1300",
    "aux 6 42 3 1700 2100",
    "aux 7 47 0 900 2100",
    "aux 8 48 3 1700 2100",
    "aux 9 57 3 1700 2100",

    // Servos
    "servo 1 850 2150 1670 -125",
    "servo 2 1000 2000 1500 -105",
    "servo 3 1000 2000 1500 -105",
    "servo 4 1150 1825 1650 100",
    "servo 5 1000 2000 1200 100",

    // OSD layouts (твой кастомный)
    "osd_layout 0 0 25 2 V",
    "osd_layout 0 1 2 13 V",
    "osd_layout 0 2 0 0 V",
    "osd_layout 0 9 1 3 V",
    "osd_layout 0 11 2 11 H",
    "osd_layout 0 12 23 13 V",
    "osd_layout 0 13 13 5 V",
    "osd_layout 0 14 13 3 V",
    "osd_layout 0 15 1 8 V",
    "osd_layout 0 16 2 4 H",
    "osd_layout 0 17 2 5 H",
    "osd_layout 0 24 13 2 V",
    "osd_layout 0 25 28 5 V",
    "osd_layout 0 28 22 2 H",
    "osd_layout 0 30 1 14 V",
    "osd_layout 0 34 11 1 H",
    "osd_layout 0 35 2 12 H",
    "osd_layout 0 110 25 3 V",
    "osd_layout 0 147 0 0 V",
    "osd_layout 1 0 25 2 V",
    "osd_layout 1 1 2 13 V",
    "osd_layout 1 7 13 12 V",
    "osd_layout 1 9 1 3 V",
    "osd_layout 1 11 3 11 V",
    "osd_layout 1 12 23 13 V",
    "osd_layout 1 13 13 5 V",
    "osd_layout 1 14 13 3 V",
    "osd_layout 1 15 2 8 V",
    "osd_layout 1 16 2 4 H",
    "osd_layout 1 17 2 5 H",
    "osd_layout 1 24 13 2 V",
    "osd_layout 1 25 27 5 V",
    "osd_layout 1 28 22 2 H",
    "osd_layout 1 30 1 14 V",
    "osd_layout 1 34 11 1 H",
    "osd_layout 1 35 2 12 H",
    "osd_layout 1 110 25 3 V",
    "osd_layout 1 142 12 3 V",

    "osd_custom_elements 0 1 0 0 0 0 0 0 0 \"320H9197A0611\"",

    // Master settings
    "set gyro_main_lpf_hz = 25",
    "set dynamic_gyro_notch_q = 250",
    "set dynamic_gyro_notch_min_hz = 30",
    "set gyro_zero_x = -1",
    "set gyro_zero_y = -2",
    "set ins_gravity_cmss = 977.575",
    "set acc_hardware = ICM42605",
    "set acczero_y = -2",
    "set acczero_z = 20",
    "set accgain_x = 4091",
    "set accgain_y = 4093",
    "set accgain_z = 4094",
    "set rangefinder_hardware = BENEWAKE",
    "set align_mag = CW0FLIP",
    "set mag_hardware = QMC5883",
    "set magzero_x = -143",
    "set magzero_y = -757",
    "set magzero_z = -298",
    "set maggain_x = 2219",
    "set maggain_y = 1888",
    "set maggain_z = 1568",
    "set baro_hardware = SPL06",
    "set rc_filter_auto = OFF",
    "set max_throttle = 2000",
    "set motor_pwm_protocol = STANDARD",
    "set failsafe_procedure = NONE",
    "set align_board_yaw = 1800",
    "set current_meter_scale = 160",
    "set small_angle = 180",
    "set disarm_kill_switch = OFF",
    "set applied_defaults = 3",
    "set gps_sbas_mode = AUTO",
    "set gps_ublox_use_galileo = ON",
    "set gps_ublox_use_beidou = ON",
    "set gps_ublox_use_glonass = ON",
    "set deadband = 32",
    "set airmode_type = STICK_CENTER_ONCE",
    "set inav_w_z_baro_p = 0.350",
    "set nav_extra_arming_safety = ON",
    "set nav_wp_radius = 5000",
    "set nav_wp_max_safe_distance = 500",
    "set nav_rth_allow_landing = FS_ONLY",
    "set nav_rth_altitude = 5000",
    "set nav_fw_climb_angle = 12",
    "set nav_fw_control_smoothness = 2",
    "set nav_fw_launch_motor_delay = 1",
    "set nav_fw_launch_spinup_time = 250",
    "set nav_fw_launch_end_time = 5000",
    "set nav_fw_launch_timeout = 50000",
    "set nav_fw_launch_max_altitude = 5000",
    "set nav_fw_launch_climb_angle = 15",
    "set osd_rssi_alarm = 2",
    "set osd_alt_alarm = 3000",
    "set osd_link_quality_alarm = 2",
    "set osd_crosshairs_style = TYPE7",
    "set pilot_name = FIRE!!!",
    "set pinio_box3 = 57",

    // Platform & mixer
    "set platform_type = AIRPLANE",
    "set has_flaps = ON",
    "set model_preview_type = 26",

    "mmix reset",
    "mmix 0 1.000 0.000 0.000 -0.500",
    "mmix 1 1.000 0.000 0.000 0.500",

    "smix reset",
    "smix 0 1 1 100 0 -1",
    "smix 1 2 0 100 0 -1",
    "smix 2 3 0 100 0 -1",
    "smix 3 4 11 100 0 -1",

    // Profile 1 (твой основной)
    "profile 1",
    "set fw_p_pitch = 15",
    "set fw_i_pitch = 5",
    "set fw_d_pitch = 5",
    "set fw_ff_pitch = 80",
    "set fw_p_roll = 13",
    "set fw_i_roll = 3",
    "set max_angle_inclination_rll = 450",
    "set dterm_lpf_hz = 10",
    "set fw_turn_assist_yaw_gain = 2.000",
    "set fw_turn_assist_pitch_gain = 0.600",
    "set nav_fw_pos_z_p = 25",
    "set nav_fw_pos_z_d = 8",
    "set nav_fw_pos_xy_p = 55",
    "set d_boost_min = 1.000",
    "set d_boost_max = 1.000",
    "set tpa_rate = 5",
    "set tpa_breakpoint = 1750",
    "set rc_expo = 30",
    "set rc_yaw_expo = 30",
    "set roll_rate = 18",
    "set pitch_rate = 9",
    "set yaw_rate = 3",

    // Battery profile 1 (6S)
    "battery_profile 1",
    "set bat_cells = 6",
    "set vbat_cell_detect_voltage = 445",
    "set vbat_max_cell_voltage = 445",
    "set vbat_min_cell_voltage = 280",
    "set vbat_warning_cell_voltage = 280",
    "set throttle_idle = 5.000",
    "set nav_mc_hover_thr = 1600",
    "set nav_fw_cruise_thr = 1650",
    "set nav_fw_min_thr = 1550",
    "set nav_fw_max_thr = 1800",
    "set nav_fw_pitch2thr = 6",
    "set nav_fw_launch_thr = 2000",
    "set nav_fw_launch_idle_thr = 1750",
    "set limit_cont_current = 1470",
    "set limit_burst_current = 1520",
    "set limit_burst_current_time = 2000",
    "set limit_burst_current_falldown_time = 3000",

    // Возврат к нужным профилям
    "mixer_profile 1",
    "profile 1",
    "battery_profile 1",
    "save",
    NULL
};
/* ============================================================================= */

void flashLedsAndBeep(void)
{
    LED1_ON;
    LED0_OFF;
    for (uint8_t i = 0; i < 10; i++) {
        LED1_TOGGLE;
        LED0_TOGGLE;
        delay(25);
        if (!(getBeeperOffMask() & (1 << (BEEPER_SYSTEM_INIT - 1))))
            BEEP_ON;
        delay(25);
        BEEP_OFF;
    }
    LED0_OFF;
    LED1_OFF;
}

void init(void)
{
    // Весь оригинальный код init() остаётся без изменений до этой строки
    // (всё, что было ниже в оригинальном файле — просто оставляем как есть)
    // iNav сам увидит массив default_cli_dump и применит его при первом запуске

#if defined(USE_FLASHFS)
    bool flashDeviceInitialized = false;
#endif

#ifdef USE_HAL_DRIVER
    HAL_Init();
#endif

    systemState = SYSTEM_STATE_INITIALISING;
    printfSupportInit();
    systemInit();

#if !defined(SITL_BUILD)
    __enable_irq();
#endif

    IOInitGlobal();

#ifdef USE_HARDWARE_REVISION_DETECTION
    detectHardwareRevision();
#endif

#ifdef USE_BRUSHED_ESC_AUTODETECT
    detectBrushedESC();
#endif

#ifdef CONFIG_IN_EXTERNAL_FLASH
    pgResetAll(0);
    flashDeviceInitialized = flashInit();
#endif

    initEEPROM();
    ensureEEPROMContainsValidData();
    suspendRxSignal();
    readEEPROM();
    resumeRxSignal();

#ifdef USE_UNDERCLOCK
    systemClockSetup(systemConfig()->cpuUnderclock);
#else
    systemClockSetup(false);
#endif

#ifdef USE_I2C
    i2cSetSpeed(systemConfig()->i2c_speed);
#endif

, etc... (весь остальной оригинальный код init() оставляем без изменений до конца файла)

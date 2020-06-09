set(COMMON_SRC
    main.c

    target/common_hardware.c

    build/assert.c
    build/build_config.c
    build/debug.c
    build/version.c

    common/bitarray.c
    common/calibration.c
    common/colorconversion.c
    common/crc.c
    common/encoding.c
    common/filter.c
    common/gps_conversion.c
    common/log.c
    common/logic_condition.c
    common/global_functions.c
    common/global_variables.c
    common/maths.c
    common/memory.c
    common/olc.c
    common/printf.c
    common/streambuf.c
    common/string_light.c
    common/time.c
    common/typeconversion.c
    common/uvarint.c

    config/config_eeprom.c
    config/config_streamer.c
    config/feature.c
    config/parameter_group.c
    config/general_settings.c

    drivers/1-wire.c
    drivers/1-wire/ds_crc.c
    drivers/1-wire/ds2482.c

    drivers/accgyro/accgyro.c
    drivers/accgyro/accgyro.h
    drivers/accgyro/accgyro_adxl345.c
    drivers/accgyro/accgyro_adxl345.h
    drivers/accgyro/accgyro_bma280.c
    drivers/accgyro/accgyro_bma280.h
    drivers/accgyro/accgyro_bmi160.c
    drivers/accgyro/accgyro_bmi160.h
    drivers/accgyro/accgyro_fake.c
    drivers/accgyro/accgyro_fake.h
    drivers/accgyro/accgyro_icm20689.c
    drivers/accgyro/accgyro_icm20689.h
    drivers/accgyro/accgyro_l3g4200d.c
    drivers/accgyro/accgyro_l3g4200d.h
    drivers/accgyro/accgyro_l3gd20.c
    drivers/accgyro/accgyro_l3gd20.h
    drivers/accgyro/accgyro_lsm303dlhc.c
    drivers/accgyro/accgyro_lsm303dlhc.h
    drivers/accgyro/accgyro_mma845x.c
    drivers/accgyro/accgyro_mma845x.h
    drivers/accgyro/accgyro_mpu.c
    drivers/accgyro/accgyro_mpu.h
    drivers/accgyro/accgyro_mpu3050.c
    drivers/accgyro/accgyro_mpu3050.h
    drivers/accgyro/accgyro_mpu6000.c
    drivers/accgyro/accgyro_mpu6000.h
    drivers/accgyro/accgyro_mpu6050.c
    drivers/accgyro/accgyro_mpu6050.h
    drivers/accgyro/accgyro_mpu6500.c
    drivers/accgyro/accgyro_mpu6500.h
    drivers/accgyro/accgyro_mpu9250.c
    drivers/accgyro/accgyro_mpu9250.h

    drivers/adc.c

    drivers/barometer/barometer.h
    drivers/barometer/barometer_bmp085.c
    drivers/barometer/barometer_bmp085.h
    drivers/barometer/barometer_bmp280.c
    drivers/barometer/barometer_bmp280.h
    drivers/barometer/barometer_bmp388.c
    drivers/barometer/barometer_bmp388.h
    drivers/barometer/barometer_fake.c
    drivers/barometer/barometer_fake.h
    drivers/barometer/barometer_lps25h.c
    drivers/barometer/barometer_lps25h.h
    drivers/barometer/barometer_ms56xx.c
    drivers/barometer/barometer_ms56xx.h
    drivers/barometer/barometer_spl06.c
    drivers/barometer/barometer_spl06.h

    drivers/buf_writer.c
    drivers/bus.c
    drivers/bus_busdev_i2c.c
    drivers/bus_busdev_spi.c
    drivers/bus_i2c_soft.c
    drivers/bus_spi.c

    drivers/compass/compass.h
    drivers/compass/compass_ak8963.c
    drivers/compass/compass_ak8963.h
    drivers/compass/compass_ak8975.c
    drivers/compass/compass_ak8975.h
    drivers/compass/compass_fake.c
    drivers/compass/compass_fake.h
    drivers/compass/compass_hmc5883l.c
    drivers/compass/compass_hmc5883l.h
    drivers/compass/compass_ist8308.c
    drivers/compass/compass_ist8308.h
    drivers/compass/compass_ist8310.c
    drivers/compass/compass_ist8310.h
    drivers/compass/compass_lis3mdl.c
    drivers/compass/compass_lis3mdl.h
    drivers/compass/compass_mag3110.c
    drivers/compass/compass_mag3110.h
    drivers/compass/compass_mpu9250.c
    drivers/compass/compass_mpu9250.h
    drivers/compass/compass_qmc5883l.c
    drivers/compass/compass_qmc5883l.h

    drivers/display.c
    drivers/display_canvas.c
    drivers/display_font_metadata.c
    drivers/display_ug2864hsweg01.c
    drivers/exti.c
    drivers/flash.c
    drivers/flash_m25p16.c
    drivers/io.c
    drivers/io_pca9685.c
    drivers/irlock.c
    drivers/light_led.c
    drivers/light_ws2811strip.c
    drivers/lights_io.c
    drivers/max7456.c

    drivers/opflow/opflow_fake.c
    drivers/opflow/opflow_virtual.c

    drivers/osd.c
    drivers/persistent.c
    drivers/pitotmeter_adc.c
    drivers/pitotmeter_ms4525.c
    drivers/pitotmeter_virtual.c
    drivers/pwm_esc_detect.c
    drivers/pwm_mapping.c
    drivers/pwm_output.c
    drivers/pinio.c

    drivers/rangefinder/rangefinder_hcsr04.c
    drivers/rangefinder/rangefinder_hcsr04_i2c.c
    drivers/rangefinder/rangefinder_srf10.c
    drivers/rangefinder/rangefinder_vl53l0x.c
    drivers/rangefinder/rangefinder_virtual.c

    drivers/resource.c
    drivers/rcc.c
    drivers/rx_nrf24l01.c
    drivers/rx_pwm.c
    drivers/rx_spi.c
    drivers/rx_xn297.c
    drivers/serial.c
    drivers/serial_uart.c
    drivers/sound_beeper.c
    drivers/stack_check.c
    drivers/system.c

    drivers/temperature/ds18b20.c
    drivers/temperature/lm75.c

    drivers/time.c
    drivers/timer.c
    drivers/usb_msc.c
    drivers/vtx_common.c

    fc/cli.c
    fc/config.c
    fc/controlrate_profile.c
    fc/fc_core.c
    fc/fc_init.c
    fc/fc_tasks.c
    fc/fc_hardfaults.c
    fc/fc_msp.c
    fc/fc_msp_box.c
    fc/rc_smoothing.c
    fc/rc_adjustments.c
    fc/rc_controls.c
    fc/rc_curves.c
    fc/rc_modes.c
    fc/runtime_config.c
    fc/settings.c
    fc/stats.c

    flight/failsafe.c
    flight/hil.c
    flight/imu.c
    flight/mixer.c
    flight/pid.c
    flight/pid_autotune.c
    flight/rth_estimator.c
    flight/servos.c
    flight/wind_estimator.c
    flight/gyroanalyse.c
    flight/rpm_filter.c
    flight/dynamic_gyro_notch.c

    io/beeper.c
    io/esc_serialshot.c
    io/servo_sbus.c
    io/frsky_osd.c
    io/osd_dji_hd.c
    io/lights.c
    io/piniobox.c
    io/pwmdriver_i2c.c
    io/serial.c
    io/serial_4way.c
    io/serial_4way_avrootloader.c
    io/serial_4way_stk500v2.c
    io/statusindicator.c
    io/rcdevice.c
    io/rcdevice_cam.c

    msp/msp_serial.c

    rx/crsf.c
    rx/eleres.c
    rx/fport.c
    rx/ibus.c
    rx/jetiexbus.c
    rx/msp.c
    rx/msp_override.c
    rx/nrf24_cx10.c
    rx/nrf24_inav.c
    rx/nrf24_h8_3d.c
    rx/nrf24_syma.c
    rx/nrf24_v202.c
    rx/pwm.c
    rx/frsky_crc.c
    rx/rx.c
    rx/rx_spi.c
    rx/sbus.c
    rx/sbus_channels.c
    rx/spektrum.c
    rx/sumd.c
    rx/sumh.c
    rx/uib_rx.c
    rx/xbus.c

    scheduler/scheduler.c

    sensors/acceleration.c
    sensors/battery.c
    sensors/boardalignment.c
    sensors/compass.c
    sensors/diagnostics.c
    sensors/gyro.c
    sensors/initialisation.c
    sensors/esc_sensor.c
    sensors/irlock.c
    sensors/temperature.c

    uav_interconnect/uav_interconnect_bus.c
    uav_interconnect/uav_interconnect_rangefinder.c

    blackbox/blackbox.c
    blackbox/blackbox_encoding.c
    blackbox/blackbox_io.c

    cms/cms.c
    cms/cms_menu_battery.c
    cms/cms_menu_blackbox.c
    cms/cms_menu_builtin.c
    cms/cms_menu_imu.c
    cms/cms_menu_ledstrip.c
    cms/cms_menu_misc.c
    cms/cms_menu_mixer_servo.c
    cms/cms_menu_navigation.c
    cms/cms_menu_osd.c
    cms/cms_menu_saveexit.c
    cms/cms_menu_vtx.c

    io/rangefinder_msp.c
    io/rangefinder_benewake.c
    io/opflow_cxof.c
    io/opflow_msp.c
    io/dashboard.c
    io/displayport_frsky_osd.c
    io/displayport_max7456.c
    io/displayport_msp.c
    io/displayport_oled.c
    io/displayport_hott.c
    io/flashfs.c
    io/gps.c
    io/gps_ublox.c
    io/gps_nmea.c
    io/gps_naza.c
    io/ledstrip.c
    io/osd.c
    io/osd_canvas.c
    io/osd_common.c
    io/osd_grid.c
    io/osd_hud.c

    navigation/navigation.c
    navigation/navigation_fixedwing.c
    navigation/navigation_fw_launch.c
    navigation/navigation_geo.c
    navigation/navigation_multicopter.c
    navigation/navigation_pos_estimator.c
    navigation/navigation_pos_estimator_agl.c
    navigation/navigation_pos_estimator_flow.c
    navigation/navigation_rover_boat.c

    sensors/barometer.c
    sensors/pitotmeter.c
    sensors/rangefinder.c
    sensors/opflow.c

    telemetry/crsf.c
    telemetry/frsky.c
    telemetry/frsky_d.c
    telemetry/hott.c
    telemetry/ibus_shared.c
    telemetry/ibus.c
    telemetry/ltm.c
    telemetry/mavlink.c
    telemetry/msp_shared.c
    telemetry/smartport.c
    telemetry/sim.c
    telemetry/telemetry.c

    io/vtx.c
    io/vtx_string.c
    io/vtx_smartaudio.c
    io/vtx_tramp.c
    io/vtx_ffpv24g.c
    io/vtx_control.c
)

main_sources(COMMON_SRC)

add_subdirectory(target)

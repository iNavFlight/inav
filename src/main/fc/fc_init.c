/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
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
#include "drivers/gimbal_common.h"
#include "drivers/headtracker_common.h"
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
#include "drivers/io.h"
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
#include "io/gimbal_serial.h"
#include "io/headtracker_msp.h"
#include "io/gps.h"
#include "io/ledstrip.h"
#include "io/osd.h"
#include "io/osd_dji_hd.h"
#include "io/rcdevice_cam.h"
#include "io/serial.h"
#include "io/displayport_msp.h"
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

#if defined(SITL_BUILD)
#include "target/SITL/serial_proxy.h"
#endif

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
#if defined(USE_FLASHFS)
    bool flashDeviceInitialized = false;
#endif

#ifdef USE_HAL_DRIVER
    HAL_Init();
#endif

    systemState = SYSTEM_STATE_INITIALISING;
    printfSupportInit();

    // Initialize system and CPU clocks to their initial values
    systemInit();

#if !defined(SITL_BUILD)
    __enable_irq();
#endif

    // initialize IO (needed for all IO operations)
    IOInitGlobal();

#ifdef USE_HARDWARE_REVISION_DETECTION
    detectHardwareRevision();
#endif

#ifdef USE_BRUSHED_ESC_AUTODETECT
    detectBrushedESC();
#endif

#ifdef CONFIG_IN_EXTERNAL_FLASH
    // Reset config to defaults. Note: Default flash config must be functional for config in external flash to work.
    pgResetAll(0);

    flashDeviceInitialized = flashInit();
#endif

#if defined(SITL_BUILD)
    serialProxyInit();
#endif

    initEEPROM();
    ensureEEPROMContainsValidData();
    suspendRxSignal();
    readEEPROM();
    resumeRxSignal();

#ifdef USE_I2C
    i2cSetSpeed(systemConfig()->i2c_speed);
#endif

#ifdef USE_HARDWARE_PREBOOT_SETUP
    initialisePreBootHardware();
#endif

    systemState |= SYSTEM_STATE_CONFIG_LOADED;

    debugMode = systemConfig()->debug_mode;

    // Latch active features to be used for feature() in the remainder of init().
    latchActiveFeatures();

    ledInit(false);
#if !defined(SITL_BUILD)
    EXTIInit();
#endif

#if defined(USE_SPEKTRUM_BIND) && defined(USE_SERIALRX_SPEKTRUM)
    if (rxConfig()->receiverType == RX_TYPE_SERIAL) {
        switch (rxConfig()->serialrx_provider) {
            case SERIALRX_SPEKTRUM1024:
            case SERIALRX_SPEKTRUM2048:
                // Spektrum satellite binding if enabled on startup.
                // Must be called before that 100ms sleep so that we don't lose satellite's binding window after startup.
                // The rest of Spektrum initialization will happen later - via spektrumInit()
                spektrumBind(rxConfigMutable());
                break;
        }
    }
#endif

#ifdef USE_VCP
    // Early initialize USB hardware
    usbVcpInitHardware();
#endif

    timerInit();  // timer must be initialized before any channel is allocated

    serialInit(feature(FEATURE_SOFTSERIAL));

    // Initialize MSP serial ports here so LOG can share a port with MSP.
    // XXX: Don't call mspFcInit() yet, since it initializes the boxes and needs
    // to run after the sensors have been detected.
    mspSerialInit();

#if defined(USE_DJI_HD_OSD)
    // DJI OSD uses a special flavour of MSP (subset of Betaflight 4.1.1 MSP) - process as part of serial task
    djiOsdSerialInit();
#endif

#if defined(USE_SMARTPORT_MASTER)
    smartportMasterInit();
#endif

#if defined(USE_LOG)
    // LOG might use serial output, so we only can init it after serial port is ready
    // From this point on we can use LOG_*() to produce real-time debugging information
    logInit();
#endif

#ifdef USE_PROGRAMMING_FRAMEWORK
    gvInit();
#endif

    // Initialize servo and motor mixers
    // This needs to be called early to set up platform type correctly and count required motors & servos
    mixerConfigInit();

    // Some sanity checking
    if (motorConfig()->motorPwmProtocol == PWM_TYPE_BRUSHED) {
        featureClear(FEATURE_REVERSIBLE_MOTORS);
    }
    if (!STATE(ALTITUDE_CONTROL)) {
        featureClear(FEATURE_AIRMODE);
    }
#if !defined(SITL_BUILD)
    // Initialize motor and servo outpus
    if (pwmMotorAndServoInit()) {
        DISABLE_ARMING_FLAG(ARMING_DISABLED_PWM_OUTPUT_ERROR);
    }
    else {
        ENABLE_ARMING_FLAG(ARMING_DISABLED_PWM_OUTPUT_ERROR);
    }
#else
    DISABLE_ARMING_FLAG(ARMING_DISABLED_PWM_OUTPUT_ERROR);
#endif
    systemState |= SYSTEM_STATE_MOTORS_READY;

#ifdef USE_ESC_SENSOR
    // DSHOT supports a dedicated wire ESC telemetry. Kick off the ESC-sensor receiver initialization
    // We may, however, do listen_only, so need to init this anyway
    // Initialize escSensor after having done it with outputs
    escSensorInitialize();
#endif

#ifdef BEEPER
    beeperDevConfig_t beeperDevConfig = {
        .ioTag = IO_TAG(BEEPER),
#ifdef BEEPER_INVERTED
        .isOD = false,
        .isInverted = true
#else
        .isOD = true,
        .isInverted = false
#endif
    };

    beeperInit(&beeperDevConfig);
#endif
#ifdef USE_LIGHTS
    lightsInit();
#endif

#ifdef USE_UART_INVERTER
    uartInverterInit();
#endif

    // Initialize buses
    busInit();

#ifdef CONFIG_IN_EXTERNAL_FLASH
    // busInit re-configures the SPI pins. Init flash again so it is ready to write settings
    flashDeviceInitialized = flashInit();
#endif

#ifdef USE_HARDWARE_REVISION_DETECTION
    updateHardwareRevision();
#endif

#if defined(USE_SDCARD_SDIO) && (defined(STM32H7) || defined(STM32F7))
    sdioPinConfigure();
#endif

#ifdef USE_USB_MSC
    /* MSC mode will start after init, but will not allow scheduler to run,
     * so there is no bottleneck in reading and writing data
     */
    mscInit();
#if defined(USE_FLASHFS)
        // If the blackbox device is onboard flash, then initialize and scan
        // it to identify the log files *before* starting the USB device to
        // prevent timeouts of the mass storage device.
        if (blackboxConfig()->device == BLACKBOX_DEVICE_FLASH) {
            // Must initialise the device to read _anything_
            if (!flashDeviceInitialized) {
                flashDeviceInitialized = flashInit();
            }
            emfat_init_files();
        }
#endif

    if (mscCheckBoot() || mscCheckButton()) {
        if (mscStart() == 0) {
             mscWaitForButton();
        } else {
             NVIC_SystemReset();
        }
    }
#endif

#ifdef USE_I2C
#ifdef USE_I2C_DEVICE_1
    #ifdef I2C_DEVICE_1_SHARES_UART3
        if (!doesConfigurationUsePort(SERIAL_PORT_USART3)) {
            i2cInit(I2CDEV_1);
        }
    #else
            i2cInit(I2CDEV_1);
    #endif
#endif

#ifdef USE_I2C_DEVICE_2
    #ifdef I2C_DEVICE_2_SHARES_UART3
        if (!doesConfigurationUsePort(SERIAL_PORT_USART3)) {
            i2cInit(I2CDEV_2);
        }
    #else
            i2cInit(I2CDEV_2);
    #endif
#endif

#ifdef USE_I2C_DEVICE_3
    i2cInit(I2CDEV_3);
#endif

#ifdef USE_I2C_DEVICE_4
    i2cInit(I2CDEV_4);
#endif

#ifdef USE_I2C_DEVICE_EMULATED
    #ifdef I2C_DEVICE_EMULATED_SHARES_UART3
        if (!doesConfigurationUsePort(SERIAL_PORT_USART3)) {
            i2cInit(I2CDEV_EMULATED);
        }
    #else
            i2cInit(I2CDEV_EMULATED);
    #endif
#endif
#endif

#ifdef USE_ADC
    drv_adc_config_t adc_params;
    memset(&adc_params, 0, sizeof(adc_params));

    // Allocate and initialize ADC channels if features are configured - can't rely on sensor detection here, it's done later
    if (feature(FEATURE_VBAT)) {
        adc_params.adcFunctionChannel[ADC_BATTERY] = adcChannelConfig()->adcFunctionChannel[ADC_BATTERY];
    }

    if (feature(FEATURE_RSSI_ADC)) {
        adc_params.adcFunctionChannel[ADC_RSSI] = adcChannelConfig()->adcFunctionChannel[ADC_RSSI];
    }

    if (feature(FEATURE_CURRENT_METER) && batteryMetersConfig()->current.type == CURRENT_SENSOR_ADC) {
        adc_params.adcFunctionChannel[ADC_CURRENT] =  adcChannelConfig()->adcFunctionChannel[ADC_CURRENT];
    }

#if defined(USE_PITOT) && defined(USE_ADC) && defined(USE_PITOT_ADC)
    if (pitotmeterConfig()->pitot_hardware == PITOT_ADC || pitotmeterConfig()->pitot_hardware == PITOT_AUTODETECT) {
        adc_params.adcFunctionChannel[ADC_AIRSPEED] = adcChannelConfig()->adcFunctionChannel[ADC_AIRSPEED];
    }
#endif

    adcInit(&adc_params);
#endif

#ifdef USE_PINIO
    pinioInit();
#endif

#ifdef USE_PINIOBOX
    pinioBoxInit();
#endif

#if defined(USE_GPS) || defined(USE_MAG)
    delay(500);

    /* Extra 500ms delay prior to initialising hardware if board is cold-booting */
    if (!isMPUSoftReset()) {
        LED1_ON;
        LED0_OFF;

        for (int i = 0; i < 5; i++) {
            LED1_TOGGLE;
            LED0_TOGGLE;
            delay(100);
        }

        LED0_OFF;
        LED1_OFF;
    }
#endif

    initBoardAlignment();

#ifdef USE_CMS
    cmsInit();
#endif

#ifdef USE_DASHBOARD
    if (feature(FEATURE_DASHBOARD)) {
        dashboardInit();
    }
#endif

#ifdef USE_GPS
    if (feature(FEATURE_GPS)) {
        gpsPreInit();
    }
#endif

    // 1-Wire IF chip
#ifdef USE_1WIRE
    owInit();
#endif

#ifdef USE_EZ_TUNE
    ezTuneUpdate();
#endif

#ifndef USE_GEOZONE
    featureClear(FEATURE_GEOZONE);
#endif

    if (!sensorsAutodetect()) {
        // if gyro was not detected due to whatever reason, we give up now.
        failureMode(FAILURE_MISSING_ACC);
    }

    systemState |= SYSTEM_STATE_SENSORS_READY;

    flashLedsAndBeep();

    pidInitFilters();

    imuInit();

    // Sensors have now been detected, mspFcInit() can now be called
    // to set the boxes up
    mspFcInit();

    cliInit(serialConfig());

    failsafeInit();

    rxInit();

#if defined(USE_OSD)
    displayPort_t *osdDisplayPort = NULL;
#endif

#ifdef USE_OSD
    if (feature(FEATURE_OSD)) {
#if defined(USE_FRSKYOSD)
        if (!osdDisplayPort) {
            osdDisplayPort = frskyOSDDisplayPortInit(osdConfig()->video_system);
        }
#endif
#ifdef USE_MSP_OSD
        if (!osdDisplayPort) {
            osdDisplayPort = mspOsdDisplayPortInit(osdConfig()->video_system);
        }
#endif
#if defined(USE_MAX7456)
        // If there is a max7456 chip for the OSD and we have no
        // external OSD initialized, use it.
        if (!osdDisplayPort) {
            osdDisplayPort = max7456DisplayPortInit(osdConfig()->video_system);
        }
#elif defined(USE_OSD_OVER_MSP_DISPLAYPORT) // OSD over MSP; not supported (yet)
        if (!osdDisplayPort) {
            osdDisplayPort = displayPortMspInit();
        }
#endif
        // osdInit  will register with CMS by itself.
        osdInit(osdDisplayPort);
    }
#endif

#if defined(USE_CMS) && defined(USE_SPEKTRUM_CMS_TELEMETRY) && defined(USE_TELEMETRY_SRXL)
    // Register the srxl Textgen telemetry sensor as a displayport device
    cmsDisplayPortRegister(displayPortSrxlInit());
#endif

#ifdef USE_GPS
    if (feature(FEATURE_GPS)) {
        gpsInit();
    }
#endif


    navigationInit();

#ifdef USE_LED_STRIP
    ledStripInit();

    if (feature(FEATURE_LED_STRIP)) {
        ledStripEnable();
    }
#endif

#ifdef USE_TELEMETRY
    if (feature(FEATURE_TELEMETRY)) {
        telemetryInit();
    }
#endif

#ifdef USE_BLACKBOX

    //Do not allow blackbox to be run faster that 1kHz. It can cause UAV to drop dead when digital ESC protocol is used
    const uint32_t blackboxLooptime =  getLooptime() * blackboxConfig()->rate_denom / blackboxConfig()->rate_num;

    if (blackboxLooptime < 1000) {
        blackboxConfigMutable()->rate_num = 1;
        blackboxConfigMutable()->rate_denom = ceil(1000 / getLooptime());
    }

    // SDCARD and FLASHFS are used only for blackbox
    // Make sure we only init what's necessary for blackbox
    switch (blackboxConfig()->device) {
#ifdef USE_FLASHFS
        case BLACKBOX_DEVICE_FLASH:
            if (!flashDeviceInitialized) {
                flashDeviceInitialized = flashInit();
            }
            if (flashDeviceInitialized) {
                // do not initialize flashfs if no flash was found
                flashfsInit();
            }
            break;
#endif

#ifdef USE_SDCARD
        case BLACKBOX_DEVICE_SDCARD:
            sdcardInsertionDetectInit();
            sdcard_init();
            afatfs_init();
            break;
#endif
        default:
            break;
    }

    blackboxInit();
#endif

    gyroStartCalibration();

#ifdef USE_BARO
    baroStartCalibration();
#endif

#ifdef USE_PITOT
    pitotStartCalibration();
#endif

#if defined(USE_VTX_CONTROL)
    vtxControlInit();
    vtxCommonInit();
    vtxInit();

#ifdef USE_VTX_SMARTAUDIO
    vtxSmartAudioInit();
#endif

#ifdef USE_VTX_TRAMP
    vtxTrampInit();
#endif

#ifdef USE_VTX_FFPV
    vtxFuriousFPVInit();
#endif

#ifdef USE_VTX_MSP
    vtxMspInit();
#endif

#endif // USE_VTX_CONTROL

    // Now that everything has powered up the voltage and cell count be determined.
    if (feature(FEATURE_VBAT | FEATURE_CURRENT_METER))
        batteryInit();

#ifdef USE_RCDEVICE
    rcdeviceInit();
#endif // USE_RCDEVICE

#ifdef USE_DSHOT
    initDShotCommands();
#endif

#ifdef USE_SERIAL_GIMBAL
    gimbalCommonInit();
    // Needs to be called before gimbalSerialHeadTrackerInit
    gimbalSerialInit();
#endif

#ifdef USE_HEADTRACKER
    headTrackerCommonInit();
#ifdef USE_HEADTRACKER_SERIAL
    // Needs to be called after gimbalSerialInit
    gimbalSerialHeadTrackerInit();
#endif
#ifdef USE_HEADTRACKER_MSP
    mspHeadTrackerInit();
#endif
#endif

    // Latch active features AGAIN since some may be modified by init().
    latchActiveFeatures();
    motorControlEnable = true;

    fcTasksInit();

#ifdef USE_OSD
    if (feature(FEATURE_OSD) && (osdDisplayPort != NULL)) {
        setTaskEnabled(TASK_OSD, feature(FEATURE_OSD));
    }
#endif

#ifdef USE_RPM_FILTER
    disableRpmFilters();
    if (STATE(ESC_SENSOR_ENABLED) && (rpmFilterConfig()->gyro_filter_enabled || rpmFilterConfig()->dterm_filter_enabled)) {
        rpmFiltersInit();
        setTaskEnabled(TASK_RPM_FILTER, true);
    }
#endif

#ifdef USE_I2C_IO_EXPANDER
    ioPortExpanderInit();
#endif

#ifdef USE_POWER_LIMITS
    powerLimiterInit();
#endif

#if !defined(SITL_BUILD)
    // Considering that the persistent reset reason is only used during init
    persistentObjectWrite(PERSISTENT_OBJECT_RESET_REASON, RESET_NONE);
#endif

    systemState |= SYSTEM_STATE_READY;
}

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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "platform.h"

bool cliMode = false;

#include "blackbox/blackbox.h"

#include "build/assert.h"
#include "build/build_config.h"
#include "build/version.h"

#include "common/axis.h"
#include "common/color.h"
#include "common/maths.h"
#include "common/printf.h"
#include "common/string_light.h"
#include "common/memory.h"
#include "common/time.h"
#include "common/typeconversion.h"
#include "common/fp_pid.h"
#include "programming/global_variables.h"
#include "programming/pid.h"

#include "config/config_eeprom.h"
#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/accgyro/accgyro.h"
#include "drivers/pwm_mapping.h"
#include "drivers/buf_writer.h"
#include "drivers/bus_i2c.h"
#include "drivers/compass/compass.h"
#include "drivers/flash.h"
#include "drivers/io.h"
#include "drivers/io_impl.h"
#include "drivers/osd_symbols.h"
#include "drivers/persistent.h"
#include "drivers/sdcard/sdcard.h"
#include "drivers/sensor.h"
#include "drivers/serial.h"
#include "drivers/stack_check.h"
#include "drivers/system.h"
#include "drivers/time.h"
#include "drivers/usb_msc.h"
#include "drivers/vtx_common.h"
#include "drivers/light_ws2811strip.h"

#include "fc/fc_core.h"
#include "fc/cli.h"
#include "fc/config.h"
#include "fc/controlrate_profile.h"
#include "fc/rc_adjustments.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/failsafe.h"
#include "flight/imu.h"
#include "flight/mixer_profile.h"
#include "flight/pid.h"
#include "flight/servos.h"

#include "io/asyncfatfs/asyncfatfs.h"
#include "io/beeper.h"
#include "io/flashfs.h"
#include "io/gps.h"
#include "io/gps_ublox.h"
#include "io/ledstrip.h"
#include "io/osd.h"
#include "io/osd/custom_elements.h"
#include "io/serial.h"

#include "fc/fc_msp_box.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "rx/rx.h"
#include "rx/spektrum.h"
#include "rx/srxl2.h"
#include "rx/crsf.h"

#include "scheduler/scheduler.h"

#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/battery.h"
#include "sensors/boardalignment.h"
#include "sensors/compass.h"
#include "sensors/diagnostics.h"
#include "sensors/gyro.h"
#include "sensors/pitotmeter.h"
#include "sensors/rangefinder.h"
#include "sensors/opflow.h"
#include "sensors/sensors.h"
#include "sensors/temperature.h"
#ifdef USE_ESC_SENSOR
#include "sensors/esc_sensor.h"
#endif

#include "telemetry/telemetry.h"
#include "build/debug.h"

extern timeDelta_t cycleTime; // FIXME dependency on mw.c
extern uint8_t detectedSensors[SENSOR_INDEX_COUNT];

static serialPort_t *cliPort;

static bufWriter_t *cliWriter;
static uint8_t cliWriteBuffer[sizeof(*cliWriter) + 128];

static char cliBuffer[64];
static uint32_t bufferIndex = 0;
static uint16_t cliDelayMs = 0;

#if defined(USE_ASSERT)
static void cliAssert(char *cmdline);
#endif

#ifdef USE_CLI_BATCH
static bool     commandBatchActive = false;
static bool     commandBatchError = false;
static uint8_t  commandBatchErrorCount = 0;
#endif

// sync this with features_e
static const char * const featureNames[] = {
    "THR_VBAT_COMP", "VBAT", "TX_PROF_SEL", "BAT_PROF_AUTOSWITCH", "GEOZONE",
    "", "SOFTSERIAL", "GPS", "RPM_FILTERS",
    "", "TELEMETRY", "CURRENT_METER", "REVERSIBLE_MOTORS", "",
    "", "RSSI_ADC", "LED_STRIP", "DASHBOARD", "",
    "BLACKBOX", "", "TRANSPONDER", "AIRMODE",
    "SUPEREXPO", "VTX", "", "", "", "PWM_OUTPUT_ENABLE",
    "OSD", "FW_LAUNCH", "FW_AUTOTRIM", NULL
};

static const char * outputModeNames[] = {
    "AUTO",
    "MOTORS",
    "SERVOS",
    "LED",
    NULL
};

#ifdef USE_BLACKBOX
static const char * const blackboxIncludeFlagNames[] = {
    "NAV_ACC",
    "NAV_POS",
    "NAV_PID",
    "MAG",
    "ACC",
    "ATTI",
    "RC_DATA",
    "RC_COMMAND",
    "MOTORS",
    "GYRO_RAW",
    "PEAKS_R",
    "PEAKS_P",
    "PEAKS_Y",
    "SERVOS",
    NULL
};
#endif

static const char *debugModeNames[DEBUG_COUNT] = {
    "NONE",
    "AGL",
    "FLOW_RAW",
    "FLOW",
    "ALWAYS",
    "SAG_COMP_VOLTAGE",
    "VIBE",
    "CRUISE",
    "REM_FLIGHT_TIME",
    "SMARTAUDIO",
    "ACC",
    "NAV_YAW",
    "PCF8574",
    "DYN_GYRO_LPF",
    "AUTOLEVEL",
    "ALTITUDE",
    "AUTOTRIM",
    "AUTOTUNE",
    "RATE_DYNAMICS",
    "LANDING",
    "POS_EST",
    "ADAPTIVE_FILTER",
    "HEADTRACKER",
    "GPS",
    "LULU",
    "SBUS2"
};

/* Sensor names (used in lookup tables for *_hardware settings and in status
   command output) */
// sync with gyroSensor_e
static const char *const gyroNames[] = {
    "NONE",     "AUTO",   "MPU6000",  "MPU6500", "MPU9250", "BMI160",
    "ICM20689", "BMI088", "ICM42605", "BMI270",  "LSM6DXX", "FAKE"};

// sync this with sensors_e
static const char * const sensorTypeNames[] = {
    "GYRO", "ACC", "BARO", "MAG", "RANGEFINDER", "PITOT", "OPFLOW", "GPS", "GPS+MAG", NULL
};

#define SENSOR_NAMES_MASK (SENSOR_GYRO | SENSOR_ACC | SENSOR_BARO | SENSOR_MAG | SENSOR_RANGEFINDER | SENSOR_PITOT | SENSOR_OPFLOW)

static const char * const hardwareSensorStatusNames[] = {
    "NONE", "OK", "UNAVAILABLE", "FAILING"
};

static const char * const *sensorHardwareNames[] = {
        gyroNames,
        table_acc_hardware,
#ifdef USE_BARO
        table_baro_hardware,
#else
        NULL,
#endif
#ifdef USE_MAG
        table_mag_hardware,
#else
        NULL,
#endif
#ifdef USE_RANGEFINDER
        table_rangefinder_hardware,
#else
        NULL,
#endif
#ifdef USE_PITOT
        table_pitot_hardware,
#else
        NULL,
#endif
#ifdef USE_OPFLOW
        table_opflow_hardware,
#else
        NULL,
#endif
};

static void cliPrint(const char *str)
{
    while (*str) {
        bufWriterAppend(cliWriter, *str++);
    }
}

static void cliPrintLinefeed(void)
{
    cliPrint("\r\n");
    if (cliDelayMs) {
        delay(cliDelayMs);
    }
}

static void cliPrintLine(const char *str)
{
    cliPrint(str);
    cliPrintLinefeed();
}


static void cliPrintError(const char *str)
{
    cliPrint("### ERROR: ");
    cliPrint(str);
#ifdef USE_CLI_BATCH
    if (commandBatchActive) {
        commandBatchError = true;
        commandBatchErrorCount++;
    }
#endif
}

static void cliPrintErrorLine(const char *str)
{
    cliPrint("### ERROR: ");
    cliPrintLine(str);
#ifdef USE_CLI_BATCH
    if (commandBatchActive) {
        commandBatchError = true;
        commandBatchErrorCount++;
    }
#endif
}

#ifdef CLI_MINIMAL_VERBOSITY
#define cliPrintHashLine(str)
#else
static void cliPrintHashLine(const char *str)
{
    cliPrint("\r\n# ");
    cliPrintLine(str);
}
#endif

static void cliPutp(void *p, char ch)
{
    bufWriterAppend(p, ch);
}

typedef enum {
    DUMP_MASTER = (1 << 0),
    DUMP_CONTROL_PROFILE = (1 << 1),
    DUMP_BATTERY_PROFILE = (1 << 2),
    DUMP_MIXER_PROFILE = (1 << 3),
    DUMP_ALL = (1 << 4),
    DO_DIFF = (1 << 5),
    SHOW_DEFAULTS = (1 << 6),
    HIDE_UNUSED = (1 << 7)
} dumpFlags_e;

static void cliPrintfva(const char *format, va_list va)
{
    tfp_format(cliWriter, cliPutp, format, va);
    bufWriterFlush(cliWriter);
}

static void cliPrintLinefva(const char *format, va_list va)
{
    tfp_format(cliWriter, cliPutp, format, va);
    bufWriterFlush(cliWriter);
    cliPrintLinefeed();
}

static bool cliDumpPrintLinef(uint8_t dumpMask, bool equalsDefault, const char *format, ...)
{
    if (!((dumpMask & DO_DIFF) && equalsDefault)) {
        va_list va;
        va_start(va, format);
        cliPrintLinefva(format, va);
        va_end(va);
        return true;
    } else {
        return false;
    }
}

static void cliWrite(uint8_t ch)
{
    bufWriterAppend(cliWriter, ch);
}

static bool cliDefaultPrintLinef(uint8_t dumpMask, bool equalsDefault, const char *format, ...)
{
    if ((dumpMask & SHOW_DEFAULTS) && !equalsDefault) {
        cliWrite('#');

        va_list va;
        va_start(va, format);
        cliPrintLinefva(format, va);
        va_end(va);
        return true;
    } else {
        return false;
    }
}

static void cliPrintf(const char *format, ...)
{
    va_list va;
    va_start(va, format);
    cliPrintfva(format, va);
    va_end(va);
}


static void cliPrintLinef(const char *format, ...)
{
    va_list va;
    va_start(va, format);
    cliPrintLinefva(format, va);
    va_end(va);
}

static void cliPrintErrorVa(const char *format, va_list va)
{
    cliPrint("### ERROR: ");
    cliPrintfva(format, va);
    va_end(va);

#ifdef USE_CLI_BATCH
    if (commandBatchActive) {
        commandBatchError = true;
        commandBatchErrorCount++;
    }
#endif
}

static void cliPrintErrorLinef(const char *format, ...)
{
    va_list va;
    va_start(va, format);
    cliPrintErrorVa(format, va);
    cliPrintLinefeed();
}

static void printValuePointer(const setting_t *var, const void *valuePointer, uint32_t full)
{
    int32_t value = 0;
    char buf[SETTING_MAX_NAME_LENGTH];

    switch (SETTING_TYPE(var)) {
    case VAR_UINT8:
        value = *(uint8_t *)valuePointer;
        break;

    case VAR_INT8:
        value = *(int8_t *)valuePointer;
        break;

    case VAR_UINT16:
        value = *(uint16_t *)valuePointer;
        break;

    case VAR_INT16:
        value = *(int16_t *)valuePointer;
        break;

    case VAR_UINT32:
        value = *(uint32_t *)valuePointer;
        break;

    case VAR_FLOAT:
        cliPrintf("%s", ftoa(*(float *)valuePointer, buf));
        if (full) {
            if (SETTING_MODE(var) == MODE_DIRECT) {
                cliPrintf(" %s", ftoa((float)settingGetMin(var), buf));
                cliPrintf(" %s", ftoa((float)settingGetMax(var), buf));
            }
        }
        return; // return from case for float only

    case VAR_STRING:
        cliPrintf("%s", (const char *)valuePointer);
        return;
    }

    switch (SETTING_MODE(var)) {
    case MODE_DIRECT:
        if (SETTING_TYPE(var) == VAR_UINT32)
            cliPrintf("%u", value);
        else
            cliPrintf("%d", value);
        if (full) {
            if (SETTING_MODE(var) == MODE_DIRECT) {
                cliPrintf(" %d %u", settingGetMin(var), settingGetMax(var));
            }
        }
        break;
    case MODE_LOOKUP:
    {
        const char *name = settingLookupValueName(var, value);
        if (name) {
            cliPrintf(name);
        } else {
            settingGetName(var, buf);
            cliPrintErrorLinef("VALUE %d OUT OF RANGE FOR %s", (int)value, buf);
        }
        break;
    }
    }
}

static bool valuePtrEqualsDefault(const setting_t *value, const void *ptr, const void *ptrDefault)
{
    bool result = false;
    switch (SETTING_TYPE(value)) {
    case VAR_UINT8:
        result = *(uint8_t *)ptr == *(uint8_t *)ptrDefault;
        break;

    case VAR_INT8:
        result = *(int8_t *)ptr == *(int8_t *)ptrDefault;
        break;

    case VAR_UINT16:
        result = *(uint16_t *)ptr == *(uint16_t *)ptrDefault;
        break;

    case VAR_INT16:
        result = *(int16_t *)ptr == *(int16_t *)ptrDefault;
        break;

    case VAR_UINT32:
        result = *(uint32_t *)ptr == *(uint32_t *)ptrDefault;
        break;

    case VAR_FLOAT:
        result = *(float *)ptr == *(float *)ptrDefault;
        break;

    case VAR_STRING:
        result = strncmp(ptr, ptrDefault, settingGetStringMaxLength(value) + 1) == 0;
        break;
    }
    return result;
}

static void dumpPgValue(const setting_t *value, uint8_t dumpMask)
{
    char name[SETTING_MAX_NAME_LENGTH];
    const char *format = "set %s = ";
    const char *defaultFormat = "#set %s = ";
    // During a dump, the PGs have been backed up to their "copy"
    // regions and the actual values have been reset to its
    // defaults. This means that settingGetValuePointer() will
    // return the default value while settingGetCopyValuePointer()
    // will return the actual value.
    const void *valuePointer = settingGetCopyValuePointer(value);
    const void *defaultValuePointer = settingGetValuePointer(value);
    const bool equalsDefault = valuePtrEqualsDefault(value, valuePointer, defaultValuePointer);
    if (((dumpMask & DO_DIFF) == 0) || !equalsDefault) {
        settingGetName(value, name);
        if (dumpMask & SHOW_DEFAULTS && !equalsDefault) {
            cliPrintf(defaultFormat, name);
            // if the craftname has a leading space, then enclose the name in quotes
            if (strcmp(name, "name") == 0 && ((const char *)valuePointer)[0] == ' ') {
                cliPrintf("\"%s\"", (const char *)valuePointer);
            } else {
                printValuePointer(value, valuePointer, 0);
            }
            cliPrintLinefeed();
        }
        cliPrintf(format, name);
        printValuePointer(value, valuePointer, 0);
        cliPrintLinefeed();
    }
}

static void dumpAllValues(uint16_t valueSection, uint8_t dumpMask)
{
    for (unsigned i = 0; i < SETTINGS_TABLE_COUNT; i++) {
        const setting_t *value = settingGet(i);
        bufWriterFlush(cliWriter);
        if (SETTING_SECTION(value) == valueSection) {
            dumpPgValue(value, dumpMask);
        }
    }
}

static void cliPrintVar(const setting_t *var, uint32_t full)
{
    const void *ptr = settingGetValuePointer(var);

    printValuePointer(var, ptr, full);
}

static void cliPrintVarRange(const setting_t *var)
{
    switch (SETTING_MODE(var)) {
    case MODE_DIRECT:
        if (SETTING_TYPE(var) == VAR_STRING) {
           cliPrintLinef("Max. length: %u", settingGetStringMaxLength(var));
           break;
        }
        cliPrintLinef("Allowed range: %d - %u", settingGetMin(var), settingGetMax(var));
        break;
    case MODE_LOOKUP:
    {
        const lookupTableEntry_t *tableEntry = settingLookupTable(var);
        cliPrint("Allowed values:");
        for (uint32_t i = 0; i < tableEntry->valueCount ; i++) {
            if (i > 0)
                cliPrint(",");
            cliPrintf(" %s", tableEntry->values[i]);
        }
        cliPrintLinefeed();
    }
        break;
    }
}

typedef union {
    uint32_t uint_value;
    int32_t int_value;
    float float_value;
} int_float_value_t;

static void cliSetIntFloatVar(const setting_t *var, const int_float_value_t value)
{
    void *ptr = settingGetValuePointer(var);

    switch (SETTING_TYPE(var)) {
    case VAR_UINT8:
    case VAR_INT8:
        *(int8_t *)ptr = value.int_value;
        break;

    case VAR_UINT16:
    case VAR_INT16:
        *(int16_t *)ptr = value.int_value;
        break;

    case VAR_UINT32:
        *(uint32_t *)ptr = value.uint_value;
        break;

    case VAR_FLOAT:
        *(float *)ptr = (float)value.float_value;
        break;

    case VAR_STRING:
        // Handled by cliSet directly
        break;
    }
}

static void cliPrompt(void)
{
    cliPrint("\r\n# ");
    bufWriterFlush(cliWriter);
}

static void cliShowParseError(void)
{
    cliPrintErrorLinef("Parse error");
}

static void cliShowArgumentRangeError(char *name, int min, int max)
{
    cliPrintErrorLinef("%s must be between %d and %d", name, min, max);
}

static const char *nextArg(const char *currentArg)
{
    const char *ptr = strchr(currentArg, ' ');
    while (ptr && *ptr == ' ') {
        ptr++;
    }

    return ptr;
}

static const char *processChannelRangeArgs(const char *ptr, channelRange_t *range, uint8_t *validArgumentCount)
{
    for (uint32_t argIndex = 0; argIndex < 2; argIndex++) {
        ptr = nextArg(ptr);
        if (ptr) {
            int val = fastA2I(ptr);
            val = CHANNEL_VALUE_TO_STEP(val);
            if (val >= MIN_MODE_RANGE_STEP && val <= MAX_MODE_RANGE_STEP) {
                if (argIndex == 0) {
                    range->startStep = val;
                } else {
                    range->endStep = val;
                }
                (*validArgumentCount)++;
            }
        }
    }

    return ptr;
}

// Check if a string's length is zero
static bool isEmpty(const char *string)
{
    return (string == NULL || *string == '\0') ? true : false;
}

#if defined(USE_ASSERT)
static void cliAssert(char *cmdline)
{
    UNUSED(cmdline);

    if (assertFailureLine) {
        if (assertFailureFile) {
            cliPrintErrorLinef("Assertion failed at line %d, file %s", assertFailureLine, assertFailureFile);
        }
        else {
            cliPrintErrorLinef("Assertion failed at line %d", assertFailureLine);
        }
#ifdef USE_CLI_BATCH
        if (commandBatchActive) {
            commandBatchError = true;
            commandBatchErrorCount++;
        }
#endif
    }
    else {
        cliPrintLine("No assert() failed");
    }
}
#endif

static void printAux(uint8_t dumpMask, const modeActivationCondition_t *modeActivationConditions, const modeActivationCondition_t *defaultModeActivationConditions)
{
    const char *format = "aux %u %u %u %u %u";
    // print out aux channel settings
    for (uint32_t i = 0; i < MAX_MODE_ACTIVATION_CONDITION_COUNT; i++) {
        const modeActivationCondition_t *mac = &modeActivationConditions[i];
        bool equalsDefault = false;
        if (defaultModeActivationConditions) {
            const modeActivationCondition_t *macDefault = &defaultModeActivationConditions[i];
            equalsDefault = mac->modeId == macDefault->modeId
                && mac->auxChannelIndex == macDefault->auxChannelIndex
                && mac->range.startStep == macDefault->range.startStep
                && mac->range.endStep == macDefault->range.endStep;
            const box_t *box = findBoxByActiveBoxId(macDefault->modeId);
            cliDefaultPrintLinef(dumpMask, equalsDefault, format,
                i,
                box->permanentId,
                macDefault->auxChannelIndex,
                MODE_STEP_TO_CHANNEL_VALUE(macDefault->range.startStep),
                MODE_STEP_TO_CHANNEL_VALUE(macDefault->range.endStep)
            );
        }
        const box_t *box = findBoxByActiveBoxId(mac->modeId);
        cliDumpPrintLinef(dumpMask, equalsDefault, format,
            i,
            box->permanentId,
            mac->auxChannelIndex,
            MODE_STEP_TO_CHANNEL_VALUE(mac->range.startStep),
            MODE_STEP_TO_CHANNEL_VALUE(mac->range.endStep)
        );
    }
}

static void cliAux(char *cmdline)
{
    int i, val = 0;
    const char *ptr;

    if (isEmpty(cmdline)) {
        printAux(DUMP_MASTER, modeActivationConditions(0), NULL);
    } else {
        ptr = cmdline;
        i = fastA2I(ptr++);
        if (i < MAX_MODE_ACTIVATION_CONDITION_COUNT) {
            modeActivationCondition_t *mac = modeActivationConditionsMutable(i);
            uint8_t validArgumentCount = 0;
            ptr = nextArg(ptr);
            if (ptr) {
                val = fastA2I(ptr);
                if (val >= 0) {
                    const box_t *box = findBoxByPermanentId(val);
                    if (box != NULL) {
                        mac->modeId = box->boxId;
                        validArgumentCount++;
                    }
                }
            }
            ptr = nextArg(ptr);
            if (ptr) {
                val = fastA2I(ptr);
                if (val >= 0 && val < MAX_AUX_CHANNEL_COUNT) {
                    mac->auxChannelIndex = val;
                    validArgumentCount++;
                }
            }
            ptr = processChannelRangeArgs(ptr, &mac->range, &validArgumentCount);

            if (validArgumentCount != 4) {
                memset(mac, 0, sizeof(modeActivationCondition_t));
            }
        } else {
            cliShowArgumentRangeError("index", 0, MAX_MODE_ACTIVATION_CONDITION_COUNT - 1);
        }
    }
}

static void printSerial(uint8_t dumpMask, const serialConfig_t *serialConfig, const serialConfig_t *serialConfigDefault)
{
    const char *format = "serial %d %d %ld %ld %ld %ld";
    for (uint32_t i = 0; i < SERIAL_PORT_COUNT; i++) {
        if (!serialIsPortAvailable(serialConfig->portConfigs[i].identifier)) {
            continue;
        };
        bool equalsDefault = false;
        if (serialConfigDefault) {
            equalsDefault = serialConfig->portConfigs[i].identifier == serialConfigDefault->portConfigs[i].identifier
                && serialConfig->portConfigs[i].functionMask == serialConfigDefault->portConfigs[i].functionMask
                && serialConfig->portConfigs[i].msp_baudrateIndex == serialConfigDefault->portConfigs[i].msp_baudrateIndex
                && serialConfig->portConfigs[i].gps_baudrateIndex == serialConfigDefault->portConfigs[i].gps_baudrateIndex
                && serialConfig->portConfigs[i].telemetry_baudrateIndex == serialConfigDefault->portConfigs[i].telemetry_baudrateIndex
                && serialConfig->portConfigs[i].peripheral_baudrateIndex == serialConfigDefault->portConfigs[i].peripheral_baudrateIndex;
            cliDefaultPrintLinef(dumpMask, equalsDefault, format,
                serialConfigDefault->portConfigs[i].identifier,
                serialConfigDefault->portConfigs[i].functionMask,
                baudRates[serialConfigDefault->portConfigs[i].msp_baudrateIndex],
                baudRates[serialConfigDefault->portConfigs[i].gps_baudrateIndex],
                baudRates[serialConfigDefault->portConfigs[i].telemetry_baudrateIndex],
                baudRates[serialConfigDefault->portConfigs[i].peripheral_baudrateIndex]
            );
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format,
            serialConfig->portConfigs[i].identifier,
            serialConfig->portConfigs[i].functionMask,
            baudRates[serialConfig->portConfigs[i].msp_baudrateIndex],
            baudRates[serialConfig->portConfigs[i].gps_baudrateIndex],
            baudRates[serialConfig->portConfigs[i].telemetry_baudrateIndex],
            baudRates[serialConfig->portConfigs[i].peripheral_baudrateIndex]
            );
    }
}

static void cliSerial(char *cmdline)
{
    if (isEmpty(cmdline)) {
        printSerial(DUMP_MASTER, serialConfig(), NULL);
        return;
    }
    serialPortConfig_t portConfig;

    serialPortConfig_t *currentConfig;

    uint8_t validArgumentCount = 0;

    const char *ptr = cmdline;

    int val = fastA2I(ptr++);
    currentConfig = serialFindPortConfiguration(val);
    if (!currentConfig) {
        // Invalid port ID
        cliPrintErrorLinef("Invalid port ID %d", val);
        return;
    }
    memcpy(&portConfig, currentConfig, sizeof(portConfig));
    validArgumentCount++;

    ptr = nextArg(ptr);
    if (ptr) {
        switch (*ptr) {
            case '+':
                // Add function
                ptr++;
                val = fastA2I(ptr);
                portConfig.functionMask |= (1 << val);
                break;
            case '-':
                // Remove function
                ptr++;
                val = fastA2I(ptr);
                portConfig.functionMask &= 0xFFFFFFFF ^ (1 << val);
                break;
            default:
                // Set functions
                val = fastA2I(ptr);
                portConfig.functionMask = val & 0xFFFFFFFF;
                break;
        }
        validArgumentCount++;
    }

    for (int i = 0; i < 4; i ++) {
        ptr = nextArg(ptr);
        if (!ptr) {
            break;
        }

        val = fastA2I(ptr);

        uint8_t baudRateIndex = lookupBaudRateIndex(val);
        if (baudRates[baudRateIndex] != (uint32_t) val) {
            break;
        }

        switch (i) {
        case 0:
            baudRateIndex = constrain(baudRateIndex, BAUD_MIN, BAUD_MAX);
            portConfig.msp_baudrateIndex = baudRateIndex;
            break;
        case 1:
            baudRateIndex = constrain(baudRateIndex, BAUD_MIN, BAUD_MAX);
            portConfig.gps_baudrateIndex = baudRateIndex;
            break;
        case 2:
            baudRateIndex = constrain(baudRateIndex, BAUD_MIN, BAUD_MAX);
            portConfig.telemetry_baudrateIndex = baudRateIndex;
            break;
        case 3:
            baudRateIndex = constrain(baudRateIndex, BAUD_MIN, BAUD_MAX);
            portConfig.peripheral_baudrateIndex = baudRateIndex;
            break;
        }

        validArgumentCount++;
    }

    if (validArgumentCount < 2) {
        cliShowParseError();
        return;
    }

    memcpy(currentConfig, &portConfig, sizeof(portConfig));
}

#ifdef USE_SERIAL_PASSTHROUGH

portOptions_t constructPortOptions(char *options) {
    if (strlen(options) != 3 || options[0] != '8') {
        // Invalid format
        return -1;
    }

    portOptions_t result = 0;

    switch (options[1]) {
        case 'N':
            result |= SERIAL_PARITY_NO;
            break;
        case 'E':
            result |= SERIAL_PARITY_EVEN;
            break;
        default:
            // Invalid format
            return -1;
    }

    switch (options[2]) {
        case '1':
            result |= SERIAL_STOPBITS_1;
            break;
        case '2':
            result |= SERIAL_STOPBITS_2;
            break;
        default:
            // Invalid format
            return -1;
    }

    return result;
}

static void cliSerialPassthrough(char *cmdline)
{
    char * saveptr;

    if (isEmpty(cmdline)) {
        cliShowParseError();
        return;
    }

    int id = -1;
    uint32_t baud = 0;
    unsigned mode = 0;
    portOptions_t options = SERIAL_NOT_INVERTED;
    char* tok = strtok_r(cmdline, " ", &saveptr);
    int index = 0;

    while (tok != NULL) {
        switch (index) {
            case 0:
                id = fastA2I(tok);
                break;
            case 1:
                baud = fastA2I(tok);
                break;
            case 2:
                if (strstr(tok, "rx") || strstr(tok, "RX"))
                    mode |= MODE_RX;
                if (strstr(tok, "tx") || strstr(tok, "TX"))
                    mode |= MODE_TX;
                break;
            case 3:
                options |= constructPortOptions(tok);
                break;
        }
        index++;
        tok = strtok_r(NULL, " ", &saveptr);
    }

    serialPort_t *passThroughPort;
    serialPortUsage_t *passThroughPortUsage = findSerialPortUsageByIdentifier(id);
    if (!passThroughPortUsage || passThroughPortUsage->serialPort == NULL) {
        if (!baud) {
            tfp_printf("Port %d is closed, must specify baud.\r\n", id);
            return;
        }
        if (!mode)
            mode = MODE_RXTX;

        passThroughPort = openSerialPort(id, FUNCTION_NONE, NULL, NULL,
                                         baud, mode,
                                         options);
        if (!passThroughPort) {
            tfp_printf("Port %d could not be opened.\r\n", id);
            return;
        }
        tfp_printf("Port %d opened, baud = %u.\r\n", id, (unsigned)baud);
    } else {
        passThroughPort = passThroughPortUsage->serialPort;
        // If the user supplied a mode, override the port's mode, otherwise
        // leave the mode unchanged. serialPassthrough() handles one-way ports.
        tfp_printf("Port %d already open.\r\n", id);
        if (mode && passThroughPort->mode != mode) {
            tfp_printf("Adjusting mode from %d to %d.\r\n",
                   passThroughPort->mode, mode);
            serialSetMode(passThroughPort, mode);
        }
        if (options && passThroughPort->options != options) {
            tfp_printf("Adjusting options from %d to %d.\r\n",
                   passThroughPort->options, options);
            serialSetOptions(passThroughPort, options);
        }
        // If this port has a rx callback associated we need to remove it now.
        // Otherwise no data will be pushed in the serial port buffer!
        if (passThroughPort->rxCallback) {
            tfp_printf("Removing rxCallback\r\n");
            passThroughPort->rxCallback = 0;
        }
    }

    tfp_printf("Forwarding data to %d, power cycle to exit.\r\n", id);

    serialPassthrough(cliPort, passThroughPort, NULL, NULL);
}
#endif

static void printAdjustmentRange(uint8_t dumpMask, const adjustmentRange_t *adjustmentRanges, const adjustmentRange_t *defaultAdjustmentRanges)
{
    const char *format = "adjrange %u %u %u %u %u %u %u";
    // print out adjustment ranges channel settings
    for (uint32_t i = 0; i < MAX_ADJUSTMENT_RANGE_COUNT; i++) {
        const adjustmentRange_t *ar = &adjustmentRanges[i];
        bool equalsDefault = false;
        if (defaultAdjustmentRanges) {
            const adjustmentRange_t *arDefault = &defaultAdjustmentRanges[i];
            equalsDefault = ar->auxChannelIndex == arDefault->auxChannelIndex
                && ar->range.startStep == arDefault->range.startStep
                && ar->range.endStep == arDefault->range.endStep
                && ar->adjustmentFunction == arDefault->adjustmentFunction
                && ar->auxSwitchChannelIndex == arDefault->auxSwitchChannelIndex
                && ar->adjustmentIndex == arDefault->adjustmentIndex;
            cliDefaultPrintLinef(dumpMask, equalsDefault, format,
                i,
                arDefault->adjustmentIndex,
                arDefault->auxChannelIndex,
                MODE_STEP_TO_CHANNEL_VALUE(arDefault->range.startStep),
                MODE_STEP_TO_CHANNEL_VALUE(arDefault->range.endStep),
                arDefault->adjustmentFunction,
                arDefault->auxSwitchChannelIndex
            );
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format,
            i,
            ar->adjustmentIndex,
            ar->auxChannelIndex,
            MODE_STEP_TO_CHANNEL_VALUE(ar->range.startStep),
            MODE_STEP_TO_CHANNEL_VALUE(ar->range.endStep),
            ar->adjustmentFunction,
            ar->auxSwitchChannelIndex
        );
    }
}

static void cliAdjustmentRange(char *cmdline)
{
    int i, val = 0;
    const char *ptr;

    if (isEmpty(cmdline)) {
        printAdjustmentRange(DUMP_MASTER, adjustmentRanges(0), NULL);
    } else {
        ptr = cmdline;
        i = fastA2I(ptr++);
        if (i < MAX_ADJUSTMENT_RANGE_COUNT) {
            adjustmentRange_t *ar = adjustmentRangesMutable(i);
            uint8_t validArgumentCount = 0;

            ptr = nextArg(ptr);
            if (ptr) {
                val = fastA2I(ptr);
                if (val >= 0 && val < MAX_SIMULTANEOUS_ADJUSTMENT_COUNT) {
                    ar->adjustmentIndex = val;
                    validArgumentCount++;
                }
            }
            ptr = nextArg(ptr);
            if (ptr) {
                val = fastA2I(ptr);
                if (val >= 0 && val < MAX_AUX_CHANNEL_COUNT) {
                    ar->auxChannelIndex = val;
                    validArgumentCount++;
                }
            }

            ptr = processChannelRangeArgs(ptr, &ar->range, &validArgumentCount);

            ptr = nextArg(ptr);
            if (ptr) {
                val = fastA2I(ptr);
                if (val >= 0 && val < ADJUSTMENT_FUNCTION_COUNT) {
                    ar->adjustmentFunction = val;
                    validArgumentCount++;
                }
            }
            ptr = nextArg(ptr);
            if (ptr) {
                val = fastA2I(ptr);
                if (val >= 0 && val < MAX_AUX_CHANNEL_COUNT) {
                    ar->auxSwitchChannelIndex = val;
                    validArgumentCount++;
                }
            }

            if (validArgumentCount != 6) {
                memset(ar, 0, sizeof(adjustmentRange_t));
                cliShowParseError();
            }
        } else {
            cliShowArgumentRangeError("index", 0, MAX_ADJUSTMENT_RANGE_COUNT - 1);
        }
    }
}

static void printMotorMix(uint8_t dumpMask, const motorMixer_t *primaryMotorMixer, const motorMixer_t *defaultprimaryMotorMixer)
{
    const char *format = "mmix %d %s %s %s %s";
    char buf0[FTOA_BUFFER_SIZE];
    char buf1[FTOA_BUFFER_SIZE];
    char buf2[FTOA_BUFFER_SIZE];
    char buf3[FTOA_BUFFER_SIZE];
    for (uint32_t i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
        if (primaryMotorMixer[i].throttle == 0.0f)
            break;
        const float thr = primaryMotorMixer[i].throttle;
        const float roll = primaryMotorMixer[i].roll;
        const float pitch = primaryMotorMixer[i].pitch;
        const float yaw = primaryMotorMixer[i].yaw;
        bool equalsDefault = false;
        if (defaultprimaryMotorMixer) {
            const float thrDefault = defaultprimaryMotorMixer[i].throttle;
            const float rollDefault = defaultprimaryMotorMixer[i].roll;
            const float pitchDefault = defaultprimaryMotorMixer[i].pitch;
            const float yawDefault = defaultprimaryMotorMixer[i].yaw;
            const bool equalsDefault = thr == thrDefault && roll == rollDefault && pitch == pitchDefault && yaw == yawDefault;

            cliDefaultPrintLinef(dumpMask, equalsDefault, format,
                i,
                ftoa(thrDefault, buf0),
                ftoa(rollDefault, buf1),
                ftoa(pitchDefault, buf2),
                ftoa(yawDefault, buf3));
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format,
            i,
            ftoa(thr, buf0),
            ftoa(roll, buf1),
            ftoa(pitch, buf2),
            ftoa(yaw, buf3));
    }
}

static void cliMotorMix(char *cmdline)
{
    int check = 0;
    const char *ptr;

    if (isEmpty(cmdline)) {
        printMotorMix(DUMP_MASTER, primaryMotorMixer(0), NULL);
    } else if (sl_strncasecmp(cmdline, "reset", 5) == 0) {
        // erase custom mixer
        for (uint32_t i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
            primaryMotorMixerMutable(i)->throttle = 0.0f;
        }
    } else {
        ptr = cmdline;
        uint32_t i = fastA2I(ptr); // get motor number
        if (i < MAX_SUPPORTED_MOTORS) {
            ptr = nextArg(ptr);
            if (ptr) {
                primaryMotorMixerMutable(i)->throttle = fastA2F(ptr);
                check++;
            }
            ptr = nextArg(ptr);
            if (ptr) {
                primaryMotorMixerMutable(i)->roll = fastA2F(ptr);
                check++;
            }
            ptr = nextArg(ptr);
            if (ptr) {
                primaryMotorMixerMutable(i)->pitch = fastA2F(ptr);
                check++;
            }
            ptr = nextArg(ptr);
            if (ptr) {
                primaryMotorMixerMutable(i)->yaw = fastA2F(ptr);
                check++;
            }
            if (check != 4) {
                cliShowParseError();
            } else {
                printMotorMix(DUMP_MASTER, primaryMotorMixer(0), NULL);
            }
        } else {
            cliShowArgumentRangeError("index", 0, MAX_SUPPORTED_MOTORS - 1);
        }
    }
}

static void printRxRange(uint8_t dumpMask, const rxChannelRangeConfig_t *channelRangeConfigs, const rxChannelRangeConfig_t *defaultChannelRangeConfigs)
{
    const char *format = "rxrange %u %u %u";
    for (uint32_t i = 0; i < NON_AUX_CHANNEL_COUNT; i++) {
        bool equalsDefault = false;
        if (defaultChannelRangeConfigs) {
            equalsDefault = channelRangeConfigs[i].min == defaultChannelRangeConfigs[i].min
                && channelRangeConfigs[i].max == defaultChannelRangeConfigs[i].max;
            cliDefaultPrintLinef(dumpMask, equalsDefault, format,
                i,
                defaultChannelRangeConfigs[i].min,
                defaultChannelRangeConfigs[i].max
            );
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format,
            i,
            channelRangeConfigs[i].min,
            channelRangeConfigs[i].max
        );
    }
}

static void cliRxRange(char *cmdline)
{
    int i, validArgumentCount = 0;
    const char *ptr;

    if (isEmpty(cmdline)) {
        printRxRange(DUMP_MASTER, rxChannelRangeConfigs(0), NULL);
    } else if (sl_strcasecmp(cmdline, "reset") == 0) {
        resetAllRxChannelRangeConfigurations();
    } else {
        ptr = cmdline;
        i = fastA2I(ptr);
        if (i >= 0 && i < NON_AUX_CHANNEL_COUNT) {
            int rangeMin = 0, rangeMax = 0;

            ptr = nextArg(ptr);
            if (ptr) {
                rangeMin = fastA2I(ptr);
                validArgumentCount++;
            }

            ptr = nextArg(ptr);
            if (ptr) {
                rangeMax = fastA2I(ptr);
                validArgumentCount++;
            }

            if (validArgumentCount != 2) {
                cliShowParseError();
            } else if (rangeMin < PWM_PULSE_MIN || rangeMin > PWM_PULSE_MAX || rangeMax < PWM_PULSE_MIN || rangeMax > PWM_PULSE_MAX) {
                cliShowParseError();
            } else {
                rxChannelRangeConfig_t *channelRangeConfig = rxChannelRangeConfigsMutable(i);
                channelRangeConfig->min = rangeMin;
                channelRangeConfig->max = rangeMax;
            }
        } else {
            cliShowArgumentRangeError("channel", 0, NON_AUX_CHANNEL_COUNT - 1);
        }
    }
}

#ifdef USE_TEMPERATURE_SENSOR
static void printTempSensor(uint8_t dumpMask, const tempSensorConfig_t *tempSensorConfigs, const tempSensorConfig_t *defaultTempSensorConfigs)
{
    const char *format = "temp_sensor %u %u %s %d %d %u %s";
    for (uint8_t i = 0; i < MAX_TEMP_SENSORS; i++) {
        bool equalsDefault = false;
        char label[5], hex_address[17];
        strncpy(label, tempSensorConfigs[i].label, TEMPERATURE_LABEL_LEN);
        label[4] = '\0';
        tempSensorAddressToString(tempSensorConfigs[i].address, hex_address);
        if (defaultTempSensorConfigs) {
            equalsDefault = tempSensorConfigs[i].type == defaultTempSensorConfigs[i].type
                && tempSensorConfigs[i].address == defaultTempSensorConfigs[i].address
                && tempSensorConfigs[i].osdSymbol == defaultTempSensorConfigs[i].osdSymbol
                && !memcmp(tempSensorConfigs[i].label, defaultTempSensorConfigs[i].label, TEMPERATURE_LABEL_LEN)
                && tempSensorConfigs[i].alarm_min == defaultTempSensorConfigs[i].alarm_min
                && tempSensorConfigs[i].alarm_max == defaultTempSensorConfigs[i].alarm_max;
            cliDefaultPrintLinef(dumpMask, equalsDefault, format,
                i,
                defaultTempSensorConfigs[i].type,
                "0",
                defaultTempSensorConfigs[i].alarm_min,
                defaultTempSensorConfigs[i].alarm_max,
                0,
                ""
            );
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format,
            i,
            tempSensorConfigs[i].type,
            hex_address,
            tempSensorConfigs[i].alarm_min,
            tempSensorConfigs[i].alarm_max,
            tempSensorConfigs[i].osdSymbol,
            label
        );
    }
}

static void cliTempSensor(char *cmdline)
{
    if (isEmpty(cmdline)) {
        printTempSensor(DUMP_MASTER, tempSensorConfig(0), NULL);
    } else if (sl_strcasecmp(cmdline, "reset") == 0) {
        resetTempSensorConfig();
    } else {
        int16_t i;
        const char *ptr = cmdline, *label;
        int16_t type=0, alarm_min=0, alarm_max=0;
        bool addressValid = false;
        uint64_t address;
        int8_t osdSymbol=0;
        uint8_t validArgumentCount = 0;
        i = fastA2I(ptr);
        if (i >= 0 && i < MAX_TEMP_SENSORS) {

            ptr = nextArg(ptr);
            if (ptr) {
                type = fastA2I(ptr);
                validArgumentCount++;
            }

            ptr = nextArg(ptr);
            if (ptr) {
                addressValid = tempSensorStringToAddress(ptr, &address);
                validArgumentCount++;
            }

            ptr = nextArg(ptr);
            if (ptr) {
                alarm_min = fastA2I(ptr);
                validArgumentCount++;
            }

            ptr = nextArg(ptr);
            if (ptr) {
                alarm_max = fastA2I(ptr);
                validArgumentCount++;
            }

            ptr = nextArg(ptr);
            if (ptr) {
                osdSymbol = fastA2I(ptr);
                validArgumentCount++;
            }

            label = nextArg(ptr);
            if (label)
                ++validArgumentCount;
            else
                label = "";

            if (validArgumentCount < 4) {
                cliShowParseError();
            } else if (type < 0 || type > TEMP_SENSOR_DS18B20 || alarm_min < -550 || alarm_min > 1250 || alarm_max < -550 || alarm_max > 1250 || osdSymbol < 0 || osdSymbol > TEMP_SENSOR_SYM_COUNT || strlen(label) > TEMPERATURE_LABEL_LEN || !addressValid) {
                cliShowParseError();
            } else {
                tempSensorConfig_t *sensorConfig = tempSensorConfigMutable(i);
                sensorConfig->type = type;
                sensorConfig->address = address;
                sensorConfig->alarm_min = alarm_min;
                sensorConfig->alarm_max = alarm_max;
                sensorConfig->osdSymbol = osdSymbol;
                for (uint8_t index = 0; index < TEMPERATURE_LABEL_LEN; ++index) {
                    sensorConfig->label[index] = toupper(label[index]);
                    if (label[index] == '\0') break;
                }
            }
        } else {
            cliShowArgumentRangeError("sensor index", 0, MAX_TEMP_SENSORS - 1);
        }
    }
}
#endif

#ifdef USE_FW_AUTOLAND
static void printFwAutolandApproach(uint8_t dumpMask, const navFwAutolandApproach_t *navFwAutolandApproach, const navFwAutolandApproach_t *defaultFwAutolandApproach)
{
    const char *format = "fwapproach %u %d %d %u %d %d %u";
    for (uint8_t i = 0; i < MAX_FW_LAND_APPOACH_SETTINGS; i++) {
        bool equalsDefault = false;
        if (defaultFwAutolandApproach) {
               equalsDefault = navFwAutolandApproach[i].approachDirection == defaultFwAutolandApproach[i].approachDirection
               && navFwAutolandApproach[i].approachAlt == defaultFwAutolandApproach[i].approachAlt
               && navFwAutolandApproach[i].landAlt == defaultFwAutolandApproach[i].landAlt
               && navFwAutolandApproach[i].landApproachHeading1 == defaultFwAutolandApproach[i].landApproachHeading1
               && navFwAutolandApproach[i].landApproachHeading2 == defaultFwAutolandApproach[i].landApproachHeading2
               && navFwAutolandApproach[i].isSeaLevelRef == defaultFwAutolandApproach[i].isSeaLevelRef;
            cliDefaultPrintLinef(dumpMask, equalsDefault, format, i,
               defaultFwAutolandApproach[i].approachAlt, defaultFwAutolandApproach[i].landAlt, defaultFwAutolandApproach[i].approachDirection, defaultFwAutolandApproach[i].landApproachHeading1, defaultFwAutolandApproach[i].landApproachHeading2, defaultFwAutolandApproach[i].isSeaLevelRef);
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format, i,
            navFwAutolandApproach[i].approachAlt, navFwAutolandApproach[i].landAlt, navFwAutolandApproach[i].approachDirection, navFwAutolandApproach[i].landApproachHeading1, navFwAutolandApproach[i].landApproachHeading2, navFwAutolandApproach[i].isSeaLevelRef);
    }
}

static void cliFwAutolandApproach(char * cmdline)
{
     if (isEmpty(cmdline)) {
        printFwAutolandApproach(DUMP_MASTER, fwAutolandApproachConfig(0), NULL);
    } else if (sl_strcasecmp(cmdline, "reset") == 0) {
        resetFwAutolandApproach(-1);
    } else {
        int32_t approachAlt = 0, heading1 = 0, heading2 = 0, landDirection = 0, landAlt = 0;
        bool isSeaLevelRef = false;
        uint8_t validArgumentCount = 0;
        const char *ptr = cmdline;
        int8_t i = fastA2I(ptr);
        if (i < 0 || i >= MAX_FW_LAND_APPOACH_SETTINGS) {
             cliShowArgumentRangeError("fwapproach index", 0, MAX_FW_LAND_APPOACH_SETTINGS - 1);
        } else {
            if ((ptr = nextArg(ptr))) {
                approachAlt = fastA2I(ptr);
                validArgumentCount++;
            }

            if ((ptr = nextArg(ptr))) {
                landAlt = fastA2I(ptr);
                validArgumentCount++;
            }

            if ((ptr = nextArg(ptr))) {
                landDirection = fastA2I(ptr);

                if (landDirection != 0 && landDirection != 1) {
                    cliShowParseError();
                    return;
                }

                validArgumentCount++;
            }

            if ((ptr = nextArg(ptr))) {
                heading1 = fastA2I(ptr);

                if (heading1 < -360 || heading1 > 360) {
                    cliShowParseError();
                    return;
                }

                validArgumentCount++;
            }

            if ((ptr = nextArg(ptr))) {
                heading2 = fastA2I(ptr);

                if (heading2 < -360 || heading2 > 360) {
                    cliShowParseError();
                    return;
                }

                validArgumentCount++;
            }

            if ((ptr = nextArg(ptr))) {
                isSeaLevelRef = fastA2I(ptr);
                validArgumentCount++;
            }

            if ((ptr = nextArg(ptr))) {
                // check for too many arguments
                validArgumentCount++;
            }

            if (validArgumentCount != 6) {
                cliShowParseError();
            } else {
                fwAutolandApproachConfigMutable(i)->approachAlt = approachAlt;
                fwAutolandApproachConfigMutable(i)->landAlt = landAlt;
                fwAutolandApproachConfigMutable(i)->approachDirection = (fwAutolandApproachDirection_e)landDirection;
                fwAutolandApproachConfigMutable(i)->landApproachHeading1 = (int16_t)heading1;
                fwAutolandApproachConfigMutable(i)->landApproachHeading2 = (int16_t)heading2;
                fwAutolandApproachConfigMutable(i)->isSeaLevelRef = isSeaLevelRef;
            }
        }
    }
}
#endif

#if defined(USE_SAFE_HOME)
static void printSafeHomes(uint8_t dumpMask, const navSafeHome_t *navSafeHome, const navSafeHome_t *defaultSafeHome)
{
    const char *format = "safehome %u %u %d %d"; // uint8_t enabled, int32_t lat; int32_t lon
    for (uint8_t i = 0; i < MAX_SAFE_HOMES; i++) {
        bool equalsDefault = false;
        if (defaultSafeHome) {
            equalsDefault = navSafeHome[i].enabled == defaultSafeHome[i].enabled
               && navSafeHome[i].lat == defaultSafeHome[i].lat
               && navSafeHome[i].lon == defaultSafeHome[i].lon;
            cliDefaultPrintLinef(dumpMask, equalsDefault, format, i,
                defaultSafeHome[i].enabled, defaultSafeHome[i].lat, defaultSafeHome[i].lon);
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format, i,
            navSafeHome[i].enabled, navSafeHome[i].lat, navSafeHome[i].lon);
    }
}

static void cliSafeHomes(char *cmdline)
{
    if (isEmpty(cmdline)) {
        printSafeHomes(DUMP_MASTER, safeHomeConfig(0), NULL);
    } else if (sl_strcasecmp(cmdline, "reset") == 0) {
        resetSafeHomes();
    } else {
        int32_t lat=0, lon=0;
        bool enabled=false;
        uint8_t validArgumentCount = 0;
        const char *ptr = cmdline;
        int8_t i = fastA2I(ptr);
        if (i < 0 || i >= MAX_SAFE_HOMES) {
             cliShowArgumentRangeError("safehome index", 0, MAX_SAFE_HOMES - 1);
        } else {
            if ((ptr = nextArg(ptr))) {
                enabled = fastA2I(ptr);
                validArgumentCount++;
            }
            if ((ptr = nextArg(ptr))) {
                lat = fastA2I(ptr);
                validArgumentCount++;
            }
            if ((ptr = nextArg(ptr))) {
                lon = fastA2I(ptr);
                validArgumentCount++;
            }
            if ((ptr = nextArg(ptr))) {
                // check for too many arguments
                validArgumentCount++;
            }
            if (validArgumentCount != 3) {
                cliShowParseError();
            } else {
                safeHomeConfigMutable(i)->enabled = enabled;
                safeHomeConfigMutable(i)->lat = lat;
                safeHomeConfigMutable(i)->lon = lon;
            }
        }
    }
}

#endif

#if defined(USE_GEOZONE)
static void printGeozones(uint8_t dumpMask, const geoZoneConfig_t *geoZone, const geoZoneConfig_t *defaultGeoZone)
{
    const char *format = "geozone %u %u %u %d %d %u %u %u";
    for (uint8_t i = 0; i < MAX_GEOZONES_IN_CONFIG; i++) {
        bool equalsDefault = false;
        if (defaultGeoZone) {
            equalsDefault = geoZone[i].fenceAction == defaultGeoZone->fenceAction
            && geoZone[i].shape == defaultGeoZone->shape
            && geoZone[i].type == defaultGeoZone->type
            && geoZone[i].maxAltitude == defaultGeoZone->maxAltitude
            && geoZone[i].minAltitude == defaultGeoZone->minAltitude
            && geoZone[i].isSealevelRef == defaultGeoZone->isSealevelRef
            && geoZone[i].fenceAction == defaultGeoZone->fenceAction
            && geoZone[i].vertexCount == defaultGeoZone->vertexCount;

            cliDefaultPrintLinef(dumpMask, equalsDefault, format, defaultGeoZone[i].shape, defaultGeoZone[i].type, defaultGeoZone[i].minAltitude, defaultGeoZone[i].maxAltitude, defaultGeoZone[i].isSealevelRef, defaultGeoZone[i].fenceAction, defaultGeoZone[i].vertexCount);  
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format, i, geoZone[i].shape, geoZone[i].type, geoZone[i].minAltitude, geoZone[i].maxAltitude, geoZone[i].isSealevelRef, geoZone[i].fenceAction, geoZone[i].vertexCount);    
    }
} 

static void printGeozoneVertices(uint8_t dumpMask, const vertexConfig_t *vertices, const vertexConfig_t *defaultVertices)
{
    const char *format = "geozone vertex %d %u %d %d";
    for (uint8_t i = 0; i < MAX_VERTICES_IN_CONFIG; i++) {
        bool equalsDefault = false;
        if (defaultVertices) {
            equalsDefault = vertices[i].idx == defaultVertices->idx
            && vertices[i].lat == defaultVertices->lat
            && vertices[i].lon == defaultVertices->lon
            && vertices[i].zoneId == defaultVertices->zoneId;
            
            cliDefaultPrintLinef(dumpMask, equalsDefault, format, defaultVertices[i].zoneId, defaultVertices[i].idx, defaultVertices[i].lat, defaultVertices[i].lon);
        }
        
        cliDumpPrintLinef(dumpMask, equalsDefault, format, vertices[i].zoneId, vertices[i].idx, vertices[i].lat, vertices[i].lon);    
    }

    if (!defaultVertices) {
        uint8_t totalVertices = geozoneGetUsedVerticesCount();
        cliPrintLinef("# %u vertices free (Used %u of %u)", MAX_VERTICES_IN_CONFIG - totalVertices, totalVertices, MAX_VERTICES_IN_CONFIG);
    }
}

static void cliGeozone(char* cmdLine)
{  
    if (isEmpty(cmdLine)) {
        printGeozones(DUMP_MASTER, geoZonesConfig(0), NULL);
    } else if (sl_strcasecmp(cmdLine, "vertex") == 0) {    
        printGeozoneVertices(DUMP_MASTER, geoZoneVertices(0), NULL);
    } else if (sl_strncasecmp(cmdLine, "vertex reset", 12) == 0) {
         const char* ptr = &cmdLine[12];
         uint8_t zoneId = 0, idx = 0;
         uint8_t argumentCount = 1;

         if ((ptr = nextArg(ptr))) {
            zoneId = fastA2I(ptr);
         } else {
           geozoneResetVertices(-1, -1);
           return;
        }

         if ((ptr = nextArg(ptr))) {
            argumentCount++;
            idx = fastA2I(ptr);
        } else {
            geozoneResetVertices(zoneId, -1);
            return;
        }

         if (argumentCount != 2) {
            cliShowParseError();
            return;
        }

        geozoneResetVertices(zoneId, idx);

    } else if (sl_strncasecmp(cmdLine, "vertex", 6) == 0) {
        int32_t lat = 0, lon = 0;
        int8_t zoneId = 0;
        int16_t vertexIdx = -1;
        uint8_t vertexZoneIdx = 0;
        const char* ptr = cmdLine;
        uint8_t argumentCount = 1;

        if ((ptr = nextArg(ptr))) {         
            zoneId = fastA2I(ptr);
            if (zoneId < 0) {
                return;
            }

            if (zoneId >= MAX_GEOZONES_IN_CONFIG) {
                cliShowArgumentRangeError("geozone index", 0, MAX_GEOZONES_IN_CONFIG - 1);
                return;
            }
        } else {
            cliShowParseError();
            return;
        }

         if ((ptr = nextArg(ptr))) {
            argumentCount++;
            vertexZoneIdx = fastA2I(ptr);
        } else {
            cliShowParseError();
            return;
        }

        if ((ptr = nextArg(ptr))) {
            argumentCount++;
            lat = fastA2I(ptr);
        } else {
            cliShowParseError();
            return;
        }
        
        if ((ptr = nextArg(ptr))) {
            argumentCount++;
            lon = fastA2I(ptr);
        } else {
            cliShowParseError();
            return;
        }

        if ((ptr = nextArg(ptr))) {
            argumentCount++;
        }

        if (argumentCount != 4) {
            cliShowParseError();
            return;
        }
        
        for (uint8_t i = 0; i < MAX_VERTICES_IN_CONFIG; i++) {
            if (geoZoneVertices(i)->zoneId == zoneId && geoZoneVertices(i)->idx == vertexZoneIdx)  {
                geoZoneVerticesMutable(i)->lat = lat;
                geoZoneVerticesMutable(i)->lon = lon;
                return;
            }
        }

        for (uint8_t i = 0; i < MAX_VERTICES_IN_CONFIG; i++) {
            if (geoZoneVertices(i)->zoneId == -1) {
                vertexIdx = i;
                break;
            }
        }

        if (vertexIdx < 0 || vertexIdx >= MAX_VERTICES_IN_CONFIG || vertexZoneIdx > MAX_VERTICES_IN_CONFIG) {
            cliPrintError("Maximum number of vertices reached.");
            return;
        }

        geoZoneVerticesMutable(vertexIdx)->lat = lat;
        geoZoneVerticesMutable(vertexIdx)->lon = lon;
        geoZoneVerticesMutable(vertexIdx)->zoneId = zoneId;
        geoZoneVerticesMutable(vertexIdx)->idx = vertexZoneIdx;  
        
        uint8_t totalVertices = geozoneGetUsedVerticesCount();
        cliPrintLinef("# %u vertices free (Used %u of %u)", MAX_VERTICES_IN_CONFIG - totalVertices, totalVertices, MAX_VERTICES_IN_CONFIG);

    } else if (sl_strncasecmp(cmdLine, "reset", 5) == 0) {
        const char* ptr = &cmdLine[5];
        if ((ptr = nextArg(ptr))) {
            int idx = fastA2I(ptr);
            geozoneReset(idx);
            geozoneResetVertices(idx, -1);
        } else {
            geozoneReset(-1);
            geozoneResetVertices(-1, -1);
        } 
    } else {
        int8_t idx = 0, isPolygon = 0, isInclusive = 0, fenceAction = 0, seaLevelRef = 0, vertexCount = 0;
        int32_t minAltitude = 0, maxAltitude = 0;
        const char* ptr = cmdLine;
        uint8_t argumentCount = 1;

        idx = fastA2I(ptr);
        if (idx < 0 || idx > MAX_GEOZONES_IN_CONFIG) {
            cliShowArgumentRangeError("geozone index", 0, MAX_GEOZONES_IN_CONFIG - 1);
            return;
        }
        
        if ((ptr = nextArg(ptr))) {
            argumentCount++;
            isPolygon = fastA2I(ptr);
        } else {
            cliShowParseError();
            return;
        }

        if ((ptr = nextArg(ptr))){
            argumentCount++;
            isInclusive = fastA2I(ptr);
        } else {
            cliShowParseError();
            return;
        }

        if ((ptr = nextArg(ptr))){
            argumentCount++;
            minAltitude = fastA2I(ptr);
        } else {
            cliShowParseError();
            return;
        }

        if ((ptr = nextArg(ptr))){
            argumentCount++;
            maxAltitude = fastA2I(ptr);
        } else {
            cliShowParseError();
            return;
        }

        if ((ptr = nextArg(ptr))){
            argumentCount++;
            seaLevelRef = fastA2I(ptr);
        } else {
            cliShowParseError();
            return;
        }

        if ((ptr = nextArg(ptr))){
            argumentCount++;        
            fenceAction = fastA2I(ptr);
            if (fenceAction < 0 || fenceAction > GEOFENCE_ACTION_RTH) {
                cliShowArgumentRangeError("fence action", 0, GEOFENCE_ACTION_RTH);
                return;
            }
        } else {
            cliShowParseError();
            return;
        }

        if ((ptr = nextArg(ptr))){
            argumentCount++;
            vertexCount = fastA2I(ptr);
            if (vertexCount < 1 || vertexCount > MAX_VERTICES_IN_CONFIG) {
                cliShowArgumentRangeError("vertex count", 1, MAX_VERTICES_IN_CONFIG);
                return;
            }
        } else {
            cliShowParseError();
            return;
        }

        if ((ptr = nextArg(ptr))){
            argumentCount++;
        } 

        if (argumentCount != 8) {
            cliShowParseError();
            return;
        }

        if (isPolygon) {
            geoZonesConfigMutable(idx)->shape = GEOZONE_SHAPE_POLYGON;
        } else {
            geoZonesConfigMutable(idx)->shape = GEOZONE_SHAPE_CIRCULAR;
        }

        if (isInclusive) {
            geoZonesConfigMutable(idx)->type = GEOZONE_TYPE_INCLUSIVE;
        } else {
            geoZonesConfigMutable(idx)->type = GEOZONE_TYPE_EXCLUSIVE;
        }

        geoZonesConfigMutable(idx)->maxAltitude = maxAltitude;
        geoZonesConfigMutable(idx)->minAltitude = minAltitude;
        geoZonesConfigMutable(idx)->isSealevelRef = (bool)seaLevelRef;
        geoZonesConfigMutable(idx)->fenceAction = fenceAction;
        geoZonesConfigMutable(idx)->vertexCount = vertexCount;
    }
}
#endif

#if defined(NAV_NON_VOLATILE_WAYPOINT_STORAGE) && defined(NAV_NON_VOLATILE_WAYPOINT_CLI)
static void printWaypoints(uint8_t dumpMask, const navWaypoint_t *navWaypoint, const navWaypoint_t *defaultNavWaypoint)
{
    cliPrintLinef("#wp %d %svalid", posControl.waypointCount, posControl.waypointListValid ? "" : "in"); //int8_t bool
    const char *format = "wp %u %u %d %d %d %d %d %d %u"; //uint8_t action; int32_t lat; int32_t lon; int32_t alt; int16_t p1 int16_t p2 int16_t p3; uint8_t flag
    for (uint8_t i = 0; i < NAV_MAX_WAYPOINTS; i++) {
        bool equalsDefault = false;
        if (defaultNavWaypoint) {
            equalsDefault = navWaypoint[i].action == defaultNavWaypoint[i].action
                && navWaypoint[i].lat == defaultNavWaypoint[i].lat
                && navWaypoint[i].lon == defaultNavWaypoint[i].lon
                && navWaypoint[i].alt == defaultNavWaypoint[i].alt
                && navWaypoint[i].p1 == defaultNavWaypoint[i].p1
                && navWaypoint[i].p2 == defaultNavWaypoint[i].p2
                && navWaypoint[i].p3 == defaultNavWaypoint[i].p3
                && navWaypoint[i].flag == defaultNavWaypoint[i].flag;
            cliDefaultPrintLinef(dumpMask, equalsDefault, format,
                i,
                defaultNavWaypoint[i].action,
                defaultNavWaypoint[i].lat,
                defaultNavWaypoint[i].lon,
                defaultNavWaypoint[i].alt,
                defaultNavWaypoint[i].p1,
                defaultNavWaypoint[i].p2,
                defaultNavWaypoint[i].p3,
                defaultNavWaypoint[i].flag
            );
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format,
            i,
            navWaypoint[i].action,
            navWaypoint[i].lat,
            navWaypoint[i].lon,
            navWaypoint[i].alt,
            navWaypoint[i].p1,
            navWaypoint[i].p2,
            navWaypoint[i].p3,
            navWaypoint[i].flag
        );
    }
}

static void cliWaypoints(char *cmdline)
{
#ifdef USE_MULTI_MISSION
    static int8_t multiMissionWPCounter = 0;
#endif
    if (isEmpty(cmdline)) {
        printWaypoints(DUMP_MASTER, posControl.waypointList, NULL);
    } else if (sl_strcasecmp(cmdline, "reset") == 0) {
        resetWaypointList();
    } else if (sl_strcasecmp(cmdline, "load") == 0) {
        loadNonVolatileWaypointList(true);
    } else if (sl_strcasecmp(cmdline, "save") == 0) {
        posControl.waypointListValid = false;
        for (int i = 0; i < NAV_MAX_WAYPOINTS; i++) {
            if (!(posControl.waypointList[i].action == NAV_WP_ACTION_WAYPOINT || posControl.waypointList[i].action == NAV_WP_ACTION_JUMP || posControl.waypointList[i].action == NAV_WP_ACTION_RTH || posControl.waypointList[i].action == NAV_WP_ACTION_HOLD_TIME || posControl.waypointList[i].action == NAV_WP_ACTION_LAND || posControl.waypointList[i].action == NAV_WP_ACTION_SET_POI || posControl.waypointList[i].action == NAV_WP_ACTION_SET_HEAD)) break;
            if (posControl.waypointList[i].flag == NAV_WP_FLAG_LAST) {
#ifdef USE_MULTI_MISSION
                if (posControl.multiMissionCount == 1) {
                    posControl.waypointCount = i + 1;
                    posControl.waypointListValid = true;
                    multiMissionWPCounter = 0;
                    posControl.multiMissionCount = 0;
                    break;
                } else {
                    posControl.multiMissionCount -= 1;
                }
#else
                posControl.waypointCount = i + 1;
                posControl.waypointListValid = true;
                break;
#endif
            }
        }
        if (posControl.waypointListValid) {
            saveNonVolatileWaypointList();
        } else {
            cliShowParseError();
        }
    } else {
        int16_t i, p1=0,p2=0,p3=0,tmp=0;
        uint8_t action=0, flag=0;
        int32_t lat=0, lon=0, alt=0;
        uint8_t validArgumentCount = 0;
        const char *ptr = cmdline;
        i = fastA2I(ptr);
#ifdef USE_MULTI_MISSION
        if (i + multiMissionWPCounter >= 0 && i + multiMissionWPCounter < NAV_MAX_WAYPOINTS) {
#else
        if (i >= 0 && i < NAV_MAX_WAYPOINTS) {
#endif
            ptr = nextArg(ptr);
            if (ptr) {
                action = fastA2I(ptr);
                validArgumentCount++;
            }
            ptr = nextArg(ptr);
            if (ptr) {
                lat = fastA2I(ptr);
                validArgumentCount++;
            }
            ptr = nextArg(ptr);
            if (ptr) {
                lon = fastA2I(ptr);
                validArgumentCount++;
            }
            ptr = nextArg(ptr);
            if (ptr) {
                alt = fastA2I(ptr);
                validArgumentCount++;
            }
            ptr = nextArg(ptr);
            if (ptr) {
                p1 = fastA2I(ptr);
                validArgumentCount++;
            }
            ptr = nextArg(ptr);
            if (ptr) {
                tmp = fastA2I(ptr);
                validArgumentCount++;
            }
                /* We support pre-2.5 6 values (... p1,flags) or
                 *  2.5 and later, 8 values (... p1,p2,p3,flags)
                 */
            ptr = nextArg(ptr);
            if (ptr) {
                p2 = tmp;
                p3 = fastA2I(ptr);
                validArgumentCount++;
                ptr = nextArg(ptr);
                 if (ptr) {
                    flag = fastA2I(ptr);
                    validArgumentCount++;
                }
            } else {
                flag = tmp;
            }

            if (!(validArgumentCount == 6 || validArgumentCount == 8)) {
                cliShowParseError();
            } else if (!(action == 0 || action == NAV_WP_ACTION_WAYPOINT || action == NAV_WP_ACTION_RTH || action == NAV_WP_ACTION_JUMP || action == NAV_WP_ACTION_HOLD_TIME || action == NAV_WP_ACTION_LAND || action == NAV_WP_ACTION_SET_POI || action == NAV_WP_ACTION_SET_HEAD) || !(flag == 0 || flag == NAV_WP_FLAG_LAST || flag == NAV_WP_FLAG_HOME)) {
                cliShowParseError();
            } else {
#ifdef USE_MULTI_MISSION
                if (i + multiMissionWPCounter == 0) {
                    posControl.multiMissionCount = 0;
                }

                posControl.waypointList[i + multiMissionWPCounter].action = action;
                posControl.waypointList[i + multiMissionWPCounter].lat = lat;
                posControl.waypointList[i + multiMissionWPCounter].lon = lon;
                posControl.waypointList[i + multiMissionWPCounter].alt = alt;
                posControl.waypointList[i + multiMissionWPCounter].p1 = p1;
                posControl.waypointList[i + multiMissionWPCounter].p2 = p2;
                posControl.waypointList[i + multiMissionWPCounter].p3 = p3;
                posControl.waypointList[i + multiMissionWPCounter].flag = flag;

                // Process WP entries made up of multiple successive WP missions (multiple NAV_WP_FLAG_LAST entries)
                // Individial missions loaded at runtime, mission selected nav_waypoint_multi_mission_index
                if (flag == NAV_WP_FLAG_LAST) {
                    multiMissionWPCounter += i + 1;
                    posControl.multiMissionCount += 1;
                }
#else
                posControl.waypointList[i].action = action;
                posControl.waypointList[i].lat = lat;
                posControl.waypointList[i].lon = lon;
                posControl.waypointList[i].alt = alt;
                posControl.waypointList[i].p1 = p1;
                posControl.waypointList[i].p2 = p2;
                posControl.waypointList[i].p3 = p3;
                posControl.waypointList[i].flag = flag;
#endif
            }
        } else {
            cliShowArgumentRangeError("wp index", 0, NAV_MAX_WAYPOINTS - 1);
        }
    }
}

#endif

#ifdef USE_LED_STRIP
static void printLed(uint8_t dumpMask, const ledConfig_t *ledConfigs, const ledConfig_t *defaultLedConfigs)
{
    const char *format = "led %u %s";
    char ledConfigBuffer[20];
    char ledConfigDefaultBuffer[20];
    for (uint32_t i = 0; i < LED_MAX_STRIP_LENGTH; i++) {
        ledConfig_t ledConfig = ledConfigs[i];
        generateLedConfig(&ledConfig, ledConfigBuffer, sizeof(ledConfigBuffer));
        bool equalsDefault = false;
        if (defaultLedConfigs) {
            ledConfig_t ledConfigDefault = defaultLedConfigs[i];
            equalsDefault = !memcmp(&ledConfig, &ledConfigDefault, sizeof(ledConfig_t));
            generateLedConfig(&ledConfigDefault, ledConfigDefaultBuffer, sizeof(ledConfigDefaultBuffer));
            cliDefaultPrintLinef(dumpMask, equalsDefault, format, i, ledConfigDefaultBuffer);
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format, i, ledConfigBuffer);
    }
}

static void cliLed(char *cmdline)
{
    int i;
    const char *ptr;

    if (isEmpty(cmdline)) {
        printLed(DUMP_MASTER, ledStripConfig()->ledConfigs, NULL);
    } else {
        ptr = cmdline;
        i = fastA2I(ptr);
        if (i < LED_MAX_STRIP_LENGTH) {
            ptr = nextArg(cmdline);
            if (!parseLedStripConfig(i, ptr)) {
                cliShowParseError();
            }
        } else {
            cliShowArgumentRangeError("index", 0, LED_MAX_STRIP_LENGTH - 1);
        }
    }
}

static void printColor(uint8_t dumpMask, const hsvColor_t *colors, const hsvColor_t *defaultColors)
{
    const char *format = "color %u %d,%u,%u";
    for (uint32_t i = 0; i < LED_CONFIGURABLE_COLOR_COUNT; i++) {
        const hsvColor_t *color = &colors[i];
        bool equalsDefault = false;
        if (defaultColors) {
            const hsvColor_t *colorDefault = &defaultColors[i];
            equalsDefault = color->h == colorDefault->h
                && color->s == colorDefault->s
                && color->v == colorDefault->v;
            cliDefaultPrintLinef(dumpMask, equalsDefault, format, i,colorDefault->h, colorDefault->s, colorDefault->v);
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format, i, color->h, color->s, color->v);
    }
}

static void cliColor(char *cmdline)
{
    if (isEmpty(cmdline)) {
        printColor(DUMP_MASTER, ledStripConfig()->colors, NULL);
    } else {
        const char *ptr = cmdline;
        const int i = fastA2I(ptr);
        if (i < LED_CONFIGURABLE_COLOR_COUNT) {
            ptr = nextArg(cmdline);
            if (!parseColor(i, ptr)) {
                cliShowParseError();
            }
        } else {
            cliShowArgumentRangeError("index", 0, LED_CONFIGURABLE_COLOR_COUNT - 1);
        }
    }
}

static void printModeColor(uint8_t dumpMask, const ledStripConfig_t *ledStripConfig, const ledStripConfig_t *defaultLedStripConfig)
{
    const char *format = "mode_color %u %u %u";
    for (uint32_t i = 0; i < LED_MODE_COUNT; i++) {
        for (uint32_t j = 0; j < LED_DIRECTION_COUNT; j++) {
            int colorIndex = ledStripConfig->modeColors[i].color[j];
            bool equalsDefault = false;
            if (defaultLedStripConfig) {
                int colorIndexDefault = defaultLedStripConfig->modeColors[i].color[j];
                equalsDefault = colorIndex == colorIndexDefault;
                cliDefaultPrintLinef(dumpMask, equalsDefault, format, i, j, colorIndexDefault);
            }
            cliDumpPrintLinef(dumpMask, equalsDefault, format, i, j, colorIndex);
        }
    }

    for (uint32_t j = 0; j < LED_SPECIAL_COLOR_COUNT; j++) {
        const int colorIndex = ledStripConfig->specialColors.color[j];
        bool equalsDefault = false;
        if (defaultLedStripConfig) {
            const int colorIndexDefault = defaultLedStripConfig->specialColors.color[j];
            equalsDefault = colorIndex == colorIndexDefault;
            cliDefaultPrintLinef(dumpMask, equalsDefault, format, LED_SPECIAL, j, colorIndexDefault);
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format, LED_SPECIAL, j, colorIndex);
    }
}

static void cliModeColor(char *cmdline)
{
    char * saveptr;

    if (isEmpty(cmdline)) {
        printModeColor(DUMP_MASTER, ledStripConfig(), NULL);
    } else {
        enum {MODE = 0, FUNCTION, COLOR, ARGS_COUNT};
        int args[ARGS_COUNT];
        int argNo = 0;
        const char* ptr = strtok_r(cmdline, " ", &saveptr);
        while (ptr && argNo < ARGS_COUNT) {
            args[argNo++] = fastA2I(ptr);
            ptr = strtok_r(NULL, " ", &saveptr);
        }

        if (ptr != NULL || argNo != ARGS_COUNT) {
            cliShowParseError();
            return;
        }

        int modeIdx  = args[MODE];
        int funIdx = args[FUNCTION];
        int color = args[COLOR];
        if (!setModeColor(modeIdx, funIdx, color)) {
            cliShowParseError();
            return;
        }
        // values are validated
        cliPrintLinef("mode_color %u %u %u", modeIdx, funIdx, color);
    }
}

static void cliLedPinPWM(char *cmdline)
{
    int i;

    if (isEmpty(cmdline)) {
        ledPinStopPWM();
        cliPrintLine("PWM stopped");
    } else {
        i = fastA2I(cmdline);
        ledPinStartPWM(i);
        cliPrintLinef("PWM started: %d%%",i);
    }
}
#endif

static void cliDelay(char* cmdLine) {
    int ms = 0;
    if (isEmpty(cmdLine)) {
        cliDelayMs = 0;
        cliPrintLine("CLI delay deactivated");
        return;
    }

    ms = fastA2I(cmdLine);
    if (ms) {
        cliDelayMs = ms;
        cliPrintLinef("CLI delay set to %d ms", ms);

    } else {
        cliShowParseError();
    }

}

static void printServo(uint8_t dumpMask, const servoParam_t *servoParam, const servoParam_t *defaultServoParam)
{
    // print out servo settings
    const char *format = "servo %u %d %d %d %d";
    for (uint32_t i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
        const servoParam_t *servoConf = &servoParam[i];
        bool equalsDefault = false;
        if (defaultServoParam) {
            const servoParam_t *servoConfDefault = &defaultServoParam[i];
            equalsDefault = servoConf->min == servoConfDefault->min
                && servoConf->max == servoConfDefault->max
                && servoConf->middle == servoConfDefault->middle
                && servoConf->rate == servoConfDefault->rate;
            cliDefaultPrintLinef(dumpMask, equalsDefault, format,
                i,
                servoConfDefault->min,
                servoConfDefault->max,
                servoConfDefault->middle,
                servoConfDefault->rate
            );
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format,
            i,
            servoConf->min,
            servoConf->max,
            servoConf->middle,
            servoConf->rate
        );
    }
}

static void cliServo(char *cmdline)
{
    enum { SERVO_ARGUMENT_COUNT = 5 };
    int16_t arguments[SERVO_ARGUMENT_COUNT];

    servoParam_t *servo;

    int i;
    const char *ptr;

    if (isEmpty(cmdline)) {
        printServo(DUMP_MASTER, servoParams(0), NULL);
    } else {
        int validArgumentCount = 0;

        ptr = cmdline;

        // Command line is integers (possibly negative) separated by spaces, no other characters allowed.

        // If command line doesn't fit the format, don't modify the config
        while (*ptr) {
            if (*ptr == '-' || (*ptr >= '0' && *ptr <= '9')) {
                if (validArgumentCount >= SERVO_ARGUMENT_COUNT) {
                    cliShowParseError();
                    return;
                }

                arguments[validArgumentCount++] = fastA2I(ptr);

                do {
                    ptr++;
                } while (*ptr >= '0' && *ptr <= '9');
            } else if (*ptr == ' ') {
                ptr++;
            } else {
                cliShowParseError();
                return;
            }
        }

        enum {INDEX = 0, MIN, MAX, MIDDLE, RATE};

        i = arguments[INDEX];

        // Check we got the right number of args and the servo index is correct (don't validate the other values)
        if (validArgumentCount != SERVO_ARGUMENT_COUNT || i < 0 || i >= MAX_SUPPORTED_SERVOS) {
            cliShowParseError();
            return;
        }

        servo = servoParamsMutable(i);

        if (
            arguments[MIN] < SERVO_OUTPUT_MIN || arguments[MIN] > SERVO_OUTPUT_MAX ||
            arguments[MAX] < SERVO_OUTPUT_MIN || arguments[MAX] > SERVO_OUTPUT_MAX ||
            arguments[MIDDLE] < arguments[MIN] || arguments[MIDDLE] > arguments[MAX] ||
            arguments[MIN] > arguments[MAX] || arguments[MAX] < arguments[MIN] ||
            arguments[RATE] < -125 || arguments[RATE] > 125
        ) {
            cliShowParseError();
            return;
        }

        servo->min = arguments[MIN];
        servo->max = arguments[MAX];
        servo->middle = arguments[MIDDLE];
        servo->rate = arguments[RATE];
    }
}

static void printServoMix(uint8_t dumpMask, const servoMixer_t *customServoMixers, const servoMixer_t *defaultCustomServoMixers)
{
    const char *format = "smix %d %d %d %d %d %d";
    for (uint32_t i = 0; i < MAX_SERVO_RULES; i++) {
        const servoMixer_t customServoMixer = customServoMixers[i];
        if (customServoMixer.rate == 0) {
            break;
        }

        bool equalsDefault = false;
        if (defaultCustomServoMixers) {
            servoMixer_t customServoMixerDefault = defaultCustomServoMixers[i];
            equalsDefault = customServoMixer.targetChannel == customServoMixerDefault.targetChannel
                && customServoMixer.inputSource == customServoMixerDefault.inputSource
                && customServoMixer.rate == customServoMixerDefault.rate
                && customServoMixer.speed == customServoMixerDefault.speed
            #ifdef USE_PROGRAMMING_FRAMEWORK
                && customServoMixer.conditionId == customServoMixerDefault.conditionId
            #endif
            ;

            cliDefaultPrintLinef(dumpMask, equalsDefault, format,
                i,
                customServoMixerDefault.targetChannel,
                customServoMixerDefault.inputSource,
                customServoMixerDefault.rate,
                customServoMixerDefault.speed,
            #ifdef USE_PROGRAMMING_FRAMEWORK
                customServoMixer.conditionId
            #else
                0
            #endif
            );
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format,
            i,
            customServoMixer.targetChannel,
            customServoMixer.inputSource,
            customServoMixer.rate,
            customServoMixer.speed,
        #ifdef USE_PROGRAMMING_FRAMEWORK
            customServoMixer.conditionId
        #else
            0
        #endif
        );
    }
}

static void cliServoMix(char *cmdline)
{
    char * saveptr;
    int args[6], check = 0;
    uint8_t len = strlen(cmdline);

    if (len == 0) {
        printServoMix(DUMP_MASTER, customServoMixers(0), NULL);
    } else if (sl_strncasecmp(cmdline, "reset", 5) == 0) {
        // erase custom mixer
        Reset_servoMixers(customServoMixersMutable(0));
    } else {
        enum {RULE = 0, TARGET, INPUT, RATE, SPEED, CONDITION, ARGS_COUNT};
        char *ptr = strtok_r(cmdline, " ", &saveptr);
        args[CONDITION] = -1;
        while (ptr != NULL && check < ARGS_COUNT) {
            args[check++] = fastA2I(ptr);
            ptr = strtok_r(NULL, " ", &saveptr);
        }

        if (ptr != NULL || (check < ARGS_COUNT - 1)) {
            cliShowParseError();
            return;
        }

        int32_t i = args[RULE];
        if (
            i >= 0 && i < MAX_SERVO_RULES &&
            args[TARGET] >= 0 && args[TARGET] < MAX_SUPPORTED_SERVOS &&
            args[INPUT] >= 0 && args[INPUT] < INPUT_SOURCE_COUNT &&
            args[RATE] >= -1000 && args[RATE] <= 1000 &&
            args[SPEED] >= 0 && args[SPEED] <= MAX_SERVO_SPEED &&
            args[CONDITION] >= -1 && args[CONDITION] < MAX_LOGIC_CONDITIONS
        ) {
            customServoMixersMutable(i)->targetChannel = args[TARGET];
            customServoMixersMutable(i)->inputSource = args[INPUT];
            customServoMixersMutable(i)->rate = args[RATE];
            customServoMixersMutable(i)->speed = args[SPEED];
        #ifdef USE_PROGRAMMING_FRAMEWORK
            customServoMixersMutable(i)->conditionId = args[CONDITION];
        #endif
            cliServoMix("");
        } else {
            cliShowParseError();
        }
    }
}

#ifdef USE_PROGRAMMING_FRAMEWORK

static void printLogic(uint8_t dumpMask, const logicCondition_t *logicConditions, const logicCondition_t *defaultLogicConditions, int16_t showLC)
{
    const char *format = "logic %d %d %d %d %d %d %d %d %d";
    for (uint8_t i = 0; i < MAX_LOGIC_CONDITIONS; i++) {
        if (showLC == -1 || showLC == i) {
            const logicCondition_t logic = logicConditions[i];

            bool equalsDefault = false;
            if (defaultLogicConditions) {
                logicCondition_t defaultValue = defaultLogicConditions[i];
                equalsDefault =
                    logic.enabled == defaultValue.enabled &&
                    logic.activatorId == defaultValue.activatorId &&
                    logic.operation == defaultValue.operation &&
                    logic.operandA.type == defaultValue.operandA.type &&
                    logic.operandA.value == defaultValue.operandA.value &&
                    logic.operandB.type == defaultValue.operandB.type &&
                    logic.operandB.value == defaultValue.operandB.value &&
                    logic.flags == defaultValue.flags;

                cliDefaultPrintLinef(dumpMask, equalsDefault, format,
                    i,
                    logic.enabled,
                    logic.activatorId,
                    logic.operation,
                    logic.operandA.type,
                    logic.operandA.value,
                    logic.operandB.type,
                    logic.operandB.value,
                    logic.flags
                );
            }
            cliDumpPrintLinef(dumpMask, equalsDefault, format,
                i,
                logic.enabled,
                logic.activatorId,
                logic.operation,
                logic.operandA.type,
                logic.operandA.value,
                logic.operandB.type,
                logic.operandB.value,
                logic.flags
            );
        }
    }
}

static void processCliLogic(char *cmdline, int16_t lcIndex) {
    char * saveptr;
    int args[9], check = 0;
    uint8_t len = strlen(cmdline);

    if (len == 0) {
        if (!commandBatchActive) {
            printLogic(DUMP_MASTER, logicConditions(0), NULL, -1);
        } else if (lcIndex >= 0) {
            printLogic(DUMP_MASTER, logicConditions(0), NULL, lcIndex);
        }
    } else if (sl_strncasecmp(cmdline, "reset", 5) == 0) {
        pgResetCopy(logicConditionsMutable(0), PG_LOGIC_CONDITIONS);
    } else {
        enum {
            INDEX = 0,
            ENABLED,
            ACTIVATOR_ID,
            OPERATION,
            OPERAND_A_TYPE,
            OPERAND_A_VALUE,
            OPERAND_B_TYPE,
            OPERAND_B_VALUE,
            FLAGS,
            ARGS_COUNT
            };
        char *ptr = strtok_r(cmdline, " ", &saveptr);
        while (ptr != NULL && check < ARGS_COUNT) {
            args[check++] = fastA2I(ptr);
            ptr = strtok_r(NULL, " ", &saveptr);
        }

        if (ptr != NULL || check != ARGS_COUNT) {
            cliShowParseError();
            return;
        }

        int32_t i = args[INDEX];
        if (
            i >= 0 && i < MAX_LOGIC_CONDITIONS &&
            args[ENABLED] >= 0 && args[ENABLED] <= 1 &&
            args[ACTIVATOR_ID] >= -1 && args[ACTIVATOR_ID] < MAX_LOGIC_CONDITIONS &&
            args[OPERATION] >= 0 && args[OPERATION] < LOGIC_CONDITION_LAST &&
            args[OPERAND_A_TYPE] >= 0 && args[OPERAND_A_TYPE] < LOGIC_CONDITION_OPERAND_TYPE_LAST &&
            args[OPERAND_A_VALUE] >= -1000000 && args[OPERAND_A_VALUE] <= 1000000 &&
            args[OPERAND_B_TYPE] >= 0 && args[OPERAND_B_TYPE] < LOGIC_CONDITION_OPERAND_TYPE_LAST &&
            args[OPERAND_B_VALUE] >= -1000000 && args[OPERAND_B_VALUE] <= 1000000 &&
            args[FLAGS] >= 0 && args[FLAGS] <= 255

        ) {
            logicConditionsMutable(i)->enabled = args[ENABLED];
            logicConditionsMutable(i)->activatorId = args[ACTIVATOR_ID];
            logicConditionsMutable(i)->operation = args[OPERATION];
            logicConditionsMutable(i)->operandA.type = args[OPERAND_A_TYPE];
            logicConditionsMutable(i)->operandA.value = args[OPERAND_A_VALUE];
            logicConditionsMutable(i)->operandB.type = args[OPERAND_B_TYPE];
            logicConditionsMutable(i)->operandB.value = args[OPERAND_B_VALUE];
            logicConditionsMutable(i)->flags = args[FLAGS];

            processCliLogic("", i);
        } else {
            cliShowParseError();
        }
    }
}

static void cliLogic(char *cmdline) {
    processCliLogic(cmdline, -1);
}

static void printGvar(uint8_t dumpMask, const globalVariableConfig_t *gvars, const globalVariableConfig_t *defaultGvars)
{
    const char *format = "gvar %d %d %d %d";
    for (uint32_t i = 0; i < MAX_GLOBAL_VARIABLES; i++) {
        const globalVariableConfig_t gvar = gvars[i];

        bool equalsDefault = false;
        if (defaultGvars) {
            globalVariableConfig_t defaultValue = defaultGvars[i];
            equalsDefault =
                gvar.defaultValue == defaultValue.defaultValue &&
                gvar.min == defaultValue.min &&
                gvar.max == defaultValue.max;

            cliDefaultPrintLinef(dumpMask, equalsDefault, format,
                i,
                gvar.defaultValue,
                gvar.min,
                gvar.max
            );
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format,
            i,
            gvar.defaultValue,
            gvar.min,
            gvar.max
        );
    }
}

static void cliGvar(char *cmdline) {
    char * saveptr;
    int args[4], check = 0;
    uint8_t len = strlen(cmdline);

    if (len == 0) {
        printGvar(DUMP_MASTER, globalVariableConfigs(0), NULL);
    } else if (sl_strncasecmp(cmdline, "reset", 5) == 0) {
        pgResetCopy(globalVariableConfigsMutable(0), PG_GLOBAL_VARIABLE_CONFIG);
    } else {
        enum {
            INDEX = 0,
            DEFAULT,
            MIN,
            MAX,
            ARGS_COUNT
            };
        char *ptr = strtok_r(cmdline, " ", &saveptr);
        while (ptr != NULL && check < ARGS_COUNT) {
            args[check++] = fastA2I(ptr);
            ptr = strtok_r(NULL, " ", &saveptr);
        }

        if (ptr != NULL || check != ARGS_COUNT) {
            cliShowParseError();
            return;
        }

        int32_t i = args[INDEX];
        if (
            i >= 0 && i < MAX_GLOBAL_VARIABLES &&
            args[DEFAULT] >= INT32_MIN && args[DEFAULT] <= INT32_MAX &&
            args[MIN] >= INT32_MIN && args[MIN] <= INT32_MAX &&
            args[MAX] >= INT32_MIN && args[MAX] <= INT32_MAX
        ) {
            globalVariableConfigsMutable(i)->defaultValue = args[DEFAULT];
            globalVariableConfigsMutable(i)->min = args[MIN];
            globalVariableConfigsMutable(i)->max = args[MAX];

            cliGvar("");
        } else {
            cliShowParseError();
        }
    }
}

static void printPid(uint8_t dumpMask, const programmingPid_t *programmingPids, const programmingPid_t *defaultProgrammingPids)
{
    const char *format = "pid %d %d %d %d %d %d %d %d %d %d";
    for (uint32_t i = 0; i < MAX_PROGRAMMING_PID_COUNT; i++) {
        const programmingPid_t pid = programmingPids[i];

        bool equalsDefault = false;
        if (defaultProgrammingPids) {
            programmingPid_t defaultValue = defaultProgrammingPids[i];
            equalsDefault =
                pid.enabled == defaultValue.enabled &&
                pid.setpoint.type == defaultValue.setpoint.type &&
                pid.setpoint.value == defaultValue.setpoint.value &&
                pid.measurement.type == defaultValue.measurement.type &&
                pid.measurement.value == defaultValue.measurement.value &&
                pid.gains.P == defaultValue.gains.P &&
                pid.gains.I == defaultValue.gains.I &&
                pid.gains.D == defaultValue.gains.D &&
                pid.gains.FF == defaultValue.gains.FF;

            cliDefaultPrintLinef(dumpMask, equalsDefault, format,
                i,
                pid.enabled,
                pid.setpoint.type,
                pid.setpoint.value,
                pid.measurement.type,
                pid.measurement.value,
                pid.gains.P,
                pid.gains.I,
                pid.gains.D,
                pid.gains.FF
            );
        }
        cliDumpPrintLinef(dumpMask, equalsDefault, format,
            i,
            pid.enabled,
            pid.setpoint.type,
            pid.setpoint.value,
            pid.measurement.type,
            pid.measurement.value,
            pid.gains.P,
            pid.gains.I,
            pid.gains.D,
            pid.gains.FF
        );
    }
}

static void cliPid(char *cmdline) {
    char * saveptr;
    int args[10], check = 0;
    uint8_t len = strlen(cmdline);

    if (len == 0) {
        printPid(DUMP_MASTER, programmingPids(0), NULL);
    } else if (sl_strncasecmp(cmdline, "reset", 5) == 0) {
        pgResetCopy(programmingPidsMutable(0), PG_LOGIC_CONDITIONS);
    } else {
        enum {
            INDEX = 0,
            ENABLED,
            SETPOINT_TYPE,
            SETPOINT_VALUE,
            MEASUREMENT_TYPE,
            MEASUREMENT_VALUE,
            P_GAIN,
            I_GAIN,
            D_GAIN,
            FF_GAIN,
            ARGS_COUNT
            };
        char *ptr = strtok_r(cmdline, " ", &saveptr);
        while (ptr != NULL && check < ARGS_COUNT) {
            args[check++] = fastA2I(ptr);
            ptr = strtok_r(NULL, " ", &saveptr);
        }

        if (ptr != NULL || check != ARGS_COUNT) {
            cliShowParseError();
            return;
        }

        int32_t i = args[INDEX];
        if (
            i >= 0 && i < MAX_PROGRAMMING_PID_COUNT &&
            args[ENABLED] >= 0 && args[ENABLED] <= 1 &&
            args[SETPOINT_TYPE] >= 0 && args[SETPOINT_TYPE] < LOGIC_CONDITION_OPERAND_TYPE_LAST &&
            args[SETPOINT_VALUE] >= -1000000 && args[SETPOINT_VALUE] <= 1000000 &&
            args[MEASUREMENT_TYPE] >= 0 && args[MEASUREMENT_TYPE] < LOGIC_CONDITION_OPERAND_TYPE_LAST &&
            args[MEASUREMENT_VALUE] >= -1000000 && args[MEASUREMENT_VALUE] <= 1000000 &&
            args[P_GAIN] >= 0 && args[P_GAIN] <= INT16_MAX &&
            args[I_GAIN] >= 0 && args[I_GAIN] <= INT16_MAX &&
            args[D_GAIN] >= 0 && args[D_GAIN] <= INT16_MAX &&
            args[FF_GAIN] >= 0 && args[FF_GAIN] <= INT16_MAX
        ) {
            programmingPidsMutable(i)->enabled = args[ENABLED];
            programmingPidsMutable(i)->setpoint.type = args[SETPOINT_TYPE];
            programmingPidsMutable(i)->setpoint.value = args[SETPOINT_VALUE];
            programmingPidsMutable(i)->measurement.type = args[MEASUREMENT_TYPE];
            programmingPidsMutable(i)->measurement.value = args[MEASUREMENT_VALUE];
            programmingPidsMutable(i)->gains.P = args[P_GAIN];
            programmingPidsMutable(i)->gains.I = args[I_GAIN];
            programmingPidsMutable(i)->gains.D = args[D_GAIN];
            programmingPidsMutable(i)->gains.FF = args[FF_GAIN];

            cliPid("");
        } else {
            cliShowParseError();
        }
    }
}

static void printOsdCustomElements(uint8_t dumpMask, const osdCustomElement_t *osdCustomElements, const osdCustomElement_t *defaultosdCustomElements)
{
    const char *format = "osd_custom_elements %d %d %d %d %d %d %d %d %d \"%s\"";

    if(CUSTOM_ELEMENTS_PARTS != 3)
    {
        cliPrintHashLine("Incompatible count of elements for custom OSD elements");
    }

    for (uint8_t i = 0; i < MAX_CUSTOM_ELEMENTS; i++) {
        bool equalsDefault = false;

        const osdCustomElement_t osdCustomElement = osdCustomElements[i];
        if(defaultosdCustomElements){
            const osdCustomElement_t defaultValue = defaultosdCustomElements[i];
            equalsDefault =
                    osdCustomElement.part[0].type == defaultValue.part[0].type &&
                    osdCustomElement.part[0].value == defaultValue.part[0].value &&
                    osdCustomElement.part[1].type == defaultValue.part[1].type &&
                    osdCustomElement.part[1].value == defaultValue.part[1].value &&
                    osdCustomElement.part[2].type == defaultValue.part[2].type &&
                    osdCustomElement.part[2].value == defaultValue.part[2].value &&
                    osdCustomElement.visibility.type == defaultValue.visibility.type &&
                    osdCustomElement.visibility.value == defaultValue.visibility.value &&
                    strcmp(osdCustomElement.osdCustomElementText, defaultValue.osdCustomElementText) == 0;

            cliDefaultPrintLinef(dumpMask, equalsDefault, format,
                i,
                osdCustomElement.part[0].type,
                osdCustomElement.part[0].value,
                osdCustomElement.part[1].type,
                osdCustomElement.part[1].value,
                osdCustomElement.part[2].type,
                osdCustomElement.part[2].value,
                osdCustomElement.visibility.type,
                osdCustomElement.visibility.value,
                osdCustomElement.osdCustomElementText
            );
        }

        cliDumpPrintLinef(dumpMask, equalsDefault, format,
            i,
            osdCustomElement.part[0].type,
            osdCustomElement.part[0].value,
            osdCustomElement.part[1].type,
            osdCustomElement.part[1].value,
            osdCustomElement.part[2].type,
            osdCustomElement.part[2].value,
            osdCustomElement.visibility.type,
            osdCustomElement.visibility.value,
            osdCustomElement.osdCustomElementText
        );
    }
}

static void osdCustom(char *cmdline){
    char * saveptrMain;
    char * saveptrParams;
    int args[10], check = 0;
    char text[OSD_CUSTOM_ELEMENT_TEXT_SIZE];
    uint8_t len = strlen(cmdline);

    if (len == 0) {
        printOsdCustomElements(DUMP_MASTER, osdCustomElements(0), NULL);
    } else {
        //split by ", first are params second is text
        char *ptrMain = strtok_r(cmdline, "\"", &saveptrMain);
        enum {
            INDEX = 0,
            PART0_TYPE,
            PART0_VALUE,
            PART1_TYPE,
            PART1_VALUE,
            PART2_TYPE,
            PART2_VALUE,
            VISIBILITY_TYPE,
            VISIBILITY_VALUE,
            ARGS_COUNT
        };
        char *ptrParams = strtok_r(ptrMain, " ", &saveptrParams);
        while (ptrParams != NULL && check < ARGS_COUNT) {
            args[check++] = fastA2I(ptrParams);
            ptrParams = strtok_r(NULL, " ", &saveptrParams);
        }

        if (check != ARGS_COUNT) {
            cliShowParseError();
            return;
        }

        //text
        char *ptrText = strtok_r(NULL, "\"", &saveptrMain);
        size_t copySize = 0;
        if(ptrText != NULL){
            copySize = MIN(strlen(ptrText), (size_t)(sizeof(text) - 1));
            if(copySize > 0){
                memcpy(text, ptrText, copySize);
            }
        }
        text[copySize] = '\0';

        int32_t i = args[INDEX];
        if (
                i >= 0 && i < MAX_CUSTOM_ELEMENTS &&
                args[PART0_TYPE] >= 0 && args[PART0_TYPE] < CUSTOM_ELEMENT_TYPE_END &&
                args[PART0_VALUE] >= 0 && args[PART0_VALUE] <= UINT8_MAX &&
                args[PART1_TYPE] >= 0 && args[PART1_TYPE] < CUSTOM_ELEMENT_TYPE_END &&
                args[PART1_VALUE] >= 0 && args[PART1_VALUE] <= UINT8_MAX &&
                args[PART2_TYPE] >= 0 && args[PART2_TYPE] < CUSTOM_ELEMENT_TYPE_END &&
                args[PART2_VALUE] >= 0 && args[PART2_VALUE] <= UINT8_MAX &&
                args[VISIBILITY_TYPE] >= 0 && args[VISIBILITY_TYPE] <= 2 &&
                args[VISIBILITY_VALUE] >= 0 && args[VISIBILITY_VALUE] <= UINT8_MAX
                ) {
            osdCustomElementsMutable(i)->part[0].type = args[PART0_TYPE];
            osdCustomElementsMutable(i)->part[0].value = args[PART0_VALUE];
            osdCustomElementsMutable(i)->part[1].type = args[PART1_TYPE];
            osdCustomElementsMutable(i)->part[1].value = args[PART1_VALUE];
            osdCustomElementsMutable(i)->part[2].type = args[PART2_TYPE];
            osdCustomElementsMutable(i)->part[2].value = args[PART2_VALUE];
            osdCustomElementsMutable(i)->visibility.type = args[VISIBILITY_TYPE];
            osdCustomElementsMutable(i)->visibility.value = args[VISIBILITY_VALUE];
            memcpy(osdCustomElementsMutable(i)->osdCustomElementText, text, OSD_CUSTOM_ELEMENT_TEXT_SIZE);

            osdCustom("");
        } else {
            cliShowParseError();
        }
    }
}


#endif

#ifdef USE_SDCARD

static void cliWriteBytes(const uint8_t *buffer, int count)
{
    while (count > 0) {
        cliWrite(*buffer);
        buffer++;
        count--;
    }
}

static void cliSdInfo(char *cmdline)
{
    UNUSED(cmdline);

    cliPrint("SD card: ");

    if (!sdcard_isInserted()) {
        cliPrintLine("None inserted");
        return;
    }

    if (!sdcard_isInitialized()) {
        cliPrintLine("Startup failed");
        return;
    }

    const sdcardMetadata_t *metadata = sdcard_getMetadata();

    cliPrintf("Manufacturer 0x%x, %ukB, %02d/%04d, v%d.%d, '",
        metadata->manufacturerID,
        metadata->numBlocks / 2, /* One block is half a kB */
        metadata->productionMonth,
        metadata->productionYear,
        metadata->productRevisionMajor,
        metadata->productRevisionMinor
    );

    cliWriteBytes((uint8_t*)metadata->productName, sizeof(metadata->productName));

    cliPrint("'\r\n" "Filesystem: ");

    switch (afatfs_getFilesystemState()) {
        case AFATFS_FILESYSTEM_STATE_READY:
            cliPrint("Ready");
        break;
        case AFATFS_FILESYSTEM_STATE_INITIALIZATION:
            cliPrint("Initializing");
        break;
        case AFATFS_FILESYSTEM_STATE_UNKNOWN:
        case AFATFS_FILESYSTEM_STATE_FATAL:
            cliPrint("Fatal");

            switch (afatfs_getLastError()) {
                case AFATFS_ERROR_BAD_MBR:
                    cliPrint(" - no FAT MBR partitions");
                break;
                case AFATFS_ERROR_BAD_FILESYSTEM_HEADER:
                    cliPrint(" - bad FAT header");
                break;
                case AFATFS_ERROR_GENERIC:
                case AFATFS_ERROR_NONE:
                    ; // Nothing more detailed to print
                break;
            }
        break;
    }
    cliPrintLinefeed();
}

#endif

#ifdef USE_FLASHFS

static void cliFlashInfo(char *cmdline)
{
    UNUSED(cmdline);

    const flashGeometry_t *layout = flashGetGeometry();

    if (layout->totalSize == 0) {
        cliPrintLine("Flash not available");
        return;
    }

    cliPrintLinef("Flash sectors=%u, sectorSize=%u, pagesPerSector=%u, pageSize=%u, totalSize=%u",
            layout->sectors, layout->sectorSize, layout->pagesPerSector, layout->pageSize, layout->totalSize);

    for (uint8_t index = 0; index < FLASH_MAX_PARTITIONS; index++) {
        const flashPartition_t *partition;
        if (index == 0) {
            cliPrintLine("Paritions:");
        }
        partition = flashPartitionFindByIndex(index);
        if (!partition) {
            break;
        }
        cliPrintLinef("  %d: %s %u %u", index, flashPartitionGetTypeName(partition->type), partition->startSector, partition->endSector);
    }
#ifdef USE_FLASHFS
    const flashPartition_t *flashPartition = flashPartitionFindByType(FLASH_PARTITION_TYPE_FLASHFS);

    cliPrintLinef("FlashFS size=%u, usedSize=%u",
            FLASH_PARTITION_SECTOR_COUNT(flashPartition) * layout->sectorSize,
            flashfsGetOffset()
    );
#endif
}

static void cliFlashErase(char *cmdline)
{
    UNUSED(cmdline);

    const flashGeometry_t *layout = flashGetGeometry();

    if (layout->totalSize == 0) {
        cliPrintLine("Flash not available");
        return;
    }

    cliPrintLine("Erasing...");
    flashfsEraseCompletely();

    while (!flashIsReady()) {
        delay(100);
    }

    cliPrintLine("Done.");
}

#ifdef USE_FLASH_TOOLS

static void cliFlashWrite(char *cmdline)
{
    const uint32_t address = fastA2I(cmdline);
    const char *text = strchr(cmdline, ' ');

    if (!text) {
        cliShowParseError();
    } else {
        flashfsSeekAbs(address);
        flashfsWrite((uint8_t*)text, strlen(text), true);
        flashfsFlushSync();

        cliPrintLinef("Wrote %u bytes at %u.", strlen(text), address);
    }
}

static void cliFlashRead(char *cmdline)
{
    uint32_t address = fastA2I(cmdline);

    const char *nextArg = strchr(cmdline, ' ');

    if (!nextArg) {
        cliShowParseError();
    } else {
        uint32_t length = fastA2I(nextArg);

        cliPrintLinef("Reading %u bytes at %u:", length, address);

        uint8_t buffer[32];
        while (length > 0) {
            int bytesRead = flashfsReadAbs(address, buffer, length < sizeof(buffer) ? length : sizeof(buffer));

            for (int i = 0; i < bytesRead; i++) {
                cliWrite(buffer[i]);
            }

            length -= bytesRead;
            address += bytesRead;

            if (bytesRead == 0) {
                //Assume we reached the end of the volume or something fatal happened
                break;
            }
        }
        cliPrintLinefeed();
    }
}

#endif
#endif

#ifdef USE_OSD
static void printOsdLayout(uint8_t dumpMask, const osdLayoutsConfig_t *config, const osdLayoutsConfig_t *configDefault, int layout, int item)
{
    // "<layout> <item> <col> <row> <visible>"
    const char *format = "osd_layout %d %d %d %d %c";
    for (int ii = 0; ii < OSD_LAYOUT_COUNT; ii++) {
        if (layout >= 0 && layout != ii) {
            continue;
        }
        const uint16_t *layoutItems = config->item_pos[ii];
        const uint16_t *defaultLayoutItems = configDefault->item_pos[ii];
        for (int jj = 0; jj < OSD_ITEM_COUNT; jj++) {
            if (item >= 0 && item != jj) {
                continue;
            }
            bool equalsDefault = layoutItems[jj] == defaultLayoutItems[jj];
            cliDefaultPrintLinef(dumpMask, equalsDefault, format,
                ii, jj,
                OSD_X(defaultLayoutItems[jj]),
                OSD_Y(defaultLayoutItems[jj]),
                OSD_VISIBLE(defaultLayoutItems[jj]) ? 'V' : 'H');

            cliDumpPrintLinef(dumpMask, equalsDefault, format,
                ii, jj,
                OSD_X(layoutItems[jj]),
                OSD_Y(layoutItems[jj]),
                OSD_VISIBLE(layoutItems[jj]) ? 'V' : 'H');
        }
    }
}

static void cliOsdLayout(char *cmdline)
{
    char * saveptr;

    int layout = -1;
    int item = -1;
    int col = 0;
    int row = 0;
    bool visible = false;
    char *tok = strtok_r(cmdline, " ", &saveptr);

    int ii;

    for (ii = 0; tok != NULL; ii++, tok = strtok_r(NULL, " ", &saveptr)) {
        switch (ii) {
            case 0:
                layout = fastA2I(tok);
                if (layout < 0 || layout >= OSD_LAYOUT_COUNT) {
                    cliShowParseError();
                    return;
                }
                break;
            case 1:
                item = fastA2I(tok);
                if (item < 0 || item >= OSD_ITEM_COUNT) {
                    cliShowParseError();
                    return;
                }
                break;
            case 2:
                col = fastA2I(tok);
                if (col < 0 || col > OSD_X(OSD_POS_MAX)) {
                    cliShowParseError();
                    return;
                }
                break;
            case 3:
                row = fastA2I(tok);
                if (row < 0 || row > OSD_Y(OSD_POS_MAX)) {
                    cliShowParseError();
                    return;
                }
                break;
            case 4:
                switch (*tok) {
                    case 'H':
                        visible = false;
                        break;
                    case 'V':
                        visible = true;
                        break;
                    default:
                        cliShowParseError();
                        return;
                }
                break;
            default:
                cliShowParseError();
                return;
        }
    }

    switch (ii) {
        case 0:
            FALLTHROUGH;
        case 1:
            FALLTHROUGH;
        case 2:
            // No args, or just layout or layout and item. If any of them not provided,
            // it will be the -1 that we used during initialization, so printOsdLayout()
            // won't use them for filtering.
            printOsdLayout(DUMP_MASTER, osdLayoutsConfig(), osdLayoutsConfig(), layout, item);
            break;
        case 4:
            // No visibility provided. Keep the previous one.
            visible = OSD_VISIBLE(osdLayoutsConfig()->item_pos[layout][item]);
            FALLTHROUGH;
        case 5:
            // Layout, item, pos and visibility. Set the item.
            osdLayoutsConfigMutable()->item_pos[layout][item] = OSD_POS(col, row) | (visible ? OSD_VISIBLE_FLAG : 0);
            break;
        default:
            // Unhandled
            cliShowParseError();
            return;
    }
}

#endif

static void printTimerOutputModes(dumpFlags_e dumpFlags, const timerOverride_t* to, const timerOverride_t* defaultTimerOverride, int timer)
{
    const char *format = "timer_output_mode %d %s";

    for (int i = 0; i < HARDWARE_TIMER_DEFINITION_COUNT; ++i) {
        if (timer < 0 || timer == i) {
            outputMode_e mode = to[i].outputMode;
            bool equalsDefault = false;
            if(defaultTimerOverride) {
                outputMode_e defaultMode = defaultTimerOverride[i].outputMode;
                equalsDefault = mode == defaultMode;
                cliDefaultPrintLinef(dumpFlags, equalsDefault, format, i, outputModeNames[defaultMode]);
            }
            cliDumpPrintLinef(dumpFlags, equalsDefault, format, i, outputModeNames[mode]);
        }
    }
}

static void cliTimerOutputMode(char *cmdline)
{
    char * saveptr;

    int timer = -1;
    uint8_t mode;
    char *tok = strtok_r(cmdline, " ", &saveptr);

    int ii;

    for (ii = 0; tok != NULL; ii++, tok = strtok_r(NULL, " ", &saveptr)) {
        switch (ii) {
            case 0:
                timer = fastA2I(tok);
                if (timer < 0 || timer >= HARDWARE_TIMER_DEFINITION_COUNT) {
                    cliShowParseError();
                    return;
                }
                break;
            case 1:
                if(!sl_strcasecmp("AUTO", tok)) {
                    mode =  OUTPUT_MODE_AUTO;
                } else if(!sl_strcasecmp("MOTORS", tok)) {
                    mode = OUTPUT_MODE_MOTORS;
                } else if(!sl_strcasecmp("SERVOS", tok)) {
                    mode = OUTPUT_MODE_SERVOS;
                } else if(!sl_strcasecmp("LED", tok)) {
                    mode = OUTPUT_MODE_LED;
                } else {
                    cliShowParseError();
                    return;
                }
                break;
           default:
                cliShowParseError();
                return;
        }
    }

    switch (ii) {
        case 0:
            FALLTHROUGH;
        case 1:
            // No args, or just timer. If any of them not provided,
            // it will be the -1 that we used during initialization, so printOsdLayout()
            // won't use them for filtering.
            printTimerOutputModes(DUMP_MASTER, timerOverrides(0), NULL, timer);
            break;
        case 2:
            timerOverridesMutable(timer)->outputMode = mode;
            printTimerOutputModes(DUMP_MASTER, timerOverrides(0), NULL, timer);
            break;
        default:
            // Unhandled
            cliShowParseError();
            return;
    }

}

static void printFeature(uint8_t dumpMask, const featureConfig_t *featureConfig, const featureConfig_t *featureConfigDefault)
{
    uint32_t mask = featureConfig->enabledFeatures;
    uint32_t defaultMask = featureConfigDefault->enabledFeatures;
    for (uint32_t i = 0; ; i++) { // disable all feature first
        if (featureNames[i] == NULL)
            break;
        if (featureNames[i][0] == '\0')
            continue;
        const char *format = "feature -%s";
        cliDefaultPrintLinef(dumpMask, (defaultMask | ~mask) & (1 << i), format, featureNames[i]);
        cliDumpPrintLinef(dumpMask, (~defaultMask | mask) & (1 << i), format, featureNames[i]);
    }
    for (uint32_t i = 0; ; i++) {  // reenable what we want.
        if (featureNames[i] == NULL)
            break;
        if (featureNames[i][0] == '\0')
            continue;
        const char *format = "feature %s";
        if (defaultMask & (1 << i)) {
            cliDefaultPrintLinef(dumpMask, (~defaultMask | mask) & (1 << i), format, featureNames[i]);
        }
        if (mask & (1 << i)) {
            cliDumpPrintLinef(dumpMask, (defaultMask | ~mask) & (1 << i), format, featureNames[i]);
        }
    }
}

static void cliFeature(char *cmdline)
{
    uint32_t len = strlen(cmdline);
    uint32_t mask = featureMask();

    if (len == 0) {
        cliPrint("Enabled: ");
        for (uint32_t i = 0; ; i++) {
            if (featureNames[i] == NULL)
                break;
            if (featureNames[i][0] == '\0')
                continue;
            if (mask & (1 << i))
                cliPrintf("%s ", featureNames[i]);
        }
        cliPrintLinefeed();
    } else if (sl_strncasecmp(cmdline, "list", len) == 0) {
        cliPrint("Available: ");
        for (uint32_t i = 0; ; i++) {
            if (featureNames[i] == NULL)
                break;
            if (featureNames[i][0] == '\0')
                continue;
            cliPrintf("%s ", featureNames[i]);
        }
        cliPrintLinefeed();
        return;
    } else {
        bool remove = false;
        if (cmdline[0] == '-') {
            // remove feature
            remove = true;
            cmdline++; // skip over -
            len--;
        }

        for (uint32_t i = 0; ; i++) {
            if (featureNames[i] == NULL) {
                cliPrintErrorLine("Invalid name");
                break;
            }

            if (sl_strncasecmp(cmdline, featureNames[i], len) == 0) {

                mask = 1 << i;
#ifndef USE_GPS
                if (mask & FEATURE_GPS) {
                    cliPrintErrorLine("unavailable");
                    break;
                }
#endif
                if (remove) {
                    featureClear(mask);
                    cliPrint("Disabled");
                } else {
                    featureSet(mask);
                    cliPrint("Enabled");
                }
                cliPrintLinef(" %s", featureNames[i]);
                break;
            }
        }
    }
}

#ifdef USE_BLACKBOX
static void printBlackbox(uint8_t dumpMask, const blackboxConfig_t *config, const blackboxConfig_t *configDefault)
{

    UNUSED(configDefault);

    uint32_t mask = config->includeFlags;

    for (uint8_t i = 0; ; i++) {  // reenable what we want.
        if (blackboxIncludeFlagNames[i] == NULL) {
            break;
        }

        const char *formatOn = "blackbox %s";
        const char *formatOff = "blackbox -%s";

        if (mask & (1 << i)) {
            cliDumpPrintLinef(dumpMask, false, formatOn, blackboxIncludeFlagNames[i]);
            cliDefaultPrintLinef(dumpMask, false, formatOn, blackboxIncludeFlagNames[i]);
        } else {
            cliDumpPrintLinef(dumpMask, false, formatOff, blackboxIncludeFlagNames[i]);
            cliDefaultPrintLinef(dumpMask, false, formatOff, blackboxIncludeFlagNames[i]);
        }
    }

}

static void cliBlackbox(char *cmdline)
{
    uint32_t len = strlen(cmdline);
    uint32_t mask = blackboxConfig()->includeFlags;

    if (len == 0) {
        cliPrint("Enabled: ");
        for (uint8_t i = 0; ; i++) {
            if (blackboxIncludeFlagNames[i] == NULL) {
                break;
            }

            if (mask & (1 << i))
                cliPrintf("%s ", blackboxIncludeFlagNames[i]);
        }
        cliPrintLinefeed();
    } else if (sl_strncasecmp(cmdline, "list", len) == 0) {
        cliPrint("Available: ");
        for (uint32_t i = 0; ; i++) {
            if (blackboxIncludeFlagNames[i] == NULL) {
                break;
            }

            cliPrintf("%s ", blackboxIncludeFlagNames[i]);
        }
        cliPrintLinefeed();
        return;
    } else {
        bool remove = false;
        if (cmdline[0] == '-') {
            // remove feature
            remove = true;
            cmdline++; // skip over -
            len--;
        }

        for (uint32_t i = 0; ; i++) {
            if (blackboxIncludeFlagNames[i] == NULL) {
                cliPrintErrorLine("Invalid name");
                break;
            }

            if (sl_strncasecmp(cmdline, blackboxIncludeFlagNames[i], len) == 0) {

                mask = 1 << i;

                if (remove) {
                    blackboxIncludeFlagClear(mask);
                    cliPrint("Disabled");
                } else {
                    blackboxIncludeFlagSet(mask);
                    cliPrint("Enabled");
                }
                cliPrintLinef(" %s", blackboxIncludeFlagNames[i]);
                break;
            }
        }
    }
}
#endif

#if defined(BEEPER) || defined(USE_DSHOT)
static void printBeeper(uint8_t dumpMask, const beeperConfig_t *beeperConfig, const beeperConfig_t *beeperConfigDefault)
{
    const uint8_t beeperCount = beeperTableEntryCount();
    const uint32_t mask = beeperConfig->beeper_off_flags;
    const uint32_t defaultMask = beeperConfigDefault->beeper_off_flags;
    for (int i = 0; i < beeperCount - 2; i++) {
        const char *formatOff = "beeper -%s";
        const char *formatOn = "beeper %s";
        cliDefaultPrintLinef(dumpMask, ~(mask ^ defaultMask) & (1 << i), mask & (1 << i) ? formatOn : formatOff, beeperNameForTableIndex(i));
        cliDumpPrintLinef(dumpMask, ~(mask ^ defaultMask) & (1 << i), mask & (1 << i) ? formatOff : formatOn, beeperNameForTableIndex(i));
    }
}

static void cliBeeper(char *cmdline)
{
    uint32_t len = strlen(cmdline);
    uint8_t beeperCount = beeperTableEntryCount();
    uint32_t mask = getBeeperOffMask();

    if (len == 0) {
        cliPrintf("Disabled:");
        for (int32_t i = 0; ; i++) {
            if (i == beeperCount - 2){
                if (mask == 0)
                    cliPrint("  none");
                break;
            }
            if (mask & (1 << (beeperModeForTableIndex(i) - 1)))
                cliPrintf("  %s", beeperNameForTableIndex(i));
        }
        cliPrintLinefeed();
    } else if (sl_strncasecmp(cmdline, "list", len) == 0) {
        cliPrint("Available:");
        for (uint32_t i = 0; i < beeperCount; i++)
            cliPrintf("  %s", beeperNameForTableIndex(i));
        cliPrintLinefeed();
        return;
    } else {
        bool remove = false;
        if (cmdline[0] == '-') {
            remove = true;     // this is for beeper OFF condition
            cmdline++;
            len--;
        }

        for (uint32_t i = 0; ; i++) {
            if (i == beeperCount) {
                cliPrintErrorLine("Invalid name");
                break;
            }
            if (sl_strncasecmp(cmdline, beeperNameForTableIndex(i), len) == 0) {
                if (remove) { // beeper off
                    if (i == BEEPER_ALL-1)
                        beeperOffSetAll(beeperCount-2);
                    else
                        if (i == BEEPER_PREFERENCE-1)
                            setBeeperOffMask(getPreferredBeeperOffMask());
                        else {
                            mask = 1 << (beeperModeForTableIndex(i) - 1);
                            beeperOffSet(mask);
                        }
                    cliPrint("Disabled");
                }
                else { // beeper on
                    if (i == BEEPER_ALL-1)
                        beeperOffClearAll();
                    else
                        if (i == BEEPER_PREFERENCE-1)
                            setPreferredBeeperOffMask(getBeeperOffMask());
                        else {
                            mask = 1 << (beeperModeForTableIndex(i) - 1);
                            beeperOffClear(mask);
                        }
                    cliPrint("Enabled");
                }
            cliPrintLinef(" %s", beeperNameForTableIndex(i));
            break;
            }
        }
    }
}
#endif

static void printMap(uint8_t dumpMask, const rxConfig_t *rxConfig, const rxConfig_t *defaultRxConfig)
{
    bool equalsDefault = true;
    char buf[16];
    char bufDefault[16];
    uint32_t i;

    for (i = 0; i < MAX_MAPPABLE_RX_INPUTS; i++) {
        buf[i] = bufDefault[i] = 0;
    }

    for (i = 0; i < MAX_MAPPABLE_RX_INPUTS; i++) {
        buf[rxConfig->rcmap[i]] = rcChannelLetters[i];
        if (defaultRxConfig) {
            bufDefault[defaultRxConfig->rcmap[i]] = rcChannelLetters[i];
            equalsDefault = equalsDefault && (rxConfig->rcmap[i] == defaultRxConfig->rcmap[i]);
        }
    }
    buf[i] = '\0';

    const char *formatMap = "map %s";
    cliDefaultPrintLinef(dumpMask, equalsDefault, formatMap, bufDefault);
    cliDumpPrintLinef(dumpMask, equalsDefault, formatMap, buf);
}

static void cliMap(char *cmdline)
{
    uint32_t len;
    char out[MAX_MAPPABLE_RX_INPUTS + 1];

    len = strlen(cmdline);

    if (len == MAX_MAPPABLE_RX_INPUTS) {
        // uppercase it
        for (uint32_t i = 0; i < MAX_MAPPABLE_RX_INPUTS; i++) {
            cmdline[i] = sl_toupper((unsigned char)cmdline[i]);
        }
        for (uint32_t i = 0; i < MAX_MAPPABLE_RX_INPUTS; i++) {
            if (strchr(rcChannelLetters, cmdline[i]) && !strchr(cmdline + i + 1, cmdline[i])) {
                continue;
            }
            cliShowParseError();
            return;
        }
        parseRcChannels(cmdline);
    } else if (len != 0) {
        cliShowParseError();
    }
    cliPrint("Map: ");
    uint32_t i;
    for (i = 0; i < MAX_MAPPABLE_RX_INPUTS; i++){
        out[rxConfig()->rcmap[i]] = rcChannelLetters[i];
    }
    out[i] = '\0';
    cliPrintLinef("%s", out);
}

static const char *checkCommand(const char *cmdLine, const char *command)
{
    if (!sl_strncasecmp(cmdLine, command, strlen(command))    // command names match
        && !sl_isalnum((unsigned)cmdLine[strlen(command)])) { // next characted in bufffer is not alphanumeric (command is correctly terminated)
        return cmdLine + strlen(command) + 1;
    } else {
        return 0;
    }
}

static void cliRebootEx(bool bootLoader)
{
    cliPrint("\r\nRebooting");
    bufWriterFlush(cliWriter);
    waitForSerialPortToFinishTransmitting(cliPort);

    fcReboot(bootLoader);
}

static void cliReboot(void)
{
    cliRebootEx(false);
}

static void cliDfu(char *cmdline)
{
    UNUSED(cmdline);
#ifndef CLI_MINIMAL_VERBOSITY
    cliPrint("\r\nRestarting in DFU mode");
#endif
    cliRebootEx(true);
}

#if defined (USE_SERIALRX_SRXL2)
void cliRxBind(char *cmdline){
    UNUSED(cmdline);
    if (rxConfig()->receiverType == RX_TYPE_SERIAL) {
        switch (rxConfig()->serialrx_provider) {
        default:
            cliPrint("Not supported.");
            break;
#if defined(USE_SERIALRX_SRXL2)
        case SERIALRX_SRXL2:
            srxl2Bind();
            cliPrint("Binding SRXL2 receiver...");
            break;
#endif
#if defined(USE_SERIALRX_CRSF)
        case SERIALRX_CRSF:
            crsfBind();
            cliPrint("Binding CRSF receiver...");
            break;
#endif
        }
    }
}
#endif

static void cliExit(char *cmdline)
{
    UNUSED(cmdline);

#ifndef CLI_MINIMAL_VERBOSITY
    cliPrintLine("\r\nLeaving CLI mode, unsaved changes lost.");
#endif
    bufWriterFlush(cliWriter);

    *cliBuffer = '\0';
    bufferIndex = 0;
    cliMode = false;
    // incase a motor was left running during motortest, clear it here
    mixerResetDisarmedMotors();
    cliReboot();

    cliWriter = NULL;
}

#ifdef USE_GPS
static void cliGpsPassthrough(char *cmdline)
{
    UNUSED(cmdline);

    gpsEnablePassthrough(cliPort);
}
#endif

static void cliMotor(char *cmdline)
{
    int motor_index = 0;
    int motor_value = 0;
    int index = 0;
    char *pch = NULL;
    char *saveptr;

    if (isEmpty(cmdline)) {
        cliShowParseError();

        return;
    }

    pch = strtok_r(cmdline, " ", &saveptr);
    while (pch != NULL) {
        switch (index) {
            case 0:
                motor_index = fastA2I(pch);
                break;
            case 1:
                motor_value = fastA2I(pch);
                break;
        }
        index++;
        pch = strtok_r(NULL, " ", &saveptr);
    }

    if (motor_index < 0 || motor_index >= MAX_SUPPORTED_MOTORS) {
        cliShowArgumentRangeError("index", 0, MAX_SUPPORTED_MOTORS - 1);
        return;
    }

    if (index == 2) {
        if (motor_value < PWM_RANGE_MIN || motor_value > PWM_RANGE_MAX) {
            cliShowArgumentRangeError("value", 1000, 2000);
            return;
        } else {
            motor_disarmed[motor_index] = motor_value;
        }
    }

    cliPrintLinef("motor %d: %d", motor_index, motor_disarmed[motor_index]);
}

static void cliPlaySound(char *cmdline)
{
    int i;
    const char *name;
    static int lastSoundIdx = -1;

    if (isEmpty(cmdline)) {
        i = lastSoundIdx + 1;     //next sound index
        if ((name=beeperNameForTableIndex(i)) == NULL) {
            while (true) {   //no name for index; try next one
                if (++i >= beeperTableEntryCount())
                    i = 0;   //if end then wrap around to first entry
                if ((name=beeperNameForTableIndex(i)) != NULL)
                    break;   //if name OK then play sound below
                if (i == lastSoundIdx + 1) {     //prevent infinite loop
                    cliPrintLine("Error playing sound");
                    return;
                }
            }
        }
    } else {       //index value was given
        i = fastA2I(cmdline);
        if ((name=beeperNameForTableIndex(i)) == NULL) {
            cliPrintLinef("No sound for index %d", i);
            return;
        }
    }
    lastSoundIdx = i;
    beeperSilence();
    cliPrintLinef("Playing sound %d: %s", i, name);
    beeper(beeperModeForTableIndex(i));
}

static void cliControlProfile(char *cmdline)
{
    // CLI profile index is 1-based
    if (isEmpty(cmdline)) {
        cliPrintLinef("control_profile %d", getConfigProfile() + 1);
        return;
    } else {
        const int i = fastA2I(cmdline) - 1;
        if (i >= 0 && i < MAX_PROFILE_COUNT) {
            setConfigProfileAndWriteEEPROM(i);
            cliControlProfile("");
        }
    }
}

static void cliDumpControlProfile(uint8_t profileIndex, uint8_t dumpMask)
{
    if (profileIndex >= MAX_PROFILE_COUNT) {
        // Faulty values
        return;
    }
    setConfigProfile(profileIndex);
    cliPrintHashLine("control_profile");
    cliPrintLinef("control_profile %d\r\n", getConfigProfile() + 1);
    dumpAllValues(PROFILE_VALUE, dumpMask);
    dumpAllValues(CONTROL_RATE_VALUE, dumpMask);
    dumpAllValues(EZ_TUNE_VALUE, dumpMask);
}

static void cliBatteryProfile(char *cmdline)
{
    // CLI profile index is 1-based
    if (isEmpty(cmdline)) {
        cliPrintLinef("battery_profile %d", getConfigBatteryProfile() + 1);
        return;
    } else {
        const int i = fastA2I(cmdline) - 1;
        if (i >= 0 && i < MAX_PROFILE_COUNT) {
            setConfigBatteryProfileAndWriteEEPROM(i);
            cliBatteryProfile("");
        }
    }
}

static void cliDumpBatteryProfile(uint8_t profileIndex, uint8_t dumpMask)
{
    if (profileIndex >= MAX_BATTERY_PROFILE_COUNT) {
        // Faulty values
        return;
    }
    setConfigBatteryProfile(profileIndex);
    cliPrintHashLine("battery_profile");
    cliPrintLinef("battery_profile %d\r\n", getConfigBatteryProfile() + 1);
    dumpAllValues(BATTERY_CONFIG_VALUE, dumpMask);
}

static void cliMixerProfile(char *cmdline)
{
    // CLI profile index is 1-based
    if (isEmpty(cmdline)) {
        cliPrintLinef("mixer_profile %d", getConfigMixerProfile() + 1);
        return;
    } else {
        const int i = fastA2I(cmdline) - 1;
        if (i >= 0 && i < MAX_MIXER_PROFILE_COUNT) {
            setConfigMixerProfileAndWriteEEPROM(i);
            cliMixerProfile("");
        }
    }
}

static void cliDumpMixerProfile(uint8_t profileIndex, uint8_t dumpMask)
{
    if (profileIndex >= MAX_MIXER_PROFILE_COUNT) {
        // Faulty values
        return;
    }
    setConfigMixerProfile(profileIndex);
    cliPrintHashLine("mixer_profile");
    cliPrintLinef("mixer_profile %d\r\n", getConfigMixerProfile() + 1);
    dumpAllValues(MIXER_CONFIG_VALUE, dumpMask);
    cliPrintHashLine("Mixer: motor mixer");
    cliDumpPrintLinef(dumpMask, primaryMotorMixer_CopyArray()[0].throttle == 0.0f, "\r\nmmix reset\r\n");
    printMotorMix(dumpMask, primaryMotorMixer_CopyArray(), primaryMotorMixer(0));
    cliPrintHashLine("Mixer: servo mixer");
    cliDumpPrintLinef(dumpMask, customServoMixers_CopyArray()[0].rate == 0, "smix reset\r\n");
    printServoMix(dumpMask, customServoMixers_CopyArray(), customServoMixers(0));
}

#ifdef USE_CLI_BATCH
static void cliPrintCommandBatchWarning(const char *warning)
{
    char errorBuf[59];
    tfp_sprintf(errorBuf, "%d ERRORS WERE DETECTED - Please review and fix before continuing!", commandBatchErrorCount);

    cliPrintErrorLinef(errorBuf);
    if (warning) {
        cliPrintErrorLinef(warning);
    }
}

static void resetCommandBatch(void)
{
    commandBatchActive = false;
    commandBatchError = false;
    commandBatchErrorCount = 0;
}

static void cliBatch(char *cmdline)
{
    if (strncasecmp(cmdline, "start", 5) == 0) {
        if (!commandBatchActive) {
            commandBatchActive = true;
            commandBatchError = false;
            commandBatchErrorCount = 0;
        }
        cliPrintLine("Command batch started");
    } else if (strncasecmp(cmdline, "end", 3) == 0) {
        if (commandBatchActive && commandBatchError) {
            cliPrintCommandBatchWarning(NULL);
        } else {
            cliPrintLine("Command batch ended");
        }
        resetCommandBatch();
    } else {
        cliPrintErrorLinef("Invalid option");
    }
}
#endif

static void cliSave(char *cmdline)
{
    UNUSED(cmdline);

#ifdef USE_CLI_BATCH
    if (commandBatchActive && commandBatchError) {
        cliPrintCommandBatchWarning("PLEASE FIX ERRORS THEN 'SAVE'");
        resetCommandBatch();
        return;
    }
#endif

    cliPrint("Saving");
    //copyCurrentProfileToProfileSlot(getConfigProfile();
    suspendRxSignal();
    writeEEPROM();
    resumeRxSignal();
    cliReboot();
}

static void cliDefaults(char *cmdline)
{
    UNUSED(cmdline);

    cliPrint("Resetting to defaults");
    resetEEPROM();
    suspendRxSignal();
    writeEEPROM();
    resumeRxSignal();

#ifdef USE_CLI_BATCH
    commandBatchError = false;
#endif

    if (!checkCommand(cmdline, "noreboot"))
        cliReboot();
}

static void cliGet(char *cmdline)
{
    const setting_t *val;
    int matchedCommands = 0;
    char name[SETTING_MAX_NAME_LENGTH];

    while(*cmdline == ' ') ++cmdline; // ignore spaces

    for (uint32_t i = 0; i < SETTINGS_TABLE_COUNT; i++) {
        val = settingGet(i);
        if (settingNameContains(val, name, cmdline)) {
            cliPrintf("%s = ", name);
            if (strcmp(name, "name") == 0) {
                // if the craftname has a leading space, then enclose the name in quotes
                const char * v = (const char *)settingGetValuePointer(val);
                cliPrintf(v[0] == ' ' ? "\"%s\"" : "%s", v);
            } else {
                cliPrintVar(val, 0);
            }
            cliPrintLinefeed();
            cliPrintVarRange(val);
            cliPrintLinefeed();

            matchedCommands++;
        }
    }


    if (matchedCommands) {
        return;
    }

    cliPrintErrorLine("Invalid name");
}

static void cliSet(char *cmdline)
{
    uint32_t len;
    const setting_t *val;
    char *eqptr = NULL;
    char name[SETTING_MAX_NAME_LENGTH];

    while(*cmdline == ' ') ++cmdline; // ignore spaces

    len = strlen(cmdline);

    if (len == 0 || (len == 1 && cmdline[0] == '*')) {
        cliPrintLine("Current settings:");
        for (uint32_t i = 0; i < SETTINGS_TABLE_COUNT; i++) {
            val = settingGet(i);
            settingGetName(val, name);
            cliPrintf("%s = ", name);
            cliPrintVar(val, len); // when len is 1 (when * is passed as argument), it will print min/max values as well, for gui
            cliPrintLinefeed();
        }
    } else if ((eqptr = strstr(cmdline, "=")) != NULL) {
        // has equals

        char *lastNonSpaceCharacter = eqptr;
        while (*(lastNonSpaceCharacter - 1) == ' ') {
            lastNonSpaceCharacter--;
        }
        uint8_t variableNameLength = lastNonSpaceCharacter - cmdline;

        // skip the '=' and any ' ' characters
        eqptr++;
        while (*(eqptr) == ' ') {
            eqptr++;
        }

        for (uint32_t i = 0; i < SETTINGS_TABLE_COUNT; i++) {
            val = settingGet(i);
            // ensure exact match when setting to prevent setting variables with shorter names
            if (settingNameIsExactMatch(val, name, cmdline, variableNameLength)) {
                const setting_type_e type = SETTING_TYPE(val);
                if (type == VAR_STRING) {
                    // Convert strings to uppercase. Lower case is not supported by the OSD.
                    sl_toupperptr(eqptr);
                    // if setting the craftname, remove any quotes around the name.  This allows leading spaces in the name
                    if ((strcmp(name, "name") == 0 || strcmp(name, "pilot_name") == 0) && (eqptr[0] == '"' && eqptr[strlen(eqptr)-1] == '"')) {
                        settingSetString(val, eqptr + 1, strlen(eqptr)-2);
                    } else {
                        settingSetString(val, eqptr, strlen(eqptr));
                    }
                    return;
                }
                const setting_mode_e mode = SETTING_MODE(val);
                bool changeValue = false;
                int_float_value_t tmp = {0};
                switch (mode) {
                case MODE_DIRECT: {
                        if (*eqptr != 0 && strspn(eqptr, "0123456789.+-") == strlen(eqptr)) {
                            float valuef = fastA2F(eqptr);
                            // note: compare float values
                            if (valuef >= (float)settingGetMin(val) && valuef <= (float)settingGetMax(val)) {

                                if (type == VAR_FLOAT)
                                    tmp.float_value = valuef;
                                else if (type == VAR_UINT32)
                                    tmp.uint_value = fastA2UL(eqptr);
                                else
                                    tmp.int_value = fastA2I(eqptr);

                                changeValue = true;
                            }
                        }
                    }
                    break;
                case MODE_LOOKUP: {
                        const lookupTableEntry_t *tableEntry = settingLookupTable(val);
                        bool matched = false;
                        for (uint32_t tableValueIndex = 0; tableValueIndex < tableEntry->valueCount && !matched; tableValueIndex++) {
                            matched = sl_strcasecmp(tableEntry->values[tableValueIndex], eqptr) == 0;

                            if (matched) {
                                tmp.int_value = tableValueIndex;
                                changeValue = true;
                            }
                        }
                    }
                    break;
                }

                if (changeValue) {
                    // If changing the battery capacity unit, update the osd stats energy unit to match
                    if (strcmp(name, "battery_capacity_unit") == 0) {
                        if (batteryMetersConfig()->capacity_unit != (uint8_t)tmp.int_value) {
                            if (tmp.int_value == BAT_CAPACITY_UNIT_MAH) {
                                osdConfigMutable()->stats_energy_unit = OSD_STATS_ENERGY_UNIT_MAH;
                            } else {
                                osdConfigMutable()->stats_energy_unit = OSD_STATS_ENERGY_UNIT_WH;
                            }
                        }
                    }

                    cliSetIntFloatVar(val, tmp);

                    cliPrintf("%s set to ", name);
                    cliPrintVar(val, 0);
                } else {
                    cliPrintError("Invalid value. ");
                    cliPrintVarRange(val);
                    cliPrintLinefeed();
                }

                return;
            }
        }
        cliPrintErrorLine("Invalid name");
    } else {
        // no equals, check for matching variables.
        cliGet(cmdline);
    }
}

static const char * getBatteryStateString(void)
{
    static const char * const batteryStateStrings[] = {"OK", "WARNING", "CRITICAL", "NOT PRESENT"};

    return batteryStateStrings[getBatteryState()];
}

static void cliStatus(char *cmdline)
{
    UNUSED(cmdline);

    char buf[MAX(FORMATTED_DATE_TIME_BUFSIZE, SETTING_MAX_NAME_LENGTH)];
    dateTime_t dt;

    cliPrintLinef("%s/%s %s %s / %s (%s) %s",
        FC_FIRMWARE_NAME,
        targetName,
        FC_VERSION_STRING,
        buildDate,
        buildTime,
        shortGitRevision,
        FC_VERSION_TYPE
    );
    cliPrintLinef("GCC-%s",
        compilerVersion
    );
    cliPrintLinef("System Uptime: %d seconds", millis() / 1000);
    rtcGetDateTime(&dt);
    dateTimeFormatLocal(buf, &dt);
    cliPrintLinef("Current Time: %s", buf);
    cliPrintLinef("Voltage: %d.%02dV (%dS battery - %s)", getBatteryVoltage() / 100, getBatteryVoltage() % 100, getBatteryCellCount(), getBatteryStateString());
    cliPrintf("CPU Clock=%dMHz", (SystemCoreClock / 1000000));

    const uint32_t detectedSensorsMask = sensorsMask();

    for (int i = 0; i < SENSOR_INDEX_COUNT; i++) {

        const uint32_t mask = (1 << i);
        if ((detectedSensorsMask & mask) && (mask & SENSOR_NAMES_MASK)) {
            const int sensorHardwareIndex = detectedSensors[i];
            if (sensorHardwareNames[i]) {
                const char *sensorHardware = sensorHardwareNames[i][sensorHardwareIndex];
                cliPrintf(", %s=%s", sensorTypeNames[i], sensorHardware);
            }
        }
    }
    cliPrintLinefeed();
#if !defined(SITL_BUILD)
#if defined(AT32F43x)
    cliPrintLine("AT32 system clocks:");
    crm_clocks_freq_type clocks;
    crm_clocks_freq_get(&clocks);

    cliPrintLinef("  SYSCLK = %d MHz", clocks.sclk_freq / 1000000);
    cliPrintLinef("  ABH    = %d MHz", clocks.ahb_freq  / 1000000);
    cliPrintLinef("  ABP1   = %d MHz", clocks.apb1_freq / 1000000);
    cliPrintLinef("  ABP2   = %d MHz", clocks.apb2_freq / 1000000);
#else
    cliPrintLine("STM32 system clocks:");
#if defined(USE_HAL_DRIVER)
    cliPrintLinef("  SYSCLK = %d MHz", HAL_RCC_GetSysClockFreq() / 1000000);
    cliPrintLinef("  HCLK   = %d MHz", HAL_RCC_GetHCLKFreq() / 1000000);
    cliPrintLinef("  PCLK1  = %d MHz", HAL_RCC_GetPCLK1Freq() / 1000000);
    cliPrintLinef("  PCLK2  = %d MHz", HAL_RCC_GetPCLK2Freq() / 1000000);
#else
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);
    cliPrintLinef("  SYSCLK = %d MHz", clocks.SYSCLK_Frequency / 1000000);
    cliPrintLinef("  HCLK   = %d MHz", clocks.HCLK_Frequency / 1000000);
    cliPrintLinef("  PCLK1  = %d MHz", clocks.PCLK1_Frequency / 1000000);
    cliPrintLinef("  PCLK2  = %d MHz", clocks.PCLK2_Frequency / 1000000);
#endif
#endif // for if at32
#endif // for SITL

    cliPrintLinef("Sensor status: GYRO=%s, ACC=%s, MAG=%s, BARO=%s, RANGEFINDER=%s, OPFLOW=%s, GPS=%s",
        hardwareSensorStatusNames[getHwGyroStatus()],
        hardwareSensorStatusNames[getHwAccelerometerStatus()],
        hardwareSensorStatusNames[getHwCompassStatus()],
        hardwareSensorStatusNames[getHwBarometerStatus()],
        hardwareSensorStatusNames[getHwRangefinderStatus()],
        hardwareSensorStatusNames[getHwOpticalFlowStatus()],
        hardwareSensorStatusNames[getHwGPSStatus()]
    );

#ifdef USE_ESC_SENSOR
    uint8_t motorCount = getMotorCount();
    if (STATE(ESC_SENSOR_ENABLED) && motorCount > 0) {
        cliPrintLinef("ESC Temperature(s): Motor Count = %d", motorCount);
        for (uint8_t i = 0; i < motorCount; i++) {
            const escSensorData_t *escState = getEscTelemetry(i); //Get ESC telemetry
            cliPrintf("ESC %d: %d\260C, ", i, escState->temperature);
        }
        cliPrintLinefeed();
    }
#endif

#ifdef USE_SDCARD
    cliSdInfo(NULL);
#endif
#ifdef USE_I2C
    const uint16_t i2cErrorCounter = i2cGetErrorCounter();
#elif !defined(SITL_BUILD)
    const uint16_t i2cErrorCounter = 0;
#endif

#ifdef STACK_CHECK
    cliPrintf("Stack used: %d, ", stackUsedSize());
#endif
#if !defined(SITL_BUILD)
    cliPrintLinef("Stack size: %d, Stack address: 0x%x, Heap available: %d", stackTotalSize(), stackHighMem(), memGetAvailableBytes());

    cliPrintLinef("I2C Errors: %d, config size: %d, max available config: %d", i2cErrorCounter, getEEPROMConfigSize(), &__config_end - &__config_start);
#endif
#if defined(USE_ADC) && !defined(SITL_BUILD)
    static char * adcFunctions[] = { "BATTERY", "RSSI", "CURRENT", "AIRSPEED" };
    cliPrintLine("ADC channel usage:");
    for (int i = 0; i < ADC_FUNCTION_COUNT; i++) {
        cliPrintf("  %8s :", adcFunctions[i]);

        cliPrint(" configured = ");
        if (adcChannelConfig()->adcFunctionChannel[i] == ADC_CHN_NONE) {
            cliPrint("none");
        }
        else {
            cliPrintf("ADC %d", adcChannelConfig()->adcFunctionChannel[i]);
        }

        cliPrint(", used = ");
        if (adcGetFunctionChannelAllocation(i) == ADC_CHN_NONE) {
            cliPrintLine("none");
        }
        else {
            cliPrintLinef("ADC %d", adcGetFunctionChannelAllocation(i));
        }
    }
#endif

    cliPrintf("System load: %d", averageSystemLoadPercent);
    const timeDelta_t pidTaskDeltaTime = getTaskDeltaTime(TASK_PID);
    const int pidRate = pidTaskDeltaTime == 0 ? 0 : (int)(1000000.0f / ((float)pidTaskDeltaTime));
    const int rxRate = getTaskDeltaTime(TASK_RX) == 0 ? 0 : (int)(1000000.0f / ((float)getTaskDeltaTime(TASK_RX)));
    const int systemRate = getTaskDeltaTime(TASK_SYSTEM) == 0 ? 0 : (int)(1000000.0f / ((float)getTaskDeltaTime(TASK_SYSTEM)));
    cliPrintLinef(", cycle time: %d, PID rate: %d, RX rate: %d, System rate: %d",  (uint16_t)cycleTime, pidRate, rxRate, systemRate);
#if !defined(CLI_MINIMAL_VERBOSITY)
    cliPrint("Arming disabled flags:");
    uint32_t flags = armingFlags & ARMING_DISABLED_ALL_FLAGS;
    while (flags) {
        int bitpos = ffs(flags) - 1;
        flags &= ~(1 << bitpos);
	if (bitpos > 6) cliPrintf(" %s", armingDisableFlagNames[bitpos - 7]);
    }
    cliPrintLinefeed();
    if (armingFlags & ARMING_DISABLED_INVALID_SETTING) {
        unsigned invalidIndex;
        if (!settingsValidate(&invalidIndex)) {
            settingGetName(settingGet(invalidIndex), buf);
            cliPrintErrorLinef("Invalid setting: %s", buf);
        }
    }

#if defined(USE_OSD)
    if (armingFlags & ARMING_DISABLED_NAVIGATION_UNSAFE) {
	    navArmingBlocker_e reason = navigationIsBlockingArming(NULL);
        if (reason & NAV_ARMING_BLOCKER_JUMP_WAYPOINT_ERROR)
            cliPrintLinef("  %s", OSD_MSG_JUMP_WP_MISCONFIG);
        if (reason & NAV_ARMING_BLOCKER_MISSING_GPS_FIX) {
            cliPrintLinef("  %s", OSD_MSG_WAITING_GPS_FIX);
		} else {
            if (reason & NAV_ARMING_BLOCKER_NAV_IS_ALREADY_ACTIVE)
                cliPrintLinef("  %s", OSD_MSG_DISABLE_NAV_FIRST);
            if (reason & NAV_ARMING_BLOCKER_FIRST_WAYPOINT_TOO_FAR)
                cliPrintLinef("  FIRST WP TOO FAR");
       }
    }
#endif


#else
    cliPrintLinef("Arming disabled flags: 0x%lx", armingFlags & ARMING_DISABLED_ALL_FLAGS);
#endif

#if !defined(CLI_MINIMAL_VERBOSITY)
    cliPrint("OSD: ");
#if defined(USE_OSD)
    displayPort_t *osdDisplayPort = osdGetDisplayPort();
    if (osdDisplayPort != NULL) {
        cliPrintf("%s [%u x %u]", osdDisplayPort->displayPortType, osdDisplayPort->cols, osdDisplayPort->rows);
    } else {
        cliPrint("not enabled");
    }
#else
    cliPrint("not used");
#endif
    cliPrintLinefeed();

    cliPrint("VTX: ");
#if defined(USE_VTX_CONTROL)
    if (vtxCommonDeviceIsReady(vtxCommonDevice())) {
        vtxDeviceOsdInfo_t osdInfo;
        vtxCommonGetOsdInfo(vtxCommonDevice(), &osdInfo);
        cliPrintf("band: %c, chan: %s, power: %c", osdInfo.bandLetter, osdInfo.channelName, osdInfo.powerIndexLetter);

        if (osdInfo.powerMilliwatt) {
            cliPrintf(" (%d mW)", osdInfo.powerMilliwatt);
        }

        if (osdInfo.frequency) {
            cliPrintf(", freq: %d MHz", osdInfo.frequency);
        }
    }
    else {
        cliPrint("not detected");
    }
#else
    cliPrint("no VTX control");
#endif

    cliPrintLinefeed();
#endif

    if (featureConfigured(FEATURE_GPS) && isGpsUblox()) {
        cliPrint("GPS: ");
        cliPrintf("HW Version: %s Proto: %d.%02d Baud: %d", getGpsHwVersion(), getGpsProtoMajorVersion(), getGpsProtoMinorVersion(), getGpsBaudrate());
        if(ubloxVersionLT(15, 0)) {
            cliPrintf(" (UBLOX Proto >= 15.0 required)");
        }
        cliPrintLinefeed();
        cliPrintLinef("  SATS: %i", gpsSol.numSat);
        cliPrintLinef("  HDOP: %f", (double)(gpsSol.hdop / (float)HDOP_SCALE));
        cliPrintLinef("  EPH : %f m", (double)(gpsSol.eph / 100.0f));
        cliPrintLinef("  EPV : %f m", (double)(gpsSol.epv / 100.0f));
        //cliPrintLinef("  GNSS Capabilities: %d", gpsUbloxCapLastUpdate());
        cliPrintLinef("  GNSS Capabilities:");
        cliPrintLine("    GNSS Provider active/default");
        cliPrintLine("    GPS 1/1");
        if(gpsUbloxHasGalileo())
            cliPrintLinef("    Galileo %d/%d", gpsUbloxGalileoEnabled(), gpsUbloxGalileoDefault());
        if(gpsUbloxHasBeidou())
            cliPrintLinef("    BeiDou %d/%d", gpsUbloxBeidouEnabled(), gpsUbloxBeidouDefault());
        if(gpsUbloxHasGlonass())
            cliPrintLinef("    Glonass %d/%d", gpsUbloxGlonassEnabled(), gpsUbloxGlonassDefault());
        cliPrintLinef("    Max concurrent: %d", gpsUbloxMaxGnss());
    }

    // If we are blocked by PWM init - provide more information
    if (getPwmInitError() != PWM_INIT_ERROR_NONE) {
        cliPrintLinef("PWM output init error: %s", getPwmInitErrorMessage());
    }
}

static void cliTasks(char *cmdline)
{
    UNUSED(cmdline);
    int maxLoadSum = 0;
    int averageLoadSum = 0;
    cfCheckFuncInfo_t checkFuncInfo;

    cliPrintLinef("Task list         rate/hz  max/us  avg/us maxload avgload     total/ms");
    for (cfTaskId_e taskId = 0; taskId < TASK_COUNT; taskId++) {
        cfTaskInfo_t taskInfo;
        getTaskInfo(taskId, &taskInfo);
        if (taskInfo.isEnabled) {
            const int taskFrequency = taskInfo.latestDeltaTime == 0 ? 0 : (int)(1000000.0f / ((float)taskInfo.latestDeltaTime));
            const int maxLoad = (taskInfo.maxExecutionTime * taskFrequency + 5000) / 1000;
            const int averageLoad = (taskInfo.averageExecutionTime * taskFrequency + 5000) / 1000;
            if (taskId != TASK_SERIAL) {
                maxLoadSum += maxLoad;
                averageLoadSum += averageLoad;
            }
            cliPrintLinef("%2d - %12s  %6d   %5d   %5d %4d.%1d%% %4d.%1d%%  %8d",
                    taskId, taskInfo.taskName, taskFrequency, (uint32_t)taskInfo.maxExecutionTime, (uint32_t)taskInfo.averageExecutionTime,
                    maxLoad/10, maxLoad%10, averageLoad/10, averageLoad%10, (uint32_t)taskInfo.totalExecutionTime / 1000);
        }
    }
    getCheckFuncInfo(&checkFuncInfo);
    cliPrintLinef("Task check function %13d %7d %25d", (uint32_t)checkFuncInfo.maxExecutionTime, (uint32_t)checkFuncInfo.averageExecutionTime, (uint32_t)checkFuncInfo.totalExecutionTime / 1000);
    cliPrintLinef("Total (excluding SERIAL) %21d.%1d%% %4d.%1d%%", maxLoadSum/10, maxLoadSum%10, averageLoadSum/10, averageLoadSum%10);
}

static void cliVersion(char *cmdline)
{
    UNUSED(cmdline);

    cliPrintLinef("# %s/%s %s %s / %s (%s) %s",
        FC_FIRMWARE_NAME,
        targetName,
        FC_VERSION_STRING,
        buildDate,
        buildTime,
        shortGitRevision,
        FC_VERSION_TYPE
    );
    cliPrintLinef("# GCC-%s",
        compilerVersion
    );
}

static void cliMemory(char *cmdline)
{
    UNUSED(cmdline);
    cliPrintLinef("Dynamic memory usage:");
    for (unsigned i = 0; i < OWNER_TOTAL_COUNT; i++) {
        const char * owner = ownerNames[i];
        const uint32_t memUsed = memGetUsedBytesByOwner(i);

        if (memUsed) {
            cliPrintLinef("%s : %d bytes", owner, memUsed);
        }
    }
}

static void cliResource(char *cmdline)
{
    UNUSED(cmdline);
    cliPrintLinef("IO:\r\n----------------------");
    for (int i = 0; i < DEFIO_IO_USED_COUNT; i++) {
        const char* owner;
        owner = ownerNames[ioRecs[i].owner];

        const char* resource;
        resource = resourceNames[ioRecs[i].resource];

        if (ioRecs[i].index > 0) {
            cliPrintLinef("%c%02d: %s%d %s", IO_GPIOPortIdx(ioRecs + i) + 'A', IO_GPIOPinIdx(ioRecs + i), owner, ioRecs[i].index, resource);
        } else {
            cliPrintLinef("%c%02d: %s %s", IO_GPIOPortIdx(ioRecs + i) + 'A', IO_GPIOPinIdx(ioRecs + i), owner, resource);
        }
    }
}

static void backupConfigs(void)
{
    // make copies of configs to do differencing
    PG_FOREACH(pg) {
        if (pgIsProfile(pg)) {
            memcpy(pg->copy, pg->address, pgSize(pg) * MAX_PROFILE_COUNT);
        } else {
            memcpy(pg->copy, pg->address, pgSize(pg));
        }
    }
}

static void restoreConfigs(void)
{
    PG_FOREACH(pg) {
        if (pgIsProfile(pg)) {
            memcpy(pg->address, pg->copy, pgSize(pg) * MAX_PROFILE_COUNT);
        } else {
            memcpy(pg->address, pg->copy, pgSize(pg));
        }
    }
}

static void printConfig(const char *cmdline, bool doDiff)
{
    uint8_t dumpMask = DUMP_MASTER;
    const char *options;
    if ((options = checkCommand(cmdline, "master"))) {
        dumpMask = DUMP_MASTER; // only
    } else if ((options = checkCommand(cmdline, "control_profile"))) {
        dumpMask = DUMP_CONTROL_PROFILE; // only
    } else if ((options = checkCommand(cmdline, "mixer_profile"))) {
        dumpMask = DUMP_MIXER_PROFILE; // only
    } else if ((options = checkCommand(cmdline, "battery_profile"))) {
        dumpMask = DUMP_BATTERY_PROFILE; // only
    } else if ((options = checkCommand(cmdline, "all"))) {
        dumpMask = DUMP_ALL;   // all profiles and rates
    } else {
        options = cmdline;
    }

    if (doDiff) {
        dumpMask = dumpMask | DO_DIFF;
    }

    const int currentControlProfileIndexSave = getConfigProfile();
    const int currentMixerProfileIndexSave = getConfigMixerProfile();
    const int currentBatteryProfileIndexSave = getConfigBatteryProfile();
    backupConfigs();
    // reset all configs to defaults to do differencing
    resetConfigs();
    // restore the profile indices, since they should not be reset for proper comparison
    setConfigProfile(currentControlProfileIndexSave);
    setConfigMixerProfile(currentMixerProfileIndexSave);
    setConfigBatteryProfile(currentBatteryProfileIndexSave);

    if (checkCommand(options, "showdefaults")) {
        dumpMask = dumpMask | SHOW_DEFAULTS;   // add default values as comments for changed values
    }

#ifdef USE_CLI_BATCH
    bool batchModeEnabled = false;
#endif

    if ((dumpMask & DUMP_MASTER) || (dumpMask & DUMP_ALL)) {
        cliPrintHashLine("version");
        cliVersion(NULL);

#ifdef USE_CLI_BATCH
        cliPrintHashLine("start the command batch");
        cliPrintLine("batch start");
        batchModeEnabled = true;
#endif

        if ((dumpMask & (DUMP_ALL | DO_DIFF)) == (DUMP_ALL | DO_DIFF)) {
#ifndef CLI_MINIMAL_VERBOSITY
            cliPrintHashLine("reset configuration to default settings\r\ndefaults noreboot");
#else
            cliPrintLinef("defaults noreboot");
#endif
        }

        cliPrintHashLine("resources");
        //printResource(dumpMask, &defaultConfig);

        cliPrintHashLine("Timer overrides");
        printTimerOutputModes(dumpMask, timerOverrides_CopyArray, timerOverrides(0), -1);

        // print servo parameters
        cliPrintHashLine("Outputs [servo]");
        printServo(dumpMask, servoParams_CopyArray, servoParams(0));

#if defined(USE_SAFE_HOME)
        cliPrintHashLine("safehome");
        printSafeHomes(dumpMask, safeHomeConfig_CopyArray, safeHomeConfig(0));
#endif

#ifdef USE_FW_AUTOLAND
        cliPrintHashLine("Fixed Wing Approach");
        printFwAutolandApproach(dumpMask, fwAutolandApproachConfig_CopyArray, fwAutolandApproachConfig(0));
#endif

#if defined(USE_GEOZONE)
        cliPrintHashLine("geozone");
        printGeozones(dumpMask, geoZonesConfig_CopyArray, geoZonesConfig(0));

        cliPrintHashLine("geozone vertices");
        printGeozoneVertices(dumpMask, geoZoneVertices_CopyArray, geoZoneVertices(0));
#endif

        cliPrintHashLine("features");
        printFeature(dumpMask, &featureConfig_Copy, featureConfig());

#if defined(BEEPER) || defined(USE_DSHOT)
        cliPrintHashLine("beeper");
        printBeeper(dumpMask, &beeperConfig_Copy, beeperConfig());
#endif

#ifdef USE_BLACKBOX
        cliPrintHashLine("blackbox");
        printBlackbox(dumpMask, &blackboxConfig_Copy, blackboxConfig());
#endif

        cliPrintHashLine("Receiver: Channel map");
        printMap(dumpMask, &rxConfig_Copy, rxConfig());

        cliPrintHashLine("Ports");
        printSerial(dumpMask, &serialConfig_Copy, serialConfig());

#ifdef USE_LED_STRIP
        cliPrintHashLine("LEDs");
        printLed(dumpMask, ledStripConfig_Copy.ledConfigs, ledStripConfig()->ledConfigs);

        cliPrintHashLine("LED color");
        printColor(dumpMask, ledStripConfig_Copy.colors, ledStripConfig()->colors);

        cliPrintHashLine("LED mode_color");
        printModeColor(dumpMask, &ledStripConfig_Copy, ledStripConfig());
#endif

        cliPrintHashLine("Modes [aux]");
        printAux(dumpMask, modeActivationConditions_CopyArray, modeActivationConditions(0));

        cliPrintHashLine("Adjustments [adjrange]");
        printAdjustmentRange(dumpMask, adjustmentRanges_CopyArray, adjustmentRanges(0));

        cliPrintHashLine("Receiver rxrange");
        printRxRange(dumpMask, rxChannelRangeConfigs_CopyArray, rxChannelRangeConfigs(0));

#ifdef USE_TEMPERATURE_SENSOR
        cliPrintHashLine("temp_sensor");
        printTempSensor(dumpMask, tempSensorConfig_CopyArray, tempSensorConfig(0));
#endif

#if defined(NAV_NON_VOLATILE_WAYPOINT_STORAGE) && defined(NAV_NON_VOLATILE_WAYPOINT_CLI)
        cliPrintHashLine("Mission Control Waypoints [wp]");
        printWaypoints(dumpMask, posControl.waypointList, nonVolatileWaypointList(0));
#endif

#ifdef USE_OSD
        cliPrintHashLine("OSD [osd_layout]");
        printOsdLayout(dumpMask, &osdLayoutsConfig_Copy, osdLayoutsConfig(), -1, -1);
#endif

#ifdef USE_PROGRAMMING_FRAMEWORK
        cliPrintHashLine("Programming: logic");
        printLogic(dumpMask, logicConditions_CopyArray, logicConditions(0), -1);

        cliPrintHashLine("Programming: global variables");
        printGvar(dumpMask, globalVariableConfigs_CopyArray, globalVariableConfigs(0));

        cliPrintHashLine("Programming: PID controllers");
        printPid(dumpMask, programmingPids_CopyArray, programmingPids(0));
#endif
#ifdef USE_PROGRAMMING_FRAMEWORK
        cliPrintHashLine("OSD: custom elements");
        printOsdCustomElements(dumpMask, osdCustomElements_CopyArray, osdCustomElements(0));
#endif

        cliPrintHashLine("master");
        dumpAllValues(MASTER_VALUE, dumpMask);

        if (dumpMask & DUMP_ALL) {
            // dump all profiles
            const int currentControlProfileIndexSave = getConfigProfile();
            const int currentMixerProfileIndexSave = getConfigMixerProfile();
            const int currentBatteryProfileIndexSave = getConfigBatteryProfile();
            for (int ii = 0; ii < MAX_PROFILE_COUNT; ++ii) {
                cliDumpControlProfile(ii, dumpMask);
            }
            for (int ii = 0; ii < MAX_MIXER_PROFILE_COUNT; ++ii) {
                cliDumpMixerProfile(ii, dumpMask);
            }
            for (int ii = 0; ii < MAX_BATTERY_PROFILE_COUNT; ++ii) {
                cliDumpBatteryProfile(ii, dumpMask);
            }
            setConfigProfile(currentControlProfileIndexSave);
            setConfigMixerProfile(currentMixerProfileIndexSave);
            setConfigBatteryProfile(currentBatteryProfileIndexSave);

            cliPrintHashLine("restore original profile selection");
            cliPrintLinef("control_profile %d", currentControlProfileIndexSave + 1);
            cliPrintLinef("mixer_profile %d", currentMixerProfileIndexSave + 1);
            cliPrintLinef("battery_profile %d", currentBatteryProfileIndexSave + 1);

#ifdef USE_CLI_BATCH
            batchModeEnabled = false;
#endif
        } else {
            // dump just the current profiles
            cliDumpControlProfile(getConfigProfile(), dumpMask);
            cliDumpMixerProfile(getConfigMixerProfile(), dumpMask);
            cliDumpBatteryProfile(getConfigBatteryProfile(), dumpMask);
        }
    }

    if (dumpMask & DUMP_CONTROL_PROFILE) {
        cliDumpControlProfile(getConfigProfile(), dumpMask);
    }

    if (dumpMask & DUMP_MIXER_PROFILE) {
        cliDumpMixerProfile(getConfigMixerProfile(), dumpMask);
    }

    if (dumpMask & DUMP_BATTERY_PROFILE) {
        cliDumpBatteryProfile(getConfigBatteryProfile(), dumpMask);
    }

    if ((dumpMask & DUMP_MASTER) || (dumpMask & DUMP_ALL)) {
        cliPrintHashLine("save configuration\r\nsave");
    }

#ifdef USE_CLI_BATCH
    if (batchModeEnabled) {
        cliPrintHashLine("end the command batch");
        cliPrintLine("batch end");
    }
#endif

    // restore configs from copies
    restoreConfigs();
}

static void cliDump(char *cmdline)
{
    printConfig(cmdline, false);
}

static void cliDiff(char *cmdline)
{
    printConfig(cmdline, true);
}

#ifdef USE_USB_MSC
static void cliMsc(char *cmdline)
{
    UNUSED(cmdline);

    if (false
#ifdef USE_SDCARD
        || sdcard_isFunctional()
#endif
#ifdef USE_FLASHFS
        || flashfsGetSize() > 0
#endif
    ) {
        cliPrintHashLine("restarting in mass storage mode");
        cliPrint("\r\nRebooting");
        bufWriterFlush(cliWriter);
        delay(1000);
        waitForSerialPortToFinishTransmitting(cliPort);
        stopPwmAllMotors();
        systemResetRequest(RESET_MSC_REQUEST);
    } else {
        cliPrint("\r\nStorage not present or failed to initialize!");
        bufWriterFlush(cliWriter);
    }
}
#endif


typedef struct {
    const char *name;
#ifndef SKIP_CLI_COMMAND_HELP
    const char *description;
    const char *args;
#endif
    void (*func)(char *cmdline);
} clicmd_t;

#ifndef SKIP_CLI_COMMAND_HELP
#define CLI_COMMAND_DEF(name, description, args, method) \
{ \
    name , \
    description , \
    args , \
    method \
}
#else
#define CLI_COMMAND_DEF(name, description, args, method) \
{ \
    name, \
    method \
}
#endif

static void cliCmdDebug(char *arg)
{
    UNUSED(arg);
    if (debugMode != DEBUG_NONE) {
        cliPrintLinef("Debug fields: [%s (%i)]", debugMode < DEBUG_COUNT ? debugModeNames[debugMode] : "unknown", debugMode);
        for (int i = 0; i < DEBUG32_VALUE_COUNT; i++) {
            cliPrintLinef("debug[%d] = %d", i, debug[i]);
        }
    } else {
        cliPrintLine("Debug mode is disabled");
    }
}


#if defined(USE_GPS) && defined(USE_GPS_PROTO_UBLOX)

static const char* _ubloxGetSigId(uint8_t gnssId, uint8_t sigId)
{
    if(gnssId == 0) {
        switch(sigId) {
            case 0: return "GPS L1C/A";
            case 3: return "GPS L2 CL";
            case 4: return "GPS L2 CM";
            case 6: return "GPS L5 I";
            case 7: return "GPS L5 Q";
            default: return "GPS Unknown";
        }
    } else if(gnssId == 1) {
        switch(sigId) {
            case 0: return "SBAS L1C/A";
            default: return "SBAS Unknown";
        }
    } else if(gnssId == 2) {
        switch(sigId) {
            case 0: return "Galileo E1 C";
            case 1: return "Galileo E1 B";
            case 3: return "Galileo E5 al";
            case 4: return "Galileo E5 aQ";
            case 5: return "Galileo E5 bl";
            case 6: return "Galileo E5 bQ";
            default: return "Galileo Unknown";
        }
    } else if(gnssId == 3) {
        switch(sigId) {
            case 0: return "BeiDou B1I D1";
            case 1: return "BeiDou B1I D2";
            case 2: return "BeiDou B2I D1";
            case 3: return "BeiDou B2I D2";
            case 5: return "BeiDou B1C";
            case 7: return "BeiDou B2a";
            default: return "BeiDou Unknown";
        }
    } else if(gnssId == 5) {
        switch(sigId) {
            case 0: return "QZSS L1C/A";
            case 1: return "QZSS L1S";
            case 4: return "QZSS L2 CM";
            case 5: return "QZSS L2 CL";
            case 8: return "QZSS L5 I";
            case 9: return "QZSS L5 Q";
            default: return "QZSS Unknown";
        }
    } else if(gnssId == 6) {
        switch(sigId) {
            case 0: return "GLONASS L1 OF";
            case 2: return "GLONASS L2 OF";
            default: return "GLONASS Unknown";
        }
    }

    return "Unknown GNSS/SigId";
}

static const char *_ubloxGetQuality(uint8_t quality)
{
    switch(quality) {
        case UBLOX_SIG_QUALITY_NOSIGNAL: return "No signal";
        case UBLOX_SIG_QUALITY_SEARCHING: return "Searching signal...";
        case UBLOX_SIG_QUALITY_ACQUIRED: return "Signal acquired";
        case UBLOX_SIG_QUALITY_UNUSABLE: return "Signal detected but unusable";
        case UBLOX_SIG_QUALITY_CODE_LOCK_TIME_SYNC: return "Code locked and time sync";
        case UBLOX_SIG_QUALITY_CODE_CARRIER_LOCK_TIME_SYNC:
        case UBLOX_SIG_QUALITY_CODE_CARRIER_LOCK_TIME_SYNC2:
        case UBLOX_SIG_QUALITY_CODE_CARRIER_LOCK_TIME_SYNC3:
            return "Code and carrier locked and time sync";
        default: return "Unknown";
    }
}

static void cliUbloxPrintSatelites(char *arg)
{
    UNUSED(arg);
    if(!isGpsUblox() /*|| !(gpsState.flags.sig || gpsState.flags.sat)*/) {
        cliPrint("GPS is not UBLOX or does not report satelites.");
        return;
    }

    cliPrintLine("UBLOX Satelites");

    for(int i = 0; i < UBLOX_MAX_SIGNALS; ++i)
    {
        const ubx_nav_sig_info *sat = gpsGetUbloxSatelite(i);
        if(sat == NULL) {
            continue;
        }

        cliPrintLinef("satelite[%d]: %d:%d", i+1, sat->gnssId, sat->svId);
        cliPrintLinef("sigId: %d (%s)", sat->sigId, _ubloxGetSigId(sat->gnssId, sat->sigId));
        cliPrintLinef("signal strength: %i dbHz", sat->cno);
        cliPrintLinef("quality: %i (%s)", sat->quality, _ubloxGetQuality(sat->quality));
        //cliPrintLinef("Correlation: %i", sat->corrSource);
        //cliPrintLinef("Iono model: %i", sat->ionoModel);
        cliPrintLinef("signal flags: 0x%02X", sat->sigFlags);
        switch(sat->sigFlags & UBLOX_SIG_HEALTH_MASK) {
            case UBLOX_SIG_HEALTH_HEALTHY:
                cliPrintLine("signal: Healthy");
                break;
            case UBLOX_SIG_HEALTH_UNHEALTHY:
                cliPrintLine("signal: Unhealthy");
                break;
            case UBLOX_SIG_HEALTH_UNKNOWN:
            default:
                cliPrintLinef("signal: Unknown (0x%X)", sat->sigFlags & UBLOX_SIG_HEALTH_MASK);
                break;
        }
        cliPrintLinefeed();
    }
}
#endif

static void cliHelp(char *cmdline);

// should be sorted a..z for bsearch()
const clicmd_t cmdTable[] = {
    CLI_COMMAND_DEF("adjrange", "configure adjustment ranges", NULL, cliAdjustmentRange),
#if defined(USE_ASSERT)
    CLI_COMMAND_DEF("assert", "", NULL, cliAssert),
#endif
    CLI_COMMAND_DEF("aux", "configure modes", NULL, cliAux),
#ifdef USE_CLI_BATCH
    CLI_COMMAND_DEF("batch", "start or end a batch of commands", "start | end", cliBatch),
#endif
#if defined(BEEPER) || defined(USE_DSHOT)
    CLI_COMMAND_DEF("beeper", "turn on/off beeper", "list\r\n"
            "\t<+|->[name]", cliBeeper),
#endif
#if defined (USE_SERIALRX_SRXL2)
    CLI_COMMAND_DEF("bind_rx", "initiate binding for RX SPI or SRXL2", NULL, cliRxBind),
#endif
#if defined(USE_BOOTLOG)
    CLI_COMMAND_DEF("bootlog", "show boot events", NULL, cliBootlog),
#endif
#ifdef USE_LED_STRIP
    CLI_COMMAND_DEF("color", "configure colors", NULL, cliColor),
    CLI_COMMAND_DEF("mode_color", "configure mode and special colors", NULL, cliModeColor),
#endif
    CLI_COMMAND_DEF("cli_delay", "CLI Delay", "Delay in ms", cliDelay),
    CLI_COMMAND_DEF("defaults", "reset to defaults and reboot", NULL, cliDefaults),
    CLI_COMMAND_DEF("dfu", "DFU mode on reboot", NULL, cliDfu),
    CLI_COMMAND_DEF("diff", "list configuration changes from default",
        "[master|battery_profile|control_profile|mixer_profile|rates|all] {showdefaults}", cliDiff),
    CLI_COMMAND_DEF("dump", "dump configuration",
        "[master|battery_profile|control_profile|mixer_profile|rates|all] {showdefaults}", cliDump),
#ifdef USE_RX_ELERES
    CLI_COMMAND_DEF("eleres_bind", NULL, NULL, cliEleresBind),
#endif // USE_RX_ELERES
    CLI_COMMAND_DEF("exit", NULL, NULL, cliExit),
    CLI_COMMAND_DEF("feature", "configure features",
        "list\r\n"
        "\t<+|->[name]", cliFeature),
#ifdef USE_BLACKBOX
    CLI_COMMAND_DEF("blackbox", "configure blackbox fields",
        "list\r\n"
        "\t<+|->[name]", cliBlackbox),
#endif
#ifdef USE_FLASHFS
    CLI_COMMAND_DEF("flash_erase", "erase flash chip", NULL, cliFlashErase),
    CLI_COMMAND_DEF("flash_info", "show flash chip info", NULL, cliFlashInfo),
#ifdef USE_FLASH_TOOLS
    CLI_COMMAND_DEF("flash_read", NULL, "<length> <address>", cliFlashRead),
    CLI_COMMAND_DEF("flash_write", NULL, "<address> <message>", cliFlashWrite),
#endif
#endif
#ifdef USE_FW_AUTOLAND
    CLI_COMMAND_DEF("fwapproach", "Fixed Wing Approach Settings", NULL, cliFwAutolandApproach),
#endif
    CLI_COMMAND_DEF("get", "get variable value", "[name]", cliGet),
#ifdef USE_GEOZONE
    CLI_COMMAND_DEF("geozone", "get or set geo zones", NULL, cliGeozone),
#endif
#ifdef USE_GPS
    CLI_COMMAND_DEF("gpspassthrough", "passthrough gps to serial", NULL, cliGpsPassthrough),
    CLI_COMMAND_DEF("gpssats", "show GPS satellites", NULL, cliUbloxPrintSatelites),
#endif
    CLI_COMMAND_DEF("help", NULL, NULL, cliHelp),
#ifdef USE_LED_STRIP
    CLI_COMMAND_DEF("led", "configure leds", NULL, cliLed),
    CLI_COMMAND_DEF("ledpinpwm", "start/stop PWM on LED pin, 0..100 duty ratio", "[<value>]\r\n", cliLedPinPWM),
#endif
    CLI_COMMAND_DEF("map", "configure rc channel order", "[<map>]", cliMap),
    CLI_COMMAND_DEF("memory", "view memory usage", NULL, cliMemory),
    CLI_COMMAND_DEF("mmix", "custom motor mixer", NULL, cliMotorMix),
    CLI_COMMAND_DEF("motor",  "get/set motor", "<index> [<value>]", cliMotor),
#ifdef USE_USB_MSC
    CLI_COMMAND_DEF("msc", "switch into msc mode", NULL, cliMsc),
#endif
    CLI_COMMAND_DEF("play_sound", NULL, "[<index>]\r\n", cliPlaySound),
    CLI_COMMAND_DEF("control_profile", "change control profile", "[<index>]", cliControlProfile),
    CLI_COMMAND_DEF("mixer_profile", "change mixer profile", "[<index>]", cliMixerProfile),
    CLI_COMMAND_DEF("battery_profile", "change battery profile", "[<index>]", cliBatteryProfile),
    CLI_COMMAND_DEF("resource", "view currently used resources", NULL, cliResource),
    CLI_COMMAND_DEF("rxrange", "configure rx channel ranges", NULL, cliRxRange),
#if defined(USE_SAFE_HOME)
    CLI_COMMAND_DEF("safehome", "safe home list", NULL, cliSafeHomes),
#endif
    CLI_COMMAND_DEF("save", "save and reboot", NULL, cliSave),
    CLI_COMMAND_DEF("serial", "configure serial ports", NULL, cliSerial),
#ifdef USE_SERIAL_PASSTHROUGH
    CLI_COMMAND_DEF("serialpassthrough", "passthrough serial data to port", "<id> [baud] [mode] [options]: passthrough to serial", cliSerialPassthrough),
#endif
    CLI_COMMAND_DEF("servo", "configure servos", NULL, cliServo),
#ifdef USE_PROGRAMMING_FRAMEWORK
    CLI_COMMAND_DEF("logic", "configure logic conditions",
        "<rule> <enabled> <activatorId> <operation> <operand A type> <operand A value> <operand B type> <operand B value> <flags>\r\n"
        "\treset\r\n", cliLogic),

    CLI_COMMAND_DEF("gvar", "configure global variables",
        "<gvar> <default> <min> <max>\r\n"
        "\treset\r\n", cliGvar),

    CLI_COMMAND_DEF("pid", "configurable PID controllers",
        "<#> <enabled> <setpoint type> <setpoint value> <measurement type> <measurement value> <P gain> <I gain> <D gain> <FF gain>\r\n"
        "\treset\r\n", cliPid),

    CLI_COMMAND_DEF("osd_custom_elements", "configurable OSD custom elements",
                    "<#> <part0 type> <part0 value> <part1 type> <part1 value> <part2 type> <part2 value> <visibility type> <visibility value> <text>\r\n"
                    , osdCustom),
#endif
    CLI_COMMAND_DEF("set", "change setting", "[<name>=<value>]", cliSet),
    CLI_COMMAND_DEF("smix", "servo mixer",
        "<rule> <servo> <source> <rate> <speed> <conditionId>\r\n"
        "\treset\r\n", cliServoMix),
#ifdef USE_SDCARD
    CLI_COMMAND_DEF("sd_info", "sdcard info", NULL, cliSdInfo),
#endif
    CLI_COMMAND_DEF("showdebug", "Show debug fields.", NULL, cliCmdDebug),
    CLI_COMMAND_DEF("status", "show status", NULL, cliStatus),
    CLI_COMMAND_DEF("tasks", "show task stats", NULL, cliTasks),
#ifdef USE_TEMPERATURE_SENSOR
    CLI_COMMAND_DEF("temp_sensor", "change temp sensor settings", NULL, cliTempSensor),
#endif
    CLI_COMMAND_DEF("version", "show version", NULL, cliVersion),
#if defined(NAV_NON_VOLATILE_WAYPOINT_STORAGE) && defined(NAV_NON_VOLATILE_WAYPOINT_CLI)
    CLI_COMMAND_DEF("wp", "waypoint list", NULL, cliWaypoints),
#endif
#ifdef USE_OSD
    CLI_COMMAND_DEF("osd_layout", "get or set the layout of OSD items", "[<layout> [<item> [<col> <row> [<visible>]]]]", cliOsdLayout),
#endif
    CLI_COMMAND_DEF("timer_output_mode", "get or set the outputmode for a given timer.",  "[<timer> [<AUTO|MOTORS|SERVOS>]]", cliTimerOutputMode),
};

static void cliHelp(char *cmdline)
{
    UNUSED(cmdline);

    for (uint32_t i = 0; i < ARRAYLEN(cmdTable); i++) {
        cliPrint(cmdTable[i].name);
#ifndef SKIP_CLI_COMMAND_HELP
        if (cmdTable[i].description) {
            cliPrintf(" - %s", cmdTable[i].description);
        }
        if (cmdTable[i].args) {
            cliPrintf("\r\n\t%s", cmdTable[i].args);
        }
#endif
        cliPrintLinefeed();
    }
}

void cliProcess(void)
{
    if (!cliWriter) {
        return;
    }

    // Be a little bit tricky.  Flush the last inputs buffer, if any.
    bufWriterFlush(cliWriter);

    while (serialRxBytesWaiting(cliPort)) {
        uint8_t c = serialRead(cliPort);
        if (c == '\t' || c == '?') {
            // do tab completion
            const clicmd_t *cmd, *pstart = NULL, *pend = NULL;
            uint32_t i = bufferIndex;
            for (cmd = cmdTable; cmd < cmdTable + ARRAYLEN(cmdTable); cmd++) {
                if (bufferIndex && (sl_strncasecmp(cliBuffer, cmd->name, bufferIndex) != 0))
                    continue;
                if (!pstart)
                    pstart = cmd;
                pend = cmd;
            }
            if (pstart) {    /* Buffer matches one or more commands */
                for (; ; bufferIndex++) {
                    if (pstart->name[bufferIndex] != pend->name[bufferIndex])
                        break;
                    if (!pstart->name[bufferIndex] && bufferIndex < sizeof(cliBuffer) - 2) {
                        /* Unambiguous -- append a space */
                        cliBuffer[bufferIndex++] = ' ';
                        cliBuffer[bufferIndex] = '\0';
                        break;
                    }
                    cliBuffer[bufferIndex] = pstart->name[bufferIndex];
                }
            }
            if (!bufferIndex || pstart != pend) {
                /* Print list of ambiguous matches */
                cliPrint("\r\033[K");
                for (cmd = pstart; cmd <= pend; cmd++) {
                    cliPrint(cmd->name);
                    cliWrite('\t');
                }
                cliPrompt();
                i = 0;    /* Redraw prompt */
            }
            for (; i < bufferIndex; i++)
                cliWrite(cliBuffer[i]);
        } else if (!bufferIndex && c == 4) {   // CTRL-D
            cliExit(cliBuffer);
            return;
        } else if (c == 12) {                  // NewPage / CTRL-L
            // clear screen
            cliPrint("\033[2J\033[1;1H");
            cliPrompt();
        } else if (bufferIndex && (c == '\n' || c == '\r')) {
            // enter pressed
            cliPrintLinefeed();

            // Strip comment starting with # from line
            char *p = cliBuffer;
            p = strchr(p, '#');
            if (NULL != p) {
                bufferIndex = (uint32_t)(p - cliBuffer);
            }

            // Strip trailing whitespace
            while (bufferIndex > 0 && cliBuffer[bufferIndex - 1] == ' ') {
                bufferIndex--;
            }

            // Process non-empty lines
            if (bufferIndex > 0) {
                cliBuffer[bufferIndex] = 0; // null terminate

                const clicmd_t *cmd;
                for (cmd = cmdTable; cmd < cmdTable + ARRAYLEN(cmdTable); cmd++) {
                    if (!sl_strncasecmp(cliBuffer, cmd->name, strlen(cmd->name))   // command names match
                       && !sl_isalnum((unsigned)cliBuffer[strlen(cmd->name)]))    // next characted in bufffer is not alphanumeric (command is correctly terminated)
                        break;
                }
                if (cmd < cmdTable + ARRAYLEN(cmdTable))
                    cmd->func(cliBuffer + strlen(cmd->name) + 1);
                else
                    cliPrintError("Unknown command, try 'help'");
                bufferIndex = 0;
            }

            ZERO_FARRAY(cliBuffer);

            // 'exit' will reset this flag, so we don't need to print prompt again
            if (!cliMode)
                return;

            cliPrompt();
        } else if (c == 127) {
            // backspace
            if (bufferIndex) {
                cliBuffer[--bufferIndex] = 0;
                cliPrint("\010 \010");
            }
        } else if (bufferIndex < sizeof(cliBuffer) && c >= 32 && c <= 126) {
            if (!bufferIndex && c == ' ')
                continue; // Ignore leading spaces
            cliBuffer[bufferIndex++] = c;
            cliWrite(c);
        }
    }
}

void cliEnter(serialPort_t *serialPort)
{
    if (cliMode) {
        return;
    }

    cliMode = true;
    cliPort = serialPort;
    setPrintfSerialPort(cliPort);
    cliWriter = bufWriterInit(cliWriteBuffer, sizeof(cliWriteBuffer), (bufWrite_t)serialWriteBufShim, serialPort);

#ifndef CLI_MINIMAL_VERBOSITY
    cliPrintLine("\r\nEntering CLI Mode, type 'exit' to return, or 'help'");
#else
    cliPrintLine("\r\nCLI");
#endif
    cliPrompt();

#ifdef USE_CLI_BATCH
    resetCommandBatch();
#endif

    ENABLE_ARMING_FLAG(ARMING_DISABLED_CLI);
}

void cliInit(const serialConfig_t *serialConfig)
{
    UNUSED(serialConfig);
}

#include "platform.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_SIM)

#include <string.h>

#include "common/axis.h"
#include "common/streambuf.h"
#include "common/utils.h"
#include "common/printf.h"

#include "drivers/time.h"

#include "fc/config.h"
#include "fc/fc_core.h"
#include "fc/rc_controls.h"
#include "fc/runtime_config.h"
#include "fc/fc_init.h"

#include "flight/imu.h"
#include "flight/failsafe.h"
#include "flight/pid.h"

#include "io/gps.h"
#include "io/serial.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "sensors/acceleration.h"
#include "sensors/battery.h"
#include "sensors/boardalignment.h"
#include "sensors/diagnostics.h"
#include "sensors/gyro.h"
#include "sensors/sensors.h"

#include "common/string_light.h"
#include "common/typeconversion.h"
#include "build/debug.h"

#include "telemetry/sim.h"
#include "telemetry/telemetry.h"


static serialPort_t *simPort;
static serialPortConfig_t *portConfig;
static bool simEnabled = false;

static uint8_t atCommand[SIM_AT_COMMAND_MAX_SIZE];
static int simTelemetryState = SIM_STATE_INIT;
static timeMs_t sim_t_stateChange = 0;
static uint8_t simResponse[SIM_RESPONSE_BUFFER_SIZE + 1];
static int atCommandStatus = SIM_AT_OK;
static bool simWaitAfterResponse = false;
static uint8_t readState = SIM_READSTATE_RESPONSE;
static uint8_t transmitFlags = 0;
static timeMs_t  t_lastMessageSent = 0;
uint8_t simModuleState = SIM_MODULE_NOT_DETECTED;

int simRssi;
uint8_t accEvent = ACC_EVENT_NONE;
char* accEventDescriptions[] = { "", "HIT! ", "DROP ", "HIT " };
char* modeDescriptions[] = { "MAN", "ACR", "ANG", "HOR", "ALH", "POS", "RTH", "WP", "LAU", "FS" };
const char gpsFixIndicators[] = { '!', '*', ' ' };


bool isGroundStationNumberDefined() {
    return telemetryConfig()->simGroundStationNumber[0] != '\0';
}

bool checkGroundStationNumber(uint8_t* rv)
{
    int i;
    const uint8_t* gsn = telemetryConfig()->simGroundStationNumber;

    for (i = 0; i < 16 && *gsn != '\0'; i++) gsn++;
    if (i == 0)
        return false;
    for (i = 0; i < 16 && *rv != '\"'; i++) rv++;
    gsn--; rv--;
    for (i = 0; i < SIM_GROUND_STATION_NUMBER_DIGITS; i++) {
        if (*rv != *gsn) return false;
        gsn--; rv--;
    }
    return true;
}


void readOriginatingNumber(uint8_t* rv)
{
    int i;
    uint8_t* gsn = telemetryConfigMutable()->simGroundStationNumber;
    if (gsn[0] != '\0')
        return;
    for (i = 0; i < 15 && rv[i] != '\"'; i++)
         gsn[i] = rv[i];
    gsn[i] = '\0';
}

void readTransmitFlags(const uint8_t* fs)
{
    int i;

    transmitFlags = 0;
    for (i = 0; i < 4 && fs[i] != '\0'; i++) {
        switch (fs[i]) {
            case 't':
            transmitFlags |= SIM_TX_FLAG;
            break;
            case 'f':
            transmitFlags |= SIM_TX_FLAG_FAILSAFE;
            break;
            case 'g':
            transmitFlags |= SIM_TX_FLAG_GPS;
            break;
        }
    }
}

void requestSendSMS()
{
    if (simTelemetryState == SIM_STATE_SEND_SMS_ENTER_MESSAGE)
        return; // sending right now, don't reissue AT command
    simTelemetryState = SIM_STATE_SEND_SMS;
    if (atCommandStatus != SIM_AT_WAITING_FOR_RESPONSE)
        sim_t_stateChange = 0; // send immediately
}

void readSMS()
{
    if (sl_strcasecmp((char*)simResponse, SIM_SMS_COMMAND_RTH) == 0) {
        if (!posControl.flags.forcedRTHActivated) {
            activateForcedRTH();
        } else {
            abortForcedRTH();
        }
    } else {
        readTransmitFlags(simResponse);
    }
    requestSendSMS();
}

void readSimResponse()
{
    if (readState == SIM_READSTATE_SKIP) {
        readState = SIM_READSTATE_RESPONSE;
        return;
    } else if (readState == SIM_READSTATE_SMS) {
        readSMS();
        readState = SIM_READSTATE_RESPONSE;
        return;
    }

    uint8_t* resp = simResponse;
    uint32_t responseCode = 0;
    if (simResponse[0] == '+') {
        resp++;
    }
    responseCode = *resp++;
    responseCode <<= 8; responseCode |= *resp++;
    responseCode <<= 8; responseCode |= *resp++;
    responseCode <<= 8; responseCode |= *resp++;

    if (responseCode == SIM_RESPONSE_CODE_OK) {
        // OK
        atCommandStatus = SIM_AT_OK;
        if (!simWaitAfterResponse) {
            sim_t_stateChange = millis() + SIM_AT_COMMAND_DELAY_MIN_MS;
        }
        return;
    } else if (responseCode == SIM_RESPONSE_CODE_ERROR) {
        // ERROR
        atCommandStatus = SIM_AT_ERROR;
        if (!simWaitAfterResponse) {
            sim_t_stateChange = millis() + SIM_AT_COMMAND_DELAY_MIN_MS;
        }
        return;
    } else if (responseCode == SIM_RESPONSE_CODE_RING) {
        // RING
    } else if (responseCode == SIM_RESPONSE_CODE_CSQ) {
        // +CSQ: 26,0
        simRssi = fastA2I((char*)&simResponse[6]);
    } else if (responseCode == SIM_RESPONSE_CODE_CLIP) {
        // we always get this after a RING when a call is incoming
        // +CLIP: "+3581234567"
        readOriginatingNumber(&simResponse[8]);
        if (checkGroundStationNumber(&simResponse[8])) {
            requestSendSMS();
        }
    } else if (responseCode == SIM_RESPONSE_CODE_CREG) {
        // +CREG: 0,1
        if (simResponse[9] == '1' || simResponse[9] == '5') {
            simModuleState = SIM_MODULE_REGISTERED;
        } else {
            simModuleState = SIM_MODULE_NOT_REGISTERED;
        }        
    } else if (responseCode == SIM_RESPONSE_CODE_CMT) {
        // +CMT: <oa>,[<alpha>],<scts>[,<tooa>,<fo>,<pid>,<dcs>,<sca>,<tosca>,<length>]<CR><LF><data>
        // +CMT: "+3581234567","","19/02/12,14:57:24+08"
        readOriginatingNumber(&simResponse[7]);
        if (checkGroundStationNumber(&simResponse[7])) {
            readState = SIM_READSTATE_SMS; // next simResponse line will be SMS content
        } else {
            readState = SIM_READSTATE_SKIP; // skip SMS content
        }
    }
}

void detectAccEvents()
{
    timeMs_t now = millis();
//    float acceleration = sqrtf(vectorNormSquared(acc.accADCf));
    uint32_t accSq = sq(imuMeasuredAccelBF.x) + sq(imuMeasuredAccelBF.y) + sq(imuMeasuredAccelBF.z);

    if (telemetryConfig()->accEventThresholdHigh > 0 && accSq > sq(telemetryConfig()->accEventThresholdHigh))
        accEvent = ACC_EVENT_HIGH;
    else if (accSq < sq(telemetryConfig()->accEventThresholdLow))
        accEvent = ACC_EVENT_LOW;
    else if (telemetryConfig()->accEventThresholdNegX > 0 && imuMeasuredAccelBF.x < -telemetryConfig()->accEventThresholdNegX)
        accEvent = ACC_EVENT_NEG_X;
    else
        return;

    if (now - t_lastMessageSent > 1000 * SIM_MIN_TRANSMIT_INTERVAL) {
        requestSendSMS();
    }
}

void transmit()
{
    timeMs_t now = millis();

    uint8_t transmitConditions = SIM_TX_FLAG;

    if (now - t_lastMessageSent < 1000 * MAX(SIM_MIN_TRANSMIT_INTERVAL, telemetryConfig()->simTransmitInterval))
        return;

    if (FLIGHT_MODE(FAILSAFE_MODE))
        transmitConditions |= SIM_TX_FLAG_FAILSAFE;
    if (gpsSol.fixType != GPS_NO_FIX && gpsSol.numSat < gpsConfig()->gpsMinSats)
        transmitConditions |= SIM_TX_FLAG_GPS;

    if (ARMING_FLAG(WAS_EVER_ARMED) && (transmitConditions & transmitFlags)) {
        requestSendSMS();
    }
}

void sendATCommand(const char* command)
{
    atCommandStatus = SIM_AT_WAITING_FOR_RESPONSE;
    int len = strlen((char*)command);
    if (len >SIM_AT_COMMAND_MAX_SIZE)
        len = SIM_AT_COMMAND_MAX_SIZE;
    serialWriteBuf(simPort, (const uint8_t*) command, len);
}

void sendSMS(void)
{
    int32_t lat = 0, lon = 0, alt = 0, gs = 0;
    int vbat = getBatteryVoltage();
    int16_t amps = isAmperageConfigured() ? getAmperage() / 10 : 0; // 1 = 100 milliamps
    int avgSpeed = (int)round(10 * calculateAverageSpeed());
    uint32_t now = millis();

    if (sensors(SENSOR_GPS)) {
        lat = gpsSol.llh.lat;
        lon = gpsSol.llh.lon;
        gs = gpsSol.groundSpeed / 100;
    }
#if defined(USE_NAV)
    alt = getEstimatedActualPosition(Z); // cm
#else
    alt = sensors(SENSOR_GPS) ? gpsSol.llh.alt : 0; // cm
#endif
    int len;
    int32_t E7 = 10000000;
    // \x1a sends msg, \x1b cancels
    len = tfp_sprintf((char*)atCommand, "%s%d.%02dV %d.%dA ALT:%ld SPD:%ld/%d.%d DIS:%d/%d SAT:%d%c SIG:%d %s maps.google.com/?q=@%ld.%07ld,%ld.%07ld\x1a",
        accEventDescriptions[accEvent],
        vbat / 100, vbat % 100,
        amps / 10, amps % 10,
        alt / 100,
        gs, avgSpeed / 10, avgSpeed % 10,
        GPS_distanceToHome, getTotalTravelDistance() / 100,
        gpsSol.numSat, gpsFixIndicators[gpsSol.fixType],
        simRssi,
        posControl.flags.forcedRTHActivated ? "RTH" : modeDescriptions[getFlightModeForTelemetry()],
        lat / E7, lat % E7, lon / E7, lon % E7);
    serialWriteBuf(simPort, atCommand, len);
    t_lastMessageSent = now;
    accEvent = ACC_EVENT_NONE;
    atCommandStatus = SIM_AT_WAITING_FOR_RESPONSE;
}

void handleSimTelemetry()
{
    static uint16_t simResponseIndex = 0;
    uint32_t now = millis();

    if (!simEnabled)
        return;
    if (!simPort)
        return;

    while (serialRxBytesWaiting(simPort) > 0) {
        uint8_t c = serialRead(simPort);
        if (c == '\n' || simResponseIndex == SIM_RESPONSE_BUFFER_SIZE) {
            simResponse[simResponseIndex] = '\0';
            if (simResponseIndex > 0) simResponseIndex--;
            if (simResponse[simResponseIndex] == '\r') simResponse[simResponseIndex] = '\0';
            simResponseIndex = 0; //data ok
            readSimResponse();
            break;
        } else {
            simResponse[simResponseIndex] = c;
            simResponseIndex++;
        }
    }

    detectAccEvents();
    transmit();

    if (now < sim_t_stateChange)
        return;

    sim_t_stateChange = now + SIM_AT_COMMAND_DELAY_MS;       // by default, if OK or ERROR not received, wait this long
    simWaitAfterResponse = false;   // by default, if OK or ERROR received, go to next state immediately.
    switch (simTelemetryState) {
        case SIM_STATE_INIT:
        sendATCommand("AT\r");
        simTelemetryState = SIM_STATE_INIT2;
        break;
        case SIM_STATE_INIT2:
        sendATCommand("ATE0\r");
        simTelemetryState = SIM_STATE_INIT_ENTER_PIN;
        break;
        case SIM_STATE_INIT_ENTER_PIN:
        sendATCommand("AT+CPIN=" SIM_PIN "\r");
        simTelemetryState = SIM_STATE_SET_MODES;
        break;
        case SIM_STATE_SET_MODES:
        sendATCommand("AT+CMGF=1;+CNMI=3,2;+CLIP=1;+CSQ\r");
        simTelemetryState = SIM_STATE_INIT;
        sim_t_stateChange = now + SIM_CYCLE_MS;
        simWaitAfterResponse = true;
        break;
        case SIM_STATE_SEND_SMS:
        sendATCommand("AT+CMGS=\"");
        sendATCommand((char*)telemetryConfig()->simGroundStationNumber);
        sendATCommand("\"\r");
        simTelemetryState = SIM_STATE_SEND_SMS_ENTER_MESSAGE;
        sim_t_stateChange = now + 100;
        break;
        case SIM_STATE_SEND_SMS_ENTER_MESSAGE:
        sendSMS();
        simTelemetryState = SIM_STATE_INIT;
        sim_t_stateChange = now + SIM_CYCLE_MS;
        simWaitAfterResponse = true;
        break;
    }
}

void freeSimTelemetryPort(void)
{
    closeSerialPort(simPort);
    simPort = NULL;
    simEnabled = false;
}

void initSimTelemetry(void)
{
    portConfig = findSerialPortConfig(FUNCTION_TELEMETRY_SIM);
}

void checkSimTelemetryState(void)
{
    if (simEnabled) {
        return;
    }
    configureSimTelemetryPort();
}

void configureSimTelemetryPort(void)
{
    if (!portConfig) {
        return;
    }
    baudRate_e baudRateIndex = portConfig->telemetry_baudrateIndex;
    simPort = openSerialPort(portConfig->identifier, FUNCTION_TELEMETRY_SIM, NULL, NULL,
        baudRates[baudRateIndex], MODE_RXTX, SERIAL_NOT_INVERTED);

    if (!simPort) {
        return;
    }

    sim_t_stateChange = millis() + SIM_STARTUP_DELAY_MS;
    simTelemetryState = SIM_STATE_INIT;
    readState = SIM_READSTATE_RESPONSE;
    readTransmitFlags(telemetryConfig()->simTransmitFlags);
    simEnabled = true;
}

#endif

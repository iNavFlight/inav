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
 *
 * FlySky iBus telemetry implementation by CraigJPerry.
 *
 * Many thanks to Dave Borthwick's iBus telemetry dongle converter for
 * PIC 12F1572 (also distributed under GPLv3) which was referenced to
 * clarify the protocol.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_IBUS)

#include "common/maths.h"
#include "common/axis.h"

#include "drivers/serial.h"
#include "drivers/time.h"

#include "fc/fc_core.h"
#include "fc/rc_controls.h"
#include "fc/runtime_config.h"
#include "scheduler/scheduler.h"

#include "io/serial.h"

#include "sensors/barometer.h"
#include "sensors/acceleration.h"
#include "sensors/battery.h"
#include "sensors/sensors.h"
#include "io/gps.h"

#include "flight/imu.h"
#include "flight/failsafe.h"

#include "navigation/navigation.h"

#include "telemetry/ibus.h"
#include "telemetry/ibus_shared.h"
#include "telemetry/telemetry.h"
#include "fc/config.h"
#include "config/feature.h"

/*
 * iBus Telemetry is a half-duplex serial protocol. It shares 1 line for
 * both TX and RX. It runs at a fixed baud rate of 115200. Queries are sent
 * every 7ms by the iBus receiver. Multiple sensors can be daisy chained with
 * ibus but not with this implementation, only because i don't have one of the
 * sensors to test with!
 *
 *     _______
 *    /       \                                             /---------\
 *    | STM32 |--UART TX-->[Bi-directional @ 115200 baud]<--| IBUS RX |
 *    |  uC   |--UART RX--x[not connected]                  \---------/
 *    \_______/
 *
 *
 * The protocol is driven by the iBus receiver, currently either an IA6B or
 * IA10. All iBus traffic is little endian. It begins with the iBus rx
 * querying for a sensor on the iBus:
 *
 *
 *  /---------\
 *  | IBUS RX | > Hello sensor at address 1, are you there?
 *  \---------/     [ 0x04, 0x81, 0x7A, 0xFF ]
 *
 *     0x04       - Packet Length
 *     0x81       - bits 7-4 Command (1000 = discover sensor)
 *                  bits 3-0 Address (0001 = address 1)
 *     0x7A, 0xFF - Checksum, 0xFFFF - (0x04 + 0x81)
 *
 *
 * Due to the daisy-chaining, this hello also serves to inform the sensor
 * of its address (position in the chain). There are 16 possible addresses
 * in iBus, however address 0 is reserved for the rx's internally measured
 * voltage leaving 0x1 to 0xF remaining.
 *
 * Having learned it's address, the sensor simply echos the message back:
 *
 *
 *                                                      /--------\
 *                              Yes, i'm here, hello! < | Sensor |
 *                       [ 0x04, 0x81, 0x7A, 0xFF ]     \--------/
 *
 *     0x04, 0x81, 0x7A, 0xFF - Echo back received packet
 *
 *
 * On receipt of a response, the iBus rx next requests the sensor's type:
 *
 *
 *  /---------\
 *  | IBUS RX | > Sensor at address 1, what type are you?
 *  \---------/     [ 0x04, 0x91, 0x6A, 0xFF ]
 *
 *     0x04       - Packet Length
 *     0x91       - bits 7-4 Command (1001 = request sensor type)
 *                  bits 3-0 Address (0001 = address 1)
 *     0x6A, 0xFF - Checksum, 0xFFFF - (0x04 + 0x91)
 *
 *
 * To which the sensor responds with its details:
 *
 *
 *                                                      /--------\
 *                              Yes, i'm here, hello! < | Sensor |
 *                [ 0x06 0x91 0x00 0x02 0x66 0xFF ]     \--------/
 *
 *     0x06       - Packet Length
 *     0x91       - bits 7-4 Command (1001 = request sensor type)
 *                  bits 3-0 Address (0001 = address 1)
 *     0x00       - Measurement type (0 = internal voltage)
 *     0x02       - Unknown, always 0x02
 *     0x66, 0xFF - Checksum, 0xFFFF - (0x06 + 0x91 + 0x00 + 0x02)
 *
 *
 * The iBus rx continues the discovery process by querying the next
 * address. Discovery stops at the first address which does not respond.
 *
 * The iBus rx then begins a continual loop, requesting measurements from
 * each sensor discovered:
 *
 *
 *  /---------\
 *  | IBUS RX | > Sensor at address 1, please send your measurement
 *  \---------/     [ 0x04, 0xA1, 0x5A, 0xFF ]
 *
 *     0x04       - Packet Length
 *     0xA1       - bits 7-4 Command (1010 = request measurement)
 *                  bits 3-0 Address (0001 = address 1)
 *     0x5A, 0xFF - Checksum, 0xFFFF - (0x04 + 0xA1)
 *
 *
 *                                                      /--------\
 *                                I'm reading 0 volts < | Sensor |
 *                [ 0x06 0xA1 0x00 0x00 0x5E 0xFF ]     \--------/
 *
 *     0x06       - Packet Length
 *     0xA1       - bits 7-4 Command (1010 = request measurement)
 *                  bits 3-0 Address (0001 = address 1)
 *     0x00, 0x00 - The measurement
 *     0x5E, 0xFF - Checksum, 0xFF - (0x06 + 0xA1 + 0x00 + 0x00)
 *
 *
 * Due to the limited telemetry data types possible with ibus, we
 * simply send everything which can be represented. Currently this
 * is voltage and temperature.
 *
 */

static serialPort_t *ibusSerialPort = NULL;
static serialPortConfig_t *ibusSerialPortConfig;
static uint8_t outboundBytesToIgnoreOnRxCount = 0;
static bool ibusTelemetryEnabled = false;
static portSharing_e ibusPortSharing;

static uint8_t ibusReceiveBuffer[IBUS_RX_BUF_LEN] = { 0x0 };

static void pushOntoTail(uint8_t buffer[IBUS_MIN_LEN], size_t bufferLength, uint8_t value) {
    for (size_t i = 0; i < bufferLength - 1; i++) {
        buffer[i] = buffer[i + 1];
    }
    ibusReceiveBuffer[bufferLength - 1] = value;
}

void initIbusTelemetry(void) {
    ibusSerialPortConfig = findSerialPortConfig(FUNCTION_TELEMETRY_IBUS);
    uint8_t type = telemetryConfig()->ibusTelemetryType;
    uint8_t speed = type & 0x80;
    type = type & 0x7F;
    if (type == 3) {
        changeTypeIbusTelemetry(3, IBUS_MEAS_TYPE_S85, IBUS_MEAS_VALUE_STATUS);
        changeTypeIbusTelemetry(4, IBUS_MEAS_TYPE_ACC_Z, IBUS_MEAS_VALUE_ACC_Z);
        changeTypeIbusTelemetry(5, IBUS_MEAS_TYPE_CURRENT, IBUS_MEAS_VALUE_CURRENT);
        changeTypeIbusTelemetry(6, IBUS_MEAS_TYPE_ALT, IBUS_MEAS_VALUE_ALT);
        changeTypeIbusTelemetry(7, IBUS_MEAS_TYPE_HEADING, IBUS_MEAS_VALUE_HEADING);
        changeTypeIbusTelemetry(8, IBUS_MEAS_TYPE_DIST, IBUS_MEAS_VALUE_DIST);
        changeTypeIbusTelemetry(9, IBUS_MEAS_TYPE_COG, IBUS_MEAS_VALUE_COG);
        changeTypeIbusTelemetry(11,IBUS_MEAS_TYPE_GPS_LON, IBUS_MEAS_VALUE_GPS_LON2);
        changeTypeIbusTelemetry(12,IBUS_MEAS_TYPE_GPS_LAT, IBUS_MEAS_VALUE_GPS_LAT2);
        changeTypeIbusTelemetry(13,IBUS_MEAS_TYPE_ACC_X, IBUS_MEAS_VALUE_GPS_LON1);
        changeTypeIbusTelemetry(14,IBUS_MEAS_TYPE_ACC_Y, IBUS_MEAS_VALUE_GPS_LAT1);
    }
    if (type == 4) {
        changeTypeIbusTelemetry(3, IBUS_MEAS_TYPE_S85, IBUS_MEAS_VALUE_STATUS);
#ifdef USE_PITOT
        if (sensors(SENSOR_PITOT)) changeTypeIbusTelemetry(11,IBUS_MEAS_TYPE_VSPEED, IBUS_MEAS_VALUE_VSPEED);
        else 
#endif
            changeTypeIbusTelemetry(11,IBUS_MEAS_TYPE_PRES, IBUS_MEAS_VALUE_PRES);
    }
    if (type == 5) {
        changeTypeIbusTelemetry(2, IBUS_MEAS_TYPE_ARMED, IBUS_MEAS_VALUE_ARMED);
        changeTypeIbusTelemetry(3, IBUS_MEAS_TYPE_MODE, IBUS_MEAS_VALUE_MODE);
        changeTypeIbusTelemetry(11,IBUS_MEAS_TYPE_CLIMB, IBUS_MEAS_VALUE_CLIMB);
    }
    if (type == 4 || type == 5) {
        changeTypeIbusTelemetry(4, IBUS_MEAS_TYPE_ACC_Z, IBUS_MEAS_VALUE_ACC_Z);
        changeTypeIbusTelemetry(5, IBUS_MEAS_TYPE_CURRENT, IBUS_MEAS_VALUE_CURRENT);
        changeTypeIbusTelemetry(6, IBUS_MEAS_TYPE_S8A, IBUS_MEAS_VALUE_ALT4);
        changeTypeIbusTelemetry(7, IBUS_MEAS_TYPE_HEADING, IBUS_MEAS_VALUE_HEADING);
        changeTypeIbusTelemetry(8, IBUS_MEAS_TYPE_DIST, IBUS_MEAS_VALUE_DIST);
        changeTypeIbusTelemetry(9, IBUS_MEAS_TYPE_COG, IBUS_MEAS_VALUE_COG);
        changeTypeIbusTelemetry(12,IBUS_MEAS_TYPE_GPS_STATUS, IBUS_MEAS_VALUE_GPS_STATUS);
        changeTypeIbusTelemetry(13,IBUS_MEAS_TYPE_S88, IBUS_MEAS_VALUE_GPS_LON);
        changeTypeIbusTelemetry(14,IBUS_MEAS_TYPE_S89, IBUS_MEAS_VALUE_GPS_LAT);
    }
    if (type == 2 || type == 3 || type == 4 || type == 5) 
        changeTypeIbusTelemetry(10, IBUS_MEAS_TYPE_GALT, IBUS_MEAS_VALUE_GALT);
    if (type == 1 || type == 2 || type == 3 || type == 4 || type == 5) 
        changeTypeIbusTelemetry(15, IBUS_MEAS_TYPE_SPE, IBUS_MEAS_VALUE_SPE);
    if ((type == 3 || type == 4 || type == 5) && speed)
        changeTypeIbusTelemetry(15,IBUS_MEAS_TYPE_SPEED, IBUS_MEAS_VALUE_SPEED);
    if (type == 6) {
#ifdef USE_PITOT
        if (sensors(SENSOR_PITOT)) changeTypeIbusTelemetry(9,IBUS_MEAS_TYPE1_VERTICAL_SPEED, IBUS_MEAS_VALUE_VSPEED);
        else 
#endif
            changeTypeIbusTelemetry(9, IBUS_MEAS_TYPE1_PRES, IBUS_MEAS_VALUE_PRES);
        changeTypeIbusTelemetry(15, IBUS_MEAS_TYPE1_S85, IBUS_MEAS_VALUE_STATUS);
    }
    if (type == 7 || type == 8) {
        changeTypeIbusTelemetry(2, IBUS_MEAS_TYPE1_GPS_STATUS, IBUS_MEAS_VALUE_GPS_STATUS);
        changeTypeIbusTelemetry(9, IBUS_MEAS_TYPE1_ARMED, IBUS_MEAS_VALUE_ARMED);
        changeTypeIbusTelemetry(15,IBUS_MEAS_TYPE1_FLIGHT_MODE, IBUS_MEAS_VALUE_MODE);
    }
    if (type == 6 || type == 7 || type == 8) {
        if (batteryConfig()->current.type == CURRENT_SENSOR_VIRTUAL)
            changeTypeIbusTelemetry(3, IBUS_MEAS_TYPE1_FUEL, IBUS_MEAS_VALUE_FUEL);
        else changeTypeIbusTelemetry(3, IBUS_MEAS_TYPE1_BAT_CURR, IBUS_MEAS_VALUE_CURRENT);
        changeTypeIbusTelemetry(4, IBUS_MEAS_TYPE1_CMP_HEAD, IBUS_MEAS_VALUE_HEADING);
        changeTypeIbusTelemetry(6, IBUS_MEAS_TYPE1_CLIMB_RATE, IBUS_MEAS_VALUE_CLIMB);
        changeTypeIbusTelemetry(5, IBUS_MEAS_TYPE1_COG, IBUS_MEAS_VALUE_COG);
        changeTypeIbusTelemetry(7, IBUS_MEAS_TYPE1_YAW, IBUS_MEAS_VALUE_ACC_Z);
        changeTypeIbusTelemetry(8, IBUS_MEAS_TYPE1_GPS_DIST, IBUS_MEAS_VALUE_DIST);
        if (speed) changeTypeIbusTelemetry(10,IBUS_MEAS_TYPE1_GROUND_SPEED, IBUS_MEAS_VALUE_SPEED);
        else changeTypeIbusTelemetry(10, IBUS_MEAS_TYPE1_SPE, IBUS_MEAS_VALUE_SPE);
        changeTypeIbusTelemetry(11,IBUS_MEAS_TYPE1_GPS_LAT, IBUS_MEAS_VALUE_GPS_LAT);
        changeTypeIbusTelemetry(12,IBUS_MEAS_TYPE1_GPS_LON, IBUS_MEAS_VALUE_GPS_LON);
        changeTypeIbusTelemetry(13,IBUS_MEAS_TYPE1_GPS_ALT, IBUS_MEAS_VALUE_GALT4);
        changeTypeIbusTelemetry(14,IBUS_MEAS_TYPE1_ALT, IBUS_MEAS_VALUE_ALT4);
    }
    ibusPortSharing = determinePortSharing(ibusSerialPortConfig, FUNCTION_TELEMETRY_IBUS);
    ibusTelemetryEnabled = false;
}

void handleIbusTelemetry(void) {
    if (!ibusTelemetryEnabled) {
        return;
    }
    while (serialRxBytesWaiting(ibusSerialPort) > 0) {
        uint8_t c = serialRead(ibusSerialPort);
        if (outboundBytesToIgnoreOnRxCount) {
            outboundBytesToIgnoreOnRxCount--;
            continue;
        }
        pushOntoTail(ibusReceiveBuffer, IBUS_RX_BUF_LEN, c);
        if (ibusIsChecksumOkIa6b(ibusReceiveBuffer, IBUS_RX_BUF_LEN)) {
            outboundBytesToIgnoreOnRxCount += respondToIbusRequest(ibusReceiveBuffer);
        }
    }
}

bool checkIbusTelemetryState(void) {
    bool newTelemetryEnabledValue = telemetryDetermineEnabledState(ibusPortSharing);

    if (newTelemetryEnabledValue == ibusTelemetryEnabled) {
        return false;
    }

    if (newTelemetryEnabledValue) {
        rescheduleTask(TASK_TELEMETRY, IBUS_TASK_PERIOD_US);
        configureIbusTelemetryPort();
    } else {
        freeIbusTelemetryPort();
    }

    return true;
}

void configureIbusTelemetryPort(void) {
    if (!ibusSerialPortConfig) {
        return;
    }
    if (isSerialPortShared(ibusSerialPortConfig, FUNCTION_RX_SERIAL, FUNCTION_TELEMETRY_IBUS)) {
        // serialRx will open port and handle telemetry
        return;
    }
    ibusSerialPort = openSerialPort(ibusSerialPortConfig->identifier, FUNCTION_TELEMETRY_IBUS, NULL, NULL, IBUS_BAUDRATE, MODE_RXTX, SERIAL_BIDIR | SERIAL_NOT_INVERTED);
    if (!ibusSerialPort) {
        return;
    }
    initSharedIbusTelemetry(ibusSerialPort);
    ibusTelemetryEnabled = true;
    outboundBytesToIgnoreOnRxCount = 0;
}

void freeIbusTelemetryPort(void) {
    closeSerialPort(ibusSerialPort);
    ibusSerialPort = NULL;

    ibusTelemetryEnabled = false;
}

#endif

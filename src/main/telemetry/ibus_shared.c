#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

#if defined(TELEMETRY) && defined(TELEMETRY_IBUS)

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
#include "telemetry/telemetry.h"
#include "fc/config.h"
#include "config/feature.h"

#define IBUS_TEMPERATURE_OFFSET (0x0190)

typedef uint8_t ibusAddress_t;

typedef enum {
    IBUS_COMMAND_DISCOVER_SENSOR      = 0x80,
    IBUS_COMMAND_SENSOR_TYPE          = 0x90,
    IBUS_COMMAND_MEASUREMENT          = 0xA0
} ibusCommand_e;

typedef enum {
    IBUS_MEAS_TYPE_INTERNAL_VOLTAGE = 0x00, // Internal Voltage
    IBUS_MEAS_TYPE_TEMPERATURE      = 0x01, // Temperature -##0.0 C, 0=-40.0 C, 400=0.0 C, 65535=6513.5 C
    IBUS_MEAS_TYPE_RPM              = 0x02, // Rotation RPM, ####0RPM, 0=0RPM, 65535=65535RPM
    IBUS_MEAS_TYPE_EXTERNAL_VOLTAGE = 0x03, // External Voltage, -##0.00V, 0=0.00V, 32767=327.67V, 32768=na, 32769=-327.67V, 65535=-0.01V
    IBUS_MEAS_TYPE_PRES             = 0x41, // Pressure, not work
    IBUS_MEAS_TYPE_ODO1             = 0x7c, // Odometer1, 0.0km, 0.0 only
    IBUS_MEAS_TYPE_ODO2             = 0x7d, // Odometer2, 0.0km, 0.0 only
    IBUS_MEAS_TYPE_SPE              = 0x7e, // Speed km/h, ###0km/h, 0=0Km/h, 1000=100Km/h
    IBUS_MEAS_TYPE_ALT              = 0xf9, // Altitude m, not work
    IBUS_MEAS_TYPE_SNR              = 0xfa, // SNR, not work
    IBUS_MEAS_TYPE_NOISE            = 0xfb, // Noise, not work
    IBUS_MEAS_TYPE_RSSI             = 0xfc, // RSSI, not work
    IBUS_MEAS_TYPE_ERR              = 0xfe  // Error rate, #0%
} ibusSensorType_e;

static uint8_t SENSOR_ADDRESS_TYPE_LOOKUP[] = {
    IBUS_MEAS_TYPE_INTERNAL_VOLTAGE,  // Address 0, sensor 1, not usable since it is reserved for internal voltage
    IBUS_MEAS_TYPE_EXTERNAL_VOLTAGE,  // Address 1 ,sensor 2, VBAT
    IBUS_MEAS_TYPE_TEMPERATURE,       // Address 2, sensor 3, Baro/Gyro Temp	
    IBUS_MEAS_TYPE_RPM,               // Address 3, sensor 4, Status AS RPM
    IBUS_MEAS_TYPE_RPM,               // Address 4, sensor 5, MAG_COURSE in deg AS RPM
    IBUS_MEAS_TYPE_EXTERNAL_VOLTAGE,  // Address 5, sensor 6, Current in A AS ExtV
    IBUS_MEAS_TYPE_EXTERNAL_VOLTAGE   // Address 6, sensor 7, Baro Alt in cm AS ExtV
#if defined(GPS)
   ,IBUS_MEAS_TYPE_RPM,               // Address 7, sensor 8, HOME_DIR in deg AS RPM	
    IBUS_MEAS_TYPE_RPM,               // Address 8, sensor 9, HOME_DIST in m AS RPM
    IBUS_MEAS_TYPE_RPM,               // Address 9, sensor 10,GPS_COURSE in deg AS RPM
    IBUS_MEAS_TYPE_RPM,               // Address 10,sensor 11,GPS_ALT in m AS RPM
    IBUS_MEAS_TYPE_RPM,               // Address 11,sensor 12,GPS_LAT2 AS RPM 5678
    IBUS_MEAS_TYPE_RPM,               // Address 12,sensor 13,GPS_LON2 AS RPM 6789
    IBUS_MEAS_TYPE_EXTERNAL_VOLTAGE,  // Address 13,sensor 14,GPS_LAT1 AS ExtV -12.45 (-12.3456789 N)
    IBUS_MEAS_TYPE_EXTERNAL_VOLTAGE,  // Address 14,sensor 15,GPS_LON1 AS ExtV -123.45 (-123.4567890 E)
    IBUS_MEAS_TYPE_RPM                // Address 15,sensor 16,GPS_SPEED in km/h AS RPM
#endif
    //IBUS_MEAS_TYPE_TX_VOLTAGE,
    //IBUS_MEAS_TYPE_ERR
};

static serialPort_t *ibusSerialPort = NULL;

static uint8_t transmitIbusPacket(uint8_t ibusPacket[static IBUS_MIN_LEN], size_t packetLength) {
    uint16_t checksum = calculateChecksum(ibusPacket, packetLength);
    ibusPacket[packetLength - IBUS_CHECKSUM_SIZE] = (checksum & 0xFF);
    ibusPacket[packetLength - IBUS_CHECKSUM_SIZE + 1] = (checksum >> 8);
    for (size_t i = 0; i < packetLength; i++) {
        serialWrite(ibusSerialPort, ibusPacket[i]);
    }
	return packetLength;
}

static uint8_t sendIbusCommand(ibusAddress_t address) {
    uint8_t sendBuffer[] = { 0x04, IBUS_COMMAND_DISCOVER_SENSOR | address, 0x00, 0x00 };
    return transmitIbusPacket(sendBuffer, sizeof sendBuffer);
}

static uint8_t sendIbusSensorType(ibusAddress_t address) {
    uint8_t sendBuffer[] = { 0x06, IBUS_COMMAND_SENSOR_TYPE | address, SENSOR_ADDRESS_TYPE_LOOKUP[address], 0x02, 0x0, 0x0 };
    return transmitIbusPacket(sendBuffer, sizeof sendBuffer);
}

static uint8_t sendIbusMeasurement(ibusAddress_t address, uint16_t measurement) {
    uint8_t sendBuffer[] = { 0x06, IBUS_COMMAND_MEASUREMENT | address, measurement & 0xFF, measurement >> 8, 0x0, 0x0 };
    return transmitIbusPacket(sendBuffer, sizeof sendBuffer);
}

static bool isCommand(ibusCommand_e expected, uint8_t ibusPacket[static IBUS_MIN_LEN]) {
    return (ibusPacket[1] & 0xF0) == expected;
}

static ibusAddress_t getAddress(uint8_t ibusPacket[static IBUS_MIN_LEN]) {
    return (ibusPacket[1] & 0x0F);
}

static uint8_t dispatchMeasurementRequest(ibusAddress_t address) {
    if (address == 1) { //2. VBAT
        return sendIbusMeasurement(address, vbat * 10);
    } else if (address == 2) { //3. BARO_TEMP\GYRO_TEMP
        if (sensors(SENSOR_BARO)) return sendIbusMeasurement(address, (uint16_t) ((baro.baroTemperature + 50) / 10  + IBUS_TEMPERATURE_OFFSET)); //int32_t 
        else return sendIbusMeasurement(address, (uint16_t) (telemTemperature1 + IBUS_TEMPERATURE_OFFSET)); //int16_t
    } else if (address == 3) { //4. STATUS (sat num AS #0, FIX AS 0, HDOP AS 0, Mode AS 0)
        int16_t status = 0;
        if (ARMING_FLAG(ARMED)) status = 1; //Rate
        if (FLIGHT_MODE(HORIZON_MODE)) status = 2;
        if (FLIGHT_MODE(ANGLE_MODE)) status = 3;
        if (FLIGHT_MODE(HEADFREE_MODE) || FLIGHT_MODE(MAG_MODE)) status = 4; 
	if (FLIGHT_MODE(NAV_ALTHOLD_MODE)) status = 5; 
        if (FLIGHT_MODE(NAV_POSHOLD_MODE)) status = 6;
        if (FLIGHT_MODE(NAV_RTH_MODE)) status = 7;
        if (failsafeIsActive()) {
        if (FLIGHT_MODE(NAV_RTH_MODE)) status = 8; else status = 9; 
    }
#if defined(GPS)
    if (sensors(SENSOR_GPS)) {
        status += gpsSol.numSat * 1000;
        if (gpsSol.fixType == GPS_NO_FIX) status += 100;
        else if (gpsSol.fixType == GPS_FIX_2D) status += 200;
        else if (gpsSol.fixType == GPS_FIX_3D) status += 300;
        if (STATE(GPS_FIX_HOME)) status += 500;
            status += constrain(gpsSol.hdop / 1000, 0, 9) * 10;
        }
#endif
        return sendIbusMeasurement(address, (uint16_t) status);
    } else if (address == 4) { //5. MAG_COURSE (0-360*, 0=north) //In ddeg ==> deg, 10ddeg = 1deg
        return sendIbusMeasurement(address, (uint16_t) (attitude.values.yaw / 10));
    } else if (address == 5) { //6. CURR //In 10*mA, 1 = 10 mA
        if (feature(FEATURE_CURRENT_METER)) return sendIbusMeasurement(address, (uint16_t) amperage); //int32_t
        else return sendIbusMeasurement(address, 0); 
    } else if (address == 6) { //7. BARO_ALT //In cm => m
        if (sensors(SENSOR_BARO)) return sendIbusMeasurement(address, (uint16_t) baro.BaroAlt); //int32_t 
        else return sendIbusMeasurement(address, 0);
    }
#if defined(GPS)
      else if (address == 7) { //8. HOME_DIR (0-360deg, 0=head)
        if (sensors(SENSOR_GPS)) return sendIbusMeasurement(address, (uint16_t) GPS_directionToHome); //int16_t
        else return sendIbusMeasurement(address, 0);
    } else if (address == 8) { //9. HOME_DIST //In m
        if (sensors(SENSOR_GPS)) return sendIbusMeasurement(address, (uint16_t) GPS_distanceToHome); //uint16_t
        else return sendIbusMeasurement(address, 0);
    } else if (address == 9) { //10.GPS_COURSE (0-360deg, 0=north)
        if (sensors(SENSOR_GPS)) return sendIbusMeasurement(address, (uint16_t) (gpsSol.groundCourse / 10)); //int16_t
        else return sendIbusMeasurement(address, 0);
    } else if (address == 10) { //11. GPS_ALT //In cm => m
        if (sensors(SENSOR_GPS) && STATE(GPS_FIX)) return sendIbusMeasurement(address, (uint16_t) (gpsSol.llh.alt / 100));
        else return sendIbusMeasurement(address, 0);
    } else if (address == 11) { //12. GPS_LAT2 //Lattitude * 1e+7
        if (sensors(SENSOR_GPS)) return sendIbusMeasurement(address, (uint16_t) ((gpsSol.llh.lat % 100000)/10));
        else return sendIbusMeasurement(address, 0);
    } else if (address == 12) { //13. GPS_LON2 //Longitude * 1e+7
        if (sensors(SENSOR_GPS)) return sendIbusMeasurement(address, (uint16_t) ((gpsSol.llh.lon % 100000)/10));
        else return sendIbusMeasurement(address, 0);
    } else if (address == 13) { //14. GPS_LAT1 //Lattitude * 1e+7
        if (sensors(SENSOR_GPS)) return sendIbusMeasurement(address, (uint16_t) (gpsSol.llh.lat / 100000)); 
        else return sendIbusMeasurement(address, 0);
    } else if (address == 14) { //15. GPS_LON1 //Longitude * 1e+7
        if (sensors(SENSOR_GPS)) sendIbusMeasurement(address, (uint16_t) (gpsSol.llh.lon / 100000));
        else return sendIbusMeasurement(address, 0);
    } else if (address == 15) { //16. GPS_SPEED //In cm/s => km/h, 1cm/s = 0.0194384449 knots
        if (sensors(SENSOR_GPS)) return sendIbusMeasurement(address, (uint16_t) gpsSol.groundSpeed * 1944 / 10000); //int16_t
        else return sendIbusMeasurement(address, 0);
    }
#endif
	else return 0;
}

static uint8_t respondToIbusRequest(uint8_t ibusPacket[static IBUS_RX_BUF_LEN]) {
    ibusAddress_t returnAddress = getAddress(ibusPacket);
    if (returnAddress < sizeof SENSOR_ADDRESS_TYPE_LOOKUP) {
        if (isCommand(IBUS_COMMAND_DISCOVER_SENSOR, ibusPacket)) {
            return sendIbusCommand(returnAddress);
        } else if (isCommand(IBUS_COMMAND_SENSOR_TYPE, ibusPacket)) {
            return sendIbusSensorType(returnAddress);
        } else if (isCommand(IBUS_COMMAND_MEASUREMENT, ibusPacket)) {
            return dispatchMeasurementRequest(returnAddress);
        }
    }
	return 0;
}

void initSharedIbusTelemetry(serialPort_t *port) {
	ibusSerialPort = port;
}

#endif //defined(TELEMETRY) && defined(TELEMETRY_IBUS)

static uint16_t calculateChecksum(uint8_t ibusPacket[static IBUS_CHECKSUM_SIZE], size_t packetLength) {
    uint16_t checksum = 0xFFFF;
    for (size_t i = 0; i < packetLength - IBUS_CHECKSUM_SIZE; i++) {
        checksum -= ibusPacket[i];
    }
    return checksum;
}

static bool isChecksumOk(uint8_t ibusPacket[static IBUS_CHECKSUM_SIZE], size_t packetLength, uint16_t calculatedChecksum) {
	//uint16_t calculatedChecksum = calculateChecksum(ibusPacket, length);
    // Note that there's a byte order swap to little endian here
    return (calculatedChecksum >> 8) == ibusPacket[packetLength - 1]
            && (calculatedChecksum & 0xFF) == ibusPacket[packetLength - 2];
}

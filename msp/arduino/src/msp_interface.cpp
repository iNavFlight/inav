/*
  MSP.cpp - Arduino library for MSP V2 protocol (focused on INAV)

  Based on original work by Fabrizio Di Vittorio (fdivitto2013@gmail.com)
  Extended and refactored for wider INAV MSP support.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "msp_interface.h"

// --- CRC8 DVB-S2 Calculation ---
// (Copied from Betaflight/INAV source)
uint8_t MSPIntf::crc8_dvb_s2(uint8_t crc, unsigned char a)
{
    crc ^= a;
    for (int ii = 0; ii < 8; ++ii) {
        if (crc & 0x80) {
            crc = (crc << 1) ^ 0xD5;
        } else {
            crc = crc << 1;
        }
    }
    return crc;
}

// --- Constructor ---
MSPIntf::MSPIntf() {}

// --- Basic Setup ---
void MSPIntf::begin(Stream &stream, uint32_t timeout)
{
    _stream = &stream;
    _timeout = timeout;
    _last_status_arming_flags = 0; // Reset cached armed status
}

void MSPIntf::reset()
{
    if (!_stream) return;
    _stream->flush(); // Wait for outgoing data to be sent
    // Clear incoming buffer
    unsigned long start = millis();
    while (_stream->available() > 0 && (millis() - start < 100)) { // Timeout failsafe
        _stream->read();
    }
}

// --- Low Level MSP Communication ---

// Send MSP V2 frame ($X<)
void MSPIntf::send(uint16_t messageID, const void *payload, uint16_t size)
{
    if (!_stream) return;

    const uint8_t header[] = {'$', 'X', '<'};
    _stream->write(header, 3);

    uint8_t flag = 0; // Flag is always 0 for requests from GCS/companion
    uint8_t crc = 0;
    uint8_t tmp_buf[2];

    // Flag
    crc = crc8_dvb_s2(crc, flag);
    _stream->write(flag);

    // Message ID (LSB first)
    tmp_buf[0] = messageID & 0xFF;
    tmp_buf[1] = (messageID >> 8) & 0xFF;
    crc = crc8_dvb_s2(crc, tmp_buf[0]);
    crc = crc8_dvb_s2(crc, tmp_buf[1]);
    _stream->write(tmp_buf, 2);

    // Payload Size (LSB first)
    tmp_buf[0] = size & 0xFF;
    tmp_buf[1] = (size >> 8) & 0xFF;
    crc = crc8_dvb_s2(crc, tmp_buf[0]);
    crc = crc8_dvb_s2(crc, tmp_buf[1]);
    _stream->write(tmp_buf, 2);

    // Payload
    if (payload && size > 0) {
        const uint8_t *payloadPtr = (const uint8_t *)payload;
        for (uint16_t i = 0; i < size; ++i) {
            crc = crc8_dvb_s2(crc, payloadPtr[i]);
        }
        _stream->write(payloadPtr, size);
    }

    // Checksum
    _stream->write(crc);
    _stream->flush(); // Ensure data is sent
}

// Receive MSP V2 frame ($X>)
bool MSPIntf::recv(uint16_t *messageID, void *payload, uint16_t maxSize, uint16_t *recvSize)
{
    if (!_stream) return false;

    uint32_t t0 = millis();
    uint8_t state = 0; // 0: Wait $, 1: Wait X, 2: Wait >, 3: Read Flag, 4: Read ID, 5: Read Size, 6: Read Payload, 7: Read CRC
    uint8_t crcCalc = 0;
    uint8_t crcRecv = 0;
    uint16_t id = 0;
    uint16_t payloadSize = 0;
    uint16_t payloadRead = 0;
    uint8_t *payloadPtr = (uint8_t *)payload;
#if defined(MSP_DEBUG_RX)
    uint8_t dbgBuf[256];
    uint16_t dbgLen = 0;
#endif

    if (recvSize) *recvSize = 0; // Initialize received size

    while (millis() - t0 < _timeout) {
        if (_stream->available() > 0) {
            uint8_t c = _stream->read();
#if defined(MSP_DEBUG_RX)
            if (dbgLen < sizeof(dbgBuf)) dbgBuf[dbgLen++] = c;
#endif

            switch (state) {
                case 0: // Wait $
                    if (c == '$') state++;
                    break;
                case 1: // Wait X
                    state = (c == 'X') ? state + 1 : 0;
                    break;
                case 2: // Wait >
                    state = (c == '>') ? state + 1 : 0;
                    break;
                case 3: // Read Flag
                    crcCalc = crc8_dvb_s2(0, c); // Initialize CRC with flag
                    state++;
                    break;
                case 4: // Read ID (LSB, MSB)
                    id = c; // LSB
                    crcCalc = crc8_dvb_s2(crcCalc, c);
                    while (_stream->available() == 0 && millis() - t0 < _timeout) {} // Wait for next byte
                    if (_stream->available() == 0) return false; // Timeout
                    c = _stream->read();
                    id |= (uint16_t)c << 8; // MSB
                    crcCalc = crc8_dvb_s2(crcCalc, c);
                    if (messageID) *messageID = id;
                    state++;
                    break;
                case 5: // Read Size (LSB, MSB)
                    payloadSize = c; // LSB
                    crcCalc = crc8_dvb_s2(crcCalc, c);
                     while (_stream->available() == 0 && millis() - t0 < _timeout) {} // Wait for next byte
                    if (_stream->available() == 0) return false; // Timeout
                    c = _stream->read();
                    payloadSize |= (uint16_t)c << 8; // MSB
                    crcCalc = crc8_dvb_s2(crcCalc, c);
                    if (recvSize) *recvSize = payloadSize;
                    payloadRead = 0;
                    state = (payloadSize > 0) ? state + 1 : state + 2; // Skip to CRC if no payload
                     // Reset payload pointer for each message
                    payloadPtr = (uint8_t *)payload;
                    break;
                case 6: // Read Payload
                    if (payloadRead < maxSize && payload) {
                         *(payloadPtr++) = c;
                    } // else discard byte if buffer too small
                    crcCalc = crc8_dvb_s2(crcCalc, c);
                    payloadRead++;
                    if (payloadRead >= payloadSize) {
                        // Zero out remaining buffer if payload was smaller than max size
                        if (payload && payloadRead < maxSize) {
                             memset(payloadPtr, 0, maxSize - payloadRead);
                        }
                        state++;
                    }
                    break;
                case 7: // Read CRC
                    crcRecv = c;
                    // Serial.printf(" | ID: %d, Size: %d, CRC_Recv: %02X, CRC_Calc: %02X\n", id, payloadSize, crcRecv, crcCalc); // Debugging
                    if (crcRecv == crcCalc) {
#if defined(MSP_DEBUG_RX)
                        Serial.printf("MSP RX id=%u len=%u raw=", id, payloadSize);
                        for (uint16_t i = 0; i < dbgLen; ++i) {
                            Serial.printf("%02X", dbgBuf[i]);
                            if (i + 1 < dbgLen) Serial.print(' ');
                        }
                        Serial.println();
#endif
                        return true; // Success if CRC matches
                    }
                    return false;
            } // switch(state)
        } // if available
    } // while timeout

    // Serial.println(" | Timeout or Error"); // Debugging
    return false; // Timeout
}

// Wait for a specific message ID
bool MSPIntf::waitFor(uint16_t messageID, void *payload, uint16_t maxSize, uint16_t *recvSize)
{
    uint16_t recvMessageID;
    uint16_t currentRecvSize;
    uint32_t t0 = millis();

    // Use local recvSize if caller doesn't provide one
    uint16_t *pRecvSize = recvSize ? recvSize : &currentRecvSize;

    while (millis() - t0 < _timeout) {
        if (recv(&recvMessageID, payload, maxSize, pRecvSize)) {
            if (messageID == recvMessageID) {
                return true; 
            }
        }
         // Allow other tasks to run briefly
        // delay(1); // Consider adding a small delay if needed in cooperative multitasking
    }

    return false; // Timeout
}

// Send a request and wait for the reply with the same message ID
bool MSPIntf::request(uint16_t messageID, void *payload, uint16_t maxSize, uint16_t *recvSize)
{
    // Clear any stale data before sending
    //reset(); // Optional: Consider if clearing buffer before request is needed

    send(messageID, nullptr, 0);
    return waitFor(messageID, payload, maxSize, recvSize);
}

// Send a request whose reply is a fixed-size struct and require the FC to send
// the whole struct. recv() reports the wire payload size, so a short or empty
// frame would otherwise leave part of the caller's struct stale.
bool MSPIntf::requestFixed(uint16_t messageID, void *payload, uint16_t size)
{
    uint16_t recvSize = 0;
    return request(messageID, payload, size, &recvSize) && recvSize >= size;
}

// Send a command, optionally wait for ACK (reply with same message ID, zero payload)
bool MSPIntf::command(uint16_t messageID, const void *payload, uint16_t size, bool waitACK)
{
    // Clear any stale data before sending
    //reset(); // Optional: Consider if clearing buffer before command is needed

    send(messageID, payload, size);

    if (waitACK) {
        uint16_t ackRecvSize = 0;
        if (waitFor(messageID, nullptr, 0, &ackRecvSize)) {
             return (ackRecvSize == 0);
        } else {
            return false; 
        }
    }

    return true; 
}

// --- High Level Functions ---

// --- Version & Board Info ---
bool MSPIntf::requestApiVersion(mspApiVersionReply_t *reply) {
    return requestFixed(MSP_API_VERSION, reply, sizeof(*reply));
}
bool MSPIntf::requestFcVariant(mspFcVariantReply_t *reply) {
    return requestFixed(MSP_FC_VARIANT, reply, sizeof(*reply));
}
bool MSPIntf::requestFcVersion(mspFcVersionReply_t *reply) {
    return requestFixed(MSP_FC_VERSION, reply, sizeof(*reply));
}
bool MSPIntf::requestBoardInfo(mspBoardInfoReply_t *reply, char *targetNameBuf, uint8_t targetNameBufLen) {
    // The fixed header and the target-name tail arrive in one frame, and recv()
    // consumes the whole frame, so request both in one piece.
    uint8_t payload[sizeof(mspBoardInfoReply_t) + MAX_TARGET_NAME_LENGTH];
    uint16_t recvSize = 0;
    if (!request(MSP_BOARD_INFO, payload, sizeof(payload), &recvSize)) return false;
    if (recvSize < sizeof(*reply)) return false;
    memcpy(reply, payload, sizeof(*reply));
    if (!targetNameBuf || targetNameBufLen == 0) return true;

    uint16_t nameLen = (uint16_t)(recvSize - sizeof(*reply));
    if (nameLen > reply->targetNameLength) nameLen = reply->targetNameLength;
    if (nameLen > (uint16_t)(targetNameBufLen - 1)) nameLen = (uint16_t)(targetNameBufLen - 1);
    memcpy(targetNameBuf, payload + sizeof(*reply), nameLen);
    targetNameBuf[nameLen] = '\0';
    return true;
}
bool MSPIntf::requestBuildInfo(mspBuildInfoReply_t *reply) {
    return requestFixed(MSP_BUILD_INFO, reply, sizeof(*reply));
}
bool MSPIntf::requestUID(mspUidReply_t *reply) {
    return requestFixed(MSP_UID, reply, sizeof(*reply));
}
bool MSPIntf::requestCraftName(char *name, uint8_t bufLen) {
    uint16_t recvSize = 0;
    if (!name || bufLen == 0) return false;
    if (request(MSP_NAME, name, bufLen - 1, &recvSize)) {
        name[min((uint16_t)(bufLen - 1), recvSize)] = '\0';
        return true;
    }
    return false;
}
bool MSPIntf::setCraftName(const char *name) {
    if (!name) return false;
    uint16_t len = strlen(name);
    if (len > MAX_NAME_LENGTH) len = MAX_NAME_LENGTH;
    return command(MSP_SET_NAME, name, len);
}

// --- Status ---
bool MSPIntf::requestStatus(mspStatusReply_t *reply) {
    return requestFixed(MSP_STATUS, reply, sizeof(*reply));
}
bool MSPIntf::requestStatusEx(mspStatusExReply_t *reply) {
    return requestFixed(MSP_STATUS_EX, reply, sizeof(*reply));
}
bool MSPIntf::requestInavStatus(msp2InavStatusReply_t *reply) {
    bool success = requestFixed(MSP2_INAV_STATUS, reply, sizeof(*reply));
    if (success) {
        _last_status_arming_flags = reply->armingFlags; // Cache for isArmed()
    }
    return success;
}

// Check the ARMED flag (bit 2) from last cached status
bool MSPIntf::isArmed() {
    return (_last_status_arming_flags & ARMED) != 0;
}

// --- Sensor Data ---
bool MSPIntf::requestRawIMU(mspRawImuReply_t *reply) {
    return requestFixed(MSP_RAW_IMU, reply, sizeof(*reply));
}
bool MSPIntf::requestAttitude(mspAttitudeReply_t *reply) {
    return requestFixed(MSP_ATTITUDE, reply, sizeof(*reply));
}
bool MSPIntf::requestAltitude(mspAltitudeReply_t *reply) {
    return requestFixed(MSP_ALTITUDE, reply, sizeof(*reply));
}
bool MSPIntf::requestSonarAltitude(mspSonarAltitudeReply_t *reply) {
    return requestFixed(MSP_SONAR_ALTITUDE, reply, sizeof(*reply));
}
bool MSPIntf::requestAirspeed(msp2InavAirSpeedReply_t *reply) {
    return requestFixed(MSP2_INAV_AIR_SPEED, reply, sizeof(*reply));
}

// --- GPS & Navigation ---
bool MSPIntf::requestRawGPS(mspRawGpsReply_t *reply) {
    return requestFixed(MSP_RAW_GPS, reply, sizeof(*reply));
}
bool MSPIntf::requestCompGPS(mspCompGpsReply_t *reply) {
    return requestFixed(MSP_COMP_GPS, reply, sizeof(*reply));
}
bool MSPIntf::requestNavStatus(mspNavStatusReply_t *reply) {
    return requestFixed(MSP_NAV_STATUS, reply, sizeof(*reply));
}
bool MSPIntf::setHeading(int16_t headingDeg) {
    // Ensure heading is within 0-359 range if necessary, though FC might handle wrap-around.
    uint16_t heading = (uint16_t)constrain(headingDeg, 0, 359);
    mspSetHeadRequest_t payload;
    payload.heading = heading;
    return command(MSP_SET_HEAD, &payload, sizeof(payload));
}

// --- Waypoints & Missions ---
bool MSPIntf::requestWaypointInfo(mspWpGetinfoReply_t *reply) {
    return requestFixed(MSP_WP_GETINFO, reply, sizeof(*reply));
}

bool MSPIntf::requestWaypoint(uint8_t index, mspWpReply_t *reply) {
    uint8_t payload = index;
    uint16_t recvSize = 0;
    send(MSP_WP, &payload, sizeof(payload));
    if (!waitFor(MSP_WP, reply, sizeof(*reply), &recvSize)) return false;
    if (recvSize < sizeof(*reply)) return false;
    return (reply->waypointIndex == index);
}

bool MSPIntf::setWaypoint(const mspSetWpRequest_t *wp) {
    if (!wp) return false;
    // Check if index is valid (optional, FC validates too)
    // if (wp->waypointIndex >= MSP_MAX_WAYPOINTS) return false; // Assuming MSP_MAX_WAYPOINTS is defined
    // INAV does not send a zero-length ACK for MSP_SET_WP, so don't wait for one.
    return command(MSP_SET_WP, wp, sizeof(*wp), false);
}

bool MSPIntf::commandMissionLoad(uint8_t missionId) {
    mspWpMissionLoadRequest_t payload;
    payload.missionID = missionId;
    // This command might take time, ACK might not be immediate?
    // Check INAV source if ACK behavior is standard. Assuming standard ACK.
    return command(MSP_WP_MISSION_LOAD, &payload, sizeof(payload));
}

bool MSPIntf::commandMissionSave(uint8_t missionId) {
    mspWpMissionLoadRequest_t payload;
    payload.missionID = missionId;
     // This command might take time (EEPROM write). Increase timeout if needed?
    return command(MSP_WP_MISSION_SAVE, &payload, sizeof(payload));
}

// --- Modes ---
bool MSPIntf::requestBoxIDs(uint8_t *boxIds, uint16_t maxIds, uint16_t *count) {
     if (!boxIds || maxIds == 0 || !count) return false;
     uint16_t recvSize = 0;
     if (request(MSP_BOXIDS, boxIds, maxIds, &recvSize)) {
         // recv() reports the wire size even when it discarded bytes past the
         // buffer, so count only the IDs that were stored.
         *count = (recvSize > maxIds) ? maxIds : recvSize;
         return true;
     }
     *count = 0;
     return false;
}

bool MSPIntf::requestModeRanges(mspModeRangesReply_t *table, uint16_t *count) {
    if (!table || !count) return false;
    *count = 0;
    if (!requestFixed(MSP_MODE_RANGES, table, sizeof(*table))) return false;
    // The reply is the whole fixed table; the wire size cannot exceed what was
    // stored, so report exactly the entries the table holds.
    *count = (uint16_t)(sizeof(table->items) / sizeof(table->items[0]));
    return true;
}

// --- RC & Motors ---
bool MSPIntf::requestRcChannels(mspRcReplyElem_t *channels, uint8_t maxChannels,
                                uint8_t *received) {
    uint16_t recvSize = 0;
    if (!channels || maxChannels == 0) return false;
    if (!request(MSP_RC, channels, sizeof(mspRcReplyElem_t) * maxChannels, &recvSize)) {
        return false;
    }
    if (recvSize == 0 || recvSize % sizeof(mspRcReplyElem_t) != 0) return false;
    uint16_t count = (uint16_t)(recvSize / sizeof(mspRcReplyElem_t));
    if (count > maxChannels) count = maxChannels; // recv() discarded the rest
    if (received) *received = (uint8_t)count;
    return true;
}

bool MSPIntf::commandRawRC(const int16_t *channels, uint8_t channelCount) {
    if (!channels || channelCount == 0 || channelCount > MAX_RC_CHANNELS) return false;
    uint16_t payload[MAX_RC_CHANNELS];
    for (uint8_t i = 0; i < channelCount; ++i) {
        payload[i] = (channels[i] < 0) ? 0 : static_cast<uint16_t>(channels[i]);
    }
    return command(MSP_SET_RAW_RC, payload, sizeof(uint16_t) * channelCount, false);
}

bool MSPIntf::requestMotorOutputs(mspMotorReply_t *reply) {
    return requestFixed(MSP_MOTOR, reply, sizeof(*reply));
}

bool MSPIntf::commandMotorOutputs(const uint16_t *motorValues) {
     if (!motorValues) return false;
     uint16_t payload[MAX_SUPPORTED_MOTORS];
     memcpy(payload, motorValues, sizeof(payload));
     return command(MSP_SET_MOTOR, payload, sizeof(payload), false);
}

/*
// --- Configuration ---
// We are not doing configuration
bool MSPIntf::requestNavPosholdConfig(mspNavPosholdReply_t *reply) {
    return request(MSP_NAV_POSHOLD, reply, sizeof(*reply));
}
bool MSPIntf::requestVoltageMeterConfig(mspVoltageMeterConfigReply_t *reply){
    return request(MSP_VOLTAGE_METER_CONFIG, reply, sizeof(*reply));
}
bool MSPIntf::requestBatteryConfig(msp2InavBatteryConfigReply_t *reply){
    return request(MSP2_INAV_BATTERY_CONFIG, reply, sizeof(*reply));
}
bool MSPIntf::requestSensorConfig(mspSensorConfigReply_t *reply){
     return request(MSP_SENSOR_CONFIG, reply, sizeof(*reply));
}
bool MSPIntf::setVoltageMeterConfig(const mspVoltageMeterConfigReply_t *config){
    if (!config) return false;
    // Size check: INAV expects exactly 4 bytes for V1 SET_VOLTAGE_METER_CONFIG
    if (sizeof(*config) != 4) { return false; }
    return command(MSP_SET_VOLTAGE_METER_CONFIG, config, sizeof(*config));
}
bool MSPIntf::setNavPosholdConfig(const mspNavPosholdReply_t *config) {
    if (!config) return false;
    if (sizeof(*config) != 13) { return false; }
    return command(MSP_SET_NAV_POSHOLD, config, sizeof(*config));
}
bool MSPIntf::setBatteryConfig(const msp2InavBatteryConfigReply_t *config){
     if (!config) return false;
    if (sizeof(*config) != 29) { return false; }
    return command(MSP2_INAV_SET_BATTERY_CONFIG, config, sizeof(*config));
}
bool MSPIntf::setSensorConfig(const mspSensorConfigReply_t *config){
    if (!config) return false;
    // Size check: INAV expects exactly 6 bytes for SET_SENSOR_CONFIG
    if (sizeof(*config) != 6) { return false; }
    return command(MSP_SET_SENSOR_CONFIG, config, sizeof(*config));
}
*/

// --- Battery & Power ---
bool MSPIntf::requestBatteryState(mspBatteryStateReply_t *reply){
    return requestFixed(MSP_BATTERY_STATE, reply, sizeof(*reply));
}
bool MSPIntf::requestAnalog(msp2InavAnalogReply_t *reply){
    return requestFixed(MSP2_INAV_ANALOG, reply, sizeof(*reply));
}

// --- Calibration & System ---
/*bool MSPIntf::commandAccCalibration() {
    // No payload, expect ACK
    return command(MSP_ACC_CALIBRATION, nullptr, 0);
}
bool MSPIntf::commandMagCalibration() {
    // No payload, expect ACK
    return command(MSP_MAG_CALIBRATION, nullptr, 0);
}
bool MSPIntf::commandResetConfig() {
    // No payload, potentially long operation, ACK might be delayed or sent before reset?
    // Test required. Assuming standard ACK for now.
    return command(MSP_RESET_CONF, nullptr, 0);
}
bool MSPIntf::commandEepromWrite() {
    // No payload, potentially long operation (EEPROM write).
    return command(MSP_EEPROM_WRITE, nullptr, 0);
} 
//Hell no.
*/

// --- Programming Framework ---
bool MSPIntf::requestGvarStatus(msp2InavGvarStatusReply_t *reply){
    return requestFixed(MSP2_INAV_GVAR_STATUS, reply, sizeof(*reply));
}
bool MSPIntf::requestLogicConditionsStatus(msp2InavLogicConditionsStatusReply_t *reply){
    return requestFixed(MSP2_INAV_LOGIC_CONDITIONS_STATUS, reply, sizeof(*reply));
}

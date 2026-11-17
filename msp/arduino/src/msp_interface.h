/*
  MSP.h - Arduino library for MSP V2 protocol (focused on INAV)

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

#pragma once

#include <Arduino.h>
#include <Stream.h>
#include "msp_config.h"
#include "generated/msp_consts.h"
#include "generated/msp_enums.h"
#include "generated/msp_wire_types.h"
#include "generated/msp_msgs.h"

class MSPIntf {
  public:
    // --- Constructor & Basic Setup ---
    MSPIntf();
    void begin(Stream &stream, uint32_t timeout = 500);
    void reset(); // Clears serial buffer

    // --- Low Level MSP Communication ---
    void send(uint16_t messageID, const void *payload = nullptr, uint16_t size = 0);
    bool recv(uint16_t *messageID, void *payload, uint16_t maxSize, uint16_t *recvSize = nullptr);
    bool waitFor(uint16_t messageID, void *payload, uint16_t maxSize, uint16_t *recvSize = nullptr);
    bool request(uint16_t messageID, void *payload, uint16_t maxSize, uint16_t *recvSize = nullptr);
    bool command(uint16_t messageID, const void *payload = nullptr, uint16_t size = 0, bool waitACK = true);

    // --- High Level Functions ---

    // Version & Board Info
    bool requestApiVersion(mspApiVersionReply_t *reply);
    bool requestFcVariant(mspFcVariantReply_t *reply);
    bool requestFcVersion(mspFcVersionReply_t *reply);
    bool requestBoardInfo(mspBoardInfoReply_t *reply, char *targetNameBuf = nullptr, uint8_t targetNameBufLen = 0); // Handles variable length name
    bool requestBuildInfo(mspBuildInfoReply_t *reply);
    bool requestUID(mspUidReply_t *reply);
    // MSP_NAME's payload is a bare char array, so the caller supplies the buffer.
    bool requestCraftName(char *name, uint8_t bufLen);
    bool setCraftName(const char *name);

    // Status
    bool requestStatus(mspStatusReply_t *reply); // Legacy status
    bool requestStatusEx(mspStatusExReply_t *reply); // Extended V1 status
    bool requestInavStatus(msp2InavStatusReply_t *reply); // Recommended modern status
    bool isArmed(); // Checks armed status based on last requestInavStatus

    // Sensor Data
    bool requestRawIMU(mspRawImuReply_t *reply);
    bool requestAttitude(mspAttitudeReply_t *reply);
    bool requestAltitude(mspAltitudeReply_t *reply);
    bool requestSonarAltitude(mspSonarAltitudeReply_t *reply); // Rangefinder
    bool requestAirspeed(msp2InavAirSpeedReply_t *reply); // Pitot/Estimated

    // GPS & Navigation
    bool requestRawGPS(mspRawGpsReply_t *reply);
    bool requestCompGPS(mspCompGpsReply_t *reply); // Distance/Direction to home
    bool requestNavStatus(mspNavStatusReply_t *reply);
    bool setHeading(int16_t headingDeg); // Sets MAGHOLD target

    // Waypoints & Missions
    bool requestWaypointInfo(mspWpGetinfoReply_t *reply);
    bool requestWaypoint(uint8_t index, mspWpReply_t *reply);
    bool setWaypoint(const mspSetWpRequest_t *wp);
    bool commandMissionLoad(uint8_t missionId = 0);
    bool commandMissionSave(uint8_t missionId = 0);

    // Modes
    bool requestBoxIDs(uint8_t *boxIds, uint16_t maxIds, uint16_t *count); // Get permanent IDs
    // getActiveModes is complex due to bitmask size, better handled in sketch using requestInavStatus
    // The reply is the whole table; count reports how many entries arrived.
    bool requestModeRanges(mspModeRangesReply_t *table, uint16_t *count);

    // RC & Motors
    // MSP_RC's payload is a bare int16 array; returns how many channels arrived.
    bool requestRcChannels(mspRcReplyElem_t *channels, uint8_t maxChannels,
                           uint8_t *received = nullptr);
    bool commandRawRC(const int16_t *channels, uint8_t channelCount);
    bool requestMotorOutputs(mspMotorReply_t *reply);
    bool commandMotorOutputs(const uint16_t *motorValues); // For motor testing

    // Configuration
    // We will *not* implement configuration messages
    bool requestNavPosholdConfig(mspNavPosholdReply_t *reply);
    //bool setNavPosholdConfig(const mspNavPosholdReply_t *config);
    bool requestVoltageMeterConfig(mspVoltageMeterConfigReply_t *reply); // Legacy
    //bool setVoltageMeterConfig(const mspVoltageMeterConfigReply_t *config); // Legacy
    bool requestBatteryConfig(msp2InavBatteryConfigReply_t *reply); // Modern
    //bool setBatteryConfig(const msp2_inav_battery_config_t *config); // Modern
    bool requestSensorConfig(mspSensorConfigReply_t *reply);
    //bool setSensorConfig(const mspSensorConfigReply_t *config);

    // Battery & Power
    bool requestBatteryState(mspBatteryStateReply_t *reply);
    bool requestAnalog(msp2InavAnalogReply_t *reply); // Modern analog data

    // Calibration
    /*bool commandAccCalibration();
    bool commandMagCalibration();
    bool commandResetConfig(); // Reset to defaults
    bool commandEepromWrite(); // Save current config*/
    // Hell no

    // Programming Framework (Example)
    bool requestGvarStatus(msp2InavGvarStatusReply_t *reply);
    bool requestLogicConditionsStatus(msp2InavLogicConditionsStatusReply_t *reply);

  private:
    Stream *_stream = nullptr;
    uint32_t _timeout = 500;
    uint32_t _last_status_arming_flags = 0; // Cache for isArmed()

    // Fixed-size replies must carry the whole struct: a short or empty frame
    // would leave part of the caller's struct stale.
    bool requestFixed(uint16_t messageID, void *payload, uint16_t size);

    static uint8_t crc8_dvb_s2(uint8_t crc, unsigned char a);
};

#pragma once

#include <Arduino.h>
#include <Stream.h>
#include <vector>

#include "msp_interface.h"
#include "msp_config.h"
#include "generated/msp_consts.h"
#include "generated/msp_enums.h"

#include "generated/msp_wire_types.h"
#include "generated/msp_msgs.h"

// ---------- High level, SI-friendly structs ----------

struct inav_nav_waypoint {
    unsigned int  waypointIndex;
    unsigned int  action;
    double        latitude;   // deg
    double        longitude;  // deg
    float         altitude;   // m
    unsigned int  param1;
    unsigned int  param2;
    unsigned int  param3;
    unsigned int  flag;
};

struct inav_nav_data {
    double latitude;     
    double longitude;     
    float  altitude;      
    int    navMode;
    int    navState;
    int    activeWpAction;
    int    activeWpNumber;
    int    navError;
    int    targetHeading; // deg
};

// MSP_RC's payload is a bare channel array, so the count is whatever the FC
// sent. This carries it alongside the values.
struct inav_rc_channels {
    mspRcReplyElem_t channel[MAX_RC_CHANNELS];
    uint8_t count;
};

struct inav_altitude {
    float estimatedAltitude; // m
    float variometer;        // m/s
    float baroAltitude;      // m
};

struct inav_attitude {
    float roll;   // deg
    float pitch;  // deg
    int   yaw;    // deg (FC already sends whole deg)
};

struct inav_gps_data {
    uint8_t fixType;
    uint8_t sats;
    double  latitude;   // deg
    double  longitude;  // deg
    float   altitude;   // m
    float   speed;      // m/s
    float   course;     // deg
    float   hdop;       // unitless
};

struct inav_comp_gps {
    uint16_t distanceToHome; // m
    uint16_t directionToHome;// deg
};

struct inav_gps_stats {
    // drop in later if you care
};

struct inav_mode_ranges {
    // The generated reply already holds the whole table, one entry per
    // MAX_MODE_ACTIVATION_CONDITION_COUNT; count is how many the FC filled in.
    mspModeRangesReply_t table;
    uint16_t count;
};

struct inav_sensors {
    bool acc, baro, mag, gps, rangefinder, opflow, pitot, temp, hwFailure;
};

struct inav_status {
    uint16_t     cycleTime_us;
    uint16_t     cpuLoad_pct;
    inav_sensors sensors;
    uint8_t      profile;
    uint8_t      battProfile;
    uint8_t      mixerProfile;
    bool         armed;
    uint32_t     armingFlags;
    boxBitmask_t activeModes;
};

struct inav_analog {
    uint8_t  batteryFlags;
    float    vbat_V;
    float    amps_A;
    uint32_t power_mW;
    uint32_t mAhDrawn;
    uint32_t mWhDrawn;
    uint32_t remainingCapacity;
    uint8_t  percentRemaining;
    uint16_t rssi;
};

// ---------- API class ----------
class InavAPI {
  public:
    InavAPI();

    // Init with an already-started Stream
    void Init(Stream &serial, uint32_t timeoutMs = 500);

    // Helpers
    int  float_to_pwm(float in);
    bool isConnected();

    // Pass-through info getters (raw MSP structs are fine)
    bool get_api_version(mspApiVersionReply_t &out);
    bool get_fc_variant(mspFcVariantReply_t &out);
    bool get_fc_version(mspFcVersionReply_t &out);
    bool get_board_info(mspBoardInfoReply_t &out, char *targetNameBuf = nullptr, uint8_t len = 0);

    // High-level GETs (return true on success)
    bool get_altitude(inav_altitude &out);
    bool get_gps(inav_gps_data &out);
    bool get_compgps(inav_comp_gps &out);
    bool get_nav_status(inav_nav_data &out);
    bool get_attitude(inav_attitude &out);
    bool get_imu(mspRawImuReply_t &out);
    bool get_rc(inav_rc_channels &out);
    bool get_rc_ch(int ch, int &value);
    bool get_sensor_config(mspSensorConfigReply_t &out);
    bool get_wp(int index, inav_nav_waypoint &out);
    bool get_mode_ranges(inav_mode_ranges &out);
    bool get_active_modes(std::vector<uint8_t> &out); // bit indices (or translate if you cache BOXIDs)
    bool get_status(inav_status &out);
    bool get_analog(inav_analog &out);

    // SETs / commands
    bool set_waypoint(const inav_nav_waypoint &wp);
    bool set_rc(const inav_rc_channels &rcin);
    bool command_mission_save(uint8_t id = 0);
    bool command_mission_load(uint8_t id = 0);
    bool command_heading(int16_t headingDeg);

    // CHECKs
    bool check_can_arm();          // crude
    bool check_is_armed();
    bool check_override_enabled(); // always true w/ MSP override
    bool check_override_active();  // ditto

  private:
    MSPIntf msp;
    bool    _inited = false;

    msp2InavStatusReply_t _lastStatus{};
    bool               _haveStatus = false;

    void   _updateStatus();

    // unit helpers
    static double _i32_to_deg(int32_t x)   { return (double)x / 1e7; }
    static float  _cm_to_m(int32_t cm)     { return (float)cm / 100.0f; }
    static float  _cms_to_ms(int16_t cms)  { return (float)cms / 100.0f; }
    static float  _cms_to_ms_u16(uint16_t cms){ return (float)cms / 100.0f; }
    static float  _ddeg_to_deg(int16_t dd) { return (float)dd / 10.0f; }
};

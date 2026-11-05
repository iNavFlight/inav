#include "inav_api.h"

// ---------- ctor ----------
InavAPI::InavAPI() {}

// ---------- init ----------
void InavAPI::Init(Stream &serial, uint32_t timeoutMs) {
    msp.begin(serial, timeoutMs);
    _inited = true;
    _haveStatus = false;
}

// ---------- helpers ----------
int InavAPI::float_to_pwm(float in) {
    if (in < 0.0f) in = 0.0f;
    if (in > 1.0f) in = 1.0f;
    return PWM_RANGE_MIN + (int)((PWM_RANGE_MAX - PWM_RANGE_MIN) * in);
}

bool InavAPI::isConnected() {
    if (!_inited) return false;
    mspApiVersionReply_t v{};
    return msp.requestApiVersion(&v);
}

void InavAPI::_updateStatus() {
    if (msp.requestInavStatus(&_lastStatus)) {
        _haveStatus = true;
    } else {
        _haveStatus = false;
    }
}

// ---------- pass-through basics ----------
bool InavAPI::get_api_version(mspApiVersionReply_t &o) {
    return msp.requestApiVersion(&o);
}
bool InavAPI::get_fc_variant(mspFcVariantReply_t &o) {
    return msp.requestFcVariant(&o);
}
bool InavAPI::get_fc_version(mspFcVersionReply_t &o) {
    return msp.requestFcVersion(&o);
}
bool InavAPI::get_board_info(mspBoardInfoReply_t &o, char *buf, uint8_t len) {
    return msp.requestBoardInfo(&o, buf, len);
}

// ---------- GETs ----------
bool InavAPI::get_altitude(inav_altitude &out) {
    mspAltitudeReply_t a{};
    if (!msp.requestAltitude(&a)) return false;
    out.estimatedAltitude = _cm_to_m(a.estimatedAltitude);
    out.variometer        = _cms_to_ms(a.variometer);
    out.baroAltitude      = _cm_to_m(a.baroAltitude);
    return true;
}

bool InavAPI::get_gps(inav_gps_data &out) {
    mspRawGpsReply_t g{};
    if (!msp.requestRawGPS(&g)) return false;
    out.fixType   = g.fixType;
    out.sats      = g.numSat;
    out.latitude  = _i32_to_deg(g.latitude);
    out.longitude = _i32_to_deg(g.longitude);
    out.altitude  = (float)g.altitude;
    out.speed     = _cms_to_ms_u16(g.speed);
    out.course    = _ddeg_to_deg(g.groundCourse);
    out.hdop      = (float)g.hdop / 100.0f;
    return true;
}

bool InavAPI::get_compgps(inav_comp_gps &out) {
    mspCompGpsReply_t c{};
    if (!msp.requestCompGPS(&c)) return false;
    out.distanceToHome  = c.distanceToHome;
    out.directionToHome = c.directionToHome;
    return true;
}

bool InavAPI::get_nav_status(inav_nav_data &nd) {
    mspNavStatusReply_t ns{};
    if (!msp.requestNavStatus(&ns)) return false;
    nd.navMode        = ns.navMode;
    nd.navState       = ns.navState;
    nd.activeWpAction = ns.activeWpAction;
    nd.activeWpNumber = ns.activeWpNumber;
    nd.navError       = ns.navError;
    nd.targetHeading  = ns.targetHeading;
    return true;
}

bool InavAPI::get_attitude(inav_attitude &out) {
    mspAttitudeReply_t a{};
    if (!msp.requestAttitude(&a)) return false;
    out.roll  = _ddeg_to_deg(a.roll);
    out.pitch = _ddeg_to_deg(a.pitch);
    out.yaw   = a.yaw;
    return true;
}

bool InavAPI::get_imu(mspRawImuReply_t &out) {
    return msp.requestRawIMU(&out);
}

bool InavAPI::get_rc(inav_rc_channels &out) {
    out.count = 0;
    return msp.requestRcChannels(out.channel, MAX_RC_CHANNELS, &out.count);
}

bool InavAPI::get_rc_ch(int ch, int &value) {
    if (ch < 0 || ch >= MAX_RC_CHANNELS) return false;
    inav_rc_channels rc{};
    if (!get_rc(rc)) return false;
    if (ch >= rc.count) return false;
    value = rc.channel[ch];
    return true;
}

bool InavAPI::get_sensor_config(mspSensorConfigReply_t &out) {
    return msp.requestSensorConfig(&out);
}

bool InavAPI::get_wp(int index, inav_nav_waypoint &out) {
    if (index < 0 || index >= NAV_MAX_WAYPOINTS) return false;
    mspWpReply_t raw{};
    if (!msp.requestWaypoint((uint8_t)index, &raw)) return false;

    out.waypointIndex = raw.waypointIndex;
    out.action        = raw.action;
    out.latitude      = _i32_to_deg(raw.latitude);
    out.longitude     = _i32_to_deg(raw.longitude);
    out.altitude      = _cm_to_m(raw.altitude);
    out.param1        = raw.param1;
    out.param2        = raw.param2;
    out.param3        = raw.param3;
    out.flag          = raw.flag;
    return true;
}

bool InavAPI::get_mode_ranges(inav_mode_ranges &out) {
    uint16_t count = 0;
    if (!msp.requestModeRanges(&out.table, &count)) {
        out.count = 0;
        return false;
    }
    out.count = count;
    return true;
}

bool InavAPI::get_active_modes(std::vector<uint8_t> &out) {
    _updateStatus();
    if (!_haveStatus) return false;
    out.clear();
    for (uint8_t bit = 0; bit < CHECKBOX_ITEM_COUNT; ++bit) {
        /* activeModes is INAV's boxBitmask_t, so walk it with INAV's own
         * bitarray helpers rather than assuming a width. */
        if (bitArrayGet(_lastStatus.activeModes.bits, bit)) {
            out.push_back(bit); // still bit index; translate if you cached BOXIDs
        }
    }
    return true;
}

static void decodeSensorBits(uint16_t bits, inav_sensors &s) {
    s.acc         = bits & (1 << 0);
    s.baro        = bits & (1 << 1);
    s.mag         = bits & (1 << 2);
    s.gps         = bits & (1 << 3);
    s.rangefinder = bits & (1 << 4);
    s.opflow      = bits & (1 << 5);
    s.pitot       = bits & (1 << 6);
    s.temp        = bits & (1 << 7);
    s.hwFailure   = bits & (1 << 15);
}

bool InavAPI::get_status(inav_status &out) {
    msp2InavStatusReply_t raw{};
    if (!msp.requestInavStatus(&raw)) return false;

    _lastStatus = raw;
    _haveStatus = true;

    out.cycleTime_us  = raw.cycleTime;
    out.cpuLoad_pct   = raw.cpuLoad;
    out.profile       =  raw.profileAndBattProfile & 0x0F;
    out.battProfile   = (raw.profileAndBattProfile >> 4) & 0x0F;
    out.mixerProfile  = raw.mixerProfile;
    out.armed         = msp.isArmed();
    out.armingFlags   = raw.armingFlags;
    out.activeModes   = raw.activeModes;

    decodeSensorBits(raw.sensorStatus, out.sensors);
    return true;
}

bool InavAPI::get_analog(inav_analog &out) {
    msp2InavAnalogReply_t a{};
    if (!msp.requestAnalog(&a)) return false;

    out.batteryFlags      = a.batteryFlags;
    out.vbat_V            = (float)a.vbat / 100.0f;
    out.amps_A            = (float)a.amperage / 100.0f;
    out.power_mW          = a.powerDraw;
    out.mAhDrawn          = a.mAhDrawn;
    out.mWhDrawn          = a.mWhDrawn;
    out.remainingCapacity = a.remainingCapacity;
    out.percentRemaining  = a.percentageRemaining;
    out.rssi              = a.rssi;
    return true;
}

// ---------- SETs / Cmd ----------
bool InavAPI::set_waypoint(const inav_nav_waypoint &wp) {
    mspSetWpRequest_t raw{};
    raw.waypointIndex = (uint8_t)wp.waypointIndex;
    raw.action        = (navWaypointActions_e)wp.action;
    raw.latitude      = (int32_t)(wp.latitude  * 1e7);
    raw.longitude     = (int32_t)(wp.longitude * 1e7);
    raw.altitude      = (int32_t)(wp.altitude  * 100.0f);
    raw.param1        = (uint16_t)wp.param1;
    raw.param2        = (uint16_t)wp.param2;
    raw.param3        = (uint16_t)wp.param3;
    raw.flag          = (navWaypointFlags_e)wp.flag;
    return msp.setWaypoint(&raw);
}

bool InavAPI::set_rc(const inav_rc_channels &rcin) {
    return msp.commandRawRC(rcin.channel, rcin.count ? rcin.count : MAX_RC_CHANNELS);
}

bool InavAPI::command_mission_save(uint8_t id) {
    return msp.commandMissionSave(id);
}

bool InavAPI::command_mission_load(uint8_t id) {
    return msp.commandMissionLoad(id);
}

bool InavAPI::command_heading(int16_t headingDeg) {
    return msp.setHeading(headingDeg);
}

// ---------- CHECKs ----------
bool InavAPI::check_can_arm() {
    _updateStatus();
    if (!_haveStatus) return false;
    return !check_is_armed();
}

bool InavAPI::check_is_armed() {
    _updateStatus();
    if (!_haveStatus) return false;
    return msp.isArmed();
}

bool InavAPI::check_override_enabled() { return true; }
bool InavAPI::check_override_active()  { return true; }

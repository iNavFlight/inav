#pragma once
// MESSAGE MISSION_CHANGED PACKING

#define MAVLINK_MSG_ID_MISSION_CHANGED 52


typedef struct __mavlink_mission_changed_t {
 int16_t start_index; /*<  Start index for partial mission change (-1 for all items).*/
 int16_t end_index; /*<  End index of a partial mission change. -1 is a synonym for the last mission item (i.e. selects all items from start_index). Ignore field if start_index=-1.*/
 uint8_t origin_sysid; /*<  System ID of the author of the new mission.*/
 uint8_t origin_compid; /*<  Compnent ID of the author of the new mission.*/
 uint8_t mission_type; /*<  Mission type.*/
} mavlink_mission_changed_t;

#define MAVLINK_MSG_ID_MISSION_CHANGED_LEN 7
#define MAVLINK_MSG_ID_MISSION_CHANGED_MIN_LEN 7
#define MAVLINK_MSG_ID_52_LEN 7
#define MAVLINK_MSG_ID_52_MIN_LEN 7

#define MAVLINK_MSG_ID_MISSION_CHANGED_CRC 132
#define MAVLINK_MSG_ID_52_CRC 132



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_MISSION_CHANGED { \
    52, \
    "MISSION_CHANGED", \
    5, \
    {  { "start_index", NULL, MAVLINK_TYPE_INT16_T, 0, 0, offsetof(mavlink_mission_changed_t, start_index) }, \
         { "end_index", NULL, MAVLINK_TYPE_INT16_T, 0, 2, offsetof(mavlink_mission_changed_t, end_index) }, \
         { "origin_sysid", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_mission_changed_t, origin_sysid) }, \
         { "origin_compid", NULL, MAVLINK_TYPE_UINT8_T, 0, 5, offsetof(mavlink_mission_changed_t, origin_compid) }, \
         { "mission_type", NULL, MAVLINK_TYPE_UINT8_T, 0, 6, offsetof(mavlink_mission_changed_t, mission_type) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_MISSION_CHANGED { \
    "MISSION_CHANGED", \
    5, \
    {  { "start_index", NULL, MAVLINK_TYPE_INT16_T, 0, 0, offsetof(mavlink_mission_changed_t, start_index) }, \
         { "end_index", NULL, MAVLINK_TYPE_INT16_T, 0, 2, offsetof(mavlink_mission_changed_t, end_index) }, \
         { "origin_sysid", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_mission_changed_t, origin_sysid) }, \
         { "origin_compid", NULL, MAVLINK_TYPE_UINT8_T, 0, 5, offsetof(mavlink_mission_changed_t, origin_compid) }, \
         { "mission_type", NULL, MAVLINK_TYPE_UINT8_T, 0, 6, offsetof(mavlink_mission_changed_t, mission_type) }, \
         } \
}
#endif

/**
 * @brief Pack a mission_changed message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param start_index  Start index for partial mission change (-1 for all items).
 * @param end_index  End index of a partial mission change. -1 is a synonym for the last mission item (i.e. selects all items from start_index). Ignore field if start_index=-1.
 * @param origin_sysid  System ID of the author of the new mission.
 * @param origin_compid  Compnent ID of the author of the new mission.
 * @param mission_type  Mission type.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_mission_changed_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               int16_t start_index, int16_t end_index, uint8_t origin_sysid, uint8_t origin_compid, uint8_t mission_type)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_MISSION_CHANGED_LEN];
    _mav_put_int16_t(buf, 0, start_index);
    _mav_put_int16_t(buf, 2, end_index);
    _mav_put_uint8_t(buf, 4, origin_sysid);
    _mav_put_uint8_t(buf, 5, origin_compid);
    _mav_put_uint8_t(buf, 6, mission_type);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_MISSION_CHANGED_LEN);
#else
    mavlink_mission_changed_t packet;
    packet.start_index = start_index;
    packet.end_index = end_index;
    packet.origin_sysid = origin_sysid;
    packet.origin_compid = origin_compid;
    packet.mission_type = mission_type;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_MISSION_CHANGED_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_MISSION_CHANGED;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_MISSION_CHANGED_MIN_LEN, MAVLINK_MSG_ID_MISSION_CHANGED_LEN, MAVLINK_MSG_ID_MISSION_CHANGED_CRC);
}

/**
 * @brief Pack a mission_changed message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param start_index  Start index for partial mission change (-1 for all items).
 * @param end_index  End index of a partial mission change. -1 is a synonym for the last mission item (i.e. selects all items from start_index). Ignore field if start_index=-1.
 * @param origin_sysid  System ID of the author of the new mission.
 * @param origin_compid  Compnent ID of the author of the new mission.
 * @param mission_type  Mission type.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_mission_changed_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   int16_t start_index,int16_t end_index,uint8_t origin_sysid,uint8_t origin_compid,uint8_t mission_type)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_MISSION_CHANGED_LEN];
    _mav_put_int16_t(buf, 0, start_index);
    _mav_put_int16_t(buf, 2, end_index);
    _mav_put_uint8_t(buf, 4, origin_sysid);
    _mav_put_uint8_t(buf, 5, origin_compid);
    _mav_put_uint8_t(buf, 6, mission_type);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_MISSION_CHANGED_LEN);
#else
    mavlink_mission_changed_t packet;
    packet.start_index = start_index;
    packet.end_index = end_index;
    packet.origin_sysid = origin_sysid;
    packet.origin_compid = origin_compid;
    packet.mission_type = mission_type;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_MISSION_CHANGED_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_MISSION_CHANGED;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_MISSION_CHANGED_MIN_LEN, MAVLINK_MSG_ID_MISSION_CHANGED_LEN, MAVLINK_MSG_ID_MISSION_CHANGED_CRC);
}

/**
 * @brief Encode a mission_changed struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param mission_changed C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_mission_changed_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_mission_changed_t* mission_changed)
{
    return mavlink_msg_mission_changed_pack(system_id, component_id, msg, mission_changed->start_index, mission_changed->end_index, mission_changed->origin_sysid, mission_changed->origin_compid, mission_changed->mission_type);
}

/**
 * @brief Encode a mission_changed struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param mission_changed C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_mission_changed_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_mission_changed_t* mission_changed)
{
    return mavlink_msg_mission_changed_pack_chan(system_id, component_id, chan, msg, mission_changed->start_index, mission_changed->end_index, mission_changed->origin_sysid, mission_changed->origin_compid, mission_changed->mission_type);
}

/**
 * @brief Send a mission_changed message
 * @param chan MAVLink channel to send the message
 *
 * @param start_index  Start index for partial mission change (-1 for all items).
 * @param end_index  End index of a partial mission change. -1 is a synonym for the last mission item (i.e. selects all items from start_index). Ignore field if start_index=-1.
 * @param origin_sysid  System ID of the author of the new mission.
 * @param origin_compid  Compnent ID of the author of the new mission.
 * @param mission_type  Mission type.
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_mission_changed_send(mavlink_channel_t chan, int16_t start_index, int16_t end_index, uint8_t origin_sysid, uint8_t origin_compid, uint8_t mission_type)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_MISSION_CHANGED_LEN];
    _mav_put_int16_t(buf, 0, start_index);
    _mav_put_int16_t(buf, 2, end_index);
    _mav_put_uint8_t(buf, 4, origin_sysid);
    _mav_put_uint8_t(buf, 5, origin_compid);
    _mav_put_uint8_t(buf, 6, mission_type);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_MISSION_CHANGED, buf, MAVLINK_MSG_ID_MISSION_CHANGED_MIN_LEN, MAVLINK_MSG_ID_MISSION_CHANGED_LEN, MAVLINK_MSG_ID_MISSION_CHANGED_CRC);
#else
    mavlink_mission_changed_t packet;
    packet.start_index = start_index;
    packet.end_index = end_index;
    packet.origin_sysid = origin_sysid;
    packet.origin_compid = origin_compid;
    packet.mission_type = mission_type;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_MISSION_CHANGED, (const char *)&packet, MAVLINK_MSG_ID_MISSION_CHANGED_MIN_LEN, MAVLINK_MSG_ID_MISSION_CHANGED_LEN, MAVLINK_MSG_ID_MISSION_CHANGED_CRC);
#endif
}

/**
 * @brief Send a mission_changed message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_mission_changed_send_struct(mavlink_channel_t chan, const mavlink_mission_changed_t* mission_changed)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_mission_changed_send(chan, mission_changed->start_index, mission_changed->end_index, mission_changed->origin_sysid, mission_changed->origin_compid, mission_changed->mission_type);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_MISSION_CHANGED, (const char *)mission_changed, MAVLINK_MSG_ID_MISSION_CHANGED_MIN_LEN, MAVLINK_MSG_ID_MISSION_CHANGED_LEN, MAVLINK_MSG_ID_MISSION_CHANGED_CRC);
#endif
}

#if MAVLINK_MSG_ID_MISSION_CHANGED_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_mission_changed_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  int16_t start_index, int16_t end_index, uint8_t origin_sysid, uint8_t origin_compid, uint8_t mission_type)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_int16_t(buf, 0, start_index);
    _mav_put_int16_t(buf, 2, end_index);
    _mav_put_uint8_t(buf, 4, origin_sysid);
    _mav_put_uint8_t(buf, 5, origin_compid);
    _mav_put_uint8_t(buf, 6, mission_type);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_MISSION_CHANGED, buf, MAVLINK_MSG_ID_MISSION_CHANGED_MIN_LEN, MAVLINK_MSG_ID_MISSION_CHANGED_LEN, MAVLINK_MSG_ID_MISSION_CHANGED_CRC);
#else
    mavlink_mission_changed_t *packet = (mavlink_mission_changed_t *)msgbuf;
    packet->start_index = start_index;
    packet->end_index = end_index;
    packet->origin_sysid = origin_sysid;
    packet->origin_compid = origin_compid;
    packet->mission_type = mission_type;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_MISSION_CHANGED, (const char *)packet, MAVLINK_MSG_ID_MISSION_CHANGED_MIN_LEN, MAVLINK_MSG_ID_MISSION_CHANGED_LEN, MAVLINK_MSG_ID_MISSION_CHANGED_CRC);
#endif
}
#endif

#endif

// MESSAGE MISSION_CHANGED UNPACKING


/**
 * @brief Get field start_index from mission_changed message
 *
 * @return  Start index for partial mission change (-1 for all items).
 */
static inline int16_t mavlink_msg_mission_changed_get_start_index(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int16_t(msg,  0);
}

/**
 * @brief Get field end_index from mission_changed message
 *
 * @return  End index of a partial mission change. -1 is a synonym for the last mission item (i.e. selects all items from start_index). Ignore field if start_index=-1.
 */
static inline int16_t mavlink_msg_mission_changed_get_end_index(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int16_t(msg,  2);
}

/**
 * @brief Get field origin_sysid from mission_changed message
 *
 * @return  System ID of the author of the new mission.
 */
static inline uint8_t mavlink_msg_mission_changed_get_origin_sysid(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  4);
}

/**
 * @brief Get field origin_compid from mission_changed message
 *
 * @return  Compnent ID of the author of the new mission.
 */
static inline uint8_t mavlink_msg_mission_changed_get_origin_compid(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  5);
}

/**
 * @brief Get field mission_type from mission_changed message
 *
 * @return  Mission type.
 */
static inline uint8_t mavlink_msg_mission_changed_get_mission_type(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  6);
}

/**
 * @brief Decode a mission_changed message into a struct
 *
 * @param msg The message to decode
 * @param mission_changed C-struct to decode the message contents into
 */
static inline void mavlink_msg_mission_changed_decode(const mavlink_message_t* msg, mavlink_mission_changed_t* mission_changed)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mission_changed->start_index = mavlink_msg_mission_changed_get_start_index(msg);
    mission_changed->end_index = mavlink_msg_mission_changed_get_end_index(msg);
    mission_changed->origin_sysid = mavlink_msg_mission_changed_get_origin_sysid(msg);
    mission_changed->origin_compid = mavlink_msg_mission_changed_get_origin_compid(msg);
    mission_changed->mission_type = mavlink_msg_mission_changed_get_mission_type(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_MISSION_CHANGED_LEN? msg->len : MAVLINK_MSG_ID_MISSION_CHANGED_LEN;
        memset(mission_changed, 0, MAVLINK_MSG_ID_MISSION_CHANGED_LEN);
    memcpy(mission_changed, _MAV_PAYLOAD(msg), len);
#endif
}

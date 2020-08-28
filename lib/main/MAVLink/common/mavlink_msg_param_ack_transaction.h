#pragma once
// MESSAGE PARAM_ACK_TRANSACTION PACKING

#define MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION 19


typedef struct __mavlink_param_ack_transaction_t {
 float param_value; /*<  Parameter value (new value if PARAM_ACCEPTED, current value otherwise)*/
 uint8_t target_system; /*<  Id of system that sent PARAM_SET message.*/
 uint8_t target_component; /*<  Id of system that sent PARAM_SET message.*/
 char param_id[16]; /*<  Parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string*/
 uint8_t param_type; /*<  Parameter type.*/
 uint8_t param_result; /*<  Result code.*/
} mavlink_param_ack_transaction_t;

#define MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN 24
#define MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_MIN_LEN 24
#define MAVLINK_MSG_ID_19_LEN 24
#define MAVLINK_MSG_ID_19_MIN_LEN 24

#define MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_CRC 137
#define MAVLINK_MSG_ID_19_CRC 137

#define MAVLINK_MSG_PARAM_ACK_TRANSACTION_FIELD_PARAM_ID_LEN 16

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_PARAM_ACK_TRANSACTION { \
    19, \
    "PARAM_ACK_TRANSACTION", \
    6, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_param_ack_transaction_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 5, offsetof(mavlink_param_ack_transaction_t, target_component) }, \
         { "param_id", NULL, MAVLINK_TYPE_CHAR, 16, 6, offsetof(mavlink_param_ack_transaction_t, param_id) }, \
         { "param_value", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_param_ack_transaction_t, param_value) }, \
         { "param_type", NULL, MAVLINK_TYPE_UINT8_T, 0, 22, offsetof(mavlink_param_ack_transaction_t, param_type) }, \
         { "param_result", NULL, MAVLINK_TYPE_UINT8_T, 0, 23, offsetof(mavlink_param_ack_transaction_t, param_result) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_PARAM_ACK_TRANSACTION { \
    "PARAM_ACK_TRANSACTION", \
    6, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_param_ack_transaction_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 5, offsetof(mavlink_param_ack_transaction_t, target_component) }, \
         { "param_id", NULL, MAVLINK_TYPE_CHAR, 16, 6, offsetof(mavlink_param_ack_transaction_t, param_id) }, \
         { "param_value", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_param_ack_transaction_t, param_value) }, \
         { "param_type", NULL, MAVLINK_TYPE_UINT8_T, 0, 22, offsetof(mavlink_param_ack_transaction_t, param_type) }, \
         { "param_result", NULL, MAVLINK_TYPE_UINT8_T, 0, 23, offsetof(mavlink_param_ack_transaction_t, param_result) }, \
         } \
}
#endif

/**
 * @brief Pack a param_ack_transaction message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target_system  Id of system that sent PARAM_SET message.
 * @param target_component  Id of system that sent PARAM_SET message.
 * @param param_id  Parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string
 * @param param_value  Parameter value (new value if PARAM_ACCEPTED, current value otherwise)
 * @param param_type  Parameter type.
 * @param param_result  Result code.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_param_ack_transaction_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t target_system, uint8_t target_component, const char *param_id, float param_value, uint8_t param_type, uint8_t param_result)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN];
    _mav_put_float(buf, 0, param_value);
    _mav_put_uint8_t(buf, 4, target_system);
    _mav_put_uint8_t(buf, 5, target_component);
    _mav_put_uint8_t(buf, 22, param_type);
    _mav_put_uint8_t(buf, 23, param_result);
    _mav_put_char_array(buf, 6, param_id, 16);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN);
#else
    mavlink_param_ack_transaction_t packet;
    packet.param_value = param_value;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.param_type = param_type;
    packet.param_result = param_result;
    mav_array_memcpy(packet.param_id, param_id, sizeof(char)*16);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_MIN_LEN, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_CRC);
}

/**
 * @brief Pack a param_ack_transaction message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param target_system  Id of system that sent PARAM_SET message.
 * @param target_component  Id of system that sent PARAM_SET message.
 * @param param_id  Parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string
 * @param param_value  Parameter value (new value if PARAM_ACCEPTED, current value otherwise)
 * @param param_type  Parameter type.
 * @param param_result  Result code.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_param_ack_transaction_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t target_system,uint8_t target_component,const char *param_id,float param_value,uint8_t param_type,uint8_t param_result)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN];
    _mav_put_float(buf, 0, param_value);
    _mav_put_uint8_t(buf, 4, target_system);
    _mav_put_uint8_t(buf, 5, target_component);
    _mav_put_uint8_t(buf, 22, param_type);
    _mav_put_uint8_t(buf, 23, param_result);
    _mav_put_char_array(buf, 6, param_id, 16);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN);
#else
    mavlink_param_ack_transaction_t packet;
    packet.param_value = param_value;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.param_type = param_type;
    packet.param_result = param_result;
    mav_array_memcpy(packet.param_id, param_id, sizeof(char)*16);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_MIN_LEN, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_CRC);
}

/**
 * @brief Encode a param_ack_transaction struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param param_ack_transaction C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_param_ack_transaction_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_param_ack_transaction_t* param_ack_transaction)
{
    return mavlink_msg_param_ack_transaction_pack(system_id, component_id, msg, param_ack_transaction->target_system, param_ack_transaction->target_component, param_ack_transaction->param_id, param_ack_transaction->param_value, param_ack_transaction->param_type, param_ack_transaction->param_result);
}

/**
 * @brief Encode a param_ack_transaction struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param param_ack_transaction C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_param_ack_transaction_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_param_ack_transaction_t* param_ack_transaction)
{
    return mavlink_msg_param_ack_transaction_pack_chan(system_id, component_id, chan, msg, param_ack_transaction->target_system, param_ack_transaction->target_component, param_ack_transaction->param_id, param_ack_transaction->param_value, param_ack_transaction->param_type, param_ack_transaction->param_result);
}

/**
 * @brief Send a param_ack_transaction message
 * @param chan MAVLink channel to send the message
 *
 * @param target_system  Id of system that sent PARAM_SET message.
 * @param target_component  Id of system that sent PARAM_SET message.
 * @param param_id  Parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string
 * @param param_value  Parameter value (new value if PARAM_ACCEPTED, current value otherwise)
 * @param param_type  Parameter type.
 * @param param_result  Result code.
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_param_ack_transaction_send(mavlink_channel_t chan, uint8_t target_system, uint8_t target_component, const char *param_id, float param_value, uint8_t param_type, uint8_t param_result)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN];
    _mav_put_float(buf, 0, param_value);
    _mav_put_uint8_t(buf, 4, target_system);
    _mav_put_uint8_t(buf, 5, target_component);
    _mav_put_uint8_t(buf, 22, param_type);
    _mav_put_uint8_t(buf, 23, param_result);
    _mav_put_char_array(buf, 6, param_id, 16);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION, buf, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_MIN_LEN, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_CRC);
#else
    mavlink_param_ack_transaction_t packet;
    packet.param_value = param_value;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.param_type = param_type;
    packet.param_result = param_result;
    mav_array_memcpy(packet.param_id, param_id, sizeof(char)*16);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION, (const char *)&packet, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_MIN_LEN, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_CRC);
#endif
}

/**
 * @brief Send a param_ack_transaction message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_param_ack_transaction_send_struct(mavlink_channel_t chan, const mavlink_param_ack_transaction_t* param_ack_transaction)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_param_ack_transaction_send(chan, param_ack_transaction->target_system, param_ack_transaction->target_component, param_ack_transaction->param_id, param_ack_transaction->param_value, param_ack_transaction->param_type, param_ack_transaction->param_result);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION, (const char *)param_ack_transaction, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_MIN_LEN, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_CRC);
#endif
}

#if MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_param_ack_transaction_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t target_system, uint8_t target_component, const char *param_id, float param_value, uint8_t param_type, uint8_t param_result)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_float(buf, 0, param_value);
    _mav_put_uint8_t(buf, 4, target_system);
    _mav_put_uint8_t(buf, 5, target_component);
    _mav_put_uint8_t(buf, 22, param_type);
    _mav_put_uint8_t(buf, 23, param_result);
    _mav_put_char_array(buf, 6, param_id, 16);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION, buf, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_MIN_LEN, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_CRC);
#else
    mavlink_param_ack_transaction_t *packet = (mavlink_param_ack_transaction_t *)msgbuf;
    packet->param_value = param_value;
    packet->target_system = target_system;
    packet->target_component = target_component;
    packet->param_type = param_type;
    packet->param_result = param_result;
    mav_array_memcpy(packet->param_id, param_id, sizeof(char)*16);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION, (const char *)packet, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_MIN_LEN, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_CRC);
#endif
}
#endif

#endif

// MESSAGE PARAM_ACK_TRANSACTION UNPACKING


/**
 * @brief Get field target_system from param_ack_transaction message
 *
 * @return  Id of system that sent PARAM_SET message.
 */
static inline uint8_t mavlink_msg_param_ack_transaction_get_target_system(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  4);
}

/**
 * @brief Get field target_component from param_ack_transaction message
 *
 * @return  Id of system that sent PARAM_SET message.
 */
static inline uint8_t mavlink_msg_param_ack_transaction_get_target_component(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  5);
}

/**
 * @brief Get field param_id from param_ack_transaction message
 *
 * @return  Parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string
 */
static inline uint16_t mavlink_msg_param_ack_transaction_get_param_id(const mavlink_message_t* msg, char *param_id)
{
    return _MAV_RETURN_char_array(msg, param_id, 16,  6);
}

/**
 * @brief Get field param_value from param_ack_transaction message
 *
 * @return  Parameter value (new value if PARAM_ACCEPTED, current value otherwise)
 */
static inline float mavlink_msg_param_ack_transaction_get_param_value(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field param_type from param_ack_transaction message
 *
 * @return  Parameter type.
 */
static inline uint8_t mavlink_msg_param_ack_transaction_get_param_type(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  22);
}

/**
 * @brief Get field param_result from param_ack_transaction message
 *
 * @return  Result code.
 */
static inline uint8_t mavlink_msg_param_ack_transaction_get_param_result(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  23);
}

/**
 * @brief Decode a param_ack_transaction message into a struct
 *
 * @param msg The message to decode
 * @param param_ack_transaction C-struct to decode the message contents into
 */
static inline void mavlink_msg_param_ack_transaction_decode(const mavlink_message_t* msg, mavlink_param_ack_transaction_t* param_ack_transaction)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    param_ack_transaction->param_value = mavlink_msg_param_ack_transaction_get_param_value(msg);
    param_ack_transaction->target_system = mavlink_msg_param_ack_transaction_get_target_system(msg);
    param_ack_transaction->target_component = mavlink_msg_param_ack_transaction_get_target_component(msg);
    mavlink_msg_param_ack_transaction_get_param_id(msg, param_ack_transaction->param_id);
    param_ack_transaction->param_type = mavlink_msg_param_ack_transaction_get_param_type(msg);
    param_ack_transaction->param_result = mavlink_msg_param_ack_transaction_get_param_result(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN? msg->len : MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN;
        memset(param_ack_transaction, 0, MAVLINK_MSG_ID_PARAM_ACK_TRANSACTION_LEN);
    memcpy(param_ack_transaction, _MAV_PAYLOAD(msg), len);
#endif
}

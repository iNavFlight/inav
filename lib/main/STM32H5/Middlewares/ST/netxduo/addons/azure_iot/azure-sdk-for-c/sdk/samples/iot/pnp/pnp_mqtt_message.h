// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef PNP_MQTT_MESSAGE_H
#define PNP_MQTT_MESSAGE_H

#include <stddef.h>

#include <azure/az_core.h>

#ifdef _MSC_VER
#pragma warning(push)
// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable : 4201)
#endif
#include <MQTTClient.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define MQTT_TIMEOUT_RECEIVE_MAX_MESSAGE_COUNT 100
#define MQTT_TIMEOUT_RECEIVE_MS (8 * 1000)
#define MQTT_TIMEOUT_DISCONNECT_MS (10 * 1000)

typedef struct
{
  char* topic;
  size_t* out_topic_length;
  size_t topic_length;
  az_span out_payload;
  az_span payload;
} pnp_mqtt_message;

/**
 * @brief Initialize a #pnp_mqtt_message which holds info for publishing an mqtt message.
 *
 * @param[out] out_mqtt_message A pointer to a #pnp_mqtt_message instance to initialize.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK #pnp_mqtt_message is initialied successfully.
 * @retval #AZ_ERROR_ARG The pointer to the #pnp_mqtt_message instance is NULL.
 */
az_result pnp_mqtt_message_init(pnp_mqtt_message* out_mqtt_message);

/**
 * @brief Creates a request id #az_span for use in sending property messages. Value increments on
 * each call.  Capable of holding a 10 digit number (base 10).
 *
 * @return An #az_span containing the request id.
 */
az_span pnp_mqtt_get_request_id(void);

/**
 * @brief Wrapper to publish an MQTT message.
 *
 * @param[in] mqtt_client Pointer to connected MQTTClient handle to use to send message.
 * @param[in] topic MQTT topic string to send message on.
 * @param[in] payload An #az_span containing the payload.
 * @param[in] qos MQTT QOS setting.
 */
void publish_mqtt_message(MQTTClient mqtt_client, char const* topic, az_span payload, int qos);

#endif // PNP_MQTT_MESSAGE_H

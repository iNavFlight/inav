// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <azure/az_core.h>

#include <iot_sample_common.h>

#include "pnp_mqtt_message.h"

static char publish_topic_buffer[128];
static char publish_payload_buffer[512];
static uint32_t request_id_int;
static char request_id_buffer[16];

az_result pnp_mqtt_message_init(pnp_mqtt_message* out_mqtt_message)
{
  if (out_mqtt_message == NULL)
  {
    return AZ_ERROR_ARG;
  }

  out_mqtt_message->topic = publish_topic_buffer;
  out_mqtt_message->topic_length = sizeof(publish_topic_buffer);
  out_mqtt_message->out_topic_length = 0;
  out_mqtt_message->payload = AZ_SPAN_FROM_BUFFER(publish_payload_buffer);
  out_mqtt_message->out_payload = out_mqtt_message->payload;

  return AZ_OK;
}

az_span pnp_mqtt_get_request_id(void)
{
  az_span remainder;
  az_span out_span = az_span_create((uint8_t*)request_id_buffer, sizeof(request_id_buffer));

  // Note that if left to run for a long time, this will overflow and reset back to 0.
  az_result rc = az_span_u32toa(out_span, request_id_int++, &remainder);
  if (az_result_failed(rc))
  {
    IOT_SAMPLE_LOG_ERROR("Failed to get request id: az_result return code 0x%08x.", rc);
    exit(rc);
  }

  return az_span_slice(out_span, 0, az_span_size(out_span) - az_span_size(remainder));
}

void publish_mqtt_message(MQTTClient mqtt_client, char const* topic, az_span payload, int qos)
{
  int rc = MQTTClient_publish(
      mqtt_client, topic, az_span_size(payload), az_span_ptr(payload), qos, 0, NULL);

  if (rc != MQTTCLIENT_SUCCESS)
  {
    IOT_SAMPLE_LOG_ERROR("Failed to publish message: MQTTClient return code %d", rc);
    exit(rc);
  }
}

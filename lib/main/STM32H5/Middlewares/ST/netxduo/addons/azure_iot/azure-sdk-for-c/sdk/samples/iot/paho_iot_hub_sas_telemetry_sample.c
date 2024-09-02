// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#pragma warning(push)
// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable : 4201)
#endif
#include <MQTTClient.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <azure/az_core.h>
#include <azure/az_iot.h>

#include "iot_sample_common.h"

#define SAMPLE_TYPE PAHO_IOT_HUB
#define SAMPLE_NAME PAHO_IOT_HUB_SAS_TELEMETRY_SAMPLE

#define TELEMETRY_SEND_INTERVAL_SEC 1
#define MAX_TELEMETRY_MESSAGE_COUNT 5
#define MQTT_TIMEOUT_DISCONNECT_MS (10 * 1000)

static iot_sample_environment_variables env_vars;
static az_iot_hub_client hub_client;
static MQTTClient mqtt_client;
static char mqtt_client_username_buffer[128];

// Generate SAS key variables
static char sas_signature_buffer[128];
static char sas_base64_encoded_signed_signature_buffer[128];
static char mqtt_password_buffer[256];

// Functions
static void create_and_configure_mqtt_client(void);
static void connect_mqtt_client_to_iot_hub(void);
static void send_telemetry_messages_to_iot_hub(void);
static void disconnect_mqtt_client_from_iot_hub(void);

static void generate_sas_key(void);

/*
 * This sample sends five telemetry messages to the Azure IoT Hub. SAS certification is used.
 */
int main(void)
{
  create_and_configure_mqtt_client();
  IOT_SAMPLE_LOG_SUCCESS("Client created and configured.");

  connect_mqtt_client_to_iot_hub();
  IOT_SAMPLE_LOG_SUCCESS("Client connected to IoT Hub.\n");

  send_telemetry_messages_to_iot_hub();
  IOT_SAMPLE_LOG_SUCCESS("Client sent telemetry messages to IoT Hub.");

  disconnect_mqtt_client_from_iot_hub();
  IOT_SAMPLE_LOG_SUCCESS("Client disconnected from IoT Hub.");

  return 0;
}

static void create_and_configure_mqtt_client(void)
{
  int rc;

  // Reads in environment variables set by user for purposes of running sample.
  iot_sample_read_environment_variables(SAMPLE_TYPE, SAMPLE_NAME, &env_vars);

  // Build an MQTT endpoint c-string.
  char mqtt_endpoint_buffer[128];
  iot_sample_create_mqtt_endpoint(
      SAMPLE_TYPE, &env_vars, mqtt_endpoint_buffer, sizeof(mqtt_endpoint_buffer));

  // Initialize the hub client with the default connection options.
  rc = az_iot_hub_client_init(&hub_client, env_vars.hub_hostname, env_vars.hub_device_id, NULL);
  if (az_result_failed(rc))
  {
    IOT_SAMPLE_LOG_ERROR("Failed to initialize hub client: az_result return code 0x%08x.", rc);
    exit(rc);
  }

  // Get the MQTT client id used for the MQTT connection.
  char mqtt_client_id_buffer[128];
  rc = az_iot_hub_client_get_client_id(
      &hub_client, mqtt_client_id_buffer, sizeof(mqtt_client_id_buffer), NULL);
  if (az_result_failed(rc))
  {
    IOT_SAMPLE_LOG_ERROR("Failed to get MQTT client id: az_result return code 0x%08x.", rc);
    exit(rc);
  }

  // Create the Paho MQTT client.
  rc = MQTTClient_create(
      &mqtt_client, mqtt_endpoint_buffer, mqtt_client_id_buffer, MQTTCLIENT_PERSISTENCE_NONE, NULL);
  if (rc != MQTTCLIENT_SUCCESS)
  {
    IOT_SAMPLE_LOG_ERROR("Failed to create MQTT client: MQTTClient return code %d.", rc);
    exit(rc);
  }

  generate_sas_key();
  IOT_SAMPLE_LOG_SUCCESS("Client generated SAS Key.");
}

static void connect_mqtt_client_to_iot_hub(void)
{
  int rc;

  // Get the MQTT client username.
  rc = az_iot_hub_client_get_user_name(
      &hub_client, mqtt_client_username_buffer, sizeof(mqtt_client_username_buffer), NULL);
  if (az_result_failed(rc))
  {
    IOT_SAMPLE_LOG_ERROR("Failed to get MQTT client username: az_result return code 0x%08x.", rc);
    exit(rc);
  }

  IOT_SAMPLE_LOG("MQTT client username: %s\n", mqtt_client_username_buffer);

  // Set MQTT connection options.
  MQTTClient_connectOptions mqtt_connect_options = MQTTClient_connectOptions_initializer;
  mqtt_connect_options.username = mqtt_client_username_buffer;
  mqtt_connect_options.password = mqtt_password_buffer;
  mqtt_connect_options.cleansession = false; // Set to false so can receive any pending messages.
  mqtt_connect_options.keepAliveInterval = AZ_IOT_DEFAULT_MQTT_CONNECT_KEEPALIVE_SECONDS;

  MQTTClient_SSLOptions mqtt_ssl_options = MQTTClient_SSLOptions_initializer;
  mqtt_ssl_options.verify = 1;
  mqtt_ssl_options.enableServerCertAuth = 1;
  if (az_span_size(env_vars.x509_trust_pem_file_path) != 0) // Is only set if required by OS.
  {
    mqtt_ssl_options.trustStore = (char*)az_span_ptr(env_vars.x509_trust_pem_file_path);
  }
  mqtt_connect_options.ssl = &mqtt_ssl_options;

  // Connect MQTT client to the Azure IoT Hub.
  rc = MQTTClient_connect(mqtt_client, &mqtt_connect_options);
  if (rc != MQTTCLIENT_SUCCESS)
  {
    IOT_SAMPLE_LOG_ERROR(
        "Failed to connect: MQTTClient return code %d.\n"
        "If on Windows, confirm the AZ_IOT_DEVICE_X509_TRUST_PEM_FILE_PATH environment variable is "
        "set correctly.",
        rc);
    exit(rc);
  }
}

static void send_telemetry_messages_to_iot_hub(void)
{
  int rc;

  // Get the Telemetry topic to publish the telemetry messages.
  char telemetry_topic_buffer[128];
  rc = az_iot_hub_client_telemetry_get_publish_topic(
      &hub_client, NULL, telemetry_topic_buffer, sizeof(telemetry_topic_buffer), NULL);
  if (az_result_failed(rc))
  {
    IOT_SAMPLE_LOG_ERROR("Failed to get the Telemetry topic: az_result return code 0x%08x.", rc);
    exit(rc);
  }

  char const* telemetry_message_payloads[MAX_TELEMETRY_MESSAGE_COUNT] = {
    "{\"message_number\":1}", "{\"message_number\":2}", "{\"message_number\":3}",
    "{\"message_number\":4}", "{\"message_number\":5}",
  };

  // Publish # of telemetry messages.
  for (uint8_t message_count = 0; message_count < MAX_TELEMETRY_MESSAGE_COUNT; message_count++)
  {
    rc = MQTTClient_publish(
        mqtt_client,
        telemetry_topic_buffer,
        (int)strlen(telemetry_message_payloads[message_count]),
        telemetry_message_payloads[message_count],
        IOT_SAMPLE_MQTT_PUBLISH_QOS,
        0,
        NULL);
    if (rc != MQTTCLIENT_SUCCESS)
    {
      IOT_SAMPLE_LOG_ERROR(
          "Failed to publish Telemetry message #%d: MQTTClient return code %d.",
          message_count + 1,
          rc);
      exit(rc);
    }
    IOT_SAMPLE_LOG_SUCCESS(
        "Message #%d: Client published the Telemetry message.", message_count + 1);
    IOT_SAMPLE_LOG("Payload: %s\n", telemetry_message_payloads[message_count]);

    iot_sample_sleep_for_seconds(TELEMETRY_SEND_INTERVAL_SEC);
  }
}

static void disconnect_mqtt_client_from_iot_hub(void)
{
  int rc = MQTTClient_disconnect(mqtt_client, MQTT_TIMEOUT_DISCONNECT_MS);
  if (rc != MQTTCLIENT_SUCCESS)
  {
    IOT_SAMPLE_LOG_ERROR("Failed to disconnect MQTT client: MQTTClient return code %d.", rc);
    exit(rc);
  }

  MQTTClient_destroy(&mqtt_client);
}

static void generate_sas_key(void)
{
  az_result rc;

  // Create the POSIX expiration time from input minutes.
  uint64_t sas_duration
      = iot_sample_get_epoch_expiration_time_from_minutes(env_vars.sas_key_duration_minutes);

  // Get the signature that will later be signed with the decoded key.
  az_span sas_signature = AZ_SPAN_FROM_BUFFER(sas_signature_buffer);
  rc = az_iot_hub_client_sas_get_signature(
      &hub_client, sas_duration, sas_signature, &sas_signature);
  if (az_result_failed(rc))
  {
    IOT_SAMPLE_LOG_ERROR(
        "Could not get the signature for SAS key: az_result return code 0x%08x.", rc);
    exit(rc);
  }

  // Generate the encoded, signed signature (b64 encoded, HMAC-SHA256 signing).
  az_span sas_base64_encoded_signed_signature
      = AZ_SPAN_FROM_BUFFER(sas_base64_encoded_signed_signature_buffer);
  iot_sample_generate_sas_base64_encoded_signed_signature(
      env_vars.hub_sas_key,
      sas_signature,
      sas_base64_encoded_signed_signature,
      &sas_base64_encoded_signed_signature);

  // Get the resulting MQTT password, passing the base64 encoded, HMAC signed bytes.
  size_t mqtt_password_length;
  rc = az_iot_hub_client_sas_get_password(
      &hub_client,
      sas_duration,
      sas_base64_encoded_signed_signature,
      AZ_SPAN_EMPTY,
      mqtt_password_buffer,
      sizeof(mqtt_password_buffer),
      &mqtt_password_length);
  if (az_result_failed(rc))
  {
    IOT_SAMPLE_LOG_ERROR("Could not get the password: az_result return code 0x%08x.", rc);
    exit(rc);
  }
}

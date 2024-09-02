// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/*
 * This sample connects an IoT Plug and Play enabled device.
 *
 * IoT Plug and Play requires the device to advertise its capabilities in a device model. This
 * sample's model is available in
 * https://github.com/Azure/opendigitaltwins-dtdl/blob/master/DTDL/v2/samples/Thermostat.json. See
 * the sample README for more information on this model. For more information about IoT Plug and
 * Play, see https://aka.ms/iotpnp.
 *
 * The sample loops listening for incoming commands and property updates and periodically (every
 * MQTT_TIMEOUT_RECEIVE_MS milliseconds) will send a telemetry event.  After
 * MQTT_TIMEOUT_RECEIVE_MAX_MESSAGE_COUNT loops without any service initiated operations, the sample
 * will exit.
 *
 * The Device Provisioning Service is used for authentication.
 */

#ifdef _MSC_VER
#pragma warning(push)
// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable : 4201)
#endif
#include <MQTTClient.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <azure/core/az_result.h>
#include <azure/core/az_span.h>
#include <azure/iot/az_iot_hub_client.h>
#include <azure/iot/az_iot_provisioning_client.h>

#include "iot_sample_common.h"
#include "paho_iot_pnp_sample_common.h"

#define SAMPLE_TYPE PAHO_IOT_PROVISIONING
#define SAMPLE_NAME PAHO_IOT_PNP_WITH_PROVISIONING_SAMPLE

#define QUERY_TOPIC_BUFFER_LENGTH 256
#define REGISTER_TOPIC_BUFFER_LENGTH 128
#define PROVISIONING_ENDPOINT_BUFFER_LENGTH 256
#define MQTT_PAYLOAD_BUFFER_LENGTH 256

// The model this device implements
static az_span const model_id = AZ_SPAN_LITERAL_FROM_STR("dtmi:com:example:Thermostat;1");
static az_span custom_registration_payload_property
    = AZ_SPAN_LITERAL_FROM_STR("{\"modelId\":\"dtmi:com:example:Thermostat;1\"}");

static iot_sample_environment_variables env_vars;
static az_iot_provisioning_client provisioning_client;
static char iot_hub_endpoint_buffer[128];
static char iot_hub_device_id_buffer[128];
static az_span device_iot_hub_endpoint;
static az_span device_id;
static char mqtt_client_username_buffer[256];

//
// Provisioning Functions
//
static void create_and_configure_mqtt_client_for_provisioning(void);
static void connect_mqtt_client_to_provisioning_service(void);
static void subscribe_mqtt_client_to_provisioning_service_topics(void);
static void register_device_with_provisioning_service(void);
static void receive_device_registration_status_message(void);
static void disconnect_mqtt_client_from_provisioning_service(void);
static void parse_device_registration_status_message(
    char* topic,
    int topic_len,
    MQTTClient_message const* message,
    az_iot_provisioning_client_register_response* out_register_response);
static void handle_device_registration_status_message(
    az_iot_provisioning_client_register_response const* register_response,
    bool* ref_is_operation_complete);
static void send_operation_query_message(
    az_iot_provisioning_client_register_response const* response);

//
// Hub Functions
//
static void create_and_configure_mqtt_client_for_iot_hub(void);
static void connect_mqtt_client_to_iot_hub(void);
static void disconnect_mqtt_client_from_iot_hub(void);

int main(void)
{
  // The initial functions connect to the Device Provisioning Service (DPS) to
  // retrieve the device's provisioning information (IoT Hub and device name).
  create_and_configure_mqtt_client_for_provisioning();
  IOT_SAMPLE_LOG_SUCCESS("Client created and configured.");

  connect_mqtt_client_to_provisioning_service();
  IOT_SAMPLE_LOG_SUCCESS("Client connected to provisioning service.");

  subscribe_mqtt_client_to_provisioning_service_topics();
  IOT_SAMPLE_LOG_SUCCESS("Client subscribed to provisioning service topics.");

  register_device_with_provisioning_service();
  IOT_SAMPLE_LOG_SUCCESS("Client registering with provisioning service.");

  receive_device_registration_status_message();
  IOT_SAMPLE_LOG_SUCCESS("Client received registration status message.");

  disconnect_mqtt_client_from_provisioning_service();
  IOT_SAMPLE_LOG_SUCCESS("Client disconnected from provisioning service.");

  // Now that we have been provisioned by DPS, create an MQTT connection to
  // Azure IoT Hub.
  create_and_configure_mqtt_client_for_iot_hub();
  IOT_SAMPLE_LOG_SUCCESS("Client created and configured.");

  connect_mqtt_client_to_iot_hub();
  IOT_SAMPLE_LOG_SUCCESS("Client connected to IoT Hub.");

  // The device's main loop including primary Plug and Play interaction is
  // in paho_iot_pnp_sample_device_implement.
  paho_iot_pnp_sample_device_implement();
  IOT_SAMPLE_LOG_SUCCESS("Completed sample device implementation run.");

  // Disconnect the MQTT connection.
  disconnect_mqtt_client_from_iot_hub();
  IOT_SAMPLE_LOG_SUCCESS("Client disconnected from IoT Hub.");

  return 0;
}

// create_and_configure_mqtt_client_for_provisioning reads configuration variables from the
// environment, makes calls to the Azure SDK for C for initial setup, and initiates an MQTT client
// from Paho for a call to Device Provisioning Service.
static void create_and_configure_mqtt_client_for_provisioning(void)
{
  // Reads in environment variables set by user for purposes of running sample.
  iot_sample_read_environment_variables(SAMPLE_TYPE, SAMPLE_NAME, &env_vars);

  // Build an MQTT endpoint c-string.
  char mqtt_endpoint_buffer[PROVISIONING_ENDPOINT_BUFFER_LENGTH];
  iot_sample_create_mqtt_endpoint(
      SAMPLE_TYPE, &env_vars, mqtt_endpoint_buffer, sizeof(mqtt_endpoint_buffer));

  // Initialize the provisioning client with the provisioning global endpoint and the default
  // connection options.
  int rc = az_iot_provisioning_client_init(
      &provisioning_client,
      az_span_create_from_str(mqtt_endpoint_buffer),
      env_vars.provisioning_id_scope,
      env_vars.provisioning_registration_id,
      NULL);
  if (az_result_failed(rc))
  {
    IOT_SAMPLE_LOG_ERROR(
        "Failed to initialize provisioning client: az_result return code 0x%08x.", rc);
    exit(rc);
  }

  // Get the MQTT client id used for the MQTT connection.
  char mqtt_client_id_buffer[CLIENT_ID_BUFFER_LENGTH];
  rc = az_iot_provisioning_client_get_client_id(
      &provisioning_client, mqtt_client_id_buffer, sizeof(mqtt_client_id_buffer), NULL);
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
}

// connect_mqtt_client_to_provisioning_service sets up basic security and MQTT topics and then
// initiates an MQTT connection to the Device Provisioning Service.
static void connect_mqtt_client_to_provisioning_service(void)
{
  // Get the MQTT client username.
  int rc = az_iot_provisioning_client_get_user_name(
      &provisioning_client, mqtt_client_username_buffer, sizeof(mqtt_client_username_buffer), NULL);
  if (az_result_failed(rc))
  {
    IOT_SAMPLE_LOG_ERROR("Failed to get MQTT client username: az_result return code 0x%08x.", rc);
    exit(rc);
  }

  // Set MQTT connection options.
  MQTTClient_connectOptions mqtt_connect_options = MQTTClient_connectOptions_initializer;
  mqtt_connect_options.username = mqtt_client_username_buffer;
  mqtt_connect_options.password = NULL; // This sample uses x509 authentication.
  mqtt_connect_options.cleansession = false; // Set to false so can receive any pending messages.
  mqtt_connect_options.keepAliveInterval = AZ_IOT_DEFAULT_MQTT_CONNECT_KEEPALIVE_SECONDS;

  MQTTClient_SSLOptions mqtt_ssl_options = MQTTClient_SSLOptions_initializer;
  mqtt_ssl_options.keyStore = (char*)az_span_ptr(env_vars.x509_cert_pem_file_path);
  if (az_span_size(env_vars.x509_trust_pem_file_path) != 0) // Is only set if required by OS.
  {
    mqtt_ssl_options.trustStore = (char*)az_span_ptr(env_vars.x509_trust_pem_file_path);
  }
  mqtt_connect_options.ssl = &mqtt_ssl_options;

  // Connect MQTT client to the Azure IoT Device Provisioning Service.
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

// subscribe_mqtt_client_to_provisioning_service_topics subscribes to the MQTT topic that
// responses to the device's requests will be sent on.
static void subscribe_mqtt_client_to_provisioning_service_topics(void)
{
  // Messages received on the Register topic will be registration responses from the server.
  int rc
      = MQTTClient_subscribe(mqtt_client, AZ_IOT_PROVISIONING_CLIENT_REGISTER_SUBSCRIBE_TOPIC, 1);
  if (rc != MQTTCLIENT_SUCCESS)
  {
    IOT_SAMPLE_LOG_ERROR(
        "Failed to subscribe to the Register topic: MQTTClient return code %d.", rc);
    exit(rc);
  }
}

// register_device_with_provisioning_service uses Paho to PUBLISH a message to the
// Device Provisioning Service to register this device.  The service will respond
// asynchronously on the topic registered in subscribe_mqtt_client_to_provisioning_service_topics().
static void register_device_with_provisioning_service(void)
{
  // Get the Register topic to publish the register request.
  char register_topic_buffer[REGISTER_TOPIC_BUFFER_LENGTH];
  int rc = az_iot_provisioning_client_register_get_publish_topic(
      &provisioning_client, register_topic_buffer, sizeof(register_topic_buffer), NULL);
  if (az_result_failed(rc))
  {
    IOT_SAMPLE_LOG_ERROR("Failed to get the Register topic: az_result return code 0x%08x.", rc);
    exit(rc);
  }

  // Devices registering a ModelId while using Device Provisioning Service must specify
  // their ModelId in an MQTT payload sent during registration.
  uint8_t mqtt_payload[MQTT_PAYLOAD_BUFFER_LENGTH];
  size_t mqtt_payload_length;

  rc = az_iot_provisioning_client_get_request_payload(
      &provisioning_client,
      custom_registration_payload_property,
      NULL,
      mqtt_payload,
      sizeof(mqtt_payload),
      &mqtt_payload_length);
  if (az_result_failed(rc))
  {
    IOT_SAMPLE_LOG_ERROR(
        "Failed to initialize provisioning client: az_result return code 0x%08x.", rc);
    exit(rc);
  }

  // Set MQTT message options.
  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  pubmsg.payload = mqtt_payload;
  pubmsg.payloadlen = (int)mqtt_payload_length;
  pubmsg.qos = 1;
  pubmsg.retained = 0;

  // Publish the register request.
  rc = MQTTClient_publishMessage(mqtt_client, register_topic_buffer, &pubmsg, NULL);
  if (rc != MQTTCLIENT_SUCCESS)
  {
    IOT_SAMPLE_LOG_ERROR("Failed to publish Register request: MQTTClient return code %d.", rc);
    exit(rc);
  }
}

// receive_device_registration_status_message polls waiting for a response to our registration
// message and parses the response when it arrives.
static void receive_device_registration_status_message(void)
{
  char* topic = NULL;
  int topic_len = 0;
  MQTTClient_message* message = NULL;
  bool is_operation_complete = false;

  // Continue to parse incoming responses from the provisioning service until the device has been
  // successfully provisioned or an error occurs.
  do
  {
    IOT_SAMPLE_LOG(" "); // Formatting
    IOT_SAMPLE_LOG("Waiting for registration status message.\n");

    // MQTTCLIENT_SUCCESS or MQTTCLIENT_TOPICNAME_TRUNCATED if a message is received.
    // MQTTCLIENT_SUCCESS can also indicate that the timeout expired, in which case message is NULL.
    // MQTTCLIENT_TOPICNAME_TRUNCATED if the topic contains embedded NULL characters.
    // An error code is returned if there was a problem trying to receive a message.
    int rc = MQTTClient_receive(mqtt_client, &topic, &topic_len, &message, MQTT_TIMEOUT_RECEIVE_MS);
    if ((rc != MQTTCLIENT_SUCCESS) && (rc != MQTTCLIENT_TOPICNAME_TRUNCATED))
    {
      IOT_SAMPLE_LOG_ERROR("Failed to receive message: MQTTClient return code %d.", rc);
      exit(rc);
    }
    else if (message == NULL)
    {
      IOT_SAMPLE_LOG_ERROR("Receive message timeout expired: MQTTClient return code %d.", rc);
      exit(rc);
    }
    else if (rc == MQTTCLIENT_TOPICNAME_TRUNCATED)
    {
      topic_len = (int)strlen(topic);
    }
    IOT_SAMPLE_LOG_SUCCESS("Client received a message from the provisioning service.");

    // Parse registration status message.
    az_iot_provisioning_client_register_response register_response;
    parse_device_registration_status_message(topic, topic_len, message, &register_response);
    IOT_SAMPLE_LOG_SUCCESS("Client parsed registration status message.");

    handle_device_registration_status_message(&register_response, &is_operation_complete);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topic);

  } while (!is_operation_complete); // Will loop to receive new operation message.
}

// disconnect_mqtt_client_from_iot_hub disconnects and destroys the underlying MQTT connection and
// Paho handle of the Device Provisioning Service connection.
static void disconnect_mqtt_client_from_provisioning_service(void)
{
  int rc = MQTTClient_disconnect(mqtt_client, MQTT_TIMEOUT_DISCONNECT_MS);
  if (rc != MQTTCLIENT_SUCCESS)
  {
    IOT_SAMPLE_LOG_ERROR("Failed to disconnect MQTT client: MQTTClient return code %d.", rc);
    exit(rc);
  }

  MQTTClient_destroy(&mqtt_client);
}

static void parse_device_registration_status_message(
    char* topic,
    int topic_len,
    MQTTClient_message const* message,
    az_iot_provisioning_client_register_response* out_register_response)
{
  az_span const topic_span = az_span_create((uint8_t*)topic, topic_len);
  az_span const message_span = az_span_create((uint8_t*)message->payload, message->payloadlen);

  // Parse message and retrieve register_response info.
  az_result rc = az_iot_provisioning_client_parse_received_topic_and_payload(
      &provisioning_client, topic_span, message_span, out_register_response);
  if (az_result_failed(rc))
  {
    IOT_SAMPLE_LOG_ERROR("Message from unknown topic: az_result return code 0x%08x.", rc);
    IOT_SAMPLE_LOG_AZ_SPAN("Topic:", topic_span);
    exit(rc);
  }
  IOT_SAMPLE_LOG_SUCCESS("Client received a valid topic response:");
  IOT_SAMPLE_LOG_AZ_SPAN("Topic:", topic_span);
  IOT_SAMPLE_LOG_AZ_SPAN("Payload:", message_span);
  IOT_SAMPLE_LOG("Status: %d", out_register_response->status);
}

// handle_device_registration_status_message parses the response from Device Provisioning
// Service, storing the returning Azure IoT Hub and device information.
static void handle_device_registration_status_message(
    az_iot_provisioning_client_register_response const* register_response,
    bool* ref_is_operation_complete)
{
  *ref_is_operation_complete
      = az_iot_provisioning_client_operation_complete(register_response->operation_status);

  // If operation is not complete, send query. On return, will loop to receive new operation
  // message.
  if (!*ref_is_operation_complete)
  {
    IOT_SAMPLE_LOG("Operation is still pending.");

    send_operation_query_message(register_response);
    IOT_SAMPLE_LOG_SUCCESS("Client sent operation query message.");
  }
  else // Operation is complete.
  {
    if (register_response->operation_status
        == AZ_IOT_PROVISIONING_STATUS_ASSIGNED) // Successful assignment
    {
      IOT_SAMPLE_LOG_SUCCESS("Device provisioned:");
      IOT_SAMPLE_LOG_AZ_SPAN(
          "Hub Hostname:", register_response->registration_state.assigned_hub_hostname);
      IOT_SAMPLE_LOG_AZ_SPAN("Device Id:", register_response->registration_state.device_id);

      device_iot_hub_endpoint
          = az_span_create((uint8_t*)iot_hub_endpoint_buffer, sizeof(iot_hub_endpoint_buffer));
      device_id
          = az_span_create((uint8_t*)iot_hub_device_id_buffer, sizeof(iot_hub_device_id_buffer));

      az_span_copy(
          device_iot_hub_endpoint, register_response->registration_state.assigned_hub_hostname);
      device_iot_hub_endpoint = az_span_slice(
          device_iot_hub_endpoint,
          0,
          az_span_size(register_response->registration_state.assigned_hub_hostname));

      az_span_copy(device_id, register_response->registration_state.device_id);
      device_id = az_span_slice(
          device_id, 0, az_span_size(register_response->registration_state.device_id));

      IOT_SAMPLE_LOG(" "); // Formatting
    }
    else // Unsuccessful assignment (unassigned, failed or disabled states)
    {
      IOT_SAMPLE_LOG_ERROR("Device provisioning failed:");
      IOT_SAMPLE_LOG("Registration state: %d", register_response->operation_status);
      IOT_SAMPLE_LOG("Last operation status: %d", register_response->status);
      IOT_SAMPLE_LOG_AZ_SPAN("Operation ID:", register_response->operation_id);
      IOT_SAMPLE_LOG("Error code: %u", register_response->registration_state.extended_error_code);
      IOT_SAMPLE_LOG_AZ_SPAN("Error message:", register_response->registration_state.error_message);
      IOT_SAMPLE_LOG_AZ_SPAN(
          "Error timestamp:", register_response->registration_state.error_timestamp);
      IOT_SAMPLE_LOG_AZ_SPAN(
          "Error tracking ID:", register_response->registration_state.error_tracking_id);
      exit((int)register_response->registration_state.extended_error_code);
    }
  }
}

// send_operation_query_message PUBLISHes a message to query
// the status of the registration request.
static void send_operation_query_message(
    az_iot_provisioning_client_register_response const* register_response)
{
  // Get the Query Status topic to publish the query status request.
  char query_topic_buffer[QUERY_TOPIC_BUFFER_LENGTH];
  int rc = az_iot_provisioning_client_query_status_get_publish_topic(
      &provisioning_client,
      register_response->operation_id,
      query_topic_buffer,
      sizeof(query_topic_buffer),
      NULL);
  if (az_result_failed(rc))
  {
    IOT_SAMPLE_LOG_ERROR(
        "Unable to get query status publish topic: az_result return code 0x%08x.", rc);
    exit(rc);
  }

  // IMPORTANT: Wait the recommended retry-after number of seconds before query.
  IOT_SAMPLE_LOG("Querying after %u seconds...", register_response->retry_after_seconds);
  iot_sample_sleep_for_seconds(register_response->retry_after_seconds);

  // Publish the query status request.
  rc = MQTTClient_publish(
      mqtt_client, query_topic_buffer, 0, NULL, IOT_SAMPLE_MQTT_PUBLISH_QOS, 0, NULL);
  if (rc != MQTTCLIENT_SUCCESS)
  {
    IOT_SAMPLE_LOG_ERROR("Failed to publish query status request: MQTTClient return code %d.", rc);
    exit(rc);
  }
}

// After successful registration with Device Provisioning Service,
// create_and_configure_mqtt_client_for_iot_hub will perform the basic setup for
// a connection to Azure IoT hub.  This function will NOT initiate any network I/O.
static void create_and_configure_mqtt_client_for_iot_hub(void)
{
  // The environment is filled in based on the provisioning results.
  env_vars.hub_device_id = device_id;
  env_vars.hub_hostname = device_iot_hub_endpoint;

  // Build an MQTT endpoint c-string.
  char mqtt_endpoint_buffer[HUB_ENDPOINT_BUFFER_LENGTH];
  iot_sample_create_mqtt_endpoint(
      PAHO_IOT_HUB, &env_vars, mqtt_endpoint_buffer, sizeof(mqtt_endpoint_buffer));

  // The Plug and Play model ID is specified as an option during initial client initialization.
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = model_id;

  int rc = az_iot_hub_client_init(&hub_client, device_iot_hub_endpoint, device_id, &options);
  if (az_result_failed(rc))
  {
    IOT_SAMPLE_LOG_ERROR("Failed to initialize hub client: az_result return code 0x%08x.", rc);
    exit(rc);
  }

  // Get the MQTT client id used for the MQTT connection.
  char mqtt_client_id_buffer[CLIENT_ID_BUFFER_LENGTH];
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
}

// connect_mqtt_client_to_iot_hub sets up basic security and MQTT topics and then
// initiates an MQTT connection to Azure IoT Hub.
static void connect_mqtt_client_to_iot_hub(void)
{
  // Get the MQTT client username.
  int rc = az_iot_hub_client_get_user_name(
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
  mqtt_connect_options.password = NULL; // This sample uses x509 authentication.
  mqtt_connect_options.cleansession = false; // Set to false so can receive any pending messages.
  mqtt_connect_options.keepAliveInterval = AZ_IOT_DEFAULT_MQTT_CONNECT_KEEPALIVE_SECONDS;

  MQTTClient_SSLOptions mqtt_ssl_options = MQTTClient_SSLOptions_initializer;
  mqtt_ssl_options.keyStore = (char*)az_span_ptr(env_vars.x509_cert_pem_file_path);
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

// disconnect_mqtt_client_from_iot_hub disconnects and destroys the underlying MQTT connection and
// Paho handle of the IoT Hub connection.
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

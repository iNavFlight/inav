// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/*
 * This sample connects an IoT Plug and Play enabled device.
 *
 * IoT Plug and Play requires the device to advertise its capabilities in a device model. This
 * sample implements the modeled declared by dtmi:com:example:TemperatureController;1.  See the
 * readme for more information on this model.  For more information about IoT Plug and Play, see
 * https://aka.ms/iotpnp.
 *
 * The sample listens for incoming commands and property updates.  It also sends telemetry every
 *
 * MQTT_TIMEOUT_RECEIVE_MS milliseconds.  After MQTT_TIMEOUT_RECEIVE_MAX_MESSAGE_COUNT
 * attempts
 * to receive a message from the service, the sample will exit.
 *
 * This sample is composed of multiple sub-components.  These are implemented in separate .c files:
 *   - ./pnp/pnp_temperature_controller_component.c - The temperature controller is the root
 * component.
 *   - ./pnp/pnp_thermostat_component.c - There are two separate simulated thermostats which are
 * modeled as "thermostat1" and "thermostat2".
 *  - ./pnp/pnp_device_info_component.c - The device information component returns simulated device
 *      information for this device.
 *
 * An X509 self-signed certificate is used for authentication, directly to IoT Hub.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <azure/core/az_json.h>
#include <azure/core/az_result.h>
#include <azure/core/az_span.h>
#include <azure/iot/az_iot_hub_client_properties.h>

#include "iot_sample_common.h"
#include "pnp/pnp_device_info_component.h"
#include "pnp/pnp_mqtt_message.h"
#include "pnp/pnp_temperature_controller_component.h"
#include "pnp/pnp_thermostat_component.h"

#define SAMPLE_TYPE PAHO_IOT_HUB
#define SAMPLE_NAME PAHO_IOT_PNP_COMPONENT_SAMPLE

#define HUB_ENDPOINT_BUFFER_LENGTH 128
#define CLIENT_ID_BUFFER_LENGTH 128

#define DEFAULT_START_TEMP_CELSIUS 22.0

bool is_device_operational = true;

// The model this device implements
static az_span const model_id
    = AZ_SPAN_LITERAL_FROM_STR("dtmi:com:example:TemperatureController;1");

// Components thermostat_1 and thermostat_2 are effectively handles to
// the underlying thermostat implementations.
static pnp_thermostat_component thermostat_1;
static pnp_thermostat_component thermostat_2;
static az_span thermostat_1_name = AZ_SPAN_LITERAL_FROM_STR("thermostat1");
static az_span thermostat_2_name = AZ_SPAN_LITERAL_FROM_STR("thermostat2");

// All components that the model supports need to be declared to the az_iot_hub_client.
// In general the az_iot_hub_client knows nothing about the model definition, leaving this entirely
// to the application.  The component list is the only exception.
static az_span pnp_device_components[] = { AZ_SPAN_LITERAL_FROM_STR("thermostat1"),
                                           AZ_SPAN_LITERAL_FROM_STR("thermostat2"),
                                           AZ_SPAN_LITERAL_FROM_STR("deviceInformation") };
static int32_t const pnp_components_length
    = sizeof(pnp_device_components) / sizeof(pnp_device_components[0]);

// Plug and Play command values
static az_span const command_empty_response_payload = AZ_SPAN_LITERAL_FROM_STR("{}");

static iot_sample_environment_variables env_vars;
static az_iot_hub_client hub_client;
static MQTTClient mqtt_client;
static char mqtt_client_username_buffer[512];
static pnp_mqtt_message publish_message;

//
// Functions
//
static void create_and_configure_mqtt_client(void);
static void connect_mqtt_client_to_iot_hub(void);
static void subscribe_mqtt_client_to_iot_hub_topics(void);
static void initialize_thermostat_components(void);
static void send_device_info(void);
static void send_serial_number(void);
static void send_maximum_temperature_since_last_reboot(void);
static void request_all_properties(void);
static void receive_messages_and_send_telemetry_loop(void);
static void disconnect_mqtt_client_from_iot_hub(void);

// General message sending and receiving functions
static void receive_mqtt_message(void);
static void on_message_received(
    char* topic,
    int topic_len,
    MQTTClient_message const* receive_message);

// Device property, command request, telemetry functions
static void handle_device_property_message(
    MQTTClient_message const* receive_message,
    az_iot_hub_client_properties_message const* property_response);
static void handle_command_request(
    MQTTClient_message const* receive_message,
    az_iot_hub_client_command_request const* command_request);
static void send_telemetry_messages(void);

int main(void)
{
  create_and_configure_mqtt_client();
  IOT_SAMPLE_LOG_SUCCESS("Client created and configured.");

  connect_mqtt_client_to_iot_hub();
  IOT_SAMPLE_LOG_SUCCESS("Client connected to IoT Hub.");

  subscribe_mqtt_client_to_iot_hub_topics();
  IOT_SAMPLE_LOG_SUCCESS("Client subscribed to IoT Hub topics.");

  // Initializations
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      pnp_mqtt_message_init(&publish_message), "Failed to initialize pnp_mqtt_message");

  initialize_thermostat_components();
  IOT_SAMPLE_LOG_SUCCESS("Client initialized all components.");

  // Send properties to the IoT Hub that will not change during
  // the lifecycle of a connection (e.g. DeviceInfo and serial number) right
  // after the initial connection.
  send_device_info();
  send_serial_number();
  send_maximum_temperature_since_last_reboot();
  IOT_SAMPLE_LOG_SUCCESS("Initial properties sent");

  // Sends an asychronous request to retrieve all properties about the device
  // on Azure IoT Hub side.
  request_all_properties();
  IOT_SAMPLE_LOG_SUCCESS(
      "Request sent for device's properties.  Response will be received asynchronously.");

  // The device's main loop including primary Plug and Play interaction is
  // in receive_messages_and_send_telemetry_loop.
  receive_messages_and_send_telemetry_loop();
  IOT_SAMPLE_LOG_SUCCESS("Exited receive and send loop.");

  // Disconnect the MQTT connection.
  disconnect_mqtt_client_from_iot_hub();
  IOT_SAMPLE_LOG_SUCCESS("Client disconnected from IoT Hub.");

  return 0;
}

// create_and_configure_mqtt_client reads configuration variables from the environment,
// makes calls to the Azure SDK for C for initial setup, and initiates an MQTT client
// from Paho.
static void create_and_configure_mqtt_client(void)
{
  // Reads in environment variables set by user for purposes of running sample.
  iot_sample_read_environment_variables(SAMPLE_TYPE, SAMPLE_NAME, &env_vars);

  // Build an MQTT endpoint c-string.
  char mqtt_endpoint_buffer[HUB_ENDPOINT_BUFFER_LENGTH];
  iot_sample_create_mqtt_endpoint(
      SAMPLE_TYPE, &env_vars, mqtt_endpoint_buffer, sizeof(mqtt_endpoint_buffer));

  // Initialize the hub client with the connection options.
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = model_id;
  options.component_names = pnp_device_components;
  options.component_names_length = pnp_components_length;

  int rc = az_iot_hub_client_init(
      &hub_client, env_vars.hub_hostname, env_vars.hub_device_id, &options);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Failed to initialize pnp client");

  // Get the MQTT client id used for the MQTT connection.
  char mqtt_client_id_buffer[CLIENT_ID_BUFFER_LENGTH];

  rc = az_iot_hub_client_get_client_id(
      &hub_client, mqtt_client_id_buffer, sizeof(mqtt_client_id_buffer), NULL);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Failed to get MQTT client id");

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
  az_result rc = az_iot_hub_client_get_user_name(
      &hub_client, mqtt_client_username_buffer, sizeof(mqtt_client_username_buffer), NULL);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Failed to get MQTT client username");

  // Set MQTT connection options.
  MQTTClient_connectOptions mqtt_connect_options = MQTTClient_connectOptions_initializer;
  mqtt_connect_options.username = mqtt_client_username_buffer;
  mqtt_connect_options.password = NULL; // This sample uses x509 authentication.
  mqtt_connect_options.cleansession = false; // Set to false so can receive any pending messages.
  mqtt_connect_options.keepAliveInterval = AZ_IOT_DEFAULT_MQTT_CONNECT_KEEPALIVE_SECONDS;

  MQTTClient_SSLOptions mqtt_ssl_options = MQTTClient_SSLOptions_initializer;
  mqtt_ssl_options.verify = 1;
  mqtt_ssl_options.enableServerCertAuth = 1;
  mqtt_ssl_options.keyStore = (char*)az_span_ptr(env_vars.x509_cert_pem_file_path);
  if (az_span_size(env_vars.x509_trust_pem_file_path) != 0) // Is only set if required by OS.
  {
    mqtt_ssl_options.trustStore = (char*)az_span_ptr(env_vars.x509_trust_pem_file_path);
  }
  mqtt_connect_options.ssl = &mqtt_ssl_options;

  // Connect MQTT client to the Azure IoT Hub.  This will block until the connection
  // is established or fails.
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

// subscribe_mqtt_client_to_iot_hub_topics subscribes to well-known MQTT topics that Azure IoT Hub
// uses to signal incoming commands to the device and notify device of properties.
static void subscribe_mqtt_client_to_iot_hub_topics(void)
{
  // Subscribe to incoming commands.
  int rc = MQTTClient_subscribe(mqtt_client, AZ_IOT_HUB_CLIENT_COMMANDS_SUBSCRIBE_TOPIC, 1);
  if (rc != MQTTCLIENT_SUCCESS)
  {
    IOT_SAMPLE_LOG_ERROR(
        "Failed to subscribe to the command topic: MQTTClient return code %d.", rc);
    exit(rc);
  }

  // Subscribe to writable property update notifications.  Messages will be sent to this topic when
  // writable properties are updated by the service.
  rc = MQTTClient_subscribe(
      mqtt_client, AZ_IOT_HUB_CLIENT_PROPERTIES_WRITABLE_UPDATES_SUBSCRIBE_TOPIC, 1);
  if (rc != MQTTCLIENT_SUCCESS)
  {
    IOT_SAMPLE_LOG_ERROR(
        "Failed to subscribe to the property writable updates topic: MQTTClient return code %d.",
        rc);
    exit(rc);
  }

  // Subscribe to the properties response topic.  When the device invokes a PUBLISH to get
  // all properties (both reported from device and reported - see request_all_properties() below)
  // the property payload will be sent to this topic.
  rc = MQTTClient_subscribe(mqtt_client, AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_SUBSCRIBE_TOPIC, 1);
  if (rc != MQTTCLIENT_SUCCESS)
  {
    IOT_SAMPLE_LOG_ERROR(
        "Failed to subscribe to the property response topic: MQTTClient return code %d.", rc);
    exit(rc);
  }
}

// initialize_thermostat_components invokes the underlying pnp_thermostat_init routines.
static void initialize_thermostat_components(void)
{
  // Initialize thermostats 1 and 2.
  az_result rc = pnp_thermostat_init(&thermostat_1, thermostat_1_name, DEFAULT_START_TEMP_CELSIUS);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Failed to initialize Temperature Sensor 1");

  rc = pnp_thermostat_init(&thermostat_2, thermostat_2_name, DEFAULT_START_TEMP_CELSIUS);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Failed to initialize Temperature Sensor 2");
}

// send_device_info invokes the underlying device info component to send the device's simulated
// information.
static void send_device_info(void)
{
  pnp_device_info_send_reported_properties(&hub_client, mqtt_client);
}

// send_serial_number invokes the temperature controller to send its simulated serial number
// property.
static void send_serial_number(void)
{
  pnp_temperature_controller_send_serial_number(&hub_client, mqtt_client);
}

// send_maximum_temperature_since_last_reboot sends the maximum temperature since last reboot
// for both thermostat components.
static void send_maximum_temperature_since_last_reboot(void)
{
  pnp_thermostat_update_maximum_temperature_property(&thermostat_1, &hub_client, mqtt_client);
  pnp_thermostat_update_maximum_temperature_property(&thermostat_2, &hub_client, mqtt_client);
}

// request_all_properties sends a request to Azure IoT Hub to request all properties for
// the device.  This call does not block.  Properties will be received on
// a topic previously subscribed to (AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_SUBSCRIBE_TOPIC.)
static void request_all_properties(void)
{
  IOT_SAMPLE_LOG("Client requesting device property document from service.");

  // Get the property document topic to publish the property document request.
  az_result rc = az_iot_hub_client_properties_document_get_publish_topic(
      &hub_client,
      pnp_mqtt_get_request_id(),
      publish_message.topic,
      publish_message.topic_length,
      NULL);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Failed to get the property document topic");

  // Publish the property document request.
  publish_mqtt_message(
      mqtt_client, publish_message.topic, AZ_SPAN_EMPTY, IOT_SAMPLE_MQTT_PUBLISH_QOS);
  IOT_SAMPLE_LOG(" "); // Formatting
}

// receive_messages_and_send_telemetry_loop will loop to check if there are incoming MQTT
// messages, waiting up to MQTT_TIMEOUT_RECEIVE_MS.  It will also send a telemetry message
// every time through the loop.
static void receive_messages_and_send_telemetry_loop(void)
{
  // Continue to receive commands or device property messages while device is operational.
  while (is_device_operational)
  {
    // Send telemetry messages. No response requested from server.
    send_telemetry_messages();
    IOT_SAMPLE_LOG(" "); // Formatting.

    // Wait for any server-initiated messages.
    receive_mqtt_message();
  }
}

// disconnect_mqtt_client_from_iot_hub disconnects and destroys the underlying MQTT connection and
// Paho handle.
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

// receive_mqtt_message checks for incoming messages from Azure IoT Hub.
static void receive_mqtt_message(void)
{
  char* topic = NULL;
  int topic_len = 0;
  MQTTClient_message* receive_message = NULL;
  static int8_t timeout_counter;

  IOT_SAMPLE_LOG("Waiting for command request or device property message.\n");

  // MQTTCLIENT_SUCCESS or MQTTCLIENT_TOPICNAME_TRUNCATED if a message is received.
  // MQTTCLIENT_SUCCESS can also indicate that the timeout expired, in which case message is NULL.
  // MQTTCLIENT_TOPICNAME_TRUNCATED if the topic contains embedded NULL characters.
  // An error code is returned if there was a problem trying to receive a message.
  int rc = MQTTClient_receive(
      mqtt_client, &topic, &topic_len, &receive_message, MQTT_TIMEOUT_RECEIVE_MS);
  if ((rc != MQTTCLIENT_SUCCESS) && (rc != MQTTCLIENT_TOPICNAME_TRUNCATED))
  {
    IOT_SAMPLE_LOG_ERROR("Failed to receive message: MQTTClient return code %d.", rc);
    exit(rc);
  }
  else if (receive_message == NULL)
  {
    // Allow up to TIMEOUT_MQTT_RECEIVE_MAX_COUNT before disconnecting.
    if (++timeout_counter >= MQTT_TIMEOUT_RECEIVE_MAX_MESSAGE_COUNT)
    {
      IOT_SAMPLE_LOG(
          "Receive message timeout expiration count of %d reached.",
          MQTT_TIMEOUT_RECEIVE_MAX_MESSAGE_COUNT);
      is_device_operational = false;
    }
  }
  else
  {
    IOT_SAMPLE_LOG_SUCCESS("Client received a message from the service.");
    timeout_counter = 0; // Reset

    if (rc == MQTTCLIENT_TOPICNAME_TRUNCATED)
    {
      topic_len = (int)strlen(topic);
    }

    on_message_received(topic, topic_len, receive_message);
    IOT_SAMPLE_LOG(" "); // Formatting

    MQTTClient_freeMessage(&receive_message);
    MQTTClient_free(topic);
  }
}

// on_message_received dispatches an MQTT message when the underlying MQTT stack provides one
static void on_message_received(
    char* topic,
    int topic_len,
    MQTTClient_message const* receive_message)
{
  az_result rc;

  az_span const topic_span = az_span_create((uint8_t*)topic, topic_len);
  az_span const message_span
      = az_span_create((uint8_t*)receive_message->payload, receive_message->payloadlen);

  az_iot_hub_client_properties_message property_message;
  az_iot_hub_client_command_request command_request;

  // Parse the incoming message topic and handle appropriately.
  rc = az_iot_hub_client_properties_parse_received_topic(
      &hub_client, topic_span, &property_message);
  if (az_result_succeeded(rc))
  {
    IOT_SAMPLE_LOG_SUCCESS("Client received a valid property topic response.");
    IOT_SAMPLE_LOG_AZ_SPAN("Topic:", topic_span);
    IOT_SAMPLE_LOG_AZ_SPAN("Payload:", message_span);
    IOT_SAMPLE_LOG("Status: %d", property_message.status);

    handle_device_property_message(receive_message, &property_message);
  }
  else
  {
    rc = az_iot_hub_client_commands_parse_received_topic(&hub_client, topic_span, &command_request);
    if (az_result_succeeded(rc))
    {
      IOT_SAMPLE_LOG_SUCCESS("Client received a valid command topic message.");
      IOT_SAMPLE_LOG_AZ_SPAN("Topic:", topic_span);
      IOT_SAMPLE_LOG_AZ_SPAN("Payload:", message_span);

      handle_command_request(receive_message, &command_request);
    }
    else
    {
      IOT_SAMPLE_LOG_ERROR("Message from unknown topic: az_result return code 0x%08x.", rc);
      IOT_SAMPLE_LOG_AZ_SPAN("Topic:", topic_span);
      exit(rc);
    }
  }
}

// process_device_property_message handles incoming properties from Azure IoT Hub.
static void process_device_property_message(
    az_span property_message_span,
    az_iot_hub_client_properties_message_type message_type)
{
  az_json_reader jr;
  az_span component_name;
  int32_t version = 0;
  az_result rc = az_json_reader_init(&jr, property_message_span, NULL);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Could not initialize the json reader");

  rc = az_iot_hub_client_properties_get_properties_version(
      &hub_client, &jr, message_type, &version);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Could not get the property version");

  rc = az_json_reader_init(&jr, property_message_span, NULL);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Could not initialize the json reader");

  // Applications call az_iot_hub_client_properties_get_next_component_property to enumerate
  // properties received.
  while (az_result_succeeded(
      rc = az_iot_hub_client_properties_get_next_component_property(
          &hub_client, &jr, message_type, AZ_IOT_HUB_CLIENT_PROPERTY_WRITABLE, &component_name)))
  {
    if (rc == AZ_OK)
    {
      // Once we have received a property, determine which component the property is intended for
      // and route to the intended property component handler.  The component handler
      // takes over from this point, for both parsing the request and responding to IoT Hub.
      if (az_span_is_content_equal(component_name, thermostat_1_name))
      {
        pnp_thermostat_process_property_update(
            &thermostat_1, &hub_client, mqtt_client, &jr, version);
      }
      else if (az_span_is_content_equal(component_name, thermostat_2_name))
      {
        pnp_thermostat_process_property_update(
            &thermostat_2, &hub_client, mqtt_client, &jr, version);
      }
      else
      {
        char const* const log_message = "Failed to process property update";

        // Only the thermostat component supports writable properties;
        // the models for DeviceInfo and the temperature controller do not.
        IOT_SAMPLE_LOG_ERROR("Received property update for an unsupported component");

        // We do NOT report back an error for an unknown property to IoT Hub,
        // but we do need to skip past the JSON part of the body to continue reading.
        IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_reader_next_token(&jr), log_message);
        IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_reader_skip_children(&jr), log_message);
        IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_reader_next_token(&jr), log_message);
      }
    }
    else
    {
      IOT_SAMPLE_LOG_ERROR("Failed to update a property: az_result return code 0x%08x.", rc);
      exit(rc);
    }
  }
}

// handle_device_property_message handles incoming properties from Azure IoT Hub.
static void handle_device_property_message(
    MQTTClient_message const* receive_message,
    az_iot_hub_client_properties_message const* property_message)
{
  az_span const message_span
      = az_span_create((uint8_t*)receive_message->payload, receive_message->payloadlen);

  // Invoke appropriate action per message type (3 types only).
  switch (property_message->message_type)
  {
    // A response from a property GET publish message with the property document as a payload.
    case AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_GET_RESPONSE:
      IOT_SAMPLE_LOG("Message Type: GET");
      (void)process_device_property_message(message_span, property_message->message_type);
      break;

    // An update to the desired properties with the properties as a payload.
    case AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_WRITABLE_UPDATED:
      IOT_SAMPLE_LOG("Message Type: Desired Properties");
      (void)process_device_property_message(message_span, property_message->message_type);
      break;

    // When the device publishes a property update, this message type arrives when
    // server acknowledges this.
    case AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_ACKNOWLEDGEMENT:
      IOT_SAMPLE_LOG("Message Type: Previous property update from device acknowledged by IoT Hub");
      break;

    // An error has occurred
    case AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_ERROR:
      IOT_SAMPLE_LOG_ERROR("Message Type: Request Error");
      break;
  }
}

// handle_command_request handles an incoming command from Azure IoT Hub
static void handle_command_request(
    MQTTClient_message const* receive_message,
    az_iot_hub_client_command_request const* command_request)
{
  az_span const message_span
      = az_span_create((uint8_t*)receive_message->payload, receive_message->payloadlen);
  az_iot_status status = AZ_IOT_STATUS_UNKNOWN;

  // Once we have received the command, this function disptaches to the underlying command handler
  // to parse the request and respond to Azure IoT Hub.
  if (az_span_size(command_request->component_name) == 0)
  {
    pnp_temperature_controller_process_command_request(
        &hub_client, mqtt_client, command_request, message_span);
  }
  else if (az_span_is_content_equal(thermostat_1.component_name, command_request->component_name))
  {
    pnp_thermostat_process_command_request(
        &thermostat_1, &hub_client, mqtt_client, command_request, message_span);
  }
  else if (az_span_is_content_equal(thermostat_2.component_name, command_request->component_name))
  {
    pnp_thermostat_process_command_request(
        &thermostat_2, &hub_client, mqtt_client, command_request, message_span);
  }
  else
  {
    // The request was sent to a component that this model does not support.  In this case the main
    // handler sends a 404 response to the server.
    IOT_SAMPLE_LOG_AZ_SPAN(
        "Requested command sent to unsupported component:", command_request->component_name);
    publish_message.out_payload = command_empty_response_payload;
    status = AZ_IOT_STATUS_NOT_FOUND;

    // Get the commands response topic to publish the command response.
    az_result rc = az_iot_hub_client_commands_response_get_publish_topic(
        &hub_client,
        command_request->request_id,
        (uint16_t)status,
        publish_message.topic,
        publish_message.topic_length,
        NULL);
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Failed to get the commands response topic");

    // Publish the command response.
    publish_mqtt_message(
        mqtt_client,
        publish_message.topic,
        publish_message.out_payload,
        IOT_SAMPLE_MQTT_PUBLISH_QOS);
    IOT_SAMPLE_LOG_SUCCESS("Client published command response.");
    IOT_SAMPLE_LOG("Status: %d", status);
    IOT_SAMPLE_LOG_AZ_SPAN("Payload:", publish_message.out_payload);
  }
}

// send_telemetry_messages invokes those components that periodically send telemetry.
static void send_telemetry_messages(void)
{
  pnp_thermostat_send_telemetry_message(&thermostat_1, &hub_client, mqtt_client);
  pnp_thermostat_send_telemetry_message(&thermostat_2, &hub_client, mqtt_client);
  pnp_temperature_controller_send_workingset(&hub_client, mqtt_client);
}

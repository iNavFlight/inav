// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/*
 * Common implementation for *a device* that implements the Model Id
 * "dtmi:com:example:Thermostat;1".  The model JSON is available in
 * https://github.com/Azure/opendigitaltwins-dtdl/blob/master/DTDL/v2/samples/Thermostat.json.
 *
 * This code assumes that an MQTT connection to Azure IoT hub and that the underlying
 * az_iot_hub_client have already been initialized in the variables mqtt_client and hub_client.  The
 * sample callers paho_iot_pnp_sample.c and paho_iot_pnp_sample_with_provisioning.c do this before
 * invoking paho_iot_pnp_sample_device_implement().
 *
 * This should not be confused with ./pnp/pnp_thermostat_component.c.  Both C files implement
 * The Thermostat Model Id.  In this file, the Thermostat is the only Model that the device
 * implements.  In ./pnp/pnp_thermostat_component.c, the Thermostat is a subcomponent of a more
 * complex device and hence the logic is more complex.
 */

#ifdef _MSC_VER
// warning C4204: nonstandard extension used: non-constant aggregate initializer
#pragma warning(disable : 4204)
// warning C4996: 'localtime': This function or variable may be unsafe.  Consider using localtime_s
// instead.
#pragma warning(disable : 4996)
#endif

#ifdef _MSC_VER
#pragma warning(push)
// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable : 4201)
#endif
#include <MQTTClient.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <stdlib.h>
#include <time.h>

#include <azure/core/az_json.h>
#include <azure/core/az_result.h>
#include <azure/core/az_span.h>
#include <azure/iot/az_iot_hub_client_properties.h>

#include "iot_sample_common.h"
#include "paho_iot_pnp_sample_common.h"

MQTTClient mqtt_client;
az_iot_hub_client hub_client;

#define DEFAULT_START_TEMP_COUNT 1
#define DEFAULT_START_TEMP_CELSIUS 22.0
#define DOUBLE_DECIMAL_PLACE_DIGITS 2

#define SAMPLE_MQTT_TOPIC_LENGTH 128
#define SAMPLE_MQTT_PAYLOAD_LENGTH 128

bool is_device_operational = true;
static char const iso_spec_time_format[] = "%Y-%m-%dT%H:%M:%SZ"; // ISO8601 Time Format

// MQTT Connection Values
static uint16_t connection_request_id = 0;
static char connection_request_id_buffer[16];

// Property Values
static az_span const property_success_name = AZ_SPAN_LITERAL_FROM_STR("success");
static az_span const property_desired_temperature_name
    = AZ_SPAN_LITERAL_FROM_STR("targetTemperature");
static az_span const property_reported_maximum_temperature_name
    = AZ_SPAN_LITERAL_FROM_STR("maxTempSinceLastReboot");

// Command Values
static az_span const command_getMaxMinReport_name = AZ_SPAN_LITERAL_FROM_STR("getMaxMinReport");
static az_span const command_max_temp_name = AZ_SPAN_LITERAL_FROM_STR("maxTemp");
static az_span const command_min_temp_name = AZ_SPAN_LITERAL_FROM_STR("minTemp");
static az_span const command_avg_temp_name = AZ_SPAN_LITERAL_FROM_STR("avgTemp");
static az_span const command_start_time_name = AZ_SPAN_LITERAL_FROM_STR("startTime");
static az_span const command_end_time_name = AZ_SPAN_LITERAL_FROM_STR("endTime");
static az_span const command_empty_response_payload = AZ_SPAN_LITERAL_FROM_STR("{}");
static char command_start_time_value_buffer[32];
static char command_end_time_value_buffer[32];
static char command_response_payload_buffer[256];

// Telemetry Values
static az_span const telemetry_temperature_name = AZ_SPAN_LITERAL_FROM_STR("temperature");

// Device Values
static double device_current_temperature = DEFAULT_START_TEMP_CELSIUS;
static double device_maximum_temperature = DEFAULT_START_TEMP_CELSIUS;
static double device_minimum_temperature = DEFAULT_START_TEMP_CELSIUS;
static double device_temperature_summation = DEFAULT_START_TEMP_CELSIUS;
static uint32_t device_temperature_count = DEFAULT_START_TEMP_COUNT;
static double device_average_temperature = DEFAULT_START_TEMP_CELSIUS;

//
// Functions
//
static void subscribe_mqtt_client_to_iot_hub_topics(void);
static void request_all_properties(void);
static void receive_messages_and_send_telemetry_loop(void);

static az_span get_request_id(void);
static void publish_mqtt_message(char const* topic, az_span payload, int qos);
static void on_message_received(char* topic, int topic_len, MQTTClient_message const* message);

// Device Property functions
static void handle_device_property_message(
    MQTTClient_message const* message,
    az_iot_hub_client_properties_message const* property_message);
static void process_device_property_message(
    az_span message_span,
    az_iot_hub_client_properties_message_type message_type);
static void update_device_temperature_property(double temperature, bool* out_is_max_temp_changed);
static void send_reported_property(
    az_span name,
    double value,
    int32_t version,
    bool write_payload_with_status);

// Command functions
static void handle_command_request(
    MQTTClient_message const* message,
    az_iot_hub_client_command_request const* command_request);
static void send_command_response(
    az_iot_hub_client_command_request const* command_request,
    az_iot_status status,
    az_span response);
static bool invoke_getMaxMinReport(az_span payload, az_span response, az_span* out_response);

// Telemetry functions
static void send_telemetry_message(void);

// JSON write functions
static void write_property_payload(
    uint8_t property_count,
    az_span const names[],
    double const values[],
    az_span const times[],
    az_span property_payload,
    az_span* out_property_payload);
static void write_property_payload_with_status(
    az_span name,
    double value,
    int32_t status_code_value,
    int32_t version_value,
    az_span description_value,
    az_span property_payload,
    az_span* out_property_payload);

// thermostat_device_implement is invoked by the caller to simulate the thermostat device.
// It assumes that the underlying MQTT connection to Azure IoT Hub has already been established.
void paho_iot_pnp_sample_device_implement(void)
{
  subscribe_mqtt_client_to_iot_hub_topics();
  IOT_SAMPLE_LOG_SUCCESS("Client subscribed to IoT Hub topics.");

  send_reported_property(
      property_reported_maximum_temperature_name, device_maximum_temperature, 0, false);
  IOT_SAMPLE_LOG_SUCCESS("Publishing update of device's maximum temperature.  Response will be "
                         "received asynchronously.");

  request_all_properties();
  IOT_SAMPLE_LOG_SUCCESS(
      "Request sent for device's properties.  Response will be received asynchronously.");

  receive_messages_and_send_telemetry_loop();
  IOT_SAMPLE_LOG_SUCCESS("Exited receive and send loop.");
}

// subscribe_mqtt_client_to_iot_hub_topics subscribes to well-known MQTT topics that Azure IoT Hub
// uses to signal incoming commands to the device and notify device of properties.
static void subscribe_mqtt_client_to_iot_hub_topics(void)
{
  int rc;

  // Subscribe to incoming commands.
  rc = MQTTClient_subscribe(mqtt_client, AZ_IOT_HUB_CLIENT_COMMANDS_SUBSCRIBE_TOPIC, 1);
  if (rc != MQTTCLIENT_SUCCESS)
  {
    IOT_SAMPLE_LOG_ERROR(
        "Failed to subscribe to the commands topic: MQTTClient return code %d.", rc);
    exit(rc);
  }

  // Subscribe to property update notifications.  Messages will be sent to this topic when
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

  // Subscribe to the properties message topic.  When the device invokes a PUBLISH to get
  // all properties (both reported from device and reported - see request_all_properties() below)
  // the property payload will be sent to this topic.
  rc = MQTTClient_subscribe(mqtt_client, AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_SUBSCRIBE_TOPIC, 1);
  if (rc != MQTTCLIENT_SUCCESS)
  {
    IOT_SAMPLE_LOG_ERROR(
        "Failed to subscribe to the property message topic: MQTTClient return code %d.", rc);
    exit(rc);
  }
}

// request_all_properties sends a request to Azure IoT Hub to request all properties for
// the device.  This call does not block.  Properties will be received on
// a topic previously subscribed to (AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_SUBSCRIBE_TOPIC.)
static void request_all_properties(void)
{
  az_result rc;

  IOT_SAMPLE_LOG("Client requesting device property document from service.");

  // Get the topic to publish the property document request.
  char property_document_topic_buffer[SAMPLE_MQTT_TOPIC_LENGTH];
  rc = az_iot_hub_client_properties_document_get_publish_topic(
      &hub_client,
      get_request_id(),
      property_document_topic_buffer,
      sizeof(property_document_topic_buffer),
      NULL);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Failed to get the property document topic");

  // Publish the property document request.
  publish_mqtt_message(property_document_topic_buffer, AZ_SPAN_EMPTY, IOT_SAMPLE_MQTT_PUBLISH_QOS);
}

// receive_messages_and_send_telemetry_loop will loop to check if there are incoming MQTT
// messages, waiting up to MQTT_TIMEOUT_RECEIVE_MS.  It will also send a telemetry message
// every time through the loop.
static void receive_messages_and_send_telemetry_loop(void)
{
  char* topic = NULL;
  int topic_len = 0;
  MQTTClient_message* message = NULL;
  uint8_t timeout_counter = 0;

  // Continue to receive commands or device property messages while device is operational.
  while (is_device_operational)
  {
    IOT_SAMPLE_LOG(" "); // Formatting
    IOT_SAMPLE_LOG("Waiting for command request or device property message.\n");

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
      // Allow up to MQTT_TIMEOUT_RECEIVE_MAX_MESSAGE_COUNT timeouts before disconnecting.
      if (++timeout_counter >= MQTT_TIMEOUT_RECEIVE_MAX_MESSAGE_COUNT)
      {
        IOT_SAMPLE_LOG(
            "Receive message timeout expiration count of %d reached.",
            MQTT_TIMEOUT_RECEIVE_MAX_MESSAGE_COUNT);
        return;
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

      on_message_received(topic, topic_len, message);
      IOT_SAMPLE_LOG(" "); // Formatting

      MQTTClient_freeMessage(&message);
      MQTTClient_free(topic);
    }

    send_telemetry_message();
  }
}

// get_request_id sets a request Id into connection_request_id_buffer and monotonically
// increases the counter for the next MQTT operation.
static az_span get_request_id(void)
{
  az_span remainder;
  az_span out_span = az_span_create(
      (uint8_t*)connection_request_id_buffer, sizeof(connection_request_id_buffer));

  connection_request_id++;
  if (connection_request_id == UINT16_MAX)
  {
    // Connection id has looped.  Reset.
    connection_request_id = 1;
  }

  az_result rc = az_span_u32toa(out_span, connection_request_id, &remainder);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Failed to get request id");

  return az_span_slice(out_span, 0, az_span_size(out_span) - az_span_size(remainder));
}

// publish_mqtt_message is a wrapper to the underlying Paho PUBLISH method
static void publish_mqtt_message(const char* topic, az_span payload, int qos)
{
  int rc = MQTTClient_publish(
      mqtt_client, topic, az_span_size(payload), az_span_ptr(payload), qos, 0, NULL);
  if (rc != MQTTCLIENT_SUCCESS)
  {
    IOT_SAMPLE_LOG_ERROR("Failed to publish message: MQTTClient return code %d", rc);
    exit(rc);
  }
}

// on_message_received dispatches an MQTT message when the underlying MQTT stack provides one
static void on_message_received(char* topic, int topic_len, MQTTClient_message const* message)
{
  az_result rc;

  az_span const topic_span = az_span_create((uint8_t*)topic, topic_len);
  az_span const message_span = az_span_create((uint8_t*)message->payload, message->payloadlen);

  az_iot_hub_client_properties_message property_message;
  az_iot_hub_client_command_request command_request;

  // Parse the incoming message topic and handle appropriately.
  // Note that if a topic does not match - e.g. az_iot_hub_client_properties_parse_received_topic is
  // invoked to process a command message - the function returns AZ_ERROR_IOT_TOPIC_NO_MATCH.  This
  // is NOT a fatal error but is used to indicate to the caller to see if the topic matches other
  // topics.
  rc = az_iot_hub_client_properties_parse_received_topic(
      &hub_client, topic_span, &property_message);
  if (az_result_succeeded(rc))
  {
    IOT_SAMPLE_LOG_SUCCESS("Client received a valid topic.");
    IOT_SAMPLE_LOG_AZ_SPAN("Topic:", topic_span);
    IOT_SAMPLE_LOG_AZ_SPAN("Payload:", message_span);
    IOT_SAMPLE_LOG("Status: %d", property_message.status);

    handle_device_property_message(message, &property_message);
  }
  else
  {
    rc = az_iot_hub_client_commands_parse_received_topic(&hub_client, topic_span, &command_request);
    if (az_result_succeeded(rc))
    {
      IOT_SAMPLE_LOG_SUCCESS("Client received a valid topic.");
      IOT_SAMPLE_LOG_AZ_SPAN("Topic:", topic_span);
      IOT_SAMPLE_LOG_AZ_SPAN("Payload:", message_span);

      handle_command_request(message, &command_request);
    }
    else
    {
      IOT_SAMPLE_LOG_ERROR("Message from unknown topic: az_result return code 0x%08x.", rc);
      IOT_SAMPLE_LOG_AZ_SPAN("Topic:", topic_span);
      exit(rc);
    }
  }
}

// handle_device_property_message handles incoming properties from Azure IoT Hub.
static void handle_device_property_message(
    MQTTClient_message const* message,
    az_iot_hub_client_properties_message const* property_message)
{
  az_span const message_span = az_span_create((uint8_t*)message->payload, message->payloadlen);

  // Invoke appropriate action per message type (3 types only).
  switch (property_message->message_type)
  {
    // A message from a property GET publish message with the property document as a payload.
    case AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_GET_RESPONSE:
      IOT_SAMPLE_LOG("Message Type: GET");
      process_device_property_message(message_span, property_message->message_type);
      break;

    // An update to the desired properties with the properties as a payload.
    case AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_WRITABLE_UPDATED:
      IOT_SAMPLE_LOG("Message Type: Desired Properties");
      process_device_property_message(message_span, property_message->message_type);
      break;

    // When the device publishes a property update, this message type arrives when
    // server acknowledges this.
    case AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_ACKNOWLEDGEMENT:
      IOT_SAMPLE_LOG("Message Type: IoT Hub has acknowledged properties that the device sent");
      break;

    // An error has occurred
    case AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_ERROR:
      IOT_SAMPLE_LOG_ERROR("Message Type: Request Error");
      break;
  }
}

// process_device_property_message handles incoming properties from Azure IoT Hub.
static void process_device_property_message(
    az_span message_span,
    az_iot_hub_client_properties_message_type message_type)
{
  az_json_reader jr;
  az_result rc = az_json_reader_init(&jr, message_span, NULL);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Could not initialize json reader");

  int32_t version_number;
  rc = az_iot_hub_client_properties_get_properties_version(
      &hub_client, &jr, message_type, &version_number);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Could not get property version");

  rc = az_json_reader_init(&jr, message_span, NULL);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Could not initialize json reader");

  double desired_temperature;
  az_span component_name;

  // Applications call az_iot_hub_client_properties_get_next_component_property to enumerate
  // properties received.
  while (az_result_succeeded(az_iot_hub_client_properties_get_next_component_property(
      &hub_client, &jr, message_type, AZ_IOT_HUB_CLIENT_PROPERTY_WRITABLE, &component_name)))
  {
    if (az_json_token_is_text_equal(&jr.token, property_desired_temperature_name))
    {
      // Process an update for desired temperature.
      rc = az_json_reader_next_token(&jr);
      if (az_result_failed(rc))
      {
        IOT_SAMPLE_LOG_ERROR("Could not move to property value");
      }

      rc = az_json_token_get_double(&jr.token, &desired_temperature);
      if (az_result_failed(rc))
      {
        IOT_SAMPLE_LOG_ERROR("Could not get property value");
      }

      IOT_SAMPLE_LOG(" "); // Formatting

      bool is_max_temp_changed;

      // Update device temperature locally and report update to server.
      update_device_temperature_property(desired_temperature, &is_max_temp_changed);
      send_reported_property(
          property_desired_temperature_name, desired_temperature, version_number, true);

      if (is_max_temp_changed)
      {
        send_reported_property(
            property_reported_maximum_temperature_name, device_maximum_temperature, 0, false);
      }

      // Skip to next property value
      rc = az_json_reader_next_token(&jr);
      if (az_result_failed(rc))
      {
        IOT_SAMPLE_LOG_ERROR("Invalid JSON. Could not move to next property name");
      }
    }
    else
    {
      IOT_SAMPLE_LOG_AZ_SPAN("Unknown Property Received:", jr.token.slice);
      // The JSON reader must be advanced regardless of whether the property
      // is of interest or not.
      rc = az_json_reader_next_token(&jr);
      if (az_result_failed(rc))
      {
        IOT_SAMPLE_LOG_ERROR("Invalid JSON. Could not move to next property value");
      }

      // Skip children in case the property value is an object
      rc = az_json_reader_skip_children(&jr);
      if (az_result_failed(rc))
      {
        IOT_SAMPLE_LOG_ERROR("Invalid JSON. Could not skip children");
      }

      rc = az_json_reader_next_token(&jr);
      if (az_result_failed(rc))
      {
        IOT_SAMPLE_LOG_ERROR("Invalid JSON. Could not move to next property name");
      }
    }
  }
}

// update_device_temperature_property updates the simulated device temperature as well
// as the temperature's statistics over time
static void update_device_temperature_property(double temperature, bool* out_is_max_temp_changed)
{
  *out_is_max_temp_changed = false;
  device_current_temperature = temperature;

  // Update maximum or minimum temperatures.
  if (device_current_temperature > device_maximum_temperature)
  {
    device_maximum_temperature = device_current_temperature;
    *out_is_max_temp_changed = true;
  }
  else if (device_current_temperature < device_minimum_temperature)
  {
    device_minimum_temperature = device_current_temperature;
  }

  // Calculate the new average temperature.
  device_temperature_count++;
  device_temperature_summation += device_current_temperature;
  device_average_temperature = device_temperature_summation / device_temperature_count;

  IOT_SAMPLE_LOG_SUCCESS("Client updated desired temperature variables locally.");
  IOT_SAMPLE_LOG("Current Temperature: %2f", device_current_temperature);
  IOT_SAMPLE_LOG("Maximum Temperature: %2f", device_maximum_temperature);
  IOT_SAMPLE_LOG("Minimum Temperature: %2f", device_minimum_temperature);
  IOT_SAMPLE_LOG("Average Temperature: %2f", device_average_temperature);
}

// send_reported_property writes a property payload reporting device state and then sends it to
// Azure IoT Hub.
static void send_reported_property(
    az_span name,
    double value,
    int32_t version,
    bool write_payload_with_status)
{
  az_result rc;

  // Get the property topic to send a reported property update.
  char property_update_topic_buffer[SAMPLE_MQTT_TOPIC_LENGTH];
  rc = az_iot_hub_client_properties_get_reported_publish_topic(
      &hub_client,
      get_request_id(),
      property_update_topic_buffer,
      sizeof(property_update_topic_buffer),
      NULL);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Failed to get the property update topic");

  // Write the updated reported property message.
  char reported_property_payload_buffer[SAMPLE_MQTT_PAYLOAD_LENGTH];
  az_span reported_property_payload = AZ_SPAN_FROM_BUFFER(reported_property_payload_buffer);

  if (write_payload_with_status)
  {
    write_property_payload_with_status(
        name,
        value,
        AZ_IOT_STATUS_OK,
        version,
        property_success_name,
        reported_property_payload,
        &reported_property_payload);
  }
  else
  {
    uint8_t count = 1;
    az_span const names[1] = { name };
    double const values[1] = { value };

    write_property_payload(
        count, names, values, NULL, reported_property_payload, &reported_property_payload);
  }

  // Publish the reported property update.
  publish_mqtt_message(
      property_update_topic_buffer, reported_property_payload, IOT_SAMPLE_MQTT_PUBLISH_QOS);
  IOT_SAMPLE_LOG_SUCCESS("Client published the property update reported property message.");
  IOT_SAMPLE_LOG_AZ_SPAN("Payload:", reported_property_payload);
}

// handle_command_request handles an incoming command from Azure IoT Hub
static void handle_command_request(
    MQTTClient_message const* message,
    az_iot_hub_client_command_request const* command_request)
{
  az_span const message_span = az_span_create((uint8_t*)message->payload, message->payloadlen);

  if ((az_span_is_content_equal(command_request->component_name, AZ_SPAN_EMPTY))
      && (az_span_is_content_equal(command_getMaxMinReport_name, command_request->command_name)))
  {
    az_iot_status status;
    az_span command_response_payload = AZ_SPAN_FROM_BUFFER(command_response_payload_buffer);

    // Invoke command.
    if (!invoke_getMaxMinReport(message_span, command_response_payload, &command_response_payload))
    {
      status = AZ_IOT_STATUS_BAD_REQUEST;
    }
    else
    {
      status = AZ_IOT_STATUS_OK;
    }
    IOT_SAMPLE_LOG_SUCCESS("Client invoked command 'getMaxMinReport'.");

    send_command_response(command_request, status, command_response_payload);
  }
  else
  {
    IOT_SAMPLE_LOG_AZ_SPAN("Command not supported:", command_request->command_name);
    send_command_response(command_request, AZ_IOT_STATUS_NOT_FOUND, command_empty_response_payload);
  }
}

// send_command_response sends a response to a command invoked by Azure IoT Hub
static void send_command_response(
    az_iot_hub_client_command_request const* command_request,
    az_iot_status status,
    az_span response)
{
  az_result rc;

  // Get the command response topic to publish the command response.
  char command_response_topic_buffer[SAMPLE_MQTT_TOPIC_LENGTH];
  rc = az_iot_hub_client_commands_response_get_publish_topic(
      &hub_client,
      command_request->request_id,
      (uint16_t)status,
      command_response_topic_buffer,
      sizeof(command_response_topic_buffer),
      NULL);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Failed to get the command response topic");

  // Publish the command response.
  publish_mqtt_message(command_response_topic_buffer, response, IOT_SAMPLE_MQTT_PUBLISH_QOS);
  IOT_SAMPLE_LOG_SUCCESS("Client published the Command response.");
  IOT_SAMPLE_LOG("Status: %d", status);
  IOT_SAMPLE_LOG_AZ_SPAN("Payload:", response);
}

// invoke_getMaxMinReport is called when the command "getMaxMinReport" arrives
// from the service.  It writes the response payload based on simulated
// temperature data.
static bool invoke_getMaxMinReport(az_span payload, az_span response, az_span* out_response)
{
  int32_t incoming_since_value_len = 0;

  // Parse the `since` field in the payload.
  char const* const log_message = "Failed to parse for `since` field in payload";

  az_json_reader jr;
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_reader_init(&jr, payload, NULL), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_reader_next_token(&jr), log_message);
  if (az_result_failed(az_json_token_get_string(
          &jr.token,
          command_start_time_value_buffer,
          sizeof(command_start_time_value_buffer),
          &incoming_since_value_len)))
  {
    *out_response = command_empty_response_payload;
    return false;
  }

  // Set the response payload to error if the `since` value was empty.
  if (incoming_since_value_len == 0)
  {
    *out_response = command_empty_response_payload;
    return false;
  }

  az_span start_time_span
      = az_span_create((uint8_t*)command_start_time_value_buffer, incoming_since_value_len);

  IOT_SAMPLE_LOG_AZ_SPAN("Start time:", start_time_span);

  // Get the current time as a string.
  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  size_t length = strftime(
      command_end_time_value_buffer,
      sizeof(command_end_time_value_buffer),
      iso_spec_time_format,
      timeinfo);
  az_span end_time_span = az_span_create((uint8_t*)command_end_time_value_buffer, (int32_t)length);

  IOT_SAMPLE_LOG_AZ_SPAN("End Time:", end_time_span);

  // Write command response message.
  uint8_t count = 3;
  az_span const names[3] = { command_max_temp_name, command_min_temp_name, command_avg_temp_name };
  double const values[3]
      = { device_maximum_temperature, device_minimum_temperature, device_average_temperature };
  az_span const times[2] = { start_time_span, end_time_span };

  write_property_payload(count, names, values, times, response, out_response);

  return true;
}

// send_telemetry_message builds the body of a telemetry message containing the current temperature
// and then sends it to Azure IoT Hub
static void send_telemetry_message(void)
{
  az_result rc;

  // Get the Telemetry topic to publish the telemetry message.
  char telemetry_topic_buffer[SAMPLE_MQTT_TOPIC_LENGTH];
  rc = az_iot_hub_client_telemetry_get_publish_topic(
      &hub_client, NULL, telemetry_topic_buffer, sizeof(telemetry_topic_buffer), NULL);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Failed to get the Telemetry topic");

  // Build the telemetry message.
  uint8_t count = 1;
  az_span const names[1] = { telemetry_temperature_name };
  double const values[1] = { device_current_temperature };

  char telemetry_payload_buffer[SAMPLE_MQTT_PAYLOAD_LENGTH];
  az_span telemetry_payload = AZ_SPAN_FROM_BUFFER(telemetry_payload_buffer);
  write_property_payload(count, names, values, NULL, telemetry_payload, &telemetry_payload);

  // Publish the telemetry message.
  publish_mqtt_message(telemetry_topic_buffer, telemetry_payload, IOT_SAMPLE_MQTT_PUBLISH_QOS);
  IOT_SAMPLE_LOG_SUCCESS("Client published the Telemetry message.");
  IOT_SAMPLE_LOG_AZ_SPAN("Payload:", telemetry_payload);
}

// write_property_payload writes a desired JSON payload.  The JSON built just needs to conform to
// the DTDLv2 that defined it.
static void write_property_payload(
    uint8_t property_count,
    az_span const names[],
    double const values[],
    az_span const times[],
    az_span property_payload,
    az_span* out_property_payload)
{
  char const* const log_message = "Failed to write property payload";

  az_json_writer jw;
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_init(&jw, property_payload, NULL), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_begin_object(&jw), log_message);

  for (uint8_t i = 0; i < property_count; i++)
  {
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_property_name(&jw, names[i]), log_message);
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(
        az_json_writer_append_double(&jw, values[i], DOUBLE_DECIMAL_PLACE_DIGITS), log_message);
  }

  if (times != NULL)
  {
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(
        az_json_writer_append_property_name(&jw, command_start_time_name), log_message);
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_string(&jw, times[0]), log_message);
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(
        az_json_writer_append_property_name(&jw, command_end_time_name), log_message);
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_string(&jw, times[1]), log_message);
  }

  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_end_object(&jw), log_message);
  *out_property_payload = az_json_writer_get_bytes_used_in_destination(&jw);
}

// write_property_payload_with_status writes a desired JSON status.  This payload is invoked
// in response to a desired property, e.g. when the device signals its availability to process
// a desired temperature request.
static void write_property_payload_with_status(
    az_span name,
    double value,
    int32_t status_code_value,
    int32_t version_value,
    az_span description_value,
    az_span property_payload,
    az_span* out_property_payload)
{
  char const* const log_message = "Failed to write property payload with status";

  az_json_writer jw;

  // First initialize the az_json_writer object, as in all JSON writes.
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_init(&jw, property_payload, NULL), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_begin_object(&jw), log_message);

  // Responding to a desired property requires additional metadata embedded in the JSON payload.
  // The az_iot_hub_client_properties_writer_begin_response_status will write this metadata
  // to the az_json_writer.
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_iot_hub_client_properties_writer_begin_response_status(
          &hub_client, &jw, name, status_code_value, version_value, description_value),
      log_message);

  // At this point the application writes the value of the desired property it is acknowledging.
  // This must conform to the DTDLv2 model definition that the device is implementing.
  // In this sample's case, the data we write is the double representing the desired temperature.
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_double(&jw, value, DOUBLE_DECIMAL_PLACE_DIGITS), log_message);

  // After writing the value, az_iot_hub_client_properties_writer_end_response_status is used
  // to close the property in JSON.
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_iot_hub_client_properties_writer_end_response_status(&hub_client, &jw), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_end_object(&jw), log_message);

  *out_property_payload = az_json_writer_get_bytes_used_in_destination(&jw);
}

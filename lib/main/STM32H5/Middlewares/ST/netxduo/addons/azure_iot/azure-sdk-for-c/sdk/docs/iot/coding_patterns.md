# Azure IoT Client Coding Patterns

## Introduction

This page introduces you to coding patterns that are MQTT stack agnostic. These examples will give you a general overview of the API calls and structure needed to use the Azure IoT Embedded C SDK features.

To view scenario-focused examples using the API calls, please view the [introductory examples](https://github.com/Azure/azure-sdk-for-c/blob/main/sdk/docs/iot/README.md#examples) on the Azure IoT Client README file.

For a more extensive demonstration of the API, please view and run the [sample code](https://github.com/Azure/azure-sdk-for-c/blob/main/sdk/samples/iot/), which uses Paho MQTT.

## Examples

### IoT Hub Client with MQTT Stack

Below is an implementation for using the IoT Hub Client SDK. This is meant to guide users in incorporating their MQTT stack with the IoT Hub Client SDK. Note for simplicity reasons, this code will not compile. Ideally, guiding principles can be inferred from reading through this snippet to create an IoT solution.

```C
#include <az/az_core.h>
#include <az/az_iot.h>

az_iot_hub_client my_client;
static az_span my_iothub_hostname = AZ_SPAN_LITERAL_FROM_STR("<your hub fqdn here>");
static az_span my_device_id = AZ_SPAN_LITERAL_FROM_STR("<your device id here>");

//Make sure the buffer is large enough to fit the user name (100 is an example)
static char my_mqtt_user_name[100];

//Make sure the buffer is large enough to fit the client id (16 is an example)
static char my_mqtt_client_id[16];

//This assumes an X509 Cert. SAS keys may also be used.
static const char my_device_cert[]= "-----BEGIN CERTIFICATE-----abcdefg-----END CERTIFICATE-----";

static char telemetry_topic[128];
static char telemetry_payload[] = "Hello World";

void handle_iot_message(mqtt_client_message* msg);

int main(void)
{
  //Get the default IoT Hub options
  az_iot_hub_client_options options = az_iot_hub_client_options_default();

  //Initialize the client with hostname, device id, and options
  az_iot_hub_client_init(&my_client, my_iothub_hostname, my_device_id, &options);

  //Get the MQTT user name to connect
  az_iot_hub_client_get_user_name(&my_client, my_mqtt_user_name,
                sizeof(my_mqtt_user_name), NULL);

  //Get the MQTT client id to connect
  az_iot_hub_client_get_client_id(&my_client, my_mqtt_client_id,
                sizeof(my_mqtt_client_id), NULL);

  //Initialize MQTT client with necessary parameters (example params shown)
  mqtt_client my_mqtt_client;
  mqtt_client_init(&my_mqtt_client, my_iothub_hostname, my_mqtt_client_id);

  //Subscribe to c2d messages
  mqtt_client_subscribe(&my_mqtt_client, AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC);

  //Subscribe to device methods
  mqtt_client_subscribe(&my_mqtt_client, AZ_IOT_HUB_CLIENT_METHODS_SUBSCRIBE_TOPIC);

  //Subscribe to twin patch topic
  mqtt_client_subscribe(&my_mqtt_client, AZ_IOT_HUB_CLIENT_TWIN_PATCH_SUBSCRIBE_TOPIC);

  //Subscribe to twin response topic
  mqtt_client_subscribe(&my_mqtt_client, AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_SUBSCRIBE_TOPIC);

  //Connect to the IoT Hub with your chosen mqtt stack
  mqtt_client_connect(&my_mqtt_client, my_mqtt_user_name, my_device_cert);

  //This example would run to receive any incoming message and send a telemetry message five times
  int iterations = 0;
  mqtt_client_message msg;
  while(iterations++ < 5)
  {
    if(mqtt_client_receive(&msg))
    {
      handle_iot_message(&msg);
    }

    send_telemetry_message();
  }

  //Disconnect from the IoT Hub
  mqtt_client_disconnect(&my_mqtt_client);

  //Destroy the mqtt client
  mqtt_client_destroy(&my_mqtt_client);
}

void send_telemetry_message(void)
{
  //Get the topic to send a telemetry message
  az_iot_hub_client_telemetry_get_publish_topic(&client, NULL, telemetry_topic, sizeof(telemetry_topic), NULL);

  //Send the telemetry message with the MQTT client
  mqtt_client_publish(telemetry_topic, telemetry_payload, AZ_HUB_CLIENT_DEFAULT_MQTT_TELEMETRY_QOS);
}

void handle_iot_message(mqtt_client_message* msg)
{
  //Initialize the incoming topic to a span
  az_span incoming_topic = az_span_create(msg->topic, msg->topic_len);

  //The message could be for three features so parse the topic to see which it is for
  az_iot_hub_client_method_request method_request;
  az_iot_hub_client_c2d_request c2d_request;
  az_iot_hub_client_twin_response twin_response;
  if (az_result_succeeded(az_iot_hub_client_methods_parse_received_topic(&client, incoming_topic, &method_request)))
  {
    //Handle the method request
  }
  else if (az_result_succeeded(az_iot_hub_client_c2d_parse_received_topic(&client, incoming_topic, &c2d_request)))
  {
    //Handle the c2d message
  }
  else if (az_result_succeeded(az_iot_hub_client_twin_parse_received_topic(&client, incoming_topic, &twin_response)))
  {
    //Handle the twin message
  }
}

```

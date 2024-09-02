// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <string.h>
#include <stdbool.h>
#include <NTPClient.h>
#include <sntp/sntp.h>

#include <cstdlib>

#include <WiFi.h>
#include <PubSubClient.h>

#include <mbedtls/base64.h>
#include <mbedtls/sha256.h>

#include <az_result.h>
#include <az_span.h>
#include <az_iot_hub_client.h>

#include "iot_configs.h"
#include "ca.h"

// Status LED: will remain high on error and pulled high for a short time for each successful send.
#define LED_PIN 2
#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))
#define MQTT_PACKET_SIZE 1024

static const char* ssid = IOT_CONFIG_WIFI_SSID;
static const char* password = IOT_CONFIG_WIFI_PASSWORD;
static const char* host = IOT_CONFIG_IOTHUB_FQDN;
static const int mqtt_port = 8883;

static WiFiUDP ntp_udp_client;
static WiFiSSLClient wifi_client;
static PubSubClient mqtt_client(wifi_client);
static az_iot_hub_client hub_client;
static char sas_token[200];
static size_t sas_token_length;

static uint8_t signature[512];
static unsigned long next_telemetry_send_time_ms = 0;
static char telemetry_topic[128];
static uint8_t telemetry_payload[100];
static uint32_t telemetry_send_count = 0;
static unsigned char* ca_pem_nullterm;

extern "C"{
  extern int mbedtls_base64_decode( unsigned char *dst, size_t dlen, size_t *olen, const unsigned char *src, size_t slen);
  extern int rom_hmac_sha256(const u8 *key, size_t key_len, const u8 *data, size_t data_len, u8 *mac);
  extern int mbedtls_base64_encode( unsigned char *dst, size_t dlen, size_t *olen, const unsigned char *src, size_t slen);
}
static void createNullTerminatedRootCert()
{
  ca_pem_nullterm = (unsigned char*)malloc(ca_pem_len + 1);

  if (ca_pem_nullterm == NULL)
  {
    Serial.println("Failed allocating memory for null-terminated root ca");
  }
  else
  {
    memcpy(ca_pem_nullterm, ca_pem, ca_pem_len);
    ca_pem_nullterm[ca_pem_len] = '\0';
  }
}


static void connectToWiFi()
{
  Serial.begin(115200);
  Serial.println();

  Serial.print("Connecting to WIFI SSID ");
  Serial.println(ssid);

  WiFi.begin((char*)ssid, (char*)password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void receivedCallback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println("");
}

static void initializeClients()
{
  Serial.println("Initializing MQTT client");
  
  wifi_client.setRootCA(ca_pem_nullterm);

  if (az_result_failed(az_iot_hub_client_init(
          &hub_client,
          az_span_create((uint8_t*)host, strlen(host)),
          AZ_SPAN_FROM_STR(IOT_CONFIG_DEVICE_ID),
          NULL)))
  {
    Serial.println("Failed initializing Azure IoT Hub client");
    return;
  }

  mqtt_client.setServer(host, mqtt_port);
  mqtt_client.setCallback(receivedCallback);

  Serial.println("MQTT client initialized");
}

int64_t iot_sample_get_epoch_expiration_time_from_minutes(uint32_t minutes)
{

  long ts = 0;
  long tus = 0;
  unsigned int ttk = 0;

  //it should be ok to do init more than one time. It'll handle inside sntp_init().
  sntp_init();

  sntp_get_lasttime(&ts, &tus, &ttk);
  
  while(ts == 0){
    vTaskDelay(1000 / portTICK_RATE_MS);
    sntp_get_lasttime(&ts, &tus, &ttk);
  } 

  return (int64_t)ts + minutes * 60;
}

static void hmac_sha256_sign_signature(
    az_span decoded_key,
    az_span signature,
    az_span signed_signature,
    az_span* out_signed_signature)
{ 
  if(rom_hmac_sha256(
    az_span_ptr(decoded_key), 
    (size_t)az_span_size(decoded_key),
    az_span_ptr(signature),
    (size_t)az_span_size(signature),
    az_span_ptr(signed_signature)) != 0)

  {
    Serial.println("[ERROR] rom_hmac_sha256 failed");
  }

  *out_signed_signature = az_span_create(az_span_ptr(signed_signature), 32);
}

static void base64_encode_bytes(
    az_span decoded_bytes,
    az_span base64_encoded_bytes,
    az_span* out_base64_encoded_bytes)
{
  size_t len;
  if(mbedtls_base64_encode(az_span_ptr(base64_encoded_bytes), (size_t)az_span_size(base64_encoded_bytes), 
    &len, az_span_ptr(decoded_bytes), (size_t)az_span_size(decoded_bytes)) != 0)
  {
    Serial.println("[ERROR] mbedtls_base64_encode fail");
  }
  
  *out_base64_encoded_bytes = az_span_create(az_span_ptr(base64_encoded_bytes), (int32_t)len);
}

static void decode_base64_bytes(
    az_span base64_encoded_bytes,
    az_span decoded_bytes,
    az_span* out_decoded_bytes)
{
  
  memset(az_span_ptr(decoded_bytes), 0, (size_t)az_span_size(decoded_bytes));

  size_t len;
  if( mbedtls_base64_decode( az_span_ptr(decoded_bytes), (size_t)az_span_size(decoded_bytes), 
      &len, az_span_ptr(base64_encoded_bytes), (size_t)az_span_size(base64_encoded_bytes)) != 0)
  {
    Serial.println("[ERROR] mbedtls_base64_decode fail");
  }
  
  *out_decoded_bytes = az_span_create(az_span_ptr(decoded_bytes), (int32_t)len);
}

static void iot_sample_generate_sas_base64_encoded_signed_signature(
    az_span sas_base64_encoded_key,
    az_span sas_signature,
    az_span sas_base64_encoded_signed_signature,
    az_span* out_sas_base64_encoded_signed_signature)
{
  // Decode the sas base64 encoded key to use for HMAC signing.
  char sas_decoded_key_buffer[32];
  az_span sas_decoded_key = AZ_SPAN_FROM_BUFFER(sas_decoded_key_buffer);
  decode_base64_bytes(sas_base64_encoded_key, sas_decoded_key, &sas_decoded_key);

  // HMAC-SHA256 sign the signature with the decoded key.
  char sas_hmac256_signed_signature_buffer[32];
  az_span sas_hmac256_signed_signature = AZ_SPAN_FROM_BUFFER(sas_hmac256_signed_signature_buffer);
  hmac_sha256_sign_signature(sas_decoded_key, sas_signature, sas_hmac256_signed_signature, &sas_hmac256_signed_signature);

  // Base64 encode the result of the HMAC signing.
  base64_encode_bytes(
    sas_hmac256_signed_signature,
    sas_base64_encoded_signed_signature,
    out_sas_base64_encoded_signed_signature);
}

static void generate_sas_key(void)
{
  az_result rc;
  // Create the POSIX expiration time from input minutes.
  uint64_t sas_duration = iot_sample_get_epoch_expiration_time_from_minutes(SAS_TOKEN_EXPIRY_IN_MINUTES);


  // Get the signature that will later be signed with the decoded key.
  az_span sas_signature = AZ_SPAN_FROM_BUFFER(signature);
  rc = az_iot_hub_client_sas_get_signature(
    &hub_client, sas_duration, sas_signature, &sas_signature);
  if (az_result_failed(rc))
  {
    Serial.print("Could not get the signature for SAS key: az_result return code ");
    Serial.println(rc);
  }

  // Generate the encoded, signed signature (b64 encoded, HMAC-SHA256 signing).
  char b64enc_hmacsha256_signature[64];
  az_span sas_base64_encoded_signed_signature = AZ_SPAN_FROM_BUFFER(b64enc_hmacsha256_signature);
  iot_sample_generate_sas_base64_encoded_signed_signature(
    AZ_SPAN_FROM_STR(IOT_CONFIG_DEVICE_KEY),
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
    sas_token,
    sizeof(sas_token),
    &sas_token_length);
  if (az_result_failed(rc))
  {
    Serial.print("Could not get the password: az_result return code ");
    Serial.println(rc);
  }
}

static int connect_to_azure_iot_hub()
{
  size_t client_id_length;
  char mqtt_client_id[128];
  if (az_result_failed(az_iot_hub_client_get_client_id(
          &hub_client, mqtt_client_id, sizeof(mqtt_client_id) - 1, &client_id_length)))
  {
    Serial.println("[ERROR] Failed getting client id");
    return 1;
  }

  char mqtt_username[128];
  // Get the MQTT user name used to connect to IoT Hub
  if (az_result_failed(az_iot_hub_client_get_user_name(
          &hub_client, mqtt_username, sizeofarray(mqtt_username), NULL)))
  {
    printf("[ERROR] Failed to get MQTT clientId, return code\n");
    return 1;
  }

  Serial.print("Client ID: ");
  Serial.println(mqtt_client_id);

  Serial.print("Username: ");
  Serial.println(mqtt_username);

  while (!mqtt_client.connected())
  {
    Serial.print("MQTT connecting ... ");

    if (mqtt_client.connect(mqtt_client_id, mqtt_username, sas_token))
    {
      Serial.println("connected.");
    }
    else
    {
      Serial.print("[ERROR] failed, status code =");
      Serial.print(mqtt_client.state());
      Serial.println(". Trying again in 5 seconds.");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }

  mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC);

  return 0;
}

void establishConnection() 
{
  connectToWiFi();

  initializeClients();

  generate_sas_key();

  connect_to_azure_iot_hub();

  digitalWrite(LED_PIN, LOW);
}

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  createNullTerminatedRootCert();
  establishConnection();
}

static char* get_telemetry_payload()
{
  az_span temp_span = az_span_create(telemetry_payload, sizeof(telemetry_payload));
  temp_span = az_span_copy(temp_span, AZ_SPAN_FROM_STR("{ \"msgCount\": "));
  (void)az_span_u32toa(temp_span, telemetry_send_count++, &temp_span);  
  temp_span = az_span_copy(temp_span, AZ_SPAN_FROM_STR(" }"));
  temp_span = az_span_copy_u8(temp_span, '\0');

  return (char*)telemetry_payload;
}

static void send_telemetry()
{
  digitalWrite(LED_PIN, HIGH);
  Serial.print(millis());
  Serial.print(" Realtek Ameba-D Sending telemetry . . . ");
  if (az_result_failed(az_iot_hub_client_telemetry_get_publish_topic(
          &hub_client, NULL, telemetry_topic, sizeof(telemetry_topic), NULL)))
  {
    Serial.println("Failed az_iot_hub_client_telemetry_get_publish_topic");
    return;
  }

  mqtt_client.publish(telemetry_topic, get_telemetry_payload(), false);
  Serial.println("OK");
  delay(100);
  digitalWrite(LED_PIN, LOW);
}

void loop()
{
  if (millis() > next_telemetry_send_time_ms)
  {
    // Check if connected, reconnect if needed.
    if(!mqtt_client.connected())
    {
      establishConnection();
    }

    send_telemetry();
    next_telemetry_send_time_ms = millis() + TELEMETRY_FREQUENCY_MILLISECS;
  }

  // MQTT loop must be called to process Device-to-Cloud and Cloud-to-Device.
  mqtt_client.loop();
  delay(500);
}

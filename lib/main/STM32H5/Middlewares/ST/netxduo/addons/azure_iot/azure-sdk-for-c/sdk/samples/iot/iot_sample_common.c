// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifdef _MSC_VER
// warning C4996: 'getenv': This function or variable may be unsafe. Consider using _dupenv_s
// instead.
#pragma warning(disable : 4996)
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
// Required for Sleep(DWORD)
#include <Windows.h>
#else
// Required for sleep(unsigned int)
#include <unistd.h>
#endif

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <azure/az_core.h>
#include <azure/az_iot.h>

#include "iot_sample_common.h"

#define IOT_SAMPLE_PRECONDITION_NOT_NULL(arg)   \
  do                                            \
  {                                             \
    if ((arg) == NULL)                          \
    {                                           \
      IOT_SAMPLE_LOG_ERROR("Pointer is NULL."); \
      exit(1);                                  \
    }                                           \
  } while (0)

static char iot_sample_hub_hostname_buffer[128];
static char iot_sample_provisioning_id_scope_buffer[16];

static char iot_sample_hub_device_id_buffer[64];
static char iot_sample_provisioning_registration_id_buffer[256];

static char iot_sample_hub_sas_key_buffer[128];
static char iot_sample_provisioning_sas_key_buffer[128];

static char iot_sample_x509_cert_pem_file_path_buffer[256];
static char iot_sample_x509_trust_pem_file_path_buffer[256];

//
// MQTT endpoints
//
//#define USE_WEB_SOCKET // Comment to use MQTT without WebSockets.
#ifdef USE_WEB_SOCKET
static az_span const mqtt_url_prefix = AZ_SPAN_LITERAL_FROM_STR("wss://");
// Note: Paho fails to connect to Hub when using AZ_IOT_HUB_CLIENT_WEB_SOCKET_PATH or an X509
// certificate.
static az_span const mqtt_url_suffix
    = AZ_SPAN_LITERAL_FROM_STR(":443" AZ_IOT_HUB_CLIENT_WEB_SOCKET_PATH_NO_X509_CLIENT_CERT);
#else
static az_span const mqtt_url_prefix = AZ_SPAN_LITERAL_FROM_STR("ssl://");
static az_span const mqtt_url_suffix = AZ_SPAN_LITERAL_FROM_STR(":8883");
#endif
static az_span const provisioning_global_endpoint
    = AZ_SPAN_LITERAL_FROM_STR("ssl://global.azure-devices-provisioning.net:8883");

//
// Functions
//
void build_error_message(
    char* out_full_message,
    size_t full_message_buf_size,
    char const* const error_message,
    ...)
{
  char const* const append_message = ": az_result return code 0x%08x.";

  size_t message_len = strlen(error_message) + 1;
  strncpy(out_full_message, error_message, full_message_buf_size);
  out_full_message[full_message_buf_size - 1] = 0;
  if (full_message_buf_size > message_len)
  {
    strncat(out_full_message, append_message, full_message_buf_size - message_len);
    out_full_message[full_message_buf_size - 1] = 0;
  }
}

bool get_az_span(az_span* out_span, char const* const error_message, ...)
{
  va_list args;
  va_start(args, error_message);

  *out_span = va_arg(args, az_span);
  va_end(args);

  if (az_span_size(*out_span) == 0) // There was no span
  {
    return false;
  }

  return true;
}

static void read_configuration_entry(
    char const* env_name,
    char* default_value,
    bool show_value,
    az_span destination,
    az_span* out_env_value)
{
  char* env_value = getenv(env_name);

  if (env_value == NULL && default_value != NULL)
  {
    env_value = default_value;
  }

  if (env_value != NULL)
  {
    (void)printf("%s = %s\n", env_name, show_value ? env_value : "***");
    az_span env_span = az_span_create_from_str(env_value);

    // Check the buffer is large enough to store the environment variable.
    if ((az_span_size(destination) < az_span_size(env_span)) || (az_span_size(env_span) < 0))
    {
      IOT_SAMPLE_LOG_ERROR(
          "Failed to read configuration from environment variables: Buffer is too small.");
      exit(1);
    }

    az_span_copy(destination, env_span);
    *out_env_value = az_span_slice(destination, 0, az_span_size(env_span));
  }
  else
  {
    IOT_SAMPLE_LOG_ERROR(
        "Failed to read configuration from environment variables: Environment variable %s not set.",
        env_name);
    exit(1);
  }
}

void iot_sample_read_environment_variables(
    iot_sample_type type,
    iot_sample_name name,
    iot_sample_environment_variables* out_env_vars)
{
  IOT_SAMPLE_PRECONDITION_NOT_NULL(out_env_vars);

  bool show_value = true;

  if (type == PAHO_IOT_HUB)
  {
    out_env_vars->hub_hostname = AZ_SPAN_FROM_BUFFER(iot_sample_hub_hostname_buffer);
    read_configuration_entry(
        IOT_SAMPLE_ENV_HUB_HOSTNAME,
        NULL,
        show_value,
        out_env_vars->hub_hostname,
        &(out_env_vars->hub_hostname));

    switch (name)
    {
      case PAHO_IOT_HUB_C2D_SAMPLE:
      case PAHO_IOT_HUB_METHODS_SAMPLE:
      case PAHO_IOT_HUB_TELEMETRY_SAMPLE:
      case PAHO_IOT_HUB_TWIN_SAMPLE:
      case PAHO_IOT_PNP_SAMPLE:
      case PAHO_IOT_PNP_COMPONENT_SAMPLE:
        out_env_vars->hub_device_id = AZ_SPAN_FROM_BUFFER(iot_sample_hub_device_id_buffer);
        read_configuration_entry(
            IOT_SAMPLE_ENV_HUB_DEVICE_ID,
            NULL,
            show_value,
            out_env_vars->hub_device_id,
            &(out_env_vars->hub_device_id));

        out_env_vars->x509_cert_pem_file_path
            = AZ_SPAN_FROM_BUFFER(iot_sample_x509_cert_pem_file_path_buffer);
        read_configuration_entry(
            IOT_SAMPLE_ENV_DEVICE_X509_CERT_PEM_FILE_PATH,
            NULL,
            show_value,
            out_env_vars->x509_cert_pem_file_path,
            &(out_env_vars->x509_cert_pem_file_path));
        break;

      case PAHO_IOT_HUB_SAS_TELEMETRY_SAMPLE:
        out_env_vars->hub_device_id = AZ_SPAN_FROM_BUFFER(iot_sample_hub_device_id_buffer);
        read_configuration_entry(
            IOT_SAMPLE_ENV_HUB_SAS_DEVICE_ID,
            NULL,
            show_value,
            out_env_vars->hub_device_id,
            &(out_env_vars->hub_device_id));

        out_env_vars->hub_sas_key = AZ_SPAN_FROM_BUFFER(iot_sample_hub_sas_key_buffer);
        read_configuration_entry(
            IOT_SAMPLE_ENV_HUB_SAS_KEY,
            NULL,
            !show_value,
            out_env_vars->hub_sas_key,
            &(out_env_vars->hub_sas_key));

        char duration_buffer[IOT_SAMPLE_SAS_KEY_DURATION_TIME_DIGITS];
        az_span duration = AZ_SPAN_FROM_BUFFER(duration_buffer);
        read_configuration_entry(
            IOT_SAMPLE_ENV_SAS_KEY_DURATION_MINUTES, "120", show_value, duration, &duration);

        az_result rc = az_span_atou32(duration, &(out_env_vars->sas_key_duration_minutes));
        if (az_result_failed(rc))
        {
          IOT_SAMPLE_LOG_ERROR(
              "Failed to read environment variables: az_result return code 0x%08x.", rc);
          exit(rc);
        }
        break;

      default:
        IOT_SAMPLE_LOG_ERROR("Failed to read environment variables: Hub sample name undefined.");
        exit(1);
    }
  }
  else if (type == PAHO_IOT_PROVISIONING)
  {
    out_env_vars->provisioning_id_scope
        = AZ_SPAN_FROM_BUFFER(iot_sample_provisioning_id_scope_buffer);
    read_configuration_entry(
        IOT_SAMPLE_ENV_PROVISIONING_ID_SCOPE,
        NULL,
        show_value,
        out_env_vars->provisioning_id_scope,
        &(out_env_vars->provisioning_id_scope));

    switch (name)
    {
      case PAHO_IOT_PNP_WITH_PROVISIONING_SAMPLE:
      case PAHO_IOT_PROVISIONING_SAMPLE:
        out_env_vars->provisioning_registration_id
            = AZ_SPAN_FROM_BUFFER(iot_sample_provisioning_registration_id_buffer);
        read_configuration_entry(
            IOT_SAMPLE_ENV_PROVISIONING_REGISTRATION_ID,
            NULL,
            show_value,
            out_env_vars->provisioning_registration_id,
            &(out_env_vars->provisioning_registration_id));

        out_env_vars->x509_cert_pem_file_path
            = AZ_SPAN_FROM_BUFFER(iot_sample_x509_cert_pem_file_path_buffer);
        read_configuration_entry(
            IOT_SAMPLE_ENV_DEVICE_X509_CERT_PEM_FILE_PATH,
            NULL,
            show_value,
            out_env_vars->x509_cert_pem_file_path,
            &(out_env_vars->x509_cert_pem_file_path));
        break;

      case PAHO_IOT_PROVISIONING_SAS_SAMPLE:
        out_env_vars->provisioning_registration_id
            = AZ_SPAN_FROM_BUFFER(iot_sample_provisioning_registration_id_buffer);
        read_configuration_entry(
            IOT_SAMPLE_ENV_PROVISIONING_SAS_REGISTRATION_ID,
            NULL,
            show_value,
            out_env_vars->provisioning_registration_id,
            &(out_env_vars->provisioning_registration_id));

        out_env_vars->provisioning_sas_key
            = AZ_SPAN_FROM_BUFFER(iot_sample_provisioning_sas_key_buffer);
        read_configuration_entry(
            IOT_SAMPLE_ENV_PROVISIONING_SAS_KEY,
            NULL,
            !show_value,
            out_env_vars->provisioning_sas_key,
            &(out_env_vars->provisioning_sas_key));

        char duration_buffer[IOT_SAMPLE_SAS_KEY_DURATION_TIME_DIGITS];
        az_span duration = AZ_SPAN_FROM_BUFFER(duration_buffer);
        read_configuration_entry(
            IOT_SAMPLE_ENV_SAS_KEY_DURATION_MINUTES, "120", show_value, duration, &duration);

        az_result rc = az_span_atou32(duration, &(out_env_vars->sas_key_duration_minutes));
        if (az_result_failed(rc))
        {
          IOT_SAMPLE_LOG_ERROR(
              "Failed to read environment variables: az_result return code 0x%08x.", rc);
          exit(rc);
        }
        break;

      default:
        IOT_SAMPLE_LOG_ERROR(
            "Failed to read environment variables: Provisioning sample name undefined.");
        exit(1);
    }
  }
  else
  {
    IOT_SAMPLE_LOG_ERROR("Failed to read environment variables: Sample type undefined.");
    exit(1);
  }

  out_env_vars->x509_trust_pem_file_path
      = AZ_SPAN_FROM_BUFFER(iot_sample_x509_trust_pem_file_path_buffer);
  read_configuration_entry(
      IOT_SAMPLE_ENV_DEVICE_X509_TRUST_PEM_FILE_PATH,
      "",
      show_value,
      out_env_vars->x509_trust_pem_file_path,
      &(out_env_vars->x509_trust_pem_file_path));

  IOT_SAMPLE_LOG(" "); // Formatting
}

void iot_sample_create_mqtt_endpoint(
    iot_sample_type type,
    iot_sample_environment_variables const* env_vars,
    char* out_endpoint,
    size_t endpoint_size)
{
  IOT_SAMPLE_PRECONDITION_NOT_NULL(env_vars);
  IOT_SAMPLE_PRECONDITION_NOT_NULL(out_endpoint);

  if (type == PAHO_IOT_HUB)
  {
    int32_t const required_size = az_span_size(mqtt_url_prefix)
        + az_span_size(env_vars->hub_hostname) + az_span_size(mqtt_url_suffix)
        + (int32_t)sizeof('\0');

    if ((size_t)required_size > endpoint_size)
    {
      IOT_SAMPLE_LOG_ERROR("Failed to create MQTT endpoint: Buffer is too small.");
      exit(1);
    }

    az_span hub_mqtt_endpoint = az_span_create((uint8_t*)out_endpoint, (int32_t)endpoint_size);
    az_span remainder = az_span_copy(hub_mqtt_endpoint, mqtt_url_prefix);
    remainder = az_span_copy(remainder, env_vars->hub_hostname);
    remainder = az_span_copy(remainder, mqtt_url_suffix);
    az_span_copy_u8(remainder, '\0');
  }
  else if (type == PAHO_IOT_PROVISIONING)
  {
    int32_t const required_size
        = az_span_size(provisioning_global_endpoint) + (int32_t)sizeof('\0');

    if ((size_t)required_size > endpoint_size)
    {
      IOT_SAMPLE_LOG_ERROR("Failed to create MQTT endpoint: Buffer is too small.");
      exit(1);
    }

    az_span provisioning_mqtt_endpoint
        = az_span_create((uint8_t*)out_endpoint, (int32_t)endpoint_size);
    az_span remainder = az_span_copy(provisioning_mqtt_endpoint, provisioning_global_endpoint);
    az_span_copy_u8(remainder, '\0');
  }
  else
  {
    IOT_SAMPLE_LOG_ERROR("Failed to create MQTT endpoint: Sample type undefined.");
    exit(1);
  }

  IOT_SAMPLE_LOG_SUCCESS("MQTT endpoint created at \"%s\".", out_endpoint);
}

void iot_sample_sleep_for_seconds(uint32_t seconds)
{
#ifdef _WIN32
  Sleep((DWORD)seconds * 1000);
#else
  sleep(seconds);
#endif
}

uint32_t iot_sample_get_epoch_expiration_time_from_minutes(uint32_t minutes)
{
  return (uint32_t)(time(NULL) + minutes * 60);
}

static void decode_base64_bytes(
    az_span base64_encoded_bytes,
    az_span decoded_bytes,
    az_span* out_decoded_bytes)
{
  BIO* base64_decoder;
  BIO* source_mem_bio;

  memset(az_span_ptr(decoded_bytes), 0, (size_t)az_span_size(decoded_bytes));

  // Create a BIO filter to process the bytes.
  base64_decoder = BIO_new(BIO_f_base64());
  if (base64_decoder == NULL)
  {
    IOT_SAMPLE_LOG_ERROR("Could not decode the SAS key: Failed to create BIO.");
    exit(1);
  }

  // Get the source BIO to push through the filter.
  source_mem_bio
      = BIO_new_mem_buf(az_span_ptr(base64_encoded_bytes), (int)az_span_size(base64_encoded_bytes));
  if (source_mem_bio == NULL)
  {
    BIO_free(base64_decoder);
    IOT_SAMPLE_LOG_ERROR("Could not decode the SAS key: Failed to create BIO new memory buffer.");
    exit(1);
  }

  // Push the memory through the filter.
  source_mem_bio = BIO_push(base64_decoder, source_mem_bio);
  if (source_mem_bio == NULL)
  {
    BIO_free(base64_decoder);
    BIO_free(source_mem_bio);
    IOT_SAMPLE_LOG_ERROR("Could not decode the SAS key: Failed to push memory through filter.");
    exit(1);
  }

  // Set flags to not have a newline and close the BIO.
  BIO_set_flags(source_mem_bio, BIO_FLAGS_BASE64_NO_NL);
  BIO_set_close(source_mem_bio, BIO_CLOSE);

  // Read the memory which was pushed through the filter.
  int read_data = BIO_read(source_mem_bio, az_span_ptr(decoded_bytes), az_span_size(decoded_bytes));

  // Set the output span.
  if (read_data > 0)
  {
    *out_decoded_bytes = az_span_create(az_span_ptr(decoded_bytes), (int32_t)read_data);
  }
  else
  {
    IOT_SAMPLE_LOG_ERROR("Could not decode the SAS key: Data could not be read from BIO.");
    exit(1);
  }

  // Free the BIO chain.
  BIO_free_all(source_mem_bio);
}

static void hmac_sha256_sign_signature(
    az_span decoded_key,
    az_span signature,
    az_span signed_signature,
    az_span* out_signed_signature)
{
  unsigned int hmac_encode_len;
  unsigned char const* hmac = HMAC(
      EVP_sha256(),
      (void*)az_span_ptr(decoded_key),
      az_span_size(decoded_key),
      az_span_ptr(signature),
      (size_t)az_span_size(signature),
      az_span_ptr(signed_signature),
      &hmac_encode_len);

  if (hmac != NULL)
  {
    *out_signed_signature = az_span_create(az_span_ptr(signed_signature), (int32_t)hmac_encode_len);
  }
  else
  {
    IOT_SAMPLE_LOG_ERROR("Could not sign the signature: Buffer is too small.");
    exit(1);
  }
}

static void base64_encode_bytes(
    az_span decoded_bytes,
    az_span base64_encoded_bytes,
    az_span* out_base64_encoded_bytes)
{
  BIO* base64_encoder;
  BIO* sink_mem_bio;
  BUF_MEM* encoded_mem_ptr;

  // Create a BIO filter to process the bytes.
  base64_encoder = BIO_new(BIO_f_base64());
  if (base64_encoder == NULL)
  {
    IOT_SAMPLE_LOG_ERROR("Could not base64 encode the password: Failed to create BIO.");
    exit(1);
  }

  // Create a memory sink BIO to process bytes to.
  sink_mem_bio = BIO_new(BIO_s_mem());
  if (sink_mem_bio == NULL)
  {
    BIO_free(base64_encoder);
    IOT_SAMPLE_LOG_ERROR("Could not base64 encode the password: Failed to create BIO.");
    exit(1);
  }

  // Push the sink to the encoder.
  base64_encoder = BIO_push(base64_encoder, sink_mem_bio);
  if (base64_encoder == NULL)
  {
    BIO_free(sink_mem_bio);
    BIO_free(base64_encoder);
    IOT_SAMPLE_LOG_ERROR(
        "Could not base64 encode the password: Failed to push memory through filter.");
    exit(1);
  }

  // Set no newline flag for the encoder.
  BIO_set_flags(base64_encoder, BIO_FLAGS_BASE64_NO_NL);

  // Write the bytes to be encoded.
  int const bytes_written
      = BIO_write(base64_encoder, az_span_ptr(decoded_bytes), (int)az_span_size(decoded_bytes));
  if (bytes_written < 1)
  {
    BIO_free(sink_mem_bio);
    BIO_free(base64_encoder);
    IOT_SAMPLE_LOG_ERROR("Could not base64 encode the password: Failed to write bytes.");
    exit(1);
  }

  // Flush the BIO
  BIO_flush(base64_encoder);

  // Get the pointer to the encoded bytes.
  BIO_get_mem_ptr(base64_encoder, &encoded_mem_ptr);

  if ((size_t)az_span_size(base64_encoded_bytes) >= encoded_mem_ptr->length)
  {
    // Copy the bytes to the output and initialize output span.
    memcpy(az_span_ptr(base64_encoded_bytes), encoded_mem_ptr->data, encoded_mem_ptr->length);
    *out_base64_encoded_bytes
        = az_span_create(az_span_ptr(base64_encoded_bytes), (int32_t)encoded_mem_ptr->length);
  }
  else
  {
    IOT_SAMPLE_LOG_ERROR("Could not base64 encode the password: Buffer is too small.");
    exit(1);
  }

  // Free the BIO chain.
  BIO_free_all(base64_encoder);
}

void iot_sample_generate_sas_base64_encoded_signed_signature(
    az_span sas_base64_encoded_key,
    az_span sas_signature,
    az_span sas_base64_encoded_signed_signature,
    az_span* out_sas_base64_encoded_signed_signature)
{
  IOT_SAMPLE_PRECONDITION_NOT_NULL(out_sas_base64_encoded_signed_signature);

  // Decode the sas base64 encoded key to use for HMAC signing.
  char sas_decoded_key_buffer[64];
  az_span sas_decoded_key = AZ_SPAN_FROM_BUFFER(sas_decoded_key_buffer);
  decode_base64_bytes(sas_base64_encoded_key, sas_decoded_key, &sas_decoded_key);

  // HMAC-SHA256 sign the signature with the decoded key.
  char sas_hmac256_signed_signature_buffer[128];
  az_span sas_hmac256_signed_signature = AZ_SPAN_FROM_BUFFER(sas_hmac256_signed_signature_buffer);
  hmac_sha256_sign_signature(
      sas_decoded_key, sas_signature, sas_hmac256_signed_signature, &sas_hmac256_signed_signature);

  // Base64 encode the result of the HMAC signing.
  base64_encode_bytes(
      sas_hmac256_signed_signature,
      sas_base64_encoded_signed_signature,
      out_sas_base64_encoded_signed_signature);
}

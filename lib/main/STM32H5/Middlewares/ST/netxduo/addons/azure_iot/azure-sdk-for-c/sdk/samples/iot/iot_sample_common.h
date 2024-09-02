// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef IOT_SAMPLE_COMMON_H
#define IOT_SAMPLE_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <azure/az_core.h>

#define IOT_SAMPLE_SAS_KEY_DURATION_TIME_DIGITS 4
#define IOT_SAMPLE_MQTT_PUBLISH_QOS 0
#define IOT_SAMPLE_MQTT_SUBSCRIBE_QOS 1

//
// Logging
//
#define IOT_SAMPLE_LOG_ERROR(...)                                                  \
  do                                                                               \
  {                                                                                \
    (void)fprintf(stderr, "ERROR:\t\t%s:%s():%d: ", __FILE__, __func__, __LINE__); \
    (void)fprintf(stderr, __VA_ARGS__);                                            \
    (void)fprintf(stderr, "\n");                                                   \
    fflush(stdout);                                                                \
    fflush(stderr);                                                                \
  } while (0)

#define IOT_SAMPLE_LOG_SUCCESS(...) \
  do                                \
  {                                 \
    (void)printf("SUCCESS:\t");     \
    (void)printf(__VA_ARGS__);      \
    (void)printf("\n");             \
  } while (0)

#define IOT_SAMPLE_LOG(...)    \
  do                           \
  {                            \
    (void)printf("\t\t");      \
    (void)printf(__VA_ARGS__); \
    (void)printf("\n");        \
  } while (0)

#define IOT_SAMPLE_LOG_AZ_SPAN(span_description, span)                                           \
  do                                                                                             \
  {                                                                                              \
    (void)printf("\t\t%s ", span_description);                                                   \
    (void)fwrite((char*)az_span_ptr(span), sizeof(uint8_t), (size_t)az_span_size(span), stdout); \
    (void)printf("\n");                                                                          \
  } while (0)

//
// Error handling
//
// Note: Only handles a single variadic parameter of type char const*, or two variadic parameters of
// type char const* and az_span.
void build_error_message(
    char* out_full_message,
    size_t full_message_buf_size,
    char const* const error_message,
    ...);
bool get_az_span(az_span* out_span, char const* const error_message, ...);
#define IOT_SAMPLE_EXIT_IF_AZ_FAILED(azfn, ...)                                            \
  do                                                                                       \
  {                                                                                        \
    az_result const result = (azfn);                                                       \
                                                                                           \
    if (az_result_failed(result))                                                          \
    {                                                                                      \
      char full_message[256];                                                              \
      build_error_message(full_message, sizeof(full_message), __VA_ARGS__);                \
                                                                                           \
      az_span span;                                                                        \
      bool has_az_span = get_az_span(&span, __VA_ARGS__, AZ_SPAN_EMPTY);                   \
      if (has_az_span)                                                                     \
      {                                                                                    \
        IOT_SAMPLE_LOG_ERROR(full_message, az_span_size(span), az_span_ptr(span), result); \
      }                                                                                    \
      else                                                                                 \
      {                                                                                    \
        IOT_SAMPLE_LOG_ERROR(full_message, result);                                        \
      }                                                                                    \
      exit(1);                                                                             \
    }                                                                                      \
  } while (0)

//
// Environment Variables
//
// DO NOT MODIFY: Service information
#define IOT_SAMPLE_ENV_HUB_HOSTNAME "AZ_IOT_HUB_HOSTNAME"
#define IOT_SAMPLE_ENV_PROVISIONING_ID_SCOPE "AZ_IOT_PROVISIONING_ID_SCOPE"

// DO NOT MODIFY: Device information
#define IOT_SAMPLE_ENV_HUB_DEVICE_ID "AZ_IOT_HUB_DEVICE_ID"
#define IOT_SAMPLE_ENV_HUB_SAS_DEVICE_ID "AZ_IOT_HUB_SAS_DEVICE_ID"
#define IOT_SAMPLE_ENV_PROVISIONING_REGISTRATION_ID "AZ_IOT_PROVISIONING_REGISTRATION_ID"
#define IOT_SAMPLE_ENV_PROVISIONING_SAS_REGISTRATION_ID "AZ_IOT_PROVISIONING_SAS_REGISTRATION_ID"

// DO NOT MODIFY: SAS Key
#define IOT_SAMPLE_ENV_HUB_SAS_KEY "AZ_IOT_HUB_SAS_KEY"
#define IOT_SAMPLE_ENV_PROVISIONING_SAS_KEY "AZ_IOT_PROVISIONING_SAS_KEY"
#define IOT_SAMPLE_ENV_SAS_KEY_DURATION_MINUTES \
  "AZ_IOT_SAS_KEY_DURATION_MINUTES" // default is 2 hrs.

// DO NOT MODIFY: the path to a PEM file containing the device certificate and
// key as well as any intermediate certificates chaining to an uploaded group certificate.
#define IOT_SAMPLE_ENV_DEVICE_X509_CERT_PEM_FILE_PATH "AZ_IOT_DEVICE_X509_CERT_PEM_FILE_PATH"

// DO NOT MODIFY: the path to a PEM file containing the server trusted CA
// This is usually not needed on Linux or Mac but needs to be set on Windows.
#define IOT_SAMPLE_ENV_DEVICE_X509_TRUST_PEM_FILE_PATH "AZ_IOT_DEVICE_X509_TRUST_PEM_FILE_PATH"

typedef struct
{
  az_span hub_device_id;
  az_span hub_hostname;
  az_span hub_sas_key;
  az_span provisioning_id_scope;
  az_span provisioning_registration_id;
  az_span provisioning_sas_key;
  az_span x509_cert_pem_file_path;
  az_span x509_trust_pem_file_path;
  uint32_t sas_key_duration_minutes;
} iot_sample_environment_variables;

typedef enum
{
  PAHO_IOT_HUB,
  PAHO_IOT_PROVISIONING
} iot_sample_type;

typedef enum
{
  PAHO_IOT_HUB_C2D_SAMPLE,
  PAHO_IOT_HUB_METHODS_SAMPLE,
  PAHO_IOT_HUB_SAS_TELEMETRY_SAMPLE,
  PAHO_IOT_HUB_TELEMETRY_SAMPLE,
  PAHO_IOT_HUB_TWIN_SAMPLE,
  PAHO_IOT_PNP_SAMPLE,
  PAHO_IOT_PNP_WITH_PROVISIONING_SAMPLE,
  PAHO_IOT_PNP_COMPONENT_SAMPLE,
  PAHO_IOT_PROVISIONING_SAMPLE,
  PAHO_IOT_PROVISIONING_SAS_SAMPLE
} iot_sample_name;

extern bool is_device_operational;

/*
 * @brief Reads in environment variables set by user for purposes of running sample.
 *
 * @param[in] type The enumerated type of the sample.
 * @param[in] name The enumerated name of the sample.
 * @param[out] out_env_vars A pointer to the struct containing all read-in environment variables.
 */
void iot_sample_read_environment_variables(
    iot_sample_type type,
    iot_sample_name name,
    iot_sample_environment_variables* out_env_vars);

/*
 * @brief Builds an MQTT endpoint c-string for an Azure IoT Hub or provisioning service.
 *
 * @param[in] type The enumerated type of the sample.
 * @param[in] env_vars A pointer to environment variable struct.
 * @param[out] endpoint A buffer with sufficient capacity to hold the built endpoint. If
 * successful, contains a null-terminated string of the endpoint.
 * @param[in] endpoint_size The size of \p out_endpoint in bytes.
 */
void iot_sample_create_mqtt_endpoint(
    iot_sample_type type,
    iot_sample_environment_variables const* env_vars,
    char* endpoint,
    size_t endpoint_size);

/*
 * @brief Sleep for given seconds.
 *
 * @param[in] seconds Number of seconds to sleep.
 */
void iot_sample_sleep_for_seconds(uint32_t seconds);

/*
 * @brief Return total seconds passed including supplied minutes.
 *
 * @param[in] minutes Number of minutes to include in total seconds returned.
 *
 * @return Total time in seconds.
 */
uint32_t iot_sample_get_epoch_expiration_time_from_minutes(uint32_t minutes);

/*
 * @brief Generate the base64 encoded and signed signature using HMAC-SHA256 signing.
 *
 * @param[in] sas_base64_encoded_key An #az_span containing the SAS key that will be used for
 * signing.
 * @param[in] sas_signature An #az_span containing the signature.
 * @param[in] sas_base64_encoded_signed_signature An #az_span with sufficient capacity to hold the
 * encoded signed signature.
 * @param[out] out_sas_base64_encoded_signed_signature A pointer to the #az_span containing the
 * encoded signed signature.
 */
void iot_sample_generate_sas_base64_encoded_signed_signature(
    az_span sas_base64_encoded_key,
    az_span sas_signature,
    az_span sas_base64_encoded_signed_signature,
    az_span* out_sas_base64_encoded_signed_signature);

#endif // IOT_SAMPLE_COMMON_H

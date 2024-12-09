// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/az_precondition.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_result_internal.h>
#include <azure/iot/az_iot_hub_client.h>
#include <azure/iot/internal/az_iot_common_internal.h>

#include <azure/core/internal/az_log_internal.h>
#include <azure/core/internal/az_precondition_internal.h>
#include <azure/core/internal/az_span_internal.h>

#include <stdint.h>

#include <azure/core/_az_cfg.h>

#define LF '\n'
#define AMPERSAND '&'
#define EQUAL_SIGN '='
#define STRING_NULL_TERMINATOR '\0'
#define SCOPE_DEVICES_STRING "%2Fdevices%2F"
#define SCOPE_MODULES_STRING "%2Fmodules%2F"
#define SAS_TOKEN_SR "SharedAccessSignature sr"
#define SAS_TOKEN_SE "se"
#define SAS_TOKEN_SIG "sig"
#define SAS_TOKEN_SKN "skn"

static const az_span devices_string = AZ_SPAN_LITERAL_FROM_STR(SCOPE_DEVICES_STRING);
static const az_span modules_string = AZ_SPAN_LITERAL_FROM_STR(SCOPE_MODULES_STRING);
static const az_span skn_string = AZ_SPAN_LITERAL_FROM_STR(SAS_TOKEN_SKN);
static const az_span sr_string = AZ_SPAN_LITERAL_FROM_STR(SAS_TOKEN_SR);
static const az_span sig_string = AZ_SPAN_LITERAL_FROM_STR(SAS_TOKEN_SIG);
static const az_span se_string = AZ_SPAN_LITERAL_FROM_STR(SAS_TOKEN_SE);

AZ_NODISCARD az_result az_iot_hub_client_sas_get_signature(
    az_iot_hub_client const* client,
    uint64_t token_expiration_epoch_time,
    az_span signature,
    az_span* out_signature)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION(token_expiration_epoch_time > 0);
  _az_PRECONDITION_VALID_SPAN(signature, 1, false);
  _az_PRECONDITION_NOT_NULL(out_signature);

  az_span remainder = signature;
  int32_t signature_size = az_span_size(signature);

  _az_RETURN_IF_FAILED(
      _az_span_copy_url_encode(remainder, client->_internal.iot_hub_hostname, &remainder));

  _az_RETURN_IF_NOT_ENOUGH_SIZE(remainder, az_span_size(devices_string));
  remainder = az_span_copy(remainder, devices_string);

  _az_RETURN_IF_FAILED(
      _az_span_copy_url_encode(remainder, client->_internal.device_id, &remainder));

  if (az_span_size(client->_internal.options.module_id) > 0)
  {
    _az_RETURN_IF_NOT_ENOUGH_SIZE(remainder, az_span_size(modules_string));
    remainder = az_span_copy(remainder, modules_string);

    _az_RETURN_IF_FAILED(
        _az_span_copy_url_encode(remainder, client->_internal.options.module_id, &remainder));
  }

  _az_RETURN_IF_NOT_ENOUGH_SIZE(
      remainder,
      1 + // LF
          _az_iot_u64toa_size(token_expiration_epoch_time));

  remainder = az_span_copy_u8(remainder, LF);

  _az_RETURN_IF_FAILED(az_span_u64toa(remainder, token_expiration_epoch_time, &remainder));

  *out_signature = az_span_slice(signature, 0, signature_size - az_span_size(remainder));
  _az_LOG_WRITE(AZ_LOG_IOT_SAS_TOKEN, *out_signature);

  return AZ_OK;
}

AZ_NODISCARD az_result az_iot_hub_client_sas_get_password(
    az_iot_hub_client const* client,
    uint64_t token_expiration_epoch_time,
    az_span base64_hmac_sha256_signature,
    az_span key_name,
    char* mqtt_password,
    size_t mqtt_password_size,
    size_t* out_mqtt_password_length)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_VALID_SPAN(base64_hmac_sha256_signature, 1, false);
  _az_PRECONDITION(token_expiration_epoch_time > 0);
  _az_PRECONDITION_NOT_NULL(mqtt_password);
  _az_PRECONDITION(mqtt_password_size > 0);

  // Concatenates: "SharedAccessSignature sr=" scope "&sig=" sig  "&se=" expiration_time_secs
  //               plus, if key_name size > 0, "&skn=" key_name

  az_span mqtt_password_span = az_span_create((uint8_t*)mqtt_password, (int32_t)mqtt_password_size);

  // SharedAccessSignature
  _az_RETURN_IF_NOT_ENOUGH_SIZE(mqtt_password_span, az_span_size(sr_string) + 1 /* EQUAL_SIGN */);
  mqtt_password_span = az_span_copy(mqtt_password_span, sr_string);
  mqtt_password_span = az_span_copy_u8(mqtt_password_span, EQUAL_SIGN);

  _az_RETURN_IF_FAILED(_az_span_copy_url_encode(
      mqtt_password_span, client->_internal.iot_hub_hostname, &mqtt_password_span));

  // Device ID
  _az_RETURN_IF_NOT_ENOUGH_SIZE(mqtt_password_span, az_span_size(devices_string));
  mqtt_password_span = az_span_copy(mqtt_password_span, devices_string);

  _az_RETURN_IF_FAILED(_az_span_copy_url_encode(
      mqtt_password_span, client->_internal.device_id, &mqtt_password_span));

  // Module ID
  if (az_span_size(client->_internal.options.module_id) > 0)
  {
    _az_RETURN_IF_NOT_ENOUGH_SIZE(mqtt_password_span, az_span_size(modules_string));
    mqtt_password_span = az_span_copy(mqtt_password_span, modules_string);

    _az_RETURN_IF_FAILED(_az_span_copy_url_encode(
        mqtt_password_span, client->_internal.options.module_id, &mqtt_password_span));
  }

  // Signature
  _az_RETURN_IF_NOT_ENOUGH_SIZE(
      mqtt_password_span, 1 /* AMPERSAND */ + az_span_size(sig_string) + 1 /* EQUAL_SIGN */);

  mqtt_password_span = az_span_copy_u8(mqtt_password_span, AMPERSAND);
  mqtt_password_span = az_span_copy(mqtt_password_span, sig_string);
  mqtt_password_span = az_span_copy_u8(mqtt_password_span, EQUAL_SIGN);

  _az_RETURN_IF_FAILED(_az_span_copy_url_encode(
      mqtt_password_span, base64_hmac_sha256_signature, &mqtt_password_span));

  // Expiration
  _az_RETURN_IF_NOT_ENOUGH_SIZE(
      mqtt_password_span, 1 /* AMPERSAND */ + az_span_size(se_string) + 1 /* EQUAL_SIGN */);
  mqtt_password_span = az_span_copy_u8(mqtt_password_span, AMPERSAND);
  mqtt_password_span = az_span_copy(mqtt_password_span, se_string);
  mqtt_password_span = az_span_copy_u8(mqtt_password_span, EQUAL_SIGN);

  _az_RETURN_IF_FAILED(
      az_span_u64toa(mqtt_password_span, token_expiration_epoch_time, &mqtt_password_span));

  if (az_span_size(key_name) > 0)
  {
    // Key Name
    _az_RETURN_IF_NOT_ENOUGH_SIZE(
        mqtt_password_span,
        1 /* AMPERSAND */ + az_span_size(skn_string) + 1 /* EQUAL_SIGN */ + az_span_size(key_name));
    mqtt_password_span = az_span_copy_u8(mqtt_password_span, AMPERSAND);
    mqtt_password_span = az_span_copy(mqtt_password_span, skn_string);
    mqtt_password_span = az_span_copy_u8(mqtt_password_span, EQUAL_SIGN);
    mqtt_password_span = az_span_copy(mqtt_password_span, key_name);
  }

  _az_RETURN_IF_NOT_ENOUGH_SIZE(mqtt_password_span, 1 /* NULL TERMINATOR */);

  mqtt_password_span = az_span_copy_u8(mqtt_password_span, STRING_NULL_TERMINATOR);

  if (out_mqtt_password_length != NULL)
  {
    *out_mqtt_password_length
        = mqtt_password_size - (size_t)az_span_size(mqtt_password_span) - 1 /* NULL TERMINATOR */;
  }

  return AZ_OK;
}

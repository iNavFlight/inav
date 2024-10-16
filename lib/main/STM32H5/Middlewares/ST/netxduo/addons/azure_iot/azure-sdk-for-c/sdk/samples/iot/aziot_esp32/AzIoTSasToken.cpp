// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "AzIoTSasToken.h"
#include "SerialLogger.h"
#include <az_result.h>
#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include <mbedtls/sha256.h>
#include <stdlib.h>
#include <time.h>

#define INDEFINITE_TIME ((time_t)-1)

#define az_span_is_empty(x) (az_span_size(x) == az_span_size(AZ_SPAN_EMPTY) && az_span_ptr(x) == az_span_ptr(AZ_SPAN_EMPTY))

static uint32_t getSasTokenExpiration(const char* sasToken)
{
  const char SE[] = { '&', 's', 'e', '=' };
  uint32_t se_as_unix_time = 0;

  int i, j;
  for (i = 0, j = 0; sasToken[i] != '\0'; i++)
  {
    if (sasToken[i] == SE[j])
    {
      j++;
      if (j == sizeof(SE))
      {
        // i is still at the '=' position. We must advance it by 1.
        i++;
        break;
      }
    }
    else
    {
      j = 0;
    }
  }

  if (j != sizeof(SE))
  {
    Logger.Error("Failed finding `se` field in SAS token");
  }
  else
  {
    int k = i; 
    while (sasToken[k] != '\0' && sasToken[k] != '&') { k++; }

    if (az_result_failed(
            az_span_atou32(az_span_create((uint8_t*)sasToken + i, k - i), &se_as_unix_time)))
    {
      Logger.Error("Failed parsing SAS token expiration timestamp");
    }
  }

  return se_as_unix_time;
}

static void mbedtls_hmac_sha256(az_span key, az_span payload, az_span signed_payload)
{
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
  mbedtls_md_hmac_starts(&ctx, (const unsigned char*)az_span_ptr(key), az_span_size(key));
  mbedtls_md_hmac_update(&ctx, (const unsigned char*)az_span_ptr(payload), az_span_size(payload));
  mbedtls_md_hmac_finish(&ctx, (byte*)az_span_ptr(signed_payload));
  mbedtls_md_free(&ctx);
}

static void hmac_sha256_sign_signature(
    az_span decoded_key,
    az_span signature,
    az_span signed_signature,
    az_span* out_signed_signature)
{
  mbedtls_hmac_sha256(decoded_key, signature, signed_signature);
  *out_signed_signature = az_span_slice(signed_signature, 0, 32);
}

static void base64_encode_bytes(
    az_span decoded_bytes,
    az_span base64_encoded_bytes,
    az_span* out_base64_encoded_bytes)
{
  size_t len;
  int32_t mbedtls_ret;
  if ((mbedtls_ret = mbedtls_base64_encode(
          az_span_ptr(base64_encoded_bytes),
          (size_t)az_span_size(base64_encoded_bytes),
          &len,
          az_span_ptr(decoded_bytes),
          (size_t)az_span_size(decoded_bytes)))
      != 0)
  {
    Logger.Error("mbedtls_base64_encode fail: mbedtls return code " + String(mbedtls_ret));
  }

  *out_base64_encoded_bytes = az_span_create(az_span_ptr(base64_encoded_bytes), (int32_t)len);
}

static int decode_base64_bytes(
    az_span base64_encoded_bytes,
    az_span decoded_bytes,
    az_span* out_decoded_bytes)
{
  memset(az_span_ptr(decoded_bytes), 0, (size_t)az_span_size(decoded_bytes));

  size_t len;
  int32_t mbedtls_ret;
  if ((mbedtls_ret = mbedtls_base64_decode(
          az_span_ptr(decoded_bytes),
          (size_t)az_span_size(decoded_bytes),
          &len,
          az_span_ptr(base64_encoded_bytes),
          (size_t)az_span_size(base64_encoded_bytes)))
      != 0)
  {
    Logger.Error("mbedtls_base64_decode fail: mbedtls return code " + String(mbedtls_ret));
    return 1;
  }
  else
  {
    *out_decoded_bytes = az_span_create(az_span_ptr(decoded_bytes), (int32_t)len);
    return 0;
  }
}

static int iot_sample_generate_sas_base64_encoded_signed_signature(
    az_span sas_base64_encoded_key,
    az_span sas_signature,
    az_span sas_base64_encoded_signed_signature,
    az_span* out_sas_base64_encoded_signed_signature)
{
  // Decode the sas base64 encoded key to use for HMAC signing.
  char sas_decoded_key_buffer[32];
  az_span sas_decoded_key = AZ_SPAN_FROM_BUFFER(sas_decoded_key_buffer);
  
  if (decode_base64_bytes(sas_base64_encoded_key, sas_decoded_key, &sas_decoded_key) != 0)
  {
    Logger.Error("Failed generating encoded signed signature");
    return 1;
  }

  // HMAC-SHA256 sign the signature with the decoded key.
  char sas_hmac256_signed_signature_buffer[32];
  az_span sas_hmac256_signed_signature = AZ_SPAN_FROM_BUFFER(sas_hmac256_signed_signature_buffer);
  hmac_sha256_sign_signature(
      sas_decoded_key, sas_signature, sas_hmac256_signed_signature, &sas_hmac256_signed_signature);

  // Base64 encode the result of the HMAC signing.
  base64_encode_bytes(
      sas_hmac256_signed_signature,
      sas_base64_encoded_signed_signature,
      out_sas_base64_encoded_signed_signature);

  return 0;
}

int64_t iot_sample_get_epoch_expiration_time_from_minutes(uint32_t minutes)
{
  time_t now = time(NULL);
  return (int64_t)(now + minutes * 60);
}

az_span generate_sas_token(
    az_iot_hub_client* hub_client,
    az_span device_key,
    az_span sas_signature,
    unsigned int expiryTimeInMinutes,
    az_span sas_token)
{
  az_result rc;
  // Create the POSIX expiration time from input minutes.
  uint64_t sas_duration = iot_sample_get_epoch_expiration_time_from_minutes(expiryTimeInMinutes);

  // Get the signature that will later be signed with the decoded key.
  // az_span sas_signature = AZ_SPAN_FROM_BUFFER(signature);
  rc = az_iot_hub_client_sas_get_signature(hub_client, sas_duration, sas_signature, &sas_signature);
  if (az_result_failed(rc))
  {
    Logger.Error("Could not get the signature for SAS key: az_result return code " + String(rc));
    return AZ_SPAN_EMPTY;
  }

  // Generate the encoded, signed signature (b64 encoded, HMAC-SHA256 signing).
  char b64enc_hmacsha256_signature[64];
  az_span sas_base64_encoded_signed_signature = AZ_SPAN_FROM_BUFFER(b64enc_hmacsha256_signature);
  
  if (iot_sample_generate_sas_base64_encoded_signed_signature(
      device_key,
      sas_signature,
      sas_base64_encoded_signed_signature,
      &sas_base64_encoded_signed_signature) != 0)
  {
    Logger.Error("Failed generating SAS token signed signature");
    return AZ_SPAN_EMPTY;
  }

  // Get the resulting MQTT password, passing the base64 encoded, HMAC signed bytes.
  size_t mqtt_password_length;
  rc = az_iot_hub_client_sas_get_password(
      hub_client,
      sas_duration,
      sas_base64_encoded_signed_signature,
      AZ_SPAN_EMPTY,
      (char*)az_span_ptr(sas_token),
      az_span_size(sas_token),
      &mqtt_password_length);

  if (az_result_failed(rc))
  {
    Logger.Error("Could not get the password: az_result return code " + String(rc));
    return AZ_SPAN_EMPTY;
  }
  else
  {
    return az_span_slice(sas_token, 0, mqtt_password_length); 
  }
}

AzIoTSasToken::AzIoTSasToken(
    az_iot_hub_client* client,
    az_span deviceKey,
    az_span signatureBuffer,
    az_span sasTokenBuffer)
{
  this->client = client;
  this->deviceKey = deviceKey;
  this->signatureBuffer = signatureBuffer;
  this->sasTokenBuffer = sasTokenBuffer;
  this->expirationUnixTime = 0;
  this->sasToken = AZ_SPAN_EMPTY;
}

int AzIoTSasToken::Generate(unsigned int expiryTimeInMinutes)
{
  this->sasToken = generate_sas_token(
      this->client,
      this->deviceKey,
      this->signatureBuffer,
      expiryTimeInMinutes,
      this->sasTokenBuffer);
  
  if (az_span_is_empty(this->sasToken))
  {
    Logger.Error("Failed generating SAS token");
    return 1;
  }
  else
  {
    this->expirationUnixTime = getSasTokenExpiration((const char*)az_span_ptr(this->sasToken));

    if (this->expirationUnixTime == 0)
    {
      Logger.Error("Failed getting the SAS token expiration time");
      this->sasToken = AZ_SPAN_EMPTY;
      return 1;
    }
    else
    {
      return 0; 
    }
  }
}

bool AzIoTSasToken::IsExpired()
{
  time_t now = time(NULL);

  if (now == INDEFINITE_TIME)
  {
    Logger.Error("Failed getting current time");
    return true;
  }
  else
  {
    return (now >= this->expirationUnixTime);
  }
}

az_span AzIoTSasToken::Get() { return this->sasToken; }

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef AZIOTSASTOKEN_H
#define AZIOTSASTOKEN_H

#include <Arduino.h>
#include <az_iot_hub_client.h>
#include <az_span.h>

class AzIoTSasToken
{
public:
  AzIoTSasToken(
      az_iot_hub_client* client,
      az_span deviceKey,
      az_span signatureBuffer,
      az_span sasTokenBuffer);
  int Generate(unsigned int expiryTimeInMinutes);
  bool IsExpired();
  az_span Get();

private:
  az_iot_hub_client* client;
  az_span deviceKey;
  az_span signatureBuffer;
  az_span sasTokenBuffer;
  az_span sasToken;
  uint32_t expirationUnixTime;
};

#endif // AZIOTSASTOKEN_H

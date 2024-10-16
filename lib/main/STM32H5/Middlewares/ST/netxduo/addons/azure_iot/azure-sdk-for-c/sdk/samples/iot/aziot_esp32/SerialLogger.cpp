// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "SerialLogger.h"
#include <time.h>

#define UNIX_EPOCH_START_YEAR 1900

static void writeTime()
{
  struct tm* ptm;
  time_t now = time(NULL);

  ptm = gmtime(&now);

  Serial.print(ptm->tm_year + UNIX_EPOCH_START_YEAR);
  Serial.print("/");
  Serial.print(ptm->tm_mon + 1);
  Serial.print("/");
  Serial.print(ptm->tm_mday);
  Serial.print(" ");

  if (ptm->tm_hour < 10)
  {
    Serial.print(0);
  }

  Serial.print(ptm->tm_hour);
  Serial.print(":");

  if (ptm->tm_min < 10)
  {
    Serial.print(0);
  }

  Serial.print(ptm->tm_min);
  Serial.print(":");

  if (ptm->tm_sec < 10)
  {
    Serial.print(0);
  }

  Serial.print(ptm->tm_sec);
}

SerialLogger::SerialLogger() { Serial.begin(SERIAL_LOGGER_BAUD_RATE); }

void SerialLogger::Info(String message)
{
  writeTime();
  Serial.print(" [INFO] ");
  Serial.println(message);
}

void SerialLogger::Error(String message)
{
  writeTime();
  Serial.print(" [ERROR] ");
  Serial.println(message);
}

SerialLogger Logger;

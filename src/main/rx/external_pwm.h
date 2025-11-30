#pragma once
#include <stdint.h>

void externalPwmInit(void);
void externalPwmUpdate(void);
uint16_t getExternalPwmUs(void);
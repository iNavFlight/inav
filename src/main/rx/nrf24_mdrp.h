/*
 * nrf24_mdrp.h
 *
 *  Created on: 30.05.2017
 *      Author: Jan
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

struct rxConfig_s;
struct rxRuntimeConfig_s;
void mdrpNrf24Init(const struct rxConfig_s *rxConfig, struct rxRuntimeConfig_s *rxRuntimeConfig);
void mdrpNrf24SetRcDataFromPayload(uint16_t *rcData, const uint8_t *payload);
rx_spi_received_e mdrpNrf24DataReceived(uint8_t *payload);

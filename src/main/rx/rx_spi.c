/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>

#include <platform.h>

#ifdef USE_RX_SPI

#include "build/build_config.h"

#include "common/utils.h"

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/rx_nrf24l01.h"

#include "fc/config.h"

#include "rx/rx.h"
#include "rx/rx_spi.h"
#include "rx/eleres.h"
#include "rx/cc2500_frsky_common.h"
#include "rx/nrf24_cx10.h"
#include "rx/nrf24_syma.h"
#include "rx/nrf24_v202.h"
#include "rx/nrf24_h8_3d.h"
#include "rx/nrf24_inav.h"


static uint16_t rxSpiRcData[MAX_SUPPORTED_RC_CHANNEL_COUNT];
STATIC_UNIT_TESTED uint8_t rxSpiPayload[RX_SPI_MAX_PAYLOAD_SIZE];
STATIC_UNIT_TESTED uint8_t rxSpiNewPacketAvailable; // set true when a new packet is received

typedef void (*protocolInitPtr)(rxRuntimeConfig_t *rxRuntimeConfig);
typedef rx_spi_received_e (*protocolDataReceivedPtr)(uint8_t *payload, uint16_t *linkQuality);
typedef rx_spi_received_e (*protocolProcessFrameFnPtr)(uint8_t *payload);
typedef void (*protocolSetRcDataFromPayloadPtr)(uint16_t *rcData, const uint8_t *payload);

static protocolInitPtr protocolInit;
static protocolDataReceivedPtr protocolDataReceived;
static protocolProcessFrameFnPtr protocolProcessFrame;
static protocolSetRcDataFromPayloadPtr protocolSetRcDataFromPayload;

PG_REGISTER_WITH_RESET_TEMPLATE(rxSpiConfig_t, rxSpiConfig, PG_RX_SPI_CONFIG, 0);

PG_RESET_TEMPLATE(rxSpiConfig_t, rxSpiConfig,
    .rx_spi_protocol = RX_SPI_DEFAULT_PROTOCOL,
    .rx_spi_id = 0,
    .auto_bind = false,
    .bind_offset = 0,
    .bind_tx_id = {0, 0, 0},
    .bind_hop_data =   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
);

STATIC_UNIT_TESTED uint16_t rxSpiReadRawRC(const rxRuntimeConfig_t *rxRuntimeConfig, uint8_t channel)
{
    BUILD_BUG_ON(NRF24L01_MAX_PAYLOAD_SIZE > RX_SPI_MAX_PAYLOAD_SIZE);
    if (channel >= rxRuntimeConfig->channelCount) {
        return 0;
    }
    if (rxSpiNewPacketAvailable) {
        protocolSetRcDataFromPayload(rxSpiRcData, rxSpiPayload);
        rxSpiNewPacketAvailable = false;
    }
    return rxSpiRcData[channel];
}

STATIC_UNIT_TESTED bool rxSpiSetProtocol(rx_spi_protocol_e protocol)
{
    switch ((uint8_t)protocol) {
#ifdef USE_RX_V202
    case NRF24RX_V202_250K:
    case NRF24RX_V202_1M:
        protocolInit = v202Nrf24Init;
        protocolDataReceived = v202Nrf24DataReceived;
        protocolSetRcDataFromPayload = v202Nrf24SetRcDataFromPayload;
        break;
#endif
#ifdef USE_RX_SYMA
    case NRF24RX_SYMA_X:
    case NRF24RX_SYMA_X5C:
        protocolInit = symaNrf24Init;
        protocolDataReceived = symaNrf24DataReceived;
        protocolSetRcDataFromPayload = symaNrf24SetRcDataFromPayload;
        break;
#endif
#ifdef USE_RX_CX10
    case NRF24RX_CX10:
    case NRF24RX_CX10A:
        protocolInit = cx10Nrf24Init;
        protocolDataReceived = cx10Nrf24DataReceived;
        protocolSetRcDataFromPayload = cx10Nrf24SetRcDataFromPayload;
        break;
#endif
#ifdef USE_RX_H8_3D
    case NRF24RX_H8_3D:
        protocolInit = h8_3dNrf24Init;
        protocolDataReceived = h8_3dNrf24DataReceived;
        protocolSetRcDataFromPayload = h8_3dNrf24SetRcDataFromPayload;
        break;
#endif
#ifdef USE_RX_INAV
    case NRF24RX_INAV:
        protocolInit = inavNrf24Init;
        protocolDataReceived = inavNrf24DataReceived;
        protocolSetRcDataFromPayload = inavNrf24SetRcDataFromPayload;
        break;
#endif
#ifdef USE_RX_ELERES
    case RFM22_ELERES:
        protocolInit = eleresInit;
        protocolDataReceived = eleresDataReceived;
        protocolSetRcDataFromPayload = eleresSetRcDataFromPayload;
        break;
#endif
#if defined(USE_RX_FRSKY_SPI)
#if defined(USE_RX_FRSKY_SPI_D)
    case RX_SPI_FRSKY_D:
#endif
#if defined(USE_RX_FRSKY_SPI_X)
    case RX_SPI_FRSKY_X:
    case RX_SPI_FRSKY_X_LBT:
    case RX_SPI_FRSKY_X_V2:
    case RX_SPI_FRSKY_X_LBT_V2:
        protocolInit = frSkySpiInit;
        protocolDataReceived = frSkySpiDataReceived;
        protocolSetRcDataFromPayload = frSkySpiSetRcData;
        protocolProcessFrame = frSkySpiProcessFrame;
        break;
#endif
#endif

	default:
        return false;
    }
    return true;
}

/*
 * Returns true if the RX has received new data.
 * Called from updateRx in rx.c, updateRx called from taskUpdateRxCheck.
 * If taskUpdateRxCheck returns true, then taskUpdateRxMain will shortly be called.
 */
static uint8_t rxSpiFrameStatus(rxRuntimeConfig_t *rxRuntimeConfig)
{
    uint16_t linkQuality = 0;
    uint8_t status = RX_FRAME_PENDING;

    rx_spi_received_e result = protocolDataReceived(rxSpiPayload, &linkQuality);

    if (result & RX_SPI_RECEIVED_DATA) {
        lqTrackerSet(rxRuntimeConfig->lqTracker, linkQuality);
        rxSpiNewPacketAvailable = true;
        status = RX_FRAME_COMPLETE;
    }

    if (result & RX_SPI_ROCESSING_REQUIRED) {
        status |= RX_FRAME_PROCESSING_REQUIRED;
    }

    return status;
}


static bool rxSpiProcessFrame(const rxRuntimeConfig_t *rxRuntimeConfig)
{
    UNUSED(rxRuntimeConfig);

    if (protocolProcessFrame) {
        rx_spi_received_e result = protocolProcessFrame(rxSpiPayload);

        if (result & RX_SPI_RECEIVED_DATA) {
            rxSpiNewPacketAvailable = true;
        }

        if (result & RX_SPI_ROCESSING_REQUIRED) {
            return false;
        }
    }

    return true;
}
	
/*
 * Set and initialize the RX protocol
 */
bool rxSpiInit(const rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig)
{
    UNUSED(rxConfig);

    bool ret = false;
    rxSpiDeviceInit();
    if (rxSpiSetProtocol(rxSpiConfig()->rx_spi_protocol)) {
        protocolInit(rxRuntimeConfig);

        if (rxSpiExtiConfigured()) {
            rxSpiExtiInit();
        }

        ret = true;
    }

    rxSpiNewPacketAvailable = false;
	rxRuntimeConfig->rxRefreshRate = 20000;
    rxRuntimeConfig->rcReadRawFn = rxSpiReadRawRC;
    rxRuntimeConfig->rcFrameStatusFn = rxSpiFrameStatus;
    rxRuntimeConfig->rcProcessFrameFn = rxSpiProcessFrame;

    return ret;
}
#endif

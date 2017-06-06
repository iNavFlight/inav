#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#ifdef USE_RX_MDRP

#include "build/build_config.h"

#include "build/debug.h"

#include "common/utils.h"

#include "drivers/rx_nrf24l01.h"
#include "drivers/time.h"

#include "rx/rx.h"
#include "rx/rx_spi.h"
#include "rx/nrf24_mdrp.h"

#include "telemetry/ltm.h"

#include "navigation/navigation.h"

#define RX_TX_ADDR_LEN 5
#define RC_CHANNEL_COUNT 14
#define RC_CHANNEL_COUNT_MAX MAX_SUPPORTED_RC_CHANNEL_COUNT
#define PAYLOAD_SIZE 10
#define Bind_ackSize 5
#define BIND_PAYLOAD0 0
#define BIND_PAYLOAD1 1
#define DataHeader 255
STATIC_UNIT_TESTED uint8_t ackPayload[];
STATIC_UNIT_TESTED const uint8_t payloadSize = PAYLOAD_SIZE;
STATIC_UNIT_TESTED uint8_t RxTxAddr[RX_TX_ADDR_LEN] =
  { 0xE8, 0xE8, 0xF0, 0xF0, 0xE1};

uint8_t ltmCurrentFrame = 0;
#if defined(GPS)
static uint8_t ltmFrameType[4] =
  { LTM_SFRAME, LTM_GFRAME, LTM_OFRAME, LTM_NFRAME};
#else
static uint8_t ltmFrameType[4] =
  { LTM_SFRAME};
#endif

typedef enum
  {
    STATE_BIND = 0,
    STATE_DATA
  }protocol_state_t;
STATIC_UNIT_TESTED protocol_state_t protocolState;

static void mdrpSetBound(void)
  {
    protocolState = STATE_DATA;
    NRF24L01_WriteRegisterMulti(NRF24L01_0A_RX_ADDR_P0, RxTxAddr, RX_TX_ADDR_LEN);
    NRF24L01_WriteRegisterMulti(NRF24L01_10_TX_ADDR, RxTxAddr, RX_TX_ADDR_LEN);
    rxConfigMutable()->rx_spi_id = *RxTxAddr;
  }

static uint16_t mapRCRange(uint16_t in)
  {
    return ((in * 1000) / 1023) + 1000;
  }

STATIC_UNIT_TESTED bool mdrpCheckPayload(uint8_t *payload)
  {
    uint8_t Checksum = 0;
    bool validPacket = false;
    for(int s = 0; s < PAYLOAD_SIZE -1; s++)
      {
        Checksum ^= payload[s];
      }
    if(Checksum == payload[9]) validPacket = true;
    return validPacket;
  }

STATIC_UNIT_TESTED bool mdrpCheckBindPacket(const uint8_t *payload)
  {
    bool bindPacket = false;
    if (payload[0] == BIND_PAYLOAD0 && payload[1] == BIND_PAYLOAD1)
      {
        bindPacket = true;
        if (protocolState == STATE_BIND)
          {
            RxTxAddr[0] = payload[2];
            RxTxAddr[1] = payload[3];
            RxTxAddr[2] = payload[4];
            RxTxAddr[3] = payload[5];
            RxTxAddr[4] = payload[6];
          }
      }
    return bindPacket;
  }

STATIC_UNIT_TESTED bool mdrpCheckDataPacket(const uint8_t *payload) {
  bool dataPacket = false;
  if(payload[0] == DataHeader) {
      dataPacket = true;
  }
  return dataPacket;
}

void mdrpNrf24SetRcDataFromPayload(uint16_t *rcData, const uint8_t *payload)
  {

    memset(rcData, 0, MAX_SUPPORTED_RC_CHANNEL_COUNT * sizeof(uint16_t));
    uint8_t Exbyte = payload[4]; //bit 8 and 9 of 10 bit value
    //
    rcData[RC_SPI_ROLL] = mapRCRange((payload[0] + ((Exbyte & 0xC0) << 2)));
    Exbyte <<= 2;
    rcData[RC_SPI_PITCH] = mapRCRange((payload[1] + ((Exbyte & 0xC0) << 2)));
    Exbyte <<= 2;
    rcData[RC_SPI_THROTTLE] = mapRCRange((payload[2] + ((Exbyte & 0xC0) << 2)));
    Exbyte <<= 2;
    rcData[RC_SPI_YAW] = mapRCRange((payload[3] + ((Exbyte & 0xC0) << 2)));

    uint8_t Switch = payload[5];
    if ((Switch & 3) == 2) rcData[RC_SPI_AUX1] = PWM_RANGE_MAX;
    if ((Switch & 3) == 1) rcData[RC_SPI_AUX1] = PWM_RANGE_MIDDLE;
    if ((Switch & 3) == 0) rcData[RC_SPI_AUX1] = PWM_RANGE_MIN;
    Switch >>= 2;

    if ((Switch & 3) == 2) rcData[RC_SPI_AUX2] = PWM_RANGE_MAX;
    if ((Switch & 3) == 1) rcData[RC_SPI_AUX2] = PWM_RANGE_MIDDLE;
    if ((Switch & 3) == 0) rcData[RC_SPI_AUX2] = PWM_RANGE_MIN;
    Switch >>= 2;

    if ((Switch & 1) == 1) rcData[RC_SPI_AUX3] = PWM_RANGE_MAX;
    if ((Switch & 1) == 0) rcData[RC_SPI_AUX3] = PWM_RANGE_MIN;
    Switch >>= 1;

    if ((Switch & 1) == 1) rcData[RC_SPI_AUX4] = PWM_RANGE_MAX;
    if ((Switch & 1) == 0) rcData[RC_SPI_AUX4] = PWM_RANGE_MIN;
    Switch >>= 1;

    rcData[RC_SPI_AUX11] = PWM_RANGE_MIN + payload[6];
    rcData[RC_SPI_AUX12] = PWM_RANGE_MIN + payload[7];

  }

static void writeAckPayload(uint8_t *payload, uint8_t ackSize)
  {
    NRF24L01_WriteReg(NRF24L01_07_STATUS, BV(NRF24L01_07_STATUS_MAX_RT));
    NRF24L01_WriteAckPayload(payload, ackSize, NRF24L01_PIPE0);
  }

static uint8_t getAckCheck(uint8_t ackSize)
  {
    uint8_t ackCheck = 0;
    for(uint8_t i = 0; i <= ackSize - 1; i++)
      {
        ackCheck ^= ackPayload[i];
      }
    return ackCheck;
  }

static void writeTelemetryAck(void)
  {
    ackPayload[0] = ltmFrameType[ltmCurrentFrame];
    uint8_t ltmSize = getLtmFrame(&ackPayload[1], ltmFrameType[ltmCurrentFrame]);
    ackPayload[ltmSize + 1] = getAckCheck(ltmSize + 2);
    ltmCurrentFrame ++;
    if(ltmCurrentFrame >= 4) ltmCurrentFrame = 0;
    writeAckPayload(ackPayload, ltmSize +2);
  }

static void writeBindAck(void)
  {
    ackPayload[0] = BIND_PAYLOAD0;
    ackPayload[1] = BIND_PAYLOAD1;
    //ackPayload[2] = HoppingChannel1
    //ackPayload[3] = HoppingChannel2
    ackPayload[4] = getAckCheck(Bind_ackSize);
    writeAckPayload(ackPayload, Bind_ackSize);
  }

rx_spi_received_e mdrpNrf24DataReceived(uint8_t *payload)
  {
    rx_spi_received_e ret = RX_SPI_RECEIVED_NONE;

    switch (protocolState)
      {
        case STATE_BIND:
        if (NRF24L01_ReadPayloadIfAvailable(payload, payloadSize))
          {
            const bool validPacket = mdrpCheckPayload(payload);
            const bool bindPacket = mdrpCheckBindPacket(payload);
            if (validPacket && bindPacket)
              {
                ret = RX_SPI_RECEIVED_BIND;
                writeBindAck();
                mdrpSetBound();
              }
          }
        break;
        case STATE_DATA:

        // read the payload, processing of payload is deferred
        if (NRF24L01_ReadPayloadIfAvailable(payload, payloadSize))
          {
            const bool validPacket = mdrpCheckPayload(payload);
            const bool bindPacket = mdrpCheckBindPacket(payload);
            const bool dataPacket = mdrpCheckDataPacket(payload);
            if (validPacket && dataPacket)
              {
                ret = RX_SPI_RECEIVED_DATA;
                writeTelemetryAck();
              }
            if (validPacket && bindPacket)
              {
                // transmitter may still continue to transmit bind packets after we have switched to data mode
                ret = RX_SPI_RECEIVED_BIND;
                writeBindAck();
              }
          }
        break;
      }
    return ret;
  }

static void mdrpNrf24Setup(rx_spi_protocol_e protocol, const uint32_t *rxSpiId, int rfChannelHoppingCount)
  {
    UNUSED(protocol);
    UNUSED(rfChannelHoppingCount);

    // sets PWR_UP, EN_CRC, CRCO - 2 byte CRC, only get IRQ pin interrupt on RX_DR
    NRF24L01_Initialize(BV(NRF24L01_00_CONFIG_EN_CRC) | BV(NRF24L01_00_CONFIG_CRCO) | BV(NRF24L01_00_CONFIG_MASK_MAX_RT) | BV(NRF24L01_00_CONFIG_MASK_TX_DS));

    NRF24L01_WriteReg(NRF24L01_01_EN_AA, BV(NRF24L01_01_EN_AA_ENAA_P0));// auto acknowledgment on P0
    NRF24L01_WriteReg(NRF24L01_02_EN_RXADDR, BV(NRF24L01_02_EN_RXADDR_ERX_P0));
    NRF24L01_WriteReg(NRF24L01_03_SETUP_AW, NRF24L01_03_SETUP_AW_5BYTES);// 5-byte RX/TX address
    NRF24L01_WriteReg(NRF24L01_04_SETUP_RETR, 0);
    NRF24L01_Activate(0x73);// activate R_RX_PL_WID, W_ACK_PAYLOAD, and W_TX_PAYLOAD_NOACK registers
    NRF24L01_WriteReg(NRF24L01_1D_FEATURE, BV(NRF24L01_1D_FEATURE_EN_ACK_PAY) | BV(NRF24L01_1D_FEATURE_EN_DPL));
    NRF24L01_WriteReg(NRF24L01_1C_DYNPD, BV(NRF24L01_1C_DYNPD_DPL_P0));// enable dynamic payload length on P0
    //NRF24L01_Activate(0x73); // deactivate R_RX_PL_WID, W_ACK_PAYLOAD, and W_TX_PAYLOAD_NOACK registers

    NRF24L01_WriteRegisterMulti(NRF24L01_10_TX_ADDR, RxTxAddr, RX_TX_ADDR_LEN);
    NRF24L01_WriteReg(NRF24L01_06_RF_SETUP, NRF24L01_06_RF_SETUP_RF_DR_250Kbps | NRF24L01_06_RF_SETUP_RF_PWR_n12dbm);
    // RX_ADDR for pipes P1-P5 are left at default values
    NRF24L01_WriteRegisterMulti(NRF24L01_0A_RX_ADDR_P0, RxTxAddr, RX_TX_ADDR_LEN);
    NRF24L01_WriteReg(NRF24L01_11_RX_PW_P0, payloadSize);

    if (rxSpiId == NULL || *rxSpiId == 0)
      {
        protocolState = STATE_BIND;
        NRF24L01_SetChannel(100);
      }
    else
      {
        memcpy(RxTxAddr, rxSpiId, sizeof(uint32_t));
        mdrpSetBound();
      }
    NRF24L01_SetRxMode();
    writeAckPayload(ackPayload, payloadSize);
  }

void mdrpNrf24Init(const rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig)
  {
    rxRuntimeConfig->channelCount = RC_CHANNEL_COUNT_MAX;
    mdrpNrf24Setup((rx_spi_protocol_e)rxConfig->rx_spi_protocol, &rxConfig->rx_spi_id, rxConfig->rx_spi_rf_channel_count);
  }
#endif


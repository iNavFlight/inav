#include "mavlink/mavlink_internal.h"

#include "fc/fc_mavlink.h"

#include "mavlink/mavlink_command.h"
#include "mavlink/mavlink_guided.h"
#include "mavlink/mavlink_runtime.h"
#include "mavlink/mavlink_streams.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

static bool handleIncoming_RC_CHANNELS_OVERRIDE(void) {
    mavlink_rc_channels_override_t msg;
    mavlink_msg_rc_channels_override_decode(&mavlinkContext.recvMsg, &msg);
    mavlinkRxHandleMessage(&msg);
    return true;
}

static bool handleIncoming_PARAM_REQUEST_LIST(void) {
    mavlink_param_request_list_t msg;
    mavlink_msg_param_request_list_decode(&mavlinkContext.recvMsg, &msg);

    if (msg.target_system != 0 && msg.target_system != mavSystemId) {
        return false;
    }
    if (msg.target_component != 0 && msg.target_component != mavComponentId) {
        return false;
    }

    mavlink_msg_param_value_pack(mavSystemId, mavComponentId, &mavSendMsg, 0, 0, 0, 0, 0);
    mavlinkSendMessage();
    return true;
}

static bool mavlinkIngressPortIsMavlinkSerialRx(uint8_t ingressPortIndex)
{
    return ingressPortIndex < mavPortCount &&
        mavPortStates[ingressPortIndex].portConfig &&
        (mavPortStates[ingressPortIndex].portConfig->functionMask & FUNCTION_RX_SERIAL) &&
        rxConfig()->receiverType == RX_TYPE_SERIAL &&
        rxConfig()->serialrx_provider == SERIALRX_MAVLINK;
}

static uint16_t mavlinkDbmToMilliwatts(int8_t powerDbm)
{
    if (powerDbm == INT8_MAX) {
        return 0;
    }

    return powerDbm <= 0 ? 0 : lrintf(powf(10.0f, powerDbm / 10.0f));
}

static void mavlinkParseRxStats(const mavlink_radio_status_t *msg) {
    switch(mavActiveConfig->radio_type) {
        case MAVLINK_RADIO_SIK:
            rxLinkStatistics.uplinkRSSI = (msg->rssi / 1.9) - 127;
            rxLinkStatistics.uplinkSNR = msg->noise / 1.9;
            rxLinkStatistics.uplinkLQ = msg->rssi != 255 ? scaleRange(msg->rssi, 0, 254, 0, 100) : 0;
            break;
        case MAVLINK_RADIO_ELRS:
            rxLinkStatistics.uplinkRSSI = -msg->remrssi;
            rxLinkStatistics.uplinkSNR = msg->noise;
            rxLinkStatistics.uplinkLQ = scaleRange(msg->rssi, 0, 255, 0, 100);
            break;
        case MAVLINK_RADIO_MLRS:
            break;
        case MAVLINK_RADIO_GENERIC:
        default:
            rxLinkStatistics.uplinkRSSI = msg->rssi;
            rxLinkStatistics.uplinkSNR = msg->noise;
            rxLinkStatistics.uplinkLQ = msg->rssi != 255 ? scaleRange(msg->rssi, 0, 254, 0, 100) : 0;
            break;
    }
}

static bool handleIncoming_RADIO_STATUS(void) {
    mavlink_radio_status_t msg;
    mavlink_msg_radio_status_decode(&mavlinkContext.recvMsg, &msg);
    if (msg.txbuf > 0) {
        mavActivePort->txbuffValid = true;
        mavActivePort->txbuffFree = msg.txbuf;
    } else {
        mavActivePort->txbuffValid = false;
        mavActivePort->txbuffFree = 100;
    }

    if (rxConfig()->receiverType == RX_TYPE_SERIAL &&
        rxConfig()->serialrx_provider == SERIALRX_MAVLINK) {
        mavlinkParseRxStats(&msg);
    }

    return true;
}

static bool handleIncoming_MLRS_RADIO_LINK_STATS(uint8_t ingressPortIndex)
{
    if (mavlinkContext.recvMsg.compid != MAV_COMP_ID_TELEMETRY_RADIO) {
        return false;
    }

    mavlink_mlrs_radio_link_stats_t msg;
    mavlink_msg_mlrs_radio_link_stats_decode(&mavlinkContext.recvMsg, &msg);

    if (msg.target_system != mavSystemId) {
        return false;
    }
    if (msg.target_component != 0 && msg.target_component != mavComponentId) {
        return false;
    }

    mavlinkMlrsLinkStatsRuntime_t *stats = &mavPortStates[ingressPortIndex].mlrs.stats;
    stats->valid = true;
    stats->packet = msg;
    stats->rssiIsDbm = (msg.flags & MLRS_RADIO_LINK_STATS_FLAGS_RSSI_DBM) != 0;
    stats->activeAntenna = (msg.flags & MLRS_RADIO_LINK_STATS_FLAGS_RX_RECEIVE_ANTENNA2) ? 1 : 0;
    stats->rxLinkQualityRc = msg.rx_LQ_rc == UINT8_MAX ? 0 : MIN(msg.rx_LQ_rc, 100);
    stats->rxLinkQualitySerial = msg.rx_LQ_ser == UINT8_MAX ? 0 : MIN(msg.rx_LQ_ser, 100);

    uint8_t rxRssi = stats->activeAntenna ? msg.rx_rssi2 : msg.rx_rssi1;
    if (stats->activeAntenna && rxRssi == UINT8_MAX) {
        rxRssi = msg.rx_rssi1;
    }
    stats->rxRssi = 0;
    if (rxRssi != 0 && rxRssi != UINT8_MAX) {
        stats->rxRssi = stats->rssiIsDbm ? -rxRssi : rxRssi;
    }

    int8_t rxSnr = stats->activeAntenna ? msg.rx_snr2 : msg.rx_snr1;
    if (stats->activeAntenna && rxSnr == INT8_MAX) {
        rxSnr = msg.rx_snr1;
    }
    stats->rxSnr = rxSnr == INT8_MAX ? 0 : rxSnr;

    if (!mavlinkIngressPortIsMavlinkSerialRx(ingressPortIndex)) {
        return true;
    }

    rxLinkStatistics.uplinkLQ = stats->rxLinkQualityRc;
    rxLinkStatistics.downlinkLQ = stats->rxLinkQualitySerial;
    rxLinkStatistics.uplinkRSSI = stats->rxRssi;
    rxLinkStatistics.uplinkSNR = stats->rxSnr;
    rxLinkStatistics.activeAntenna = stats->activeAntenna;

    return true;
}

static bool handleIncoming_MLRS_RADIO_LINK_INFORMATION(uint8_t ingressPortIndex)
{
    if (mavlinkContext.recvMsg.compid != MAV_COMP_ID_TELEMETRY_RADIO) {
        return false;
    }

    mavlink_mlrs_radio_link_information_t msg;
    mavlink_msg_mlrs_radio_link_information_decode(&mavlinkContext.recvMsg, &msg);

    if (msg.target_system != mavSystemId) {
        return false;
    }
    if (msg.target_component != 0 && msg.target_component != mavComponentId) {
        return false;
    }

    mavlinkMlrsLinkInformationRuntime_t *info = &mavPortStates[ingressPortIndex].mlrs.info;
    info->valid = true;
    info->packet = msg;
    memset(info->modeStr, 0, sizeof(info->modeStr));
    memset(info->bandStr, 0, sizeof(info->bandStr));
    memcpy(info->modeStr, msg.mode_str, MAVLINK_MSG_MLRS_RADIO_LINK_INFORMATION_FIELD_MODE_STR_LEN);
    memcpy(info->bandStr, msg.band_str, MAVLINK_MSG_MLRS_RADIO_LINK_INFORMATION_FIELD_BAND_STR_LEN);
    info->txPowerMw = mavlinkDbmToMilliwatts(msg.tx_power);
    info->rxPowerMw = mavlinkDbmToMilliwatts(msg.rx_power);
    info->txReceiveSensitivityDbm = msg.tx_receive_sensitivity ? -(int16_t)msg.tx_receive_sensitivity : 0;
    info->rxReceiveSensitivityDbm = msg.rx_receive_sensitivity ? -(int16_t)msg.rx_receive_sensitivity : 0;

    if (!mavlinkIngressPortIsMavlinkSerialRx(ingressPortIndex)) {
        return true;
    }

    rxLinkStatistics.uplinkTXPower = info->txPowerMw;
    rxLinkStatistics.downlinkTXPower = info->rxPowerMw;
    memset(rxLinkStatistics.band, 0, sizeof(rxLinkStatistics.band));
    memset(rxLinkStatistics.mode, 0, sizeof(rxLinkStatistics.mode));
    memcpy(rxLinkStatistics.band, info->bandStr, sizeof(rxLinkStatistics.band) - 1);
    memcpy(rxLinkStatistics.mode, info->modeStr, sizeof(rxLinkStatistics.mode) - 1);
    sl_toupperptr(rxLinkStatistics.band);
    sl_toupperptr(rxLinkStatistics.mode);

    return true;
}

static bool handleIncoming_MLRS_RADIO_LINK_FLOW_CONTROL(uint8_t ingressPortIndex)
{
    if (mavlinkContext.recvMsg.compid != MAV_COMP_ID_TELEMETRY_RADIO) {
        return false;
    }

    mavlink_mlrs_radio_link_flow_control_t msg;
    mavlink_msg_mlrs_radio_link_flow_control_decode(&mavlinkContext.recvMsg, &msg);

    mavlinkMlrsFlowControlRuntime_t *flowControl = &mavPortStates[ingressPortIndex].mlrs.flowControl;
    flowControl->valid = true;
    flowControl->packet = msg;

    if (msg.txbuf <= 100) {
        mavActivePort->txbuffValid = true;
        mavActivePort->txbuffFree = msg.txbuf;
    } else {
        mavActivePort->txbuffValid = false;
        mavActivePort->txbuffFree = 100;
    }

    return true;
}

#ifdef USE_ADSB
static bool handleIncoming_ADSB_VEHICLE(void) {
    mavlink_adsb_vehicle_t msg;
    mavlink_msg_adsb_vehicle_decode(&mavlinkContext.recvMsg, &msg);

    adsbVehicleValues_t* vehicle = getVehicleForFill();
    if(vehicle != NULL){
        vehicle->icao = msg.ICAO_address;
        vehicle->gps.lat = msg.lat;
        vehicle->gps.lon = msg.lon;
        vehicle->alt = (int32_t)(msg.altitude / 10);
        vehicle->horVelocity = msg.hor_velocity;
        vehicle->heading = msg.heading;
        vehicle->flags = msg.flags;
        vehicle->altitudeType = msg.altitude_type;
        memcpy(&(vehicle->callsign), msg.callsign, sizeof(vehicle->callsign));
        vehicle->emitterType = msg.emitter_type;
        vehicle->tslc = msg.tslc;

        adsbNewVehicle(vehicle);
    }

    return true;
}
#endif

mavlinkFcDispatchResult_e mavlinkFcDispatchIncomingMessage(uint8_t ingressPortIndex)
{
    switch (mavlinkContext.recvMsg.msgid) {
    case MAVLINK_MSG_ID_HEARTBEAT:
        return mavlinkHandleIncomingHeartbeat() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
        return handleIncoming_PARAM_REQUEST_LIST() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_COMMAND_LONG:
        return mavlinkHandleIncomingCommandLong() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_COMMAND_INT:
        return mavlinkHandleIncomingCommandInt() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
        handleIncoming_RC_CHANNELS_OVERRIDE();
        return MAVLINK_FC_DISPATCH_HANDLED_NO_ACTIVITY;
#ifdef USE_ADSB
    case MAVLINK_MSG_ID_ADSB_VEHICLE:
        return handleIncoming_ADSB_VEHICLE() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
#endif
    case MAVLINK_MSG_ID_MLRS_RADIO_LINK_STATS:
        return handleIncoming_MLRS_RADIO_LINK_STATS(ingressPortIndex) ? MAVLINK_FC_DISPATCH_HANDLED_NO_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MLRS_RADIO_LINK_INFORMATION:
        return handleIncoming_MLRS_RADIO_LINK_INFORMATION(ingressPortIndex) ? MAVLINK_FC_DISPATCH_HANDLED_NO_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MLRS_RADIO_LINK_FLOW_CONTROL:
        return handleIncoming_MLRS_RADIO_LINK_FLOW_CONTROL(ingressPortIndex) ? MAVLINK_FC_DISPATCH_HANDLED_NO_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_RADIO_STATUS:
        handleIncoming_RADIO_STATUS();
        return MAVLINK_FC_DISPATCH_HANDLED_NO_ACTIVITY;
    default:
        return MAVLINK_FC_DISPATCH_NOT_HANDLED;
    }
}


#endif

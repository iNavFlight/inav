#include "mavlink/mavlink_internal.h"

#include "mavlink/mavlink_ports.h"
#include "mavlink/mavlink_runtime.h"
#include "mavlink/mavlink_streams.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

void freeMAVLinkTelemetryPortByIndex(uint8_t portIndex)
{
    mavlinkPortRuntime_t *state = &mavPortStates[portIndex];

    if (state->port) {
        closeSerialPort(state->port);
    }

    state->port = NULL;
    state->telemetryEnabled = false;
    state->txbuffValid = false;
    state->txbuffFree = 100;
    state->lastMavlinkMessageUs = 0;
    state->lastRxFrameUs = 0;
    state->lastHighLatencyMessageUs = 0;
    state->highLatencyEnabled = mavlinkGetPortConfig(portIndex)->high_latency;
    state->txSeq = 0;
    state->txDroppedFrames = 0;
    memset(state->mavStreamNextDue, 0, sizeof(state->mavStreamNextDue));
    memset(state->mavMessageOverrideIntervalsUs, 0, sizeof(state->mavMessageOverrideIntervalsUs));
    memset(state->mavMessageNextDue, 0, sizeof(state->mavMessageNextDue));
    memset(&state->mavRecvStatus, 0, sizeof(state->mavRecvStatus));
    memset(&state->mavRecvMsg, 0, sizeof(state->mavRecvMsg));
    memset(&state->mlrs, 0, sizeof(state->mlrs));
    resetMspPort(&mavTunnelMspPorts[portIndex], NULL);
    mavTunnelRemoteSystemIds[portIndex] = 0;
    mavTunnelRemoteComponentIds[portIndex] = 0;
}

void configureMAVLinkTelemetryPort(uint8_t portIndex)
{
    mavlinkPortRuntime_t *state = &mavPortStates[portIndex];

    if (!state->portConfig) {
        return;
    }

    baudRate_e baudRateIndex = state->portConfig->telemetry_baudrateIndex;
    if (baudRateIndex == BAUD_AUTO) {
        // default rate for minimOSD
        baudRateIndex = BAUD_57600;
    }

    state->port = openSerialPort(state->portConfig->identifier, FUNCTION_TELEMETRY_MAVLINK, NULL, NULL, baudRates[baudRateIndex], TELEMETRY_MAVLINK_PORT_MODE, SERIAL_NOT_INVERTED);
    if (!state->port) {
        return;
    }

    state->telemetryEnabled = true;
    state->txbuffValid = false;
    state->txbuffFree = 100;
    state->lastMavlinkMessageUs = 0;
    state->lastRxFrameUs = 0;
    state->lastHighLatencyMessageUs = 0;
    state->highLatencyEnabled = mavlinkGetPortConfig(portIndex)->high_latency;
    state->txSeq = 0;
    state->txDroppedFrames = 0;
    memset(state->mavStreamNextDue, 0, sizeof(state->mavStreamNextDue));
    memset(state->mavMessageOverrideIntervalsUs, 0, sizeof(state->mavMessageOverrideIntervalsUs));
    memset(state->mavMessageNextDue, 0, sizeof(state->mavMessageNextDue));
    memset(&state->mavRecvStatus, 0, sizeof(state->mavRecvStatus));
    memset(&state->mavRecvMsg, 0, sizeof(state->mavRecvMsg));
    memset(&state->mlrs, 0, sizeof(state->mlrs));
    resetMspPort(&mavTunnelMspPorts[portIndex], NULL);
    mavTunnelRemoteSystemIds[portIndex] = 0;
    mavTunnelRemoteComponentIds[portIndex] = 0;
}

#endif

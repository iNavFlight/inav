#pragma once

#include "platform.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "build/build_config.h"
#include "build/debug.h"
#include "build/version.h"

#include "common/axis.h"
#include "common/color.h"
#include "common/maths.h"
#include "common/utils.h"
#include "common/string_light.h"

#include "config/feature.h"

#include "drivers/serial.h"
#include "drivers/time.h"
#include "drivers/display.h"
#include "drivers/osd_symbols.h"

#include "fc/config.h"
#include "fc/fc_core.h"
#include "fc/fc_msp.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/failsafe.h"
#include "flight/imu.h"
#include "flight/mixer_profile.h"
#include "flight/pid.h"
#include "flight/servos.h"
#include "flight/wind_estimator.h"

#include "io/adsb.h"
#include "io/gps.h"
#include "io/ledstrip.h"
#include "io/serial.h"
#include "io/osd.h"

#include "msp/msp_protocol.h"
#include "msp/msp_serial.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "rx/rx.h"
#include "rx/mavlink.h"

#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/battery.h"
#include "sensors/boardalignment.h"
#include "sensors/gyro.h"
#include "sensors/pitotmeter.h"
#include "sensors/diagnostics.h"
#include "sensors/sensors.h"
#include "sensors/temperature.h"
#include "sensors/esc_sensor.h"

#include "mavlink/mavlink_types.h"
#include "telemetry/telemetry.h"

#include "blackbox/blackbox_io.h"

#include "scheduler/scheduler.h"

#define MAVLINK_TUNNEL_PAYLOAD_TYPE_INAV_MSP 0x8001
#define MAVLINK_TUNNEL_MSP_TIMEOUT_MS 1000
#define MAVLINK_TUNNEL_MSP_FRAMEBUF_SIZE (MSP_PORT_OUTBUF_SIZE + 16)
#define MAVLINK_MISSION_UPLOAD_TIMEOUT_MS 10000

typedef struct mavlinkContext_s {
    mavlinkPortRuntime_t portStates[MAX_MAVLINK_PORTS];
    uint8_t portCount;
    mavlinkRouteEntry_t routeTable[MAVLINK_MAX_ROUTES];
    uint8_t routeCount;
    mspPort_t tunnelMspPorts[MAX_MAVLINK_PORTS];
    uint8_t tunnelRemoteSystemIds[MAX_MAVLINK_PORTS];
    uint8_t tunnelRemoteComponentIds[MAX_MAVLINK_PORTS];
    uint8_t sendMask;
    mavlinkPortRuntime_t *activePort;
    const mavlinkTelemetryPortConfig_t *activeConfig;
    mavlink_message_t sendMsg;
    mavlink_message_t recvMsg;
    uint8_t systemId;
    uint8_t autopilotType;
    uint8_t componentId;
    uint8_t tunnelReplyPayloadBuf[MSP_PORT_OUTBUF_SIZE];
    uint8_t tunnelFrameBuf[MAVLINK_TUNNEL_MSP_FRAMEBUF_SIZE];
    int incomingMissionWpCount;
    int incomingMissionWpSequence;
    uint8_t incomingMissionSourceSystem;
    uint8_t incomingMissionSourceComponent;
    timeMs_t incomingMissionLastActivityMs;
} mavlinkContext_t;

extern mavlinkContext_t mavlinkContext;

#define mavPortStates (mavlinkContext.portStates)
#define mavPortCount (mavlinkContext.portCount)
#define mavRouteTable (mavlinkContext.routeTable)
#define mavRouteCount (mavlinkContext.routeCount)
#define mavTunnelMspPorts (mavlinkContext.tunnelMspPorts)
#define mavTunnelRemoteSystemIds (mavlinkContext.tunnelRemoteSystemIds)
#define mavTunnelRemoteComponentIds (mavlinkContext.tunnelRemoteComponentIds)
#define mavSendMask (mavlinkContext.sendMask)
#define mavActivePort (mavlinkContext.activePort)
#define mavActiveConfig (mavlinkContext.activeConfig)
#define mavSendMsg (mavlinkContext.sendMsg)
#define mavSystemId (mavlinkContext.systemId)
#define mavAutopilotType (mavlinkContext.autopilotType)
#define mavComponentId (mavlinkContext.componentId)
#define mavTunnelReplyPayloadBuf (mavlinkContext.tunnelReplyPayloadBuf)
#define mavTunnelFrameBuf (mavlinkContext.tunnelFrameBuf)
#define incomingMissionWpCount (mavlinkContext.incomingMissionWpCount)
#define incomingMissionWpSequence (mavlinkContext.incomingMissionWpSequence)
#define incomingMissionSourceSystem (mavlinkContext.incomingMissionSourceSystem)
#define incomingMissionSourceComponent (mavlinkContext.incomingMissionSourceComponent)
#define incomingMissionLastActivityMs (mavlinkContext.incomingMissionLastActivityMs)

#endif

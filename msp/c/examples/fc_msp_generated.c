/*
 * fc_msp, rewritten against the generated wire structs.
 *
 * Every struct below comes from schema/msp_v2.yaml via generator/gen_c.py.
 * None of them is declared here, and none of the byte offsets is written by
 * hand: the schema states the layout, the generator emits it, and a
 * MSP_STATIC_ASSERT in the generated header pins the size. A handler that
 * disagrees with the schema does not compile.
 *
 * Handlers are grouped by the construct they demonstrate, so this covers the
 * whole protocol rather than only the easy fixed-length cases.
 */

#include "msp_handler.h"

#include "msp_protocol.h"   /* generated: message ids + protocol constants */
#include "msp_consts.h"     /* generated: schema-declared sizes            */
#include "msp_enums.h"      /* generated: enums the payloads reference     */
#include "msp_wire_types.h" /* support types; a firmware build uses INAV's own */
#include "msp_msgs.h"       /* generated: the wire structs                 */

#include "fc_msp_stubs.h"   /* the INAV symbols a real build already provides */

/* ------------------------------------------------------------------ *
 * 1. Fixed reply, constants only.
 *    Was three sbufWriteU8() calls whose order had to match the docs.
 * ------------------------------------------------------------------ */

static mspResult_e mspFcApiVersion(sbuf_t *dst)
{
    mspApiVersionReply_t reply = {
        .mspProtocolVersion = MSP_PROTOCOL_VERSION,
        .apiVersionMajor    = API_VERSION_MAJOR,
        .apiVersionMinor    = API_VERSION_MINOR,
    };
    mspWriteReply(dst, &reply);
    return MSP_RESULT_ACK;
}

static mspResult_e mspFcVersion(sbuf_t *dst)
{
    mspFcVersionReply_t reply = {
        .fcVersionMajor = FC_VERSION_MAJOR,
        .fcVersionMinor = FC_VERSION_MINOR,
        .fcVersionPatch = FC_VERSION_PATCH_LEVEL,
    };
    mspWriteReply(dst, &reply);
    return MSP_RESULT_ACK;
}

/* ------------------------------------------------------------------ *
 * 2. Fixed reply with enums and build-conditional sources.
 *    The #ifdefs stay: they choose the *value*, never the layout, so a
 *    sensor compiled out still occupies its byte.
 * ------------------------------------------------------------------ */

static mspResult_e mspFcSensorConfig(sbuf_t *dst)
{
    mspSensorConfigReply_t reply = {
        .accHardware         = accelerometerConfig()->acc_hardware,
#ifdef USE_BARO
        .baroHardware        = barometerConfig()->baro_hardware,
#endif
#ifdef USE_MAG
        .magHardware         = compassConfig()->mag_hardware,
#endif
#ifdef USE_PITOT
        .pitotHardware       = pitotmeterConfig()->pitot_hardware,
#endif
#ifdef USE_RANGEFINDER
        .rangefinderHardware = rangefinderConfig()->rangefinder_hardware,
#endif
#ifdef USE_OPFLOW
        .opflowHardware      = opticalFlowConfig()->opflow_hardware,
#endif
    };
    mspWriteReply(dst, &reply);
    return MSP_RESULT_ACK;
}

/* ------------------------------------------------------------------ *
 * 3. Fixed reply containing arrays.
 * ------------------------------------------------------------------ */

static mspResult_e mspFcRawImu(sbuf_t *dst)
{
    /* The schema names these per axis rather than as arrays, so the fields are
     * assigned individually: the generated struct is the authority on shape. */
    mspRawImuReply_t reply = {
        .accX  = (int16_t)lrintf(acc.accADCf[0] * 512),
        .accY  = (int16_t)lrintf(acc.accADCf[1] * 512),
        .accZ  = (int16_t)lrintf(acc.accADCf[2] * 512),
        .gyroX = (int16_t)gyroRateDps(0),
        .gyroY = (int16_t)gyroRateDps(1),
        .gyroZ = (int16_t)gyroRateDps(2),
#ifdef USE_MAG
        .magX  = (int16_t)lrintf(mag.magADC[0]),
        .magY  = (int16_t)lrintf(mag.magADC[1]),
        .magZ  = (int16_t)lrintf(mag.magADC[2]),
#endif
    };
    mspWriteReply(dst, &reply);
    return MSP_RESULT_ACK;
}

/* ------------------------------------------------------------------ *
 * 4. Length-dispatched reply: MSP_VTX_CONFIG is one byte when no VTX is
 *    configured, twelve when one is. The short form is a strict prefix, so
 *    the schema marks everything after vtxDeviceType optional and the
 *    handler simply chooses how much of the struct to emit.
 * ------------------------------------------------------------------ */

static mspResult_e mspFcVtxConfig(sbuf_t *dst)
{
    mspVtxConfigReply_t reply = {0};

#ifdef USE_VTX_CONTROL
    vtxDevice_t *vtxDevice = vtxCommonDevice();
    if (vtxDevice) {
        uint8_t pitmode = 0;
        vtxCommonGetPitMode(vtxDevice, &pitmode);

        reply.vtxDeviceType     = vtxCommonGetDeviceType(vtxDevice);
        reply.band              = vtxSettingsConfig()->band;
        reply.channel           = vtxSettingsConfig()->channel;
        reply.power             = vtxSettingsConfig()->power;
        reply.pitMode           = pitmode;
        reply.vtxReady          = vtxCommonDeviceIsReady(vtxDevice) ? 1 : 0;
        reply.lowPowerDisarm    = vtxSettingsConfig()->lowPowerDisarm;
        reply.vtxTableAvailable = 1;
        reply.bandCount         = vtxDevice->capability.bandCount;
        reply.channelCount      = vtxDevice->capability.channelCount;
        reply.powerCount        = vtxDevice->capability.powerCount;
        reply.minPowerIndex     = (reply.vtxDeviceType == VTXDEV_MSP) ? 0 : 1;

        mspWriteReply(dst, &reply);
        return MSP_RESULT_ACK;
    }
#endif

    /* No VTX: only the leading field is on the wire. */
    reply.vtxDeviceType = VTXDEV_UNKNOWN;
    mspWriteReplyBytes(dst, &reply, sizeof(reply.vtxDeviceType));
    return MSP_RESULT_ACK;
}

/* ------------------------------------------------------------------ *
 * 5. Variable-length reply: a count followed by N records. The generated
 *    struct uses a C99 flexible array member, so sizeof() is the fixed head
 *    alone and sizeof(items[0]) is the record -- both compiler-checked.
 * ------------------------------------------------------------------ */

#ifdef USE_DRONECAN
static mspResult_e mspFcDronecanNodes(sbuf_t *dst)
{
    msp2InavDronecanNodesReply_t head = { .nodeCount = dronecanGetNodeCount() };
    mspWriteReplyBytes(dst, &head, sizeof(head));

    for (uint8_t i = 0; i < head.nodeCount; i++) {
        const dronecanNodeInfo_t *node = dronecanGetNode(i);
        __typeof__(head.items[0]) record = {
            .nodeID             = node->nodeID,
            .health             = node->health,
            .mode               = node->mode,
            .last_seen_ms       = millis() - node->last_seen_ms,
            .uptime_sec         = node->uptime_sec,
            .vendor_status_code = node->vendor_status_code,
        };
        sbufWriteData(dst, &record, (int)sizeof(record));
    }
    return MSP_RESULT_ACK;
}
#endif

/* ------------------------------------------------------------------ *
 * 6. Fixed SET handler with clamping. Compare against the hand-written
 *    version in fc_msp.c: identical shape, but mspSetRcTuning_t is no longer
 *    declared beside the handler, and the read/advance pair cannot be
 *    mismatched.
 * ------------------------------------------------------------------ */

static mspResult_e mspFcSetRcTuning(sbuf_t *src, int dataSize)
{
    mspSetRcTuningRequest_t pkt;
    if (!mspReadRequest(src, &pkt, dataSize)) {
        return MSP_RESULT_ERROR;
    }

    controlConfig_t *profile = (controlConfig_t *)currentControlProfile;
    profile->stabilized.rcExpo8      = pkt.rcExpo;
    profile->stabilized.rates[FD_ROLL]  = constrain(pkt.rollRate,
        SETTING_CONSTANT_ROLL_PITCH_RATE_MIN, SETTING_CONSTANT_ROLL_PITCH_RATE_MAX);
    profile->stabilized.rates[FD_PITCH] = constrain(pkt.pitchRate,
        SETTING_CONSTANT_ROLL_PITCH_RATE_MIN, SETTING_CONSTANT_ROLL_PITCH_RATE_MAX);
    profile->stabilized.rates[FD_YAW]   = constrain(pkt.yawRate,
        SETTING_YAW_RATE_MIN, SETTING_YAW_RATE_MAX);
    profile->throttle.dynPID         = MIN(pkt.dynamicThrottlePID, SETTING_TPA_RATE_MAX);
    profile->throttle.rcMid8         = pkt.throttleMid;
    profile->throttle.rcExpo8        = pkt.throttleExpo;
    profile->throttle.pa_breakpoint  = pkt.tpaBreakpoint;

    /* The 11th byte is an appended field; older senders stop before it. */
    if (mspReadOptional(src, &pkt.rcYawExpo)) {
        profile->stabilized.rcYawExpo8 = pkt.rcYawExpo;
    }

    schedulePidGainsUpdate();
    return MSP_RESULT_ACK;
}

/* ------------------------------------------------------------------ *
 * 7. SET handler with an optional trailing field that changes behaviour.
 *    The schema marks loiterRadius optional, matching fc_msp.c's
 *    "dataSize >= 4*int32+uint8" test, so the shape is declared rather
 *    than recomputed from byte arithmetic in the handler.
 * ------------------------------------------------------------------ */

static mspResult_e mspFcSetGlobalTarget(sbuf_t *src, int dataSize)
{
    msp2InavSetGlobalTargetRequest_t pkt;
    if (!mspReadRequest(src, &pkt, dataSize)) {
        return MSP_RESULT_ERROR;
    }

    int32_t loiterRadius = 0;
    if (mspReadOptional(src, &pkt.loiterRadius)) {
        loiterRadius = pkt.loiterRadius;
    }
    if (pkt.altitudeDatum == NAV_WP_TERRAIN_DATUM || loiterRadius < 0) {
        return MSP_RESULT_ERROR;
    }

    return mspApplyGlobalTarget(pkt.latitude, pkt.longitude, pkt.altitudeTarget,
                                pkt.altitudeDatum, loiterRadius)
           ? MSP_RESULT_ACK : MSP_RESULT_ERROR;
}

/* ------------------------------------------------------------------ *
 * Dispatch. Unchanged in shape from the current switch; only the bodies
 * moved out into the handlers above.
 * ------------------------------------------------------------------ */

mspResult_e mspFcProcessGenerated(uint16_t cmdMSP, sbuf_t *src, sbuf_t *dst, int dataSize)
{
    switch (cmdMSP) {
    case MSP_API_VERSION:            return mspFcApiVersion(dst);
    case MSP_FC_VERSION:             return mspFcVersion(dst);
    case MSP_SENSOR_CONFIG:          return mspFcSensorConfig(dst);
    case MSP_RAW_IMU:                return mspFcRawImu(dst);
    case MSP_VTX_CONFIG:             return mspFcVtxConfig(dst);
#ifdef USE_DRONECAN
    case MSP2_INAV_DRONECAN_NODES:   return mspFcDronecanNodes(dst);
#endif
    case MSP_SET_RC_TUNING:          return mspFcSetRcTuning(src, dataSize);
    case MSP2_INAV_SET_GLOBAL_TARGET: return mspFcSetGlobalTarget(src, dataSize);
    default:                         return MSP_RESULT_CMD_UNKNOWN;
    }
}

// Compile-time proof that the generated headers describe the wire correctly.
// Sizes here are checked against INAV source, not against the generator.
#include <stdio.h>
#include <msp_wire_types.h>
#include <msp_msgs.h>
#include <msp_enums.h>

// Hand-verified against INAV source; if the schema drifts, these fail.
MSP_STATIC_ASSERT(sizeof(escSensorData_t) == 16, escSensorData_t_wire);
MSP_STATIC_ASSERT(sizeof(ledConfig_t) == 5, ledConfig_t_wire);
MSP_STATIC_ASSERT(sizeof(msp2SensorHeadtrackerRequest_t) == 9, headtracker_wire);
MSP_STATIC_ASSERT(sizeof(mspApiVersionReply_t) == 3, api_version_wire);

// A flexible-array payload: sizeof is the fixed header, records follow.
MSP_STATIC_ASSERT(sizeof(msp2InavDronecanNodesReply_t) == 1, dronecan_hdr);

int main(void)
{
    msp2InavDronecanNodesReply_t *nodes = NULL;
    if (sizeof(nodes->items[0]) != 13) {
        printf("dronecan record is %zu, expected 13\n", sizeof(nodes->items[0]));
        return 1;
    }
    mspFcVersionReply_t version = { 9, 1, 0 };
    printf("ok: %u messages, FC %u.%u.%u\n", (unsigned)MSP_FC_VERSION,
           version.fcVersionMajor, version.fcVersionMinor, version.fcVersionPatch);
    return 0;
}

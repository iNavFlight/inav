#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "mavlink/mavlink_types.h"

bool mavlinkIsFromLocalIdentity(uint8_t sysid, uint8_t compid);
void mavlinkLearnRoute(uint8_t ingressPortIndex);
void mavlinkExtractTargets(const mavlink_message_t *msg, int16_t *targetSystem, int16_t *targetComponent);
void mavlinkForwardMessage(uint8_t ingressPortIndex, int16_t targetSystem, int16_t targetComponent);
int8_t mavlinkResolveLocalPortForTarget(int16_t targetSystem, int16_t targetComponent, uint8_t ingressPortIndex);
bool mavlinkShouldFanOutLocalBroadcast(const mavlink_message_t *msg);

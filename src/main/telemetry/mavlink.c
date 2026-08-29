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

/*
 * telemetry_mavlink.c
 *
 * Author: Konstantin Sharlaimov
 */

#include "platform.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

#include "mavlink/mavlink_runtime.h"

#include "telemetry/mavlink.h"

void initMAVLinkTelemetry(void)
{
    mavlinkRuntimeInit();
}

void handleMAVLinkTelemetry(timeUs_t currentTimeUs)
{
    mavlinkRuntimeHandle(currentTimeUs);
}

void checkMAVLinkTelemetryState(void)
{
    mavlinkRuntimeCheckState();
}

void freeMAVLinkTelemetryPort(void)
{
    mavlinkRuntimeFreePorts();
}

#endif

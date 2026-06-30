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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "common/time.h"
#include "common/vector.h"

#include "navigation/navigation_vtol_mc_protection_logic.h"

#ifdef USE_AUTO_TRANSITION
bool navigationVtolMcProtectionIsVtolMcMode(void);
bool navigationVtolMcProtectionIsNavActive(void);
bool navigationVtolMcProtectionVelocityUsable(void);
uint16_t navigationVtolMcProtectionMaxAbsAttitudeDeciDeg(void);

vtolMcProtectionThrottleBounds_t navigationVtolMcProtectionGetThrottleBounds(int16_t idleThrottle, int16_t hoverThrottle, int16_t maxThrottle);
bool navigationVtolMcProtectionShouldFreezeAltitudeIntegrator(void);
int16_t navigationVtolMcProtectionApplyBailoutThrottle(int16_t requestedThrottle, const vtolMcProtectionThrottleBounds_t *bounds, int16_t hoverThrottle);

bool navigationVtolMcProtectionApplyCapture(uint32_t navStateFlags);
bool navigationVtolMcProtectionApplySoftAltitudeCapture(uint32_t navStateFlags);
bool navigationVtolMcProtectionLandingSettleReady(const fpVector3_t *landingPos);
void navigationVtolMcProtectionApplyStabilizedCommandShaping(int16_t *rollCommand, int16_t *pitchCommand, int16_t *yawCommand);
void navigationVtolMcProtectionPublishThrottleDebug(const vtolMcProtectionThrottleBounds_t *bounds, int16_t protectedThrottle);
void navigationVtolMcProtectionResetTransientStates(void);
#else
static inline bool navigationVtolMcProtectionIsVtolMcMode(void) { return false; }
static inline bool navigationVtolMcProtectionIsNavActive(void) { return false; }
static inline bool navigationVtolMcProtectionVelocityUsable(void) { return false; }
static inline uint16_t navigationVtolMcProtectionMaxAbsAttitudeDeciDeg(void) { return 0; }
static inline vtolMcProtectionThrottleBounds_t navigationVtolMcProtectionGetThrottleBounds(int16_t idleThrottle, int16_t hoverThrottle, int16_t maxThrottle)
{
    return vtolMcProtectionComputeThrottleBounds(false, idleThrottle, hoverThrottle, maxThrottle, 0);
}
static inline bool navigationVtolMcProtectionShouldFreezeAltitudeIntegrator(void) { return false; }
static inline int16_t navigationVtolMcProtectionApplyBailoutThrottle(int16_t requestedThrottle, const vtolMcProtectionThrottleBounds_t *bounds, int16_t hoverThrottle)
{
    (void)bounds;
    (void)hoverThrottle;
    return requestedThrottle;
}
static inline bool navigationVtolMcProtectionApplyCapture(uint32_t navStateFlags)
{
    (void)navStateFlags;
    return false;
}
static inline bool navigationVtolMcProtectionApplySoftAltitudeCapture(uint32_t navStateFlags)
{
    (void)navStateFlags;
    return false;
}
static inline bool navigationVtolMcProtectionLandingSettleReady(const fpVector3_t *landingPos)
{
    (void)landingPos;
    return true;
}
static inline void navigationVtolMcProtectionApplyStabilizedCommandShaping(int16_t *rollCommand, int16_t *pitchCommand, int16_t *yawCommand)
{
    (void)rollCommand;
    (void)pitchCommand;
    (void)yawCommand;
}
static inline void navigationVtolMcProtectionPublishThrottleDebug(const vtolMcProtectionThrottleBounds_t *bounds, int16_t protectedThrottle)
{
    (void)bounds;
    (void)protectedThrottle;
}
static inline void navigationVtolMcProtectionResetTransientStates(void) {}
#endif

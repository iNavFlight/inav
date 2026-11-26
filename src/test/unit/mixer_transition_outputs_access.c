// The host has no MCU_FLASH_SIZE; common.h otherwise selects a one-profile
// small-flash target. Exercise the real controller with its two-profile layout.
#include "target/common.h"
#undef MAX_MIXER_PROFILE_COUNT
#define MAX_MIXER_PROFILE_COUNT 2
#include "flight/mixer_profile.c"

void testResetMixerTransitionRuntime(void)
{
    manualTransitionModeWasActive = false;
    manualTransitionReadyForEdge = true;
    manualTransitionSessionMode = MIXER_TRANSITION_MANUAL_SESSION_NONE;
    manualFwToMcProtectionLatched = false;
    manualFwToMcProtectionTransitionActive = false;
    fwToMcProtectionAirspeedConfirmed = false;
    fwToMcProtectionAirspeedConfirmation = (mixerTransitionConditionConfirmationState_t){0};
    mixerTransitionServoInput = 0;
    navigationProfileSwitchWasOwned = false;
    navigationProfileHandbackPending = false;
    manualProfileSwitchAutoTransitionActive = false;
    isMixerTransitionMixing = false;
    isMixerTransitionMixing_requested = false;
}

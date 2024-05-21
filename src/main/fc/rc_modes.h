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

#include "common/bitarray.h"

#include "config/parameter_group.h"

#define BOXID_NONE 255

typedef enum {
    BOXARM,             // 0
    BOXANGLE,
    BOXHORIZON,
    BOXANGLEHOLD,
    BOXMANUAL,
    BOXNAVALTHOLD,
    BOXHEADINGHOLD,
    BOXNAVPOSHOLD,
    BOXNAVCOURSEHOLD,
    BOXNAVCRUISE,
    BOXNAVRTH,          // 10
    BOXNAVWP,
    BOXNAVLAUNCH,
    BOXSOARING,
    BOXFAILSAFE,
    BOXAUTOLEVEL,
    BOXHEADFREE,
    BOXHEADADJ,
    BOXBEEPERON,
    BOXAUTOTRIM,
    BOXAUTOTUNE,        // 20
    BOXOSD,
    BOXTELEMETRY,
    BOXBLACKBOX,
    BOXAIRMODE,
    BOXGCSNAV,
    BOXSURFACE,
    BOXFLAPERON,
    BOXTURNASSIST,
    BOXBRAKING,
    BOXMIXERPROFILE,    // 30
    BOXMIXERTRANSITION, // end of blackbox boxmode record
    BOXHOMERESET,
    BOXCAMSTAB,
    BOXLIGHTS,
    BOXCAMERA1,
    BOXCAMERA2,
    BOXCAMERA3,
    BOXOSDALT1,
    BOXOSDALT2,
    BOXOSDALT3,         // 40
    BOXLEDLOW,
    BOXUSER1,
    BOXUSER2,
    BOXUSER3,
    BOXUSER4,
    BOXFPVANGLEMIX,
    BOXLOITERDIRCHN,
    BOXMSPRCOVERRIDE,
    BOXPREARM,
    BOXTURTLE,          // 50
    BOXPLANWPMISSION,
    BOXCHANGEMISSION,
    BOXBEEPERMUTE,
    BOXMULTIFUNCTION,
    CHECKBOX_ITEM_COUNT
} boxId_e;

// type to hold enough bits for CHECKBOX_ITEM_COUNT. Struct used for value-like behavior
typedef struct boxBitmask_s { BITARRAY_DECLARE(bits, CHECKBOX_ITEM_COUNT); } boxBitmask_t;

#define MAX_MODE_ACTIVATION_CONDITION_COUNT 40

#define CHANNEL_RANGE_MIN 900
#define CHANNEL_RANGE_MAX 2100

#define CHANNEL_RANGE_STEP_WIDTH 25

#define MODE_STEP_TO_CHANNEL_VALUE(step) (CHANNEL_RANGE_MIN + CHANNEL_RANGE_STEP_WIDTH * step)
#define CHANNEL_VALUE_TO_STEP(channelValue) ((constrain(channelValue, CHANNEL_RANGE_MIN, CHANNEL_RANGE_MAX) - CHANNEL_RANGE_MIN) / CHANNEL_RANGE_STEP_WIDTH)

#define MIN_MODE_RANGE_STEP 0
#define MAX_MODE_RANGE_STEP ((CHANNEL_RANGE_MAX - CHANNEL_RANGE_MIN) / CHANNEL_RANGE_STEP_WIDTH)

#define IS_RANGE_USABLE(range) ((range)->startStep < (range)->endStep)

// steps are 25 apart
// a value of 0 corresponds to a channel value of 900 or less
// a value of 48 corresponds to a channel value of 2100 or more
// 48 steps between 900 and 1200
typedef struct channelRange_s {
    uint8_t startStep;
    uint8_t endStep;
} channelRange_t;

typedef struct modeActivationCondition_s {
    boxId_e modeId;
    uint8_t auxChannelIndex;
    channelRange_t range;
} modeActivationCondition_t;

typedef enum {
    MODE_OPERATOR_OR, // default
    MODE_OPERATOR_AND
} modeActivationOperator_e;

typedef struct modeActivationOperatorConfig_s {
    modeActivationOperator_e modeActivationOperator;
} modeActivationOperatorConfig_t;

PG_DECLARE_ARRAY(modeActivationCondition_t, MAX_MODE_ACTIVATION_CONDITION_COUNT, modeActivationConditions);
PG_DECLARE(modeActivationOperatorConfig_t, modeActivationOperatorConfig);

bool IS_RC_MODE_ACTIVE(boxId_e boxId);
void rcModeUpdate(boxBitmask_t *newState);

bool isModeActivationConditionPresent(boxId_e modeId);

void processAirmode(void);
bool isUsingNavigationModes(void);
bool isRangeActive(uint8_t auxChannelIndex, const channelRange_t *range);

void updateActivatedModes(void);
void updateUsedModeActivationConditionFlags(void);

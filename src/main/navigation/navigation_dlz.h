#pragma once

#include <stdio.h>
#include "navigation_private.h"

typedef struct {
    bool        active; // If DLZ data is valid
    timeMs_t    lastUpdateTime; // millis
    float       fadeValue;
    int16_t     posX;         
    int16_t     posY;         
    int16_t     posZ;         
} NavDlzData_t;

extern NavDlzData_t NavDlzData;


// Func prototypes

fpVector2_t navigationDLZUpdateState(
    navigationDesiredState_t* desiredState, 
    const float posErrorX, 
    const float posErrorY);



// Inline funcs


static inline void navigationDLZFader(const float target)
{

    static const float FADE_STEP = 0.05f;

    if (NavDlzData.fadeValue < target) {
        NavDlzData.fadeValue += FADE_STEP;
        if (NavDlzData.fadeValue > target) {
            NavDlzData.fadeValue = target;
        }
    } else if (NavDlzData.fadeValue > target) {
        NavDlzData.fadeValue -= FADE_STEP;
        if (NavDlzData.fadeValue < target) {
            NavDlzData.fadeValue = target;
        }
    }

    NavDlzData.fadeValue = constrainf(NavDlzData.fadeValue, 0.0f, 1.0f);

}

static inline void navigationDLZReset(void) {
    NavDlzData.active = false;
    NavDlzData.lastUpdateTime = 0;
    NavDlzData.fadeValue = 0.0f;
    NavDlzData.posX = 0;
    NavDlzData.posY = 0;
    NavDlzData.posZ = 0;
}


static inline void navigationDLZOnRxData(const int16_t posX, const int16_t posY, const int16_t posZ)
{
    NavDlzData.lastUpdateTime = millis();
    NavDlzData.posX = posX;
    NavDlzData.posY = posY;
    NavDlzData.posZ = posZ;
}


#pragma once

#include <stdio.h>
#include "navigation_private.h"
#include "drivers/time.h"

typedef struct {
    bool        active; // If DLZ data is valid
    timeMs_t    lastUpdateTime; // millis
    //float       fadeValue;
    //bool        mspWpUpdate;

    int16_t     posX;         
    int16_t     posY;         
    int16_t     velZ;       // Vertical speed command in cm/s
    int16_t     gpsFade;    // How much gps signal to use (1000 - full cam, 0 - full gps)

                            
} NavDlzData_t;


typedef struct __attribute__((packed)) {
    int16_t  posX;
    int16_t  posY;
    int16_t  velZ;
    int16_t  gpsFade;
} mspSensorSkyvis_t;


extern NavDlzData_t NavDlzData;


// Func prototypes

fpVector2_t navigationDLZUpdateState(
    navigationDesiredState_t* desiredState, 
    const float posErrorX, 
    const float posErrorY);


const float navigationDLZLandingController(
    const float vspd_in, 
    const int32_t landingElevation);


void mspSkyvisReceiveNewData(
    const uint8_t * bufferPtr, 
    unsigned int dataSize);



// Inline funcs


//static inline void navigationDLZFader(const float target)
//{
//
//    static const float FADE_STEP = 0.05f;
//
//    if (NavDlzData.fadeValue < target) {
//        NavDlzData.fadeValue += FADE_STEP;
//        if (NavDlzData.fadeValue > target) {
//            NavDlzData.fadeValue = target;
//        }
//    } else if (NavDlzData.fadeValue > target) {
//        NavDlzData.fadeValue -= FADE_STEP;
//        if (NavDlzData.fadeValue < target) {
//            NavDlzData.fadeValue = target;
//        }
//    }
//
//    NavDlzData.fadeValue = constrainf(NavDlzData.fadeValue, 0.0f, 1.0f);
//
//}

static inline void navigationDLZReset(void) {
    NavDlzData.active = false;
    //NavDlzData.lastUpdateTime = 0; // Should not nullyfy !!!
    //NavDlzData.fadeValue = 0.0f;
    //NavDlzData.mspWpUpdate = false;
    NavDlzData.posX = 0;
    NavDlzData.posY = 0;
    NavDlzData.velZ = 0;
    NavDlzData.gpsFade = 0;
}


static inline float scaleRangeClippedf(float x, float srcMin, float srcMax, float destMin, float destMax) {
    float a = (destMax - destMin) * (x - srcMin);
    float b = srcMax - srcMin;
    float c = (a / b) + destMin;
    return fmin(fmax(c, destMin), destMax);
}
#include "navigation_dlz.h"

#define UPDATE_TIMEOUT_MS 1000  // if no update in this time, DLZ is considered lost
#define MAX_POS_CMD_METERS 20.0f


fpVector2_t navigationDLZUpdateState(
    navigationDesiredState_t* desiredState, 
    const float posErrorX, 
    const float posErrorY)
{
    (void)desiredState;
    fpVector2_t offset;

    if (NavDlzData.active == false) {
        navigationDLZReset();
        offset.x = posErrorX;
        offset.y = posErrorY;
        return offset;
    }

    if (millis() - NavDlzData.lastUpdateTime < UPDATE_TIMEOUT_MS) {
        navigationDLZFader(1.0f);
    } else {
        navigationDLZFader(0.0f);
    }

    float cosYaw = posControl.actualState.cosYaw;
    float sinYaw = posControl.actualState.sinYaw;

    float dlzx = (cosYaw * NavDlzData.posX - sinYaw * NavDlzData.posY);  // centimeters
    float dlzy = (sinYaw * NavDlzData.posX + cosYaw * NavDlzData.posY);

    dlzx = constrainf(dlzx, -MAX_POS_CMD_METERS * 100.0f, MAX_POS_CMD_METERS * 100.0f);
    dlzy = constrainf(dlzy, -MAX_POS_CMD_METERS * 100.0f, MAX_POS_CMD_METERS * 100.0f);

    offset.x = (1.0f - NavDlzData.fadeValue) * posErrorX + NavDlzData.fadeValue * dlzx; // centimeters
    offset.y = (1.0f - NavDlzData.fadeValue) * posErrorY + NavDlzData.fadeValue * dlzy; // centimeters

    return offset;
}

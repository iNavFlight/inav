#include "navigation_dlz.h"

#define UPDATE_TIMEOUT_MS 1000  // if no update in this time, DLZ is considered lost
#define MAX_POS_CMD_METERS 20.0f


fpVector2_t navigationDLZUpdateState(
    navigationDesiredState_t* desiredState, 
    const float posErrorX, 
    const float posErrorY)
{

//    printf("Num WP: %d\n", posControl.waypointCount);
//
//    // Debug    : print waypoint list (index, action, flag, lat/lon/alt, p1/p2/p3)
//    for (int i = 0; i < posControl.waypointCount; ++i) {
//        const navWaypoint_t *wp = &posControl.waypointList[i];
//        printf("WP[%d]: action=%u flag=0x%02x lat=%ld lon=%ld alt=%ld p1=%d p2=%d p3=%d\n",
//               i,
//               (unsigned)wp->action,
//               (unsigned)wp->flag,
//               (long)wp->lat,
//               (long)wp->lon,
//               (long)wp->alt,
//               (int)wp->p1,
//               (int)wp->p2,
//               (int)wp->p3);
//    }
//    
//    printf("WP active index: %d\n", posControl.activeWaypointIndex);
//    printf("WP list valid: %s\n", posControl.waypointListValid ? "TRUE" : "FALSE");
//    printf("Nav FMS state: %d\n", (int)posControl.navState);

    // 

    (void)desiredState;
    fpVector2_t offset;

    if (NavDlzData.active == false) {
        navigationDLZReset();
        offset.x = posErrorX;
        offset.y = posErrorY;
        return offset;
    }


    float fadeValue = (float)NavDlzData.gpsFade / 1000.0f;

    if (millis() - NavDlzData.lastUpdateTime > UPDATE_TIMEOUT_MS)
        fadeValue = 0.0f;

    float cosYaw = posControl.actualState.cosYaw;
    float sinYaw = posControl.actualState.sinYaw;

    float dlzx = (cosYaw * NavDlzData.posX - sinYaw * NavDlzData.posY);  // centimeters
    float dlzy = (sinYaw * NavDlzData.posX + cosYaw * NavDlzData.posY);

    dlzx = constrainf(dlzx, -MAX_POS_CMD_METERS * 100.0f, MAX_POS_CMD_METERS * 100.0f);
    dlzy = constrainf(dlzy, -MAX_POS_CMD_METERS * 100.0f, MAX_POS_CMD_METERS * 100.0f);

    offset.x = (1.0f - fadeValue) * posErrorX + fadeValue * dlzx; // centimeters
    offset.y = (1.0f - fadeValue) * posErrorY + fadeValue * dlzy; // centimeters

    return offset;
}



const float navigationDLZLandingController(const float vspd_in, const int32_t landingElevation) {


    //posControl.fwLandState.landAltAgl
    const float radalt = navGetCurrentActualPositionAndVelocity()->pos.z - (float)landingElevation;

    //printf("DLZ.Land.RadAlt= %.2f\n", radalt);
    //printf("DLZ.Land.AglAlt= %d\n", posControl.fwLandState.landAltAgl);
    //printf("DLZ.Land.LndElv= %d\n", landingElevation);
    //printf("DLZ.Land.VsIN= %.2f\n", vspd_in);


    // Fallback if signal lost
    if (millis() - NavDlzData.lastUpdateTime > UPDATE_TIMEOUT_MS) {
        return vspd_in;
    }

    float vspd = -(float)NavDlzData.velZ; // cm/s 

    // Sanity check
    vspd = fmin(fmax(vspd, -100.0f), 200.0f);

    if (radalt < 500.0f) {
       vspd = fmin(fmax(vspd, -100.0f), 50.0f); 
    }

    //printf("DLZ.Land.VsOUT= %.2f\n", vspd);
    //printf("# ---------------------------------------# \n");
    return vspd;

}

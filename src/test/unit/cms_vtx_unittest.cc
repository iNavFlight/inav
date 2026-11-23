#include <cstdarg>
#include <cstdio>
#include <cstring>
#include "gtest/gtest.h"

extern "C" {
#include "platform.h"
#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_vtx.h"
#include "drivers/vtx_common.h"
#include "io/vtx.h"
#include "fc/runtime_config.h"

uint32_t armingFlags;
vtxSettingsConfig_t vtxSettingsConfig_System;
static vtxDevice_t device{};
static bool available;
static uint8_t sentPower;
vtxDevice_t *vtxCommonDevice(void) { return available ? &device : nullptr; }
bool vtxCommonGetDeviceCapability(vtxDevice_t *dev, vtxDeviceCapability_t *out) {
    if (!dev) return false;
    *out = dev->capability;
    return true;
}
bool vtxCommonDeviceIsReady(vtxDevice_t *) { return false; }
bool vtxCommonGetPitMode(vtxDevice_t *, uint8_t *out) { *out = 0; return true; }
bool vtxCommonGetOsdInfo(vtxDevice_t *, vtxDeviceOsdInfo_t *) { return false; }
void vtxCommonSetBandAndChannel(vtxDevice_t *, uint8_t, uint8_t) {}
void vtxCommonSetPowerByIndex(vtxDevice_t *, uint8_t power) { sentPower = power; }
void vtxCommonSetPitMode(vtxDevice_t *, uint8_t) {}
void saveConfigAndNotify(void) {}
int tfp_sprintf(char *out, const char *format, ...) {
    va_list args; va_start(args, format);
    const int result = vsprintf(out, format, args);
    va_end(args); return result;
}
}

class CmsVtxTest : public ::testing::Test {
protected:
    const OSD_Entry *powerEntry;
    OSD_TAB_t *power;
    static char *twoLevels[3];
    static char *fiveLevels[6];
    void SetUp() override {
        memset(&vtxSettingsConfig_System, 0, sizeof(vtxSettingsConfig_System));
        memset(&device.capability, 0, sizeof(device.capability));
        available = true; armingFlags = 0; sentPower = 0;
        device.capability.powerCount = 2;
        device.capability.powerNames = twoLevels;
        for (const OSD_Entry *entry = cmsx_menuVtxControl.entries; entry->type != OME_BACK_AND_END; entry++) {
            if (!strcmp(entry->text, "POWER")) {
                powerEntry = entry;
                power = (OSD_TAB_t *)entry->data;
            }
        }
    }
    void enter(uint8_t index) {
        vtxSettingsConfig_System.power = index;
        cmsx_menuVtxControl.onEnter(nullptr);
    }
    void confirm() {
        for (const OSD_Entry *entry = cmsx_menuVtxControl.entries; entry->type != OME_BACK_AND_END; entry++) {
            if (entry->type == OME_Submenu) {
                const CMS_Menu *menu = (const CMS_Menu *)entry->data;
                menu->entries[1].func(nullptr, nullptr);
                return;
            }
        }
        FAIL() << "Missing confirmation menu";
    }
};
char *CmsVtxTest::twoLevels[] = {(char *)"---", (char *)"25", (char *)"400"};
char *CmsVtxTest::fiveLevels[] = {(char *)"---", (char *)"25", (char *)"100", (char *)"200", (char *)"400", (char *)"600"};

TEST_F(CmsVtxTest, SavedIndexIsClampedBeforePowerNameIsRead) {
    enter(8);
    ASSERT_EQ(2, *power->val);
    EXPECT_STREQ("400", power->names[*power->val]);
    EXPECT_EQ(8, vtxSettingsConfig_System.power);
}

TEST_F(CmsVtxTest, OpenMenuRefreshesAfterCapabilityDetection) {
    device.capability.powerCount = 5; device.capability.powerNames = fiveLevels;
    enter(5);
    device.capability.powerCount = 2; device.capability.powerNames = twoLevels;
    cmsVtxUpdatePowerMetadata();
    ASSERT_EQ(2, power->max); ASSERT_EQ(2, *power->val);
    EXPECT_STREQ("400", power->names[*power->val]);
    EXPECT_TRUE(powerEntry->flags & DYNAMIC);
}

TEST_F(CmsVtxTest, RefreshPreservesValidUnsavedSelection) {
    enter(2); *power->val = 1;
    cmsVtxUpdatePowerMetadata();
    EXPECT_EQ(1, *power->val);
    EXPECT_EQ(2, vtxSettingsConfig_System.power);
}

TEST_F(CmsVtxTest, MissingDeviceHasSafeUnknownPowerSelection) {
    available = false; enter(8);
    EXPECT_EQ(0, power->max); ASSERT_EQ(0, *power->val);
    powerEntry->func(nullptr, power);
    ASSERT_EQ(0, *power->val);
    EXPECT_STREQ("---", power->names[*power->val]);
}

TEST_F(CmsVtxTest, ConfirmationRefreshesBeforeSendingOrSaving) {
    device.capability.powerCount = 5; device.capability.powerNames = fiveLevels;
    enter(5);
    device.capability.powerCount = 2; device.capability.powerNames = twoLevels;
    confirm();
    EXPECT_EQ(2, sentPower);
    EXPECT_EQ(2, vtxSettingsConfig_System.power);
}

TEST_F(CmsVtxTest, MissingDeviceCannotSaveTheUnknownPowerPlaceholder) {
    available = false; enter(8);
    confirm();
    EXPECT_EQ(0, sentPower);
    EXPECT_EQ(8, vtxSettingsConfig_System.power);
}

#include <cstring>
#include "gtest/gtest.h"
extern "C" {
#include "platform.h"
#include "config/vtx_settings_migration.h"
PG_REGISTER_WITH_RESET_TEMPLATE(uint32_t, migrationTestSection, 4090, 0);
PG_RESET_TEMPLATE(uint32_t, migrationTestSection, 0);
}
TEST(VtxSettingsMigration, LoaderPreservesEveryOldFieldAndStartsWithAutomaticTable) {
    vtxSettingsConfigV2_t old{};
    old.band=5; old.channel=8; old.power=4; old.pitModeChan=5658;
    old.lowPowerDisarm=2; old.maxPowerOverride=2500; old.frequencyGroup=2;
    vtxSettingsConfig_t dest;
    memset(&dest,0xff,sizeof(dest));
    pgRegistry_t reg{}; reg.pgn=PG_VTX_SETTINGS_CONFIG | (3<<12);
    reg.size=sizeof(dest); reg.address=reinterpret_cast<uint8_t *>(&dest);
    pgLoad(&reg,0,&old,sizeof(old),2);
    EXPECT_EQ(old.band,dest.band); EXPECT_EQ(old.channel,dest.channel); EXPECT_EQ(old.power,dest.power);
    EXPECT_EQ(old.pitModeChan,dest.pitModeChan); EXPECT_EQ(old.lowPowerDisarm,dest.lowPowerDisarm);
    EXPECT_EQ(old.maxPowerOverride,dest.maxPowerOverride); EXPECT_EQ(old.frequencyGroup,dest.frequencyGroup);
    for(auto level:dest.trampPowerLevels) EXPECT_EQ(0,level);
    const auto current=dest;
    pgLoad(&reg,0,&current,sizeof(current),3);
    EXPECT_EQ(0,memcmp(&current,&dest,sizeof(dest)));
    EXPECT_FALSE(pgMigrateVtxSettings(&reg,&dest,&old,sizeof(old)-1,2));
    EXPECT_FALSE(pgMigrateVtxSettings(&reg,&dest,&old,sizeof(old),1));
    reg.pgn=PG_VTX_SETTINGS_CONFIG | (4<<12);
    EXPECT_FALSE(pgMigrateVtxSettings(&reg,&dest,&old,sizeof(old),3));
}

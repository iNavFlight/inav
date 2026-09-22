#include <array>
#include <cstring>
#include <vector>
#include "gtest/gtest.h"
extern "C" {
#include "platform.h"
#include "config/profile_name_migration.h"
PG_REGISTER_WITH_RESET_TEMPLATE(uint32_t, migrationTestSection, 4090, 0);
PG_RESET_TEMPLATE(uint32_t, migrationTestSection, 0);
}

template<class T> static void checkMigration(pgn_t id, int version)
{
    constexpr size_t count = 3;
    const size_t prefix = offsetof(T, name);
    const size_t oldStride = (prefix + alignof(T) - 1) / alignof(T) * alignof(T);
    std::vector<uint8_t> old(count * oldStride, 0xee);
    std::array<T, count> dest;
    for (size_t slot=0; slot<count; slot++) {
        for (size_t byte=0; byte<prefix; byte++) old[slot*oldStride+byte]=(slot*71+byte)%256;
    }
    pgRegistry_t reg{};
    reg.pgn=id | (version<<12);
    reg.size=sizeof(dest);
    reg.address=reinterpret_cast<uint8_t *>(dest.data());
    std::memset(dest.data(),0xab,sizeof(dest));
    ASSERT_TRUE(pgMigrateProfileNames(&reg,dest.data(),old.data(),old.size(),version-1));
    for (size_t slot=0;slot<count;slot++) {
        EXPECT_EQ(0,std::memcmp(&dest[slot],old.data()+slot*oldStride,prefix));
        for (char byte:dest[slot].name) EXPECT_EQ(0,byte);
    }
    // Exercise the actual loader as well, including its reset-before-migration.
    std::memset(dest.data(),0xab,sizeof(dest));
    pgLoad(&reg,0,old.data(),old.size(),version-1);
    for (size_t slot=0;slot<count;slot++) {
        EXPECT_EQ(0,std::memcmp(&dest[slot],old.data()+slot*oldStride,prefix));
        for (char byte:dest[slot].name) EXPECT_EQ(0,byte);
    }
    // A current-version round trip retains names as well as prior fields.
    dest[1].name[0]='B';
    const auto current=dest;
    pgLoad(&reg,0,current.data(),sizeof(current),version);
    EXPECT_EQ(0,std::memcmp(dest.data(),current.data(),sizeof(dest)));
    // Rejected versions/sizes must leave reset defaults untouched.
    const auto migrated=dest;
    EXPECT_FALSE(pgMigrateProfileNames(&reg,dest.data(),old.data(),old.size()-1,version-1));
    EXPECT_FALSE(pgMigrateProfileNames(&reg,dest.data(),old.data(),old.size()+1,version-1));
    EXPECT_FALSE(pgMigrateProfileNames(&reg,dest.data(),old.data(),-1,version-1));
    EXPECT_FALSE(pgMigrateProfileNames(&reg,dest.data(),old.data(),old.size(),version-2));
    EXPECT_FALSE(pgMigrateProfileNames(&reg,dest.data(),old.data(),old.size(),version));
    reg.pgn=id | ((version+1)<<12);
    EXPECT_FALSE(pgMigrateProfileNames(&reg,dest.data(),old.data(),old.size(),version));
    EXPECT_EQ(0,std::memcmp(dest.data(),migrated.data(),sizeof(dest)));
}
TEST(ProfileNameMigration, ControlSlotsRetainTheirOwnFields) { checkMigration<controlConfig_t>(PG_CONTROL_PROFILES,1); }
TEST(ProfileNameMigration, BatterySlotsRetainTheirOwnFields) { checkMigration<batteryProfile_t>(PG_BATTERY_PROFILES,5); }
TEST(ProfileNameMigration, MixerSlotsRetainTheirOwnFields) {
#ifdef USE_AUTO_TRANSITION
    checkMigration<mixerProfile_t>(PG_MIXER_PROFILE,5);
#else
    checkMigration<mixerProfile_t>(PG_MIXER_PROFILE,2);
#endif
}
TEST(ProfileNameMigration, UnrelatedGroupsAreNotMigrated) {
    pgRegistry_t reg{}; reg.pgn=PG_OSD_LAYOUTS_CONFIG;
    EXPECT_FALSE(pgMigrateProfileNames(&reg,nullptr,nullptr,0,4));
}

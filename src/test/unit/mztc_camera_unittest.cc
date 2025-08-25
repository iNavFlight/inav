#include <gtest/gtest.h>

#ifdef USE_MZTC

// Include the thermal camera configuration header
#include "config/mztc_camera.h"

class MztcCameraTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup needed for structure/enum tests
    }
    
    void TearDown() override {
        // No cleanup needed
    }
};

// Test enum definitions
TEST_F(MztcCameraTest, EnumDefinitions) {
    // Test mode enums
    EXPECT_EQ(MZTC_MODE_DISABLED, 0);
    EXPECT_EQ(MZTC_MODE_STANDBY, 1);
    EXPECT_EQ(MZTC_MODE_CONTINUOUS, 2);
    EXPECT_EQ(MZTC_MODE_TRIGGERED, 3);
    EXPECT_EQ(MZTC_MODE_ALERT, 4);
    EXPECT_EQ(MZTC_MODE_RECORDING, 5);
    EXPECT_EQ(MZTC_MODE_CALIBRATION, 6);
    EXPECT_EQ(MZTC_MODE_SURVEILLANCE, 7);
    
    // Test temperature unit enums
    EXPECT_EQ(MZTC_UNIT_CELSIUS, 0);
    EXPECT_EQ(MZTC_UNIT_FAHRENHEIT, 1);
    EXPECT_EQ(MZTC_UNIT_KELVIN, 2);
    
    // Test palette enums
    EXPECT_EQ(MZTC_PALETTE_WHITE_HOT, 0);
    EXPECT_EQ(MZTC_PALETTE_BLACK_HOT, 1);
    EXPECT_EQ(MZTC_PALETTE_FUSION_1, 2);
    EXPECT_EQ(MZTC_PALETTE_RAINBOW, 3);
    EXPECT_EQ(MZTC_PALETTE_FUSION_2, 4);
    EXPECT_EQ(MZTC_PALETTE_IRON_RED_1, 5);
    EXPECT_EQ(MZTC_PALETTE_IRON_RED_2, 6);
    EXPECT_EQ(MZTC_PALETTE_SEPIA, 7);
    EXPECT_EQ(MZTC_PALETTE_COLOR_1, 8);
    EXPECT_EQ(MZTC_PALETTE_COLOR_2, 9);
    EXPECT_EQ(MZTC_PALETTE_ICE_FIRE, 10);
    EXPECT_EQ(MZTC_PALETTE_RAIN, 11);
    EXPECT_EQ(MZTC_PALETTE_GREEN_HOT, 12);
    EXPECT_EQ(MZTC_PALETTE_RED_HOT, 13);
    
    // Test zoom level enums
    EXPECT_EQ(MZTC_ZOOM_1X, 0);
    EXPECT_EQ(MZTC_ZOOM_2X, 1);
    EXPECT_EQ(MZTC_ZOOM_4X, 2);
    EXPECT_EQ(MZTC_ZOOM_8X, 3);
    
    // Test mirror mode enums
    EXPECT_EQ(MZTC_MIRROR_NONE, 0);
    EXPECT_EQ(MZTC_MIRROR_HORIZONTAL, 1);
    EXPECT_EQ(MZTC_MIRROR_VERTICAL, 2);
    EXPECT_EQ(MZTC_MIRROR_CENTRAL, 3);
    
    // Test shutter mode enums
    EXPECT_EQ(MZTC_SHUTTER_TEMP_ONLY, 0);
    EXPECT_EQ(MZTC_SHUTTER_TIME_ONLY, 1);
    EXPECT_EQ(MZTC_SHUTTER_TIME_AND_TEMP, 2);
}

// Test status values
TEST_F(MztcCameraTest, StatusValues) {
    EXPECT_EQ(MZTC_STATUS_OFFLINE, 0x00);
    EXPECT_EQ(MZTC_STATUS_INITIALIZING, 0x01);
    EXPECT_EQ(MZTC_STATUS_READY, 0x02);
    EXPECT_EQ(MZTC_STATUS_CAPTURING, 0x03);
    EXPECT_EQ(MZTC_STATUS_CALIBRATING, 0x04);
    EXPECT_EQ(MZTC_STATUS_ERROR, 0x05);
    EXPECT_EQ(MZTC_STATUS_ALERT, 0x06);
    EXPECT_EQ(MZTC_STATUS_RECORDING, 0x07);
}

// Test error flags
TEST_F(MztcCameraTest, ErrorFlags) {
    EXPECT_EQ(MZTC_ERROR_COMMUNICATION, 0x01);
    EXPECT_EQ(MZTC_ERROR_CALIBRATION, 0x02);
    EXPECT_EQ(MZTC_ERROR_TEMPERATURE, 0x04);
    EXPECT_EQ(MZTC_ERROR_MEMORY, 0x08);
    EXPECT_EQ(MZTC_ERROR_TIMEOUT, 0x10);
    EXPECT_EQ(MZTC_ERROR_INVALID_CONFIG, 0x20);
}

// Test limits
TEST_F(MztcCameraTest, Limits) {
    EXPECT_EQ(MZTC_MAX_UPDATE_RATE, 30);
    EXPECT_EQ(MZTC_MIN_UPDATE_RATE, 1);
    EXPECT_EQ(MZTC_MAX_FFC_INTERVAL, 60);
    EXPECT_EQ(MZTC_MIN_FFC_INTERVAL, 1);
}

// Test structure sizes
TEST_F(MztcCameraTest, StructureSizes) {
    // Test that structures have reasonable sizes
    EXPECT_GT(sizeof(mztcConfig_t), 0);
    EXPECT_GT(sizeof(mztcStatus_t), 0);
    EXPECT_GT(sizeof(mztcFrameData_t), 0);
    
    // Test specific field sizes
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->enabled), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->port), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->baudrate), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->mode), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->update_rate), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->temperature_unit), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->palette_mode), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->auto_shutter), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->digital_enhancement), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->spatial_denoise), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->temporal_denoise), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->brightness), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->contrast), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->zoom_level), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->mirror_mode), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->crosshair_enabled), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->temperature_alerts), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->alert_high_temp), 4); // float
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->alert_low_temp), 4);  // float
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->ffc_interval), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->bad_pixel_removal), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->vignetting_correction), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->zoom_channel), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->palette_channel), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->ffc_channel), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->brightness_channel), 1);
    EXPECT_EQ(sizeof(((mztcConfig_t*)0)->contrast_channel), 1);
}

// Test status structure fields
TEST_F(MztcCameraTest, StatusStructureFields) {
    EXPECT_EQ(sizeof(((mztcStatus_t*)0)->status), 1);
    EXPECT_EQ(sizeof(((mztcStatus_t*)0)->mode), 1);
    EXPECT_EQ(sizeof(((mztcStatus_t*)0)->connected), 1); // bool
    EXPECT_EQ(sizeof(((mztcStatus_t*)0)->connection_quality), 1);
    EXPECT_EQ(sizeof(((mztcStatus_t*)0)->last_calibration), 1);
    EXPECT_EQ(sizeof(((mztcStatus_t*)0)->camera_temperature), 4); // float
    EXPECT_EQ(sizeof(((mztcStatus_t*)0)->ambient_temperature), 4); // float
    EXPECT_EQ(sizeof(((mztcStatus_t*)0)->frame_count), 4); // uint32_t
    EXPECT_EQ(sizeof(((mztcStatus_t*)0)->error_flags), 1);
    EXPECT_EQ(sizeof(((mztcStatus_t*)0)->last_frame_time), 4); // uint32_t
}

// Test frame data structure fields
TEST_F(MztcCameraTest, FrameDataStructureFields) {
    EXPECT_EQ(sizeof(((mztcFrameData_t*)0)->width), 2); // uint16_t
    EXPECT_EQ(sizeof(((mztcFrameData_t*)0)->height), 2); // uint16_t
    EXPECT_EQ(sizeof(((mztcFrameData_t*)0)->min_temp), 4); // float
    EXPECT_EQ(sizeof(((mztcFrameData_t*)0)->max_temp), 4); // float
    EXPECT_EQ(sizeof(((mztcFrameData_t*)0)->center_temp), 4); // float
    EXPECT_EQ(sizeof(((mztcFrameData_t*)0)->hottest_temp), 4); // float
    EXPECT_EQ(sizeof(((mztcFrameData_t*)0)->coldest_temp), 4); // float
    EXPECT_EQ(sizeof(((mztcFrameData_t*)0)->hottest_x), 2); // uint16_t
    EXPECT_EQ(sizeof(((mztcFrameData_t*)0)->hottest_y), 2); // uint16_t
    EXPECT_EQ(sizeof(((mztcFrameData_t*)0)->coldest_x), 2); // uint16_t
    EXPECT_EQ(sizeof(((mztcFrameData_t*)0)->coldest_y), 2); // uint16_t
    EXPECT_EQ(sizeof(((mztcFrameData_t*)0)->data), 256); // uint8_t array
}

// Test configuration validation
TEST_F(MztcCameraTest, ConfigurationValidation) {
    // Test that configuration ranges make sense
    mztcConfig_t testConfig = {0};
    
    // Test boolean fields
    testConfig.enabled = 1;
    EXPECT_EQ(testConfig.enabled, 1);
    testConfig.enabled = 0;
    EXPECT_EQ(testConfig.enabled, 0);
    
    // Test port field (should be reasonable range)
    testConfig.port = 1;
    EXPECT_EQ(testConfig.port, 1);
    testConfig.port = 8;
    EXPECT_EQ(testConfig.port, 8);
    
    // Test baudrate field
    testConfig.baudrate = 0;
    EXPECT_EQ(testConfig.baudrate, 0);
    testConfig.baudrate = 3;
    EXPECT_EQ(testConfig.baudrate, 3);
    
    // Test mode field
    testConfig.mode = MZTC_MODE_DISABLED;
    EXPECT_EQ(testConfig.mode, MZTC_MODE_DISABLED);
    testConfig.mode = MZTC_MODE_SURVEILLANCE;
    EXPECT_EQ(testConfig.mode, MZTC_MODE_SURVEILLANCE);
    
    // Test update rate field
    testConfig.update_rate = MZTC_MIN_UPDATE_RATE;
    EXPECT_EQ(testConfig.update_rate, MZTC_MIN_UPDATE_RATE);
    testConfig.update_rate = MZTC_MAX_UPDATE_RATE;
    EXPECT_EQ(testConfig.update_rate, MZTC_MAX_UPDATE_RATE);
    
    // Test temperature unit field
    testConfig.temperature_unit = MZTC_UNIT_CELSIUS;
    EXPECT_EQ(testConfig.temperature_unit, MZTC_UNIT_CELSIUS);
    testConfig.temperature_unit = MZTC_UNIT_KELVIN;
    EXPECT_EQ(testConfig.temperature_unit, MZTC_UNIT_KELVIN);
    
    // Test palette mode field
    testConfig.palette_mode = MZTC_PALETTE_WHITE_HOT;
    EXPECT_EQ(testConfig.palette_mode, MZTC_PALETTE_WHITE_HOT);
    testConfig.palette_mode = MZTC_PALETTE_RED_HOT;
    EXPECT_EQ(testConfig.palette_mode, MZTC_PALETTE_RED_HOT);
    
    // Test auto shutter field
    testConfig.auto_shutter = MZTC_SHUTTER_TEMP_ONLY;
    EXPECT_EQ(testConfig.auto_shutter, MZTC_SHUTTER_TEMP_ONLY);
    testConfig.auto_shutter = MZTC_SHUTTER_TIME_AND_TEMP;
    EXPECT_EQ(testConfig.auto_shutter, MZTC_SHUTTER_TIME_AND_TEMP);
    
    // Test percentage fields (0-100)
    testConfig.digital_enhancement = 0;
    EXPECT_EQ(testConfig.digital_enhancement, 0);
    testConfig.digital_enhancement = 50;
    EXPECT_EQ(testConfig.digital_enhancement, 50);
    testConfig.digital_enhancement = 100;
    EXPECT_EQ(testConfig.digital_enhancement, 100);
    
    testConfig.spatial_denoise = 0;
    EXPECT_EQ(testConfig.spatial_denoise, 0);
    testConfig.spatial_denoise = 75;
    EXPECT_EQ(testConfig.spatial_denoise, 75);
    testConfig.spatial_denoise = 100;
    EXPECT_EQ(testConfig.spatial_denoise, 100);
    
    testConfig.temporal_denoise = 0;
    EXPECT_EQ(testConfig.temporal_denoise, 0);
    testConfig.temporal_denoise = 25;
    EXPECT_EQ(testConfig.temporal_denoise, 25);
    testConfig.temporal_denoise = 100;
    EXPECT_EQ(testConfig.temporal_denoise, 100);
    
    testConfig.brightness = 0;
    EXPECT_EQ(testConfig.brightness, 0);
    testConfig.brightness = 50;
    EXPECT_EQ(testConfig.brightness, 50);
    testConfig.brightness = 100;
    EXPECT_EQ(testConfig.brightness, 100);
    
    testConfig.contrast = 0;
    EXPECT_EQ(testConfig.contrast, 0);
    testConfig.contrast = 60;
    EXPECT_EQ(testConfig.contrast, 60);
    testConfig.contrast = 100;
    EXPECT_EQ(testConfig.contrast, 100);
    
    // Test zoom level field
    testConfig.zoom_level = MZTC_ZOOM_1X;
    EXPECT_EQ(testConfig.zoom_level, MZTC_ZOOM_1X);
    testConfig.zoom_level = MZTC_ZOOM_4X;
    EXPECT_EQ(testConfig.zoom_level, MZTC_ZOOM_4X);
    testConfig.zoom_level = MZTC_ZOOM_8X;
    EXPECT_EQ(testConfig.zoom_level, MZTC_ZOOM_8X);
    
    // Test mirror mode field
    testConfig.mirror_mode = MZTC_MIRROR_NONE;
    EXPECT_EQ(testConfig.mirror_mode, MZTC_MIRROR_NONE);
    testConfig.mirror_mode = MZTC_MIRROR_HORIZONTAL;
    EXPECT_EQ(testConfig.mirror_mode, MZTC_MIRROR_HORIZONTAL);
    testConfig.mirror_mode = MZTC_MIRROR_VERTICAL;
    EXPECT_EQ(testConfig.mirror_mode, MZTC_MIRROR_VERTICAL);
    testConfig.mirror_mode = MZTC_MIRROR_CENTRAL;
    EXPECT_EQ(testConfig.mirror_mode, MZTC_MIRROR_CENTRAL);
    
    // Test boolean fields
    testConfig.crosshair_enabled = 1;
    EXPECT_EQ(testConfig.crosshair_enabled, 1);
    testConfig.crosshair_enabled = 0;
    EXPECT_EQ(testConfig.crosshair_enabled, 0);
    
    testConfig.temperature_alerts = 1;
    EXPECT_EQ(testConfig.temperature_alerts, 1);
    testConfig.temperature_alerts = 0;
    EXPECT_EQ(testConfig.temperature_alerts, 0);
    
    // Test float fields
    testConfig.alert_high_temp = 50.0f;
    EXPECT_EQ(testConfig.alert_high_temp, 50.0f);
    testConfig.alert_high_temp = -20.0f;
    EXPECT_EQ(testConfig.alert_high_temp, -20.0f);
    
    testConfig.alert_low_temp = -10.0f;
    EXPECT_EQ(testConfig.alert_low_temp, -10.0f);
    testConfig.alert_low_temp = 100.0f;
    EXPECT_EQ(testConfig.alert_low_temp, 100.0f);
    
    // Test interval fields
    testConfig.ffc_interval = MZTC_MIN_FFC_INTERVAL;
    EXPECT_EQ(testConfig.ffc_interval, MZTC_MIN_FFC_INTERVAL);
    testConfig.ffc_interval = MZTC_MAX_FFC_INTERVAL;
    EXPECT_EQ(testConfig.ffc_interval, MZTC_MAX_FFC_INTERVAL);
    
    // Test boolean fields
    testConfig.bad_pixel_removal = 1;
    EXPECT_EQ(testConfig.bad_pixel_removal, 1);
    testConfig.bad_pixel_removal = 0;
    EXPECT_EQ(testConfig.bad_pixel_removal, 0);
    
    testConfig.vignetting_correction = 1;
    EXPECT_EQ(testConfig.vignetting_correction, 1);
    testConfig.vignetting_correction = 0;
    EXPECT_EQ(testConfig.vignetting_correction, 0);
    
    // Test channel fields (0 = disabled, 1-18 = AUX1-18)
    testConfig.zoom_channel = 0;
    EXPECT_EQ(testConfig.zoom_channel, 0);
    testConfig.zoom_channel = 1;
    EXPECT_EQ(testConfig.zoom_channel, 1);
    testConfig.zoom_channel = 18;
    EXPECT_EQ(testConfig.zoom_channel, 18);
    
    testConfig.palette_channel = 0;
    EXPECT_EQ(testConfig.palette_channel, 0);
    testConfig.palette_channel = 5;
    EXPECT_EQ(testConfig.palette_channel, 5);
    testConfig.palette_channel = 18;
    EXPECT_EQ(testConfig.palette_channel, 18);
    
    testConfig.ffc_channel = 0;
    EXPECT_EQ(testConfig.ffc_channel, 0);
    testConfig.ffc_channel = 10;
    EXPECT_EQ(testConfig.ffc_channel, 10);
    testConfig.ffc_channel = 18;
    EXPECT_EQ(testConfig.ffc_channel, 18);
    
    testConfig.brightness_channel = 0;
    EXPECT_EQ(testConfig.brightness_channel, 0);
    testConfig.brightness_channel = 15;
    EXPECT_EQ(testConfig.brightness_channel, 15);
    testConfig.brightness_channel = 18;
    EXPECT_EQ(testConfig.brightness_channel, 18);
    
    testConfig.contrast_channel = 0;
    EXPECT_EQ(testConfig.contrast_channel, 0);
    testConfig.contrast_channel = 12;
    EXPECT_EQ(testConfig.contrast_channel, 12);
    testConfig.contrast_channel = 18;
    EXPECT_EQ(testConfig.contrast_channel, 18);
}

// Test status structure validation
TEST_F(MztcCameraTest, StatusStructureValidation) {
    // Test that status structure can be initialized
    mztcStatus_t testStatus = {0};
    
    // Test status field
    testStatus.status = MZTC_STATUS_OFFLINE;
    EXPECT_EQ(testStatus.status, MZTC_STATUS_OFFLINE);
    testStatus.status = MZTC_STATUS_READY;
    EXPECT_EQ(testStatus.status, MZTC_STATUS_READY);
    testStatus.status = MZTC_STATUS_ERROR;
    EXPECT_EQ(testStatus.status, MZTC_STATUS_ERROR);
    
    // Test mode field
    testStatus.mode = MZTC_MODE_DISABLED;
    EXPECT_EQ(testStatus.mode, MZTC_MODE_DISABLED);
    testStatus.mode = MZTC_MODE_CONTINUOUS;
    EXPECT_EQ(testStatus.mode, MZTC_MODE_CONTINUOUS);
    testStatus.mode = MZTC_MODE_SURVEILLANCE;
    EXPECT_EQ(testStatus.mode, MZTC_MODE_SURVEILLANCE);
    
    // Test boolean field
    testStatus.connected = false;
    EXPECT_EQ(testStatus.connected, false);
    testStatus.connected = true;
    EXPECT_EQ(testStatus.connected, true);
    
    // Test connection quality field
    testStatus.connection_quality = 0;
    EXPECT_EQ(testStatus.connection_quality, 0);
    testStatus.connection_quality = 100;
    EXPECT_EQ(testStatus.connection_quality, 100);
    
    // Test last calibration field
    testStatus.last_calibration = 0;
    EXPECT_EQ(testStatus.last_calibration, 0);
    testStatus.last_calibration = 60;
    EXPECT_EQ(testStatus.last_calibration, 60);
    
    // Test temperature fields
    testStatus.camera_temperature = 25.0f;
    EXPECT_EQ(testStatus.camera_temperature, 25.0f);
    testStatus.camera_temperature = -10.0f;
    EXPECT_EQ(testStatus.camera_temperature, -10.0f);
    
    testStatus.ambient_temperature = 20.0f;
    EXPECT_EQ(testStatus.ambient_temperature, 20.0f);
    testStatus.ambient_temperature = 35.0f;
    EXPECT_EQ(testStatus.ambient_temperature, 35.0f);
    
    // Test frame count field
    testStatus.frame_count = 0;
    EXPECT_EQ(testStatus.frame_count, 0);
    testStatus.frame_count = 1000;
    EXPECT_EQ(testStatus.frame_count, 1000);
    testStatus.frame_count = 0xFFFFFFFF;
    EXPECT_EQ(testStatus.frame_count, 0xFFFFFFFF);
    
    // Test error flags field
    testStatus.error_flags = 0;
    EXPECT_EQ(testStatus.error_flags, 0);
    testStatus.error_flags = MZTC_ERROR_COMMUNICATION;
    EXPECT_EQ(testStatus.error_flags, MZTC_ERROR_COMMUNICATION);
    testStatus.error_flags = MZTC_ERROR_COMMUNICATION | MZTC_ERROR_CALIBRATION;
    EXPECT_EQ(testStatus.error_flags, MZTC_ERROR_COMMUNICATION | MZTC_ERROR_CALIBRATION);
    
    // Test last frame time field
    testStatus.last_frame_time = 0;
    EXPECT_EQ(testStatus.last_frame_time, 0);
    testStatus.last_frame_time = 1000000;
    EXPECT_EQ(testStatus.last_frame_time, 1000000);
    testStatus.last_frame_time = 0xFFFFFFFF;
    EXPECT_EQ(testStatus.last_frame_time, 0xFFFFFFFF);
}

// Test frame data structure validation
TEST_F(MztcCameraTest, FrameDataStructureValidation) {
    // Test that frame data structure can be initialized
    mztcFrameData_t testFrame = {0};
    
    // Test dimension fields
    testFrame.width = 160;
    EXPECT_EQ(testFrame.width, 160);
    testFrame.width = 320;
    EXPECT_EQ(testFrame.width, 320);
    
    testFrame.height = 120;
    EXPECT_EQ(testFrame.height, 120);
    testFrame.height = 240;
    EXPECT_EQ(testFrame.height, 240);
    
    // Test temperature fields
    testFrame.min_temp = -20.0f;
    EXPECT_EQ(testFrame.min_temp, -20.0f);
    testFrame.min_temp = 100.0f;
    EXPECT_EQ(testFrame.min_temp, 100.0f);
    
    testFrame.max_temp = 50.0f;
    EXPECT_EQ(testFrame.max_temp, 50.0f);
    testFrame.max_temp = 200.0f;
    EXPECT_EQ(testFrame.max_temp, 200.0f);
    
    testFrame.center_temp = 25.0f;
    EXPECT_EQ(testFrame.center_temp, 25.0f);
    testFrame.center_temp = 75.0f;
    EXPECT_EQ(testFrame.center_temp, 75.0f);
    
    testFrame.hottest_temp = 80.0f;
    EXPECT_EQ(testFrame.hottest_temp, 80.0f);
    testFrame.hottest_temp = 150.0f;
    EXPECT_EQ(testFrame.hottest_temp, 150.0f);
    
    testFrame.coldest_temp = -30.0f;
    EXPECT_EQ(testFrame.coldest_temp, -30.0f);
    testFrame.coldest_temp = 10.0f;
    EXPECT_EQ(testFrame.coldest_temp, 10.0f);
    
    // Test coordinate fields
    testFrame.hottest_x = 0;
    EXPECT_EQ(testFrame.hottest_x, 0);
    testFrame.hottest_x = 159;
    EXPECT_EQ(testFrame.hottest_x, 159);
    
    testFrame.hottest_y = 0;
    EXPECT_EQ(testFrame.hottest_y, 0);
    testFrame.hottest_y = 119;
    EXPECT_EQ(testFrame.hottest_y, 119);
    
    testFrame.coldest_x = 0;
    EXPECT_EQ(testFrame.coldest_x, 0);
    testFrame.coldest_x = 159;
    EXPECT_EQ(testFrame.coldest_x, 159);
    
    testFrame.coldest_y = 0;
    EXPECT_EQ(testFrame.coldest_y, 0);
    testFrame.coldest_y = 119;
    EXPECT_EQ(testFrame.coldest_y, 119);
    
    // Test data array
    for (int i = 0; i < 256; i++) {
        testFrame.data[i] = i & 0xFF;
        EXPECT_EQ(testFrame.data[i], i & 0xFF);
    }
}

// Test enum combinations
TEST_F(MztcCameraTest, EnumCombinations) {
    // Test that enum values can be combined logically
    uint8_t combinedFlags = MZTC_ERROR_COMMUNICATION | MZTC_ERROR_CALIBRATION;
    EXPECT_EQ(combinedFlags, 0x03);
    
    combinedFlags |= MZTC_ERROR_TEMPERATURE;
    EXPECT_EQ(combinedFlags, 0x07);
    
    combinedFlags |= MZTC_ERROR_MEMORY;
    EXPECT_EQ(combinedFlags, 0x0F);
    
    // Test that we can check individual flags
    EXPECT_TRUE(combinedFlags & MZTC_ERROR_COMMUNICATION);
    EXPECT_TRUE(combinedFlags & MZTC_ERROR_CALIBRATION);
    EXPECT_TRUE(combinedFlags & MZTC_ERROR_TEMPERATURE);
    EXPECT_TRUE(combinedFlags & MZTC_ERROR_MEMORY);
    EXPECT_FALSE(combinedFlags & MZTC_ERROR_TIMEOUT);
    EXPECT_FALSE(combinedFlags & MZTC_ERROR_INVALID_CONFIG);
}

// Test structure alignment
TEST_F(MztcCameraTest, StructureAlignment) {
    // Test that structures are properly aligned
    mztcConfig_t config;
    mztcStatus_t status;
    mztcFrameData_t frame;
    
    // Test that we can create arrays
    mztcConfig_t configArray[2];
    mztcStatus_t statusArray[2];
    mztcFrameData_t frameArray[2];
    
    // Test that we can access array elements
    configArray[0] = config;
    configArray[1] = config;
    statusArray[0] = status;
    statusArray[1] = status;
    frameArray[0] = frame;
    frameArray[1] = frame;
    
    EXPECT_TRUE(true); // If we get here, alignment is correct
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    return RUN_ALL_TESTS();
}

#endif // USE_MZTC

/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <platform.h>
#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/sensor.h"

#if !defined(USE_TARGET_HARDWARE_DESCRIPTORS)

/** IMU **/
#if !defined(USE_TARGET_IMU_HARDWARE_DESCRIPTORS)

    #if !defined(MPU_ADDRESS)
    #define MPU_ADDRESS 0x68
    #endif

    #if defined(USE_IMU_MPU6000)
        BUSDEV_REGISTER_SPI(busdev_mpu6000,     DEVHW_MPU6000,      MPU6000_SPI_BUS,    MPU6000_CS_PIN,     NONE,  DEVFLAGS_NONE,  IMU_MPU6000_ALIGN);
    #endif

    #if defined(USE_IMU_MPU6500)
        #if defined(MPU6500_SPI_BUS)
        BUSDEV_REGISTER_SPI(busdev_mpu6500,     DEVHW_MPU6500,      MPU6500_SPI_BUS,    MPU6500_CS_PIN,     NONE,  DEVFLAGS_NONE,  IMU_MPU6500_ALIGN);
        #elif defined(MPU6500_I2C_BUS)
        BUSDEV_REGISTER_I2C(busdev_mpu6500,     DEVHW_MPU6500,      MPU6500_I2C_BUS,    MPU_ADDRESS,        NONE,  DEVFLAGS_NONE,  IMU_MPU6500_ALIGN);
        #endif
    #endif

    #if defined(USE_IMU_MPU9250)
        #if defined(MPU9250_SPI_BUS)
        BUSDEV_REGISTER_SPI(busdev_mpu9250,     DEVHW_MPU9250,      MPU9250_SPI_BUS,    MPU9250_CS_PIN,     NONE,  DEVFLAGS_NONE,  IMU_MPU9250_ALIGN);
        #elif defined(MPU9250_I2C_BUS)
        BUSDEV_REGISTER_I2C(busdev_mpu9250,     DEVHW_MPU9250,      MPU9250_I2C_BUS,    MPU_ADDRESS,        NONE,  DEVFLAGS_NONE,  IMU_MPU9250_ALIGN);
        #endif
    #endif

    #if defined(USE_IMU_ICM20689)
        BUSDEV_REGISTER_SPI(busdev_icm20689,    DEVHW_ICM20689,     ICM20689_SPI_BUS,   ICM20689_CS_PIN,    NONE,  DEVFLAGS_NONE,  IMU_ICM20689_ALIGN);
    #endif

    #if defined(USE_IMU_ICM42605)
        BUSDEV_REGISTER_SPI(busdev_icm42605,    DEVHW_ICM42605,     ICM42605_SPI_BUS,   ICM42605_CS_PIN,    NONE,  DEVFLAGS_NONE,  IMU_ICM42605_ALIGN);
    #endif

    #if defined(USE_IMU_BMI160)
        #if defined(BMI160_SPI_BUS)
        BUSDEV_REGISTER_SPI(busdev_bmi160,      DEVHW_BMI160,       BMI160_SPI_BUS,     BMI160_CS_PIN,      NONE,  DEVFLAGS_NONE,  IMU_BMI160_ALIGN);
        #elif defined(BMI160_I2C_BUS)
        BUSDEV_REGISTER_I2C(busdev_bmi160,      DEVHW_BMI160,       BMI160_I2C_BUS,     0x68,               NONE,  DEVFLAGS_NONE,  IMU_BMI160_ALIGN);
        #endif
    #endif

    #if defined(USE_IMU_BMI088)
        #if defined(BMI088_SPI_BUS)
        BUSDEV_REGISTER_SPI(busdev_bmi088_gyro, DEVHW_BMI088_GYRO,  BMI088_SPI_BUS,     BMI088_GYRO_CS_PIN, NONE,  DEVFLAGS_NONE,  IMU_BMI088_ALIGN);
        BUSDEV_REGISTER_SPI(busdev_bmi088_acc,  DEVHW_BMI088_ACC,   BMI088_SPI_BUS,     BMI088_ACC_CS_PIN,  NONE,  DEVFLAGS_NONE,  IMU_BMI088_ALIGN);
        #elif defined(BMI088_I2C_BUS)
        BUSDEV_REGISTER_I2C(busdev_bmi088,      DEVHW_BMI088,       BMI088_I2C_BUS,     0x68,               NONE,  DEVFLAGS_NONE,  IMU_BMI088_ALIGN);
        #endif
    #endif

    #if defined(USE_IMU_BMI270)
        BUSDEV_REGISTER_SPI(busdev_bmi270,      DEVHW_BMI270,       BMI270_SPI_BUS,     BMI270_CS_PIN,      NONE,  DEVFLAGS_NONE,  IMU_BMI270_ALIGN);
    #endif

    #if defined(USE_IMU_LSM6DXX)
        BUSDEV_REGISTER_SPI(busdev_lsm6dxx,      DEVHW_LSM6D,       LSM6DXX_SPI_BUS,     LSM6DXX_CS_PIN,      NONE,  DEVFLAGS_NONE,  IMU_LSM6DXX_ALIGN);
    #endif

#endif


/** BAROMETERS **/

#if defined(USE_BARO_BMP085)
    #if !defined(BMP085_I2C_BUS)
        #define BMP085_I2C_BUS BARO_I2C_BUS
    #endif
    #if !defined(BMP085_I2C_ADDR)
        #define BMP085_I2C_ADDR (0x77)
    #endif
    BUSDEV_REGISTER_I2C(busdev_bmp085,      DEVHW_BMP085,       BMP085_I2C_BUS,     BMP085_I2C_ADDR,    NONE,           DEVFLAGS_NONE,      0);
#endif

#if defined(USE_BARO_BMP280)
    #if defined(BMP280_SPI_BUS)
    BUSDEV_REGISTER_SPI(busdev_bmp280,      DEVHW_BMP280,       BMP280_SPI_BUS,     BMP280_CS_PIN,      NONE,           DEVFLAGS_NONE,      0);
    #elif defined(BMP280_I2C_BUS) || defined(BARO_I2C_BUS)
    #if !defined(BMP280_I2C_BUS)
        #define BMP280_I2C_BUS BARO_I2C_BUS
    #endif
    #if !defined(BMP280_I2C_ADDR)
        #define BMP280_I2C_ADDR (0x76)
    #endif
    BUSDEV_REGISTER_I2C(busdev_bmp280,      DEVHW_BMP280,       BMP280_I2C_BUS,     BMP280_I2C_ADDR,	NONE,           DEVFLAGS_NONE,      0);
    #endif
#endif

#if defined(USE_BARO_BMP388)
    #if defined(BMP388_SPI_BUS)
    BUSDEV_REGISTER_SPI(busdev_bmp388,      DEVHW_BMP388,       BMP388_SPI_BUS,     BMP388_CS_PIN,      NONE,           DEVFLAGS_NONE,      0);
    #elif defined(BMP388_I2C_BUS) || defined(BARO_I2C_BUS)
    #if !defined(BMP388_I2C_BUS)
        #define BMP388_I2C_BUS BARO_I2C_BUS
    #endif
    #if !defined(BMP388_I2C_ADDR)
        #define BMP388_I2C_ADDR (0x76)
    #endif
    BUSDEV_REGISTER_I2C(busdev_bmp388,      DEVHW_BMP388,       BMP388_I2C_BUS,     BMP388_I2C_ADDR,    NONE,           DEVFLAGS_NONE,      0);
    #endif
#endif

#if defined(USE_BARO_SPL06)
    #if defined(SPL06_SPI_BUS)
      BUSDEV_REGISTER_SPI(busdev_spl06,     DEVHW_SPL06,        SPL06_SPI_BUS,      SPL06_CS_PIN,       NONE,           DEVFLAGS_NONE,      0);
    #elif defined(SPL06_I2C_BUS) || defined(BARO_I2C_BUS)
      #if !defined(SPL06_I2C_BUS)
        #define SPL06_I2C_BUS BARO_I2C_BUS
      #endif
      #if !defined(SPL06_I2C_ADDR)
        #define SPL06_I2C_ADDR (0x76)
      #endif
      BUSDEV_REGISTER_I2C(busdev_spl06,     DEVHW_SPL06,        SPL06_I2C_BUS,      SPL06_I2C_ADDR,     NONE,           DEVFLAGS_NONE,      0);
    #endif
#endif

#if defined(USE_BARO_LPS25H)
    #if defined(LPS25H_SPI_BUS)
    BUSDEV_REGISTER_SPI(busdev_lps25h,      DEVHW_LPS25H,       LPS25H_SPI_BUS,     LPS25H_CS_PIN,      NONE,           DEVFLAGS_NONE,      0);
    #endif
#endif

#if defined(USE_BARO_MS5607)
    #if !defined(MS5607_I2C_BUS)
        #define MS5607_I2C_BUS BARO_I2C_BUS
    #endif
    #if !defined(MS5607_I2C_ADDR)
        #define MS5607_I2C_ADDR (0x77)
    #endif
    BUSDEV_REGISTER_I2C(busdev_ms5607,      DEVHW_MS5607,       MS5607_I2C_BUS,     MS5607_I2C_ADDR,    NONE,           DEVFLAGS_USE_RAW_REGISTERS, 0);
#endif

#if defined(USE_BARO_MS5611)
    #if defined(MS5611_SPI_BUS)
    BUSDEV_REGISTER_SPI(busdev_ms5611,      DEVHW_MS5611,       MS5611_SPI_BUS,     MS5611_CS_PIN,      NONE,           DEVFLAGS_USE_RAW_REGISTERS, 0);
    #elif defined(MS5611_I2C_BUS) || defined(BARO_I2C_BUS)
    #if !defined(MS5611_I2C_BUS)
        #define MS5611_I2C_BUS BARO_I2C_BUS
    #endif
    #if !defined(MS5611_I2C_ADDR)
        #define MS5611_I2C_ADDR (0x77)
    #endif
    BUSDEV_REGISTER_I2C(busdev_ms5611,      DEVHW_MS5611,       MS5611_I2C_BUS,     MS5611_I2C_ADDR,    NONE,           DEVFLAGS_USE_RAW_REGISTERS, 0);
    #endif
#endif

#if defined(USE_BARO_DPS310)
    #if defined(DPS310_SPI_BUS)
    BUSDEV_REGISTER_SPI(busdev_dps310,      DEVHW_DPS310,       DPS310_SPI_BUS,     DPS310_CS_PIN,      NONE,           DEVFLAGS_NONE, 0);
    #elif defined(DPS310_I2C_BUS) || defined(BARO_I2C_BUS)
    #if !defined(DPS310_I2C_BUS)
        #define DPS310_I2C_BUS BARO_I2C_BUS
    #endif
    #if !defined(DPS310_I2C_ADDR)
        #define DPS310_I2C_ADDR (0x76)
    #endif
    BUSDEV_REGISTER_I2C(busdev_dps310,      DEVHW_DPS310,       DPS310_I2C_BUS,     DPS310_I2C_ADDR,    NONE,           DEVFLAGS_NONE, 0);
    #endif
#endif

#if defined(USE_BARO_B2SMPB)
    #if defined(B2SMPB_SPI_BUS)
    BUSDEV_REGISTER_SPI(busdev_b2smpb,     DEVHW_B2SMPB,        B2SMPB_SPI_BUS,     B2SMPB_CS_PIN,      NONE,           DEVFLAGS_NONE, 0);
    #elif defined(B2SMPB_I2C_BUS) || defined(BARO_I2C_BUS)
    #if !defined(B2SMPB_I2C_BUS)
        #define B2SMPB_I2C_BUS BARO_I2C_BUS
    #endif
    #if !defined(B2SMPB_I2C_ADDR)
        #define B2SMPB_I2C_ADDR (0x70)
    #endif
    BUSDEV_REGISTER_I2C(busdev_b2smpb,     DEVHW_B2SMPB,        B2SMPB_I2C_BUS,      B2SMPB_I2C_ADDR,   NONE,           DEVFLAGS_NONE, 0);
    #endif
#endif

/** COMPASS SENSORS **/
#if !defined(USE_TARGET_MAG_HARDWARE_DESCRIPTORS)
#if defined(USE_MAG_HMC5883)
    #if !defined(HMC5883_I2C_BUS)
        #define HMC5883_I2C_BUS MAG_I2C_BUS
    #endif
    BUSDEV_REGISTER_I2C(busdev_hmc5883,     DEVHW_HMC5883,      HMC5883_I2C_BUS,    0x1E,               NONE,           DEVFLAGS_NONE,  0);
#endif

#if defined(USE_MAG_QMC5883)
    #if !defined(QMC5883_I2C_BUS)
        #define QMC5883_I2C_BUS MAG_I2C_BUS
    #endif
    BUSDEV_REGISTER_I2C(busdev_qmc5883,     DEVHW_QMC5883,      QMC5883_I2C_BUS,    0x0D,               NONE,           DEVFLAGS_NONE,  0);
#endif

#if defined(USE_MAG_AK8963)
    #if defined(AK8963_SPI_BUS)
    BUSDEV_REGISTER_SPI(busdev_ak8963,      DEVHW_AK8963,       AK8963_SPI_BUS,     AK8963_CS_PIN,      NONE,           DEVFLAGS_NONE,  0);
    #elif defined(AK8963_I2C_BUS) || defined(MAG_I2C_BUS)
    #if !defined(AK8963_I2C_BUS)
        #define AK8963_I2C_BUS MAG_I2C_BUS
    #endif
    BUSDEV_REGISTER_I2C(busdev_ak8963,      DEVHW_AK8963,       AK8963_I2C_BUS,     0x0C,               NONE,           DEVFLAGS_NONE,  0);
    #endif
#endif

#if defined(USE_MAG_AK8975)
    #if defined(AK8975_SPI_BUS)
    BUSDEV_REGISTER_SPI(busdev_ak8975,      DEVHW_AK8975,       AK8975_SPI_BUS,     AK8975_CS_PIN,      NONE,           DEVFLAGS_NONE,  0);
    #elif defined(AK8975_I2C_BUS) || defined(MAG_I2C_BUS)
    #if !defined(AK8975_I2C_BUS)
        #define AK8975_I2C_BUS MAG_I2C_BUS
    #endif
    BUSDEV_REGISTER_I2C(busdev_ak8975,      DEVHW_AK8975,       AK8975_I2C_BUS,     0x0C,               NONE,           DEVFLAGS_NONE,  0);
    #endif
#endif

#if defined(USE_MAG_MAG3110)
    #if !defined(MAG3110_I2C_BUS)
        #define MAG3110_I2C_BUS MAG_I2C_BUS
    #endif
    BUSDEV_REGISTER_I2C(busdev_mag3110,     DEVHW_MAG3110,      MAG3110_I2C_BUS,    0x0E,               NONE,           DEVFLAGS_NONE,  0);
#endif

#if defined(USE_MAG_LIS3MDL)
    #if !defined(LIS3MDL_I2C_BUS)
        #define LIS3MDL_I2C_BUS MAG_I2C_BUS
    #endif
    BUSDEV_REGISTER_I2C(busdev_lis3mdl,     DEVHW_LIS3MDL,      LIS3MDL_I2C_BUS,    0x1E,               NONE,           DEVFLAGS_NONE,  0);
//  ST LIS3MDL address can be changed by connecting it's SDO/SA1 pin to either supply or ground.
//  BUSDEV_REGISTER_I2C(busdev_lis3mdl,     DEVHW_LIS3MDL,      LIS3MDL_I2C_BUS,    0x1C,               NONE,           DEVFLAGS_NONE);
#endif

#if defined(USE_MAG_IST8310)
    #if !defined(IST8310_I2C_BUS)
        #define IST8310_I2C_BUS MAG_I2C_BUS
    #endif
    BUSDEV_REGISTER_I2C(busdev_ist8310_0,   DEVHW_IST8310_0,    IST8310_I2C_BUS,    0x0C,               NONE,           DEVFLAGS_NONE,  0);
    BUSDEV_REGISTER_I2C(busdev_ist8310_1,   DEVHW_IST8310_1,    IST8310_I2C_BUS,    0x0E,               NONE,           DEVFLAGS_NONE,  0);
#endif

#if defined(USE_MAG_IST8308)
    #if !defined(IST8308_I2C_BUS)
        #define IST8308_I2C_BUS MAG_I2C_BUS
    #endif
    BUSDEV_REGISTER_I2C(busdev_ist8308,     DEVHW_IST8308,      IST8308_I2C_BUS,    0x0C,               NONE,           DEVFLAGS_NONE,  0);
#endif

#if defined(USE_MAG_RM3100)
    #if defined(RM3100_SPI_BUS)
    BUSDEV_REGISTER_SPI(busdev_rm3100,      DEVHW_RM3100,       RM3100_SPI_BUS,     RM3100_CS_PIN,      NONE,           DEVFLAGS_NONE,  0);
    #endif
#endif

#if defined(USE_MAG_VCM5883)
    #if !defined(VCM5883_I2C_BUS)
        #define VCM5883_I2C_BUS MAG_I2C_BUS
    #endif
    BUSDEV_REGISTER_I2C(busdev_vcm5883,     DEVHW_VCM5883,      VCM5883_I2C_BUS,    0x0C,               NONE,           DEVFLAGS_NONE,  0);
#endif

#if defined(USE_MAG_MLX90393)
    #if !defined(MLX90393_I2C_BUS)
        #define MLX90393_I2C_BUS MAG_I2C_BUS
    #endif
    BUSDEV_REGISTER_I2C(busdev_mlx90393,     DEVHW_MLX90393,      MLX90393_I2C_BUS,    0x0C,               NONE,           DEVFLAGS_NONE,  0);
#endif

#endif


/** 1-Wire IF **/

#ifdef USE_1WIRE

#if defined(TEMPERATURE_I2C_BUS) && !defined(DS2482_I2C_BUS)
    #define DS2482_I2C_BUS TEMPERATURE_I2C_BUS
#endif

#if defined(USE_1WIRE_DS2482) && defined(DS2482_I2C_BUS)
    BUSDEV_REGISTER_I2C(busdev_ds2482,      DEVHW_DS2482,       DS2482_I2C_BUS,     0x18,               NONE,           DEVFLAGS_USE_RAW_REGISTERS,  0);
#endif

#endif


/** TEMP SENSORS **/

#if defined(TEMPERATURE_I2C_BUS) && !defined(LM75_I2C_BUS)
    #define LM75_I2C_BUS TEMPERATURE_I2C_BUS
#endif

#if defined(USE_TEMPERATURE_LM75) && defined(LM75_I2C_BUS)
    BUSDEV_REGISTER_I2C(busdev_lm75_0,      DEVHW_LM75_0,         LM75_I2C_BUS,     0x48,               NONE,           DEVFLAGS_NONE,  0);
    BUSDEV_REGISTER_I2C(busdev_lm75_1,      DEVHW_LM75_1,         LM75_I2C_BUS,     0x49,               NONE,           DEVFLAGS_NONE,  0);
    BUSDEV_REGISTER_I2C(busdev_lm75_2,      DEVHW_LM75_2,         LM75_I2C_BUS,     0x4A,               NONE,           DEVFLAGS_NONE,  0);
    BUSDEV_REGISTER_I2C(busdev_lm75_3,      DEVHW_LM75_3,         LM75_I2C_BUS,     0x4B,               NONE,           DEVFLAGS_NONE,  0);
    BUSDEV_REGISTER_I2C(busdev_lm75_4,      DEVHW_LM75_4,         LM75_I2C_BUS,     0x4C,               NONE,           DEVFLAGS_NONE,  0);
    BUSDEV_REGISTER_I2C(busdev_lm75_5,      DEVHW_LM75_5,         LM75_I2C_BUS,     0x4D,               NONE,           DEVFLAGS_NONE,  0);
    BUSDEV_REGISTER_I2C(busdev_lm75_6,      DEVHW_LM75_6,         LM75_I2C_BUS,     0x4E,               NONE,           DEVFLAGS_NONE,  0);
    BUSDEV_REGISTER_I2C(busdev_lm75_7,      DEVHW_LM75_7,         LM75_I2C_BUS,     0x4F,               NONE,           DEVFLAGS_NONE,  0);
#endif


/** RANGEFINDER SENSORS **/

#if defined(USE_RANGEFINDER_SRF10)
    #if !defined(SRF10_I2C_BUS)
        #define SRF10_I2C_BUS RANGEFINDER_I2C_BUS
    #endif
    #if defined(SRF10_I2C_BUS)
    BUSDEV_REGISTER_I2C(busdev_srf10,       DEVHW_SRF10,        SRF10_I2C_BUS,      0x70,               NONE,           DEVFLAGS_NONE,  0);
    #endif
#endif

#if defined(USE_RANGEFINDER_VL53L0X)
    #if !defined(VL53L0X_I2C_BUS) && defined(RANGEFINDER_I2C_BUS)
        #define VL53L0X_I2C_BUS RANGEFINDER_I2C_BUS
    #endif

    #if defined(VL53L0X_I2C_BUS)
    BUSDEV_REGISTER_I2C(busdev_vl53l0x,     DEVHW_VL53L0X,      VL53L0X_I2C_BUS,    0x29,               NONE,           DEVFLAGS_NONE,  0);
    #endif
#endif

#if defined(USE_RANGEFINDER_VL53L1X)
    #if !defined(VL53L1X_I2C_BUS) && defined(RANGEFINDER_I2C_BUS)
        #define VL53L1X_I2C_BUS RANGEFINDER_I2C_BUS
    #endif

    #if defined(VL53L1X_I2C_BUS)
    BUSDEV_REGISTER_I2C(busdev_vl53l1x,     DEVHW_VL53L1X,      VL53L1X_I2C_BUS,    0x29,               NONE,           DEVFLAGS_USE_RAW_REGISTERS,  0);
    #endif
#endif

#if defined(USE_RANGEFINDER_US42)
    #if !defined(US42_I2C_BUS) && defined(RANGEFINDER_I2C_BUS)
        #define US42_I2C_BUS RANGEFINDER_I2C_BUS
    #endif
    #if defined(US42_I2C_BUS)
    BUSDEV_REGISTER_I2C(busdev_us42,       DEVHW_US42,           US42_I2C_BUS,       0x70,               NONE,           DEVFLAGS_USE_RAW_REGISTERS, 0); // Requires null data to passthrough
    #endif
#endif

#if defined(USE_RANGEFINDER_TERARANGER_EVO_I2C)
    #if !defined(TERARANGER_EVO_I2C_BUS) && defined(RANGEFINDER_I2C_BUS)
        #define TERARANGER_EVO_I2C_BUS RANGEFINDER_I2C_BUS
    #endif
    #if defined(TERARANGER_EVO_I2C_BUS)
    BUSDEV_REGISTER_I2C(busdev_teraranger_evo,       DEVHW_TERARANGER_EVO_I2C,       TERARANGER_EVO_I2C_BUS,       0x31,       NONE,       DEVFLAGS_NONE,  0);
    #endif
#endif

#if defined(USE_RANGEFINDER_TOF10120_I2C) && (defined(TOF10120_I2C_BUS) || defined(RANGEFINDER_I2C_BUS))
    #if !defined(TOF10120_I2C_BUS)
        #define TOF10120_I2C_BUS RANGEFINDER_I2C_BUS
    #endif
    #if defined(TOF10120_I2C_BUS)
    BUSDEV_REGISTER_I2C(busdev_tof10120,      DEVHW_TOF10120_I2C,   TOF10120_I2C_BUS,     0x52,               NONE,           DEVFLAGS_USE_RAW_REGISTERS,  0);
    #endif
#endif

/** AIRSPEED SENSORS **/

#if defined(PITOT_I2C_BUS) && !defined(MS4525_I2C_BUS)
    #define MS4525_I2C_BUS PITOT_I2C_BUS
#endif

#if defined(USE_PITOT_MS4525) && defined(MS4525_I2C_BUS)
    BUSDEV_REGISTER_I2C(busdev_ms4525,      DEVHW_MS4525,       MS4525_I2C_BUS,     0x28,               NONE,           DEVFLAGS_USE_RAW_REGISTERS,  0);    // Requires 0xFF to passthrough
#endif


#if defined(PITOT_I2C_BUS) && !defined(DLVR_I2C_BUS)
    #define DLVR_I2C_BUS PITOT_I2C_BUS
#endif

#if defined(USE_PITOT_DLVR) && defined(DLVR_I2C_BUS)
    BUSDEV_REGISTER_I2C(busdev_dlvr,        DEVHW_DLVR,         DLVR_I2C_BUS,        0x28,              NONE,           DEVFLAGS_USE_RAW_REGISTERS,  0);    // Requires 0xFF to passthrough
#endif

/** OTHER HARDWARE **/

#if defined(USE_MAX7456)
    BUSDEV_REGISTER_SPI(busdev_max7456,     DEVHW_MAX7456,      MAX7456_SPI_BUS,    MAX7456_CS_PIN,     NONE,           DEVFLAGS_USE_RAW_REGISTERS | DEVFLAGS_SPI_MODE_0,  0);
#endif

#if defined(USE_FLASH_M25P16) && defined(M25P16_SPI_BUS)
    BUSDEV_REGISTER_SPI(busdev_m25p16,      DEVHW_M25P16,       M25P16_SPI_BUS,     M25P16_CS_PIN,      NONE,           DEVFLAGS_NONE,  0);
#endif

#if defined(USE_FLASH_W25N01G)
    BUSDEV_REGISTER_SPI(busdev_w25n01g,     DEVHW_W25N01G,      W25N01G_SPI_BUS,    W25N01G_CS_PIN,     NONE,           DEVFLAGS_NONE,  0);
#endif

#if defined(USE_SDCARD) && defined(USE_SDCARD_SPI)
    BUSDEV_REGISTER_SPI(busdev_sdcard_spi,  DEVHW_SDCARD,       SDCARD_SPI_BUS,     SDCARD_CS_PIN,      NONE,           DEVFLAGS_SPI_MODE_0,  0);
#endif

/*
// FIXME(digitalentity): This is unnecessary at the moment as SDIO is not part of BusDevice infrastructure
#if defined(USE_SDCARD) && defined(USE_SDCARD_SDIO)
    BUSDEV_REGISTER_SDIO(busdev_sdcard_sdio,DEVHW_SDCARD,       SDCARD_SDIO_BUS,    SDCARD_CS_PIN,      NONE,           DEVFLAGS_USE_MANUAL_DEVICE_SELECT);
#endif
*/

#if defined(USE_OLED_UG2864)
    #if !defined(UG2864_I2C_BUS)
        #define UG2864_I2C_BUS BUS_I2C1
    #endif
    BUSDEV_REGISTER_I2C(busdev_ug2864,      DEVHW_UG2864,       UG2864_I2C_BUS,     0x3C,               NONE,           DEVFLAGS_NONE,  0);
#endif

#if defined(USE_IRLOCK) && defined(USE_I2C)
    #if !defined(IRLOCK_I2C_BUS) && defined(EXTERNAL_I2C_BUS)
        #define IRLOCK_I2C_BUS EXTERNAL_I2C_BUS
    #else
        #define IRLOCK_I2C_BUS BUS_I2C1
    #endif
    BUSDEV_REGISTER_I2C(busdev_irlock,      DEVHW_IRLOCK,       IRLOCK_I2C_BUS,     0x54,               NONE,           DEVFLAGS_USE_RAW_REGISTERS);
#endif

#if defined(USE_I2C) && defined(USE_I2C_IO_EXPANDER)

    #if !defined(PCF8574_I2C_BUS) && defined(EXTERNAL_I2C_BUS)
        #define PCF8574_I2C_BUS EXTERNAL_I2C_BUS
    #endif

    #if !defined(PCF8574_I2C_BUS) && defined(DEFAULT_I2C_BUS)
        #define PCF8574_I2C_BUS DEFAULT_I2C_BUS
    #endif

    #if !defined(PCF8574_I2C_BUS)
        #define PCF8574_I2C_BUS BUS_I2C1
    #endif

    BUSDEV_REGISTER_I2C(busdev_pcf8574,      DEVHW_PCF8574,       PCF8574_I2C_BUS,     0x20,               NONE,           DEVFLAGS_NONE, 0);
#endif

#endif  // USE_TARGET_HARDWARE_DESCRIPTORS

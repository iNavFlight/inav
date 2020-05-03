/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#pragma once

// Make sure IMU alignments are migrated to IMU_xxx_ALIGN
#if defined (GYRO_MPU6050_ALIGN) || defined (ACC_MPU6050_ALIGN)
#error "Replace GYRO_MPU6050_ALIGN and ACC_MPU6050_ALIGN with IMU_MPU6050_ALIGN"
#endif

#if defined (GYRO_MPU6000_ALIGN) || defined (ACC_MPU6000_ALIGN)
#error "Replace GYRO_MPU6000_ALIGN and ACC_MPU6000_ALIGN with IMU_MPU6000_ALIGN"
#endif

#if defined (GYRO_MPU6500_ALIGN) || defined (ACC_MPU6500_ALIGN)
#error "Replace GYRO_MPU6500_ALIGN and ACC_MPU6500_ALIGN with IMU_MPU6500_ALIGN"
#endif

#if defined (GYRO_MPU9250_ALIGN) || defined (ACC_MPU9250_ALIGN)
#error "Replace GYRO_MPU9250_ALIGN and ACC_MPU9250_ALIGN with IMU_MPU9250_ALIGN"
#endif

#if defined (GYRO_BMI160_ALIGN) || defined (ACC_BMI160_ALIGN)
#error "Replace GYRO_BMI160_ALIGN and ACC_BMI160_ALIGN with IMU_BMI160_ALIGN"
#endif

#if defined (GYRO_ICM20689_ALIGN) || defined (ACC_ICM20689_ALIGN)
#error "Replace GYRO_ICM20689_ALIGN and ACC_ICM20689_ALIGN with IMU_ICM20689_ALIGN"
#endif

#if defined (GYRO_L3G4200D_ALIGN)
#error "Replace GYRO_L3G4200D_ALIGN with IMU_L3G4200D_ALIGN"
#endif

#if defined (GYRO_MPU3050_ALIGN)
#error "Replace GYRO_MPU3050_ALIGN with IMU_MPU3050_ALIGN"
#endif

#if defined (GYRO_L3GD20_ALIGN)
#error "Replace GYRO_L3GD20_ALIGN with IMU_L3GD20_ALIGN"
#endif

#if defined (ACC_BMA280_ALIGN)
#error "Replace ACC_BMA280_ALIGN with IMU_BMA280_ALIGN"
#endif

#if defined (ACC_MMA8452_ALIGN)
#error "Replace ACC_MMA8452_ALIGN with IMU_MMA8452_ALIGN"
#endif


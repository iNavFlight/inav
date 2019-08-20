/*
 * This file is part of iNav.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define SPL006_I2C_ADDR                         0x76
#define SPL006_DEFAULT_CHIP_ID                  0x10

#define SPL006_PRESSURE_START_REG               0x00
#define SPL006_PRESSURE_LEN                     3       // 24 bits, 3 bytes
#define SPL006_PRESSURE_B2_REG                  0x00    // Pressure MSB Register
#define SPL006_PRESSURE_B1_REG                  0x01    // Pressure middle byte Register
#define SPL006_PRESSURE_B0_REG                  0x02    // Pressure LSB Register
#define SPL006_TEMPERATURE_START_REG            0x03
#define SPL006_TEMPERATURE_LEN                  3       // 24 bits, 3 bytes
#define SPL006_TEMPERATURE_B2_REG               0x03    // Temperature MSB Register
#define SPL006_TEMPERATURE_B1_REG               0x04    // Temperature middle byte Register
#define SPL006_TEMPERATURE_B0_REG               0x05    // Temperature LSB Register
#define SPL006_PRESSURE_CFG_REG                 0x06    // Pressure config
#define SPL006_TEMPERATURE_CFG_REG              0x07    // Temperature config
#define SPL006_MODE_AND_STATUS_REG              0x08    // Mode and status
#define SPL006_INT_AND_FIFO_CFG_REG             0x09    // Interrupt and FIFO config
#define SPL006_INT_STATUS_REG                   0x0A    // Interrupt and FIFO config
#define SPL006_FIFO_STATUS_REG                  0x0B    // Interrupt and FIFO config
#define SPL006_RST_REG                          0x0C    // Softreset Register
#define SPL006_CHIP_ID_REG                      0x0D    // Chip ID Register
#define SPL006_CALIB_COEFFS_START               0x10
#define SPL006_CALIB_COEFFS_END                 0x21

#define SPL006_CALIB_COEFFS_LEN                 (SPL006_CALIB_COEFFS_END - SPL006_CALIB_COEFFS_START + 1)

// TEMPERATURE_CFG_REG
#define SPL006_TEMP_USE_EXT_SENSOR              (1<<7)

// MODE_AND_STATUS_REG
#define SPL006_MEAS_PRESSURE                    (1<<0)  // measure pressure
#define SPL006_MEAS_TEMPERATURE                 (1<<1)  // measure temperature

#define SPL006_MEAS_CFG_CONTINUOUS              (1<<2)
#define SPL006_MEAS_CFG_PRESSURE_RDY            (1<<4)
#define SPL006_MEAS_CFG_TEMPERATURE_RDY         (1<<5)
#define SPL006_MEAS_CFG_SENSOR_RDY              (1<<6)
#define SPL006_MEAS_CFG_COEFFS_RDY              (1<<7)

// INT_AND_FIFO_CFG_REG
#define SPL006_PRESSURE_RESULT_BIT_SHIFT        (1<<2)  // necessary for pressure oversampling > 8
#define SPL006_TEMPERATURE_RESULT_BIT_SHIFT     (1<<3)  // necessary for temperature oversampling > 8

#define SPL006_PRESSURE_OVERSAMPLING            8       // oversampling 8
#define SPL006_TEMPERATURE_OVERSAMPLING         8       // oversampling 8

#define SPL006_MEASUREMENT_TIME(oversampling)   ((2 + lrintf(oversampling * 1.6)) + 1) // ms

bool spl006Detect(baroDev_t *baro);


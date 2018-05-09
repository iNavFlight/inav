/*
 * This file is part of Cleanflight.
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

 ******************************************************************************
 * @brief   contains definitions from the ST MEMS team
 * @author  MEMS Application Team
 * @version V1.0.0
 * @date    11-February-2015
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 * */

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include <platform.h>
#include "build/build_config.h"
#include "build/debug.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/barometer/barometer.h"
#include "drivers/barometer/barometer_bmp280.h"

#if defined(USE_BARO_LPS25H)

#define PRESSURE_OK     (true)
#define PRESSURE_ERROR  (false)

/**
 * @brief Reference pressure (LSB data)
 * \code
 * Read/write
 * Default value: 0x00
 * 7:0 REF7-ODR0: Lower part of the reference pressure that
 *      is sum to the sensor output pressure.
 * \endcode
 */
#define LPS25HB_REF_P_XL_ADDR         0x08

/**
 * @brief Reference pressure (middle part)
 * \code
 * Read/write
 * Default value: 0x00
 * 7:0 REF15-ODR8: Middle part of the reference pressure that
 *      is sum to the sensor output pressure.
 * \endcode
 */
#define LPS25HB_REF_P_L_ADDR          0x09

/**
 * @brief Reference pressure (MSB part)
 * \code
 * Read/write
 * Default value: 0x00
 * 7:0 REF15-ODR8: Higher part of the reference pressure that
 *      is sum to the sensor output pressure.
 * \endcode
 */
#define LPS25HB_REF_P_H_ADDR          0x0A

/**
 * @brief Device identifier register.
 * \code
 * Read
 * Default value: 0xBD
 * 7:0 This read-only register contains the device identifier that,
 for LPS25HB, is set to 0xCA.
 * \endcode
 */
#define LPS25HB_WHO_AM_I_ADDR         0x0F

/**
 * @brief Pressure and temperature resolution mode register.
 * \code
 * Read
 * Default value: 0x05
 * [7:4] Reserved
 * [3:2] AVGP1-0: select the pressure internal average.
 * [1:0] AVGT1-0: select the temperature internal average.
 * \endcode
 */
#define LPS25HB_RES_CONF_ADDR         0x10

/**
 * @brief Pressure sensor control register 1
 * \code
 * Read/write
 * Default value: 0x00
 * 7 PD: power down control. 0 - disable; 1 - enable
 * 6:4 ODR2, ODR1, ODR0: output data rate selection.
 *     ODR2  | ODR1  | ODR0  | Pressure output data-rate(Hz)  | Temperature output data-rate(Hz)
 *   ----------------------------------------------------------------------------------
 *      0    |  0    |  0    |         one shot               |         one shot
 *      0    |  0    |  1    |            1                   |            1
 *      0    |  1    |  0    |            7                   |            7
 *      0    |  1    |  1    |            12.5                |            12.5
 *      1    |  0    |  0    |            25                  |            25
 *      1    |  0    |  1    |         Reserved               |         Reserved
 *      1    |  1    |  0    |         Reserved               |         Reserved
 *      1    |  1    |  1    |         Reserved               |         Reserved
 *
 * 3 DIFF_EN: Interrupt circuit. 0 - disable; 1 - enable
 * 2 BDU: block data update. 0 - disable; 1 - enable
 * 1 DELTA_EN: delta pressure. 0 - disable; 1 - enable
 * 1 RESET_AZ: reset AutoZero. 0 - disable; 1 - enable  ///////ALE REVIEW
 * 0 SIM: SPI Serial Interface Mode selection. 0 - SPI 4-wire; 1 - SPI 3-wire ///////ALE REVIEW
 * \endcode
 */
#define LPS25HB_CTRL_REG1_ADDR        0x20

/**
 * @brief Pressure sensor control register 2
 * \code
 * Read/write
 * Default value: 0x00
 * 7 BOOT:  Reboot memory content. 0: normal mode; 1: reboot memory content
 * 6 FIFO_EN: FIFO. 0: disable; 1:  enable
 * 5 WTM_EN:  FIFO Watermark level use. 0: disable; 1: enable
 * 4:3 Reserved. keep these bits at 0
 * 2 SWRESET: Software reset. 0: normal mode; 1: SW reset.
 * 1 AUTO_ZERO: Autozero enable. 0: normal mode; 1: autozero enable.
 * 0 ONE_SHOT: One shot enable. 0: waiting for start of conversion; 1: start for a new dataset
 * \endcode
 */
#define LPS25HB_CTRL_REG2_ADDR        0x21

/**
 * @brief Pressure sensor control register 3
 * \code
 * Read/write
 * Default value: 0x00
 * 7 INT_H_L: Interrupt. 0:active high; 1: active low.
 * 6 PP_OD: Push-Pull/OpenDrain selection on interrupt pads. 0: Push-pull; 1: open drain.
 * 5 Reserved
 * 4:3 INT2_S2, INT2_S1: INT2 output signal selection control bits. // TO DO
 * 1:0 INT1_S2, INT1_S1: data signal on INT1 pad control bits.
 *    INT1(2)_S2  | INT1(2)_S1  | INT1(2) pin
 *   ------------------------------------------------------
 *        0       |      0      |     Data signal
 *        0       |      1      |     Pressure high (P_high)
 *        1       |      0      |     Pressure low (P_low)
 *        1       |      1      |     P_low OR P_high
 * \endcode
 */
#define LPS25HB_CTRL_REG3_ADDR        0x22

/**
 * @brief Pressure sensor control register 4
 * \code
 * Read/write
 * Default value: 0x00
 * 7 P2_EMPTY: Empty Signal on INT2 pin.
 * 6 P2_WTM: Watermark Signal on INT2 pin.
 * 5 P2_Overrun:Overrun Signal on INT2 pin.
 * 4 P2_DRDY: Data Ready Signal on INT2 pin.
 * 3 P1_EMPTY: Empty Signal on INT1 pin.
 * 2 P1_WTM: Watermark Signal on INT1 pin.
 * 1 P1_Overrunn:Overrun Signal on INT1 pin.
 * 0 P1_DRDY: Data Ready Signal on INT1 pin.
 * \endcode
 */
#define LPS25HB_CTRL_REG4_ADDR        0x23

/**
 * @brief Interrupt configuration Register
 * \code
 * Read/write
 * Default value: 0x00.
 * 7:3 Reserved.
 * 2 LIR: Latch Interrupt request into INT_SOURCE register. 0 - disable; 1 - enable
 * 1 PL_E: Enable interrupt generation on differential pressure low event. 0 - disable; 1 - enable
 * 0 PH_E: Enable interrupt generation on differential pressure high event. 0 - disable; 1 - enable
 * \endcode
 */
#define LPS25HB_INT_CFG_REG_ADDR                  0x24

/**
 * @brief Interrupt source Register
 * \code
 * Read
 * Default value: 0x00.
 * 7:3 0.
 * 2 IA: Interrupt Active.0: no interrupt has been generated; 1: one or more interrupt events have been generated.
 * 1 PL: Differential pressure Low. 0: no interrupt has been generated; 1: Low differential pressure event has occurred.
 * 0 PH: Differential pressure High. 0: no interrupt has been generated; 1: High differential pressure event has occurred.
 * \endcode
 */
#define LPS25HB_INT_SOURCE_REG_ADDR               0x25

/**
 * @brief Threshold pressure (LSB)
 * \code
 * Read
 * Default value: 0x00.
 * 7:0 THS7-THS0: Low part of threshold value for pressure interrupt
 * generation. The complete threshold value is given by THS_P_H & THS_P_L and is
 * expressed as unsigned number. P_ths(mbar)=(THS_P_H & THS_P_L)[dec]/16.
 * \endcode
 */
#define LPS25HB_THS_P_LOW_REG_ADDR                0x30

/**
 * @brief Threshold pressure (MSB)
 * \code
 * Read
 * Default value: 0x00.
 * 7:0 THS15-THS8: High part of threshold value for pressure interrupt
 * generation. The complete threshold value is given by THS_P_H & THS_P_L and is
 * expressed as unsigned number. P_ths(mbar)=(THS_P_H & THS_P_L)[dec]/16.
 * \endcode
 */
#define LPS25HB_THS_P_HIGH_REG_ADDR              0x31

/**
 * @brief  Status Register
 * \code
 * Read
 * Default value: 0x00
 * 7:6 0
 * 5 P_OR: Pressure data overrun. 0: no overrun has occurred; 1: new data for pressure has overwritten the previous one.
 * 4 T_OR: Temperature data overrun. 0: no overrun has occurred; 1: a new data for temperature has overwritten the previous one.
 * 3:2 0
 * 1 P_DA: Pressure data available. 0: new data for pressure is not yet available; 1: new data for pressure is available.
 * 0 T_DA: Temperature data available. 0: new data for temperature is not yet available; 1: new data for temperature is available.
 * \endcode
 */
#define LPS25HB_STATUS_REG_ADDR                 0x27

/**
 * @brief  Pressure data (LSB).
 * \code
 * Read
 * Default value: 0x00.
 * POUT7 - POUT0: Pressure data LSB (2's complement).
 * Pressure output data: Pout(mbar)=(PRESS_OUT_H & PRESS_OUT_L &
 * PRESS_OUT_XL)[dec]/4096.
 * \endcode
 */
#define LPS25HB_PRESS_POUT_XL_ADDR              0x28

/**
 * @brief  Pressure data (Middle part).
 * \code
 * Read
 * Default value: 0x80.
 * POUT15 - POUT8: Pressure data middle part (2's complement).
 * Pressure output data: Pout(mbar)=(PRESS_OUT_H & PRESS_OUT_L &
 * PRESS_OUT_XL)[dec]/4096.
 * \endcode
 */
#define LPS25HB_PRESS_OUT_L_ADDR                0x29

/**
 * @brief  Pressure data (MSB).
 * \code
 * Read
 * Default value: 0x2F.
 * POUT23 - POUT16: Pressure data MSB (2's complement).
 * Pressure output data: Pout(mbar)=(PRESS_OUT_H & PRESS_OUT_L &
 * PRESS_OUT_XL)[dec]/4096.
 * \endcode
 */
#define LPS25HB_PRESS_OUT_H_ADDR                0x2A

/**
 * @brief  Temperature data (LSB).
 * \code
 * Read
 * Default value: 0x00.
 * TOUT7 - TOUT0: temperature data LSB.
 * T(degC) = 42.5 + (Temp_OUTH & TEMP_OUT_L)[dec]/480.
 * \endcode
 */
#define LPS25HB_TEMP_OUT_L_ADDR                 0x2B

/**
 * @brief  Temperature data (MSB).
 * \code
 * Read
 * Default value: 0x00.
 * TOUT15 - TOUT8: temperature data MSB.
 * T(degC) = 42.5 + (Temp_OUTH & TEMP_OUT_L)[dec]/480.
 * \endcode
 */
#define LPS25HB_TEMP_OUT_H_ADDR                 0x2C

/**
 * @brief FIFO control register
 * \code
 * Read/write
 * Default value: 0x00
 * 7:5 F_MODE2, F_MODE1, F_MODE0: FIFO mode selection.
 *     FM2   | FM1   | FM0   |    FIFO MODE
 *   ---------------------------------------------------
 *      0    |  0    |  0    |      BYPASS MODE
 *      0    |  0    |  1    | FIFO MODE. Stops collecting data when full
 *      0    |  1    |  0    | STREAM MODE: Keep the newest measurements in the FIFO
 *      0    |  1    |  1    | STREAM MODE until trigger deasserted, then change to FIFO MODE
 *      1    |  0    |  0    | BYPASS MODE until trigger deasserted, then STREAM MODE
 *      1    |  0    |  1    |       Reserved
 *      1    |  1    |  0    | FIFO_MEAN MODE: Fifo is used to generate a running average filtered pressure
 *      1    |  1    |  1    | BYPASS mode until trigger deasserted, then FIFO MODE
 *
 * 4:0 FIFO Mean Mode Sample size
 *     WTM_POINT4 | WTM_POINT4 | WTM_POINT4 |  WTM_POINT4 | WTM_POINT4 | Sample Size
 *   ----------------------------------------------------------------------------------
 *      0         |    0       |    0       |      0      |     1      |       2
 *      0         |    0       |    0       |      1      |     1      |       4
 *      0         |    0       |    1       |      1      |     1      |       8
 *      0         |    1       |    1       |      1      |     1      |       16
 *      1         |    1       |    1       |      1      |     1      |       32
 * other values operation not guaranteed
 * \endcode
 */
#define LPS25HB_CTRL_FIFO_ADDR                    0x2E

/**
 * @brief FIFO Status register
 * \code
 * Read/write
 * Default value: 0x00
 * 7 WTM_FIFO: Watermark status. 0:FIFO filling is lower than watermark level; 1: FIFO is equal or higher than watermark level.
 * 6 FULL_FIFO: Overrun bit status. 0 - FIFO not full; 1 -FIFO is full.
 * 5 EMPTY_FIFO: Empty FIFO bit. 0 - FIFO not empty; 1 -FIFO is empty.
 * 4:0 DIFF_POINT4...0: FIFOsStored data level.
 * \endcode
 */
#define LPS25HB_STATUS_FIFO_ADDR                    0x2F

/**
 * @brief Pressure offset register
 * \code
 * Read/write
 * Default value: 0x00
 * 7:0 RPDS15...8:Pressure Offset for 1 point calibration after soldering.
 * \endcode
 */
#define LPS25HB_RPDS_TRIM_L_ADDR                    0x39

/**
 * @brief Pressure offset register
 * \code
 * Read/write
 * Default value: 0x00
 * 7:0 RPDS23...16:Pressure Offset for 1 point calibration after soldering.
 * \endcode
 */
#define LPS25HB_RPDS_TRIM_H_ADDR                    0x3A

/******************************************************************************/
/**************************** END REGISTER MAPPING  ***************************/
/******************************************************************************/

/**
 * @brief Multiple Byte. Mask for enabling multiple byte read/write command.
 */
#define LPS25HB_I2C_MULTIPLEBYTE_CMD                      ((uint8_t)0x80)

/**
 * @brief Device Address
 */
#define LPS25HB_ADDRESS_LOW           0xB8
#define LPS25HB_ADDRESS_HIGH          0xBA


/**
 * @brief Device Identifier. Default value of the WHO_AM_I register.
 */
#define I_AM_LPS25HB                 ((uint8_t)0xBD)

/** @defgroup LPS25HB_Power_Mode_Selection_CTRL_REG1 LPS25HB_Power_Mode_Selection_CTRL_REG1
 * @{
 */
#define LPS25HB_MODE_POWERDOWN            ((uint8_t)0x00)
#define LPS25HB_MODE_ACTIVE               ((uint8_t)0x80)

#define LPS25HB_MODE_MASK                 ((uint8_t)0x80)
/**
 * @}
 */

/** @defgroup LPS25HB_Output_Data_Rate_Selection_CTRL_REG1 LPS25HB_Output_Data_Rate_Selection_CTRL_REG1
 * @{
 */
#define LPS25HB_ODR_ONE_SHOT             ((uint8_t)0x00) /*!< Output Data Rate: P - one shot, T - one shot */
#define LPS25HB_ODR_1Hz                  ((uint8_t)0x10) /*!< Output Data Rate: P - 1Hz, T - 1Hz */
#define LPS25HB_ODR_7Hz                  ((uint8_t)0x20) /*!< Output Data Rate: P - 7Hz, T - 7Hz */
#define LPS25HB_ODR_12_5Hz               ((uint8_t)0x30) /*!< Output Data Rate: P - 12.5Hz, T - 12.5Hz */
#define LPS25HB_ODR_25Hz                 ((uint8_t)0x40) /*!< Output Data Rate: P - 25Hz, T - 25Hz */

#define LPS25HB_ODR_MASK                 ((uint8_t)0x70)
/**
 * @}
 */

/** @defgroup LPS25HB_Interrupt_Circuit_Enable_CTRL_REG1 LPS25HB_Interrupt_Circuit_Enable_CTRL_REG1
 * @{
 */
#define LPS25HB_DIFF_DISABLE             ((uint8_t)0x00) /*!< interrupt circuit enabled */
#define LPS25HB_DIFF_ENABLE              ((uint8_t)0x08) /*!< interrupt generation disabled */

#define LPS25HB_DIFF_EN_MASK             ((uint8_t)0x08)
/**
 * @}
 */

/** @defgroup LPS25HB_Block_Data_Update_CTRL_REG1 LPS25HB_Block_Data_Update_CTRL_REG1
 * @{
 */
#define LPS25HB_BDU_CONT              ((uint8_t)0x00) /*!< continuous update */
#define LPS25HB_BDU_READ              ((uint8_t)0x04) /*!< output registers not updated until MSB and LSB reading */

#define LPS25HB_BDU_MASK              ((uint8_t)0x04)
/**
 * @}
 */

/** @defgroup LPS25HB_SPI_Serial_Interface_Mode_Selection_CTRL_REG1 LPS25HB_SPI_Serial_Interface_Mode_Selection_CTRL_REG1
 * @{
 */
#define LPS25HB_SPI_SIM_4W            ((uint8_t)0x00) /*!< 4-wire interface */
#define LPS25HB_SPI_SIM_3W            ((uint8_t)0x01) /*!< 3-wire interface */

#define LPS25HB_SPI_SIM_MASK          ((uint8_t)0x01)
/**
 * @}
 */

/** @defgroup LPS25HB_Refresh_Registers_Flash_Memory_CTRL_REG2 LPS25HB_Refresh_Registers_Flash_Memory_CTRL_REG2
 * @{
 */
#define LPS25HB_NORMAL_MODE           ((uint8_t)0x00)
#define LPS25HB_RESET_MEMORY          ((uint8_t)0x80)

#define LPS25HB_RESET_MEMORY_MASK     ((uint8_t)0x80)
/**
 * @}
 */

/** @defgroup LPS25HB_Pressure_Resolution_Selection_RES_CONF LPS25HB_Pressure_Resolution_Selection_RES_CONF
 * @{
 */
#define LPS25HB_P_RES_AVG_8              ((uint8_t)0x00)
#define LPS25HB_P_RES_AVG_32             ((uint8_t)0x01)
#define LPS25HB_P_RES_AVG_128            ((uint8_t)0x02)
#define LPS25HB_P_RES_AVG_512            ((uint8_t)0x03)

#define LPS25HB_P_RES_MASK               ((uint8_t)0x03)
/**
 * @}
 */

/** @defgroup LPS25HB_Temperature_Resolution_Selection_RES_CONF LPS25HB_Temperature_Resolution_Selection_RES_CONF
 * @{
 */
#define LPS25HB_T_RES_AVG_8              ((uint8_t)0x00)
#define LPS25HB_T_RES_AVG_16             ((uint8_t)0x04)
#define LPS25HB_T_RES_AVG_32             ((uint8_t)0x08)
#define LPS25HB_T_RES_AVG_64             ((uint8_t)0x0C)

#define LPS25HB_T_RES_MASK               ((uint8_t)0x0C)
/**
 * @}
 */


#define LPS25H_CHIP_ID                   (0xBD)
#define LPS25H_ODR                       (25)

#define TEMPERATURE_READY    (0x0)
#define PRESSURE_READY       (0x1)

static bool lps25h_start_ut(baroDev_t * baro)
{
    UNUSED(baro);
    return true;
}

static bool lps25h_get_ut(baroDev_t * baro)
{
    UNUSED(baro);
    return true;
}

/**
 * @brief  Reboot memory content of LPS25HB
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static bool LPS25HB_RebootCmd(baroDev_t * baro)
{
    uint8_t tmpreg;

    /* Read CTRL_REG5 register */
    bool ack = busRead(baro->busDev, LPS25HB_CTRL_REG2_ADDR, &tmpreg);
    if (ack != true)
    {
        return PRESSURE_ERROR;
    }

    /* Enable or Disable the reboot memory */
    tmpreg |= LPS25HB_RESET_MEMORY;

    /* Write value to MEMS CTRL_REG5 regsister */
    ack = busWrite(baro->busDev, LPS25HB_CTRL_REG2_ADDR, tmpreg);
    if (ack != true)
    {
        return PRESSURE_ERROR;
    }

    return PRESSURE_OK;
}

/**
 * @brief  Exit the shutdown mode for LPS25HB
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static bool LPS25HB_Power(baroDev_t * baro, bool enable)
{
    uint8_t tmpreg;

    /* Read the register content */
    bool ack = busRead(baro->busDev, LPS25HB_CTRL_REG1_ADDR, &tmpreg);
    if (ack != PRESSURE_OK)
    {
        return PRESSURE_ERROR;
    }


    tmpreg &= ~LPS25HB_MODE_MASK;

    if (enable)
    {
        /* Set the power down bit */
        tmpreg |= LPS25HB_MODE_ACTIVE;
    }
    else
    {
        /* Clear the power down bit */
         tmpreg |= LPS25HB_MODE_POWERDOWN;
    }

    /* Write register */
    ack = busWrite(baro->busDev, LPS25HB_CTRL_REG1_ADDR, tmpreg);
    if (ack != PRESSURE_OK)
    {
        return PRESSURE_ERROR;
    }

    return PRESSURE_OK;
}

/**
 * @brief  Set LPS25HB Initialization
 * @param  LPS25HB_Init the configuration setting for the LPS25HB
 * @retval PRESSURE_OK in case of success, an error code otherwise
 */
static bool LPS25HB_Init(baroDev_t * baro)
{
    uint8_t tmp1 = 0x00;

    /* Configure the low level interface ---------------------------------------*/
    if (LPS25HB_RebootCmd(baro) != PRESSURE_OK)
    {
        return PRESSURE_ERROR;
    }
    delay(100);

    if (LPS25HB_Power(baro, false) != PRESSURE_OK)
    {
        return PRESSURE_ERROR;
    }

    bool ack = busRead(baro->busDev, LPS25HB_CTRL_REG1_ADDR, &tmp1);
    if (ack != PRESSURE_OK)
    {
        return PRESSURE_ERROR;
    }

    /* Output Data Rate selection */
    tmp1 &= ~(LPS25HB_ODR_MASK);
    tmp1 |= LPS25HB_ODR_25Hz;           // output data rate set to maximum

    /* Interrupt circuit selection */
    tmp1 &= ~(LPS25HB_DIFF_EN_MASK);
    tmp1 |= LPS25HB_DIFF_ENABLE;        // interrupt generation disabled

    /* Block Data Update selection */
    tmp1 &= ~(LPS25HB_BDU_MASK);
    tmp1 |= LPS25HB_BDU_CONT;           // continuous mode

    /* Serial Interface Mode selection */
//    tmp1 &= ~(LPS25HB_SPI_SIM_MASK);
//    tmp1 |= LPS25HB_SPI_SIM_3W;         // 3-wire interface

    ack = busWrite(baro->busDev, LPS25HB_CTRL_REG1_ADDR, tmp1);
    if (ack != PRESSURE_OK)
    {
        return PRESSURE_ERROR;
    }

    tmp1 = 0;
    ack = busRead(baro->busDev, LPS25HB_RES_CONF_ADDR, &tmp1);
    if (ack != PRESSURE_OK)
    {
        return PRESSURE_ERROR;
    }

    tmp1 &= ~(LPS25HB_P_RES_MASK);
    tmp1 |= LPS25HB_P_RES_AVG_512;       // PressureResolution;

     tmp1 &= ~(LPS25HB_T_RES_MASK);
    tmp1 |= LPS25HB_T_RES_AVG_32;       // TemperatureResolution;

    ack = busWrite(baro->busDev, LPS25HB_RES_CONF_ADDR, tmp1);
    if (ack != PRESSURE_OK)
    {
        return PRESSURE_ERROR;
    }

    if (LPS25HB_Power(baro, true) != PRESSURE_OK)
    {
        return PRESSURE_ERROR;
    }
    return PRESSURE_OK;
}


// check the status register bit to be present
static bool data_ready(baroDev_t * baro, uint8_t bit)
{
    uint8_t tmp = 0;

    bool ack = busRead(baro->busDev, LPS25HB_STATUS_REG_ADDR, &tmp);
    if (!ack)
    {
        return false;
    }

    if (tmp & (1 << bit))
    {
        return true;
    }
    else
    {
        return false;
    }

}
// reads pressure and returns raw 24-bit sensor output
static int32_t readPressureRaw(baroDev_t * baro)
{
    static int32_t _pressure    = 0;
    uint8_t buffer[3] = {0};

    if (data_ready(baro, PRESSURE_READY))
    {
        bool ack = busReadBuf(baro->busDev, LPS25HB_PRESS_POUT_XL_ADDR, buffer, sizeof(buffer));
        UNUSED(ack);
        _pressure = (int32_t) (buffer[2] << 16 | buffer[1] << 8 | buffer[0]);
   }

    return _pressure;
}

// reads temperature and returns raw 16-bit sensor output
static int16_t readTemperatureRaw(baroDev_t * baro)
{
    static int16_t _temperature = 0;
    uint8_t data[2] = {0};

    if (data_ready(baro, TEMPERATURE_READY))
    {
        bool ack = busReadBuf(baro->busDev, LPS25HB_TEMP_OUT_L_ADDR, data, sizeof(data));
        UNUSED(ack);
        // combine bytes
        _temperature = (int16_t)(data[1] << 8 | data[0]);
    }

    return _temperature;
}

static bool lps25h_start_up(baroDev_t * baro)
{
    UNUSED(baro);
    return true;
}

static bool lps25h_get_up(baroDev_t * baro)
{
    UNUSED(baro);
    return true;
}


#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(busDevice_t * busDev)
{
    for (int retry = 0; retry < DETECTION_MAX_RETRY_COUNT; retry++)
    {
        uint8_t chipId = 0;

        delay(100);

        bool ack = busRead(busDev, LPS25HB_WHO_AM_I_ADDR, &chipId);
        // verified: id is 189 (0xBD)
        //debug[0] = chipId;

        if (ack && chipId == LPS25H_CHIP_ID)
        {
            return true;
        }
    };

    return false;
}

STATIC_UNIT_TESTED bool lps25h_calculate(baroDev_t * baro, int32_t * pressure, int32_t * temperature)
{
    UNUSED(baro);

    if (pressure)
    {
        *pressure = (float)readPressureRaw(baro) / 4096.0f;
     }

    if (temperature)
    {
        *temperature = 42.5f + (float)readTemperatureRaw(baro) / 480.0f;
    }

    return true;
}

bool lps25hDetect(baroDev_t *baro)
{
    baro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_LPS25H, 0, OWNER_BARO);
    if (baro->busDev == NULL) {
        return false;
    }

    busSetSpeed(baro->busDev, BUS_SPEED_STANDARD);

    if (!deviceDetect(baro->busDev)) {
        busDeviceDeInit(baro->busDev);
        return false;
    }

    LPS25HB_Init(baro);

    baro->ut_delay = 0;
    baro->get_ut = lps25h_get_ut;
    baro->start_ut = lps25h_start_ut;

    baro->up_delay = 1000;
    baro->start_up = lps25h_start_up;
    baro->get_up = lps25h_get_up;

    baro->calculate = lps25h_calculate;

    return true;
}


#endif

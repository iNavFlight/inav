/**
  ******************************************************************************
  * @file    mfxstm32l152_reg.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the
  *          mfxstm32l152_reg.c IO expander driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MFXSTM32L152_REG_H
#define MFXSTM32L152_REG_H

#ifdef __cplusplus
 extern "C" {
#endif   
   
#include <stdint.h>
   
/* Includes ------------------------------------------------------------------*/
/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup Component
  * @{
  */
    
/** @defgroup MFXSTM32L152
  * @{
  */    
  
/** @addtogroup MFXSTM32L152_Exported_Constants
  * @{
  */ 
   
/**
  * @brief Register address: chip IDs (R)
  */
#define MFXSTM32L152_REG_ADR_ID                 0x00U

/**
  * @brief Register address: chip FW_VERSION  (R)
  */
#define MFXSTM32L152_REG_ADR_FW_VERSION_MSB     0x01U
#define MFXSTM32L152_REG_ADR_FW_VERSION_LSB     0x00U

/**
  * @brief Register address: System Control Register (R/W)
  */
#define MFXSTM32L152_REG_ADR_SYS_CTRL           0x40U

/**
  * @brief Register address: Vdd monitoring (R)
 */
#define MFXSTM32L152_REG_ADR_VDD_REF_MSB        0x06U
#define MFXSTM32L152_REG_ADR_VDD_REF_LSB        0x07U

/**
  * @brief Register address: Error source 
  */
#define MFXSTM32L152_REG_ADR_ERROR_SRC          0x03U

/**
  * @brief Register address: Error Message
  */
#define MFXSTM32L152_REG_ADR_ERROR_MSG          0x04U

/**
  * @brief Reg Addr IRQs: to config the pin that informs Main MCU that MFX events appear
  */
#define MFXSTM32L152_REG_ADR_MFX_IRQ_OUT        0x41U

/**
  * @brief Reg Addr IRQs: to select the events which activate the MFXSTM32L152_IRQ_OUT signal 
  */
#define MFXSTM32L152_REG_ADR_IRQ_SRC_EN         0x42U

/**
  * @brief Reg Addr IRQs: the Main MCU must read the IRQ_PENDING register to know the interrupt reason
  */
#define MFXSTM32L152_REG_ADR_IRQ_PENDING        0x08U

/**
  * @brief Reg Addr IRQs: the Main MCU must acknowledge it thanks to a writing access to the IRQ_ACK register 
  */
#define MFXSTM32L152_REG_ADR_IRQ_ACK            0x44U
   
/**
  * @brief  GPIO: 24 programmable input/output called MFXSTM32L152_GPIO[23:0] are provided
  */

/**
  * @brief Reg addr: GPIO DIRECTION (R/W): GPIO pins direction: (0) input, (1) output.
  */
#define MFXSTM32L152_REG_ADR_GPIO_DIR1          0x60U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_GPIO_DIR2          0x61U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_GPIO_DIR3          0x62U  /* agpio [0:7] */

/**
  * @brief Reg addr: GPIO TYPE (R/W): If GPIO in output: (0) output push pull, (1) output open drain.
             If GPIO in input: (0) input without pull resistor, (1) input with pull resistor.
  */
#define MFXSTM32L152_REG_ADR_GPIO_TYPE1         0x64U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_GPIO_TYPE2         0x65U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_GPIO_TYPE3         0x66U  /* agpio [0:7] */

/**
  * @brief Reg addr: GPIO PULL_UP_PULL_DOWN (R/W)
  */
#define MFXSTM32L152_REG_ADR_GPIO_PUPD1         0x68U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_GPIO_PUPD2         0x69U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_GPIO_PUPD3         0x6AU  /* agpio [0:7] */

/**
  * @brief Reg addr: GPIO SET (W): When GPIO is in output mode, write (1) puts 
           the corresponding GPO in High level.
 */
#define MFXSTM32L152_REG_ADR_GPO_SET1           0x6CU  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_GPO_SET2           0x6DU  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_GPO_SET3           0x6EU  /* agpio [0:7] */

/**
  * @brief Reg addr: GPIO CLEAR (W): When GPIO is in output mode, write (1) puts 
           the corresponding GPO in Low level.
 */
#define MFXSTM32L152_REG_ADR_GPO_CLR1           0x70U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_GPO_CLR2           0x71U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_GPO_CLR3           0x72U  /* agpio [0:7] */

/**
  * @brief Reg addr: GPIO STATE (R): Give state of the GPIO pin.
  */
#define MFXSTM32L152_REG_ADR_GPIO_STATE1         0x10U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_GPIO_STATE2         0x11U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_GPIO_STATE3         0x12U  /* agpio [0:7] */

/* GPIOs can INDIVIDUALLY generate interruption to the Main MCU thanks to the MFXSTM32L152_IRQ_OUT signal */
/* the general MFXSTM32L152_IRQ_GPIO_SRC_EN shall be enabled too          */
/* GPIO IRQ_GPI_SRC1/2/3 (R/W): registers enable or not the feature to generate irq */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_SRC1       0x48U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_SRC2       0x49U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_SRC3       0x4AU  /* agpio [0:7] */

/**
  * @brief GPIO IRQ_GPI_EVT1/2/3 (R/W): Irq generated on level (0) or edge (1).
  */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_EVT1       0x4CU  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_EVT2       0x4DU  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_EVT3       0x4EU  /* agpio [0:7] */

/**
  * @brief GPIO IRQ_GPI_TYPE1/2/3 (R/W): Irq generated on (0) : Low level or Falling edge. 
          (1) : High level or Rising edge.
  */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_TYPE1      0x50U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_TYPE2      0x51U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_TYPE3      0x52U  /* agpio [0:7] */

/**
  * @brief GPIO IRQ_GPI_PENDING1/2/3 (R): irq occurs
  */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_PENDING1   0x0CU  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_PENDING2   0x0DU  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_PENDING3   0x0EU  /* agpio [0:7] */

/**
  * @brief GPIO IRQ_GPI_ACK1/2/3 (W): Write (1) to acknowledge IRQ event
  */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_ACK1       0x54U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_ACK2       0x55U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_ACK3       0x56U  /* agpio [0:7] */ 
                                             
/**
  * @brief Touch Screen Registers
 */                 
#define MFXSTM32L152_TS_SETTLING                0xA0U
#define MFXSTM32L152_TS_TOUCH_DET_DELAY         0xA1U
#define MFXSTM32L152_TS_AVE                     0xA2U
#define MFXSTM32L152_TS_TRACK                   0xA3U
#define MFXSTM32L152_TS_FIFO_TH                 0xA4U
#define MFXSTM32L152_TS_FIFO_STA                0x20U
#define MFXSTM32L152_TS_FIFO_LEVEL              0x21U
#define MFXSTM32L152_TS_XY_DATA                 0x24U

/**
  * @brief Register address: Idd control register (R/W)
  */
#define MFXSTM32L152_REG_ADR_IDD_CTRL           0x80U

/**
  * @brief Register address: Idd pre delay  register (R/W)
  */
#define MFXSTM32L152_REG_ADR_IDD_PRE_DELAY      0x81U

/**
  * @brief Register address: Idd Shunt registers (R/W)
  */
#define MFXSTM32L152_REG_ADR_IDD_SHUNT0_MSB     0x82U
#define MFXSTM32L152_REG_ADR_IDD_SHUNT0_LSB     0x83U
#define MFXSTM32L152_REG_ADR_IDD_SHUNT1_MSB     0x84U
#define MFXSTM32L152_REG_ADR_IDD_SHUNT1_LSB     0x85U
#define MFXSTM32L152_REG_ADR_IDD_SHUNT2_MSB     0x86U
#define MFXSTM32L152_REG_ADR_IDD_SHUNT2_LSB     0x87U
#define MFXSTM32L152_REG_ADR_IDD_SHUNT3_MSB     0x88U
#define MFXSTM32L152_REG_ADR_IDD_SHUNT3_LSB     0x89U
#define MFXSTM32L152_REG_ADR_IDD_SHUNT4_MSB     0x8AU
#define MFXSTM32L152_REG_ADR_IDD_SHUNT4_LSB     0x8BU

/**
  * @brief Register address: Idd ampli gain register (R/W)
  */
#define MFXSTM32L152_REG_ADR_IDD_GAIN_MSB       0x8CU
#define MFXSTM32L152_REG_ADR_IDD_GAIN_LSB       0x8DU

/**
  * @brief Register address: Idd VDD min register (R/W)
  */
#define MFXSTM32L152_REG_ADR_IDD_VDD_MIN_MSB    0x8EU
#define MFXSTM32L152_REG_ADR_IDD_VDD_MIN_LSB    0x8FU

/**
  * @brief Register address: Idd value register (R)
  */
#define MFXSTM32L152_REG_ADR_IDD_VALUE_MSB      0x14U
#define MFXSTM32L152_REG_ADR_IDD_VALUE_MID      0x15U
#define MFXSTM32L152_REG_ADR_IDD_VALUE_LSB      0x16U

/**
  * @brief Register address: Idd calibration offset register (R)
  */
#define MFXSTM32L152_REG_ADR_IDD_CAL_OFFSET_MSB 0x18U
#define MFXSTM32L152_REG_ADR_IDD_CAL_OFFSET_LSB 0x19U

/**
  * @brief Register address: Idd shunt used offset register (R)
  */
#define MFXSTM32L152_REG_ADR_IDD_SHUNT_USED     0x1AU

/**
  * @brief Register address: shunt stabilisation delay registers (R/W)
  */
#define MFXSTM32L152_REG_ADR_IDD_SH0_STABILIZATION  0x90U
#define MFXSTM32L152_REG_ADR_IDD_SH1_STABILIZATION  0x91U
#define MFXSTM32L152_REG_ADR_IDD_SH2_STABILIZATION  0x92U
#define MFXSTM32L152_REG_ADR_IDD_SH3_STABILIZATION  0x93U
#define MFXSTM32L152_REG_ADR_IDD_SH4_STABILIZATION  0x94U

/**
  * @brief Register address: Idd number of measurements register (R/W)
  */
#define MFXSTM32L152_REG_ADR_IDD_NBR_OF_MEAS        0x96U

/**
  * @brief Register address: Idd delta delay between 2 measurements register (R/W)
  */
#define MFXSTM32L152_REG_ADR_IDD_MEAS_DELTA_DELAY   0x97U

/**
  * @brief Register address: Idd number of shunt on board register (R/W)
  */
#define MFXSTM32L152_REG_ADR_IDD_SHUNTS_ON_BOARD    0x98U
/**
  * @}
  */

/** @addtogroup MFXSTM32L152_Exported_Types
  * @{
  */

typedef int32_t (*MFXSTM32L152_Write_Func)(void *, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*MFXSTM32L152_Read_Func) (void *, uint16_t, uint8_t*, uint16_t);

typedef struct
{
  MFXSTM32L152_Write_Func   WriteReg;
  MFXSTM32L152_Read_Func    ReadReg;
  void                      *handle;
  /* Internal resources */
} mfxstm32l152_ctx_t;

/**
  * @}
  */ 

/** @addtogroup MFXSTM32L152_Exported_Functions
  * @{
  */
int32_t mfxstm32l152_write_reg(mfxstm32l152_ctx_t *ctx, uint16_t reg, uint8_t *data, uint16_t length);
int32_t mfxstm32l152_read_reg(mfxstm32l152_ctx_t *ctx, uint16_t reg, uint8_t *data, uint16_t length);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif
#endif /* MFXSTM32L152_REG_H */


/**
  * @}
  */

/**
  * @}
  */ 

/**
  * @}
  */       
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

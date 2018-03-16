/*
    SPC5 HAL - Copyright (C) 2013 STMicroelectronics

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    SPC570Sxx/spc5_registry.h
 * @brief   SPC570Sxx capabilities registry.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef SPC5_REGISTRY_H
#define SPC5_REGISTRY_H

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/


#if defined(_SPC570S50L1_) || defined(_SPC570S50L3_)
#define _SPC570S50_
#define _SPC570SXX_SMALL_
#else
#error "SPC57xSxx platform not defined"
#endif

/*===========================================================================*/
/* Platform capabilities.                                                    */
/*===========================================================================*/

/**
 * @name    SPC570Sxx capabilities
 * @{
 */
/* Clock attributes.*/
#if defined(_SPC570SXX_SMALL_)
#define SPC5_HAS_FMPLL1                     FALSE
#define SPC5_HAS_CLOCKOUT                   TRUE
#define SPC5_HAS_AC0                        FALSE
#define SPC5_HAS_AC1                        FALSE
#define SPC5_HAS_AC2                        FALSE
#define SPC5_HAS_AC3                        FALSE
#define SPC5_HAS_CMU0                       TRUE
#define SPC5_HAS_CMU1                       FALSE
#endif

/* DSPI attribures.*/
#define SPC5_HAS_DSPI0                      TRUE
#define SPC5_HAS_DSPI1                      TRUE
#define SPC5_HAS_DSPI2                      TRUE
#define SPC5_DSPI_FIFO_DEPTH                5
#define SPC5_DSPI0_PCTL                     4
#define SPC5_DSPI1_PCTL                     5
#define SPC5_DSPI2_PCTL                     6
#define SPC5_DSPI0_TX1_DMA_DEV_ID           1
#define SPC5_DSPI0_TX2_DMA_DEV_ID           0
#define SPC5_DSPI0_RX_DMA_DEV_ID            2
#define SPC5_DSPI1_TX1_DMA_DEV_ID           3
#define SPC5_DSPI1_TX2_DMA_DEV_ID           0
#define SPC5_DSPI1_RX_DMA_DEV_ID            4
#define SPC5_DSPI2_TX1_DMA_DEV_ID           5
#define SPC5_DSPI2_TX2_DMA_DEV_ID           0
#define SPC5_DSPI2_RX_DMA_DEV_ID            6
#define SPC5_DSPI0_TFFF_HANDLER             vector76
#define SPC5_DSPI0_TFFF_NUMBER              76
#define SPC5_DSPI0_RFDF_HANDLER             vector78
#define SPC5_DSPI0_RFDF_NUMBER              78
#define SPC5_DSPI1_TFFF_HANDLER             vector96
#define SPC5_DSPI1_TFFF_NUMBER              96
#define SPC5_DSPI1_RFDF_HANDLER             vector98
#define SPC5_DSPI1_RFDF_NUMBER              98
#define SPC5_DSPI2_TFFF_HANDLER             vector116
#define SPC5_DSPI2_TFFF_NUMBER              116
#define SPC5_DSPI2_RFDF_HANDLER             vector118
#define SPC5_DSPI2_RFDF_NUMBER              118
#define SPC5_DSPI0_ENABLE_CLOCK()                                           \
  halSPCSetPeripheralClockMode(SPC5_DSPI0_PCTL, SPC5_SPI_DSPI0_START_PCTL)
#define SPC5_DSPI0_DISABLE_CLOCK()                                          \
  halSPCSetPeripheralClockMode(SPC5_DSPI0_PCTL, SPC5_SPI_DSPI0_STOP_PCTL)
#define SPC5_DSPI1_ENABLE_CLOCK()                                           \
  halSPCSetPeripheralClockMode(SPC5_DSPI1_PCTL, SPC5_SPI_DSPI1_START_PCTL)
#define SPC5_DSPI1_DISABLE_CLOCK()                                          \
  halSPCSetPeripheralClockMode(SPC5_DSPI1_PCTL, SPC5_SPI_DSPI1_STOP_PCTL)
#define SPC5_DSPI2_ENABLE_CLOCK()                                           \
  halSPCSetPeripheralClockMode(SPC5_DSPI2_PCTL, SPC5_SPI_DSPI2_START_PCTL)
#define SPC5_DSPI2_DISABLE_CLOCK()                                          \
  halSPCSetPeripheralClockMode(SPC5_DSPI2_PCTL, SPC5_SPI_DSPI2_STOP_PCTL)

#if defined(_SPC570SXX_MEDIUM_) || defined(_SPC570SXX_LARGE_)
#define SPC5_HAS_DSPI3                      TRUE
#define SPC5_DSPI3_PCTL                     7
#define SPC5_DSPI3_TX1_DMA_DEV_ID           7
#define SPC5_DSPI3_TX2_DMA_DEV_ID           0
#define SPC5_DSPI3_RX_DMA_DEV_ID            8
#define SPC5_DSPI3_TFFF_HANDLER             vector219
#define SPC5_DSPI3_TFFF_NUMBER              219
#define SPC5_DSPI3_RFDF_HANDLER             vector221
#define SPC5_DSPI3_RFDF_NUMBER              221
#define SPC5_DSPI3_ENABLE_CLOCK()                                           \
  halSPCSetPeripheralClockMode(SPC5_DSPI3_PCTL, SPC5_SPI_DSPI3_START_PCTL)
#define SPC5_DSPI3_DISABLE_CLOCK()                                          \
  halSPCSetPeripheralClockMode(SPC5_DSPI3_PCTL, SPC5_SPI_DSPI3_STOP_PCTL)
#else
#define SPC5_HAS_DSPI3                      FALSE
#endif

#if defined(_SPC570SXX_LARGE_)
#define SPC5_HAS_DSPI4                      TRUE
#define SPC5_DSPI4_PCTL                     8
#define SPC5_DSPI4_TX1_DMA_DEV_ID           15
#define SPC5_DSPI4_TX2_DMA_DEV_ID           0
#define SPC5_DSPI4_RX_DMA_DEV_ID            21
#define SPC5_DSPI4_TFFF_HANDLER             vector258
#define SPC5_DSPI4_TFFF_NUMBER              258
#define SPC5_DSPI4_RFDF_HANDLER             vector260
#define SPC5_DSPI4_RFDF_NUMBER              260
#define SPC5_DSPI4_ENABLE_CLOCK()                                           \
  halSPCSetPeripheralClockMode(SPC5_DSPI4_PCTL, SPC5_SPI_DSPI4_START_PCTL)
#define SPC5_DSPI4_DISABLE_CLOCK()                                          \
  halSPCSetPeripheralClockMode(SPC5_DSPI0_PCTL, SPC5_SPI_DSPI4_STOP_PCTL)
#else
#define SPC5_HAS_DSPI4                      FALSE
#endif

#define SPC5_HAS_DSPI5                      FALSE
#define SPC5_HAS_DSPI6                      FALSE
#define SPC5_HAS_DSPI7                      FALSE

/* eDMA attributes.*/
#define SPC5_HAS_EDMA                       TRUE
#define SPC5_EDMA_NCHANNELS                 16
#define SPC5_EDMA_HAS_MUX                   TRUE

/* LINFlex attributes.*/
#define SPC5_HAS_LINFLEX0                   TRUE
#define SPC5_LINFLEX0_PCTL                  92
#define SPC5_LINFLEX0_RXI_HANDLER           vector376
#define SPC5_LINFLEX0_TXI_HANDLER           vector377
#define SPC5_LINFLEX0_ERR_HANDLER           vector378
#define SPC5_LINFLEX0_RXI_NUMBER            376
#define SPC5_LINFLEX0_TXI_NUMBER            377
#define SPC5_LINFLEX0_ERR_NUMBER            378
#define SPC5_LINFLEX0_CLK                   SPC5_LIN_CLK

#define SPC5_HAS_LINFLEX1                   TRUE
#define SPC5_LINFLEX1_PCTL                  220
#define SPC5_LINFLEX1_RXI_HANDLER           vector384
#define SPC5_LINFLEX1_TXI_HANDLER           vector385
#define SPC5_LINFLEX1_ERR_HANDLER           vector386
#define SPC5_LINFLEX1_RXI_NUMBER            384
#define SPC5_LINFLEX1_TXI_NUMBER            385
#define SPC5_LINFLEX1_ERR_NUMBER            386
#define SPC5_LINFLEX1_CLK                   SPC5_LIN_CLK

#define SPC5_HAS_LINFLEX2                   FALSE
#define SPC5_HAS_LINFLEX3                   FALSE
#define SPC5_HAS_LINFLEX4                   FALSE
#define SPC5_HAS_LINFLEX5                   FALSE
#define SPC5_HAS_LINFLEX6                   FALSE
#define SPC5_HAS_LINFLEX7                   FALSE
#define SPC5_HAS_LINFLEX8                   FALSE
#define SPC5_HAS_LINFLEX9                   FALSE

/* SIUL attributes.*/
#define SPC5_HAS_SIUL                       TRUE
#define SPC5_SIUL_NUM_PORTS                 8
#if defined(_SPC570SXX_SMALL_)
#define SPC5_SIUL_NUM_PCRS                  72
#else
#define SPC5_SIUL_NUM_PCRS                  108
#endif
#define SPC5_SIUL_NUM_PADSELS               36

/* FlexPWM attributes.*/
#if defined(_SPC570SXX_SMALL_) || defined(_SPC570SXX_MEDIUM_)
#define SPC5_HAS_FLEXPWM0                   TRUE
#define SPC5_FLEXPWM0_PCTL                  41
#define SPC5_FLEXPWM0_RF0_HANDLER           vector179
#define SPC5_FLEXPWM0_COF0_HANDLER          vector180
#define SPC5_FLEXPWM0_CAF0_HANDLER          vector181
#define SPC5_FLEXPWM0_RF1_HANDLER           vector182
#define SPC5_FLEXPWM0_COF1_HANDLER          vector183
#define SPC5_FLEXPWM0_CAF1_HANDLER          vector184
#define SPC5_FLEXPWM0_RF2_HANDLER           vector185
#define SPC5_FLEXPWM0_COF2_HANDLER          vector186
#define SPC5_FLEXPWM0_CAF2_HANDLER          vector187
#define SPC5_FLEXPWM0_RF3_HANDLER           vector188
#define SPC5_FLEXPWM0_COF3_HANDLER          vector189
#define SPC5_FLEXPWM0_CAF3_HANDLER          vector190
#define SPC5_FLEXPWM0_FFLAG_HANDLER         vector191
#define SPC5_FLEXPWM0_REF_HANDLER           vector192
#define SPC5_FLEXPWM0_RF0_NUMBER            179
#define SPC5_FLEXPWM0_COF0_NUMBER           180
#define SPC5_FLEXPWM0_CAF0_NUMBER           181
#define SPC5_FLEXPWM0_RF1_NUMBER            182
#define SPC5_FLEXPWM0_COF1_NUMBER           183
#define SPC5_FLEXPWM0_CAF1_NUMBER           184
#define SPC5_FLEXPWM0_RF2_NUMBER            185
#define SPC5_FLEXPWM0_COF2_NUMBER           186
#define SPC5_FLEXPWM0_CAF2_NUMBER           187
#define SPC5_FLEXPWM0_RF3_NUMBER            188
#define SPC5_FLEXPWM0_COF3_NUMBER           189
#define SPC5_FLEXPWM0_CAF3_NUMBER           190
#define SPC5_FLEXPWM0_FFLAG_NUMBER          191
#define SPC5_FLEXPWM0_REF_NUMBER            192
#define SPC5_FLEXPWM0_CLK                   SPC5_MCONTROL_CLK
#else /* defined(_SPC570SXX_LARGE_) */
#define SPC5_HAS_FLEXPWM0                   FALSE
#endif /* defined(_SPC570SXX_LARGE_) */

#define SPC5_HAS_FLEXPWM1                   FALSE

/* eTimer attributes.*/
#define SPC5_HAS_ETIMER0                    TRUE
#define SPC5_ETIMER0_PCTL                   129
#define SPC5_ETIMER0_TC0IR_HANDLER          vector706
#define SPC5_ETIMER0_TC1IR_HANDLER          vector707
#define SPC5_ETIMER0_TC2IR_HANDLER          vector708
#define SPC5_ETIMER0_TC3IR_HANDLER          vector709
#define SPC5_ETIMER0_TC4IR_HANDLER          vector710
#define SPC5_ETIMER0_TC5IR_HANDLER          vector711
#define SPC5_ETIMER0_WTIF_HANDLER           vector714
#define SPC5_ETIMER0_RCF_HANDLER            vector715
#define SPC5_ETIMER0_TC0IR_NUMBER           706
#define SPC5_ETIMER0_TC1IR_NUMBER           707
#define SPC5_ETIMER0_TC2IR_NUMBER           708
#define SPC5_ETIMER0_TC3IR_NUMBER           709
#define SPC5_ETIMER0_TC4IR_NUMBER           710
#define SPC5_ETIMER0_TC5IR_NUMBER           711
#define SPC5_ETIMER0_WTIF_NUMBER            714
#define SPC5_ETIMER0_RCF_NUMBER             715
#define SPC5_ETIMER0_CLK                    SPC5_ETIMER_CLK

#define SPC5_HAS_ETIMER1                    TRUE
#define SPC5_ETIMER1_PCTL                   128
#define SPC5_ETIMER1_TC0IR_HANDLER          vector717
#define SPC5_ETIMER1_TC1IR_HANDLER          vector718
#define SPC5_ETIMER1_TC2IR_HANDLER          vector719
#define SPC5_ETIMER1_TC3IR_HANDLER          vector720
#define SPC5_ETIMER1_TC4IR_HANDLER          vector721
#define SPC5_ETIMER1_TC5IR_HANDLER          vector722
#define SPC5_ETIMER1_RCF_HANDLER            vector726
#define SPC5_ETIMER1_TC0IR_NUMBER           717
#define SPC5_ETIMER1_TC1IR_NUMBER           718
#define SPC5_ETIMER1_TC2IR_NUMBER           719
#define SPC5_ETIMER1_TC3IR_NUMBER           720
#define SPC5_ETIMER1_TC4IR_NUMBER           721
#define SPC5_ETIMER1_TC5IR_NUMBER           722
#define SPC5_ETIMER1_RCF_NUMBER             726
#define SPC5_ETIMER1_CLK                    SPC5_ETIMER_CLK

#define SPC5_HAS_ETIMER2                    TRUE
#define SPC5_ETIMER2_PCTL                   131
#define SPC5_ETIMER2_TC0IR_HANDLER          vector728
#define SPC5_ETIMER2_TC1IR_HANDLER          vector729
#define SPC5_ETIMER2_TC2IR_HANDLER          vector730
#define SPC5_ETIMER2_TC3IR_HANDLER          vector731
#define SPC5_ETIMER2_TC4IR_HANDLER          vector732
#define SPC5_ETIMER2_TC5IR_HANDLER          vector733
#define SPC5_ETIMER2_RCF_HANDLER            vector737
#define SPC5_ETIMER2_TC0IR_NUMBER           728
#define SPC5_ETIMER2_TC1IR_NUMBER           729
#define SPC5_ETIMER2_TC2IR_NUMBER           730
#define SPC5_ETIMER2_TC3IR_NUMBER           731
#define SPC5_ETIMER2_TC4IR_NUMBER           732
#define SPC5_ETIMER2_TC5IR_NUMBER           733
#define SPC5_ETIMER2_RCF_NUMBER             737
#define SPC5_ETIMER2_CLK                    SPC5_ETIMER_CLK

#define SPC5_HAS_ETIMER3                    TRUE
#define SPC5_ETIMER3_PCTL                   130
#define SPC5_ETIMER3_TC0IR_HANDLER          vector739
#define SPC5_ETIMER3_TC1IR_HANDLER          vector740
#define SPC5_ETIMER3_TC2IR_HANDLER          vector741
#define SPC5_ETIMER3_TC3IR_HANDLER          vector742
#define SPC5_ETIMER3_TC4IR_HANDLER          vector743
#define SPC5_ETIMER3_TC5IR_HANDLER          vector744
#define SPC5_ETIMER3_RCF_HANDLER            vector748
#define SPC5_ETIMER3_TC0IR_NUMBER           739
#define SPC5_ETIMER3_TC1IR_NUMBER           740
#define SPC5_ETIMER3_TC2IR_NUMBER           741
#define SPC5_ETIMER3_TC3IR_NUMBER           742
#define SPC5_ETIMER3_TC4IR_NUMBER           743
#define SPC5_ETIMER3_TC5IR_NUMBER           744
#define SPC5_ETIMER3_RCF_NUMBER             748
#define SPC5_ETIMER3_CLK                    SPC5_ETIMER_CLK

/* FlexCAN attributes.*/
#define SPC5_HAS_FLEXCAN0                                   TRUE
#define SPC5_FLEXCAN0_PCTL                                  79
#define SPC5_FLEXCAN0_MB                                    32
#define SPC5_FLEXCAN0_SHARED_IRQ                            TRUE
#define SPC5_FLEXCAN0_FLEXCAN_ESR_ERR_INT_HANDLER           vector687
#define SPC5_FLEXCAN0_FLEXCAN_ESR_BOFF_HANDLER              vector688
//#define SPC5_FLEXCAN0_FLEXCAN_ESR_WAK_HANDLER               vector67
#define SPC5_FLEXCAN0_FLEXCAN_BUF_00_03_HANDLER             vector677
#define SPC5_FLEXCAN0_FLEXCAN_BUF_04_07_HANDLER             vector678
#define SPC5_FLEXCAN0_FLEXCAN_BUF_08_11_HANDLER             vector679
#define SPC5_FLEXCAN0_FLEXCAN_BUF_12_15_HANDLER             vector680
#define SPC5_FLEXCAN0_FLEXCAN_BUF_16_31_HANDLER             vector681
#define SPC5_FLEXCAN0_FLEXCAN_ESR_ERR_INT_NUMBER            687
#define SPC5_FLEXCAN0_FLEXCAN_ESR_BOFF_NUMBER               688
//#define SPC5_FLEXCAN0_FLEXCAN_ESR_WAK_NUMBER                67
#define SPC5_FLEXCAN0_FLEXCAN_BUF_00_03_NUMBER              677
#define SPC5_FLEXCAN0_FLEXCAN_BUF_04_07_NUMBER              678
#define SPC5_FLEXCAN0_FLEXCAN_BUF_08_11_NUMBER              679
#define SPC5_FLEXCAN0_FLEXCAN_BUF_12_15_NUMBER              680
#define SPC5_FLEXCAN0_FLEXCAN_BUF_16_31_NUMBER              681
#define SPC5_FLEXCAN0_ENABLE_CLOCK()                                        \
  halSPCSetPeripheralClockMode(SPC5_FLEXCAN0_PCTL, SPC5_CAN_FLEXCAN0_START_PCTL)
#define SPC5_FLEXCAN0_DISABLE_CLOCK()                                       \
  halSPCSetPeripheralClockMode(SPC5_FLEXCAN0_PCTL, SPC5_CAN_FLEXCAN0_STOP_PCTL)

/* ADC attributes.*/
#define SPC5_ADC_HAS_TRC                    TRUE

#define SPC5_HAS_ADC0                       TRUE
#define SPC5_ADC_ADC0_HAS_CTR0              TRUE
#define SPC5_ADC_ADC0_HAS_CTR1              FALSE
#define SPC5_ADC_ADC0_HAS_CTR2              FALSE
#define SPC5_ADC_ADC0_HAS_NCMR0             TRUE
#define SPC5_ADC_ADC0_HAS_NCMR1             FALSE
#define SPC5_ADC_ADC0_HAS_NCMR2             FALSE
#define SPC5_ADC_ADC0_HAS_THRHLR0           TRUE
#define SPC5_ADC_ADC0_HAS_THRHLR1           TRUE
#define SPC5_ADC_ADC0_HAS_THRHLR2           TRUE
#define SPC5_ADC_ADC0_HAS_THRHLR3           TRUE
#define SPC5_ADC_ADC0_HAS_CIMR0             FALSE
#define SPC5_ADC_ADC0_HAS_CIMR1             FALSE
#define SPC5_ADC_ADC0_HAS_CIMR2             FALSE
#define SPC5_ADC_ADC0_HAS_CEOCFR0           FALSE
#define SPC5_ADC_ADC0_HAS_CEOCFR1           FALSE
#define SPC5_ADC_ADC0_HAS_CEOCFR2           FALSE
#define SPC5_ADC0_PCTL                      32
#define SPC5_ADC0_DMA_DEV_ID                20
#define SPC5_ADC0_EOC_HANDLER               vector62
#define SPC5_ADC0_EOC_NUMBER                62
#define SPC5_ADC0_WD_HANDLER                vector64
#define SPC5_ADC0_WD_NUMBER                 64

#define SPC5_HAS_ADC1                       TRUE
#define SPC5_ADC_ADC1_HAS_CTR0              TRUE
#define SPC5_ADC_ADC1_HAS_CTR1              FALSE
#define SPC5_ADC_ADC1_HAS_CTR2              FALSE
#define SPC5_ADC_ADC1_HAS_NCMR0             TRUE
#define SPC5_ADC_ADC1_HAS_NCMR1             FALSE
#define SPC5_ADC_ADC1_HAS_NCMR2             FALSE
#define SPC5_ADC_ADC1_HAS_THRHLR0           TRUE
#define SPC5_ADC_ADC1_HAS_THRHLR1           TRUE
#define SPC5_ADC_ADC1_HAS_THRHLR2           TRUE
#define SPC5_ADC_ADC1_HAS_THRHLR3           TRUE
#define SPC5_ADC_ADC0_HAS_CIMR0             FALSE
#define SPC5_ADC_ADC0_HAS_CIMR1             FALSE
#define SPC5_ADC_ADC0_HAS_CIMR2             FALSE
#define SPC5_ADC_ADC0_HAS_CEOCFR0           FALSE
#define SPC5_ADC_ADC0_HAS_CEOCFR1           FALSE
#define SPC5_ADC_ADC0_HAS_CEOCFR2           FALSE
#define SPC5_ADC1_PCTL                      33
#define SPC5_ADC1_DMA_DEV_ID                21
#define SPC5_ADC1_EOC_HANDLER               vector82
#define SPC5_ADC1_EOC_NUMBER                82
#define SPC5_ADC1_WD_HANDLER                vector84
#define SPC5_ADC1_WD_NUMBER                 84
/** @} */

#endif /* SPC5_REGISTRY_H */

/** @} */

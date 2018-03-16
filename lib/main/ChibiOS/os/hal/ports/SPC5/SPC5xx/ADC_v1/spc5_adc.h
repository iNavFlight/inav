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
 * @file    ADC_v1/spc5_adc.h
 * @brief   SPC5xx eTimer header file.
 *
 * @addtogroup ADC
 * @{
 */

#ifndef _SPC5_ADC_H_
#define _SPC5_ADC_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   SPC5 FlexPWM registers block.
 * @note    Redefined from the SPC5 headers because the non uniform
 *          declaration of the SubModules registers among the various
 *          sub-families.
 */
struct spc5_adc {

  union {
    vuint32_t R;
    struct {
        vuint32_t OWREN:1;
        vuint32_t WLSIDE:1;
        vuint32_t MODE:1;
        vuint32_t EDGLEV:1;
        vuint32_t TRGEN:1;
        vuint32_t EDGE:1;
        vuint32_t XSTRTEN:1;
        vuint32_t NSTART:1;
        vuint32_t:1;
        vuint32_t JTRGEN:1;
        vuint32_t JEDGE:1;
        vuint32_t JSTART:1;
        vuint32_t:2;
        vuint32_t CTUEN:1;
        vuint32_t:8;
        vuint32_t ADCLKSEL:1;
        vuint32_t ABORTCHAIN:1;
        vuint32_t ABORT:1;
        vuint32_t ACK0:1;
        vuint32_t OFFREFRESH:1;
        vuint32_t OFFCANC:1;
        vuint32_t:2;
        vuint32_t PWDN:1;
    } B;
} MCR;                 /* MAIN CONFIGURATION REGISTER */

union {
    vuint32_t R;
    struct {
        vuint32_t:7;
        vuint32_t NSTART:1;
        vuint32_t JABORT:1;
        vuint32_t:2;
        vuint32_t JSTART:1;
        vuint32_t:3;
        vuint32_t CTUSTART:1;
        vuint32_t CHADDR:7;
        vuint32_t:3;
        vuint32_t ACK0:1;
        vuint32_t OFFREFRESH:1;
        vuint32_t OFFCANC:1;
        vuint32_t ADCSTATUS:3;
    } B;
} MSR;                 /* MAIN STATUS REGISTER */

int32_t ADC_reserved1[2];       /* (0x008 - 0x00F)/4 = 0x02 */

union {
    vuint32_t R;
    struct {
        vuint32_t:25;
        vuint32_t OFFCANCOVR:1;
        vuint32_t EOFFSET:1;
        vuint32_t EOCTU:1;
        vuint32_t JEOC:1;
        vuint32_t JECH:1;
        vuint32_t EOC:1;
        vuint32_t ECH:1;
    } B;
} ISR;                 /* INTERRUPT STATUS REGISTER */

union {
    vuint32_t R;
    struct {
        vuint32_t EOC31:1;
        vuint32_t EOC30:1;
        vuint32_t EOC29:1;
        vuint32_t EOC28:1;
        vuint32_t EOC27:1;
        vuint32_t EOC26:1;
        vuint32_t EOC25:1;
        vuint32_t EOC24:1;
        vuint32_t EOC23:1;
        vuint32_t EOC22:1;
        vuint32_t EOC21:1;
        vuint32_t EOC20:1;
        vuint32_t EOC19:1;
        vuint32_t EOC18:1;
        vuint32_t EOC17:1;
        vuint32_t EOC16:1;
        vuint32_t EOC15:1;
        vuint32_t EOC14:1;
        vuint32_t EOC13:1;
        vuint32_t EOC12:1;
        vuint32_t EOC11:1;
        vuint32_t EOC10:1;
        vuint32_t EOC9:1;
        vuint32_t EOC8:1;
        vuint32_t EOC7:1;
        vuint32_t EOC6:1;
        vuint32_t EOC5:1;
        vuint32_t EOC4:1;
        vuint32_t EOC3:1;
        vuint32_t EOC2:1;
        vuint32_t EOC1:1;
        vuint32_t EOC0:1;
    } B;
} CEOCFR[3];                 /* Channel Pending Register 0 */

union {
    vuint32_t R;
    struct {
        vuint32_t:25;           //One bit added
        vuint32_t MSKOFFCANCOVR:1;  //Moved up
        vuint32_t MSKEOFFSET:1;     //Moved up
        vuint32_t MSKEOCTU:1;       //New for cut 2
        vuint32_t MSKJEOC:1;
        vuint32_t MSKJECH:1;
        vuint32_t MSKEOC:1;
        vuint32_t MSKECH:1;
    } B;
} IMR;                  /* INTERRUPT MASK REGISTER */

union {
    vuint32_t R;
    struct {
        vuint32_t CIM31:1;
        vuint32_t CIM30:1;
        vuint32_t CIM29:1;
        vuint32_t CIM28:1;
        vuint32_t CIM27:1;
        vuint32_t CIM26:1;
        vuint32_t CIM25:1;
        vuint32_t CIM24:1;
        vuint32_t CIM23:1;
        vuint32_t CIM22:1;
        vuint32_t CIM21:1;
        vuint32_t CIM20:1;
        vuint32_t CIM19:1;
        vuint32_t CIM18:1;
        vuint32_t CIM17:1;
        vuint32_t CIM16:1;
        vuint32_t CIM15:1;
        vuint32_t CIM14:1;
        vuint32_t CIM13:1;
        vuint32_t CIM12:1;
        vuint32_t CIM11:1;
        vuint32_t CIM10:1;
        vuint32_t CIM9:1;
        vuint32_t CIM8:1;
        vuint32_t CIM7:1;
        vuint32_t CIM6:1;
        vuint32_t CIM5:1;
        vuint32_t CIM4:1;
        vuint32_t CIM3:1;
        vuint32_t CIM2:1;
        vuint32_t CIM1:1;
        vuint32_t CIM0:1;
    } B;
} CIMR[3];                 /* Channel Interrupt Mask Register 0 */

union {
    vuint32_t R;
    struct {
        vuint32_t:24;
        vuint32_t WDG3H:1;
        vuint32_t WDG2H:1;
        vuint32_t WDG1H:1;
        vuint32_t WDG0H:1;
        vuint32_t WDG3L:1;
        vuint32_t WDG2L:1;
        vuint32_t WDG1L:1;
        vuint32_t WDG0L:1;
    } B;
} WTISR;               /* WATCHDOG INTERRUPT THRESHOLD REGISTER was WDGTHR */

union {
    vuint32_t R;
    struct {
        vuint32_t:24;
        vuint32_t MSKWDG3H:1;
        vuint32_t MSKWDG2H:1;
        vuint32_t MSKWDG1H:1;
        vuint32_t MSKWDG0H:1;
        vuint32_t MSKWDG3L:1;
        vuint32_t MSKWDG2L:1;
        vuint32_t MSKWDG1L:1;
        vuint32_t MSKWDG0L:1;
    } B;
} WTIMR;             /* WATCHDOG INTERRUPT MASK REGISTER was IMWDGTHR */

int32_t ADC_reserved2[2];       /* (0x038 - 0x03F)/4 = 0x02 */

union {
    vuint32_t R;
    struct {
        vuint32_t:30;           //was 16
        vuint32_t DCLR:1;       //moved
        vuint32_t DMAEN:1;      //moved
    } B;
} DMAE;                 /* DMAE REGISTER */

union {
    vuint32_t R;
    struct {
        vuint32_t DMA31:1;  //was unused [16]
        vuint32_t DMA30:1;
        vuint32_t DMA29:1;
        vuint32_t DMA28:1;
        vuint32_t DMA27:1;
        vuint32_t DMA26:1;
        vuint32_t DMA25:1;
        vuint32_t DMA24:1;
        vuint32_t DMA23:1;
        vuint32_t DMA22:1;
        vuint32_t DMA21:1;
        vuint32_t DMA20:1;
        vuint32_t DMA19:1;
        vuint32_t DMA18:1;
        vuint32_t DMA17:1;
        vuint32_t DMA16:1;
        vuint32_t DMA15:1;
        vuint32_t DMA14:1;
        vuint32_t DMA13:1;
        vuint32_t DMA12:1;
        vuint32_t DMA11:1;
        vuint32_t DMA10:1;
        vuint32_t DMA9:1;
        vuint32_t DMA8:1;
        vuint32_t DMA7:1;
        vuint32_t DMA6:1;
        vuint32_t DMA5:1;
        vuint32_t DMA4:1;
        vuint32_t DMA3:1;
        vuint32_t DMA2:1;
        vuint32_t DMA1:1;
        vuint32_t DMA0:1;
    } B;
} DMAR[3];              /* DMA REGISTER  was [6] */

union {
    vuint32_t R;
    struct {
        vuint32_t:16;
        vuint32_t THREN:1;
        vuint32_t THRINV:1;
        vuint32_t THROP:1;
          vuint32_t:6;
        vuint32_t THRCH:7;
    } B;
} TRC[4];               /* ADC THRESHOLD REGISTER REGISTER */

union {
    vuint32_t R;
    struct {        //were in TRA & TRB
        vuint32_t:4;
        vuint32_t THRH:12;
        vuint32_t:4;
        vuint32_t THRL:12;
    } B;
} THRHLR[4];               /* THRESHOLD REGISTER */

union {
    vuint32_t R;
    struct {        //were in TRAALT & TRBALT
        vuint32_t:4;
        vuint32_t THRH:12;
        vuint32_t:4;
        vuint32_t THRL:12;
    } B;
} THRALT[4];            /* ADC THRESHOLD REGISTER REGISTER */

union {
    vuint32_t R;
    struct {
        vuint32_t:25;       //was 26
        vuint32_t PREVAL2:2;
        vuint32_t PREVAL1:2;
        vuint32_t PREVAL0:2;
        vuint32_t PREONCE:1;
    } B;
} PSCR;               /* PRESAMPLING CONTROL REGISTER was PREREG */

union {
    vuint32_t R;
    struct {
        vuint32_t PRES31:1; //was reserved 16
        vuint32_t PRES30:1;
        vuint32_t PRES29:1;
        vuint32_t PRES28:1;
        vuint32_t PRES27:1;
        vuint32_t PRES26:1;
        vuint32_t PRES25:1;
        vuint32_t PRES24:1;
        vuint32_t PRES23:1;
        vuint32_t PRES22:1;
        vuint32_t PRES21:1;
        vuint32_t PRES20:1;
        vuint32_t PRES19:1;
        vuint32_t PRES18:1;
        vuint32_t PRES17:1;
        vuint32_t PRES16:1;
        vuint32_t PRES15:1;
        vuint32_t PRES14:1;
        vuint32_t PRES13:1;
        vuint32_t PRES12:1;
        vuint32_t PRES11:1;
        vuint32_t PRES10:1;
        vuint32_t PRES9:1;
        vuint32_t PRES8:1;
        vuint32_t PRES7:1;
        vuint32_t PRES6:1;
        vuint32_t PRES5:1;
        vuint32_t PRES4:1;
        vuint32_t PRES3:1;
        vuint32_t PRES2:1;
        vuint32_t PRES1:1;
        vuint32_t PRES0:1;
    } B;
} PSR[3];              /* PRESAMPLING REGISTER was PRER[6]*/

int32_t ADC_reserved3[1];       /* (0x090 - 0x093)/4 = 0x01 */

union {
    vuint32_t R;
    struct {
        vuint32_t:16;
        vuint32_t INPLATCH:1;
          vuint32_t:1;
        vuint32_t OFFSHIFT:2;       //!!! This field only in CTR[0]
          vuint32_t:1;
        vuint32_t INPCMP:2;
          vuint32_t:1;
        vuint32_t INPSAMP:8;
    } B;
} CTR[3];                /* CONVERSION TIMING REGISTER was CT[3] */

int32_t ADC_reserved4[1];       /* (0x0A0 - 0x0A3)/4 = 0x01 */

union {
    vuint32_t R;
    struct {
        vuint32_t CH31:1;       //was reserved 16
        vuint32_t CH30:1;
        vuint32_t CH29:1;
        vuint32_t CH28:1;
        vuint32_t CH27:1;
        vuint32_t CH26:1;
        vuint32_t CH25:1;
        vuint32_t CH24:1;
        vuint32_t CH23:1;
        vuint32_t CH22:1;
        vuint32_t CH21:1;
        vuint32_t CH20:1;
        vuint32_t CH19:1;
        vuint32_t CH18:1;
        vuint32_t CH17:1;
        vuint32_t CH16:1;
        vuint32_t CH15:1;
        vuint32_t CH14:1;
        vuint32_t CH13:1;
        vuint32_t CH12:1;
        vuint32_t CH11:1;
        vuint32_t CH10:1;
        vuint32_t CH9:1;
        vuint32_t CH8:1;
        vuint32_t CH7:1;
        vuint32_t CH6:1;
        vuint32_t CH5:1;
        vuint32_t CH4:1;
        vuint32_t CH3:1;
        vuint32_t CH2:1;
        vuint32_t CH1:1;
        vuint32_t CH0:1;
    } B;
} NCMR[3];              /* NORMAL CONVERSION MASK REGISTER was [6] */

int32_t ADC_reserved5[1];       /* (0x0B0 - 0x0B3)/4 = 0x01 */

union {
    vuint32_t R;
    struct {
        vuint32_t CH31:1;       //was reserved 16
        vuint32_t CH30:1;
        vuint32_t CH29:1;
        vuint32_t CH28:1;
        vuint32_t CH27:1;
        vuint32_t CH26:1;
        vuint32_t CH25:1;
        vuint32_t CH24:1;
        vuint32_t CH23:1;
        vuint32_t CH22:1;
        vuint32_t CH21:1;
        vuint32_t CH20:1;
        vuint32_t CH19:1;
        vuint32_t CH18:1;
        vuint32_t CH17:1;
        vuint32_t CH16:1;
        vuint32_t CH15:1;
        vuint32_t CH14:1;
        vuint32_t CH13:1;
        vuint32_t CH12:1;
        vuint32_t CH11:1;
        vuint32_t CH10:1;
        vuint32_t CH9:1;
        vuint32_t CH8:1;
        vuint32_t CH7:1;
        vuint32_t CH6:1;
        vuint32_t CH5:1;
        vuint32_t CH4:1;
        vuint32_t CH3:1;
        vuint32_t CH2:1;
        vuint32_t CH1:1;
        vuint32_t CH0:1;
    } B;
} JCMR[3];              /* Injected CONVERSION MASK REGISTER was ICMR[6] */

union {
    vuint32_t R;
    struct {
        vuint32_t:15;
        vuint32_t OFFSETLOAD:1;     //new
        vuint32_t:8;
        vuint32_t OFFSETWORD:8;
    } B;
} OFFWR;               /* OFFSET WORD REGISTER was OFFREG*/

union {
    vuint32_t R;
    struct {
        vuint32_t:24;
        vuint32_t DSD:8;
    } B;
} DSDR;                  /* DECODE SIGNALS DELAY REGISTER was DSD */

union {
    vuint32_t R;
    struct {
        vuint32_t:24;
        vuint32_t PDED:8;   //was PDD
    } B;
} PDEDR;                  /* POWER DOWN DELAY REGISTER was PDD */

int32_t ADC_reserved6[9];       /* (0x0CC - 0x0EF)/4 = 0x09 */

union {
    vuint32_t R;
    struct {
        vuint32_t TEST_CTL:32;
    } B;
} TCTLR;                 /* Test control REGISTER */

int32_t ADC_reserved7[3];       /* (0x0F4 - 0x0FF)/4 = 0x03 */

union {
    vuint32_t R;
    struct {
        vuint32_t:12;
        vuint32_t VALID:1;
        vuint32_t OVERW:1;
        vuint32_t RESULT:2;
        vuint32_t:4;
        vuint32_t CDATA:12;
    } B;
} CDR[96];      /* Channel 0-95 Data REGISTER */

union {
    vuint32_t R;
    struct {        //were in TRA & TRB
        vuint32_t:4;
        vuint32_t THRH:12;
        vuint32_t:4;
        vuint32_t THRL:12;
    } B;
} THRHLR_2[12];               /* THRESHOLD REGISTER */

union {
    vuint32_t R;
    struct {
        vuint32_t WSEL_CH7:4;
        vuint32_t WSEL_CH6:4;
        vuint32_t WSEL_CH5:4;
        vuint32_t WSEL_CH4:4;
        vuint32_t WSEL_CH3:4;
        vuint32_t WSEL_CH2:4;
        vuint32_t WSEL_CH1:4;
        vuint32_t WSEL_CH0:4;
    } B;
} CWSEL[12];            /* CHANNEL WATCHDOG SELECTION REGISTER */

union {
    vuint32_t R;
    struct {
        vuint32_t:16;
        vuint32_t CWEN15:1;
        vuint32_t CWEN14:1;
        vuint32_t CWEN13:1;
        vuint32_t CWEN12:1;
        vuint32_t CWEN11:1;
        vuint32_t CWEN10:1;
        vuint32_t CWEN9:1;
        vuint32_t CWEN8:1;
        vuint32_t CWEN7:1;
        vuint32_t CWEN6:1;
        vuint32_t CWEN5:1;
        vuint32_t CWEN4:1;
        vuint32_t CWEN3:1;
        vuint32_t CWEN2:1;
        vuint32_t CWEN1:1;
        vuint32_t CWEN0:1;
    } B;
} CWENR[3];            /* CHANNEL WATCHDOG ENABLE REGISTER */

int32_t ADC_reserved8[3];         /* (0x2EC - 0x2EF)/4 = 0x1 */

union {
    vuint32_t R;
    struct {
        vuint32_t:16;
        vuint32_t AWOR_CH15:1;
        vuint32_t AWOR_CH14:1;
        vuint32_t AWOR_CH13:1;
        vuint32_t AWOR_CH12:1;
        vuint32_t AWOR_CH11:1;
        vuint32_t AWOR_CH10:1;
        vuint32_t AWOR_CH9:1;
        vuint32_t AWOR_CH8:1;
        vuint32_t AWOR_CH7:1;
        vuint32_t AWOR_CH6:1;
        vuint32_t AWOR_CH5:1;
        vuint32_t AWOR_CH4:1;
        vuint32_t AWOR_CH3:1;
        vuint32_t AWOR_CH2:1;
        vuint32_t AWOR_CH1:1;
        vuint32_t AWOR_CH0:1;
    } B;
} AWORR[3];            /* ANALOG WATCHDOG OUT OF RANGE REGISTER */

};
/* end of ADC_tag */

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    ADC units references
 * @{
 */
#if SPC5_HAS_ADC0
#define SPC5_ADC_0          (*(volatile struct spc5_adc *)0xFFE00000UL)
#endif

#if SPC5_HAS_ADC1
#define SPC5_ADC_1          (*(volatile struct spc5_adc *)0xFFE04000UL)
#endif
/** @} */

#endif /* _SPC5_ADC_H_ */

/** @} */

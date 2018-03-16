/*
 * Copyright (C) 2014 Fabio Utzig, http://fabioutzig.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _MK20D5_H_
#define _MK20D5_H_

/*
 * ==============================================================
 * ---------- Interrupt Number Definition -----------------------
 * ==============================================================
 */
typedef enum IRQn
{
/******  Cortex-M0 Processor Exceptions Numbers ****************/
  InitialSP_IRQn                = -15,
  InitialPC_IRQn                = -15,
  NonMaskableInt_IRQn           = -14,
  HardFault_IRQn                = -13,
  MemoryManagement_IRQn         = -12,
  BusFault_IRQn                 = -11,
  UsageFault_IRQn               = -10,
  SVCall_IRQn                   = -5,
  DebugMonitor_IRQn             = -4,
  PendSV_IRQn                   = -2,
  SysTick_IRQn                  = -1,

/******  K20x Specific Interrupt Numbers ***********************/
  DMA0_IRQn                     = 0,
  DMA1_IRQn                     = 1,
  DMA2_IRQn                     = 2,
  DMA3_IRQn                     = 3,
  DMAError_IRQn                 = 4,
  DMA_IRQn                      = 5,
  FlashMemComplete_IRQn         = 6,
  FlashMemReadCollision_IRQn    = 7,
  LowVoltageWarning_IRQn        = 8,
  LLWU_IRQn                     = 9,
  WDOG_IRQn                     = 10,
  I2C0_IRQn                     = 11,
  SPI0_IRQn                     = 12,
  I2S0_IRQn                     = 13,
  I2S1_IRQn                     = 14,
  UART0LON_IRQn                 = 15,
  UART0Status_IRQn              = 16,
  UART0Error_IRQn               = 17,
  UART1Status_IRQn              = 18,
  UART1Error_IRQn               = 19,
  UART2Status_IRQn              = 20,
  UART2Error_IRQn               = 21,
  ADC0_IRQn                     = 22,
  CMP0_IRQn                     = 23,
  CMP1_IRQn                     = 24,
  FTM0_IRQn                     = 25,
  FTM1_IRQn                     = 26,
  CMT_IRQn                      = 27,
  RTCAlarm_IRQn                 = 28,
  RTCSeconds_IRQn               = 29,
  PITChannel0_IRQn              = 30,
  PITChannel1_IRQn              = 31,
  PITChannel2_IRQn              = 32,
  PITChannel3_IRQn              = 33,
  PDB_IRQn                      = 34,
  USB_OTG_IRQn                  = 35,
  USBChargerDetect_IRQn         = 36,
  TSI_IRQn                      = 37,
  MCG_IRQn                      = 38,
  LowPowerTimer_IRQn            = 39,
  PINA_IRQn                     = 40,
  PINB_IRQn                     = 41,
  PINC_IRQn                     = 42,
  PIND_IRQn                     = 43,
  PINE_IRQn                     = 44,
  SoftInitInt_IRQn              = 45,
} IRQn_Type;

/*
 * ==========================================================================
 * ----------- Processor and Core Peripheral Section ------------------------
 * ==========================================================================
 */

/**
 * @brief K20x Interrupt Number Definition, according to the selected device
 *        in @ref Library_configuration_section
 */
#define __MPU_PRESENT             0
#define __FPU_PRESENT             0
#define __NVIC_PRIO_BITS          4
#define __Vendor_SysTickConfig    0

#include "core_cm4.h"            /* Cortex-M4 processor and core peripherals */

typedef struct
{
  __IO uint32_t SOPT1;
  __IO uint32_t SOPT1CFG;
       uint32_t RESERVED0[1023];
  __IO uint32_t SOPT2;
       uint32_t RESERVED1[1];
  __IO uint32_t SOPT4;
  __IO uint32_t SOPT5;
       uint32_t RESERVED2[1];
  __IO uint32_t SOPT7;
       uint32_t RESERVED3[2];
  __I  uint32_t SDID;
       uint32_t RESERVED4[3];
  __IO uint32_t SCGC4;
  __IO uint32_t SCGC5;
  __IO uint32_t SCGC6;
  __IO uint32_t SCGC7;
  __IO uint32_t CLKDIV1;
  __IO uint32_t CLKDIV2;
  __I  uint32_t FCFG1;
  __I  uint32_t FCFG2;
  __I  uint32_t UIDH;
  __I  uint32_t UIDMH;
  __I  uint32_t UIDML;
  __I  uint32_t UIDL;
} SIM_TypeDef;

typedef struct
{
  __IO uint8_t  PE1;
  __IO uint8_t  PE2;
  __IO uint8_t  PE3;
  __IO uint8_t  PE4;
  __IO uint8_t  ME;
  __IO uint8_t  F1;
  __IO uint8_t  F2;
  __I  uint8_t  F3;
  __IO uint8_t  FILT1;
  __IO uint8_t  FILT2;
} LLWU_TypeDef;

typedef struct
{
  __IO uint32_t PCR[32];
  __O  uint32_t GPCLR;
  __O  uint32_t GPCHR;
       uint32_t RESERVED0[6];
  __IO uint32_t ISFR;
} PORT_TypeDef;

typedef struct
{
  __IO uint8_t  C1;
  __IO uint8_t  C2;
  __IO uint8_t  C3;
  __IO uint8_t  C4;
  __IO uint8_t  C5;
  __IO uint8_t  C6;
  __I  uint8_t  S;
       uint8_t  RESERVED0[1];
  __IO uint8_t  SC;
       uint8_t  RESERVED1[1];
  __IO uint8_t  ATCVH;
  __IO uint8_t  ATCVL;
  __IO uint8_t  C7;
  __IO uint8_t  C8;
} MCG_TypeDef;

typedef struct
{
  __IO uint8_t  CR;
} OSC_TypeDef;

typedef struct  {
  uint32_t SADDR;             /* TCD Source Address */
  uint16_t SOFF;              /* TCD Signed Source Address Offset */
  uint16_t ATTR;              /* TCD Transfer Attributes */
  union {
    uint32_t NBYTES_MLNO;     /* TCD Minor Byte Count (Minor Loop Disabled) */
    uint32_t NBYTES_MLOFFNO;  /* TCD Signed Minor Loop Offset (Minor Loop Enabled and Offset Disabled) */
    uint32_t NBYTES_MLOFFYES; /* TCD Signed Minor Loop Offset (Minor Loop and Offset Enabled) */
  };
  uint32_t SLAST;             /* TCD Last Source Address Adjustment */
  uint32_t DADDR;             /* TCD Destination Address */
  uint16_t DOFF;              /* TCD Signed Destination Address Offset */
  union {
    uint16_t CITER_ELINKNO;   /* TCD Current Minor Loop Link, Major Loop Count (Channel Linking Disabled)  */
    uint16_t CITER_ELINKYES;  /* TCD Current Minor Loop Link, Major Loop Count (Channel Linking Enabled)  */
  };
  uint32_t DLASTSGA;          /* TCD Last Destination Address Adjustment/Scatter Gather Address */
  uint16_t CSR;               /* TCD Control and Status */
  union {
    uint16_t BITER_ELINKNO;   /* TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Disabled) */
    uint16_t BITER_ELINKYES;  /* TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Enabled) */
  };
} DMA_TCD_TypeDef;

/** DMA - Peripheral register structure */
typedef struct {
  __IO uint32_t CR;             /* Control Register                         */
  __IO uint32_t ES;             /* Error Status Register                    */
  __IO uint8_t RESERVED_0[4];
  __IO uint32_t ERQ;            /* Enable Request Register                  */
  __IO uint8_t RESERVED_1[4];
  __IO uint32_t EEI;            /* Enable Error Interrupt Register          */
  __IO uint8_t CEEI;            /* Clear Enable Error Interrupt Register    */
  __IO uint8_t SEEI;            /* Set Enable Error Interrupt Register      */
  __IO uint8_t CERQ;            /* Clear Enable Request Register            */
  __IO uint8_t SERQ;            /* Set Enable Request Register              */
  __IO uint8_t CDNE;            /* Clear DONE Status Bit Register           */
  __IO uint8_t SSRT;            /* Set START Bit Register                   */
  __IO uint8_t CERR;            /* Clear Error Register                     */
  __IO uint8_t CINT;            /* Clear Interrupt Request Register         */
  __IO uint8_t RESERVED_2[4];
  __IO uint32_t INT;            /* Interrupt Request Register               */
  __IO uint8_t RESERVED_3[4];
  __IO uint32_t ERR;            /* Error Register                           */
  __IO uint8_t RESERVED_4[4];
  __IO uint32_t HRS;            /* Hardware Request Status Register         */
  __IO uint8_t RESERVED_5[200];
  __IO uint8_t DCHPRI3;         /* Channel 3 Priority Register              */
  __IO uint8_t DCHPRI2;         /* Channel 2 Priority Register              */
  __IO uint8_t DCHPRI1;         /* Channel 1 Priority Register              */
  __IO uint8_t DCHPRI0;         /* Channel 0 Priority Register              */
  __IO uint8_t RESERVED_6[3836];
  DMA_TCD_TypeDef TCD[4];
} DMA_TypeDef;

typedef struct
{
  __IO uint8_t  CHCFG[4];
} DMAMUX_TypeDef;

/** PIT - Peripheral register structure */
typedef struct  {
  __IO uint32_t MCR;             /* PIT Module Control Register */
       uint8_t  RESERVED0[252];
  struct PIT_CHANNEL {
    __IO uint32_t LDVAL;         /* Timer Load Value Register */
    __IO uint32_t CVAL;          /* Current Timer Value Register */
    __IO uint32_t TCTRL;         /* Timer Control Register */
    __IO uint32_t TFLG;          /* Timer Flag Register */
  } CHANNEL[4];
} PIT_TypeDef;

typedef struct
{
  __IO uint32_t SC;         /* Status and Control */
  __IO uint32_t CNT;        /* Counter */
  __IO uint32_t MOD;        /* Modulo */
   struct FTM_Channel {
     __IO uint32_t CnSC;     /* Channel Status and Control */
     __IO uint32_t CnV;      /* Channel Value */
   } CHANNEL[8];
  __IO uint32_t CNTIN;      /* Counter Initial Value */
  __IO uint32_t STATUS;     /* Capture and Compare Status */
  __IO uint32_t MODE;       /* Features Mode Selection */
  __IO uint32_t SYNC;       /* Synchronization */
  __IO uint32_t OUTINIT;    /* Initial State for Channels Output */
  __IO uint32_t OUTMASK;    /* Output Mask */
  __IO uint32_t COMBINE;    /* Function for Linked Channels */
  __IO uint32_t DEADTIME;   /* Deadtime Insertion Control */
  __IO uint32_t EXTTRIG;    /* FTM External Trigger */
  __IO uint32_t POL;        /* Channels Polarity */
  __IO uint32_t FMS;        /* Fault Mode Status */
  __IO uint32_t FILTER;     /* Input Capture Filter Control */
  __IO uint32_t FLTCTRL;    /* Fault Control */
  __IO uint32_t QDCTRL;     /* Quadrature Decode Control and Status */
  __IO uint32_t CONF;       /* Configuration */
  __IO uint32_t FTLPOL;     /* FTM Fault Input Polarity */
  __IO uint32_t SYNCONF;    /* Synchronization Configuration */
  __IO uint32_t INVCTRL;    /* FTM Inverting Control */
  __IO uint32_t SWOCTRL;    /* FTM Software Output Control */
  __IO uint32_t PWMLOAD;    /* FTM PWM Load */
} FTM_TypeDef;

typedef struct
{
  __IO uint32_t SC1A;           // offset: 0x00
  __IO uint32_t SC1B;           // offset: 0x04
  __IO uint32_t CFG1;           // offset: 0x08
  __IO uint32_t CFG2;           // offset: 0x0C
  __I  uint32_t RA;             // offset: 0x10
  __I  uint32_t RB;             // offset: 0x14
  __IO uint32_t CV1;            // offset: 0x18
  __IO uint32_t CV2;            // offset: 0x1C
  __IO uint32_t SC2;            // offset: 0x20
  __IO uint32_t SC3;            // offset: 0x24
  __IO uint32_t OFS;            // offset: 0x28
  __IO uint32_t PG;             // offset: 0x2C
  __IO uint32_t MG;             // offset: 0x30
  __IO uint32_t CLPD;           // offset: 0x34
  __IO uint32_t CLPS;           // offset: 0x38
  __IO uint32_t CLP4;           // offset: 0x3C
  __IO uint32_t CLP3;           // offset: 0x40
  __IO uint32_t CLP2;           // offset: 0x44
  __IO uint32_t CLP1;           // offset: 0x48
  __IO uint32_t CLP0;           // offset: 0x4C
       uint32_t RESERVED0[1];   // offset: 0x50
  __IO uint32_t CLMD;           // offset: 0x54
  __IO uint32_t CLMS;           // offset: 0x58
  __IO uint32_t CLM4;           // offset: 0x5C
  __IO uint32_t CLM3;           // offset: 0x60
  __IO uint32_t CLM2;           // offset: 0x64
  __IO uint32_t CLM1;           // offset: 0x68
  __IO uint32_t CLM0;           // offset: 0x6C
} ADC_TypeDef;

typedef struct
{
  __IO uint32_t CSR;
  __IO uint32_t PSR;
  __IO uint32_t CMR;
  __I  uint32_t CNR;
} LPTMR_TypeDef;

typedef struct
{
  __IO uint32_t GENCS;
  __IO uint32_t DATA;
  __IO uint32_t TSHD;
} TSI_TypeDef;

typedef struct
{
  __IO uint32_t PDOR;
  __IO uint32_t PSOR;
  __IO uint32_t PCOR;
  __IO uint32_t PTOR;
  __IO uint32_t PDIR;
  __IO uint32_t PDDR;
} GPIO_TypeDef;

/** SPI - Peripheral register structure */
typedef struct {
  __IO uint32_t MCR;                /**< DSPI Module Configuration Register, offset: 0x0 */
       uint32_t RESERVED0[1];
  __IO uint32_t TCR;                /**< DSPI Transfer Count Register, offset: 0x8 */
  union {                           /* offset: 0xC */
    __IO uint32_t CTAR[2];          /**< DSPI Clock and Transfer Attributes Register (In Master Mode), array offset: 0xC, array step: 0x4 */
    __IO uint32_t CTAR_SLAVE[1];    /**< DSPI Clock and Transfer Attributes Register (In Slave Mode), array offset: 0xC, array step: 0x4 */
  };
       uint32_t RESERVED1[6];
  __IO uint32_t SR;                 /**< DSPI Status Register, offset: 0x2C */
  __IO uint32_t RSER;               /**< DSPI DMA/Interrupt Request Select and Enable Register, offset: 0x30 */
  union {                           /* offset: 0x34 */
    __IO uint32_t PUSHR;            /**< DSPI PUSH TX FIFO Register In Master Mode, offset: 0x34 */
    __IO uint32_t PUSHR_SLAVE;      /**< DSPI PUSH TX FIFO Register In Slave Mode, offset: 0x34 */
  };
  __I  uint32_t POPR;               /**< DSPI POP RX FIFO Register, offset: 0x38 */
  __I  uint32_t TXFR[4];            /**< DSPI Transmit FIFO Registers, offset: 0x3C */
       uint32_t RESERVED2[12];
  __I  uint32_t RXFR[4];            /**< DSPI Receive FIFO Registers, offset: 0x7C */
} SPI_TypeDef;

typedef struct
{
  __IO uint8_t  A1;
  __IO uint8_t  F;
  __IO uint8_t  C1;
  __IO uint8_t  S;
  __IO uint8_t  D;
  __IO uint8_t  C2;
  __IO uint8_t  FLT;
  __IO uint8_t  RA;
  __IO uint8_t  SMB;
  __IO uint8_t  A2;
  __IO uint8_t  SLTH;
  __IO uint8_t  SLTL;
} I2C_TypeDef;

typedef struct
{
  __IO uint8_t  BDH;
  __IO uint8_t  BDL;
  __IO uint8_t  C1;
  __IO uint8_t  C2;
  __I  uint8_t  S1;
  __IO uint8_t  S2;
  __IO uint8_t  C3;
  __IO uint8_t  D;
  __IO uint8_t  MA1;
  __IO uint8_t  MA2;
  __IO uint8_t  C4;
  __IO uint8_t  C5;
  __I  uint8_t  ED;
  __IO uint8_t  MODEM;
  __IO uint8_t  IR;
       uint8_t RESERVED0[1];
  __IO uint8_t  PFIFO;
  __IO uint8_t  CFIFO;
  __IO uint8_t  SFIFO;
  __IO uint8_t  TWFIFO;
  __I  uint8_t  TCFIFO;
  __IO uint8_t  RWFIFO;
  __I  uint8_t  RCFIFO;
       uint8_t RESERVED1[1];
  __IO uint8_t  C7816;
  __IO uint8_t  IE7816;
  __IO uint8_t  IS7816;
  union {
    __IO uint8_t  WP7816T0;
    __IO uint8_t  WP7816T1;
  };
  __IO uint8_t  WN7816;
  __IO uint8_t  WF7816;
  __IO uint8_t  ET7816;
  __IO uint8_t  TL7816;
       uint8_t RESERVED2[2];
  __IO uint8_t  C6;
  __IO uint8_t  PCTH;
  __IO uint8_t  PCTL;
  __IO uint8_t  B1T;
  __IO uint8_t  SDTH;
  __IO uint8_t  SDTL;
  __IO uint8_t  PRE;
  __IO uint8_t  TPL;
  __IO uint8_t  IE;
  __IO uint8_t  WB;
  __IO uint8_t  S3;
  __IO uint8_t  S4;
  __I  uint8_t  RPL;
  __I  uint8_t  RPREL;
  __IO uint8_t  CPW;
  __IO uint8_t  RIDT;
  __IO uint8_t  TIDT;
} UART_TypeDef;

typedef struct
{
  __IO uint8_t  LVDSC1;
  __IO uint8_t  LVDSC2;
  __IO uint8_t  REGSC;
} PMC_TypeDef;

typedef struct
{
  __IO uint16_t STCTRLH;
  __IO uint16_t STCTRLL;
  __IO uint16_t TOVALH;
  __IO uint16_t TOVALL;
  __IO uint16_t WINH;
  __IO uint16_t WINL;
  __IO uint16_t REFRESH;
  __IO uint16_t UNLOCK;
  __IO uint16_t TMROUTH;
  __IO uint16_t TMROUTL;
  __IO uint16_t RSTCNT;
  __IO uint16_t PRESC;
} WDOG_TypeDef;

typedef struct {
  __I  uint8_t  PERID;               // 0x00
       uint8_t  RESERVED0[3];
  __I  uint8_t  IDCOMP;              // 0x04
       uint8_t  RESERVED1[3];
  __I  uint8_t  REV;                 // 0x08
       uint8_t  RESERVED2[3];
  __I  uint8_t  ADDINFO;             // 0x0C
       uint8_t  RESERVED3[3];
  __IO uint8_t  OTGISTAT;            // 0x10
       uint8_t  RESERVED4[3];
  __IO uint8_t  OTGICR;              // 0x14
       uint8_t  RESERVED5[3];
  __IO uint8_t  OTGSTAT;             // 0x18
       uint8_t  RESERVED6[3];
  __IO uint8_t  OTGCTL;              // 0x1C
       uint8_t  RESERVED7[99];
  __IO uint8_t  ISTAT;               // 0x80
       uint8_t  RESERVED8[3];
  __IO uint8_t  INTEN;               // 0x84
       uint8_t  RESERVED9[3];
  __IO uint8_t  ERRSTAT;             // 0x88
       uint8_t  RESERVED10[3];
  __IO uint8_t  ERREN;               // 0x8C
       uint8_t  RESERVED11[3];
  __I  uint8_t  STAT;                // 0x90
       uint8_t  RESERVED12[3];
  __IO uint8_t  CTL;                 // 0x94
       uint8_t  RESERVED13[3];
  __IO uint8_t  ADDR;                // 0x98
       uint8_t  RESERVED14[3];
  __IO uint8_t  BDTPAGE1;            // 0x9C
       uint8_t  RESERVED15[3];
  __IO uint8_t  FRMNUML;             // 0xA0
       uint8_t  RESERVED16[3];
  __IO uint8_t  FRMNUMH;             // 0xA4
       uint8_t  RESERVED17[3];
  __IO uint8_t  TOKEN;               // 0xA8
       uint8_t  RESERVED18[3];
  __IO uint8_t  SOFTHLD;             // 0xAC
       uint8_t  RESERVED19[3];
  __IO uint8_t  BDTPAGE2;            // 0xB0
       uint8_t  RESERVED20[3];
  __IO uint8_t  BDTPAGE3;            // 0xB4
       uint8_t  RESERVED21[11];
  struct {
    __IO uint8_t  V;                 // 0xC0
         uint8_t  RESERVED[3];
  } ENDPT[16];
  __IO uint8_t  USBCTRL;             // 0x100
       uint8_t  RESERVED22[3];
  __I  uint8_t  OBSERVE;             // 0x104
       uint8_t  RESERVED23[3];
  __IO uint8_t  CONTROL;             // 0x108
       uint8_t  RESERVED24[3];
  __IO uint8_t  USBTRC0;             // 0x10C
       uint8_t  RESERVED25[7];
  __IO uint8_t  USBFRMADJUST;        // 0x114
} USBOTG_TypeDef;

/****************************************************************/
/*                  Peripheral memory map                       */
/****************************************************************/
#define DMA_BASE                ((uint32_t)0x40008000)
#define DMAMUX_BASE             ((uint32_t)0x40021000)
#define SPI0_BASE               ((uint32_t)0x4002C000)
#define PIT_BASE                ((uint32_t)0x40037000)
#define FTM0_BASE               ((uint32_t)0x40038000)
#define FTM1_BASE               ((uint32_t)0x40039000)
#define ADC0_BASE               ((uint32_t)0x4003B000)
#define LPTMR0_BASE             ((uint32_t)0x40040000)
#define TSI0_BASE               ((uint32_t)0x40045000)
#define SIM_BASE                ((uint32_t)0x40047000)
#define PORTA_BASE              ((uint32_t)0x40049000)
#define PORTB_BASE              ((uint32_t)0x4004A000)
#define PORTC_BASE              ((uint32_t)0x4004B000)
#define PORTD_BASE              ((uint32_t)0x4004C000)
#define PORTE_BASE              ((uint32_t)0x4004D000)
#define WDOG_BASE               ((uint32_t)0x40052000)
#define MCG_BASE                ((uint32_t)0x40064000)
#define OSC0_BASE               ((uint32_t)0x40065000)
#define I2C0_BASE               ((uint32_t)0x40066000)
#define UART0_BASE              ((uint32_t)0x4006A000)
#define UART1_BASE              ((uint32_t)0x4006B000)
#define UART2_BASE              ((uint32_t)0x4006C000)
#define USBOTG_BASE             ((uint32_t)0x40072000)
#define LLWU_BASE               ((uint32_t)0x4007C000)
#define PMC_BASE                ((uint32_t)0x4007D000)
#define GPIOA_BASE              ((uint32_t)0x400FF000)
#define GPIOB_BASE              ((uint32_t)0x400FF040)
#define GPIOC_BASE              ((uint32_t)0x400FF080)
#define GPIOD_BASE              ((uint32_t)0x400FF0C0)
#define GPIOE_BASE              ((uint32_t)0x400FF100)

/****************************************************************/
/*                 Peripheral declaration                       */
/****************************************************************/
#define DMA                     ((DMA_TypeDef *)     DMA_BASE)
#define DMAMUX                  ((DMAMUX_TypeDef *)  DMAMUX_BASE)
#define PIT                     ((PIT_TypeDef *)     PIT_BASE)
#define FTM0                    ((FTM_TypeDef *)     FTM0_BASE)
#define FTM1                    ((FTM_TypeDef *)     FTM1_BASE)
#define ADC0                    ((ADC_TypeDef *)     ADC0_BASE)
#define LPTMR0                  ((LPTMR_TypeDef *)   LPTMR0_BASE)
#define TSI0                    ((TSI_TypeDef *)     TSI0_BASE)
#define SIM                     ((SIM_TypeDef  *)    SIM_BASE)
#define LLWU                    ((LLWU_TypeDef  *)   LLWU_BASE)
#define PMC                     ((PMC_TypeDef  *)    PMC_BASE)
#define PORTA                   ((PORT_TypeDef  *)   PORTA_BASE)
#define PORTB                   ((PORT_TypeDef  *)   PORTB_BASE)
#define PORTC                   ((PORT_TypeDef  *)   PORTC_BASE)
#define PORTD                   ((PORT_TypeDef  *)   PORTD_BASE)
#define PORTE                   ((PORT_TypeDef  *)   PORTE_BASE)
#define WDOG                    ((WDOG_TypeDef  *)   WDOG_BASE)
#define USBOTG                  ((USBOTG_TypeDef *)  USBOTG_BASE)
#define MCG                     ((MCG_TypeDef  *)    MCG_BASE)
#define OSC                     ((OSC_TypeDef  *)    OSC0_BASE)
#define SPI0                    ((SPI_TypeDef *)     SPI0_BASE)
#define I2C0                    ((I2C_TypeDef *)     I2C0_BASE)
#define UART0                   ((UART_TypeDef *)    UART0_BASE)
#define UART1                   ((UART_TypeDef *)    UART1_BASE)
#define UART2                   ((UART_TypeDef *)    UART2_BASE)
#define GPIOA                   ((GPIO_TypeDef  *)   GPIOA_BASE)
#define GPIOB                   ((GPIO_TypeDef  *)   GPIOB_BASE)
#define GPIOC                   ((GPIO_TypeDef  *)   GPIOC_BASE)
#define GPIOD                   ((GPIO_TypeDef  *)   GPIOD_BASE)
#define GPIOE                   ((GPIO_TypeDef  *)   GPIOE_BASE)

/****************************************************************/
/*           Peripheral Registers Bits Definition               */
/****************************************************************/

/****************************************************************/
/*                                                              */
/*             System Integration Module (SIM)                  */
/*                                                              */
/****************************************************************/
/*********  Bits definition for SIM_SOPT1 register  *************/
#define SIM_SOPT1_USBREGEN           ((uint32_t)0x80000000)    /*!< USB voltage regulator enable */
#define SIM_SOPT1_USBSSTBY           ((uint32_t)0x40000000)    /*!< USB voltage regulator in standby mode during Stop, VLPS, LLS and VLLS modes */
#define SIM_SOPT1_USBVSTBY           ((uint32_t)0x20000000)    /*!< USB voltage regulator in standby mode during VLPR and VLPW modes */
#define SIM_SOPT1_OSC32KSEL_SHIFT    18                        /*!< 32K oscillator clock select (shift) */
#define SIM_SOPT1_OSC32KSEL_MASK     ((uint32_t)((uint32_t)0x3 << SIM_SOPT1_OSC32KSEL_SHIFT))                              /*!< 32K oscillator clock select (mask) */
#define SIM_SOPT1_OSC32KSEL(x)       ((uint32_t)(((uint32_t)(x) << SIM_SOPT1_OSC32KSEL_SHIFT) & SIM_SOPT1_OSC32KSEL_MASK))  /*!< 32K oscillator clock select */
#define SIM_SOPT1_RAMSIZE_SHIFT      12
#define SIM_SOPT1_RAMSIZE_MASK       ((uint32_t)((uint32_t)0xf << SIM_SOPT1_RAMSIZE_SHIFT))
#define SIM_SOPT1_RAMSIZE(x)         ((uint32_t)(((uint32_t)(x) << SIM_SOPT1_RAMSIZE_SHIFT) & SIM_SOPT1_RAMSIZE_MASK))

/*******  Bits definition for SIM_SOPT1CFG register  ************/
#define SIM_SOPT1CFG_USSWE           ((uint32_t)0x04000000)    /*!< USB voltage regulator stop standby write enable */
#define SIM_SOPT1CFG_UVSWE           ((uint32_t)0x02000000)    /*!< USB voltage regulator VLP standby write enable */
#define SIM_SOPT1CFG_URWE            ((uint32_t)0x01000000)    /*!< USB voltage regulator voltage regulator write enable */

/*******  Bits definition for SIM_SOPT2 register  ************/
#define SIM_SOPT2_USBSRC             ((uint32_t)0x00040000)    /*!< USB clock source select */
#define SIM_SOPT2_PLLFLLSEL          ((uint32_t)0x00010000)    /*!< PLL/FLL clock select */
#define SIM_SOPT2_TRACECLKSEL        ((uint32_t)0x00001000)
#define SIM_SOPT2_PTD7PAD            ((uint32_t)0x00000800)
#define SIM_SOPT2_CLKOUTSEL_SHIFT    5
#define SIM_SOPT2_CLKOUTSEL_MASK     ((uint32_t)((uint32_t)0x7 << SIM_SOPT2_CLKOUTSEL_SHIFT))
#define SIM_SOPT2_CLKOUTSEL(x)       ((uint32_t)(((uint32_t)(x) << SIM_SOPT2_CLKOUTSEL_SHIFT) & SIM_SOPT2_CLKOUTSEL_MASK))
#define SIM_SOPT2_RTCCLKOUTSEL       ((uint32_t)0x00000010)    /*!< RTC clock out select */

/*******  Bits definition for SIM_SCGC4 register  ************/
#define SIM_SCGC4_VREF               ((uint32_t)0x00100000)    /*!< VREF Clock Gate Control */
#define SIM_SCGC4_CMP                ((uint32_t)0x00080000)    /*!< Comparator Clock Gate Control */
#define SIM_SCGC4_USBOTG             ((uint32_t)0x00040000)    /*!< USB Clock Gate Control */
#define SIM_SCGC4_UART2              ((uint32_t)0x00001000)    /*!< UART2 Clock Gate Control */
#define SIM_SCGC4_UART1              ((uint32_t)0x00000800)    /*!< UART1 Clock Gate Control */
#define SIM_SCGC4_UART0              ((uint32_t)0x00000400)    /*!< UART0 Clock Gate Control */
#define SIM_SCGC4_I2C0               ((uint32_t)0x00000040)    /*!< I2C0 Clock Gate Control */
#define SIM_SCGC4_CMT                ((uint32_t)0x00000004)    /*!< CMT Clock Gate Control */
#define SIM_SCGC4_EMW                ((uint32_t)0x00000002)    /*!< EWM Clock Gate Control */

/*******  Bits definition for SIM_SCGC5 register  ************/
#define SIM_SCGC5_PORTE              ((uint32_t)0x00002000)    /*!< Port E Clock Gate Control */
#define SIM_SCGC5_PORTD              ((uint32_t)0x00001000)    /*!< Port D Clock Gate Control */
#define SIM_SCGC5_PORTC              ((uint32_t)0x00000800)    /*!< Port C Clock Gate Control */
#define SIM_SCGC5_PORTB              ((uint32_t)0x00000400)    /*!< Port B Clock Gate Control */
#define SIM_SCGC5_PORTA              ((uint32_t)0x00000200)    /*!< Port A Clock Gate Control */
#define SIM_SCGC5_TSI                ((uint32_t)0x00000020)    /*!< TSI Access Control */
#define SIM_SCGC5_LPTIMER            ((uint32_t)0x00000001)    /*!< Low Power Timer Access Control */

/*******  Bits definition for SIM_SCGC6 register  ************/
#define SIM_SCGC6_RTC                ((uint32_t)0x20000000)    /*!< RTC Access Control */
#define SIM_SCGC6_ADC0               ((uint32_t)0x08000000)    /*!< ADC0 Clock Gate Control */
#define SIM_SCGC6_FTM1               ((uint32_t)0x02000000)    /*!< FTM1 Clock Gate Control */
#define SIM_SCGC6_FTM0               ((uint32_t)0x01000000)    /*!< FTM0 Clock Gate Control */
#define SIM_SCGC6_PIT                ((uint32_t)0x00800000)    /*!< PIT Clock Gate Control */
#define SIM_SCGC6_PDB                ((uint32_t)0x00400000)    /*!< PDB Clock Gate Control */
#define SIM_SCGC6_USBDCD             ((uint32_t)0x00200000)    /*!< USB DCD Clock Gate Control */
#define SIM_SCGC6_CRC                ((uint32_t)0x00040000)    /*!< Low Power Timer Access Control */
#define SIM_SCGC6_I2S                ((uint32_t)0x00008000)    /*!< CRC Clock Gate Control */
#define SIM_SCGC6_SPI0               ((uint32_t)0x00001000)    /*!< SPI0 Clock Gate Control */
#define SIM_SCGC6_DMAMUX             ((uint32_t)0x00000002)    /*!< DMA Mux Clock Gate Control */
#define SIM_SCGC6_FTFL               ((uint32_t)0x00000001)    /*!< Flash Memory Clock Gate Control */

/*******  Bits definition for SIM_SCGC6 register  ************/
#define SIM_SCGC7_DMA                ((uint32_t)0x00000002)    /*!< DMA Clock Gate Control */

/******  Bits definition for SIM_CLKDIV1 register  ***********/
#define SIM_CLKDIV1_OUTDIV1_SHIFT    28
#define SIM_CLKDIV1_OUTDIV1_MASK     ((uint32_t)((uint32_t)0xF << SIM_CLKDIV1_OUTDIV1_SHIFT))
#define SIM_CLKDIV1_OUTDIV1(x)       ((uint32_t)(((uint32_t)(x) << SIM_CLKDIV1_OUTDIV1_SHIFT) & SIM_CLKDIV1_OUTDIV1_MASK))
#define SIM_CLKDIV1_OUTDIV2_SHIFT    24
#define SIM_CLKDIV1_OUTDIV2_MASK     ((uint32_t)((uint32_t)0xF << SIM_CLKDIV1_OUTDIV2_SHIFT))
#define SIM_CLKDIV1_OUTDIV2(x)       ((uint32_t)(((uint32_t)(x) << SIM_CLKDIV1_OUTDIV2_SHIFT) & SIM_CLKDIV1_OUTDIV2_MASK))
#define SIM_CLKDIV1_OUTDIV4_SHIFT    16
#define SIM_CLKDIV1_OUTDIV4_MASK     ((uint32_t)((uint32_t)0x7 << SIM_CLKDIV1_OUTDIV4_SHIFT))
#define SIM_CLKDIV1_OUTDIV4(x)       ((uint32_t)(((uint32_t)(x) << SIM_CLKDIV1_OUTDIV4_SHIFT) & SIM_CLKDIV1_OUTDIV4_MASK))

/******  Bits definition for SIM_CLKDIV2 register  ***********/
#define SIM_CLKDIV2_USBDIV_SHIFT     1
#define SIM_CLKDIV2_USBDIV_MASK      ((uint32_t)((uint32_t)0x7 << SIM_CLKDIV2_USBDIV_SHIFT))
#define SIM_CLKDIV2_USBDIV(x)        ((uint32_t)(((uint32_t)(x) << SIM_CLKDIV2_USBDIV_SHIFT) & SIM_CLKDIV2_USBDIV_MASK))
#define SIM_CLKDIV2_USBFRAC          ((uint32_t)0x00000001)

/****************************************************************/
/*                                                              */
/*              Low-Leakage Wakeup Unit (LLWU)                  */
/*                                                              */
/****************************************************************/
/**********  Bits definition for LLWU_PE1 register  *************/
#define LLWU_PE1_WUPE3_SHIFT        6                                                                          /*!< Wakeup Pin Enable for LLWU_P3 (shift) */
#define LLWU_PE1_WUPE3_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE1_WUPE3_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P3 (mask) */
#define LLWU_PE1_WUPE3(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE1_WUPE3_SHIFT) & LLWU_PE1_WUPE3_MASK))  /*!< Wakeup Pin Enable for LLWU_P3 */
#define LLWU_PE1_WUPE2_SHIFT        4                                                                          /*!< Wakeup Pin Enable for LLWU_P2 (shift) */
#define LLWU_PE1_WUPE2_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE1_WUPE2_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P2 (mask) */
#define LLWU_PE1_WUPE2(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE1_WUPE2_SHIFT) & LLWU_PE1_WUPE2_MASK))  /*!< Wakeup Pin Enable for LLWU_P2 */
#define LLWU_PE1_WUPE1_SHIFT        2                                                                          /*!< Wakeup Pin Enable for LLWU_P1 (shift) */
#define LLWU_PE1_WUPE1_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE1_WUPE1_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P1 (mask) */
#define LLWU_PE1_WUPE1(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE1_WUPE1_SHIFT) & LLWU_PE1_WUPE1_MASK))  /*!< Wakeup Pin Enable for LLWU_P1 */
#define LLWU_PE1_WUPE0_SHIFT        0                                                                          /*!< Wakeup Pin Enable for LLWU_P0 (shift) */
#define LLWU_PE1_WUPE0_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE1_WUPE0_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P0 (mask) */
#define LLWU_PE1_WUPE0(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE1_WUPE0_SHIFT) & LLWU_PE1_WUPE0_MASK))  /*!< Wakeup Pin Enable for LLWU_P0 */

/**********  Bits definition for LLWU_PE2 register  *************/
#define LLWU_PE2_WUPE7_SHIFT        6                                                                          /*!< Wakeup Pin Enable for LLWU_P7 (shift) */
#define LLWU_PE2_WUPE7_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE2_WUPE7_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P7 (mask) */
#define LLWU_PE2_WUPE7(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE2_WUPE7_SHIFT) & LLWU_PE2_WUPE7_MASK))  /*!< Wakeup Pin Enable for LLWU_P7 */
#define LLWU_PE2_WUPE6_SHIFT        4                                                                          /*!< Wakeup Pin Enable for LLWU_P6 (shift) */
#define LLWU_PE2_WUPE6_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE2_WUPE6_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P6 (mask) */
#define LLWU_PE2_WUPE6(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE2_WUPE6_SHIFT) & LLWU_PE2_WUPE6_MASK))  /*!< Wakeup Pin Enable for LLWU_P6 */
#define LLWU_PE2_WUPE5_SHIFT        2                                                                          /*!< Wakeup Pin Enable for LLWU_P5 (shift) */
#define LLWU_PE2_WUPE5_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE2_WUPE5_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P5 (mask) */
#define LLWU_PE2_WUPE5(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE2_WUPE5_SHIFT) & LLWU_PE2_WUPE5_MASK))  /*!< Wakeup Pin Enable for LLWU_P5 */
#define LLWU_PE2_WUPE4_SHIFT        0                                                                          /*!< Wakeup Pin Enable for LLWU_P4 (shift) */
#define LLWU_PE2_WUPE4_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE2_WUPE4_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P4 (mask) */
#define LLWU_PE2_WUPE4(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE2_WUPE4_SHIFT) & LLWU_PE2_WUPE4_MASK))  /*!< Wakeup Pin Enable for LLWU_P4 */

/**********  Bits definition for LLWU_PE3 register  *************/
#define LLWU_PE3_WUPE11_SHIFT       6                                                                            /*!< Wakeup Pin Enable for LLWU_P11 (shift) */
#define LLWU_PE3_WUPE11_MASK        ((uint8_t)((uint8_t)0x03 << LLWU_PE3_WUPE11_SHIFT))                          /*!< Wakeup Pin Enable for LLWU_P11 (mask) */
#define LLWU_PE3_WUPE11(x)          ((uint8_t)(((uint8_t)(x) << LLWU_PE3_WUPE11_SHIFT) & LLWU_PE3_WUPE11_MASK))  /*!< Wakeup Pin Enable for LLWU_P11 */
#define LLWU_PE3_WUPE10_SHIFT       4                                                                            /*!< Wakeup Pin Enable for LLWU_P10 (shift) */
#define LLWU_PE3_WUPE10_MASK        ((uint8_t)((uint8_t)0x03 << LLWU_PE3_WUPE10_SHIFT))                          /*!< Wakeup Pin Enable for LLWU_P10 (mask) */
#define LLWU_PE3_WUPE10(x)          ((uint8_t)(((uint8_t)(x) << LLWU_PE3_WUPE10_SHIFT) & LLWU_PE3_WUPE10_MASK))  /*!< Wakeup Pin Enable for LLWU_P10 */
#define LLWU_PE3_WUPE13_SHIFT        2                                                                          /*!< Wakeup Pin Enable for LLWU_P9 (shift) */
#define LLWU_PE3_WUPE13_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE3_WUPE13_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P9 (mask) */
#define LLWU_PE3_WUPE13(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE3_WUPE13_SHIFT) & LLWU_PE3_WUPE13_MASK))  /*!< Wakeup Pin Enable for LLWU_P9 */
#define LLWU_PE3_WUPE8_SHIFT        0                                                                          /*!< Wakeup Pin Enable for LLWU_P8 (shift) */
#define LLWU_PE3_WUPE8_MASK         ((uint8_t)((uint8_t)0x03 << LLWU_PE3_WUPE8_SHIFT))                         /*!< Wakeup Pin Enable for LLWU_P8 (mask) */
#define LLWU_PE3_WUPE8(x)           ((uint8_t)(((uint8_t)(x) << LLWU_PE3_WUPE8_SHIFT) & LLWU_PE3_WUPE8_MASK))  /*!< Wakeup Pin Enable for LLWU_P8 */

/**********  Bits definition for LLWU_PE4 register  *************/
#define LLWU_PE4_WUPE15_SHIFT       6                                                                            /*!< Wakeup Pin Enable for LLWU_P15 (shift) */
#define LLWU_PE4_WUPE15_MASK        ((uint8_t)((uint8_t)0x03 << LLWU_PE4_WUPE15_SHIFT))                          /*!< Wakeup Pin Enable for LLWU_P15 (mask) */
#define LLWU_PE4_WUPE15(x)          ((uint8_t)(((uint8_t)(x) << LLWU_PE4_WUPE15_SHIFT) & LLWU_PE4_WUPE15_MASK))  /*!< Wakeup Pin Enable for LLWU_P15 */
#define LLWU_PE4_WUPE14_SHIFT       4                                                                            /*!< Wakeup Pin Enable for LLWU_P14 (shift) */
#define LLWU_PE4_WUPE14_MASK        ((uint8_t)((uint8_t)0x03 << LLWU_PE4_WUPE14_SHIFT))                          /*!< Wakeup Pin Enable for LLWU_P14 (mask) */
#define LLWU_PE4_WUPE14(x)          ((uint8_t)(((uint8_t)(x) << LLWU_PE4_WUPE14_SHIFT) & LLWU_PE4_WUPE14_MASK))  /*!< Wakeup Pin Enable for LLWU_P14 */
#define LLWU_PE4_WUPE13_SHIFT       2                                                                            /*!< Wakeup Pin Enable for LLWU_P13 (shift) */
#define LLWU_PE4_WUPE13_MASK        ((uint8_t)((uint8_t)0x03 << LLWU_PE4_WUPE13_SHIFT))                          /*!< Wakeup Pin Enable for LLWU_P13 (mask) */
#define LLWU_PE4_WUPE13(x)          ((uint8_t)(((uint8_t)(x) << LLWU_PE4_WUPE13_SHIFT) & LLWU_PE4_WUPE13_MASK))  /*!< Wakeup Pin Enable for LLWU_P13 */
#define LLWU_PE4_WUPE12_SHIFT       0                                                                            /*!< Wakeup Pin Enable for LLWU_P12 (shift) */
#define LLWU_PE4_WUPE12_MASK        ((uint8_t)((uint8_t)0x03 << LLWU_PE4_WUPE12_SHIFT))                          /*!< Wakeup Pin Enable for LLWU_P12 (mask) */
#define LLWU_PE4_WUPE12(x)          ((uint8_t)(((uint8_t)(x) << LLWU_PE4_WUPE12_SHIFT) & LLWU_PE4_WUPE12_MASK))  /*!< Wakeup Pin Enable for LLWU_P12 */

/**********  Bits definition for LLWU_ME register  *************/
#define LLWU_ME_WUME7               ((uint8_t)((uint8_t)1 << 7))    /*!< Wakeup Module Enable for Module 7 */
#define LLWU_ME_WUME6               ((uint8_t)((uint8_t)1 << 6))    /*!< Wakeup Module Enable for Module 6 */
#define LLWU_ME_WUME5               ((uint8_t)((uint8_t)1 << 5))    /*!< Wakeup Module Enable for Module 5 */
#define LLWU_ME_WUME4               ((uint8_t)((uint8_t)1 << 4))    /*!< Wakeup Module Enable for Module 4 */
#define LLWU_ME_WUME3               ((uint8_t)((uint8_t)1 << 3))    /*!< Wakeup Module Enable for Module 3 */
#define LLWU_ME_WUME2               ((uint8_t)((uint8_t)1 << 2))    /*!< Wakeup Module Enable for Module 2 */
#define LLWU_ME_WUME1               ((uint8_t)((uint8_t)1 << 1))    /*!< Wakeup Module Enable for Module 1 */
#define LLWU_ME_WUME0               ((uint8_t)((uint8_t)1 << 0))    /*!< Wakeup Module Enable for Module 0 */

/**********  Bits definition for LLWU_F1 register  *************/
#define LLWU_F1_WUF7                ((uint8_t)((uint8_t)1 << 7))    /*!< Wakeup Flag for LLWU_P7 */
#define LLWU_F1_WUF6                ((uint8_t)((uint8_t)1 << 6))    /*!< Wakeup Flag for LLWU_P6 */
#define LLWU_F1_WUF5                ((uint8_t)((uint8_t)1 << 5))    /*!< Wakeup Flag for LLWU_P5 */
#define LLWU_F1_WUF4                ((uint8_t)((uint8_t)1 << 4))    /*!< Wakeup Flag for LLWU_P4 */
#define LLWU_F1_WUF3                ((uint8_t)((uint8_t)1 << 3))    /*!< Wakeup Flag for LLWU_P3 */
#define LLWU_F1_WUF2                ((uint8_t)((uint8_t)1 << 2))    /*!< Wakeup Flag for LLWU_P2 */
#define LLWU_F1_WUF1                ((uint8_t)((uint8_t)1 << 1))    /*!< Wakeup Flag for LLWU_P1 */
#define LLWU_F1_WUF0                ((uint8_t)((uint8_t)1 << 0))    /*!< Wakeup Flag for LLWU_P0 */

/**********  Bits definition for LLWU_F2 register  *************/
#define LLWU_F2_WUF15               ((uint8_t)((uint8_t)1 << 7))    /*!< Wakeup Flag for LLWU_P15 */
#define LLWU_F2_WUF14               ((uint8_t)((uint8_t)1 << 6))    /*!< Wakeup Flag for LLWU_P14 */
#define LLWU_F2_WUF13               ((uint8_t)((uint8_t)1 << 5))    /*!< Wakeup Flag for LLWU_P13 */
#define LLWU_F2_WUF12               ((uint8_t)((uint8_t)1 << 4))    /*!< Wakeup Flag for LLWU_P12 */
#define LLWU_F2_WUF11               ((uint8_t)((uint8_t)1 << 3))    /*!< Wakeup Flag for LLWU_P11 */
#define LLWU_F2_WUF10               ((uint8_t)((uint8_t)1 << 2))    /*!< Wakeup Flag for LLWU_P10 */
#define LLWU_F2_WUF9                ((uint8_t)((uint8_t)1 << 1))    /*!< Wakeup Flag for LLWU_P9 */
#define LLWU_F2_WUF8                ((uint8_t)((uint8_t)1 << 0))    /*!< Wakeup Flag for LLWU_P8 */

/**********  Bits definition for LLWU_F3 register  *************/
#define LLWU_F3_MWUF7               ((uint8_t)((uint8_t)1 << 7))    /*!< Wakeup Flag for Module 7 */
#define LLWU_F3_MWUF6               ((uint8_t)((uint8_t)1 << 6))    /*!< Wakeup Flag for Module 6 */
#define LLWU_F3_MWUF5               ((uint8_t)((uint8_t)1 << 5))    /*!< Wakeup Flag for Module 5 */
#define LLWU_F3_MWUF4               ((uint8_t)((uint8_t)1 << 4))    /*!< Wakeup Flag for Module 4 */
#define LLWU_F3_MWUF3               ((uint8_t)((uint8_t)1 << 3))    /*!< Wakeup Flag for Module 3 */
#define LLWU_F3_MWUF2               ((uint8_t)((uint8_t)1 << 2))    /*!< Wakeup Flag for Module 2 */
#define LLWU_F3_MWUF1               ((uint8_t)((uint8_t)1 << 1))    /*!< Wakeup Flag for Module 1 */
#define LLWU_F3_MWUF0               ((uint8_t)((uint8_t)1 << 0))    /*!< Wakeup Flag for Module 0 */

/**********  Bits definition for LLWU_FILT1 register  *************/
#define LLWU_FILT1_FILTF            ((uint8_t)((uint8_t)1 << 7))    /*!< Filter Detect Flag */
#define LLWU_FILT1_FILTE_SHIFT      5                                                                              /*!< Digital Filter on External Pin (shift) */
#define LLWU_FILT1_FILTE_MASK       ((uint8_t)((uint8_t)0x03 << LLWU_FILT1_FILTE_SHIFT))                           /*!< Digital Filter on External Pin (mask) */
#define LLWU_FILT1_FILTE(x)         ((uint8_t)(((uint8_t)(x) << LLWU_FILT1_FILTE_SHIFT) & LLWU_FILT1_FILTE_MASK))  /*!< Digital Filter on External Pin */
#define LLWU_FILT1_FILTE_DISABLED   LLWU_FILT1_FILTE(0)  /*!< Filter disabled */
#define LLWU_FILT1_FILTE_POSEDGE    LLWU_FILT1_FILTE(1)  /*!< Filter posedge detect enabled */
#define LLWU_FILT1_FILTE_NEGEDGE    LLWU_FILT1_FILTE(2)  /*!< Filter negedge detect enabled */
#define LLWU_FILT1_FILTE_ANYEDGE    LLWU_FILT1_FILTE(3)  /*!< Filter any edge detect enabled */
#define LLWU_FILT1_FILTSEL_SHIFT    0                                                                                  /*!< Filter Pin Select (LLWU_P0 ... LLWU_P15) (shift) */
#define LLWU_FILT1_FILTSEL_MASK     ((uint8_t)((uint8_t)0x0F << LLWU_FILT1_FILTSEL_SHIFT))                             /*!< Filter Pin Select (LLWU_P0 ... LLWU_P15) (mask) */
#define LLWU_FILT1_FILTSEL(x)       ((uint8_t)(((uint8_t)(x) << LLWU_FILT1_FILTSEL_SHIFT) & LLWU_FILT1_FILTSEL_MASK))  /*!< Filter Pin Select (LLWU_P0 ... LLWU_P15) */

/**********  Bits definition for LLWU_FILT2 register  *************/
#define LLWU_FILT2_FILTF            ((uint8_t)((uint8_t)1 << 7))    /*!< Filter Detect Flag */
#define LLWU_FILT2_FILTE_SHIFT      5                                                                              /*!< Digital Filter on External Pin (shift) */
#define LLWU_FILT2_FILTE_MASK       ((uint8_t)((uint8_t)0x03 << LLWU_FILT2_FILTE_SHIFT))                           /*!< Digital Filter on External Pin (mask) */
#define LLWU_FILT2_FILTE(x)         ((uint8_t)(((uint8_t)(x) << LLWU_FILT2_FILTE_SHIFT) & LLWU_FILT2_FILTE_MASK))  /*!< Digital Filter on External Pin */
#define LLWU_FILT2_FILTE_DISABLED   LLWU_FILT2_FILTE(0)  /*!< Filter disabled */
#define LLWU_FILT2_FILTE_POSEDGE    LLWU_FILT2_FILTE(1)  /*!< Filter posedge detect enabled */
#define LLWU_FILT2_FILTE_NEGEDGE    LLWU_FILT2_FILTE(2)  /*!< Filter negedge detect enabled */
#define LLWU_FILT2_FILTE_ANYEDGE    LLWU_FILT2_FILTE(3)  /*!< Filter any edge detect enabled */
#define LLWU_FILT2_FILTSEL_SHIFT    0                                                                                  /*!< Filter Pin Select (LLWU_P0 ... LLWU_P15) (shift) */
#define LLWU_FILT2_FILTSEL_MASK     ((uint8_t)((uint8_t)0x0F << LLWU_FILT2_FILTSEL_SHIFT))                             /*!< Filter Pin Select (LLWU_P0 ... LLWU_P15) (mask) */
#define LLWU_FILT2_FILTSEL(x)       ((uint8_t)(((uint8_t)(x) << LLWU_FILT2_FILTSEL_SHIFT) & LLWU_FILT2_FILTSEL_MASK))  /*!< Filter Pin Select (LLWU_P0 ... LLWU_P15) */

/****************************************************************/
/*                                                              */
/*           Port Control and interrupts (PORT)                 */
/*                                                              */
/****************************************************************/
/********  Bits definition for PORTx_PCRn register  *************/
#define PORTx_PCRn_ISF               ((uint32_t)0x01000000)    /*!< Interrupt Status Flag */
#define PORTx_PCRn_IRQC_SHIFT        16
#define PORTx_PCRn_IRQC_MASK         ((uint32_t)((uint32_t)0xF << PORTx_PCRn_IRQC_SHIFT))
#define PORTx_PCRn_IRQC(x)           ((uint32_t)(((uint32_t)(x) << PORTx_PCRn_IRQC_SHIFT) & PORTx_PCRn_IRQC_MASK))
#define PORTx_PCRn_LK                ((uint32_t)0x00008000)    /*!< Lock Register */
#define PORTx_PCRn_MUX_SHIFT         8                         /*!< Pin Mux Control (shift) */
#define PORTx_PCRn_MUX_MASK          ((uint32_t)((uint32_t)0x7 << PORTx_PCRn_MUX_SHIFT))   /*!< Pin Mux Control (mask) */
#define PORTx_PCRn_MUX(x)            ((uint32_t)(((uint32_t)(x) << PORTx_PCRn_MUX_SHIFT) & PORTx_PCRn_MUX_MASK))  /*!< Pin Mux Control */
#define PORTx_PCRn_DSE               ((uint32_t)0x00000040)    /*!< Drive Strength Enable */
#define PORTx_PCRn_ODE               ((uint32_t)0x00000020)    /*!< Open Drain Enable */
#define PORTx_PCRn_PFE               ((uint32_t)0x00000010)    /*!< Passive Filter Enable */
#define PORTx_PCRn_SRE               ((uint32_t)0x00000004)    /*!< Slew Rate Enable */
#define PORTx_PCRn_PE                ((uint32_t)0x00000002)    /*!< Pull Enable */
#define PORTx_PCRn_PS                ((uint32_t)0x00000001)    /*!< Pull Select */

/****************************************************************/
/*                                                              */
/*                   Oscillator (OSC)                           */
/*                                                              */
/****************************************************************/
/***********  Bits definition for OSC_CR register  **************/
#define OSC_CR_ERCLKEN               ((uint8_t)0x80)    /*!< External Reference Enable */
#define OSC_CR_EREFSTEN              ((uint8_t)0x20)    /*!< External Reference Stop Enable */
#define OSC_CR_SC2P                  ((uint8_t)0x08)    /*!< Oscillator 2pF Capacitor Load Configure */
#define OSC_CR_SC4P                  ((uint8_t)0x04)    /*!< Oscillator 4pF Capacitor Load Configure */
#define OSC_CR_SC8P                  ((uint8_t)0x02)    /*!< Oscillator 8pF Capacitor Load Configure */
#define OSC_CR_SC16P                 ((uint8_t)0x01)    /*!< Oscillator 16pF Capacitor Load Configure */

/****************************************************************/
/*                                                              */
/*                 Direct Memory Access (DMA)                   */
/*                                                              */
/****************************************************************/
/* ----------------------------------------------------------------------------
   -- DMA - Register accessor macros
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup DMA_Register_Accessor_Macros DMA - Register accessor macros
 * @{
 */


/* DMA - Register accessors */
#define DMA_CR_REG(base)                         ((base)->CR)
#define DMA_ES_REG(base)                         ((base)->ES)
#define DMA_ERQ_REG(base)                        ((base)->ERQ)
#define DMA_EEI_REG(base)                        ((base)->EEI)
#define DMA_CEEI_REG(base)                       ((base)->CEEI)
#define DMA_SEEI_REG(base)                       ((base)->SEEI)
#define DMA_CERQ_REG(base)                       ((base)->CERQ)
#define DMA_SERQ_REG(base)                       ((base)->SERQ)
#define DMA_CDNE_REG(base)                       ((base)->CDNE)
#define DMA_SSRT_REG(base)                       ((base)->SSRT)
#define DMA_CERR_REG(base)                       ((base)->CERR)
#define DMA_CINT_REG(base)                       ((base)->CINT)
#define DMA_INT_REG(base)                        ((base)->INT)
#define DMA_ERR_REG(base)                        ((base)->ERR)
#define DMA_HRS_REG(base)                        ((base)->HRS)
#define DMA_DCHPRI3_REG(base)                    ((base)->DCHPRI3)
#define DMA_DCHPRI2_REG(base)                    ((base)->DCHPRI2)
#define DMA_DCHPRI1_REG(base)                    ((base)->DCHPRI1)
#define DMA_DCHPRI0_REG(base)                    ((base)->DCHPRI0)
#define DMA_SADDR_REG(base,index)                ((base)->TCD[index].SADDR)
#define DMA_SOFF_REG(base,index)                 ((base)->TCD[index].SOFF)
#define DMA_ATTR_REG(base,index)                 ((base)->TCD[index].ATTR)
#define DMA_NBYTES_MLNO_REG(base,index)          ((base)->TCD[index].NBYTES_MLNO)
#define DMA_NBYTES_MLOFFNO_REG(base,index)       ((base)->TCD[index].NBYTES_MLOFFNO)
#define DMA_NBYTES_MLOFFYES_REG(base,index)      ((base)->TCD[index].NBYTES_MLOFFYES)
#define DMA_SLAST_REG(base,index)                ((base)->TCD[index].SLAST)
#define DMA_DADDR_REG(base,index)                ((base)->TCD[index].DADDR)
#define DMA_DOFF_REG(base,index)                 ((base)->TCD[index].DOFF)
#define DMA_CITER_ELINKNO_REG(base,index)        ((base)->TCD[index].CITER_ELINKNO)
#define DMA_CITER_ELINKYES_REG(base,index)       ((base)->TCD[index].CITER_ELINKYES)
#define DMA_DLAST_SGA_REG(base,index)            ((base)->TCD[index].DLAST_SGA)
#define DMA_CSR_REG(base,index)                  ((base)->TCD[index].CSR)
#define DMA_BITER_ELINKNO_REG(base,index)        ((base)->TCD[index].BITER_ELINKNO)
#define DMA_BITER_ELINKYES_REG(base,index)       ((base)->TCD[index].BITER_ELINKYES)

/*!
 * @}
 */ /* end of group DMA_Register_Accessor_Macros */


/* ----------------------------------------------------------------------------
   -- DMA Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup DMA_Register_Masks DMA Register Masks
 * @{
 */

/* CR Bit Fields */
#define DMA_CR_EDBG_MASK                         0x2u
#define DMA_CR_EDBG_SHIFT                        1
#define DMA_CR_ERCA_MASK                         0x4u
#define DMA_CR_ERCA_SHIFT                        2
#define DMA_CR_HOE_MASK                          0x10u
#define DMA_CR_HOE_SHIFT                         4
#define DMA_CR_HALT_MASK                         0x20u
#define DMA_CR_HALT_SHIFT                        5
#define DMA_CR_CLM_MASK                          0x40u
#define DMA_CR_CLM_SHIFT                         6
#define DMA_CR_EMLM_MASK                         0x80u
#define DMA_CR_EMLM_SHIFT                        7
#define DMA_CR_ECX_MASK                          0x10000u
#define DMA_CR_ECX_SHIFT                         16
#define DMA_CR_CX_MASK                           0x20000u
#define DMA_CR_CX_SHIFT                          17
/* ES Bit Fields */
#define DMA_ES_DBE_MASK                          0x1u
#define DMA_ES_DBE_SHIFT                         0
#define DMA_ES_SBE_MASK                          0x2u
#define DMA_ES_SBE_SHIFT                         1
#define DMA_ES_SGE_MASK                          0x4u
#define DMA_ES_SGE_SHIFT                         2
#define DMA_ES_NCE_MASK                          0x8u
#define DMA_ES_NCE_SHIFT                         3
#define DMA_ES_DOE_MASK                          0x10u
#define DMA_ES_DOE_SHIFT                         4
#define DMA_ES_DAE_MASK                          0x20u
#define DMA_ES_DAE_SHIFT                         5
#define DMA_ES_SOE_MASK                          0x40u
#define DMA_ES_SOE_SHIFT                         6
#define DMA_ES_SAE_MASK                          0x80u
#define DMA_ES_SAE_SHIFT                         7
#define DMA_ES_ERRCHN_MASK                       0xF00u
#define DMA_ES_ERRCHN_SHIFT                      8
#define DMA_ES_ERRCHN(x)                         (((uint32_t)(((uint32_t)(x))<<DMA_ES_ERRCHN_SHIFT))&DMA_ES_ERRCHN_MASK)
#define DMA_ES_CPE_MASK                          0x4000u
#define DMA_ES_CPE_SHIFT                         14
#define DMA_ES_ECX_MASK                          0x10000u
#define DMA_ES_ECX_SHIFT                         16
#define DMA_ES_VLD_MASK                          0x80000000u
#define DMA_ES_VLD_SHIFT                         31
/* ERQ Bit Fields */
#define DMA_ERQ_ERQ0_MASK                        0x1u
#define DMA_ERQ_ERQ0_SHIFT                       0
#define DMA_ERQ_ERQ1_MASK                        0x2u
#define DMA_ERQ_ERQ1_SHIFT                       1
#define DMA_ERQ_ERQ2_MASK                        0x4u
#define DMA_ERQ_ERQ2_SHIFT                       2
#define DMA_ERQ_ERQ3_MASK                        0x8u
#define DMA_ERQ_ERQ3_SHIFT                       3
/* EEI Bit Fields */
#define DMA_EEI_EEI0_MASK                        0x1u
#define DMA_EEI_EEI0_SHIFT                       0
#define DMA_EEI_EEI1_MASK                        0x2u
#define DMA_EEI_EEI1_SHIFT                       1
#define DMA_EEI_EEI2_MASK                        0x4u
#define DMA_EEI_EEI2_SHIFT                       2
#define DMA_EEI_EEI3_MASK                        0x8u
#define DMA_EEI_EEI3_SHIFT                       3
/* CEEI Bit Fields */
#define DMA_CEEI_CEEI_MASK                       0xFu
#define DMA_CEEI_CEEI_SHIFT                      0
#define DMA_CEEI_CEEI(x)                         (((uint8_t)(((uint8_t)(x))<<DMA_CEEI_CEEI_SHIFT))&DMA_CEEI_CEEI_MASK)
#define DMA_CEEI_CAEE_MASK                       0x40u
#define DMA_CEEI_CAEE_SHIFT                      6
#define DMA_CEEI_NOP_MASK                        0x80u
#define DMA_CEEI_NOP_SHIFT                       7
/* SEEI Bit Fields */
#define DMA_SEEI_SEEI_MASK                       0xFu
#define DMA_SEEI_SEEI_SHIFT                      0
#define DMA_SEEI_SEEI(x)                         (((uint8_t)(((uint8_t)(x))<<DMA_SEEI_SEEI_SHIFT))&DMA_SEEI_SEEI_MASK)
#define DMA_SEEI_SAEE_MASK                       0x40u
#define DMA_SEEI_SAEE_SHIFT                      6
#define DMA_SEEI_NOP_MASK                        0x80u
#define DMA_SEEI_NOP_SHIFT                       7
/* CERQ Bit Fields */
#define DMA_CERQ_CERQ_MASK                       0xFu
#define DMA_CERQ_CERQ_SHIFT                      0
#define DMA_CERQ_CERQ(x)                         (((uint8_t)(((uint8_t)(x))<<DMA_CERQ_CERQ_SHIFT))&DMA_CERQ_CERQ_MASK)
#define DMA_CERQ_CAER_MASK                       0x40u
#define DMA_CERQ_CAER_SHIFT                      6
#define DMA_CERQ_NOP_MASK                        0x80u
#define DMA_CERQ_NOP_SHIFT                       7
/* SERQ Bit Fields */
#define DMA_SERQ_SERQ_MASK                       0xFu
#define DMA_SERQ_SERQ_SHIFT                      0
#define DMA_SERQ_SERQ(x)                         (((uint8_t)(((uint8_t)(x))<<DMA_SERQ_SERQ_SHIFT))&DMA_SERQ_SERQ_MASK)
#define DMA_SERQ_SAER_MASK                       0x40u
#define DMA_SERQ_SAER_SHIFT                      6
#define DMA_SERQ_NOP_MASK                        0x80u
#define DMA_SERQ_NOP_SHIFT                       7
/* CDNE Bit Fields */
#define DMA_CDNE_CDNE_MASK                       0xFu
#define DMA_CDNE_CDNE_SHIFT                      0
#define DMA_CDNE_CDNE(x)                         (((uint8_t)(((uint8_t)(x))<<DMA_CDNE_CDNE_SHIFT))&DMA_CDNE_CDNE_MASK)
#define DMA_CDNE_CADN_MASK                       0x40u
#define DMA_CDNE_CADN_SHIFT                      6
#define DMA_CDNE_NOP_MASK                        0x80u
#define DMA_CDNE_NOP_SHIFT                       7
/* SSRT Bit Fields */
#define DMA_SSRT_SSRT_MASK                       0xFu
#define DMA_SSRT_SSRT_SHIFT                      0
#define DMA_SSRT_SSRT(x)                         (((uint8_t)(((uint8_t)(x))<<DMA_SSRT_SSRT_SHIFT))&DMA_SSRT_SSRT_MASK)
#define DMA_SSRT_SAST_MASK                       0x40u
#define DMA_SSRT_SAST_SHIFT                      6
#define DMA_SSRT_NOP_MASK                        0x80u
#define DMA_SSRT_NOP_SHIFT                       7
/* CERR Bit Fields */
#define DMA_CERR_CERR_MASK                       0xFu
#define DMA_CERR_CERR_SHIFT                      0
#define DMA_CERR_CERR(x)                         (((uint8_t)(((uint8_t)(x))<<DMA_CERR_CERR_SHIFT))&DMA_CERR_CERR_MASK)
#define DMA_CERR_CAEI_MASK                       0x40u
#define DMA_CERR_CAEI_SHIFT                      6
#define DMA_CERR_NOP_MASK                        0x80u
#define DMA_CERR_NOP_SHIFT                       7
/* CINT Bit Fields */
#define DMA_CINT_CINT_MASK                       0xFu
#define DMA_CINT_CINT_SHIFT                      0
#define DMA_CINT_CINT(x)                         (((uint8_t)(((uint8_t)(x))<<DMA_CINT_CINT_SHIFT))&DMA_CINT_CINT_MASK)
#define DMA_CINT_CAIR_MASK                       0x40u
#define DMA_CINT_CAIR_SHIFT                      6
#define DMA_CINT_NOP_MASK                        0x80u
#define DMA_CINT_NOP_SHIFT                       7
/* INT Bit Fields */
#define DMA_INT_INT0_MASK                        0x1u
#define DMA_INT_INT0_SHIFT                       0
#define DMA_INT_INT1_MASK                        0x2u
#define DMA_INT_INT1_SHIFT                       1
#define DMA_INT_INT2_MASK                        0x4u
#define DMA_INT_INT2_SHIFT                       2
#define DMA_INT_INT3_MASK                        0x8u
#define DMA_INT_INT3_SHIFT                       3
/* ERR Bit Fields */
#define DMA_ERR_ERR0_MASK                        0x1u
#define DMA_ERR_ERR0_SHIFT                       0
#define DMA_ERR_ERR1_MASK                        0x2u
#define DMA_ERR_ERR1_SHIFT                       1
#define DMA_ERR_ERR2_MASK                        0x4u
#define DMA_ERR_ERR2_SHIFT                       2
#define DMA_ERR_ERR3_MASK                        0x8u
#define DMA_ERR_ERR3_SHIFT                       3
/* HRS Bit Fields */
#define DMA_HRS_HRS0_MASK                        0x1u
#define DMA_HRS_HRS0_SHIFT                       0
#define DMA_HRS_HRS1_MASK                        0x2u
#define DMA_HRS_HRS1_SHIFT                       1
#define DMA_HRS_HRS2_MASK                        0x4u
#define DMA_HRS_HRS2_SHIFT                       2
#define DMA_HRS_HRS3_MASK                        0x8u
#define DMA_HRS_HRS3_SHIFT                       3
/* DCHPRI3 Bit Fields */
#define DMA_DCHPRI3_CHPRI_MASK                   0xFu
#define DMA_DCHPRI3_CHPRI_SHIFT                  0
#define DMA_DCHPRI3_CHPRI(x)                     (((uint8_t)(((uint8_t)(x))<<DMA_DCHPRI3_CHPRI_SHIFT))&DMA_DCHPRI3_CHPRI_MASK)
#define DMA_DCHPRI3_DPA_MASK                     0x40u
#define DMA_DCHPRI3_DPA_SHIFT                    6
#define DMA_DCHPRI3_ECP_MASK                     0x80u
#define DMA_DCHPRI3_ECP_SHIFT                    7
/* DCHPRI2 Bit Fields */
#define DMA_DCHPRI2_CHPRI_MASK                   0xFu
#define DMA_DCHPRI2_CHPRI_SHIFT                  0
#define DMA_DCHPRI2_CHPRI(x)                     (((uint8_t)(((uint8_t)(x))<<DMA_DCHPRI2_CHPRI_SHIFT))&DMA_DCHPRI2_CHPRI_MASK)
#define DMA_DCHPRI2_DPA_MASK                     0x40u
#define DMA_DCHPRI2_DPA_SHIFT                    6
#define DMA_DCHPRI2_ECP_MASK                     0x80u
#define DMA_DCHPRI2_ECP_SHIFT                    7
/* DCHPRI1 Bit Fields */
#define DMA_DCHPRI1_CHPRI_MASK                   0xFu
#define DMA_DCHPRI1_CHPRI_SHIFT                  0
#define DMA_DCHPRI1_CHPRI(x)                     (((uint8_t)(((uint8_t)(x))<<DMA_DCHPRI1_CHPRI_SHIFT))&DMA_DCHPRI1_CHPRI_MASK)
#define DMA_DCHPRI1_DPA_MASK                     0x40u
#define DMA_DCHPRI1_DPA_SHIFT                    6
#define DMA_DCHPRI1_ECP_MASK                     0x80u
#define DMA_DCHPRI1_ECP_SHIFT                    7
/* DCHPRI0 Bit Fields */
#define DMA_DCHPRI0_CHPRI_MASK                   0xFu
#define DMA_DCHPRI0_CHPRI_SHIFT                  0
#define DMA_DCHPRI0_CHPRI(x)                     (((uint8_t)(((uint8_t)(x))<<DMA_DCHPRI0_CHPRI_SHIFT))&DMA_DCHPRI0_CHPRI_MASK)
#define DMA_DCHPRI0_DPA_MASK                     0x40u
#define DMA_DCHPRI0_DPA_SHIFT                    6
#define DMA_DCHPRI0_ECP_MASK                     0x80u
#define DMA_DCHPRI0_ECP_SHIFT                    7
/* SADDR Bit Fields */
#define DMA_SADDR_SADDR_MASK                     0xFFFFFFFFu
#define DMA_SADDR_SADDR_SHIFT                    0
#define DMA_SADDR_SADDR(x)                       (((uint32_t)(((uint32_t)(x))<<DMA_SADDR_SADDR_SHIFT))&DMA_SADDR_SADDR_MASK)
/* SOFF Bit Fields */
#define DMA_SOFF_SOFF_MASK                       0xFFFFu
#define DMA_SOFF_SOFF_SHIFT                      0
#define DMA_SOFF_SOFF(x)                         (((uint16_t)(((uint16_t)(x))<<DMA_SOFF_SOFF_SHIFT))&DMA_SOFF_SOFF_MASK)
/* ATTR Bit Fields */
#define DMA_ATTR_DSIZE_MASK                      0x7u
#define DMA_ATTR_DSIZE_SHIFT                     0
#define DMA_ATTR_DSIZE(x)                        (((uint16_t)(((uint16_t)(x))<<DMA_ATTR_DSIZE_SHIFT))&DMA_ATTR_DSIZE_MASK)
#define DMA_ATTR_DMOD_MASK                       0xF8u
#define DMA_ATTR_DMOD_SHIFT                      3
#define DMA_ATTR_DMOD(x)                         (((uint16_t)(((uint16_t)(x))<<DMA_ATTR_DMOD_SHIFT))&DMA_ATTR_DMOD_MASK)
#define DMA_ATTR_SSIZE_MASK                      0x700u
#define DMA_ATTR_SSIZE_SHIFT                     8
#define DMA_ATTR_SSIZE(x)                        (((uint16_t)(((uint16_t)(x))<<DMA_ATTR_SSIZE_SHIFT))&DMA_ATTR_SSIZE_MASK)
#define DMA_ATTR_SMOD_MASK                       0xF800u
#define DMA_ATTR_SMOD_SHIFT                      11
#define DMA_ATTR_SMOD(x)                         (((uint16_t)(((uint16_t)(x))<<DMA_ATTR_SMOD_SHIFT))&DMA_ATTR_SMOD_MASK)
/* NBYTES_MLNO Bit Fields */
#define DMA_NBYTES_MLNO_NBYTES_MASK              0xFFFFFFFFu
#define DMA_NBYTES_MLNO_NBYTES_SHIFT             0
#define DMA_NBYTES_MLNO_NBYTES(x)                (((uint32_t)(((uint32_t)(x))<<DMA_NBYTES_MLNO_NBYTES_SHIFT))&DMA_NBYTES_MLNO_NBYTES_MASK)
/* NBYTES_MLOFFNO Bit Fields */
#define DMA_NBYTES_MLOFFNO_NBYTES_MASK           0x3FFFFFFFu
#define DMA_NBYTES_MLOFFNO_NBYTES_SHIFT          0
#define DMA_NBYTES_MLOFFNO_NBYTES(x)             (((uint32_t)(((uint32_t)(x))<<DMA_NBYTES_MLOFFNO_NBYTES_SHIFT))&DMA_NBYTES_MLOFFNO_NBYTES_MASK)
#define DMA_NBYTES_MLOFFNO_DMLOE_MASK            0x40000000u
#define DMA_NBYTES_MLOFFNO_DMLOE_SHIFT           30
#define DMA_NBYTES_MLOFFNO_SMLOE_MASK            0x80000000u
#define DMA_NBYTES_MLOFFNO_SMLOE_SHIFT           31
/* NBYTES_MLOFFYES Bit Fields */
#define DMA_NBYTES_MLOFFYES_NBYTES_MASK          0x3FFu
#define DMA_NBYTES_MLOFFYES_NBYTES_SHIFT         0
#define DMA_NBYTES_MLOFFYES_NBYTES(x)            (((uint32_t)(((uint32_t)(x))<<DMA_NBYTES_MLOFFYES_NBYTES_SHIFT))&DMA_NBYTES_MLOFFYES_NBYTES_MASK)
#define DMA_NBYTES_MLOFFYES_MLOFF_MASK           0x3FFFFC00u
#define DMA_NBYTES_MLOFFYES_MLOFF_SHIFT          10
#define DMA_NBYTES_MLOFFYES_MLOFF(x)             (((uint32_t)(((uint32_t)(x))<<DMA_NBYTES_MLOFFYES_MLOFF_SHIFT))&DMA_NBYTES_MLOFFYES_MLOFF_MASK)
#define DMA_NBYTES_MLOFFYES_DMLOE_MASK           0x40000000u
#define DMA_NBYTES_MLOFFYES_DMLOE_SHIFT          30
#define DMA_NBYTES_MLOFFYES_SMLOE_MASK           0x80000000u
#define DMA_NBYTES_MLOFFYES_SMLOE_SHIFT          31
/* SLAST Bit Fields */
#define DMA_SLAST_SLAST_MASK                     0xFFFFFFFFu
#define DMA_SLAST_SLAST_SHIFT                    0
#define DMA_SLAST_SLAST(x)                       (((uint32_t)(((uint32_t)(x))<<DMA_SLAST_SLAST_SHIFT))&DMA_SLAST_SLAST_MASK)
/* DADDR Bit Fields */
#define DMA_DADDR_DADDR_MASK                     0xFFFFFFFFu
#define DMA_DADDR_DADDR_SHIFT                    0
#define DMA_DADDR_DADDR(x)                       (((uint32_t)(((uint32_t)(x))<<DMA_DADDR_DADDR_SHIFT))&DMA_DADDR_DADDR_MASK)
/* DOFF Bit Fields */
#define DMA_DOFF_DOFF_MASK                       0xFFFFu
#define DMA_DOFF_DOFF_SHIFT                      0
#define DMA_DOFF_DOFF(x)                         (((uint16_t)(((uint16_t)(x))<<DMA_DOFF_DOFF_SHIFT))&DMA_DOFF_DOFF_MASK)
/* CITER_ELINKNO Bit Fields */
#define DMA_CITER_ELINKNO_CITER_MASK             0x7FFFu
#define DMA_CITER_ELINKNO_CITER_SHIFT            0
#define DMA_CITER_ELINKNO_CITER(x)               (((uint16_t)(((uint16_t)(x))<<DMA_CITER_ELINKNO_CITER_SHIFT))&DMA_CITER_ELINKNO_CITER_MASK)
#define DMA_CITER_ELINKNO_ELINK_MASK             0x8000u
#define DMA_CITER_ELINKNO_ELINK_SHIFT            15
/* CITER_ELINKYES Bit Fields */
#define DMA_CITER_ELINKYES_CITER_MASK            0x1FFu
#define DMA_CITER_ELINKYES_CITER_SHIFT           0
#define DMA_CITER_ELINKYES_CITER(x)              (((uint16_t)(((uint16_t)(x))<<DMA_CITER_ELINKYES_CITER_SHIFT))&DMA_CITER_ELINKYES_CITER_MASK)
#define DMA_CITER_ELINKYES_LINKCH_MASK           0x1E00u
#define DMA_CITER_ELINKYES_LINKCH_SHIFT          9
#define DMA_CITER_ELINKYES_LINKCH(x)             (((uint16_t)(((uint16_t)(x))<<DMA_CITER_ELINKYES_LINKCH_SHIFT))&DMA_CITER_ELINKYES_LINKCH_MASK)
#define DMA_CITER_ELINKYES_ELINK_MASK            0x8000u
#define DMA_CITER_ELINKYES_ELINK_SHIFT           15
/* DLAST_SGA Bit Fields */
#define DMA_DLAST_SGA_DLASTSGA_MASK              0xFFFFFFFFu
#define DMA_DLAST_SGA_DLASTSGA_SHIFT             0
#define DMA_DLAST_SGA_DLASTSGA(x)                (((uint32_t)(((uint32_t)(x))<<DMA_DLAST_SGA_DLASTSGA_SHIFT))&DMA_DLAST_SGA_DLASTSGA_MASK)
/* CSR Bit Fields */
#define DMA_CSR_START_MASK                       0x1u
#define DMA_CSR_START_SHIFT                      0
#define DMA_CSR_INTMAJOR_MASK                    0x2u
#define DMA_CSR_INTMAJOR_SHIFT                   1
#define DMA_CSR_INTHALF_MASK                     0x4u
#define DMA_CSR_INTHALF_SHIFT                    2
#define DMA_CSR_DREQ_MASK                        0x8u
#define DMA_CSR_DREQ_SHIFT                       3
#define DMA_CSR_ESG_MASK                         0x10u
#define DMA_CSR_ESG_SHIFT                        4
#define DMA_CSR_MAJORELINK_MASK                  0x20u
#define DMA_CSR_MAJORELINK_SHIFT                 5
#define DMA_CSR_ACTIVE_MASK                      0x40u
#define DMA_CSR_ACTIVE_SHIFT                     6
#define DMA_CSR_DONE_MASK                        0x80u
#define DMA_CSR_DONE_SHIFT                       7
#define DMA_CSR_MAJORLINKCH_MASK                 0xF00u
#define DMA_CSR_MAJORLINKCH_SHIFT                8
#define DMA_CSR_MAJORLINKCH(x)                   (((uint16_t)(((uint16_t)(x))<<DMA_CSR_MAJORLINKCH_SHIFT))&DMA_CSR_MAJORLINKCH_MASK)
#define DMA_CSR_BWC_MASK                         0xC000u
#define DMA_CSR_BWC_SHIFT                        14
#define DMA_CSR_BWC(x)                           (((uint16_t)(((uint16_t)(x))<<DMA_CSR_BWC_SHIFT))&DMA_CSR_BWC_MASK)
/* BITER_ELINKNO Bit Fields */
#define DMA_BITER_ELINKNO_BITER_MASK             0x7FFFu
#define DMA_BITER_ELINKNO_BITER_SHIFT            0
#define DMA_BITER_ELINKNO_BITER(x)               (((uint16_t)(((uint16_t)(x))<<DMA_BITER_ELINKNO_BITER_SHIFT))&DMA_BITER_ELINKNO_BITER_MASK)
#define DMA_BITER_ELINKNO_ELINK_MASK             0x8000u
#define DMA_BITER_ELINKNO_ELINK_SHIFT            15
/* BITER_ELINKYES Bit Fields */
#define DMA_BITER_ELINKYES_BITER_MASK            0x1FFu
#define DMA_BITER_ELINKYES_BITER_SHIFT           0
#define DMA_BITER_ELINKYES_BITER(x)              (((uint16_t)(((uint16_t)(x))<<DMA_BITER_ELINKYES_BITER_SHIFT))&DMA_BITER_ELINKYES_BITER_MASK)
#define DMA_BITER_ELINKYES_LINKCH_MASK           0x1E00u
#define DMA_BITER_ELINKYES_LINKCH_SHIFT          9
#define DMA_BITER_ELINKYES_LINKCH(x)             (((uint16_t)(((uint16_t)(x))<<DMA_BITER_ELINKYES_LINKCH_SHIFT))&DMA_BITER_ELINKYES_LINKCH_MASK)
#define DMA_BITER_ELINKYES_ELINK_MASK            0x8000u
#define DMA_BITER_ELINKYES_ELINK_SHIFT           15

/*!
 * @}
 */ /* end of group DMA_Register_Masks */


/* DMA - Peripheral instance base addresses */
/** Peripheral DMA base pointer */
#define DMA_BASE_PTR                             ((DMA_MemMapPtr)0x40008000u)
/** Array initializer of DMA peripheral base pointers */
#define DMA_BASE_PTRS                            { DMA_BASE_PTR }

/* ----------------------------------------------------------------------------
   -- DMA - Register accessor macros
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup DMA_Register_Accessor_Macros DMA - Register accessor macros
 * @{
 */


/* DMA - Register instance definitions */
/* DMA */
#define DMA_CR                                   DMA_CR_REG(DMA_BASE_PTR)
#define DMA_ES                                   DMA_ES_REG(DMA_BASE_PTR)
#define DMA_ERQ                                  DMA_ERQ_REG(DMA_BASE_PTR)
#define DMA_EEI                                  DMA_EEI_REG(DMA_BASE_PTR)
#define DMA_CEEI                                 DMA_CEEI_REG(DMA_BASE_PTR)
#define DMA_SEEI                                 DMA_SEEI_REG(DMA_BASE_PTR)
#define DMA_CERQ                                 DMA_CERQ_REG(DMA_BASE_PTR)
#define DMA_SERQ                                 DMA_SERQ_REG(DMA_BASE_PTR)
#define DMA_CDNE                                 DMA_CDNE_REG(DMA_BASE_PTR)
#define DMA_SSRT                                 DMA_SSRT_REG(DMA_BASE_PTR)
#define DMA_CERR                                 DMA_CERR_REG(DMA_BASE_PTR)
#define DMA_CINT                                 DMA_CINT_REG(DMA_BASE_PTR)
#define DMA_INT                                  DMA_INT_REG(DMA_BASE_PTR)
#define DMA_ERR                                  DMA_ERR_REG(DMA_BASE_PTR)
#define DMA_HRS                                  DMA_HRS_REG(DMA_BASE_PTR)
#define DMA_DCHPRI3                              DMA_DCHPRI3_REG(DMA_BASE_PTR)
#define DMA_DCHPRI2                              DMA_DCHPRI2_REG(DMA_BASE_PTR)
#define DMA_DCHPRI1                              DMA_DCHPRI1_REG(DMA_BASE_PTR)
#define DMA_DCHPRI0                              DMA_DCHPRI0_REG(DMA_BASE_PTR)
#define DMA_TCD0_SADDR                           DMA_SADDR_REG(DMA_BASE_PTR,0)
#define DMA_TCD0_SOFF                            DMA_SOFF_REG(DMA_BASE_PTR,0)
#define DMA_TCD0_ATTR                            DMA_ATTR_REG(DMA_BASE_PTR,0)
#define DMA_TCD0_NBYTES_MLNO                     DMA_NBYTES_MLNO_REG(DMA_BASE_PTR,0)
#define DMA_TCD0_NBYTES_MLOFFNO                  DMA_NBYTES_MLOFFNO_REG(DMA_BASE_PTR,0)
#define DMA_TCD0_NBYTES_MLOFFYES                 DMA_NBYTES_MLOFFYES_REG(DMA_BASE_PTR,0)
#define DMA_TCD0_SLAST                           DMA_SLAST_REG(DMA_BASE_PTR,0)
#define DMA_TCD0_DADDR                           DMA_DADDR_REG(DMA_BASE_PTR,0)
#define DMA_TCD0_DOFF                            DMA_DOFF_REG(DMA_BASE_PTR,0)
#define DMA_TCD0_CITER_ELINKNO                   DMA_CITER_ELINKNO_REG(DMA_BASE_PTR,0)
#define DMA_TCD0_CITER_ELINKYES                  DMA_CITER_ELINKYES_REG(DMA_BASE_PTR,0)
#define DMA_TCD0_DLASTSGA                        DMA_DLAST_SGA_REG(DMA_BASE_PTR,0)
#define DMA_TCD0_CSR                             DMA_CSR_REG(DMA_BASE_PTR,0)
#define DMA_TCD0_BITER_ELINKNO                   DMA_BITER_ELINKNO_REG(DMA_BASE_PTR,0)
#define DMA_TCD0_BITER_ELINKYES                  DMA_BITER_ELINKYES_REG(DMA_BASE_PTR,0)
#define DMA_TCD1_SADDR                           DMA_SADDR_REG(DMA_BASE_PTR,1)
#define DMA_TCD1_SOFF                            DMA_SOFF_REG(DMA_BASE_PTR,1)
#define DMA_TCD1_ATTR                            DMA_ATTR_REG(DMA_BASE_PTR,1)
#define DMA_TCD1_NBYTES_MLNO                     DMA_NBYTES_MLNO_REG(DMA_BASE_PTR,1)
#define DMA_TCD1_NBYTES_MLOFFNO                  DMA_NBYTES_MLOFFNO_REG(DMA_BASE_PTR,1)
#define DMA_TCD1_NBYTES_MLOFFYES                 DMA_NBYTES_MLOFFYES_REG(DMA_BASE_PTR,1)
#define DMA_TCD1_SLAST                           DMA_SLAST_REG(DMA_BASE_PTR,1)
#define DMA_TCD1_DADDR                           DMA_DADDR_REG(DMA_BASE_PTR,1)
#define DMA_TCD1_DOFF                            DMA_DOFF_REG(DMA_BASE_PTR,1)
#define DMA_TCD1_CITER_ELINKNO                   DMA_CITER_ELINKNO_REG(DMA_BASE_PTR,1)
#define DMA_TCD1_CITER_ELINKYES                  DMA_CITER_ELINKYES_REG(DMA_BASE_PTR,1)
#define DMA_TCD1_DLASTSGA                        DMA_DLAST_SGA_REG(DMA_BASE_PTR,1)
#define DMA_TCD1_CSR                             DMA_CSR_REG(DMA_BASE_PTR,1)
#define DMA_TCD1_BITER_ELINKNO                   DMA_BITER_ELINKNO_REG(DMA_BASE_PTR,1)
#define DMA_TCD1_BITER_ELINKYES                  DMA_BITER_ELINKYES_REG(DMA_BASE_PTR,1)
#define DMA_TCD2_SADDR                           DMA_SADDR_REG(DMA_BASE_PTR,2)
#define DMA_TCD2_SOFF                            DMA_SOFF_REG(DMA_BASE_PTR,2)
#define DMA_TCD2_ATTR                            DMA_ATTR_REG(DMA_BASE_PTR,2)
#define DMA_TCD2_NBYTES_MLNO                     DMA_NBYTES_MLNO_REG(DMA_BASE_PTR,2)
#define DMA_TCD2_NBYTES_MLOFFNO                  DMA_NBYTES_MLOFFNO_REG(DMA_BASE_PTR,2)
#define DMA_TCD2_NBYTES_MLOFFYES                 DMA_NBYTES_MLOFFYES_REG(DMA_BASE_PTR,2)
#define DMA_TCD2_SLAST                           DMA_SLAST_REG(DMA_BASE_PTR,2)
#define DMA_TCD2_DADDR                           DMA_DADDR_REG(DMA_BASE_PTR,2)
#define DMA_TCD2_DOFF                            DMA_DOFF_REG(DMA_BASE_PTR,2)
#define DMA_TCD2_CITER_ELINKNO                   DMA_CITER_ELINKNO_REG(DMA_BASE_PTR,2)
#define DMA_TCD2_CITER_ELINKYES                  DMA_CITER_ELINKYES_REG(DMA_BASE_PTR,2)
#define DMA_TCD2_DLASTSGA                        DMA_DLAST_SGA_REG(DMA_BASE_PTR,2)
#define DMA_TCD2_CSR                             DMA_CSR_REG(DMA_BASE_PTR,2)
#define DMA_TCD2_BITER_ELINKNO                   DMA_BITER_ELINKNO_REG(DMA_BASE_PTR,2)
#define DMA_TCD2_BITER_ELINKYES                  DMA_BITER_ELINKYES_REG(DMA_BASE_PTR,2)
#define DMA_TCD3_SADDR                           DMA_SADDR_REG(DMA_BASE_PTR,3)
#define DMA_TCD3_SOFF                            DMA_SOFF_REG(DMA_BASE_PTR,3)
#define DMA_TCD3_ATTR                            DMA_ATTR_REG(DMA_BASE_PTR,3)
#define DMA_TCD3_NBYTES_MLNO                     DMA_NBYTES_MLNO_REG(DMA_BASE_PTR,3)
#define DMA_TCD3_NBYTES_MLOFFNO                  DMA_NBYTES_MLOFFNO_REG(DMA_BASE_PTR,3)
#define DMA_TCD3_NBYTES_MLOFFYES                 DMA_NBYTES_MLOFFYES_REG(DMA_BASE_PTR,3)
#define DMA_TCD3_SLAST                           DMA_SLAST_REG(DMA_BASE_PTR,3)
#define DMA_TCD3_DADDR                           DMA_DADDR_REG(DMA_BASE_PTR,3)
#define DMA_TCD3_DOFF                            DMA_DOFF_REG(DMA_BASE_PTR,3)
#define DMA_TCD3_CITER_ELINKNO                   DMA_CITER_ELINKNO_REG(DMA_BASE_PTR,3)
#define DMA_TCD3_CITER_ELINKYES                  DMA_CITER_ELINKYES_REG(DMA_BASE_PTR,3)
#define DMA_TCD3_DLASTSGA                        DMA_DLAST_SGA_REG(DMA_BASE_PTR,3)
#define DMA_TCD3_CSR                             DMA_CSR_REG(DMA_BASE_PTR,3)
#define DMA_TCD3_BITER_ELINKNO                   DMA_BITER_ELINKNO_REG(DMA_BASE_PTR,3)
#define DMA_TCD3_BITER_ELINKYES                  DMA_BITER_ELINKYES_REG(DMA_BASE_PTR,3)

/* DMA - Register array accessors */
#define DMA_SADDR(index)                         DMA_SADDR_REG(DMA_BASE_PTR,index)
#define DMA_SOFF(index)                          DMA_SOFF_REG(DMA_BASE_PTR,index)
#define DMA_ATTR(index)                          DMA_ATTR_REG(DMA_BASE_PTR,index)
#define DMA_NBYTES_MLNO(index)                   DMA_NBYTES_MLNO_REG(DMA_BASE_PTR,index)
#define DMA_NBYTES_MLOFFNO(index)                DMA_NBYTES_MLOFFNO_REG(DMA_BASE_PTR,index)
#define DMA_NBYTES_MLOFFYES(index)               DMA_NBYTES_MLOFFYES_REG(DMA_BASE_PTR,index)
#define DMA_SLAST(index)                         DMA_SLAST_REG(DMA_BASE_PTR,index)
#define DMA_DADDR(index)                         DMA_DADDR_REG(DMA_BASE_PTR,index)
#define DMA_DOFF(index)                          DMA_DOFF_REG(DMA_BASE_PTR,index)
#define DMA_CITER_ELINKNO(index)                 DMA_CITER_ELINKNO_REG(DMA_BASE_PTR,index)
#define DMA_CITER_ELINKYES(index)                DMA_CITER_ELINKYES_REG(DMA_BASE_PTR,index)
#define DMA_DLAST_SGA(index)                     DMA_DLAST_SGA_REG(DMA_BASE_PTR,index)
#define DMA_CSR(index)                           DMA_CSR_REG(DMA_BASE_PTR,index)
#define DMA_BITER_ELINKNO(index)                 DMA_BITER_ELINKNO_REG(DMA_BASE_PTR,index)
#define DMA_BITER_ELINKYES(index)                DMA_BITER_ELINKYES_REG(DMA_BASE_PTR,index)

/****************************************************************/
/*                                                              */
/*         Direct Memory Access Multiplexer (DMAMUX)            */
/*                                                              */
/****************************************************************/
/********  Bits definition for DMAMUX_CHCFGn register  **********/
#define DMAMUX_CHCFGn_ENBL           ((uint8_t)((uint8_t)1 << 7))  /*!< DMA Channel Enable */
#define DMAMUX_CHCFGn_TRIG           ((uint8_t)((uint8_t)1 << 6))  /*!< DMA Channel Trigger Enable */
#define DMAMUX_CHCFGn_SOURCE_SHIFT   0                                                                                      /*!< DMA Channel Source (Slot) (shift) */
#define DMAMUX_CHCFGn_SOURCE_MASK    ((uint8_t)((uint8_t)0x3F << DMAMUX_CHCFGn_SOURCE_SHIFT))                               /*!< DMA Channel Source (Slot) (mask) */
#define DMAMUX_CHCFGn_SOURCE(x)      ((uint8_t)(((uint8_t)(x) << DMAMUX_CHCFGn_SOURCE_SHIFT) & DMAMUX_CHCFGn_SOURCE_MASK))  /*!< DMA Channel Source (Slot) */

/****************************************************************/
/*                                                              */
/*                   FlexTimer Module (FTM)                     */
/*                                                              */
/****************************************************************/

/* SC Bit Fields */
#define FTM_SC_PS_MASK                           0x7u
#define FTM_SC_PS_SHIFT                          0
#define FTM_SC_PS(x)                             (((uint32_t)(((uint32_t)(x))<<FTM_SC_PS_SHIFT))&FTM_SC_PS_MASK)
#define FTM_SC_CLKS_MASK                         0x18u
#define FTM_SC_CLKS_SHIFT                        3
#define FTM_SC_CLKS(x)                           (((uint32_t)(((uint32_t)(x))<<FTM_SC_CLKS_SHIFT))&FTM_SC_CLKS_MASK)
#define FTM_SC_CPWMS                             0x20u
#define FTM_SC_TOIE                              0x40u
#define FTM_SC_TOF                               0x80u
/* CNT Bit Fields */
#define FTM_CNT_COUNT_MASK                       0xFFFFu
#define FTM_CNT_COUNT_SHIFT                      0
#define FTM_CNT_COUNT(x)                         (((uint32_t)(((uint32_t)(x))<<FTM_CNT_COUNT_SHIFT))&FTM_CNT_COUNT_MASK)
/* MOD Bit Fields */
#define FTM_MOD_MOD_MASK                         0xFFFFu
#define FTM_MOD_MOD_SHIFT                        0
#define FTM_MOD_MOD(x)                           (((uint32_t)(((uint32_t)(x))<<FTM_MOD_MOD_SHIFT))&FTM_MOD_MOD_MASK)
/* CnSC Bit Fields */
#define FTM_CnSC_DMA                             0x1u
#define FTM_CnSC_ELSA                            0x4u
#define FTM_CnSC_ELSB                            0x8u
#define FTM_CnSC_MSA                             0x10u
#define FTM_CnSC_MSB                             0x20u
#define FTM_CnSC_CHIE                            0x40u
#define FTM_CnSC_CHF                             0x80u
/* CnV Bit Fields */
#define FTM_CnV_VAL_MASK                         0xFFFFu
#define FTM_CnV_VAL_SHIFT                        0
#define FTM_CnV_VAL(x)                           (((uint32_t)(((uint32_t)(x))<<FTM_CnV_VAL_SHIFT))&FTM_CnV_VAL_MASK)
/* CNTIN Bit Fields */
#define FTM_CNTIN_INIT_MASK                      0xFFFFu
#define FTM_CNTIN_INIT_SHIFT                     0
#define FTM_CNTIN_INIT(x)                        (((uint32_t)(((uint32_t)(x))<<FTM_CNTIN_INIT_SHIFT))&FTM_CNTIN_INIT_MASK)
/* STATUS Bit Fields */
#define FTM_STATUS_CH0F_MASK                     0x1u
#define FTM_STATUS_CH0F_SHIFT                    0
#define FTM_STATUS_CH1F_MASK                     0x2u
#define FTM_STATUS_CH1F_SHIFT                    1
#define FTM_STATUS_CH2F_MASK                     0x4u
#define FTM_STATUS_CH2F_SHIFT                    2
#define FTM_STATUS_CH3F_MASK                     0x8u
#define FTM_STATUS_CH3F_SHIFT                    3
#define FTM_STATUS_CH4F_MASK                     0x10u
#define FTM_STATUS_CH4F_SHIFT                    4
#define FTM_STATUS_CH5F_MASK                     0x20u
#define FTM_STATUS_CH5F_SHIFT                    5
#define FTM_STATUS_CH6F_MASK                     0x40u
#define FTM_STATUS_CH6F_SHIFT                    6
#define FTM_STATUS_CH7F_MASK                     0x80u
#define FTM_STATUS_CH7F_SHIFT                    7
/* MODE Bit Fields */
#define FTM_MODE_FTMEN_MASK                      0x1u
#define FTM_MODE_FTMEN_SHIFT                     0
#define FTM_MODE_INIT_MASK                       0x2u
#define FTM_MODE_INIT_SHIFT                      1
#define FTM_MODE_WPDIS_MASK                      0x4u
#define FTM_MODE_WPDIS_SHIFT                     2
#define FTM_MODE_PWMSYNC_MASK                    0x8u
#define FTM_MODE_PWMSYNC_SHIFT                   3
#define FTM_MODE_CAPTEST_MASK                    0x10u
#define FTM_MODE_CAPTEST_SHIFT                   4
#define FTM_MODE_FAULTM_MASK                     0x60u
#define FTM_MODE_FAULTM_SHIFT                    5
#define FTM_MODE_FAULTM(x)                       (((uint32_t)(((uint32_t)(x))<<FTM_MODE_FAULTM_SHIFT))&FTM_MODE_FAULTM_MASK)
#define FTM_MODE_FAULTIE_MASK                    0x80u
#define FTM_MODE_FAULTIE_SHIFT                   7
/* SYNC Bit Fields */
#define FTM_SYNC_CNTMIN_MASK                     0x1u
#define FTM_SYNC_CNTMIN_SHIFT                    0
#define FTM_SYNC_CNTMAX_MASK                     0x2u
#define FTM_SYNC_CNTMAX_SHIFT                    1
#define FTM_SYNC_REINIT_MASK                     0x4u
#define FTM_SYNC_REINIT_SHIFT                    2
#define FTM_SYNC_SYNCHOM_MASK                    0x8u
#define FTM_SYNC_SYNCHOM_SHIFT                   3
#define FTM_SYNC_TRIG0_MASK                      0x10u
#define FTM_SYNC_TRIG0_SHIFT                     4
#define FTM_SYNC_TRIG1_MASK                      0x20u
#define FTM_SYNC_TRIG1_SHIFT                     5
#define FTM_SYNC_TRIG2_MASK                      0x40u
#define FTM_SYNC_TRIG2_SHIFT                     6
#define FTM_SYNC_SWSYNC_MASK                     0x80u
#define FTM_SYNC_SWSYNC_SHIFT                    7
/* OUTINIT Bit Fields */
#define FTM_OUTINIT_CH0OI_MASK                   0x1u
#define FTM_OUTINIT_CH0OI_SHIFT                  0
#define FTM_OUTINIT_CH1OI_MASK                   0x2u
#define FTM_OUTINIT_CH1OI_SHIFT                  1
#define FTM_OUTINIT_CH2OI_MASK                   0x4u
#define FTM_OUTINIT_CH2OI_SHIFT                  2
#define FTM_OUTINIT_CH3OI_MASK                   0x8u
#define FTM_OUTINIT_CH3OI_SHIFT                  3
#define FTM_OUTINIT_CH4OI_MASK                   0x10u
#define FTM_OUTINIT_CH4OI_SHIFT                  4
#define FTM_OUTINIT_CH5OI_MASK                   0x20u
#define FTM_OUTINIT_CH5OI_SHIFT                  5
#define FTM_OUTINIT_CH6OI_MASK                   0x40u
#define FTM_OUTINIT_CH6OI_SHIFT                  6
#define FTM_OUTINIT_CH7OI_MASK                   0x80u
#define FTM_OUTINIT_CH7OI_SHIFT                  7
/* OUTMASK Bit Fields */
#define FTM_OUTMASK_CH0OM_MASK                   0x1u
#define FTM_OUTMASK_CH0OM_SHIFT                  0
#define FTM_OUTMASK_CH1OM_MASK                   0x2u
#define FTM_OUTMASK_CH1OM_SHIFT                  1
#define FTM_OUTMASK_CH2OM_MASK                   0x4u
#define FTM_OUTMASK_CH2OM_SHIFT                  2
#define FTM_OUTMASK_CH3OM_MASK                   0x8u
#define FTM_OUTMASK_CH3OM_SHIFT                  3
#define FTM_OUTMASK_CH4OM_MASK                   0x10u
#define FTM_OUTMASK_CH4OM_SHIFT                  4
#define FTM_OUTMASK_CH5OM_MASK                   0x20u
#define FTM_OUTMASK_CH5OM_SHIFT                  5
#define FTM_OUTMASK_CH6OM_MASK                   0x40u
#define FTM_OUTMASK_CH6OM_SHIFT                  6
#define FTM_OUTMASK_CH7OM_MASK                   0x80u
#define FTM_OUTMASK_CH7OM_SHIFT                  7
/* COMBINE Bit Fields */
#define FTM_COMBINE_COMBINE0_MASK                0x1u
#define FTM_COMBINE_COMBINE0_SHIFT               0
#define FTM_COMBINE_COMP0_MASK                   0x2u
#define FTM_COMBINE_COMP0_SHIFT                  1
#define FTM_COMBINE_DECAPEN0_MASK                0x4u
#define FTM_COMBINE_DECAPEN0_SHIFT               2
#define FTM_COMBINE_DECAP0_MASK                  0x8u
#define FTM_COMBINE_DECAP0_SHIFT                 3
#define FTM_COMBINE_DTEN0_MASK                   0x10u
#define FTM_COMBINE_DTEN0_SHIFT                  4
#define FTM_COMBINE_SYNCEN0_MASK                 0x20u
#define FTM_COMBINE_SYNCEN0_SHIFT                5
#define FTM_COMBINE_FAULTEN0_MASK                0x40u
#define FTM_COMBINE_FAULTEN0_SHIFT               6
#define FTM_COMBINE_COMBINE1_MASK                0x100u
#define FTM_COMBINE_COMBINE1_SHIFT               8
#define FTM_COMBINE_COMP1_MASK                   0x200u
#define FTM_COMBINE_COMP1_SHIFT                  9
#define FTM_COMBINE_DECAPEN1_MASK                0x400u
#define FTM_COMBINE_DECAPEN1_SHIFT               10
#define FTM_COMBINE_DECAP1_MASK                  0x800u
#define FTM_COMBINE_DECAP1_SHIFT                 11
#define FTM_COMBINE_DTEN1_MASK                   0x1000u
#define FTM_COMBINE_DTEN1_SHIFT                  12
#define FTM_COMBINE_SYNCEN1_MASK                 0x2000u
#define FTM_COMBINE_SYNCEN1_SHIFT                13
#define FTM_COMBINE_FAULTEN1_MASK                0x4000u
#define FTM_COMBINE_FAULTEN1_SHIFT               14
#define FTM_COMBINE_COMBINE2_MASK                0x10000u
#define FTM_COMBINE_COMBINE2_SHIFT               16
#define FTM_COMBINE_COMP2_MASK                   0x20000u
#define FTM_COMBINE_COMP2_SHIFT                  17
#define FTM_COMBINE_DECAPEN2_MASK                0x40000u
#define FTM_COMBINE_DECAPEN2_SHIFT               18
#define FTM_COMBINE_DECAP2_MASK                  0x80000u
#define FTM_COMBINE_DECAP2_SHIFT                 19
#define FTM_COMBINE_DTEN2_MASK                   0x100000u
#define FTM_COMBINE_DTEN2_SHIFT                  20
#define FTM_COMBINE_SYNCEN2_MASK                 0x200000u
#define FTM_COMBINE_SYNCEN2_SHIFT                21
#define FTM_COMBINE_FAULTEN2_MASK                0x400000u
#define FTM_COMBINE_FAULTEN2_SHIFT               22
#define FTM_COMBINE_COMBINE3_MASK                0x1000000u
#define FTM_COMBINE_COMBINE3_SHIFT               24
#define FTM_COMBINE_COMP3_MASK                   0x2000000u
#define FTM_COMBINE_COMP3_SHIFT                  25
#define FTM_COMBINE_DECAPEN3_MASK                0x4000000u
#define FTM_COMBINE_DECAPEN3_SHIFT               26
#define FTM_COMBINE_DECAP3_MASK                  0x8000000u
#define FTM_COMBINE_DECAP3_SHIFT                 27
#define FTM_COMBINE_DTEN3_MASK                   0x10000000u
#define FTM_COMBINE_DTEN3_SHIFT                  28
#define FTM_COMBINE_SYNCEN3_MASK                 0x20000000u
#define FTM_COMBINE_SYNCEN3_SHIFT                29
#define FTM_COMBINE_FAULTEN3_MASK                0x40000000u
#define FTM_COMBINE_FAULTEN3_SHIFT               30
/* DEADTIME Bit Fields */
#define FTM_DEADTIME_DTVAL_MASK                  0x3Fu
#define FTM_DEADTIME_DTVAL_SHIFT                 0
#define FTM_DEADTIME_DTVAL(x)                    (((uint32_t)(((uint32_t)(x))<<FTM_DEADTIME_DTVAL_SHIFT))&FTM_DEADTIME_DTVAL_MASK)
#define FTM_DEADTIME_DTPS_MASK                   0xC0u
#define FTM_DEADTIME_DTPS_SHIFT                  6
#define FTM_DEADTIME_DTPS(x)                     (((uint32_t)(((uint32_t)(x))<<FTM_DEADTIME_DTPS_SHIFT))&FTM_DEADTIME_DTPS_MASK)
/* EXTTRIG Bit Fields */
#define FTM_EXTTRIG_CH2TRIG_MASK                 0x1u
#define FTM_EXTTRIG_CH2TRIG_SHIFT                0
#define FTM_EXTTRIG_CH3TRIG_MASK                 0x2u
#define FTM_EXTTRIG_CH3TRIG_SHIFT                1
#define FTM_EXTTRIG_CH4TRIG_MASK                 0x4u
#define FTM_EXTTRIG_CH4TRIG_SHIFT                2
#define FTM_EXTTRIG_CH5TRIG_MASK                 0x8u
#define FTM_EXTTRIG_CH5TRIG_SHIFT                3
#define FTM_EXTTRIG_CH0TRIG_MASK                 0x10u
#define FTM_EXTTRIG_CH0TRIG_SHIFT                4
#define FTM_EXTTRIG_CH1TRIG_MASK                 0x20u
#define FTM_EXTTRIG_CH1TRIG_SHIFT                5
#define FTM_EXTTRIG_INITTRIGEN_MASK              0x40u
#define FTM_EXTTRIG_INITTRIGEN_SHIFT             6
#define FTM_EXTTRIG_TRIGF_MASK                   0x80u
#define FTM_EXTTRIG_TRIGF_SHIFT                  7
/* POL Bit Fields */
#define FTM_POL_POL0_MASK                        0x1u
#define FTM_POL_POL0_SHIFT                       0
#define FTM_POL_POL1_MASK                        0x2u
#define FTM_POL_POL1_SHIFT                       1
#define FTM_POL_POL2_MASK                        0x4u
#define FTM_POL_POL2_SHIFT                       2
#define FTM_POL_POL3_MASK                        0x8u
#define FTM_POL_POL3_SHIFT                       3
#define FTM_POL_POL4_MASK                        0x10u
#define FTM_POL_POL4_SHIFT                       4
#define FTM_POL_POL5_MASK                        0x20u
#define FTM_POL_POL5_SHIFT                       5
#define FTM_POL_POL6_MASK                        0x40u
#define FTM_POL_POL6_SHIFT                       6
#define FTM_POL_POL7_MASK                        0x80u
#define FTM_POL_POL7_SHIFT                       7
/* FMS Bit Fields */
#define FTM_FMS_FAULTF0_MASK                     0x1u
#define FTM_FMS_FAULTF0_SHIFT                    0
#define FTM_FMS_FAULTF1_MASK                     0x2u
#define FTM_FMS_FAULTF1_SHIFT                    1
#define FTM_FMS_FAULTF2_MASK                     0x4u
#define FTM_FMS_FAULTF2_SHIFT                    2
#define FTM_FMS_FAULTF3_MASK                     0x8u
#define FTM_FMS_FAULTF3_SHIFT                    3
#define FTM_FMS_FAULTIN_MASK                     0x20u
#define FTM_FMS_FAULTIN_SHIFT                    5
#define FTM_FMS_WPEN_MASK                        0x40u
#define FTM_FMS_WPEN_SHIFT                       6
#define FTM_FMS_FAULTF_MASK                      0x80u
#define FTM_FMS_FAULTF_SHIFT                     7
/* FILTER Bit Fields */
#define FTM_FILTER_CH0FVAL_MASK                  0xFu
#define FTM_FILTER_CH0FVAL_SHIFT                 0
#define FTM_FILTER_CH0FVAL(x)                    (((uint32_t)(((uint32_t)(x))<<FTM_FILTER_CH0FVAL_SHIFT))&FTM_FILTER_CH0FVAL_MASK)
#define FTM_FILTER_CH1FVAL_MASK                  0xF0u
#define FTM_FILTER_CH1FVAL_SHIFT                 4
#define FTM_FILTER_CH1FVAL(x)                    (((uint32_t)(((uint32_t)(x))<<FTM_FILTER_CH1FVAL_SHIFT))&FTM_FILTER_CH1FVAL_MASK)
#define FTM_FILTER_CH2FVAL_MASK                  0xF00u
#define FTM_FILTER_CH2FVAL_SHIFT                 8
#define FTM_FILTER_CH2FVAL(x)                    (((uint32_t)(((uint32_t)(x))<<FTM_FILTER_CH2FVAL_SHIFT))&FTM_FILTER_CH2FVAL_MASK)
#define FTM_FILTER_CH3FVAL_MASK                  0xF000u
#define FTM_FILTER_CH3FVAL_SHIFT                 12
#define FTM_FILTER_CH3FVAL(x)                    (((uint32_t)(((uint32_t)(x))<<FTM_FILTER_CH3FVAL_SHIFT))&FTM_FILTER_CH3FVAL_MASK)
/* FLTCTRL Bit Fields */
#define FTM_FLTCTRL_FAULT0EN_MASK                0x1u
#define FTM_FLTCTRL_FAULT0EN_SHIFT               0
#define FTM_FLTCTRL_FAULT1EN_MASK                0x2u
#define FTM_FLTCTRL_FAULT1EN_SHIFT               1
#define FTM_FLTCTRL_FAULT2EN_MASK                0x4u
#define FTM_FLTCTRL_FAULT2EN_SHIFT               2
#define FTM_FLTCTRL_FAULT3EN_MASK                0x8u
#define FTM_FLTCTRL_FAULT3EN_SHIFT               3
#define FTM_FLTCTRL_FFLTR0EN_MASK                0x10u
#define FTM_FLTCTRL_FFLTR0EN_SHIFT               4
#define FTM_FLTCTRL_FFLTR1EN_MASK                0x20u
#define FTM_FLTCTRL_FFLTR1EN_SHIFT               5
#define FTM_FLTCTRL_FFLTR2EN_MASK                0x40u
#define FTM_FLTCTRL_FFLTR2EN_SHIFT               6
#define FTM_FLTCTRL_FFLTR3EN_MASK                0x80u
#define FTM_FLTCTRL_FFLTR3EN_SHIFT               7
#define FTM_FLTCTRL_FFVAL_MASK                   0xF00u
#define FTM_FLTCTRL_FFVAL_SHIFT                  8
#define FTM_FLTCTRL_FFVAL(x)                     (((uint32_t)(((uint32_t)(x))<<FTM_FLTCTRL_FFVAL_SHIFT))&FTM_FLTCTRL_FFVAL_MASK)
/* QDCTRL Bit Fields */
#define FTM_QDCTRL_QUADEN_MASK                   0x1u
#define FTM_QDCTRL_QUADEN_SHIFT                  0
#define FTM_QDCTRL_TOFDIR_MASK                   0x2u
#define FTM_QDCTRL_TOFDIR_SHIFT                  1
#define FTM_QDCTRL_QUADIR_MASK                   0x4u
#define FTM_QDCTRL_QUADIR_SHIFT                  2
#define FTM_QDCTRL_QUADMODE_MASK                 0x8u
#define FTM_QDCTRL_QUADMODE_SHIFT                3
#define FTM_QDCTRL_PHBPOL_MASK                   0x10u
#define FTM_QDCTRL_PHBPOL_SHIFT                  4
#define FTM_QDCTRL_PHAPOL_MASK                   0x20u
#define FTM_QDCTRL_PHAPOL_SHIFT                  5
#define FTM_QDCTRL_PHBFLTREN_MASK                0x40u
#define FTM_QDCTRL_PHBFLTREN_SHIFT               6
#define FTM_QDCTRL_PHAFLTREN_MASK                0x80u
#define FTM_QDCTRL_PHAFLTREN_SHIFT               7
/* CONF Bit Fields */
#define FTM_CONF_NUMTOF_MASK                     0x1Fu
#define FTM_CONF_NUMTOF_SHIFT                    0
#define FTM_CONF_NUMTOF(x)                       (((uint32_t)(((uint32_t)(x))<<FTM_CONF_NUMTOF_SHIFT))&FTM_CONF_NUMTOF_MASK)
#define FTM_CONF_BDMMODE_MASK                    0xC0u
#define FTM_CONF_BDMMODE_SHIFT                   6
#define FTM_CONF_BDMMODE(x)                      (((uint32_t)(((uint32_t)(x))<<FTM_CONF_BDMMODE_SHIFT))&FTM_CONF_BDMMODE_MASK)
#define FTM_CONF_GTBEEN_MASK                     0x200u
#define FTM_CONF_GTBEEN_SHIFT                    9
#define FTM_CONF_GTBEOUT_MASK                    0x400u
#define FTM_CONF_GTBEOUT_SHIFT                   10
/* FLTPOL Bit Fields */
#define FTM_FLTPOL_FLT0POL_MASK                  0x1u
#define FTM_FLTPOL_FLT0POL_SHIFT                 0
#define FTM_FLTPOL_FLT1POL_MASK                  0x2u
#define FTM_FLTPOL_FLT1POL_SHIFT                 1
#define FTM_FLTPOL_FLT2POL_MASK                  0x4u
#define FTM_FLTPOL_FLT2POL_SHIFT                 2
#define FTM_FLTPOL_FLT3POL_MASK                  0x8u
#define FTM_FLTPOL_FLT3POL_SHIFT                 3
/* SYNCONF Bit Fields */
#define FTM_SYNCONF_HWTRIGMODE_MASK              0x1u
#define FTM_SYNCONF_HWTRIGMODE_SHIFT             0
#define FTM_SYNCONF_CNTINC_MASK                  0x4u
#define FTM_SYNCONF_CNTINC_SHIFT                 2
#define FTM_SYNCONF_INVC_MASK                    0x10u
#define FTM_SYNCONF_INVC_SHIFT                   4
#define FTM_SYNCONF_SWOC_MASK                    0x20u
#define FTM_SYNCONF_SWOC_SHIFT                   5
#define FTM_SYNCONF_SYNCMODE_MASK                0x80u
#define FTM_SYNCONF_SYNCMODE_SHIFT               7
#define FTM_SYNCONF_SWRSTCNT_MASK                0x100u
#define FTM_SYNCONF_SWRSTCNT_SHIFT               8
#define FTM_SYNCONF_SWWRBUF_MASK                 0x200u
#define FTM_SYNCONF_SWWRBUF_SHIFT                9
#define FTM_SYNCONF_SWOM_MASK                    0x400u
#define FTM_SYNCONF_SWOM_SHIFT                   10
#define FTM_SYNCONF_SWINVC_MASK                  0x800u
#define FTM_SYNCONF_SWINVC_SHIFT                 11
#define FTM_SYNCONF_SWSOC_MASK                   0x1000u
#define FTM_SYNCONF_SWSOC_SHIFT                  12
#define FTM_SYNCONF_HWRSTCNT_MASK                0x10000u
#define FTM_SYNCONF_HWRSTCNT_SHIFT               16
#define FTM_SYNCONF_HWWRBUF_MASK                 0x20000u
#define FTM_SYNCONF_HWWRBUF_SHIFT                17
#define FTM_SYNCONF_HWOM_MASK                    0x40000u
#define FTM_SYNCONF_HWOM_SHIFT                   18
#define FTM_SYNCONF_HWINVC_MASK                  0x80000u
#define FTM_SYNCONF_HWINVC_SHIFT                 19
#define FTM_SYNCONF_HWSOC_MASK                   0x100000u
#define FTM_SYNCONF_HWSOC_SHIFT                  20
/* INVCTRL Bit Fields */
#define FTM_INVCTRL_INV0EN_MASK                  0x1u
#define FTM_INVCTRL_INV0EN_SHIFT                 0
#define FTM_INVCTRL_INV1EN_MASK                  0x2u
#define FTM_INVCTRL_INV1EN_SHIFT                 1
#define FTM_INVCTRL_INV2EN_MASK                  0x4u
#define FTM_INVCTRL_INV2EN_SHIFT                 2
#define FTM_INVCTRL_INV3EN_MASK                  0x8u
#define FTM_INVCTRL_INV3EN_SHIFT                 3
/* SWOCTRL Bit Fields */
#define FTM_SWOCTRL_CH0OC_MASK                   0x1u
#define FTM_SWOCTRL_CH0OC_SHIFT                  0
#define FTM_SWOCTRL_CH1OC_MASK                   0x2u
#define FTM_SWOCTRL_CH1OC_SHIFT                  1
#define FTM_SWOCTRL_CH2OC_MASK                   0x4u
#define FTM_SWOCTRL_CH2OC_SHIFT                  2
#define FTM_SWOCTRL_CH3OC_MASK                   0x8u
#define FTM_SWOCTRL_CH3OC_SHIFT                  3
#define FTM_SWOCTRL_CH4OC_MASK                   0x10u
#define FTM_SWOCTRL_CH4OC_SHIFT                  4
#define FTM_SWOCTRL_CH5OC_MASK                   0x20u
#define FTM_SWOCTRL_CH5OC_SHIFT                  5
#define FTM_SWOCTRL_CH6OC_MASK                   0x40u
#define FTM_SWOCTRL_CH6OC_SHIFT                  6
#define FTM_SWOCTRL_CH7OC_MASK                   0x80u
#define FTM_SWOCTRL_CH7OC_SHIFT                  7
#define FTM_SWOCTRL_CH0OCV_MASK                  0x100u
#define FTM_SWOCTRL_CH0OCV_SHIFT                 8
#define FTM_SWOCTRL_CH1OCV_MASK                  0x200u
#define FTM_SWOCTRL_CH1OCV_SHIFT                 9
#define FTM_SWOCTRL_CH2OCV_MASK                  0x400u
#define FTM_SWOCTRL_CH2OCV_SHIFT                 10
#define FTM_SWOCTRL_CH3OCV_MASK                  0x800u
#define FTM_SWOCTRL_CH3OCV_SHIFT                 11
#define FTM_SWOCTRL_CH4OCV_MASK                  0x1000u
#define FTM_SWOCTRL_CH4OCV_SHIFT                 12
#define FTM_SWOCTRL_CH5OCV_MASK                  0x2000u
#define FTM_SWOCTRL_CH5OCV_SHIFT                 13
#define FTM_SWOCTRL_CH6OCV_MASK                  0x4000u
#define FTM_SWOCTRL_CH6OCV_SHIFT                 14
#define FTM_SWOCTRL_CH7OCV_MASK                  0x8000u
#define FTM_SWOCTRL_CH7OCV_SHIFT                 15
/* PWMLOAD Bit Fields */
#define FTM_PWMLOAD_CH0SEL_MASK                  0x1u
#define FTM_PWMLOAD_CH0SEL_SHIFT                 0
#define FTM_PWMLOAD_CH1SEL_MASK                  0x2u
#define FTM_PWMLOAD_CH1SEL_SHIFT                 1
#define FTM_PWMLOAD_CH2SEL_MASK                  0x4u
#define FTM_PWMLOAD_CH2SEL_SHIFT                 2
#define FTM_PWMLOAD_CH3SEL_MASK                  0x8u
#define FTM_PWMLOAD_CH3SEL_SHIFT                 3
#define FTM_PWMLOAD_CH4SEL_MASK                  0x10u
#define FTM_PWMLOAD_CH4SEL_SHIFT                 4
#define FTM_PWMLOAD_CH5SEL_MASK                  0x20u
#define FTM_PWMLOAD_CH5SEL_SHIFT                 5
#define FTM_PWMLOAD_CH6SEL_MASK                  0x40u
#define FTM_PWMLOAD_CH6SEL_SHIFT                 6
#define FTM_PWMLOAD_CH7SEL_MASK                  0x80u
#define FTM_PWMLOAD_CH7SEL_SHIFT                 7
#define FTM_PWMLOAD_LDOK_MASK                    0x200u
#define FTM_PWMLOAD_LDOK_SHIFT                   9

/****************************************************************/
/*                                                              */
/*                 Periodic Interrupt Timer (PIT)               */
/*                                                              */
/****************************************************************/
/* MCR Bit Fields */
#define PIT_MCR_FRZ                              0x1u
#define PIT_MCR_MDIS                             0x2u
/* LDVAL Bit Fields */
#define PIT_LDVAL_TSV_MASK                       0xFFFFFFFFu
#define PIT_LDVAL_TSV_SHIFT                      0
#define PIT_LDVAL_TSV(x)                         (((uint32_t)(((uint32_t)(x))<<PIT_LDVAL_TSV_SHIFT))&PIT_LDVAL_TSV_MASK)
/* CVAL Bit Fields */
#define PIT_CVAL_TVL_MASK                        0xFFFFFFFFu
#define PIT_CVAL_TVL_SHIFT                       0
#define PIT_CVAL_TVL(x)                          (((uint32_t)(((uint32_t)(x))<<PIT_CVAL_TVL_SHIFT))&PIT_CVAL_TVL_MASK)
/* TCTRL Bit Fields */
#define PIT_TCTRL_TEN                            0x1u
#define PIT_TCTRL_TIE                            0x2u
/* TFLG Bit Fields */
#define PIT_TFLG_TIF                             0x1u

/****************************************************************/
/*                                                              */
/*              Analog-to-Digital Converter (ADC)               */
/*                                                              */
/****************************************************************/
/***********  Bits definition for ADCx_SC1n register  ***********/
#define ADCx_SC1n_COCO          ((uint32_t)((uint32_t)1 << 7))  /*!< Conversion Complete Flag */
#define ADCx_SC1n_AIEN          ((uint32_t)((uint32_t)1 << 6))  /*!< Interrupt Enable */
#define ADCx_SC1n_DIFF          ((uint32_t)((uint32_t)1 << 5))  /*!< Differential Mode Enable */
#define ADCx_SC1n_ADCH_SHIFT    0                                                                            /*!< Input channel select (shift) */
#define ADCx_SC1n_ADCH_MASK     ((uint32_t)((uint32_t)0x1F << ADCx_SC1n_ADCH_SHIFT))                         /*!< Input channel select (mask) */
#define ADCx_SC1n_ADCH(x)       ((uint32_t)(((uint32_t)(x) << ADCx_SC1n_ADCH_SHIFT) & ADCx_SC1n_ADCH_MASK))  /*!< Input channel select */

/***********  Bits definition for ADCx_CFG1 register  ***********/
#define ADCx_CFG1_ADLPC         ((uint32_t)((uint32_t)1 << 7))  /*!< Low-Power Configuration */
#define ADCx_CFG1_ADIV_SHIFT    5                                                                            /*!< Clock Divide Select (shift) */
#define ADCx_CFG1_ADIV_MASK     ((uint32_t)((uint32_t)0x03 << ADCx_CFG1_ADIV_SHIFT))                         /*!< Clock Divide Select (mask) */
#define ADCx_CFG1_ADIV(x)       ((uint32_t)(((uint32_t)(x) << ADCx_CFG1_ADIV_SHIFT) & ADCx_CFG1_ADIV_MASK))  /*!< Clock Divide Select */
#define ADCx_CFG1_ADLSMP        ((uint32_t)((uint32_t)1 << 4))  /*!< Sample time configuration */
#define ADCx_CFG1_MODE_SHIFT    2                                                                            /*!< Conversion mode (resolution) selection (shift) */
#define ADCx_CFG1_MODE_MASK     ((uint32_t)((uint32_t)0x03 << ADCx_CFG1_MODE_SHIFT))                         /*!< Conversion mode (resolution) selection (mask) */
#define ADCx_CFG1_MODE(x)       ((uint32_t)(((uint32_t)(x) << ADCx_CFG1_MODE_SHIFT) & ADCx_CFG1_MODE_MASK))  /*!< Conversion mode (resolution) selection */
#define ADCx_CFG1_ADICLK_SHIFT  0                                                                                /*!< Input Clock Select (shift) */
#define ADCx_CFG1_ADICLK_MASK   ((uint32_t)((uint32_t)0x03 << ADCx_CFG1_ADICLK_SHIFT))                           /*!< Input Clock Select (mask) */
#define ADCx_CFG1_ADICLK(x)     ((uint32_t)(((uint32_t)(x) << ADCx_CFG1_ADICLK_SHIFT) & ADCx_CFG1_ADICLK_MASK))  /*!< Input Clock Select */

/***********  Bits definition for ADCx_CFG2 register  ***********/
#define ADCx_CFG2_MUXSEL        ((uint32_t)((uint32_t)1 << 4))  /*!< ADC Mux Select */
#define ADCx_CFG2_ADACKEN       ((uint32_t)((uint32_t)1 << 3))  /*!< Asynchronous Clock Output Enable */
#define ADCx_CFG2_ADHSC         ((uint32_t)((uint32_t)1 << 2))  /*!< High-Speed Configuration */
#define ADCx_CFG2_ADLSTS_SHIFT  0                                                                                /*!< Long Sample Time Select (shift) */
#define ADCx_CFG2_ADLSTS_MASK   ((uint32_t)((uint32_t)0x03 << ADCx_CFG2_ADLSTS_SHIFT))                           /*!< Long Sample Time Select (mask) */
#define ADCx_CFG2_ADLSTS(x)     ((uint32_t)(((uint32_t)(x) << ADCx_CFG2_ADLSTS_SHIFT) & ADCx_CFG2_ADLSTS_MASK))  /*!< Long Sample Time Select */

/***********  Bits definition for ADCx_SC2 register  ***********/
#define ADCx_SC2_ADACT          ((uint32_t)((uint32_t)1 << 7))  /*!< Conversion Active */
#define ADCx_SC2_ADTRG          ((uint32_t)((uint32_t)1 << 6))  /*!< Conversion Trigger Select */
#define ADCx_SC2_ACFE           ((uint32_t)((uint32_t)1 << 5))  /*!< Compare Function Enable */
#define ADCx_SC2_ACFGT          ((uint32_t)((uint32_t)1 << 4))  /*!< Compare Function Greater Than Enable */
#define ADCx_SC2_ACREN          ((uint32_t)((uint32_t)1 << 3))  /*!< Compare Function Range Enable */
#define ADCx_SC2_DMAEN          ((uint32_t)((uint32_t)1 << 2))  /*!< DMA Enable */
#define ADCx_SC2_REFSEL_SHIFT   0                                                                              /*!< Voltage Reference Selection (shift) */
#define ADCx_SC2_REFSEL_MASK    ((uint32_t)((uint32_t)0x03 << ADCx_SC2_REFSEL_SHIFT))                          /*!< Voltage Reference Selection (mask) */
#define ADCx_SC2_REFSEL(x)      ((uint32_t)(((uint32_t)(x) << ADCx_SC2_REFSEL_SHIFT) & ADCx_SC2_REFSEL_MASK))  /*!< Voltage Reference Selection */

/***********  Bits definition for ADCx_SC3 register  ***********/
#define ADCx_SC3_CAL            ((uint32_t)((uint32_t)1 << 7))  /*!< Calibration */
#define ADCx_SC3_CALF           ((uint32_t)((uint32_t)1 << 6))  /*!< Calibration Failed Flag */
#define ADCx_SC3_ADCO           ((uint32_t)((uint32_t)1 << 3))  /*!< Continuous Conversion Enable */
#define ADCx_SC3_AVGE           ((uint32_t)((uint32_t)1 << 2))  /*!< Hardware Average Enable */
#define ADCx_SC3_AVGS_SHIFT     0                                                                          /*!< Hardware Average Select (shift) */
#define ADCx_SC3_AVGS_MASK      ((uint32_t)((uint32_t)0x03 << ADCx_SC3_AVGS_SHIFT))                        /*!< Hardware Average Select (mask) */
#define ADCx_SC3_AVGS(x)        ((uint32_t)(((uint32_t)(x) << ADCx_SC3_AVGS_SHIFT) & ADCx_SC3_AVGS_MASK))  /*!< Hardware Average Select */

/****************************************************************/
/*                                                              */
/*                   Low-Power Timer (LPTMR)                    */
/*                                                              */
/****************************************************************/
/**********  Bits definition for LPTMRx_CSR register  ***********/
#define LPTMRx_CSR_TCF              ((uint32_t)((uint32_t)1 << 7))  /*!< Timer Compare Flag */
#define LPTMRx_CSR_TIE              ((uint32_t)((uint32_t)1 << 6))  /*!< Timer Interrupt Enable */
#define LPTMRx_CSR_TPS_SHIFT        4                                                                            /*!< Timer Pin Select (shift) */
#define LPTMRx_CSR_TPS_MASK         ((uint32_t)((uint32_t)0x03 << LPTMRx_CSR_TPS_SHIFT))                         /*!< Timer Pin Select (mask) */
#define LPTMRx_CSR_TPS(x)           ((uint32_t)(((uint32_t)(x) << LPTMRx_CSR_TPS_SHIFT) & LPTMRx_CSR_TPS_MASK))  /*!< Timer Pin Select */
#define LPTMRx_CSR_TPP              ((uint32_t)((uint32_t)1 << 3))  /*!< Timer Pin Polarity */
#define LPTMRx_CSR_TFC              ((uint32_t)((uint32_t)1 << 2))  /*!< Timer Free-Running Counter */
#define LPTMRx_CSR_TMS              ((uint32_t)((uint32_t)1 << 1))  /*!< Timer Mode Select */
#define LPTMRx_CSR_TEN              ((uint32_t)((uint32_t)1 << 0))  /*!< Timer Enable */

/**********  Bits definition for LPTMRx_PSR register  ***********/
#define LPTMRx_PSR_PRESCALE_SHIFT   3                                                                                      /*!< Prescale Value (shift) */
#define LPTMRx_PSR_PRESCALE_MASK    ((uint32_t)((uint32_t)0x0F << LPTMRx_PSR_PRESCALE_SHIFT))                              /*!< Prescale Value (mask) */
#define LPTMRx_PSR_PRESCALE(x)      ((uint32_t)(((uint32_t)(x) << LPTMRx_PSR_PRESCALE_SHIFT) & LPTMRx_PSR_PRESCALE_MASK))  /*!< Prescale Value */
#define LPTMRx_PSR_PBYP             ((uint32_t)((uint32_t)1 << 2))  /*!< Prescaler Bypass */
#define LPTMRx_PSR_PCS_SHIFT        0                                                                            /*!< Prescaler Clock Select (shift) */
#define LPTMRx_PSR_PCS_MASK         ((uint32_t)((uint32_t)0x03 << LPTMRx_PSR_PCS_SHIFT))                         /*!< Prescaler Clock Select (mask) */
#define LPTMRx_PSR_PCS(x)           ((uint32_t)(((uint32_t)(x) << LPTMRx_PSR_PCS_SHIFT) & LPTMRx_PSR_PCS_MASK))  /*!< Prescaler Clock Select */

/**********  Bits definition for LPTMRx_CMR register  ***********/
#define LPTMRx_CMR_COMPARE_SHIFT    0                                                                                    /*!< Compare Value (shift) */
#define LPTMRx_CMR_COMPARE_MASK     ((uint32_t)((uint32_t)0xFFFF << LPTMRx_CMR_COMPARE_SHIFT))                           /*!< Compare Value (mask) */
#define LPTMRx_CMR_COMPARE(x)       ((uint32_t)(((uint32_t)(x) << LPTMRx_CMR_COMPARE_SHIFT) & LPTMRx_CMR_COMPARE_MASK))  /*!< Compare Value */

/**********  Bits definition for LPTMRx_CNR register  ***********/
#define LPTMRx_CNR_COUNTER_SHIFT    0                                                                                    /*!< Counter Value (shift) */
#define LPTMRx_CNR_COUNTER_MASK     ((uint32_t)((uint32_t)0xFFFF << LPTMRx_CNR_COUNTER_SHIFT))                           /*!< Counter Value (mask) */
#define LPTMRx_CNR_COUNTER(x)       ((uint32_t)(((uint32_t)(x) << LPTMRx_CNR_COUNTER_SHIFT) & LPTMRx_CNR_COUNTER_MASK))  /*!< Counter Value */

/****************************************************************/
/*                                                              */
/*                  Touch Sensing Input (TSI)                   */
/*                                                              */
/****************************************************************/
/**********  Bits definition for TSIx_GENCS register  ***********/
#define TSIx_GENCS_OUTRGF           ((uint32_t)((uint32_t)1 << 31))  /*!< Out of Range Flag */
#define TSIx_GENCS_ESOR             ((uint32_t)((uint32_t)1 << 28))  /*!< End-of-scan/Out-of-Range Interrupt Selection */
#define TSIx_GENCS_MODE_SHIFT       24                                                                                 /*!< TSI analog modes setup and status bits (shift) */
#define TSIx_GENCS_MODE_MASK        ((uint32_t)((uint32_t)0x0F << TSIx_GENCS_MODE_SHIFT))                            /*!< TSI analog modes setup and status bits (mask) */
#define TSIx_GENCS_MODE(x)          ((uint32_t)(((uint32_t)(x) << TSIx_GENCS_MODE_SHIFT) & TSIx_GENCS_MODE_MASK))  /*!< TSI analog modes setup and status bits */
#define TSIx_GENCS_REFCHRG_SHIFT    21                                                                                       /*!< Reference oscillator charge/discharge current (shift) */
#define TSIx_GENCS_REFCHRG_MASK     ((uint32_t)((uint32_t)0x07 << TSIx_GENCS_REFCHRG_SHIFT))                               /*!< Reference oscillator charge/discharge current (mask) */
#define TSIx_GENCS_REFCHRG(x)       ((uint32_t)(((uint32_t)(x) << TSIx_GENCS_REFCHRG_SHIFT) & TSIx_GENCS_REFCHRG_MASK))  /*!< Reference oscillator charge/discharge current */
#define TSIx_GENCS_DVOLT_SHIFT      19                                                                                   /*!< Oscillator voltage rails (shift) */
#define TSIx_GENCS_DVOLT_MASK       ((uint32_t)((uint32_t)0x03 << TSIx_GENCS_DVOLT_SHIFT))                             /*!< Oscillator voltage rails (mask) */
#define TSIx_GENCS_DVOLT(x)         ((uint32_t)(((uint32_t)(x) << TSIx_GENCS_DVOLT_SHIFT) & TSIx_GENCS_DVOLT_MASK))  /*!< Oscillator voltage rails */
#define TSIx_GENCS_EXTCHRG_SHIFT    16                                                                                       /*!< Electrode oscillator charge/discharge current (shift) */
#define TSIx_GENCS_EXTCHRG_MASK     ((uint32_t)((uint32_t)0x07 << TSIx_GENCS_EXTCHRG_SHIFT))                               /*!< Electrode oscillator charge/discharge current (mask) */
#define TSIx_GENCS_EXTCHRG(x)       ((uint32_t)(((uint32_t)(x) << TSIx_GENCS_EXTCHRG_SHIFT) & TSIx_GENCS_EXTCHRG_MASK))  /*!< Electrode oscillator charge/discharge current */
#define TSIx_GENCS_PS_SHIFT         13                                                                             /*!< Electrode oscillator prescaler (shift) */
#define TSIx_GENCS_PS_MASK          ((uint32_t)((uint32_t)0x07 << TSIx_GENCS_PS_SHIFT))                          /*!< Electrode oscillator prescaler (mask) */
#define TSIx_GENCS_PS(x)            ((uint32_t)(((uint32_t)(x) << TSIx_GENCS_PS_SHIFT) & TSIx_GENCS_PS_MASK))  /*!< Electrode oscillator prescaler */
#define TSIx_GENCS_NSCN_SHIFT       8                                                                                  /*!< Number of scans per electrode minus 1 (shift) */
#define TSIx_GENCS_NSCN_MASK        ((uint32_t)((uint32_t)0x1F << TSIx_GENCS_NSCN_SHIFT))                            /*!< Number of scans per electrode minus 1 (mask) */
#define TSIx_GENCS_NSCN(x)          ((uint32_t)(((uint32_t)(x) << TSIx_GENCS_NSCN_SHIFT) & TSIx_GENCS_NSCN_MASK))  /*!< Number of scans per electrode minus 1 */
#define TSIx_GENCS_TSIEN            ((uint32_t)((uint32_t)1 << 7))  /*!< TSI Module Enable */
#define TSIx_GENCS_TSIIEN           ((uint32_t)((uint32_t)1 << 6))  /*!< TSI Interrupt Enable */
#define TSIx_GENCS_STPE             ((uint32_t)((uint32_t)1 << 5))  /*!< TSI STOP Enable */
#define TSIx_GENCS_STM              ((uint32_t)((uint32_t)1 << 4))  /*!< Scan Trigger Mode (0=software; 1=hardware) */
#define TSIx_GENCS_SCNIP            ((uint32_t)((uint32_t)1 << 3))  /*!< Scan in Progress Status */
#define TSIx_GENCS_EOSF             ((uint32_t)((uint32_t)1 << 2))  /*!< End of Scan Flag */
#define TSIx_GENCS_CURSW            ((uint32_t)((uint32_t)1 << 1))  /*!< Swap electrode and reference current sources */

/**********  Bits definition for TSIx_DATA register  ************/
#define TSIx_DATA_TSICH_SHIFT       28                                                                             /*!< Specify channel to be measured (shift) */
#define TSIx_DATA_TSICH_MASK        ((uint32_t)((uint32_t)0x0F << TSIx_DATA_TSICH_SHIFT))                          /*!< Specify channel to be measured (mask) */
#define TSIx_DATA_TSICH(x)          ((uint32_t)(((uint32_t)(x) << TSIx_DATA_TSICH_SHIFT) & TSIx_DATA_TSICH_MASK))  /*!< Specify channel to be measured */
#define TSIx_DATA_DMAEN             ((uint32_t)((uint32_t)1 << 23))  /*!< DMA Transfer Enabled */
#define TSIx_DATA_SWTS              ((uint32_t)((uint32_t)1 << 22))  /*!< Software Trigger Start */
#define TSIx_DATA_TSICNT_SHIFT      0                                                                                /*!< TSI Conversion Counter Value (shift) */
#define TSIx_DATA_TSICNT_MASK       ((uint32_t)((uint32_t)0xFFFF << TSIx_DATA_TSICNT_SHIFT))                         /*!< TSI Conversion Counter Value (mask) */
#define TSIx_DATA_TSICNT(x)         ((uint32_t)(((uint32_t)(x) << TSIx_DATA_TSICNT_SHIFT) & TSIx_DATA_TSICNT_MASK))  /*!< TSI Conversion Counter Value */

/**********  Bits definition for TSIx_TSHD register  ************/
#define TSIx_TSHD_THRESH_SHIFT      16                                                                               /*!< TSI Wakeup Channel High-Threshold (shift) */
#define TSIx_TSHD_THRESH_MASK       ((uint32_t)((uint32_t)0xFFFF << TSIx_TSHD_THRESH_SHIFT))                         /*!< TSI Wakeup Channel High-Threshold (mask) */
#define TSIx_TSHD_THRESH(x)         ((uint32_t)(((uint32_t)(x) << TSIx_TSHD_THRESH_SHIFT) & TSIx_TSHD_THRESH_MASK))  /*!< TSI Wakeup Channel High-Threshold */
#define TSIx_TSHD_THRESL_SHIFT      0                                                                                /*!< TSI Wakeup Channel Low-Threshold (shift) */
#define TSIx_TSHD_THRESL_MASK       ((uint32_t)((uint32_t)0xFFFF << TSIx_TSHD_THRESL_SHIFT))                         /*!< TSI Wakeup Channel Low-Threshold (mask) */
#define TSIx_TSHD_THRESL(x)         ((uint32_t)(((uint32_t)(x) << TSIx_TSHD_THRESL_SHIFT) & TSIx_TSHD_THRESL_MASK))  /*!< TSI Wakeup Channel Low-Threshold */

/****************************************************************/
/*                                                              */
/*             Multipurpose Clock Generator (MCG)               */
/*                                                              */
/****************************************************************/
/***********  Bits definition for MCG_C1 register  **************/
#define MCG_C1_CLKS_SHIFT           6                                                           /*!< Clock source select (shift) */
#define MCG_C1_CLKS_MASK            ((uint8_t)((uint8_t)0x3 << MCG_C1_CLKS_SHIFT))             /*!< Clock source select (mask) */
#define MCG_C1_CLKS(x)              ((uint8_t)(((uint8_t)(x) << MCG_C1_CLKS_SHIFT) & MCG_C1_CLKS_MASK))  /*!< Clock source select */
#define MCG_C1_CLKS_FLLPLL          MCG_C1_CLKS(0)  /*!< Select output of FLL or PLL, depending on PLLS control bit */
#define MCG_C1_CLKS_IRCLK           MCG_C1_CLKS(1)  /*!< Select internal reference clock */
#define MCG_C1_CLKS_ERCLK           MCG_C1_CLKS(2)  /*!< Select external reference clock */
#define MCG_C1_FRDIV_SHIFT          3                                                           /*!< FLL External Reference Divider (shift) */
#define MCG_C1_FRDIV_MASK           ((uint8_t)((uint8_t)0x7 << MCG_C1_FRDIV_SHIFT))            /*!< FLL External Reference Divider (mask) */
#define MCG_C1_FRDIV(x)             ((uint8_t)(((uint8_t)(x) << MCG_C1_FRDIV_SHIFT) & MCG_C1_FRDIV_MASK))  /*!< FLL External Reference Divider */
#define MCG_C1_IREFS                ((uint8_t)0x04) /*!< Internal Reference Select (0=ERCLK; 1=slow IRCLK) */
#define MCG_C1_IRCLKEN              ((uint8_t)0x02) /*!< Internal Reference Clock Enable */
#define MCG_C1_IREFSTEN             ((uint8_t)0x01) /*!< Internal Reference Stop Enable */

/***********  Bits definition for MCG_C2 register  **************/
#define MCG_C2_LOCRE0               ((uint8_t)0x80) /*!< Loss of Clock Reset Enable */
#define MCG_C2_RANGE0_SHIFT         4               /*!< Frequency Range Select (shift) */
#define MCG_C2_RANGE0_MASK          ((uint8_t)((uint8_t)0x3 << MCG_C2_RANGE0_SHIFT))  /*!< Frequency Range Select (mask) */
#define MCG_C2_RANGE0(x)            ((uint8_t)(((uint8_t)(x) << MCG_C2_RANGE0_SHIFT) & MCG_C2_RANGE0_MASK))  /*!< Frequency Range Select */
#define MCG_C2_HGO0                 ((uint8_t)0x08) /*!< High Gain Oscillator Select (0=low power; 1=high gain) */
#define MCG_C2_EREFS0               ((uint8_t)0x04) /*!< External Reference Select (0=clock; 1=oscillator) */
#define MCG_C2_LP                   ((uint8_t)0x02) /*!< Low Power Select (1=FLL/PLL disabled in bypass modes) */
#define MCG_C2_IRCS                 ((uint8_t)0x01) /*!< Internal Reference Clock Select (0=slow; 1=fast) */

/***********  Bits definition for MCG_C4 register  **************/
#define MCG_C4_DMX32                ((uint8_t)0x80) /*!< DCO Maximum Frequency with 32.768 kHz Reference */
#define MCG_C4_DRST_DRS_SHIFT       5               /*!< DCO Range Select (shift) */
#define MCG_C4_DRST_DRS_MASK        ((uint8_t)((uint8_t)0x3 << MCG_C4_DRST_DRS_SHIFT)) /*!< DCO Range Select (mask) */
#define MCG_C4_DRST_DRS(x)          ((uint8_t)(((uint8_t)(x) << MCG_C4_DRST_DRS_SHIFT) & MCG_C4_DRST_DRS_MASK))  /*!< DCO Range Select */
#define MCG_C4_FCTRIM_SHIFT         1               /*!< Fast Internal Reference Clock Trim Setting (shift) */
#define MCG_C4_FCTRIM_MASK          ((uint8_t)((uint8_t)0xF << MCG_C4_FCTRIM_SHIFT))   /*!< Fast Internal Reference Clock Trim Setting (mask) */
#define MCG_C4_FCTRIM(x)            ((uint8_t)(((uint8_t)(x) << MCG_C4_FCTRIM_SHIFT) & MCG_C4_FCTRIM_MASK))  /*!< Fast Internal Reference Clock Trim Setting */
#define MCG_C4_SCFTRIM              ((uint8_t)0x01) /*!< Slow Internal Reference Clock Fine Trim */

/***********  Bits definition for MCG_C5 register  **************/
#define MCG_C5_PLLCLKEN0            ((uint8_t)0x40) /*!< PLL Clock Enable */
#define MCG_C5_PLLSTEN0             ((uint8_t)0x20) /*!< PLL Stop Enable */
#define MCG_C5_PRDIV0_MASK          ((uint8_t)0x1F) /*!< PLL External Reference Divider (mask) */
#define MCG_C5_PRDIV0(x)            ((uint8_t)((uint8_t)(x) & MCG_C5_PRDIV0_MASK))  /*!< PLL External Reference Divider */

/***********  Bits definition for MCG_C6 register  **************/
#define MCG_C6_LOLIE0               ((uint8_t)0x80) /*!< Loss of Lock Interrupt Enable */
#define MCG_C6_PLLS                 ((uint8_t)0x40) /*!< PLL Select */
#define MCG_C6_CME0                 ((uint8_t)0x20) /*!< Clock Monitor Enable */
#define MCG_C6_VDIV0_MASK           ((uint8_t)0x1F) /*!< VCO 0 Divider (mask) */
#define MCG_C6_VDIV0(x)             ((uint8_t)((uint8_t)(x) & MCG_C6_VDIV0_MASK))  /*!< VCO 0 Divider */

/************  Bits definition for MCG_S register  **************/
#define MCG_S_LOLS                  ((uint8_t)0x80) /*!< Loss of Lock Status */
#define MCG_S_LOCK0                 ((uint8_t)0x40) /*!< Lock Status */
#define MCG_S_PLLST                 ((uint8_t)0x20) /*!< PLL Select Status */
#define MCG_S_IREFST                ((uint8_t)0x10) /*!< Internal Reference Status */
#define MCG_S_CLKST_SHIFT           2               /*!< Clock Mode Status (shift) */
#define MCG_S_CLKST_MASK            ((uint8_t)((uint8_t)0x3 << MCG_S_CLKST_SHIFT))  /*!< Clock Mode Status (mask) */
#define MCG_S_CLKST(x)              ((uint8_t)(((uint8_t)(x) << MCG_S_CLKST_SHIFT) & MCG_S_CLKST_MASK))  /*!< Clock Mode Status */
#define MCG_S_CLKST_FLL             MCG_S_CLKST(0)   /*!< Output of the FLL is selected */
#define MCG_S_CLKST_IRCLK           MCG_S_CLKST(1)   /*!< Internal reference clock is selected */
#define MCG_S_CLKST_ERCLK           MCG_S_CLKST(2)   /*!< External reference clock is selected */
#define MCG_S_CLKST_PLL             MCG_S_CLKST(3)   /*!< Output of the PLL is selected */
#define MCG_S_OSCINIT0              ((uint8_t)0x02)  /*!< OSC Initialization */
#define MCG_S_IRCST                 ((uint8_t)0x01)  /*!< Internal Reference Clock Status */

/************  Bits definition for MCG_SC register  **************/
#define MCG_SC_ATME                 ((uint8_t)0x80)  /*!< Automatic Trim Machine Enable */
#define MCG_SC_ATMS                 ((uint8_t)0x40)  /*!< Automatic Trim Machine Select */
#define MCG_SC_ATMF                 ((uint8_t)0x20)  /*!< Automatic Trim Machine Fail Flag */
#define MCG_SC_FLTPRSRV             ((uint8_t)0x10)  /*!< FLL Filter Preserve Enable */
#define MCG_SC_FCRDIV_SHIFT         1                /*!< Fast Clock Internal Reference Divider (shift) */
#define MCG_SC_FCRDIV_MASK          ((uint8_t)((uint8_t)0x7 << MCG_SC_FCRDIV_SHIFT))  /*!< Fast Clock Internal Reference Divider (mask) */
#define MCG_SC_FCRDIV(x)            ((uint8_t)(((uint8_t)(x) << MCG_SC_FCRDIV_SHIFT) & MCG_SC_FCRDIV_MASK))  /*!< Fast Clock Internal Reference Divider */
#define MCG_SC_FCRDIV_DIV1          MCG_SC_FCRDIV(0)  /*!< Divide Factor is 1 */
#define MCG_SC_FCRDIV_DIV2          MCG_SC_FCRDIV(1)  /*!< Divide Factor is 2 */
#define MCG_SC_FCRDIV_DIV4          MCG_SC_FCRDIV(2)  /*!< Divide Factor is 4 */
#define MCG_SC_FCRDIV_DIV8          MCG_SC_FCRDIV(3)  /*!< Divide Factor is 8 */
#define MCG_SC_FCRDIV_DIV16         MCG_SC_FCRDIV(4)  /*!< Divide Factor is 16 */
#define MCG_SC_FCRDIV_DIV32         MCG_SC_FCRDIV(5)  /*!< Divide Factor is 32 */
#define MCG_SC_FCRDIV_DIV64         MCG_SC_FCRDIV(6)  /*!< Divide Factor is 64 */
#define MCG_SC_FCRDIV_DIV128        MCG_SC_FCRDIV(7)  /*!< Divide Factor is 128 */
#define MCG_SC_LOCS0                ((uint8_t)0x01)   /*!< OSC0 Loss of Clock Status */

/************  Bits definition for MCG_C7 register  **************/
#define MCG_C7_OSCSEL               ((uint8_t)0x01)   /*!< MCG OSC Clock Select */

/************  Bits definition for MCG_C8 register  **************/
#define MCG_C8_LOCRE1               ((uint8_t)0x80)   /*!< PLL Loss of Clock Reset Enable */
#define MCG_C8_LOLRE                ((uint8_t)0x40)   /*!< PLL Loss of Lock Reset Enable */
#define MCG_C8_CME1                 ((uint8_t)0x20)   /*!< PLL Clock Monitor Enable */
#define MCG_C8_LOCS1                ((uint8_t)0x01)   /*!< RTC Loss of Clock Status */

/****************************************************************/
/*                                                              */
/*             Serial Peripheral Interface (SPI)                */
/*                                                              */
/****************************************************************/

/***********  Bits definition for SPIx_MCR register  *************/
#define SPIx_MCR_MSTR            ((uint32_t)0x80000000)      // Master/Slave Mode Select
#define SPIx_MCR_CONT_SCKE       ((uint32_t)0x40000000)      // Continuous SCK Enable
#define SPIx_MCR_DCONF(n)        (((n) & 3) << 28)           // DSPI Configuration
#define SPIx_MCR_FRZ             ((uint32_t)0x08000000)      // Freeze
#define SPIx_MCR_MTFE            ((uint32_t)0x04000000)      // Modified Timing Format Enable
#define SPIx_MCR_ROOE            ((uint32_t)0x01000000)      // Receive FIFO Overflow Overwrite Enable
#define SPIx_MCR_PCSIS(n)        (((n) & 0x1F) << 16)        // Peripheral Chip Select x Inactive State
#define SPIx_MCR_DOZE            ((uint32_t)0x00008000)      // Doze Enable
#define SPIx_MCR_MDIS            ((uint32_t)0x00004000)      // Module Disable
#define SPIx_MCR_DIS_TXF         ((uint32_t)0x00002000)      // Disable Transmit FIFO
#define SPIx_MCR_DIS_RXF         ((uint32_t)0x00001000)      // Disable Receive FIFO
#define SPIx_MCR_CLR_TXF         ((uint32_t)0x00000800)      // Clear the TX FIFO and counter
#define SPIx_MCR_CLR_RXF         ((uint32_t)0x00000400)      // Clear the RX FIFO and counter
#define SPIx_MCR_SMPL_PT(n)      (((n) & 3) << 8)            // Sample Point
#define SPIx_MCR_HALT            ((uint32_t)0x00000001)      // Halt

/***********  Bits definition for SPIx_TCR register  *************/
#define SPIx_TCR_TCNT(n)         (((n) & 0xffff) << 16)      // DSPI Transfer Count Register

/***********  Bits definition for SPIx_CTARn register  *************/
#define SPIx_CTARn_DBR            ((uint32_t)0x80000000)     // Double Baud Rate
#define SPIx_CTARn_FMSZ_SHIFT     27                         // Frame Size Shift
#define SPIx_CTARn_FMSZ_MASK      0xF                        // Frame Size Mask
#define SPIx_CTARn_FMSZ(n)        (((n) & 15) << 27)         // Frame Size (+1)
#define SPIx_CTARn_CPOL           ((uint32_t)0x04000000)     // Clock Polarity
#define SPIx_CTARn_CPHA           ((uint32_t)0x02000000)     // Clock Phase
#define SPIx_CTARn_LSBFE          ((uint32_t)0x01000000)     // LSB First
#define SPIx_CTARn_PCSSCK(n)      (((n) & 3) << 22)          // PCS to SCK Delay Prescaler
#define SPIx_CTARn_PASC(n)        (((n) & 3) << 20)          // After SCK Delay Prescaler
#define SPIx_CTARn_PDT(n)         (((n) & 3) << 18)          // Delay after Transfer Prescaler
#define SPIx_CTARn_PBR(n)         (((n) & 3) << 16)          // Baud Rate Prescaler
#define SPIx_CTARn_CSSCK(n)       (((n) & 15) << 12)         // PCS to SCK Delay Scaler
#define SPIx_CTARn_ASC(n)         (((n) & 15) << 8)          // After SCK Delay Scaler
#define SPIx_CTARn_DT(n)          (((n) & 15) << 4)          // Delay After Transfer Scaler
#define SPIx_CTARn_BR(n)          (((n) & 15) << 0)          // Baud Rate Scaler


/***********  Bits definition for SPIx_CTARn_SLAVE register  *************/
#define SPIx_CTARn_SLAVE_FMSZ(n)  (((n) & 15) << 27)         // Frame Size (+1)
#define SPIx_CTARn_SLAVE_CPOL     ((uint32_t)0x04000000)     // Clock Polarity
#define SPIx_CTARn_SLAVE_CPHA     ((uint32_t)0x02000000)     // Clock Phase

/***********  Bits definition for SPIx_SR register  *************/
#define SPIx_SR_TCF               ((uint32_t)0x80000000)     // Transfer Complete Flag
#define SPIx_SR_TXRXS             ((uint32_t)0x40000000)     // TX and RX Status
#define SPIx_SR_EOQF              ((uint32_t)0x10000000)     // End of Queue Flag
#define SPIx_SR_TFUF              ((uint32_t)0x08000000)     // Transmit FIFO Underflow Flag
#define SPIx_SR_TFFF              ((uint32_t)0x02000000)     // Transmit FIFO Fill Flag
#define SPIx_SR_RFOF              ((uint32_t)0x00080000)     // Receive FIFO Overflow Flag
#define SPIx_SR_RFDF              ((uint32_t)0x00020000)     // Receive FIFO Drain Flag
#define SPIx_SR_TXCTR             (((n) & 15) << 12)         // TX FIFO Counter
#define SPIx_SR_TXNXPTR           (((n) & 15) << 8)          // Transmit Next Pointer
#define SPIx_SR_RXCTR             (((n) & 15) << 4)          // RX FIFO Counter
#define SPIx_SR_POPNXTPTR         ((n) & 15)                 // POP Next Pointer

/***********  Bits definition for SPIx_SR register  *************/
#define SPIx_RSER_TCF_RE         ((uint32_t)0x80000000)      // Transmission Complete Request Enable
#define SPIx_RSER_EOQF_RE        ((uint32_t)0x10000000)      // DSPI Finished Request Request Enable
#define SPIx_RSER_TFUF_RE        ((uint32_t)0x08000000)      // Transmit FIFO Underflow Request Enable
#define SPIx_RSER_TFFF_RE        ((uint32_t)0x02000000)      // Transmit FIFO Fill Request Enable
#define SPIx_RSER_TFFF_DIRS      ((uint32_t)0x01000000)      // Transmit FIFO FIll Dma or Interrupt Request Select
#define SPIx_RSER_RFOF_RE        ((uint32_t)0x00080000)      // Receive FIFO Overflow Request Enable
#define SPIx_RSER_RFDF_RE        ((uint32_t)0x00020000)      // Receive FIFO Drain Request Enable
#define SPIx_RSER_RFDF_DIRS      ((uint32_t)0x00010000)      // Receive FIFO Drain DMA or Interrupt Request Select

/***********  Bits definition for SPIx_PUSHR register  *************/
#define SPIx_PUSHR_CONT          ((uint32_t)0x80000000)      // Continuous Peripheral Chip Select Enable
#define SPIx_PUSHR_CTAS(n)       (((n) & 7) << 28)           // Clock and Transfer Attributes Select
#define SPIx_PUSHR_EOQ           ((uint32_t)0x08000000)      // End Of Queue
#define SPIx_PUSHR_CTCNT         ((uint32_t)0x04000000)      // Clear Transfer Counter
#define SPIx_PUSHR_PCS(n)        (((n) & 31) << 16)          // Peripheral Chip Select
#define SPIx_PUSHR_TXDATA(n)     ((n) & 0xffff)              // Transmit Data

/***********  Bits definition for SPIx_PUSHR_SLAVE register  *************/
#define SPIx_PUSHR_SLAVE_TXDATA(n) (((n) & 0xffff) << 0)     // Transmit Data in slave mode

/***********  Bits definition for SPIx_POPR register  *************/
#define SPIx_POPR_RXDATA(n)      (((n) & 0xffff) << 16)      // Received Data

/***********  Bits definition for SPIx_TXFRn register  *************/
#define SPIx_TXFRn_TXCMD_TXDATA  (((n) & 0xffff) << 16)      // Transmit Command (in master mode)
#define SPIx_TXFRn_TXDATA(n)     (((n) & 0xffff) << 0)       // Transmit Data

/***********  Bits definition for SPIx_RXFRn register  *************/
#define SPIx_RXFRn_RXDATA(n)     (((n) & 0xffff) << 0)       // Receive Data

/****************************************************************/
/*                                                              */
/*             Inter-Integrated Circuit (I2C)                   */
/*                                                              */
/****************************************************************/
/***********  Bits definition for I2Cx_A1 register  *************/
#define I2Cx_A1_AD                   ((uint8_t)0xFE)    /*!< Address [7:1] */

#define I2Cx_A1_AD_SHIT              1

/***********  Bits definition for I2Cx_F register  **************/
#define I2Cx_F_MULT                  ((uint8_t)0xC0)    /*!< Multiplier factor */
#define I2Cx_F_ICR                   ((uint8_t)0x3F)    /*!< Clock rate */

#define I2Cx_F_MULT_SHIFT            5

/***********  Bits definition for I2Cx_C1 register  *************/
#define I2Cx_C1_IICEN                ((uint8_t)0x80)    /*!< I2C Enable */
#define I2Cx_C1_IICIE                ((uint8_t)0x40)    /*!< I2C Interrupt Enable */
#define I2Cx_C1_MST                  ((uint8_t)0x20)    /*!< Master Mode Select */
#define I2Cx_C1_TX                   ((uint8_t)0x10)    /*!< Transmit Mode Select */
#define I2Cx_C1_TXAK                 ((uint8_t)0x08)    /*!< Transmit Acknowledge Enable */
#define I2Cx_C1_RSTA                 ((uint8_t)0x04)    /*!< Repeat START */
#define I2Cx_C1_WUEN                 ((uint8_t)0x02)    /*!< Wakeup Enable */
#define I2Cx_C1_DMAEN                ((uint8_t)0x01)    /*!< DMA Enable */

/***********  Bits definition for I2Cx_S register  **************/
#define I2Cx_S_TCF                   ((uint8_t)0x80)    /*!< Transfer Complete Flag */
#define I2Cx_S_IAAS                  ((uint8_t)0x40)    /*!< Addressed As A Slave */
#define I2Cx_S_BUSY                  ((uint8_t)0x20)    /*!< Bus Busy */
#define I2Cx_S_ARBL                  ((uint8_t)0x10)    /*!< Arbitration Lost */
#define I2Cx_S_RAM                   ((uint8_t)0x08)    /*!< Range Address Match */
#define I2Cx_S_SRW                   ((uint8_t)0x04)    /*!< Slave Read/Write */
#define I2Cx_S_IICIF                 ((uint8_t)0x02)    /*!< Interrupt Flag */
#define I2Cx_S_RXAK                  ((uint8_t)0x01)    /*!< Receive Acknowledge */

/***********  Bits definition for I2Cx_D register  **************/
#define I2Cx_D_DATA                  ((uint8_t)0xFF)    /*!< Data */

/***********  Bits definition for I2Cx_C2 register  *************/
#define I2Cx_C2_GCAEN                ((uint8_t)0x80)    /*!< General Call Address Enable */
#define I2Cx_C2_ADEXT                ((uint8_t)0x40)    /*!< Address Extension */
#define I2Cx_C2_HDRS                 ((uint8_t)0x20)    /*!< High Drive Select */
#define I2Cx_C2_SBRC                 ((uint8_t)0x10)    /*!< Slave Baud Rate Control */
#define I2Cx_C2_RMEN                 ((uint8_t)0x08)    /*!< Range Address Matching Enable */
#define I2Cx_C2_AD_10_8              ((uint8_t)0x03)    /*!< Slave Address [10:8] */

/***********  Bits definition for I2Cx_FLT register  ************/
#define I2Cx_FLT_SHEN                ((uint8_t)0x80)    /*!< Stop Hold Enable */
#define I2Cx_FLT_STOPF               ((uint8_t)0x40)    /*!< I2C Bus Stop Detect Flag */
#define I2Cx_FLT_STOPIE              ((uint8_t)0x20)    /*!< I2C Bus Stop Interrupt Enable */
#define I2Cx_FLT_FLT                 ((uint8_t)0x1F)    /*!< I2C Programmable Filter Factor */

/***********  Bits definition for I2Cx_RA register  *************/
#define I2Cx_RA_RAD                  ((uint8_t)0xFE)    /*!< Range Slave Address */

#define I2Cx_RA_RAD_SHIFT            1

/***********  Bits definition for I2Cx_SMB register  ************/
#define I2Cx_SMB_FACK                ((uint8_t)0x80)    /*!< Fast NACK/ACK Enable */
#define I2Cx_SMB_ALERTEN             ((uint8_t)0x40)    /*!< SMBus Alert Response Address Enable */
#define I2Cx_SMB_SIICAEN             ((uint8_t)0x20)    /*!< Second I2C Address Enable */
#define I2Cx_SMB_TCKSEL              ((uint8_t)0x10)    /*!< Timeout Counter Clock Select */
#define I2Cx_SMB_SLTF                ((uint8_t)0x08)    /*!< SCL Low Timeout Flag */
#define I2Cx_SMB_SHTF1               ((uint8_t)0x04)    /*!< SCL High Timeout Flag 1 */
#define I2Cx_SMB_SHTF2               ((uint8_t)0x02)    /*!< SCL High Timeout Flag 2 */
#define I2Cx_SMB_SHTF2IE             ((uint8_t)0x01)    /*!< SHTF2 Interrupt Enable */

/***********  Bits definition for I2Cx_A2 register  *************/
#define I2Cx_A2_SAD                  ((uint8_t)0xFE)    /*!< SMBus Address */

#define I2Cx_A2_SAD_SHIFT            1

/***********  Bits definition for I2Cx_SLTH register  ***********/
#define I2Cx_SLTH_SSLT               ((uint8_t)0xFF)    /*!< MSB of SCL low timeout value */

/***********  Bits definition for I2Cx_SLTL register  ***********/
#define I2Cx_SLTL_SSLT               ((uint8_t)0xFF)    /*!< LSB of SCL low timeout value */

/****************************************************************/
/*                                                              */
/*     Universal Asynchronous Receiver/Transmitter (UART)       */
/*                                                              */
/****************************************************************/
/*********  Bits definition for UARTx_BDH register  *************/
#define UARTx_BDH_LBKDIE             ((uint8_t)0x80)    /*!< LIN Break Detect Interrupt Enable */
#define UARTx_BDH_RXEDGIE            ((uint8_t)0x40)    /*!< RxD Input Active Edge Interrupt Enable */
#define UARTx_BDH_SBR_MASK           ((uint8_t)0x1F)
#define UARTx_BDH_SBR(x)             ((uint8_t)((uint8_t)(x) & UARTx_BDH_SBR_MASK))  /*!< Baud Rate Modulo Divisor */

/*********  Bits definition for UARTx_BDL register  *************/
#define UARTx_BDL_SBR_MASK           ((uint8_t)0xFF)    /*!< Baud Rate Modulo Divisor */

/*********  Bits definition for UARTx_C1 register  **************/
#define UARTx_C1_LOOPS               ((uint8_t)0x80)    /*!< Loop Mode Select */
#define UARTx_C1_DOZEEN              ((uint8_t)0x40)    /*!< Doze Enable */
#define UARTx_C1_UARTSWAI            ((uint8_t)0x40)    /*!< UART Stops in Wait Mode */
#define UARTx_C1_RSRC                ((uint8_t)0x20)    /*!< Receiver Source Select */
#define UARTx_C1_M                   ((uint8_t)0x10)    /*!< 9-Bit or 8-Bit Mode Select */
#define UARTx_C1_WAKE                ((uint8_t)0x08)    /*!< Receiver Wakeup Method Select */
#define UARTx_C1_ILT                 ((uint8_t)0x04)    /*!< Idle Line Type Select */
#define UARTx_C1_PE                  ((uint8_t)0x02)    /*!< Parity Enable */
#define UARTx_C1_PT                  ((uint8_t)0x01)    /*!< Parity Type */

/*********  Bits definition for UARTx_C2 register  **************/
#define UARTx_C2_TIE                 ((uint8_t)0x80)    /*!< Transmit Interrupt Enable for TDRE */
#define UARTx_C2_TCIE                ((uint8_t)0x40)    /*!< Transmission Complete Interrupt Enable for TC */
#define UARTx_C2_RIE                 ((uint8_t)0x20)    /*!< Receiver Interrupt Enable for RDRF */
#define UARTx_C2_ILIE                ((uint8_t)0x10)    /*!< Idle Line Interrupt Enable for IDLE */
#define UARTx_C2_TE                  ((uint8_t)0x08)    /*!< Transmitter Enable */
#define UARTx_C2_RE                  ((uint8_t)0x04)    /*!< Receiver Enable */
#define UARTx_C2_RWU                 ((uint8_t)0x02)    /*!< Receiver Wakeup Control */
#define UARTx_C2_SBK                 ((uint8_t)0x01)    /*!< Send Break */

/*********  Bits definition for UARTx_S1 register  **************/
#define UARTx_S1_TDRE                ((uint8_t)0x80)    /*!< Transmit Data Register Empty Flag */
#define UARTx_S1_TC                  ((uint8_t)0x40)    /*!< Transmission Complete Flag */
#define UARTx_S1_RDRF                ((uint8_t)0x20)    /*!< Receiver Data Register Full Flag */
#define UARTx_S1_IDLE                ((uint8_t)0x10)    /*!< Idle Line Flag */
#define UARTx_S1_OR                  ((uint8_t)0x08)    /*!< Receiver Overrun Flag */
#define UARTx_S1_NF                  ((uint8_t)0x04)    /*!< Noise Flag */
#define UARTx_S1_FE                  ((uint8_t)0x02)    /*!< Framing Error Flag */
#define UARTx_S1_PF                  ((uint8_t)0x01)    /*!< Parity Error Flag */

/*********  Bits definition for UARTx_S2 register  **************/
#define UARTx_S2_LBKDIF              ((uint8_t)0x80)    /*!< LIN Break Detect Interrupt Flag */
#define UARTx_S2_RXEDGIF             ((uint8_t)0x40)    /*!< UART_RX Pin Active Edge Interrupt Flag */
#define UARTx_S2_MSBF                ((uint8_t)0x20)    /*!< MSB First */
#define UARTx_S2_RXINV               ((uint8_t)0x10)    /*!< Receive Data Inversion */
#define UARTx_S2_RWUID               ((uint8_t)0x08)    /*!< Receive Wake Up Idle Detect */
#define UARTx_S2_BRK13               ((uint8_t)0x04)    /*!< Break Character Generation Length */
#define UARTx_S2_LBKDE               ((uint8_t)0x02)    /*!< LIN Break Detect Enable */
#define UARTx_S2_RAF                 ((uint8_t)0x01)    /*!< Receiver Active Flag */

/*********  Bits definition for UARTx_C3 register  **************/
#define UARTx_C3_R8                  ((uint8_t)0x80)    /*!< Ninth Data Bit for Receiver */
#define UARTx_C3_T8                  ((uint8_t)0x40)    /*!< Ninth Data Bit for Transmitter */
#define UARTx_C3_TXDIR               ((uint8_t)0x20)    /*!< UART_TX Pin Direction in Single-Wire Mode */
#define UARTx_C3_TXINV               ((uint8_t)0x10)    /*!< Transmit Data Inversion */
#define UARTx_C3_ORIE                ((uint8_t)0x08)    /*!< Overrun Interrupt Enable */
#define UARTx_C3_NEIE                ((uint8_t)0x04)    /*!< Noise Error Interrupt Enable */
#define UARTx_C3_FEIE                ((uint8_t)0x02)    /*!< Framing Error Interrupt Enable */
#define UARTx_C3_PEIE                ((uint8_t)0x01)    /*!< Parity Error Interrupt Enable */

/*********  Bits definition for UARTx_D register  ***************/
#define UARTx_D_R7T7                 ((uint8_t)0x80)    /*!< Read receive data buffer 7 or write transmit data buffer 7 */
#define UARTx_D_R6T6                 ((uint8_t)0x40)    /*!< Read receive data buffer 6 or write transmit data buffer 6 */
#define UARTx_D_R5T5                 ((uint8_t)0x20)    /*!< Read receive data buffer 5 or write transmit data buffer 5 */
#define UARTx_D_R4T4                 ((uint8_t)0x10)    /*!< Read receive data buffer 4 or write transmit data buffer 4 */
#define UARTx_D_R3T3                 ((uint8_t)0x08)    /*!< Read receive data buffer 3 or write transmit data buffer 3 */
#define UARTx_D_R2T2                 ((uint8_t)0x04)    /*!< Read receive data buffer 2 or write transmit data buffer 2 */
#define UARTx_D_R1T1                 ((uint8_t)0x02)    /*!< Read receive data buffer 1 or write transmit data buffer 1 */
#define UARTx_D_R0T0                 ((uint8_t)0x01)    /*!< Read receive data buffer 0 or write transmit data buffer 0 */

/*********  Bits definition for UARTx_MA1 register  *************/
#define UARTx_MA1_MA                 ((uint8_t)0xFF)    /*!< Match Address */

/*********  Bits definition for UARTx_MA2 register  *************/
#define UARTx_MA2_MA                 ((uint8_t)0xFF)    /*!< Match Address */

/*********  Bits definition for UARTx_C4 register  **************/
#define UARTx_C4_MAEN1               ((uint8_t)0x80)    /*!< Match Address Mode Enable 1 */
#define UARTx_C4_MAEN2               ((uint8_t)0x40)    /*!< Match Address Mode Enable 2 */
#define UARTx_C4_M10                 ((uint8_t)0x20)    /*!< 10-bit Mode Select */
#define UARTx_C4_BRFA_MASK           ((uint8_t)0x1F)
#define UARTx_C4_BRFA(x)             ((uint8_t)((uint8_t)(x) & UARTx_C4_BRFA_MASK))  /*!< Baud Rate Fine Adjust */

/*********  Bits definition for UARTx_C5 register  **************/
#define UARTx_C5_TDMAE               ((uint8_t)0x80)    /*!< Transmitter DMA Enable */
#define UARTx_C5_RDMAE               ((uint8_t)0x20)    /*!< Receiver Full DMA Enable */
#define UARTx_C5_BOTHEDGE            ((uint8_t)0x02)    /*!< Both Edge Sampling */
#define UARTx_C5_RESYNCDIS           ((uint8_t)0x01)    /*!< Resynchronization Disable */

/*******  Bits definition for UARTx_CFIFO register  ************/
#define UARTx_CFIFO_TXFLUSH          ((uint8_t)0x80)    /*!< Transmit FIFO/Buffer Flush */
#define UARTx_CFIFO_RXFLUSH          ((uint8_t)0x40)    /*!< Receive FIFO/Buffer Flush */
#define UARTx_CFIFO_RXOFE            ((uint8_t)0x04)    /*!< Receive FIFO Overflow Interrupt Enable */
#define UARTx_CFIFO_TXOFE            ((uint8_t)0x02)    /*!< Transmit FIFO Overflow Interrupt Enable */
#define UARTx_CFIFO_RXUFE            ((uint8_t)0x01)    /*!< Receive FIFO Underflow Interrupt Enable */

/*******  Bits definition for UARTx_PFIFO register  ************/
#define UARTx_PFIFO_TXFE             ((uint8_t)0x80)    /*!< Transmit FIFO Enable */
#define UARTx_PFIFO_TXFIFOSIZE_SHIFT 4
#define UARTx_PFIFO_TXFIFOSIZE_MASK  ((uint8_t)((uint8_t)0x7 << UARTx_PFIFO_TXFIFOSIZE_SHIFT))
#define UARTx_PFIFO_TXFIFOSIZE(x)    ((uint8_t)(((uint8_t)(x) << UARTx_PFIFO_TXFIFOSIZE_SHIFT) & UARTx_PFIFO_TXFIFOSIZE_MASK))  /*!< Transmit FIFO Buffer depth */
#define UARTx_PFIFO_RXFE             ((uint8_t)0x08)    /*!< Receive FIFOh */
#define UARTx_PFIFO_RXFIFOSIZE_SHIFT 0
#define UARTx_PFIFO_RXFIFOSIZE_MASK  ((uint8_t)((uint8_t)0x7 << UARTx_PFIFO_RXFIFOSIZE_SHIFT))
#define UARTx_PFIFO_RXFIFOSIZE(x)    ((uint8_t)(((uint8_t)(x) << UARTx_PFIFO_RXFIFOSIZE_SHIFT) & UARTx_PFIFO_RXFIFOSIZE_MASK))  /*!< Receive FIFO Buffer depth */

/****************************************************************/
/*                                                              */
/*             Power Management Controller (PMC)                */
/*                                                              */
/****************************************************************/
/*********  Bits definition for PMC_LVDSC1 register  *************/
#define PMC_LVDSC1_LVDF               ((uint8_t)0x80)   /*!< Low-Voltage Detect Flag */
#define PMC_LVDSC1_LVDACK             ((uint8_t)0x40)   /*!< Low-Voltage Detect Acknowledge */
#define PMC_LVDSC1_LVDIE              ((uint8_t)0x20)   /*!< Low-Voltage Detect Interrupt Enable */
#define PMC_LVDSC1_LVDRE              ((uint8_t)0x10)   /*!< Low-Voltage Detect Reset Enable */
#define PMC_LVDSC1_LVDV_MASK          ((uint8_t)0x3)    /*!< Low-Voltage Detect Voltage Select */
#define PMC_LVDSC1_LVDV_SHIFT         0
#define PMC_LVDSC1_LVDV(x)            (((uint8_t)(((uint8_t)(x))<<PMC_LVDSC1_LVDV_SHIFT))&PMC_LVDSC1_LVDV_MASK)
/*********  Bits definition for PMC_LVDSC1 register  *************/
#define PMC_LVDSC2_LVWF               ((uint8_t)0x80)   /*!< Low-Voltage Warning Flag */
#define PMC_LVDSC2_LVWACK             ((uint8_t)0x40)   /*!< Low-Voltage Warning Acknowledge */
#define PMC_LVDSC2_LVWIE              ((uint8_t)0x20)   /*!< Low-Voltage Warning Interrupt Enable */
#define PMC_LVDSC2_LVWV_MASK          0x3               /*!< Low-Voltage Warning Voltage Select */
#define PMC_LVDSC2_LVWV_SHIFT         0
#define PMC_LVDSC2_LVWV(x)            (((uint8_t)(((uint8_t)(x))<<PMC_LVDSC2_LVWV_SHIFT))&PMC_LVDSC2_LVWV_MASK)
/*********  Bits definition for PMC_REGSC register  *************/
#define PMC_REGSC_BGEN                ((uint8_t)0x10)   /*!< Bandgap Enable In VLPx Operation */
#define PMC_REGSC_ACKISO              ((uint8_t)0x8)    /*!< Acknowledge Isolation */
#define PMC_REGSC_REGONS              ((uint8_t)0x4)    /*!< Regulator In Run Regulation Status */
#define PMC_REGSC_BGBE                ((uint8_t)0x1)    /*!< Bandgap Buffer Enable */

/****************************************************************/
/*                                                              */
/*                         Watchdog                             */
/*                                                              */
/****************************************************************/
/********  Bits definition for WDOG_STCTRLH register  ***********/
#define WDOG_STCTRLH_DISTESTWDOG     ((uint16_t)0x4000)
#define WDOG_STCTRLH_BYTESEL_1_0     ((uint16_t)0x3000)
#define WDOG_STCTRLH_TESTSEL         ((uint16_t)0x0800)
#define WDOG_STCTRLH_TESTWDOG        ((uint16_t)0x0400)
#define WDOG_STCTRLH_WAITEN          ((uint16_t)0x0080)
#define WDOG_STCTRLH_STOPEN          ((uint16_t)0x0040)
#define WDOG_STCTRLH_DBGEN           ((uint16_t)0x0020)
#define WDOG_STCTRLH_ALLOWUPDATE     ((uint16_t)0x0010)
#define WDOG_STCTRLH_WINEN           ((uint16_t)0x0008)
#define WDOG_STCTRLH_IRQRSTEN        ((uint16_t)0x0004)
#define WDOG_STCTRLH_CLKSRC          ((uint16_t)0x0002)
#define WDOG_STCTRLH_WDOGEN          ((uint16_t)0x0001)

/********  Bits definition for WDOG_STCTRLL register  ***********/
#define WDOG_STCTRLL_INTFLG          ((uint16_t)0x8000)

/*********  Bits definition for WDOG_PRESC register  ************/
#define WDOG_PRESC_PRESCVAL          ((uint16_t)0x0700)

/****************************************************************/
/*                                                              */
/*                         USB OTG                              */
/*                                                              */
/****************************************************************/
/********  Bits definition for USBx_ISTAT register  *************/
#define USBx_ISTAT_STALL             ((uint8_t)0x80) /*!< Stall interrupt */
#define USBx_ISTAT_ATTACH            ((uint8_t)0x40) /*!< Attach interrupt */
#define USBx_ISTAT_RESUME            ((uint8_t)0x20) /*!< Signal remote wakeup on the bus */
#define USBx_ISTAT_SLEEP             ((uint8_t)0x10) /*!< Detected bus idle for 3ms */
#define USBx_ISTAT_TOKDNE            ((uint8_t)0x08) /*!< Completed processing of current token */
#define USBx_ISTAT_SOFTOK            ((uint8_t)0x04) /*!< Received start of frame */
#define USBx_ISTAT_ERROR             ((uint8_t)0x02) /*!< Error (must check ERRSTAT!) */
#define USBx_ISTAT_USBRST            ((uint8_t)0x01) /*!< USB reset detected */

/********  Bits definition for USBx_ERRSTAT register  ***********/
#define USBx_ERRSTAT_BTSERR          ((uint8_t)0x80) /*!< Bit stuff error detected */
#define USBx_ERRSTAT_DMAERR          ((uint8_t)0x20) /*!< DMA request was not given */
#define USBx_ERRSTAT_BTOERR          ((uint8_t)0x10) /*!< BUS turnaround timeout error */
#define USBx_ERRSTAT_DFN8            ((uint8_t)0x08) /*!< Received data not 8-bit sized */
#define USBx_ERRSTAT_CRC16           ((uint8_t)0x04) /*!< Packet with CRC16 error */
#define USBx_ERRSTAT_CRC5EOF         ((uint8_t)0x02) /*!< CRC5 (device) or EOF (host) error */
#define USBx_ERRSTAT_PIDERR          ((uint8_t)0x01) /*!< PID check field fail */

/******** Bits definition for USBx_CTL register *****************/
#define USBx_CTL_JSTATE              ((uint8_t)0x80) /*!< Live USB differential receiver JSTATE signal */
#define USBx_CTL_SE0                 ((uint8_t)0x40) /*!< Live USB single ended zero signal */
#define USBx_CTL_TXSUSPENDTOKENBUS   ((uint8_t)0x20) /*!<  */
#define USBx_CTL_RESET               ((uint8_t)0x10) /*!< Generates an USB reset signal (host mode) */
#define USBx_CTL_HOSTMODEEN          ((uint8_t)0x08) /*!< Operate in Host mode */
#define USBx_CTL_RESUME              ((uint8_t)0x04) /*!< Executes resume signaling */
#define USBx_CTL_ODDRST              ((uint8_t)0x02) /*!< Reset all BDT ODD ping/pong bits */
#define USBx_CTL_USBENSOFEN          ((uint8_t)0x01) /*!< USB Enable! */

/******** Bits definition for USBx_INTEN register ***************/
#define USBx_INTEN_STALLEN           ((uint8_t)0x80) /*!< STALL interrupt enable */
#define USBx_INTEN_ATTACHEN          ((uint8_t)0x40) /*!< ATTACH interrupt enable */
#define USBx_INTEN_RESUMEEN          ((uint8_t)0x20) /*!< RESUME interrupt enable */
#define USBx_INTEN_SLEEPEN           ((uint8_t)0x10) /*!< SLEEP interrupt enable */
#define USBx_INTEN_TOKDNEEN          ((uint8_t)0x08) /*!< TOKDNE interrupt enable */
#define USBx_INTEN_SOFTOKEN          ((uint8_t)0x04) /*!< SOFTOK interrupt enable */
#define USBx_INTEN_ERROREN           ((uint8_t)0x02) /*!< ERROR interrupt enable */
#define USBx_INTEN_USBRSTEN          ((uint8_t)0x01) /*!< USBRST interrupt enable */

/******** Bits definition for USBx_ENDPTn register **************/
#define USBx_ENDPTn_HOSTWOHUB        ((uint8_t)0x80)
#define USBx_ENDPTn_RETRYDIS         ((uint8_t)0x40)
#define USBx_ENDPTn_EPCTLDIS         ((uint8_t)0x10) /*!< Disables control transfers */
#define USBx_ENDPTn_EPRXEN           ((uint8_t)0x08) /*!< Enable RX transfers */
#define USBx_ENDPTn_EPTXEN           ((uint8_t)0x04) /*!< Enable TX transfers */
#define USBx_ENDPTn_EPSTALL          ((uint8_t)0x02) /*!< Endpoint is called and in STALL */
#define USBx_ENDPTn_EPHSHK           ((uint8_t)0x01) /*!< Enable handshaking during transaction */

/******** Bits definition for USBx_CTRL register ****************/
#define USBx_CTRL_SUSP               ((uint8_t)0x80) /*!< USB transceiver in suspend state */
#define USBx_CTRL_PDE                ((uint8_t)0x40) /*!< Enable weak pull-downs */

/******** Bits definition for USBx_USBTRC0 register *************/
#define USBx_USBTRC0_USBRESET        ((uint8_t)0x80) /*!< USB reset */
#define USBx_USBTRC0_USBRESMEN       ((uint8_t)0x20) /*!< Asynchronous resume interrupt enable */
#define USBx_USBTRC0_SYNC_DET        ((uint8_t)0x02) /*!< Synchronous USB interrupt detect */
#define USBx_USBTRC0_USB_RESUME_INT  ((uint8_t)0x01) /*!< USB asynchronous interrupt */

/******** Bits definition for USBx_CONTROL register *************/
#define USBx_CONTROL_DPPULLUPNONOTG  ((uint8_t)0x10) /*!< Control pull-ups in device mode */

#endif

/* ============================================================================ */
/* Copyright (c) 2016, Texas Instruments Incorporated                           */
/*  All rights reserved.                                                        */
/*                                                                              */
/*  Redistribution and use in source and binary forms, with or without          */
/*  modification, are permitted provided that the following conditions          */
/*  are met:                                                                    */
/*                                                                              */
/*  *  Redistributions of source code must retain the above copyright           */
/*     notice, this list of conditions and the following disclaimer.            */
/*                                                                              */
/*  *  Redistributions in binary form must reproduce the above copyright        */
/*     notice, this list of conditions and the following disclaimer in the      */
/*     documentation and/or other materials provided with the distribution.     */
/*                                                                              */
/*  *  Neither the name of Texas Instruments Incorporated nor the names of      */
/*     its contributors may be used to endorse or promote products derived      */
/*     from this software without specific prior written permission.            */
/*                                                                              */
/*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" */
/*  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,       */
/*  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR      */
/*  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR            */
/*  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,       */
/*  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,         */
/*  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; */
/*  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,    */
/*  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR     */
/*  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,              */
/*  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                          */
/* ============================================================================ */

/* This file supports MSP430FR5969 devices. */
/* Version: 1.198 */

/************************************************************
* STANDARD BITS
************************************************************/
/************************************************************
* STATUS REGISTER BITS
************************************************************/
/************************************************************
* PERIPHERAL FILE MAP
************************************************************/
/************************************************************
* ADC12_B
************************************************************/
PROVIDE(ADC12CTL0          = 0x0800);
PROVIDE(ADC12CTL0_L        = 0x0800);
PROVIDE(ADC12CTL0_H        = 0x0801);
PROVIDE(ADC12CTL1          = 0x0802);
PROVIDE(ADC12CTL1_L        = 0x0802);
PROVIDE(ADC12CTL1_H        = 0x0803);
PROVIDE(ADC12CTL2          = 0x0804);
PROVIDE(ADC12CTL2_L        = 0x0804);
PROVIDE(ADC12CTL2_H        = 0x0805);
PROVIDE(ADC12CTL3          = 0x0806);
PROVIDE(ADC12CTL3_L        = 0x0806);
PROVIDE(ADC12CTL3_H        = 0x0807);
PROVIDE(ADC12LO            = 0x0808);
PROVIDE(ADC12LO_L          = 0x0808);
PROVIDE(ADC12LO_H          = 0x0809);
PROVIDE(ADC12HI            = 0x080A);
PROVIDE(ADC12HI_L          = 0x080A);
PROVIDE(ADC12HI_H          = 0x080B);
PROVIDE(ADC12IFGR0         = 0x080C);
PROVIDE(ADC12IFGR0_L       = 0x080C);
PROVIDE(ADC12IFGR0_H       = 0x080D);
PROVIDE(ADC12IFGR1         = 0x080E);
PROVIDE(ADC12IFGR1_L       = 0x080E);
PROVIDE(ADC12IFGR1_H       = 0x080F);
PROVIDE(ADC12IFGR2         = 0x0810);
PROVIDE(ADC12IFGR2_L       = 0x0810);
PROVIDE(ADC12IFGR2_H       = 0x0811);
PROVIDE(ADC12IER0          = 0x0812);
PROVIDE(ADC12IER0_L        = 0x0812);
PROVIDE(ADC12IER0_H        = 0x0813);
PROVIDE(ADC12IER1          = 0x0814);
PROVIDE(ADC12IER1_L        = 0x0814);
PROVIDE(ADC12IER1_H        = 0x0815);
PROVIDE(ADC12IER2          = 0x0816);
PROVIDE(ADC12IER2_L        = 0x0816);
PROVIDE(ADC12IER2_H        = 0x0817);
PROVIDE(ADC12IV            = 0x0818);
PROVIDE(ADC12IV_L          = 0x0818);
PROVIDE(ADC12IV_H          = 0x0819);
PROVIDE(ADC12MCTL0         = 0x0820);
PROVIDE(ADC12MCTL0_L       = 0x0820);
PROVIDE(ADC12MCTL0_H       = 0x0821);
PROVIDE(ADC12MCTL1         = 0x0822);
PROVIDE(ADC12MCTL1_L       = 0x0822);
PROVIDE(ADC12MCTL1_H       = 0x0823);
PROVIDE(ADC12MCTL2         = 0x0824);
PROVIDE(ADC12MCTL2_L       = 0x0824);
PROVIDE(ADC12MCTL2_H       = 0x0825);
PROVIDE(ADC12MCTL3         = 0x0826);
PROVIDE(ADC12MCTL3_L       = 0x0826);
PROVIDE(ADC12MCTL3_H       = 0x0827);
PROVIDE(ADC12MCTL4         = 0x0828);
PROVIDE(ADC12MCTL4_L       = 0x0828);
PROVIDE(ADC12MCTL4_H       = 0x0829);
PROVIDE(ADC12MCTL5         = 0x082A);
PROVIDE(ADC12MCTL5_L       = 0x082A);
PROVIDE(ADC12MCTL5_H       = 0x082B);
PROVIDE(ADC12MCTL6         = 0x082C);
PROVIDE(ADC12MCTL6_L       = 0x082C);
PROVIDE(ADC12MCTL6_H       = 0x082D);
PROVIDE(ADC12MCTL7         = 0x082E);
PROVIDE(ADC12MCTL7_L       = 0x082E);
PROVIDE(ADC12MCTL7_H       = 0x082F);
PROVIDE(ADC12MCTL8         = 0x0830);
PROVIDE(ADC12MCTL8_L       = 0x0830);
PROVIDE(ADC12MCTL8_H       = 0x0831);
PROVIDE(ADC12MCTL9         = 0x0832);
PROVIDE(ADC12MCTL9_L       = 0x0832);
PROVIDE(ADC12MCTL9_H       = 0x0833);
PROVIDE(ADC12MCTL10        = 0x0834);
PROVIDE(ADC12MCTL10_L      = 0x0834);
PROVIDE(ADC12MCTL10_H      = 0x0835);
PROVIDE(ADC12MCTL11        = 0x0836);
PROVIDE(ADC12MCTL11_L      = 0x0836);
PROVIDE(ADC12MCTL11_H      = 0x0837);
PROVIDE(ADC12MCTL12        = 0x0838);
PROVIDE(ADC12MCTL12_L      = 0x0838);
PROVIDE(ADC12MCTL12_H      = 0x0839);
PROVIDE(ADC12MCTL13        = 0x083A);
PROVIDE(ADC12MCTL13_L      = 0x083A);
PROVIDE(ADC12MCTL13_H      = 0x083B);
PROVIDE(ADC12MCTL14        = 0x083C);
PROVIDE(ADC12MCTL14_L      = 0x083C);
PROVIDE(ADC12MCTL14_H      = 0x083D);
PROVIDE(ADC12MCTL15        = 0x083E);
PROVIDE(ADC12MCTL15_L      = 0x083E);
PROVIDE(ADC12MCTL15_H      = 0x083F);
PROVIDE(ADC12MCTL16        = 0x0840);
PROVIDE(ADC12MCTL16_L      = 0x0840);
PROVIDE(ADC12MCTL16_H      = 0x0841);
PROVIDE(ADC12MCTL17        = 0x0842);
PROVIDE(ADC12MCTL17_L      = 0x0842);
PROVIDE(ADC12MCTL17_H      = 0x0843);
PROVIDE(ADC12MCTL18        = 0x0844);
PROVIDE(ADC12MCTL18_L      = 0x0844);
PROVIDE(ADC12MCTL18_H      = 0x0845);
PROVIDE(ADC12MCTL19        = 0x0846);
PROVIDE(ADC12MCTL19_L      = 0x0846);
PROVIDE(ADC12MCTL19_H      = 0x0847);
PROVIDE(ADC12MCTL20        = 0x0848);
PROVIDE(ADC12MCTL20_L      = 0x0848);
PROVIDE(ADC12MCTL20_H      = 0x0849);
PROVIDE(ADC12MCTL21        = 0x084A);
PROVIDE(ADC12MCTL21_L      = 0x084A);
PROVIDE(ADC12MCTL21_H      = 0x084B);
PROVIDE(ADC12MCTL22        = 0x084C);
PROVIDE(ADC12MCTL22_L      = 0x084C);
PROVIDE(ADC12MCTL22_H      = 0x084D);
PROVIDE(ADC12MCTL23        = 0x084E);
PROVIDE(ADC12MCTL23_L      = 0x084E);
PROVIDE(ADC12MCTL23_H      = 0x084F);
PROVIDE(ADC12MCTL24        = 0x0850);
PROVIDE(ADC12MCTL24_L      = 0x0850);
PROVIDE(ADC12MCTL24_H      = 0x0851);
PROVIDE(ADC12MCTL25        = 0x0852);
PROVIDE(ADC12MCTL25_L      = 0x0852);
PROVIDE(ADC12MCTL25_H      = 0x0853);
PROVIDE(ADC12MCTL26        = 0x0854);
PROVIDE(ADC12MCTL26_L      = 0x0854);
PROVIDE(ADC12MCTL26_H      = 0x0855);
PROVIDE(ADC12MCTL27        = 0x0856);
PROVIDE(ADC12MCTL27_L      = 0x0856);
PROVIDE(ADC12MCTL27_H      = 0x0857);
PROVIDE(ADC12MCTL28        = 0x0858);
PROVIDE(ADC12MCTL28_L      = 0x0858);
PROVIDE(ADC12MCTL28_H      = 0x0859);
PROVIDE(ADC12MCTL29        = 0x085A);
PROVIDE(ADC12MCTL29_L      = 0x085A);
PROVIDE(ADC12MCTL29_H      = 0x085B);
PROVIDE(ADC12MCTL30        = 0x085C);
PROVIDE(ADC12MCTL30_L      = 0x085C);
PROVIDE(ADC12MCTL30_H      = 0x085D);
PROVIDE(ADC12MCTL31        = 0x085E);
PROVIDE(ADC12MCTL31_L      = 0x085E);
PROVIDE(ADC12MCTL31_H      = 0x085F);
PROVIDE(ADC12MEM0          = 0x0860);
PROVIDE(ADC12MEM0_L        = 0x0860);
PROVIDE(ADC12MEM0_H        = 0x0861);
PROVIDE(ADC12MEM1          = 0x0862);
PROVIDE(ADC12MEM1_L        = 0x0862);
PROVIDE(ADC12MEM1_H        = 0x0863);
PROVIDE(ADC12MEM2          = 0x0864);
PROVIDE(ADC12MEM2_L        = 0x0864);
PROVIDE(ADC12MEM2_H        = 0x0865);
PROVIDE(ADC12MEM3          = 0x0866);
PROVIDE(ADC12MEM3_L        = 0x0866);
PROVIDE(ADC12MEM3_H        = 0x0867);
PROVIDE(ADC12MEM4          = 0x0868);
PROVIDE(ADC12MEM4_L        = 0x0868);
PROVIDE(ADC12MEM4_H        = 0x0869);
PROVIDE(ADC12MEM5          = 0x086A);
PROVIDE(ADC12MEM5_L        = 0x086A);
PROVIDE(ADC12MEM5_H        = 0x086B);
PROVIDE(ADC12MEM6          = 0x086C);
PROVIDE(ADC12MEM6_L        = 0x086C);
PROVIDE(ADC12MEM6_H        = 0x086D);
PROVIDE(ADC12MEM7          = 0x086E);
PROVIDE(ADC12MEM7_L        = 0x086E);
PROVIDE(ADC12MEM7_H        = 0x086F);
PROVIDE(ADC12MEM8          = 0x0870);
PROVIDE(ADC12MEM8_L        = 0x0870);
PROVIDE(ADC12MEM8_H        = 0x0871);
PROVIDE(ADC12MEM9          = 0x0872);
PROVIDE(ADC12MEM9_L        = 0x0872);
PROVIDE(ADC12MEM9_H        = 0x0873);
PROVIDE(ADC12MEM10         = 0x0874);
PROVIDE(ADC12MEM10_L       = 0x0874);
PROVIDE(ADC12MEM10_H       = 0x0875);
PROVIDE(ADC12MEM11         = 0x0876);
PROVIDE(ADC12MEM11_L       = 0x0876);
PROVIDE(ADC12MEM11_H       = 0x0877);
PROVIDE(ADC12MEM12         = 0x0878);
PROVIDE(ADC12MEM12_L       = 0x0878);
PROVIDE(ADC12MEM12_H       = 0x0879);
PROVIDE(ADC12MEM13         = 0x087A);
PROVIDE(ADC12MEM13_L       = 0x087A);
PROVIDE(ADC12MEM13_H       = 0x087B);
PROVIDE(ADC12MEM14         = 0x087C);
PROVIDE(ADC12MEM14_L       = 0x087C);
PROVIDE(ADC12MEM14_H       = 0x087D);
PROVIDE(ADC12MEM15         = 0x087E);
PROVIDE(ADC12MEM15_L       = 0x087E);
PROVIDE(ADC12MEM15_H       = 0x087F);
PROVIDE(ADC12MEM16         = 0x0880);
PROVIDE(ADC12MEM16_L       = 0x0880);
PROVIDE(ADC12MEM16_H       = 0x0881);
PROVIDE(ADC12MEM17         = 0x0882);
PROVIDE(ADC12MEM17_L       = 0x0882);
PROVIDE(ADC12MEM17_H       = 0x0883);
PROVIDE(ADC12MEM18         = 0x0884);
PROVIDE(ADC12MEM18_L       = 0x0884);
PROVIDE(ADC12MEM18_H       = 0x0885);
PROVIDE(ADC12MEM19         = 0x0886);
PROVIDE(ADC12MEM19_L       = 0x0886);
PROVIDE(ADC12MEM19_H       = 0x0887);
PROVIDE(ADC12MEM20         = 0x0888);
PROVIDE(ADC12MEM20_L       = 0x0888);
PROVIDE(ADC12MEM20_H       = 0x0889);
PROVIDE(ADC12MEM21         = 0x088A);
PROVIDE(ADC12MEM21_L       = 0x088A);
PROVIDE(ADC12MEM21_H       = 0x088B);
PROVIDE(ADC12MEM22         = 0x088C);
PROVIDE(ADC12MEM22_L       = 0x088C);
PROVIDE(ADC12MEM22_H       = 0x088D);
PROVIDE(ADC12MEM23         = 0x088E);
PROVIDE(ADC12MEM23_L       = 0x088E);
PROVIDE(ADC12MEM23_H       = 0x088F);
PROVIDE(ADC12MEM24         = 0x0890);
PROVIDE(ADC12MEM24_L       = 0x0890);
PROVIDE(ADC12MEM24_H       = 0x0891);
PROVIDE(ADC12MEM25         = 0x0892);
PROVIDE(ADC12MEM25_L       = 0x0892);
PROVIDE(ADC12MEM25_H       = 0x0893);
PROVIDE(ADC12MEM26         = 0x0894);
PROVIDE(ADC12MEM26_L       = 0x0894);
PROVIDE(ADC12MEM26_H       = 0x0895);
PROVIDE(ADC12MEM27         = 0x0896);
PROVIDE(ADC12MEM27_L       = 0x0896);
PROVIDE(ADC12MEM27_H       = 0x0897);
PROVIDE(ADC12MEM28         = 0x0898);
PROVIDE(ADC12MEM28_L       = 0x0898);
PROVIDE(ADC12MEM28_H       = 0x0899);
PROVIDE(ADC12MEM29         = 0x089A);
PROVIDE(ADC12MEM29_L       = 0x089A);
PROVIDE(ADC12MEM29_H       = 0x089B);
PROVIDE(ADC12MEM30         = 0x089C);
PROVIDE(ADC12MEM30_L       = 0x089C);
PROVIDE(ADC12MEM30_H       = 0x089D);
PROVIDE(ADC12MEM31         = 0x089E);
PROVIDE(ADC12MEM31_L       = 0x089E);
PROVIDE(ADC12MEM31_H       = 0x089F);
/************************************************************
* AES256 Accelerator
************************************************************/
PROVIDE(AESACTL0           = 0x09C0);
PROVIDE(AESACTL0_L         = 0x09C0);
PROVIDE(AESACTL0_H         = 0x09C1);
PROVIDE(AESACTL1           = 0x09C2);
PROVIDE(AESACTL1_L         = 0x09C2);
PROVIDE(AESACTL1_H         = 0x09C3);
PROVIDE(AESASTAT           = 0x09C4);
PROVIDE(AESASTAT_L         = 0x09C4);
PROVIDE(AESASTAT_H         = 0x09C5);
PROVIDE(AESAKEY            = 0x09C6);
PROVIDE(AESAKEY_L          = 0x09C6);
PROVIDE(AESAKEY_H          = 0x09C7);
PROVIDE(AESADIN            = 0x09C8);
PROVIDE(AESADIN_L          = 0x09C8);
PROVIDE(AESADIN_H          = 0x09C9);
PROVIDE(AESADOUT           = 0x09CA);
PROVIDE(AESADOUT_L         = 0x09CA);
PROVIDE(AESADOUT_H         = 0x09CB);
PROVIDE(AESAXDIN           = 0x09CC);
PROVIDE(AESAXDIN_L         = 0x09CC);
PROVIDE(AESAXDIN_H         = 0x09CD);
PROVIDE(AESAXIN            = 0x09CE);
PROVIDE(AESAXIN_L          = 0x09CE);
PROVIDE(AESAXIN_H          = 0x09CF);
/************************************************************
* Capacitive_Touch_IO 0
************************************************************/
PROVIDE(CAPTIO0CTL         = 0x043E);
PROVIDE(CAPTIO0CTL_L       = 0x043E);
PROVIDE(CAPTIO0CTL_H       = 0x043F);
/************************************************************
* Capacitive_Touch_IO 1
************************************************************/
PROVIDE(CAPTIO1CTL         = 0x047E);
PROVIDE(CAPTIO1CTL_L       = 0x047E);
PROVIDE(CAPTIO1CTL_H       = 0x047F);
/************************************************************
* Comparator E
************************************************************/
PROVIDE(CECTL0             = 0x08C0);
PROVIDE(CECTL0_L           = 0x08C0);
PROVIDE(CECTL0_H           = 0x08C1);
PROVIDE(CECTL1             = 0x08C2);
PROVIDE(CECTL1_L           = 0x08C2);
PROVIDE(CECTL1_H           = 0x08C3);
PROVIDE(CECTL2             = 0x08C4);
PROVIDE(CECTL2_L           = 0x08C4);
PROVIDE(CECTL2_H           = 0x08C5);
PROVIDE(CECTL3             = 0x08C6);
PROVIDE(CECTL3_L           = 0x08C6);
PROVIDE(CECTL3_H           = 0x08C7);
PROVIDE(CEINT              = 0x08CC);
PROVIDE(CEINT_L            = 0x08CC);
PROVIDE(CEINT_H            = 0x08CD);
PROVIDE(CEIV               = 0x08CE);
PROVIDE(CEIV_L             = 0x08CE);
PROVIDE(CEIV_H             = 0x08CF);
/*************************************************************
* CRC Module
*************************************************************/
PROVIDE(CRCDI              = 0x0150);
PROVIDE(CRCDI_L            = 0x0150);
PROVIDE(CRCDI_H            = 0x0151);
PROVIDE(CRCDIRB            = 0x0152);
PROVIDE(CRCDIRB_L          = 0x0152);
PROVIDE(CRCDIRB_H          = 0x0153);
PROVIDE(CRCINIRES          = 0x0154);
PROVIDE(CRCINIRES_L        = 0x0154);
PROVIDE(CRCINIRES_H        = 0x0155);
PROVIDE(CRCRESR            = 0x0156);
PROVIDE(CRCRESR_L          = 0x0156);
PROVIDE(CRCRESR_H          = 0x0157);
/************************************************************
* CLOCK SYSTEM
************************************************************/
PROVIDE(CSCTL0             = 0x0160);
PROVIDE(CSCTL0_L           = 0x0160);
PROVIDE(CSCTL0_H           = 0x0161);
PROVIDE(CSCTL1             = 0x0162);
PROVIDE(CSCTL1_L           = 0x0162);
PROVIDE(CSCTL1_H           = 0x0163);
PROVIDE(CSCTL2             = 0x0164);
PROVIDE(CSCTL2_L           = 0x0164);
PROVIDE(CSCTL2_H           = 0x0165);
PROVIDE(CSCTL3             = 0x0166);
PROVIDE(CSCTL3_L           = 0x0166);
PROVIDE(CSCTL3_H           = 0x0167);
PROVIDE(CSCTL4             = 0x0168);
PROVIDE(CSCTL4_L           = 0x0168);
PROVIDE(CSCTL4_H           = 0x0169);
PROVIDE(CSCTL5             = 0x016A);
PROVIDE(CSCTL5_L           = 0x016A);
PROVIDE(CSCTL5_H           = 0x016B);
PROVIDE(CSCTL6             = 0x016C);
PROVIDE(CSCTL6_L           = 0x016C);
PROVIDE(CSCTL6_H           = 0x016D);
/************************************************************
* DMA_X
************************************************************/
PROVIDE(DMACTL0            = 0x0500);
PROVIDE(DMACTL0_L          = 0x0500);
PROVIDE(DMACTL0_H          = 0x0501);
PROVIDE(DMACTL1            = 0x0502);
PROVIDE(DMACTL1_L          = 0x0502);
PROVIDE(DMACTL1_H          = 0x0503);
PROVIDE(DMACTL2            = 0x0504);
PROVIDE(DMACTL2_L          = 0x0504);
PROVIDE(DMACTL2_H          = 0x0505);
PROVIDE(DMACTL3            = 0x0506);
PROVIDE(DMACTL3_L          = 0x0506);
PROVIDE(DMACTL3_H          = 0x0507);
PROVIDE(DMACTL4            = 0x0508);
PROVIDE(DMACTL4_L          = 0x0508);
PROVIDE(DMACTL4_H          = 0x0509);
PROVIDE(DMAIV              = 0x050E);
PROVIDE(DMAIV_L            = 0x050E);
PROVIDE(DMAIV_H            = 0x050F);
PROVIDE(DMA0CTL            = 0x0510);
PROVIDE(DMA0CTL_L          = 0x0510);
PROVIDE(DMA0CTL_H          = 0x0511);
PROVIDE(DMA0SA             = 0x0512);
PROVIDE(DMA0SAL            = 0x0512);
PROVIDE(DMA0DA             = 0x0516);
PROVIDE(DMA0DAL            = 0x0516);
PROVIDE(DMA0SZ             = 0x051A);
PROVIDE(DMA1CTL            = 0x0520);
PROVIDE(DMA1CTL_L          = 0x0520);
PROVIDE(DMA1CTL_H          = 0x0521);
PROVIDE(DMA1SA             = 0x0522);
PROVIDE(DMA1SAL            = 0x0522);
PROVIDE(DMA1DA             = 0x0526);
PROVIDE(DMA1DAL            = 0x0526);
PROVIDE(DMA1SZ             = 0x052A);
PROVIDE(DMA2CTL            = 0x0530);
PROVIDE(DMA2CTL_L          = 0x0530);
PROVIDE(DMA2CTL_H          = 0x0531);
PROVIDE(DMA2SA             = 0x0532);
PROVIDE(DMA2SAL            = 0x0532);
PROVIDE(DMA2DA             = 0x0536);
PROVIDE(DMA2DAL            = 0x0536);
PROVIDE(DMA2SZ             = 0x053A);
/*************************************************************
* FRAM Memory
*************************************************************/
PROVIDE(FRCTL0             = 0x0140);
PROVIDE(FRCTL0_L           = 0x0140);
PROVIDE(FRCTL0_H           = 0x0141);
PROVIDE(GCCTL0             = 0x0144);
PROVIDE(GCCTL0_L           = 0x0144);
PROVIDE(GCCTL0_H           = 0x0145);
PROVIDE(GCCTL1             = 0x0146);
PROVIDE(GCCTL1_L           = 0x0146);
PROVIDE(GCCTL1_H           = 0x0147);
/************************************************************
* Memory Protection Unit
************************************************************/
PROVIDE(MPUCTL0            = 0x05A0);
PROVIDE(MPUCTL0_L          = 0x05A0);
PROVIDE(MPUCTL0_H          = 0x05A1);
PROVIDE(MPUCTL1            = 0x05A2);
PROVIDE(MPUCTL1_L          = 0x05A2);
PROVIDE(MPUCTL1_H          = 0x05A3);
PROVIDE(MPUSEGB2           = 0x05A4);
PROVIDE(MPUSEGB2_L         = 0x05A4);
PROVIDE(MPUSEGB2_H         = 0x05A5);
PROVIDE(MPUSEGB1           = 0x05A6);
PROVIDE(MPUSEGB1_L         = 0x05A6);
PROVIDE(MPUSEGB1_H         = 0x05A7);
PROVIDE(MPUSAM             = 0x05A8);
PROVIDE(MPUSAM_L           = 0x05A8);
PROVIDE(MPUSAM_H           = 0x05A9);
PROVIDE(MPUIPC0            = 0x05AA);
PROVIDE(MPUIPC0_L          = 0x05AA);
PROVIDE(MPUIPC0_H          = 0x05AB);
PROVIDE(MPUIPSEGB2         = 0x05AC);
PROVIDE(MPUIPSEGB2_L       = 0x05AC);
PROVIDE(MPUIPSEGB2_H       = 0x05AD);
PROVIDE(MPUIPSEGB1         = 0x05AE);
PROVIDE(MPUIPSEGB1_L       = 0x05AE);
PROVIDE(MPUIPSEGB1_H       = 0x05AF);
/************************************************************
* HARDWARE MULTIPLIER 32Bit
************************************************************/
PROVIDE(MPY                = 0x04C0);
PROVIDE(MPY_L              = 0x04C0);
PROVIDE(MPY_H              = 0x04C1);
PROVIDE(MPYS               = 0x04C2);
PROVIDE(MPYS_L             = 0x04C2);
PROVIDE(MPYS_H             = 0x04C3);
PROVIDE(MAC                = 0x04C4);
PROVIDE(MAC_L              = 0x04C4);
PROVIDE(MAC_H              = 0x04C5);
PROVIDE(MACS               = 0x04C6);
PROVIDE(MACS_L             = 0x04C6);
PROVIDE(MACS_H             = 0x04C7);
PROVIDE(OP2                = 0x04C8);
PROVIDE(OP2_L              = 0x04C8);
PROVIDE(OP2_H              = 0x04C9);
PROVIDE(RESLO              = 0x04CA);
PROVIDE(RESLO_L            = 0x04CA);
PROVIDE(RESLO_H            = 0x04CB);
PROVIDE(RESHI              = 0x04CC);
PROVIDE(RESHI_L            = 0x04CC);
PROVIDE(RESHI_H            = 0x04CD);
PROVIDE(SUMEXT             = 0x04CE);
PROVIDE(SUMEXT_L           = 0x04CE);
PROVIDE(SUMEXT_H           = 0x04CF);
PROVIDE(MPY32L             = 0x04D0);
PROVIDE(MPY32L_L           = 0x04D0);
PROVIDE(MPY32L_H           = 0x04D1);
PROVIDE(MPY32H             = 0x04D2);
PROVIDE(MPY32H_L           = 0x04D2);
PROVIDE(MPY32H_H           = 0x04D3);
PROVIDE(MPYS32L            = 0x04D4);
PROVIDE(MPYS32L_L          = 0x04D4);
PROVIDE(MPYS32L_H          = 0x04D5);
PROVIDE(MPYS32H            = 0x04D6);
PROVIDE(MPYS32H_L          = 0x04D6);
PROVIDE(MPYS32H_H          = 0x04D7);
PROVIDE(MAC32L             = 0x04D8);
PROVIDE(MAC32L_L           = 0x04D8);
PROVIDE(MAC32L_H           = 0x04D9);
PROVIDE(MAC32H             = 0x04DA);
PROVIDE(MAC32H_L           = 0x04DA);
PROVIDE(MAC32H_H           = 0x04DB);
PROVIDE(MACS32L            = 0x04DC);
PROVIDE(MACS32L_L          = 0x04DC);
PROVIDE(MACS32L_H          = 0x04DD);
PROVIDE(MACS32H            = 0x04DE);
PROVIDE(MACS32H_L          = 0x04DE);
PROVIDE(MACS32H_H          = 0x04DF);
PROVIDE(OP2L               = 0x04E0);
PROVIDE(OP2L_L             = 0x04E0);
PROVIDE(OP2L_H             = 0x04E1);
PROVIDE(OP2H               = 0x04E2);
PROVIDE(OP2H_L             = 0x04E2);
PROVIDE(OP2H_H             = 0x04E3);
PROVIDE(RES0               = 0x04E4);
PROVIDE(RES0_L             = 0x04E4);
PROVIDE(RES0_H             = 0x04E5);
PROVIDE(RES1               = 0x04E6);
PROVIDE(RES1_L             = 0x04E6);
PROVIDE(RES1_H             = 0x04E7);
PROVIDE(RES2               = 0x04E8);
PROVIDE(RES2_L             = 0x04E8);
PROVIDE(RES2_H             = 0x04E9);
PROVIDE(RES3               = 0x04EA);
PROVIDE(RES3_L             = 0x04EA);
PROVIDE(RES3_H             = 0x04EB);
PROVIDE(MPY32CTL0          = 0x04EC);
PROVIDE(MPY32CTL0_L        = 0x04EC);
PROVIDE(MPY32CTL0_H        = 0x04ED);
/************************************************************
* PMM - Power Management System for FRAM
************************************************************/
PROVIDE(PMMCTL0            = 0x0120);
PROVIDE(PMMCTL0_L          = 0x0120);
PROVIDE(PMMCTL0_H          = 0x0121);
PROVIDE(PMMIFG             = 0x012A);
PROVIDE(PMMIFG_L           = 0x012A);
PROVIDE(PMMIFG_H           = 0x012B);
PROVIDE(PM5CTL0            = 0x0130);
PROVIDE(PM5CTL0_L          = 0x0130);
PROVIDE(PM5CTL0_H          = 0x0131);
/************************************************************
* DIGITAL I/O Port1/2 Pull up / Pull down Resistors
************************************************************/
PROVIDE(PAIN               = 0x0200);
PROVIDE(PAIN_L             = 0x0200);
PROVIDE(PAIN_H             = 0x0201);
PROVIDE(PAOUT              = 0x0202);
PROVIDE(PAOUT_L            = 0x0202);
PROVIDE(PAOUT_H            = 0x0203);
PROVIDE(PADIR              = 0x0204);
PROVIDE(PADIR_L            = 0x0204);
PROVIDE(PADIR_H            = 0x0205);
PROVIDE(PAREN              = 0x0206);
PROVIDE(PAREN_L            = 0x0206);
PROVIDE(PAREN_H            = 0x0207);
PROVIDE(PASEL0             = 0x020A);
PROVIDE(PASEL0_L           = 0x020A);
PROVIDE(PASEL0_H           = 0x020B);
PROVIDE(PASEL1             = 0x020C);
PROVIDE(PASEL1_L           = 0x020C);
PROVIDE(PASEL1_H           = 0x020D);
PROVIDE(PASELC             = 0x0216);
PROVIDE(PASELC_L           = 0x0216);
PROVIDE(PASELC_H           = 0x0217);
PROVIDE(PAIES              = 0x0218);
PROVIDE(PAIES_L            = 0x0218);
PROVIDE(PAIES_H            = 0x0219);
PROVIDE(PAIE               = 0x021A);
PROVIDE(PAIE_L             = 0x021A);
PROVIDE(PAIE_H             = 0x021B);
PROVIDE(PAIFG              = 0x021C);
PROVIDE(PAIFG_L            = 0x021C);
PROVIDE(PAIFG_H            = 0x021D);
PROVIDE(P1IV               = 0x020E);
PROVIDE(P2IV               = 0x021E);
/************************************************************
* DIGITAL I/O Port3/4 Pull up / Pull down Resistors
************************************************************/
PROVIDE(PBIN               = 0x0220);
PROVIDE(PBIN_L             = 0x0220);
PROVIDE(PBIN_H             = 0x0221);
PROVIDE(PBOUT              = 0x0222);
PROVIDE(PBOUT_L            = 0x0222);
PROVIDE(PBOUT_H            = 0x0223);
PROVIDE(PBDIR              = 0x0224);
PROVIDE(PBDIR_L            = 0x0224);
PROVIDE(PBDIR_H            = 0x0225);
PROVIDE(PBREN              = 0x0226);
PROVIDE(PBREN_L            = 0x0226);
PROVIDE(PBREN_H            = 0x0227);
PROVIDE(PBSEL0             = 0x022A);
PROVIDE(PBSEL0_L           = 0x022A);
PROVIDE(PBSEL0_H           = 0x022B);
PROVIDE(PBSEL1             = 0x022C);
PROVIDE(PBSEL1_L           = 0x022C);
PROVIDE(PBSEL1_H           = 0x022D);
PROVIDE(PBSELC             = 0x0236);
PROVIDE(PBSELC_L           = 0x0236);
PROVIDE(PBSELC_H           = 0x0237);
PROVIDE(PBIES              = 0x0238);
PROVIDE(PBIES_L            = 0x0238);
PROVIDE(PBIES_H            = 0x0239);
PROVIDE(PBIE               = 0x023A);
PROVIDE(PBIE_L             = 0x023A);
PROVIDE(PBIE_H             = 0x023B);
PROVIDE(PBIFG              = 0x023C);
PROVIDE(PBIFG_L            = 0x023C);
PROVIDE(PBIFG_H            = 0x023D);
PROVIDE(P3IV               = 0x022E);
PROVIDE(P4IV               = 0x023E);
/************************************************************
* DIGITAL I/O PortJ Pull up / Pull down Resistors
************************************************************/
PROVIDE(PJIN               = 0x0320);
PROVIDE(PJIN_L             = 0x0320);
PROVIDE(PJIN_H             = 0x0321);
PROVIDE(PJOUT              = 0x0322);
PROVIDE(PJOUT_L            = 0x0322);
PROVIDE(PJOUT_H            = 0x0323);
PROVIDE(PJDIR              = 0x0324);
PROVIDE(PJDIR_L            = 0x0324);
PROVIDE(PJDIR_H            = 0x0325);
PROVIDE(PJREN              = 0x0326);
PROVIDE(PJREN_L            = 0x0326);
PROVIDE(PJREN_H            = 0x0327);
PROVIDE(PJSEL0             = 0x032A);
PROVIDE(PJSEL0_L           = 0x032A);
PROVIDE(PJSEL0_H           = 0x032B);
PROVIDE(PJSEL1             = 0x032C);
PROVIDE(PJSEL1_L           = 0x032C);
PROVIDE(PJSEL1_H           = 0x032D);
PROVIDE(PJSELC             = 0x0336);
PROVIDE(PJSELC_L           = 0x0336);
PROVIDE(PJSELC_H           = 0x0337);
/************************************************************
* Shared Reference
************************************************************/
PROVIDE(REFCTL0            = 0x01B0);
PROVIDE(REFCTL0_L          = 0x01B0);
PROVIDE(REFCTL0_H          = 0x01B1);
/************************************************************
* Real Time Clock
************************************************************/
PROVIDE(RTCCTL01           = 0x04A0);
PROVIDE(RTCCTL01_L         = 0x04A0);
PROVIDE(RTCCTL01_H         = 0x04A1);
PROVIDE(RTCCTL23           = 0x04A2);
PROVIDE(RTCCTL23_L         = 0x04A2);
PROVIDE(RTCCTL23_H         = 0x04A3);
PROVIDE(RTCPS0CTL          = 0x04A8);
PROVIDE(RTCPS0CTL_L        = 0x04A8);
PROVIDE(RTCPS0CTL_H        = 0x04A9);
PROVIDE(RTCPS1CTL          = 0x04AA);
PROVIDE(RTCPS1CTL_L        = 0x04AA);
PROVIDE(RTCPS1CTL_H        = 0x04AB);
PROVIDE(RTCPS              = 0x04AC);
PROVIDE(RTCPS_L            = 0x04AC);
PROVIDE(RTCPS_H            = 0x04AD);
PROVIDE(RTCIV              = 0x04AE);
PROVIDE(RTCTIM0            = 0x04B0);
PROVIDE(RTCTIM0_L          = 0x04B0);
PROVIDE(RTCTIM0_H          = 0x04B1);
PROVIDE(RTCTIM1            = 0x04B2);
PROVIDE(RTCTIM1_L          = 0x04B2);
PROVIDE(RTCTIM1_H          = 0x04B3);
PROVIDE(RTCDATE            = 0x04B4);
PROVIDE(RTCDATE_L          = 0x04B4);
PROVIDE(RTCDATE_H          = 0x04B5);
PROVIDE(RTCYEAR            = 0x04B6);
PROVIDE(RTCYEAR_L          = 0x04B6);
PROVIDE(RTCYEAR_H          = 0x04B7);
PROVIDE(RTCAMINHR          = 0x04B8);
PROVIDE(RTCAMINHR_L        = 0x04B8);
PROVIDE(RTCAMINHR_H        = 0x04B9);
PROVIDE(RTCADOWDAY         = 0x04BA);
PROVIDE(RTCADOWDAY_L       = 0x04BA);
PROVIDE(RTCADOWDAY_H       = 0x04BB);
PROVIDE(BIN2BCD            = 0x04BC);
PROVIDE(BCD2BIN            = 0x04BE);
/************************************************************
* SFR - Special Function Register Module
************************************************************/
PROVIDE(SFRIE1             = 0x0100);
PROVIDE(SFRIE1_L           = 0x0100);
PROVIDE(SFRIE1_H           = 0x0101);
PROVIDE(SFRIFG1            = 0x0102);
PROVIDE(SFRIFG1_L          = 0x0102);
PROVIDE(SFRIFG1_H          = 0x0103);
PROVIDE(SFRRPCR            = 0x0104);
PROVIDE(SFRRPCR_L          = 0x0104);
PROVIDE(SFRRPCR_H          = 0x0105);
/************************************************************
* SYS - System Module
************************************************************/
PROVIDE(SYSCTL             = 0x0180);
PROVIDE(SYSCTL_L           = 0x0180);
PROVIDE(SYSCTL_H           = 0x0181);
PROVIDE(SYSJMBC            = 0x0186);
PROVIDE(SYSJMBC_L          = 0x0186);
PROVIDE(SYSJMBC_H          = 0x0187);
PROVIDE(SYSJMBI0           = 0x0188);
PROVIDE(SYSJMBI0_L         = 0x0188);
PROVIDE(SYSJMBI0_H         = 0x0189);
PROVIDE(SYSJMBI1           = 0x018A);
PROVIDE(SYSJMBI1_L         = 0x018A);
PROVIDE(SYSJMBI1_H         = 0x018B);
PROVIDE(SYSJMBO0           = 0x018C);
PROVIDE(SYSJMBO0_L         = 0x018C);
PROVIDE(SYSJMBO0_H         = 0x018D);
PROVIDE(SYSJMBO1           = 0x018E);
PROVIDE(SYSJMBO1_L         = 0x018E);
PROVIDE(SYSJMBO1_H         = 0x018F);
PROVIDE(SYSUNIV            = 0x019A);
PROVIDE(SYSUNIV_L          = 0x019A);
PROVIDE(SYSUNIV_H          = 0x019B);
PROVIDE(SYSSNIV            = 0x019C);
PROVIDE(SYSSNIV_L          = 0x019C);
PROVIDE(SYSSNIV_H          = 0x019D);
PROVIDE(SYSRSTIV           = 0x019E);
PROVIDE(SYSRSTIV_L         = 0x019E);
PROVIDE(SYSRSTIV_H         = 0x019F);
/************************************************************
* Timer0_A3
************************************************************/
PROVIDE(TA0CTL             = 0x0340);
PROVIDE(TA0CCTL0           = 0x0342);
PROVIDE(TA0CCTL1           = 0x0344);
PROVIDE(TA0CCTL2           = 0x0346);
PROVIDE(TA0R               = 0x0350);
PROVIDE(TA0CCR0            = 0x0352);
PROVIDE(TA0CCR1            = 0x0354);
PROVIDE(TA0CCR2            = 0x0356);
PROVIDE(TA0IV              = 0x036E);
PROVIDE(TA0EX0             = 0x0360);
/************************************************************
* Timer1_A3
************************************************************/
PROVIDE(TA1CTL             = 0x0380);
PROVIDE(TA1CCTL0           = 0x0382);
PROVIDE(TA1CCTL1           = 0x0384);
PROVIDE(TA1CCTL2           = 0x0386);
PROVIDE(TA1R               = 0x0390);
PROVIDE(TA1CCR0            = 0x0392);
PROVIDE(TA1CCR1            = 0x0394);
PROVIDE(TA1CCR2            = 0x0396);
PROVIDE(TA1IV              = 0x03AE);
PROVIDE(TA1EX0             = 0x03A0);
/************************************************************
* Timer2_A2
************************************************************/
PROVIDE(TA2CTL             = 0x0400);
PROVIDE(TA2CCTL0           = 0x0402);
PROVIDE(TA2CCTL1           = 0x0404);
PROVIDE(TA2R               = 0x0410);
PROVIDE(TA2CCR0            = 0x0412);
PROVIDE(TA2CCR1            = 0x0414);
PROVIDE(TA2IV              = 0x042E);
PROVIDE(TA2EX0             = 0x0420);
/************************************************************
* Timer3_A2
************************************************************/
PROVIDE(TA3CTL             = 0x0440);
PROVIDE(TA3CCTL0           = 0x0442);
PROVIDE(TA3CCTL1           = 0x0444);
PROVIDE(TA3R               = 0x0450);
PROVIDE(TA3CCR0            = 0x0452);
PROVIDE(TA3CCR1            = 0x0454);
PROVIDE(TA3IV              = 0x046E);
PROVIDE(TA3EX0             = 0x0460);
/************************************************************
* Timer0_B7
************************************************************/
PROVIDE(TB0CTL             = 0x03C0);
PROVIDE(TB0CCTL0           = 0x03C2);
PROVIDE(TB0CCTL1           = 0x03C4);
PROVIDE(TB0CCTL2           = 0x03C6);
PROVIDE(TB0CCTL3           = 0x03C8);
PROVIDE(TB0CCTL4           = 0x03CA);
PROVIDE(TB0CCTL5           = 0x03CC);
PROVIDE(TB0CCTL6           = 0x03CE);
PROVIDE(TB0R               = 0x03D0);
PROVIDE(TB0CCR0            = 0x03D2);
PROVIDE(TB0CCR1            = 0x03D4);
PROVIDE(TB0CCR2            = 0x03D6);
PROVIDE(TB0CCR3            = 0x03D8);
PROVIDE(TB0CCR4            = 0x03DA);
PROVIDE(TB0CCR5            = 0x03DC);
PROVIDE(TB0CCR6            = 0x03DE);
PROVIDE(TB0EX0             = 0x03E0);
PROVIDE(TB0IV              = 0x03EE);
/************************************************************
* USCI A0
************************************************************/
PROVIDE(UCA0CTLW0          = 0x05C0);
PROVIDE(UCA0CTLW0_L        = 0x05C0);
PROVIDE(UCA0CTLW0_H        = 0x05C1);
PROVIDE(UCA0CTLW1          = 0x05C2);
PROVIDE(UCA0CTLW1_L        = 0x05C2);
PROVIDE(UCA0CTLW1_H        = 0x05C3);
PROVIDE(UCA0BRW            = 0x05C6);
PROVIDE(UCA0BRW_L          = 0x05C6);
PROVIDE(UCA0BRW_H          = 0x05C7);
PROVIDE(UCA0MCTLW          = 0x05C8);
PROVIDE(UCA0MCTLW_L        = 0x05C8);
PROVIDE(UCA0MCTLW_H        = 0x05C9);
PROVIDE(UCA0STATW          = 0x05CA);
PROVIDE(UCA0RXBUF          = 0x05CC);
PROVIDE(UCA0RXBUF_L        = 0x05CC);
PROVIDE(UCA0RXBUF_H        = 0x05CD);
PROVIDE(UCA0TXBUF          = 0x05CE);
PROVIDE(UCA0TXBUF_L        = 0x05CE);
PROVIDE(UCA0TXBUF_H        = 0x05CF);
PROVIDE(UCA0ABCTL          = 0x05D0);
PROVIDE(UCA0IRCTL          = 0x05D2);
PROVIDE(UCA0IRCTL_L        = 0x05D2);
PROVIDE(UCA0IRCTL_H        = 0x05D3);
PROVIDE(UCA0IE             = 0x05DA);
PROVIDE(UCA0IE_L           = 0x05DA);
PROVIDE(UCA0IE_H           = 0x05DB);
PROVIDE(UCA0IFG            = 0x05DC);
PROVIDE(UCA0IFG_L          = 0x05DC);
PROVIDE(UCA0IFG_H          = 0x05DD);
PROVIDE(UCA0IV             = 0x05DE);
/************************************************************
* USCI A1
************************************************************/
PROVIDE(UCA1CTLW0          = 0x05E0);
PROVIDE(UCA1CTLW0_L        = 0x05E0);
PROVIDE(UCA1CTLW0_H        = 0x05E1);
PROVIDE(UCA1CTLW1          = 0x05E2);
PROVIDE(UCA1CTLW1_L        = 0x05E2);
PROVIDE(UCA1CTLW1_H        = 0x05E3);
PROVIDE(UCA1BRW            = 0x05E6);
PROVIDE(UCA1BRW_L          = 0x05E6);
PROVIDE(UCA1BRW_H          = 0x05E7);
PROVIDE(UCA1MCTLW          = 0x05E8);
PROVIDE(UCA1MCTLW_L        = 0x05E8);
PROVIDE(UCA1MCTLW_H        = 0x05E9);
PROVIDE(UCA1STATW          = 0x05EA);
PROVIDE(UCA1RXBUF          = 0x05EC);
PROVIDE(UCA1RXBUF_L        = 0x05EC);
PROVIDE(UCA1RXBUF_H        = 0x05ED);
PROVIDE(UCA1TXBUF          = 0x05EE);
PROVIDE(UCA1TXBUF_L        = 0x05EE);
PROVIDE(UCA1TXBUF_H        = 0x05EF);
PROVIDE(UCA1ABCTL          = 0x05F0);
PROVIDE(UCA1IRCTL          = 0x05F2);
PROVIDE(UCA1IRCTL_L        = 0x05F2);
PROVIDE(UCA1IRCTL_H        = 0x05F3);
PROVIDE(UCA1IE             = 0x05FA);
PROVIDE(UCA1IE_L           = 0x05FA);
PROVIDE(UCA1IE_H           = 0x05FB);
PROVIDE(UCA1IFG            = 0x05FC);
PROVIDE(UCA1IFG_L          = 0x05FC);
PROVIDE(UCA1IFG_H          = 0x05FD);
PROVIDE(UCA1IV             = 0x05FE);
/************************************************************
* USCI B0
************************************************************/
PROVIDE(UCB0CTLW0          = 0x0640);
PROVIDE(UCB0CTLW0_L        = 0x0640);
PROVIDE(UCB0CTLW0_H        = 0x0641);
PROVIDE(UCB0CTLW1          = 0x0642);
PROVIDE(UCB0CTLW1_L        = 0x0642);
PROVIDE(UCB0CTLW1_H        = 0x0643);
PROVIDE(UCB0BRW            = 0x0646);
PROVIDE(UCB0BRW_L          = 0x0646);
PROVIDE(UCB0BRW_H          = 0x0647);
PROVIDE(UCB0STATW          = 0x0648);
PROVIDE(UCB0STATW_L        = 0x0648);
PROVIDE(UCB0STATW_H        = 0x0649);
PROVIDE(UCB0TBCNT          = 0x064A);
PROVIDE(UCB0TBCNT_L        = 0x064A);
PROVIDE(UCB0TBCNT_H        = 0x064B);
PROVIDE(UCB0RXBUF          = 0x064C);
PROVIDE(UCB0RXBUF_L        = 0x064C);
PROVIDE(UCB0RXBUF_H        = 0x064D);
PROVIDE(UCB0TXBUF          = 0x064E);
PROVIDE(UCB0TXBUF_L        = 0x064E);
PROVIDE(UCB0TXBUF_H        = 0x064F);
PROVIDE(UCB0I2COA0         = 0x0654);
PROVIDE(UCB0I2COA0_L       = 0x0654);
PROVIDE(UCB0I2COA0_H       = 0x0655);
PROVIDE(UCB0I2COA1         = 0x0656);
PROVIDE(UCB0I2COA1_L       = 0x0656);
PROVIDE(UCB0I2COA1_H       = 0x0657);
PROVIDE(UCB0I2COA2         = 0x0658);
PROVIDE(UCB0I2COA2_L       = 0x0658);
PROVIDE(UCB0I2COA2_H       = 0x0659);
PROVIDE(UCB0I2COA3         = 0x065A);
PROVIDE(UCB0I2COA3_L       = 0x065A);
PROVIDE(UCB0I2COA3_H       = 0x065B);
PROVIDE(UCB0ADDRX          = 0x065C);
PROVIDE(UCB0ADDRX_L        = 0x065C);
PROVIDE(UCB0ADDRX_H        = 0x065D);
PROVIDE(UCB0ADDMASK        = 0x065E);
PROVIDE(UCB0ADDMASK_L      = 0x065E);
PROVIDE(UCB0ADDMASK_H      = 0x065F);
PROVIDE(UCB0I2CSA          = 0x0660);
PROVIDE(UCB0I2CSA_L        = 0x0660);
PROVIDE(UCB0I2CSA_H        = 0x0661);
PROVIDE(UCB0IE             = 0x066A);
PROVIDE(UCB0IE_L           = 0x066A);
PROVIDE(UCB0IE_H           = 0x066B);
PROVIDE(UCB0IFG            = 0x066C);
PROVIDE(UCB0IFG_L          = 0x066C);
PROVIDE(UCB0IFG_H          = 0x066D);
PROVIDE(UCB0IV             = 0x066E);
/************************************************************
* WATCHDOG TIMER A
************************************************************/
PROVIDE(WDTCTL             = 0x015C);
PROVIDE(WDTCTL_L           = 0x015C);
PROVIDE(WDTCTL_H           = 0x015D);
/************************************************************
* TLV Descriptors
************************************************************/
/************************************************************
* Interrupt Vectors (offset from 0xFF80 + 0x10 for Password)
************************************************************/
/************************************************************
* End of Modules
************************************************************/

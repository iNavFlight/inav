/**************************************************************************
 * FILE NAME: spc570s50l.h         COPYRIGHT (c) ST Microelectronics 2012 * 
 * REVISION:  0.1A                                    All Rights Reserved *
 *                                                                        *
 * DESCRIPTION:                                                           *
 * This file contains all of the register and bit field definitions for   *
 * the SPC570S50L.                                                        *
 *========================================================================*
 * UPDATE HISTORY                                                         *
 * REV       AUTHOR        DATE       	DESCRIPTION OF CHANGE             *
 * ---   -------------   ---------    	---------------------             *
 * 0.1a  Juhee MALA     17-Mar-13    	Initial Release for FLASH Core    *
 *                                      Host Project, only following      *
 *                                      Peripherals are updated for SPC570S50L       *
 *                                        - pFLASH
 *                                         - FLASH 
 *                                         - SIUL2
 *                                         - PRAM  
 *                                         - MC_ME
 *                                         - MC_CGM  
 *                                         - PLLDIG
 *                                         - XOSC
 *                                         - STM
 **************************************************************************/ 
    
    
/**************************************************************************
 * Example register & bit field write:                                    *
 *                                                                        *
 *  <MODULE>.<REGISTER>.B.<BIT> = 1;                                      *
 *  <MODULE>.<REGISTER>.R       = 0x10000000;                             *
 *                                                                        *
 **************************************************************************/ 
    
#ifndef _SPC570S50L_H_
#define _SPC570S50L_H_
    
#include "typedefs.h"
    
#ifdef  __cplusplus
extern "C" {
#endif
    
#ifdef __MWERKS__
#pragma push
#pragma ANSI_strict off
#endif

/**************************************************************************/
/*                   Module: CMU                                          */
/**************************************************************************/
    struct CMU_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:8;
            vuint32_t SFM:1;
            vuint32_t:13;
            vuint32_t CKSEL1:2;
            vuint32_t:5;
            vuint32_t RCDIV:2;
            vuint32_t CME:1;
        } B;
    } CSR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t FD:20;
        } B;
    } FDR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:20;
            vuint32_t HFREF:12;
        } B;
    } HFREFR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:20;
            vuint32_t LFREF:12;
        } B;
    } LFREFR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:28;
            vuint32_t FLCI:1;
            vuint32_t FHHI:1;
            vuint32_t FLLI:1;
            vuint32_t OLRI:1;
        } B;
    } ISR;

    uint8_t CMU_reserved1[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t MD:20;
        } B;
    } MDR;
};
/**************************************************************************/
/*                   Module: CRC                                          */
/**************************************************************************/
struct CRC_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:28;
            vuint32_t POLYG:2;
            vuint32_t SWAP:1;
            vuint32_t INV:1;
        } B;
    } CFG1;

    union {
        vuint32_t R;
        struct {
            vuint32_t INP:32;
        } B;
    } INP1;

    union {
        vuint32_t R;
        struct {
            vuint32_t CSTAT:32;
        } B;
    } CSTAT1;

    union {
        vuint32_t R;
        struct {
            vuint32_t OUTP:32;
        } B;
    } OUTP1;

    union {
        vuint32_t R;
        struct {
            vuint32_t:28;
            vuint32_t POLYG:2;
            vuint32_t SWAP:1;
            vuint32_t INV:1;
        } B;
    } CFG2;

    union {
        vuint32_t R;
        struct {
            vuint32_t INP:32;
        } B;
    } INP2;

    union {
        vuint32_t R;
        struct {
            vuint32_t CSTAT:32;
        } B;
    } CSTAT2;

    union {
        vuint32_t R;
        struct {
            vuint32_t OUTP:32;
        } B;
    } OUTP2;

    union {
        vuint32_t R;
        struct {
            vuint32_t:28;
            vuint32_t POLYG:2;
            vuint32_t SWAP:1;
            vuint32_t INV:1;
        } B;
    } CFG3;

    union {
        vuint32_t R;
        struct {
            vuint32_t INP:32;
        } B;
    } INP3;

    union {
        vuint32_t R;
        struct {
            vuint32_t CSTAT:32;
        } B;
    } CSTAT3;

    union {
        vuint32_t R;
        struct {
            vuint32_t OUTP:32;
        } B;
    } OUTP3;

    uint8_t CRC_reserved1[208];

    union {
        vuint32_t R;
        struct {
            vuint32_t OUTP_CHK:32;
        } B;
    } OUTP_CHK1;

    union {
        vuint32_t R;
        struct {
            vuint32_t OUTP_CHK:32;
        } B;
    } OUTP_CHK2;

    union {
        vuint32_t R;
        struct {
            vuint32_t OUTP_CHK:32;
        } B;
    } OUTP_CHK3;
};
/**************************************************************************/
/*                   Module: DigRF                                        */
/**************************************************************************/
struct DigRF_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t MSEN:1;
            vuint32_t:6;
            vuint32_t IPGDBG:1;
            vuint32_t:7;
            vuint32_t LSSEL:1;
            vuint32_t DRFEN:1;
            vuint32_t RXEN:1;
            vuint32_t TXEN:1;
            vuint32_t:8;
            vuint32_t TXARBD:1;
            vuint32_t CTSEN:1;
            vuint32_t:1;
            vuint32_t DRFRST:1;
            vuint32_t DATAEN:1;
        } B;
    } MCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:15;
            vuint32_t DRMD:1;
            vuint32_t:7;
            vuint32_t RDR:1;
            vuint32_t:7;
            vuint32_t TDR:1;
        } B;
    } SCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t SMPSEL:8;
            vuint32_t:20;
            vuint32_t CORRTH:3;
            vuint32_t PHSSEL:1;
        } B;
    } COCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t CLKTST:1;
            vuint32_t LPON:1;
            vuint32_t:5;
            vuint32_t LPMOD:3;
            vuint32_t LPFRMTH:16;
        } B;
    } TMCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:15;
            vuint32_t LPCNTEN:1;
            vuint32_t LPFMCNT:16;
        } B;
    } ALCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t DRCNT:4;
            vuint32_t:16;
        } B;
    } RCDCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t HSCNT:8;
            vuint32_t:4;
            vuint32_t LSCNT:4;
            vuint32_t HWKCNT:8;
            vuint32_t:4;
            vuint32_t LWKCNT:4;
        } B;
    } SLCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:14;
            vuint32_t ICLCSEQ:1;
            vuint32_t SNDICLC:1;
            vuint32_t:8;
            vuint32_t ICLCPLD:8;
        } B;
    } ICR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:15;
            vuint32_t PNGREQ:1;
            vuint32_t PNGAUTO:1;
            vuint32_t:7;
            vuint32_t PNGPYLD:8;
        } B;
    } PICR;

    uint8_t DigRF_reserved1[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:10;
            vuint32_t CTSMX:6;
            vuint32_t:10;
            vuint32_t CTSMN:6;
        } B;
    } RFCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:14;
            vuint32_t TXIIE:1;
            vuint32_t TXOVIE:1;
            vuint32_t:11;
            vuint32_t TXPNGIE:1;
            vuint32_t:1;
            vuint32_t TXUNSIE:1;
            vuint32_t TXICLCIE:1;
            vuint32_t TXDTIE:1;
        } B;
    } TIER;

    union {
        vuint32_t R;
        struct {
            vuint32_t:8;
            vuint32_t RXUOIE:1;
            vuint32_t RXMNIE:1;
            vuint32_t RXMXIE:1;
            vuint32_t RXUFIE:1;
            vuint32_t RXOFIE:1;
            vuint32_t RXSZIE:1;
            vuint32_t RXICIE:1;
            vuint32_t RXLCEIE:1;
            vuint32_t:12;
            vuint32_t RXCTSIE:1;
            vuint32_t RXDIE:1;
            vuint32_t RXUNSIE:1;
            vuint32_t:1;
        } B;
    } RIER;

    union {
        vuint32_t R;
        struct {
            vuint32_t:18;
            vuint32_t ICPFIE:1;
            vuint32_t ICPSIE:1;
            vuint32_t ICPRIE:1;
            vuint32_t ICTOIE:1;
            vuint32_t ICLPIE:1;
            vuint32_t ICCTIE:1;
            vuint32_t ICTDIE:1;
            vuint32_t ICTEIE:1;
            vuint32_t ICRFIE:1;
            vuint32_t ICRSIE:1;
            vuint32_t ICTFIE:1;
            vuint32_t ICTSIE:1;
            vuint32_t ICPOFIE:1;
            vuint32_t ICPONIE:1;
        } B;
    } RIIER;

    union {
        vuint32_t R;
        struct {
            vuint32_t IPTMOD:3;
            vuint32_t:11;
            vuint32_t SWPOFF:1;
            vuint32_t SWPON:1;
            vuint32_t REFINV:1;
            vuint32_t LPCFG:2;
            vuint32_t:2;
            vuint32_t PLCKCW:2;
            vuint32_t FDIVEN:1;
            vuint32_t FBDIV:6;
            vuint32_t PREDIV:2;
        } B;
    } PLLCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:8;
            vuint32_t SWWKLD:1;
            vuint32_t SWSLPLD:1;
            vuint32_t SWWKLR:1;
            vuint32_t SWSLPLR:1;
            vuint32_t SWOFFLD:1;
            vuint32_t SWONLD:1;
            vuint32_t SWOFFLR:1;
            vuint32_t SWONLR:1;
            vuint32_t LVRXOFF:1;
            vuint32_t LVTXOE:1;
            vuint32_t TXCMUX:1;
            vuint32_t LVRFEN:1;
            vuint32_t LVLPEN:1;
            vuint32_t:5;
            vuint32_t LVRXOP:3;
            vuint32_t LVTXOP:1;
            vuint32_t LVCKSS:1;
            vuint32_t LVCKP:1;
        } B;
    } LCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:15;
            vuint32_t USNDRQ:1;
            vuint32_t:9;
            vuint32_t UNSHDR:7;
        } B;
    } UNSTCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNTXD:32;
        } B;
    } UNSTDR8;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNTXD:32;
        } B;
    } UNSTDR7;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNTXD:32;
        } B;
    } UNSTDR6;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNTXD:32;
        } B;
    } UNSTDR5;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNTXD:32;
        } B;
    } UNSTDR4;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNTXD:32;
        } B;
    } UNSTDR3;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNTXD:32;
        } B;
    } UNSTDR2;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNTXD:32;
        } B;
    } UNSTDR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNTXD:32;
        } B;
    } UNSTDR0;

    uint8_t DigRF_reserved2[20];

    union {
        vuint32_t R;
        struct {
            vuint32_t DUALMD:1;
            vuint32_t:12;
            vuint32_t LRMD:1;
            vuint32_t LDSM:1;
            vuint32_t DRSM:1;
            vuint32_t:11;
            vuint32_t LPTXDN:1;
            vuint32_t LPFPDV:1;
            vuint32_t LPCPDV:1;
            vuint32_t LPCHDV:1;
            vuint32_t LPCSDV:1;
        } B;
    } GSR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t RXPNGD:8;
        } B;
    } PISR;

    uint8_t DigRF_reserved3[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t:2;
            vuint32_t RXDCNT:6;
            vuint32_t:5;
            vuint32_t RXFCNT:3;
            vuint32_t:2;
            vuint32_t TXDCNT:6;
            vuint32_t:5;
            vuint32_t TXFCNT:3;
        } B;
    } DFSR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:14;
            vuint32_t TXIEF:1;
            vuint32_t TXOVF:1;
            vuint32_t:11;
            vuint32_t TXPNGF:1;
            vuint32_t:1;
            vuint32_t TXUNSF:1;
            vuint32_t TXICLCF:1;
            vuint32_t TXDTF:1;
        } B;
    } TISR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:8;
            vuint32_t RXUOF:1;
            vuint32_t RXMNF:1;
            vuint32_t RXMXF:1;
            vuint32_t RXUFF:1;
            vuint32_t RXOFF:1;
            vuint32_t RXSZF:1;
            vuint32_t RXICF:1;
            vuint32_t RXLCEF:1;
            vuint32_t:12;
            vuint32_t RXCTSF:1;
            vuint32_t RXDF:1;
            vuint32_t RXUNSF:1;
            vuint32_t:1;
        } B;
    } RISR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:18;
            vuint32_t ICPFF:1;
            vuint32_t ICPSF:1;
            vuint32_t ICPRF:1;
            vuint32_t ICTOF:1;
            vuint32_t ICLPF:1;
            vuint32_t ICCTF:1;
            vuint32_t ICTDF:1;
            vuint32_t ICTEF:1;
            vuint32_t ICRFF:1;
            vuint32_t ICRSF:1;
            vuint32_t ICTFF:1;
            vuint32_t ICTSF:1;
            vuint32_t ICPOFF:1;
            vuint32_t ICPONF:1;
        } B;
    } RIISR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:14;
            vuint32_t PLLDIS:1;
            vuint32_t PLDCR:1;
            vuint32_t:12;
            vuint32_t LDSLPS:1;
            vuint32_t LRSLPS:1;
            vuint32_t LDPDS:1;
            vuint32_t LRPDS:1;
        } B;
    } PLLLSR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:23;
            vuint32_t URXDV:1;
            vuint32_t:5;
            vuint32_t URPCNT:3;
        } B;
    } UNSRSR;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNRXD:32;
        } B;
    } UNSRDR8;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNRXD:32;
        } B;
    } UNSRDR7;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNRXD:32;
        } B;
    } UNSRDR6;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNRXD:32;
        } B;
    } UNSRDR5;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNRXD:32;
        } B;
    } UNSRDR4;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNRXD:32;
        } B;
    } UNSRDR3;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNRXD:32;
        } B;
    } UNSRDR2;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNRXD:32;
        } B;
    } UNSRDR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t UNRXD:32;
        } B;
    } UNSRDR0;
};
/**************************************************************************/
/*                   Module: DMA                                          */
/**************************************************************************/
struct DMA_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:14;
            vuint32_t CX:1;
            vuint32_t ECX:1;
            vuint32_t GRP3PRI:2;
            vuint32_t GRP2PRI:2;
            vuint32_t GRP1PRI:2;
            vuint32_t GRP0PRI:2;
            vuint32_t EMLM:1;
            vuint32_t CLM:1;
            vuint32_t HALT:1;
            vuint32_t HOE:1;
            vuint32_t ERGA:1;
            vuint32_t ERCA:1;
            vuint32_t EDBG:1;
            vuint32_t:1;
        } B;
    } CR;

    union {
        vuint32_t R;
        struct {
            vuint32_t VLD:1;
            vuint32_t:14;
            vuint32_t ECX:1;
            vuint32_t GPE:1;
            vuint32_t CPE:1;
            vuint32_t ERRCHN:6;
            vuint32_t SAE:1;
            vuint32_t SOE:1;
            vuint32_t DAE:1;
            vuint32_t DOE:1;
            vuint32_t NCE:1;
            vuint32_t SGE:1;
            vuint32_t SBE:1;
            vuint32_t DBE:1;
        } B;
    } ES;

    union {
        vuint32_t R;
        struct {
            vuint32_t ERQ63:1;
            vuint32_t ERQ62:1;
            vuint32_t ERQ61:1;
            vuint32_t ERQ60:1;
            vuint32_t ERQ59:1;
            vuint32_t ERQ58:1;
            vuint32_t ERQ57:1;
            vuint32_t ERQ56:1;
            vuint32_t ERQ55:1;
            vuint32_t ERQ54:1;
            vuint32_t ERQ53:1;
            vuint32_t ERQ52:1;
            vuint32_t ERQ51:1;
            vuint32_t ERQ50:1;
            vuint32_t ERQ49:1;
            vuint32_t ERQ48:1;
            vuint32_t ERQ47:1;
            vuint32_t ERQ46:1;
            vuint32_t ERQ45:1;
            vuint32_t ERQ44:1;
            vuint32_t ERQ43:1;
            vuint32_t ERQ42:1;
            vuint32_t ERQ41:1;
            vuint32_t ERQ40:1;
            vuint32_t ERQ39:1;
            vuint32_t ERQ38:1;
            vuint32_t ERQ37:1;
            vuint32_t ERQ36:1;
            vuint32_t ERQ35:1;
            vuint32_t ERQ34:1;
            vuint32_t ERQ33:1;
            vuint32_t ERQ32:1;
        } B;
    } ERQH;

    union {
        vuint32_t R;
        struct {
            vuint32_t ERQ31:1;
            vuint32_t ERQ30:1;
            vuint32_t ERQ29:1;
            vuint32_t ERQ28:1;
            vuint32_t ERQ27:1;
            vuint32_t ERQ26:1;
            vuint32_t ERQ25:1;
            vuint32_t ERQ24:1;
            vuint32_t ERQ23:1;
            vuint32_t ERQ22:1;
            vuint32_t ERQ21:1;
            vuint32_t ERQ20:1;
            vuint32_t ERQ19:1;
            vuint32_t ERQ18:1;
            vuint32_t ERQ17:1;
            vuint32_t ERQ16:1;
            vuint32_t ERQ15:1;
            vuint32_t ERQ14:1;
            vuint32_t ERQ13:1;
            vuint32_t ERQ12:1;
            vuint32_t ERQ11:1;
            vuint32_t ERQ10:1;
            vuint32_t ERQ9:1;
            vuint32_t ERQ8:1;
            vuint32_t ERQ7:1;
            vuint32_t ERQ6:1;
            vuint32_t ERQ5:1;
            vuint32_t ERQ4:1;
            vuint32_t ERQ3:1;
            vuint32_t ERQ2:1;
            vuint32_t ERQ1:1;
            vuint32_t ERQ0:1;
        } B;
    } ERQL;

    union {
        vuint32_t R;
        struct {
            vuint32_t EEI63:1;
            vuint32_t EEI62:1;
            vuint32_t EEI61:1;
            vuint32_t EEI60:1;
            vuint32_t EEI59:1;
            vuint32_t EEI58:1;
            vuint32_t EEI57:1;
            vuint32_t EEI56:1;
            vuint32_t EEI55:1;
            vuint32_t EEI54:1;
            vuint32_t EEI53:1;
            vuint32_t EEI52:1;
            vuint32_t EEI51:1;
            vuint32_t EEI50:1;
            vuint32_t EEI49:1;
            vuint32_t EEI48:1;
            vuint32_t EEI47:1;
            vuint32_t EEI46:1;
            vuint32_t EEI45:1;
            vuint32_t EEI44:1;
            vuint32_t EEI43:1;
            vuint32_t EEI42:1;
            vuint32_t EEI41:1;
            vuint32_t EEI40:1;
            vuint32_t EEI39:1;
            vuint32_t EEI38:1;
            vuint32_t EEI37:1;
            vuint32_t EEI36:1;
            vuint32_t EEI35:1;
            vuint32_t EEI34:1;
            vuint32_t EEI33:1;
            vuint32_t EEI32:1;
        } B;
    } EEIH;

    union {
        vuint32_t R;
        struct {
            vuint32_t EEI31:1;
            vuint32_t EEI30:1;
            vuint32_t EEI29:1;
            vuint32_t EEI28:1;
            vuint32_t EEI27:1;
            vuint32_t EEI26:1;
            vuint32_t EEI25:1;
            vuint32_t EEI24:1;
            vuint32_t EEI23:1;
            vuint32_t EEI22:1;
            vuint32_t EEI21:1;
            vuint32_t EEI20:1;
            vuint32_t EEI19:1;
            vuint32_t EEI18:1;
            vuint32_t EEI17:1;
            vuint32_t EEI16:1;
            vuint32_t EEI15:1;
            vuint32_t EEI14:1;
            vuint32_t EEI13:1;
            vuint32_t EEI12:1;
            vuint32_t EEI11:1;
            vuint32_t EEI10:1;
            vuint32_t EEI9:1;
            vuint32_t EEI8:1;
            vuint32_t EEI7:1;
            vuint32_t EEI6:1;
            vuint32_t EEI5:1;
            vuint32_t EEI4:1;
            vuint32_t EEI3:1;
            vuint32_t EEI2:1;
            vuint32_t EEI1:1;
            vuint32_t EEI0:1;
        } B;
    } EEIL;

    union {
        vuint8_t R;
        struct {
            vuint8_t NOP:1;
            vuint8_t SAER:1;
            vuint8_t SERQ:6;
        } B;
    } SERQ;

    union {
        vuint8_t R;
        struct {
            vuint8_t NOP:1;
            vuint8_t CAER:1;
            vuint8_t CERQ:6;
        } B;
    } CERQ;

    union {
        vuint8_t R;
        struct {
            vuint8_t NOP:1;
            vuint8_t SAEE:1;
            vuint8_t SEEI:6;
        } B;
    } SEEI;

    union {
        vuint8_t R;
        struct {
            vuint8_t NOP:1;
            vuint8_t CAEE:1;
            vuint8_t CEEI:6;
        } B;
    } CEEI;

    union {
        vuint8_t R;
        struct {
            vuint8_t NOP:1;
            vuint8_t CAIR:1;
            vuint8_t CINT:6;
        } B;
    } CINT;

    union {
        vuint8_t R;
        struct {
            vuint8_t NOP:1;
            vuint8_t CAEI:1;
            vuint8_t CERR:6;
        } B;
    } CERR;

    union {
        vuint8_t R;
        struct {
            vuint8_t NOP:1;
            vuint8_t SAST:1;
            vuint8_t SSRT:6;
        } B;
    } SSRT;

    union {
        vuint8_t R;
        struct {
            vuint8_t NOP:1;
            vuint8_t CADN:1;
            vuint8_t CDNE:6;
        } B;
    } CDNE;

    union {
        vuint32_t R;
        struct {
            vuint32_t INT63:1;
            vuint32_t INT62:1;
            vuint32_t INT61:1;
            vuint32_t INT60:1;
            vuint32_t INT59:1;
            vuint32_t INT58:1;
            vuint32_t INT57:1;
            vuint32_t INT56:1;
            vuint32_t INT55:1;
            vuint32_t INT54:1;
            vuint32_t INT53:1;
            vuint32_t INT52:1;
            vuint32_t INT51:1;
            vuint32_t INT50:1;
            vuint32_t INT49:1;
            vuint32_t INT48:1;
            vuint32_t INT47:1;
            vuint32_t INT46:1;
            vuint32_t INT45:1;
            vuint32_t INT44:1;
            vuint32_t INT43:1;
            vuint32_t INT42:1;
            vuint32_t INT41:1;
            vuint32_t INT40:1;
            vuint32_t INT39:1;
            vuint32_t INT38:1;
            vuint32_t INT37:1;
            vuint32_t INT36:1;
            vuint32_t INT35:1;
            vuint32_t INT34:1;
            vuint32_t INT33:1;
            vuint32_t INT32:1;
        } B;
    } INTH;

    union {
        vuint32_t R;
        struct {
            vuint32_t INT31:1;
            vuint32_t INT30:1;
            vuint32_t INT29:1;
            vuint32_t INT28:1;
            vuint32_t INT27:1;
            vuint32_t INT26:1;
            vuint32_t INT25:1;
            vuint32_t INT24:1;
            vuint32_t INT23:1;
            vuint32_t INT22:1;
            vuint32_t INT21:1;
            vuint32_t INT20:1;
            vuint32_t INT19:1;
            vuint32_t INT18:1;
            vuint32_t INT17:1;
            vuint32_t INT16:1;
            vuint32_t INT15:1;
            vuint32_t INT14:1;
            vuint32_t INT13:1;
            vuint32_t INT12:1;
            vuint32_t INT11:1;
            vuint32_t INT10:1;
            vuint32_t INT9:1;
            vuint32_t INT8:1;
            vuint32_t INT7:1;
            vuint32_t INT6:1;
            vuint32_t INT5:1;
            vuint32_t INT4:1;
            vuint32_t INT3:1;
            vuint32_t INT2:1;
            vuint32_t INT1:1;
            vuint32_t INT0:1;
        } B;
    } INTL;

    union {
        vuint32_t R;
        struct {
            vuint32_t ERR63:1;
            vuint32_t ERR62:1;
            vuint32_t ERR61:1;
            vuint32_t ERR60:1;
            vuint32_t ERR59:1;
            vuint32_t ERR58:1;
            vuint32_t ERR57:1;
            vuint32_t ERR56:1;
            vuint32_t ERR55:1;
            vuint32_t ERR54:1;
            vuint32_t ERR53:1;
            vuint32_t ERR52:1;
            vuint32_t ERR51:1;
            vuint32_t ERR50:1;
            vuint32_t ERR49:1;
            vuint32_t ERR48:1;
            vuint32_t ERR47:1;
            vuint32_t ERR46:1;
            vuint32_t ERR45:1;
            vuint32_t ERR44:1;
            vuint32_t ERR43:1;
            vuint32_t ERR42:1;
            vuint32_t ERR41:1;
            vuint32_t ERR40:1;
            vuint32_t ERR39:1;
            vuint32_t ERR38:1;
            vuint32_t ERR37:1;
            vuint32_t ERR36:1;
            vuint32_t ERR35:1;
            vuint32_t ERR34:1;
            vuint32_t ERR33:1;
            vuint32_t ERR32:1;
        } B;
    } ERRH;

    union {
        vuint32_t R;
        struct {
            vuint32_t ERR31:1;
            vuint32_t ERR30:1;
            vuint32_t ERR29:1;
            vuint32_t ERR28:1;
            vuint32_t ERR27:1;
            vuint32_t ERR26:1;
            vuint32_t ERR25:1;
            vuint32_t ERR24:1;
            vuint32_t ERR23:1;
            vuint32_t ERR22:1;
            vuint32_t ERR21:1;
            vuint32_t ERR20:1;
            vuint32_t ERR19:1;
            vuint32_t ERR18:1;
            vuint32_t ERR17:1;
            vuint32_t ERR16:1;
            vuint32_t ERR15:1;
            vuint32_t ERR14:1;
            vuint32_t ERR13:1;
            vuint32_t ERR12:1;
            vuint32_t ERR11:1;
            vuint32_t ERR10:1;
            vuint32_t ERR9:1;
            vuint32_t ERR8:1;
            vuint32_t ERR7:1;
            vuint32_t ERR6:1;
            vuint32_t ERR5:1;
            vuint32_t ERR4:1;
            vuint32_t ERR3:1;
            vuint32_t ERR2:1;
            vuint32_t ERR1:1;
            vuint32_t ERR0:1;
        } B;
    } ERRL;

    union {
        vuint32_t R;
        struct {
            vuint32_t HRS63:1;
            vuint32_t HRS62:1;
            vuint32_t HRS61:1;
            vuint32_t HRS60:1;
            vuint32_t HRS59:1;
            vuint32_t HRS58:1;
            vuint32_t HRS57:1;
            vuint32_t HRS56:1;
            vuint32_t HRS55:1;
            vuint32_t HRS54:1;
            vuint32_t HRS53:1;
            vuint32_t HRS52:1;
            vuint32_t HRS51:1;
            vuint32_t HRS50:1;
            vuint32_t HRS49:1;
            vuint32_t HRS48:1;
            vuint32_t HRS47:1;
            vuint32_t HRS46:1;
            vuint32_t HRS45:1;
            vuint32_t HRS44:1;
            vuint32_t HRS43:1;
            vuint32_t HRS42:1;
            vuint32_t HRS41:1;
            vuint32_t HRS40:1;
            vuint32_t HRS39:1;
            vuint32_t HRS38:1;
            vuint32_t HRS37:1;
            vuint32_t HRS36:1;
            vuint32_t HRS35:1;
            vuint32_t HRS34:1;
            vuint32_t HRS33:1;
            vuint32_t HRS32:1;
        } B;
    } HRSH;

    union {
        vuint32_t R;
        struct {
            vuint32_t HRS31:1;
            vuint32_t HRS30:1;
            vuint32_t HRS29:1;
            vuint32_t HRS28:1;
            vuint32_t HRS27:1;
            vuint32_t HRS26:1;
            vuint32_t HRS25:1;
            vuint32_t HRS24:1;
            vuint32_t HRS23:1;
            vuint32_t HRS22:1;
            vuint32_t HRS21:1;
            vuint32_t HRS20:1;
            vuint32_t HRS19:1;
            vuint32_t HRS18:1;
            vuint32_t HRS17:1;
            vuint32_t HRS16:1;
            vuint32_t HRS15:1;
            vuint32_t HRS14:1;
            vuint32_t HRS13:1;
            vuint32_t HRS12:1;
            vuint32_t HRS11:1;
            vuint32_t HRS10:1;
            vuint32_t HRS9:1;
            vuint32_t HRS8:1;
            vuint32_t HRS7:1;
            vuint32_t HRS6:1;
            vuint32_t HRS5:1;
            vuint32_t HRS4:1;
            vuint32_t HRS3:1;
            vuint32_t HRS2:1;
            vuint32_t HRS1:1;
            vuint32_t HRS0:1;
        } B;
    } HRSL;

    uint8_t DMA_reserved1[200];

    union {
        vuint8_t R;
        struct {
            vuint8_t ECP:1;
            vuint8_t DPA:1;
            vuint8_t GRPPRI:2;
            vuint8_t CHPRI:4;
        } B;
    } DCHPRI[64];

    union {
        vuint8_t R;
        struct {
            vuint8_t EMI:1;
            vuint8_t PAL:1;
            vuint8_t:2;
            vuint8_t MID:4;
        } B;
    } DCHMID[64];

    uint8_t DMA_reserved2[3712];

/****************************************************************************/
/*       DMA Transfer Control Descriptor                                   */
/****************************************************************************/

    struct tcd_t {              /*for "standard" format TCDs (when EDMA.TCD[x].CITER.E_LINK==BITER.E_LINK=0 && EDMA.EMLM=0 ) */
        vuint32_t SADDR;        /* source address */

        vuint16_t SMOD:5;       /* source address modulo */
        vuint16_t SSIZE:3;      /* source transfer size */
        vuint16_t DMOD:5;       /* destination address modulo */
        vuint16_t DSIZE:3;      /* destination transfer size */
        vint16_t SOFF;          /* signed source address offset */

        vuint32_t NBYTES;       /* inner (“minor”) byte count */
        //vuint32_t NBYTES:32;    /* MLNO */
        // vuint32_t SMLOE:1;         /* MLNOFFNO */
        // vuint32_t DMLOE:1;
        // vuint32_t NBYTES:30;
        // vuint32_t SMLOE:1;         /* MLNOFFYES */
        // vuint32_t DMLOE:1;
        // vuint32_t MLOFF: 20;
        // vuint32_t NBYTES:10;

        vint32_t SLAST;         /* last destination address adjustment, or

                                   scatter/gather address (if e_sg = 1) */
        vuint32_t DADDR;        /* destination address */

        vuint16_t CITER_ELINK:1;        /* ELINK yes */
        vuint16_t CITER_LINKCH:6;
        vuint16_t CITER:9;
        // vuint16_t ELINK:1;         /* ELINK no */
        // vuint16_t CITER:15;

        vint16_t DOFF;          /* signed destination address offset */

        vint32_t DLAST_SGA;

        vuint16_t BITER_ELINK:1;        /* ELINK yes */
        vuint16_t BITER_LINKCH:6;
        vuint16_t BITER:9;
        // vuint16_t ELINK:1;         /* ELINK no */
        // vuint16_t BITER:15;

        vuint16_t BWC:2;        /* bandwidth control */
        vuint16_t MAJORLINKCH:6;        /* enable channel-to-channel link */
        vuint16_t DONE:1;       /* channel done */
        vuint16_t ACTIVE:1;     /* channel active */
        vuint16_t MAJORELINK:1; /* enable channel-to-channel link */
        vuint16_t ESG:1;        /* enable scatter/gather descriptor */
        vuint16_t DREQ:1;       /* disable ipd_req when done */
        vuint16_t INTHALF:1;    /* interrupt on citer = (biter >> 1) */
        vuint16_t INTMAJ:1;     /* interrupt on major loop completion */
        vuint16_t START:1;      /* explicit channel start */

    } TCD[64];                  /* transfer_control_descriptor */
};
/**************************************************************************/
/*                   Module: DMACHMUX                                     */
/**************************************************************************/
struct DMACHMUX_tag {
    union {
        vuint8_t R;
        struct {
            vuint8_t ENBL:1;
            vuint8_t TRIG:1;
            vuint8_t SOURCE:6;
        } B;
    } CHCONFIG[16];
};
/**************************************************************************/
/*                   Module: DSPI                                         */
/**************************************************************************/
struct DSPI_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t MSTR:1;
            vuint32_t CONT_SCKE:1;
            vuint32_t DCONF:2;
            vuint32_t FRZ:1;
            vuint32_t MTFE:1;
            vuint32_t PCSSE:1;
            vuint32_t ROOE:1;
            vuint32_t PCSIS:8;
            vuint32_t DOZE:1;
            vuint32_t MDIS:1;
            vuint32_t DIS_TXF:1;
            vuint32_t DIS_RXF:1;
            vuint32_t CLR_TXF:1;
            vuint32_t CLR_RXF:1;
            vuint32_t SMPL_PT:2;
            vuint32_t:5;
            vuint32_t FCPCS:1;
            vuint32_t PES:1;
            vuint32_t HALT:1;
        } B;
    } MCR;

    uint8_t DSPI_reserved1[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t SPI_TCNT:16;
            vuint32_t:16;
        } B;
    } TCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t DBR:1;    /* Master Mode */
            vuint32_t FMSZ:4;
            vuint32_t CPOL:1;
            vuint32_t CPHA:1;
            vuint32_t LSBFE:1;
            vuint32_t PCSSCK:2;
            vuint32_t PASC:2;
            vuint32_t PDT:2;
            vuint32_t PBR:2;
            vuint32_t CSSCK:4;
            vuint32_t ASC:4;
            vuint32_t DT:4;
            vuint32_t BR:4;
            // vuint32_t FMSZ:5; /* Slave Mode */
            // vuint32_t CPOL:1;
            // vuint32_t CPHA:1;
            // vuint32_t PE:1;
            // vuint32_t PP:1;
            // vuint32_t:23;
        } B;
    } CTAR[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t TCF:1;
            vuint32_t TXRXS:1;
            vuint32_t SPITCF:1;
            vuint32_t EOQF:1;
            vuint32_t TFUF:1;
            vuint32_t DSITCF:1;
            vuint32_t TFFF:1;
            vuint32_t:1;
            vuint32_t:1;
            vuint32_t DPEF:1;
            vuint32_t SPEF:1;
            vuint32_t DDIF:1;
            vuint32_t RFOF:1;
            vuint32_t:1;
            vuint32_t RFDF:1;
            vuint32_t:1;
            vuint32_t TXCTR:4;
            vuint32_t TXNXTPTR:4;
            vuint32_t RXCTR:4;
            vuint32_t POPNXTPTR:4;
        } B;
    } SR;

    union {
        vuint32_t R;
        struct {
            vuint32_t TCF_RE:1;
            vuint32_t:1;
            vuint32_t SPITCF_RE:1;
            vuint32_t EOQF_RE:1;
            vuint32_t TFUF_RE:1;
            vuint32_t DSITCF_RE:1;
            vuint32_t TFFF_RE:1;
            vuint32_t TFFF_DIRS:1;
            vuint32_t:1;
            vuint32_t DPEF_RE:1;
            vuint32_t SPEF_RE:1;
            vuint32_t DDIF_RE:1;
            vuint32_t RFOF_RE:1;
            vuint32_t:1;
            vuint32_t RFDF_RE:1;
            vuint32_t RFDF_DIRS:1;
            vuint32_t:16;
        } B;
    } RSER;

    union {
        vuint32_t R;
        struct {
            vuint32_t CONT:1;   /* Master Mode */
            vuint32_t CTAS:3;
            vuint32_t EOQ:1;
            vuint32_t CTCNT:1;
            vuint32_t PE_MASC:1;
            vuint32_t PP_MCSC:1;
            vuint32_t PCS:8;
            vuint32_t TXDATA:16;
            // vuint32_t TXDATA:32; /* Slave Mode */
        } B;
    } PUSHR;

    union {
        vuint32_t R;
        struct {
            vuint32_t RXDATA:32;
        } B;
    } POPR;

    union {
        vuint32_t R;
        struct {
            vuint32_t TXCMD_TXDATA:16;
            vuint32_t TXDATA:16;
        } B;
    } TXFR[4];

    uint8_t DSPI_reserved2[48];

    union {
        vuint32_t R;
        struct {
            vuint32_t RXDATA:32;
        } B;
    } RXFR[4];

    uint8_t DSPI_reserved3[48];

    union {
        vuint32_t R;
        struct {
            vuint32_t MTOE:1;
            vuint32_t FMSZ:1;
            vuint32_t MTOCNT:6;
            vuint32_t:1;
            vuint32_t TRG:1;
            vuint32_t ITSB:1;
            vuint32_t TSBC:1;
            vuint32_t TXSS:1;
            vuint32_t TPOL:1;
            vuint32_t TRRE:1;
            vuint32_t CID:1;
            vuint32_t DCONT:1;
            vuint32_t DSICTAS:3;
            vuint32_t DMS:1;
            vuint32_t PES:1;
            vuint32_t PE:1;
            vuint32_t PP:1;
            vuint32_t DPCSx:8;
        } B;
    } DSICR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t SER_DATA:32;
        } B;
    } SDR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t ASER_DATA:32;
        } B;
    } ASDR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t COMP_DATA:32;
        } B;
    } COMPR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t DESER_DATA:32;
        } B;
    } DDR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t:3;
            vuint32_t TSBCNT:5;
            vuint32_t:6;
            vuint32_t DSE1:1;
            vuint32_t DSE0:1;
            vuint32_t TRGPRD:8;
            vuint32_t DPCS1_x:8;
        } B;
    } DSICR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t SS:32;
        } B;
    } SSR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t IPS7:4;
            vuint32_t IPS6:4;
            vuint32_t IPS5:4;
            vuint32_t IPS4:4;
            vuint32_t IPS3:4;
            vuint32_t IPS2:4;
            vuint32_t IPS1:4;
            vuint32_t IPS0:4;
        } B;
    } PISR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t IPS15:4;
            vuint32_t IPS14:4;
            vuint32_t IPS13:4;
            vuint32_t IPS12:4;
            vuint32_t IPS11:4;
            vuint32_t IPS10:4;
            vuint32_t IPS9:4;
            vuint32_t IPS8:4;
        } B;
    } PISR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t IPS23:4;
            vuint32_t IPS22:4;
            vuint32_t IPS21:4;
            vuint32_t IPS20:4;
            vuint32_t IPS19:4;
            vuint32_t IPS18:4;
            vuint32_t IPS17:4;
            vuint32_t IPS16:4;
        } B;
    } PISR2;

    union {
        vuint32_t R;
        struct {
            vuint32_t IPS31:4;
            vuint32_t IPS30:4;
            vuint32_t IPS29:4;
            vuint32_t IPS28:4;
            vuint32_t IPS27:4;
            vuint32_t IPS26:4;
            vuint32_t IPS25:4;
            vuint32_t IPS24:4;
        } B;
    } PISR3;

    union {
        vuint32_t R;
        struct {
            vuint32_t MASK:32;
        } B;
    } DIMR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t DP:32;
        } B;
    } DPIR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t SER_DATA:32;
        } B;
    } SDR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t ASER_DATA:32;
        } B;
    } ASDR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t COMP_DATA:32;
        } B;
    } COMPR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t DESER_DATA:32;
        } B;
    } DDR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t SS:32;
        } B;
    } SSR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t IPS39:4;
            vuint32_t IPS38:4;
            vuint32_t IPS37:4;
            vuint32_t IPS36:4;
            vuint32_t IPS35:4;
            vuint32_t IPS34:4;
            vuint32_t IPS33:4;
            vuint32_t IPS32:4;
        } B;
    } PISR4;

    union {
        vuint32_t R;
        struct {
            vuint32_t IPS47:4;
            vuint32_t IPS46:4;
            vuint32_t IPS45:4;
            vuint32_t IPS44:4;
            vuint32_t IPS43:4;
            vuint32_t IPS42:4;
            vuint32_t IPS41:4;
            vuint32_t IPS40:4;
        } B;
    } PISR5;

    union {
        vuint32_t R;
        struct {
            vuint32_t IPS55:4;
            vuint32_t IPS54:4;
            vuint32_t IPS53:4;
            vuint32_t IPS52:4;
            vuint32_t IPS51:4;
            vuint32_t IPS50:4;
            vuint32_t IPS49:4;
            vuint32_t IPS48:4;
        } B;
    } PISR6;

    union {
        vuint32_t R;
        struct {
            vuint32_t IPS63:4;
            vuint32_t IPS62:4;
            vuint32_t IPS61:4;
            vuint32_t IPS60:4;
            vuint32_t IPS59:4;
            vuint32_t IPS58:4;
            vuint32_t IPS57:4;
            vuint32_t IPS56:4;
        } B;
    } PISR7;

    union {
        vuint32_t R;
        struct {
            vuint32_t MASK:32;
        } B;
    } DIMR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t DP:32;
        } B;
    } DPIR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t:15;
            vuint32_t FMSZE:1;
            vuint32_t:5;
            vuint32_t DTCP:11;
        } B;
    } CTARE[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t CMDCTR:4;
            vuint32_t CMDNXTPTR:4;
        } B;
    } SREX;
};
/**************************************************************************/
/*                   Module: DTS                                          */
/**************************************************************************/
struct DTS_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:30;
            vuint32_t DTS_EN_B:1;
            vuint32_t DTS_EN:1;
        } B;
    } ENABLE;

    union {
        vuint32_t R;
        struct {
            vuint32_t AD:32;
        } B;
    } STARTUP;

    union {
        vuint32_t R;
        struct {
            vuint32_t ST:32;
        } B;
    } SEMAPHORE;

    union {
        vuint32_t R;
        struct {
            vuint32_t ST_B:32;
        } B;
    } SEMAPHORE_B;
};
/**************************************************************************/
/*                   Module: FCCU                                        */
/**************************************************************************/
struct FCCU_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:22;
            vuint32_t DEBUG:1;
            vuint32_t:1;
            vuint32_t OPS:2;
            vuint32_t:1;
            vuint32_t OPR:5;
        } B;
    } CTRL;

    union {
        vuint32_t R;
        struct {
            vuint32_t CTRLK:32;
        } B;
    } CTRLK;

    union {
        vuint32_t R;
        struct {
            vuint32_t:7;
            vuint32_t FCCU_SET_AFTER_RESET:1;
            vuint32_t FCCU_SET_CLEAR:2;
            vuint32_t:10;
            vuint32_t CM:1;
            vuint32_t SM:1;
            vuint32_t PS:1;
            vuint32_t FOM:3;
            vuint32_t:6;
        } B;
    } CFG;

    uint8_t FCCU_reserved1[16];

    union {
        vuint32_t R;
        struct {
            vuint32_t NCFC31:1;
            vuint32_t NCFC30:1;
            vuint32_t NCFC29:1;
            vuint32_t NCFC28:1;
            vuint32_t NCFC27:1;
            vuint32_t NCFC26:1;
            vuint32_t NCFC25:1;
            vuint32_t NCFC24:1;
            vuint32_t NCFC23:1;
            vuint32_t NCFC22:1;
            vuint32_t NCFC21:1;
            vuint32_t NCFC20:1;
            vuint32_t NCFC19:1;
            vuint32_t NCFC18:1;
            vuint32_t NCFC17:1;
            vuint32_t NCFC16:1;
            vuint32_t NCFC15:1;
            vuint32_t NCFC14:1;
            vuint32_t NCFC13:1;
            vuint32_t NCFC12:1;
            vuint32_t NCFC11:1;
            vuint32_t NCFC10:1;
            vuint32_t NCFC9:1;
            vuint32_t NCFC8:1;
            vuint32_t NCFC7:1;
            vuint32_t NCFC6:1;
            vuint32_t NCFC5:1;
            vuint32_t NCFC4:1;
            vuint32_t NCFC3:1;
            vuint32_t NCFC2:1;
            vuint32_t NCFC1:1;
            vuint32_t NCFC0:1;
        } B;
    } NCF_CFG[4];

    uint8_t FCCU_reserved2[32];

    union {
        vuint32_t R;
        struct {
            vuint32_t NCFSC15:2;
            vuint32_t NCFSC14:2;
            vuint32_t NCFSC13:2;
            vuint32_t NCFSC12:2;
            vuint32_t NCFSC11:2;
            vuint32_t NCFSC10:2;
            vuint32_t NCFSC9:2;
            vuint32_t NCFSC8:2;
            vuint32_t NCFSC7:2;
            vuint32_t NCFSC6:2;
            vuint32_t NCFSC5:2;
            vuint32_t NCFSC4:2;
            vuint32_t NCFSC3:2;
            vuint32_t NCFSC2:2;
            vuint32_t NCFSC1:2;
            vuint32_t NCFSC0:2;
        } B;
    } NCFS_CFG[8];

    uint8_t FCCU_reserved3[20];

    union {
        vuint32_t R;
        struct {
            vuint32_t NCFS31:1;
            vuint32_t NCFS30:1;
            vuint32_t NCFS29:1;
            vuint32_t NCFS28:1;
            vuint32_t NCFS27:1;
            vuint32_t NCFS26:1;
            vuint32_t NCFS25:1;
            vuint32_t NCFS24:1;
            vuint32_t NCFS23:1;
            vuint32_t NCFS22:1;
            vuint32_t NCFS21:1;
            vuint32_t NCFS20:1;
            vuint32_t NCFS19:1;
            vuint32_t NCFS18:1;
            vuint32_t NCFS17:1;
            vuint32_t NCFS16:1;
            vuint32_t NCFS15:1;
            vuint32_t NCFS14:1;
            vuint32_t NCFS13:1;
            vuint32_t NCFS12:1;
            vuint32_t NCFS11:1;
            vuint32_t NCFS10:1;
            vuint32_t NCFS9:1;
            vuint32_t NCFS8:1;
            vuint32_t NCFS7:1;
            vuint32_t NCFS6:1;
            vuint32_t NCFS5:1;
            vuint32_t NCFS4:1;
            vuint32_t NCFS3:1;
            vuint32_t NCFS2:1;
            vuint32_t NCFS1:1;
            vuint32_t NCFS0:1;
        } B;
    } NCFS[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t NCFK:32;
        } B;
    } NCFK;

    union {
        vuint32_t R;
        struct {
            vuint32_t NCFE31:1;
            vuint32_t NCFE30:1;
            vuint32_t NCFE29:1;
            vuint32_t NCFE28:1;
            vuint32_t NCFE27:1;
            vuint32_t NCFE26:1;
            vuint32_t NCFE25:1;
            vuint32_t NCFE24:1;
            vuint32_t NCFE23:1;
            vuint32_t NCFE22:1;
            vuint32_t NCFE21:1;
            vuint32_t NCFE20:1;
            vuint32_t NCFE19:1;
            vuint32_t NCFE18:1;
            vuint32_t NCFE17:1;
            vuint32_t NCFE16:1;
            vuint32_t NCFE15:1;
            vuint32_t NCFE14:1;
            vuint32_t NCFE13:1;
            vuint32_t NCFE12:1;
            vuint32_t NCFE11:1;
            vuint32_t NCFE10:1;
            vuint32_t NCFE9:1;
            vuint32_t NCFE8:1;
            vuint32_t NCFE7:1;
            vuint32_t NCFE6:1;
            vuint32_t NCFE5:1;
            vuint32_t NCFE4:1;
            vuint32_t NCFE3:1;
            vuint32_t NCFE2:1;
            vuint32_t NCFE1:1;
            vuint32_t NCFE0:1;
        } B;
    } NCFE[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t NCFTOE31:1;
            vuint32_t NCFTOE30:1;
            vuint32_t NCFTOE29:1;
            vuint32_t NCFTOE28:1;
            vuint32_t NCFTOE27:1;
            vuint32_t NCFTOE26:1;
            vuint32_t NCFTOE25:1;
            vuint32_t NCFTOE24:1;
            vuint32_t NCFTOE23:1;
            vuint32_t NCFTOE22:1;
            vuint32_t NCFTOE21:1;
            vuint32_t NCFTOE20:1;
            vuint32_t NCFTOE19:1;
            vuint32_t NCFTOE18:1;
            vuint32_t NCFTOE17:1;
            vuint32_t NCFTOE16:1;
            vuint32_t NCFTOE15:1;
            vuint32_t NCFTOE14:1;
            vuint32_t NCFTOE13:1;
            vuint32_t NCFTOE12:1;
            vuint32_t NCFTOE11:1;
            vuint32_t NCFTOE10:1;
            vuint32_t NCFTOE9:1;
            vuint32_t NCFTOE8:1;
            vuint32_t NCFTOE7:1;
            vuint32_t NCFTOE6:1;
            vuint32_t NCFTOE5:1;
            vuint32_t NCFTOE4:1;
            vuint32_t NCFTOE3:1;
            vuint32_t NCFTOE2:1;
            vuint32_t NCFTOE1:1;
            vuint32_t NCFTOE0:1;
        } B;
    } NCF_TOE[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t TO:32;
        } B;
    } NCF_TO;

    union {
        vuint32_t R;
        struct {
            vuint32_t:29;
            vuint32_t TO:3;
        } B;
    } CFG_TO;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t EIN1:1;
            vuint32_t EIN0:1;
            vuint32_t:2;
            vuint32_t EOUT1:1;
            vuint32_t EOUT0:1;
        } B;
    } EINOUT;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t PSTAT:2;
            vuint32_t ESTAT:1;
            vuint32_t STATUS:3;
        } B;
    } STAT;

    union {
        vuint32_t R;
        struct {
            vuint32_t:23;
            vuint32_t NAFS:9;
        } B;
    } NAFS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:22;
            vuint32_t SRC:2;
            vuint32_t AFFS:8;
        } B;
    } AFFS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:22;
            vuint32_t SRC:2;
            vuint32_t NFFS:8;
        } B;
    } NFFS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:23;
            vuint32_t FAFS:9;
        } B;
    } FAFS;

    uint8_t FCCU_reserved4[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t FNCFC:7;
        } B;
    } NCFF;

    union {
        vuint32_t R;
        struct {
            vuint32_t:29;
            vuint32_t NMI_STAT:1;
            vuint32_t ALRM_STAT:1;
            vuint32_t CFG_TO_STAT:1;
        } B;
    } IRQ_STAT;

    union {
        vuint32_t R;
        struct {
            vuint32_t:31;
            vuint32_t CFG_TO_IEN:1;
        } B;
    } IRQ_EN;

    union {
        vuint32_t R;
        struct {
            vuint32_t XTMR:32;
        } B;
    } XTMR;

    union {
        vuint32_t R;
        struct {
            vuint32_t VL3:1;
            vuint32_t FS3:1;
            vuint32_t:2;
            vuint32_t MCS3:4;
            vuint32_t VL2:1;
            vuint32_t FS2:1;
            vuint32_t:2;
            vuint32_t MCS2:4;
            vuint32_t VL1:1;
            vuint32_t FS1:1;
            vuint32_t:2;
            vuint32_t MCS1:4;
            vuint32_t VL0:1;
            vuint32_t FS0:1;
            vuint32_t:2;
            vuint32_t MCS0:4;
        } B;
    } MCS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t TRANSKEY:7;
        } B;
    } TRANS_LOCK;

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t PERMNTKEY:7;
        } B;
    } PERMNT_LOCK;

    union {
        vuint32_t R;
        struct {
            vuint32_t:18;
            vuint32_t DELTA_T:14;
        } B;
    } DELTA_T;

    union {
        vuint32_t R;
        struct {
            vuint32_t IRQEN31:1;
            vuint32_t IRQEN30:1;
            vuint32_t IRQEN29:1;
            vuint32_t IRQEN28:1;
            vuint32_t IRQEN27:1;
            vuint32_t IRQEN26:1;
            vuint32_t IRQEN25:1;
            vuint32_t IRQEN24:1;
            vuint32_t IRQEN23:1;
            vuint32_t IRQEN22:1;
            vuint32_t IRQEN21:1;
            vuint32_t IRQEN20:1;
            vuint32_t IRQEN19:1;
            vuint32_t IRQEN18:1;
            vuint32_t IRQEN17:1;
            vuint32_t IRQEN16:1;
            vuint32_t IRQEN15:1;
            vuint32_t IRQEN14:1;
            vuint32_t IRQEN13:1;
            vuint32_t IRQEN12:1;
            vuint32_t IRQEN11:1;
            vuint32_t IRQEN10:1;
            vuint32_t IRQEN9:1;
            vuint32_t IRQEN8:1;
            vuint32_t IRQEN7:1;
            vuint32_t IRQEN6:1;
            vuint32_t IRQEN5:1;
            vuint32_t IRQEN4:1;
            vuint32_t IRQEN3:1;
            vuint32_t IRQEN2:1;
            vuint32_t IRQEN1:1;
            vuint32_t IRQEN0:1;
        } B;
    } IRQ_ALARM_EN[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t NMIEN31:1;
            vuint32_t NMIEN30:1;
            vuint32_t NMIEN29:1;
            vuint32_t NMIEN28:1;
            vuint32_t NMIEN27:1;
            vuint32_t NMIEN26:1;
            vuint32_t NMIEN25:1;
            vuint32_t NMIEN24:1;
            vuint32_t NMIEN23:1;
            vuint32_t NMIEN22:1;
            vuint32_t NMIEN21:1;
            vuint32_t NMIEN20:1;
            vuint32_t NMIEN19:1;
            vuint32_t NMIEN18:1;
            vuint32_t NMIEN17:1;
            vuint32_t NMIEN16:1;
            vuint32_t NMIEN15:1;
            vuint32_t NMIEN14:1;
            vuint32_t NMIEN13:1;
            vuint32_t NMIEN12:1;
            vuint32_t NMIEN11:1;
            vuint32_t NMIEN10:1;
            vuint32_t NMIEN9:1;
            vuint32_t NMIEN8:1;
            vuint32_t NMIEN7:1;
            vuint32_t NMIEN6:1;
            vuint32_t NMIEN5:1;
            vuint32_t NMIEN4:1;
            vuint32_t NMIEN3:1;
            vuint32_t NMIEN2:1;
            vuint32_t NMIEN1:1;
            vuint32_t NMIEN0:1;
        } B;
    } NMI_EN[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t EOUTEN31:1;
            vuint32_t EOUTEN30:1;
            vuint32_t EOUTEN29:1;
            vuint32_t EOUTEN28:1;
            vuint32_t EOUTEN27:1;
            vuint32_t EOUTEN26:1;
            vuint32_t EOUTEN25:1;
            vuint32_t EOUTEN24:1;
            vuint32_t EOUTEN23:1;
            vuint32_t EOUTEN22:1;
            vuint32_t EOUTEN21:1;
            vuint32_t EOUTEN20:1;
            vuint32_t EOUTEN19:1;
            vuint32_t EOUTEN18:1;
            vuint32_t EOUTEN17:1;
            vuint32_t EOUTEN16:1;
            vuint32_t EOUTEN15:1;
            vuint32_t EOUTEN14:1;
            vuint32_t EOUTEN13:1;
            vuint32_t EOUTEN12:1;
            vuint32_t EOUTEN11:1;
            vuint32_t EOUTEN10:1;
            vuint32_t EOUTEN9:1;
            vuint32_t EOUTEN8:1;
            vuint32_t EOUTEN7:1;
            vuint32_t EOUTEN6:1;
            vuint32_t EOUTEN5:1;
            vuint32_t EOUTEN4:1;
            vuint32_t EOUTEN3:1;
            vuint32_t EOUTEN2:1;
            vuint32_t EOUTEN1:1;
            vuint32_t EOUTEN0:1;
        } B;
    } EOUT_SIG_EN[4];
};
/**************************************************************************/
/*                              Module: FEC                               */
/**************************************************************************/
struct FEC_tag {
    uint8_t FEC_reserved1[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t HBERR:1;
            vuint32_t BABR:1;
            vuint32_t BABT:1;
            vuint32_t GRA:1;
            vuint32_t TXF:1;
            vuint32_t TXB:1;
            vuint32_t RXF:1;
            vuint32_t RXB:1;
            vuint32_t MII:1;
            vuint32_t EBERR:1;
            vuint32_t LC:1;
            vuint32_t RL:1;
            vuint32_t UN:1;
            vuint32_t:19;
        } B;
    } EIR;

    union {
        vuint32_t R;
        struct {
            vuint32_t HBERR:1;
            vuint32_t BABR:1;
            vuint32_t BABT:1;
            vuint32_t GRA:1;
            vuint32_t TXF:1;
            vuint32_t TXB:1;
            vuint32_t RXF:1;
            vuint32_t RXB:1;
            vuint32_t MII:1;
            vuint32_t EBERR:1;
            vuint32_t LC:1;
            vuint32_t RL:1;
            vuint32_t UN:1;
            vuint32_t:19;
        } B;
    } EIMR;

    uint8_t FEC_reserved2[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:7;
            vuint32_t RDAR:1;
            vuint32_t:24;
        } B;
    } RDAR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:7;
            vuint32_t TDAR:1;
            vuint32_t:24;
        } B;
    } TDAR;

    uint8_t FEC_reserved3[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t:30;
            vuint32_t ETHER_EN:1;
            vuint32_t RESET:1;
        } B;
    } ECR;

    uint8_t FEC_reserved4[24];

    union {
        vuint32_t R;
        struct {
            vuint32_t ST:2;
            vuint32_t OP:2;
            vuint32_t PA:5;
            vuint32_t RA:5;
            vuint32_t TA:2;
            vuint32_t DATA:16;
        } B;
    } MMFR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t DIS_PRE:1;
            vuint32_t MII_SPEED:6;
            vuint32_t:1;
        } B;
    } MSCR;

    uint8_t FEC_reserved5[28];

    union {
        vuint32_t R;
        struct {
            vuint32_t MIB_DIS:1;
            vuint32_t MIB_IDLE:1;
            vuint32_t:30;
        } B;
    } MIBC;

    uint8_t FEC_reserved6[28];

    union {
        vuint32_t R;
        struct {
            vuint32_t:5;
            vuint32_t MAX_FL:11;
            vuint32_t:10;
            vuint32_t FCE:1;
            vuint32_t BC_REJ:1;
            vuint32_t PROM:1;
            vuint32_t MII_MODE:1;
            vuint32_t DRT:1;
            vuint32_t LOOP:1;
        } B;
    } RCR;

    uint8_t FEC_reserved7[60];

    union {
        vuint32_t R;
        struct {
            vuint32_t:27;
            vuint32_t RFC_PAUSE:1;
            vuint32_t TFC_PAUSE:1;
            vuint32_t FDEN:1;
            vuint32_t HBC:1;
            vuint32_t GTS:1;
        } B;
    } TCR;

    uint8_t FEC_reserved8[28];

    union {
        vuint32_t R;
        struct {
            vuint32_t PADDR1:32;
        } B;
    } PALR;

    union {
        vuint32_t R;
        struct {
            vuint32_t PADDR2:16;
            vuint32_t TYPE:16;
        } B;
    } PAUR;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPCODE:16;
            vuint32_t PAUSE_DUR:16;
        } B;
    } OPD;

    uint8_t FEC_reserved9[40];

    union {
        vuint32_t R;
        struct {
            vuint32_t IADDR1:32;
        } B;
    } IAUR;

    union {
        vuint32_t R;
        struct {
            vuint32_t IADDR2:32;
        } B;
    } IALR;

    union {
        vuint32_t R;
        struct {
            vuint32_t GADDR1:32;
        } B;
    } GAUR;

    union {
        vuint32_t R;
        struct {
            vuint32_t GADDR2:32;
        } B;
    } GALR;

    uint8_t FEC_reserved10[28];

    union {
        vuint32_t R;
        struct {
            vuint32_t:30;
            vuint32_t TFWR:2;
        } B;
    } TFWR;

    uint8_t FEC_reserved11[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:22;
            vuint32_t R_BOUND:8;
            vuint32_t:2;
        } B;
    } FRBR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:22;
            vuint32_t R_FSTART:8;
            vuint32_t:2;
        } B;
    } FRSR;

    uint8_t FEC_reserved12[44];

    union {
        vuint32_t R;
        struct {
            vuint32_t R_DES_START:30;
            vuint32_t:2;
        } B;
    } ERDSR;

    union {
        vuint32_t R;
        struct {
            vuint32_t X_DES_START:30;
            vuint32_t:2;
        } B;
    } ETDSR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:21;
            vuint32_t R_BUF_SIZE:7;
            vuint32_t:4;
        } B;
    } EMRBR;

    uint8_t FEC_reserved13[116];

    union {
        vuint32_t R;
    } RMON_T_DROP;              /* Count of frames not counted correctly */

    union {
        vuint32_t R;
    } RMON_T_PACKETS;           /* RMON Tx packet count */

    union {
        vuint32_t R;
    } RMON_T_BC_PKT;            /* RMON Tx Broadcast Packets */

    union {
        vuint32_t R;
    } RMON_T_MC_PKT;            /* RMON Tx Multicast Packets */

    union {
        vuint32_t R;
    } RMON_T_CRC_ALIGN;         /* RMON Tx Packets w CRC/Align error */

    union {
        vuint32_t R;
    } RMON_T_UNDERSIZE;         /* RMON Tx Packets < 64 bytes, good crc */

    union {
        vuint32_t R;
    } RMON_T_OVERSIZE;          /* RMON Tx Packets > MAX_FL bytes, good crc */

    union {
        vuint32_t R;
    } RMON_T_FRAG;              /* RMON Tx Packets < 64 bytes, bad crc */

    union {
        vuint32_t R;
    } RMON_T_JAB;               /* RMON Tx Packets > MAX_FL bytes, bad crc */

    union {
        vuint32_t R;
    } RMON_T_COL;               /* RMON Tx collision count */

    union {
        vuint32_t R;
    } RMON_T_P64;               /* RMON Tx 64 byte packets */

    union {
        vuint32_t R;
    } RMON_T_P65TO127;          /* RMON Tx 65 to 127 byte packets */

    union {
        vuint32_t R;
    } RMON_T_P128TO255;         /* RMON Tx 128 to 255 byte packets */

    union {
        vuint32_t R;
    } RMON_T_P256TO511;         /* RMON Tx 256 to 511 byte packets */

    union {
        vuint32_t R;
    } RMON_T_P512TO1023;        /* RMON Tx 512 to 1023 byte packets */

    union {
        vuint32_t R;
    } RMON_T_P1024TO2047;       /* RMON Tx 1024 to 2047 byte packets */

    union {
        vuint32_t R;
    } RMON_T_P_GTE2048;         /* RMON Tx packets w > 2048 bytes */

    union {
        vuint32_t R;
    } RMON_T_OCTETS;            /* RMON Tx Octets */

    union {
        vuint32_t R;
    } IEEE_T_DROP;              /* Count of frames not counted correctly */

    union {
        vuint32_t R;
    } IEEE_T_FRAME_OK;          /* Frames Transmitted OK */

    union {
        vuint32_t R;
    } IEEE_T_1COL;              /* Frames Transmitted with Single Collision */

    union {
        vuint32_t R;
    } IEEE_T_MCOL;              /* Frames Transmitted with Multiple Collisions */

    union {
        vuint32_t R;
    } IEEE_T_DEF;               /* Frames Transmitted after Deferral Delay */

    union {
        vuint32_t R;
    } IEEE_T_LCOL;              /* Frames Transmitted with Late Collision */

    union {
        vuint32_t R;
    } IEEE_T_EXCOL;             /* Frames Transmitted with Excessive Collisions */

    union {
        vuint32_t R;
    } IEEE_T_MACERR;            /* Frames Transmitted with Tx FIFO Underrun */

    union {
        vuint32_t R;
    } IEEE_T_CSERR;             /* Frames Transmitted with Carrier Sense Error */

    union {
        vuint32_t R;
    } IEEE_T_SQE;               /* Frames Transmitted with SQE Error */

    union {
        vuint32_t R;
    } IEEE_T_FDXFC;             /* Flow Control Pause frames transmitted */

    union {
        vuint32_t R;
    } IEEE_T_OCTETS_OK;         /* Octet count for Frames Transmitted w/o Error */

    uint8_t FEC_reserved14[8];

    union {
        vuint32_t R;
    } RMON_R_DROP;              /*  Count of frames not counted correctly  */

    union {
        vuint32_t R;
    } RMON_R_PACKETS;           /* RMON Rx packet count */

    union {
        vuint32_t R;
    } RMON_R_BC_PKT;            /* RMON Rx Broadcast Packets */

    union {
        vuint32_t R;
    } RMON_R_MC_PKT;            /* RMON Rx Multicast Packets */

    union {
        vuint32_t R;
    } RMON_R_CRC_ALIGN;         /* RMON Rx Packets w CRC/Align error */

    union {
        vuint32_t R;
    } RMON_R_UNDERSIZE;         /* RMON Rx Packets < 64 bytes, good crc */

    union {
        vuint32_t R;
    } RMON_R_OVERSIZE;          /* RMON Rx Packets > MAX_FL bytes, good crc */

    union {
        vuint32_t R;
    } RMON_R_FRAG;              /* RMON Rx Packets < 64 bytes, bad crc */

    union {
        vuint32_t R;
    } RMON_R_JAB;               /* RMON Rx Packets > MAX_FL bytes, bad crc */

    uint8_t FEC_reserved15[4];

    union {
        vuint32_t R;
    } RMON_R_P64;               /* RMON Rx 64 byte packets */

    union {
        vuint32_t R;
    } RMON_R_P65TO127;          /* RMON Rx 65 to 127 byte packets */

    union {
        vuint32_t R;
    } RMON_R_P128TO255;         /* RMON Rx 128 to 255 byte packets */

    union {
        vuint32_t R;
    } RMON_R_P256TO511;         /* RMON Rx 256 to 511 byte packets */

    union {
        vuint32_t R;
    } RMON_R_P512TO1023;        /* RMON Rx 512 to 1023 byte packets */

    union {
        vuint32_t R;
    } RMON_R_P1024TO2047;       /* RMON Rx 1024 to 2047 byte packets */

    union {
        vuint32_t R;
    } RMON_R_P_GTE2048;         /* RMON Rx packets w > 2048 bytes */

    union {
        vuint32_t R;
    } RMON_R_OCTETS;            /* RMON Rx Octets */

    union {
        vuint32_t R;
    } IEEE_R_DROP;              /* Count of frames not counted correctly */

    union {
        vuint32_t R;
    } IEEE_R_FRAME_OK;          /* Frames Received OK */

    union {
        vuint32_t R;
    } IEEE_R_CRC;               /* Frames Received with CRC Error */

    union {
        vuint32_t R;
    } IEEE_R_ALIGN;             /* Frames Received with Alignment Error */

    union {
        vuint32_t R;
    } IEEE_R_MACERR;            /* Receive Fifo Overflow count */

    union {
        vuint32_t R;
    } IEEE_R_FDXFC;             /* Flow Control Pause frames received */

    union {
        vuint32_t R;
    } IEEE_R_OCTETS_OK;         /* Octet count for Frames Rcvd w/o Error */
};
/**************************************************************************/
/*                   Module: FLASH                                        */
/**************************************************************************/
struct FLASH_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t RRE:1;
            vuint32_t AEE:1;
            vuint32_t EEE:1;
            vuint32_t:12;
            vuint32_t EER:1;
            vuint32_t RWE:1;
            vuint32_t SBC:1;
            vuint32_t:1;
            vuint32_t PEAS:1;
            vuint32_t DONE:1;
            vuint32_t PEG:1;
            vuint32_t PECIE:1;
            vuint32_t FERS:1;
            vuint32_t:2;
            vuint32_t PGM:1;
            vuint32_t PSUS:1;
            vuint32_t ERS:1;
            vuint32_t ESUS:1;
            vuint32_t EHV:1;
        } B;
    } MCR;

    uint8_t FLASH_reserved1[4];
 
    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t n8Kh:2;
            vuint32_t n128K:5;
            vuint32_t n64Kh:3;
            vuint32_t n32Kh:2;
            vuint32_t n16Kh:3;
            vuint32_t n64Km:3;
            vuint32_t n32Km:2;
            vuint32_t n16Km:3;
            vuint32_t n64Kl:3;
            vuint32_t n32Kl:2;
            vuint32_t n16Kl:3;
        } B;
    } MCRE;

    uint8_t FLASH_reserved2[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t TSLOCK:1;
            vuint32_t:1;
            vuint32_t LLLK:14;
            vuint32_t MLLK:16;
        } B;
    } LOCK0;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t HLK:16;
        } B;
    } LOCK1;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t n128LK:31;     
        } B;
    } LOCK2;


    uint8_t FLASH_reserved3[28];


    union {
        vuint32_t R;
        struct {
            vuint32_t:2;
            vuint32_t LSL:14;
            vuint32_t MSEL:16;
        } B;
    } SEL0;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t HSEL:16;
        } B;
    } SEL1;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t n128KSEL:31;
        } B;
    } SEL2;

    uint8_t FLASH_reserved4[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t SAD:1;
            vuint32_t:7 ;
            vuint32_t ADD:21;
            vuint32_t:3 ;
        } B;
    } ADR;

    union {
        vuint32_t R;
        struct {
            vuint32_t UTE:1;
            vuint32_t SBCE:1;
            vuint32_t:11;
            vuint32_t CPR:1;
            vuint32_t CPA:1;
            vuint32_t CPE:1;
            vuint32_t:6;
            vuint32_t NAIBP:1;
            vuint32_t AIBPE:1;
            vuint32_t:1;
            vuint32_t AISUS:1;
            vuint32_t MRE:1;
            vuint32_t MRV:1;
            vuint32_t:1;
            vuint32_t AIS:1;
            vuint32_t AIE:1;
            vuint32_t AID:1;
        } B;
    } UT0;

    union {
        vuint32_t R;
        struct {
            vuint32_t MISR:32;
        } B;
    } UM0;

    union {
        vuint32_t R;
        struct {
            vuint32_t MISR:32;
        } B;
    } UM1;

    union {
        vuint32_t R;
        struct {
            vuint32_t MISR:32;
        } B;
    } UM2;

    union {
        vuint32_t R;
        struct {
            vuint32_t MISR:32;
        } B;
    } UM3;

    uint8_t FLASH_reserved5[16];

    union {
        vuint32_t R;
        struct {
            vuint32_t MISR:32;
        } B;
    } UM8;

    union {
        vuint32_t R;
        struct {
            vuint32_t:31;
            vuint32_t MISR:1;
        } B;
    } UM9;

    union {
        vuint32_t R;
        struct {
            vuint32_t:2;
            vuint32_t LOWOPP:14;
            vuint32_t MIDOPP:16;
        } B;
    } OPP0;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t HIGHOPP:16;
        } B;
    } OPP1;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;    
            vuint32_t n128KOPP:31;
        } B;
    } OPP2;

    uint8_t FLASH_reserved6[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t TDMPC:32;
        } B;
    } TDMPC;
};
/**************************************************************************/
/*                   Module: FR                                           */
/**************************************************************************/
struct FR_tag {
    union {
        vuint16_t R;
        struct {
            vuint16_t CHIVER:8;
            vuint16_t PEVER:8;
        } B;
    } MVR;

    union {
        vuint16_t R;
        struct {
            vuint16_t MEN:1;
            vuint16_t SBFF:1;
            vuint16_t SCM:1;
            vuint16_t CHB:1;
            vuint16_t CHA:1;
            vuint16_t SFFE:1;
            vuint16_t ECCE:1;
            vuint16_t:1;
            vuint16_t FUM:1;
            vuint16_t FAM:1;
            vuint16_t:1;
            vuint16_t CLKSEL:1;
            vuint16_t BITRATE:3;
            vuint16_t:1;
        } B;
    } MCR;

    union {
        vuint16_t R;
        struct {
            vuint16_t SMBA:16;
        } B;
    } SYMBADHR;

    union {
        vuint16_t R;
        struct {
            vuint16_t SMBA:12;
            vuint16_t:4;
        } B;
    } SYMBADLR;

    union {
        vuint16_t R;
        struct {
            vuint16_t WMD:1;
            vuint16_t:3;
            vuint16_t SEL:4;
            vuint16_t:3;
            vuint16_t ENB:1;
            vuint16_t:2;
            vuint16_t STBPSEL:2;
        } B;
    } STBSCR;

    uint8_t FR_reserved1[2];

    union {
        vuint16_t R;
        struct {
            vuint16_t:1;
            vuint16_t MBSEG2DS:7;
            vuint16_t:1;
            vuint16_t MBSEG1DS:7;
        } B;
    } MBDSR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:1;
            vuint16_t LAST_MB_SEG1:7;
            vuint16_t:1;
            vuint16_t LAST_MB_UTIL:7;
        } B;
    } MBSSUTR;

    union {
        vuint16_t R;
        struct {
            vuint16_t INST:4;
            vuint16_t ADDR:11;
            vuint16_t DAD:1;
        } B;
    } PEDRAR;

    union {
        vuint16_t R;
        struct {
            vuint16_t DATA:16;
        } B;
    } PEDRDR;

    union {
        vuint16_t R;
        struct {
            vuint16_t WME:1;
            vuint16_t:3;
            vuint16_t EOC_AP:2;
            vuint16_t ERC_AP:2;
            vuint16_t BSY_WMC:1;
            vuint16_t:3;
            vuint16_t POCCMD:4;
        } B;
    } POCR;

    union {
        vuint16_t R;
        struct {
            vuint16_t MIF:1;
            vuint16_t PRIF:1;
            vuint16_t CHIF:1;
            vuint16_t WUPIF:1;
            vuint16_t FAFBIF:1;
            vuint16_t FAFAIF:1;
            vuint16_t RBIF:1;
            vuint16_t TBIF:1;
            vuint16_t MIE:1;
            vuint16_t PRIE:1;
            vuint16_t CHIE:1;
            vuint16_t WUPIE:1;
            vuint16_t FAFBIE:1;
            vuint16_t FAFAIE:1;
            vuint16_t RBIE:1;
            vuint16_t TBIE:1;
        } B;
    } GIFER;

    union {
        vuint16_t R;
        struct {
            vuint16_t FATL_IF:1;
            vuint16_t INTL_IF:1;
            vuint16_t ILCF_IF:1;
            vuint16_t CSA_IF:1;
            vuint16_t MRC_IF:1;
            vuint16_t MOC_IF:1;
            vuint16_t CCL_IF:1;
            vuint16_t MXS_IF:1;
            vuint16_t MTX_IF:1;
            vuint16_t LTXB_IF:1;
            vuint16_t LTXA_IF:1;
            vuint16_t TBVB_IF:1;
            vuint16_t TBVA_IF:1;
            vuint16_t TI2_IF:1;
            vuint16_t TI1_IF:1;
            vuint16_t CYS_IF:1;
        } B;
    } PIFR0;

    union {
        vuint16_t R;
        struct {
            vuint16_t EMC_IF:1;
            vuint16_t IPC_IF:1;
            vuint16_t PECF_IF:1;
            vuint16_t PSC_IF:1;
            vuint16_t SSI3_IF:1;
            vuint16_t SSI2_IF:1;
            vuint16_t SSI1_IF:1;
            vuint16_t SSI0_IF:1;
            vuint16_t:2;
            vuint16_t EVT_IF:1;
            vuint16_t ODT_IF:1;
            vuint16_t:4;
        } B;
    } PIFR1;

    union {
        vuint16_t R;
        struct {
            vuint16_t FATL_IE:1;
            vuint16_t INTL_IE:1;
            vuint16_t ILCF_IE:1;
            vuint16_t CSA_IE:1;
            vuint16_t MRC_IE:1;
            vuint16_t MOC_IE:1;
            vuint16_t CCL_IE:1;
            vuint16_t MXS_IE:1;
            vuint16_t MTX_IE:1;
            vuint16_t LTXB_IE:1;
            vuint16_t LTXA_IE:1;
            vuint16_t TBVB_IE:1;
            vuint16_t TBVA_IE:1;
            vuint16_t TI2_IE:1;
            vuint16_t TI1_IE:1;
            vuint16_t CYS_IE:1;
        } B;
    } PIER0;

    union {
        vuint16_t R;
        struct {
            vuint16_t EMC_IE:1;
            vuint16_t IPC_IE:1;
            vuint16_t PECF_IE:1;
            vuint16_t PSC_IE:1;
            vuint16_t SSI3_IE:1;
            vuint16_t SSI2_IE:1;
            vuint16_t SSI1_IE:1;
            vuint16_t SSI0_IE:1;
            vuint16_t:2;
            vuint16_t EVT_IE:1;
            vuint16_t ODT_IE:1;
           vuint16_t:4;
        } B;
    } PIER1;

    union {
        vuint16_t R;
        struct {
            vuint16_t FRLB_EF:1;
            vuint16_t FRLA_EF:1;
            vuint16_t PCMI_EF:1;
            vuint16_t FOVB_EF:1;
            vuint16_t FOVA_EF:1;
            vuint16_t MBS_EF:1;
            vuint16_t MBU_EF:1;
            vuint16_t LCK_EF:1;
            vuint16_t:1;
            vuint16_t SBCF_EF:1;
            vuint16_t FID_EF:1;
            vuint16_t DPL_EF:1;
            vuint16_t SPL_EF:1;
            vuint16_t NML_EF:1;
            vuint16_t NMF_EF:1;
            vuint16_t ILSA_EF:1;
        } B;
    } CHIERFR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:1;
            vuint16_t TBIVEC:7;
            vuint16_t:1;
            vuint16_t RBIVEC:7;
        } B;
    } MBIVEC;

    union {
        vuint16_t R;
        struct {
            vuint16_t STATUS_ERR_CNT:16;
        } B;
    } CASERCR;

    union {
        vuint16_t R;
        struct {
            vuint16_t STATUS_ERR_CNT:16;
        } B;
    } CBSERCR;

    union {
        vuint16_t R;
        struct {
            vuint16_t ERRMODE:2;
            vuint16_t SLOTMODE:2;
            vuint16_t:1;
            vuint16_t PROTSTATE:3;
            vuint16_t STARTUPSTATE:4;
            vuint16_t:1;
            vuint16_t WAKEUPSTATUS:3;
        } B;
    } PSR0;

    union {
        vuint16_t R;
        struct {
            vuint16_t CSAA:1;
            vuint16_t CSP:1;
            vuint16_t:1;
            vuint16_t REMCSAT:5;
            vuint16_t CPN:1;
            vuint16_t HHR:1;
            vuint16_t FRZ:1;
            vuint16_t APTAC:5;
        } B;
    } PSR1;

    union {
        vuint16_t R;
        struct {
            vuint16_t NBVB:1;
            vuint16_t NSEB:1;
            vuint16_t STCB:1;
            vuint16_t SBVB:1;
            vuint16_t SSEB:1;
            vuint16_t MTB:1;
            vuint16_t NBVA:1;
            vuint16_t NSEA:1;
            vuint16_t STCA:1;
            vuint16_t SBVA:1;
            vuint16_t SSEA:1;
            vuint16_t MTA:1;
            vuint16_t CLKCORRFAILCNT:4;
        } B;
    } PSR2;

    union {
        vuint16_t R;
        struct {
            vuint16_t:2;
            vuint16_t WUB:1;
            vuint16_t ABVB:1;
            vuint16_t AACB:1;
            vuint16_t ACEB:1;
            vuint16_t ASEB:1;
            vuint16_t AVFB:1;
            vuint16_t:2;
            vuint16_t WUA:1;
            vuint16_t ABVA:1;
            vuint16_t AACA:1;
            vuint16_t ACEA:1;
            vuint16_t ASEA:1;
            vuint16_t AVFA:1;
        } B;
    } PSR3;

    union {
        vuint16_t R;
        struct {
            vuint16_t:2;
            vuint16_t MTCT:14;
        } B;
    } MTCTR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:10;
            vuint16_t CYCCNT:6;
        } B;
    } CYCTR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:5;
            vuint16_t SLOTCNTA:11;
        } B;
    } SLTCTAR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:5;
            vuint16_t SLOTCNTB:11;
        } B;
    } SLTCTBR;

    union {
        vuint16_t R;
        struct {
            vuint16_t RATECORR:16;
        } B;
    } RTCORVR;

    union {
        vuint16_t R;
        struct {
            vuint16_t OFFSETCORR:16;
        } B;
    } OFCORVR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:8;
            vuint16_t MIF:1;
            vuint16_t PRIF:1;
            vuint16_t CHIF:1;
            vuint16_t WUPIF:1;
            vuint16_t FAFBIF:1;
            vuint16_t FAFAIF:1;
            vuint16_t RBIF:1;
            vuint16_t TBIF:1;
        } B;
    } CIFR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:8;
            vuint16_t TIMEOUT:8;
        } B;
    } SYMATOR;

    union {
        vuint16_t R;
        struct {
            vuint16_t SFEVB:4;
            vuint16_t SFEVA:4;
            vuint16_t SFODB:4;
            vuint16_t SFODA:4;
        } B;
    } SFCNTR;

    union {
        vuint16_t R;
        struct {
            vuint16_t SFT_OFFSET:15;
            vuint16_t:1;
        } B;
    } SFTOR;

    union {
        vuint16_t R;
        struct {
            vuint16_t ELKT:1;
            vuint16_t OLKT:1;
            vuint16_t CYCNUM:6;
            vuint16_t ELKS:1;
            vuint16_t OLKS:1;
            vuint16_t EVAL:1;
            vuint16_t OVAL:1;
            vuint16_t:1;
            vuint16_t OPT:1;
            vuint16_t SDVEN:1;
            vuint16_t SIDEN:1;
        } B;
    } SFTCCSR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:6;
            vuint16_t SYNFRID:10;
        } B;
    } SFIDRFR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:6;
            vuint16_t FVAL:10;
        } B;
    } SFIDAFVR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:6;
            vuint16_t FMSK:10;
        } B;
    } SFIDAFMR;

    union {
        vuint16_t R;
        struct {
            vuint16_t NMVP:16;
        } B;
    } NMVR[6];

    union {
        vuint16_t R;
        struct {
            vuint16_t:12;
            vuint16_t NMVL:4;
        } B;
    } NMVLR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:2;
            vuint16_t T2_CFG:1;
            vuint16_t T2_REP:1;
            vuint16_t:1;
            vuint16_t T2SP:1;
            vuint16_t T2TR:1;
            vuint16_t T2ST:1;
            vuint16_t:3;
            vuint16_t T1_REP:1;
            vuint16_t:1;
            vuint16_t T1SP:1;
            vuint16_t T1TR:1;
            vuint16_t T1ST:1;
        } B;
    } TICCR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:2;
            vuint16_t T1_CYC_VAL:6;
            vuint16_t:2;
            vuint16_t T1_CYC_MSK:6;
        } B;
    } TI1CYSR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:2;
            vuint16_t T1_MTOFFSET:14;
        } B;
    } TI1MTOR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:2;        /* for rabsolute timer selected */
            vuint16_t T2_CYC_VAL:6;     /* for rabsolute timer selected */
            vuint16_t:2;      /* for rabsolute timer selected */
            vuint16_t T2_CYC_MSK:6;     /* for rabsolute timer selected */
            // vuint16_t T2_MTCNT:16;   /* for relative timer selected */
        } B;
    } TI2CR0;

    union {
        vuint16_t R;
        struct {
            vuint16_t:2;        /* for rabsolute timer selected */
            vuint16_t T2_MTOFFSET:14;   /* for rabsolute timer selected */
            // vuint16_t T2_MTCNT:16;    /* for relative timer selected */
        } B;
    } TI2CR1;

    union {
        vuint16_t R;
        struct {
            vuint16_t WMD:1;
            vuint16_t:1;
            vuint16_t SEL:2;
            vuint16_t:1;
            vuint16_t SLOTNUMBER:11;
        } B;
    } SSSR;

    union {
        vuint16_t R;
        struct {
            vuint16_t WMD:1;
            vuint16_t:1;
            vuint16_t SEL:2;
            vuint16_t:1;
            vuint16_t CNTCFG:2;
            vuint16_t MCY:1;
            vuint16_t VFR:1;
            vuint16_t SYF:1;
            vuint16_t NUF:1;
            vuint16_t SUF:1;
            vuint16_t STATUSMASK:4;
        } B;
    } SSCCR;

    union {
        vuint16_t R;
        struct {
            vuint16_t VFB:1;
            vuint16_t SYB:1;
            vuint16_t NFB:1;
            vuint16_t SUB:1;
            vuint16_t SEB:1;
            vuint16_t CEB:1;
            vuint16_t BVB:1;
            vuint16_t TCB:1;
            vuint16_t VFA:1;
            vuint16_t SYA:1;
            vuint16_t NFA:1;
            vuint16_t SUA:1;
            vuint16_t SEA:1;
            vuint16_t CEA:1;
            vuint16_t BVA:1;
            vuint16_t TCA:1;
        } B;
    } SSR[8];

    union {
        vuint16_t R;
        struct {
            vuint16_t SLOTSTATUSCNT:16;
        } B;
    } SSCR[4];

    union {
        vuint16_t R;
        struct {
            vuint16_t MTE:1;
            vuint16_t:1;
            vuint16_t CYCCNTMSK:6;
            vuint16_t:2;
            vuint16_t CYCCNTVAL:6;
        } B;
    } MTSACFR;

    union {
        vuint16_t R;
        struct {
            vuint16_t MTE:1;
            vuint16_t:1;
            vuint16_t CYCCNTMSK:6;
            vuint16_t:2;
            vuint16_t CYCCNTVAL:6;
        } B;
    } MTSBCFR;

    union {
        vuint16_t R;
        struct {
            vuint16_t WMD:1;
            vuint16_t:1;
            vuint16_t SEL:2;
            vuint16_t:4;
            vuint16_t RSBIDX:8;
        } B;
    } RSBIR;

    union {
        vuint16_t R;
        struct {
            vuint16_t WM:8;
            vuint16_t:7;
            vuint16_t SEL:1;
        } B;
    } RFWMSR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:6;
            vuint16_t SIDX:10;
        } B;
    } RFSIR;

    union {
        vuint16_t R;
        struct {
            vuint16_t FIFO_DEPTH:8;
            vuint16_t:1;
            vuint16_t ENTRY_SIZE:7;
        } B;
    } RFDSR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:6;
            vuint16_t RDIDX:10;
        } B;
    } RFARIR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:6;
            vuint16_t RDIDX:10;
        } B;
    } RFBRIR;

    union {
        vuint16_t R;
        struct {
            vuint16_t MIDAFVAL:16;
        } B;
    } RFMIDAFVR;

    union {
        vuint16_t R;
        struct {
            vuint16_t MIDAFMSK:16;
        } B;
    } RFMIDAFMR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:5;
            vuint16_t FIDRFVAL:11;
        } B;
    } RFFIDRFVR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:5;
            vuint16_t FIDRFMSK:11;
        } B;
    } RFFIDRFMR;

    union {
        vuint16_t R;
        struct {
            vuint16_t WMD:1;
            vuint16_t IBD:1;
            vuint16_t SEL:2;
            vuint16_t:1;
            vuint16_t SID:11;
        } B;
    } RFRFCFR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:4;
            vuint16_t F3MD:1;
            vuint16_t F2MD:1;
            vuint16_t F1MD:1;
            vuint16_t F0MD:1;
            vuint16_t:4;
            vuint16_t F3EN:1;
            vuint16_t F2EN:1;
            vuint16_t F1EN:1;
            vuint16_t F0EN:1;
        } B;
    } RFRFCTR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:5;
            vuint16_t LASTDYNTXSLOTA:11;
        } B;
    } LDTXSLAR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:5;
            vuint16_t LASTDYNTXSLOTB:11;
        } B;
    } LDTXSLBR;

    union {
        vuint16_t R;
        struct {
            vuint16_t action_point_offset:6;
            vuint16_t static_slot_length:10;
        } B;
    } PCR0;

    union {
        vuint16_t R;
        struct {
            vuint16_t:2;
            vuint16_t macro_after_first_static_slot:14;
        } B;
    } PCR1;

    union {
        vuint16_t R;
        struct {
            vuint16_t minislot_after_action_point:6;
            vuint16_t number_of_static_slots:10;
        } B;
    } PCR2;

    union {
        vuint16_t R;
        struct {
            vuint16_t wakeup_symbol_rx_low:6;
            vuint16_t minislot_action_point_offset:5;
            vuint16_t coldstart_attempts:5;
        } B;
    } PCR3;

    union {
        vuint16_t R;
        struct {
            vuint16_t cas_rx_low_max:7;
            vuint16_t wakeup_symbol_rx_window:9;
        } B;
    } PCR4;

    union {
        vuint16_t R;
        struct {
            vuint16_t tss_transmitter:4;
            vuint16_t wakeup_symbol_tx_low:6;
            vuint16_t wakeup_symbol_rx_idle:6;
        } B;
    } PCR5;

    union {
        vuint16_t R;
        struct {
            vuint16_t:1;
            vuint16_t symbol_window_after_action_point:8;
            vuint16_t macro_initial_offset_a:7;
        } B;
    } PCR6;

    union {
        vuint16_t R;
        struct {
            vuint16_t decoding_correction_b:9;
            vuint16_t micro_per_macro_nom_half:7;
        } B;
    } PCR7;

    union {
        vuint16_t R;
        struct {
            vuint16_t max_without_clock_correction_fatal:4;
            vuint16_t max_without_clock_correction_passive:4;
            vuint16_t wakeup_symbol_tx_idle:8;
        } B;
    } PCR8;

    union {
        vuint16_t R;
        struct {
            vuint16_t minislot_exists:1;
            vuint16_t symbol_window_exists:1;
            vuint16_t offset_correction_out:14;
        } B;
    } PCR9;

    union {
        vuint16_t R;
        struct {
            vuint16_t single_slot_enabled:1;
            vuint16_t wakeup_channel:1;
            vuint16_t macro_per_cycle:14;
        } B;
    } PCR10;

    union {
        vuint16_t R;
        struct {
            vuint16_t key_slot_used_for_startup:1;
            vuint16_t key_slot_used_for_sync:1;
            vuint16_t offset_correction_start:14;
        } B;
    } PCR11;

    union {
        vuint16_t R;
        struct {
            vuint16_t allow_passive_to_active:5;
            vuint16_t key_slot_header_crc:11;
        } B;
    } PCR12;

    union {
        vuint16_t R;
        struct {
            vuint16_t first_minislot_action_point_offset:6;
            vuint16_t static_slot_after_action_point:10;
        } B;
    } PCR13;

    union {
        vuint16_t R;
        struct {
            vuint16_t rate_correction_out:11;
            vuint16_t listen_timeout:5;
        } B;
    } PCR14;

    union {
        vuint16_t R;
        struct {
            vuint16_t listen_timeout:16;
        } B;
    } PCR15;

    union {
        vuint16_t R;
        struct {
            vuint16_t macro_initial_offset_b:7;
            vuint16_t noise_listen_timeout:9;
        } B;
    } PCR16;

    union {
        vuint16_t R;
        struct {
            vuint16_t noise_listen_timeout:16;
        } B;
    } PCR17;

    union {
        vuint16_t R;
        struct {
            vuint16_t wakeup_pattern:6;
            vuint16_t key_slot_id:10;
        } B;
    } PCR18;

    union {
        vuint16_t R;
        struct {
            vuint16_t decoding_correction_a:9;
            vuint16_t payload_length_static:7;
        } B;
    } PCR19;

    union {
        vuint16_t R;
        struct {
            vuint16_t micro_initial_offset_b:8;
            vuint16_t micro_initial_offset_a:8;
        } B;
    } PCR20;

    union {
        vuint16_t R;
        struct {
            vuint16_t extern_rate_correction:3;
            vuint16_t latest_tx:13;
        } B;
    } PCR21;

    union {
        vuint16_t R;
        struct {
            vuint16_t:1;
            vuint16_t comp_accepted_startup_range_a:11;
            vuint16_t micro_per_cycle:4;
        } B;
    } PCR22;

    union {
        vuint16_t R;
        struct {
            vuint16_t micro_per_cycle:16;
        } B;
    } PCR23;

    union {
        vuint16_t R;
        struct {
            vuint16_t cluster_drift_damping:5;
            vuint16_t max_payload_length_dynamic:7;
            vuint16_t micro_per_cycle_min:4;
        } B;
    } PCR24;

    union {
        vuint16_t R;
        struct {
            vuint16_t micro_per_cycle_min:16;
        } B;
    } PCR25;

    union {
        vuint16_t R;
        struct {
            vuint16_t allow_halt_due_to_clock:1;
            vuint16_t comp_accepted_startup_range_b:11;
            vuint16_t micro_per_cycle_max:4;
        } B;
    } PCR26;

    union {
        vuint16_t R;
        struct {
            vuint16_t micro_per_cycle_max:16;
        } B;
    } PCR27;

    union {
        vuint16_t R;
        struct {
            vuint16_t dynamic_slot_idle_phase:2;
            vuint16_t macro_after_offset_correction:14;
        } B;
    } PCR28;

    union {
        vuint16_t R;
        struct {
            vuint16_t extern_offset_correction:3;
            vuint16_t minislots_max:13;
        } B;
    } PCR29;

    union {
        vuint16_t R;
        struct {
            vuint16_t:12;
            vuint16_t sync_node_max:4;
        } B;
    } PCR30;

    uint8_t FR_reserved4[4];

    union {
        vuint16_t R;
        struct {
            vuint16_t:13;
            vuint16_t TIM2_EE:1;
            vuint16_t TIM1_EE:1;
            vuint16_t CYC_EE:1;
        } B;
    } PEOER;

    uint8_t FR_reserved5[2];

    union {
        vuint16_t R;
        struct {
            vuint16_t SDO:16;
        } B;
    } RFSDOR;

    union {
        vuint16_t R;
        struct {
            vuint16_t SMBA:16;
        } B;
    } RFSYMBADHR;

    union {
        vuint16_t R;
        struct {
            vuint16_t SMBA:12;
            vuint16_t:4;
        } B;
    } RFSYMBADLR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:2;
            vuint16_t PTD:14;
        } B;
    } RFPTR;

    union {
        vuint16_t R;
        struct {
            vuint16_t FLB_PCB:8;
            vuint16_t FLA_PCA:8;
        } B;
    } RFFLPCR;

    union {
        vuint16_t R;
        struct {
            vuint16_t LRNE_OF:1;
            vuint16_t LRCE_OF:1;
            vuint16_t DRNE_OF:1;
            vuint16_t DRCE_OF:1;
            vuint16_t LRNE_IF:1;
            vuint16_t LRCE_IF:1;
            vuint16_t DRNE_IF:1;
            vuint16_t DRCE_IF:1;
            vuint16_t:4;
            vuint16_t LRNE_IE:1;
            vuint16_t LRCE_IE:1;
            vuint16_t DRNE_IE:1;
            vuint16_t DRCE_IE:1;
        } B;
    } EEIFER;

    union {
        vuint16_t R;
        struct {
            vuint16_t BSY:1;
            vuint16_t:5;
            vuint16_t ERS:2;
            vuint16_t:3;
            vuint16_t ERM:1;
            vuint16_t:2;
            vuint16_t EIM:1;
            vuint16_t EIE:1;
        } B;
    } EERICR;

    union {
        vuint16_t R;
        struct {
            vuint16_t MID:1;
            vuint16_t BANK:3;
            vuint16_t ADDR:12;
        } B;
    } EERAR;

    union {
        vuint16_t R;
        struct {
            vuint16_t DATA:16;
        } B;
    } EERDR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:11;
            vuint16_t CODE:5;
        } B;
    } EERCR;

    union {
        vuint16_t R;
        struct {
            vuint16_t MID:1;
            vuint16_t BANK:3;
            vuint16_t ADDR:12;
        } B;
    } EEIAR;

    union {
        vuint16_t R;
        struct {
            vuint16_t DATA:16;
        } B;
    } EEIDR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:11;
            vuint16_t CODE:5;
        } B;
    } EEICR;

    uint8_t FR_reserved6[1792];

    struct {
        union {
            vuint16_t R;
            struct {
                vuint16_t:3;
                vuint16_t MTD:1;
                vuint16_t CMT:1;
                vuint16_t EDT:1;
                vuint16_t LCKT:1;
                vuint16_t MBIE:1;
                vuint16_t:3;
                vuint16_t DUP:1;
                vuint16_t DVAL:1;
                vuint16_t EDS:1;
                vuint16_t LCKS:1;
                vuint16_t MBIF:1;
            } B;
        } MBCCSR;

        union {
            vuint16_t R;
            struct {
                vuint16_t MTM:1;
                vuint16_t CHA:1;
                vuint16_t CHB:1;
                vuint16_t CCFE:1;
                vuint16_t CCFMSK:6;
                vuint16_t CCFVAL:6;
            } B;
        } MBCCFR;

        union {
            vuint16_t R;
            struct {
                vuint16_t:5;
                vuint16_t FID:11;
            } B;
        } MBFIDR;

        union {
            vuint16_t R;
            struct {
                vuint16_t:8;
                vuint16_t MBIDX:8;
            } B;
        } MBIDXR;
    } Channel[128];

    uint8_t FR_reserved7[1024];

    union {
        vuint16_t R;
        struct {
            vuint16_t MBDO:16;
        } B;
    } MBDOR[132];

    union {
        vuint16_t R;
        struct {
            vuint16_t LEETD:16;
        } B;
    } LEETR[6];
};
/**************************************************************************/
/*                   Module: GTMINT                                       */
/**************************************************************************/
struct GTMINT_tag {
    uint8_t GTMINT_reserved0[48];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t MDIS:1;
            vuint32_t:13;
            vuint32_t AEISREN:1;
            vuint32_t:1;
            vuint32_t STPS:1;
            vuint32_t:14;
        } B;
    } MCR;

    uint8_t GTMINT_reserved1[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:22;
            vuint32_t INTCLR_PTR:10;
        } B;
    } CLR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:31;
            vuint32_t AEISRST:1;
        } B;
    } AEICR;
};
/**************************************************************************/
/*                   Module: I2C                                          */
/**************************************************************************/
struct I2C_tag {
    union {
        vuint8_t R;
        struct {
            vuint8_t ADR:7;
            vuint8_t:1;
        } B;
    } IBAD;

    union {
        vuint8_t R;
        struct {
            vuint8_t IBC:8;
        } B;
    } IBFD;

    union {
        vuint8_t R;
        struct {
            vuint8_t MDIS:1;
            vuint8_t IBIE:1;
            vuint8_t MS_SL:1;
            vuint8_t Tx_Rx:1;
            vuint8_t NOACK:1;
            vuint8_t RSTA:1;
            vuint8_t DMAEN:1;
            vuint8_t IBSDOZE:1;
        } B;
    } IBCR;

    union {
        vuint8_t R;
        struct {
            vuint8_t TCF:1;
            vuint8_t IAAS:1;
            vuint8_t IBB:1;
            vuint8_t IBAL:1;
            vuint8_t:1;
            vuint8_t SRW:1;
            vuint8_t IBIF:1;
            vuint8_t RXAK:1;
        } B;
    } IBSR;

    union {
        vuint8_t R;
        struct {
            vuint8_t DATA:8;
        } B;
    } IBDR;

    union {
        vuint8_t R;
        struct {
            vuint8_t BIIE:1;
            vuint8_t:7;
        } B;
    } IBIC;

    union {
        vuint8_t R;
        struct {
            vuint8_t:6;
            vuint8_t IPG_DEBUG_HALTED:1;
            vuint8_t IPG_DEBUG_EN:1;
        } B;
    } IBDBG;
};
/**************************************************************************/
/*                   Module: IMA                                          */
/**************************************************************************/
struct IMA_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t INC:1;
            vuint32_t:30;
            vuint32_t READ:1;
        } B;
    } CTRL;

    union {
        vuint32_t R;
        struct {
            vuint32_t:31;
            vuint32_t EN:1;
        } B;
    } ENABLE;

    union {
        vuint32_t R;
        struct {
            vuint32_t:2;
            vuint32_t LOCK_TIMER:14;
            vuint32_t:7;
            vuint32_t WRITE_LOCK:1;
            vuint32_t:7;
            vuint32_t READ_LOCK:1;
        } B;
    } STATUS;

    union {
        vuint32_t R;
        struct {
            vuint32_t ADDR:32;
        } B;
    } SLCT;

    union {
        vuint32_t R;
        struct {
            vuint32_t WRITE_KEY:32;
        } B;
    } WRITE_UNLOCK;

    union {
        vuint32_t R;
        struct {
            vuint32_t READ_KEY:32;
        } B;
    } READ_UNLOCK;

    uint8_t IMA_reserved1[20];

    union {
        vuint32_t R;
        struct {
            vuint32_t WRITE_n:32;
        } B;
    } WRITE_DATA_4;

    union {
        vuint32_t R;
        struct {
            vuint32_t WRITE_n:32;
        } B;
    } WRITE_DATA_3;

    union {
        vuint32_t R;
        struct {
            vuint32_t WRITE_n:32;
        } B;
    } WRITE_DATA_2;

    union {
        vuint32_t R;
        struct {
            vuint32_t WRITE_n:32;
        } B;
    } WRITE_DATA_1;

    union {
        vuint32_t R;
        struct {
            vuint32_t WRITE_n:32;
        } B;
    } WRITE_DATA_0;

    uint8_t IMA_reserved2[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t READ_n:32;
        } B;
    } READ_DATA_4;

    union {
        vuint32_t R;
        struct {
            vuint32_t READ_n:32;
        } B;
    } READ_DATA_3;

    union {
        vuint32_t R;
        struct {
            vuint32_t READ_n:32;
        } B;
    } READ_DATA_2;

    union {
        vuint32_t R;
        struct {
            vuint32_t READ_n:32;
        } B;
    } READ_DATA_1;

    union {
        vuint32_t R;
        struct {
            vuint32_t READ_n:32;
        } B;
    } READ_DATA_0;
};
/**************************************************************************/
/*                   Module: INTC                                         */
/**************************************************************************/
struct INTC_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:19;
            vuint32_t HVEN3:1;
            vuint32_t:3;
            vuint32_t HVEN2:1;
            vuint32_t:3;
            vuint32_t HVEN1:1;
            vuint32_t:3;
            vuint32_t HVEN0:1;
        } B;
    } BCR;

    uint8_t INTC_reserved1[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t PRI:6;
        } B;
    } CPR[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t VTBA:20;
            vuint32_t INTVEC:10;
            vuint32_t:2;
        } B;
    } IACKR[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t EOI:32;
        } B;
    } EOIR[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t SETa:1;
            vuint32_t CLRa:1;
            vuint32_t:6;
            vuint32_t SETb:1;
            vuint32_t CLRb:1;
            vuint32_t:6;
            vuint32_t SETc:1;
            vuint32_t CLRc:1;
            vuint32_t:6;
            vuint32_t SETd:1;
            vuint32_t CLRd:1;
        } B;
    } SSCIR0_3;

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t SETa:1;
            vuint32_t CLRa:1;
            vuint32_t:6;
            vuint32_t SETb:1;
            vuint32_t CLRb:1;
            vuint32_t:6;
            vuint32_t SETc:1;
            vuint32_t CLRc:1;
            vuint32_t:6;
            vuint32_t SETd:1;
            vuint32_t CLRd:1;
        } B;
    } SSCIR4_7;

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t SETa:1;
            vuint32_t CLRa:1;
            vuint32_t:6;
            vuint32_t SETb:1;
            vuint32_t CLRb:1;
            vuint32_t:6;
            vuint32_t SETc:1;
            vuint32_t CLRc:1;
            vuint32_t:6;
            vuint32_t SETd:1;
            vuint32_t CLRd:1;
        } B;
    } SSCIR8_11;

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t SETa:1;
            vuint32_t CLRa:1;
            vuint32_t:6;
            vuint32_t SETb:1;
            vuint32_t CLRb:1;
            vuint32_t:6;
            vuint32_t SETc:1;
            vuint32_t CLRc:1;
            vuint32_t:6;
            vuint32_t SETd:1;
            vuint32_t CLRd:1;
        } B;
    } SSCIR12_15;

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t SETa:1;
            vuint32_t CLRa:1;
            vuint32_t:6;
            vuint32_t SETb:1;
            vuint32_t CLRb:1;
            vuint32_t:6;
            vuint32_t SETc:1;
            vuint32_t CLRc:1;
            vuint32_t:6;
            vuint32_t SETd:1;
            vuint32_t CLRd:1;
        } B;
    } SSCIR16_19;

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t SETa:1;
            vuint32_t CLRa:1;
            vuint32_t:6;
            vuint32_t SETb:1;
            vuint32_t CLRb:1;
            vuint32_t:6;
            vuint32_t SETc:1;
            vuint32_t CLRc:1;
            vuint32_t:6;
            vuint32_t SETd:1;
            vuint32_t CLRd:1;
        } B;
    } SSCIR20_23;

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t SETa:1;
            vuint32_t CLRa:1;
            vuint32_t:6;
            vuint32_t SETb:1;
            vuint32_t CLRb:1;
            vuint32_t:6;
            vuint32_t SETc:1;
            vuint32_t CLRc:1;
            vuint32_t:6;
            vuint32_t SETd:1;
            vuint32_t CLRd:1;
        } B;
    } SSCIR24_27;

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t SETa:1;
            vuint32_t CLRa:1;
            vuint32_t:6;
            vuint32_t SETb:1;
            vuint32_t CLRb:1;
            vuint32_t:6;
            vuint32_t SETc:1;
            vuint32_t CLRc:1;
            vuint32_t:6;
            vuint32_t SETd:1;
            vuint32_t CLRd:1;
        } B;
    } SSCIR28_31;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR0_1;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR2_3;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR4_5;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR6_7;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR8_9;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR10_11;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR12_13;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR14_15;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR16_17;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR18_19;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR20_21;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR22_23;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR24_25;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR26_27;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR28_29;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR30_31;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR32_33;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR34_35;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR36_37;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR38_39;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR40_41;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR42_43;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR44_45;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR46_47;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR48_49;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR50_51;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR52_53;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR54_55;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR56_57;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR58_59;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR60_61;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR62_63;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR64_65;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR66_67;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR68_69;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR70_71;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR72_73;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR74_75;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR76_77;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR78_79;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR80_81;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR82_83;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR84_85;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR86_87;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR88_89;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR90_91;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR92_93;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR94_95;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR96_97;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR98_99;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR100_101;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR102_103;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR104_105;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR106_107;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR108_109;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR110_111;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR112_113;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR114_115;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR116_117;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR118_119;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR120_121;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR122_123;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR124_125;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR126_127;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR128_129;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR130_131;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR132_133;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR134_135;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR136_137;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR138_139;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR140_141;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR142_143;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR144_145;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR146_147;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR148_149;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR150_151;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR152_153;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR154_155;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR156_157;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR158_159;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR160_161;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR162_163;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR164_165;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR166_167;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR168_169;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR170_171;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR172_173;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR174_175;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR176_177;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR178_179;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR180_181;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR182_183;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR184_185;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR186_187;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR188_189;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR190_191;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR192_193;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR194_195;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR196_197;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR198_199;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR200_201;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR202_203;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR204_205;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR206_207;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR208_209;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR210_211;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR212_213;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR214_215;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR216_217;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR218_219;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR220_221;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR222_223;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR224_225;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR226_227;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR228_229;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR230_231;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR232_233;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR234_235;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR236_237;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR238_239;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR240_241;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR242_243;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR244_245;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR246_247;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR248_249;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR250_251;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR252_253;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR254_255;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR256_257;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR258_259;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR260_261;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR262_263;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR264_265;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR266_267;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR268_269;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR270_271;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR272_273;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR274_275;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR276_277;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR278_279;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR280_281;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR282_283;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR284_285;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR286_287;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR288_289;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR290_291;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR292_293;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR294_295;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR296_297;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR298_299;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR300_301;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR302_303;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR304_305;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR306_307;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR308_309;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR310_311;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR312_313;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR314_315;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR316_317;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR318_319;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR320_321;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR322_323;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR324_325;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR326_327;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR328_329;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR330_331;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR332_333;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR334_335;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR336_337;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR338_339;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR340_341;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR342_343;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR344_345;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR346_347;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR348_349;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR350_351;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR352_353;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR354_355;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR356_357;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR358_359;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR360_361;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR362_363;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR364_365;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR366_367;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR368_369;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR370_371;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR372_373;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR374_375;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR376_377;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR378_379;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR380_381;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR382_383;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR384_385;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR386_387;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR388_389;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR390_391;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR392_393;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR394_395;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR396_397;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR398_399;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR400_401;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR402_403;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR404_405;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR406_407;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR408_409;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR410_411;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR412_413;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR414_415;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR416_417;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR418_419;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR420_421;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR422_423;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR424_425;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR426_427;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR428_429;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR430_431;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR432_433;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR434_435;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR436_437;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR438_439;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR440_441;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR442_443;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR444_445;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR446_447;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR448_449;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR450_451;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR452_453;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR454_455;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR456_457;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR458_459;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR460_461;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR462_463;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR464_465;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR466_467;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR468_469;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR470_471;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR472_473;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR474_475;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR476_477;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR478_479;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR480_481;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR482_483;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR484_485;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR486_487;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR488_489;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR490_491;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR492_493;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR494_495;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR496_497;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR498_499;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR500_501;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR502_503;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR504_505;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR506_507;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR508_509;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR510_511;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR512_513;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR514_515;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR516_517;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR518_519;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR520_521;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR522_523;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR524_525;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR526_527;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR528_529;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR530_531;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR532_533;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR534_535;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR536_537;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR538_539;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR540_541;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR542_543;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR544_545;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR546_547;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR548_549;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR550_551;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR552_553;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR554_555;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR556_557;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR558_559;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR560_561;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR562_563;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR564_565;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR566_567;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR568_569;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR570_571;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR572_573;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR574_575;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR576_577;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR578_579;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR580_581;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR582_583;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR584_585;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR586_587;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR588_589;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR590_591;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR592_593;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR594_595;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR596_597;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR598_599;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR600_601;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR602_603;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR604_605;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR606_607;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR608_609;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR610_611;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR612_613;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR614_615;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR616_617;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR618_619;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR620_621;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR622_623;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR624_625;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR626_627;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR628_629;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR630_631;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR632_633;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR634_635;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR636_637;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR638_639;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR640_641;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR642_643;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR644_645;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR646_647;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR648_649;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR650_651;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR652_653;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR654_655;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR656_657;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR658_659;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR660_661;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR662_663;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR664_665;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR666_667;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR668_669;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR670_671;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR672_673;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR674_675;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR676_677;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR678_679;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR680_681;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR682_683;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR684_685;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR686_687;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR688_689;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR690_691;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR692_693;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR694_695;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR696_697;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR698_699;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR700_701;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR702_703;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR704_705;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR706_707;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR708_709;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR710_711;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR712_713;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR714_715;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR716_717;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR718_719;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR720_721;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR722_723;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR724_725;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR726_727;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR728_729;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR730_731;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR732_733;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR734_735;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR736_737;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR738_739;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR740_741;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR742_743;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR744_745;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR746_747;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR748_749;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR750_751;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR752_753;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR754_755;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR756_757;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR758_759;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR760_761;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR762_763;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR764_765;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR766_767;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR768_769;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR770_771;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR772_773;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR774_775;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR776_777;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR778_779;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR780_781;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR782_783;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR784_785;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR786_787;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR788_789;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR790_791;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR792_793;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR794_795;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR796_797;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR798_799;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR800_801;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR802_803;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR804_805;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR806_807;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR808_809;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR810_811;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR812_813;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR814_815;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR816_817;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR818_819;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR820_821;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR822_823;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR824_825;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR826_827;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR828_829;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR830_831;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR832_833;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR834_835;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR836_837;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR838_839;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR840_841;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR842_843;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR844_845;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR846_847;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR848_849;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR850_851;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR852_853;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR854_855;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR856_857;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR858_859;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR860_861;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR862_863;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR864_865;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR866_867;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR868_869;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR870_871;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR872_873;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR874_875;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR876_877;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR878_879;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR880_881;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR882_883;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR884_885;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR886_887;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR888_889;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR890_891;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR892_893;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR894_895;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR896_897;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR898_899;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR900_901;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR902_903;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR904_905;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR906_907;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR908_909;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR910_911;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR912_913;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR914_915;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR916_917;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR918_919;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR920_921;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR922_923;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR924_925;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR926_927;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR928_929;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR930_931;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR932_933;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR934_935;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR936_937;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR938_939;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR940_941;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR942_943;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR944_945;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR946_947;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR948_949;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR950_951;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR952_953;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR954_955;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR956_957;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR958_959;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR960_961;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR962_963;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR964_965;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR966_967;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR968_969;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR970_971;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR972_973;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR974_975;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR976_977;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR978_979;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR980_981;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR982_983;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR984_985;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR986_987;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR988_989;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR990_991;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR992_993;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR994_995;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR996_997;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR998_999;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR1000_1001;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR1002_1003;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR1004_1005;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR1006_1007;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR1008_1009;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR1010_1011;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR1012_1013;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR1014_1015;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR1016_1017;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR1018_1019;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR1020_1021;

    union {
        vuint32_t R;
        struct {
            vuint32_t PRC_SELa:4;
            vuint32_t:3;
            vuint32_t SWTa:1;
            vuint32_t:2;
            vuint32_t PRIa:6;
            vuint32_t PRC_SELb:4;
            vuint32_t:3;
            vuint32_t SWTb:1;
            vuint32_t:2;
            vuint32_t PRIb:6;
        } B;
    } PSR1022_1023;
};
/**************************************************************************/
/*                   Module: JDC                                          */
/**************************************************************************/
struct JDC_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:15;
            vuint32_t JIN_IEN:1;
            vuint32_t:15;
            vuint32_t JOUT_IEN:1;
        } B;
    } MCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:13;
            vuint32_t JIN_RDY:1;
            vuint32_t:1;
            vuint32_t JIN_INT:1;
            vuint32_t:13;
            vuint32_t JOUT_RDY:1;
            vuint32_t:1;
            vuint32_t JOUT_INT:1;
        } B;
    } MSR;

    union {
        vuint32_t R;
        struct {
            vuint32_t JOUT_IPSData:32;
        } B;
    } JOUT_IPS;

    union {
        vuint32_t R;
        struct {
            vuint32_t JIN_IPSData:32;
        } B;
    } JIN_IPS;
};
/**************************************************************************/
/*                   Module: JTAGM                                        */
/**************************************************************************/
struct JTAGM_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t DTM:1;
            vuint32_t jtagm_JCOMP:1;
            vuint32_t TCKSEL:3;
            vuint32_t IIE:1;
            vuint32_t SIE:1;
            vuint32_t:1;
            vuint32_t inter_jtag_frame_timer:6;
            vuint32_t:2;
            vuint32_t:8;
            vuint32_t evti1_assert:1;
            vuint32_t evti0_assert:1;
            vuint32_t evto_IE:1;
            vuint32_t evto1_sense:2;
            vuint32_t evto0_sense:2;
            vuint32_t SWRESET:1;
        } B;
    } MCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t CRC:8;    /* DigRF mode */
            vuint32_t NR:1;
            vuint32_t Idle:1;
            vuint32_t Nexus_err:1;
            vuint32_t CRC_err:1;
            vuint32_t SPU_INT:1;
            vuint32_t SPU_INT_CLR:1;
            vuint32_t evto1_edge:1;
            vuint32_t evto0_edge:1;
            vuint32_t dci_status:8;
            vuint32_t digrf_status:6;
            vuint32_t:1;
            vuint32_t overrun:1;
            // vuint32_t:6;                              /* SW mode */
            // vuint32_t evto1_clr:1;
            // vuint32_t evto0_clr:1;
            // vuint32_t NR:1;
            // vuint32_t Idle:1;
            // vuint32_t Nexus_err:1;
            // vuint32_t CRC_err:1;
            // vuint32_t SPU_INT:1;
            // vuint32_t SPU_INT_CLR:1;
            // vuint32_t evto1_edge:1;
            // vuint32_t evto0_edge:1;
            // vuint32_t dci_status:8;
            // vuint32_t digrf_status:6;
            // vuint32_t:2;
        } B;
    } SR;

    union {
        vuint32_t R;
        struct {
            vuint32_t TMS_HIGH:32;
        } B;
    } DOR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t TMS_LOW:28;
        } B;
    } DOR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t TDI_HIGH:32;
        } B;
    } DOR2;

    union {
        vuint32_t R;
        struct {
            vuint32_t Send:1;
            vuint32_t:3;
            vuint32_t TDI_LOW:28;
        } B;
    } DOR3;

    union {
        vuint32_t R;
        struct {
            vuint32_t CRC:8;
            vuint32_t:24;
        } B;
    } RxCRC;

    union {
        vuint32_t R;
        struct {
            vuint32_t TDO_LOW:32;
        } B;
    } DIR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t TDO_HIGH:28;
        } B;
    } DIR1;
};
/**************************************************************************/
/*                   Module: LINFlexD                                     */
/**************************************************************************/
struct LINFlexD_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t CCD:1;
            vuint32_t CFD:1;
            vuint32_t LASE:1;
            vuint32_t AUTOWU:1;
            vuint32_t MBL:4;
            vuint32_t BF:1;
            vuint32_t:1;
            vuint32_t LBKM:1;
            vuint32_t MME:1;
            vuint32_t SSBL:1;
            vuint32_t RBLM:1;
            vuint32_t SLEEP:1;
            vuint32_t INIT:1;
        } B;
    } LINCR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t SZIE:1;
            vuint32_t OCIE:1;
            vuint32_t BEIE:1;
            vuint32_t CEIE:1;
            vuint32_t HEIE:1;
            vuint32_t:2;
            vuint32_t FEIE:1;
            vuint32_t BOIE:1;
            vuint32_t LSIE:1;
            vuint32_t WUIE:1;
            vuint32_t DBFIE:1;
            vuint32_t DBEIE_TOIE:1;
            vuint32_t DRIE:1;
            vuint32_t DTIE:1;
            vuint32_t HRIE:1;
        } B;
    } LINIER;

    union {
        vuint32_t R;
        struct {
            vuint32_t:13;
            vuint32_t RDC:3;
            vuint32_t LINS:4;
            vuint32_t:2;
            vuint32_t RMB:1;
            vuint32_t DRBNE:1;
            vuint32_t RXbusy:1;
            vuint32_t RDI:1;
            vuint32_t WUF:1;
            vuint32_t DBFF:1;
            vuint32_t DBEF:1;
            vuint32_t DRF:1;
            vuint32_t DTF:1;
            vuint32_t HRF:1;
        } B;
    } LINSR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t SZF:1;
            vuint32_t OCF:1;
            vuint32_t BEF:1;
            vuint32_t CEF:1;
            vuint32_t SFEF:1;
            vuint32_t SDEF:1;
            vuint32_t IDPEF:1;
            vuint32_t FEF:1;
            vuint32_t BOF:1;
            vuint32_t:6;
            vuint32_t NF:1;
        } B;
    } LINESR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:9;
            vuint32_t NEF:3;
            vuint32_t DTU:1;
            vuint32_t SBUR:2;
            vuint32_t WLS:1;
            vuint32_t TDFL_TFC:3;
            vuint32_t RDFL_RFC:3;
            vuint32_t RFBM:1;
            vuint32_t TFBM:1;
            vuint32_t WL1:1;
            vuint32_t PC1:1;
            vuint32_t RxEn:1;
            vuint32_t TxEn:1;
            vuint32_t PC0:1;
            vuint32_t PCE:1;
            vuint32_t WL0:1;
            vuint32_t UART:1;
        } B;
    } UARTCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t SZF:1;
            vuint32_t OCF:1;
            vuint32_t PE:4;
            vuint32_t RMB:1;
            vuint32_t FEF:1;
            vuint32_t BOF:1;
            vuint32_t RDI:1;
            vuint32_t WUF:1;
            vuint32_t RFNE:1;
            vuint32_t TO:1;
            vuint32_t DRF_RFE:1;
            vuint32_t DTF_TFF:1;
            vuint32_t NF:1;
        } B;
    } UARTSR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:21;
            vuint32_t MODE:1;
            vuint32_t IOT:1;
            vuint32_t TOCE:1;
            vuint32_t CNT:8;
        } B;
    } LINTCSR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t OC2:8;
            vuint32_t OC1:8;
        } B;
    } LINOCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:20;
            vuint32_t RTO:4;
            vuint32_t:1;
            vuint32_t HTO:7;
        } B;
    } LINTOCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:28;
            vuint32_t FBR:4;
        } B;
    } LINFBRR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t IBR:20;
        } B;
    } LINIBRR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t CF:8;
        } B;
    } LINCFR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t TBDE:1;
            vuint32_t IOBE:1;
            vuint32_t IOPE:1;
            vuint32_t WURQ:1;
            vuint32_t DDRQ:1;
            vuint32_t DTRQ:1;
            vuint32_t ABRQ:1;
            vuint32_t HTRQ:1;
            vuint32_t:8;
        } B;
    } LINCR2;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t DFL:6;
            vuint32_t DIR:1;
            vuint32_t CCS:1;
            vuint32_t:2;
            vuint32_t ID:6;
        } B;
    } BIDR;

    union {
        vuint32_t R;
        struct {
            vuint32_t DATA3:8;
            vuint32_t DATA2:8;
            vuint32_t DATA1:8;
            vuint32_t DATA0:8;
        } B;
    } BDRL;

    union {
        vuint32_t R;
        struct {
            vuint32_t DATA7:8;
            vuint32_t DATA6:8;
            vuint32_t DATA5:8;
            vuint32_t DATA4:8;
        } B;
    } BDRM;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t FACT:16;
        } B;
    } IFER;

    union {
        vuint32_t R;
        struct {
            vuint32_t:27;
            vuint32_t IFMI:5;
        } B;
    } IFMI;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t IFM:8;
        } B;
    } IFMR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t DFL:6;
            vuint32_t DIR:1;
            vuint32_t CCS:1;
            vuint32_t:2;
            vuint32_t ID:6;
        } B;
    } IFCR[16];

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TDFBM:1;
            vuint32_t RDFBM:1;
            vuint32_t TDLIS:1;
            vuint32_t RDLIS:1;
            vuint32_t STOP:1;
            vuint32_t SR:1;
        } B;
    } GCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:20;
            vuint32_t PTO:12;
        } B;
    } UARTPTO;

    union {
        vuint32_t R;
        struct {
            vuint32_t:20;
            vuint32_t CTO:12;
        } B;
    } UARTCTO;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t DTE:31;
        } B;
    } DMATXE;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t DTE:31;
        } B;
    } DMARXE;
};
/**************************************************************************/
/*                   Module: MCAN                                         */
/**************************************************************************/
struct MCAN_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t REL:4;
            vuint32_t STEP:4;
            vuint32_t SUBSTEP:4;
            vuint32_t YEAR:4;
            vuint32_t MON:8;
            vuint32_t DAY:8;
        } B;
    } CREL;

    union {
        vuint32_t R;
        struct {
            vuint32_t ETV:32;
        } B;
    } ENDN;

    uint8_t MCAN_reserved1[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t RX:1;
            vuint32_t TX:2;
            vuint32_t LBCK:1;
            vuint32_t:4;
        } B;
    } TEST;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t WDV:8;
            vuint32_t WDC:8;
        } B;
    } RWD;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t TEST:1;
            vuint32_t DAR:1;
            vuint32_t MON:1;
            vuint32_t CSR:1;
            vuint32_t CSA:1;
            vuint32_t ASM:1;
            vuint32_t CCE:1;
            vuint32_t INIT:1;
        } B;
    } CCCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t BRPE:4;
            vuint32_t:1;
            vuint32_t TSEG2:3;
            vuint32_t TSEG1:4;
            vuint32_t SJW:2;
            vuint32_t BRPL:6;
        } B;
    } BTP;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t TCP:4;
            vuint32_t:14;
            vuint32_t TSS:2;
        } B;
    } TSCC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t TSC:16;
        } B;
    } TSCV;

    union {
        vuint32_t R;
        struct {
            vuint32_t TOP:16;
            vuint32_t:13;
            vuint32_t TOS:2;
            vuint32_t ETOC:1;
        } B;
    } TOCC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t TSC:16;
        } B;
    } TOCV;

    uint8_t MCAN_reserved2[16];

    union {
        vuint32_t R;
        struct {
            vuint32_t:8;
            vuint32_t CEL:8;
            vuint32_t:1;
            vuint32_t REC:7;
            vuint32_t TEC:8;
        } B;
    } ECR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t BO:1;
            vuint32_t EW:1;
            vuint32_t EP:1;
            vuint32_t ACT:2;
            vuint32_t LEC:3;
        } B;
    } PSR;

    uint8_t MCAN_reserved3[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t STE:1;
            vuint32_t FOE:1;
            vuint32_t ACKE:1;
            vuint32_t BE:1;
            vuint32_t CRCE:1;
            vuint32_t WDI:1;
            vuint32_t BO:1;
            vuint32_t EW:1;
            vuint32_t EP:1;
            vuint32_t ELO:1;
            vuint32_t BEU:1;
            vuint32_t BEC:1;
            vuint32_t:1;
            vuint32_t TOO:1;
            vuint32_t UMD:1;
            vuint32_t TSW:1;
            vuint32_t TEFL:1;
            vuint32_t TEFF:1;
            vuint32_t TEFW:1;
            vuint32_t TEFN:1;
            vuint32_t TFE:1;
            vuint32_t TCF:1;
            vuint32_t TC:1;
            vuint32_t HPM:1;
            vuint32_t RF1L:1;
            vuint32_t RF1F:1;
            vuint32_t RF1W:1;
            vuint32_t RF1N:1;
            vuint32_t RF0L:1;
            vuint32_t RF0F:1;
            vuint32_t RF0W:1;
            vuint32_t RF0N:1;
        } B;
    } IR;

    union {
        vuint32_t R;
        struct {
            vuint32_t STEE:1;
            vuint32_t FOEE:1;
            vuint32_t ACKEE:1;
            vuint32_t BEE:1;
            vuint32_t CRCEE:1;
            vuint32_t WDIE:1;
            vuint32_t BOE:1;
            vuint32_t EWE:1;
            vuint32_t EPE:1;
            vuint32_t ELOE:1;
            vuint32_t BEUE:1;
            vuint32_t BECE:1;
            vuint32_t:1;
            vuint32_t TOOE:1;
            vuint32_t UMDE:1;
            vuint32_t TSWE:1;
            vuint32_t TEFLE:1;
            vuint32_t TEFFE:1;
            vuint32_t TEFWE:1;
            vuint32_t TEFNE:1;
            vuint32_t TFEE:1;
            vuint32_t TCFE:1;
            vuint32_t TCE:1;
            vuint32_t HPME:1;
            vuint32_t RF1LE:1;
            vuint32_t RF1FE:1;
            vuint32_t RF1WE:1;
            vuint32_t RF1NE:1;
            vuint32_t RF0LE:1;
            vuint32_t RF0FE:1;
            vuint32_t RF0WE:1;
            vuint32_t RF0NE:1;
        } B;
    } IE;

    union {
        vuint32_t R;
        struct {
            vuint32_t STEL:1;
            vuint32_t FOEL:1;
            vuint32_t ACKEL:1;
            vuint32_t BEL:1;
            vuint32_t CRCEL:1;
            vuint32_t WDIL:1;
            vuint32_t BOL:1;
            vuint32_t EWL:1;
            vuint32_t EPL:1;
            vuint32_t ELOL:1;
            vuint32_t BEUL:1;
            vuint32_t BECL:1;
            vuint32_t:1;
            vuint32_t TOOL:1;
            vuint32_t UMDL:1;
            vuint32_t TSWL:1;
            vuint32_t TEFLL:1;
            vuint32_t TEFFL:1;
            vuint32_t TEFWL:1;
            vuint32_t TEFNL:1;
            vuint32_t TFEL:1;
            vuint32_t TCFL:1;
            vuint32_t TCL:1;
            vuint32_t HPML:1;
            vuint32_t RF1LL:1;
            vuint32_t RF1FL:1;
            vuint32_t RF1WL:1;
            vuint32_t RF1NL:1;
            vuint32_t RF0LL:1;
            vuint32_t RF0FL:1;
            vuint32_t RF0WL:1;
            vuint32_t RF0NL:1;
        } B;
    } ILS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:30;
            vuint32_t EINT1:1;
            vuint32_t EINT0:1;
        } B;
    } ILE;

    uint8_t MCAN_reserved4[32];

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t ANS:2;
            vuint32_t ANFE:2;
            vuint32_t RRFS:1;
            vuint32_t RRFE:1;
        } B;
    } GFC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:8;
            vuint32_t LSS:8;
            vuint32_t FLSSA:14;
            vuint32_t:2;
        } B;
    } SIDFC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:9;
            vuint32_t LSE:7;
            vuint32_t FLESA:14;
            vuint32_t:2;
        } B;
    } XIDFC;

    uint8_t MCAN_reserved5[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:3;
            vuint32_t EIDM:29;
        } B;
    } XIDAM;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t FLST:1;
            vuint32_t FIDX:7;
            vuint32_t MSI:2;
            vuint32_t BIDX:6;
        } B;
    } HPMS;

    uint8_t MCAN_reserved6[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t F0WM:7;
            vuint32_t:2;
            vuint32_t F0S:6;
            vuint32_t FOSA:14;
            vuint32_t:2;
        } B;
    } RXF0C;

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t RF0L:1;
            vuint32_t F0F:1;
            vuint32_t:10;
            vuint32_t F0GI:6;
            vuint32_t:1;
            vuint32_t F0FL:7;
        } B;
    } RXF0S;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t F0AI:6;
        } B;
    } RXF0A;

    uint8_t MCAN_reserved7[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t F1WM:7;
            vuint32_t:1;
            vuint32_t F1S:7;
            vuint32_t F1SA:14;
            vuint32_t:2;
        } B;
    } RXF1C;

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t RF1L:1;
            vuint32_t F1F:1;
            vuint32_t:10;
            vuint32_t F1G1:6;
            vuint32_t:1;
            vuint32_t F1FL:7;
        } B;
    } RXF1S;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t F1AI:6;
        } B;
    } RXF1A;

    uint8_t MCAN_reserved8[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t TQFM:1;
            vuint32_t TQFS:6;
            vuint32_t:2;
            vuint32_t NDTB:6;
            vuint32_t TBSA:14;
            vuint32_t:2;
        } B;
    } TXBC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:10;
            vuint32_t TFQF:1;
            vuint32_t TFQPI:5;
            vuint32_t:10;
            vuint32_t TFFL:6;
        } B;
    } TXFQS;

    uint8_t MCAN_reserved9[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t TRP:32;
        } B;
    } TXBRP;

    union {
        vuint32_t R;
        struct {
            vuint32_t AR:32;
        } B;
    } TXBAR;

    union {
        vuint32_t R;
        struct {
            vuint32_t CR:32;
        } B;
    } TXBCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t TO:32;
        } B;
    } TXBTO;

    union {
        vuint32_t R;
        struct {
            vuint32_t CF:32;
        } B;
    } TXBCF;

    union {
        vuint32_t R;
        struct {
            vuint32_t TIE:32;
        } B;
    } TXBTIE;

    union {
        vuint32_t R;
        struct {
            vuint32_t CFIE:32;
        } B;
    } TXBCIE;

    uint8_t MCAN_reserved10[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:2;
            vuint32_t EFWM:6;
            vuint32_t:2;
            vuint32_t EFS:6;
            vuint32_t EFSA:14;
            vuint32_t:2;
        } B;
    } TXEFC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t TEFL:1;
            vuint32_t EFF:1;
            vuint32_t:11;
            vuint32_t EFGI:5;
            vuint32_t:2;
            vuint32_t EFFL:6;
        } B;
    } TXEFS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:27;
            vuint32_t EFAI:5;
        } B;
    } TXEFA;
};
/**************************************************************************/
/*                   Module: MC_CGM                                       */
/**************************************************************************/
struct MC_CGM_tag {
    union {
        vuint8_t R;
        struct {
            vuint8_t SDUR:8;
        } B;
    } PCS_SDUR;

    uint8_t MC_CGM_reserved1[3];

    union {
        vuint32_t R;
        struct {
            vuint32_t INIT:16;
            vuint32_t:8;
            vuint32_t RATE:8;
        } B;
    } PCS_DIVC1;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t DIVS:20;
        } B;
    } PCS_DIVS1;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t DIVE:20;
        } B;
    } PCS_DIVE1;

    union {
        vuint32_t R;
        struct {
            vuint32_t INIT:16;
            vuint32_t:8;
            vuint32_t RATE:8;
        } B;
    } PCS_DIVC2;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t DIVS:20;
        } B;
    } PCS_DIVS2;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t DIVE:20;
        } B;
    } PCS_DIVE2;

    uint8_t MC_CGM_reserved2[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t INIT:16;
            vuint32_t:8;
            vuint32_t RATE:8;
        } B;
    } PCS_DIVC4;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t DIVS:20;
        } B;
    } PCS_DIVS4;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t DIVE:20;
        } B;
    } PCS_DIVE4;

	uint8_t MC_CGM_reserved3[156];
	
    union {
        vuint32_t R;
        struct {
            vuint32_t:31;
            vuint32_t SYS_DIV_RATIO_CHNG:1;
        } B;
    } SC_DIV_RC;
	
    union {
        vuint32_t R;
        struct {
            vuint32_t SYS_UPD_TYPE:1;
            vuint32_t:29;
            vuint32_t AUX1_UPD_TYPE:1;
            vuint32_t AUX0_UPD_TYPE:1;
        } B;
    } DIV_UPD_TYPE;
	
    union {
        vuint32_t R;
        struct {
            vuint32_t DIV_UPD_TGR:32;
        } B;
    } DIV_UPD_TRIG;
	
    union {
        vuint32_t R;
        struct {
            vuint32_t SYS_UPD_STAT:1;
            vuint32_t:29;
            vuint32_t AUX1_UPD_STAT:1;
            vuint32_t AUX0_UPD_STAT:1;
        } B;
    } DIV_UPD_STAT;
	
	uint8_t MC_CGM_reserved4[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t SELSTAT:4;
            vuint32_t:4;
            vuint32_t SWTRG:3;
            vuint32_t SWIP:1;
            vuint32_t:16;
        } B;
    } SC_SS;

    union {
        vuint32_t R;
        struct {
            vuint32_t DE:1;
            vuint32_t:12;
            vuint32_t DIV:3;
            vuint32_t:16;
        } B;
    } SC_DC0;

     union {
        vuint32_t R;
        struct {
            vuint32_t DE:1;
            vuint32_t:11;
            vuint32_t DIV:4;
            vuint32_t:16;
        } B;
    } SC_DC1;

     union {
        vuint32_t R;
        struct {
            vuint32_t DE:1;
            vuint32_t:7;
            vuint32_t DIV:8;
            vuint32_t:16;
        } B;
    } SC_DC2;

    uint8_t MC_CGM_reserved5[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t SELCTL:4;
            vuint32_t:24;
        } B;
    } AC0_SC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t SELSTAT:4;
            vuint32_t:24;
        } B;
    } AC0_SS;

    union {
        vuint32_t R;
        struct {
            vuint32_t DE:1;
            vuint32_t:11;
            vuint32_t DIV:4;
            vuint32_t:16;
        } B;
    } AC0_DC0;

     union {
        vuint32_t R;
        struct {
            vuint32_t DE:1;
            vuint32_t:8;
            vuint32_t DIV:7;
            vuint32_t:16;
        } B;
    } AC0_DC1;
    
     union {
        vuint32_t R;
        struct {
            vuint32_t DE:1;
            vuint32_t:8;
            vuint32_t DIV:7;
            vuint32_t:16;
        } B;
    } AC0_DC2;

    union {
        vuint32_t R;
        struct {
            vuint32_t DE:1;
            vuint32_t:8;
            vuint32_t DIV:7;
            vuint32_t:16;
        } B;
    } AC0_DC3;

    union {
        vuint32_t R;
        struct {
            vuint32_t DE:1;
            vuint32_t:11;
            vuint32_t DIV:4;
            vuint32_t:16;
        } B;
    } AC0_DC4;

    union {
        vuint32_t R;
        struct {
            vuint32_t DE:1;
            vuint32_t:11;
            vuint32_t DIV:4;
            vuint32_t:16;
        } B;
    } AC0_DC5;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t SELCTL:4;
            vuint32_t:24;
        } B;
    } AC1_SC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t SELSTAT:4;
            vuint32_t:24;
        } B;
    } AC1_SS;

    union {
        vuint32_t R;
        struct {
            vuint32_t DE:1;
            vuint32_t:9;
            vuint32_t DIV:6;
            vuint32_t:16;
        } B;
    } AC1_DC0;

    uint8_t MC_CGM_reserved6[20];

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t SELCTL:4;
            vuint32_t:24;
        } B;
    } AC2_SC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t SELSTAT:4;
            vuint32_t:24;
        } B;
    } AC2_SS;

    uint8_t MC_CGM_reserved7[24];

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t SELCTL:4;
            vuint32_t:24;
        } B;
    } AC3_SC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t SELSTAT:4;
            vuint32_t:24;
        } B;
    } AC3_SS;

 
};
/**************************************************************************/
/*                   Module: MC_ME                                        */
/**************************************************************************/
struct MC_ME_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t S_CURRENT_MODE:4;
            vuint32_t S_MTRANS:1;
            vuint32_t:3;
            vuint32_t S_PDO:1;
            vuint32_t:2;
            vuint32_t S_MVR:1;
            vuint32_t:2;
            vuint32_t S_FLA:2;
            vuint32_t:8;
            vuint32_t S_PLL1:1;
            vuint32_t S_PLL0:1;
            vuint32_t S_XOSC:1;
            vuint32_t S_IRC:1;
            vuint32_t S_SYSCLK:4;
        } B;
    } GS;

    union {
        vuint32_t R;
        struct {
            vuint32_t TARGET_MODE:4;
            vuint32_t:12;
            vuint32_t KEY:16;
        } B;
    } MCTL;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t RESET_DEST:1;
            vuint32_t:4;
            vuint32_t STOP0:1;
            vuint32_t:1;
            vuint32_t HALT0:1;
            vuint32_t RUN3:1;
            vuint32_t RUN2:1;
            vuint32_t RUN1:1;
            vuint32_t RUN0:1;
            vuint32_t DRUN:1;
            vuint32_t SAFE:1;
            vuint32_t TEST:1;
            vuint32_t RESET_FUNC:1;
        } B;
    } ME;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t I_ICONF_CC:1;
            vuint32_t I_ICONF_CU:1;
            vuint32_t I_ICONF:1;
            vuint32_t I_IMODE:1;
            vuint32_t I_SAFE:1;
            vuint32_t I_MTC:1;
        } B;
    } IS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:27;
            vuint32_t M_ICONF_CU:1;
            vuint32_t M_ICONF:1;
            vuint32_t M_IMODE:1;
            vuint32_t M_SAFE:1;
            vuint32_t M_MTC:1;
        } B;
    } IM;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
			vuint32_t S_MRIG:1;
            vuint32_t S_MTI:1;
            vuint32_t S_MRI:1;
            vuint32_t S_DMA:1;
            vuint32_t S_NMA:1;
            vuint32_t S_SEA:1;
        } B;
    } IMTS;

    union {
        vuint32_t R;
        struct {
            vuint32_t PREVIOUS_MODE:4;
            vuint32_t:4;
            vuint32_t MPH_BUSY:1;
            vuint32_t:2;
            vuint32_t PMC_PROG:1;
            vuint32_t DBG_MODE:1;
            vuint32_t CCKL_PROG:1;
            vuint32_t PCS_PROG:1;
            vuint32_t SMR:1;
            vuint32_t:1;
            vuint32_t VREG_CSRC_SC:1;
            vuint32_t CSRC_CSRC_SC:1;
            vuint32_t IRCOSC_SC:1;
            vuint32_t SCSRC_SC:1;
            vuint32_t SYSCLK_SW:1;
            vuint32_t:1;
            vuint32_t FLASH_SC:1;
            vuint32_t CDP_PRPH_224_255:1;
            vuint32_t CDP_PRPH_192_223:1;
            vuint32_t CDP_PRPH_160_191:1;
            vuint32_t CDP_PRPH_128_159:1;
            vuint32_t CDP_PRPH_96_127:1;
            vuint32_t CDP_PRPH_64_95:1;
            vuint32_t CDP_PRPH_32_63:1;
            vuint32_t CDP_PRPH_0_31:1;
        } B;
    } DMTS;

    uint8_t MC_ME_reserved1[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t PWRLVL:3;
            vuint32_t:4;
            vuint32_t PDO:1;
            vuint32_t:2;
            vuint32_t MVRON:1;
            vuint32_t:2;
            vuint32_t FLAON:2;
            vuint32_t:8;
            vuint32_t PLL1ON:1;
            vuint32_t PLL0ON:1;
            vuint32_t XOSCON:1;
            vuint32_t IRCON:1;
            vuint32_t SYSCLK:4;
        } B;
    } RESET_MC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t PWRLVL:3;
            vuint32_t:4;
            vuint32_t PDO:1;
            vuint32_t:2;
            vuint32_t MVRON:1;
            vuint32_t:2;
            vuint32_t FLAON:2;
            vuint32_t:8;
            vuint32_t PLL1ON:1;
            vuint32_t PLL0ON:1;
            vuint32_t XOSCON:1;
            vuint32_t IRCON:1;
            vuint32_t SYSCLK:4;
        } B;
    } TEST_MC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t PWRLVL:3;
            vuint32_t:4;
            vuint32_t PDO:1;
            vuint32_t:2;
            vuint32_t MVRON:1;
            vuint32_t:2;
            vuint32_t FLAON:2;
            vuint32_t:8;
            vuint32_t PLL1ON:1;
            vuint32_t PLL0ON:1;
            vuint32_t XOSCON:1;
            vuint32_t IRCON:1;
            vuint32_t SYSCLK:4;
        } B;
    } SAFE_MC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t PWRLVL:3;
            vuint32_t:4;
            vuint32_t PDO:1;
            vuint32_t:2;
            vuint32_t MVRON:1;
            vuint32_t:2;
            vuint32_t FLAON:2;
            vuint32_t:8;
            vuint32_t PLL1ON:1;
            vuint32_t PLL0ON:1;
            vuint32_t XOSCON:1;
            vuint32_t IRCON:1;
            vuint32_t SYSCLK:4;
        } B;
    } DRUN_MC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t PWRLVL:3;
            vuint32_t:4;
            vuint32_t PDO:1;
            vuint32_t:2;
            vuint32_t MVRON:1;
            vuint32_t:2;
            vuint32_t FLAON:2;
            vuint32_t:8;
            vuint32_t PLL1ON:1;
            vuint32_t PLL0ON:1;
            vuint32_t XOSCON:1;
            vuint32_t IRCON:1;
            vuint32_t SYSCLK:4;
        } B;
    } RUN_MC[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t PWRLVL:3;
            vuint32_t:4;
            vuint32_t PDO:1;
            vuint32_t:2;
            vuint32_t MVRON:1;
            vuint32_t:2;
            vuint32_t FLAON:2;
            vuint32_t:8;
            vuint32_t PLL1ON:1;
            vuint32_t PLL0ON:1;
            vuint32_t XOSCON:1;
            vuint32_t IRCON:1;
            vuint32_t SYSCLK:4;
        } B;
    } HALT_MC;

    uint8_t MC_ME_reserved2[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t PWRLVL:3;
            vuint32_t:4;
            vuint32_t PDO:1;
            vuint32_t:2;
            vuint32_t MVRON:1;
            vuint32_t:2;
            vuint32_t FLAON:2;
            vuint32_t:8;
            vuint32_t PLL1ON:1;
            vuint32_t PLL0ON:1;
            vuint32_t XOSCON:1;
            vuint32_t IRCON:1;
            vuint32_t SYSCLK:4;
        } B;
    } STOP_MC;

    uint8_t MC_ME_reserved3[20];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t S_PIT_0:1;
            vuint32_t:30;
        } B;
    } PS0;

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t S_CRC_A0:1;
            vuint32_t:1;
            vuint32_t S_DMA_CH_MUX0:1;
            vuint32_t:4;
        } B;
    } PS1;

    union {
        vuint32_t R;
        struct {
            vuint32_t:3;
            vuint32_t S_LINFlexD_0:1;
            vuint32_t:12;
            vuint32_t S_FlexCAN_0:1;
            vuint32_t:15;
        } B;
    } PS2;

    union {
        vuint32_t R;
        struct {
            vuint32_t S_ADCSAR_0:1;
            vuint32_t S_ADCSEQ_1:1;
            vuint32_t:12;
            vuint32_t S_ADCSEQ_B:1;
            vuint32_t S_ADCSAR_B:1;
            vuint32_t:12;
            vuint32_t S_DSPI_0:1;
            vuint32_t S_DSPI_1:1;
            vuint32_t:2;
        } B;
    } PS3;

    union {
        vuint32_t R;
        struct {
            vuint32_t:28;
            vuint32_t S_eTimer_2:1;
            vuint32_t S_eTimer_3:1;
            vuint32_t S_eTimer_0:1;
            vuint32_t S_eTimer_1:1;        
         } B;
    } PS4;

    union {
        vuint32_t R;
        struct {
            vuint32_t:32;
        } B;
    } PS5;

    union {
        vuint32_t R;
        struct {
            vuint32_t:3;
            vuint32_t S_LINFlexD_1:1;
            vuint32_t:12;
            vuint32_t S_FlexCAN_1:1;
            vuint32_t:15;
        } B;
    } PS6;

    union {
        vuint32_t R;
        struct {
            vuint32_t:11;
            vuint32_t S_CTU:1;
            vuint32_t:16;
            vuint32_t S_DSPI_2:1;
            vuint32_t S_DSPI_3:1;
            vuint32_t:2;
        } B;
    } PS7;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t RUN3:1;
            vuint32_t RUN2:1;
            vuint32_t RUN1:1;
            vuint32_t RUN0:1;
            vuint32_t DRUN:1;
            vuint32_t SAFE:1;
            vuint32_t TEST:1;
            vuint32_t RESET:1;
        } B;
    } RUN_PC[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:21;
            vuint32_t STOP0:1;
            vuint32_t:1;
            vuint32_t HALT0:1;
            vuint32_t:8;
        } B;
    } LP_PC[8];

  /* SPC5STUDIO FIX */
  /* Removed dumb way to declare equivalent registers.*/
    union {
        vuint8_t R;
        struct {
            vuint8_t  :1;
            vuint8_t DBG_F:1;
            vuint8_t LP_CFG:3;
            vuint8_t RUN_CFG:3;
        } B;
    } PCTL[256];

    union {
        vuint32_t R;
        struct {
            vuint32_t:31;
            vuint32_t S_CORE0:1;
        } B;
    } CS;

    union {
        vuint16_t R;
        struct {
            vuint16_t:5;
            vuint16_t STOP0:1;
            vuint16_t:1;
            vuint16_t HALT0:1;
            vuint16_t RUN3:1;
            vuint16_t RUN2:1;
            vuint16_t RUN1:1;
            vuint16_t RUN0:1;
            vuint16_t DRUN:1;
            vuint16_t SAFE:1;
            vuint16_t TEST:1;
            vuint16_t RESET:1;
        } B;
    } CCTL0;

    uint8_t MC_ME_reserved4[26];

    union {
        vuint32_t R;
        struct {
            vuint32_t ADDR:30;
            vuint32_t:1;
            vuint32_t RMC:1;

        } B;
    } CADDR0;

    uint8_t MC_ME_reserved5[16];

};
/**************************************************************************/
/*                   Module: MC_RGM                                        */
/**************************************************************************/
struct MC_RGM_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:7;
            vuint32_t F_VOR_DEST:1;
            vuint32_t F_TSR_DEST:1;
            vuint32_t:7;
            vuint32_t F_HSM_DEST:1;
            vuint32_t:4;
            vuint32_t F_JTAG_DEST:1;
            vuint32_t:1;
            vuint32_t F_EDR:1;
            vuint32_t:3;
            vuint32_t F_FFRR:1;
            vuint32_t F_SOFT_DEST:1;
            vuint32_t:1;
            vuint32_t F_PORST:1;
            vuint32_t F_POR:1;
        } B;
    } DES;

    uint8_t MC_RGM_reserved1[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t:7;
            vuint32_t D_VOR_DEST:1;
            vuint32_t D_TSR_DEST:1;
            vuint32_t:7;
            vuint32_t D_HSM_DEST:1;
            vuint32_t:4;
            vuint32_t D_JTAG_DEST:1;
            vuint32_t:1;
            vuint32_t D_EDR:1;
            vuint32_t:3;
            vuint32_t D_FFRR:1;
            vuint32_t D_SOFT_DEST:1;
            vuint32_t:1;
            vuint32_t D_PORST:1;
            vuint32_t D_POR:1;
        } B;
    } DERD;

    uint8_t MC_RGM_reserved2[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t:8;
            vuint32_t AR_TSR_DEST:1;
            vuint32_t:21;
            vuint32_t AR_PORST:1;
            vuint32_t:1;
        } B;
    } DEAR;

    uint8_t MC_RGM_reserved3[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t:7;
            vuint32_t BE_VOR_DEST:1;
            vuint32_t BE_TSR_DEST:1;
            vuint32_t:7;
            vuint32_t BE_HSM_DEST:1;
            vuint32_t:4;
            vuint32_t BE_JTAG_DEST:1;
            vuint32_t:1;
            vuint32_t BE_EDR:1;
            vuint32_t:3;
            vuint32_t BE_FFRR:1;
            vuint32_t BE_SOF_DEST:1;
            vuint32_t:1;
            vuint32_t BE_PORST:1;
            vuint32_t BE_POR:1;
        } B;
    } DBRE;

    uint8_t MC_RGM_reserved4[716];

    union {
        vuint32_t R;
        struct {
            vuint32_t:7;
            vuint32_t F_VOR_FUNC:1;
            vuint32_t F_TSR_FUNC:1;
            vuint32_t:7;
            vuint32_t F_HSM_FUNC:1;
            vuint32_t:4;
            vuint32_t F_JTAG_FUNC:1;
            vuint32_t:3;
            vuint32_t F_FCCU_SOFT:1;
            vuint32_t F_FCCU_HARD:1;
            vuint32_t:1;
            vuint32_t F_SOFT_FUNC:1;
            vuint32_t F_ST_DONE:1;
            vuint32_t F_ESR1:1;
            vuint32_t F_ESR0:1;
        } B;
    } FES;

    uint8_t MC_RGM_reserved5[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t:7;
            vuint32_t D_VOR_FUNC:1;
            vuint32_t D_TSR_FUNC:1;
            vuint32_t:7;
            vuint32_t D_HSM_FUNC:1;
            vuint32_t:4;
            vuint32_t D_JTAG_FUNC:1;
            vuint32_t:3;
            vuint32_t D_FCCU_SOFT:1;
            vuint32_t D_FCCU_HARD:1;
            vuint32_t:1;
            vuint32_t D_SOFT_FUNC:1;
            vuint32_t D_ST_DONE:1;
            vuint32_t D_ESR1:1;
            vuint32_t D_ESR0:1;
        } B;
    } FERD;

    uint8_t MC_RGM_reserved6[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t:7;
            vuint32_t AR_VOR_FUNC:1;
            vuint32_t AR_TSR_FUNC:1;
            vuint32_t:12;
            vuint32_t AR_JTAG_FUNC:1;
            vuint32_t:6;
            vuint32_t AR_SOFT_FUNC:1;
            vuint32_t:1;
            vuint32_t AR_ESR1:1;
            vuint32_t AR_ESR0:1;
        } B;
    } FEAR;

    uint8_t MC_RGM_reserved7[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t:7;
            vuint32_t BE_VOR_FUNC:1;
            vuint32_t BE_TSR_FUNC:1;
            vuint32_t:7;
            vuint32_t BE_HSM_FUNC:1;
            vuint32_t:4;
            vuint32_t BE_JTAG_FUNC:1;
            vuint32_t:3;
            vuint32_t BE_FCCU_SOFT:1;
            vuint32_t BE_FCCU_HARD:1;
            vuint32_t:1;
            vuint32_t BE_SOFT_FUNC:1;
            vuint32_t BE_ST_DONE:1;
            vuint32_t BE_ESR1:1;
            vuint32_t BE_ESR0:1;
        } B;
    } FBRE;

    uint8_t MC_RGM_reserved8[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t:7;
            vuint32_t SS_VOR_FUNC:1;
            vuint32_t SS_TSR_FUNC:1;
            vuint32_t:7;
            vuint32_t SS_HSM_FUNC:1;
            vuint32_t:4;
            vuint32_t SS_JTAG_FUNC:1;
            vuint32_t:3;
            vuint32_t SS_FCCU_SOFT:1;
            vuint32_t SS_FCCU_HARD:1;
            vuint32_t:1;
            vuint32_t SS_SOFT_FUNC:1;
            vuint32_t SS_ST_DONE:1;
            vuint32_t SS_ESR1:1;
            vuint32_t SS_ESR0:1;
        } B;
    } FESS;

    uint8_t MC_RGM_reserved9[704];

    union {
        vuint16_t R;
        struct {
            vuint16_t RESERVED:12;
            vuint16_t FRET:4;
        } B;
    } FRET;

    uint8_t MC_RGM_reserved10[10];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t PIT_RTI_0_RST:1;
            vuint32_t:20;
            vuint32_t DIGRF_0_RST:1;
            vuint32_t:9;
        } B;
    } PRST0;

    union {
        vuint32_t R;
        struct {
            vuint32_t:3;
            vuint32_t ADCSD_A0_RST:1;
            vuint32_t ADCSD_A1_RST:1;
            vuint32_t ADCSD_A2_RST:1;
            vuint32_t:19;
            vuint32_t CRC_A0_RST:1;
            vuint32_t:1;
            vuint32_t DMA_CH_MUX0_RST:1;
            vuint32_t:4;
        } B;
    } PRST1;

    union {
        vuint32_t R;
        struct {
            vuint32_t:3;
            vuint32_t LINFLEX_A0_RST:1;
            vuint32_t LINFLEX_A1_RST:1;
            vuint32_t LINFLEX_A2_RST:1;
            vuint32_t:15;
            vuint32_t CAN_RAM_CTRL0_RST:1;
            vuint32_t:1;
            vuint32_t TTCAN_A0_RST:1;
            vuint32_t:1;
            vuint32_t MCAN_1_RST:1;
            vuint32_t MCAN_2_RST:1;
            vuint32_t MCAN_3_RST:1;
            vuint32_t:4;
        } B;
    } PRST2;

    union {
        vuint32_t R;
        struct {
            vuint32_t ADCSAR_A0_RST:1;
            vuint32_t ADCSAR_A1_RST:1;
            vuint32_t:13;
            vuint32_t ADCSARB_RST:1;
            vuint32_t PSI5_A0_RST:1;
            vuint32_t:3;
            vuint32_t FLEXRAY_A0_RST:1;
            vuint32_t:2;
            vuint32_t SENT_A0_RST:1;
            vuint32_t:2;
            vuint32_t IIC_A0_RST:1;
            vuint32_t:1;
            vuint32_t DSPI_A0_RST:1;
            vuint32_t DSPI_A1_RST:1;
            vuint32_t DSPI_A2_RST:1;
            vuint32_t:1;
        } B;
    } PRST3;

    union {
        vuint32_t R;
        struct {
            vuint32_t:32;
        } B;
    } PRST4;

    union {
        vuint32_t R;
        struct {
            vuint32_t:3;
            vuint32_t ADCSD_B0_RST:1;
            vuint32_t ADCSD_B1_RST:1;
            vuint32_t ADCSD_B2_RST:1;
            vuint32_t:19;
            vuint32_t CRC_B0_RST:1;
            vuint32_t:1;
            vuint32_t DMA_CH_MUX1_RST:1;
            vuint32_t:4;
        } B;
    } PRST5;

    union {
        vuint32_t R;
        struct {
            vuint32_t:3;
            vuint32_t LINFLEX_B0_RST:1;
            vuint32_t LINFLEX_B1_RST:1;
            vuint32_t:27;
        } B;
    } PRST6;

    union {
        vuint32_t R;
        struct {
            vuint32_t:2;
            vuint32_t ADCSAR_B0_RST:1;
            vuint32_t ADCSAR_B1_RST:1;
            vuint32_t ADCSAR_B2_RST:1;
            vuint32_t ADCSAR_B3_RST:1;
            vuint32_t ADCSAR_B4_RST:1;
            vuint32_t:9;
            vuint32_t PSI5_B0_RST:1;
            vuint32_t:6;
            vuint32_t SENT_B0_RST:1;
            vuint32_t:4;
            vuint32_t DSPI_B2_RST:1;
            vuint32_t DSPI_B1_RST:1;
            vuint32_t DSPI_B0_RST:1;
            vuint32_t:1;
        } B;
    } PRST7;
};
/**************************************************************************/
/*                   Module: MEMU                                         */
/**************************************************************************/
struct MEMU_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t SWR:1;
            vuint32_t:15;
        } B;
    } CTRL;

    union {
        vuint32_t R;
        struct {
            vuint32_t:11;
            vuint32_t PR_CE:1;
            vuint32_t PR_UCE:1;
            vuint32_t PR_CEO:1;
            vuint32_t PR_UCO:1;
            vuint32_t PR_EBO:1;
            vuint32_t:3;
            vuint32_t F_CE:1;
            vuint32_t F_UCE:1;
            vuint32_t F_CEO:1;
            vuint32_t F_UCO:1;
            vuint32_t F_EBO:1;
            vuint32_t:3;
            vuint32_t SR_CE:1;
            vuint32_t SR_UCE:1;
            vuint32_t SR_CEO:1;
            vuint32_t SR_UCO:1;
            vuint32_t SR_EBO:1;
        } B;
    } ERR_FLAG;

    uint8_t MEMU_reserved1[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:11;
            vuint32_t FR_PR_CE:1;
            vuint32_t FR_PR_UCE:1;
            vuint32_t FR_PR_CEO:1;
            vuint32_t FR_PR_UCO:1;
            vuint32_t FR_PR_EBO:1;
            vuint32_t:3;
            vuint32_t FR_F_CE:1;
            vuint32_t FR_F_UCE:1;
            vuint32_t FR_F_CEO:1;
            vuint32_t FR_F_UCO:1;
            vuint32_t FR_F_EBO:1;
            vuint32_t:3;
            vuint32_t FR_SR_CE:1;
            vuint32_t FR_SR_UCE:1;
            vuint32_t FR_SR_CEO:1;
            vuint32_t FR_SR_UCO:1;
            vuint32_t FR_SR_EBO:1;
        } B;
    } DEBUG;

    uint8_t MEMU_reserved2[4080];

    struct {
        union {
            vuint32_t R;
            struct {
                vuint32_t VLD:1;
                vuint32_t:23;
                vuint32_t BAD_BIT:8;
            } B;
        } SYS_RAM_CERR_STS;

        union {
            vuint32_t R;
            struct {
                vuint32_t ERR_ADD:32;
            } B;
        } SYS_RAM_CERR_ADDR;
    } CHANNEL0[10];

    union {
        vuint32_t R;
        struct {
            vuint32_t VLD:1;
            vuint32_t:31;
        } B;
    } SYS_RAM_UNCERR_STS;

    union {
        vuint32_t R;
        struct {
            vuint32_t ERR_ADD:32;
        } B;
    } SYS_RAM_UNCERR_ADDR;

    union {
        vuint32_t R;
        struct {
            vuint32_t OFLW:32;
        } B;
    } SYS_RAM_OFLW[3];

    uint8_t MEMU_reserved3[3996];

    struct {
        union {
            vuint32_t R;
            struct {
                vuint32_t VLD:1;
                vuint32_t:23;
                vuint32_t BAD_BIT:8;
            } B;
        } PERIPH_RAM_CERR_STS;

        union {
            vuint32_t R;
            struct {
                vuint32_t ERR_ADD:32;
            } B;
        } PERIPH_RAM_CERR_ADDR;
    } CHANNEL1[2];

    union {
        vuint32_t R;
        struct {
            vuint32_t VLD:1;
            vuint32_t:31;
        } B;
    } PERIPH_RAM_UNCERR_STS;

    union {
        vuint32_t R;
        struct {
            vuint32_t ERR_ADD:32;
        } B;
    } PERIPH_RAM_UNCERR_ADDR;

    union {
        vuint32_t R;
        struct {
            vuint32_t OFLW:32;
        } B;
    } PERIPH_RAM_OFLW;

    uint8_t MEMU_reserved4[4068];

    struct {
        union {
            vuint32_t R;
            struct {
                vuint32_t VLD:1;
                vuint32_t:23;
                vuint32_t BAD_BIT:8;
            } B;
        } FLASH_CERR_STS;

        union {
            vuint32_t R;
            struct {
                vuint32_t ERR_ADD:32;
            } B;
        } FLASH_CERR_ADDR;
    } CHANNEL2[20];

    union {
        vuint32_t R;
        struct {
            vuint32_t VLD:1;
            vuint32_t:31;
        } B;
    } FLASH_UNCERR_STS;

    union {
        vuint32_t R;
        struct {
            vuint32_t ERR_ADD:32;
        } B;
    } FLASH_UNCERR_ADDR;

    uint8_t MEMU_reserved5[152];

    union {
        vuint32_t R;
        struct {
            vuint32_t OFLW:32;
        } B;
    } FLASH_OFLW;
};
/**************************************************************************/
/*                   Module: MC_PCU                                       */
/**************************************************************************/
struct MC_PCU_tag {

    uint8_t MC_PCU_reserved[64];

    union {
        vuint32_t R;
        struct {
            vuint32_t:31;
            vuint32_t PD0:1;
        } B;
    } PSTAT;
};
/**************************************************************************/
/*                              Module: PASS                              */
/**************************************************************************/
struct PASS_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t CNS:1;
            vuint32_t JUN:1;
            vuint32_t:22;
            vuint32_t LIFE:8;
        } B;
    } LCSTAT;

    uint8_t PASS_reserved1[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:30;
            vuint32_t GRP:2;
        } B;
    } CHSEL;

    uint8_t PASS_reserved2[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:28;
            vuint32_t CMST:4;
        } B;
    } CSTAT;

    uint8_t PASS_reserved3[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t PW32:32;
        } B;
    } CIN[8];

    uint8_t PASS_reserved4[144];

    union {
        vuint32_t R;
        struct {
            vuint32_t CJE:1;
            vuint32_t:31;
        } B;
    } CJE;

    uint8_t PASS_reserved5[44];

    struct {
        union {
            vuint32_t R;
            struct {
                vuint32_t TSLOCK:1;
                vuint32_t ATSL:1;
                vuint32_t LOWLOCK:14;
                vuint32_t MIDLOCK:16;
            } B;
        } LOCK0_PG;

        union {
            vuint32_t R;
            struct {
                vuint32_t:16;
                vuint32_t HIGHLOCK:16;
            } B;
        } LOCK1_PG;

        union {
            vuint32_t R;
            struct {
                vuint32_t L256L_CK_L:32;
            } B;
        } LOCK2_PG;

        union {
            vuint32_t R;
            struct {
                vuint32_t PGL:1;
                vuint32_t DBL:1;
                vuint32_t MO:1;
                vuint32_t:1;
                vuint32_t MSTR:4;
                vuint32_t:3;
                vuint32_t RL4:1;
                vuint32_t RL3:1;
                vuint32_t RL2:1;
                vuint32_t RL1:1;
                vuint32_t RL0:1;
                vuint32_t U256L_CK_U:16;
            } B;
        } LOCK3_PG;
    } TIMER[4];
};
/**************************************************************************/
/*                   Module: PBRIDGE                                      */
/**************************************************************************/
struct PBRIDGE_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t MPROTa:4;
            vuint32_t MPROTb:4;
            vuint32_t MPROTc:4;
            vuint32_t MPROTd:4;
            vuint32_t MPROTe:4;
            vuint32_t MPROTf:4;
            vuint32_t MPROTg:4;
            vuint32_t MPROTh:4;
        } B;
    } MPRA;

    union {
        vuint32_t R;
        struct {
            vuint32_t MPROTa:4;
            vuint32_t MPROTb:4;
            vuint32_t MPROTc:4;
            vuint32_t MPROTd:4;
            vuint32_t MPROTe:4;
            vuint32_t MPROTf:4;
            vuint32_t MPROTg:4;
            vuint32_t MPROTh:4;
        } B;
    } MPRB;

    uint8_t PBRIDGE_reserved1[248];

    union {
        vuint32_t R;
        struct {
            vuint32_t PACRa:4;
            vuint32_t PACRb:4;
            vuint32_t PACRc:4;
            vuint32_t PACRd:4;
            vuint32_t PACRe:4;
            vuint32_t PACRf:4;
            vuint32_t PACRg:4;
            vuint32_t PACRh:4;
        } B;
    } PACRA;

    union {
        vuint32_t R;
        struct {
            vuint32_t PACRa:4;
            vuint32_t PACRb:4;
            vuint32_t PACRc:4;
            vuint32_t PACRd:4;
            vuint32_t PACRe:4;
            vuint32_t PACRf:4;
            vuint32_t PACRg:4;
            vuint32_t PACRh:4;
        } B;
    } PACRB;

    union {
        vuint32_t R;
        struct {
            vuint32_t PACRa:4;
            vuint32_t PACRb:4;
            vuint32_t PACRc:4;
            vuint32_t PACRd:4;
            vuint32_t PACRe:4;
            vuint32_t PACRf:4;
            vuint32_t PACRg:4;
            vuint32_t PACRh:4;
        } B;
    } PACRC;

    union {
        vuint32_t R;
        struct {
            vuint32_t PACRa:4;
            vuint32_t PACRb:4;
            vuint32_t PACRc:4;
            vuint32_t PACRd:4;
            vuint32_t PACRe:4;
            vuint32_t PACRf:4;
            vuint32_t PACRg:4;
            vuint32_t PACRh:4;
        } B;
    } PACRD;

    union {
        vuint32_t R;
        struct {
            vuint32_t PACRa:4;
            vuint32_t PACRb:4;
            vuint32_t PACRc:4;
            vuint32_t PACRd:4;
            vuint32_t PACRe:4;
            vuint32_t PACRf:4;
            vuint32_t PACRg:4;
            vuint32_t PACRh:4;
        } B;
    } PACRE;

    union {
        vuint32_t R;
        struct {
            vuint32_t PACRa:4;
            vuint32_t PACRb:4;
            vuint32_t PACRc:4;
            vuint32_t PACRd:4;
            vuint32_t PACRe:4;
            vuint32_t PACRf:4;
            vuint32_t PACRg:4;
            vuint32_t PACRh:4;
        } B;
    } PACRF;

    union {
        vuint32_t R;
        struct {
            vuint32_t PACRa:4;
            vuint32_t PACRb:4;
            vuint32_t PACRc:4;
            vuint32_t PACRd:4;
            vuint32_t PACRe:4;
            vuint32_t PACRf:4;
            vuint32_t PACRg:4;
            vuint32_t PACRh:4;
        } B;
    } PACRG;

    union {
        vuint32_t R;
        struct {
            vuint32_t PACRa:4;
            vuint32_t PACRb:4;
            vuint32_t PACRc:4;
            vuint32_t PACRd:4;
            vuint32_t PACRe:4;
            vuint32_t PACRf:4;
            vuint32_t PACRg:4;
            vuint32_t PACRh:4;
        } B;
    } PACRH;

    uint8_t PBRIDGE_reserved2[32];

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRA;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRB;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRC;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRD;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRE;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRF;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRG;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRH;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRI;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRJ;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRK;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRL;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRM;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRN;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRO;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRP;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRQ;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRR;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRS;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRT;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRU;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRV;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRW;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRX;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRY;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRZ;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRAA;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRAB;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRAC;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRAD;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRAE;

    union {
        vuint32_t R;
        struct {
            vuint32_t OPACRa:4;
            vuint32_t OPACRb:4;
            vuint32_t OPACRc:4;
            vuint32_t OPACRd:4;
            vuint32_t OPACRe:4;
            vuint32_t OPACRf:4;
            vuint32_t OPACRg:4;
            vuint32_t OPACRh:4;
        } B;
    } OPACRAF;
};
/**************************************************************************/
/*                   Module: PFLASH                                       */
/**************************************************************************/
struct PFLASH_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t P0_M15PFE:1;
            vuint32_t P0_M14PFE:1;
            vuint32_t P0_M13PFE:1;
            vuint32_t P0_M12PFE:1;
            vuint32_t P0_M11PFE:1;
            vuint32_t P0_M10PFE:1;
            vuint32_t P0_M9PFE:1;
            vuint32_t P0_M8PFE:1;
            vuint32_t P0_M7PFE:1;
            vuint32_t P0_M6PFE:1;
            vuint32_t P0_M5PFE:1;
            vuint32_t P0_M4PFE:1;
            vuint32_t P0_M3PFE:1;
            vuint32_t P0_M2PFE:1;
            vuint32_t P0_M1PFE:1;
            vuint32_t P0_M0PFE:1;
            vuint32_t:3;
            vuint32_t RWSC:5;
            vuint32_t:1;
            vuint32_t P0_DPFEN:1;
            vuint32_t:1;
            vuint32_t P0_IPFEN:1;
            vuint32_t:1;
            vuint32_t P0_PFLIM:2;
            vuint32_t P0_BFEN:1;
        } B;
    } PFCR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t P1_M15PFE:1;
            vuint32_t P1_M14PFE:1;
            vuint32_t P1_M13PFE:1;
            vuint32_t P1_M12PFE:1;
            vuint32_t P1_M11PFE:1;
            vuint32_t P1_M10PFE:1;
            vuint32_t P1_M9PFE:1;
            vuint32_t P1_M8PFE:1;
            vuint32_t P1_M7PFE:1;
            vuint32_t P1_M6PFE:1;
            vuint32_t P1_M5PFE:1;
            vuint32_t P1_M4PFE:1;
            vuint32_t P1_M3PFE:1;
            vuint32_t P1_M2PFE:1;
            vuint32_t P1_M1PFE:1;
            vuint32_t P1_M0PFE:1;
            vuint32_t:9;
            vuint32_t P1_DPFEN:1;
            vuint32_t:1;
            vuint32_t P1_IPFEN:1;
            vuint32_t:1;
            vuint32_t P1_PFLIM:2;
            vuint32_t P1_BFEN:1;
        } B;
    } PFCR2;

    union {
        vuint32_t R;
        struct {
            vuint32_t P0_WCFG:2;
            vuint32_t P1_WCFG:2;
            vuint32_t:11;
            vuint32_t BAF_DIS:1;
            vuint32_t ARBM:2;
            vuint32_t:14;
        } B;
    } PFCR3;

    union {
        vuint32_t R;
        struct {
            vuint32_t M0AP:2;
            vuint32_t M1AP:2;
            vuint32_t M2AP:2;
            vuint32_t M3AP:2;
            vuint32_t M4AP:2;
            vuint32_t M5AP:2;
            vuint32_t M6AP:2;
            vuint32_t M7AP:2;
            vuint32_t M8AP:2;
            vuint32_t M9AP:2;
            vuint32_t M10AP:2;
            vuint32_t M11AP:2;
            vuint32_t M12AP:2;
            vuint32_t M13AP:2;
            vuint32_t M14AP:2;
            vuint32_t M15AP:2;
        } B;
    } PFAPR;

};
/**************************************************************************/
/*                   Module: PIT                                          */
/**************************************************************************/
struct PIT_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:30;
            vuint32_t MDIS:1;
            vuint32_t FRZ:1;
        } B;
    } MCR;

    uint8_t PIT_reserved1[220];

    union {
        vuint32_t R;
        struct {
            vuint32_t LTH:32;
        } B;
    } LTMR64H;

    union {
        vuint32_t R;
        struct {
            vuint32_t LTL:32;
        } B;
    } LTMR64L;

    uint8_t PIT_reserved2[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t TSV:32;
        } B;
    } RTI_LDVAL;

    union {
        vuint32_t R;
        struct {
            vuint32_t TVL:32;
        } B;
    } RTI_CVAL;

    union {
        vuint32_t R;
        struct {
            vuint32_t:29;
            vuint32_t CHN:1;
            vuint32_t TIE:1;
            vuint32_t TEN:1;
        } B;
    } RTI_TCTRL;

    union {
        vuint32_t R;
        struct {
            vuint32_t:31;
            vuint32_t TIF:1;
        } B;
    } RTI_TFLG;

    struct {
        union {
            vuint32_t R;
            struct {
                vuint32_t TSV:32;
            } B;
        } LDVAL;

        union {
            vuint32_t R;
            struct {
                vuint32_t TVL:32;
            } B;
        } CVAL;

        union {
            vuint32_t R;
            struct {
                vuint32_t:29;
                vuint32_t CHN:1;
                vuint32_t TIE:1;
                vuint32_t TEN:1;
            } B;
        } TCTRL;

        union {
            vuint32_t R;
            struct {
                vuint32_t:31;
                vuint32_t TIF:1;
            } B;
        } TFLG;
    } TIMER[8];
};
/**************************************************************************/
/*                   Module: PLLDIG                                       */
/**************************************************************************/
struct PLLDIG_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:22;
            vuint32_t CLKCFG:2;
            vuint32_t EXPDIE:1;
            vuint32_t:3;
            vuint32_t LOLIE:1;
            vuint32_t LOLRE:1;
            vuint32_t:2;
        } B;
    } PLL0CR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t EXTPDF:1;
            vuint32_t:3;
            vuint32_t LOLF:1;
            vuint32_t LOCK:1;
            vuint32_t:2;
        } B;
    } PLL0SR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t RFDPHI1:4;
            vuint32_t:5;
            vuint32_t RFDPHI:6;
            vuint32_t:1;
            vuint32_t PREDIV:3;
            vuint32_t:5;
            vuint32_t MFD:7;
        } B;
    } PLL0DV;

    uint8_t PLLDIG_reserved1[20];

    union {
        vuint32_t R;
        struct {
            vuint32_t:22;
            vuint32_t CLKCFG:2;
            vuint32_t EXPDIE:1;
            vuint32_t:3;
            vuint32_t LOLIE:1;
            vuint32_t LOLRE:1;
            vuint32_t:2;
        } B;
    } PLL1CR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t EXTPDF:1;
            vuint32_t:2;
            vuint32_t:1;
            vuint32_t LOLF:1;
            vuint32_t LOCK:1;
            vuint32_t:2;
        } B;
    } PLL1SR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:10;
            vuint32_t RFDPHI:6;
            vuint32_t:9;
            vuint32_t MFD:7;
        } B;
    } PLL1DV;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t MODEN:1;
            vuint32_t MODSEL:1;
            vuint32_t MODPRD:13;
            vuint32_t:1;
            vuint32_t INCSTP:15;
        } B;
    } PLL1FM;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t FDEN:1;
            vuint32_t:12;
            vuint32_t DTHDIS:2;
            vuint32_t:4;
            vuint32_t FRCDIV:12;
        } B;
    } PLL1FD;
};
/**************************************************************************/
/*                   Module: PMCDIG                                       */
/**************************************************************************/
struct PMCDIG_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:17;
            vuint32_t VD15:1;
            vuint32_t VD14:1;
            vuint32_t VD13:1;
            vuint32_t VD12:1;
            vuint32_t:1;
            vuint32_t VD10:1;
            vuint32_t VD9:1;
            vuint32_t:1;
            vuint32_t VD7:1;
            vuint32_t:1;
            vuint32_t VD4:1;
            vuint32_t VD3:1;
            vuint32_t:3;
        } B;
    } GR_S;

    union {
        vuint32_t R;
        struct {
            vuint32_t:5;
            vuint32_t TS3:1;
            vuint32_t TS2:1;
            vuint32_t TS0:1;
            vuint32_t VD15_A:1;
            vuint32_t VD15_C:1;
            vuint32_t VD14_A:1;
            vuint32_t VD14_IM:1;
            vuint32_t VD13_A:1;
            vuint32_t VD13_IM:1;
            vuint32_t VD12_F:1;
            vuint32_t VD10_F:1;
            vuint32_t VD9_O:1;
            vuint32_t VD9_IF:1;
            vuint32_t VD9_IJ:1;
            vuint32_t VD9_IM:1;
            vuint32_t VD9_F:1;
            vuint32_t VD9_C:1;
            vuint32_t:1;
            vuint32_t VD7_F:1;
            vuint32_t VD7_C:1;
            vuint32_t VD4_C:1;
            vuint32_t VD3_P:1;
            vuint32_t VD3_F:1;
            vuint32_t VD3_C:1;
            vuint32_t VD2_F:1;
            vuint32_t VD2_C:1;
            vuint32_t VD1:1;
        } B;
    } GR_P;

    union {
        vuint32_t R;
        struct {
            vuint32_t IE_EN:1;
            vuint32_t:4;
            vuint32_t TS3IE:1;
            vuint32_t TS2IE:1;
            vuint32_t TS0IE:1;
            vuint32_t VD15IE_A:1;
            vuint32_t VD15IE_C:1;
            vuint32_t VD14IE_A:1;
            vuint32_t VD14IE_IM:1;
            vuint32_t VD13IE_A:1;
            vuint32_t VD13IE_IM:1;
            vuint32_t VD12IE_F:1;
            vuint32_t VD10IE_F:1;
            vuint32_t VD9IE_O:1;
            vuint32_t VD9IE_IF:1;
            vuint32_t VD9IE_IJ:1;
            vuint32_t VD9IE_IM:1;
            vuint32_t VD9IE_F:1;
            vuint32_t VD9IE_C:1;
            vuint32_t:1;
            vuint32_t VD7IE_F:1;
            vuint32_t VD7IE_C:1;
            vuint32_t VD4IE_C:1;
            vuint32_t VD3IE_P:1;
            vuint32_t VD3IE_F:1;
            vuint32_t VD3IE_C:1;
            vuint32_t:3;
        } B;
    } IE_P;

    uint8_t PMCDIG_reserved1[20];

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t LVD2_F:1;
            vuint32_t:5;
            vuint32_t LVD2_C:1;
        } B;
    } EPR_VD2;

    uint8_t PMCDIG_reserved2[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t LVD3_P:1;
            vuint32_t LVD3_F:1;
            vuint32_t:5;
            vuint32_t LVD3_C:1;
        } B;
    } EPR_VD3;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t LVD3_P:1;
            vuint32_t LVD3_F:1;
            vuint32_t:5;
            vuint32_t LVD3_C:1;
        } B;
    } REE_VD3;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t LVD3_P:1;
            vuint32_t LVD3_F:1;
            vuint32_t:5;
            vuint32_t LVD3_C:1;
        } B;
    } RES_VD3;

    uint8_t PMCDIG_reserved3[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:31;
            vuint32_t LVD4_C:1;
        } B;
    } EPR_VD4;

    union {
        vuint32_t R;
        struct {
            vuint32_t:31;
            vuint32_t LVD4_C:1;
        } B;
    } REE_VD4;

    union {
        vuint32_t R;
        struct {
            vuint32_t:31;
            vuint32_t LVD4_C:1;
        } B;
    } RES_VD4;

    uint8_t PMCDIG_reserved4[36];

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t HVD7_F:1;
            vuint32_t:5;
            vuint32_t HVD7_C:1;
        } B;
    } EPR_VD7;

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t HVD7_F:1;
            vuint32_t:5;
            vuint32_t HVD7_C:1;
        } B;
    } REE_VD7;

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t HVD7_F:1;
            vuint32_t:5;
            vuint32_t HVD7_C:1;
        } B;
    } RES_VD7;

    uint8_t PMCDIG_reserved5[20];

    union {
        vuint32_t R;
        struct {
            vuint32_t:17;
            vuint32_t LVD9_O:1;
            vuint32_t:3;
            vuint32_t LVD9_IF:1;
            vuint32_t LVD9_IJ:1;
            vuint32_t LVD9_IM:1;
            vuint32_t:1;
            vuint32_t LVD9_F:1;
            vuint32_t:5;
            vuint32_t LVD9_C:1;
        } B;
    } EPR_VD9;

    union {
        vuint32_t R;
        struct {
            vuint32_t:17;
            vuint32_t LVD9_O:1;
            vuint32_t:3;
            vuint32_t LVD9_IF:1;
            vuint32_t LVD9_IJ:1;
            vuint32_t LVD9_IM:1;
            vuint32_t:1;
            vuint32_t LVD9_F:1;
            vuint32_t:5;
            vuint32_t LVD9_C:1;
        } B;
    } REE_VD9;

    union {
        vuint32_t R;
        struct {
            vuint32_t:17;
            vuint32_t LVD9_O:1;
            vuint32_t:3;
            vuint32_t LVD9_IF:1;
            vuint32_t LVD9_IJ:1;
            vuint32_t LVD9_IM:1;
            vuint32_t:1;
            vuint32_t LVD9_F:1;
            vuint32_t:5;
            vuint32_t LVD9_C:1;
        } B;
    } RES_VD9;

    uint8_t PMCDIG_reserved6[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t LVD10_F:1;
            vuint32_t:6;
        } B;
    } EPR_VD10;

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t LVD10_F:1;
            vuint32_t:6;
        } B;
    } REE_VD10;

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t LVD10_F:1;
            vuint32_t:6;
        } B;
    } RES_VD10;

    uint8_t PMCDIG_reserved7[20];

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t HVD12_F:1;
            vuint32_t:6;
        } B;
    } EPR_VD12;

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t HVD12_F:1;
            vuint32_t:6;
        } B;
    } REE_VD12;

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t HVD12_F:1;
            vuint32_t:6;
        } B;
    } RES_VD12;

    uint8_t PMCDIG_reserved8[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t LVD13_A:1;
            vuint32_t:6;
            vuint32_t LVD13_IM:1;
            vuint32_t:8;
        } B;
    } EPR_VD13;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t LVD13_A:1;
            vuint32_t:6;
            vuint32_t LVD13_IM:1;
            vuint32_t:8;
        } B;
    } REE_VD13;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t LVD13_A:1;
            vuint32_t:6;
            vuint32_t LVD13_IM:1;
            vuint32_t:8;
        } B;
    } RES_VD13;

    uint8_t PMCDIG_reserved9[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t LVD14_A:1;
            vuint32_t:6;
            vuint32_t LVD14_IM:1;
            vuint32_t:8;
        } B;
    } EPR_VD14;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t LVD14_A:1;
            vuint32_t:6;
            vuint32_t LVD14_IM:1;
            vuint32_t:8;
        } B;
    } REE_VD14;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t LVD14_A:1;
            vuint32_t:6;
            vuint32_t LVD14_IM:1;
            vuint32_t:8;
        } B;
    } RES_VD14;

    uint8_t PMCDIG_reserved10[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t HVD15_A:1;
            vuint32_t:14;
            vuint32_t HVD15_C:1;
        } B;
    } EPR_VD15;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t HVD15_A:1;
            vuint32_t:14;
            vuint32_t HVD15_C:1;
        } B;
    } REE_VD15;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t HVD15_A:1;
            vuint32_t:14;
            vuint32_t HVD15_C:1;
        } B;
    } RES_VD15;

    uint8_t PMCDIG_reserved11[20];

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD3_C:6;
        } B;
    } TRIM_VD3_C;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD3_F:6;
        } B;
    } TRIM_VD3_F;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD3_P:6;
        } B;
    } TRIM_VD3_P;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD4_C:6;
        } B;
    } TRIM_VD4_C;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD7_C:6;
        } B;
    } TRIM_VD7_C;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD7_F:6;
        } B;
    } TRIM_VD7_F;

    uint8_t PMCDIG_reserved12[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD9_C:6;
        } B;
    } TRIM_VD9_C;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD9_F:6;
        } B;
    } TRIM_VD9_F;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD9_IM:6;
        } B;
    } TRIM_VD9_IM;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD9_IJ:6;
        } B;
    } TRIM_VD9_IJ;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD9_IF:6;
        } B;
    } TRIM_VD9_IF;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD9_O:6;
        } B;
    } TRIM_VD9_O;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD10_F:6;
        } B;
    } TRIM_VD10_F;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD12_F:6;
        } B;
    } TRIM_VD12_F;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD13_IM:6;
        } B;
    } TRIM_VD13_IM;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD13_A:6;
        } B;
    } TRIM_VD13_A;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD14_IM:6;
        } B;
    } TRIM_VD14_IM;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD14_A:6;
        } B;
    } TRIM_VD14_A;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD15_C:6;
        } B;
    } TRIM_VD15_C;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_VD15_A:6;
        } B;
    } TRIM_VD15_A;

    uint8_t PMCDIG_reserved13[156];

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t BG_CTRIM_C_EN:1;
            vuint32_t BG_CTRIM_C:6;
        } B;
    } BG_CTRIM_C;

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t BG_CTRIM_R_EN:1;
            vuint32_t BG_CTRIM_R:6;
        } B;
    } BG_CTRIM_R;

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t BG_CTRIM_H_EN:1;
            vuint32_t BG_CTRIM_H:6;
        } B;
    } BG_CTRIM_H;

    uint8_t PMCDIG_reserved14[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t BG_ATRIM_C_EN:1;
            vuint32_t BG_ATRIM_C:6;
        } B;
    } BG_ATRIM_C;

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t BG_ATRIM_R_EN:1;
            vuint32_t BG_ATRIM_R:6;
        } B;
    } BG_ATRIM_R;

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t BG_ATRIM_H_EN:1;
            vuint32_t BG_ATRIM_H:6;
        } B;
    } BG_ATRIM_H;

    uint8_t PMCDIG_reserved15[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t VREG3P3_TRIM:6;
        } B;
    } VREG3P3_TRIM;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t VREG1P2_TRIM:6;
        } B;
    } VREG1P2_TRIM;

    uint8_t PMCDIG_reserved16[216];

    union {
        vuint32_t R;
        struct {
            vuint32_t:28;
            vuint32_t TEMP_3:1;
            vuint32_t TEMP_2:1;
            vuint32_t:1;
            vuint32_t TEMP_0:1;
        } B;
    } EPR_TD;

    union {
        vuint32_t R;
        struct {
            vuint32_t:28;
            vuint32_t TEMP_2:1;
            vuint32_t TEMP_1:1;
            vuint32_t:1;
            vuint32_t TEMP_0:1;
        } B;
    } REE_TD;

    union {
        vuint32_t R;
        struct {
            vuint32_t:28;
            vuint32_t TEMP_2:1;
            vuint32_t TEMP_1:1;
            vuint32_t:1;
            vuint32_t TEMP_0:1;
        } B;
    } RES_TD;

    union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t TRIM_ADJ:5;
            vuint32_t DOUT_EN:1;
            vuint32_t AOUT_EN:1;
        } B;
    } CTL_TD;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_TEMP0:6;
        } B;
    } NT_TD0;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t TRIM_TEMP2:6;
        } B;
    } NT_TD2;

    uint8_t PMCDIG_reserved17[24];

    union {
        vuint32_t R;
        struct {
            vuint32_t:18;
            vuint32_t BG_SLCT:1;
            vuint32_t VBG_CTRIM:4;
            vuint32_t VREF_ATRIM:5;
            vuint32_t LVI_TRIM:4;
        } B;
    } ADC_BGT;

    uint8_t PMCDIG_reserved18[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t VD_UTST:6;
        } B;
    } VD_UTST;
};
/**************************************************************************/
/*                   Module: PRAM                                         */
/**************************************************************************/
struct PRAM_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:31;
            vuint32_t FT_DIS:1;
        } B;
    } PRCR1;
};
/**************************************************************************/
/*                   Module: PSI5                                         */
/**************************************************************************/
struct PSI5_tag {

    union {
        vuint32_t R;
        struct {
            vuint32_t:14;
            vuint32_t CTC_GED:1;
            vuint32_t GLOBAL_DISABLE_REQ:1;
            vuint32_t:16;
        } B;
    } GCR;

    struct {
        uint8_t PSI5_reserved1[4];
        union {
            vuint32_t R;
            struct {
                vuint32_t CTC_GED_SEL:1;
                vuint32_t CTC_ED:1;
                vuint32_t:1;
                vuint32_t MEM_DEPTH:5;
                vuint32_t:3;
                vuint32_t ERROR_SELECT:5;
                vuint32_t:6;
                vuint32_t DEBUG_FREEZE_CTRL:1;
                vuint32_t SP_TS_CLK_SEL:1;
                vuint32_t:2;
                vuint32_t FAST_CLK_SENT:1;
                vuint32_t FAST_CLK_PSI5:1;
                vuint32_t BIT_RATE:1;
                vuint32_t MODE:1;
                vuint32_t PSI5_CH_CONFIG:1;
                vuint32_t PSI5_CH_EN:1;
            } B;
        } PCCR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:3;
                vuint32_t DMA_PM_DS_WM:5;
                vuint32_t:5;
                vuint32_t IE_DMA_TF_SF:1;
                vuint32_t IE_DMA_TF_PM_DS:1;
                vuint32_t:5;
                vuint32_t IE_DMA_PM_DS_FIFO_FULL:1;
                vuint32_t IE_DMA_SFUF:1;
                vuint32_t:1;
                vuint32_t IE_DMA_PM_DS_UF:1;
                vuint32_t:5;
                vuint32_t DMA_EN_SF:1;
                vuint32_t DMA_PM_DS_CONFIG:2;
            } B;
        } DCR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:13;
                vuint32_t IS_DMA_TF_SF:1;
                vuint32_t:1;
                vuint32_t IS_DMA_TF_PM_DS:1;
                vuint32_t:4;
                vuint32_t IS_DMA_PM_DS_FIFO_FULL:1;
                vuint32_t IS_DMA_SFUF:1;
                vuint32_t:1;
                vuint32_t IS_DMA_PM_DS_UF:1;
                vuint32_t:8;
            } B;
        } DSR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:2;
                vuint32_t IE_CESM:6;
                vuint32_t IE_STS:1;
                vuint32_t IE_DTS:1;
                vuint32_t IE_DSROW:1;
                vuint32_t IE_BROW:1;
                vuint32_t IE_PROW:1;
                vuint32_t IE_DSRR:1;
                vuint32_t IE_BRR:1;
                vuint32_t IE_PRR:1;
                vuint32_t:2;
                vuint32_t IE_OWSM:6;
                vuint32_t:2;
                vuint32_t IE_NVSM:6;
            } B;
        } GICR;

        union {
            vuint32_t R;
            struct {
                vuint32_t IE_ND:32;
            } B;
        } NDICR;

        union {
            vuint32_t R;
            struct {
                vuint32_t IE_OW:32;
            } B;
        } OWICR;

        union {
            vuint32_t R;
            struct {
                vuint32_t IE_ERROR:32;
            } B;
        } EICR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:2;
                vuint32_t IS_CESM:6;
                vuint32_t IS_STS:1;
                vuint32_t IS_DTS:1;
                vuint32_t IS_DSROW:1;
                vuint32_t IS_BROW:1;
                vuint32_t IS_PROW:1;
                vuint32_t DSR_RDY:1;
                vuint32_t DBR_RDY:1;
                vuint32_t DPR_RDY:1;
                vuint32_t:2;
                vuint32_t IS_OWSM:6;
                vuint32_t:2;
                vuint32_t IS_NVSM:6;
            } B;
        } GISR;

        union {
            vuint32_t R;
            struct {
                vuint32_t PSI5_RXDATA:32;
            } B;
        } DPMR;

        union {
            vuint32_t R;
            struct {
                vuint32_t SENT_RXDATA:32;
            } B;
        } DSFR;

        union {
            vuint32_t R;
            struct {
                vuint32_t DDS:32;
            } B;
        } DDSR;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRRL;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRRH;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL0;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH0;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL1;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH1;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL2;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH2;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL3;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH3;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL4;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH4;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL5;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH5;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL6;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH6;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL7;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH7;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL8;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH8;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL9;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH9;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL10;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH10;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL11;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH11;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL12;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH12;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL13;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH13;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL14;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH14;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL15;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH15;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL16;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH16;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL17;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH17;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL18;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH18;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL19;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH19;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL20;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH20;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL21;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH21;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL22;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH22;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL23;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH23;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL24;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH24;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL25;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH25;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL26;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH26;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL27;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH27;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL28;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH28;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL29;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH29;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL30;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH30;

        union {
            vuint32_t R;
            struct {
                vuint32_t DATA_REGION:28;
                vuint32_t CRC:3;
                vuint32_t C:1;
            } B;
        } PMRL31;

        union {
            vuint32_t R;
            struct {
                vuint32_t OW:1;
                vuint32_t F:1;
                vuint32_t EM:1;
                vuint32_t E:1;
                vuint32_t T:1;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t TIME_STAMP_VALUE:24;
            } B;
        } PMRH31;

        union {
            vuint32_t R;
            struct {
                vuint32_t SLOT_NO:3;
                vuint32_t CER:1;
                vuint32_t OW:1;
                vuint32_t CRC:6;
                vuint32_t C:1;
                vuint32_t ID:8;
                vuint32_t DATA:12;
            } B;
        } SFR1;

        union {
            vuint32_t R;
            struct {
                vuint32_t SLOT_NO:3;
                vuint32_t CER:1;
                vuint32_t OW:1;
                vuint32_t CRC:6;
                vuint32_t C:1;
                vuint32_t ID:8;
                vuint32_t DATA:12;
            } B;
        } SFR2;

        union {
            vuint32_t R;
            struct {
                vuint32_t SLOT_NO:3;
                vuint32_t CER:1;
                vuint32_t OW:1;
                vuint32_t CRC:6;
                vuint32_t C:1;
                vuint32_t ID:8;
                vuint32_t DATA:12;
            } B;
        } SFR3;

        union {
            vuint32_t R;
            struct {
                vuint32_t SLOT_NO:3;
                vuint32_t CER:1;
                vuint32_t OW:1;
                vuint32_t CRC:6;
                vuint32_t C:1;
                vuint32_t ID:8;
                vuint32_t DATA:12;
            } B;
        } SFR4;

        union {
            vuint32_t R;
            struct {
                vuint32_t SLOT_NO:3;
                vuint32_t CER:1;
                vuint32_t OW:1;
                vuint32_t CRC:6;
                vuint32_t C:1;
                vuint32_t ID:8;
                vuint32_t DATA:12;
            } B;
        } SFR5;

        union {
            vuint32_t R;
            struct {
                vuint32_t SLOT_NO:3;
                vuint32_t CER:1;
                vuint32_t OW:1;
                vuint32_t CRC:6;
                vuint32_t C:1;
                vuint32_t ID:8;
                vuint32_t DATA:12;
            } B;
        } SFR6;

        union {
            vuint32_t R;
            struct {
                vuint32_t NDS:32;
            } B;
        } NDSR;

        union {
            vuint32_t R;
            struct {
                vuint32_t OWS:32;
            } B;
        } OWSR;

        union {
            vuint32_t R;
            struct {
                vuint32_t ERROR:32;
            } B;
        } EISR;

        union {
            vuint32_t R;
            struct {
                vuint32_t SNDS:32;
            } B;
        } SNDSR;

        union {
            vuint32_t R;
            struct {
                vuint32_t SOWS:32;
            } B;
        } SOWSR;

        union {
            vuint32_t R;
            struct {
                vuint32_t SERROR:32;
            } B;
        } SEISR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:2;
                vuint32_t SCESM:6;
                vuint32_t:10;
                vuint32_t SOWSM:6;
                vuint32_t:2;
                vuint32_t SNVSM:6;
            } B;
        } SSESR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:8;
                vuint32_t STSV:24;
            } B;
        } STSRR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:5;
                vuint32_t FRAME_COUNTER:3;
                vuint32_t DTSV:24;
            } B;
        } DTSRR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:13;
                vuint32_t SLOT_EN:1;
                vuint32_t TS_CAPT:1;
                vuint32_t:1;
                vuint32_t SMCL:1;
                vuint32_t:9;
                vuint32_t DRL:5;
                vuint32_t CRC_P:1;
            } B;
        } S1FCR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:13;
                vuint32_t SLOT_EN:1;
                vuint32_t TS_CAPT:1;
                vuint32_t:1;
                vuint32_t SMCL:1;
                vuint32_t:9;
                vuint32_t DRL:5;
                vuint32_t CRC_P:1;
            } B;
        } S2FCR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:13;
                vuint32_t SLOT_EN:1;
                vuint32_t TS_CAPT:1;
                vuint32_t:1;
                vuint32_t SMCL:1;
                vuint32_t:9;
                vuint32_t DRL:5;
                vuint32_t CRC_P:1;
            } B;
        } S3FCR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:13;
                vuint32_t SLOT_EN:1;
                vuint32_t TS_CAPT:1;
                vuint32_t:1;
                vuint32_t SMCL:1;
                vuint32_t:9;
                vuint32_t DRL:5;
                vuint32_t CRC_P:1;
            } B;
        } S4FCR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:13;
                vuint32_t SLOT_EN:1;
                vuint32_t TS_CAPT:1;
                vuint32_t:1;
                vuint32_t SMCL:1;
                vuint32_t:9;
                vuint32_t DRL:5;
                vuint32_t CRC_P:1;
            } B;
        } S5FCR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:13;
                vuint32_t SLOT_EN:1;
                vuint32_t TS_CAPT:1;
                vuint32_t:1;
                vuint32_t SMCL:1;
                vuint32_t:9;
                vuint32_t DRL:5;
                vuint32_t CRC_P:1;
            } B;
        } S6FCR;

        union {
            vuint16_t R;
            struct {
                vuint16_t:1;
                vuint16_t SNSBT:15;
            } B;
        } S1SBR;

        union {
            vuint16_t R;
            struct {
                vuint16_t:1;
                vuint16_t SNSBT:15;
            } B;
        } S2SBR;

        union {
            vuint16_t R;
            struct {
                vuint16_t:1;
                vuint16_t SNSBT:15;
            } B;
        } S3SBR;

        union {
            vuint16_t R;
            struct {
                vuint16_t:1;
                vuint16_t SNSBT:15;
            } B;
        } S4SBR;

        union {
            vuint16_t R;
            struct {
                vuint16_t:1;
                vuint16_t SNSBT:15;
            } B;
        } S5SBR;

        union {
            vuint16_t R;
            struct {
                vuint16_t:1;
                vuint16_t SNSBT:15;
            } B;
        } S6SBR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:13;
                vuint32_t SLOT_NO:3;
                vuint32_t:1;
                vuint32_t SNEBT:15;
            } B;
        } SNEBR;

        union {
            vuint16_t R;
            struct {
                vuint16_t:6;
                vuint16_t DBR_RST:1;
                vuint16_t DSR_RST:1;
                vuint16_t CMD_TYPE:3;
                vuint16_t DEFAULT_SYNC:1;
                vuint16_t GTM_TRIG_SEL:1;
                vuint16_t SPULSE_SEL:1;
                vuint16_t OP_SEL:1;
                vuint16_t SW_READY:1;
            } B;
        } DOBCR;

        union {
            vuint16_t R;
            struct {
                vuint16_t:9;
                vuint16_t MDDIS_OFF:7;
            } B;
        } MDDIS_OFF;

        union {
            vuint16_t R;
            struct {
                vuint16_t:9;
                vuint16_t PULSE_WIDTH0:7;
            } B;
        } PW0D;

        union {
            vuint16_t R;
            struct {
                vuint16_t:9;
                vuint16_t PULSE_WIDTH:7;
            } B;
        } PW1D;

        union {
            vuint16_t R;
            struct {
                vuint16_t CTPR:16;
            } B;
        } CTPR;

        union {
            vuint16_t R;
            struct {
                vuint16_t CIPR:16;
            } B;
        } CIPR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:8;
                vuint32_t D19:1;
                vuint32_t D18:1;
                vuint32_t D17:1;
                vuint32_t D16:1;
                vuint32_t D15:1;
                vuint32_t D14:1;
                vuint32_t D13:1;
                vuint32_t D12:1;
                vuint32_t D11:1;
                vuint32_t D10:1;
                vuint32_t D9:1;
                vuint32_t D8:1;
                vuint32_t D7:1;
                vuint32_t D6:1;
                vuint32_t D5:1;
                vuint32_t D4:1;
                vuint32_t D3:1;
                vuint32_t D2:1;
                vuint32_t D1:1;
                vuint32_t D0:1;
                vuint32_t A3:1;
                vuint32_t A2:1;
                vuint32_t A1:1;
                vuint32_t A0:1;
            } B;
        } DPRL;

        union {
            vuint32_t R;
            struct {
                vuint32_t Reserved:32;
            } B;
        } DPRH;

        union {
            vuint32_t R;
            struct {
                vuint32_t D:19;
                vuint32_t A:4;
                vuint32_t START_SEQUENCE:9;
            } B;
        } DBRL;

        union {
            vuint32_t R;
            struct {
                vuint32_t:21;
                vuint32_t CRC:6;
                vuint32_t:1;
                vuint32_t D:4;
            } B;
        } DBRH;

        union {
            vuint32_t R;
            struct {
                vuint32_t D:19;
                vuint32_t A:4;
                vuint32_t START_SEQUENCE:9;
            } B;
        } DSRL;

        union {
            vuint32_t R;
            struct {
                vuint32_t:21;
                vuint32_t CRC:6;
                vuint32_t:1;
                vuint32_t D:4;
            } B;
        } DSRH;
    } CH[5];
};/**************************************************************************/
/*                    Module: RCOSC Digital Interface                     */
/**************************************************************************/
struct RCOSC_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:7;
            vuint32_t DEM_EN:1;
            vuint32_t:3;
            vuint32_t USER_TRIM:5;
            vuint32_t:3;
            vuint32_t RCDIV:5;
            vuint32_t:8;
        } B;
    } CTL;

    union {
        vuint32_t R;
        struct {
            vuint32_t:20;
            vuint32_t REG_EN:1;
            vuint32_t TSENS_EN:1;
            vuint32_t:2;
            vuint32_t RCTRIM:8;
        } B;
    } NT;

    union {
        vuint32_t R;
        struct {
            vuint32_t:2;
            vuint32_t COLD_TRIM:10;
            vuint32_t:2;
            vuint32_t HOT_TRIM:10;
            vuint32_t:2;
            vuint32_t TSENS_TRIM:6;
        } B;
    } TT;
};
/**************************************************************************/
/*                   Module: SARADC                                       */
/**************************************************************************/
struct SARADC_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t OWREN:1;
            vuint32_t WLSIDE:1;
            vuint32_t MODE:1;
            vuint32_t:1;
            vuint32_t NSTART:1;
            vuint32_t:1;
            vuint32_t:2;
            vuint32_t JSTART:1;
            vuint32_t JTRGEN:1;
            vuint32_t JEDGESEL:2;
            vuint32_t JTRGSEQ:1;
            vuint32_t:1;
            vuint32_t:1;
            vuint32_t:1;
            vuint32_t:4;
            vuint32_t JTRGSEL:4;
            vuint32_t ABORTCHAIN:1;
            vuint32_t ABORT:1;
            vuint32_t:1;
            vuint32_t FRZ:1;
            vuint32_t:3;
            vuint32_t PWDN:1;
        } B;
    } MCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t NSTART:1;
            vuint32_t:3;
            vuint32_t JSTART:1;
            vuint32_t:4;
            vuint32_t JABORT:1;
            vuint32_t:1;
            vuint32_t:1;
            vuint32_t CHADDR:8;
            vuint32_t:5;
            vuint32_t ADCSTATUS:3;
        } B;
    } MSR;

    uint8_t SARADC_reserved1[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:27;
            vuint32_t:1;
            vuint32_t JEOC:1;
            vuint32_t JECH:1;
            vuint32_t NEOC:1;
            vuint32_t NECH:1;
        } B;
    } ISR;

    union {
        vuint32_t R;
        struct {
            vuint32_t EOC_CH:32;
        } B;
    } SARADC_ICIPR[3];

    union {
        vuint32_t R;
        struct {
            vuint32_t:27;
            vuint32_t:1;
            vuint32_t MSKJEOC:1;
            vuint32_t MSKJECH:1;
            vuint32_t MSKNEOC:1;
            vuint32_t MSKNECH:1;
        } B;
    } IMR;

    union {
        vuint32_t R;
        struct {
            vuint32_t IM_CH:32;
        } B;
    } SARADC_ICIMR[3];

    union {
        vuint32_t R;
        struct {
            vuint32_t WDG15H:1;
            vuint32_t WDG15L:1;
            vuint32_t WDG14H:1;
            vuint32_t WDG14L:1;
            vuint32_t WDG13H:1;
            vuint32_t WDG13L:1;
            vuint32_t WDG12H:1;
            vuint32_t WDG12L:1;
            vuint32_t WDG11H:1;
            vuint32_t WDG11L:1;
            vuint32_t WDG10H:1;
            vuint32_t WDG10L:1;
            vuint32_t WDG9H:1;
            vuint32_t WDG9L:1;
            vuint32_t WDG8H:1;
            vuint32_t WDG8L:1;
            vuint32_t WDG7H:1;
            vuint32_t WDG7L:1;
            vuint32_t WDG6H:1;
            vuint32_t WDG6L:1;
            vuint32_t WDG5H:1;
            vuint32_t WDG5L:1;
            vuint32_t WDG4H:1;
            vuint32_t WDG4L:1;
            vuint32_t WDG3H:1;
            vuint32_t WDG3L:1;
            vuint32_t WDG2H:1;
            vuint32_t WDG2L:1;
            vuint32_t WDG1H:1;
            vuint32_t WDG1L:1;
            vuint32_t WDG0H:1;
            vuint32_t WDG0L:1;
        } B;
    } WTISR;

    union {
        vuint32_t R;
        struct {
            vuint32_t MSKWDG15H:1;
            vuint32_t MSKWDG15L:1;
            vuint32_t MSKWDG14H:1;
            vuint32_t MSKWDG14L:1;
            vuint32_t MSKWDG13H:1;
            vuint32_t MSKWDG13L:1;
            vuint32_t MSKWDG12H:1;
            vuint32_t MSKWDG12L:1;
            vuint32_t MSKWDG11H:1;
            vuint32_t MSKWDG11L:1;
            vuint32_t MSKWDG10H:1;
            vuint32_t MSKWDG10L:1;
            vuint32_t MSKWDG9H:1;
            vuint32_t MSKWDG9L:1;
            vuint32_t MSKWDG8H:1;
            vuint32_t MSKWDG8L:1;
            vuint32_t MSKWDG7H:1;
            vuint32_t MSKWDG7L:1;
            vuint32_t MSKWDG6H:1;
            vuint32_t MSKWDG6L:1;
            vuint32_t MSKWDG5H:1;
            vuint32_t MSKWDG5L:1;
            vuint32_t MSKWDG4H:1;
            vuint32_t MSKWDG4L:1;
            vuint32_t MSKWDG3H:1;
            vuint32_t MSKWDG3L:1;
            vuint32_t MSKWDG2H:1;
            vuint32_t MSKWDG2L:1;
            vuint32_t MSKWDG1H:1;
            vuint32_t MSKWDG1L:1;
            vuint32_t MSKWDG0H:1;
            vuint32_t MSKWDG0L:1;
        } B;
    } WTIMR;

    uint8_t SARADC_reserved2[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:30;
            vuint32_t DCLR:1;
            vuint32_t DMAEN:1;
        } B;
    } DMAE;

    union {
        vuint32_t R;
        struct {
            vuint32_t DS_CH:32;
        } B;
    } ICDSR[3];

    uint8_t SARADC_reserved3[16];

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t THRH:12;
            vuint32_t:4;
            vuint32_t THRL:12;
        } B;
    } WTHRHLR[4];

    uint8_t SARADC_reserved4[36];

    union {
        vuint32_t R;
        struct {
            vuint32_t:20;
            vuint32_t PRECHG:4;
            vuint32_t INPSAMP:8;
        } B;
    } CTR[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t NCE_CH:32;
        } B;
    } ICNCMR[3];

    uint8_t SARADC_reserved5[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t JCE_CH:32;
        } B;
    } ICJCMR[3];

    uint8_t SARADC_reserved6[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t PDED:8;
        } B;
    } PDEDR;

    uint8_t SARADC_reserved7[52];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ICDR[96];

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t THRH:12;
            vuint32_t:4;
            vuint32_t THRL:12;
        } B;
    } WTHRHLR4;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t THRH:12;
            vuint32_t:4;
            vuint32_t THRL:12;
        } B;
    } WTHRHLR5;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t THRH:12;
            vuint32_t:4;
            vuint32_t THRL:12;
        } B;
    } WTHRHLR6;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t THRH:12;
            vuint32_t:4;
            vuint32_t THRL:12;
        } B;
    } WTHRHLR7;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t THRH:12;
            vuint32_t:4;
            vuint32_t THRL:12;
        } B;
    } WTHRHLR8;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t THRH:12;
            vuint32_t:4;
            vuint32_t THRL:12;
        } B;
    } WTHRHLR9;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t THRH:12;
            vuint32_t:4;
            vuint32_t THRL:12;
        } B;
    } WTHRHLR10;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t THRH:12;
            vuint32_t:4;
            vuint32_t THRL:12;
        } B;
    } WTHRHLR11;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t THRH:12;
            vuint32_t:4;
            vuint32_t THRL:12;
        } B;
    } WTHRHLR12;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t THRH:12;
            vuint32_t:4;
            vuint32_t THRL:12;
        } B;
    } WTHRHLR13;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t THRH:12;
            vuint32_t:4;
            vuint32_t THRL:12;
        } B;
    } WTHRHLR14;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t THRH:12;
            vuint32_t:4;
            vuint32_t THRL:12;
        } B;
    } WTHRHLR15;

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
    } ICWSELR[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t WEN_CH:32;
        } B;
    } ICWENR[3];

    uint8_t SARADC_reserved8[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t AWOR_CH:32;
        } B;
    } ICAWORR[3];

    uint8_t SARADC_reserved9[260];

    union {
        vuint32_t R;
        struct {
            vuint32_t EOC_CH:32;
        } B;
    } TCIPR;

    union {
        vuint32_t R;
        struct {
            vuint32_t IM_CH:32;
        } B;
    } TCIMR;

    union {
        vuint32_t R;
        struct {
            vuint32_t DS_CH:32;
        } B;
    } TCDSR;

    union {
        vuint32_t R;
        struct {
            vuint32_t NCE_CH:32;
        } B;
    } TCNCMR;

    union {
        vuint32_t R;
        struct {
            vuint32_t JCE_CH:32;
        } B;
    } TCJCMR;

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
    } TCWSELR[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t WEN_CH:32;
        } B;
    } TCWENR;

    union {
        vuint32_t R;
        struct {
            vuint32_t AWOR_CH:32;
        } B;
    } TCAWORR;

    uint8_t SARADC_reserved10[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t ESIC_TCH3:1;
            vuint32_t ICSEL_TCH3:7;
            vuint32_t ESIC_TCH2:1;
            vuint32_t ICSEL_TCH2:7;
            vuint32_t ESIC_TCH1:1;
            vuint32_t ICSEL_TCH1:7;
            vuint32_t ESIC_TCH0:1;
            vuint32_t ICSEL_TCH0:7;
        } B;
    } TCCAPR[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR96;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR97;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR98;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR99;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR100;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR101;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR102;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR103;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR104;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR105;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR106;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR107;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR108;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR109;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR110;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR111;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR112;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR113;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR114;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR115;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR116;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR117;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR118;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR119;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR120;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR121;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR122;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR123;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR124;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR125;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR126;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } TCDR127;

    uint8_t SARADC_0_reserved11[48];

    union {
        vuint32_t R;
        struct {
            vuint32_t:20;
            vuint32_t DSD:12;
        } B;
    } ECDSD;

    uint8_t SARADC_reserved12[12];

    union {
        vuint32_t R;
        struct {
            vuint32_t EOC_CH:32;
        } B;
    } ECIPR[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t IM_CH:32;
        } B;
    } ECIMR[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t DS_CH:32;
        } B;
    } ECDSR[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t NCE_CH:32;
        } B;
    } ECNCMR[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t JCE_CH:32;
        } B;
    } ECJCMR[4];

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
    } ECWSELR[16];

    union {
        vuint32_t R;
        struct {
            vuint32_t WEN_CH:32;
        } B;
    } ECWENR[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t AWOR_CH:32;
        } B;
    } ECAWORR[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t ICSEL_ECH152_159:7;
            vuint32_t:1;
            vuint32_t ICSEL_ECH144_151:7;
            vuint32_t:1;
            vuint32_t ICSEL_ECH136_143:7;
            vuint32_t:1;
            vuint32_t ICSEL_ECH128_135:7;
        } B;
    } ECMICR[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR128;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR129;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR130;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR131;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR132;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR133;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR134;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR135;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR136;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR137;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR138;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR139;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR140;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR141;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR142;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR143;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR144;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR145;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR146;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR147;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR148;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR149;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR150;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR151;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR152;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR153;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR154;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR155;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR156;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR157;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR158;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR159;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR160;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR161;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR162;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR163;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR164;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR165;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR166;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR167;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR168;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR169;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR170;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR171;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR172;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR173;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR174;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR175;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR176;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR177;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR178;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR179;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR180;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR181;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR182;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR183;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR184;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR185;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR186;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR187;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR188;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR189;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR190;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR191;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR192;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR193;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR194;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR195;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR196;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR197;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR198;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR199;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR200;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR201;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR202;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR203;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR204;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR205;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR206;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR207;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR208;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR209;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR210;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR211;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR212;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR213;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR214;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR215;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR216;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR217;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR218;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR219;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR220;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR221;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR222;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR223;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR224;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR225;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR226;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR227;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR228;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR229;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR230;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR231;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR232;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR233;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR234;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR235;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR236;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR237;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR238;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR239;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR240;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR241;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR242;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR243;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR244;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR245;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR246;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR247;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR248;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR249;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR250;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR251;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR252;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR253;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR254;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t REFSEL:1;
            vuint32_t:2;
            vuint32_t PCE:1;
            vuint32_t:1;
            vuint32_t CTSEL:2;
            vuint32_t:4;
            vuint32_t VALID:1;
            vuint32_t OVERW:1;
            vuint32_t RESULT:2;
            vuint32_t:4;
            vuint32_t CDATA:12;
        } B;
    } ECDR255;
};
/**************************************************************************/
/*                   Module: SDADC                                        */
/**************************************************************************/
struct SDADC_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:3;
            vuint32_t PDR:5;
            vuint32_t:1;
            vuint32_t PGAN:3;
            vuint32_t ODF:1;
            vuint32_t ODA:1;
            vuint32_t EMSEL:1;
            vuint32_t HPFEN:1;
            vuint32_t:1;
            vuint32_t TRIGEDSEL:2;
            vuint32_t TRIGEN:1;
            vuint32_t TRIGSEL:4;
            vuint32_t FRZ:1;
            vuint32_t:2;
            vuint32_t VCOMSEL:1;
            vuint32_t:1;
            vuint32_t GECEN:1;
            vuint32_t MODE:1;
            vuint32_t EN:1;
        } B;
    } MCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:8;
            vuint32_t BIASEN:8;
            vuint32_t:13;
            vuint32_t ANCHSEL:3;
        } B;
    } CSR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t RESET_KEY:16;
        } B;
    } RKR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:23;
            vuint32_t DFEF:1;
            vuint32_t:5;
            vuint32_t CDVF:1;
            vuint32_t DFORF:1;
            vuint32_t DFFF:1;
        } B;
    } SFR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:15;
            vuint32_t DFFDIRS:1;
            vuint32_t GDIGE:1;
            vuint32_t:12;
            vuint32_t CDVEE:1;
            vuint32_t DFORIE:1;
            vuint32_t DFFDIRE:1;
        } B;
    } RSER;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t OSD:8;
        } B;
    } OSDR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:20;
            vuint32_t FTHLD:4;
            vuint32_t:5;
            vuint32_t FSIZE:2;
            vuint32_t FE:1;
        } B;
    } FCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t ST_KEY:16;
        } B;
    } STKR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t CDATA:16; /* ODA=0 */
            // vuint32_t CDATA:14;      /* ODA=1 */
            // vuint32_t:2;             /* ODA=1 */
        } B;
    } CDR;
};
/**************************************************************************/
/*                   Module: SEMA42                                       */
/**************************************************************************/
struct SEMA42_tag {
    union {
        vuint8_t R;
        struct {
            vuint8_t:4;
            vuint8_t GTFSM:4;
        } B;
    } GATE[64];

    union {
        vuint16_t R;
        struct {
            // vuint16_t:2; /* different bitfield names docuemnted for read and wirte accesses */
            // vuint16_t RSTGSM:2;
            // vuint16_t RSTGMS:4;
            vuint16_t RSTGDP:8; /* Write access */
            vuint16_t RSTGTN:8;
        } B;
    } RSTGT;
};
/**************************************************************************/
/*                   Module: SIPI                                         */
/**************************************************************************/
struct SIPI_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:15;
            vuint32_t TC:1;
            vuint32_t:8;
            vuint32_t WL:2;
            vuint32_t CHEN:1;
            vuint32_t ST:1;
            vuint32_t IDT:1;
            vuint32_t RRT:1;
            vuint32_t WRT:1;
            vuint32_t DEN:1;
        } B;
    } CCR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t RAR:1;
            vuint32_t TID:3;
            vuint32_t ACKR:1;
            vuint32_t CB:1;
            vuint32_t:2;
        } B;
    } CSR0;

    uint8_t SIPI_reserved1[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t WAIE:1;
            vuint32_t RAIE:1;
            vuint32_t TCIE:1;
            vuint32_t TOIE:1;
            vuint32_t TIDIE:1;
            vuint32_t ACKIE:1;
        } B;
    } CIR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t TOR:8;
        } B;
    } CTOR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t CRCI:16;
            vuint32_t CRCT:16;
        } B;
    } CCRC0;

    union {
        vuint32_t R;
        struct {
            vuint32_t CAR:32;
        } B;
    } CAR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t CDR:32;
        } B;
    } CDR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t:15;
            vuint32_t TC:1;
            vuint32_t:8;
            vuint32_t WL:2;
            vuint32_t CHEN:1;
            vuint32_t ST:1;
            vuint32_t IDT:1;
            vuint32_t RRT:1;
            vuint32_t WRT:1;
            vuint32_t DEN:1;
        } B;
    } CCR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t RAR:1;
            vuint32_t TID:3;
            vuint32_t ACKR:1;
            vuint32_t CB:1;
            vuint32_t:2;
        } B;
    } CSR1;

    uint8_t SIPI_reserved2[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t WAIE:1;
            vuint32_t RAIE:1;
            vuint32_t TCIE:1;
            vuint32_t TOIE:1;
            vuint32_t TIDIE:1;
            vuint32_t ACKIE:1;
        } B;
    } CIR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t TOR:8;
        } B;
    } CTOR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t CRCI:16;
            vuint32_t CRCT:16;
        } B;
    } CCRC1;

    union {
        vuint32_t R;
        struct {
            vuint32_t CAR:32;
        } B;
    } CAR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t CDR:32;
        } B;
    } CDR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t:15;
            vuint32_t TC:1;
            vuint32_t:8;
            vuint32_t WL:2;
            vuint32_t CHEN:1;
            vuint32_t ST:1;
            vuint32_t IDT:1;
            vuint32_t RRT:1;
            vuint32_t WRT:1;
            vuint32_t DEN:1;
        } B;
    } CCR2;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t RAR:1;
            vuint32_t TID:3;
            vuint32_t ACKR:1;
            vuint32_t CB:1;
            vuint32_t:2;
        } B;
    } CSR2;

    uint8_t SIPI_reserved3[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t WAIE:1;
            vuint32_t RAIE:1;
            vuint32_t TCIE:1;
            vuint32_t TOIE:1;
            vuint32_t TIDIE:1;
            vuint32_t ACKIE:1;
        } B;
    } CIR2;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t TOR:8;
        } B;
    } CTOR2;

    union {
        vuint32_t R;
        struct {
            vuint32_t CRCI:16;
            vuint32_t CRCT:16;
        } B;
    } CCRC2;

    union {
        vuint32_t R;
        struct {
            vuint32_t CAR:32;
        } B;
    } CAR2;

    union {
        vuint32_t R;
        struct {
            vuint32_t CDR2:32;
        } B;
    } CDR2[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:15;
            vuint32_t TC:1;
            vuint32_t:8;
            vuint32_t WL:2;
            vuint32_t CHEN:1;
            vuint32_t ST:1;
            vuint32_t IDT:1;
            vuint32_t RRT:1;
            vuint32_t WRT:1;
            vuint32_t DEN:1;
        } B;
    } CCR3;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t RAR:1;
            vuint32_t TID:3;
            vuint32_t ACKR:1;
            vuint32_t CB:1;
            vuint32_t:2;
        } B;
    } CSR3;

    uint8_t SIPI_reserved4[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t WAIE:1;
            vuint32_t RAIE:1;
            vuint32_t TCIE:1;
            vuint32_t TOIE:1;
            vuint32_t TIDIE:1;
            vuint32_t ACKIE:1;
        } B;
    } CIR3;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t TOR:8;
        } B;
    } CTOR3;

    union {
        vuint32_t R;
        struct {
            vuint32_t CRCI:16;
            vuint32_t CRCT:16;
        } B;
    } CCRC3;

    union {
        vuint32_t R;
        struct {
            vuint32_t CAR:32;
        } B;
    } CAR3;

    union {
        vuint32_t R;
        struct {
            vuint32_t CDR2:32;
        } B;
    } CDR3;

    union {
        vuint32_t R;
        struct {
            vuint32_t FRZ:1;
            vuint32_t:1;
            vuint32_t HALT:1;
            vuint32_t:2;
            vuint32_t PRSCLR:11;
            vuint32_t AID:2;
            vuint32_t:3;
            vuint32_t CRCIE:1;
            vuint32_t MCRIE:1;
            vuint32_t:5;
            vuint32_t TEN:1;
            vuint32_t INIT:1;
            vuint32_t MOEN:1;
            vuint32_t SR:1;
        } B;
    } MCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t FRZACK:1;
            vuint32_t LPMACK:1;
            vuint32_t:19;
            vuint32_t GCRCE:1;
            vuint32_t MCR:1;
            vuint32_t:1;
            vuint32_t TE:4;
            vuint32_t STATE:4;
        } B;
    } SR;

    union {
        vuint32_t R;
        struct {
            vuint32_t MXCNT:30;
            vuint32_t:2;
        } B;
    } MAXCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t ADRLD:30;
            vuint32_t:2;
        } B;
    } ARR;

    union {
        vuint32_t R;
        struct {
            vuint32_t ADCNT:30;
            vuint32_t:2;
        } B;
    } ACR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:5;
            vuint32_t TOE3:1;
            vuint32_t TIDE3:1;
            vuint32_t ACKE3:1;
            vuint32_t:5;
            vuint32_t TOE2:1;
            vuint32_t TIDE2:1;
            vuint32_t ACKE2:1;
            vuint32_t:5;
            vuint32_t TOE1:1;
            vuint32_t TIDE1:1;
            vuint32_t ACKE1:1;
            vuint32_t:5;
            vuint32_t TOE0:1;
            vuint32_t TIDE0:1;
            vuint32_t ACKE0:1;
        } B;
    } ERR;
};
/**************************************************************************/
/*                   Module: SIUL2                                        */
/**************************************************************************/
struct SIUL2_tag {
    uint8_t SIUL2_reserved0[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t PARTNUM:16;
            vuint32_t ED:1;
            vuint32_t PKG:5;
            vuint32_t:2;
            vuint32_t MAJOR_MASK:4;
            vuint32_t MINOR_MASK:4;
        } B;
    } MIDR1;

    union {
        vuint32_t R;
        struct {
            vuint32_t SF:1;
            vuint32_t FLASH_SIZE_1:4;
            vuint32_t FLASH_SIZE_2:4;
            vuint32_t:7;
            vuint32_t FAMILYNUM:8;
            vuint32_t:8;
        } B;
    } MIDR2;

    uint8_t SIUL2_reserved1[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:21;
            vuint32_t:2;
            vuint32_t:3;
            vuint32_t EIF:6;
        } B;
    } DISR0;

    uint8_t SIUL2_reserved2[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:21;
            vuint32_t:2;
            vuint32_t:3;
            vuint32_t EIRE:6;
        } B;
    } DIRER0;

    uint8_t SIUL2_reserved3[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:21;
            vuint32_t:2;
            vuint32_t DIRSR:6;
        } B;
    } DIRSR0;

    uint8_t SIUL2_reserved4[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:21;
            vuint32_t:2;
            vuint32_t:3;
            vuint32_t IREE:6;
        } B;
    } IREER0;

    uint8_t SIUL2_reserved5[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:21;
            vuint32_t:2;
            vuint32_t:3;
            vuint32_t IFEE:6;
        } B;
    } IFEER0;

    uint8_t SIUL2_reserved6[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:21;
            vuint32_t:2;
            vuint32_t:3;
            vuint32_t IFE:6;
        } B;
    } IFER0;

    uint8_t SIUL2_reserved7[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:28;
            vuint32_t MAXCNTx:4;
        } B;
    } IFMCR[32];

    union {
        vuint32_t R;
        struct {
            vuint32_t:28;
            vuint32_t IFCP:4;
        } B;
    } IFCPR;


    uint8_t SIUL2_reserved8[380];

    union {                              /* I/O Pin Multiplexed Signal Configuration Registers */
        vuint32_t R;
        struct {
          vuint32_t  :2;
          vuint32_t OERC:2;
          vuint32_t  :1;
          vuint32_t ODC:3;
          vuint32_t SMC:1;
          vuint32_t APC:1;
          vuint32_t ILS:2;
          vuint32_t IBE:1;
          vuint32_t HYS:1;
          vuint32_t WPDE:1;
          vuint32_t WPUE:1;
          vuint32_t INV:1;
          vuint32_t  :7;
          vuint32_t SSS:8;
        } B;
      } MSCR_IO[512];

      union {                              /* Multiplexed Signal Configuration Register for Multiplexed Input Selection */
        vuint32_t R;
        struct {
          vuint32_t  :16;
          vuint32_t INV:1;
          vuint32_t  :7;
          vuint32_t SSS:8;
        } B;
      } MSCR_MUX[512];

    uint8_t SIUL2_reserved9[192];

    union {
        vuint8_t R;
        struct {
            vuint8_t:7;
            vuint8_t PDO:1;
        } B;
    } GPDO[512];

    union {
        vuint8_t R;
        struct {
            vuint8_t:7;
            vuint8_t PDI:1;
        } B;
    } GPDI[512];

    union {
        vuint16_t R;
        struct {
            vuint16_t PPDO:16;
        } B;
    } PGPDO[32];

    union {
        vuint16_t R;
        struct {
            vuint16_t PPDI:16;
        } B;
    } PGPDI[32];

    union {
        vuint32_t R;
        struct {
            vuint32_t MASK:16;
            vuint32_t MPPDO:16;
        } B;
    } MPGPDO[32];
};
/**************************************************************************/
/*                   Module: SMPU                                         */
/**************************************************************************/
struct SMPU_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t MERR:16;
            vuint32_t:1;
            vuint32_t:11;
            vuint32_t HRL:3;
            vuint32_t GVLD:1;
        } B;
    } CESR0;

    union {
        vuint32_t R;
        struct {
            vuint32_t MEOVR:16;
            vuint32_t:1;
            vuint32_t:11;
            vuint32_t NRGD:4;
        } B;
    } CESR1;

    uint8_t SMPU_reserved1[248];

    struct {
        union {
            vuint32_t R;
            struct {
                vuint32_t EADDR:32;
            } B;
        } EAR;

        union {
            vuint32_t R;
            struct {
                vuint32_t EACD:24;
                vuint32_t:1;
                vuint32_t EATTR:2;
                vuint32_t ERW:1;
                vuint32_t EMN:4;
            } B;
        } EDR;
    } Channel[16];

    uint8_t SMPU_reserved2[640];

    union {
        vuint32_t R;
        struct {
            vuint32_t SRTADDR:32;
        } B;
    } RGD0_WORD0;

    union {
        vuint32_t R;
        struct {
            vuint32_t ENDADDR:32;
        } B;
    } RGD0_WORD1;

    union {
        vuint32_t R;
        struct {
            vuint32_t M0P:2;    /* FMT0 */
            vuint32_t M1P:2;
            vuint32_t M2P:2;
            vuint32_t M3P:2;
            vuint32_t M4P:2;
            vuint32_t M5P:2;
            vuint32_t M6P:2;
            vuint32_t M7P:2;
            vuint32_t M8P:2;
            vuint32_t M9P:2;
            vuint32_t M10P:2;
            vuint32_t M11P:2;
            vuint32_t M12P:2;
            vuint32_t M13P:2;
            vuint32_t M14P:2;
            vuint32_t M15P:2;
            // vuint32_t M0S:2;  /* FMT1 */
            // vuint32_t M1S:2;
            // vuint32_t M2S:2;
            // vuint32_t M3S:2;
            // vuint32_t M4S:2;
            // vuint32_t M5S:2;
            // vuint32_t M6S:2;
            // vuint32_t M7S:2;
            // vuint32_t M8S:2;
            // vuint32_t M9S:2;
            // vuint32_t M10S:2;
            // vuint32_t M11S:2;
            // vuint32_t M12S:2;
            // vuint32_t M13S:2;
            // vuint32_t M14S:2;
            // vuint32_t M15S:2;
        } B;
    } RGD0_WORD2;

    union {
        vuint32_t R;
        struct {
            vuint32_t ACCSET1:6;
            vuint32_t ACCSET2:6;
            vuint32_t ACCSET3:6;
            vuint32_t:9;
            vuint32_t FMT:1;
            vuint32_t RO:1;
            vuint32_t:1;
            vuint32_t CI:1;
            vuint32_t VLD:1;
        } B;
    } RGD0_WORD3;

    union {
        vuint32_t R;
        struct {
            vuint32_t SRTADDR:32;
        } B;
    } RGD1_WORD0;

    union {
        vuint32_t R;
        struct {
            vuint32_t ENDADDR:32;
        } B;
    } RGD1_WORD1;

    union {
        vuint32_t R;
        struct {
            vuint32_t M0P:2;    /* FMT0 */
            vuint32_t M1P:2;
            vuint32_t M2P:2;
            vuint32_t M3P:2;
            vuint32_t M4P:2;
            vuint32_t M5P:2;
            vuint32_t M6P:2;
            vuint32_t M7P:2;
            vuint32_t M8P:2;
            vuint32_t M9P:2;
            vuint32_t M10P:2;
            vuint32_t M11P:2;
            vuint32_t M12P:2;
            vuint32_t M13P:2;
            vuint32_t M14P:2;
            vuint32_t M15P:2;
            // vuint32_t M0S:2;  /* FMT1 */
            // vuint32_t M1S:2;
            // vuint32_t M2S:2;
            // vuint32_t M3S:2;
            // vuint32_t M4S:2;
            // vuint32_t M5S:2;
            // vuint32_t M6S:2;
            // vuint32_t M7S:2;
            // vuint32_t M8S:2;
            // vuint32_t M9S:2;
            // vuint32_t M10S:2;
            // vuint32_t M11S:2;
            // vuint32_t M12S:2;
            // vuint32_t M13S:2;
            // vuint32_t M14S:2;
            // vuint32_t M15S:2;
        } B;
    } RGD1_WORD2;

    union {
        vuint32_t R;
        struct {
            vuint32_t ACCSET1:6;
            vuint32_t ACCSET2:6;
            vuint32_t ACCSET3:6;
            vuint32_t:9;
            vuint32_t FMT:1;
            vuint32_t RO:1;
            vuint32_t:1;
            vuint32_t CI:1;
            vuint32_t VLD:1;
        } B;
    } RGD1_WORD3;

    union {
        vuint32_t R;
        struct {
            vuint32_t SRTADDR:32;
        } B;
    } RGD2_WORD0;

    union {
        vuint32_t R;
        struct {
            vuint32_t ENDADDR:32;
        } B;
    } RGD2_WORD1;

    union {
        vuint32_t R;
        struct {
            vuint32_t M0P:2;    /* FMT0 */
            vuint32_t M1P:2;
            vuint32_t M2P:2;
            vuint32_t M3P:2;
            vuint32_t M4P:2;
            vuint32_t M5P:2;
            vuint32_t M6P:2;
            vuint32_t M7P:2;
            vuint32_t M8P:2;
            vuint32_t M9P:2;
            vuint32_t M10P:2;
            vuint32_t M11P:2;
            vuint32_t M12P:2;
            vuint32_t M13P:2;
            vuint32_t M14P:2;
            vuint32_t M15P:2;
            // vuint32_t M0S:2;  /* FMT1 */
            // vuint32_t M1S:2;
            // vuint32_t M2S:2;
            // vuint32_t M3S:2;
            // vuint32_t M4S:2;
            // vuint32_t M5S:2;
            // vuint32_t M6S:2;
            // vuint32_t M7S:2;
            // vuint32_t M8S:2;
            // vuint32_t M9S:2;
            // vuint32_t M10S:2;
            // vuint32_t M11S:2;
            // vuint32_t M12S:2;
            // vuint32_t M13S:2;
            // vuint32_t M14S:2;
            // vuint32_t M15S:2;
        } B;
    } RGD2_WORD2;

    union {
        vuint32_t R;
        struct {
            vuint32_t ACCSET1:6;
            vuint32_t ACCSET2:6;
            vuint32_t ACCSET3:6;
            vuint32_t:9;
            vuint32_t FMT:1;
            vuint32_t RO:1;
            vuint32_t:1;
            vuint32_t CI:1;
            vuint32_t VLD:1;
        } B;
    } RGD2_WORD3;

    union {
        vuint32_t R;
        struct {
            vuint32_t SRTADDR:32;
        } B;
    } RGD3_WORD0;

    union {
        vuint32_t R;
        struct {
            vuint32_t ENDADDR:32;
        } B;
    } RGD3_WORD1;

    union {
        vuint32_t R;
        struct {
            vuint32_t M0P:2;    /* FMT0 */
            vuint32_t M1P:2;
            vuint32_t M2P:2;
            vuint32_t M3P:2;
            vuint32_t M4P:2;
            vuint32_t M5P:2;
            vuint32_t M6P:2;
            vuint32_t M7P:2;
            vuint32_t M8P:2;
            vuint32_t M9P:2;
            vuint32_t M10P:2;
            vuint32_t M11P:2;
            vuint32_t M12P:2;
            vuint32_t M13P:2;
            vuint32_t M14P:2;
            vuint32_t M15P:2;
            // vuint32_t M0S:2;  /* FMT1 */
            // vuint32_t M1S:2;
            // vuint32_t M2S:2;
            // vuint32_t M3S:2;
            // vuint32_t M4S:2;
            // vuint32_t M5S:2;
            // vuint32_t M6S:2;
            // vuint32_t M7S:2;
            // vuint32_t M8S:2;
            // vuint32_t M9S:2;
            // vuint32_t M10S:2;
            // vuint32_t M11S:2;
            // vuint32_t M12S:2;
            // vuint32_t M13S:2;
            // vuint32_t M14S:2;
            // vuint32_t M15S:2;
        } B;
    } RGD3_WORD2;

    union {
        vuint32_t R;
        struct {
            vuint32_t ACCSET1:6;
            vuint32_t ACCSET2:6;
            vuint32_t ACCSET3:6;
            vuint32_t:9;
            vuint32_t FMT:1;
            vuint32_t RO:1;
            vuint32_t:1;
            vuint32_t CI:1;
            vuint32_t VLD:1;
        } B;
    } RGD3_WORD3;

    union {
        vuint32_t R;
        struct {
            vuint32_t SRTADDR:32;
        } B;
    } RGD4_WORD0;

    union {
        vuint32_t R;
        struct {
            vuint32_t ENDADDR:32;
        } B;
    } RGD4_WORD1;

    union {
        vuint32_t R;
        struct {
            vuint32_t M0P:2;    /* FMT0 */
            vuint32_t M1P:2;
            vuint32_t M2P:2;
            vuint32_t M3P:2;
            vuint32_t M4P:2;
            vuint32_t M5P:2;
            vuint32_t M6P:2;
            vuint32_t M7P:2;
            vuint32_t M8P:2;
            vuint32_t M9P:2;
            vuint32_t M10P:2;
            vuint32_t M11P:2;
            vuint32_t M12P:2;
            vuint32_t M13P:2;
            vuint32_t M14P:2;
            vuint32_t M15P:2;
            // vuint32_t M0S:2;  /* FMT1 */
            // vuint32_t M1S:2;
            // vuint32_t M2S:2;
            // vuint32_t M3S:2;
            // vuint32_t M4S:2;
            // vuint32_t M5S:2;
            // vuint32_t M6S:2;
            // vuint32_t M7S:2;
            // vuint32_t M8S:2;
            // vuint32_t M9S:2;
            // vuint32_t M10S:2;
            // vuint32_t M11S:2;
            // vuint32_t M12S:2;
            // vuint32_t M13S:2;
            // vuint32_t M14S:2;
            // vuint32_t M15S:2;
        } B;
    } RGD4_WORD2;

    union {
        vuint32_t R;
        struct {
            vuint32_t ACCSET1:6;
            vuint32_t ACCSET2:6;
            vuint32_t ACCSET3:6;
            vuint32_t:9;
            vuint32_t FMT:1;
            vuint32_t RO:1;
            vuint32_t:1;
            vuint32_t CI:1;
            vuint32_t VLD:1;
        } B;
    } RGD4_WORD3;

    union {
        vuint32_t R;
        struct {
            vuint32_t SRTADDR:32;
        } B;
    } RGD5_WORD0;

    union {
        vuint32_t R;
        struct {
            vuint32_t ENDADDR:32;
        } B;
    } RGD5_WORD1;

    union {
        vuint32_t R;
        struct {
            vuint32_t M0P:2;    /* FMT0 */
            vuint32_t M1P:2;
            vuint32_t M2P:2;
            vuint32_t M3P:2;
            vuint32_t M4P:2;
            vuint32_t M5P:2;
            vuint32_t M6P:2;
            vuint32_t M7P:2;
            vuint32_t M8P:2;
            vuint32_t M9P:2;
            vuint32_t M10P:2;
            vuint32_t M11P:2;
            vuint32_t M12P:2;
            vuint32_t M13P:2;
            vuint32_t M14P:2;
            vuint32_t M15P:2;
            // vuint32_t M0S:2;  /* FMT1 */
            // vuint32_t M1S:2;
            // vuint32_t M2S:2;
            // vuint32_t M3S:2;
            // vuint32_t M4S:2;
            // vuint32_t M5S:2;
            // vuint32_t M6S:2;
            // vuint32_t M7S:2;
            // vuint32_t M8S:2;
            // vuint32_t M9S:2;
            // vuint32_t M10S:2;
            // vuint32_t M11S:2;
            // vuint32_t M12S:2;
            // vuint32_t M13S:2;
            // vuint32_t M14S:2;
            // vuint32_t M15S:2;
        } B;
    } RGD5_WORD2;

    union {
        vuint32_t R;
        struct {
            vuint32_t ACCSET1:6;
            vuint32_t ACCSET2:6;
            vuint32_t ACCSET3:6;
            vuint32_t:9;
            vuint32_t FMT:1;
            vuint32_t RO:1;
            vuint32_t:1;
            vuint32_t CI:1;
            vuint32_t VLD:1;
        } B;
    } RGD5_WORD3;

    union {
        vuint32_t R;
        struct {
            vuint32_t SRTADDR:32;
        } B;
    } RGD6_WORD0;

    union {
        vuint32_t R;
        struct {
            vuint32_t ENDADDR:32;
        } B;
    } RGD6_WORD1;

    union {
        vuint32_t R;
        struct {
            vuint32_t M0P:2;    /* FMT0 */
            vuint32_t M1P:2;
            vuint32_t M2P:2;
            vuint32_t M3P:2;
            vuint32_t M4P:2;
            vuint32_t M5P:2;
            vuint32_t M6P:2;
            vuint32_t M7P:2;
            vuint32_t M8P:2;
            vuint32_t M9P:2;
            vuint32_t M10P:2;
            vuint32_t M11P:2;
            vuint32_t M12P:2;
            vuint32_t M13P:2;
            vuint32_t M14P:2;
            vuint32_t M15P:2;
            // vuint32_t M0S:2;  /* FMT1 */
            // vuint32_t M1S:2;
            // vuint32_t M2S:2;
            // vuint32_t M3S:2;
            // vuint32_t M4S:2;
            // vuint32_t M5S:2;
            // vuint32_t M6S:2;
            // vuint32_t M7S:2;
            // vuint32_t M8S:2;
            // vuint32_t M9S:2;
            // vuint32_t M10S:2;
            // vuint32_t M11S:2;
            // vuint32_t M12S:2;
            // vuint32_t M13S:2;
            // vuint32_t M14S:2;
            // vuint32_t M15S:2;
        } B;
    } RGD6_WORD2;

    union {
        vuint32_t R;
        struct {
            vuint32_t ACCSET1:6;
            vuint32_t ACCSET2:6;
            vuint32_t ACCSET3:6;
            vuint32_t:9;
            vuint32_t FMT:1;
            vuint32_t RO:1;
            vuint32_t:1;
            vuint32_t CI:1;
            vuint32_t VLD:1;
        } B;
    } RGD6_WORD3;

    union {
        vuint32_t R;
        struct {
            vuint32_t SRTADDR:32;
        } B;
    } RGD7_WORD0;

    union {
        vuint32_t R;
        struct {
            vuint32_t ENDADDR:32;
        } B;
    } RGD7_WORD1;

    union {
        vuint32_t R;
        struct {
            vuint32_t M0P:2;    /* FMT0 */
            vuint32_t M1P:2;
            vuint32_t M2P:2;
            vuint32_t M3P:2;
            vuint32_t M4P:2;
            vuint32_t M5P:2;
            vuint32_t M6P:2;
            vuint32_t M7P:2;
            vuint32_t M8P:2;
            vuint32_t M9P:2;
            vuint32_t M10P:2;
            vuint32_t M11P:2;
            vuint32_t M12P:2;
            vuint32_t M13P:2;
            vuint32_t M14P:2;
            vuint32_t M15P:2;
            // vuint32_t M0S:2;  /* FMT1 */
            // vuint32_t M1S:2;
            // vuint32_t M2S:2;
            // vuint32_t M3S:2;
            // vuint32_t M4S:2;
            // vuint32_t M5S:2;
            // vuint32_t M6S:2;
            // vuint32_t M7S:2;
            // vuint32_t M8S:2;
            // vuint32_t M9S:2;
            // vuint32_t M10S:2;
            // vuint32_t M11S:2;
            // vuint32_t M12S:2;
            // vuint32_t M13S:2;
            // vuint32_t M14S:2;
            // vuint32_t M15S:2;
        } B;
    } RGD7_WORD2;

    union {
        vuint32_t R;
        struct {
            vuint32_t ACCSET1:6;
            vuint32_t ACCSET2:6;
            vuint32_t ACCSET3:6;
            vuint32_t:9;
            vuint32_t FMT:1;
            vuint32_t RO:1;
            vuint32_t:1;
            vuint32_t CI:1;
            vuint32_t VLD:1;
        } B;
    } RGD7_WORD3;

    union {
        vuint32_t R;
        struct {
            vuint32_t SRTADDR:32;
        } B;
    } RGD8_WORD0;

    union {
        vuint32_t R;
        struct {
            vuint32_t ENDADDR:32;
        } B;
    } RGD8_WORD1;

    union {
        vuint32_t R;
        struct {
            vuint32_t M0P:2;    /* FMT0 */
            vuint32_t M1P:2;
            vuint32_t M2P:2;
            vuint32_t M3P:2;
            vuint32_t M4P:2;
            vuint32_t M5P:2;
            vuint32_t M6P:2;
            vuint32_t M7P:2;
            vuint32_t M8P:2;
            vuint32_t M9P:2;
            vuint32_t M10P:2;
            vuint32_t M11P:2;
            vuint32_t M12P:2;
            vuint32_t M13P:2;
            vuint32_t M14P:2;
            vuint32_t M15P:2;
            // vuint32_t M0S:2;  /* FMT1 */
            // vuint32_t M1S:2;
            // vuint32_t M2S:2;
            // vuint32_t M3S:2;
            // vuint32_t M4S:2;
            // vuint32_t M5S:2;
            // vuint32_t M6S:2;
            // vuint32_t M7S:2;
            // vuint32_t M8S:2;
            // vuint32_t M9S:2;
            // vuint32_t M10S:2;
            // vuint32_t M11S:2;
            // vuint32_t M12S:2;
            // vuint32_t M13S:2;
            // vuint32_t M14S:2;
            // vuint32_t M15S:2;
        } B;
    } RGD8_WORD2;

    union {
        vuint32_t R;
        struct {
            vuint32_t ACCSET1:6;
            vuint32_t ACCSET2:6;
            vuint32_t ACCSET3:6;
            vuint32_t:9;
            vuint32_t FMT:1;
            vuint32_t RO:1;
            vuint32_t:1;
            vuint32_t CI:1;
            vuint32_t VLD:1;
        } B;
    } RGD8_WORD3;

    union {
        vuint32_t R;
        struct {
            vuint32_t SRTADDR:32;
        } B;
    } RGD9_WORD0;

    union {
        vuint32_t R;
        struct {
            vuint32_t ENDADDR:32;
        } B;
    } RGD9_WORD1;

    union {
        vuint32_t R;
        struct {
            vuint32_t M0P:2;    /* FMT0 */
            vuint32_t M1P:2;
            vuint32_t M2P:2;
            vuint32_t M3P:2;
            vuint32_t M4P:2;
            vuint32_t M5P:2;
            vuint32_t M6P:2;
            vuint32_t M7P:2;
            vuint32_t M8P:2;
            vuint32_t M9P:2;
            vuint32_t M10P:2;
            vuint32_t M11P:2;
            vuint32_t M12P:2;
            vuint32_t M13P:2;
            vuint32_t M14P:2;
            vuint32_t M15P:2;
            // vuint32_t M0S:2;  /* FMT1 */
            // vuint32_t M1S:2;
            // vuint32_t M2S:2;
            // vuint32_t M3S:2;
            // vuint32_t M4S:2;
            // vuint32_t M5S:2;
            // vuint32_t M6S:2;
            // vuint32_t M7S:2;
            // vuint32_t M8S:2;
            // vuint32_t M9S:2;
            // vuint32_t M10S:2;
            // vuint32_t M11S:2;
            // vuint32_t M12S:2;
            // vuint32_t M13S:2;
            // vuint32_t M14S:2;
            // vuint32_t M15S:2;
        } B;
    } RGD9_WORD2;

    union {
        vuint32_t R;
        struct {
            vuint32_t ACCSET1:6;
            vuint32_t ACCSET2:6;
            vuint32_t ACCSET3:6;
            vuint32_t:9;
            vuint32_t FMT:1;
            vuint32_t RO:1;
            vuint32_t:1;
            vuint32_t CI:1;
            vuint32_t VLD:1;
        } B;
    } RGD9_WORD3;

    union {
        vuint32_t R;
        struct {
            vuint32_t SRTADDR:32;
        } B;
    } RGD10_WORD0;

    union {
        vuint32_t R;
        struct {
            vuint32_t ENDADDR:32;
        } B;
    } RGD10_WORD1;

    union {
        vuint32_t R;
        struct {
            vuint32_t M0P:2;    /* FMT0 */
            vuint32_t M1P:2;
            vuint32_t M2P:2;
            vuint32_t M3P:2;
            vuint32_t M4P:2;
            vuint32_t M5P:2;
            vuint32_t M6P:2;
            vuint32_t M7P:2;
            vuint32_t M8P:2;
            vuint32_t M9P:2;
            vuint32_t M10P:2;
            vuint32_t M11P:2;
            vuint32_t M12P:2;
            vuint32_t M13P:2;
            vuint32_t M14P:2;
            vuint32_t M15P:2;
            // vuint32_t M0S:2;  /* FMT1 */
            // vuint32_t M1S:2;
            // vuint32_t M2S:2;
            // vuint32_t M3S:2;
            // vuint32_t M4S:2;
            // vuint32_t M5S:2;
            // vuint32_t M6S:2;
            // vuint32_t M7S:2;
            // vuint32_t M8S:2;
            // vuint32_t M9S:2;
            // vuint32_t M10S:2;
            // vuint32_t M11S:2;
            // vuint32_t M12S:2;
            // vuint32_t M13S:2;
            // vuint32_t M14S:2;
            // vuint32_t M15S:2;
        } B;
    } RGD10_WORD2;

    union {
        vuint32_t R;
        struct {
            vuint32_t ACCSET1:6;
            vuint32_t ACCSET2:6;
            vuint32_t ACCSET3:6;
            vuint32_t:9;
            vuint32_t FMT:1;
            vuint32_t RO:1;
            vuint32_t:1;
            vuint32_t CI:1;
            vuint32_t VLD:1;
        } B;
    } RGD10_WORD3;

    union {
        vuint32_t R;
        struct {
            vuint32_t SRTADDR:32;
        } B;
    } RGD11_WORD0;

    union {
        vuint32_t R;
        struct {
            vuint32_t ENDADDR:32;
        } B;
    } RGD11_WORD1;

    union {
        vuint32_t R;
        struct {
            vuint32_t M0P:2;    /* FMT0 */
            vuint32_t M1P:2;
            vuint32_t M2P:2;
            vuint32_t M3P:2;
            vuint32_t M4P:2;
            vuint32_t M5P:2;
            vuint32_t M6P:2;
            vuint32_t M7P:2;
            vuint32_t M8P:2;
            vuint32_t M9P:2;
            vuint32_t M10P:2;
            vuint32_t M11P:2;
            vuint32_t M12P:2;
            vuint32_t M13P:2;
            vuint32_t M14P:2;
            vuint32_t M15P:2;
            // vuint32_t M0S:2;  /* FMT1 */
            // vuint32_t M1S:2;
            // vuint32_t M2S:2;
            // vuint32_t M3S:2;
            // vuint32_t M4S:2;
            // vuint32_t M5S:2;
            // vuint32_t M6S:2;
            // vuint32_t M7S:2;
            // vuint32_t M8S:2;
            // vuint32_t M9S:2;
            // vuint32_t M10S:2;
            // vuint32_t M11S:2;
            // vuint32_t M12S:2;
            // vuint32_t M13S:2;
            // vuint32_t M14S:2;
            // vuint32_t M15S:2;
        } B;
    } RGD11_WORD2;

    union {
        vuint32_t R;
        struct {
            vuint32_t ACCSET1:6;
            vuint32_t ACCSET2:6;
            vuint32_t ACCSET3:6;
            vuint32_t:9;
            vuint32_t FMT:1;
            vuint32_t RO:1;
            vuint32_t:1;
            vuint32_t CI:1;
            vuint32_t VLD:1;
        } B;
    } RGD11_WORD3;

};
/**************************************************************************/
/*                   Module: SRX                                          */
/**************************************************************************/
struct SRX_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t TSPRSC:8;
            vuint32_t:14;
            vuint32_t FMDUIE:1;
            vuint32_t SMDUIE:1;
            vuint32_t:3;
            vuint32_t FAST_CLR:1;
            vuint32_t:1;
            vuint32_t DBG_FRZ:1;
            vuint32_t:1;
            vuint32_t SENT_EN:1;
        } B;
    } GBL_CTRL;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t EN_CHn:16;
        } B;
    } CHNL_EN;

    union {
        vuint32_t R;
        struct {
            vuint32_t:22;
            vuint32_t FDMU:1;
            vuint32_t SMDU:1;
            vuint32_t:8;
        } B;
    } GBL_STATUS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t F_RDYn:16;
        } B;
    } FMSG_RDY;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t S_RDYn:16;
        } B;
    } SMSG_RDY;

    uint8_t SRX_reserved1[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t NIBBCH0:3;
            vuint32_t:1;
            vuint32_t NIBBCH1:3;
            vuint32_t:1;
            vuint32_t NIBBCH2:3;
            vuint32_t:1;
            vuint32_t NIBBCH3:3;
            vuint32_t:1;
            vuint32_t NICCH4:3;
            vuint32_t:1;
            vuint32_t NIBBCH5:3;
            vuint32_t:1;
            vuint32_t NIBBCH6:3;
            vuint32_t:1;
            vuint32_t NIBBCH7:3;
        } B;
    } DATA_CTRL1;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t NIBBCH8:3;
            vuint32_t:1;
            vuint32_t NIBBCH9:3;
            vuint32_t:1;
            vuint32_t NIBBCH10:3;
            vuint32_t:1;
            vuint32_t NIBBCH11:3;
            vuint32_t:1;
            vuint32_t NIBBCH12:3;
            vuint32_t:1;
            vuint32_t NIBBCH13:3;
            vuint32_t:1;
            vuint32_t NIBBCH14:3;
            vuint32_t:1;
            vuint32_t NIBBCH15:3;
        } B;
    } DATA_CTRL2;

    uint8_t SRX_reserved2[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t FDMA_ENn:16;
        } B;
    } FDMA_CTRL;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t SDMA_ENn:16;
        } B;
    } SDMA_CTRL;

    uint8_t SRX_reserved3[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t FRDY_IEn:16;
        } B;
    } FRDY_IE;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t SRDY_IEn:16;
        } B;
    } SRDY_IE;

    uint8_t SRX_reserved4[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t CHNUM:4;
            vuint32_t SCNIB:4;
            vuint32_t DNIB1:4;
            vuint32_t DNIB2:4;
            vuint32_t DNIB3:4;
            vuint32_t DNIB4:4;
            vuint32_t DNIB5:4;
            vuint32_t DNIB6:4;
        } B;
    } DMA_FMSG_DATA;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t CRC4b:4;
            vuint32_t:16;
        } B;
    } DMA_FMSG_CRC;

    union {
        vuint32_t R;
        struct {
            vuint32_t TS:32;
        } B;
    } DMA_FMSG_TS;

    uint8_t SRX_reserved5[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t CHNUM:4;
            vuint32_t TYPE:1;
            vuint32_t:16;
            vuint32_t CFG:1;
            vuint32_t ID:4;
            vuint32_t:1;
            vuint32_t IDorDATA:4;
            vuint32_t:1;
        } B;
    } DMA_SMSG3;

    union {
        vuint32_t R;
        struct {
            vuint32_t:10;
            vuint32_t SMCRC:6;
            vuint32_t:4;
            vuint32_t DATA:12;
        } B;
    } DMA_SMSG2;

    union {
        vuint32_t R;
        struct {
            vuint32_t TS:32;
        } B;
    } DMA_SMSG_TS;

    uint8_t SRX_reserved6[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t CMPRSC:15;
            vuint32_t COMP_EN:1;
            vuint32_t:1;
            vuint32_t PRSC:14;
        } B;
    } CH0_CLK_CTRL;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t CAL_RESYNC:1;
            vuint32_t CAL_20_25:1;
            vuint32_t SMSG_OFLW:1;
            vuint32_t FMSG_OFLW:1;
            vuint32_t:1;
            vuint32_t PP_DIAG_ERR:1;
            vuint32_t CAL_LEN_ERR:1;
            vuint32_t CAL_DIAG_ERR:1;
            vuint32_t NIB_VAL_ERR:1;
            vuint32_t SMSG_CRC_ERR:1;
            vuint32_t FMSG_CRC_ERR:1;
            vuint32_t NUM_EDGES_ERR:1;
            vuint32_t:16;
        } B;
    } CH0_STATUS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t IE_CAL_RESYNC:1;
            vuint32_t IE_CAL_20_25:1;
            vuint32_t IE_SMSG_OFLW:1;
            vuint32_t IE_FMSG_OFLW:1;
            vuint32_t:1;
            vuint32_t IE_PP_DIAG_ERR:1;
            vuint32_t IE_CAL_LEN_ERR:1;
            vuint32_t IE_CAL_DIAG_ERR:1;
            vuint32_t IE_NIB_VAL_ERR:1;
            vuint32_t IE_SMSG_CRC_ERR:1;
            vuint32_t IE_FMSG_CRC_ERR:1;
            vuint32_t IE_NUM_EDGES_ERR:1;
            vuint32_t DCHNG_INT:1;
            vuint32_t CAL_RNG:1;
            vuint32_t PP_CHKSEL:1;
            vuint32_t FCRC_TYPE:1;
            vuint32_t FCRC_SC_EN:1;
            vuint32_t SCRC_TYPE:1;
            vuint32_t PAUSE_EN:1;
            vuint32_t SUCC_CAL_CHK:1;
            vuint32_t FIL_CNT:8;
        } B;
    } CH0_CONFIG;

    uint8_t SRX_reserved7[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t CMPRSC:15;
            vuint32_t COMP_EN:1;
            vuint32_t:1;
            vuint32_t PRSC:14;
        } B;
    } CH1_CLK_CTRL;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t CAL_RESYNC:1;
            vuint32_t CAL_20_25:1;
            vuint32_t SMSG_OFLW:1;
            vuint32_t FMSG_OFLW:1;
            vuint32_t:1;
            vuint32_t PP_DIAG_ERR:1;
            vuint32_t CAL_LEN_ERR:1;
            vuint32_t CAL_DIAG_ERR:1;
            vuint32_t NIB_VAL_ERR:1;
            vuint32_t SMSG_CRC_ERR:1;
            vuint32_t FMSG_CRC_ERR:1;
            vuint32_t NUM_EDGES_ERR:1;
            vuint32_t:16;
        } B;
    } CH1_STATUS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t IE_CAL_RESYNC:1;
            vuint32_t IE_CAL_20_25:1;
            vuint32_t IE_SMSG_OFLW:1;
            vuint32_t IE_FMSG_OFLW:1;
            vuint32_t:1;
            vuint32_t IE_PP_DIAG_ERR:1;
            vuint32_t IE_CAL_LEN_ERR:1;
            vuint32_t IE_CAL_DIAG_ERR:1;
            vuint32_t IE_NIB_VAL_ERR:1;
            vuint32_t IE_SMSG_CRC_ERR:1;
            vuint32_t IE_FMSG_CRC_ERR:1;
            vuint32_t IE_NUM_EDGES_ERR:1;
            vuint32_t DCHNG_INT:1;
            vuint32_t CAL_RNG:1;
            vuint32_t PP_CHKSEL:1;
            vuint32_t FCRC_TYPE:1;
            vuint32_t FCRC_SC_EN:1;
            vuint32_t SCRC_TYPE:1;
            vuint32_t PAUSE_EN:1;
            vuint32_t SUCC_CAL_CHK:1;
            vuint32_t FIL_CNT:8;
        } B;
    } CH1_CONFIG;

    uint8_t SRX_reserved8[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t CMPRSC:15;
            vuint32_t COMP_EN:1;
            vuint32_t:1;
            vuint32_t PRSC:14;
        } B;
    } CH2_CLK_CTRL;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t CAL_RESYNC:1;
            vuint32_t CAL_20_25:1;
            vuint32_t SMSG_OFLW:1;
            vuint32_t FMSG_OFLW:1;
            vuint32_t:1;
            vuint32_t PP_DIAG_ERR:1;
            vuint32_t CAL_LEN_ERR:1;
            vuint32_t CAL_DIAG_ERR:1;
            vuint32_t NIB_VAL_ERR:1;
            vuint32_t SMSG_CRC_ERR:1;
            vuint32_t FMSG_CRC_ERR:1;
            vuint32_t NUM_EDGES_ERR:1;
            vuint32_t:16;
        } B;
    } CH2_STATUS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t IE_CAL_RESYNC:1;
            vuint32_t IE_CAL_20_25:1;
            vuint32_t IE_SMSG_OFLW:1;
            vuint32_t IE_FMSG_OFLW:1;
            vuint32_t:1;
            vuint32_t IE_PP_DIAG_ERR:1;
            vuint32_t IE_CAL_LEN_ERR:1;
            vuint32_t IE_CAL_DIAG_ERR:1;
            vuint32_t IE_NIB_VAL_ERR:1;
            vuint32_t IE_SMSG_CRC_ERR:1;
            vuint32_t IE_FMSG_CRC_ERR:1;
            vuint32_t IE_NUM_EDGES_ERR:1;
            vuint32_t DCHNG_INT:1;
            vuint32_t CAL_RNG:1;
            vuint32_t PP_CHKSEL:1;
            vuint32_t FCRC_TYPE:1;
            vuint32_t FCRC_SC_EN:1;
            vuint32_t SCRC_TYPE:1;
            vuint32_t PAUSE_EN:1;
            vuint32_t SUCC_CAL_CHK:1;
            vuint32_t FIL_CNT:8;
        } B;
    } CH2_CONFIG;

    uint8_t SRX_reserved9[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t CMPRSC:15;
            vuint32_t COMP_EN:1;
            vuint32_t:1;
            vuint32_t PRSC:14;
        } B;
    } CH3_CLK_CTRL;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t CAL_RESYNC:1;
            vuint32_t CAL_20_25:1;
            vuint32_t SMSG_OFLW:1;
            vuint32_t FMSG_OFLW:1;
            vuint32_t:1;
            vuint32_t PP_DIAG_ERR:1;
            vuint32_t CAL_LEN_ERR:1;
            vuint32_t CAL_DIAG_ERR:1;
            vuint32_t NIB_VAL_ERR:1;
            vuint32_t SMSG_CRC_ERR:1;
            vuint32_t FMSG_CRC_ERR:1;
            vuint32_t NUM_EDGES_ERR:1;
            vuint32_t:16;
        } B;
    } CH3_STATUS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t IE_CAL_RESYNC:1;
            vuint32_t IE_CAL_20_25:1;
            vuint32_t IE_SMSG_OFLW:1;
            vuint32_t IE_FMSG_OFLW:1;
            vuint32_t:1;
            vuint32_t IE_PP_DIAG_ERR:1;
            vuint32_t IE_CAL_LEN_ERR:1;
            vuint32_t IE_CAL_DIAG_ERR:1;
            vuint32_t IE_NIB_VAL_ERR:1;
            vuint32_t IE_SMSG_CRC_ERR:1;
            vuint32_t IE_FMSG_CRC_ERR:1;
            vuint32_t IE_NUM_EDGES_ERR:1;
            vuint32_t DCHNG_INT:1;
            vuint32_t CAL_RNG:1;
            vuint32_t PP_CHKSEL:1;
            vuint32_t FCRC_TYPE:1;
            vuint32_t FCRC_SC_EN:1;
            vuint32_t SCRC_TYPE:1;
            vuint32_t PAUSE_EN:1;
            vuint32_t SUCC_CAL_CHK:1;
            vuint32_t FIL_CNT:8;
        } B;
    } CH3_CONFIG;

    uint8_t SRX_reserved10[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t CMPRSC:15;
            vuint32_t COMP_EN:1;
            vuint32_t:1;
            vuint32_t PRSC:14;
        } B;
    } CH4_CLK_CTRL;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t CAL_RESYNC:1;
            vuint32_t CAL_20_25:1;
            vuint32_t SMSG_OFLW:1;
            vuint32_t FMSG_OFLW:1;
            vuint32_t:1;
            vuint32_t PP_DIAG_ERR:1;
            vuint32_t CAL_LEN_ERR:1;
            vuint32_t CAL_DIAG_ERR:1;
            vuint32_t NIB_VAL_ERR:1;
            vuint32_t SMSG_CRC_ERR:1;
            vuint32_t FMSG_CRC_ERR:1;
            vuint32_t NUM_EDGES_ERR:1;
            vuint32_t:16;
        } B;
    } CH4_STATUS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t IE_CAL_RESYNC:1;
            vuint32_t IE_CAL_20_25:1;
            vuint32_t IE_SMSG_OFLW:1;
            vuint32_t IE_FMSG_OFLW:1;
            vuint32_t:1;
            vuint32_t IE_PP_DIAG_ERR:1;
            vuint32_t IE_CAL_LEN_ERR:1;
            vuint32_t IE_CAL_DIAG_ERR:1;
            vuint32_t IE_NIB_VAL_ERR:1;
            vuint32_t IE_SMSG_CRC_ERR:1;
            vuint32_t IE_FMSG_CRC_ERR:1;
            vuint32_t IE_NUM_EDGES_ERR:1;
            vuint32_t DCHNG_INT:1;
            vuint32_t CAL_RNG:1;
            vuint32_t PP_CHKSEL:1;
            vuint32_t FCRC_TYPE:1;
            vuint32_t FCRC_SC_EN:1;
            vuint32_t SCRC_TYPE:1;
            vuint32_t PAUSE_EN:1;
            vuint32_t SUCC_CAL_CHK:1;
            vuint32_t FIL_CNT:8;
        } B;
    } CH4_CONFIG;

    uint8_t SRX_reserved11[180];

    union {
        vuint32_t R;
        struct {
            vuint32_t CHNUM:4;
            vuint32_t SCNIB:4;
            vuint32_t DNIB1:4;
            vuint32_t DNIB2:4;
            vuint32_t DNIB3:4;
            vuint32_t DNIB4:4;
            vuint32_t DNIB5:4;
            vuint32_t DNIB6:4;
        } B;
    } CH0_FMSG_DATA;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t CRC4b:4;
            vuint32_t:16;
        } B;
    } CH0_FMSG_CRC;

    union {
        vuint32_t R;
        struct {
            vuint32_t TS:32;
        } B;
    } CH0_FMSG_TS;

    union {
        vuint32_t R;
        struct {
            vuint32_t CHNUM:4;
            vuint32_t TYPE:1;
            vuint32_t:16;
            vuint32_t CFG:1;
            vuint32_t ID:4;
            vuint32_t:1;
            vuint32_t IDorDATA:4;
            vuint32_t:1;
        } B;
    } CH0_SMSG3;

    union {
        vuint32_t R;
        struct {
            vuint32_t:10;
            vuint32_t SMCRC:6;
            vuint32_t:4;
            vuint32_t DATA:12;
        } B;
    } CH0_SMSG2;

    union {
        vuint32_t R;
        struct {
            vuint32_t TS:32;
        } B;
    } CH0_SMSG_TS;

    union {
        vuint32_t R;
        struct {
            vuint32_t CHNUM:4;
            vuint32_t SCNIB:4;
            vuint32_t DNIB1:4;
            vuint32_t DNIB2:4;
            vuint32_t DNIB3:4;
            vuint32_t DNIB4:4;
            vuint32_t DNIB5:4;
            vuint32_t DNIB6:4;
        } B;
    } CH1_FMSG_DATA;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t CRC4b:4;
            vuint32_t:16;
        } B;
    } CH1_FMSG_CRC;

    union {
        vuint32_t R;
        struct {
            vuint32_t TS:32;
        } B;
    } CH1_FMSG_TS;

    union {
        vuint32_t R;
        struct {
            vuint32_t CHNUM:4;
            vuint32_t TYPE:1;
            vuint32_t:16;
            vuint32_t CFG:1;
            vuint32_t ID:4;
            vuint32_t:1;
            vuint32_t IDorDATA:4;
            vuint32_t:1;
        } B;
    } CH1_SMSG3;

    union {
        vuint32_t R;
        struct {
            vuint32_t:10;
            vuint32_t SMCRC:6;
            vuint32_t:4;
            vuint32_t DATA:12;
        } B;
    } CH1_SMSG2;

    union {
        vuint32_t R;
        struct {
            vuint32_t TS:32;
        } B;
    } CH1_SMSG_TS;

    union {
        vuint32_t R;
        struct {
            vuint32_t CHNUM:4;
            vuint32_t SCNIB:4;
            vuint32_t DNIB1:4;
            vuint32_t DNIB2:4;
            vuint32_t DNIB3:4;
            vuint32_t DNIB4:4;
            vuint32_t DNIB5:4;
            vuint32_t DNIB6:4;
        } B;
    } CH2_FMSG_DATA;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t CRC4b:4;
            vuint32_t:16;
        } B;
    } CH2_FMSG_CRC;

    union {
        vuint32_t R;
        struct {
            vuint32_t TS:32;
        } B;
    } CH2_FMSG_TS;

    union {
        vuint32_t R;
        struct {
            vuint32_t CHNUM:4;
            vuint32_t TYPE:1;
            vuint32_t:16;
            vuint32_t CFG:1;
            vuint32_t ID:4;
            vuint32_t:1;
            vuint32_t IDorDATA:4;
            vuint32_t:1;
        } B;
    } CH2_SMSG3;

    union {
        vuint32_t R;
        struct {
            vuint32_t:10;
            vuint32_t SMCRC:6;
            vuint32_t:4;
            vuint32_t DATA:12;
        } B;
    } CH2_SMSG2;

    union {
        vuint32_t R;
        struct {
            vuint32_t TS:32;
        } B;
    } CH2_SMSG_TS;

    union {
        vuint32_t R;
        struct {
            vuint32_t CHNUM:4;
            vuint32_t SCNIB:4;
            vuint32_t DNIB1:4;
            vuint32_t DNIB2:4;
            vuint32_t DNIB3:4;
            vuint32_t DNIB4:4;
            vuint32_t DNIB5:4;
            vuint32_t DNIB6:4;
        } B;
    } CH3_FMSG_DATA;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t CRC4b:4;
            vuint32_t:16;
        } B;
    } CH3_FMSG_CRC;

    union {
        vuint32_t R;
        struct {
            vuint32_t TS:32;
        } B;
    } CH3_FMSG_TS;

    union {
        vuint32_t R;
        struct {
            vuint32_t CHNUM:4;
            vuint32_t TYPE:1;
            vuint32_t:16;
            vuint32_t CFG:1;
            vuint32_t ID:4;
            vuint32_t:1;
            vuint32_t IDorDATA:4;
            vuint32_t:1;
        } B;
    } CH3_SMSG3;

    union {
        vuint32_t R;
        struct {
            vuint32_t:10;
            vuint32_t SMCRC:6;
            vuint32_t:4;
            vuint32_t DATA:12;
        } B;
    } CH3_SMSG2;

    union {
        vuint32_t R;
        struct {
            vuint32_t TS:32;
        } B;
    } CH3_SMSG_TS;

    union {
        vuint32_t R;
        struct {
            vuint32_t CHNUM:4;
            vuint32_t SCNIB:4;
            vuint32_t DNIB1:4;
            vuint32_t DNIB2:4;
            vuint32_t DNIB3:4;
            vuint32_t DNIB4:4;
            vuint32_t DNIB5:4;
            vuint32_t DNIB6:4;
        } B;
    } CH4_FMSG_DATA;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t CRC4b:4;
            vuint32_t:16;
        } B;
    } CH4_FMSG_CRC;

    union {
        vuint32_t R;
        struct {
            vuint32_t TS:32;
        } B;
    } CH4_FMSG_TS;

    union {
        vuint32_t R;
        struct {
            vuint32_t CHNUM:4;
            vuint32_t TYPE:1;
            vuint32_t:16;
            vuint32_t CFG:1;
            vuint32_t ID:4;
            vuint32_t:1;
            vuint32_t IDorDATA:4;
            vuint32_t:1;
        } B;
    } CH4_SMSG3;

    union {
        vuint32_t R;
        struct {
            vuint32_t:10;
            vuint32_t SMCRC:6;
            vuint32_t:4;
            vuint32_t DATA:12;
        } B;
    } CH4_SMSG2;

    union {
        vuint32_t R;
        struct {
            vuint32_t TS:32;
        } B;
    } CH4_SMSG_TS;
};
/**************************************************************************/
/*                   Module: SSCM                                         */
/**************************************************************************/
struct SSCM_tag {
    union {
        vuint16_t R;
        struct {
            vuint16_t:1;
            vuint16_t CER:1;
            vuint16_t:1;
            vuint16_t NXEN1:1;
            vuint16_t NXEN:1;
            vuint16_t:3;
            vuint16_t BMODE:3;
            vuint16_t VLE:1;
            vuint16_t:4;
        } B;
    } STATUS;

    union {
        vuint16_t R;
        struct {
            vuint16_t JPIN:10;
            vuint16_t:1;
            vuint16_t MREV:4;
            vuint16_t:1;
        } B;
    } MEMCONFIG;

    uint8_t SSCM_reserved1[2];

    union {
        vuint16_t R;
        struct {
            vuint16_t:14;
            vuint16_t PAE:1;
            vuint16_t RAE:1;
        } B;
    } ERROR;

    union {
        vuint16_t R;
        struct {
            vuint16_t:13;
            vuint16_t DEBUG_MODE:3;
        } B;
    } DEBUGPORT;

    uint8_t SSCM_reserved2[22];

    union {
        vuint32_t R;
        struct {
            vuint32_t:28;
            vuint32_t HSB:3;
            vuint32_t HSE:1;
        } B;
    } UOPS;

    uint8_t SSCM_reserved3[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t SADR:32;
        } B;
    } PSA;

    uint8_t SSCM_reserved4[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t HADR:32;
        } B;
    } HSA;
};
/**************************************************************************/
/*                   Module: STCU2                                        */
/**************************************************************************/
struct STCU2_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:21;
            vuint32_t BYP:1;
            vuint32_t MBPLLEN:1;
            vuint32_t LBPLLEN:1;
            vuint32_t:7;
            vuint32_t RUN:1;
        } B;
    } RUN;

    union {
        vuint32_t R;
        struct {
            vuint32_t:20;
            vuint32_t MBIE:1;
            vuint32_t LBIE:1;
            vuint32_t MBSWPLLEN:1;
            vuint32_t LBSWPLLEN:1;
            vuint32_t:6;
            vuint32_t RUNSW_ABORT:1;
            vuint32_t RUNSW:1;
        } B;
    } RUNSW;

    union {
        vuint32_t R;
        struct {
            vuint32_t SKC:32;
        } B;
    } SKC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t PTR:7;
            vuint32_t LB_DELAY:8;
            vuint32_t:7;
            vuint32_t WRP:1;
            vuint32_t:2;
            vuint32_t CRCEN:1;
            vuint32_t PMOSEN:1;
            vuint32_t CHBRD:1;
            vuint32_t CLK_CFG:3;
        } B;
    } CFG;

    union {
        vuint32_t R;
        struct {
            vuint32_t:2;
            vuint32_t PLLODF:6;
            vuint32_t:5;
            vuint32_t PLLIDF:3;
            vuint32_t:9;
            vuint32_t PLLLDF:7;
        } B;
    } PLL_CFG;

    union {
        vuint32_t R;
        struct {
            vuint32_t WDGEOC:32;
        } B;
    } WDG;

    union {
        vuint32_t R;
        struct {
            vuint32_t:30;
            vuint32_t MBIFLG:1;
            vuint32_t LBIFLG:1;
        } B;
    } INT_FLG;

    union {
        vuint32_t R;
        struct {
            vuint32_t CRCE:32;
        } B;
    } CRCE;

    union {
        vuint32_t R;
        struct {
            vuint32_t CRCR:32;
        } B;
    } CRCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t ABORTHW:1;
            vuint32_t ABORTSW:1;
            vuint32_t:3;
            vuint32_t LOCKESW:1;
            vuint32_t WDTOSW:1;
            vuint32_t CRCSSW:1;
            vuint32_t ENGESW:1;
            vuint32_t INVPSW:1;
            vuint32_t:6;
            vuint32_t CFSF:1;
            vuint32_t NCFSF:1;
            vuint32_t:3;
            vuint32_t LOCKE:1;
            vuint32_t WDTO:1;
            vuint32_t CRCS:1;
            vuint32_t ENGE:1;
            vuint32_t INVP:1;
        } B;
    } ERR_STAT;

    union {
        vuint32_t R;
        struct {
            vuint32_t:27;
            vuint32_t LOCKECFM:1;
            vuint32_t WDTOCFM:1;
            vuint32_t CRCSFM:1;
            vuint32_t ENGECFM:1;
            vuint32_t INVPCFM:1;
        } B;
    } ERR_FM;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t LBS15:1;
            vuint32_t LBS14:1;
            vuint32_t LBS13:1;
            vuint32_t LBS12:1;
            vuint32_t LBS11:1;
            vuint32_t LBS10:1;
            vuint32_t LBS9:1;
            vuint32_t LBS8:1;
            vuint32_t LBS7:1;
            vuint32_t LBS6:1;
            vuint32_t LBS5:1;
            vuint32_t LBS4:1;
            vuint32_t LBS3:1;
            vuint32_t LBS2:1;
            vuint32_t LBS1:1;
            vuint32_t LBS0:1;
        } B;
    } LBS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t LBE15:1;
            vuint32_t LBE14:1;
            vuint32_t LBE13:1;
            vuint32_t LBE12:1;
            vuint32_t LBE11:1;
            vuint32_t LBE10:1;
            vuint32_t LBE9:1;
            vuint32_t LBE8:1;
            vuint32_t LBE7:1;
            vuint32_t LBE6:1;
            vuint32_t LBE5:1;
            vuint32_t LBE4:1;
            vuint32_t LBE3:1;
            vuint32_t LBE2:1;
            vuint32_t LBE1:1;
            vuint32_t LBE0:1;
        } B;
    } LBE;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t LBSSW15:1;
            vuint32_t LBSSW14:1;
            vuint32_t LBSSW13:1;
            vuint32_t LBSSW12:1;
            vuint32_t LBSSW11:1;
            vuint32_t LBSSW10:1;
            vuint32_t LBSSW9:1;
            vuint32_t LBSSW8:1;
            vuint32_t LBSSW7:1;
            vuint32_t LBSSW6:1;
            vuint32_t LBSSW5:1;
            vuint32_t LBSSW4:1;
            vuint32_t LBSSW3:1;
            vuint32_t LBSSW2:1;
            vuint32_t LBSSW1:1;
            vuint32_t LBSSW0:1;
        } B;
    } LBSSW;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t LBESW15:1;
            vuint32_t LBESW14:1;
            vuint32_t LBESW13:1;
            vuint32_t LBESW12:1;
            vuint32_t LBESW11:1;
            vuint32_t LBESW10:1;
            vuint32_t LBESW9:1;
            vuint32_t LBESW8:1;
            vuint32_t LBESW7:1;
            vuint32_t LBESW6:1;
            vuint32_t LBESW5:1;
            vuint32_t LBESW4:1;
            vuint32_t LBESW3:1;
            vuint32_t LBESW2:1;
            vuint32_t LBESW1:1;
            vuint32_t LBESW0:1;
        } B;
    } LBESW;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t LBRMSW15:1;
            vuint32_t LBRMSW14:1;
            vuint32_t LBRMSW13:1;
            vuint32_t LBRMSW12:1;
            vuint32_t LBRMSW11:1;
            vuint32_t LBRMSW10:1;
            vuint32_t LBRMSW9:1;
            vuint32_t LBRMSW8:1;
            vuint32_t LBRMSW7:1;
            vuint32_t LBRMSW6:1;
            vuint32_t LBRMSW5:1;
            vuint32_t LBRMSW4:1;
            vuint32_t LBRMSW3:1;
            vuint32_t LBRMSW2:1;
            vuint32_t LBRMSW1:1;
            vuint32_t LBRMSW0:1;
        } B;
    } LBRMSW;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t LBCFM15:1;
            vuint32_t LBCFM14:1;
            vuint32_t LBCFM13:1;
            vuint32_t LBCFM12:1;
            vuint32_t LBCFM11:1;
            vuint32_t LBCFM10:1;
            vuint32_t LBCFM9:1;
            vuint32_t LBCFM8:1;
            vuint32_t LBCFM7:1;
            vuint32_t LBCFM6:1;
            vuint32_t LBCFM5:1;
            vuint32_t LBCFM4:1;
            vuint32_t LBCFM3:1;
            vuint32_t LBCFM2:1;
            vuint32_t LBCFM1:1;
            vuint32_t LBCFM0:1;
        } B;
    } LBCFM;

    union {
        vuint32_t R;
        struct {
            vuint32_t MBS31:1;
            vuint32_t MBS30:1;
            vuint32_t MBS29:1;
            vuint32_t MBS28:1;
            vuint32_t MBS27:1;
            vuint32_t MBS26:1;
            vuint32_t MBS25:1;
            vuint32_t MBS24:1;
            vuint32_t MBS23:1;
            vuint32_t MBS22:1;
            vuint32_t MBS21:1;
            vuint32_t MBS20:1;
            vuint32_t MBS19:1;
            vuint32_t MBS18:1;
            vuint32_t MBS17:1;
            vuint32_t MBS16:1;
            vuint32_t MBS15:1;
            vuint32_t MBS14:1;
            vuint32_t MBS13:1;
            vuint32_t MBS12:1;
            vuint32_t MBS11:1;
            vuint32_t MBS10:1;
            vuint32_t MBS9:1;
            vuint32_t MBS8:1;
            vuint32_t MBS7:1;
            vuint32_t MBS6:1;
            vuint32_t MBS5:1;
            vuint32_t MBS4:1;
            vuint32_t MBS3:1;
            vuint32_t MBS2:1;
            vuint32_t MBS1:1;
            vuint32_t MBS0:1;
        } B;
    } MBSL;

    union {
        vuint32_t R;
        struct {
            vuint32_t MBS63:1;
            vuint32_t MBS62:1;
            vuint32_t MBS61:1;
            vuint32_t MBS60:1;
            vuint32_t MBS59:1;
            vuint32_t MBS58:1;
            vuint32_t MBS57:1;
            vuint32_t MBS56:1;
            vuint32_t MBS55:1;
            vuint32_t MBS54:1;
            vuint32_t MBS53:1;
            vuint32_t MBS52:1;
            vuint32_t MBS51:1;
            vuint32_t MBS50:1;
            vuint32_t MBS49:1;
            vuint32_t MBS48:1;
            vuint32_t MBS47:1;
            vuint32_t MBS46:1;
            vuint32_t MBS45:1;
            vuint32_t MBS44:1;
            vuint32_t MBS43:1;
            vuint32_t MBS42:1;
            vuint32_t MBS41:1;
            vuint32_t MBS40:1;
            vuint32_t MBS39:1;
            vuint32_t MBS38:1;
            vuint32_t MBS37:1;
            vuint32_t MBS36:1;
            vuint32_t MBS35:1;
            vuint32_t MBS34:1;
            vuint32_t MBS33:1;
            vuint32_t MBS32:1;
        } B;
    } MBSM;

    union {
        vuint32_t R;
        struct {
            vuint32_t MBS95:1;
            vuint32_t MBS94:1;
            vuint32_t MBS93:1;
            vuint32_t MBS92:1;
            vuint32_t MBS91:1;
            vuint32_t MBS90:1;
            vuint32_t MBS89:1;
            vuint32_t MBS88:1;
            vuint32_t MBS87:1;
            vuint32_t MBS86:1;
            vuint32_t MBS85:1;
            vuint32_t MBS84:1;
            vuint32_t MBS83:1;
            vuint32_t MBS82:1;
            vuint32_t MBS81:1;
            vuint32_t MBS80:1;
            vuint32_t MBS79:1;
            vuint32_t MBS78:1;
            vuint32_t MBS77:1;
            vuint32_t MBS76:1;
            vuint32_t MBS75:1;
            vuint32_t MBS74:1;
            vuint32_t MBS73:1;
            vuint32_t MBS72:1;
            vuint32_t MBS71:1;
            vuint32_t MBS70:1;
            vuint32_t MBS69:1;
            vuint32_t MBS68:1;
            vuint32_t MBS67:1;
            vuint32_t MBS66:1;
            vuint32_t MBS65:1;
            vuint32_t MBS64:1;
        } B;
    } MBSH;

    union {
        vuint32_t R;
        struct {
            vuint32_t MBE31:1;
            vuint32_t MBE30:1;
            vuint32_t MBE29:1;
            vuint32_t MBE28:1;
            vuint32_t MBE27:1;
            vuint32_t MBE26:1;
            vuint32_t MBE25:1;
            vuint32_t MBE24:1;
            vuint32_t MBE23:1;
            vuint32_t MBE22:1;
            vuint32_t MBE21:1;
            vuint32_t MBE20:1;
            vuint32_t MBE19:1;
            vuint32_t MBE18:1;
            vuint32_t MBE17:1;
            vuint32_t MBE16:1;
            vuint32_t MBE15:1;
            vuint32_t MBE14:1;
            vuint32_t MBE13:1;
            vuint32_t MBE12:1;
            vuint32_t MBE11:1;
            vuint32_t MBE10:1;
            vuint32_t MBE9:1;
            vuint32_t MBE8:1;
            vuint32_t MBE7:1;
            vuint32_t MBE6:1;
            vuint32_t MBE5:1;
            vuint32_t MBE4:1;
            vuint32_t MBE3:1;
            vuint32_t MBE2:1;
            vuint32_t MBE1:1;
            vuint32_t MBE0:1;
        } B;
    } MBEL;

    union {
        vuint32_t R;
        struct {
            vuint32_t MBE63:1;
            vuint32_t MBE62:1;
            vuint32_t MBE61:1;
            vuint32_t MBE60:1;
            vuint32_t MBE59:1;
            vuint32_t MBE58:1;
            vuint32_t MBE57:1;
            vuint32_t MBE56:1;
            vuint32_t MBE55:1;
            vuint32_t MBE54:1;
            vuint32_t MBE53:1;
            vuint32_t MBE52:1;
            vuint32_t MBE51:1;
            vuint32_t MBE50:1;
            vuint32_t MBE49:1;
            vuint32_t MBE48:1;
            vuint32_t MBE47:1;
            vuint32_t MBE46:1;
            vuint32_t MBE45:1;
            vuint32_t MBE44:1;
            vuint32_t MBE43:1;
            vuint32_t MBE42:1;
            vuint32_t MBE41:1;
            vuint32_t MBE40:1;
            vuint32_t MBE39:1;
            vuint32_t MBE38:1;
            vuint32_t MBE37:1;
            vuint32_t MBE36:1;
            vuint32_t MBE35:1;
            vuint32_t MBE34:1;
            vuint32_t MBE33:1;
            vuint32_t MBE32:1;
        } B;
    } MBEM;

    union {
        vuint32_t R;
        struct {
            vuint32_t MBE95:1;
            vuint32_t MBE94:1;
            vuint32_t MBE93:1;
            vuint32_t MBE92:1;
            vuint32_t MBE91:1;
            vuint32_t MBE90:1;
            vuint32_t MBE89:1;
            vuint32_t MBE88:1;
            vuint32_t MBE87:1;
            vuint32_t MBE86:1;
            vuint32_t MBE85:1;
            vuint32_t MBE84:1;
            vuint32_t MBE83:1;
            vuint32_t MBE82:1;
            vuint32_t MBE81:1;
            vuint32_t MBE80:1;
            vuint32_t MBE79:1;
            vuint32_t MBE78:1;
            vuint32_t MBE77:1;
            vuint32_t MBE76:1;
            vuint32_t MBE75:1;
            vuint32_t MBE74:1;
            vuint32_t MBE73:1;
            vuint32_t MBE72:1;
            vuint32_t MBE71:1;
            vuint32_t MBE70:1;
            vuint32_t MBE69:1;
            vuint32_t MBE68:1;
            vuint32_t MBE67:1;
            vuint32_t MBE66:1;
            vuint32_t MBE65:1;
            vuint32_t MBE64:1;
        } B;
    } MBEH;

    union {
        vuint32_t R;
        struct {
            vuint32_t MBSSW31:1;
            vuint32_t MBSSW30:1;
            vuint32_t MBSSW29:1;
            vuint32_t MBSSW28:1;
            vuint32_t MBSSW27:1;
            vuint32_t MBSSW26:1;
            vuint32_t MBSSW25:1;
            vuint32_t MBSSW24:1;
            vuint32_t MBSSW23:1;
            vuint32_t MBSSW22:1;
            vuint32_t MBSSW21:1;
            vuint32_t MBSSW20:1;
            vuint32_t MBSSW19:1;
            vuint32_t MBSSW18:1;
            vuint32_t MBSSW17:1;
            vuint32_t MBSSW16:1;
            vuint32_t MBSSW15:1;
            vuint32_t MBSSW14:1;
            vuint32_t MBSSW13:1;
            vuint32_t MBSSW12:1;
            vuint32_t MBSSW11:1;
            vuint32_t MBSSW10:1;
            vuint32_t MBSSW9:1;
            vuint32_t MBSSW8:1;
            vuint32_t MBSSW7:1;
            vuint32_t MBSSW6:1;
            vuint32_t MBSSW5:1;
            vuint32_t MBSSW4:1;
            vuint32_t MBSSW3:1;
            vuint32_t MBSSW2:1;
            vuint32_t MBSSW1:1;
            vuint32_t MBSSW0:1;
        } B;
    } MBSLSW;

    union {
        vuint32_t R;
        struct {
            vuint32_t MBSSW63:1;
            vuint32_t MBSSW62:1;
            vuint32_t MBSSW61:1;
            vuint32_t MBSSW60:1;
            vuint32_t MBSSW59:1;
            vuint32_t MBSSW58:1;
            vuint32_t MBSSW57:1;
            vuint32_t MBSSW56:1;
            vuint32_t MBSSW55:1;
            vuint32_t MBSSW54:1;
            vuint32_t MBSSW53:1;
            vuint32_t MBSSW52:1;
            vuint32_t MBSSW51:1;
            vuint32_t MBSSW50:1;
            vuint32_t MBSSW49:1;
            vuint32_t MBSSW48:1;
            vuint32_t MBSSW47:1;
            vuint32_t MBSSW46:1;
            vuint32_t MBSSW45:1;
            vuint32_t MBSSW44:1;
            vuint32_t MBSSW43:1;
            vuint32_t MBSSW42:1;
            vuint32_t MBSSW41:1;
            vuint32_t MBSSW40:1;
            vuint32_t MBSSW39:1;
            vuint32_t MBSSW38:1;
            vuint32_t MBSSW37:1;
            vuint32_t MBSSW36:1;
            vuint32_t MBSSW35:1;
            vuint32_t MBSSW34:1;
            vuint32_t MBSSW33:1;
            vuint32_t MBSSW32:1;
        } B;
    } MBSMSW;

    union {
        vuint32_t R;
        struct {
            vuint32_t MBSSW95:1;
            vuint32_t MBSSW94:1;
            vuint32_t MBSSW93:1;
            vuint32_t MBSSW92:1;
            vuint32_t MBSSW91:1;
            vuint32_t MBSSW90:1;
            vuint32_t MBSSW89:1;
            vuint32_t MBSSW88:1;
            vuint32_t MBSSW87:1;
            vuint32_t MBSSW86:1;
            vuint32_t MBSSW85:1;
            vuint32_t MBSSW84:1;
            vuint32_t MBSSW83:1;
            vuint32_t MBSSW82:1;
            vuint32_t MBSSW81:1;
            vuint32_t MBSSW80:1;
            vuint32_t MBSSW79:1;
            vuint32_t MBSSW78:1;
            vuint32_t MBSSW77:1;
            vuint32_t MBSSW76:1;
            vuint32_t MBSSW75:1;
            vuint32_t MBSSW74:1;
            vuint32_t MBSSW73:1;
            vuint32_t MBSSW72:1;
            vuint32_t MBSSW71:1;
            vuint32_t MBSSW70:1;
            vuint32_t MBSSW69:1;
            vuint32_t MBSSW68:1;
            vuint32_t MBSSW67:1;
            vuint32_t MBSSW66:1;
            vuint32_t MBSSW65:1;
            vuint32_t MBSSW64:1;
        } B;
    } MBSHSW;

    union {
        vuint32_t R;
        struct {
            vuint32_t MBESW31:1;
            vuint32_t MBESW30:1;
            vuint32_t MBESW29:1;
            vuint32_t MBESW28:1;
            vuint32_t MBESW27:1;
            vuint32_t MBESW26:1;
            vuint32_t MBESW25:1;
            vuint32_t MBESW24:1;
            vuint32_t MBESW23:1;
            vuint32_t MBESW22:1;
            vuint32_t MBESW21:1;
            vuint32_t MBESW20:1;
            vuint32_t MBESW19:1;
            vuint32_t MBESW18:1;
            vuint32_t MBESW17:1;
            vuint32_t MBESW16:1;
            vuint32_t MBESW15:1;
            vuint32_t MBESW14:1;
            vuint32_t MBESW13:1;
            vuint32_t MBESW12:1;
            vuint32_t MBESW11:1;
            vuint32_t MBESW10:1;
            vuint32_t MBESW9:1;
            vuint32_t MBESW8:1;
            vuint32_t MBESW7:1;
            vuint32_t MBESW6:1;
            vuint32_t MBESW5:1;
            vuint32_t MBESW4:1;
            vuint32_t MBESW3:1;
            vuint32_t MBESW2:1;
            vuint32_t MBESW1:1;
            vuint32_t MBESW0:1;
        } B;
    } MBELSW;

    union {
        vuint32_t R;
        struct {
            vuint32_t MBESW63:1;
            vuint32_t MBESW62:1;
            vuint32_t MBESW61:1;
            vuint32_t MBESW60:1;
            vuint32_t MBESW59:1;
            vuint32_t MBESW58:1;
            vuint32_t MBESW57:1;
            vuint32_t MBESW56:1;
            vuint32_t MBESW55:1;
            vuint32_t MBESW54:1;
            vuint32_t MBESW53:1;
            vuint32_t MBESW52:1;
            vuint32_t MBESW51:1;
            vuint32_t MBESW50:1;
            vuint32_t MBESW49:1;
            vuint32_t MBESW48:1;
            vuint32_t MBESW47:1;
            vuint32_t MBESW46:1;
            vuint32_t MBESW45:1;
            vuint32_t MBESW44:1;
            vuint32_t MBESW43:1;
            vuint32_t MBESW42:1;
            vuint32_t MBESW41:1;
            vuint32_t MBESW40:1;
            vuint32_t MBESW39:1;
            vuint32_t MBESW38:1;
            vuint32_t MBESW37:1;
            vuint32_t MBESW36:1;
            vuint32_t MBESW35:1;
            vuint32_t MBESW34:1;
            vuint32_t MBESW33:1;
            vuint32_t MBESW32:1;
        } B;
    } MBEMSW;

    union {
        vuint32_t R;
        struct {
            vuint32_t MBESW95:1;
            vuint32_t MBESW94:1;
            vuint32_t MBESW93:1;
            vuint32_t MBESW92:1;
            vuint32_t MBESW91:1;
            vuint32_t MBESW90:1;
            vuint32_t MBESW89:1;
            vuint32_t MBESW88:1;
            vuint32_t MBESW87:1;
            vuint32_t MBESW86:1;
            vuint32_t MBESW85:1;
            vuint32_t MBESW84:1;
            vuint32_t MBESW83:1;
            vuint32_t MBESW82:1;
            vuint32_t MBESW81:1;
            vuint32_t MBESW80:1;
            vuint32_t MBESW79:1;
            vuint32_t MBESW78:1;
            vuint32_t MBESW77:1;
            vuint32_t MBESW76:1;
            vuint32_t MBESW75:1;
            vuint32_t MBESW74:1;
            vuint32_t MBESW73:1;
            vuint32_t MBESW72:1;
            vuint32_t MBESW71:1;
            vuint32_t MBESW70:1;
            vuint32_t MBESW69:1;
            vuint32_t MBESW68:1;
            vuint32_t MBESW67:1;
            vuint32_t MBESW66:1;
            vuint32_t MBESW65:1;
            vuint32_t MBESW64:1;
        } B;
    } MBEHSW;

    union {
        vuint32_t R;
        struct {
            vuint32_t MBCFM31:1;
            vuint32_t MBCFM30:1;
            vuint32_t MBCFM29:1;
            vuint32_t MBCFM28:1;
            vuint32_t MBCFM27:1;
            vuint32_t MBCFM26:1;
            vuint32_t MBCFM25:1;
            vuint32_t MBCFM24:1;
            vuint32_t MBCFM23:1;
            vuint32_t MBCFM22:1;
            vuint32_t MBCFM21:1;
            vuint32_t MBCFM20:1;
            vuint32_t MBCFM19:1;
            vuint32_t MBCFM18:1;
            vuint32_t MBCFM17:1;
            vuint32_t MBCFM16:1;
            vuint32_t MBCFM15:1;
            vuint32_t MBCFM14:1;
            vuint32_t MBCFM13:1;
            vuint32_t MBCFM12:1;
            vuint32_t MBCFM11:1;
            vuint32_t MBCFM10:1;
            vuint32_t MBCFM9:1;
            vuint32_t MBCFM8:1;
            vuint32_t MBCFM7:1;
            vuint32_t MBCFM6:1;
            vuint32_t MBCFM5:1;
            vuint32_t MBCFM4:1;
            vuint32_t MBCFM3:1;
            vuint32_t MBCFM2:1;
            vuint32_t MBCFM1:1;
            vuint32_t MBCFM0:1;
        } B;
    } MBCFML;

    union {
        vuint32_t R;
        struct {
            vuint32_t MBCFM63:1;
            vuint32_t MBCFM62:1;
            vuint32_t MBCFM61:1;
            vuint32_t MBCFM60:1;
            vuint32_t MBCFM59:1;
            vuint32_t MBCFM58:1;
            vuint32_t MBCFM57:1;
            vuint32_t MBCFM56:1;
            vuint32_t MBCFM55:1;
            vuint32_t MBCFM54:1;
            vuint32_t MBCFM53:1;
            vuint32_t MBCFM52:1;
            vuint32_t MBCFM51:1;
            vuint32_t MBCFM50:1;
            vuint32_t MBCFM49:1;
            vuint32_t MBCFM48:1;
            vuint32_t MBCFM47:1;
            vuint32_t MBCFM46:1;
            vuint32_t MBCFM45:1;
            vuint32_t MBCFM44:1;
            vuint32_t MBCFM43:1;
            vuint32_t MBCFM42:1;
            vuint32_t MBCFM41:1;
            vuint32_t MBCFM40:1;
            vuint32_t MBCFM39:1;
            vuint32_t MBCFM38:1;
            vuint32_t MBCFM37:1;
            vuint32_t MBCFM36:1;
            vuint32_t MBCFM35:1;
            vuint32_t MBCFM34:1;
            vuint32_t MBCFM33:1;
            vuint32_t MBCFM32:1;
        } B;
    } MBCFMM;

    union {
        vuint32_t R;
        struct {
            vuint32_t MBCFM95:1;
            vuint32_t MBCFM94:1;
            vuint32_t MBCFM93:1;
            vuint32_t MBCFM92:1;
            vuint32_t MBCFM91:1;
            vuint32_t MBCFM90:1;
            vuint32_t MBCFM89:1;
            vuint32_t MBCFM88:1;
            vuint32_t MBCFM87:1;
            vuint32_t MBCFM86:1;
            vuint32_t MBCFM85:1;
            vuint32_t MBCFM84:1;
            vuint32_t MBCFM83:1;
            vuint32_t MBCFM82:1;
            vuint32_t MBCFM81:1;
            vuint32_t MBCFM80:1;
            vuint32_t MBCFM79:1;
            vuint32_t MBCFM78:1;
            vuint32_t MBCFM77:1;
            vuint32_t MBCFM76:1;
            vuint32_t MBCFM75:1;
            vuint32_t MBCFM74:1;
            vuint32_t MBCFM73:1;
            vuint32_t MBCFM72:1;
            vuint32_t MBCFM71:1;
            vuint32_t MBCFM70:1;
            vuint32_t MBCFM69:1;
            vuint32_t MBCFM68:1;
            vuint32_t MBCFM67:1;
            vuint32_t MBCFM66:1;
            vuint32_t MBCFM65:1;
            vuint32_t MBCFM64:1;
        } B;
    } MBCFMH;

    uint8_t STCU2_reserved1[704];

    struct {
        union {
            vuint32_t R;
            struct {
                vuint32_t CSM:1;
                vuint32_t PTR:7;
                vuint32_t:4;
                vuint32_t PRPGEN:1;
                vuint32_t SHS:3;
                vuint32_t SCEN_OFF:4;
                vuint32_t SCEN_ON:4;
                vuint32_t:4;
                vuint32_t PFT:1;
                vuint32_t CWS:3;
            } B;
        } LB_CTRL;

        union {
            vuint32_t R;
            struct {
                vuint32_t:6;
                vuint32_t PCS:26;
            } B;
        } LB_PCS;

        union {
            vuint32_t R;
            struct {
                vuint32_t PRPG31:1;
                vuint32_t PRPG30:1;
                vuint32_t PRPG29:1;
                vuint32_t PRPG28:1;
                vuint32_t PRPG27:1;
                vuint32_t PRPG26:1;
                vuint32_t PRPG25:1;
                vuint32_t PRPG24:1;
                vuint32_t PRPG23:1;
                vuint32_t PRPG22:1;
                vuint32_t PRPG21:1;
                vuint32_t PRPG20:1;
                vuint32_t PRPG19:1;
                vuint32_t PRPG18:1;
                vuint32_t PRPG17:1;
                vuint32_t PRPG16:1;
                vuint32_t PRPG15:1;
                vuint32_t PRPG14:1;
                vuint32_t PRPG13:1;
                vuint32_t PRPG12:1;
                vuint32_t PRPG11:1;
                vuint32_t PRPG10:1;
                vuint32_t PRPG9:1;
                vuint32_t PRPG8:1;
                vuint32_t PRPG7:1;
                vuint32_t PRPG6:1;
                vuint32_t PRPG5:1;
                vuint32_t PRPG4:1;
                vuint32_t PRPG3:1;
                vuint32_t PRPG2:1;
                vuint32_t PRPG1:1;
                vuint32_t PRPG0:1;
            } B;
        } LB_PRPGL;

        union {
            vuint32_t R;
            struct {
                vuint32_t PRPG63:1;
                vuint32_t PRPG62:1;
                vuint32_t PRPG61:1;
                vuint32_t PRPG60:1;
                vuint32_t PRPG59:1;
                vuint32_t PRPG58:1;
                vuint32_t PRPG57:1;
                vuint32_t PRPG56:1;
                vuint32_t PRPG55:1;
                vuint32_t PRPG54:1;
                vuint32_t PRPG53:1;
                vuint32_t PRPG52:1;
                vuint32_t PRPG51:1;
                vuint32_t PRPG50:1;
                vuint32_t PRPG49:1;
                vuint32_t PRPG48:1;
                vuint32_t PRPG47:1;
                vuint32_t PRPG46:1;
                vuint32_t PRPG45:1;
                vuint32_t PRPG44:1;
                vuint32_t PRPG43:1;
                vuint32_t PRPG42:1;
                vuint32_t PRPG41:1;
                vuint32_t PRPG40:1;
                vuint32_t PRPG39:1;
                vuint32_t PRPG38:1;
                vuint32_t PRPG37:1;
                vuint32_t PRPG36:1;
                vuint32_t PRPG35:1;
                vuint32_t PRPG34:1;
                vuint32_t PRPG33:1;
                vuint32_t PRPG32:1;
            } B;
        } LB_PRPGH;

        union {
            vuint32_t R;
            struct {
                vuint32_t MISREx:32;
            } B;
        } LB_MISREL;

        union {
            vuint32_t R;
            struct {
                vuint32_t MISREx:32;
            } B;
        } LB_MISREH;

        union {
            vuint32_t R;
            struct {
                vuint32_t MISRR31:1;
                vuint32_t MISRR30:1;
                vuint32_t MISRR29:1;
                vuint32_t MISRR28:1;
                vuint32_t MISRR27:1;
                vuint32_t MISRR26:1;
                vuint32_t MISRR25:1;
                vuint32_t MISRR24:1;
                vuint32_t MISRR23:1;
                vuint32_t MISRR22:1;
                vuint32_t MISRR21:1;
                vuint32_t MISRR20:1;
                vuint32_t MISRR19:1;
                vuint32_t MISRR18:1;
                vuint32_t MISRR17:1;
                vuint32_t MISRR16:1;
                vuint32_t MISRR15:1;
                vuint32_t MISRR14:1;
                vuint32_t MISRR13:1;
                vuint32_t MISRR12:1;
                vuint32_t MISRR11:1;
                vuint32_t MISRR10:1;
                vuint32_t MISRR9:1;
                vuint32_t MISRR8:1;
                vuint32_t MISRR7:1;
                vuint32_t MISRR6:1;
                vuint32_t MISRR5:1;
                vuint32_t MISRR4:1;
                vuint32_t MISRR3:1;
                vuint32_t MISRR2:1;
                vuint32_t MISRR1:1;
                vuint32_t MISRR0:1;
            } B;
        } LB_MISRRL;

        union {
            vuint32_t R;
            struct {
                vuint32_t MISRR63:1;
                vuint32_t MISRR62:1;
                vuint32_t MISRR61:1;
                vuint32_t MISRR60:1;
                vuint32_t MISRR59:1;
                vuint32_t MISRR58:1;
                vuint32_t MISRR57:1;
                vuint32_t MISRR56:1;
                vuint32_t MISRR55:1;
                vuint32_t MISRR54:1;
                vuint32_t MISRR53:1;
                vuint32_t MISRR52:1;
                vuint32_t MISRR51:1;
                vuint32_t MISRR50:1;
                vuint32_t MISRR49:1;
                vuint32_t MISRR48:1;
                vuint32_t MISRR47:1;
                vuint32_t MISRR46:1;
                vuint32_t MISRR45:1;
                vuint32_t MISRR44:1;
                vuint32_t MISRR43:1;
                vuint32_t MISRR42:1;
                vuint32_t MISRR41:1;
                vuint32_t MISRR40:1;
                vuint32_t MISRR39:1;
                vuint32_t MISRR38:1;
                vuint32_t MISRR37:1;
                vuint32_t MISRR36:1;
                vuint32_t MISRR35:1;
                vuint32_t MISRR34:1;
                vuint32_t MISRR33:1;
                vuint32_t MISRR32:1;
            } B;
        } LB_MISRRH;

        union {
            vuint32_t R;
            struct {
                vuint32_t MISRESW31:1;
                vuint32_t MISRESW30:1;
                vuint32_t MISRESW29:1;
                vuint32_t MISRESW28:1;
                vuint32_t MISRESW27:1;
                vuint32_t MISRESW26:1;
                vuint32_t MISRESW25:1;
                vuint32_t MISRESW24:1;
                vuint32_t MISRESW23:1;
                vuint32_t MISRESW22:1;
                vuint32_t MISRESW21:1;
                vuint32_t MISRESW20:1;
                vuint32_t MISRESW19:1;
                vuint32_t MISRESW18:1;
                vuint32_t MISRESW17:1;
                vuint32_t MISRESW16:1;
                vuint32_t MISRESW15:1;
                vuint32_t MISRESW14:1;
                vuint32_t MISRESW13:1;
                vuint32_t MISRESW12:1;
                vuint32_t MISRESW11:1;
                vuint32_t MISRESW10:1;
                vuint32_t MISRESW9:1;
                vuint32_t MISRESW8:1;
                vuint32_t MISRESW7:1;
                vuint32_t MISRESW6:1;
                vuint32_t MISRESW5:1;
                vuint32_t MISRESW4:1;
                vuint32_t MISRESW3:1;
                vuint32_t MISRESW2:1;
                vuint32_t MISRESW1:1;
                vuint32_t MISRESW0:1;
            } B;
        } LB_MISRELSW;

        union {
            vuint32_t R;
            struct {
                vuint32_t MISRESW63:1;
                vuint32_t MISRESW62:1;
                vuint32_t MISRESW61:1;
                vuint32_t MISRESW60:1;
                vuint32_t MISRESW59:1;
                vuint32_t MISRESW58:1;
                vuint32_t MISRESW57:1;
                vuint32_t MISRESW56:1;
                vuint32_t MISRESW55:1;
                vuint32_t MISRESW54:1;
                vuint32_t MISRESW53:1;
                vuint32_t MISRESW52:1;
                vuint32_t MISRESW51:1;
                vuint32_t MISRESW50:1;
                vuint32_t MISRESW49:1;
                vuint32_t MISRESW48:1;
                vuint32_t MISRESW47:1;
                vuint32_t MISRESW46:1;
                vuint32_t MISRESW45:1;
                vuint32_t MISRESW44:1;
                vuint32_t MISRESW43:1;
                vuint32_t MISRESW42:1;
                vuint32_t MISRESW41:1;
                vuint32_t MISRESW40:1;
                vuint32_t MISRESW39:1;
                vuint32_t MISRESW38:1;
                vuint32_t MISRESW37:1;
                vuint32_t MISRESW36:1;
                vuint32_t MISRESW35:1;
                vuint32_t MISRESW34:1;
                vuint32_t MISRESW33:1;
                vuint32_t MISRESW32:1;
            } B;
        } LB_MISREHSW;

        union {
            vuint32_t R;
            struct {
                vuint32_t MISRRSW31:1;
                vuint32_t MISRRSW30:1;
                vuint32_t MISRRSW29:1;
                vuint32_t MISRRSW28:1;
                vuint32_t MISRRSW27:1;
                vuint32_t MISRRSW26:1;
                vuint32_t MISRRSW25:1;
                vuint32_t MISRRSW24:1;
                vuint32_t MISRRSW23:1;
                vuint32_t MISRRSW22:1;
                vuint32_t MISRRSW21:1;
                vuint32_t MISRRSW20:1;
                vuint32_t MISRRSW19:1;
                vuint32_t MISRRSW18:1;
                vuint32_t MISRRSW17:1;
                vuint32_t MISRRSW16:1;
                vuint32_t MISRRSW15:1;
                vuint32_t MISRRSW14:1;
                vuint32_t MISRRSW13:1;
                vuint32_t MISRRSW12:1;
                vuint32_t MISRRSW11:1;
                vuint32_t MISRRSW10:1;
                vuint32_t MISRRSW9:1;
                vuint32_t MISRRSW8:1;
                vuint32_t MISRRSW7:1;
                vuint32_t MISRRSW6:1;
                vuint32_t MISRRSW5:1;
                vuint32_t MISRRSW4:1;
                vuint32_t MISRRSW3:1;
                vuint32_t MISRRSW2:1;
                vuint32_t MISRRSW1:1;
                vuint32_t MISRRSW0:1;
            } B;
        } LB_MISRRLSW;

        union {
            vuint32_t R;
            struct {
                vuint32_t MISRRSW63:1;
                vuint32_t MISRRSW62:1;
                vuint32_t MISRRSW61:1;
                vuint32_t MISRRSW60:1;
                vuint32_t MISRRSW59:1;
                vuint32_t MISRRSW58:1;
                vuint32_t MISRRSW57:1;
                vuint32_t MISRRSW56:1;
                vuint32_t MISRRSW55:1;
                vuint32_t MISRRSW54:1;
                vuint32_t MISRRSW53:1;
                vuint32_t MISRRSW52:1;
                vuint32_t MISRRSW51:1;
                vuint32_t MISRRSW50:1;
                vuint32_t MISRRSW49:1;
                vuint32_t MISRRSW48:1;
                vuint32_t MISRRSW47:1;
                vuint32_t MISRRSW46:1;
                vuint32_t MISRRSW45:1;
                vuint32_t MISRRSW44:1;
                vuint32_t MISRRSW43:1;
                vuint32_t MISRR42:1;
                vuint32_t MISRR41:1;
                vuint32_t MISRR40:1;
                vuint32_t MISRR39:1;
                vuint32_t MISRR38:1;
                vuint32_t MISRR37:1;
                vuint32_t MISRR36:1;
                vuint32_t MISRR35:1;
                vuint32_t MISRR34:1;
                vuint32_t MISRR33:1;
                vuint32_t MISRR32:1;
            } B;
        } LB_MISRRHSW;

        uint8_t STCU2_reserved2[16];
    } LB[16];

    uint8_t STCU2_reserved3[256];

    struct {
        union {
            vuint32_t R;
            struct {
                vuint32_t CSM:1;
                vuint32_t PTR:7;
                vuint32_t:24;
            } B;
        } MB_CTRL;
    } MB[96];
};
/**************************************************************************/
/*                   Module: STM                                          */
/**************************************************************************/
struct STM_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t CPS:8;
            vuint32_t:6;
            vuint32_t FRZ:1;
            vuint32_t TEN:1;
        } B;
    } CR;

    union {
        vuint32_t R;
        struct {
            vuint32_t CNT:32;
        } B;
    } CNT;

    uint8_t STM_reserved1[8];

    struct {
        union {
            vuint32_t R;
            struct {
                vuint32_t:31;
                vuint32_t CEN:1;
            } B;
        } CCR;

        union {
            vuint32_t R;
            struct {
                vuint32_t:31;
                vuint32_t CIF:1;
            } B;
        } CIR;

        union {
            vuint32_t R;
            struct {
                vuint32_t CMP:32;
            } B;
        } CMP;

        uint8_t Channel_reserved[4];

    } Channel[4];
};
/**************************************************************************/
/*                   Module: SWT                                          */
/**************************************************************************/
struct SWT_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t MAP0:1;
            vuint32_t MAP1:1;
            vuint32_t MAP2:1;
            vuint32_t MAP3:1;
            vuint32_t MAP4:1;
            vuint32_t MAP5:1;
            vuint32_t MAP6:1;
            vuint32_t MAP7:1;
            vuint32_t:13;
            vuint32_t SMD:2;
            vuint32_t RIA:1;
            vuint32_t WND:1;
            vuint32_t ITR:1;
            vuint32_t HLK:1;
            vuint32_t SLK:1;
            vuint32_t CSL:1;
            vuint32_t STP:1;
            vuint32_t FRZ:1;
            vuint32_t WEN:1;
        } B;
    } CR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:31;
            vuint32_t TIF:1;
        } B;
    } IR;

    union {
        vuint32_t R;
        struct {
            vuint32_t WTO:32;
        } B;
    } TO;

    union {
        vuint32_t R;
        struct {
            vuint32_t WST:32;
        } B;
    } WN;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t WSC:16;
        } B;
    } SR;

    union {
        vuint32_t R;
        struct {
            vuint32_t CNT:32;
        } B;
    } CO;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t SK:16;
        } B;
    } SK;
};
/**************************************************************************/
/*                   Module: TTCAN                                        */
/**************************************************************************/
struct TTCAN_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t REL:4;
            vuint32_t STEP:4;
            vuint32_t SUBSTEP:4;
            vuint32_t YEAR:4;
            vuint32_t MON:8;
            vuint32_t DAY:8;
        } B;
    } CREL;

    union {
        vuint32_t R;
        struct {
            vuint32_t ETV:32;
        } B;
    } ENDN;

    uint8_t TTCAN_reserved1[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t RX:1;
            vuint32_t TX:2;
            vuint32_t LBCK:1;
            vuint32_t:4;
        } B;
    } TEST;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t WDV:8;
            vuint32_t WDC:8;
        } B;
    } RWD;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t TEST:1;
            vuint32_t DAR:1;
            vuint32_t MON:1;
            vuint32_t CSR:1;
            vuint32_t CSA:1;
            vuint32_t ASM:1;
            vuint32_t CCE:1;
            vuint32_t INIT:1;
        } B;
    } CCCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t BRPE:4;
            vuint32_t:1;
            vuint32_t TSEG2:3;
            vuint32_t CSR:4;
            vuint32_t SJW:2;
            vuint32_t BRPL:6;
        } B;
    } BTP;

    union {
        vuint32_t R;
        struct {
            vuint32_t:12;
            vuint32_t TCP:4;
            vuint32_t:14;
            vuint32_t TSS:2;
        } B;
    } TSCC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t TSC:16;
        } B;
    } TSCV;

    union {
        vuint32_t R;
        struct {
            vuint32_t TOP:16;
            vuint32_t:13;
            vuint32_t TOS:2;
            vuint32_t ETOC:1;
        } B;
    } TOCC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t TSC:16;
        } B;
    } TOCV;

    uint8_t TTCAN_reserved2[16];

    union {
        vuint32_t R;
        struct {
            vuint32_t:8;
            vuint32_t CEL:8;
            vuint32_t RP:1;
            vuint32_t REC:7;
            vuint32_t TEC:8;
        } B;
    } ECR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t BO:1;
            vuint32_t EW:1;
            vuint32_t EP:1;
            vuint32_t ACT:2;
            vuint32_t LEC:3;
        } B;
    } PSR;

    uint8_t TTCAN_reserved3[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t STE:1;
            vuint32_t FOE:1;
            vuint32_t ACKE:1;
            vuint32_t BE:1;
            vuint32_t CRCE:1;
            vuint32_t WDI:1;
            vuint32_t BO:1;
            vuint32_t EW:1;
            vuint32_t EP:1;
            vuint32_t ELO:1;
            vuint32_t BEU:1;
            vuint32_t BEC:1;
            vuint32_t:1;
            vuint32_t TOO:1;
            vuint32_t UMD:1;
            vuint32_t TSW:1;
            vuint32_t TEFL:1;
            vuint32_t TEFF:1;
            vuint32_t TEFW:1;
            vuint32_t TEFN:1;
            vuint32_t TFE:1;
            vuint32_t TCF:1;
            vuint32_t TC:1;
            vuint32_t HPM:1;
            vuint32_t RF1L:1;
            vuint32_t RF1F:1;
            vuint32_t RF1W:1;
            vuint32_t RF1N:1;
            vuint32_t RF0L:1;
            vuint32_t RF0F:1;
            vuint32_t RF0W:1;
            vuint32_t RF0N:1;
        } B;
    } IR;

    union {
        vuint32_t R;
        struct {
            vuint32_t STEE:1;
            vuint32_t FOEE:1;
            vuint32_t ACKEE:1;
            vuint32_t BEE:1;
            vuint32_t CRCEE:1;
            vuint32_t WDIE:1;
            vuint32_t BOE:1;
            vuint32_t EWE:1;
            vuint32_t EPE:1;
            vuint32_t ELOE:1;
            vuint32_t BEUE:1;
            vuint32_t BECE:1;
            vuint32_t:1;
            vuint32_t TOOE:1;
            vuint32_t UMDE:1;
            vuint32_t TSWE:1;
            vuint32_t TEFLE:1;
            vuint32_t TEFFE:1;
            vuint32_t TEFWE:1;
            vuint32_t TEFNE:1;
            vuint32_t TFEE:1;
            vuint32_t TCFE:1;
            vuint32_t TCE:1;
            vuint32_t HPME:1;
            vuint32_t RF1LE:1;
            vuint32_t RF1FE:1;
            vuint32_t RF1WE:1;
            vuint32_t RF1NE:1;
            vuint32_t RF0LE:1;
            vuint32_t RF0FE:1;
            vuint32_t RF0WE:1;
            vuint32_t RF0NE:1;
        } B;
    } IE;

    union {
        vuint32_t R;
        struct {
            vuint32_t STEL:1;
            vuint32_t FOEL:1;
            vuint32_t ACKEL:1;
            vuint32_t BEL:1;
            vuint32_t CRCEL:1;
            vuint32_t WDIL:1;
            vuint32_t BOL:1;
            vuint32_t EWL:1;
            vuint32_t EPL:1;
            vuint32_t ELOL:1;
            vuint32_t BEUL:1;
            vuint32_t BECL:1;
            vuint32_t:1;
            vuint32_t TOOL:1;
            vuint32_t UMDL:1;
            vuint32_t TSWL:1;
            vuint32_t TEFLL:1;
            vuint32_t TEFFL:1;
            vuint32_t TEFWL:1;
            vuint32_t TEFNL:1;
            vuint32_t TFEL:1;
            vuint32_t TCFL:1;
            vuint32_t TCL:1;
            vuint32_t HPML:1;
            vuint32_t RF1LL:1;
            vuint32_t RF1FL:1;
            vuint32_t RF1WL:1;
            vuint32_t RF1NL:1;
            vuint32_t RF0LL:1;
            vuint32_t RF0FL:1;
            vuint32_t RF0WL:1;
            vuint32_t RF0NL:1;
        } B;
    } ILS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:30;
            vuint32_t EINT1:1;
            vuint32_t EINT0:1;
        } B;
    } ILE;

    uint8_t TTCAN_reserved4[32];

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t ANFS:2;
            vuint32_t ANFE:2;
            vuint32_t RRFS:1;
            vuint32_t RRFE:1;
        } B;
    } GFC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:8;
            vuint32_t LSS:8;
            vuint32_t FLSSA:14;
            vuint32_t:2;
        } B;
    } SIDFC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:9;
            vuint32_t LSELSE:7;
            vuint32_t FLESA:14;
            vuint32_t:2;
        } B;
    } XIDFC;

    uint8_t TTCAN_reserved5[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:3;
            vuint32_t EIDM:29;
        } B;
    } XIDAM;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t FLST:1;
            vuint32_t FIDX:7;
            vuint32_t MSI:2;
            vuint32_t BIDX:6;
        } B;
    } HPMS;

    uint8_t TTCAN_reserved6[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t F0WM:7;
            vuint32_t:1;
            vuint32_t F0S:7;
            vuint32_t FOSA:14;
            vuint32_t:2;
        } B;
    } RXF0C;

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t RF0L:1;
            vuint32_t F0F:1;
            vuint32_t:10;
            vuint32_t F0GI:6;
            vuint32_t:1;
            vuint32_t F0FL:7;
        } B;
    } RXF0S;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t F0AI:6;
        } B;
    } RXF0A;

    uint8_t TTCAN_reserved7[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t F1WM:7;
            vuint32_t:1;
            vuint32_t F1S:7;
            vuint32_t F1SA:14;
            vuint32_t:2;
        } B;
    } RXF1C;

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t RF1L:1;
            vuint32_t F1F:1;
            vuint32_t:10;
            vuint32_t F1GI:6;
            vuint32_t:1;
            vuint32_t F1FL:7;
        } B;
    } RXF1S;

    union {
        vuint32_t R;
        struct {
            vuint32_t:26;
            vuint32_t F1A:6;
        } B;
    } RXF1A;

    uint8_t TTCAN_reserved8[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:1;
            vuint32_t TFQM:1;
            vuint32_t TFQS:6;
            vuint32_t:2;
            vuint32_t NDTB:6;
            vuint32_t TBSA:14;
            vuint32_t:2;
        } B;
    } TXBC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:10;
            vuint32_t TFQF:1;
            vuint32_t TFQPI:5;
            vuint32_t:10;
            vuint32_t TFFL:6;
        } B;
    } TXFQS;

    uint8_t TTCAN_reserved9[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t TRP:32;
        } B;
    } TXBRP;

    union {
        vuint32_t R;
        struct {
            vuint32_t AR:32;
        } B;
    } TXBAR;

    union {
        vuint32_t R;
        struct {
            vuint32_t CR:32;
        } B;
    } TXBCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t TO:32;
        } B;
    } TXBTO;

    union {
        vuint32_t R;
        struct {
            vuint32_t CF:32;
        } B;
    } TXBCF;

    union {
        vuint32_t R;
        struct {
            vuint32_t TIE:32;
        } B;
    } TXBTIE;

    union {
        vuint32_t R;
        struct {
            vuint32_t CFIE:32;
        } B;
    } TXBCIE;

    uint8_t TTCAN_reserved10[8];

    union {
        vuint32_t R;
        struct {
            vuint32_t:2;
            vuint32_t EFWM:6;
            vuint32_t:2;
            vuint32_t EFS:6;
            vuint32_t EFSA:14;
            vuint32_t:2;
        } B;
    } TXEFC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:6;
            vuint32_t TEFL:1;
            vuint32_t EFF:1;
            vuint32_t:11;
            vuint32_t EFGI:5;
            vuint32_t:2;
            vuint32_t EFFL:6;
        } B;
    } TXEFS;

    union {
        vuint32_t R;
        struct {
            vuint32_t:27;
            vuint32_t EFAI:5;
        } B;
    } TXEFA;

    uint8_t TTCAN_reserved11[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t:9;
            vuint32_t TME:7;
            vuint32_t TMSA:14;
            vuint32_t:2;
        } B;
    } TTTMC;

    union {
        vuint32_t R;
        struct {
            vuint32_t RMPS:1;
            vuint32_t XTD:1;
            vuint32_t:1;
            vuint32_t RID:29;
        } B;
    } TTRMC;

    union {
        vuint32_t R;
        struct {
            vuint32_t:5;
            vuint32_t EVTP:1;
            vuint32_t ECC:1;
            vuint32_t EGTF:1;
            vuint32_t AWL:8;
            vuint32_t EECS:1;
            vuint32_t IRTO:7;
            vuint32_t LDSDL:3;
            vuint32_t TM:1;
            vuint32_t GEN:1;
            vuint32_t:1;
            vuint32_t OM:2;
        } B;
    } TTOCF;

    union {
        vuint32_t R;
        struct {
            vuint32_t:4;
            vuint32_t ENTT:12;
            vuint32_t RESERVED9:4;
            vuint32_t TXEW:4;
            vuint32_t:2;
            vuint32_t CCM:6;
        } B;
    } TTMLM;

    union {
        vuint32_t R;
        struct {
            vuint32_t ELT:1;
            vuint32_t:1;
            vuint32_t DC:14;
            vuint32_t NCL:16;
        } B;
    } TURCF;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t LCKC:1;
            vuint32_t:1;
            vuint32_t ESCN:1;
            vuint32_t NIG:1;
            vuint32_t TMG:1;
            vuint32_t FGP:1;
            vuint32_t GCS:1;
            vuint32_t:1;
            vuint32_t TMC:2;
            vuint32_t RTIE:1;
            vuint32_t SWS:2;
            vuint32_t SWP:1;
            vuint32_t ECS:1;
            vuint32_t SGT:1;
        } B;
    } TTOCN;

    union {
        vuint32_t R;
        struct {
            vuint32_t CTP:16;
            vuint32_t TP:16;
        } B;
    } TTGTP;

    union {
        vuint32_t R;
        struct {
            vuint32_t LCKM:1;
            vuint32_t:8;
            vuint32_t TICC:7;
            vuint32_t TM:16;
        } B;
    } TTTMK;

    union {
        vuint32_t R;
        struct {
            vuint32_t:13;
            vuint32_t CER:1;
            vuint32_t AW:1;
            vuint32_t WT:1;
            vuint32_t IWT:1;
            vuint32_t ELC:1;
            vuint32_t SE2:1;
            vuint32_t SE1:1;
            vuint32_t TXO:1;
            vuint32_t TXU:1;
            vuint32_t GTE:1;
            vuint32_t GTD:1;
            vuint32_t GTW:1;
            vuint32_t SWE:1;
            vuint32_t TTMI:1;
            vuint32_t RTMI:1;
            vuint32_t SOG:1;
            vuint32_t CSM:1;
            vuint32_t SMC:1;
            vuint32_t SBC:1;
        } B;
    } TTIR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:13;
            vuint32_t CERE:1;
            vuint32_t AWE:1;
            vuint32_t WTE:1;
            vuint32_t IWTE:1;
            vuint32_t ELCE:1;
            vuint32_t SE2E:1;
            vuint32_t SE1E:1;
            vuint32_t TXOE:1;
            vuint32_t TXUE:1;
            vuint32_t GTEE:1;
            vuint32_t GTDE:1;
            vuint32_t GTWE:1;
            vuint32_t SWEE:1;
            vuint32_t TTMIE:1;
            vuint32_t RTMIE:1;
            vuint32_t SOGE:1;
            vuint32_t CSME:1;
            vuint32_t SMCE:1;
            vuint32_t SBCE:1;
        } B;
    } TTIE;

    union {
        vuint32_t R;
        struct {
            vuint32_t:13;
            vuint32_t CERL:1;
            vuint32_t AWL:1;
            vuint32_t WTL:1;
            vuint32_t IWTL:1;
            vuint32_t ELCL:1;
            vuint32_t SE2L:1;
            vuint32_t SE1L:1;
            vuint32_t TXOL:1;
            vuint32_t TXUL:1;
            vuint32_t GTEL:1;
            vuint32_t GTDL:1;
            vuint32_t GTWE:1;
            vuint32_t SWEL:1;
            vuint32_t TTMIL:1;
            vuint32_t RTMIL:1;
            vuint32_t SOGL:1;
            vuint32_t CSML:1;
            vuint32_t SMCL:1;
            vuint32_t SBCL:1;
        } B;
    } TTILS;

    union {
        vuint32_t R;
        struct {
            vuint32_t SPL:1;
            vuint32_t WECS:1;
            vuint32_t AWE:1;
            vuint32_t WFE:1;
            vuint32_t GSI:1;
            vuint32_t TMP:3;
            vuint32_t GFI:1;
            vuint32_t WGTD:1;
            vuint32_t:6;
            vuint32_t RTO:8;
            vuint32_t QCS:1;
            vuint32_t QGTP:1;
            vuint32_t SYS:2;
            vuint32_t MS:2;
            vuint32_t EL:2;
        } B;
    } TTOST;

    union {
        vuint32_t R;
        struct {
            vuint32_t:14;
            vuint32_t NAV:18;
        } B;
    } TURNA;

    union {
        vuint32_t R;
        struct {
            vuint32_t GT:16;
            vuint32_t LT:16;
        } B;
    } TTLGT;

    union {
        vuint32_t R;
        struct {
            vuint32_t:10;
            vuint32_t CC:6;
            vuint32_t CT:16;
        } B;
    } TTCTC;

    union {
        vuint32_t R;
        struct {
            vuint32_t SWV:16;
            vuint32_t:10;
            vuint32_t CCV:6;
        } B;
    } TTCPT;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t CSM:16;
        } B;
    } TTCSM;
};
/**************************************************************************/
/*                   Module: WKPU                                        */
/**************************************************************************/
struct WKPU_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t NIF0:1;
            vuint32_t NOVF0:1;
            vuint32_t:6;
            vuint32_t NIF1:1;
            vuint32_t NOVF1:1;
            vuint32_t:6;
            vuint32_t NIF2:1;
            vuint32_t NOVF2:1;
            vuint32_t:6;
            vuint32_t RIF:1;
            vuint32_t ROVF:1;
            vuint32_t:6;
        } B;
    } NSR;

    uint8_t WKPU_reserved1[4];

    union {
        vuint32_t R;
        struct {
            vuint32_t NLOCK0:1;
            vuint32_t NDSS0:2;
            vuint32_t NWRE0:1;
            vuint32_t:1;
            vuint32_t NREE0:1;
            vuint32_t NFEE0:1;
            vuint32_t NFE0:1;
            vuint32_t NLOCK1:1;
            vuint32_t NDSS1:2;
            vuint32_t NWRE1:1;
            vuint32_t:1;
            vuint32_t NREE1:1;
            vuint32_t NFEE1:1;
            vuint32_t:1;
            vuint32_t NLOCK2:1;
            vuint32_t NDSS2:2;
            vuint32_t NWRE2:1;
            vuint32_t:1;
            vuint32_t NREE2:1;
            vuint32_t NFEE2:1;
            vuint32_t:1;
            vuint32_t RLOCK:1;
            vuint32_t RDSS:2;
            vuint32_t RWRE:1;
            vuint32_t:1;
            vuint32_t RREE:1;
            vuint32_t RFEE:1;
            vuint32_t:1;
        } B;
    } NCR;
};
/**************************************************************************/
/*                   Module: XBAR                                         */
/**************************************************************************/
struct XBAR_tag {
    struct {
        union {
            vuint32_t R;
            struct {
                vuint32_t:1;
                vuint32_t M7:3;
                vuint32_t:1;
                vuint32_t M6:3;
                vuint32_t:1;
                vuint32_t M5:3;
                vuint32_t:1;
                vuint32_t M4:3;
                vuint32_t:1;
                vuint32_t M3:3;
                vuint32_t:1;
                vuint32_t M2:3;
                vuint32_t:1;
                vuint32_t M1:3;
                vuint32_t:1;
                vuint32_t M0:3;
            } B;
        } PRS;

        uint8_t Channel_reserved1[12];

        union {
            vuint32_t R;
            struct {
                vuint32_t RO:1;
                vuint32_t:1;
                vuint32_t:20;
                vuint32_t ARB:2;
                vuint32_t:2;
                vuint32_t PCTL:2;
                vuint32_t:1;
                vuint32_t PARK:3;
            } B;
        } CRS;

        uint8_t Channel_reserved2[236];
    } CHANNEL[8];
};
/**************************************************************************/
/*                   Module: XBIC                                         */
/**************************************************************************/
struct XBIC_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t:24;
            vuint32_t SE0:1;
            vuint32_t SE1:1;
            vuint32_t SE2:1;
            vuint32_t SE3:1;
            vuint32_t SE4:1;
            vuint32_t SE5:1;
            vuint32_t SE6:1;
            vuint32_t SE7:1;
        } B;
    } MCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t EIE:1;
            vuint32_t:16;
            vuint32_t SLV:3;
            vuint32_t MST:4;
            vuint32_t SYN:8;
        } B;
    } EIR;

    union {
        vuint32_t R;
        struct {
            vuint32_t VLD:1;
            vuint32_t:16;
            vuint32_t SLV:3;
            vuint32_t MST:4;
            vuint32_t SYN:8;
        } B;
    } ESR;

    union {
        vuint32_t R;
        struct {
            vuint32_t ADDR:16;
            vuint32_t:16;
        } B;
    } EAR;
};
/**************************************************************************/
/*                   Module: XOSC                                         */
/**************************************************************************/
struct XOSC_tag {
    union {
        vuint32_t R;
        struct {
            vuint32_t OSCBYP:1;
            vuint32_t:7;
            vuint32_t EOCV:8;
            vuint32_t M_OSC:1;
            vuint32_t:7;
            vuint32_t I_OSC:1;
            vuint32_t:5;
            vuint32_t S_OSC:1;
            vuint32_t OSCON:1;  
        } B;
    } XOSC_CTL;
};


    

    
/* Define instances of modules PBRIDGE_B */ 
#define PBRIDGE_B     (*(volatile struct PBRIDGE_tag *)   0xF8000000UL)
#define SARADC_2      (*(volatile struct SAR_tag *)       0xFBE08000UL)
#define SARADC_6      (*(volatile struct SAR_tag *)       0xFBE18000UL)
#define PSI5_1        (*(volatile struct PSI5_tag *)      0xFBE40000UL)
#define SRX_1         (*(volatile struct SRX_tag *)       0xFBE5C000UL)
#define DSPI_2        (*(volatile struct DSPI_tag *)      0xFBE70000UL)
#define DSPI_5        (*(volatile struct DSPI_tag *)      0xFBE78000UL)
#define LINFlexD_2    (*(volatile struct LINFlexD_tag *)  0xFBE8C000UL)
#define LINFlexD_15   (*(volatile struct LINFlexD_tag *)  0xFBEA8000UL)
#define SDADC_3       (*(volatile struct SDADC_tag *)     0xFBF10000UL)
#define FCCU          (*(volatile struct FCCU_tag *)      0xFBF58000UL)
#define CRC_1         (*(volatile struct CRC_tag *)       0xFBF64000UL)
#define CMU_CORE      (*(volatile struct CMU_tag *)       0xFBFB0200UL)
#define CMU_SYS       (*(volatile struct CMU_tag *)       0xFBFB0240UL)
#define CMU_SXBAR     (*(volatile struct CMU_tag *)       0xFBFB0280UL)
#define CMU_PBRIDGE   (*(volatile struct CMU_tag *)       0xFBFB02C0UL)
#define CMU_PER       (*(volatile struct CMU_tag *)       0xFBFB0300UL)
#define CMU_ADCSD     (*(volatile struct CMU_tag *)       0xFBFB0340UL)
#define CMU_ADCSAR    (*(volatile struct CMU_tag *)       0xFBFB0380UL)
#define CMU_CAN       (*(volatile struct CMU_tag *)       0xFBFB03C0UL)
#define CMU_FLEXRAY   (*(volatile struct CMU_tag *)       0xFBFB0400UL)
#define CMU_SENT      (*(volatile struct CMU_tag *)       0xFBFB0440UL)
#define CMU_PSI5_F189 (*(volatile struct CMU_tag *)       0xFBFB0480UL)
#define CMU_PSI5_1US  (*(volatile struct CMU_tag *)       0xFBFB04C0UL)
    
/* Define instances of modules PBRIDGE_A */ 
#define PBRIDGE_A     (*(volatile struct PBRIDGE_tag *)   0xFC000000UL)
#define XBAR_0        (*(volatile struct XBAR_tag *)      0xFC004000UL)
#define XBAR_1        (*(volatile struct XBAR_tag *)      0xFC008000UL)
#define SMPU_0        (*(volatile struct SMPU_tag *)      0xFC010000UL)
#define SMPU_1        (*(volatile struct SMPU_tag *)      0xFC014000UL)
#define PRAM          (*(volatile struct PRAM_tag *)      0xFC020000UL)
#define PCM           (*(volatile struct PCM_tag *)       0xFC028000UL)
#define PFLASH        (*(volatile struct PFLASH_tag *)    0xFC030000UL)
#define SEMA42        (*(volatile struct SEMA42_tag *)    0xFC03C000UL)
#define INTC          (*(volatile struct INTC_tag *)      0xFC040000UL)
#define SWT_0         (*(volatile struct SWT_tag *)       0xFC050000UL)
#define SWT_2         (*(volatile struct SWT_tag *)       0xFC058000UL)
#define SWT_3         (*(volatile struct SWT_tag *)       0xFC05C000UL)
#define STM_0         (*(volatile struct STM_tag *)       0xFC068000UL)
#define STM_2         (*(volatile struct STM_tag *)       0xFC070000UL)
#define DMA_0         (*(volatile struct DMA_tag *)       0xFC0A0000UL)
#define FEC           (*(volatile struct FEC_tag *)       0xFC0B0000UL)
#define GTMINT        (*(volatile struct GTMINT_tag *)    0xFFD00000UL)
#define SARADC_0      (*(volatile struct SARADC_tag *)    0xFFE00000UL)
#define SARADC_4      (*(volatile struct SARADC_tag *)    0xFFE10000UL)
#define SARADC_B      (*(volatile struct SARADC_tag *)    0xFFE3C000UL)
#define PSI5_0        (*(volatile struct PSI5_tag *)      0xFFE40000UL)
#define FR_0          (*(volatile struct FR_tag *)        0xFFE50000UL)   
#define SRX_0         (*(volatile struct SRX_tag *)       0xFFE5C000UL)
#define I2C_0         (*(volatile struct I2C_tag *)       0xFFE68000UL)
#define DSPI_0        (*(volatile struct DSPI_tag *)      0xFFE70000UL)
#define DSPI_1        (*(volatile struct DSPI_tag *)      0xFFE74000UL)
#define DSPI_4        (*(volatile struct DSPI_tag *)      0xFFE78000UL)
#define LINFlexD_0    (*(volatile struct LINFlexD_tag *)  0xFFE8C000UL)
#define LINFlexD_1    (*(volatile struct LINFlexD_tag *)  0xFBE8C000UL)
#define TTCAN_0       (*(volatile struct TTCAN_tag *)     0xFFEDC000UL)
#define MCAN_1        (*(volatile struct MCAN_tag *)      0xFFEE4000UL)
#define MCAN_2        (*(volatile struct MCAN_tag *)      0xFFEE8000UL)
#define SDADC_0       (*(volatile struct SDADC_tag *)     0xFF0C0000UL)
#define SDADC_2       (*(volatile struct SDADC_tag *)     0xFFF10000UL)
#define DTS           (*(volatile struct DTS_tag *)       0xFFF38000UL)
#define JDC           (*(volatile struct JDC_tag *)       0xFFF3C000UL)
#define STCU2         (*(volatile struct STCU2_tag *)     0xFFF44000UL)
#define JTAGM         (*(volatile struct JTAGM_tag *)     0xFFF48000UL)
#define MEMU          (*(volatile struct MEMU_tag *)      0xFFF50000UL)
#define IMA           (*(volatile struct IMA_tag *)       0xFFF54000UL)
#define CRC_0         (*(volatile struct CRC_tag *)       0xFFF64000UL)
#define DMAMUX        (*(volatile struct DMACHMUX_tag *)  0xFFF6C000UL)
#define PIT_1         (*(volatile struct PIT_tag *)       0xFFF80000UL)
#define PIT_0         (*(volatile struct PIT_tag *)       0xFFF84000UL)
#define WKPU          (*(volatile struct WKPU_tag *)      0xFFF98000UL)
#define MC_PCU        (*(volatile struct MC_PCU_tag *)    0xFFFA0000UL)
#define PMCDIG        (*(volatile struct PMCDIG_tag *)    0xFFFA0400UL)  
#define RGM           (*(volatile struct MC_RGM_tag *)    0xFFFA8000UL)
#define RCOSC         (*(volatile struct RCOSC_tag *)     0xFFFB0000UL)
#define XOSC          (*(volatile struct XOSC_tag *)      0xFFFB0080UL)
#define PLLDIG        (*(volatile struct PLLDIG_tag *)    0xFFFB0100UL)
#define CMU_IOP       (*(volatile struct CMU_tag *)       0xFFFB0200UL)
#define MC_CGM        (*(volatile struct MC_CGM_tag *)    0xFFFB0700UL)
#define MC_ME         (*(volatile struct MC_ME_tag *)     0xFFFB8000UL)
#define SIUL2         (*(volatile struct SIUL2_tag *)     0xFFFC0000UL)
#define SIPI_0        (*(volatile struct SIPI_tag *)      0xFFFD0000UL)
#define DIGIRF_0      (*(volatile struct DigRF_tag *)     0xFFFD8000UL)	//Not exist
#define DIGIRF_1      (*(volatile struct DigRF_tag *)     0xFFFDC000UL) //Not exist
#define FLASH         (*(volatile struct FLASH_tag *)     0xFFFE0000UL) 
#define PASS          (*(volatile struct PASS_tag *)      0xFFFF4000UL)
#define SSCM          (*(volatile struct SSCM_tag *)      0xFFFF8000UL)
    
#ifdef __MWERKS__
#pragma pop
#endif

#ifdef  __cplusplus
} 
#endif

#endif /* ifdef _SPC570S50L_H */


/******************************************************************************
 * FILE NAME: HSM_registers.h			COPYRIGHT (c) STMicroelectronics 2013 * 
 * REVISION:  0.1		                                  All Rights Reserved *
 *																			  *
 * DESCRIPTION:                                                               *
 * This file contains the register and bit field definitions for              *
 * the HSM module in the SPC57EM80.                                           *
 *=============================================================================*
 * UPDATE HISTORY                                                         *
 * REV      AUTHOR      DATE       	DESCRIPTION OF CHANGE                 *
 * ---   -----------  ---------    	---------------------                 *
 * 0.1	  C.A. 			04-Apr13   	Initial Prototype Release             *
 * 0.2	  D.C.			18-Jul13	Add HSM internal registers                                                                       *
 *
 *========================================================================*
 * COPYRIGHT:                                                             *
 *	STMicroelectronics, All Rights Reserved. You are hereby     *
 *  granted a copyright license to use, modify, and distribute the        *
 *  SOFTWARE so long as this entire notice is retained without alteration *
 *  in any modified and/or redistributed versions, and that such modified *
 *  versions are clearly identified as such. No licenses are granted by   *
 *  implication, estoppel or otherwise under any patentsor trademarks     *
 *  of STMicroelectronics. This software is provided on an      *
 *  "AS IS" basis and without warranty.                                   *
 *                                                                        *
 *  To the maximum extent permitted by applicable law, STMicroelectronics *
 *  DISCLAIMS ALL WARRANTIES WHETHER EXPRESS OR IMPLIED,                  *
 *  INCLUDING IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A      *
 *  PARTICULAR PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH REGARD  *
 *  TO THE SOFTWARE (INCLUDING ANY MODIFIED VERSIONS THEREOF) AND ANY     *
 *  ACCOMPANYING WRITTEN MATERIALS.                                       *
 *                                                                        *
 *  To the maximum extent permitted by applicable law, IN NO EVENT        *
 *  SHALL STMicroelectronics BE LIABLE FOR ANY DAMAGES WHATSOEVER         *
 *  (INCLUDING WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,  *
 *  BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR OTHER         *
 *  PECUNIARY LOSS) ARISING OF THE USE OR INABILITY TO USE THE SOFTWARE.  *
 *                                                                        *
 *  STMicroelectronics assumes no responsibility for the                  *
 *  maintenance and support of this software                              *
 *                                                                        *
 **************************************************************************/ 
    
/*>>>>>>> NOTE! this file is auto-generated please do not edit it! <<<<<<<*/ 
    
/**************************************************************************
 * Example register & bit field write:                                    *
 *                                                                        *
 *  <MODULE>.<REGISTER>.B.<BIT> = 1;                                      *
 *  <MODULE>.<REGISTER>.R       = 0x10000000;                             *
 *                                                                        *
 **************************************************************************/ 
    
#ifndef _HSM_H_
#define _HSM_H_
    
#include "typedefs.h"
    
#ifdef  __cplusplus
extern "C" {
#endif


/****************************************************************/
/*                                                              */
/* Module: HSM_INT internal registers							*/
/*                                                              */
/****************************************************************/

struct HSM_INT_tag {
    union {
        vuint32_t R;
        struct {
			vuint32_t:28;
			vuint32_t HSMTUR:1;
			vuint32_t HSMDUR:1;
			vuint32_t MDUR:1;
			vuint32_t:1;
        } B;
    } CR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:16;
            vuint32_t HSM_DATA_AF:1;
			vuint32_t HSM_CODE_AF6:1;
			vuint32_t HSM_CODE_AF5:1;
			vuint32_t HSM_CODE_AF4:1;
			vuint32_t HSM_CODE_AF3:1;
			vuint32_t HSM_CODE_AF2:1;
			vuint32_t HSM_CODE_AF1:1;
			vuint32_t HSM_CODE_AF0:1;
			vuint32_t HSM_DATA_EX:1;
			vuint32_t HSM_DATA_EX6:1;
			vuint32_t HSM_DATA_EX5:1;
			vuint32_t HSM_DATA_EX4:1;
			vuint32_t HSM_DATA_EX3:1;
			vuint32_t HSM_DATA_EX2:1;
			vuint32_t HSM_DATA_EX1:1;
			vuint32_t HSM_DATA_EX0:1;
        } B;
    } CONF;

    union {
        vuint32_t R;
        struct {
		    vuint32_t:23;
            vuint32_t RIE:1;
			vuint32_t:4;
            vuint32_t ID_ON:2;
            vuint32_t AD_ON:2;
        } B;
    } SR;

    union {
        vuint32_t R;
        struct {
		    vuint32_t:23;
            vuint32_t ME_TRAN:1;
			vuint32_t:7;
			vuint32_t DREQ:1;
        } B;
    } IF;
	
	union {
        vuint32_t R;
        struct {
            vuint32_t:23;
            vuint32_t IE8:1;
			vuint32_t:7;
			vuint32_t IE0:1;
        } B;
    } IE;
	
	union {
        vuint32_t R;
        struct {
            vuint32_t NSIF31:1;
			vuint32_t NSIF30:1;
			vuint32_t NSIF29:1;
			vuint32_t NSIF28:1;
			vuint32_t NSIF27:1;
			vuint32_t NSIF26:1;
			vuint32_t NSIF25:1;
			vuint32_t NSIF24:1;
			vuint32_t NSIF23:1;
			vuint32_t NSIF22:1;
			vuint32_t NSIF21:1;
			vuint32_t NSIF20:1;
			vuint32_t NSIF19:1;
			vuint32_t NSIF18:1;
			vuint32_t NSIF17:1;
			vuint32_t NSIF16:1;
			vuint32_t NSIF15:1;
			vuint32_t NSIF14:1;
			vuint32_t NSIF13:1;
			vuint32_t NSIF12:1;
			vuint32_t NSIF11:1;
			vuint32_t NSIF10:1;
			vuint32_t NSIF9:1;
			vuint32_t NSIF8:1;
			vuint32_t NSIF7:1;
			vuint32_t NSIF6:1;
			vuint32_t NSIF5:1;
			vuint32_t NSIF4:1;
			vuint32_t NSIF3:1;
			vuint32_t NSIF2:1;
			vuint32_t NSIF1:1;
			vuint32_t NSIF0:1;
        } B;
    } NSIF;
	
	union {
        vuint32_t R;
        struct {
            vuint32_t NSIE31:1;
			vuint32_t NSIE30:1;
			vuint32_t NSIE29:1;
			vuint32_t NSIE28:1;
			vuint32_t NSIE27:1;
			vuint32_t NSIE26:1;
			vuint32_t NSIE25:1;
			vuint32_t NSIE24:1;
			vuint32_t NSIE23:1;
			vuint32_t NSIE22:1;
			vuint32_t NSIE21:1;
			vuint32_t NSIE20:1;
			vuint32_t NSIE19:1;
			vuint32_t NSIE18:1;
			vuint32_t NSIE17:1;
			vuint32_t NSIE16:1;
			vuint32_t NSIE15:1;
			vuint32_t NSIE14:1;
			vuint32_t NSIE13:1;
			vuint32_t NSIE12:1;
			vuint32_t NSIE11:1;
			vuint32_t NSIE10:1;
			vuint32_t NSIE9:1;
			vuint32_t NSIE8:1;
			vuint32_t NSIE7:1;
			vuint32_t NSIE6:1;
			vuint32_t NSIE5:1;
			vuint32_t NSIE4:1;
			vuint32_t NSIE3:1;
			vuint32_t NSIE2:1;
			vuint32_t NSIE1:1;
			vuint32_t NSIE0:1;
        } B;
    } NSIE;
	
	union {
        vuint32_t R;
        struct {
            vuint32_t:28;
			vuint32_t TC_L:1;
			vuint32_t TC_H2:1;
			vuint32_t TC_H1:1;
			vuint32_t HVD_E:1;
        } B;
    } ESTS;
	
	union {
        vuint32_t R;
        struct {
            vuint32_t:28;
			vuint32_t ESIE3:1;
			vuint32_t ESIE2:1;
			vuint32_t ESIE1:1;
			vuint32_t ESIE0:1;
        } B;
    } ESIE;
	
	vuint16_t HSM_INT_reserved[14]; //0x0024--0x0040
	
	union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t IC_ECCAERR:1;
            vuint32_t IC_ECC2BIT:1;
            vuint32_t IC_ECC1BIT:1;
            vuint32_t:1;
            vuint32_t PR_ECCAERR:1;
            vuint32_t PR_ECC2BIT:1;
            vuint32_t PR_ECC1BIT:1;
        } B;
    } ECCSR;
	
	union {
        vuint32_t R;
        struct {
            vuint32_t:25;
            vuint32_t ECCIE6:1;
            vuint32_t ECCIE5:1;
            vuint32_t ECCIE4:1;
            vuint32_t:1;
            vuint32_t ECCIE2:1;
            vuint32_t ECCIE1:1;
            vuint32_t ECCIE0:1;
        } B;
    } ECCIE;
	
	union {
        vuint32_t R;
        struct {
            vuint32_t PRECCA:32;
        } B;
    } PRECCA;
	
	union {
        vuint32_t R;
        struct {
            vuint32_t ICECCA:32;
        } B;
    } ICECCA;
	
	vuint16_t HSM_INT_reserved1[8]; //0x0050--0x0060
	
	union {
        vuint32_t R;
        struct {
            vuint32_t DO_EN15:1;
            vuint32_t DO_EN14:1;
            vuint32_t DO_EN13:1;
            vuint32_t DO_EN12:1;
            vuint32_t DO_EN11:1;
            vuint32_t DO_EN10:1;
            vuint32_t DO_EN9:1;
            vuint32_t DO_EN8:1;
            vuint32_t DO_EN7:1;
            vuint32_t DO_EN6:1;
            vuint32_t DO_EN5:1;
            vuint32_t DO_EN4:1;
            vuint32_t DO_EN3:1;
            vuint32_t DO_EN2:1;
            vuint32_t DO_EN1:1;
            vuint32_t DO_EN0:1;
            vuint32_t DO15:1;
            vuint32_t DO14:1;
            vuint32_t DO13:1;
            vuint32_t DO12:1;
            vuint32_t DO11:1;
            vuint32_t DO10:1;
            vuint32_t DO9:1;
            vuint32_t DO8:1;
            vuint32_t DO7:1;
            vuint32_t DO6:1;
            vuint32_t DO5:1;
            vuint32_t DO4:1;
            vuint32_t DO3:1;
            vuint32_t DO2:1;
            vuint32_t DO1:1;
            vuint32_t DO0:1;
        } B;
    } IOCTL;
	
	vuint16_t HSM_INT_reserved2[6]; //0x0064--0x0070
	
	union {
        vuint32_t R;
        struct {
            vuint32_t:15;
			vuint32_t HSM_ASO:1;
			vuint32_t:14;
			vuint32_t HSM_FRES:1;
			vuint32_t HSM_DRES:1;
        } B;
    } RESCR;
	
	vuint16_t HSM_INT_reserved3[6]; //0x0074--0x0080
	
	union {
        vuint32_t R;
        struct {
            vuint32_t:16;
			vuint32_t PCKG15:1;
			vuint32_t PCKG14:1;
			vuint32_t PCKG13:1;
			vuint32_t PCKG12:1;
			vuint32_t PCKG11:1;
			vuint32_t PCKG10:1;
			vuint32_t PCKG9:1;
			vuint32_t PCKG8:1;
			vuint32_t PCKG7:1;
			vuint32_t PCKG6:1;
			vuint32_t PCKG5:1;
			vuint32_t PCKG4:1;
			vuint32_t PCKG3:1;
			vuint32_t PCKG2:1;
			vuint32_t PCKG1:1;
			vuint32_t PCKG0:1;
        } B;
    } ICKCR;
	
	vuint16_t HSM_INT_reserved4[2]; //0x0084--0x0088
	
	union {
        vuint32_t R;
        struct {
            vuint32_t DE0:1;
			vuint32_t:9;
			vuint32_t DIV0:6;
			vuint32_t:16;
        } B;
    } CKDIV0;
	
	union {
        vuint32_t R;
        struct {
            vuint32_t DE1:1;
			vuint32_t:9;
			vuint32_t DIV1:6;
			vuint32_t:16;
        } B;
    } CKDIV1;
	
};

/****************************************************************/
/*                                                              */
/* Module: HSM_SBI			  									*/
/*                                                              */
/****************************************************************/

struct HSM_SBI_tag {
	
    union {
        vuint32_t R;
        struct {
            vuint32_t:30;
			vuint32_t WBE:1;
			vuint32_t RBE:1;
        } B;
    } BCR;

    union {
        vuint32_t R;
        struct {
            vuint32_t:29;
            vuint32_t LOE:1;
            vuint32_t INV:1;
            vuint32_t CE:1;
        } B;
    } CCR;
	
};

/****************************************************************/
/*                                                              */
/* Module: HSM_HOST_IF                                          */
/*                                                              */
/****************************************************************/

struct HSM_HT2HSM_tag {
    /* HSM to HOST flags */
    union {
        vuint32_t R;
        struct {
            vuint32_t FLAG31:1;
            vuint32_t FLAG30:1;
            vuint32_t FLAG29:1;
            vuint32_t FLAG28:1;
            vuint32_t FLAG27:1;
            vuint32_t FLAG26:1;
            vuint32_t FLAG25:1;
            vuint32_t FLAG24:1;
            vuint32_t FLAG23:1;
            vuint32_t FLAG22:1;
            vuint32_t FLAG21:1;
            vuint32_t FLAG20:1;
            vuint32_t FLAG19:1;
            vuint32_t FLAG18:1;
            vuint32_t FLAG17:1;
            vuint32_t FLAG16:1;
            vuint32_t FLAG15:1;
            vuint32_t FLAG14:1;
            vuint32_t FLAG13:1;
            vuint32_t FLAG12:1;
            vuint32_t FLAG11:1;
            vuint32_t FLAG10:1;
            vuint32_t FLAG9:1;
            vuint32_t FLAG8:1;
            vuint32_t FLAG7:1;
            vuint32_t FLAG6:1;
            vuint32_t FLAG5:1;
            vuint32_t FLAG4:1;
            vuint32_t FLAG3:1;
            vuint32_t FLAG2:1;
            vuint32_t FLAG1:1;
            vuint32_t FLAG0:1;
        } B;
    } HSM2HTF;

    /* HSM to HOST Interrupt enable */
    union {
        vuint32_t R;
        struct {
            vuint32_t IE31:1;
            vuint32_t IE30:1;
            vuint32_t IE29:1;
            vuint32_t IE28:1;
            vuint32_t IE27:1;
            vuint32_t IE26:1;
            vuint32_t IE25:1;
            vuint32_t IE24:1;
            vuint32_t IE23:1;
            vuint32_t IE22:1;
            vuint32_t IE21:1;
            vuint32_t IE20:1;
            vuint32_t IE19:1;
            vuint32_t IE18:1;
            vuint32_t IE17:1;
            vuint32_t IE16:1;
            vuint32_t IE15:1;
            vuint32_t IE14:1;
            vuint32_t IE13:1;
            vuint32_t IE12:1;
            vuint32_t IE11:1;
            vuint32_t IE10:1;
            vuint32_t IE9:1;
            vuint32_t IE8:1;
            vuint32_t IE7:1;
            vuint32_t IE6:1;
            vuint32_t IE5:1;
            vuint32_t IE4:1;
            vuint32_t IE3:1;
            vuint32_t IE2:1;
            vuint32_t IE1:1;
            vuint32_t IE0:1;
        } B;
    } HSM2HTIE;

    /* HOST to HSM flags */
    union {
        vuint32_t R;
        struct {
            vuint32_t FLAG31:1;
            vuint32_t FLAG30:1;
            vuint32_t FLAG29:1;
            vuint32_t FLAG28:1;
            vuint32_t FLAG27:1;
            vuint32_t FLAG26:1;
            vuint32_t FLAG25:1;
            vuint32_t FLAG24:1;
            vuint32_t FLAG23:1;
            vuint32_t FLAG22:1;
            vuint32_t FLAG21:1;
            vuint32_t FLAG20:1;
            vuint32_t FLAG19:1;
            vuint32_t FLAG18:1;
            vuint32_t FLAG17:1;
            vuint32_t FLAG16:1;
            vuint32_t FLAG15:1;
            vuint32_t FLAG14:1;
            vuint32_t FLAG13:1;
            vuint32_t FLAG12:1;
            vuint32_t FLAG11:1;
            vuint32_t FLAG10:1;
            vuint32_t FLAG9:1;
            vuint32_t FLAG8:1;
            vuint32_t FLAG7:1;
            vuint32_t FLAG6:1;
            vuint32_t FLAG5:1;
            vuint32_t FLAG4:1;
            vuint32_t FLAG3:1;
            vuint32_t FLAG2:1;
            vuint32_t FLAG1:1;
            vuint32_t FLAG0:1;
        } B;
    } HT2HSMF;

    /* HOST to HSM Interrupt enable */
    union {
        vuint32_t R;
        struct {
            vuint32_t IE31:1;
            vuint32_t IE30:1;
            vuint32_t IE29:1;
            vuint32_t IE28:1;
            vuint32_t IE27:1;
            vuint32_t IE26:1;
            vuint32_t IE25:1;
            vuint32_t IE24:1;
            vuint32_t IE23:1;
            vuint32_t IE22:1;
            vuint32_t IE21:1;
            vuint32_t IE20:1;
            vuint32_t IE19:1;
            vuint32_t IE18:1;
            vuint32_t IE17:1;
            vuint32_t IE16:1;
            vuint32_t IE15:1;
            vuint32_t IE14:1;
            vuint32_t IE13:1;
            vuint32_t IE12:1;
            vuint32_t IE11:1;
            vuint32_t IE10:1;
            vuint32_t IE9:1;
            vuint32_t IE8:1;
            vuint32_t IE7:1;
            vuint32_t IE6:1;
            vuint32_t IE5:1;
            vuint32_t IE4:1;
            vuint32_t IE3:1;
            vuint32_t IE2:1;
            vuint32_t IE1:1;
            vuint32_t IE0:1;
        } B;
    } HT2HSMIE;

    /* HSM to HOST Status */
    union {
        vuint32_t R;
        struct {
            vuint32_t STATUS31:1;
            vuint32_t STATUS30:1;
            vuint32_t STATUS29:1;
            vuint32_t STATUS28:1;
            vuint32_t STATUS27:1;
            vuint32_t STATUS26:1;
            vuint32_t STATUS25:1;
            vuint32_t STATUS24:1;
            vuint32_t STATUS23:1;
            vuint32_t STATUS22:1;
            vuint32_t STATUS21:1;
            vuint32_t STATUS20:1;
            vuint32_t STATUS19:1;
            vuint32_t STATUS18:1;
            vuint32_t STATUS17:1;
            vuint32_t STATUS16:1;
            vuint32_t STATUS15:1;
            vuint32_t STATUS14:1;
            vuint32_t STATUS13:1;
            vuint32_t STATUS12:1;
            vuint32_t STATUS11:1;
            vuint32_t STATUS10:1;
            vuint32_t STATUS9:1;
            vuint32_t STATUS8:1;
            vuint32_t STATUS7:1;
            vuint32_t STATUS6:1;
            vuint32_t STATUS5:1;
            vuint32_t STATUS4:1;
            vuint32_t STATUS3:1;
            vuint32_t STATUS2:1;
            vuint32_t STATUS1:1;
            vuint32_t STATUS0:1;
        } B;
    } HSM2HTS;

    /* HOST to HSM Status */
    union {
        vuint32_t R;
        struct {
            vuint32_t STATUS31:1;
            vuint32_t STATUS30:1;
            vuint32_t STATUS29:1;
            vuint32_t STATUS28:1;
            vuint32_t STATUS27:1;
            vuint32_t STATUS26:1;
            vuint32_t STATUS25:1;
            vuint32_t STATUS24:1;
            vuint32_t STATUS23:1;
            vuint32_t STATUS22:1;
            vuint32_t STATUS21:1;
            vuint32_t STATUS20:1;
            vuint32_t STATUS19:1;
            vuint32_t STATUS18:1;
            vuint32_t STATUS17:1;
            vuint32_t STATUS16:1;
            vuint32_t STATUS15:1;
            vuint32_t STATUS14:1;
            vuint32_t STATUS13:1;
            vuint32_t STATUS12:1;
            vuint32_t STATUS11:1;
            vuint32_t STATUS10:1;
            vuint32_t STATUS9:1;
            vuint32_t STATUS8:1;
            vuint32_t STATUS7:1;
            vuint32_t STATUS6:1;
            vuint32_t STATUS5:1;
            vuint32_t STATUS4:1;
            vuint32_t STATUS3:1;
            vuint32_t STATUS2:1;
            vuint32_t STATUS1:1;
            vuint32_t STATUS0:1;
        } B;
    } HT2HSMS;
} ;

#define HSM_INT			(*(volatile struct HSM_INT_tag *) 	    0xA3F84000UL)
#define CMU_HSM			(*(volatile struct CMU_tag *)           0xA3F8C000UL)
#define SMPU_HSM		(*(volatile struct SMPU_tag *)          0xA3F10000UL)
#define HSM_SBI    		(*(volatile struct HSM_SBI_tag *) 	    0xA3F14000UL)
#define PIT_HSM		    (*(volatile struct PIT_tag *)           0xA3F18000UL)
#define INTC_HSM		(*(volatile struct INTC_tag *)          0xA3F48000UL)
#define HSM_HOST_IF     (*(volatile struct HSM_HT2HSM_tag *)    0xA3F88000UL)
#define FLASH_0A        (*(volatile struct FLASH_tag *)         0xF7FE4000UL)

#ifdef  __cplusplus
} 
#endif 
    
#endif /* ifdef _HSM_H */

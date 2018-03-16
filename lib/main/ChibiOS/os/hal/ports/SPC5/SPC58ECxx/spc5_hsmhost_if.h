/******************************************************************************
 * FILE NAME: spc58ne_hsmhost_if.h	    COPYRIGHT (c) STMicroelectronics 2015 *
 * REVISION:  0.4		                                 All Rights Reserved  *
 *																		      *
 * DESCRIPTION:                                                               *
 * This file contains the register and bit field definitions for              *
 * the HSM / HOST mailbox module in Eiger.                                    *
 *============================================================================*
 * UPDATE HISTORY                                                         *
 * REV      AUTHOR      DATE       	DESCRIPTION OF CHANGE                 *
 * ---   -----------  ---------    	---------------------                 *
 * 0.1	  C.A. 			04-Apr13   	Initial Prototype Release             *
 * 0.2	  D.C.			18-Jul13	Add HSM internal registers            *
 * 0.3	  D.C.			15-Nov13	Add HSM2HOST registers.				  *
 * 0.4    D.C			05-Mar15	Modified for Eiger device			  *
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
/* Module: HSM_HOST			  									*/
/*                                                              */
/****************************************************************/

struct HSM_HOST_tag {
	
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
	
};

/* HSM / HOST mailbox as seen from Z4 side */
#define HSM_HOST_IF        (*(volatile struct HSM_HOST_tag *)  0xF7F30000UL)


#ifdef  __cplusplus
} 
#endif 
    
#endif /* ifdef _HSM_H */

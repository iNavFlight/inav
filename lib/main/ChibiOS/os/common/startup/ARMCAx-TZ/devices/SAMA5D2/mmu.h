/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    mmu.h
 * @brief   MMU macros and structures.
 *
 * @addtogroup MMU
 * @{
 */

#ifndef MMU_H
#define MMU_H

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/
#define DCISW_INVALIDATE    0
#define DCISW_CLEAN         1
#define DCISW_CLEAN_AND_INV 2

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/


/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/*
 * Translation Table Entry descriptor macros
 *
 * Type Section layout:
 * +---------+--+-+--+-+-----+--------+-------+-+------+--+-+-+-+---+
 * |3       2|1 |1|1 |1|1    |1      1|1     1| |      |  | | | |   |
 * |1       0|9 |8|7 |6|5    |4      2|1     0|9|8    5|4 |3|2|1|0  |
 * +---------+--+-+--+-+-----+--------+-------+-+------+--+-+-+-+---+
 * | section |NS|0|nG|S|AP[2]|TEX[2:0]|AP[1:0]| |domain|XN|C|B|1|PXN|
 * +---------+--+-+--+-+-----+--------+-------+-+------+--+-+-+-+---+
 *
 */
#define TTE_TYPE_SECT             (0x01 << 1)
#define TTE_SECT_B                (0x01 << 2)
#define TTE_SECT_C                (0x01 << 3)
#define TTE_SECT_XN               (0x01 << 4)
#define TTE_SECT_DOM(x)           ((x)  << 5)
#define TTE_SECT_AP0              (0x01 << 10)
#define TTE_SECT_AP1              (0x01 << 11)
#define TTE_SECT_TEX(x)           ((x)  << 12)
#define TTE_SECT_AP2              (0x01 << 15)
#define TTE_SECT_S                (0x01 << 16)
#define TTE_SECT_NG               (0x01 << 17)
#define TTE_SECT_NS               (0x01 << 19)

#define TTE_SECT_MEM_CACHEABLE    (TTE_SECT_TEX(0b111)|TTE_SECT_B|TTE_SECT_C)
#define TTE_SECT_MEM_NO_CACHEABLE (TTE_SECT_TEX(0b100))
#define TTE_SECT_MEM_STRONGLY_ORD (TTE_SECT_TEX(0b000))
#define TTE_SECT_DEVICE           (TTE_SECT_B)
#define TTE_SECT_EXE_NEVER        (TTE_SECT_XN)
#define TTE_SECT_RW_ACCESS        (TTE_SECT_AP1|TTE_SECT_AP0)
#define TTE_SECT_RO_ACCESS        (TTE_SECT_AP1)
#define TTE_SECT_UNDEF            (0)

#define TTE_SECT_SECTION(addr)    ((addr) & 0xFFF00000)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void __mmu_init(void);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

#endif /* MMU_H */

/** @} */

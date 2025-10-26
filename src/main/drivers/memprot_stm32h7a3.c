/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "platform.h"

#include "common/utils.h"

#include "memprot.h"

// Defined in linker script
extern uint8_t dmaram_start;
extern uint8_t dmaram_end;

extern uint8_t dmarwaxi_start;
extern uint8_t dmarwaxi_end;

#if 0
  /** Initializes and configures the Region and the memory to be protected - ITC RAM
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected - FLASH
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x08000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_2MB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RO;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected - FLASH_CONFIG
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x081E0000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
#endif

mpuRegion_t mpuRegions[] = {
#ifdef USE_ITCM_RAM
    {
        //  Mark ITCM-RAM as read-only
        // "For Cortex®-M7, TCMs memories always behave as Non-cacheable, Non-shared normal memories, irrespective of the memory type attributes defined in the MPU for a memory region containing addresses held in the TCM"
        // See AN4838
        .start      = 0x00000000,
        .end        = 0, // Size defined by "size"
        .size       = MPU_REGION_SIZE_4GB,
        .perm       = MPU_REGION_PRIV_RO,
        .exec       = MPU_INSTRUCTION_ACCESS_ENABLE,
        .shareable  = MPU_ACCESS_NOT_SHAREABLE,
        .cacheable  = MPU_ACCESS_NOT_CACHEABLE,
        .bufferable = MPU_ACCESS_BUFFERABLE,
    },
#endif
    {
        .start      = 0x08000000, // Flash 2mb
        .end        = 0, // Size defined by "size"
        .size       = MPU_REGION_SIZE_2MB,
        .perm       = MPU_REGION_PRIV_RO,
        .exec       = MPU_INSTRUCTION_ACCESS_ENABLE,
        .shareable  = MPU_ACCESS_NOT_SHAREABLE,
        .cacheable  = MPU_ACCESS_CACHEABLE,
        .bufferable = MPU_ACCESS_BUFFERABLE,
 

    },
    {
        .start      = 0x081C0000, // Flash config, 128kb 0x81E0000 - 0x8200000
        .end        = 0, // Size defined by "size"
        .size       = MPU_REGION_SIZE_256KB,
        .perm       = MPU_REGION_FULL_ACCESS,
        .exec       = MPU_INSTRUCTION_ACCESS_ENABLE,
        .shareable  = MPU_ACCESS_NOT_SHAREABLE,
        .cacheable  = MPU_ACCESS_CACHEABLE,
        .bufferable = MPU_ACCESS_BUFFERABLE,
    },
    {
        // DMA transmit buffer in D2 SRAM1
        // Reading needs cache coherence operation
        .start      = (uint32_t)&dmaram_start,
        .end        = (uint32_t)&dmaram_end,
        .size       = 0,  // Size determined by ".end"
        .perm       = MPU_REGION_FULL_ACCESS,
        .exec       = MPU_INSTRUCTION_ACCESS_ENABLE,
        .shareable  = MPU_ACCESS_SHAREABLE,
        .cacheable  = MPU_ACCESS_CACHEABLE,
        .bufferable = MPU_ACCESS_NOT_BUFFERABLE,
    },
#ifdef USE_SDCARD_SDIO
    {
        // A region in AXI RAM accessible from SDIO internal DMA
        .start      = (uint32_t)&dmarwaxi_start,
        .end        = (uint32_t)&dmarwaxi_end,
        .size       = 0,  // Size determined by ".end"
        .perm       = MPU_REGION_FULL_ACCESS,
        .exec       = MPU_INSTRUCTION_ACCESS_ENABLE,
        .shareable  = MPU_ACCESS_NOT_SHAREABLE,
        .cacheable  = MPU_ACCESS_CACHEABLE,
        .bufferable = MPU_ACCESS_NOT_BUFFERABLE,
    },
#endif
};

unsigned mpuRegionCount = ARRAYLEN(mpuRegions);

STATIC_ASSERT(ARRAYLEN(mpuRegions) <= MAX_MPU_REGIONS, MPU_region_count_exceeds_limit);

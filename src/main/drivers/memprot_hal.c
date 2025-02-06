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

#include <string.h>

#include "platform.h"

#include "memprot.h"

#ifndef STM32H7A3xx
static void memProtConfigError(void)
{
    for (;;) {}
}
#endif

void memProtConfigure(mpuRegion_t *regions, unsigned regionCount)
{
    MPU_Region_InitTypeDef MPU_InitStruct = {0};
    /* Disables the MPU */
    HAL_MPU_Disable();
#ifdef STM32H7A3xx
    UNUSED(regions);
    UNUSED(regionCount);
    /** Initializes and configures the Region and the memory to be protected
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
#else

    if (regionCount > MAX_MPU_REGIONS) {
        memProtConfigError();
    }

    // Setup common members
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;

    for (unsigned number = 0; number < regionCount; number++) {
        mpuRegion_t *region = &regions[number];

        if (region->end == 0 && region->size == 0) {
            memProtConfigError();
        }

        MPU_InitStruct.Number      = number;
        MPU_InitStruct.BaseAddress = region->start;

        if (region->size) {
            MPU_InitStruct.Size = region->size;
        } else {
            // Adjust start of the region to align with cache line size.
            uint32_t start = region->start & ~0x1F;
            uint32_t length = region->end - start;

            if (length < 32) {
                // This will also prevent flsl from returning negative (case length == 0)
                length = 32;
            }

            int msbpos = flsl(length) - 1;

            if (length == (1U << msbpos)) {
                msbpos += 1;
            }

            MPU_InitStruct.Size = msbpos;
        }

        // Copy per region attributes
        MPU_InitStruct.AccessPermission = region->perm;
        MPU_InitStruct.DisableExec      = region->exec;
        MPU_InitStruct.IsShareable      = region->shareable;
        MPU_InitStruct.IsCacheable      = region->cacheable;
        MPU_InitStruct.IsBufferable     = region->bufferable;

        HAL_MPU_ConfigRegion(&MPU_InitStruct);
    }
#endif

    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void memProtReset(void)
{
    MPU_Region_InitTypeDef MPU_InitStruct = {0};

    /* Disable the MPU */
    HAL_MPU_Disable();

#ifdef STM32H7A3xx
    MPU_InitStruct.Enable = MPU_REGION_DISABLE;
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
#else

    // Disable existing regions

    for (uint8_t region = 0; region <= MAX_MPU_REGIONS; region++) {
        MPU_InitStruct.Enable = MPU_REGION_DISABLE;
        MPU_InitStruct.Number = region;
        HAL_MPU_ConfigRegion(&MPU_InitStruct);
    }
#endif
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

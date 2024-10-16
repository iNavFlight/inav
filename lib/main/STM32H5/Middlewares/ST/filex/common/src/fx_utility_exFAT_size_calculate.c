/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** FileX Component                                                       */
/**                                                                       */
/**   Utility                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/
#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"


#ifdef FX_ENABLE_EXFAT
#include "fx_system.h"
#include "fx_media.h"
#include "fx_utility.h"
#include "fx_directory_exFAT.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_exFAT_size_calculate                    PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates FAT size according to given parameters.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    boundary_unit                         Data area alignment in sectors*/
/*    size_in_sectors                       Partition size in sectors     */
/*    sectors_per_cluster                   Number of sectors per cluster */
/*    sectors_per_fat_ptr                   Pointer to sector per FAT     */
/*    fat_offset_ptr                        Pointer to offset of FAT      */
/*    cluster_heap_offset_ptr               Pointer to offset of cluster  */
/*                                            heap                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_media_exFAT_format                                              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  03-02-2021     William E. Lamie         Modified comment(s), fixed    */
/*                                            FAT size calculation issue, */
/*                                            resulting in version 6.1.5  */
/*  01-31-2022     Bhupendra Naphade        Modified comment(s), and      */
/*                                            replaced sector size        */
/*                                            constant,                   */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
VOID  _fx_utility_exFAT_size_calculate(UINT bytes_per_sector, ULONG boundary_unit, ULONG64 size_in_sectors, ULONG sectors_per_cluster,
                                       ULONG *sectors_per_fat_ptr, ULONG *fat_offset_ptr, ULONG *cluster_heap_offset_ptr)
{

ULONG   system_area_sectors = 0; /* Number of sectors for exFTA system area (from partition begining) */
ULONG64 total_cluster_heap_sectors;


    /* Align the boundary unit. If zero supplied, then set to default value.  */
    boundary_unit = ALIGN_UP(boundary_unit, 128);

    /* Align System Area according Boundary Unit.  */
    while (system_area_sectors < (EXFAT_BOOT_REGION_SIZE + EXFAT_MIN_NUM_OF_RESERVED_SECTORS))
    {

        system_area_sectors += boundary_unit;
    }

    /* Save preliminary FAT offset.  */
    *fat_offset_ptr = system_area_sectors;

    /* Save preliminary Clusters Heap offset.  */
    *cluster_heap_offset_ptr = *fat_offset_ptr;

    /* Calculate number of available sectors without system sectors.  */
    total_cluster_heap_sectors = size_in_sectors - system_area_sectors;

    /* Calculate required number of FAT sectors.  */
    *sectors_per_fat_ptr = (ULONG)DIVIDE_TO_CEILING(((total_cluster_heap_sectors / sectors_per_cluster) * EXFAT_FAT_BITS),
                                                    (bytes_per_sector * BITS_PER_BYTE));

    *sectors_per_fat_ptr = ALIGN_UP(*sectors_per_fat_ptr, boundary_unit >> 1);

    /* Check is it possible to allocate FAT inside calculated System Area sectors or not.  */
    if (system_area_sectors >
        (EXFAT_BOOT_REGION_SIZE +
         EXFAT_MIN_NUM_OF_RESERVED_SECTORS + *sectors_per_fat_ptr))
    {

        /* We able allocate FAT inside calculated logical_system_area_sectors.  */
        *fat_offset_ptr -= *sectors_per_fat_ptr;
    }
    else /* Recalculate FAT size since System Area will be increased on FAT size.  */
    {
        do
        {

            /* Increase System Area size on number of FAT sectors aligned according BU.  */
            system_area_sectors = *fat_offset_ptr + *sectors_per_fat_ptr;

            /* Decrease sectors available for clusters heap.  */
            total_cluster_heap_sectors = size_in_sectors - system_area_sectors;

            /* Re-calculate number of sectors per FAT.  */
            *sectors_per_fat_ptr = (ULONG)DIVIDE_TO_CEILING(((total_cluster_heap_sectors / sectors_per_cluster) * EXFAT_FAT_BITS),
                                                            (bytes_per_sector * BITS_PER_BYTE));

            *sectors_per_fat_ptr = ALIGN_UP(*sectors_per_fat_ptr, boundary_unit >> 1);

            /* Increase Cluster Heap offset according new FAT size.  */
            *cluster_heap_offset_ptr = *fat_offset_ptr + *sectors_per_fat_ptr;

        /* Loop until we find a FAT size that can hold all the clusters.  */
        }while (*sectors_per_fat_ptr * bytes_per_sector * BITS_PER_BYTE / EXFAT_FAT_BITS < ((size_in_sectors - *cluster_heap_offset_ptr) / sectors_per_cluster));
    }
}

#endif /* FX_ENABLE_EXFAT */


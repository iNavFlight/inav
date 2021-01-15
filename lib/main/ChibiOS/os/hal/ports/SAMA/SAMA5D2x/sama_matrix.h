/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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
 * @file    SAMA5D2x/sama_matrix.h
 * @brief   SAMA MATRIX support macros and structures.
 *
 * @addtogroup SAMA5D2x_MATRIX
 * @{
 */

#ifndef SAMA_MATRIX_H
#define SAMA_MATRIX_H

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
/**
 * @name    SECURE MATRIX mode macros
 * @{
 */
/**
 * @brief   The low area of region is the securable one.
 */
#define LOWER_AREA_SECURABLE     0x0u

/**
 * @brief   The upper area of region is the non-securable one.
 */
#define UPPER_AREA_SECURABLE     0x1u

/**
 * @brief   Securable area is secured for reads.
 */
#define SECURE_READ              0x0u

/**
 * @brief   Securable area is secured for writes.
 */
#define SECURE_WRITE             0x0u

/**
 * @brief   Securable area is non-secured for reads.
 */
#define NOT_SECURE_READ          0x1u

/**
 * @brief   Securable area is non-secured for writes.
 */
#define NOT_SECURE_WRITE         0x1u

/**
 * @brief   Peripheral Securable as secure.
 */
#define SECURE_PER               FALSE

/**
 * @brief   Peripheral Securable as not-secure.
 */
#define NOT_SECURE_PER           TRUE
/** @} */

/**
 * @name    MASTER TYPE MATRIX macros
 * @{
 */
/**
 * @brief   No Default Master.
 */
#define NO_DEFAULT_MASTER        0x0u

/**
 * @brief   Last Default Master.
 */
#define LAST_DEFAULT_MASTER      0x1u

/**
 * @brief   Fixed Default Master.
 */
#define FIXED_DEFAULT_MASTER     0x2u
/** @} */

/**
 * @name    REGION MATRIX MASK macros
 * @{
 */
/**
 * @brief   Region 0.
 */
#define REGION_0_MSK             (0x1u << 0)

/**
 * @brief   Region 1.
 */
#define REGION_1_MSK             (0x1u << 1)

/**
 * @brief   Region 2.
 */
#define REGION_2_MSK             (0x1u << 2)

/**
 * @brief   Region 3.
 */
#define REGION_3_MSK             (0x1u << 3)

/**
 * @brief   Region 4.
 */
#define REGION_4_MSK             (0x1u << 4)

/**
 * @brief   Region 5.
 */
#define REGION_5_MSK             (0x1u << 5)

/**
 * @brief   Region 6.
 */
#define REGION_6_MSK             (0x1u << 6)

/**
 * @brief   Region 7.
 */
#define REGION_7_MSK             (0x1u << 7)
/** @} */

/**
 * @name    REGION MATRIX macros
 * @{
 */
/**
 * @brief   Region 0.
 */
#define REGION_0                 0x0u

/**
 * @brief   Region 1.
 */
#define REGION_1                 0x1u

/**
 * @brief   Region 2.
 */
#define REGION_2                 0x2u

/**
 * @brief   Region 3.
 */
#define REGION_3                 0x3u

/**
 * @brief   Region 4.
 */
#define REGION_4                 0x4u

/**
 * @brief   Region 5.
 */
#define REGION_5                 0x5u

/**
 * @brief   Region 6.
 */
#define REGION_6                 0x6u

/**
 * @brief   Region 7.
 */
#define REGION_7                 0x7u
/** @} */

/**
 * @name    AREA SIZE MATRIX macros
 * @{
 */
/**
 * @brief   Area size 4 KB.
 */
#define MATRIX_AREA_SIZE_4K      0x0u

/**
 * @brief   Area size 8 KB.
 */
#define MATRIX_AREA_SIZE_8K      0x1u

/**
 * @brief   Area size 16 KB.
 */
#define MATRIX_AREA_SIZE_16K     0x2u

/**
 * @brief   Area size 32 KB.
 */
#define MATRIX_AREA_SIZE_32K     0x3u

/**
 * @brief   Area size 64 KB.
 */
#define MATRIX_AREA_SIZE_64K     0x4u

/**
 * @brief   Area size 128 KB.
 */
#define MATRIX_AREA_SIZE_128K    0x5u

/**
 * @brief   Area size 256 KB.
 */
#define MATRIX_AREA_SIZE_256K    0x6u

/**
 * @brief   Area size 512 KB.
 */
#define MATRIX_AREA_SIZE_512K    0x7u

/**
 * @brief   Area size 1 MB.
 */
#define MATRIX_AREA_SIZE_1M      0x8u

/**
 * @brief   Area size 2 MB.
 */
#define MATRIX_AREA_SIZE_2M      0x9u

/**
 * @brief   Area size 4 MB.
 */
#define MATRIX_AREA_SIZE_4M      0xAu

/**
 * @brief   Area size 8 MB.
 */
#define MATRIX_AREA_SIZE_8M      0xBu

/**
 * @brief   Area size 16 MB.
 */
#define MATRIX_AREA_SIZE_16M     0xCu

/**
 * @brief   Area size 32 MB.
 */
#define MATRIX_AREA_SIZE_32M     0xDu

/**
 * @brief   Area size 64 MB.
 */
#define MATRIX_AREA_SIZE_64M     0xEu

/**
 * @brief   Area size 128 MB.
 */
#define MATRIX_AREA_SIZE_128M    0xFu
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
/**
 * @brief   Configure LANSECH per Region.
 *
 * @param[in] region    Region to configure.
 * @param[in] lansech   Securable mode.
 *
 * @api
 */
#define mtxRegionLansech(region, lansech)                   (lansech << region)

/**
 * @brief   Configure RDNSECH per Region.
 *
 * @param[in] region    Region to configure.
 * @param[in] rdnsech   Read securable mode.
 *
 * @api
 */
#define mtxRegionRdnsech(region, rdnsech)                   (rdnsech << region)

/**
 * @brief   Configure WRNSECH per Region.
 *
 * @param[in] region    Region to configure.
 * @param[in] wrnsech   Write securable mode.
 *
 * @api
 */
#define mtxRegionWrnsech(region, wrnsech)                   (wrnsech << region)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
 bool mtxConfigPeriphSecurity(Matrix *mtxp, uint32_t id, bool mode);
 void mtxConfigDefaultMaster(Matrix *mtxp, uint8_t slaveID,
                             uint8_t type, uint8_t masterID);
 void mtxConfigSlaveSec(Matrix *mtxp, uint8_t slaveID,
                        uint8_t selMask, uint8_t readMask,
                        uint8_t writeMask);
 void mtxSetSlaveSplitAddr(Matrix *mtxp, uint8_t slaveID,
                           uint8_t area, uint8_t mask);
 void mtxSetSlaveRegionSize(Matrix *mtxp, uint8_t slaveID,
                            uint8_t areaSize, uint8_t mask);
 void mtxRemapRom(void);
 void mtxRemapRam(void);
#ifdef __cplusplus
}
#endif

#endif /* SAMA_MATRIX_H */

/** @} */

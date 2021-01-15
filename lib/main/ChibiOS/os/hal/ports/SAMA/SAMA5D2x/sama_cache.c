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
 * @file    SAMA5D2x/sama_cache.c
 * @brief   SAMA CACHE support code.
 *
 * @addtogroup SAMA5D2x_CACHE
 * @{
 */

#include "hal.h"

#if !defined(SAMA_L2CC_ASSUME_ENABLED)
#define SAMA_L2CC_ASSUME_ENABLED 0
#endif

#if !defined(SAMA_L2CC_ENABLE)
#define SAMA_L2CC_ENABLE 0
#endif

/**
 * @brief   Invalidate D-Cache Region
 *
 * @param[in] start      Pointer to beginning of memory region.
 * @param[in] length     Length of the memory location.
 */
void cacheInvalidateRegion(void *start, uint32_t length) {

  uint32_t start_addr = (uint32_t)start;
  uint32_t end_addr = start_addr + length;
  uint32_t mva;

  /* Invalidate L1 D-Cache */
  for (mva = start_addr & ~(L1_CACHE_BYTES-1); mva < end_addr; mva += L1_CACHE_BYTES) {
    L1C_InvalidateDCacheMVA((uint32_t *)mva);
  }
#if ARM_SUPPORTS_L2CC
#if SAMA_L2CC_ASSUME_ENABLED || SAMA_L2CC_ENABLE
  /* Invalidate L2 Cache */
  for (mva = start_addr & ~(L2_CACHE_BYTES-1); mva < end_addr; mva += L2_CACHE_BYTES) {
    L2C_InvPa((uint32_t *)mva);
  }
#endif
#endif
}

/**
 * @brief   Clean D-Cache Region
 *
 * @param[in] start      Pointer to beginning of memory region.
 * @param[in] length     Length of the memory location.
 */
void cacheCleanRegion(void *start, uint32_t length) {

  uint32_t start_addr = (uint32_t)start;
  uint32_t end_addr = start_addr + length;
  uint32_t mva;

  /* Clean L1 D-Cache */
  for (mva = start_addr & ~(L1_CACHE_BYTES-1); mva < end_addr; mva += L1_CACHE_BYTES) {
    L1C_CleanDCacheMVA((uint32_t *)mva);
  }
#if ARM_SUPPORTS_L2CC
#if SAMA_L2CC_ASSUME_ENABLED || SAMA_L2CC_ENABLE
  /* Invalidate L2 Cache */
  for (mva = start_addr & ~(L2_CACHE_BYTES-1); mva < end_addr; mva += L2_CACHE_BYTES) {
    L2C_CleanPa((uint32_t *)mva);
  }
#endif
#endif
}

/**
 * @brief   Clean and Invalidate D-Cache Region
 *
 * @param[in] start      Pointer to beginning of memory region.
 * @param[in] length     Length of the memory location.
 */
void cacheCleanInvalidateRegion(void *start, uint32_t length) {

  uint32_t start_addr = (uint32_t)start;
  uint32_t end_addr = start_addr + length;
  uint32_t mva;

  /* Clean L1 D-Cache */
  for (mva = start_addr & ~(L1_CACHE_BYTES-1); mva < end_addr; mva += L1_CACHE_BYTES) {
    L1C_CleanInvalidateDCacheMVA((uint32_t *)mva);
  }
#if ARM_SUPPORTS_L2CC
#if SAMA_L2CC_ASSUME_ENABLED || SAMA_L2CC_ENABLE
  /* Invalidate L2 Cache */
  for (mva = start_addr & ~(L2_CACHE_BYTES-1); mva < end_addr; mva += L2_CACHE_BYTES) {
    L2C_CleanInvPa((uint32_t *)mva);
  }
#endif
#endif
}

/** @} */

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
 * @file    SAMA5D2x/sama_cache.h
 * @brief   SAMA CACHE support macros and structures.
 *
 * @addtogroup SAMA5D2x_CACHE
 * @{
 */
#ifndef SAMA_CACHE_H_
#define SAMA_CACHE_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
#define L1_CACHE_BYTES  32u
#define L2_CACHE_BYTES  32u

#define CACHE_ALIGNED   ALIGNED_VAR(L1_CACHE_BYTES)
#define NO_CACHE        __attribute__((section (".nocache")))


#ifdef __cplusplus
extern "C" {
#endif
  extern void cacheInvalidateRegion(void *start, uint32_t length);
  extern void cacheCleanRegion(void *start, uint32_t length);
  extern void cacheCleanInvalidateRegion(void *start, uint32_t length);
#ifdef __cplusplus
}
#endif

#endif /* SAMA_CACHE_H_ */

/** @} */

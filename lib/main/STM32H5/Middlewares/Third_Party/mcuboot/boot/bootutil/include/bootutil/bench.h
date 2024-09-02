/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2019 Linaro Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef H_BOOTUTIL_BENCH_H__
#define H_BOOTUTIL_BENCH_H__

#include "ignore.h"

#ifdef MCUBOOT_USE_BENCH

/* The platform-specific benchmark code should define a
 * `bench_state_t` type that holds the information needed for the
 * benchmark.  This is generally something small, such as an integer
 * holding the state.  This should also define plat_bench_start and
 * plat_bench_end, which likely have to be macros so that log messages
 * come from the right place in the code. */
#include <platform-bench.h>

/*
 * These are simple barrier-type benchmarks.  If a platform has
 * benchmarks that are enabled, calling `boot_bench_start()` before a
 * block of code and `boot_bench_stop()` after that block of code will
 * present this information in some manner (usually through logging).
 * The details of what is measured and how it is printed are specific
 * to the platform and the implementation.  A pointer to the
 * platform-specific state should be passed in.
 */
#define boot_bench_start(_state) do { \
    plat_bench_start(_state); \
} while (0)

#define boot_bench_stop(_state) do { \
    plat_bench_stop(_state); \
} while (0)

#else /* not MCUBOOT_USE_BENCH */

/* The type needs to take space.  As long as it remains unused, the C
 * compiler should eliminate this value entirely. */
typedef int bench_state_t;

/* Without benchmarking enabled, these are just empty. */
#define boot_bench_start(_state) do { \
    IGNORE(_state); \
} while(0)

#define boot_bench_stop(_state) do { \
    IGNORE(_state); \
} while(0)

#endif /* not MCUBOOT_USE_BENCH */

#endif /* not H_BOOTUTIL_BENCH_H__ */

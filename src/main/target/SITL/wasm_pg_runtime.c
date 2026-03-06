/*
 * WASM Parameter Group Runtime Support
 *
 * This file provides lazy memory allocation for the parameter group system
 * in WebAssembly builds. Unlike native builds where config memory is allocated
 * by the linker at compile-time, WASM builds must allocate memory at runtime.
 *
 * Key functions:
 * - wasmPgEnsureAllocated(): Lazy allocator called by PG accessor macros
 * - wasmPgInitAll(): Explicit initialization (optional, for eager allocation)
 */

#ifdef __EMSCRIPTEN__

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <emscripten.h>

#include "platform.h"
#include "config/parameter_group.h"
#include "fc/config.h"
#include "target/SITL/wasm_pg_runtime.h"

// Log allocation failures to browser console for debugging
#define PG_ALLOC_ERROR(pgn, bytes) \
    EM_ASM({ console.error('[WASM PG] Allocation failed: pgn=' + $0 + ' size=' + $1); }, pgn, bytes)

// Debug: Warn about suspicious pointer values that could break our heuristic.
// Values 1-100 are in a gray zone - could be small function table indices or
// unlikely but valid memory addresses. Log these for investigation.
#ifdef DEBUG
#define PG_WARN_SUSPICIOUS_RESET_PTR(pgn, ptr) \
    do { \
        uintptr_t _p = (uintptr_t)(ptr); \
        if (_p > 0 && _p < 100) { \
            EM_ASM({ console.warn('[WASM PG] Suspicious reset pointer pgn=' + $0 + ' ptr=' + $1 + ' (may be misidentified function/data)'); }, pgn, _p); \
        } \
    } while(0)
#else
#define PG_WARN_SUSPICIOUS_RESET_PTR(pgn, ptr) ((void)0)
#endif

/**
 * Fix up a profile's current-profile pointer if it's missing or NULL.
 * Returns the current profile pointer, or NULL on allocation failure.
 */
static void* fixupProfilePointer(const pgRegistry_t *reg, pgRegistry_t *mutableReg)
{
    if (!mutableReg->ptr) {
        // Allocate the pointer variable
        uint8_t **currentPtr = (uint8_t**)calloc(1, sizeof(uint8_t*));
        if (!currentPtr) {
            PG_ALLOC_ERROR(pgN(reg), sizeof(uint8_t*));
            return NULL;
        }
        *currentPtr = mutableReg->address;
        mutableReg->ptr = currentPtr;
    } else if (!*mutableReg->ptr) {
        // Pointer exists but points to NULL
        *mutableReg->ptr = mutableReg->address;
    }
    return *mutableReg->ptr;
}

/**
 * Ensure a parameter group has allocated memory.
 *
 * This function is called by the PG_DECLARE accessor macros on every config access.
 * It checks if memory has been allocated, and if not:
 * 1. Allocates memory via malloc()
 * 2. Initializes with defaults via pgResetInstance()
 * 3. For profile configs, allocates storage for all profiles
 *
 * NOT thread-safe - safe only for single-threaded WASM builds.
 *
 * @param reg  The parameter group registry entry
 * @return Pointer to the allocated memory (system config or current profile)
 */
void* wasmPgEnsureAllocated(const pgRegistry_t *reg)
{
    if (!reg) {
        return NULL;
    }

    const uint16_t regSize = pgSize(reg);
    const bool isProfile = pgIsProfile(reg);

    // Check if already allocated by testing if address is NULL
    if (reg->address != NULL) {
        if (isProfile) {
            // Ensure profile pointer is valid, fix if needed
            if (!reg->ptr || !*reg->ptr) {
                return fixupProfilePointer(reg, (pgRegistry_t*)reg);
            }
            return *reg->ptr;
        } else {
            return reg->address;
        }
    }

    // Need to allocate memory
    if (isProfile) {
        // Profile configs: Allocate arrays for all profiles
        const size_t arraySize = regSize * MAX_PROFILE_COUNT;

        // Allocate storage arrays (main config and backup copy)
        uint8_t *storage = (uint8_t*)calloc(1, arraySize);
        uint8_t *copyStorage = (uint8_t*)calloc(1, arraySize);

        if (!storage || !copyStorage) {
            // Allocation failed - clean up partial allocation
            PG_ALLOC_ERROR(pgN(reg), arraySize);
            free(storage);      // safe if NULL
            free(copyStorage);  // safe if NULL
            return NULL;
        }

        // Cast away const to update registry pointers during initialization.
        // This is safe because:
        // 1. In WASM, PG registry entries are in the heap (mutable), not ROM
        // 2. This is one-time initialization, not runtime modification
        // 3. The const declaration prevents accidental modification elsewhere
        pgRegistry_t *mutableReg = (pgRegistry_t*)reg;
        mutableReg->address = storage;
        mutableReg->copy = copyStorage;

        // For WASM, profile configs may not have reg->ptr allocated
        // (native builds create _ProfileCurrent global, WASM doesn't)
        if (!reg->ptr) {
            // Allocate the current profile pointer
            uint8_t **currentPtr = (uint8_t**)calloc(1, sizeof(uint8_t*));
            if (!currentPtr) {
                PG_ALLOC_ERROR(pgN(reg), sizeof(uint8_t*));
                free(storage);
                free(copyStorage);
                return NULL;
            }
            *currentPtr = storage;  // Point to first profile by default
            mutableReg->ptr = currentPtr;
        } else if (!*reg->ptr) {
            // Pointer exists but not initialized - point it to first profile
            *reg->ptr = storage;
        }

        // Initialize all profiles with defaults
        for (int profileIndex = 0; profileIndex < MAX_PROFILE_COUNT; profileIndex++) {
            uint8_t *base = storage + (regSize * profileIndex);

            // Zero initialize
            memset(base, 0, regSize);

            // Load reset template if available.
            // The reset union contains either a function pointer (reset.fn) or
            // a data pointer to a reset template (reset.ptr). In Emscripten:
            // - Function pointers are indices into the WebAssembly function table
            //   (typically small values < 4096)
            // - Data pointers are actual linear memory addresses (>= 4096 since
            //   the first 4KB is typically reserved/unmapped)
            // This heuristic works for current Emscripten versions but may need
            // adjustment if Emscripten changes its memory layout.
            PG_WARN_SUSPICIOUS_RESET_PTR(pgN(reg), reg->reset.ptr);
            if (reg->reset.ptr && (uintptr_t)reg->reset.ptr >= 4096) {
                memcpy(base, reg->reset.ptr, regSize);
            }
        }

        return *reg->ptr;  // Return current profile

    } else {
        // System configs: Allocate single instance + copy
        uint8_t *memory = (uint8_t*)calloc(1, regSize);
        uint8_t *copyMemory = (uint8_t*)calloc(1, regSize);

        if (!memory || !copyMemory) {
            // Clean up partial allocation
            PG_ALLOC_ERROR(pgN(reg), regSize);
            free(memory);       // safe if NULL
            free(copyMemory);   // safe if NULL
            return NULL;
        }

        // Cast away const (see profile config comment above for safety rationale)
        pgRegistry_t *mutableReg = (pgRegistry_t*)reg;
        mutableReg->address = memory;
        mutableReg->copy = copyMemory;

        // Initialize with defaults
        memset(memory, 0, regSize);

        // Load reset template if available (see profile config loop for
        // explanation of the >= 4096 heuristic for distinguishing data vs function pointers)
        PG_WARN_SUSPICIOUS_RESET_PTR(pgN(reg), reg->reset.ptr);
        if (reg->reset.ptr && (uintptr_t)reg->reset.ptr >= 4096) {
            memcpy(memory, reg->reset.ptr, regSize);
        }

        return memory;
    }
}

/**
 * Eagerly allocate all parameter groups at once.
 *
 * This is optional - the lazy allocation in wasmPgEnsureAllocated() will handle
 * on-demand allocation. However, calling this at boot can:
 * - Detect memory allocation failures early
 * - Avoid runtime allocation overhead on first access
 * - Ensure consistent initialization order
 */
void wasmPgInitAll(void)
{
    PG_FOREACH(reg) {
        wasmPgEnsureAllocated(reg);
    }
}

#endif // __EMSCRIPTEN__

// ------------------------------------------------------------
// v7-A Cache, TLB and Branch Prediction Maintenance Operations
// Header File
//
// Copyright (c) 2011 ARM Ltd.  All rights reserved.
// ------------------------------------------------------------

#ifndef _ARMV7A_GENERIC_H
#define _ARMV7A_GENERIC_H

//
// Note:
// *_IS() stands for "inner shareable"
// DO NOT USE THESE FUNCTIONS ON A CORTEX-A8
//

// ------------------------------------------------------------
// Interrupts
// Enable/disables IRQs (not FIQs)
void enableInterrupts(void);
void disableInterrupts(void);

// ------------------------------------------------------------
// Caches

void invalidateCaches_IS(void);
void cleanInvalidateDCache(void);
void invalidateCaches_IS(void);
void enableCaches(void);
void disableCaches(void);
void invalidateCaches(void);
void cleanDCache(void);

// ------------------------------------------------------------
// TLBs

void invalidateUnifiedTLB(void);
void invalidateUnifiedTLB_IS(void);

// ------------------------------------------------------------
// Branch prediction

void enableBranchPrediction(void);
void disableBranchPrediction(void);
void flushBranchTargetCache(void);
void flushBranchTargetCache_IS(void);

// ------------------------------------------------------------
// High Vecs

void enableHighVecs(void);
void disableHighVecs(void);

// ------------------------------------------------------------
// ID Registers

unsigned int getMIDR(void);

#define MIDR_IMPL_SHIFT  24
#define MIDR_IMPL_MASK   0xFF
#define MIDR_VAR_SHIFT   20
#define MIDR_VAR_MASK    0xF
#define MIDR_ARCH_SHIFT  16
#define MIDR_ARCH_MASK   0xF
#define MIDR_PART_SHIFT  4
#define MIDR_PART_MASK   0xFFF
#define MIDR_REV_SHIFT   0
#define MIDR_REV_MASK    0xF

// tmp = get_MIDR();
// implementor = (tmp >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK;
// variant     = (tmp >> MIDR_VAR_SHIFT)  & MIDR_VAR_MASK;
// architecture= (tmp >> MIDR_ARCH_SHIFT) & MIDR_ARCH_MASK;
// part_number = (tmp >> MIDR_PART_SHIFT) & MIDR_PART_MASK;
// revision    = tmp & MIDR_REV_MASK;

#define MIDR_PART_CA5    0xC05
#define MIDR_PART_CA8    0xC08
#define MIDR_PART_CA9    0xC09

unsigned int getMPIDR(void);

#define MPIDR_FORMAT_SHIFT  31
#define MPIDR_FORMAT_MASK   0x1
#define MPIDR_UBIT_SHIFT    30
#define MPIDR_UBIT_MASK     0x1
#define MPIDR_CLUSTER_SHIFT 7
#define MPIDR_CLUSTER_MASK  0xF
#define MPIDR_CPUID_SHIFT   0
#define MPIDR_CPUID_MASK    0x3

#define MPIDR_CPUID_CPU0    0x0
#define MPIDR_CPUID_CPU1    0x1
#define MPIDR_CPUID_CPU2    0x2
#define MPIDR_CPUID_CPU3    0x3

#define MPIDR_UNIPROCESSPR  0x1

#define MPDIR_NEW_FORMAT    0x1

// ------------------------------------------------------------
// Context ID

unsigned int getContextID(void);

void setContextID(unsigned int);

#define CONTEXTID_ASID_SHIFT   0
#define CONTEXTID_ASID_MASK    0xFF
#define CONTEXTID_PROCID_SHIFT 8
#define CONTEXTID_PROCID_MASK  0x00FFFFFF

// tmp    = getContextID();
// ASID   = tmp & CONTEXTID_ASID_MASK;
// PROCID = (tmp >> CONTEXTID_PROCID_SHIFT) & CONTEXTID_PROCID_MASK;

// ------------------------------------------------------------
// SMP related for ARMv7-A MPCore processors
//
// DO NOT CALL THESE FUNCTIONS ON A CORTEX-A8

// Returns the base address of the private peripheral memory space
unsigned int getBaseAddr(void);

// Returns the CPU ID (0 to 3) of the CPU executed on
#define MP_CPU0   (0)
#define MP_CPU1   (1)
#define MP_CPU2   (2)
#define MP_CPU3   (3)
unsigned int getCPUID(void);

// Set this core as participating in SMP
void joinSMP(void);

// Set this core as NOT participating in SMP
void leaveSMP(void);

// Go to sleep, never returns
void goToSleep(void);

#endif

// ------------------------------------------------------------
// End of v7.h
// ------------------------------------------------------------

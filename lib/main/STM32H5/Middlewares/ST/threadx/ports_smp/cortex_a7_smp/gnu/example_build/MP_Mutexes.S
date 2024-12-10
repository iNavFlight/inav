@; ------------------------------------------------------------
@; Cortex-A MPCore - Mutex Code
@;
@; Copyright (c) 2011-2012 ARM Ltd.  All rights reserved.
@; ------------------------------------------------------------
@
@  PRESERVE8
@
@  AREA  MP_Mutexes, CODE, READONLY
@
@  ;NOTES
@  ; struct mutex_t defined in MP_Mutexes.h
@  ; typedef struct mutex_t
@  ; {
@  ;   unsigned int lock;          <-- offset 0
@  ; }
@  ;
@  ; lock:   0xFF=unlocked  0x0 = Locked by CPU 0, 0x1 = Locked by CPU 1, 0x2 = Locked by CPU 2, 0x3 = Locked by CPU 3
@  ;

UNLOCKED =  0xFF

@; ------------------------------------------------------------
@
@  EXPORT initMutex
@  ; void initMutex(mutex_t* pMutex)
@  ; Places mutex into a known state
@  ; r0 = address of mutex_t
    .text
    .align 2
    .global $initMutex
    .type   $initMutex,function
initMutex:

  MOV     r1, #UNLOCKED           @ Mark as unlocked
  STR     r1, [r0]

  BX      lr


@; ------------------------------------------------------------
@
@  EXPORT lockMutex
@  ; void lockMutex(mutex_t* pMutex)
@  ; Blocking call, returns once successfully locked a mutex
@  ; r0 = address of mutex_t
    .text
    .align 2
    .global $lockMutex
    .type   $lockMutex,function
lockMutex:

  @ Is mutex locked?
  @ -----------------
  LDREX   r1, [r0]                @ Read lock field
  CMP     r1, #UNLOCKED           @ Compare with "unlocked"

  WFENE                           @ If mutex is locked, go into standby
  BNE     lockMutex               @ On waking re-check the mutex

  @ Attempt to lock mutex
  @ -----------------------
  MRC     p15, 0, r1, c0, c0, 5   @ Read CPU ID register
  AND     r1, r1, #0x03           @ Mask off, leaving the CPU ID field.
  STREX   r2, r1, [r0]            @ Attempt to lock mutex, by write CPU's ID to lock field
  CMP     r2, #0x0                @ Check whether store completed successfully (0=succeeded)
  BNE     lockMutex               @ If store failed, go back to beginning and try again

  DMB

  BX      lr                      @ Return as mutex is now locked by this cpu


@; ------------------------------------------------------------
@
@  EXPORT unlockMutex
@  ; unsigned int unlockMutex(mutex_t* pMutex)
@  ; Releases mutex, returns 0x0 for success and 0x1 for failure
@  ; r0 = address of mutex_t
    .text
    .align 2
    .global $unlockMutex
    .type   $unlockMutex,function
unlockMutex:

  @ Does this CPU own the mutex?
  @ -----------------------------
  MRC     p15, 0, r1, c0, c0, 5   @ Read CPU ID register
  AND     r1, r1, #0x03           @ Mask off, leaving the CPU ID in r1
  LDR     r2, [r0]                @ Read the lock field of the mutex
  CMP     r1, r2                  @ Compare ID of this CPU with the lock owner
  MOVNE   r0, #0x1                @ If ID doesn't match, return "fail"
  BXNE    lr


  @ Unlock mutex
  @ -------------
  DMB                             @ Ensure that accesses to shared resource have completed

  MOV     r1, #UNLOCKED           @ Write "unlocked" into lock field
  STR     r1, [r0]

  DSB                             @ Ensure that no instructions following the barrier execute until
                                  @ all memory accesses prior to the barrier have completed.

  SEV                             @ Send event to other CPUs, wakes anyone waiting on a mutex (using WFE)

  MOV     r0, #0x0                @ Return "success"
  BX      lr


@; ------------------------------------------------------------
@
@  EXPORT isMutexLocked
@  ; void isMutexLocked(mutex_t* pMutex)
@  ; Returns 0x0 if mutex unlocked, 0x1 is locked
@  ; r0 = address of mutex_t
    .text
    .align 2
    .global $isMutexLocked
    .type   $isMutexLocked,function
isMutexLocked:
  LDR     r0, [r0]
  CMP     r0, #UNLOCKED
  MOVEQ   r0, #0x0
  MOVNE   r0, #0x1
  BX      lr


@; ------------------------------------------------------------
@; End of code
@; ------------------------------------------------------------
@
@  END
@
@; ------------------------------------------------------------
@; End of MP_Mutexes.s
@; ------------------------------------------------------------

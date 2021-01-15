/*
    ChibiOS - Copyright (C) 2006..2018 Isidoro Orabona

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
 * @file    chtssi.c
 * @brief   Trusted services related API and definition.
 *
 * @addtogroup TSSI
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "chtssi.h"
#include "ARMCA5.h"
#if defined(__GNUC__) || defined(__DOXYGEN__)
#include "cmsis_gcc.h"
#else
#include "cmsis_armcc.h"
#endif
#include "ccportab.h"
#include <string.h>

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/
/* Granted timeslices to trusted service. DO NOT modify those values.
   They have non secure world counterparts.*/
typedef enum {
  TS_TIMEINT_1000_US  = 1000,
  TS_TIMEINT_10000_US = 10000
} ts_timeint_t;

#define LOWORD(in64) ((int64_t)in64 & 0x0FFFFFFFF)
#define TS_TIME2I(tmo)                                                      \
   (tmo == TS_TIMEINT_10000_US ? TIME_US2I(TS_TIMEINT_10000_US) :           \
                                 TIME_US2I(TS_TIMEINT_1000_US))
#define FDT_MAGIC 0xd00dfeed

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/* If a services file is missing in the user application.*/
CC_WEAK ts_state_t ts_state[TS_MAX_SVCS];
CC_WEAK const thread_descriptor_t ts_configs[TS_MAX_SVCS];
CC_WEAK const fc_descriptor_t ts_fc_configs[1];
uint32_t ts_fc_configs_n;

/* The reference to the suspended NSEC main thread.*/
thread_reference_t _ns_thread = NULL;

/* The services may broadcast and listen event flags via this object.*/
EVENTSOURCE_DECL(tsEventSource);

extern uint32_t __ram0_start__;

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

struct fdt_header {
  uint32_t magic;           /* magic word FDT_MAGIC */
  uint32_t totalsize;       /* total size of DT block */
  uint32_t off_dt_struct;   /* offset to structure */
  uint32_t off_dt_strings;  /* offset to strings */
  uint32_t off_mem_rsvmap;  /* offset to memory reserve map */
  uint32_t version;         /* format version */
  uint32_t last_comp_version;   /* last compatible version */

  /* version 2 fields below */
  uint32_t boot_cpuid_phys; /* Which physical CPU id we're
                               booting on */
  /* version 3 fields below */
  uint32_t size_dt_strings; /* size of the strings block */

  /* version 17 fields below */
  uint32_t size_dt_struct;  /* size of the structure block */
};

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

/* The ts module listen to the tsEventSource via this object.*/
static event_listener_t tsEventListener;

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

static inline uint32_t uswap32(uint32_t v)
{
  uint32_t result;

  __asm volatile ("rev %0, %1" : "=r" (result) : "r" (v));
  return result;
}

static bool isAddrSpaceValid(uint8_t *addr, size_t size)
{
  if (size == 0)
    return TRUE;
  return (bool)((addr - NSEC_MEMORY_START_ADDR) <
                (NSEC_MEMORY_END_ADDR - NSEC_MEMORY_START_ADDR)) &&
         (bool)((addr + size - NSEC_MEMORY_START_ADDR) <
                (NSEC_MEMORY_END_ADDR - NSEC_MEMORY_START_ADDR));
}

static bool isHndlValid(ts_state_t *handle)
{
  if ((handle < TS_STATE(0)) || (handle >= TS_STATE(TS_MAX_SVCS)))
    return FALSE;
  if (((char *)handle - (char *)TS_STATE(0)) % sizeof *TS_STATE(0))
    return FALSE;
  return TRUE;
}

static ts_state_t *findSvcsEntry(const char *name)
{
  int i;
  for (i = 0; i < TS_MAX_SVCS; ++i) {
    if (TS_CONF_TABLE(i)->name == NULL)
      continue;
    if (!strcmp(TS_CONF_TABLE(i)->name, name))
      return TS_CONF_TABLE(i)->arg;
  }
  return NULL;
}

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   The trusted service call entry point.
 * @pre     The foreign interrupts are disabled.
 * @post    A request is passed to the thread registered for the service.
 * @post    The service thread is resumed.
 *
 * @param[in] svc_handle    the handle of the service to be invoked.
 * @param[in,out] svc_data  service request data, often a reference to a more
 *                          complex structure.
 * @param[in] svc_datalen   size of the svc_data memory area.
 * @param[in] svc_timeout   after this time interval, the service execution
 *                          will be interrupted. Time is in microseconds.
 *                          This interval represents the time slice granted
 *                          to the services to continue their work.
 *
 * @return                  A 64bit value. It is the OR of the 32bit service
 *                          status combined with a 32bit event mask (in the
 *                          hi-word).
 *                          The retval values are returned in the lower word
 *                          as 32bit int.
 * @retval SMC_SVC_OK       generic success value.
 * @retval SMC_SVC_INTR     call interrupted.
 * @retval SMC_SVC_BUSY     the service has a pending request.
 * @retval SMC_SVC_INVALID  bad parameters.
 * @retval SMC_SVC_NOENT    no such service.
 * @retval SMC_SVC_BADH     bad handle.
 *
 * @notapi
 */
int64_t smcEntry(ts_state_t *svc_handle, ts_params_area_t svc_data,
               size_t svc_datalen, ts_timeint_t svc_timeout) {
  ts_state_t *tssp = NULL;
  msg_t r;

  if (svc_handle == TS_HND_STQRY) {

    /* Internal query status service.*/
    ts_state_t *tsqryd;

    /* svc_data is the handle of the service to whom 'query' the state.*/
    tsqryd = (ts_state_t *)svc_data;

    /* handle argument validation.*/
    if (!isHndlValid(tsqryd))
      return LOWORD(SMC_SVC_BADH);

    /* if the service has done, return its last status.*/
    if (tsqryd->ts_thdp != NULL) {
      r = tsqryd->ts_status;
      return LOWORD(r);
    }
  }
  else if (svc_handle != TS_HND_IDLE) {
    if (!isAddrSpaceValid(svc_data, svc_datalen))
      return LOWORD(SMC_SVC_INVALID);

    uint32_t i = (uint32_t)svc_handle;

    if ((i & TS_FASTCALL_MASK) == TS_FASTCALL_MASK) {

      /* Fast call user service.*/
      i &= ~TS_FASTCALL_MASK;

      if (i >= ts_fc_configs_n)
        return LOWORD(SMC_SVC_BADH);

      return TS_FC_CONF_TABLE(i)->funcp(svc_data, svc_datalen);
    }
    else if (svc_handle == TS_HND_VERSION) {

      /* Internal get version service.*/
      return LOWORD(TSSI_VERSION);
    }
    else if (svc_handle == TS_HND_DISCOVERY) {

      /* Internal discovery service.*/
      if (svc_datalen) {
        *((char *)svc_data + svc_datalen - 1) = '\0';
        tssp = findSvcsEntry((char *)svc_data);
      }
      if (tssp == NULL)
        return LOWORD(SMC_SVC_NOENT);
      return LOWORD((int32_t)tssp);
    }
    else {

      /* User service.*/
      if (!isHndlValid(svc_handle))
        return LOWORD(SMC_SVC_BADH);
      tssp = svc_handle;

      /* If the service is not waiting requests, it's busy.*/
      if (tssp->ts_thdp == NULL)
        return LOWORD(SMC_SVC_BUSY);
      tssp->ts_datap = svc_data;
      tssp->ts_datalen = svc_datalen;
    }
  }

#if (CH_DBG_SYSTEM_STATE_CHECK == TRUE)
  _dbg_check_lock();
#endif

  /* Limit the max timeout interval.*/
  if (svc_timeout > TS_MAX_TMO)
    svc_timeout = TS_MAX_TMO;

  if (tssp)
    chThdResumeS(&tssp->ts_thdp, MSG_OK);
  r = chThdSuspendTimeoutS(&_ns_thread, TS_TIME2I(svc_timeout));

  /* Map MSG_TIMEOUT to SMC_SVC_INTR.*/
  if (r == MSG_TIMEOUT)
    r = SMC_SVC_INTR;

  /* Get and clear any pending event flags.*/
  eventflags_t f = chEvtGetAndClearFlagsI(&tsEventListener);

#if (CH_DBG_SYSTEM_STATE_CHECK == TRUE)
  _dbg_check_unlock();
#endif
  return LOWORD(r) | ((int64_t)f << 32);
}

/**
 * @brief   The calling thread is a service and wait the arrival of
 *          a request.
 * @post    The service object state is filled with the parameters of
 *          the requester.
 *
 * @param[in] svcp          the service object reference.
 *
 * @return                  The wakeup message.
 * @retval MSG_OK           a new request has to be processed.
 *
 * @api
 */
msg_t tssiWaitRequest(ts_state_t *svcp)
{
  msg_t r;

  chDbgCheck(svcp != NULL);

  chSysLock();
  if (_ns_thread) {
    /* Ack a previous service invocation. Not schedule.*/
    chThdResumeI(&_ns_thread, SMC_SVC_INTR);
  }
  r = chThdSuspendS(&svcp->ts_thdp);
  chSysUnlock();
  return r;
}

/**
 * @brief   Check that the specified memory space is a subspace of
 *          the non secure memory space.
 *
 * @param[in] addr    start address of the memory space.
 * @param[in] size    size of the memory space.
 *
 * @return            TRUE, if the space is valid.
 *
 * @api
 */
bool tsIsAddrSpaceValid(void *addr, size_t size)
{
    return isAddrSpaceValid((uint8_t *)addr, size);
}

/**
 * @brief   Initializes the trusted services and jumps in the NSEC world.
 *
 * @init
 */
CC_NO_RETURN void tssiInit(void)
{
  int32_t i;
  uint32_t d;
  uint32_t *tt;
  struct fdt_header *pfdt = (struct fdt_header *)NSEC_MEMORY_START_ADDR;
  void *moveto = NULL;

  /*
   * The main DDR memory, PORT0, is divided in 4 region, each 32MB large.
   * The last region is split in two areas, each 16MB large.
   * The first 3 region and the lower area of this last region is non secure.
   * All the rest of the regions space is secured.
   * The same applies to AESB view of the DDR, PORT1, and LCDC view.
   *
   * Those settings depend on the designed memory mapping.
   */
  mtxSetSlaveRegionSize(MATRIX0, H64MX_SLAVE_DDR_PORT0, MATRIX_AREA_SIZE_32M, REGION_0_MSK);
  mtxSetSlaveRegionSize(MATRIX0, H64MX_SLAVE_DDR_PORT1, MATRIX_AREA_SIZE_32M, REGION_0_MSK);
  mtxSetSlaveRegionSize(MATRIX0, H64MX_SLAVE_DDR_PORT2, MATRIX_AREA_SIZE_32M, REGION_0_MSK);
  mtxSetSlaveRegionSize(MATRIX0, H64MX_SLAVE_DDR_PORT3, MATRIX_AREA_SIZE_32M, REGION_0_MSK);

  mtxSetSlaveSplitAddr(MATRIX0, H64MX_SLAVE_DDR_PORT0, MATRIX_AREA_SIZE_32M,
      REGION_0_MSK | REGION_1_MSK | REGION_2_MSK);
  mtxSetSlaveSplitAddr(MATRIX0, H64MX_SLAVE_DDR_PORT1, MATRIX_AREA_SIZE_32M,
      REGION_0_MSK | REGION_1_MSK | REGION_2_MSK);
  mtxSetSlaveSplitAddr(MATRIX0, H64MX_SLAVE_DDR_PORT2, MATRIX_AREA_SIZE_32M,
      REGION_0_MSK | REGION_1_MSK | REGION_2_MSK);
  mtxSetSlaveSplitAddr(MATRIX0, H64MX_SLAVE_DDR_PORT3, MATRIX_AREA_SIZE_32M,
      REGION_0_MSK | REGION_1_MSK | REGION_2_MSK);

  mtxSetSlaveSplitAddr(MATRIX0, H64MX_SLAVE_DDR_PORT0, MATRIX_AREA_SIZE_16M, REGION_3_MSK);
  mtxSetSlaveSplitAddr(MATRIX0, H64MX_SLAVE_DDR_PORT1, MATRIX_AREA_SIZE_16M, REGION_3_MSK);
  mtxSetSlaveSplitAddr(MATRIX0, H64MX_SLAVE_DDR_PORT2, MATRIX_AREA_SIZE_16M, REGION_3_MSK);
  mtxSetSlaveSplitAddr(MATRIX0, H64MX_SLAVE_DDR_PORT3, MATRIX_AREA_SIZE_16M, REGION_3_MSK);

  mtxConfigSlaveSec(MATRIX0, H64MX_SLAVE_DDR_PORT0,
      mtxRegionLansech(REGION_0, UPPER_AREA_SECURABLE) |
      mtxRegionLansech(REGION_1, UPPER_AREA_SECURABLE) |
      mtxRegionLansech(REGION_2, UPPER_AREA_SECURABLE) |
      mtxRegionLansech(REGION_3, UPPER_AREA_SECURABLE),
      mtxRegionRdnsech(REGION_0, NOT_SECURE_READ) |
      mtxRegionRdnsech(REGION_1, NOT_SECURE_READ) |
      mtxRegionRdnsech(REGION_2, NOT_SECURE_READ),
      mtxRegionWrnsech(REGION_0, NOT_SECURE_WRITE) |
      mtxRegionWrnsech(REGION_1, NOT_SECURE_WRITE) |
      mtxRegionWrnsech(REGION_2, NOT_SECURE_WRITE));

  mtxConfigSlaveSec(MATRIX0, H64MX_SLAVE_DDR_PORT1,
      mtxRegionLansech(REGION_0, UPPER_AREA_SECURABLE) |
      mtxRegionLansech(REGION_1, UPPER_AREA_SECURABLE) |
      mtxRegionLansech(REGION_2, UPPER_AREA_SECURABLE) |
      mtxRegionLansech(REGION_3, UPPER_AREA_SECURABLE),
      mtxRegionRdnsech(REGION_0, NOT_SECURE_READ) |
      mtxRegionRdnsech(REGION_1, NOT_SECURE_READ) |
      mtxRegionRdnsech(REGION_2, NOT_SECURE_READ),
      mtxRegionWrnsech(REGION_0, NOT_SECURE_WRITE) |
      mtxRegionWrnsech(REGION_1, NOT_SECURE_WRITE) |
      mtxRegionWrnsech(REGION_2, NOT_SECURE_WRITE));

  mtxConfigSlaveSec(MATRIX0, H64MX_SLAVE_DDR_PORT2,
      mtxRegionLansech(REGION_0, UPPER_AREA_SECURABLE) |
      mtxRegionLansech(REGION_1, UPPER_AREA_SECURABLE) |
      mtxRegionLansech(REGION_2, UPPER_AREA_SECURABLE) |
      mtxRegionLansech(REGION_3, UPPER_AREA_SECURABLE),
      mtxRegionRdnsech(REGION_0, NOT_SECURE_READ) |
      mtxRegionRdnsech(REGION_1, NOT_SECURE_READ) |
      mtxRegionRdnsech(REGION_2, NOT_SECURE_READ),
      mtxRegionWrnsech(REGION_0, NOT_SECURE_WRITE) |
      mtxRegionWrnsech(REGION_1, NOT_SECURE_WRITE) |
      mtxRegionWrnsech(REGION_2, NOT_SECURE_WRITE));

  mtxConfigSlaveSec(MATRIX0, H64MX_SLAVE_DDR_PORT3,
      mtxRegionLansech(REGION_0, UPPER_AREA_SECURABLE) |
      mtxRegionLansech(REGION_1, UPPER_AREA_SECURABLE) |
      mtxRegionLansech(REGION_2, UPPER_AREA_SECURABLE) |
      mtxRegionLansech(REGION_3, UPPER_AREA_SECURABLE),
      mtxRegionRdnsech(REGION_0, NOT_SECURE_READ) |
      mtxRegionRdnsech(REGION_1, NOT_SECURE_READ) |
      mtxRegionRdnsech(REGION_2, NOT_SECURE_READ),
      mtxRegionWrnsech(REGION_0, NOT_SECURE_WRITE) |
      mtxRegionWrnsech(REGION_1, NOT_SECURE_WRITE) |
      mtxRegionWrnsech(REGION_2, NOT_SECURE_WRITE));

#if !SAMA_USE_SDMMC

  /* Configure the SDMMCx regions as non secure.*/
  mtxSetSlaveSplitAddr(MATRIX0, H64MX_SLAVE_SDMMC, MATRIX_AREA_SIZE_128M, REGION_1_MSK|REGION_2_MSK);
  mtxConfigSlaveSec(MATRIX0, H64MX_SLAVE_SDMMC,
      mtxRegionLansech(REGION_1, UPPER_AREA_SECURABLE) |
      mtxRegionLansech(REGION_2, UPPER_AREA_SECURABLE),
      mtxRegionRdnsech(REGION_1, NOT_SECURE_READ) |
      mtxRegionRdnsech(REGION_2, NOT_SECURE_READ),
      mtxRegionWrnsech(REGION_1, NOT_SECURE_WRITE) |
      mtxRegionWrnsech(REGION_2, NOT_SECURE_WRITE));
#endif

  /* Mark the whole non secure memory region non executable
     by the secure side, and set the Non-Secure access bit, so that
     any access to this region is in the non-secure physical
     space. This ensures the coherence of the cache between
     secure and non secure accesses.*/
  tt = (uint32_t *)(__get_TTBR0() & 0xFFFFC000);
  for (d = ((uint32_t)NSEC_MEMORY_START_ADDR >> 20);
       d < ((uint32_t)NSEC_MEMORY_END_ADDR >> 20); d += 1) {
    MMU_SecureSection(tt + d, NON_SECURE);
    MMU_XNSection(tt + d, NON_EXECUTE);
  }

  /* The same, but for the AESB view of the DDR memory region.*/
  for (d = ((uint32_t)(NSEC_MEMORY_START_ADDR + 0x20000000) >> 20);
       d < ((uint32_t)(NSEC_MEMORY_END_ADDR + 0x20000000) >> 20); d += 1) {
    MMU_SecureSection(tt + d, NON_SECURE);
    MMU_XNSection(tt + d, NON_EXECUTE);
  }
  MMU_InvalidateTLB();

  /* Flush the modified MMU table.*/
  cacheCleanRegion(tt, d * sizeof (uint32_t));
  __DSB();
  __ISB();

  /* Make sure that prio is NORMALPRIO.*/
  chThdSetPriority(NORMALPRIO);

  /* Initialize the services.*/
  for (i = 0; i < TS_MAX_SVCS; ++i) {
    if (TS_CONF_TABLE(i)->arg == NULL)
      continue;

    /* Check that the initialization of the TS_TABLE against TS_STATE_TABLE
       has been set right.*/
    if (TS_CONF_TABLE(i)->arg != TS_STATE(i)) {
      chSysHalt("Bad TS_STATE setting in the services configuration table.");
    }

    /* Check that the service priority has been set right.*/
    if ((TS_CONF_TABLE(i)->prio <= NORMALPRIO) ||
        (TS_CONF_TABLE(i)->prio >= HIGHPRIO)) {
      chSysHalt("Bad prio setting in the services configuration table.");
    }

    /* Create the service thread.*/
    chThdCreate(TS_CONF_TABLE(i));
  }

  /* Fast call services.*/
  for (i = 0; TS_FC_CONF_TABLE(i)->name; ++i) {

    /* Check that the 'code' field of the
       fast call table has been set right.*/
    if ((TS_FC_CONF_TABLE(i)->code &~ TS_FASTCALL_MASK) != (uint32_t)i) {
      chSysHalt("Bad 'code' setting in the fast call configuration table.");
    }
  }
  ts_fc_configs_n = i;

  /* Register to the daemon services events. All flags.*/
  chEvtRegister(&tsEventSource, &tsEventListener, EVT_DAEMON_REQ_ATN);

  /* Now set the priority at the max.*/
  chThdSetPriority(HIGHPRIO);

  /* Allow non secure access to CP10 and CP11.*/
  asm volatile (
    "mrc p15, 0, r0, c1, c1, 2    \n"
    "orr r0, r0, #0b11<<10        \n"
    "mcr p15, 0, r0, c1, c1, 2    \n"
  );

  /* Check if a fdt image exists at the start
     of the non secure memory region.*/
  if (uswap32(pfdt->magic) == FDT_MAGIC) {
    uint32_t fdtsize;

    /* Detected a fdt structure.
       Move it to the end of non secure area.*/
    fdtsize = uswap32(pfdt->totalsize);
    fdtsize = (fdtsize + 4095) &~ 4095;
    moveto = (void *)(SEC_MEMORY_START_ADDR - fdtsize);
    memmove(moveto, pfdt, fdtsize);

    /* Invalidate the original fdt image.*/
    pfdt->magic = 0;
  }

  /* Jump in the NON SECURE world.
     This thread becomes the non secure environment as view by
     the secure world.*/

  /* r2 address of the moved fdt, if any.*/
  asm volatile (
    "mov r2, %0 \n" :: "r" (moveto)
  );

  _ns_trampoline(NSEC_MEMORY_START_ADDR + NSEC_MEMORY_EXE_OFFSET);

  /* It never goes here.*/
}

/** @} */

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
 * @file    hal_serial_nor.c
 * @brief   Serial NOR serial flash driver code.
 *
 * @addtogroup HAL_SERIAL_NOR
 * @{
 */

#include "hal.h"
#include "hal_serial_nor.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

static const flash_descriptor_t *snor_get_descriptor(void *instance);
static flash_error_t snor_read(void *instance, flash_offset_t offset,
                               size_t n, uint8_t *rp);
static flash_error_t snor_program(void *instance, flash_offset_t offset,
                                  size_t n, const uint8_t *pp);
static flash_error_t snor_start_erase_all(void *instance);
static flash_error_t snor_start_erase_sector(void *instance,
                                             flash_sector_t sector);
static flash_error_t snor_verify_erase(void *instance,
                                       flash_sector_t sector);
static flash_error_t snor_query_erase(void *instance, uint32_t *msec);
static flash_error_t snor_read_sfdp(void *instance, flash_offset_t offset,
                                    size_t n, uint8_t *rp);

/**
 * @brief   Virtual methods table.
 */
static const struct SNORDriverVMT snor_vmt = {
  (size_t)0,
  snor_get_descriptor, snor_read, snor_program,
  snor_start_erase_all, snor_start_erase_sector,
  snor_query_erase, snor_verify_erase,
  snor_read_sfdp
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

#if ((SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI) &&                           \
     (SNOR_SHARED_BUS == TRUE)) || defined(__DOXYGEN__)
/**
 * @brief   Bus acquisition and lock.
 *
 * @param[in] busp      pointer to the bus driver
 * @param[in] config    bus configuration
 *
 * @notapi
 */
static void bus_acquire(BUSDriver *busp, const BUSConfig *config) {

  (void)config;

  wspiAcquireBus(busp);
  if (busp->config != config) {
    wspiStart(busp, config);
  }
}

/**
 * @brief   Bus release.
 *
 * @param[in] busp      pointer to the bus driver
 *
 * @notapi
 */
static void bus_release(BUSDriver *busp) {

  wspiReleaseBus(busp);
}

#elif (SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_SPI) &&                           \
      (SNOR_SHARED_BUS == TRUE)
void bus_acquire(BUSDriver *busp, const BUSConfig *config) {

  spiAcquireBus(busp);
  if (busp->config != config) {
    spiStart(busp, config);
  }
}

void bus_release(BUSDriver *busp) {

  spiReleaseBus(busp);
}

#else
/* No bus sharing, empty macros.*/
#define bus_acquire(busp)
#define bus_release(busp)
#endif

/**
 * @brief   Returns a pointer to the device descriptor.
 *
 * @param[in] instance  instance pointer
 * @return              Pointer to a static descriptor structure.
 */
static const flash_descriptor_t *snor_get_descriptor(void *instance) {
  SNORDriver *devp = (SNORDriver *)instance;

  osalDbgCheck(instance != NULL);
  osalDbgAssert((devp->state != FLASH_UNINIT) && (devp->state != FLASH_STOP),
                "invalid state");

  return &snor_descriptor;
}

static flash_error_t snor_read(void *instance, flash_offset_t offset,
                               size_t n, uint8_t *rp) {
  SNORDriver *devp = (SNORDriver *)instance;
  flash_error_t err;

  osalDbgCheck((instance != NULL) && (rp != NULL) && (n > 0U));
  osalDbgCheck((size_t)offset + n <= (size_t)snor_descriptor.sectors_count *
                                     (size_t)snor_descriptor.sectors_size);
  osalDbgAssert((devp->state == FLASH_READY) || (devp->state == FLASH_ERASE),
                "invalid state");

  if (devp->state == FLASH_ERASE) {
    return FLASH_BUSY_ERASING;
  }

  /* Bus acquired.*/
  bus_acquire(devp->config->busp, devp->config->buscfg);

  /* FLASH_READY state while the operation is performed.*/
  devp->state = FLASH_READ;

  /* Actual read implementation.*/
  err = snor_device_read(devp, offset, n, rp);

  /* Ready state again.*/
  devp->state = FLASH_READY;

  /* Bus released.*/
  bus_release(devp->config->busp);

  return err;
}

static flash_error_t snor_program(void *instance, flash_offset_t offset,
                                  size_t n, const uint8_t *pp) {
  SNORDriver *devp = (SNORDriver *)instance;
  flash_error_t err;

  osalDbgCheck((instance != NULL) && (pp != NULL) && (n > 0U));
  osalDbgCheck((size_t)offset + n <= (size_t)snor_descriptor.sectors_count *
                                     (size_t)snor_descriptor.sectors_size);
  osalDbgAssert((devp->state == FLASH_READY) || (devp->state == FLASH_ERASE),
                "invalid state");

  if (devp->state == FLASH_ERASE) {
    return FLASH_BUSY_ERASING;
  }

  /* Bus acquired.*/
  bus_acquire(devp->config->busp, devp->config->buscfg);

  /* FLASH_PGM state while the operation is performed.*/
  devp->state = FLASH_PGM;

  /* Actual program implementation.*/
  err = snor_device_program(devp, offset, n, pp);

  /* Ready state again.*/
  devp->state = FLASH_READY;

  /* Bus released.*/
  bus_release(devp->config->busp);

  return err;
}

static flash_error_t snor_start_erase_all(void *instance) {
  SNORDriver *devp = (SNORDriver *)instance;
  flash_error_t err;

  osalDbgCheck(instance != NULL);
  osalDbgAssert((devp->state == FLASH_READY) || (devp->state == FLASH_ERASE),
                "invalid state");

  if (devp->state == FLASH_ERASE) {
    return FLASH_BUSY_ERASING;
  }

  /* Bus acquired.*/
  bus_acquire(devp->config->busp, devp->config->buscfg);

  /* FLASH_ERASE state while the operation is performed.*/
  devp->state = FLASH_ERASE;

  /* Actual erase implementation.*/
  err = snor_device_start_erase_all(devp);

  /* Ready state again.*/
  devp->state = FLASH_READY;

  /* Bus released.*/
  bus_release(devp->config->busp);

  return err;
}

static flash_error_t snor_start_erase_sector(void *instance,
                                             flash_sector_t sector) {
  SNORDriver *devp = (SNORDriver *)instance;
  flash_error_t err;

  osalDbgCheck(instance != NULL);
  osalDbgCheck(sector < snor_descriptor.sectors_count);
  osalDbgAssert((devp->state == FLASH_READY) || (devp->state == FLASH_ERASE),
                "invalid state");

  if (devp->state == FLASH_ERASE) {
    return FLASH_BUSY_ERASING;
  }

  /* Bus acquired.*/
  bus_acquire(devp->config->busp, devp->config->buscfg);

  /* FLASH_ERASE state while the operation is performed.*/
  devp->state = FLASH_ERASE;

  /* Actual erase implementation.*/
  err = snor_device_start_erase_sector(devp, sector);

  /* Bus released.*/
  bus_release(devp->config->busp);

  return err;
}

static flash_error_t snor_verify_erase(void *instance,
                                       flash_sector_t sector) {
  SNORDriver *devp = (SNORDriver *)instance;
  flash_error_t err;

  osalDbgCheck(instance != NULL);
  osalDbgCheck(sector < snor_descriptor.sectors_count);
  osalDbgAssert((devp->state == FLASH_READY) || (devp->state == FLASH_ERASE),
                "invalid state");

  if (devp->state == FLASH_ERASE) {
    return FLASH_BUSY_ERASING;
  }

  /* Bus acquired.*/
  bus_acquire(devp->config->busp, devp->config->buscfg);

  /* FLASH_READY state while the operation is performed.*/
  devp->state = FLASH_READ;

  /* Actual verify erase implementation.*/
  err = snor_device_verify_erase(devp, sector);

  /* Ready state again.*/
  devp->state = FLASH_READY;

  /* Bus released.*/
  bus_release(devp->config->busp);

  return err;
}

static flash_error_t snor_query_erase(void *instance, uint32_t *msec) {
  SNORDriver *devp = (SNORDriver *)instance;
  flash_error_t err;

  osalDbgCheck(instance != NULL);
  osalDbgAssert((devp->state == FLASH_READY) || (devp->state == FLASH_ERASE),
                "invalid state");

  /* If there is an erase in progress then the device must be checked.*/
  if (devp->state == FLASH_ERASE) {

    /* Bus acquired.*/
    bus_acquire(devp->config->busp, devp->config->buscfg);

    /* Actual query erase implementation.*/
    err = snor_device_query_erase(devp, msec);

    /* The device is ready to accept commands.*/
    if (err == FLASH_NO_ERROR) {
      devp->state = FLASH_READY;
    }

    /* Bus released.*/
    bus_release(devp->config->busp);
  }
  else {
    err = FLASH_NO_ERROR;
  }

  return err;
}

static flash_error_t snor_read_sfdp(void *instance, flash_offset_t offset,
                                    size_t n, uint8_t *rp) {
  SNORDriver *devp = (SNORDriver *)instance;
  flash_error_t err;

  osalDbgCheck((instance != NULL) && (rp != NULL) && (n > 0U));
  osalDbgAssert((devp->state == FLASH_READY) || (devp->state == FLASH_ERASE),
                "invalid state");

  if (devp->state == FLASH_ERASE) {
    return FLASH_BUSY_ERASING;
  }

  /* Bus acquired.*/
  bus_acquire(devp->config->busp, devp->config->buscfg);

  /* Actual read SFDP implementation.*/
  err = snor_device_read_sfdp(devp, offset, n, rp);

  /* The device is ready to accept commands.*/
  if (err == FLASH_NO_ERROR) {
    devp->state = FLASH_READY;
  }

  /* Bus released.*/
  bus_release(devp->config->busp);

  return err;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Stops the underlying bus driver.
 *
 * @param[in] busp      pointer to the bus driver
 *
 * @notapi
 */
void bus_stop(BUSDriver *busp) {

#if SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI
  wspiStop(busp);
#else
  spiStop(busp);
#endif
}

/**
 * @brief   Sends a naked command.
 *
 * @param[in] busp      pointer to the bus driver
 * @param[in] cmd       instruction code
 *
 * @notapi
 */
void bus_cmd(BUSDriver *busp, uint32_t cmd) {
#if SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI
  wspi_command_t mode;

  mode.cmd   = cmd;
  mode.cfg   = SNOR_WSPI_CFG_CMD;
  mode.addr  = 0U;
  mode.alt   = 0U;
  mode.dummy = 0U;
  wspiCommand(busp, &mode);
#else
  uint8_t buf[1];

  spiSelect(busp);
  buf[0] = cmd;
  spiSend(busp, 1, buf);
  spiUnselect(busp);
#endif
}

/**
 * @brief   Sends a command followed by a data transmit phase.
 *
 * @param[in] busp      pointer to the bus driver
 * @param[in] cmd       instruction code
 * @param[in] n         number of bytes to receive
 * @param[in] p         data buffer
 *
 * @notapi
 */
void bus_cmd_send(BUSDriver *busp, uint32_t cmd, size_t n, const uint8_t *p) {
#if SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI
  wspi_command_t mode;

  mode.cmd   = cmd;
  mode.cfg   = SNOR_WSPI_CFG_CMD_DATA;
  mode.addr  = 0U;
  mode.alt   = 0U;
  mode.dummy = 0U;
  wspiSend(busp, &mode, n, p);
#else
  uint8_t buf[1];

  spiSelect(busp);
  buf[0] = cmd;
  spiSend(busp, 1, buf);
  spiSend(busp, n, p);
  spiUnselect(busp);
#endif
}

/**
 * @brief   Sends a command followed by a data receive phase.
 *
 * @param[in] busp      pointer to the bus driver
 * @param[in] cmd       instruction code
 * @param[in] n         number of bytes to receive
 * @param[out] p        data buffer
 *
 * @notapi
 */
void bus_cmd_receive(BUSDriver *busp,
                     uint32_t cmd,
                     size_t n,
                     uint8_t *p) {
#if SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI
  wspi_command_t mode;

  mode.cmd   = cmd;
  mode.cfg   = SNOR_WSPI_CFG_CMD_DATA;
  mode.addr  = 0U;
  mode.alt   = 0U;
  mode.dummy = 0U;
  wspiReceive(busp, &mode, n, p);
#else
  uint8_t buf[1];

  spiSelect(busp);
  buf[0] = cmd;
  spiSend(busp, 1, buf);
  spiReceive(busp, n, p);
  spiUnselect(busp);
#endif
}

/**
 * @brief   Sends a command followed by a flash address.
 *
 * @param[in] busp      pointer to the bus driver
 * @param[in] cmd       instruction code
 * @param[in] offset    flash offset
 *
 * @notapi
 */
void bus_cmd_addr(BUSDriver *busp, uint32_t cmd, flash_offset_t offset) {
#if SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI
  wspi_command_t mode;

  mode.cmd   = cmd;
  mode.cfg   = SNOR_WSPI_CFG_CMD_ADDR;
  mode.addr  = offset;
  mode.alt   = 0U;
  mode.dummy = 0U;
  wspiCommand(busp, &mode);
#else
  uint8_t buf[4];

  spiSelect(busp);
  buf[0] = cmd;
  buf[1] = (uint8_t)(offset >> 16);
  buf[2] = (uint8_t)(offset >> 8);
  buf[3] = (uint8_t)(offset >> 0);
  spiSend(busp, 4, buf);
  spiUnselect(busp);
#endif
}

/**
 * @brief   Sends a command followed by a flash address and a data transmit
 *          phase.
 *
 * @param[in] busp      pointer to the bus driver
 * @param[in] cmd       instruction code
 * @param[in] offset    flash offset
 * @param[in] n         number of bytes to receive
 * @param[in] p         data buffer
 *
 * @notapi
 */
void bus_cmd_addr_send(BUSDriver *busp,
                       uint32_t cmd,
                       flash_offset_t offset,
                       size_t n,
                       const uint8_t *p) {
#if SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI
  wspi_command_t mode;

  mode.cmd   = cmd;
  mode.cfg   = SNOR_WSPI_CFG_CMD_ADDR_DATA;
  mode.addr  = offset;
  mode.alt   = 0U;
  mode.dummy = 0U;
  wspiSend(busp, &mode, n, p);
#else
  uint8_t buf[4];

  spiSelect(busp);
  buf[0] = cmd;
  buf[1] = (uint8_t)(offset >> 16);
  buf[2] = (uint8_t)(offset >> 8);
  buf[3] = (uint8_t)(offset >> 0);
  spiSend(busp, 4, buf);
  spiSend(busp, n, p);
  spiUnselect(busp);
#endif
}

/**
 * @brief   Sends a command followed by a flash address and a data receive
 *          phase.
 *
 * @param[in] busp      pointer to the bus driver
 * @param[in] cmd       instruction code
 * @param[in] offset    flash offset
 * @param[in] n         number of bytes to receive
 * @param[out] p        data buffer
 *
 * @notapi
 */
void bus_cmd_addr_receive(BUSDriver *busp,
                          uint32_t cmd,
                          flash_offset_t offset,
                          size_t n,
                          uint8_t *p) {
#if SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI
  wspi_command_t mode;

  mode.cmd   = cmd;
  mode.cfg   = SNOR_WSPI_CFG_CMD_ADDR_DATA;
  mode.addr  = offset;
  mode.alt   = 0U;
  mode.dummy = 0U;
  wspiReceive(busp, &mode, n, p);
#else
  uint8_t buf[4];

  spiSelect(busp);
  buf[0] = cmd;
  buf[1] = (uint8_t)(offset >> 16);
  buf[2] = (uint8_t)(offset >> 8);
  buf[3] = (uint8_t)(offset >> 0);
  spiSend(busp, 4, buf);
  spiReceive(busp, n, p);
  spiUnselect(busp);
#endif
}

#if (SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI) || defined(__DOXYGEN__)
/**
 * @brief   Sends a command followed by dummy cycles and a
 *          data receive phase.
 *
 * @param[in] busp      pointer to the bus driver
 * @param[in] cmd       instruction code
 * @param[in] dummy     number of dummy cycles
 * @param[in] n         number of bytes to receive
 * @param[out] p        data buffer
 *
 * @notapi
 */
void bus_cmd_dummy_receive(BUSDriver *busp,
                           uint32_t cmd,
                           uint32_t dummy,
                           size_t n,
                           uint8_t *p) {
  wspi_command_t mode;

  mode.cmd   = cmd;
  mode.cfg   = SNOR_WSPI_CFG_CMD_DATA;
  mode.addr  = 0U;
  mode.alt   = 0U;
  mode.dummy = dummy;
  wspiReceive(busp, &mode, n, p);
}

/**
 * @brief   Sends a command followed by a flash address, dummy cycles and a
 *          data receive phase.
 *
 * @param[in] busp      pointer to the bus driver
 * @param[in] cmd       instruction code
 * @param[in] offset    flash offset
 * @param[in] dummy     number of dummy cycles
 * @param[in] n         number of bytes to receive
 * @param[out] p        data buffer
 *
 * @notapi
 */
void bus_cmd_addr_dummy_receive(BUSDriver *busp,
                                uint32_t cmd,
                                flash_offset_t offset,
                                uint32_t dummy,
                                size_t n,
                                uint8_t *p) {
  wspi_command_t mode;

  mode.cmd   = cmd;
  mode.cfg   = SNOR_WSPI_CFG_CMD_ADDR_DATA;
  mode.addr  = offset;
  mode.alt   = 0U;
  mode.dummy = dummy;
  wspiReceive(busp, &mode, n, p);
}
#endif /* SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI */

/**
 * @brief   Initializes an instance.
 *
 * @param[out] devp     pointer to the @p SNORDriver object
 *
 * @init
 */
void snorObjectInit(SNORDriver *devp) {

  osalDbgCheck(devp != NULL);

  devp->vmt         = &snor_vmt;
  devp->state       = FLASH_STOP;
  devp->config      = NULL;
}

/**
 * @brief   Configures and activates SNOR driver.
 *
 * @param[in] devp      pointer to the @p SNORDriver object
 * @param[in] config    pointer to the configuration
 *
 * @api
 */
void snorStart(SNORDriver *devp, const SNORConfig *config) {

  osalDbgCheck((devp != NULL) && (config != NULL));
  osalDbgAssert(devp->state != FLASH_UNINIT, "invalid state");

  devp->config = config;

  if (devp->state == FLASH_STOP) {

    /* Bus acquisition.*/
    bus_acquire(devp->config->busp, devp->config->buscfg);

    /* Device identification and initialization.*/
    snor_device_init(devp);

    /* Driver in ready state.*/
    devp->state = FLASH_READY;

    /* Bus release.*/
    bus_release(devp->config->busp);
  }
} 

/**
 * @brief   Deactivates the SNOR driver.
 *
 * @param[in] devp      pointer to the @p SNORDriver object
 *
 * @api
 */
void snorStop(SNORDriver *devp) {

  osalDbgCheck(devp != NULL);
  osalDbgAssert(devp->state != FLASH_UNINIT, "invalid state");

  if (devp->state != FLASH_STOP) {

    /* Bus acquisition.*/
    bus_acquire(devp->config->busp, devp->config->buscfg);

    /* Stopping bus device.*/
    bus_stop(devp->config->busp);

    /* Driver stopped.*/
    devp->state = FLASH_STOP;

    /* Bus release.*/
    bus_release(devp->config->busp);

    /* Deleting current configuration.*/
    devp->config = NULL;
  }
}

#if (SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI) || defined(__DOXYGEN__)
#if (WSPI_SUPPORTS_MEMMAP == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Enters the memory Mapping mode.
 * @details The memory mapping mode is only available when the WSPI mode
 *          is selected and the underlying WSPI controller supports the
 *          feature.
 *
 * @param[in] devp      pointer to the @p SNORDriver object
 * @param[out] addrp    pointer to the memory start address of the mapped
 *                      flash or @p NULL
 *
 * @api
 */
void snorMemoryMap(SNORDriver *devp, uint8_t **addrp) {

  /* Bus acquisition.*/
  bus_acquire(devp->config->busp, devp->config->buscfg);

#if SNOR_DEVICE_SUPPORTS_XIP == TRUE
  /* Activating XIP mode in the device.*/
  snor_activate_xip(devp);
#endif

  /* Starting WSPI memory mapped mode.*/
  wspiMapFlash(devp->config->busp, &snor_memmap_read, addrp);

  /* Bus release.*/
  bus_release(devp->config->busp);
}

/**
 * @brief   Leaves the memory Mapping mode.
 *
 * @param[in] devp      pointer to the @p SNORDriver object
 *
 * @api
 */
void snorMemoryUnmap(SNORDriver *devp) {

  /* Bus acquisition.*/
  bus_acquire(devp->config->busp, devp->config->buscfg);

  /* Stopping WSPI memory mapped mode.*/
  wspiUnmapFlash(devp->config->busp);

#if SNOR_DEVICE_SUPPORTS_XIP == TRUE
  snor_reset_xip(devp);
#endif

  /* Bus release.*/
  bus_release(devp->config->busp);
}
#endif /* WSPI_SUPPORTS_MEMMAP == TRUE */
#endif /* SNOR_BUS_DRIVER == SNOR_BUS_DRIVER_WSPI */

/** @} */

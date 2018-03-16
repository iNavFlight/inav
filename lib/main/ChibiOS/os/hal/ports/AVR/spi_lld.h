/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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
 * @file    AVR/spi_lld.h
 * @brief   AVR SPI subsystem low level driver header.
 *
 * @addtogroup SPI
 * @{
 */

#ifndef _SPI_LLD_H_
#define _SPI_LLD_H_

#if HAL_USE_SPI || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/** @brief SPI Mode (Polarity/Phase) */
#define SPI_CPOL0_CPHA0    0
#define SPI_CPOL0_CPHA1    1
#define SPI_CPOL1_CPHA0    2
#define SPI_CPOL1_CPHA1    3

#define SPI_MODE_0         SPI_CPOL0_CPHA0
#define SPI_MODE_1         SPI_CPOL0_CPHA1
#define SPI_MODE_2         SPI_CPOL1_CPHA0
#define SPI_MODE_3         SPI_CPOL1_CPHA1

/** @brief Bit order */
#define SPI_LSB_FIRST      0
#define SPI_MSB_FIRST      1

/** @brief SPI clock rate FOSC/x */
#define SPI_SCK_FOSC_2     0
#define SPI_SCK_FOSC_4     1
#define SPI_SCK_FOSC_8     2
#define SPI_SCK_FOSC_16    3
#define SPI_SCK_FOSC_32    4
#define SPI_SCK_FOSC_64    5
#define SPI_SCK_FOSC_128   6

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   SPI driver enable switch.
 * @details If set to @p TRUE the support for SPI1 is included.
 */
#if !defined(AVR_SPI_USE_SPI1) || defined(__DOXYGEN__)
#define AVR_SPI_USE_SPI1               FALSE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a structure representing an SPI driver.
 */
typedef struct SPIDriver SPIDriver;

/**
 * @brief   SPI notification callback type.
 *
 * @param[in] spip      pointer to the @p SPIDriver object triggering the
 *                      callback
 */
typedef void (*spicallback_t)(SPIDriver *spip);

/**
 * @brief   Driver configuration structure.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct {
  /**
   * @brief Port used of Slave Select
   */
  ioportid_t            ssport;
  /**
   * @brief Pad used of Slave Select
   */
  uint8_t               sspad;
  /**
   * @brief Polarity/Phase mode
   */
  uint8_t               mode;
  /**
   * @brief Use MSB/LSB first?
   */
  uint8_t               bitorder;
  /**
   * @brief Clock rate of the subsystem
   */
  uint8_t               clockrate;
  /**
   * @brief Operation complete callback.
   */
  spicallback_t         end_cb;
  /* End of the mandatory fields.*/
} SPIConfig;

/**
 * @brief   Structure representing an SPI driver.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
struct SPIDriver {
  /**
   * @brief Driver state.
   */
  spistate_t                state;
  /**
   * @brief Current configuration data.
   */
  SPIConfig                 *config;
#if SPI_USE_WAIT || defined(__DOXYGEN__)
  /**
   * @brief Waiting thread.
   */
  thread_reference_t        thread;
#endif /* SPI_USE_WAIT */
#if SPI_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
  /**
   * @brief Mutex protecting the bus.
   */
  mutex_t                   mutex;
#endif /* SPI_USE_MUTUAL_EXCLUSION */
#if defined(SPI_DRIVER_EXT_FIELDS)
  SPI_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief   Pointer to the buffer with data to send.
   */
  uint8_t                   *txbuf;
  /**
   * @brief   Number of bytes of data to send.
   */
  size_t                    txbytes;
  /**
   * @brief   Current index in buffer when sending data.
   */
  size_t                    txidx;
  /**
   * @brief   Pointer to the buffer to put received data.
   */
  uint8_t                   *rxbuf;
  /**
   * @brief   Number of bytes of data to receive.
   */
  size_t                    rxbytes;
  /**
   * @brief   Current index in buffer when receiving data.
   */
  size_t                    rxidx;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if AVR_SPI_USE_SPI1 && !defined(__DOXYGEN__)
extern SPIDriver SPID1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void spi_lld_init(void);
  void spi_lld_start(SPIDriver *spip);
  void spi_lld_stop(SPIDriver *spip);
  void spi_lld_select(SPIDriver *spip);
  void spi_lld_unselect(SPIDriver *spip);
  void spi_lld_ignore(SPIDriver *spip, size_t n);
  void spi_lld_exchange(SPIDriver *spip, size_t n,
                        const void *txbuf, void *rxbuf);
  void spi_lld_send(SPIDriver *spip, size_t n, const void *txbuf);
  void spi_lld_receive(SPIDriver *spip, size_t n, void *rxbuf);

#if AVR_SPI_USE_16BIT_POLLED_EXCHANGE
  uint16_t spi_lld_polled_exchange(SPIDriver *spip, uint16_t frame);
#else
  uint8_t spi_lld_polled_exchange(SPIDriver *spip, uint8_t frame);
#endif


#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SPI */

#endif /* _SPI_LLD_H_ */

/** @} */

/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV is distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <platform.h>

#include "drivers/bus_spi.h"

#if defined(USE_SPI_DATA_READY) && defined(STM32H7)

#include "drivers/bus_spi_data_ready_impl.h"
#include "drivers/dma.h"
#include "drivers/nvic.h"
#include "drivers/resource.h"

#include "stm32h7xx_ll_dma.h"

// DMA streams for data-ready reads, taken at the end of init from those nobody owns; a bus with
// no free pair reads without DMA. H7 routes any request through its DMAMUX

#define SPI_DMA_FLAGS       (DMA_IT_TCIF | DMA_IT_HTIF | DMA_IT_TEIF | DMA_IT_DMEIF | DMA_IT_FEIF)

// Streams named for serial ports stay theirs: a port may open after init
static const dmaTag_t spiDmaReserved[] = {
#ifdef UART1_RX_DMA
    UART1_RX_DMA,
#endif
#ifdef UART1_TX_DMA
    UART1_TX_DMA,
#endif
#ifdef UART2_RX_DMA
    UART2_RX_DMA,
#endif
#ifdef UART2_TX_DMA
    UART2_TX_DMA,
#endif
#ifdef UART3_RX_DMA
    UART3_RX_DMA,
#endif
#ifdef UART3_TX_DMA
    UART3_TX_DMA,
#endif
#ifdef UART4_RX_DMA
    UART4_RX_DMA,
#endif
#ifdef UART4_TX_DMA
    UART4_TX_DMA,
#endif
#ifdef UART5_RX_DMA
    UART5_RX_DMA,
#endif
#ifdef UART5_TX_DMA
    UART5_TX_DMA,
#endif
#ifdef UART6_RX_DMA
    UART6_RX_DMA,
#endif
#ifdef UART6_TX_DMA
    UART6_TX_DMA,
#endif
#ifdef UART7_RX_DMA
    UART7_RX_DMA,
#endif
#ifdef UART7_TX_DMA
    UART7_TX_DMA,
#endif
#ifdef UART8_RX_DMA
    UART8_RX_DMA,
#endif
#ifdef UART8_TX_DMA
    UART8_TX_DMA,
#endif
    DMA_NONE
};

static bool spiDmaIsFree(DMA_t d)
{
    if (!d || dmaGetOwner(d) != OWNER_FREE) {
        return false;
    }
    for (unsigned i = 0; spiDmaReserved[i] != DMA_NONE; i++) {
        if (dmaGetByTag(spiDmaReserved[i]) == d) {
            return false;
        }
    }
    return true;
}

static const uint32_t spiDmaRequest[SPIDEV_COUNT][2] = {
    { DMA_REQUEST_SPI1_RX, DMA_REQUEST_SPI1_TX },
    { DMA_REQUEST_SPI2_RX, DMA_REQUEST_SPI2_TX },
    { DMA_REQUEST_SPI3_RX, DMA_REQUEST_SPI3_TX },
    { DMA_REQUEST_SPI4_RX, DMA_REQUEST_SPI4_TX },
};

static DMA_t spiDmaFindFree(DMA_t other)
{
    for (int dma = 2; dma >= 1; dma--) {
        for (int stream = 7; stream >= 0; stream--) {
            DMA_t d = dmaGetByTag(DMA_TAG(dma, stream, 0));
            if (d != other && spiDmaIsFree(d)) {
                return d;
            }
        }
    }
    return NULL;
}

bool spiDataReadyDmaInit(SPIDevice device, spiDataReadyDma_t *dma, dmaCallbackHandlerFuncPtr done, uint32_t userParam)
{
    DMA_t rx = spiDmaFindFree(NULL);
    DMA_t tx = spiDmaFindFree(rx);
    if (!rx || !tx) {
        return false;
    }
    LL_DMA_SetPeriphRequest(rx->dma, DMATAG_GET_STREAM(rx->tag), spiDmaRequest[device][0]);
    LL_DMA_SetPeriphRequest(tx->dma, DMATAG_GET_STREAM(tx->tag), spiDmaRequest[device][1]);

    dmaInit(rx, OWNER_SPI, device + 1);
    dmaInit(tx, OWNER_SPI, device + 1);
    dma->rx = rx;
    dma->tx = tx;

    // The receive stream ends the read; same priority as the data-ready interrupt, so neither
    // preempts the other
    dmaSetHandler(rx, done, NVIC_PRIO_GYRO_DATA_READY, userParam);
    return true;
}

static void spiDmaStreamDisable(DMA_t d)
{
    d->ref->CR &= ~(DMA_SxCR_EN | DMA_SxCR_TCIE | DMA_SxCR_TEIE);
    // A stream that was moving data stops after the current beat
    for (int timeout = 1000; timeout && (d->ref->CR & DMA_SxCR_EN); timeout--);
    DMA_CLEAR_FLAG(d, SPI_DMA_FLAGS);
}

static void spiDmaStreamStart(DMA_t d, uint32_t cr, volatile void *periph, const void *mem, uint16_t n)
{
    spiDmaStreamDisable(d);
    d->ref->PAR = (uint32_t)periph;
    d->ref->M0AR = (uint32_t)mem;
    d->ref->NDTR = n;
    d->ref->FCR = 0;                    // direct mode, byte by byte
    d->ref->CR = cr;
    d->ref->CR = cr | DMA_SxCR_EN;
}

void spiDataReadyDmaStart(const spiDataReadyDma_t *dma, volatile void *txReg, volatile void *rxReg, const uint8_t *tx, uint8_t *rx, uint16_t n)
{
    spiDmaStreamStart(dma->rx, DMA_SxCR_MINC | DMA_SxCR_PL_1 | DMA_SxCR_TCIE | DMA_SxCR_TEIE, rxReg, rx, n);
    spiDmaStreamStart(dma->tx, DMA_SxCR_DIR_0 | DMA_SxCR_MINC | DMA_SxCR_PL_0, txReg, tx, n);
}

void spiDataReadyDmaStop(const spiDataReadyDma_t *dma)
{
    spiDmaStreamDisable(dma->rx);
    spiDmaStreamDisable(dma->tx);
}

bool spiDataReadyDmaEnded(DMA_t rx)
{
    const bool ended = DMA_GET_FLAG_STATUS(rx, DMA_IT_TCIF);
    DMA_CLEAR_FLAG(rx, SPI_DMA_FLAGS);
    return ended;
}

void spiDataReadyDmaDisable(const spiDataReadyDma_t *dma)
{
    NVIC_DisableIRQ(dma->rx->irqNumber);
}
#endif

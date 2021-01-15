#ifndef _CH_SDMMC_LIB_H
#define _CH_SDMMC_LIB_H

#include "sama_sdmmc_conf.h"


/**
 * \brief SD/MMC card driver structure.
 * It holds the current command being processed and the SD/MMC card address.
 *
 * The first members of this structure may have to follow the DMA alignment
 * requirements.
 */
typedef struct _SdCard {


	uint8_t *EXT; /**< MMC Extended Device-Specific Data Register
					 * (EXT_CSD). This member may have to follow
					 * the DMA alignment requirements. */
	uint8_t *SSR; /**< SD Status Register (SSR).
					 * This member may have to follow the DMA
					 * alignment requirements. */
	uint8_t *SCR; /**< SD CARD Configuration Register (SCR).
					 * This member may have to follow the DMA
					 * alignment requirements. */
	uint8_t *sandbox1; /**< Multi-purpose temporary buffer.
						 * This member may have to follow the DMA
						 * alignment requirements. */
	uint8_t *sandbox2; /**< Multi-purpose temporary buffer.
						 * This member may have to follow the DMA
						 * alignment requirements. */

	uint32_t CID[128 / 8 / 4]; /**< Card Identification (CID register) */
	uint32_t CSD[128 / 8 / 4]; /**< Card-specific data (CSD register) */

	void *pExt;		/**< Pointer to extension data for SD/MMC/SDIO */

	uint32_t dwTotalSize;	/**< Card total size
                                (0xffffffff to see number of blocks */
	uint32_t dwNbBlocks;	/**< Card total number of blocks */
	uint16_t wBlockSize;	/**< Card block size reported */

	uint16_t wCurrBlockLen;	/**< Block length used */
	uint32_t dwCurrSpeed;	/**< Device clock frequency used, in Hz */
	uint16_t wAddress;	/**< Current card address */
	uint8_t bCardType;	/**< SD/MMC/SDIO card type \sa sdmmc_cardtype */
	uint8_t bCardSigLevel;	/**< 0/1/2 for low/ready_for_low/high signaling
				 	 	 	 * level used by the card, respectively. */
	uint8_t bSpeedMode;	/**< Timing mode */
	uint8_t bBusMode;	/**< 1/4/8 bit data bus mode */

	uint8_t bStatus;	/**< Unrecovered error */
	uint8_t bSetBlkCnt;	/**< Explicit SET_BLOCK_COUNT command used */
	uint8_t bStopMultXfer;	/**< Explicit STOP_TRANSMISSION command used */
} sSdCard;


/**
 * Sdmmc command operation settings union.
 */
typedef union _SdmmcCmdOperation {
	uint16_t wVal;
	struct _SdmmcOpBm {
		uint16_t powerON:1, /**< Do power on initialize */
		 sendCmd:1,	    /**< Send SD/MMC command */
		 xfrData:2,	    /**< Send/Stop data transfer */
		 respType:3,	    /**< Response type (1~7) */
		 crcON:1,	    /**< CRC is used (SPI) */
		 odON:1,	    /**< Open-Drain is ON (MMC) */
		 ioCmd:1,	    /**< SDIO command */
		 checkBsy:1;	    /**< Busy check is ON */
	} bmBits;
} uSdmmcCmdOp;

/**
 * Sdmmc command instance.
 */
typedef struct _SdmmcCommand {

	/** Optional user-provided callback function. */
	//fSdmmcCallback fCallback;
	/** Optional argument to the callback function. */
	void *pArg;

	/** Data buffer. It shall follow the peripheral and DMA alignment
	 * requirements, which are peripheral and driver dependent. */
	uint8_t *pData;
	/** Size of data block in bytes. */
	uint16_t wBlockSize;
	/** Number of blocks to be transfered */
	uint16_t wNbBlocks;
	/** Response buffer. */
	uint32_t *pResp;

	/** Command argument. */
	uint32_t dwArg;
	/** Command operation settings */
	uSdmmcCmdOp cmdOp;
	/** Command index */
	uint8_t bCmd;
	/** Command return status */
	volatile uint8_t bStatus;
} sSdmmcCommand;


/** SD/MMC Return codes */
typedef enum {
	SDMMC_OK = 0,		/**< Operation OK */
	SDMMC_LOCKED = 1,	/**< Failed because driver locked */
	SDMMC_BUSY = 2,		/**< Failed because driver busy */
	SDMMC_NO_RESPONSE = 3,	/**< Failed because card not respond */
	SDMMC_CHANGED,		/**< Setting param changed due to limitation */
	SDMMC_ERR,		/**< Failed with general error */
	SDMMC_ERR_IO,		/**< Failed because of IO error */
	SDMMC_ERR_RESP,		/**< Error reported in response code */
	SDMMC_NOT_INITIALIZED,	/**< Fail to initialize */
	SDMMC_PARAM,		/**< Parameter error */
	SDMMC_STATE,		/**< State error */
	SDMMC_USER_CANCEL,	/**< Canceled by user */
	SDMMC_NOT_SUPPORTED	/**< Command(Operation) not supported */
} eSDMMC_RC;

/**
 *  \addtogroup sdmmc_cardtype SD/MMC Card Types
 *  Here lists the SD/MMC card types.
 *  - Card Type Category Bitmap
 *    - \ref CARD_TYPE_bmHC
 *    - \ref CARD_TYPE_bmSDMMC
 *      - \ref CARD_TYPE_bmUNKNOWN
 *      - \ref CARD_TYPE_bmSD
 *      - \ref CARD_TYPE_bmMMC
 *    - \ref CARD_TYPE_bmSDIO
 *  - Card Types
 *    - \ref CARD_UNKNOWN
 *    - \ref CARD_SD
 *    - \ref CARD_SDHC
 *    - \ref CARD_MMC
 *    - \ref CARD_MMCHD
 *    - \ref CARD_SDIO
 *    - \ref CARD_SDCOMBO
 *    - \ref CARD_SDHCCOMBO
 *      @{*/
#define CARD_TYPE_bmHC           (1 << 0)   /**< Bit for High-Capacity(Density) */
#define CARD_TYPE_bmSDMMC        (0x3 << 1) /**< Bits mask for SD/MMC */
#define CARD_TYPE_bmUNKNOWN      (0x0 << 1) /**< Bits for Unknown card */
#define CARD_TYPE_bmSD           (0x1 << 1) /**< Bits for SD */
#define CARD_TYPE_bmMMC          (0x2 << 1) /**< Bits for MMC */
#define CARD_TYPE_bmSDIO         (1 << 3)   /**< Bit for SDIO */
/** Card can not be identified */
#define CARD_UNKNOWN    (0)
/** SD Card (0x2) */
#define CARD_SD         (CARD_TYPE_bmSD)
/** SD High Capacity Card (0x3) */
#define CARD_SDHC       (CARD_TYPE_bmSD|CARD_TYPE_bmHC)
/** MMC Card (0x4) */
#define CARD_MMC        (CARD_TYPE_bmMMC)
/** MMC High-Density Card (0x5) */
#define CARD_MMCHD      (CARD_TYPE_bmMMC|CARD_TYPE_bmHC)
/** SDIO only card (0x8) */
#define CARD_SDIO       (CARD_TYPE_bmSDIO)
/** SDIO Combo, with SD embedded (0xA) */
#define CARD_SDCOMBO    (CARD_TYPE_bmSDIO|CARD_SD)
/** SDIO Combo, with SDHC embedded (0xB) */
#define CARD_SDHCCOMBO  (CARD_TYPE_bmSDIO|CARD_SDHC)

#include "ch_sdmmc_macros.h"
#include "ch_sdmmc_trace.h"

extern const uint16_t sdmmcTransUnits[8];
extern const uint8_t sdTransMultipliers[16];
extern const uint8_t mmcTransMultipliers[16];
extern void SdParamReset(sSdCard * pSd);
extern uint32_t SdmmcDecodeTransSpeed(uint32_t code,const uint16_t * unitCodes, const uint8_t * multiCodes);



#endif				/*_CH_SDMMC_LIB_H*/

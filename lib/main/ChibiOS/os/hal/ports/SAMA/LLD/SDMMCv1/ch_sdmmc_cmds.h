
#ifndef CH_SDMMC_CMDS_H_
#define CH_SDMMC_CMDS_H_


/**
 * \struct SdCmd6Arg
 * Argument for SD CMD6
 */
typedef struct _SdCmd6Arg {
	uint32_t acc_mode:4,	/**< [ 3: 0] function group 1, access mode */
	 cmd_sys:4,		/**< [ 7: 4] function group 2, command system */
	 drv_strgth:4,		/**< [11: 8] function group 3, driver strength */
	 pwr_limit:4,		/**< [15:12] function group 4, power limit */
	 func_grp5:4,		/**< [19:16] function group 5, 0xF or 0x0 */
	 func_grp6:4,		/**< [23:20] function group 6, 0xF or 0x0 */
	 reserved:7,		/**< [30:24] reserved 0 */
	 set:1;			/**< [31   ] operation: 0 to check or 1 to set */
} SdCmd6Arg, SdSwitchArg;



/** \addtogroup sdmmc_struct_cmdarg SD/MMC command arguments
 *  Here lists the command arguments for SD/MMC.
 *  - CMD6 Argument
 *    - \ref MmcCmd6Arg "MMC CMD6"
 *    - \ref SdCmd6Arg  "SD CMD6"
 *  - \ref SdioCmd52Arg CMD52
 *  - \ref SdioCmd53Arg CMD53
 *      @{*/
/**
 * \struct MmcCmd6Arg
 * Argument for MMC CMD6
 */
typedef struct _MmcCmd6Arg {
	uint8_t access;
	uint8_t index;
	uint8_t value;
	uint8_t cmdSet;
} MmcCmd6Arg, MmcSwitchArg;
/**
 * \struct SdioCmd52Arg
 * Argument for SDIO CMD52
 */
typedef struct _SdioCmd52Arg {
	uint32_t data:8,	/**< [ 7: 0] data for writing */
	 stuff0:1,		/**< [    8] reserved */
	 regAddress:17,		/**< [25: 9] register address */
	 stuff1:1,		/**< [   26] reserved */
	 rawFlag:1,		/**< [   27] Read after Write flag */
	 functionNum:3,		/**< [30:28] Number of the function */
	 rwFlag:1;		/**< [   31] Direction, 1:write, 0:read. */
} SdioCmd52Arg, SdioRwDirectArg;

#define SDMMC_CMD_bmPOWERON     (0x1      ) /**< Do Power ON sequence */
#define SDMMC_CMD_bmCOMMAND     (0x1 <<  1) /**< Send command */
#define SDMMC_CMD_bmDATAMASK    (0x3 <<  2) /**< Data operation mask */
#define SDMMC_CMD_bmNODATA      (0x0 <<  2) /**< No data transfer */
#define SDMMC_CMD_RX             0x1	    /**< data RX */
#define SDMMC_CMD_bmDATARX      (0x1 <<  2) /**< Bits for data RX */
#define SDMMC_CMD_TX             0x2	    /**< data TX */
#define SDMMC_CMD_bmDATATX      (0x2 <<  2) /**< Bits for data TX */
#define SDMMC_CMD_STOPXFR        0x3	    /**< data stop */
#define SDMMC_CMD_bmSTOPXFR     (0x3 <<  2) /**< Bits for transfer stop */
#define SDMMC_CMD_bmRESPMASK    (0x7 <<  4) /**< Bits masks response option */
#define SDMMC_CMD_bmRESP(R)     (((R)&0x7) << 4)    /**< Bits setup response type: 1 for R1, 2 for R2, ... 7 for R7 */



/** \addtogroup sdmmc_status_bm SD/MMC Status register constants
 *      @{*/
#define STATUS_APP_CMD          (1UL << 5)
#define STATUS_SWITCH_ERROR     (1UL << 7)
#define STATUS_READY_FOR_DATA   (1UL << 8)
#define STATUS_IDLE             (0UL << 9)
#define STATUS_READY            (1UL << 9)
#define STATUS_IDENT            (2UL << 9)
#define STATUS_STBY             (3UL << 9)
#define STATUS_TRAN             (4UL << 9)
#define STATUS_DATA             (5UL << 9)
#define STATUS_RCV              (6UL << 9)
#define STATUS_PRG              (7UL << 9)
#define STATUS_DIS              (8UL << 9)
#define STATUS_BTST             (9UL << 9)
#define STATUS_SLEEP            (10UL << 9)
#define STATUS_STATE            (0xFUL << 9)
#define STATUS_ERASE_RESET       (1UL << 13)
#define STATUS_WP_ERASE_SKIP     (1UL << 15)
#define STATUS_CIDCSD_OVERWRITE  (1UL << 16)
#define STATUS_OVERRUN           (1UL << 17)
#define STATUS_UNERRUN           (1UL << 18)
#define STATUS_ERROR             (1UL << 19)
#define STATUS_CC_ERROR          (1UL << 20)
#define STATUS_CARD_ECC_FAILED   (1UL << 21)
#define STATUS_ILLEGAL_COMMAND   (1UL << 22)
#define STATUS_COM_CRC_ERROR     (1UL << 23)
#define STATUS_UN_LOCK_FAILED    (1UL << 24)
#define STATUS_CARD_IS_LOCKED    (1UL << 25)
#define STATUS_WP_VIOLATION      (1UL << 26)
#define STATUS_ERASE_PARAM       (1UL << 27)
#define STATUS_ERASE_SEQ_ERROR   (1UL << 28)
#define STATUS_BLOCK_LEN_ERROR   (1UL << 29)
#define STATUS_ADDRESS_MISALIGN  (1UL << 30)
#define STATUS_ADDR_OUT_OR_RANGE (1UL << 31)

#define STATUS_STOP ((uint32_t)( STATUS_CARD_IS_LOCKED \
                        | STATUS_COM_CRC_ERROR \
                        | STATUS_ILLEGAL_COMMAND \
                        | STATUS_CC_ERROR \
                        | STATUS_ERROR \
                        | STATUS_STATE \
                        | STATUS_READY_FOR_DATA ))

#define STATUS_WRITE ((uint32_t)( STATUS_ADDR_OUT_OR_RANGE \
                        | STATUS_ADDRESS_MISALIGN \
                        | STATUS_BLOCK_LEN_ERROR \
                        | STATUS_WP_VIOLATION \
                        | STATUS_CARD_IS_LOCKED \
                        | STATUS_COM_CRC_ERROR \
                        | STATUS_ILLEGAL_COMMAND \
                        | STATUS_CC_ERROR \
                        | STATUS_ERROR \
                        | STATUS_ERASE_RESET \
                        | STATUS_STATE \
                        | STATUS_READY_FOR_DATA ))

#define STATUS_READ  ((uint32_t)( STATUS_ADDR_OUT_OR_RANGE \
                        | STATUS_ADDRESS_MISALIGN \
                        | STATUS_BLOCK_LEN_ERROR \
                        | STATUS_CARD_IS_LOCKED \
                        | STATUS_COM_CRC_ERROR \
                        | STATUS_ILLEGAL_COMMAND \
                        | STATUS_CARD_ECC_FAILED \
                        | STATUS_CC_ERROR \
                        | STATUS_ERROR \
                        | STATUS_ERASE_RESET \
                        | STATUS_STATE \
                        | STATUS_READY_FOR_DATA ))

#define STATUS_SD_SWITCH ((uint32_t)( STATUS_ADDR_OUT_OR_RANGE \
                            | STATUS_CARD_IS_LOCKED \
                            | STATUS_COM_CRC_ERROR \
                            | STATUS_ILLEGAL_COMMAND \
                            | STATUS_CARD_ECC_FAILED \
                            | STATUS_CC_ERROR \
                            | STATUS_ERROR \
                            | STATUS_UNERRUN \
                            | STATUS_OVERRUN \
                            /*| STATUS_STATE*/))

#define STATUS_MMC_SWITCH ((uint32_t)( STATUS_CARD_IS_LOCKED \
                            | STATUS_COM_CRC_ERROR \
                            | STATUS_ILLEGAL_COMMAND \
                            | STATUS_CC_ERROR \
                            | STATUS_ERROR \
                            | STATUS_ERASE_RESET \
                            /*| STATUS_STATE*/ \
                            /*| STATUS_READY_FOR_DATA*/ \
                            | STATUS_SWITCH_ERROR ))

#define SD_OCR_S18A             (1ul << 24)	/**< Switching to 1.8V signaling level Accepted */
#define SDIO_OCR_MP             (0x1ul << 27)	/**< SDIO: Memory present */
#define SDIO_OCR_NF             (0x3ul << 28)	/**< SDIO: Number of functions */
#define MMC_OCR_ACCESS_MODE     (0x3ul << 29)	/**< MMC: Access mode, 0x2 is sector mode */
#define MMC_OCR_ACCESS_BYTE     (0x0 << 29)	/**< MMC: Byte access mode */
#define MMC_OCR_ACCESS_SECTOR   (0x2ul << 29)	/**< MMC: Sector access mode */
#define SD_OCR_UHS_II           (1ul << 29)	/**< SD: UHS-II Card Status */
#define SD_OCR_CCS              (1ul << 30)	/**< SD: Card Capacity Status */
#define SD_OCR_BUSYN            (1ul << 31)	/**< SD/MMC: Busy Status */


/** \addtogroup sdmmc_sd_status SD/MMC status fields
 *      @{
 */
/** SSR (SD Status) access macros (512 bits, 16 * 32 bits, 64 * 8 bits). */
#define SD_ST(pSt, field, bits)            SD_GetField(pSt, 512, field, bits)
#define SD_SSR_DAT_BUS_WIDTH(pSt)          (uint8_t)SD_ST(pSt, 510, 2) /**< Bus width, 00: default, 10:4-bit */
#define     SD_SSR_DATA_BUS_WIDTH_1BIT     0x0 /**< 1-bit bus width */
#define     SD_SSR_DATA_BUS_WIDTH_4BIT     0x2 /**< 4-bit bus width */
#define SD_SSR_SECURED_MODE(pSt)           (uint8_t)SD_ST(pSt, 509, 1)  /**< Secured Mode */
#define SD_SSR_CARD_TYPE(pSt)              (uint16_t)SD_ST(pSt, 480, 16)
#define     SD_SSR_CARD_TYPE_RW            0x0000 /**< Regular SD R/W Card */
#define     SD_SSR_CARD_TYPE_ROM           0x0001 /**< SD ROM Card */
#define     SD_SSR_CARD_TYPE_OTP           0x0002 /**< OTP SD Card */
#define SD_SSR_SIZE_OF_PROTECTED_AREA(pSt) SD_ST(pSt, 448, 32) /**< STD: ThisSize*Multi*BlockLen, HC: Size in bytes */
#define SD_SSR_SPEED_CLASS(pSt)            (uint8_t)SD_ST(pSt, 440, 8) /**< Speed Class, value can be calculated by Pw/2 */
#define     SD_SSR_SPEED_CLASS_0           0
#define     SD_SSR_SPEED_CLASS_2           1	// >= 2MB/s
#define     SD_SSR_SPEED_CLASS_4           2	// >= 4MB/s
#define     SD_SSR_SPEED_CLASS_6           3	// >= 6MB/s
#define     SD_SSR_SPEED_CLASS_10          4	// >= 10MB/s
#define SD_SSR_PERFORMANCE_MOVE(pSt)       (uint8_t)SD_ST(pSt, 432, 8) /**< 8-bit, by 1MB/s step. */
#define SD_SSR_AU_SIZE(pSt)                (uint8_t)SD_ST(pSt, 428, 4) /**< AU Size, in power of 2 from 16KB */
#define     SD_SSR_AU_SIZE_16K             1
#define     SD_SSR_AU_SIZE_32K             2
#define     SD_SSR_AU_SIZE_64K             3
#define     SD_SSR_AU_SIZE_128K            4
#define     SD_SSR_AU_SIZE_256K            5
#define     SD_SSR_AU_SIZE_512K            6
#define     SD_SSR_AU_SIZE_1M              7
#define     SD_SSR_AU_SIZE_2M              8
#define     SD_SSR_AU_SIZE_4M              9
#define     SD_SSR_AU_SIZE_8M              0xa
#define     SD_SSR_AU_SIZE_12M             0xb
#define     SD_SSR_AU_SIZE_16M             0xc
#define     SD_SSR_AU_SIZE_24M             0xd
#define     SD_SSR_AU_SIZE_32M             0xe
#define     SD_SSR_AU_SIZE_64M             0xf
#define SD_SSR_ERASE_SIZE(pSt)             (uint16_t)SD_ST(pSt, 408, 16) /**< 16-bit, number of AUs erased. */
#define SD_SSR_ERASE_TIMEOUT(pSt)          (uint8_t)SD_ST(pSt, 402, 6) /**< Timeout value for erasing areas */
#define SD_SSR_ERASE_OFFSET(pSt)           (uint8_t)SD_ST(pSt, 400, 2) /**< Fixed offset value added to erase time */
#define SD_SSR_UHS_SPEED_GRADE(pSt)        (uint8_t)SD_ST(pSt, 396, 4) /**< Speed Grade for UHS mode */
#define     SD_SSR_SPEED_GRADE_0           0x0
#define     SD_SSR_SPEED_GRADE_1           0x1
#define     SD_SSR_SPEED_GRADE_3           0x3
#define SD_SSR_UHS_AU_SIZE(pSt)            (uint8_t)SD_ST(pSt, 392, 4) /**< Size of AU for UHS mode */
#define     SD_SSR_UHS_AU_SIZE_UNDEF       0
#define     SD_SSR_UHS_AU_SIZE_1M          0x7
#define     SD_SSR_UHS_AU_SIZE_2M          0x8
#define     SD_SSR_UHS_AU_SIZE_4M          0x9
#define     SD_SSR_UHS_AU_SIZE_8M          0xa
#define     SD_SSR_UHS_AU_SIZE_12M         0xb
#define     SD_SSR_UHS_AU_SIZE_16M         0xc
#define     SD_SSR_UHS_AU_SIZE_24M         0xd
#define     SD_SSR_UHS_AU_SIZE_32M         0xe
#define     SD_SSR_UHS_AU_SIZE_64M         0xf


extern uint8_t tuneSampling(SdmmcDriver *driver);
extern uint8_t CancelCommand(SdmmcDriver *driver);
extern uint8_t CmdPowerOn(SdmmcDriver *drv);
extern uint8_t SdCmd6(SdmmcDriver *drv,
	       const SdCmd6Arg * pSwitchArg, uint8_t * pStatus, uint32_t * pResp);
extern uint8_t SdCmd8(SdmmcDriver *drv, uint8_t supplyVoltage);
extern uint8_t Acmd6(SdmmcDriver *pSd, uint8_t busWidth);
extern  uint8_t Acmd13(SdmmcDriver *drv, uint8_t * pSSR, uint32_t * pResp);
extern uint8_t Acmd41(SdmmcDriver *drv, bool * low_sig_lvl, bool * hc);
extern uint8_t Acmd51(SdmmcDriver *drv, uint8_t * pSCR, uint32_t * pResp);
extern uint8_t Cmd0(SdmmcDriver *drv, uint8_t arg);
extern uint8_t Cmd1(SdmmcDriver *drv, bool * hc);
extern uint8_t Cmd2(SdmmcDriver *drv);
extern uint8_t Cmd3(SdmmcDriver *drv);
extern uint8_t Cmd5(SdmmcDriver *drv, uint32_t * pIo);
extern uint8_t Cmd7(SdmmcDriver *drv, uint16_t address);
extern uint8_t Cmd9(SdmmcDriver *drv);
extern uint8_t Cmd11(SdmmcDriver *drv, uint32_t * pStatus);
extern uint8_t Cmd12(SdmmcDriver *drv, uint32_t * pStatus);
extern uint8_t Cmd13(SdmmcDriver *drv, uint32_t * pStatus);
extern uint8_t Cmd14(SdmmcDriver *drv, uint8_t * pData, uint8_t len, uint32_t * pStatus);

extern uint8_t Cmd16(SdmmcDriver *drv, uint16_t blkLen);
extern uint8_t Cmd17(SdmmcDriver *drv,
	      uint8_t * pData,
	      uint32_t address, uint32_t * pStatus);
extern uint8_t Cmd18(SdmmcDriver *drv,uint16_t * nbBlock,uint8_t * pData,uint32_t address, uint32_t * pStatus);
extern uint8_t Cmd19(SdmmcDriver *drv, uint8_t * pData, uint8_t len, uint32_t * pStatus);
extern uint8_t Cmd23(SdmmcDriver *drv, uint8_t write, uint32_t blocks, uint32_t * pStatus);
extern uint8_t Cmd24(SdmmcDriver *drv,
	      uint8_t * pData,
	      uint32_t address, uint32_t * pStatus);
extern uint8_t Cmd25(SdmmcDriver *drv,
	      uint16_t * nbBlock,
	      uint8_t * pData,
	      uint32_t address, uint32_t * pStatus);
extern uint8_t Cmd52(SdmmcDriver *drv,uint8_t wrFlag,uint8_t funcNb, uint8_t rdAfterWr, uint32_t addr, uint32_t * pIoData);

extern uint8_t Cmd55(SdmmcDriver *drv, uint16_t cardAddr);

extern uint8_t MmcCmd8(SdmmcDriver *drv);
extern uint8_t MmcCmd6(SdmmcDriver *drv, const void *pSwitchArg, uint32_t * pResp);


#endif /* CH_SDMMC_CMDS_H_ */

#ifndef CH_SDMMC_DEVICE_H_
#define CH_SDMMC_DEVICE_H_


/** \addtogroup sdmmc_ocr_acc SD/MMC OCR register fields (SD 2.0 & MMC 4.3)
 *      @{
 */
#define SD_OCR_VDD_LOW          (1ul <<  7)	/**< SD: Reserved for Low Voltage Range */
#define MMC_OCR_VDD_170_195     (1ul <<  7)	/**< MMC: 1.7 ~ 1.95V, Dual vol and eMMC is 1 */
#define MMC_OCR_VDD_200_270     (0x7Ful << 8)	/**< MMC: 2.0 ~ 2.7 V */
#define SD_OCR_VDD_20_21        (1ul <<  8)
#define SD_OCR_VDD_21_22        (1ul <<  9)
#define SD_OCR_VDD_22_23        (1ul << 10)
#define SD_OCR_VDD_23_24        (1ul << 11)
#define SD_OCR_VDD_24_25        (1ul << 12)
#define SD_OCR_VDD_25_26        (1ul << 13)
#define SD_OCR_VDD_26_27        (1ul << 14)
#define SD_OCR_VDD_27_28        (1ul << 15)
#define SD_OCR_VDD_28_29        (1ul << 16)
#define SD_OCR_VDD_29_30        (1ul << 17)
#define SD_OCR_VDD_30_31        (1ul << 18)
#define SD_OCR_VDD_31_32        (1ul << 19)
#define SD_OCR_VDD_32_33        (1ul << 20)
#define SD_OCR_VDD_33_34        (1ul << 21)
#define SD_OCR_VDD_34_35        (1ul << 22)
#define SD_OCR_VDD_35_36        (1ul << 23)

/**
 *  sdmmc_speedmode SD/MMC Bus speed modes
 *  Here lists the MMC, e.MMC and SD bus speed modes.
 */
#define SDMMC_TIM_MMC_BC         (0x00)
#define SDMMC_TIM_MMC_HS_SDR     (0x01)
#define SDMMC_TIM_MMC_HS_DDR     (0x02)
#define SDMMC_TIM_MMC_HS200      (0x03)
#define SDMMC_TIM_SD_DS          (0x10)
#define SDMMC_TIM_SD_HS          (0x11)
#define SDMMC_TIM_SD_SDR12       (0x12)
#define SDMMC_TIM_SD_SDR25       (0x13)
#define SDMMC_TIM_SD_SDR50       (0x14)
#define SDMMC_TIM_SD_DDR50       (0x15)
#define SDMMC_TIM_SD_SDR104      (0x16)


/**
 *  \addtogroup sdmmc_powermode SD/MMC power supply modes
 *  Here we list the voltage level configurations we may apply when supplying
 *  power to the device.
 *      @{*/
#define SDMMC_PWR_OFF            (0)
#define SDMMC_PWR_STD            (1)
#define SDMMC_PWR_STD_VDD_LOW_IO (2)

/** SD/MMC Low Level IO Control: Check busy.
    Must implement for low level driver busy check.
    IOCtrl(pSd, SDMMC_IOCTL_BUSY_CHECK, (uint32_t)pBusyFlag) */
#define SDMMC_IOCTL_BUSY_CHECK    0x0
/** SD/MMC Low Level IO Control: Power control.
    Recommended for SD/MMC/SDIO power control.
    IOCtrl(pSd, SDMMC_IOCTL_POWER, (uint32_t)ON/OFF) */
#define SDMMC_IOCTL_POWER         0x1
/** SD/MMC Low Level IO Control: Cancel command.
    IOCtrl(pSd, SDMMC_IOCTL_CANCEL_CMD, NULL) */
#define SDMMC_IOCTL_CANCEL_CMD    0x2
/** SD/MMC Low Level IO Control: Reset & disable HW.
    IOCtrl(pSd, SDMMC_IOCTL_RESET, NULL) */
#define SDMMC_IOCTL_RESET         0x3
/** SD/MMC Low Level IO Control: Set clock frequency, return applied frequency
    Recommended for clock selection
    IOCtrl(pSd, SDMMC_IOCTL_SET_CLOCK, (uint32_t*)pIoFreq) */
#define SDMMC_IOCTL_SET_CLOCK     0x11
/** SD/MMC Low Level IO Control: Set bus mode, return applied mode
    Recommended for bus mode selection
    IOCtrl(pSd, SDMMC_IOCTL_SET_BUSMODE, (uint32_t*)pIoBusMode) */
#define SDMMC_IOCTL_SET_BUSMODE   0x12
/** SD/MMC Low Level IO Control: Select one of the SDMMC_TIM_x timing modes.
    Returns the effective mode, further to this operation.
    IOCtrl(pSd, SDMMC_IOCTL_SET_HSMODE, (uint32_t*)pIoTimingMode) */
#define SDMMC_IOCTL_SET_HSMODE    0x13
/** SD/MMC Low Level IO Control: Set Boot mode */
#define SDMMC_IOCTL_SET_BOOTMODE  0x14
/** SD/MMC Low Level IO Control: Enable or disable implicit SET_BLOCK_COUNT
    command, return applied mode.
    Recommended with devices that support the SET_BLOCK_COUNT command.
    IOCtrl(pSd, SDMMC_IOCTL_SET_LENPREFIX, (uint32_t*)pIoLenPrefix) */
#define SDMMC_IOCTL_SET_LENPREFIX 0x15
/** SD/MMC Low Level IO Control: Get clock frequency */
#define SDMMC_IOCTL_GET_CLOCK     0x21
/** SD/MMC Low Level IO Control: Bus mode */
#define SDMMC_IOCTL_GET_BUSMODE   0x22
/** SD/MMC Low Level IO Control: Query whether the driver supports the specified
    SDMMC_TIM_x timing mode. */
#define SDMMC_IOCTL_GET_HSMODE    0x23
/** SD/MMC Low Level IO Control: Boot mode */
#define SDMMC_IOCTL_GET_BOOTMODE  0x24
/** SD/MMC Low Level IO Control: Query driver capability, whether the driver
    implicitly sends the STOP_TRANSMISSION command when multiple-block data
    transfers complete successfully.
    IOCtrl(pSd, SDMMC_IOCTL_GET_XFERCOMPL, (uint32_t*)pOAutoXferCompletion) */
#define SDMMC_IOCTL_GET_XFERCOMPL 0x25
/** SD/MMC Low Level IO Control: Query whether a device is detected in this slot
    IOCtrl(pSd, SDMMC_IOCTL_GET_DEVICE, (uint32_t*)pODetected) */
#define SDMMC_IOCTL_GET_DEVICE    0x26
/** SD/MMC Low Level IO Control: Query whether the card is writeprotected
or not by mechanical write protect switch */
#define SDMMC_IOCTL_GET_WP        0x27
/**     @}*/

#define SD_IFC_CHK_PATTERN_Pos  0		/**< Check pattern */
#define SD_IFC_CHK_PATTERN_Msk  (0xffu << SD_IFC_CHK_PATTERN_Pos)
#define   SD_IFC_CHK_PATTERN_STD  (0xaau << 0)
#define SD_IFC_VHS_Pos          8		/**< Host Supplied Voltage range */
#define SD_IFC_VHS_Msk          (0xfu << SD_IFC_VHS_Pos)
#define   SD_IFC_VHS_27_36        (0x1u << 8)
#define   SD_IFC_VHS_LOW          (0x2u << 8)

/** Get u8 from byte pointed data area */
#define SD_U8(pD, nBytes, iByte)    ( ((uint8_t*)(pD))[(iByte)] )
/** Get u16 from data area */
#define SD_U16(pD, nBytes, iByte)  \
    ( (uint16_t)((uint8_t*)(pD))[(iByte)] |\
      (uint16_t)((uint8_t*)(pD))[(iByte) + 1] << 8 )
/**Get u32 from data area */
#define SD_U32(pD, nBytes, iByte)  \
    ( (uint32_t)((uint8_t*)(pD))[(iByte)] |\
      (uint32_t)((uint8_t*)(pD))[(iByte) + 1] << 8 |\
      (uint32_t)((uint8_t*)(pD))[(iByte) + 2] << 16 |\
      (uint32_t)((uint8_t*)(pD))[(iByte) + 3] << 24 )




/** \addtogroup mmc_ext_csd MMC Extended CSD byte fields
 *      @{
 */
/** MMC Extended CSD access macro: get one byte (512 bytes). */
#define MMC_EXT8(p, i)                  SD_U8(p, 512, i)
/** MMC Extended CSD access macro: get one word (512 bytes). */
#define MMC_EXT32(p, i)                 SD_U32(p, 512, i)
#define MMC_EXT_S_CMD_SET_I             504 /**< Supported Command Sets slice */
#define MMC_EXT_S_CMD_SET(p)            MMC_EXT8(p, MMC_EXT_S_CMD_SET_I)
#define MMC_EXT_PWR_CL_DDR_52_360_I     239 /**< Power Class for 52MHz DDR @ 3.6V */
#define MMC_EXT_PWR_CL_DDR_52_360(p)    MMC_EXT8(p, MMC_EXT_PWR_CL_DDR_52_360_I)
#define MMC_EXT_PWR_CL_200_195_I        237 /**< Power Class for 200MHz HS200 @ VCCQ=1.95V VCC=3.6V */
#define MMC_EXT_PWR_CL_200_195(p)       MMC_EXT8(p, MMC_EXT_PWR_CL_200_195_I)
#define MMC_EXT_BOOT_INFO_I             228 /**< Boot information  slice */
#define MMC_EXT_BOOT_INFO(p)            MMC_EXT8(p, MMC_EXT_BOOT_INFO_I)
#define MMC_EXT_BOOT_SIZE_MULTI_I       226 /**< Boot partition size  slice */
#define MMC_EXT_BOOT_SIZE_MULTI(p)      MMC_EXT8(p, MMC_EXT_BOOT_SIZE_MULTI_I)
#define MMC_EXT_ACC_SIZE_I              225 /**< Access size slice */
#define MMC_EXT_ACC_SIZE(p)             MMC_EXT8(p, MMC_EXT_ACC_SIZE_I)
#define MMC_EXT_HC_ERASE_GRP_SIZE_I     224 /**< High-capacity erase time unit size slice */
#define MMC_EXT_HC_ERASE_GRP_SIZE(p)    MMC_EXT8(p, MMC_EXT_HC_ERASE_GRP_SIZE_I)
#define MMC_EXT_ERASE_TIMEOUT_MULT_I    223 /**< High-capacity erase timeout slice */
#define MMC_EXT_ERASE_TIMEOUT_MULT(p)   MMC_EXT8(p, MMC_EXT_ERASE_TIMEOUT_MULT_I)
#define MMC_EXT_REL_WR_SEC_C_I          222 /**< Reliable write sector count slice */
#define MMC_EXT_REL_WR_SEC_C(p)         MMC_EXT8(p, MMC_EXT_REL_WR_SEC_C_I)
#define MMC_EXT_HC_WP_GRP_SIZE_I        221 /**< High-capacity write protect group size slice */
#define MMC_EXT_HC_WP_GRP_SIZE(p)       MMC_EXT8(p, MMC_EXT_HC_WP_GRP_SIZE_I)
#define MMC_EXT_S_C_VCC_I               220 /**< Sleep current (VCC) */
#define MMC_EXT_S_C_VCC(p)              MMC_EXT8(p, MMC_EXT_S_C_VCC_I)
#define MMC_EXT_S_C_VCCQ_I              219 /**< Sleep current (VCC) */
#define MMC_EXT_S_C_VCCQ(p)             MMC_EXT8(p, MMC_EXT_S_C_VCCQ_I)
#define MMC_EXT_S_A_TIMEOUT_I           217 /**< Sleep current (VCCQ) */
#define MMC_EXT_S_A_TIMEOUT(p)          MMC_EXT8(p, MMC_EXT_S_A_TIMEOUT_I)
#define MMC_EXT_SEC_COUNT_I             212 /**< Sector Count slice */
#define MMC_EXT_SEC_COUNT(p)            MMC_EXT32(p, MMC_EXT_SEC_COUNT_I)
#define MMC_EXT_MIN_PERF_W_8_52_I       210 /**< Minimum Write Performance for 8bit @ 52MHz */
#define MMC_EXT_MIN_PERF_W_8_52(p)      MMC_EXT8(p, MMC_EXT_MIN_PERF_W_8_52_I)
#define MMC_EXT_MIN_PERF_R_8_52_I       209 /**< Minimum Read Performance for 8bit @ 52MHz */
#define MMC_EXT_MIN_PERF_R_8_52(p)      MMC_EXT8(p, MMC_EXT_MIN_PERF_R_8_52_I)
#define MMC_EXT_MIN_PERF_W_8_26_4_52_I  208 /**< Minimum Write Performance for 8bit @ 26MHz or 4bit @ 52MHz */
#define MMC_EXT_MIN_PERF_W_8_26_4_52(p) MMC_EXT8(p, MMC_EXT_MIN_PERF_W_8_26_4_52_I)
#define MMC_EXT_MIN_PERF_R_8_26_4_52_I  207 /**< Minimum Read Performance for 8bit @ 26MHz or 4bit @ 52MHz */
#define MMC_EXT_MIN_PERF_R_8_26_4_52(p) MMC_EXT8(p, MMC_EXT_MIN_PERF_R_8_26_4_52_I)
#define MMC_EXT_MIN_PERF_W_4_26_I       206 /**< Minimum Write Performance for 4bit @ 26MHz */
#define MMC_EXT_MIN_PERF_W_4_26(p)      MMC_EXT8(p, MMC_EXT_MIN_PERF_W_4_26_I)
#define MMC_EXT_MIN_PERF_R_4_26_I       205 /**< Minimum Read Performance for 4bit @ 26MHz */
#define MMC_EXT_MIN_PERF_R_4_26(p)      MMC_EXT8(p, MMC_EXT_MIN_PERF_R_4_26_I)
#define MMC_EXT_PWR_CL_26_360_I         203 /**< Power Class for 26MHz @ 3.6V */
#define MMC_EXT_PWR_CL_26_360(p)        MMC_EXT8(p, MMC_EXT_PWR_CL_26_360_I)
#define MMC_EXT_PWR_CL_52_360_I         202 /**< Power Class for 52MHz @ 3.6V */
#define MMC_EXT_PWR_CL_52_360(p)        MMC_EXT8(p, MMC_EXT_PWR_CL_52_360_I)
#define MMC_EXT_PWR_CL_26_195_I         201 /**< Power Class for 26MHz @ 1.95V */
#define MMC_EXT_PWR_CL_26_195(p)        MMC_EXT8(p, MMC_EXT_PWR_CL_26_195_I)
#define MMC_EXT_PWR_CL_52_195_I         200 /**< Power Class for 52MHz @ 1.95V */
#define MMC_EXT_PWR_CL_52_195(p)        MMC_EXT8(p, MMC_EXT_PWR_CL_52_195_I)
#define MMC_EXT_DRV_STRENGTH_I          197 /**< Supported I/O driver strength types */
#define MMC_EXT_DRV_STRENGTH(p)         MMC_EXT8(p, MMC_EXT_DRV_STRENGTH_I)
#define MMC_EXT_CARD_TYPE_I             196 /**< Card Type */
#define MMC_EXT_CARD_TYPE(p)            MMC_EXT8(p, MMC_EXT_CARD_TYPE_I)
#define MMC_EXT_CSD_STRUCTURE_I         194 /**< CSD Structure Version */
#define MMC_EXT_CSD_STRUCTURE(p)        MMC_EXT8(p, MMC_EXT_CSD_STRUCTURE_I)
#define MMC_EXT_EXT_CSD_REV_I           192 /**< Extended CSD Revision */
#define MMC_EXT_EXT_CSD_REV(p)          MMC_EXT8(p, MMC_EXT_EXT_CSD_REV_I)
#define MMC_EXT_CMD_SET_I               191 /**< Command Set */
#define MMC_EXT_CMD_SET(p)              MMC_EXT8(p, MMC_EXT_CMD_SET_I)
#define MMC_EXT_CMD_SET_REV_I           189 /**< Command Set Revision */
#define MMC_EXT_CMD_SET_REV(p)          MMC_EXT8(p, MMC_EXT_CMD_SET_REV_I)
#define MMC_EXT_POWER_CLASS_I           187 /**< Power Class */
#define MMC_EXT_POWER_CLASS(p)          MMC_EXT8(p, MMC_EXT_POWER_CLASS_I)
#define MMC_EXT_HS_TIMING_I             185 /**< High Speed Interface Timing */
#define MMC_EXT_HS_TIMING(p)            MMC_EXT8(p, MMC_EXT_HS_TIMING_I)
#define     MMC_EXT_HS_TIMING_HS400     0x3
#define     MMC_EXT_HS_TIMING_HS200     0x2
#define     MMC_EXT_HS_TIMING_EN        0x1
#define     MMC_EXT_HS_TIMING_DIS       0x0
#define     MMC_EXT_HS_TIMING_40R       0x40
#define     MMC_EXT_HS_TIMING_100R      0x30
#define     MMC_EXT_HS_TIMING_66R       0x20
#define     MMC_EXT_HS_TIMING_33R       0x10
#define     MMC_EXT_HS_TIMING_50R       0x00
#define MMC_EXT_BUS_WIDTH_I             183 /**< Bus Width Mode */
#define MMC_EXT_BUS_WIDTH(p)            MMC_EXT8(p, MMC_EXT_BUS_WIDTH_I)
#define     MMC_EXT_BUS_WIDTH_1BIT      0
#define     MMC_EXT_BUS_WIDTH_4BITS     1
#define     MMC_EXT_BUS_WIDTH_8BITS     2
#define     MMC_EXT_BUS_WIDTH_DDR       0x4
#define MMC_EXT_ERASED_MEM_CONT_I       181 /**< Erased Memory Content */
#define MMC_EXT_ERASED_MEM_CONT(p)      MMC_EXT8(p, MMC_EXT_ERASED_MEM_CONT_I)
#define MMC_EXT_BOOT_CONFIG_I           179 /**< Boot configuration slice */
#define MMC_EXT_BOOT_CONFIG(p)          MMC_EXT8(p, MMC_EXT_BOOT_CONFIG_I)
#define MMC_EXT_BOOT_BUS_WIDTH_I        177 /**< Boot bus width slice */
#define MMC_EXT_BOOT_BUS_WIDTH(p)       MMC_EXT8(p, MMC_EXT_BOOT_BUS_WIDTH_I)
#define MMC_EXT_ERASE_GROUP_DEF_I       175 /**< High-density erase group definition */
#define MMC_EXT_ERASE_GROUP_DEF(p)      MMC_EXT8(p, MMC_EXT_ERASE_GROUP_DEF_I)
#define MMC_EXT_BOOT_WP_STATUS_I        174 /**< Current protection status of the boot partitions */
#define MMC_EXT_BOOT_WP_STATUS(p)       MMC_EXT8(p, MMC_EXT_BOOT_WP_STATUS_I)
#define MMC_EXT_DATA_SECTOR_SIZE_I      61  /**< Current sector size */
#define MMC_EXT_DATA_SECTOR_SIZE(p)     MMC_EXT8(p, MMC_EXT_DATA_SECTOR_SIZE_I)
#define     MMC_EXT_DATA_SECT_512B      0
#define     MMC_EXT_DATA_SECT_4KIB      1



#define SD_BITS32(pDw, nbits, ibit, bits)    \
    ( (((uint32_t*)(pDw))[(nbits)/32-(ibit)/32-1] >> ((ibit)%32)) & ((uint32_t)(1ul << (bits)) - 1 ) )


/** \addtogroup sdmmc_cid_acc SD/MMC CID register fields
 *      @{
 */
/** CID register access (128 bits, 16 * 8 bits, 4 * 32 bits) */
#define SD_CID(pCid, field, bits)        SD_BITS32(pCid, 128, field, bits)
#define SD_CID_MID(pCid)    (uint8_t)SD_CID(pCid, 120, 8)   /**< Manufacture ID */
#define eMMC_CID_CBX(pCid)  (uint8_t)SD_CID(pCid, 112, 2)   /**< eMMC BGA(01)/CARD(00) */
#define SD_CID_OID1(pCid)   (uint8_t)SD_CID(pCid, 112, 8)   /**< OEM/App ID Byte 1 */
#define SD_CID_OID0(pCid)   (uint8_t)SD_CID(pCid, 104, 8)   /**< OEM/App ID Byte 0 */
#define MMC_CID_OID(pCid)   (uint16_t)SD_CID(pCid, 104, 16)  /**< MMC OEM/App ID */
#define eMMC_CID_OID(pCid)  (uint8_t)SD_CID(pCid, 104, 8)   /**< MMC v4.3+ OEM/App ID */
#define SD_CID_PNM4(pCid)   (uint8_t)SD_CID(pCid,  96, 8)   /**< Product name byte 4 */
#define SD_CID_PNM3(pCid)   (uint8_t)SD_CID(pCid,  88, 8)   /**< Product name byte 3 */
#define SD_CID_PNM2(pCid)   (uint8_t)SD_CID(pCid,  80, 8)   /**< Product name byte 2 */
#define SD_CID_PNM1(pCid)   (uint8_t)SD_CID(pCid,  72, 8)   /**< Product name byte 1 */
#define SD_CID_PNM0(pCid)   (uint8_t)SD_CID(pCid,  64, 8)   /**< Product name byte 0 */
#define MMC_CID_PNM5(pCid)  (uint8_t)SD_CID(pCid,  96, 8)   /**< Product name byte 5 */
#define MMC_CID_PNM4(pCid)  (uint8_t)SD_CID(pCid,  88, 8)   /**< Product name byte 4 */
#define MMC_CID_PNM3(pCid)  (uint8_t)SD_CID(pCid,  80, 8)   /**< Product name byte 3 */
#define MMC_CID_PNM2(pCid)  (uint8_t)SD_CID(pCid,  72, 8)   /**< Product name byte 2 */
#define MMC_CID_PNM1(pCid)  (uint8_t)SD_CID(pCid,  64, 8)   /**< Product name byte 1 */
#define MMC_CID_PNM0(pCid)  (uint8_t)SD_CID(pCid,  56, 8)   /**< Product name byte 0 */

#define SD_CID_PRV1(pCid)   (uint8_t)SD_CID(pCid,  60, 4)   /**< Product revision major number */
#define SD_CID_PRV0(pCid)   (uint8_t)SD_CID(pCid,  56, 4)   /**< Product revision minor number */
#define MMC_CID_PRV1(pCid)  (uint8_t)SD_CID(pCid,  52, 4)   /**< Product revision major number */
#define MMC_CID_PRV0(pCid)  (uint8_t)SD_CID(pCid,  48, 4)   /**< Product revision minor number */

#define SD_CID_PSN3(pCid)   (uint8_t)SD_CID(pCid,  48, 8)  /**< Product serial 3 */
#define SD_CID_PSN2(pCid)   (uint8_t)SD_CID(pCid,  40, 8)  /**< Product serial 2 */
#define SD_CID_PSN1(pCid)   (uint8_t)SD_CID(pCid,  32, 8)  /**< Product serial 1 */
#define SD_CID_PSN0(pCid)   (uint8_t)SD_CID(pCid,  24, 8)  /**< Product serial 0 */
#define MMC_CID_PSN3(pCid)  (uint8_t)SD_CID(pCid,  40, 8)  /**< Product serial 3 */
#define MMC_CID_PSN2(pCid)  (uint8_t)SD_CID(pCid,  32, 8)  /**< Product serial 2 */
#define MMC_CID_PSN1(pCid)  (uint8_t)SD_CID(pCid,  24, 8)  /**< Product serial 1 */
#define MMC_CID_PSN0(pCid)  (uint8_t)SD_CID(pCid,  16, 8)  /**< Product serial 0 */

#define SD_CID_MDT_Y(pCid)  (uint8_t)SD_CID(pCid,  12, 8)   /**< Manufacturing Year (0=2000) */
#define SD_CID_MDT_M(pCid)  (uint8_t)SD_CID(pCid,   8, 4)   /**< Manufacturing month */
#define MMC_CID_MDT_Y(pCid) (uint8_t)SD_CID(pCid,   8, 4)   /**< Manufacturing Year (0=1997 or 2013) */
#define MMC_CID_MDT_M(pCid) (uint8_t)SD_CID(pCid,  12, 4)   /**< Manufacturing month */

#define SD_CID_CRC(pCid)    (uint8_t)SD_CID(pCid,   1, 7)   /**< CRC7 checksum */
/**     @}*/

/** CSD register access macros (128 bits, 16 * 8 bits, 4 * 32  bits */
#define SD_CSD(pCsd, field, bits)    SD_BITS32(pCsd, 128, field, bits)
#define SD_CSD_STRUCTURE(pCsd)          (uint8_t)SD_CSD(pCsd, 126, 2) /**< CSD structure */
#define SD_CSD_STRUCTURE_1_0            0 /**< SD v1.01~1.10, v2.0/Std Capacity */
#define SD_CSD_STRUCTURE_2_0            1 /**< SD v2.0/HC */
#define MMC_CSD_STRUCTURE_1_0           0 /**< MMC v1.0~1.2 */
#define MMC_CSD_STRUCTURE_1_1           1 /**< MMC v1.4~2.2 */
#define MMC_CSD_STRUCTURE_1_2           2 /**< MMC v3.1~3.31(v4.0), v4.1~(>v4.1) */
#define MMC_CSD_SPEC_VERS(pCsd)         (uint8_t)SD_CSD(pCsd, 122, 4) /**< System spec version */
#define MMC_CSD_SPEC_VERS_1_0           0 /**< MMC v1.0~1.2 */
#define MMC_CSD_SPEC_VERS_1_4           1 /**< MMC v1.4 */
#define MMC_CSD_SPEC_VERS_2_0           2 /**< MMC v2.0~2.2 */
#define MMC_CSD_SPEC_VERS_3_1           3 /**< MMC v3.1~3.31 */
#define MMC_CSD_SPEC_VERS_4_0           4 /**< MMC v4.0(v4.0), v4.1~(>v4.1) */
#define SD_CSD_TAAC(pCsd)               (uint8_t)SD_CSD(pCsd, 112, 8) /**< Data read-access-time-1 */
#define SD_CSD_NSAC(pCsd)               (uint8_t)SD_CSD(pCsd, 104, 8) /**< Data read access-time-2 in CLK cycles */
#define SD_CSD_TRAN_SPEED(pCsd)         (uint8_t)SD_CSD(pCsd,  96, 8) /**< Max. data transfer rate */
#define SD_CSD_CCC(pCsd)                (uint16_t)SD_CSD(pCsd, 84, 12) /**< Card command class */
#define SD_CSD_READ_BL_LEN(pCsd)        (uint8_t)SD_CSD(pCsd,  80, 4) /**< Max. read data block length */
#define SD_CSD_READ_BL_PARTIAL(pCsd)    (uint8_t)SD_CSD(pCsd,  79, 1) /**< Bartial blocks for read allowed */
#define SD_CSD_WRITE_BLK_MISALIGN(pCsd) (uint8_t)SD_CSD(pCsd,  78, 1) /**< Write block misalignment */
#define SD_CSD_READ_BLK_MISALIGN(pCsd)  (uint8_t)SD_CSD(pCsd,  77, 1) /**< Read block misalignment */
#define SD_CSD_DSR_IMP(pCsd)            (uint8_t)SD_CSD(pCsd,  76, 1) /**< DSP implemented */
#define SD_CSD_C_SIZE(pCsd)             (uint16_t)((SD_CSD(pCsd, 72, 2) << 10) | \
					(SD_CSD(pCsd, 64, 8) << 2)  | \
					 SD_CSD(pCsd, 62, 2)) /**< Device size */
#define SD2_CSD_C_SIZE(pCsd)            ((SD_CSD(pCsd, 64, 6) << 16) | \
					(SD_CSD(pCsd, 56, 8) << 8)  | \
					 SD_CSD(pCsd, 48, 8)) /**< Device size v2.0 */
#define SD_CSD_VDD_R_CURR_MIN(pCsd)     (uint8_t)SD_CSD(pCsd,  59, 3) /**< Max. read current VDD min */
#define SD_CSD_VDD_R_CURR_MAX(pCsd)     (uint8_t)SD_CSD(pCsd,  56, 3) /**< Max. read current VDD max */
#define SD_CSD_VDD_W_CURR_MIN(pCsd)     (uint8_t)SD_CSD(pCsd,  53, 3) /**< Max. write current VDD min */
#define SD_CSD_VDD_W_CURR_MAX(pCsd)     (uint8_t)SD_CSD(pCsd,  50, 3) /**< Max. write current VDD max */
#define SD_CSD_C_SIZE_MULT(pCsd)        (uint8_t)SD_CSD(pCsd,  47, 3) /**< Device size multiplier */
#define SD_CSD_ERASE_BLK_EN(pCsd)       (uint8_t)SD_CSD(pCsd,  46, 1) /**< Erase single block enable */
#define SD_CSD_SECTOR_SIZE(pCsd)        (uint8_t)((SD_CSD(pCsd, 40, 6) << 1) | \
					 SD_CSD(pCsd, 39, 1)) /**< Erase sector size*/
#define SD_CSD_WP_GRP_SIZE(pCsd)        (uint8_t)SD_CSD(pCsd,  32, 7) /**< Write protect group size*/
#define MMC_CSD_ERASE_GRP_SIZE(pCsd)    (uint8_t)SD_CSD(pCsd,  42, 5) /**< Erase group size */
#define MMC_CSD_ERASE_GRP_MULT(pCsd)    (uint8_t)SD_CSD(pCsd,  37, 5) /**< Erase group size multiplier */
#define MMC_CSD_WP_GRP_SIZE(pCsd)       (uint8_t)SD_CSD(pCsd,  32, 5) /**< Write protect group size*/
#define SD_CSD_WP_GRP_ENABLE(pCsd)      (uint8_t)SD_CSD(pCsd,  31, 1) /**< write protect group enable*/
#define MMC_CSD_DEFAULT_ECC(pCsd)       (uint8_t)SD_CSD(pCsd,  29, 2) /**< Manufacturer default ECC */
#define SD_CSD_R2W_FACTOR(pCsd)         (uint8_t)SD_CSD(pCsd,  26, 3) /**< Write speed factor*/
#define SD_CSD_WRITE_BL_LEN(pCsd)       (uint8_t)((SD_CSD(pCsd, 24, 2) << 2) | \
					 SD_CSD(pCsd, 22, 2)) /**< Max write block length*/
#define SD_CSD_WRITE_BL_PARTIAL(pCsd)   (uint8_t)SD_CSD(pCsd,  21, 1) /**< Partial blocks for write allowed*/
#define SD_CSD_CONTENT_PROT_APP(pCsd)   (uint8_t)SD_CSD(pCsd,  16, 1) /**< File format group*/
#define SD_CSD_FILE_FORMAT_GRP(pCsd)    (uint8_t)SD_CSD(pCsd,  15, 1) /**< File format group*/
#define SD_CSD_COPY(pCsd)               (uint8_t)SD_CSD(pCsd,  14, 1) /**< Copy flag (OTP)*/
#define SD_CSD_PERM_WRITE_PROTECT(pCsd) (uint8_t)SD_CSD(pCsd,  13, 1) /**< Permanent write protect*/
#define SD_CSD_TMP_WRITE_PROTECT(pCsd)  (uint8_t)SD_CSD(pCsd,  12, 1) /**< Temporary write protection*/
#define SD_CSD_FILE_FORMAT(pCsd)        (uint8_t)SD_CSD(pCsd,  10, 2) /**< File format*/
#define MMC_CSD_ECC(pCsd)               (uint8_t)SD_CSD(pCsd,   8, 2) /**< ECC */
#define MMC_CSD_ECC_NONE                0 /**< none */
#define MMC_CSD_ECC_BCH                 1 /**< BCH, 3 correctable bits per block */
#define SD_CSD_CRC(pCsd)                (uint8_t)SD_CSD(pCsd,   1, 7) /**< CRC*/

#define SD_CSD_MULT(pCsd)               (uint16_t)(1u << (SD_CSD_C_SIZE_MULT(pCsd) + 2))
#define SD_CSD_BLOCKNR(pCsd)            ((SD_CSD_C_SIZE(pCsd) + 1ul) * SD_CSD_MULT(pCsd))
#define SD_CSD_BLOCKNR_HC(pCsd)         ((SD2_CSD_C_SIZE(pCsd) + 1ul) * 1024ull)
#define SD_CSD_BLOCK_LEN(pCsd)          (uint16_t)(1u << SD_CSD_READ_BL_LEN(pCsd))
#define SD_CSD_TOTAL_SIZE(pCsd)         ((uint64_t)SD_CSD_BLOCKNR(pCsd) * SD_CSD_BLOCK_LEN(pCsd))
#define SD_CSD_TOTAL_SIZE_HC(pCsd)      ((SD2_CSD_C_SIZE(pCsd) + 1ul) * 512ull * 1024ull)
/**     @}*/




#define   t_usleep(d,t) sdmmc_device_sleep(d,t,2)
#define   t_msleep(d,t) sdmmc_device_sleep(d,t,1)
#define   t_xsleep(d,t) sdmmc_device_sleep(d,t,0)

extern uint8_t  sdmmc_device_lowlevelcfg(SdmmcDriver *driver);
extern bool sdmmc_device_initialize(SdmmcDriver *driver);
extern void sdmmc_device_deInit(SdmmcDriver *drv);
extern void sdmmc_device_poll(SdmmcDriver *driver);
extern  uint32_t sdmmc_device_command(SdmmcDriver *driver);
extern uint32_t sdmmc_device_control(SdmmcDriver *driver, uint32_t bCtl);
extern void  sdmmc_device_sleep(SdmmcDriver *driver,uint32_t t,uint32_t m);
extern void sdmmc_device_startTimeCount(SdmmcDriver *driver);
extern void sdmmc_device_checkTimeCount(SdmmcDriver *driver);

extern uint8_t sdmmc_device_start(SdmmcDriver *drv);
extern uint8_t sdmmc_device_identify(SdmmcDriver *drv);

extern  uint8_t sdmmc_get_bus_width(SdmmcDriver *driver);
extern uint8_t HwSetHsMode(SdmmcDriver *drv, uint8_t timingMode);
extern   uint32_t HwSetBusWidth(	SdmmcDriver *drv,uint8_t newWidth);
extern  uint8_t HwSetClock(SdmmcDriver *drv, uint32_t * pIoValClk);
extern uint8_t HwPowerDevice(SdmmcDriver *drv, uint8_t nowSwitchOn);
extern bool HwIsTimingSupported(SdmmcDriver *drv, uint8_t timingMode);
extern uint32_t SdmmcGetMaxFreq(SdmmcDriver *drv);
extern uint8_t SdMmcSelect(SdmmcDriver *drv, uint16_t address, uint8_t statCheck);
extern uint8_t SdMmcIdentify(SdmmcDriver *drv);
extern uint8_t SDMMC_Lib_SdStart(SdmmcDriver *drv, bool * retry);
extern void SdMmcUpdateInformation(SdmmcDriver *drv, bool csd, bool extData);
extern bool sdmmc_is_busy(SdmmcDriver *driver);

#endif /* CH_SDMMC_DEVICE_H_ */

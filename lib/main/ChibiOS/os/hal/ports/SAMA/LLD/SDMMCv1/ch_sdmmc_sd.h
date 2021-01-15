#ifndef CH_SDMMC_SD_H_
#define CH_SDMMC_SD_H_


/** \addtogroup sd_scr_acc SD SCR register fields
 *      @{
 */
/** SCR (Configuration register) access macros (64 bits, 2 * 32 bits, 8 * 8 bits). */
#define SD_SCR(pScr, field, bits)           SD_GetField(pScr, 64, field, bits)
#define SD_SCR_STRUCTURE(pScr)              (uint8_t)SD_SCR(pScr, 60, 4)
#define     SD_SCR_STRUCTURE_1_0            0 /**< SD v1.01~3.01 */
#define SD_SCR_SD_SPEC(pScr)                (uint8_t)SD_SCR(pScr, 56, 4)
#define     SD_SCR_SD_SPEC_1_0              0 /**< SD v1.0~1.01 */
#define     SD_SCR_SD_SPEC_1_10             1 /**< SD v1.10 */
#define     SD_SCR_SD_SPEC_2_00             2 /**< SD v2.00 */
#define SD_SCR_DATA_STAT_AFTER_ERASE(pScr)  (uint8_t)SD_SCR(pScr, 55, 1)
#define SD_SCR_SD_SECURITY(pScr)            (uint8_t)SD_SCR(pScr, 52, 3)
#define     SD_SCR_SD_SECURITY_NO           0 /**< No security */
#define     SD_SCR_SD_SECURITY_NOTUSED      1 /**< Not used */
#define     SD_SCR_SD_SECURITY_1_01         2 /**< Version 1.01 */
#define     SD_SCR_SD_SECURITY_2_00         3 /**< Version 2.00 */
#define SD_SCR_SD_BUS_WIDTHS(pScr)          (uint8_t)SD_SCR(pScr, 48, 4)
#define     SD_SCR_SD_BUS_WIDTH_1BITS       (1 << 0) /**< 1 bit (DAT0) */
#define     SD_SCR_SD_BUS_WIDTH_4BITS       (1 << 2) /**< 4 bit (DAT0~3) */
#define SD_SCR_SD_SPEC3(pScr)               (uint8_t)SD_SCR(pScr, 47, 1)
#define     SD_SCR_SD_SPEC_3_0              1 /**< SD v3.0X */
#define SD_SCR_EX_SECURITY(pScr)            (uint8_t)SD_SCR(pScr, 43, 4)
#define     SD_SCR_EX_SECURITY_NO           0 /**< No extended security */
#define SD_SCR_SD_SPEC4(pScr)               (uint8_t)SD_SCR(pScr, 42, 1)
#define     SD_SCR_SD_SPEC_4_X              1 /**< SD v4.XX */
#define SD_SCR_CMD58_SUPPORT(pScr)          (uint8_t)SD_SCR(pScr, 35, 1)
#define SD_SCR_CMD48_SUPPORT(pScr)          (uint8_t)SD_SCR(pScr, 34, 1)
#define SD_SCR_CMD23_SUPPORT(pScr)          (uint8_t)SD_SCR(pScr, 33, 1)
#define SD_SCR_CMD20_SUPPORT(pScr)          (uint8_t)SD_SCR(pScr, 32, 1)
/** \addtogroup sd_switch_status SD Switch Status fields
 *      @{
 */
/** SD Switch Status access macros (512 bits, 16 * 32 bits, 64 * 8 bits). */
#define SD_SWITCH_ST(p, field, bits)          SD_GetField(p, 512, field, bits)
#define SD_SWITCH_ST_MAX_CURR_CONSUMPTION(p)  (uint16_t)SD_SWITCH_ST(p, 496, 16)
#define SD_SWITCH_ST_FUN_GRP6_INFO(p)         (uint16_t)SD_SWITCH_ST(p, 480, 16)
#define SD_SWITCH_ST_FUN_GRP5_INFO(p)         (uint16_t)SD_SWITCH_ST(p, 464, 16)
#define SD_SWITCH_ST_FUN_GRP4_INFO(p)         (uint16_t)SD_SWITCH_ST(p, 448, 16)
#define     SD_SWITCH_ST_MAX_PWR_0_72W        0x0
#define     SD_SWITCH_ST_MAX_PWR_1_44W        0x1
#define     SD_SWITCH_ST_MAX_PWR_2_16W        0x2
#define     SD_SWITCH_ST_MAX_PWR_2_88W        0x3
#define     SD_SWITCH_ST_MAX_PWR_1_80W        0x4
#define SD_SWITCH_ST_FUN_GRP3_INFO(p)         (uint16_t)SD_SWITCH_ST(p, 432, 16)
#define     SD_SWITCH_ST_OUT_DRV_B            0x0
#define     SD_SWITCH_ST_OUT_DRV_A            0x1
#define     SD_SWITCH_ST_OUT_DRV_C            0x2
#define     SD_SWITCH_ST_OUT_DRV_D            0x3
#define SD_SWITCH_ST_FUN_GRP2_INFO(p)         (uint16_t)SD_SWITCH_ST(p, 416, 16)
#define SD_SWITCH_ST_FUN_GRP1_INFO(p)         (uint16_t)SD_SWITCH_ST(p, 400, 16)
#define     SD_SWITCH_ST_ACC_DS               0x0
#define     SD_SWITCH_ST_ACC_HS               0x1
#define     SD_SWITCH_ST_ACC_SDR50            0x2
#define     SD_SWITCH_ST_ACC_SDR104           0x3
#define     SD_SWITCH_ST_ACC_DDR50            0x4
#define SD_SWITCH_ST_FUN_GRP6_RC(p)           (uint8_t) SD_SWITCH_ST(p, 396, 4)
#define SD_SWITCH_ST_FUN_GRP5_RC(p)           (uint8_t) SD_SWITCH_ST(p, 392, 4)
#define SD_SWITCH_ST_FUN_GRP4_RC(p)           (uint8_t) SD_SWITCH_ST(p, 388, 4)
#define SD_SWITCH_ST_FUN_GRP3_RC(p)           (uint8_t) SD_SWITCH_ST(p, 384, 4)
#define SD_SWITCH_ST_FUN_GRP2_RC(p)           (uint8_t) SD_SWITCH_ST(p, 380, 4)
#define SD_SWITCH_ST_FUN_GRP1_RC(p)           (uint8_t) SD_SWITCH_ST(p, 376, 4)
#define     SD_SWITCH_ST_FUN_GRP_RC_ERROR     0xF
#define SD_SWITCH_ST_DATA_STRUCT_VER(p)       (uint8_t) SD_SWITCH_ST(p, 368, 8)
#define SD_SWITCH_ST_FUN_GRP6_BUSY(p)         (uint16_t)SD_SWITCH_ST(p, 352, 16)
#define SD_SWITCH_ST_FUN_GRP5_BUSY(p)         (uint16_t)SD_SWITCH_ST(p, 336, 16)
#define SD_SWITCH_ST_FUN_GRP4_BUSY(p)         (uint16_t)SD_SWITCH_ST(p, 320, 16)
#define SD_SWITCH_ST_FUN_GRP3_BUSY(p)         (uint16_t)SD_SWITCH_ST(p, 304, 16)
#define SD_SWITCH_ST_FUN_GRP2_BUSY(p)         (uint16_t)SD_SWITCH_ST(p, 288, 16)
#define SD_SWITCH_ST_FUN_GRP1_BUSY(p)         (uint16_t)SD_SWITCH_ST(p, 272, 16)
#define SD_SWITCH_ST_FUN_GRP_FUN_BUSY(funNdx) (1 << (funNdx))
/**     @}*/

/** We support 2.7 ~ 3.3V cards */
#define SD_HOST_VOLTAGE_RANGE     (SD_OCR_VDD_27_28 +\
                                   SD_OCR_VDD_28_29 +\
                                   SD_OCR_VDD_29_30 +\
                                   SD_OCR_VDD_30_31 +\
                                   SD_OCR_VDD_31_32 +\
                                   SD_OCR_VDD_32_33 +\
                                   SD_OCR_VDD_33_34 +\
                                   SD_OCR_VDD_34_35 +\
                                   SD_OCR_VDD_35_36 )



/**     @}*/

extern uint8_t SD_GetStatus(SdmmcDriver *driver);
extern uint8_t SdDecideBuswidth(SdmmcDriver *drv);
extern uint8_t SdEnableHighSpeed(SdmmcDriver *drv);
extern uint32_t SD_GetField(const uint8_t *reg, uint16_t reg_len, uint16_t field_start,
        uint8_t field_len);
extern void SdGetExtInformation(SdmmcDriver *drv);

extern void SD_DumpStatus(const sSdCard *pSd);
extern void SD_DumpCID(const sSdCard *pSd);
extern void SD_DumpSCR(const uint8_t *pSCR);
extern void SD_DumpCSD(const sSdCard *pSd);
extern void SD_DumpExtCSD(const uint8_t *pExtCSD);
extern void SD_DumpSSR(const uint8_t *pSSR);

extern const char * SD_StringifyRetCode(uint32_t dwRCode);
extern const char * SD_StringifyIOCtrl(uint32_t dwCtrl);

extern uint8_t SD_Read(SdmmcDriver *driver,uint32_t address,void *pData, uint32_t length);
extern uint8_t SD_Write(SdmmcDriver *driver,uint32_t address,const void *pData,uint32_t length);
extern uint8_t SD_ReadBlocks(SdmmcDriver *driver, uint32_t address, void *pData, uint32_t nbBlocks);
extern uint8_t SD_WriteBlocks(SdmmcDriver *driver, uint32_t address, const void *pData, uint32_t nbBlocks);
extern uint8_t SD_ReadBlocks(SdmmcDriver *driver, uint32_t address, void *pData, uint32_t nbBlocks);
extern uint8_t SD_GetWpStatus(SdmmcDriver *driver);

#endif /* CH_SDMMC_SD_H_ */

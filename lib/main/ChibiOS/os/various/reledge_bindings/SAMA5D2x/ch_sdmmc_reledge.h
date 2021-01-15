
#ifndef CH_SDMMC_RELEDGE_H_
#define CH_SDMMC_RELEDGE_H_




extern eSDMMC_RC sd_mmc_test_unit_ready( SdmmcDriver *sdmmcp);
extern bool sd_mmc_is_write_protected(SdmmcDriver *sdmmcp);
extern eSDMMC_RC sd_mmc_read_capacity(SdmmcDriver *sdmmcp, uint32_t *nb_sector);



#endif /* CH_SDMMC_RELEDGE_H_ */

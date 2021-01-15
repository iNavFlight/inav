#include "hal.h"

#if (HAL_USE_SDMMC == TRUE)

#include "sama_sdmmc_lld.h"

#include "ch_sdmmc_device.h"
#include "ch_sdmmc_cmds.h"
#include "ch_sdmmc_sdio.h"
#include "ch_sdmmc_sd.h"
#include "ch_sdmmc_mmc.h"

#include "ch_sdmmc_reledge.h"

eSDMMC_RC sd_mmc_test_unit_ready( SdmmcDriver *sdmmcp)
{

	uint32_t rc;

	if (sdmmc_is_busy(sdmmcp))
		return SDMMC_BUSY;

	rc = SD_GetStatus(sdmmcp);

	if (rc != SDMMC_OK)
	{
		return rc;
	}


	if ( !(sdmmcp->card.bCardType == CARD_UNKNOWN) ) {
		return SDMMC_OK;
	}
	// It is not a memory card
	return SDMMC_ERR;

}

bool sd_mmc_is_write_protected(SdmmcDriver *sdmmcp)
{
	uint32_t rc;

	rc = SD_GetWpStatus(sdmmcp);

	return (rc == SDMMC_LOCKED);
}



eSDMMC_RC sd_mmc_read_capacity(SdmmcDriver *sdmmcp, uint32_t *nb_sector)
{
	// Return last sector address (-1)
	*nb_sector = sdmmcp->card.dwNbBlocks  - 1;

	return sd_mmc_test_unit_ready(sdmmcp);
}





#endif



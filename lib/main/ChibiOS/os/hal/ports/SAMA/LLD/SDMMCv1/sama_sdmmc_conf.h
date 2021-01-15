#ifndef SAMA_SDMMC_CONF_H
#define SAMA_SDMMC_CONF_H

#include "ch_sdmmc_sama5d2.h"




#ifndef SAMA_SDMMC_SDMMCDRIVER_IRQ_PRIORITY
#define SAMA_SDMMC_SDMMCDRIVER_IRQ_PRIORITY	4
#endif

#ifndef SAMA_SDMMC_TRACE
#define SAMA_SDMMC_TRACE		0
#endif

#ifndef SAMA_SDMMC_TRACE_LEVEL
#define SAMA_SDMMC_TRACE_LEVEL 	5
#endif

/** Default block size for SD/MMC access */
#ifndef SDMMC_BLOCK_SIZE
#define SDMMC_BLOCK_SIZE        512
#endif


#endif //SAMA_SDMMC_CONF_H


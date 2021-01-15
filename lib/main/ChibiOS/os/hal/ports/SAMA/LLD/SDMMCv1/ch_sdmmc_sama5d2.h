#ifndef CH_SDMMC_SAMA5D2_H_
#define CH_SDMMC_SAMA5D2_H_


#define EXT_SIZE	512
#define SSR_SIZE	64
#define SCR_SIZE	8
#define SB1_SIZE	64
#define SB2_SIZE	8

#define SDMMC_BUFFER_SIZE (EXT_SIZE + SSR_SIZE + SCR_SIZE + SB1_SIZE + SB2_SIZE)

typedef enum
{
	SDMMC_SLOT0 = 0,
	SDMMC_SLOT1
}sdmmcslots_t;

/* mask for board capabilities defines: voltage, slot type and 8-bit support */
#define CAPS0_MASK (SDMMC_CA0R_V33VSUP | SDMMC_CA0R_V30VSUP | \
                    SDMMC_CA0R_V18VSUP | SDMMC_CA0R_SLTYPE_Msk | \
                    SDMMC_CA0R_ED8SUP)
/* SOM1 */
#if defined(BOARD_ATSAM5D27_SOM1)
#define BOARD_SDMMC0_CAPS0 (SDMMC_CA0R_V33VSUP | \
                            SDMMC_CA0R_V18VSUP | \
                            SDMMC_CA0R_SLTYPE_REMOVABLECARD | \
                            SDMMC_CA0R_ED8SUP)

#define BOARD_SDMMC1_CAPS0 (SDMMC_CA0R_V33VSUP | \
                            SDMMC_CA0R_SLTYPE_REMOVABLECARD)
#elif defined(BOARD_ATSAM5D2_XULT)
#define BOARD_SDMMC0_CAPS0 (SDMMC_CA0R_V33VSUP | \
                            SDMMC_CA0R_V18VSUP | \
                            SDMMC_CA0R_SLTYPE_EMBEDDED | \
                            SDMMC_CA0R_ED8SUP)

#define BOARD_SDMMC1_CAPS0 (SDMMC_CA0R_V33VSUP | \
                            SDMMC_CA0R_SLTYPE_REMOVABLECARD)
#else
#define BOARD_SDMMC0_CAPS0 (SDMMC_CA0R_V33VSUP | \
                            SDMMC_CA0R_V18VSUP | \
                            SDMMC_CA0R_SLTYPE_EMBEDDED | \
                            SDMMC_CA0R_ED8SUP)

#define BOARD_SDMMC1_CAPS0 (SDMMC_CA0R_V33VSUP | \
                            SDMMC_CA0R_SLTYPE_REMOVABLECARD)
#endif

#endif /* CH_SDMMC_SAMA5D2_H_ */

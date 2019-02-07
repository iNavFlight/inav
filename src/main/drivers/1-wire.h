
#pragma once

#include "drivers/bus.h"

#ifdef USE_1WIRE

#define OW_TRIPLET_FIRST_BIT(tripletResult) (tripletResult & (1 << 0))
#define OW_TRIPLET_SECOND_BIT(tripletResult) (tripletResult & (1 << 1))
#define OW_TRIPLET_DIRECTION_BIT(tripletResult) (tripletResult & (1 << 2))

#define OW_SINGLE_BIT_WRITE0 0
#define OW_SINGLE_BIT_WRITE1_READ (1<<7)

typedef struct owDev_s {
    busDevice_t *busDev;

    uint8_t status;

    bool (*reset)(struct owDev_s *owDev);
    bool (*owResetCommand)(struct owDev_s *owDev);
    bool (*owReset)(struct owDev_s *owDev);
    bool (*waitForBus)(struct owDev_s *owDev);
    bool (*readConfig)(struct owDev_s *owDev, uint8_t *config);
    bool (*writeConfig)(struct owDev_s *owDev, uint8_t config);
    bool (*readStatus)(struct owDev_s *owDev, uint8_t *status);
    uint8_t (*getStatus)(struct owDev_s *owDev);
    bool (*poll)(struct owDev_s *owDev, bool waitForBus, uint8_t *status);
    bool (*owBusReady)(struct owDev_s *owDev);

    // 1-Wire ROM
    bool (*owSearchRom)(struct owDev_s *owDev, uint8_t familyCode, uint64_t *romTable, uint8_t *romTableLen);
    bool (*owMatchRomCommand)(struct owDev_s *owDev);
    bool (*owMatchRom)(struct owDev_s *owDev, uint64_t rom);
    bool (*owSkipRomCommand)(struct owDev_s *owDev);
    bool (*owSkipRom)(struct owDev_s *owDev);

    // 1-Wire read/write
    bool (*owWriteByteCommand)(struct owDev_s *owDev, uint8_t byte);
    bool (*owWriteByte)(struct owDev_s *owDev, uint8_t byte);
    bool (*owWriteBuf)(struct owDev_s *owDev, const uint8_t *buf, uint8_t len);
    bool (*owReadByteCommand)(struct owDev_s *owDev);
    bool (*owReadByteResult)(struct owDev_s *owDev, uint8_t *result);
    bool (*owReadByte)(struct owDev_s *owDev, uint8_t *result);
    bool (*owReadBuf)(struct owDev_s *owDev, uint8_t *buf, uint8_t len);
    bool (*owSingleBitCommand)(struct owDev_s *owDev, uint8_t type);
    bool (*owSingleBitResult)(struct owDev_s *owDev);
    bool (*owSingleBit)(struct owDev_s *owDev, uint8_t type, bool *result);
    bool (*owTripletCommand)(struct owDev_s *owDev, uint8_t direction);
    uint8_t (*owTripletResult)(struct owDev_s *owDev);
    bool (*owTriplet)(struct owDev_s *owDev, uint8_t direction, uint8_t *result);
} owDev_t;

void owInit(void);
owDev_t *getOwDev(void);

#endif /* defined(USE_1WIRE) && defined(USE_1WIRE_DS2482) */

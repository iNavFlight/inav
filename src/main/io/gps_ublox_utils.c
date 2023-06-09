#include <stdint.h>

#include "gps_ublox_utils.h"

void ublox_update_checksum(uint8_t *data, uint8_t len, uint8_t *ck_a, uint8_t *ck_b)
{
    while (len--) {
        *ck_a += *data;
        *ck_b += *ck_a;
        data++;
    }
}

int ubloxCfgFillBytes(ubx_config_data8_t *cfg, ubx_config_data8_payload_t *kvPairs, uint8_t count)
{
    if (count > MAX_CONFIG_SET_VAL_VALUES)
        count = MAX_CONFIG_SET_VAL_VALUES;

    cfg->header.preamble1 = 0xb5;
    cfg->header.preamble2 = 0x62;
    cfg->header.msg_class = 0x06;
    cfg->header.msg_id = 0x8A;
    cfg->header.length = sizeof(ubx_config_data_header_t) + ((sizeof(ubx_config_data8_payload_t) * count));
    cfg->configHeader.layers = 0x1;
    cfg->configHeader.transation = 0;
    cfg->configHeader.reserved = 0;
    cfg->configHeader.version = 1;
    

    for (int i = 0; i < count; ++i) {
        cfg->data.payload[i].key = kvPairs[i].key; //htonl(kvPairs[i].key);
        cfg->data.payload[i].value = kvPairs[i].value;
    }

    uint8_t *buf = (uint8_t *)cfg;
    uint8_t ck_a, ck_b;
    ublox_update_checksum(buf + 2, cfg->header.length + 4, &ck_a, &ck_b);
    buf[cfg->header.length + 6] = ck_a;
    buf[cfg->header.length + 7] = ck_b;

    return count;
}


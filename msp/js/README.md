# MSP SDK for INAV — JavaScript

ES-module MSP codec generated from `msp/schema/msp_v2.yaml`, for JavaScript consumers such as INAV Configurator. It decodes and encodes MSP payloads; framing and transport stay with the consumer.

## Files

- `generated/messages.js` — `MESSAGES` (every message resolved to its field layout), `MSP_CODES` (code by name), `BY_CODE` (message by code).
- `generated/enums.js` — `InavEnums` (enum members by C typedef name, members carry the C prefix) and `CONDITIONAL_MEMBERS`.
- `generated/constants.js` — `InavConstants`, the constants with a fixed value.
- `generated/inav_version.js` — the INAV version the files were generated from.
- `mspcodec.js` — the runtime: `decodeReply`, `decodeRequest`, `encodeRequest`, `encodeReply`.

`generated/` is written by `./msp/build.sh generate` (`msp/generator/gen_js.py`); do not edit it by hand. `mspcodec.js` is hand-written.

## Use

```js
import { MSP_CODES } from './generated/messages.js';
import { decodeReply, encodeRequest } from './mspcodec.js';

const status = decodeReply(MSP_CODES.MSP_STATUS_EX, dataView);
const payload = encodeRequest(MSP_CODES.MSP_SET_NAME, { craftName: 'frog' });
```

- Payloads are accepted as `Uint8Array`, `DataView`, `ArrayBuffer` or an array of bytes. Encoders return `Uint8Array`.
- Values are keyed by the schema's field names. Integers decode to `Number`, 64-bit integers to `BigInt`, character arrays and strings to `String`, opaque INAV types to `Uint8Array`.
- A message whose shape depends on payload length (`variants` in the table) takes `{ variant }`, keyed by the schema's condition: `encodeRequest(MSP_CODES.MSP2_COMMON_SET_TZ, { tz_offset: 60 }, { variant: 'dataSize == 2' })`.
- Decoding ignores trailing bytes and rejects short payloads, as the firmware does. An omitted optional trailing field ends the encoded payload.

## Testing

`./msp/build.sh test` packs sample values for every request and reply with the Python codec (`tests/make_vectors.py`) and requires this codec to decode the same bytes to the same values and pack them back to the same bytes (`tests/crosscheck.mjs`). It needs `node`.

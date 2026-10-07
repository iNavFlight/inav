// MSP codec driven by the generated message table.
//
// Everything is resolved at generation time in generated/messages.js; this
// module only executes: look the message up, pack or unpack. Nothing here reads
// a schema. It follows msp/python/src/mspapi2/mspcodec.py rule for rule, and
// `msp/build.sh test` checks the two against each other on every message.
//
// Decoding is lenient about trailing bytes, matching the firmware's own
// forward-compatibility rule - MSP payloads only ever gain fields at the end, so
// a newer peer's longer message decodes to the fields this build knows and the
// remainder is ignored. Short payloads are still rejected.
//
// Values: integers decode to Number, 64-bit integers to BigInt, character
// arrays and strings to String, opaque types to Uint8Array.

import { BY_CODE } from './generated/messages.js';

export class MSPCodecError extends Error {}
export class MSPUnpackError extends MSPCodecError {}
export class MSPPackError extends MSPCodecError {}

// struct format code -> size, DataView accessors, integer range
const SCALARS = {
    B: { size: 1, get: 'getUint8', set: 'setUint8', min: 0, max: 0xFF },
    b: { size: 1, get: 'getInt8', set: 'setInt8', min: -0x80, max: 0x7F },
    H: { size: 2, get: 'getUint16', set: 'setUint16', min: 0, max: 0xFFFF },
    h: { size: 2, get: 'getInt16', set: 'setInt16', min: -0x8000, max: 0x7FFF },
    I: { size: 4, get: 'getUint32', set: 'setUint32', min: 0, max: 0xFFFFFFFF },
    i: { size: 4, get: 'getInt32', set: 'setInt32', min: -0x80000000, max: 0x7FFFFFFF },
    Q: { size: 8, get: 'getBigUint64', set: 'setBigUint64', min: 0n, max: 0xFFFFFFFFFFFFFFFFn },
    q: { size: 8, get: 'getBigInt64', set: 'setBigInt64', min: -0x8000000000000000n, max: 0x7FFFFFFFFFFFFFFFn },
    f: { size: 4, get: 'getFloat32', set: 'setFloat32' },
    d: { size: 8, get: 'getFloat64', set: 'setFloat64' },
};

// C types INAV declares (wire_layouts.json) -> struct format code
const LAYOUT_CODES = {
    uint8_t: 'B', int8_t: 'b', uint16_t: 'H', int16_t: 'h',
    uint32_t: 'I', int32_t: 'i', uint64_t: 'Q', int64_t: 'q',
};

const utf8Decoder = new TextDecoder('utf-8');
const utf8Encoder = new TextEncoder();

function messageFor(code) {
    const key = Number(code);
    const entry = BY_CODE.get(key);
    if (entry === undefined) {
        throw new MSPCodecError(`unknown MSP code ${key} (0x${key.toString(16).toUpperCase().padStart(4, '0')})`);
    }
    if (entry.not_implemented) {
        throw new MSPCodecError(`${entry.name} is not implemented by the firmware`);
    }
    return entry;
}

function sideFor(entry, side, variant) {
    if (variant !== undefined) {
        const shape = entry.variants?.[variant];
        if (shape === undefined) {
            throw new MSPCodecError(`${entry.name} has no variant ${JSON.stringify(variant)}`);
        }
        return shape[side];
    }
    if (entry[side] === null && entry.variants) {
        throw new MSPCodecError(`${entry.name} is length-dispatched; select a variant explicitly ` +
            `from ${JSON.stringify(Object.keys(entry.variants))}`);
    }
    return entry[side];
}

function asBytes(payload) {
    if (payload instanceof Uint8Array) {
        return payload;
    }
    if (payload instanceof DataView) {
        return new Uint8Array(payload.buffer, payload.byteOffset, payload.byteLength);
    }
    if (payload instanceof ArrayBuffer) {
        return new Uint8Array(payload);
    }
    if (Array.isArray(payload)) {
        return Uint8Array.from(payload);
    }
    throw new MSPCodecError(`cannot decode a payload of type ${Object.prototype.toString.call(payload)}`);
}

function decodeString(bytes) {
    return utf8Decoder.decode(bytes).replace(/\0+$/, '');
}

function readScalar(view, offset, code) {
    const scalar = SCALARS[code];
    return view[scalar.get](offset, true);
}

/*
 * One value onto the wire. A scalar field accepts any number and truncates it,
 * as Python's int() does; array elements must already be integers. Floats are
 * written as given.
 */
function writeScalar(view, offset, code, value, label, truncate) {
    const scalar = SCALARS[code];
    if (code === 'f' || code === 'd') {
        view[scalar.set](offset, Number(value), true);
        return;
    }
    if (code === 'Q' || code === 'q') {
        let big;
        if (typeof value === 'bigint') {
            big = value;
        } else if (truncate || Number.isInteger(value)) {
            big = BigInt(Math.trunc(Number(value)));
        } else {
            throw new MSPPackError(`${label}: ${value} is not an integer`);
        }
        if (big < scalar.min || big > scalar.max) {
            throw new MSPPackError(`${label}: ${big} out of range for '${code}'`);
        }
        view[scalar.set](offset, big, true);
        return;
    }
    const number = truncate ? Math.trunc(Number(value)) : value;
    if (!Number.isInteger(number)) {
        throw new MSPPackError(`${label}: ${value} is not an integer`);
    }
    if (number < scalar.min || number > scalar.max) {
        throw new MSPPackError(`${label}: ${number} out of range for '${code}'`);
    }
    view[scalar.set](offset, number, true);
}

function packScalars(code, values, label, truncate) {
    const size = SCALARS[code].size;
    const out = new Uint8Array(size * values.length);
    const view = new DataView(out.buffer);
    values.forEach((value, index) => writeScalar(view, index * size, code, value, label, truncate));
    return out;
}

function decodeLayout(layout, bytes) {
    const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
    const out = {};
    if (layout.kind === 'bitfield') {
        let word = 0n;
        for (let index = bytes.length - 1; index >= 0; index--) {
            word = (word << 8n) | BigInt(bytes[index]);
        }
        for (const f of layout.fields) {
            out[f.name] = Number((word >> BigInt(f.bit_offset)) & ((1n << BigInt(f.bits)) - 1n));
        }
        return out;
    }
    for (const f of layout.fields) {
        out[f.name] = readScalar(view, f.offset, LAYOUT_CODES[f.ctype]);
    }
    return out;
}

function encodeLayout(layout, values, label) {
    if (values instanceof Uint8Array) {
        return values;
    }
    const out = new Uint8Array(layout.size);
    if (layout.kind === 'bitfield') {
        let word = 0n;
        for (const f of layout.fields) {
            const mask = (1n << BigInt(f.bits)) - 1n;
            word |= (BigInt(Math.trunc(Number(values[f.name]))) & mask) << BigInt(f.bit_offset);
        }
        for (let index = 0; index < layout.size; index++) {
            out[index] = Number((word >> BigInt(8 * index)) & 0xFFn);
        }
        return out;
    }
    const view = new DataView(out.buffer);
    for (const f of layout.fields) {
        writeScalar(view, f.offset, LAYOUT_CODES[f.ctype], values[f.name], `${label}.${f.name}`, true);
    }
    return out;
}

function needs(name, field, size, end) {
    return new MSPUnpackError(`${name}.${field.name}: needs ${size} bytes for ${field.type}, payload ends at ${end}`);
}

function decodeFields(fields, bytes, view, start, name) {
    const out = {};
    let offset = start;
    for (const field of fields) {
        const kind = field.kind;

        if (kind === 'group') {
            let count = field.count;
            if (count === null) {
                const countField = field.count_field;
                count = countField && countField in out ? Number(out[countField]) : null;
            }
            const records = [];
            while (offset < bytes.length && (count === null || records.length < count)) {
                const [record, next] = decodeFields(field.fields, bytes, view, offset, name);
                records.push(record);
                offset = next;
            }
            if (count !== null && records.length !== count) {
                throw new MSPUnpackError(`${name}.${field.name}: expected ${count} records, got ${records.length}`);
            }
            out[field.name] = records;
            continue;
        }

        if (kind === 'struct') {
            [out[field.name], offset] = decodeFields(field.fields, bytes, view, offset, name);
            continue;
        }

        if (kind === 'discriminated') {
            // A leading NUL selects the index form; anything else is a name.
            const caseName = offset < bytes.length && bytes[offset] === 0 ? 'index' : 'name';
            let value;
            [value, offset] = decodeFields(field.cases[caseName], bytes, view, offset, name);
            out[field.name] = { case: caseName, ...value };
            continue;
        }

        if (kind === 'layout' || kind === 'opaque') {
            const size = field.size;
            if (offset + size > bytes.length) {
                if (field.optional) {
                    return [out, offset];
                }
                throw needs(name, field, size, bytes.length);
            }
            const raw = bytes.slice(offset, offset + size);
            out[field.name] = kind === 'layout' ? decodeLayout(field.layout, raw) : raw;
            offset += size;
            continue;
        }

        if (kind === 'cstring') {
            const end = bytes.indexOf(0, offset);
            if (end < 0) {
                throw new MSPUnpackError(`${name}.${field.name}: unterminated string`);
            }
            out[field.name] = utf8Decoder.decode(bytes.subarray(offset, end));
            offset = end + 1;
            continue;
        }

        if (kind === 'tail') {
            const data = bytes.subarray(offset);
            if (field.char) {
                out[field.name] = decodeString(data);
            } else {
                const items = Math.floor(data.length / field.unit);
                out[field.name] = Array.from({ length: items },
                    (_, index) => readScalar(view, offset + index * field.unit, field.code));
            }
            offset = bytes.length;
            continue;
        }

        const size = kind === 'scalar' ? field.size : field.unit * field.count;
        if (offset + size > bytes.length) {
            if (field.optional) {
                return [out, offset];       // absent trailing field: stop here
            }
            throw new MSPUnpackError(`${name}.${field.name}: payload ends at ${bytes.length}, needs ${offset + size}`);
        }
        if (kind === 'array' && field.char) {
            out[field.name] = decodeString(bytes.subarray(offset, offset + size));
        } else if (kind === 'array') {
            out[field.name] = Array.from({ length: field.count },
                (_, index) => readScalar(view, offset + index * field.unit, field.code));
        } else {
            out[field.name] = readScalar(view, offset, field.code);
        }
        offset += size;
    }
    return [out, offset];
}

function encodeFields(fields, values, name) {
    const parts = [];
    for (const field of fields) {
        const kind = field.kind;
        const value = values[field.name];
        const label = `${name}.${field.name}`;

        if (value === undefined) {
            if (field.optional) {
                break;                      // optional tail: stop, nothing after it
            }
            if (kind === 'group') {
                throw new MSPPackError(`${name}: missing records for '${field.name}'`);
            }
            throw new MSPPackError(`${name}: missing value for field '${field.name}'`);
        }

        if (kind === 'group') {
            for (const record of value) {
                parts.push(encodeFields(field.fields, record, name));
            }
            continue;
        }
        if (kind === 'struct') {
            parts.push(encodeFields(field.fields, value, name));
            continue;
        }
        if (kind === 'discriminated') {
            const caseName = value.case || ('settingIndex' in value ? 'index' : 'name');
            parts.push(encodeFields(field.cases[caseName], value, name));
            continue;
        }
        if (kind === 'layout') {
            parts.push(encodeLayout(field.layout, value, label));
            continue;
        }
        if (kind === 'opaque') {
            const raw = Uint8Array.from(value);
            if (raw.length !== field.size) {
                throw new MSPPackError(`${label}: ${field.type} is ${field.size} bytes, got ${raw.length}`);
            }
            parts.push(raw);
            continue;
        }
        if (kind === 'cstring') {
            parts.push(utf8Encoder.encode(String(value)), Uint8Array.of(0));
            continue;
        }
        if (kind === 'tail') {
            parts.push(field.char ? utf8Encoder.encode(String(value)) : packScalars(field.code, value, label, false));
            continue;
        }
        if (kind === 'array' && field.char) {
            const raw = new Uint8Array(field.count);
            raw.set(utf8Encoder.encode(String(value)).subarray(0, field.count));
            parts.push(raw);
        } else if (kind === 'array') {
            if (value.length !== field.count) {
                throw new MSPPackError(`${label}: expected ${field.count} items, got ${value.length}`);
            }
            parts.push(packScalars(field.code, value, label, false));
        } else {
            parts.push(packScalars(field.code, [value], label, true));
        }
    }
    return concat(parts);
}

function concat(parts) {
    const out = new Uint8Array(parts.reduce((total, part) => total + part.length, 0));
    let offset = 0;
    for (const part of parts) {
        out.set(part, offset);
        offset += part.length;
    }
    return out;
}

/* A payload-level `repeat` means the whole field block repeats, so the result
 * is a list of records rather than one object. */
function decodeSide(side, payload, name) {
    const bytes = asBytes(payload);
    const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
    if (side.repeat === null) {
        return decodeFields(side.fields, bytes, view, 0, name)[0];
    }
    const records = [];
    const limit = typeof side.repeat === 'number' ? side.repeat : null;
    let offset = 0;
    while (offset < bytes.length && (limit === null || records.length < limit)) {
        const [record, next] = decodeFields(side.fields, bytes, view, offset, name);
        if (next === offset) {
            break;
        }
        records.push(record);
        offset = next;
    }
    return records;
}

/* Positional values are matched to the fields in order. */
function valuesFor(side, values) {
    if (side.repeat !== null) {
        return Array.isArray(values) ? values : [values];
    }
    if (Array.isArray(values)) {
        return Object.fromEntries(side.fields.slice(0, values.length).map((field, index) => [field.name, values[index]]));
    }
    return values;
}

function encodeSide(side, values, name) {
    const prepared = valuesFor(side, values);
    if (side.repeat === null) {
        return encodeFields(side.fields, prepared, name);
    }
    return concat(prepared.map((record) => encodeFields(side.fields, record, name)));
}

function pack(code, side, values, variant) {
    const entry = messageFor(code);
    const shape = sideFor(entry, side, variant);
    return shape === null ? new Uint8Array(0) : encodeSide(shape, values, entry.name);
}

function unpack(code, side, payload, variant) {
    const entry = messageFor(code);
    const shape = sideFor(entry, side, variant);
    return shape === null ? {} : decodeSide(shape, payload, entry.name);
}

// `variant` selects one shape of a length-dispatched message (entry.variants).
export function encodeRequest(code, values = {}, { variant } = {}) {
    return pack(code, 'request', values, variant);
}

export function encodeReply(code, values = {}, { variant } = {}) {
    return pack(code, 'reply', values, variant);
}

export function decodeReply(code, payload, { variant } = {}) {
    return unpack(code, 'reply', payload, variant);
}

export function decodeRequest(code, payload, { variant } = {}) {
    return unpack(code, 'request', payload, variant);
}

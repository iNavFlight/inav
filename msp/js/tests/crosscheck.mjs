// Check the JS codec against the Python codec on the vectors make_vectors.py
// wrote: the same bytes must decode to the same values, and those values must
// pack back to the same bytes.
//
// Usage: node crosscheck.mjs VECTORS.json

import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';

import { decodeReply, decodeRequest, encodeReply, encodeRequest } from '../mspcodec.js';

const SAFE_INTEGER = 2n ** 53n;

function fromHex(hex) {
    return Uint8Array.from(hex.match(/../g) ?? [], (pair) => parseInt(pair, 16));
}

function toHex(bytes) {
    return Array.from(bytes, (byte) => byte.toString(16).padStart(2, '0')).join('');
}

// The vectors' JSON form: bytes and integers past 2**53 are tagged.
function fromJson(value) {
    if (Array.isArray(value)) {
        return value.map(fromJson);
    }
    if (value !== null && typeof value === 'object') {
        if ('$bytes' in value) {
            return fromHex(value.$bytes);
        }
        if ('$bigint' in value) {
            return BigInt(value.$bigint);
        }
        return Object.fromEntries(Object.entries(value).map(([key, item]) => [key, fromJson(item)]));
    }
    return value;
}

// The codec's output in the same form, so the two compare directly.
function toJson(value) {
    if (value instanceof Uint8Array) {
        return { $bytes: toHex(value) };
    }
    if (typeof value === 'bigint') {
        return value > SAFE_INTEGER || value < -SAFE_INTEGER ? { $bigint: value.toString() } : Number(value);
    }
    if (Array.isArray(value)) {
        return value.map(toJson);
    }
    if (value !== null && typeof value === 'object') {
        return Object.fromEntries(Object.entries(value).map(([key, item]) => [key, toJson(item)]));
    }
    return value;
}

const vectors = JSON.parse(readFileSync(process.argv[2], 'utf8'));
const failures = [];

for (const vector of vectors) {
    const label = `${vector.message} ${vector.side}${vector.variant ? ` [${vector.variant}]` : ''}` +
        `${vector.truncated ? ' (optional tail omitted)' : ''}`;
    const options = vector.variant === null ? {} : { variant: vector.variant };
    const decode = vector.side === 'request' ? decodeRequest : decodeReply;
    const encode = vector.side === 'request' ? encodeRequest : encodeReply;
    try {
        assert.deepStrictEqual(toJson(decode(vector.code, fromHex(vector.payload), options)), vector.values,
            'decoded values differ');
        assert.equal(toHex(encode(vector.code, fromJson(vector.values), options)), vector.payload,
            'packed bytes differ');
    } catch (error) {
        failures.push(`${label}: ${error.message.split('\n')[0]}`);
    }
}

for (const failure of failures) {
    console.log(`  FAIL ${failure}`);
}
console.log(`  ${vectors.length - failures.length}/${vectors.length} vectors agree with the Python codec`);
process.exit(failures.length ? 1 : 0);

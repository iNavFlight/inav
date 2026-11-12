#!/usr/bin/env python3
"""
Convert ARM ELF (via objcopy .bin) to UF2 format for RP2350.

Usage: elf2uf2.py <input.bin> <output.uf2> [--base-addr 0x10000000] [--family-id 0xe48bff59]

UF2 spec: https://github.com/microsoft/uf2
"""

import struct
import sys
import argparse

UF2_MAGIC_START0 = 0x0A324655  # "UF2\n"
UF2_MAGIC_START1 = 0x9E5D5157
UF2_MAGIC_END    = 0x0AB16F30
UF2_FLAG_FAMILY  = 0x00002000

RP2350_FAMILY_ID = 0xe48bff59
RP2350_FLASH_BASE = 0x10000000
PAYLOAD_SIZE = 256


def bin_to_uf2(data, base_addr, family_id):
    blocks = []
    num_blocks = (len(data) + PAYLOAD_SIZE - 1) // PAYLOAD_SIZE

    for i in range(num_blocks):
        offset = i * PAYLOAD_SIZE
        chunk = data[offset:offset + PAYLOAD_SIZE]
        # Pad last chunk if needed
        chunk = chunk + b'\x00' * (PAYLOAD_SIZE - len(chunk))

        # 32-byte header
        header = struct.pack('<IIIIIIII',
            UF2_MAGIC_START0,
            UF2_MAGIC_START1,
            UF2_FLAG_FAMILY,
            base_addr + offset,
            PAYLOAD_SIZE,
            i,
            num_blocks,
            family_id,
        )
        # 476 bytes: 256 payload + 220 padding
        block = header + chunk + b'\x00' * 220 + struct.pack('<I', UF2_MAGIC_END)
        assert len(block) == 512
        blocks.append(block)

    return b''.join(blocks)


def main():
    parser = argparse.ArgumentParser(description='Convert .bin to UF2 for RP2350')
    parser.add_argument('input', help='Input .bin file')
    parser.add_argument('output', help='Output .uf2 file')
    parser.add_argument('--base-addr', type=lambda x: int(x, 0),
                        default=RP2350_FLASH_BASE,
                        help=f'Flash base address (default: 0x{RP2350_FLASH_BASE:08X})')
    parser.add_argument('--family-id', type=lambda x: int(x, 0),
                        default=RP2350_FAMILY_ID,
                        help=f'UF2 family ID (default: 0x{RP2350_FAMILY_ID:08x})')
    args = parser.parse_args()

    with open(args.input, 'rb') as f:
        data = f.read()

    uf2 = bin_to_uf2(data, args.base_addr, args.family_id)

    with open(args.output, 'wb') as f:
        f.write(uf2)

    print(f'{args.input}: {len(data)} bytes -> {args.output}: {len(uf2)} bytes '
          f'({len(uf2) // 512} UF2 blocks)')


if __name__ == '__main__':
    main()

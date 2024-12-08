# ECDSA signature format

When ECDSA SECP256R1 (EC256) signature support was added to MCUboot, a
shortcut was taken, and these signatures were padded to make them
always a fixed length.  Unfortunately, this padding was done in a way
that is not easily reversible.  Some crypto libraries are fairly
strict about the formatting of the ECDSA signature (specifically, mbed
TLS).  This currently means that the ECDSA SECP224R1 (EC) signature
checking code will fail to boot about 1 out of every 256 images,
because the signature itself will end in a 0x00 byte, and the code
will remove too much data, invalidating the signature.

There are a couple of ways to fix this:

  1.  Use a reversible padding scheme.  This will work, but requires
      at least one pad byte always be added (to set the length).  This
      padding would be somewhat incompatible across versions (older
      EC256 would work, newer mcuboot code would reject old
      signatures.  EC code would only reliably work in the new
      combination).

  2.  Remove the padding entirely.  Depending on which tool, this will
      require some rethinking of how TLV generation is implemented so
      that the length does not need to be known until the signature is
      generated.  These tools are all written in higher-level
      languages and this change should not be difficult.

      However, this will also break compatibility with older versions,
      specifically in that images generated with newer tools will not
      work with older versions of MCUboot.

This document proposes a multi-stage approach, to give a transition
period.

  - First, add a `--no-pad-sig` argument to the sign command in
    `imgtool.py`.  Without this, the images will be padded with the
    existing scheme, and with the argument, the ecdsa will be encoded
    without any padding.  The `--pad-sig` argument will also be
    accepted, but this will initially be the default.

  - MCUboot will be modified to allow unpadded signatures right away.
    The existing EC256 implementations will still work (with or
    without padding), and the existing EC implementation will begin
    accepting padded and unpadded signatures.

  - An mbed TLS implementation of EC256 can be added, but will require
    the `--no-pad-sig` signature to be able to boot all generated
    images (without the argument 3 of out 4 images generated will have
    padding, and be considered invalid).

After one or more MCUboot release cycles, and announcements over
relevant channels, the arguments to `imgtool.py` will change:

  - `--no-pad-sig` will still be accepted, but have no effect.

  - `--pad-sig` will now bring back the old padding behavior.

This will require a change to any scripts that are relying on a
default, but not specifying a specific version of imgtool.

The signature generation in the simulator can be changed at the same
time the boot code begins to accept unpadded signatures.  The sim is
always run out of the same tree as the mcuboot code, so there should
not be any compatibility issues.

## Background

ECDSA signatures are encoded as ASN.1, notably with the signature
itself being encoded as:

    ECDSA-Sig-Value ::= SEQUENCE {
      r  INTEGER,
      s  INTEGER
    }

where both `r` and `s` are 256-bit numbers.  Because these are
unsigned numbers that are being encoded in ASN.1 as signed values, if
the high bit of the number is set, the DER encoded representation will
require 33 bytes instead of 32.  This means that the length of the
signature will vary by a couple of bytes, depending on whether one of
both of these numbers has the high bit set.

Originally, MCUboot added padding to the entire signature, and just
removed any trailing 0 bytes from the data block.  This would be fine 255/256
times, when the last byte of the signature was non-zero, but if the
signature ended in a zero, it would remove too many bytes, and the
signature would be considered invalid.

The correct approach here is to accept that ECDSA signatures are
variable length, and make sure that we can handle them as such.

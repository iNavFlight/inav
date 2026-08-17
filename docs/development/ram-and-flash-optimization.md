# RAM and Flash Optimization

> **Status: seed draft.** This is a stub started while fixing a RAM overflow
> on PR #11718, so the technique wouldn't get lost before the full guide is
> written. It is not yet linked from `Development.md` and is not a complete
> guide — see the tracked project for the rest of this doc's scope.

Static RAM headroom is a binding constraint on several boards (128KB-RAM
F4 targets in particular). Treat RAM and flash reduction as an active goal
when writing or reviewing code that adds static buffers, not just something
to check for overflow after the fact.

## Don't size a buffer for a different subsystem's worst case

If a buffer is reused across features, check what its size actually derives
from before assuming a new consumer needs the same size. On PR #11718 (the
MSP-over-MAVLink tunnel), a reply buffer was sized off `MSP_PORT_OUTBUF_SIZE`
(4112 bytes under `USE_FLASHFS`) simply because that was the constant already
in scope for MSP replies. But that size exists to let `MSP_DATAFLASH_READ`
return a full 4096-byte flash page in one shot for fast blackbox downloads
over USB/serial — the tunnel's own use case (MSP replies fragmented into
128-byte MAVLink `TUNNEL` payload chunks regardless of source buffer size)
never benefited from that size at all. Giving the tunnel its own,
independently-sized constant (512 bytes, matching the size every
non-FLASHFS board already uses for ordinary MSP replies) cut ~3.6KB with no
functional loss — data that used to arrive in one large read now arrives in
more, smaller reads, which costs nothing extra when it's already being
fragmented for the wire.

**Before reusing an existing size constant for a new buffer, ask what
specifically drove that number** — it may be tied to an unrelated worst
case that doesn't apply to the new use.

## When shrinking a shared buffer, audit what else writes into it

A buffer's "extra" size can be silently load-bearing for a bug elsewhere,
not just wasted space. While sizing the tunnel buffer above, auditing every
MSP command reachable through it surfaced `serializeDataflashReadReply()`
computing its available-space clamp *before* writing a 4-byte address
header, then writing `address + clamped_data` — an unconditional 4-byte
overflow whenever a request is large enough to hit the clamp. On the
original 4112-byte buffer this was practically unreachable (16 bytes of
incidental headroom absorbed it for a standard 4096-byte request); shrinking
to 512 bytes with no equivalent headroom made it trivially reachable by any
normal-sized request.

**When shrinking a buffer other code paths also write into, check whether
any of those paths were relying on the buffer being larger than their own
worst-case write** — not just whether the new size covers the new
consumer's own worst case. Fix the root cause (the clamp math, here) rather
than padding the buffer back up to paper over it.

## Survey actual message/data sizes before assuming a buffer needs to be large

"How big does this actually need to be" is often answerable by grep, not
guesswork — but match the audit's scope to everything that can actually
reach the buffer. A first pass on the tunnel buffer above checked only MSP2
command handlers and found 432 bytes as the largest reply; widening to MSP1
(legacy) handlers — reachable through the same buffer — found 512 bytes
exactly (`MSP_LED_STRIP_CONFIG`, no bounds check, no margin). Scoping an
audit to "the obviously relevant" category of code instead of everything
that can reach the buffer produces a confident-looking wrong answer.

## Related

- Full strategy pattern library (more entries, other PRs):
  `claude/projects/active/ram-reduction-program/ram-flash-strategies.md` in
  the `inav-claude` tooling repo (not part of this firmware repo).
- Tracked follow-up: `document-ram-flash-optimization-practices` project —
  expands this into a full guide, links it from `Development.md`, and
  updates review checklists to treat RAM/flash reduction as an active
  review category.

# RAM and Flash Optimization

> **Status: draft.** Started from PR #11718; not yet linked from
> `Development.md` and not a complete guide.

Static RAM headroom is a binding constraint on several boards (128KB-RAM F4
targets in particular). Treat RAM and flash reduction as an active goal when
writing or reviewing code that adds static buffers or new functions, not
just something to check for overflow after the fact.

## Buffer sizing

### Don't reuse another subsystem's size constant

An existing size constant exists for a specific consumer; reusing it for a
different buffer usually over-allocates. Before reusing a constant, check
what specifically drove that number.

Example: the MSP-over-MAVLink tunnel reply buffer was sized off
`MSP_PORT_OUTBUF_SIZE` (4112 B), which exists for `MSP_DATAFLASH_READ`
(returning a full flash page in one reply). The tunnel fragments replies
into 128-byte chunks regardless of source buffer size, so it never
benefited; its own 512-byte constant cut ~3.6 KB with no functional loss.

### Audit every writer before shrinking a shared buffer

Shrinking a shared buffer can expose code that relied on it being larger
than its own worst case. Check every code path into the buffer, not just
the new consumer.

Example: shrinking the tunnel buffer (4112 B → 512 B) exposed
`serializeDataflashReadReply()`, which computed its space clamp *before*
writing a 4-byte address header — an unconditional 4-byte overflow once the
buffer had no spare headroom.

### Measure real message sizes before sizing a buffer

Audit everything that can reach the buffer, not just the obvious category.
A narrow audit produces a confident-looking wrong answer.

Example: a first pass on the tunnel buffer checked only MSP2 handlers
(432 B largest reply); MSP1 legacy handlers reachable through the same
buffer needed 512 B.

### Chunk a large buffer into a circular buffer when the peripheral drains continuously

If the peripheral consumes the buffer continuously, a circular buffer
refilled from a DMA half/full-transfer interrupt replaces one large static
array. Refill must come from a real hardware DMA interrupt — a
task-scheduler tick leaves gaps that read as a protocol reset.

Example: the WS2812 driver buffered all 128 possible LEDs in one static
array (12,460 B) for a single DMA burst; a circular buffer holding 2 groups
of 4 LEDs (768 B), refilled from the DMA interrupt as each group finishes,
cut it >16x with no protocol change.

### A minimum-duration constraint doesn't need buffer space

A "hold low for at least ~50-60 us" requirement is a minimum, not a
window. Minimums are usually free — idle state already exceeds them —
while maximums aren't. Check which one you have before reserving buffer
space or adding a blocking wait.

Example: the LED driver's 42-element reset/latch preamble became a
`micros()` compare (the line already idles low longer than the minimum);
the idle-tail became a direct write to the timer's preload-buffered
compare register.

## Bounding work

### Bound work to what's actually configured

Process only the configured count when a runtime count is available, even
if the buffer stays sized for the worst case. This cuts CPU work and DMA
transfer proportionally, at zero cost to full-length users.

Example: the LED driver processed all 128 possible slots every update;
threading `ledCounts.count` into the fill loop and DMA transfer length cut
work for short strips.

### Budget speculative reads against the shared cache they consume

A feature that pre-reads a shared cache (tile cache, lookup tables) must
size its speculative distance against that cache's capacity, not against
what the feature would like to know. Otherwise the speculative consumer can
evict the block the cache's primary consumer needs next, turning the cache
into a thrash loop. Compute the budget from the cache size with margin for
the primary consumer, and additionally cap by a time horizon so the scan
scales with what the aircraft can actually reach.

Example: PR #11785's terrain lookahead caps its scan at
`(TERRAIN_GRID_BLOCK_CACHE_SIZE - 3) * 540 m` per query cycle — the `-3`
reserves the current block plus margin — and then at `groundspeed * 35 s`.
On an 8-entry cache (F765/H743) that is a 2,700 m ceiling, with the time
horizon usually binding first. The scan can never evict the block the AGL
query reads.

### Consume an existing cache in place; add no private copy

When a feature needs data from a cache-backed source, read through the
existing cache and mark misses for loading instead of allocating its own
shadow buffer "for cleanliness". A miss is a `return not-ready`, not a
reason to copy.

Example: PR #11785's `terrainNavGetHeightAtLocation()` reads the #11438
terrain grid cache directly and calls `markGridBlockNeedRead()` on a miss;
the whole terrain_nav layer holds zero tile buffers of its own.

## Sending data

### Send components separately instead of concatenating

Concatenating buffers costs the sum of their sizes. If the consumer is
sequential, send the components in order and skip the intermediate buffer.

Example: a protocol of header + large body + trailer saved 4 KB of RAM by
sending the three components in order over serial instead of assembling a
"message" buffer.

## Functions, linkage, and headers

### Don't default to inlining large or multi-call-site functions

Inlining creates one copy of the function body per call site. Let the
compiler decide — it's usually better at this than manual `inline` hints —
and reach for `NOINLINE` on large functions with several call sites that
don't need to be fast-path.

### Be wary of `static` in a header

`static` in a header (with or without `inline`) creates a private copy in
every `.c` file that includes it — of the function *and* of any local
`static` array/struct/table inside it. The linker cannot merge
internal-linkage symbols. `inline` is not the cause; `static` is. When you
don't need internal linkage, prefer `extern`: declare the function/object
in the header, define it exactly once in a matching `.c` file. Vendored
libraries often ship a documented switch for this — check before
hand-patching generated files, which get wiped on regeneration.

Example: `mavlink_get_msg_entry()` in
`lib/main/MAVLink/mavlink_helpers.h` declares its local
`mavlink_message_crcs[]` table (337 entries, 4,044 B on the storm32
dialect) with `static` linkage via the `MAVLINK_HELPER` macro. Two firmware
TUs call the function, so the binary carried two copies — 8,088 B on
BLUEBERRYF405. A grep for `"static inline"` missed it because the macro
hides the `static` keyword from the call site. Defining
`MAVLINK_SEPARATE_HELPERS` plus one compiled `mavlink_helpers.c` removed
the duplication.

Use the audit tool in
`claude/developer/scripts/static-linkage-audit/` (inav-claude tooling
repo) to scan for static-in-header symbols and estimate duplication cost.

### `static` in a header is okay only for pure, stateless functions

The exception to the rule above: a `static inline` function with no static
state — no file-scope statics, no local `static` arrays or tables — has
nothing to duplicate, so it costs no RAM. It is not literally free,
though: inline still multiplies copies when there are multiple callers.
Every call site the compiler inlines embeds its own copy of the body in
flash, and if the compiler cannot inline the function and more than one TU
calls it, each TU gets an out-of-line copy that the linker cannot merge.
Keep such functions small and few-call-site, or fall back to the `extern`
pattern above.

Example: PR #11553's four `*_logic.h` files (~1,700 lines, VTOL
auto-transition) are 100% `static inline` with no top-level static state —
each inlines into its one or two callers or gets discarded, and each has a
unit test that exercises the logic without linking the surrounding
subsystem.

## State machines

### Extend an enum-indexed dispatch table — don't grow an if/else-if chain

New states belong in an existing table/switch as entries, not as branches
bolted onto the dispatch site. A parallel if/else-if chain costs more
flash and adds a second place to keep state in sync.

Example: PR #11553 added navigation FSM states as
`[NAV_STATE_...] = { ... }` entries in `navFSM[NAV_STATE_COUNT]`, and its
mixer-profile transition FSM uses a dense `switch` the compiler can turn
into a jump table.

### Keep a feature's whole static state in ONE caller-owned struct

When a feature's decision logic has many state fields, don't scatter them
as file-scope statics across the module — hold them in one struct that the
caller passes in/out. That makes the feature's RAM cost equal to one
visible `sizeof()`, keeps the core logic pure (unit-testable without
linking the subsystem), and makes reset = one memset-style function
instead of a dozen assignments.

Example: PR #11785's terrain_nav_hold layer keeps its entire state machine
in `terrainNavHoldState_t` (64 B on MATEKF765, verified via nm) plus one
small output struct — the whole feature's static RAM is ~100 B, not
scattered buffers. `terrain_nav_hold_core.c` is pure logic with zero
file-scope state; `terrain_nav_hold.c` owns the single state struct.

## Duplication: state vs code

### Distinguish necessary state duplication from avoidable code duplication

A large duplicated struct may be necessary state — a second copy evolving
in parallel across time — while near-identical duplicated functions are
usually avoidable code, one parameterization away from being unified.
Trace which pattern a duplication finding is before accepting it as an
inherent cost.

Example (necessary state): PR #11553's
`autoTransitionTargetPidState[FLIGHT_DYNAMICS_INDEX_COUNT]` is a full
second copy of `pidState_t` (3,072 B on MATEKF405, ~95% of the PR's net
RAM+CCM growth) — a bumpless handoff needs the *target* controller's
filter/integrator state evolving continuously in parallel, not a single
output value computed once.

Example (avoidable code): the ~10-11 near-line-for-line duplicate
functions in `pid.c` (`applyAutoTransitionTargetDBoost` vs `applyDBoost`,
+6,632 B) exist only because the live PID functions read tuning values
from file-scope globals instead of taking a `pidProfile_t*` — the new
functions already take the profile explicitly, so the duplication is a
refactor away from being unified.

## Measuring size

### Sum every RAM-like region when reconciling against CI

CI's single "RAM Δ" figure on F4/F7/H7 targets can be `RAM` + `CCM`/`DTCM`
summed together, not the `RAM:` linker region alone. Check which regions
the figure spans before suspecting the build or the measurement.

Example: on PR #11553/MATEKF405, `RAM:` alone grew only 608 B against a
reported CI delta of +3,740 B — the gap was CCM growing another 3,128 B
and CI counting both regions as one number.

### In an LTO-merged symbol, check every function folded into it

LTO merges several unrelated functions into one symbol, so an
`nm --size-sort` diff attributes their combined growth to whichever name
survives — usually the caller, not the function that actually grew. Don't
attribute a merged symbol's growth to the most-recently-reviewed nearby
change; check every function folded into the blob.

Example: a +5,168 B growth in `programmingFrameworkUpdateTask` was
initially guessed to be `logic_condition.c`'s new operands; that symbol
actually folds `programmingPidUpdateTask`, `outputProfileUpdateTask`, and
`logicConditionUpdateTask`, and the growth was entirely
`outputProfileUpdateTask()` in `mixer_profile.c`.

### Isolate one function's real cost with NOINLINE at both commits

To measure one function's true standalone cost inside an LTO-merged blob,
add `NOINLINE` to just that function in both the base and HEAD builds so it
gets its own `nm` entry at both points. **Caveat:** `NOINLINE` is
`__attribute__((noinline))` only under `USE_ITCM_RAM` (F7/H7 targets) —
it's a no-op on F4, so run this kind of isolation on an F7/H7 target or
the "before" and "after" builds will look identical for the wrong reason.

Example: `outputProfileUpdateTask()` measured 148 B → 4,708 B once pulled
out of the merged symbol.

## Hardware width pitfalls

### Verify register-width assumptions on real hardware

A DMA transfer's element width must match the target register width. This
class of bug is invisible to the compiler and to code review — confirm on
real hardware with a scope.

Example: the LED strip DMA buffer used `uint8_t` elements (the theoretical
minimum for a duty value); TIM3/TIM4's CCR is a 16-bit register, so the
byte-sized DMA writes only updated its low byte and the output pin stuck
high. `uint16_t`, matching the register width, fixed it.

### Use the codebase's DMA-safe width type, not a narrower derivation

When the codebase defines a type for "the width DMA needs to write this
register safely," use it instead of deriving a narrower one from a single
datasheet — a width verified on one board doesn't prove it on every board,
because different MCU families route through different code branches.

Example: `timerDMASafeType_t` is `uint32_t` on every F4/F7/H7/AT32
`timer_def_*.h`, because DMA writes to a timer CCR must be word-width
across the whole family. A `uint16_t` fix compiled cleanly on H7 but
routed through a previously-unreached branch containing an unrelated
copy-paste bug that silently downgraded the write to a single byte.

## Related

- Full strategy pattern library (more entries, other PRs):
  `claude/projects/active/ram-reduction-program/ram-flash-strategies.md` in
  the `inav-claude` tooling repo (not part of this firmware repo).
- Tracked follow-up: `document-ram-flash-optimization-practices` project —
  expands this into a full guide, links it from `Development.md`, and
  updates review checklists to treat RAM/flash reduction as an active
  review category.

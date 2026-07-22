# Timer and DMA Conflict Resolution

Understanding and resolving timer/DMA conflicts is crucial for target development. STM32 MCUs have limited DMA channels, and conflicts can prevent motors from working properly.

## What Are DMA Conflicts?

**DMA (Direct Memory Access)** allows peripherals like timers to transfer data without CPU intervention. Each timer channel can use specific DMA streams/channels.

**Conflict occurs when:** Multiple timer channels try to use the same DMA stream/channel.

**Result:** One or more motor outputs fail, or only some motors respond to throttle.

## STM32 DMA Architecture

### F4/F7: Fixed DMA Mapping

Each timer channel has 1-3 possible DMA streams, fixed by the MCU's DMA request mapping table (see your MCU's reference manual, e.g. RM0090 for F4, RM0385 for F7 — DMA/DMA2 request mapping chapter).

**Conflict example:**
```c
DEF_TIM(TIM3, CH1, PB4,  TIM_USE_OUTPUT_AUTO, 0, 0), // Uses DMA1_Stream4
DEF_TIM(TIM2, CH3, PB10, TIM_USE_OUTPUT_AUTO, 0, 0), // Also uses DMA1_Stream4!
// ^ CONFLICT - only one will work
```

### H7: DMAMUX (Flexible)

H7 series uses DMAMUX which allows any timer to use any DMA channel - much more flexible and fewer conflicts. Still check for outright *missing* DMA request lines, though — e.g. `TIM15_CH2` has **no** DMA request line at all on H7 (only `TIM15_CH1` does, confirmed in `drivers/timer_def_stm32h7xx.h`), which is a hardware constraint DMAMUX can't route around, not a conflict to resolve.

## Detecting DMA Conflicts

### Symptoms
- Some motors don't respond to throttle
- Motors work in some configurations but not others
- "DMA conflict" warning in INAV CLI `status` command

### Finding Conflicts

Cross-reference every `DEF_TIM()` entry in your `target.c` against your MCU's DMA request mapping table (from its reference manual or STM32CubeMX's pinout view) and check whether any two entries in your timer table resolve to the same DMA stream/channel with the same `dmaopt`.

## DMA Assignment in target.c

The last two parameters in `DEF_TIM()` control DMA assignment:

```c
DEF_TIM(TIM, CH, PIN, USE, dmaopt, dmavar)
                           ^^^^^^  ^^^^^^
```

- **dmaopt (DMA Option):** Which alternate DMA mapping to use (0, 1, 2...)
- **dmavar:** Usually 0 (legacy, not used on modern targets)

### F405/F411 Example

```c
// TIM3_CH1 has only one DMA option: DMA1_Stream4
DEF_TIM(TIM3, CH1, PB4, TIM_USE_OUTPUT_AUTO, 0, 0),  // dmaopt=0

// TIM2_CH4 has two DMA options: DMA1_Stream7 or DMA1_Stream6
DEF_TIM(TIM2, CH4, PB11, TIM_USE_OUTPUT_AUTO, 0, 0), // dmaopt=0 → uses DMA1_Stream7
DEF_TIM(TIM2, CH4, PB11, TIM_USE_OUTPUT_AUTO, 1, 0), // dmaopt=1 → uses DMA1_Stream6
```

## Resolving Conflicts

### Strategy 1: Use Different Timers

Instead of multiple channels on same timer, spread across timers:

```c
// Before (potential conflicts)
DEF_TIM(TIM3, CH1, PB4, TIM_USE_OUTPUT_AUTO, 0, 0),
DEF_TIM(TIM3, CH2, PB5, TIM_USE_OUTPUT_AUTO, 0, 0),
DEF_TIM(TIM3, CH3, PB0, TIM_USE_OUTPUT_AUTO, 0, 0),
DEF_TIM(TIM3, CH4, PB1, TIM_USE_OUTPUT_AUTO, 0, 0),

// After (distributed across timers)
DEF_TIM(TIM3, CH1, PB4, TIM_USE_OUTPUT_AUTO, 0, 0),
DEF_TIM(TIM2, CH3, PB0, TIM_USE_OUTPUT_AUTO, 0, 0),
DEF_TIM(TIM4, CH1, PB6, TIM_USE_OUTPUT_AUTO, 0, 0),
DEF_TIM(TIM1, CH1, PA8, TIM_USE_OUTPUT_AUTO, 0, 0),
```

### Strategy 2: Change DMA Option (dmaopt)

Use alternate DMA mapping:

```c
// Before (conflict)
DEF_TIM(TIM1, CH1, PA8,  TIM_USE_OUTPUT_AUTO, 0, 0), // DMA2_Stream6
DEF_TIM(TIM1, CH3, PA10, TIM_USE_OUTPUT_AUTO, 0, 0), // DMA2_Stream6 - CONFLICT!

// After (use alternate for CH1)
DEF_TIM(TIM1, CH1, PA8,  TIM_USE_OUTPUT_AUTO, 1, 0), // DMA2_Stream1
DEF_TIM(TIM1, CH3, PA10, TIM_USE_OUTPUT_AUTO, 0, 0), // DMA2_Stream6 - OK!
```

### Strategy 3: Check Hardware Design

If conflicts can't be resolved, hardware may have design issue. Check:
- Can pins be reassigned to different timers?
- Are all motor outputs necessary?
- Can LED strip use different pin?

## Pin to Timer Mapping

Not all pins can use all timers. Check STM32 datasheet "Alternate Functions" table.

**Example for PA8:**
- Can be TIM1_CH1 (AF1)
- Can be TIM8_CH1 (AF3)
- Cannot be TIM2, TIM3, TIM4

**Finding alternate functions:**
1. STM32 datasheet → Pinout section
2. Look up pin (e.g., PA8)
3. Check "AF" (Alternate Function) columns
4. Look for TIMx_CHy entries

## Best Practices

1. **Cross-reference against the reference manual** - Always check DMA request mapping before finalizing target.c
2. **Spread Timers** - Don't put all motors on one timer if avoidable
3. **Reserve for UART/SPI** - Some DMA streams may be needed for UART/SPI
4. **Test All Outputs** - Test every motor output after defining timers
5. **Document** - Add comments showing DMA assignment

```c
// Good: Shows DMA usage
DEF_TIM(TIM3, CH1, PB4, TIM_USE_OUTPUT_AUTO, 0, 0), // DMA1_S4
DEF_TIM(TIM3, CH2, PB5, TIM_USE_OUTPUT_AUTO, 0, 0), // DMA1_S5
```

## Testing for Conflicts

After defining timers:

1. **Build firmware**
2. **Flash to FC**
3. **Open INAV Configurator**
4. **Go to Motors tab**
5. **Test each motor slider** - all should respond
6. **Check CLI:** Type `status` - look for DMA warnings

## H7 Advantages

STM32H7 uses DMAMUX which eliminates most DMA conflicts:
- Any timer can use any DMA channel
- Much more flexible
- Fewer headaches for target developers
- Conflicts still possible but rare, and some channels (e.g. `TIM15_CH2`, see above) have no DMA request line at all regardless of DMAMUX

## Related Documentation

- **overview.md** - Target system basics
- **common-issues.md** - See "Timer Configuration" section for real examples
- **creating-targets.md** - Timer setup during target creation
- **STM32 Reference Manual** - Complete DMA tables

## External Resources

- **STM32F4 Reference Manual:** RM0090 - Chapter 10 (DMA)
- **STM32F7 Reference Manual:** RM0385 - Chapter 9 (DMA)
- **STM32H7 Reference Manual:** RM0433 - Chapter 17 (DMAMUX)

# Pattern: Timer Channel Multiplexing for Motor + Servo on Shared TIM

## The Problem

On several INAV-supported flight controllers (notably the Airbot OMNIBUS F4), the first two
signal outputs share the same STM32 hardware timer peripheral:

| Pin | Timer | Channel | Typical Use |
|-----|-------|---------|-------------|
| PB0 | TIM3  | CH3     | M1 (motor)  |
| PB1 | TIM3  | CH4     | S2 (servo)  |

Source: `src/main/target/OMNIBUSF4/target.c`, `timerHardware[]` array.

All channels on a STM32 TIM share one counter and one ARR (auto-reload register), so all
channels must run at the same frequency. This creates an irreconcilable conflict for fixed-wing
aircraft, where the motor ESC needs ~400 Hz and analog servos need ~50 Hz.

### Why DSHOT Doesn't Help Here

DSHOT configures TIM3 to run at its bit-clock frequency (3–12 MHz). TIM3 CH4 (S2) is then
also running at that frequency — completely unusable as a servo. Switching the motor to DSHOT
frees S2 from the 400 Hz conflict but creates a worse one.

---

## The Solution: Timer Channel Multiplexing

Run TIM3 at **400 Hz** (appropriate for the motor on CH3) and manage the servo output on CH4
via **Output Compare interrupts**, firing the servo pulse on every 8th timer overflow.

### Why 8th tick

- Motor period: 1,000,000 Hz / 400 Hz = **2,500 ticks**
- Servo period: 1,000,000 Hz / 50 Hz = **20,000 ticks**
- Ratio: 20,000 / 2,500 = **8 (exact integer)**

The servo sees a pulse every 8 × 2.5 ms = 20 ms — a standard 50 Hz servo signal.

### STM32 Output Modes (per channel)

- **PWM mode**: Pin goes HIGH at counter reset, LOW when counter hits CCR. Pure hardware,
  zero CPU. Used for the motor on CH3.
- **Output Compare mode**: An interrupt fires when the counter hits CCR. Software decides
  what to do (set pin HIGH, set pin LOW, schedule next event). Used for the servo on CH4.

Both modes can coexist on different channels of the same timer.

---

## Implementation Sketch

**This pattern is a design proposal, not an implemented feature** — there is no
working reference implementation in the tree today. The sketch below shows the
timing/logic concept only; it is not something to copy directly. In particular,
`TIM3_IRQHandler` is already defined via the generic
`_TIM_IRQ_HANDLER(TIM3_IRQHandler, 3)` macro (`src/main/drivers/timer_stm32f4xx.c`
and the F7/H7 equivalents), which dispatches through INAV's existing per-channel
callback registration (`timer.c`) — the same mechanism the motor PWM/DSHOT output
on this exact TIM3/CH3 already depends on. A bare second definition of
`TIM3_IRQHandler` as shown below would not link, and bypassing the existing
dispatcher would break every other TIM3 consumer's callback dispatch. A real
implementation needs to register an Output Compare callback through that existing
mechanism, not a raw `IRQHandler` function.

```c
// TIM3 configured: ARR = 2499 (400 Hz), CH3 in PWM mode (motor)
// CH3 CCR set normally by motor output code — no changes needed there.

// CH4 in Output Compare mode (servo)
static uint8_t servoTickCount = 0;
static uint16_t servoPulseWidthTicks = 1500; // 1.5 ms = center

// TIM3 Update (overflow) ISR — fires at 400 Hz
void TIM3_IRQHandler(void) {
    if (TIM_GetITStatus(TIM3, TIM_IT_Update)) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

        if (++servoTickCount >= 8) {
            servoTickCount = 0;
            // Start servo pulse
            SERVO_S2_PIN_HIGH();
            // Schedule falling edge: pulse width ticks from NOW
            // Counter just reset to 0, so target = servoPulseWidthTicks
            TIM3->CCR4 = servoPulseWidthTicks;
            TIM_ITConfig(TIM3, TIM_IT_CC4, ENABLE);
        }
    }
}

// TIM3 CH4 Compare Match ISR — fires once per servo pulse
void TIM3_CC4_IRQHandler(void) {
    if (TIM_GetITStatus(TIM3, TIM_IT_CC4)) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC4);
        SERVO_S2_PIN_LOW();                         // End servo pulse
        TIM_ITConfig(TIM3, TIM_IT_CC4, DISABLE);    // Don't fire again until next servo cycle
    }
}
```

### Pulse Width Calculation

At 1 MHz effective timer clock with 400 Hz period (ARR = 2500):

| Servo position | Pulse width | Ticks |
|----------------|-------------|-------|
| Minimum        | 1000 µs     | 1000  |
| Center         | 1500 µs     | 1500  |
| Maximum        | 2000 µs     | 2000  |

All pulse widths (1000–2000 ticks) fit within one motor period (2500 ticks), so there is no
overflow into the next motor cycle.

---

## Trade-offs

| Factor | Assessment |
|--------|------------|
| Servo jitter | One ISR latency (~100–200 ns on F405). Well within analog servo tolerance (~10 µs). |
| CPU overhead | Two ISR firings per servo pulse at 50 Hz = 100 ISR calls/sec. Negligible on F4. |
| Motor signal | Unaffected — pure hardware PWM on CH3, no ISR involvement. |
| Digital servos | Most accept 333–400 Hz update rates; could simplify to pure PWM at 400 Hz if servo supports it. |
| Code complexity | Non-trivial INAV driver change; output compare path currently unused for servos. |

---

## Where to Implement in INAV

The changes would be concentrated in:

- `src/main/drivers/pwm_output.c` — add an output-compare-driven servo init path
  (alongside the existing `pwmServoConfig()`)
- `src/main/drivers/timer.c` — expose per-channel ISR registration for the shared timer
- Target-specific config (`OMNIBUSF4/target.c`) — tag the affected channel with a new
  `TIM_USE` flag indicating it requires multiplexed servo mode

A new `TIM_USE_SERVO_MUXED` flag (or similar) would allow the timer allocation code to
select this path automatically when it detects a motor and servo on the same TIM peripheral.

---

## Boards Affected

Any board where motor outputs and servo outputs share a TIM peripheral. Known cases:

| Board | Timer | Motor pin | Servo pin |
|-------|-------|-----------|-----------|
| Airbot OMNIBUS F4 | TIM3 | PB0 (CH3) | PB1 (CH4) |

Check `target.c` for other boards: if `DEF_TIM(TIM3, ...)` (or any shared TIM) appears for
both a motor-capable and servo-capable output, the same conflict exists.

---

## Related Files

- `src/main/drivers/pwm_output.c` — `pwmWriteMotor()`/`pwmMotorConfig()` (motor),
  `pwmServoConfig()` (servo), `pwmWriteBeeper()` (beeper)
- `src/main/drivers/pwm_mapping.h` — `PWM_TIMER_HZ` (1,000,000)
- `src/main/target/OMNIBUSF4/target.c` — `timerHardware[]` array

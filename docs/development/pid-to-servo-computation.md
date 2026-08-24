# PID to Servo Deflection Computation

This document explains how INAV computes servo deflection from PID and feedforward (FF) parameters for fixed-wing aircraft.

## Overview

The signal flow is:

```
Rate Target → PID Controller → axisPID[] → Servo Mixer → servo[] → PWM Output
```

## 1. PID Controller Output (Fixed-Wing)

**Source:** `src/main/flight/pid.c`, function `pidApplyFixedWingRateController()`.

### Formula

```
axisPID[axis] = constrain(P + FF + I + D, -limit, +limit)
```

Where:
- **P (Proportional):** `rateError * kP`, low-pass filtered (`pTermProcess()`), then multiplied by the P attenuation factor `aP`
- **FF (FeedForward):** `rateTarget * kFF` — a separate additive term, not multiplied with P or D
- **I (Integral):** accumulated `rateError * kI * dT`, multiplied by the I attenuation factor `aI`, only updated when the axis isn't in an Iterm-freeze state
- **D (Derivative):** `gyroRateDelta * kD / dT`, computed on a (optionally low-pass filtered) gyro rate delta and scaled by a dynamic "D-Boost" factor (`dTermProcess()`), then multiplied by the D attenuation factor `aD`

The attenuation factors `aP`/`aI`/`aD` come from the "Iterm Lock" mechanism (`iTermLockApply()`), which damps P/I/D response during large stick inputs so the feedforward term dominates, and locks the I-term to prevent windup/bounceback after a rapid stick release.

| Variable | Meaning |
|----------|---------|
| `rateError` | `rateTarget - gyroRate` (desired minus actual) |
| `rateTarget` | Desired rotation rate from stick input or autopilot |
| `gyroRate` | Actual measured rotation rate from gyro |
| `kP, kI, kD, kFF` | PID gains (from settings) |
| `aP, aI, aD` | Attenuation factors from Iterm Lock (see above) |
| `dT` | Time delta between iterations |
| `limit` | `getPidSumLimit()` — maximum PID output |

### The Math Is Additive

**All four terms (P, FF, I, D) are added together:**

```c
axisPID[pidState->axis] = constrainf(newPTerm + newFFTerm + pidState->errorGyroIf + newDTerm, -limit, +limit);
```

FeedForward is not multiplied with P or D — it's a separate additive term.

## 2. Servo Mixer

**Source:** `src/main/flight/servos.c`, function `servoMixer()`.

### Input Sources

The mixer receives PID output as one of several input sources:

```c
input[INPUT_STABILIZED_ROLL]  = axisPID[ROLL];   // PID output
input[INPUT_STABILIZED_PITCH] = axisPID[PITCH];  // PID output
input[INPUT_STABILIZED_YAW]   = axisPID[YAW];    // PID output
```

In manual mode, raw RC commands are used instead:

```c
input[INPUT_STABILIZED_ROLL] = rcCommand[ROLL];  // RC stick
```

### Mixing Rules

Each servo can have multiple mixer rules that add together:

```c
int16_t inputLimited = (int16_t) rateLimitFilterApply4(&servoSpeedLimitFilter[i], inputRaw, currentServoMixer[i].speed * 10, dT);
servo[target] += ((int32_t)inputLimited * currentServoMixer[i].rate) / 100;
```

- `inputRaw` is first passed through a rate limiter (a rule's `speed` field, 0 = no limiting) before being applied — this is what lets a mixer rule slew a servo gradually instead of jumping instantly.
- `currentServoMixer[i].rate`: percentage weight from -1000 to +1000 (representing -1000% to +1000%)
- Multiple rules targeting the same servo channel are summed

### Example

If a servo has two rules:
- Rule 1: `INPUT_STABILIZED_PITCH` at 100%
- Rule 2: `INPUT_FEATURE_FLAPS` at 50%

```
servo[n] = (axisPID[PITCH] * 100 / 100) + (flapValue * 50 / 100)
```

## 3. Servo Scaling and Output

After mixing, each servo is scaled individually (verified against the exact sequence in `servoMixer()`):

### Step 1: Apply Servo Rate

```c
servo[i] = (servoParams(i)->rate * servo[i]) / 100;
```

`rate` is typically 100 but can go higher/lower for asymmetric throws.

### Step 2: Apply Asymmetric Scaling

Different scaling for positive vs. negative deflection:

```c
if (servo[i] > 0) {
    servo[i] *= scaleMax;  // = (max - middle) / 500.0
} else {
    servo[i] *= scaleMin;  // = (middle - min) / 500.0
}
```

This allows different throw in each direction.

### Step 3: Add Midpoint

```c
servo[i] += servoParams(i)->middle;  // typically 1500us
```

### Step 4: Constrain to Limits

```c
servo[i] = constrain(servo[i], servoParams(i)->min, servoParams(i)->max);  // typically 1000-2000us
```

## 4. Complete Signal Path Example

For a roll servo on a fixed-wing (illustrative numbers, not measured telemetry):

```
1. Pilot moves stick right (roll)
   → rateTarget = 200 deg/s

2. Gyro measures actual: gyroRate = 50 deg/s
   → rateError = 200 - 50 = 150 deg/s

3. PID computes (with example gains, ignoring filtering/attenuation for clarity):
   P = 150 * 0.5 = 75
   FF = 200 * 0.3 = 60
   I = accumulated = 10
   D = (delta) * 0.1 = 5
   → axisPID[ROLL] = 75 + 60 + 10 + 5 = 150

4. Servo mixer applies rule (100% weight):
   → servo[0] = 150 * 100 / 100 = 150

5. Servo scaling (rate=100%, middle=1500, min=1000, max=2000):
   → scaleMax = (2000-1500)/500 = 1.0
   → servo[0] = 150 * 1.0 = 150
   → servo[0] = 150 + 1500 = 1650us

6. Final PWM output: 1650us
```

## 5. Key Functions Reference

| Component | File | Function |
|-----------|------|----------|
| PID Controller | `flight/pid.c` | `pidApplyFixedWingRateController()` |
| P-term | `flight/pid.c` | `pTermProcess()` |
| D-term | `flight/pid.c` | `dTermProcess()` |
| Iterm Lock / attenuation | `flight/pid.c` | `iTermLockApply()` |
| Servo Mixer | `flight/servos.c` | `servoMixer()` |
| Servo Scaling | `flight/servos.c` | `servoComputeScalingFactors()` |

## 6. Summary

**Is P multiplied by D?** No. P and D are computed independently and added together.

**What about FF?** FF is also added (not multiplied). It's computed from the rate target directly, providing feedforward response.

**The complete formula:**

```
axisPID = P + FF + I + D

servo = (axisPID * mixerRate / 100) * servoRate / 100 * scale + middle
```

Where `scale` is `scaleMax` for positive or `scaleMin` for negative deflection.

# RadioMaster Nexus (Original)

Flight controller originally designed for helicopters using Rotorflight.
Based on STM32F722RET6. This is the **original** (discontinued) Nexus,
not the Nexus-X or Nexus-XR.

For the Nexus-X and Nexus-XR, use the `NEXUSX` target instead.

## Hardware

| Component | Spec |
|-----------|------|
| MCU | STM32F722RET6 (216MHz, 256KB RAM, 512KB flash) |
| IMU | ICM-42688-P (SPI1), CW90 alignment |
| Barometer | SPL06-001 (I2C1, internal) |
| Blackbox | W25N01G 128MB (SPI2) |
| Input voltage | 5 - 12.6V |
| BEC output | 5V / 2A on Port A, B, C |
| DSM power | 3.3V / 0.5A |
| Dimensions | 41.3 x 25.4 x 13.1mm |
| Weight | 16.8g |

## Differences from Nexus-X/XR

| | OG Nexus | Nexus-X/XR |
|---|----------|------------|
| IMU EXTI | PA15 | PB8 |
| IMU alignment | CW90 | CW180 |
| Baro I2C | I2C1 (PB8/PB9) | I2C2 (PB10/PB11) |
| Flash | W25N01G (128MB) | W25N02K (256MB) |
| Internal ELRS RX | None | RP4TD-M on UART5 |
| PINIO1 (RX power) | None | PC8 |
| UART1 pins | PA9/PA10 | PB6/PB7 |
| Voltage sense | Vin ADC only (5-12.6V) | EXT-V input (3.6-70V) |
| Servo outputs | 5 | 7 default (9 max) |
| Rotorflight target | NEXUS_F7 | NEXUSX |

## Pin Functions

### Default Output Assignment

| Output | Pin | Timer | Connector |
|--------|-----|-------|-----------|
| S1 | PB4 | TIM3_CH1 | Servo header |
| S2 | PB5 | TIM3_CH2 | Servo header |
| S3 | PB0 | TIM3_CH3 | Servo header |
| S4 | PB3 | TIM2_CH2 | Servo header (Tail) |
| M1 | PB6 | TIM4_CH1 | ESC header |

### UART Ports

| UART | Label | TX | RX | Notes |
|------|-------|----|----|-------|
| UART1 | DSM | PA9 | PA10 | |
| UART2 | SBUS | PA2 | PA3 | Shared with FREQ/PPM |
| UART3 | Port C | PB11 | PB10 | Shared with I2C2 |
| UART4 | Port A | PA1 | PA0 | Primary receiver (CRSF) |
| UART6 | Port B | PC7 | PC6 | |

### I2C Buses

| Bus | SCL | SDA | Usage |
|-----|-----|-----|-------|
| I2C1 | PB8 | PB9 | Internal barometer (SPL06) |
| I2C2 | PB10 | PB11 | External sensors via Port C |

### ADC Channels

| Channel | Pin | Divider | Usage |
|---------|-----|---------|-------|
| BUS | PC2 | 320 | Vin rail, 5-12.6V (mapped as VBAT) |
| BEC | PC1 | 160 | 5V BEC rail monitoring |

Note: Unlike the Nexus-X/XR, the OG Nexus has no dedicated EXT-V
high-voltage battery sense input. VBAT monitors the board input power.

### Connector Pinouts

**Port A (UART4 - CRSF receiver):**
1. GND
2. 5V
3. RX (PA0, connect to receiver TX)
4. TX (PA1, connect to receiver RX / telemetry)

**Port B (UART6):**
1. GND
2. 5V
3. TX (PC7)
4. RX (PC6)

**Port C (UART3 / I2C2):**
1. GND
2. 5V
3. SDA/TX (PB11)
4. SCL/RX (PB10)

**ESC Header:**
- Signal: PB6 (TIM4_CH1 PWM)

## Typical Glider Setup (Elevon / Flying Wing)

For a 5-channel elevon glider like the Kunai:

1. Flash NEXUS target via DFU
2. Set aircraft type: **Flying Wing**
3. Connect RP3-H receiver to **Port A** (CRSF on UART4)
4. Connect ESC to **ESC header** (M1)
5. Connect left elevon servo to **S1**
6. Connect right elevon servo to **S2**
7. Configure elevon mixing in the Mixer tab
8. Vario: SPL06 baro provides altitude-based vario out of the box
9. GPS (Phase 2): Connect to **Port B** (UART6)

## Verified on Hardware

- [x] MCU boots, LEDs active (PC14, PC15)
- [x] USB CDC enumeration and iNAV CLI
- [x] IMU (gyro + accel) detected and responding
- [x] Gyro alignment (CW90) confirmed
- [x] Accelerometer working
- [x] Barometer (SPL06 on I2C1) working
- [x] VBAT ADC (PC2) working (scale 320)
- [x] All UART ports (1-4, 6) verified
- [x] UART4 CRSF receiver working (TX/RX swap confirmed)
- [x] DShot on all motor outputs
- [x] Servo outputs working
- [x] Blackbox logging working
- [x] LEDs working
- [x] I2C2 bus working

## Building

Requires NixOS flake (included in repo root) or standard iNAV build deps.

```sh
nix develop --impure   # or set up arm-none-eabi-gcc manually
mkdir -p build && cd build
cmake -GNinja -DCOMPILER_VERSION_CHECK=OFF ..
ninja NEXUS
```

## Flashing via DFU

Hold button while connecting USB, then:

```sh
arm-none-eabi-objcopy -O binary build/bin/NEXUS.elf build/NEXUS.bin
dfu-util -a 0 -s 0x08000000:leave -D build/NEXUS.bin
```

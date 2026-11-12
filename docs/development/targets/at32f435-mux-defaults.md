# AT32F435/437 INAV MUX Defaults

The AT32F435 uses `GPIO_MUX_n` (MUX0-MUX15) registers to select alternate
functions per pin — the AT32 equivalent of STM32's AFn numbering. For the
full MUX-number-to-peripheral-group table, see the AT32F435/437 Reference
Manual's alternate function chapter.

## INAV Default MUX Values

In INAV target.h, MUX values are NOT specified for standard pin assignments;
INAV uses these defaults from the driver source:

| Peripheral | Default MUX | Override in target.h |
|------------|-------------|----------------------|
| SPI1, SPI2 | GPIO_MUX_5 | `SPI1_SCK_AF`, `SPI1_MISO_AF`, `SPI1_MOSI_AF` |
| SPI3, SPI4 | GPIO_MUX_6 | `SPI3_SCK_AF`, etc. |
| I2C1/2/3   | GPIO_MUX_4 | (always MUX4 for I2C — hardcoded in `bus_i2c_at32f43x.c`, no override) |
| USART1/2/3 | GPIO_MUX_7 | `UART1_AF` or `UART1_TX_AF`/`UART1_RX_AF` |
| UART4–8    | GPIO_MUX_8 | `UART4_AF` etc. |

## Timer DEF_TIM() Notes

In `target.c`, `DEF_TIM(TMR3, CH3, PB0, ...)` automatically resolves
the correct MUX number from the `DEF_TIM_AF__PB0__TCH_TMR3_CH3` macro
defined in `timer_def_at32f43x.h`. The `af` (flags) parameter in
`DEF_TIM` is unused for AT32 (pass 0).

## Related Documentation

- `timer-dma-conflicts.md` — DMA conflict theory (AT32 uses DMAMUX, similar to STM32H7)

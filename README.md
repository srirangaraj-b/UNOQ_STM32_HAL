<<<<<<< HEAD
# UNOQ_STM32_HAL

A minimal, register-level HAL for the STM32U5-based Arduino UNO Q. No ST
HAL/LL layer in the way — direct CMSIS register access, same approach as a
hand-rolled bare-metal driver, just packaged as a reusable library with a
generic API instead of one hardcoded sketch.

## Install

1. Zip this folder as `UNOQ_STM32_HAL.zip` (make sure `library.properties`
   sits at the top level of the zip, not nested one folder deeper).
2. Arduino IDE → **Sketch → Include Library → Add .ZIP Library...** → select
   the zip.
3. `File → Examples → UNOQ_STM32_HAL` will show the two example sketches.

## What's in it

- **GPIO**: `pinMode_STM`, `digitalWrite_STM`, `digitalRead_STM`, `togglePin_STM`
- **PWM output**: `PWM_Setup` / `PWM_Setup_STM` (any timer/channel, optional
  complementary output + dead-time on TIM1/TIM8), `PWM_AddChannel(_STM)` for
  additional channels on an already-configured timer, `PWM_SetDuty`,
  `PWM_SetFrequency`, `PWM_SetDeadTime`
- **ADC**: `adc1Read_STM` — polled single-conversion read on ADC1
- **PWM input capture**: `PWM_Capture_Init_CH1` + `PWM_Capture_GetDuty/
  GetFrequency/GetHighTicks/GetPeriodTicks`
- **Pin table**: `PA0`...`PI7` matching the UNO Q silkscreen, as `GP_Pin`
  structs you pass straight into the `_STM` function variants

## Before you rely on exact numbers

This was distilled from a working sketch, generalized to cover any
timer/channel/pin instead of one hardcoded configuration. A few things are
still board/clock-config dependent and are flagged with `NOTE:` comments in
the source — check these against your actual clock tree and the STM32U585
reference manual (RM0456) before trusting exact PWM frequency, dead-time, or
ADC channel numbers:

- `UNOQ_TIM_CLK_HZ` (default 160 MHz) — override it with a `#define` before
  `#include <UNOQ_STM32_HAL.h>` if your timer input clock differs
- The `af` argument to every PWM/capture call — always the caller's
  responsibility, cross-check against the AF table for your exact pin
- ADC1 channel numbers in `adc1Read_STM` (peripheral channel, not pin number)
- `PWM_SetDeadTime`'s DTG encoding only covers the 0–127×tDTS range

## Using these pins with other Arduino libraries

Every `GP_Pin` (the `PA8`, `PB0`, ... table) implicitly converts to the Arduino
core's own digital pin number, so you can hand it straight to any standard
library that expects an `int` pin - `LiquidCrystal`, `Servo`, `SoftwareSerial`,
`SPI`/`Wire` pin overrides, and so on. No separate lookup table to maintain:

```cpp
#include <UNOQ_STM32_HAL.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(PB6, PB7, PC0, PC1, PA4, PA5); // GP_Pin -> Arduino pin, automatic
```

Under the hood this goes through the STM32 core's own `PinName` / `GPIOPort[]`
mapping (`GP_ToArduinoPin()` does the conversion explicitly, if you'd rather
call it directly or check the result). If a pin you picked isn't exposed in
the current board variant's Arduino pin table, the conversion returns `NC`
(`0xFFFFFFFF`) - our own register-level HAL functions still work on that pin
regardless, since they never go through the Arduino pin-number layer; it's
only *other* libraries built on `digitalWrite`/`pinMode` that need a valid
mapping.

## Examples

- `GPIO_PWM_ADC_Basics` — smallest possible smoke test: one digital output,
  one PWM channel, one ADC read
- `PWM_UART_Control` — the full 4-channel PWM-over-serial demo (same command
  set as the original sketch: `"<ch> <duty>"` and `"p <freq_hz>"`)
=======
# UNOQ_STM32_HAL
>>>>>>> bbbefbaa9abc4115c761d6e7a178978256fc5f8a

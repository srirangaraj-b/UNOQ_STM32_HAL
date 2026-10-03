# UNOQ_STM32_HAL

A high-performance, register-level Hardware Abstraction Layer (HAL) library designed specifically for the **STM32U585** microcontroller on the **Arduino UNO Q**.

This library bridges the gap between the Zephyr-based Arduino core and direct hardware control, offering robust, low-overhead support for **GPIO**, **PWM Generation**, **PWM Input Capture**, and **ADC Analog Readings**—fully optimized for rigid hardware layouts.

---

## Features

* **Advanced GPIO Control:** Fast atomic bit-manipulation (`BSRR`), direct register access, and automatic alternate-function (AF) release to force system-locked pins into pure software GPIO mode.
* **Hardware PWM Output:** Multi-channel support across STM32 timers (`TIM1`, `TIM2`, `TIM3`, `TIM4`, `TIM5`, `TIM8`, `TIM15`, `TIM16`, `TIM17`) with preloaded duty cycles, frequency scaling, dead-time insertion, and complementary outputs.
* **PWM Input Capture:** Hardware-assisted frequency and duty cycle measurement using timer input capture modes.
* **Flexible ADC Helpers:** 12-bit analog input reading supporting both string-based pin names (e.g., `"PA0"`) and native `GP_Pin` objects, with built-in millivolt conversion and safe Zephyr ADC initialization.
* **Arduino Compatibility:** Seamless integration with standard Arduino APIs and libraries (such as `LiquidCrystal`).

---

## Installation

1. Download or clone this repository into your Arduino libraries folder:
* `Documents/Arduino/libraries/UNOQ_STM32_HAL/`


2. Ensure you have the **Arduino UNO Q** hardware core installed via the Arduino IDE Board Manager.
3. Restart the Arduino IDE.

---

## API Quick Reference

### 1. GPIO & Digital I/O

```cpp
pinMode_STM(pin, mode, pull); // e.g., pinMode_STM(PA5, OUTPUT);
digitalWrite_STM(pin, state); // HIGH or LOW (1 or 0)
uint8_t val = digitalRead_STM(pin);
togglePin_STM(pin);

```

### 2. ADC Analog Reading

The library supports robust ADC measurements via pin name strings, pin objects, or convenience macros:

```cpp
// Read raw 12-bit value (0 - 4095)
int raw1 = analogRead_STM("PA0");         // Using string name
int raw2 = analogRead_Pin_STM(PA0);       // Using GP_Pin object
int raw3 = ADC_RAW(PA0);                  // Using macro shortcut

// Read converted millivolts (mV)
int mv1  = analogReadMilliVolts_STM("PA0");
int mv2  = analogReadMilliVolts_Pin_STM(PA0);
int mv3  = ADC_MV(PA0);

```

### 3. PWM Output

```cpp
// Setup PWM on a pin (e.g., TIM3, Channel 1, 1 kHz, 50% duty, AF2)
PWM_Setup_STM(PA6, TIM3, 1, false, 1000, 50, 2);

// Change duty cycle or frequency dynamically
PWM_SetDuty(TIM3, 1, false, 75); // 75% duty
PWM_SetFrequency(TIM3, 1, false, 2000); // Change to 2 kHz

```

---

## Example Usage: Analog Sensor Reading

```cpp
#include <UNOQ_STM32_HAL.h>

const GP_Pin sensorPin = PA0;

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("ADC Initialized.");
}

void loop() {
  // Read raw value and calculated voltage
  int rawVal = analogRead_Pin_STM(sensorPin);
  int milliVolts = analogReadMilliVolts_Pin_STM(sensorPin);

  Serial.print("Raw ADC (0-4095): ");
  Serial.print(rawVal);
  Serial.print(" | Voltage: ");
  Serial.print(milliVolts);
  Serial.println(" mV");

  delay(1000);
}

```

---

## Example Usage: 20x4 LCD Interop

You can easily pass library pin objects directly into standard Arduino libraries like `LiquidCrystal`:

```cpp
#include <UNOQ_STM32_HAL.h>
#include <LiquidCrystal.h>

// LiquidCrystal(rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(PE7, PE8, PF14, PF15, PA3, PD8);

void setup() {
  lcd.begin(20, 4); // 20 columns, 4 rows
  lcd.setCursor(0, 0);
  lcd.print("Arduino UNO Q");
  lcd.setCursor(0, 1);
  lcd.print("STM32U585 Active");
}

void loop() {
  lcd.setCursor(0, 2);
  lcd.print("Uptime: ");
  lcd.print(millis() / 1000);
  lcd.print("s   ");
  delay(500);
}

```

---

## License

This project is open-source and available under the MIT License.
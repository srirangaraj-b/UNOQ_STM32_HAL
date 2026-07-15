#ifndef UNOQ_STM32_HAL_H
#define UNOQ_STM32_HAL_H

/* ============================================================================
 * UNOQ_STM32_HAL
 * Minimal register-level HAL for the STM32U5-based Arduino UNO Q.
 *
 * No ST HAL/LL dependency - direct CMSIS register access, same style as the
 * original working sketch this library was distilled from. Kept intentionally
 * thin so it stays fast and predictable; you are still expected to know which
 * pin/timer/AF combination you are wiring up (check the UNO Q pinout + the
 * STM32U585 datasheet Alternate Function table before calling PWM_Setup).
 *
 * Timer clock assumption:
 *   All frequency/duty math uses UNOQ_TIM_CLK_HZ (default 160 MHz, the
 *   typical APB2 timer clock for TIM1/TIM8 on the stock UNO Q clock config).
 *   If you reconfigure SystemClock (different PLL/prescaler settings), define
 *   UNOQ_TIM_CLK_HZ to your actual timer input clock *before* including this
 *   header, e.g.:
 *       #define UNOQ_TIM_CLK_HZ 96000000UL
 *       #include <UNOQ_STM32_HAL.h>
 * ==========================================================================*/

#include <stdint.h>
#include <stdbool.h>
#include <Arduino.h>
#include "stm32u5xx.h"   /* STM32U5 CMSIS device header */

#ifndef UNOQ_TIM_CLK_HZ
#define UNOQ_TIM_CLK_HZ 160000000UL /* verify against your actual clock config */
#endif

/* ArduinoCore-zephyr (used on the UNO Q) doesn't define NC the way stm32duino
 * does, so we provide it ourselves - guarded, in case a future core version
 * does define it. */
#ifndef NC
#define NC ((uint32_t)0xFFFFFFFF)
#endif

typedef struct GP_Pin {
    GPIO_TypeDef* port;
    uint8_t pin;
#ifdef __cplusplus
    /* Implicit conversion to the Arduino core's own digital pin number, so any
     * GP_Pin from the table below can be passed directly wherever a normal
     * Arduino library expects an int pin - LiquidCrystal, Servo, SoftwareSerial,
     * SPI.pins()/Wire.setSCL() overrides, etc. Goes through the STM32 core's own
     * PinName <-> digital-pin mapping, so it stays correct for whichever variant
     * you're compiling for. See GP_ToArduinoPin() below for the explicit form. */
    operator uint32_t() const;
#endif
} GP_Pin;

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------- GPIO / pin config ---------------- */

/* Explicit form of the GP_Pin -> Arduino digital pin number conversion (same
 * thing the implicit operator above does). Returns NC (0xFFFFFFFF) if this
 * physical pin isn't exposed in the board variant's Arduino pin table -
 * our own register-level functions still work on it either way, but standard
 * Arduino-API-based libraries won't be able to address it by pin number. */
uint32_t GP_ToArduinoPin(GP_Pin pin);

/* Enable the AHB2 clock for a given GPIO port (idempotent, safe to call repeatedly). */
void enableGPIOClock(GPIO_TypeDef* port);

/* Put a pin into alternate-function mode with the given AF number (push-pull, high speed, no pull). */
void SetPinAF(GPIO_TypeDef* port, uint8_t pin, uint8_t af);

/* mode: INPUT_STM / OUTPUT_STM / ANALOG_STM.  pull: NOPULL_STM / PULLUP_STM */
void pinMode_STM(GP_Pin pin, uint8_t mode, uint8_t pull);
void digitalWrite_STM(GP_Pin pin, uint8_t state);
uint8_t digitalRead_STM(GP_Pin pin);
void togglePin_STM(GP_Pin pin);

/* One-shot single-conversion read on ADC1. adc_ch is the ADC input channel number
 * (NOT the pin number) - check the datasheet's ADC1 channel assignment table. */
int adc1Read_STM(GP_Pin pin, uint8_t adc_ch);

/* ---------------- PWM output ---------------- */

/* Configure PWM on any timer/channel from raw port+pin.
 * complementary is only honored on TIM1/TIM8 (the two advanced-control timers with
 * CHxN outputs + dead-time); it is silently ignored on general-purpose timers. */
void PWM_Setup(GPIO_TypeDef* port, uint8_t pin, TIM_TypeDef* timer, uint8_t channel,
               bool complementary, uint32_t freq, uint8_t dutyPercent, uint8_t af);

/* Same as PWM_Setup but takes a GP_Pin from the pin table below. */
void PWM_Setup_STM(GP_Pin pin, TIM_TypeDef* timer, uint8_t channel,
                    bool complementary, uint32_t freq, uint8_t dutyPercent, uint8_t af);

/* Add another channel on a timer that already has PWM_Setup applied (shares that
 * timer's PSC/ARR, i.e. shares its frequency). GPIO must already be in AF mode. */
void PWM_AddChannel(TIM_TypeDef* timer, uint8_t channel, bool complementary, uint8_t dutyPercent);

/* Same, but also configures the GPIO/AF for you. */
void PWM_AddChannel_STM(GP_Pin pin, TIM_TypeDef* timer, uint8_t channel,
                         bool complementary, uint8_t dutyPercent, uint8_t af);

/* Change duty cycle (0-100) of an already-configured channel. */
void PWM_SetDuty(TIM_TypeDef* timer, uint8_t channel, bool complementary, uint8_t dutyPercent);

/* Change PWM frequency for an entire timer (affects all channels on it).
 * Rescales CCR1..CCR4 proportionally so existing duty ratios are preserved. */
void PWM_SetFrequency(TIM_TypeDef* timer, uint8_t channel, bool complementary, uint32_t freq);

/* Dead-time insertion (TIM1/TIM8 only - no-op on general-purpose timers).
 * dead_ns: desired dead time in nanoseconds. timer_clk: timer input clock in Hz
 * (pass UNOQ_TIM_CLK_HZ unless you know your prescaled deadtime clock differs).
 * NOTE: this simple version only covers the DTG[7]=0 range (0..127 x tDTS);
 * for longer dead times consult RM0456 DTG encoding and extend as needed. */
void PWM_SetDeadTime(TIM_TypeDef *TIMx, uint32_t dead_ns, uint32_t timer_clk);

/* ---------------- PWM input capture (CH1/CH2 pair, "PWM input mode") ---------------- */

/* Configures the timer's CH1 as the signal input and internally uses CH2 as the
 * paired capture (standard ST "PWM input" trick) - measures external PWM period + duty. */
void PWM_Capture_Init_CH1(GP_Pin pin, TIM_TypeDef *timer, uint8_t af);

float PWM_Capture_GetDuty(TIM_TypeDef *timer);          /* 0.0 - 100.0 %        */
float PWM_Capture_GetFrequency(TIM_TypeDef *timer);     /* Hz                   */
uint32_t PWM_Capture_GetHighTicks(TIM_TypeDef *timer);  /* raw CCR2 (high time) */
uint32_t PWM_Capture_GetPeriodTicks(TIM_TypeDef *timer);/* raw CCR1 (period)    */

#ifdef __cplusplus
}
#endif

/* ---------------- Mode / pull constants ---------------- */
#define INPUT_STM       0
#define OUTPUT_STM      1
#define ANALOG_STM      2

#define NOPULL_STM      0
#define PULLUP_STM      1

/* ---------------- UNO Q pin table ---------------- */

/* PORT A */
extern GP_Pin PA0, PA1, PA3, PA4, PA5, PA6, PA7, PA8, PA11, PA12;

/* PORT B */
extern GP_Pin PB0, PB1, PB2, PB3, PB4, PB6, PB7, PB8, PB9, PB10, PB11, PB13, PB14, PB15;

/* PORT C */
extern GP_Pin PC0, PC1, PC2, PC3, PC6, PC7, PC8, PC9; /* PC2=MISO, PC3=MOSI (#JSPI) */

/* PORT D */
extern GP_Pin PD1, PD2, PD8, PD9, PD12, PD13; /* PD12=QWIIC SCL, PD13=QWIIC SDA */

/* PORT E */
extern GP_Pin PE2, PE3, PE4, PE5, PE6, PE7, PE8;

/* PORT F */
extern GP_Pin PF14, PF15;

/* PORT H (#RGB LEDS) */
extern GP_Pin PH10, PH11, PH12, PH13, PH14, PH15;

/* PORT I */
extern GP_Pin PI4, PI5, PI6, PI7;

#endif /* UNOQ_STM32_HAL_H */

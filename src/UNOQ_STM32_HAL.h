/*
 * UNOQ_STM32_HAL.h - Register-level GPIO / PWM / PWM-capture / ADC helpers
 *                    for the STM32U585 on the Arduino UNO Q.
 */
#ifndef UNOQ_STM32_HAL_H
#define UNOQ_STM32_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <Arduino.h>
#include "stm32u5xx.h"

#ifndef UNOQ_TIM_CLK_HZ
#define UNOQ_TIM_CLK_HZ 160000000UL
#endif

#ifndef NC
#define NC ((uint32_t)0xFFFFFFFF)
#endif

typedef struct GP_Pin {
    GPIO_TypeDef* port;
    uint8_t pin;    /* 0..15 */
#ifdef __cplusplus
    operator uint32_t() const;
#endif
} GP_Pin;

/* ---- pinMode_STM() modes ---- */
#define INPUT_STM       0
#define OUTPUT_STM      1   /* push-pull  */
#define ANALOG_STM      2
#define OUTPUT_OD_STM   3   /* open-drain */

/* ---- pinMode_STM() pull configuration ---- */
#define NOPULL_STM      0
#define PULLUP_STM      1
#define PULLDOWN_STM    2

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Interop with Arduino / Zephyr Devicetree                          */
/* ------------------------------------------------------------------ */
uint32_t GP_ToArduinoPin(GP_Pin pin);

/* ------------------------------------------------------------------ */
/* GPIO                                                               */
/* ------------------------------------------------------------------ */
void    enableGPIOClock(GPIO_TypeDef* port);
void    SetPinAF(GPIO_TypeDef* port, uint8_t pin, uint8_t af);
void    pinMode_STM(GP_Pin pin, uint8_t mode, uint8_t pull = NOPULL_STM);
uint8_t digitalRead_STM(GP_Pin pin);
void    digitalWrite_STM(GP_Pin pin, uint8_t state);   /* atomic (BSRR) */
void    togglePin_STM(GP_Pin pin);

/* ------------------------------------------------------------------ */
/* PWM output                                                         */
/* ------------------------------------------------------------------ */
bool PWM_Setup(GPIO_TypeDef* port, uint8_t pin, TIM_TypeDef* timer, uint8_t channel,
               bool complementary, uint32_t freq, uint8_t dutyPercent, uint8_t af);
bool PWM_Setup_STM(GP_Pin pin, TIM_TypeDef* timer, uint8_t channel,
                   bool complementary, uint32_t freq, uint8_t dutyPercent, uint8_t af);

bool PWM_AddChannel(TIM_TypeDef* timer, uint8_t channel, bool complementary, uint8_t dutyPercent);
bool PWM_AddChannel_STM(GP_Pin pin, TIM_TypeDef* timer, uint8_t channel,
                       bool complementary, uint8_t dutyPercent, uint8_t af);

bool PWM_SetDuty(TIM_TypeDef* timer, uint8_t channel, bool complementary, uint8_t dutyPercent);
bool PWM_SetDutyPermille(TIM_TypeDef* timer, uint8_t channel, uint16_t permille);
bool PWM_SetDutyRaw(TIM_TypeDef* timer, uint8_t channel, uint32_t ticks);

bool PWM_SetFrequency(TIM_TypeDef* timer, uint8_t channel, bool complementary, uint32_t freq);

bool PWM_Enable(TIM_TypeDef* timer, uint8_t channel, bool complementary, bool enable);
bool PWM_Stop(TIM_TypeDef* timer, uint8_t channel, bool complementary);
bool PWM_SetPolarity(TIM_TypeDef* timer, uint8_t channel, bool complementary, bool activeHigh);

bool PWM_SetDeadTime(TIM_TypeDef* timer, uint32_t dead_ns, uint32_t timer_clk);

/* ------------------------------------------------------------------ */
/* PWM input capture                                                  */
/* ------------------------------------------------------------------ */
bool     PWM_Capture_Init(GP_Pin pin, TIM_TypeDef* timer, uint8_t af, uint8_t inputChannel);
bool     PWM_Capture_Init_CH1(GP_Pin pin, TIM_TypeDef* timer, uint8_t af);
bool     PWM_Capture_Init_CH2(GP_Pin pin, TIM_TypeDef* timer, uint8_t af);
bool     PWM_Capture_Init_Ex(GP_Pin pin, TIM_TypeDef* timer, uint8_t af,
                            uint8_t inputChannel, uint16_t prescaler);
void     PWM_Capture_SetTimeout(uint32_t ms);
bool     PWM_Capture_IsActive(TIM_TypeDef* timer);
uint32_t PWM_Capture_GetPeriodTicks(TIM_TypeDef* timer);
uint32_t PWM_Capture_GetHighTicks(TIM_TypeDef* timer);
float    PWM_Capture_GetFrequency(TIM_TypeDef* timer);
float    PWM_Capture_GetDuty(TIM_TypeDef* timer);

/* ------------------------------------------------------------------ */
/* ADC (STM32U585 ADC1 Helpers)                                      */
/* ------------------------------------------------------------------ */
int analogRead_STM(const char *pinName);
int analogReadMilliVolts_STM(const char *pinName);
int analogRead_Pin_STM(GP_Pin pin);
int analogReadMilliVolts_Pin_STM(GP_Pin pin);

#ifdef __cplusplus
}
#endif

/* Convenience Macros */
#define ADC_RAW(pin) analogRead_STM(#pin)
#define ADC_MV(pin)  analogReadMilliVolts_STM(#pin)

/* ------------------------------------------------------------------ */
/* UNO Q pin table                                                    */
/* ------------------------------------------------------------------ */
extern const GP_Pin PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7, PA8, PA9, PA10, PA11, PA12, PA13, PA14, PA15;
extern const GP_Pin PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7, PB8, PB9, PB10, PB11, PB12, PB13, PB14, PB15;
extern const GP_Pin PC0, PC1, PC2, PC3, PC4, PC5, PC6, PC7, PC8, PC9, PC10, PC11, PC12, PC13, PC14, PC15;
extern const GP_Pin PD0, PD1, PD2, PD3, PD4, PD5, PD6, PD7, PD8, PD9, PD10, PD11, PD12, PD13, PD14, PD15;
extern const GP_Pin PE0, PE1, PE2, PE3, PE4, PE5, PE6, PE7, PE8, PE9, PE10, PE11, PE12, PE13, PE14, PE15;
extern const GP_Pin PF0, PF1, PF2, PF3, PF4, PF5, PF6, PF7, PF8, PF9, PF10, PF11, PF12, PF13, PF14, PF15;
extern const GP_Pin PH10, PH11, PH12, PH13, PH14, PH15;
extern const GP_Pin PI4, PI5, PI6, PI7;

#endif /* UNOQ_STM32_HAL_H */
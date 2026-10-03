/*
 * UNOQ_STM32_HAL.cpp - see UNOQ_STM32_HAL.h
 */
#include <string.h>
#include "UNOQ_STM32_HAL.h"

#if __has_include(<zephyr/drivers/adc.h>)
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#if DT_NODE_HAS_STATUS(DT_NODELABEL(adc1), okay)
#define UNOQ_HAVE_ADC1 1
#endif
#endif

/* =============================================================================
 * Interop with the standard Arduino API / other libraries
 * ===========================================================================*/
struct GP_PinMapEntry {
    GPIO_TypeDef* port;
    uint8_t pin;
    uint32_t arduinoPin;
};

static const GP_PinMapEntry GP_PinMap[] = {
    { GPIOB, 7,  0 },  { GPIOB, 6,  1 },  { GPIOB, 3,  2 },  { GPIOB, 0,  3 },
    { GPIOA, 12, 4 },  { GPIOA, 11, 5 },  { GPIOB, 1,  6 },  { GPIOB, 2,  7 },
    { GPIOB, 4,  8 },  { GPIOB, 8,  9 },  { GPIOB, 9,  10 }, { GPIOB, 15, 11 },
    { GPIOB, 14, 12 }, { GPIOB, 13, 13 },
    { GPIOA, 4,  14 }, { GPIOA, 5,  15 }, { GPIOA, 6,  16 }, { GPIOA, 7,  17 },
    { GPIOC, 1,  18 }, { GPIOC, 0,  19 },
    { GPIOB, 11, 20 }, { GPIOB, 10, 21 },
    { GPIOC, 2,  22 }, { GPIOC, 3,  23 }, { GPIOD, 1,  24 },
    { GPIOC, 6,  25 }, { GPIOD, 2,  26 }, { GPIOC, 7,  27 }, { GPIOE, 2,  28 },
    { GPIOC, 8,  29 }, { GPIOE, 3,  30 }, { GPIOC, 9,  31 }, { GPIOE, 5,  32 },
    { GPIOE, 4,  33 }, { GPIOE, 6,  34 }, { GPIOI, 4,  35 }, { GPIOE, 7,  36 },
    { GPIOI, 6,  37 }, { GPIOE, 8,  38 }, { GPIOI, 7,  39 }, { GPIOF, 14, 40 },
    { GPIOD, 9,  41 }, { GPIOF, 15, 42 }, { GPIOI, 5,  43 }, { GPIOA, 3,  44 },
    { GPIOD, 8,  45 }, { GPIOA, 0,  46 }, { GPIOA, 8,  47 }, { GPIOA, 1,  48 },
    { GPIOA, 10, 49 },
    { GPIOH, 10, 50 }, { GPIOH, 11, 51 }, { GPIOH, 12, 52 },
    { GPIOH, 13, 53 }, { GPIOH, 14, 54 }, { GPIOH, 15, 55 },
    { GPIOF, 0,  56 }, { GPIOF, 1,  57 }, { GPIOF, 2,  58 }, { GPIOF, 3,  59 },
    { GPIOF, 4,  60 }, { GPIOF, 5,  61 }, { GPIOF, 6,  62 }, { GPIOF, 7,  63 },
    { GPIOF, 8,  64 }, { GPIOF, 9,  65 }, { GPIOF, 10, 66 },
    { GPIOG, 13, 67 },
    { GPIOA, 2,  68 },
    { GPIOH, 3,  69 },
};

uint32_t GP_ToArduinoPin(GP_Pin pin)
{
    for (size_t i = 0; i < sizeof(GP_PinMap) / sizeof(GP_PinMap[0]); i++) {
        if (GP_PinMap[i].port == pin.port && GP_PinMap[i].pin == pin.pin) {
            return GP_PinMap[i].arduinoPin;
        }
    }
    return NC;
}

GP_Pin::operator uint32_t() const
{
    return GP_ToArduinoPin(*this);
}

/* =============================================================================
 * Pin table definitions
 * ===========================================================================*/
const GP_Pin PA0 = {GPIOA, 0};   const GP_Pin PA1 = {GPIOA, 1};
const GP_Pin PA2 = {GPIOA, 2};   const GP_Pin PA3 = {GPIOA, 3};
const GP_Pin PA4 = {GPIOA, 4};   const GP_Pin PA5 = {GPIOA, 5};
const GP_Pin PA6 = {GPIOA, 6};   const GP_Pin PA7 = {GPIOA, 7};
const GP_Pin PA8 = {GPIOA, 8};   const GP_Pin PA9 = {GPIOA, 9};
const GP_Pin PA10 = {GPIOA, 10}; const GP_Pin PA11 = {GPIOA, 11};
const GP_Pin PA12 = {GPIOA, 12}; const GP_Pin PA13 = {GPIOA, 13};
const GP_Pin PA14 = {GPIOA, 14}; const GP_Pin PA15 = {GPIOA, 15};

const GP_Pin PB0 = {GPIOB, 0};   const GP_Pin PB1 = {GPIOB, 1};
const GP_Pin PB2 = {GPIOB, 2};   const GP_Pin PB3 = {GPIOB, 3};
const GP_Pin PB4 = {GPIOB, 4};   const GP_Pin PB5 = {GPIOB, 5};
const GP_Pin PB6 = {GPIOB, 6};   const GP_Pin PB7 = {GPIOB, 7};
const GP_Pin PB8 = {GPIOB, 8};   const GP_Pin PB9 = {GPIOB, 9};
const GP_Pin PB10 = {GPIOB, 10}; const GP_Pin PB11 = {GPIOB, 11};
const GP_Pin PB12 = {GPIOB, 12}; const GP_Pin PB13 = {GPIOB, 13};
const GP_Pin PB14 = {GPIOB, 14}; const GP_Pin PB15 = {GPIOB, 15};

const GP_Pin PC0 = {GPIOC, 0};   const GP_Pin PC1 = {GPIOC, 1};
const GP_Pin PC2 = {GPIOC, 2};   const GP_Pin PC3 = {GPIOC, 3};
const GP_Pin PC4 = {GPIOC, 4};   const GP_Pin PC5 = {GPIOC, 5};
const GP_Pin PC6 = {GPIOC, 6};   const GP_Pin PC7 = {GPIOC, 7};
const GP_Pin PC8 = {GPIOC, 8};   const GP_Pin PC9 = {GPIOC, 9};
const GP_Pin PC10 = {GPIOC, 10}; const GP_Pin PC11 = {GPIOC, 11};
const GP_Pin PC12 = {GPIOC, 12}; const GP_Pin PC13 = {GPIOC, 13};
const GP_Pin PC14 = {GPIOC, 14}; const GP_Pin PC15 = {GPIOC, 15};

const GP_Pin PD0 = {GPIOD, 0};   const GP_Pin PD1 = {GPIOD, 1};
const GP_Pin PD2 = {GPIOD, 2};   const GP_Pin PD3 = {GPIOD, 3};
const GP_Pin PD4 = {GPIOD, 4};   const GP_Pin PD5 = {GPIOD, 5};
const GP_Pin PD6 = {GPIOD, 6};   const GP_Pin PD7 = {GPIOD, 7};
const GP_Pin PD8 = {GPIOD, 8};   const GP_Pin PD9 = {GPIOD, 9};
const GP_Pin PD10 = {GPIOD, 10}; const GP_Pin PD11 = {GPIOD, 11};
const GP_Pin PD12 = {GPIOD, 12}; const GP_Pin PD13 = {GPIOD, 13};
const GP_Pin PD14 = {GPIOD, 14}; const GP_Pin PD15 = {GPIOD, 15};

const GP_Pin PE0 = {GPIOE, 0};   const GP_Pin PE1 = {GPIOE, 1};
const GP_Pin PE2 = {GPIOE, 2};   const GP_Pin PE3 = {GPIOE, 3};
const GP_Pin PE4 = {GPIOE, 4};   const GP_Pin PE5 = {GPIOE, 5};
const GP_Pin PE6 = {GPIOE, 6};   const GP_Pin PE7 = {GPIOE, 7};
const GP_Pin PE8 = {GPIOE, 8};   const GP_Pin PE9 = {GPIOE, 9};
const GP_Pin PE10 = {GPIOE, 10}; const GP_Pin PE11 = {GPIOE, 11};
const GP_Pin PE12 = {GPIOE, 12}; const GP_Pin PE13 = {GPIOE, 13};
const GP_Pin PE14 = {GPIOE, 14}; const GP_Pin PE15 = {GPIOE, 15};

const GP_Pin PF0 = {GPIOF, 0};   const GP_Pin PF1 = {GPIOF, 1};
const GP_Pin PF2 = {GPIOF, 2};   const GP_Pin PF3 = {GPIOF, 3};
const GP_Pin PF4 = {GPIOF, 4};   const GP_Pin PF5 = {GPIOF, 5};
const GP_Pin PF6 = {GPIOF, 6};   const GP_Pin PF7 = {GPIOF, 7};
const GP_Pin PF8 = {GPIOF, 8};   const GP_Pin PF9 = {GPIOF, 9};
const GP_Pin PF10 = {GPIOF, 10}; const GP_Pin PF11 = {GPIOF, 11};
const GP_Pin PF12 = {GPIOF, 12}; const GP_Pin PF13 = {GPIOF, 13};
const GP_Pin PF14 = {GPIOF, 14}; const GP_Pin PF15 = {GPIOF, 15};

const GP_Pin PH10 = {GPIOH, 10}; const GP_Pin PH11 = {GPIOH, 11};
const GP_Pin PH12 = {GPIOH, 12}; const GP_Pin PH13 = {GPIOH, 13};
const GP_Pin PH14 = {GPIOH, 14}; const GP_Pin PH15 = {GPIOH, 15};

const GP_Pin PI4 = {GPIOI, 4};   const GP_Pin PI5 = {GPIOI, 5};
const GP_Pin PI6 = {GPIOI, 6};   const GP_Pin PI7 = {GPIOI, 7};

/* =============================================================================
 * Internal helpers
 * ===========================================================================*/
static inline bool is32bit(TIM_TypeDef* t) { return t == TIM2 || t == TIM5; }

static inline bool hasBDTR(TIM_TypeDef* t)
{
    if (t == TIM1 || t == TIM8) return true;
#ifdef TIM15
    if (t == TIM15) return true;
#endif
#ifdef TIM16
    if (t == TIM16) return true;
#endif
#ifdef TIM17
    if (t == TIM17) return true;
#endif
    return false;
}

static uint8_t maxChannels(TIM_TypeDef* t)
{
    if (t == TIM1 || t == TIM2 || t == TIM3 || t == TIM4 || t == TIM5 || t == TIM8) return 4;
#ifdef TIM15
    if (t == TIM15) return 2;
#endif
#ifdef TIM16
    if (t == TIM16) return 1;
#endif
#ifdef TIM17
    if (t == TIM17) return 1;
#endif
    return 0;
}

static bool timerClockEnable(TIM_TypeDef* t)
{
    if      (t == TIM1)  RCC->APB2ENR  |= RCC_APB2ENR_TIM1EN;
    else if (t == TIM8)  RCC->APB2ENR  |= RCC_APB2ENR_TIM8EN;
    else if (t == TIM2)  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    else if (t == TIM3)  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;
    else if (t == TIM4)  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN;
    else if (t == TIM5)  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM5EN;
#ifdef TIM15
    else if (t == TIM15) RCC->APB2ENR  |= RCC_APB2ENR_TIM15EN;
#endif
#ifdef TIM16
    else if (t == TIM16) RCC->APB2ENR  |= RCC_APB2ENR_TIM16EN;
#endif
#ifdef TIM17
    else if (t == TIM17) RCC->APB2ENR  |= RCC_APB2ENR_TIM17EN;
#endif
    else return false;
    (void)RCC->APB1ENR1;
    (void)RCC->APB2ENR;
    return true;
}

static bool calcTiming(TIM_TypeDef* t, uint32_t freq, uint32_t* psc, uint32_t* arr)
{
    if (freq == 0) return false;
    const uint64_t maxCount = is32bit(t) ? 0x100000000ULL : 0x10000ULL;
    uint64_t ticks = ((uint64_t)UNOQ_TIM_CLK_HZ + freq / 2) / freq;
    if (ticks < 2) return false;
    uint64_t p = (ticks - 1) / maxCount;
    if (p > 0xFFFF) return false;
    uint64_t a = ticks / (p + 1) - 1;
    *psc = (uint32_t)p;
    *arr = (uint32_t)a;
    return true;
}

static inline uint32_t maxCCR(TIM_TypeDef* t) { return is32bit(t) ? 0xFFFFFFFFUL : 0xFFFFUL; }

static void setCCR(TIM_TypeDef* t, uint8_t ch, uint32_t v)
{
    switch (ch) {
    case 1: t->CCR1 = v; break;
    case 2: t->CCR2 = v; break;
    case 3: t->CCR3 = v; break;
    case 4: t->CCR4 = v; break;
    }
}

static uint32_t getCCR(TIM_TypeDef* t, uint8_t ch)
{
    switch (ch) {
    case 1: return t->CCR1;
    case 2: return t->CCR2;
    case 3: return t->CCR3;
    case 4: return t->CCR4;
    }
    return 0;
}

static inline uint32_t ccerEnableBit(uint8_t ch, bool comp)
{
    return 1UL << ((ch - 1) * 4 + ((comp && ch <= 3) ? 2 : 0));
}

static inline uint32_t ccerPolBit(uint8_t ch, bool comp)
{
    return 1UL << ((ch - 1) * 4 + ((comp && ch <= 3) ? 3 : 1));
}

static uint32_t dutyToCCR(TIM_TypeDef* t, uint32_t permille)
{
    if (permille > 1000) permille = 1000;
    uint64_t v = ((uint64_t)t->ARR + 1) * permille / 1000ULL;
    uint64_t m = maxCCR(t);
    return (uint32_t)(v > m ? m : v);
}

static void ocConfigPWM(TIM_TypeDef* t, uint8_t ch)
{
    switch (ch) {
    case 1:
        t->CCMR1 = (t->CCMR1 & ~(TIM_CCMR1_CC1S | TIM_CCMR1_OC1M)) | (6UL << 4)  | TIM_CCMR1_OC1PE;
        break;
    case 2:
        t->CCMR1 = (t->CCMR1 & ~(TIM_CCMR1_CC2S | TIM_CCMR1_OC2M)) | (6UL << 12) | TIM_CCMR1_OC2PE;
        break;
    case 3:
        t->CCMR2 = (t->CCMR2 & ~(TIM_CCMR2_CC3S | TIM_CCMR2_OC3M)) | (6UL << 4)  | TIM_CCMR2_OC3PE;
        break;
    case 4:
        t->CCMR2 = (t->CCMR2 & ~(TIM_CCMR2_CC4S | TIM_CCMR2_OC4M)) | (6UL << 12) | TIM_CCMR2_OC4PE;
        break;
    }
}

static bool channelConfig(TIM_TypeDef* t, uint8_t ch, bool comp, uint32_t permille)
{
    if (ch < 1 || ch > maxChannels(t)) return false;
    ocConfigPWM(t, ch);
    setCCR(t, ch, dutyToCCR(t, permille));
    t->CCER |= ccerEnableBit(ch, comp);
    if (hasBDTR(t)) t->BDTR |= TIM_BDTR_MOE;
    return true;
}

/* =============================================================================
 * GPIO
 * ===========================================================================*/
void enableGPIOClock(GPIO_TypeDef* port)
{
    if      (port == GPIOA) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOAEN;
    else if (port == GPIOB) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOBEN;
    else if (port == GPIOC) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOCEN;
    else if (port == GPIOD) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIODEN;
    else if (port == GPIOE) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOEEN;
    else if (port == GPIOF) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOFEN;
#ifdef GPIOG
    else if (port == GPIOG) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOGEN;
#endif
    else if (port == GPIOH) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOHEN;
#ifdef GPIOI
    else if (port == GPIOI) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOIEN;
#endif
    else return;
    (void)RCC->AHB2ENR1;
}

void SetPinAF(GPIO_TypeDef* port, uint8_t pin, uint8_t af)
{
    pin &= 0x0F;
    enableGPIOClock(port);

    const uint32_t idx   = pin >> 3;
    const uint32_t shift = (pin & 7U) * 4U;
    port->AFR[idx] = (port->AFR[idx] & ~(0xFUL << shift)) | ((uint32_t)(af & 0x0F) << shift);
    port->MODER = (port->MODER & ~(3UL << (pin * 2U))) | (2UL << (pin * 2U));
}

void pinMode_STM(GP_Pin p, uint8_t mode, uint8_t pull)
{
    const uint32_t pin = p.pin & 0x0F;
    enableGPIOClock(p.port);

    uint32_t moder;
    switch (mode) {
    case OUTPUT_STM:
    case OUTPUT_OD_STM: moder = 1UL; break;
    case ANALOG_STM:    moder = 3UL; break;
    default:            moder = 0UL; break;
    }

    uint32_t pupd = (pull == PULLUP_STM) ? 1UL : (pull == PULLDOWN_STM) ? 2UL : 0UL;
    if (mode == ANALOG_STM) pupd = 0UL;
    p.port->PUPDR = (p.port->PUPDR & ~(3UL << (pin * 2U))) | (pupd << (pin * 2U));

    if (mode == OUTPUT_STM)         p.port->OTYPER &= ~(1UL << pin);
    else if (mode == OUTPUT_OD_STM) p.port->OTYPER |=  (1UL << pin);

    if (moder == 1UL)
        p.port->OSPEEDR = (p.port->OSPEEDR & ~(3UL << (pin * 2U))) | (2UL << (pin * 2U));

    p.port->MODER = (p.port->MODER & ~(3UL << (pin * 2U))) | (moder << (pin * 2U));
}

uint8_t digitalRead_STM(GP_Pin p)
{
    return (p.port->IDR & (1UL << (p.pin & 0x0F))) ? 1 : 0;
}

void digitalWrite_STM(GP_Pin p, uint8_t state)
{
    const uint32_t pin = p.pin & 0x0F;
    p.port->BSRR = state ? (1UL << pin) : (1UL << (pin + 16U));
}

void togglePin_STM(GP_Pin p)
{
    const uint32_t m = 1UL << (p.pin & 0x0F);
    p.port->BSRR = (p.port->ODR & m) ? (m << 16U) : m;
}

/* =============================================================================
 * PWM output
 * ===========================================================================*/
static void pwmPinPrepare(GPIO_TypeDef* port, uint8_t pin, uint8_t af)
{
    pin &= 0x0F;
    enableGPIOClock(port);
    port->OTYPER &= ~(1UL << pin);
    port->OSPEEDR = (port->OSPEEDR & ~(3UL << (pin * 2U))) | (2UL << (pin * 2U));
    port->PUPDR  &= ~(3UL << (pin * 2U));
    SetPinAF(port, pin, af);
}

bool PWM_Setup(GPIO_TypeDef* port, uint8_t pin, TIM_TypeDef* timer, uint8_t channel,
               bool complementary, uint32_t freq, uint8_t dutyPercent, uint8_t af)
{
    if (!timer || channel < 1 || channel > maxChannels(timer)) return false;
    if (!timerClockEnable(timer)) return false;

    const bool running = (timer->CR1 & TIM_CR1_CEN) != 0;

    if (!running) {
        uint32_t psc, arr;
        if (!calcTiming(timer, freq, &psc, &arr)) return false;
        timer->CR1 |= TIM_CR1_ARPE;
        timer->PSC = psc;
        timer->ARR = arr;
    }

    uint32_t permille = (uint32_t)dutyPercent * 10U;
    if (!channelConfig(timer, channel, complementary, permille)) return false;

    if (!running) {
        timer->EGR = TIM_EGR_UG;
        timer->CR1 |= TIM_CR1_CEN;
    }

    pwmPinPrepare(port, pin, af);
    return true;
}

bool PWM_Setup_STM(GP_Pin pin, TIM_TypeDef* timer, uint8_t channel,
                   bool complementary, uint32_t freq, uint8_t dutyPercent, uint8_t af)
{
    return PWM_Setup(pin.port, pin.pin, timer, channel, complementary, freq, dutyPercent, af);
}

bool PWM_AddChannel(TIM_TypeDef* timer, uint8_t channel, bool complementary, uint8_t dutyPercent)
{
    if (!timer) return false;
    return channelConfig(timer, channel, complementary, (uint32_t)dutyPercent * 10U);
}

bool PWM_AddChannel_STM(GP_Pin pin, TIM_TypeDef* timer, uint8_t channel,
                       bool complementary, uint8_t dutyPercent, uint8_t af)
{
    if (!PWM_AddChannel(timer, channel, complementary, dutyPercent)) return false;
    pwmPinPrepare(pin.port, pin.pin, af);
    return true;
}

bool PWM_SetDutyPermille(TIM_TypeDef* timer, uint8_t channel, uint16_t permille)
{
    if (!timer || channel < 1 || channel > maxChannels(timer)) return false;
    setCCR(timer, channel, dutyToCCR(timer, permille));
    return true;
}

bool PWM_SetDuty(TIM_TypeDef* timer, uint8_t channel, bool complementary, uint8_t dutyPercent)
{
    (void)complementary;
    if (dutyPercent > 100) dutyPercent = 100;
    return PWM_SetDutyPermille(timer, channel, (uint16_t)dutyPercent * 10U);
}

bool PWM_SetDutyRaw(TIM_TypeDef* timer, uint8_t channel, uint32_t ticks)
{
    if (!timer || channel < 1 || channel > maxChannels(timer)) return false;
    if (ticks > maxCCR(timer)) ticks = maxCCR(timer);
    setCCR(timer, channel, ticks);
    return true;
}

bool PWM_SetFrequency(TIM_TypeDef* timer, uint8_t channel, bool complementary, uint32_t freq)
{
    (void)channel; (void)complementary;
    if (!timer || maxChannels(timer) == 0) return false;

    uint32_t psc, arr;
    if (!calcTiming(timer, freq, &psc, &arr)) return false;

    const uint64_t oldPeriod = (uint64_t)timer->ARR + 1;
    const uint64_t newPeriod = (uint64_t)arr + 1;
    const uint32_t ccrMax    = maxCCR(timer);

    for (uint8_t ch = 1; ch <= maxChannels(timer); ch++) {
        if (timer->CCER & (ccerEnableBit(ch, false) | ccerEnableBit(ch, true))) {
            uint64_t v = (uint64_t)getCCR(timer, ch) * newPeriod / oldPeriod;
            setCCR(timer, ch, (uint32_t)(v > ccrMax ? ccrMax : v));
        }
    }

    timer->CR1 |= TIM_CR1_ARPE;
    timer->PSC = psc;
    timer->ARR = arr;
    if (!(timer->CR1 & TIM_CR1_CEN)) timer->EGR = TIM_EGR_UG;
    return true;
}

bool PWM_Enable(TIM_TypeDef* timer, uint8_t channel, bool complementary, bool enable)
{
    if (!timer || channel < 1 || channel > maxChannels(timer)) return false;
    if (enable) timer->CCER |=  ccerEnableBit(channel, complementary);
    else        timer->CCER &= ~ccerEnableBit(channel, complementary);
    return true;
}

bool PWM_Stop(TIM_TypeDef* timer, uint8_t channel, bool complementary)
{
    return PWM_Enable(timer, channel, complementary, false);
}

bool PWM_SetPolarity(TIM_TypeDef* timer, uint8_t channel, bool complementary, bool activeHigh)
{
    if (!timer || channel < 1 || channel > maxChannels(timer)) return false;
    if (activeHigh) timer->CCER &= ~ccerPolBit(channel, complementary);
    else            timer->CCER |=  ccerPolBit(channel, complementary);
    return true;
}

static uint8_t dtgEncode(uint32_t t)
{
    if (t < 128)   return (uint8_t)t;
    if (t < 256)   return (uint8_t)(0x80 | ((t / 2)  - 64));
    if (t < 512)   return (uint8_t)(0xC0 | ((t / 8)  - 32));
    if (t < 1008)  return (uint8_t)(0xE0 | ((t / 16) - 32));
    return 0xFF;
}

bool PWM_SetDeadTime(TIM_TypeDef* timer, uint32_t dead_ns, uint32_t timer_clk)
{
    if (!timer || !hasBDTR(timer)) return false;
    if (timer_clk == 0) timer_clk = UNOQ_TIM_CLK_HZ;

    uint64_t ticks = ((uint64_t)dead_ns * timer_clk) / 1000000000ULL;
    if (ticks > 1008) ticks = 1008;

    timer->BDTR = (timer->BDTR & ~TIM_BDTR_DTG) | dtgEncode((uint32_t)ticks) | TIM_BDTR_MOE;
    return true;
}

/* =============================================================================
 * PWM input capture
 * ===========================================================================*/
typedef struct {
    uint32_t period;
    uint32_t high;
    uint32_t lastMs;
    bool     valid;
    bool     discardFirst;
} CaptureState;

static CaptureState  capState[7];
static uint32_t      capTimeoutMs = 1000;

static int capIndex(TIM_TypeDef* t)
{
    if (t == TIM1) return 0;
    if (t == TIM2) return 1;
    if (t == TIM3) return 2;
    if (t == TIM4) return 3;
    if (t == TIM5) return 4;
    if (t == TIM8) return 5;
#ifdef TIM15
    if (t == TIM15) return 6;
#endif
    return -1;
}

static inline bool capOnCH1(TIM_TypeDef* t) { return ((t->SMCR >> 4) & 7U) == 5U; }

static void capPoll(TIM_TypeDef* t, int idx)
{
    const bool ch1 = capOnCH1(t);
    const uint32_t periodFlag = ch1 ? TIM_SR_CC1IF : TIM_SR_CC2IF;

    if (!(t->SR & periodFlag)) return;

    uint32_t period, high;
    if (ch1) { period = t->CCR1; high = t->CCR2; }
    else     { period = t->CCR2; high = t->CCR1; }
    t->SR = (uint32_t)~(TIM_SR_CC1IF | TIM_SR_CC2IF | TIM_SR_CC1OF | TIM_SR_CC2OF);

    CaptureState* s = &capState[idx];
    if (s->discardFirst) { s->discardFirst = false; return; }
    s->period = period;
    s->high   = high;
    s->lastMs = millis();
    s->valid  = true;
}

static bool capActive(TIM_TypeDef* t, int idx)
{
    capPoll(t, idx);
    const CaptureState* s = &capState[idx];
    return s->valid && s->period != 0 && (uint32_t)(millis() - s->lastMs) <= capTimeoutMs;
}

bool PWM_Capture_Init_Ex(GP_Pin pin, TIM_TypeDef* timer, uint8_t af,
                         uint8_t inputChannel, uint16_t prescaler)
{
    const int idx = capIndex(timer);
    if (idx < 0 || (inputChannel != 1 && inputChannel != 2)) return false;
    if (!timerClockEnable(timer)) return false;

    SetPinAF(pin.port, pin.pin, af);

    timer->CR1  &= ~TIM_CR1_CEN;
    timer->SMCR  = 0;
    timer->PSC   = prescaler;
    timer->ARR   = is32bit(timer) ? 0xFFFFFFFFUL : 0xFFFFUL;

    if (inputChannel == 1) {
        timer->CCMR1 = (1UL << 0) | (2UL << 8);
        timer->CCER  = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC2P;
        timer->SMCR  = (5UL << 4);
    } else {
        timer->CCMR1 = (2UL << 0) | (1UL << 8);
        timer->CCER  = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC1P;
        timer->SMCR  = (6UL << 4);
    }
    timer->SMCR |= 4UL;

    timer->EGR = TIM_EGR_UG;
    timer->SR  = 0;

    memset(&capState[idx], 0, sizeof(capState[idx]));
    capState[idx].discardFirst = true;

    timer->CR1 |= TIM_CR1_CEN;
    return true;
}

bool PWM_Capture_Init(GP_Pin pin, TIM_TypeDef* timer, uint8_t af, uint8_t inputChannel)
{
    return PWM_Capture_Init_Ex(pin, timer, af, inputChannel, is32bit(timer) ? 0 : 15);
}

bool PWM_Capture_Init_CH1(GP_Pin pin, TIM_TypeDef* timer, uint8_t af)
{
    return PWM_Capture_Init(pin, timer, af, 1);
}

bool PWM_Capture_Init_CH2(GP_Pin pin, TIM_TypeDef* timer, uint8_t af)
{
    return PWM_Capture_Init(pin, timer, af, 2);
}

void PWM_Capture_SetTimeout(uint32_t ms) { capTimeoutMs = ms; }

bool PWM_Capture_IsActive(TIM_TypeDef* timer)
{
    const int idx = capIndex(timer);
    return idx >= 0 && capActive(timer, idx);
}

uint32_t PWM_Capture_GetPeriodTicks(TIM_TypeDef* timer)
{
    const int idx = capIndex(timer);
    return (idx >= 0 && capActive(timer, idx)) ? capState[idx].period : 0;
}

uint32_t PWM_Capture_GetHighTicks(TIM_TypeDef* timer)
{
    const int idx = capIndex(timer);
    return (idx >= 0 && capActive(timer, idx)) ? capState[idx].high : 0;
}

float PWM_Capture_GetFrequency(TIM_TypeDef* timer)
{
    const uint32_t p = PWM_Capture_GetPeriodTicks(timer);
    if (!p) return 0.0f;
    return (float)UNOQ_TIM_CLK_HZ / ((float)(timer->PSC + 1U) * (float)p);
}

float PWM_Capture_GetDuty(TIM_TypeDef* timer)
{
    const uint32_t p = PWM_Capture_GetPeriodTicks(timer);
    if (!p) return 0.0f;
    uint32_t h = capState[capIndex(timer)].high;
    if (h > p) h = p;
    return (float)h * 100.0f / (float)p;
}

/* =============================================================================
 * ADC Helpers (STM32U585)
 * ===========================================================================*/
struct StmAdcPin {
    const char *name;
    uint8_t     channel;
};

static const StmAdcPin STM_ADC_PINS[] = {
    {"PC0", 1},  {"PC1", 2},  {"PC2", 3},  {"PC3", 4},
    {"PA0", 5},  {"PA1", 6},  {"PA2", 7},  {"PA3", 8},
    {"PA4", 9},  {"PA5", 10}, {"PA6", 11}, {"PA7", 12},
    {"PC4", 13}, {"PC5", 14}, {"PB0", 15}, {"PB1", 16},
};

static int stm_find_channel_by_name(const char *name) {
    for (const StmAdcPin &p : STM_ADC_PINS) {
        if (strcasecmp(name, p.name) == 0) return p.channel;
    }
    return -1;
}

static int stm_find_channel_by_pin(GP_Pin pin) {
    if (pin.port == GPIOC) {
        if (pin.pin == 0) return 1;
        if (pin.pin == 1) return 2;
        if (pin.pin == 2) return 3;
        if (pin.pin == 3) return 4;
        if (pin.pin == 4) return 13;
        if (pin.pin == 5) return 14;
    } else if (pin.port == GPIOA) {
        if (pin.pin == 0) return 5;
        if (pin.pin == 1) return 6;
        if (pin.pin == 2) return 7;
        if (pin.pin == 3) return 8;
        if (pin.pin == 4) return 9;
        if (pin.pin == 5) return 10;
        if (pin.pin == 6) return 11;
        if (pin.pin == 7) return 12;
    } else if (pin.port == GPIOB) {
        if (pin.pin == 0) return 15;
        if (pin.pin == 1) return 16;
    }
    return -1;
}

static int perform_adc_read(int ch) {
#ifdef UNOQ_HAVE_ADC1
    if (ch < 0 || ch > 31) return -1;

    const struct device *const stm_adc = DEVICE_DT_GET(DT_NODELABEL(adc1));
    static bool stm_adc_ready = false;
    static uint32_t stm_configured_ch = 0;

    if (!stm_adc_ready) {
        analogRead(A0); // makes the Arduino core initialize ADC1
        if (!device_is_ready(stm_adc)) return -2;
        stm_adc_ready = true;
    }

    if (!(stm_configured_ch & BIT(ch))) {
        struct adc_channel_cfg cfg = {};
        cfg.gain             = ADC_GAIN_1;
        cfg.reference        = ADC_REF_INTERNAL;
        cfg.acquisition_time = ADC_ACQ_TIME_DEFAULT;
        cfg.channel_id       = ch;
        if (adc_channel_setup(stm_adc, &cfg) != 0) return -3;
        stm_configured_ch |= BIT(ch);
    }

    int16_t buf = 0;
    struct adc_sequence seq = {};
    seq.channels    = BIT(ch);
    seq.buffer      = &buf;
    seq.buffer_size = sizeof(buf);
    seq.resolution  = 12; // 12-bit resolution
    if (adc_read(stm_adc, &seq) != 0) return -4;

    return buf;
#else
    (void)ch;
    return -1;
#endif
}

int analogRead_STM(const char *pinName) {
    int ch = stm_find_channel_by_name(pinName);
    return perform_adc_read(ch);
}

int analogReadMilliVolts_STM(const char *pinName) {
    int raw = analogRead_STM(pinName);
    if (raw < 0) return raw;
    return (raw * 3300) / 4095;
}

int analogRead_Pin_STM(GP_Pin pin) {
    int ch = stm_find_channel_by_pin(pin);
    return perform_adc_read(ch);
}

int analogReadMilliVolts_Pin_STM(GP_Pin pin) {
    int raw = analogRead_Pin_STM(pin);
    if (raw < 0) return raw;
    return (raw * 3300) / 4095;
}
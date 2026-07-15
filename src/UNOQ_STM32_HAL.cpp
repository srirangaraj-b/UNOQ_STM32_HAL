#include "UNOQ_STM32_HAL.h"

/* =============================================================================
 * Internal helpers
 * ===========================================================================*/

static inline bool is_advanced_timer(TIM_TypeDef *timer)
{
    /* Only TIM1/TIM8 have CHxN complementary outputs + BDTR dead-time/break unit. */
    return (timer == TIM1)
#ifdef TIM8
        || (timer == TIM8)
#endif
        ;
}

static volatile uint32_t *get_ccr(TIM_TypeDef *timer, uint8_t channel)
{
    switch (channel) {
        case 1: return &timer->CCR1;
        case 2: return &timer->CCR2;
        case 3: return &timer->CCR3;
        case 4: return &timer->CCR4;
        default: return NULL;
    }
}

static void set_ocm_pwm1(TIM_TypeDef *timer, uint8_t channel)
{
    /* Mode 110 (PWM mode 1) + preload enable, on whichever CCMR the channel lives in. */
    switch (channel) {
        case 1:
            timer->CCMR1 = (timer->CCMR1 & ~TIM_CCMR1_OC1M) | (6UL << TIM_CCMR1_OC1M_Pos);
            timer->CCMR1 |= TIM_CCMR1_OC1PE;
            break;
        case 2:
            timer->CCMR1 = (timer->CCMR1 & ~TIM_CCMR1_OC2M) | (6UL << TIM_CCMR1_OC2M_Pos);
            timer->CCMR1 |= TIM_CCMR1_OC2PE;
            break;
        case 3:
            timer->CCMR2 = (timer->CCMR2 & ~TIM_CCMR2_OC3M) | (6UL << TIM_CCMR2_OC3M_Pos);
            timer->CCMR2 |= TIM_CCMR2_OC3PE;
            break;
        case 4:
            timer->CCMR2 = (timer->CCMR2 & ~TIM_CCMR2_OC4M) | (6UL << TIM_CCMR2_OC4M_Pos);
            timer->CCMR2 |= TIM_CCMR2_OC4PE;
            break;
        default:
            break;
    }
}

static void set_ccer_enable(TIM_TypeDef *timer, uint8_t channel, bool complementary, bool advanced)
{
    switch (channel) {
        case 1:
            timer->CCER |= TIM_CCER_CC1E;
            if (complementary && advanced) timer->CCER |= TIM_CCER_CC1NE;
            break;
        case 2:
            timer->CCER |= TIM_CCER_CC2E;
            if (complementary && advanced) timer->CCER |= TIM_CCER_CC2NE;
            break;
        case 3:
            timer->CCER |= TIM_CCER_CC3E;
            if (complementary && advanced) timer->CCER |= TIM_CCER_CC3NE;
            break;
        case 4:
            timer->CCER |= TIM_CCER_CC4E;
            if (complementary && advanced) timer->CCER |= TIM_CCER_CC4NE;
            break;
        default:
            break;
    }
}

static void compute_psc_arr(uint32_t freq_hz, uint32_t *psc_out, uint32_t *arr_out)
{
    if (freq_hz == 0) freq_hz = 1;
    uint32_t psc = 0;
    uint32_t ticks = UNOQ_TIM_CLK_HZ / ((psc + 1) * freq_hz);
    while (ticks > 65536UL && psc < 65535UL) {
        psc++;
        ticks = UNOQ_TIM_CLK_HZ / ((psc + 1) * freq_hz);
    }
    if (ticks == 0) ticks = 1;
    *psc_out = psc;
    *arr_out = ticks - 1;
}

static void enableTimerClock(TIM_TypeDef *timer)
{
    /* Bit names below follow the standard STM32 CMSIS naming pattern; double-check
     * against your installed STM32U5 CMSIS pack / RM0456 if a timer here doesn't
     * enable (some U5 lines don't populate every TIMx). */
    if (timer == TIM1) { RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; return; }
#ifdef TIM2
    if (timer == TIM2) { RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN; return; }
#endif
#ifdef TIM3
    if (timer == TIM3) { RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN; return; }
#endif
#ifdef TIM4
    if (timer == TIM4) { RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN; return; }
#endif
#ifdef TIM5
    if (timer == TIM5) { RCC->APB1ENR1 |= RCC_APB1ENR1_TIM5EN; return; }
#endif
#ifdef TIM6
    if (timer == TIM6) { RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN; return; }
#endif
#ifdef TIM7
    if (timer == TIM7) { RCC->APB1ENR1 |= RCC_APB1ENR1_TIM7EN; return; }
#endif
#ifdef TIM8
    if (timer == TIM8) { RCC->APB2ENR |= RCC_APB2ENR_TIM8EN; return; }
#endif
#ifdef TIM15
    if (timer == TIM15) { RCC->APB2ENR |= RCC_APB2ENR_TIM15EN; return; }
#endif
#ifdef TIM16
    if (timer == TIM16) { RCC->APB2ENR |= RCC_APB2ENR_TIM16EN; return; }
#endif
#ifdef TIM17
    if (timer == TIM17) { RCC->APB2ENR |= RCC_APB2ENR_TIM17EN; return; }
#endif
}

/* =============================================================================
 * Interop with the standard Arduino API / other libraries (LiquidCrystal, etc.)
 *
 * The UNO Q's Arduino core (ArduinoCore-zephyr) has no PinName/GPIOPort[]/
 * pinNametoDigitalPin() - that's stm32duino-only. Instead, this board's
 * variant overlay exposes a `digital-pin-gpios` devicetree property whose
 * list ORDER is the Arduino digital pin number (index 0 = D0, index 1 = D1,
 * ...). That's not queryable at runtime, so it's mirrored here as a static
 * table, built directly from this board's overlay. Pins not present in that
 * property (PD12/PD13 - reserved for I2C4/Wire on the QWIIC connector) have
 * no Arduino pin number and correctly resolve to NC.
 *
 * NOTE: PA8 does get a valid entry here, but this board's overlay drives it
 * as MCO1 clock output at boot (&mco1, no deferred-init) - repurposing it
 * for PWM/GPIO will fight that. Prefer a different pin for new PWM use.
 * ===========================================================================*/

struct GP_PinMapEntry {
    GPIO_TypeDef* port;
    uint8_t pin;
    uint32_t arduinoPin;
};

/* Mirrors this board's `zephyr,user { digital-pin-gpios = ... }` list, in order. */
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
    { GPIOG, 13, 67 }, /* internal SPI RDY */
    { GPIOA, 2,  68 }, /* analog switch for VREF */
    { GPIOH, 3,  69 }, /* BOOT0 */
    /* PD12/PD13 intentionally absent - reserved for I2C4 (Wire/QWIIC) */
};

uint32_t GP_ToArduinoPin(GP_Pin pin)
{
    for (size_t i = 0; i < sizeof(GP_PinMap) / sizeof(GP_PinMap[0]); i++) {
        if (GP_PinMap[i].port == pin.port && GP_PinMap[i].pin == pin.pin) {
            return GP_PinMap[i].arduinoPin;
        }
    }
    return NC; /* not exposed as an Arduino digital pin on this board */
}

GP_Pin::operator uint32_t() const
{
    return GP_ToArduinoPin(*this);
}

/* =============================================================================
 * GPIO
 * ===========================================================================*/

void enableGPIOClock(GPIO_TypeDef *port)
{
    if (port == GPIOA) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOAEN;
    else if (port == GPIOB) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOBEN;
    else if (port == GPIOC) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOCEN;
#ifdef GPIOD
    else if (port == GPIOD) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIODEN;
#endif
#ifdef GPIOE
    else if (port == GPIOE) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOEEN;
#endif
#ifdef GPIOF
    else if (port == GPIOF) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOFEN;
#endif
#ifdef GPIOG
    else if (port == GPIOG) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOGEN;
#endif
#ifdef GPIOH
    else if (port == GPIOH) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOHEN;
#endif
#ifdef GPIOI
    else if (port == GPIOI) RCC->AHB2ENR1 |= RCC_AHB2ENR1_GPIOIEN;
#endif
}

void SetPinAF(GPIO_TypeDef *port, uint8_t pin, uint8_t af)
{
    enableGPIOClock(port);
    port->MODER   = (port->MODER & ~(3UL << (pin * 2))) | (2UL << (pin * 2));
    port->OSPEEDR |= (3UL << (pin * 2));
    port->OTYPER  &= ~(1UL << pin);
    port->PUPDR   &= ~(3UL << (pin * 2));

    if (pin < 8) {
        port->AFR[0] = (port->AFR[0] & ~(0xFUL << (pin * 4))) | ((uint32_t)af << (pin * 4));
    } else {
        port->AFR[1] = (port->AFR[1] & ~(0xFUL << ((pin - 8) * 4))) | ((uint32_t)af << ((pin - 8) * 4));
    }
}

void pinMode_STM(GP_Pin pin, uint8_t mode, uint8_t pull)
{
    enableGPIOClock(pin.port);
    GPIO_TypeDef *port = pin.port;
    uint8_t p = pin.pin;

    uint32_t modeVal = (mode == OUTPUT_STM) ? 1UL : (mode == ANALOG_STM) ? 3UL : 0UL;
    port->MODER = (port->MODER & ~(3UL << (p * 2))) | (modeVal << (p * 2));
    port->PUPDR = (port->PUPDR & ~(3UL << (p * 2))) | ((pull == PULLUP_STM ? 1UL : 0UL) << (p * 2));

    if (mode == OUTPUT_STM) {
        port->OTYPER &= ~(1UL << p);
        port->OSPEEDR |= (3UL << (p * 2));
    }
}

void digitalWrite_STM(GP_Pin pin, uint8_t state)
{
    if (state) pin.port->BSRR = (1UL << pin.pin);
    else       pin.port->BSRR = (1UL << (pin.pin + 16));
}

uint8_t digitalRead_STM(GP_Pin pin)
{
    return (pin.port->IDR & (1UL << pin.pin)) ? 1 : 0;
}

void togglePin_STM(GP_Pin pin)
{
    pin.port->ODR ^= (1UL << pin.pin);
}

/* =============================================================================
 * ADC1 - single channel, single conversion, polled
 * NOTE: ADC1 clock-enable bit / calibration sequence below follows the common
 * STM32U5 ADC12 pattern; verify RCC_AHB2ENR2_ADC12EN and the ADC_CR bit names
 * against your CMSIS pack (RM0456 ch. "ADC") before relying on exact timing.
 * ===========================================================================*/

int adc1Read_STM(GP_Pin pin, uint8_t adc_ch)
{
    static bool adc_ready = false;

    pinMode_STM(pin, ANALOG_STM, NOPULL_STM);

    if (!adc_ready) {
#ifdef RCC_AHB2ENR2_ADC12EN
        RCC->AHB2ENR2 |= RCC_AHB2ENR2_ADC12EN;
#endif
        ADC1->CR &= ~ADC_CR_DEEPPWD;
        ADC1->CR |= ADC_CR_ADVREGEN;
        delayMicroseconds(20); /* regulator startup */

        ADC1->CR |= ADC_CR_ADCAL;
        while (ADC1->CR & ADC_CR_ADCAL) { /* wait for calibration */ }

        ADC1->ISR |= ADC_ISR_ADRDY;
        ADC1->CR  |= ADC_CR_ADEN;
        while (!(ADC1->ISR & ADC_ISR_ADRDY)) { /* wait ready */ }

        adc_ready = true;
    }

    ADC1->SQR1 = ((uint32_t)adc_ch << ADC_SQR1_SQ1_Pos);
    ADC1->SMPR1 = (7UL << (adc_ch * 3)); /* longest sample time for this channel - safe default */

    ADC1->CR |= ADC_CR_ADSTART;
    while (!(ADC1->ISR & ADC_ISR_EOC)) { /* wait for end of conversion */ }

    return (int)ADC1->DR;
}

/* =============================================================================
 * PWM output
 * ===========================================================================*/

void PWM_Setup(GPIO_TypeDef *port, uint8_t pin, TIM_TypeDef *timer, uint8_t channel,
               bool complementary, uint32_t freq, uint8_t dutyPercent, uint8_t af)
{
    enableGPIOClock(port);
    enableTimerClock(timer);
    SetPinAF(port, pin, af);

    uint32_t psc, arr;
    compute_psc_arr(freq, &psc, &arr);
    timer->PSC = psc;
    timer->ARR = arr;

    set_ocm_pwm1(timer, channel);

    volatile uint32_t *ccr = get_ccr(timer, channel);
    if (ccr) *ccr = (uint32_t)(((uint64_t)(arr + 1) * dutyPercent) / 100);

    bool advanced = is_advanced_timer(timer);
    set_ccer_enable(timer, channel, complementary, advanced);

    if (advanced) {
        timer->BDTR |= TIM_BDTR_MOE;
    }

    timer->CR1 |= TIM_CR1_ARPE;
    timer->EGR |= TIM_EGR_UG;
    timer->CR1 |= TIM_CR1_CEN;
}

void PWM_Setup_STM(GP_Pin pin, TIM_TypeDef *timer, uint8_t channel,
                    bool complementary, uint32_t freq, uint8_t dutyPercent, uint8_t af)
{
    PWM_Setup(pin.port, pin.pin, timer, channel, complementary, freq, dutyPercent, af);
}

void PWM_AddChannel(TIM_TypeDef *timer, uint8_t channel, bool complementary, uint8_t dutyPercent)
{
    /* Assumes the timer already has PSC/ARR configured by a prior PWM_Setup() call
     * on this same timer, and the GPIO/AF for this channel is already set up. */
    set_ocm_pwm1(timer, channel);

    uint32_t arr = timer->ARR;
    volatile uint32_t *ccr = get_ccr(timer, channel);
    if (ccr) *ccr = (uint32_t)(((uint64_t)(arr + 1) * dutyPercent) / 100);

    bool advanced = is_advanced_timer(timer);
    set_ccer_enable(timer, channel, complementary, advanced);
    if (advanced) timer->BDTR |= TIM_BDTR_MOE;

    timer->CR1 |= TIM_CR1_ARPE;
    timer->EGR |= TIM_EGR_UG;
    timer->CR1 |= TIM_CR1_CEN;
}

void PWM_AddChannel_STM(GP_Pin pin, TIM_TypeDef *timer, uint8_t channel,
                         bool complementary, uint8_t dutyPercent, uint8_t af)
{
    enableGPIOClock(pin.port);
    SetPinAF(pin.port, pin.pin, af);
    PWM_AddChannel(timer, channel, complementary, dutyPercent);
}

void PWM_SetDuty(TIM_TypeDef *timer, uint8_t channel, bool complementary, uint8_t dutyPercent)
{
    (void)complementary; /* enable state doesn't change on a duty update */
    uint32_t arr = timer->ARR;
    volatile uint32_t *ccr = get_ccr(timer, channel);
    if (ccr) *ccr = (uint32_t)(((uint64_t)(arr + 1) * dutyPercent) / 100);
}

void PWM_SetFrequency(TIM_TypeDef *timer, uint8_t channel, bool complementary, uint32_t freq)
{
    (void)channel;
    (void)complementary;

    uint32_t old_arr = timer->ARR;
    uint32_t old_ccr[4] = { timer->CCR1, timer->CCR2, timer->CCR3, timer->CCR4 };

    uint32_t psc, arr;
    compute_psc_arr(freq, &psc, &arr);
    timer->PSC = psc;
    timer->ARR = arr;

    /* Rescale every channel's CCR proportionally so existing duty ratios hold. */
    if (old_arr > 0) {
        timer->CCR1 = (uint32_t)(((uint64_t)old_ccr[0] * (arr + 1)) / (old_arr + 1));
        timer->CCR2 = (uint32_t)(((uint64_t)old_ccr[1] * (arr + 1)) / (old_arr + 1));
        timer->CCR3 = (uint32_t)(((uint64_t)old_ccr[2] * (arr + 1)) / (old_arr + 1));
        timer->CCR4 = (uint32_t)(((uint64_t)old_ccr[3] * (arr + 1)) / (old_arr + 1));
    }

    timer->EGR |= TIM_EGR_UG;
}

void PWM_SetDeadTime(TIM_TypeDef *TIMx, uint32_t dead_ns, uint32_t timer_clk)
{
    if (!is_advanced_timer(TIMx)) return; /* dead-time only exists on TIM1/TIM8 */

    uint32_t ns_per_tick = 1000000000UL / timer_clk; /* assumes DTG prescaler=1, i.e. tDTS = tCK_INT */
    uint32_t dtg = dead_ns / (ns_per_tick == 0 ? 1 : ns_per_tick);
    if (dtg > 127) dtg = 127; /* covers DTG[7]=0 range only - see header note for longer dead times */

    TIMx->BDTR = (TIMx->BDTR & ~TIM_BDTR_DTG) | (dtg & TIM_BDTR_DTG);
}

/* =============================================================================
 * PWM input capture (CH1 signal in, CH1/CH2 pair per ST's "PWM input" trick)
 * ===========================================================================*/

void PWM_Capture_Init_CH1(GP_Pin pin, TIM_TypeDef *timer, uint8_t af)
{
    enableGPIOClock(pin.port);
    enableTimerClock(timer);
    SetPinAF(pin.port, pin.pin, af);

    timer->PSC = 0;
    timer->ARR = 0xFFFFFFFFUL; /* full range; 16-bit timers auto-truncate to 0xFFFF */

    /* CH1 <- TI1 direct (captures period, edge-to-edge via slave reset mode below) */
    timer->CCMR1 = (timer->CCMR1 & ~TIM_CCMR1_CC1S) | (1UL << TIM_CCMR1_CC1S_Pos);
    timer->CCMR1 &= ~TIM_CCMR1_IC1F;
    timer->CCER  &= ~TIM_CCER_CC1P;  /* rising edge */
    timer->CCER  |= TIM_CCER_CC1E;

    /* CH2 <- TI1 indirect, opposite edge (captures high time) */
    timer->CCMR1 = (timer->CCMR1 & ~TIM_CCMR1_CC2S) | (2UL << TIM_CCMR1_CC2S_Pos);
    timer->CCER  |= TIM_CCER_CC2P;   /* falling edge */
    timer->CCER  |= TIM_CCER_CC2E;

    /* Slave mode: reset on TI1FP1, so CCR1 = full period each cycle. Verify the TS
     * (trigger select) and SMS encodings below against RM0456 for your timer. */
    timer->SMCR = (timer->SMCR & ~TIM_SMCR_TS)  | (5UL << TIM_SMCR_TS_Pos);
    timer->SMCR = (timer->SMCR & ~TIM_SMCR_SMS) | (4UL << TIM_SMCR_SMS_Pos);

    timer->CR1 |= TIM_CR1_CEN;
}

uint32_t PWM_Capture_GetPeriodTicks(TIM_TypeDef *timer)
{
    return timer->CCR1;
}

uint32_t PWM_Capture_GetHighTicks(TIM_TypeDef *timer)
{
    return timer->CCR2;
}

float PWM_Capture_GetDuty(TIM_TypeDef *timer)
{
    uint32_t period = timer->CCR1;
    if (period == 0) return 0.0f;
    return (100.0f * (float)timer->CCR2) / (float)period;
}

float PWM_Capture_GetFrequency(TIM_TypeDef *timer)
{
    uint32_t period = timer->CCR1;
    if (period == 0) return 0.0f;
    uint32_t tick_hz = UNOQ_TIM_CLK_HZ / (timer->PSC + 1);
    return (float)tick_hz / (float)(period + 1);
}

/* =============================================================================
 * UNO Q pin table
 * ===========================================================================*/

GP_Pin PA0  = {GPIOA, 0};
GP_Pin PA1  = {GPIOA, 1};
GP_Pin PA3  = {GPIOA, 3};
GP_Pin PA4  = {GPIOA, 4};
GP_Pin PA5  = {GPIOA, 5};
GP_Pin PA6  = {GPIOA, 6};
GP_Pin PA7  = {GPIOA, 7};
GP_Pin PA8  = {GPIOA, 8};
GP_Pin PA11 = {GPIOA, 11};
GP_Pin PA12 = {GPIOA, 12};

GP_Pin PB0  = {GPIOB, 0};
GP_Pin PB1  = {GPIOB, 1};
GP_Pin PB2  = {GPIOB, 2};
GP_Pin PB3  = {GPIOB, 3};
GP_Pin PB4  = {GPIOB, 4};
GP_Pin PB6  = {GPIOB, 6};
GP_Pin PB7  = {GPIOB, 7};
GP_Pin PB8  = {GPIOB, 8};
GP_Pin PB9  = {GPIOB, 9};
GP_Pin PB10 = {GPIOB, 10};
GP_Pin PB11 = {GPIOB, 11};
GP_Pin PB13 = {GPIOB, 13};
GP_Pin PB14 = {GPIOB, 14};
GP_Pin PB15 = {GPIOB, 15};

GP_Pin PC0 = {GPIOC, 0};
GP_Pin PC1 = {GPIOC, 1};
GP_Pin PC2 = {GPIOC, 2};
GP_Pin PC3 = {GPIOC, 3};
GP_Pin PC6 = {GPIOC, 6};
GP_Pin PC7 = {GPIOC, 7};
GP_Pin PC8 = {GPIOC, 8};
GP_Pin PC9 = {GPIOC, 9};

#ifdef GPIOD
GP_Pin PD1  = {GPIOD, 1};
GP_Pin PD2  = {GPIOD, 2};
GP_Pin PD8  = {GPIOD, 8};
GP_Pin PD9  = {GPIOD, 9};
GP_Pin PD12 = {GPIOD, 12};
GP_Pin PD13 = {GPIOD, 13};
#endif

#ifdef GPIOE
GP_Pin PE2 = {GPIOE, 2};
GP_Pin PE3 = {GPIOE, 3};
GP_Pin PE4 = {GPIOE, 4};
GP_Pin PE5 = {GPIOE, 5};
GP_Pin PE6 = {GPIOE, 6};
GP_Pin PE7 = {GPIOE, 7};
GP_Pin PE8 = {GPIOE, 8};
#endif

#ifdef GPIOF
GP_Pin PF14 = {GPIOF, 14};
GP_Pin PF15 = {GPIOF, 15};
#endif

#ifdef GPIOH
GP_Pin PH10 = {GPIOH, 10};
GP_Pin PH11 = {GPIOH, 11};
GP_Pin PH12 = {GPIOH, 12};
GP_Pin PH13 = {GPIOH, 13};
GP_Pin PH14 = {GPIOH, 14};
GP_Pin PH15 = {GPIOH, 15};
#endif

#ifdef GPIOI
GP_Pin PI4 = {GPIOI, 4};
GP_Pin PI5 = {GPIOI, 5};
GP_Pin PI6 = {GPIOI, 6};
GP_Pin PI7 = {GPIOI, 7};
#endif

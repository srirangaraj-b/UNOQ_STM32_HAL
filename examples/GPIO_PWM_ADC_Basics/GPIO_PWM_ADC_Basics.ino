/* ============================================================================
 * GPIO_PWM_ADC_Basics
 *
 * Smallest possible tour of the library: one output pin, one PWM channel,
 * one ADC read. Good starting point to confirm the library is wired up right
 * before moving to the fuller PWM_UART_Control example.
 * ==========================================================================*/

#include <UNOQ_STM32_HAL.h>

void setup()
{
    Serial.begin(115200);
    delay(500);

    /* Digital output: blink an LED on PB8 */
    pinMode_STM(PB8, OUTPUT_STM, NOPULL_STM);

    /* PWM: 1kHz, 25% duty on PA8 (TIM1_CH1, AF1). No complementary output here. */
    PWM_Setup_STM(PA8, TIM1, 1, false, 1000, 25, 1);

    Serial.println("GPIO_PWM_ADC_Basics ready.");
}

void loop()
{
    togglePin_STM(PB8);

    /* ADC1 channel numbers are peripheral channel numbers, not pin numbers -
     * check the datasheet's ADC1 channel table for which channel your pin maps to. */
    int raw = adc1Read_STM(PA0, 5); /* example: PA0 on ADC1_IN5 - verify for your board */
    Serial.print("ADC raw: ");
    Serial.println(raw);

    delay(500);
}

/* ============================================================================
 * LiquidCrystal_Interop
 *
 * Shows that pins from the UNOQ_STM32_HAL table (PA8, PB0, ...) can be handed
 * straight to any normal Arduino library - here, the standard LiquidCrystal
 * library - because GP_Pin implicitly converts to the Arduino core's own
 * digital pin number. No separate "which D-number is this physically" lookup
 * needed; use the same PA8/PB0/etc names everywhere, register-level HAL calls
 * and stock Arduino libraries alike.
 *
 * Wiring is arbitrary here - swap in whatever pins you actually wired the LCD
 * to. PWM_Setup_STM on PA0 drives the LCD backlight brightness as a bonus,
 * mixing this library's PWM with a totally standard library in one sketch.
 * ==========================================================================*/

#include <UNOQ_STM32_HAL.h>
#include <LiquidCrystal.h>

/* LiquidCrystal(rs, en, d4, d5, d6, d7) - GP_Pin -> uint32_t happens automatically */
LiquidCrystal lcd(PB6, PB7, PC0, PC1, PA4, PA5);

void setup()
{
    /* Always safe to check: GP_ToArduinoPin() returns NC if a pin you picked
     * isn't exposed in this board's Arduino pin table. */
    if (GP_ToArduinoPin(PB6) == NC) {
        Serial.begin(115200);
        Serial.println("PB6 has no Arduino digital pin mapping on this variant!");
    }

    lcd.begin(16, 2);
    lcd.print("UNOQ_STM32_HAL");
    lcd.setCursor(0, 1);
    lcd.print("+ LiquidCrystal");

    /* Backlight PWM straight from our own HAL, same sketch, same pin table. */
    PWM_Setup_STM(PA0, TIM2, 1, false, 1000, 80, 1); /* verify TIM2_CH1 AF number for PA0 on your variant */
}

void loop()
{
    /* nothing to do */
}

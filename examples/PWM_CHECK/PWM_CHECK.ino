#include <UNOQ_STM32_HAL.h>


void setup() {
    Serial.begin(115200);
    delay(2000);

    PWM_Setup_STM(PA8, TIM1, 1, false, 10000, 20, 1);
    
    PWM_Capture_Init_Ex(PE4, TIM3, 2, 2, 0);

}

void loop() {
    Serial.print("Freq: ");
    Serial.print(PWM_Capture_GetFrequency(TIM3), 1);
    Serial.print(" Hz  |  Duty: ");
    Serial.print(PWM_Capture_GetDuty(TIM3), 1);
    Serial.println(" %");
    delay(500);
}
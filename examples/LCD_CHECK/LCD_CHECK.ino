#include <UNOQ_STM32_HAL.h>
#include <LiquidCrystal.h>


LiquidCrystal lcd(PE7, PE8, PF14, PF15, PA3, PD8);

void setup() {

  lcd.begin(20, 4);

  lcd.setCursor(0, 0);
  lcd.print("Arduino UNO Q");
  
  lcd.setCursor(0, 1);
  lcd.print("20x4 LCD Initialized");
}

void loop() {
  lcd.setCursor(0, 2);
  lcd.print("Uptime: ");
  lcd.print(millis() / 1000);
  lcd.print("s   ");

  lcd.setCursor(0, 3);
  lcd.print("Status: Working!    ");

  delay(500);
}
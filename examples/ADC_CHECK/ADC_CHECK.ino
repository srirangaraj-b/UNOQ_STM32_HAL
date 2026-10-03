#include <UNOQ_STM32_HAL.h>

#define TEST_ADC_PIN PA0

void setup() {
  Serial.begin(115200);
  Serial.println("ADC-Check");
}

void loop() {

  int rawPin  = analogRead_Pin_STM(TEST_ADC_PIN);
  int mvPin   = analogReadMilliVolts_Pin_STM(TEST_ADC_PIN);

  Serial.print("Pin Object (PA0)    -> Raw: ");
  Serial.print(rawPin);
  Serial.print(" | Voltage: ");
  Serial.print(mvPin);
  Serial.println(" mV");

  Serial.println("-----------------------------------------\n");
  delay(1000);
}
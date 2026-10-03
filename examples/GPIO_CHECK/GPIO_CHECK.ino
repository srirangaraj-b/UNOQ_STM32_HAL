#include <UNOQ_STM32_HAL.h>

// LED pins
const GP_Pin led1 = PC6;
const GP_Pin led2 = PE2;
const GP_Pin led3 = PI7;
const GP_Pin led4 = PD9;

// Switch pins 
const GP_Pin sw1 = PB14;
const GP_Pin sw2 = PB15;
const GP_Pin sw3 = PB9;
const GP_Pin sw4 = PB8;

// LED states
bool ledState1 = false;
bool ledState2 = false;
bool ledState3 = false;
bool ledState4 = false;


bool lastSw1State = HIGH;
bool lastSw2State = HIGH;
bool lastSw3State = HIGH;
bool lastSw4State = HIGH;

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); } // Wait for Serial Monitor to open
  Serial.println("================================");
  Serial.println("    Switch & LED Debug Monitor  ");
  Serial.println("================================");

  // Configure all 4 LED pins as outputs
  pinMode_STM(led1, OUTPUT);
  pinMode_STM(led2, OUTPUT);
  pinMode_STM(led3, OUTPUT);
  pinMode_STM(led4, OUTPUT);

  // Configure all 4 switch pins as inputs (hardware handles pull-up)
  pinMode_STM(sw1, INPUT);
  pinMode_STM(sw2, INPUT);
  pinMode_STM(sw3, INPUT);
  pinMode_STM(sw4, INPUT);
  
  Serial.println("Pins initialized. Press any switch...");
}

void loop() {

  bool currentSw1 = digitalRead_STM(sw1);
  if (lastSw1State == HIGH && currentSw1 == LOW) { // Button pressed
    ledState1 = !ledState1;
    digitalWrite_STM(led1, ledState1 ? HIGH : LOW);
    Serial.print("[DEBUG] SW1 (PB8) Pressed -> LED1 (PC6) set to: ");
    Serial.println(ledState1 ? "HIGH (ON)" : "LOW (OFF)");
    delay(50);
  }
  lastSw1State = currentSw1;

 
  bool currentSw2 = digitalRead_STM(sw2);
  if (lastSw2State == HIGH && currentSw2 == LOW) {
    ledState2 = !ledState2;
    digitalWrite_STM(led2, ledState2 ? HIGH : LOW);
    Serial.print("[DEBUG] SW2 (PB9) Pressed -> LED2 (PE2) set to: ");
    Serial.println(ledState2 ? "HIGH (ON)" : "LOW (OFF)");
    delay(50);
  }
  lastSw2State = currentSw2;

  
  bool currentSw3 = digitalRead_STM(sw3);
  if (lastSw3State == HIGH && currentSw3 == LOW) {
    ledState3 = !ledState3;
    digitalWrite_STM(led3, ledState3 ? HIGH : LOW);
    Serial.print("[DEBUG] SW3 (PB15) Pressed -> LED3 (PI7) set to: ");
    Serial.println(ledState3 ? "HIGH (ON)" : "LOW (OFF)");
    delay(50);
  }
  lastSw3State = currentSw3;

 
  bool currentSw4 = digitalRead_STM(sw4);
  if (lastSw4State == HIGH && currentSw4 == LOW) {
    ledState4 = !ledState4;
    digitalWrite_STM(led4, ledState4 ? HIGH : LOW);
    Serial.print("[DEBUG] SW4 (PB14) Pressed -> LED4 (PD9) set to: ");
    Serial.println(ledState4 ? "HIGH (ON)" : "LOW (OFF)");
    delay(50);
  }
  lastSw4State = currentSw4;
}
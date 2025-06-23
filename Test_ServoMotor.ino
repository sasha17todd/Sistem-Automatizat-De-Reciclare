/*****Test Servo Motor - simulare deschidere clapeta******/

#include <ESP32Servo.h> //biblioteca necesara pentru Servo motor compatibila cu ESP32

Servo mg;
const int PIN_SERVO = 13; //GPIO 13 - PIN cu PWM

void setup() {
  Serial.begin(115200);
  mg.setPeriodHertz(50);
  mg.attach(PIN_SERVO, 500, 2400);
}

void loop() {
  Serial.println("Deschid clapeta...");
  mg.write(150); //rotire la 150°
  delay(1000);

  Serial.println("Inchid clapeta...");
  mg.write(0); //revenire la pozitia initiala
  delay(2000);
}
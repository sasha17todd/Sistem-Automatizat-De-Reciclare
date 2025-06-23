/*******Test Stepper Motor*********/

#include <Stepper.h> //biblioteca necesara pentru Stepper Motor

const int STEPS = 2048;
Stepper stepper(STEPS, 25, 27, 26, 14); //IN1 - GPIO 25, IN3 - GPIO 27, IN2 - GPIO 26, IN4 - GPIO 14

const int STEP_DELAY = 2000; //2 secunde pauza intre rotatii
void setup() {
  Serial.begin(115200);
  stepper.setSpeed(10); //seteaza viteza
}

void loop() {
  Serial.println("- Rotire în sens orar -");
  for(int i = 0; i < 4; i++) {
    Serial.printf("Rotire +90° (pasul%d)\n", i + 1);
    stepper.step(STEPS / 4); //90° = 1/4 din 360°
    delay(STEP_DELAY);
  }

  Serial.println("- Rotire inversa -");
  for(int i = 0; i < 4; i++) {
    Serial.printf("Rotire -90° (pasul%d)\n", i + 1);
    stepper.step(-STEPS / 4); //invers
    delay(STEP_DELAY);
  }

  Serial.println("Ciclu complet.");
  delay(3000);
}
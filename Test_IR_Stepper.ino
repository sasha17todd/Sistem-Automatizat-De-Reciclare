/*Test Senzor IR + Stepper Motor. Cand Snezorul IR detecteaza un obiect, Stepper Motorul face o rotatie de 90°, 
asteapta 2 secunde, dupa care revine inapoi.*/
#include <Stepper.h>

const int PIN_IR = 33;

const int STEPS = 2048;
Stepper stepper(STEPS, 25, 27, 26, 14);
int positionDegree = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_IR, INPUT);
  stepper.setSpeed(10);
}

void loop() {
  static bool detectat = false;

  int ir = digitalRead(PIN_IR);

  if (ir == LOW && !detectat) {
    detectat = true;
    Serial.println("Obiect detectat!");

    rotireLa(90);
    delay(2000);
    rotireLa(0);
  }

  if (ir == HIGH && detectat) {
    detectat = false;
  }

  delay(100);
}

void rotireLa(int target) {
  int diff = ((target - positionDegree + 540) % 360) - 180;
  int steps = diff * STEPS / 360;
  stepper.step(steps);
  positionDegree = target;

  Serial.printf("Rotație la %d° (pași %d)\n", positionDegree, steps);
}

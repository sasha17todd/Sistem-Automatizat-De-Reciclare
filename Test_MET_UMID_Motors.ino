#include <Stepper.h>
#include <ESP32Servo.h>

const int PIN_IR = 33;
const int PIN_MET = 32;
const int PIN_UMID = 34;

const int STEPS = 2048;
Stepper stepper(STEPS, 25, 27, 26, 14);
int positionDegree = 0;

Servo mg;
const int PIN_SERVO = 13;
const int ANG_MAX = 150;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_IR, INPUT);
  pinMode(PIN_MET, INPUT);
  pinMode(PIN_UMID, INPUT);

  stepper.setSpeed(10);

  mg.setPeriodHertz(50);
  mg.attach(PIN_SERVO, 500, 2400);
  mg.write(0);

  Serial.println("Clasificare simplă: Metal / Menajer / Reciclabil");
}

void loop() {
  static bool detectat = false;

  int ir = digitalRead(PIN_IR);

  if (ir == LOW && !detectat) {
    detectat = true;
    Serial.println("Obiect detectat!");

    int target = 0;

    if (digitalRead(PIN_MET) == LOW) {
      Serial.println("Detectat: METAL");
      target = 90;
    } else if (digitalRead(PIN_UMID) == LOW) {
      Serial.println("Detectat: MENAJER");
      target = 0;
    } else {
      Serial.println("Detectat: Deșeu uscat");
      target = 270;
    }

    rotireLa(target);
    delay(2000);
    sweepServo();
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

void sweepServo() {
  mg.write(0);
  delay(200);
  mg.write(ANG_MAX);
  delay(800);
  mg.write(0);
  delay(200);
}

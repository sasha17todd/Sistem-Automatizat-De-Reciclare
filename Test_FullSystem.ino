/*Test pentru sistemul full cu toti senzorii integrati*/

#include <Stepper.h>
#include <ESP32Servo.h>

const int PIN_IR = 33;
const int PIN_MET = 32;
const int PIN_UMID = 34;

const int S0 = 18;
const int S1 = 19;
const int S2 = 21;
const int S3 = 22;
const int OUT = 23;

// Senzor ultrasonic
const int PIN_TRIG = 15;
const int PIN_ECHO = 2;

const float H_COUNT = 30.0; // container gol = 30 cm
const float H_FULL = 5.0;   // container plin = 5 cm

const int STEPS = 2048;
Stepper stepper(STEPS, 25, 27, 26, 14);
int positionDegree = 0;

Servo mg;
const int PIN_SERVO = 13;
const int ANG_MAX = 150;

String containerCurent = "";

void setup() {
  Serial.begin(115200);

  pinMode(PIN_IR, INPUT);
  pinMode(PIN_MET, INPUT);
  pinMode(PIN_UMID, INPUT);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  stepper.setSpeed(10);

  mg.setPeriodHertz(50);
  mg.attach(PIN_SERVO, 500, 2400);
  mg.write(0);

  Serial.println("Clasificare completă cu senzor culoare și ultrasonic");
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
      containerCurent = "Metal";
    } else if (digitalRead(PIN_UMID) == LOW) {
      Serial.println("Detectat: MENAJER");
      target = 0;
      containerCurent = "Menajer";
    } else {
      int red = readColor(LOW, LOW);
      int green = readColor(HIGH, HIGH);
      int blue = readColor(LOW, HIGH);

      Serial.printf("RGB -> R: %d, G: %d, B: %d\n", red, green, blue);

      bool hartie = (red <= 200 && green <= 400 && blue <= 400);
      if (hartie) {
        Serial.println("Detectat: HÂRTIE");
        target = 180;
        containerCurent = "Hartie";
      } else {
        Serial.println("Detectat: PLASTIC");
        target = 270;
        containerCurent = "Plastic";
      }
    }

    rotireLa(target);
    delay(2000);
    sweepServo();

    // Măsurare nivel ultrasonic
    delay(3000);
    float dist = distCM();
    if (dist != -1) {
      int pct = pctPlin(dist);
      Serial.printf("Nivel %s: %d%% (dist=%.1f cm)\n", containerCurent.c_str(), pct, dist);
    }

    rotireLa(0);
  }

  if (ir == HIGH && detectat) {
    detectat = false;
  }

  delay(100);
}

int readColor(int v2, int v3) {
  digitalWrite(S2, v2);
  digitalWrite(S3, v3);
  delay(50);
  unsigned long t = pulseIn(OUT, LOW, 50000);
  return (t == 0) ? 50000 : t;
}

float distCM() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long t = pulseIn(PIN_ECHO, HIGH, 30000);
  if (t == 0) {
    Serial.println("Fără ecou ultrasonic!");
    return -1;
  }
  return t * 0.034 / 2.0;
}

int pctPlin(float d) {
  if (d >= H_COUNT) return 0;
  if (d <= H_FULL) return 90;
  return (int)round((H_COUNT - d) / (H_COUNT - H_FULL) * 90.0);
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

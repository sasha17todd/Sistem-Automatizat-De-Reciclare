/******Test individual Senzor Ultrasonic*****/

const int PIN_TRIG = 15; //GPIO 15 
const int PIN_ECHO = 2; //GPIO 2 

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT); //trimitere impuls
  pinMode(PIN_ECHO, INPUT); //primire semnal reflectat
}

void loop() {
  long t, dist;

  digitalWrite(PIN_TRIG, LOW); //initializare TRIG
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH); // senzorul emite unda ultrasonica
  delayMicroseconds(10); //timp de 10 microsecunde
  digitalWrite(PIN_TRIG, LOW); //revine pe 0

  t = pulseIn(PIN_ECHO, HIGH); //masoara cat timp ramane ECHO pe HIGH (durata undei dus-intors)
  dist = t * 0.034 / 2; /*inmultesc timpul cu viteza undei (340 m/s sau 0.034 cm/microsecunde) pentru a afla distanta, 
                          apoi impart la 2 pentru a afla doar distanta pana la obstacol (adica unda doar dus)*/

  Serial.printf("Distanta: %ld cm\n", dist);
  delay(1000);
}
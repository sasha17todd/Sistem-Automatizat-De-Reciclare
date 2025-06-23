/******Test individual Senzor de Culoare*****/

//declarare conectivitate pini 
const int S0 = 18;
const int S1 = 19;
const int S2 = 21;
const int S3 = 22;
const int OUT = 23;

void setup() {
  Serial.begin(115200);
  //initializare pini
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);
}

void loop() {
  int red = readColor(LOW, LOW);
  int green = readColor(HIGH, HIGH);
  int blue = readColor(LOW, HIGH);

  Serial.printf("R: %d, G:%d, B:%d\n", red, green, blue);
  delay(1000);
}

int readColor(int v2, int v3){
  digitalWrite(S2, v2);
  digitalWrite(S3, v3);
  delay(50);
  return pulseIn(OUT, LOW);
}
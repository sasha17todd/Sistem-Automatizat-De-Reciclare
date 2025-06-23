/******Test individual Senzor de Metal*****/

const int PIN_MET = 32; //conectat la GPIO 32 al placii ESP32

void setup() {
  Serial.begin(115200);
  pinMode(PIN_MET, INPUT); //initializare pin
}

void loop() {
  int val = digitalRead(PIN_MET);
  Serial. println(val == LOW ? "Obiect metalic detectat!" : "Obiect nemetalic");
  delay(500);
}

/*Apropii o doză de aluminiu de borna senzorului inductiv. Led-ul de la capătul senzorului trebuie să se aprindă roșu.*/
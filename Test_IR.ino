/*****Test individual Senzor IR******/

const int PIN_IR = 33; //conectat la GPIO 33 al placii ESP32

void setup() {
  Serial.begin(115200); //baud rate
  pinMode(PIN_IR, INPUT); //inițializare pin
}

void loop() {
  int val = digitalRead(PIN_IR);
  Serial. println(val == LOW ? "Obiect detectat!" : "Nimic detectat.");
  delay(500);
}

/*Apropii mâna până la 0.5 cm de senzor. Dacă semnalul senzorului este setat pe Low, înseamnă că s-a detecat un obiect. 
În caz contrar, senzorul nu a detectat nimic.*/
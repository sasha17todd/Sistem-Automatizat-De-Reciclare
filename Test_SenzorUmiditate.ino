/******Test individual Senzor de Umiditate*****/

const int PIN_UMID = 34; //conectat la GPIO 34 al placii ESP32

void setup() {
  Serial.begin(115200);
  pinMode(PIN_UMID, INPUT);
}

void loop() {
  int val = digitalRead(PIN_UMID);
  Serial.println(val == LOW ? "Deșeu menajer!" : "Deșeu uscat.");
  delay(500);
}

/*Când se detectează un obiect umed/deșeu menajer, semnalul senzorului de umiditate este LOW. În caz contrar, se clasifică ca deșeu uscat.*/
/*****************************************/
/*** Sistem Automatizat de Reciclare ****/
/***************************************/


/*importarea bibliotecilor necesare*/
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <time.h>
#include <Stepper.h>
#include <ESP32Servo.h>

/*Conectarea la WiFi & Firebase*/
#define WIFI_SSID "Sasha-HUAWEI P60 Pro"
#define WIFI_PASS "********"

#define API_KEY   "*******************************"
#define DB_URL    "https://monitorizaredeseuri-90e3c-default-rtdb.europe-west1.firebasedatabase.app/"

#define USER_EMAIL "ale******.********@gmail.com" 
#define USER_PASS  "********"

/****Declararea porturilor de conectivitate la ESP32****/
const int PIN_IR = 33; //senzor infrarosu
const int PIN_MET = 32; //senzor de metal (inductiv)
const int PIN_UMID = 34; //senzor de umiditate

/*Senzor de culoare*/
const int S0 = 18;
const int S1 = 19;
const int S2 = 21;
const int S3 = 22;
const int OUT = 23;

/*Senzor ultrasonic*/
const int PIN_TRIG = 15;
const int PIN_ECHO = 2;

/*Stepper motor*/
const int STEPS = 2048;
Stepper stp(STEPS, 25, 27, 26, 14); //IN1, IN3, IN2, IN4
int positionDegree = 0;
#define DIR_SIGN (+1) //semnal de direcție

/*Servo motor*/
Servo mg;
const int PIN_SERVO = 13;
const int ANG_MAX = 150;

/*Nivel deseuri cm*/
const float H_COUNT = 30.0; //30 cm - container gol
const float H_FULL = 5.0; // <5 cm - container plin

/*Contorizare tipuri de deseuri*/
int countMenajer = 0;
int countMetal = 0;
int countPlastic = 0;
int countHartie = 0;
String containerCurent = "Menajer"; //evidența ultimului container folosit

/*****Obiecte Firebase******/
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig cfg;

/***Funcțiile prototip***/
int readColor(int, int); //funcție senzor de culoare - citirea culorii
void rotireLa(int); //funcție rotire Stepper Motor
void sweepServo(); //funcție control clapeta cu Servo Motor
long distCM(); //funcție senzor ultrasonic de măsurare a distanței
int pctPlin(float); //interschimbare cm în % de umplere
void uploadCounters(); //Update date în Firebase și Interfață
void incrementHour(); //Calcul colectări pe oră

/***************SETUP*******************/
void setup()
{
  Serial.begin(115200); //baud rate
  Serial.println("------- Inițializare Sistem -------");

  /*inițializare pini*/
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

  stp.setSpeed(10); //inițializare viteză stepper

//inițializare servo
  mg.setPeriodHertz(50); //setare pulsații
  mg.attach(PIN_SERVO, 500, 2400); //setare speed range
  mg.write(0); //inițializare poziție 0

/***Conectare la WiFi***/
Serial.print("Conectare Wi-Fi");
WiFi.begin(WIFI_SSID, WIFI_PASS);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print('.');
    delay(300);
  }
Serial.print("OK");

/*Calculare oră pentru histogramă - NTP (UTC+2)*/
configTime(2*3600,0,"pool.ntp.org", "time.nist.gov");

/***Configurare Firebase***/
cfg.api_key = API_KEY;
cfg.database_url = DB_URL;
auth.user.email = USER_EMAIL;
auth.user.password = USER_PASS;
Firebase.begin(&cfg, &auth);
Firebase.reconnectWiFi(true);
Serial.println("Conectare cu Firebase reușită!");

rotireLa(0); //inițializare stepper la poziția 0
}

/********Bucla principală de funcționare********/
void loop()
{
  static bool obiectDetectat = false;
  int ir = digitalRead(PIN_IR);

  /*Detecție - începutul unui ciclu*/
  if(ir == LOW && !obiectDetectat){
    obiectDetectat = true;
    Serial.println("\n[IR] Obiect detectat");

    delay(200);
    int target = 0; //inițializare unghi de rotație

    /*Clasificare*/
    if(digitalRead(PIN_MET) == LOW){
      containerCurent = "Metal"; //se setează containerul curent
      target = 90; //rotire platformă la 90 de grade
      countMetal++; //se incrementează numărul de deșeuri
      Serial.println("Deșeu detectat: Metal");
    }
    else if(digitalRead(PIN_UMID) == LOW){
      containerCurent = "Menajer";
      target = 0;
      countMenajer++;
      Serial.println("Deșeu detectat: Menajer");
    }
    else{
      int red = readColor(LOW, LOW);
      int green = readColor(HIGH, HIGH);
      int blue = readColor(LOW, HIGH);
      Serial.printf("Valori RGB: R:%d  G:%d  B:%d\n", red, green, blue);

      bool hârtie = (red <= 200 && green <= 400 && blue <= 400);
      if(hârtie){
        containerCurent = "Hartie";
        target = 180;
        countHartie++;
        Serial.println("Deșeu detectat: Hârtie");
      }
      else{
        containerCurent = "Plastic";
        target = 270;
        countPlastic++;
        Serial.println("Deșeu detectat: Plastic");
      }
    }
    
    /*Rotire platformă cu containere*/
    Serial.printf("Rotire la %d° ...\n", target);
    rotireLa(target);

    /*Deschidere clapetă pentru evacuare*/
    Serial.print("Aștept evacuarea");
    sweepServo();

    /*Măsurare nivel deșeuri & upload date*/
    delay(3000);
    long dist = distCM();
    if(dist != -1){
      int pct = pctPlin(dist);
      Serial.printf("Nivel %s: %d%% (dist=%.1f cm)\n", containerCurent.c_str(), pct, (float)dist); 

      if(Firebase.ready()){
        String p = "/container/" + containerCurent; //creare nod cu *containerCurent în Firebase
        Firebase.RTDB.setInt(&fbdo, p + "/percent", pct); //încărcare/actualizare procentaj de umplere
        Firebase.RTDB.setInt(&fbdo, p + "/distance_cm", dist);
        Firebase.RTDB.setString(&fbdo, p + "/last_update", String(millis()/1000));
      }
    }

    uploadCounters();
    incrementHour();
  }

  /***Resetare flag***/
  if(ir == HIGH && obiectDetectat) {
    obiectDetectat = false;
  }

  delay(200);
}

/*****Definirea Funcțiilor*****/

/*Funcție pentru citirea culorii - Senzor de culoare TCS2300*/
int readColor(int v2, int v3){
  digitalWrite(S2, v2);
  digitalWrite(S3, v3);
  delay(50);
  unsigned long t = pulseIn(OUT, LOW, 50000);
  
  return(t == 0) ? 50000 : t;
}

/*Funcție pentru mișcarea platformei cu cel mai scurt drum - Stepper Motor*/
void rotireLa(int tgt){
  int diff = ((tgt - positionDegree + 540) % 360 ) - 180;
  int steps = diff * STEPS / 360;
  stp.step(DIR_SIGN * steps); //schimbă direcția
  positionDegree = tgt;
  Serial.printf("Ajuns la %d° (pași %d)\n", positionDegree, steps);
}

/*Funcție pentru mișcare clapetei - Servo Motor*/
void sweepServo(){
  mg.write(0); //poziție inițială 0
  delay(200);  
  mg.write(ANG_MAX); //mișcă la 150°
  delay(800);
  mg.write(0); //revine la poziția inițială
  delay(200);
}

/*Funcție pentru măsurarea distanței - Senzor Ultrasonic HC-SR04*/
long distCM(){
  digitalWrite(PIN_TRIG, LOW); 
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long t = pulseIn(PIN_ECHO, HIGH, 30000);
  if(t == 0){
    Serial.println("Fără ecou ultrasonic!");
    return -1;
  }
  return t * 0.034 / 2;
}

/*Funcție pentru calcularea procentului de umplere*/
int pctPlin(float d){
  if(d >= H_COUNT) return 0;
  if (d <= H_FULL) return 90;
  return (int)round((H_COUNT - d) / (H_COUNT - H_FULL) * 90.0);
}

/*Funcție pentru incrementarea counterilor în Firebase*/
void uploadCounters(){
  if(!Firebase.ready()) return;
  Firebase.RTDB.setInt(&fbdo, "/stats/Menajer", countMenajer);
  Firebase.RTDB.setInt(&fbdo, "/stats/Metal", countMetal);
  Firebase.RTDB.setInt(&fbdo, "/stats/Hartie", countHartie);
  Firebase.RTDB.setInt(&fbdo, "/stats/Plastic", countPlastic);
  Serial.printf("Counteri actualizați: Men: %d, Met: %d, H: %d, P: %d", countMenajer, countMetal, countHartie, countPlastic);
}

/*Funcție pentru calcularea colectărilor pe ore*/
void incrementHour(){
  if(!Firebase.ready()) return;
  time_t now;
  time(&now);
  struct tm *tmNow = localtime(&now);
  int hr = tmNow ? tmNow -> tm_hour : (millis()/3600000) % 24;
  String p = "/hours" + String(hr);
  int current = 0;
  Firebase.RTDB.getInt(&fbdo, p, &current);
  Firebase.RTDB.setInt(&fbdo, p, current + 1);
  Serial.printf("Ora %d -> %d\n", hr, current + 1);
}


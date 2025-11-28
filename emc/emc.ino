#include <WiFi.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include "DHT.h"
#include "LittleFS.h"

// ---------------------------
// Configuracion wifi
// ---------------------------
const char* ssid = "Pixel";
const char* password = "andres12345";

AsyncWebServer server(80);

// ---------------------------
// Pines del MQ135, DHT11 y FC-28 (solo A0)
// ---------------------------
const int MQ_135 = 34;
const int DHT_PIN = 33;
const int SOIL_MOISTURE_PIN = 35;

DHT dht(DHT_PIN, DHT11);

// ---------------------------
// Pines del Display 5461AS-1
// ---------------------------
const int segmentPins[8] = {2, 4, 5, 17, 19, 21, 22, 23};
const int digitPins[4]   = {25, 26, 27, 14};

// Patrones
const byte digits[12] = {
  0b11111100,  // 0
  0b01100000,  // 1
  0b11011010,  // 2
  0b11110010,  // 3
  0b01100110,  // 4
  0b10110110,  // 5
  0b10111110,  // 6
  0b11100000,  // 7
  0b11111110,  // 8
  0b11110110,  // 9
  0b00000000,  // 10 - Apagado
  0b10011100   // 11 - C
};

// ---------------------------
// Variables
// ---------------------------
int filteredValue = 0;
float currentTempC = 0.0;
float airHumidity = 0.0;
int soilMoisture = 0;
int soilMoistureRaw = 0;

unsigned long lastReadingTime = 0;
const unsigned long READING_INTERVAL = 5000;

unsigned long lastDisplayTime = 0;
const unsigned long DISPLAY_INTERVAL = 2;  // 2 ms por dígito
int currentDigit = 0;
int d[4];

// ---------------------------
// Valores calibrados del FC-28
// ---------------------------
const int FC28_DRY_VALUE = 4095;  // seco
const int FC28_WET_VALUE = 430;   // muy mojado

// ---------------------------
void displayDigit(int number, int position, bool decimalPoint = false);

// ---------------------------
// Lectura del sensor FC-28
// ---------------------------
int readFC28Moisture() {
  long sum = 0;
  const int samples = 8;

  for (int i = 0; i < samples; i++) {
    sum += analogRead(SOIL_MOISTURE_PIN);
  }

  soilMoistureRaw = sum / samples;

  float ratio = (float)(FC28_DRY_VALUE - soilMoistureRaw) /
                (float)(FC28_DRY_VALUE - FC28_WET_VALUE);

  int percent = (int)(ratio * 100.0);
  return constrain(percent, 0, 100);
}

// ---------------------------
// Setup
// ---------------------------
void setup() {
  Serial.begin(115200);
  if(!LittleFS.begin(true)){   // true = formatea si falla
    Serial.println("Error al montar LittleFS");
    return;
  }
  Serial.println("LittleFS montado correctamente");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print('.');
  }
  
  Serial.println(WiFi.localIP());
  //Aqui se crea la pagina web que se subio con el littlefs 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });

  //Esta es una funcion que toma el javascript para jalar los datos de aqui
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
  String json = createJson();
  request->send(200, "application/json", json);
  });

  server.begin();
  dht.begin();

  for (int i = 0; i < 8; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], LOW);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], HIGH);
  }

  pinMode(MQ_135, INPUT);
  pinMode(SOIL_MOISTURE_PIN, INPUT);
}

// ---------------------------
// Crear JSON
// ---------------------------
String createJson(){
  StaticJsonDocument<200> jsonData;
  jsonData["temperature"] = currentTempC;
  jsonData["airHumidity"] = airHumidity;
  jsonData["airQuality"] = filteredValue;
  jsonData["dirtHumidity"] = soilMoisture;
  String finalJson;
  serializeJson(jsonData, finalJson);
  return finalJson;
}

// ---------------------------
// Lecturas de sensores
// ---------------------------
void takeReadings() {

  // MQ135
  int raw = analogRead(MQ_135);
  filteredValue = (filteredValue * 0.7) + (raw * 0.3);

  // Calidad del aire
  String presGas;
  if (filteredValue < 370) presGas = "Buena Calidad";
  else if (filteredValue < 500) presGas = "Calidad Media";
  else presGas = "Mala Calidad";

  // DHT11
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) currentTempC = t;
  if (!isnan(h)) airHumidity = h;

  // FC-28
  soilMoisture = readFC28Moisture();

  // Serial
  Serial.println("");
  Serial.println("==========Lecturas==========");
  Serial.print("Temperatura = ");
  Serial.print(currentTempC, 1);
  Serial.print("C | Humedad en el Aire = ");
  Serial.print(airHumidity, 0);
  Serial.print("% | Calidad de aire = ");
  Serial.print(filteredValue);
  Serial.print(" (");
  Serial.print(presGas);
  Serial.print(") | Humedad en el Suelo = ");
  Serial.print(soilMoisture);
  Serial.println("%");
  Serial.print("============================");
}

// ---------------------------
// Mostrar en display
// ---------------------------
void displayDigit(int number, int position, bool decimalPoint) {
  for (int i = 0; i < 4; i++)
    digitalWrite(digitPins[i], HIGH);

  byte segmentPattern = digits[number];
  if (decimalPoint) segmentPattern |= 0b00000001;

  for (int s = 0; s < 8; s++) {
    bool segmentState = segmentPattern & (1 << (7 - s));
    digitalWrite(segmentPins[s], segmentState);
  }

  digitalWrite(digitPins[position], LOW);
}

void updateDisplay() {
  if (millis() - lastDisplayTime >= DISPLAY_INTERVAL) {
    lastDisplayTime = millis();

    int tempInt = (int)(currentTempC * 10);

    if (currentTempC >= 10.0 && currentTempC < 100.0) {
      d[0] = (tempInt / 100) % 10;
      d[1] = (tempInt / 10) % 10;
      d[2] = tempInt % 10;
      d[3] = 11;
    } 
    else if (currentTempC < 10.0) {
      d[0] = 10;
      d[1] = (tempInt / 100) % 10;
      d[2] = (tempInt / 10) % 10;
      d[3] = 11;
    } 
    else {
      int ti = (int)currentTempC;
      d[0] = ti / 100;
      d[1] = (ti / 10) % 10;
      d[2] = ti % 10;
      d[3] = 11;
    }

    for (int i = 0; i < 4; i++)
      digitalWrite(digitPins[i], HIGH);

    bool decimalPoint = false;
    if (currentTempC >= 10.0 && currentTempC < 100.0)
      decimalPoint = (currentDigit == 1);
    else if (currentTempC < 10.0)
      decimalPoint = (currentDigit == 0);

    displayDigit(d[currentDigit], currentDigit, decimalPoint);

    currentDigit = (currentDigit + 1) % 4;
  }
}


// ---------------------------
// Loop
// ---------------------------
void loop() {
  unsigned long currentTime = millis();

  if (currentTime - lastReadingTime >= READING_INTERVAL) {
    lastReadingTime = currentTime;
    takeReadings();
  }

  updateDisplay();
}

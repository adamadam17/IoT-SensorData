#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_sleep.h>

// Piny pre senzor dažďa/snehu
const int rainSensorPin = 25; // Digitálny pin, kam je pripojený senzor

// Inicializácia senzorov
Adafruit_BME280 bme;      // BME280
BH1750 lightMeter;        // BH1750

// MQTT nastavenia
const char* mqtt_server = "192.168.0.243";  // IP adresa tvojho MQTT brokera
const int mqtt_port = 1883;  // Port MQTT brokera

WiFiClient espClient;
PubSubClient client(espClient);

// WiFi pripojenie
const char* ssid = "Adam";  // WiFi SSID
const char* password = "tibor217";  // WiFi heslo

void connectWiFi() {
  Serial.print("Pripojenie na WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Pripojené k WiFi!");
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Pokúšam sa pripojiť k MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(WiFi.macAddress());

    if (client.connect(clientId.c_str())) {
      Serial.println("Pripojené k MQTT brokeru");
    } else {
      Serial.print("Neúspešné, stav: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void sendData() {
  // Čítanie dát zo senzorov
  float temperature = bme.readTemperature();
  float pressure = bme.readPressure() / 100.0F;
  float humidity = bme.readHumidity();
  float lux = lightMeter.readLightLevel();
  int rainDetected = digitalRead(rainSensorPin);

  int rainStatus = (rainDetected == LOW) ? 1 : 0;


  // Vytvorenie JSON formátu
  String payload = "{\"temperature\": " + String(temperature, 2) +
                   ", \"humidity\": " + String(humidity, 2) +
                   ", \"pressure\": " + String(pressure, 2) +
                   ", \"light_level\": " + String(lux, 2) +
                   ", \"rain_status\": " + String(rainStatus) + "}";

  // Odoslanie dát na MQTT broker
  if (client.publish("sensor/data", payload.c_str())) {
    Serial.println("Dáta odoslané na MQTT broker: " + payload);
  } else {
    Serial.println("Chyba pri odosielaní dát.");
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Inicializácia WiFi a MQTT
  connectWiFi();
  client.setServer(mqtt_server, mqtt_port);

  // Inicializácia BME280
  if (!bme.begin(0x76)) { // Skús adresu 0x76 alebo 0x77
    Serial.println("Chyba: Senzor BME280 nenájdený!");
    while (1);
  }

  // Inicializácia BH1750
  if (!lightMeter.begin()) {
    Serial.println("Chyba: Senzor BH1750 nenájdený!");
    while (1);
  }

  // Inicializácia pinu pre senzor daža/snehu
  pinMode(rainSensorPin, INPUT);
  Serial.println("Dažďový senzor pripravený.");

  Serial.println("Senzory inicializované.");
}

void loop() {
  // Skontroluj pripojenie k MQTT
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Odoslanie dát
  sendData();

  // Prechod do režimu spánku na 5 minut
  Serial.println("Prechádzam do režimu spánku na  minut...");
  delay(200); // Krátke oneskorenie na spracovanie pred spánkom
  esp_sleep_enable_timer_wakeup(30*60*1000000); // Nastavenie wake-up časovača
  esp_deep_sleep_start(); // Prechod do hlbokého spánku
}
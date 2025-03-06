#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_sleep.h>

const int rainSensorPin = 25; 

Adafruit_BME280 bme;      
BH1750 lightMeter;        

const char* mqtt_server = "192.168.***.***";  
const int mqtt_port = 1883;  

WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid = "****";  
const char* password = "****";  

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

  float temperature = bme.readTemperature();
  float pressure = bme.readPressure() / 100.0F;
  float humidity = bme.readHumidity();
  float lux = lightMeter.readLightLevel();
  int rainDetected = digitalRead(rainSensorPin);

  int rainStatus = (rainDetected == LOW) ? 1 : 0;

  String payload = "{\"temperature\": " + String(temperature, 2) +
                   ", \"humidity\": " + String(humidity, 2) +
                   ", \"pressure\": " + String(pressure, 2) +
                   ", \"light_level\": " + String(lux, 2) +
                   ", \"rain_status\": " + String(rainStatus) + "}";

  
  if (client.publish("sensor/data", payload.c_str())) {
    Serial.println("Dáta odoslané na MQTT broker: " + payload);
  } else {
    Serial.println("Chyba pri odosielaní dát.");
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  
  connectWiFi();
  client.setServer(mqtt_server, mqtt_port);

  
  if (!bme.begin(0x76)) { 
    Serial.println("Chyba: Senzor BME280 nenájdený!");
    while (1);
  }

  if (!lightMeter.begin()) {
    Serial.println("Chyba: Senzor BH1750 nenájdený!");
    while (1);
  }

  
  pinMode(rainSensorPin, INPUT);
  Serial.println("Dažďový senzor pripravený.");

  Serial.println("Senzory inicializované.");
}

void loop() {
  
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  sendData();

  Serial.println("Prechádzam do režimu spánku na 30 minut...");
  delay(200); 
  esp_sleep_enable_timer_wakeup(30*60*1000000); 
  esp_deep_sleep_start(); 
}

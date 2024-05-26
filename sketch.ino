#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHTesp.h"

const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* ssid = "Wokwi-GUEST";
const char* password = "";

WiFiClient espClient;
PubSubClient client(espClient);

long lastTime = 0;

const int DHT_PIN = 15;
const int POT_PIN = 34;
const int LED_PIN = 2;

DHTesp dhtSensor;

void setup_wifi() {
  Serial.println("Starting Setup Wifi");
  delay(10);
  Serial.println();
  Serial.print("Connecting to Wifi ");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
}

void setup_humidity_temperature_sensor() {
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
}

void callback(String topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String strPayload = "";
  for (int i = 0; i < length; i++) {
    strPayload = strPayload + (char)payload[i];
  }

  Serial.println(strPayload);
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "AulaIoT-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected to the Broker");
      client.subscribe("IPB/IoT/Projeto/Alisson_Joao/Planta");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n Start Setup");
  setup_wifi();
  setup_humidity_temperature_sensor();
  pinMode(LED_PIN, OUTPUT);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
  }
  if (!client.connected()) {
    Serial.println("Node disconnected of Broker. Trying to connect.. ");
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastTime > 1000) {
    lastTime = now;

    int value = analogRead(POT_PIN);
    int intensity = map(value, 0, 4095, 0, 255);
    analogWrite(LED_PIN, intensity);

    TempAndHumidity data = dhtSensor.getTempAndHumidity();

    StaticJsonDocument<200> jsonDoc;
    jsonDoc["temperature"] = data.temperature;
    jsonDoc["airHumidity"] = data.humidity;
    jsonDoc["soilMoisture"] = intensity;

    char jsonBuffer[256];
    serializeJson(jsonDoc, jsonBuffer);

    client.publish("IPB/IoT/Projeto/Alisson_Joao/Planta", jsonBuffer);
  }
}
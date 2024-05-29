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

const int DHT_PIN1 = 18;
const int DHT_PIN2 = 19;

const int LED_PIN = 2;

DHTesp dhtSensor1;
DHTesp dhtSensor2;

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
  dhtSensor1.setup(DHT_PIN1, DHTesp::DHT22);
  dhtSensor2.setup(DHT_PIN2, DHTesp::DHT22);
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
  Serial.println("\n Start Setup");
  Serial.begin(115200);
  setup_wifi();
  setup_humidity_temperature_sensor();
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

    TempAndHumidity data1 = dhtSensor1.getTempAndHumidity();
    TempAndHumidity data2 = dhtSensor2.getTempAndHumidity();

    StaticJsonDocument<200> jsonDoc;
    jsonDoc["temperature"] = data1.temperature;
    jsonDoc["airHumidity"] = data1.humidity;
    jsonDoc["soilMoisture"] = data2.humidity;

    char jsonBuffer[256];
    serializeJson(jsonDoc, jsonBuffer);

    client.publish("IPB/IoT/Projeto/Alisson_Joao/Planta", jsonBuffer);
  }
}
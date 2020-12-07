#include <ArduinoJson.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <PubSubClient.h>

const char *ssid = "";
const char *password = "";
const char *mqtt_server = "";
const char *clientId = "nodemcu_client1";

const int oneWireBus = D2;
const int ledPin = D1;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

WiFiClient espClient;
PubSubClient mqttClient(espClient);
unsigned long previousMillis = 0;
#define INTERVAL 30000

#define PUBLISH_TOPIC "nodemcu/ds18b20"
#define SUBSCRIBE_TOPIC "nodemcu/led"
#define MSG_BUFFER_SIZE (100)
char msg[MSG_BUFFER_SIZE];

void setupWifi() {
  delay(10);
  Serial.printf("\nConnecting to %s", ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.printf("\nWiFi connected\nIP address: %s\n",
                WiFi.localIP().toString().c_str());
}

void dataCallback(char *topic, byte *payload, unsigned int length) {
  char payloadStr[length + 1];
  memset(payloadStr, 0, length + 1);
  strncpy(payloadStr, (char *)payload, length);
  Serial.printf("dataCallback. Topic : [%s]\n", topic);
  Serial.printf("dataCallback. Payload : %s\n", payloadStr);

  StaticJsonDocument<100> doc;
  deserializeJson(doc, payload, length);

  if (doc.containsKey("led")) {
    if (doc["led"] == "ON") {
      digitalWrite(ledPin, HIGH);
    } else if (doc["led"] == "OFF") {
      digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(clientId)) {
      Serial.println("connected");
      mqttClient.subscribe(SUBSCRIBE_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void publishTemperature() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > INTERVAL) {
    previousMillis = currentMillis;
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);
    snprintf(msg, MSG_BUFFER_SIZE,
             "{\"client\":\"%s\", \"temperature\": \"%02.02f\"}", clientId,
             temperatureC);
    Serial.print("Publish message: ");
    Serial.println(msg);
    mqttClient.publish(PUBLISH_TOPIC, msg);
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  sensors.begin();
  setupWifi();
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(dataCallback);
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }

  mqttClient.loop();
  publishTemperature();
}
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "SuaRedeWiFi";
const char* password = "SuaSenhaWiFi";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

#define LED_PIN 2  // Pino onde o LED está conectado

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == "led/acionar" && message == "ON") {
    digitalWrite(LED_PIN, HIGH);  // Aciona o LED
  } else {
    digitalWrite(LED_PIN, LOW);  // Desliga o LED
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client2")) {
      client.subscribe("led/acionar");
    } else {
      delay(5000);
    }
  }
}

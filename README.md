# Projeto de Monitoramento de Temperatura IoT com ESP32 e MQTT

Este projeto utiliza uma ESP32 conectada a um sensor de temperatura e umidade DHT11, que envia dados para um broker MQTT. Os dados são processados pelo Node-RED, que decide se a temperatura excede um limite de 30°C. Caso isso ocorra, um comando é enviado para outra ESP32 que aciona um LED.

## Arquitetura do Projeto

- ESP32_1: Coleta dados do sensor DHT11 e envia via MQTT para o broker HiveMQ.
- HiveMQ Broker: Recebe os dados de temperatura e os repassa para o Node-RED.
- Node-RED: Verifica se a temperatura excede 30°C. Se sim, publica um comando para acionar o LED.
- ESP32_2: Recebe o comando e aciona o LED quando necessário.

## Componentes Utilizados

- ESP32 (2 unidades)
- Sensor DHT11: Para leitura de temperatura e umidade.
- LED: Indicador visual quando a temperatura excede o limite.
- HiveMQ Broker: Broker MQTT gratuito para enviar e receber mensagens.
- Node-RED: Processamento dos dados recebidos do HiveMQ e tomada de decisões.
- Bibliotecas: WiFi, PubSubClient, DHT.

## Estrutura de Funcionamento


## Estrutura de Pastas do Projeto

   ```bash
/monitoramento-temperatura-iot
│
├── /ESP32_1
│   ├── main.cpp          # Código para ESP32 1 - Coleta e envio de dados
│   └── platformio.ini    # Configurações do projeto PlatformIO
│
├── /ESP32_2
│   ├── main.cpp          # Código para ESP32 2 - Recebe dados e aciona LED
│   └── platformio.ini    # Configurações do projeto PlatformIO
│
├── /NodeRED
│   └── node-red-fluxo.json  # Arquivo de exportação do fluxo Node-RED
│
└── README.md             # Documentação do projeto
   ```

## Configuração do Projeto no ESP32
### Código para ESP32 (1) - Coleta e Envio de Dados

   ```bash
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 4  // Pino onde o DHT11 está conectado
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "SuaRedeWiFi";
const char* password = "SuaSenhaWiFi";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  
  if (!isnan(temp) && !isnan(hum)) {
    String payload = "Temperatura: " + String(temp) + "C, Umidade: " + String(hum) + "%";
    client.publish("sensor/temperatura", payload.c_str());
  }
  
  delay(2000);  // A cada 2 segundos envia os dados
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      client.subscribe("sensor/temperatura");
    } else {
      delay(5000);
    }
  }
}
   ```

### Código para ESP32 (2) - Recebimento e Acionamento do LED

   ```bash
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
   ```

## Configuração do Node-RED

- O fluxo do Node-RED está disponível no arquivo **node-red-fluxo.json**.

### Como Importar o Fluxo

1. Abra o Node-RED no navegador **(geralmente em http://localhost:1880)**.
2. Clique no menu, selecione **Importar** e cole o conteúdo do arquivo **node-red-fluxo.json**.
3. Ajuste as configurações do MQTT com as suas credenciais, se necessário.

- node-red-fluxo.json.

     ```bash
     [
    {
        "id": "mqtt_in_node",
        "type": "mqtt in",
        "z": "node-red-flow",
        "name": "Receber Temperatura",
        "topic": "sensor/temperatura",
        "qos": "2",
        "datatype": "auto",
        "broker": "mqtt_broker",
        "x": 150,
        "y": 100,
        "wires": [
            [
                "function_node"
            ]
        ]
    },
    {
        "id": "function_node",
        "type": "function",
        "z": "node-red-flow",
        "name": "Verificar Temperatura",
        "func": "var temperatura = parseFloat(msg.payload);\n\nif (temperatura > 30) {\n    msg.payload = \"ON\";\n} else {\n    msg.payload = \"OFF\";\n}\n\nreturn msg;",
        "outputs": 1,
        "noerr": 0,
        "x": 350,
        "y": 100,
        "wires": [
            [
                "mqtt_out_node"
            ]
        ]
    },
    {
        "id": "mqtt_out_node",
        "type": "mqtt out",
        "z": "node-red-flow",
        "name": "Publicar Comando LED",
        "topic": "led/acionar",
        "qos": "",
        "retain": "",
        "broker": "mqtt_broker",
        "x": 550,
        "y": 100,
        "wires": []
    },
    {
        "id": "mqtt_broker",
        "type": "mqtt-broker",
        "name": "HiveMQ Broker",
        "broker": "broker.hivemq.com",
        "port": "1883",
        "clientid": "node-red",
        "usetls": false,
        "compatmode": true,
        "keepalive": "60",
        "cleansession": true,
        "birthTopic": "",
        "birthQos": "0",
        "birthPayload": "",
        "closeTopic": "",
        "closePayload": "",
        "willTopic": "",
        "willQos": "0",
        "willPayload": ""
    }
  ]
   ```

## Tutorial de Configuração do Servidor

1. Configuração do HiveMQ Broker:
   
    - Acesse o site HiveMQ Cloud.
    - Crie uma conta gratuita e configure seu broker MQTT.
    - Use as credenciais fornecidas para configurar o **Node-RED** e os ESP32.
      
2. Instalação do Node-RED:
   
    - Instale o Node-RED no servidor local ou em uma máquina com suporte a Node.js:
      ```bash
      sudo npm install -g --unsafe-perm node-red
      ```
    - Inicie o Node-RED:
      ```bash
      node-red
      ```
    - Acesse o Node-RED no navegador **(http://localhost:1880)** e importe o fluxo.
      
3. Conexão do ESP32 ao HiveMQ:

- Configure o código do ESP32 com o endereço do broker HiveMQ e as credenciais da sua rede Wi-Fi.
- Suba o código para o ESP32 usando o PlatformIO ou a IDE Arduino.

## Conclusão

Este projeto demonstra como utilizar a placa ESP32 e o sensor DHT11 para monitorar a temperatura, processar dados no Node-RED e realizar ações baseadas em limites de temperatura, como acionar um LED. A arquitetura modular facilita a expansão para diferentes sensores e dispositivos IoT.

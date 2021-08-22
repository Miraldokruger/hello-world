

//Libs de WiFi, MQTT e Json
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <WebServer.h>

//Substitua pelo SSID da sua rede
#define SSID "miraldo"

//Substitua pela senha da sua rede
#define PASSWORD "redkruger00"

//Altere estes dados para os obtidos quando se cadastra um novo dispositivo
//no site do Watson IoT
const String ORG = "anx3e9";
const String DEVICE_TYPE = "ESP32";
const String DEVICE_ID = "Mestre";
#define DEVICE_TOKEN "Gado_Rastreamento"

//ID que usaremos para conectar 
const String CLIENT_ID = "d:"+ORG+":"+DEVICE_TYPE+":"+DEVICE_ID;

//Server MQTT que iremos utilizar
const String MQTT_SERVER = ORG + ".messaging.internetofthings.ibmcloud.com";

//Comandos para o Relê 1
#define COMMAND_TOPIC_R1 "iot-2/cmd/commandR1/fmt/json"

//Comandos para o Relê 2
#define COMMAND_TOPIC_R2 "iot-2/cmd/commandR2/fmt/json"


//Cliente WiFi que o MQTT irá utilizar para se conectar
WiFiClient wifiClient;

//Cliente MQTT, passamos a url do server, a porta
//e o cliente WiFi
PubSubClient client(MQTT_SERVER.c_str(), 1883, wifiClient);
int status = WL_IDLE_STATUS;
const char publishTopic[] = "iot-2/evt/status/fmt/json";

void setup() {
   

Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);

    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

   connectMQTTServer();

}

void loop() {
  // put your main code here, to run repeatedly:
    //Verifica se existe alguma mensagem em algum tópico
    //que seguimos
    client.loop();

    delay(100);
    String payload = "{\"d\":";
payload += "mensagem";
payload += "}";


if (client.publish(publishTopic, (char *)payload.c_str())) 
Serial.println("Publish ok");

delay(600000);
}


//Função responsável por conectar ao server MQTT
void connectMQTTServer() {
  Serial.println("Connecting to MQTT Server...");
  //Se conecta com as credenciais obtidas no site do Watson IoT
  //quando cadastramos um novo device
  if (client.connect(CLIENT_ID.c_str(), "use-token-auth", DEVICE_TOKEN)) {
    //Se a conexão foi bem sucedida
    Serial.println("Connected to MQTT Broker");
    //Quando algo for postado em algum tópico que estamos inscritos
    //a função "callback" será executada
    client.setCallback(callback);
    //Se inscreve nos tópicos de interesse
    client.subscribe(COMMAND_TOPIC_R1);
    client.subscribe(COMMAND_TOPIC_R2);
    
  } else {
    //Se ocorreu algum erro
    Serial.print("error = ");
    Serial.println(client.state());
    connectMQTTServer(); //tenta conectar novamente
  }
  
}


void callback(char* topic, unsigned char* payload, unsigned int length) {
    Serial.print("topic ");
    Serial.println(topic);
    //Faz o parse do payload para um objeto json
    StaticJsonBuffer<30> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);

    //Se não conseguiu fazer o parser
    if(!root.success())
    {
        //Exibe mensagem de erro
        Serial.println("Json Parse Error");
        return;
    }

    //Recupera o atributo "value" do json
    int value = root["value"];

    //Verifica se a string topic é igual ao que está em  COMMAND_TOPIC_R1
    if (strcmp(topic, COMMAND_TOPIC_R1) == 0)
    {
        //Modifica o valor de saída do pino onde está o relê 1 para o que nos foi enviado
        Serial.println("R1_PIN: " + value);
    }
    //senão, como estamos inscritos em apenas 2 tópicos, significa que é do tópico que sobrou
    else
    {
        //Modifica o valor de saída do pino one está o relê 2 para o que nos foi enviado
        Serial.println("R2_PIN: " + value);
    }
}





///////////////////////////////////////////////////////////////////////////////////

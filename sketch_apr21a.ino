#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>


EspSoftwareSerial::UART nucleoSerial;

const char* ssid = "Al1"; // change to your wifi name
const char* password = "just317balk"; // change to your password

const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;

const char* mqtt_Client = "6df8dd31-9065-485f-9361-1f2991420729";
const char* mqtt_username = "xC6993zTngx9mNnQ4TMCtW6hNULZFWKB";
const char* mqtt_password = "jWflrPNC#Z5JKmk*2GKKLmbx4Y$-Sm~h";

char necleo_data[8];

WiFiClient espClient;
PubSubClient client(espClient);
char msg[100];

void reconnect() {
  while (!client.connected()) {
    Serial.print("Sensor MQTT connection…");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("Netpie connected");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to "); 
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, mqtt_port);

  nucleoSerial.begin(115200, EspSoftwareSerial::SWSERIAL_8N1, D6, D5);
}

void receiveData(){
  Serial.println("get data...");
  nucleoSerial.listen();
  nucleoSerial.write('a');
  int x = 0;
  delay(50);
  while (nucleoSerial.available() > 0) {
    necleo_data[x] = nucleoSerial.read();
    x = (++x)%8;
  }
}

float recieveTemp() {
  // temp in .2f
  return 1.0*((int)necleo_data[2]) + ((((int)necleo_data[3])*1.0)/100);
}

float recieveHumd() {
  // humid in % .2f
  return ((int)necleo_data[4]) + ((((int)necleo_data[5])*1.0)/100);
}

float recieveHeight() {
  //height in cm .1f
  return ((int)necleo_data[0]) + ((((int)necleo_data[1])*1.0)/100);
}

float recieveLight() {
  // light in % 
  return ((int)necleo_data[6]) + ((((int)necleo_data[7])*1.0)/100);
}

void loop() {
    // recieve some value here
    receiveData();
    float temperature = recieveTemp();
    float humidity = recieveHumd();
    float height = recieveHeight();
    float light = recieveLight();

    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    // format to JSON
    String data = 
    "{\"data\": {\"temperature\":" + String(temperature) + 
    ", \"humidity\":" + String(humidity) +
     ", \"height\":" + String(height) +
      ", \"light\":" + String(light) + "}}";

    Serial.println(data);
    data.toCharArray(msg, (data.length() + 1));

    // send to Gavin's Netpie
    client.publish("@shadow/data/update", msg);
    delay(2000);
}

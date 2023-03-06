#include <PubSubClient.h>
#include <Wire.h>
#include <WiFi.h>
#include "ThingSpeak.h"

const char* ssid = "wifi free";// ssid and password wifi
const char* password = "Khongchodau";

const char* mqttServer = "192.168.1.34";//ID local host
const int mqttPort = 1883;//default port
const char* mqttUser = "KhaiGau";
const char* mqttPassword = "29032001";

WiFiClient espClient;
PubSubClient client(espClient);


unsigned long myChannelNumber = 2053604;//IDchannel and WriteAPIKey to push on thingspeak 
const char* myWriteAPIKey = "RMB0AI52RRQFKPPK";

int dem2 = 0;
WiFiClient client2;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600); //uart2 esp32
  setup_wifi();
  ThingSpeak.begin(client2);  // Initialize ThingSpeak
  client.setServer(mqttServer, mqttPort);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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
}

void reconnect() {
  client.setServer(mqttServer, mqttPort);
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

unsigned long t1 = 0;
int dem = 0;
int gtcambien = 0;

struct pms7003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};

struct pms7003data data;


void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (readPMSdata(&Serial2)) {
    // reading data was successful!
    Serial.println();
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (standard)");
    //Serial.print("PM 1.0: "); Serial.print(data.pm10_standard);
    Serial.print("\t\tPM 2.5: ");
    Serial.print(data.pm25_standard);
    //Serial.print("\t\tPM 10: "); Serial.println(data.pm100_standard);
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (environmental)");
    //Serial.print("PM 1.0: "); Serial.print(data.pm10_env);
    Serial.print("\t\tPM 2.5: ");
    Serial.print(data.pm25_env);
  }




  if (millis() - t1 > 1000) {
    dem++;
    dem2++;

    if (dem == 3) {
      String s = String(data.pm25_env);
      client.publish("cambien", s.c_str());
      dem = 0;
    }

    if (dem2 == 20) {
      int x = ThingSpeak.writeField(myChannelNumber, 1, data.pm25_env, myWriteAPIKey);
      if (x == 200) {
        Serial.println("Channel update successful.");
      } else {
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }
      dem2 = 0;
    }

    t1 = millis();
  }
}


boolean readPMSdata(Stream* s) {
  if (!s->available()) {
    return false;
  }

  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }

  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }

  uint8_t buffer[32];
  uint16_t sum = 0;
  s->readBytes(buffer, 32);

  // get checksum ready
  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
  }

  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }

  // put it into a nice struct :)
  memcpy((void*)&data, (void*)buffer_u16, 30);

  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}

#define BLYNK_TEMPLATE_ID "TMPL6sU_mp0Hg"
#define BLYNK_TEMPLATE_NAME "Test Blynk"
#define BLYNK_AUTH_TOKEN "S4doE35XQWDS2Tttrp-mGfK8kS6TE8t4"
#include "ESP8266WiFi.h"
#include <PubSubClient.h>
#include <DHT.h>
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>

// Your WiFi credentials.
// Set password to "" for open networks.
const char* ssid = "NOPIXELS";
const char* password = "99999999";
const char* mqtt_server = "test.mosquitto.org"; //Initial MQTT broker

WiFiClient espClient; //Object
PubSubClient client(espClient); //Object
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

float tempReceived;
char led; 

// MQTT SETUP/////////////////////////////////////////////////////////////////////////////////////////
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to Wi-Fi");

  WiFi.mode(WIFI_STA); // Mode user - standalone
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  tempReceived = atof((char*)payload);
  Serial.print("Temp Received: ");
  Serial.print(tempReceived);
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish("Control", "hello IoT Node...");
      client.subscribe("Sensor");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
// BLYNK SETUP////////////////////////////////////////////////////////////////////////////////////////////////////
BlynkTimer timer;

void myTimer() 
{
  Blynk.virtualWrite(V0, tempReceived);  
}

BLYNK_WRITE(V1) // Executes when the value of virtual pin 0 changes
{
  if(param.asInt() == 1)
  {
    led = 1;
  }
  else
  {
    led = 0;
  }
}

void setup() {
  Serial.begin(9600);

  setup_wifi();
  client.setServer(mqtt_server, 1883); // default port
  client.setCallback(callback);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  timer.setInterval(1000L, myTimer);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long now = millis();

  if (now - lastMsg > 2000) {
    lastMsg = now;

    char get[20]; 
    dtostrf(led, 4, 2, get); 

    snprintf (msg, MSG_BUFFER_SIZE, get); //merge temp(string type) to msg
    Serial.print("Publish message - Led mode: ");
    Serial.println(msg);
    client.publish("Control", msg);
  }

  Serial.println();
  Blynk.run();
  timer.run();
  delay(2000);
}


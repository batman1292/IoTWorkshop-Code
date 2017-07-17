#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include "Adafruit_NeoPixel.h"
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Update these with values suitable for your network.
const char* ssid = "ESL_Lab1";
const char* password = "wifi@esl";

// Config MQTT Server
#define mqtt_server "m12.cloudmqtt.com"
#define mqtt_port 11419
#define mqtt_user "esp8266_2"
#define mqtt_password "asdf1234"

int esp_id = 2;

WiFiClient espClient;
PubSubClient client(espClient);

// On a Trinket or Gemma we suggest changing this to 1
#define PIN            5

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      1
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
  
  Serial.begin(115200);
  delay(10);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  pixels.setPixelColor(0, pixels.Color(127, 127, 127)); // Moderately bright green color.

  pixels.show(); // This sends the updated pixel color to the hardware.
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

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
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    String clientId = "ESP8266Client-"+String(esp_id);
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      char inTopic[64];
      sprintf(inTopic, "/esp8266/%d/+", esp_id);
      Serial.println("connected");
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  const char s[2] = "/";
  char *token;
  token = strtok(topic, s);
  token = strtok(NULL, s);
  token = strtok(NULL, s);
  Serial.println(token);
  Serial.write(payload, length);
  Serial.println("");
  if(!strcmp(token, "led")){
    setWS2812(payload, length);
  }
}

void setWS2812(byte* payload, unsigned int length){
//  Serial.write(payload, length);
//  Serial.println(" ");
  char hexval[2];
  char *ptr;

  hexval[0] = payload[1];
  hexval[1] = payload[2];
  int red_val = strtol(hexval, &ptr, 16);
  
  hexval[0] = payload[3];
  hexval[1] = payload[4];
  int green_val = strtol(hexval, &ptr, 16);

  hexval[0] = payload[5];
  hexval[1] = payload[6];
  int blue_val = strtol(hexval, &ptr, 16);
  
  pixels.setPixelColor(0, pixels.Color(red_val, green_val, blue_val)); // Moderately bright green color.

  pixels.show(); // This sends the updated pixel color to the hardware.
}


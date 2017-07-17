#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include "DHT.h"

#define DHTPIN D4     // what pin we're connected to
#define wifi_ssid "ESL_Lab1"
#define wifi_password "wifi@esl"

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Config MQTT Server
#define mqtt_server "m13.cloudmqtt.com"
#define mqtt_port 13437
#define mqtt_user "esp8266-1"
#define mqtt_password "12345678"

int esp_id = 1;

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("DHTxx test!");
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  dht.begin();
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

long lastMsg = 0;
float temp = 0.0;
float hum = 0.0;
float diff = 1.0;

void loop() {
  // Wait a few seconds between measurements.
 // delay(10000);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    char temperature_topic[64];
    sprintf(temperature_topic, "/esp8266/%d/temperature", esp_id);
    Serial.println(temperature_topic);
    char humidity_topic[64];
    sprintf(humidity_topic, "/esp8266/%d/humidity", esp_id);
    Serial.println(humidity_topic);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float newHum = dht.readHumidity();
    float newTemp = dht.readTemperature();
    Serial.print("New temperature:");
    Serial.println(String(newTemp).c_str());
    client.publish(temperature_topic, String(newTemp).c_str(), true);
    Serial.print("New humidity:");
    Serial.println(String(newHum).c_str());
    client.publish(humidity_topic, String(newHum).c_str(), true);
 }
}
#include <ESP8266WiFi.h>
#include "PubSubClient.h"

#define wifi_ssid "ECC_IoTWorkshop"
#define wifi_password "iotworkshop@ecc"

// User-defined pin definitions for ESP8266 Board: WeMos D1 mini
#define TRIG_PIN   5    // GPIO-5 / D1 pin
#define ECHO_PIN   4    // GPIO-4 / D2 pin

// Config MQTT Server
#define mqtt_server "m12.cloudmqtt.com"
#define mqtt_port 11419
#define mqtt_user "esp8266_2"
#define mqtt_password "asdf1234"

int esp_id = 2;

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

WiFiClient espClient;
PubSubClient client(espClient);

const uint32_t timeout_usec = 40000;     // timeout in microseconds
const uint32_t sound_speed  = 34300;      // centimeters/second

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
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

uint32_t read_distance() {
   // send a PING signal (a short-pulse signal on TRIG pin)
   digitalWrite( TRIG_PIN, HIGH );
   delayMicroseconds( 20 );
   digitalWrite( TRIG_PIN, LOW );
   // see: https://www.arduino.cc/en/Reference/pulseIn
   // measure pulse width of the ECHO signal
   uint32_t duration = pulseIn( ECHO_PIN, HIGH, timeout_usec );
   uint32_t distance_cm = (sound_speed * duration / 1000000) / 2;
   return distance_cm;
}

#define NUM_READINGS  (4)

void loop() {
  // Wait a few seconds between measurements.
 // delay(10000);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 500) {
    lastMsg = now;
    
    char distance[64];
    sprintf(distance, "/esp8266/%d/distance", esp_id);
    Serial.println(distance);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    uint32_t newDistance = read_distance();
    Serial.print("New distance:");
    Serial.println(String(newDistance).c_str());
    
    client.publish(distance, String(newDistance).c_str(), true);
 }
}

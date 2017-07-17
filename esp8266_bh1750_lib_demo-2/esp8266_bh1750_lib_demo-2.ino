#include <Wire.h> // use the Wire library
#include "BH1750.h"            // https://github.com/claws/BH1750
#include <ESP8266WiFi.h>
#include "PubSubClient.h"

#define I2C_SCL_PIN       (4)       // D2 pin (SCL / GPIO-4)
#define I2C_SDA_PIN       (0)       // D3 pin (SDA / GPIO-0)
#define I2C_BH1750_ADDR       (0x23)

BH1750 bh( I2C_BH1750_ADDR );

char sbuf[64];
uint32_t ts;

#define INTERVAL_MSEC  (2000)

void i2c_scan() {
  int count = 0;
  Serial.println( "\n\nScanning I2C slave devices..." );
  for( uint8_t addr=0x01; addr <= 0x7f; addr++ ) {
     Wire.beginTransmission( addr ); 
     if ( Wire.endTransmission() == 0 ) {
       sprintf( sbuf, "I2C device found at 0x%02X.", addr );
       Serial.println( sbuf );
       count++;
    }
  }
  if ( count > 0 ) {
    sprintf( sbuf, "Found %d I2C device(s).\n\n", count );
  } else {
    sprintf( sbuf, "No I2C device found.\n\n" );
  }
  Serial.println( sbuf );
}

inline void i2c_write_byte( uint8_t i2c_addr, uint8_t data ) {
  Wire.beginTransmission( i2c_addr );
  Wire.write( data );
  Wire.endTransmission();
}

#define wifi_ssid "ECC_IoTWorkshop"
#define wifi_password "iotworkshop@ecc"

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

void setup() {
  // put your setup code here, to run once:
  Serial.begin( 115200 );
  Serial.println( "\n\n\n" );

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  
  Wire.begin( I2C_SDA_PIN, I2C_SCL_PIN );
  delay(1000);
  i2c_scan();
  delay(1000);
  
  bh.begin( BH1750_CONTINUOUS_HIGH_RES_MODE, I2C_SDA_PIN, I2C_SCL_PIN, 400000 );

  Serial.print( "Light Intensity" );
  ts = millis();
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

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if ( millis() - ts >= INTERVAL_MSEC ) {
     char val_str[8];
     ts += INTERVAL_MSEC;

     uint16_t lux = bh.readLightLevel(); 
     dtostrf( lux, 6, 1, val_str );

     char light_topic[64];
     sprintf(light_topic, "/esp8266/%d/light", esp_id);
     Serial.println(light_topic);
     sprintf( sbuf, "    %s Lux", val_str );
     //lcd.setCursor(0 /*col*/, 1 /*row*/); 
     Serial.println( sbuf );
     client.publish(light_topic, val_str, true);
  }
  delay(1);
}

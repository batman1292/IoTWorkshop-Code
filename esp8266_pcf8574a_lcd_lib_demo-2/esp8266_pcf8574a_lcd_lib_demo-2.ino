//////////////////////////////////////////////////////////////////////////
// Author: RSP @ Embedded Systems Lab (ESL), KMUTNB, Bangkok / Thailand
// Date: 2017-07-01
// Arduino IDE: v1.8.2 + esp8266 v2.3.0
// Boards with ESP-12E
// Note: use Vcc=3.3V for both 16x2 LCD module and PCF8574(A) adapter !!!
//////////////////////////////////////////////////////////////////////////

#include <ESP8266WiFi.h>

#include <Wire.h> // use the Wire library
#include "LiquidCrystal_I2C.h" // -> https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library

#define I2C_SCL_PIN       (4)       // D2 pin (SCL / GPIO-4)
#define I2C_SDA_PIN       (0)       // D3 pin (SDA / GPIO-0)
#define I2C_ADDR          (0x3F)    // set the I2C address for PCF8574 LCD adapter (0x27 or 0x3F)

LiquidCrystal_I2C lcd( I2C_ADDR, 16, 2 ); // 16x2 LCD display, set I2C address

// global variables 
char sbuf[64];   // used for sprintf()
uint32_t ts;     // used to save timestamp value

#define INTERVAL_MSEC  (1000)

#include "PubSubClient.h"

#define wifi_ssid "ECC_IoTWorkshop"
#define wifi_password "iotworkshop@ecc"

///////////////////////////// List MQTT Server and MQTT port ///////////////////////////////////////
//              | NUM 1-5           | NUM 6-10          | NUM 11-15         | NUM 16-20         | //
//  mqtt_server | m12.cloudmqtt.com | m11.cloudmqtt.com | m13.cloudmqtt.com | m12.cloudmqtt.com | //
//  mqtt_port   | 11419             | 19226             | 14939             | 14072             | //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Config MQTT Server
#define mqtt_server "m12.cloudmqtt.com"
#define mqtt_port 11419
#define mqtt_user "esp8266_X"
#define mqtt_password "asdf1234"
int esp_id = X;

// free board url : http://192.168.10.164/IoTWorkshop_Freeboard 

WiFiClient espClient;
PubSubClient client(espClient);

void i2c_scan() {
  int count = 0;
  Serial.println( F("\n\nScanning I2C slave devices...") );
  for( uint8_t addr=0x01; addr <= 0x7f; addr++ ) {
     Wire.beginTransmission( addr ); 
     if ( Wire.endTransmission() == 0 ) {
       sprintf_P( sbuf, PSTR("I2C device found at 0x%02X."), addr );
       Serial.println( sbuf );
       count++;
    }
  }
  if ( count > 0 ) {
    sprintf_P( sbuf, PSTR("Found %d I2C device(s).\n\n"), count );
  } else {
    sprintf_P( sbuf, PSTR("No I2C device found.\n\n") );
  }
  Serial.println( sbuf );
}

inline void i2c_write_byte( uint8_t i2c_addr, uint8_t data ) {
  Wire.beginTransmission( i2c_addr );
  Wire.write( data );
  Wire.endTransmission();
}

void setup() {
  Serial.begin( 115200 );

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  Wire.begin( I2C_SDA_PIN, I2C_SCL_PIN );
  delay(1000);
  i2c_scan();
  delay(1000);
  
  lcd.begin( I2C_SDA_PIN, I2C_SCL_PIN, 400000 );
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0); // set cursor at top-level position on the first row
  lcd.print( F("IoT Workshop") );
  delay(1000);
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

String text = "IoT Workshop";
int count_scroll = 0;

void loop() {
  connectCloudMQTT();
  if( millis() - ts >= INTERVAL_MSEC){
    ts += INTERVAL_MSEC;
    if (count_scroll%(text.length()+16) == 0){
      lcd.clear();
      lcd.setCursor(16,0); // set cursor at top-level position on the first row
      lcd.print(text);
    }else{
      lcd.scrollDisplayLeft();
    }
    count_scroll++;
  }
}

void connectCloudMQTT(){
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
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
      delay(5000);
      return;
    }
  }
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  const char s[2] = "/";
  char *token;
  token = strtok(topic, s);
  token = strtok(NULL, s);
  token = strtok(NULL, s);
  if(!strcmp(token, "text")){
    payload[length] = '\0';
    text = String((char*)payload);
    Serial.println(text);
  }
}
//////////////////////////////////////////////////////////////////////////


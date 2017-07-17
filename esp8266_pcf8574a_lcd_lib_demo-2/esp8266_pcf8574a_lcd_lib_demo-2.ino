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

// Update these with values suitable for your network.
const char* ssid = "ESL_Lab1";
const char* password = "wifi@esl";

// Config MQTT Server
#define mqtt_server "m13.cloudmqtt.com"
#define mqtt_port 13437
#define mqtt_user "esp8266-1"
#define mqtt_password "12345678"

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
  client.setCallback(callback);
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
      Serial.println("connected");
      client.subscribe("/esp8266/1/+");
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
//  Serial.println(token);
//  Serial.write(payload, length);
//  Serial.println("");
  if(!strcmp(token, "text")){
    payload[length] = '\0';
  text = String((char*)payload);
//    text = String((char*)payload, length);
    Serial.println(text);
  }
}
//////////////////////////////////////////////////////////////////////////


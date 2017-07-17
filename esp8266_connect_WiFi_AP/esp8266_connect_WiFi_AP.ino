#include <ESP8266WiFi.h>

#define wifi_ssid "ECC_IoTWorkshop"
#define wifi_password "iotworkshop@ecc"

void setup() {
  Serial.begin(115200);                   // เปิดการใช้งาน Serial Port
  Serial.println("\n\n\n\n");
  setup_wifi();
}

void loop() {}

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

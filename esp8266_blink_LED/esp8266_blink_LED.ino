#define LED_PIN 2      // GPIO 2 or D4 pin (built-in LED on ESP-12E module)

int  state = 0;     

void setup() {
  Serial.begin(115200);   // Open Serial Port
  Serial.println("\n\n\n\n");
  pinMode( LED_PIN, OUTPUT);  // set output mode
}

void loop() {
  Serial.println( state );      // show the current value of the state variable
  digitalWrite( LED_PIN, state );   // update the output using the state variable
  state ^= 1;         // toggle the state variable
  delay(500);       // wait for 500 ms
}


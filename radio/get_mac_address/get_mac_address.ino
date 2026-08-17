#include <WiFi.h>

// Run this on the DRONE receiver ESP32 first, once.
// It prints that board's MAC address, which you then paste into
// the transmitter code (esp_now_transmitter.cpp) so it knows
// where to send joystick data.

void setup() {
  Serial.begin(115200);
  delay(1000);
  WiFi.mode(WIFI_STA);
  Serial.print("This board's MAC address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  // nothing to do
}

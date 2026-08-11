// led_blink.ino
// Simple LED blink test for ESP32 — useful sanity check for board and Serial

#include <Arduino.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2  // ESP32 dev boards commonly use GPIO2 for the onboard LED
#endif

// Blink interval (ms)
const unsigned long BLINK_MS = 500;

void setup() {
  Serial.begin(115200);
  // Allow serial monitor to connect briefly
  delay(50);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("led_blink test starting");
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(BLINK_MS);
  digitalWrite(LED_BUILTIN, LOW);
  delay(BLINK_MS);
}

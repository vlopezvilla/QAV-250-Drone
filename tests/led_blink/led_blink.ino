#include <Arduino.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2  // most ESP32 dev boards use GPIO2 for the onboard LED
#endif

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}
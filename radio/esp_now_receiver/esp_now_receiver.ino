#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

// ============================================================
// SINGLE MOTOR TEST — wireless throttle control via joystick,
// received over ESP-NOW (built-in WiFi, no NRF24L01 needed).
// PROPS OFF. Motor secured/clamped. Keep clear of the motor.
// ============================================================
//
// Wiring reminder: ESC Signal -> ESC_PIN, ESC GND -> ESP32 GND.
// The ESC's VCC/red wire must NOT be connected to the ESP32.
//
// SETUP STEPS:
// 1. Flash THIS file first, open Serial Monitor, note the MAC
//    address it prints — that's what goes into the transmitter's
//    receiverMac[] array.
// 2. Then set up esp_now_transmitter.cpp on the handheld ESP32
//    with that MAC address filled in.
//
// POWER SEQUENCE: disconnect battery during upload (USB only),
// then reconnect the battery afterward for the actual test.

#define ESC_PIN 13  // <-- confirm this matches your ESC signal wiring

struct ControlPacket {
  int16_t throttle;
  int16_t yaw;
  int16_t pitch;
  int16_t roll;
  bool armed;
};

ControlPacket packet;
Servo esc;

const int MIN_THROTTLE = 1000;
const int MAX_THROTTLE = 2000;

unsigned long lastPacketTime = 0;
const unsigned long FAILSAFE_TIMEOUT = 500;  // ms — cut motor if signal is lost this long

// Newer ESP32 Arduino core (3.x, IDF5-based) uses this signature.
// If you're on an older core (2.x) and get an error here instead,
// change the first parameter back to: const uint8_t *mac
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  memcpy(&packet, incomingData, sizeof(packet));
  lastPacketTime = millis();
}

void setup() {
  Serial.begin(115200);

  esc.attach(ESC_PIN, MIN_THROTTLE, MAX_THROTTLE);
  esc.writeMicroseconds(MIN_THROTTLE);

  Serial.println("=================================================");
  Serial.println("SINGLE MOTOR TEST — PROPS OFF, motor secured, stay clear.");
  Serial.println("=================================================");
  Serial.println("Arming ESC. Listen for the arm beep sequence.");
  delay(3000);

  WiFi.mode(WIFI_STA);
  Serial.print("This board's MAC address (put this in the transmitter code): ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed.");
    while (1) delay(1000);
  }

  esp_now_register_recv_cb(onDataRecv);

  packet.throttle = 0;
  packet.armed = false;

  Serial.println("Ready. Waiting for transmitter...");
}

void loop() {
  // FAILSAFE: no packet recently received -> cut motor immediately.
  if (millis() - lastPacketTime > FAILSAFE_TIMEOUT) {
    esc.writeMicroseconds(MIN_THROTTLE);
    return;
  }

  // Not armed, or throttle basically zero -> motor off.
  if (!packet.armed || packet.throttle < 10) {
    esc.writeMicroseconds(MIN_THROTTLE);
    return;
  }

  int t = MIN_THROTTLE + packet.throttle;  // 0-1000 -> 1000-2000us
  t = constrain(t, MIN_THROTTLE, MAX_THROTTLE);
  esc.writeMicroseconds(t);

  Serial.print("Throttle: ");
  Serial.print(t);
  Serial.println(" us");
}

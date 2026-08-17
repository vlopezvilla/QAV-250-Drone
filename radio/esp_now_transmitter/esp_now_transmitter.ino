#include <esp_now.h>
#include <WiFi.h>

// ============================================================
// HANDHELD TRANSMITTER — sends joystick positions to the drone
// over ESP-NOW (built-in WiFi, no NRF24L01 needed).
// ============================================================
//
// SETUP STEPS:
// 1. Flash get_mac_address.cpp to the DRONE receiver ESP32 first.
// 2. Open its Serial Monitor, copy the printed MAC address.
// 3. Paste it into receiverMac[] below, replacing the placeholder.
// 4. Then flash THIS file to the handheld transmitter ESP32.

// Drone receiver's MAC address
uint8_t receiverMac[] = {0x70, 0x4B, 0xCA, 0x4D, 0x81, 0x94};

#define JOY1_X 34  // yaw      (left stick, left/right)
#define JOY1_Y 35  // throttle (left stick, up/down)
#define JOY2_X 32  // roll     (right stick, left/right)
#define JOY2_Y 33  // pitch    (right stick, up/down)

struct ControlPacket {
  int16_t throttle;  // 0 to 1000
  int16_t yaw;        // -500 to 500
  int16_t pitch;      // -500 to 500
  int16_t roll;        // -500 to 500
  bool armed;
};

ControlPacket packet;

// Your throttle joystick is spring-centered, so "not touching it" rests
// at the MIDDLE of the ADC range (~2048), not at 0. Without correcting
// for this, letting go of the stick reads as ~50% throttle instead of 0 —
// which is why the motor was spinning on its own. This maps only the
// upper half of the stick's travel (center -> full up) to 0-1000 throttle,
// and treats center-or-below as 0 (off), with a small deadzone for noise.
const int JOY_CENTER   = 2048;
const int JOY_DEADZONE = 150;

int readThrottle(int rawValue) {
  if (rawValue <= JOY_CENTER + JOY_DEADZONE) {
    return 0;
  }
  int t = map(rawValue, JOY_CENTER + JOY_DEADZONE, 4095, 0, 1000);
  return constrain(t, 0, 1000);
}

// Newer ESP32 Arduino core (3.x, IDF5-based) uses this signature.
// If you're on an older core (2.x) and get an error here instead,
// change the first parameter back to: const uint8_t *mac
void onDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  // Optional: uncomment to debug send failures
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent OK" : "Send FAILED");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed.");
    while (1) delay(1000);
  }

  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer — check receiverMac[] is correct.");
    while (1) delay(1000);
  }

  Serial.println("Transmitter ready. Sending joystick data...");
}

void loop() {
  int rawThrottle = analogRead(JOY1_Y);
  int rawYaw      = analogRead(JOY1_X);
  int rawRoll     = analogRead(JOY2_X);
  int rawPitch    = analogRead(JOY2_Y);

  // ESP32 ADC reads 0-4095 (12-bit)
  packet.throttle = readThrottle(rawThrottle);
  packet.yaw      = map(rawYaw, 0, 4095, -500, 500);
  packet.roll     = map(rawRoll, 0, 4095, -500, 500);
  packet.pitch    = map(rawPitch, 0, 4095, -500, 500);
  packet.armed    = true;  // simple version — tie to a button later if desired

  esp_now_send(receiverMac, (uint8_t *)&packet, sizeof(packet));

  Serial.print("T:"); Serial.print(packet.throttle);
  Serial.print(" Y:"); Serial.print(packet.yaw);
  Serial.print(" P:"); Serial.print(packet.pitch);
  Serial.print(" R:"); Serial.println(packet.roll);

  delay(20);  // ~50Hz update rate
}

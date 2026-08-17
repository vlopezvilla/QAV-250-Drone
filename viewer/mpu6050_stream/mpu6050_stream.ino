#include <Wire.h>

// Streams roll,pitch,yaw as plain CSV lines over Serial at 115200 baud —
// the format the browser drone viewer expects. Close any open Serial
// Monitor before connecting the viewer; only one program can hold the port.
//
// Wiring: VCC -> 3V3, GND -> GND, SDA -> GPIO21, SCL -> GPIO22.
//
// Roll/pitch come from the accelerometer only (no gyro fusion), which is
// fine for a bench demo with slow movement. Yaw stays 0 — the MPU6050 has
// no magnetometer, so it can't sense absolute heading.

const int MPU_ADDR = 0x68;
bool sensorOK = false;

bool probeSensor() {
  Wire.beginTransmission(MPU_ADDR);
  return Wire.endTransmission() == 0;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Wire.begin(21, 22);  // SDA, SCL

  // Diagnostics run once at startup. The viewer ignores any line that
  // isn't exactly three comma-separated numbers, so these are harmless.
  Serial.println("# Probing MPU6050 at 0x68...");

  if (!probeSensor()) {
    Serial.println("# NOT FOUND at 0x68. Scanning the I2C bus...");
    int found = 0;
    for (byte addr = 1; addr < 127; addr++) {
      Wire.beginTransmission(addr);
      if (Wire.endTransmission() == 0) {
        Serial.print("#   device at 0x");
        Serial.println(addr, HEX);
        found++;
      }
    }
    if (found == 0) {
      Serial.println("# No I2C devices at all. Check: VCC on 3V3, GND connected,");
      Serial.println("# SDA on GPIO21, SCL on GPIO22. A loose wire is the usual cause.");
    } else {
      Serial.println("# Found a device at another address — if it's 0x69, the AD0");
      Serial.println("# pin is pulled high. Tie AD0 to GND, or change MPU_ADDR to 0x69.");
    }
    return;  // leaves sensorOK false; loop() will idle instead of spamming zeros
  }

  // Wake the sensor — it boots into sleep mode.
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // PWR_MGMT_1
  Wire.write(0);
  Wire.endTransmission(true);

  sensorOK = true;
  Serial.println("# MPU6050 found and awake. Streaming roll,pitch,yaw...");
}

void loop() {
  if (!sensorOK) {
    Serial.println("# sensor not initialised — fix wiring and reset the board");
    delay(2000);
    return;
  }

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  // ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  if (Wire.available() < 6) {
    Serial.println("# read failed — sensor stopped responding");
    delay(500);
    return;
  }

  // Each read is its own statement on purpose: C++ does not guarantee the
  // evaluation order of `Wire.read() << 8 | Wire.read()`, so combining them
  // in one expression can silently swap the high and low bytes.
  int16_t axh = Wire.read(); int16_t axl = Wire.read();
  int16_t ayh = Wire.read(); int16_t ayl = Wire.read();
  int16_t azh = Wire.read(); int16_t azl = Wire.read();

  int16_t ax = (axh << 8) | axl;
  int16_t ay = (ayh << 8) | ayl;
  int16_t az = (azh << 8) | azl;

  float roll  = atan2((float)ay, (float)az) * 180.0 / PI;
  float pitch = atan2((float)-ax, sqrt((float)ay * ay + (float)az * az)) * 180.0 / PI;
  float yaw   = 0;

  Serial.print(roll, 1);
  Serial.print(",");
  Serial.print(pitch, 1);
  Serial.print(",");
  Serial.println(yaw, 1);

  delay(50);  // ~20 Hz
}

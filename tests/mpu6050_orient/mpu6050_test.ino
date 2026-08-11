#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// I2C pins — matches SDA->D21, SCL->D22 (ESP32 default I2C pins)

Adafruit_MPU6050 mpu;

void setup() {
  // ensure that the serial monitor baud rate matches the one entered here
  Serial.begin(115200);
  while (!Serial) delay(10);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 - check wiring!");
    while (1) delay(10);
  }
  Serial.println("MPU6050 found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Pitch/roll derived from accelerometer (degrees)
  float pitch = atan2(a.acceleration.y,
                       sqrt(a.acceleration.x * a.acceleration.x +
                            a.acceleration.z * a.acceleration.z)) * 180.0 / PI;
  float roll = atan2(-a.acceleration.x, a.acceleration.z) * 180.0 / PI;

  Serial.print("Pitch: ");
  Serial.print(pitch, 1);
  Serial.print("  Roll: ");
  Serial.print(roll, 1);
  Serial.print("  | Accel X:");
  Serial.print(a.acceleration.x, 2);
  Serial.print(" Y:");
  Serial.print(a.acceleration.y, 2);
  Serial.print(" Z:");
  Serial.print(a.acceleration.z, 2);
  Serial.print(" | Gyro X:");
  Serial.print(g.gyro.x, 2);
  Serial.print(" Y:");
  Serial.print(g.gyro.y, 2);
  Serial.print(" Z:");
  Serial.println(g.gyro.z, 2);

  delay(200);
}
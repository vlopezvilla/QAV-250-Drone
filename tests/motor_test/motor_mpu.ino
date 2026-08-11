
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ESP32Servo.h>

// PROPS OFF. Motor secured/clamped. Separate battery for ESC. Common ground required.
// Type any character + Enter at ANY time to immediately cut throttle (kill switch).

// I2C pins for MPU6050 — SDA->D21, SCL->D22
#define SDA_PIN 21
#define SCL_PIN 22

// ESC signal pin
#define ESC_PIN 5

Adafruit_MPU6050 mpu;
Servo esc;

// ---- Complementary filter state ----
float pitch = 0;
unsigned long lastTime = 0;
const float ALPHA = 0.98;

// ---- PID state ----
struct PID {
  float kp, ki, kd;
  float integral = 0;
  float prevError = 0;
};
PID pitchPID = {2.0, 0.0, 0.5};
const float TARGET_PITCH = 0.0;  // level

// ---- Throttle limits ----
const int MIN_THROTTLE   = 1000;
const int MAX_THROTTLE   = 2000;
const int BASE_THROTTLE  = 1150;  // modest spin so correction is visible in both directions
const int THROTTLE_CAP   = 1350;  // safety ceiling — halfway

bool killed = false;

float computePID(PID &pid, float setpoint, float measured, float dt) {
  float error = setpoint - measured;
  pid.integral += error * dt;
  pid.integral = constrain(pid.integral, -100, 100);  // anti-windup
  float derivative = (dt > 0) ? (error - pid.prevError) / dt : 0;
  pid.prevError = error;
  return pid.kp * error + pid.ki * pid.integral + pid.kd * derivative;
}

void checkKillSwitch() {
  if (Serial.available()) {
    while (Serial.available()) Serial.read();
    esc.writeMicroseconds(MIN_THROTTLE);
    Serial.println("KILL SWITCH TRIGGERED — throttle cut to minimum.");
    killed = true;
  }
}

void setup() {
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

  esc.attach(ESC_PIN, MIN_THROTTLE, MAX_THROTTLE);
  Serial.println("Arming ESC. PROPS OFF. Keep clear of the motor.");
  esc.writeMicroseconds(MIN_THROTTLE);
  delay(3000);  // give the ESC time to arm (listen for its beep sequence)

  Serial.println("Armed. Type any character + Enter at any time to cut throttle immediately.");
  Serial.println("Tilt the board to see motor throttle respond.");

  lastTime = micros();
}

void loop() {
  checkKillSwitch();
  if (killed) {
    delay(50);
    return;
  }

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  unsigned long now = micros();
  float dt = (now - lastTime) / 1000000.0;
  lastTime = now;

  // Accelerometer-based pitch
  float accPitch = atan2(a.acceleration.y,
                          sqrt(a.acceleration.x * a.acceleration.x +
                               a.acceleration.z * a.acceleration.z)) * 180.0 / PI;
  float gyroPitchRate = g.gyro.x * 180.0 / PI;

  // Complementary filter — fused stable pitch estimate
  pitch = ALPHA * (pitch + gyroPitchRate * dt) + (1 - ALPHA) * accPitch;

  // PID correction to bring pitch back to level
  float pitchCorrection = computePID(pitchPID, TARGET_PITCH, pitch, dt);

  int throttle = BASE_THROTTLE + (int)pitchCorrection;
  throttle = constrain(throttle, MIN_THROTTLE, THROTTLE_CAP);

  esc.writeMicroseconds(throttle);

  Serial.print("Pitch: ");
  Serial.print(pitch, 1);
  Serial.print("  Correction: ");
  Serial.print(pitchCorrection, 1);
  Serial.print("  Throttle: ");
  Serial.println(throttle);

  delay(10);  // ~100Hz loop
}
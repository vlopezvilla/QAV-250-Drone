#include <ESP32Servo.h>

// ============================================================
// ESC MOTOR TEST — SAFETY FIRST
// PROPS OFF. Motor secured/clamped down. Keep clear of the motor.
// Common ground required between ESP32 and ESC.
// ============================================================
//
// POWER SEQUENCE — do not skip this:
// 1. Disconnect the LiPo battery from the PDB.
// 2. Plug in USB-C, upload this code.
// 3. Once upload finishes, keep USB-C connected (you need it for
//    the Serial Monitor) and now connect the LiPo battery to the PDB.
// 4. Never have USB and the battery BOTH connected during an upload —
//    only during normal testing/running, after the upload is done.
//
// WHY THIS VERSION IS SAFER:
// - The kill switch triggers on ANY byte received, instantly — even
//   line noise or a stray character will cut throttle. Fail-safe by
//   design: when in doubt, it cuts power, never guesses "maybe that
//   wasn't real."
// - Arming and starting the ramp instead require typing an EXACT word
//   ("ARM" / "GO") followed by Enter. This avoids a spurious noise
//   byte (electrical interference, a bad USB cable, etc.) accidentally
//   being misread as "yes, go ahead" — the two most consequential
//   actions in this program require deliberate, specific input.
// - Every state change prints a clear, distinct confirmation line so
//   you never have to guess whether input registered.

#define ESC_PIN 13  // <-- CONFIRM this matches the GPIO you physically
                    //     soldered the ESC's ORANGE signal wire to.
                    //     (Brown = GND, Red = do not connect.)

Servo esc;

const int MIN_THROTTLE = 1000;  // us — zero throttle / arm signal
const int MAX_THROTTLE = 2000;  // us — full throttle (do not exceed)
const int TEST_MAX     = 1500;  // us — halfway throttle (midpoint of 1000-2000 range)
const int STEP         = 5;     // us increment per step
const int STEP_DELAY   = 100;   // ms between steps

volatile bool killed = false;

// Cuts throttle immediately on ANY received byte. Deliberately not
// picky — a kill switch should never hesitate to decide "was that real?"
void checkKillSwitch() {
  if (Serial.available()) {
    while (Serial.available()) Serial.read();  // clear buffer
    esc.writeMicroseconds(MIN_THROTTLE);
    if (!killed) {
      Serial.println("*** KILL SWITCH TRIGGERED — throttle cut to minimum. ***");
    }
    killed = true;
  }
}

// Blocks until the user types the exact expected word + Enter.
// Anything else is rejected and re-prompted — reduces the chance
// that noise or a stray keystroke accidentally arms/starts the motor.
void waitForExactWord(const char* word, const char* prompt) {
  Serial.print(prompt);
  Serial.print(" (type '");
  Serial.print(word);
  Serial.println("' and press Enter)");

  String line = "";
  unsigned long lastHeartbeat = millis();

  while (true) {
    checkKillSwitch();
    if (killed) return;

    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        if (line.length() > 0) {
          line.trim();
          if (line.equalsIgnoreCase(word)) {
            Serial.print(">>> '");
            Serial.print(word);
            Serial.println("' RECEIVED. Continuing... <<<");
            return;
          } else {
            Serial.print("Ignored — got '");
            Serial.print(line);
            Serial.print("', expected '");
            Serial.print(word);
            Serial.println("'.");
            line = "";
          }
        }
      } else {
        line += c;
      }
    }

    // Heartbeat every 5s so it's clear the program is alive, not frozen.
    if (millis() - lastHeartbeat > 5000) {
      Serial.println("(still waiting...)");
      lastHeartbeat = millis();
    }
    delay(10);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  esc.attach(ESC_PIN, MIN_THROTTLE, MAX_THROTTLE);

  Serial.println("=================================================");
  Serial.println("ESC MOTOR TEST — PROPS OFF, motor secured, stay clear.");
  Serial.println("=================================================");

  // Hold minimum throttle immediately so the ESC has a valid signal
  // present as soon as it powers up.
  esc.writeMicroseconds(MIN_THROTTLE);

  waitForExactWord("ARM", "Ready to arm the ESC?");
  if (killed) return;

  Serial.println("Arming ESC. Listen for the arm beep sequence.");
  delay(3000);  // give the ESC time to arm
  Serial.println("If you heard the arm tone (and it went silent), the ESC is armed.");

  waitForExactWord("GO", "Ready to begin the throttle ramp?");
  if (killed) return;

  Serial.println("Ramping up...");
  for (int t = MIN_THROTTLE; t <= TEST_MAX && !killed; t += STEP) {
    checkKillSwitch();
    if (killed) break;
    esc.writeMicroseconds(t);
    int pct = map(t, MIN_THROTTLE, MAX_THROTTLE, 0, 100);
    Serial.print("Throttle: ");
    Serial.print(t);
    Serial.print(" us  (");
    Serial.print(pct);
    Serial.println("%)");
    delay(STEP_DELAY);
  }

  if (!killed) {
    Serial.println("Reached test max. Holding 2s...");
    delay(2000);

    Serial.println("Ramping down...");
    for (int t = TEST_MAX; t >= MIN_THROTTLE; t -= STEP) {
      checkKillSwitch();
      if (killed) break;
      esc.writeMicroseconds(t);
      int pct = map(t, MIN_THROTTLE, MAX_THROTTLE, 0, 100);
      Serial.print("Throttle: ");
      Serial.print(t);
      Serial.print(" us  (");
      Serial.print(pct);
      Serial.println("%)");
      delay(STEP_DELAY);
    }
  }

  esc.writeMicroseconds(MIN_THROTTLE);
  if (killed) {
    Serial.println("Test stopped by kill switch. Throttle at minimum.");
  } else {
    Serial.println("Test complete. Throttle at minimum.");
  }
}

void loop() {
  // Kill switch stays live even after the ramp finishes.
  checkKillSwitch();
  delay(50);
}

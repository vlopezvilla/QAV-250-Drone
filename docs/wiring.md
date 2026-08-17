# Wiring reference

## Drone ESP32

| Signal | ESP32 pin |
| ------ | --------- |
| ESC 1 signal | GPIO13 |
| ESC 2 signal | GPIO14 |
| ESC 3 signal | GPIO25 |
| ESC 4 signal | GPIO26 |
| MPU6050 SDA | GPIO21 |
| MPU6050 SCL | GPIO22 |
| MPU6050 VCC | 3V3 |
| Power in | PDB 5V pad → VIN |
| Ground | PDB GND → GND |

A 470µF electrolytic capacitor goes across VIN and GND, soldered as close to
the ESP32's power pins as possible. It buffers the inrush when the LiPo
connector is first seated, which otherwise browns the board out and triggers a
panic reset.

### ESC servo connector

The three-wire connector carries **signal / 5V / ground**, usually orange,
red and brown.

| Wire | Goes to |
| ---- | ------- |
| Orange (signal) | assigned GPIO |
| Brown (ground) | ESP32 GND |
| Red (5V) | **nothing — leave disconnected** |

The red wire is the ESC's own BEC *output*, not an input. The board is already
powered from the PDB, so connecting it puts a second 5V source on the rail.
Connecting it to the 3V3 pin is worse: that pin is the output of the ESP32's
internal regulator, and backfeeding 5V into it overvolts the logic core.

## Transmitter ESP32

| Signal | ESP32 pin |
| ------ | --------- |
| Left stick VRy (throttle) | GPIO35 |
| Left stick VRx (yaw) | GPIO34 |
| Right stick VRy (pitch) | GPIO33 |
| Right stick VRx (roll) | GPIO32 |
| Both joysticks VCC | 3V3 |
| Both joysticks GND | GND |
| SW (stick button) | unused |

Joystick modules are silkscreened "5V" because they're usually paired with an
Arduino Uno. Power them from **3V3** anyway — the analog output swings up to
whatever the supply rail is, and the ESP32's ADC tops out at 3.3V.

## Pin choices to avoid

GPIO2 is a boot-strapping pin: its level at power-up selects the boot mode, so
driving it from a peripheral causes intermittent boot failures. GPIO6–11 are
wired to the internal flash and aren't broken out. GPIO34–39 are input-only,
which is fine for the joystick axes but not for anything driven.

## Physical stack

Bottom to top:

1. **PDB / power distribution** — battery in, ESC power out, closest to the arms
2. **MPU6050** — on a vibration-isolated mount (foam tape or rubber grommets),
   as close to the frame's centre of gravity as the layout allows, X-axis
   aligned with the nose
3. **ESP32 protoboard** — short I2C run down to the IMU, USB port facing an
   open edge so it can be reflashed without unstacking

The battery straps underneath the bottom plate, centred, keeping the centre of
gravity below the plane of the motors.

## Power-up order

Flash over USB with the **LiPo disconnected**. Having both sources live during
an upload makes the flash handshake fail with `No serial data received`.
Reconnect the battery afterwards for bench testing — that combination is fine
once the board is already running.

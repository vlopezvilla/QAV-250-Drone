# QAV250 Drone Project

A QAV250 quadcopter running an ESP32 as the flight controller, with a second
ESP32 as a handheld transmitter. The two boards talk over **ESP-NOW**, the
ESP32's built-in peer-to-peer WiFi mode, so no separate radio module is needed.

## Hardware

| Part | Notes |
| ---- | ----- |
| ESP32 DevKit (WROOM-32) | ×2 — one on the drone, one in the controller |
| MPU6050 (GY-521) | Gyro + accelerometer over I2C |
| 40A ESCs | ×4, PWM signal from the ESP32 |
| Matek PDB | Battery distribution with an onboard 5V BEC |
| 3S 2250mAh LiPo | 11.1V nominal |
| Dual analog joysticks | ×2 on the transmitter, 4 axes total |

Pinouts, the ESC connector caveat and the physical stack layout are in
[`docs/wiring.md`](docs/wiring.md).

## Repository structure

```
QAV250-drone-project/
├── README.md            — project overview (this file)
├── PROGRESS.md          — date-stamped progress notes and milestones
├── docs/
│   └── wiring.md        — pinouts, power path, stack order, safety
├── tests/               — bench tests, one sketch per folder
│   ├── led_blink/       — LED blink sanity check
│   ├── motor_basic/     — single ESC ramp test, typed ARM/GO + kill switch
│   ├── motor_mpu/       — motor + MPU6050 bench test (complementary filter, PID)
│   └── mpu6050_orient/  — MPU6050 pitch/roll readout
├── radio/               — ESP-NOW link between the two boards
│   ├── get_mac_address/ — prints a board's MAC, needed for pairing
│   ├── esp_now_transmitter/ — reads joysticks, broadcasts a control packet
│   └── esp_now_receiver/    — receives the packet, drives one ESC
└── viewer/
    ├── drone_viewer.html    — live 3D orientation viewer in the browser
    └── mpu6050_stream/      — streams roll,pitch,yaw as CSV for the viewer
```

Each sketch lives in its own folder named after the `.ino` file — the Arduino
IDE requires this, and it concatenates *every* `.ino` in a folder into one
translation unit, so two sketches sharing a folder collide on `setup()`.

## Bring-up order

Each step depends on the one before it, so work down the list rather than
wiring everything and testing at the end.

1. `tests/led_blink` — confirms the board, the USB cable and the toolchain
2. `tests/mpu6050_orient` — confirms I2C wiring; the MPU6050 answers at `0x68`
3. `tests/motor_basic` — one ESC, props off, motor clamped
4. `radio/get_mac_address` on the drone board, then the transmitter/receiver pair
5. `viewer/` — visual check that the IMU reads orientation correctly

## Orientation viewer

`viewer/drone_viewer.html` renders a 3D drone in the browser that mirrors the
MPU6050's live orientation, reading the serial port through the Web Serial API.

1. Flash `viewer/mpu6050_stream`
2. Close the Arduino serial monitor — only one program can hold the port
3. Open the page in **Chrome or Edge** and click **Connect ESP32**

Sliders drive the model manually with no hardware attached. Drag the canvas to
orbit, scroll to zoom.

Web Serial needs HTTPS or localhost. Opening the file directly works, and so
does GitHub Pages; a plain `http://` host will not.

## Radio link

ESP-NOW pairs by MAC address:

1. Flash `radio/get_mac_address` to the **drone** board and read its MAC from
   the serial monitor
2. Paste it into `receiverMac[]` in `radio/esp_now_transmitter`
3. Flash the transmitter to the handheld board and the receiver to the drone

The receiver cuts throttle if no packet arrives for 500ms, so losing the
transmitter stops the motors instead of latching the last command.

## Safety

Props off for all bench testing, and the motor clamped down. Disconnect the
LiPo before flashing over USB.

## Pushing to GitHub

```
git remote add origin <repo-url>
git push -u origin main
```

To host the viewer: Settings → Pages → source `main`, root. It lands at
`https://<username>.github.io/QAV250-drone-project/viewer/drone_viewer.html`.

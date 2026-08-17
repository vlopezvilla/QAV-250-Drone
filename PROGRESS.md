# Progress log

Date | Entry
---- | -----
2026-08-16 | Added ESP-NOW transmitter/receiver pair, MPU6050 CSV stream and the browser orientation viewer. Split `tests/motor_test/` into `motor_basic/` and `motor_mpu/` so each sketch has its own folder. Added `docs/wiring.md`.
2026-08-10 | Repository initialized and scaffold added (README.md, .gitignore, PROGRESS.md).

Milestones
- [ ] Project plan and requirements
- [x] Hardware procurement
- [ ] Flight controller setup
- [ ] Baseline flight test
- [ ] Autonomy stack prototype

Bring-up checklist
- [x] LED blink — board and toolchain confirmed
- [x] MPU6050 responding on I2C at 0x68
- [x] Orientation viewer showing live tilt
- [ ] Single motor ramp test, props off
- [ ] ESP-NOW link between the two boards
- [ ] Joystick throttle driving one motor wirelessly
- [ ] All four motors mixed and responding
- [ ] MPU6050 stabilization loop

Open items
- Confirm the motor that was resoldered is undamaged (resistance across each
  pair of phase wires should be roughly equal, under ~1Ω).
- 3.5mm bullet connectors from the current kit fit poorly; check whether it's
  one bad pair or the whole batch before ordering a replacement.
- Yaw is hardcoded to 0 in `mpu6050_stream` — the MPU6050 has no magnetometer,
  so absolute heading needs gyro integration or a compass.
- Roll/pitch are accelerometer-only. Fine for a bench demo, but flight
  stabilization needs a complementary or Kalman filter blending the gyro.

How to update
- Add a new dated entry above.
- Use checklist items to mark milestone progress.

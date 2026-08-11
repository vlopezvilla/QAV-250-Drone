# QAV250 Drone Project

This repository tracks development and progress for the QAV250 drone project.

Repository structure

```
QAV250-drone-project/
├── README.md              — project overview and file structure (this file)
├── PROGRESS.md            — date-stamped progress notes and milestone checklist
├── .gitignore             — files and folders to ignore
└── tests/                 — tests and experiment code
    ├── led_blink/
    │   └── led_blink.ino          — simple LED blink sanity test for ESP32
    ├── motor_test/
    │   ├── motor_basic.ino        — basic motor/ESC spin-up test
    │   └── motor_mpu.ino          — motor + MPU6050 bench test (ESC safety, complementary filter, PID)
    └── mpu6050_orient/
        └── mpu6050_test.ino       — MPU6050 orientation readout (pitch/roll + raw sensors)
```

Getting started
1. Review PROGRESS.md to see current status and next steps.
2. Inspect the test/ folder for example tests or experimental code added so far.
3. Commit progress updates or open issues to track work.

How to push to GitHub
- Create a remote repo on GitHub and add it as a remote:
  git remote add origin <repo-url>
  git push -u origin main

Notes
- Use PROGRESS.md for quick daily notes; create issues for specific tasks.
- Keep test/ focused: use subfolders for unit, integration, and hardware tests.

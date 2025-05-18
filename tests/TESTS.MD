<h1 align="center">🧪 Tests Folder</h1>

<p align="center">
  <img src="../assets/icons/testing.png" alt="Tests" width="200"/>
</p>


This directory contains tests to ensure code reliability and functionality for Embedded Systems and Power Electronics projects.

## ✅ Purpose

- Validate code correctness for various hardware and software components.
- Detect bugs and errors in embedded system and power electronics designs.
- Maintain code quality and ensure proper operation of electronic systems.
---

## 📂 Folder Structure

The `tests` folder is organized into two main categories:

-   **Embedded Systems**: Contains tests related to embedded systems projects.

    -   `COMPLETE`: Contains a completed embedded systems project.
    -   `DUAL-MPU6050`: Contains tests for a dual MPU6050 sensor setup.
    -   `FLEX-SENSORS`: Contains tests for flex sensors.

-   **Power Electronics**: Contains tests related to power electronics projects.

    -   `complete`: Contains a complete power electronics project, the stepper and servo **don't** move together.
    -   `complete-dynamic`: Contains tests for a complete dynamic power electronics system, by dynamic what is meant is both the servo and stepper motors move together. This test code will aid in determining the maximum current draw needed.
    -   `nema_17_test`: Contains tests for NEMA 17 stepper motors.
    -   `servo_test`: Contains tests for servo motors.
---

## ⚙️ Usage

Each subfolder contains an `.ino` file, which can be opened and run using the Arduino IDE. Follow these steps to run the tests:

1.  Open the desired `.ino` file in the Arduino IDE.
2.  Connect the appropriate hardware components to your Arduino board.
3.  Upload the code to the Arduino board.
4.  Monitor the output and verify the results.

---

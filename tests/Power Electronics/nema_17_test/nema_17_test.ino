#include <AccelStepper.h>

const int DIR = 4;  // Direction pin
const int STEP = 2; // Step pin

#define motorInterfaceType 1
AccelStepper motor(motorInterfaceType, STEP, DIR);

int currentSpeed = 200;
int currentAcceleration = 1000;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 2000;

/**
 * @brief Initialize the serial communication at 115200bps and set the
 * stepper motor settings.
 *
 * The stepper motor is set to a maximum speed of 32000 steps/s, an
 * acceleration of 1000 steps/s^2 and a speed of 200 steps/s. The motor is
 * moved to 6400 steps (a full 360 rotation at 1/32 microstepping).
 */
void setup() {
  Serial.begin(115200);
  motor.setMaxSpeed(32000);
  motor.setAcceleration(currentAcceleration);
  motor.setSpeed(currentSpeed);
  motor.moveTo(6400); // Move 6400 steps (360 rotation for 1/32 microstepping)
}


/**
 * @brief Continuously manages the stepper motor's movements and updates speed and acceleration at intervals.
 *
 * This function checks if the motor has reached its target position and reverses the direction.
 * It runs the motor, updates speed and acceleration every 2000 milliseconds, and prints the current speed,
 * acceleration, and position to the serial monitor.
 * The speed and acceleration are increased over time, resetting to initial values upon reaching their maximum limits.
 */
void loop() {
  if (motor.distanceToGo() == 0) {
    motor.moveTo(-motor.currentPosition());
  }

  motor.run();

  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate > updateInterval) {
    lastUpdate = currentMillis;

    currentSpeed += 100;
    currentAcceleration += 200;

    if (currentSpeed > 32000) currentSpeed = 200;
    if (currentAcceleration > 8000) currentAcceleration = 1000;

    motor.setSpeed(currentSpeed);
    motor.setAcceleration(currentAcceleration);

    Serial.print("Speed: ");
    Serial.print(currentSpeed);
    Serial.print(" | Acceleration: ");
    Serial.println(currentAcceleration);
  }

  Serial.print("Position: ");
  Serial.println(motor.currentPosition());
}
// Complete Test Code for PE 
// not dynamic first servos then stepper moves

#include <AccelStepper.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Define pins for stepper motor
const int DIR = 4;  // Direction pin
const int STEP = 2; // Step pin

#define motorInterfaceType 1
AccelStepper motor(motorInterfaceType, STEP, DIR);

// Create a PCA9685 object for controlling servos
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Define servo parameters
#define SERVOMIN 150  // Minimum pulse length count
#define SERVOMAX 600  // Maximum pulse length count
#define SERVO_FREQ 50 // Frequency for analog servos (~50 Hz)

void moveServos(int targetPosition, bool isAscending = true); 

/**
 * @brief Initializes the serial communication, I2C bus, PWM servo driver, and stepper motor.
 *
 * - Initializes serial communication with a baud rate of 115200.
 * - Initializes the I2C bus.
 * - Initializes the PWM servo driver and sets the PWM frequency to 50 Hz.
 * - Initializes the stepper motor and sets its maximum speed, acceleration, and initial position to 6400 microsteps.
 */
void setup() {
  Serial.begin(115200);


  motor.setMaxSpeed(32000);
  motor.setAcceleration(1000);  // Set a reasonable acceleration
  motor.setSpeed(1000);         // Set a reasonable speed
  motor.moveTo(6400);           // Full 360° rotation for 1/32 microstepping

  // Initialize the PCA9685 servo driver
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ);
}

/**
 * @brief Continuously moves the servos and stepper motor to demonstrate a complete cycle.
 * 
 * @details
 * - Moves the servos to their maximum position (180°).
 * - Rotates the stepper motor 360° clockwise.
 * - Returns servos to their minimum position (0°) smoothly.
 * - Rotates the stepper motor 360° counterclockwise.
 * - Includes delays to allow time for the motors to complete their movements.
 */
void loop() {
  moveServos(SERVOMAX);
  delay(1000);  

  // Move stepper motor 360° clockwise
  motor.moveTo(6400);  // Move 6400 steps (360° rotation for 1/32 microstepping)
  while (motor.distanceToGo() != 0) {
    motor.run();
  }

  delay(1000);  // Allow time for stepper to complete the movement

  // Move servos back to 0° (smoothly)
  moveServos(SERVOMIN, false);  // false for descending to 0°
  delay(1000);  // Allow time for servos to reach position

  // Move stepper motor 360° counterclockwise
  motor.moveTo(0);  // Move to 0 position (360° counterclockwise)
  while (motor.distanceToGo() != 0) {
    motor.run();
  }

  delay(1000);  // Allow time for stepper to complete the movement
}


/**
 * @brief Move all four servos to the target position.
 * 
 * @param[in] targetPosition Target position for the servos, in the range [SERVOMIN, SERVOMAX].
 * @param[in] isAscending When true, the servos will move from SERVOMIN to SERVOMAX; when false, they will move from SERVOMAX to SERVOMIN.
 */
void moveServos(int targetPosition, bool isAscending) {
  int startPos = isAscending ? SERVOMIN : SERVOMAX;
  int endPos = isAscending ? SERVOMAX : SERVOMIN;
  int step = isAscending ? 1 : -1;

  for (int pulselen = startPos; pulselen != endPos + step; pulselen += step) {
    pwm.setPWM(0, 0, pulselen); // Control servo on channel 0
    pwm.setPWM(1, 0, pulselen); // Control servo on channel 1
    pwm.setPWM(2, 0, pulselen); // Control servo on channel 2
    pwm.setPWM(3, 0, pulselen); // Control servo on channel 3
    delay(10);  // Delay for smooth movement
  }
}

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Create a PCA9685 object
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Define the servo parameters
#define SERVOMIN 150  // Minimum pulse length count
#define SERVOMAX 600  // Maximum pulse length count
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz

/**
 * Initializes the serial communication, I2C bus, and PWM servo driver.
 *
 * - Initializes serial communication with a baud rate of 115200.
 * - Initializes the I2C bus.
 * - Initializes the PWM servo driver and sets the PWM frequency to 50 Hz.
 */
void setup() {
  Serial.begin(115200);
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ); // Set PWM frequency to 50 Hz
}

/**
 * Continuously sweeps servos back and forth across defined positions.
 * 
 * This loop iterates through pulse lengths from SERVOMIN to SERVOMAX,
 * incrementing and then decrementing. It sets the PWM signal for each
 * servo on channels 0 to 3, creating a smooth sweeping motion. Each 
 * pulse length is held for 10 milliseconds to control the speed of 
 * the sweep.
 */
void loop() {
  // Sweep the servo back and forth
  for (int pulselen = SERVOMIN; pulselen <= SERVOMAX; pulselen++) {
    pwm.setPWM(0, 0, pulselen); // Channel 0 controls the servo
    pwm.setPWM(1, 0, pulselen); // Channel 0 controls the servo
    pwm.setPWM(2, 0, pulselen); // Channel 0 controls the servo
    pwm.setPWM(3, 0, pulselen); // Channel 0 controls the servo
    delay(10);
  }
  for (int pulselen = SERVOMAX; pulselen >= SERVOMIN; pulselen--) {
    pwm.setPWM(0, 0, pulselen);
    pwm.setPWM(1, 0, pulselen);
    pwm.setPWM(2, 0, pulselen);
    pwm.setPWM(3, 0, pulselen);
    delay(10);
  }
}
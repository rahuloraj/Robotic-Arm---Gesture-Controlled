#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <AccelStepper.h>

// PWM Servo Driver
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// Servo Channels
// #define BASE_SERVO 0
#define SHOULDER_SERVO 0
#define ELBOW_SERVO 1
#define WRIST_SERVO 2

// Stepper Motor (Base Rotation)
#define STEP_PIN 4
#define DIR_PIN 2
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// Servo Angles
int baseAngle = 0, shoulderAngle = 0, elbowAngle = 0, wristAngle = 0;
bool gripperClosed = false;

// Wi-Fi and MQTT Credentials
const char *ssid = "Your_SSID";
const char *password = "Your_PASSWORD";
const char *mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char *mqttUser = "Your_MQTT_Username";
const char *mqttPassword = "Your_MQTT_Password";

// Led Pins
const int wifiLedPin = 19;  // Wi-Fi LED on GPIO-19
const int mqttLedPin = 18;  // MQTT LED on GPIO-18

float prevAngles[2] = {0, 0};  // 2 joints 
const float MAX_STEP = 3.0;             // Maximum angle change per update (degrees)

// MQTT Client
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Function Prototypes
void updateServos();
void mqttCallback(char *topic, byte *message, unsigned int length);
int angleToPulse(int angle);
void controlStepper(float angle);
void smoothStartToAngle(int channel, int targetAngle, int delayMs = 20);

/**
 * @brief Initializes the serial communication, I2C bus, PWM servo driver, and stepper motor.
 *        Connects to WiFi and MQTT broker.
 * @details
 * - Initializes serial communication with a baud rate of 115200.
 * - Initializes the I2C bus.
 * - Initializes the PWM servo driver and sets the PWM frequency to 50 Hz.
 * - Initializes the stepper motor and sets its maximum speed and acceleration.
 * - Connects to WiFi with the provided ssid and password.
 * - Connects to the MQTT broker at the provided server and port.
 * - Subscribes to the "robot/angles" topic.
 * - Updates the servo angles to the initial values.
 */
void setup()
{
    Serial.begin(115200);
    Wire.begin();

    pinMode(wifiLedPin, OUTPUT);  // Set Wi-Fi LED pin as output
    pinMode(mqttLedPin, OUTPUT);  // Set MQTT LED pin as output
    
    pwm.begin();
    pwm.setPWMFreq(50);

    stepper.setMaxSpeed(3000);
    stepper.setAcceleration(3000);

    smoothStartToAngle(SHOULDER_SERVO,0);
    smoothStartToAngle(ELBOW_SERVO,0);

    WiFi.begin(ssid, password);
    mqttClient.setServer(mqttServer, mqttPort);
    mqttClient.setCallback(mqttCallback);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.println("WiFi connected!");
    updateLedStatus();  // Update LED status after Wi-Fi is connected

    while (!mqttClient.connected())
    {
        Serial.print("Connecting to MQTT...");
        if (mqttClient.connect("ESP32_ACTUATOR", mqttUser, mqttPassword))
        {
            Serial.println("Connected to MQTT.");
            mqttClient.subscribe("robot/angles");
            updateLedStatus();  // Update LED status after MQTT is connected
        }
        else
        {
            Serial.println("Failed. Retrying...");
            delay(2000);
        }
    }
    updateServos();
    updateLedStatus();  // Update LED status
}

/**
 * @brief Main loop of the program.
 *        Checks for any MQTT messages and updates the stepper motor
 *        if a new angle is received.
 */
void loop()
{
    mqttClient.loop();
    stepper.run();
}

/**
 * @brief MQTT callback function.
 *        Updates the servo angles and the stepper motor when an MQTT message is received.
 *        The message should be in the format "SX:<angleServoX>,SY2:<angleServoY2>,ST:<angleStepperY>,GR:<grip>".
 * @param[in] topic MQTT topic of the message.
 * @param[in] message MQTT message payload.
 * @param[in] length Length of the MQTT message payload.
 */
void mqttCallback(char* topic, byte* message, unsigned int length) {
    String data;
    for (unsigned int i = 0; i < length; i++) {
        data += (char)message[i];
    }
    // Print received data for debugging purposes uncomment if needed
    // Serial.println("Received: " + data); 
    
    float angleServoX, angleServoY2, angleStepperY;
    int grip;
    sscanf(data.c_str(), "SX:%f,SY2:%f,ST:%f,GR:%d", &angleServoX, &angleServoY2, &angleStepperY, &grip);

    shoulderAngle = map(angleServoX, -90, 90, 0, 180);
    elbowAngle = map(angleServoY2, -90, 90, 0, 180);
    gripperClosed = grip;

    controlStepper(angleStepperY);
    updateServos();
}

/**
 * @brief Move the stepper motor to a specific angle.
 *        The angle should be between -90 and 90 degrees.
 *        The stepper motor will move to the specified angle and stop.
 * @param[in] angle Angle to move the stepper motor to, in degrees.
 */
void controlStepper(float angle)
{
    stepper.moveTo(map(angle, -90, 90, -3200, 3200));
}


/**
 * @brief Updates the servo motor positions for shoulder, elbow, and wrist joints.
 *        The function ensures smooth transitions by limiting the change in angle
 *        per update to a maximum step size. The wrist servo is controlled based
 *        on the gripper state, either fully open or fully closed.
 * @details
 * - The shoulder and elbow servo angles are updated incrementally to ensure smooth movement.
 * - The delta angle is capped by MAX_STEP to prevent abrupt movements.
 * - The wrist servo is set to 0 degrees if the gripper is closed and 180 degrees if open.
 */
void updateServos()
{
    // Shoulder
    float deltaS = shoulderAngle - prevAngles[0];
    if (abs(deltaS) > MAX_STEP) deltaS = (deltaS > 0) ? MAX_STEP : -MAX_STEP;
    prevAngles[0] += deltaS;
    // Serial.println("Old S: " + String(shoulderAngle) + " New S: " + String(prevAngles[0])); // For debugging uncomment if needed
    pwm.setPWM(SHOULDER_SERVO, 0, angleToPulse(prevAngles[0]));

    // Elbow
    float deltaE = elbowAngle - prevAngles[1];
    if (abs(deltaE) > MAX_STEP) deltaE = (deltaE > 0) ? MAX_STEP : -MAX_STEP;
    prevAngles[1] += deltaE;
    // Serial.println("Old E: " + String(elbowAngle) + " New E: " + String(prevAngles[1])); // For debugging uncomment if needed
    pwm.setPWM(ELBOW_SERVO, 0, angleToPulse(prevAngles[1]));

    pwm.setPWM(WRIST_SERVO, 0, angleToPulse(gripperClosed ? 0 : 180));
}

/**
 * @brief Converts an angle in degrees to a pulse length suitable for controlling a servo motor.
 *        The angle should be in the range of 0-180 degrees.
 *        The pulse length is mapped from the range of 0-180 degrees to the range of 150-600,
 *        which is a good range for most servo motors.
 * @param[in] angle Angle to convert, in degrees.
 * @return Pulse length suitable for controlling a servo motor, in microseconds.
 */
int angleToPulse(int angle)
{
    int pulse = map(angle, 0, 180, 150, 600); // 150-600 is a good range for most servos
    return pulse;
}

/**
 * @brief Updates the status of the Wi-Fi and MQTT LEDs.
 *        The Wi-Fi LED is HIGH if Wi-Fi is connected, LOW otherwise.
 *        The MQTT LED is HIGH if MQTT is connected, LOW otherwise.
 * @details
 * - Uses the digitalWrite() function to set the LED pin to HIGH or LOW.
 * - Wi-Fi status is checked using the WiFi.status() function.
 * - MQTT status is checked using the mqttClient.connected() function.
 */
void updateLedStatus() {
    // Toggle Wi-Fi LED based on Wi-Fi connection status
    digitalWrite(wifiLedPin, (WiFi.status() == WL_CONNECTED) ? HIGH : LOW);

    // Toggle MQTT LED based on MQTT connection status
    digitalWrite(mqttLedPin, (mqttClient.connected()) ? HIGH : LOW);
}


/**
 * @brief Smoothly moves a servo motor to a target angle from an assumed starting angle.
 *        The servo motor will be moved to the target angle in a smooth, incremental manner.
 *        The assumed starting angle is 90 degrees.
 *        The delay between each incremental movement is specified by the delayMs parameter.
 *        The servo motor will move to the specified angle and stop.
 * @param[in] channel PWM channel to control the servo motor.
 * @param[in] targetAngle Target angle to move the servo motor to, in degrees.
 * @param[in] delayMs Delay between each incremental movement, in milliseconds. Default is 20ms.
 */
void smoothStartToAngle(int channel, int targetAngle, int delayMs) {
  int assumedStart = 90;

  // Sweep from assumed position to target
  if (targetAngle < assumedStart) {
    for (int angle = assumedStart; angle >= targetAngle; angle--) {
      pwm.setPWM(channel, 0, angleToPulse(angle));
      delay(delayMs);
    }
  } else {
    for (int angle = assumedStart; angle <= targetAngle; angle++) {
      pwm.setPWM(channel, 0, angleToPulse(angle));
      delay(delayMs);
    }
  }
}
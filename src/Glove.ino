#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <MPU6050.h>

// Wi-Fi and MQTT Credentials
const char *ssid = "Your_SSID";
const char *password = "Your_PASSWORD";
const char *mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char *mqttUser = "Your_MQTT_Username";
const char *mqttPassword = "Your_MQTT_Password";

// MQTT client
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// MPU6050 Sensors (with different I2C addresses)
MPU6050 mpuServo(0x68);   // AD0 = GND default address
MPU6050 mpuStepper(0x69); // AD0 = VCC

// FSR Configuration
#define FSR1_PIN 34     // FSR for Gripper
#define FSR2_PIN 35     // Second FSR for additional grip check
#define Touch_Sensor 18 // Touch Sensor Pin
int fsr1Value, fsr2Value;

// Kalman Filter Variables
float angleServoX, angleServoY2, angleStepperY;
float biasServoX = 0, biasServoY2 = 0, biasStepperY = 0;
float P_ServoX[2][2] = {{1, 0}, {0, 1}};
float P_ServoY2[2][2] = {{1, 0}, {0, 1}};
float P_StepperY[2][2] = {{1, 0}, {0, 1}};
const float kalmanQ = 0.001, kalmanR = 0.03, dt = 0.05; // Smooth update delay
bool gripLatched = false;                               // Latched grip state
bool lastTouchState = LOW;                              // Previous sensor state
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // Adjust as needed

// Function Prototypes
void connectToWiFi();
void ensureWiFiConnection();
void connectToMQTT();
void ensureMQTTConnection();
void kalmanFilter(float &angle, float &bias, float newAngle, float newRate);
void sendAnglesToMQTT(float angleServoX, float angleServoY2, float angleStepperY, bool gripActive);

// Delay for smoother updates
unsigned long previousMillis = 0;
const long interval = 200; // Adjusted update interval for smooth transition

/**
 * @brief Initializes the serial communication, I2C bus, and MPU6050 sensors.
 *        Connects to WiFi and MQTT broker.
 * @details
 * - Initializes serial communication with a baud rate of 115200.
 * - Initializes the I2C bus.
 * - Initializes two MPU6050 sensors and checks for a successful connection.
 *   If either sensor fails to connect, the function enters an infinite loop.
 * - Connects to WiFi with the provided ssid and password.
 * - Connects to the MQTT broker at the provided server and port.
 */
void setup()
{
    Serial.begin(115200);
    Wire.begin();

    // Initialize MPU6050 sensors
    mpuServo.initialize();
    mpuStepper.initialize();

    // Touch sensor as input
    // pinMode(Touch_Sensor, INPUT); // Uncomment if using touch sensor

    if (!mpuServo.testConnection() || !mpuStepper.testConnection())
    {
        Serial.println("MPU6050 initialization failed!");
        while (1)
            ;
    }

    mqttClient.setServer(mqttServer, mqttPort);
    connectToWiFi();
    connectToMQTT();
}

/**
 * @brief Main loop of the program.
 *        Ensures WiFi and MQTT connections are established and maintained.
 *        Reads data from the MPU6050 sensors and the FSR sensors.
 *        Applies Kalman filter to the data for smoother transitions.
 *        Sends the filtered data to the MQTT broker.
 * @details
 * - Ensures WiFi connection with the provided ssid and password.
 * - Ensures MQTT connection with the provided server and port.
 * - Reads data from the MPU6050 sensors for the servos and the stepper motor.
 * - Applies the Kalman filter to the data to reduce noise and jitter.
 * - Reads data from the FSR sensors and Touch sensor to detect grip activity.
 * - Sends the filtered data to the MQTT broker.
 */
void loop()
{
    ensureWiFiConnection();
    ensureMQTTConnection();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;

        // Read MPU6050 data for Servo and Stepper
        int16_t ax, ay, az, gx, gy, gz;
        mpuServo.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        float accelAngleX = atan2(ay, sqrt(ax * ax + az * az)) * 180.0 / M_PI;
        float accelStepperY = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / M_PI;
        float gyroRateX = gx / 131.0, gyroStepperY = gy / 131.0;
        kalmanFilter(angleServoX, biasServoX, accelAngleX, gyroRateX, P_ServoX);          // 1st servo "X" angle
        kalmanFilter(angleStepperY, biasStepperY, accelStepperY, gyroStepperY, P_StepperY); // Stepper "Y" angle

        // Read MPU6050 data for 2nd Servo
        int16_t ax2, ay2, az2, gx2, gy2, gz2;
        mpuStepper.getMotion6(&ax2, &ay2, &az2, &gx2, &gy2, &gz2);
        float accelAngleY2 = atan2(-ax2, sqrt(ax2 * ax2 + az2 * az2)) * 180.0 / M_PI;
        float gyroRateY2 = gy2 / 131.0;
        kalmanFilter(angleServoY2, biasServoY2, accelAngleY2, gyroRateY2, P_ServoY2);

        // Read FSR sensor values
        fsr1Value = analogRead(FSR1_PIN);
        fsr2Value = analogRead(FSR2_PIN);

        // Read touch sensor value. Uncomment lines below if using touch sensor
        // bool reading = digitalRead(Touch_Sensor);

        // if (reading != lastTouchState) {
        //     lastDebounceTime = millis();  // reset debounce timer
        // }

        // if ((millis() - lastDebounceTime) > debounceDelay) {
        //     if (reading == HIGH && lastTouchState == LOW) {
        //         gripLatched = !gripLatched; // toggle grip state only on valid rising edge
        //     }
        // }

        // lastTouchState = reading;

        // Determine grip status
        bool gripActive = (fsr1Value > 200 || fsr2Value > 50 || gripLatched);

        // Send data via MQTT
        sendAnglesToMQTT(angleServoX, angleServoY2, angleStepperY, gripActive);
    }
}

/**
 * @brief Publishes the angles of the robotic arm's servos and
 *        the stepper motor, as well as the grip status, to the
 *        MQTT broker.
 * @param[in] angleServoX Angle of the servo motor controlling
 *                        the base rotation in degrees.
 * @param[in] angleServoY Angle of the servo motor controlling
 *                        the shoulder rotation in degrees.
 * @param[in] angleStepperX Angle of the stepper motor controlling
 *                          the elbow rotation in degrees.
 * @param[in] gripActive true if the grip is active, false otherwise.
 */
void sendAnglesToMQTT(float angleServoX, float angleServoY2, float angleStepperY, bool gripActive)
{
    char msg[50];
    snprintf(msg, sizeof(msg), "SX:%.2f,SY2:%.2f,ST:%.2f,GR:%d", angleServoX, angleServoY2, angleStepperY, gripActive);
    mqttClient.publish("robot/angles", msg);
    
    // Debugging Statements below uncomment to see the output
//     if (!mqttClient.publish("robot/angles", msg)) {
//       Serial.println("Failed to publish angles!");
//   } else {
//       Serial.print("Published angles: ");
//       Serial.println(msg);
//   }
}


/**
 * @brief Applies the Kalman filter to estimate the angle and bias.
 * @param[in,out] angle Reference to the angle estimate, updated with the filtered value.
 * @param[in,out] bias Reference to the bias estimate, updated with the filtered value.
 * @param[in] newAngle The new angle measurement.
 * @param[in] newRate The new rate measurement from the gyroscope.
 * @param[in,out] P The error covariance matrix, updated during the filtering process.
 * @details
 * - The function uses the Kalman filter algorithm to reduce noise in angle and bias readings.
 * - It updates the angle and bias using a prediction-correction model.
 * - The error covariance matrix P is adjusted to reflect the uncertainty in the estimates.
 */
void kalmanFilter(float &angle, float &bias, float newAngle, float newRate, float P[2][2])
{
    float rate = newRate - bias;
    angle += dt * rate;
    P[0][0] += dt * (dt * P[1][1] - P[0][1] - P[1][0] + kalmanQ);
    P[0][1] -= dt * P[1][1];
    P[1][0] -= dt * P[1][1];
    P[1][1] += kalmanQ * dt;
    float S = P[0][0] + kalmanR;
    float K[2] = {P[0][0] / S, P[1][0] / S};
    float y = newAngle - angle;
    angle += K[0] * y;
    bias += K[1] * y;
    P[0][0] -= K[0] * P[0][0];
    P[0][1] -= K[0] * P[0][1];
    P[1][0] -= K[1] * P[0][0];
    P[1][1] -= K[1] * P[0][1];
}

/**
 * @brief Ensures that the MQTT client is connected.
 * @details
 * - Checks if the MQTT client is currently connected to the broker.
 * - If not connected, attempts to establish a connection by calling connectToMQTT().
 */
void ensureMQTTConnection()
{
    if (!mqttClient.connected())
    {
        connectToMQTT();
    }
}

/**
 * @brief Establishes a connection to the MQTT broker.
 * @details
 * - Connects to the MQTT broker using the client ID, username, and password.
 * - If the connection fails, prints the error state and waits for 2 seconds before retrying.
 * - Repeats until the connection is established.
 */
void connectToMQTT()
{
    while (!mqttClient.connected())
    {
        Serial.print("Connecting to MQTT...");
        if (mqttClient.connect("ESP32_DATA_ACQ", mqttUser, mqttPassword))
        {
            Serial.println("Connected to MQTT Broker.");
        }
        else
        {
            Serial.print("Failed. State=");
            Serial.println(mqttClient.state());
            delay(2000);
        }
    }
}

/**
 * @brief Ensures that the Wi-Fi connection is established.
 * @details
 * - Checks if the Wi-Fi connection is currently established.
 * - If not connected, attempts to establish a connection by calling connectToWiFi().
 */
void ensureWiFiConnection()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        connectToWiFi();
    }
}

/**
 * @brief Establishes a connection to the Wi-Fi network.
 * @details
 * - Initiates connection to the Wi-Fi using the provided SSID and password.
 * - Continuously checks the connection status until connected, displaying
 *   progress via serial output.
 * - Prints a confirmation message upon successful connection.
 */
void connectToWiFi()
{
    Serial.print("Connecting to Wi-Fi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi Connected.");
}

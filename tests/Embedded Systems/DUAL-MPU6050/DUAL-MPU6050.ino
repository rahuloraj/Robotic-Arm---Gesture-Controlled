#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>

Adafruit_MPU6050 mpu1;
Adafruit_MPU6050 mpu2;

/**
 * @brief Initialises the two MPU6050 sensors and configures their settings.
 *
 * This function sets up the serial connection, initialises the two MPU6050
 * sensors with their respective I2C addresses, and configures the settings
 * for both sensors.
 *
 * @return void
 */
void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // Wait for the Serial Monitor to open

  // Initialize the first MPU6050 sensor
  if (!mpu1.begin(0x68)) { // Default I2C address is 0x68
    Serial.println("Failed to find MPU6050 #1");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 #1 Found!");

  // Initialize the second MPU6050 sensor
  if (!mpu2.begin(0x69)) { // Alternate I2C address is 0x69
    Serial.println("Failed to find MPU6050 #2");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 #2 Found!");

  // Configure settings for MPU1
  mpu1.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu1.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu1.setFilterBandwidth(MPU6050_BAND_5_HZ);

  // Configure settings for MPU2
  mpu2.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu2.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu2.setFilterBandwidth(MPU6050_BAND_5_HZ);
}

/**
 * @brief The main loop function.
 *
 * This function continuously reads new sensor events from both MPU6050
 * sensors and prints their values to the serial monitor.
 *
 * @return void
 */
void loop() {
  /* Get new sensor events for both sensors */
  sensors_event_t a1, g1, temp1;
  sensors_event_t a2, g2, temp2;

  mpu1.getEvent(&a1, &g1, &temp1);
  mpu2.getEvent(&a2, &g2, &temp2);

  /* Print values from MPU1 */
  Serial.println("MPU6050 #1:");
  Serial.print("Acceleration X: ");
  Serial.print(a1.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a1.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a1.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g1.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g1.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g1.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp1.temperature);
  Serial.println(" degC");
  Serial.println("");

  /* Print values from MPU2 */
  Serial.println("MPU6050 #2:");
  Serial.print("Acceleration X: ");
  Serial.print(a2.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a2.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a2.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g2.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g2.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g2.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp2.temperature);
  Serial.println(" degC");
  Serial.println("");

  delay(500);
}

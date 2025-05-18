// Complete Test Code for EMBDD

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>



// Flex sensor pins
const int flexPin1 = 4;  // First flex sensor connected to GPIO4
const int flexPin2 = 2;  // Second flex sensor connected to GPIO2

// MPU6050 sensors
Adafruit_MPU6050 mpu1;
Adafruit_MPU6050 mpu2;

/**
 * Sets up the serial communication and initializes two MPU6050 sensors.
 * Configures the accelerometer and gyroscope ranges, as well as the filter bandwidth for each sensor.
 * The function waits until a connection to the serial monitor is established.
 * If either MPU6050 sensor is not found, the setup process halts with an error message.
 */
void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10); 


  if (!mpu1.begin(0x68)) { // Default I2C address is 0x68
    Serial.println("Failed to find MPU6050 #1");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 #1 Found!");


  if (!mpu2.begin(0x69)) { // Alternate I2C address is 0x69
    Serial.println("Failed to find MPU6050 #2");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 #2 Found!");


  mpu1.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu1.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu1.setFilterBandwidth(MPU6050_BAND_5_HZ);


  mpu2.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu2.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu2.setFilterBandwidth(MPU6050_BAND_5_HZ);


  analogSetAttenuation(ADC_11db);  
}

/**
 * Reads the values from the flex sensors and prints their values to the serial monitor.
 * Retrieves the sensor events for both MPU6050 sensors and prints the acceleration, rotation, and temperature values for each sensor.
 * Waits for 500 milliseconds before taking the next reading.
 */
void loop() {

  int flexValue1 = analogRead(flexPin1);
  int flexValue2 = analogRead(flexPin2);


  Serial.print("Flex Sensor 1 (GPIO4): ");
  Serial.println(flexValue1);
  Serial.print("Flex Sensor 2 (GPIO2): ");
  Serial.println(flexValue2);

  //new events
  sensors_event_t a1, g1, temp1;
  sensors_event_t a2, g2, temp2;

  mpu1.getEvent(&a1, &g1, &temp1);
  mpu2.getEvent(&a2, &g2, &temp2);

  // 1st MPU
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

  // 2nd MPU
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

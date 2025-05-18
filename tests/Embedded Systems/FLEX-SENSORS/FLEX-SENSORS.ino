const int flexPin1 = 4;  // First flex sensor connected to GPIO4
const int flexPin2 = 2;  // Second flex sensor connected to GPIO2

  /**
   * @brief Initialises the serial port and sets the ADC to a wider input range.
   *
   * Configures the ADC for a wider input range to accomodate the flex sensor's
   * output voltage range. The ADC range is set to 0-3.6V by default. More info
   * can be found at:
   * https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
   */
void setup() {
  Serial.begin(115200); 
  analogSetAttenuation(ADC_11db);  // Configure ADC for a wider input range-> https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
}

  /**
   * @brief Reads the analog values from the flex sensors and prints them to the serial console.
   *
   * The two flex sensors are read in sequence and the values are printed to the serial console
   * with a 100ms delay in between. The values are printed in the range of 0-4095.
   */
void loop() {

  int flexValue1 = analogRead(flexPin1);
  Serial.print("Flex Sensor 1 (GPIO4): ");
  Serial.println(flexValue1);


  int flexValue2 = analogRead(flexPin2);
  Serial.print("Flex Sensor 2 (GPIO2): ");
  Serial.println(flexValue2);

  delay(100);  
}

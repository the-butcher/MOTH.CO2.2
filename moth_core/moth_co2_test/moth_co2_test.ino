#include <SensirionI2CScd4x.h>

SensirionI2CScd4x baseSensor;

void setup() {
  
  Serial.begin(115200);
  Wire.begin();
  delay(2000);  

  baseSensor.begin(Wire);
  baseSensor.startPeriodicMeasurement();

}

void loop() {

  bool isDataReady;
  baseSensor.getDataReadyFlag(isDataReady); 
  if (isDataReady) {
    uint16_t co2;
    float temperature;
    float humidity;
    baseSensor.readMeasurement(co2, temperature, humidity);    

    Serial.print("co2, ");
    Serial.println(co2);
    Serial.print("temperature, ");
    Serial.println(temperature);
    Serial.print("humidity, ");
    Serial.println(humidity);

  } 

  delay(1000);


}

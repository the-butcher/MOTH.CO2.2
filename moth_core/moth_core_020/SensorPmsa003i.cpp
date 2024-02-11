#include "SensorPmsa003i.h"

/**
 * ################################################
 * ## constants
 * ################################################
 */

/**
 * ################################################
 * ## static class variabales
 * ################################################
 */
Adafruit_PM25AQI SensorPmsa003i::baseSensor = Adafruit_PM25AQI();
pms_mode_t SensorPmsa003i::mode = PMS_____OFF;
int SensorPmsa003i::WARMUP_SECONDS = 30;
bool SensorPmsa003i::ACTIVE = false;
ValuesPms SensorPmsa003i::values;
gpio_num_t SensorPmsa003i::PMS_ENABLE___GPIO = GPIO_NUM_17;

void SensorPmsa003i::begin() {

  if (SensorPmsa003i::ACTIVE) {
    
    SensorPmsa003i::baseSensor.begin_I2C();

    // off initially
    pinMode(SensorPmsa003i::PMS_ENABLE___GPIO, OUTPUT);
    digitalWrite(SensorPmsa003i::PMS_ENABLE___GPIO, LOW);

  }

}

void SensorPmsa003i::setMode(pms_mode_t mode) {

  if (SensorPmsa003i::ACTIVE) {

    SensorPmsa003i::mode = mode;
    if (SensorPmsa003i::mode == PMS____ON_M || SensorPmsa003i::mode == PMS____ON_D) {
      digitalWrite(PMS_ENABLE___GPIO, HIGH);
    } else {
      digitalWrite(PMS_ENABLE___GPIO, LOW);
    }

  }

}

pms_mode_t SensorPmsa003i::getMode() {
  return SensorPmsa003i::mode;
}

bool SensorPmsa003i::tryRead() {
  
  if (SensorPmsa003i::ACTIVE) {

    PM25_AQI_Data data;
    if (SensorPmsa003i::getMode() == PMS_____OFF || !SensorPmsa003i::baseSensor.read(&data)) { // if off or not readable
      SensorPmsa003i::values = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
      return false;
    } else {
      values = {
        data.pm10_standard,
        data.pm25_standard,
        data.pm100_standard,
        data.particles_03um,
        data.particles_05um,
        data.particles_10um,
        data.particles_25um,
        data.particles_50um,
        data.particles_100um
      };
      return true;
    }

  } else {
    return false;
  }

}







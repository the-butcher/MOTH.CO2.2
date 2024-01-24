#include "SensorPmsa003i.h"

/**
 * ################################################
 * ## constants
 * ################################################
 */
const int PMS_ENABLE___GPIO = GPIO_NUM_17;

/**
 * ################################################
 * ## static class variabales
 * ################################################
 */
Adafruit_PM25AQI SensorPmsa003i::baseSensor = Adafruit_PM25AQI();
pms_mode_t SensorPmsa003i::mode = PMS____OFF;
int SensorPmsa003i::WARMUP_SECONDS = 40;
bool SensorPmsa003i::ACTIVE = true;

RunningAverage SensorPmsa003i::avgPm010 = RunningAverage();
RunningAverage SensorPmsa003i::avgPm025 = RunningAverage();
RunningAverage SensorPmsa003i::avgPm100 = RunningAverage();

RunningAverage SensorPmsa003i::avgPc003 = RunningAverage();
RunningAverage SensorPmsa003i::avgPc005 = RunningAverage();
RunningAverage SensorPmsa003i::avgPc010 = RunningAverage();
RunningAverage SensorPmsa003i::avgPc025 = RunningAverage();
RunningAverage SensorPmsa003i::avgPc050 = RunningAverage();
RunningAverage SensorPmsa003i::avgPc100 = RunningAverage();

void SensorPmsa003i::begin() {

  if (SensorPmsa003i::ACTIVE) {
    
    SensorPmsa003i::baseSensor.begin_I2C();

    // off initially
    pinMode(PMS_ENABLE___GPIO, OUTPUT);
    digitalWrite(PMS_ENABLE___GPIO, LOW);

  }

}

void SensorPmsa003i::setMode(pms_mode_t mode) {

  if (SensorPmsa003i::ACTIVE) {

    SensorPmsa003i::mode = mode;
    SensorPmsa003i::avgPm010.reset();
    SensorPmsa003i::avgPm025.reset();
    SensorPmsa003i::avgPm100.reset();
    SensorPmsa003i::avgPc003.reset();
    SensorPmsa003i::avgPc005.reset();
    SensorPmsa003i::avgPc010.reset();
    SensorPmsa003i::avgPc025.reset();
    SensorPmsa003i::avgPc050.reset();
    SensorPmsa003i::avgPc100.reset();
    if (SensorPmsa003i::mode == PMS_____ON) {
      digitalWrite(PMS_ENABLE___GPIO, HIGH);
    } else {
      digitalWrite(PMS_ENABLE___GPIO, LOW);
    }

  }

}

pms_mode_t SensorPmsa003i::getMode() {
  return SensorPmsa003i::mode;
}

ValuesPms SensorPmsa003i::getValues() {
  return {
    SensorPmsa003i::avgPm010.getAvg(),
    SensorPmsa003i::avgPm025.getAvg(),
    SensorPmsa003i::avgPm100.getAvg(),
    round(SensorPmsa003i::avgPc003.getAvg()),
    round(SensorPmsa003i::avgPc005.getAvg()),
    round(SensorPmsa003i::avgPc010.getAvg()),
    round(SensorPmsa003i::avgPc025.getAvg()),
    round(SensorPmsa003i::avgPc050.getAvg()),
    round(SensorPmsa003i::avgPc100.getAvg())
  };
}

bool SensorPmsa003i::tryRead() {
  
  if (SensorPmsa003i::ACTIVE) {

    PM25_AQI_Data data;
    if (!SensorPmsa003i::baseSensor.read(&data)) {
      // values = { -1, -1, -1, -1, -1, -1, -1, -1, -1};
      return false;
    } else {
      // values = {
      //   data.pm10_standard,
      //   data.pm25_standard,
      //   data.pm100_standard,
      //   data.particles_03um,
      //   data.particles_05um,
      //   data.particles_10um,
      //   data.particles_25um,
      //   data.particles_50um,
      //   data.particles_100um
      // };
      if (data.pm10_standard >= 0) {
        SensorPmsa003i::avgPm010.push(data.pm10_standard);
      }
      if (data.pm25_standard >= 0) {
        SensorPmsa003i::avgPm025.push(data.pm25_standard);
      }
      if (data.pm100_standard >= 0) {
        SensorPmsa003i::avgPm100.push(data.pm100_standard);
      }
      if (data.particles_03um >= 0) {
        SensorPmsa003i::avgPc003.push(data.particles_03um);
      }
      if (data.particles_05um >= 0) {
        SensorPmsa003i::avgPc005.push(data.particles_05um);
      }
      if (data.particles_10um >= 0) {
        SensorPmsa003i::avgPc010.push(data.particles_10um);
      }
      if (data.particles_25um >= 0) {
        SensorPmsa003i::avgPc025.push(data.particles_25um);
      }
      if (data.particles_50um >= 0) {
        SensorPmsa003i::avgPc050.push(data.particles_50um);
      }
      if (data.particles_100um >= 0) {
        SensorPmsa003i::avgPc100.push(data.particles_100um);
      }
      return true;
    }

  } else {
    return false;
  }

}







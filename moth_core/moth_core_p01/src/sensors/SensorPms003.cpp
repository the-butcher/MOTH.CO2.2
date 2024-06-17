#include "SensorPms003.h"

Adafruit_PM25AQI SensorPms003::baseSensor;
values_pms_t SensorPms003::values = {0, 0, 0};

void SensorPms003::begin() {
    SensorPms003::baseSensor.begin_I2C();
}

bool SensorPms003::measure() {
    PM25_AQI_Data data;
    bool success = baseSensor.read(&data);
    if (success) {
        SensorPms003::values = {data.pm10_standard, data.pm25_standard, data.pm100_standard};
    }
    return success;
}

values_pms_t SensorPms003::readval() {
    return SensorPms003::values;
}

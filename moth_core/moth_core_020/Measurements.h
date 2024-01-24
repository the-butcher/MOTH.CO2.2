#ifndef Measurements_h
#define Measurements_h

#include <Arduino.h>
#include "Measurement.h"

class Measurements {
  
  private:
    static void saveToFile();
    static void checkCalibrationOffsetBme280();
    static int regBufferSize; // regression value count
    static int memBufferSize; // count of measurements kept in PSRAM
    static Measurement* measurements;
    static float getSlope(std::function<float(const Measurement)> func);
    static char* CSV_FRMT;
    
  public:
    static String dataFileNameCurr;
    static int64_t measurementIntervalSeconds;
    static String CSV_HEAD;
    static int csvBufferSize;
    static int memBufferIndx;
    static void begin();
    static void putMeasurement(Measurement measurement);
    static int getCsvBufferIndex();
    static Measurement getOffsetMeasurement(int offset);
    static Measurement getLatestMeasurement();
    static Measurement getMeasurement(int memIndex);
    static int getFirstPublishableIndex();
    static void setPublished(int memIndex);
    static int getPublishableCount();
    static float getTemperatureSlope();
    static float getHumiditySlope();
    static String toCsv(Measurement measurement);
    static float toMagnus(float temperatureDeg);

};

#endif
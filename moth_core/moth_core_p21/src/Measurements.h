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
    static int lowBufferSize; // low-pass filter size
    static float lowBufferVals[];
    static float lowBufferMult; // low-pass filter multiplicator (alpha)
    static Measurement* measurements;
    static char* CSV_FRMT;
    
  public:
    static String dataFileNameCurr;
    static int64_t measurementIntervalSeconds;
    static String CSV_HEAD;
    static int csvBufferSize;
    static int memBufferIndx;
    static void begin();
    static void putMeasurement(Measurement measurement);
    static void putValuesBme(ValuesBme valuesBme);
    static int getCsvBufferIndex();
    static Measurement getOffsetMeasurement(int offset);
    static Measurement getLatestMeasurement();
    static Measurement getMeasurement(int memIndex);
    static int getFirstPublishableIndex();
    static void setPublished(int memIndex);
    static int getPublishableCount();
    static String toCsv(Measurement measurement);
    static float toMagnus(float temperatureDeg);

};

#endif
#include "Measurements.h"
#include "BoxClock.h"
#include "SdFat.h"

/**
 * ################################################
 * ## static class variabales                     
 * ################################################
 */
int Measurements::regBufferSize = 5;
int Measurements::csvBufferSize = 60; // 60;
int Measurements::memBufferSize = 10080; // 10080; // 60 * 24 * 7, every minute over the last 7 days (space-wise it could also handle 14 days)
int Measurements::memBufferIndx = 0;
int64_t Measurements::microsecondsMeasurementInterval = 60000000; // 1 minute
Measurement* Measurements::measurements;
String Measurements::CSV_HEAD = "time; co2; temperature; humidity; pressure; percent; voltage\r\n";

void Measurements::begin() {

  // https://www.esp32.com/viewtopic.php?t=27771
  Measurements::measurements = (Measurement*) ps_malloc(Measurements::memBufferSize * sizeof(Measurement));
  memset(Measurements::measurements, 0, Measurements::memBufferSize * sizeof(Measurement));

  // Serial.print(memBufferSize);
  // Serial.println(" measurements allocated from PSRAM");

}

void Measurements::saveToFile() {

  DateTime date;
  String dataFileNameCurr = "";
  String dataFileNameLast = "";
  File32 csvFile;
  Measurement measurement;
  for (int offsetIndex = Measurements::csvBufferSize - 1; offsetIndex >= 0; offsetIndex--) {
    measurement = Measurements::getOffsetMeasurement(offsetIndex);
    date = DateTime(SECONDS_FROM_1970_TO_2000 + measurement.secondstime);
    dataFileNameCurr = BoxClock::getDataFileName(date); // the file name that shall be written to
    if (dataFileNameCurr != dataFileNameLast) {
      if (csvFile) {
        csvFile.sync(); // write anything pending
        csvFile.close();
      }
      csvFile.open(dataFileNameCurr.c_str(), O_RDWR | O_CREAT | O_AT_END);
      if (csvFile.size() == 0) { // first time this file is being written to -> write csv header
        csvFile.print(Measurements::CSV_HEAD);
      }      
    }
    csvFile.print(Measurements::toCsv(measurement));
    dataFileNameLast = dataFileNameCurr;
  }
  csvFile.sync();
  csvFile.close();

}

int Measurements::getCsvBufferIndex() {
  return Measurements::memBufferIndx % Measurements::csvBufferSize;
}

Measurement Measurements::getOffsetMeasurement(int offset) {
  return Measurements::measurements[(Measurements::memBufferIndx - offset - 1) % Measurements::memBufferSize];
}

Measurement Measurements::getLatestMeasurement() {
  return Measurements::measurements[(Measurements::memBufferIndx - 1) % Measurements::memBufferSize];
}

void Measurements::putMeasurement(Measurement measurement) {
  Measurements::measurements[Measurements::memBufferIndx % Measurements::memBufferSize] = measurement;
  memBufferIndx++;
  if (Measurements::memBufferIndx % Measurements::csvBufferSize == 0) { // csv buffer turnaround
    Measurements::saveToFile();
  }
}

float Measurements::getTemperatureSlope() {
  return getSlope([&] (const Measurement measurement) { return measurement.valuesCo2.temperature; });
}

float Measurements::getHumiditySlope() {
  return getSlope([&] (const Measurement measurement) { return measurement.valuesCo2.humidity; });
}

float Measurements::getSlope(std::function<float(const Measurement)> func) {
  
  // pass x and y arrays (pointers), lrCoef pointer, and n. The lrCoef array is comprised of the slope=lrCoef[0] and intercept=lrCoef[1]. n is length of the x and y arrays.
  // http://en.wikipedia.org/wiki/Simple_linear_regression
  // http://jwbrooks.blogspot.com/2014/02/arduino-linear-regression-function.html

  // dont calculate before enough values are available
  if (memBufferIndx >= Measurements::regBufferSize) {

    float sumX = 0;
    float sumY = 0;
    float sumXY = 0;
    float sumXX = 0;
    float x;
    float y;
    Measurement measurement = Measurements::getOffsetMeasurement(0);
    float xRef = measurement.secondstime / 60.0;
    for (int offsetIndex = 0; offsetIndex < Measurements::regBufferSize; offsetIndex++) {
      measurement = Measurements::getOffsetMeasurement(offsetIndex);
      x = measurement.secondstime / 60.0 - xRef;
      y = func(measurement);
      sumX += x;
      sumY += y;
      sumXY += x * y;
      sumXX += x * x;
    }

    return (Measurements::regBufferSize * sumXY - sumX * sumY) / (Measurements::regBufferSize * sumXX - sumX * sumX);

  } else {
    return 0;
  }

  // simple linear regression algorithm
  // lrCoef[0]=(n*sum_xy-sum_x*sum_y)/(n*sum_xx-sum_x*sum_x);
  // lrCoef[1]=(sum_y/n)-((lrCoef[0]*sum_x)/n);

}

Measurement Measurements::getMeasurement(int memIndex) {
  return Measurements::measurements[memIndex];
}

int Measurements::getFirstPublishableIndex() {  
  // oldest first, this iterates the full array (if there is a single measurement only)
  for (int index = memBufferIndx; index < Measurements::memBufferSize + memBufferIndx; index++) {
    if (Measurements::measurements[index % memBufferIndx].publishable) {
      return index % memBufferIndx;
    }
  }
  return -1;
}

int Measurements::getPublishableCount() {  
  int publishableCount = 0;
  for (int index = 0; index < Measurements::memBufferSize; index++) {
    if (Measurements::measurements[index].publishable) {
      publishableCount++;
    }
  }
  return publishableCount;
}

void Measurements::setPublished(int memIndex) {
  Measurements::measurements[memIndex].publishable = false;
}

String Measurements::toCsv(Measurement measurement) {

  char csvBuffer[128];

  DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + measurement.secondstime);

  sprintf(csvBuffer, "%04d-%02d-%02d %02d:%02d:%02d; %s; %s; %s; %s; %s; %s\r\n", 
  date.year(), date.month(), date.day(), date.hour(), date.minute(), date.second(),
  String(measurement.valuesCo2.co2), 
  String(measurement.valuesCo2.temperature, 1), 
  String(measurement.valuesCo2.humidity, 1), 
  String(measurement.valuesBme.pressure), 
  String(measurement.valuesBat.percent, 1), 
  String(measurement.valuesBat.voltage, 3));

  // TODO :: make the number locale configurable
  for(int i = 0; i < 128; i++) {
    if(csvBuffer[i] == '.') {
      csvBuffer[i] = ',';
    }
  }

  return csvBuffer;

}




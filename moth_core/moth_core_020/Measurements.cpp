#include "Measurements.h"
#include "BoxClock.h"
#include "SdFat.h"
#include "DataFileDef.h"
#include "BoxFiles.h"
#include "SensorBme280.h"
#include "SensorPmsa003i.h"

/**
 * ################################################
 * ## constants
 * ################################################
 */
const float MAGIC_NO = 13.2473225884051;
const float MAGNUS_B = 17.670;
const float MAGNUS_C = 243.50;
const float KELVIN_Z = 273.15;

const int CALIBRATION_INTERVAL_BME280 = 20;

/**
 * ################################################
 * ## static class variabales                     
 * ################################################
 */
int Measurements::regBufferSize = 5;
int Measurements::csvBufferSize = 60; // 60;
int Measurements::memBufferSize = 10080; // 10080; // 60 * 24 * 7, every minute over the last 7 days (space-wise it could also handle 14 days)
int Measurements::memBufferIndx = 0;

int64_t Measurements::measurementIntervalSeconds = 60; // 1 minute
Measurement* Measurements::measurements;

// csv settings
String Measurements::CSV_HEAD = "time; co2; temperature; humidity; temperature_bme; humidity_bme; pressure; percent\r\n";
char* Measurements::CSV_FRMT = "%04d-%02d-%02d %02d:%02d:%02d; %s; %s; %s; %s; %s; %s; %s\r\n";
String Measurements::dataFileNameCurr = "";

void Measurements::begin() {

  // https://www.esp32.com/viewtopic.php?t=27771
  Measurements::measurements = (Measurement*) ps_malloc(Measurements::memBufferSize * sizeof(Measurement));
  memset(Measurements::measurements, 0, Measurements::memBufferSize * sizeof(Measurement));

  if (SensorPmsa003i::ACTIVE) {
    Measurements::CSV_HEAD = "time; co2; temperature; humidity; temperature_bme; humidity_bme; pm010; pm025; pm100; pc003; pc005; pc010; pc025; pc050; pc100; pressure; percent\r\n";
    Measurements::CSV_FRMT = "%04d-%02d-%02d %02d:%02d:%02d; %s; %s; %s; %s; %s; %s; %s; %s; %s; %s; %s; %s; %s; %s; %s; %s\r\n";
  }

}

float Measurements::toMagnus(float temperatureDeg) {
  return exp((MAGNUS_B * temperatureDeg) / (temperatureDeg + MAGNUS_C)) * MAGIC_NO / (KELVIN_Z + temperatureDeg);
}  

void Measurements::checkCalibrationOffsetBme280() {

  Measurement measurement;

  // calculating the variance of temperature offset between voc and co2
  double calibrationThreshold = 0.25; // TODO maybe refine this value
  double temperatureOffsetVal = 0.0;
  double temperatureOffsetAvg = 0.0;
  double temperatureOffsetVar = 0.0; // variance

  for (int offsetIndex = CALIBRATION_INTERVAL_BME280 - 1; offsetIndex >= 0; offsetIndex--) {
    measurement = Measurements::getOffsetMeasurement(offsetIndex);
    temperatureOffsetVal = measurement.valuesBme.temperature - measurement.valuesCo2.temperature;
    temperatureOffsetAvg += temperatureOffsetVal;
  }

  temperatureOffsetAvg = temperatureOffsetAvg / CALIBRATION_INTERVAL_BME280;

  for (int offsetIndex = CALIBRATION_INTERVAL_BME280 - 1; offsetIndex >= 0; offsetIndex--) {
    measurement = Measurements::getOffsetMeasurement(offsetIndex);
    temperatureOffsetVal = measurement.valuesBme.temperature - measurement.valuesCo2.temperature;
    temperatureOffsetVar += pow(temperatureOffsetVal - temperatureOffsetAvg, 2);
  }

  temperatureOffsetVar = sqrt(temperatureOffsetVar);

  if (temperatureOffsetVar < calibrationThreshold) { 
    SensorBme280::setTemperatureOffset(SensorBme280::getTemperatureOffset() + temperatureOffsetAvg);
  } 

}

void Measurements::saveToFile() {

  DateTime date;
  DataFileDef dataFileDef;
  String dataFileNameLast = "";
  String dataFilePathLast = "";
  File32 csvFile;

  Measurement measurement;
  for (int offsetIndex = Measurements::csvBufferSize - 1; offsetIndex >= 0; offsetIndex--) {
    measurement = Measurements::getOffsetMeasurement(offsetIndex);
    date = DateTime(SECONDS_FROM_1970_TO_2000 + measurement.secondstime);
    dataFileDef = BoxClock::getDataFileDef(date); // the file name that shall be written to
    if (dataFileDef.name != dataFileNameLast) {
      if (csvFile) { // file already open -> file change at midnight
        csvFile.sync(); // write anything pending
        csvFile.close();
      }
      if (dataFileDef.path != dataFilePathLast) { // if not only the file name changed, but also the path (a change in month or year, the folders need to be ready)
        BoxFiles::buildFolders(dataFileDef.path);
        dataFilePathLast = dataFileDef.path;
      }
      csvFile.open(dataFileDef.name.c_str(), O_RDWR | O_CREAT | O_AT_END);
      if (csvFile.size() == 0) { // first time this file is being written to -> write csv header
        csvFile.print(Measurements::CSV_HEAD);
      }      
      dataFileNameLast = dataFileDef.name;
    }
    csvFile.print(Measurements::toCsv(measurement));
  }

  csvFile.sync();
  csvFile.close();

  Measurements::dataFileNameCurr = dataFileNameLast;

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
  if (Measurements::memBufferIndx % CALIBRATION_INTERVAL_BME280 == 0) { // csv buffer turnaround
    Measurements::checkCalibrationOffsetBme280();
  }
  if (Measurements::memBufferIndx % Measurements::csvBufferSize == 0) { // csv buffer turnaround
    Measurements::saveToFile();
  }
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

  char csvBuffer[256];

  DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + measurement.secondstime);

  if (SensorPmsa003i::ACTIVE) {
    sprintf(csvBuffer, Measurements::CSV_FRMT, 
      date.year(), date.month(), date.day(), date.hour(), date.minute(), date.second(),
      String(measurement.valuesCo2.co2), 
      String(measurement.valuesCo2.temperature, 1), 
      String(measurement.valuesCo2.humidity, 1), 
      String(measurement.valuesBme.temperature, 1), 
      String(measurement.valuesBme.humidity, 1),
      String((int)round(measurement.valuesPms.pm010)), 
      String((int)round(measurement.valuesPms.pm025)), 
      String((int)round(measurement.valuesPms.pm100)), 
      String(measurement.valuesPms.pc003), 
      String(measurement.valuesPms.pc005), 
      String(measurement.valuesPms.pc010), 
      String(measurement.valuesPms.pc025), 
      String(measurement.valuesPms.pc050), 
      String(measurement.valuesPms.pc100), 
      String(measurement.valuesBme.pressure),
      String(measurement.valuesBat.percent, 1)
    );
  } else {
    sprintf(csvBuffer, Measurements::CSV_FRMT, 
      date.year(), date.month(), date.day(), date.hour(), date.minute(), date.second(),
      String(measurement.valuesCo2.co2), 
      String(measurement.valuesCo2.temperature, 1), 
      String(measurement.valuesCo2.humidity, 1), 
      String(measurement.valuesBme.temperature, 1), 
      String(measurement.valuesBme.humidity, 1),
      String(measurement.valuesBme.pressure),
      String(measurement.valuesBat.percent, 1)
    );
  }


  // TODO :: make the number locale configurable
  for(int i = 0; i < 256; i++) {
    if(csvBuffer[i] == '.') {
      csvBuffer[i] = ',';
    }
  }

  return csvBuffer;

}




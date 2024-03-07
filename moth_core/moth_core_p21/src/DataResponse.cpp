#include "DataResponse.h"
#include "Measurements.h"

#define MAX_MEASUREMENT_COUNT 60

DataResponse::~DataResponse(){
  // nothing to do
}

DataResponse::DataResponse(): AsyncAbstractResponse(){

  _code = 200;
  _offsetIndex = min(MAX_MEASUREMENT_COUNT, Measurements::memBufferIndx); // not more than 60, not more than available

  _contentLength = 0;
  _contentLength += Measurements::CSV_HEAD.length();
  for (int _offsetIndexLength = _offsetIndex - 1; _offsetIndexLength >= 0; _offsetIndexLength--) {
    _contentLength += Measurements::toCsv(Measurements::getOffsetMeasurement(_offsetIndexLength)).length();
  }  

  _contentType = "text/csv";

}

size_t DataResponse::_fillBuffer(uint8_t *data, size_t len) {

  int readSize = 0;
  if (_offsetIndex >= 0) {

    String csvLine;
    if (_offsetIndex == min(MAX_MEASUREMENT_COUNT, Measurements::memBufferIndx)) {
      csvLine = Measurements::CSV_HEAD;
    } else {
      csvLine = Measurements::toCsv(Measurements::getOffsetMeasurement(_offsetIndex));
    }
    readSize = csvLine.length();

    char csvLineCharBuffer[readSize + 1];
    csvLine.toCharArray(csvLineCharBuffer, readSize + 1);

    // copy to buffer
    for (int i = 0; i < readSize; i++){
      data[i] = csvLineCharBuffer[i];
    }

    _offsetIndex--;

  } 
  return readSize;

}


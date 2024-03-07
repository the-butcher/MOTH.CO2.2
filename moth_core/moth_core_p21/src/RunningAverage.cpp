#include "RunningAverage.h"

RunningAverage::RunningAverage() {
  valueIndex = 0;
  dirty = false;
  avg = 0;
}

void RunningAverage::push(float value) {
  valueArray[valueIndex % dim] = value;
  valueIndex++;
  dirty = true;
}

void RunningAverage::reset() {
  valueIndex = 0;
  dirty = false;
  avg = 0;
}

void RunningAverage::recalculate() {

  if (dirty) {

    // // clone value array
    // float _valueArray[dim];
    // for (int i = 0; i < dim; i++) {
    //   _valueArray[i] = valueArray[i];
    // }

    // // sort the cloned array
    // for (int i = 0; i < dim - 1; i++) {
    //   for (int j = i + 1; j < dim; j++) {
    //     if (_valueArray[i] > _valueArray[j]) {
    //       float swap = _valueArray[i];
    //       _valueArray[i] = _valueArray[j];
    //       _valueArray[j] = swap;
    //     }
    //   }
    // }

    float _avg = 0;
    for (int i = 0; i < dim; i++) {
      _avg += valueArray[i];
    }
    avg = _avg / dim;

    dirty = false;

  }

}

float RunningAverage::getAvg() {
  if (dirty) {
    recalculate();
  }
  return avg;
}


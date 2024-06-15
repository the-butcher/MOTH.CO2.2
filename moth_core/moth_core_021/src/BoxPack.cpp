#include "BoxPack.h"
#include "Measurements.h"
#include "Measurement.h"

/**
 * ################################################
 * ## static class variabales                     
 * ################################################
 */
Adafruit_LC709203F BoxPack::basePack;
ValuesBat BoxPack::values;

void BoxPack::begin() {
  BoxPack::basePack.begin();
  BoxPack::basePack.setPackSize(LC709203F_APA_3000MAH);
  BoxPack::basePack.setAlarmVoltage(3.8);
}

bool BoxPack::tryRead() {
  BoxPack::values = {
    BoxPack::basePack.cellPercent(),
    BoxPack::basePack.cellVoltage(),
    BoxPack::isPowered()
  };
  return true;
}

bool BoxPack::isPowered() {
  if (Measurements::memBufferIndx > 1) {
    Measurement measurementA = Measurements::getOffsetMeasurement(0);
    Measurement measurementB = Measurements::getOffsetMeasurement(2);
    if (measurementA.valuesBat.percent > measurementB.valuesBat.percent) {
      return true; // going up
    } else if (measurementA.valuesBat.percent >= 98 && measurementA.valuesBat.percent == measurementB.valuesBat.percent) {
      return true; // high and staying stable
    } else {
      return false;
    }
  }
  return false;
}


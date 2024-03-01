#include "BoxClock.h"
#include "BoxBeep.h"
#include "BoxConn.h"
#include "BoxMqtt.h"
#include "BoxEncr.h"
#include "BoxPack.h"
#include "BoxFiles.h"
#include "BoxDisplay.h"
#include "ButtonHandler.h"
#include "ButtonHandlers.h"
#include "LoopReason.h"

#include "Measurements.h"
#include "Measurement.h"

#include "SensorBme280.h" // wrapper for bme280 sensor
#include "SensorScd041.h" // wrapper for scd41 sensor
#include "SensorPmsa003i.h"

#include "driver/rtc_io.h"

typedef enum {
    SENSORS_TRYREAD, // sensors need to read before measurement can be taken
    SENSORS_GETVALS // sensors can provide values from measurements previously taken
} sensors_mode_t;

loop_reason_t loopReason = LOOP_REASON___________UNKNOWN;
loop_reason_t loopAction;

sensors_mode_t sensorsMode = SENSORS_TRYREAD; // start with SENSORS_TRYREAD (measurement needs to be taken)

const int64_t MICROSECONDS_PER_SECOND = 1000000; // 1 second
const int64_t MILLISECONDS_PER_SECOND = 1000;    // 1 second
const int64_t MEASUREMENT_WAIT_SECONDS_MAX = 1;
const int64_t WAIT_SECONDS_NEVER = 60 * 60 * 24; // 24 hours, which will effectively never be waited for due to other (shorter) timeouts
const int64_t TRYREAD_SECONDS = 6;

int64_t offsetBeginSeconds = 0;
int64_t lastMemBufferIndex = -2; // the last mem buffer index that got rendered

bool isAudio;

std::function<void(void)> displayFunc = nullptr; // [=]()->void{};
// TODO :: define functions for all buttons (maybe attachable to the button handlers)
// this could maybe create the possibility to move the handleButtonChangeA, ... methods to the button handlers
// could there be a typedef that holds both action AND display character for that action

/**
 * -- perform tests with various configurations, including invalid ones
 *
 * -- validate (test)
 *    ✓ preventSleep == false
 *    ✓ csvBufferSize == 60
 *    ✓ csv file is written
 *    ✓ file response is working
 *    ✓ mqtt config was successfully read
 *    ✓ encr was successfully read
 *    ✓ timezone works
 *    ✓ time gets adjusted while permanently being online
 *    ✓ time gets adjusted hourly when offline
 *    ✓ mqtt gets published hourly when offline
 */

void setup() {

  Serial.begin(115200);
  Wire.begin();
  delay(2000);

  BoxBeep::begin();
  BoxFiles::begin();
  BoxClock::begin();
  BoxDisplay::begin(); // needs BoxFiles to be ready because it will read config

  BoxPack::begin();
  BoxEncr::begin(); // needs BoxFiles to be ready because it will read config
  BoxConn::begin(); // needs BoxFiles and BoxEncr to be ready because it will read config containing passwords
  BoxMqtt::begin(); // needs BoxFiles and BoxEncr to be ready because it will read config containing passwords
  Measurements::begin();

  BoxPack::tryRead(); // need to read, or no battery values will be present in the starting info
  BoxDisplay::renderMothInfo(BoxConn::VNUM);
  BoxDisplay::hibernate(true); // must be true here (isAwakeRequired), because Serial would get lost otherwise

  ButtonHandlers::begin();

  SensorBme280::begin();
  SensorScd041::begin();
  SensorPmsa003i::begin();

  offsetBeginSeconds = BoxClock::getDate().secondstime();

}

int64_t getMeasureNextSeconds() {
  return (Measurements::memBufferIndx + 1) * Measurements::measurementIntervalSeconds + offsetBeginSeconds;
}

int64_t getMeasureWaitSeconds() {
  return getMeasureNextSeconds() - BoxClock::getDate().secondstime();
}

int64_t getDisplayNextSeconds() {
  return (lastMemBufferIndex + BoxDisplay::renderStateSeconds / 60) * Measurements::measurementIntervalSeconds + offsetBeginSeconds;
}

int64_t getDisplayWaitSeconds() {
  return getDisplayNextSeconds() - BoxClock::getDate().secondstime();
}

int64_t getTryReadWaitSeconds() {
  if (sensorsMode == SENSORS_TRYREAD) {
    return getMeasureWaitSeconds() - TRYREAD_SECONDS; // 5 seconds is the command duration of a single shot measurement
  } else {
    return WAIT_SECONDS_NEVER;
  }
}

int64_t getWarmupWaitSeconds() {
  if (SensorPmsa003i::getMode() == PMS_PAUSE_M) {
    return getMeasureWaitSeconds() - SensorPmsa003i::WARMUP_SECONDS;
  }
  else if (SensorPmsa003i::getMode() == PMS_PAUSE_D) {
    return getDisplayWaitSeconds() - SensorPmsa003i::WARMUP_SECONDS;
  }
  else {
    return WAIT_SECONDS_NEVER;
  }
}

void handleButtonChangeA() {
  fallrise_t fallrise11 = ButtonHandlers::A.getFallRise();
  if (fallrise11 == FALL_RISE_FAST) {
    if (ButtonHandlers::A.buttonActionFast.loopReason != LOOP_REASON___________UNKNOWN) {
      loopReason = ButtonHandlers::A.buttonActionFast.loopReason;
    }
  } else if (fallrise11 == FALL_RISE_SLOW) {
    if (ButtonHandlers::A.buttonActionSlow.loopReason != LOOP_REASON___________UNKNOWN) {
      loopReason = ButtonHandlers::A.buttonActionSlow.loopReason;
    }
  }
}
void handleButtonChangeB() {
  fallrise_t fallrise12 = ButtonHandlers::B.getFallRise();
  if (fallrise12 == FALL_RISE_FAST) {
    if (ButtonHandlers::B.buttonActionFast.loopReason != LOOP_REASON___________UNKNOWN) {
      loopReason = ButtonHandlers::B.buttonActionFast.loopReason;
    }
  } else if (fallrise12 == FALL_RISE_SLOW) {
    if (ButtonHandlers::B.buttonActionSlow.loopReason != LOOP_REASON___________UNKNOWN) {
      loopReason = ButtonHandlers::B.buttonActionSlow.loopReason;
    }
  }
}
void handleButtonChangeC() {
  fallrise_t fallrise13 = ButtonHandlers::C.getFallRise();
  if (fallrise13 == FALL_RISE_FAST) {
    if (ButtonHandlers::C.buttonActionFast.loopReason != LOOP_REASON___________UNKNOWN) {
      loopReason = ButtonHandlers::C.buttonActionFast.loopReason;
    }
  } else if (fallrise13 == FALL_RISE_SLOW) {
    if (ButtonHandlers::C.buttonActionSlow.loopReason != LOOP_REASON___________UNKNOWN) {
      loopReason = ButtonHandlers::C.buttonActionSlow.loopReason;
    }
  }
}

bool isRenderStateRequired() {
  return getDisplayWaitSeconds() <= 0 || BoxDisplay::hasSignificantChange();
}

/**
 * render either a current chart or current numeric values
 */
void renderState() { // bool force

  // store the last memBufferIndex to know when to redraw the next time
  lastMemBufferIndex = Measurements::memBufferIndx;

  bool publishable = BoxMqtt::isPublishable();
  bool autoConnect = BoxConn::getMode() == WIFI_OFF && (publishable || BoxClock::isUpdateable());
  if (autoConnect) {
    BoxConn::on(); // turn wifi on, the station_connected event will take care of adjusting time if BoxClock::isUpdateable() is true
  }
  else {
    BoxClock::optNtpUpdate(); // will only update if BoxClock::isUpdateable() is true, needs to be called explicitly due to no station_connected event
  }

  // it was either on in the first place or just forced to be on
  if (BoxConn::getMode() == WIFI_STA && publishable) {
    BoxMqtt::publish(); // simply publish
  }

  if (autoConnect) {
    BoxConn::off(); // if it was forced to be on, turn off directly afterwards
  }

  BoxDisplay::renderState();

}

bool isAnyButtonPressed() {
  return ButtonHandlers::A.getWakeupLevel() == GPIO_INTR_HIGH_LEVEL || ButtonHandlers::B.getWakeupLevel() == GPIO_INTR_HIGH_LEVEL || ButtonHandlers::C.getWakeupLevel() == GPIO_INTR_HIGH_LEVEL;
}

bool isAwakeRequired() {
  bool _isAwakeRequired = false; // MUST be false in deployment, or battery life will be much shorter
  return _isAwakeRequired || BoxConn::getMode() != WIFI_OFF || isAnyButtonPressed();
}

void loop() {

  BoxBeep::setPixelColor(COLOR__YELLOW);

  loopAction = loopReason;
  loopReason = LOOP_REASON___________UNKNOWN; // start new with "unknown"

  attachInterrupt(ButtonHandlers::A.ipin, handleButtonChangeA, CHANGE);
  attachInterrupt(ButtonHandlers::B.ipin, handleButtonChangeB, CHANGE);
  attachInterrupt(ButtonHandlers::C.ipin, handleButtonChangeC, CHANGE);

  int64_t warmupWaitSecondsA = getWarmupWaitSeconds();
  if (warmupWaitSecondsA <= 1) {

    if (SensorPmsa003i::getMode() == PMS_PAUSE_M) {
      SensorPmsa003i::setMode(PMS____ON_M);
    } else if (SensorPmsa003i::getMode() == PMS_PAUSE_D) {
      SensorPmsa003i::setMode(PMS____ON_D);
    }

  }

  int64_t tryReadWaitSecondsA = getTryReadWaitSeconds();
  if (tryReadWaitSecondsA <= 1) {

    BoxBeep::setPixelColor(COLOR_____RED);

    SensorBme280::tryRead();
    SensorScd041::tryRead(); // delays 5000 ms (no button press possible during that time)

    sensorsMode = SENSORS_GETVALS;
  
    BoxBeep::setPixelColor(COLOR__YELLOW);

  }

  // regardless of loopAction, a measurement will be taken if it is time to do so
  // however, there may be issues when this happens shortly before a measurement ...
  // ... in such cases it can happen that i.e. turning on WiFi consumes enough time to have a negative wait time to the next measurement
  int64_t measureWaitSecondsA = getMeasureWaitSeconds();
  if (measureWaitSecondsA <= 1) {

    BoxBeep::setPixelColor(COLOR_MAGENTA);

    // a final, short delay to hit the same second at all times, as far as possible
    if (measureWaitSecondsA == 1) {
      delay(MILLISECONDS_PER_SECOND);
    }

    // if the PMS is read in the SCD41 and BME280 tryread section, it will be 10 seconds early the fan may not have run long enough as of spec
    SensorPmsa003i::tryRead();
    BoxPack::tryRead();

    ValuesBme valuesBme = SensorBme280::getValues();
    Measurement measurement = {
        BoxClock::getDate().secondstime(),
        SensorScd041::getValues(),
        SensorPmsa003i::values,
        valuesBme,
        BoxPack::values,
        true // publishable
    };
    Measurements::putMeasurement(measurement);

    SensorScd041::setPressure(valuesBme.pressure / 100.0);

    sensorsMode = SENSORS_TRYREAD; // only after reading a measurement, another read can be tried

    // beep when on an thresholds exceeded, measuremnt needs to be re-fetched, because co2 gets altered by low-pass filter
    if (BoxBeep::getSound() == SOUND__ON) {
      int co2 = Measurements::getOffsetMeasurement(0).valuesCo2.co2;
      int co2Risk = BoxDisplay::getCo2RiskHi();
      if (co2 > co2Risk) {
        BoxBeep::beep(co2);
      }
    }

    // when the PM sensor is on, pause it after measurement
    if (SensorPmsa003i::getMode() == PMS____ON_M) {
      SensorPmsa003i::setMode(PMS_PAUSE_M);
    } else if (SensorPmsa003i::getMode() == PMS____ON_D) {
      SensorPmsa003i::setMode(PMS_PAUSE_D);
    }

    if (isRenderStateRequired()) {
      displayFunc = [=]() -> void { renderState(); };
    }

    BoxBeep::setPixelColor(COLOR__YELLOW);   

  }

  if (loopAction == LOOP_REASON______CALIBRRATION) {

    BoxBeep::beep();
    SensorScd041::stopPeriodicMeasurement(); // no effect

    /**
     * assumption is that the last three measurement are used as average reference value
     * but with single shot this may be noisy, therefore the average is subtracted from the requested value and 
     * the current low pass filter value is added in an attempt for compensation
     */
    Measurement measurement;
    int lowPassValue;
    int avgHistValue = 0;
    for (int offset = 0; offset < 3; offset++) {
      measurement = Measurements::getOffsetMeasurement(offset);
      if (offset == 0) {
        lowPassValue = measurement.valuesCo2.co2; // subtract the low-pass filtered value
      }
      avgHistValue += measurement.valuesCo2.co2Raw; // add the average of the last three measurements
    }

    int requestedCalibrationReference = BoxConn::requestedCalibrationReference - lowPassValue + (int)round(avgHistValue / 3.0);

    uint16_t result = SensorScd041::forceCalibration(requestedCalibrationReference);
    SensorScd041::startPeriodicMeasurement();

    // reset calibration reference to avoid recursive calibration
    BoxConn::requestedCalibrationReference = -1;

    if (result == 0xffff) {
      displayFunc = [=]() -> void { BoxDisplay::renderMothInfo("failure"); };
    } else {
      displayFunc = [=]() -> void { BoxDisplay::renderMothInfo("success (" + String(requestedCalibrationReference) + ", " + String(result - 0x8000) + ")"); };
    }

  } else if (loopAction == LOOP_REASON_______HIBERNATION) {

    BoxBeep::beep();
    BoxConn::isHibernationRequired = false; // does not make a difference, but anyways

    BoxConn::off();
    // TODO :: write any unsaved measurements to file
    BoxDisplay::renderMothInfo("hibernated"); // not within display func!
    pinMode(I2C_POWER, OUTPUT);
    digitalWrite(I2C_POWER, LOW);                          // turn off power to stemma QT Port (https://learn.adafruit.com/assets/110811)
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL); // go to sleep, no wakeup source but reset button (maybe a combination of all three buttons is an option)

    // https://stackoverflow.com/questions/53324715/esp32-external-pin-wakeup-with-internal-pullup-resistor
    const uint64_t ext_wakeup_pin_11_mask = 1ULL << GPIO_NUM_11;
    rtc_gpio_pullup_en(GPIO_NUM_11);
    rtc_gpio_pulldown_dis(GPIO_NUM_11);
    esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_11_mask, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_deep_sleep_start();

  } else if (loopAction == LOOP_REASON_RESET_CALIBRATION) {

    BoxBeep::beep();
    BoxConn::isCo2CalibrationReset = false;
    SensorScd041::factoryReset();

  } else if (loopAction == LOOP_REASON______WIFI______ON) {

    BoxBeep::beep();
    BoxConn::on(); // turn on, dont expire immediately
    displayFunc = [=]() -> void { BoxDisplay::renderQRCode(); };

  } else if (loopAction == LOOP_REASON______WIFI_____OFF) {

    BoxBeep::beep();
    BoxConn::off();
    displayFunc = [=]() -> void { renderState(); };

  } else if (loopAction == LOOP_REASON______TOGGLE_STATE) {

    BoxBeep::beep();
    BoxDisplay::toggleState();
    displayFunc = [=]() -> void { renderState(); };

  } else if (loopAction == LOOP_REASON______TOGGLE_THEME) {

    BoxBeep::beep();
    BoxDisplay::toggleTheme();
    displayFunc = [=]() -> void { renderState(); };

  } else if (loopAction == LOOP_REASON______TOGGLE___PMS) {

    BoxBeep::beep();
    if (SensorPmsa003i::getMode() == PMS____ON_M || SensorPmsa003i::getMode() == PMS_PAUSE_M) { // if in measurement interval mode switch to display interval mode
      SensorPmsa003i::setMode(PMS_PAUSE_D);
    } else if (SensorPmsa003i::getMode() == PMS____ON_D || SensorPmsa003i::getMode() == PMS_PAUSE_D) { // if in display interval mode, switch off
      SensorPmsa003i::setMode(PMS_____OFF);
      // TODO :: 
    } else {
      SensorPmsa003i::setMode(PMS_PAUSE_M);
    }
    displayFunc = [=]() -> void { renderState(); }; // this is only to render the PMS active indicator

  } else if (loopAction == LOOP_REASON___TOGGLE_VALUE_FW) {

    BoxBeep::beep();
    BoxDisplay::toggleValueFw();
    displayFunc = [=]() -> void { renderState(); };

  } else if (loopAction == LOOP_REASON___TOGGLE_VALUE_BW) {

    BoxBeep::beep();
    BoxDisplay::toggleValueBw();
    displayFunc = [=]() -> void { renderState(); };

  } else if (loopAction == LOOP_REASON___TOGGLE_HOURS_FW) {

    BoxBeep::beep();
    BoxDisplay::toggleChartMeasurementHoursFw();
    displayFunc = [=]() -> void { renderState(); };

  } else if (loopAction == LOOP_REASON___TOGGLE_HOURS_BW) {

    BoxBeep::beep();
    BoxDisplay::toggleChartMeasurementHoursBw();
    displayFunc = [=]() -> void { renderState(); };

  } else if (loopAction == LOOP_REASON______TOGGLE_SOUND) {

    BoxBeep::beep();
    BoxBeep::toggleSound();
    displayFunc = [=]() -> void { renderState(); };

  } else if (loopAction == LOOP_REASON___ADD_10_ALTITUDE) { 

    BoxBeep::beep();
    SensorBme280::updateAltitude(10);
    displayFunc = [=]() -> void { renderState(); };    

  } else if (loopAction == LOOP_REASON___ADD_50_ALTITUDE) {

    BoxBeep::beep();
    SensorBme280::updateAltitude(50);
    displayFunc = [=]() -> void { renderState(); };    

  } else if (loopAction == LOOP_REASON___DEL_10_ALTITUDE) {

    BoxBeep::beep();
    SensorBme280::updateAltitude(-10);
    displayFunc = [=]() -> void { renderState(); };    

  } else if (loopAction == LOOP_REASON___DEL_50_ALTITUDE) {

    BoxBeep::beep();
    SensorBme280::updateAltitude(-50);
    displayFunc = [=]() -> void { renderState(); };    

  } else if (loopAction == LOOP_REASON______RENDER_STATE) {

    BoxBeep::beep();
    BoxConn::isRenderStateRequired = false;
    displayFunc = [=]() -> void { renderState(); };

  }

  // depending on display state, reassign the button handlers
  if (BoxDisplay::getState() == DISPLAY_STATE_TABLE) {
    if (BoxDisplay::getValueTable() == DISPLAY_VAL_T___ALT) {
      ButtonHandlers::assignAltitudeModifiers(); // altitude on A and B
    } else {
      // some value other than altitude
      ButtonHandlers::assignWifiAndPms(); // wifi on pms(?) on A
      ButtonHandlers::assignThemeAndState(); // theme and state on B
    }
  } else {
    ButtonHandlers::assignChartHours(); // hours modifier on A
    ButtonHandlers::assignThemeAndState(); // theme and state on B
  }

  if (displayFunc) {

    BoxBeep::setPixelColor(COLOR___WHITE);

    displayFunc();
    BoxDisplay::hibernate(isAwakeRequired());
    
    displayFunc = nullptr;

    BoxBeep::setPixelColor(COLOR__YELLOW);

  }

  // there could have been an mqtt wifi-on request, lets check for it
  if (BoxMqtt::isWifiConnectionRequested) {
    BoxMqtt::isWifiConnectionRequested = false; // wifi conn requested over mqtt
    loopReason = LOOP_REASON______WIFI______ON;
    // seems like wifi is not off yet
  }

  BoxBeep::setPixelColor(COLOR____CYAN);

  // whatever happens here, happens at least once / minute, maybe more often depending on user interaction, wifi expiriy, ...
  while (isAwakeRequired()) {

    if (BoxMqtt::isConfiguredToBeActive() && BoxConn::getMode() == WIFI_STA) {
      BoxMqtt::loop(); // maintain mqtt connection
    }

    // actions that may be requested while WiFi is on
    if (BoxConn::isExpireable()) {
      loopReason = LOOP_REASON______WIFI_____OFF; // wifi has expired
      break;
    } else if (BoxConn::requestedCalibrationReference >= 400) {
      loopReason = LOOP_REASON______CALIBRRATION; // user requested calibration
      break;
    } else if (BoxConn::isHibernationRequired) {
      loopReason = LOOP_REASON_______HIBERNATION; // user requested hibernation
      break;
    } else if (BoxConn::isCo2CalibrationReset) {
      loopReason = LOOP_REASON_RESET_CALIBRATION; // user requested calibration reset
      break;
    } else if (BoxConn::isRenderStateRequired) {
      loopReason = LOOP_REASON______RENDER_STATE;
      break;
    }

    int64_t waitSecondsB = min(MEASUREMENT_WAIT_SECONDS_MAX, min(getMeasureWaitSeconds(), min(getTryReadWaitSeconds(), getWarmupWaitSeconds()))); // not more than 1 second
    if (waitSecondsB <= 0) {
      loopReason = LOOP_REASON_______MEASUREMENT; // time to measure
      break;
    }
    else {

      int64_t _delay = waitSecondsB * MILLISECONDS_PER_SECOND / 5;
      for (int i = 0; i < 5; i++) {
        delay(_delay); // wait for some time (max 1 second)
        handleButtonChangeA();
        handleButtonChangeB();
        handleButtonChangeC();
        if (loopReason != LOOP_REASON___________UNKNOWN) {
          break;
        }
      }

      if (loopReason != LOOP_REASON___________UNKNOWN) {
        break;
      }

    }
  }

  detachInterrupt(ButtonHandlers::A.ipin);
  detachInterrupt(ButtonHandlers::B.ipin);
  detachInterrupt(ButtonHandlers::C.ipin);

  if (loopReason != LOOP_REASON___________UNKNOWN) { // doublecheck for loop reason and dont let code proceed to sleep phase
    return;
  }

  // can go to sleep, but doublecheck that there is no negative sleep
  int64_t waitSecondsC = min(getMeasureWaitSeconds(), min( getTryReadWaitSeconds(), getWarmupWaitSeconds()));

  if (waitSecondsC > 1) { // longer than one second --> sleep

    BoxBeep::setPixelColor(COLOR____BLUE);

    // gpio_wakeup_disable(GPIO_NUM_14); // TODO :: this should reference a static variable on BoxDisplay
    gpio_wakeup_disable(ButtonHandlers::A.gpin);
    gpio_wakeup_disable(ButtonHandlers::B.gpin);
    gpio_wakeup_disable(ButtonHandlers::C.gpin);

    gpio_wakeup_enable(ButtonHandlers::A.gpin, ButtonHandlers::A.getWakeupLevel());
    gpio_wakeup_enable(ButtonHandlers::B.gpin, ButtonHandlers::B.getWakeupLevel());
    gpio_wakeup_enable(ButtonHandlers::C.gpin, ButtonHandlers::C.getWakeupLevel());

    esp_sleep_enable_gpio_wakeup();
    esp_sleep_enable_timer_wakeup(waitSecondsC * MICROSECONDS_PER_SECOND);

    esp_light_sleep_start();

    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_GPIO) {
      handleButtonChangeA();
      handleButtonChangeB();
      handleButtonChangeC();
    }
    else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
      loopReason = LOOP_REASON_______MEASUREMENT;
    }
  }
}
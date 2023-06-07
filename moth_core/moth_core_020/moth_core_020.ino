#include "BoxClock.h"
#include "BoxConn.h"
#include "BoxMqtt.h"
#include "BoxEncr.h"
#include "BoxPack.h"
#include "BoxFiles.h"
#include "BoxDisplay.h"
#include "ButtonHandler.h"

#include "Measurements.h"
#include "Measurement.h"

#include "SensorBme280.h" // wrapper for bme280 sensor
#include "SensorScd041.h" // wrapper for scd41 sensor

#include "driver/rtc_io.h"

typedef enum {
  LOOP_REASON______WIFI______ON,
  LOOP_REASON______WIFI_____OFF,
  LOOP_REASON______TOGGLE_STATE,
  LOOP_REASON______TOGGLE_THEME,
  LOOP_REASON______TOGGLE_AUDIO,
  LOOP_REASON______TOGGLE_VALUE,
  LOOP_REASON______RENDER_STATE, // a simple re-render
  LOOP_REASON_______MEASUREMENT, // time for a measurement
  LOOP_REASON______CALIBRRATION, // calibrate the sensor to a given reference value
  LOOP_REASON_______HIBERNATION, // hibernate the device
  LOOP_REASON_RESET_CALIBRATION, // reset SCD41 to factory
  LOOP_REASON___________UNKNOWN
} loop_reason_t;

const int BUZZER_______FREQ = 1000; // 3755
const int BUZZER____CHANNEL = 0;
const int BUZZER_RESOLUTION = 8; // 0 - 255
const int BUZZER_______GPIO = GPIO_NUM_17;

loop_reason_t loopReason = LOOP_REASON___________UNKNOWN;
loop_reason_t loopAction;

const int64_t MICROSECONDS_PER_SECOND = 1000000; // 1 second
const int64_t MILLISECONDS_PER_SECOND = 1000; // 1 second
const int64_t MEASUREMENT_WAIT_SECONDS_MAX = 1;

int64_t offsetBeginSeconds = 0;
int64_t renderStateSeconds = 0; // -3 minutes (be sure that even cases, when esp_get_time starts negative (?) the first state gets rendered)

ButtonHandler buttonHander11(GPIO_NUM_11);
ButtonHandler buttonHander12(GPIO_NUM_12);
ButtonHandler buttonHander13(GPIO_NUM_13);

bool isAudio;

std::function<void(void)> displayFunc = nullptr; // [=]()->void{};

/**
 * -- perform tests with various configurations, including invalid ones
 *
 * -- validate
 *    ✓ preventSleep == false
 *    ✓ csvBufferSize == 60
 *    ✓ csv file is written
 *    ✓ file response is working
 *    ✓ mqtt config was successfully read
 *    ✓ encr was successfully read
 *    ✓ display config was successfully read
 *    ✓ folder listing is working
 *    ✓ upload is working
 *    ✓ timezone works
 *    ✓ time gets adjusted while permanently being online
 *    ✓ time gets adjusted hourly when offline
 *    ✓ mqtt gets published hourly when offline
 *    ✓ stale never shows minus values
 *
 */

void setup() {

  // de-power the neopixel
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, LOW);

  Serial.begin(115200);
  Wire.begin();
  delay(2000);

  BoxFiles::begin();
  BoxClock::begin();
  BoxDisplay::begin(); // needs BoxFiles to be ready because it will read config

  buttonHander11.begin();
  buttonHander12.begin();
  buttonHander13.begin();

  // pinMode(GPIO_NUM_17, OUTPUT); // sound
  ledcSetup(BUZZER____CHANNEL, BUZZER_______FREQ, BUZZER_RESOLUTION);
  ledcAttachPin(BUZZER_______GPIO, BUZZER____CHANNEL);

  BoxPack::begin();
  BoxEncr::begin(); // needs BoxFiles to be ready because it will read config
  BoxConn::begin(); // needs BoxFiles and BoxEncr to be ready because it will read config containing passwords
  BoxMqtt::begin(); // needs BoxFiles and BoxEncr to be ready because it will read config containing passwords
  Measurements::begin();

  BoxPack::tryRead(); // need to read, or no battery values will be present in the starting info
  BoxDisplay::renderMothInfo(BoxConn::VNUM);

  SensorBme280::begin();
  SensorScd041::begin();

  offsetBeginSeconds = BoxClock::getDate().secondstime();

}

int64_t getMeasurementNextSeconds() {
  return (Measurements::memBufferIndx + 1) * Measurements::measurementIntervalSeconds + offsetBeginSeconds;
}

int64_t getMeasurementWaitSeconds() {
  return getMeasurementNextSeconds() - BoxClock::getDate().secondstime();
}

void handleButton11Change() {
  fallrise_t fallrise11 = buttonHander11.getFallRise();
  if (fallrise11 == FALL_RISE_FAST) {
    loopReason = BoxConn::getMode() == WIFI_OFF ? LOOP_REASON______WIFI______ON : LOOP_REASON______WIFI_____OFF;
  }
}
void handleButton12Change() {
  fallrise_t fallrise12 = buttonHander12.getFallRise();
  if (fallrise12 == FALL_RISE_FAST) {
    loopReason = LOOP_REASON______TOGGLE_STATE;
  } else if (fallrise12 == FALL_RISE_SLOW) {
    loopReason = LOOP_REASON______TOGGLE_THEME;
  }
}
void handleButton13Change() {
  fallrise_t fallrise13 = buttonHander13.getFallRise();
  if (fallrise13 == FALL_RISE_FAST) {
    if (BoxDisplay::getState() == DISPLAY_STATE_CHART) {
      loopReason = LOOP_REASON______TOGGLE_VALUE;      
    } else {
      loopReason = LOOP_REASON______TOGGLE_AUDIO;
    }
  }
}

void beep() {
  ledcWrite(BUZZER____CHANNEL, 255);
  ledcWriteTone(BUZZER____CHANNEL, BUZZER_______FREQ); // 3755
  delay(50);
  ledcWrite(BUZZER____CHANNEL, 0);  
}

/**
 * render either a current chart or current numeric values
 */
void renderState(bool force) {
  if (force || (BoxClock::getDate().secondstime() - renderStateSeconds) >= BoxDisplay::renderStateSeconds) {

    renderStateSeconds = BoxClock::getDate().secondstime() - 10; // add some safety to be sure the first state gets rendered

    bool publishable = BoxMqtt::isPublishable();
    bool autoConnect = BoxConn::getMode() == WIFI_OFF && (publishable || BoxClock::isUpdateable());
    if (autoConnect) {
      BoxConn::on(); // turn wifi on, the station_connected event will take care of adjusting time if BoxClock::isUpdateable() is true
    } else {
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
}

void loop() {

  loopAction = loopReason;
  loopReason = LOOP_REASON___________UNKNOWN; // start new with "unknown"

  attachInterrupt(buttonHander11.ipin, handleButton11Change, CHANGE);
  attachInterrupt(buttonHander12.ipin, handleButton12Change, CHANGE);
  attachInterrupt(buttonHander13.ipin, handleButton13Change, CHANGE);

  // regardless of loopAction, a measurement will be taken if it is time to do so
  // however, there may be issues when this happens shortly before a measurement ...
  // ... in such cases it can happen that i.e. turning on WiFi consumes enough time to have a negative wait time to the next measurement
  int64_t measureWaitSecondsA = getMeasurementWaitSeconds();
  if (measureWaitSecondsA <= 1) {

    // a final, short delay to hit the same second at all times, as far as possible
    if (measureWaitSecondsA == 1) {
      delay(MILLISECONDS_PER_SECOND);
    }

    SensorBme280::tryRead();
    SensorScd041::tryRead();
    BoxPack::tryRead();
    SensorScd041::setPressure(SensorBme280::values.pressure / 100.0);

    Measurement measurement = {
      BoxClock::getDate().secondstime(),
      SensorScd041::values,
      SensorBme280::values,
      BoxPack::values,
      true // publishable
    };
    Measurements::putMeasurement(measurement);

    if (isAudio && measurement.valuesCo2.co2 >= BoxDisplay::thresholdsCo2.riskHi) {
      beep();
    }
    displayFunc = [=]()->void{ renderState(false); };

  }

  if (loopAction == LOOP_REASON______CALIBRRATION) {

    beep(); // confirmation beep
    SensorScd041::stopPeriodicMeasurement();
    delay(500);
    uint16_t result = SensorScd041::forceCalibration(BoxConn::requestedCalibrationReference);
    delay(400);
    SensorScd041::startPeriodicMeasurement();

    // reset calibration reference to avoid recursive calibration
    BoxConn::requestedCalibrationReference = -1;

    if (result == 0xffff) {
      displayFunc = [=]()->void{ BoxDisplay::renderMothInfo("failure"); };
    } else {
      displayFunc = [=]()->void{ BoxDisplay::renderMothInfo("success (" + String(result - 0x8000) + ")"); };
    }

  } else if (loopAction == LOOP_REASON_______HIBERNATION) {

    beep(); // confirmation beep
    BoxConn::isHibernationRequired = false; // does not make a difference, but anyways

    BoxConn::off();
    // TODO :: write any unsaved measurements to file
    BoxDisplay::renderMothInfo("hibernated"); // not within display func!
    pinMode(I2C_POWER, OUTPUT);
    digitalWrite(I2C_POWER, LOW); // turn off power to stemma QT Port (https://learn.adafruit.com/assets/110811)
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL); // go to sleep, no wakeup source but reset button (maybe a combination of all three buttons is an option)

    // https://stackoverflow.com/questions/53324715/esp32-external-pin-wakeup-with-internal-pullup-resistor
    const uint64_t ext_wakeup_pin_11_mask = 1ULL << GPIO_NUM_11;
    rtc_gpio_pullup_en(GPIO_NUM_11);
    rtc_gpio_pulldown_dis(GPIO_NUM_11);
    esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_11_mask, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_deep_sleep_start();

  } else if (loopAction == LOOP_REASON_RESET_CALIBRATION) {

    beep(); // confirmation beep
    BoxConn::isCo2CalibrationReset = false;
    SensorScd041::factoryReset();

  } else if (loopAction == LOOP_REASON______WIFI______ON) {

    beep(); // confirmation beep
    BoxConn::on(); // turn on, dont expire immediately
    displayFunc = [=]()->void{ BoxDisplay::renderQRCode(); };

  } else if (loopAction == LOOP_REASON______WIFI_____OFF) {

    beep(); // confirmation beep
    BoxConn::off();
    displayFunc = [=]()->void{ renderState(true); };

  } else if (loopAction == LOOP_REASON______TOGGLE_STATE) {

    beep(); // confirmation beep
    BoxDisplay::toggleState();
    displayFunc = [=]()->void{  renderState(true); };

  } else if (loopAction == LOOP_REASON______TOGGLE_THEME) {

    beep(); // confirmation beep
    BoxDisplay::toggleTheme();
    displayFunc = [=]()->void{  renderState(true); };

  } else if (loopAction == LOOP_REASON______TOGGLE_AUDIO) {

    isAudio = !isAudio;

    beep(); // confirmation beep
    delay(200);
    if (isAudio) {
      beep(); // double beep to signal "ON"
    }

  } else if (loopAction == LOOP_REASON______TOGGLE_VALUE) {

    beep(); // confirmation beep
    BoxDisplay::toggleValue();
    displayFunc = [=]()->void{ renderState(true); };
    
  } else if (loopAction == LOOP_REASON______RENDER_STATE) {

    beep(); // confirmation beep
    BoxConn::isRenderStateRequired = false;
    displayFunc = [=]()->void{ renderState(true); };

  }

  if (displayFunc) {
    displayFunc();
    displayFunc = nullptr; // [=]()->void{};
  }

  // whatever happens here, happens at least once / minute, maybe more often

  bool preventSleep = false; // MUST be false in deployment, or battery life will be much shorter
  while (preventSleep || BoxConn::getMode() != WIFI_OFF) { // no sleep while wifi is active

    // whatever happens in this loop, happens about once per second

    if (BoxMqtt::isConfiguredToBeActive() && BoxConn::getMode() == WIFI_STA) {
      BoxMqtt::loop(); // maintain mqtt connection
    }

    if (BoxConn::isExpireable()) {
      loopReason = LOOP_REASON______WIFI_____OFF; // wifi has expired
    } else if (BoxConn::requestedCalibrationReference >= 400) {
      loopReason = LOOP_REASON______CALIBRRATION; // user requested calibration
    } else if (BoxConn::isHibernationRequired) {
      loopReason = LOOP_REASON_______HIBERNATION; // user requested hibernation
    } else if (BoxConn::isCo2CalibrationReset) {
      loopReason = LOOP_REASON_RESET_CALIBRATION; // user requested calibration reset
    } else if (BoxMqtt::isWifiConnectionRequested) {
      BoxMqtt::isWifiConnectionRequested = false; // wifi conn requested over mqtt
      if (BoxConn::getMode() == WIFI_OFF) {
        loopReason = LOOP_REASON______WIFI______ON;
      }
    } else if (BoxConn::isRenderStateRequired) {
      loopReason = LOOP_REASON______RENDER_STATE;
    }

    int64_t measureWaitSecondsB = min(MEASUREMENT_WAIT_SECONDS_MAX, getMeasurementWaitSeconds()); // not more than 1 second
    if (measureWaitSecondsB <= 0) {
      loopReason = LOOP_REASON_______MEASUREMENT; // time to measure
      break;
    } else if (loopReason != LOOP_REASON___________UNKNOWN) {
      break; // expiry or user request
    } else {
      delay(measureWaitSecondsB * MILLISECONDS_PER_SECOND);
    }

  }

  detachInterrupt(buttonHander11.ipin);
  detachInterrupt(buttonHander12.ipin);
  detachInterrupt(buttonHander13.ipin);

  if (loopReason != LOOP_REASON___________UNKNOWN) { // doublecheck for loop reason and dont let code proceed to sleep phase
    return;
  }

  // can go to sleep, but doublecheck that there is no negative sleep
  int64_t measureWaitSecondsC = getMeasurementWaitSeconds();
  if (measureWaitSecondsC > 1) { // longer than one second --> sleep

    gpio_wakeup_disable(buttonHander11.gpin);
    gpio_wakeup_disable(buttonHander12.gpin);
    gpio_wakeup_disable(buttonHander13.gpin);

    gpio_wakeup_enable(buttonHander11.gpin, buttonHander11.getWakeupLevel());
    gpio_wakeup_enable(buttonHander12.gpin, buttonHander12.getWakeupLevel());
    gpio_wakeup_enable(buttonHander13.gpin, buttonHander13.getWakeupLevel());

    esp_sleep_enable_gpio_wakeup();
    esp_sleep_enable_timer_wakeup(measureWaitSecondsC * MICROSECONDS_PER_SECOND);

    esp_light_sleep_start();

    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_GPIO) {
      handleButton11Change();
      handleButton12Change();
      handleButton13Change();
    } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
      loopReason = LOOP_REASON_______MEASUREMENT;
    }

  }

}
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
#include "SensorPmsa003i.h"

#include "driver/rtc_io.h"

typedef enum {
  LOOP_REASON______WIFI______ON,
  LOOP_REASON______WIFI_____OFF,
  LOOP_REASON______TOGGLE___PMS,
  LOOP_REASON______TOGGLE_STATE, // co2, pms, chart
  LOOP_REASON______TOGGLE_THEME, // light dark
  LOOP_REASON______TOGGLE_VALUE, // within state, the acual value being shown (co2, pm1.0, pm2.5, pm10.0), let a long press return to co2
  LOOP_REASON______RESET__VALUE,
  LOOP_REASON______RENDER_STATE, // a simple re-render
  LOOP_REASON_______MEASUREMENT, // time for a measurement or for sensor wakeup
  LOOP_REASON______CALIBRRATION, // calibrate the sensor to a given reference value
  LOOP_REASON_______HIBERNATION, // hibernate the device
  LOOP_REASON_RESET_CALIBRATION, // reset SCD41 to factory
  LOOP_REASON___________UNKNOWN
} loop_reason_t;

const int BUZZER____FREQ_LO = 1000; // 3755
const int BUZZER____CHANNEL = 0;
const int BUZZER_RESOLUTION = 8; // 0 - 255
const int BUZZER_______GPIO = SensorPmsa003i::ACTIVE ? GPIO_NUM_8 : GPIO_NUM_17;

loop_reason_t loopReason = LOOP_REASON___________UNKNOWN;
loop_reason_t loopAction;

const int64_t MICROSECONDS_PER_SECOND = 1000000; // 1 second
const int64_t MILLISECONDS_PER_SECOND = 1000; // 1 second
const int64_t MEASUREMENT_WAIT_SECONDS_MAX = 1;
const int64_t WARMUP_WAIT_SECONDS_NEVER = 60 * 60 * 24;

int64_t offsetBeginSeconds = 0;
int64_t lastMemBufferIndex = -2; // the last mem buffer index that got rendered

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

  ledcSetup(BUZZER____CHANNEL, BUZZER____FREQ_LO, BUZZER_RESOLUTION);
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

int64_t getWarmupWaitSeconds() {
  if (SensorPmsa003i::getMode() == PMS_PAUSE_M) {
    return getMeasureWaitSeconds() - SensorPmsa003i::WARMUP_SECONDS;
  } else if (SensorPmsa003i::getMode() == PMS_PAUSE_D) {
    return getDisplayWaitSeconds() - SensorPmsa003i::WARMUP_SECONDS;
  } else {
    return WARMUP_WAIT_SECONDS_NEVER;
  }
}

void handleButton11Change() {
  fallrise_t fallrise11 = buttonHander11.getFallRise();
  if (fallrise11 == FALL_RISE_FAST) {
    if (SensorPmsa003i::ACTIVE) {
      loopReason = LOOP_REASON______TOGGLE___PMS;
    }
  } else if (fallrise11 == FALL_RISE_SLOW) {
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
    loopReason = LOOP_REASON______TOGGLE_VALUE;       
  } else if (fallrise13 == FALL_RISE_SLOW) {
    loopReason = LOOP_REASON______RESET__VALUE;
  }
}

void beep(int frequency) {
  ledcWrite(BUZZER____CHANNEL, 255);
  ledcWriteTone(BUZZER____CHANNEL, frequency); // 3755
  delay(50);
  ledcWrite(BUZZER____CHANNEL, 0);  
}

/**
 * render either a current chart or current numeric values
 */
void renderState(bool force) {
  
  if (force || getDisplayWaitSeconds() <= 0) {

    // store the last memBufferIndex to know when to redraw the next time
    lastMemBufferIndex = Measurements::memBufferIndx;

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

bool isAnyButtonPressed() {
  return buttonHander11.getWakeupLevel() == GPIO_INTR_HIGH_LEVEL || buttonHander12.getWakeupLevel() == GPIO_INTR_HIGH_LEVEL || buttonHander13.getWakeupLevel() == GPIO_INTR_HIGH_LEVEL;
}

void loop() {

  loopAction = loopReason;
  loopReason = LOOP_REASON___________UNKNOWN; // start new with "unknown"

  attachInterrupt(buttonHander11.ipin, handleButton11Change, CHANGE);
  attachInterrupt(buttonHander12.ipin, handleButton12Change, CHANGE);
  attachInterrupt(buttonHander13.ipin, handleButton13Change, CHANGE);

  int64_t measureWaitSecondsA = getMeasureWaitSeconds();
  int64_t displayWaitSecondsA = getDisplayWaitSeconds();
  int64_t warmupWaitSecondsA = getWarmupWaitSeconds();

  if (warmupWaitSecondsA <= 1) {
    if (SensorPmsa003i::getMode() == PMS_PAUSE_M) {
      SensorPmsa003i::setMode(PMS____ON_M);
    } else if (SensorPmsa003i::getMode() == PMS_PAUSE_D) {
      SensorPmsa003i::setMode(PMS____ON_D);
    }
  }

  // regardless of loopAction, a measurement will be taken if it is time to do so
  // however, there may be issues when this happens shortly before a measurement ...
  // ... in such cases it can happen that i.e. turning on WiFi consumes enough time to have a negative wait time to the next measurement
  if (measureWaitSecondsA <= 1) {

    // a final, short delay to hit the same second at all times, as far as possible
    if (measureWaitSecondsA == 1) {
      delay(MILLISECONDS_PER_SECOND);
    }

    SensorBme280::tryRead();
    SensorScd041::tryRead();
    SensorPmsa003i::tryRead();
    BoxPack::tryRead();
    SensorScd041::setPressure(SensorBme280::values.pressure / 100.0);

    Measurement measurement = {
      BoxClock::getDate().secondstime(),
      SensorScd041::values,
      SensorPmsa003i::values,
      SensorBme280::values,
      BoxPack::values,
      true // publishable
    };
    Measurements::putMeasurement(measurement);

    // when the PM sensor is on, pause it after measurement
    if (SensorPmsa003i::getMode() == PMS____ON_M) {
      SensorPmsa003i::setMode(PMS_PAUSE_M);
    } else if (SensorPmsa003i::getMode() == PMS____ON_D) {
      SensorPmsa003i::setMode(PMS_PAUSE_D);
    }

    displayFunc = [=]()->void{ renderState(false); };

  }

  if (loopAction == LOOP_REASON______CALIBRRATION) {

    beep(BUZZER____FREQ_LO);
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

    beep(BUZZER____FREQ_LO);
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

    beep(BUZZER____FREQ_LO);
    BoxConn::isCo2CalibrationReset = false;
    SensorScd041::factoryReset();

  } else if (loopAction == LOOP_REASON______WIFI______ON) {

    beep(BUZZER____FREQ_LO);
    BoxConn::on(); // turn on, dont expire immediately
    displayFunc = [=]()->void{ BoxDisplay::renderQRCode(); };

  } else if (loopAction == LOOP_REASON______WIFI_____OFF) {

    beep(BUZZER____FREQ_LO);
    BoxConn::off();
    displayFunc = [=]()->void{ renderState(true); };

  } else if (loopAction == LOOP_REASON______TOGGLE_STATE) {

    beep(BUZZER____FREQ_LO);
    BoxDisplay::toggleState();
    displayFunc = [=]()->void{  renderState(true); };

  } else if (loopAction == LOOP_REASON______TOGGLE_THEME) {

    beep(BUZZER____FREQ_LO);
    BoxDisplay::toggleTheme();
    displayFunc = [=]()->void{  renderState(true); };

  } else if (loopAction == LOOP_REASON______TOGGLE___PMS) {

    beep(BUZZER____FREQ_LO);
    if (SensorPmsa003i::getMode() == PMS____ON_M || SensorPmsa003i::getMode() == PMS_PAUSE_M) { // if in measurement interval mode switch to display interval mode
      SensorPmsa003i::setMode(PMS_PAUSE_D);
    } else if (SensorPmsa003i::getMode() == PMS____ON_D || SensorPmsa003i::getMode() == PMS_PAUSE_D) { // if in display interval mode, switch off
      SensorPmsa003i::setMode(PMS_____OFF);
      BoxDisplay::resetValue(); // reset to CO2 display (or it may display zero PM values after turning off)
    } else {
      SensorPmsa003i::setMode(PMS_PAUSE_M);
    }
    displayFunc = [=]()->void{  renderState(true); }; // this is only to render the PMS active indicator

  } else if (loopAction == LOOP_REASON______TOGGLE_VALUE) {

    beep(BUZZER____FREQ_LO);
    BoxDisplay::toggleValue();
    displayFunc = [=]()->void{ renderState(true); };
    
  } else if (loopAction == LOOP_REASON______RESET__VALUE) {
    
    beep(BUZZER____FREQ_LO);
    BoxDisplay::resetValue();
    displayFunc = [=]()->void{ renderState(true); };
    
  } else if (loopAction == LOOP_REASON______RENDER_STATE) {

    beep(BUZZER____FREQ_LO);
    BoxConn::isRenderStateRequired = false;
    displayFunc = [=]()->void{ renderState(true); };

  }

  if (displayFunc) {
    displayFunc();
    displayFunc = nullptr; // [=]()->void{};
  }

  // there could have been an mqtt wifi-on request, lets check for it
  if (BoxMqtt::isWifiConnectionRequested) {
    BoxMqtt::isWifiConnectionRequested = false; // wifi conn requested over mqtt
    loopReason = LOOP_REASON______WIFI______ON;
    // seems like wifi is not off yet
  }

  // whatever happens here, happens at least once / minute, maybe more often depending on user interaction, wifi expiriy, ...
  bool forceAwake = false; // MUST be false in deployment, or battery life will be much shorter
  while (forceAwake || BoxConn::getMode() != WIFI_OFF || isAnyButtonPressed()) { //  || SensorPmsa003i::getMode() == PMS_____ON no sleep while wifi is active or pms is active

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

    int64_t waitSecondsB = min(MEASUREMENT_WAIT_SECONDS_MAX, min(getMeasureWaitSeconds(), getWarmupWaitSeconds())); // not more than 1 second
    if (waitSecondsB <= 0) {
      loopReason = LOOP_REASON_______MEASUREMENT; // time to measure
      break;
    } else {

      int64_t _delay = waitSecondsB * MILLISECONDS_PER_SECOND / 5;
      for (int i = 0; i < 5; i++) {
        delay(_delay); // wait for some time (max 1 second)
        handleButton11Change();
        handleButton12Change();
        handleButton13Change();
        if (loopReason != LOOP_REASON___________UNKNOWN) {
          break;
        }
      }

      if (loopReason != LOOP_REASON___________UNKNOWN) {
        break;
      } 

    }

  }

  detachInterrupt(buttonHander11.ipin);
  detachInterrupt(buttonHander12.ipin);
  detachInterrupt(buttonHander13.ipin);
  
  if (loopReason != LOOP_REASON___________UNKNOWN) { // doublecheck for loop reason and dont let code proceed to sleep phase
    return;
  }

  // can go to sleep, but doublecheck that there is no negative sleep
  int64_t waitSecondsC = min(getMeasureWaitSeconds(), getWarmupWaitSeconds());

  if (waitSecondsC > 1) { // longer than one second --> sleep

    gpio_wakeup_disable(buttonHander11.gpin);
    gpio_wakeup_disable(buttonHander12.gpin);
    gpio_wakeup_disable(buttonHander13.gpin);

    gpio_wakeup_enable(buttonHander11.gpin, buttonHander11.getWakeupLevel());
    gpio_wakeup_enable(buttonHander12.gpin, buttonHander12.getWakeupLevel());
    gpio_wakeup_enable(buttonHander13.gpin, buttonHander13.getWakeupLevel());

    esp_sleep_enable_gpio_wakeup();
    esp_sleep_enable_timer_wakeup(waitSecondsC * MICROSECONDS_PER_SECOND);

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
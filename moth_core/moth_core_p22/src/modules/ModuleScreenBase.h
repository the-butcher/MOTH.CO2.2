#ifndef ModuleScreenBase_h
#define ModuleScreenBase_h

// This file is #included by Adafruit_ThinkInk.h and does not need to
// #include anything else to pick up the EPD header or ink mode enum.

// clang-format off

static const uint8_t ti_290t5_gray4_init_code[] {
  IL0373_POWER_SETTING, 5, 0x03, 0x00, 0x2b, 0x2b, 0x13,
  IL0373_BOOSTER_SOFT_START, 3, 0x17, 0x17, 0x17,
  IL0373_POWER_ON, 0,
  0xFF, 200,
  IL0373_PANEL_SETTING, 1, 0x3F,
  IL0373_PLL, 1, 0x3C,
  IL0373_VCM_DC_SETTING, 1, 0x12,
  IL0373_CDI, 1, 0x97,
  0xFE // EOM
};

static const uint8_t ti_290t5_monopart_init_code[] {
  IL0373_POWER_SETTING, 5, 0x03, 0x00, 0x2b, 0x2b, 0x03,
  IL0373_BOOSTER_SOFT_START, 3, 0x17, 0x17, 0x17,
  IL0373_POWER_ON, 0,
  0xFF, 200,
  IL0373_PANEL_SETTING, 2, 0xbF, 0x0d,
  IL0373_PLL, 1, 0x3C,
  IL0373_VCM_DC_SETTING, 1, 0x12,
  IL0373_CDI, 1, 0x47,
  0xFE // EOM
};

static const uint8_t ti_290t5_monofull_init_code[] {
  IL0373_BOOSTER_SOFT_START, 3, 0x17, 0x17, 0x17,
  IL0373_POWER_ON, 0,
  0xFF, 200,
  IL0373_PANEL_SETTING, 2, 0x1f, 0x0d,
  IL0373_CDI, 1, 0x97,
  0xFE // EOM
};

static const uint8_t ti_290t5_monopart_lut_code[] = {

  // const unsigned char lut_vcom1[]
  0x20, 44,
  0x00, 0x01, 0x0E, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00,
  // const unsigned char lut_ww1[]
  0x21, 42,
  0x00, 0x01, 0x0E, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // const unsigned char lut_bw1[]
  0x22, 42,
  0x20, 0x01, 0x0E, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // const unsigned char lut_wb1[]
  0x23, 42,
  0x10, 0x01, 0x0E, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // const unsigned char lut_bb1[]
  0x24, 42,
  0x00, 0x01, 0x0E, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  0xFE // EOM

};

const uint8_t ti_290t5_gray4_lut_code[] = {
  //const unsigned char lut_vcom[] =
  0x20, 42,
  0x00, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x60, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x13, 0x0A, 0x01, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  //const unsigned char lut_ww[] ={
  0x21, 42,
  0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x10, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0xA0, 0x13, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  //const unsigned char lut_bw[] ={
  0x22, 42,
  0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0x99, 0x0C, 0x01, 0x03, 0x04, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  //const unsigned char lut_wb[] ={
  0x23, 42,
  0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0x99, 0x0B, 0x04, 0x04, 0x01, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  //const unsigned char lut_bb[] ={
  0x24, 42,
  0x80, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x20, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0x50, 0x13, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  0xFE // EOM

};

const int16_t PIN_EPD_DC = 10;
const int16_t PIN_EPD_RESET = 8;  // A0 -> has been bridged to the reset pin
const int16_t PIN_EPD_CS = 9;
const int16_t PIN_SRAM_CS = -1;
const int16_t PIN_EPD_BUSY = 14;  // A4 -> has been solder-connected to the busy pad

// clang-format on

class ModuleScreenBase : public Adafruit_IL0373 {
   public:
    ModuleScreenBase() : Adafruit_IL0373(296, 128, PIN_EPD_DC, PIN_EPD_RESET, PIN_EPD_CS, PIN_SRAM_CS, PIN_EPD_BUSY){};

    ModuleScreenBase(int16_t SID, int16_t SCLK, int16_t DC, int16_t RST, int16_t CS, int16_t SRCS, int16_t MISO, int16_t BUSY = -1) : Adafruit_IL0373(296, 128, SID, SCLK, DC, RST, CS, SRCS, MISO, BUSY){};

    ModuleScreenBase(int16_t DC, int16_t RST, int16_t CS, int16_t SRCS, int16_t BUSY = -1, SPIClass *spi = &SPI) : Adafruit_IL0373(296, 128, DC, RST, CS, SRCS, BUSY, spi){};

    void begin(thinkinkmode_t mode, bool isLightMode) {
        Adafruit_IL0373::begin(true);
        setColorBuffer(0, isLightMode);  // layer 0 uninverted
        setBlackBuffer(1, isLightMode);  // layer 1 uninverted

        inkmode = mode;  // Preserve ink mode for ImageReader or others

        if (mode == THINKINK_GRAYSCALE4) {
            _epd_init_code = ti_290t5_gray4_init_code;
            _epd_lut_code = ti_290t5_gray4_lut_code;

            layer_colors[EPD_WHITE] = 0b00;
            layer_colors[EPD_BLACK] = 0b11;
            layer_colors[EPD_RED] = 0b01;
            layer_colors[EPD_GRAY] = 0b10;
            layer_colors[EPD_LIGHT] = 0b01;
            layer_colors[EPD_DARK] = 0b10;

        } else if (mode == THINKINK_MONO) {
            _epd_init_code = ti_290t5_monofull_init_code;
            _epd_partial_init_code = ti_290t5_monopart_init_code;
            _epd_partial_lut_code = ti_290t5_monopart_lut_code;

            layer_colors[EPD_WHITE] = 0b11;
            layer_colors[EPD_BLACK] = 0b01;
            layer_colors[EPD_RED] = 0b01;
            layer_colors[EPD_GRAY] = 0b01;
            layer_colors[EPD_LIGHT] = 0b10;
            layer_colors[EPD_DARK] = 0b01;
        }

        default_refresh_delay = 800;

        powerDown();
    }

    void writeFrameBuffers() {
        // Adafruit_EPD::display(bool sleep)
        powerUp();
        setRAMAddress(0, 0);
        writeRAMFramebufferToEPD(buffer1, buffer1_size, 0);
        delay(2);
        setRAMAddress(0, 0);
        writeRAMFramebufferToEPD(buffer2, buffer2_size, 1);

        // Adafruit_IL0373::update()
        EPD_command(IL0373_DISPLAY_REFRESH);  // 0x12
        delay(10);                            // TODO :: was 100ms in original code

        // busy_wait();

        // // Adafruit_EPD::display(bool sleep)
        // partialsSinceLastFullUpdate = 0;
        // depower();  // will be powered down anyways
    }

    void depower() {
        busy_wait();  // doublecheck for busy
        partialsSinceLastFullUpdate = 0;
        powerDown();
        busy_wait();  // short busy wait for power down
        uint8_t buf[4];
        if (_reset_pin >= 0) {
            buf[0] = 0xA5;  // deep sleep
            EPD_command(0x07, buf, 1);
        }
    }
};

#endif  // _THINKINK_290_GRAYSCALE4_T5_H

#ifndef BoxEncr_h
#define BoxEncr_h

#include <Arduino.h>
#include "BoxStatus.h"

class BoxEncr {
  
  private:
    static String encryptAes(char * msg, byte iv[]);
    static String decryptAes(char * msg, byte iv[]);
    static void reset();

  public:
    static String CONFIG_PATH;
    static config_status_t configStatus;
    static void begin();
    static String encrypt(String value);
    static String decrypt(String value);
    static void updateConfiguration();

};

#endif
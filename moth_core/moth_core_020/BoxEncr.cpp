#include "BoxEncr.h"
#include "BoxFiles.h"
#include <ArduinoJson.h>
#include <SdFat.h>
#include "AESLib.h"

/**
 * ################################################
 * ## mutable variables
 * ################################################
 */
byte aes_key[] =        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
byte aes_inv[N_BLOCK] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
byte enc_inv[N_BLOCK] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
byte dec_inv[N_BLOCK] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

AESLib aesLib;
String key = "moth_aes128__key";
String inv = "moth_aes128__inv";

/**
 * ################################################
 * ## static class variabales                     
 * ################################################
 */
String BoxEncr::CONFIG_PATH = "/config/encr.json";
config_status_t BoxEncr::configStatus = CONFIG_STATUS_PENDING;

void BoxEncr::begin() {
  BoxEncr::updateConfiguration();
}

void BoxEncr::updateConfiguration() {

  BoxEncr::configStatus = CONFIG_STATUS_PENDING;

  String _key = key;
  String _inv = inv;
  if (BoxFiles::existsFile32(BoxEncr::CONFIG_PATH)) {

    BoxEncr::configStatus = CONFIG_STATUS_PRESENT;

    File32 encrFile;
    encrFile.open(BoxEncr::CONFIG_PATH.c_str(), O_RDONLY);

    BoxEncr::configStatus = CONFIG_STATUS__LOADED;

    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(encrFile);    
    _key = root["key"] | _key;
    _inv = root["inv"] | _inv;
    if (_key.length() == 16) {
      key = _key;
    }
    if (_inv.length() == 16) {
      inv = _inv;
    }
    encrFile.close();

    BoxEncr::configStatus = CONFIG_STATUS__PARSED;

  } else {
    BoxEncr::configStatus = CONFIG_STATUS_MISSING;
  }

  BoxEncr::reset();
  aesLib.gen_iv(aes_inv);
  aesLib.set_paddingmode((paddingMode)0);

}

void BoxEncr::reset() {
  for (int i = 0; i < 16; i++) {
    aes_key[i] = (int)key.charAt(i);
  }
  for (int i = 0; i < 16; i++) {
    aes_inv[i] = (int)inv.charAt(i);
    enc_inv[i] = (int)inv.charAt(i);
    dec_inv[i] = (int)inv.charAt(i);
  }
}

String BoxEncr::encrypt(String encryptable) {
  char encryptableBuf[encryptable.length() + 1];
  encryptable.toCharArray(encryptableBuf, encryptable.length() + 1);
  String encrypted = encryptAes(encryptableBuf, enc_inv);
  BoxEncr::reset();
  return encrypted;
}

String BoxEncr::decrypt(String decryptable) {
  char decryptableBuf[decryptable.length() + 1];
  decryptable.toCharArray(decryptableBuf, decryptable.length() + 1);
  String decrypted = decryptAes(decryptableBuf, dec_inv);
  BoxEncr::reset();
  return decrypted;
}

String BoxEncr::encryptAes(char * msg, byte iv[]) {
  int msgLen = strlen(msg);
  char encrypted[2 * msgLen] = {0};
  aesLib.encrypt64((const byte*)msg, msgLen, encrypted, aes_key, sizeof(aes_key), iv);
  return String(encrypted);
}

String BoxEncr::decryptAes(char * msg, byte iv[]) {
  int msgLen = strlen(msg);
  char decrypted[msgLen] = {0};
  aesLib.decrypt64(msg, msgLen, (byte*)decrypted, aes_key, sizeof(aes_key), iv);
  int lastValidChar = 0;
  for (int i = 0; i < sizeof(decrypted); i++)  {
    if (decrypted[i] < 0x20) {
      lastValidChar = i;
      break;
    }
  }
  return String(decrypted).substring(0, lastValidChar);
}

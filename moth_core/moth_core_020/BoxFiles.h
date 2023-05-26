#ifndef BoxFiles_h
#define BoxFiles_h

#include <Arduino.h>
#include "SdFat.h"
#include "Measurements.h"
#include "RTClib.h"

class BoxFiles {
  
  private:
    static SdFat32 sd32;
   
  public:
  
    static void begin();
    static bool buildFolders(String dataFilePath);
    static bool existsFile32(String dataFileName);
    static bool removeFile32(String dataFileName);

};

#endif
#ifndef ModuleSdcard_h
#define ModuleSdcard_h

#include <Arduino.h>
#include <SdFat.h>

#include "modules/ModuleTicker.h"
#include "sensors/SensorEnergy.h"
#include "sensors/SensorScd041.h"
#include "types/Action.h"
#include "types/Config.h"
#include "types/Define.h"
#include "types/Values.h"

class ModuleSdcard {
   private:
    static SdFat32 sd32;
    static String CSV_FRMT;
    static String CSV_HEAD;

   public:
    static void begin();
    static bool buildFolders(String folder);
    static bool removeFolder(String folder);
    static bool existsPath(String path);
    static bool removeFile32(String file);
    static void historyValues(values_t* values, config_t* config, values_all_t history[HISTORY_____BUFFER_SIZE]);
    static void persistValues(values_t* values);
    // static String toCsvLine(values_all_t* value);
};

#endif
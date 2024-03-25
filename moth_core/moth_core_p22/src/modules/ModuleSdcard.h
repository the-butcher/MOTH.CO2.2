#ifndef ModuleSdcard_h
#define ModuleSdcard_h

#include <Arduino.h>
#include <SdFat.h>

#include "sensors/SensorEnergy.h"
#include "sensors/SensorScd041.h"
#include "sensors/SensorTime.h"
#include "types/Action.h"
#include "types/Config.h"
#include "types/Define.h"
#include "types/Values.h"

const String CSV_HEAD = "time; co2; co2_raw; temperature; humidity; pressure; percent\r\n";
const String CSV_FRMT = "%04d-%02d-%02d %02d:%02d:%02d;%s;%s;%s;%s;%s;%s\r\n";

class ModuleSdcard {
   private:
    static SdFat32 sd32;
    static bool hasBegun;

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
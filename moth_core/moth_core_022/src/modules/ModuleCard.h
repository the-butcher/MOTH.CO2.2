#ifndef ModuleCard_h
#define ModuleCard_h

#include <Arduino.h>
#include <SdFat.h>

#include "sensors/SensorEnergy.h"
#include "sensors/SensorScd041.h"
#include "sensors/SensorTime.h"
#include "types/Action.h"
#include "types/Config.h"
#include "types/Define.h"
#include "types/Values.h"

const String FILE_FORMAT_DATA = "dat";
const String FILE_FORMAT_DATA_PUBLISHABLE = "dap";  // data, publishable
const String FILE_FORMAT_DATA____ARCHIVED = "dar";  // data, archived (no publishing required)
const String FILE_FORMAT_DATA_____INVALID = "";

class ModuleCard {
   private:
    static SdFat32 sd32;
    static bool hasBegun;

   public:
    static void begin();
    static bool buildFolders(String folder);
    static bool removeFolder(String folder);
    static bool existsPath(String path);
    static bool isDataPath(String path);
    static String toDataPath(String path);
    static bool removeFile32(String file);
    static bool renameFile32(String pathCurr, String pathDest);
    static void historyValues(config_t& config, values_all_t history[HISTORY_____BUFFER_SIZE]);
    static void persistValues();
};

#endif
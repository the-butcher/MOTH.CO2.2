#ifndef ModuleCard_h
#define ModuleCard_h

#include <Arduino.h>
#include <LittleFS.h>

#include "FS.h"
#include "types/Action.h"
#include "types/Config.h"
#include "types/Define.h"
#include "types/Values.h"

class ModuleCard {
   private:
    static bool hasBegun;

   public:
    static void begin();
    static bool buildFolders(String folder);
    static bool removeFile(String file);
    static bool removeFolder(String folder);
    static bool existsPath(String path);
};

#endif
#ifndef BoxData_h
#define BoxData_h

#include <SdFat.h>

#include "BoxTime.h"
#include "sensors/SensorEnergy.h";
#include "sensors/SensorScd041.h";
#include "types/Values.h"

class BoxData {
   private:
    SdFat32 sd32;
    static String CSV_FRMT;
    static String CSV_HEAD;

   public:
    void begin();
    bool buildFolders(String folder);
    bool removeFolder(String folder);
    bool existsPath(String path);
    bool removeFile32(String file);
    void persistValues(values_all_t* values, int count);
    String toCsvLine(values_all_t value);
};

#endif
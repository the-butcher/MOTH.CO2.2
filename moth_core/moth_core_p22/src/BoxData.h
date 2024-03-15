#ifndef BoxData_h
#define BoxData_h

#include "SdFat.h"

class BoxData {
   private:
    SdFat32 sd32;

   public:
    void begin();
    bool buildFolders(String folder);
    bool removeFolder(String folder);
    bool existsPath(String path);
    bool removeFile32(String file);
};

#endif
#ifndef BoxFiles_h
#define BoxFiles_h

#include "SdFat.h"

class BoxFiles {
  
  private:
    static SdFat32 sd32;
   
  public:
  
    static void begin();
    static bool buildFolders(String folder);
    static bool removeFolder(String folder);
    static bool existsPath(String path);
    static bool removeFile32(String file);

};

#endif
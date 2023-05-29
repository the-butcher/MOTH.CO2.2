#include "BoxFiles.h"

#define SD_CS 5  // SD card chip select
#define SPI_CLOCK SD_SCK_MHZ(10)

SdFat32 BoxFiles::sd32;

void BoxFiles::begin() {
  sd32.begin(SD_CS, SPI_CLOCK);
}

bool BoxFiles::buildFolders(String folder) {
  return BoxFiles::sd32.mkdir(folder);
}
bool BoxFiles::removeFolder(String folder) {
  return BoxFiles::sd32.rmdir(folder);
}
bool BoxFiles::existsPath(String path) {
  return BoxFiles::sd32.exists(path);
}
bool BoxFiles::removeFile32(String file) {
  return BoxFiles::sd32.remove(file);
}

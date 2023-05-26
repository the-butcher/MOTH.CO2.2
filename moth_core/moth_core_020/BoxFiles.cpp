#include "BoxFiles.h"

#define SD_CS 5  // SD card chip select
#define SPI_CLOCK SD_SCK_MHZ(10)

SdFat32 BoxFiles::sd32;

void BoxFiles::begin() {
  sd32.begin(SD_CS, SPI_CLOCK);
}

bool BoxFiles::buildFolders(String dataFilePath) {
  return BoxFiles::sd32.mkdir(dataFilePath);
}
bool BoxFiles::existsFile32(String fileName) {
  return BoxFiles::sd32.exists(fileName);
}
bool BoxFiles::removeFile32(String fileName) {
  return BoxFiles::sd32.remove(fileName);
}

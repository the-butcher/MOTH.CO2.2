#include "BoxData.h"

#define SD_CS 5  // SD card chip select
#define SPI_CLOCK SD_SCK_MHZ(10)

void BoxData::begin() {
    sd32.begin(SD_CS, SPI_CLOCK);
}

bool BoxData::buildFolders(String folder) {
    return BoxData::sd32.mkdir(folder);
}

bool BoxData::removeFolder(String folder) {
    return BoxData::sd32.rmdir(folder);
}

bool BoxData::existsPath(String path) {
    return BoxData::sd32.exists(path);
}

bool BoxData::removeFile32(String file) {
    return BoxData::sd32.remove(file);
}
